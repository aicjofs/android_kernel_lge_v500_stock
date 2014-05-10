/* drivers/usb/gadget/f_diag.c
 * Diag Function Device - Route ARM9 and ARM11 DIAG messages
 * between HOST and DEVICE.
 * Copyright (C) 2007 Google, Inc.
 * Copyright (c) 2008-2013, Linux Foundation. All rights reserved.
 * Author: Brian Swetland <swetland@google.com>
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
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/platform_device.h>
#include <linux/ratelimit.h>

#include <mach/usbdiag.h>

#include <linux/usb/composite.h>
#include <linux/usb/gadget.h>
#include <linux/workqueue.h>
#include <linux/debugfs.h>
#ifdef CONFIG_USB_G_LGE_ANDROID_DIAG_OSP_SUPPORT
#include "../../char/diag/diagchar.h"
#endif

static DEFINE_SPINLOCK(ch_lock);
static LIST_HEAD(usb_diag_ch_list);

static struct usb_interface_descriptor intf_desc = {
	.bLength            =	sizeof intf_desc,
	.bDescriptorType    =	USB_DT_INTERFACE,
	.bNumEndpoints      =	2,
	.bInterfaceClass    =	0xFF,
	.bInterfaceSubClass =	0xFF,
	.bInterfaceProtocol =	0xFF,
};

static struct usb_endpoint_descriptor hs_bulk_in_desc = {
	.bLength 			=	USB_DT_ENDPOINT_SIZE,
	.bDescriptorType 	=	USB_DT_ENDPOINT,
	.bEndpointAddress =	USB_DIR_IN,
	.bmAttributes 		=	USB_ENDPOINT_XFER_BULK,
	.wMaxPacketSize 	=	__constant_cpu_to_le16(512),
	.bInterval 			=	0,
};
static struct usb_endpoint_descriptor fs_bulk_in_desc = {
	.bLength          =	USB_DT_ENDPOINT_SIZE,
	.bDescriptorType  =	USB_DT_ENDPOINT,
	.bEndpointAddress =	USB_DIR_IN,
	.bmAttributes     =	USB_ENDPOINT_XFER_BULK,
	.wMaxPacketSize   = __constant_cpu_to_le16(64),
	.bInterval        =	0,
};

static struct usb_endpoint_descriptor hs_bulk_out_desc = {
	.bLength          =	USB_DT_ENDPOINT_SIZE,
	.bDescriptorType  =	USB_DT_ENDPOINT,
	.bEndpointAddress =	USB_DIR_OUT,
	.bmAttributes     =	USB_ENDPOINT_XFER_BULK,
	.wMaxPacketSize   = __constant_cpu_to_le16(512),
	.bInterval        =	0,
};

static struct usb_endpoint_descriptor fs_bulk_out_desc = {
	.bLength          =	USB_DT_ENDPOINT_SIZE,
	.bDescriptorType  =	USB_DT_ENDPOINT,
	.bEndpointAddress =	USB_DIR_OUT,
	.bmAttributes     =	USB_ENDPOINT_XFER_BULK,
	.wMaxPacketSize   = __constant_cpu_to_le16(64),
	.bInterval        =	0,
};

static struct usb_descriptor_header *fs_diag_desc[] = {
	(struct usb_descriptor_header *) &intf_desc,
	(struct usb_descriptor_header *) &fs_bulk_in_desc,
	(struct usb_descriptor_header *) &fs_bulk_out_desc,
	NULL,
	};
static struct usb_descriptor_header *hs_diag_desc[] = {
	(struct usb_descriptor_header *) &intf_desc,
	(struct usb_descriptor_header *) &hs_bulk_in_desc,
	(struct usb_descriptor_header *) &hs_bulk_out_desc,
	NULL,
};

/* 
                                                                   
                                                  
                                
                              
                                              
                                                
                                                    
                                                    
                                                          
                                     
                        
  
 */
struct diag_context {
	struct usb_function function;
	struct usb_ep *out;
	struct usb_ep *in;
	struct list_head read_pool;
	struct list_head write_pool;
	spinlock_t lock;
	unsigned configured;
	struct usb_composite_dev *cdev;
	int (*update_pid_and_serial_num)(uint32_t, const char *);
	struct usb_diag_ch ch;

	/*              */
	unsigned long dpkts_tolaptop;
	unsigned long dpkts_tomodem;
	unsigned dpkts_tolaptop_pending;
};

static inline struct diag_context *func_to_diag(struct usb_function *f)
{
	return container_of(f, struct diag_context, function);
}

static void diag_update_pid_and_serial_num(struct diag_context *ctxt)
{
	struct usb_composite_dev *cdev = ctxt->cdev;
	struct usb_gadget_strings *table;
	struct usb_string *s;

	if (!ctxt->update_pid_and_serial_num)
		return;

	/*                                               */
	if (!cdev->desc.iSerialNumber) {
		ctxt->update_pid_and_serial_num(
					cdev->desc.idProduct, 0);
		return;
	}

	/*
                                                       
                                                      
                                    
  */
	table = *(cdev->driver->strings);
	for (s = table->strings; s && s->s; s++)
		if (s->id == cdev->desc.iSerialNumber) {
			ctxt->update_pid_and_serial_num(
					cdev->desc.idProduct, s->s);
			break;
		}
}

static void diag_write_complete(struct usb_ep *ep,
		struct usb_request *req)
{
	struct diag_context *ctxt = ep->driver_data;
	struct diag_request *d_req = req->context;
	unsigned long flags;

	ctxt->dpkts_tolaptop_pending--;

	if (!req->status) {
		if ((req->length >= ep->maxpacket) &&
				((req->length % ep->maxpacket) == 0)) {
			ctxt->dpkts_tolaptop_pending++;
			req->length = 0;
			d_req->actual = req->actual;
			d_req->status = req->status;
			/*                          */
			usb_ep_queue(ctxt->in, req, GFP_ATOMIC);
			return;
		}
	}

	spin_lock_irqsave(&ctxt->lock, flags);
	list_add_tail(&req->list, &ctxt->write_pool);
	if (req->length != 0) {
		d_req->actual = req->actual;
		d_req->status = req->status;
	}
	spin_unlock_irqrestore(&ctxt->lock, flags);

	if (ctxt->ch.notify)
		ctxt->ch.notify(ctxt->ch.priv, USB_DIAG_WRITE_DONE, d_req);
}

#ifdef CONFIG_USB_G_LGE_ANDROID_DIAG_OSP_SUPPORT
#define DIAG_OSP_TYPE 0xf7
#endif
static void diag_read_complete(struct usb_ep *ep,
		struct usb_request *req)
{
	struct diag_context *ctxt = ep->driver_data;
	struct diag_request *d_req = req->context;
	unsigned long flags;

#ifdef CONFIG_USB_G_LGE_ANDROID_DIAG_OSP_SUPPORT
    if (!strcmp(ctxt->ch.name, DIAG_LEGACY)) {
        struct diagchar_dev *driver = ctxt->ch.priv;

        if (((unsigned char *)(d_req->buf))[0] == DIAG_OSP_TYPE) {
            driver->diag_read_status = 0;
        }
        else {
            driver->diag_read_status = 1;
        }
    }
#endif
	d_req->actual = req->actual;
	d_req->status = req->status;

	spin_lock_irqsave(&ctxt->lock, flags);
	list_add_tail(&req->list, &ctxt->read_pool);
	spin_unlock_irqrestore(&ctxt->lock, flags);

	ctxt->dpkts_tomodem++;

	if (ctxt->ch.notify)
		ctxt->ch.notify(ctxt->ch.priv, USB_DIAG_READ_DONE, d_req);
}

/* 
                                                 
                             
                                                                    
                                                      
  
                                                                  
                                                                         
                                                            
  
 */
struct usb_diag_ch *usb_diag_open(const char *name, void *priv,
		void (*notify)(void *, unsigned, struct diag_request *))
{
	struct usb_diag_ch *ch;
	struct diag_context *ctxt;
	unsigned long flags;
	int found = 0;

	spin_lock_irqsave(&ch_lock, flags);
	/*                                                   */
	list_for_each_entry(ch, &usb_diag_ch_list, list) {
		if (!strcmp(name, ch->name)) {
			found = 1;
			break;
		}
	}
	spin_unlock_irqrestore(&ch_lock, flags);

	if (!found) {
		ctxt = kzalloc(sizeof(*ctxt), GFP_KERNEL);
		if (!ctxt)
			return ERR_PTR(-ENOMEM);

		ch = &ctxt->ch;
	}

	ch->name = name;
	ch->priv = priv;
	ch->notify = notify;

	spin_lock_irqsave(&ch_lock, flags);
	list_add_tail(&ch->list, &usb_diag_ch_list);
	spin_unlock_irqrestore(&ch_lock, flags);

	return ch;
}
EXPORT_SYMBOL(usb_diag_open);

/* 
                                                   
                       
  
                                         
  
 */
void usb_diag_close(struct usb_diag_ch *ch)
{
	struct diag_context *dev = container_of(ch, struct diag_context, ch);
	unsigned long flags;

	spin_lock_irqsave(&ch_lock, flags);
	ch->priv = NULL;
	ch->notify = NULL;
	/*                                                    */
	if (!ch->priv_usb) {
		list_del(&ch->list);
		kfree(dev);
	}

	spin_unlock_irqrestore(&ch_lock, flags);
}
EXPORT_SYMBOL(usb_diag_close);

static void free_reqs(struct diag_context *ctxt)
{
	struct list_head *act, *tmp;
	struct usb_request *req;

	list_for_each_safe(act, tmp, &ctxt->write_pool) {
		req = list_entry(act, struct usb_request, list);
		list_del(&req->list);
		usb_ep_free_request(ctxt->in, req);
	}

	list_for_each_safe(act, tmp, &ctxt->read_pool) {
		req = list_entry(act, struct usb_request, list);
		list_del(&req->list);
		usb_ep_free_request(ctxt->out, req);
	}
}

/* 
                                          
                       
  
                                                                   
                                
  
 */
void usb_diag_free_req(struct usb_diag_ch *ch)
{
	struct diag_context *ctxt = ch->priv_usb;
	unsigned long flags;

	if (ctxt) {
		spin_lock_irqsave(&ctxt->lock, flags);
		free_reqs(ctxt);
		spin_unlock_irqrestore(&ctxt->lock, flags);
	}

}
EXPORT_SYMBOL(usb_diag_free_req);

/* 
                                               
                       
                                      
                                     
  
                                                                       
                                                                    
                                            
  
 */
int usb_diag_alloc_req(struct usb_diag_ch *ch, int n_write, int n_read)
{
	struct diag_context *ctxt = ch->priv_usb;
	struct usb_request *req;
	int i;
	unsigned long flags;

	if (!ctxt)
		return -ENODEV;

	spin_lock_irqsave(&ctxt->lock, flags);
	/*                                        */
	free_reqs(ctxt);
	for (i = 0; i < n_write; i++) {
		req = usb_ep_alloc_request(ctxt->in, GFP_ATOMIC);
		if (!req)
			goto fail;
		req->complete = diag_write_complete;
		list_add_tail(&req->list, &ctxt->write_pool);
	}

	for (i = 0; i < n_read; i++) {
		req = usb_ep_alloc_request(ctxt->out, GFP_ATOMIC);
		if (!req)
			goto fail;
		req->complete = diag_read_complete;
		list_add_tail(&req->list, &ctxt->read_pool);
	}
	spin_unlock_irqrestore(&ctxt->lock, flags);
	return 0;
fail:
	free_reqs(ctxt);
	spin_unlock_irqrestore(&ctxt->lock, flags);
	return -ENOMEM;

}
EXPORT_SYMBOL(usb_diag_alloc_req);

/* 
                                                    
                       
                              
  
                                                                           
                                                                         
                                                                        
  
                                                                           
                             
  
 */
int usb_diag_read(struct usb_diag_ch *ch, struct diag_request *d_req)
{
	struct diag_context *ctxt = ch->priv_usb;
	unsigned long flags;
	struct usb_request *req;
	static DEFINE_RATELIMIT_STATE(rl, 10*HZ, 1);

#ifdef CONFIG_USB_G_LGE_ANDROID_DIAG_OSP_SUPPORT
    if (!strcmp(ch->name, DIAG_LEGACY)) {
        struct diagchar_dev *driver = ch->priv;

        if(!driver)
            return -ENODEV;

        wait_event_interruptible_timeout(driver->diag_read_wait_q,
                driver->diag_read_status, 1*HZ);
    }
#endif

	if (!ctxt)
		return -ENODEV;

	spin_lock_irqsave(&ctxt->lock, flags);

	if (!ctxt->configured) {
		spin_unlock_irqrestore(&ctxt->lock, flags);
		return -EIO;
	}

	if (list_empty(&ctxt->read_pool)) {
		spin_unlock_irqrestore(&ctxt->lock, flags);
		ERROR(ctxt->cdev, "%s: no requests available\n", __func__);
		return -EAGAIN;
	}

	req = list_first_entry(&ctxt->read_pool, struct usb_request, list);
	list_del(&req->list);
	spin_unlock_irqrestore(&ctxt->lock, flags);

	req->buf = d_req->buf;
	req->length = d_req->length;
	req->context = d_req;
	if (usb_ep_queue(ctxt->out, req, GFP_ATOMIC)) {
		/*                                           */
		spin_lock_irqsave(&ctxt->lock, flags);
		list_add_tail(&req->list, &ctxt->read_pool);
		spin_unlock_irqrestore(&ctxt->lock, flags);
		/*                                  */
		if (__ratelimit(&rl))
			ERROR(ctxt->cdev, "%s: cannot queue"
				" read request\n", __func__);
		return -EIO;
	}

	return 0;
}
EXPORT_SYMBOL(usb_diag_read);

/* 
                                                      
                       
                              
  
                                                                          
                                                                         
                                                                        
  
                                                                            
                            
  
 */
int usb_diag_write(struct usb_diag_ch *ch, struct diag_request *d_req)
{
	struct diag_context *ctxt = ch->priv_usb;
	unsigned long flags;
	struct usb_request *req = NULL;
	static DEFINE_RATELIMIT_STATE(rl, 10*HZ, 1);

	if (!ctxt)
		return -ENODEV;

	spin_lock_irqsave(&ctxt->lock, flags);

	if (!ctxt->configured) {
		spin_unlock_irqrestore(&ctxt->lock, flags);
		return -EIO;
	}

	if (list_empty(&ctxt->write_pool)) {
		spin_unlock_irqrestore(&ctxt->lock, flags);
		ERROR(ctxt->cdev, "%s: no requests available\n", __func__);
		return -EAGAIN;
	}

	req = list_first_entry(&ctxt->write_pool, struct usb_request, list);
	list_del(&req->list);
	spin_unlock_irqrestore(&ctxt->lock, flags);

	req->buf = d_req->buf;
	req->length = d_req->length;
	req->context = d_req;
	if (usb_ep_queue(ctxt->in, req, GFP_ATOMIC)) {
		/*                                           */
		spin_lock_irqsave(&ctxt->lock, flags);
		list_add_tail(&req->list, &ctxt->write_pool);
		spin_unlock_irqrestore(&ctxt->lock, flags);
		/*                                  */
		if (__ratelimit(&rl))
			ERROR(ctxt->cdev, "%s: cannot queue"
				" read request\n", __func__);
		return -EIO;
	}

	ctxt->dpkts_tolaptop++;
	ctxt->dpkts_tolaptop_pending++;

	return 0;
}
EXPORT_SYMBOL(usb_diag_write);

static void diag_function_disable(struct usb_function *f)
{
	struct diag_context  *dev = func_to_diag(f);
	unsigned long flags;

	DBG(dev->cdev, "diag_function_disable\n");

	spin_lock_irqsave(&dev->lock, flags);
	dev->configured = 0;
	spin_unlock_irqrestore(&dev->lock, flags);

	if (dev->ch.notify)
		dev->ch.notify(dev->ch.priv, USB_DIAG_DISCONNECT, NULL);

	usb_ep_disable(dev->in);
	dev->in->driver_data = NULL;

	usb_ep_disable(dev->out);
	dev->out->driver_data = NULL;

}

static int diag_function_set_alt(struct usb_function *f,
		unsigned intf, unsigned alt)
{
	struct diag_context  *dev = func_to_diag(f);
	struct usb_composite_dev *cdev = f->config->cdev;
	unsigned long flags;
	int rc = 0;

	if (config_ep_by_speed(cdev->gadget, f, dev->in) ||
	    config_ep_by_speed(cdev->gadget, f, dev->out)) {
		dev->in->desc = NULL;
		dev->out->desc = NULL;
		return -EINVAL;
	}

	dev->in->driver_data = dev;
	rc = usb_ep_enable(dev->in);
	if (rc) {
		ERROR(dev->cdev, "can't enable %s, result %d\n",
						dev->in->name, rc);
		return rc;
	}
	dev->out->driver_data = dev;
	rc = usb_ep_enable(dev->out);
	if (rc) {
		ERROR(dev->cdev, "can't enable %s, result %d\n",
						dev->out->name, rc);
		usb_ep_disable(dev->in);
		return rc;
	}

	dev->dpkts_tolaptop = 0;
	dev->dpkts_tomodem = 0;
	dev->dpkts_tolaptop_pending = 0;

	spin_lock_irqsave(&dev->lock, flags);
	dev->configured = 1;
	spin_unlock_irqrestore(&dev->lock, flags);

	if (dev->ch.notify)
		dev->ch.notify(dev->ch.priv, USB_DIAG_CONNECT, NULL);

	return rc;
}

static void diag_function_unbind(struct usb_configuration *c,
		struct usb_function *f)
{
	struct diag_context *ctxt = func_to_diag(f);

	if (gadget_is_dualspeed(c->cdev->gadget))
		usb_free_descriptors(f->hs_descriptors);

	usb_free_descriptors(f->descriptors);
	ctxt->ch.priv_usb = NULL;
}

static int diag_function_bind(struct usb_configuration *c,
		struct usb_function *f)
{
	struct usb_composite_dev *cdev = c->cdev;
	struct diag_context *ctxt = func_to_diag(f);
	struct usb_ep *ep;
	int status = -ENODEV;

	intf_desc.bInterfaceNumber =  usb_interface_id(c, f);

	ep = usb_ep_autoconfig(cdev->gadget, &fs_bulk_in_desc);
	if (!ep)
		goto fail;
	ctxt->in = ep;
	ep->driver_data = ctxt;

	ep = usb_ep_autoconfig(cdev->gadget, &fs_bulk_out_desc);
	if (!ep)
		goto fail;
	ctxt->out = ep;
	ep->driver_data = ctxt;

	/*                                             */
	f->descriptors = usb_copy_descriptors(fs_diag_desc);
	if (!f->descriptors)
		goto fail;

	if (gadget_is_dualspeed(c->cdev->gadget)) {
		hs_bulk_in_desc.bEndpointAddress =
				fs_bulk_in_desc.bEndpointAddress;
		hs_bulk_out_desc.bEndpointAddress =
				fs_bulk_out_desc.bEndpointAddress;

		/*                                             */
		f->hs_descriptors = usb_copy_descriptors(hs_diag_desc);
	}
	diag_update_pid_and_serial_num(ctxt);
	return 0;
fail:
	if (ctxt->out)
		ctxt->out->driver_data = NULL;
	if (ctxt->in)
		ctxt->in->driver_data = NULL;
	return status;

}

int diag_function_add(struct usb_configuration *c, const char *name,
			int (*update_pid)(uint32_t, const char *))
{
	struct diag_context *dev;
	struct usb_diag_ch *_ch;
	int found = 0, ret;

	DBG(c->cdev, "diag_function_add\n");

	list_for_each_entry(_ch, &usb_diag_ch_list, list) {
		if (!strcmp(name, _ch->name)) {
			found = 1;
			break;
		}
	}
	if (!found) {
		ERROR(c->cdev, "unable to get diag usb channel\n");
		return -ENODEV;
	}

	dev = container_of(_ch, struct diag_context, ch);
	/*                                          */
	_ch->priv_usb = dev;

	dev->update_pid_and_serial_num = update_pid;
	dev->cdev = c->cdev;
	dev->function.name = _ch->name;
	dev->function.descriptors = fs_diag_desc;
	dev->function.hs_descriptors = hs_diag_desc;
	dev->function.bind = diag_function_bind;
	dev->function.unbind = diag_function_unbind;
	dev->function.set_alt = diag_function_set_alt;
	dev->function.disable = diag_function_disable;
	spin_lock_init(&dev->lock);
	INIT_LIST_HEAD(&dev->read_pool);
	INIT_LIST_HEAD(&dev->write_pool);

	ret = usb_add_function(c, &dev->function);
	if (ret) {
		INFO(c->cdev, "usb_add_function failed\n");
		_ch->priv_usb = NULL;
	}

	return ret;
}


#if defined(CONFIG_DEBUG_FS)
static char debug_buffer[PAGE_SIZE];

static ssize_t debug_read_stats(struct file *file, char __user *ubuf,
		size_t count, loff_t *ppos)
{
	char *buf = debug_buffer;
	int temp = 0;
	struct usb_diag_ch *ch;

	list_for_each_entry(ch, &usb_diag_ch_list, list) {
		struct diag_context *ctxt = ch->priv_usb;

		if (ctxt)
			temp += scnprintf(buf + temp, PAGE_SIZE - temp,
					"---Name: %s---\n"
					"endpoints: %s, %s\n"
					"dpkts_tolaptop: %lu\n"
					"dpkts_tomodem:  %lu\n"
					"pkts_tolaptop_pending: %u\n",
					ch->name,
					ctxt->in->name, ctxt->out->name,
					ctxt->dpkts_tolaptop,
					ctxt->dpkts_tomodem,
					ctxt->dpkts_tolaptop_pending);
	}

	return simple_read_from_buffer(ubuf, count, ppos, buf, temp);
}

static ssize_t debug_reset_stats(struct file *file, const char __user *buf,
				 size_t count, loff_t *ppos)
{
	struct usb_diag_ch *ch;

	list_for_each_entry(ch, &usb_diag_ch_list, list) {
		struct diag_context *ctxt = ch->priv_usb;

		if (ctxt) {
			ctxt->dpkts_tolaptop = 0;
			ctxt->dpkts_tomodem = 0;
			ctxt->dpkts_tolaptop_pending = 0;
		}
	}

	return count;
}

static int debug_open(struct inode *inode, struct file *file)
{
	return 0;
}

static const struct file_operations debug_fdiag_ops = {
	.open = debug_open,
	.read = debug_read_stats,
	.write = debug_reset_stats,
};

struct dentry *dent_diag;
static void fdiag_debugfs_init(void)
{
	dent_diag = debugfs_create_dir("usb_diag", 0);
	if (IS_ERR(dent_diag))
		return;

	debugfs_create_file("status", 0444, dent_diag, 0, &debug_fdiag_ops);
}
#else
static void fdiag_debugfs_init(void)
{
	return;
}
#endif

static void diag_cleanup(void)
{
	struct diag_context *dev;
	struct list_head *act, *tmp;
	struct usb_diag_ch *_ch;
	unsigned long flags;

	debugfs_remove_recursive(dent_diag);

	list_for_each_safe(act, tmp, &usb_diag_ch_list) {
		_ch = list_entry(act, struct usb_diag_ch, list);
		dev = container_of(_ch, struct diag_context, ch);

		spin_lock_irqsave(&ch_lock, flags);
		/*                                                   */
		if (!_ch->priv) {
			list_del(&_ch->list);
			kfree(dev);
		}
		spin_unlock_irqrestore(&ch_lock, flags);
	}
}

static int diag_setup(void)
{
	fdiag_debugfs_init();

	return 0;
}
