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
#include "abe_mem.h"
#include "abe_aess.h"

/* 
                            
  
 */
void omap_abe_hw_configuration(struct omap_abe *abe)
{
	/*                                                       */
	omap_abe_reg_writel(abe, AESS_DMAENABLE_SET, DMA_ENABLE_ALL);
	/*                                            */
	omap_abe_reg_writel(abe, AESS_MCU_IRQENABLE_SET, INT_SET);
}

/* 
                                           
                              
  
                                           
 */
int omap_abe_clear_irq(struct omap_abe *abe)
{
	omap_abe_reg_writel(abe, ABE_MCU_IRQSTATUS, INT_CLR);
	return 0;
}
EXPORT_SYMBOL(omap_abe_clear_irq);

/* 
                                                             
                              
                                                        
  
                                                  
                                             
                                                                          
                                            
                                                                     
                                                     
  
                                                                      
                                                         
                                                                               
               
                                                                      
                                          
                                                             
                                        
 */
int omap_abe_write_event_generator(struct omap_abe *abe, u32 e)
{
	u32 event, selection;
	u32 counter = EVENT_GENERATOR_COUNTER_DEFAULT;

	_log(ABE_ID_WRITE_EVENT_GENERATOR, e, 0, 0);

	switch (e) {
	case EVENT_TIMER:
		selection = EVENT_SOURCE_COUNTER;
		event = 0;
		break;
	case EVENT_44100:
		selection = EVENT_SOURCE_COUNTER;
		event = 0;
		counter = EVENT_GENERATOR_COUNTER_44100;
		break;
	default:
		omap_abe_dbg_error(abe, OMAP_ABE_ERR_API, ABE_BLOCK_COPY_ERR);
	}
	omap_abe_reg_writel(abe, EVENT_GENERATOR_COUNTER, counter);
	omap_abe_reg_writel(abe, EVENT_SOURCE_SELECTION, selection);
	omap_abe_reg_writel(abe, EVENT_GENERATOR_START, EVENT_GENERATOR_ON);
	omap_abe_reg_writel(abe, AUDIO_ENGINE_SCHEDULER, event);
	return 0;
}
EXPORT_SYMBOL(omap_abe_write_event_generator);

/* 
                                                                 
                              
  
                                                                               
                                                               
                        
 */
int omap_abe_start_event_generator(struct omap_abe *abe)
{
	/*                           */
	omap_abe_reg_writel(abe, EVENT_GENERATOR_START, 1);
	return 0;
}
EXPORT_SYMBOL(omap_abe_start_event_generator);

/* 
                                                               
                              
  
                                                                              
                                                               
                        
 */
int omap_abe_stop_event_generator(struct omap_abe *abe)
{
	/*                          */
	omap_abe_reg_writel(abe, EVENT_GENERATOR_START, 0);
	return 0;
}
EXPORT_SYMBOL(omap_abe_stop_event_generator);

/* 
                                                       
                              
  
                                               
 */
int omap_abe_disable_irq(struct omap_abe *abe)
{
	/*                                                       
                                                               */
	omap_abe_reg_writel(abe, AESS_DMAENABLE_CLR, 0x7F);
	/*                                             */
	omap_abe_reg_writel(abe, AESS_MCU_IRQENABLE_CLR, 0x01);
	return 0;
}
EXPORT_SYMBOL(omap_abe_disable_irq);
