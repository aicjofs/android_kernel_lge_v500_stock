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

#ifndef _ABE_TYPEDEF_H_
#define _ABE_TYPEDEF_H_

#include "abe_define.h"
#include "abe_typ.h"

/*
                         
 */
/*
                           
 */
typedef struct abetaskTag {
	/*                                */
	u16 iF;
	/*                         */
	u16 A0;
	/*                         */
	u16 A1;
	/*                              */
	u16 A2_3;
	/*                              */
	u16 A4_5;
	/*                                      */
	u16 R;
	/*    */
	u16 misc0;
	/*    */
	u16 misc1;
} ABE_STask;
typedef ABE_STask *pABE_STask;
typedef ABE_STask **ppABE_STask;

struct ABE_SIODescriptor {
	/*   */
	u16 drift_ASRC;
	/*   */
	u16 drift_io;
	/*                                             */
	u8 io_type_idx;
	/*                                          */
	u8 samp_size;
	/*                           */
	s16 flow_counter;
	/*                                  */
	u16 hw_ctrl_addr;
	/*                                           */
	u8 atc_irq_data;
	/*                        */
	u8 direction_rw;
	/*    */
	u8 repeat_last_samp;
	/*                     */
	u8 nsamp;
	/*                      */
	u8 x_io;
	/*                          */
	u8 on_off;
	/*                                */
	u16 split_addr1;
	/*    */
	u16 split_addr2;
	/*    */
	u16 split_addr3;
	/*    */
	u8 before_f_index;
	/*    */
	u8 after_f_index;
	/*                        */
	u16 smem_addr1;
	/*             */
	u16 atc_address1;
	/*                                        */
	u16 atc_pointer_saved1;
	/*                                         */
	u8 data_size1;
	/*                                              */
	u8 copy_f_index1;
	/*                                */
	u16 smem_addr2;
	/*    */
	u16 atc_address2;
	/*    */
	u16 atc_pointer_saved2;
	/*    */
	u8 data_size2;
	/*    */
	u8 copy_f_index2;
};

/*                                                         */
#define drift_asrc_ 0
/*                                                                 
                          */
#define drift_io_ 2
/*                              */
#define io_type_idx_ 4
#define samp_size_ 5
/*                    */
#define flow_counter_ 6
/*                                                         */
#define hw_ctrl_addr_ 8
/*                                             */
#define atc_irq_data_ 10
/*                                                                */
#define direction_rw_ 11
/*                                                               */
#define repeat_last_samp_ 12
/*                                           */
#define nsamp_ 13
/*                                 */
#define x_io_ 14
#define on_off_ 15
/*                                            */
#define split_addr1_ 16
/*                                            */
#define split_addr2_ 18
/*                                            */
#define split_addr3_ 20
/*                              */
#define before_f_index_ 22
/*                              */
#define after_f_index_ 23
#define minidesc1_ 24
/*                                            */
#define rel_smem_ 0
/*                                          */
#define rel_atc_ 2
/*                                                 */
#define rel_atc_saved 4
/*                                                 */
#define rel_size_ 6
/*                              */
#define rel_f_ 7
#define s_mem_mm_ul 24
#define s_mm_ul_size 30
#define minidesc2_ 32
#define Struct_Size 40

struct ABE_SPingPongDescriptor {
	/*                                                            */
	u16 drift_ASRC;
	/*                                                      
                                        */
	u16 drift_io;
	/*                                                            */
	u16 hw_ctrl_addr;
	/*                                 */
	u8 copy_func_index;
	/*                                     */
	u8 x_io;
	/*                                       */
	u8 data_size;
	/*                                               */
	u8 smem_addr;
	/*                                                 */
	u8 atc_irq_data;
	/*                           */
	u8 counter;
	/*                                                */
	u16 workbuff_BaseAddr;
	/*                                        */
	u16 workbuff_Samples;
	/*                                            */
	u16 nextbuff0_BaseAddr;
	/*                                                 */
	u16 nextbuff0_Samples;
	/*                                            */
	u16 nextbuff1_BaseAddr;
	/*                                                 */
	u16 nextbuff1_Samples;
};

#endif/*                 */
