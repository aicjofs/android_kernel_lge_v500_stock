/*
 * Copyright (c) 2012-2013, The Linux Foundation. All rights reserved.
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

#include "limTypes.h"
#include "limUtils.h"
#include "limSerDesUtils.h"
#include "limApi.h"
#include "limSession.h"
#if defined WLAN_FEATURE_VOWIFI
#include "rrmApi.h"
#endif



/* 
                                         
  
           
                                                     
                                                        
                    
  
                                                                                                  
  
        
  
              
     
  
       
     
  
                                                 
  
                                          
 */

tANI_U32
limDeactivateMinChannelTimerDuringScan(tpAniSirGlobal pMac)
{
    if ((pMac->lim.gLimMlmState == eLIM_MLM_WT_PROBE_RESP_STATE) && (pMac->lim.gLimHalScanState == eLIM_HAL_SCANNING_STATE))
    {
        /* 
                                                                       
                                                      
            */
        
        limDeactivateAndChangeTimer(pMac,eLIM_MIN_CHANNEL_TIMER);
        MTRACE(macTrace(pMac, TRACE_CODE_TIMER_ACTIVATE, NO_SESSION, eLIM_MAX_CHANNEL_TIMER));
        if (tx_timer_activate(&pMac->lim.limTimers.gLimMaxChannelTimer)
                                          == TX_TIMER_ERROR)
        {
            //                                       
            //          
            limLog(pMac,LOGP, FL("could not activate max channel timer"));

            limCompleteMlmScan(pMac, eSIR_SME_RESOURCES_UNAVAILABLE);
            return TX_TIMER_ERROR;
        }
    }
    return eSIR_SUCCESS;
} /*                                                  */



/* 
                             
  
           
                                                     
                                                       
                                                       
                                     
  
        
  
              
     
  
       
     
  
                                                 
                                                                   
                                                          
                                                
                                                             
                                                
  
               
 */
#if defined WLAN_FEATURE_VOWIFI
void
limCollectBssDescription(tpAniSirGlobal pMac,
                         tSirBssDescription *pBssDescr,
                         tpSirProbeRespBeacon pBPR,
                         tANI_U8  *pRxPacketInfo,
                         tANI_U8  fScanning)
#else
void
limCollectBssDescription(tpAniSirGlobal pMac,
                         tSirBssDescription *pBssDescr,
                         tpSirProbeRespBeacon pBPR,
                         tANI_U8 *pRxPacketInfo)
#endif
{
    tANI_U8             *pBody;
    tANI_U32            ieLen = 0;
    tpSirMacMgmtHdr     pHdr;
    tANI_U8             channelNum;
    tANI_U8             rxChannel;

    pHdr = WDA_GET_RX_MAC_HEADER(pRxPacketInfo);
    VOS_ASSERT(WDA_GET_RX_PAYLOAD_LEN(pRxPacketInfo) >= SIR_MAC_B_PR_SSID_OFFSET);
    ieLen    = WDA_GET_RX_PAYLOAD_LEN(pRxPacketInfo) - SIR_MAC_B_PR_SSID_OFFSET;
    rxChannel = WDA_GET_RX_CH(pRxPacketInfo);
    pBody = WDA_GET_RX_MPDU_DATA(pRxPacketInfo);


    /* 
                                                    
                                          
                                          
     */
    pBssDescr->length = (tANI_U16)(
                    sizeof(tSirBssDescription) - sizeof(tANI_U16) -
                    sizeof(tANI_U32) + ieLen);

    //            
    palCopyMemory( pMac->hHdd, (tANI_U8 *) &pBssDescr->bssId,
                  (tANI_U8 *) pHdr->bssId,
                  sizeof(tSirMacAddr));

    //                                                    
    pBssDescr->scanSysTimeMsec = vos_timer_get_system_time();

    pBssDescr->timeStamp[0]   = pBPR->timeStamp[0];
    pBssDescr->timeStamp[1]   = pBPR->timeStamp[1];
    pBssDescr->beaconInterval = pBPR->beaconInterval;
    pBssDescr->capabilityInfo = limGetU16((tANI_U8 *) &pBPR->capabilityInfo);


    /*
                                                                                                 
                                                                                                
                                                                       
                                                                                                  
                                      
                                                                                                        
                                                                                                             
                                                                                                                    
                                                                              
                                                                                                                                   
                                                                                              
    */
    pBssDescr->channelId = limGetChannelFromBeacon(pMac, pBPR);

    if (pBssDescr->channelId == 0)
   {
      /*                                                                             */
      /*                                                                                 
                                                                                              */
      if (!( rxChannel = limUnmapChannel(rxChannel)))
      {
         rxChannel = pMac->lim.gLimCurrentScanChannelId;
      }
      pBssDescr->channelId = rxChannel;
   }

    pBssDescr->channelIdSelf = pBssDescr->channelId;
    //                                       
    channelNum = pBssDescr->channelId;
    pBssDescr->nwType = limGetNwType(pMac, channelNum, SIR_MAC_MGMT_FRAME, pBPR);

    pBssDescr->aniIndicator = pBPR->propIEinfo.aniIndicator;

    //                         

    PELOG4(limLog(pMac, LOG4, "***********BSS Description for BSSID:*********** ");
    sirDumpBuf(pMac, SIR_LIM_MODULE_ID, LOG4, pBssDescr->bssId, 6 );
    sirDumpBuf( pMac, SIR_LIM_MODULE_ID, LOG4, (tANI_U8*)pRxPacketInfo, 36 );)

    pBssDescr->rssi = (tANI_S8)WDA_GET_RX_RSSI_DB(pRxPacketInfo);
    
    //                             
    pBssDescr->sinr = 0;

    pBssDescr->nReceivedTime = (tANI_TIMESTAMP)palGetTickCount(pMac->hHdd);

#if defined WLAN_FEATURE_VOWIFI
    if( fScanning )
    {
       rrmGetStartTSF( pMac, pBssDescr->startTSF );
       pBssDescr->parentTSF = WDA_GET_RX_TIMESTAMP(pRxPacketInfo); 
    }
#endif

#ifdef WLAN_FEATURE_VOWIFI_11R
    //               
    pBssDescr->mdie[0] = 0;
    pBssDescr->mdie[1] = 0;
    pBssDescr->mdie[2] = 0;
    pBssDescr->mdiePresent = FALSE;
    //                                         
    //                               
    if( pBPR->mdiePresent) 
    {
        pBssDescr->mdiePresent = TRUE;
        pBssDescr->mdie[0] = pBPR->mdie[0];
        pBssDescr->mdie[1] = pBPR->mdie[1];
        pBssDescr->mdie[2] = pBPR->mdie[2];
    }
#endif

#ifdef FEATURE_WLAN_CCX
    pBssDescr->QBSSLoad_present = FALSE;
    pBssDescr->QBSSLoad_avail = 0; 
    if( pBPR->QBSSLoad.present) 
    {
        pBssDescr->QBSSLoad_present = TRUE;
        pBssDescr->QBSSLoad_avail = pBPR->QBSSLoad.avail;
    }
#endif
    //               
    palCopyMemory( pMac->hHdd, (tANI_U8 *) &pBssDescr->ieFields,
                  pBody + SIR_MAC_B_PR_SSID_OFFSET,
                  ieLen);

    //                                                                                          
    limLog( pMac, LOG3,
        FL("Collected BSS Description for Channel(%1d), length(%u), aniIndicator(%d), IE Fields(%u)"),
        pBssDescr->channelId,
        pBssDescr->length,
        pBssDescr->aniIndicator,
        ieLen );

    return;
} /*                                    */

/* 
                           
  
           
                                                     
                                                       
                                                         
  
        
  
              
     
  
       
     
  
                                                 
                                                                                        
                                                        
                                                
  
                                                                               
 */

tANI_BOOLEAN limIsScanRequestedSSID(tpAniSirGlobal pMac, tSirMacSSid *ssId)
{
    tANI_U8 i = 0;

    for (i = 0; i < pMac->lim.gpLimMlmScanReq->numSsid; i++)
    {
        if ( eANI_BOOLEAN_TRUE == palEqualMemory( pMac->hHdd,(tANI_U8 *) ssId,
                   (tANI_U8 *) &pMac->lim.gpLimMlmScanReq->ssId[i],
                   (tANI_U8) (pMac->lim.gpLimMlmScanReq->ssId[i].length + 1)))
        {
            return eANI_BOOLEAN_TRUE;
        }
    }
    return eANI_BOOLEAN_FALSE;
}

/* 
                                 
  
           
                                                     
                                                       
                                                       
                                     
  
        
  
              
     
  
       
     
  
                                                 
                                                                   
                                                          
                                                                                                                  
  
               
 */

void
limCheckAndAddBssDescription(tpAniSirGlobal pMac,
                             tpSirProbeRespBeacon pBPR,
                             tANI_U8 *pRxPacketInfo,
                             tANI_BOOLEAN fScanning,
                             tANI_U8 fProbeRsp)
{
    tLimScanResultNode   *pBssDescr;
    tANI_U32              frameLen, ieLen = 0;
    tANI_U8               rxChannelInBeacon = 0;
    eHalStatus            status;
    tANI_U8               dontUpdateAll = 0;

    tSirMacAddr bssid = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
    tANI_BOOLEAN fFound = FALSE;
    tpSirMacDataHdr3a pHdr;

    pHdr = WDA_GET_RX_MPDUHEADER3A((tANI_U8 *)pRxPacketInfo);

    //                                           
    if ((fScanning) && (pMac->lim.gpLimMlmScanReq)) 
    {
        fFound = palEqualMemory(pMac->hHdd, pHdr->addr3, &pMac->lim.gpLimMlmScanReq->bssId, 6);
        if (!fFound)
        {
            if ((pMac->lim.gpLimMlmScanReq->p2pSearch) &&
               (palEqualMemory(pMac->hHdd, pBPR->P2PProbeRes.P2PDeviceInfo.P2PDeviceAddress, 
               &pMac->lim.gpLimMlmScanReq->bssId, 6)))
            {
                fFound = eANI_BOOLEAN_TRUE;
            }
        }
    }

    /* 
                                        
                                   
                                      
                    
                                                 
                                
                                                                             
                                                                     
     */

#ifdef WLAN_FEATURE_ROAM_SCAN_OFFLOAD
    if (!(WDA_GET_OFFLOADSCANLEARN(pRxPacketInfo)))
    {
#endif
      if ((pMac->lim.gpLimMlmScanReq) &&
         (((fScanning) &&
           ( pMac->lim.gLimReturnAfterFirstMatch & 0x01 ) &&
           (pMac->lim.gpLimMlmScanReq->numSsid) &&
           !limIsScanRequestedSSID(pMac, &pBPR->ssId)) ||
          (!fFound && (pMac->lim.gpLimMlmScanReq &&
                       pMac->lim.gpLimMlmScanReq->bssId) &&
           !palEqualMemory(pMac->hHdd, bssid,
                           &pMac->lim.gpLimMlmScanReq->bssId, 6))))
    {
        /* 
                                            
                                      
                                       
         */

        return;
    }
#ifdef WLAN_FEATURE_ROAM_SCAN_OFFLOAD
    }
#endif

    /*                                                                  
                                                                        
                                                                              
                                                    
     */
    if(pBPR->channelSwitchPresent)
    {
        return;
    }

    /*                                                           
                                                                      
                                                                          
     */

    if (pBPR->dsParamsPresent)
    {
       /*                                                        */
       rxChannelInBeacon = limGetChannelFromBeacon(pMac, pBPR);
       if (rxChannelInBeacon < 15)
       {
          /*                                       */
          if(WDA_GET_RX_CH(pRxPacketInfo) != rxChannelInBeacon)
          {
             /*                                       */
             if(WDA_IS_RX_BCAST(pRxPacketInfo))
             {
                limLog(pMac, LOG3, FL("Beacon/Probe Rsp dropped. Channel in BD %d. "
                                      "Channel in beacon" " %d"),
                       WDA_GET_RX_CH(pRxPacketInfo),limGetChannelFromBeacon(pMac, pBPR));
                return;
             }
             /*                                         */
             else
             {
                dontUpdateAll = 1;
                limLog(pMac, LOG3, FL("SSID %s, CH in ProbeRsp %d, CH in BD %d, miss-match, Do Not Drop"),
                                       pBPR->ssId.ssId,
                                       rxChannelInBeacon,
                                       WDA_GET_RX_CH(pRxPacketInfo));
                WDA_GET_RX_CH(pRxPacketInfo) = rxChannelInBeacon;
             }
          }
       }
    }

    /* 
                                                   
                             
                                                  
     */

    ieLen = WDA_GET_RX_PAYLOAD_LEN(pRxPacketInfo);
    if (ieLen <= SIR_MAC_B_PR_SSID_OFFSET)
    {
               limLog(pMac, LOGP,
                   FL("RX packet has invalid length %d\n"), ieLen);
                  return;
    }

    ieLen -= SIR_MAC_B_PR_SSID_OFFSET;

    frameLen = sizeof(tLimScanResultNode) + ieLen - sizeof(tANI_U32); //                                   

    if( eHAL_STATUS_SUCCESS != palAllocateMemory( pMac->hHdd, (void **)&pBssDescr, frameLen))
    {
        //          
        limLog(pMac, LOGP,
           FL("call for palAllocateMemory failed for storing BSS description"));

        return;
    }

    //                                  
#if defined WLAN_FEATURE_VOWIFI
    limCollectBssDescription(pMac, &pBssDescr->bssDescription,
                             pBPR, pRxPacketInfo, fScanning);
#else
    limCollectBssDescription(pMac, &pBssDescr->bssDescription,
                             pBPR, pRxPacketInfo);
#endif
    pBssDescr->bssDescription.fProbeRsp = fProbeRsp;

    pBssDescr->next = NULL;

    /* 
                                                  
                                                   
                                                         
     */

#ifdef WLAN_FEATURE_ROAM_SCAN_OFFLOAD
    if (WDA_GET_OFFLOADSCANLEARN(pRxPacketInfo))
    {
       limLog(pMac, LOG2, FL(" pHdr->addr1:%02x:%02x:%02x:%02x:%02x:%02x\n"),
              pHdr->addr1[0],
              pHdr->addr1[1],
              pHdr->addr1[2],
              pHdr->addr1[3],
              pHdr->addr1[4],
              pHdr->addr1[5]);
       limLog(pMac, LOG2, FL(" pHdr->addr2:%02x:%02x:%02x:%02x:%02x:%02x\n"),
              pHdr->addr2[0],
              pHdr->addr2[1],
              pHdr->addr2[2],
              pHdr->addr2[3],
              pHdr->addr2[4],
              pHdr->addr2[5]);
       limLog(pMac, LOG2, FL(" pHdr->addr3:%02x:%02x:%02x:%02x:%02x:%02x\n"),
              pHdr->addr3[0],
              pHdr->addr3[1],
              pHdr->addr3[2],
              pHdr->addr3[3],
              pHdr->addr3[4],
              pHdr->addr3[5]);
       limLog( pMac, LOG2, FL("Save this entry in LFR cache"));
       status = limLookupNaddLfrHashEntry(pMac, pBssDescr, LIM_HASH_ADD, dontUpdateAll);
    }
    else
#endif
    //                                               
    if (pMac->lim.gLimReturnUniqueResults || (!fScanning))
    {
        status = limLookupNaddHashEntry(pMac, pBssDescr, LIM_HASH_UPDATE, dontUpdateAll);
    }
    else
    {
        status = limLookupNaddHashEntry(pMac, pBssDescr, LIM_HASH_ADD, dontUpdateAll);
    }

    if(fScanning)
    {
        if ((pBssDescr->bssDescription.channelId <= 14) &&
            (pMac->lim.gLimReturnAfterFirstMatch & 0x40) &&
            pBPR->countryInfoPresent)
            pMac->lim.gLim24Band11dScanDone = 1;

        if ((pBssDescr->bssDescription.channelId > 14) &&
            (pMac->lim.gLimReturnAfterFirstMatch & 0x80) &&
            pBPR->countryInfoPresent)
            pMac->lim.gLim50Band11dScanDone = 1;

        if ( ( pMac->lim.gLimReturnAfterFirstMatch & 0x01 ) ||
             ( pMac->lim.gLim24Band11dScanDone && ( pMac->lim.gLimReturnAfterFirstMatch & 0x40 ) ) ||
             ( pMac->lim.gLim50Band11dScanDone && ( pMac->lim.gLimReturnAfterFirstMatch & 0x80 ) ) ||
              fFound )
        {
            /* 
                                                              
                                
             */
            limLog(pMac,
                   LOGW,
                   FL("Completed scan: 24Band11dScan = %d, 50Band11dScan = %d BSS id"),
                   pMac->lim.gLim24Band11dScanDone,
                   pMac->lim.gLim50Band11dScanDone);

            //                                                                 
            //                                                                             
            limDeactivateAndChangeTimer(pMac, eLIM_MIN_CHANNEL_TIMER);
            limDeactivateAndChangeTimer(pMac, eLIM_MAX_CHANNEL_TIMER);
            //                                                       
            //                                                               
            peSetResumeChannel(pMac, 0, 0);
            limSendHalFinishScanReq( pMac, eLIM_HAL_FINISH_SCAN_WAIT_STATE );
            //                                                                 
        }
    }//                                

    if( eHAL_STATUS_SUCCESS != status )
    {
        palFreeMemory( pMac->hHdd, pBssDescr );
    }
} /*                                              */



/* 
                        
  
           
                                                            
  
        
  
              
     
  
       
     
  
                                 
  
                     
 */

tANI_U8
limScanHashFunction(tSirMacAddr bssId)
{
    tANI_U16    i, hash = 0;

    for (i = 0; i < sizeof(tSirMacAddr); i++)
        hash += bssId[i];

    return hash % LIM_MAX_NUM_OF_SCAN_RESULTS;
} /*                                     */



/* 
                     
  
           
                                                       
                                              
  
        
  
              
     
  
       
     
  
                                                 
               
 */

void
limInitHashTable(tpAniSirGlobal pMac)
{
    tANI_U16 i;
    for (i = 0; i < LIM_MAX_NUM_OF_SCAN_RESULTS; i++)
        pMac->lim.gLimCachedScanHashTable[i] = NULL;
} /*                                  */



/* 
                           
  
           
                                                     
                                                  
                                                        
  
        
  
              
     
  
       
     
  
                                                 
                                                       
                                               
                                                    
                                                      
                                                     
                            
  
               
 */

eHalStatus
limLookupNaddHashEntry(tpAniSirGlobal pMac,
                       tLimScanResultNode *pBssDescr, tANI_U8 action,
                       tANI_U8 dontUpdateAll)
{
    tANI_U8                  index, ssidLen = 0;
    tANI_U8                found = false;
    tLimScanResultNode *ptemp, *pprev;
    tSirMacCapabilityInfo *pSirCap, *pSirCapTemp;
    int idx, len;
    tANI_U8 *pbIe;
    tANI_S8  rssi = 0;

    index = limScanHashFunction(pBssDescr->bssDescription.bssId);
    ptemp = pMac->lim.gLimCachedScanHashTable[index];

    //                                  
    ssidLen = * ((tANI_U8 *) &pBssDescr->bssDescription.ieFields + 1);
    pSirCap = (tSirMacCapabilityInfo *)&pBssDescr->bssDescription.capabilityInfo;

    for (pprev = ptemp; ptemp; pprev = ptemp, ptemp = ptemp->next)
    {
        //                                                              
        pSirCapTemp = (tSirMacCapabilityInfo *)&ptemp->bssDescription.capabilityInfo;
        if((pSirCapTemp->ess == pSirCap->ess) && //                       
            (palEqualMemory( pMac->hHdd,(tANI_U8 *) pBssDescr->bssDescription.bssId,
                      (tANI_U8 *) ptemp->bssDescription.bssId,
                      sizeof(tSirMacAddr))) &&   //              
            (pBssDescr->bssDescription.channelId ==
                                      ptemp->bssDescription.channelId) &&
            palEqualMemory( pMac->hHdd,((tANI_U8 *) &pBssDescr->bssDescription.ieFields + 1),
                           ((tANI_U8 *) &ptemp->bssDescription.ieFields + 1),
                           (tANI_U8) (ssidLen + 1)) &&
            ((pSirCapTemp->ess) || //                              
            //                              
            (((pBssDescr->bssDescription.nwType ==
                                         ptemp->bssDescription.nwType) &&
            (pBssDescr->bssDescription.channelId ==
                                      ptemp->bssDescription.channelId))))
        )
        {
            //                               
            if (action == LIM_HASH_UPDATE)
            {
                if(dontUpdateAll)
                {
                   rssi = ptemp->bssDescription.rssi;
                }

                if(pBssDescr->bssDescription.fProbeRsp != ptemp->bssDescription.fProbeRsp)
                {
                    //                                                            
                    idx = 0;
                    len = ptemp->bssDescription.length - sizeof(tSirBssDescription) + 
                       sizeof(tANI_U16) + sizeof(tANI_U32) - DOT11F_IE_WSCPROBERES_MIN_LEN - 2;
                    pbIe = (tANI_U8 *)ptemp->bssDescription.ieFields;
                    //                        
                    pBssDescr->bssDescription.WscIeLen = 0;
                    while(idx < len)
                    {
                        if((DOT11F_EID_WSCPROBERES == pbIe[0]) &&
                           (0x00 == pbIe[2]) && (0x50 == pbIe[3]) && (0xf2 == pbIe[4]) && (0x04 == pbIe[5]))
                        {
                            //        
                            if((DOT11F_IE_WSCPROBERES_MAX_LEN - 2) >= pbIe[1])
                            {
                                palCopyMemory(pMac->hHdd, pBssDescr->bssDescription.WscIeProbeRsp,
                                   pbIe, pbIe[1] + 2);
                                pBssDescr->bssDescription.WscIeLen = pbIe[1] + 2;
                            }
                            break;
                        }
                        idx += pbIe[1] + 2;
                        pbIe += pbIe[1] + 2;
                    }
                }


                if(NULL != pMac->lim.gpLimMlmScanReq)
                {
                   if((pMac->lim.gpLimMlmScanReq->numSsid)&&
                      ( limIsNullSsid((tSirMacSSid *)((tANI_U8 *)
                      &pBssDescr->bssDescription.ieFields + 1))))
                      return eHAL_STATUS_FAILURE;
                }

                //                  
                if (ptemp == pMac->lim.gLimCachedScanHashTable[index])
                    pprev = pMac->lim.gLimCachedScanHashTable[index] = ptemp->next;
                else
                    pprev->next = ptemp->next;

                pMac->lim.gLimMlmScanResultLength -=
                    ptemp->bssDescription.length + sizeof(tANI_U16);

                palFreeMemory( pMac->hHdd, (tANI_U8 *) ptemp);
            }
            found = true;
            break;
        }
    }

    //                                             
    if ((action == LIM_HASH_UPDATE) && dontUpdateAll && rssi)
    {
        pBssDescr->bssDescription.rssi = rssi;
    }

    //                                       
    if (pprev == pMac->lim.gLimCachedScanHashTable[index])
    {
        pBssDescr->next = pMac->lim.gLimCachedScanHashTable[index];
        pMac->lim.gLimCachedScanHashTable[index] = pBssDescr;
    }
    else
    {
        pBssDescr->next = pprev->next;
        pprev->next = pBssDescr;
    }
    pMac->lim.gLimMlmScanResultLength +=
        pBssDescr->bssDescription.length + sizeof(tANI_U16);

    PELOG2(limLog(pMac, LOG2, FL("Added new BSS description size %d TOT %d BSS id"),
           pBssDescr->bssDescription.length,
           pMac->lim.gLimMlmScanResultLength);
    limPrintMacAddr(pMac, pBssDescr->bssDescription.bssId, LOG2);)

    //                                                          
    if (!found) limSendSmeNeighborBssInd(pMac, pBssDescr);

    //
    //                                         
    //                                     
    //                                                        
    //
    return eHAL_STATUS_SUCCESS;
}



/* 
                       
  
           
                                         
                                                 
  
        
  
              
     
  
       
                                          
  
                                                       
                                                   
  
               
 */

void    limDeleteHashEntry(tLimScanResultNode *pBssDescr)
{
} /*                                    */


#ifdef WLAN_FEATURE_ROAM_SCAN_OFFLOAD
/* 
                        
  
           
                                                       
                                                  
  
        
  
              
     
  
       
     
  
                                                 
               
 */

void
limInitLfrHashTable(tpAniSirGlobal pMac)
{
    tANI_U16 i;
    for (i = 0; i < LIM_MAX_NUM_OF_SCAN_RESULTS; i++)
        pMac->lim.gLimCachedLfrScanHashTable[i] = NULL;
} /*                                     */



/* 
                              
  
           
                                                     
                                                              
                                                            
  
        
  
              
     
  
       
     
  
                                                 
                                                       
                                                   
                                                    
                                                      
                                                     
                            
  
               
 */

eHalStatus
limLookupNaddLfrHashEntry(tpAniSirGlobal pMac,
                          tLimScanResultNode *pBssDescr, tANI_U8 action,
                          tANI_U8 dontUpdateAll)
{
    tANI_U8                  index, ssidLen = 0;
    tLimScanResultNode *ptemp, *pprev;
    tSirMacCapabilityInfo *pSirCap, *pSirCapTemp;
    int idx, len;
    tANI_U8 *pbIe;
    tANI_S8  rssi = 0;

    index = limScanHashFunction(pBssDescr->bssDescription.bssId);
    ptemp = pMac->lim.gLimCachedLfrScanHashTable[index];

    //                                  
    ssidLen = * ((tANI_U8 *) &pBssDescr->bssDescription.ieFields + 1);
    pSirCap = (tSirMacCapabilityInfo *)&pBssDescr->bssDescription.capabilityInfo;

    for (pprev = ptemp; ptemp; pprev = ptemp, ptemp = ptemp->next)
    {
        //                                                              
        pSirCapTemp = (tSirMacCapabilityInfo *)&ptemp->bssDescription.capabilityInfo;
        if((pSirCapTemp->ess == pSirCap->ess) && //                       
            (palEqualMemory( pMac->hHdd,(tANI_U8 *) pBssDescr->bssDescription.bssId,
                      (tANI_U8 *) ptemp->bssDescription.bssId,
                      sizeof(tSirMacAddr))) &&   //              
            (pBssDescr->bssDescription.channelId ==
                                      ptemp->bssDescription.channelId) &&
            palEqualMemory( pMac->hHdd,((tANI_U8 *) &pBssDescr->bssDescription.ieFields + 1),
                           ((tANI_U8 *) &ptemp->bssDescription.ieFields + 1),
                           (tANI_U8) (ssidLen + 1)) &&
            ((pSirCapTemp->ess) || //                              
            //                              
            (((pBssDescr->bssDescription.nwType ==
                                         ptemp->bssDescription.nwType) &&
            (pBssDescr->bssDescription.channelId ==
                                      ptemp->bssDescription.channelId))))
        )
        {
            //                               
            if (action == LIM_HASH_UPDATE)
            {
                if(dontUpdateAll)
                {
                   rssi = ptemp->bssDescription.rssi;
                }

                if(pBssDescr->bssDescription.fProbeRsp != ptemp->bssDescription.fProbeRsp)
                {
                    //                                                            
                    idx = 0;
                    len = ptemp->bssDescription.length - sizeof(tSirBssDescription) +
                       sizeof(tANI_U16) + sizeof(tANI_U32) - DOT11F_IE_WSCPROBERES_MIN_LEN - 2;
                    pbIe = (tANI_U8 *)ptemp->bssDescription.ieFields;
                    //                        
                    pBssDescr->bssDescription.WscIeLen = 0;
                    while(idx < len)
                    {
                        if((DOT11F_EID_WSCPROBERES == pbIe[0]) &&
                           (0x00 == pbIe[2]) && (0x50 == pbIe[3]) &&
                           (0xf2 == pbIe[4]) && (0x04 == pbIe[5]))
                        {
                            //        
                            if((DOT11F_IE_WSCPROBERES_MAX_LEN - 2) >= pbIe[1])
                            {
                                palCopyMemory(pMac->hHdd, pBssDescr->bssDescription.WscIeProbeRsp,
                                   pbIe, pbIe[1] + 2);
                                pBssDescr->bssDescription.WscIeLen = pbIe[1] + 2;
                            }
                            break;
                        }
                        idx += pbIe[1] + 2;
                        pbIe += pbIe[1] + 2;
                    }
                }


                if(NULL != pMac->lim.gpLimMlmScanReq)
                {
                   if((pMac->lim.gpLimMlmScanReq->numSsid)&&
                      ( limIsNullSsid((tSirMacSSid *)((tANI_U8 *)
                      &pBssDescr->bssDescription.ieFields + 1))))
                      return eHAL_STATUS_FAILURE;
                }

                //                  
                if (ptemp == pMac->lim.gLimCachedLfrScanHashTable[index])
                    pprev = pMac->lim.gLimCachedLfrScanHashTable[index] = ptemp->next;
                else
                    pprev->next = ptemp->next;

                pMac->lim.gLimMlmLfrScanResultLength -=
                    ptemp->bssDescription.length + sizeof(tANI_U16);

                palFreeMemory( pMac->hHdd, (tANI_U8 *) ptemp);
            }
            break;
        }
    }

    //                                             
    if ((action == LIM_HASH_UPDATE) && dontUpdateAll && rssi)
    {
        pBssDescr->bssDescription.rssi = rssi;
    }

    //                                       
    if (pprev == pMac->lim.gLimCachedLfrScanHashTable[index])
    {
        pBssDescr->next = pMac->lim.gLimCachedLfrScanHashTable[index];
        pMac->lim.gLimCachedLfrScanHashTable[index] = pBssDescr;
    }
    else
    {
        pBssDescr->next = pprev->next;
        pprev->next = pBssDescr;
    }
    pMac->lim.gLimMlmLfrScanResultLength +=
        pBssDescr->bssDescription.length + sizeof(tANI_U16);

    PELOG2(limLog(pMac, LOG2, FL("Added new BSS description size %d TOT %d BSS id\n"),
           pBssDescr->bssDescription.length,
           pMac->lim.gLimMlmLfrScanResultLength);
    limPrintMacAddr(pMac, pBssDescr->bssDescription.bssId, LOG2);)

    //
    //                                         
    //                                     
    //                                                        
    //
    return eHAL_STATUS_SUCCESS;
}



/* 
                          
  
           
                                         
                                                     
  
        
  
              
     
  
       
                                          
  
                                                       
                                                       
  
               
 */

void    limDeleteLfrHashEntry(tLimScanResultNode *pBssDescr)
{
} /*                                       */

#endif //                              

/* 
                      
  
           
                                                           
                                                
  
        
                                                                  
  
              
     
  
       
     
  
                                                 
                                      
  
               
 */

void
limCopyScanResult(tpAniSirGlobal pMac, tANI_U8 *pDest)
{
    tLimScanResultNode    *ptemp;
    tANI_U16 i;
    for (i = 0; i < LIM_MAX_NUM_OF_SCAN_RESULTS; i++)
    {
        if ((ptemp = pMac->lim.gLimCachedScanHashTable[i]) != NULL)
        {
            while(ptemp)
            {
                //                                              
                palCopyMemory( pMac->hHdd, pDest,
                              (tANI_U8 *) &ptemp->bssDescription,
                              ptemp->bssDescription.length + 2);
                pDest += ptemp->bssDescription.length + 2;
                ptemp = ptemp->next;
            }
        }
    }
} /*                                   */



/* 
                               
  
           
                                                                    
                                                
  
        
                                                                  
                                 
  
              
     
  
       
     
  
                                                 
               
 */

void
limDeleteCachedScanResults(tpAniSirGlobal pMac)
{
    tLimScanResultNode    *pNode, *pNextNode;
    tANI_U16 i;
    for (i = 0; i < LIM_MAX_NUM_OF_SCAN_RESULTS; i++)
    {
        if ((pNode = pMac->lim.gLimCachedScanHashTable[i]) != NULL)
        {
            while (pNode)
            {
                pNextNode = pNode->next;

                //                        
                palFreeMemory( pMac->hHdd, (tANI_U8 *) pNode);

                pNode = pNextNode;
            }
        }
    }

    pMac->lim.gLimSmeScanResultLength = 0;
} /*                                            */



/* 
                         
  
           
                                                        
                                     
  
        
  
              
     
  
       
     
  
                                                 
               
 */

void
limReInitScanResults(tpAniSirGlobal pMac)
{
    limDeleteCachedScanResults(pMac);
    limInitHashTable(pMac);

    //                                                        
    //                                                    
    pMac->lim.gLimMlmScanResultLength = 0;

} /*                                      */
#ifdef WLAN_FEATURE_ROAM_SCAN_OFFLOAD
/* 
                                  
  
           
                                                                    
                                                        
  
        
                                                                
                                       
  
              
     
  
       
     
  
                                                 
               
 */

void
limDeleteCachedLfrScanResults(tpAniSirGlobal pMac)
{
    tLimScanResultNode    *pNode, *pNextNode;
    tANI_U16 i;
    for (i = 0; i < LIM_MAX_NUM_OF_SCAN_RESULTS; i++)
    {
        if ((pNode = pMac->lim.gLimCachedLfrScanHashTable[i]) != NULL)
        {
            while (pNode)
            {
                pNextNode = pNode->next;

                //                        
                palFreeMemory( pMac->hHdd, (tANI_U8 *) pNode);

                pNode = pNextNode;
            }
        }
    }

    pMac->lim.gLimSmeLfrScanResultLength = 0;
} /*                                               */



/* 
                            
  
           
                                                        
                                         
  
        
  
              
     
  
       
     
  
                                                 
               
 */

void
limReInitLfrScanResults(tpAniSirGlobal pMac)
{
    limDeleteCachedLfrScanResults(pMac);
    limInitLfrHashTable(pMac);

    //                                                        
    //                                                    
    pMac->lim.gLimMlmLfrScanResultLength = 0;

} /*                                         */
#endif //                              
