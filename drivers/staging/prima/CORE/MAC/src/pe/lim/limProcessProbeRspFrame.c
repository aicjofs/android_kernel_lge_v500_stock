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
#include "aniGlobal.h"
#ifdef FEATURE_WLAN_NON_INTEGRATED_SOC
#include "halCommonApi.h"
#endif
#include "schApi.h"
#include "utilsApi.h"
#include "limApi.h"
#include "limTypes.h"
#include "limUtils.h"
#include "limAssocUtils.h"
#include "limPropExtsUtils.h"
#include "limSerDesUtils.h"
#include "limSendMessages.h"

#include "parserApi.h"

/* 
                          
  
           
                                                           
                                  
  
        
                                                         
  
              
  
       
                                               
                                                             
                                                                  
                                                                
  
                                                
                                                                           
               
 */


void
limProcessProbeRspFrame(tpAniSirGlobal pMac, tANI_U8 *pRxPacketInfo,tpPESession psessionEntry)
{
    tANI_U8                 *pBody;
    tANI_U32                frameLen = 0;
    tSirMacAddr             currentBssId;
    tpSirMacMgmtHdr         pHdr;
    tSirProbeRespBeacon    *pProbeRsp;
    tANI_U8 qosEnabled =    false;
    tANI_U8 wmeEnabled =    false;

    if(eHAL_STATUS_SUCCESS != palAllocateMemory(pMac->hHdd, 
                                                (void **)&pProbeRsp, sizeof(tSirProbeRespBeacon)))
    {
        limLog(pMac, LOGE, FL("Unable to PAL allocate memory in limProcessProbeRspFrame\n") );
        return;
    }

    pProbeRsp->ssId.length              = 0;
    pProbeRsp->wpa.length               = 0;
    pProbeRsp->propIEinfo.apName.length = 0;
#if (WNI_POLARIS_FW_PACKAGE == ADVANCED)
    pProbeRsp->propIEinfo.aniIndicator  = 0;
    pProbeRsp->propIEinfo.wdsLength     = 0;
#endif


    pHdr = WDA_GET_RX_MAC_HEADER(pRxPacketInfo);


   PELOG2(limLog(pMac, LOG2,
             FL("Received Probe Response frame with length=%d from "),
             WDA_GET_RX_MPDU_LEN(pRxPacketInfo));
    limPrintMacAddr(pMac, pHdr->sa, LOG2);)

   if (limDeactivateMinChannelTimerDuringScan(pMac) != eSIR_SUCCESS)
   {
       palFreeMemory(pMac->hHdd, pProbeRsp);    
       return;
   }


    /* 
                                      
                                                                  
                                                              
                         
                                                     
                                 
                                                                             
                                      
                                                                    
                                                                             
                                                    
                                                                 
      
                                                      
     */
        //                  
    if (((pMac->lim.gLimMlmState == eLIM_MLM_WT_PROBE_RESP_STATE) ||   //                                           
        (pMac->lim.gLimMlmState == eLIM_MLM_PASSIVE_SCAN_STATE) ||     //                                           
        (pMac->lim.gLimMlmState == eLIM_MLM_LEARN_STATE) ||            //                                            
        (psessionEntry->limMlmState == eLIM_MLM_WT_JOIN_BEACON_STATE) ||
        (psessionEntry->limMlmState == eLIM_MLM_LINK_ESTABLISHED_STATE) )||
        ((GET_LIM_SYSTEM_ROLE(psessionEntry) == eLIM_STA_IN_IBSS_ROLE) &&
         (psessionEntry->limMlmState == eLIM_MLM_BSS_STARTED_STATE)))
    {
        frameLen = WDA_GET_RX_PAYLOAD_LEN(pRxPacketInfo);

        //                                         
        pBody = WDA_GET_RX_MPDU_DATA(pRxPacketInfo);

        if (sirConvertProbeFrame2Struct(pMac, pBody, frameLen, pProbeRsp)
                          ==eSIR_FAILURE)
        {
            PELOG1(limLog(pMac, LOG1,
               FL("PArse error ProbeResponse, length=%d\n"),
               frameLen);)
            palFreeMemory(pMac->hHdd, pProbeRsp);
            return;
        }
                                                                            //                                     
        if ((pMac->lim.gLimMlmState == eLIM_MLM_WT_PROBE_RESP_STATE) ||    //                                           
            (pMac->lim.gLimMlmState == eLIM_MLM_PASSIVE_SCAN_STATE))
            limCheckAndAddBssDescription(pMac, pProbeRsp, pRxPacketInfo, 
               ((pMac->lim.gLimHalScanState == eLIM_HAL_SCANNING_STATE) ? eANI_BOOLEAN_TRUE : eANI_BOOLEAN_FALSE), eANI_BOOLEAN_TRUE);
        else if (pMac->lim.gLimMlmState == eLIM_MLM_LEARN_STATE)           //                                           
        {
#if defined(ANI_PRODUCT_TYPE_AP) && (WNI_POLARIS_FW_PACKAGE == ADVANCED)
            //                        
            /*                                                                                            
                                                                                                      
                                               */
            //                                              
              //                                                                                  
            limCollectMeasurementData(pMac, pRxPacketInfo, pProbeRsp);
           PELOG3(limLog(pMac, LOG3,
               FL("Parsed WDS info in ProbeRsp frames: wdsLength=%d\n"),
               pProbeRsp->propIEinfo.wdsLength);)
#endif
        }
        else if (psessionEntry->limMlmState ==
                                     eLIM_MLM_WT_JOIN_BEACON_STATE)
        {
            if( psessionEntry->beacon != NULL )//                                                                        
            {
                palFreeMemory(pMac->hHdd, psessionEntry->beacon);
                psessionEntry->beacon = NULL;
            }
            psessionEntry->bcnLen = WDA_GET_RX_PAYLOAD_LEN(pRxPacketInfo);
            if ((palAllocateMemory(pMac->hHdd, (void**)&psessionEntry->beacon,
                                   psessionEntry->bcnLen))
                != eHAL_STATUS_SUCCESS)
            {
                PELOGE(limLog(pMac, LOGE,
                              FL("Unable to allocate memory to store beacon"));)
            }
            else
            {
                //                                                                         
                palCopyMemory(pMac->hHdd, psessionEntry->beacon,
                              WDA_GET_RX_MPDU_DATA(pRxPacketInfo),
                              psessionEntry->bcnLen);
            }

            //                            
            limCheckAndAnnounceJoinSuccess(pMac, pProbeRsp, pHdr, psessionEntry);
        }
        else if(psessionEntry->limMlmState == eLIM_MLM_LINK_ESTABLISHED_STATE)
        {
            tpDphHashNode pStaDs = NULL;
            /* 
                                                  
                                                  
                                  
            */
            #if 0
            if (wlan_cfgGetStr(pMac,
                          WNI_CFG_BSSID,
                          currentBssId,
                          &cfg) != eSIR_SUCCESS)
            {
                //                                          
                limLog(pMac, LOGP, FL("could not retrieve BSSID\n"));
            }
            #endif //                 
            sirCopyMacAddr(currentBssId,psessionEntry->bssId);

            if ( !palEqualMemory( pMac->hHdd,currentBssId, pHdr->bssId, sizeof(tSirMacAddr)) )
            {
                palFreeMemory(pMac->hHdd, pProbeRsp);    
                return;
            }

            if (!LIM_IS_CONNECTION_ACTIVE(psessionEntry))
            {
                limLog(pMac, LOGW,
                    FL("Received Probe Resp from AP. So it is alive!!\n"));

                if (pProbeRsp->HTInfo.present)
                    limReceivedHBHandler(pMac, (tANI_U8)pProbeRsp->HTInfo.primaryChannel, psessionEntry);
                else
                    limReceivedHBHandler(pMac, (tANI_U8)pProbeRsp->channelNumber, psessionEntry);
            }

#if defined ANI_PRODUCT_TYPE_CLIENT || defined (ANI_AP_CLIENT_SDK)
            
            if (psessionEntry->limSystemRole == eLIM_STA_ROLE)
            {
                if (pProbeRsp->quietIEPresent)
                {
                    limUpdateQuietIEFromBeacon(pMac, &(pProbeRsp->quietIE), psessionEntry);
                }
                else if ((psessionEntry->gLimSpecMgmt.quietState == eLIM_QUIET_BEGIN) ||
                     (psessionEntry->gLimSpecMgmt.quietState == eLIM_QUIET_RUNNING))
                {
                    PELOG1(limLog(pMac, LOG1, FL("Received a probe rsp without Quiet IE\n"));)
                    limCancelDot11hQuiet(pMac, psessionEntry);
                }

                if (pProbeRsp->channelSwitchPresent ||
                    pProbeRsp->propIEinfo.propChannelSwitchPresent)
                {
                    limUpdateChannelSwitch(pMac, pProbeRsp, psessionEntry);
                }
                else if (psessionEntry->gLimSpecMgmt.dot11hChanSwState == eLIM_11H_CHANSW_RUNNING)
                {
                    limCancelDot11hChannelSwitch(pMac, psessionEntry);
                }
            }
        
#endif
            
            /* 
                                                                              
                                                                                                
                                                                        
                                                           
                                                                        
            */

            pStaDs = dphGetHashEntry(pMac, DPH_STA_HASH_INDEX_PEER, &psessionEntry->dph.dphHashTable);

            limGetQosMode(psessionEntry, &qosEnabled);
            limGetWmeMode(psessionEntry, &wmeEnabled);
           PELOG2(limLog(pMac, LOG2,
                    FL("wmeEdcaPresent: %d wmeEnabled: %d, edcaPresent: %d, qosEnabled: %d,  edcaParams.qosInfo.count: %d schObject.gLimEdcaParamSetCount: %d\n"),
                          pProbeRsp->wmeEdcaPresent, wmeEnabled, pProbeRsp->edcaPresent, qosEnabled,
                          pProbeRsp->edcaParams.qosInfo.count, psessionEntry->gLimEdcaParamSetCount);)
            if (((pProbeRsp->wmeEdcaPresent && wmeEnabled) ||
                (pProbeRsp->edcaPresent && qosEnabled)) &&
                (pProbeRsp->edcaParams.qosInfo.count != psessionEntry->gLimEdcaParamSetCount))
            {
                if (schBeaconEdcaProcess(pMac, &pProbeRsp->edcaParams, psessionEntry) != eSIR_SUCCESS)
                    PELOGE(limLog(pMac, LOGE, FL("EDCA parameter processing error\n"));)
                else if (pStaDs != NULL)
                {
                    //                                         
                    limSetActiveEdcaParams(pMac, psessionEntry->gLimEdcaParams, psessionEntry); 

                    if (pStaDs->aniPeer == eANI_BOOLEAN_TRUE)
                        limSendEdcaParams(pMac, psessionEntry->gLimEdcaParamsActive, pStaDs->bssId, eANI_BOOLEAN_TRUE);
                    else
                        limSendEdcaParams(pMac, psessionEntry->gLimEdcaParamsActive, pStaDs->bssId, eANI_BOOLEAN_FALSE);
                }
                else
                    PELOGE(limLog(pMac, LOGE, FL("Self Entry missing in Hash Table\n"));)

            }
        }
        else if ((psessionEntry->limSystemRole == eLIM_STA_IN_IBSS_ROLE) &&
                 (psessionEntry->limMlmState == eLIM_MLM_BSS_STARTED_STATE))
                limHandleIBSScoalescing(pMac, pProbeRsp, pRxPacketInfo,psessionEntry);
    } //                                                                    

    palFreeMemory(pMac->hHdd, pProbeRsp);
    //                                                
    return;
} /*                                   */


void
limProcessProbeRspFrameNoSession(tpAniSirGlobal pMac, tANI_U8 *pRxPacketInfo)
{
    tANI_U8                 *pBody;
    tANI_U32                frameLen = 0;
    tpSirMacMgmtHdr         pHdr;
    tSirProbeRespBeacon    *pProbeRsp;

    if(eHAL_STATUS_SUCCESS != palAllocateMemory(pMac->hHdd, 
                                                (void **)&pProbeRsp, sizeof(tSirProbeRespBeacon)))
    {
        limLog(pMac, LOGE, FL("Unable to PAL allocate memory in limProcessProbeRspFrameNoSession\n") );
        return;
    }

    pProbeRsp->ssId.length              = 0;
    pProbeRsp->wpa.length               = 0;
    pProbeRsp->propIEinfo.apName.length = 0;
#if (WNI_POLARIS_FW_PACKAGE == ADVANCED)
    pProbeRsp->propIEinfo.aniIndicator  = 0;
    pProbeRsp->propIEinfo.wdsLength     = 0;
#endif


    pHdr = WDA_GET_RX_MAC_HEADER(pRxPacketInfo);


    limLog(pMac, LOG2,
             FL("Received Probe Response frame with length=%d from "),
             WDA_GET_RX_MPDU_LEN(pRxPacketInfo));
    limPrintMacAddr(pMac, pHdr->sa, LOG2);

    if (limDeactivateMinChannelTimerDuringScan(pMac) != eSIR_SUCCESS)
    {
        palFreeMemory(pMac->hHdd, pProbeRsp);
        return;
    }

    /*                                                                        
                                        
                                          
                                     
                                              
                                                                  
        
                                                       
     */
    if( (pMac->lim.gLimMlmState == eLIM_MLM_WT_PROBE_RESP_STATE) ||
        (pMac->lim.gLimMlmState == eLIM_MLM_PASSIVE_SCAN_STATE)  ||     //                                           
        (pMac->lim.gLimMlmState == eLIM_MLM_LEARN_STATE) )
    {
        frameLen = WDA_GET_RX_PAYLOAD_LEN(pRxPacketInfo);

        //                                         
        pBody = WDA_GET_RX_MPDU_DATA(pRxPacketInfo);

        if (sirConvertProbeFrame2Struct(pMac, pBody, frameLen, pProbeRsp) == eSIR_FAILURE)
        {
            limLog(pMac, LOG1, FL("Parse error ProbeResponse, length=%d\n"), frameLen);
            palFreeMemory(pMac->hHdd, pProbeRsp);
            return;
        }

        if( (pMac->lim.gLimMlmState == eLIM_MLM_WT_PROBE_RESP_STATE) ||
             (pMac->lim.gLimMlmState == eLIM_MLM_PASSIVE_SCAN_STATE) )
            limCheckAndAddBssDescription(pMac, pProbeRsp, pRxPacketInfo, eANI_BOOLEAN_TRUE, eANI_BOOLEAN_TRUE);
        else if (pMac->lim.gLimMlmState == eLIM_MLM_LEARN_STATE)
        {
#if defined(ANI_PRODUCT_TYPE_AP) && (WNI_POLARIS_FW_PACKAGE == ADVANCED)
            //                        
            /*                                                                                            
                                                                                                      
                                               */
            //                                              
              //                                                                                  
            limCollectMeasurementData(pMac, pRxPacketInfo, pProbeRsp);
            limLog(pMac, LOG3,
               FL("Parsed WDS info in ProbeRsp frames: wdsLength=%d\n"),
               pProbeRsp->propIEinfo.wdsLength);
#endif
        }
    } 
    palFreeMemory(pMac->hHdd, pProbeRsp);
    return;
} /*                                      */
