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

#ifndef __VOSS_WRAPPER_H
#define __VOSS_WRAPPER_H

#ifdef __cplusplus
extern "C" {
#endif

/*===========================================================================
  @file VossWrapper.h

  @brief This header file contains the various structure definitions and 
  function prototypes for the RTOS abstraction layer, implemented for VOSS

  Copyright (c) 2008 QUALCOMM Incorporated.
  All Rights Reserved.
  Qualcomm Confidential and Proprietary
===========================================================================*/

/*                                                                            
    
                                             
   
   
                                                                        
                                                                 
   
   
                                    
   
   
                                      
                                                                             
                                                                         
                                         
                                                                           
                                                                              
                              
                                                                               
                                                    
                                    
     
                                                                           */ 

/*                                                                           
                
                                                                           */

#include "sirTypes.h"
#include "sirParams.h"
#include "sysDef.h"
#ifdef FEATURE_WLAN_NON_INTEGRATED_SOC
#include "halDataStruct.h"
#endif
#include "aniDbgTest.h"
#include "vos_timer.h"
#include "palApi.h"
#include "vos_types.h"
#ifdef FEATURE_WLAN_INTEGRATED_SOC
#include "vos_trace.h"
#include "vos_memory.h"
#endif

/*                                                  */



/*                                                 */

#define TX_NO_WAIT          0
#define TX_WAIT_FOREVER     0xFFFFFFFFUL
#define TX_AUTO_ACTIVATE    1
#define TX_NO_ACTIVATE      0



/*                     */
#define TX_SUCCESS          0x00
#define TX_QUEUE_FULL    0x01
//    
#define TX_NO_INSTANCE      0x0D
//    
#define TX_TIMER_ERROR      0x15
#define TX_TICK_ERROR       0x16
//    


#ifndef true
#define true                1
#endif

#ifndef false
#define false               0
#endif

/*                                                                                                 
                                                      */
//                                                                         
//                                                        
#define TX_MSECS_IN_1_TICK  SYS_TICK_DUR_MS

//                                                                                   
#define TX_AIRGO_TMR_SIGNATURE   0xDEADBEEF


/*                                                                                        
                                            */

#define  tx_thread_create(a, b, c, d, e, f, g, h, i)
#define  tx_thread_delete(a)
#define  tx_thread_resume(a)

//                                                                                                   
//                                                                                                       
//                                       
#define  tx_thread_sleep(uSecs)

#define  tx_thread_suspend(a)
#define  tx_thread_terminate(a)
#define  tx_thread_identify() NULL

#define  tx_queue_create(a, b, c, d, e) TX_SUCCESS
#define  tx_queue_delete(a) TX_SUCCESS
#define  tx_queue_get_ptr(a)    TX_SUCCESS
#define  tx_queue_flush(a)  TX_SUCCESS
#define  tx_queue_receive(a, b, c)  TX_SUCCESS
#define  tx_queue_send(a, b, c) TX_SUCCESS

#define  tx_disable_intr()
#define  tx_enable_intr() 


#define TX_DISABLE_INTR tx_disable_intr()
#define TX_ENABLE_INTR  tx_enable_intr()

#ifdef TIMER_MANAGER
#define  tx_timer_create(a, b, c, d, e, f, g)   tx_timer_create_intern_debug((v_PVOID_t)pMac, a, b, c, d, e, f, g, __FILE__, __LINE__)
#else
#define  tx_timer_create(a, b, c, d, e, f, g)   tx_timer_create_intern((v_PVOID_t)pMac, a, b, c, d, e, f, g)
#endif


/*                                                                         */
/*                                                                         */
/*                                                                         */
/*                                                                         */
/*                                                                         */
/*                                                                         */
/*                                                                         */
/*                                                                         */
/*                                                                         */
/*                                                                         */
/*                                                                         */

typedef v_PVOID_t TX_THREAD;
typedef v_PVOID_t TX_QUEUE;
typedef v_PVOID_t TX_MUTEX;

/*                                                                    */
/*                                                                    */
/*                                                                    */
/*                                                                    */
/*                                                                    */
typedef v_SLONG_t TX_SEMAPHORE;

/*                                                                    */
/*                                                                    */
/*                                                                    */
/*                                                                    */
/*                                                                    */
/*                                                                    */
typedef struct TX_TIMER_STRUCT
{
#ifdef WLAN_DEBUG
#define TIMER_MAX_NAME_LEN 50
    char timerName[TIMER_MAX_NAME_LEN];
#endif
    v_ULONG_t  tmrSignature;
    v_VOID_t   (*pExpireFunc)(v_PVOID_t, tANI_U32);
    tANI_U32   expireInput;
    v_ULONG_t  initScheduleTimeInMsecs;
    v_ULONG_t  rescheduleTimeInMsecs;
    vos_timer_t vosTimer;

    //                                                                           
    //                                             
    v_PVOID_t  pMac;
    tANI_U8 sessionId;

} TX_TIMER;

#define TX_TIMER_VALID(timer) (timer.pMac != 0)

typedef struct sAniSirTxWrapper
{
    //                                                                                 
    v_PVOID_t  pAdapter;

} tAniSirTxWrapper, *tpAniSirTxWrapper;



extern v_ULONG_t tx_time_get(v_VOID_t);
extern v_UINT_t  tx_timer_activate(TX_TIMER*);
extern v_UINT_t  tx_timer_change(TX_TIMER*, v_ULONG_t, v_ULONG_t);
extern v_UINT_t  tx_timer_change_context(TX_TIMER*, tANI_U32);
#ifdef TIMER_MANAGER
extern v_UINT_t  tx_timer_create_intern_debug(v_PVOID_t, TX_TIMER*, char *, v_VOID_t(*)(v_PVOID_t, tANI_U32), 
               tANI_U32, v_ULONG_t, v_ULONG_t, v_ULONG_t, char* fileName, v_U32_t lineNum );
#else
extern v_UINT_t  tx_timer_create_intern(v_PVOID_t, TX_TIMER*, char *, v_VOID_t(*)(v_PVOID_t, tANI_U32), tANI_U32, v_ULONG_t, v_ULONG_t, v_ULONG_t);
#endif
extern v_UINT_t  tx_timer_deactivate(TX_TIMER*);
extern v_UINT_t  tx_timer_delete(TX_TIMER*);
extern v_BOOL_t  tx_timer_running(TX_TIMER*);



//                                                              
extern v_VOID_t tx_voss_wrapper_init(v_PVOID_t, v_PVOID_t);

#define schAddDphActivityQueueWin( a, b, c )

//                              
extern v_ULONG_t gDbgLevel;
extern v_ULONG_t gDbgMask;
extern int gHalBufCnt;

/*                                                                */

extern v_U64_t utilGetCurrentTime(void);


#ifdef __cplusplus
}
#endif


#endif



