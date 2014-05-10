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

#ifndef __LINUX_DEVFREQ_H__
#define __LINUX_DEVFREQ_H__

#include <linux/device.h>
#include <linux/notifier.h>
#include <linux/opp.h>

#define DEVFREQ_NAME_LEN 16

struct devfreq;

/* 
                                                                     
                                               
                     
                                                              
                       
                                                             
                
                                              
                                                                 
                                                    
                                                     
                                                     
                                                      
 */
struct devfreq_dev_status {
	/*                             */
	unsigned long total_time;
	unsigned long busy_time;
	unsigned long current_frequency;
	void *private_data;
};

/*
                                                                     
                                                                       
                                                                        
                               
 */
#define DEVFREQ_FLAG_LEAST_UPPER_BOUND		0x1

/* 
                                                             
                                                                     
            
                                                               
                                                            
                                                     
                                                     
                                                 
                                   
                                                
                                                  
                                                                    
                                                   
                                                          
                                                   
                                                   
                                                   
                                       
 */
struct devfreq_dev_profile {
	unsigned long initial_freq;
	unsigned int polling_ms;

	int (*target)(struct device *dev, unsigned long *freq, u32 flags);
	int (*get_dev_status)(struct device *dev,
			      struct devfreq_dev_status *stat);
	void (*exit)(struct device *dev);
};

/* 
                                                    
                         
                                                                       
                                        
                                                    
                                                          
                                                          
                                                
                                                               
                                                         
                                                       
                                              
                        
                                                                      
                                            
                                                     
                                           
                                                
  
                                                                           
 */
struct devfreq_governor {
	const char name[DEVFREQ_NAME_LEN];
	int (*get_target_freq)(struct devfreq *this, unsigned long *freq);
	int (*init)(struct devfreq *this);
	void (*exit)(struct devfreq *this);
	const bool no_central_polling;
};

/* 
                                            
                                                                     
               
                                              
                                                                     
                  
                                           
                                                               
                                                                   
                                                          
                                                                 
                                        
                                                        
                                                             
                                               
                                            
                                   
                                                                     
               
                                                                     
                                                                 
                                                                
                                                                
  
                                                                   
  
                                                                      
                                                                  
                                                                    
                                                                     
                                                         
 */
struct devfreq {
	struct list_head node;

	struct mutex lock;
	struct device dev;
	struct devfreq_dev_profile *profile;
	const struct devfreq_governor *governor;
	struct notifier_block nb;

	unsigned long polling_jiffies;
	unsigned long previous_freq;
	unsigned int next_polling;

	void *data; /*                            */

	bool being_removed;

	unsigned long min_freq;
	unsigned long max_freq;
};

#if defined(CONFIG_PM_DEVFREQ)
extern struct devfreq *devfreq_add_device(struct device *dev,
				  struct devfreq_dev_profile *profile,
				  const struct devfreq_governor *governor,
				  void *data);
extern int devfreq_remove_device(struct devfreq *devfreq);

/*                                                           */
extern struct opp *devfreq_recommended_opp(struct device *dev,
					   unsigned long *freq, u32 flags);
extern int devfreq_register_opp_notifier(struct device *dev,
					 struct devfreq *devfreq);
extern int devfreq_unregister_opp_notifier(struct device *dev,
					   struct devfreq *devfreq);

#ifdef CONFIG_DEVFREQ_GOV_POWERSAVE
extern const struct devfreq_governor devfreq_powersave;
#endif
#ifdef CONFIG_DEVFREQ_GOV_PERFORMANCE
extern const struct devfreq_governor devfreq_performance;
#endif
#ifdef CONFIG_DEVFREQ_GOV_USERSPACE
extern const struct devfreq_governor devfreq_userspace;
#endif
#ifdef CONFIG_DEVFREQ_GOV_SIMPLE_ONDEMAND
extern const struct devfreq_governor devfreq_simple_ondemand;
/* 
                                                                         
                         
                                                                     
                                                          
                                                                          
                                                          
                                                          
                                              
  
                                                                           
                                        
 */
struct devfreq_simple_ondemand_data {
	unsigned int upthreshold;
	unsigned int downdifferential;
};
#endif

#else /*                    */
static struct devfreq *devfreq_add_device(struct device *dev,
					  struct devfreq_dev_profile *profile,
					  struct devfreq_governor *governor,
					  void *data)
{
	return NULL;
}

static int devfreq_remove_device(struct devfreq *devfreq)
{
	return 0;
}

static struct opp *devfreq_recommended_opp(struct device *dev,
					   unsigned long *freq, u32 flags)
{
	return -EINVAL;
}

static int devfreq_register_opp_notifier(struct device *dev,
					 struct devfreq *devfreq)
{
	return -EINVAL;
}

static int devfreq_unregister_opp_notifier(struct device *dev,
					   struct devfreq *devfreq)
{
	return -EINVAL;
}

#define devfreq_powersave	NULL
#define devfreq_performance	NULL
#define devfreq_userspace	NULL
#define devfreq_simple_ondemand	NULL

#endif /*                   */

#endif /*                     */
