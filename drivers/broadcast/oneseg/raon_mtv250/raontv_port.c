/******************************************************************************** 
* (c) COPYRIGHT 2010 RAONTECH, Inc. ALL RIGHTS RESERVED.
* 
* This software is the property of RAONTECH and is furnished under license by RAONTECH.                
* This software may be used only in accordance with the terms of said license.                         
* This copyright noitce may not be remoced, modified or obliterated without the prior                  
* written permission of RAONTECH, Inc.                                                                 
*                                                                                                      
* This software may not be copied, transmitted, provided to or otherwise made available                
* to any other person, company, corporation or other entity except as specified in the                 
* terms of said license.                                                                               
*                                                                                                      
* No right, title, ownership or other interest in the software is hereby granted or transferred.       
*                                                                                                      
* The information contained herein is subject to change without notice and should 
* not be construed as a commitment by RAONTECH, Inc.                                                                    
* 
* TITLE 	  : RAONTECH TV OEM source file. 
*
* FILENAME    : raontv_port.c
*
* DESCRIPTION : 
*		User-supplied Routines for RAONTECH TV Services.
*
********************************************************************************/

/*                                                                                
                  
 
                              
                                                                                 
                                                    
                                                       
                                                                                
                             
                                            
                                                                                          
                                                                               */

#include "raontv.h"
#include "raontv_internal.h"


/*                                                   */
#if defined(RTV_IF_SPI) || (defined(RTV_TDMB_ENABLE) && !defined(RTV_TDMB_FIC_POLLING_MODE))	
	#if defined(__KERNEL__)	
		struct mutex raontv_guard;
	#else
		//     

	#endif
	
#endif


void rtvOEM_ConfigureInterrupt(void) 
{
#if defined(RTV_IF_SPI) || defined(RTV_IF_EBI2) \
	|| (defined(RTV_TDMB_ENABLE) && !defined(RTV_TDMB_MULTI_SUB_CHANNEL_ENABLE))

	RTV_REG_SET(0x09, 0x00); //                                                
	RTV_REG_SET(0x0B, 0x00); //                                        

	RTV_REG_MAP_SEL(HOST_PAGE);
	RTV_REG_SET(0x28, 0x01); //                                                                        
	
	RTV_REG_SET(0x29, 0x08); //                                                                                                      
	
	RTV_REG_SET(0x2A, 0x13); //                                                                                       
#endif
}


#include "mtv250.h"
#include "mtv250_ioctl.h"
#include "mtv250_gpio.h"

void rtvOEM_PowerOn(int on)
{
#ifndef RTV_DUAL_CHIP_USED
	if( on )
	{
//                                 

#if defined(CONFIG_KS1001) || defined(CONFIG_KS1103)
		gpio_set_value(MTV_1_2V_EN, 1);
#endif
//                                        
#if defined(CONFIG_MACH_MSM8960_L_DCM)
		gpio_set_value(MTV_LDO_EN, 1);
#endif
//                                      

//                                 
		
		/*                                    */
		gpio_set_value(MTV_PWR_EN, 0);
		RTV_DELAY_MS(10);
		
		/*                                     */
		gpio_set_value(MTV_PWR_EN, 1);
		RTV_DELAY_MS(20);	
	}
	else
	{
		/*                                    */		
		gpio_set_value(MTV_PWR_EN, 0);

#if defined(CONFIG_KS1001) || defined(CONFIG_KS1103)
		gpio_set_value(MTV_1_2V_EN, 0);
#endif
//                                        
#if defined(CONFIG_MACH_MSM8960_L_DCM)
		gpio_set_value(MTV_LDO_EN, 0);
#endif
//                                      
	}

#else
	if( on )
	{
		/*                                    */
		if(RaonTvChipIdx == 0)
		{ /*                 */
		}
		else
		{
		}		
		RTV_DELAY_MS(10);
		
		/*                                     */
		if(RaonTvChipIdx == 0)
		{ /*                 */
		}
		else
		{
		}		
		RTV_DELAY_MS(20);	
	}
	else
	{
		/*                                    */
		if(RaonTvChipIdx == 0)
		{ /*                 */
		}
		else
		{
		}
	}
#endif	
}



