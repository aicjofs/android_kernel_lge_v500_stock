/*
 * devfreq: Generic Dynamic Voltage and Frequency Scaling (DVFS) Framework
 *	    for Non-CPU Devices.
 *
 * Copyright (C) 2011 Samsung Electronics
 *	MyungJoo Ham <myungjoo.ham@samsung.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/errno.h>
#include <linux/err.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/stat.h>
#include <linux/opp.h>
#include <linux/devfreq.h>
#include <linux/workqueue.h>
#include <linux/platform_device.h>
#include <linux/list.h>
#include <linux/printk.h>
#include <linux/hrtimer.h>
#include "governor.h"

struct class *devfreq_class;

/*
                                                              
                                                                     
                                                                     
                                                            
 */
static bool polling;
static struct workqueue_struct *devfreq_wq;
static struct delayed_work devfreq_work;

/*                                        */
static struct devfreq *wait_remove_device;

/*                                */
static LIST_HEAD(devfreq_list);
static DEFINE_MUTEX(devfreq_list_lock);

/* 
                                                                   
                                                      
  
                                                                     
                                                                
 */
static struct devfreq *find_device_devfreq(struct device *dev)
{
	struct devfreq *tmp_devfreq;

	if (unlikely(IS_ERR_OR_NULL(dev))) {
		pr_err("DEVFREQ: %s: Invalid parameters\n", __func__);
		return ERR_PTR(-EINVAL);
	}
	WARN(!mutex_is_locked(&devfreq_list_lock),
	     "devfreq_list_lock must be locked.");

	list_for_each_entry(tmp_devfreq, &devfreq_list, node) {
		if (tmp_devfreq->dev.parent == dev)
			return tmp_devfreq;
	}

	return ERR_PTR(-ENODEV);
}

/* 
                                                                    
                                  
  
                                                         
                                            
 */
int update_devfreq(struct devfreq *devfreq)
{
	unsigned long freq;
	int err = 0;
	u32 flags = 0;

	if (!mutex_is_locked(&devfreq->lock)) {
		WARN(true, "devfreq->lock must be locked by the caller.\n");
		return -EINVAL;
	}

	/*                                 */
	err = devfreq->governor->get_target_freq(devfreq, &freq);
	if (err)
		return err;

	/*
                                                
   
                                   
                                                           
            
  */

	if (devfreq->min_freq && freq < devfreq->min_freq) {
		freq = devfreq->min_freq;
		flags &= ~DEVFREQ_FLAG_LEAST_UPPER_BOUND; /*         */
	}
	if (devfreq->max_freq && freq > devfreq->max_freq) {
		freq = devfreq->max_freq;
		flags |= DEVFREQ_FLAG_LEAST_UPPER_BOUND; /*         */
	}

	err = devfreq->profile->target(devfreq->dev.parent, &freq, flags);
	if (err)
		return err;

	devfreq->previous_freq = freq;
	return err;
}

/* 
                                                                          
                                                  
                                                       
                 
                 
  
                                              
 */
static int devfreq_notifier_call(struct notifier_block *nb, unsigned long type,
				 void *devp)
{
	struct devfreq *devfreq = container_of(nb, struct devfreq, nb);
	int ret;

	mutex_lock(&devfreq->lock);
	ret = update_devfreq(devfreq);
	mutex_unlock(&devfreq->lock);

	return ret;
}

/* 
                                                      
                               
                                           
  
                                                                
                                                          
                                                               
                                  
  
              
                                     
                                    
                                         
                                
                                         
  
                
          
 */
static void _remove_devfreq(struct devfreq *devfreq, bool skip)
{
	if (!mutex_is_locked(&devfreq->lock)) {
		WARN(true, "devfreq->lock must be locked by the caller.\n");
		return;
	}
	if (!devfreq->governor->no_central_polling &&
	    !mutex_is_locked(&devfreq_list_lock)) {
		WARN(true, "devfreq_list_lock must be locked by the caller.\n");
		return;
	}

	if (devfreq->being_removed)
		return;

	devfreq->being_removed = true;

	if (devfreq->profile->exit)
		devfreq->profile->exit(devfreq->dev.parent);

	if (devfreq->governor->exit)
		devfreq->governor->exit(devfreq);

	if (!skip && get_device(&devfreq->dev)) {
		device_unregister(&devfreq->dev);
		put_device(&devfreq->dev);
	}

	if (!devfreq->governor->no_central_polling)
		list_del(&devfreq->node);

	mutex_unlock(&devfreq->lock);
	mutex_destroy(&devfreq->lock);

	kfree(devfreq);
}

/* 
                                                                            
                           
  
                                                                   
                                                                          
                                              
 */
static void devfreq_dev_release(struct device *dev)
{
	struct devfreq *devfreq = to_devfreq(dev);
	bool central_polling = !devfreq->governor->no_central_polling;

	/*
                                                                 
                                                               
                                                                     
                                                                  
                                                           
                                                                
   
                                        
                                                           
                  
  */
	if (devfreq->being_removed)
		return;

	if (central_polling)
		mutex_lock(&devfreq_list_lock);

	mutex_lock(&devfreq->lock);

	/*
                                                     
                                                                   
                                      
  */
	if (devfreq->being_removed) {
		mutex_unlock(&devfreq->lock);
		goto out;
	}

	/*                                                             */
	_remove_devfreq(devfreq, true);

out:
	if (central_polling)
		mutex_unlock(&devfreq_list_lock);
}

/* 
                                                         
                                                                   
  
 */
static void devfreq_monitor(struct work_struct *work)
{
	static unsigned long last_polled_at;
	struct devfreq *devfreq, *tmp;
	int error;
	unsigned long jiffies_passed;
	unsigned long next_jiffies = ULONG_MAX, now = jiffies;
	struct device *dev;

	/*                                                              */
	jiffies_passed = now - last_polled_at;
	last_polled_at = now;
	if (jiffies_passed == 0)
		jiffies_passed = 1;

	mutex_lock(&devfreq_list_lock);
	list_for_each_entry_safe(devfreq, tmp, &devfreq_list, node) {
		mutex_lock(&devfreq->lock);
		dev = devfreq->dev.parent;

		/*                               */
		wait_remove_device = tmp;

		if (devfreq->governor->no_central_polling ||
		    devfreq->next_polling == 0) {
			mutex_unlock(&devfreq->lock);
			continue;
		}
		mutex_unlock(&devfreq_list_lock);

		/*
                                                         
                                       
   */
		if (devfreq->next_polling <= jiffies_passed) {
			error = update_devfreq(devfreq);

			/*                                 */
			if (error && error != -EAGAIN) {

				dev_err(dev, "Due to update_devfreq error(%d), devfreq(%s) is removed from the device\n",
					error, devfreq->governor->name);

				/*
                                             
                                      
                                    
     */
				mutex_unlock(&devfreq->lock);
				mutex_lock(&devfreq_list_lock);
				/*                                     */
				if (IS_ERR(find_device_devfreq(dev)))
					continue;
				mutex_lock(&devfreq->lock);
				/*                                        */
				_remove_devfreq(devfreq, false);
				continue;
			}
			devfreq->next_polling = devfreq->polling_jiffies;
		} else {
			devfreq->next_polling -= jiffies_passed;
		}

		if (devfreq->next_polling)
			next_jiffies = (next_jiffies > devfreq->next_polling) ?
					devfreq->next_polling : next_jiffies;

		mutex_unlock(&devfreq->lock);
		mutex_lock(&devfreq_list_lock);
	}
	wait_remove_device = NULL;
	mutex_unlock(&devfreq_list_lock);

	if (next_jiffies > 0 && next_jiffies < ULONG_MAX) {
		polling = true;
		queue_delayed_work(devfreq_wq, &devfreq_work, next_jiffies);
	} else {
		polling = false;
	}
}

/* 
                                                           
                                           
                                                    
                                             
                                                                       
                     
 */
struct devfreq *devfreq_add_device(struct device *dev,
				   struct devfreq_dev_profile *profile,
				   const struct devfreq_governor *governor,
				   void *data)
{
	struct devfreq *devfreq;
	int err = 0;

	if (!dev || !profile || !governor) {
		dev_err(dev, "%s: Invalid parameters.\n", __func__);
		return ERR_PTR(-EINVAL);
	}


	if (!governor->no_central_polling) {
		mutex_lock(&devfreq_list_lock);
		devfreq = find_device_devfreq(dev);
		mutex_unlock(&devfreq_list_lock);
		if (!IS_ERR(devfreq)) {
			dev_err(dev, "%s: Unable to create devfreq for the device. It already has one.\n", __func__);
			err = -EINVAL;
			goto err_out;
		}
	}

	devfreq = kzalloc(sizeof(struct devfreq), GFP_KERNEL);
	if (!devfreq) {
		dev_err(dev, "%s: Unable to create devfreq for the device\n",
			__func__);
		err = -ENOMEM;
		goto err_out;
	}

	mutex_init(&devfreq->lock);
	mutex_lock(&devfreq->lock);
	devfreq->dev.parent = dev;
	devfreq->dev.class = devfreq_class;
	devfreq->dev.release = devfreq_dev_release;
	devfreq->profile = profile;
	devfreq->governor = governor;
	devfreq->previous_freq = profile->initial_freq;
	devfreq->data = data;
	devfreq->next_polling = devfreq->polling_jiffies
			      = msecs_to_jiffies(devfreq->profile->polling_ms);
	devfreq->nb.notifier_call = devfreq_notifier_call;

	dev_set_name(&devfreq->dev, dev_name(dev));
	err = device_register(&devfreq->dev);
	if (err) {
		put_device(&devfreq->dev);
		goto err_dev;
	}

	if (governor->init)
		err = governor->init(devfreq);
	if (err)
		goto err_init;

	mutex_unlock(&devfreq->lock);

	if (governor->no_central_polling)
		goto out;

	mutex_lock(&devfreq_list_lock);

	list_add(&devfreq->node, &devfreq_list);

	if (devfreq_wq && devfreq->next_polling && !polling) {
		polling = true;
		queue_delayed_work(devfreq_wq, &devfreq_work,
				   devfreq->next_polling);
	}
	mutex_unlock(&devfreq_list_lock);
out:
	return devfreq;

err_init:
	device_unregister(&devfreq->dev);
err_dev:
	mutex_unlock(&devfreq->lock);
	kfree(devfreq);
err_out:
	return ERR_PTR(err);
}

/* 
                                                                  
                                              
 */
int devfreq_remove_device(struct devfreq *devfreq)
{
	bool central_polling;

	if (!devfreq)
		return -EINVAL;

	central_polling = !devfreq->governor->no_central_polling;

	if (central_polling) {
		mutex_lock(&devfreq_list_lock);
		while (wait_remove_device == devfreq) {
			mutex_unlock(&devfreq_list_lock);
			schedule();
			mutex_lock(&devfreq_list_lock);
		}
	}

	mutex_lock(&devfreq->lock);
	_remove_devfreq(devfreq, false); /*                          */

	if (central_polling)
		mutex_unlock(&devfreq_list_lock);

	return 0;
}

static ssize_t show_governor(struct device *dev,
			     struct device_attribute *attr, char *buf)
{
	return sprintf(buf, "%s\n", to_devfreq(dev)->governor->name);
}

static ssize_t show_freq(struct device *dev,
			 struct device_attribute *attr, char *buf)
{
	return sprintf(buf, "%lu\n", to_devfreq(dev)->previous_freq);
}

static ssize_t show_polling_interval(struct device *dev,
				     struct device_attribute *attr, char *buf)
{
	return sprintf(buf, "%d\n", to_devfreq(dev)->profile->polling_ms);
}

static ssize_t store_polling_interval(struct device *dev,
				      struct device_attribute *attr,
				      const char *buf, size_t count)
{
	struct devfreq *df = to_devfreq(dev);
	unsigned int value;
	int ret;

	ret = sscanf(buf, "%u", &value);
	if (ret != 1)
		goto out;

	mutex_lock(&df->lock);
	df->profile->polling_ms = value;
	df->next_polling = df->polling_jiffies
			 = msecs_to_jiffies(value);
	mutex_unlock(&df->lock);

	ret = count;

	if (df->governor->no_central_polling)
		goto out;

	mutex_lock(&devfreq_list_lock);
	if (df->next_polling > 0 && !polling) {
		polling = true;
		queue_delayed_work(devfreq_wq, &devfreq_work,
				   df->next_polling);
	}
	mutex_unlock(&devfreq_list_lock);
out:
	return ret;
}

static ssize_t show_central_polling(struct device *dev,
				    struct device_attribute *attr, char *buf)
{
	return sprintf(buf, "%d\n",
		       !to_devfreq(dev)->governor->no_central_polling);
}

static ssize_t store_min_freq(struct device *dev, struct device_attribute *attr,
			      const char *buf, size_t count)
{
	struct devfreq *df = to_devfreq(dev);
	unsigned long value;
	int ret;
	unsigned long max;

	ret = sscanf(buf, "%lu", &value);
	if (ret != 1)
		goto out;

	mutex_lock(&df->lock);
	max = df->max_freq;
	if (value && max && value > max) {
		ret = -EINVAL;
		goto unlock;
	}

	df->min_freq = value;
	update_devfreq(df);
	ret = count;
unlock:
	mutex_unlock(&df->lock);
out:
	return ret;
}

static ssize_t show_min_freq(struct device *dev, struct device_attribute *attr,
			     char *buf)
{
	return sprintf(buf, "%lu\n", to_devfreq(dev)->min_freq);
}

static ssize_t store_max_freq(struct device *dev, struct device_attribute *attr,
			      const char *buf, size_t count)
{
	struct devfreq *df = to_devfreq(dev);
	unsigned long value;
	int ret;
	unsigned long min;

	ret = sscanf(buf, "%lu", &value);
	if (ret != 1)
		goto out;

	mutex_lock(&df->lock);
	min = df->min_freq;
	if (value && min && value < min) {
		ret = -EINVAL;
		goto unlock;
	}

	df->max_freq = value;
	update_devfreq(df);
	ret = count;
unlock:
	mutex_unlock(&df->lock);
out:
	return ret;
}

static ssize_t show_max_freq(struct device *dev, struct device_attribute *attr,
			     char *buf)
{
	return sprintf(buf, "%lu\n", to_devfreq(dev)->max_freq);
}

static struct device_attribute devfreq_attrs[] = {
	__ATTR(governor, S_IRUGO, show_governor, NULL),
	__ATTR(cur_freq, S_IRUGO, show_freq, NULL),
	__ATTR(central_polling, S_IRUGO, show_central_polling, NULL),
	__ATTR(polling_interval, S_IRUGO | S_IWUSR, show_polling_interval,
	       store_polling_interval),
	__ATTR(min_freq, S_IRUGO | S_IWUSR, show_min_freq, store_min_freq),
	__ATTR(max_freq, S_IRUGO | S_IWUSR, show_max_freq, store_max_freq),
	{ },
};

/* 
                                                                                
                                                 
 */
static int __init devfreq_start_polling(void)
{
	mutex_lock(&devfreq_list_lock);
	polling = false;
	devfreq_wq = create_freezable_workqueue("devfreq_wq");
	INIT_DELAYED_WORK_DEFERRABLE(&devfreq_work, devfreq_monitor);
	mutex_unlock(&devfreq_list_lock);

	devfreq_monitor(&devfreq_work.work);
	return 0;
}
late_initcall(devfreq_start_polling);

static int __init devfreq_init(void)
{
	devfreq_class = class_create(THIS_MODULE, "devfreq");
	if (IS_ERR(devfreq_class)) {
		pr_err("%s: couldn't create class\n", __FILE__);
		return PTR_ERR(devfreq_class);
	}
	devfreq_class->dev_attrs = devfreq_attrs;
	return 0;
}
subsys_initcall(devfreq_init);

static void __exit devfreq_exit(void)
{
	class_destroy(devfreq_class);
}
module_exit(devfreq_exit);

/*
                                                                           
                 
 */

/* 
                                                                        
                                              
                                                     
                                               
                                              
  
 */
struct opp *devfreq_recommended_opp(struct device *dev, unsigned long *freq,
				    u32 flags)
{
	struct opp *opp;

	if (flags & DEVFREQ_FLAG_LEAST_UPPER_BOUND) {
		/*                                                 */
		opp = opp_find_freq_floor(dev, freq);

		/*                                       */
		if (opp == ERR_PTR(-ENODEV))
			opp = opp_find_freq_ceil(dev, freq);
	} else {
		/*                                                  */
		opp = opp_find_freq_ceil(dev, freq);

		/*                                       */
		if (opp == ERR_PTR(-ENODEV))
			opp = opp_find_freq_floor(dev, freq);
	}

	return opp;
}

/* 
                                                                            
                                                
                
                                                     
                               
 */
int devfreq_register_opp_notifier(struct device *dev, struct devfreq *devfreq)
{
	struct srcu_notifier_head *nh = opp_get_notifier(dev);

	if (IS_ERR(nh))
		return PTR_ERR(nh);
	return srcu_notifier_chain_register(nh, &devfreq->nb);
}

/* 
                                                                              
                                              
                                        
                                                     
                               
  
                                                                      
                                   
 */
int devfreq_unregister_opp_notifier(struct device *dev, struct devfreq *devfreq)
{
	struct srcu_notifier_head *nh = opp_get_notifier(dev);

	if (IS_ERR(nh))
		return PTR_ERR(nh);
	return srcu_notifier_chain_unregister(nh, &devfreq->nb);
}

MODULE_AUTHOR("MyungJoo Ham <myungjoo.ham@samsung.com>");
MODULE_DESCRIPTION("devfreq class support");
MODULE_LICENSE("GPL");
