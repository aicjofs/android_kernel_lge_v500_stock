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
#include "aniGlobal.h"
#include "wniCfgAp.h"
#include "sirApi.h"
#include "cfgApi.h"

#include "schApi.h"
#include "pmmApi.h"
#include "utilsApi.h"
#include "limTypes.h"
#include "limUtils.h"
#include "limAssocUtils.h"
#include "limSecurityUtils.h"
#include "limSerDesUtils.h"
#include "limStaHashApi.h"
#include "limAdmitControl.h"
#include "palApi.h"


#include "vos_types.h"
/* 
                              
  
           
                                                          
                                                            
                                                            
  
       
  
                                                           
                                                            
                                                                  
  
               
 */
static void
limConvertSupportedChannels(tpAniSirGlobal pMac,
                            tpLimMlmAssocInd pMlmAssocInd,
                            tSirAssocReq *assocReq)
{

    tANI_U16   i, j, index=0;
    tANI_U8    firstChannelNumber;
    tANI_U8    numberOfChannel;
    tANI_U8    nextChannelNumber;

    if(assocReq->supportedChannels.length >= SIR_MAX_SUPPORTED_CHANNEL_LIST)
    {
        limLog(pMac, LOG1, FL("Number of supported channels:%d is more than MAX\n"),
                              assocReq->supportedChannels.length);
        pMlmAssocInd->supportedChannels.numChnl = 0;
        return;
    }

    for(i=0; i < (assocReq->supportedChannels.length); i++)
    {
        //                         
        firstChannelNumber = assocReq->supportedChannels.supportedChannels[i];
        pMlmAssocInd->supportedChannels.channelList[index] = firstChannelNumber;
        i++;
        index++;
        if (index >= SIR_MAX_SUPPORTED_CHANNEL_LIST)
        {
            pMlmAssocInd->supportedChannels.numChnl = 0;
            return;
        }
        //                                    
        numberOfChannel = assocReq->supportedChannels.supportedChannels[i];
        PELOG2(limLog(pMac, LOG2, FL("Rcv AssocReq: chnl=%d, numOfChnl=%d \n"),
                              firstChannelNumber, numberOfChannel);)

        if (numberOfChannel > 1)
        {
            nextChannelNumber = firstChannelNumber;
            if(SIR_BAND_5_GHZ == limGetRFBand(firstChannelNumber))
            {
                for (j=1; j < numberOfChannel; j++)
                {
                    nextChannelNumber += SIR_11A_FREQUENCY_OFFSET;
                    pMlmAssocInd->supportedChannels.channelList[index] = nextChannelNumber;
                    index++;
                    if (index >= SIR_MAX_SUPPORTED_CHANNEL_LIST)
                    {
                        pMlmAssocInd->supportedChannels.numChnl = 0;
                        return;
                    }
                }
            }
            else if(SIR_BAND_2_4_GHZ == limGetRFBand(firstChannelNumber))
            {
                for (j=1; j < numberOfChannel; j++)
                {
                    nextChannelNumber += SIR_11B_FREQUENCY_OFFSET;
                    pMlmAssocInd->supportedChannels.channelList[index] = nextChannelNumber;
                    index++;
                    if (index >= SIR_MAX_SUPPORTED_CHANNEL_LIST)
                    {
                        pMlmAssocInd->supportedChannels.numChnl = 0;
                        return;
                    }
                }
            }
        }
    }

    pMlmAssocInd->supportedChannels.numChnl = (tANI_U8) index;
   PELOG2(limLog(pMac, LOG2,
        FL("Send AssocInd to WSM: spectrum ON, minPwr %d, maxPwr %d, numChnl %d\n"),
        pMlmAssocInd->powerCap.minTxPower,
        pMlmAssocInd->powerCap.maxTxPower,
        pMlmAssocInd->supportedChannels.numChnl);)
}


/*                                                                
                               
                                                           
                                                       
                                 
 
           
                                                                           
                                                                 
                                                   
            
                                                                  */
void
limProcessAssocReqFrame(tpAniSirGlobal pMac, tANI_U8 *pRxPacketInfo,
                        tANI_U8 subType, tpPESession psessionEntry)
{
    tANI_U8                 updateContext;
    tANI_U8                 *pBody;
    tANI_U16                aid, temp;
    tANI_U32                val;
    tANI_S32                framelen;
    tSirRetStatus           status;
    tpSirMacMgmtHdr         pHdr;
    struct tLimPreAuthNode  *pStaPreAuthContext;
    tAniAuthType            authType;
    tSirMacCapabilityInfo   localCapabilities;
    tpDphHashNode           pStaDs = NULL;
    tpSirAssocReq           pAssocReq;
#ifdef WLAN_SOFTAP_FEATURE
    tLimMlmStates           mlmPrevState;
    tDot11fIERSN            Dot11fIERSN;
    tDot11fIEWPA            Dot11fIEWPA;
#endif
    tANI_U32 phyMode;
    tHalBitVal qosMode;
    tHalBitVal wsmMode, wmeMode;
    tANI_U8    *wpsIe = NULL;
    tSirMacRateSet  basicRates;
    tANI_U8 i = 0, j = 0;

    limGetPhyMode(pMac, &phyMode, psessionEntry);

    limGetQosMode(psessionEntry, &qosMode);

    pHdr = WDA_GET_RX_MAC_HEADER(pRxPacketInfo);
    framelen = WDA_GET_RX_PAYLOAD_LEN(pRxPacketInfo);

   if (psessionEntry->limSystemRole == eLIM_STA_ROLE || psessionEntry->limSystemRole == eLIM_BT_AMP_STA_ROLE )
   {
        limLog(pMac, LOGE, FL("received unexpected ASSOC REQ subType=%d for role=%d, radioId=%d from \n"),
                                            subType, psessionEntry->limSystemRole, pMac->sys.gSirRadioId);
        limPrintMacAddr(pMac, pHdr->sa, LOGE);
        sirDumpBuf(pMac, SIR_LIM_MODULE_ID, LOG3,
        WDA_GET_RX_MPDU_DATA(pRxPacketInfo), framelen);
        return;
    }

    //                                                 
    pBody = WDA_GET_RX_MPDU_DATA(pRxPacketInfo);

    if (limIsGroupAddr(pHdr->sa))
    {
        //                                                 
        //                        
        if (subType == LIM_ASSOC)
			limLog(pMac, LOGW, FL("received Assoc frame from a BC/MC address "MAC_ADDRESS_STR),
                   MAC_ADDR_ARRAY(pHdr->sa));
        else
            limLog(pMac, LOGW, FL("received ReAssoc frame from a BC/MC address "MAC_ADDRESS_STR),
                   MAC_ADDR_ARRAY(pHdr->sa));
        return;
    }
    limLog(pMac, LOGW, FL("Received AssocReq Frame: "MAC_ADDRESS_STR), MAC_ADDR_ARRAY(pHdr->sa));

    sirDumpBuf(pMac, SIR_LIM_MODULE_ID, LOG2, (tANI_U8 *) pBody, framelen);

    if( palEqualMemory( pMac->hHdd,  (tANI_U8* ) pHdr->sa, (tANI_U8 *) pHdr->da, 
                        (tANI_U8) (sizeof(tSirMacAddr))))
    {
        limSendAssocRspMgmtFrame(pMac,
                    eSIR_MAC_UNSPEC_FAILURE_STATUS,
                    1,
                    pHdr->sa,
                    subType, 0,psessionEntry);
        limLog(pMac, LOGE, FL("Rejected Assoc Req frame Since same mac as SAP/GO\n"));
        return ;
    }

#ifdef WLAN_SOFTAP_FEATURE
    //                                                                                                 
    if ((psessionEntry->bTkipCntrMeasActive) && (psessionEntry->limSystemRole == eLIM_AP_ROLE))
    {
        limSendAssocRspMgmtFrame(pMac,
                                    eSIR_MAC_MIC_FAILURE_REASON,
                                    1,
                                    pHdr->sa,
                                    subType, 0, psessionEntry);
        return;
    }
#endif

    //                                            
    if ( palAllocateMemory(pMac->hHdd, (void **)&pAssocReq, sizeof(*pAssocReq)) != eHAL_STATUS_SUCCESS) 
    {
        limLog(pMac, LOGP, FL("PAL Allocate Memory failed in AssocReq\n"));
        return;
    }
    palZeroMemory( pMac->hHdd, (void *)pAssocReq , sizeof(*pAssocReq));
   
    //                          
    if (subType == LIM_ASSOC)
        status = sirConvertAssocReqFrame2Struct(pMac, pBody, framelen, pAssocReq);
    else
        status = sirConvertReassocReqFrame2Struct(pMac, pBody, framelen, pAssocReq);

    if (status != eSIR_SUCCESS)
    {
        limLog(pMac, LOGW, FL("Parse error AssocRequest, length=%d from \n"),framelen);
        limPrintMacAddr(pMac, pHdr->sa, LOGW);
        limSendAssocRspMgmtFrame(pMac, eSIR_MAC_UNSPEC_FAILURE_STATUS, 1, pHdr->sa, subType, 0, psessionEntry);
        goto error;
    }

    if ( palAllocateMemory(pMac->hHdd, (void **)&pAssocReq->assocReqFrame, framelen) != eHAL_STATUS_SUCCESS) 
    {
        limLog(pMac, LOGE, FL("Unable to allocate memory for the assoc req, length=%d from \n"),framelen);
        goto error;
    }
    
    palCopyMemory( pMac->hHdd, (tANI_U8 *) pAssocReq->assocReqFrame,
                  (tANI_U8 *) pBody, framelen);
    pAssocReq->assocReqFrameLength = framelen;    

    if (cfgGetCapabilityInfo(pMac, &temp,psessionEntry) != eSIR_SUCCESS)
    {
        limLog(pMac, LOGP, FL("could not retrieve Capabilities\n"));
        goto error;
    }
#if defined(ANI_PRODUCT_TYPE_AP) && defined(ANI_LITTLE_BYTE_ENDIAN)
    *(tANI_U16*)&localCapabilities=(tANI_U16)(temp);
#else
    limCopyU16((tANI_U8 *) &localCapabilities, temp);
#endif

    if (limCompareCapabilities(pMac,
                               pAssocReq,
                               &localCapabilities,psessionEntry) == false)
    {
        /* 
                                                             
                                                                      
                       
         */
        limSendAssocRspMgmtFrame(
                        pMac,
                        eSIR_MAC_CAPABILITIES_NOT_SUPPORTED_STATUS,
                        1,
                        pHdr->sa,
                        subType, 0,psessionEntry);

        limLog(pMac, LOGW, FL("local caps 0x%x received 0x%x\n"), localCapabilities, pAssocReq->capabilityInfo);

        //          
        if (subType == LIM_ASSOC)
            limLog(pMac, LOGW,
               FL("received Assoc req with unsupported capabilities "MAC_ADDRESS_STR),
                  MAC_ADDR_ARRAY(pHdr->sa));
        else
            limLog(pMac, LOGW,
                   FL("received ReAssoc req with unsupported capabilities "MAC_ADDRESS_STR),
                   MAC_ADDR_ARRAY(pHdr->sa));
        goto error;
    }

    updateContext = false;

#if (WNI_POLARIS_FW_PACKAGE == ADVANCED)
    //                                              
    if (psessionEntry->pLimStartBssReq->ssId.length)
    {
        if (limCmpSSid(pMac, &pAssocReq->ssId, psessionEntry) == false)
        {
            /*                                                                     
                                                                   */
            if ( 0 != vos_get_skip_ssid_check())
            {
                limLog(pMac, LOG1, FL("Received unmatched SSID but cfg to suppress - continuing\n"));
            }
            else
            {
              /* 
                                                           
                                                         
                                     
                                                     
              */
              limSendAssocRspMgmtFrame(pMac,
                               eSIR_MAC_UNSPEC_FAILURE_STATUS,
                               1,
                               pHdr->sa,
                               subType, 0,psessionEntry);
  
              //          
              if (subType == LIM_ASSOC)
                  limLog(pMac, LOGW, FL("received Assoc req with unmatched SSID from \n"));
              else
                  limLog(pMac, LOGW, FL("received ReAssoc req with unmatched SSID from \n"));
              limPrintMacAddr(pMac, pHdr->sa, LOGW);
              goto error;
          }
      }
    }
    else
        limLog(pMac, LOG1, FL("Suppressed SSID, App is going to check SSID\n"));
#else
    if (limCmpSSid(pMac, &pAssocReq->ssId, psessionEntry) == false)
    {
        /* 
                                                      
                                                    
                                
                                                
         */
        limSendAssocRspMgmtFrame(pMac,
                             eSIR_MAC_UNSPEC_FAILURE_STATUS,
                             1,
                             pHdr->sa,
                             subType, 0,psessionEntry);

        //          
        if (subType == LIM_ASSOC)
            limLog(pMac, LOGW,
                   FL("received Assoc req with unmatched SSID from \n"));
        else
            limLog(pMac, LOGW,
                   FL("received ReAssoc req with unmatched SSID from \n"));
        limPrintMacAddr(pMac, pHdr->sa, LOGW);
        goto error;
    }
#endif

    /*                                                              
                                                                      
                                                                     
                            
                                                                    */
    basicRates.numRates = 0;

    for(i = 0; i < pAssocReq->supportedRates.numRates; i++)
    {
        basicRates.rate[i] = pAssocReq->supportedRates.rate[i];
        basicRates.numRates++;
    }

    for(j = 0; (j < pAssocReq->extendedRates.numRates) && (i < SIR_MAC_RATESET_EID_MAX); i++,j++)
    {
        basicRates.rate[i] = pAssocReq->extendedRates.rate[j];
        basicRates.numRates++;
    }
    if (limCheckRxBasicRates(pMac, basicRates, psessionEntry) == false)
    {
        /* 
                                                        
                                                          
                       
         */
        limSendAssocRspMgmtFrame(
                    pMac,
                    eSIR_MAC_BASIC_RATES_NOT_SUPPORTED_STATUS,
                    1,
                    pHdr->sa,
                    subType, 0,psessionEntry);

        //          
        if (subType == LIM_ASSOC)
            limLog(pMac, LOGW,
               FL("received Assoc req with unsupported rates from \n"));
        else
            limLog(pMac, LOGW,
               FL("received ReAssoc req with unsupported rates from\n"));
        limPrintMacAddr(pMac, pHdr->sa, LOGW);
        goto error;
    }

#ifdef WLAN_SOFTAP_FEATURE

    if((psessionEntry->limSystemRole == eLIM_AP_ROLE ) &&
       (psessionEntry->dot11mode == WNI_CFG_DOT11_MODE_11G_ONLY) &&
       ((!pAssocReq->extendedRatesPresent ) || (pAssocReq->HTCaps.present)))
    {
        limSendAssocRspMgmtFrame( pMac, eSIR_MAC_CAPABILITIES_NOT_SUPPORTED_STATUS, 
                                  1, pHdr->sa, subType, 0, psessionEntry );
        limLog(pMac, LOGE, FL("SOFTAP was in 11G only mode, rejecting legacy STA's\n"));
        goto error;

    }//                          
 
    if((psessionEntry->limSystemRole == eLIM_AP_ROLE) && 
       (psessionEntry->dot11mode == WNI_CFG_DOT11_MODE_11N_ONLY) && 
       (!pAssocReq->HTCaps.present))
    {
        limSendAssocRspMgmtFrame( pMac, eSIR_MAC_CAPABILITIES_NOT_SUPPORTED_STATUS, 
                                  1, pHdr->sa, subType, 0, psessionEntry );
        limLog(pMac, LOGE, FL("SOFTAP was in 11N only mode, rejecting legacy STA's\n"));
        goto error;
    }//                          

#endif

    /*                                           */
    if (localCapabilities.spectrumMgt)
    {
        tSirRetStatus status = eSIR_SUCCESS;

        /*                                                              
                                                         
         */
        if (pAssocReq->capabilityInfo.spectrumMgt)
        {
            if (!((pAssocReq->powerCapabilityPresent) && (pAssocReq->supportedChannelsPresent)))
            {
                /*                                                                            */
                if (!pAssocReq->powerCapabilityPresent)
                {
                    if(subType == LIM_ASSOC)
                       limLog(pMac, LOG1, FL("LIM Info: Missing Power capability IE in assoc request\n"));
                    else
                       limLog(pMac, LOG1, FL("LIM Info: Missing Power capability IE in Reassoc request\n"));
                }
                if (!pAssocReq->supportedChannelsPresent)
                {
                    if(subType == LIM_ASSOC)
                        limLog(pMac, LOG1, FL("LIM Info: Missing Supported channel IE in assoc request\n"));
                    else
                        limLog(pMac, LOG1, FL("LIM Info: Missing Supported channel IE in Reassoc request\n"));
                }
                limPrintMacAddr(pMac, pHdr->sa, LOG1);
            }
            else
            {
                /*                                    */
                status = limIsDot11hPowerCapabilitiesInRange(pMac, pAssocReq, psessionEntry);
                if (eSIR_SUCCESS != status)
                {
                    if (subType == LIM_ASSOC)
                        limLog(pMac, LOGW, FL("LIM Info: Association MinTxPower(STA) > MaxTxPower(AP)\n"));
                    else
                        limLog(pMac, LOGW, FL("LIM Info: Reassociation MinTxPower(STA) > MaxTxPower(AP)\n"));
                    limPrintMacAddr(pMac, pHdr->sa, LOGW);
                }
                status = limIsDot11hSupportedChannelsValid(pMac, pAssocReq);
                if (eSIR_SUCCESS != status)
                {
                    if (subType == LIM_ASSOC)
                        limLog(pMac, LOGW, FL("LIM Info: Association wrong supported channels (STA)\n"));
                    else
                        limLog(pMac, LOGW, FL("LIM Info: Reassociation wrong supported channels (STA)\n"));
                    limPrintMacAddr(pMac, pHdr->sa, LOGW);
                }
                /*                                   */
            }
        } //                                    
        else
        {
            /*                                                                              
                                                                                             
                                                                                        
             */

            /*                                                                                 
                                                                                             
             */
        }
    }//                                              

    if ( (pAssocReq->HTCaps.present) && (limCheckMCSSet(pMac, pAssocReq->HTCaps.supportedMCSSet) == false))
    {
        /* 
                                                                            
                                                                  
         */
        limSendAssocRspMgmtFrame(
                    pMac,
                    eSIR_MAC_OUTSIDE_SCOPE_OF_SPEC_STATUS,
                    1,
                    pHdr->sa,
                    subType, 0,psessionEntry);

        //          
        if (subType == LIM_ASSOC)
            limLog(pMac, LOGW,
               FL("received Assoc req with unsupported MCS Rate Set from \n"));
        else
            limLog(pMac, LOGW,
               FL("received ReAssoc req with unsupported MCS Rate Set from\n"));
        limPrintMacAddr(pMac, pHdr->sa, LOGW);
        goto error;
    }

    //                                                  
    if (phyMode == WNI_CFG_PHY_MODE_11G)
    {

        if (wlan_cfgGetInt(pMac, WNI_CFG_11G_ONLY_POLICY, &val) != eSIR_SUCCESS)
        {
            limLog(pMac, LOGP, FL("could not retrieve 11g-only flag\n"));
            goto error;
        }

        if (!pAssocReq->extendedRatesPresent && val)
        {
            /* 
                                                   
                                                  
                      
                                                   
             */
            limSendAssocRspMgmtFrame(
                           pMac,
                           eSIR_MAC_BASIC_RATES_NOT_SUPPORTED_STATUS,
                           1,
                           pHdr->sa,
                           subType, 0,psessionEntry);

            limLog(pMac, LOGW, FL("Rejecting Re/Assoc req from 11b STA: "));
            limPrintMacAddr(pMac, pHdr->sa, LOGW);
            
#ifdef WLAN_DEBUG    
            pMac->lim.gLim11bStaAssocRejectCount++;
#endif
            goto error;
        }
    }

#ifdef WMM_APSD
    //                                             
    limGetWmeMode(pMac, &wmeMode);
    if (wmeMode == eHAL_SET)
    {
        tpQosInfoSta qInfo;

        qInfo = (tpQosInfoSta) (pAssocReq->qosCapability.qosInfo);

        if ((pMac->lim.gWmmApsd.apsdEnable == 0) && (qInfo->ac_be || qInfo->ac_bk || qInfo->ac_vo || qInfo->ac_vi))
        {

            /* 
                                                   
                                                  
                      
                                                   
             */
            limSendAssocRspMgmtFrame(
                           pMac,
                           eSIR_MAC_WME_REFUSED_STATUS,
                           1,
                           pHdr->sa,
                           subType, 0,psessionEntry);

            limLog(pMac, LOGW,
                   FL("Rejecting Re/Assoc req from STA: "));
            limPrintMacAddr(pMac, pHdr->sa, LOGW);
            limLog(pMac, LOGE, FL("APSD not enabled, qosInfo - 0x%x\n"), *qInfo);
            goto error;
        }
    }
#endif

    //                                                             
    //                  
    if ( psessionEntry->htCapability )
    {
        //                                          
        if ( pAssocReq->HTCaps.present )
        {
            //                                                    

            limLog( pMac, LOG1, FL( "AdvCodingCap:%d ChaWidthSet:%d "
                                    "PowerSave:%d greenField:%d "
                                    "shortGI20:%d shortGI40:%d\n"
                                    "txSTBC:%d rxSTBC:%d delayBA:%d"
                                    "maxAMSDUsize:%d DSSS/CCK:%d "
                                    "PSMP:%d stbcCntl:%d lsigTXProt:%d\n"),
                    pAssocReq->HTCaps.advCodingCap,
                    pAssocReq->HTCaps.supportedChannelWidthSet,
                    pAssocReq->HTCaps.mimoPowerSave,
                    pAssocReq->HTCaps.greenField,
                    pAssocReq->HTCaps.shortGI20MHz,
                    pAssocReq->HTCaps.shortGI40MHz,
                    pAssocReq->HTCaps.txSTBC,
                    pAssocReq->HTCaps.rxSTBC,
                    pAssocReq->HTCaps.delayedBA,
                    pAssocReq->HTCaps.maximalAMSDUsize,
                    pAssocReq->HTCaps.dsssCckMode40MHz,
                    pAssocReq->HTCaps.psmp,
                    pAssocReq->HTCaps.stbcControlFrame,
                    pAssocReq->HTCaps.lsigTXOPProtection );

                //                                                      
                //                                     
                //                                                                              
                //                                 

            //                                    
            /*
                                                                                              
             
                                                                       
                                                                   
                                                              
                                     
                                                                                                   
                           
             
            */
        }
    } //                                    

#ifdef WLAN_SOFTAP_FEATURE
    /*                                                                                                        */
    palZeroMemory( pMac->hHdd, ( tANI_U8* )&Dot11fIERSN, sizeof( Dot11fIERSN ) );
    palZeroMemory( pMac->hHdd, ( tANI_U8* )&Dot11fIEWPA, sizeof( Dot11fIEWPA ) );

    /*                                                    */
    if( pAssocReq->addIEPresent && pAssocReq->addIE.length )
        wpsIe = limGetWscIEPtr(pMac, pAssocReq->addIE.addIEdata, pAssocReq->addIE.length);
    /*                                              */
    if( wpsIe == NULL )
    {
        /*                                     */
        if(psessionEntry->limSystemRole == eLIM_AP_ROLE 
            && psessionEntry->pLimStartBssReq->privacy 
            && psessionEntry->pLimStartBssReq->rsnIE.length)
        {
            limLog(pMac, LOGE,
                   FL("AP supports RSN enabled authentication\n"));

            if(pAssocReq->rsnPresent)
            {
                if(pAssocReq->rsn.length)
                {
                    //                   
                    dot11fUnpackIeRSN(pMac, 
                                        &pAssocReq->rsn.info[0], 
                                        pAssocReq->rsn.length, 
                                        &Dot11fIERSN);

                    /*                                       */
                    if(SIR_MAC_OUI_VERSION_1 == Dot11fIERSN.version)
                    {
                        /*                                                */
                        if(eSIR_SUCCESS != (status = limCheckRxRSNIeMatch(pMac, Dot11fIERSN, psessionEntry, pAssocReq->HTCaps.present) ) )
                        {
                            /*                              */
                            /*                                                            */
                            limSendAssocRspMgmtFrame(
                                           pMac,
                                           status,
                                           1,
                                           pHdr->sa,
                                           subType, 0,psessionEntry);

                            limLog(pMac, LOGW, FL("Rejecting Re/Assoc req from STA: "));
                            limPrintMacAddr(pMac, pHdr->sa, LOGW);
                            goto error;

                        }
                    }
                    else
                    {
                        /*                                                          */
                        limSendAssocRspMgmtFrame(
                                       pMac,
                                       eSIR_MAC_UNSUPPORTED_RSN_IE_VERSION_STATUS,
                                       1,
                                       pHdr->sa,
                                       subType, 0,psessionEntry);

                        limLog(pMac, LOGW, FL("Rejecting Re/Assoc req from STA: "));
                        limPrintMacAddr(pMac, pHdr->sa, LOGW);
                        goto error;

                    }
                }
                else
                {
                    /*                                                            */
                    limSendAssocRspMgmtFrame(
                                   pMac,
                                   eSIR_MAC_INVALID_INFORMATION_ELEMENT_STATUS,
                                   1,
                                   pHdr->sa,
                                   subType, 0,psessionEntry);

                    limLog(pMac, LOGW, FL("Rejecting Re/Assoc req from STA: "));
                    limPrintMacAddr(pMac, pHdr->sa, LOGW);
                    goto error;
                    
                }
            } /*                                 */
            if((!pAssocReq->rsnPresent) && pAssocReq->wpaPresent)
            {
                //                   
                if(pAssocReq->wpa.length)
                {
                    dot11fUnpackIeWPA(pMac, 
                                        &pAssocReq->wpa.info[4], //                     
                                        pAssocReq->wpa.length, 
                                        &Dot11fIEWPA);
                    /*                                                */
                    if(eSIR_SUCCESS != (status = limCheckRxWPAIeMatch(pMac, Dot11fIEWPA, psessionEntry, pAssocReq->HTCaps.present)))
                    {
                        /*                                                         */
                        limSendAssocRspMgmtFrame(
                                       pMac,
                                       status,
                                       1,
                                       pHdr->sa,
                                       subType, 0,psessionEntry);

                        limLog(pMac, LOGW, FL("Rejecting Re/Assoc req from STA: "));
                        limPrintMacAddr(pMac, pHdr->sa, LOGW);
                        goto error;

                    }
                }
                else
                {
                    /*                                                    */
                    limSendAssocRspMgmtFrame(
                                   pMac,
                                   eSIR_MAC_INVALID_INFORMATION_ELEMENT_STATUS,
                                   1,
                                   pHdr->sa,
                                   subType, 0,psessionEntry);

                    limLog(pMac, LOGW, FL("Rejecting Re/Assoc req from STA: "));
                    limPrintMacAddr(pMac, pHdr->sa, LOGW);
                    goto error;
                }/*                                 */
            } /*                                 */
        } /*                                                   
                                                              */

    } /*                                               */
#endif //                   

    /* 
                                                    
                                                    
     */
    pStaDs = dphLookupHashEntry(pMac, pHdr->sa, &aid, &psessionEntry->dph.dphHashTable);

    //                                               
    pStaPreAuthContext = limSearchPreAuthList(pMac, pHdr->sa);

    if (pStaDs == NULL)
    {
        //                                            
        if (pMac->lim.gLimNumOfCurrentSTAs == pMac->lim.maxStation)
        {
            /* 
                                                                 
                                                           
             */
            limRejectAssociation(pMac, pHdr->sa,
                                 subType, false,
                                 (tAniAuthType) 0, 0,
                                 false,
                                 (tSirResultCodes) eSIR_MAC_UNSPEC_FAILURE_STATUS, psessionEntry);

            goto error;
        }

        //                                    
        if ((pStaPreAuthContext == NULL) ||
            (pStaPreAuthContext &&
             (pStaPreAuthContext->mlmState !=
                              eLIM_MLM_AUTHENTICATED_STATE)))
        {
            /* 
                                                          
                                                    
                                                                
                                               
                                                      
                                                    
             */
            limSendDeauthMgmtFrame(
                     pMac,
                     eSIR_MAC_STA_NOT_PRE_AUTHENTICATED_REASON, //  
                     pHdr->sa,psessionEntry);

            //          
            if (subType == LIM_ASSOC)
                limLog(pMac, LOGW,
                       FL("received Assoc req from STA that does not have pre-auth context "MAC_ADDRESS_STR),
                       MAC_ADDR_ARRAY(pHdr->sa));
            else
                limLog(pMac, LOGW,
                       FL("received ReAssoc req from STA that does not have pre-auth context "
                       MAC_ADDRESS_STR), MAC_ADDR_ARRAY(pHdr->sa));
            goto error;
        }

        //                                  
        authType = pStaPreAuthContext->authType;
        limDeletePreAuthNode(pMac, pHdr->sa);

        //                                          

    } //                    
    else
    {
        //                                    

        if (pStaDs->mlmStaContext.mlmState != eLIM_MLM_LINK_ESTABLISHED_STATE)
        {
            /* 
                                                           
                                                            
                                             
             */
            if (subType == LIM_ASSOC)
            {
            
#ifdef WLAN_DEBUG    
                pMac->lim.gLimNumAssocReqDropInvldState++;
#endif
                limLog(pMac, LOG1, FL("received Assoc req in state %X from "), pStaDs->mlmStaContext.mlmState);
            }
            else
            {     
#ifdef WLAN_DEBUG    
                pMac->lim.gLimNumReassocReqDropInvldState++;
#endif
                limLog(pMac, LOG1, FL("received ReAssoc req in state %X from "), pStaDs->mlmStaContext.mlmState);
            }
            limPrintMacAddr(pMac, pHdr->sa, LOG1);
            limPrintMlmState(pMac, LOG1, (tLimMlmStates) pStaDs->mlmStaContext.mlmState);

            goto error;
        } //                                                                       

        /* 
                                                                 
                                                          
                                                        
         */

        pStaDs->mlmStaContext.capabilityInfo = pAssocReq->capabilityInfo;

        if (pStaPreAuthContext &&
            (pStaPreAuthContext->mlmState ==
                                       eLIM_MLM_AUTHENTICATED_STATE))
        {
            //                                  
            authType = pStaPreAuthContext->authType;
            limDeletePreAuthNode(pMac, pHdr->sa);
        }
        else
            authType = pStaDs->mlmStaContext.authType;

        updateContext = true;

        if (dphInitStaState(pMac, pHdr->sa, aid, true, &psessionEntry->dph.dphHashTable) == NULL)   
        {
            limLog(pMac, LOGE, FL("could not Init STAid=%d\n"), aid);
            goto  error;
        }


        goto sendIndToSme;
    } //                                          



    //                                         
    //                                                         
    limGetWmeMode(psessionEntry, &wmeMode);
    if ((qosMode == eHAL_SET) || (wmeMode == eHAL_SET))
    {
        //                                                
        //              
        //                                                
        if (pAssocReq->addtsPresent)
        {
            tANI_U8 tspecIdx = 0; //                             
            if (limAdmitControlAddTS(pMac, pHdr->sa, &(pAssocReq->addtsReq),
                                     &(pAssocReq->qosCapability), 0, false, NULL, &tspecIdx, psessionEntry) != eSIR_SUCCESS)
            {
                limLog(pMac, LOGW, FL("AdmitControl: TSPEC rejected\n"));
                limSendAssocRspMgmtFrame(
                               pMac,
                               eSIR_MAC_QAP_NO_BANDWIDTH_REASON,
                               1,
                               pHdr->sa,
                               subType, 0,psessionEntry);
#ifdef WLAN_DEBUG    
                pMac->lim.gLimNumAssocReqDropACRejectTS++;
#endif
                goto error;
            }
        }
        else if (limAdmitControlAddSta(pMac, pHdr->sa, false)
                                               != eSIR_SUCCESS)
        {
            limLog(pMac, LOGW, FL("AdmitControl: Sta rejected\n"));
            limSendAssocRspMgmtFrame(
                    pMac,
                    eSIR_MAC_QAP_NO_BANDWIDTH_REASON,
                    1,
                    pHdr->sa,
                    subType, 0,psessionEntry);
#ifdef WLAN_DEBUG    
            pMac->lim.gLimNumAssocReqDropACRejectSta++;
#endif
            goto error;
        }

        //            
        limLog(pMac, LOG1, FL("AdmitControl: Sta OK!\n"));
    }

    /* 
                          
     */
    if (subType == LIM_ASSOC)
        limLog(pMac, LOGW, FL("received Assoc req successful "MAC_ADDRESS_STR),
               MAC_ADDR_ARRAY(pHdr->sa));
    else
        limLog(pMac, LOGW, FL("received ReAssoc req successful"MAC_ADDRESS_STR),
               MAC_ADDR_ARRAY(pHdr->sa));

    /* 
                                                           
                                             
                                                                      
     */

    aid = limAssignAID(pMac);

    if (!aid)
    {
        //                     
        //                   
        limRejectAssociation(pMac, pHdr->sa,
                             subType, true, authType,
                             aid, false,
                             (tSirResultCodes) eSIR_MAC_UNSPEC_FAILURE_STATUS, psessionEntry);

        goto error;
    }

    /* 
                                                          
     */

    pStaDs = dphAddHashEntry(pMac, pHdr->sa, aid, &psessionEntry->dph.dphHashTable);

    if (pStaDs == NULL)
    {
        //                                      
        limLog(pMac, LOGE,
           FL("could not add hash entry at DPH for aid=%d, MacAddr:\n"),
           aid);
        limPrintMacAddr(pMac, pHdr->sa, LOGE);

        //            
        limReleaseAID(pMac, aid);

        limRejectAssociation(pMac, pHdr->sa,
                             subType, true, authType, aid, false,
                             (tSirResultCodes) eSIR_MAC_UNSPEC_FAILURE_STATUS, psessionEntry);

        goto error;
    }


sendIndToSme:

    psessionEntry->parsedAssocReq[pStaDs->assocId] = pAssocReq;

    pStaDs->mlmStaContext.htCapability = pAssocReq->HTCaps.present;
#ifdef WLAN_FEATURE_11AC
    pStaDs->mlmStaContext.vhtCapability = pAssocReq->VHTCaps.present;
#endif
    pStaDs->qos.addtsPresent = (pAssocReq->addtsPresent==0) ? false : true;
    pStaDs->qos.addts        = pAssocReq->addtsReq;
    pStaDs->qos.capability   = pAssocReq->qosCapability;
    pStaDs->versionPresent   = 0;
    /*                                                                        */
    pStaDs->shortPreambleEnabled = (tANI_U8)pAssocReq->capabilityInfo.shortPreamble;
    pStaDs->shortSlotTimeEnabled = (tANI_U8)pAssocReq->capabilityInfo.shortSlotTime;
 
    if (pAssocReq->propIEinfo.versionPresent) //                       
    {
        pStaDs->versionPresent = 1;
        pStaDs->version = pAssocReq->propIEinfo.version;
    }
    pStaDs->propCapability = 0;
    if (pAssocReq->propIEinfo.capabilityPresent)
    {
        if (sirGetCfgPropCaps(pMac, &pStaDs->propCapability))
            pStaDs->propCapability &= pAssocReq->propIEinfo.capability;
    }

    pStaDs->mlmStaContext.mlmState = eLIM_MLM_WT_ASSOC_CNF_STATE;
    pStaDs->valid                  = 0;
    pStaDs->mlmStaContext.authType = authType;
    pStaDs->staType = STA_ENTRY_PEER;

    //                                                                            
    //                                                       
    pStaDs->mlmStaContext.listenInterval = pAssocReq->listenInterval;
    pStaDs->mlmStaContext.capabilityInfo = pAssocReq->capabilityInfo;

    /*                                                                        
                                                                                     
                                                                                      
                                                             
     */

    /*                                                                     */
    pStaDs->timWaitCount = (tANI_U8)GET_TIM_WAIT_COUNT(pAssocReq->listenInterval);
    
    /*                                                                            */
    pStaDs->curTxMpduCnt = 0;

    if(IS_DOT11_MODE_HT(psessionEntry->dot11mode) &&
      (pAssocReq->HTCaps.present))
    {
        pStaDs->htGreenfield = (tANI_U8)pAssocReq->HTCaps.greenField;
        pStaDs->htAMpduDensity = pAssocReq->HTCaps.mpduDensity;
        pStaDs->htDsssCckRate40MHzSupport = (tANI_U8)pAssocReq->HTCaps.dsssCckMode40MHz;
        pStaDs->htLsigTXOPProtection = (tANI_U8)pAssocReq->HTCaps.lsigTXOPProtection;
        pStaDs->htMaxAmsduLength = (tANI_U8)pAssocReq->HTCaps.maximalAMSDUsize;
        pStaDs->htMaxRxAMpduFactor = pAssocReq->HTCaps.maxRxAMPDUFactor;
        pStaDs->htMIMOPSState = pAssocReq->HTCaps.mimoPowerSave;
        pStaDs->htShortGI20Mhz = (tANI_U8)pAssocReq->HTCaps.shortGI20MHz;
        pStaDs->htShortGI40Mhz = (tANI_U8)pAssocReq->HTCaps.shortGI40MHz;
        pStaDs->htSupportedChannelWidthSet = (tANI_U8)pAssocReq->HTCaps.supportedChannelWidthSet;
        /*                                                                                                                                  
                                                                                                         
         */
        pStaDs->htSecondaryChannelOffset = (pStaDs->htSupportedChannelWidthSet)?psessionEntry->htSecondaryChannelOffset:0;
#ifdef WLAN_FEATURE_11AC
        if (pAssocReq->VHTCaps.present)
        {
            pStaDs->vhtSupportedChannelWidthSet = (tANI_U8)pAssocReq->VHTCaps.supportedChannelWidthSet; 
        }
#endif
        pStaDs->baPolicyFlag = 0xFF;
    }


#ifdef WLAN_FEATURE_11AC
if (limPopulateMatchingRateSet(pMac,
                               pStaDs,
                               &(pAssocReq->supportedRates),
                               &(pAssocReq->extendedRates),
                               pAssocReq->HTCaps.supportedMCSSet,
                               &(pAssocReq->propIEinfo.propRates),
                               psessionEntry , &pAssocReq->VHTCaps) 
                               != eSIR_SUCCESS)
#else

    if (limPopulateMatchingRateSet(pMac,
                                   pStaDs,
                                   &(pAssocReq->supportedRates),
                                   &(pAssocReq->extendedRates),
                                   pAssocReq->HTCaps.supportedMCSSet,
                                   &(pAssocReq->propIEinfo.propRates), psessionEntry) != eSIR_SUCCESS)
#endif
    {
        //                                                      
        limLog(pMac, LOGE,
           FL("could not update hash entry at DPH for aid=%d, MacAddr:\n"),
           aid);
        limPrintMacAddr(pMac, pHdr->sa, LOGE);

                //            
        limReleaseAID(pMac, aid);


        limRejectAssociation(pMac, pHdr->sa,
                             subType, true, authType, aid, true,
                             (tSirResultCodes) eSIR_MAC_UNSPEC_FAILURE_STATUS, psessionEntry);

        /*                                                                                                     */
        return;
        //           
    }

    palCopyMemory( pMac->hHdd, (tANI_U8 *) &pStaDs->mlmStaContext.propRateSet,
                  (tANI_U8 *) &(pAssocReq->propIEinfo.propRates),
                  pAssocReq->propIEinfo.propRates.numPropRates + 1);

    //                                            

    pStaDs->qosMode    = eANI_BOOLEAN_FALSE;
    pStaDs->lleEnabled = eANI_BOOLEAN_FALSE;
    if (pAssocReq->capabilityInfo.qos && (qosMode == eHAL_SET))
    {
        pStaDs->lleEnabled = eANI_BOOLEAN_TRUE;
        pStaDs->qosMode    = eANI_BOOLEAN_TRUE; 
    }

    pStaDs->wmeEnabled = eANI_BOOLEAN_FALSE;
    pStaDs->wsmEnabled = eANI_BOOLEAN_FALSE;
    limGetWmeMode(psessionEntry, &wmeMode);
    //                                                                               
    if ((! pStaDs->lleEnabled) && pAssocReq->wmeInfoPresent && (wmeMode == eHAL_SET))
    {
        pStaDs->wmeEnabled = eANI_BOOLEAN_TRUE;
        pStaDs->qosMode = eANI_BOOLEAN_TRUE;
        limGetWsmMode(psessionEntry, &wsmMode);
        /*                                                                             
                     */
#ifdef WLAN_SOFTAP_FEATURE
        if( pAssocReq->WMMInfoStation.present)
        {
            /*                                  */
            if ((psessionEntry->limSystemRole == eLIM_AP_ROLE)
                 && (psessionEntry->apUapsdEnable == 0) && (pAssocReq->WMMInfoStation.acbe_uapsd 
                    || pAssocReq->WMMInfoStation.acbk_uapsd 
                    || pAssocReq->WMMInfoStation.acvo_uapsd 
                    || pAssocReq->WMMInfoStation.acvi_uapsd))
            {

                /* 
                                                       
                                                   
                 */
                limLog( pMac, LOGE, FL( "AP do not support UPASD REASSOC Failed\n" ));
                limRejectAssociation(pMac, pHdr->sa,
                                     subType, true, authType, aid, true,
                                     (tSirResultCodes) eSIR_MAC_WME_REFUSED_STATUS, psessionEntry);


                /*                                                                                                                            */
                return;
                //           
            }
            else
            {
                /*                                            */
                pStaDs->qos.capability.qosInfo.acbe_uapsd = pAssocReq->WMMInfoStation.acbe_uapsd;
                pStaDs->qos.capability.qosInfo.acbk_uapsd = pAssocReq->WMMInfoStation.acbk_uapsd;
                pStaDs->qos.capability.qosInfo.acvo_uapsd = pAssocReq->WMMInfoStation.acvo_uapsd;
                pStaDs->qos.capability.qosInfo.acvi_uapsd = pAssocReq->WMMInfoStation.acvi_uapsd;
                pStaDs->qos.capability.qosInfo.maxSpLen = pAssocReq->WMMInfoStation.max_sp_length;
            }
        }
#endif
        //                                                        
        if (pAssocReq->wsmCapablePresent && (wsmMode == eHAL_SET))
            pStaDs->wsmEnabled = eANI_BOOLEAN_TRUE;

    }

    //                                          
    pStaDs->mlmStaContext.subType = subType;

    if (pAssocReq->propIEinfo.aniIndicator)
        pStaDs->aniPeer = 1;

    //                                                                   
    psessionEntry->parsedAssocReq[pStaDs->assocId] = pAssocReq;

    /*                                                              
                                                                
                                                                        
      
                                                             
                                                                
                                                               
                                                              
                                              
     */
    if (!updateContext)
    {
        pStaDs->mlmStaContext.updateContext = 0;

        //                                                            
        if (limAddSta(pMac, pStaDs,psessionEntry) != eSIR_SUCCESS)
        {
            limLog(pMac, LOGE, FL("could not Add STA with assocId=%d\n"), pStaDs->assocId);
            limRejectAssociation( pMac, pStaDs->staAddr, pStaDs->mlmStaContext.subType,
                                  true, pStaDs->mlmStaContext.authType, pStaDs->assocId, true,
                                  (tSirResultCodes) eSIR_MAC_UNSPEC_FAILURE_STATUS, psessionEntry);

            /*                                                                                                     */
            return;
            //           
        }
    }
    else
    {
        pStaDs->mlmStaContext.updateContext = 1;

#ifdef WLAN_SOFTAP_FEATURE
        mlmPrevState = pStaDs->mlmStaContext.mlmState;

        /*                                                                       */
        if(subType != LIM_REASSOC)
        {
            //                                                                     
            pStaDs->mlmStaContext.mlmState = eLIM_MLM_WT_ASSOC_DEL_STA_RSP_STATE;
            if(limDelSta(pMac, pStaDs, true, psessionEntry) != eSIR_SUCCESS)
            {
                limLog(pMac, LOGE, FL("could not DEL STA with assocId=%d staId %d\n"), pStaDs->assocId, pStaDs->staIndex);
                limRejectAssociation( pMac, pStaDs->staAddr, pStaDs->mlmStaContext.subType, true, pStaDs->mlmStaContext.authType,
                                      pStaDs->assocId, true,(tSirResultCodes) eSIR_MAC_UNSPEC_FAILURE_STATUS, psessionEntry);

                //                         
                pStaDs->mlmStaContext.mlmState = mlmPrevState;
                goto error;
            }
        }
        else
        {
            /*                                          */
            /*                                     */
            if (limAddSta(pMac, pStaDs,psessionEntry) != eSIR_SUCCESS)
            {
                    limLog( pMac, LOGE, FL( "AP do not support UPASD REASSOC Failed\n" ));
                    limRejectAssociation( pMac, pStaDs->staAddr, pStaDs->mlmStaContext.subType, true, pStaDs->mlmStaContext.authType,
                                          pStaDs->assocId, true,(tSirResultCodes) eSIR_MAC_WME_REFUSED_STATUS, psessionEntry);

                    //                         
                    pStaDs->mlmStaContext.mlmState = mlmPrevState;
                    goto error;
            }

        }
#endif

    }

    return;

error:
    if (pAssocReq != NULL)
    {
        if ( pAssocReq->assocReqFrame ) 
        {
            palFreeMemory(pMac->hHdd, pAssocReq->assocReqFrame);
            pAssocReq->assocReqFrame = NULL;
        }

        if (palFreeMemory(pMac->hHdd, pAssocReq) != eHAL_STATUS_SUCCESS) 
        {
            limLog(pMac, LOGP, FL("PalFree Memory failed \n"));
            return;
        }
    }

    /*                                                             */
    if ((pStaDs != NULL) &&
          (pStaDs->mlmStaContext.mlmState != eLIM_MLM_WT_ADD_STA_RSP_STATE))
        psessionEntry->parsedAssocReq[pStaDs->assocId] = NULL;

    return;

} /*                                   */



/*                                                                
                          
                                                      
                                      
 
            
                                        
                                        
            

                       
                                                                   
                                      
                                      
              
                                               
            
                                                                  */
void limSendMlmAssocInd(tpAniSirGlobal pMac, tpDphHashNode pStaDs, tpPESession psessionEntry)
{
    tpLimMlmAssocInd        pMlmAssocInd = NULL;
    tpLimMlmReassocInd      pMlmReassocInd;
    tpSirAssocReq           pAssocReq;
    tANI_U16                temp;
    tANI_U32                phyMode;
    tANI_U8                 subType;
#ifdef WLAN_SOFTAP_FEATURE
    tANI_U8                 *wpsIe = NULL;
#endif
    tANI_U32                tmp;
//                                           
    tANI_U16                i, j=0;

    //                                               
    pAssocReq = (tpSirAssocReq) psessionEntry->parsedAssocReq[pStaDs->assocId];

    //                
    limGetPhyMode(pMac, &phyMode, psessionEntry);
 
    //                                                         
 
    //                                           
    if (pAssocReq->reassocRequest == 1)
        subType = LIM_REASSOC;
    else 
        subType = LIM_ASSOC;
#ifdef WLAN_SOFTAP_FEATURE
    if (subType == LIM_ASSOC || subType == LIM_REASSOC)
#else
    if (subType == LIM_ASSOC )
#endif
    {
        temp  = sizeof(tLimMlmAssocInd);
#if (WNI_POLARIS_FW_PACKAGE == ADVANCED) && defined(ANI_PRODUCT_TYPE_AP)        
        temp += pAssocReq->propIEinfo.numBss * sizeof(tSirNeighborBssInfo);
#endif        

        if( eHAL_STATUS_SUCCESS != palAllocateMemory( pMac->hHdd, (void **)&pMlmAssocInd, temp))
        {
            limReleaseAID(pMac, pStaDs->assocId);
            limLog(pMac, LOGP, FL("palAllocateMemory failed for pMlmAssocInd\n"));
            return;
        }
        palZeroMemory( pMac->hHdd, pMlmAssocInd, temp);

        palCopyMemory( pMac->hHdd,(tANI_U8 *)pMlmAssocInd->peerMacAddr,(tANI_U8 *)pStaDs->staAddr,sizeof(tSirMacAddr));
 
        pMlmAssocInd->aid    = pStaDs->assocId;
        palCopyMemory( pMac->hHdd, (tANI_U8 *)&pMlmAssocInd->ssId,(tANI_U8 *)&(pAssocReq->ssId), pAssocReq->ssId.length + 1);
        pMlmAssocInd->sessionId = psessionEntry->peSessionId;
        pMlmAssocInd->authType =  pStaDs->mlmStaContext.authType;
 
#if (WNI_POLARIS_FW_PACKAGE == ADVANCED) && defined(ANI_PRODUCT_TYPE_AP)
        //                                                        
        pMlmAssocInd->wniIndicator = (tAniBool) pAssocReq->propIEinfo.aniIndicator;
        pMlmAssocInd->bpIndicator  = (tAniBool) pAssocReq->propIEinfo.bpIndicator;
        pMlmAssocInd->bpType       = (tSirBpIndicatorType) pAssocReq->propIEinfo.bpType;
        if (pAssocReq->extendedRatesPresent)
        {
            pMlmAssocInd->nwType = eSIR_11G_NW_TYPE;
            limSetStaHashErpMode(pMac, pStaDs->assocId, eHAL_SET);
        }
        else
        {
            if (phyMode == WNI_CFG_PHY_MODE_11A)
                pMlmAssocInd->nwType = eSIR_11A_NW_TYPE;
            else
            {
                pMlmAssocInd->nwType = eSIR_11B_NW_TYPE;
                limSetStaHashErpMode(pMac, pStaDs->assocId, eHAL_CLEAR);
            }
        }
        pMlmAssocInd->assocType = (tSirAssocType)pAssocReq->propIEinfo.assocType;
        pMlmAssocInd->load.numStas = pMac->lim.gLimNumOfCurrentSTAs;
        pMlmAssocInd->load.channelUtilization =(pMac->lim.gpLimMeasData) ? pMac->lim.gpLimMeasData->avgChannelUtilization : 0;
        pMlmAssocInd->numBss = (tANI_U32) pAssocReq->propIEinfo.numBss;
        if (pAssocReq->propIEinfo.numBss)
        {
            palCopyMemory( pMac->hHdd,(tANI_U8 *) pMlmAssocInd->neighborList,(tANI_U8 *)pAssocReq->propIEinfo.pBssList,
                           (sizeof(tSirNeighborBssInfo) * pAssocReq->propIEinfo.numBss));
        } 
#endif
        pMlmAssocInd->capabilityInfo = pAssocReq->capabilityInfo;

        //                           
        pMlmAssocInd->rsnIE.length = 0;
#ifdef WLAN_SOFTAP_FEATURE
        //                                    
        if (pAssocReq->addIEPresent && pAssocReq->addIE.length ) {
            wpsIe = limGetWscIEPtr(pMac, pAssocReq->addIE.addIEdata, pAssocReq->addIE.length);
        }
        if (pAssocReq->rsnPresent && (NULL == wpsIe))
#else
        if (pAssocReq->rsnPresent)
#endif
        {
            limLog(pMac, LOG2, FL("Assoc Req RSN IE len = %d\n"), pAssocReq->rsn.length);
            pMlmAssocInd->rsnIE.length = 2 + pAssocReq->rsn.length;
            pMlmAssocInd->rsnIE.rsnIEdata[0] = SIR_MAC_RSN_EID;
            pMlmAssocInd->rsnIE.rsnIEdata[1] = pAssocReq->rsn.length;
            palCopyMemory( pMac->hHdd, 
                           &pMlmAssocInd->rsnIE.rsnIEdata[2],
                           pAssocReq->rsn.info,
                           pAssocReq->rsn.length);
        }

        //                             
        if (pAssocReq->powerCapabilityPresent && pAssocReq->supportedChannelsPresent)
        {
            pMlmAssocInd->spectrumMgtIndicator = eSIR_TRUE;
            pMlmAssocInd->powerCap.minTxPower = pAssocReq->powerCapability.minTxPower;
            pMlmAssocInd->powerCap.maxTxPower = pAssocReq->powerCapability.maxTxPower;
            limConvertSupportedChannels(pMac, pMlmAssocInd, pAssocReq);
        }
        else
            pMlmAssocInd->spectrumMgtIndicator = eSIR_FALSE;


#ifdef WLAN_SOFTAP_FEATURE
        /*                                                            */
        if (pAssocReq->wpaPresent && (NULL == wpsIe))
#else
        if ((pAssocReq->wpaPresent) && (pMlmAssocInd->rsnIE.length < SIR_MAC_MAX_IE_LENGTH))
#endif
        {
            if((pMlmAssocInd->rsnIE.length + pAssocReq->wpa.length) >= SIR_MAC_MAX_IE_LENGTH)
            {
                PELOGE(limLog(pMac, LOGE, FL("rsnIEdata index out of bounds %d\n"), pMlmAssocInd->rsnIE.length);)
                return;
            }
            pMlmAssocInd->rsnIE.rsnIEdata[pMlmAssocInd->rsnIE.length] = SIR_MAC_WPA_EID;
            pMlmAssocInd->rsnIE.rsnIEdata[pMlmAssocInd->rsnIE.length + 1] = pAssocReq->wpa.length;
            palCopyMemory( pMac->hHdd,
                           &pMlmAssocInd->rsnIE.rsnIEdata[pMlmAssocInd->rsnIE.length + 2],
                           pAssocReq->wpa.info,
                           pAssocReq->wpa.length);
            pMlmAssocInd->rsnIE.length += 2 + pAssocReq->wpa.length;
        }


       pMlmAssocInd->addIE.length = 0;
       if (pAssocReq->addIEPresent)
       {
            palCopyMemory( pMac->hHdd,
                           &pMlmAssocInd->addIE.addIEdata,
                           pAssocReq->addIE.addIEdata,
                           pAssocReq->addIE.length);

            pMlmAssocInd->addIE.length = pAssocReq->addIE.length;
       }

#ifdef WLAN_SOFTAP_FEATURE
        if(pAssocReq->wmeInfoPresent)
        {

            if (wlan_cfgGetInt(pMac, (tANI_U16) WNI_CFG_WME_ENABLED, &tmp) != eSIR_SUCCESS)
                 limLog(pMac, LOGP, FL("wlan_cfgGetInt failed for id %d\n"), WNI_CFG_WME_ENABLED );

            /*                                      */
            if(tmp)
            {
                pMlmAssocInd->WmmStaInfoPresent = 1;
            }
            else
            {
                pMlmAssocInd->WmmStaInfoPresent= 0;
            }
            /*                                                                   */

        }
#endif

        //                                                  
        pMlmAssocInd->assocReqLength = pAssocReq->assocReqFrameLength;
        pMlmAssocInd->assocReqPtr = pAssocReq->assocReqFrame;
        
        pMlmAssocInd->beaconPtr = psessionEntry->beacon;
        pMlmAssocInd->beaconLength = psessionEntry->bcnLen;

        limPostSmeMessage(pMac, LIM_MLM_ASSOC_IND, (tANI_U32 *) pMlmAssocInd);
        palFreeMemory( pMac->hHdd, pMlmAssocInd);
    }
    else
    {
        //                                                                
        temp  = sizeof(tLimMlmReassocInd);

#if (WNI_POLARIS_FW_PACKAGE == ADVANCED)
        temp += pAssocReq->propIEinfo.numBss * sizeof(tSirNeighborBssInfo);
#endif
        if( eHAL_STATUS_SUCCESS != palAllocateMemory( pMac->hHdd, (void **)&pMlmReassocInd, temp))
        {
            limLog(pMac, LOGP, FL("call to palAllocateMemory failed for pMlmReassocInd\n"));
            limReleaseAID(pMac, pStaDs->assocId);
            return;
        }
        palZeroMemory( pMac->hHdd, pMlmReassocInd, temp);

        palCopyMemory( pMac->hHdd,(tANI_U8 *) pMlmReassocInd->peerMacAddr, (tANI_U8 *)pStaDs->staAddr, sizeof(tSirMacAddr));
        palCopyMemory( pMac->hHdd,(tANI_U8 *) pMlmReassocInd->currentApAddr, (tANI_U8 *)&(pAssocReq->currentApAddr), sizeof(tSirMacAddr));
        pMlmReassocInd->aid = pStaDs->assocId;
        pMlmReassocInd->authType = pStaDs->mlmStaContext.authType;
        palCopyMemory( pMac->hHdd,(tANI_U8 *)&pMlmReassocInd->ssId, (tANI_U8 *)&(pAssocReq->ssId), pAssocReq->ssId.length + 1);

#if (WNI_POLARIS_FW_PACKAGE == ADVANCED) && defined(ANI_PRODUCT_TYPE_AP)
        //                                                        
        pMlmReassocInd->wniIndicator = (tAniBool) pAssocReq->propIEinfo.aniIndicator;
        pMlmReassocInd->bpIndicator  = (tAniBool) pAssocReq->propIEinfo.bpIndicator;
        pMlmReassocInd->bpType       = (tSirBpIndicatorType) pAssocReq->propIEinfo.bpType;
        if (pAssocReq->extendedRatesPresent)
        {
            pMlmReassocInd->nwType = eSIR_11G_NW_TYPE;
            limSetStaHashErpMode(pMac, pStaDs->assocId, eHAL_SET);
        }
        else
        {
            if (phyMode == WNI_CFG_PHY_MODE_11A)
                pMlmReassocInd->nwType = eSIR_11A_NW_TYPE;
            else
            {
                pMlmReassocInd->nwType = eSIR_11B_NW_TYPE;
                limSetStaHashErpMode(pMac, pStaDs->assocId, eHAL_CLEAR);
            }
        }

        pMlmReassocInd->reassocType  = (tSirAssocType)pAssocReq->propIEinfo.assocType;
        pMlmReassocInd->load.numStas = pMac->lim.gLimNumOfCurrentSTAs;
        pMlmReassocInd->load.channelUtilization = (pMac->lim.gpLimMeasData) ?
                                                  pMac->lim.gpLimMeasData->avgChannelUtilization : 0;
        pMlmReassocInd->numBss = (tANI_U32) pAssocReq->propIEinfo.numBss;
        if (pAssocReq->propIEinfo.numBss)
        {
            palCopyMemory( pMac->hHdd, 
                           (tANI_U8 *) pMlmReassocInd->neighborList,
                           (tANI_U8 *) pAssocReq->propIEinfo.pBssList,
                           (sizeof(tSirNeighborBssInfo) * pAssocReq->propIEinfo.numBss));
        }
#endif
        if (pAssocReq->propIEinfo.aniIndicator)
            pStaDs->aniPeer = 1;

        pMlmReassocInd->capabilityInfo = pAssocReq->capabilityInfo;
        pMlmReassocInd->rsnIE.length = 0;

#ifdef WLAN_SOFTAP_FEATURE
        if (pAssocReq->addIEPresent && pAssocReq->addIE.length )
            wpsIe = limGetWscIEPtr(pMac, pAssocReq->addIE.addIEdata, pAssocReq->addIE.length);

        if (pAssocReq->rsnPresent && (NULL == wpsIe))
#else
        if (pAssocReq->rsnPresent)
#endif
        {
            limLog(pMac, LOG2, FL("Assoc Req: RSN IE length = %d\n"), pAssocReq->rsn.length);
            pMlmReassocInd->rsnIE.length = 2 + pAssocReq->rsn.length;
            pMlmReassocInd->rsnIE.rsnIEdata[0] = SIR_MAC_RSN_EID;
            pMlmReassocInd->rsnIE.rsnIEdata[1] = pAssocReq->rsn.length;
            palCopyMemory( pMac->hHdd, &pMlmReassocInd->rsnIE.rsnIEdata[2], pAssocReq->rsn.info, pAssocReq->rsn.length);
        }

        //                
        if (pAssocReq->powerCapabilityPresent && pAssocReq->supportedChannelsPresent)
        {
            pMlmReassocInd->spectrumMgtIndicator = eSIR_TRUE;
            pMlmReassocInd->powerCap.minTxPower = pAssocReq->powerCapability.minTxPower;
            pMlmReassocInd->powerCap.maxTxPower = pAssocReq->powerCapability.maxTxPower;
            pMlmReassocInd->supportedChannels.numChnl = (tANI_U8)(pAssocReq->supportedChannels.length / 2);

            limLog(pMac, LOG1,
                FL("Sending Reassoc Ind: spectrum ON, minPwr %d, maxPwr %d, numChnl %d\n"),
                pMlmReassocInd->powerCap.minTxPower,
                pMlmReassocInd->powerCap.maxTxPower,
                pMlmReassocInd->supportedChannels.numChnl);

            for(i=0; i < pMlmReassocInd->supportedChannels.numChnl; i++)
            {
                pMlmReassocInd->supportedChannels.channelList[i] = pAssocReq->supportedChannels.supportedChannels[j];
                limLog(pMac, LOG1, FL("Sending ReassocInd: chn[%d] = %d \n"),
                       i, pMlmReassocInd->supportedChannels.channelList[i]);
                j+=2;
            }
        }
        else
            pMlmReassocInd->spectrumMgtIndicator = eSIR_FALSE;


#ifdef WLAN_SOFTAP_FEATURE
        /*                                                            */
        if (pAssocReq->wpaPresent && (NULL == wpsIe))
#else
        if (pAssocReq->wpaPresent)
#endif
        {
            limLog(pMac, LOG2, FL("Received WPA IE length in Assoc Req is %d\n"), pAssocReq->wpa.length);
            pMlmReassocInd->rsnIE.rsnIEdata[pMlmReassocInd->rsnIE.length] = SIR_MAC_WPA_EID;
            pMlmReassocInd->rsnIE.rsnIEdata[pMlmReassocInd->rsnIE.length + 1] = pAssocReq->wpa.length;
            palCopyMemory( pMac->hHdd,
                           &pMlmReassocInd->rsnIE.rsnIEdata[pMlmReassocInd->rsnIE.length + 2],
                           pAssocReq->wpa.info,
                           pAssocReq->wpa.length);
            pMlmReassocInd->rsnIE.length += 2 + pAssocReq->wpa.length;
        }

       pMlmReassocInd->addIE.length = 0;
       if (pAssocReq->addIEPresent)
       {
            palCopyMemory( pMac->hHdd,
                           &pMlmReassocInd->addIE.addIEdata,
                           pAssocReq->addIE.addIEdata,
                           pAssocReq->addIE.length);

            pMlmReassocInd->addIE.length = pAssocReq->addIE.length;
       }

#ifdef WLAN_SOFTAP_FEATURE
        if(pAssocReq->wmeInfoPresent)
        {

            if (wlan_cfgGetInt(pMac, (tANI_U16) WNI_CFG_WME_ENABLED, &tmp) != eSIR_SUCCESS)
                 limLog(pMac, LOGP, FL("wlan_cfgGetInt failed for id %d\n"), WNI_CFG_WME_ENABLED );

            /*                                      */
            if(tmp)
            {
                pMlmReassocInd->WmmStaInfoPresent = 1;
            }
            else
            {
                pMlmReassocInd->WmmStaInfoPresent = 0;
            }
            /*                                                                      */

        }
#endif

        //                                                  
        pMlmReassocInd->assocReqLength = pAssocReq->assocReqFrameLength;
        pMlmReassocInd->assocReqPtr = pAssocReq->assocReqFrame;

        pMlmReassocInd->beaconPtr = psessionEntry->beacon;
        pMlmReassocInd->beaconLength = psessionEntry->bcnLen;

        limPostSmeMessage(pMac, LIM_MLM_REASSOC_IND, (tANI_U32 *) pMlmReassocInd);
        palFreeMemory( pMac->hHdd, pMlmReassocInd);
    }

    return;

} /*                              */
