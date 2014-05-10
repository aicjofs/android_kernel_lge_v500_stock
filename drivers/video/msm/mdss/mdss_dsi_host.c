
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
#include <linux/interrupt.h>
#include <linux/spinlock.h>
#include <linux/delay.h>
#include <linux/io.h>
#include <linux/dma-mapping.h>
#include <linux/slab.h>
#include <linux/iopoll.h>

#include "mdss.h"
#include "mdss_dsi.h"

static struct completion dsi_dma_comp;
static int dsi_irq_enabled;
static spinlock_t dsi_irq_lock;
static spinlock_t dsi_mdp_lock;
static int dsi_mdp_busy;

spinlock_t dsi_clk_lock;

struct mdss_hw mdss_dsi_hw = {
	.hw_ndx = MDSS_HW_DSI0,
	.irq_handler = mdss_dsi_isr,
};

void mdss_dsi_init(void)
{
	init_completion(&dsi_dma_comp);
	spin_lock_init(&dsi_irq_lock);
	spin_lock_init(&dsi_mdp_lock);
	spin_lock_init(&dsi_clk_lock);
}

void mdss_dsi_enable_irq(void)
{
	unsigned long flags;

	spin_lock_irqsave(&dsi_irq_lock, flags);
	if (dsi_irq_enabled) {
		pr_debug("%s: IRQ aleady enabled\n", __func__);
		spin_unlock_irqrestore(&dsi_irq_lock, flags);
		return;
	}
	mdss_enable_irq(&mdss_dsi_hw);
	dsi_irq_enabled = 1;
	/*                                          */
	spin_unlock_irqrestore(&dsi_irq_lock, flags);
}

void mdss_dsi_disable_irq(void)
{
	unsigned long flags;

	spin_lock_irqsave(&dsi_irq_lock, flags);
	if (dsi_irq_enabled == 0) {
		pr_debug("%s: IRQ already disabled\n", __func__);
		spin_unlock_irqrestore(&dsi_irq_lock, flags);
		return;
	}
	mdss_disable_irq(&mdss_dsi_hw);
	dsi_irq_enabled = 0;
	/*                                           */
	spin_unlock_irqrestore(&dsi_irq_lock, flags);
}

/*
                                                
                         
 */
void mdss_dsi_disable_irq_nosync(void)
{
	spin_lock(&dsi_irq_lock);
	if (dsi_irq_enabled == 0) {
		pr_debug("%s: IRQ cannot be disabled\n", __func__);
		spin_unlock(&dsi_irq_lock);
		return;
	}

	dsi_irq_enabled = 0;
	spin_unlock(&dsi_irq_lock);
}

/*
                         
 */
char *mdss_dsi_buf_reserve(struct dsi_buf *dp, int len)
{
	dp->data += len;
	return dp->data;
}

char *mdss_dsi_buf_unreserve(struct dsi_buf *dp, int len)
{
	dp->data -= len;
	return dp->data;
}

char *mdss_dsi_buf_push(struct dsi_buf *dp, int len)
{
	dp->data -= len;
	dp->len += len;
	return dp->data;
}

char *mdss_dsi_buf_reserve_hdr(struct dsi_buf *dp, int hlen)
{
	dp->hdr = (u32 *)dp->data;
	return mdss_dsi_buf_reserve(dp, hlen);
}

char *mdss_dsi_buf_init(struct dsi_buf *dp)
{
	int off;

	dp->data = dp->start;
	off = (int)dp->data;
	/*              */
	off &= 0x07;
	if (off)
		off = 8 - off;
	dp->data += off;
	dp->len = 0;
	return dp->data;
}

int mdss_dsi_buf_alloc(struct dsi_buf *dp, int size)
{

	dp->start = kmalloc(size, GFP_KERNEL);
	if (dp->start == NULL) {
		pr_err("%s:%u\n", __func__, __LINE__);
		return -ENOMEM;
	}

	dp->end = dp->start + size;
	dp->size = size;

	if ((int)dp->start & 0x07)
		pr_err("%s: buf NOT 8 bytes aligned\n", __func__);

	dp->data = dp->start;
	dp->len = 0;
	return size;
}

/*
                              
 */
static int mdss_dsi_generic_lwrite(struct dsi_buf *dp, struct dsi_cmd_desc *cm)
{
	char *bp;
	u32 *hp;
	int i, len;

	bp = mdss_dsi_buf_reserve_hdr(dp, DSI_HOST_HDR_SIZE);

	/*                 */
	if (cm->payload) {
		len = cm->dlen;
		len += 3;
		len &= ~0x03;	/*                */
		for (i = 0; i < cm->dlen; i++)
			*bp++ = cm->payload[i];

		/*                        */
		for (; i < len; i++)
			*bp++ = 0xff;

		dp->len += len;
	}

	/*                */
	hp = dp->hdr;
	*hp = 0;
	*hp = DSI_HDR_WC(cm->dlen);
	*hp |= DSI_HDR_VC(cm->vc);
	*hp |= DSI_HDR_LONG_PKT;
	*hp |= DSI_HDR_DTYPE(DTYPE_GEN_LWRITE);
	if (cm->last)
		*hp |= DSI_HDR_LAST;

	mdss_dsi_buf_push(dp, DSI_HOST_HDR_SIZE);

	return dp->len;
}

/*
                                                      
 */
static int mdss_dsi_generic_swrite(struct dsi_buf *dp, struct dsi_cmd_desc *cm)
{
	u32 *hp;
	int len;

	if (cm->dlen && cm->payload == 0) {
		pr_err("%s: NO payload error\n", __func__);
		return 0;
	}

	mdss_dsi_buf_reserve_hdr(dp, DSI_HOST_HDR_SIZE);
	hp = dp->hdr;
	*hp = 0;
	*hp |= DSI_HDR_VC(cm->vc);
	if (cm->last)
		*hp |= DSI_HDR_LAST;


	len = (cm->dlen > 2) ? 2 : cm->dlen;

	if (len == 1) {
		*hp |= DSI_HDR_DTYPE(DTYPE_GEN_WRITE1);
		*hp |= DSI_HDR_DATA1(cm->payload[0]);
		*hp |= DSI_HDR_DATA2(0);
	} else if (len == 2) {
		*hp |= DSI_HDR_DTYPE(DTYPE_GEN_WRITE2);
		*hp |= DSI_HDR_DATA1(cm->payload[0]);
		*hp |= DSI_HDR_DATA2(cm->payload[1]);
	} else {
		*hp |= DSI_HDR_DTYPE(DTYPE_GEN_WRITE);
		*hp |= DSI_HDR_DATA1(0);
		*hp |= DSI_HDR_DATA2(0);
	}

	mdss_dsi_buf_push(dp, DSI_HOST_HDR_SIZE);

	return dp->len;	/*         */
}

/*
                                                
 */
static int mdss_dsi_generic_read(struct dsi_buf *dp, struct dsi_cmd_desc *cm)
{
	u32 *hp;
	int len;

	if (cm->dlen && cm->payload == 0) {
		pr_err("%s: NO payload error\n", __func__);
		return 0;
	}

	mdss_dsi_buf_reserve_hdr(dp, DSI_HOST_HDR_SIZE);
	hp = dp->hdr;
	*hp = 0;
	*hp |= DSI_HDR_VC(cm->vc);
	*hp |= DSI_HDR_BTA;
	if (cm->last)
		*hp |= DSI_HDR_LAST;

	len = (cm->dlen > 2) ? 2 : cm->dlen;

	if (len == 1) {
		*hp |= DSI_HDR_DTYPE(DTYPE_GEN_READ1);
		*hp |= DSI_HDR_DATA1(cm->payload[0]);
		*hp |= DSI_HDR_DATA2(0);
	} else if (len == 2) {
		*hp |= DSI_HDR_DTYPE(DTYPE_GEN_READ2);
		*hp |= DSI_HDR_DATA1(cm->payload[0]);
		*hp |= DSI_HDR_DATA2(cm->payload[1]);
	} else {
		*hp |= DSI_HDR_DTYPE(DTYPE_GEN_READ);
		*hp |= DSI_HDR_DATA1(0);
		*hp |= DSI_HDR_DATA2(0);
	}

	mdss_dsi_buf_push(dp, DSI_HOST_HDR_SIZE);
	return dp->len;	/*         */
}

/*
                          
 */
static int mdss_dsi_dcs_lwrite(struct dsi_buf *dp, struct dsi_cmd_desc *cm)
{
	char *bp;
	u32 *hp;
	int i, len;

	bp = mdss_dsi_buf_reserve_hdr(dp, DSI_HOST_HDR_SIZE);

	/*
                   
                                                     
  */
	if (cm->payload) {
		len = cm->dlen;
		len += 3;
		len &= ~0x03;	/*                */
		for (i = 0; i < cm->dlen; i++)
			*bp++ = cm->payload[i];

		/*                        */
		for (; i < len; i++)
			*bp++ = 0xff;

		dp->len += len;
	}

	/*                */
	hp = dp->hdr;
	*hp = 0;
	*hp = DSI_HDR_WC(cm->dlen);
	*hp |= DSI_HDR_VC(cm->vc);
	*hp |= DSI_HDR_LONG_PKT;
	*hp |= DSI_HDR_DTYPE(DTYPE_DCS_LWRITE);
	if (cm->last)
		*hp |= DSI_HDR_LAST;

	mdss_dsi_buf_push(dp, DSI_HOST_HDR_SIZE);

	return dp->len;
}

/*
                                             
 */
static int mdss_dsi_dcs_swrite(struct dsi_buf *dp, struct dsi_cmd_desc *cm)
{
	u32 *hp;
	int len;

	if (cm->payload == 0) {
		pr_err("%s: NO payload error\n", __func__);
		return -EINVAL;
	}

	mdss_dsi_buf_reserve_hdr(dp, DSI_HOST_HDR_SIZE);
	hp = dp->hdr;
	*hp = 0;
	*hp |= DSI_HDR_VC(cm->vc);
	if (cm->ack)		/*                                      */
		*hp |= DSI_HDR_BTA;
	if (cm->last)
		*hp |= DSI_HDR_LAST;

	len = (cm->dlen > 1) ? 1 : cm->dlen;

	*hp |= DSI_HDR_DTYPE(DTYPE_DCS_WRITE);
	*hp |= DSI_HDR_DATA1(cm->payload[0]);	/*                  */
	*hp |= DSI_HDR_DATA2(0);

	mdss_dsi_buf_push(dp, DSI_HOST_HDR_SIZE);
	return dp->len;
}

/*
                                             
 */
static int mdss_dsi_dcs_swrite1(struct dsi_buf *dp, struct dsi_cmd_desc *cm)
{
	u32 *hp;

	if (cm->dlen < 2 || cm->payload == 0) {
		pr_err("%s: NO payload error\n", __func__);
		return -EINVAL;
	}

	mdss_dsi_buf_reserve_hdr(dp, DSI_HOST_HDR_SIZE);
	hp = dp->hdr;
	*hp = 0;
	*hp |= DSI_HDR_VC(cm->vc);
	if (cm->ack)		/*                                      */
		*hp |= DSI_HDR_BTA;
	if (cm->last)
		*hp |= DSI_HDR_LAST;

	*hp |= DSI_HDR_DTYPE(DTYPE_DCS_WRITE1);
	*hp |= DSI_HDR_DATA1(cm->payload[0]);	/*                  */
	*hp |= DSI_HDR_DATA2(cm->payload[1]);	/*           */

	mdss_dsi_buf_push(dp, DSI_HOST_HDR_SIZE);

	return dp->len;
}
/*
                                      
 */

static int mdss_dsi_dcs_read(struct dsi_buf *dp, struct dsi_cmd_desc *cm)
{
	u32 *hp;

	if (cm->payload == 0) {
		pr_err("%s: NO payload error\n", __func__);
		return -EINVAL;
	}

	mdss_dsi_buf_reserve_hdr(dp, DSI_HOST_HDR_SIZE);
	hp = dp->hdr;
	*hp = 0;
	*hp |= DSI_HDR_VC(cm->vc);
	*hp |= DSI_HDR_BTA;
	*hp |= DSI_HDR_DTYPE(DTYPE_DCS_READ);
	if (cm->last)
		*hp |= DSI_HDR_LAST;

	*hp |= DSI_HDR_DATA1(cm->payload[0]);	/*                  */
	*hp |= DSI_HDR_DATA2(0);

	mdss_dsi_buf_push(dp, DSI_HOST_HDR_SIZE);

	return dp->len;	/*         */
}

static int mdss_dsi_cm_on(struct dsi_buf *dp, struct dsi_cmd_desc *cm)
{
	u32 *hp;

	mdss_dsi_buf_reserve_hdr(dp, DSI_HOST_HDR_SIZE);
	hp = dp->hdr;
	*hp = 0;
	*hp |= DSI_HDR_VC(cm->vc);
	*hp |= DSI_HDR_DTYPE(DTYPE_CM_ON);
	if (cm->last)
		*hp |= DSI_HDR_LAST;

	mdss_dsi_buf_push(dp, DSI_HOST_HDR_SIZE);

	return dp->len;	/*         */
}

static int mdss_dsi_cm_off(struct dsi_buf *dp, struct dsi_cmd_desc *cm)
{
	u32 *hp;

	mdss_dsi_buf_reserve_hdr(dp, DSI_HOST_HDR_SIZE);
	hp = dp->hdr;
	*hp = 0;
	*hp |= DSI_HDR_VC(cm->vc);
	*hp |= DSI_HDR_DTYPE(DTYPE_CM_OFF);
	if (cm->last)
		*hp |= DSI_HDR_LAST;

	mdss_dsi_buf_push(dp, DSI_HOST_HDR_SIZE);

	return dp->len;	/*         */
}

static int mdss_dsi_peripheral_on(struct dsi_buf *dp, struct dsi_cmd_desc *cm)
{
	u32 *hp;

	mdss_dsi_buf_reserve_hdr(dp, DSI_HOST_HDR_SIZE);
	hp = dp->hdr;
	*hp = 0;
	*hp |= DSI_HDR_VC(cm->vc);
	*hp |= DSI_HDR_DTYPE(DTYPE_PERIPHERAL_ON);
	if (cm->last)
		*hp |= DSI_HDR_LAST;

	mdss_dsi_buf_push(dp, DSI_HOST_HDR_SIZE);

	return dp->len;	/*         */
}

static int mdss_dsi_peripheral_off(struct dsi_buf *dp, struct dsi_cmd_desc *cm)
{
	u32 *hp;

	mdss_dsi_buf_reserve_hdr(dp, DSI_HOST_HDR_SIZE);
	hp = dp->hdr;
	*hp = 0;
	*hp |= DSI_HDR_VC(cm->vc);
	*hp |= DSI_HDR_DTYPE(DTYPE_PERIPHERAL_OFF);
	if (cm->last)
		*hp |= DSI_HDR_LAST;

	mdss_dsi_buf_push(dp, DSI_HOST_HDR_SIZE);

	return dp->len;	/*         */
}

static int mdss_dsi_set_max_pktsize(struct dsi_buf *dp, struct dsi_cmd_desc *cm)
{
	u32 *hp;

	if (cm->payload == 0) {
		pr_err("%s: NO payload error\n", __func__);
		return 0;
	}

	mdss_dsi_buf_reserve_hdr(dp, DSI_HOST_HDR_SIZE);
	hp = dp->hdr;
	*hp = 0;
	*hp |= DSI_HDR_VC(cm->vc);
	*hp |= DSI_HDR_DTYPE(DTYPE_MAX_PKTSIZE);
	if (cm->last)
		*hp |= DSI_HDR_LAST;

	*hp |= DSI_HDR_DATA1(cm->payload[0]);
	*hp |= DSI_HDR_DATA2(cm->payload[1]);

	mdss_dsi_buf_push(dp, DSI_HOST_HDR_SIZE);

	return dp->len;	/*         */
}

static int mdss_dsi_null_pkt(struct dsi_buf *dp, struct dsi_cmd_desc *cm)
{
	u32 *hp;

	mdss_dsi_buf_reserve_hdr(dp, DSI_HOST_HDR_SIZE);
	hp = dp->hdr;
	*hp = 0;
	*hp = DSI_HDR_WC(cm->dlen);
	*hp |= DSI_HDR_LONG_PKT;
	*hp |= DSI_HDR_VC(cm->vc);
	*hp |= DSI_HDR_DTYPE(DTYPE_NULL_PKT);
	if (cm->last)
		*hp |= DSI_HDR_LAST;

	mdss_dsi_buf_push(dp, DSI_HOST_HDR_SIZE);

	return dp->len;	/*         */
}

static int mdss_dsi_blank_pkt(struct dsi_buf *dp, struct dsi_cmd_desc *cm)
{
	u32 *hp;

	mdss_dsi_buf_reserve_hdr(dp, DSI_HOST_HDR_SIZE);
	hp = dp->hdr;
	*hp = 0;
	*hp = DSI_HDR_WC(cm->dlen);
	*hp |= DSI_HDR_LONG_PKT;
	*hp |= DSI_HDR_VC(cm->vc);
	*hp |= DSI_HDR_DTYPE(DTYPE_BLANK_PKT);
	if (cm->last)
		*hp |= DSI_HDR_LAST;

	mdss_dsi_buf_push(dp, DSI_HOST_HDR_SIZE);

	return dp->len;	/*         */
}

/*
                                
 */
int mdss_dsi_cmd_dma_add(struct dsi_buf *dp, struct dsi_cmd_desc *cm)
{
	int len = 0;

	switch (cm->dtype) {
	case DTYPE_GEN_WRITE:
	case DTYPE_GEN_WRITE1:
	case DTYPE_GEN_WRITE2:
		len = mdss_dsi_generic_swrite(dp, cm);
		break;
	case DTYPE_GEN_LWRITE:
		len = mdss_dsi_generic_lwrite(dp, cm);
		break;
	case DTYPE_GEN_READ:
	case DTYPE_GEN_READ1:
	case DTYPE_GEN_READ2:
		len = mdss_dsi_generic_read(dp, cm);
		break;
	case DTYPE_DCS_LWRITE:
		len = mdss_dsi_dcs_lwrite(dp, cm);
		break;
	case DTYPE_DCS_WRITE:
		len = mdss_dsi_dcs_swrite(dp, cm);
		break;
	case DTYPE_DCS_WRITE1:
		len = mdss_dsi_dcs_swrite1(dp, cm);
		break;
	case DTYPE_DCS_READ:
		len = mdss_dsi_dcs_read(dp, cm);
		break;
	case DTYPE_MAX_PKTSIZE:
		len = mdss_dsi_set_max_pktsize(dp, cm);
		break;
	case DTYPE_NULL_PKT:
		len = mdss_dsi_null_pkt(dp, cm);
		break;
	case DTYPE_BLANK_PKT:
		len = mdss_dsi_blank_pkt(dp, cm);
		break;
	case DTYPE_CM_ON:
		len = mdss_dsi_cm_on(dp, cm);
		break;
	case DTYPE_CM_OFF:
		len = mdss_dsi_cm_off(dp, cm);
		break;
	case DTYPE_PERIPHERAL_ON:
		len = mdss_dsi_peripheral_on(dp, cm);
		break;
	case DTYPE_PERIPHERAL_OFF:
		len = mdss_dsi_peripheral_off(dp, cm);
		break;
	default:
		pr_debug("%s: dtype=%x NOT supported\n",
					__func__, cm->dtype);
		break;

	}

	return len;
}

/*
                                         
 */
static int mdss_dsi_short_read1_resp(struct dsi_buf *rp)
{
	/*                    */
	rp->data++;
	rp->len = 1;
	return rp->len;
}

/*
                                         
 */
static int mdss_dsi_short_read2_resp(struct dsi_buf *rp)
{
	/*                    */
	rp->data++;
	rp->len = 2;
	return rp->len;
}

static int mdss_dsi_long_read_resp(struct dsi_buf *rp)
{
	short len;

	len = rp->data[2];
	len <<= 8;
	len |= rp->data[1];
	/*                      */
	rp->data += 4;
	rp->len -= 4;
	/*                               */
	rp->len -= 2;
	return len;
}

void mdss_dsi_host_init(struct mipi_panel_info *pinfo,
				struct mdss_panel_data *pdata)
{
	u32 dsi_ctrl, intr_ctrl;
	u32 data;

	pinfo->rgb_swap = DSI_RGB_SWAP_RGB;

	if (pinfo->mode == DSI_VIDEO_MODE) {
		data = 0;
		if (pinfo->pulse_mode_hsa_he)
			data |= BIT(28);
		if (pinfo->hfp_power_stop)
			data |= BIT(24);
		if (pinfo->hbp_power_stop)
			data |= BIT(20);
		if (pinfo->hsa_power_stop)
			data |= BIT(16);
		if (pinfo->eof_bllp_power_stop)
			data |= BIT(15);
		if (pinfo->bllp_power_stop)
			data |= BIT(12);
		data |= ((pinfo->traffic_mode & 0x03) << 8);
		data |= ((pinfo->dst_format & 0x03) << 4); /*        */
		data |= (pinfo->vc & 0x03);
		MIPI_OUTP((pdata->dsi_base) + 0x0010, data);

		data = 0;
		data |= ((pinfo->rgb_swap & 0x07) << 12);
		if (pinfo->b_sel)
			data |= BIT(8);
		if (pinfo->g_sel)
			data |= BIT(4);
		if (pinfo->r_sel)
			data |= BIT(0);
		MIPI_OUTP((pdata->dsi_base) + 0x0020, data);
	} else if (pinfo->mode == DSI_CMD_MODE) {
		data = 0;
		data |= ((pinfo->interleave_max & 0x0f) << 20);
		data |= ((pinfo->rgb_swap & 0x07) << 16);
		if (pinfo->b_sel)
			data |= BIT(12);
		if (pinfo->g_sel)
			data |= BIT(8);
		if (pinfo->r_sel)
			data |= BIT(4);
		data |= (pinfo->dst_format & 0x0f);	/*        */
		MIPI_OUTP((pdata->dsi_base) + 0x003c, data);

		/*                                   */
		data = pinfo->wr_mem_continue & 0x0ff;
		data <<= 8;
		data |= (pinfo->wr_mem_start & 0x0ff);
		if (pinfo->insert_dcs_cmd)
			data |= BIT(16);
		MIPI_OUTP((pdata->dsi_base) + 0x0044, data);
	} else
		pr_err("%s: Unknown DSI mode=%d\n", __func__, pinfo->mode);

	dsi_ctrl = BIT(8) | BIT(2);	/*                         */
	intr_ctrl = 0;
	intr_ctrl = (DSI_INTR_CMD_DMA_DONE_MASK | DSI_INTR_CMD_MDP_DONE_MASK);

	if (pinfo->crc_check)
		dsi_ctrl |= BIT(24);
	if (pinfo->ecc_check)
		dsi_ctrl |= BIT(20);
	if (pinfo->data_lane3)
		dsi_ctrl |= BIT(7);
	if (pinfo->data_lane2)
		dsi_ctrl |= BIT(6);
	if (pinfo->data_lane1)
		dsi_ctrl |= BIT(5);
	if (pinfo->data_lane0)
		dsi_ctrl |= BIT(4);

	/*                                   */
	/*                           */
	MIPI_OUTP((pdata->dsi_base) + 0x3C, 0x14000000);

	data = 0;
	if (pinfo->te_sel)
		data |= BIT(31);
	data |= pinfo->mdp_trigger << 4;/*                 */
	data |= pinfo->dma_trigger;	/*                 */
	data |= (pinfo->stream & 0x01) << 8;
	MIPI_OUTP((pdata->dsi_base) + 0x0084, data); /*               */

	/*                   */
	MIPI_OUTP((pdata->dsi_base) + 0x00b0, pinfo->dlane_swap);

	/*                */
	data = pinfo->t_clk_post & 0x3f;	/*        */
	data <<= 8;
	data |= pinfo->t_clk_pre & 0x3f;	/*         */
	/*                        */
	MIPI_OUTP((pdata->dsi_base) + 0xc4, data);

	data = 0;
	if (pinfo->rx_eot_ignore)
		data |= BIT(4);
	if (pinfo->tx_eot_append)
		data |= BIT(0);
	MIPI_OUTP((pdata->dsi_base) + 0x00cc, data); /*                     */


	/*                                                  */
	/*                   */
	MIPI_OUTP((pdata->dsi_base) + 0x010c, 0x13ff3fe0);

	intr_ctrl |= DSI_INTR_ERROR_MASK;
	MIPI_OUTP((pdata->dsi_base) + 0x0110, intr_ctrl); /*               */

	/*                                          */
	MIPI_OUTP((pdata->dsi_base) + 0x11c, 0x23f); /*              */

	dsi_ctrl |= BIT(0);	/*            */
	MIPI_OUTP((pdata->dsi_base) + 0x0004, dsi_ctrl);

	wmb();
}

void mipi_set_tx_power_mode(int mode, struct mdss_panel_data *pdata)
{
	u32 data = MIPI_INP((pdata->dsi_base) + 0x3c);

	if (mode == 0)
		data &= ~BIT(26);
	else
		data |= BIT(26);

	MIPI_OUTP((pdata->dsi_base) + 0x3c, data);
}

void mdss_dsi_sw_reset(struct mdss_panel_data *pdata)
{
	MIPI_OUTP((pdata->dsi_base) + 0x118, 0x01);
	wmb();
	MIPI_OUTP((pdata->dsi_base) + 0x118, 0x00);
	wmb();
}

void mdss_dsi_controller_cfg(int enable,
			     struct mdss_panel_data *pdata)
{

	u32 dsi_ctrl;
	u32 status;
	u32 sleep_us = 1000;
	u32 timeout_us = 16000;

	/*                             */
	if (readl_poll_timeout(((pdata->dsi_base) + 0x0008),
			   status,
			   ((status & 0x02) == 0),
			       sleep_us, timeout_us))
		pr_info("%s: DSI status=%x failed\n", __func__, status);

	/*                           */
	if (readl_poll_timeout(((pdata->dsi_base) + 0x000c),
			   status,
			   ((status & 0x11111000) == 0x11111000),
			       sleep_us, timeout_us))
		pr_info("%s: FIFO status=%x failed\n", __func__, status);

	dsi_ctrl = MIPI_INP((pdata->dsi_base) + 0x0004);
	if (enable)
		dsi_ctrl |= 0x01;
	else
		dsi_ctrl &= ~0x01;

	MIPI_OUTP((pdata->dsi_base) + 0x0004, dsi_ctrl);
	wmb();
}

void mdss_dsi_op_mode_config(int mode,
			     struct mdss_panel_data *pdata)
{

	u32 dsi_ctrl, intr_ctrl;

	dsi_ctrl = MIPI_INP((pdata->dsi_base) + 0x0004);
	dsi_ctrl &= ~0x07;
	if (mode == DSI_VIDEO_MODE) {
		dsi_ctrl |= 0x03;
		intr_ctrl = DSI_INTR_CMD_DMA_DONE_MASK;
	} else {		/*              */
		dsi_ctrl |= 0x05;
		intr_ctrl = DSI_INTR_CMD_DMA_DONE_MASK | DSI_INTR_ERROR_MASK |
				DSI_INTR_CMD_MDP_DONE_MASK;
	}

	pr_debug("%s: dsi_ctrl=%x intr=%x\n", __func__, dsi_ctrl, intr_ctrl);

	MIPI_OUTP((pdata->dsi_base) + 0x0110, intr_ctrl); /*               */
	MIPI_OUTP((pdata->dsi_base) + 0x0004, dsi_ctrl);
	wmb();
}

void mdss_dsi_cmd_mdp_start(void)
{
	unsigned long flag;

	spin_lock_irqsave(&dsi_mdp_lock, flag);
	mdss_dsi_enable_irq();
	dsi_mdp_busy = true;
	spin_unlock_irqrestore(&dsi_mdp_lock, flag);
}


void mdss_dsi_cmd_bta_sw_trigger(struct mdss_panel_data *pdata)
{
	u32 status;
	int timeout_us = 10000;

	MIPI_OUTP((pdata->dsi_base) + 0x098, 0x01);	/*         */
	wmb();

	/*                             */
	if (readl_poll_timeout(((pdata->dsi_base) + 0x0008),
				status, ((status & 0x0010) == 0),
				0, timeout_us))
		pr_info("%s: DSI status=%x failed\n", __func__, status);

	mdss_dsi_ack_err_status((pdata->dsi_base));

	pr_debug("%s: BTA done, status = %d\n", __func__, status);
}

int mdss_dsi_cmd_reg_tx(u32 data,
			struct mdss_panel_data *pdata)
{
	int i;
	char *bp;

	bp = (char *)&data;
	pr_debug("%s: ", __func__);
	for (i = 0; i < 4; i++)
		pr_debug("%x ", *bp++);

	pr_debug("\n");

	MIPI_OUTP((pdata->dsi_base) + 0x0084, 0x04);/*            */
	MIPI_OUTP((pdata->dsi_base) + 0x0004, 0x135);

	wmb();

	MIPI_OUTP((pdata->dsi_base) + 0x03c, data);
	wmb();
	MIPI_OUTP((pdata->dsi_base) + 0x090, 0x01);	/*         */
	wmb();

	udelay(300);

	return 4;
}

/*
                    
                                                          
 */
int mdss_dsi_cmds_tx(struct mdss_panel_data *pdata,
		struct dsi_buf *tp, struct dsi_cmd_desc *cmds, int cnt)
{
	struct dsi_cmd_desc *cm;
	u32 dsi_ctrl, ctrl;
	int i, video_mode;
	unsigned long flag;

	/*                 
                                             
                                            
               
 */
	dsi_ctrl = MIPI_INP((pdata->dsi_base) + 0x0004);
	video_mode = dsi_ctrl & 0x02; /*               */
	if (video_mode) {
		ctrl = dsi_ctrl | 0x04; /*             */
		MIPI_OUTP((pdata->dsi_base) + 0x0004, ctrl);
	}

	spin_lock_irqsave(&dsi_mdp_lock, flag);
	mdss_dsi_enable_irq();
	dsi_mdp_busy = true;
	spin_unlock_irqrestore(&dsi_mdp_lock, flag);

	cm = cmds;
	mdss_dsi_buf_init(tp);
	for (i = 0; i < cnt; i++) {
		mdss_dsi_buf_init(tp);
		mdss_dsi_cmd_dma_add(tp, cm);
		mdss_dsi_cmd_dma_tx(tp, pdata);
		if (cm->wait)
			msleep(cm->wait);
		cm++;
	}

	spin_lock_irqsave(&dsi_mdp_lock, flag);
	dsi_mdp_busy = false;
	mdss_dsi_disable_irq();
	spin_unlock_irqrestore(&dsi_mdp_lock, flag);

	if (video_mode)
		MIPI_OUTP((pdata->dsi_base) + 0x0004, dsi_ctrl); /*         */

	return cnt;
}

/*                                           */
static char max_pktsize[2] = {0x00, 0x00}; /*                        */

static struct dsi_cmd_desc pkt_size_cmd[] = {
	{DTYPE_MAX_PKTSIZE, 1, 0, 0, 0,
		sizeof(max_pktsize), max_pktsize}
};

/*
                                                             
                                                          
                                                                  
                             
                          
                              
                                                               
                            
  
                                                          
 */
int mdss_dsi_cmds_rx(struct mdss_panel_data *pdata,
			struct dsi_buf *tp, struct dsi_buf *rp,
			struct dsi_cmd_desc *cmds, int rlen)
{
	int cnt, len, diff, pkt_size;
	unsigned long flag;
	char cmd;

	if (pdata->panel_info.mipi.no_max_pkt_size)
		rlen = ALIGN(rlen, 4); /*                         */

	len = rlen;
	diff = 0;

	if (len <= 2)
		cnt = 4;	/*            */
	else {
		if (len > MDSS_DSI_LEN)
			len = MDSS_DSI_LEN;	/*                 */

		len = ALIGN(len, 4); /*                   */
		diff = len - rlen;
		/*
                                             
                                                
                                            
                                     
                                             
   */
		len += 2;
		cnt = len + 6; /*                              */
	}

	spin_lock_irqsave(&dsi_mdp_lock, flag);
	mdss_dsi_enable_irq();
	dsi_mdp_busy = true;
	spin_unlock_irqrestore(&dsi_mdp_lock, flag);

	if (!pdata->panel_info.mipi.no_max_pkt_size) {
		/*                                          */
		pkt_size = len;
		max_pktsize[0] = pkt_size;
		mdss_dsi_buf_init(tp);
		mdss_dsi_cmd_dma_add(tp, pkt_size_cmd);
		mdss_dsi_cmd_dma_tx(tp, pdata);
	}

	mdss_dsi_buf_init(tp);
	mdss_dsi_cmd_dma_add(tp, cmds);

	/*                                 */
	mdss_dsi_cmd_dma_tx(tp, pdata);
	/*
                                         
                                               
                                 
  */
	mdss_dsi_buf_init(rp);
	if (pdata->panel_info.mipi.no_max_pkt_size) {
		/*
                        
                                    
   */
		rp->data += 2;
	}

	mdss_dsi_cmd_dma_rx(rp, cnt, pdata);

	spin_lock_irqsave(&dsi_mdp_lock, flag);
	dsi_mdp_busy = false;
	mdss_dsi_disable_irq();
	spin_unlock_irqrestore(&dsi_mdp_lock, flag);

	if (pdata->panel_info.mipi.no_max_pkt_size) {
		/*
                                       
                                     
                                   
                                 
                                              
   */
		rp->data += 2;
	}

	cmd = rp->data[0];
	switch (cmd) {
	case DTYPE_ACK_ERR_RESP:
		pr_debug("%s: rx ACK_ERR_PACLAGE\n", __func__);
		break;
	case DTYPE_GEN_READ1_RESP:
	case DTYPE_DCS_READ1_RESP:
		mdss_dsi_short_read1_resp(rp);
		break;
	case DTYPE_GEN_READ2_RESP:
	case DTYPE_DCS_READ2_RESP:
		mdss_dsi_short_read2_resp(rp);
		break;
	case DTYPE_GEN_LREAD_RESP:
	case DTYPE_DCS_LREAD_RESP:
		mdss_dsi_long_read_resp(rp);
		rp->len -= 2; /*                     */
		rp->len -= diff; /*             */
		break;
	default:
		break;
	}

	return rp->len;
}

int mdss_dsi_cmd_dma_tx(struct dsi_buf *tp,
			struct mdss_panel_data *pdata)
{
	int len;
	int i;
	char *bp;

	bp = tp->data;

	pr_debug("%s: ", __func__);
	for (i = 0; i < tp->len; i++)
		pr_debug("%x ", *bp++);

	pr_debug("\n");

	len = tp->len;
	len += 3;
	len &= ~0x03;	/*                */

	tp->dmap = dma_map_single(&dsi_dev, tp->data, len, DMA_TO_DEVICE);
	if (dma_mapping_error(&dsi_dev, tp->dmap))
		pr_err("%s: dmap mapp failed\n", __func__);

	INIT_COMPLETION(dsi_dma_comp);

	MIPI_OUTP((pdata->dsi_base) + 0x048, tp->dmap);
	MIPI_OUTP((pdata->dsi_base) + 0x04c, len);
	wmb();

	MIPI_OUTP((pdata->dsi_base) + 0x090, 0x01);	/*         */
	wmb();

	wait_for_completion(&dsi_dma_comp);

	dma_unmap_single(&dsi_dev, tp->dmap, len, DMA_TO_DEVICE);
	tp->dmap = 0;
	return tp->len;
}

int mdss_dsi_cmd_dma_rx(struct dsi_buf *rp, int rlen,
			struct mdss_panel_data *pdata)
{
	u32 *lp, data;
	int i, off, cnt;

	lp = (u32 *)rp->data;
	cnt = rlen;
	cnt += 3;
	cnt >>= 2;

	if (cnt > 4)
		cnt = 4; /*                            */

	off = 0x06c;	/*                */
	off += ((cnt - 1) * 4);


	for (i = 0; i < cnt; i++) {
		data = (u32)MIPI_INP((pdata->dsi_base) + off);
		*lp++ = ntohl(data);	/*                       */
		off -= 4;
		rp->len += sizeof(*lp);
	}

	return rlen;
}

void mdss_dsi_ack_err_status(unsigned char *dsi_base)
{
	u32 status;

	status = MIPI_INP(dsi_base + 0x0068);/*                    */

	if (status) {
		MIPI_OUTP(dsi_base + 0x0068, status);
		pr_debug("%s: status=%x\n", __func__, status);
	}
}

void mdss_dsi_timeout_status(unsigned char *dsi_base)
{
	u32 status;

	status = MIPI_INP(dsi_base + 0x00c0);/*                    */
	if (status & 0x0111) {
		MIPI_OUTP(dsi_base + 0x00c0, status);
		pr_debug("%s: status=%x\n", __func__, status);
	}
}

void mdss_dsi_dln0_phy_err(unsigned char *dsi_base)
{
	u32 status;

	status = MIPI_INP(dsi_base + 0x00b4);/*                  */

	if (status & 0x011111) {
		MIPI_OUTP(dsi_base + 0x00b4, status);
		pr_debug("%s: status=%x\n", __func__, status);
	}
}

void mdss_dsi_fifo_status(unsigned char *dsi_base)
{
	u32 status;

	status = MIPI_INP(dsi_base + 0x000c);/*                 */

	if (status & 0x44444489) {
		MIPI_OUTP(dsi_base + 0x000c, status);
		pr_debug("%s: status=%x\n", __func__, status);
	}
}

void mdss_dsi_status(unsigned char *dsi_base)
{
	u32 status;

	status = MIPI_INP(dsi_base + 0x0008);/*            */

	if (status & 0x80000000) {
		MIPI_OUTP(dsi_base + 0x0008, status);
		pr_debug("%s: status=%x\n", __func__, status);
	}
}

void mdss_dsi_error(unsigned char *dsi_base)
{
	/*                   */
	mdss_dsi_ack_err_status(dsi_base);	/*              */
	mdss_dsi_timeout_status(dsi_base);	/*              */
	mdss_dsi_fifo_status(dsi_base);		/*                 */
	mdss_dsi_status(dsi_base);		/*                */
	mdss_dsi_dln0_phy_err(dsi_base);	/*                  */
}


irqreturn_t mdss_dsi_isr(int irq, void *ptr)
{
	u32 isr;
	unsigned char *dsi_base;

	dsi_base = mdss_dsi_get_base_adr();
	if (!dsi_base)
		pr_err("%s:%d DSI base adr no Initialized",
				       __func__, __LINE__);

	isr = MIPI_INP(dsi_base + 0x0110);/*               */
	MIPI_OUTP(dsi_base + 0x0110, isr);

	if (isr & DSI_INTR_ERROR)
		mdss_dsi_error(dsi_base);

	if (isr & DSI_INTR_VIDEO_DONE) {
		/*
                      
  */
	}

	if (isr & DSI_INTR_CMD_DMA_DONE)
		complete(&dsi_dma_comp);

	if (isr & DSI_INTR_CMD_MDP_DONE) {
		spin_lock(&dsi_mdp_lock);
		dsi_mdp_busy = false;
		mdss_dsi_disable_irq_nosync();
		spin_unlock(&dsi_mdp_lock);
	}

	return IRQ_HANDLED;
}
