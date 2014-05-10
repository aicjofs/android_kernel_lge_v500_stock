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

#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/init.h>
#include <linux/err.h>
#include <linux/slab.h>

#include "abe_dbg.h"
#include "abe.h"
#include "abe_gain.h"
#include "abe_aess.h"
#include "abe_port.h"
#include "abe_mem.h"
#include "abe_taskid.h"

#define OMAP_ABE_IRQ_FIFO_MASK ((OMAP_ABE_D_MCUIRQFIFO_SIZE >> 2) - 1)

void abe_init_asrc_vx_dl(s32 dppm);
void abe_init_asrc_vx_ul(s32 dppm);
void abe_init_asrc_mm_ext_in(s32 dppm);
void abe_init_asrc_bt_ul(s32 dppm);
void abe_init_asrc_bt_dl(s32 dppm);

void abe_irq_aps(u32 aps_info);
void abe_irq_ping_pong(void);
void abe_irq_check_for_sequences(u32 seq_info);
extern u32 abe_size_pingpong;
extern u32 abe_base_address_pingpong[];

void abe_add_subroutine(u32 *id, abe_subroutine2 f, u32 nparam, u32 *params);


/* 
                                             
                              
  
                                                                   
                          
                                              
 */
int omap_abe_reset_hal(struct omap_abe *abe)
{
	u32 i;

	omap_abe_dbg_reset(&abe->dbg);

	_log(ABE_ID_RESET_HAL, 0, 0, 0);

	/*                                         */
	abe->irq_dbg_read_ptr = 0;

	/*                                                     */
	omap_abe_use_compensated_gain(abe, 0);

	/*                               */
	for (i = 0; i < MAX_NBGAIN_CMEM; i++) {
		abe->muted_gains_indicator[i] = 0;
		abe->desired_gains_decibel[i] = (u32) GAIN_MUTE;
		abe->desired_gains_linear[i] = 0;
		abe->desired_ramp_delay_ms[i] = 0;
		abe->muted_gains_decibel[i] = (u32) GAIN_TOOLOW;
	}
	omap_abe_hw_configuration(abe);
	return 0;
}
EXPORT_SYMBOL(omap_abe_reset_hal);

/* 
                               
                              
  
                                  
 */
int omap_abe_wakeup(struct omap_abe *abe)
{
	/*                         */
	omap_abe_write_event_generator(abe, EVENT_TIMER);

	/*                                            */
	omap_abe_hw_configuration(abe);
	return 0;
}
EXPORT_SYMBOL(omap_abe_wakeup);

/* 
                 
  
                                            
 */
void abe_monitoring(void)
{
}

/* 
                                                  
                              
  
                                                                          
                                                                      
                                                                           
                                                                            
                                                                   
                                                                              
          
 */
int omap_abe_irq_processing(struct omap_abe *abe)
{
	u32 abe_irq_dbg_write_ptr, i, cmem_src, sm_cm;
	abe_irq_data_t IRQ_data;

	_log(ABE_ID_IRQ_PROCESSING, 0, 0, 0);

	/*                                                                   */
	/*                                            */
	cmem_src = MCU_IRQ_FIFO_ptr_labelID << 2;
	omap_abe_mem_read(abe, OMAP_ABE_CMEM, cmem_src,
			&sm_cm, sizeof(abe_irq_dbg_write_ptr));
	/*                                         */
	abe_irq_dbg_write_ptr = sm_cm >> 16;
	abe_irq_dbg_write_ptr &= 0xFF;
	/*                              */
	for (i = 0; i < OMAP_ABE_D_MCUIRQFIFO_SIZE; i++) {
		/*                             */
		if (abe_irq_dbg_write_ptr == abe->irq_dbg_read_ptr)
			break;
		/*                       */
		omap_abe_mem_read(abe, OMAP_ABE_DMEM,
			       (OMAP_ABE_D_MCUIRQFIFO_ADDR +
				(abe->irq_dbg_read_ptr << 2)),
			       (u32 *) &IRQ_data, sizeof(IRQ_data));
		abe->irq_dbg_read_ptr = (abe->irq_dbg_read_ptr + 1) & OMAP_ABE_IRQ_FIFO_MASK;
		/*                                    */
		switch (IRQ_data.tag) {
		case IRQtag_APS:
			_log(ABE_ID_IRQ_PROCESSING, IRQ_data.data, 0, 1);
			abe_irq_aps(IRQ_data.data);
			break;
		case IRQtag_PP:
			_log(ABE_ID_IRQ_PROCESSING, 0, 0, 2);
			abe_irq_ping_pong();
			break;
		case IRQtag_COUNT:
			_log(ABE_ID_IRQ_PROCESSING, IRQ_data.data, 0, 3);
			abe_irq_check_for_sequences(IRQ_data.data);
			break;
		default:
			break;
		}
	}
	abe_monitoring();
	return 0;
}
EXPORT_SYMBOL(omap_abe_irq_processing);

/* 
                                
                              
                     
                                     
  
                                                                      
                                                                           
 */
int omap_abe_set_ping_pong_buffer(struct omap_abe *abe, u32 port, u32 n_bytes)
{
	u32 sio_pp_desc_address, struct_offset, n_samples, datasize,
		base_and_size, *src;
	struct ABE_SPingPongDescriptor desc_pp;

	_log(ABE_ID_SET_PING_PONG_BUFFER, port, n_bytes, n_bytes >> 8);

	/*                                      */
	if (port != OMAP_ABE_MM_DL_PORT) {
		omap_abe_dbg_error(abe, OMAP_ABE_ERR_API,
				   ABE_PARAMETER_ERROR);
	}
	/*                                           */
	/*                         */
	datasize = omap_abe_dma_port_iter_factor((struct omap_abe_data_format *)&((abe_port[port]).format));
	/*                    */
	datasize = datasize << 2;
	n_samples = n_bytes / datasize;
	omap_abe_mem_read(abe, OMAP_ABE_DMEM, OMAP_ABE_D_PINGPONGDESC_ADDR,
			(u32 *) &desc_pp, sizeof(desc_pp));
	/*
                                                                
                                     
  */
	if ((desc_pp.counter & 0x1) == 0) {
		struct_offset = (u32) &(desc_pp.nextbuff0_BaseAddr) -
			(u32) &(desc_pp);
		base_and_size = desc_pp.nextbuff0_BaseAddr;
	} else {
		struct_offset = (u32) &(desc_pp.nextbuff1_BaseAddr) -
			(u32) &(desc_pp);
		base_and_size = desc_pp.nextbuff1_BaseAddr;
	}
	base_and_size = (base_and_size & 0xFFFFL) + (n_samples << 16);
	sio_pp_desc_address = OMAP_ABE_D_PINGPONGDESC_ADDR + struct_offset;
	src = &base_and_size;
	omap_abe_mem_write(abe, OMAP_ABE_DMEM, sio_pp_desc_address,
			(u32 *) &base_and_size, sizeof(u32));

	return 0;
}
EXPORT_SYMBOL(omap_abe_set_ping_pong_buffer);

/* 
                                      
                              
                    
                                    
                                 
  
                                                                       
 */
int omap_abe_read_next_ping_pong_buffer(struct omap_abe *abe, u32 port, u32 *p, u32 *n)
{
	u32 sio_pp_desc_address;
	struct ABE_SPingPongDescriptor desc_pp;

	_log(ABE_ID_READ_NEXT_PING_PONG_BUFFER, port, 0, 0);

	/*                                      */
	if (port != OMAP_ABE_MM_DL_PORT) {
		omap_abe_dbg_error(abe, OMAP_ABE_ERR_API,
				   ABE_PARAMETER_ERROR);
	}
	/*                                                             
                                      */
	sio_pp_desc_address = OMAP_ABE_D_PINGPONGDESC_ADDR;
	omap_abe_mem_read(abe, OMAP_ABE_DMEM, sio_pp_desc_address,
			(u32 *) &desc_pp, sizeof(struct ABE_SPingPongDescriptor));
	if ((desc_pp.counter & 0x1) == 0) {
		_log(ABE_ID_READ_NEXT_PING_PONG_BUFFER, port, 0, 0);
		*p = desc_pp.nextbuff0_BaseAddr;
	} else {
		_log(ABE_ID_READ_NEXT_PING_PONG_BUFFER, port, 1, 0);
		*p = desc_pp.nextbuff1_BaseAddr;
	}
	/*                                           */
	*n = abe_size_pingpong;

	return 0;
}
EXPORT_SYMBOL(omap_abe_read_next_ping_pong_buffer);

/* 
                                 
                              
                   
                                    
                                               
                                                              
                                
  
                                                     
 */
int omap_abe_init_ping_pong_buffer(struct omap_abe *abe,
				   u32 id, u32 size_bytes, u32 n_buffers,
				   u32 *p)
{
	u32 i, dmem_addr;

	_log(ABE_ID_INIT_PING_PONG_BUFFER, id, size_bytes, n_buffers);

	/*                                                                   
                                        */
	if (id != OMAP_ABE_MM_DL_PORT || n_buffers > MAX_PINGPONG_BUFFERS) {
		omap_abe_dbg_error(abe, OMAP_ABE_ERR_API,
				   ABE_PARAMETER_ERROR);
	}
	for (i = 0; i < n_buffers; i++) {
		dmem_addr = OMAP_ABE_D_PING_ADDR + (i * size_bytes);
		/*                                                    */
		abe_base_address_pingpong[i] = dmem_addr;
	}
	/*             */
	abe_size_pingpong = size_bytes;
	*p = (u32) OMAP_ABE_D_PING_ADDR;
	return 0;
}
EXPORT_SYMBOL(omap_abe_init_ping_pong_buffer);

/* 
                                        
                              
                   
                                      
                                                  
  
                                                                 
                                                                             
 */
int omap_abe_read_offset_from_ping_buffer(struct omap_abe *abe,
					  u32 id, u32 *n)
{
	u32 sio_pp_desc_address;
	struct ABE_SPingPongDescriptor desc_pp;

	/*                                      */
	if (OMAP_ABE_MM_DL_PORT != id) {
		omap_abe_dbg_error(abe, OMAP_ABE_ERR_API,
				   ABE_PARAMETER_ERROR);
	} else {
		/*                                        */
		sio_pp_desc_address = OMAP_ABE_D_PINGPONGDESC_ADDR;
		omap_abe_mem_read(abe, OMAP_ABE_DMEM,
			       sio_pp_desc_address, (u32 *) &desc_pp,
			       sizeof(struct ABE_SPingPongDescriptor));
		/*                                                           
                              */
		if ((desc_pp.counter & 0x1) == 0) {
			/*                                                   */
			switch (abe_port[OMAP_ABE_MM_DL_PORT].format.samp_format) {
			case MONO_MSB:
			case MONO_RSHIFTED_16:
			case STEREO_16_16:
				*n = abe_size_pingpong / 4 +
					desc_pp.nextbuff1_Samples -
					desc_pp.workbuff_Samples;
				break;
			case STEREO_MSB:
			case STEREO_RSHIFTED_16:
				*n = abe_size_pingpong / 8 +
					desc_pp.nextbuff1_Samples -
					desc_pp.workbuff_Samples;
				break;
			default:
				omap_abe_dbg_error(abe, OMAP_ABE_ERR_API,
						   ABE_PARAMETER_ERROR);
				break;
			}
		} else {
			/*                                                   */
			*n = desc_pp.nextbuff0_Samples -
				desc_pp.workbuff_Samples;
		}
	}

	return 0;
}
EXPORT_SYMBOL(omap_abe_read_offset_from_ping_buffer);

/* 
                               
                          
                                 
                                            
  
                                                                            
                                                                           
                                        
  
                                                                              
  
                                                                
                                                                      
                                                                        
                              
                                                             
                                    
                                                                  
                                  
                                     
                              
 */
int omap_abe_set_router_configuration(struct omap_abe *abe,
				      u32 id, u32 k, u32 *param)
{
	_log(ABE_ID_SET_ROUTER_CONFIGURATION, id, (u32) param, (u32) param >> 8);

	omap_abe_mem_write(abe, OMAP_ABE_DMEM,
			       OMAP_ABE_D_AUPLINKROUTING_ADDR,
			       param, OMAP_ABE_D_AUPLINKROUTING_SIZE);
	return 0;
}
EXPORT_SYMBOL(omap_abe_set_router_configuration);

/* 
                                                         
                  
  
                                  
                                                                                
                                                                      
                                          
                                             
  
                                                                            
                                                                             
               
  
 */
int omap_abe_set_opp_processing(struct omap_abe *abe, u32 opp)
{
	u32 dOppMode32, sio_desc_address;
	struct ABE_SIODescriptor sio_desc;

	_log(ABE_ID_SET_OPP_PROCESSING, opp, 0, 0);

	switch (opp) {
	case ABE_OPP25:
		/*        */
		dOppMode32 = DOPPMODE32_OPP25;
		break;
	case ABE_OPP50:
		/*        */
		dOppMode32 = DOPPMODE32_OPP50;
		break;
	default:
		omap_abe_dbg_error(abe, OMAP_ABE_ERR_API,
				   ABE_BLOCK_COPY_ERR);
	case ABE_OPP100:
		/*         */
		dOppMode32 = DOPPMODE32_OPP100;
		break;
	}
	/*                              */
	omap_abe_mem_write(abe, OMAP_ABE_DMEM,
		       OMAP_ABE_D_MAXTASKBYTESINSLOT_ADDR, &dOppMode32, sizeof(u32));

	sio_desc_address = OMAP_ABE_D_IODESCR_ADDR + (OMAP_ABE_MM_EXT_IN_PORT *
				sizeof(struct ABE_SIODescriptor));
	omap_abe_mem_read(abe, OMAP_ABE_DMEM, sio_desc_address,
			(u32 *) &sio_desc, sizeof(sio_desc));
	if (dOppMode32 == DOPPMODE32_OPP100) {
		/*                            */
		sio_desc.smem_addr1 = smem_mm_ext_in_opp100;
		/*                                               */
		abe_init_asrc_mm_ext_in(250);
	} else
		/*                           */
		sio_desc.smem_addr1 = smem_mm_ext_in_opp50;

	omap_abe_mem_write(abe, OMAP_ABE_DMEM, sio_desc_address,
		       (u32 *) &sio_desc, sizeof(sio_desc));

	sio_desc_address = OMAP_ABE_D_IODESCR_ADDR + (OMAP_ABE_BT_VX_UL_PORT *
				sizeof(struct ABE_SIODescriptor));
	omap_abe_mem_read(abe, OMAP_ABE_DMEM, sio_desc_address,
			(u32 *) &sio_desc, sizeof(sio_desc));

	if (abe_port[OMAP_ABE_BT_VX_UL_PORT].format.f == 8000) {
		if (dOppMode32 == DOPPMODE32_OPP100)
			/*                            */
			sio_desc.smem_addr1 = smem_bt_vx_ul_opp100;
		else
			/*                        */
			sio_desc.smem_addr1 = BT_UL_8k_labelID;
	} else {
		if (dOppMode32 == DOPPMODE32_OPP100)
			/*                            */
			sio_desc.smem_addr1 = smem_bt_vx_ul_opp100;
		else
			/*                        */
			sio_desc.smem_addr1 = BT_UL_16k_labelID;
	}

	omap_abe_mem_write(abe, OMAP_ABE_DMEM, sio_desc_address,
		       (u32 *) &sio_desc, sizeof(sio_desc));

	sio_desc_address = OMAP_ABE_D_IODESCR_ADDR + (OMAP_ABE_BT_VX_DL_PORT *
				sizeof(struct ABE_SIODescriptor));
	omap_abe_mem_read(abe, OMAP_ABE_DMEM, sio_desc_address,
			(u32 *) &sio_desc, sizeof(sio_desc));

#define ABE_TASK_ID(ID) (OMAP_ABE_D_TASKSLIST_ADDR + sizeof(ABE_STask)*(ID))
#define TASK_BT_DL_48_8_SLT 14
#define TASK_BT_DL_48_8_IDX 4
	if (abe_port[OMAP_ABE_BT_VX_DL_PORT].format.f == 8000) {
		if (dOppMode32 == DOPPMODE32_OPP100) {
			abe->MultiFrame[TASK_BT_DL_48_8_SLT][TASK_BT_DL_48_8_IDX] =
				ABE_TASK_ID(C_ABE_FW_TASK_BT_DL_48_8_OPP100);
			sio_desc.smem_addr1 = BT_DL_8k_opp100_labelID;
		} else {
			abe->MultiFrame[TASK_BT_DL_48_8_SLT][TASK_BT_DL_48_8_IDX] =
				ABE_TASK_ID(C_ABE_FW_TASK_BT_DL_48_8);
			sio_desc.smem_addr1 = BT_DL_8k_labelID;
		}
	} else {
		if (dOppMode32 == DOPPMODE32_OPP100) {
			abe->MultiFrame[TASK_BT_DL_48_8_SLT][TASK_BT_DL_48_8_IDX] =
				ABE_TASK_ID(C_ABE_FW_TASK_BT_DL_48_16_OPP100);
			sio_desc.smem_addr1 = BT_DL_16k_opp100_labelID;
		} else {
			abe->MultiFrame[TASK_BT_DL_48_8_SLT][TASK_BT_DL_48_8_IDX] =
				ABE_TASK_ID(C_ABE_FW_TASK_BT_DL_48_16);
			sio_desc.smem_addr1 = BT_DL_16k_labelID;
		}
	}

	omap_abe_mem_write(abe, OMAP_ABE_DMEM, OMAP_ABE_D_MULTIFRAME_ADDR,
		       (u32 *) abe->MultiFrame, sizeof(abe->MultiFrame));

	omap_abe_mem_write(abe, OMAP_ABE_DMEM, sio_desc_address,
		       (u32 *) &sio_desc, sizeof(sio_desc));

	if (dOppMode32 == DOPPMODE32_OPP100) {
		/*                                              */
		abe_init_asrc_bt_ul(250);
		/*                                              */
		abe_init_asrc_bt_dl(-250);
	}
	return 0;

}
EXPORT_SYMBOL(omap_abe_set_opp_processing);

/* 
                                                        
  
                                      
                                  
                                                                    
  
 */
int omap_abe_check_activity(struct omap_abe *abe)
{
	int i, ret = 0;

	for (i = 0; i < (LAST_PORT_ID - 1); i++) {
		if (abe_port[abe_port_priority[i]].status ==
				OMAP_ABE_PORT_ACTIVITY_RUNNING)
			break;
	}
	if (i < (LAST_PORT_ID - 1))
		ret = 1;
	return ret;
}
EXPORT_SYMBOL(omap_abe_check_activity);

/* 
                      
                                                               
                                        
                                              
                                 
  
                                                       
 */
abehal_status abe_plug_subroutine(u32 *id, abe_subroutine2 f, u32 n,
				  u32 *params)
{
	_log(ABE_ID_PLUG_SUBROUTINE, (u32) (*id), (u32) f, n);

	abe_add_subroutine(id, (abe_subroutine2) f, n, (u32 *) params);
	return 0;
}
EXPORT_SYMBOL(abe_plug_subroutine);
