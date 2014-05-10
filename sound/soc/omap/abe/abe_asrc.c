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

#include "abe_legacy.h"
#include "abe_dbg.h"

#include "abe_typedef.h"
#include "abe_initxxx_labels.h"
#include "abe_dbg.h"
#include "abe_mem.h"
#include "abe_sm_addr.h"
#include "abe_cm_addr.h"

/* 
                 
                                               
                                                                            
                                  
                               
                                                        
  
                                              
                                                        
 */
void abe_write_fifo(u32 memory_bank, u32 descr_addr, u32 *data, u32 nb_data32)
{
	u32 fifo_addr[4];
	u32 i;
	/*                                */
	omap_abe_mem_read(abe, OMAP_ABE_DMEM, descr_addr,
		       &fifo_addr[0], 4 * sizeof(u32));
	/*                                */
	if (fifo_addr[1] < fifo_addr[2])
		omap_abe_dbg_error(abe, OMAP_ABE_ERR_DBG,
				   ABE_FW_FIFO_WRITE_PTR_ERR);
	/*                              */
	if (fifo_addr[1] > fifo_addr[3])
		omap_abe_dbg_error(abe, OMAP_ABE_ERR_DBG,
				   ABE_FW_FIFO_WRITE_PTR_ERR);
	switch (memory_bank) {
	case ABE_DMEM:
		for (i = 0; i < nb_data32; i++) {
			omap_abe_mem_write(abe, OMAP_ABE_DMEM,
				       (s32) fifo_addr[1], (u32 *) (data + i),
				       4);
			/*                         */
			fifo_addr[1] = fifo_addr[1] + 4;
			if (fifo_addr[1] > fifo_addr[3])
				fifo_addr[1] = fifo_addr[2];
			if (fifo_addr[1] == fifo_addr[0])
				omap_abe_dbg_error(abe, OMAP_ABE_ERR_DBG,
						   ABE_FW_FIFO_WRITE_PTR_ERR);
		}
		/*                              */
		omap_abe_mem_write(abe, OMAP_ABE_DMEM, descr_addr +
			       sizeof(u32), &fifo_addr[1], 4);
		break;
	default:
		break;
	}
}

/* 
                 
                        
                                          
  
                                                                         
                                                                       
                                                                       
                                                                            
                                                                             
                                        
 */
abehal_status abe_write_asrc(u32 port, s32 dppm)
{
	s32 dtempvalue, adppm, drift_sign, drift_sign_addr, alpha_params_addr;
	s32 alpha_params[3];
	_log(ABE_ID_WRITE_ASRC, port, dppm, dppm >> 8);
	/*
           
   
                                      
                                                                         
                              
                     
                                       
   
             
                      
                                            
  */
	/*                        */
	/*                                    */
	/*                                             */
	/*                                                  */
	/*                                                  */
	/*                     */
	/*                                   */
	/*                                             */
	/*                                                  */
	/*                                                  */
	/*                               */
	if (dppm >= 0) {
		/*             */
		drift_sign = 1;
		adppm = dppm;
	} else {
		/*             */
		drift_sign = -1;
		adppm = (-1 * dppm);
	}
	if (dppm == 0) {
		/*             */
		alpha_params[0] = 0;
		/*                  */
		alpha_params[1] = 0;
		/*                  */
		alpha_params[2] = 0x003ffff0;
	} else {
		dtempvalue = (adppm << 4) + adppm - ((adppm * 3481L) / 15625L);
		/*             */
		alpha_params[0] = dtempvalue << 2;
		/*                  */
		alpha_params[1] = (-dtempvalue) << 2;
		/*                  */
		alpha_params[2] = (0x00100000 - (dtempvalue / 2)) << 2;
	}
	switch (port) {
	/*                                                              */
	case OMAP_ABE_VX_DL_PORT:
		drift_sign_addr = OMAP_ABE_D_ASRCVARS_DL_VX_ADDR + (1 * sizeof(s32));
		alpha_params_addr = OMAP_ABE_D_ASRCVARS_DL_VX_ADDR + (3 * sizeof(s32));
		break;
	/*                                                                */
	case OMAP_ABE_VX_UL_PORT:
		drift_sign_addr = OMAP_ABE_D_ASRCVARS_UL_VX_ADDR + (1 * sizeof(s32));
		alpha_params_addr = OMAP_ABE_D_ASRCVARS_UL_VX_ADDR + (3 * sizeof(s32));
		break;
	/*                                                       */
	case OMAP_ABE_BT_VX_UL_PORT:
		drift_sign_addr = OMAP_ABE_D_ASRCVARS_BT_UL_ADDR + (1 * sizeof(s32));
		alpha_params_addr = OMAP_ABE_D_ASRCVARS_BT_UL_ADDR + (3 * sizeof(s32));
		break;
	/*                                                       */
	case OMAP_ABE_BT_VX_DL_PORT:
		drift_sign_addr = OMAP_ABE_D_ASRCVARS_BT_DL_ADDR + (1 * sizeof(s32));
		alpha_params_addr = OMAP_ABE_D_ASRCVARS_BT_DL_ADDR + (3 * sizeof(s32));
		break;
	default:
	/*                                                           */
	case OMAP_ABE_MM_EXT_IN_PORT:
		drift_sign_addr = OMAP_ABE_D_ASRCVARS_MM_EXT_IN_ADDR + (1 * sizeof(s32));
		alpha_params_addr =
			OMAP_ABE_D_ASRCVARS_MM_EXT_IN_ADDR + (3 * sizeof(s32));
		break;
	}
	omap_abe_mem_write(abe, OMAP_ABE_DMEM, drift_sign_addr,
		       (u32 *) &drift_sign, 4);
	omap_abe_mem_write(abe, OMAP_ABE_DMEM, alpha_params_addr,
		       (u32 *) &alpha_params[0], 12);
	return 0;
}
EXPORT_SYMBOL(abe_write_asrc);
/* 
                      
  
                                                   
                                         
                                  
                                  
                                                      
                                       
                                                       
                                                        
                                                                      
                                                                    
                                                                 
                                   
                                       
                                        
                                                                      
                                                  
                                     
                                     
                                                                          
                                     
                                     
                  
                                               
                                         
 */
void abe_init_asrc_vx_dl(s32 dppm)
{
	s32 el[45];
	s32 temp0, temp1, adppm, dtemp, mem_tag, mem_addr;
	u32 i = 0;
	u32 n_fifo_el = 42;
	temp0 = 0;
	temp1 = 1;
	/*                                  */
	mem_tag = ABE_DMEM;
	mem_addr = OMAP_ABE_D_ASRCVARS_DL_VX_ADDR + (1 * sizeof(s32));
	el[i] = (mem_tag << 16) + mem_addr;
	if (dppm >= 0) {
		el[i + 1] = 1;
		adppm = dppm;
	} else {
		el[i + 1] = -1;
		adppm = (-1 * dppm);
	}
	/*             */
	el[i + 2] = temp0;
	i = i + 3;
	dtemp = (adppm << 4) + adppm - ((adppm * 3481L) / 15625L);
	/*                                 */
	mem_tag = ABE_DMEM;
	mem_addr = OMAP_ABE_D_ASRCVARS_DL_VX_ADDR + (2 * sizeof(s32));
	el[i] = (mem_tag << 16) + mem_addr;
	el[i + 1] = temp0;
	/*             */
	el[i + 2] = temp0;
	i = i + 3;
	/*                                   */
	mem_tag = ABE_DMEM;
	mem_addr = OMAP_ABE_D_ASRCVARS_DL_VX_ADDR + (3 * sizeof(s32));
	el[i] = (mem_tag << 16) + mem_addr;
	if (dppm == 0)
		el[i + 1] = 0;
	else
		el[i + 1] = dtemp << 2;
	/*             */
	el[i + 2] = temp0;
	i = i + 3;
	/*                                        */
	mem_tag = ABE_DMEM;
	mem_addr = OMAP_ABE_D_ASRCVARS_DL_VX_ADDR + (4 * sizeof(s32));
	el[i] = (mem_tag << 16) + mem_addr;
	if (dppm == 0)
		el[i + 1] = 0;
	else
		el[i + 1] = (-dtemp) << 2;
	/*             */
	el[i + 2] = temp0;
	i = i + 3;
	/*                                                */
	mem_tag = ABE_DMEM;
	mem_addr = OMAP_ABE_D_ASRCVARS_DL_VX_ADDR + (5 * sizeof(s32));
	el[i] = (mem_tag << 16) + mem_addr;
	if (dppm == 0)
		el[i + 1] = 0x00400000;
	else
		el[i + 1] = (0x00100000 - (dtemp / 2)) << 2;
	/*             */
	el[i + 2] = temp0;
	i = i + 3;
	/*                                   */
	mem_tag = ABE_CMEM;
	mem_addr = OMAP_ABE_C_ALPHACURRENT_DL_VX_ADDR;
	el[i] = (mem_tag << 16) + mem_addr;
	el[i + 1] = 0x00000020;
	/*             */
	el[i + 2] = temp0;
	i = i + 3;
	/*                                  */
	mem_tag = ABE_CMEM;
	mem_addr = OMAP_ABE_C_BETACURRENT_DL_VX_ADDR;
	el[i] = (mem_tag << 16) + mem_addr;
	el[i + 1] = 0x003fffe0;
	/*             */
	el[i + 2] = temp0;
	i = i + 3;
	/*                                  */
	mem_tag = ABE_DMEM;
	mem_addr = OMAP_ABE_D_IODESCR_ADDR + (OMAP_ABE_VX_DL_PORT * sizeof(struct ABE_SIODescriptor))
		+ drift_asrc_;
	el[i] = (mem_tag << 16) + mem_addr;
	el[i + 1] = temp0;
	/*             */
	el[i + 2] = temp0;
	i = i + 3;
	/*                                      */
	/*                                                                    
                                                  */
	mem_tag = ABE_SMEM;
	mem_addr = ASRC_DL_VX_Coefs_labelID;
	el[i] = (mem_tag << 16) + (mem_addr << 2);
	if (dppm == 0) {
		el[i + 1] = OMAP_ABE_C_COEFASRC16_VX_ADDR >> 2;
		el[i + 1] = (el[i + 1] << 8) + (OMAP_ABE_C_COEFASRC16_VX_SIZE >> 2);
		el[i + 2] = OMAP_ABE_C_COEFASRC15_VX_ADDR >> 2;
		el[i + 2] = (el[i + 2] << 8) + (OMAP_ABE_C_COEFASRC15_VX_SIZE >> 2);
	} else {
		el[i + 1] = OMAP_ABE_C_COEFASRC1_VX_ADDR >> 2;
		el[i + 1] = (el[i + 1] << 8) + (OMAP_ABE_C_COEFASRC1_VX_SIZE >> 2);
		el[i + 2] = OMAP_ABE_C_COEFASRC2_VX_ADDR >> 2;
		el[i + 2] = (el[i + 2] << 8) + (OMAP_ABE_C_COEFASRC2_VX_SIZE >> 2);
	}
	i = i + 3;
	/*                                       */
	/*                                                                    
                                                  */
	mem_tag = ABE_CMEM;
	mem_addr = ASRC_DL_VX_Coefs_labelID;
	el[i] = (mem_tag << 16) + (mem_addr << 2);
	/*                                                */
	el[i + 1] = (temp0 << 16) + (temp1 << 12) + (temp0 << 4) + temp1;
	/*             */
	el[i + 2] = temp0;
	i = i + 3;
	/*                                    */
	/*                
                                                          */
	mem_tag = ABE_SMEM;
	mem_addr = XinASRC_DL_VX_labelID;
	el[i] = (mem_tag << 16) + (mem_addr << 2);
	el[i + 1] = OMAP_ABE_S_XINASRC_DL_VX_ADDR >> 3;
	el[i + 1] = (el[i + 1] << 8) + (OMAP_ABE_S_XINASRC_DL_VX_SIZE >> 3);
	el[i + 2] = temp0;
	i = i + 3;
	/*                                    */
	/*                
                                                          */
	mem_tag = ABE_CMEM;
	mem_addr = XinASRC_DL_VX_labelID;
	el[i] = (mem_tag << 16) + (mem_addr << 2);
	/*                                                */
	el[i + 1] = (temp0 << 16) + (temp1 << 12) + (temp0 << 4) + temp0;
	/*             */
	el[i + 2] = temp0;
	i = i + 3;
	/*                                    */
	/*                                                             
                                           */
	mem_tag = ABE_SMEM;
	mem_addr = IO_VX_DL_ASRC_labelID;
	el[i] = (mem_tag << 16) + (mem_addr << 2);
	el[i + 1] = OMAP_ABE_S_XINASRC_DL_VX_ADDR >> 3;
	el[i + 1] = (el[i + 1] << 8) + (OMAP_ABE_S_XINASRC_DL_VX_SIZE >> 3);
	el[i + 2] = temp0;
	i = i + 3;
	/*                                    */
	/*                                                             
                                           */
	mem_tag = ABE_CMEM;
	mem_addr = IO_VX_DL_ASRC_labelID;
	el[i] = (mem_tag << 16) + (mem_addr << 2);
	/*                                                */
	el[i + 1] = ((ASRC_DL_VX_FIR_L + ASRC_margin) << 16) + (temp1 << 12)
		+ (temp0 << 4) + temp0;
	/*             */
	el[i + 2] = temp0;
	abe_write_fifo(ABE_DMEM, OMAP_ABE_D_FWMEMINITDESCR_ADDR, (u32 *) &el[0],
		       n_fifo_el);
}
/* 
                      
  
                                                   
                                         
                                  
                                  
                                                      
                                       
                                                       
                                                        
                                                                      
                                                                    
                                                                 
                                   
                                       
                                        
                                                                      
                                                  
                                     
                                     
                                                                          
                                   
                                   
                                                             
                                         
                                    
                                    
                                                              
                                         
 */
void abe_init_asrc_vx_ul(s32 dppm)
{
	s32 el[51];
	s32 temp0, temp1, adppm, dtemp, mem_tag, mem_addr;
	u32 i = 0;
	u32 n_fifo_el = 48;
	temp0 = 0;
	temp1 = 1;
	/*                                  */
	mem_tag = ABE_DMEM;
	mem_addr = OMAP_ABE_D_ASRCVARS_UL_VX_ADDR + (1 * sizeof(s32));
	el[i] = (mem_tag << 16) + mem_addr;
	if (dppm >= 0) {
		el[i + 1] = 1;
		adppm = dppm;
	} else {
		el[i + 1] = -1;
		adppm = (-1 * dppm);
	}
	/*             */
	el[i + 2] = temp0;
	i = i + 3;
	dtemp = (adppm << 4) + adppm - ((adppm * 3481L) / 15625L);
	/*                                 */
	mem_tag = ABE_DMEM;
	mem_addr = OMAP_ABE_D_ASRCVARS_UL_VX_ADDR + (2 * sizeof(s32));
	el[i] = (mem_tag << 16) + mem_addr;
	el[i + 1] = temp0;
	/*             */
	el[i + 2] = temp0;
	i = i + 3;
	/*                                   */
	mem_tag = ABE_DMEM;
	mem_addr = OMAP_ABE_D_ASRCVARS_UL_VX_ADDR + (3 * sizeof(s32));
	el[i] = (mem_tag << 16) + mem_addr;
	if (dppm == 0)
		el[i + 1] = 0;
	else
		el[i + 1] = dtemp << 2;
	/*             */
	el[i + 2] = temp0;
	i = i + 3;
	/*                                        */
	mem_tag = ABE_DMEM;
	mem_addr = OMAP_ABE_D_ASRCVARS_UL_VX_ADDR + (4 * sizeof(s32));
	el[i] = (mem_tag << 16) + mem_addr;
	if (dppm == 0)
		el[i + 1] = 0;
	else
		el[i + 1] = (-dtemp) << 2;
	/*             */
	el[i + 2] = temp0;
	i = i + 3;
	/*                                                 */
	mem_tag = ABE_DMEM;
	mem_addr = OMAP_ABE_D_ASRCVARS_UL_VX_ADDR + (5 * sizeof(s32));
	el[i] = (mem_tag << 16) + mem_addr;
	if (dppm == 0)
		el[i + 1] = 0x00400000;
	else
		el[i + 1] = (0x00100000 - (dtemp / 2)) << 2;
	/*             */
	el[i + 2] = temp0;
	i = i + 3;
	/*                                   */
	mem_tag = ABE_CMEM;
	mem_addr = OMAP_ABE_C_ALPHACURRENT_UL_VX_ADDR;
	el[i] = (mem_tag << 16) + mem_addr;
	el[i + 1] = 0x00000020;
	/*             */
	el[i + 2] = temp0;
	i = i + 3;
	/*                                  */
	mem_tag = ABE_CMEM;
	mem_addr = OMAP_ABE_C_BETACURRENT_UL_VX_ADDR;
	el[i] = (mem_tag << 16) + mem_addr;
	el[i + 1] = 0x003fffe0;
	/*             */
	el[i + 2] = temp0;
	i = i + 3;
	/*                                  */
	mem_tag = ABE_DMEM;
	mem_addr = OMAP_ABE_D_IODESCR_ADDR + (OMAP_ABE_VX_UL_PORT * sizeof(struct ABE_SIODescriptor))
		+ drift_asrc_;
	el[i] = (mem_tag << 16) + mem_addr;
	el[i + 1] = temp0;
	/*             */
	el[i + 2] = temp0;
	i = i + 3;
	/*                                      */
	/*                                                                    
                                                  */
	mem_tag = ABE_SMEM;
	mem_addr = ASRC_UL_VX_Coefs_labelID;
	el[i] = (mem_tag << 16) + (mem_addr << 2);
	if (dppm == 0) {
		el[i + 1] = OMAP_ABE_C_COEFASRC16_VX_ADDR >> 2;
		el[i + 1] = (el[i + 1] << 8) + (OMAP_ABE_C_COEFASRC16_VX_SIZE >> 2);
		el[i + 2] = OMAP_ABE_C_COEFASRC15_VX_ADDR >> 2;
		el[i + 2] = (el[i + 2] << 8) + (OMAP_ABE_C_COEFASRC15_VX_SIZE >> 2);
	} else {
		el[i + 1] = OMAP_ABE_C_COEFASRC1_VX_ADDR >> 2;
		el[i + 1] = (el[i + 1] << 8) + (OMAP_ABE_C_COEFASRC1_VX_SIZE >> 2);
		el[i + 2] = OMAP_ABE_C_COEFASRC2_VX_ADDR >> 2;
		el[i + 2] = (el[i + 2] << 8) + (OMAP_ABE_C_COEFASRC2_VX_SIZE >> 2);
	}
	i = i + 3;
	/*                                       */
	/*                                                                    
                                                  */
	mem_tag = ABE_CMEM;
	mem_addr = ASRC_UL_VX_Coefs_labelID;
	el[i] = (mem_tag << 16) + (mem_addr << 2);
	/*                                                */
	el[i + 1] = (temp0 << 16) + (temp1 << 12) + (temp0 << 4) + temp1;
	/*             */
	el[i + 2] = temp0;
	i = i + 3;
	/*                                    */
	/*                                                                 
          */
	mem_tag = ABE_SMEM;
	mem_addr = XinASRC_UL_VX_labelID;
	el[i] = (mem_tag << 16) + (mem_addr << 2);
	el[i + 1] = OMAP_ABE_S_XINASRC_UL_VX_ADDR >> 3;
	el[i + 1] = (el[i + 1] << 8) + (OMAP_ABE_S_XINASRC_UL_VX_SIZE >> 3);
	el[i + 2] = temp0;
	i = i + 3;
	/*                                    */
	/*                                                                 
          */
	mem_tag = ABE_CMEM;
	mem_addr = XinASRC_UL_VX_labelID;
	el[i] = (mem_tag << 16) + (mem_addr << 2);
	/*                                                */
	el[i + 1] = (temp0 << 16) + (temp1 << 12) + (temp0 << 4) + temp0;
	/*             */
	el[i + 2] = temp0;
	i = i + 3;
	/*                                  */
	/*                                                           
                                           */
	mem_tag = ABE_SMEM;
	mem_addr = UL_48_8_DEC_labelID;
	el[i] = (mem_tag << 16) + (mem_addr << 2);
	el[i + 1] = OMAP_ABE_S_XINASRC_UL_VX_ADDR >> 3;
	el[i + 1] = (el[i + 1] << 8) + (OMAP_ABE_S_XINASRC_UL_VX_SIZE >> 3);
	el[i + 2] = temp0;
	i = i + 3;
	/*                                  */
	/*                                                           
                                           */
	mem_tag = ABE_CMEM;
	mem_addr = UL_48_8_DEC_labelID;
	el[i] = (mem_tag << 16) + (mem_addr << 2);
	/*                                                */
	el[i + 1] = ((ASRC_UL_VX_FIR_L + ASRC_margin) << 16) + (temp1 << 12)
		+ (temp0 << 4) + temp0;
	/*             */
	el[i + 2] = temp0;
	i = i + 3;
	/*                                   */
	/*                                                            
                                           */
	mem_tag = ABE_SMEM;
	mem_addr = UL_48_16_DEC_labelID;
	el[i] = (mem_tag << 16) + (mem_addr << 2);
	el[i + 1] = OMAP_ABE_S_XINASRC_UL_VX_ADDR >> 3;
	el[i + 1] = (el[i + 1] << 8) + (OMAP_ABE_S_XINASRC_UL_VX_SIZE >> 3);
	el[i + 2] = temp0;
	i = i + 3;
	/*                                   */
	/*                                                            
                                           */
	mem_tag = ABE_CMEM;
	mem_addr = UL_48_16_DEC_labelID;
	el[i] = (mem_tag << 16) + (mem_addr << 2);
	/*                                                */
	el[i + 1] = ((ASRC_UL_VX_FIR_L + ASRC_margin) << 16) + (temp1 << 12)
		+ (temp0 << 4) + temp0;
	/*             */
	el[i + 2] = temp0;
	abe_write_fifo(ABE_DMEM, OMAP_ABE_D_FWMEMINITDESCR_ADDR, (u32 *) &el[0],
		       n_fifo_el);
}
/* 
                          
  
                                                       
                                         
                                  
                                  
                                                      
                                       
                                                       
                                                        
                                                                      
                                                                    
                                                                 
                                   
                                           
                                            
                                                                          
                                                  
                                         
                                         
                                                                               
          
                                         
                                         
                                                                           
                                                        
 */
void abe_init_asrc_mm_ext_in(s32 dppm)
{
	s32 el[45];
	s32 temp0, temp1, adppm, dtemp, mem_tag, mem_addr;
	u32 i = 0;
	u32 n_fifo_el = 42;
	temp0 = 0;
	temp1 = 1;
	/*                                  */
	mem_tag = ABE_DMEM;
	mem_addr = OMAP_ABE_D_ASRCVARS_MM_EXT_IN_ADDR + (1 * sizeof(s32));
	el[i] = (mem_tag << 16) + mem_addr;
	if (dppm >= 0) {
		el[i + 1] = 1;
		adppm = dppm;
	} else {
		el[i + 1] = -1;
		adppm = (-1 * dppm);
	}
	/*             */
	el[i + 2] = temp0;
	i = i + 3;
	dtemp = (adppm << 4) + adppm - ((adppm * 3481L) / 15625L);
	/*                                 */
	mem_tag = ABE_DMEM;
	mem_addr = OMAP_ABE_D_ASRCVARS_MM_EXT_IN_ADDR + (2 * sizeof(s32));
	el[i] = (mem_tag << 16) + mem_addr;
	el[i + 1] = temp0;
	/*             */
	el[i + 2] = temp0;
	i = i + 3;
	/*                                   */
	mem_tag = ABE_DMEM;
	mem_addr = OMAP_ABE_D_ASRCVARS_MM_EXT_IN_ADDR + (3 * sizeof(s32));
	el[i] = (mem_tag << 16) + mem_addr;
	if (dppm == 0)
		el[i + 1] = 0;
	else
		el[i + 1] = dtemp << 2;
	/*             */
	el[i + 2] = temp0;
	i = i + 3;
	/*                                        */
	mem_tag = ABE_DMEM;
	mem_addr = OMAP_ABE_D_ASRCVARS_MM_EXT_IN_ADDR + (4 * sizeof(s32));
	el[i] = (mem_tag << 16) + mem_addr;
	if (dppm == 0)
		el[i + 1] = 0;
	else
		el[i + 1] = (-dtemp) << 2;
	/*             */
	el[i + 2] = temp0;
	i = i + 3;
	/*                                                 */
	mem_tag = ABE_DMEM;
	mem_addr = OMAP_ABE_D_ASRCVARS_MM_EXT_IN_ADDR + (5 * sizeof(s32));
	el[i] = (mem_tag << 16) + mem_addr;
	if (dppm == 0)
		el[i + 1] = 0x00400000;
	else
		el[i + 1] = (0x00100000 - (dtemp / 2)) << 2;
	/*             */
	el[i + 2] = temp0;
	i = i + 3;
	/*                                   */
	mem_tag = ABE_CMEM;
	mem_addr = OMAP_ABE_C_ALPHACURRENT_MM_EXT_IN_ADDR;
	el[i] = (mem_tag << 16) + mem_addr;
	el[i + 1] = 0x00000020;
	/*             */
	el[i + 2] = temp0;
	i = i + 3;
	/*                                  */
	mem_tag = ABE_CMEM;
	mem_addr = OMAP_ABE_C_BETACURRENT_MM_EXT_IN_ADDR;
	el[i] = (mem_tag << 16) + mem_addr;
	el[i + 1] = 0x003fffe0;
	/*             */
	el[i + 2] = temp0;
	i = i + 3;
	/*                                  */
	mem_tag = ABE_DMEM;
	mem_addr = OMAP_ABE_D_IODESCR_ADDR + (OMAP_ABE_MM_EXT_IN_PORT * sizeof(struct ABE_SIODescriptor))
		+ drift_asrc_;
	el[i] = (mem_tag << 16) + mem_addr;
	el[i + 1] = temp0;
	/*             */
	el[i + 2] = temp0;
	i = i + 3;
	/*                                          */
	/*                                                                    
                                                      */
	mem_tag = ABE_SMEM;
	mem_addr = ASRC_MM_EXT_IN_Coefs_labelID;
	el[i] = (mem_tag << 16) + (mem_addr << 2);
	if (dppm == 0) {
		el[i + 1] = OMAP_ABE_C_COEFASRC16_MM_ADDR >> 2;
		el[i + 1] = (el[i + 1] << 8) + (OMAP_ABE_C_COEFASRC16_MM_SIZE >> 2);
		el[i + 2] = OMAP_ABE_C_COEFASRC15_MM_ADDR >> 2;
		el[i + 2] = (el[i + 2] << 8) + (OMAP_ABE_C_COEFASRC15_MM_SIZE >> 2);
	} else {
		el[i + 1] = OMAP_ABE_C_COEFASRC1_MM_ADDR >> 2;
		el[i + 1] = (el[i + 1] << 8) + (OMAP_ABE_C_COEFASRC1_MM_SIZE >> 2);
		el[i + 2] = OMAP_ABE_C_COEFASRC2_MM_ADDR >> 2;
		el[i + 2] = (el[i + 2] << 8) + (OMAP_ABE_C_COEFASRC2_MM_SIZE >> 2);
	}
	i = i + 3;
	/*                                          */
	/*                                                                    
                                                      */
	mem_tag = ABE_CMEM;
	mem_addr = ASRC_MM_EXT_IN_Coefs_labelID;
	el[i] = (mem_tag << 16) + (mem_addr << 2);
	/*                                                */
	el[i + 1] = (temp0 << 16) + (temp1 << 12) + (temp0 << 4) + temp1;
	/*             */
	el[i + 2] = temp0;
	i = i + 3;
	/*                                        */
	/*                                              
                                         */
	mem_tag = ABE_SMEM;
	mem_addr = XinASRC_MM_EXT_IN_labelID;
	el[i] = (mem_tag << 16) + (mem_addr << 2);
	el[i + 1] = OMAP_ABE_S_XINASRC_MM_EXT_IN_ADDR >> 3;
	el[i + 1] = (el[i + 1] << 8) + (OMAP_ABE_S_XINASRC_MM_EXT_IN_SIZE >> 3);
	el[i + 2] = temp0;
	i = i + 3;
	/*                                        */
	/*                                              
                                         */
	mem_tag = ABE_CMEM;
	mem_addr = XinASRC_MM_EXT_IN_labelID;
	el[i] = (mem_tag << 16) + (mem_addr << 2);
	/*                                                */
	el[i + 1] = (temp0 << 16) + (temp1 << 12) + (temp0 << 4) + temp0;
	/*             */
	el[i + 2] = temp0;
	i = i + 3;
	/*                                        */
	/*                    
                                                      
                                                        */
	mem_tag = ABE_SMEM;
	mem_addr = IO_MM_EXT_IN_ASRC_labelID;
	el[i] = (mem_tag << 16) + (mem_addr << 2);
	el[i + 1] = OMAP_ABE_S_XINASRC_MM_EXT_IN_ADDR >> 3;
	el[i + 1] = (el[i + 1] << 8) + (OMAP_ABE_S_XINASRC_MM_EXT_IN_SIZE >> 3);
	el[i + 2] = temp0;
	i = i + 3;
	/*                                        */
	/*                    
                                                      
                                                        */
	mem_tag = ABE_CMEM;
	mem_addr = IO_MM_EXT_IN_ASRC_labelID;
	el[i] = (mem_tag << 16) + (mem_addr << 2);
	/*                                                */
	el[i + 1] = ((ASRC_MM_EXT_IN_FIR_L + ASRC_margin + ASRC_N_48k) << 16) +
		(temp1 << 12) + (temp0 << 4) + temp0;
	/*             */
	el[i + 2] = temp0;
	abe_write_fifo(ABE_DMEM, OMAP_ABE_D_FWMEMINITDESCR_ADDR, (u32 *) &el[0],
		       n_fifo_el);
}
/* 
                      
  
                                                   
                                         
                                  
                                  
                                                      
                                       
                                                       
                                                        
                                                                      
                                                                    
                                                                 
                                   
                                       
                                        
                                                                      
                                                  
                                     
                                     
                                                                          
                                     
                                     
                                                               
                                         
 */
void abe_init_asrc_bt_ul(s32 dppm)
{
	s32 el[45];
	s32 temp0, temp1, adppm, dtemp, mem_tag, mem_addr;
	u32 i = 0;
	u32 n_fifo_el = 42;
	temp0 = 0;
	temp1 = 1;
	/*                                  */
	mem_tag = ABE_DMEM;
	mem_addr = OMAP_ABE_D_ASRCVARS_BT_UL_ADDR + (1 * sizeof(s32));
	el[i] = (mem_tag << 16) + mem_addr;
	if (dppm >= 0) {
		el[i + 1] = 1;
		adppm = dppm;
	} else {
		el[i + 1] = -1;
		adppm = (-1 * dppm);
	}
	/*             */
	el[i + 2] = temp0;
	i = i + 3;
	dtemp = (adppm << 4) + adppm - ((adppm * 3481L) / 15625L);
	/*                                 */
	mem_tag = ABE_DMEM;
	mem_addr = OMAP_ABE_D_ASRCVARS_BT_UL_ADDR + (2 * sizeof(s32));
	el[i] = (mem_tag << 16) + mem_addr;
	el[i + 1] = temp0;
	/*             */
	el[i + 2] = temp0;
	i = i + 3;
	/*                                   */
	mem_tag = ABE_DMEM;
	mem_addr = OMAP_ABE_D_ASRCVARS_BT_UL_ADDR + (3 * sizeof(s32));
	el[i] = (mem_tag << 16) + mem_addr;
	if (dppm == 0)
		el[i + 1] = 0;
	else
		el[i + 1] = dtemp << 2;
	/*             */
	el[i + 2] = temp0;
	i = i + 3;
	/*                                        */
	mem_tag = ABE_DMEM;
	mem_addr = OMAP_ABE_D_ASRCVARS_BT_UL_ADDR + (4 * sizeof(s32));
	el[i] = (mem_tag << 16) + mem_addr;
	if (dppm == 0)
		el[i + 1] = 0;
	else
		el[i + 1] = (-dtemp) << 2;
	/*             */
	el[i + 2] = temp0;
	i = i + 3;
	/*                                                */
	mem_tag = ABE_DMEM;
	mem_addr = OMAP_ABE_D_ASRCVARS_BT_UL_ADDR + (5 * sizeof(s32));
	el[i] = (mem_tag << 16) + mem_addr;
	if (dppm == 0)
		el[i + 1] = 0x00400000;
	else
		el[i + 1] = (0x00100000 - (dtemp / 2)) << 2;
	/*             */
	el[i + 2] = temp0;
	i = i + 3;
	/*                                   */
	mem_tag = ABE_CMEM;
	mem_addr = OMAP_ABE_C_ALPHACURRENT_BT_UL_ADDR;
	el[i] = (mem_tag << 16) + mem_addr;
	el[i + 1] = 0x00000020;
	/*             */
	el[i + 2] = temp0;
	i = i + 3;
	/*                                  */
	mem_tag = ABE_CMEM;
	mem_addr = OMAP_ABE_C_BETACURRENT_BT_UL_ADDR;
	el[i] = (mem_tag << 16) + mem_addr;
	el[i + 1] = 0x003fffe0;
	/*             */
	el[i + 2] = temp0;
	i = i + 3;
	/*                                  */
	mem_tag = ABE_DMEM;
	mem_addr = OMAP_ABE_D_IODESCR_ADDR + (OMAP_ABE_BT_VX_UL_PORT * sizeof(struct ABE_SIODescriptor))
		+ drift_asrc_;
	el[i] = (mem_tag << 16) + mem_addr;
	el[i + 1] = temp0;
	/*             */
	el[i + 2] = temp0;
	i = i + 3;
	/*                                      */
	/*                                                                    
                                                  */
	mem_tag = ABE_SMEM;
	mem_addr = ASRC_BT_UL_Coefs_labelID;
	el[i] = (mem_tag << 16) + (mem_addr << 2);
	if (dppm == 0) {
		el[i + 1] = OMAP_ABE_C_COEFASRC16_VX_ADDR >> 2;
		el[i + 1] = (el[i + 1] << 8) + (OMAP_ABE_C_COEFASRC16_VX_SIZE >> 2);
		el[i + 2] = OMAP_ABE_C_COEFASRC15_VX_ADDR >> 2;
		el[i + 2] = (el[i + 2] << 8) + (OMAP_ABE_C_COEFASRC15_VX_SIZE >> 2);
	} else {
		el[i + 1] = OMAP_ABE_C_COEFASRC1_VX_ADDR >> 2;
		el[i + 1] = (el[i + 1] << 8) + (OMAP_ABE_C_COEFASRC1_VX_SIZE >> 2);
		el[i + 2] = OMAP_ABE_C_COEFASRC2_VX_ADDR >> 2;
		el[i + 2] = (el[i + 2] << 8) + (OMAP_ABE_C_COEFASRC2_VX_SIZE >> 2);
	}
	i = i + 3;
	/*                                       */
	/*                                                                    
                                                  */
	mem_tag = ABE_CMEM;
	mem_addr = ASRC_BT_UL_Coefs_labelID;
	el[i] = (mem_tag << 16) + (mem_addr << 2);
	/*                                                */
	el[i + 1] = (temp0 << 16) + (temp1 << 12) + (temp0 << 4) + temp1;
	/*             */
	el[i + 2] = temp0;
	i = i + 3;
	/*                                    */
	/*                                                                 
          */
	mem_tag = ABE_SMEM;
	mem_addr = XinASRC_BT_UL_labelID;
	el[i] = (mem_tag << 16) + (mem_addr << 2);
	el[i + 1] = OMAP_ABE_S_XINASRC_BT_UL_ADDR >> 3;
	el[i + 1] = (el[i + 1] << 8) + (OMAP_ABE_S_XINASRC_BT_UL_SIZE >> 3);
	el[i + 2] = temp0;
	i = i + 3;
	/*                                    */
	/*                                                                 
          */
	mem_tag = ABE_CMEM;
	mem_addr = XinASRC_BT_UL_labelID;
	el[i] = (mem_tag << 16) + (mem_addr << 2);
	/*                                                */
	el[i + 1] = (temp0 << 16) + (temp1 << 12) + (temp0 << 4) + temp0;
	/*             */
	el[i + 2] = temp0;
	i = i + 3;
	/*                                    */
	/*                                                             
                                         */
	mem_tag = ABE_SMEM;
	mem_addr = IO_BT_UL_ASRC_labelID;
	el[i] = (mem_tag << 16) + (mem_addr << 2);
	el[i + 1] = OMAP_ABE_S_XINASRC_BT_UL_ADDR >> 3;
	el[i + 1] = (el[i + 1] << 8) + (OMAP_ABE_S_XINASRC_BT_UL_SIZE >> 3);
	el[i + 2] = temp0;
	i = i + 3;
	/*                                    */
	/*                                                             
                                         */
	mem_tag = ABE_CMEM;
	mem_addr = IO_BT_UL_ASRC_labelID;
	el[i] = (mem_tag << 16) + (mem_addr << 2);
	/*                                                */
	el[i + 1] = ((ASRC_BT_UL_FIR_L + ASRC_margin) << 16) + (temp1 << 12)
		+ (temp0 << 4) + temp0;
	/*             */
	el[i + 2] = temp0;
	abe_write_fifo(ABE_DMEM, OMAP_ABE_D_FWMEMINITDESCR_ADDR, (u32 *) &el[0],
		       n_fifo_el);
}
/* 
                      
  
                                                   
                                         
                                  
                                  
                                                      
                                       
                                                       
                                                        
                                                                      
                                                                    
                                                                 
                                   
                                       
                                        
                                                                      
                                                  
                                     
                                     
                                                                          
                                   
                                   
                                                             
                                         
                                    
                                    
                                                              
                                         
 */
void abe_init_asrc_bt_dl(s32 dppm)
{
	s32 el[51];
	s32 temp0, temp1, adppm, dtemp, mem_tag, mem_addr;
	u32 i = 0;
	u32 n_fifo_el = 48;
	temp0 = 0;
	temp1 = 1;
	/*                                  */
	mem_tag = ABE_DMEM;
	mem_addr = OMAP_ABE_D_ASRCVARS_BT_DL_ADDR + (1 * sizeof(s32));
	el[i] = (mem_tag << 16) + mem_addr;
	if (dppm >= 0) {
		el[i + 1] = 1;
		adppm = dppm;
	} else {
		el[i + 1] = -1;
		adppm = (-1 * dppm);
	}
	/*             */
	el[i + 2] = temp0;
	i = i + 3;
	dtemp = (adppm << 4) + adppm - ((adppm * 3481L) / 15625L);
	/*                                 */
	mem_tag = ABE_DMEM;
	mem_addr = OMAP_ABE_D_ASRCVARS_BT_DL_ADDR + (2 * sizeof(s32));
	el[i] = (mem_tag << 16) + mem_addr;
	el[i + 1] = temp0;
	/*             */
	el[i + 2] = temp0;
	i = i + 3;
	/*                                   */
	mem_tag = ABE_DMEM;
	mem_addr = OMAP_ABE_D_ASRCVARS_BT_DL_ADDR + (3 * sizeof(s32));
	el[i] = (mem_tag << 16) + mem_addr;
	if (dppm == 0)
		el[i + 1] = 0;
	else
		el[i + 1] = dtemp << 2;
	/*             */
	el[i + 2] = temp0;
	i = i + 3;
	/*                                        */
	mem_tag = ABE_DMEM;
	mem_addr = OMAP_ABE_D_ASRCVARS_BT_DL_ADDR + (4 * sizeof(s32));
	el[i] = (mem_tag << 16) + mem_addr;
	if (dppm == 0)
		el[i + 1] = 0;
	else
		el[i + 1] = (-dtemp) << 2;
	/*             */
	el[i + 2] = temp0;
	i = i + 3;
	/*                                                 */
	mem_tag = ABE_DMEM;
	mem_addr = OMAP_ABE_D_ASRCVARS_BT_DL_ADDR + (5 * sizeof(s32));
	el[i] = (mem_tag << 16) + mem_addr;
	if (dppm == 0)
		el[i + 1] = 0x00400000;
	else
		el[i + 1] = (0x00100000 - (dtemp / 2)) << 2;
	/*             */
	el[i + 2] = temp0;
	i = i + 3;
	/*                                   */
	mem_tag = ABE_CMEM;
	mem_addr = OMAP_ABE_C_ALPHACURRENT_BT_DL_ADDR;
	el[i] = (mem_tag << 16) + mem_addr;
	el[i + 1] = 0x00000020;
	/*             */
	el[i + 2] = temp0;
	i = i + 3;
	/*                                  */
	mem_tag = ABE_CMEM;
	mem_addr = OMAP_ABE_C_BETACURRENT_BT_DL_ADDR;
	el[i] = (mem_tag << 16) + mem_addr;
	el[i + 1] = 0x003fffe0;
	/*             */
	el[i + 2] = temp0;
	i = i + 3;
	/*                                  */
	mem_tag = ABE_DMEM;
	mem_addr = OMAP_ABE_D_IODESCR_ADDR + (OMAP_ABE_BT_VX_DL_PORT * sizeof(struct ABE_SIODescriptor))
		+ drift_asrc_;
	el[i] = (mem_tag << 16) + mem_addr;
	el[i + 1] = temp0;
	/*             */
	el[i + 2] = temp0;
	i = i + 3;
	/*                                      */
	/*                                                                    
                                                  */
	mem_tag = ABE_SMEM;
	mem_addr = ASRC_BT_DL_Coefs_labelID;
	el[i] = (mem_tag << 16) + (mem_addr << 2);
	if (dppm == 0) {
		el[i + 1] = OMAP_ABE_C_COEFASRC16_VX_ADDR >> 2;
		el[i + 1] = (el[i + 1] << 8) + (OMAP_ABE_C_COEFASRC16_VX_SIZE >> 2);
		el[i + 2] = OMAP_ABE_C_COEFASRC15_VX_ADDR >> 2;
		el[i + 2] = (el[i + 2] << 8) + (OMAP_ABE_C_COEFASRC15_VX_SIZE >> 2);
	} else {
		el[i + 1] = OMAP_ABE_C_COEFASRC1_VX_ADDR >> 2;
		el[i + 1] = (el[i + 1] << 8) + (OMAP_ABE_C_COEFASRC1_VX_SIZE >> 2);
		el[i + 2] = OMAP_ABE_C_COEFASRC2_VX_ADDR >> 2;
		el[i + 2] = (el[i + 2] << 8) + (OMAP_ABE_C_COEFASRC2_VX_SIZE >> 2);
	}
	i = i + 3;
	/*                                       */
	/*                                                                    
                                                  */
	mem_tag = ABE_CMEM;
	mem_addr = ASRC_BT_DL_Coefs_labelID;
	el[i] = (mem_tag << 16) + (mem_addr << 2);
	/*                                                */
	el[i + 1] = (temp0 << 16) + (temp1 << 12) + (temp0 << 4) + temp1;
	/*             */
	el[i + 2] = temp0;
	i = i + 3;
	/*                                    */
	/*                
                                                          */
	mem_tag = ABE_SMEM;
	mem_addr = XinASRC_BT_DL_labelID;
	el[i] = (mem_tag << 16) + (mem_addr << 2);
	el[i + 1] = OMAP_ABE_S_XINASRC_BT_DL_ADDR >> 3;
	el[i + 1] = (el[i + 1] << 8) + (OMAP_ABE_S_XINASRC_BT_DL_SIZE >> 3);
	el[i + 2] = temp0;
	i = i + 3;
	/*                                    */
	/*                
                                                          */
	mem_tag = ABE_CMEM;
	mem_addr = XinASRC_BT_DL_labelID;
	el[i] = (mem_tag << 16) + (mem_addr << 2);
	/*                                                */
	el[i + 1] = (temp0 << 16) + (temp1 << 12) + (temp0 << 4) + temp0;
	/*             */
	el[i + 2] = temp0;
	i = i + 3;
	/*                                  */
	/*                                                           
                                         */
	mem_tag = ABE_SMEM;
	mem_addr = DL_48_8_DEC_labelID;
	el[i] = (mem_tag << 16) + (mem_addr << 2);
	el[i + 1] = OMAP_ABE_S_XINASRC_BT_DL_ADDR >> 3;
	el[i + 1] = (el[i + 1] << 8) + (OMAP_ABE_S_XINASRC_BT_DL_SIZE >> 3);
	el[i + 2] = temp0;
	i = i + 3;
	/*                                  */
	/*                                                           
                                         */
	mem_tag = ABE_CMEM;
	mem_addr = DL_48_8_DEC_labelID;
	el[i] = (mem_tag << 16) + (mem_addr << 2);
	/*                                                */
	el[i + 1] = ((ASRC_BT_DL_FIR_L + ASRC_margin) << 16) + (temp1 << 12)
		+ (temp0 << 4) + temp0;
	/*             */
	el[i + 2] = temp0;
	i = i + 3;
	/*                                   */
	/*                                                            
                                         */
	mem_tag = ABE_SMEM;
	mem_addr = DL_48_16_DEC_labelID;
	el[i] = (mem_tag << 16) + (mem_addr << 2);
	el[i + 1] = OMAP_ABE_S_XINASRC_BT_DL_ADDR >> 3;
	el[i + 1] = (el[i + 1] << 8) + (OMAP_ABE_S_XINASRC_BT_DL_SIZE >> 3);
	el[i + 2] = temp0;
	i = i + 3;
	/*                                   */
	/*                                                            
                                         */
	mem_tag = ABE_CMEM;
	mem_addr = DL_48_16_DEC_labelID;
	el[i] = (mem_tag << 16) + (mem_addr << 2);
	/*                                                */
	el[i + 1] = ((ASRC_BT_DL_FIR_L + ASRC_margin) << 16) + (temp1 << 12)
		+ (temp0 << 4) + temp0;
	/*             */
	el[i + 2] = temp0;
	abe_write_fifo(ABE_DMEM, OMAP_ABE_D_FWMEMINITDESCR_ADDR, (u32 *) &el[0],
		       n_fifo_el);
}
