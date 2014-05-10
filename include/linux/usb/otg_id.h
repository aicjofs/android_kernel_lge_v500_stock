/*
 * Copyright (C) 2011 Google, Inc.
 *
 * Author:
 *	Colin Cross <ccross@android.com>
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

#ifndef __LINUX_USB_OTG_ID_H
#define __LINUX_USB_OTG_ID_H

#include <linux/notifier.h>
#include <linux/plist.h>

/* 
                        
  
                                                                        
                       
                                                                                
                         
                                                                         
                                                                                
                                                                         
                        
                                                                        
                                                                         
 */

struct otg_id_notifier_block {
	int priority;
	int (*detect)(struct otg_id_notifier_block *otg_id_nb);
	int (*proxy_wait)(struct otg_id_notifier_block *otg_id_nb);
	void (*cancel)(struct otg_id_notifier_block *otg_id_nb);
	struct plist_node p;
};

#define OTG_ID_PROXY_WAIT 2
#define OTG_ID_HANDLED 1
#define OTG_ID_UNHANDLED 0

int otg_id_register_notifier(struct otg_id_notifier_block *otg_id_nb);
void otg_id_unregister_notifier(struct otg_id_notifier_block *otg_id_nb);

void otg_id_notify(void);
int otg_id_suspend(void);
void otg_id_resume(void);

#endif /*                      */
