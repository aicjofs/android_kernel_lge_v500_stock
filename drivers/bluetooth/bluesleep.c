/*

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2 as
   published by the Free Software Foundation.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

   This program is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
   or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
   for more details.


   Copyright (C) 2006-2007 - Motorola
   Copyright (c) 2008-2010, The Linux Foundation. All rights reserved.

   Date         Author           Comment
   -----------  --------------   --------------------------------
   2006-Apr-28	Motorola	 The kernel module for running the Bluetooth(R)
				 Sleep-Mode Protocol from the Host side
   2006-Sep-08  Motorola         Added workqueue for handling sleep work.
   2007-Jan-24  Motorola         Added mbm_handle_ioi() call to ISR.

*/

#define DEBUG


#include <linux/module.h>	/*                           */
#include <linux/errno.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/kernel.h>
#include <linux/notifier.h>
#include <linux/proc_fs.h>
#include <linux/spinlock.h>
#include <linux/timer.h>
#include <linux/uaccess.h>
#include <linux/version.h>
#include <linux/workqueue.h>
#include <linux/platform_device.h>

#include <linux/irq.h>
#include <linux/param.h>
#include <linux/bitops.h>
#include <linux/termios.h>
//                                                    
#ifdef CONFIG_LGE_BLUESLEEP
#include <linux/wakelock.h>
#endif/*                    */
//                                                    

#include <mach/gpio.h>
#include <mach/msm_serial_hs.h>

#include <net/bluetooth/bluetooth.h>
#include <net/bluetooth/hci_core.h> /*                     */
#include "hci_uart.h"

#include <linux/gpio.h>

//                                                    
#ifdef CONFIG_LGE_BLUESLEEP
#define BT_PORT_NUM	0

#define BTA_NOT_USE_ROOT_PERM

#ifdef BTA_NOT_USE_ROOT_PERM
#define AID_BLUETOOTH       1002  /*                     */
#define AID_NET_BT_STACK  	3008  /*                                */
#endif  //                     
#endif/*                    */
//                                                    


#define BT_SLEEP_DBG
#ifndef BT_SLEEP_DBG
#define BT_DBG(fmt, arg...)
#endif
/*
          
 */

#define VERSION		"1.1"
#define PROC_DIR	"bluetooth/sleep"


struct bluesleep_info {
	unsigned host_wake;
	unsigned ext_wake;
	unsigned host_wake_irq;
	struct uart_port *uport;
//                                                    
#ifdef CONFIG_LGE_BLUESLEEP
	struct wake_lock wake_lock;
#endif/*                */
//                                                    
};

/*               */
static void bluesleep_sleep_work(struct work_struct *work);

/*            */
DECLARE_DELAYED_WORK(sleep_workqueue, bluesleep_sleep_work);

/*                                */
#define bluesleep_rx_busy()     schedule_delayed_work(&sleep_workqueue, 0)
#define bluesleep_tx_busy()     schedule_delayed_work(&sleep_workqueue, 0)
#define bluesleep_rx_idle()     schedule_delayed_work(&sleep_workqueue, 0)
#define bluesleep_tx_idle()     schedule_delayed_work(&sleep_workqueue, 0)

//                                                    
#ifdef CONFIG_LGE_BLUESLEEP
//                                                                            
/*                  */
#define TX_TIMER_INTERVAL	5
#else/*                    */
#define TX_TIMER_INTERVAL	1
#endif/*                    */
//                                                    


/*                                        */
#define BT_PROTO	0x01
#define BT_TXDATA	0x02
#define BT_ASLEEP	0x04

/*                                        */
static struct hci_dev *bluesleep_hdev;

static struct bluesleep_info *bsi;

/*              */
static atomic_t open_count = ATOMIC_INIT(1);

//                                                    
/*
                            
 */
#ifndef CONFIG_LGE_BLUESLEEP
static int bluesleep_hci_event(struct notifier_block *this,
			    unsigned long event, void *data);
#endif/*                    */				
//                                                    

/*
                   
 */

/*                     */
static unsigned long flags;

/*                                                */
static struct tasklet_struct hostwake_task;

/*                     */
static struct timer_list tx_timer;

/*                             */
static spinlock_t rw_lock;

//                                                    
#ifndef CONFIG_LGE_BLUESLEEP
/*                                */
struct notifier_block hci_event_nblock = {
	.notifier_call = bluesleep_hci_event,
};
#endif/*                    */
//                                                    


struct proc_dir_entry *bluetooth_dir, *sleep_dir;

/*
                  
 */

static void hsuart_power(int on)
{	
	if (on) {
		BT_DBG("hsuart_power : 1");
		msm_hs_request_clock_on(bsi->uport);
		msm_hs_set_mctrl(bsi->uport, TIOCM_RTS);
	} else {
		BT_DBG("hsuart_power : 0");
		msm_hs_set_mctrl(bsi->uport, 0);
		msm_hs_request_clock_off(bsi->uport);
	}
}


/* 
                                                      
 */
static inline int bluesleep_can_sleep(void)
{
	/*                                                                    */
	return gpio_get_value(bsi->ext_wake) &&
		gpio_get_value(bsi->host_wake) &&
		(bsi->uport != NULL);
}

void bluesleep_sleep_wakeup(void)
{
	if (test_bit(BT_ASLEEP, &flags)) {
		BT_DBG("waking up...");
//                                                    
#ifdef CONFIG_LGE_BLUESLEEP
		wake_lock(&bsi->wake_lock);		
#else/*                    */
		/*                 */
		mod_timer(&tx_timer, jiffies + (TX_TIMER_INTERVAL * HZ));
		gpio_set_value(bsi->ext_wake, 0);
#endif/*                    */	
//                                                    

		clear_bit(BT_ASLEEP, &flags);		
		/*                */
		hsuart_power(1);
	}
//                                                    
#ifdef CONFIG_LGE_BLUESLEEP
	else
	{
		int wake, host_wake;
		wake = gpio_get_value(bsi->ext_wake);
		host_wake = gpio_get_value(bsi->host_wake);

		printk(KERN_DEBUG "bluesleep_sleep_wakeup %d, %d", wake, host_wake);
	
		if(wake == 1 && host_wake == 0)
		{
			BT_DBG("Start Timer : check hostwake status when timer expired");
			mod_timer(&tx_timer, jiffies + (TX_TIMER_INTERVAL * HZ));
		}
	
		BT_DBG("Workaround for uart runtime suspend : Check UART Satus");
		
		if(bsi->uport != NULL && msm_hs_get_bt_uport_clock_state(bsi->uport) == CLOCK_REQUEST_AVAILABLE)
		{			
			BT_DBG("[LG_BTUI] Enter abnormal status, HAVE to Call hsuart_power(1)!!!!");		
			hsuart_power(1);						
		}
	}
#endif/*                    */
//                                                    
}

/* 
                                                                    
                                                
 */
static void bluesleep_sleep_work(struct work_struct *work)
{
	BT_DBG("+++ bluesleep_sleep_work");

	if (bluesleep_can_sleep()) {
		/*                                       */
		BT_DBG("bluesleep_can_sleep is true");
		if (test_bit(BT_ASLEEP, &flags)) {
			BT_DBG("already asleep");
			return;
		}
//                                                    
#ifndef CONFIG_LGE_BLUESLEEP
		if (msm_hs_tx_empty(bsi->uport)) {
#endif/*                    */			
//                                                    
			BT_DBG("going to sleep...");
			set_bit(BT_ASLEEP, &flags);
			/*                  */
			hsuart_power(0);

//                                                     
#ifdef CONFIG_LGE_BLUESLEEP
			wake_lock_timeout(&bsi->wake_lock, HZ / 2);
#endif/*                    */
//                                                    
#ifndef CONFIG_LGE_BLUESLEEP
		}
#endif/*                    */		

//                                                      
#ifndef CONFIG_LGE_BLUESLEEP		 
		else {
			//                                      
		  	mod_timer(&tx_timer, jiffies + (TX_TIMER_INTERVAL * HZ));
			return;
		}
#endif/*                    */		
//                                                    
	} else {
		BT_DBG("Call BT Wake Up");
		bluesleep_sleep_wakeup();
	}

	BT_DBG("--- bluesleep_sleep_work");
}

/* 
                                                                      
                                                        
                        
 */
static void bluesleep_hostwake_task(unsigned long data)
{
	BT_DBG("hostwake line change");

	spin_lock(&rw_lock);

//                                                    
#ifdef CONFIG_LGE_BLUESLEEP
	if(gpio_get_value(bsi->host_wake) == 0)
	{
		BT_DBG("hostwake GPIO Low");
		//                          
		bluesleep_rx_busy();
		//                                                                             
		mod_timer(&tx_timer, jiffies + (TX_TIMER_INTERVAL * HZ));		
	}
	else
	{
		BT_DBG("hostwake GPIO High");
	}
#else/*                    */
	if (gpio_get_value(bsi->host_wake))
		bluesleep_rx_busy();
	else
		bluesleep_rx_idle();
#endif/*                    */
//                                                    

	spin_unlock(&rw_lock);
}

//                                                    
#ifndef CONFIG_LGE_BLUESLEEP
/* 
                                                                     
                                       
 */
static void bluesleep_outgoing_data(void)
{
	unsigned long irq_flags;

	spin_lock_irqsave(&rw_lock, irq_flags);

	/*                     */
	set_bit(BT_TXDATA, &flags);

	/*                               */
	if (gpio_get_value(bsi->ext_wake)) {

		BT_DBG("tx was sleeping");
		bluesleep_sleep_wakeup();
	}

	spin_unlock_irqrestore(&rw_lock, irq_flags);
}

/* 
                             
                        
                                        
                                                        
                                    
 */
static int bluesleep_hci_event(struct notifier_block *this,
				unsigned long event, void *data)
{
	struct hci_dev *hdev = (struct hci_dev *) data;
	struct hci_uart *hu;
	struct uart_state *state;

	if (!hdev)
		return NOTIFY_DONE;

	switch (event) {
	case HCI_DEV_REG:
		if (!bluesleep_hdev) {
			bluesleep_hdev = hdev;
			hu  = (struct hci_uart *) hdev->driver_data;
			state = (struct uart_state *) hu->tty->driver_data;
			bsi->uport = state->uart_port;
		}
		break;
	case HCI_DEV_UNREG:
		bluesleep_hdev = NULL;
		bsi->uport = NULL;
		break;
	case HCI_DEV_WRITE:
		bluesleep_outgoing_data();
		break;
	}

	return NOTIFY_DONE;
}
#endif/*                    */
//                                                    

/* 
                                         
                        
 */
static void bluesleep_tx_timer_expire(unsigned long data)
{
	unsigned long irq_flags;

	spin_lock_irqsave(&rw_lock, irq_flags);

	BT_DBG("Tx timer expired");

//                                                     
#ifdef CONFIG_LGE_BLUESLEEP
	/*                                       */
	if (test_bit(BT_ASLEEP, &flags)) {
		BT_DBG("already asleep");
		spin_unlock_irqrestore(&rw_lock, irq_flags);
		return;
	}

//                                   
//                          
//                                              
//         
//   

	bluesleep_tx_idle();	
#else/*                    */
	/*                                         */
	if (!test_bit(BT_TXDATA, &flags)) {
		BT_DBG("Tx has been idle");
		gpio_set_value(bsi->ext_wake, 1);
		bluesleep_tx_idle();
	} else {
		BT_DBG("Tx data during last period");
		mod_timer(&tx_timer, jiffies + (TX_TIMER_INTERVAL*HZ));
	}

	/*                              */
	clear_bit(BT_TXDATA, &flags);
#endif/*                    */
//                                                    

	spin_unlock_irqrestore(&rw_lock, irq_flags);
}

/* 
                                                                
                                   
                       
                          
 */
static irqreturn_t bluesleep_hostwake_isr(int irq, void *dev_id)
{
//                                             
//                                                         
#ifdef CONFIG_LGE_BLUESLEEP
	/*                                                               */
	int wake, host_wake;
	wake = gpio_get_value(bsi->ext_wake);
	host_wake = gpio_get_value(bsi->host_wake);
	printk(KERN_DEBUG "bluesleep_hostwake_isr %d, %d", wake, host_wake);

	irq_set_irq_type(irq, host_wake ? IRQF_TRIGGER_LOW : IRQF_TRIGGER_HIGH);

	if(host_wake == 0)
	{
		BT_DBG("bluesleep_hostwake_isr : Registration Tasklet");
		tasklet_schedule(&hostwake_task);
	}
#else/*                    */
	tasklet_schedule(&hostwake_task);
#endif/*                    */	
//                                           
	
	return IRQ_HANDLED;
}

/* 
                                              
                                                                     
                 
 */
static int bluesleep_start(void)
{
	int retval;
	unsigned long irq_flags;
	
	BT_DBG("bluesleep_start");

	spin_lock_irqsave(&rw_lock, irq_flags);

	if (test_bit(BT_PROTO, &flags)) {
		spin_unlock_irqrestore(&rw_lock, irq_flags);
		return 0;
	}

	spin_unlock_irqrestore(&rw_lock, irq_flags);

	if (!atomic_dec_and_test(&open_count)) {
		atomic_inc(&open_count);
		return -EBUSY;
	}

//                                             
//                                                          
#ifndef CONFIG_LGE_BLUESLEEP
	/*                 */

	mod_timer(&tx_timer, jiffies + (TX_TIMER_INTERVAL*HZ));
	/*                */
	gpio_set_value(bsi->ext_wake, 0);	
#endif/*                    */	
//                                            
	

//                                             
//                                                           
#ifdef CONFIG_LGE_BLUESLEEP
	BT_DBG("bluesleep_start");
	hsuart_power(1);
#endif/*                    */
//                                           

//                                             
//                                                         
#ifdef CONFIG_LGE_BLUESLEEP
	retval = request_irq(bsi->host_wake_irq, bluesleep_hostwake_isr,
				IRQF_DISABLED | IRQF_TRIGGER_LOW,
				"bluetooth hostwake", NULL);
#else/*                    */
	retval = request_irq(bsi->host_wake_irq, bluesleep_hostwake_isr,
				IRQF_DISABLED | IRQF_TRIGGER_FALLING,
				"bluetooth hostwake", NULL);
#endif/*                    */	
//                                           
				
	if (retval  < 0) {
		BT_ERR("Couldn't acquire BT_HOST_WAKE IRQ");
		goto fail;
	}

	retval = enable_irq_wake(bsi->host_wake_irq);
	if (retval < 0) {
		BT_ERR("Couldn't enable BT_HOST_WAKE as wakeup interrupt");
		free_irq(bsi->host_wake_irq, NULL);
		goto fail;
	}

	set_bit(BT_PROTO, &flags);
//                                             
//                                                           
#ifdef CONFIG_LGE_BLUESLEEP
	wake_lock(&bsi->wake_lock);
#endif/*                    */
//                                           
	return 0;
fail:
//                                             
//                                                         
#ifndef CONFIG_LGE_BLUESLEEP
	del_timer(&tx_timer);
#endif/*                    */	
//                                           
	atomic_inc(&open_count);

	return retval;
}

/* 
                                             
 */
static void bluesleep_stop(void)
{
	unsigned long irq_flags;

	spin_lock_irqsave(&rw_lock, irq_flags);

	if (!test_bit(BT_PROTO, &flags)) {
		spin_unlock_irqrestore(&rw_lock, irq_flags);
		return;
	}

	BT_DBG("bluesleep_stop");	


//                                                    
#ifdef CONFIG_LGE_BLUESLEEP
	del_timer(&tx_timer);
#else/*                    */
	/*                */
	gpio_set_value(bsi->ext_wake, 0);
	del_timer(&tx_timer);
#endif/*                    */
//                                                    


	clear_bit(BT_PROTO, &flags);

	if (test_bit(BT_ASLEEP, &flags)) {
		clear_bit(BT_ASLEEP, &flags);
//                                             
//                                                           
#ifndef CONFIG_LGE_BLUESLEEP
		hsuart_power(1);
#endif/*                    */
//                                           
	}
//                                             
//                                                           
#ifdef CONFIG_LGE_BLUESLEEP	
	else
	{
		//                           
		hsuart_power(0);
	}
#endif/*                    */
//                                           

	atomic_inc(&open_count);

	spin_unlock_irqrestore(&rw_lock, irq_flags);
	if (disable_irq_wake(bsi->host_wake_irq))
		BT_ERR("Couldn't disable hostwake IRQ wakeup mode\n");
	free_irq(bsi->host_wake_irq, NULL);
//                                             
//                                                           
#ifdef CONFIG_LGE_BLUESLEEP		
	wake_lock_timeout(&bsi->wake_lock, HZ / 2);
#endif/*                    */
//                                            
}
/* 
                                                                       
                                                                        
                            
                                       
                         
                          
                         
                                                           
                        
                                       
 */
static int bluepower_read_proc_btwake(char *page, char **start, off_t offset,
					int count, int *eof, void *data)
{
	*eof = 1;
	return sprintf(page, "btwake:%u\n", gpio_get_value(bsi->ext_wake));
}

/* 
                                                                        
                        
                                         
                                                  
                        
                                                                     
                                           
 */
static int bluepower_write_proc_btwake(struct file *file, const char *buffer,
					unsigned long count, void *data)
{
	char *buf;

	if (count < 1)
		return -EINVAL;

	buf = kmalloc(count, GFP_KERNEL);
	if (!buf)
		return -ENOMEM;

	if (copy_from_user(buf, buffer, count)) {
		kfree(buf);
		return -EFAULT;
	}

	if (buf[0] == '0') {
		BT_DBG("BT WAKE Set to Wake");
	
		gpio_set_value(bsi->ext_wake, 0);
//                                             
//                                                          
#ifdef CONFIG_LGE_BLUESLEEP		
		//                           
		bluesleep_sleep_wakeup();
#endif/*                    */
//                                             
	} else if (buf[0] == '1') {
		BT_DBG("BT WAKE Set to Sleep");
		gpio_set_value(bsi->ext_wake, 1);
//                                             
//                                                          
#ifdef CONFIG_LGE_BLUESLEEP			
		bluesleep_tx_idle();
#endif/*                    */
//                                            
	} else {
		kfree(buf);
		return -EINVAL;
	}

	kfree(buf);
	return count;
}

/* 
                                                                            
                                                                            
                        
                                       
                         
                          
                         
                                                           
                        
                                       
 */
static int bluepower_read_proc_hostwake(char *page, char **start, off_t offset,
					int count, int *eof, void *data)
{
	*eof = 1;
	return sprintf(page, "hostwake: %u \n", gpio_get_value(bsi->host_wake));
}


/* 
                                                                
                                                                         
                          
                                       
                         
                          
                         
                                                           
                        
                                       
 */
static int bluesleep_read_proc_asleep(char *page, char **start, off_t offset,
					int count, int *eof, void *data)
{
	unsigned int asleep;

	asleep = test_bit(BT_ASLEEP, &flags) ? 1 : 0;
	*eof = 1;
	return sprintf(page, "asleep: %u\n", asleep);
}

/* 
                                                                             
                                                                             
                                                 
                                       
                         
                          
                         
                                                           
                        
                                       
 */
static int bluesleep_read_proc_proto(char *page, char **start, off_t offset,
					int count, int *eof, void *data)
{
	unsigned int proto;

	proto = test_bit(BT_PROTO, &flags) ? 1 : 0;
	*eof = 1;
	return sprintf(page, "proto: %u\n", proto);
}

/* 
                                                                         
                        
                                         
                                                  
                        
                                                                     
                                           
 */
static int bluesleep_write_proc_proto(struct file *file, const char *buffer,
					unsigned long count, void *data)
{
	char proto;

	if (count < 1)
		return -EINVAL;

	if (copy_from_user(&proto, buffer, 1))
		return -EFAULT;

	if (proto == '0')
		bluesleep_stop();
	else
		bluesleep_start();

	/*                                */
	return count;
}

static int __init bluesleep_probe(struct platform_device *pdev)
{
	int ret;
	struct resource *res;

	bsi = kzalloc(sizeof(struct bluesleep_info), GFP_KERNEL);
	if (!bsi)
		return -ENOMEM;

	res = platform_get_resource_byname(pdev, IORESOURCE_IO,
				"gpio_host_wake");
	if (!res) {
		BT_ERR("couldn't find host_wake gpio\n");
		ret = -ENODEV;
		goto free_bsi;
	}
	bsi->host_wake = res->start;

	ret = gpio_request(bsi->host_wake, "bt_host_wake");
	if (ret)
		goto free_bsi;
	ret = gpio_direction_input(bsi->host_wake);
	if (ret)
		goto free_bt_host_wake;

	res = platform_get_resource_byname(pdev, IORESOURCE_IO,
				"gpio_ext_wake");
	if (!res) {
		BT_ERR("couldn't find ext_wake gpio\n");
		ret = -ENODEV;
		goto free_bt_host_wake;
	}
	bsi->ext_wake = res->start;

	ret = gpio_request(bsi->ext_wake, "bt_ext_wake");
	if (ret)
		goto free_bt_host_wake;
	/*                */
	ret = gpio_direction_output(bsi->ext_wake, 0);
	if (ret)
		goto free_bt_ext_wake;

	bsi->host_wake_irq = platform_get_irq_byname(pdev, "host_wake");
	if (bsi->host_wake_irq < 0) {
		BT_ERR("couldn't find host_wake irq\n");
		ret = -ENODEV;
		goto free_bt_ext_wake;
	}

//                                             
//                                                         
#ifdef CONFIG_LGE_BLUESLEEP
	bsi->uport= msm_hs_get_bt_uport(BT_PORT_NUM);
#endif/*                             */
//                                           

//                                             
//                                                          
#ifdef CONFIG_LGE_BLUESLEEP	
	wake_lock_init(&bsi->wake_lock, WAKE_LOCK_SUSPEND, "bluesleep");
#endif/*                    */
//                                            

	return 0;

free_bt_ext_wake:
	gpio_free(bsi->ext_wake);
free_bt_host_wake:
	gpio_free(bsi->host_wake);
free_bsi:
	kfree(bsi);
	return ret;
}

static int bluesleep_remove(struct platform_device *pdev)
{
	/*                */
	gpio_set_value(bsi->ext_wake, 0);
	if (test_bit(BT_PROTO, &flags)) {
		if (disable_irq_wake(bsi->host_wake_irq))
			BT_ERR("Couldn't disable hostwake IRQ wakeup mode \n");
		free_irq(bsi->host_wake_irq, NULL);
		del_timer(&tx_timer);
		if (test_bit(BT_ASLEEP, &flags))
			hsuart_power(1);
	}

	gpio_free(bsi->host_wake);
	gpio_free(bsi->ext_wake);
//                                             
//                                                         
#ifdef CONFIG_LGE_BLUESLEEP	
	wake_lock_destroy(&bsi->wake_lock);
#endif/*                    */
//                                           
	kfree(bsi);
	return 0;
}

static struct platform_driver bluesleep_driver = {
	.remove = bluesleep_remove,
	.driver = {
		.name = "bluesleep",
		.owner = THIS_MODULE,
	},
};
/* 
                          
                                                                     
                 
 */
static int __init bluesleep_init(void)
{
	int retval;
	struct proc_dir_entry *ent;

	BT_INFO("MSM Sleep Mode Driver Ver %s", VERSION);

	retval = platform_driver_probe(&bluesleep_driver, bluesleep_probe);
	if (retval)
		return retval;

	bluesleep_hdev = NULL;

	bluetooth_dir = proc_mkdir("bluetooth", NULL);
	if (bluetooth_dir == NULL) {
		BT_ERR("Unable to create /proc/bluetooth directory");
		return -ENOMEM;
	}

	sleep_dir = proc_mkdir("sleep", bluetooth_dir);
	if (sleep_dir == NULL) {
		BT_ERR("Unable to create /proc/%s directory", PROC_DIR);
		return -ENOMEM;
	}

	/*                                    */
	ent = create_proc_entry("btwake", 0, sleep_dir);
	if (ent == NULL) {
		BT_ERR("Unable to create /proc/%s/btwake entry", PROC_DIR);
		retval = -ENOMEM;
		goto fail;
	}
	ent->read_proc = bluepower_read_proc_btwake;
	ent->write_proc = bluepower_write_proc_btwake;
//                                             
//                                                         
#ifdef BTA_NOT_USE_ROOT_PERM
    ent->uid = AID_BLUETOOTH;
    ent->gid = AID_NET_BT_STACK;
#endif  //                     
//                                           

	/*                        */
	if (create_proc_read_entry("hostwake", 0, sleep_dir,
				bluepower_read_proc_hostwake, NULL) == NULL) {
		BT_ERR("Unable to create /proc/%s/hostwake entry", PROC_DIR);
		retval = -ENOMEM;
		goto fail;
	}

	/*                         */
	ent = create_proc_entry("proto", 0, sleep_dir);
	if (ent == NULL) {
		BT_ERR("Unable to create /proc/%s/proto entry", PROC_DIR);
		retval = -ENOMEM;
		goto fail;
	}
	ent->read_proc = bluesleep_read_proc_proto;
	ent->write_proc = bluesleep_write_proc_proto;
//                                             
//                                                         
#ifdef BTA_NOT_USE_ROOT_PERM
    ent->uid = AID_BLUETOOTH;
    ent->gid = AID_NET_BT_STACK;
#endif  //                     
//                                           

	/*                        */
	if (create_proc_read_entry("asleep", 0,
			sleep_dir, bluesleep_read_proc_asleep, NULL) == NULL) {
		BT_ERR("Unable to create /proc/%s/asleep entry", PROC_DIR);
		retval = -ENOMEM;
		goto fail;
	}

	flags = 0; /*                       */

	/*                      */
	spin_lock_init(&rw_lock);

	/*                  */
	init_timer(&tx_timer);
	tx_timer.function = bluesleep_tx_timer_expire;
	tx_timer.data = 0;

	/*                              */
	tasklet_init(&hostwake_task, bluesleep_hostwake_task, 0);

//                                             
//                                                         
#ifndef CONFIG_LGE_BLUESLEEP
	hci_register_notifier(&hci_event_nblock);
#endif/*                    */
//                                            

	return 0;

fail:
	remove_proc_entry("asleep", sleep_dir);
	remove_proc_entry("proto", sleep_dir);
	remove_proc_entry("hostwake", sleep_dir);
	remove_proc_entry("btwake", sleep_dir);
	remove_proc_entry("sleep", bluetooth_dir);
	remove_proc_entry("bluetooth", 0);
	return retval;
}

/* 
                        
 */
static void __exit bluesleep_exit(void)
{
//                                             
//                                                         
#ifndef CONFIG_LGE_BLUESLEEP
	hci_unregister_notifier(&hci_event_nblock);
#endif/*                    */
//                                             
	platform_driver_unregister(&bluesleep_driver);

	remove_proc_entry("asleep", sleep_dir);
	remove_proc_entry("proto", sleep_dir);
	remove_proc_entry("hostwake", sleep_dir);
	remove_proc_entry("btwake", sleep_dir);
	remove_proc_entry("sleep", bluetooth_dir);
	remove_proc_entry("bluetooth", 0);
}

module_init(bluesleep_init);
module_exit(bluesleep_exit);

MODULE_DESCRIPTION("Bluetooth Sleep Mode Driver ver %s " VERSION);
#ifdef MODULE_LICENSE
MODULE_LICENSE("GPL");
#endif
