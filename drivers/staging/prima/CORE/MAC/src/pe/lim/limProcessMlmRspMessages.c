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
#include "wniApi.h"
#ifdef ANI_PRODUCT_TYPE_AP
#include "wniCfgAp.h"
#else
#include "wniCfgSta.h"
#endif
#include "cfgApi.h"
#include "sirApi.h"
#include "schApi.h"
#include "utilsApi.h"
#include "limUtils.h"
#include "limAssocUtils.h"
#include "limSecurityUtils.h"
#include "limSerDesUtils.h"
#include "limTimerUtils.h"
#include "limSendMessages.h"
#include "limAdmitControl.h"
#include "limSendMessages.h"
#include "limIbssPeerMgmt.h"
#include "limSession.h"
#include "limSessionUtils.h"
#if defined WLAN_FEATURE_VOWIFI
#include "rrmApi.h"
#endif
#if defined WLAN_FEATURE_VOWIFI_11R
#include <limFT.h>
#endif
#include "wlan_qct_wda.h"

static void limHandleSmeJoinResult(tpAniSirGlobal, tSirResultCodes, tANI_U16,tpPESession);
static void limHandleSmeReaasocResult(tpAniSirGlobal, tSirResultCodes, tANI_U16, tpPESession);
void limProcessMlmScanCnf(tpAniSirGlobal, tANI_U32 *);
#ifdef FEATURE_OEM_DATA_SUPPORT
void limProcessMlmOemDataReqCnf(tpAniSirGlobal, tANI_U32 *);
#endif
void limProcessMlmJoinCnf(tpAniSirGlobal, tANI_U32 *);
void limProcessMlmAuthCnf(tpAniSirGlobal, tANI_U32 *);
void limProcessMlmStartCnf(tpAniSirGlobal, tANI_U32 *);
void limProcessMlmAuthInd(tpAniSirGlobal, tANI_U32 *);
void limProcessMlmAssocInd(tpAniSirGlobal, tANI_U32 *);
void limProcessMlmAssocCnf(tpAniSirGlobal, tANI_U32 *);
void limProcessMlmReassocCnf(tpAniSirGlobal, tANI_U32 *);
void limProcessMlmReassocInd(tpAniSirGlobal, tANI_U32 *);
void limProcessMlmSetKeysCnf(tpAniSirGlobal, tANI_U32 *);
void limProcessMlmDisassocInd(tpAniSirGlobal, tANI_U32 *);
void limProcessMlmDisassocCnf(tpAniSirGlobal, tANI_U32 *);
void limProcessMlmDeauthInd(tpAniSirGlobal, tANI_U32 *);
void limProcessMlmDeauthCnf(tpAniSirGlobal, tANI_U32 *);
void limProcessMlmPurgeStaInd(tpAniSirGlobal, tANI_U32 *);
void limProcessMlmAddBACnf(tpAniSirGlobal, tANI_U32 *);
void limProcessMlmDelBACnf(tpAniSirGlobal, tANI_U32 *);
void limProcessMlmRemoveKeyCnf(tpAniSirGlobal pMac, tANI_U32 * pMsgBuf);
static void  limHandleDelBssInReAssocContext(tpAniSirGlobal pMac, tpDphHashNode pStaDs,tpPESession psessionEntry);
void limGetSessionInfo(tpAniSirGlobal pMac, tANI_U8 *, tANI_U8 *, tANI_U16 *);
static void
limProcessBtampAddBssRsp( tpAniSirGlobal pMac, tpSirMsgQ limMsgQ ,tpPESession psessionEntry);
/* 
                             
  
           
                                                                     
                                   
  
        
  
              
  
       
  
                                                    
                                                   
                                                        
  
               
 */
void
limProcessMlmRspMessages(tpAniSirGlobal pMac, tANI_U32 msgType, tANI_U32 *pMsgBuf)
{

   if(pMsgBuf == NULL)
    {
           PELOGE(limLog(pMac, LOGE,FL("Buffer is Pointing to NULL\n"));)
           return;
    }
    switch (msgType)
    {
        case LIM_MLM_SCAN_CNF:
            limProcessMlmScanCnf(pMac, pMsgBuf);
            break;

#ifdef FEATURE_OEM_DATA_SUPPORT
        case LIM_MLM_OEM_DATA_CNF:
            limProcessMlmOemDataReqCnf(pMac, pMsgBuf);
            break;
#endif

        case LIM_MLM_AUTH_CNF:
            limProcessMlmAuthCnf(pMac, pMsgBuf);
            break;
        case LIM_MLM_AUTH_IND:
            limProcessMlmAuthInd(pMac, pMsgBuf);
            break;
        case LIM_MLM_ASSOC_CNF:
            limProcessMlmAssocCnf(pMac, pMsgBuf);
            break;
        case LIM_MLM_START_CNF:
            limProcessMlmStartCnf(pMac, pMsgBuf);
            break;
        case LIM_MLM_JOIN_CNF:
            limProcessMlmJoinCnf(pMac, pMsgBuf);
            break;
        case LIM_MLM_ASSOC_IND:
             limProcessMlmAssocInd(pMac, pMsgBuf);
            break;
        case LIM_MLM_REASSOC_CNF:
            limProcessMlmReassocCnf(pMac, pMsgBuf);
            break;
        case LIM_MLM_REASSOC_IND:
           limProcessMlmReassocInd(pMac, pMsgBuf);
           break;
        case LIM_MLM_DISASSOC_CNF:
            limProcessMlmDisassocCnf(pMac, pMsgBuf);
            break;
        case LIM_MLM_DISASSOC_IND:
            limProcessMlmDisassocInd(pMac, pMsgBuf);
            break;
        case LIM_MLM_PURGE_STA_IND:
            limProcessMlmPurgeStaInd(pMac, pMsgBuf);
            break;
        case LIM_MLM_DEAUTH_CNF:
            limProcessMlmDeauthCnf(pMac, pMsgBuf);
            break;
        case LIM_MLM_DEAUTH_IND:
            limProcessMlmDeauthInd(pMac, pMsgBuf);
            break;
        case LIM_MLM_SETKEYS_CNF:
            limProcessMlmSetKeysCnf(pMac, pMsgBuf);
            break;
        case LIM_MLM_REMOVEKEY_CNF:
            limProcessMlmRemoveKeyCnf(pMac, pMsgBuf);
            break;
        case LIM_MLM_TSPEC_CNF:
            break;
        case LIM_MLM_ADDBA_CNF:
            limProcessMlmAddBACnf( pMac, pMsgBuf );
            break;
        case LIM_MLM_DELBA_CNF:
            limProcessMlmDelBACnf( pMac, pMsgBuf );
            break;
        default:
            break;
    } //                 
    return;
} /*                                    */

/* 
                         
  
           
                                                    
                                  
  
        
  
              
  
       
  
                                                    
                                                        
  
               
 */
void
limProcessMlmScanCnf(tpAniSirGlobal pMac, tANI_U32 *pMsgBuf)
{
    switch(pMac->lim.gLimSmeState)
    {
        case eLIM_SME_WT_SCAN_STATE:
        //                                                          
        //                                                              
            pMac->lim.gLimSmeState = pMac->lim.gLimPrevSmeState;
            MTRACE(macTrace(pMac, TRACE_CODE_SME_STATE, NO_SESSION, pMac->lim.gLimSmeState));
            pMac->lim.gLimSystemInScanLearnMode = 0;
            break;
        default:
            /* 
                                                    
                                        
                        
             */
            PELOGE(limLog(pMac, LOGE,
               FL("received unexpected MLM_SCAN_CNF in state %X\n"),
               pMac->lim.gLimSmeState);)
            return;
    }

    //                               
    //                                         
    pMac->lim.gLimSmeScanResultLength +=
                    ((tLimMlmScanCnf *) pMsgBuf)->scanResultLength;
    if ((pMac->lim.gLimRspReqd) || pMac->lim.gLimReportBackgroundScanResults)
    {
        tANI_U16    scanRspLen = 0;
        //                               
        pMac->lim.gLimRspReqd = false;
        if ((((tLimMlmScanCnf *) pMsgBuf)->resultCode ==
                                                eSIR_SME_SUCCESS) ||
            pMac->lim.gLimSmeScanResultLength)
        {
                    scanRspLen = sizeof(tSirSmeScanRsp) +
                                 pMac->lim.gLimSmeScanResultLength -
                                 sizeof(tSirBssDescription);
        }
        else
        {
            scanRspLen = sizeof(tSirSmeScanRsp);
        }
       if(pMac->lim.gLimReportBackgroundScanResults)
        {
            pMac->lim.gLimBackgroundScanTerminate = TRUE;
        }
        if (pMac->lim.gLimSmeScanResultLength == 0)
        {
            limSendSmeScanRsp(pMac, scanRspLen, eSIR_SME_SUCCESS, pMac->lim.gSmeSessionId, pMac->lim.gTransactionId);
        }
        else
        {
            limSendSmeScanRsp(pMac, scanRspLen,
                              eSIR_SME_SUCCESS,pMac->lim.gSmeSessionId, pMac->lim.gTransactionId);
        }
    } //                           
    //                                                
    if(pMac->lim.gLimBackgroundScanTerminate == FALSE)
    {
        if (tx_timer_activate(
            &pMac->lim.limTimers.gLimBackgroundScanTimer) != TX_SUCCESS)
        {
            //                                           
            //          
            limLog(pMac, LOGP,
            FL("could not activate background scan timer\n"));
            pMac->lim.gLimBackgroundScanStarted = FALSE;
        }
        else
        {
            pMac->lim.gLimBackgroundScanStarted = TRUE;
        }
    }
} /*                                */

#ifdef FEATURE_OEM_DATA_SUPPORT

/* 
                               
  
           
                                                                
                                  
  
        
  
              
  
       
  
                                                    
                                                        
  
               
 */

void limProcessMlmOemDataReqCnf(tpAniSirGlobal pMac, tANI_U32 *pMsgBuf)
{
    tLimMlmOemDataRsp*    measRsp;

    tSirResultCodes resultCode = eSIR_SME_SUCCESS;

    measRsp = (tLimMlmOemDataRsp*)(pMsgBuf);

    //                                            
    limSendSmeOemDataRsp(pMac, (tANI_U32*)measRsp, resultCode);

    //                                                            

    return;
}
#endif

/* 
                          
  
           
                                                     
                                  
  
        
  
              
  
       
  
                                                    
                                                        
  
               
 */
void
limProcessMlmStartCnf(tpAniSirGlobal pMac, tANI_U32 *pMsgBuf)
{
    tpPESession         psessionEntry = NULL;
    tLimMlmStartCnf     *pLimMlmStartCnf;
    tANI_U8             smesessionId;
    tANI_U16            smetransactionId;

    if(pMsgBuf == NULL)
    {
           PELOGE(limLog(pMac, LOGE,FL("Buffer is Pointing to NULL\n"));)
           return;
    }
    pLimMlmStartCnf = (tLimMlmStartCnf*)pMsgBuf;
    if((psessionEntry = peFindSessionBySessionId(pMac,pLimMlmStartCnf->sessionId))==NULL)
    {
        PELOGE(limLog(pMac, LOGE,FL("Session does Not exist with given sessionId \n"));)
        return;
    }
   smesessionId = psessionEntry->smeSessionId;
   smetransactionId = psessionEntry->transactionId;

    if (psessionEntry->limSmeState != eLIM_SME_WT_START_BSS_STATE)
    {
        /* 
                                                          
                           
                    
         */
        PELOGE(limLog(pMac, LOGE,
           FL("received unexpected MLM_START_CNF in state %X\n"),
           psessionEntry->limSmeState);)
        return;
    }
    if (((tLimMlmStartCnf *) pMsgBuf)->resultCode ==
                                            eSIR_SME_SUCCESS)
    {

        /* 
                                                            
                                                         
                                
         */
        psessionEntry->limSmeState = eLIM_SME_NORMAL_STATE;
        MTRACE(macTrace(pMac, TRACE_CODE_SME_STATE, psessionEntry->peSessionId, psessionEntry->limSmeState));
        if(psessionEntry->bssType == eSIR_BTAMP_STA_MODE)
        {
             limLog(pMac, LOG1, FL("*** Started BSS in BT_AMP STA SIDE***\n"));
        }
        else if(psessionEntry->bssType == eSIR_BTAMP_AP_MODE)
        {
             limLog(pMac, LOG1, FL("*** Started BSS in BT_AMP AP SIDE***\n"));
        }
#ifdef WLAN_SOFTAP_FEATURE
        else if(psessionEntry->bssType == eSIR_INFRA_AP_MODE)
        {
             limLog(pMac, LOG1, FL("*** Started BSS in INFRA AP SIDE***\n"));
        }
#endif
        else
            PELOG1(limLog(pMac, LOG1, FL("*** Started BSS ***\n"));)
    }
    else
    {
        //                        
        peDeleteSession(pMac,psessionEntry);
        psessionEntry = NULL;
    }
    //                       
    limSendSmeStartBssRsp(pMac, eWNI_SME_START_BSS_RSP,
                          ((tLimMlmStartCnf *) pMsgBuf)->resultCode,psessionEntry,
                          smesessionId,smetransactionId);
#ifdef WLAN_SOFTAP_FEATURE
    if (((tLimMlmStartCnf *) pMsgBuf)->resultCode == eSIR_SME_SUCCESS)
    {
        //                                        
        limSendBeaconInd(pMac, psessionEntry);
    }
#endif
}

 /*                                 */

/* 
                         
  
           
                                                    
                                  
  
        
  
              
  
       
  
                                                    
                                                        
  
               
 */
void
limProcessMlmJoinCnf(tpAniSirGlobal pMac, tANI_U32 *pMsgBuf)
{
    tSirResultCodes resultCode;
    tLimMlmJoinCnf      *pLimMlmJoinCnf;
    tpPESession         psessionEntry;
     pLimMlmJoinCnf = (tLimMlmJoinCnf*)pMsgBuf;
    if( (psessionEntry = peFindSessionBySessionId(pMac,pLimMlmJoinCnf->sessionId))== NULL)
    {
        PELOGE(limLog(pMac, LOGE,FL("Session does not exist for given sessionId\n"));)
        return;
    }

    if (psessionEntry->limSmeState!= eLIM_SME_WT_JOIN_STATE)
    {
        PELOGE(limLog(pMac, LOGE,
               FL("received unexpected MLM_JOIN_CNF in state %X\n"),
               psessionEntry->limSmeState);)
         return;
    }

    resultCode = ((tLimMlmJoinCnf *) pMsgBuf)->resultCode ;
    //                               
    if (resultCode ==  eSIR_SME_SUCCESS)
    {
            PELOG1(limLog(pMac, LOG1, FL("*** Joined ESS ***"));)
            //                      
           //                                                               
            if(limStaSendAddBssPreAssoc( pMac, false, psessionEntry) == eSIR_SUCCESS)
                return;
            else
                resultCode = eSIR_SME_REFUSED;
    }
    {
        //              
        psessionEntry->limSmeState = eLIM_SME_JOIN_FAILURE_STATE;
        MTRACE(macTrace(pMac, TRACE_CODE_SME_STATE, psessionEntry->peSessionId, psessionEntry->limSmeState));
        //                            
        limHandleSmeJoinResult(pMac, resultCode, ((tLimMlmJoinCnf *) pMsgBuf)->protStatusCode, psessionEntry );
    }
} /*                                */

/* 
                         
  
           
                                                    
                                  
  
        
  
              
  
       
  
                                                    
                                                        
  
               
 */
void
limProcessMlmAuthCnf(tpAniSirGlobal pMac, tANI_U32 *pMsgBuf)
{
    tANI_U16                caps;
    tANI_U32                val;
    tAniAuthType       cfgAuthType, authMode;
    tLimMlmAuthReq     *pMlmAuthReq;
    tLimMlmAssocReq    *pMlmAssocReq;
    tLimMlmAuthCnf     *pMlmAuthCnf;
    tpPESession     psessionEntry;
    tANI_U32        teleBcnEn = 0;
//                              

    if(pMsgBuf == NULL)
    {
           PELOGE(limLog(pMac, LOGE,FL("Buffer is Pointing to NULL\n"));)
           return;
    }
    pMlmAuthCnf = (tLimMlmAuthCnf*)pMsgBuf;
    if((psessionEntry = peFindSessionBySessionId(pMac,pMlmAuthCnf->sessionId))== NULL)
    {
        PELOGE(limLog(pMac, LOGE, FL("session does not exist for given sessionId\n"));)
        return;
    }

    if (((psessionEntry->limSmeState != eLIM_SME_WT_AUTH_STATE) &&
         (psessionEntry->limSmeState != eLIM_SME_WT_PRE_AUTH_STATE)) ||
        (psessionEntry->limSystemRole == eLIM_AP_ROLE)|| (psessionEntry->limSystemRole == eLIM_BT_AMP_AP_ROLE))
    {
        /* 
                                                
                                             
                    
         */
        PELOGE(limLog(pMac, LOGE,
               FL("received unexpected MLM_AUTH_CNF in state %X\n"),
               psessionEntry->limSmeState);)
        return;
    }
    //                               
    if (((tLimMlmAuthCnf *) pMsgBuf)->resultCode != eSIR_SME_SUCCESS)
    {
        if (psessionEntry->limSmeState == eLIM_SME_WT_AUTH_STATE)
                {
            if (wlan_cfgGetInt(pMac, WNI_CFG_AUTHENTICATION_TYPE,
                          (tANI_U32 *) &cfgAuthType) != eSIR_SUCCESS)
            {
                /* 
                                                         
                             
                 */
                limLog(pMac, LOGP,
                       FL("could not retrieve AuthType value\n"));
            }
                }
        else
            cfgAuthType = pMac->lim.gLimPreAuthType;
        
        if ((cfgAuthType == eSIR_AUTO_SWITCH) &&
                (((tLimMlmAuthCnf *) pMsgBuf)->authType == eSIR_OPEN_SYSTEM)
                && (eSIR_MAC_AUTH_ALGO_NOT_SUPPORTED_STATUS == ((tLimMlmAuthCnf *) pMsgBuf)->protStatusCode))
        {
            /* 
                                                                       
                                                                            
             */
            authMode = eSIR_SHARED_KEY;
            //                                 
            if( eHAL_STATUS_SUCCESS != palAllocateMemory( pMac->hHdd, (void **)&pMlmAuthReq, sizeof(tLimMlmAuthReq)))
            {
                //          
                limLog(pMac, LOGP,
                   FL("call to palAllocateMemory failed for mlmAuthReq\n"));
                return;
            }
            palZeroMemory( pMac->hHdd, (tANI_U8 *) pMlmAuthReq, sizeof(tLimMlmAuthReq));
            val = sizeof(tSirMacAddr);
            if (psessionEntry->limSmeState == eLIM_SME_WT_AUTH_STATE)
            {
                sirCopyMacAddr(pMlmAuthReq->peerMacAddr,psessionEntry->bssId);
            }
            else
                palCopyMemory( pMac->hHdd, (tANI_U8 *) &pMlmAuthReq->peerMacAddr,
                              (tANI_U8 *) &pMac->lim.gLimPreAuthPeerAddr,
                              sizeof(tSirMacAddr));
            pMlmAuthReq->authType = authMode;
            /*                     */
            pMlmAuthReq->sessionId = pMlmAuthCnf->sessionId;
            if (wlan_cfgGetInt(pMac, WNI_CFG_AUTHENTICATE_FAILURE_TIMEOUT,
                          (tANI_U32 *) &pMlmAuthReq->authFailureTimeout)
                            != eSIR_SUCCESS)
            {
                /* 
                                                                   
                             
                 */
                limLog(pMac, LOGP,
                   FL("could not retrieve AuthFailureTimeout value\n"));
            }
            limPostMlmMessage(pMac,
                              LIM_MLM_AUTH_REQ,
                              (tANI_U32 *) pMlmAuthReq);
            return;
        }
        else
        {
            //                                 
            if (psessionEntry->limSmeState == eLIM_SME_WT_AUTH_STATE)
            {
                PELOGE(limLog(pMac, LOGE, FL("Auth Failure occurred.\n"));)
                psessionEntry->limSmeState = eLIM_SME_JOIN_FAILURE_STATE;
                MTRACE(macTrace(pMac, TRACE_CODE_SME_STATE, psessionEntry->peSessionId, psessionEntry->limSmeState));
                psessionEntry->limMlmState = eLIM_MLM_IDLE_STATE;
                MTRACE(macTrace(pMac, TRACE_CODE_MLM_STATE, psessionEntry->peSessionId, psessionEntry->limMlmState));

#if defined(ANI_AP_CLIENT_SDK)
                if (psessionEntry->limSystemRole == eLIM_STA_ROLE)
                {
                    tSirMacAddr nullMacAddr = {0, 0, 0, 0, 0, 0};
                    PELOGE(limLog(pMac, LOGE, FL("Setting current BSSID as NULL in cfg\n"));)
                    palCopyMemory(pMac->hHdd, pMac->lim.gLimBssid, nullMacAddr, sizeof(tSirMacAddr));
                    if (cfgSetStr(pMac, WNI_CFG_BSSID, nullMacAddr, sizeof(tSirMacAddr)) != eSIR_SUCCESS)
                    {
                        limLog(pMac, LOGP, FL("Could not update BSSID on CFG"));
                    }
                }
#endif
                /* 
                                                  
                                        
                 */
                limHandleSmeJoinResult(pMac,
                              ((tLimMlmAuthCnf *) pMsgBuf)->resultCode, ((tLimMlmAuthCnf *) pMsgBuf)->protStatusCode,psessionEntry);
            }
            else
            {
                /* 
                                              
                                                         
                 */
                psessionEntry->limSmeState = psessionEntry->limPrevSmeState;
                MTRACE(macTrace(pMac, TRACE_CODE_SME_STATE, psessionEntry->peSessionId, psessionEntry->limSmeState));
                limSendSmeAuthRsp(
                            pMac,
                            ((tLimMlmAuthCnf *) pMsgBuf)->resultCode,
                            ((tLimMlmAuthCnf *) pMsgBuf)->peerMacAddr,
                            ((tLimMlmAuthCnf *) pMsgBuf)->authType,
                            ((tLimMlmAuthCnf *) pMsgBuf)->protStatusCode,psessionEntry,psessionEntry->smeSessionId,psessionEntry->transactionId);
            }
        } //                                     
    } //                                                    
    else
    {
        if (psessionEntry->limSmeState == eLIM_SME_WT_AUTH_STATE)
        {
            /* 
                                                  
                                           
             */
            PELOG1(limLog(pMac, LOG1,
                   FL("*** Authenticated with BSS ***\n"));)
            if( eHAL_STATUS_SUCCESS != palAllocateMemory( pMac->hHdd, (void **)&pMlmAssocReq, sizeof(tLimMlmAssocReq)))
            {
                //          
                limLog(pMac, LOGP,
                   FL("call to palAllocateMemory failed for mlmAssocReq\n"));
                return;
            }
            val = sizeof(tSirMacAddr);
            #if 0
            if (cfgGetStr(pMac, WNI_CFG_BSSID,
                          pMlmAssocReq->peerMacAddr,
                          &val) != eSIR_SUCCESS)
            {
                //                                          
                limLog(pMac, LOGP, FL("could not retrieve BSSID\n"));
            }
            #endif //              
            sirCopyMacAddr(pMlmAssocReq->peerMacAddr,psessionEntry->bssId);
            if (wlan_cfgGetInt(pMac, WNI_CFG_ASSOCIATION_FAILURE_TIMEOUT,
                          (tANI_U32 *) &pMlmAssocReq->assocFailureTimeout)
                           != eSIR_SUCCESS)
            {
                /* 
                                                          
                                       
                 */
                limLog(pMac, LOGP,
                   FL("could not retrieve AssocFailureTimeout value\n"));
            }
            if (cfgGetCapabilityInfo(pMac, &caps,psessionEntry) != eSIR_SUCCESS)
            {
                /* 
                                                   
                                       
                 */
                limLog(pMac, LOGP,
                       FL("could not retrieve Capabilities value\n"));
            }
            /*                                                      */
            if(!(psessionEntry->pLimJoinReq->bssDescription.capabilityInfo & LIM_SPECTRUM_MANAGEMENT_BIT_MASK))
            {
                /*                                                                    */
                caps &= (~LIM_SPECTRUM_MANAGEMENT_BIT_MASK);
            }

            pMlmAssocReq->capabilityInfo = caps;
           PELOG3(limLog(pMac, LOG3,
               FL("Capabilities to be used in AssocReq=0x%X, privacy bit=%x shortSlotTime %x\n"),
               caps,
               ((tpSirMacCapabilityInfo) &pMlmAssocReq->capabilityInfo)->privacy,
               ((tpSirMacCapabilityInfo) &pMlmAssocReq->capabilityInfo)->shortSlotTime);)

           /*                                                           
                                      */
            if(wlan_cfgGetInt(pMac, WNI_CFG_TELE_BCN_WAKEUP_EN, &teleBcnEn) !=
               eSIR_SUCCESS)
               limLog(pMac, LOGP, FL("Couldn't get WNI_CFG_TELE_BCN_WAKEUP_EN\n"));

            val = WNI_CFG_LISTEN_INTERVAL_STADEF;

            if(teleBcnEn)
            {
               if(wlan_cfgGetInt(pMac, WNI_CFG_TELE_BCN_MAX_LI, &val) !=
                  eSIR_SUCCESS)
               {
                   /* 
                                                      
                                        
                  */
                  limLog(pMac, LOGP, FL("could not retrieve ListenInterval\n"));
               }
            }
            else
            {
            if (wlan_cfgGetInt(pMac, WNI_CFG_LISTEN_INTERVAL, &val) != eSIR_SUCCESS)
            {
                /* 
                                                     
                                       
                 */
                  limLog(pMac, LOGP, FL("could not retrieve ListenInterval\n"));
               }
            }

            pMlmAssocReq->listenInterval = (tANI_U16)val;
            /*                     */
            pMlmAssocReq->sessionId = psessionEntry->peSessionId;
            psessionEntry->limPrevSmeState = psessionEntry->limSmeState;
            psessionEntry->limSmeState     = eLIM_SME_WT_ASSOC_STATE;
            MTRACE(macTrace(pMac, TRACE_CODE_SME_STATE, psessionEntry->peSessionId, psessionEntry->limSmeState));
            limPostMlmMessage(pMac,
                              LIM_MLM_ASSOC_REQ,
                              (tANI_U32 *) pMlmAssocReq);
        }
        else
        {
            /* 
                                             
                                             
             */
            psessionEntry->limSmeState = psessionEntry->limPrevSmeState;
            MTRACE(macTrace(pMac, TRACE_CODE_SME_STATE, psessionEntry->peSessionId, psessionEntry->limSmeState));
            limSendSmeAuthRsp(
                        pMac,
                        ((tLimMlmAuthCnf *) pMsgBuf)->resultCode,
                        ((tLimMlmAuthCnf *) pMsgBuf)->peerMacAddr,
                        ((tLimMlmAuthCnf *) pMsgBuf)->authType,
                        ((tLimMlmAuthCnf *) pMsgBuf)->protStatusCode,psessionEntry,psessionEntry->smeSessionId,psessionEntry->transactionId);
        }
    } //                                                        
} /*                                */

/* 
                          
  
           
                                                     
                                  
  
        
  
              
  
       
  
                                                    
                                                        
  
               
 */
void
limProcessMlmAssocCnf(tpAniSirGlobal pMac, tANI_U32 *pMsgBuf)
{
    tpPESession     psessionEntry;
    tLimMlmAssocCnf *pLimMlmAssocCnf;

    if(pMsgBuf == NULL)
    {
           limLog(pMac, LOGE,FL("Buffer is Pointing to NULL\n"));
           return;
    }
    pLimMlmAssocCnf = (tLimMlmAssocCnf*)pMsgBuf;
    if((psessionEntry = peFindSessionBySessionId(pMac,pLimMlmAssocCnf->sessionId)) == NULL)
    {
         PELOGE(limLog(pMac, LOGE,FL("Session does not exist for given sessionId\n"));)
         return;
    }
    if (psessionEntry->limSmeState != eLIM_SME_WT_ASSOC_STATE ||
        psessionEntry->limSystemRole == eLIM_AP_ROLE || psessionEntry ->limSystemRole == eLIM_BT_AMP_AP_ROLE)
    {
        /* 
                                                        
                                             
                    
         */
        PELOGE(limLog(pMac, LOGE,
               FL("received unexpected MLM_ASSOC_CNF in state %X\n"),
               psessionEntry->limSmeState);)
        return;
    }
    if (((tLimMlmAssocCnf *) pMsgBuf)->resultCode != eSIR_SME_SUCCESS)
    {
        //                    
        PELOG1(limLog(pMac, LOG1, FL("*** Association failure ***\n"));)
        psessionEntry->limSmeState = eLIM_SME_JOIN_FAILURE_STATE;
        MTRACE(macTrace(pMac, TRACE_CODE_SME_STATE, psessionEntry->peSessionId, pMac->lim.gLimSmeState));
#if defined(ANI_AP_CLIENT_SDK)
        if (psessionEntry->limSystemRole == eLIM_STA_ROLE)
        {
            tSirMacAddr nullMacAddr = {0, 0, 0, 0, 0, 0};
            palCopyMemory(pMac->hHdd, pMac->lim.gLimBssid, nullMacAddr, sizeof(tSirMacAddr));
            if (cfgSetStr(pMac, WNI_CFG_BSSID, nullMacAddr, sizeof(tSirMacAddr)) != eSIR_SUCCESS)
            {
                limLog(pMac, LOGP, FL("Could not update BSSID on CFG"));
            }
        }
#endif
        /* 
                                          
                                       
         */
        limHandleSmeJoinResult(pMac,
                            ((tLimMlmAssocCnf *) pMsgBuf)->resultCode,
                            ((tLimMlmAssocCnf *) pMsgBuf)->protStatusCode,psessionEntry);
    } //                                                     
    else
    {
        //                       
        PELOG1(limLog(pMac, LOG1, FL("*** Associated with BSS ***\n"));)
        psessionEntry->limSmeState = eLIM_SME_LINK_EST_STATE;
        MTRACE(macTrace(pMac, TRACE_CODE_SME_STATE, psessionEntry->peSessionId, psessionEntry->limSmeState));
        /* 
                                          
                                       
         */
        limHandleSmeJoinResult(pMac,
                            ((tLimMlmAssocCnf *) pMsgBuf)->resultCode,
                            ((tLimMlmAssocCnf *) pMsgBuf)->protStatusCode,psessionEntry);
    } //                                                          
} /*                                 */

/* 
                            
  
           
                                                       
                                  
  
        
  
              
  
       
  
                                                    
                                                        
  
               
 */
void
limProcessMlmReassocCnf(tpAniSirGlobal pMac, tANI_U32 *pMsgBuf)
{
    tpPESession psessionEntry;
    tLimMlmReassocCnf *pLimMlmReassocCnf;

    if(pMsgBuf == NULL)
    {
           PELOGE(limLog(pMac, LOGE,FL("Buffer is Pointing to NULL\n"));)
           return;
    }
    pLimMlmReassocCnf = (tLimMlmReassocCnf*) pMsgBuf;
    if((psessionEntry = peFindSessionBySessionId(pMac,pLimMlmReassocCnf->sessionId))==NULL)
    {
         PELOGE(limLog(pMac, LOGE, FL("session Does not exist for given session Id\n"));)
         return;
    }
    if ((psessionEntry->limSmeState != eLIM_SME_WT_REASSOC_STATE) ||
         (psessionEntry->limSystemRole == eLIM_AP_ROLE)||(psessionEntry->limSystemRole == eLIM_BT_AMP_AP_ROLE))
    {
        /* 
                                                          
                                             
                    
         */
        PELOGE(limLog(pMac, LOGE,
                   FL("Rcv unexpected MLM_REASSOC_CNF in role %d, sme state 0x%X\n"),
                   psessionEntry->limSystemRole, psessionEntry->limSmeState);)
                return;
    }
    if (psessionEntry->pLimReAssocReq) {
        palFreeMemory( pMac->hHdd, psessionEntry->pLimReAssocReq);
        psessionEntry->pLimReAssocReq = NULL;
    }

    PELOGE(limLog(pMac, LOGE, FL("Rcv MLM_REASSOC_CNF with result code %d\n"), pLimMlmReassocCnf->resultCode);)
    if (pLimMlmReassocCnf->resultCode == eSIR_SME_SUCCESS) {
        //                         
        PELOG1(limLog(pMac, LOG1, FL("*** Reassociated with new BSS ***\n"));)

        psessionEntry->limSmeState = eLIM_SME_LINK_EST_STATE;
        MTRACE(macTrace(pMac, TRACE_CODE_SME_STATE, psessionEntry->peSessionId, psessionEntry->limSmeState));

        /* 
                                            
                                        
        */
        limSendSmeJoinReassocRsp(
                               pMac, eWNI_SME_REASSOC_RSP,
                              pLimMlmReassocCnf->resultCode, pLimMlmReassocCnf->protStatusCode,psessionEntry,
                              psessionEntry->smeSessionId,psessionEntry->transactionId);
    }else if (pLimMlmReassocCnf->resultCode == eSIR_SME_REASSOC_REFUSED) {
        /*                                       
                                                        
        */
        psessionEntry->limSmeState = eLIM_SME_LINK_EST_STATE;
        MTRACE(macTrace(pMac, TRACE_CODE_SME_STATE, psessionEntry->peSessionId, psessionEntry->limSmeState));

        /* 
                                            
                                      
        */
        limSendSmeJoinReassocRsp(pMac, eWNI_SME_REASSOC_RSP,
                              pLimMlmReassocCnf->resultCode, pLimMlmReassocCnf->protStatusCode,psessionEntry,
                              psessionEntry->smeSessionId,psessionEntry->transactionId);
    }else {
        //                      
        psessionEntry->limSmeState = eLIM_SME_JOIN_FAILURE_STATE;
    MTRACE(macTrace(pMac, TRACE_CODE_SME_STATE, psessionEntry->peSessionId, psessionEntry->limSmeState));
        /* 
                                             
                                       
         */
        limHandleSmeReaasocResult(pMac, pLimMlmReassocCnf->resultCode, pLimMlmReassocCnf->protStatusCode, psessionEntry);
    }
} /*                                   */

/* 
                            
  
           
                                                       
                                  
  
        
  
              
  
       
  
                                                    
                                                        
  
               
 */
void
limProcessMlmReassocInd(tpAniSirGlobal pMac, tANI_U32 *pMsgBuf)
{
    tANI_U32                len;
    tSirMsgQ           msgQ;
    tSirSmeReassocInd  *pSirSmeReassocInd;
    tpDphHashNode      pStaDs=0;
    tpPESession  psessionEntry;
    tANI_U8      sessionId;
    if(pMsgBuf == NULL)
    {
        PELOGE(limLog(pMac, LOGE,FL("Buffer is Pointing to NULL\n"));)
        return;
    }
    if((psessionEntry = peFindSessionByBssid(pMac,((tpLimMlmReassocInd)pMsgBuf)->peerMacAddr, &sessionId))== NULL)
    {
        PELOGE(limLog(pMac, LOGE,FL("session does not exist for given BSSId\n"));)
        return;
    }
    //                                  
#if (WNI_POLARIS_FW_PACKAGE == ADVANCED)
    len = sizeof(tSirSmeReassocInd) -
          sizeof(tSirNeighborBssInfo) +
          (((tpLimMlmReassocInd) pMsgBuf)->numBss *
           sizeof(tSirNeighborBssInfo)) -
           SIR_MAC_MAX_SSID_LENGTH +
           ((tpLimMlmReassocInd) pMsgBuf)->ssId.length;
#else
    len = sizeof(tSirSmeReassocInd);
#endif
    if( eHAL_STATUS_SUCCESS != palAllocateMemory( pMac->hHdd, (void **)&pSirSmeReassocInd, len))
    {
        //          
        limLog(pMac, LOGP,
           FL("call to palAllocateMemory failed for eWNI_SME_REASSOC_IND\n"));
        return;

    }
    sirStoreU16N((tANI_U8 *) &pSirSmeReassocInd->messageType,
                 eWNI_SME_REASSOC_IND);
    limReassocIndSerDes(pMac, (tpLimMlmReassocInd) pMsgBuf,
                        (tANI_U8 *) &(pSirSmeReassocInd->length), psessionEntry);

    //                                                  
    pSirSmeReassocInd->assocReqLength = ((tpLimMlmReassocInd) pMsgBuf)->assocReqLength;
    pSirSmeReassocInd->assocReqPtr = ((tpLimMlmReassocInd) pMsgBuf)->assocReqPtr;
    pSirSmeReassocInd->beaconPtr = psessionEntry->beacon;
    pSirSmeReassocInd->beaconLength = psessionEntry->bcnLen;
    
    msgQ.type = eWNI_SME_REASSOC_IND;
    msgQ.bodyptr = pSirSmeReassocInd;
    msgQ.bodyval = 0;
    MTRACE(macTraceMsgTx(pMac, psessionEntry->peSessionId, msgQ.type));
#ifdef FEATURE_WLAN_DIAG_SUPPORT_LIM //                         
    limDiagEventReport(pMac, WLAN_PE_DIAG_REASSOC_IND_EVENT, psessionEntry, 0, 0);
#endif //                         
    pStaDs = dphGetHashEntry(pMac, ((tpLimMlmReassocInd) pMsgBuf)->aid, &psessionEntry->dph.dphHashTable);
    if (! pStaDs)
    {
        limLog( pMac, LOGP, FL("MLM ReAssocInd: Station context no longer valid (aid %d)\n"),
                ((tpLimMlmReassocInd) pMsgBuf)->aid);
        palFreeMemory(pMac->hHdd, pSirSmeReassocInd);
        return;
    }

    limSysProcessMmhMsgApi(pMac, &msgQ,  ePROT);
    PELOG1(limLog(pMac, LOG1,
       FL("Create CNF_WAIT_TIMER after received LIM_MLM_REASSOC_IND\n"));)
    /*
                                                         
      */
    limActivateCnfTimer(pMac,
                        (tANI_U16) ((tpLimMlmReassocInd) pMsgBuf)->aid, psessionEntry);
} /*                                   */

/* 
                         
  
           
                                                    
                                  
  
        
  
              
  
       
  
                                                    
                                                        
  
               
 */
void
limProcessMlmAuthInd(tpAniSirGlobal pMac, tANI_U32 *pMsgBuf)
{
    tSirMsgQ           msgQ;
    tSirSmeAuthInd     *pSirSmeAuthInd;

    if(pMsgBuf == NULL)
    {
           PELOGE(limLog(pMac, LOGE,FL("Buffer is Pointing to NULL\n"));)
           return;
    }
    if( eHAL_STATUS_SUCCESS != palAllocateMemory( pMac->hHdd, (void **)&pSirSmeAuthInd, sizeof(tSirSmeAuthInd)))
    {
        //          
        limLog(pMac, LOGP,
           FL("call to palAllocateMemory failed for eWNI_SME_AUTH_IND\n"));
    }
    limCopyU16((tANI_U8 *) &pSirSmeAuthInd->messageType, eWNI_SME_AUTH_IND);
    limAuthIndSerDes(pMac, (tpLimMlmAuthInd) pMsgBuf,
                        (tANI_U8 *) &(pSirSmeAuthInd->length));
    msgQ.type = eWNI_SME_AUTH_IND;
    msgQ.bodyptr = pSirSmeAuthInd;
    msgQ.bodyval = 0;
    MTRACE(macTraceMsgTx(pMac, NO_SESSION, msgQ.type));
#ifdef FEATURE_WLAN_DIAG_SUPPORT_LIM //                         
    limDiagEventReport(pMac, WLAN_PE_DIAG_AUTH_IND_EVENT, NULL, 0, 0);
#endif //                         
    limSysProcessMmhMsgApi(pMac, &msgQ,  ePROT);
} /*                                */




void
limFillAssocIndParams(tpAniSirGlobal pMac, tpLimMlmAssocInd pAssocInd,
                                            tSirSmeAssocInd    *pSirSmeAssocInd,
                                            tpPESession psessionEntry)
{
    pSirSmeAssocInd->length = sizeof(tSirSmeAssocInd);
    pSirSmeAssocInd->sessionId = psessionEntry->smeSessionId;

    //                                                  
    pSirSmeAssocInd->assocReqLength = pAssocInd->assocReqLength;
    pSirSmeAssocInd->assocReqPtr = pAssocInd->assocReqPtr;

    pSirSmeAssocInd->beaconPtr = psessionEntry->beacon;
    pSirSmeAssocInd->beaconLength = psessionEntry->bcnLen;    

    //                    
    palCopyMemory( pMac->hHdd, pSirSmeAssocInd->peerMacAddr, pAssocInd->peerMacAddr, sizeof(tSirMacAddr));
    //            
    pSirSmeAssocInd->aid = pAssocInd->aid;
    //              
    palCopyMemory( pMac->hHdd, pSirSmeAssocInd->bssId, psessionEntry->bssId, sizeof(tSirMacAddr));
    //              
    //                                              
    //                 
    pSirSmeAssocInd->authType = pAssocInd->authType;
    //             
    palCopyMemory( pMac->hHdd,  (tANI_U8*)&pSirSmeAssocInd->ssId,
                                (tANI_U8 *) &(pAssocInd->ssId), pAssocInd->ssId.length + 1);
    pSirSmeAssocInd->rsnIE.length = pAssocInd->rsnIE.length;
    palCopyMemory( pMac->hHdd, (tANI_U8*) &pSirSmeAssocInd->rsnIE.rsnIEdata,
                                (tANI_U8 *) &(pAssocInd->rsnIE.rsnIEdata),
                                pAssocInd->rsnIE.length);

    pSirSmeAssocInd->addIE.length = pAssocInd->addIE.length;
    palCopyMemory( pMac->hHdd, (tANI_U8*) &pSirSmeAssocInd->addIE.addIEdata,
                                (tANI_U8 *) &(pAssocInd->addIE.addIEdata),
                                pAssocInd->addIE.length);

    //                                
    pSirSmeAssocInd->spectrumMgtIndicator = pAssocInd->spectrumMgtIndicator;
    if (pAssocInd->spectrumMgtIndicator == eSIR_TRUE)
    {
        pSirSmeAssocInd->powerCap.minTxPower = pAssocInd->powerCap.minTxPower;
        pSirSmeAssocInd->powerCap.maxTxPower = pAssocInd->powerCap.maxTxPower;
        pSirSmeAssocInd->supportedChannels.numChnl = pAssocInd->supportedChannels.numChnl;
        palCopyMemory( pMac->hHdd, (tANI_U8*) &pSirSmeAssocInd->supportedChannels.channelList,
                     (tANI_U8 *) &(pAssocInd->supportedChannels.channelList),
                     pAssocInd->supportedChannels.numChnl);
    }
#ifdef WLAN_SOFTAP_FEATURE
    //                
    pSirSmeAssocInd->wmmEnabledSta = pAssocInd->WmmStaInfoPresent;
#endif
} /*                             */



/* 
                          
  
           
                                                     
                                  
  
        
  
              
  
       
  
                                                    
                                                        
  
               
 */
void
limProcessMlmAssocInd(tpAniSirGlobal pMac, tANI_U32 *pMsgBuf)
{
    tANI_U32            len;
    tSirMsgQ            msgQ;
    tSirSmeAssocInd    *pSirSmeAssocInd;
    tpDphHashNode       pStaDs=0;
    tpPESession         psessionEntry;
    if(pMsgBuf == NULL)
    {
           PELOGE(limLog(pMac, LOGE,FL("Buffer is Pointing to NULL\n"));)
           return;
    }
    if((psessionEntry = peFindSessionBySessionId(pMac,((tpLimMlmAssocInd) pMsgBuf)->sessionId))== NULL)
    {
        limLog( pMac, LOGE, FL( "Session Does not exist for given sessionId\n" ));
        return;
    }
    //                                
#if defined (ANI_PRODUCT_TYPE_AP)
    len = sizeof(tSirSmeAssocInd) -
              sizeof(tSirNeighborBssInfo) +
              (((tpLimMlmAssocInd) pMsgBuf)->numBss *
               sizeof(tSirNeighborBssInfo)) -
              SIR_MAC_MAX_SSID_LENGTH +
              ((tpLimMlmAssocInd) pMsgBuf)->ssId.length;
#else
    len = sizeof(tSirSmeAssocInd);
#endif
    if( eHAL_STATUS_SUCCESS != palAllocateMemory( pMac->hHdd, (void **)&pSirSmeAssocInd, len))
    {
        //          
        limLog(pMac, LOGP,
               FL("call to palAllocateMemory failed for eWNI_SME_ASSOC_IND\n"));
        return;
    }

#if defined (ANI_PRODUCT_TYPE_AP)
    sirStoreU16N((tANI_U8 *) &pSirSmeAssocInd->messageType,
                 eWNI_SME_ASSOC_IND);
    limAssocIndSerDes(pMac, (tpLimMlmAssocInd) pMsgBuf,
                      (tANI_U8 *) &(pSirSmeAssocInd->length), psessionEntry);
#else
    pSirSmeAssocInd->messageType = eWNI_SME_ASSOC_IND;
    limFillAssocIndParams(pMac, (tpLimMlmAssocInd) pMsgBuf, pSirSmeAssocInd, psessionEntry);
#endif
    msgQ.type = eWNI_SME_ASSOC_IND;
    msgQ.bodyptr = pSirSmeAssocInd;
    msgQ.bodyval = 0;
    pStaDs = dphGetHashEntry(pMac,
                             ((tpLimMlmAssocInd) pMsgBuf)->aid, &psessionEntry->dph.dphHashTable);
    if (! pStaDs)
    {   //                      
        limLog(pMac, LOGE, FL("MLM AssocInd: Station context no longer valid (aid %d)\n"),
               ((tpLimMlmAssocInd) pMsgBuf)->aid);
        palFreeMemory(pMac->hHdd, pSirSmeAssocInd);

        return;
    }
    pSirSmeAssocInd->staId = pStaDs->staIndex;
#ifdef WLAN_SOFTAP_FEATURE
   pSirSmeAssocInd->reassocReq = pStaDs->mlmStaContext.subType;
#endif
    MTRACE(macTraceMsgTx(pMac, psessionEntry->peSessionId, msgQ.type));
#ifdef FEATURE_WLAN_DIAG_SUPPORT_LIM //                         
    limDiagEventReport(pMac, WLAN_PE_DIAG_ASSOC_IND_EVENT, psessionEntry, 0, 0);
#endif //                         
    limSysProcessMmhMsgApi(pMac, &msgQ,  ePROT);

    PELOG1(limLog(pMac, LOG1,
       FL("Create CNF_WAIT_TIMER after received LIM_MLM_ASSOC_IND\n"));)
    /*
                                                       
      */
    limActivateCnfTimer(pMac, (tANI_U16) ((tpLimMlmAssocInd) pMsgBuf)->aid, psessionEntry);

//                                                               
#ifdef TEST_BTAMP_AP
//                  
{
    tpSirSmeAssocCnf     pSmeAssoccnf;
    if(!palAllocateMemory(pMac->hHdd,(void **)&pSmeAssoccnf,sizeof(tSirSmeAssocCnf)))
        PELOGE(limLog(pMac, LOGE, FL("palAllocateMemory failed for pSmeAssoccnf \n"));)
    pSmeAssoccnf->messageType = eWNI_SME_ASSOC_CNF;
    pSmeAssoccnf->length = sizeof(tSirSmeAssocCnf);
    palCopyMemory( pMac->hHdd,pSmeAssoccnf->peerMacAddr,((tpLimMlmAssocInd)pMsgBuf)->peerMacAddr,6);
    pSmeAssoccnf->statusCode = eSIR_SME_SUCCESS;
    pSmeAssoccnf->aid = ((tpLimMlmAssocInd)pMsgBuf)->aid;
    palCopyMemory( pMac->hHdd, pSmeAssoccnf->alternateBssId,pSmeAssoccnf->peerMacAddr,sizeof(tSirMacAddr));
    pSmeAssoccnf->alternateChannelId = 6;
    palCopyMemory( pMac->hHdd,pSmeAssoccnf->bssId,psessionEntry->selfMacAddr,6);
    pMsgBuf = (tANI_U32)pSmeAssoccnf;
    __limProcessSmeAssocCnfNew(pMac, eWNI_SME_ASSOC_CNF, pMsgBuf);
    palFreeMemory(pMac->hHdd,pSmeAssoccnf);
}
#endif


} /*                                 */




/* 
                             
  
           
                                                        
                                  
  
        
  
              
  
       
  
                                                    
                                                        
  
               
 */
void
limProcessMlmDisassocInd(tpAniSirGlobal pMac, tANI_U32 *pMsgBuf)
{
    tLimMlmDisassocInd  *pMlmDisassocInd;
    tpPESession         psessionEntry;
    pMlmDisassocInd = (tLimMlmDisassocInd *) pMsgBuf;
    if( (psessionEntry = peFindSessionBySessionId(pMac,pMlmDisassocInd->sessionId) )== NULL)
    {
        limLog(pMac, LOGP,FL("Session Does not exist for given sessionID\n"));
        return;
    }
    switch (psessionEntry->limSystemRole)
    {
        case eLIM_STA_IN_IBSS_ROLE:
            break;
        case eLIM_STA_ROLE:
        case eLIM_BT_AMP_STA_ROLE:
        psessionEntry->limSmeState = eLIM_SME_WT_DISASSOC_STATE;
            MTRACE(macTrace(pMac, TRACE_CODE_SME_STATE, psessionEntry->peSessionId, psessionEntry->limSmeState));
            break;
        default: //                                   
                PELOG1(limLog(pMac, LOG1,
                       FL("*** Peer staId=%d Disassociated ***\n"),
                        pMlmDisassocInd->aid);)
            //                                           
            //                                        
            break;
    } //                                          
} /*                                    */

/* 
                             
  
           
                                                        
                                  
  
        
  
              
  
       
  
                                                    
                                                        
  
               
 */
void
limProcessMlmDisassocCnf(tpAniSirGlobal pMac, tANI_U32 *pMsgBuf)
{
    tSirResultCodes         resultCode;
    tLimMlmDisassocCnf      *pMlmDisassocCnf;
    tpPESession             psessionEntry;
    pMlmDisassocCnf = (tLimMlmDisassocCnf *) pMsgBuf;
    if((psessionEntry = peFindSessionBySessionId(pMac,pMlmDisassocCnf->sessionId))== NULL)
    {
        PELOGE(limLog(pMac, LOGE,FL("session Does not exist for given session Id\n"));)
        return;
    }
    resultCode = (tSirResultCodes)
                 (pMlmDisassocCnf->disassocTrigger ==
                  eLIM_LINK_MONITORING_DISASSOC) ?
                 eSIR_SME_LOST_LINK_WITH_PEER_RESULT_CODE :
                 pMlmDisassocCnf->resultCode;
    if ((psessionEntry->limSystemRole == eLIM_STA_ROLE)|| (psessionEntry->limSystemRole == eLIM_BT_AMP_STA_ROLE))
    {
#if defined(ANI_AP_CLIENT_SDK)
        tSirMacAddr         nullMacAddr = {0, 0, 0, 0, 0, 0};
#endif
        //                              
        if ( (psessionEntry->limSmeState != eLIM_SME_WT_DISASSOC_STATE) &&
             (psessionEntry->limSmeState != eLIM_SME_WT_DEAUTH_STATE) )
        {
            /* 
                                       
                                  
                                        
                        
             */
            PELOGE(limLog(pMac, LOGE,
               FL("received unexpected MLM_DISASSOC_CNF in state %X\n"),psessionEntry->limSmeState);)
            return;
        }
#if defined(ANI_AP_CLIENT_SDK)
        //                                                                                  
        //                                                                                     
        //                                                                                         
        //                                                                   
        palCopyMemory(pMac->hHdd, pMac->lim.gLimBssid, nullMacAddr, sizeof(tSirMacAddr));
        if (cfgSetStr(pMac, WNI_CFG_BSSID, nullMacAddr, sizeof(tSirMacAddr)) != eSIR_SUCCESS)
        {
            limLog(pMac, LOGP, FL("Could not update BSSID on CFG"));
        }
#endif
        if (pMac->lim.gLimRspReqd)
            pMac->lim.gLimRspReqd = false;
        if (pMlmDisassocCnf->disassocTrigger ==
                                    eLIM_PROMISCUOUS_MODE_DISASSOC)
        {
            if (pMlmDisassocCnf->resultCode != eSIR_SME_SUCCESS)
                psessionEntry->limSmeState = psessionEntry->limPrevSmeState;
            else
                psessionEntry->limSmeState = eLIM_SME_OFFLINE_STATE;
            MTRACE(macTrace(pMac, TRACE_CODE_SME_STATE, psessionEntry->peSessionId, psessionEntry->limSmeState));
            //                                       
            limSendSmePromiscuousModeRsp(pMac);
        }
        else
        {
            if (pMlmDisassocCnf->resultCode != eSIR_SME_SUCCESS)
                psessionEntry->limSmeState = psessionEntry->limPrevSmeState;
            else
                psessionEntry->limSmeState = eLIM_SME_IDLE_STATE;
            MTRACE(macTrace(pMac, TRACE_CODE_SME_STATE, psessionEntry->peSessionId, psessionEntry->limSmeState));
            limSendSmeDisassocNtf(pMac, pMlmDisassocCnf->peerMacAddr,
                                  resultCode,
                                  pMlmDisassocCnf->disassocTrigger,
                                  pMlmDisassocCnf->aid,psessionEntry->smeSessionId,psessionEntry->transactionId,psessionEntry);
        }
    }
    else if  ( (psessionEntry->limSystemRole == eLIM_AP_ROLE)|| (psessionEntry->limSystemRole == eLIM_BT_AMP_AP_ROLE) )
    {
        limSendSmeDisassocNtf(pMac, pMlmDisassocCnf->peerMacAddr,
                              resultCode,
                              pMlmDisassocCnf->disassocTrigger,
                              pMlmDisassocCnf->aid,psessionEntry->smeSessionId,psessionEntry->transactionId,psessionEntry);
    }
} /*                                    */

/* 
                           
  
           
                                                      
                                  
  
        
  
              
  
       
  
                                                    
                                                        
  
               
 */
void
limProcessMlmDeauthInd(tpAniSirGlobal pMac, tANI_U32 *pMsgBuf)
{
    tLimMlmDeauthInd  *pMlmDeauthInd;
    tpPESession psessionEntry;
    tANI_U8     sessionId;
    pMlmDeauthInd = (tLimMlmDeauthInd *) pMsgBuf;
    if((psessionEntry = peFindSessionByBssid(pMac,pMlmDeauthInd->peerMacAddr,&sessionId))== NULL)
    {
         PELOGE(limLog(pMac, LOGE,FL("session does not exist for given BSSId\n"));)
         return;
    }
    switch (psessionEntry->limSystemRole)
    {
        case eLIM_STA_IN_IBSS_ROLE:
            break;
        case eLIM_STA_ROLE:
        case eLIM_BT_AMP_STA_ROLE:
            psessionEntry->limSmeState = eLIM_SME_WT_DEAUTH_STATE;
            MTRACE(macTrace(pMac, TRACE_CODE_SME_STATE, psessionEntry->peSessionId, psessionEntry->limSmeState));

        default: //             
            {
                PELOG1(limLog(pMac, LOG1,
                   FL("*** Received Deauthentication from staId=%d ***\n"),
                   pMlmDeauthInd->aid);)
            }
            //                                          
            //                                        
            break;
    } //                                          
} /*                                  */

/* 
                           
  
           
                                                      
                                  
  
        
  
              
  
       
  
                                                    
                                                        
  
               
 */
void
limProcessMlmDeauthCnf(tpAniSirGlobal pMac, tANI_U32 *pMsgBuf)
{
    tANI_U16                aid;
    tSirResultCodes         resultCode;
    tLimMlmDeauthCnf        *pMlmDeauthCnf;
    tpPESession             psessionEntry;

    if(pMsgBuf == NULL)
    {
        PELOGE(limLog(pMac, LOGE,FL("Buffer is Pointing to NULL\n"));)
        return;
    }
    pMlmDeauthCnf = (tLimMlmDeauthCnf *) pMsgBuf;
    if((psessionEntry = peFindSessionBySessionId(pMac,pMlmDeauthCnf->sessionId))==NULL)
    {
         PELOGE(limLog(pMac, LOGE,FL("session does not exist for given session Id \n"));)
         return;
    }

    resultCode = (tSirResultCodes)
                 (pMlmDeauthCnf->deauthTrigger ==
                  eLIM_LINK_MONITORING_DEAUTH) ?
                 eSIR_SME_LOST_LINK_WITH_PEER_RESULT_CODE :
                 pMlmDeauthCnf->resultCode;
    aid = (psessionEntry->limSystemRole == eLIM_AP_ROLE) ?
          pMlmDeauthCnf->aid : 1;
    if ((psessionEntry->limSystemRole == eLIM_STA_ROLE)|| (psessionEntry->limSystemRole == eLIM_BT_AMP_STA_ROLE))
    {
        #if defined(ANI_AP_CLIENT_SDK)
        tSirMacAddr         nullMacAddr = {0, 0, 0, 0, 0, 0};
        #endif
        //                        
        if (psessionEntry->limSmeState != eLIM_SME_WT_DEAUTH_STATE)
        {
            /* 
                                                      
                                        
                        
             */
            PELOGE(limLog(pMac, LOGE,
               FL("received unexpected MLM_DEAUTH_CNF in state %X\n"),
               psessionEntry->limSmeState);)
            return;
        }
#if defined(ANI_AP_CLIENT_SDK)
        //                                                                                  
        //                                                                                     
        //                                                                                         
        //                                                                   
        palCopyMemory(pMac->hHdd, pMac->lim.gLimBssid, nullMacAddr, sizeof(tSirMacAddr));
        if (cfgSetStr(pMac, WNI_CFG_BSSID, nullMacAddr, sizeof(tSirMacAddr)) != eSIR_SUCCESS)
        {
            limLog(pMac, LOGP, FL("Could not update BSSID on CFG"));
        }
#endif
        if (pMlmDeauthCnf->resultCode == eSIR_SME_SUCCESS)
        {
            psessionEntry->limSmeState = eLIM_SME_IDLE_STATE;
            PELOG1(limLog(pMac, LOG1,
                   FL("*** Deauthenticated with BSS ***\n"));)
        }
        else
            psessionEntry->limSmeState = psessionEntry->limPrevSmeState;
        MTRACE(macTrace(pMac, TRACE_CODE_SME_STATE, psessionEntry->peSessionId, psessionEntry->limSmeState));

        if (pMac->lim.gLimRspReqd)
            pMac->lim.gLimRspReqd = false;
    }
#if (WNI_POLARIS_FW_PACKAGE == ADVANCED) && defined(ANI_PRODUCT_TYPE_AP)
    //                                       
    //                               
    limSendSmeDisassocNtf(pMac, pMlmDeauthCnf->peerMacAddr,
                          resultCode,
                          pMlmDeauthCnf->deauthTrigger,
                          aid);
#else
    //                                                   
    limSendSmeDeauthNtf(pMac, pMlmDeauthCnf->peerMacAddr,
                        resultCode,
                        pMlmDeauthCnf->deauthTrigger,
                        aid,psessionEntry->smeSessionId,psessionEntry->transactionId);
#endif
} /*                                  */

/* 
                             
  
           
                                                         
                                  
  
        
  
              
  
       
  
                                                    
                                                        
  
               
 */
void
limProcessMlmPurgeStaInd(tpAniSirGlobal pMac, tANI_U32 *pMsgBuf)
{
    tSirResultCodes      resultCode;
    tpLimMlmPurgeStaInd  pMlmPurgeStaInd;
    tpPESession          psessionEntry;
#if defined(ANI_AP_CLIENT_SDK)
    tSirMacAddr         nullMacAddr = {0, 0, 0, 0, 0, 0};
#endif
    if(pMsgBuf == NULL)
    {
        PELOGE(limLog(pMac, LOGE,FL("Buffer is Pointing to NULL\n"));)
        return;
    }
    pMlmPurgeStaInd = (tpLimMlmPurgeStaInd) pMsgBuf;
    if((psessionEntry = peFindSessionBySessionId(pMac,pMlmPurgeStaInd->sessionId))==NULL)
    {
        PELOGE(limLog(pMac, LOGE,FL("session does not exist for given bssId\n"));)
        return;
    }
    //                              
    resultCode = (tSirResultCodes) pMlmPurgeStaInd->reasonCode;
    switch (psessionEntry->limSystemRole)
    {
        case eLIM_STA_IN_IBSS_ROLE:
            break;
        case eLIM_STA_ROLE:
        case eLIM_BT_AMP_STA_ROLE:
        default: //             
            if ((psessionEntry->limSystemRole == eLIM_STA_ROLE) &&
                (psessionEntry->limSmeState !=
                                       eLIM_SME_WT_DISASSOC_STATE) &&
                (psessionEntry->limSmeState != eLIM_SME_WT_DEAUTH_STATE))
            {
                /* 
                                           
                                       
                                            
                            
                 */
                PELOGE(limLog(pMac, LOGE,
                   FL("received unexpected MLM_PURGE_STA_IND in state %X\n"),
                   psessionEntry->limSmeState);)
                break;
            }
            PELOG1(limLog(pMac, LOG1,
               FL("*** Polaris cleanup completed for staId=%d ***\n"),
               pMlmPurgeStaInd->aid);)
            if ((psessionEntry->limSystemRole == eLIM_STA_ROLE)||(psessionEntry->limSystemRole == eLIM_BT_AMP_STA_ROLE))
            {
                psessionEntry->limSmeState = eLIM_SME_IDLE_STATE;
                MTRACE(macTrace(pMac, TRACE_CODE_SME_STATE, psessionEntry->peSessionId, psessionEntry->limSmeState));

#if defined(ANI_AP_CLIENT_SDK)
                //                                                                                  
                //                                                                                     
                //                                                                                         
                //                                                                   
                palCopyMemory(pMac->hHdd, pMac->lim.gLimBssid, nullMacAddr, sizeof(tSirMacAddr));
                if (cfgSetStr(pMac, WNI_CFG_BSSID, nullMacAddr, sizeof(tSirMacAddr)) != eSIR_SUCCESS)
                {
                    limLog(pMac, LOGP, FL("Could not update BSSID on CFG"));
                }
#endif
            }
            if (pMlmPurgeStaInd->purgeTrigger == eLIM_PEER_ENTITY_DEAUTH)
            {
#if (WNI_POLARIS_FW_PACKAGE == ADVANCED) && defined(ANI_PRODUCT_TYPE_AP)
                //                                       
                //                               
                limSendSmeDisassocNtf(pMac,
                                    pMlmPurgeStaInd->peerMacAddr,
                                    resultCode,
                                    pMlmPurgeStaInd->purgeTrigger,
                                    pMlmPurgeStaInd->aid,psessionEntry);
#else
                limSendSmeDeauthNtf(pMac,
                            pMlmPurgeStaInd->peerMacAddr,
                            resultCode,
                            pMlmPurgeStaInd->purgeTrigger,
                            pMlmPurgeStaInd->aid,psessionEntry->smeSessionId,psessionEntry->transactionId);
#endif
            }
            else
                limSendSmeDisassocNtf(pMac,
                                    pMlmPurgeStaInd->peerMacAddr,
                                    resultCode,
                                    pMlmPurgeStaInd->purgeTrigger,
                                    pMlmPurgeStaInd->aid,psessionEntry->smeSessionId,psessionEntry->transactionId,psessionEntry);
    } //                                          
} /*                                    */

/* 
                            
  
           
                                                       
                                  
  
        
  
              
  
       
  
                                                    
                                                        
  
               
 */
void
limProcessMlmSetKeysCnf(tpAniSirGlobal pMac, tANI_U32 *pMsgBuf)
{
    //                                            
    tLimMlmSetKeysCnf   *pMlmSetKeysCnf;
    tpPESession        psessionEntry;

    if(pMsgBuf == NULL)
    {
        PELOGE(limLog(pMac, LOGE,FL("Buffer is Pointing to NULL\n"));)
        return;
    }
    pMlmSetKeysCnf = (tLimMlmSetKeysCnf *) pMsgBuf;
    if ((psessionEntry = peFindSessionBySessionId(pMac, pMlmSetKeysCnf->sessionId))== NULL)
    {
        PELOGE(limLog(pMac, LOGE,FL("session does not exist for given sessionId \n"));)
        return;
    }
    limLog( pMac, LOG1,
        FL("Received MLM_SETKEYS_CNF with resultCode = %d\n"),
        pMlmSetKeysCnf->resultCode );
    limSendSmeSetContextRsp(pMac,
                            pMlmSetKeysCnf->peerMacAddr,
#ifdef ANI_PRODUCT_TYPE_AP
                            pMlmSetKeysCnf->aid,
#else
                            1,
#endif
                            (tSirResultCodes) pMlmSetKeysCnf->resultCode,psessionEntry,psessionEntry->smeSessionId,
                            psessionEntry->transactionId);
} /*                                   */
/* 
                              
  
           
                                                         
                                  
  
        
  
              
  
       
  
                                                    
                                                        
  
               
 */
void
limProcessMlmRemoveKeyCnf(tpAniSirGlobal pMac, tANI_U32 *pMsgBuf)
{
    //                                               
    tLimMlmRemoveKeyCnf *pMlmRemoveKeyCnf;
    tpPESession          psessionEntry;

    if(pMsgBuf == NULL)
    {
        PELOGE(limLog(pMac, LOGE,FL("Buffer is Pointing to NULL\n"));)
        return;
    }
    pMlmRemoveKeyCnf = (tLimMlmRemoveKeyCnf *) pMsgBuf;
    if((psessionEntry = peFindSessionBySessionId(pMac,pMlmRemoveKeyCnf->sessionId))== NULL)
    {
        PELOGE(limLog(pMac, LOGE,FL("session Does not exist for given session Id\n"));)
        return;
    }
    limLog( pMac, LOG1,
        FL("Received MLM_REMOVEKEYS_CNF with resultCode = %d\n"),
        pMlmRemoveKeyCnf->resultCode );
    limSendSmeRemoveKeyRsp(pMac,
                           pMlmRemoveKeyCnf->peerMacAddr,
                            (tSirResultCodes) pMlmRemoveKeyCnf->resultCode,psessionEntry,
                            psessionEntry->smeSessionId,psessionEntry->transactionId);
} /*                                     */


/* 
                           
  
           
                                                              
                                                                
                                                            
                                                         
  
        
  
              
  
       
  
                                                       
                                               
  
  
               
 */
static void
limHandleSmeJoinResult(tpAniSirGlobal pMac, tSirResultCodes resultCode, tANI_U16 protStatusCode, tpPESession psessionEntry)
{
    tpDphHashNode pStaDs = NULL;
    tANI_U8         smesessionId;
    tANI_U16        smetransactionId;

    /*                         */
    if(psessionEntry == NULL)
    {
        PELOGE(limLog(pMac, LOGE,FL("psessionEntry is NULL \n"));)
        return;
    }
    smesessionId = psessionEntry->smeSessionId;
    smetransactionId = psessionEntry->transactionId;
    /*                                                                                                         */
    if(resultCode != eSIR_SME_SUCCESS)
    {
          pStaDs = dphGetHashEntry(pMac, DPH_STA_HASH_INDEX_PEER, &psessionEntry->dph.dphHashTable);
          if (pStaDs != NULL)
          {
            pStaDs->mlmStaContext.disassocReason = eSIR_MAC_UNSPEC_FAILURE_REASON;
            pStaDs->mlmStaContext.cleanupTrigger = eLIM_JOIN_FAILURE;
            pStaDs->mlmStaContext.resultCode = resultCode;
            pStaDs->mlmStaContext.protStatusCode = protStatusCode;
            //                                                                                                             
            limCleanupRxPath(pMac, pStaDs, psessionEntry);
            palFreeMemory( pMac->hHdd, psessionEntry->pLimJoinReq);
            psessionEntry->pLimJoinReq = NULL;
            return;
        }
    }

    palFreeMemory( pMac->hHdd, psessionEntry->pLimJoinReq);
    psessionEntry->pLimJoinReq = NULL;
    //                                            
    if(resultCode != eSIR_SME_SUCCESS)
    {
        if(NULL != psessionEntry)
        {
           if(limSetLinkState(pMac, eSIR_LINK_IDLE_STATE,psessionEntry->bssId,
                psessionEntry->selfMacAddr, NULL, NULL) != eSIR_SUCCESS)
               PELOGE(limLog(pMac, LOGE,  FL("Failed to set the LinkState.\n"));)
            peDeleteSession(pMac,psessionEntry);
            psessionEntry = NULL;
        }
    }
    limSendSmeJoinReassocRsp(pMac, eWNI_SME_JOIN_RSP, resultCode, protStatusCode,psessionEntry,
                                                smesessionId,  smetransactionId);
} /*                                  */

/* 
                              
  
           
                                                      
                                                    
                                                           
  
        
  
              
  
       
  
                                                       
                                               
  
  
               
 */
static void
limHandleSmeReaasocResult(tpAniSirGlobal pMac, tSirResultCodes resultCode, tANI_U16 protStatusCode, tpPESession psessionEntry)
{
    tpDphHashNode pStaDs = NULL;
    tANI_U8         smesessionId;
    tANI_U16        smetransactionId;

    if(psessionEntry == NULL)
    {
        PELOGE(limLog(pMac, LOGE,FL("psessionEntry is NULL \n"));)
        return;
    }
    smesessionId = psessionEntry->smeSessionId;
    smetransactionId = psessionEntry->transactionId;
    /*                                                                                                         */
    if(resultCode != eSIR_SME_SUCCESS)
    {
          pStaDs = dphGetHashEntry(pMac, DPH_STA_HASH_INDEX_PEER, &psessionEntry->dph.dphHashTable);
          if (pStaDs != NULL)
          {
            pStaDs->mlmStaContext.disassocReason = eSIR_MAC_UNSPEC_FAILURE_REASON;
            pStaDs->mlmStaContext.cleanupTrigger = eLIM_JOIN_FAILURE;
            pStaDs->mlmStaContext.resultCode = resultCode;
            pStaDs->mlmStaContext.protStatusCode = protStatusCode;
            limCleanupRxPath(pMac, pStaDs, psessionEntry);
            return;
        }
    }

    //                                               
    if(resultCode != eSIR_SME_SUCCESS)
    {
        if(NULL != psessionEntry)
        {
            peDeleteSession(pMac,psessionEntry);
            psessionEntry = NULL;
        }
    }
    limSendSmeJoinReassocRsp(pMac, eWNI_SME_REASSOC_RSP, resultCode, protStatusCode,psessionEntry,
                                                smesessionId,  smetransactionId);
} /*                                     */

/* 
                            
  
           
                                                                 
                                                
                                                              
                                            
  
              
  
       
  
                                                    
                                                                        
  
               
 */
void limProcessMlmAddStaRsp( tpAniSirGlobal pMac, tpSirMsgQ limMsgQ,tpPESession psessionEntry )
{
    //                                                                                                
    //                                                                                                     
    //                  
    SET_LIM_PROCESS_DEFD_MESGS(pMac, true);
    if ((psessionEntry->limSystemRole == eLIM_BT_AMP_AP_ROLE)
#ifdef WLAN_SOFTAP_FEATURE
    || (psessionEntry->limSystemRole == eLIM_AP_ROLE)
#endif
    )
    {
        limProcessBtAmpApMlmAddStaRsp(pMac, limMsgQ,psessionEntry);
        return;
    }
#if defined(ANI_PRODUCT_TYPE_CLIENT) || defined(ANI_AP_CLIENT_SDK)
    limProcessStaMlmAddStaRsp(pMac, limMsgQ,psessionEntry);
#endif
}
void limProcessStaMlmAddStaRsp( tpAniSirGlobal pMac, tpSirMsgQ limMsgQ ,tpPESession psessionEntry)
{
    tLimMlmAssocCnf mlmAssocCnf;
    tpDphHashNode   pStaDs;
    tANI_U32        mesgType = LIM_MLM_ASSOC_CNF;
    tpAddStaParams  pAddStaParams = (tpAddStaParams) limMsgQ->bodyptr;

    if(NULL == pAddStaParams )
    {
        limLog( pMac, LOGE, FL( "Encountered NULL Pointer\n" ));
        return;
    }
    if( eHAL_STATUS_SUCCESS == pAddStaParams->status )
    {
        if( eLIM_MLM_WT_ADD_STA_RSP_STATE != psessionEntry->limMlmState)
        {
            //                                        
            limLog( pMac, LOGE,
                FL( "Received unexpected WDA_ADD_STA_RSP in state %X\n" ),
                psessionEntry->limMlmState);
            mlmAssocCnf.resultCode = (tSirResultCodes) eSIR_SME_REFUSED;
            goto end;
        }
    if (psessionEntry->limSmeState == eLIM_SME_WT_REASSOC_STATE)
             mesgType = LIM_MLM_REASSOC_CNF;
        //
        //                                       
        //                       
        //
        pStaDs = dphGetHashEntry( pMac, DPH_STA_HASH_INDEX_PEER, &psessionEntry->dph.dphHashTable);
        if( NULL != pStaDs)
            pStaDs->mlmStaContext.mlmState = eLIM_MLM_LINK_ESTABLISHED_STATE;
        else
            limLog( pMac, LOGW,
            FL( "Unable to get the DPH Hash Entry for AID - %d\n" ),
            DPH_STA_HASH_INDEX_PEER);
        psessionEntry->limMlmState = eLIM_MLM_LINK_ESTABLISHED_STATE;
        MTRACE(macTrace(pMac, TRACE_CODE_MLM_STATE, psessionEntry->peSessionId, psessionEntry->limMlmState));
        /*
                                                                         
                                                               
                                                                           
                                         
        */
        psessionEntry->staId = pAddStaParams->staIdx;
        //                                                                                            
        limReactivateHeartBeatTimer(pMac, psessionEntry);
        MTRACE(macTrace(pMac, TRACE_CODE_TIMER_ACTIVATE, psessionEntry->peSessionId, eLIM_KEEPALIVE_TIMER));

        //                                        
        pMac->lim.limTimers.gLimKeepaliveTimer.sessionId = psessionEntry->peSessionId;
        if (tx_timer_activate(&pMac->lim.limTimers.gLimKeepaliveTimer) != TX_SUCCESS)
            limLog(pMac, LOGP, FL("Cannot activate keepalive timer.\n"));
#ifdef WLAN_DEBUG
        pMac->lim.gLimNumLinkEsts++;
#endif
        //                                         
        //                                                
        //                      
        //                                                            
        mlmAssocCnf.resultCode = (tSirResultCodes) eSIR_SME_SUCCESS;
    }
    else
    {
        limLog( pMac, LOGW, FL( "ADD_STA failed!\n"));
        mlmAssocCnf.resultCode = (tSirResultCodes) eSIR_SME_REFUSED;
    }
end:
    if( 0 != limMsgQ->bodyptr )
    {
      palFreeMemory( pMac->hHdd, (void *) pAddStaParams );
    }
    /*                       */
    mlmAssocCnf.sessionId = psessionEntry->peSessionId;
    limPostSmeMessage( pMac, mesgType, (tANI_U32 *) &mlmAssocCnf );
    return;
}
void limProcessMlmDelBssRsp( tpAniSirGlobal pMac, tpSirMsgQ limMsgQ,tpPESession psessionEntry)
{
 //                                                                                                
  //                                                                                                     
  //                  
   //                                   
  //                                                                             
   //                                                                                         
  //   
   //                                                                                
   //            
  //   
  SET_LIM_PROCESS_DEFD_MESGS(pMac, true);

    if (((psessionEntry->limSystemRole == eLIM_BT_AMP_AP_ROLE)  ||
         (psessionEntry->limSystemRole == eLIM_BT_AMP_STA_ROLE)
#ifdef WLAN_SOFTAP_FEATURE
         || (psessionEntry->limSystemRole == eLIM_AP_ROLE)
#endif
         ) &&
        (psessionEntry->statypeForBss == STA_ENTRY_SELF))
    {
        limProcessBtAmpApMlmDelBssRsp(pMac, limMsgQ,psessionEntry);
        return;
    }
#if defined(ANI_PRODUCT_TYPE_CLIENT) || defined(ANI_AP_CLIENT_SDK)
    limProcessStaMlmDelBssRsp(pMac, limMsgQ,psessionEntry);
#endif
}

void limProcessStaMlmDelBssRsp( tpAniSirGlobal pMac, tpSirMsgQ limMsgQ,tpPESession psessionEntry)
{
    tpDeleteBssParams pDelBssParams =   (tpDeleteBssParams) limMsgQ->bodyptr;
    tpDphHashNode pStaDs =              dphGetHashEntry(pMac, DPH_STA_HASH_INDEX_PEER, &psessionEntry->dph.dphHashTable);
    tSirResultCodes statusCode =        eSIR_SME_SUCCESS;
    if( eHAL_STATUS_SUCCESS == pDelBssParams->status )
    {
        PELOGW(limLog( pMac, LOGW,
                      FL( "STA received the DEL_BSS_RSP for BSSID: %X.\n"),pDelBssParams->bssIdx);)
        if (limSetLinkState(pMac, eSIR_LINK_IDLE_STATE, psessionEntry->bssId,
             psessionEntry->selfMacAddr, NULL, NULL) != eSIR_SUCCESS)

        {
            PELOGE(limLog( pMac, LOGE, FL( "Failure in setting link state to IDLE\n"));)
            statusCode = eSIR_SME_REFUSED;
            goto end;
        }
        if(pStaDs == NULL)
        {
            limLog( pMac, LOGE, FL( "DPH Entry for STA 1 missing.\n"));
            statusCode = eSIR_SME_REFUSED;
            goto end;
        }
         if( eLIM_MLM_WT_DEL_BSS_RSP_STATE != pStaDs->mlmStaContext.mlmState)
        {
            PELOGE(limLog( pMac, LOGE, FL( "Received unexpected WDA_DEL_BSS_RSP in state %X\n" ),
                           pStaDs->mlmStaContext.mlmState);)
            statusCode = eSIR_SME_REFUSED;
            goto end;
        }
        PELOG1(limLog( pMac, LOG1, FL("STA AssocID %d MAC "), pStaDs->assocId );
        limPrintMacAddr(pMac, pStaDs->staAddr, LOG1);)
    }
    else
    {
        limLog( pMac, LOGP, FL( "DEL BSS failed!\n" ) );
        if( NULL != pDelBssParams )
        {
            palFreeMemory( pMac->hHdd, (void *) pDelBssParams );
    	}
        return;
    }
   end:
     if( 0 != limMsgQ->bodyptr )
     {
        palFreeMemory( pMac->hHdd, (void *) pDelBssParams );
     }
    if(pStaDs == NULL)
          return;
    if ( ((psessionEntry->limSystemRole  == eLIM_STA_ROLE) || (psessionEntry->limSystemRole  == eLIM_BT_AMP_STA_ROLE)) &&
                (psessionEntry->limSmeState != eLIM_SME_WT_DISASSOC_STATE  &&
                 psessionEntry->limSmeState != eLIM_SME_WT_DEAUTH_STATE) &&
                 pStaDs->mlmStaContext.cleanupTrigger != eLIM_JOIN_FAILURE)
   {
        /*                                           
                                                             
                                   
        */
        limHandleDelBssInReAssocContext(pMac, pStaDs,psessionEntry);
        return;
    }
    limPrepareAndSendDelStaCnf(pMac, pStaDs, statusCode,psessionEntry);
    return;
}

void limProcessBtAmpApMlmDelBssRsp( tpAniSirGlobal pMac, tpSirMsgQ limMsgQ,tpPESession psessionEntry)
{
    tSirResultCodes rc = eSIR_SME_SUCCESS;
    tSirRetStatus status;
    tpDeleteBssParams pDelBss = (tpDeleteBssParams) limMsgQ->bodyptr;
    tSirMacAddr             nullBssid = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

    if(psessionEntry == NULL)
    {
        limLog(pMac, LOGE,FL("Session entry passed is NULL\n"));
        if(pDelBss != NULL)
            palFreeMemory( pMac->hHdd, (void *) pDelBss );
        return;
    }

    if (pDelBss == NULL)
    {
        PELOGE(limLog(pMac, LOGE, FL("BSS: DEL_BSS_RSP with no body!\n"));)
        rc = eSIR_SME_REFUSED;
        goto end;
    }
    pMac->lim.gLimMlmState = eLIM_MLM_IDLE_STATE;
    if( eLIM_MLM_WT_DEL_BSS_RSP_STATE != psessionEntry->limMlmState)
    {
            limLog( pMac, LOGE,
               FL( "Received unexpected WDA_DEL_BSS_RSP in state %X" ),
               psessionEntry->limMlmState);
            rc = eSIR_SME_REFUSED;
           goto end;
    }
    if (pDelBss->status != eHAL_STATUS_SUCCESS)
    {
        limLog(pMac, LOGE, FL("BSS: DEL_BSS_RSP error (%x) Bss %d "),
               pDelBss->status, pDelBss->bssIdx);
        rc = eSIR_SME_STOP_BSS_FAILURE;
        goto end;
    }
    status = limSetLinkState(pMac, eSIR_LINK_IDLE_STATE, nullBssid,
           psessionEntry->selfMacAddr, NULL, NULL);
    if (status != eSIR_SUCCESS)
    {
        rc = eSIR_SME_REFUSED;
        goto end;
    }
    /*                                                                                       
                                                                                                 
                                   
     */
#if 0 //                                    
    if (LIM_IS_RADAR_DETECTED(pMac))
    {
         limFrameTransmissionControl(pMac, eLIM_TX_BSS_BUT_BEACON, eLIM_RESUME_TX);
         LIM_SET_RADAR_DETECTED(pMac, eANI_BOOLEAN_FALSE);
    }
#endif
    dphHashTableClassInit(pMac, &psessionEntry->dph.dphHashTable);//                         
    limDeletePreAuthList(pMac);
#ifdef WLAN_SOFTAP_FEATURE
    //                                                       
    pMac->lim.gLimNumOfCurrentSTAs = 0;
#endif
    end:
    limSendSmeRsp(pMac, eWNI_SME_STOP_BSS_RSP, rc,  psessionEntry->smeSessionId,  psessionEntry->transactionId);
    peDeleteSession(pMac, psessionEntry);

    if(pDelBss != NULL)
        palFreeMemory( pMac->hHdd, (void *) pDelBss );
}

void limProcessMlmDelStaRsp( tpAniSirGlobal pMac, tpSirMsgQ limMsgQ )
{
 //                                                                                                
  //                                                                                                     
  //                  

    tpPESession         psessionEntry;
    tpDeleteStaParams   pDeleteStaParams;
    pDeleteStaParams = (tpDeleteStaParams)limMsgQ->bodyptr;
    SET_LIM_PROCESS_DEFD_MESGS(pMac, true);
    if((psessionEntry = peFindSessionBySessionId(pMac,pDeleteStaParams->sessionId))==NULL)
    {
        limLog(pMac, LOGP,FL("Session Does not exist for given sessionID\n"));
        if(pDeleteStaParams != NULL)
            palFreeMemory( pMac->hHdd, (void *) pDeleteStaParams );
        return;
    }

    if ((psessionEntry->limSystemRole == eLIM_BT_AMP_AP_ROLE)
#ifdef WLAN_SOFTAP_FEATURE
      || (psessionEntry->limSystemRole == eLIM_AP_ROLE)
#endif
    )
    {
        limProcessBtAmpApMlmDelStaRsp(pMac,limMsgQ,psessionEntry);
        return;
    }
#if defined(ANI_PRODUCT_TYPE_CLIENT) || defined(ANI_AP_CLIENT_SDK)
    limProcessStaMlmDelStaRsp(pMac, limMsgQ,psessionEntry);
#endif
}

void limProcessBtAmpApMlmDelStaRsp( tpAniSirGlobal pMac, tpSirMsgQ limMsgQ,tpPESession psessionEntry)
{
    tpDeleteStaParams pDelStaParams = (tpDeleteStaParams) limMsgQ->bodyptr;
    tpDphHashNode pStaDs;
    tSirResultCodes statusCode = eSIR_SME_SUCCESS;
    if(limMsgQ->bodyptr == NULL)
    {
      return;
    }

    pStaDs = dphGetHashEntry(pMac, pDelStaParams->assocId, &psessionEntry->dph.dphHashTable);
    if(pStaDs == NULL)
    {
        limLog( pMac, LOGE,
             FL( "DPH Entry for STA %X missing.\n"), pDelStaParams->assocId);
        statusCode = eSIR_SME_REFUSED;
        palFreeMemory( pMac->hHdd, (void *) pDelStaParams );

        return;
    }
    if( eHAL_STATUS_SUCCESS == pDelStaParams->status )
    {
        limLog( pMac, LOGW,
                   FL( "AP received the DEL_STA_RSP for assocID: %X.\n"), pDelStaParams->assocId);

        if(( eLIM_MLM_WT_DEL_STA_RSP_STATE != pStaDs->mlmStaContext.mlmState) &&
           ( eLIM_MLM_WT_ASSOC_DEL_STA_RSP_STATE != pStaDs->mlmStaContext.mlmState))
        {
            limLog( pMac, LOGE,
              FL( "Received unexpected WDA_DEL_STA_RSP in state %s for staId %d assocId %d \n" ),
               limMlmStateStr(pStaDs->mlmStaContext.mlmState), pStaDs->staIndex, pStaDs->assocId);
            statusCode = eSIR_SME_REFUSED;
            goto end;
        }

        limLog( pMac, LOG1,
            FL("Deleted STA AssocID %d staId %d MAC "),
            pStaDs->assocId, pStaDs->staIndex);
        limPrintMacAddr(pMac, pStaDs->staAddr, LOG1);
       if(eLIM_MLM_WT_ASSOC_DEL_STA_RSP_STATE == pStaDs->mlmStaContext.mlmState)
       {
            palFreeMemory( pMac->hHdd, (void *) pDelStaParams );
            if (limAddSta(pMac, pStaDs,psessionEntry) != eSIR_SUCCESS)
            {
                PELOGE(limLog(pMac, LOGE,
                       FL("could not Add STA with assocId=%d\n"),
                       pStaDs->assocId);)
              //                                            
               //                                     
                if(pStaDs->qos.addtsPresent)
                {
                  tpLimTspecInfo pTspecInfo;
                  if(eSIR_SUCCESS == limTspecFindByAssocId(pMac, pStaDs->assocId,
                            &pStaDs->qos.addts.tspec, &pMac->lim.tspecInfo[0], &pTspecInfo))
                  {
                    limAdmitControlDeleteTS(pMac, pStaDs->assocId, &pStaDs->qos.addts.tspec.tsinfo,
                                                            NULL, &pTspecInfo->idx);
                  }
                }
                limRejectAssociation(pMac,
                         pStaDs->staAddr,
                         pStaDs->mlmStaContext.subType,
                         true, pStaDs->mlmStaContext.authType,
                         pStaDs->assocId, true,
                         (tSirResultCodes) eSIR_MAC_UNSPEC_FAILURE_STATUS,
                         psessionEntry);
            }
            return;
        }
    }
    else
    {
        limLog( pMac, LOGW,
             FL( "DEL STA failed!\n" ));
        statusCode = eSIR_SME_REFUSED;
    }
    end:
    palFreeMemory( pMac->hHdd, (void *) pDelStaParams );
    if(eLIM_MLM_WT_ASSOC_DEL_STA_RSP_STATE != pStaDs->mlmStaContext.mlmState)
    {
       limPrepareAndSendDelStaCnf(pMac, pStaDs, statusCode,psessionEntry);
    }
    return;
}

void limProcessStaMlmDelStaRsp( tpAniSirGlobal pMac, tpSirMsgQ limMsgQ,tpPESession psessionEntry)
{
    tSirResultCodes   statusCode    = eSIR_SME_SUCCESS;
    tpDeleteStaParams pDelStaParams = (tpDeleteStaParams) limMsgQ->bodyptr;
    tpDphHashNode     pStaDs        = NULL;
    if(NULL == pDelStaParams )
    {
        limLog( pMac, LOGE, FL( "Encountered NULL Pointer\n" ));
        goto end;
    }
    if( eHAL_STATUS_SUCCESS == pDelStaParams->status )
    {
        pStaDs = dphGetHashEntry(pMac, DPH_STA_HASH_INDEX_PEER, &psessionEntry->dph.dphHashTable);
        if (pStaDs == NULL)
        {
            //                                        
            limLog( pMac, LOGE, FL( "DPH Entry for STA %X missing.\n"),
                    pDelStaParams->assocId);
            statusCode = eSIR_SME_REFUSED;
            goto end;
        }
        if( eLIM_MLM_WT_DEL_STA_RSP_STATE != psessionEntry->limMlmState)
        {
            //                                        
            limLog( pMac, LOGE, FL( "Received unexpected WDA_DELETE_STA_RSP in state %s\n" ),
                  limMlmStateStr(psessionEntry->limMlmState));
            statusCode = eSIR_SME_REFUSED;
            goto end;
        }
        PELOG1(limLog( pMac, LOG1, FL("STA AssocID %d MAC "), pStaDs->assocId );
        limPrintMacAddr(pMac, pStaDs->staAddr, LOG1);)
        limLog( pMac, LOGW, FL( "DEL_STA_RSP received for assocID: %X\n"), pDelStaParams->assocId);
        //                                                                        
        if( 0 != limMsgQ->bodyptr )
        {
            palFreeMemory( pMac->hHdd, (void *) pDelStaParams );
        }
        statusCode = (tSirResultCodes) limDelBss(pMac, pStaDs, 0,psessionEntry);
        return;
    }
    else
    {
        limLog( pMac, LOGW, FL( "DEL_STA failed!\n" ));
        statusCode = eSIR_SME_REFUSED;
    }
end:
    if( 0 != limMsgQ->bodyptr )
    {
        palFreeMemory( pMac->hHdd, (void *) pDelStaParams );
    }
    return;
}

void limProcessBtAmpApMlmAddStaRsp( tpAniSirGlobal pMac, tpSirMsgQ limMsgQ,tpPESession psessionEntry)
{
    tpAddStaParams pAddStaParams = (tpAddStaParams) limMsgQ->bodyptr;
    tpDphHashNode pStaDs = dphGetHashEntry(pMac, pAddStaParams->assocId, &psessionEntry->dph.dphHashTable);
    if(pStaDs == NULL)
    {
        //                                        
        limLog( pMac, LOGE, FL( "DPH Entry for STA %X missing.\n"), pAddStaParams->assocId);
        goto end;
    }
    //
    //                  
    //                                                            
    //
    //                                            
    if( eLIM_MLM_WT_ADD_STA_RSP_STATE != pStaDs->mlmStaContext.mlmState)
    {
        //                                        
        limLog( pMac, LOGE,
                FL( "Received unexpected WDA_ADD_STA_RSP in state %X\n" ),
                pStaDs->mlmStaContext.mlmState);
        goto end;
    }
    if(eHAL_STATUS_SUCCESS != pAddStaParams->status)
    {
        PELOGE(limLog(pMac, LOGE, FL("Error! rcvd delSta rsp from HAL with status %d\n"),pAddStaParams->status);)
        limRejectAssociation(pMac, pStaDs->staAddr,
                 pStaDs->mlmStaContext.subType,
                 true, pStaDs->mlmStaContext.authType,
                 pStaDs->assocId, true,
                 (tSirResultCodes) eSIR_MAC_UNSPEC_FAILURE_STATUS,
                 psessionEntry);
        goto end;
    }
    pStaDs->bssId = pAddStaParams->bssIdx;
    pStaDs->staIndex = pAddStaParams->staIdx;
    //                                                                                            
    pStaDs->valid = 1;
    pStaDs->mlmStaContext.mlmState = eLIM_MLM_WT_ASSOC_CNF_STATE;
    limLog( pMac, LOG1,
            FL("STA AssocID %d staId %d MAC "),
            pStaDs->assocId,
            pStaDs->staIndex);
    limPrintMacAddr(pMac, pStaDs->staAddr, LOG1);

    /*                                          
                                            
                                                 
                                                             
     */
    limSendMlmAssocInd(pMac, pStaDs, psessionEntry);
    //                                                             
end:
    if( 0 != limMsgQ->bodyptr )
    {
        palFreeMemory( pMac->hHdd, (void *) pAddStaParams );
    }
    return;
}

/* 
                             
  
           
                                                                 
                                                
                                            
                                       
                                      
                                           
                                              
                                                                
                                                    
  
        
                                          
                                      
                                    
                                            
                                              
                                                  
  
              
                                                                  
                                                  
  
       
  
                                                    
                                                                        
  
               
 */
static void
limProcessApMlmAddBssRsp( tpAniSirGlobal pMac, tpSirMsgQ limMsgQ)
{
    tLimMlmStartCnf mlmStartCnf;
    tANI_U32 val;
    tpPESession psessionEntry;
//                          
    tpAddBssParams pAddBssParams = (tpAddBssParams) limMsgQ->bodyptr;
    if(NULL == pAddBssParams )
    {
        limLog( pMac, LOGE, FL( "Encountered NULL Pointer\n" ));
        goto end;
    }
    //                                                                               
    if((psessionEntry = peFindSessionBySessionId(pMac,pAddBssParams->sessionId))== NULL)
    {
        PELOGE(limLog(pMac, LOGE,FL("session does not exist for given sessionId\n"));)
        if( NULL != pAddBssParams )
            palFreeMemory( pMac->hHdd, (void *) pAddBssParams );
        return;
    }
    /*                     */
    mlmStartCnf.sessionId = pAddBssParams->sessionId;
    if( eHAL_STATUS_SUCCESS == pAddBssParams->status )
    {
        PELOG2(limLog(pMac, LOG2, FL("WDA_ADD_BSS_RSP returned with eHAL_STATUS_SUCCESS\n"));)
        if (limSetLinkState(pMac, eSIR_LINK_AP_STATE,psessionEntry->bssId,
              psessionEntry->selfMacAddr, NULL, NULL) != eSIR_SUCCESS )
            goto end;
        //               
        psessionEntry->limMlmState = eLIM_MLM_BSS_STARTED_STATE;
        MTRACE(macTrace(pMac, TRACE_CODE_MLM_STATE, psessionEntry->peSessionId,  psessionEntry->limMlmState));
        if( eSIR_IBSS_MODE == pAddBssParams->bssType )
        {
            /*                                  
                                                                        
                                                          
             */
            psessionEntry->limIbssActive = false;
            psessionEntry->statypeForBss = STA_ENTRY_PEER; //                                     
            limResetHBPktCount( psessionEntry );
            limHeartBeatDeactivateAndChangeTimer(pMac, psessionEntry);
            MTRACE(macTrace(pMac, TRACE_CODE_TIMER_ACTIVATE, psessionEntry->peSessionId, eLIM_HEART_BEAT_TIMER));
            if (limActivateHearBeatTimer(pMac) != TX_SUCCESS)
                limLog(pMac, LOGP, FL("could not activate Heartbeat timer\n"));
        }
        psessionEntry->bssIdx     = (tANI_U8) pAddBssParams->bssIdx;

#ifdef ANI_PRODUCT_TYPE_AP
        psessionEntry->limSystemRole = eLIM_AP_ROLE;
#else
        psessionEntry->limSystemRole = eLIM_STA_IN_IBSS_ROLE;
#endif

#ifdef WLAN_SOFTAP_FEATURE
        if ( eSIR_INFRA_AP_MODE == pAddBssParams->bssType )
            psessionEntry->limSystemRole = eLIM_AP_ROLE;
        else
            psessionEntry->limSystemRole = eLIM_STA_IN_IBSS_ROLE;
#endif
        schEdcaProfileUpdate(pMac, psessionEntry);
        limInitPreAuthList(pMac);
        limInitAIDpool(pMac,psessionEntry);
        //                          
        if (!pMac->lim.gLimTimersCreated)
            limCreateTimers(pMac);

        //                 
        if (tx_timer_activate(&pMac->lim.limTimers.gLimUpdateOlbcCacheTimer) != TX_SUCCESS)
        {
            limLog(pMac, LOGE, FL("tx_timer_activate failed\n"));
        }

        /*                                                               */
        if( eSIR_SUCCESS != wlan_cfgGetInt( pMac, WNI_CFG_TRIG_STA_BK_SCAN, &val ))
            limLog( pMac, LOGP, FL("Failed to get WNI_CFG_TRIG_STA_BK_SCAN!\n"));
        pMac->lim.gLimTriggerBackgroundScanDuringQuietBss = (val) ? 1 : 0;
        //                                         
        limApplyConfiguration(pMac,psessionEntry);
        psessionEntry->staId = pAddBssParams->staContext.staIdx;
        mlmStartCnf.resultCode  = eSIR_SME_SUCCESS;
    }
    else
    {
        limLog( pMac, LOGE, FL( "WDA_ADD_BSS_REQ failed with status %d\n" ),pAddBssParams->status );
        mlmStartCnf.resultCode = eSIR_SME_HAL_SEND_MESSAGE_FAIL;
    }
    limPostSmeMessage( pMac, LIM_MLM_START_CNF, (tANI_U32 *) &mlmStartCnf );
    end:
    if( 0 != limMsgQ->bodyptr )
        palFreeMemory( pMac->hHdd, (void *) pAddBssParams );
}


/* 
                               
  
           
                                                                 
                                                
                                            
                                       
                                      
                                           
                                              
                                                                
                                                    
  
        
                                          
                                      
                                    
                                            
                                              
                                                  
  
              
                                                                  
                                                  
  
       
  
                                                    
                                                                        
  
               
 */
static void
limProcessIbssMlmAddBssRsp( tpAniSirGlobal pMac, tpSirMsgQ limMsgQ ,tpPESession psessionEntry)
{
    tLimMlmStartCnf mlmStartCnf;
    tpAddBssParams pAddBssParams = (tpAddBssParams) limMsgQ->bodyptr;
    tANI_U32 val;
    if( eHAL_STATUS_SUCCESS == pAddBssParams->status )
    {
        PELOG1(limLog(pMac, LOG1, FL("WDA_ADD_BSS_RSP returned with eHAL_STATUS_SUCCESS\n"));)
        if (limSetLinkState(pMac, eSIR_LINK_IBSS_STATE,psessionEntry->bssId,
             psessionEntry->selfMacAddr, NULL, NULL) != eSIR_SUCCESS )
            goto end;
        //               
        psessionEntry->limMlmState = eLIM_MLM_BSS_STARTED_STATE;
        MTRACE(macTrace(pMac, TRACE_CODE_MLM_STATE, psessionEntry->peSessionId, psessionEntry->limMlmState));
        /*                                  
                                                                    
                                                      
         */
        psessionEntry->limIbssActive = false;
        limResetHBPktCount( psessionEntry );
        /*                                                                  */
        limHeartBeatDeactivateAndChangeTimer(pMac, psessionEntry);
        MTRACE(macTrace(pMac, TRACE_CODE_TIMER_ACTIVATE, psessionEntry->peSessionId, eLIM_HEART_BEAT_TIMER));
        if (limActivateHearBeatTimer(pMac) != TX_SUCCESS)
            limLog(pMac, LOGP, FL("could not activate Heartbeat timer\n"));
        psessionEntry->bssIdx     = (tANI_U8) pAddBssParams->bssIdx;
        psessionEntry->limSystemRole = eLIM_STA_IN_IBSS_ROLE;
        psessionEntry->statypeForBss = STA_ENTRY_SELF;
        schEdcaProfileUpdate(pMac, psessionEntry);
        //                                                                   
       //                         
        limInitAIDpool(pMac,psessionEntry);
        //                          
#ifdef FIXME_GEN6  //                                                                                                  
        if (!pMac->lim.gLimTimersCreated)
            limCreateTimers(pMac);
#endif
        /*                                                               */
        if( eSIR_SUCCESS != wlan_cfgGetInt( pMac, WNI_CFG_TRIG_STA_BK_SCAN, &val ))
            limLog( pMac, LOGP, FL("Failed to get WNI_CFG_TRIG_STA_BK_SCAN!\n"));
        pMac->lim.gLimTriggerBackgroundScanDuringQuietBss = (val) ? 1 : 0;
        //                                         
        limApplyConfiguration(pMac,psessionEntry);
        psessionEntry->staId = pAddBssParams->staContext.staIdx;
        mlmStartCnf.resultCode  = eSIR_SME_SUCCESS;
        //                                                                                                           
        if(true == pMac->lim.gLimIbssCoalescingHappened)
        {
            limIbssAddBssRspWhenCoalescing(pMac, limMsgQ->bodyptr, psessionEntry);
            goto end;
        }
    }
    else
    {
        limLog( pMac, LOGE, FL( "WDA_ADD_BSS_REQ failed with status %d\n" ),
            pAddBssParams->status );
        mlmStartCnf.resultCode = eSIR_SME_HAL_SEND_MESSAGE_FAIL;
    }
    //                                                          
    //                                                            
    /*                     */
    mlmStartCnf.sessionId =psessionEntry->peSessionId;
    limPostSmeMessage( pMac, LIM_MLM_START_CNF, (tANI_U32 *) &mlmStartCnf );
    end:
    if( 0 != limMsgQ->bodyptr )
        palFreeMemory( pMac->hHdd, (void *) pAddBssParams );
}

static void
limProcessStaMlmAddBssRspPreAssoc( tpAniSirGlobal pMac, tpSirMsgQ limMsgQ, tpPESession psessionEntry )
{
    tpAddBssParams pAddBssParams = (tpAddBssParams) limMsgQ->bodyptr;
    tAniAuthType       cfgAuthType, authMode;
    tLimMlmAuthReq     *pMlmAuthReq;
    tpDphHashNode pStaDs = NULL;
    if( eHAL_STATUS_SUCCESS == pAddBssParams->status )
    {
            if ((pStaDs = dphAddHashEntry(pMac, pAddBssParams->staContext.staMac, DPH_STA_HASH_INDEX_PEER, &psessionEntry->dph.dphHashTable)) == NULL)
            {
                //                               
                PELOGE(limLog(pMac, LOGE, FL("could not add hash entry at DPH for \n"));)
                limPrintMacAddr(pMac, pAddBssParams->staContext.staMac, LOGE);
                goto joinFailure;
            }
            psessionEntry->bssIdx     = (tANI_U8) pAddBssParams->bssIdx;
            //                     
            pStaDs->bssId = pAddBssParams->bssIdx;
            //                                                       
            pStaDs->staIndex = pAddBssParams->staContext.staIdx;
            //                               
            if (wlan_cfgGetInt(pMac, WNI_CFG_AUTHENTICATION_TYPE,
                          (tANI_U32 *) &cfgAuthType) != eSIR_SUCCESS)
            {
                /* 
                                                   
                             
                 */
                limLog(pMac, LOGP,
                       FL("could not retrieve AuthType\n"));
            }
            if (cfgAuthType == eSIR_AUTO_SWITCH) 
                authMode = eSIR_OPEN_SYSTEM; //                              
            else
                authMode = cfgAuthType;

            //                                 
            if( eHAL_STATUS_SUCCESS != palAllocateMemory( pMac->hHdd, (void **)&pMlmAuthReq, sizeof(tLimMlmAuthReq)))
            {
                //          
                limLog(pMac, LOGP,
                       FL("call to palAllocateMemory failed for mlmAuthReq\n"));
                return;
            }
            #if 0
            val = sizeof(tSirMacAddr);
            if (wlan_cfgGetStr(pMac, WNI_CFG_BSSID,
                          pMlmAuthReq->peerMacAddr,
                          &val) != eSIR_SUCCESS)
            {
                //                                          
                limLog(pMac, LOGP, FL("could not retrieve BSSID\n"));
            }
            #endif //                 
            sirCopyMacAddr(pMlmAuthReq->peerMacAddr,psessionEntry->bssId);

            pMlmAuthReq->authType = authMode;
            if (wlan_cfgGetInt(pMac, WNI_CFG_AUTHENTICATE_FAILURE_TIMEOUT,
                          (tANI_U32 *) &pMlmAuthReq->authFailureTimeout)
                          != eSIR_SUCCESS)
            {
                /* 
                                                   
                                             
                 */
                limLog(pMac, LOGP,
                   FL("could not retrieve AuthFailureTimeout value\n"));
            }
            //                                                                   
            //                                               
            psessionEntry->limMlmState = eLIM_MLM_JOINED_STATE;
            MTRACE(macTrace(pMac, TRACE_CODE_MLM_STATE, psessionEntry->peSessionId, eLIM_MLM_JOINED_STATE));
            pMlmAuthReq->sessionId = psessionEntry->peSessionId;
            psessionEntry->limPrevSmeState = psessionEntry->limSmeState;
            psessionEntry->limSmeState     = eLIM_SME_WT_AUTH_STATE;
            //                                                         
            psessionEntry->staId = pAddBssParams->staContext.staIdx;

            MTRACE(macTrace(pMac, TRACE_CODE_SME_STATE, psessionEntry->peSessionId, psessionEntry->limSmeState));
            limPostMlmMessage(pMac,
                              LIM_MLM_AUTH_REQ,
                              (tANI_U32 *) pMlmAuthReq);
            return;
    }

joinFailure:
    {
        psessionEntry->limSmeState = eLIM_SME_JOIN_FAILURE_STATE;
        MTRACE(macTrace(pMac, TRACE_CODE_SME_STATE, psessionEntry->peSessionId, psessionEntry->limSmeState));

        //                            
        limHandleSmeJoinResult(pMac,  eSIR_SME_REFUSED, eSIR_MAC_UNSPEC_FAILURE_STATUS, psessionEntry);

    }

}

#ifdef WLAN_FEATURE_VOWIFI_11R
/*                                                                                          
  
                                                           
  
  
                                                                                            
 */
static inline void
limProcessStaMlmAddBssRspFT(tpAniSirGlobal pMac, tpSirMsgQ limMsgQ, tpPESession psessionEntry)
{
    tpDphHashNode pStaDs    = NULL;
    tpAddBssParams pAddBssParams = (tpAddBssParams) limMsgQ->bodyptr;
    tpAddStaParams pAddStaParams = NULL;
    tANI_U32 listenInterval = WNI_CFG_LISTEN_INTERVAL_STADEF;
    tLimMlmReassocReq       *pMlmReassocReq;
    tLimMlmReassocCnf       mlmReassocCnf; //          

    pMlmReassocReq = (tLimMlmReassocReq *)(psessionEntry->pLimMlmReassocReq);

    if ( eLIM_MLM_WT_ADD_BSS_RSP_FT_REASSOC_STATE != psessionEntry->limMlmState )
    {
        goto end;
    }

    if ((pStaDs = dphAddHashEntry(pMac, pAddBssParams->bssId, DPH_STA_HASH_INDEX_PEER,
        &psessionEntry->dph.dphHashTable)) == NULL)
    {
        //                               
        PELOGE(limLog(pMac, LOGE, FL("could not add hash entry at DPH for \n"));)
        limPrintMacAddr(pMac, pAddBssParams->staContext.staMac, LOGE);
        goto end;
    }

    //                                   
    if (limSetLinkState(pMac, eSIR_LINK_POSTASSOC_STATE,
            pAddBssParams->bssId, psessionEntry->selfMacAddr,
            NULL, NULL) != eSIR_SUCCESS)
    {
        PELOGE(limLog(pMac, LOGE,  FL("Failed to set the LinkState\n"));)
        goto end;
    }

    //                                             
    //                     
    pMac->lim.limTimers.gLimReassocFailureTimer.sessionId = psessionEntry->peSessionId;
    //                                   
    MTRACE(macTrace(pMac, TRACE_CODE_TIMER_ACTIVATE, psessionEntry->peSessionId, eLIM_REASSOC_FAIL_TIMER));
    if (tx_timer_activate(&pMac->lim.limTimers.gLimReassocFailureTimer)
                                               != TX_SUCCESS)
    {
        //                                        
        //          
        limLog(pMac, LOGP,
           FL("could not start Reassociation failure timer\n"));
        //                            
        //                      
        mlmReassocCnf.resultCode = eSIR_SME_RESOURCES_UNAVAILABLE;
        mlmReassocCnf.protStatusCode = eSIR_MAC_UNSPEC_FAILURE_STATUS;
        goto end;
    }
    limSendReassocReqWithFTIEsMgmtFrame(pMac, psessionEntry->pLimMlmReassocReq, psessionEntry);
    psessionEntry->limPrevMlmState = psessionEntry->limMlmState;
    psessionEntry->limMlmState = eLIM_MLM_WT_FT_REASSOC_RSP_STATE;
    PELOGE(limLog(pMac, LOGE,  FL("Set the mlm state to %d session=%d\n"),
        psessionEntry->limMlmState, psessionEntry->peSessionId);)

    psessionEntry->bssIdx     = (tANI_U8) pAddBssParams->bssIdx;

    //                     
    pStaDs->bssId = pAddBssParams->bssIdx;
    //                                                       
    pStaDs->staIndex = pAddBssParams->staContext.staIdx;
    pStaDs->ucUcastSig   = pAddBssParams->staContext.ucUcastSig;
    pStaDs->ucBcastSig   = pAddBssParams->staContext.ucBcastSig;

    //                                        
    limSetActiveEdcaParams(pMac, psessionEntry->gLimEdcaParams, psessionEntry);

    //                                       
    if (pStaDs->aniPeer == eANI_BOOLEAN_TRUE)
    {
        limSendEdcaParams(pMac, psessionEntry->gLimEdcaParamsActive, pStaDs->bssId, eANI_BOOLEAN_TRUE);
    }
    else
    {
        limSendEdcaParams(pMac, psessionEntry->gLimEdcaParamsActive, pStaDs->bssId, eANI_BOOLEAN_FALSE);
    }

#if defined WLAN_FEATURE_VOWIFI
    rrmCacheMgmtTxPower( pMac, pAddBssParams->txMgmtPower, psessionEntry );
#endif

    if( eHAL_STATUS_SUCCESS !=
        palAllocateMemory( pMac->hHdd, (void **) &pAddStaParams, sizeof( tAddStaParams )))
    {
        limLog( pMac, LOGP, FL( "Unable to PAL allocate memory during ADD_STA\n" ));
        return;
    }
    palZeroMemory( pMac->hHdd, (tANI_U8 *) pAddStaParams, sizeof(tAddStaParams));

    //                                            
    palCopyMemory( pMac->hHdd, (tANI_U8 *) pAddStaParams->staMac,
                  (tANI_U8 *) psessionEntry->selfMacAddr, sizeof(tSirMacAddr));

    palCopyMemory( pMac->hHdd, (tANI_U8 *) pAddStaParams->bssId,
                psessionEntry->bssId, sizeof(tSirMacAddr));

    //                                                    
    //                                                

    pAddStaParams->staType = STA_ENTRY_SELF;
    pAddStaParams->status = eHAL_STATUS_SUCCESS;
    pAddStaParams->respReqd = 1;

    /*                       */
    pAddStaParams->sessionId = psessionEntry->peSessionId;

    //                                                     
    pAddStaParams->staIdx = HAL_STA_INVALID_IDX;
    pAddStaParams->updateSta = FALSE;

    pAddStaParams->shortPreambleSupported = (tANI_U8)psessionEntry->beaconParams.fShortPreamble;
#ifdef WLAN_FEATURE_11AC
    limPopulateOwnRateSet(pMac, &pAddStaParams->supportedRates, NULL, false,psessionEntry, NULL);
#else
    limPopulateOwnRateSet(pMac, &pAddStaParams->supportedRates, NULL, false,psessionEntry);
#endif

    if( psessionEntry->htCapability)
    {
        pAddStaParams->htCapable = psessionEntry->htCapability;
#ifdef DISABLE_GF_FOR_INTEROP
        /*
                                                           
                                                               
                                                         
                                                
         */
        if( (psessionEntry->pLimJoinReq != NULL) && (!psessionEntry->pLimJoinReq->bssDescription.aniIndicator))
        {
            limLog( pMac, LOGE, FL(" Turning off Greenfield, when adding self entry"));
            pAddStaParams->greenFieldCapable = WNI_CFG_GREENFIELD_CAPABILITY_DISABLE;
        }
        else
#endif

#ifdef WLAN_SOFTAP_FEATURE
        pAddStaParams->greenFieldCapable = limGetHTCapability( pMac, eHT_GREENFIELD, psessionEntry);
        pAddStaParams->txChannelWidthSet = limGetHTCapability( pMac, eHT_SUPPORTED_CHANNEL_WIDTH_SET, psessionEntry);
        pAddStaParams->mimoPS            = limGetHTCapability( pMac, eHT_MIMO_POWER_SAVE, psessionEntry );
        pAddStaParams->rifsMode          = limGetHTCapability( pMac, eHT_RIFS_MODE, psessionEntry );
        pAddStaParams->lsigTxopProtection = limGetHTCapability( pMac, eHT_LSIG_TXOP_PROTECTION, psessionEntry );
        pAddStaParams->delBASupport      = limGetHTCapability( pMac, eHT_DELAYED_BA, psessionEntry );
        pAddStaParams->maxAmpduDensity   = limGetHTCapability( pMac, eHT_MPDU_DENSITY, psessionEntry );
        pAddStaParams->maxAmpduSize   = limGetHTCapability(pMac, eHT_MAX_RX_AMPDU_FACTOR, psessionEntry);
        pAddStaParams->maxAmsduSize      = limGetHTCapability( pMac, eHT_MAX_AMSDU_LENGTH, psessionEntry );
        pAddStaParams->fDsssCckMode40Mhz = limGetHTCapability( pMac, eHT_DSSS_CCK_MODE_40MHZ, psessionEntry);
        pAddStaParams->fShortGI20Mhz     = limGetHTCapability( pMac, eHT_SHORT_GI_20MHZ, psessionEntry);
        pAddStaParams->fShortGI40Mhz     = limGetHTCapability( pMac, eHT_SHORT_GI_40MHZ, psessionEntry);
#else
        pAddStaParams->greenFieldCapable = limGetHTCapability( pMac, eHT_GREENFIELD );
        pAddStaParams->txChannelWidthSet = limGetHTCapability( pMac, eHT_SUPPORTED_CHANNEL_WIDTH_SET );
        pAddStaParams->mimoPS            = limGetHTCapability( pMac, eHT_MIMO_POWER_SAVE );
        pAddStaParams->rifsMode          = limGetHTCapability( pMac, eHT_RIFS_MODE );
        pAddStaParams->lsigTxopProtection = limGetHTCapability( pMac, eHT_LSIG_TXOP_PROTECTION );
        pAddStaParams->delBASupport      = limGetHTCapability( pMac, eHT_DELAYED_BA );
        pAddStaParams->maxAmpduDensity   = limGetHTCapability( pMac, eHT_MPDU_DENSITY );
        pAddStaParams->maxAmpduSize   = limGetHTCapability(pMac, eHT_MAX_RX_AMPDU_FACTOR);
        pAddStaParams->maxAmsduSize      = limGetHTCapability( pMac, eHT_MAX_AMSDU_LENGTH );
        pAddStaParams->fDsssCckMode40Mhz = limGetHTCapability( pMac, eHT_DSSS_CCK_MODE_40MHZ);
        pAddStaParams->fShortGI20Mhz     = limGetHTCapability( pMac, eHT_SHORT_GI_20MHZ);
        pAddStaParams->fShortGI40Mhz     = limGetHTCapability( pMac, eHT_SHORT_GI_40MHZ);
#endif
    }

    if (wlan_cfgGetInt(pMac, WNI_CFG_LISTEN_INTERVAL, &listenInterval) != eSIR_SUCCESS)
        limLog(pMac, LOGP, FL("Couldn't get LISTEN_INTERVAL\n"));
    pAddStaParams->listenInterval = (tANI_U16)listenInterval;

    limFillSupportedRatesInfo(pMac, NULL, &pAddStaParams->supportedRates,psessionEntry);

    //                                                   
    pMac->ft.ftPEContext.pAddStaReq = pAddStaParams;
    return;

end:
    //                                        
    if (pMlmReassocReq != NULL)
    {
        palFreeMemory( pMac->hHdd, (tANI_U8 *) pMlmReassocReq);
    }
    mlmReassocCnf.resultCode = eSIR_SME_FT_REASSOC_FAILURE;
    mlmReassocCnf.protStatusCode = eSIR_MAC_UNSPEC_FAILURE_STATUS;
    /*                    */
    mlmReassocCnf.sessionId = psessionEntry->peSessionId;

    limPostSmeMessage(pMac, LIM_MLM_REASSOC_CNF, (tANI_U32 *) &mlmReassocCnf);
}
#endif /*                         */


/* 
                              
  
           
                                                                 
                                                
                                            
                                                               
  
        
                                       
                                             
                                        
  
              
                                                                 
                                                  
  
       
  
                                                    
                                                                        
  
               
 */
static void
limProcessStaMlmAddBssRsp( tpAniSirGlobal pMac, tpSirMsgQ limMsgQ,tpPESession psessionEntry)
{
    tpAddBssParams pAddBssParams = (tpAddBssParams) limMsgQ->bodyptr;
    tLimMlmAssocCnf mlmAssocCnf;
    tANI_U32 mesgType       = LIM_MLM_ASSOC_CNF;
    tANI_U32 subType        = LIM_ASSOC;
    tpDphHashNode pStaDs    = NULL;
    tANI_U16 staIdx = HAL_STA_INVALID_IDX;
    tANI_U8 updateSta = false;
    mlmAssocCnf.resultCode  = eSIR_SME_SUCCESS;

    if(eLIM_MLM_WT_ADD_BSS_RSP_PREASSOC_STATE == psessionEntry->limMlmState)
    {
        //                                                               
        limProcessStaMlmAddBssRspPreAssoc(pMac, limMsgQ, psessionEntry);
        goto end;
    }
    if( eLIM_MLM_WT_ADD_BSS_RSP_REASSOC_STATE == psessionEntry->limMlmState )
    {
        mesgType = LIM_MLM_REASSOC_CNF;
        subType = LIM_REASSOC;
     //                                                                                         
     //                                 
     //                                                                                                   
     //                                                                                                     
    if (sirCompareMacAddr( psessionEntry->bssId, psessionEntry->limReAssocbssId))
        {
            staIdx = psessionEntry->staId;
            updateSta  = true;
        }
    }
    if( eHAL_STATUS_SUCCESS == pAddBssParams->status )
    {
#if defined(WLAN_FEATURE_VOWIFI_11R) || defined(FEATURE_WLAN_CCX) || defined(FEATURE_WLAN_LFR)
        if( eLIM_MLM_WT_ADD_BSS_RSP_FT_REASSOC_STATE == psessionEntry->limMlmState )
        {
#ifdef WLAN_FEATURE_VOWIFI_11R_DEBUG
            PELOGE(limLog(pMac, LOGE, FL("Mlm=%d %d\n"),
                psessionEntry->limMlmState,
                eLIM_MLM_WT_ADD_BSS_RSP_REASSOC_STATE);)
#endif
            limProcessStaMlmAddBssRspFT( pMac, limMsgQ, psessionEntry);
            goto end;
        }
#endif /*                         */

         //               
        psessionEntry->limMlmState = eLIM_MLM_WT_ADD_STA_RSP_STATE;
        MTRACE(macTrace(pMac, TRACE_CODE_MLM_STATE, psessionEntry->peSessionId, psessionEntry->limMlmState));
        psessionEntry->statypeForBss = STA_ENTRY_PEER; //                                                         
        //                          
        limLog( pMac, LOGW, FL( "On STA: ADD_BSS was successful\n" ));
        pStaDs = dphGetHashEntry(pMac, DPH_STA_HASH_INDEX_PEER, &psessionEntry->dph.dphHashTable);
        if (pStaDs == NULL)
        {
            PELOGE(limLog(pMac, LOGE, FL("could not Add Self Entry for the station\n"));)
            mlmAssocCnf.resultCode = (tSirResultCodes) eSIR_SME_REFUSED;
        }
        else
        {
            psessionEntry->bssIdx     = (tANI_U8) pAddBssParams->bssIdx;
            //                     
            pStaDs->bssId = pAddBssParams->bssIdx;
            //                                                       
            pStaDs->staIndex = pAddBssParams->staContext.staIdx;
            pStaDs->ucUcastSig   = pAddBssParams->staContext.ucUcastSig;
            pStaDs->ucBcastSig   = pAddBssParams->staContext.ucBcastSig;
            //                                        
            limSetActiveEdcaParams(pMac, psessionEntry->gLimEdcaParams, psessionEntry);
            //                                       
            if (pStaDs->aniPeer == eANI_BOOLEAN_TRUE) {
                limSendEdcaParams(pMac, psessionEntry->gLimEdcaParamsActive, pStaDs->bssId, eANI_BOOLEAN_TRUE);
            } else {
                limSendEdcaParams(pMac, psessionEntry->gLimEdcaParamsActive, pStaDs->bssId, eANI_BOOLEAN_FALSE);
            }
#if defined WLAN_FEATURE_VOWIFI
            rrmCacheMgmtTxPower( pMac, pAddBssParams->txMgmtPower, psessionEntry );
#endif

            if (subType == LIM_REASSOC)
                limDeactivateAndChangeTimer(pMac, eLIM_KEEPALIVE_TIMER);
            if (limAddStaSelf(pMac,staIdx, updateSta, psessionEntry) != eSIR_SUCCESS)
            {
                //                      
                PELOGE(limLog(pMac, LOGE, FL("could not Add Self Entry for the station\n"));)
                mlmAssocCnf.resultCode = (tSirResultCodes) eSIR_SME_REFUSED;
            }
        }
    }
    else
    {
        limLog( pMac, LOGP, FL( "ADD_BSS failed!\n" ));
        //                                         
        mlmAssocCnf.resultCode = (tSirResultCodes) eSIR_SME_REFUSED;
    }

    if(mlmAssocCnf.resultCode != eSIR_SME_SUCCESS)
    {
        /*                     */
        mlmAssocCnf.sessionId = psessionEntry->peSessionId;
        limPostSmeMessage( pMac, mesgType, (tANI_U32 *) &mlmAssocCnf );
    }
    end:
    if( 0 != limMsgQ->bodyptr )
        palFreeMemory( pMac->hHdd,(void *) pAddBssParams );
}



/* 
                           
  
           
                                                                 
                                                
                                                              
                                            
  
        
                                                           
                               
                                                    
                                                
                                                              
                       
  
              
  
       
  
                                                    
                                                                        
  
               
 */
void limProcessMlmAddBssRsp( tpAniSirGlobal pMac, tpSirMsgQ limMsgQ )
{
    tLimMlmStartCnf     mlmStartCnf;
    tpPESession         psessionEntry;
    tpAddBssParams      pAddBssParams = (tpAddBssParams) (limMsgQ->bodyptr);

    if(NULL == pAddBssParams )
    {
        limLog( pMac, LOGE, FL( "Encountered NULL Pointer\n" ));
        return;
    }

    //
    //                  
    //                                                            
    //
    //                                                                                                
    //                                                                                                     
    //                  
    SET_LIM_PROCESS_DEFD_MESGS(pMac, true);
    //                       
    //                    
    if((psessionEntry = peFindSessionBySessionId(pMac,pAddBssParams->sessionId))== NULL)
    {
        limLog( pMac, LOGE, FL( "Session Does not exist for given sessionId\n" ));
            if( NULL != pAddBssParams )
                palFreeMemory( pMac->hHdd, (void *) pAddBssParams );
        return;
    }
    /*                     */
    mlmStartCnf.sessionId = psessionEntry->peSessionId;
    if( eSIR_IBSS_MODE == psessionEntry->bssType )
        limProcessIbssMlmAddBssRsp( pMac, limMsgQ, psessionEntry );
    else
    {
        if( eLIM_SME_WT_START_BSS_STATE == psessionEntry->limSmeState )
        {
            if( eLIM_MLM_WT_ADD_BSS_RSP_STATE != psessionEntry->limMlmState )
            {
                //                                         
                limLog( pMac, LOGE,
                  FL( "Received unexpected WDA_ADD_BSS_RSP in state %X\n" ),
                  psessionEntry->limMlmState );
                mlmStartCnf.resultCode = eSIR_SME_BSS_ALREADY_STARTED_OR_JOINED;
                if( 0 != limMsgQ->bodyptr )
                    palFreeMemory( pMac->hHdd, (void *) pAddBssParams );
                limPostSmeMessage( pMac, LIM_MLM_START_CNF, (tANI_U32 *) &mlmStartCnf );
            }
            else if ((psessionEntry->bssType == eSIR_BTAMP_AP_MODE)||(psessionEntry->bssType == eSIR_BTAMP_STA_MODE))
            {
                limProcessBtampAddBssRsp(pMac,limMsgQ,psessionEntry);
            }
            else
            limProcessApMlmAddBssRsp( pMac,limMsgQ);
        }
        else
            /*                                        */
            limProcessStaMlmAddBssRsp( pMac, limMsgQ,psessionEntry);
    }
}
/* 
                           
  
           
                                                       
                     
                        
                        
                              
                                         
         
                                                              
                                            
  
        
                                               
                                                 
                                                    
                                                    
                                                  
                                                        
                       
  
              
                                                           
                                                       
                                                            
  
       
  
                                                    
                                                                        
  
               
 */
void limProcessMlmSetStaKeyRsp( tpAniSirGlobal pMac, tpSirMsgQ limMsgQ )
{
    tANI_U8           respReqd = 1;
    tLimMlmSetKeysCnf mlmSetKeysCnf;
    tANI_U8  sessionId = 0;
    tpPESession  psessionEntry;
    SET_LIM_PROCESS_DEFD_MESGS(pMac, true);
    palZeroMemory( pMac->hHdd, (void *)&mlmSetKeysCnf, sizeof( tLimMlmSetKeysCnf ));
   //     
    if( NULL == limMsgQ->bodyptr )
    {
        PELOGE(limLog(pMac, LOGE,FL("limMsgQ bodyptr is NULL\n"));)
        return;
    }
    sessionId = ((tpSetStaKeyParams) limMsgQ->bodyptr)->sessionId;
    if((psessionEntry = peFindSessionBySessionId(pMac, sessionId))== NULL)
    {
        PELOGE(limLog(pMac, LOGE,FL("session does not exist for given sessionId\n"));)
        palFreeMemory( pMac->hHdd, (void *) limMsgQ->bodyptr );
        return;
    }
    if( eLIM_MLM_WT_SET_STA_KEY_STATE != psessionEntry->limMlmState )
    {
        //                                         
        limLog( pMac, LOGW, FL( "Received unexpected [Mesg Id - %d] in state %X\n" ), limMsgQ->type, psessionEntry->limMlmState );
        //                                                   
        respReqd = 0;
    }
    else
      mlmSetKeysCnf.resultCode = (tANI_U16) (((tpSetStaKeyParams) limMsgQ->bodyptr)->status);

    palFreeMemory( pMac->hHdd, (void *) limMsgQ->bodyptr );
    //                   
    psessionEntry->limMlmState = psessionEntry->limPrevMlmState;
    MTRACE(macTrace(pMac, TRACE_CODE_MLM_STATE, psessionEntry->peSessionId, psessionEntry->limMlmState));
    if( respReqd )
    {
        tpLimMlmSetKeysReq lpLimMlmSetKeysReq = (tpLimMlmSetKeysReq) pMac->lim.gpLimMlmSetKeysReq;
        //                                     
        if( NULL != lpLimMlmSetKeysReq )
        {
            palCopyMemory( pMac->hHdd, (tANI_U8 *) &mlmSetKeysCnf.peerMacAddr, (tANI_U8 *) lpLimMlmSetKeysReq->peerMacAddr, sizeof(tSirMacAddr) );
#ifdef ANI_PRODUCT_TYPE_AP
            mlmSetKeysCnf.aid = lpLimMlmSetKeysReq->aid;
#endif
            // Free the buffer cached for the global pMac->lim.gpLimMlmSetKeysReq
            palFreeMemory(pMac->hHdd, (tANI_U8 *) pMac->lim.gpLimMlmSetKeysReq);
            pMac->lim.gpLimMlmSetKeysReq = NULL;
        }
        mlmSetKeysCnf.sessionId = sessionId;
        limPostSmeMessage(pMac, LIM_MLM_SETKEYS_CNF, (tANI_U32 *) &mlmSetKeysCnf);
    }
}
void limProcessMlmSetBssKeyRsp( tpAniSirGlobal pMac, tpSirMsgQ limMsgQ )
{
    tANI_U8 respReqd = 1;
    tLimMlmSetKeysCnf mlmSetKeysCnf;
    tANI_U16          resultCode;
    tANI_U8           sessionId = 0;
    tpPESession  psessionEntry;
    SET_LIM_PROCESS_DEFD_MESGS(pMac, true);
    palZeroMemory( pMac->hHdd, (void *)&mlmSetKeysCnf, sizeof( tLimMlmSetKeysCnf ));
   //     
    if( NULL == limMsgQ->bodyptr )
    {
        PELOGE(limLog(pMac, LOGE,FL("limMsgQ bodyptr is null\n"));)
        return;
    }
    sessionId = ((tpSetBssKeyParams) limMsgQ->bodyptr)->sessionId;
    if((psessionEntry = peFindSessionBySessionId(pMac, sessionId))== NULL)
    {
        PELOGE(limLog(pMac, LOGE,FL("session does not exist for given sessionId\n"));)
        palFreeMemory( pMac->hHdd, (void *) limMsgQ->bodyptr );
        return;
    }
    if( eLIM_MLM_WT_SET_BSS_KEY_STATE == psessionEntry->limMlmState )
        resultCode = (tANI_U16) (((tpSetBssKeyParams) limMsgQ->bodyptr)->status);
    else
        resultCode = (tANI_U16) (((tpSetStaKeyParams) limMsgQ->bodyptr)->status); //                                                                    

    //
    //                  
    //                                                            
    //
  //                                                      
  //                                                       
  // 
    //                    
    if( eLIM_MLM_WT_SET_BSS_KEY_STATE != psessionEntry->limMlmState &&
        eLIM_MLM_WT_SET_STA_BCASTKEY_STATE != psessionEntry->limMlmState )
    {
        //                                         
        limLog( pMac, LOGW, FL( "Received unexpected [Mesg Id - %d] in state %X\n" ), limMsgQ->type, psessionEntry->limMlmState );
        //                                                   
        respReqd = 0;
    }
    else
      mlmSetKeysCnf.resultCode = resultCode;

    palFreeMemory( pMac->hHdd, (void *) limMsgQ->bodyptr );
    //                   
    psessionEntry->limMlmState = psessionEntry->limPrevMlmState;

    MTRACE(macTrace(pMac, TRACE_CODE_MLM_STATE, psessionEntry->peSessionId, psessionEntry->limMlmState));
    if( respReqd )
    {
        tpLimMlmSetKeysReq lpLimMlmSetKeysReq = (tpLimMlmSetKeysReq) pMac->lim.gpLimMlmSetKeysReq;
        mlmSetKeysCnf.sessionId = sessionId;

        //                                     
        if( NULL != lpLimMlmSetKeysReq )
        {
            palCopyMemory( pMac->hHdd, (tANI_U8 *) &mlmSetKeysCnf.peerMacAddr, (tANI_U8 *) lpLimMlmSetKeysReq->peerMacAddr, sizeof(tSirMacAddr) );
#ifdef ANI_PRODUCT_TYPE_AP
            mlmSetKeysCnf.aid = lpLimMlmSetKeysReq->aid;
#endif
            // Free the buffer cached for the global pMac->lim.gpLimMlmSetKeysReq
            palFreeMemory(pMac->hHdd, (tANI_U8 *) pMac->lim.gpLimMlmSetKeysReq);
            pMac->lim.gpLimMlmSetKeysReq = NULL;
        }
        limPostSmeMessage(pMac, LIM_MLM_SETKEYS_CNF, (tANI_U32 *) &mlmSetKeysCnf);
    }
}
/* 
                              
  
           
  
        
  
              
  
       
  
                                                    
                                                                        
  
               
 */
void limProcessMlmRemoveKeyRsp( tpAniSirGlobal pMac, tpSirMsgQ limMsgQ )
{
    tANI_U8 respReqd = 1;
    tLimMlmRemoveKeyCnf mlmRemoveCnf;
    tANI_U16             resultCode;
    tANI_U8              sessionId = 0;
    tpPESession  psessionEntry;
    SET_LIM_PROCESS_DEFD_MESGS(pMac, true);
    palZeroMemory( pMac->hHdd, (void *) &mlmRemoveCnf, sizeof( tLimMlmRemoveKeyCnf ));

    if( NULL == limMsgQ->bodyptr )
    {
        PELOGE(limLog(pMac, LOGE,FL("limMsgQ bodyptr is NULL\n"));)
        return;
    }

    if (limMsgQ->type == WDA_REMOVE_STAKEY_RSP)
        sessionId = ((tpRemoveStaKeyParams) limMsgQ->bodyptr)->sessionId;
    else if (limMsgQ->type == WDA_REMOVE_BSSKEY_RSP)
        sessionId = ((tpRemoveBssKeyParams) limMsgQ->bodyptr)->sessionId;

    if((psessionEntry = peFindSessionBySessionId(pMac, sessionId))== NULL)
    {
        PELOGE(limLog(pMac, LOGE,FL("session does not exist for given sessionId\n"));)
        return;
    }

    if( eLIM_MLM_WT_REMOVE_BSS_KEY_STATE == psessionEntry->limMlmState )
      resultCode = (tANI_U16) (((tpRemoveBssKeyParams) limMsgQ->bodyptr)->status);
    else
      resultCode = (tANI_U16) (((tpRemoveStaKeyParams) limMsgQ->bodyptr)->status);

    //                    
    if( eLIM_MLM_WT_REMOVE_BSS_KEY_STATE != psessionEntry->limMlmState &&
        eLIM_MLM_WT_REMOVE_STA_KEY_STATE != psessionEntry->limMlmState )
    {
        //                                         
        limLog(pMac, LOGW,
            FL("Received unexpected [Mesg Id - %d] in state %X\n"),
          limMsgQ->type,
          psessionEntry->limMlmState );
          respReqd = 0;
    }
    else
        mlmRemoveCnf.resultCode = resultCode;

    //
    //                  
    //                                                            
    //

    palFreeMemory( pMac->hHdd, (void *) limMsgQ->bodyptr );

    //                   
    psessionEntry->limMlmState = psessionEntry->limPrevMlmState;
    MTRACE(macTrace(pMac, TRACE_CODE_MLM_STATE, psessionEntry->peSessionId, psessionEntry->limMlmState));

    if( respReqd )
    {
        tpLimMlmRemoveKeyReq lpLimMlmRemoveKeyReq = (tpLimMlmRemoveKeyReq) pMac->lim.gpLimMlmRemoveKeyReq;
        mlmRemoveCnf.sessionId = sessionId;

    //                                       
        if( NULL != lpLimMlmRemoveKeyReq )
    {
            palCopyMemory( pMac->hHdd, (tANI_U8 *) &mlmRemoveCnf.peerMacAddr, (tANI_U8 *) lpLimMlmRemoveKeyReq->peerMacAddr,
                      sizeof( tSirMacAddr ));
        // Free the buffer cached for the global pMac->lim.gpLimMlmRemoveKeyReq
        palFreeMemory(pMac->hHdd, (tANI_U8 *) pMac->lim.gpLimMlmRemoveKeyReq);
        pMac->lim.gpLimMlmRemoveKeyReq = NULL;
    }
        limPostSmeMessage( pMac, LIM_MLM_REMOVEKEY_CNF, (tANI_U32 *) &mlmRemoveCnf );
    }
}

#if (defined(ANI_PRODUCT_TYPE_AP) || defined(ANI_PRODUCT_TYPE_AP_SDK))
/*                                                     
                                  
                                                   
            
            
                                                     */
static void __limProcessFinishLearnRsp(tpAniSirGlobal pMac)
{
    PELOG2(limLog(pMac, LOG2, FL("System Role: %d\n"), pMac->lim.gLimSystemRole);)
    if (pMac->lim.gpLimMeasReq == NULL)
    {
        limRestorePreLearnState(pMac);
        return;
    }
    /* 
                                                                                
                                                                             
                                                                         
                                                                           
                          
                                                                             
                                                           
                                                                            
                                                     
      */
    if (pMac->lim.gpLimMeasReq->measControl.periodicMeasEnabled &&
                !pMac->lim.gLimMeasParams.isMeasIndTimerActive)
    {
#if 0 /*                                            */
        if (pMac->lim.gLimSystemRole == eLIM_UNKNOWN_ROLE)
        {
            limSendSmeMeasurementInd(pMac);
            limCleanupMeasResources(pMac);
            limRestorePreLearnState(pMac);
            return;
        }
        else
#endif
        {
#ifdef GEN6_TODO
            /*                                                                     
                                
             */
            pMac->lim.gLimMeasParams.measurementIndTimer.sessionId = sessionId;
#endif
            //                                               
            if (tx_timer_activate(
               &pMac->lim.gLimMeasParams.measurementIndTimer)
               != TX_SUCCESS)
            {
                limLog(pMac, LOGP, FL("could not start Meas IND timer\n"));
                return;
            }
            pMac->lim.gLimMeasParams.isMeasIndTimerActive = 1;
        }
    }
    if (pMac->lim.gLimMeasParams.nextLearnChannelId >=
            pMac->lim.gpLimMeasReq->channelList.numChannels - 1)
    {
        //                                             
        pMac->lim.gLimMeasParams.nextLearnChannelId = 0;
    }
    //                           
    limRestorePreLearnState(pMac);
    //                                  
    if (pMac->lim.gpLimMeasReq->measControl.periodicMeasEnabled)
        limReEnableLearnMode(pMac);
    return;
}
#endif

/*                                                                       
                              
                                                                    
                                                           
                                                               
                             
                             
             
                                                                          */
void limProcessInitScanRsp(tpAniSirGlobal pMac,  void *body)
{
    tpInitScanParams    pInitScanParam;
    eHalStatus          status;
#if defined(ANI_PRODUCT_TYPE_AP) && (WNI_POLARIS_FW_PACKAGE == ADVANCED)
    tANI_U8             channelNum;
#endif
    SET_LIM_PROCESS_DEFD_MESGS(pMac, true);
    pInitScanParam = (tpInitScanParams) body;
    status = pInitScanParam->status;
    palFreeMemory( pMac->hHdd, (char *)body);

    //                                       
    if( pMac->lim.abortScan && 
       (eLIM_HAL_INIT_SCAN_WAIT_STATE == pMac->lim.gLimHalScanState) )
    {
        limLog( pMac, LOGW, FL(" finish scan\n") );
        pMac->lim.abortScan = 0;
        limDeactivateAndChangeTimer(pMac, eLIM_MIN_CHANNEL_TIMER);
        limDeactivateAndChangeTimer(pMac, eLIM_MAX_CHANNEL_TIMER);
        //                                                       
        //                                                               
        peSetResumeChannel(pMac, 0, 0);
        limSendHalFinishScanReq(pMac, eLIM_HAL_FINISH_SCAN_WAIT_STATE);
    }
    switch(pMac->lim.gLimHalScanState)
    {
        case eLIM_HAL_INIT_SCAN_WAIT_STATE:
            if (status != (tANI_U32) eHAL_STATUS_SUCCESS)
            {
               PELOGW(limLog(pMac, LOGW, FL("InitScanRsp with failed status= %d\n"), status);)
               pMac->lim.gLimHalScanState = eLIM_HAL_IDLE_SCAN_STATE;
               pMac->lim.gLimNumOfConsecutiveBkgndScanFailure += 1;
               /*
                                                                                     
                                                                                       
                                                                                                               
                                                                                                  
                                                               
                */
               limCompleteMlmScan(pMac, eSIR_SME_HAL_SCAN_INIT_FAILED);
#if defined(ANI_PRODUCT_TYPE_AP) && (WNI_POLARIS_FW_PACKAGE == ADVANCED)
                /*                                                                     */
#if 0
               if (pMac->lim.gLimSystemRole == eLIM_STA_ROLE && pMac->lim.gpLimMeasReq != NULL)
                {
                    limRestorePreLearnState(pMac);
                    pMac->lim.gLimHalScanState = eLIM_HAL_IDLE_SCAN_STATE;
                    limReEnableLearnMode(pMac);
                }
#endif
#endif
                return;
            }
            else if (status == eHAL_STATUS_SUCCESS)
            {
                /*                                                        
                                                                        
                 */
                pMac->lim.gLimNumOfConsecutiveBkgndScanFailure = 0;
                pMac->lim.gLimNumOfBackgroundScanSuccess += 1;
            }
            limContinueChannelScan(pMac);
            break;
#if defined(ANI_PRODUCT_TYPE_AP) && (WNI_POLARIS_FW_PACKAGE == ADVANCED)
        case eLIM_HAL_INIT_LEARN_WAIT_STATE:
//                                                         
            {
                if (status != (tANI_U32) eHAL_STATUS_SUCCESS)
                {
                    limRestorePreLearnState(pMac);
                    pMac->lim.gLimHalScanState = eLIM_HAL_IDLE_SCAN_STATE;
                    limReEnableLearnMode(pMac);
                    return;
                }
                channelNum = limGetCurrentLearnChannel(pMac);
                limSendHalStartScanReq(pMac, channelNum, eLIM_HAL_START_LEARN_WAIT_STATE);
            }
            break;
#endif
//                         
        case eLIM_HAL_SUSPEND_LINK_WAIT_STATE:
            if( pMac->lim.gpLimSuspendCallback )
            {
               if( status == eHAL_STATUS_SUCCESS )
                  pMac->lim.gLimHalScanState = eLIM_HAL_SUSPEND_LINK_STATE;
               else
                  pMac->lim.gLimHalScanState = eLIM_HAL_IDLE_SCAN_STATE;

               pMac->lim.gpLimSuspendCallback( pMac, status, pMac->lim.gpLimSuspendData );
               pMac->lim.gpLimSuspendCallback = NULL;
               pMac->lim.gpLimSuspendData = NULL;
            }
            else
            {
               limLog( pMac, LOGP, "No suspend link callback set but station is in suspend state\n");
               return;
            }
            break;
//                             
        default:
            limLog(pMac, LOGW, FL("limProcessInitScanRsp: Rcvd InitScanRsp not in WAIT State, state %d\n"),
                   pMac->lim.gLimHalScanState);
            break;
    }
    return;
}
/* 
                                      
  
           
                                                                       
                                                 
  
        
  
              
     
  
       
     
  
                                                           
                                                       
                                                          
  
               
 */
static void limProcessSwitchChannelReAssocReq(tpAniSirGlobal pMac, tpPESession psessionEntry, eHalStatus status)
{
    tLimMlmReassocCnf       mlmReassocCnf;
    tLimMlmReassocReq       *pMlmReassocReq;
    pMlmReassocReq = (tLimMlmReassocReq *)(psessionEntry->pLimMlmReassocReq);
    if(pMlmReassocReq == NULL)
    {
        limLog(pMac, LOGP, FL("pLimMlmReassocReq does not exist for given switchChanSession\n"));
        mlmReassocCnf.resultCode = eSIR_SME_RESOURCES_UNAVAILABLE;
        goto end;
    }

    if(status != eHAL_STATUS_SUCCESS)
    {
        PELOGE(limLog(pMac, LOGE, FL("Change channel failed!!\n"));)
        mlmReassocCnf.resultCode = eSIR_SME_CHANNEL_SWITCH_FAIL;
        goto end;
    }
    //                                   
    MTRACE(macTrace(pMac, TRACE_CODE_TIMER_ACTIVATE, psessionEntry->peSessionId, eLIM_REASSOC_FAIL_TIMER));
    if (tx_timer_activate(&pMac->lim.limTimers.gLimReassocFailureTimer)
                                               != TX_SUCCESS)
    {
        //                                        
        //          
        limLog(pMac, LOGP,
           FL("could not start Reassociation failure timer\n"));
        //                            
        //                      
        mlmReassocCnf.resultCode = eSIR_SME_RESOURCES_UNAVAILABLE;
        goto end;
    }
    //                                              
    limSendReassocReqMgmtFrame(pMac, pMlmReassocReq, psessionEntry);
    return;
end:
    //                                        
    if(pMlmReassocReq != NULL)
    {
        /*                     */
        mlmReassocCnf.sessionId = pMlmReassocReq->sessionId;
        palFreeMemory( pMac->hHdd, (tANI_U8 *) pMlmReassocReq);
    }
    else
    {
        mlmReassocCnf.sessionId = 0;
    }

    mlmReassocCnf.protStatusCode = eSIR_MAC_UNSPEC_FAILURE_STATUS;
    /*                    */
    mlmReassocCnf.sessionId = psessionEntry->peSessionId;

    limPostSmeMessage(pMac, LIM_MLM_REASSOC_CNF, (tANI_U32 *) &mlmReassocCnf);
}
/* 
                                   
  
           
                                                                     
                                                 
  
        
  
              
     
  
       
     
  
                                                           
                                                       
                                                          
  
               
 */
static void limProcessSwitchChannelJoinReq(tpAniSirGlobal pMac, tpPESession psessionEntry, eHalStatus status)
{
    tANI_U32            val;
    tSirMacSSid         ssId;
    tLimMlmJoinCnf      mlmJoinCnf;
    if(status != eHAL_STATUS_SUCCESS)
    {
        PELOGE(limLog(pMac, LOGE, FL("Change channel failed!!\n"));)
        goto error;
    }

    if ( (NULL == psessionEntry ) || (NULL == psessionEntry->pLimMlmJoinReq) )
    {
        PELOGE(limLog(pMac, LOGE, FL("invalid pointer!!\n"));)
        goto error;
    }

   
    /*                                                                                                      */
    if(psessionEntry->bssType == eSIR_BTAMP_AP_MODE)
    {
        if (limSetLinkState(pMac, eSIR_LINK_BTAMP_PREASSOC_STATE, psessionEntry->bssId,
             psessionEntry->selfMacAddr, NULL, NULL) != eSIR_SUCCESS )
            goto error;
    }
    else
    {
        if(limSetLinkState(pMac, eSIR_LINK_PREASSOC_STATE, psessionEntry->bssId,
            psessionEntry->selfMacAddr, NULL, NULL) != eSIR_SUCCESS)
            goto error;
    }
    //                             
#if 0
    if (cfgSetStr(pMac, WNI_CFG_BSSID, pMac->lim.gpLimMlmJoinReq->bssDescription.bssId,
                  sizeof(tSirMacAddr))
        != eSIR_SUCCESS)
        limLog(pMac, LOGP, FL("could not update BSSID at CFG\n"));
#endif //                 
    //                                                                                         

    /*                                                               */
    if(wlan_cfgGetInt(pMac, WNI_CFG_TRIG_STA_BK_SCAN, &val) != eSIR_SUCCESS)
        limLog(pMac, LOGP, FL("failed to get WNI_CFG_TRIG_STA_BK_SCAN cfg value!\n"));
    pMac->lim.gLimTriggerBackgroundScanDuringQuietBss = (val) ? 1 : 0;
    //                                         
    limApplyConfiguration(pMac, psessionEntry);
    //                                          
#if 0
    if (cfgGetStr(pMac, WNI_CFG_SSID, ssId.ssId, &cfgLen) != eSIR_SUCCESS)
        limLog(pMac, LOGP, FL("could not retrive SSID\n"));
#endif //                 
    palCopyMemory( pMac->hHdd, ssId.ssId,
                          psessionEntry->ssId.ssId,
                          psessionEntry->ssId.length);
    ssId.length = psessionEntry->ssId.length;
    //                                  
    limSendProbeReqMgmtFrame( pMac, &ssId,
           psessionEntry->pLimMlmJoinReq->bssDescription.bssId, psessionEntry->currentOperChannel/*       */,
           psessionEntry->selfMacAddr, psessionEntry->dot11mode,
           psessionEntry->pLimJoinReq->addIEScan.length, psessionEntry->pLimJoinReq->addIEScan.addIEdata);

    //                                                                      
    MTRACE(macTrace(pMac, TRACE_CODE_TIMER_ACTIVATE, psessionEntry->peSessionId, eLIM_JOIN_FAIL_TIMER));
    if (tx_timer_activate(&pMac->lim.limTimers.gLimJoinFailureTimer) != TX_SUCCESS)
    {
        limLog(pMac, LOGP, FL("could not activate Join failure timer\n"));
        psessionEntry->limMlmState = psessionEntry->limPrevMlmState;
         MTRACE(macTrace(pMac, TRACE_CODE_MLM_STATE, psessionEntry->peSessionId, pMac->lim.gLimMlmState));
        //                         
        psessionEntry->pLimMlmJoinReq = NULL;
        goto error;
    }

    return;
error:  
    if(NULL != psessionEntry)
    {
        palFreeMemory( pMac->hHdd, (tANI_U8 *) (psessionEntry->pLimMlmJoinReq));
        psessionEntry->pLimMlmJoinReq = NULL;
        mlmJoinCnf.sessionId = psessionEntry->peSessionId;
    }
    else
    {
        mlmJoinCnf.sessionId = 0;
    }
    mlmJoinCnf.resultCode = eSIR_SME_RESOURCES_UNAVAILABLE;
    mlmJoinCnf.protStatusCode = eSIR_MAC_UNSPEC_FAILURE_STATUS;
    limPostSmeMessage(pMac, LIM_MLM_JOIN_CNF, (tANI_U32 *) &mlmJoinCnf);
}

/* 
                               
  
           
                                                                        
  
        
  
              
     
  
       
     
  
                                                    
                               
  
               
 */
void limProcessSwitchChannelRsp(tpAniSirGlobal pMac,  void *body)
{
    tpSwitchChannelParams pChnlParams = NULL;
    eHalStatus status;
    tANI_U16 channelChangeReasonCode;
    tANI_U8 peSessionId;
    tpPESession psessionEntry;
    //                                                                                                
    //                                                                                                     
    //                  
    SET_LIM_PROCESS_DEFD_MESGS(pMac, true);
    pChnlParams = (tpSwitchChannelParams) body;
    status = pChnlParams->status;
    peSessionId = pChnlParams->peSessionId;
    if((psessionEntry = peFindSessionBySessionId(pMac, peSessionId))== NULL)
    {
        palFreeMemory( pMac->hHdd, (tANI_U8 *)body);
        limLog(pMac, LOGP, FL("session does not exist for given sessionId\n"));
        return;
    }
#if defined WLAN_FEATURE_VOWIFI
    //                                                             
    //                                         
    rrmCacheMgmtTxPower( pMac, pChnlParams->txMgmtPower, psessionEntry );
#endif
    palFreeMemory( pMac->hHdd, (tANI_U8 *)body);
    channelChangeReasonCode = psessionEntry->channelChangeReasonCode;
    //                                 
    psessionEntry->channelChangeReasonCode = 0xBAD;
    switch(channelChangeReasonCode)
    {
        case LIM_SWITCH_CHANNEL_REASSOC:
            limProcessSwitchChannelReAssocReq(pMac, psessionEntry, status);
            break;
        case LIM_SWITCH_CHANNEL_JOIN:
            limProcessSwitchChannelJoinReq(pMac, psessionEntry, status);
            break;

        case LIM_SWITCH_CHANNEL_OPERATION:
            /*
                                                           
                                                                
                                                                            
                                                      
             */
            if (pMac->lim.gpchangeChannelCallback)
            {
                PELOG1(limLog( pMac, LOG1, "Channel changed hence invoke registered call back\n");)
                pMac->lim.gpchangeChannelCallback(pMac, status, pMac->lim.gpchangeChannelData, psessionEntry);
            }
            break;
        default:
            break;
    }
}
/* 
                           
  
           
                                                                                                 
                                                          
  
        
  
              
     
  
       
     
  
                                                    
                               
  
               
 */

void limProcessStartScanRsp(tpAniSirGlobal pMac,  void *body)
{
    tpStartScanParams       pStartScanParam;
    eHalStatus              status;
    SET_LIM_PROCESS_DEFD_MESGS(pMac, true);
    pStartScanParam = (tpStartScanParams) body;
    status = pStartScanParam->status;
#if defined WLAN_FEATURE_VOWIFI
    //                                                             
    //                                         
    rrmCacheMgmtTxPower( pMac, pStartScanParam->txMgmtPower, NULL );
    //                                                                 
    rrmUpdateStartTSF( pMac, pStartScanParam->startTSF );
#endif
    palFreeMemory( pMac->hHdd, (tANI_U8 *)body);
    if( pMac->lim.abortScan )
    {
        limLog( pMac, LOGW, FL(" finish scan\n") );
        pMac->lim.abortScan = 0;
        limDeactivateAndChangeTimer(pMac, eLIM_MIN_CHANNEL_TIMER);
        limDeactivateAndChangeTimer(pMac, eLIM_MAX_CHANNEL_TIMER);
        //                                                       
        //                                                               
        peSetResumeChannel(pMac, 0, 0);
        limSendHalFinishScanReq(pMac, eLIM_HAL_FINISH_SCAN_WAIT_STATE);
    }
    switch(pMac->lim.gLimHalScanState)
    {
        case eLIM_HAL_START_SCAN_WAIT_STATE:
            if (status != (tANI_U32) eHAL_STATUS_SUCCESS)
            {
               PELOGW(limLog(pMac, LOGW, FL("StartScanRsp with failed status= %d\n"), status);)
               //
               //                                                       
               //                                               
               //                       
               //
               //                                                       
               //                                                               
               peSetResumeChannel(pMac, 0, 0);
               limSendHalFinishScanReq( pMac, eLIM_HAL_FINISH_SCAN_WAIT_STATE );
               //                                                        
            }
            else
            {
               pMac->lim.gLimHalScanState = eLIM_HAL_SCANNING_STATE;
               limContinuePostChannelScan(pMac);
            }
            break;
#if defined(ANI_PRODUCT_TYPE_AP) && (WNI_POLARIS_FW_PACKAGE == ADVANCED)
        case eLIM_HAL_START_LEARN_WAIT_STATE:
 //                                                        
            {
                if (status != (tANI_U32) eHAL_STATUS_SUCCESS)
                {
                    //                                                       
                    //                                                               
                    peSetResumeChannel(pMac, 0, 0);
                    limSendHalFinishScanReq(pMac, eLIM_HAL_FINISH_LEARN_WAIT_STATE);
                }
                else
                {
                    limContinueChannelLearn(pMac);
                }
            }
            break;
#endif
        default:
            limLog(pMac, LOGW, FL("Rcvd StartScanRsp not in WAIT State, state %d\n"),
                     pMac->lim.gLimHalScanState);
            break;
    }
    return;
}
void limProcessEndScanRsp(tpAniSirGlobal pMac,  void *body)
{
    tpEndScanParams     pEndScanParam;
    eHalStatus          status;
#if defined(ANI_PRODUCT_TYPE_AP) && (WNI_POLARIS_FW_PACKAGE == ADVANCED)
    tANI_U8             channelNum;
#endif
    SET_LIM_PROCESS_DEFD_MESGS(pMac, true);
    pEndScanParam = (tpEndScanParams) body;
    status = pEndScanParam->status;
    palFreeMemory( pMac->hHdd, (char *)body);
    switch(pMac->lim.gLimHalScanState)
    {
        case eLIM_HAL_END_SCAN_WAIT_STATE:
            if (status != (tANI_U32) eHAL_STATUS_SUCCESS)
            {
               PELOGW(limLog(pMac, LOGW, FL("EndScanRsp with failed status= %d\n"), status);)
               pMac->lim.gLimHalScanState = eLIM_HAL_IDLE_SCAN_STATE;
               limCompleteMlmScan(pMac, eSIR_SME_HAL_SCAN_INIT_FAILED);
            }
            else
            {
               //                                      
               //                                       
               if( ( pMac->lim.gpLimMlmScanReq != NULL ) &&
                   pMac->lim.gpLimMlmScanReq->p2pSearch &&
                   pMac->lim.gpLimMlmScanReq->skipDfsChnlInP2pSearch )
               {
                  int flag = 0;
                  while(!flag)
                  {
                     pMac->lim.gLimCurrentScanChannelId++;
                     if( (pMac->lim.gLimCurrentScanChannelId >
                   (tANI_U32) (pMac->lim.gpLimMlmScanReq->channelList.numChannels - 1))||
                        (limActiveScanAllowed(pMac, limGetCurrentScanChannel(pMac))))
                     {
                       flag=1; //                  
                     }
                  }
               }
               else
               {
                  pMac->lim.gLimCurrentScanChannelId++;
               }

               limContinueChannelScan(pMac);
            }
            break;
#if defined(ANI_PRODUCT_TYPE_AP) && (WNI_POLARIS_FW_PACKAGE == ADVANCED)
        case eLIM_HAL_END_LEARN_WAIT_STATE:
            if (status != (tANI_U32) eHAL_STATUS_SUCCESS)
            {
                //                                                       
                //                                                               
                peSetResumeChannel(pMac, 0, 0);
                limSendHalFinishScanReq(pMac, eLIM_HAL_FINISH_LEARN_WAIT_STATE);
            }
#if 0 /*                                                            */
            else if (pMac->lim.gLimSystemRole == eLIM_UNKNOWN_ROLE)
            {
               /*                                                                       
                                                  
                */
               channelNum = limGetCurrentLearnChannel(pMac);
               limSendHalStartScanReq(pMac, channelNum, eLIM_HAL_START_LEARN_WAIT_STATE);
            }
            else
#endif
            {
               limLog(pMac, LOGW, FL("ERROR! This state is set only when AP in UNKNOWN_ROLE\n"),
                        pMac->lim.gLimHalScanState);
                limSendHalFinishScanReq(pMac, eLIM_HAL_FINISH_LEARN_WAIT_STATE);
            }
            break;
#endif
        default:
            limLog(pMac, LOGW, FL("Rcvd endScanRsp not in WAIT State, state %d\n"),
                        pMac->lim.gLimHalScanState);
            break;
    }
    return;
}
/* 
                        
  
           
                                                                        
  
                                                                    
  
               
  
 */
static void
limStopTxAndSwitch (tpAniSirGlobal pMac)
{
    tANI_U8 i;

    for(i =0; i < pMac->lim.maxBssId; i++)
    {
        if(pMac->lim.gpSession[i].valid && 
            pMac->lim.gpSession[i].gLimSpecMgmt.dot11hChanSwState == eLIM_11H_CHANSW_RUNNING)
        {
            limStopTxAndSwitchChannel(pMac, i);
        }
    }
    return; 
}
/* 
                           
  
           
                                                                               
                                 
  
        
  
              
     
  
       
     
  
                                                    
  
               
 */
static void
limStartQuietOnSession (tpAniSirGlobal pMac)
{
    tANI_U8 i;

    for(i =0; i < pMac->lim.maxBssId; i++)
    {
        if(pMac->lim.gpSession[i].valid && 
            pMac->lim.gpSession[i].gLimSpecMgmt.quietState == eLIM_QUIET_BEGIN)
        {
            limStartQuietTimer(pMac, i);
        }
    }
    return;
}
void limProcessFinishScanRsp(tpAniSirGlobal pMac,  void *body)
{
    tpFinishScanParams      pFinishScanParam;
    eHalStatus              status;
    SET_LIM_PROCESS_DEFD_MESGS(pMac, true);
    pFinishScanParam = (tpFinishScanParams) body;
    status = pFinishScanParam->status;
    palFreeMemory( pMac->hHdd, (char *)body);
    switch(pMac->lim.gLimHalScanState)
    {
        case eLIM_HAL_FINISH_SCAN_WAIT_STATE:
            pMac->lim.gLimHalScanState = eLIM_HAL_IDLE_SCAN_STATE;
            limCompleteMlmScan(pMac, eSIR_SME_SUCCESS);
            if (limIsChanSwitchRunning(pMac))
            {
                /*                                                               */
                /*                                                                                
                                               
                */
                limStopTxAndSwitch(pMac);
            }
            else if (limIsQuietBegin(pMac))
            {
                /*                     */
                /*                                                                                
                                               
                */
                limStartQuietOnSession(pMac);
            }
#ifdef ANI_PRODUCT_TYPE_AP
            /*                                                                     */
#if 0
            if (pMac->lim.gLimSystemRole == eLIM_STA_ROLE && pMac->lim.gpLimMeasReq != NULL)
            {
                limSendSmeMeasurementInd(pMac);
                limCleanupMeasResources(pMac);
                limRestorePreLearnState(pMac);
            }
#endif
#endif
            if (status != (tANI_U32) eHAL_STATUS_SUCCESS)
            {
               PELOGW(limLog(pMac, LOGW, FL("EndScanRsp with failed status= %d\n"), status);)
            }
            break;
#if (defined(ANI_PRODUCT_TYPE_AP) || defined(ANI_PRODUCT_TYPE_AP_SDK))
        case eLIM_HAL_FINISH_LEARN_WAIT_STATE:
            pMac->lim.gLimHalScanState = eLIM_HAL_IDLE_SCAN_STATE;
            __limProcessFinishLearnRsp(pMac);
            break;
#endif //                                                                      
//                         
        case eLIM_HAL_RESUME_LINK_WAIT_STATE:
            if( pMac->lim.gpLimResumeCallback )
            {
               pMac->lim.gLimHalScanState = eLIM_HAL_IDLE_SCAN_STATE;
               pMac->lim.gpLimResumeCallback( pMac, status, pMac->lim.gpLimResumeData );
               pMac->lim.gpLimResumeCallback = NULL;
               pMac->lim.gpLimResumeData = NULL;
               pMac->lim.gLimSystemInScanLearnMode = 0;
            }
            else
            {
               limLog( pMac, LOGP, "No Resume link callback set but station is in suspend state\n");
               return;
            }
            break;
//                             

        default:
            limLog(pMac, LOGW, FL("Rcvd FinishScanRsp not in WAIT State, state %d\n"),
                        pMac->lim.gLimHalScanState);
            break;
    }
    return;
}
/* 
                                       
  
                                                    
  
  
                                               
  
                                                                 
  
               
 */
void limProcessMlmHalAddBARsp( tpAniSirGlobal pMac,
    tpSirMsgQ limMsgQ )
{
    //                              
    tpLimMlmAddBACnf pMlmAddBACnf;
    tpPESession     psessionEntry;
    tpAddBAParams pAddBAParams = (tpAddBAParams) limMsgQ->bodyptr;
#ifdef FEATURE_WLAN_DIAG_SUPPORT_LIM //                         
    limDiagEventReport(pMac, WLAN_PE_DIAG_HAL_ADDBA_RSP_EVENT, psessionEntry, 0, 0);
#endif //                         
    //                                      
    SET_LIM_PROCESS_DEFD_MESGS(pMac, true);
    if (pAddBAParams == NULL) {
        PELOGE(limLog(pMac, LOGE,FL("NULL ADD BA Response from HAL\n"));)
        return;
    }
    if((psessionEntry = peFindSessionBySessionId(pMac, pAddBAParams->sessionId))==NULL)
    {
        PELOGE(limLog(pMac, LOGE,FL("session does not exist for given sessionID: %d\n"),pAddBAParams->sessionId );)
        palFreeMemory(pMac->hHdd, (void*)limMsgQ->bodyptr);
        return;
    }
#ifdef FEATURE_WLAN_DIAG_SUPPORT_LIM //                          
    limDiagEventReport(pMac, WLAN_PE_DIAG_HAL_ADDBA_RSP_EVENT, psessionEntry, 0, 0);
#endif //                         

    //                               
    if( eHAL_STATUS_SUCCESS != palAllocateMemory( pMac->hHdd,
          (void **) &pMlmAddBACnf, sizeof( tLimMlmAddBACnf ))) {
        limLog( pMac, LOGP, FL(" palAllocateMemory failed with error code %d\n"));
        palFreeMemory(pMac->hHdd, (void*)limMsgQ->bodyptr);
        return;
    }
    palZeroMemory( pMac->hHdd, (void *) pMlmAddBACnf, sizeof( tLimMlmAddBACnf ));
     //                  
     palCopyMemory( pMac->hHdd, pMlmAddBACnf->peerMacAddr, pAddBAParams->peerMacAddr,
                                                                                 sizeof( tSirMacAddr ));
     //                                
     pMlmAddBACnf->baDialogToken = pAddBAParams->baDialogToken;
     pMlmAddBACnf->baTID = pAddBAParams->baTID;
     pMlmAddBACnf->baPolicy = pAddBAParams->baPolicy;
     pMlmAddBACnf->baBufferSize = pAddBAParams->baBufferSize;
     pMlmAddBACnf->baTimeout = pAddBAParams->baTimeout;
     pMlmAddBACnf->baDirection = pAddBAParams->baDirection;
     pMlmAddBACnf->sessionId = psessionEntry->peSessionId;
     if(eHAL_STATUS_SUCCESS == pAddBAParams->status)
        pMlmAddBACnf->addBAResultCode = eSIR_MAC_SUCCESS_STATUS;
     else
        pMlmAddBACnf->addBAResultCode = eSIR_MAC_UNSPEC_FAILURE_STATUS;
     palFreeMemory(pMac->hHdd, (void*)limMsgQ->bodyptr);
     //                      
     limPostSmeMessage( pMac, LIM_MLM_ADDBA_CNF, (tANI_U32 *) pMlmAddBACnf );
}
/* 
                                   
  
                            
  
                                               
  
                                                                 
  
               
 */
void limProcessMlmAddBACnf( tpAniSirGlobal pMac,
    tANI_U32 *pMsgBuf )
{
tpLimMlmAddBACnf pMlmAddBACnf;
tpDphHashNode pSta;
tANI_U16 aid;
tLimBAState curBaState;
tpPESession psessionEntry = NULL;
if(pMsgBuf == NULL)
{
    PELOGE(limLog(pMac, LOGE,FL("Buffer is Pointing to NULL\n"));)
    return;
}
pMlmAddBACnf = (tpLimMlmAddBACnf) pMsgBuf;
  if((psessionEntry = peFindSessionBySessionId(pMac,pMlmAddBACnf->sessionId))== NULL)
  {
        PELOGE(limLog(pMac, LOGE,FL("session does not exist for given BSSId\n"));)
        palFreeMemory( pMac->hHdd, (void *) pMsgBuf );
        return;
  }
  //                             
  pSta = dphLookupHashEntry( pMac, pMlmAddBACnf->peerMacAddr, &aid, &psessionEntry->dph.dphHashTable);
  if( NULL == pSta )
  {
    PELOGE(limLog( pMac, LOGE,
        FL( "STA context not found - ignoring ADDBA CNF from HAL\n" ));)
    palFreeMemory( pMac->hHdd, (void *) pMsgBuf );
    return;
  }
  LIM_GET_STA_BA_STATE(pSta, pMlmAddBACnf->baTID, &curBaState);
  //                           
  if( eLIM_BA_STATE_WT_ADD_RSP != curBaState)
  {
    PELOGE(limLog( pMac, LOGE,
        FL( "Received unexpected ADDBA CNF when STA BA state is %d\n" ),
        curBaState );)
      palFreeMemory( pMac->hHdd, (void *) pMsgBuf );
    return;
  }
  //                     
  LIM_SET_STA_BA_STATE(pSta, pMlmAddBACnf->baTID, eLIM_BA_STATE_IDLE);
  if( eSIR_MAC_SUCCESS_STATUS == pMlmAddBACnf->addBAResultCode )
  {
    //                             
    if( eBA_RECIPIENT == pMlmAddBACnf->baDirection )
    {
      pSta->tcCfg[pMlmAddBACnf->baTID].fUseBARx = 1;
      pSta->tcCfg[pMlmAddBACnf->baTID].fRxCompBA = 1;
      pSta->tcCfg[pMlmAddBACnf->baTID].fRxBApolicy = pMlmAddBACnf->baPolicy;
      pSta->tcCfg[pMlmAddBACnf->baTID].rxBufSize = pMlmAddBACnf->baBufferSize;
      pSta->tcCfg[pMlmAddBACnf->baTID].tuRxBAWaitTimeout = pMlmAddBACnf->baTimeout;
    }
    else
    {
      pSta->tcCfg[pMlmAddBACnf->baTID].fUseBATx = 1;
      pSta->tcCfg[pMlmAddBACnf->baTID].fTxCompBA = 1;
      pSta->tcCfg[pMlmAddBACnf->baTID].fTxBApolicy = pMlmAddBACnf->baPolicy;
      pSta->tcCfg[pMlmAddBACnf->baTID].txBufSize = pMlmAddBACnf->baBufferSize;
      pSta->tcCfg[pMlmAddBACnf->baTID].tuTxBAWaitTimeout = pMlmAddBACnf->baTimeout;
    }
  }
  if( eBA_RECIPIENT == pMlmAddBACnf->baDirection )
  {
    //
    //                                               
    //                                              
    //                                    
    //
    if( eSIR_SUCCESS != limPostMlmAddBARsp( pMac,
          pMlmAddBACnf->peerMacAddr,
          pMlmAddBACnf->addBAResultCode,
          pMlmAddBACnf->baDialogToken,
          (tANI_U8) pMlmAddBACnf->baTID,
          (tANI_U8) pMlmAddBACnf->baPolicy,
          pMlmAddBACnf->baBufferSize,
          pMlmAddBACnf->baTimeout,psessionEntry))
    {
      PELOGW(limLog( pMac, LOGW,
          FL( "Failed to post LIM_MLM_ADDBA_RSP to " ));
      limPrintMacAddr( pMac, pMlmAddBACnf->peerMacAddr, LOGW );)
    }
  }
  //                                                
  palFreeMemory( pMac->hHdd, (void *) pMsgBuf );
}
/* 
                                   
  
                            
  
                                               
  
                                                                 
  
               
 */
void limProcessMlmDelBACnf( tpAniSirGlobal pMac,
    tANI_U32 *pMsgBuf )
{
    tpLimMlmDelBACnf    pMlmDelBACnf;
    tpDphHashNode       pSta;
    tANI_U16            aid;
//                                  
    tLimBAState         curBaState;
    tpPESession         psessionEntry;

    if(pMsgBuf == NULL)
    {
         PELOGE(limLog(pMac, LOGE,FL("Buffer is Pointing to NULL\n"));)
         return;
    }
    pMlmDelBACnf = (tpLimMlmDelBACnf) pMsgBuf;
    if((psessionEntry = peFindSessionBySessionId(pMac, pMlmDelBACnf->sessionId))== NULL)
   {
        limLog(pMac, LOGP,FL("Session Does not exist for given sessionID\n"));
        palFreeMemory( pMac->hHdd, (void *) pMsgBuf );
        return;
   }
    //                             
    pSta = dphLookupHashEntry( pMac, pMlmDelBACnf->peerMacAddr, &aid, &psessionEntry->dph.dphHashTable );
    if( NULL == pSta )
    {
        limLog( pMac, LOGE,
            FL( "STA context not found - ignoring DELBA CNF from HAL\n" ));
        palFreeMemory( pMac->hHdd, (void *) pMsgBuf );
        return;
    }
    if(NULL == pMlmDelBACnf)
    {
        limLog( pMac, LOGE,
        FL( "pMlmDelBACnf is NULL - ignoring DELBA CNF from HAL\n" ));
        return;
    }
    //                         
    LIM_GET_STA_BA_STATE(pSta, pMlmDelBACnf->baTID, &curBaState);
    if( eLIM_BA_STATE_WT_DEL_RSP != curBaState )
    {
        limLog( pMac, LOGE,
        FL( "Received unexpected DELBA CNF when STA BA state is %d\n" ),
        curBaState );
        palFreeMemory( pMac->hHdd, (void *) pMsgBuf );
        return;
    }
    //                     
    LIM_SET_STA_BA_STATE(pSta, pMlmDelBACnf->baTID, eLIM_BA_STATE_IDLE);
    //                                                
    palFreeMemory( pMac->hHdd, (void *) pMsgBuf );
}
/* 
                                    
  
                                  
  
                                               
  
                                                                   
  
               
 */
void limProcessMlmHalBADeleteInd( tpAniSirGlobal pMac,
    tpSirMsgQ limMsgQ )
{
    tSirRetStatus       status = eSIR_SUCCESS;
    tpBADeleteParams    pBADeleteParams;
    tpDphHashNode       pSta;
    tANI_U16            aid;
    tLimBAState         curBaState;
    tpPESession         psessionEntry;
    tANI_U8             sessionId;

  pBADeleteParams = (tpBADeleteParams) limMsgQ->bodyptr;

    if((psessionEntry = peFindSessionByBssid(pMac,pBADeleteParams->bssId,&sessionId))== NULL)
    {
        PELOGE(limLog(pMac, LOGE,FL("session does not exist for given BSSId\n"));)
        palFreeMemory( pMac->hHdd, (void *) limMsgQ->bodyptr );
        return;
    }
    //                             
    pSta = dphLookupHashEntry( pMac, pBADeleteParams->peerMacAddr, &aid, &psessionEntry->dph.dphHashTable );
    if( NULL == pSta )
    {
        limLog( pMac, LOGE,
        FL( "STA context not found - ignoring BA Delete IND from HAL\n" ));
        goto returnAfterCleanup;
    }

  //                          
  LIM_GET_STA_BA_STATE(pSta, pBADeleteParams->baTID, &curBaState);
  if( eLIM_BA_STATE_IDLE != curBaState )
  {
    limLog( pMac, LOGE,
        FL( "Received unexpected BA Delete IND when STA BA state is %d\n" ),
        curBaState );
        goto returnAfterCleanup;
    }

  //                                                 
  //                              
  if( eBA_INITIATOR == pBADeleteParams->baDirection )
  {
    if( 0 == pSta->tcCfg[pBADeleteParams->baTID].fUseBATx )
      status = eSIR_FAILURE;
  }
  else
  {
    if( 0 == pSta->tcCfg[pBADeleteParams->baTID].fUseBARx )
      status = eSIR_FAILURE;
  }
    if( eSIR_FAILURE == status )
    {
        limLog( pMac, LOGW,
        FL("Received an INVALID DELBA Delete Ind for TID %d...\n"),
        pBADeleteParams->baTID );
    }
    else
    {
        //                          
        if( eSIR_SUCCESS !=
        (status = limPostMlmDelBAReq( pMac,
                                      pSta,
                                      pBADeleteParams->baDirection,
                                      pBADeleteParams->baTID,
                                      eSIR_MAC_UNSPEC_FAILURE_REASON,psessionEntry )))
        {
            limLog( pMac, LOGE,
            FL( "Attempt to post LIM_MLM_DELBA_REQ failed with status %d\n" ), status);
    }
    else
    {
      limLog( pMac, LOGE,
          FL( "BA Delete - Reason 0x%08x. Attempting to delete BA session for TID %d with peer STA "  ),
          pBADeleteParams->reasonCode, pBADeleteParams->baTID );
            limPrintMacAddr( pMac, pSta->staAddr, LOGE );
        }
  }
returnAfterCleanup:
  //                                                 
  palFreeMemory( pMac->hHdd, (void *) limMsgQ->bodyptr );
}
/* 
                                      
  
                                                                                        
                                                                   
  
              
  
                    
              
  
             
              
  
                                                 
                                                                             
               
 */

void
limProcessSetMimoRsp(tpAniSirGlobal pMac, tpSirMsgQ limMsg)
{
#if 0
    tSirRetStatus           retStatus;
    tpSetMIMOPS          pMIMO_PSParams;


    do {

        pMIMO_PSParams = (tpSetMIMOPS)limMsg->bodyptr;
        if( NULL == pMIMO_PSParams ) {
            PELOGE(limLog(pMac, LOGE, "Received the WDA_SET_MIMOPS_RSP with NULL as the PS param");)
            return;
        }

        /*                               */
        if (pMIMO_PSParams->status != eSIR_SUCCESS) {
            limLog(pMac, LOGP, FL("Update HAL / SW Mac for MIMO State has Failed\n"));
            break;
        }

        if ((pMac->lim.gLimSystemRole != eSYSTEM_STA_ROLE) ||
                (pMac->lim.gLimSmeState != eLIM_SME_LINK_EST_STATE) )
            break;

        pMac->lim.gLimMlmState = pMac->lim.gLimPrevMlmState;
        MTRACE(macTrace(pMac, TRACE_CODE_MLM_STATE, 0, pMac->lim.gLimMlmState));

        /*                                                                                               
                                                                                                                  
                                                                                            
          */
        if (!isEnteringMimoPS(pMac->lim.gHTMIMOPSState, pMIMO_PSParams->htMIMOPSState)) {
            tSirMacAddr            macAddr;

            /*                              */
            palCopyMemory(pMac -> hHdd, (tANI_U8 *)macAddr, pMac->lim.gLimBssid, sizeof(tSirMacAddr));

            /*                                                */
            retStatus = limSendSMPowerStateFrame(pMac, macAddr, pMIMO_PSParams->htMIMOPSState);
            if (retStatus != eSIR_SUCCESS) {
                PELOGE(limLog(pMac, LOGE, FL("Sending Action Frame has failed\n"));)
                break;
            }
        }
        PELOG1(limLog(pMac, LOG1, FL("The Setting up of LimGlobals is successful for MIMOPS"));)
    }while(0);

    palFreeMemory( pMac->hHdd, (void *) pMIMO_PSParams );
#endif
}
/* 
                                                   
                                                                               
                                                                                                
                                                                                          
   
                                                                                                       
                                                                                           
                                                                                                      
                                        
   
                                        
                                                   
   
                       
  */
static void
limHandleDelBssInReAssocContext(tpAniSirGlobal pMac, tpDphHashNode pStaDs,tpPESession psessionEntry)
{
    tLimMlmReassocCnf           mlmReassocCnf;
    /*                                                                */
    /*                          */
    psessionEntry->limMlmState = eLIM_MLM_IDLE_STATE;
    MTRACE(macTrace(pMac, TRACE_CODE_MLM_STATE, psessionEntry->peSessionId, psessionEntry->limMlmState));
   /*                     */
    mlmReassocCnf.sessionId = psessionEntry->peSessionId;
    switch (psessionEntry->limMlmState) {
#if defined(ANI_PRODUCT_TYPE_CLIENT) || defined(ANI_AP_CLIENT_SDK)
        case eLIM_SME_WT_REASSOC_STATE :
        {
            tpSirAssocRsp assocRsp;
            tpDphHashNode   pStaDs;
            tSirRetStatus       retStatus = eSIR_SUCCESS;
#ifdef ANI_PRODUCT_TYPE_CLIENT
            tSchBeaconStruct beaconStruct;
#endif
            /*                                   */
            limDeleteDphHashEntry(pMac, psessionEntry->bssId, DPH_STA_HASH_INDEX_PEER, psessionEntry);
       /* 
                                                
                                       
             */
            if ((pStaDs = dphAddHashEntry(pMac, psessionEntry->limReAssocbssId, DPH_STA_HASH_INDEX_PEER, &psessionEntry->dph.dphHashTable)) == NULL)
            {
                //                               
                PELOGE(limLog(pMac, LOGE, FL("could not add hash entry at DPH for \n"));)
                limPrintMacAddr(pMac, psessionEntry->limReAssocbssId, LOGE);
                mlmReassocCnf.resultCode = eSIR_SME_RESOURCES_UNAVAILABLE;
                mlmReassocCnf.protStatusCode = eSIR_SME_SUCCESS;
                goto Error;
            }
            /*                                                                  
                                                                  
            */
            assocRsp = (tpSirAssocRsp)psessionEntry->limAssocResponseData;
            limUpdateAssocStaDatas(pMac, pStaDs, assocRsp,psessionEntry);
            limUpdateReAssocGlobals(pMac, assocRsp,psessionEntry);
#ifdef ANI_PRODUCT_TYPE_CLIENT
            limExtractApCapabilities( pMac,
                  (tANI_U8 *) psessionEntry->pLimReAssocReq->bssDescription.ieFields,
                  limGetIElenFromBssDescription( &psessionEntry->pLimReAssocReq->bssDescription ),
                    &beaconStruct );
            if(pMac->lim.gLimProtectionControl != WNI_CFG_FORCE_POLICY_PROTECTION_DISABLE)
                limDecideStaProtectionOnAssoc(pMac, &beaconStruct, psessionEntry);
                if(beaconStruct.erpPresent) {
                if (beaconStruct.erpIEInfo.barkerPreambleMode)
                    psessionEntry->beaconParams.fShortPreamble = 0;
                else
                    psessionEntry->beaconParams.fShortPreamble = 1;
            }
            //                                                                                                          
            if (eSIR_SUCCESS != limStaSendAddBss( pMac, assocRsp, &beaconStruct,
                                                    &psessionEntry->pLimReAssocReq->bssDescription, false, psessionEntry))  {
                limLog( pMac, LOGE, FL( "Posting ADDBSS in the ReAssocContext has Failed \n"));
                retStatus = eSIR_FAILURE;
            }
#elif defined(ANI_AP_CLIENT_SDK)
            if (eSIR_SUCCESS != limStaSendAddBss( pMac, (*assocRsp), &psessionEntry->pLimReAssocReq->neighborBssList.bssList[0],
                                                  false, psessionEntry))  {
                limLog( pMac, LOGE, FL( "Posting ADDBSS in the ReAssocContext has Failed \n"));
                retStatus = eSIR_FAILURE;
            }
#endif
            if (retStatus != eSIR_SUCCESS)
            {
                mlmReassocCnf.resultCode = eSIR_SME_RESOURCES_UNAVAILABLE;
                mlmReassocCnf.protStatusCode = eSIR_MAC_UNSPEC_FAILURE_STATUS;
                palFreeMemory(pMac->hHdd, assocRsp);
                pMac->lim.gLimAssocResponseData = NULL;
                goto Error;
            }
            palFreeMemory(pMac->hHdd, assocRsp);
            psessionEntry->limAssocResponseData = NULL;
        }
        break;
        case eLIM_SME_WT_REASSOC_LINK_FAIL_STATE:
        {
            /*                                    
                                                      */
            /*                                                                      */
            mlmReassocCnf.resultCode = pStaDs->mlmStaContext.disassocReason;
            mlmReassocCnf.protStatusCode = pStaDs->mlmStaContext.cleanupTrigger;
            /*                                            */
            psessionEntry->limSmeState = eLIM_SME_WT_REASSOC_STATE;
            MTRACE(macTrace(pMac, TRACE_CODE_SME_STATE, psessionEntry->peSessionId, psessionEntry->limSmeState));
            limDeleteDphHashEntry(pMac, pStaDs->staAddr, pStaDs->assocId,psessionEntry);
            if((psessionEntry->limSystemRole == eLIM_STA_ROLE)||
                (psessionEntry->limSystemRole == eLIM_BT_AMP_STA_ROLE))
            {
               psessionEntry->limMlmState = eLIM_MLM_IDLE_STATE;
               MTRACE(macTrace(pMac, TRACE_CODE_MLM_STATE, psessionEntry->peSessionId, psessionEntry->limMlmState));
            }
            limPostSmeMessage(pMac, LIM_MLM_REASSOC_CNF, (tANI_U32 *) &mlmReassocCnf);
        }
        break;
#endif
        default:
            PELOGE(limLog(pMac, LOGE, FL("DelBss is being invoked in the wrong system Role /unhandled  SME State\n"));)
            mlmReassocCnf.resultCode = eSIR_SME_REFUSED;
            mlmReassocCnf.protStatusCode = eSIR_SME_UNEXPECTED_REQ_RESULT_CODE;
            goto Error;
    }
    return;
Error:
    limPostSmeMessage(pMac, LIM_MLM_REASSOC_CNF, (tANI_U32 *) &mlmReassocCnf);
}

/*                           */
static void
limProcessBtampAddBssRsp( tpAniSirGlobal pMac, tpSirMsgQ limMsgQ ,tpPESession psessionEntry)
{
    tLimMlmStartCnf mlmStartCnf;
    tANI_U32 val;
    tpAddBssParams pAddBssParams = (tpAddBssParams) limMsgQ->bodyptr;

    if( eHAL_STATUS_SUCCESS == pAddBssParams->status )
    {
        limLog(pMac, LOG2, FL("WDA_ADD_BSS_RSP returned with eHAL_STATUS_SUCCESS\n"));
         if (psessionEntry->bssType == eSIR_BTAMP_AP_MODE)
         {
             if (limSetLinkState(pMac, eSIR_LINK_BTAMP_AP_STATE, psessionEntry->bssId,
                  psessionEntry->selfMacAddr, NULL, NULL) != eSIR_SUCCESS )
               goto end;
         } else if (psessionEntry->bssType == eSIR_BTAMP_STA_MODE) {
            if (limSetLinkState(pMac, eSIR_LINK_SCAN_STATE, psessionEntry->bssId,
                 psessionEntry->selfMacAddr, NULL, NULL) != eSIR_SUCCESS )
                goto end;
         }

        //               
        psessionEntry->limMlmState= eLIM_MLM_BSS_STARTED_STATE;
        psessionEntry->statypeForBss = STA_ENTRY_SELF; //                                             
        psessionEntry->bssIdx = (tANI_U8) pAddBssParams->bssIdx;
        schEdcaProfileUpdate(pMac, psessionEntry);
        limInitAIDpool(pMac,psessionEntry);
        //                          
        if (!pMac->lim.gLimTimersCreated)
        limCreateTimers(pMac);
      /*                                                               */
        if( eSIR_SUCCESS != wlan_cfgGetInt( pMac, WNI_CFG_TRIG_STA_BK_SCAN, &val ))
            limLog( pMac, LOGP, FL("Failed to get WNI_CFG_TRIG_STA_BK_SCAN!\n"));
        pMac->lim.gLimTriggerBackgroundScanDuringQuietBss = (val) ? 1 : 0;
        //                                         
        limApplyConfiguration(pMac,psessionEntry);
        psessionEntry->staId = pAddBssParams->staContext.staIdx;
        mlmStartCnf.resultCode  = eSIR_SME_SUCCESS;
    }
    else
    {
        limLog( pMac, LOGE, FL( "WDA_ADD_BSS_REQ failed with status %d\n" ),pAddBssParams->status );
        mlmStartCnf.resultCode = eSIR_SME_HAL_SEND_MESSAGE_FAIL;
    }
    mlmStartCnf.sessionId = psessionEntry->peSessionId;
    limPostSmeMessage( pMac, LIM_MLM_START_CNF, (tANI_U32 *) &mlmStartCnf );
    end:
    if( 0 != limMsgQ->bodyptr )
        palFreeMemory( pMac->hHdd, (void *) pAddBssParams );
}

/* 
                                                   
                                                                               
                                                                                                
                                                                                          
   
                                                                                                       
                                                                                           
                                                                                                      
                                        
   
                                        
                                                   
   
                       
  */
void
limHandleAddBssInReAssocContext(tpAniSirGlobal pMac, tpDphHashNode pStaDs, tpPESession psessionEntry)
{
    tLimMlmReassocCnf           mlmReassocCnf;
    /*                                                                */
    /*                          */
    psessionEntry->limMlmState = eLIM_MLM_IDLE_STATE;
    MTRACE(macTrace(pMac, TRACE_CODE_MLM_STATE, psessionEntry->peSessionId, psessionEntry->limMlmState));
    switch (psessionEntry->limSmeState) {
#if defined(ANI_PRODUCT_TYPE_CLIENT) || defined(ANI_AP_CLIENT_SDK)
        case eLIM_SME_WT_REASSOC_STATE : {
            tpSirAssocRsp assocRsp;
            tpDphHashNode   pStaDs;
            tSirRetStatus       retStatus = eSIR_SUCCESS;
#ifdef ANI_PRODUCT_TYPE_CLIENT
            tSchBeaconStruct *pBeaconStruct;
            if(eHAL_STATUS_SUCCESS != palAllocateMemory(pMac->hHdd, 
                                                        (void **)&pBeaconStruct, sizeof(tSchBeaconStruct)))
            {
                limLog(pMac, LOGE, FL("Unable to PAL allocate memory in limHandleAddBssInReAssocContext\n") );
                mlmReassocCnf.resultCode = eSIR_SME_RESOURCES_UNAVAILABLE;
                mlmReassocCnf.protStatusCode = eSIR_SME_RESOURCES_UNAVAILABLE;
                goto Error;
            }

#endif
            //                                     
            pStaDs = dphGetHashEntry(pMac, DPH_STA_HASH_INDEX_PEER, &psessionEntry->dph.dphHashTable);
            if (pStaDs == NULL )
            {
                PELOGE(limLog(pMac, LOGE, FL("Fail to get STA PEER entry from hash\n"));)
                mlmReassocCnf.resultCode = eSIR_SME_RESOURCES_UNAVAILABLE;
                mlmReassocCnf.protStatusCode = eSIR_SME_SUCCESS;
                palFreeMemory(pMac->hHdd, pBeaconStruct);
                goto Error;
            }
            /*                                                                  
                                                                  
            */
            assocRsp = (tpSirAssocRsp)psessionEntry->limAssocResponseData;
            limUpdateAssocStaDatas(pMac, pStaDs, assocRsp, psessionEntry);
            limUpdateReAssocGlobals(pMac, assocRsp, psessionEntry);
#ifdef ANI_PRODUCT_TYPE_CLIENT
            limExtractApCapabilities( pMac,
                  (tANI_U8 *) psessionEntry->pLimReAssocReq->bssDescription.ieFields,
                  limGetIElenFromBssDescription( &psessionEntry->pLimReAssocReq->bssDescription ),
                    pBeaconStruct );
            if(pMac->lim.gLimProtectionControl != WNI_CFG_FORCE_POLICY_PROTECTION_DISABLE)
                limDecideStaProtectionOnAssoc(pMac, pBeaconStruct, psessionEntry);

            if(pBeaconStruct->erpPresent) 
            {
                if (pBeaconStruct->erpIEInfo.barkerPreambleMode)
                    psessionEntry->beaconParams.fShortPreamble = 0;
                else
                    psessionEntry->beaconParams.fShortPreamble = 1;
            }

            if (eSIR_SUCCESS != limStaSendAddBss( pMac, assocRsp, pBeaconStruct,
                                                    &psessionEntry->pLimReAssocReq->bssDescription, true, psessionEntry))  {
                limLog( pMac, LOGE, FL( "Posting ADDBSS in the ReAssocContext has Failed \n"));
                retStatus = eSIR_FAILURE;
            }
#elif defined(ANI_AP_CLIENT_SDK)
            if (eSIR_SUCCESS != limStaSendAddBss( pMac, (*assocRsp), &pMac->lim.gpLimReassocReq->neighborBssList.bssList[0], true))  {
                limLog( pMac, LOGE, FL( "Posting ADDBSS in the ReAssocContext has Failed \n"));
                retStatus = eSIR_FAILURE;
            }
#endif
            if (retStatus != eSIR_SUCCESS)
            {
                mlmReassocCnf.resultCode = eSIR_SME_RESOURCES_UNAVAILABLE;
                mlmReassocCnf.protStatusCode = eSIR_MAC_UNSPEC_FAILURE_STATUS;
                palFreeMemory(pMac->hHdd, assocRsp);
                pMac->lim.gLimAssocResponseData = NULL;
                palFreeMemory(pMac->hHdd, pBeaconStruct);
                goto Error;
            }
            palFreeMemory(pMac->hHdd, assocRsp);
            psessionEntry->limAssocResponseData = NULL;
            palFreeMemory(pMac->hHdd, pBeaconStruct);
        }
        break;
        case eLIM_SME_WT_REASSOC_LINK_FAIL_STATE: {     /*                                    
                                                                                                                              */
            /*                                                                      */
            mlmReassocCnf.resultCode = pStaDs->mlmStaContext.disassocReason;
            mlmReassocCnf.protStatusCode = pStaDs->mlmStaContext.cleanupTrigger;
            /*                                            */
            psessionEntry->limSmeState = eLIM_SME_WT_REASSOC_STATE;
            limDeleteDphHashEntry(pMac, pStaDs->staAddr, pStaDs->assocId, psessionEntry);
            if(psessionEntry->limSystemRole == eLIM_STA_ROLE)
              psessionEntry->limMlmState = eLIM_MLM_IDLE_STATE;

            limPostSmeMessage(pMac, LIM_MLM_REASSOC_CNF, (tANI_U32 *) &mlmReassocCnf);
        }
        break;
#endif
        default:
            PELOGE(limLog(pMac, LOGE, FL("DelBss is being invoked in the wrong system Role /unhandled  SME State\n"));)
            mlmReassocCnf.resultCode = eSIR_SME_REFUSED;
            mlmReassocCnf.protStatusCode = eSIR_SME_UNEXPECTED_REQ_RESULT_CODE;
            goto Error;
    }
return;
Error:
    limPostSmeMessage(pMac, LIM_MLM_REASSOC_CNF, (tANI_U32 *) &mlmReassocCnf);
}

#if 0
 static void
limProcessSmeAssocCnfNew(tpAniSirGlobal pMac, tANI_U32 msgType, tANI_U32 *pMsgBuf)
{
    tSirSmeAssocCnf    assocCnf;
    tpDphHashNode      pStaDs;
    tpPESession        psessionEntry;
    tANI_U8            sessionId;

    if(pMsgBuf == NULL)
    {
        limLog(pMac, LOGE, FL("pMsgBuf is NULL \n"));
        goto end;
    }
    if ((limAssocCnfSerDes(pMac, &assocCnf, (tANI_U8 *) pMsgBuf) == eSIR_FAILURE) ||
        !__limIsSmeAssocCnfValid(&assocCnf))
    {
        limLog(pMac, LOGE, FL("Received invalid SME_RE(ASSOC)_CNF message \n"));
        goto end;
    }
    if((psessionEntry = peFindSessionByBssid(pMac, assocCnf.bssId, &sessionId))== NULL)
    {
        limLog(pMac, LOGE, FL("session does not exist for given bssId\n"));
        goto end;
    }
    if ( ((psessionEntry->limSystemRole != eLIM_AP_ROLE) && (psessionEntry->limSystemRole != eLIM_BT_AMP_AP_ROLE)) ||
         ((psessionEntry->limSmeState != eLIM_SME_NORMAL_STATE) && (psessionEntry->limSmeState != eLIM_SME_NORMAL_CHANNEL_SCAN_STATE)))
    {
        limLog(pMac, LOGE, FL("Received unexpected message %X in state %X, in role %X\n"),
               msgType, psessionEntry->limSmeState , psessionEntry->limSystemRole);
        goto end;
    }
    pStaDs = dphGetHashEntry(pMac, assocCnf.aid, &psessionEntry->dph.dphHashTable);
    if (pStaDs == NULL)
    {
        limLog(pMac, LOG1,
            FL("Received invalid message %X due to no STA context, for aid %d, peer "),
            msgType, assocCnf.aid);
        limPrintMacAddr(pMac, assocCnf.peerMacAddr, LOG1);
        /*
                                                          
                                               
         */
       limSendSmeDisassocNtf( pMac, assocCnf.peerMacAddr, eSIR_SME_STA_NOT_ASSOCIATED,
                              eLIM_PEER_ENTITY_DISASSOC, assocCnf.aid,psessionEntry->smeSessionId,psessionEntry->transactionId,psessionEntry);
       goto end;
    }
    if ((pStaDs &&
         (( !palEqualMemory( pMac->hHdd,(tANI_U8 *) pStaDs->staAddr,
                     (tANI_U8 *) assocCnf.peerMacAddr,
                     sizeof(tSirMacAddr)) ) ||
          (pStaDs->mlmStaContext.mlmState != eLIM_MLM_WT_ASSOC_CNF_STATE) ||
          ((pStaDs->mlmStaContext.subType == LIM_ASSOC) &&
           (msgType != eWNI_SME_ASSOC_CNF)) ||
          ((pStaDs->mlmStaContext.subType == LIM_REASSOC) &&
           (msgType != eWNI_SME_REASSOC_CNF)))))
    {
        limLog(pMac, LOG1,
           FL("Received invalid message %X due to peerMacAddr mismatched or not in eLIM_MLM_WT_ASSOC_CNF_STATE state, for aid %d, peer "),
           msgType, assocCnf.aid);
        limPrintMacAddr(pMac, assocCnf.peerMacAddr, LOG1);
        goto end;
    }
    /*
                                                      
                        
     */
    limLog(pMac, LOG1, FL("Received SME_ASSOC_CNF. Delete Timer\n"));
    limDeactivateAndChangePerStaIdTimer(pMac, eLIM_CNF_WAIT_TIMER, pStaDs->assocId);
    if (assocCnf.statusCode == eSIR_SME_SUCCESS)
    {
        /*                                                          
                                                                               
                                                                  
         */
        pStaDs->mlmStaContext.mlmState = eLIM_MLM_LINK_ESTABLISHED_STATE;
        limLog(pMac, LOG1, FL("sending Assoc Rsp frame to STA (assoc id=%d) \n"), pStaDs->assocId);
        limSendAssocRspMgmtFrame( pMac, eSIR_SUCCESS, pStaDs->assocId, pStaDs->staAddr,
                                  pStaDs->mlmStaContext.subType, pStaDs, psessionEntry);
        goto end;
    } //                                          
    else
    {
        //                                                                            
        limRejectAssociation(pMac, pStaDs->staAddr,
                             pStaDs->mlmStaContext.subType,
                             true, pStaDs->mlmStaContext.authType,
                             pStaDs->assocId, true,
                             assocCnf.statusCode, psessionEntry);
        return;
    }
end:
    if ( psessionEntry->parsedAssocReq[pStaDs->assocId] != NULL )
    {
        if ( ((tpSirAssocReq)(psessionEntry->parsedAssocReq[pStaDs->assocId]))->assocReqFrame) 
        {
            palFreeMemory(pMac->hHdd,((tpSirAssocReq)(psessionEntry->parsedAssocReq[pStaDs->assocId]))->assocReqFrame);
            ((tpSirAssocReq)(psessionEntry->parsedAssocReq[pStaDs->assocId]))->assocReqFrame = NULL;
        }        

        palFreeMemory(pMac->hHdd, psessionEntry->parsedAssocReq[pStaDs->assocId]);
        psessionEntry->parsedAssocReq[pStaDs->assocId] = NULL;
    }
} /*                                      */
#endif

#ifdef WLAN_SOFTAP_FEATURE
void
limSendBeaconInd(tpAniSirGlobal pMac, tpPESession psessionEntry){
    tBeaconGenParams *pBeaconGenParams = NULL;
    tSirMsgQ limMsg;
    /*                                                                         */
    if(psessionEntry == NULL ){
       PELOGE( limLog( pMac, LOGE,
                        FL( "Error:Unable to get the PESessionEntry\n" ));)
       return;
    }
    if( eHAL_STATUS_SUCCESS != palAllocateMemory( pMac->hHdd,
                                            (void **) &pBeaconGenParams, (sizeof(*pBeaconGenParams))))
    {
        PELOGE( limLog( pMac, LOGP,
                        FL( "Unable to PAL allocate memory during sending beaconPreMessage\n" ));)
        return;
    }
    palZeroMemory( pMac->hHdd, pBeaconGenParams, sizeof(*pBeaconGenParams));
    palCopyMemory( pMac->hHdd, (void *) pBeaconGenParams->bssId,
                    (void *)psessionEntry->bssId,
                    SIR_MAC_ADDR_LENGTH );
    limMsg.bodyptr = pBeaconGenParams;
    schProcessPreBeaconInd(pMac, &limMsg);
    return;
}
#endif
