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

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/io.h>
#include <linux/module.h>
#include <linux/slab.h>

#include "abe_legacy.h"
#include "abe_dbg.h"
#include "abe_port.h"


struct omap_abe_equ {
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
};

#include "abe_gain.h"
#include "abe_aess.h"
#include "abe_seq.h"


int omap_abe_connect_debug_trace(struct omap_abe *abe,
				 struct omap_abe_dma *dma2);

int omap_abe_reset_hal(struct omap_abe *abe);
int omap_abe_load_fw(struct omap_abe *abe, u32 *firmware);
int omap_abe_reload_fw(struct omap_abe *abe, u32 *firmware);
u32* omap_abe_get_default_fw(struct omap_abe *abe);
int omap_abe_wakeup(struct omap_abe *abe);
int omap_abe_irq_processing(struct omap_abe *abe);
int omap_abe_clear_irq(struct omap_abe *abe);
int omap_abe_disable_irq(struct omap_abe *abe);
int omap_abe_set_debug_trace(struct omap_abe_dbg *dbg, int debug);
int omap_abe_set_ping_pong_buffer(struct omap_abe *abe,
						u32 port, u32 n_bytes);
int omap_abe_read_next_ping_pong_buffer(struct omap_abe *abe,
						u32 port, u32 *p, u32 *n);
int omap_abe_init_ping_pong_buffer(struct omap_abe *abe,
					u32 id, u32 size_bytes, u32 n_buffers,
					u32 *p);
int omap_abe_read_offset_from_ping_buffer(struct omap_abe *abe,
						u32 id, u32 *n);
int omap_abe_set_router_configuration(struct omap_abe *abe,
					u32 id, u32 k, u32 *param);
int omap_abe_set_opp_processing(struct omap_abe *abe, u32 opp);
int omap_abe_disable_data_transfer(struct omap_abe *abe, u32 id);
int omap_abe_enable_data_transfer(struct omap_abe *abe, u32 id);
int omap_abe_connect_cbpr_dmareq_port(struct omap_abe *abe,
						u32 id, abe_data_format_t *f,
						u32 d,
						abe_dma_t *returned_dma_t);
int omap_abe_connect_irq_ping_pong_port(struct omap_abe *abe,
					     u32 id, abe_data_format_t *f,
					     u32 subroutine_id, u32 size,
					     u32 *sink, u32 dsp_mcu_flag);
int omap_abe_connect_serial_port(struct omap_abe *abe,
				 u32 id, abe_data_format_t *f,
				 u32 mcbsp_id);
int omap_abe_read_port_address(struct omap_abe *abe,
			       u32 port, abe_dma_t *dma2);
int omap_abe_check_activity(struct omap_abe *abe);

int omap_abe_use_compensated_gain(struct omap_abe *abe, int on_off);
int omap_abe_write_equalizer(struct omap_abe *abe,
			     u32 id, struct omap_abe_equ *param);

int omap_abe_disable_gain(struct omap_abe *abe, u32 id, u32 p);
int omap_abe_enable_gain(struct omap_abe *abe, u32 id, u32 p);
int omap_abe_mute_gain(struct omap_abe *abe, u32 id, u32 p);
int omap_abe_unmute_gain(struct omap_abe *abe, u32 id, u32 p);

int omap_abe_write_gain(struct omap_abe *abe,
			u32 id, s32 f_g, u32 ramp, u32 p);
int omap_abe_write_mixer(struct omap_abe *abe,
			 u32 id, s32 f_g, u32 f_ramp, u32 p);
int omap_abe_read_gain(struct omap_abe *abe,
		       u32 id, u32 *f_g, u32 p);
int omap_abe_read_mixer(struct omap_abe *abe,
			u32 id, u32 *f_g, u32 p);

extern struct omap_abe *abe;

#if 0
/* 
                                                          
  
                                                         
 */
void abe_init_mem(void __iomem *_io_base)
{
	omap_abe_init_mem(abe, _io_base);
}
EXPORT_SYMBOL(abe_init_mem);

struct omap_abe* abe_probe_aess(void)
{
	return omap_abe_probe_aess(abe);
}
EXPORT_SYMBOL(abe_probe_aess);

void abe_remove_aess(void)
{
	omap_abe_remove_aess(abe);
}
EXPORT_SYMBOL(abe_remove_aess);

void abe_add_subroutine(u32 *id, abe_subroutine2 f,
						u32 nparam, u32 *params)
{
	omap_abe_add_subroutine(abe, id, f, nparam, params);
}
EXPORT_SYMBOL(abe_add_subroutine);

#endif

/* 
                                    
                          
                                     
  
                                                                   
                          
                                              
 */
u32 abe_reset_hal(void)
{
	omap_abe_reset_hal(abe);
	return 0;
}
EXPORT_SYMBOL(abe_reset_hal);

/* 
                                                          
  
 */
u32 abe_load_fw(u32 *firmware)
{
	omap_abe_load_fw(abe, firmware);
	return 0;
}
EXPORT_SYMBOL(abe_load_fw);

/* 
                                                              
  
 */
u32 abe_reload_fw(u32 *firmware)
{
	omap_abe_reload_fw(abe, firmware);
	return 0;
}
EXPORT_SYMBOL(abe_reload_fw);

u32* abe_get_default_fw(void)
{
	return omap_abe_get_default_fw(abe);
}
EXPORT_SYMBOL(abe_get_default_fw);

/* 
                          
  
                                  
 */
u32 abe_wakeup(void)
{
	omap_abe_wakeup(abe);
	return 0;
}
EXPORT_SYMBOL(abe_wakeup);

/* 
                                             
  
                                                                          
                                                                      
                                                                           
                                                                            
                                                                   
                                                                              
          
 */
u32 abe_irq_processing(void)
{
	omap_abe_irq_processing(abe);
	return 0;
}
EXPORT_SYMBOL(abe_irq_processing);

/* 
                                      
  
                                           
 */
u32 abe_clear_irq(void)
{
	omap_abe_clear_irq(abe);
	return 0;
}
EXPORT_SYMBOL(abe_clear_irq);

/* 
                                                  
  
                                               
 */
u32 abe_disable_irq(void)
{
	omap_abe_disable_irq(abe);

	return 0;
}
EXPORT_SYMBOL(abe_disable_irq);

/* 
                                                             
                                                        
  
                                                  
                                             
                                                                          
                                            
                                                                     
                                                     
  
                                                                      
                                                         
                                                                               
               
                                                                      
                                          
                                                             
                                        
 */
u32 abe_write_event_generator(u32 e) //                                  
{
	omap_abe_write_event_generator(abe, e);
	return 0;
}
EXPORT_SYMBOL(abe_write_event_generator);

/* 
                                                            
  
                                                                               
                                                               
                        
 */
u32 abe_stop_event_generator(void)
{
	omap_abe_stop_event_generator(abe);
	return 0;
}
EXPORT_SYMBOL(abe_stop_event_generator);

/* 
                          
                                         
  
                                                                    
                                                                      
 */
u32 abe_connect_debug_trace(abe_dma_t *dma2)
{
	omap_abe_connect_debug_trace(abe, (struct omap_abe_dma *)dma2);
	return 0;
}
EXPORT_SYMBOL(abe_connect_debug_trace);

/* 
                      
                                             
  
                                                                        
 */
u32 abe_set_debug_trace(abe_dbg_t debug)
{
	omap_abe_set_debug_trace(&abe->dbg, (int)(debug));
	return 0;
}
EXPORT_SYMBOL(abe_set_debug_trace);

/* 
                           
                     
                                     
  
                                                                      
                                                                           
 */
u32 abe_set_ping_pong_buffer(u32 port, u32 n_bytes)
{
	omap_abe_set_ping_pong_buffer(abe, port, n_bytes);
	return 0;
}
EXPORT_SYMBOL(abe_set_ping_pong_buffer);

/* 
                                 
                    
                                    
                                 
  
                                                                       
 */
u32 abe_read_next_ping_pong_buffer(u32 port, u32 *p, u32 *n)
{
	omap_abe_read_next_ping_pong_buffer(abe, port, p, n);
	return 0;
}
EXPORT_SYMBOL(abe_read_next_ping_pong_buffer);

/* 
                            
                   
                                    
                                               
                                                              
                                
  
                                                     
 */
u32 abe_init_ping_pong_buffer(u32 id, u32 size_bytes, u32 n_buffers,
					u32 *p)
{
	omap_abe_init_ping_pong_buffer(abe, id, size_bytes, n_buffers, p);
	return 0;
}
EXPORT_SYMBOL(abe_init_ping_pong_buffer);

/* 
                                   
                   
                                      
                                                  
  
                                                                 
                                                                             
 */
u32 abe_read_offset_from_ping_buffer(u32 id, u32 *n)
{
	omap_abe_read_offset_from_ping_buffer(abe, id, n);
	return 0;
}
EXPORT_SYMBOL(abe_read_offset_from_ping_buffer);

/* 
                      
                             
                                  
  
                                 
 */
u32 abe_write_equalizer(u32 id, abe_equ_t *param)
{
	omap_abe_write_equalizer(abe, id, (struct omap_abe_equ *)param);
	return 0;
}
EXPORT_SYMBOL(abe_write_equalizer);
/* 
                   
              
           
              
  
 */
u32 abe_disable_gain(u32 id, u32 p)
{
	omap_abe_disable_gain(abe, id, p);
	return 0;
}
EXPORT_SYMBOL(abe_disable_gain);
/* 
                  
              
           
              
  
 */
u32 abe_enable_gain(u32 id, u32 p)
{
	omap_abe_enable_gain(abe, id, p);
	return 0;
}
EXPORT_SYMBOL(abe_enable_gain);

/* 
                
              
           
              
  
 */
u32 abe_mute_gain(u32 id, u32 p)
{
	omap_abe_mute_gain(abe, id, p);
	return 0;
}
EXPORT_SYMBOL(abe_mute_gain);

/* 
                  
              
           
              
  
 */
u32 abe_unmute_gain(u32 id, u32 p)
{
	omap_abe_unmute_gain(abe, id, p);
	return 0;
}
EXPORT_SYMBOL(abe_unmute_gain);

/* 
                 
                               
                                         
                                
                                                     
  
                                                                        
                                                                         
                                                                             
                                                                     
                                 
 */
u32 abe_write_gain(u32 id, s32 f_g, u32 ramp, u32 p)
{
	omap_abe_write_gain(abe, id, f_g, ramp, p);
	return 0;
}
EXPORT_SYMBOL(abe_write_gain);

/* 
                  
                         
                                                  
                                            
  
                                                                       
                                                                         
                                                                        
                                                                          
                                 
 */
u32 abe_write_mixer(u32 id, s32 f_g, u32 f_ramp, u32 p)
{
	omap_abe_write_gain(abe, id, f_g, f_ramp, p);
	return 0;
}
EXPORT_SYMBOL(abe_write_mixer);

/* 
                
                         
                                           
                                                    
  
 */
u32 abe_read_gain(u32 id, u32 *f_g, u32 p)
{
	omap_abe_read_gain(abe, id, f_g, p);
	return 0;
}
EXPORT_SYMBOL(abe_read_gain);

/* 
                 
                         
                             
                                            
  
                                                                       
                                                                         
                                                                        
                                                                          
                                 
 */
u32 abe_read_mixer(u32 id, u32 *f_g, u32 p)
{
	omap_abe_read_gain(abe, id, f_g, p);
	return 0;
}
EXPORT_SYMBOL(abe_read_mixer);

/* 
                               
                          
                                 
                                            
  
                                                                            
                                                                           
                                        
  
                                                                              
  
                                                                
                                                                      
                                                                        
                              
                                                             
                                    
                                                                  
                                  
                                     
                              
 */
u32 abe_set_router_configuration(u32 id, u32 k, u32 *param)
{
	omap_abe_set_router_configuration(abe, id, k, param);
	return 0;
}
EXPORT_SYMBOL(abe_set_router_configuration);

/* 
                                                         
                  
  
                                  
                                                                                
                                                                      
                                          
                                             
  
                                                                            
                                                                             
               
  
 */
u32 abe_set_opp_processing(u32 opp)
{
	omap_abe_set_opp_processing(abe, opp);
	return 0;
}
EXPORT_SYMBOL(abe_set_opp_processing);

/* 
                            
                   
  
                                                          
                               
                                     
 */
u32 abe_disable_data_transfer(u32 id)
{
	omap_abe_disable_data_transfer(abe, id);
	return 0;
}
EXPORT_SYMBOL(abe_disable_data_transfer);

/* 
                           
                   
  
                             
                     
                               
 */
u32 abe_enable_data_transfer(u32 id)
{
	omap_abe_enable_data_transfer(abe, id);
	return 0;
}
EXPORT_SYMBOL(abe_enable_data_transfer);

/* 
                               
                 
                          
                                      
                                                                              
                                            
  
                                                                 
                          
 */
u32 abe_connect_cbpr_dmareq_port(u32 id, abe_data_format_t *f, u32 d,
					   abe_dma_t *returned_dma_t)
{
	omap_abe_connect_cbpr_dmareq_port(abe, id, f, d, returned_dma_t);
	return 0;
}
EXPORT_SYMBOL(abe_connect_cbpr_dmareq_port);

/* 
                                 
                 
                          
                                                
                              
                                                        
  
                                                                
                                                                       
                                                                        
                                                                        
                                                                        
               
 */
u32 abe_connect_irq_ping_pong_port(u32 id, abe_data_format_t *f,
					     u32 subroutine_id, u32 size,
					     u32 *sink, u32 dsp_mcu_flag)
{
	omap_abe_connect_irq_ping_pong_port(abe, id, f, subroutine_id, size,
					     sink, dsp_mcu_flag);
	return 0;
}
EXPORT_SYMBOL(abe_connect_irq_ping_pong_port);

/* 
                            
                 
                  
                                       
  
                                                                              
                                                                          
                                                                      
                      
 */
u32 abe_connect_serial_port(u32 id, abe_data_format_t *f,
				      u32 mcbsp_id)
{
	omap_abe_connect_serial_port(abe, id, f, mcbsp_id);
	return 0;
}
EXPORT_SYMBOL(abe_connect_serial_port);

/* 
                        
                                                                         
  
                                                                            
                                                                        
                     
 */
u32 abe_read_port_address(u32 port, abe_dma_t *dma2)
{
	omap_abe_read_port_address(abe, port, dma2);
	return 0;
}
EXPORT_SYMBOL(abe_read_port_address);

/* 
                                                   
  
                                      
                                  
                                                                    
  
 */
u32 abe_check_activity(void)
{
	return (u32)omap_abe_check_activity(abe);
}
EXPORT_SYMBOL(abe_check_activity);
/* 
                           
           
  
                                                
                                                               
                                                          
 */
abehal_status abe_use_compensated_gain(u32 on_off)
{
	omap_abe_use_compensated_gain(abe, (int)(on_off));
	return 0;
}
EXPORT_SYMBOL(abe_use_compensated_gain);
