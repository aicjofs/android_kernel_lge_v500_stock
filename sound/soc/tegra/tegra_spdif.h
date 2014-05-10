/*
 * tegra_spdif.h - Definitions for Tegra SPDIF driver
 *
 * Author: Stephen Warren <swarren@nvidia.com>
 * Copyright (C) 2011 - NVIDIA, Inc.
 *
 * Based on code copyright/by:
 * Copyright (c) 2008-2009, NVIDIA Corporation
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
 * 02110-1301 USA
 *
 */

#ifndef __TEGRA_SPDIF_H__
#define __TEGRA_SPDIF_H__

#include "tegra_pcm.h"

/*                               */

#define TEGRA_SPDIF_CTRL					0x0
#define TEGRA_SPDIF_STATUS					0x4
#define TEGRA_SPDIF_STROBE_CTRL					0x8
#define TEGRA_SPDIF_DATA_FIFO_CSR				0x0C
#define TEGRA_SPDIF_DATA_OUT					0x40
#define TEGRA_SPDIF_DATA_IN					0x80
#define TEGRA_SPDIF_CH_STA_RX_A					0x100
#define TEGRA_SPDIF_CH_STA_RX_B					0x104
#define TEGRA_SPDIF_CH_STA_RX_C					0x108
#define TEGRA_SPDIF_CH_STA_RX_D					0x10C
#define TEGRA_SPDIF_CH_STA_RX_E					0x110
#define TEGRA_SPDIF_CH_STA_RX_F					0x114
#define TEGRA_SPDIF_CH_STA_TX_A					0x140
#define TEGRA_SPDIF_CH_STA_TX_B					0x144
#define TEGRA_SPDIF_CH_STA_TX_C					0x148
#define TEGRA_SPDIF_CH_STA_TX_D					0x14C
#define TEGRA_SPDIF_CH_STA_TX_E					0x150
#define TEGRA_SPDIF_CH_STA_TX_F					0x154
#define TEGRA_SPDIF_USR_STA_RX_A				0x180
#define TEGRA_SPDIF_USR_DAT_TX_A				0x1C0

/*                            */

/*                                              */
#define TEGRA_SPDIF_CTRL_CAP_LC					(1 << 30)

/*                           */
#define TEGRA_SPDIF_CTRL_RX_EN					(1 << 29)

/*                              */
#define TEGRA_SPDIF_CTRL_TX_EN					(1 << 28)

/*                         */
#define TEGRA_SPDIF_CTRL_TC_EN					(1 << 27)

/*                    */
#define TEGRA_SPDIF_CTRL_TU_EN					(1 << 26)

/*                             */
#define TEGRA_SPDIF_CTRL_IE_TXE					(1 << 25)

/*                            */
#define TEGRA_SPDIF_CTRL_IE_RXE					(1 << 24)

/*                               */
#define TEGRA_SPDIF_CTRL_IE_P					(1 << 23)

/*                           */
#define TEGRA_SPDIF_CTRL_IE_B					(1 << 22)

/*                                                 */
#define TEGRA_SPDIF_CTRL_IE_C					(1 << 21)

/*                                                          */
#define TEGRA_SPDIF_CTRL_IE_U					(1 << 20)

/*                                                        */
#define TEGRA_SPDIF_CTRL_QE_RU					(1 << 19)

/*                                                        */
#define TEGRA_SPDIF_CTRL_QE_TU					(1 << 18)

/*                                                        */
#define TEGRA_SPDIF_CTRL_QE_RX					(1 << 17)

/*                                                        */
#define TEGRA_SPDIF_CTRL_QE_TX					(1 << 16)

/*                           */
#define TEGRA_SPDIF_CTRL_LBK_EN					(1 << 15)

/*
                  
                                                           
                                
                                                           
 */
#define TEGRA_SPDIF_CTRL_PACK					(1 << 14)

/*
                  
                  
                  
                
 */
#define TEGRA_SPDIF_BIT_MODE_16BIT				0
#define TEGRA_SPDIF_BIT_MODE_20BIT				1
#define TEGRA_SPDIF_BIT_MODE_24BIT				2
#define TEGRA_SPDIF_BIT_MODE_RAW				3

#define TEGRA_SPDIF_CTRL_BIT_MODE_SHIFT				12
#define TEGRA_SPDIF_CTRL_BIT_MODE_MASK				(3                          << TEGRA_SPDIF_CTRL_BIT_MODE_SHIFT)
#define TEGRA_SPDIF_CTRL_BIT_MODE_16BIT				(TEGRA_SPDIF_BIT_MODE_16BIT << TEGRA_SPDIF_CTRL_BIT_MODE_SHIFT)
#define TEGRA_SPDIF_CTRL_BIT_MODE_20BIT				(TEGRA_SPDIF_BIT_MODE_20BIT << TEGRA_SPDIF_CTRL_BIT_MODE_SHIFT)
#define TEGRA_SPDIF_CTRL_BIT_MODE_24BIT				(TEGRA_SPDIF_BIT_MODE_24BIT << TEGRA_SPDIF_CTRL_BIT_MODE_SHIFT)
#define TEGRA_SPDIF_CTRL_BIT_MODE_RAW				(TEGRA_SPDIF_BIT_MODE_RAW   << TEGRA_SPDIF_CTRL_BIT_MODE_SHIFT)

/*                              */

/*
                                                                  
                                                                   
 */

/*
                                               
                                                               
                                                                   
                          
                                                                   
                                              
 */
#define TEGRA_SPDIF_STATUS_RX_BSY				(1 << 29)

/*
                                                     
                                               
                                                                  
                       
 */
#define TEGRA_SPDIF_STATUS_TX_BSY				(1 << 28)

/*
                                          
                                                                  
                                                                      
                                      
                                                                  
                                                                
 */
#define TEGRA_SPDIF_STATUS_TC_BSY				(1 << 27)

/*
                          
                                                              
                                                                       
                                                                  
                                                
 */
#define TEGRA_SPDIF_STATUS_TU_BSY				(1 << 26)

/*                               */
#define TEGRA_SPDIF_STATUS_TX_ERR				(1 << 25)

/*                              */
#define TEGRA_SPDIF_STATUS_RX_ERR				(1 << 24)

/*                                                        */
#define TEGRA_SPDIF_STATUS_IS_P					(1 << 23)

/*                                                                    */
#define TEGRA_SPDIF_STATUS_IS_B					(1 << 22)

/*
                                        
                                   
                                             
 */
#define TEGRA_SPDIF_STATUS_IS_C					(1 << 21)

/*                                                                    */
#define TEGRA_SPDIF_STATUS_IS_U					(1 << 20)

/*
                       
                                                            
 */
#define TEGRA_SPDIF_STATUS_QS_RU				(1 << 19)

/*
                       
                                                            
 */
#define TEGRA_SPDIF_STATUS_QS_TU				(1 << 18)

/*
                       
                                                            
 */
#define TEGRA_SPDIF_STATUS_QS_RX				(1 << 17)

/*
                       
                                                            
 */
#define TEGRA_SPDIF_STATUS_QS_TX				(1 << 16)

/*                                   */

/*
                                                                       
                   
 */
#define TEGRA_SPDIF_STROBE_CTRL_PERIOD_SHIFT			16
#define TEGRA_SPDIF_STROBE_CTRL_PERIOD_MASK			(0xff << TEGRA_SPDIF_STROBE_CTRL_PERIOD_SHIFT)

/*                                                 */
#define TEGRA_SPDIF_STROBE_CTRL_STROBE				(1 << 15)

/*
                                                                        
                                       
 */
#define TEGRA_SPDIF_STROBE_CTRL_DATA_STROBES_SHIFT		8
#define TEGRA_SPDIF_STROBE_CTRL_DATA_STROBES_MASK		(0x1f << TEGRA_SPDIF_STROBE_CTRL_DATA_STROBES_SHIFT)

/*
                                                                  
                         
 */
#define TEGRA_SPDIF_STROBE_CTRL_CLOCK_PERIOD_SHIFT		0
#define TEGRA_SPDIF_STROBE_CTRL_CLOCK_PERIOD_MASK		(0x3f << TEGRA_SPDIF_STROBE_CTRL_CLOCK_PERIOD_SHIFT)

/*                               */

/*                                        */
#define TEGRA_SPDIF_DATA_FIFO_CSR_RU_CLR			(1 << 31)

#define TEGRA_SPDIF_FIFO_ATN_LVL_U_ONE_SLOT			0
#define TEGRA_SPDIF_FIFO_ATN_LVL_U_TWO_SLOTS			1
#define TEGRA_SPDIF_FIFO_ATN_LVL_U_THREE_SLOTS			2
#define TEGRA_SPDIF_FIFO_ATN_LVL_U_FOUR_SLOTS			3

/*                         */
#define TEGRA_SPDIF_DATA_FIFO_CSR_RU_ATN_LVL_SHIFT		29
#define TEGRA_SPDIF_DATA_FIFO_CSR_RU_ATN_LVL_MASK		\
		(0x3                                    << TEGRA_SPDIF_DATA_FIFO_CSR_RU_ATN_LVL_SHIFT)
#define TEGRA_SPDIF_DATA_FIFO_CSR_RU_ATN_LVL_RU1_WORD_FULL	\
		(TEGRA_SPDIF_FIFO_ATN_LVL_U_ONE_SLOT    << TEGRA_SPDIF_DATA_FIFO_CSR_RU_ATN_LVL_SHIFT)
#define TEGRA_SPDIF_DATA_FIFO_CSR_RU_ATN_LVL_RU2_WORD_FULL	\
		(TEGRA_SPDIF_FIFO_ATN_LVL_U_TWO_SLOTS   << TEGRA_SPDIF_DATA_FIFO_CSR_RU_ATN_LVL_SHIFT)
#define TEGRA_SPDIF_DATA_FIFO_CSR_RU_ATN_LVL_RU3_WORD_FULL	\
		(TEGRA_SPDIF_FIFO_ATN_LVL_U_THREE_SLOTS << TEGRA_SPDIF_DATA_FIFO_CSR_RU_ATN_LVL_SHIFT)
#define TEGRA_SPDIF_DATA_FIFO_CSR_RU_ATN_LVL_RU4_WORD_FULL	\
		(TEGRA_SPDIF_FIFO_ATN_LVL_U_FOUR_SLOTS  << TEGRA_SPDIF_DATA_FIFO_CSR_RU_ATN_LVL_SHIFT)

/*                                               */
#define TEGRA_SPDIF_DATA_FIFO_CSR_RU_FULL_COUNT_SHIFT		24
#define TEGRA_SPDIF_DATA_FIFO_CSR_RU_FULL_COUNT_MASK		(0x1f << TEGRA_SPDIF_DATA_FIFO_CSR_RU_FULL_COUNT_SHIFT)

/*                                           */
#define TEGRA_SPDIF_DATA_FIFO_CSR_TU_CLR			(1 << 23)

/*                         */
#define TEGRA_SPDIF_DATA_FIFO_CSR_TU_ATN_LVL_SHIFT		21
#define TEGRA_SPDIF_DATA_FIFO_CSR_TU_ATN_LVL_MASK		\
		(0x3                                   << TEGRA_SPDIF_DATA_FIFO_CSR_TU_ATN_LVL_SHIFT)
#define TEGRA_SPDIF_DATA_FIFO_CSR_TU_ATN_LVL_TU1_WORD_FULL	\
		(TEGRA_SPDIF_FIFO_ATN_LVL_U_ONE_SLOT    << TEGRA_SPDIF_DATA_FIFO_CSR_TU_ATN_LVL_SHIFT)
#define TEGRA_SPDIF_DATA_FIFO_CSR_TU_ATN_LVL_TU2_WORD_FULL	\
		(TEGRA_SPDIF_FIFO_ATN_LVL_U_TWO_SLOTS   << TEGRA_SPDIF_DATA_FIFO_CSR_TU_ATN_LVL_SHIFT)
#define TEGRA_SPDIF_DATA_FIFO_CSR_TU_ATN_LVL_TU3_WORD_FULL	\
		(TEGRA_SPDIF_FIFO_ATN_LVL_U_THREE_SLOTS << TEGRA_SPDIF_DATA_FIFO_CSR_TU_ATN_LVL_SHIFT)
#define TEGRA_SPDIF_DATA_FIFO_CSR_TU_ATN_LVL_TU4_WORD_FULL	\
		(TEGRA_SPDIF_FIFO_ATN_LVL_U_FOUR_SLOTS  << TEGRA_SPDIF_DATA_FIFO_CSR_TU_ATN_LVL_SHIFT)

/*                                                    */
#define TEGRA_SPDIF_DATA_FIFO_CSR_TU_EMPTY_COUNT_SHIFT		16
#define TEGRA_SPDIF_DATA_FIFO_CSR_TU_EMPTY_COUNT_MASK		(0x1f << SPDIF_DATA_FIFO_CSR_TU_EMPTY_COUNT_SHIFT)

/*                                         */
#define TEGRA_SPDIF_DATA_FIFO_CSR_RX_CLR			(1 << 15)

#define TEGRA_SPDIF_FIFO_ATN_LVL_D_ONE_SLOT			0
#define TEGRA_SPDIF_FIFO_ATN_LVL_D_FOUR_SLOTS			1
#define TEGRA_SPDIF_FIFO_ATN_LVL_D_EIGHT_SLOTS			2
#define TEGRA_SPDIF_FIFO_ATN_LVL_D_TWELVE_SLOTS			3

/*                         */
#define TEGRA_SPDIF_DATA_FIFO_CSR_RX_ATN_LVL_SHIFT		13
#define TEGRA_SPDIF_DATA_FIFO_CSR_RX_ATN_LVL_MASK		\
		(0x3                                     << TEGRA_SPDIF_DATA_FIFO_CSR_RX_ATN_LVL_SHIFT)
#define TEGRA_SPDIF_DATA_FIFO_CSR_RX_ATN_LVL_RU1_WORD_FULL	\
		(TEGRA_SPDIF_FIFO_ATN_LVL_D_ONE_SLOT     << TEGRA_SPDIF_DATA_FIFO_CSR_RX_ATN_LVL_SHIFT)
#define TEGRA_SPDIF_DATA_FIFO_CSR_RX_ATN_LVL_RU4_WORD_FULL	\
		(TEGRA_SPDIF_FIFO_ATN_LVL_D_FOUR_SLOTS   << TEGRA_SPDIF_DATA_FIFO_CSR_RX_ATN_LVL_SHIFT)
#define TEGRA_SPDIF_DATA_FIFO_CSR_RX_ATN_LVL_RU8_WORD_FULL	\
		(TEGRA_SPDIF_FIFO_ATN_LVL_D_EIGHT_SLOTS  << TEGRA_SPDIF_DATA_FIFO_CSR_RX_ATN_LVL_SHIFT)
#define TEGRA_SPDIF_DATA_FIFO_CSR_RX_ATN_LVL_RU12_WORD_FULL	\
		(TEGRA_SPDIF_FIFO_ATN_LVL_D_TWELVE_SLOTS << TEGRA_SPDIF_DATA_FIFO_CSR_RX_ATN_LVL_SHIFT)

/*                                                */
#define TEGRA_SPDIF_DATA_FIFO_CSR_RX_FULL_COUNT_SHIFT		8
#define TEGRA_SPDIF_DATA_FIFO_CSR_RX_FULL_COUNT_MASK		(0x1f << TEGRA_SPDIF_DATA_FIFO_CSR_RX_FULL_COUNT_SHIFT)

/*                                            */
#define TEGRA_SPDIF_DATA_FIFO_CSR_TX_CLR			(1 << 7)

/*                         */
#define TEGRA_SPDIF_DATA_FIFO_CSR_TX_ATN_LVL_SHIFT		5
#define TEGRA_SPDIF_DATA_FIFO_CSR_TX_ATN_LVL_MASK		\
		(0x3                                     << TEGRA_SPDIF_DATA_FIFO_CSR_TX_ATN_LVL_SHIFT)
#define TEGRA_SPDIF_DATA_FIFO_CSR_TX_ATN_LVL_TU1_WORD_FULL	\
		(TEGRA_SPDIF_FIFO_ATN_LVL_D_ONE_SLOT     << TEGRA_SPDIF_DATA_FIFO_CSR_TX_ATN_LVL_SHIFT)
#define TEGRA_SPDIF_DATA_FIFO_CSR_TX_ATN_LVL_TU4_WORD_FULL	\
		(TEGRA_SPDIF_FIFO_ATN_LVL_D_FOUR_SLOTS   << TEGRA_SPDIF_DATA_FIFO_CSR_TX_ATN_LVL_SHIFT)
#define TEGRA_SPDIF_DATA_FIFO_CSR_TX_ATN_LVL_TU8_WORD_FULL	\
		(TEGRA_SPDIF_FIFO_ATN_LVL_D_EIGHT_SLOTS  << TEGRA_SPDIF_DATA_FIFO_CSR_TX_ATN_LVL_SHIFT)
#define TEGRA_SPDIF_DATA_FIFO_CSR_TX_ATN_LVL_TU12_WORD_FULL	\
		(TEGRA_SPDIF_FIFO_ATN_LVL_D_TWELVE_SLOTS << TEGRA_SPDIF_DATA_FIFO_CSR_TX_ATN_LVL_SHIFT)

/*                                                     */
#define TEGRA_SPDIF_DATA_FIFO_CSR_TX_EMPTY_COUNT_SHIFT		0
#define TEGRA_SPDIF_DATA_FIFO_CSR_TX_EMPTY_COUNT_MASK		(0x1f << SPDIF_DATA_FIFO_CSR_TX_EMPTY_COUNT_SHIFT)

/*                                */

/*
                                         
                                      
                                      
                                      
                                      
                                      
 */

#define TEGRA_SPDIF_DATA_OUT_DATA_16_SHIFT			0
#define TEGRA_SPDIF_DATA_OUT_DATA_16_MASK			(0xffff << TEGRA_SPDIF_DATA_OUT_DATA_16_SHIFT)

#define TEGRA_SPDIF_DATA_OUT_DATA_20_SHIFT			0
#define TEGRA_SPDIF_DATA_OUT_DATA_20_MASK			(0xfffff << TEGRA_SPDIF_DATA_OUT_DATA_20_SHIFT)

#define TEGRA_SPDIF_DATA_OUT_DATA_24_SHIFT			0
#define TEGRA_SPDIF_DATA_OUT_DATA_24_MASK			(0xffffff << TEGRA_SPDIF_DATA_OUT_DATA_24_SHIFT)

#define TEGRA_SPDIF_DATA_OUT_DATA_RAW_P				(1 << 31)
#define TEGRA_SPDIF_DATA_OUT_DATA_RAW_C				(1 << 30)
#define TEGRA_SPDIF_DATA_OUT_DATA_RAW_U				(1 << 29)
#define TEGRA_SPDIF_DATA_OUT_DATA_RAW_V				(1 << 28)

#define TEGRA_SPDIF_DATA_OUT_DATA_RAW_DATA_SHIFT		8
#define TEGRA_SPDIF_DATA_OUT_DATA_RAW_DATA_MASK			(0xfffff << TEGRA_SPDIF_DATA_OUT_DATA_RAW_DATA_SHIFT)

#define TEGRA_SPDIF_DATA_OUT_DATA_RAW_AUX_SHIFT			4
#define TEGRA_SPDIF_DATA_OUT_DATA_RAW_AUX_MASK			(0xf << TEGRA_SPDIF_DATA_OUT_DATA_RAW_AUX_SHIFT)

#define TEGRA_SPDIF_DATA_OUT_DATA_RAW_PREAMBLE_SHIFT		0
#define TEGRA_SPDIF_DATA_OUT_DATA_RAW_PREAMBLE_MASK		(0xf << TEGRA_SPDIF_DATA_OUT_DATA_RAW_PREAMBLE_SHIFT)

#define TEGRA_SPDIF_DATA_OUT_DATA_16_PACKED_RIGHT_SHIFT		16
#define TEGRA_SPDIF_DATA_OUT_DATA_16_PACKED_RIGHT_MASK		(0xffff << TEGRA_SPDIF_DATA_OUT_DATA_16_PACKED_RIGHT_SHIFT)

#define TEGRA_SPDIF_DATA_OUT_DATA_16_PACKED_LEFT_SHIFT		0
#define TEGRA_SPDIF_DATA_OUT_DATA_16_PACKED_LEFT_MASK		(0xffff << TEGRA_SPDIF_DATA_OUT_DATA_16_PACKED_LEFT_SHIFT)

/*                               */

/*
                                         
                                      
                                      
                                      
                                      
                                      
  
                                                          
 */

#define TEGRA_SPDIF_DATA_IN_DATA_P				(1 << 31)
#define TEGRA_SPDIF_DATA_IN_DATA_C				(1 << 30)
#define TEGRA_SPDIF_DATA_IN_DATA_U				(1 << 29)
#define TEGRA_SPDIF_DATA_IN_DATA_V				(1 << 28)

#define TEGRA_SPDIF_DATA_IN_DATA_PREAMBLE_SHIFT			24
#define TEGRA_SPDIF_DATA_IN_DATA_PREAMBLE_MASK			(0xf << TEGRA_SPDIF_DATA_IN_DATA_PREAMBLE_SHIFT)

#define TEGRA_SPDIF_DATA_IN_DATA_16_SHIFT			0
#define TEGRA_SPDIF_DATA_IN_DATA_16_MASK			(0xffff << TEGRA_SPDIF_DATA_IN_DATA_16_SHIFT)

#define TEGRA_SPDIF_DATA_IN_DATA_20_SHIFT			0
#define TEGRA_SPDIF_DATA_IN_DATA_20_MASK			(0xfffff << TEGRA_SPDIF_DATA_IN_DATA_20_SHIFT)

#define TEGRA_SPDIF_DATA_IN_DATA_24_SHIFT			0
#define TEGRA_SPDIF_DATA_IN_DATA_24_MASK			(0xffffff << TEGRA_SPDIF_DATA_IN_DATA_24_SHIFT)

#define TEGRA_SPDIF_DATA_IN_DATA_RAW_DATA_SHIFT			8
#define TEGRA_SPDIF_DATA_IN_DATA_RAW_DATA_MASK			(0xfffff << TEGRA_SPDIF_DATA_IN_DATA_RAW_DATA_SHIFT)

#define TEGRA_SPDIF_DATA_IN_DATA_RAW_AUX_SHIFT			4
#define TEGRA_SPDIF_DATA_IN_DATA_RAW_AUX_MASK			(0xf << TEGRA_SPDIF_DATA_IN_DATA_RAW_AUX_SHIFT)

#define TEGRA_SPDIF_DATA_IN_DATA_RAW_PREAMBLE_SHIFT		0
#define TEGRA_SPDIF_DATA_IN_DATA_RAW_PREAMBLE_MASK		(0xf << TEGRA_SPDIF_DATA_IN_DATA_RAW_PREAMBLE_SHIFT)

#define TEGRA_SPDIF_DATA_IN_DATA_16_PACKED_RIGHT_SHIFT		16
#define TEGRA_SPDIF_DATA_IN_DATA_16_PACKED_RIGHT_MASK		(0xffff << TEGRA_SPDIF_DATA_IN_DATA_16_PACKED_RIGHT_SHIFT)

#define TEGRA_SPDIF_DATA_IN_DATA_16_PACKED_LEFT_SHIFT		0
#define TEGRA_SPDIF_DATA_IN_DATA_16_PACKED_LEFT_MASK		(0xffff << TEGRA_SPDIF_DATA_IN_DATA_16_PACKED_LEFT_SHIFT)

/*                                   */
/*                                   */
/*                                   */
/*                                   */
/*                                   */
/*                                   */

/*
                                                                            
                                                                      
                                                                     
 */

/*                                   */
/*                                   */
/*                                   */
/*                                   */
/*                                   */
/*                                   */

/*
                                                                             
                                                                           
                                                                     
 */

/*                                    */

/*
                                                                           
                                  
 */

/*                                    */

/*
                                                                            
                                       
 */

struct tegra_spdif {
	struct clk *clk_spdif_out;
	int clk_refs;
	struct tegra_pcm_dma_params capture_dma_data;
	struct tegra_pcm_dma_params playback_dma_data;
	void __iomem *regs;
	struct dentry *debug;
	u32 reg_ctrl;
};

#endif
