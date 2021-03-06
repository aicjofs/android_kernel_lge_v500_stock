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

#if !defined( BAP_HDD_MISC_H )
#define BAP_HDD_MISC_H

/**===========================================================================
  
  \file  BAP_HDD_MISC.h
  
  \brief Linux HDD Adapter Type
         Copyright 2008 (c) Qualcomm, Incorporated.
         All Rights Reserved.
         Qualcomm Confidential and Proprietary.
  
  ==========================================================================*/
  
/*                                                                            
               
                                                                           */ 
  
#include <bapApi.h>
#include <vos_types.h>
/*                                                                            
                                        
                                                                           */ 

/*                                                                            
  
                                                                              
     
  
                                              
  
              
  
           
                                                                

                                                               
                                

              
                                                                            */
VOS_STATUS WLANBAP_SetConfig
(
    WLANBAP_ConfigType *pConfig
);

/*                                                                            
  
                                                                  
  
  
        
                         
  
           
                                                                

                                                       
                                

              
                                                                            */
VOS_STATUS WLANBAP_RegisterWithHCI(hdd_adapter_t *pAdapter);

/*                                                                            
  
                                                                      
  
  
             
  
           
                                                                

                                                         
                                

              
                                                                            */
VOS_STATUS WLANBAP_DeregisterFromHCI(void);

/*                                                                            
  
                                                                       
  
  
             
  
           
                                                                

                                                         
                                

              
                                                                            */
VOS_STATUS WLANBAP_StopAmp(void);

/*                                                                            
  
                                                                            
  
  
             
  
           
                                                                

                                    
                                       

              
                                                                            */
v_BOOL_t WLANBAP_AmpSessionOn(void);
#endif    //                                   
