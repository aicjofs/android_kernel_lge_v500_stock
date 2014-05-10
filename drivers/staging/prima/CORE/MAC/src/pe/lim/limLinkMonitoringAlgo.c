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

/*
                                                        
                                                           
                                                         
                   
                                  
                          
            
                                                         
                                                                       
  
 */

#include "aniGlobal.h"
#include "wniCfgAp.h"
#include "cfgApi.h"

#ifdef FEATURE_WLAN_NON_INTEGRATED_SOC
#include "halDataStruct.h"
#include "halCommonApi.h"
#endif

#include "schApi.h"
#include "pmmApi.h"
#include "utilsApi.h"
#include "limAssocUtils.h"
#include "limTypes.h"
#include "limUtils.h"
#include "limPropExtsUtils.h"

#ifdef FEATURE_WLAN_DIAG_SUPPORT_LIM //                         
#include "vos_diag_core_log.h"
#endif //                         
#include "limSession.h"
#include "limSerDesUtils.h"


/* 
                           
  
           
                                                             
  
        
  
              
  
       
     
  
                                                        
               
 */

void
limSendKeepAliveToPeer(tpAniSirGlobal pMac)
{

#ifdef ANI_PRODUCT_TYPE_AP   //              

    tpDphHashNode   pStaDs;
    //                                             
    tpPESession psessionEntry;

    if((psessionEntry = peFindSessionBySessionId(pMac, pMac->lim.limTimers.gLimKeepaliveTimer.sessionId))== NULL)
    {
        limLog(pMac, LOGP,FL("Session Does not exist for given sessionID\n"));
        return;
    }

    //                                     
    if (pMac->sch.keepAlive == 0)
        return;

    if ( (limIsSystemInScanState(pMac) == false) &&
          (psessionEntry->limSystemRole == eLIM_AP_ROLE))
    {
        tANI_U16 i;
        tANI_U32        len = SIR_MAC_MAX_SSID_LENGTH;
        tAniSSID   ssId;

        /*
                                                  
                          
        */

        for (i=2; i<pMac->lim.maxStation; i++)
        {
            pStaDs = dphGetHashEntry(pMac, i, &psessionEntry->dph.dphHashTable);

            if (pStaDs && pStaDs->added &&
                (pStaDs->mlmStaContext.mlmState == eLIM_MLM_LINK_ESTABLISHED_STATE))
            {
                //                                                              
                //                                                            
                //                                                           
                //                                       
                if (wlan_cfgGetStr(pMac, WNI_CFG_SSID,
                                (tANI_U8 *) &ssId.ssId,
                                (tANI_U32 *) &len) != eSIR_SUCCESS)
                {
                        //                                         
                    limLog(pMac, LOGP, FL("could not retrieve SSID\n"));
                }
                ssId.length = (tANI_U8) len;

                PELOG2(limLog(pMac, LOG2,  FL("Sending keepalive Probe Rsp Msg to "));
                limPrintMacAddr(pMac, pStaDs->staAddr, LOG2);)
                limSendProbeRspMgmtFrame(pMac,
                                         pStaDs->staAddr,
                                         &ssId,
                                         i,
                                         DPH_KEEPALIVE_FRAME, 0);
            }
        }
    }
    #endif
} /*                              */


/*                                                           
                            
                                                    
                                                  
                                                              
                                                              
                            
                              
             
                                                             */
void
limDeleteStaContext(tpAniSirGlobal pMac, tpSirMsgQ limMsg)
{
    tpDeleteStaContext  pMsg = (tpDeleteStaContext)limMsg->bodyptr;
    tpDphHashNode       pStaDs;
    tpPESession psessionEntry ;
    tANI_U8     sessionId;

    if((psessionEntry = peFindSessionByBssid(pMac,pMsg->bssId,&sessionId))== NULL)
    {
         PELOGE(limLog(pMac, LOGE,FL("session does not exist for given BSSId\n"));)
         palFreeMemory(pMac->hHdd, pMsg);
         return;
    }

    if (NULL != pMsg)
    {
#ifdef WLAN_SOFTAP_FEATURE
        switch(pMsg->reasonCode)
        {
            case HAL_DEL_STA_REASON_CODE_KEEP_ALIVE:
            case HAL_DEL_STA_REASON_CODE_TIM_BASED:
                PELOGE(limLog(pMac, LOGE, FL(" Deleting station: staId = %d, reasonCode = %d\n"), pMsg->staId, pMsg->reasonCode);)
#endif
                if((eLIM_BT_AMP_AP_ROLE == psessionEntry->limSystemRole) ||
                   (eLIM_AP_ROLE == psessionEntry->limSystemRole))
                {
                    pStaDs = dphGetHashEntry(pMac, pMsg->assocId, &psessionEntry->dph.dphHashTable);
                }
                else
                {
                    pStaDs = dphGetHashEntry(pMac, DPH_STA_HASH_INDEX_PEER, &psessionEntry->dph.dphHashTable);
                }
                if (! pStaDs)
                {
                   PELOGE(limLog(pMac, LOGE, FL("Skip STA deletion (invalid STA) limSystemRole=%d\n"),psessionEntry->limSystemRole);)
                   palFreeMemory(pMac->hHdd, pMsg);
                   return;
                }

                /*                                                           
                                                                     
                */
                if (pStaDs->staIndex != pMsg->staId)
                {
                    PELOGE(limLog(pMac, LOGE, FL("staid mismatch: %d vs %d \n"), pStaDs->staIndex, pMsg->staId);)
                    palFreeMemory(pMac->hHdd, pMsg);
                    return;
                }

                if((eLIM_BT_AMP_AP_ROLE == psessionEntry->limSystemRole) ||
                   (eLIM_AP_ROLE == psessionEntry->limSystemRole))
                {
                    PELOG1(limLog(pMac, LOG1, FL("SAP:lim Delete Station Context (staId: %d, assocId: %d) \n"),
                                  pMsg->staId, pMsg->assocId);)
                    limTriggerSTAdeletion(pMac, pStaDs, psessionEntry);
                }
                else
                {
                    //                    
                    tLimMlmDeauthInd  mlmDeauthInd;
                    PELOGW(limLog(pMac, LOGW, FL("lim Delete Station Context (staId: %d, assocId: %d) \n"),
                                  pMsg->staId, pMsg->assocId);)

                    pStaDs->mlmStaContext.disassocReason = eSIR_MAC_UNSPEC_FAILURE_REASON;
                    pStaDs->mlmStaContext.cleanupTrigger = eLIM_LINK_MONITORING_DEAUTH;

                    //                                
                    palCopyMemory( pMac->hHdd, (tANI_U8 *) &mlmDeauthInd.peerMacAddr,
                                   pStaDs->staAddr, sizeof(tSirMacAddr));
                    mlmDeauthInd.reasonCode    = (tANI_U8) pStaDs->mlmStaContext.disassocReason;
                    mlmDeauthInd.deauthTrigger =  pStaDs->mlmStaContext.cleanupTrigger;

                    limPostSmeMessage(pMac, LIM_MLM_DEAUTH_IND, (tANI_U32 *) &mlmDeauthInd);
 
                    limSendSmeDeauthInd(pMac, pStaDs, psessionEntry);
                }
#ifdef WLAN_SOFTAP_FEATURE
                break;        
            
            case HAL_DEL_STA_REASON_CODE_UNKNOWN_A2:
                PELOGE(limLog(pMac, LOGE, FL(" Deleting Unknown station \n"));)
                limPrintMacAddr(pMac, pMsg->addr2, LOGE);
               
                limSendDeauthMgmtFrame( pMac, eSIR_MAC_CLASS3_FRAME_FROM_NON_ASSOC_STA_REASON, pMsg->addr2, psessionEntry);
                break;

            default:
                PELOGE(limLog(pMac, LOGE, FL(" Unknown reason code \n"));)
                break;

        }
#endif
    }

    palFreeMemory(pMac->hHdd, pMsg);
    return;
}


/* 
                          
  
           
                                                          
  
        
  
              
  
       
     
  
                                                   
                                                         
               
 */
void
limTriggerSTAdeletion(tpAniSirGlobal pMac, tpDphHashNode pStaDs, tpPESession psessionEntry)
{
    tSirSmeDeauthReq    *pSmeDeauthReq;
    tANI_U8             *pBuf;
    tANI_U8             *pLen;
    tANI_U16            msgLength = 0;

    if (! pStaDs)
    {
        PELOGW(limLog(pMac, LOGW, FL("Skip STA deletion (invalid STA)\n"));)
        return;
    }
    /* 
                                                 
                                                   
                                           
     */

    if( eHAL_STATUS_SUCCESS != palAllocateMemory( pMac->hHdd, (void **)&pSmeDeauthReq, sizeof(tSirSmeDeauthReq)))
    {
        limLog(pMac, LOGP, FL("palAllocateMemory failed for eWNI_SME_DEAUTH_REQ \n"));
        return;
    }

    pBuf = (tANI_U8 *) &pSmeDeauthReq->messageType;

    //           
#ifdef WLAN_SOFTAP_FEATURE
    limCopyU16((tANI_U8*)pBuf, eWNI_SME_DISASSOC_REQ);
#else
    limCopyU16((tANI_U8*)pBuf, eWNI_SME_DEAUTH_REQ);
#endif
    pBuf += sizeof(tANI_U16);
    msgLength += sizeof(tANI_U16);

    //      
    pLen = pBuf;
    pBuf += sizeof(tANI_U16);
    msgLength += sizeof(tANI_U16);

    //         
    *pBuf = psessionEntry->smeSessionId;
    pBuf++;
    msgLength++;

    //             
    limCopyU16((tANI_U8*)pBuf, psessionEntry->transactionId);
    pBuf += sizeof(tANI_U16);
    msgLength += sizeof(tANI_U16);

    //     
    palCopyMemory( pMac->hHdd, pBuf, psessionEntry->bssId, sizeof(tSirMacAddr) );
    pBuf += sizeof(tSirMacAddr);
    msgLength += sizeof(tSirMacAddr);

    //           
    palCopyMemory( pMac->hHdd, pBuf, pStaDs->staAddr, sizeof(tSirMacAddr) );
    pBuf += sizeof(tSirMacAddr);
    msgLength += sizeof(tSirMacAddr);

    //           
#ifdef WLAN_SOFTAP_FEATURE
    limCopyU16((tANI_U8*)pBuf, (tANI_U16)eLIM_LINK_MONITORING_DISASSOC);
#else
    limCopyU16((tANI_U8*)pBuf, (tANI_U16)eLIM_LINK_MONITORING_DEAUTH);
#endif
    pBuf += sizeof(tANI_U16);
    msgLength += sizeof(tANI_U16);

    //                        
    //                                                             
    //                                                      
    pBuf[0]= 0;
    pBuf += sizeof(tANI_U8);
    msgLength += sizeof(tANI_U8);


#if (WNI_POLARIS_FW_PRODUCT == AP)
    //   
    limCopyU16((tANI_U8*)pBuf, pStaDs->assocId);
    pBuf += sizeof(tANI_U16);
    msgLength += sizeof(tANI_U16);
#endif
  
    //              
    limCopyU16((tANI_U8*)pLen, msgLength);

#ifdef WLAN_SOFTAP_FEATURE
    limPostSmeMessage(pMac, eWNI_SME_DISASSOC_REQ, (tANI_U32 *) pSmeDeauthReq);
#else
    limPostSmeMessage(pMac, eWNI_SME_DEAUTH_REQ, (tANI_U32 *) pSmeDeauthReq);
#endif
    palFreeMemory( pMac->hHdd, pSmeDeauthReq );

} /*                                 */



/* 
                          
  
           
                                                            
               
  
        
  
              
  
       
  
                                                 
               
 */

void
limTearDownLinkWithAp(tpAniSirGlobal pMac, tANI_U8 sessionId, tSirMacReasonCodes reasonCode)
{
    tpDphHashNode pStaDs = NULL;

    //                                    
    tpPESession psessionEntry;

    if((psessionEntry = peFindSessionBySessionId(pMac, sessionId))== NULL)
    {
        limLog(pMac, LOGP,FL("Session Does not exist for given sessionID\n"));
        return;
    }
    /* 
                                                 
                                                
                              
     */

    pMac->pmm.inMissedBeaconScenario = FALSE;
    limLog(pMac, LOGW,
       FL("No ProbeRsp from AP after HB failure. Tearing down link\n"));

    //                           
    limHeartBeatDeactivateAndChangeTimer(pMac, psessionEntry);

    //                                           
    //                                               

    pStaDs = dphGetHashEntry(pMac, DPH_STA_HASH_INDEX_PEER, &psessionEntry->dph.dphHashTable);

    
    if (pStaDs != NULL)
    {
        tLimMlmDeauthInd  mlmDeauthInd;

        pStaDs->mlmStaContext.disassocReason = reasonCode;
        pStaDs->mlmStaContext.cleanupTrigger = eLIM_LINK_MONITORING_DEAUTH;

        //                                 
        palCopyMemory( pMac->hHdd, (tANI_U8 *) &mlmDeauthInd.peerMacAddr,
                      pStaDs->staAddr,
                      sizeof(tSirMacAddr));
        mlmDeauthInd.reasonCode    = (tANI_U8) pStaDs->mlmStaContext.disassocReason;
        mlmDeauthInd.deauthTrigger =  pStaDs->mlmStaContext.cleanupTrigger;

        limPostSmeMessage(pMac, LIM_MLM_DEAUTH_IND, (tANI_U32 *) &mlmDeauthInd);

        limSendSmeDeauthInd(pMac, pStaDs, psessionEntry);
    }    
} /*                             */




/* 
                              
  
           
                                                            
               
  
        
  
              
  
       
  
                                                 
               
 */

void limHandleHeartBeatFailure(tpAniSirGlobal pMac,tpPESession psessionEntry)
{

#ifdef FEATURE_WLAN_DIAG_SUPPORT_LIM //                         
    vos_log_beacon_update_pkt_type *log_ptr = NULL;
#endif //                          

    /*                                                                                    
                                                                                                   
                                                                                             
                                                                                         
                                                   
     */

    if(!limIsSystemInActiveState(pMac))
        return;

#ifdef FEATURE_WLAN_DIAG_SUPPORT_LIM //                         
    WLAN_VOS_DIAG_LOG_ALLOC(log_ptr, vos_log_beacon_update_pkt_type, LOG_WLAN_BEACON_UPDATE_C);
    if(log_ptr)
        log_ptr->bcn_rx_cnt = psessionEntry->LimRxedBeaconCntDuringHB;
    WLAN_VOS_DIAG_LOG_REPORT(log_ptr);
#endif //                         

    /*                                                   */
    psessionEntry->LimHBFailureStatus = eANI_BOOLEAN_FALSE;
    /*                                                                 */
    if(((psessionEntry->limSystemRole == eLIM_STA_IN_IBSS_ROLE) || 
        (psessionEntry->limSystemRole == eLIM_STA_ROLE) ||
        (psessionEntry->limSystemRole == eLIM_BT_AMP_STA_ROLE)) && 
       (LIM_IS_CONNECTION_ACTIVE(psessionEntry) ||
        (limIsReassocInProgress(pMac, psessionEntry))))
    {
        if(psessionEntry->LimRxedBeaconCntDuringHB < MAX_NO_BEACONS_PER_HEART_BEAT_INTERVAL)
            pMac->lim.gLimHeartBeatBeaconStats[psessionEntry->LimRxedBeaconCntDuringHB]++;
        else
            pMac->lim.gLimHeartBeatBeaconStats[0]++;

        /*      
                                                            
                                                  
                                      
           
              */
        //                                                               
        limReactivateHeartBeatTimer(pMac, psessionEntry);

        //                                 
        limResetHBPktCount(psessionEntry);
        return;
    }
    if (((psessionEntry->limSystemRole == eLIM_STA_ROLE)||(psessionEntry->limSystemRole == eLIM_BT_AMP_STA_ROLE))&&
         (psessionEntry->limMlmState == eLIM_MLM_LINK_ESTABLISHED_STATE))
    {
        if (!pMac->sys.gSysEnableLinkMonitorMode)
            return;

        /* 
                                                              
         */
        PELOGW(limLog(pMac, LOGW, FL("Heartbeat Failure\n"));)
        pMac->lim.gLimHBfailureCntInLinkEstState++;

        /* 
                                                                     
                                                                                    
         */
        if(!limIsconnectedOnDFSChannel(psessionEntry->currentOperChannel))
        {
            /*                                       */
             psessionEntry->LimHBFailureStatus= eANI_BOOLEAN_TRUE;
            /* 
                                                       
                                                     
                                                  
             */
            PELOGW(limLog(pMac, LOGW, FL("Heart Beat missed from AP. Sending Probe Req\n"));)
            /*                                                      */
            limSendProbeReqMgmtFrame(pMac, &psessionEntry->ssId, psessionEntry->bssId,
                                      psessionEntry->currentOperChannel,psessionEntry->selfMacAddr,
                                      psessionEntry->dot11mode, 0, NULL);
        }
        else
        {
            /*                                                              
                                          */
            limTearDownLinkWithAp(pMac, psessionEntry->peSessionId, eSIR_MAC_UNSPEC_FAILURE_REASON);
        }
    }
    else
    {
        /* 
                                                 
                                                            
                                                             
                        
            */
        PELOG1(limLog(pMac, LOG1, FL("received heartbeat timeout in state %X\n"),
               psessionEntry->limMlmState);)
        limPrintMlmState(pMac, LOG1, psessionEntry->limMlmState);
        pMac->lim.gLimHBfailureCntInOtherStates++;
        limReactivateHeartBeatTimer(pMac, psessionEntry);
    }
} /*                                 */
