#ifndef _ISDBT_COMMON_H_
#define _ISDBT_COMMON_H_

#include "../../../../arch/arm/mach-msm/lge/j1/board-j1.h"

#define GPIO_MB86A35S_SPIC_XIRQ 		27	//     
#define GPIO_ISDBT_IRQ 					33	
#define GPIO_MB86A35S_FRAME_LOCK		46	//                
//                        
//                                                                
//                                                                                   
#define GPIO_MB86A35S_SPIS_XIRQ 		77	//                                            
//                      
#define GPIO_ISDBT_PWR_EN 				85	//                           
#define GPIO_ISDBT_RST 					1
//                                          

#define ISDBT_DEFAULT_NOTUSE_MODE -1
#define ISDBT_UHF_MODE 0
#define ISDBT_VHF_MODE 1

//                               
#define DMB_ANT_SEL_P_EAR       	    PM8921_GPIO_PM_TO_SYS(11)
//                                                         
#define GPIO_ONESEG_INT		       	    PM8921_GPIO_PM_TO_SYS(16)

#endif //                