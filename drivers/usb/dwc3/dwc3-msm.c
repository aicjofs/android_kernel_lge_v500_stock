/* Copyright (c) 2012, The Linux Foundation. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/platform_device.h>
#include <linux/dma-mapping.h>
#include <linux/pm_runtime.h>
#include <linux/interrupt.h>
#include <linux/ioport.h>
#include <linux/clk.h>
#include <linux/io.h>
#include <linux/module.h>
#include <linux/types.h>
#include <linux/delay.h>
#include <linux/of.h>
#include <linux/list.h>
#include <linux/debugfs.h>
#include <linux/uaccess.h>
#include <linux/usb/ch9.h>
#include <linux/usb/gadget.h>
#include <linux/usb/msm_hsusb.h>
#include <linux/regulator/consumer.h>

#include <mach/rpm-regulator.h>

#include "dwc3_otg.h"
#include "core.h"
#include "gadget.h"

/* 
                               
  
 */
#define DBM_BASE		0x000F8000
#define DBM_EP_CFG(n)		(DBM_BASE + (0x00 + 4 * (n)))
#define DBM_DATA_FIFO(n)	(DBM_BASE + (0x10 + 4 * (n)))
#define DBM_DATA_FIFO_SIZE(n)	(DBM_BASE + (0x20 + 4 * (n)))
#define DBM_DATA_FIFO_EN	(DBM_BASE + (0x30))
#define DBM_GEVNTADR		(DBM_BASE + (0x34))
#define DBM_GEVNTSIZ		(DBM_BASE + (0x38))
#define DBM_DBG_CNFG		(DBM_BASE + (0x3C))
#define DBM_HW_TRB0_EP(n)	(DBM_BASE + (0x40 + 4 * (n)))
#define DBM_HW_TRB1_EP(n)	(DBM_BASE + (0x50 + 4 * (n)))
#define DBM_HW_TRB2_EP(n)	(DBM_BASE + (0x60 + 4 * (n)))
#define DBM_HW_TRB3_EP(n)	(DBM_BASE + (0x70 + 4 * (n)))
#define DBM_PIPE_CFG		(DBM_BASE + (0x80))
#define DBM_SOFT_RESET		(DBM_BASE + (0x84))
#define DBM_GEN_CFG		(DBM_BASE + (0x88))

/* 
                                        
  
 */
/*            */
#define DBM_EN_EP		0x00000001
#define USB3_EPNUM		0x0000003E
#define DBM_BAM_PIPE_NUM	0x000000C0
#define DBM_PRODUCER		0x00000100
#define DBM_DISABLE_WB		0x00000200
#define DBM_INT_RAM_ACC		0x00000400

/*                    */
#define DBM_DATA_FIFO_SIZE_MASK	0x0000ffff

/*              */
#define DBM_GEVNTSIZ_MASK	0x0000ffff

/*              */
#define DBM_ENABLE_IOC_MASK	0x0000000f

/*                */
#define DBM_SFT_RST_EP0		0x00000001
#define DBM_SFT_RST_EP1		0x00000002
#define DBM_SFT_RST_EP2		0x00000004
#define DBM_SFT_RST_EP3		0x00000008
#define DBM_SFT_RST_EPS_MASK	0x0000000F
#define DBM_SFT_RST_MASK	0x80000000
#define DBM_EN_MASK		0x00000002

#define DBM_MAX_EPS		4

/*                        */
#define DBM_TRB_BIT		0x80000000
#define DBM_TRB_DATA_SRC	0x40000000
#define DBM_TRB_DMA		0x20000000
#define DBM_TRB_EP_NUM(ep)	(ep<<24)

/* 
                                   
  
 */
#define QSCRATCH_REG_OFFSET	(0x000F8800)
#define QSCRATCH_GENERAL_CFG	(QSCRATCH_REG_OFFSET + 0x08)
#define CHARGING_DET_CTRL_REG	(QSCRATCH_REG_OFFSET + 0x18)
#define CHARGING_DET_OUTPUT_REG	(QSCRATCH_REG_OFFSET + 0x1C)
#define ALT_INTERRUPT_EN_REG	(QSCRATCH_REG_OFFSET + 0x20)
#define HS_PHY_IRQ_STAT_REG	(QSCRATCH_REG_OFFSET + 0x24)

struct dwc3_msm_req_complete {
	struct list_head list_item;
	struct usb_request *req;
	void (*orig_complete)(struct usb_ep *ep,
			      struct usb_request *req);
};

struct dwc3_msm {
	struct platform_device *dwc3;
	struct device *dev;
	void __iomem *base;
	u32 resource_size;
	int dbm_num_eps;
	u8 ep_num_mapping[DBM_MAX_EPS];
	const struct usb_ep_ops *original_ep_ops[DWC3_ENDPOINTS_NUM];
	struct list_head req_complete_list;
	struct clk		*core_clk;
	struct regulator	*hsusb_3p3;
	struct regulator	*hsusb_1p8;
	struct regulator	*hsusb_vddcx;
	struct regulator	*ssusb_1p8;
	struct regulator	*ssusb_vddcx;
	enum usb_vdd_type	ss_vdd_type;
	enum usb_vdd_type	hs_vdd_type;
	struct dwc3_ext_xceiv	ext_xceiv;
	bool			resume_pending;
	atomic_t                pm_suspended;
	atomic_t		in_lpm;
	struct delayed_work	resume_work;
	struct wake_lock	wlock;
	struct dwc3_charger	charger;
	struct usb_phy		*otg_xceiv;
	struct delayed_work	chg_work;
	enum usb_chg_state	chg_state;
	u8			dcd_retries;
};

#define USB_HSPHY_3P3_VOL_MIN		3050000 /*    */
#define USB_HSPHY_3P3_VOL_MAX		3300000 /*    */
#define USB_HSPHY_3P3_HPM_LOAD		16000	/*    */

#define USB_HSPHY_1P8_VOL_MIN		1800000 /*    */
#define USB_HSPHY_1P8_VOL_MAX		1800000 /*    */
#define USB_HSPHY_1P8_HPM_LOAD		19000	/*    */

#define USB_SSPHY_1P8_VOL_MIN		1800000 /*    */
#define USB_SSPHY_1P8_VOL_MAX		1800000 /*    */
#define USB_SSPHY_1P8_HPM_LOAD		23000	/*    */

#define USB_PHY_VDD_DIG_VOL_NONE	0	/*    */
#define USB_PHY_VDD_DIG_VOL_MIN		1045000 /*    */
#define USB_PHY_VDD_DIG_VOL_MAX		1320000 /*    */

static const int vdd_val[VDD_TYPE_MAX][VDD_VAL_MAX] = {
		{  /*                      */
			[VDD_NONE]	= RPM_VREG_CORNER_NONE,
			[VDD_MIN]	= RPM_VREG_CORNER_NOMINAL,
			[VDD_MAX]	= RPM_VREG_CORNER_HIGH,
		},
		{ /*                       */
			[VDD_NONE]	= USB_PHY_VDD_DIG_VOL_NONE,
			[VDD_MIN]	= USB_PHY_VDD_DIG_VOL_MIN,
			[VDD_MAX]	= USB_PHY_VDD_DIG_VOL_MAX,
		},
};

static struct dwc3_msm *context;
static u64 dwc3_msm_dma_mask = DMA_BIT_MASK(64);

/* 
  
                                 
  
                                     
                             
  
              
 */
static inline u32 dwc3_msm_read_reg(void *base, u32 offset)
{
	u32 val = ioread32(base + offset);
	return val;
}

/* 
                                              
  
                                     
                             
                            
  
              
 */
static inline u32 dwc3_msm_read_reg_field(void *base,
					  u32 offset,
					  const u32 mask)
{
	u32 shift = find_first_bit((void *)&mask, 32);
	u32 val = ioread32(base + offset);
	val &= mask;		/*                  */
	val >>= shift;
	return val;
}

/* 
  
                                  
  
                                     
                             
                         
  
 */
static inline void dwc3_msm_write_reg(void *base, u32 offset, u32 val)
{
	iowrite32(val, base + offset);
}

/* 
                                               
  
                                     
                             
                            
                         
  
 */
static inline void dwc3_msm_write_reg_field(void *base, u32 offset,
					    const u32 mask, u32 val)
{
	u32 shift = find_first_bit((void *)&mask, 32);
	u32 tmp = ioread32(base + offset);

	tmp &= ~mask;		/*                    */
	val = tmp | (val << shift);
	iowrite32(val, base + offset);
}

/* 
                                                                     
  
                                     
                             
                                                             
                         
  
 */
static inline void dwc3_msm_write_readback(void *base, u32 offset,
					    const u32 mask, u32 val)
{
	u32 write_val, tmp = ioread32(base + offset);

	tmp &= ~mask;		/*                   */
	write_val = tmp | val;

	iowrite32(write_val, base + offset);

	/*                                     */
	tmp = ioread32(base + offset);
	tmp &= mask;		/*                  */

	if (tmp != val)
		dev_err(context->dev, "%s: write: %x to QSCRATCH: %x FAILED\n",
						__func__, val, offset);
}

/* 
                                                         
  
 */
static int dwc3_msm_find_matching_dbm_ep(u8 usb_ep)
{
	int i;

	for (i = 0; i < context->dbm_num_eps; i++)
		if (context->ep_num_mapping[i] == usb_ep)
			return i;

	return -ENODEV; /*           */
}

/* 
                                             
  
 */
static int dwc3_msm_configured_dbm_ep_num(void)
{
	int i;
	int count = 0;

	for (i = 0; i < context->dbm_num_eps; i++)
		if (context->ep_num_mapping[i])
			count++;

	return count;
}

/* 
                                                     
                                                               
  
                                       
                                    
  
 */
static int dwc3_msm_event_buffer_config(u32 addr, u16 size)
{
	dev_dbg(context->dev, "%s\n", __func__);

	dwc3_msm_write_reg(context->base, DBM_GEVNTADR, addr);
	dwc3_msm_write_reg_field(context->base, DBM_GEVNTSIZ,
		DBM_GEVNTSIZ_MASK, size);

	return 0;
}

/* 
                                               
  
 */
static int dwc3_msm_dbm_soft_reset(int enter_reset)
{
	dev_dbg(context->dev, "%s\n", __func__);
	if (enter_reset) {
		dev_dbg(context->dev, "enter DBM reset\n");
		dwc3_msm_write_reg_field(context->base, DBM_SOFT_RESET,
			DBM_SFT_RST_MASK, 1);
	} else {
		dev_dbg(context->dev, "exit DBM reset\n");
		dwc3_msm_write_reg_field(context->base, DBM_SOFT_RESET,
			DBM_SFT_RST_MASK, 0);
		/*          */
		dwc3_msm_write_reg_field(context->base, QSCRATCH_GENERAL_CFG,
			DBM_EN_MASK, 0x1);
	}

	return 0;
}

/* 
                              
                                                             
                                                        
                 
  
                           
                                                                 
  
 */
static int dwc3_msm_dbm_ep_soft_reset(u8 dbm_ep, bool enter_reset)
{
	dev_dbg(context->dev, "%s\n", __func__);

	if (dbm_ep >= context->dbm_num_eps) {
		dev_err(context->dev,
				"%s: Invalid DBM ep index\n", __func__);
		return -ENODEV;
	}

	if (enter_reset) {
		dwc3_msm_write_reg_field(context->base, DBM_SOFT_RESET,
			DBM_SFT_RST_EPS_MASK & 1 << dbm_ep, 1);
	} else {
		dwc3_msm_write_reg_field(context->base, DBM_SOFT_RESET,
			DBM_SFT_RST_EPS_MASK & 1 << dbm_ep, 0);
	}

	return 0;
}

/* 
                                              
  
  
                                    
                                 
                                                     
                                                         
                                         
  
                               
 */
static int dwc3_msm_dbm_ep_config(u8 usb_ep, u8 bam_pipe,
				  bool producer, bool disable_wb,
				  bool internal_mem, bool ioc)
{
	u8 dbm_ep;
	u32 ep_cfg;

	dev_dbg(context->dev, "%s\n", __func__);

	dbm_ep = dwc3_msm_find_matching_dbm_ep(usb_ep);

	if (dbm_ep < 0) {
		dev_err(context->dev,
				"%s: Invalid usb ep index\n", __func__);
		return -ENODEV;
	}
	/*                               */
	dwc3_msm_dbm_ep_soft_reset(dbm_ep, 0);

	/*                                  */
	dwc3_msm_write_reg_field(context->base, DBM_DBG_CNFG,
		DBM_ENABLE_IOC_MASK & 1 << dbm_ep, ioc ? 1 : 0);

	ep_cfg = (producer ? DBM_PRODUCER : 0) |
		(disable_wb ? DBM_DISABLE_WB : 0) |
		(internal_mem ? DBM_INT_RAM_ACC : 0);

	dwc3_msm_write_reg_field(context->base, DBM_EP_CFG(dbm_ep),
		DBM_PRODUCER | DBM_DISABLE_WB | DBM_INT_RAM_ACC, ep_cfg >> 8);

	dwc3_msm_write_reg_field(context->base, DBM_EP_CFG(dbm_ep), USB3_EPNUM,
		usb_ep);
	dwc3_msm_write_reg_field(context->base, DBM_EP_CFG(dbm_ep),
		DBM_BAM_PIPE_NUM, bam_pipe);
	dwc3_msm_write_reg_field(context->base, DBM_PIPE_CFG, 0x000000ff,
		0xe4);
	dwc3_msm_write_reg_field(context->base, DBM_EP_CFG(dbm_ep), DBM_EN_EP,
		1);

	return dbm_ep;
}

/* 
                                                 
  
                           
  
 */
static int dwc3_msm_dbm_ep_unconfig(u8 usb_ep)
{
	u8 dbm_ep;

	dev_dbg(context->dev, "%s\n", __func__);

	dbm_ep = dwc3_msm_find_matching_dbm_ep(usb_ep);

	if (dbm_ep < 0) {
		dev_err(context->dev,
				"%s: Invalid usb ep index\n", __func__);
		return -ENODEV;
	}

	context->ep_num_mapping[dbm_ep] = 0;

	dwc3_msm_write_reg(context->base, DBM_EP_CFG(dbm_ep), 0);

	/*                        */
	dwc3_msm_dbm_ep_soft_reset(dbm_ep, true);

	return 0;
}

/* 
                                              
                                                
                       
  
                                 
                                
                             
  
 */
int msm_data_fifo_config(struct usb_ep *ep, u32 addr, u32 size, u8 dst_pipe_idx)
{
	u8 dbm_ep;
	struct dwc3_ep *dep = to_dwc3_ep(ep);
	u8 bam_pipe = dst_pipe_idx;

	dev_dbg(context->dev, "%s\n", __func__);

	dbm_ep = bam_pipe;
	context->ep_num_mapping[dbm_ep] = dep->number;

	dwc3_msm_write_reg(context->base, DBM_DATA_FIFO(dbm_ep), addr);
	dwc3_msm_write_reg_field(context->base, DBM_DATA_FIFO_SIZE(dbm_ep),
		DBM_DATA_FIFO_SIZE_MASK, size);

	return 0;
}

/* 
                                                
 
                                      
 
                                       
                                             
 
                                                
*/
static void dwc3_msm_req_complete_func(struct usb_ep *ep,
				       struct usb_request *request)
{
	struct dwc3_request *req = to_dwc3_request(request);
	struct dwc3_ep *dep = to_dwc3_ep(ep);
	struct dwc3_msm_req_complete *req_complete = NULL;

	/*                                                                 */
	list_for_each_entry(req_complete,
				&context->req_complete_list,
				list_item) {
		if (req_complete->req == request)
			break;
	}
	if (!req_complete || req_complete->req != request) {
		dev_err(dep->dwc->dev, "%s: could not find the request\n",
					__func__);
		return;
	}
	list_del(&req_complete->list_item);

	/*
                                                                   
                                                                    
                      
  */
	if (req->queued)
		dep->busy_slot++;

	/*                    */
	dwc3_msm_dbm_ep_unconfig(dep->number);

	/*
                                                                 
                      
  */
	if (0 == dwc3_msm_configured_dbm_ep_num())
		dwc3_msm_event_buffer_config(0, 0);

	/*
                                                                     
                                                                  
  */
	request->complete = req_complete->orig_complete;
	if (request->complete)
		request->complete(ep, request);

	kfree(req_complete);
}

/* 
                  
                                                   
 
                                         
                                          
 
                                                
*/
static int __dwc3_msm_ep_queue(struct dwc3_ep *dep, struct dwc3_request *req)
{
	struct dwc3_trb *trb;
	struct dwc3_trb *trb_link;
	struct dwc3_gadget_ep_cmd_params params;
	u32 cmd;
	int ret = 0;

	/*                                                                 
                                                                       
                                                                     
                                                                        
                                                                      
                                                                      
  */
	req->queued = true;
	list_add_tail(&req->list, &dep->req_queued);

	/*                                                       */
	trb = &dep->trb_pool[dep->free_slot & DWC3_TRB_MASK];
	dep->free_slot++;
	memset(trb, 0, sizeof(*trb));

	req->trb = trb;
	trb->bph = DBM_TRB_BIT | DBM_TRB_DMA | DBM_TRB_EP_NUM(dep->number);
	trb->size = DWC3_TRB_SIZE_LENGTH(req->request.length);
	trb->ctrl = DWC3_TRBCTL_NORMAL | DWC3_TRB_CTRL_HWO | DWC3_TRB_CTRL_CHN;
	req->trb_dma = dwc3_trb_dma_offset(dep, trb);

	/*                                                        */
	trb_link = &dep->trb_pool[dep->free_slot & DWC3_TRB_MASK];
	dep->free_slot++;
	memset(trb_link, 0, sizeof *trb_link);

	trb_link->bpl = lower_32_bits(req->trb_dma);
	trb_link->bph = DBM_TRB_BIT |
			DBM_TRB_DMA | DBM_TRB_EP_NUM(dep->number);
	trb_link->size = 0;
	trb_link->ctrl = DWC3_TRBCTL_LINK_TRB | DWC3_TRB_CTRL_HWO;

	/*
                          
  */
	memset(&params, 0, sizeof(params));
	params.param0 = 0; /*             */
	params.param1 = lower_32_bits(req->trb_dma); /*           */

	cmd = DWC3_DEPCMD_STARTTRANSFER;
	ret = dwc3_send_gadget_ep_cmd(dep->dwc, dep->number, cmd, &params);
	if (ret < 0) {
		dev_dbg(dep->dwc->dev,
			"%s: failed to send STARTTRANSFER command\n",
			__func__);

		list_del(&req->list);
		return ret;
	}

	return ret;
}

/* 
                                          
                                                   
                               
 
                                                        
                                                         
                            
 
                                                       
                                                      
                               
 
                                                    
                                                              
 
                                       
                                             
                              
 
                                                
*/
static int dwc3_msm_ep_queue(struct usb_ep *ep,
			     struct usb_request *request, gfp_t gfp_flags)
{
	struct dwc3_request *req = to_dwc3_request(request);
	struct dwc3_ep *dep = to_dwc3_ep(ep);
	struct dwc3 *dwc = dep->dwc;
	struct dwc3_msm_req_complete *req_complete;
	unsigned long flags;
	int ret = 0;
	u8 bam_pipe;
	bool producer;
	bool disable_wb;
	bool internal_mem;
	bool ioc;
	u8 speed;

	if (!(request->udc_priv & MSM_SPS_MODE)) {
		/*                                   */
		dev_vdbg(dwc->dev, "%s: not sps mode, use regular queue\n",
					__func__);

		return (context->original_ep_ops[dep->number])->queue(ep,
								request,
								gfp_flags);
	}

	if (!dep->endpoint.desc) {
		dev_err(dwc->dev,
			"%s: trying to queue request %p to disabled ep %s\n",
			__func__, request, ep->name);
		return -EPERM;
	}

	if (dep->number == 0 || dep->number == 1) {
		dev_err(dwc->dev,
			"%s: trying to queue dbm request %p to control ep %s\n",
			__func__, request, ep->name);
		return -EPERM;
	}

	if (dep->free_slot > 0 || dep->busy_slot > 0 ||
		!list_empty(&dep->request_list) ||
		!list_empty(&dep->req_queued)) {

		dev_err(dwc->dev,
			"%s: trying to queue dbm request %p tp ep %s\n",
			__func__, request, ep->name);
		return -EPERM;
	}

	/*
                                                           
                                                         
  */
	req_complete = kzalloc(sizeof(*req_complete), GFP_KERNEL);
	if (!req_complete) {
		dev_err(dep->dwc->dev, "%s: not enough memory\n", __func__);
		return -ENOMEM;
	}
	req_complete->req = request;
	req_complete->orig_complete = request->complete;
	list_add_tail(&req_complete->list_item, &context->req_complete_list);
	request->complete = dwc3_msm_req_complete_func;

	/*
                              
  */
	bam_pipe = request->udc_priv & MSM_PIPE_ID_MASK;
	producer = ((request->udc_priv & MSM_PRODUCER) ? true : false);
	disable_wb = ((request->udc_priv & MSM_DISABLE_WB) ? true : false);
	internal_mem = ((request->udc_priv & MSM_INTERNAL_MEM) ? true : false);
	ioc = ((request->udc_priv & MSM_ETD_IOC) ? true : false);

	ret = dwc3_msm_dbm_ep_config(dep->number,
					bam_pipe, producer,
					disable_wb, internal_mem, ioc);
	if (ret < 0) {
		dev_err(context->dev,
			"error %d after calling dwc3_msm_dbm_ep_config\n",
			ret);
		return ret;
	}

	dev_vdbg(dwc->dev, "%s: queing request %p to ep %s length %d\n",
			__func__, request, ep->name, request->length);

	/*
                                                    
                                                      
                                                          
                                                           
                                                         
  */
	spin_lock_irqsave(&dwc->lock, flags);
	ret = __dwc3_msm_ep_queue(dep, req);
	spin_unlock_irqrestore(&dwc->lock, flags);
	if (ret < 0) {
		dev_err(context->dev,
			"error %d after calling __dwc3_msm_ep_queue\n", ret);
		return ret;
	}

	speed = dwc3_readl(dwc->regs, DWC3_DSTS) & DWC3_DSTS_CONNECTSPD;
	dwc3_msm_write_reg(context->base, DBM_GEN_CFG, speed >> 2);

	return 0;
}

/* 
                          
                                           
                                                   
                           
  
                                                       
                                                      
                                                          
  
                                          
  
                                                 
 */
int msm_ep_config(struct usb_ep *ep)
{
	struct dwc3_ep *dep = to_dwc3_ep(ep);
	struct usb_ep_ops *new_ep_ops;

	/*                                        */
	if (context->original_ep_ops[dep->number]) {
		dev_err(context->dev,
			"ep [%s,%d] already configured as msm endpoint\n",
			ep->name, dep->number);
		return -EPERM;
	}
	context->original_ep_ops[dep->number] = ep->ops;

	/*                            */
	new_ep_ops = kzalloc(sizeof(struct usb_ep_ops), GFP_KERNEL);
	if (!new_ep_ops) {
		dev_err(context->dev,
			"%s: unable to allocate mem for new usb ep ops\n",
			__func__);
		return -ENOMEM;
	}
	(*new_ep_ops) = (*ep->ops);
	new_ep_ops->queue = dwc3_msm_ep_queue;
	ep->ops = new_ep_ops;

	/*
                                            
                              
  */

	return 0;
}
EXPORT_SYMBOL(msm_ep_config);

/* 
                             
                                       
                               
  
                                          
  
                                                 
 */
int msm_ep_unconfig(struct usb_ep *ep)
{
	struct dwc3_ep *dep = to_dwc3_ep(ep);
	struct usb_ep_ops *old_ep_ops;

	/*                         */
	if (!context->original_ep_ops[dep->number]) {
		dev_err(context->dev,
			"ep [%s,%d] was not configured as msm endpoint\n",
			ep->name, dep->number);
		return -EINVAL;
	}
	old_ep_ops = (struct usb_ep_ops	*)ep->ops;
	ep->ops = context->original_ep_ops[dep->number];
	context->original_ep_ops[dep->number] = NULL;
	kfree(old_ep_ops);

	/*
                                               
                              
  */

	return 0;
}
EXPORT_SYMBOL(msm_ep_unconfig);

/*       */
static int dwc3_hsusb_config_vddcx(int high)
{
	int min_vol, ret;
	struct dwc3_msm *dwc = context;
	enum usb_vdd_type vdd_type = context->hs_vdd_type;
	int max_vol = vdd_val[vdd_type][VDD_MAX];

	min_vol = vdd_val[vdd_type][high ? VDD_MIN : VDD_NONE];
	ret = regulator_set_voltage(dwc->hsusb_vddcx, min_vol, max_vol);
	if (ret) {
		dev_err(dwc->dev, "unable to set voltage for HSUSB_VDDCX\n");
		return ret;
	}

	dev_dbg(dwc->dev, "%s: min_vol:%d max_vol:%d\n", __func__,
							min_vol, max_vol);

	return ret;
}

static int dwc3_hsusb_ldo_init(int init)
{
	int rc = 0;
	struct dwc3_msm *dwc = context;

	if (!init) {
		regulator_set_voltage(dwc->hsusb_1p8, 0, USB_HSPHY_1P8_VOL_MAX);
		regulator_set_voltage(dwc->hsusb_3p3, 0, USB_HSPHY_3P3_VOL_MAX);
		return 0;
	}

	dwc->hsusb_3p3 = devm_regulator_get(dwc->dev, "HSUSB_3p3");
	if (IS_ERR(dwc->hsusb_3p3)) {
		dev_err(dwc->dev, "unable to get hsusb 3p3\n");
		return PTR_ERR(dwc->hsusb_3p3);
	}

	rc = regulator_set_voltage(dwc->hsusb_3p3,
			USB_HSPHY_3P3_VOL_MIN, USB_HSPHY_3P3_VOL_MAX);
	if (rc) {
		dev_err(dwc->dev, "unable to set voltage for hsusb 3p3\n");
		return rc;
	}
	dwc->hsusb_1p8 = devm_regulator_get(dwc->dev, "HSUSB_1p8");
	if (IS_ERR(dwc->hsusb_1p8)) {
		dev_err(dwc->dev, "unable to get hsusb 1p8\n");
		rc = PTR_ERR(dwc->hsusb_1p8);
		goto devote_3p3;
	}
	rc = regulator_set_voltage(dwc->hsusb_1p8,
			USB_HSPHY_1P8_VOL_MIN, USB_HSPHY_1P8_VOL_MAX);
	if (rc) {
		dev_err(dwc->dev, "unable to set voltage for hsusb 1p8\n");
		goto devote_3p3;
	}

	return 0;

devote_3p3:
	regulator_set_voltage(dwc->hsusb_3p3, 0, USB_HSPHY_3P3_VOL_MAX);

	return rc;
}

static int dwc3_hsusb_ldo_enable(int on)
{
	int rc = 0;
	struct dwc3_msm *dwc = context;

	dev_dbg(dwc->dev, "reg (%s)\n", on ? "HPM" : "LPM");

	if (!on)
		goto disable_regulators;


	rc = regulator_set_optimum_mode(dwc->hsusb_1p8, USB_HSPHY_1P8_HPM_LOAD);
	if (rc < 0) {
		dev_err(dwc->dev, "Unable to set HPM of regulator HSUSB_1p8\n");
		return rc;
	}

	rc = regulator_enable(dwc->hsusb_1p8);
	if (rc) {
		dev_err(dwc->dev, "Unable to enable HSUSB_1p8\n");
		goto put_1p8_lpm;
	}

	rc = regulator_set_optimum_mode(dwc->hsusb_3p3,	USB_HSPHY_3P3_HPM_LOAD);
	if (rc < 0) {
		dev_err(dwc->dev, "Unable to set HPM of regulator HSUSB_3p3\n");
		goto disable_1p8;
	}

	rc = regulator_enable(dwc->hsusb_3p3);
	if (rc) {
		dev_err(dwc->dev, "Unable to enable HSUSB_3p3\n");
		goto put_3p3_lpm;
	}

	return 0;

disable_regulators:
	rc = regulator_disable(dwc->hsusb_3p3);
	if (rc)
		dev_err(dwc->dev, "Unable to disable HSUSB_3p3\n");

put_3p3_lpm:
	rc = regulator_set_optimum_mode(dwc->hsusb_3p3, 0);
	if (rc < 0)
		dev_err(dwc->dev, "Unable to set LPM of regulator HSUSB_3p3\n");

disable_1p8:
	rc = regulator_disable(dwc->hsusb_1p8);
	if (rc)
		dev_err(dwc->dev, "Unable to disable HSUSB_1p8\n");

put_1p8_lpm:
	rc = regulator_set_optimum_mode(dwc->hsusb_1p8, 0);
	if (rc < 0)
		dev_err(dwc->dev, "Unable to set LPM of regulator HSUSB_1p8\n");

	return rc < 0 ? rc : 0;
}

/*       */
static int dwc3_ssusb_config_vddcx(int high)
{
	int min_vol, ret;
	struct dwc3_msm *dwc = context;
	enum usb_vdd_type vdd_type = context->ss_vdd_type;
	int max_vol = vdd_val[vdd_type][VDD_MAX];

	min_vol = vdd_val[vdd_type][high ? VDD_MIN : VDD_NONE];
	ret = regulator_set_voltage(dwc->ssusb_vddcx, min_vol, max_vol);
	if (ret) {
		dev_err(dwc->dev, "unable to set voltage for SSUSB_VDDCX\n");
		return ret;
	}

	dev_dbg(dwc->dev, "%s: min_vol:%d max_vol:%d\n", __func__,
							min_vol, max_vol);
	return ret;
}

/*                                   */
static int dwc3_ssusb_ldo_init(int init)
{
	int rc = 0;
	struct dwc3_msm *dwc = context;

	if (!init) {
		regulator_set_voltage(dwc->ssusb_1p8, 0, USB_SSPHY_1P8_VOL_MAX);
		return 0;
	}

	dwc->ssusb_1p8 = devm_regulator_get(dwc->dev, "SSUSB_1p8");
	if (IS_ERR(dwc->ssusb_1p8)) {
		dev_err(dwc->dev, "unable to get ssusb 1p8\n");
		return PTR_ERR(dwc->ssusb_1p8);
	}
	rc = regulator_set_voltage(dwc->ssusb_1p8,
			USB_SSPHY_1P8_VOL_MIN, USB_SSPHY_1P8_VOL_MAX);
	if (rc)
		dev_err(dwc->dev, "unable to set voltage for ssusb 1p8\n");

	return rc;
}

static int dwc3_ssusb_ldo_enable(int on)
{
	int rc = 0;
	struct dwc3_msm *dwc = context;

	dev_dbg(context->dev, "reg (%s)\n", on ? "HPM" : "LPM");

	if (!on)
		goto disable_regulators;


	rc = regulator_set_optimum_mode(dwc->ssusb_1p8, USB_SSPHY_1P8_HPM_LOAD);
	if (rc < 0) {
		dev_err(dwc->dev, "Unable to set HPM of SSUSB_1p8\n");
		return rc;
	}

	rc = regulator_enable(dwc->ssusb_1p8);
	if (rc) {
		dev_err(dwc->dev, "Unable to enable SSUSB_1p8\n");
		goto put_1p8_lpm;
	}

	return 0;

disable_regulators:
	rc = regulator_disable(dwc->ssusb_1p8);
	if (rc)
		dev_err(dwc->dev, "Unable to disable SSUSB_1p8\n");

put_1p8_lpm:
	rc = regulator_set_optimum_mode(dwc->ssusb_1p8, 0);
	if (rc < 0)
		dev_err(dwc->dev, "Unable to set LPM of SSUSB_1p8\n");

	return rc < 0 ? rc : 0;
}

static void dwc3_chg_enable_secondary_det(struct dwc3_msm *mdwc)
{
	u32 chg_ctrl;

	/*                  */
	dwc3_msm_write_reg(mdwc->base, CHARGING_DET_CTRL_REG, 0x0);
	msleep(20);

	/*                                            */
	chg_ctrl = dwc3_msm_read_reg(mdwc->base, CHARGING_DET_CTRL_REG);
	if (chg_ctrl & 0x3F)
		dev_err(mdwc->dev, "%s Unable to reset chg_det block: %x\n",
						 __func__, chg_ctrl);
	/*
                                                      
                                            
  */
	dwc3_msm_write_readback(mdwc->base, CHARGING_DET_CTRL_REG, 0x3F, 0x34);
}

static bool dwc3_chg_det_check_output(struct dwc3_msm *mdwc)
{
	u32 chg_det;
	bool ret = false;

	chg_det = dwc3_msm_read_reg(mdwc->base, CHARGING_DET_OUTPUT_REG);
	ret = chg_det & 1;

	return ret;
}

static void dwc3_chg_enable_primary_det(struct dwc3_msm *mdwc)
{
	/*
                                                      
                                            
  */
	dwc3_msm_write_readback(mdwc->base, CHARGING_DET_CTRL_REG, 0x3F, 0x30);
}

static inline bool dwc3_chg_check_dcd(struct dwc3_msm *mdwc)
{
	u32 chg_state;
	bool ret = false;

	chg_state = dwc3_msm_read_reg(mdwc->base, CHARGING_DET_OUTPUT_REG);
	ret = chg_state & 2;

	return ret;
}

static inline void dwc3_chg_disable_dcd(struct dwc3_msm *mdwc)
{
	dwc3_msm_write_readback(mdwc->base, CHARGING_DET_CTRL_REG, 0x3F, 0x0);
}

static inline void dwc3_chg_enable_dcd(struct dwc3_msm *mdwc)
{
	/*                                       */
	dwc3_msm_write_readback(mdwc->base, CHARGING_DET_CTRL_REG, 0x3F, 0x2);
}

static void dwc3_chg_block_reset(struct dwc3_msm *mdwc)
{
	u32 chg_ctrl;

	/*                                      */
	dwc3_msm_write_reg(mdwc->base, CHARGING_DET_CTRL_REG, 0x0);

	/*                                           */
	dwc3_msm_write_reg(mdwc->base, HS_PHY_IRQ_STAT_REG, 0xFFF);
	dwc3_msm_write_reg(mdwc->base, ALT_INTERRUPT_EN_REG, 0x0);

	udelay(100);

	/*                                                    */
	chg_ctrl = dwc3_msm_read_reg(mdwc->base, CHARGING_DET_CTRL_REG);
	if (chg_ctrl & 0x3F)
		dev_err(mdwc->dev, "%s Unable to reset chg_det block: %x\n",
						 __func__, chg_ctrl);
}

static const char *chg_to_string(enum dwc3_chg_type chg_type)
{
	switch (chg_type) {
	case USB_SDP_CHARGER:		return "USB_SDP_CHARGER";
	case USB_DCP_CHARGER:		return "USB_DCP_CHARGER";
	case USB_CDP_CHARGER:		return "USB_CDP_CHARGER";
	default:			return "INVALID_CHARGER";
	}
}

#define DWC3_CHG_DCD_POLL_TIME		(100 * HZ/1000) /*          */
#define DWC3_CHG_DCD_MAX_RETRIES	6 /*                           */
#define DWC3_CHG_PRIMARY_DET_TIME	(50 * HZ/1000) /*            */
#define DWC3_CHG_SECONDARY_DET_TIME	(50 * HZ/1000) /*            */

static void dwc3_chg_detect_work(struct work_struct *w)
{
	struct dwc3_msm *mdwc = container_of(w, struct dwc3_msm, chg_work.work);
	bool is_dcd = false, tmout, vout;
	unsigned long delay;

	dev_dbg(mdwc->dev, "chg detection work\n");
	switch (mdwc->chg_state) {
	case USB_CHG_STATE_UNDEFINED:
		dwc3_chg_block_reset(mdwc);
		dwc3_chg_enable_dcd(mdwc);
		mdwc->chg_state = USB_CHG_STATE_WAIT_FOR_DCD;
		mdwc->dcd_retries = 0;
		delay = DWC3_CHG_DCD_POLL_TIME;
		break;
	case USB_CHG_STATE_WAIT_FOR_DCD:
		is_dcd = dwc3_chg_check_dcd(mdwc);
		tmout = ++mdwc->dcd_retries == DWC3_CHG_DCD_MAX_RETRIES;
		if (is_dcd || tmout) {
			dwc3_chg_disable_dcd(mdwc);
			dwc3_chg_enable_primary_det(mdwc);
			delay = DWC3_CHG_PRIMARY_DET_TIME;
			mdwc->chg_state = USB_CHG_STATE_DCD_DONE;
		} else {
			delay = DWC3_CHG_DCD_POLL_TIME;
		}
		break;
	case USB_CHG_STATE_DCD_DONE:
		vout = dwc3_chg_det_check_output(mdwc);
		if (vout) {
			dwc3_chg_enable_secondary_det(mdwc);
			delay = DWC3_CHG_SECONDARY_DET_TIME;
			mdwc->chg_state = USB_CHG_STATE_PRIMARY_DONE;
		} else {
			mdwc->charger.chg_type = USB_SDP_CHARGER;
			mdwc->chg_state = USB_CHG_STATE_DETECTED;
			delay = 0;
		}
		break;
	case USB_CHG_STATE_PRIMARY_DONE:
		vout = dwc3_chg_det_check_output(mdwc);
		if (vout)
			mdwc->charger.chg_type = USB_DCP_CHARGER;
		else
			mdwc->charger.chg_type = USB_CDP_CHARGER;
		mdwc->chg_state = USB_CHG_STATE_SECONDARY_DONE;
		/*              */
	case USB_CHG_STATE_SECONDARY_DONE:
		mdwc->chg_state = USB_CHG_STATE_DETECTED;
		/*              */
	case USB_CHG_STATE_DETECTED:
		dwc3_chg_block_reset(mdwc);
		dev_dbg(mdwc->dev, "chg_type = %s\n",
			chg_to_string(mdwc->charger.chg_type));
		mdwc->charger.notify_detection_complete(mdwc->otg_xceiv->otg,
								&mdwc->charger);
		return;
	default:
		return;
	}

	queue_delayed_work(system_nrt_wq, &mdwc->chg_work, delay);
}

static void dwc3_start_chg_det(struct dwc3_charger *charger, bool start)
{
	struct dwc3_msm *mdwc = context;

	if (start == false) {
		cancel_delayed_work_sync(&mdwc->chg_work);
		mdwc->chg_state = USB_CHG_STATE_UNDEFINED;
		charger->chg_type = DWC3_INVALID_CHARGER;
		return;
	}

	mdwc->chg_state = USB_CHG_STATE_UNDEFINED;
	charger->chg_type = DWC3_INVALID_CHARGER;
	queue_delayed_work(system_nrt_wq, &mdwc->chg_work, 0);
}

static int dwc3_msm_suspend(struct dwc3_msm *mdwc)
{
	dev_dbg(mdwc->dev, "%s: entering lpm\n", __func__);

	if (atomic_read(&mdwc->in_lpm)) {
		dev_dbg(mdwc->dev, "%s: Already suspended\n", __func__);
		return 0;
	}

	clk_disable_unprepare(mdwc->core_clk);
	dwc3_hsusb_ldo_enable(0);
	dwc3_ssusb_ldo_enable(0);
	wake_unlock(&mdwc->wlock);

	atomic_set(&mdwc->in_lpm, 1);
	dev_info(mdwc->dev, "DWC3 in low power mode\n");

	return 0;
}

static int dwc3_msm_resume(struct dwc3_msm *mdwc)
{
	dev_dbg(mdwc->dev, "%s: exiting lpm\n", __func__);

	if (!atomic_read(&mdwc->in_lpm)) {
		dev_dbg(mdwc->dev, "%s: Already resumed\n", __func__);
		return 0;
	}

	wake_lock(&mdwc->wlock);
	clk_prepare_enable(mdwc->core_clk);
	dwc3_hsusb_ldo_enable(1);
	dwc3_ssusb_ldo_enable(1);

	atomic_set(&mdwc->in_lpm, 0);
	dev_info(mdwc->dev, "DWC3 exited from low power mode\n");

	return 0;
}

static void dwc3_resume_work(struct work_struct *w)
{
	struct dwc3_msm *mdwc = container_of(w, struct dwc3_msm,
							resume_work.work);

	dev_dbg(mdwc->dev, "%s: dwc3 resume work\n", __func__);
	/*                                                                 */
	if (!atomic_read(&mdwc->in_lpm)) {
		dev_dbg(mdwc->dev, "%s: notifying xceiv event\n", __func__);
		if (mdwc->otg_xceiv)
			mdwc->ext_xceiv.notify_ext_events(mdwc->otg_xceiv->otg,
							DWC3_EVENT_XCEIV_STATE);
		return;
	}

	/*                                                            */
	if (atomic_read(&mdwc->pm_suspended)) {
		mdwc->resume_pending = true;
	} else {
		pm_runtime_get_sync(mdwc->dev);
		if (mdwc->otg_xceiv)
			mdwc->ext_xceiv.notify_ext_events(mdwc->otg_xceiv->otg,
							DWC3_EVENT_PHY_RESUME);
		pm_runtime_put_sync(mdwc->dev);
	}
}

static bool debug_id, debug_bsv, debug_connect;

static int dwc3_connect_show(struct seq_file *s, void *unused)
{
	if (debug_connect)
		seq_printf(s, "true\n");
	else
		seq_printf(s, "false\n");

	return 0;
}

static int dwc3_connect_open(struct inode *inode, struct file *file)
{
	return single_open(file, dwc3_connect_show, inode->i_private);
}

static ssize_t dwc3_connect_write(struct file *file, const char __user *ubuf,
				size_t count, loff_t *ppos)
{
	struct seq_file *s = file->private_data;
	struct dwc3_msm *mdwc = s->private;
	char buf[8];

	memset(buf, 0x00, sizeof(buf));

	if (copy_from_user(&buf, ubuf, min_t(size_t, sizeof(buf) - 1, count)))
		return -EFAULT;

	if (!strncmp(buf, "enable", 6) || !strncmp(buf, "true", 4)) {
		debug_connect = true;
	} else {
		debug_connect = debug_bsv = false;
		debug_id = true;
	}

	mdwc->ext_xceiv.bsv = debug_bsv;
	mdwc->ext_xceiv.id = debug_id ? DWC3_ID_FLOAT : DWC3_ID_GROUND;

	if (atomic_read(&mdwc->in_lpm)) {
		dev_dbg(mdwc->dev, "%s: calling resume_work\n", __func__);
		dwc3_resume_work(&mdwc->resume_work.work);
	} else {
		dev_dbg(mdwc->dev, "%s: notifying xceiv event\n", __func__);
		if (mdwc->otg_xceiv)
			mdwc->ext_xceiv.notify_ext_events(mdwc->otg_xceiv->otg,
							DWC3_EVENT_XCEIV_STATE);
	}

	return count;
}

const struct file_operations dwc3_connect_fops = {
	.open = dwc3_connect_open,
	.read = seq_read,
	.write = dwc3_connect_write,
	.llseek = seq_lseek,
	.release = single_release,
};

static struct dentry *dwc3_debugfs_root;

static void dwc3_debugfs_init(struct dwc3_msm *mdwc)
{
	dwc3_debugfs_root = debugfs_create_dir("msm_dwc3", NULL);

	if (!dwc3_debugfs_root || IS_ERR(dwc3_debugfs_root))
		return;

	if (!debugfs_create_bool("id", S_IRUGO | S_IWUSR, dwc3_debugfs_root,
				 (u32 *)&debug_id))
		goto error;

	if (!debugfs_create_bool("bsv", S_IRUGO | S_IWUSR, dwc3_debugfs_root,
				 (u32 *)&debug_bsv))
		goto error;

	if (!debugfs_create_file("connect", S_IRUGO | S_IWUSR,
				dwc3_debugfs_root, mdwc, &dwc3_connect_fops))
		goto error;

	return;

error:
	debugfs_remove_recursive(dwc3_debugfs_root);
}

static int __devinit dwc3_msm_probe(struct platform_device *pdev)
{
	struct device_node *node = pdev->dev.of_node;
	struct platform_device *dwc3;
	struct dwc3_msm *msm;
	struct resource *res;
	int ret = 0;

	msm = devm_kzalloc(&pdev->dev, sizeof(*msm), GFP_KERNEL);
	if (!msm) {
		dev_err(&pdev->dev, "not enough memory\n");
		return -ENOMEM;
	}

	platform_set_drvdata(pdev, msm);
	context = msm;
	msm->dev = &pdev->dev;

	INIT_LIST_HEAD(&msm->req_complete_list);
	INIT_DELAYED_WORK(&msm->chg_work, dwc3_chg_detect_work);
	INIT_DELAYED_WORK(&msm->resume_work, dwc3_resume_work);

	/*
                                                             
                                                          
  */
	msm->core_clk = devm_clk_get(&pdev->dev, "core_clk");
	if (IS_ERR(msm->core_clk)) {
		dev_err(&pdev->dev, "failed to get core_clk\n");
		return PTR_ERR(msm->core_clk);
	}
	clk_set_rate(msm->core_clk, 125000000);
	clk_prepare_enable(msm->core_clk);

	/*        */
	msm->ss_vdd_type = VDDCX_CORNER;
	msm->ssusb_vddcx = devm_regulator_get(&pdev->dev, "ssusb_vdd_dig");
	if (IS_ERR(msm->ssusb_vddcx)) {
		msm->ssusb_vddcx = devm_regulator_get(&pdev->dev,
							"SSUSB_VDDCX");
		if (IS_ERR(msm->ssusb_vddcx)) {
			dev_err(&pdev->dev, "unable to get ssusb vddcx\n");
			ret = PTR_ERR(msm->ssusb_vddcx);
			goto disable_core_clk;
		}
		msm->ss_vdd_type = VDDCX;
		dev_dbg(&pdev->dev, "ss_vdd_type: VDDCX\n");
	}

	ret = dwc3_ssusb_config_vddcx(1);
	if (ret) {
		dev_err(&pdev->dev, "ssusb vddcx configuration failed\n");
		goto disable_core_clk;
	}

	ret = regulator_enable(context->ssusb_vddcx);
	if (ret) {
		dev_err(&pdev->dev, "unable to enable the ssusb vddcx\n");
		goto unconfig_ss_vddcx;
	}

	ret = dwc3_ssusb_ldo_init(1);
	if (ret) {
		dev_err(&pdev->dev, "ssusb vreg configuration failed\n");
		goto disable_ss_vddcx;
	}

	ret = dwc3_ssusb_ldo_enable(1);
	if (ret) {
		dev_err(&pdev->dev, "ssusb vreg enable failed\n");
		goto free_ss_ldo_init;
	}

	/*        */
	msm->hs_vdd_type = VDDCX_CORNER;
	msm->hsusb_vddcx = devm_regulator_get(&pdev->dev, "hsusb_vdd_dig");
	if (IS_ERR(msm->hsusb_vddcx)) {
		msm->hsusb_vddcx = devm_regulator_get(&pdev->dev,
							"HSUSB_VDDCX");
		if (IS_ERR(msm->hsusb_vddcx)) {
			dev_err(&pdev->dev, "unable to get hsusb vddcx\n");
			ret = PTR_ERR(msm->ssusb_vddcx);
			goto disable_ss_ldo;
		}
		msm->hs_vdd_type = VDDCX;
		dev_dbg(&pdev->dev, "hs_vdd_type: VDDCX\n");
	}

	ret = dwc3_hsusb_config_vddcx(1);
	if (ret) {
		dev_err(&pdev->dev, "hsusb vddcx configuration failed\n");
		goto disable_ss_ldo;
	}

	ret = regulator_enable(context->hsusb_vddcx);
	if (ret) {
		dev_err(&pdev->dev, "unable to enable the hsusb vddcx\n");
		goto unconfig_hs_vddcx;
	}

	ret = dwc3_hsusb_ldo_init(1);
	if (ret) {
		dev_err(&pdev->dev, "hsusb vreg configuration failed\n");
		goto disable_hs_vddcx;
	}

	ret = dwc3_hsusb_ldo_enable(1);
	if (ret) {
		dev_err(&pdev->dev, "hsusb vreg enable failed\n");
		goto free_hs_ldo_init;
	}

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (!res) {
		dev_err(&pdev->dev, "missing memory base resource\n");
		ret = -ENODEV;
		goto disable_hs_ldo;
	}

	msm->base = devm_ioremap_nocache(&pdev->dev, res->start,
		resource_size(res));
	if (!msm->base) {
		dev_err(&pdev->dev, "ioremap failed\n");
		ret = -ENODEV;
		goto disable_hs_ldo;
	}

	dwc3 = platform_device_alloc("dwc3", -1);
	if (!dwc3) {
		dev_err(&pdev->dev, "couldn't allocate dwc3 device\n");
		ret = -ENODEV;
		goto disable_hs_ldo;
	}

	dwc3->dev.parent = &pdev->dev;
	dwc3->dev.coherent_dma_mask = DMA_BIT_MASK(32);
	dwc3->dev.dma_mask = &dwc3_msm_dma_mask;
	dwc3->dev.dma_parms = pdev->dev.dma_parms;
	msm->resource_size = resource_size(res);
	msm->dwc3 = dwc3;

	pm_runtime_set_active(msm->dev);
	pm_runtime_enable(msm->dev);

	if (of_property_read_u32(node, "qcom,dwc-usb3-msm-dbm-eps",
				 &msm->dbm_num_eps)) {
		dev_err(&pdev->dev,
			"unable to read platform data num of dbm eps\n");
		msm->dbm_num_eps = DBM_MAX_EPS;
	}

	if (msm->dbm_num_eps > DBM_MAX_EPS) {
		dev_err(&pdev->dev,
			"Driver doesn't support number of DBM EPs. "
			"max: %d, dbm_num_eps: %d\n",
			DBM_MAX_EPS, msm->dbm_num_eps);
		ret = -ENODEV;
		goto put_pdev;
	}

	ret = platform_device_add_resources(dwc3, pdev->resource,
		pdev->num_resources);
	if (ret) {
		dev_err(&pdev->dev, "couldn't add resources to dwc3 device\n");
		goto put_pdev;
	}

	ret = platform_device_add(dwc3);
	if (ret) {
		dev_err(&pdev->dev, "failed to register dwc3 device\n");
		goto put_pdev;
	}

	/*               */
	dwc3_msm_dbm_soft_reset(1);
	usleep_range(1000, 1200);
	dwc3_msm_dbm_soft_reset(0);

	dwc3_msm_event_buffer_config(dwc3_readl(msm->base, DWC3_GEVNTADRLO(0)),
		dwc3_readl(msm->base, DWC3_GEVNTSIZ(0)));

	msm->otg_xceiv = usb_get_transceiver();
	if (msm->otg_xceiv) {
		msm->charger.start_detection = dwc3_start_chg_det;
		ret = dwc3_set_charger(msm->otg_xceiv->otg, &msm->charger);
		if (ret || !msm->charger.notify_detection_complete) {
			dev_err(&pdev->dev, "failed to register charger: %d\n",
									ret);
			goto put_xcvr;
		}

		ret = dwc3_set_ext_xceiv(msm->otg_xceiv->otg, &msm->ext_xceiv);
		if (ret || !msm->ext_xceiv.notify_ext_events) {
			dev_err(&pdev->dev, "failed to register xceiver: %d\n",
									ret);
			goto put_xcvr;
		}
	} else {
		dev_err(&pdev->dev, "%s: No OTG transceiver found\n", __func__);
	}

	wake_lock_init(&msm->wlock, WAKE_LOCK_SUSPEND, "msm_dwc3");
	wake_lock(&msm->wlock);
	dwc3_debugfs_init(msm);

	return 0;

put_xcvr:
	usb_put_transceiver(msm->otg_xceiv);
	platform_device_del(dwc3);
put_pdev:
	platform_device_put(dwc3);
disable_hs_ldo:
	dwc3_hsusb_ldo_enable(0);
free_hs_ldo_init:
	dwc3_hsusb_ldo_init(0);
disable_hs_vddcx:
	regulator_disable(context->hsusb_vddcx);
unconfig_hs_vddcx:
	dwc3_hsusb_config_vddcx(0);
disable_ss_ldo:
	dwc3_ssusb_ldo_enable(0);
free_ss_ldo_init:
	dwc3_ssusb_ldo_init(0);
disable_ss_vddcx:
	regulator_disable(context->ssusb_vddcx);
unconfig_ss_vddcx:
	dwc3_ssusb_config_vddcx(0);
disable_core_clk:
	clk_disable_unprepare(msm->core_clk);

	return ret;
}

static int __devexit dwc3_msm_remove(struct platform_device *pdev)
{
	struct dwc3_msm	*msm = platform_get_drvdata(pdev);

	if (dwc3_debugfs_root)
		debugfs_remove_recursive(dwc3_debugfs_root);
	if (msm->otg_xceiv) {
		dwc3_start_chg_det(&msm->charger, false);
		usb_put_transceiver(msm->otg_xceiv);
	}
	pm_runtime_disable(msm->dev);
	platform_device_unregister(msm->dwc3);
	wake_lock_destroy(&msm->wlock);

	dwc3_hsusb_ldo_enable(0);
	dwc3_hsusb_ldo_init(0);
	regulator_disable(msm->hsusb_vddcx);
	dwc3_hsusb_config_vddcx(0);
	dwc3_ssusb_ldo_enable(0);
	dwc3_ssusb_ldo_init(0);
	regulator_disable(msm->ssusb_vddcx);
	dwc3_ssusb_config_vddcx(0);
	clk_disable_unprepare(msm->core_clk);

	return 0;
}

static int dwc3_msm_pm_suspend(struct device *dev)
{
	int ret = 0;
	struct dwc3_msm *mdwc = dev_get_drvdata(dev);

	dev_dbg(dev, "dwc3-msm PM suspend\n");

	ret = dwc3_msm_suspend(mdwc);
	if (!ret)
		atomic_set(&mdwc->pm_suspended, 1);

	return ret;
}

static int dwc3_msm_pm_resume(struct device *dev)
{
	int ret = 0;
	struct dwc3_msm *mdwc = dev_get_drvdata(dev);

	dev_dbg(dev, "dwc3-msm PM resume\n");

	atomic_set(&mdwc->pm_suspended, 0);
	if (mdwc->resume_pending) {
		mdwc->resume_pending = false;

		ret = dwc3_msm_resume(mdwc);
		/*                          */
		pm_runtime_disable(dev);
		pm_runtime_set_active(dev);
		pm_runtime_enable(dev);

		/*                                                     */
		if (mdwc->otg_xceiv)
			mdwc->ext_xceiv.notify_ext_events(mdwc->otg_xceiv->otg,
							DWC3_EVENT_PHY_RESUME);
	}

	return ret;
}

static int dwc3_msm_runtime_idle(struct device *dev)
{
	dev_dbg(dev, "DWC3-msm runtime idle\n");

	return 0;
}

static int dwc3_msm_runtime_suspend(struct device *dev)
{
	struct dwc3_msm *mdwc = dev_get_drvdata(dev);

	dev_dbg(dev, "DWC3-msm runtime suspend\n");

	return dwc3_msm_suspend(mdwc);
}

static int dwc3_msm_runtime_resume(struct device *dev)
{
	struct dwc3_msm *mdwc = dev_get_drvdata(dev);

	dev_dbg(dev, "DWC3-msm runtime resume\n");

	return dwc3_msm_resume(mdwc);
}

static const struct dev_pm_ops dwc3_msm_dev_pm_ops = {
	SET_SYSTEM_SLEEP_PM_OPS(dwc3_msm_pm_suspend, dwc3_msm_pm_resume)
	SET_RUNTIME_PM_OPS(dwc3_msm_runtime_suspend, dwc3_msm_runtime_resume,
				dwc3_msm_runtime_idle)
};

static const struct of_device_id of_dwc3_matach[] = {
	{
		.compatible = "qcom,dwc-usb3-msm",
	},
	{ },
};
MODULE_DEVICE_TABLE(of, of_dwc3_matach);

static struct platform_driver dwc3_msm_driver = {
	.probe		= dwc3_msm_probe,
	.remove		= __devexit_p(dwc3_msm_remove),
	.driver		= {
		.name	= "msm-dwc3",
		.pm	= &dwc3_msm_dev_pm_ops,
		.of_match_table	= of_dwc3_matach,
	},
};

MODULE_LICENSE("GPLV2");
MODULE_DESCRIPTION("DesignWare USB3 MSM Glue Layer");

static int __devinit dwc3_msm_init(void)
{
	return platform_driver_register(&dwc3_msm_driver);
}
module_init(dwc3_msm_init);

static void __exit dwc3_msm_exit(void)
{
	platform_driver_unregister(&dwc3_msm_driver);
}
module_exit(dwc3_msm_exit);
