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
* TITLE 	  : RAONTECH TV configuration header file. 
*
* FILENAME    : raontv_port.h
*
* DESCRIPTION : 
*		Configuration for RAONTECH TV Services.
*
********************************************************************************/

/*                                                                                
                  
 
                              
                                                                                 
                                                                                   
                                                                     
                                                    
                                                       
                                                                                
                             
                                            
                                                                                          
                                                                               */

#ifndef __RAONTV_PORT_H__
#define __RAONTV_PORT_H__

#ifdef __cplusplus 
extern "C"{ 
#endif  


/*                                                                              
 
                       
 
                                                                                */
/*                                                                              
                                               
                                                                              */ 
#ifdef  __KERNEL__ /*              */
#include <asm/io.h>
#include <linux/kernel.h>
#include <linux/delay.h>
#include <linux/mm.h>
#include <linux/mutex.h>
#endif

/*                                                                              
                                                                
                                                                              */ 
#define RAONTV_CHIP_ADDR	0x86 

/*                                                                              
                                              
                                                                              */ 
typedef int				BOOL;
typedef signed char		S8;
typedef unsigned char		U8;
typedef signed short		S16;
typedef unsigned short		U16;
typedef signed int			S32;
typedef unsigned int		U32;

typedef int                 INT;
typedef unsigned int        UINT;
typedef long                LONG;
typedef unsigned long       ULONG;
 
typedef volatile U8			VU8;
typedef volatile U16		VU16;
typedef volatile U32		VU32;

#define INLINE			inline

/*                                                                              
                                                  
                                                                         
                                                                              */ 
//                          

/*                                                                              
                                            
                                                                              */ 
#define RTV_ISDBT_ENABLE
//                       
//                     


/*                                                                              
                                                      
                                                                              */ 
//                                           
//                                      
//                                                 

#if defined(CONFIG_KS1001)  || defined(CONFIG_KS1103)
	#define RAONTV_CHIP_PKG_LGA	//                   
#elif defined(CONFIG_MACH_LGE_P940)
	#define RAONTV_CHIP_PKG_LGA	//                   
//                                       
#elif 1
	#define RAONTV_CHIP_PKG_LGA	//                   
//                                     
#else
	#error "Code not present"
#endif

/*                                                                              
                                                
                                                  
                                                                              */ 
#define RTV_SRC_CLK_FREQ_KHz			32000
	

/*                                                                              
                         
                                                                              */  
#if defined(CONFIG_KS1001)  || defined(CONFIG_KS1103)
	#define RTV_PWR_EXTERNAL
	
#elif defined(CONFIG_MACH_LGE_P940)
	#define RTV_PWR_LDO
//                                       
#elif defined(CONFIG_MACH_MSM8960_L_DCM)
	#define RTV_PWR_LDO
//                                     
#else
	#error "Code not present"
#endif
//                    


/*                                                                              
                                   
                                                                              */  
#define RTV_IO_1_8V
//                   
//                   

/*                                                                              
                              
                                                                              */  
//                                                            
//                                                                                                  
//                                                      
#define RTV_IF_SPI //                    
//                                              
//                                                        

/*                                                                              
                                           
                                                                              */  
#define RTV_DELAY_MS(ms)    mdelay(ms) 


/*                                                                              
                                   
                                                                              */  
#if 1
	#define RTV_DBGMSG0(fmt)					printk(fmt)
	#define RTV_DBGMSG1(fmt, arg1)				printk(fmt, arg1) 
	#define RTV_DBGMSG2(fmt, arg1, arg2)		printk(fmt, arg1, arg2) 
	#define RTV_DBGMSG3(fmt, arg1, arg2, arg3)	printk(fmt, arg1, arg2, arg3) 
#else
	/*                                   */
	#define RTV_DBGMSG0(fmt)					((void)0) 
	#define RTV_DBGMSG1(fmt, arg1)				((void)0) 
	#define RTV_DBGMSG2(fmt, arg1, arg2)		((void)0) 
	#define RTV_DBGMSG3(fmt, arg1, arg2, arg3)	((void)0) 
#endif
/*                              */


/*                                                                              
 
                                
 
                                                                                */
/*                                                                              
                                           
                                                                     
                                                                              */ 
#if defined(RTV_ISDBT_ENABLE)
	//                               
#endif


/*                                                                              
 
                               
 
                                                                                */
#if defined(RTV_TDMB_ENABLE) 
	#define RTV_TDMB_FIC_POLLING_MODE

	/*                                                             */
//                                          
	
	#define RTV_MAX_NUM_MULTI_SUB_CHANNEL		2  //                                       
	
	#if defined(RTV_TDMB_MULTI_SUB_CHANNEL_ENABLE) && defined(__KERNEL__)
		/*                                                                                                                             */
		#define RTV_CIF_LINUX_USER_SPACE_COPY_USED
	#endif
#endif


/*                                                                              
 
                                        
 
                                                                                */
#if defined(RTV_IF_MPEG2_SERIAL_TSIF) || defined(RTV_IF_QUALCOMM_TSIF) ||defined(RTV_IF_SPI_SLAVE) || defined(RTV_IF_MPEG2_PARALLEL_TSIF) 
	/*                                                                              
                                                            
                                                                               */  
	//                         
	//                         
	//                         
	#define RTV_TSIF_FORMAT_4
	//                         

	//                                              
	#define RTV_TSIF_CLK_SPEED_DIV_4 //           
	//                                              
	//                                              

	/*                                                                          
                                    
                                                                           */  
	unsigned char mtv818_i2c_read(U8 reg);
	void mtv818_i2c_read_burst(U8 reg, U8 *buf, int size);
	void mtv818_i2c_write(U8 reg, U8 val);
    #define	RTV_REG_GET(reg)            		mtv818_i2c_read((U8)reg)
    #define	RTV_REG_BURST_GET(reg, buf, size) 	mtv818_i2c_read_burst(reg, buf, size)
    #define	RTV_REG_SET(reg, val)       		mtv818_i2c_write((U8)reg, (U8)val)
	#define	RTV_REG_MASK_SET(reg, mask, val) 								\
		do {																\
			U8 tmp;															\
			tmp = (RTV_REG_GET(reg)|(U8)(mask)) & (U8)((~(mask))|(val));	\
			RTV_REG_SET(reg, tmp);											\
		} while(0)

#elif defined(RTV_IF_SPI)
	/*                                                                          
                                    
                                                                           */  
	unsigned char mtv250_spi_read(unsigned char reg);
	void mtv250_spi_read_burst(unsigned char reg, unsigned char *buf, int size);
	void mtv250_spi_write(unsigned char reg, unsigned char val);

    #define	RTV_REG_GET(reg)            			(U8)mtv250_spi_read(reg)
    #define	RTV_REG_BURST_GET(reg, buf, size) 		mtv250_spi_read_burst(reg, buf, size)
    #define	RTV_REG_SET(reg, val)       			mtv250_spi_write(reg, val)       
    #define	RTV_REG_MASK_SET(reg, mask, val) 								\
		do {																\
			U8 tmp;															\
			tmp = (RTV_REG_GET(reg)|(U8)(mask)) & (U8)((~(mask))|(val));	\
			RTV_REG_SET(reg, tmp);											\
		} while(0)
    
#elif defined(RTV_IF_EBI2)
	/*                                                                          
                                    
                                                                           */  
	#define RTV_EBI2_MEM_WITDH  8 // 
	//                                  
	//                                 
		
    #if (RTV_EBI2_MEM_WITDH == 8)
	extern VU8 g_bRtvEbiMapSelData;

	static INLINE U8 RTV_REG_GET(U8 reg)
	{	
		U8 bData;
		if(reg == 0x3)
		{
			bData = g_bRtvEbiMapSelData;
			if(bData ==0x09 || bData == 0x0A || bData == 0x0B || bData == 0x0C )
			{
				*(VU8 *)(RAONTV_CHIP_ADDR | 0x04 ) = 0x03;
				*(VU8 *)(RAONTV_CHIP_ADDR | 0x06 ) = 0x00;
				*(VU8 *)(RAONTV_CHIP_ADDR | 0x00 ) = bData;
			}
		}
		else
		{
			   *(VU8 *)(RAONTV_CHIP_ADDR | 0x04 ) = reg;
			   *(VU8 *)(RAONTV_CHIP_ADDR | 0x06 ) = g_bRtvEbiMapSelData;
			   bData = *(VU8 *)(RAONTV_CHIP_ADDR | 0x00 );
		
		}	

		return bData;
	}

	static INLINE void RTV_REG_SET(U8 reg, U8 val)
	{
		if(reg == 0x3) 
		{
		   	g_bRtvEbiMapSelData = val;
			if(val ==0x09 || val ==0x0A || val ==0x0B || val ==0x0C )
			{
				   *(VU8 *)(RAONTV_CHIP_ADDR | 0x04 ) = 0x03;
				   *(VU8 *)(RAONTV_CHIP_ADDR | 0x06 ) = 0x00;
				   *(VU8 *)(RAONTV_CHIP_ADDR | 0x00 ) = val;
			}
		}
		else
		{
			   *(VU8 *)(RAONTV_CHIP_ADDR | 0x04 ) = reg;
			   *(VU8 *)(RAONTV_CHIP_ADDR | 0x06 ) = g_bRtvEbiMapSelData;
			   *(VU8 *)(RAONTV_CHIP_ADDR | 0x00 ) = val;
		}
	}
	
	#define RTV_REG_MASK_SET(reg, mask, val)								\
		do {																\
		U8 tmp;															\
		tmp = (RTV_REG_GET(reg)|(U8)(mask)) & (U8)((~(mask))|(val));	\
		RTV_REG_SET(reg, tmp);											\
		} while(0)
		    
    #elif (RTV_EBI2_MEM_WITDH == 16)
    
    #elif (RTV_EBI2_MEM_WITDH == 32)
        
    #else
        #error "Can't support to memory witdh!"
    #endif
    
#else
	#error "Must define the interface definition !"
#endif


#if defined(RTV_IF_SPI) || (defined(RTV_TDMB_ENABLE) && !defined(RTV_TDMB_FIC_POLLING_MODE))	
	#if defined(__KERNEL__)	
		extern struct mutex raontv_guard;
		#define RTV_GUARD_INIT		mutex_init(&raontv_guard)
		#define RTV_GUARD_LOCK		mutex_lock(&raontv_guard)
		#define RTV_GUARD_FREE		mutex_unlock(&raontv_guard)
	#else
		//     
		#define RTV_GUARD_INIT		((void)0)
		#define RTV_GUARD_LOCK		((void)0)
		#define RTV_GUARD_FREE 	((void)0)
	#endif
	
#else
	#define RTV_GUARD_INIT		((void)0)
	#define RTV_GUARD_LOCK		((void)0)
	#define RTV_GUARD_FREE 	((void)0)
#endif


/*                                                                              
              
                                                                              */  
#if !defined(RAONTV_CHIP_PKG_WLCSP) && !defined(RAONTV_CHIP_PKG_QFN)  && !defined(RAONTV_CHIP_PKG_LGA)
	#error "Must define the package type !"
#endif

#if !defined(RTV_PWR_EXTERNAL) && !defined(RTV_PWR_LDO)  && !defined(RTV_PWR_DCDC)
	#error "Must define the power type !"
#endif

#if !defined(RTV_IO_1_8V) && !defined(RTV_IO_2_5V)  && !defined(RTV_IO_3_3V)
	#error "Must define I/O voltage!"
#endif

 
#if defined(RTV_IF_MPEG2_SERIAL_TSIF) || defined(RTV_IF_SPI_SLAVE) || defined(RTV_IF_MPEG2_PARALLEL_TSIF)  || defined(RTV_IF_QUALCOMM_TSIF) || defined(RTV_IF_SPI)
    #if (RAONTV_CHIP_ADDR >= 0xFF)
        #error "Invalid chip address"
    #endif
#elif defined(RTV_IF_EBI2)
    #if (RAONTV_CHIP_ADDR <= 0xFF)
        #error "Invalid chip address"
    #endif
    
#else
	#error "Must define the interface definition !"
#endif


#ifdef RTV_IF_MPEG2_PARALLEL_TSIF
	#if defined(RTV_FM_ENABLE) || defined(RTV_DAB_ENABLE) || defined(RAONTV_CHIP_PKG_WLCSP)  || defined(RAONTV_CHIP_PKG_LGA)
		#error "Not support parallel TSIF!"
	#endif
	
	#if defined(RTV_TDMB_ENABLE) && defined(RTV_TDMB_MULTI_SUB_CHANNEL_ENABLE)
		#error "Not support T-DMB multi sub channel mode!"
	#endif
#endif


#ifdef RTV_DUAL_CHIP_USED
	#if defined(RTV_IF_MPEG2_PARALLEL_TSIF) || defined(RTV_IF_EBI2)
		#error "Not support for diversity!"
	#endif
#endif


#if defined(RTV_TDMB_ENABLE) && defined(RTV_TDMB_MULTI_SUB_CHANNEL_ENABLE)
   #ifdef RTV_MAX_NUM_MULTI_SUB_CHANNEL
	#if (RTV_MAX_NUM_MULTI_SUB_CHANNEL > 4)
		#error "Must less than 4"
	#endif		
   #else
   	#define RTV_MAX_NUM_MULTI_SUB_CHANNEL 	1
   #endif
#else
   #ifdef RTV_MAX_NUM_MULTI_SUB_CHANNEL
	#undef RTV_MAX_NUM_MULTI_SUB_CHANNEL
	#define RTV_MAX_NUM_MULTI_SUB_CHANNEL		1 //                   
   #else
   	#define RTV_MAX_NUM_MULTI_SUB_CHANNEL		1
   #endif
#endif


void rtvOEM_ConfigureInterrupt(void);
void rtvOEM_PowerOn(int on);

#ifdef __cplusplus 
} 
#endif 

#endif /*                   */

