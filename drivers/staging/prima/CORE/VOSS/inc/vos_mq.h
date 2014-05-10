/*
 * Copyright (c) 2012, The Linux Foundation. All rights reserved.
 *
 * Previously licensed under the ISC license by Qualcomm Atheros, Inc.
 *
 *
 * Permission to use, copy, modify, and/or distribute this software for
 * any purpose with or without fee is hereby granted, provided that the
 * above copyright notice and this permission notice appear in all
 * copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL
 * WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE
 * AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL
 * DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR
 * PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */

#if !defined( __VOS_MQ_H )
#define __VOS_MQ_H

/**=========================================================================
  
  \file  vos_mq.h
  
  \brief virtual Operating System Services (vOSS) message queue APIs
               
   Message Queue Definitions and API
  
   Copyright 2008 (c) Qualcomm, Incorporated.  All Rights Reserved.
   
   Qualcomm Confidential and Proprietary.
  
  ========================================================================*/

/*          */

/*                                                                          
               
                                                                          */
#include <vos_types.h>
#include <vos_status.h>

/*                                                                           
                                        
                                                                          */

/*                                                                           
                   
                                                                          */
  
//                     
//                                                                        
//                            
// 
//                                                            
//                                                 

typedef struct vos_msg_s
{
    v_U16_t type;
    /*
                                                                          
                              
     */
    v_U16_t reserved;
    /* 
                                                      
                                                  
                                                               
                                                 
       
                                                                   
     */
    void *bodyptr;
    
    v_U32_t bodyval;    
    
} vos_msg_t;


/*                                                                          
                                        
                                                                          */
  
//                     
typedef enum
{
  //                                             
  VOS_MQ_ID_SME = VOS_MODULE_ID_SME,
  
  //                                            
  VOS_MQ_ID_PE = VOS_MODULE_ID_PE, 
  
#ifndef FEATURE_WLAN_INTEGRATED_SOC
  //                                             
  VOS_MQ_ID_HAL = VOS_MODULE_ID_HAL,
#else
  //                                             
  VOS_MQ_ID_WDA = VOS_MODULE_ID_WDA,
#endif

  //                                            
  VOS_MQ_ID_TL = VOS_MODULE_ID_TL,

#ifndef FEATURE_WLAN_INTEGRATED_SOC
  //                                             
  VOS_MQ_ID_SSC = VOS_MODULE_ID_SSC,
#endif
  
  //                                                        
  VOS_MQ_ID_SYS = VOS_MODULE_ID_SYS,

#ifndef FEATURE_WLAN_INTEGRATED_SOC
  //                                                   
  VOS_MQ_ID_SAL = VOS_MODULE_ID_SAL
#else
  //                                             
  VOS_MQ_ID_WDI = VOS_MODULE_ID_WDI,
#endif
  
} VOS_MQ_ID;


/*                                                                            
  
                                                                  

                                                                              
                                                
  
      
            
           
            
           
        
  
                                                                         
                        
         
                                                                           
                                                                           
                                                                         
                                                                              
                                                         
  
                                                                       
                               
          
                                                                          
                                            
          
                                                                   
          
                                                                       
                             

     
  
                                                                            */
VOS_STATUS vos_mq_post_message( VOS_MQ_ID msgQueueId, vos_msg_t *message );


/*                                                                            
  
                                                                             

                                                                           
                                                                           
                         
  
      
           
                
       
  
                                                                         
                        
         
                                                                                
                                                                           
                                                                              
                                                                             
                                                                
  
                                                                       
                               
          
                                                                          
                                            
          
                                                                   
          
                                                                       
                             

     
  
                                                                            */
VOS_STATUS vos_tx_mq_serialize( VOS_MQ_ID msgQueueId, vos_msg_t *message );

#ifdef FEATURE_WLAN_INTEGRATED_SOC
/*                                                                            

                                                                             

                                                                          
                                                                          
                         

      
            
       

                                                                         
                        

                                                                               
                                                                           
                                                                             
                                                                            
                                                                

                                                                       
                               

                                                                         
                                            

                                                              

                                                                       
                             

     

                                                                            */
VOS_STATUS vos_rx_mq_serialize( VOS_MQ_ID msgQueueId, vos_msg_t *message );

#endif

#endif //                       
