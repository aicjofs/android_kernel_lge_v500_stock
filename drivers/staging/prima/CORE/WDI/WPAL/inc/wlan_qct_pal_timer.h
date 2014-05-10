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

#if !defined( __WLAN_QCT_PAL_TIMER_H )
#define __WLAN_QCT_PAL_TIMER_H

/**=========================================================================
  
  \file  wlan_qct_pal_timer.h
  
  \brief define synchronization objects PAL exports. wpt = (Wlan Pal Type) wpal = (Wlan PAL)
               
   Definitions for platform independent.
  
   Copyright 2010-2011 (c) Qualcomm, Incorporated.  All Rights Reserved.
   
   Qualcomm Confidential and Proprietary.
  
  ========================================================================*/

#include "wlan_qct_pal_type.h"
#include "wlan_qct_pal_status.h"
#include "wlan_qct_os_timer.h"


typedef void (*wpal_timer_callback)(void *pUserData);

typedef struct
{
   wpt_os_timer timer;
   wpal_timer_callback callback;
   void *pUserData;
} wpt_timer;


/*                                                                           
                                                 
          
                                                               
                                      
                                                                                 
           
                                                           
                                                                           */
wpt_status wpalTimerInit(wpt_timer * pTimer, wpal_timer_callback callback, void *pUserData);

/*                                                                           
                                                   
          
                                                               
           
                                                           
                                                                           */
wpt_status wpalTimerDelete(wpt_timer * pTimer);

/*                                                                           
                                                                  
          
                                                               
                                                                       
           
                                                           
                                                                           */
wpt_status wpalTimerStart(wpt_timer * pTimer, wpt_uint32 timeout);

/*                                                                           
                                                                                                                             
          
                                                               
           
                                                           
                                                                           */
wpt_status wpalTimerStop(wpt_timer * pTimer);

/*                                                                           
                                              
          
                                                                    
           
                                                           
                                                                           */
wpt_status wpalSleep(wpt_uint32 timeout);



#endif //                       
