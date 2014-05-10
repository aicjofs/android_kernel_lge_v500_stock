/*
 * Copyright (C) 2009 Google, Inc.
 * Copyright (C) 2009 HTC Corporation.
 * Copyright (C) 2011 Broadcom Corporation.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include <linux/delay.h>
#include <linux/device.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/rfkill.h>
#include <asm/gpio.h>
#include <asm/mach-types.h>
#include <linux/regulator/consumer.h>
#include <linux/regulator/machine.h>

#include "board-L05E.h"

#if defined(CONFIG_BCM4335BT)
//                      
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/miscdevice.h>
//       
#endif //                          

#define LGPS3_GPIO_BT_RESET_N	PM8921_GPIO_PM_TO_SYS(30)

static struct rfkill *bt_rfk;
static const char bt_name[] = "brcm_Bluetooth_rfkill";

#if defined(CONFIG_BCM4335BT)
//                      
#define BTLOCK_NAME     "btlock"
#define BTLOCK_MINOR    224
/*                                    */
#define BTLOCK_TIMEOUT	2

#define PR(msg, ...) printk("####"msg, ##__VA_ARGS__)

struct btlock {
	int lock;
	int cookie;
};
static struct semaphore btlock;
static int count = 1;
static int owner_cookie = -1;

int bcm_bt_lock(int cookie)
{
	int ret;
	char cookie_msg[5] = {0};

	ret = down_timeout(&btlock, msecs_to_jiffies(BTLOCK_TIMEOUT*1000));
	if (ret == 0) {
		memcpy(cookie_msg, &cookie, sizeof(cookie));
		owner_cookie = cookie;
		count--;
		PR("btlock acquired cookie: %s\n", cookie_msg);
	}

	return ret;
}
EXPORT_SYMBOL(bcm_bt_lock);


void bcm_bt_unlock(int cookie)
{
	char owner_msg[5] = {0};
	char cookie_msg[5] = {0};

	memcpy(cookie_msg, &cookie, sizeof(cookie));
	if (owner_cookie == cookie) {
		owner_cookie = -1;
		if (count++ > 1)
			PR("error, release a lock that was not acquired**\n");
		up(&btlock);
		PR("btlock released, cookie: %s\n", cookie_msg);
	} else {
		memcpy(owner_msg, &owner_cookie, sizeof(owner_cookie));
		PR("ignore lock release,  cookie mismatch: %s owner %s \n", cookie_msg, 
				owner_cookie == 0 ? "NULL" : owner_msg);
	}
}
EXPORT_SYMBOL(bcm_bt_unlock);


static int btlock_open(struct inode *inode, struct file *file)
{
	return 0;
}

static int btlock_release(struct inode *inode, struct file *file)
{
	return 0;
}

static ssize_t btlock_write(struct file *file, const char __user *buffer, size_t count, loff_t *ppos)
{
	struct btlock lock_para;
	ssize_t ret = 0;

	if (count < sizeof(struct btlock))
		return -EINVAL;

	if (copy_from_user(&lock_para, buffer, sizeof(struct btlock))) {
		return -EFAULT;
	}

	if (lock_para.lock == 0) {
		bcm_bt_unlock(lock_para.cookie);	
	} else if (lock_para.lock == 1) {
		ret = bcm_bt_lock(lock_para.cookie);	
	} else if (lock_para.lock == 2) {
		ret = bcm_bt_lock(lock_para.cookie);	
	}

	return ret;
}

static long btlock_unlocked_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	return 0;
}

static const struct file_operations btlock_fops = {
	.owner   = THIS_MODULE,
	.open    = btlock_open,
	.release = btlock_release,
	.write   = btlock_write,
	.unlocked_ioctl = btlock_unlocked_ioctl,
};

static struct miscdevice btlock_misc = {
	.name  = BTLOCK_NAME,
	.minor = BTLOCK_MINOR,
	.fops  = &btlock_fops,
};

static int bcm_btlock_init(void)
{
	int ret;

	PR("init\n");

	ret = misc_register(&btlock_misc);
	if (ret != 0) {
		PR("Error: failed to register Misc driver,  ret = %d\n", ret);
		return ret;
	}
	sema_init(&btlock, 1);

	return ret;
}

static void bcm_btlock_exit(void)
{
	PR("btlock_exit:\n");

	misc_deregister(&btlock_misc);
}

//       
#endif //                           
static int bluetooth_set_power(void *data, bool blocked)
{
#if defined(CONFIG_BCM4335BT)
//                      
    int lock_cookie_bt = 'B' | 'T'<<8 | '3'<<16 | '5'<<24;	/*                  */
//       
#endif //                           

	printk(KERN_ERR "bluetooth_set_power set blocked=%d",blocked);
	if (!blocked) {

#if defined(CONFIG_BCM4335BT)
//                      
        if (bcm_bt_lock(lock_cookie_bt) != 0)
        	printk("** BT rfkill: timeout in acquiring bt lock**\n");
//       
#endif //                           

		printk(KERN_ERR "bluetooth_set_power set BT On");
	    gpio_direction_output(LGPS3_GPIO_BT_RESET_N, 0);
		msleep(30);
	    gpio_direction_output(LGPS3_GPIO_BT_RESET_N, 1);
	} else {
		printk(KERN_ERR "bluetooth_set_power set BT Off");
	    gpio_direction_output(LGPS3_GPIO_BT_RESET_N, 0);
	}
	return 0;
}

static struct rfkill_ops lgps3_rfkill_ops = {
	.set_block = bluetooth_set_power,
};

static int lgps3_rfkill_probe(struct platform_device *pdev)
{
	int rc = 0;
	bool default_state = true;  /*     */

#if defined(CONFIG_BCM4335BT)
//                      
    bcm_btlock_init();
//       
#endif //                           

      printk(KERN_ERR"lgps3_rfkill_probe\n");
	rc = gpio_request(LGPS3_GPIO_BT_RESET_N, "bt_reset");

	if (rc)
	{
	    printk(KERN_ERR "GPIO req error no=%d",rc);
	    gpio_free(LGPS3_GPIO_BT_RESET_N);
	    rc = gpio_request(LGPS3_GPIO_BT_RESET_N, "bt_reset");
	    if(rc)
	    {
	    printk(KERN_ERR "GPIO req error no=%d",rc);
		goto err_gpio_reset;
           }
	}
      gpio_direction_output(LGPS3_GPIO_BT_RESET_N, 0);

	bluetooth_set_power(NULL, default_state);

	bt_rfk = rfkill_alloc(bt_name, &pdev->dev, RFKILL_TYPE_BLUETOOTH,
				&lgps3_rfkill_ops, NULL);
	if (!bt_rfk) {
    	printk(KERN_ERR"rfkill alloc failed.\n");
		rc = -ENOMEM;
		goto err_rfkill_alloc;
	}

	rfkill_set_states(bt_rfk, default_state, false);

	/*                                         */

	rc = rfkill_register(bt_rfk);
	if (rc)
		goto err_rfkill_reg;

	return 0;


err_rfkill_reg:
	rfkill_destroy(bt_rfk);
err_rfkill_alloc:
err_gpio_reset:
	gpio_free(LGPS3_GPIO_BT_RESET_N);
	printk(KERN_ERR"lgps3_rfkill_probe error!\n");
	return rc;
}

static int lgps3_rfkill_remove(struct platform_device *dev)
{
	rfkill_unregister(bt_rfk);
	rfkill_destroy(bt_rfk);
	gpio_free(LGPS3_GPIO_BT_RESET_N);

#if defined(CONFIG_BCM4335BT)
//                      
    bcm_btlock_exit();
//       
#endif //                           
	return 0;
}
struct lgps3_rfkill_platform_data {
        int gpio_reset;
};

static struct lgps3_rfkill_platform_data lgps3_rfkill_platform;

static struct platform_device lgps3_rfkill_device = {
   .name = "lgps3_rfkill",
   .id   = -1,
   .dev = {
      .platform_data = &lgps3_rfkill_platform,
   },
};

static struct platform_driver lgps3_rfkill_driver = {
	.probe = lgps3_rfkill_probe,
	.remove = lgps3_rfkill_remove,
	.driver = {
		.name = "lgps3_rfkill",
		.owner = THIS_MODULE,
	},
};

static int __init lgps3_rfkill_init(void)
{
    int ret;
    printk(KERN_ERR"lgps3_rfkill_init\n");
	ret = platform_driver_register(&lgps3_rfkill_driver);
	if (ret) {
		printk(KERN_ERR"Fail to register rfkill platform driver\n");
	}
	printk(KERN_ERR"lgps3_rfkill_init done\n");

	return platform_device_register(&lgps3_rfkill_device);;


}

static void __exit lgps3_rfkill_exit(void)
{
    platform_device_unregister(&lgps3_rfkill_device);

	platform_driver_unregister(&lgps3_rfkill_driver);
}

//                                      
device_initcall(lgps3_rfkill_init);
module_exit(lgps3_rfkill_exit);

MODULE_DESCRIPTION("lgps3 rfkill");
MODULE_AUTHOR("Nick Pelly <npelly@google.com>");
MODULE_LICENSE("GPL");
