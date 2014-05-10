/**
 * dwc3_otg.h - DesignWare USB3 DRD Controller OTG
 *
 * Copyright (c) 2012, The Linux Foundation. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#ifndef __LINUX_USB_DWC3_OTG_H
#define __LINUX_USB_DWC3_OTG_H

#include <linux/workqueue.h>

#include <linux/usb/otg.h>

struct dwc3_charger;

/* 
                                                           
                                       
                                                  
                                           
                                    
                                           
                                    
 */
struct dwc3_otg {
	struct usb_otg otg;
	int irq;
	void __iomem *regs;
	struct work_struct	sm_work;
	struct dwc3_charger	*charger;
	struct dwc3_ext_xceiv	*ext_xceiv;
#define ID		0
#define B_SESS_VLD	1
	unsigned long inputs;
};

/* 
                    
  
                                            
                                                                         
                                                  
                                                                      
                                                                        
                                                                            
 */
enum dwc3_chg_type {
	DWC3_INVALID_CHARGER = 0,
	DWC3_SDP_CHARGER,
	DWC3_DCP_CHARGER,
	DWC3_CDP_CHARGER,
};

struct dwc3_charger {
	enum dwc3_chg_type	chg_type;

	/*                                                                   */
	void	(*start_detection)(struct dwc3_charger *charger, bool start);

	/*                                                                   */
	void	(*notify_detection_complete)(struct usb_otg *otg,
						struct dwc3_charger *charger);
};

/*                             */
extern int dwc3_set_charger(struct usb_otg *otg, struct dwc3_charger *charger);

enum dwc3_ext_events {
	DWC3_EVENT_NONE = 0,		/*                 */
	DWC3_EVENT_PHY_RESUME,		/*                         */
	DWC3_EVENT_XCEIV_STATE,		/*                                  */
};

enum dwc3_id_state {
	DWC3_ID_GROUND = 0,
	DWC3_ID_FLOAT,
};

/*                                                                            */
struct dwc3_ext_xceiv {
	enum dwc3_id_state	id;
	bool			bsv;

	/*                                                     */
	void	(*notify_ext_events)(struct usb_otg *otg,
					enum dwc3_ext_events ext_event);
};

/*                                 */
extern int dwc3_set_ext_xceiv(struct usb_otg *otg,
				struct dwc3_ext_xceiv *ext_xceiv);

#endif /*                        */
