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
* TITLE 	  : RAONTECH TV internal header file. 
*
* FILENAME    : raontv_internal.h
*
* DESCRIPTION : 
*		All the declarations and definitions necessary for the RAONTECH TV driver.
*
********************************************************************************/

/*                                                                                
                  
 
                              
                                                                                 
                                                                                   
                                                                      
                                                    
                                                       
                                                                                
                             
                                            
                                                                                                     
                                                                               */

#ifndef __RAONTV_INTERNAL_H__
#define __RAONTV_INTERNAL_H__

#ifdef __cplusplus 
extern "C"{ 
#endif  

#include "raontv.h"


//                         
typedef enum
{	
	RTV_TV_MODE_TDMB   = 0,     //          
	RTV_TV_MODE_DAB_B3 = 1,      //       
	RTV_TV_MODE_DAB_L  = 2,      //        
	RTV_TV_MODE_1SEG   = 3, //    
	RTV_TV_MODE_FM     = 4,       //   
	MAX_NUM_RTV_MODE
} E_RTV_TV_MODE_TYPE;


typedef struct
{
	U8	bReg;
	U8	bVal;
} RTV_REG_INIT_INFO;


typedef struct
{
	U8	bReg;
	U8  bMask;
	U8	bVal;
} RTV_REG_MASK_INFO;


#if defined(RTV_IF_MPEG2_SERIAL_TSIF) || defined(RTV_IF_SPI_SLAVE) || defined(RTV_IF_QUALCOMM_TSIF) || defined(RTV_IF_MPEG2_PARALLEL_TSIF)
	#if defined(RTV_TSIF_CLK_SPEED_DIV_2) //           
		#define RTV_COMM_CON47_CLK_SEL	(0<<6)
	#elif defined(RTV_TSIF_CLK_SPEED_DIV_4) //           
		#define RTV_COMM_CON47_CLK_SEL	(1<<6)
	#elif defined(RTV_TSIF_CLK_SPEED_DIV_6) //           
		#define RTV_COMM_CON47_CLK_SEL	(2<<6)
	#elif defined(RTV_TSIF_CLK_SPEED_DIV_8) //           
		#define RTV_COMM_CON47_CLK_SEL	(3<<6)
	#else
		#error "Code not present"
	#endif
#endif


#define MSC1_E_OVER_FLOW       0x40
#define MSC1_E_UNDER_FLOW      0x20
#define MSC1_E_INT             0x10
#define MSC0_E_OVER_FLOW       0x08
#define MSC0_E_UNDER_FLOW      0x04
#define MSC0_E_INT             0x02
#define FIC_E_INT              0x01
#define RE_CONFIG_E_INT        0x04

#define MSC1_INTR_BITS	(MSC1_E_INT|MSC1_E_UNDER_FLOW|MSC1_E_OVER_FLOW)
#define MSC0_INTR_BITS	(MSC0_E_INT|MSC0_E_UNDER_FLOW|MSC0_E_OVER_FLOW)

#define INT_E_UCLRL         (0x35)  //                                                      
#define INT_E_UCLRH         (0x36)  //                        

#define INT_E_STATL         (0x33)  //                                                                                                                          
#define INT_E_STATH         (0x34)  //                                                                                                                                      

#define DD_E_TOPCON         (0x45)  //                                                                                                                                        
#define FIC_E_DDCON         (0x46)  //                                                                                                                          
#define MSC0_E_CON          (0x47)  //                                                                                                                               
#define MSC1_E_CON          (0x48)  //                                                                                                                                                                                      
#define OFDM_E_DDCON        (0x49)  //                                                                                                                                                           

#define MSC0_E_RSIZE_H      (0x4E)  //                                                        
#define MSC0_E_RSIZE_L      (0x4F)  //       
#define MSC0_E_INTTH_H      (0x50)  //                                 
#define MSC0_E_INTTH_L      (0x51)  //       
#define MSC0_E_TSIZE_H      (0x52)  //        
#define MSC0_E_TSIZE_L      (0x53)  //                                           
#define MSC1_E_RSIZE_H      (0x54)  //                                                        
#define MSC1_E_RSIZE_L      (0x55)  //       
#define MSC1_E_INTTH_H      (0x56)  //                                 
#define MSC1_E_INTTH_L      (0x57)  //       
#define MSC1_E_TSIZE_H      (0x58)  //        
#define MSC1_E_TSIZE_L      (0x59)  //                         



#define MODE1 2 		
#define MODE2 1
#define MODE3 0


#define MAP_SEL_REG 	0x03

#define OFDM_PAGE       0x02 //         
#define FEC_PAGE        0x03 //         
#define COMM_PAGE       0x04
#define FM_PAGE         0x06 //              
#define HOST_PAGE       0x07
#define CAS_PAGE        0x08
#define DD_PAGE         0x09 //                      

#define FIC_PAGE        0x0A
#define MSC0_PAGE       0x0B
#define MSC1_PAGE       0x0C
#define RF_PAGE         0x0F


#define DEMOD_0SC_DIV2_ON  0x80
#define DEMOD_0SC_DIV2_OFF 0x00

#if (RTV_SRC_CLK_FREQ_KHz >= 32000)
	#define DEMOD_OSC_DIV2 	DEMOD_0SC_DIV2_ON
#else 
	#define DEMOD_OSC_DIV2 	DEMOD_0SC_DIV2_OFF
#endif 


#define MAP_SEL_VAL(page)		(DEMOD_OSC_DIV2|page)
#define RTV_REG_MAP_SEL(page)	do{RTV_REG_SET(MAP_SEL_REG, MAP_SEL_VAL(page));}while(0)


#define RTV_TS_STREAM_DISABLE_DELAY		20 //   


//                
#define ISDBT_CH_NUM_START__JAPAN			13
#define ISDBT_CH_NUM_END__JAPAN				62
#define ISDBT_CH_FREQ_START__JAPAN			473143
#define ISDBT_CH_FREQ_STEP__JAPAN			6000

#define ISDBT_CH_NUM_START__BRAZIL			14
#define ISDBT_CH_NUM_END__BRAZIL			69
#define ISDBT_CH_FREQ_START__BRAZIL			473143
#define ISDBT_CH_FREQ_STEP__BRAZIL			6000

#define ISDBT_CH_NUM_START__ARGENTINA		14
#define ISDBT_CH_NUM_END__ARGENTINA			69
#define ISDBT_CH_FREQ_START__ARGENTINA		473143
#define ISDBT_CH_FREQ_STEP__ARGENTINA		6000

//               
#define TDMB_CH_FREQ_START__KOREA		175280
#define TDMB_CH_FREQ_STEP__KOREA		1728 //         


#ifdef RTV_DUAL_CHIP_USED 
	#define NUM_ATTECHED_RTV_CHIP		2
	extern BOOL g_fRtvDiversityEnabled;

#else
	#define NUM_ATTECHED_RTV_CHIP		1
#endif	


extern volatile E_RTV_ADC_CLK_FREQ_TYPE g_aeRtvAdcClkFreqType[NUM_ATTECHED_RTV_CHIP];
extern E_RTV_COUNTRY_BAND_TYPE g_eRtvCountryBandType;

/*                                                                         */
extern volatile BOOL g_afRtvChannelChange[NUM_ATTECHED_RTV_CHIP];
extern BOOL g_afRtvStreamEnabled[NUM_ATTECHED_RTV_CHIP];

#if defined(RTV_IF_SPI) || defined(RTV_IF_EBI2)
	extern UINT g_aRtvMscThresholdSize[NUM_ATTECHED_RTV_CHIP];	
	extern U8 g_abRtvIntrMaskRegL[NUM_ATTECHED_RTV_CHIP];
	
#else
   #if defined(RTV_TDMB_ENABLE) && !defined(RTV_TDMB_MULTI_SUB_CHANNEL_ENABLE) /*                         */
	/*                                           */
	extern U8 g_abRtvIntrMaskRegL[NUM_ATTECHED_RTV_CHIP];
   #endif
#endif	


/*                                                                              
  
                           
  
                                                                              */ 
 
/*                    */
static INLINE void rtv_ResetMemory_MSC1(void);
static INLINE void rtv_SetupMemory_MSC1(U16 wThresholdSize);

//             
static INLINE void rtv_StreamDisable(E_RTV_TV_MODE_TYPE eTvMode)
{	
#ifndef RTV_IF_MPEG2_PARALLEL_TSIF
	if(g_afRtvStreamEnabled[RaonTvChipIdx] == FALSE)
		return;
	
	switch( eTvMode )
	{
  #ifdef RTV_ISDBT_ENABLE
		case RTV_TV_MODE_1SEG:
	  #if defined(RTV_IF_SPI) || defined(RTV_IF_EBI2)   
			RTV_REG_MAP_SEL(HOST_PAGE);
			RTV_REG_SET(0x62, 0xFF); //                
			rtv_ResetMemory_MSC1();
	  #endif
			break;
  #endif /*                  */

  #ifdef RTV_FM_ENABLE
		case RTV_TV_MODE_FM:	
	  #if defined(RTV_IF_SPI) || defined(RTV_IF_EBI2)	
			RTV_REG_MAP_SEL(HOST_PAGE);
		    	RTV_REG_SET(0x62, g_abRtvIntrMaskRegL[RaonTvChipIdx]|MSC0_E_INT|MSC1_E_INT); //                
		    
	  #elif defined(RTV_IF_MPEG2_SERIAL_TSIF) || defined(RTV_IF_SPI_SLAVE) || defined(RTV_IF_QUALCOMM_TSIF)
			RTV_REG_MAP_SEL(HOST_PAGE);
			RTV_REG_SET(0x29, 0x08);
			RTV_DELAY_MS(RTV_TS_STREAM_DISABLE_DELAY);
	  #endif				
		    break;
  #endif	/*               */	  
		    
  #ifdef RTV_TDMB_ENABLE		    
		case RTV_TV_MODE_TDMB:
	  #if defined(RTV_IF_SPI) || defined(RTV_IF_EBI2)
			RTV_REG_MAP_SEL(HOST_PAGE);
		    	RTV_REG_SET(0x62, g_abRtvIntrMaskRegL[RaonTvChipIdx]|MSC0_E_INT|MSC1_E_INT|FIC_E_INT); //                

	  #elif defined(RTV_IF_MPEG2_SERIAL_TSIF) || defined(RTV_IF_SPI_SLAVE) || defined(RTV_IF_QUALCOMM_TSIF) || defined(RTV_IF_MPEG2_PARALLEL_TSIF)	
	     #ifndef RTV_TDMB_MULTI_SUB_CHANNEL_ENABLE /*                         */
		 #ifndef RTV_TDMB_FIC_POLLING_MODE
		    	RTV_REG_MAP_SEL(HOST_PAGE);
		    	RTV_REG_SET(0x62, g_abRtvIntrMaskRegL[RaonTvChipIdx]|FIC_E_INT); //    
		 #endif
	     #else /*                         */
		 
	     #endif
	  #endif	  	
			break;
  #endif	/*                 */		
			
		default: break; 
    }
    
    g_afRtvStreamEnabled[RaonTvChipIdx] = FALSE; 
#endif	
}

//              
static INLINE void rtv_StreamRestore(E_RTV_TV_MODE_TYPE eTvMode)
{
#ifndef RTV_IF_MPEG2_PARALLEL_TSIF
	if(g_afRtvStreamEnabled[RaonTvChipIdx] == TRUE)
		return;

	switch( eTvMode )
	{
  #ifdef RTV_ISDBT_ENABLE 
		case RTV_TV_MODE_1SEG:
	#if defined(RTV_IF_SPI) || defined(RTV_IF_EBI2) 			  
			RTV_REG_MAP_SEL(DD_PAGE);
			RTV_REG_SET(0x35, 0x04); //                              
			
			RTV_REG_MAP_SEL(HOST_PAGE);
			RTV_REG_SET(0x62, g_abRtvIntrMaskRegL[RaonTvChipIdx]); //     
	#endif
			break;
  #endif
  			
  #ifdef RTV_FM_ENABLE
		case RTV_TV_MODE_FM:
	  #if defined(RTV_IF_SPI) || defined(RTV_IF_EBI2)				
		    	RTV_REG_MAP_SEL(DD_PAGE);
		    	RTV_REG_SET(0x35, 0x04); //                              

			RTV_REG_MAP_SEL(HOST_PAGE);
		    	RTV_REG_SET(0x62, g_abRtvIntrMaskRegL[RaonTvChipIdx]); //           
	  #elif defined(RTV_IF_MPEG2_SERIAL_TSIF) || defined(RTV_IF_SPI_SLAVE) || defined(RTV_IF_QUALCOMM_TSIF)	
			RTV_REG_MAP_SEL(HOST_PAGE);
			RTV_REG_SET(0x29, 0x00);	 
	  #endif	  
		    break;
  #endif		    
		    
  #ifdef RTV_TDMB_ENABLE		    
		case RTV_TV_MODE_TDMB:
	  #if defined(RTV_IF_SPI) || defined(RTV_IF_EBI2)						
		    	RTV_REG_MAP_SEL(DD_PAGE);
		    	RTV_REG_SET(0x35, 0x07); //                                       

		    	RTV_REG_MAP_SEL(HOST_PAGE);
		    	RTV_REG_SET(0x62, g_abRtvIntrMaskRegL[RaonTvChipIdx]); //                
		    
	  #elif defined(RTV_IF_MPEG2_SERIAL_TSIF) || defined(RTV_IF_SPI_SLAVE) || defined(RTV_IF_QUALCOMM_TSIF) || defined(RTV_IF_MPEG2_PARALLEL_TSIF)	
	      #ifndef RTV_TDMB_MULTI_SUB_CHANNEL_ENABLE /*                         */
		    #ifndef RTV_TDMB_FIC_POLLING_MODE
		    	RTV_REG_MAP_SEL(DD_PAGE);
		    	RTV_REG_SET(0x35, 0x01); //                             

			RTV_REG_MAP_SEL(HOST_PAGE);
		   	RTV_REG_SET(0x62, g_abRtvIntrMaskRegL[RaonTvChipIdx]); //    
		   #endif
		#else /*                         */
		#endif
	  #endif	  		    
	    		break;
  #endif			
			
		default:
			//                                                      
			break; 
    }
    
    g_afRtvStreamEnabled[RaonTvChipIdx] = TRUE;  
#endif	
}


/*                                                        */
static INLINE void rtv_StreamEnable(void)
{
#ifndef RTV_IF_MPEG2_PARALLEL_TSIF
  #if defined(RTV_IF_SPI) || defined(RTV_IF_EBI2)				
    	RTV_REG_MAP_SEL(DD_PAGE);
    	RTV_REG_SET(0x35, 0x04); //                              

	RTV_REG_MAP_SEL(HOST_PAGE);
	g_abRtvIntrMaskRegL[RaonTvChipIdx] = 0x8F; //                                                                           
    	RTV_REG_SET(0x62, g_abRtvIntrMaskRegL[RaonTvChipIdx]); 

	rtv_SetupMemory_MSC1(g_aRtvMscThresholdSize[RaonTvChipIdx]); //                     
	    
  #elif defined(RTV_IF_MPEG2_SERIAL_TSIF) || defined(RTV_IF_SPI_SLAVE) || defined(RTV_IF_QUALCOMM_TSIF)	
	RTV_REG_MAP_SEL(HOST_PAGE);
	RTV_REG_SET(0x29, 0x00);	 
	
	rtv_SetupMemory_MSC1(RTV_TS_PACKET_SIZE); //                     
  #endif	 
     
	g_afRtvStreamEnabled[RaonTvChipIdx] = TRUE;  
#endif /*                                    */ 
}



static INLINE void rtv_ConfigureTsifFormat(void)
{
	RTV_REG_MAP_SEL(COMM_PAGE);
	
#if defined(RTV_IF_MPEG2_SERIAL_TSIF) || defined(RTV_IF_SPI_SLAVE) 
  #if defined(RTV_TSIF_FORMAT_1)
	RTV_REG_SET(0x45, 0x00);    
  #elif defined(RTV_TSIF_FORMAT_2)
	RTV_REG_SET(0x45, 0x02);
  #elif defined(RTV_TSIF_FORMAT_3)
	RTV_REG_SET(0x45, 0x21);
  #elif defined(RTV_TSIF_FORMAT_4)
	RTV_REG_SET(0x45, 0x23);
  #else
	#error "Code not present"
  #endif

#elif defined(RTV_IF_QUALCOMM_TSIF)
  #if defined(RTV_TSIF_FORMAT_1)
	RTV_REG_SET(0x45, 0x00);    
  #elif defined(RTV_TSIF_FORMAT_2)
	RTV_REG_SET(0x45, 0xE9);

  #elif defined(RTV_TSIF_FORMAT_3)
	RTV_REG_SET(0x45, 0xE1);
  #elif defined(RTV_TSIF_FORMAT_4)
	RTV_REG_SET(0x45, 0x40);
  #elif defined(RTV_TSIF_FORMAT_5)
	RTV_REG_SET(0x45, 0x21);    
  #else
	#error "Code not present"
  #endif
#endif	

#if defined(RTV_IF_MPEG2_SERIAL_TSIF)
	RTV_REG_SET(0x46, 0x80);  
#elif defined(RTV_IF_QUALCOMM_TSIF)
	RTV_REG_SET(0x46, 0xA0); 
#elif defined(RTV_IF_SPI_SLAVE)
	RTV_REG_SET(0x46, 0x82); 
#endif
}


static INLINE void rtv_ResetMemory_MSC0(void)
{
	RTV_REG_MAP_SEL(DD_PAGE);
	RTV_REG_SET(0x47, 0x00);  //                                    
}

static INLINE void rtv_ResetMemory_MSC1(void)
{
#ifndef RTV_IF_MPEG2_PARALLEL_TSIF
	RTV_REG_MAP_SEL(DD_PAGE);
	RTV_REG_SET(0x48, 0x00);  //                                    
#endif	
}

static INLINE void rtv_SetupMemory_MSC0(U16 wThresholdSize)
{
#ifndef RTV_IF_MPEG2_PARALLEL_TSIF

	RTV_REG_MAP_SEL(DD_PAGE);
	
    RTV_REG_SET(0x50,(wThresholdSize>>8) & 0x0F);
    RTV_REG_SET(0x51,(wThresholdSize & 0xFF));

	RTV_REG_SET(0x47, 0x00);	

#if !defined(RTV_TDMB_MULTI_SUB_CHANNEL_ENABLE) /*                 */
  #if defined(RTV_IF_SPI) || defined(RTV_IF_EBI2)
  	RTV_REG_SET(0x47, (1/*             */<<3) | (1/*       */<<2) | 1/*           */);
  #else
    RTV_REG_SET(0x47, (0/*             */<<3) | (1/*       */<<2) | 1/*           */);	
  #endif
#else
  #if defined(RTV_IF_SPI) || defined(RTV_IF_EBI2)
    RTV_REG_SET(0x47, (1/*             */<<3) | (1/*       */<<2) | 0/*           */);
  #else
  	RTV_REG_SET(0x47, (0/*             */<<3) | (1/*       */<<2) | 0/*           */);
  #endif
#endif

	RTV_REG_SET(0x35, 0x02); //                      
	
#endif /*                                    */	
}

static INLINE void rtv_SetupMemory_MSC1(U16 wThresholdSize)
{
#ifndef RTV_IF_MPEG2_PARALLEL_TSIF
	//                                                           
	//                                                                                         
	//                                                         
	//                                                   
	//                                                     

	RTV_REG_MAP_SEL(DD_PAGE);

	/*                                                        */
    	RTV_REG_SET(0x56, wThresholdSize>>8);
	RTV_REG_SET(0x57, (wThresholdSize & 0xFF));

	RTV_REG_SET(0x48, 0x00);	
#if !defined(RTV_TDMB_MULTI_SUB_CHANNEL_ENABLE) /*                 */
  #if defined(RTV_IF_SPI) || defined(RTV_IF_EBI2)
  	RTV_REG_SET(0x48, (0/*               */<<5) | (0/*              */<<4) | (0/*             */<<3) | (1/*       */<<2) | 1/*           */);
  #else
    RTV_REG_SET(0x48, (0/*               */<<5) | (0/*              */<<4) | (0/*             */<<3) | (1/*       */<<2) | 1/*           */);			
  #endif
#else
  #if defined(RTV_IF_SPI) || defined(RTV_IF_EBI2)	
    RTV_REG_SET(0x48, (1/*               */<<5) | (1/*              */<<4) | (1/*             */<<3) | (1/*       */<<2) | 0/*           */);		
  #else    
  	RTV_REG_SET(0x48, (1/*               */<<5) | (0/*              */<<4) | (0/*             */<<3) | (1/*       */<<2) | 0/*           */);
  #endif  	
#endif
#endif /*                                    */	
}


/*                                                */
static INLINE void rtv_Set_MSC1_SUBCH0(UINT nSubChID, BOOL fSubChEnable, BOOL fRsEnable)
{
	RTV_REG_MAP_SEL(DD_PAGE);
	
#if !defined(RTV_IF_MPEG2_PARALLEL_TSIF)
	RTV_REG_SET(0x3A, (fSubChEnable << 7) | (fRsEnable << 6) | nSubChID);
#else
	if(fRsEnable ==TRUE)	
	{
		RTV_REG_SET(0xFF, 0x00);
		RTV_REG_SET(0x3A, (fSubChEnable << 7) | (fRsEnable << 6) | nSubChID);

	}
	else 
	{   RTV_REG_SET(0x3A, 0x00);
		RTV_REG_SET(0xFF, (fSubChEnable << 7) | nSubChID);
	}
#endif
}

static INLINE void rtv_Set_MSC1_SUBCH1(UINT nSubChID, BOOL fSubChEnable)
{
	RTV_REG_MAP_SEL(DD_PAGE);
	RTV_REG_SET(0x3B, (fSubChEnable << 7) | nSubChID);
}

static INLINE void rtv_Set_MSC1_SUBCH2(UINT nSubChID, BOOL fSubChEnable)
{
	RTV_REG_MAP_SEL(DD_PAGE);
	RTV_REG_SET(0x3C, (fSubChEnable << 7) | nSubChID);
}

static INLINE void rtv_Set_MSC0_SUBCH3(UINT nSubChID, BOOL fSubChEnable)
{
	RTV_REG_MAP_SEL(DD_PAGE);
	RTV_REG_SET(0x3D, (fSubChEnable << 7) | nSubChID);
}

static INLINE void rtv_Set_MSC0_SUBCH4(UINT nSubChID, BOOL fSubChEnable)
{
	RTV_REG_MAP_SEL(DD_PAGE);
	RTV_REG_SET(0x3E, (fSubChEnable << 7) | nSubChID);
}

static INLINE void rtv_Set_MSC0_SUBCH5(UINT nSubChID, BOOL fSubChEnable)
{
	RTV_REG_MAP_SEL(DD_PAGE);
	RTV_REG_SET(0x3F, (fSubChEnable << 7) | nSubChID);
}

static INLINE void rtv_Set_MSC0_SUBCH6(UINT nSubChID, BOOL fSubChEnable)
{
	RTV_REG_MAP_SEL(DD_PAGE);
	RTV_REG_SET(0x42, (fSubChEnable << 7) | nSubChID);
}


/*                                                                              
  
                          
  
                                                                              */ 
static INLINE void rtv_SetupMemory_FIC(void)
{
	RTV_REG_MAP_SEL(DD_PAGE);
	RTV_REG_SET(0x46, 0x10); //                                               
	RTV_REG_SET(0x46, 0x16); //           
}

static INLINE void rtv_ResetMemory_FIC(void)
{

	RTV_REG_MAP_SEL(DD_PAGE);
	RTV_REG_SET(0x46, 0x00); 
}



/*                                                                              
  
                                  
  
                                                                              */ 
#ifdef RTV_IF_MPEG2_PARALLEL_TSIF
#if defined(RTV_ISDBT_ENABLE)
static INLINE void rtv_SetParallelTsif_1SEG_Only(void)
{
	RTV_REG_MAP_SEL(FEC_PAGE);
	
#if defined(RTV_TSIF_FORMAT_1)
    RTV_REG_SET(0x6E, 0x11);    
#elif defined(RTV_TSIF_FORMAT_2)
    RTV_REG_SET(0x6E, 0x13);    
#elif defined(RTV_TSIF_FORMAT_3)
    RTV_REG_SET(0x6E, 0x10);    
#elif defined(RTV_TSIF_FORMAT_4)
    RTV_REG_SET(0x6E, 0x12);    
#else
    #error "Code not present"
#endif
	RTV_REG_SET(0x6F, 0x02);
    RTV_REG_SET(0x70, 0x88);   
}

#elif defined(RTV_TDMB_ENABLE)
static INLINE void rtv_SetParallelTsif_TDMB_Only(void)
{
	RTV_REG_MAP_SEL(0x09);
	
#if defined(RTV_TSIF_FORMAT_1)
    RTV_REG_SET(0xDD, 0xD0);    
#elif defined(RTV_TSIF_FORMAT_2)
    RTV_REG_SET(0xDD, 0xD2);    
#elif defined(RTV_TSIF_FORMAT_3)
    RTV_REG_SET(0xDD, 0xD1);    
#elif defined(RTV_TSIF_FORMAT_4)
    RTV_REG_SET(0xDD, 0xD3);    
#else
    #error "Code not present"
#endif
	RTV_REG_SET(0xDE, 0x12);
	RTV_REG_SET(0x45, 0xB0);
}
#else
	#error "Code not present"
#endif
#endif


/*                                                                              
                                             
                                                                              */ 
void rtv_ConfigureHostIF(void);
INT  rtv_InitSystem(E_RTV_TV_MODE_TYPE eTvMode, E_RTV_ADC_CLK_FREQ_TYPE eAdcClkFreqType);

#ifdef __cplusplus 
} 
#endif 

#endif /*                       */




