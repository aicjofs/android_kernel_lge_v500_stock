/* Copyright (c) 2009-2010, Code Aurora Forum. All rights reserved.
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
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 *
 */


//                                               
#include "msm_fb.h"
#include "mipi_dsi.h"
#include "mipi_lgit_fhd.h"
#include <mach/board_lge.h>


static struct msm_panel_info pinfo;

#define DSI_BIT_CLK_900MHZ

static struct mipi_dsi_phy_ctrl dsi_video_mode_phy_db = {
       /*                                             */
#if defined(DSI_BIT_CLK_900MHZ)
       /*           */
	{0x03, 0x0a, 0x04, 0x00, 0x20},
       /*        */
	{0xEB, 0x35, 0x21, 0x00, 0x59, 0x63, 0x27, 0x39, 0x42, 0x03, 0x04},
       /*          */
       {0x5f, 0x00, 0x00, 0x10},
       /*          */
       {0xff, 0x00, 0x06, 0x00},
	/*             */
#if defined(CONFIG_MACH_APQ8064_GVDCM) || defined(CONFIG_MACH_APQ8064_GVKT)
	{0x00, 0xC6, 0x01, 0x1A, 0x00, 0x50, 0x48, 0x63,
	 0x40, 0x07, 0x01, 0x00, 0x14, 0x03, 0x00, 0x02,
	 0x00, 0x20, 0x00, 0x01},
#elif defined(CONFIG_MACH_APQ8064_GKATT) || defined(CONFIG_MACH_APQ8064_GKGLOBAL)
	{0x00, 0xDF, 0x01, 0x1A, 0x00, 0x50, 0x48, 0x63,
	 0x40, 0x07, 0x01, 0x00, 0x14, 0x03, 0x00, 0x02,
	 0x00, 0x20, 0x00, 0x01},

#elif defined(CONFIG_MACH_APQ8064_GKU) || defined(CONFIG_MACH_APQ8064_GKKT)    \
    || defined(CONFIG_MACH_APQ8064_GKSK) || defined(CONFIG_MACH_APQ8064_GVAR_CMCC)
	{0x00, 0xEA, 0x01, 0x1A, 0x00, 0x50, 0x48, 0x63,
	 0x40, 0x07, 0x01, 0x00, 0x14, 0x03, 0x00, 0x02,
	 0x00, 0x20, 0x00, 0x01},
#endif
#endif
};

#if defined(CONFIG_MACH_APQ8064_GKU) || defined(CONFIG_MACH_APQ8064_GKKT) || defined(CONFIG_MACH_APQ8064_GKSK) 
static struct mipi_dsi_phy_ctrl dsi_video_mode_phy_db_domestic_rev_c = {
	/*                                             */
#if defined(DSI_BIT_CLK_900MHZ)
       /*           */
	{0x03, 0x0a, 0x04, 0x00, 0x20},
       /*        */
	{0xEB, 0x35, 0x21, 0x00, 0x59, 0x63, 0x27, 0x39, 0x42, 0x03, 0x04},
       /*          */
	{0x5f, 0x00, 0x00, 0x10},
       /*          */
	{0xff, 0x00, 0x06, 0x00},
	/*             */
	{0x00, 0xB8, 0x01, 0x1A, 0x00, 0x50, 0x48, 0x63,
	 0x40, 0x07, 0x01, 0x00, 0x14, 0x03, 0x00, 0x02,
	 0x00, 0x20, 0x00, 0x01},
#endif
};
#endif




static int __init mipi_video_lgit_fhd_pt_init(void)
{
	int ret;

#ifdef CONFIG_FB_MSM_MIPI_PANEL_DETECT
	if (msm_fb_detect_client("mipi_video_lgit_fhd"))
		return 0;
#endif
	pinfo.xres = 1080;
	pinfo.yres = 1920;
	pinfo.type = MIPI_VIDEO_PANEL;
	pinfo.pdest = DISPLAY_1;
	pinfo.wait_cycle = 0;
	pinfo.bpp = 24;
#if defined(CONFIG_MACH_APQ8064_GVDCM) || defined(CONFIG_MACH_APQ8064_GVKT)
	pinfo.lcdc.h_back_porch = 105;
	pinfo.lcdc.h_front_porch = 120;
#elif defined(CONFIG_MACH_APQ8064_GKATT) || defined(CONFIG_MACH_APQ8064_GKGLOBAL)
	pinfo.lcdc.h_back_porch = 144;		/*     */
	pinfo.lcdc.h_front_porch = 152;		/*    */

#elif defined(CONFIG_MACH_APQ8064_GKU) || defined(CONFIG_MACH_APQ8064_GKKT) || defined(CONFIG_MACH_APQ8064_GKSK) 
       if (lge_get_board_revno() == HW_REV_C) {
              pinfo.lcdc.h_back_porch = 85;		/*     */
              pinfo.lcdc.h_front_porch = 100;		/*    */
	} else {
              pinfo.lcdc.h_back_porch = 150;
              pinfo.lcdc.h_front_porch = 179;
	}
#elif defined(CONFIG_MACH_APQ8064_GVAR_CMCC)
  pinfo.lcdc.h_back_porch = 150;
  pinfo.lcdc.h_front_porch = 179;
#endif
	pinfo.lcdc.h_pulse_width = 4;
	pinfo.lcdc.v_back_porch = 9;
	pinfo.lcdc.v_front_porch = 3;
	pinfo.lcdc.v_pulse_width = 1;
	pinfo.lcdc.border_clr = 0;         /*     */
	pinfo.lcdc.underflow_clr = 0x00;   /*       */
	pinfo.lcdc.hsync_skew = 0;
	pinfo.bl_max = 0xFF;
	pinfo.bl_min = 0;
	pinfo.fb_num = 2;

	pinfo.mipi.mode = DSI_VIDEO_MODE;
	pinfo.mipi.pulse_mode_hsa_he = FALSE;

/*                                                           */

	pinfo.mipi.hfp_power_stop = FALSE;//                       
	pinfo.mipi.hbp_power_stop = FALSE;//                       
	pinfo.mipi.hsa_power_stop = FALSE;//              

	pinfo.mipi.esc_byte_ratio = 6;
	pinfo.mipi.eof_bllp_power_stop = TRUE; //              
	pinfo.mipi.bllp_power_stop = TRUE;
	pinfo.mipi.traffic_mode = DSI_NON_BURST_SYNCH_EVENT;
 	pinfo.mipi.dst_format = DSI_VIDEO_DST_FORMAT_RGB888;
	pinfo.mipi.vc = 0;
	pinfo.mipi.rgb_swap = DSI_RGB_SWAP_RGB;
	pinfo.mipi.data_lane0 = TRUE;
	pinfo.mipi.data_lane1 = TRUE;
	pinfo.mipi.data_lane2 = TRUE;
	pinfo.mipi.data_lane3 = TRUE;
#if defined(DSI_BIT_CLK_900MHZ)
	pinfo.mipi.t_clk_post = 0x21;
	pinfo.mipi.t_clk_pre = 0x3C;
#if defined(CONFIG_MACH_APQ8064_GVDCM) || defined(CONFIG_MACH_APQ8064_GVKT)
	pinfo.clk_rate = 910000000;
#elif defined(CONFIG_MACH_APQ8064_GKATT) || defined(CONFIG_MACH_APQ8064_GKGLOBAL)
	pinfo.clk_rate = 960000000;
#elif defined(CONFIG_MACH_APQ8064_GKU) || defined(CONFIG_MACH_APQ8064_GKKT) || defined(CONFIG_MACH_APQ8064_GKSK) 
       if (lge_get_board_revno() == HW_REV_C)
              pinfo.clk_rate = 884000000;
       else
              pinfo.clk_rate = 983000000;
#elif defined(CONFIG_MACH_APQ8064_GVAR_CMCC)
  pinfo.clk_rate = 983000000;
#endif
	pinfo.mipi.frame_rate = 60;	
#endif
	pinfo.mipi.stream = 0;		/*       */
	pinfo.mipi.mdp_trigger = 0;        /*                    */
	pinfo.mipi.dma_trigger = DSI_CMD_TRIGGER_SW;


#if defined(CONFIG_MACH_APQ8064_GKU) || defined(CONFIG_MACH_APQ8064_GKKT) || defined(CONFIG_MACH_APQ8064_GKSK) 
       if (lge_get_board_revno() == HW_REV_C)
              pinfo.mipi.dsi_phy_db = &dsi_video_mode_phy_db_domestic_rev_c;
       else
              pinfo.mipi.dsi_phy_db = &dsi_video_mode_phy_db;
#else
       pinfo.mipi.dsi_phy_db = &dsi_video_mode_phy_db;
#endif

	ret = mipi_lgit_device_register(&pinfo, MIPI_DSI_PRIM, MIPI_DSI_PANEL_FHD);
	if (ret)
		printk(KERN_ERR "%s: failed to register device!\n", __func__);

	return ret;
}

module_init(mipi_video_lgit_fhd_pt_init);
