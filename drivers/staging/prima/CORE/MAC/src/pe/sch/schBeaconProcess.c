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
#include "wniCfgAp.h"

#include "cfgApi.h"
#include "pmmApi.h"
#include "limApi.h"
#include "utilsApi.h"
#include "schDebug.h"
#include "schApi.h"

#ifdef FEATURE_WLAN_NON_INTEGRATED_SOC
#include "halCommonApi.h"
#endif

#include "limUtils.h"
#include "limSendMessages.h"
#include "limStaHashApi.h"

#if defined WLAN_FEATURE_VOWIFI
#include "rrmApi.h"
#endif

#ifdef FEATURE_WLAN_DIAG_SUPPORT
#include "vos_diag_core_log.h"
#endif //                          

/* 
                                                                     
                                                
 */
#define SCH_BEACON_LEN_DELTA       3

//                   
#define CW_GET(cw) (((cw) == 0) ? 1 : ((1 << (cw)) - 1))

static void
ap_beacon_process(
    tpAniSirGlobal    pMac,
    tANI_U8*      pRxPacketInfo,
    tpSchBeaconStruct pBcnStruct,
    tpUpdateBeaconParams pBeaconParams,
    tpPESession         psessionEntry)
{
    tpSirMacMgmtHdr    pMh = WDA_GET_RX_MAC_HEADER(pRxPacketInfo);
    tANI_U32           phyMode;
    tSirRFBand          rfBand = SIR_BAND_UNKNOWN;
    //                              
    rfBand = psessionEntry->limRFBand;

    limGetPhyMode(pMac, &phyMode, psessionEntry);

    if(SIR_BAND_5_GHZ == rfBand)
    {
        if (psessionEntry->htCapability)
        {
            if (pBcnStruct->channelNumber == psessionEntry->currentOperChannel)
            {
              //                            
              //                                                      
              //                                                                             
              if ((!pBcnStruct->HTInfo.present) ||
                  (eSIR_HT_OP_MODE_MIXED == pBcnStruct->HTInfo.opMode) ||
                  (eSIR_HT_OP_MODE_OVERLAP_LEGACY == pBcnStruct->HTInfo.opMode))
              {
                   limUpdateOverlapStaParam(pMac, pMh->bssId, &(pMac->lim.gLimOverlap11aParams));

                  if (pMac->lim.gLimOverlap11aParams.numSta &&
                      !pMac->lim.gLimOverlap11aParams.protectionEnabled)
                  {
                      limEnable11aProtection(pMac, true, true, pBeaconParams,psessionEntry);
                  }
              }
              //                                 
              else if(eSIR_HT_OP_MODE_NO_LEGACY_20MHZ_HT == pBcnStruct->HTInfo.opMode)
              {
                  limUpdateOverlapStaParam(pMac, pMh->bssId, &(pMac->lim.gLimOverlapHt20Params));

                  if (pMac->lim.gLimOverlapHt20Params.numSta &&
                      !pMac->lim.gLimOverlapHt20Params.protectionEnabled)
                  {
                      limEnableHT20Protection(pMac, true, true, pBeaconParams,psessionEntry);
                  }
              }
            }
        }    
    }
    else if(SIR_BAND_2_4_GHZ == rfBand)
    {
        //              
        if ((phyMode == WNI_CFG_PHY_MODE_11G) &&
              (false == psessionEntry->htCapability))
        {
            if (pBcnStruct->channelNumber == psessionEntry->currentOperChannel)        
            {
                if (((!(pBcnStruct->erpPresent)) && 
                      !(pBcnStruct->HTInfo.present))|| 
                    //                                           
                    (pBcnStruct->erpPresent &&
                    (pBcnStruct->erpIEInfo.useProtection ||
                    pBcnStruct->erpIEInfo.nonErpPresent)))
                {
#ifdef FEATURE_WLAN_CCX
                    if( psessionEntry->isCCXconnection )
                    {
                        VOS_TRACE (VOS_MODULE_ID_PE, VOS_TRACE_LEVEL_INFO, 
                            "%s: [INFOLOG]CCX 11g erpPresent=%d useProtection=%d nonErpPresent=%d\n", __func__,
                            pBcnStruct->erpPresent,
                            pBcnStruct->erpIEInfo.useProtection,
                            pBcnStruct->erpIEInfo.nonErpPresent);
                    } 
#endif 
                    limEnableOverlap11gProtection(pMac, pBeaconParams, pMh,psessionEntry);
                }

            }
        }        
        //                                                         
        else if(psessionEntry->htCapability)
        {             
            if (pBcnStruct->channelNumber == psessionEntry->currentOperChannel)
            {
              if (((!(pBcnStruct->erpPresent)) && 
                    !(pBcnStruct->HTInfo.present))|| 
                  //                                           
                  (pBcnStruct->erpPresent &&
                  (pBcnStruct->erpIEInfo.useProtection ||
                  pBcnStruct->erpIEInfo.nonErpPresent)))
              {
#ifdef FEATURE_WLAN_CCX
                  if( psessionEntry->isCCXconnection )
                  {
                      VOS_TRACE (VOS_MODULE_ID_PE, VOS_TRACE_LEVEL_INFO, 
                          "%s: [INFOLOG]CCX 11g erpPresent=%d useProtection=%d nonErpPresent=%d\n", __func__,
                          pBcnStruct->erpPresent,
                          pBcnStruct->erpIEInfo.useProtection,
                          pBcnStruct->erpIEInfo.nonErpPresent);
                  }  
#endif 
                  limEnableOverlap11gProtection(pMac, pBeaconParams, pMh,psessionEntry);
              }

              //                   
              if (pBcnStruct->erpPresent &&
                  !(pBcnStruct->erpIEInfo.useProtection || 
                    pBcnStruct->erpIEInfo.nonErpPresent) && !(pBcnStruct->HTInfo.present))
              {
#ifdef WLAN_SOFTAP_FEATURE
                    limUpdateOverlapStaParam(pMac, pMh->bssId, &(psessionEntry->gLimOverlap11gParams));

                  if (psessionEntry->gLimOverlap11gParams.numSta && 
                      !psessionEntry->gLimOverlap11gParams.protectionEnabled)
#else
                   limUpdateOverlapStaParam(pMac, pMh->bssId, &(pMac->lim.gLimOverlap11gParams));

                  if (pMac->lim.gLimOverlap11gParams.numSta &&
                      !pMac->lim.gLimOverlap11gParams.protectionEnabled)
#endif
                  {
                      limEnableHtProtectionFrom11g(pMac, true, true, pBeaconParams,psessionEntry);
                  }
              }

              //                   
              //                                                                           
              //                                                             
              //                                                    
              if (pBcnStruct->HTInfo.present)
              {
                  //                                                                       
                  //                                                   
                  //                                                    
                  //                                                                             
                  //                                                                  
                  if(eSIR_HT_OP_MODE_OVERLAP_LEGACY == pBcnStruct->HTInfo.opMode)
                  {
                      if((eSIR_HT_OP_MODE_MIXED != pMac->lim.gHTOperMode) &&
                          (eSIR_HT_OP_MODE_OVERLAP_LEGACY != pMac->lim.gHTOperMode))
                      {
#ifdef WLAN_SOFTAP_FEATURE
                          limUpdateOverlapStaParam(pMac, pMh->bssId, &(psessionEntry->gLimOverlap11gParams));
                          if (psessionEntry->gLimOverlap11gParams.numSta &&
                              !psessionEntry->gLimOverlap11gParams.protectionEnabled)
#else
                          limUpdateOverlapStaParam(pMac, pMh->bssId, &(pMac->lim.gLimOverlap11gParams));

                          if (pMac->lim.gLimOverlap11gParams.numSta &&
                              !pMac->lim.gLimOverlap11gParams.protectionEnabled)
#endif
                          {
                              limEnableHtProtectionFrom11g(pMac, true, true, pBeaconParams,psessionEntry);
                          }
                      }
                  }           
                  else if(eSIR_HT_OP_MODE_NO_LEGACY_20MHZ_HT == pBcnStruct->HTInfo.opMode)
                  {
#ifdef WLAN_SOFTAP_FEATURE
                      limUpdateOverlapStaParam(pMac, pMh->bssId, &(psessionEntry->gLimOverlapHt20Params));
                      if (psessionEntry->gLimOverlapHt20Params.numSta &&
                          !psessionEntry->gLimOverlapHt20Params.protectionEnabled)
#else
                      limUpdateOverlapStaParam(pMac, pMh->bssId, &(pMac->lim.gLimOverlapHt20Params));

                      if (pMac->lim.gLimOverlapHt20Params.numSta &&
                          !pMac->lim.gLimOverlapHt20Params.protectionEnabled)
#endif
                      {
                          limEnableHT20Protection(pMac, true, true, pBeaconParams,psessionEntry);
                      }
                  }
              }
              
            }
        }     
    }
    pMac->sch.gSchBcnIgnored++;
}
//                                                                     




/* 
                              
  
            
                                          
                               
                                        
  
         
                                                                
                                                                                
                                                                                  
                                                                                                     
  
                                                                                                    
                                                                             
                  
                              
                                                            
                                                                                      
                                                                                                                       
                                                              
                                                      
                                                                       
                                                     
                                                                   
                                                      
                                                                                
                                                      
   

 */
static void __schBeaconProcessNoSession(tpAniSirGlobal pMac, tpSchBeaconStruct pBeacon,tANI_U8* pRxPacketInfo)
{
    tpPESession psessionEntry = NULL;
 
    if(  (psessionEntry = limIsIBSSSessionActive(pMac)) != NULL)
    {
        limHandleIBSScoalescing(pMac, pBeacon, pRxPacketInfo, psessionEntry);
    }

    //                                                                                                                    
    //                                                                                                        
    //              
    limCheckAndAddBssDescription(pMac, pBeacon, pRxPacketInfo, eANI_BOOLEAN_FALSE, eANI_BOOLEAN_FALSE);
    return;  
}



/*
                               
  
            
                                          
                               
                                     
  
         
                                                       
                                                                                 
                                                                            
                                                                            
                                                                             
                                                         
                                                                             
                                                           
                                                                               
                                                                                  
                                                      
                                                                 
                                                                             
                                                                  
                                                                              
                                                                                          
                                                                                                     
                                                                  
                                                                               
                                                            
                                                                   
                                                   
                                                                 
                                                                                               
  
                                                                 
                                                   
 */

static void __schBeaconProcessForSession( tpAniSirGlobal      pMac,
                                                                     tpSchBeaconStruct   pBeacon,
                                                                     tANI_U8* pRxPacketInfo,    
                                                                     tpPESession psessionEntry)
{
    tANI_U32                     bi;
    tANI_U8 bssIdx = 0;
    //                                                                      
    //                                   
    tUpdateBeaconParams beaconParams;
    tANI_U8 sendProbeReq = FALSE;
    tpDphHashNode pStaDs = NULL;


    beaconParams.paramChangeBitmap = 0;

    if(eLIM_STA_IN_IBSS_ROLE == psessionEntry->limSystemRole )
    {
        limHandleIBSScoalescing(pMac, pBeacon,  pRxPacketInfo, psessionEntry);
    }
    else if(  (eLIM_STA_ROLE == psessionEntry->limSystemRole) || 
                  (eLIM_BT_AMP_STA_ROLE == psessionEntry->limSystemRole))
    {
        /*
                                  
                                                  
                                                      
        */
        
    
        //                                                     
        limCheckAndAddBssDescription(pMac, pBeacon, pRxPacketInfo, eANI_BOOLEAN_FALSE, eANI_BOOLEAN_FALSE);
        
        /* 
                                                                                                
                                                               
               */
        if((tANI_U8) pBeacon->channelNumber != psessionEntry->currentOperChannel)
        {
            PELOGE(limLog(pMac, LOGE, FL("Channel Change from %d --> %d  - "
                                         "Ignoring beacon!\n"), 
                          psessionEntry->currentOperChannel, pBeacon->channelNumber);)
           goto fail;
        }
        limDetectChangeInApCapabilities(pMac, pBeacon, psessionEntry);
        if(limGetStaHashBssidx(pMac, DPH_STA_HASH_INDEX_PEER, &bssIdx, psessionEntry) != eSIR_SUCCESS)
            goto fail;
        beaconParams.bssIdx = bssIdx;
        palCopyMemory( pMac->hHdd, ( tANI_U8* )&psessionEntry->lastBeaconTimeStamp, ( tANI_U8* )pBeacon->timeStamp, sizeof(tANI_U64) );
        psessionEntry->lastBeaconDtimCount = pBeacon->tim.dtimCount;
        psessionEntry->lastBeaconDtimPeriod= pBeacon->tim.dtimPeriod;
        psessionEntry->currentBssBeaconCnt++;



        MTRACE(macTrace(pMac, TRACE_CODE_RX_MGMT_TSF, psessionEntry->peSessionId, pBeacon->timeStamp[0]);)
        MTRACE(macTrace(pMac, TRACE_CODE_RX_MGMT_TSF, psessionEntry->peSessionId, pBeacon->timeStamp[1]);)

        /*                                    */
        bi = psessionEntry->beaconParams.beaconInterval;
        if (bi != pBeacon->beaconInterval)
        {
           PELOG1(schLog(pMac, LOG1, FL("Beacon interval changed from %d to %d\n"),
                   pBeacon->beaconInterval, bi);)

            bi = pBeacon->beaconInterval;
            psessionEntry->beaconParams.beaconInterval = (tANI_U16) bi;
            beaconParams.paramChangeBitmap |= PARAM_BCN_INTERVAL_CHANGED;
            beaconParams.beaconInterval = (tANI_U16)bi;
        }

        if (pBeacon->cfPresent)
        {
            cfgSetInt(pMac, WNI_CFG_CFP_PERIOD, pBeacon->cfParamSet.cfpPeriod);
            limSendCFParams(pMac, bssIdx, pBeacon->cfParamSet.cfpCount, pBeacon->cfParamSet.cfpPeriod);
        }

        if (pBeacon->timPresent)
        {
            cfgSetInt(pMac, WNI_CFG_DTIM_PERIOD, pBeacon->tim.dtimPeriod);
            //                                                 
            //                            
        }

        
        if(pMac->lim.gLimProtectionControl != WNI_CFG_FORCE_POLICY_PROTECTION_DISABLE)

        limDecideStaProtection(pMac, pBeacon, &beaconParams, psessionEntry);
        if (pBeacon->erpPresent)
        {
#ifdef WLAN_SOFTAP_FEATURE
            if (pBeacon->erpIEInfo.barkerPreambleMode)
                limEnableShortPreamble(pMac, false, &beaconParams, psessionEntry);
            else
                limEnableShortPreamble(pMac, true, &beaconParams, psessionEntry);
#else
            if (pBeacon->erpIEInfo.barkerPreambleMode)
                limEnableShortPreamble(pMac, false, &beaconParams);
            else
                limEnableShortPreamble(pMac, true, &beaconParams);
#endif
          }
        limUpdateShortSlot(pMac, pBeacon, &beaconParams,psessionEntry);

        pStaDs = dphGetHashEntry(pMac, DPH_STA_HASH_INDEX_PEER, &psessionEntry->dph.dphHashTable);
        if ((pBeacon->wmeEdcaPresent && (psessionEntry->limWmeEnabled)) ||
             (pBeacon->edcaPresent    && (psessionEntry->limQosEnabled)))
        {
            if(pBeacon->edcaParams.qosInfo.count != psessionEntry->gLimEdcaParamSetCount)
            {
                if (schBeaconEdcaProcess(pMac, &pBeacon->edcaParams, psessionEntry) != eSIR_SUCCESS)
                    PELOGE(schLog(pMac, LOGE, FL("EDCA parameter processing error\n"));)
                else if(pStaDs != NULL)
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
        else if( (pBeacon->qosCapabilityPresent && psessionEntry->limQosEnabled) &&
            (pBeacon->qosCapability.qosInfo.count != psessionEntry->gLimEdcaParamSetCount))
            sendProbeReq = TRUE;
    }

    if ( psessionEntry->htCapability && pBeacon->HTInfo.present )
    {
        limUpdateStaRunTimeHTSwitchChnlParams( pMac, &pBeacon->HTInfo, bssIdx,psessionEntry);
    }

#if defined(ANI_PRODUCT_TYPE_CLIENT) || defined(ANI_AP_CLIENT_SDK)
    if ( (psessionEntry->limSystemRole == eLIM_STA_ROLE) ||(psessionEntry->limSystemRole == eLIM_BT_AMP_STA_ROLE) ||
          (psessionEntry->limSystemRole == eLIM_STA_IN_IBSS_ROLE) )
    {
        if(pBeacon->quietIEPresent)
        {
            limUpdateQuietIEFromBeacon(pMac, &(pBeacon->quietIE), psessionEntry);
        }
        else if ((psessionEntry->gLimSpecMgmt.quietState == eLIM_QUIET_BEGIN) ||
             (psessionEntry->gLimSpecMgmt.quietState == eLIM_QUIET_RUNNING))
        {
            PELOG1(limLog(pMac, LOG1, FL("Received a beacon without Quiet IE\n"));)
            limCancelDot11hQuiet(pMac, psessionEntry);
        }

        /*                                            */
        if(pBeacon->channelSwitchPresent || 
            pBeacon->propIEinfo.propChannelSwitchPresent)
        {
            limUpdateChannelSwitch(pMac, pBeacon, psessionEntry);
        }
        else if (psessionEntry->gLimSpecMgmt.dot11hChanSwState == eLIM_11H_CHANSW_RUNNING)
        {
            limCancelDot11hChannelSwitch(pMac, psessionEntry);
        }   
    }
#endif

#if defined FEATURE_WLAN_CCX
        if( psessionEntry->isCCXconnection )
        {
           tPowerdBm  localConstraint = 0, regMax = 0, maxTxPower = 0;
           if (pBeacon->ccxTxPwr.present)
           {
              localConstraint = pBeacon->ccxTxPwr.power_limit;
              regMax = cfgGetRegulatoryMaxTransmitPower( pMac, psessionEntry->currentOperChannel ); 
              maxTxPower = limGetMaxTxPower(regMax, localConstraint, pMac->roam.configParam.nTxPowerCap);

              //                                       
             if( maxTxPower != psessionEntry->maxTxPower )
             {
                limLog( pMac, LOG1, "RegMax = %d, lpc = %d, MaxTx = %d", regMax, localConstraint, maxTxPower );
                limLog( pMac, LOG1, "Local power constraint change..updating new maxTx power to HAL");
                if( limSendSetMaxTxPowerReq ( pMac, maxTxPower, psessionEntry ) == eSIR_SUCCESS )
                   psessionEntry->maxTxPower = maxTxPower;
             }
           }
        }
#endif


#if defined WLAN_FEATURE_VOWIFI
        if( pMac->rrm.rrmPEContext.rrmEnable )
        {
           tPowerdBm  localConstraint = 0, regMax = 0, maxTxPower = 0;
           if (pBeacon->powerConstraintPresent && pMac->rrm.rrmPEContext.rrmEnable)
           {
              localConstraint = pBeacon->localPowerConstraint.localPowerConstraints;
           }
           else
           {
              localConstraint = 0;
           }
           regMax = cfgGetRegulatoryMaxTransmitPower( pMac, psessionEntry->currentOperChannel ); 
           maxTxPower = VOS_MIN( regMax , (regMax - localConstraint) );
           //                                       
           if( maxTxPower != psessionEntry->maxTxPower )
           {
#if defined WLAN_VOWIFI_DEBUG
              limLog( pMac, LOGE, "Regulatory max = %d, local power constraint = %d, max tx = %d", regMax, localConstraint, maxTxPower );
              limLog( pMac, LOGE, "Local power constraint change..updating mew maxTx power to HAL");
#endif
              if( rrmSendSetMaxTxPowerReq ( pMac, maxTxPower, psessionEntry ) == eSIR_SUCCESS )
                 psessionEntry->maxTxPower = maxTxPower;

           }
        }
#endif

    //                                        

    if (pBeacon->HTInfo.present)
        limReceivedHBHandler(pMac, (tANI_U8)pBeacon->HTInfo.primaryChannel, psessionEntry);
    else
        limReceivedHBHandler(pMac, (tANI_U8)pBeacon->channelNumber, psessionEntry);

    //                                                                                  
    if(sendProbeReq)
        limSendProbeReqMgmtFrame(pMac, &psessionEntry->ssId,
            psessionEntry->bssId, psessionEntry->currentOperChannel,psessionEntry->selfMacAddr,
            psessionEntry->dot11mode, 0, NULL);

    if(beaconParams.paramChangeBitmap)
    {
        PELOGW(schLog(pMac, LOGW, FL("Beacon for session[%d] got changed. \n"), psessionEntry->peSessionId);)
        PELOGW(schLog(pMac, LOGW, FL("sending beacon param change bitmap: 0x%x \n"), beaconParams.paramChangeBitmap);)
        limSendBeaconParams(pMac, &beaconParams, psessionEntry);
    }

fail:
    return;

}



/* 
                   
  
            
                                    
  
         
   
               
  
        
  
                                                    
               
 */
 
void schBeaconProcess(tpAniSirGlobal pMac, tANI_U8* pRxPacketInfo, tpPESession psessionEntry)
{
    static tSchBeaconStruct beaconStruct;
    tUpdateBeaconParams beaconParams;
    tpPESession pAPSession = NULL;
    beaconParams.paramChangeBitmap = 0;

    pMac->sch.gSchBcnRcvCnt++;

    //                                          
    if (sirConvertBeaconFrame2Struct(pMac, (tANI_U8 *) pRxPacketInfo, &beaconStruct)!= eSIR_SUCCESS)
    {
        PELOGE(schLog(pMac, LOGE, FL("beacon parsing failed\n"));)
        pMac->sch.gSchBcnParseErrorCnt++;
        return;
    }

    if (beaconStruct.ssidPresent)
    {
        beaconStruct.ssId.ssId[beaconStruct.ssId.length] = 0;
    }

    /*
                                                                                 
                                                  
                                
                                                                                                                         
                                    
                                                                                                           
                                                                                                                 
      
      
    */
    
    if((pAPSession = limIsApSessionActive(pMac)) != NULL)
    {
        beaconParams.bssIdx = pAPSession->bssIdx;
#ifdef WLAN_SOFTAP_FEATURE
        if (pAPSession->gLimProtectionControl != WNI_CFG_FORCE_POLICY_PROTECTION_DISABLE)
#else
        if (pMac->lim.gLimProtectionControl != WNI_CFG_FORCE_POLICY_PROTECTION_DISABLE)
#endif
            ap_beacon_process(pMac,  pRxPacketInfo, &beaconStruct, &beaconParams, pAPSession);

        if (beaconParams.paramChangeBitmap)
        {
            //                                                    
            schSetFixedBeaconFields(pMac, pAPSession);
            PELOG1(schLog(pMac, LOG1, FL("Beacon for PE session[%d] got changed.  \n"), pAPSession->peSessionId);)
            PELOG1(schLog(pMac, LOG1, FL("sending beacon param change bitmap: 0x%x \n"), beaconParams.paramChangeBitmap);)
            limSendBeaconParams(pMac, &beaconParams, pAPSession);
        }
    }

    /*
                                                                                                         
    */
    if(psessionEntry == NULL)
    {
        __schBeaconProcessNoSession(pMac,   &beaconStruct, pRxPacketInfo );   
    }
    else
    {
        __schBeaconProcessForSession(pMac,   &beaconStruct, pRxPacketInfo, psessionEntry );   
    }

}





//                                                                     
/* 
                       
  
            
                                                              
  
         
  
               
  
        
  
                                                            
                  
 */

tSirRetStatus schBeaconEdcaProcess(tpAniSirGlobal pMac, tSirMacEdcaParamSetIE *edca, tpPESession psessionEntry)
{
    tANI_U8 i;
#ifdef FEATURE_WLAN_DIAG_SUPPORT
    vos_log_qos_edca_pkt_type *log_ptr = NULL;
#endif //                          

    PELOG1(schLog(pMac, LOG1, FL("Updating parameter set count: Old %d ---> new %d\n"),
           psessionEntry->gLimEdcaParamSetCount, edca->qosInfo.count);)

    psessionEntry->gLimEdcaParamSetCount = edca->qosInfo.count;
    psessionEntry->gLimEdcaParams[EDCA_AC_BE] = edca->acbe;
    psessionEntry->gLimEdcaParams[EDCA_AC_BK] = edca->acbk;
    psessionEntry->gLimEdcaParams[EDCA_AC_VI] = edca->acvi;
    psessionEntry->gLimEdcaParams[EDCA_AC_VO] = edca->acvo;
//                        
#ifdef FEATURE_WLAN_DIAG_SUPPORT
    WLAN_VOS_DIAG_LOG_ALLOC(log_ptr, vos_log_qos_edca_pkt_type, LOG_WLAN_QOS_EDCA_C);
    if(log_ptr)
    {
       log_ptr->aci_be = psessionEntry->gLimEdcaParams[EDCA_AC_BE].aci.aci;
       log_ptr->cw_be  = psessionEntry->gLimEdcaParams[EDCA_AC_BE].cw.max << 4 |
          psessionEntry->gLimEdcaParams[EDCA_AC_BE].cw.min;
       log_ptr->txoplimit_be = psessionEntry->gLimEdcaParams[EDCA_AC_BE].txoplimit;
       log_ptr->aci_bk = psessionEntry->gLimEdcaParams[EDCA_AC_BK].aci.aci;
       log_ptr->cw_bk  = psessionEntry->gLimEdcaParams[EDCA_AC_BK].cw.max << 4 |
          psessionEntry->gLimEdcaParams[EDCA_AC_BK].cw.min;
       log_ptr->txoplimit_bk = psessionEntry->gLimEdcaParams[EDCA_AC_BK].txoplimit;
       log_ptr->aci_vi = psessionEntry->gLimEdcaParams[EDCA_AC_VI].aci.aci;
       log_ptr->cw_vi  = psessionEntry->gLimEdcaParams[EDCA_AC_VI].cw.max << 4 |
          psessionEntry->gLimEdcaParams[EDCA_AC_VI].cw.min;
       log_ptr->txoplimit_vi = psessionEntry->gLimEdcaParams[EDCA_AC_VI].txoplimit;
       log_ptr->aci_vo = psessionEntry->gLimEdcaParams[EDCA_AC_VO].aci.aci;
       log_ptr->cw_vo  = psessionEntry->gLimEdcaParams[EDCA_AC_VO].cw.max << 4 |
          psessionEntry->gLimEdcaParams[EDCA_AC_VO].cw.min;
       log_ptr->txoplimit_vo = psessionEntry->gLimEdcaParams[EDCA_AC_VO].txoplimit;
    }
    WLAN_VOS_DIAG_LOG_REPORT(log_ptr);
#endif //                         
    PELOG1(schLog(pMac, LOGE, FL("Updating Local EDCA Params(gLimEdcaParams) to: "));)
    for(i=0; i<MAX_NUM_AC; i++)
    {
        PELOG1(schLog(pMac, LOG1, FL("AC[%d]:  AIFSN: %d, ACM %d, CWmin %d, CWmax %d, TxOp %d\n"),
            i,
            psessionEntry->gLimEdcaParams[i].aci.aifsn, 
            psessionEntry->gLimEdcaParams[i].aci.acm,
            psessionEntry->gLimEdcaParams[i].cw.min,
            psessionEntry->gLimEdcaParams[i].cw.max,
            psessionEntry->gLimEdcaParams[i].txoplimit);)
    }

    return eSIR_SUCCESS;
}
