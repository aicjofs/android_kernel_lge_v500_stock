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

/**=========================================================================
  
  \file  sme_Rrm.c
  
  \brief implementation for SME RRM APIs
  
   Copyright 2008 (c) Qualcomm, Incorporated.  All Rights Reserved.
   
   Qualcomm Confidential and Proprietary.
  
  ========================================================================*/

/*          */

#if defined WLAN_FEATURE_VOWIFI
/*                                                                          
               
                                                                          */
#include "aniGlobal.h"
#include "smeInside.h"
#include "sme_Api.h"
#include "smsDebug.h"
#include "cfgApi.h"

#ifdef FEATURE_WLAN_DIAG_SUPPORT
#include "vos_diag_core_event.h"
#include "vos_diag_core_log.h"
#endif /*                           */

#include "csrInsideApi.h"

#include "rrmGlobal.h"

#ifdef FEATURE_WLAN_CCX
#include "csrCcx.h"
#endif

/*                                                                                
                                                                                                  */
#define RRM_ROAM_SCORE_NEIGHBOR_REPORT_REACHABILITY             0   /*                                                                    */
#define RRM_ROAM_SCORE_NEIGHBOR_REPORT_SECURITY                 10
#define RRM_ROAM_SCORE_NEIGHBOR_REPORT_KEY_SCOPE                20
#define RRM_ROAM_SCORE_NEIGHBOR_REPORT_CAPABILITY_SPECTRUM_MGMT 0   /*          */
#define RRM_ROAM_SCORE_NEIGHBOR_REPORT_CAPABILITY_QOS           5
#define RRM_ROAM_SCORE_NEIGHBOR_REPORT_CAPABILITY_APSD          3
#define RRM_ROAM_SCORE_NEIGHBOR_REPORT_CAPABILITY_RRM           8
#define RRM_ROAM_SCORE_NEIGHBOR_REPORT_CAPABILITY_DELAYED_BA    0   /*                            */
#define RRM_ROAM_SCORE_NEIGHBOR_REPORT_CAPABILITY_IMMEDIATE_BA  3
#define RRM_ROAM_SCORE_NEIGHBOR_REPORT_MOBILITY_DOMAIN          30

#ifdef FEATURE_WLAN_CCX
#define RRM_ROAM_SCORE_NEIGHBOR_IAPP_LIST                       30
#endif
/*                                                                            
  
                                     
                                                                                                     

                                              
                                                           
                
  
                                                                            */
static void rrmLLPurgeNeighborCache(tpAniSirGlobal pMac, tDblLinkList *pList)
{
    tListElem *pEntry;
    tRrmNeighborReportDesc *pNeighborReportDesc;
    
    csrLLLock(pList);
    
    while((pEntry = csrLLRemoveHead(pList, LL_ACCESS_NOLOCK)) != NULL)
    {
        pNeighborReportDesc = GET_BASE_ADDR( pEntry, tRrmNeighborReportDesc, List );
        vos_mem_free(pNeighborReportDesc->pNeighborBssDescription);
        vos_mem_free(pNeighborReportDesc);
    }
    
    csrLLUnlock(pList);   
     
    return;
}

/*                                                                            
  
                                             
                                                                                     
                                                                                               
                                                

                                             
                                                                                                 
                                              
                
  
                                                                            */
void rrmIndicateNeighborReportResult(tpAniSirGlobal pMac, VOS_STATUS vosStatus)
{
    NeighborReportRspCallback callback;
    void                      *callbackContext;

    /*                                            */
    pMac->rrm.rrmSmeContext.neighborReqControlInfo.isNeighborRspPending = eANI_BOOLEAN_FALSE;

    /*                                                                                                */
    if (VOS_TIMER_STATE_RUNNING == vos_timer_getCurrentState(&pMac->rrm.rrmSmeContext.neighborReqControlInfo.neighborRspWaitTimer))
    {
        smsLog( pMac, LOG1, FL("No entry in neighbor report cache"));
        vos_timer_stop(&pMac->rrm.rrmSmeContext.neighborReqControlInfo.neighborRspWaitTimer);
    }
    callback = pMac->rrm.rrmSmeContext.neighborReqControlInfo.neighborRspCallbackInfo.neighborRspCallback;
    callbackContext = pMac->rrm.rrmSmeContext.neighborReqControlInfo.neighborRspCallbackInfo.neighborRspCallbackContext;
    
    /*                                                                                                                                
                             */
    pMac->rrm.rrmSmeContext.neighborReqControlInfo.neighborRspCallbackInfo.neighborRspCallback = NULL;
    pMac->rrm.rrmSmeContext.neighborReqControlInfo.neighborRspCallbackInfo.neighborRspCallbackContext = NULL;
    
    /*                                                        */
    if (callback)
        callback(callbackContext, vosStatus);
#ifdef FEATURE_WLAN_CCX
    //                               
    //                                             
    //                                             
    else
    if (csrNeighborRoamIsCCXAssoc(pMac))
    {
        ProcessIAPPNeighborAPList(pMac);
    }
#endif

    return;

}

/*                                                                            
  
                                        

                                                            

                                             
                                      
                                                                                        
                                               
  
                                                                            */
static eHalStatus sme_RrmSendBeaconReportXmitInd( tpAniSirGlobal pMac,
                                                  tCsrScanResultInfo **pResultArr,
                                                  tANI_U8 measurementDone,
                                                  tANI_U8 bss_count )
{
   tpSirBssDescription pBssDesc = NULL;
   tpSirBeaconReportXmitInd pBeaconRep;
   tANI_U16 length, ie_len;
   tANI_U8 bssCounter=0, msgCounter=0;
   tCsrScanResultInfo *pCurResult=NULL;
   eHalStatus status = eHAL_STATUS_FAILURE;
   tpRrmSMEContext pSmeRrmContext = &pMac->rrm.rrmSmeContext;


#if defined WLAN_VOWIFI_DEBUG
   smsLog( pMac, LOGE, "Beacon report xmit Ind to PE");
#endif

   if( NULL == pResultArr && !measurementDone )
   {
      smsLog( pMac, LOGE, "Beacon report xmit Ind to PE Failed");
      return eHAL_STATUS_FAILURE;
   }

   if (pResultArr)
       pCurResult=pResultArr[bssCounter];

   do 
   {
       length = sizeof(tSirBeaconReportXmitInd);
       pBeaconRep = vos_mem_malloc ( length );
       if ( NULL == pBeaconRep )
       {
          smsLog( pMac, LOGP, "Unable to allocate memory for beacon report");
          return eHAL_STATUS_FAILED_ALLOC;
       }
       vos_mem_zero( pBeaconRep, length );
#if defined WLAN_VOWIFI_DEBUG
       smsLog( pMac, LOGE, FL("Allocated memory for pBeaconRep"));
#endif
       pBeaconRep->messageType = eWNI_SME_BEACON_REPORT_RESP_XMIT_IND;
       pBeaconRep->length = length;
       pBeaconRep->uDialogToken = pSmeRrmContext->token;
       pBeaconRep->duration = pSmeRrmContext->duration;
       pBeaconRep->regClass = pSmeRrmContext->regClass;
       vos_mem_copy( pBeaconRep->bssId, pSmeRrmContext->sessionBssId, sizeof(tSirMacAddr) );

       msgCounter=0;
       while (pCurResult) 
       {
           pBssDesc = &pCurResult->BssDescriptor;
           if(pBssDesc != NULL)
           {
               ie_len = GET_IE_LEN_IN_BSS( pBssDesc->length );
               pBeaconRep->pBssDescription[msgCounter] = vos_mem_malloc (
                                            ie_len+sizeof(tSirBssDescription));
               if (NULL == pBeaconRep->pBssDescription[msgCounter])
                   break;
               vos_mem_copy( pBeaconRep->pBssDescription[msgCounter],
                             pBssDesc,
                             sizeof(tSirBssDescription) );
               vos_mem_copy( &pBeaconRep->pBssDescription[msgCounter]->ieFields[0],
                             pBssDesc->ieFields, ie_len  );
               smsLog( pMac, LOG1,
                   "...RRM Result Bssid = %02x-%02x-%02x-%02x-%02x-%02x chan= %d, rssi = -%d",
                   pBeaconRep->pBssDescription[msgCounter]->bssId[ 0 ],
                   pBeaconRep->pBssDescription[msgCounter]->bssId[ 1 ],
                   pBeaconRep->pBssDescription[msgCounter]->bssId[ 2 ],
                   pBeaconRep->pBssDescription[msgCounter]->bssId[ 3 ],
                   pBeaconRep->pBssDescription[msgCounter]->bssId[ 4 ],
                   pBeaconRep->pBssDescription[msgCounter]->bssId[ 5 ],
                   pBeaconRep->pBssDescription[msgCounter]->channelId,
                   pBeaconRep->pBssDescription[msgCounter]->rssi * (-1));

               pBeaconRep->numBssDesc++;

               if (++msgCounter >= SIR_BCN_REPORT_MAX_BSS_DESC_PER_ACTION_FRAME)
                   break;

               pCurResult = pResultArr[bssCounter + msgCounter];
           }
           else
           {
               pCurResult = NULL;
               break;
           }
       }

       bssCounter+=msgCounter; 
       if (!pResultArr || (pCurResult == NULL) || (bssCounter >= bss_count))
       {
           pCurResult = NULL;
           smsLog(pMac, LOG1,
                  "Reached to the max/last BSS in pCurResult list");
       }
       else
       {
           pCurResult = pResultArr[bssCounter];
           smsLog(pMac, LOG1,
                  "Move to the next BSS set in pCurResult list");
       }

       pBeaconRep->fMeasureDone = (pCurResult)?false:measurementDone;

       smsLog(pMac, LOG1,
              "SME Sending BcnRepXmit to PE numBss %d msgCounter %d bssCounter %d",
              pBeaconRep->numBssDesc, msgCounter, bssCounter);

       status = palSendMBMessage(pMac->hHdd, pBeaconRep);

   } while (pCurResult);

   return status;
}

/*                                                                            
  
                                    

                                                                                     
                          

                                             
                                              
                                                                         
                                                                                        
                                               
  
                                                                            */
static eHalStatus sme_RrmSendScanResult( tpAniSirGlobal pMac,
                                         tANI_U8 num_chan,
                                         tANI_U8* chanList,
                                         tANI_U8 measurementDone )
{
   tCsrScanResultFilter filter;
   tScanResultHandle pResult;
   tCsrScanResultInfo *pScanResult, *pNextResult;
   tCsrScanResultInfo *pScanResultsArr[SIR_BCN_REPORT_MAX_BSS_PER_CHANNEL];
   eHalStatus status;
   tANI_U8 counter=0;
   tpRrmSMEContext pSmeRrmContext = &pMac->rrm.rrmSmeContext;
   tANI_U32 sessionId;

#if defined WLAN_VOWIFI_DEBUG
   smsLog( pMac, LOGE, "Send scan result to PE ");
#endif

   vos_mem_zero( &filter, sizeof(filter) );
   vos_mem_zero( pScanResultsArr, sizeof(pNextResult)*SIR_BCN_REPORT_MAX_BSS_PER_CHANNEL );

   filter.BSSIDs.numOfBSSIDs = 1;
   filter.BSSIDs.bssid = &pSmeRrmContext->bssId;

   if( pSmeRrmContext->ssId.length )
   {
      filter.SSIDs.SSIDList =( tCsrSSIDInfo *)vos_mem_malloc(sizeof(tCsrSSIDInfo));
      if( filter.SSIDs.SSIDList == NULL )
      {
         smsLog( pMac, LOGP, FL("vos_mem_malloc failed:") );
         return eHAL_STATUS_FAILURE;
      }
#if defined WLAN_VOWIFI_DEBUG
      smsLog( pMac, LOGE, FL("Allocated memory for SSIDList"));
#endif
      vos_mem_zero( filter.SSIDs.SSIDList, sizeof(tCsrSSIDInfo) );

      filter.SSIDs.SSIDList->SSID.length = pSmeRrmContext->ssId.length;
      vos_mem_copy(filter.SSIDs.SSIDList->SSID.ssId, pSmeRrmContext->ssId.ssId, pSmeRrmContext->ssId.length);
      filter.SSIDs.numOfSSIDs = 1;
   }
   else
   {
      filter.SSIDs.numOfSSIDs = 0;
   }

   filter.ChannelInfo.numOfChannels = num_chan;
   filter.ChannelInfo.ChannelList = chanList; 

   filter.fMeasurement = TRUE; 

   csrRoamGetSessionIdFromBSSID( pMac, (tCsrBssid*)pSmeRrmContext->sessionBssId, &sessionId );
   status = sme_ScanGetResult(pMac, (tANI_U8)sessionId, &filter, &pResult);

   if( filter.SSIDs.SSIDList )
   {
      //                                       
      vos_mem_free( filter.SSIDs.SSIDList );
#if defined WLAN_VOWIFI_DEBUG
      smsLog( pMac, LOGE, FL("Free memory for SSIDList") );
#endif
   }

   if (NULL == pResult)
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
      //                                            
      if( measurementDone )
         status = sme_RrmSendBeaconReportXmitInd( pMac, NULL, measurementDone, 0 );
      return status;
   }

   pScanResult = sme_ScanResultGetFirst(pMac, pResult);

   if( NULL == pScanResult && measurementDone )
      status = sme_RrmSendBeaconReportXmitInd( pMac, NULL, measurementDone, 0 );

   counter=0;
   while (pScanResult)
   {
      pNextResult = sme_ScanResultGetNext(pMac, pResult);
      pScanResultsArr[counter++] = pScanResult;
      pScanResult = pNextResult; //                                     
      if (counter >= SIR_BCN_REPORT_MAX_BSS_PER_CHANNEL)
         break;
      }

   if (counter)
   {
       status = sme_RrmSendBeaconReportXmitInd( pMac,
                                                pScanResultsArr,
                                                measurementDone,
                                                counter);
       smsLog(pMac, LOG1, " Number of BSS Desc with RRM Scan %d ",
              counter);
   }
   sme_ScanResultPurge(pMac, pResult); 

   return status;
}
/*                                                                            
  
                                        

                                                                              
                                                                            
                                         

                                                  
                                                       
                                 
                                            
                                               
  
                                                                            */

static eHalStatus sme_RrmScanRequestCallback(tHalHandle halHandle, void *pContext,
                         tANI_U32 scanId, eCsrScanStatus status)
{

   tANI_U16 interval;
   tpAniSirGlobal pMac = (tpAniSirGlobal) halHandle;
   tpRrmSMEContext pSmeRrmContext = &pMac->rrm.rrmSmeContext;
   tANI_U32 time_tick; 



#if defined WLAN_VOWIFI_DEBUG
   smsLog( pMac, LOGE, "Scan Request callback ");
#endif
   //                                                                                                
   //
   //
   if( (pSmeRrmContext->currentIndex + 1) < pSmeRrmContext->channelList.numOfChannels )
   {
      sme_RrmSendScanResult( pMac, 1, &pSmeRrmContext->channelList.ChannelList[pSmeRrmContext->currentIndex], false );

      pSmeRrmContext->currentIndex++; //                          
      //                                       
      //                                                                              
      time_tick = vos_timer_get_system_ticks();
      interval = time_tick % (pSmeRrmContext->randnIntvl - 10 + 1) + 10;

#if defined WLAN_VOWIFI_DEBUG
      smsLog( pMac, LOGE, "Set timer for interval %d ", interval);
#endif
      vos_timer_start( &pSmeRrmContext->IterMeasTimer, interval );

   }
   else
   {
      //                                                                                                        
      sme_RrmSendScanResult( pMac, 1, &pSmeRrmContext->channelList.ChannelList[pSmeRrmContext->currentIndex], true );
      vos_mem_free( pSmeRrmContext->channelList.ChannelList );
#if defined WLAN_VOWIFI_DEBUG
      smsLog( pMac, LOGE, FL("Free memory for ChannelList") );
#endif
   }

   return eHAL_STATUS_SUCCESS;
}

/*                                                                          
                                                                               
                                   
  
         
  
                                                         
  
     
  
                                                                            */
eHalStatus sme_RrmIssueScanReq( tpAniSirGlobal pMac )
{
   //                   
   tCsrScanRequest scanRequest;
   v_U32_t scanId = 0;
   eHalStatus status = eHAL_STATUS_SUCCESS;
   tpRrmSMEContext pSmeRrmContext = &pMac->rrm.rrmSmeContext;
   tANI_U32 sessionId;

#if defined WLAN_VOWIFI_DEBUG
   smsLog( pMac, LOGE, "Issue scan request " );
#endif

   vos_mem_zero( &scanRequest, sizeof(scanRequest));

   /*                                 */
   scanRequest.bcnRptReqScan = TRUE;
   scanRequest.scanType = pSmeRrmContext->measMode;

   vos_mem_copy(scanRequest.bssid,
         pSmeRrmContext->bssId, sizeof(scanRequest.bssid) );

   if( pSmeRrmContext->ssId.length )
   {
      scanRequest.SSIDs.numOfSSIDs = 1;
      scanRequest.SSIDs.SSIDList =( tCsrSSIDInfo *)vos_mem_malloc(sizeof(tCsrSSIDInfo));
      if( scanRequest.SSIDs.SSIDList == NULL )
      {
         smsLog( pMac, LOGP, FL("vos_mem_malloc failed:") );
         return eHAL_STATUS_FAILURE;
      }
#if defined WLAN_VOWIFI_DEBUG
      smsLog( pMac, LOGE, FL("Allocated memory for pSSIDList"));
#endif
      vos_mem_zero( scanRequest.SSIDs.SSIDList, sizeof(tCsrSSIDInfo) );
      scanRequest.SSIDs.SSIDList->SSID.length = pSmeRrmContext->ssId.length;
      vos_mem_copy(scanRequest.SSIDs.SSIDList->SSID.ssId, pSmeRrmContext->ssId.ssId, pSmeRrmContext->ssId.length);
   }

   /*                              */
   scanRequest.minChnTime = 0; //                                               
   scanRequest.maxChnTime = pSmeRrmContext->duration;
#if defined WLAN_VOWIFI_DEBUG
   smsLog( pMac, LOGE, "For Duration %d ", pSmeRrmContext->duration );
#endif

   /*                             */
   scanRequest.BSSType = eCSR_BSS_TYPE_ANY;

   /*                      */
   scanRequest.ChannelInfo.numOfChannels = 1;

   scanRequest.ChannelInfo.ChannelList = &pSmeRrmContext->channelList.ChannelList[pSmeRrmContext->currentIndex];
#if defined WLAN_VOWIFI_DEBUG
   smsLog( pMac, LOGE, "On channel %d ", pSmeRrmContext->channelList.ChannelList[pSmeRrmContext->currentIndex] );
#endif

   /*                              */
   scanRequest.requestType = eCSR_SCAN_REQUEST_FULL_SCAN;

   csrRoamGetSessionIdFromBSSID( pMac, (tCsrBssid*)pSmeRrmContext->sessionBssId, &sessionId );
   status = sme_ScanRequest( pMac, (tANI_U8)sessionId, &scanRequest, &scanId, &sme_RrmScanRequestCallback, NULL ); 

   if ( pSmeRrmContext->ssId.length )
   {
      vos_mem_free(scanRequest.SSIDs.SSIDList);
#if defined WLAN_VOWIFI_DEBUG
      smsLog( pMac, LOGE, FL("Free memory for SSIDList"));
#endif
   }

   return status;
}

/*                                                                          
                                                                                   
                                                           
  
                                                                              
                                       
                                                                            
  
                                                         
  
     
  
                                                                            */
void sme_RrmProcessBeaconReportReqInd(tpAniSirGlobal pMac, void *pMsgBuf)
{
   tpSirBeaconReportReqInd pBeaconReq = (tpSirBeaconReportReqInd) pMsgBuf;
   tpRrmSMEContext pSmeRrmContext = &pMac->rrm.rrmSmeContext;
   tANI_U32 len,i;  

#if defined WLAN_VOWIFI_DEBUG
   smsLog( pMac, LOGE, "Received Beacon report request ind Channel = %d", pBeaconReq->channelInfo.channelNum );
#endif
   //                                          
   //                                      
   if( (pBeaconReq->channelInfo.channelNum == 0)  || 
       ((pBeaconReq->channelInfo.channelNum == 255) && (pBeaconReq->channelList.numChannels == 0) ) ) 
   {
      //                                             
      wlan_cfgGetStrLen( pMac, WNI_CFG_VALID_CHANNEL_LIST, &len );
      pSmeRrmContext->channelList.ChannelList = vos_mem_malloc( len );
      if( pSmeRrmContext->channelList.ChannelList == NULL )
      {
         smsLog( pMac, LOGP, FL("vos_mem_malloc failed:") );
         return;
      }
#if defined WLAN_VOWIFI_DEBUG
      smsLog( pMac, LOGE, FL("Allocated memory for ChannelList") );
#endif
      csrGetCfgValidChannels( pMac, pSmeRrmContext->channelList.ChannelList, &len );
      pSmeRrmContext->channelList.numOfChannels = (tANI_U8)len;
#if defined WLAN_VOWIFI_DEBUG
      smsLog( pMac, LOGE, "channel == 0 performing on all channels");
#endif
   }
   else
   { 
      len = 0;
      pSmeRrmContext->channelList.numOfChannels = 0;

      //                                                                      
      //                                                                                   
      if ( pBeaconReq->channelInfo.channelNum != 255 )
         len = 1;
#if defined WLAN_VOWIFI_DEBUG
      else
         smsLog( pMac, LOGE, "channel == 255");
#endif

      len += pBeaconReq->channelList.numChannels;

      pSmeRrmContext->channelList.ChannelList = vos_mem_malloc( len );
      if( pSmeRrmContext->channelList.ChannelList == NULL )
      {
         smsLog( pMac, LOGP, FL("vos_mem_malloc failed") );
         return;
      }
#if defined WLAN_VOWIFI_DEBUG
      smsLog( pMac, LOGE, FL("Allocated memory for ChannelList") );
#endif

      if ( pBeaconReq->channelInfo.channelNum != 255 )
      {
#if defined WLAN_VOWIFI_DEBUG
         smsLog( pMac, LOGE, "channel == %d  ", pBeaconReq->channelInfo.channelNum );
#endif
         if(csrRoamIsChannelValid( pMac, pBeaconReq->channelInfo.channelNum ))
            pSmeRrmContext->channelList.ChannelList[pSmeRrmContext->channelList.numOfChannels++] = pBeaconReq->channelInfo.channelNum;
#if defined WLAN_VOWIFI_DEBUG
         else
            smsLog( pMac, LOGE, "is Invalid channel, Ignoring this channel" );
#endif
      }

      for ( i = 0 ; i < pBeaconReq->channelList.numChannels; i++ )
      {
         if(csrRoamIsChannelValid( pMac, pBeaconReq->channelList.channelNumber[i] ))
         {
            pSmeRrmContext->channelList.ChannelList[pSmeRrmContext->channelList.numOfChannels] = pBeaconReq->channelList.channelNumber[i];
            pSmeRrmContext->channelList.numOfChannels++;
         }
      }
   }

   //                  
   vos_mem_copy( pSmeRrmContext->sessionBssId, pBeaconReq->bssId, sizeof(tSirMacAddr) );

   //                      
   vos_mem_copy( pSmeRrmContext->bssId, pBeaconReq->macaddrBssid, sizeof(tSirMacAddr) );

   //         
   vos_mem_copy( &pSmeRrmContext->ssId, &pBeaconReq->ssId, sizeof(tAniSSID) ); 

   pSmeRrmContext->token = pBeaconReq->uDialogToken;
   pSmeRrmContext->regClass = pBeaconReq->channelInfo.regulatoryClass;

   switch( pBeaconReq->fMeasurementtype )
   {
      case 0: //       
      case 1: //      
         pSmeRrmContext->measMode = pBeaconReq->fMeasurementtype? eSIR_ACTIVE_SCAN : eSIR_PASSIVE_SCAN ;
         pSmeRrmContext->duration = pBeaconReq->measurementDuration;
         pSmeRrmContext->randnIntvl = VOS_MAX( pBeaconReq->randomizationInterval, pSmeRrmContext->rrmConfig.maxRandnInterval );
         pSmeRrmContext->currentIndex = 0;
#if defined WLAN_VOWIFI_DEBUG
         smsLog( pMac, LOGE, "Send beacon report after scan " );
#endif
         sme_RrmIssueScanReq( pMac ); 
         break;
      case 2: //     
         //                                                               
#if defined WLAN_VOWIFI_DEBUG
         smsLog( pMac, LOGE, "Send beacon report from table " );
#endif
         sme_RrmSendScanResult( pMac, pSmeRrmContext->channelList.numOfChannels, pSmeRrmContext->channelList.ChannelList, true );
         vos_mem_free( pSmeRrmContext->channelList.ChannelList );
#if defined WLAN_VOWIFI_DEBUG
         smsLog( pMac, LOGE, FL("Free memory for ChannelList") );
#endif
         break;
      default:
#if defined WLAN_VOWIFI_DEBUG
         smsLog( pMac, LOGE, "Unknown beacon report request mode");
#endif
         /*                                       */
         /*                                                                
                                                          */
         sme_RrmSendBeaconReportXmitInd(pMac, NULL, true, 0);
         break;

   }

   return;
}

/*                                                                          
                                                                               
                                       
  
                                                                                   
                                                               
  
                                                         
  
     
  
                                                                            */
VOS_STATUS sme_RrmNeighborReportRequest(tpAniSirGlobal pMac, tANI_U8 sessionId, 
                                    tpRrmNeighborReq pNeighborReq, tpRrmNeighborRspCallbackInfo callbackInfo)
{
   eHalStatus status = eHAL_STATUS_SUCCESS;
   tpSirNeighborReportReqInd pMsg;
   tCsrRoamSession *pSession;

#if defined WLAN_VOWIFI_DEBUG
   smsLog( pMac, LOGE, FL("Request to send Neighbor report request received "));
#endif
   if( !CSR_IS_SESSION_VALID( pMac, sessionId ) )
   {  
      smsLog( pMac, LOGE, FL("Invalid session %d"), sessionId );
      return VOS_STATUS_E_INVAL;
   }
   pSession = CSR_GET_SESSION( pMac, sessionId );

   /*                                                */
   if (eANI_BOOLEAN_TRUE == pMac->rrm.rrmSmeContext.neighborReqControlInfo.isNeighborRspPending)
   {
       smsLog( pMac, LOGE, FL("Neighbor request already pending.. Not allowed"));
       return VOS_STATUS_E_AGAIN;
   }
   
   pMsg = vos_mem_malloc ( sizeof(tSirNeighborReportReqInd) );
   if ( NULL == pMsg )
   {
      smsLog( pMac, LOGE, "Unable to allocate memory for Neighbor request");
      return VOS_STATUS_E_NOMEM;
   }

   
   vos_mem_zero( pMsg, sizeof(tSirNeighborReportReqInd) );
#if defined WLAN_VOWIFI_DEBUG
   smsLog( pMac, LOGE, FL(" Allocated memory for Neighbor request") );
#endif

   rrmLLPurgeNeighborCache(pMac, &pMac->rrm.rrmSmeContext.neighborReportCache);

#if defined WLAN_VOWIFI_DEBUG
   smsLog( pMac, LOGE, FL("Purged the neighbor cache before sending Neighbor request: Status = %d"), status );
#endif

   pMsg->messageType = eWNI_SME_NEIGHBOR_REPORT_REQ_IND;
   pMsg->length = sizeof( tSirNeighborReportReqInd );
   vos_mem_copy( &pMsg->bssId, &pSession->connectedProfile.bssid, sizeof(tSirMacAddr) );
   pMsg->noSSID = pNeighborReq->no_ssid;
   vos_mem_copy( &pMsg->ucSSID, &pNeighborReq->ssid, sizeof(tSirMacSSid));

   status = palSendMBMessage(pMac->hHdd, pMsg);
   if( status != eHAL_STATUS_SUCCESS )
      return VOS_STATUS_E_FAILURE;

   /*                                                                                     */
   pMac->rrm.rrmSmeContext.neighborReqControlInfo.neighborRspCallbackInfo.neighborRspCallback = 
                                                            callbackInfo->neighborRspCallback;
   pMac->rrm.rrmSmeContext.neighborReqControlInfo.neighborRspCallbackInfo.neighborRspCallbackContext = 
                                                            callbackInfo->neighborRspCallbackContext;
   pMac->rrm.rrmSmeContext.neighborReqControlInfo.isNeighborRspPending = eANI_BOOLEAN_TRUE;

   /*                                        */
   vos_timer_start(&pMac->rrm.rrmSmeContext.neighborReqControlInfo.neighborRspWaitTimer, callbackInfo->timeout);
   
   return VOS_STATUS_SUCCESS;
}

/*                                                                          
                                                                               
                                                                               
                                                                                  
                      
  
                                                                                 
                                                    
  
               
                                                                          */
static void rrmCalculateNeighborAPRoamScore(tpAniSirGlobal pMac, tpRrmNeighborReportDesc pNeighborReportDesc)
{
    tpSirNeighborBssDescripton  pNeighborBssDesc;
    tANI_U32    roamScore = 0;
    
    if (NULL == pNeighborReportDesc)
    {
        VOS_ASSERT(0);
        return;
    }
    if (NULL == pNeighborReportDesc->pNeighborBssDescription)
    {
        VOS_ASSERT(0);
        return;
    }

    pNeighborBssDesc = pNeighborReportDesc->pNeighborBssDescription;

    if (pNeighborBssDesc->bssidInfo.rrmInfo.fMobilityDomain)
    {
        roamScore += RRM_ROAM_SCORE_NEIGHBOR_REPORT_MOBILITY_DOMAIN;
        if (pNeighborBssDesc->bssidInfo.rrmInfo.fSameSecurityMode)
        {
            roamScore += RRM_ROAM_SCORE_NEIGHBOR_REPORT_SECURITY;
            if (pNeighborBssDesc->bssidInfo.rrmInfo.fSameAuthenticator)
            {
                roamScore += RRM_ROAM_SCORE_NEIGHBOR_REPORT_KEY_SCOPE;
                if (pNeighborBssDesc->bssidInfo.rrmInfo.fCapRadioMeasurement)
                {
                    roamScore += RRM_ROAM_SCORE_NEIGHBOR_REPORT_CAPABILITY_RRM;
                    if (pNeighborBssDesc->bssidInfo.rrmInfo.fCapSpectrumMeasurement)
                        roamScore += RRM_ROAM_SCORE_NEIGHBOR_REPORT_CAPABILITY_SPECTRUM_MGMT;
                    if (pNeighborBssDesc->bssidInfo.rrmInfo.fCapQos)
                        roamScore += RRM_ROAM_SCORE_NEIGHBOR_REPORT_CAPABILITY_QOS;
                    if (pNeighborBssDesc->bssidInfo.rrmInfo.fCapApsd)
                        roamScore += RRM_ROAM_SCORE_NEIGHBOR_REPORT_CAPABILITY_APSD;
                    if (pNeighborBssDesc->bssidInfo.rrmInfo.fCapDelayedBlockAck)
                        roamScore += RRM_ROAM_SCORE_NEIGHBOR_REPORT_CAPABILITY_DELAYED_BA;
                    if (pNeighborBssDesc->bssidInfo.rrmInfo.fCapImmediateBlockAck)
                        roamScore += RRM_ROAM_SCORE_NEIGHBOR_REPORT_CAPABILITY_IMMEDIATE_BA;
                    if (pNeighborBssDesc->bssidInfo.rrmInfo.fApPreauthReachable)
                        roamScore += RRM_ROAM_SCORE_NEIGHBOR_REPORT_REACHABILITY;
                }
            }
        }
    }
#ifdef FEATURE_WLAN_CCX
    //                                                
    if (csrNeighborRoamIs11rAssoc(pMac) == FALSE)
    {
        //                                         
        //                                                    
        //                                                  
        //                                                                
        roamScore += RRM_ROAM_SCORE_NEIGHBOR_IAPP_LIST;
    }
#endif
    pNeighborReportDesc->roamScore = roamScore;

    return;
}

/*                                                                          
                                                                                
                                                                                     
                                                                                           
                                                                   

                                                                                 
  
               
                                                                          */
void rrmStoreNeighborRptByRoamScore(tpAniSirGlobal pMac, tpRrmNeighborReportDesc pNeighborReportDesc)
{
   tpRrmSMEContext pSmeRrmContext = &pMac->rrm.rrmSmeContext;
   tListElem       *pEntry;
   tRrmNeighborReportDesc  *pTempNeighborReportDesc;

   if (NULL == pNeighborReportDesc)
   {
       VOS_ASSERT(0);
       return;
   }
   if (NULL == pNeighborReportDesc->pNeighborBssDescription)
   {
       VOS_ASSERT(0);
       return;
   }

   if (csrLLIsListEmpty(&pSmeRrmContext->neighborReportCache, LL_ACCESS_LOCK))
   {
       smsLog(pMac, LOGE, FL("Neighbor report cache is empty.. Adding a entry now"));
        /*                                                             */
       csrLLInsertTail(&pSmeRrmContext->neighborReportCache, &pNeighborReportDesc->List, LL_ACCESS_LOCK);
       return;
   }
   else
   {
       /*                                                                                         
                                                                                    */
        pEntry = csrLLPeekHead(&pSmeRrmContext->neighborReportCache, LL_ACCESS_LOCK);
        while (pEntry != NULL)
        {
            pTempNeighborReportDesc = GET_BASE_ADDR( pEntry, tRrmNeighborReportDesc, List );
            if (pTempNeighborReportDesc->roamScore < pNeighborReportDesc->roamScore)
                break;
            pEntry = csrLLNext(&pSmeRrmContext->neighborReportCache, pEntry, LL_ACCESS_LOCK);
        } 

        if (pEntry)
            /*                                                                                      */
            csrLLInsertEntry(&pSmeRrmContext->neighborReportCache, pEntry, &pNeighborReportDesc->List, LL_ACCESS_LOCK);
        else
            /*                                                                                            */
            csrLLInsertTail(&pSmeRrmContext->neighborReportCache, &pNeighborReportDesc->List, LL_ACCESS_LOCK);
   }
   return;
}

/*                                                                          
                                                                                 
                                 
  
                                                                              
                                       
                                                                            
  
                                                         
  
     
  
                                                                            */
eHalStatus sme_RrmProcessNeighborReport(tpAniSirGlobal pMac, void *pMsgBuf)
{
   eHalStatus status = eHAL_STATUS_SUCCESS;
   tpSirNeighborReportInd pNeighborRpt = (tpSirNeighborReportInd) pMsgBuf;
   tpRrmNeighborReportDesc  pNeighborReportDesc;
   tANI_U8 i = 0;
   VOS_STATUS vosStatus = VOS_STATUS_SUCCESS;

#ifdef FEATURE_WLAN_CCX
   //                         
   if (csrNeighborRoamIsCCXAssoc(pMac))
   {
       rrmLLPurgeNeighborCache(pMac, 
           &pMac->rrm.rrmSmeContext.neighborReportCache);
   }
#endif

   for (i = 0; i < pNeighborRpt->numNeighborReports; i++)
   {
       pNeighborReportDesc = vos_mem_malloc(sizeof(tRrmNeighborReportDesc));
       if (NULL == pNeighborReportDesc)
       {
           smsLog( pMac, LOGE, "Failed to allocate memory for RRM Neighbor report desc");
           status = eHAL_STATUS_FAILED_ALLOC;
           goto end;
            
       }

       vos_mem_zero(pNeighborReportDesc, sizeof(tRrmNeighborReportDesc));
       pNeighborReportDesc->pNeighborBssDescription = vos_mem_malloc(sizeof(tSirNeighborBssDescription));
       if (NULL == pNeighborReportDesc->pNeighborBssDescription)
       {
           smsLog( pMac, LOGE, "Failed to allocate memory for RRM Neighbor report BSS Description");
           vos_mem_free(pNeighborReportDesc);
           status = eHAL_STATUS_FAILED_ALLOC;
           goto end;
       }
       vos_mem_zero(pNeighborReportDesc->pNeighborBssDescription, sizeof(tSirNeighborBssDescription));
       vos_mem_copy(pNeighborReportDesc->pNeighborBssDescription, &pNeighborRpt->sNeighborBssDescription[i], 
                                                sizeof(tSirNeighborBssDescription));

#if defined WLAN_VOWIFI_DEBUG
       smsLog( pMac, LOGE, "Received neighbor report with Neighbor BSSID: %02x:%02x:%02x:%02x:%02x:%02x ",
                    pNeighborRpt->sNeighborBssDescription[i].bssId[0], 
                    pNeighborRpt->sNeighborBssDescription[i].bssId[1], 
                    pNeighborRpt->sNeighborBssDescription[i].bssId[2], 
                    pNeighborRpt->sNeighborBssDescription[i].bssId[3], 
                    pNeighborRpt->sNeighborBssDescription[i].bssId[4], 
                    pNeighborRpt->sNeighborBssDescription[i].bssId[5]);
#endif

       /*                                                                                                                    */
       rrmCalculateNeighborAPRoamScore(pMac, pNeighborReportDesc);

       /*                                                                     */
       if ( pNeighborReportDesc->roamScore > 0)
       {
          rrmStoreNeighborRptByRoamScore(pMac, pNeighborReportDesc);
       }
       else
       {
           smsLog(pMac, LOGE, FL("Roam score of BSSID  %02x:%02x:%02x:%02x:%02x:%02x is 0, Ignoring.."),
                        pNeighborRpt->sNeighborBssDescription[i].bssId[0],
                        pNeighborRpt->sNeighborBssDescription[i].bssId[1],
                        pNeighborRpt->sNeighborBssDescription[i].bssId[2],
                        pNeighborRpt->sNeighborBssDescription[i].bssId[3],
                        pNeighborRpt->sNeighborBssDescription[i].bssId[4],
                        pNeighborRpt->sNeighborBssDescription[i].bssId[5]);

           vos_mem_free(pNeighborReportDesc->pNeighborBssDescription);
           vos_mem_free(pNeighborReportDesc);
       }
   }
end:  
   
   if (!csrLLCount(&pMac->rrm.rrmSmeContext.neighborReportCache))
      vosStatus = VOS_STATUS_E_FAILURE;
 
   /*                                                                                           */
   rrmIndicateNeighborReportResult(pMac, vosStatus);

   return status;
}
/*                                                                          
                                                                              
                                              
  
                                                              
                                                                       
                                                                              
                                       
                                                                            
  
                                                         
  
     
  
                                                                            */
eHalStatus sme_RrmMsgProcessor( tpAniSirGlobal pMac,  v_U16_t msg_type, 
                                void *pMsgBuf)
{
   VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_INFO_HIGH, 
         FL(" Msg = %d for RRM measurement") , msg_type );

   //                                                              
   switch(msg_type)
   {
      case eWNI_SME_NEIGHBOR_REPORT_IND:
         sme_RrmProcessNeighborReport( pMac, pMsgBuf );
         break;

      case eWNI_SME_BEACON_REPORT_REQ_IND:
         sme_RrmProcessBeaconReportReqInd( pMac, pMsgBuf );
         break;

      default:
         //       
         VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_ERROR, 
               FL("sme_RrmMsgProcessor:unknown msg type = %d"), msg_type);

         break;
   }

   return eHAL_STATUS_SUCCESS;
}

/*                                                                            

                              

                                                                             

                                                                            

                            

                                                  

                

                                                                             */

void rrmIterMeasTimerHandle( v_PVOID_t userData )
{
   tpAniSirGlobal pMac = (tpAniSirGlobal) userData;
#if defined WLAN_VOWIFI_DEBUG
   smsLog( pMac, LOGE, "Randomization timer expired...send on next channel ");
#endif
    //                                  
    sme_RrmIssueScanReq( pMac ); 
}

/*                                                                            
    
                                    
    
                                                                                          
                                                                    
    
                                                  
    
                
    
                                                                           */
    
void rrmNeighborRspTimeoutHandler
( v_PVOID_t userData )
{
   tpAniSirGlobal pMac = (tpAniSirGlobal) userData;
#if defined WLAN_VOWIFI_DEBUG
   smsLog( pMac, LOGE, "Neighbor Response timed out ");
#endif
    rrmIndicateNeighborReportResult(pMac, VOS_STATUS_E_FAILURE);
    return;
}

/*                                                                            

               

            

                                                  

                      

                                         

                                       

                                                                             */

VOS_STATUS rrmOpen (tpAniSirGlobal pMac)

{

   VOS_STATUS vosStatus;
   tpRrmSMEContext pSmeRrmContext = &pMac->rrm.rrmSmeContext;
   eHalStatus   halStatus = eHAL_STATUS_SUCCESS;

   pSmeRrmContext->rrmConfig.maxRandnInterval = 50; //  

   vosStatus = vos_timer_init( &pSmeRrmContext->IterMeasTimer,

                      VOS_TIMER_TYPE_SW,

                      rrmIterMeasTimerHandle,

                      (void*) pMac);

   if (!VOS_IS_STATUS_SUCCESS(vosStatus)) {

       VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_ERROR, "rrmOpen: Fail to init timer");

       return VOS_STATUS_E_FAILURE;
   }

   vosStatus = vos_timer_init( &pSmeRrmContext->neighborReqControlInfo.neighborRspWaitTimer,

                      VOS_TIMER_TYPE_SW,

                      rrmNeighborRspTimeoutHandler,

                      (void*) pMac);

   if (!VOS_IS_STATUS_SUCCESS(vosStatus)) {

       VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_ERROR, "rrmOpen: Fail to init timer");

       return VOS_STATUS_E_FAILURE;
   }

   pSmeRrmContext->neighborReqControlInfo.isNeighborRspPending = eANI_BOOLEAN_FALSE;

   halStatus = csrLLOpen(pMac->hHdd, &pSmeRrmContext->neighborReportCache);
   if (eHAL_STATUS_SUCCESS != halStatus)
   {
       VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_ERROR, "rrmOpen: Fail to open neighbor cache result");
       return VOS_STATUS_E_FAILURE;
   }

   return VOS_STATUS_SUCCESS;
}


/*                                                                            

                

            

                                                  

                      

                                         

                                       

                                                                             */

VOS_STATUS rrmClose (tpAniSirGlobal pMac)

{

   VOS_STATUS vosStatus = VOS_STATUS_SUCCESS;
   tpRrmSMEContext pSmeRrmContext = &pMac->rrm.rrmSmeContext;

   if( VOS_TIMER_STATE_RUNNING == vos_timer_getCurrentState( &pSmeRrmContext->IterMeasTimer ) )
   {
      vosStatus = vos_timer_stop( &pSmeRrmContext->IterMeasTimer );
      if(!VOS_IS_STATUS_SUCCESS(vosStatus))
      {
         VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_ERROR, FL("Timer stop fail") );
      }
   }

   vosStatus = vos_timer_destroy( &pSmeRrmContext->IterMeasTimer );
   if (!VOS_IS_STATUS_SUCCESS(vosStatus))
   {

       VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_ERROR, FL("Fail to destroy timer") );

   }

   if( VOS_TIMER_STATE_RUNNING ==
          vos_timer_getCurrentState( &pSmeRrmContext->neighborReqControlInfo.neighborRspWaitTimer ) )
   {
      vosStatus = vos_timer_stop( &pSmeRrmContext->neighborReqControlInfo.neighborRspWaitTimer );
      if(!VOS_IS_STATUS_SUCCESS(vosStatus))
      {
         VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_FATAL, FL("Timer stop fail") );
      }
   }

   vosStatus = vos_timer_destroy( &pSmeRrmContext->neighborReqControlInfo.neighborRspWaitTimer );
   if (!VOS_IS_STATUS_SUCCESS(vosStatus))
   {
       VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_FATAL, FL("Fail to destroy timer") );

   }

   rrmLLPurgeNeighborCache(pMac, &pSmeRrmContext->neighborReportCache);

   csrLLClose(&pSmeRrmContext->neighborReportCache);

   return vosStatus;

}




/*                                                                            

                

              

                                                  

                      

                                                                             */

VOS_STATUS rrmReady (tpAniSirGlobal pMac)

{

    return VOS_STATUS_SUCCESS;
}

/*                                                                            

                                   
              

                                                  
                                                    

                      

                                                                             */
VOS_STATUS rrmChangeDefaultConfigParam(tpAniSirGlobal pMac, tpRrmConfigParam pRrmConfig)
{
   vos_mem_copy( &pMac->rrm.rrmSmeContext.rrmConfig, pRrmConfig, sizeof( tRrmConfigParam ) ); 

   return VOS_STATUS_SUCCESS;
}

/*                                                                            
    
                                                 
    
                                                                                       

                                                  
    
                
    
                                                                           */
tRrmNeighborReportDesc* smeRrmGetFirstBssEntryFromNeighborCache( tpAniSirGlobal pMac)
{
   tListElem *pEntry;
   tRrmNeighborReportDesc *pTempBssEntry = NULL;
   tpRrmSMEContext pSmeRrmContext = &pMac->rrm.rrmSmeContext;


   pEntry = csrLLPeekHead( &pSmeRrmContext->neighborReportCache, LL_ACCESS_LOCK );

   if(!pEntry || !csrLLCount(&pSmeRrmContext->neighborReportCache))
   {
      //          
      smsLog(pMac, LOGW, FL("List empty"));
      return NULL;
   }

   pTempBssEntry = GET_BASE_ADDR( pEntry, tRrmNeighborReportDesc, List );

   return pTempBssEntry;
}

/*                                                                            
    
                                                
    
                                                                             
                                            

                                                  
    
                
    
                                                                           */
tRrmNeighborReportDesc* smeRrmGetNextBssEntryFromNeighborCache( tpAniSirGlobal pMac, 
                                                        tpRrmNeighborReportDesc pBssEntry)
{
   tListElem *pEntry;
   tRrmNeighborReportDesc *pTempBssEntry = NULL;

   pEntry = csrLLNext(&pMac->rrm.rrmSmeContext.neighborReportCache, &pBssEntry->List, LL_ACCESS_LOCK);

   if(!pEntry)
   {
      //          
      smsLog(pMac, LOGW, FL("List empty"));
      return NULL;
   }

   pTempBssEntry = GET_BASE_ADDR( pEntry, tRrmNeighborReportDesc, List );

   return pTempBssEntry;
}

#ifdef FEATURE_WLAN_CCX
void csrCcxSendAdjacentApRepMsg(tpAniSirGlobal pMac, tCsrRoamSession *pSession)
{
   tpSirAdjacentApRepInd pAdjRep;
   tANI_U16 length;
   tANI_U32 roamTS2;
   
   smsLog( pMac, LOG1, "Adjacent AP Report Msg to PE");

   length = sizeof(tSirAdjacentApRepInd );
   pAdjRep = vos_mem_malloc ( length );

   if ( NULL == pAdjRep )
   {
       smsLog( pMac, LOGP, "Unable to allocate memory for Adjacent AP report");
       return;
   }

   vos_mem_zero( pAdjRep, length );
   pAdjRep->messageType = eWNI_SME_CCX_ADJACENT_AP_REPORT;
   pAdjRep->length = length;
   pAdjRep->channelNum = pSession->prevOpChannel;
   vos_mem_copy( pAdjRep->bssid, &pSession->connectedProfile.bssid, sizeof(tSirMacAddr) );
   vos_mem_copy( pAdjRep->prevApMacAddr, &pSession->prevApBssid, sizeof(tSirMacAddr) );
   vos_mem_copy( &pAdjRep->prevApSSID, &pSession->prevApSSID, sizeof(tSirMacSSid) );
   roamTS2 = vos_timer_get_system_time();
   pAdjRep->tsmRoamdelay = roamTS2 - pSession->roamTS1;
   pAdjRep->roamReason =SIR_CCX_ASSOC_REASON_UNSPECIFIED;
   pAdjRep->clientDissSecs =(pAdjRep->tsmRoamdelay/1000);

   palSendMBMessage(pMac->hHdd, pAdjRep);

   return;
}
#endif   /*                  */
#endif
