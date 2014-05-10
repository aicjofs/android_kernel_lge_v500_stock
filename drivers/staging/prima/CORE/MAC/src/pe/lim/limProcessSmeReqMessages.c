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

#include "palTypes.h"
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
#include "limTypes.h"
#include "limUtils.h"
#include "limAssocUtils.h"
#include "limSecurityUtils.h"
#include "limSerDesUtils.h"
#include "limSmeReqUtils.h"
#include "limIbssPeerMgmt.h"
#include "limAdmitControl.h"
#include "dphHashTable.h"
#include "limSendMessages.h"
#include "limApi.h"
#include "wmmApsd.h"

#ifdef WLAN_SOFTAP_FEATURE
#include "sapApi.h"
#endif

#if defined WLAN_FEATURE_VOWIFI
#include "rrmApi.h"
#endif
#if defined FEATURE_WLAN_CCX
#include "ccxApi.h"
#endif

#if defined WLAN_FEATURE_VOWIFI_11R
#include <limFT.h>
#endif

#ifdef FEATURE_WLAN_CCX
/*                                                         
                               */
#define MIN_TX_PWR_CAP    12
#define MAX_TX_PWR_CAP    19

#endif


//                                      
static void __limProcessSmeStartReq(tpAniSirGlobal, tANI_U32 *);
static tANI_BOOLEAN __limProcessSmeSysReadyInd(tpAniSirGlobal, tANI_U32 *);
static tANI_BOOLEAN __limProcessSmeStartBssReq(tpAniSirGlobal, tpSirMsgQ pMsg);
static void __limProcessSmeScanReq(tpAniSirGlobal, tANI_U32 *);
static void __limProcessSmeJoinReq(tpAniSirGlobal, tANI_U32 *);
static void __limProcessSmeReassocReq(tpAniSirGlobal, tANI_U32 *);
static void __limProcessSmeDisassocReq(tpAniSirGlobal, tANI_U32 *);
static void __limProcessSmeDisassocCnf(tpAniSirGlobal, tANI_U32 *);
static void __limProcessSmeDeauthReq(tpAniSirGlobal, tANI_U32 *);
static void __limProcessSmeSetContextReq(tpAniSirGlobal, tANI_U32 *);
static tANI_BOOLEAN __limProcessSmeStopBssReq(tpAniSirGlobal, tpSirMsgQ pMsg);

#if 0
  static void __limProcessSmeAuthReq(tpAniSirGlobal, tANI_U32 *);
  static void __limProcessSmePromiscuousReq(tpAniSirGlobal, tANI_U32 *);
#endif

#ifdef ANI_PRODUCT_TYPE_AP
static void __limProcessSmeAssocCnf(tpAniSirGlobal, tANI_U32, tANI_U32 *);
#endif
void __limProcessSmeAssocCnfNew(tpAniSirGlobal, tANI_U32, tANI_U32 *);

#ifdef VOSS_ENABLED
extern void peRegisterTLHandle(tpAniSirGlobal pMac);
#endif

#ifdef WLAN_FEATURE_P2P
extern int limProcessRemainOnChnlReq(tpAniSirGlobal pMac, tANI_U32 *pMsg);
#endif

#ifdef ANI_PRODUCT_TYPE_CLIENT
#ifdef BACKGROUND_SCAN_ENABLED

//                                                              
static void
__limBackgroundScanInitiate(tpAniSirGlobal pMac)
{
    if (pMac->lim.gLimBackgroundScanStarted)
        return;

    //                                
    if (TX_TIMER_VALID(pMac->lim.limTimers.gLimBackgroundScanTimer))
    {
        limDeactivateAndChangeTimer(pMac, eLIM_BACKGROUND_SCAN_TIMER);
     MTRACE(macTrace(pMac, TRACE_CODE_TIMER_ACTIVATE, NO_SESSION, eLIM_BACKGROUND_SCAN_TIMER));
        if (tx_timer_activate(&pMac->lim.limTimers.gLimBackgroundScanTimer) != TX_SUCCESS)
            limLog(pMac, LOGP, FL("could not activate background scan timer\n"));
        pMac->lim.gLimBackgroundScanStarted   = true;
        pMac->lim.gLimBackgroundScanChannelId = 0;
    }
}

#endif //                        
#endif

//                                                        
/*
                                                                                                  
                                                                                          
                                                                                           
                     
*/
static tANI_U8
__limFreshScanReqd(tpAniSirGlobal pMac, tANI_U8 returnFreshResults)
{

    tANI_U8 validState = TRUE;
    int i;

    if(pMac->lim.gLimSmeState != eLIM_SME_IDLE_STATE)
    {
        return FALSE;
    }
    for(i =0; i < pMac->lim.maxBssId; i++)
    {

        if(pMac->lim.gpSession[i].valid == TRUE)
        {
            if(!( ( (  (pMac->lim.gpSession[i].bssType == eSIR_INFRASTRUCTURE_MODE) ||
                        (pMac->lim.gpSession[i].limSystemRole == eLIM_BT_AMP_STA_ROLE))&&
                       (pMac->lim.gpSession[i].limSmeState == eLIM_SME_LINK_EST_STATE) )||
                  
                  (    ( (pMac->lim.gpSession[i].bssType == eSIR_IBSS_MODE)||
                           (pMac->lim.gpSession[i].limSystemRole == eLIM_BT_AMP_AP_ROLE)||
                           (pMac->lim.gpSession[i].limSystemRole == eLIM_BT_AMP_STA_ROLE) )&&
                       (pMac->lim.gpSession[i].limSmeState == eLIM_SME_NORMAL_STATE) )
#ifdef WLAN_FEATURE_P2P
               ||  ( ( ( (pMac->lim.gpSession[i].bssType == eSIR_INFRA_AP_MODE) 
                      && ( pMac->lim.gpSession[i].pePersona == VOS_P2P_GO_MODE) )
                    || (pMac->lim.gpSession[i].limSystemRole == eLIM_AP_ROLE) )
                  && (pMac->lim.gpSession[i].limSmeState == eLIM_SME_NORMAL_STATE) )
#endif
             ))
                {
                validState = FALSE;
                break;
              }
            
        }
    }
    PELOG1(limLog(pMac, LOG1, FL("FreshScanReqd: %d \n"), validState);)

   if( (validState) && (returnFreshResults & SIR_BG_SCAN_RETURN_FRESH_RESULTS))
    return TRUE;

    return FALSE;
}

#if (WNI_POLARIS_FW_PACKAGE == ADVANCED) && defined(ANI_PRODUCT_TYPE_AP)
static tANI_BOOLEAN __limProcessSmeSwitchChlReq(tpAniSirGlobal, tpSirMsgQ pMsg);
#endif


/* 
                            
  
           
                                                          
                           
  
        
                                                         
  
              
  
       
  
                                                          
                                                             
                              
                              
 */

inline static tANI_U8
__limIsSmeAssocCnfValid(tpSirSmeAssocCnf pAssocCnf)
{
    if (limIsGroupAddr(pAssocCnf->peerMacAddr))
        return false;
    else
        return true;
} /*                                   */


/* 
                                   
  
           
                                                             
                                    
                        
  
         
  
        
  
              
     
  
       
     
  
                       
                             
 */

static tANI_U16
__limGetSmeJoinReqSizeForAlloc(tANI_U8 *pBuf)
{
    tANI_U16 len = 0;

    if (!pBuf)
        return len;

    pBuf += sizeof(tANI_U16);
    len = limGetU16( pBuf );
    return (len + sizeof( tANI_U16 ));
} /*                                          */


/*                                                                 
                                 

                                                              
                                                              
                                    

            
                                                         
                         
                         
                                                                  */
static tANI_BOOLEAN
__limIsDeferedMsgForLearn(tpAniSirGlobal pMac, tpSirMsgQ pMsg)
{
    if (limIsSystemInScanState(pMac))
    {
        if (limDeferMsg(pMac, pMsg) != TX_SUCCESS)
        {
            PELOGE(limLog(pMac, LOGE, FL("Could not defer Msg = %d\n"), pMsg->type);)
            return eANI_BOOLEAN_FALSE;
        }
        PELOG1(limLog(pMac, LOG1, FL("Defer the message, in learn mode type = %d\n"),
                                                                 pMsg->type);)

        /*                                                                         
                                                           
         */        
        if (GET_LIM_PROCESS_DEFD_MESGS(pMac))
        {
            //                                                       
            //                                                               
            peSetResumeChannel(pMac, 0, 0);
            limSendHalFinishScanReq(pMac, eLIM_HAL_FINISH_LEARN_WAIT_STATE);
        }

        return eANI_BOOLEAN_TRUE;
    }
    return eANI_BOOLEAN_FALSE;
}

/*                                                                 
                                 

                                                              
                                                

            
                                                         
                         
                         
                                                                  */
static tANI_BOOLEAN
__limIsDeferedMsgForRadar(tpAniSirGlobal pMac, tpSirMsgQ pMsg)
{
    /*                                                                     
                                                    */
    //                                        
    //                                
    if( 0 )
    {
        if (limDeferMsg(pMac, pMsg) != TX_SUCCESS)
        {
            PELOGE(limLog(pMac, LOGE, FL("Could not defer Msg = %d\n"), pMsg->type);)
            return eANI_BOOLEAN_FALSE;
        }
        PELOG1(limLog(pMac, LOG1, FL("Defer the message, in learn mode type = %d\n"),
                                                                 pMsg->type);)
        return eANI_BOOLEAN_TRUE;
    }
    return eANI_BOOLEAN_FALSE;
}


/* 
                            
  
           
                                                           
                                       
  
        
  
              
  
       
  
                                                    
                                                        
               
 */

static void
__limProcessSmeStartReq(tpAniSirGlobal pMac, tANI_U32 *pMsgBuf)
{
    tSirResultCodes  retCode = eSIR_SME_SUCCESS;
    tANI_U8          smesessionId;
    tANI_U16         smetransactionId;
    

   PELOG1(limLog(pMac, LOG1, FL("Received START_REQ\n"));)

    limGetSessionInfo(pMac,(tANI_U8 *)pMsgBuf,&smesessionId,&smetransactionId);
    
    if (pMac->lim.gLimSmeState == eLIM_SME_OFFLINE_STATE)
    {
        pMac->lim.gLimSmeState = eLIM_SME_IDLE_STATE;
        
        MTRACE(macTrace(pMac, TRACE_CODE_SME_STATE, NO_SESSION, pMac->lim.gLimSmeState));
        
        //                                                 
        pMac->lim.gLimReturnAfterFirstMatch = 0;

        //                              
        limInitMlm(pMac);

        //                                       
        pMac->lim.gLimReturnUniqueResults = true;
        pMac->lim.gLimSmeScanResultLength = 0;

#if defined(ANI_PRODUCT_TYPE_CLIENT) || defined(ANI_AP_CLIENT_SDK)
        if (((tSirSmeStartReq *) pMsgBuf)->sendNewBssInd)
        {
            /*
                                                                                    
                                                         
                     */
            if (cfgSetInt(pMac, WNI_CFG_NEW_BSS_FOUND_IND, ((tSirSmeStartReq *) pMsgBuf)->sendNewBssInd)
                != eSIR_SUCCESS)
            {
                limLog(pMac, LOGP, FL("could not set NEIGHBOR_BSS_IND at CFG\n"));
                retCode = eSIR_SME_UNEXPECTED_REQ_RESULT_CODE;
            }
        }
#endif
    }
    else
    {
        /* 
                                                                
                                                          
                    
         */
        limLog(pMac, LOGE, FL("Invalid SME_START_REQ received in SME state %X\n"),pMac->lim.gLimSmeState );
        limPrintSmeState(pMac, LOGE, pMac->lim.gLimSmeState);
        retCode = eSIR_SME_UNEXPECTED_REQ_RESULT_CODE;
    }
    limSendSmeRsp(pMac, eWNI_SME_START_RSP, retCode,smesessionId,smetransactionId);
} /*                                   */


/*                                                               
                              
                                                                               
                            
                          
                                                                    
                                                                         
                                                               */
static tANI_BOOLEAN
__limProcessSmeSysReadyInd(tpAniSirGlobal pMac, tANI_U32 *pMsgBuf)
{
    tSirMsgQ msg;
    
    msg.type = WDA_SYS_READY_IND;
    msg.reserved = 0;
    msg.bodyptr =  pMsgBuf;
    msg.bodyval = 0;

#ifdef VOSS_ENABLED
    if(pMac->gDriverType != eDRIVER_TYPE_MFG)
    {
    peRegisterTLHandle(pMac);
    }
#endif
    PELOGW(limLog(pMac, LOGW, FL("sending WDA_SYS_READY_IND msg to HAL\n"));)
    MTRACE(macTraceMsgTx(pMac, NO_SESSION, msg.type));

    if(eSIR_SUCCESS != wdaPostCtrlMsg(pMac, &msg))
    {
        limLog(pMac, LOGP, FL("wdaPostCtrlMsg failed\n"));
        return eANI_BOOLEAN_TRUE;
    }
    return eANI_BOOLEAN_FALSE;
}

#ifdef WLAN_FEATURE_11AC

tANI_U32 limGetCenterChannel(tpAniSirGlobal pMac,tANI_U8 primarychanNum,ePhyChanBondState secondaryChanOffset, tANI_U8 chanWidth)
{
    if (chanWidth == WNI_CFG_VHT_CHANNEL_WIDTH_80MHZ)
    {
        switch(secondaryChanOffset)
        {
            case PHY_QUADRUPLE_CHANNEL_20MHZ_CENTERED_40MHZ_CENTERED:
                return primarychanNum;
            case PHY_QUADRUPLE_CHANNEL_20MHZ_LOW_40MHZ_CENTERED:
               return primarychanNum + 2;
            case PHY_QUADRUPLE_CHANNEL_20MHZ_HIGH_40MHZ_CENTERED:
               return primarychanNum - 2;
            case PHY_QUADRUPLE_CHANNEL_20MHZ_LOW_40MHZ_LOW:
               return primarychanNum + 6;
            case PHY_QUADRUPLE_CHANNEL_20MHZ_HIGH_40MHZ_LOW:
               return primarychanNum + 2;
            case PHY_QUADRUPLE_CHANNEL_20MHZ_LOW_40MHZ_HIGH:
               return primarychanNum - 2;
            case PHY_QUADRUPLE_CHANNEL_20MHZ_HIGH_40MHZ_HIGH:
               return primarychanNum - 6;
            default :
               return eSIR_CFG_INVALID_ID;
        }
    }
    else if (chanWidth == WNI_CFG_VHT_CHANNEL_WIDTH_20_40MHZ)
    {
        switch(secondaryChanOffset)
        {
            case PHY_DOUBLE_CHANNEL_LOW_PRIMARY:
                return primarychanNum + 2;
            case PHY_DOUBLE_CHANNEL_HIGH_PRIMARY:
                return primarychanNum - 2;
            case PHY_QUADRUPLE_CHANNEL_20MHZ_CENTERED_40MHZ_CENTERED:
                return primarychanNum;
            case PHY_QUADRUPLE_CHANNEL_20MHZ_LOW_40MHZ_CENTERED:
               return primarychanNum + 2;
            case PHY_QUADRUPLE_CHANNEL_20MHZ_HIGH_40MHZ_CENTERED:
               return primarychanNum - 2;
            case PHY_QUADRUPLE_CHANNEL_20MHZ_LOW_40MHZ_LOW:
               return primarychanNum + 2;
            case PHY_QUADRUPLE_CHANNEL_20MHZ_HIGH_40MHZ_LOW:
               return primarychanNum - 2;
            case PHY_QUADRUPLE_CHANNEL_20MHZ_LOW_40MHZ_HIGH:
               return primarychanNum + 2;
            case PHY_QUADRUPLE_CHANNEL_20MHZ_HIGH_40MHZ_HIGH:
               return primarychanNum - 2;
            default :
               return eSIR_CFG_INVALID_ID;
        }
    }
    return primarychanNum;
}

#endif
/* 
                                  
  
           
                                                               
                                       
  
        
  
              
  
       
  
                                                    
                                                        
               
 */

static void
__limHandleSmeStartBssRequest(tpAniSirGlobal pMac, tANI_U32 *pMsgBuf)
{
    tANI_U16                size;
    tANI_U32                val = 0;
    tSirRetStatus           retStatus;
    tSirMacChanNum          channelNumber;
    tLimMlmStartReq         *pMlmStartReq;
    tpSirSmeStartBssReq     pSmeStartBssReq;                //                                                            
    tSirResultCodes         retCode = eSIR_SME_SUCCESS;
    tANI_U32                autoGenBssId = FALSE;           //                                                 
    tANI_U8                 sessionId;
    tpPESession             psessionEntry = NULL;
    tANI_U8                 smesessionId;
    tANI_U16                smetransactionId;

#ifdef FEATURE_WLAN_DIAG_SUPPORT_LIM //                          
    //                                                                                               
    limDiagEventReport(pMac, WLAN_PE_DIAG_START_BSS_REQ_EVENT, NULL, 0, 0);
#endif //                         
    
    PELOG1(limLog(pMac, LOG1, FL("Received START_BSS_REQ\n"));)

    /*                                                                                        */
    if ( (pMac->lim.gLimSmeState == eLIM_SME_OFFLINE_STATE) ||
         (pMac->lim.gLimSmeState == eLIM_SME_IDLE_STATE))
    {
        size = sizeof(tSirSmeStartBssReq) + SIR_MAC_MAX_IE_LENGTH;

#ifdef ANI_PRODUCT_TYPE_AP
        size + = ANI_WDS_INFO_MAX_LENGTH;
#endif

        if( eHAL_STATUS_SUCCESS != palAllocateMemory( pMac->hHdd, (void **)&pSmeStartBssReq, size))
        {
            PELOGE(limLog(pMac, LOGE, FL("palAllocateMemory failed for pMac->lim.gpLimStartBssReq\n"));)
            //                               
            retCode = eSIR_SME_RESOURCES_UNAVAILABLE;
            goto end;
        }
        
        (void) palZeroMemory(pMac->hHdd, (void *)pSmeStartBssReq, size);
        
        if ((limStartBssReqSerDes(pMac, pSmeStartBssReq, (tANI_U8 *) pMsgBuf) == eSIR_FAILURE) ||
                (!limIsSmeStartBssReqValid(pMac, pSmeStartBssReq)))
        {
            PELOGW(limLog(pMac, LOGW, FL("Received invalid eWNI_SME_START_BSS_REQ\n"));)
            retCode = eSIR_SME_INVALID_PARAMETERS;
            goto free;
        }
#if 0   
       PELOG3(limLog(pMac, LOG3,
           FL("Parsed START_BSS_REQ fields are bssType=%d, channelId=%d\n"),
           pMac->lim.gpLimStartBssReq->bssType, pMac->lim.gpLimStartBssReq->channelId);)
#endif 

        /*                                                         
                                                                 */
        if((psessionEntry = peFindSessionByBssid(pMac,pSmeStartBssReq->bssId,&sessionId)) != NULL)
        {
            limLog(pMac, LOGW, FL("Session Already exists for given BSSID\n"));
            retCode = eSIR_SME_BSS_ALREADY_STARTED_OR_JOINED;
            psessionEntry = NULL;
            goto free;
        }
        else
        {
            if((psessionEntry = peCreateSession(pMac,pSmeStartBssReq->bssId,&sessionId, pMac->lim.maxStation)) == NULL)
            {
                limLog(pMac, LOGW, FL("Session Can not be created \n"));
                retCode = eSIR_SME_RESOURCES_UNAVAILABLE;
                goto free;
            }

        }
     
        /*                                                               */
        psessionEntry->pLimStartBssReq = pSmeStartBssReq;

        /*                                      */
        psessionEntry->peSessionId = sessionId;

        /*                                      */
        psessionEntry->smeSessionId = pSmeStartBssReq->sessionId;

        psessionEntry->transactionId = pSmeStartBssReq->transactionId;
                     
        sirCopyMacAddr(psessionEntry->selfMacAddr,pSmeStartBssReq->selfMacAddr);
        
        /*                            */
        palCopyMemory( pMac->hHdd, (tANI_U8 *)&psessionEntry->ssId,
                     (tANI_U8 *)&pSmeStartBssReq->ssId,
                      (pSmeStartBssReq->ssId.length + 1));
        
       
        
        psessionEntry->bssType = pSmeStartBssReq->bssType;
        
        psessionEntry->nwType = pSmeStartBssReq->nwType;
        
        psessionEntry->beaconParams.beaconInterval = pSmeStartBssReq->beaconInterval;

        /*                                           */
        psessionEntry->currentOperChannel = pSmeStartBssReq->channelId;

        /*              */
        psessionEntry->pePersona = pSmeStartBssReq->bssPersona;
        VOS_TRACE(VOS_MODULE_ID_PE, VOS_TRACE_LEVEL_INFO,FL("PE PERSONA=%d"),
            psessionEntry->pePersona);

        /*                  */
        psessionEntry->gLimPhyMode = pSmeStartBssReq->nwType;

        psessionEntry->maxTxPower = cfgGetRegulatoryMaxTransmitPower( pMac, 
            psessionEntry->currentOperChannel );
        /*                                              */

        psessionEntry->dot11mode = pSmeStartBssReq->dot11mode;
        psessionEntry->htCapability = IS_DOT11_MODE_HT(psessionEntry->dot11mode);
#ifdef WLAN_FEATURE_11AC
        psessionEntry->vhtCapability = IS_DOT11_MODE_VHT(psessionEntry->dot11mode);
        VOS_TRACE(VOS_MODULE_ID_PE,VOS_TRACE_LEVEL_INFO,
            FL("*****psessionEntry->vhtCapability = %d"),psessionEntry->vhtCapability);
#endif
        palCopyMemory(pMac->hHdd, (void*)&psessionEntry->rateSet,
            (void*)&pSmeStartBssReq->operationalRateSet,
            sizeof(tSirMacRateSet));
        palCopyMemory(pMac->hHdd, (void*)&psessionEntry->extRateSet,
            (void*)&pSmeStartBssReq->extendedRateSet,
            sizeof(tSirMacRateSet));

        switch(pSmeStartBssReq->bssType)
        {
#ifdef WLAN_SOFTAP_FEATURE
            case eSIR_INFRA_AP_MODE:
                 psessionEntry->limSystemRole = eLIM_AP_ROLE;
                 psessionEntry->privacy = pSmeStartBssReq->privacy;
                 psessionEntry->fwdWPSPBCProbeReq = pSmeStartBssReq->fwdWPSPBCProbeReq;
                 psessionEntry->authType = pSmeStartBssReq->authType;
                 /*                       */
                 psessionEntry->dtimPeriod = (tANI_U8)pSmeStartBssReq->dtimPeriod;
                 /*                    */
                 psessionEntry->apUapsdEnable = pSmeStartBssReq->apUapsdEnable;
                 if (psessionEntry->pePersona == VOS_P2P_GO_MODE)
                 {
                     psessionEntry->proxyProbeRspEn = 0;
                 }
                 else
                 {
                     /*                                                    
                                       
                      */
                     if(SAP_WPS_DISABLED == pSmeStartBssReq->wps_state)
                     {
                         psessionEntry->proxyProbeRspEn = 1;
                     }
                     else
                     {
                         psessionEntry->proxyProbeRspEn = 0;
                     }
                 }
                 psessionEntry->ssidHidden = pSmeStartBssReq->ssidHidden;
                 psessionEntry->wps_state = pSmeStartBssReq->wps_state;
                 break;
#endif
            case eSIR_IBSS_MODE:
                 psessionEntry->limSystemRole = eLIM_STA_IN_IBSS_ROLE;
                 break;

            case eSIR_BTAMP_AP_MODE:
                 psessionEntry->limSystemRole = eLIM_BT_AMP_AP_ROLE;
                 break;

            case eSIR_BTAMP_STA_MODE:
                 psessionEntry->limSystemRole = eLIM_BT_AMP_STA_ROLE;
                 break;

                 /*                                                                 */

                 //                                             


            default:
                                   //                                         
                 break;
        }

        //                                                                            
        if ( (pSmeStartBssReq->bssType == eSIR_BTAMP_AP_MODE)
#ifdef WLAN_SOFTAP_FEATURE
        || (pSmeStartBssReq->bssType == eSIR_INFRA_AP_MODE)
#endif
        )
        {
            if( eHAL_STATUS_SUCCESS != palAllocateMemory( pMac->hHdd, (void **)&psessionEntry->parsedAssocReq,
                                                          (psessionEntry->dph.dphHashTable.size * sizeof(tpSirAssocReq)) ))
            {
                limLog(pMac, LOGW, FL("palAllocateMemory() failed\n"));
                retCode = eSIR_SME_RESOURCES_UNAVAILABLE;
                goto free;
            }
            palZeroMemory(pMac->hHdd, psessionEntry->parsedAssocReq, (psessionEntry->dph.dphHashTable.size * sizeof(tpSirAssocReq)) );
        }

        /*                                                                                               */
        if (pSmeStartBssReq->channelId)
        {
            channelNumber = pSmeStartBssReq->channelId;
            psessionEntry->htSupportedChannelWidthSet = (pSmeStartBssReq->cbMode)?1:0; //                                                                                   
            psessionEntry->htRecommendedTxWidthSet = psessionEntry->htSupportedChannelWidthSet;
            psessionEntry->htSecondaryChannelOffset = pSmeStartBssReq->cbMode;
            VOS_TRACE(VOS_MODULE_ID_PE, VOS_TRACE_LEVEL_INFO,
                      FL("cbMode %u"), pSmeStartBssReq->cbMode);
#ifdef WLAN_FEATURE_11AC
            if(psessionEntry->vhtCapability)
            {
                tANI_U32 centerChan;
                tANI_U32 chanWidth;

                if (wlan_cfgGetInt(pMac, WNI_CFG_VHT_CHANNEL_WIDTH,
                          &chanWidth) != eSIR_SUCCESS)
                {
                    limLog(pMac, LOGP,
                      FL("Unable to retrieve Channel Width from CFG\n"));
                }

                if(chanWidth == eHT_CHANNEL_WIDTH_20MHZ || chanWidth == eHT_CHANNEL_WIDTH_40MHZ)
                {
                    if (cfgSetInt(pMac, WNI_CFG_VHT_CHANNEL_WIDTH, WNI_CFG_VHT_CHANNEL_WIDTH_20_40MHZ)
                                                                     != eSIR_SUCCESS)
                    {
                        limLog(pMac, LOGP, FL("could not set  WNI_CFG_CHANNEL_BONDING_MODE at CFG\n"));
                        retCode = eSIR_LOGP_EXCEPTION;
                         goto free;
                    }
                }
                if (chanWidth == eHT_CHANNEL_WIDTH_80MHZ)
                {
                    if (cfgSetInt(pMac, WNI_CFG_VHT_CHANNEL_WIDTH, WNI_CFG_VHT_CHANNEL_WIDTH_80MHZ)
                                                                     != eSIR_SUCCESS)
                    {
                        limLog(pMac, LOGP, FL("could not set  WNI_CFG_CHANNEL_BONDING_MODE at CFG\n"));
                        retCode = eSIR_LOGP_EXCEPTION;
                         goto free;
                    }

                    centerChan = limGetCenterChannel(pMac,channelNumber,pSmeStartBssReq->cbMode,WNI_CFG_VHT_CHANNEL_WIDTH_80MHZ);
                    if(centerChan != eSIR_CFG_INVALID_ID)
                    {
                        limLog(pMac, LOGW, FL("***Center Channel for 80MHZ channel width = %ld\n"),centerChan);
                        if (cfgSetInt(pMac, WNI_CFG_VHT_CHANNEL_CENTER_FREQ_SEGMENT1, centerChan)
                                                                     != eSIR_SUCCESS)
                        {
                            limLog(pMac, LOGP, FL("could not set  WNI_CFG_CHANNEL_BONDING_MODE at CFG\n"));
                            retCode = eSIR_LOGP_EXCEPTION;
                            goto free;
                        }
                    }
                }

                /*                                                                           
                                                                                             
                                                           */
                if (wlan_cfgGetInt(pMac, WNI_CFG_VHT_CHANNEL_WIDTH,
                                   &chanWidth) != eSIR_SUCCESS)
                {
                    limLog(pMac, LOGP,
                      FL("Unable to retrieve Channel Width from CFG\n"));
                }

                psessionEntry->vhtTxChannelWidthSet = chanWidth;
            }
            psessionEntry->htSecondaryChannelOffset = limGetHTCBState(pSmeStartBssReq->cbMode);
#endif
        }
        else
        {
            PELOGW(limLog(pMac, LOGW, FL("Received invalid eWNI_SME_START_BSS_REQ\n"));)
            retCode = eSIR_SME_INVALID_PARAMETERS;
            goto free;
        }

        //                            
        limDeletePreAuthList(pMac);

        //                                            
        //                                  



#ifdef FIXME_GEN6   //                                                                                  
        //                              
        limInitMlm(pMac);
#endif

        psessionEntry->htCapability = IS_DOT11_MODE_HT(pSmeStartBssReq->dot11mode);

#ifdef WLAN_SOFTAP_FEATURE
            /*                                                    
                                                                            
               */
        limSetRSNieWPAiefromSmeStartBSSReqMessage(pMac,&pSmeStartBssReq->rsnIE,psessionEntry);

#endif

#ifdef WLAN_SOFTAP_FEATURE
        //                                                    
        if (psessionEntry->limSystemRole == eLIM_AP_ROLE){
            psessionEntry->gLimProtectionControl =  pSmeStartBssReq->protEnabled;
            /*                                      
                                                                   
                                                           */
            palCopyMemory( pMac->hHdd, (void *) &psessionEntry->cfgProtection,
                          (void *) &pSmeStartBssReq->ht_capab,
                          sizeof( tCfgProtection ));
            psessionEntry->pAPWPSPBCSession = NULL; //                                     
        }
#endif

        //                                           
        if( eHAL_STATUS_SUCCESS != palAllocateMemory( pMac->hHdd, (void **)&pMlmStartReq, sizeof(tLimMlmStartReq)))
        {
            limLog(pMac, LOGP, FL("call to palAllocateMemory failed for mlmStartReq\n"));
            retCode = eSIR_SME_RESOURCES_UNAVAILABLE;
            goto free;
        }

        (void)palZeroMemory(pMac->hHdd, (void *) pMlmStartReq, sizeof(tLimMlmStartReq));

        /*                                      */
        palCopyMemory( pMac->hHdd, (tANI_U8 *) &pMlmStartReq->ssId,
                          (tANI_U8 *) &pSmeStartBssReq->ssId,
                          pSmeStartBssReq->ssId.length + 1);
#ifdef WLAN_SOFTAP_FEATURE
        pMlmStartReq->ssidHidden = pSmeStartBssReq->ssidHidden;
        pMlmStartReq->obssProtEnabled = pSmeStartBssReq->obssProtEnabled;
#endif


        pMlmStartReq->bssType = psessionEntry->bssType;

        /*                                           */
        pMlmStartReq->sessionId = psessionEntry->peSessionId;

        if( (pMlmStartReq->bssType == eSIR_BTAMP_STA_MODE) || (pMlmStartReq->bssType == eSIR_BTAMP_AP_MODE )
#ifdef WLAN_SOFTAP_FEATURE
            || (pMlmStartReq->bssType == eSIR_INFRA_AP_MODE)
#endif
        )
        {
            //                          
            //                                                                                        
            //                              
            //                                                                             

            /*                                                        */
            sirCopyMacAddr(pMlmStartReq->bssId,psessionEntry->bssId);
        }

        else //          
        {
            pMac->lim.gLimIbssCoalescingHappened = false;

            if((retStatus = wlan_cfgGetInt(pMac, WNI_CFG_IBSS_AUTO_BSSID, &autoGenBssId)) != eSIR_SUCCESS)
            {
                limLog(pMac, LOGP, FL("Could not retrieve Auto Gen BSSID, retStatus=%d\n"), retStatus);
                retCode = eSIR_LOGP_EXCEPTION;
                goto free;
            }

            if(!autoGenBssId)
            {   
                //                                                                    
                sirCopyMacAddr(pMlmStartReq->bssId,psessionEntry->bssId);
                
                if(pMlmStartReq->bssId[0] & 0x01)
                {
                   PELOGE(limLog(pMac, LOGE, FL("Request to start IBSS with group BSSID\n Autogenerating the BSSID\n"));)                    
                   autoGenBssId = TRUE;
                }             
            }

            if( autoGenBssId )
            {   //                                                            
                //                      
                limGetRandomBssid( pMac, pMlmStartReq->bssId);
                pMlmStartReq->bssId[0]= 0x02;
                
                /*                                                    */
                sirCopyMacAddr(psessionEntry->bssId,pMlmStartReq->bssId);
            }
        }
        /*                                                 */
        pMlmStartReq->channelNumber = psessionEntry->currentOperChannel;
        pMlmStartReq->cbMode = pSmeStartBssReq->cbMode;        
        pMlmStartReq->beaconPeriod = psessionEntry->beaconParams.beaconInterval;

#ifdef WLAN_SOFTAP_FEATURE
        if(psessionEntry->limSystemRole == eLIM_AP_ROLE ){
            pMlmStartReq->dtimPeriod = psessionEntry->dtimPeriod;
            pMlmStartReq->wps_state = psessionEntry->wps_state;

        }else
#endif        
        {
            if (wlan_cfgGetInt(pMac, WNI_CFG_DTIM_PERIOD, &val) != eSIR_SUCCESS)
                limLog(pMac, LOGP, FL("could not retrieve DTIM Period\n"));
            pMlmStartReq->dtimPeriod = (tANI_U8)val;
        }   
            
        if (wlan_cfgGetInt(pMac, WNI_CFG_CFP_PERIOD, &val) != eSIR_SUCCESS)
            limLog(pMac, LOGP, FL("could not retrieve Beacon interval\n"));
        pMlmStartReq->cfParamSet.cfpPeriod = (tANI_U8)val;
            
        if (wlan_cfgGetInt(pMac, WNI_CFG_CFP_MAX_DURATION, &val) != eSIR_SUCCESS)
            limLog(pMac, LOGP, FL("could not retrieve CFPMaxDuration\n"));
        pMlmStartReq->cfParamSet.cfpMaxDuration = (tANI_U16) val;
        
        //                                                                                                                
        palCopyMemory(pMac->hHdd, (void*)&pMlmStartReq->rateSet, (void*)&psessionEntry->rateSet,
                       sizeof(tSirMacRateSet));

                      
        //                                        
        pMlmStartReq->nwType    = psessionEntry->nwType;
        pMlmStartReq->htCapable = psessionEntry->htCapability;
        //
        //                                                   
        //
        pMlmStartReq->htOperMode        = pMac->lim.gHTOperMode;
        pMlmStartReq->dualCTSProtection = pMac->lim.gHTDualCTSProtection; //       
        pMlmStartReq->txChannelWidthSet = psessionEntry->htRecommendedTxWidthSet;

        /*              */
        psessionEntry->limRFBand = limGetRFBand(channelNumber);

        //                           
        psessionEntry->lim11hEnable = 0;
        if((pMlmStartReq->bssType != eSIR_IBSS_MODE) &&
            (SIR_BAND_5_GHZ == psessionEntry->limRFBand) )
        {
            if (wlan_cfgGetInt(pMac, WNI_CFG_11H_ENABLED, &val) != eSIR_SUCCESS)
                limLog(pMac, LOGP, FL("Fail to get WNI_CFG_11H_ENABLED \n"));
            psessionEntry->lim11hEnable = val;
        }
            
        if (!psessionEntry->lim11hEnable)
        {
            if (cfgSetInt(pMac, WNI_CFG_LOCAL_POWER_CONSTRAINT, 0) != eSIR_SUCCESS)
                limLog(pMac, LOGP, FL("Fail to get WNI_CFG_11H_ENABLED \n"));
        }

        psessionEntry ->limPrevSmeState = psessionEntry->limSmeState;
        psessionEntry ->limSmeState     =  eLIM_SME_WT_START_BSS_STATE;
        MTRACE(macTrace(pMac, TRACE_CODE_SME_STATE, psessionEntry->peSessionId, psessionEntry ->limSmeState));

        limPostMlmMessage(pMac, LIM_MLM_START_REQ, (tANI_U32 *) pMlmStartReq);
        return;
    }
    else
    {
       
        limLog(pMac, LOGE, FL("Received unexpected START_BSS_REQ, in state %X\n"),pMac->lim.gLimSmeState);
        retCode = eSIR_SME_BSS_ALREADY_STARTED_OR_JOINED;
        goto end;
    } //                                                      

free:
    palFreeMemory( pMac->hHdd, pSmeStartBssReq);
    pSmeStartBssReq = NULL;

end:

    /*                                                                     */
    limGetSessionInfo(pMac,(tANI_U8*)pMsgBuf,&smesessionId,&smetransactionId); 
    
    if(NULL != psessionEntry)
    {
        peDeleteSession(pMac,psessionEntry);
        psessionEntry = NULL;
    }     
    limSendSmeStartBssRsp(pMac, eWNI_SME_START_BSS_RSP, retCode,psessionEntry,smesessionId,smetransactionId);
} /*                                         */


/*                                                               
                                  

                                                              
                                                              
                                                             
                          
            
            

                                        
                                            
                                                                */
static tANI_BOOLEAN
__limProcessSmeStartBssReq(tpAniSirGlobal pMac, tpSirMsgQ pMsg)
{
    if (__limIsDeferedMsgForLearn(pMac, pMsg) ||
            __limIsDeferedMsgForRadar(pMac, pMsg))
    {
        /* 
                                                          
                          
         */
        return eANI_BOOLEAN_FALSE;
    }

    __limHandleSmeStartBssRequest(pMac, (tANI_U32 *) pMsg->bodyptr);
    return eANI_BOOLEAN_TRUE;
}


/* 
                       
  
                                                                                    
                                                           
                                        
  
         
  
               
  
        
                                                          
  
                                                     
                                                
                
 */
void limGetRandomBssid(tpAniSirGlobal pMac, tANI_U8 *data)
{
     tANI_U32 random[2] ;
     random[0] = tx_time_get();
     random[0] |= (random[0] << 15) ;
     random[1] = random[0] >> 1;
     palCopyMemory(pMac->hHdd, data, (tANI_U8*)random, sizeof(tSirMacAddr));
}


/* 
                           
  
           
                                                          
                                       
  
        
  
              
  
       
                                                             
                   
  
                                                    
                                                        
               
 */

static void
__limProcessSmeScanReq(tpAniSirGlobal pMac, tANI_U32 *pMsgBuf)
{
    tANI_U32            len;
    tLimMlmScanReq      *pMlmScanReq;
    tpSirSmeScanReq     pScanReq;
    tANI_U8             i = 0;

#ifdef FEATURE_WLAN_DIAG_SUPPORT_LIM //                          
    limDiagEventReport(pMac, WLAN_PE_DIAG_SCAN_REQ_EVENT, NULL, 0, 0);
#endif //                         
    
    pScanReq = (tpSirSmeScanReq) pMsgBuf;   
    PELOG1(limLog(pMac, LOG1, FL("SME SCAN REQ numChan %d min %d max %d IELen %d first %d fresh %d unique %d type %d mode %d rsp %d\n"),
           pScanReq->channelList.numChannels,
           pScanReq->minChannelTime,
           pScanReq->maxChannelTime,
           pScanReq->uIEFieldLen, 
           pScanReq->returnAfterFirstMatch,
           pScanReq->returnFreshResults,
           pScanReq->returnUniqueResults,
           pScanReq->scanType,
           pScanReq->backgroundScanMode,
           pMac->lim.gLimRspReqd ? 1 : 0);)
        
    /*                                                                                                                    */
    sirCopyMacAddr(pMac->lim.gSelfMacAddr,  pScanReq->selfMacAddr);

   /*                                                                     */
       
    if (!limIsSmeScanReqValid(pMac, pScanReq))
    {
        PELOGW(limLog(pMac, LOGW, FL("Received SME_SCAN_REQ with invalid parameters\n"));)

        if (pMac->lim.gLimRspReqd)
        {
            pMac->lim.gLimRspReqd = false;

            limSendSmeScanRsp(pMac, sizeof(tSirSmeScanRsp), eSIR_SME_INVALID_PARAMETERS, pScanReq->sessionId, pScanReq->transactionId);

        } //                           

        return;
    }

    //                                                        
    //                                                                                              
    if( (pMac->lim.fScanDisabled) || (!pMac->lim.gScanInPowersave && !limIsSystemInActiveState(pMac))  )
    {
        limSendSmeScanRsp(pMac, 8, eSIR_SME_INVALID_PARAMETERS, pScanReq->sessionId, pScanReq->transactionId);
        return;
    }
    

    /* 
                                                      
                                                        
                                                       
                                                      
                                                  
                                        
     */
  if (__limFreshScanReqd(pMac, pScanReq->returnFreshResults))
  {
        #if 0   
        //                        
        pMac->lim.gLimPrevSmeState = pMac->lim.gLimSmeState;
        if ((pMac->lim.gLimSmeState == eLIM_SME_IDLE_STATE) ||
            (pMac->lim.gLimSmeState == eLIM_SME_JOIN_FAILURE_STATE))
            pMac->lim.gLimSmeState = eLIM_SME_WT_SCAN_STATE;
        else if (pMac->lim.gLimSmeState == eLIM_SME_NORMAL_STATE)
            pMac->lim.gLimSmeState = eLIM_SME_NORMAL_CHANNEL_SCAN_STATE;
        else
            
        #endif //                 

        /*                         */

        /*                              */

         pMac->lim.gLimPrevSmeState = pMac->lim.gLimSmeState;
        
        pMac->lim.gLimSmeState = eLIM_SME_WT_SCAN_STATE;
        MTRACE(macTrace(pMac, TRACE_CODE_SME_STATE, NO_SESSION, pMac->lim.gLimSmeState));

        if (pScanReq->returnFreshResults & SIR_BG_SCAN_PURGE_RESUTLS)
        {
            //                                       
            limReInitScanResults(pMac);
        }

        pMac->lim.gLim24Band11dScanDone     = 0;
        pMac->lim.gLim50Band11dScanDone     = 0;
        pMac->lim.gLimReturnAfterFirstMatch =
                                    pScanReq->returnAfterFirstMatch;
        pMac->lim.gLimBackgroundScanMode =
                                    pScanReq->backgroundScanMode;

        pMac->lim.gLimReturnUniqueResults   =
              ((pScanReq->returnUniqueResults) > 0 ? true : false);
        /*                                                          
                                                                
                                                       */
        if(((ePMM_STATE_BMPS_WAKEUP == pMac->pmm.gPmmState) ||
           (ePMM_STATE_READY == pMac->pmm.gPmmState))
            && (pScanReq->backgroundScanMode != eSIR_ROAMING_SCAN ))
        {
          for(i=0;i<pMac->lim.maxBssId;i++)
          {
            if((peFindSessionBySessionId(pMac,i) != NULL) &&
               (pMac->lim.gpSession[i].valid == TRUE) &&
               (eLIM_MLM_LINK_ESTABLISHED_STATE == pMac->lim.gpSession[i].limMlmState))
            {
               limHeartBeatDeactivateAndChangeTimer(pMac, peFindSessionBySessionId(pMac,i));
            }   
          }
        }

        if (pScanReq->channelList.numChannels == 0)
        {
            tANI_U32            cfg_len;
            //                  
            len = sizeof(tLimMlmScanReq) + 
                  (sizeof( pScanReq->channelList.channelNumber ) * (WNI_CFG_VALID_CHANNEL_LIST_LEN - 1)) +
                  pScanReq->uIEFieldLen;
            if( eHAL_STATUS_SUCCESS != palAllocateMemory( pMac->hHdd, (void **)&pMlmScanReq, len) )
            {
                //          
                limLog(pMac, LOGP,
                       FL("call to palAllocateMemory failed for mlmScanReq (%d)\n"), len);

                return;
            }

            //                       
            palZeroMemory( pMac->hHdd, (tANI_U8 *) pMlmScanReq, len );

            cfg_len = WNI_CFG_VALID_CHANNEL_LIST_LEN;
            if (wlan_cfgGetStr(pMac, WNI_CFG_VALID_CHANNEL_LIST,
                          pMlmScanReq->channelList.channelNumber,
                          &cfg_len) != eSIR_SUCCESS)
            {
                /* 
                                                             
                             
                 */
                limLog(pMac, LOGP,
                       FL("could not retrieve Valid channel list\n"));
            }
            pMlmScanReq->channelList.numChannels = (tANI_U8) cfg_len;
        }
        else
        {
            len = sizeof( tLimMlmScanReq ) - sizeof( pScanReq->channelList.channelNumber ) + 
                   (sizeof( pScanReq->channelList.channelNumber ) * pScanReq->channelList.numChannels ) +
                   pScanReq->uIEFieldLen;

            if( eHAL_STATUS_SUCCESS != palAllocateMemory( pMac->hHdd, (void **)&pMlmScanReq, len) )
            {
                //          
                limLog(pMac, LOGP,
                    FL("call to palAllocateMemory failed for mlmScanReq(%d)\n"), len);

                return;
            }

            //                       
            palZeroMemory( pMac->hHdd, (tANI_U8 *) pMlmScanReq, len);
            pMlmScanReq->channelList.numChannels =
                            pScanReq->channelList.numChannels;

            palCopyMemory( pMac->hHdd, pMlmScanReq->channelList.channelNumber,
                          pScanReq->channelList.channelNumber,
                          pScanReq->channelList.numChannels);
        }

        pMlmScanReq->uIEFieldLen = pScanReq->uIEFieldLen;
        pMlmScanReq->uIEFieldOffset = len - pScanReq->uIEFieldLen;
        
        if(pScanReq->uIEFieldLen)
        {
            palCopyMemory( pMac->hHdd, (tANI_U8 *)pMlmScanReq+ pMlmScanReq->uIEFieldOffset,
                          (tANI_U8 *)pScanReq+(pScanReq->uIEFieldOffset),
                          pScanReq->uIEFieldLen);
        }

        pMlmScanReq->bssType = pScanReq->bssType;
        palCopyMemory( pMac->hHdd, pMlmScanReq->bssId,
                      pScanReq->bssId,
                      sizeof(tSirMacAddr));
        pMlmScanReq->numSsid = pScanReq->numSsid;

        i = 0;
        while (i < pMlmScanReq->numSsid)
        {
            palCopyMemory( pMac->hHdd, (tANI_U8 *) &pMlmScanReq->ssId[i],
                      (tANI_U8 *) &pScanReq->ssId[i],
                      pScanReq->ssId[i].length + 1);

            i++;
        } 
       

        pMlmScanReq->scanType = pScanReq->scanType;
        pMlmScanReq->backgroundScanMode = pScanReq->backgroundScanMode;
        pMlmScanReq->minChannelTime = pScanReq->minChannelTime;
        pMlmScanReq->maxChannelTime = pScanReq->maxChannelTime;
        pMlmScanReq->dot11mode = pScanReq->dot11mode;
#ifdef WLAN_FEATURE_P2P
        pMlmScanReq->p2pSearch = pScanReq->p2pSearch;
        pMlmScanReq->skipDfsChnlInP2pSearch = pScanReq->skipDfsChnlInP2pSearch;
#endif

        //                                                        
        pMac->lim.gSmeSessionId = pScanReq->sessionId;
        pMac->lim.gTransactionId = pScanReq->transactionId;

        //                              
        limPostMlmMessage(pMac, LIM_MLM_SCAN_REQ, (tANI_U32 *) pMlmScanReq);

    } //                                                           
    
    else
    {
        //                                                 
        if ((pMac->lim.gLimRspReqd) || pMac->lim.gLimReportBackgroundScanResults)
        {
            tANI_U16    scanRspLen = sizeof(tSirSmeScanRsp);

            pMac->lim.gLimRspReqd = false;

            if (pMac->lim.gLimSmeScanResultLength == 0)
            {
                limSendSmeScanRsp(pMac, scanRspLen, eSIR_SME_SUCCESS, pScanReq->sessionId, pScanReq->transactionId);
            }
            else
            {
                scanRspLen = sizeof(tSirSmeScanRsp) +
                             pMac->lim.gLimSmeScanResultLength -
                             sizeof(tSirBssDescription);
                limSendSmeScanRsp(pMac, scanRspLen, eSIR_SME_SUCCESS, pScanReq->sessionId, pScanReq->transactionId);
            }

            if (pScanReq->returnFreshResults & SIR_BG_SCAN_PURGE_RESUTLS)
            {
                //                                       
                limReInitScanResults(pMac);
            }

        } //                           
    } //                                                             

#if defined(ANI_PRODUCT_TYPE_CLIENT) || defined(ANI_AP_CLIENT_SDK)
#ifdef BACKGROUND_SCAN_ENABLED
    //                                 
    //                                                                             
    __limBackgroundScanInitiate(pMac);
#endif
#endif

} /*                                  */

#ifdef FEATURE_OEM_DATA_SUPPORT

static void __limProcessSmeOemDataReq(tpAniSirGlobal pMac, tANI_U32 *pMsgBuf)
{
    tpSirOemDataReq    pOemDataReq;
    tLimMlmOemDataReq* pMlmOemDataReq;

    pOemDataReq = (tpSirOemDataReq) pMsgBuf; 

    //                            
    if(eHAL_STATUS_SUCCESS != palAllocateMemory(pMac->hHdd, (void**)&pMlmOemDataReq, (sizeof(tLimMlmOemDataReq))))
    {
        limLog(pMac, LOGP, FL("palAllocateMemory failed for mlmOemDataReq\n"));
        return;
    }

    //                      
    palZeroMemory(pMac->hHdd, pMlmOemDataReq, (sizeof(tLimMlmOemDataReq)));

    palCopyMemory(pMac->hHdd, pMlmOemDataReq->selfMacAddr, pOemDataReq->selfMacAddr, sizeof(tSirMacAddr)); 
    palCopyMemory(pMac->hHdd, pMlmOemDataReq->oemDataReq, pOemDataReq->oemDataReq, OEM_DATA_REQ_SIZE);

    //                                 
    limPostMlmMessage(pMac, LIM_MLM_OEM_DATA_REQ, (tANI_U32*)pMlmOemDataReq);

    return;

} /*                                     */

#endif //                        


/* 
                           
  
           
                                                          
                                       
  
        
  
              
  
       
  
                                                    
                                                        
               
 */
static void
__limProcessSmeJoinReq(tpAniSirGlobal pMac, tANI_U32 *pMsgBuf)
{
  //                            
    //                        
//                                     
    tpSirSmeJoinReq     pSmeJoinReq = NULL;
    tLimMlmJoinReq      *pMlmJoinReq;
    tSirResultCodes     retCode = eSIR_SME_SUCCESS;
    tANI_U32            val = 0;
    tANI_U16            nSize;
    tANI_U8             sessionId;
    tpPESession         psessionEntry = NULL;
    tANI_U8             smesessionId;
    tANI_U16            smetransactionId;
    tPowerdBm           localPowerConstraint = 0, regMax = 0;

#ifdef FEATURE_WLAN_DIAG_SUPPORT_LIM //                          
    //                                                                                             
    limDiagEventReport(pMac, WLAN_PE_DIAG_JOIN_REQ_EVENT, NULL, 0, 0);
#endif //                         

    PELOG1(limLog(pMac, LOG1, FL("Received SME_JOIN_REQ\n"));)

#ifdef WLAN_FEATURE_VOWIFI
    /*                                                                                    
                                                                                               
                                                                                                     
                                                                                  */
    if (wlan_cfgGetInt(pMac, WNI_CFG_RRM_ENABLED, &val) != eSIR_SUCCESS)
        limLog(pMac, LOGP, FL("cfg get rrm enabled failed\n"));
    pMac->rrm.rrmPEContext.rrmEnable = (val) ? 1 : 0;
    val = 0;
#endif /*                     */

   /* 
                                         
                                                                 
     */
    
    /*                                                                  */
    if(pMac->lim.gLimSmeState == eLIM_SME_IDLE_STATE)
    {
        nSize = __limGetSmeJoinReqSizeForAlloc((tANI_U8*) pMsgBuf);
        if( eHAL_STATUS_SUCCESS != palAllocateMemory( pMac->hHdd, (void **)&pSmeJoinReq, nSize))
        {
            limLog(pMac, LOGP, FL("call to palAllocateMemory failed for pSmeJoinReq\n"));
            retCode = eSIR_SME_RESOURCES_UNAVAILABLE;
            goto end;
        }
        (void) palZeroMemory(pMac->hHdd, (void *) pSmeJoinReq, nSize);
 
        if ((limJoinReqSerDes(pMac, pSmeJoinReq, (tANI_U8 *)pMsgBuf) == eSIR_FAILURE) ||
                (!limIsSmeJoinReqValid(pMac, pSmeJoinReq)))
        {
            //                                    
            //              
            limLog(pMac, LOGW, FL("received SME_JOIN_REQ with invalid data\n"));
            retCode = eSIR_SME_INVALID_PARAMETERS;
            goto end;
        }

        //pMac->lim.gpLimJoinReq = pSmeJoinReq; TO SUPPORT BT-AMP ,review os sep 23

        /*                                               */
#ifdef FIXME_GEN6    
        if(pSmeJoinReq->bsstype == eSIR_BTAMP_AP_MODE)
        {
            if(peValidateBtJoinRequest(pMac)!= TRUE)
            {
                limLog(pMac, LOGW, FL("Start Bss session not present::SME_JOIN_REQ in unexpected state\n"));
                retCode = eSIR_SME_UNEXPECTED_REQ_RESULT_CODE;
                psessionEntry = NULL;
                goto end;   
            }
        }
        
#endif


        if((psessionEntry = peFindSessionByBssid(pMac,pSmeJoinReq->bssDescription.bssId,&sessionId)) != NULL)
        {
            limLog(pMac, LOGE, FL("Session Already exists for given BSSID\n"));
            
            if(psessionEntry->limSmeState == eLIM_SME_LINK_EST_STATE)
            {
                //                                    
                //                             
                //                               
                PELOGW(limLog(pMac, LOGW, FL("Received SME_JOIN_REQ for currently joined BSS\n"));)
                //                                    
                retCode = eSIR_SME_SUCCESS;
                goto end;
            }
            else
            {
                retCode = eSIR_SME_REFUSED;
                psessionEntry = NULL;
                goto end;
            }    
        }    
        else       /*                                              */
        {       
            /*                             */
            if((psessionEntry = peCreateSession(pMac,pSmeJoinReq->bssDescription.bssId,&sessionId, pMac->lim.maxStation)) == NULL)
            {
                limLog(pMac, LOGE, FL("Session Can not be created \n"));
                retCode = eSIR_SME_RESOURCES_UNAVAILABLE;
                goto end;
            }
        }   
        handleHTCapabilityandHTInfo(pMac, psessionEntry);

        /*                                  */
        /*                                      */
        psessionEntry->peSessionId = sessionId;

        /*                                               */
        psessionEntry->pLimJoinReq = pSmeJoinReq;
        
        /*                                      */
        psessionEntry->smeSessionId = pSmeJoinReq->sessionId;

        /*                                           */
        psessionEntry->transactionId = pSmeJoinReq->transactionId;

        /*                      */
        psessionEntry->beaconParams.beaconInterval = pSmeJoinReq->bssDescription.beaconInterval;

        /*                                                          */
        //                                                        
        sirCopyMacAddr(psessionEntry->selfMacAddr,pSmeJoinReq->selfMacAddr);
        psessionEntry->bssType = pSmeJoinReq->bsstype;

        psessionEntry->statypeForBss = STA_ENTRY_PEER;

        /*                                              */

        psessionEntry->dot11mode  = pSmeJoinReq->dot11mode;
        psessionEntry->nwType = pSmeJoinReq->bssDescription.nwType;
#ifdef WLAN_FEATURE_11AC
        psessionEntry->vhtCapability = IS_DOT11_MODE_VHT(psessionEntry->dot11mode);
        VOS_TRACE(VOS_MODULE_ID_PE, VOS_TRACE_LEVEL_INFO_MED,
            "***__limProcessSmeJoinReq: vhtCapability=%d****\n",psessionEntry->vhtCapability);
#endif

        /*        */
        psessionEntry->gLimPhyMode = pSmeJoinReq->bssDescription.nwType;

        /*                                          */
        psessionEntry->currentOperChannel = pSmeJoinReq->bssDescription.channelId;
        psessionEntry->htSupportedChannelWidthSet = (pSmeJoinReq->cbMode)?1:0; //                                                                                   
        psessionEntry->htRecommendedTxWidthSet = psessionEntry->htSupportedChannelWidthSet;
        psessionEntry->htSecondaryChannelOffset = pSmeJoinReq->cbMode;

        /*              */
        psessionEntry->pePersona = pSmeJoinReq->staPersona;
        VOS_TRACE(VOS_MODULE_ID_PE, VOS_TRACE_LEVEL_INFO,
                  FL("PE PERSONA=%d cbMode %u"), psessionEntry->pePersona, pSmeJoinReq->cbMode);
        
        /*                                                 */  
        psessionEntry->ssId.length = pSmeJoinReq->ssId.length;
        palCopyMemory( pMac->hHdd,psessionEntry->ssId.ssId,pSmeJoinReq->ssId.ssId,psessionEntry->ssId.length);
            
            /*                                                 */  
            psessionEntry->ssId.length = pSmeJoinReq->ssId.length;
            palCopyMemory( pMac->hHdd,psessionEntry->ssId.ssId,pSmeJoinReq->ssId.ssId,psessionEntry->ssId.length);
            
            //                                                       
            //                                                                   
            //                                       
#ifdef WLAN_FEATURE_VOWIFI_11R
            psessionEntry->is11Rconnection = pSmeJoinReq->is11Rconnection;
#endif
#ifdef FEATURE_WLAN_CCX
            psessionEntry->isCCXconnection = pSmeJoinReq->isCCXconnection;
#endif
#if defined WLAN_FEATURE_VOWIFI_11R || defined FEATURE_WLAN_CCX || defined(FEATURE_WLAN_LFR)
            psessionEntry->isFastTransitionEnabled = pSmeJoinReq->isFastTransitionEnabled;
#endif
            
#ifdef FEATURE_WLAN_LFR
            psessionEntry->isFastRoamIniFeatureEnabled = pSmeJoinReq->isFastRoamIniFeatureEnabled;
#endif
            if(psessionEntry->bssType == eSIR_INFRASTRUCTURE_MODE)
            {
                psessionEntry->limSystemRole = eLIM_STA_ROLE;
            }
            else if(psessionEntry->bssType == eSIR_BTAMP_AP_MODE)
            {
                psessionEntry->limSystemRole = eLIM_BT_AMP_STA_ROLE;
            }
            else
            {   
                /*                                                               */
                limLog(pMac, LOGW, FL("received SME_JOIN_REQ with invalid bss type\n"));
                retCode = eSIR_SME_INVALID_PARAMETERS;
                goto end;
            }    

        if(pSmeJoinReq->addIEScan.length)
        {
            palCopyMemory(pMac->hHdd, &psessionEntry->pLimJoinReq->addIEScan,
                          &pSmeJoinReq->addIEScan, sizeof(tSirAddie));
        }

        if(pSmeJoinReq->addIEAssoc.length)
        {
            palCopyMemory(pMac->hHdd, &psessionEntry->pLimJoinReq->addIEAssoc,
                          &pSmeJoinReq->addIEAssoc, sizeof(tSirAddie));
        }
                 
#if (WNI_POLARIS_FW_PACKAGE == ADVANCED) && defined(ANI_PRODUCT_TYPE_AP)

        val = sizeof(tLimMlmJoinReq) + sizeof(tSirMacSSidIE) +
              sizeof(tSirMacRateSetIE) + sizeof(tSirMacDsParamSetIE);
#else
        val = sizeof(tLimMlmJoinReq) + psessionEntry->pLimJoinReq->bssDescription.length + 2;
#endif
        if( eHAL_STATUS_SUCCESS != palAllocateMemory( pMac->hHdd, (void **)&pMlmJoinReq, val))
        {
            limLog(pMac, LOGP, FL("call to palAllocateMemory failed for mlmJoinReq\n"));
            return;
        }
        (void) palZeroMemory(pMac->hHdd, (void *) pMlmJoinReq, val);

        /*                                            */
        pMlmJoinReq->sessionId = psessionEntry->peSessionId;
        
        if (wlan_cfgGetInt(pMac, WNI_CFG_JOIN_FAILURE_TIMEOUT, (tANI_U32 *) &pMlmJoinReq->joinFailureTimeout)
            != eSIR_SUCCESS)
            limLog(pMac, LOGP, FL("could not retrieve JoinFailureTimer value\n"));

        /*                                         */
        palCopyMemory(pMac->hHdd, (void*)&psessionEntry->rateSet, (void*)&pSmeJoinReq->operationalRateSet,
                            sizeof(tSirMacRateSet));
        palCopyMemory(pMac->hHdd, (void*)&psessionEntry->extRateSet, (void*)&pSmeJoinReq->extendedRateSet,
                            sizeof(tSirMacRateSet));
        //                                                                                                                
        palCopyMemory(pMac->hHdd, (void*)&pMlmJoinReq->operationalRateSet, (void*)&psessionEntry->rateSet,
                           sizeof(tSirMacRateSet));

        psessionEntry->encryptType = pSmeJoinReq->UCEncryptionType;

#if (WNI_POLARIS_FW_PACKAGE == ADVANCED) && defined(ANI_PRODUCT_TYPE_AP)
        palCopyMemory( pMac->hHdd, pMlmJoinReq->bssDescription.bssId,
                       pMac->lim.gpLimJoinReq->neighborBssList.bssList[0].bssId,
                       sizeof(tSirMacAddr));
        
        pMlmJoinReq->bssDescription.capabilityInfo = 1;
           
        pMlmJoinReq->bssDescription.aniIndicator =
              (tANI_U8) pMac->lim.gpLimJoinReq->neighborBssList.bssList[0].wniIndicator;
              
        pMlmJoinReq->bssDescription.nwType =
            pMac->lim.gpLimJoinReq->neighborBssList.bssList[0].nwType;

        pMlmJoinReq->bssDescription.channelId =
            pMac->lim.gpLimJoinReq->neighborBssList.bssList[0].channelId;

            limCopyNeighborInfoToCfg(pMac,
                pMac->lim.gpLimJoinReq->neighborBssList.bssList[0], psessionEntry);

        palCopyMemory( pMac->hHdd, pMac->lim.gLimCurrentBssId,
            pMac->lim.gpLimJoinReq->neighborBssList.bssList[0].bssId,
            sizeof(tSirMacAddr));

        pMac->lim.gLimCurrentChannelId =
            pMac->lim.gpLimJoinReq->neighborBssList.bssList[0].channelId;

        pMac->lim.gLimCurrentBssCaps =
            pMac->lim.gpLimJoinReq->neighborBssList.bssList[0].capabilityInfo;
        palCopyMemory( pMac->hHdd,
         (tANI_U8 *) &pMac->lim.gLimCurrentSSID,
         (tANI_U8 *) &pMac->lim.gpLimJoinReq->neighborBssList.bssList[0].ssId,
        pMac->lim.gpLimJoinReq->neighborBssList.bssList[0].ssId.length+1);
#else
        pMlmJoinReq->bssDescription.length = psessionEntry->pLimJoinReq->bssDescription.length;

        palCopyMemory( pMac->hHdd,
           (tANI_U8 *) &pMlmJoinReq->bssDescription.bssId,
           (tANI_U8 *) &psessionEntry->pLimJoinReq->bssDescription.bssId,
           psessionEntry->pLimJoinReq->bssDescription.length + 2);

        psessionEntry->limCurrentBssCaps =
           psessionEntry->pLimJoinReq->bssDescription.capabilityInfo;

        regMax = cfgGetRegulatoryMaxTransmitPower( pMac, psessionEntry->currentOperChannel ); 
        localPowerConstraint = regMax;
        limExtractApCapability( pMac,
           (tANI_U8 *) psessionEntry->pLimJoinReq->bssDescription.ieFields,
           limGetIElenFromBssDescription(&psessionEntry->pLimJoinReq->bssDescription),
           &psessionEntry->limCurrentBssQosCaps,
           &psessionEntry->limCurrentBssPropCap,
           &pMac->lim.gLimCurrentBssUapsd //                                                          
           , &localPowerConstraint
           ); 
#ifdef FEATURE_WLAN_CCX
            psessionEntry->maxTxPower = limGetMaxTxPower(regMax, localPowerConstraint, pMac->roam.configParam.nTxPowerCap);
#else
            psessionEntry->maxTxPower = VOS_MIN( regMax , (localPowerConstraint) );
#endif
#if defined WLAN_VOWIFI_DEBUG
        limLog( pMac, LOGE, "Regulatory max = %d, local power constraint = %d, max tx = %d", regMax, localPowerConstraint, psessionEntry->maxTxPower );
#endif

        if (pMac->lim.gLimCurrentBssUapsd)
        {
            pMac->lim.gUapsdPerAcBitmask = psessionEntry->pLimJoinReq->uapsdPerAcBitmask;
            limLog( pMac, LOG1, FL("UAPSD flag for all AC - 0x%2x\n"), pMac->lim.gUapsdPerAcBitmask);

            //                                  
            pMac->lim.gUapsdPerAcDeliveryEnableMask = 0;
            pMac->lim.gUapsdPerAcTriggerEnableMask = 0;
        }
#endif

        psessionEntry->limRFBand = limGetRFBand(psessionEntry->currentOperChannel);

        //                           
        if(SIR_BAND_5_GHZ == psessionEntry->limRFBand)
        {
            if (wlan_cfgGetInt(pMac, WNI_CFG_11H_ENABLED, &val) != eSIR_SUCCESS)
                limLog(pMac, LOGP, FL("Fail to get WNI_CFG_11H_ENABLED \n"));
            psessionEntry->lim11hEnable = val;
        }
        else
            psessionEntry->lim11hEnable = 0;
        
        //                                                                              
        pMac->lim.gLimIbssCoalescingHappened = false;

            psessionEntry->limPrevSmeState = psessionEntry->limSmeState;
            psessionEntry->limSmeState = eLIM_SME_WT_JOIN_STATE;
            MTRACE(macTrace(pMac, TRACE_CODE_SME_STATE, psessionEntry->peSessionId, psessionEntry->limSmeState));

        PELOG1(limLog(pMac, LOG1, FL("SME JoinReq: SSID %d.%c%c%c%c%c%c\n"),
               psessionEntry->ssId.length,
               psessionEntry->ssId.ssId[0],
               psessionEntry->ssId.ssId[1],
               psessionEntry->ssId.ssId[2],
               psessionEntry->ssId.ssId[3],
               psessionEntry->ssId.ssId[4],
               psessionEntry->ssId.ssId[5]);
        limLog(pMac, LOG1, FL("Channel %d, BSSID %x:%x:%x:%x:%x:%x\n"),
               psessionEntry->currentOperChannel,
               psessionEntry->bssId[0],
               psessionEntry->bssId[1],
               psessionEntry->bssId[2],
               psessionEntry->bssId[3],
               psessionEntry->bssId[4],
               psessionEntry->bssId[5]);)

        /*                                                */
        psessionEntry->spectrumMgtEnabled = 
           pSmeJoinReq->spectrumMgtIndicator;
        /*                               */
        limPostMlmMessage(pMac, LIM_MLM_JOIN_REQ, (tANI_U32 *) pMlmJoinReq);
        return;

    }
    else
    {
        /*                                              */
        limLog(pMac, LOGE, FL("received unexpected SME_JOIN_REQ in state %X\n"), pMac->lim.gLimSmeState);
        limPrintSmeState(pMac, LOGE, pMac->lim.gLimSmeState);
        retCode = eSIR_SME_UNEXPECTED_REQ_RESULT_CODE;
        psessionEntry = NULL;
        goto end;
        
    }

end:
    limGetSessionInfo(pMac,(tANI_U8*)pMsgBuf,&smesessionId,&smetransactionId); 
    
    if(pSmeJoinReq)
    {
        palFreeMemory( pMac->hHdd, pSmeJoinReq);
        pSmeJoinReq = NULL;
        if (NULL != psessionEntry)
        {
            psessionEntry->pLimJoinReq = NULL;
        }
    }
    
    if(retCode != eSIR_SME_SUCCESS)
    {
        if(NULL != psessionEntry)
        {
            peDeleteSession(pMac,psessionEntry);
            psessionEntry = NULL;
        }
    } 

    limSendSmeJoinReassocRsp(pMac, eWNI_SME_JOIN_RSP, retCode, eSIR_MAC_UNSPEC_FAILURE_STATUS,psessionEntry,smesessionId,smetransactionId);
} /*                                  */


#ifdef FEATURE_WLAN_CCX
tANI_U8 limGetMaxTxPower(tPowerdBm regMax, tPowerdBm apTxPower, tPowerdBm iniTxPower)
{
    tANI_U8 maxTxPower = 0;
    tANI_U8 txPower = VOS_MIN( regMax , (apTxPower) );
    txPower = VOS_MIN(txPower, iniTxPower);
    if((txPower >= MIN_TX_PWR_CAP) && (txPower <= MAX_TX_PWR_CAP))
        maxTxPower =  txPower;
    else if (txPower < MIN_TX_PWR_CAP)
        maxTxPower = MIN_TX_PWR_CAP;
    else
        maxTxPower = MAX_TX_PWR_CAP;

    return (maxTxPower);
}
#endif

/* 
                              
  
           
                                                             
                                       
  
        
  
              
  
       
  
                                                    
                                                        
               
 */

static void
__limProcessSmeReassocReq(tpAniSirGlobal pMac, tANI_U32 *pMsgBuf)
{
    tANI_U16                caps;
    tANI_U32                val;
    tpSirSmeReassocReq pReassocReq = NULL;
    tLimMlmReassocReq  *pMlmReassocReq;
    tSirResultCodes    retCode = eSIR_SME_SUCCESS;
    tpPESession        psessionEntry = NULL;
    tANI_U8            sessionId; 
    tANI_U8            smeSessionId; 
    tANI_U16           transactionId; 
    tPowerdBm            localPowerConstraint = 0, regMax = 0;
    tANI_U32           teleBcnEn = 0;
    tANI_U16            nSize;


    PELOG3(limLog(pMac, LOG3, FL("Received REASSOC_REQ\n"));)
    
    nSize = __limGetSmeJoinReqSizeForAlloc((tANI_U8 *) pMsgBuf);
    if( eHAL_STATUS_SUCCESS != palAllocateMemory( pMac->hHdd, (void **)&pReassocReq, nSize ))
    {
        //          
        limLog(pMac, LOGP,
               FL("call to palAllocateMemory failed for pReassocReq\n"));

        retCode = eSIR_SME_RESOURCES_UNAVAILABLE;
        goto end;
    }
    (void) palZeroMemory(pMac->hHdd, (void *) pReassocReq, nSize);
    if ((limJoinReqSerDes(pMac, (tpSirSmeJoinReq) pReassocReq,
                          (tANI_U8 *) pMsgBuf) == eSIR_FAILURE) ||
        (!limIsSmeJoinReqValid(pMac,
                               (tpSirSmeJoinReq) pReassocReq)))
    {
        //                                       
        //              
        limLog(pMac, LOGW,
               FL("received SME_REASSOC_REQ with invalid data\n"));

        retCode = eSIR_SME_INVALID_PARAMETERS;
        goto end;
    }

   if((psessionEntry = peFindSessionByBssid(pMac,pReassocReq->bssDescription.bssId,&sessionId))==NULL)
    {
        limPrintMacAddr(pMac, pReassocReq->bssDescription.bssId, LOGE);
        limLog(pMac, LOGP, FL("Session does not exist for given bssId\n"));
        retCode = eSIR_SME_INVALID_PARAMETERS;
        goto end;
    }

#ifdef FEATURE_WLAN_DIAG_SUPPORT_LIM //                          
    limDiagEventReport(pMac, WLAN_PE_DIAG_REASSOC_REQ_EVENT, psessionEntry, 0, 0);
#endif //                         
    //pMac->lim.gpLimReassocReq = pReassocReq;//TO SUPPORT BT-AMP

    /*                                                                 */
    psessionEntry->pLimReAssocReq = pReassocReq;

    /* 
                                      
                                      
     */

    if (psessionEntry->limSmeState != eLIM_SME_LINK_EST_STATE)
    {
#if defined(WLAN_FEATURE_VOWIFI_11R) || defined(FEATURE_WLAN_CCX) || defined(FEATURE_WLAN_LFR)
        if (psessionEntry->limSmeState == eLIM_SME_WT_REASSOC_STATE)
        {
            //                                                                 
            limLog(pMac, LOGE, FL("Session in reassoc state is %d\n"), 
                psessionEntry->peSessionId);

            //                                
            if (!palEqualMemory( pMac->hHdd, pReassocReq->bssDescription.bssId, 
                pMac->ft.ftPEContext.pFTPreAuthReq->preAuthbssId, 6))
            {
                limPrintMacAddr(pMac, pReassocReq->bssDescription.bssId, LOGE);
                limPrintMacAddr(pMac, pMac->ft.ftPEContext.pFTPreAuthReq->preAuthbssId, LOGE);
                limLog(pMac, LOGP, FL("Unknown bssId in reassoc state\n"));
                retCode = eSIR_SME_INVALID_PARAMETERS;
                goto end;
            }

            limProcessMlmFTReassocReq(pMac, pMsgBuf, psessionEntry);
            return;
        }
#endif
        //                                               
        //              
        limLog(pMac, LOGE,
               FL("received unexpected SME_REASSOC_REQ in state %X\n"),
               psessionEntry->limSmeState);
        limPrintSmeState(pMac, LOGE, psessionEntry->limSmeState);

        retCode = eSIR_SME_UNEXPECTED_REQ_RESULT_CODE;
        goto end;
    }

    palCopyMemory( pMac->hHdd,
             psessionEntry->limReAssocbssId,
             psessionEntry->pLimReAssocReq->bssDescription.bssId,
             sizeof(tSirMacAddr));

    psessionEntry->limReassocChannelId =
         psessionEntry->pLimReAssocReq->bssDescription.channelId;

    psessionEntry->reAssocHtSupportedChannelWidthSet =
         (psessionEntry->pLimReAssocReq->cbMode)?1:0;
    psessionEntry->reAssocHtRecommendedTxWidthSet =
         psessionEntry->reAssocHtSupportedChannelWidthSet;
    psessionEntry->reAssocHtSecondaryChannelOffset =
         psessionEntry->pLimReAssocReq->cbMode;

    psessionEntry->limReassocBssCaps =
                psessionEntry->pLimReAssocReq->bssDescription.capabilityInfo;
    regMax = cfgGetRegulatoryMaxTransmitPower( pMac, psessionEntry->currentOperChannel ); 
    localPowerConstraint = regMax;
    limExtractApCapability( pMac,
              (tANI_U8 *) psessionEntry->pLimReAssocReq->bssDescription.ieFields,
              limGetIElenFromBssDescription(
                     &psessionEntry->pLimReAssocReq->bssDescription),
              &psessionEntry->limReassocBssQosCaps,
              &psessionEntry->limReassocBssPropCap,
              &pMac->lim.gLimCurrentBssUapsd //                                                    
              , &localPowerConstraint
              );

    psessionEntry->maxTxPower = VOS_MIN( regMax , (localPowerConstraint) );
#if defined WLAN_VOWIFI_DEBUG
            limLog( pMac, LOGE, "Regulatory max = %d, local power constraint = %d, max tx = %d", regMax, localPowerConstraint, psessionEntry->maxTxPower );
#endif
    {
    #if 0
    if (wlan_cfgGetStr(pMac, WNI_CFG_SSID, pMac->lim.gLimReassocSSID.ssId,
                  &cfgLen) != eSIR_SUCCESS)
    {
        //                                         
        limLog(pMac, LOGP, FL("could not retrive SSID\n"));
    }
    #endif//                 
    
    /*                                                   */
    #if 0
    palCopyMemory( pMac->hHdd, pMac->lim.gLimReassocSSID.ssId,
                   psessionEntry->ssId.ssId,
                   psessionEntry->ssId.length);
    #endif
    psessionEntry->limReassocSSID.length = pReassocReq->ssId.length;
    palCopyMemory( pMac->hHdd, psessionEntry->limReassocSSID.ssId,
                    pReassocReq->ssId.ssId, psessionEntry->limReassocSSID.length);

    }

    if (pMac->lim.gLimCurrentBssUapsd)
    {
        pMac->lim.gUapsdPerAcBitmask = psessionEntry->pLimReAssocReq->uapsdPerAcBitmask;
        limLog( pMac, LOG1, FL("UAPSD flag for all AC - 0x%2x\n"), pMac->lim.gUapsdPerAcBitmask);
    }

    if( eHAL_STATUS_SUCCESS != palAllocateMemory( pMac->hHdd, (void **)&pMlmReassocReq, sizeof(tLimMlmReassocReq)))
    {
        //          
        limLog(pMac, LOGP,
               FL("call to palAllocateMemory failed for mlmReassocReq\n"));

        retCode = eSIR_SME_RESOURCES_UNAVAILABLE;
        goto end;
    }

    palCopyMemory( pMac->hHdd, pMlmReassocReq->peerMacAddr,
                  psessionEntry->limReAssocbssId,
                  sizeof(tSirMacAddr));

    if (wlan_cfgGetInt(pMac, WNI_CFG_REASSOCIATION_FAILURE_TIMEOUT,
                  (tANI_U32 *) &pMlmReassocReq->reassocFailureTimeout)
                           != eSIR_SUCCESS)
    {
        /* 
                                                    
                               
         */
        limLog(pMac, LOGP,
               FL("could not retrieve ReassocFailureTimeout value\n"));
    }

    if (cfgGetCapabilityInfo(pMac, &caps,psessionEntry) != eSIR_SUCCESS)
    {
        /* 
                                           
                               
         */
        limLog(pMac, LOGP,
               FL("could not retrieve Capabilities value\n"));
    }
    pMlmReassocReq->capabilityInfo = caps;
    
    /*                    */
    pMlmReassocReq->sessionId = sessionId;

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

    /*                                        
                                                     
                                                      
     */

    limDelAllBASessions(pMac);

    pMlmReassocReq->listenInterval = (tANI_U16) val;

    /*                                                */
    psessionEntry->spectrumMgtEnabled = pReassocReq->spectrumMgtIndicator;

    psessionEntry->limPrevSmeState = psessionEntry->limSmeState;
    psessionEntry->limSmeState    = eLIM_SME_WT_REASSOC_STATE;

    MTRACE(macTrace(pMac, TRACE_CODE_SME_STATE, psessionEntry->peSessionId, psessionEntry->limSmeState));

    limPostMlmMessage(pMac,
                      LIM_MLM_REASSOC_REQ,
                      (tANI_U32 *) pMlmReassocReq);
    return;

end:
    if (pReassocReq)
        palFreeMemory( pMac->hHdd, pReassocReq);

    if (psessionEntry)
    {
       //                                                          
       //                                        
       smeSessionId = psessionEntry->smeSessionId;
       transactionId = psessionEntry->transactionId;
    }
    else
    {
       //                                                                   
       //                                                             
       limGetSessionInfo(pMac,(tANI_U8*)pMsgBuf, &smeSessionId, &transactionId);
    }

    //                                       
    //                                                 
    limSendSmeJoinReassocRsp(pMac, eWNI_SME_REASSOC_RSP,
                             retCode, eSIR_MAC_UNSPEC_FAILURE_STATUS,
                             psessionEntry, smeSessionId, transactionId);

} /*                                     */


tANI_BOOLEAN sendDisassocFrame = 1;
/* 
                               
  
           
                                                              
                                       
  
        
  
              
  
       
  
                                                    
                                                        
               
 */

static void
__limProcessSmeDisassocReq(tpAniSirGlobal pMac, tANI_U32 *pMsgBuf)
{
    tANI_U16                disassocTrigger, reasonCode;
    tLimMlmDisassocReq      *pMlmDisassocReq;
    tSirResultCodes         retCode = eSIR_SME_SUCCESS;
    tSirRetStatus           status;
    tSirSmeDisassocReq      smeDisassocReq;
    tpPESession             psessionEntry = NULL; 
    tANI_U8                 sessionId;
    tANI_U8                 smesessionId;
    tANI_U16                smetransactionId;

    PELOG1(limLog(pMac, LOG1,FL("received DISASSOC_REQ message\n"));)
    
    if (pMsgBuf == NULL)
    {
        limLog(pMac, LOGE, FL("Buffer is Pointing to NULL\n"));
        return;
    }

    limGetSessionInfo(pMac, (tANI_U8 *)pMsgBuf,&smesessionId, &smetransactionId);

    status = limDisassocReqSerDes(pMac, &smeDisassocReq, (tANI_U8 *) pMsgBuf);
    
    if ( (eSIR_FAILURE == status) ||
         (!limIsSmeDisassocReqValid(pMac, &smeDisassocReq, psessionEntry)) )
    {
        PELOGE(limLog(pMac, LOGE,
               FL("received invalid SME_DISASSOC_REQ message\n"));)

        if (pMac->lim.gLimRspReqd)
        {
            pMac->lim.gLimRspReqd = false;

            retCode         = eSIR_SME_INVALID_PARAMETERS;
            disassocTrigger = eLIM_HOST_DISASSOC;
            goto sendDisassoc;
        }

        return;
    }


    PELOGE(limLog(pMac, LOGE,   FL("received DISASSOC_REQ message. Reason: %d global SmeState: %d\n"), 
                                                        smeDisassocReq.reasonCode, pMac->lim.gLimSmeState);)


    if((psessionEntry = peFindSessionByBssid(pMac,smeDisassocReq.bssId,&sessionId))== NULL)
    {
        limLog(pMac, LOGE,FL("session does not exist for given bssId\n"));
        retCode = eSIR_SME_INVALID_PARAMETERS;
        disassocTrigger = eLIM_HOST_DISASSOC;
        goto sendDisassoc;
        
    }

#ifdef FEATURE_WLAN_DIAG_SUPPORT_LIM //                          
   limDiagEventReport(pMac, WLAN_PE_DIAG_DISASSOC_REQ_EVENT, psessionEntry, 0, smeDisassocReq.reasonCode);
#endif //                         
    
    /*                                             */

    psessionEntry->smeSessionId = smesessionId;
    psessionEntry->transactionId = smetransactionId;

    switch (psessionEntry->limSystemRole)
    {
        case eLIM_STA_ROLE:
        case eLIM_BT_AMP_STA_ROLE:
            switch (psessionEntry->limSmeState)
            {
                case eLIM_SME_ASSOCIATED_STATE:
                case eLIM_SME_LINK_EST_STATE:
                    psessionEntry->limPrevSmeState = psessionEntry->limSmeState;
                    psessionEntry->limSmeState= eLIM_SME_WT_DISASSOC_STATE;
                    MTRACE(macTrace(pMac, TRACE_CODE_SME_STATE, psessionEntry->peSessionId, psessionEntry->limSmeState));
                    break;

                case eLIM_SME_WT_DEAUTH_STATE:
                    /*                                                          
                                                                                 
                                                                                  
                                                                     
                     */                  
                    psessionEntry->limSmeState= eLIM_SME_WT_DISASSOC_STATE;
                    MTRACE(macTrace(pMac, TRACE_CODE_SME_STATE, psessionEntry->peSessionId, psessionEntry->limSmeState));
                    limLog(pMac, LOG1, FL("Rcvd SME_DISASSOC_REQ while in SME_WT_DEAUTH_STATE. \n"));
                    break;

                case eLIM_SME_WT_DISASSOC_STATE:
                    /*                                                                   
                                                                                            
                                                                                            
                                                                                              
                                          
                      
                                                                                                
                                                               
                     */
                    limLog(pMac, LOG1, FL("Rcvd SME_DISASSOC_REQ while in SME_WT_DISASSOC_STATE. \n"));
                    break;

                case eLIM_SME_JOIN_FAILURE_STATE: {
                    /*                                                        */
                     if (pMac->lim.gLimRspReqd) {
                        retCode = eSIR_SME_SUCCESS;  
                        disassocTrigger = eLIM_HOST_DISASSOC;
                        goto sendDisassoc;
                    }
                }break;
                default:
                    /* 
                                                       
                                                          
                     */
                    limLog(pMac, LOGE,
                       FL("received unexpected SME_DISASSOC_REQ in state %X\n"),
                       psessionEntry->limSmeState);
                    limPrintSmeState(pMac, LOGE, psessionEntry->limSmeState);

                    if (pMac->lim.gLimRspReqd)
                    {
                        if (psessionEntry->limSmeState !=
                                                eLIM_SME_WT_ASSOC_STATE)
                                    pMac->lim.gLimRspReqd = false;

                        retCode = eSIR_SME_UNEXPECTED_REQ_RESULT_CODE;
                        disassocTrigger = eLIM_HOST_DISASSOC;
                        goto sendDisassoc;
                    }

                    return;
            }

            break;

        case eLIM_AP_ROLE:
    case eLIM_BT_AMP_AP_ROLE:
            //             
            break;

        case eLIM_STA_IN_IBSS_ROLE:
        default: //                  
            limLog(pMac, LOGE,
               FL("received unexpected SME_DISASSOC_REQ for role %d\n"),
               psessionEntry->limSystemRole);

            retCode = eSIR_SME_UNEXPECTED_REQ_RESULT_CODE;
            disassocTrigger = eLIM_HOST_DISASSOC;
            goto sendDisassoc;
    } //                                      

    if (smeDisassocReq.reasonCode == eLIM_LINK_MONITORING_DISASSOC)
    {
        //                                                
        disassocTrigger = eLIM_LINK_MONITORING_DISASSOC;
        reasonCode      = eSIR_MAC_DISASSOC_DUE_TO_INACTIVITY_REASON;
    }
    else
    {
        disassocTrigger = eLIM_HOST_DISASSOC;
        reasonCode      = smeDisassocReq.reasonCode;
    }

    if (smeDisassocReq.doNotSendOverTheAir)
    {
        sendDisassocFrame = 0;     
    }
    //                                                
    if( eHAL_STATUS_SUCCESS != palAllocateMemory( pMac->hHdd, (void **)&pMlmDisassocReq, sizeof(tLimMlmDisassocReq)))
    {
        //          
        limLog(pMac, LOGP,
               FL("call to palAllocateMemory failed for mlmDisassocReq\n"));

        return;
    }

    palCopyMemory( pMac->hHdd, (tANI_U8 *) &pMlmDisassocReq->peerMacAddr,
                  (tANI_U8 *) &smeDisassocReq.peerMacAddr,
                  sizeof(tSirMacAddr));

    pMlmDisassocReq->reasonCode      = reasonCode;
    pMlmDisassocReq->disassocTrigger = disassocTrigger;
    
    /*                     */
    pMlmDisassocReq->sessionId = sessionId;
#ifdef ANI_PRODUCT_TYPE_AP
    pMlmDisassocReq->aid             = smeDisassocReq.aid;
#endif

    limPostMlmMessage(pMac,
                      LIM_MLM_DISASSOC_REQ,
                      (tANI_U32 *) pMlmDisassocReq);
    return;

sendDisassoc:
    if (psessionEntry) 
        limSendSmeDisassocNtf(pMac, smeDisassocReq.peerMacAddr,
                          retCode,
                          disassocTrigger,
#ifdef ANI_PRODUCT_TYPE_AP
                          smeDisassocReq.aid);
#else
                          1,smesessionId,smetransactionId,psessionEntry);
#endif
    else 
        limSendSmeDisassocNtf(pMac, smeDisassocReq.peerMacAddr, 
                retCode, 
                disassocTrigger,
#ifdef ANI_PRODUCT_TYPE_AP
                smeDisassocReq.aid);
#else
                1, 0, 0, NULL);
#endif


} /*                                      */


/*                                                                   
                                                                
   
                                                             
                                       
    
                                    
                                        
               
     
                                                                    */
static void
__limProcessSmeDisassocCnf(tpAniSirGlobal pMac, tANI_U32 *pMsgBuf)
{
    tSirSmeDisassocCnf  smeDisassocCnf;
    tANI_U16  aid;
    tpDphHashNode  pStaDs;
    tSirRetStatus  status = eSIR_SUCCESS;
    tpPESession         psessionEntry;
    tANI_U8             sessionId;


    PELOG1(limLog(pMac, LOG1, FL("received DISASSOC_CNF message\n"));)

    status = limDisassocCnfSerDes(pMac, &smeDisassocCnf,(tANI_U8 *) pMsgBuf);

    if (status == eSIR_FAILURE)
    {
        PELOGE(limLog(pMac, LOGE, FL("invalid SME_DISASSOC_CNF message\n"));)
        return;
    }

    if((psessionEntry = peFindSessionByBssid(pMac, smeDisassocCnf.bssId, &sessionId))== NULL)
    {
         limLog(pMac, LOGE,FL("session does not exist for given bssId\n"));
         return;
    }

    if (!limIsSmeDisassocCnfValid(pMac, &smeDisassocCnf, psessionEntry))
    {
        limLog(pMac, LOGW, FL("received invalid SME_DISASSOC_CNF message\n"));
        return;
    }

#ifdef FEATURE_WLAN_DIAG_SUPPORT_LIM //                          
    if (smeDisassocCnf.messageType == eWNI_SME_DISASSOC_CNF)
        limDiagEventReport(pMac, WLAN_PE_DIAG_DISASSOC_CNF_EVENT, psessionEntry, (tANI_U16)smeDisassocCnf.statusCode, 0);
    else if (smeDisassocCnf.messageType ==  eWNI_SME_DEAUTH_CNF)
        limDiagEventReport(pMac, WLAN_PE_DIAG_DEAUTH_CNF_EVENT, psessionEntry, (tANI_U16)smeDisassocCnf.statusCode, 0);
#endif //                         

    switch (psessionEntry->limSystemRole)
    {
        case eLIM_STA_ROLE:
        case eLIM_BT_AMP_STA_ROLE:  //              
            if ((psessionEntry->limSmeState != eLIM_SME_IDLE_STATE) &&
                (psessionEntry->limSmeState != eLIM_SME_WT_DISASSOC_STATE) &&
                (psessionEntry->limSmeState != eLIM_SME_WT_DEAUTH_STATE))
            {
                limLog(pMac, LOGE,
                   FL("received unexp SME_DISASSOC_CNF in state %X\n"),
                  psessionEntry->limSmeState);
                limPrintSmeState(pMac, LOGE, psessionEntry->limSmeState);
                return;
            }
            break;

        case eLIM_AP_ROLE:
            //             
#ifdef WLAN_SOFTAP_FEATURE
            break;
#else
            return;
#endif

        case eLIM_STA_IN_IBSS_ROLE:
        default: //                  
            limLog(pMac, LOGE,
               FL("received unexpected SME_DISASSOC_CNF role %d\n"),
               psessionEntry->limSystemRole);

            return;
    } 


    if ( (psessionEntry->limSmeState == eLIM_SME_WT_DISASSOC_STATE) || 
         (psessionEntry->limSmeState == eLIM_SME_WT_DEAUTH_STATE)
#ifdef WLAN_SOFTAP_FEATURE
          || (psessionEntry->limSystemRole == eLIM_AP_ROLE )   
#endif
     )
    {       
        pStaDs = dphLookupHashEntry(pMac, smeDisassocCnf.peerMacAddr, &aid, &psessionEntry->dph.dphHashTable);
        if (pStaDs == NULL)
        {
            PELOGW(limLog(pMac, LOGW, FL("received DISASSOC_CNF for a STA that does not have context, addr= "));
            limPrintMacAddr(pMac, smeDisassocCnf.peerMacAddr, LOGW);)
            return;
        }
        /*                                       */
        limFTCleanup(pMac);
        limCleanupRxPath(pMac, pStaDs, psessionEntry);
    }

    return;
} 


/* 
                             
  
           
                                                            
                                       
  
        
  
              
  
       
  
                                                    
                                                        
               
 */

static void
__limProcessSmeDeauthReq(tpAniSirGlobal pMac, tANI_U32 *pMsgBuf)
{
    tANI_U16                deauthTrigger, reasonCode;
    tLimMlmDeauthReq        *pMlmDeauthReq;
    tSirSmeDeauthReq        smeDeauthReq;
    tSirResultCodes         retCode = eSIR_SME_SUCCESS;
    tSirRetStatus           status = eSIR_SUCCESS;
    tpPESession             psessionEntry; 
    tANI_U8                 sessionId; //            
    tANI_U8                 smesessionId;  
    tANI_U16                smetransactionId;
    
    PELOG1(limLog(pMac, LOG1,FL("received DEAUTH_REQ message\n"));)

    status = limDeauthReqSerDes(pMac, &smeDeauthReq,(tANI_U8 *) pMsgBuf);

    limGetSessionInfo(pMac,(tANI_U8 *)pMsgBuf,&smesessionId,&smetransactionId);

    //                                                                                
    if((psessionEntry = peFindSessionByBssid(pMac, smeDeauthReq.bssId, &sessionId)) == NULL)
    {
       limLog(pMac, LOGE,FL("session does not exist for given bssId\n"));
       retCode = eSIR_SME_INVALID_PARAMETERS;
       deauthTrigger = eLIM_HOST_DEAUTH;
       goto sendDeauth;
       
    }

    if ((status == eSIR_FAILURE) || (!limIsSmeDeauthReqValid(pMac, &smeDeauthReq, psessionEntry)))
    {
        PELOGE(limLog(pMac, LOGW,FL("received invalid SME_DEAUTH_REQ message\n"));)
        if (pMac->lim.gLimRspReqd)
        {
            pMac->lim.gLimRspReqd = false;

            retCode       = eSIR_SME_INVALID_PARAMETERS;
            deauthTrigger = eLIM_HOST_DEAUTH;
            goto sendDeauth;
        }

        return;
    }

#ifdef FEATURE_WLAN_DIAG_SUPPORT_LIM //                          
    limDiagEventReport(pMac, WLAN_PE_DIAG_DEAUTH_REQ_EVENT, psessionEntry, 0, smeDeauthReq.reasonCode);
#endif //                         

    /*                                          */
    psessionEntry->smeSessionId = smesessionId;
    psessionEntry->transactionId = smetransactionId;
        

    switch (psessionEntry->limSystemRole)
    {
        case eLIM_STA_ROLE:
        case eLIM_BT_AMP_STA_ROLE:
            
            switch (psessionEntry->limSmeState)
            {
                case eLIM_SME_ASSOCIATED_STATE:
                case eLIM_SME_LINK_EST_STATE:
                case eLIM_SME_WT_ASSOC_STATE:
                case eLIM_SME_JOIN_FAILURE_STATE:
                case eLIM_SME_IDLE_STATE:
                    psessionEntry->limPrevSmeState = psessionEntry->limSmeState;
                    psessionEntry->limSmeState = eLIM_SME_WT_DEAUTH_STATE;
              MTRACE(macTrace(pMac, TRACE_CODE_SME_STATE, psessionEntry->peSessionId, pMac->lim.gLimSmeState));

                    //                                           

                    break;

                default:
                    /* 
                                                                   
                                                                 
                     */
                    limLog(pMac, LOGE,
                    FL("received unexp SME_DEAUTH_REQ in state %X\n"),psessionEntry->limSmeState);
                    limPrintSmeState(pMac, LOGE, psessionEntry->limSmeState);

                    if (pMac->lim.gLimRspReqd)
                    {
                        pMac->lim.gLimRspReqd = false;

                        retCode       = eSIR_SME_STA_NOT_AUTHENTICATED;
                        deauthTrigger = eLIM_HOST_DEAUTH;
                        goto sendDeauth;
                    }

                    return;
            }

            break;

        case eLIM_STA_IN_IBSS_ROLE:

            return;

        case eLIM_AP_ROLE:
            //             

            break;

        default:
            limLog(pMac, LOGE,
               FL("received unexpected SME_DEAUTH_REQ for role %X\n"),psessionEntry->limSystemRole);

            return;
    } //                                      

    if (smeDeauthReq.reasonCode == eLIM_LINK_MONITORING_DEAUTH)
    {
        //                                                  
        PELOG1(limLog(pMac, LOG1, FL("**** Lost link with AP ****\n"));)
        deauthTrigger = eLIM_LINK_MONITORING_DEAUTH;
        reasonCode    = eSIR_MAC_UNSPEC_FAILURE_REASON;
    }
    else
    {
        deauthTrigger = eLIM_HOST_DEAUTH;
        reasonCode    = smeDeauthReq.reasonCode;
    }

    //                                                  
    if( eHAL_STATUS_SUCCESS != palAllocateMemory( pMac->hHdd, (void **)&pMlmDeauthReq, sizeof(tLimMlmDeauthReq)))
    {
        //          
        limLog(pMac, LOGP,
               FL("call to palAllocateMemory failed for mlmDeauthReq\n"));

        return;
    }

    palCopyMemory( pMac->hHdd, (tANI_U8 *) &pMlmDeauthReq->peerMacAddr,
                  (tANI_U8 *) &smeDeauthReq.peerMacAddr,
                  sizeof(tSirMacAddr));

    pMlmDeauthReq->reasonCode = reasonCode;
    pMlmDeauthReq->deauthTrigger = deauthTrigger;
#ifdef ANI_PRODUCT_TYPE_AP
    pMlmDeauthReq->aid = smeDeauthReq.aid;
#endif

    /*                     */
    pMlmDeauthReq->sessionId = sessionId;

    limPostMlmMessage(pMac,
                      LIM_MLM_DEAUTH_REQ,
                      (tANI_U32 *) pMlmDeauthReq);
    return;

sendDeauth:
    limSendSmeDeauthNtf(pMac, smeDeauthReq.peerMacAddr,
                        retCode,
                        deauthTrigger,
#ifdef ANI_PRODUCT_TYPE_AP
                        smeDeauthReq.aid,
#else
                        1, 
#endif
                        smesessionId, smetransactionId);
} /*                                    */



/* 
                                 
  
           
                                                                
                                       
  
        
  
              
  
       
  
                                                    
                                                        
               
 */

static void
__limProcessSmeSetContextReq(tpAniSirGlobal pMac, tANI_U32 *pMsgBuf)
{
    tpSirSmeSetContextReq  pSetContextReq;
    tLimMlmSetKeysReq      *pMlmSetKeysReq;
    tpPESession             psessionEntry;
    tANI_U8                 sessionId;  //            
    tANI_U8                 smesessionId;
    tANI_U16                smetransactionId;
    

    PELOG1(limLog(pMac, LOG1,
           FL("received SETCONTEXT_REQ message\n")););

    
    if(pMsgBuf == NULL)
    {
        limLog(pMac, LOGE,FL("Buffer is Pointing to NULL\n"));
        return;
    }

    limGetSessionInfo(pMac,(tANI_U8 *)pMsgBuf,&smesessionId,&smetransactionId);
    
    if( eHAL_STATUS_SUCCESS != palAllocateMemory( pMac->hHdd, (void **)&pSetContextReq,
                                                (sizeof(tSirKeys) * SIR_MAC_MAX_NUM_OF_DEFAULT_KEYS)))
    {
        limLog(pMac, LOGP, FL("call to palAllocateMemory failed for pSetContextReq\n"));
        return;
    }

    if ((limSetContextReqSerDes(pMac, pSetContextReq, (tANI_U8 *) pMsgBuf) == eSIR_FAILURE) ||
        (!limIsSmeSetContextReqValid(pMac, pSetContextReq)))
    {
        limLog(pMac, LOGW, FL("received invalid SME_SETCONTEXT_REQ message\n")); 
        goto end;
    }

    if(pSetContextReq->keyMaterial.numKeys > SIR_MAC_MAX_NUM_OF_DEFAULT_KEYS)
    {
        PELOGE(limLog(pMac, LOGE, FL("numKeys:%d is more than SIR_MAC_MAX_NUM_OF_DEFAULT_KEYS\n"), pSetContextReq->keyMaterial.numKeys);)        
        limSendSmeSetContextRsp(pMac,
                                pSetContextReq->peerMacAddr,
#ifdef ANI_PRODUCT_TYPE_AP
                                pSetContextReq->aid,
#else
                                1,
#endif
                                eSIR_SME_INVALID_PARAMETERS,NULL,
                                smesessionId,smetransactionId);

        goto end;
    }


    if((psessionEntry = peFindSessionByBssid(pMac, pSetContextReq->bssId, &sessionId)) == NULL)
    {
        limLog(pMac, LOGW, FL("Session does not exist for given BSSID\n"));
        limSendSmeSetContextRsp(pMac,
                                pSetContextReq->peerMacAddr,
#ifdef ANI_PRODUCT_TYPE_AP
                                pSetContextReq->aid,
#else
                                1,
#endif
                                eSIR_SME_INVALID_PARAMETERS,NULL,
                                smesessionId,smetransactionId);

        goto end;
    }

#ifdef FEATURE_WLAN_DIAG_SUPPORT_LIM //                          
    limDiagEventReport(pMac, WLAN_PE_DIAG_SETCONTEXT_REQ_EVENT, psessionEntry, 0, 0);
#endif //                         


    if ((((psessionEntry->limSystemRole == eLIM_STA_ROLE) || (psessionEntry->limSystemRole == eLIM_BT_AMP_STA_ROLE)) &&
         (psessionEntry->limSmeState == eLIM_SME_LINK_EST_STATE)) ||
        (((psessionEntry->limSystemRole == eLIM_STA_IN_IBSS_ROLE) ||
          (psessionEntry->limSystemRole == eLIM_AP_ROLE)|| (psessionEntry->limSystemRole == eLIM_BT_AMP_AP_ROLE)) &&
         (psessionEntry->limSmeState == eLIM_SME_NORMAL_STATE)))
    {
        //                        
        if( eHAL_STATUS_SUCCESS != palAllocateMemory( pMac->hHdd, (void **)&pMlmSetKeysReq, sizeof(tLimMlmSetKeysReq)))
        {
            //          
            limLog(pMac, LOGP, FL("call to palAllocateMemory failed for mlmSetKeysReq\n"));
            goto end;
        }

        pMlmSetKeysReq->edType  = pSetContextReq->keyMaterial.edType;
        pMlmSetKeysReq->numKeys = pSetContextReq->keyMaterial.numKeys;
        if(pMlmSetKeysReq->numKeys > SIR_MAC_MAX_NUM_OF_DEFAULT_KEYS)
        {
            limLog(pMac, LOGP, FL("Num of keys exceeded max num of default keys limit\n"));
            goto end;
        }
        palCopyMemory( pMac->hHdd, (tANI_U8 *) &pMlmSetKeysReq->peerMacAddr,
                      (tANI_U8 *) &pSetContextReq->peerMacAddr,
                      sizeof(tSirMacAddr));

#ifdef ANI_PRODUCT_TYPE_AP
        pMlmSetKeysReq->aid = pSetContextReq->aid;
#endif

        palCopyMemory( pMac->hHdd, (tANI_U8 *) &pMlmSetKeysReq->key,
                      (tANI_U8 *) &pSetContextReq->keyMaterial.key,
                      sizeof(tSirKeys) * (pMlmSetKeysReq->numKeys ? pMlmSetKeysReq->numKeys : 1));

        pMlmSetKeysReq->sessionId = sessionId;
#ifdef WLAN_FEATURE_VOWIFI_11R_DEBUG
        PELOG1(limLog(pMac, LOG1,
           FL("received SETCONTEXT_REQ message sessionId=%d\n"), pMlmSetKeysReq->sessionId););
#endif

#ifdef WLAN_SOFTAP_FEATURE
        if(((pSetContextReq->keyMaterial.edType == eSIR_ED_WEP40) || (pSetContextReq->keyMaterial.edType == eSIR_ED_WEP104))
        && (psessionEntry->limSystemRole == eLIM_AP_ROLE))
        {
            if(pSetContextReq->keyMaterial.key[0].keyLength)
            {
                tANI_U8 keyId;
                keyId = pSetContextReq->keyMaterial.key[0].keyId;
                palCopyMemory(pMac, (tANI_U8 *)&psessionEntry->WEPKeyMaterial[keyId],
                   (tANI_U8 *) &pSetContextReq->keyMaterial, sizeof(tSirKeyMaterial));
            }
            else {
                tANI_U32 i;
                for( i = 0; i < SIR_MAC_MAX_NUM_OF_DEFAULT_KEYS; i++)
                {
                    palCopyMemory( pMac->hHdd, (tANI_U8 *) &pMlmSetKeysReq->key[i],
                        (tANI_U8 *)psessionEntry->WEPKeyMaterial[i].key, sizeof(tSirKeys));
                }
            }
        }
#endif

        limPostMlmMessage(pMac, LIM_MLM_SETKEYS_REQ, (tANI_U32 *) pMlmSetKeysReq);

#ifdef ANI_AP_SDK
        /*                                                                
                                                                         */
        if ( (psessionEntry->limSystemRole == eLIM_STA_ROLE) &&
             (psessionEntry->limSmeState == eLIM_SME_LINK_EST_STATE))
        {
            tpDphHashNode pSta;
            pSta = dphGetHashEntry(pMac, 0, &psessionEntry->dph.dphHashTable);
            if (pSta)
                pSta->staAuthenticated = 1;
        }
#endif
    }
    else
    {
        limLog(pMac, LOGE,
           FL("received unexpected SME_SETCONTEXT_REQ for role %d, state=%X\n"),
           psessionEntry->limSystemRole,
           psessionEntry->limSmeState);
        limPrintSmeState(pMac, LOGE, psessionEntry->limSmeState);

        limSendSmeSetContextRsp(pMac, pSetContextReq->peerMacAddr,
#ifdef ANI_PRODUCT_TYPE_AP
                                pSetContextReq->aid,
#else
                                1,
#endif
                                eSIR_SME_UNEXPECTED_REQ_RESULT_CODE,psessionEntry,
                                smesessionId,
                                smetransactionId);
    }

end:
    palFreeMemory( pMac->hHdd, pSetContextReq);
    return;
} /*                                        */

/* 
                                
  
           
                                                               
                                       
  
        
  
              
  
       
  
                                                    
                                                        
               
 */

static void
__limProcessSmeRemoveKeyReq(tpAniSirGlobal pMac, tANI_U32 *pMsgBuf)
{
    tpSirSmeRemoveKeyReq    pRemoveKeyReq;
    tLimMlmRemoveKeyReq     *pMlmRemoveKeyReq;
    tpPESession             psessionEntry;
    tANI_U8                 sessionId;  //            
    tANI_U8                 smesessionId;  
    tANI_U16                smetransactionId;

    PELOG1(limLog(pMac, LOG1,
           FL("received REMOVEKEY_REQ message\n"));)

    if(pMsgBuf == NULL)
    {
        limLog(pMac, LOGE,FL("Buffer is Pointing to NULL\n"));
           return;
    }

    
    limGetSessionInfo(pMac,(tANI_U8 *)pMsgBuf,&smesessionId,&smetransactionId);

    if( eHAL_STATUS_SUCCESS != palAllocateMemory( pMac->hHdd, (void **)&pRemoveKeyReq,
                                                (sizeof(*pRemoveKeyReq))))
    {
        //         
        limLog(pMac, LOGP,
               FL("call to palAllocateMemory failed for pRemoveKeyReq\n"));

        return;
     }

    if ((limRemoveKeyReqSerDes(pMac,
                                pRemoveKeyReq,
                                (tANI_U8 *) pMsgBuf) == eSIR_FAILURE))
    {
        limLog(pMac, LOGW,
               FL("received invalid SME_REMOVECONTEXT_REQ message\n"));

        /*                                                                                        */

        if((psessionEntry = peFindSessionByBssid(pMac,pRemoveKeyReq->bssId,&sessionId))== NULL)
        {     
            limLog(pMac, LOGE,FL("session does not exist for given bssId\n"));
            //         
        }

        limSendSmeRemoveKeyRsp(pMac,
                                pRemoveKeyReq->peerMacAddr,
                                eSIR_SME_INVALID_PARAMETERS,psessionEntry,
                                smesessionId,smetransactionId);

        goto end;
    }

    if((psessionEntry = peFindSessionByBssid(pMac,pRemoveKeyReq->bssId, &sessionId))== NULL)
    {
        limLog(pMac, LOGE,
                      FL("session does not exist for given bssId\n"));
        limSendSmeRemoveKeyRsp(pMac,
                                pRemoveKeyReq->peerMacAddr,
                                eSIR_SME_UNEXPECTED_REQ_RESULT_CODE, NULL,
                                smesessionId, smetransactionId);
        goto end;
    }


    if ((((psessionEntry->limSystemRole == eLIM_STA_ROLE)|| (psessionEntry->limSystemRole == eLIM_BT_AMP_STA_ROLE))&&
         (psessionEntry->limSmeState == eLIM_SME_LINK_EST_STATE)) ||
        (((psessionEntry->limSystemRole == eLIM_STA_IN_IBSS_ROLE) ||
          (psessionEntry->limSystemRole == eLIM_AP_ROLE)|| (psessionEntry->limSystemRole == eLIM_BT_AMP_AP_ROLE)) &&
         (psessionEntry->limSmeState == eLIM_SME_NORMAL_STATE)))
    {
        //                           
        if( eHAL_STATUS_SUCCESS != palAllocateMemory( pMac->hHdd, (void **)&pMlmRemoveKeyReq, sizeof(tLimMlmRemoveKeyReq)))
        {
            //          
            limLog(pMac, LOGP,
                   FL("call to palAllocateMemory failed for mlmRemoveKeysReq\n"));

            goto end;
        }

        pMlmRemoveKeyReq->edType  = (tAniEdType)pRemoveKeyReq->edType; 
        pMlmRemoveKeyReq->keyId = pRemoveKeyReq->keyId;
        pMlmRemoveKeyReq->wepType = pRemoveKeyReq->wepType;
        pMlmRemoveKeyReq->unicast = pRemoveKeyReq->unicast;
        
        /*                      */
        pMlmRemoveKeyReq->sessionId = sessionId;

        palCopyMemory( pMac->hHdd, (tANI_U8 *) &pMlmRemoveKeyReq->peerMacAddr,
                      (tANI_U8 *) &pRemoveKeyReq->peerMacAddr,
                      sizeof(tSirMacAddr));


        limPostMlmMessage(pMac,
                          LIM_MLM_REMOVEKEY_REQ,
                          (tANI_U32 *) pMlmRemoveKeyReq);
    }
    else
    {
        limLog(pMac, LOGE,
           FL("received unexpected SME_REMOVEKEY_REQ for role %d, state=%X\n"),
           psessionEntry->limSystemRole,
           psessionEntry->limSmeState);
        limPrintSmeState(pMac, LOGE, psessionEntry->limSmeState);

        limSendSmeRemoveKeyRsp(pMac,
                                pRemoveKeyReq->peerMacAddr,
                                eSIR_SME_UNEXPECTED_REQ_RESULT_CODE,psessionEntry,
                                smesessionId,smetransactionId);
    }

end:
    palFreeMemory( pMac->hHdd, pRemoveKeyReq);
} /*                                       */

void limProcessSmeGetScanChannelInfo(tpAniSirGlobal pMac, tANI_U32 *pMsgBuf)
{
    tSirMsgQ         mmhMsg;
    tpSmeGetScanChnRsp  pSirSmeRsp;
    tANI_U16 len = 0;

    if(pMac->lim.scanChnInfo.numChnInfo > SIR_MAX_SUPPORTED_CHANNEL_LIST)
    {
        limLog(pMac, LOGW, FL("numChn is out of bounds %d\n"),
                pMac->lim.scanChnInfo.numChnInfo);
        pMac->lim.scanChnInfo.numChnInfo = SIR_MAX_SUPPORTED_CHANNEL_LIST;
    }

    PELOG2(limLog(pMac, LOG2,
           FL("Sending message %s with number of channels %d\n"),
           limMsgStr(eWNI_SME_GET_SCANNED_CHANNEL_RSP), pMac->lim.scanChnInfo.numChnInfo);)

    len = sizeof(tSmeGetScanChnRsp) + (pMac->lim.scanChnInfo.numChnInfo - 1) * sizeof(tLimScanChn);
    if( eHAL_STATUS_SUCCESS != palAllocateMemory( pMac->hHdd, (void **)&pSirSmeRsp, len ))
    {
        //                                 
        limLog(pMac, LOGP,
               FL("call to palAllocateMemory failed for JOIN/REASSOC_RSP\n"));

        return;
    }
    palZeroMemory(pMac->hHdd, pSirSmeRsp, len);

#if defined(ANI_PRODUCT_TYPE_AP) && defined(ANI_LITTLE_BYTE_ENDIAN)
    sirStoreU16N((tANI_U8*)&pSirSmeRsp->mesgType, eWNI_SME_GET_SCANNED_CHANNEL_RSP);
    sirStoreU16N((tANI_U8*)&pSirSmeRsp->mesgLen, len);
#else
    pSirSmeRsp->mesgType = eWNI_SME_GET_SCANNED_CHANNEL_RSP;
    pSirSmeRsp->mesgLen = len;
#endif
    pSirSmeRsp->sessionId = 0;

    if(pMac->lim.scanChnInfo.numChnInfo)
    {
        pSirSmeRsp->numChn = pMac->lim.scanChnInfo.numChnInfo;
        palCopyMemory(pMac->hHdd, pSirSmeRsp->scanChn, pMac->lim.scanChnInfo.scanChn, sizeof(tLimScanChn) * pSirSmeRsp->numChn);
    }
    //              
    limRessetScanChannelInfo(pMac);

    mmhMsg.type = eWNI_SME_GET_SCANNED_CHANNEL_RSP;
    mmhMsg.bodyptr = pSirSmeRsp;
    mmhMsg.bodyval = 0;
  
    pMac->lim.gLimRspReqd = false;
    MTRACE(macTraceMsgTx(pMac, NO_SESSION, mmhMsg.type));
    limSysProcessMmhMsgApi(pMac, &mmhMsg,  ePROT);
}


#ifdef WLAN_SOFTAP_FEATURE
void limProcessSmeGetAssocSTAsInfo(tpAniSirGlobal pMac, tANI_U32 *pMsgBuf)
{
    tSirSmeGetAssocSTAsReq  getAssocSTAsReq;
    tpDphHashNode           pStaDs = NULL;
    tpPESession             psessionEntry = NULL;
    tSap_Event              sapEvent;
    tpWLAN_SAPEventCB       pSapEventCallback = NULL;
    tpSap_AssocMacAddr      pAssocStasTemp = NULL;//                    
    tANI_U8                 sessionId = CSR_SESSION_ID_INVALID;
    tANI_U8                 assocId = 0;
    tANI_U8                 staCount = 0;

    if (!limIsSmeGetAssocSTAsReqValid(pMac, &getAssocSTAsReq, (tANI_U8 *) pMsgBuf))
    {
        limLog(pMac, LOGE,
                        FL("received invalid eWNI_SME_GET_ASSOC_STAS_REQ message\n"));
        goto limAssocStaEnd;
    }

    switch (getAssocSTAsReq.modId)
    {
/*         
                               
                                          
                   

                              
                       
                   
*/
        case VOS_MODULE_ID_PE:
        default:
            break;
    }

    //                                 
    //                      
    if ((psessionEntry = peFindSessionByBssid(pMac, getAssocSTAsReq.bssId, &sessionId)) == NULL)
    {
        limLog(pMac, LOGE,
                        FL("session does not exist for given bssId\n"));
        goto limAssocStaEnd;
    }

    if (psessionEntry->limSystemRole != eLIM_AP_ROLE)
    {
        limLog(pMac, LOGE,
                        FL("Received unexpected message in state %X, in role %X\n"),
                        psessionEntry->limSmeState , psessionEntry->limSystemRole);
        goto limAssocStaEnd;
    }

    //                                                
    pSapEventCallback   = (tpWLAN_SAPEventCB)getAssocSTAsReq.pSapEventCallback;
    pAssocStasTemp      = (tpSap_AssocMacAddr)getAssocSTAsReq.pAssocStasArray;

    for (assocId = 0; assocId < psessionEntry->dph.dphHashTable.size; assocId++)//                             
    {
        pStaDs = dphGetHashEntry(pMac, assocId, &psessionEntry->dph.dphHashTable);

        if (NULL == pStaDs)
            continue;

        if (pStaDs->valid)
        {
            palCopyMemory(pMac->hHdd, (tANI_U8 *)&pAssocStasTemp->staMac,
                                        (tANI_U8 *)&pStaDs->staAddr,
                                        sizeof(v_MACADDR_t));  //            
            pAssocStasTemp->assocId = (v_U8_t)pStaDs->assocId;         //               
            pAssocStasTemp->staId   = (v_U8_t)pStaDs->staIndex;        //           

            limLog(pMac, LOG1, FL("dph Station Number = %d"), staCount+1);
            limLog(pMac, LOG1, FL("MAC = %02x:%02x:%02x:%02x:%02x:%02x"),
                                        pStaDs->staAddr[0],
                                        pStaDs->staAddr[1],
                                        pStaDs->staAddr[2],
                                        pStaDs->staAddr[3],
                                        pStaDs->staAddr[4],
                                        pStaDs->staAddr[5]);
            limLog(pMac, LOG1, FL("Association Id = %d"),pStaDs->assocId);
            limLog(pMac, LOG1, FL("Station Index = %d"),pStaDs->staIndex);

            pAssocStasTemp++;
            staCount++;
        }
    }

limAssocStaEnd:
    //                                                                                 
    if (pSapEventCallback != NULL)
    {
        sapEvent.sapHddEventCode = eSAP_ASSOC_STA_CALLBACK_EVENT;
        sapEvent.sapevt.sapAssocStaListEvent.module = VOS_MODULE_ID_PE;
        sapEvent.sapevt.sapAssocStaListEvent.noOfAssocSta = staCount;
        sapEvent.sapevt.sapAssocStaListEvent.pAssocStas = (tpSap_AssocMacAddr)getAssocSTAsReq.pAssocStasArray;
        pSapEventCallback(&sapEvent, getAssocSTAsReq.pUsrContext);
    }
}


/* 
                                 
  
           
                                                                             
  
        
                                                                                           
                                           
              
  
  
       
  
                                                   
                                                              
 
               
 */
void limProcessSmeGetWPSPBCSessions(tpAniSirGlobal pMac, tANI_U32 *pMsgBuf)
{
    tSirSmeGetWPSPBCSessionsReq  GetWPSPBCSessionsReq;
    tpPESession                  psessionEntry = NULL;
    tSap_Event                   sapEvent;
    tpWLAN_SAPEventCB            pSapEventCallback = NULL;
    tANI_U8                      sessionId = CSR_SESSION_ID_INVALID;
    tSirMacAddr                  zeroMac = {0,0,0,0,0,0};
        
    sapEvent.sapevt.sapGetWPSPBCSessionEvent.status = VOS_STATUS_E_FAULT;
    
    if (limIsSmeGetWPSPBCSessionsReqValid(pMac,  &GetWPSPBCSessionsReq, (tANI_U8 *) pMsgBuf) != eSIR_SUCCESS)
    {
        limLog(pMac, LOGE,
                        FL("received invalid eWNI_SME_GET_ASSOC_STAS_REQ message\n"));
        goto limGetWPSPBCSessionsEnd;
    }

    //                                 
    //                      
    if ((psessionEntry = peFindSessionByBssid(pMac, GetWPSPBCSessionsReq.bssId, &sessionId)) == NULL)
    {
        limLog(pMac, LOGE,
                        FL("session does not exist for given bssId\n"));
        goto limGetWPSPBCSessionsEnd;
    }

    if (psessionEntry->limSystemRole != eLIM_AP_ROLE)
    {
        limLog(pMac, LOGE,
                        FL("Received unexpected message in role %X\n"),
                        psessionEntry->limSystemRole);
        goto limGetWPSPBCSessionsEnd;
    }

    //                                                                         
    sapEvent.sapHddEventCode =  eSAP_GET_WPSPBC_SESSION_EVENT;
    sapEvent.sapevt.sapGetWPSPBCSessionEvent.module = VOS_MODULE_ID_PE;

    if (palEqualMemory(pMac->hHdd, zeroMac, GetWPSPBCSessionsReq.pRemoveMac, sizeof(tSirMacAddr)))
    { //                          

      limGetWPSPBCSessions(pMac,
              sapEvent.sapevt.sapGetWPSPBCSessionEvent.addr.bytes, sapEvent.sapevt.sapGetWPSPBCSessionEvent.UUID_E, 
              &sapEvent.sapevt.sapGetWPSPBCSessionEvent.wpsPBCOverlap, psessionEntry);
    }
    else
    {
      limRemovePBCSessions(pMac, GetWPSPBCSessionsReq.pRemoveMac,psessionEntry);
      /*                                   */
      return;
    }
    
    PELOG4(limLog(pMac, LOGE, FL("wpsPBCOverlap %d\n"), sapEvent.sapevt.sapGetWPSPBCSessionEvent.wpsPBCOverlap);)
    PELOG4(limPrintMacAddr(pMac, sapEvent.sapevt.sapGetWPSPBCSessionEvent.addr.bytes, LOG4);)
    
    sapEvent.sapevt.sapGetWPSPBCSessionEvent.status = VOS_STATUS_SUCCESS;
  
limGetWPSPBCSessionsEnd:
    pSapEventCallback   = (tpWLAN_SAPEventCB)GetWPSPBCSessionsReq.pSapEventCallback;
    pSapEventCallback(&sapEvent, GetWPSPBCSessionsReq.pUsrContext);
}

#endif


/* 
                         
  
            
                                                             
                          
  
                                                       
                                                         
                            
  
              
  
       
  
                                                    
               
 */

static void
__limCounterMeasures(tpAniSirGlobal pMac, tpPESession psessionEntry)
{
    tSirMacAddr mac = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
    if ( (psessionEntry->limSystemRole == eLIM_AP_ROLE) || (psessionEntry->limSystemRole == eLIM_BT_AMP_AP_ROLE)
        || (psessionEntry->limSystemRole == eLIM_BT_AMP_STA_ROLE) )
         
        limSendDisassocMgmtFrame(pMac, eSIR_MAC_MIC_FAILURE_REASON, mac, psessionEntry);

    tx_thread_sleep(10);
};


#ifdef WLAN_SOFTAP_FEATURE
void
limProcessTkipCounterMeasures(tpAniSirGlobal pMac, tANI_U32 *pMsgBuf)
{
    tSirSmeTkipCntrMeasReq  tkipCntrMeasReq;
    tpPESession             psessionEntry;
    tANI_U8                 sessionId;      //            

    if ( limTkipCntrMeasReqSerDes( pMac, &tkipCntrMeasReq, (tANI_U8 *) pMsgBuf ) != eSIR_SUCCESS )
    {
        limLog(pMac, LOGE,
                        FL("received invalid eWNI_SME_TKIP_CNTR_MEAS_REQ message\n"));
        return;
    }

    if ( NULL == (psessionEntry = peFindSessionByBssid( pMac, tkipCntrMeasReq.bssId, &sessionId )) )
    {
        limLog(pMac, LOGE, FL("session does not exist for given BSSID \n"));
        return;
    }

    if ( tkipCntrMeasReq.bEnable )
    {
        __limCounterMeasures( pMac, psessionEntry );
    }

    psessionEntry->bTkipCntrMeasActive = tkipCntrMeasReq.bEnable;
}
#endif


static void
__limHandleSmeStopBssRequest(tpAniSirGlobal pMac, tANI_U32 *pMsgBuf)
{
    tSirSmeStopBssReq  stopBssReq;
    tSirRetStatus      status;
    tLimSmeStates      prevState;
    tANI_U8            sessionId;  //            
    tpPESession        psessionEntry;
    tANI_U8            smesessionId;
    tANI_U16           smetransactionId;
    
    limGetSessionInfo(pMac,(tANI_U8 *)pMsgBuf,&smesessionId,&smetransactionId);
        


    if ((limStopBssReqSerDes(pMac, &stopBssReq, (tANI_U8 *) pMsgBuf) != eSIR_SUCCESS) ||
        !limIsSmeStopBssReqValid(pMsgBuf))
    {
        PELOGW(limLog(pMac, LOGW, FL("received invalid SME_STOP_BSS_REQ message\n"));)
        //                                
        limSendSmeRsp(pMac, eWNI_SME_STOP_BSS_RSP, eSIR_SME_INVALID_PARAMETERS,smesessionId,smetransactionId);
        return;
    }

 
    if((psessionEntry = peFindSessionByBssid(pMac,stopBssReq.bssId,&sessionId)) == NULL)
    {
        limLog(pMac, LOGW, FL("session does not exist for given BSSID \n"));
        limSendSmeRsp(pMac, eWNI_SME_STOP_BSS_RSP, eSIR_SME_INVALID_PARAMETERS,smesessionId,smetransactionId);
        return;
    }

#ifdef FEATURE_WLAN_DIAG_SUPPORT_LIM //                          
    limDiagEventReport(pMac, WLAN_PE_DIAG_STOP_BSS_REQ_EVENT, psessionEntry, 0, 0);
#endif //                         


    if ((psessionEntry->limSmeState != eLIM_SME_NORMAL_STATE) ||    /*                           */
        (psessionEntry->limSystemRole == eLIM_STA_ROLE ))
    {
        /* 
                                                          
                                                                
                                                       
         */
        limLog(pMac, LOGE,
           FL("received unexpected SME_STOP_BSS_REQ in state %X, for role %d\n"),
           psessionEntry->limSmeState, psessionEntry->limSystemRole);
        limPrintSmeState(pMac, LOGE, psessionEntry->limSmeState);
        //                                
        limSendSmeRsp(pMac, eWNI_SME_STOP_BSS_RSP, eSIR_SME_UNEXPECTED_REQ_RESULT_CODE,smesessionId,smetransactionId);
        return;
    }

#ifdef WLAN_SOFTAP_FEATURE
    if (psessionEntry->limSystemRole == eLIM_AP_ROLE )
    {
        limWPSPBCClose(pMac, psessionEntry);
    }
#endif
    PELOGW(limLog(pMac, LOGW, FL("RECEIVED STOP_BSS_REQ with reason code=%d\n"), stopBssReq.reasonCode);)

    prevState = psessionEntry->limSmeState;

    psessionEntry->limSmeState = eLIM_SME_IDLE_STATE;
    MTRACE(macTrace(pMac, TRACE_CODE_SME_STATE, psessionEntry->peSessionId, psessionEntry->limSmeState));

    /*                                          */
    psessionEntry->smeSessionId = smesessionId;
    psessionEntry->transactionId = smetransactionId;

    /*                                                          */
    if ( (eLIM_STA_IN_IBSS_ROLE != psessionEntry->limSystemRole) && (eLIM_BT_AMP_STA_ROLE != psessionEntry->limSystemRole) )
    {
        tSirMacAddr   bcAddr = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
        if ((stopBssReq.reasonCode == eSIR_SME_MIC_COUNTER_MEASURES))
            //                                                
            __limCounterMeasures(pMac,psessionEntry);
        else
            limSendDisassocMgmtFrame(pMac, eSIR_MAC_DISASSOC_LEAVING_BSS_REASON, bcAddr,psessionEntry);
    }

    //                                                                                             
    pMac->lim.gLimIbssCoalescingHappened = false;
    
    /*                                              */
    status = limDelBss(pMac, NULL,psessionEntry->bssIdx,psessionEntry);
    
    if (status != eSIR_SUCCESS)
    {
        PELOGE(limLog(pMac, LOGE, FL("delBss failed for bss %d\n"), psessionEntry->bssIdx);)
        psessionEntry->limSmeState= prevState;

        MTRACE(macTrace(pMac, TRACE_CODE_SME_STATE, psessionEntry->peSessionId, psessionEntry->limSmeState));
   
        limSendSmeRsp(pMac, eWNI_SME_STOP_BSS_RSP, eSIR_SME_STOP_BSS_FAILURE,smesessionId,smetransactionId);
    }
}


/*                                                               
                                 

                                                             
                                                              
                                                            
                                                        
            
            

                                        
                                            
                                                                */
static tANI_BOOLEAN
__limProcessSmeStopBssReq(tpAniSirGlobal pMac, tpSirMsgQ pMsg)
{
    if (__limIsDeferedMsgForLearn(pMac, pMsg))
    {
        /* 
                                                          
                          
         */
        return eANI_BOOLEAN_FALSE;
    }
    __limHandleSmeStopBssRequest(pMac, (tANI_U32 *) pMsg->bodyptr);
    return eANI_BOOLEAN_TRUE;
} /*                                     */


void limProcessSmeDelBssRsp(
    tpAniSirGlobal  pMac,
    tANI_U32        body,tpPESession psessionEntry)
{

    (void) body;
    SET_LIM_PROCESS_DEFD_MESGS(pMac, true);
    //                         
    dphHashTableClassInit(pMac, &psessionEntry->dph.dphHashTable);
    limDeletePreAuthList(pMac);
    limIbssDelete(pMac,psessionEntry);
    limSendSmeRsp(pMac, eWNI_SME_STOP_BSS_RSP, eSIR_SME_SUCCESS,psessionEntry->smeSessionId,psessionEntry->transactionId);
    return;
}


#if 0
/* 
                                  
  
           
                                                                 
                                       
  
        
  
              
  
       
  
                                                    
                                                        
               
 */

static void
__limProcessSmePromiscuousReq(tpAniSirGlobal pMac, tANI_U32 *pMsgBuf)
{

    tANI_U32                cfg = sizeof(tSirMacAddr);
    tSirMacAddr        currentBssId;
    tLimMlmDisassocReq *pMlmDisassocReq;
    tSirMacAddr        bcAddr = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
    
    
    if(pMsgBuf == NULL)
    {
        limLog(pMac, LOGE,FL("Buffer is Pointing to NULL\n"));
           return;
    }

    PELOG1(limLog(pMac, LOG1,
           FL("received PROMISCUOUS_REQ message\n"));)

    if (wlan_cfgGetStr(pMac, WNI_CFG_BSSID, currentBssId, &cfg) !=
                                eSIR_SUCCESS)
    {
        //                                          
        limLog(pMac, LOGP, FL("could not retrieve BSSID\n"));
    }

    if ((((pMac->lim.gLimSystemRole == eLIM_STA_ROLE) ||
         (pMac->lim.gLimSystemRole == eLIM_STA_IN_IBSS_ROLE)) &&
         ((pMac->lim.gLimSmeState == eLIM_SME_ASSOCIATED_STATE) ||
          (pMac->lim.gLimSmeState == eLIM_SME_LINK_EST_STATE))) ||
        ((pMac->lim.gLimSystemRole == eLIM_AP_ROLE) &&
         (pMac->lim.gLimSmeState == eLIM_SME_NORMAL_STATE)))
    {
        //                                                
        if( eHAL_STATUS_SUCCESS != palAllocateMemory( pMac->hHdd, (void **)&pMlmDisassocReq, sizeof(tLimMlmDisassocReq)))
        {
            //          
            limLog(pMac, LOGP,
                   FL("call to palAllocateMemory failed for mlmDisassocReq\n"));

            return;
        }

        if (pMac->lim.gLimSystemRole == eLIM_AP_ROLE)
            palCopyMemory( pMac->hHdd, (tANI_U8 *) &pMlmDisassocReq->peerMacAddr,
                          (tANI_U8 *) &bcAddr,
                          sizeof(tSirMacAddr));
        else
            palCopyMemory( pMac->hHdd, pMlmDisassocReq->peerMacAddr,
                          currentBssId,
                          sizeof(tSirMacAddr));

        pMlmDisassocReq->reasonCode      =
                                eSIR_MAC_DISASSOC_LEAVING_BSS_REASON;
        pMlmDisassocReq->disassocTrigger =
                                      eLIM_PROMISCUOUS_MODE_DISASSOC;

        pMac->lim.gLimPrevSmeState = pMac->lim.gLimSmeState;
        pMac->lim.gLimSmeState     = eLIM_SME_WT_DISASSOC_STATE;
     MTRACE(macTrace(pMac, TRACE_CODE_SME_STATE, 0, pMac->lim.gLimSmeState));

        limPostMlmMessage(pMac,
                          LIM_MLM_DISASSOC_REQ,
                          (tANI_U32 *) pMlmDisassocReq);
    }
    else
    {
        //                                       
        limSendSmePromiscuousModeRsp(pMac);

        pMac->lim.gLimSmeState = eLIM_SME_OFFLINE_STATE;
     MTRACE(macTrace(pMac, TRACE_CODE_SME_STATE, 0, pMac->lim.gLimSmeState));
    }

} /*                                         */
#endif


/*                                                                
                                  
                                                            
                    
 
            
                              
                                                     
            
                                                                  */

  void
__limProcessSmeAssocCnfNew(tpAniSirGlobal pMac, tANI_U32 msgType, tANI_U32 *pMsgBuf)
{
    tSirSmeAssocCnf    assocCnf;
    tpDphHashNode      pStaDs = NULL;
    tpPESession        psessionEntry= NULL;
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
#ifdef WLAN_SOFTAP_FEATURE
           (msgType != eWNI_SME_ASSOC_CNF))))) //                                                              
#else
           (msgType != eWNI_SME_REASSOC_CNF)))))
#endif
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
        /*                                                                                                                       */
        if(!pStaDs->mlmStaContext.updateContext)
           pStaDs->mlmStaContext.updateContext = 1;
        limRejectAssociation(pMac, pStaDs->staAddr,
                             pStaDs->mlmStaContext.subType,
                             true, pStaDs->mlmStaContext.authType,
                             pStaDs->assocId, true,
                             eSIR_MAC_UNSPEC_FAILURE_STATUS, psessionEntry);
        return;
    }

end:
    if((psessionEntry != NULL) && (pStaDs != NULL))
    {
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
    }

} /*                                      */


#ifdef ANI_PRODUCT_TYPE_AP
/* 
                            
  
           
                                                          
                                                    
  
        
  
              
  
       
  
                                                    
                                                        
  
               
 */

static void
__limProcessSmeAssocCnf(tpAniSirGlobal pMac, tANI_U32 msgType, tANI_U32 *pMsgBuf)
{
    tSirSmeAssocCnf    assocCnf;
    tpDphHashNode      pStaDs;
    tpPESession        psessionEntry;
    tANI_U8            sessionId; 
    tHalBitVal cbBitVal;
    tANI_U8 tspecIdx = 0; //                             

    if(pMsgBuf == NULL)
    {
        limLog(pMac, LOGE,FL("Buffer is Pointing to NULL\n"));
           return;
    }

   if ((limAssocCnfSerDes(pMac, &assocCnf, (tANI_U8 *) pMsgBuf) ==
                            eSIR_FAILURE) ||
        !__limIsSmeAssocCnfValid(&assocCnf))
    {
        limLog(pMac, LOGW,
           FL("Received re/assocCnf message with invalid parameters\n"));

        return;
    }

    if((psessionEntry = peFindSessionByBssid(pMac,assocCnf->bssId,&sessionId))== NULL)
    {
        
        limLog(pMac, LOGE,
                      FL("session does not exist for given bssId\n"));
        return;
    }

#ifdef FEATURE_WLAN_DIAG_SUPPORT_LIM //                          
    limDiagEventReport(pMac, WLAN_PE_DIAG_ASSOC_CNF_EVENT, psessionEntry, 0, 0);
#endif //                         
    
    if ( ((psessionEntry->limSystemRole != eLIM_AP_ROLE) && (psessionEntry->limSystemRole != eLIM_BT_AMP_AP_ROLE)) ||
         ((psessionEntry->limSmeState != eLIM_SME_NORMAL_STATE) && (psessionEntry->limSmeState != eLIM_SME_NORMAL_CHANNEL_SCAN_STATE)))
    {
        limLog(pMac, LOGE,
         FL("Received unexpected message %X in state %X, in role %X\n"),
         msgType, psessionEntry->limSmeState , psessionEntry->limSystemRole);

        return;
    }

    pStaDs = dphGetHashEntry(pMac, assocCnf.aid, &psessionEntry->dph.dphHashTable);


    if (pStaDs == NULL)
    {
        
        PELOG1(limLog(pMac, LOG1,
           FL("Received invalid message %X due to no STA context, for aid %d, peer "),
           msgType, assocCnf.aid);
        limPrintMacAddr(pMac, assocCnf.peerMacAddr, LOG1);)
       
        /*
                                                          
                                               
         */

        limSendSmeDisassocNtf(pMac,
            assocCnf.peerMacAddr,
            eSIR_SME_STA_NOT_ASSOCIATED,
            eLIM_PEER_ENTITY_DISASSOC,
            assocCnf.aid,psessionEntry);

        return;
    }
    if ((pStaDs &&
         (( !palEqualMemory( pMac->hHdd,(tANI_U8 *) pStaDs->staAddr,
                     (tANI_U8 *) assocCnf.peerMacAddr,
                     sizeof(tSirMacAddr)) ) ||
          (pStaDs->mlmStaContext.mlmState !=
                          eLIM_MLM_WT_ASSOC_CNF_STATE) ||
          ((pStaDs->mlmStaContext.subType == LIM_ASSOC) &&
           (msgType != eWNI_SME_ASSOC_CNF)) ||
          ((pStaDs->mlmStaContext.subType == LIM_REASSOC) &&
           (msgType != eWNI_SME_REASSOC_CNF)))))
    {
        PELOG1(limLog(pMac, LOG1,
           FL("Received invalid message %X due to peerMacAddr mismatched or not in eLIM_MLM_WT_ASSOC_CNF_STATE state, for aid %d, peer "),
           msgType, assocCnf.aid);
        limPrintMacAddr(pMac, assocCnf.peerMacAddr, LOG1);)

        return;
    }

    /*
                                                      
                        
     */

    PELOG1(limLog(pMac, LOG1, FL("Received Cnf. Delete Timer\n"));)
    limDeactivateAndChangePerStaIdTimer(pMac, eLIM_CNF_WAIT_TIMER,
                                        pStaDs->assocId);

    if (assocCnf.statusCode == eSIR_SME_SUCCESS)
    {
        tSirMacScheduleIE schedule;

        //                                                
        //                                                 
        //                                               
        //                        

        //                                
        if (pStaDs->mlmStaContext.updateContext)
        {

              tLimMlmStates mlmPrevState = pStaDs->mlmStaContext.mlmState;
              //                                                                     
              pStaDs->mlmStaContext.mlmState = eLIM_MLM_WT_ASSOC_DEL_STA_RSP_STATE;
              if(limDelSta(pMac, pStaDs, true,psessionEntry) != eSIR_SUCCESS)
              {
                  limLog(pMac, LOGE,
                         FL("could not DEL STA with assocId=%d staId %d\n"),
                         pStaDs->assocId, pStaDs->staIndex);

                  limRejectAssociation(pMac,
                           pStaDs->staAddr,
                           pStaDs->mlmStaContext.subType,
                           true, pStaDs->mlmStaContext.authType,
                           pStaDs->assocId, true,
                           (tSirResultCodes) eSIR_MAC_UNSPEC_FAILURE_STATUS,
                           psessionEntry);

                  //                         
                  pStaDs->mlmStaContext.mlmState = mlmPrevState;
                  return;
              }
              return;
        }
        else
        {
            //                      
            if (limAddSta(pMac, pStaDs,psessionEntry) != eSIR_SUCCESS)
            {
                limLog(pMac, LOGE,
                       FL("could not Add STA with assocId=%d\n"),
                       pStaDs->assocId);

              //                                            
               //                                     
                if(pStaDs->qos.addtsPresent)
                {
                  limAdmitControlDeleteTS(pMac, pStaDs->assocId, &pStaDs->qos.addts.tspec.tsinfo, NULL, &tspecIdx);
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

   } //                                             
    else
    {
        //                                    
        limRejectAssociation(pMac, pStaDs->staAddr,
                             pStaDs->mlmStaContext.subType,
                             true, pStaDs->mlmStaContext.authType,
                             pStaDs->assocId, true,
                             assocCnf.statusCode, psessionEntry);
    }
} /*                                   */
#endif


static void
__limProcessSmeAddtsReq(tpAniSirGlobal pMac, tANI_U32 *pMsgBuf)
{
    tpDphHashNode   pStaDs;
    tSirMacAddr     peerMac;
    tpSirAddtsReq   pSirAddts;
    tANI_U32        timeout;
    tpPESession     psessionEntry;
    tANI_U8         sessionId;  //            
    tANI_U8         smesessionId;
    tANI_U16        smetransactionId;


    if(pMsgBuf == NULL)
    {
        limLog(pMac, LOGE,FL("Buffer is Pointing to NULL\n"));
           return;
    }

    limGetSessionInfo(pMac,(tANI_U8 *)pMsgBuf,&smesessionId,&smetransactionId);
    
    pSirAddts = (tpSirAddtsReq) pMsgBuf;

    if((psessionEntry = peFindSessionByBssid(pMac, pSirAddts->bssId,&sessionId))== NULL)
    {
        limLog(pMac, LOGE, "Session Does not exist for given bssId\n");
        return;
    }
#ifdef FEATURE_WLAN_DIAG_SUPPORT_LIM //                          
    limDiagEventReport(pMac, WLAN_PE_DIAG_ADDTS_REQ_EVENT, psessionEntry, 0, 0);
#endif //                         
    


    /*       
                            
                                  
                                         
                                        
     */
    PELOG1(limLog(pMac, LOG1,
           FL("Received SME_ADDTS_REQ (TSid %d, UP %d)\n"),
           pSirAddts->req.tspec.tsinfo.traffic.tsid,
           pSirAddts->req.tspec.tsinfo.traffic.userPrio);)

    if ((psessionEntry->limSystemRole != eLIM_STA_ROLE)&&(psessionEntry->limSystemRole != eLIM_BT_AMP_STA_ROLE))
    {
        PELOGE(limLog(pMac, LOGE, "AddTs received on AP - ignoring\n");)
        limSendSmeAddtsRsp(pMac, pSirAddts->rspReqd, eSIR_FAILURE, psessionEntry, pSirAddts->req.tspec, 
                smesessionId,smetransactionId);
        return;
    }

    //                                         
    if(psessionEntry->dot11mode == WNI_CFG_DOT11_MODE_11B)
    {
        PELOGE(limLog(pMac, LOGE, "AddTS received while Dot11Mode is 11B - ignoring\n");)
        limSendSmeAddtsRsp(pMac, pSirAddts->rspReqd, eSIR_FAILURE, psessionEntry, pSirAddts->req.tspec, 
                smesessionId,smetransactionId);
        return;
    }


    pStaDs = dphGetHashEntry(pMac, DPH_STA_HASH_INDEX_PEER, &psessionEntry->dph.dphHashTable);

    if(pStaDs == NULL)
    {
        PELOGE(limLog(pMac, LOGE, "Cannot find AP context for addts req\n");)
        limSendSmeAddtsRsp(pMac, pSirAddts->rspReqd, eSIR_FAILURE, psessionEntry, pSirAddts->req.tspec,
            smesessionId,smetransactionId);
        return;
    }

    if ((! pStaDs->valid) ||
        (pStaDs->mlmStaContext.mlmState != eLIM_MLM_LINK_ESTABLISHED_STATE))
    {
        PELOGE(limLog(pMac, LOGE, "AddTs received in invalid MLM state\n");)
        limSendSmeAddtsRsp(pMac, pSirAddts->rspReqd, eSIR_FAILURE, psessionEntry, pSirAddts->req.tspec,
            smesessionId,smetransactionId);
        return;
    }

    pSirAddts->req.wsmTspecPresent = 0;
    pSirAddts->req.wmeTspecPresent = 0;
    pSirAddts->req.lleTspecPresent = 0;
    
    if ((pStaDs->wsmEnabled) &&
        (pSirAddts->req.tspec.tsinfo.traffic.accessPolicy != SIR_MAC_ACCESSPOLICY_EDCA))
        pSirAddts->req.wsmTspecPresent = 1;
    else if (pStaDs->wmeEnabled)
        pSirAddts->req.wmeTspecPresent = 1;
    else if (pStaDs->lleEnabled)
        pSirAddts->req.lleTspecPresent = 1;
    else
    {
        PELOGW(limLog(pMac, LOGW, FL("ADDTS_REQ ignore - qos is disabled\n"));)
        limSendSmeAddtsRsp(pMac, pSirAddts->rspReqd, eSIR_FAILURE, psessionEntry, pSirAddts->req.tspec,
            smesessionId,smetransactionId);
        return;
    }

    if ((psessionEntry->limSmeState != eLIM_SME_ASSOCIATED_STATE) &&
        (psessionEntry->limSmeState != eLIM_SME_LINK_EST_STATE))
    {
        limLog(pMac, LOGE, "AddTs received in invalid LIMsme state (%d)\n",
              psessionEntry->limSmeState);
        limSendSmeAddtsRsp(pMac, pSirAddts->rspReqd, eSIR_FAILURE, psessionEntry, pSirAddts->req.tspec,
            smesessionId,smetransactionId);
        return;
    }

    if (pMac->lim.gLimAddtsSent)
    {
        limLog(pMac, LOGE, "Addts (token %d, tsid %d, up %d) is still pending\n",
               pMac->lim.gLimAddtsReq.req.dialogToken,
               pMac->lim.gLimAddtsReq.req.tspec.tsinfo.traffic.tsid,
               pMac->lim.gLimAddtsReq.req.tspec.tsinfo.traffic.userPrio);
        limSendSmeAddtsRsp(pMac, pSirAddts->rspReqd, eSIR_FAILURE, psessionEntry, pSirAddts->req.tspec,
            smesessionId,smetransactionId);
        return;
    }

    #if 0
    val = sizeof(tSirMacAddr);
    if (wlan_cfgGetStr(pMac, WNI_CFG_BSSID, peerMac, &val) != eSIR_SUCCESS)
    {
        //                                          
        limLog(pMac, LOGP, FL("could not retrieve BSSID\n"));
        return;
    }
    #endif
    sirCopyMacAddr(peerMac,psessionEntry->bssId);

    //                       
    pMac->lim.gLimAddtsSent = true;
    palCopyMemory( pMac->hHdd, (tANI_U8 *) &pMac->lim.gLimAddtsReq, (tANI_U8 *) pSirAddts, sizeof(tSirAddtsReq));

    //                         
    limSendAddtsReqActionFrame(pMac, peerMac, &pSirAddts->req,
            psessionEntry);
    PELOG1(limLog(pMac, LOG1, "Sent ADDTS request\n");)

    //                                       
    if (pSirAddts->timeout) 
        timeout = pSirAddts->timeout;
    else if (wlan_cfgGetInt(pMac, WNI_CFG_ADDTS_RSP_TIMEOUT, &timeout) != eSIR_SUCCESS)
    {
        limLog(pMac, LOGP, FL("Unable to get Cfg param %d (Addts Rsp Timeout)\n"),
               WNI_CFG_ADDTS_RSP_TIMEOUT);
        return;
    }

    timeout = SYS_MS_TO_TICKS(timeout);
    if (tx_timer_change(&pMac->lim.limTimers.gLimAddtsRspTimer, timeout, 0) != TX_SUCCESS)
    {
        limLog(pMac, LOGP, FL("AddtsRsp timer change failed!\n"));
        return;
    }
    pMac->lim.gLimAddtsRspTimerCount++;
    if (tx_timer_change_context(&pMac->lim.limTimers.gLimAddtsRspTimer,
                                pMac->lim.gLimAddtsRspTimerCount) != TX_SUCCESS)
    {
        limLog(pMac, LOGP, FL("AddtsRsp timer change failed!\n"));
        return;
    }
    MTRACE(macTrace(pMac, TRACE_CODE_TIMER_ACTIVATE, psessionEntry->peSessionId, eLIM_ADDTS_RSP_TIMER));
    
    //                                     
    pMac->lim.limTimers.gLimAddtsRspTimer.sessionId = sessionId;
    if (tx_timer_activate(&pMac->lim.limTimers.gLimAddtsRspTimer) != TX_SUCCESS)
    {
        limLog(pMac, LOGP, FL("AddtsRsp timer activation failed!\n"));
        return;
    }
    return;
}


static void
__limProcessSmeDeltsReq(tpAniSirGlobal pMac, tANI_U32 *pMsgBuf)
{
    tSirMacAddr     peerMacAddr;
    tANI_U8         ac;
    tSirMacTSInfo   *pTsinfo;
    tpSirDeltsReq   pDeltsReq = (tpSirDeltsReq) pMsgBuf;
    tpDphHashNode   pStaDs = NULL;
    tpPESession     psessionEntry;
    tANI_U8         sessionId;
    tANI_U32        status = eSIR_SUCCESS;
    tANI_U8         smesessionId;
    tANI_U16        smetransactionId;    

    limGetSessionInfo(pMac,(tANI_U8 *)pMsgBuf,&smesessionId,&smetransactionId);
    
    if((psessionEntry = peFindSessionByBssid(pMac, pDeltsReq->bssId, &sessionId))== NULL)
    {
        limLog(pMac, LOGE, "Session Does not exist for given bssId\n");
        status = eSIR_FAILURE;
        goto end;
    }
#ifdef FEATURE_WLAN_DIAG_SUPPORT_LIM //                          
    limDiagEventReport(pMac, WLAN_PE_DIAG_DELTS_REQ_EVENT, psessionEntry, 0, 0);
#endif //                         


    if (eSIR_SUCCESS != limValidateDeltsReq(pMac, pDeltsReq, peerMacAddr,psessionEntry))
    {
        PELOGE(limLog(pMac, LOGE, FL("limValidateDeltsReq failed\n"));)
        status = eSIR_FAILURE;
        limSendSmeDeltsRsp(pMac, pDeltsReq, eSIR_FAILURE,psessionEntry,smesessionId,smetransactionId);
        return;
    }

    PELOG1(limLog(pMac, LOG1, FL("Sent DELTS request to station with assocId = %d MacAddr = %x:%x:%x:%x:%x:%x\n"),
              pDeltsReq->aid, peerMacAddr[0], peerMacAddr[1], peerMacAddr[2],
              peerMacAddr[3], peerMacAddr[4], peerMacAddr[5]);)

    limSendDeltsReqActionFrame(pMac, peerMacAddr, pDeltsReq->req.wmeTspecPresent, &pDeltsReq->req.tsinfo, &pDeltsReq->req.tspec,
              psessionEntry);

    pTsinfo = pDeltsReq->req.wmeTspecPresent ? &pDeltsReq->req.tspec.tsinfo : &pDeltsReq->req.tsinfo;

    /*                                                       
                                                              
                                                       
     */
    limSetTspecUapsdMask(pMac, pTsinfo, CLEAR_UAPSD_MASK);

    /*                                                             
                                                
                                                                  
                                                              
                              
     */ 
    ac = upToAc(pTsinfo->traffic.userPrio);

    if(pTsinfo->traffic.direction == SIR_MAC_DIRECTION_UPLINK)
    {
      pMac->lim.gAcAdmitMask[SIR_MAC_DIRECTION_UPLINK] &= ~(1 << ac);
    }
    else if(pTsinfo->traffic.direction == SIR_MAC_DIRECTION_DNLINK)
    {
      pMac->lim.gAcAdmitMask[SIR_MAC_DIRECTION_DNLINK] &= ~(1 << ac);
    }
    else if(pTsinfo->traffic.direction == SIR_MAC_DIRECTION_BIDIR)
    {
      pMac->lim.gAcAdmitMask[SIR_MAC_DIRECTION_UPLINK] &= ~(1 << ac);
      pMac->lim.gAcAdmitMask[SIR_MAC_DIRECTION_DNLINK] &= ~(1 << ac);
    }

    limSetActiveEdcaParams(pMac, psessionEntry->gLimEdcaParams, psessionEntry);

    pStaDs = dphGetHashEntry(pMac, DPH_STA_HASH_INDEX_PEER, &psessionEntry->dph.dphHashTable);
    if (pStaDs != NULL)
    {
        if (pStaDs->aniPeer == eANI_BOOLEAN_TRUE) 
            limSendEdcaParams(pMac, psessionEntry->gLimEdcaParamsActive, pStaDs->bssId, eANI_BOOLEAN_TRUE);
        else
            limSendEdcaParams(pMac, psessionEntry->gLimEdcaParamsActive, pStaDs->bssId, eANI_BOOLEAN_FALSE);
        status = eSIR_SUCCESS;
    }
    else
    {
        limLog(pMac, LOGE, FL("Self entry missing in Hash Table \n"));
     status = eSIR_FAILURE;
    }     
#ifdef FEATURE_WLAN_CCX
    limDeactivateAndChangeTimer(pMac,eLIM_TSM_TIMER);
#endif

    //                          
    end:
         limSendSmeDeltsRsp(pMac, pDeltsReq, eSIR_SUCCESS,psessionEntry,smesessionId,smetransactionId);
}


void
limProcessSmeAddtsRspTimeout(tpAniSirGlobal pMac, tANI_U32 param)
{
    //                                             
    tpPESession psessionEntry;
    if((psessionEntry = peFindSessionBySessionId(pMac, pMac->lim.limTimers.gLimAddtsRspTimer.sessionId))== NULL) 
    {
        limLog(pMac, LOGP,FL("Session Does not exist for given sessionID\n"));
        return;
    }

    if (  (psessionEntry->limSystemRole != eLIM_STA_ROLE) && (psessionEntry->limSystemRole != eLIM_BT_AMP_STA_ROLE)   )
    {
        limLog(pMac, LOGW, "AddtsRspTimeout in non-Sta role (%d)\n", psessionEntry->limSystemRole);
        pMac->lim.gLimAddtsSent = false;
        return;
    }

    if (! pMac->lim.gLimAddtsSent)
    {
        PELOGW(limLog(pMac, LOGW, "AddtsRspTimeout but no AddtsSent\n");)
        return;
    }

    if (param != pMac->lim.gLimAddtsRspTimerCount)
    {
        limLog(pMac, LOGE, FL("Invalid AddtsRsp Timer count %d (exp %d)\n"),
               param, pMac->lim.gLimAddtsRspTimerCount);
        return;
    }

    //                             
    pMac->lim.gLimAddtsSent = false;
    pMac->lim.gLimAddtsRspTimerCount++;

    limSendSmeAddtsRsp(pMac, true, eSIR_SME_ADDTS_RSP_TIMEOUT, psessionEntry, pMac->lim.gLimAddtsReq.req.tspec,
            psessionEntry->smeSessionId, psessionEntry->transactionId);
}


/* 
                                
  
           
   
  
       
  
                                                    
                                                        
               
 */
static void
__limProcessSmeStatsRequest(tpAniSirGlobal pMac, tANI_U32 *pMsgBuf)
{
    tpAniGetStatsReq    pStatsReq;
    tSirMsgQ msgQ;
    tpPESession psessionEntry;
    tANI_U8     sessionId;

    
    if(pMsgBuf == NULL)
    {
        limLog(pMac, LOGE,FL("Buffer is Pointing to NULL\n"));
           return;
    }
    
    pStatsReq = (tpAniGetStatsReq) pMsgBuf;
    
    if((psessionEntry = peFindSessionByBssid(pMac,pStatsReq->bssId,&sessionId))== NULL)
    {
        limLog(pMac, LOGE, FL("session does not exist for given bssId\n"));
        palFreeMemory( pMac, pMsgBuf );
        return;
    }

       
    
    switch(pStatsReq->msgType)
    {
        //                                       

        //                     
        case eWNI_SME_STA_STAT_REQ:
            msgQ.type = WDA_STA_STAT_REQ;
            break;
        case eWNI_SME_AGGR_STAT_REQ:
            msgQ.type = WDA_AGGR_STAT_REQ;
            break;
        case eWNI_SME_GLOBAL_STAT_REQ:
            msgQ.type = WDA_GLOBAL_STAT_REQ;
            break;
        case eWNI_SME_STAT_SUMM_REQ:
            msgQ.type = WDA_STAT_SUMM_REQ;
            break;   
        default: //                
            PELOGE(limLog(pMac, LOGE, "Unknown Statistics request\n");)
            palFreeMemory( pMac, pMsgBuf );
            return;
    }

    if ( !pMac->lim.gLimRspReqd ) 
    {
        palFreeMemory( pMac, pMsgBuf );
        return;
    }
    else
    {
        pMac->lim.gLimRspReqd = FALSE;
    }

    msgQ.reserved = 0;
    msgQ.bodyptr = pMsgBuf;
    msgQ.bodyval = 0;
    MTRACE(macTraceMsgTx(pMac, psessionEntry->peSessionId, msgQ.type));

    if( eSIR_SUCCESS != (wdaPostCtrlMsg( pMac, &msgQ ))){
        limLog(pMac, LOGP, "Unable to forward request\n");
        palFreeMemory( pMac, pMsgBuf );
        return;
    }

    return;
}


/* 
                                        
  
           
   
  
       
  
                                                    
                                                        
               
 */
static void
__limProcessSmeGetStatisticsRequest(tpAniSirGlobal pMac, tANI_U32 *pMsgBuf)
{
    tpAniGetPEStatsReq    pPEStatsReq;
    tSirMsgQ msgQ;

    pPEStatsReq = (tpAniGetPEStatsReq) pMsgBuf;
    
    //                                                          

    msgQ.type = WDA_GET_STATISTICS_REQ;    

    if ( !pMac->lim.gLimRspReqd ) 
    {
        palFreeMemory( pMac, pMsgBuf );
        return;
    }
    else
    {
        pMac->lim.gLimRspReqd = FALSE;
    }

    msgQ.reserved = 0;
    msgQ.bodyptr = pMsgBuf;
    msgQ.bodyval = 0;
    MTRACE(macTraceMsgTx(pMac, NO_SESSION, msgQ.type));

    if( eSIR_SUCCESS != (wdaPostCtrlMsg( pMac, &msgQ ))){
        palFreeMemory( pMac, pMsgBuf );
        limLog(pMac, LOGP, "Unable to forward request\n");
        return;
    }

    return;
}


#ifdef WLAN_SOFTAP_FEATURE
static void
__limProcessSmeUpdateAPWPSIEs(tpAniSirGlobal pMac, tANI_U32 *pMsgBuf)
{
    tpSirUpdateAPWPSIEsReq  pUpdateAPWPSIEsReq;
    tpPESession             psessionEntry;
    tANI_U8                 sessionId;  //            

    PELOG1(limLog(pMac, LOG1,
           FL("received UPDATE_APWPSIEs_REQ message\n")););
    
    if(pMsgBuf == NULL)
    {
        limLog(pMac, LOGE,FL("Buffer is Pointing to NULL\n"));
        return;
    }
    
    if( palAllocateMemory( pMac->hHdd, (void **)&pUpdateAPWPSIEsReq, sizeof(tSirUpdateAPWPSIEsReq)))
    {
        limLog(pMac, LOGP, FL("call to palAllocateMemory failed for pUpdateAPWPSIEsReq\n"));
        return;
    }

    if ((limUpdateAPWPSIEsReqSerDes(pMac, pUpdateAPWPSIEsReq, (tANI_U8 *) pMsgBuf) == eSIR_FAILURE)) 
    {
        limLog(pMac, LOGW, FL("received invalid SME_SETCONTEXT_REQ message\n")); 
        goto end;
    }
    
    if((psessionEntry = peFindSessionByBssid(pMac, pUpdateAPWPSIEsReq->bssId, &sessionId)) == NULL)
    {
        limLog(pMac, LOGW, FL("Session does not exist for given BSSID\n"));
        goto end;
    }

    palCopyMemory(pMac->hHdd, &psessionEntry->APWPSIEs, &pUpdateAPWPSIEsReq->APWPSIEs, sizeof(tSirAPWPSIEs));
    
    schSetFixedBeaconFields(pMac, psessionEntry);
    limSendBeaconInd(pMac, psessionEntry); 

end:
    palFreeMemory( pMac->hHdd, pUpdateAPWPSIEsReq);
    return;
} /*                                                                               */

static void
__limProcessSmeHideSSID(tpAniSirGlobal pMac, tANI_U32 *pMsgBuf)
{
    tpSirUpdateParams       pUpdateParams;
    tpPESession             psessionEntry;

    PELOG1(limLog(pMac, LOG1,
           FL("received HIDE_SSID message\n")););
    
    if(pMsgBuf == NULL)
    {
        limLog(pMac, LOGE,FL("Buffer is Pointing to NULL\n"));
        return;
    }

    pUpdateParams = (tpSirUpdateParams)pMsgBuf;
    
    if((psessionEntry = peFindSessionBySessionId(pMac, pUpdateParams->sessionId)) == NULL)
    {
        limLog(pMac, LOGW, "Session does not exist for given sessionId %d\n",
                      pUpdateParams->sessionId);
        return;
    }

    /*                          */
    psessionEntry->ssidHidden = pUpdateParams->ssidHidden;
   
    /*               */
    schSetFixedBeaconFields(pMac, psessionEntry);
    limSendBeaconInd(pMac, psessionEntry); 

    return;
} /*                                                                         */

static void
__limProcessSmeSetWPARSNIEs(tpAniSirGlobal pMac, tANI_U32 *pMsgBuf)
{
    tpSirUpdateAPWPARSNIEsReq  pUpdateAPWPARSNIEsReq;
    tpPESession             psessionEntry;
    tANI_U8                 sessionId;  //            

    if(pMsgBuf == NULL)
    {
        limLog(pMac, LOGE,FL("Buffer is Pointing to NULL\n"));
        return;
    }
    
    if( palAllocateMemory( pMac->hHdd, (void **)&pUpdateAPWPARSNIEsReq, sizeof(tSirUpdateAPWPSIEsReq)))
    {
        limLog(pMac, LOGP, FL("call to palAllocateMemory failed for pUpdateAPWPARSNIEsReq\n"));
        return;
    }

    if ((limUpdateAPWPARSNIEsReqSerDes(pMac, pUpdateAPWPARSNIEsReq, (tANI_U8 *) pMsgBuf) == eSIR_FAILURE)) 
    {
        limLog(pMac, LOGW, FL("received invalid SME_SETCONTEXT_REQ message\n")); 
        goto end;
    }
    
    if((psessionEntry = peFindSessionByBssid(pMac, pUpdateAPWPARSNIEsReq->bssId, &sessionId)) == NULL)
    {
        limLog(pMac, LOGW, FL("Session does not exist for given BSSID\n"));
        goto end;
    }

    palCopyMemory(pMac->hHdd, &psessionEntry->pLimStartBssReq->rsnIE, &pUpdateAPWPARSNIEsReq->APWPARSNIEs, sizeof(tSirRSNie));
    
    limSetRSNieWPAiefromSmeStartBSSReqMessage(pMac, &psessionEntry->pLimStartBssReq->rsnIE, psessionEntry);
    
    psessionEntry->pLimStartBssReq->privacy = 1;
    psessionEntry->privacy = 1;
    
    schSetFixedBeaconFields(pMac, psessionEntry);
    limSendBeaconInd(pMac, psessionEntry); 

end:
    palFreeMemory( pMac->hHdd, pUpdateAPWPARSNIEsReq);
    return;
} /*                                                                             */

#endif


/*                                                               
                             
                                                                    
                            
                         
            
                                                             */
void
limProcessSmeDelBaPeerInd(tpAniSirGlobal pMac, tANI_U32 *pMsgBuf)
{
    tANI_U16            assocId =0;
    tpSmeDelBAPeerInd   pSmeDelBAPeerInd = (tpSmeDelBAPeerInd)pMsgBuf;
    tpDphHashNode       pSta;
    tpPESession         psessionEntry;
    tANI_U8             sessionId;
    


    if(NULL == pSmeDelBAPeerInd)
        return;

    if ((psessionEntry = peFindSessionByBssid(pMac,pSmeDelBAPeerInd->bssId,&sessionId))==NULL)
    {
        limLog(pMac, LOGE,FL("session does not exist for given bssId\n"));
        return;
    }
    limLog(pMac, LOGW, FL("called with staId = %d, tid = %d, baDirection = %d\n"), 
              pSmeDelBAPeerInd->staIdx, pSmeDelBAPeerInd->baTID, pSmeDelBAPeerInd->baDirection);

    pSta = dphLookupAssocId(pMac, pSmeDelBAPeerInd->staIdx, &assocId, &psessionEntry->dph.dphHashTable);    
    if( eSIR_SUCCESS != limPostMlmDelBAReq( pMac,
          pSta,
          pSmeDelBAPeerInd->baDirection,
          pSmeDelBAPeerInd->baTID,
          eSIR_MAC_UNSPEC_FAILURE_REASON,psessionEntry))
    {
      limLog( pMac, LOGW,
          FL( "Failed to post LIM_MLM_DELBA_REQ to " ));
      if (pSta)
          limPrintMacAddr(pMac, pSta->staAddr, LOGW); 
    }
}

//                                                                     
/* 
                            
  
                                                                           
  
         
  
               
  
        
  
              
               
 */

void __limProcessReportMessage(tpAniSirGlobal pMac, tpSirMsgQ pMsg)
{
#ifdef WLAN_FEATURE_VOWIFI
   switch (pMsg->type)
   {
      case eWNI_SME_NEIGHBOR_REPORT_REQ_IND:
         rrmProcessNeighborReportReq( pMac, pMsg->bodyptr );
         break;
      case eWNI_SME_BEACON_REPORT_RESP_XMIT_IND:
        {
#if defined FEATURE_WLAN_CCX
         tpSirBeaconReportXmitInd pBcnReport=NULL;
         tpPESession psessionEntry=NULL;
         tANI_U8 sessionId;

         if(pMsg->bodyptr == NULL)
         {
            limLog(pMac, LOGE,FL("Buffer is Pointing to NULL\n"));
            return;
         }
         pBcnReport = (tpSirBeaconReportXmitInd )pMsg->bodyptr;
         if((psessionEntry = peFindSessionByBssid(pMac, pBcnReport->bssId,&sessionId))== NULL)
         {
            limLog(pMac, LOGE, "Session Does not exist for given bssId\n");
            return;
         }
         if (psessionEntry->isCCXconnection)
             ccxProcessBeaconReportXmit( pMac, pMsg->bodyptr);
         else
#endif
             rrmProcessBeaconReportXmit( pMac, pMsg->bodyptr );
        }
        break;
   }
#endif
}

#if defined(FEATURE_WLAN_CCX) || defined(WLAN_FEATURE_VOWIFI)
//                                                                     
/* 
                          
  
                                                                                   
  
         
  
               
  
        
  
                                    
                                      
               
 */
tSirRetStatus
limSendSetMaxTxPowerReq ( tpAniSirGlobal pMac, tPowerdBm txPower, tpPESession pSessionEntry )
{
   tpMaxTxPowerParams pMaxTxParams = NULL;
   tSirRetStatus  retCode = eSIR_SUCCESS;
   tSirMsgQ       msgQ;

   if( pSessionEntry == NULL )
   {
      PELOGE(limLog(pMac, LOGE, "%s:%d: Inavalid parameters\n", __func__, __LINE__ );)
      return eSIR_FAILURE;
   }
   if( eHAL_STATUS_SUCCESS != palAllocateMemory( pMac->hHdd,
            (void **) &pMaxTxParams, sizeof(tMaxTxPowerParams) ) ) 
   {
      limLog( pMac, LOGP, "%s:%d:Unable to allocate memory for pMaxTxParams \n", __func__, __LINE__);
      return eSIR_MEM_ALLOC_FAILED;

   }
#if defined(WLAN_VOWIFI_DEBUG) || defined(FEATURE_WLAN_CCX)
   PELOG1(limLog( pMac, LOG1, "%s:%d: Allocated memory for pMaxTxParams...will be freed in other module\n", __func__, __LINE__ );)
#endif
   pMaxTxParams->power = txPower;
   palCopyMemory( pMac->hHdd, pMaxTxParams->bssId, pSessionEntry->bssId, sizeof(tSirMacAddr) );
   palCopyMemory( pMac->hHdd, pMaxTxParams->selfStaMacAddr, pSessionEntry->selfMacAddr, sizeof(tSirMacAddr) );

    msgQ.type = WDA_SET_MAX_TX_POWER_REQ;
    msgQ.bodyptr = pMaxTxParams;
    msgQ.bodyval = 0;
    PELOGW(limLog(pMac, LOG1, FL("Posting WDA_SET_MAX_TX_POWER_REQ to WDA\n"));)
    if(eSIR_SUCCESS != (retCode = wdaPostCtrlMsg(pMac, &msgQ)))
    {
       PELOGW(limLog(pMac, LOGW, FL("wdaPostCtrlMsg() failed\n"));)
       if (NULL != pMaxTxParams)
       {
          palFreeMemory(pMac->hHdd, (tANI_U8*)pMaxTxParams);
       }
       return retCode;
    }
    return retCode;
}
#endif

/* 
                                 
  
           
                                                                  
                                                                
  
        
  
              
  
       
  
                                                    
                                                        
               
 */

static void
__limProcessSmeAddStaSelfReq(tpAniSirGlobal pMac, tANI_U32 *pMsgBuf)
{
   tSirMsgQ msg;
   tpAddStaSelfParams pAddStaSelfParams;
   tpSirSmeAddStaSelfReq pSmeReq = (tpSirSmeAddStaSelfReq) pMsgBuf;

   if ( eHAL_STATUS_SUCCESS != palAllocateMemory( pMac->hHdd, (void**) &pAddStaSelfParams,
            sizeof( tAddStaSelfParams) ) )
   {
      limLog( pMac, LOGP, FL("Unable to allocate memory for tAddSelfStaParams") );
      return;
   }

   palCopyMemory( pMac->hHdd, pAddStaSelfParams->selfMacAddr, pSmeReq->selfMacAddr, sizeof(tSirMacAddr) ); 

   msg.type = SIR_HAL_ADD_STA_SELF_REQ;
   msg.reserved = 0;
   msg.bodyptr =  pAddStaSelfParams;
   msg.bodyval = 0;

   PELOGW(limLog(pMac, LOG1, FL("sending SIR_HAL_ADD_STA_SELF_REQ msg to HAL\n"));)
      MTRACE(macTraceMsgTx(pMac, NO_SESSION, msg.type));

   if(eSIR_SUCCESS != wdaPostCtrlMsg(pMac, &msg))
   {
      limLog(pMac, LOGP, FL("wdaPostCtrlMsg failed\n"));
   }
   return;
} /*                                     */


/* 
                                 
  
           
                                                                  
                                                                
  
        
  
              
  
       
  
                                                    
                                                        
               
 */

static void
__limProcessSmeDelStaSelfReq(tpAniSirGlobal pMac, tANI_U32 *pMsgBuf)
{
   tSirMsgQ msg;
   tpDelStaSelfParams pDelStaSelfParams;
   tpSirSmeDelStaSelfReq pSmeReq = (tpSirSmeDelStaSelfReq) pMsgBuf;

   if ( eHAL_STATUS_SUCCESS != palAllocateMemory( pMac->hHdd, (void**) &pDelStaSelfParams,
            sizeof( tDelStaSelfParams) ) )
   {
      limLog( pMac, LOGP, FL("Unable to allocate memory for tDelStaSelfParams") );
      return;
   }

   palCopyMemory( pMac->hHdd, pDelStaSelfParams->selfMacAddr, pSmeReq->selfMacAddr, sizeof(tSirMacAddr) ); 

   msg.type = SIR_HAL_DEL_STA_SELF_REQ;
   msg.reserved = 0;
   msg.bodyptr =  pDelStaSelfParams;
   msg.bodyval = 0;

   PELOGW(limLog(pMac, LOG1, FL("sending SIR_HAL_ADD_STA_SELF_REQ msg to HAL\n"));)
      MTRACE(macTraceMsgTx(pMac, NO_SESSION, msg.type));

   if(eSIR_SUCCESS != wdaPostCtrlMsg(pMac, &msg))
   {
      limLog(pMac, LOGP, FL("wdaPostCtrlMsg failed\n"));
   }
   return;
} /*                                        */


#ifdef WLAN_FEATURE_P2P
/* 
                                        
  
           
                                                                              
                                                    
  
        
  
              
  
       
  
                                                    
                                                        
               
 */
static void
__limProcessSmeRegisterMgmtFrameReq(tpAniSirGlobal pMac, tANI_U32 *pMsgBuf)
{
    VOS_STATUS vosStatus;
    tpSirRegisterMgmtFrame pSmeReq = (tpSirRegisterMgmtFrame)pMsgBuf;
    tpLimMgmtFrameRegistration pLimMgmtRegistration = NULL, pNext = NULL;
    tANI_BOOLEAN match = VOS_FALSE;
    PELOG1(limLog(pMac, LOG1, 
           FL("registerFrame %d, frameType %d, matchLen %d\n"), 
            pSmeReq->registerFrame, pSmeReq->frameType, pSmeReq->matchLen);)

    /*                                         */

    vos_list_peek_front(&pMac->lim.gLimMgmtFrameRegistratinQueue,
            (vos_list_node_t**)&pLimMgmtRegistration);

    while(pLimMgmtRegistration != NULL)
    {
        if (pLimMgmtRegistration->frameType == pSmeReq->frameType)
        {
            if(pSmeReq->matchLen)
            {
                if (pLimMgmtRegistration->matchLen == pSmeReq->matchLen)
                {
                    if (palEqualMemory(pMac, pLimMgmtRegistration->matchData, 
                                pSmeReq->matchData, pLimMgmtRegistration->matchLen))
                    {
                        /*              */   
                        match = VOS_TRUE;
                        break;
                    }
                }
            }
            else
            {
                /*              */   
                match = VOS_TRUE;
                break;
            }
        }
        vosStatus = vos_list_peek_next (
                &pMac->lim.gLimMgmtFrameRegistratinQueue,
                (vos_list_node_t*) pLimMgmtRegistration,
                (vos_list_node_t**) &pNext );

        pLimMgmtRegistration = pNext;
        pNext = NULL; 

    }

    if (match)
    {
        vos_list_remove_node(&pMac->lim.gLimMgmtFrameRegistratinQueue,
                (vos_list_node_t*)pLimMgmtRegistration);
        palFreeMemory(pMac,pLimMgmtRegistration);
    }

    if(pSmeReq->registerFrame)
    {
        palAllocateMemory(pMac, (void**)&pLimMgmtRegistration,
                        sizeof(tLimMgmtFrameRegistration) + pSmeReq->matchLen);
        if(pLimMgmtRegistration != NULL)
        {
            palZeroMemory(pMac, (void*)pLimMgmtRegistration, 
              sizeof(tLimMgmtFrameRegistration) + pSmeReq->matchLen );
            pLimMgmtRegistration->frameType = pSmeReq->frameType;
            pLimMgmtRegistration->matchLen  = pSmeReq->matchLen;
            pLimMgmtRegistration->sessionId = pSmeReq->sessionId;
            if(pSmeReq->matchLen)
            {
                palCopyMemory(pMac,pLimMgmtRegistration->matchData, 
                              pSmeReq->matchData, pSmeReq->matchLen);
            }
     
            vos_list_insert_front(&pMac->lim.gLimMgmtFrameRegistratinQueue,
                              &pLimMgmtRegistration->node);
        }
    }

    return;
} /*                                               */
#endif


/* 
                             
  
           
                                                            
                                                                  
               
  
        
  
              
  
       
  
                                                    
                                                   
                                                        
                                                                    
                                                           
 */

tANI_BOOLEAN
limProcessSmeReqMessages(tpAniSirGlobal pMac, tpSirMsgQ pMsg)
{
    tANI_BOOLEAN bufConsumed = TRUE; //                                                                                                        
    tANI_U32 *pMsgBuf = pMsg->bodyptr;

    PELOG1(limLog(pMac, LOG1, FL("LIM Received SME Message %s(%d) Global LimSmeState:%s(%d) Global LimMlmState: %s(%d)\n"),
         limMsgStr(pMsg->type), pMsg->type,
         limSmeStateStr(pMac->lim.gLimSmeState), pMac->lim.gLimSmeState,
         limMlmStateStr(pMac->lim.gLimMlmState), pMac->lim.gLimMlmState );)

    switch (pMsg->type)
    {
        case eWNI_SME_START_REQ:
            __limProcessSmeStartReq(pMac, pMsgBuf);
            break;

        case eWNI_SME_SYS_READY_IND:
            bufConsumed = __limProcessSmeSysReadyInd(pMac, pMsgBuf);
            break;


        case eWNI_SME_START_BSS_REQ:
            bufConsumed = __limProcessSmeStartBssReq(pMac, pMsg);
            break;

        case eWNI_SME_SCAN_REQ:
            __limProcessSmeScanReq(pMac, pMsgBuf);

            break;

#ifdef FEATURE_OEM_DATA_SUPPORT
        case eWNI_SME_OEM_DATA_REQ:
            __limProcessSmeOemDataReq(pMac, pMsgBuf);

            break;
#endif
#ifdef WLAN_FEATURE_P2P
        case eWNI_SME_REMAIN_ON_CHANNEL_REQ:
            bufConsumed = limProcessRemainOnChnlReq(pMac, pMsgBuf);
            break;

        case eWNI_SME_UPDATE_NOA:
            __limProcessSmeNoAUpdate(pMac, pMsgBuf);
            break;
#endif
        case eWNI_SME_JOIN_REQ:
            __limProcessSmeJoinReq(pMac, pMsgBuf);

            break;

        case eWNI_SME_AUTH_REQ:
           //                                       

            break;

        case eWNI_SME_REASSOC_REQ:
            __limProcessSmeReassocReq(pMac, pMsgBuf);

            break;

        case eWNI_SME_PROMISCUOUS_MODE_REQ:
            //                                             

            break;

        case eWNI_SME_DISASSOC_REQ:
            __limProcessSmeDisassocReq(pMac, pMsgBuf);

            break;

        case eWNI_SME_DISASSOC_CNF:
        case eWNI_SME_DEAUTH_CNF:
            __limProcessSmeDisassocCnf(pMac, pMsgBuf);

            break;

        case eWNI_SME_DEAUTH_REQ:
            __limProcessSmeDeauthReq(pMac, pMsgBuf);

            break;

#if (WNI_POLARIS_FW_PACKAGE == ADVANCED) && defined(ANI_PRODUCT_TYPE_AP)
        case eWNI_SME_SWITCH_CHL_REQ:
        case eWNI_SME_SWITCH_CHL_CB_PRIMARY_REQ:
        case eWNI_SME_SWITCH_CHL_CB_SECONDARY_REQ:
            bufConsumed = __limProcessSmeSwitchChlReq(pMac, pMsg);
            break;
#endif


        case eWNI_SME_SETCONTEXT_REQ:
            __limProcessSmeSetContextReq(pMac, pMsgBuf);

            break;

        case eWNI_SME_REMOVEKEY_REQ:
            __limProcessSmeRemoveKeyReq(pMac, pMsgBuf);

            break;

        case eWNI_SME_STOP_BSS_REQ:
            bufConsumed = __limProcessSmeStopBssReq(pMac, pMsg);
            break;

        case eWNI_SME_ASSOC_CNF:
        case eWNI_SME_REASSOC_CNF:
            if (pMsg->type == eWNI_SME_ASSOC_CNF)
                PELOG1(limLog(pMac, LOG1, FL("Received ASSOC_CNF message\n"));)
            else
                PELOG1(limLog(pMac, LOG1, FL("Received REASSOC_CNF message\n"));)
#ifdef ANI_PRODUCT_TYPE_AP
            __limProcessSmeAssocCnf(pMac, pMsg->type, pMsgBuf);
#endif
            __limProcessSmeAssocCnfNew(pMac, pMsg->type, pMsgBuf);
            break;

        case eWNI_SME_ADDTS_REQ:
            PELOG1(limLog(pMac, LOG1, FL("Received ADDTS_REQ message\n"));)
            __limProcessSmeAddtsReq(pMac, pMsgBuf);
            break;

        case eWNI_SME_DELTS_REQ:
            PELOG1(limLog(pMac, LOG1, FL("Received DELTS_REQ message\n"));)
            __limProcessSmeDeltsReq(pMac, pMsgBuf);
            break;

        case SIR_LIM_ADDTS_RSP_TIMEOUT:
            PELOG1(limLog(pMac, LOG1, FL("Received SIR_LIM_ADDTS_RSP_TIMEOUT message \n"));)
            limProcessSmeAddtsRspTimeout(pMac, pMsg->bodyval);
            break;

        case eWNI_SME_STA_STAT_REQ:
        case eWNI_SME_AGGR_STAT_REQ:
        case eWNI_SME_GLOBAL_STAT_REQ:
        case eWNI_SME_STAT_SUMM_REQ:
            __limProcessSmeStatsRequest( pMac, pMsgBuf);
            //                                                                       
            bufConsumed = FALSE;
            break;
        case eWNI_SME_GET_STATISTICS_REQ:
            __limProcessSmeGetStatisticsRequest( pMac, pMsgBuf);
            //                                                                       
            bufConsumed = FALSE;
            break;              
        case eWNI_SME_DEL_BA_PEER_IND:
            limProcessSmeDelBaPeerInd(pMac, pMsgBuf);
            break;
        case eWNI_SME_GET_SCANNED_CHANNEL_REQ:
            limProcessSmeGetScanChannelInfo(pMac, pMsgBuf);
            break;
#ifdef WLAN_SOFTAP_FEATURE
        case eWNI_SME_GET_ASSOC_STAS_REQ:
            limProcessSmeGetAssocSTAsInfo(pMac, pMsgBuf);
            break;
        case eWNI_SME_TKIP_CNTR_MEAS_REQ:
            limProcessTkipCounterMeasures(pMac, pMsgBuf);
            break;

       case eWNI_SME_HIDE_SSID_REQ: 
            __limProcessSmeHideSSID(pMac, pMsgBuf);
            break;
       case eWNI_SME_UPDATE_APWPSIE_REQ: 
            __limProcessSmeUpdateAPWPSIEs(pMac, pMsgBuf);
            break;
        case eWNI_SME_GET_WPSPBC_SESSION_REQ:
             limProcessSmeGetWPSPBCSessions(pMac, pMsgBuf); 
             break;
         
        case eWNI_SME_SET_APWPARSNIEs_REQ:
              __limProcessSmeSetWPARSNIEs(pMac, pMsgBuf);        
              break;
#endif            
            
#if defined WLAN_FEATURE_VOWIFI 
        case eWNI_SME_NEIGHBOR_REPORT_REQ_IND:
        case eWNI_SME_BEACON_REPORT_RESP_XMIT_IND:
            __limProcessReportMessage(pMac, pMsg);
            break;
#endif

#if defined WLAN_FEATURE_VOWIFI_11R
       case eWNI_SME_FT_PRE_AUTH_REQ:
            bufConsumed = (tANI_BOOLEAN)limProcessFTPreAuthReq(pMac, pMsg);
            break;
       case eWNI_SME_FT_UPDATE_KEY:
            limProcessFTUpdateKey(pMac, pMsgBuf);
            break;

       case eWNI_SME_FT_AGGR_QOS_REQ:
            limProcessFTAggrQosReq(pMac, pMsgBuf);
            break;
#endif

#if defined FEATURE_WLAN_CCX
       case eWNI_SME_CCX_ADJACENT_AP_REPORT:
            limProcessAdjacentAPRepMsg ( pMac, pMsgBuf );
            break;
#endif
       case eWNI_SME_ADD_STA_SELF_REQ:
            __limProcessSmeAddStaSelfReq( pMac, pMsgBuf );
            break;
        case eWNI_SME_DEL_STA_SELF_REQ:
            __limProcessSmeDelStaSelfReq( pMac, pMsgBuf );
            break;

#ifdef WLAN_FEATURE_P2P
        case eWNI_SME_REGISTER_MGMT_FRAME_REQ:
            __limProcessSmeRegisterMgmtFrameReq( pMac, pMsgBuf );
            break;
#endif        
            

        default:
            vos_mem_free((v_VOID_t*)pMsg->bodyptr);
            pMsg->bodyptr = NULL;
            break;
    } //                 

    return bufConsumed;
} /*                                    */
