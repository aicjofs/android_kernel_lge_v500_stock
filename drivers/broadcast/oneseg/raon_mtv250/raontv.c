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
* TITLE 	  : RAONTECH TV device driver API header file. 
*
* FILENAME    : raontv.c
*
* DESCRIPTION : 
*		Configuration for RAONTECH TV Services.
*
********************************************************************************/

/*                                                                                
                  
 
                              
                                                                                 
                                                    
                                                       
                                                                                
                             
                                                                               */

#include "raontv_rf.h"

volatile BOOL g_afRtvChannelChange[NUM_ATTECHED_RTV_CHIP];


volatile E_RTV_ADC_CLK_FREQ_TYPE g_aeRtvAdcClkFreqType[NUM_ATTECHED_RTV_CHIP];
BOOL g_afRtvStreamEnabled[NUM_ATTECHED_RTV_CHIP];

#if defined(RTV_TDMB_ENABLE) || defined(RTV_ISDBT_ENABLE)
	E_RTV_COUNTRY_BAND_TYPE g_eRtvCountryBandType;
#endif

#ifdef RTV_IF_EBI2
	VU8 g_bRtvEbiMapSelData = 0x7;
#endif

#if defined(RTV_IF_SPI) || defined(RTV_IF_EBI2)
	UINT g_aRtvMscThresholdSize[NUM_ATTECHED_RTV_CHIP]; //                  
	U8 g_abRtvIntrMaskRegL[NUM_ATTECHED_RTV_CHIP];

#else	
   #if defined(RTV_TDMB_ENABLE) && !defined(RTV_TDMB_MULTI_SUB_CHANNEL_ENABLE) /*                         */
	U8 g_abRtvIntrMaskRegL[NUM_ATTECHED_RTV_CHIP];
   #endif
#endif	


#ifdef RTV_DUAL_CHIP_USED /*                   */
	BOOL g_fRtvDiversityEnabled;
	UINT RaonTvChipIdx = 0xFF; /*                                                       */
#endif	


void rtv_ConfigureHostIF(void)
{
#if defined(RTV_IF_MPEG2_SERIAL_TSIF) || defined(RTV_IF_QUALCOMM_TSIF) || defined(RTV_IF_MPEG2_PARALLEL_TSIF) || defined(RTV_IF_SPI_SLAVE)
	RTV_REG_MAP_SEL(HOST_PAGE);
    RTV_REG_SET(0x77, 0x15);   //            
    RTV_REG_SET(0x22, 0x48);   

  #if defined(RTV_IF_MPEG2_PARALLEL_TSIF)
	RTV_REG_SET(0x04, 0x01);   //                       
  #else
	RTV_REG_SET(0x04, 0x29);   //                       
  #endif
  
	RTV_REG_SET(0x0C, 0xF4);   //            

#elif defined(RTV_IF_SPI) || defined(RTV_IF_EBI2)
	RTV_REG_MAP_SEL(HOST_PAGE);
	RTV_REG_SET(0x77, 0x14);   //               
    RTV_REG_SET(0x04, 0x28);   //                
	RTV_REG_SET(0x0C, 0xF5);
 
#else
	#error "Code not present"
#endif
}

INT rtv_InitSystem(E_RTV_TV_MODE_TYPE eTvMode, E_RTV_ADC_CLK_FREQ_TYPE eAdcClkFreqType)
{
	INT nRet;
	int i;
	
	g_afRtvChannelChange[RaonTvChipIdx] = FALSE;
	
	g_afRtvStreamEnabled[RaonTvChipIdx] = FALSE;

#if defined(RTV_IF_SPI) || defined(RTV_IF_EBI2)
	g_abRtvIntrMaskRegL[RaonTvChipIdx] = 0xFF;
#else	
   #if defined(RTV_TDMB_ENABLE) && !defined(RTV_TDMB_MULTI_SUB_CHANNEL_ENABLE) /*                         */
	g_abRtvIntrMaskRegL[RaonTvChipIdx] = 0xFF;
   #endif
#endif	

	/*                                                                                             */
	RTV_GUARD_INIT;

	for(i=0; i<100; i++)
	{
//                                    

		RTV_REG_MAP_SEL(HOST_PAGE);

//                                    
		

		RTV_REG_SET(0x7D, 0x06);
		if(RTV_REG_GET(0x7D) == 0x06)
		{
			goto RTV_POWER_ON_SUCCESS;
		}

		RTV_DBGMSG1("[rtv_InitSystem] Power On wait: %d\n", i);

		RTV_DELAY_MS(5);
	}

	RTV_DBGMSG1("rtv_InitSystem: Power On Check error: %d\n", i);
	return RTV_POWER_ON_CHECK_ERROR;

RTV_POWER_ON_SUCCESS:
	
	rtvRF_ConfigurePowerType(eTvMode);

	if((nRet=rtvRF_ConfigureAdcClock(eTvMode, eAdcClkFreqType)) != RTV_SUCCESS)
		return nRet;
		
	return RTV_SUCCESS;
}





