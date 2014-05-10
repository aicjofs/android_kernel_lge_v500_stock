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

#include "abe_def.h"
#include "abe_initxxx_labels.h"

#ifndef _ABE_TYP_H_
#define _ABE_TYP_H_
/*
              
 */
#define MAX_UINT8	((((1L <<  7) - 1) << 1) + 1)
#define MAX_UINT16	((((1L << 15) - 1) << 1) + 1)
#define MAX_UINT32	((((1L << 31) - 1) << 1) + 1)
#define s8 char
#define u8 unsigned char
#define s16 short
#define u16 unsigned short
#define s32 int
#define u32 unsigned int
/*                               */
#define abehal_status u32
/*                                                                       */
typedef u32 abe_dbg_mask_t;
/*                                                               
                      */
typedef u32 abe_dbg_t;
/*                                 */
typedef u32 abe_seq_code_t;
/*                                                          */
typedef u32 abe_sub_code_t;
/*                              */
typedef void (*abe_subroutine0) (void);
/*                               */
typedef void (*abe_subroutine1) (u32);
typedef void (*abe_subroutine2) (u32, u32);
typedef void (*abe_subroutine3) (u32, u32, u32);
typedef void (*abe_subroutine4) (u32, u32, u32, u32);
/*
                                    
  
                                                                       
                                                                
                                                                    
 */
typedef u32 abe_patch_rev;
/*
        
 */
/*
                     
  
                                                  
                                           
                                           
                                             
 */
#define ABE_AUDIO_PLAYER_ON_HEADSET_OR_EARPHONE 1
#define ABE_DRIFT_MANAGEMENT_FOR_AUDIO_PLAYER 2
#define ABE_DRIFT_MANAGEMENT_FOR_VOICE_CALL 3
#define ABE_VOICE_CALL_ON_HEADSET_OR_EARPHONE_OR_BT 4
#define ABE_MULTIMEDIA_AUDIO_RECORDER 5
#define ABE_VIBRATOR_OR_HAPTICS 6
#define ABE_VOICE_CALL_ON_HANDS_FREE_SPEAKER 7
#define ABE_RINGER_TONES 8
#define ABE_VOICE_CALL_WITH_EARPHONE_ACTIVE_NOISE_CANCELLER 9
#define ABE_LAST_USE_CASE 10
/*
           
  
                                                  
                                           
                                           
                                             
 */
#define ABE_OPP0 0
#define ABE_OPP25 1
#define ABE_OPP50 2
#define ABE_OPP100 3
/*
                        
  
 */
#define ABE_DEC16 16
#define ABE_DEC25 25
#define ABE_DEC32 32
#define ABE_DEC40 40
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
#define OMAP_ABE_DMIC_PORT 0
#define OMAP_ABE_PDM_UL_PORT 1
#define OMAP_ABE_BT_VX_UL_PORT 2
#define OMAP_ABE_MM_UL_PORT 3
#define OMAP_ABE_MM_UL2_PORT 4
#define OMAP_ABE_VX_UL_PORT 5
#define OMAP_ABE_MM_DL_PORT 6
#define OMAP_ABE_VX_DL_PORT 7
#define OMAP_ABE_TONES_DL_PORT 8
#define OMAP_ABE_VIB_DL_PORT 9
#define OMAP_ABE_BT_VX_DL_PORT 10
#define OMAP_ABE_PDM_DL_PORT 11
#define OMAP_ABE_MM_EXT_OUT_PORT 12
#define OMAP_ABE_MM_EXT_IN_PORT 13
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
#define SRC_DL1_MIXER_OUTPUT DL1_M_labelID
#define SRC_SDT_MIXER_OUTPUT SDT_M_labelID
#define SRC_DL1_GAIN_OUTPUT DL1_GAIN_out_labelID
#define SRC_DL1_EQ_OUTPUT DL1_EQ_labelID
#define SRC_DL2_GAIN_OUTPUT DL2_GAIN_out_labelID
#define SRC_DL2_EQ_OUTPUT DL2_EQ_labelID
#define SRC_MM_DL MM_DL_labelID
#define SRC_TONES_DL  Tones_labelID
#define SRC_VX_DL VX_DL_labelID
#define SRC_VX_UL VX_UL_labelID
#define SRC_MM_UL2 MM_UL2_labelID
#define SRC_MM_UL MM_UL_labelID
/*
                         
                                           
                                    
 */
#define DBG_PATCH_AMIC 1
#define DBG_PATCH_DMIC1 2
#define DBG_PATCH_DMIC2 3
#define DBG_PATCH_DMIC3 4
#define DBG_PATCH_VX_REC 5
#define DBG_PATCH_BT_UL 6
#define DBG_PATCH_MM_DL 7
#define DBG_PATCH_DL2_EQ 8
#define DBG_PATCH_VIBRA 9
#define DBG_PATCH_MM_EXT_IN 10
#define DBG_PATCH_EANC_FBK_Out 11
#define DBG_PATCH_MIC4 12
#define DBG_PATCH_MM_DL_MIXDL1 13
#define DBG_PATCH_MM_DL_MIXDL2 14
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
#define EVENT_TIMER 0
#define EVENT_44100 1
/*
                                  
 */
#define MCBSP1_TX MCBSP1_DMA_TX
#define MCBSP1_RX MCBSP1_DMA_RX
#define MCBSP2_TX MCBSP2_DMA_TX
#define MCBSP2_RX MCBSP2_DMA_RX
#define MCBSP3_TX MCBSP3_DMA_TX
#define MCBSP3_RX MCBSP3_DMA_RX
/*
                                     
 */
#define SLIMBUS1_TX0  SLIMBUS1_DMA_TX0
#define SLIMBUS1_TX1  SLIMBUS1_DMA_TX1
#define SLIMBUS1_TX2  SLIMBUS1_DMA_TX2
#define SLIMBUS1_TX3  SLIMBUS1_DMA_TX3
#define SLIMBUS1_TX4  SLIMBUS1_DMA_TX4
#define SLIMBUS1_TX5  SLIMBUS1_DMA_TX5
#define SLIMBUS1_TX6  SLIMBUS1_DMA_TX6
#define SLIMBUS1_TX7  SLIMBUS1_DMA_TX7
#define SLIMBUS1_RX0  SLIMBUS1_DMA_RX0
#define SLIMBUS1_RX1  SLIMBUS1_DMA_RX1
#define SLIMBUS1_RX2  SLIMBUS1_DMA_RX2
#define SLIMBUS1_RX3  SLIMBUS1_DMA_RX3
#define SLIMBUS1_RX4  SLIMBUS1_DMA_RX4
#define SLIMBUS1_RX5  SLIMBUS1_DMA_RX5
#define SLIMBUS1_RX6  SLIMBUS1_DMA_RX6
#define SLIMBUS1_RX7  SLIMBUS1_DMA_RX7
#define SLIMBUS_UNUSED  _DUMMY_FIFO_
/*
                                                           
 */

/*
        
  
                                
 */
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

/*
        
  
                                                      
 */
struct abe_aps_t {
	s32 coef1[NBAPS1];
	s32 coef2[NBAPS2];
};

struct abe_aps_energy_t {
	/*                                                            */
	u32 e1;
	u32 e2;
};
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

typedef struct {
	/*                                    */
	u32 data;
	/*                                              */
	u32 iter;
} abe_dma_t_offset;
/*
        
  
           
                                                                   
                                                                   
                                            
                           
                                  
           
  
 */
typedef struct {
	u32 delta_time;
	u32 code;
	u32 param[4];
	u8 tag;
} abe_seq_t;

typedef struct {
	u32 mask;
	abe_seq_t seq1;
	abe_seq_t seq2;
} abe_sequence_t;
/*
                            
  
                                      
 */
/*
                                                                     
 */
/*
                 
  
                               
                                                 
                                                            
                                                 
                                                              
                                                      
                                                              
 */
typedef struct {
	unsigned int counter:16;
	unsigned int data:12;
	unsigned int tag:4;
} abe_irq_data_t;
/*
                                                                
                            
  
 */
typedef struct {
	/*                 */
	u16 status;
	/*                     */
	abe_data_format_t format;
	/*                */
	s32 drift;
	/*                                              */
	u16 callback;
	/*                  */
	u16 smem_buffer1;
	u16 smem_buffer2;
	abe_port_protocol_t protocol;
	/*                                           */
	abe_dma_t_offset dma;
	/*                                                                    */
	u16 feature_index[MAXFEATUREPORT];
	char name[NBCHARPORTNAME];
} abe_port_t;
/*
                   
  
 */
typedef struct {
	u32 sub_id;
	s32 param[4];
} abe_subroutine_t;

#endif/*                    */
