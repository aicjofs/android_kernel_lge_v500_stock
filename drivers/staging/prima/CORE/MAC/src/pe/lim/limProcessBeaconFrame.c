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

#if (WNI_POLARIS_FW_PRODUCT == AP)
#include "wniCfgAp.h"
#else
#include "wniCfgSta.h"
#endif
#include "aniGlobal.h"
#include "cfgApi.h"
#include "schApi.h"
#include "wniCfgAp.h"
#ifdef FEATURE_WLAN_NON_INTEGRATED_SOC
#include "halCommonApi.h"
#endif
#include "utilsApi.h"
#include "limTypes.h"
#include "limUtils.h"
#include "limAssocUtils.h"
#include "limPropExtsUtils.h"
#include "limSerDesUtils.h"

/* 
                        
  
           
                                                                  
                   
  
        
  
              
  
       
                                                               
                               
  
                                                 
                                                                 
               
 */

void
limProcessBeaconFrame(tpAniSirGlobal pMac, tANI_U8 *pRxPacketInfo,tpPESession psessionEntry)
{
    tpSirMacMgmtHdr      pHdr;
    tSchBeaconStruct    *pBeacon;

    pMac->lim.gLimNumBeaconsRcvd++;

    /*                                                                            */  


    
    pHdr = WDA_GET_RX_MAC_HEADER(pRxPacketInfo);


    PELOG2(limLog(pMac, LOG2, FL("Received Beacon frame with length=%d from "),
           WDA_GET_RX_MPDU_LEN(pRxPacketInfo));
    limPrintMacAddr(pMac, pHdr->sa, LOG2);)

    if (limDeactivateMinChannelTimerDuringScan(pMac) != eSIR_SUCCESS)
        return;


    /* 
                              
                                                                  
                                                       
                                   
                                 
     */
    if ((pMac->lim.gLimMlmState == eLIM_MLM_WT_PROBE_RESP_STATE) ||
        (pMac->lim.gLimMlmState == eLIM_MLM_PASSIVE_SCAN_STATE) ||
        (pMac->lim.gLimMlmState == eLIM_MLM_LEARN_STATE) ||
        (psessionEntry->limMlmState == eLIM_MLM_WT_JOIN_BEACON_STATE))
    {
        if(eHAL_STATUS_SUCCESS != palAllocateMemory(pMac->hHdd, 
                                                    (void **)&pBeacon, sizeof(tSchBeaconStruct)))
        {
            limLog(pMac, LOGE, FL("Unable to PAL allocate memory in limProcessBeaconFrame\n") );
            return;
        }

        //                      
        if (sirConvertBeaconFrame2Struct(pMac, (tANI_U8 *) pRxPacketInfo,
                                         pBeacon) != eSIR_SUCCESS)
        {
            //                                           
            //                       
            limLog(pMac, LOGW,
                   FL("Received invalid Beacon in state %X\n"),
                   psessionEntry->limMlmState);
            limPrintMlmState(pMac, LOGW,  psessionEntry->limMlmState);
            palFreeMemory(pMac->hHdd, pBeacon);
            return;
        }


        MTRACE(macTrace(pMac, TRACE_CODE_RX_MGMT_TSF, 0, pBeacon->timeStamp[0]);)
        MTRACE(macTrace(pMac, TRACE_CODE_RX_MGMT_TSF, 0, pBeacon->timeStamp[1]);)


        if ((pMac->lim.gLimMlmState  == eLIM_MLM_WT_PROBE_RESP_STATE) ||
            (pMac->lim.gLimMlmState  == eLIM_MLM_PASSIVE_SCAN_STATE))
        {
#ifdef WLAN_FEATURE_P2P
            //                                                 
            if((pMac->lim.gLimHalScanState != eLIM_HAL_SCANNING_STATE) || (NULL == pMac->lim.gpLimMlmScanReq) 
               || !pMac->lim.gpLimMlmScanReq->p2pSearch )
#endif
            {
                limCheckAndAddBssDescription(pMac, pBeacon, pRxPacketInfo, 
                       ((pMac->lim.gLimHalScanState == eLIM_HAL_SCANNING_STATE) ? eANI_BOOLEAN_TRUE : eANI_BOOLEAN_FALSE), 
                       eANI_BOOLEAN_FALSE);
            }
        }
        else if (pMac->lim.gLimMlmState == eLIM_MLM_LEARN_STATE)
        {
#if (WNI_POLARIS_FW_PRODUCT == AP) && (WNI_POLARIS_FW_PACKAGE == ADVANCED)
            //                        
            /*                                                                                            
                                                                                                      
                                               */
            //                                              
              //                                                                                
            limCollectMeasurementData(pMac, pRxPacketInfo, pBeacon);
           PELOG3(limLog(pMac, LOG3, FL("Parsed WDS info in Beacon frames: wdsLength=%d\n"),
               pBeacon->propIEinfo.wdsLength);)
#endif
        }
        else
        {
            if( psessionEntry->beacon != NULL )
            {
                palFreeMemory(pMac->hHdd, psessionEntry->beacon);
                psessionEntry->beacon = NULL;
             }
             psessionEntry->bcnLen = WDA_GET_RX_PAYLOAD_LEN(pRxPacketInfo);
             if( (palAllocateMemory(pMac->hHdd, (void**)&psessionEntry->beacon, psessionEntry->bcnLen)) != eHAL_STATUS_SUCCESS)
             {
                PELOGE(limLog(pMac, LOGE, FL("Unable to allocate memory to store beacon"));)
              }
              else
              {
                //                                                                         
                palCopyMemory(pMac->hHdd, psessionEntry->beacon, WDA_GET_RX_MPDU_DATA(pRxPacketInfo), psessionEntry->bcnLen);

               }
             
             //                                   
            limCheckAndAnnounceJoinSuccess(pMac, pBeacon, pHdr,psessionEntry);
        } //                                                            
        palFreeMemory(pMac->hHdd, pBeacon);
    } //                                                                    
    else
    {
        //                                        
        if (psessionEntry->limMlmState == eLIM_MLM_JOINED_STATE ||
            psessionEntry->limMlmState  == eLIM_MLM_BSS_STARTED_STATE ||
            psessionEntry->limMlmState  == eLIM_MLM_WT_AUTH_FRAME2_STATE ||
            psessionEntry->limMlmState == eLIM_MLM_WT_AUTH_FRAME3_STATE ||
            psessionEntry->limMlmState  == eLIM_MLM_WT_AUTH_FRAME4_STATE ||
            psessionEntry->limMlmState  == eLIM_MLM_AUTH_RSP_TIMEOUT_STATE ||
            psessionEntry->limMlmState == eLIM_MLM_AUTHENTICATED_STATE ||
            psessionEntry->limMlmState  == eLIM_MLM_WT_ASSOC_RSP_STATE ||
            psessionEntry->limMlmState == eLIM_MLM_WT_REASSOC_RSP_STATE ||
            psessionEntry->limMlmState  == eLIM_MLM_ASSOCIATED_STATE ||
            psessionEntry->limMlmState  == eLIM_MLM_REASSOCIATED_STATE ||
            psessionEntry->limMlmState  == eLIM_MLM_WT_ASSOC_CNF_STATE ||
            limIsReassocInProgress(pMac,psessionEntry)) {
            //                                                
            pMac->lim.gLimNumBeaconsIgnored++;
        }
        else
        {
            PELOG1(limLog(pMac, LOG1, FL("Received Beacon in unexpected state %d\n"),
                   psessionEntry->limMlmState);
            limPrintMlmState(pMac, LOG1, psessionEntry->limMlmState);)
#ifdef WLAN_DEBUG                    
            pMac->lim.gLimUnexpBcnCnt++;
#endif
        }
    }

    return;
} /*                                 */


/*                                                                
                                      
                                                           
                               
 
           
                                                                
            
                                                                  */
void
limProcessBeaconFrameNoSession(tpAniSirGlobal pMac, tANI_U8 *pRxPacketInfo)
{
    tpSirMacMgmtHdr      pHdr;
    tSchBeaconStruct    *pBeacon;

    pMac->lim.gLimNumBeaconsRcvd++;
    pHdr = WDA_GET_RX_MAC_HEADER(pRxPacketInfo);

    limLog(pMac, LOG2, FL("Received Beacon frame with length=%d from "),
           WDA_GET_RX_MPDU_LEN(pRxPacketInfo));
    limPrintMacAddr(pMac, pHdr->sa, LOG2);

    if (limDeactivateMinChannelTimerDuringScan(pMac) != eSIR_SUCCESS)
        return;


    /* 
                                                               
                                                                  
                                 
     */
    if ((pMac->lim.gLimMlmState == eLIM_MLM_WT_PROBE_RESP_STATE) ||
        (pMac->lim.gLimMlmState == eLIM_MLM_PASSIVE_SCAN_STATE) ||
        (pMac->lim.gLimMlmState == eLIM_MLM_LEARN_STATE))
    {
        if(eHAL_STATUS_SUCCESS != palAllocateMemory(pMac->hHdd, 
                                                    (void **)&pBeacon, sizeof(tSchBeaconStruct)))
        {
            limLog(pMac, LOGE, FL("Unable to PAL allocate memory in limProcessBeaconFrameNoSession\n") );
            return;
        }

        if (sirConvertBeaconFrame2Struct(pMac, (tANI_U8 *) pRxPacketInfo, pBeacon) != eSIR_SUCCESS)
        {
            //                                                                
            limLog(pMac, LOGW, FL("Received invalid Beacon in global MLM state %X\n"), pMac->lim.gLimMlmState);
            limPrintMlmState(pMac, LOGW,  pMac->lim.gLimMlmState);
            palFreeMemory(pMac->hHdd, pBeacon);
            return;
        }

        if ( (pMac->lim.gLimMlmState == eLIM_MLM_WT_PROBE_RESP_STATE) ||
             (pMac->lim.gLimMlmState == eLIM_MLM_PASSIVE_SCAN_STATE) )
        {
#ifdef WLAN_FEATURE_P2P
            //                                                 
            if((pMac->lim.gLimHalScanState != eLIM_HAL_SCANNING_STATE) || (NULL == pMac->lim.gpLimMlmScanReq) 
               || !pMac->lim.gpLimMlmScanReq->p2pSearch )
#endif
            {
                limCheckAndAddBssDescription(pMac, pBeacon, pRxPacketInfo, eANI_BOOLEAN_TRUE, eANI_BOOLEAN_FALSE);
            }
        }
        else if (pMac->lim.gLimMlmState == eLIM_MLM_LEARN_STATE)
        {
#if (WNI_POLARIS_FW_PRODUCT == AP) && (WNI_POLARIS_FW_PACKAGE == ADVANCED)
            //                        
            /*                                                                                            
                                                                                                      
                                               */
            //                                              
              //                                                                                
            limCollectMeasurementData(pMac, pRxPacketInfo, pBeacon);
            limLog(pMac, LOG3, FL("Parsed WDS info in Beacon frames: wdsLength=%d\n"),
               pBeacon->propIEinfo.wdsLength);
#endif
        }  //                                    
        palFreeMemory(pMac->hHdd, pBeacon);
    } //                                                                       
    else
    {
        limLog(pMac, LOG1, FL("Rcvd Beacon in unexpected MLM state %d\n"), pMac->lim.gLimMlmState);
        limPrintMlmState(pMac, LOG1, pMac->lim.gLimMlmState);
#ifdef WLAN_DEBUG                    
        pMac->lim.gLimUnexpBcnCnt++;
#endif
    }

    return;
} /*                                          */

