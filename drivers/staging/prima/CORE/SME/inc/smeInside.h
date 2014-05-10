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

#if !defined( __SMEINSIDE_H )
#define __SMEINSIDE_H


/**=========================================================================
  
  \file  smeInside.h
  
  \brief prototype for SME structures and APIs used insside SME
  
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
#include "sirApi.h"
#include "csrInternal.h"
#include "sme_QosApi.h"
#include "smeQosInternal.h"


#ifdef FEATURE_OEM_DATA_SUPPORT
#include "oemDataInternal.h"
#endif

#if defined WLAN_FEATURE_VOWIFI
#include "sme_RrmApi.h"
#endif


/*                                                                           
                   
                                                                          */

#define SME_TOTAL_COMMAND  20


typedef struct sGenericPmcCmd
{
    tANI_U32 size;  //                                    
    tRequestFullPowerReason fullPowerReason;
    tANI_BOOLEAN fReleaseWhenDone; //                                                                             
    union
    {
        tExitBmpsInfo exitBmpsInfo;
        tSirSmeWowlEnterParams enterWowlInfo;
    }u;
} tGenericPmcCmd;


typedef struct sGenericQosCmd
{
    sme_QosWmmTspecInfo tspecInfo;
    sme_QosEdcaAcType ac;
    v_U8_t tspec_mask;
} tGenericQosCmd;

#ifdef WLAN_FEATURE_P2P
typedef struct sRemainChlCmd
{
    tANI_U8 chn;
    tANI_U8 phyMode;
    tANI_U32 duration;
    void* callback;
    void* callbackCtx;
}tRemainChlCmd;

typedef struct sNoACmd
{
    tP2pPsConfig NoA;
} tNoACmd;
#endif

typedef struct tagSmeCmd
{
    tListElem Link;
    eSmeCommandType command;
    tANI_U32 sessionId;
    union
    {
        tScanCmd scanCmd;
        tRoamCmd roamCmd;
        tWmStatusChangeCmd wmStatusChangeCmd;
        tSetKeyCmd setKeyCmd;
        tRemoveKeyCmd removeKeyCmd;
        tGenericPmcCmd pmcCmd;
        tGenericQosCmd qosCmd;
#ifdef FEATURE_OEM_DATA_SUPPORT
        tOemDataCmd oemDataCmd;
#endif
#ifdef WLAN_FEATURE_P2P
        tRemainChlCmd remainChlCmd;
        tNoACmd NoACmd;
#endif
        tAddStaForSessionCmd addStaSessionCmd;
        tDelStaForSessionCmd delStaSessionCmd;
    }u;
}tSmeCmd;



/*                                                                           
                                        
                                                                          */

//                       
//                                                 
tSmeCmd *smeGetCommandBuffer( tpAniSirGlobal pMac );
void smePushCommand( tpAniSirGlobal pMac, tSmeCmd *pCmd, tANI_BOOLEAN fHighPriority );
void smeProcessPendingQueue( tpAniSirGlobal pMac );
void smeReleaseCommand(tpAniSirGlobal pMac, tSmeCmd *pCmd);
void purgeSmeSessionCmdList(tpAniSirGlobal pMac, tANI_U32 sessionId);
tANI_BOOLEAN smeCommandPending(tpAniSirGlobal pMac);
tANI_BOOLEAN pmcProcessCommand( tpAniSirGlobal pMac, tSmeCmd *pCommand );
//                                                                                   
//                                                                                             
//                           
void pmcAbortCommand( tpAniSirGlobal pMac, tSmeCmd *pCommand, tANI_BOOLEAN fStopping );
tANI_BOOLEAN qosProcessCommand( tpAniSirGlobal pMac, tSmeCmd *pCommand );

eHalStatus csrProcessScanCommand( tpAniSirGlobal pMac, tSmeCmd *pCommand );
eHalStatus csrRoamProcessCommand( tpAniSirGlobal pMac, tSmeCmd *pCommand );
void csrRoamProcessWmStatusChangeCommand( tpAniSirGlobal pMac, tSmeCmd *pCommand );
void csrReinitRoamCmd(tpAniSirGlobal pMac, tSmeCmd *pCommand); 
void csrReinitWmStatusChangeCmd(tpAniSirGlobal pMac, tSmeCmd *pCommand);
void csrReinitSetKeyCmd(tpAniSirGlobal pMac, tSmeCmd *pCommand);
void csrReinitRemoveKeyCmd(tpAniSirGlobal pMac, tSmeCmd *pCommand);
eHalStatus csrRoamProcessSetKeyCommand( tpAniSirGlobal pMac, tSmeCmd *pCommand );
eHalStatus csrRoamProcessRemoveKeyCommand( tpAniSirGlobal pMac, tSmeCmd *pCommand );
void csrReleaseCommandSetKey(tpAniSirGlobal pMac, tSmeCmd *pCommand);
void csrReleaseCommandRemoveKey(tpAniSirGlobal pMac, tSmeCmd *pCommand);
//                                                                                                                          
eHalStatus csrRoamIssueRemoveKeyCommand( tpAniSirGlobal pMac, tANI_U32 sessionId,
                                         tCsrRoamRemoveKey *pRemoveKey, tANI_U32 roamId );
eHalStatus csrIsFullPowerNeeded( tpAniSirGlobal pMac, tSmeCmd *pCommand, tRequestFullPowerReason *pReason,
                                 tANI_BOOLEAN *pfNeedPower);
void csrAbortCommand( tpAniSirGlobal pMac, tSmeCmd *pCommand, tANI_BOOLEAN fStopping );

eHalStatus sme_AcquireGlobalLock( tSmeStruct *psSme);
eHalStatus sme_ReleaseGlobalLock( tSmeStruct *psSme);

#ifdef FEATURE_OEM_DATA_SUPPORT
eHalStatus oemData_ProcessOemDataReqCommand(tpAniSirGlobal pMac, tSmeCmd *pCommand);
#endif

eHalStatus csrProcessAddStaSessionCommand( tpAniSirGlobal pMac, tSmeCmd *pCommand );
eHalStatus csrProcessAddStaSessionRsp( tpAniSirGlobal pMac, tANI_U8 *pMsg);
eHalStatus csrProcessDelStaSessionCommand( tpAniSirGlobal pMac, tSmeCmd *pCommand );
eHalStatus csrProcessDelStaSessionRsp( tpAniSirGlobal pMac, tANI_U8 *pMsg);

#ifdef WLAN_NS_OFFLOAD
/*                                                                            
                       
                                         
                                                  
                                                      
                      
                                                        
                                                   
                                                                             */
eHalStatus pmcSetNSOffload (tHalHandle hHal, tpSirHostOffloadReq pRequest, tANI_U8 *bssId);
#endif //               

#ifdef FEATURE_WLAN_SCAN_PNO
eHalStatus pmcSetPreferredNetworkList(tHalHandle hHal, tpSirPNOScanReq pRequest, tANI_U8 sessionId, preferredNetworkFoundIndCallback callbackRoutine,  void *callbackContext);
eHalStatus pmcUpdateScanParams(tHalHandle hHal, tCsrConfig *pRequest, tCsrChannel *pChannelList, tANI_U8 b11dResolved);
eHalStatus pmcSetRssiFilter(tHalHandle hHal,   v_U8_t        rssiThreshold);
#endif //                      
eHalStatus pmcSetPowerParams(tHalHandle hHal,   tSirSetPowerParamsReq*  pwParams);

tANI_BOOLEAN csrRoamGetConcurrencyConnectStatusForBmps(tpAniSirGlobal pMac);
#endif //                             
