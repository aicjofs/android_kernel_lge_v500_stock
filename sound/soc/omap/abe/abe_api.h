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

#ifndef _ABE_API_H_
#define _ABE_API_H_

#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/init.h>
#include <linux/err.h>
#include <linux/slab.h>

#include "abe_dm_addr.h"
#include "abe_dbg.h"

#define ABE_TASK_ID(ID) (OMAP_ABE_D_TASKSLIST_ADDR + sizeof(ABE_STask)*(ID))

#define TASK_ASRC_VX_DL_SLT 0
#define TASK_ASRC_VX_DL_IDX 3
#define TASK_VX_DL_SLT 1
#define TASK_VX_DL_IDX 3
#define TASK_DL2Mixer_SLT 1
#define TASK_DL2Mixer_IDX 6
#define TASK_DL1Mixer_SLT 2
#define TASK_DL1Mixer_IDX 0
#define TASK_VX_UL_SLT 12
#define TASK_VX_UL_IDX 5
#define TASK_BT_DL_48_8_SLT 14
#define TASK_BT_DL_48_8_IDX 4
#define TASK_ASRC_BT_UL_SLT 15
#define TASK_ASRC_BT_UL_IDX 6
#define TASK_ASRC_VX_UL_SLT 16
#define TASK_ASRC_VX_UL_IDX 2
#define TASK_BT_UL_8_48_SLT 17
#define TASK_BT_UL_8_48_IDX 2
#define TASK_IO_MM_DL_SLT 18
#define TASK_IO_MM_DL_IDX 0
#define TASK_ASRC_BT_DL_SLT 18
#define TASK_ASRC_BT_DL_IDX 6


struct omap_abe {
	void __iomem *io_base[5];
	u32 firmware_version_number;
	u16 MultiFrame[PROCESSING_SLOTS][TASKS_IN_SLOT];
	u32 compensated_mixer_gain;
	u8  muted_gains_indicator[MAX_NBGAIN_CMEM];
	u32 desired_gains_decibel[MAX_NBGAIN_CMEM];
	u32 muted_gains_decibel[MAX_NBGAIN_CMEM];
	u32 desired_gains_linear[MAX_NBGAIN_CMEM];
	u32 desired_ramp_delay_ms[MAX_NBGAIN_CMEM];
	struct mutex mutex;
	u32 warm_boot;

	u32 irq_dbg_read_ptr;
	u32 dbg_param;

	struct omap_abe_dbg dbg;
};

/* 
                                    
                          
                                     
  
                                                                   
                          
                                              
 */
abehal_status abe_reset_hal(void);
/* 
                                                 
                                        
                                
                                           
                                
                                       
                                
                                     
                                
  
 */
abehal_status abe_load_fw_param(u32 *FW);
/* 
                                             
  
                                                                          
                                                                      
                                                                           
                                                                            
                                                                   
                                                                              
          
 */
abehal_status abe_irq_processing(void);
/* 
                                      
  
                                           
 */
abehal_status abe_clear_irq(void);
/* 
                                                  
  
                                               
 */
abehal_status abe_disable_irq(void);
/*
                                                  
 */
u32 abe_check_activity(void);
/* 
                          
  
                                  
 */
abehal_status abe_wakeup(void);
/* 
                                                           
  
                                                                               
                                                               
                        
 */
abehal_status abe_start_event_generator(void);
/* 
                                                          
  
                                                                              
                                                               
                        
 */
abehal_status abe_stop_event_generator(void);

/* 
                                                             
                                                        
  
                                                  
                                             
                                                                          
                                            
                                                                     
                                                     
  
                                                                      
                                                         
                                                                               
               
                                                                      
                                          
                                                             
                                        
 */
abehal_status abe_write_event_generator(u32 e);
/* 
                                                         
                  
  
                                  
                                                                                
                                                                      
                                          
                                                  
  
                                                                            
                                                                             
               
  
 */
abehal_status abe_set_opp_processing(u32 opp);
/* 
                           
                     
                                     
  
                                                                      
                                                                           
 */
abehal_status abe_set_ping_pong_buffer(u32 port, u32 n_bytes);
/* 
                                 
                    
                                    
                                 
  
                                                                       
 */
abehal_status abe_read_next_ping_pong_buffer(u32 port, u32 *p, u32 *n);
/* 
                            
                   
                                    
                                               
                                                                         
                 
  
                                                     
 */
abehal_status abe_init_ping_pong_buffer(u32 id, u32 size_bytes, u32 n_buffers,
					u32 *p);
/* 
                                   
                   
                                      
                                                  
  
                                                                 
                                                                             
 */
abehal_status abe_read_offset_from_ping_buffer(u32 id, u32 *n);
/* 
                      
                                                               
                                        
                                              
                                 
  
                                                       
 */
abehal_status abe_plug_subroutine(u32 *id, abe_subroutine2 f, u32 n,
				  u32 *params);
/* 
                                 
                      
                      
  
 */
abehal_status abe_set_sequence_time_accuracy(u32 fast, u32 slow);
/* 
                 
                   
  
                                                                         
                       
                                  
 */
abehal_status abe_reset_port(u32 id);
/* 
                          
                   
                                                           
  
                                                       
 */
abehal_status abe_read_remaining_data(u32 port, u32 *n);
/* 
                            
                   
  
                                                          
                               
                                     
 */
abehal_status abe_disable_data_transfer(u32 id);
/* 
                           
                   
  
                             
                     
                               
 */
abehal_status abe_enable_data_transfer(u32 id);
/* 
                      
                                          
  
                                                                               
                                                                           
                     
                                                                              
 */
abehal_status abe_set_dmic_filter(u32 d);
/* 
                               
                 
                          
                                      
                                                                              
                                            
  
                                                                 
                          
 */
abehal_status abe_connect_cbpr_dmareq_port(u32 id, abe_data_format_t *f, u32 d,
					   abe_dma_t *returned_dma_t);
/* 
                                 
                 
                          
                                                
                              
                                                        
  
                                                                
                                                                       
                                                                        
                                                                        
                                                                        
               
 */
abehal_status abe_connect_irq_ping_pong_port(u32 id, abe_data_format_t *f,
					     u32 subroutine_id, u32 size,
					     u32 *sink, u32 dsp_mcu_flag);
/* 
                            
                 
                  
                                       
  
                                                                              
                                                                          
                                                                      
                      
 */
abehal_status abe_connect_serial_port(u32 id, abe_data_format_t *f,
				      u32 mcbsp_id);
/* 
                        
                                                                         
  
                                                                            
                                                                        
                     
 */
abehal_status abe_read_port_address(u32 port, abe_dma_t *dma2);
/* 
                      
                             
                                  
  
                                 
 */
abehal_status abe_write_equalizer(u32 id, abe_equ_t *param);
/* 
                 
                        
                                          
  
                                                                         
                                                                       
                                                                       
                                                                            
                                                                             
                                        
 */
abehal_status abe_write_asrc(u32 port, s32 dppm);
/* 
                
                              
                                       
  
                                                                     
                                                                   
                                                                         
              
                                                                     
 */
abehal_status abe_write_aps(u32 id, struct abe_aps_t *param);
/* 
                  
                         
                                           
                                                    
  
                                                                       
                                                                         
                                                                             
                                                                     
                                 
 */
abehal_status abe_write_gain(u32 id, s32 f_g, u32 ramp, u32 p);
abehal_status abe_use_compensated_gain(u32 on_off);
abehal_status abe_enable_gain(u32 id, u32 p);
abehal_status abe_disable_gain(u32 id, u32 p);
abehal_status abe_mute_gain(u32 id, u32 p);
abehal_status abe_unmute_gain(u32 id, u32 p);
/* 
                  
                         
                                                  
                                            
  
                                                                       
                                                                         
                                                                        
                                                                          
                                 
 */
abehal_status abe_write_mixer(u32 id, s32 f_g, u32 f_ramp, u32 p);
/* 
                
                         
                                           
                                                    
  
 */
abehal_status abe_read_gain(u32 id, u32 *f_g, u32 p);
/* 
                 
                         
                             
                                            
  
                                                                       
                                                                         
                                                                        
                                                                          
                                 
 */
abehal_status abe_read_mixer(u32 id, u32 *f_g, u32 p);
/* 
                 
                                           
                              
  
                                                             
                                     
 */
abehal_status abe_mono_mixer(u32 id, u32 on_off);
/* 
                               
                          
                                 
                                            
  
                                                                            
                                                                           
                                                                          
                                                                            
                                         
 */
abehal_status abe_set_router_configuration(u32 id, u32 k, u32 *param);
/* 
                       
  
               
                                  
                               
  
               
                                                                      
                                                                            
           
                                                                              
  
                 
        
 */
abehal_status abe_read_debug_trace(u32 *data, u32 *n);
/* 
                          
                                         
  
                                                                    
                                                                       
 */
abehal_status abe_connect_debug_trace(abe_dma_t *dma2);
/* 
                      
                                             
  
                                                                       
 */
abehal_status abe_set_debug_trace(abe_dbg_t debug);
/* 
                                                          
  
                                                         
 */
void abe_init_mem(void __iomem **_io_base);

/* 
                                                                         
  
              
                                                         
                                                                          
                                                                            
  
 */
void abe_write_pdmdl_offset(u32 path, u32 offset_left, u32 offset_right);

#endif/*             */
