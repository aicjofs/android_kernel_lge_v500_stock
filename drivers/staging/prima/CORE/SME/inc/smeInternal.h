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

#if !defined( __SMEINTERNAL_H )
#define __SMEINTERNAL_H


/**=========================================================================
  
  \file  smeInternal.h
  
  \brief prototype for SME internal structures and APIs used for SME and MAC
  
   Copyright 2008 (c) Qualcomm, Incorporated.  All Rights Reserved.
   
   Qualcomm Confidential and Proprietary.
  
  ========================================================================*/

/*          */

/*                                                                          
               
                                                                          */
#include "vos_status.h"
#include "vos_lock.h"
#include "vos_trace.h"
#include "vos_memory.h"
#include "vos_types.h"
#include "csrLinkList.h"

/*                                                                           
                   
                                                                          */

//                                  
typedef enum eSmeCommandType 
{
    eSmeNoCommand = 0, 
    eSmeDropCommand,
    //   
    eSmeCsrCommandMask = 0x10000,   //                                                              
    eSmeCommandScan,
    eSmeCommandRoam, 
    eSmeCommandWmStatusChange, 
    eSmeCommandSetKey,
    eSmeCommandRemoveKey,
    eSmeCommandAddStaSession,
    eSmeCommandDelStaSession,
    //   
    eSmePmcCommandMask = 0x20000, //                        
    eSmeCommandEnterImps,
    eSmeCommandExitImps,
    eSmeCommandEnterBmps,
    eSmeCommandExitBmps,
    eSmeCommandEnterUapsd,
    eSmeCommandExitUapsd,
    eSmeCommandEnterWowl,
    eSmeCommandExitWowl,
    eSmeCommandEnterStandby,
    //   
    eSmeQosCommandMask = 0x40000,  //                        
    eSmeCommandAddTs,
    eSmeCommandDelTs,
#ifdef FEATURE_OEM_DATA_SUPPORT
    eSmeCommandOemDataReq = 0x80000, //                                 
#endif
#ifdef WLAN_FEATURE_P2P
    eSmeCommandRemainOnChannel,
    eSmeCommandNoAUpdate,
#endif
} eSmeCommandType;


typedef enum eSmeState
{
    SME_STATE_STOP,
    SME_STATE_START,
    SME_STATE_READY,
} eSmeState;


#define SME_IS_START(pMac)  (SME_STATE_STOP != (pMac)->sme.state)
#define SME_IS_READY(pMac)  (SME_STATE_READY == (pMac)->sme.state)


typedef struct tagSmeStruct
{
    eSmeState state;
    vos_lock_t lkSmeGlobalLock;
    tANI_U32 totalSmeCmd;
    void *pSmeCmdBufAddr;
    tDblLinkList smeCmdActiveList;
    tDblLinkList smeCmdPendingList;
    tDblLinkList smeCmdFreeList;   //                          
    void (*pTxPerHitCallback) (void *pCallbackContext); /*                                */ 
    void *pTxPerHitCbContext;
} tSmeStruct, *tpSmeStruct;


#endif //                               
