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

#ifndef _ABE_DEF_H_
#define _ABE_DEF_H_
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
#define CIRCULAR_BUFFER_PERIPHERAL_R__0 (0x100 + ABE_CBPR0_IDX*4)
#define CIRCULAR_BUFFER_PERIPHERAL_R__1 (0x100 + ABE_CBPR1_IDX*4)
#define CIRCULAR_BUFFER_PERIPHERAL_R__2 (0x100 + ABE_CBPR2_IDX*4)
#define CIRCULAR_BUFFER_PERIPHERAL_R__3 (0x100 + ABE_CBPR3_IDX*4)
#define CIRCULAR_BUFFER_PERIPHERAL_R__4 (0x100 + ABE_CBPR4_IDX*4)
#define CIRCULAR_BUFFER_PERIPHERAL_R__5 (0x100 + ABE_CBPR5_IDX*4)
#define CIRCULAR_BUFFER_PERIPHERAL_R__6 (0x100 + ABE_CBPR6_IDX*4)
#define CIRCULAR_BUFFER_PERIPHERAL_R__7 (0x100 + ABE_CBPR7_IDX*4)
#define PING_PONG_WITH_MCU_IRQ	 1
#define PING_PONG_WITH_DSP_IRQ	 2
/*                                         */
#define COPY_FROM_ABE_TO_HOST 1
#define COPY_FROM_HOST_TO_ABE 2
/*
                       
 */
#define ABE_FIRMWARE_MAX_SIZE 26629
/*                       */
#define NBEQ1 25
/*                         */
#define NBEQ2 13
/*                                 */
#define NBAPS1 10
/*                                  */
#define NBAPS2 10
/*                                                       */
#define NBMIX_AUDIO_UL 2
/*                     */
#define NBMIX_DL1 4
/*                          */
#define NBMIX_DL2 4
/*                 */
#define NBMIX_SDT 2
/*                      */
#define NBMIX_ECHO 2
/*                    */
#define NBMIX_VXREC 4
/*                          */
#define CC_M1 0xFF
#define CS_M1 0xFFFF
#define CL_M1 0xFFFFFFFFL
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
/*                   */
#define MAXCALLBACK 100
/*             */
#define MAXNBSUBROUTINE 100
/*                           */
#define MAXNBSEQUENCE 20
/*                                       */
#define MAXACTIVESEQUENCE 20
/*                                      */
#define MAXSEQUENCESTEPS 2
/*                                            */
#define MAXFEATUREPORT 12
#define SUB_0_PARAM 0
/*                                         */
#define SUB_1_PARAM 1
#define SUB_2_PARAM 2
#define SUB_3_PARAM 3
#define SUB_4_PARAM 4
/*                                                 */
#define FREE_LINE 0
/*                                 */
#define NOMASK (1 << 0)
/*                                                              */
#define MASK_PDM_OFF (1 << 1)
/*                                                             */
#define MASK_PDM_ON (1 << 2)
/*                              */
#define NBCHARFEATURENAME 16
/*                           */
#define NBCHARPORTNAME 16
/*                                                                      */
#define SNK_P ABE_ATC_DIRECTION_IN
/*                      */
#define SRC_P ABE_ATC_DIRECTION_OUT
/*                 */
#define NODRIFT 0
/*                                */
#define FORCED_DRIFT_CONTROL 1
/*                                */
#define ADPATIVE_DRIFT_CONTROL 2
/*                                                */
#define DOPPMODE32_OPP100 (0x00000010)
#define DOPPMODE32_OPP50 (0x0000000C)
#define DOPPMODE32_OPP25 (0x0000004)
/*
                                            
 */
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
#define RAMP_MINLENGTH 0L
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
/*                   */
#define OMAP_ABE_PORT_ACTIVITY_IDLE	1
/*                                          */
#define OMAP_ABE_PORT_INITIALIZED	 3
/*                      */
#define OMAP_ABE_PORT_ACTIVITY_RUNNING	 2
#define NOCALLBACK 0
#define NOPARAMETER 0
/*                                                                       */
#define MCPDM_UL_ITER 4
/*                                                */
#define MCPDM_DL_ITER 24
/*                                               */
#define DMIC_ITER 12
/*                     */
#define MAX_PINGPONG_BUFFERS 2
/*
                             
 */
#define SUB_WRITE_MIXER 1
#define SUB_WRITE_PORT_GAIN 2
/*         */
#define c_feat_init_eq 1
#define c_feat_read_eq1 2
#define c_write_eq1 3
#define c_feat_read_eq2 4
#define c_write_eq2 5
#define c_feat_read_eq3 6
#define c_write_eq3 7
/*                                            */
#define MAX_NBGAIN_CMEM 36
/*
         
 */
#define maximum(a, b) (((a) < (b)) ? (b) : (a))
#define minimum(a, b) (((a) > (b)) ? (b) : (a))
#define absolute(a) (((a) > 0) ? (a) : ((-1)*(a)))
#define HAL_VERSIONS 9
#endif/*             */
