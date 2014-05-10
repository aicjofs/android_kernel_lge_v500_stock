/* Copyright (c) 2008-2012, The Linux Foundation. All rights reserved.
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

#ifndef MDSS_PANEL_H
#define MDSS_PANEL_H

#include <linux/platform_device.h>
#include <linux/types.h>

/*               */
struct panel_id {
	u16 id;
	u16 type;
};

/*                 */
#define NO_PANEL		0xffff	/*          */
#define MDDI_PANEL		1	/*      */
#define EBI2_PANEL		2	/*      */
#define LCDC_PANEL		3	/*                    */
#define EXT_MDDI_PANEL		4	/*          */
#define TV_PANEL		5	/*    */
#define HDMI_PANEL		6	/*         */
#define DTV_PANEL		7	/*     */
#define MIPI_VIDEO_PANEL	8	/*      */
#define MIPI_CMD_PANEL		9	/*      */
#define WRITEBACK_PANEL		10	/*              */
#define LVDS_PANEL		11	/*      */
#define EDP_PANEL		12	/*      */

/*             */
enum {
	DISPLAY_LCD = 0,	/*                 */
	DISPLAY_LCDC,		/*      */
	DISPLAY_TV,		/*        */
	DISPLAY_EXT_MDDI,	/*               */
	DISPLAY_WRITEBACK,
};

/*                       */
enum {
	DISPLAY_1 = 0,		/*                          */
	DISPLAY_2,		/*                           */
	DISPLAY_3,              /*                                    */
	MAX_PHYS_TARGET_NUM,
};

/*                 */
struct lcd_panel_info {
	u32 vsync_enable;
	u32 refx100;
	u32 v_back_porch;
	u32 v_front_porch;
	u32 v_pulse_width;
	u32 hw_vsync_mode;
	u32 vsync_notifier_period;
	u32 rev;
};

struct lcdc_panel_info {
	u32 h_back_porch;
	u32 h_front_porch;
	u32 h_pulse_width;
	u32 v_back_porch;
	u32 v_front_porch;
	u32 v_pulse_width;
	u32 border_clr;
	u32 underflow_clr;
	u32 hsync_skew;
	/*           */
	u32 xres_pad;
	/*            */
	u32 yres_pad;
};

struct mipi_panel_info {
	char mode;		/*           */
	char interleave_mode;
	char crc_check;
	char ecc_check;
	char dst_format;	/*                             */
	char data_lane0;
	char data_lane1;
	char data_lane2;
	char data_lane3;
	char dlane_swap;	/*                */
	char rgb_swap;
	char b_sel;
	char g_sel;
	char r_sel;
	char rx_eot_ignore;
	char tx_eot_append;
	char t_clk_post; /*                              */
	char t_clk_pre;  /*                              */
	char vc;	/*                 */
	struct mipi_dsi_phy_ctrl *dsi_phy_db;
	/*            */
	char pulse_mode_hsa_he;
	char hfp_power_stop;
	char hbp_power_stop;
	char hsa_power_stop;
	char eof_bllp_power_stop;
	char bllp_power_stop;
	char traffic_mode;
	char frame_rate;
	/*              */
	char interleave_max;
	char insert_dcs_cmd;
	char wr_mem_continue;
	char wr_mem_start;
	char te_sel;
	char stream;	/*        */
	char mdp_trigger;
	char dma_trigger;
	u32 dsi_pclk_rate;
	/*                                        */
	char no_max_pkt_size;
	/*                                   */
	char force_clk_lane_hs;
};

enum lvds_mode {
	LVDS_SINGLE_CHANNEL_MODE,
	LVDS_DUAL_CHANNEL_MODE,
};

struct lvds_panel_info {
	enum lvds_mode channel_mode;
	/*                           */
	char channel_swap;
};

struct mdss_panel_info {
	u32 xres;
	u32 yres;
	u32 bpp;
	u32 type;
	u32 wait_cycle;
	u32 pdest;
	u32 bl_max;
	u32 bl_min;
	u32 fb_num;
	u32 clk_rate;
	u32 clk_min;
	u32 clk_max;
	u32 frame_count;
	u32 is_3d_panel;
	u32 out_format;

	struct lcd_panel_info lcd;
	struct lcdc_panel_info lcdc;
	struct mipi_panel_info mipi;
	struct lvds_panel_info lvds;
	struct fb_info *fbi;
};

struct mdss_panel_data {
	struct mdss_panel_info panel_info;
	void (*set_backlight) (u32 bl_level);
	unsigned char *dsi_base;

	/*                      */
	int (*on) (struct mdss_panel_data *pdata);
	int (*off) (struct mdss_panel_data *pdata);
};

int mdss_register_panel(struct mdss_panel_data *pdata);
#endif /*              */
