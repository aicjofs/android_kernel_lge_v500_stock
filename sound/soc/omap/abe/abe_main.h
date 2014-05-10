/*

  This file is provided under a dual BSD/GPLv2 license.  When using or
  redistributing this file, you may do so under either license.

  GPL LICENSE SUMMARY

  Copyright(c) 2010-2011 Texas Instruments Incorporated,
  All rights reserved.

  This program is free software; you can redistribute it and/or modify
  it under the terms of version 2 of the GNU General Public License as
  published by the Free Software Foundation.

  This program is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St - Fifth Floor, Boston, MA 02110-1301 USA.
  The full GNU General Public License is included in this distribution
  in the file called LICENSE.GPL.

  BSD LICENSE

  Copyright(c) 2010-2011 Texas Instruments Incorporated,
  All rights reserved.

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions
  are met:

    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in
      the documentation and/or other materials provided with the
      distribution.
    * Neither the name of Texas Instruments Incorporated nor the names of
      its contributors may be used to endorse or promote products derived
      from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/

#ifndef _ABE_MAIN_H_
#define _ABE_MAIN_H_

#include <linux/io.h>

#include "abe_initxxx_labels.h"

#define D_DEBUG_FIFO_ADDR                                   8160
#define D_DEBUG_FIFO_ADDR_END                               8255

#define SUB_0_PARAM 0
#define SUB_1_PARAM 1

#define ABE_DEFAULT_BASE_ADDRESS_L3	0x49000000L
#define ABE_DMEM_BASE_ADDRESS_MPU	0x40180000L
#define ABE_DMEM_BASE_OFFSET_MPU	0x00080000L
#define ABE_DMEM_BASE_ADDRESS_L3	(ABE_DEFAULT_BASE_ADDRESS_L3 + \
					 ABE_DMEM_BASE_OFFSET_MPU)

/*
                                      
 */
/*       */
#define ABE_CBPR0_IDX 0
/*       */
#define ABE_CBPR1_IDX 1
/*       */
#define ABE_CBPR2_IDX 2
/*       */
#define ABE_CBPR3_IDX 3
/*        */
#define ABE_CBPR4_IDX 4
/*       */
#define ABE_CBPR5_IDX 5
/*     */
#define ABE_CBPR6_IDX 6
/*           */
#define ABE_CBPR7_IDX 7

/*
           
  
                                                  
                                           
                                           
                                             
 */
#define ABE_OPP0 0
#define ABE_OPP25 1
#define ABE_OPP50 2
#define ABE_OPP100 3
/*
               
  
                                                          
                                                                    
              
                                          
                                                       
                                                
                                                     
                                                               
                                                             
        
                                                               
 */
#define MONO_MSB 1
#define MONO_RSHIFTED_16 2
#define STEREO_RSHIFTED_16 3
#define STEREO_16_16 4
#define STEREO_MSB 5
#define THREE_MSB 6
#define FOUR_MSB 7
#define FIVE_MSB 8
#define SIX_MSB 9
#define SEVEN_MSB 10
#define EIGHT_MSB 11
#define NINE_MSB 12
#define TEN_MSB 13
/*
                                                   
 */
#define SLIMBUS_PORT_PROT 1
#define SERIAL_PORT_PROT 2
#define TDM_SERIAL_PORT_PROT 3
#define DMIC_PORT_PROT 4
#define MCPDMDL_PORT_PROT 5
#define MCPDMUL_PORT_PROT 6
#define PINGPONG_PORT_PROT 7
#define DMAREQ_PORT_PROT 8
/*
                                                          
 */
#define DMIC_PORT 0
#define PDM_UL_PORT 1
#define BT_VX_UL_PORT 2
#define MM_UL_PORT 3
#define MM_UL2_PORT 4
#define VX_UL_PORT 5
#define MM_DL_PORT 6
#define VX_DL_PORT 7
#define TONES_DL_PORT 8
#define VIB_DL_PORT 9
#define BT_VX_DL_PORT 10
#define PDM_DL_PORT 11
#define MM_EXT_OUT_PORT 12
#define MM_EXT_IN_PORT 13
#define TDM_DL_PORT 14
#define TDM_UL_PORT 15
#define DEBUG_PORT 16
#define LAST_PORT_ID 17
/*                                                */
#define PDM_DL1_PORT 18
#define PDM_DL2_PORT 19
#define PDM_VIB_PORT 20
/*                                                        
                     */
#define DMIC_PORT1 DMIC_PORT
#define DMIC_PORT2 DMIC_PORT
#define DMIC_PORT3 DMIC_PORT
/*
                                                   
 */
/*                                            */
#define FEAT_EQ1            1
/*                                                   */
#define FEAT_EQ2L           (FEAT_EQ1+1)
/*                                                    */
#define FEAT_EQ2R           (FEAT_EQ2L+1)
/*                                   */
#define FEAT_EQSDT          (FEAT_EQ2R+1)
/*                            */
#define FEAT_EQAMIC         (FEAT_EQSDT+1)
/*                            */
#define FEAT_EQDMIC         (FEAT_EQAMIC+1)
/*                                 */
#define FEAT_APS1           (FEAT_EQDMIC+1)
/*                                                           */
#define FEAT_APS2           (FEAT_APS1+1)
/*                                                            */
#define FEAT_APS3           (FEAT_APS2+1)
/*                                                                */
#define FEAT_ASRC1          (FEAT_APS3+1)
/*                                                              */
#define FEAT_ASRC2          (FEAT_ASRC1+1)
/*                                                              */
#define FEAT_ASRC3          (FEAT_ASRC2+1)
/*                                                           */
#define FEAT_ASRC4          (FEAT_ASRC3+1)
/*                                        */
#define FEAT_MIXDL1         (FEAT_ASRC4+1)
/*                              */
#define FEAT_MIXDL2         (FEAT_MIXDL1+1)
/*                                                 */
#define FEAT_MIXAUDUL       (FEAT_MIXDL2+1)
/*                                         */
#define FEAT_MIXVXREC       (FEAT_MIXAUDUL+1)
/*                     */
#define FEAT_MIXSDT         (FEAT_MIXVXREC+1)
/*                          */
#define FEAT_MIXECHO        (FEAT_MIXSDT+1)
/*                           */
#define FEAT_UPROUTE        (FEAT_MIXECHO+1)
/*           */
#define FEAT_GAINS          (FEAT_UPROUTE+1)
#define FEAT_GAINS_DMIC1    (FEAT_GAINS+1)
#define FEAT_GAINS_DMIC2    (FEAT_GAINS_DMIC1+1)
#define FEAT_GAINS_DMIC3    (FEAT_GAINS_DMIC2+1)
#define FEAT_GAINS_AMIC     (FEAT_GAINS_DMIC3+1)
#define FEAT_GAINS_SPLIT    (FEAT_GAINS_AMIC+1)
#define FEAT_GAINS_DL1      (FEAT_GAINS_SPLIT+1)
#define FEAT_GAINS_DL2      (FEAT_GAINS_DL1+1)
#define FEAT_GAIN_BTUL      (FEAT_GAINS_DL2+1)
/*                                 */
#define FEAT_SEQ            (FEAT_GAIN_BTUL+1)
/*                                     */
#define FEAT_CTL            (FEAT_SEQ+1)
/*                                                                 */
#define MAXNBFEATURE    FEAT_CTL
/*            */
/*                                            */
#define EQ1 FEAT_EQ1
/*                                                   */
#define EQ2L FEAT_EQ2L
#define EQ2R FEAT_EQ2R
/*                                   */
#define EQSDT  FEAT_EQSDT
#define EQAMIC FEAT_EQAMIC
#define EQDMIC FEAT_EQDMIC
/*            */
/*                                 */
#define APS1 FEAT_APS1
#define APS2L FEAT_APS2
#define APS2R FEAT_APS3
/*             */
/*                                                                */
#define ASRC1 FEAT_ASRC1
/*                                                              */
#define ASRC2 FEAT_ASRC2
/*                                                              */
#define ASRC3 FEAT_ASRC3
/*                                                                        */
#define ASRC4 FEAT_ASRC4
/*              */
#define MIXDL1 FEAT_MIXDL1
#define MIXDL2 FEAT_MIXDL2
#define MIXSDT FEAT_MIXSDT
#define MIXECHO FEAT_MIXECHO
#define MIXAUDUL FEAT_MIXAUDUL
#define MIXVXREC FEAT_MIXVXREC
/*               */
/*                                    */
#define UPROUTE  FEAT_UPROUTE
/*
                
 */
#define GAIN_LEFT_OFFSET 0
#define GAIN_RIGHT_OFFSET 1
/*
           
 */
#define GAINS_DMIC1     FEAT_GAINS_DMIC1
#define GAINS_DMIC2     FEAT_GAINS_DMIC2
#define GAINS_DMIC3     FEAT_GAINS_DMIC3
#define GAINS_AMIC      FEAT_GAINS_AMIC
#define GAINS_SPLIT     FEAT_GAINS_SPLIT
#define GAINS_DL1       FEAT_GAINS_DL1
#define GAINS_DL2       FEAT_GAINS_DL2
#define GAINS_BTUL      FEAT_GAIN_BTUL
/*
                                            
 */
#define sizeof_alpha_iir_table 61
#define sizeof_beta_iir_table 61
#define GAIN_MAXIMUM 3000L
#define GAIN_24dB 2400L
#define GAIN_18dB 1800L
#define GAIN_12dB 1200L
#define GAIN_6dB 600L
/*                  */
#define GAIN_0dB  0L
#define GAIN_M6dB -600L
#define GAIN_M12dB -1200L
#define GAIN_M18dB -1800L
#define GAIN_M24dB -2400L
#define GAIN_M30dB -3000L
#define GAIN_M40dB -4000L
#define GAIN_M50dB -5000L
/*                            */
#define MUTE_GAIN -12000L
#define GAIN_TOOLOW -13000L
#define GAIN_MUTE MUTE_GAIN
#define RAMP_MINLENGTH 3L
/*                             */
#define RAMP_0MS 0L
#define RAMP_1MS 1L
#define RAMP_2MS 2L
#define RAMP_5MS 5L
#define RAMP_10MS 10L
#define RAMP_20MS 20L
#define RAMP_50MS 50L
#define RAMP_100MS 100L
#define RAMP_200MS  200L
#define RAMP_500MS  500L
#define RAMP_1000MS  1000L
#define RAMP_MAXLENGTH  10000L
/*                               */
#define LINABE_TO_DECIBELS 1
#define DECIBELS_TO_LINABE 2
/*                               */
#define IIRABE_TO_MICROS 1
#define MICROS_TO_IIABE 2
/*
                                  
 */
#define EVENT_TIMER 0
#define EVENT_44100 1
/*
               
 */
/*                                                    */
#define External_DMA_0	0
/*                                    */
#define DMIC_DMA_REQ	1
/*                          */
#define McPDM_DMA_DL	2
/*                        */
#define McPDM_DMA_UP	3
/*                                  */
#define MCBSP1_DMA_TX	4
/*                                 */
#define MCBSP1_DMA_RX	5
/*                                  */
#define MCBSP2_DMA_TX	6
/*                                 */
#define MCBSP2_DMA_RX	7
/*                                  */
#define MCBSP3_DMA_TX	8
/*                                 */
#define MCBSP3_DMA_RX	9
/*
                                  
 */
#define MCBSP1_TX MCBSP1_DMA_TX
#define MCBSP1_RX MCBSP1_DMA_RX
#define MCBSP2_TX MCBSP2_DMA_TX
#define MCBSP2_RX MCBSP2_DMA_RX
#define MCBSP3_TX MCBSP3_DMA_TX
#define MCBSP3_RX MCBSP3_DMA_RX

#define PING_PONG_WITH_MCU_IRQ	 1
#define PING_PONG_WITH_DSP_IRQ	 2

/*
                                  
                       
              
             
              
                         
                 
                             
                  
                             
               
              
                             
              
              
              
*/
#define MIX_VXUL_INPUT_MM_DL 0
#define MIX_VXUL_INPUT_TONES 1
#define MIX_VXUL_INPUT_VX_UL 2
#define MIX_VXUL_INPUT_VX_DL 3
#define MIX_DL1_INPUT_MM_DL 0
#define MIX_DL1_INPUT_MM_UL2 1
#define MIX_DL1_INPUT_VX_DL 2
#define MIX_DL1_INPUT_TONES 3
#define MIX_DL2_INPUT_MM_DL 0
#define MIX_DL2_INPUT_MM_UL2 1
#define MIX_DL2_INPUT_VX_DL 2
#define MIX_DL2_INPUT_TONES 3
#define MIX_SDT_INPUT_UP_MIXER	0
#define MIX_SDT_INPUT_DL1_MIXER 1
#define MIX_AUDUL_INPUT_MM_DL 0
#define MIX_AUDUL_INPUT_TONES 1
#define MIX_AUDUL_INPUT_UPLINK 2
#define MIX_AUDUL_INPUT_VX_DL 3
#define MIX_VXREC_INPUT_MM_DL 0
#define MIX_VXREC_INPUT_TONES 1
#define MIX_VXREC_INPUT_VX_UL 2
#define MIX_VXREC_INPUT_VX_DL 3
#define MIX_ECHO_DL1	0
#define MIX_ECHO_DL2	1
/*                        */
#define NBROUTE_UL 16
/*                       */
#define NBROUTE_CONFIG_MAX 10
/*                               */
#define NBROUTE_CONFIG 6
/*               */
#define UPROUTE_CONFIG_AMIC 0
/*                          */
#define UPROUTE_CONFIG_DMIC1 1
/*                           */
#define UPROUTE_CONFIG_DMIC2 2
/*                         */
#define UPROUTE_CONFIG_DMIC3 3
/*                */
#define UPROUTE_CONFIG_BT 4
/*                    */
#define UPROUTE_ECHO_MMUL2 5

/*
        
  
                                       
 */
typedef struct {
	/*                                            */
	void *data;
	/*                                                                   */
	/*                                                            */
	void *l3_dmem;
	/*                                                  */
	void *l4_dmem;
	/*                                              */
	u32 iter;
} abe_dma_t;
typedef u32 abe_dbg_t;
/*
           
  
                                     
 */
typedef u16 abe_router_t;
/*
                
  
                           
 */
typedef struct {
	/*                                  */
	u32 f;
	/*                     */
	u32 samp_format;
} abe_data_format_t;
/*
                  
  
                   
 */
typedef struct {
	/*                                                 */
	u32 direction;
	/*                                                  */
	u32 protocol_switch;
	union {
		/*                                     */
		struct {
			/*                                           */
			u32 desc_addr1;
			/*                         */
			u32 buf_addr1;
			/*                                */
			u32 buf_size;
			/*                                  */
			u32 iter;
			/*                                                  */
			u32 desc_addr2;
			/*                         */
			u32 buf_addr2;
		} prot_slimbus;
		/*                                         */
		struct {
			u32 desc_addr;
			/*                                                  */
			u32 buf_addr;
			/*                       */
			u32 buf_size;
			/*                                  */
			u32 iter;
		} prot_serial;
		/*                                  */
		struct {
			/*                       */
			u32 buf_addr;
			/*                           */
			u32 buf_size;
			/*                          */
			u32 nbchan;
		} prot_dmic;
		/*                                     */
		struct {
			/*                       */
			u32 buf_addr;
			/*                    */
			u32 buf_size;
			/*                             */
			u32 control;
		} prot_mcpdmdl;
		/*                                     */
		struct {
			/*                            */
			u32 buf_addr;
			/*                                */
			u32 buf_size;
		} prot_mcpdmul;
		/*                                                   */
		struct {
			/*                             */
			u32 desc_addr;
			/*                                   */
			u32 buf_addr;
			/*                                                   */
			u32 buf_size;
			/*                                                */
			u32 irq_addr;
			/*                                                  */
			u32 irq_data;
			/*                                       */
			u32 callback;
		} prot_pingpong;
		/*                     */
		struct {
			/*                             */
			u32 desc_addr;
			/*                              */
			u32 buf_addr;
			/*                                */
			u32 buf_size;
			/*                                  */
			u32 iter;
			/*                */
			u32 dma_addr;
			/*                      */
			u32 dma_data;
		} prot_dmareq;
		/*                                             */
		struct {
			/*                                   */
			u32 buf_addr;
			/*                           */
			u32 buf_size;
			/*                */
			u32 dma_addr;
			/*                      */
			u32 dma_data;
		} prot_circular_buffer;
	} p;
} abe_port_protocol_t;

/*
        
  
                                
 */
/*                       */
#define NBEQ1 25
/*                         */
#define NBEQ2 13

typedef struct {
	/*                */
	u32 equ_type;
	/*               */
	u32 equ_length;
	union {
		/*                                                         */
		/*                                   */
		s32 type1[NBEQ1];
		struct {
			/*                                   */
			s32 freq[NBEQ2];
			/*                         */
			s32 gain[NBEQ2];
			/*                            */
			s32 q[NBEQ2];
		} type2;
	} coef;
	s32 equ_param3;
} abe_equ_t;


/*                              */
typedef void (*abe_subroutine0) (void);
/*                               */
typedef void (*abe_subroutine1) (u32);
typedef void (*abe_subroutine2) (u32, u32);
typedef void (*abe_subroutine3) (u32, u32, u32);
typedef void (*abe_subroutine4) (u32, u32, u32, u32);


extern u32 abe_irq_pingpong_player_id;


void abe_init_mem(void __iomem **_io_base);
u32 abe_reset_hal(void);
int abe_load_fw(u32 *firmware);
int abe_reload_fw(u32 *firmware);
u32 *abe_get_default_fw(void);
u32 abe_wakeup(void);
u32 abe_irq_processing(void);
u32 abe_clear_irq(void);
u32 abe_disable_irq(void);
u32 abe_write_event_generator(u32 e);
u32 abe_stop_event_generator(void);
u32 abe_connect_debug_trace(abe_dma_t *dma2);
u32 abe_set_debug_trace(abe_dbg_t debug);
u32 abe_set_ping_pong_buffer(u32 port, u32 n_bytes);
u32 abe_read_next_ping_pong_buffer(u32 port, u32 *p, u32 *n);
u32 abe_init_ping_pong_buffer(u32 id, u32 size_bytes, u32 n_buffers,
					u32 *p);
u32 abe_read_offset_from_ping_buffer(u32 id, u32 *n);
u32 abe_write_equalizer(u32 id, abe_equ_t *param);
u32 abe_disable_gain(u32 id, u32 p);
u32 abe_enable_gain(u32 id, u32 p);
u32 abe_mute_gain(u32 id, u32 p);
u32 abe_unmute_gain(u32 id, u32 p);
u32 abe_write_gain(u32 id, s32 f_g, u32 ramp, u32 p);
u32 abe_write_mixer(u32 id, s32 f_g, u32 f_ramp, u32 p);
u32 abe_read_gain(u32 id, u32 *f_g, u32 p);
u32 abe_read_mixer(u32 id, u32 *f_g, u32 p);
u32 abe_set_router_configuration(u32 id, u32 k, u32 *param);
u32 abe_set_opp_processing(u32 opp);
u32 abe_disable_data_transfer(u32 id);
u32 abe_enable_data_transfer(u32 id);
u32 abe_connect_cbpr_dmareq_port(u32 id, abe_data_format_t *f, u32 d,
					   abe_dma_t *returned_dma_t);
u32 abe_connect_irq_ping_pong_port(u32 id, abe_data_format_t *f,
					     u32 subroutine_id, u32 size,
					     u32 *sink, u32 dsp_mcu_flag);
u32 abe_connect_serial_port(u32 id, abe_data_format_t *f,
				      u32 mcbsp_id);
u32 abe_read_port_address(u32 port, abe_dma_t *dma2);
void abe_add_subroutine(u32 *id, abe_subroutine2 f, u32 nparam, u32 *params);
u32 abe_read_next_ping_pong_buffer(u32 port, u32 *p, u32 *n);
u32 abe_check_activity(void);
void abe_add_subroutine(u32 *id, abe_subroutine2 f,
						u32 nparam, u32 *params);

u32 abe_plug_subroutine(u32 *id, abe_subroutine2 f, u32 n,
			u32 *params);

#endif				/*              */
