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

/** ------------------------------------------------------------------------- * 
    ------------------------------------------------------------------------- *  

  
    \file csrApiScan.c
  
    Implementation for the Common Scan interfaces.
  
    Copyright (C) 2006 Airgo Networks, Incorporated 
   ========================================================================== */

#include "aniGlobal.h"

#include "palApi.h"
#include "csrInsideApi.h"
#include "smeInside.h"
#include "smsDebug.h"

#include "csrSupport.h"
#include "wlan_qct_tl.h"

#include "vos_diag_core_log.h"
#include "vos_diag_core_event.h"

#include "vos_nvitem.h"
#include "wlan_qct_wda.h"
                                                                     
#define CSR_VALIDATE_LIST  //                                                                   

#ifdef CSR_VALIDATE_LIST
tDblLinkList *g_pchannelPowerInfoList24 = NULL, * g_pchannelPowerInfoList5 = NULL;
tpAniSirGlobal g_pMac = NULL;
#endif

/*                         
                                                                                                                   
                                                                      
                                                                                                                                
                                                                                                                                          
                                                                                                                                             
                                        
*/
                                                                     
#define HIDDEN_TIMER (1*60*1000)                                                                     
#define CSR_SCAN_RESULT_RSSI_WEIGHT     80 //                                                            
                                                                     
/*                                                                           
                                         
                                                                           */  
#define CSR_SCAN_PER_FILTER_FRAC 100
                                                                     
/*                                                                           
                                          
                                                                           */  
#define CSR_SCAN_RSSI_FILTER_FRAC 100

/*                                                                           
                                                                       
                                                                        
                    
                                                                            
                                                                           */
#define CSR_SCAN_MAX_SCORE_VAL 0xFF
#define CSR_SCAN_MIN_SCORE_VAL 0x0
#define CSR_SCAN_HANDOFF_DELTA 10
#define MAX_ACTIVE_SCAN_FOR_ONE_CHANNEL 140
#define MIN_ACTIVE_SCAN_FOR_ONE_CHANNEL 120
#define CSR_SCAN_OVERALL_SCORE( rssi ) \
  ( rssi < CSR_SCAN_MAX_SCORE_VAL )\
   ? (CSR_SCAN_MAX_SCORE_VAL-rssi) : CSR_SCAN_MIN_SCORE_VAL
                                                                     

#define CSR_SCAN_IS_OVER_BSS_LIMIT(pMac)  \
   ( (pMac)->scan.nBssLimit <= (csrLLCount(&(pMac)->scan.scanResultList)) )

//                                                                              
//                            
extern tSirRetStatus wlan_cfgGetStr(tpAniSirGlobal, tANI_U16, tANI_U8*, tANI_U32*);
                                                                     
void csrScanGetResultTimerHandler(void *);
void csrScanResultAgingTimerHandler(void *pv);
void csrScanIdleScanTimerHandler(void *);
#ifdef WLAN_AP_STA_CONCURRENCY
static void csrStaApConcTimerHandler(void *);
#endif
tANI_BOOLEAN csrIsSupportedChannel(tpAniSirGlobal pMac, tANI_U8 channelId);
eHalStatus csrScanChannels( tpAniSirGlobal pMac, tSmeCmd *pCommand );
void csrSetCfgValidChannelList( tpAniSirGlobal pMac, tANI_U8 *pChannelList, tANI_U8 NumChannels );
void csrSaveTxPowerToCfg( tpAniSirGlobal pMac, tDblLinkList *pList, tANI_U32 cfgId );
void csrSetCfgCountryCode( tpAniSirGlobal pMac, tANI_U8 *countryCode );
void csrPurgeChannelPower( tpAniSirGlobal pMac, tDblLinkList *pChannelList );
//                                                                         
eHalStatus csrSetCfgBackgroundScanPeriod(tpAniSirGlobal pMac, tANI_U32 bgPeriod);
eHalStatus csrProcessSetBGScanParam(tpAniSirGlobal pMac, tSmeCmd *pCommand);
void csrReleaseScanCommand(tpAniSirGlobal pMac, tSmeCmd *pCommand, eCsrScanStatus scanStatus);
static tANI_BOOLEAN csrScanValidateScanResult( tpAniSirGlobal pMac, tANI_U8 *pChannels, 
                                               tANI_U8 numChn, tSirBssDescription *pBssDesc, 
                                               tDot11fBeaconIEs **ppIes );
eHalStatus csrSetBGScanChannelList( tpAniSirGlobal pMac, tANI_U8 *pAdjustChannels, tANI_U8 NumAdjustChannels);
void csrReleaseCmdSingle(tpAniSirGlobal pMac, tSmeCmd *pCommand);
tANI_BOOLEAN csrRoamIsValidChannel( tpAniSirGlobal pMac, tANI_U8 channel );
void csrPruneChannelListForMode( tpAniSirGlobal pMac, tCsrChannel *pChannelList );

#define CSR_IS_SOCIAL_CHANNEL(channel) (((channel) == 1) || ((channel) == 6) || ((channel) == 11) )


//                                         
void csrFreeScanResultEntry( tpAniSirGlobal pMac, tCsrScanResult *pResult )
{
    if( NULL != pResult->Result.pvIes )
    {
        palFreeMemory( pMac->hHdd, pResult->Result.pvIes );
    }
    palFreeMemory(pMac->hHdd, pResult);
}


static eHalStatus csrLLScanPurgeResult(tpAniSirGlobal pMac, tDblLinkList *pList)
{
    eHalStatus status = eHAL_STATUS_SUCCESS;
    tListElem *pEntry;
    tCsrScanResult *pBssDesc;
    
    csrLLLock(pList);
    
    while((pEntry = csrLLRemoveHead(pList, LL_ACCESS_NOLOCK)) != NULL)
    {
        pBssDesc = GET_BASE_ADDR( pEntry, tCsrScanResult, Link );
        csrFreeScanResultEntry( pMac, pBssDesc );
    }
    
    csrLLUnlock(pList);   
     
    return (status);
}


int csrCheckValidateLists(void * dest, const void *src, v_SIZE_t num, int idx)
{
#ifdef CSR_VALIDATE_LIST

    int ii = 1;

    if( (NULL == g_pMac) || (!g_pMac->scan.fValidateList ) )
    {
        return ii;
    }
    if(g_pchannelPowerInfoList24)
    {
        //              
        tListElem *pElem, *pHead;
        int count;
        
        count = (int)(g_pchannelPowerInfoList24->Count);
        pHead = &g_pchannelPowerInfoList24->ListHead;
        pElem = pHead->next;
        if((tANI_U32)(pHead->next) > 0x00010000) //                                        
        {
            //                                             
            while(pElem->next != pHead)
            {
                if((tANI_U32)(pElem->next) > 0x00010000)
                {
                    pElem = pElem->next;
                    VOS_ASSERT(count > 0);
                    count--;
                }
                else
                {
                    VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_FATAL, 
                         " %d Detect 1 list(0x%X) error Head(0x%X) next(0x%X) Count %d, dest(0x%X) src(0x%X) numBytes(%d)",
                         idx, (unsigned int)g_pchannelPowerInfoList24, (unsigned int)pHead, 
                        (unsigned int)(pHead->next), (int)g_pchannelPowerInfoList24->Count, 
                        (unsigned int)dest, (unsigned int)src, (int)num);
                    VOS_ASSERT(0);
                    ii = 0;
                    break;
                }
            }
        }
        else
        {
            //        
            VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_FATAL, " %d Detect list(0x%X) error Head(0x%X) next(0x%X) Count %d, dest(0x%X) src(0x%X) numBytes(%d)", 
                idx, (unsigned int)g_pchannelPowerInfoList24, (unsigned int)pHead, 
                (unsigned int)(pHead->next), (int)g_pchannelPowerInfoList24->Count, 
                (unsigned int)dest, (unsigned int)src, (int)num);
            VOS_ASSERT(0);
            ii = 0;
        }
    }
    else
    {
        //       
        ii = 1;
    }


    return ii;

#else
    return 1;
#endif //                        
}


eHalStatus csrScanOpen( tpAniSirGlobal pMac )
{
    eHalStatus status;
    
    do
    {
        csrLLOpen(pMac->hHdd, &pMac->scan.scanResultList);
        csrLLOpen(pMac->hHdd, &pMac->scan.tempScanResults);
        csrLLOpen(pMac->hHdd, &pMac->scan.channelPowerInfoList24);
        csrLLOpen(pMac->hHdd, &pMac->scan.channelPowerInfoList5G);
#ifdef WLAN_AP_STA_CONCURRENCY
        csrLLOpen(pMac->hHdd, &pMac->scan.scanCmdPendingList);
#endif
#ifdef CSR_VALIDATE_LIST
        g_pchannelPowerInfoList5 = &pMac->scan.channelPowerInfoList5G;
        g_pMac = pMac;
        g_pchannelPowerInfoList24 = &pMac->scan.channelPowerInfoList24;
#endif
        pMac->scan.fFullScanIssued = eANI_BOOLEAN_FALSE;
        pMac->scan.nBssLimit = CSR_MAX_BSS_SUPPORT;
        status = palTimerAlloc(pMac->hHdd, &pMac->scan.hTimerGetResult, csrScanGetResultTimerHandler, pMac);
        if(!HAL_STATUS_SUCCESS(status))
        {
            smsLog(pMac, LOGE, FL("cannot allocate memory for getResult timer\n"));
            break;
        }
#ifdef WLAN_AP_STA_CONCURRENCY
        status = palTimerAlloc(pMac->hHdd, &pMac->scan.hTimerStaApConcTimer, csrStaApConcTimerHandler, pMac);
        if(!HAL_STATUS_SUCCESS(status))
        {
            smsLog(pMac, LOGE, FL("cannot allocate memory for hTimerStaApConcTimer timer\n"));
            break;
        }
#endif        
        status = palTimerAlloc(pMac->hHdd, &pMac->scan.hTimerIdleScan, csrScanIdleScanTimerHandler, pMac);
        if(!HAL_STATUS_SUCCESS(status))
        {
            smsLog(pMac, LOGE, FL("cannot allocate memory for idleScan timer\n"));
            break;
        }
        status = palTimerAlloc(pMac->hHdd, &pMac->scan.hTimerResultAging, csrScanResultAgingTimerHandler, pMac);
        if(!HAL_STATUS_SUCCESS(status))
        {
            smsLog(pMac, LOGE, FL("cannot allocate memory for ResultAging timer\n"));
            break;
        }
    }while(0);
    
    return (status);
}


eHalStatus csrScanClose( tpAniSirGlobal pMac )
{
#ifdef CSR_VALIDATE_LIST
    g_pchannelPowerInfoList24 = NULL;
    g_pchannelPowerInfoList5 = NULL;
    g_pMac = NULL;
#endif
    csrLLScanPurgeResult(pMac, &pMac->scan.tempScanResults);
    csrLLScanPurgeResult(pMac, &pMac->scan.scanResultList);
#ifdef WLAN_AP_STA_CONCURRENCY
    csrLLScanPurgeResult(pMac, &pMac->scan.scanCmdPendingList);
#endif
    csrLLClose(&pMac->scan.scanResultList);
    csrLLClose(&pMac->scan.tempScanResults);
#ifdef WLAN_AP_STA_CONCURRENCY
    csrLLClose(&pMac->scan.scanCmdPendingList);
#endif
    csrPurgeChannelPower(pMac, &pMac->scan.channelPowerInfoList24);
    csrPurgeChannelPower(pMac, &pMac->scan.channelPowerInfoList5G);
    csrLLClose(&pMac->scan.channelPowerInfoList24);
    csrLLClose(&pMac->scan.channelPowerInfoList5G);
    csrScanDisable(pMac);
    palTimerFree(pMac->hHdd, pMac->scan.hTimerResultAging);
    palTimerFree(pMac->hHdd, pMac->scan.hTimerGetResult);
#ifdef WLAN_AP_STA_CONCURRENCY
    palTimerFree(pMac->hHdd, pMac->scan.hTimerStaApConcTimer);
#endif
    palTimerFree(pMac->hHdd, pMac->scan.hTimerIdleScan);
    return eHAL_STATUS_SUCCESS;
}


eHalStatus csrScanEnable( tpAniSirGlobal pMac )
{
    
    pMac->scan.fScanEnable = eANI_BOOLEAN_TRUE;
    pMac->scan.fRestartIdleScan = eANI_BOOLEAN_TRUE;
    
    return eHAL_STATUS_SUCCESS;
}


eHalStatus csrScanDisable( tpAniSirGlobal pMac )
{
    
    csrScanStopTimers(pMac);
    pMac->scan.fScanEnable = eANI_BOOLEAN_FALSE;
    
    return eHAL_STATUS_SUCCESS;
}


#ifdef WLAN_AP_STA_CONCURRENCY
//                                                                          
eHalStatus csrQueueScanRequest( tpAniSirGlobal pMac, tSmeCmd *pScanCmd )
{
    eHalStatus status = eHAL_STATUS_SUCCESS;

    tANI_BOOLEAN fNoCmdPending;
    tSmeCmd *pQueueScanCmd=NULL;
    tSmeCmd *pSendScanCmd=NULL;

    if (vos_get_concurrency_mode() == VOS_STA_SAP) //                                        
    {

        tCsrScanRequest scanReq;
        tANI_U8 numChn = pScanCmd->u.scanCmd.u.scanRequest.ChannelInfo.numOfChannels;
        tCsrChannelInfo *pChnInfo = &scanReq.ChannelInfo;
        tANI_U8    channelToScan[WNI_CFG_VALID_CHANNEL_LIST_LEN];
        tANI_U8    i = 0;
        tANI_BOOLEAN bMemAlloc = eANI_BOOLEAN_FALSE;

        if (numChn == 0)
        {

            numChn = pMac->scan.baseChannels.numChannels;
             
             status = palAllocateMemory( pMac->hHdd, (void **)&pScanCmd->u.scanCmd.u.scanRequest.ChannelInfo.ChannelList, numChn );
             if( !HAL_STATUS_SUCCESS( status ) )
             {
                 smsLog( pMac, LOGE, FL(" Failed to get memory for channel list \n") );
                 return eHAL_STATUS_FAILURE;
             }
             bMemAlloc = eANI_BOOLEAN_TRUE;
             status = palCopyMemory( pMac->hHdd, pScanCmd->u.scanCmd.u.scanRequest.ChannelInfo.ChannelList, 
                         pMac->scan.baseChannels.channelList, numChn );
             if( !HAL_STATUS_SUCCESS( status ) )
             {
                 palFreeMemory( pMac->hHdd, pScanCmd->u.scanCmd.u.scanRequest.ChannelInfo.ChannelList );
                 pScanCmd->u.scanCmd.u.scanRequest.ChannelInfo.ChannelList = NULL;
                 smsLog( pMac, LOGE, FL(" Failed to copy memory to channel list \n") );
                 return eHAL_STATUS_FAILURE;
             }
             pScanCmd->u.scanCmd.u.scanRequest.ChannelInfo.numOfChannels = numChn;
         }

         //                                                                                    
         //                                                                                      
         for  (i=0; i < 2; i++)
         {   //                             
             //                                                                 
             //                                                                     
            
            if (1 == numChn)
            {
                pSendScanCmd = pScanCmd;
                pSendScanCmd->u.scanCmd.u.scanRequest.ChannelInfo.numOfChannels = 1;
                pSendScanCmd->u.scanCmd.u.scanRequest.scanType = eSIR_ACTIVE_SCAN;
                pSendScanCmd->u.scanCmd.u.scanRequest.maxChnTime = 
                    CSR_MIN(pSendScanCmd->u.scanCmd.u.scanRequest.maxChnTime,CSR_ACTIVE_MAX_CHANNEL_TIME_CONC);
                pSendScanCmd->u.scanCmd.u.scanRequest.minChnTime = 
                    CSR_MIN(pSendScanCmd->u.scanCmd.u.scanRequest.minChnTime, CSR_ACTIVE_MIN_CHANNEL_TIME_CONC);
                if (i != 0)
                { //                                                                                                          
                  //                                           
                   pSendScanCmd->u.scanCmd.callback = NULL;
                }
                break; //                                                                                     
            
            } else { //                              
            
                palZeroMemory(pMac->hHdd, &scanReq, sizeof(tCsrScanRequest));

                pQueueScanCmd = csrGetCommandBuffer(pMac); //                                          
                if (!pQueueScanCmd)
                {
                    if (bMemAlloc)
                    {
                        palFreeMemory( pMac->hHdd, pScanCmd->u.scanCmd.u.scanRequest.ChannelInfo.ChannelList );
                        pScanCmd->u.scanCmd.u.scanRequest.ChannelInfo.ChannelList = NULL;
                 
                    }
                    smsLog( pMac, LOGE, FL(" Failed to get Queue command buffer\n") );
                    return eHAL_STATUS_FAILURE;
                }
                pQueueScanCmd->command = pScanCmd->command; 
                pQueueScanCmd->sessionId = pScanCmd->sessionId;
                pQueueScanCmd->u.scanCmd.callback = pScanCmd->u.scanCmd.callback;
                pQueueScanCmd->u.scanCmd.pContext = pScanCmd->u.scanCmd.pContext;
                pQueueScanCmd->u.scanCmd.reason = pScanCmd->u.scanCmd.reason;
                pQueueScanCmd->u.scanCmd.scanID = pMac->scan.nextScanID++; //                  

                pChnInfo->numOfChannels = pScanCmd->u.scanCmd.u.scanRequest.ChannelInfo.numOfChannels - 1;

                VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_WARN, 
                   FL(" &channelToScan %0x pScanCmd(0x%X) pScanCmd->u.scanCmd.u.scanRequest.ChannelInfo.ChannelList(0x%X)numChn(%d)"),
                   &channelToScan[0], (unsigned int)pScanCmd, 
                   (unsigned int)pScanCmd->u.scanCmd.u.scanRequest.ChannelInfo.ChannelList, numChn);
              
                palCopyMemory(pMac->hHdd, &channelToScan[0], &pScanCmd->u.scanCmd.u.scanRequest.ChannelInfo.ChannelList[1], 
                              pChnInfo->numOfChannels * sizeof(tANI_U8));

                pChnInfo->ChannelList = &channelToScan[0];
              

                scanReq.BSSType = eCSR_BSS_TYPE_ANY;
                //                                                
                scanReq.scanType = eSIR_ACTIVE_SCAN;
                scanReq.maxChnTime = CSR_ACTIVE_MAX_CHANNEL_TIME_CONC;
                scanReq.minChnTime = CSR_ACTIVE_MIN_CHANNEL_TIME_CONC;

                status = csrScanCopyRequest(pMac, &pQueueScanCmd->u.scanCmd.u.scanRequest, &scanReq);
                if(!HAL_STATUS_SUCCESS(status))
                {
                    if (bMemAlloc)
                    {
                        palFreeMemory( pMac->hHdd, pScanCmd->u.scanCmd.u.scanRequest.ChannelInfo.ChannelList );
                        pScanCmd->u.scanCmd.u.scanRequest.ChannelInfo.ChannelList = NULL;
                 
                    }
                    smsLog( pMac, LOGE, FL(" Failed to get copy csrScanRequest = %d\n"), status );
                    return eHAL_STATUS_FAILURE;
                }       
                numChn = 1; //                                                                 
            }  

         }

         fNoCmdPending = csrLLIsListEmpty( &pMac->scan.scanCmdPendingList, LL_ACCESS_LOCK );

         //                         
         //                                                                      
         //                                                                     
         if( fNoCmdPending )
         {
            
            if (pQueueScanCmd != NULL)
            {            
              csrLLInsertTail( &pMac->scan.scanCmdPendingList, &pQueueScanCmd->Link, LL_ACCESS_LOCK );
            }

            if (pSendScanCmd != NULL)
            {            
                return csrQueueSmeCommand(pMac, pSendScanCmd, eANI_BOOLEAN_FALSE);
            }
         }
         else
         {           
            if (pSendScanCmd != NULL)
            {
                csrLLInsertTail( &pMac->scan.scanCmdPendingList, &pSendScanCmd->Link, LL_ACCESS_LOCK );
            }
            if (pQueueScanCmd != NULL)
            {
                csrLLInsertTail( &pMac->scan.scanCmdPendingList, &pQueueScanCmd->Link, LL_ACCESS_LOCK );
            }
         }

    }
    else
    {  //                   
        return csrQueueSmeCommand(pMac, pScanCmd, eANI_BOOLEAN_FALSE);
    }
    


    return ( status );

}
#endif

/*                                                                            
                           
                                                                
                                     
               
                   
                       
                
                                                                                 */
static void csrScan2GOnyRequest(tpAniSirGlobal pMac,tSmeCmd *pScanCmd, 
                                tCsrScanRequest *pScanRequest)
{
    tANI_U8 index, channelId, channelListSize = 0;
    tANI_U8 channelList2G[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14};
    static tANI_U8 validchannelList[CSR_MAX_2_4_GHZ_SUPPORTED_CHANNELS] = {0};

    VOS_ASSERT(pScanCmd && pScanRequest);

    if (pScanCmd->u.scanCmd.scanID ||
       (eCSR_SCAN_REQUEST_FULL_SCAN != pScanRequest->requestType))
           return;

    //                                              
    for( index = 0; index < ARRAY_SIZE(channelList2G); index++ )
    {
        channelId = channelList2G[index];
        if ( csrIsSupportedChannel( pMac, channelId ) )
        {
            validchannelList[channelListSize++] = channelId;
        }
    }

    pScanRequest->ChannelInfo.numOfChannels = channelListSize;
    pScanRequest->ChannelInfo.ChannelList = validchannelList;
}

eHalStatus csrScanRequest(tpAniSirGlobal pMac, tANI_U16 sessionId, 
              tCsrScanRequest *pScanRequest, tANI_U32 *pScanRequestID, 
              csrScanCompleteCallback callback, void *pContext)
{
    eHalStatus status = eHAL_STATUS_FAILURE;
    tSmeCmd *pScanCmd = NULL;
	eCsrConnectState ConnectState;
    
    do
    {
        if(pMac->scan.fScanEnable)
        {
            pScanCmd = csrGetCommandBuffer(pMac);
            if(pScanCmd)
            {
                palZeroMemory(pMac->hHdd, &pScanCmd->u.scanCmd, sizeof(tScanCmd));
                pScanCmd->command = eSmeCommandScan; 
                pScanCmd->sessionId = sessionId;
                pScanCmd->u.scanCmd.callback = callback;
                pScanCmd->u.scanCmd.pContext = pContext;
                if(eCSR_SCAN_REQUEST_11D_SCAN == pScanRequest->requestType)
                {
                    pScanCmd->u.scanCmd.reason = eCsrScan11d1;
                }
                else if((eCSR_SCAN_REQUEST_FULL_SCAN == pScanRequest->requestType) ||
                        (eCSR_SCAN_P2P_DISCOVERY == pScanRequest->requestType)
#ifdef SOFTAP_CHANNEL_RANGE
                        ||(eCSR_SCAN_SOFTAP_CHANNEL_RANGE == pScanRequest->requestType)
#endif
                 )
                {
                    pScanCmd->u.scanCmd.reason = eCsrScanUserRequest;
                }
                else if(eCSR_SCAN_HO_BG_SCAN == pScanRequest->requestType)
                {
                    pScanCmd->u.scanCmd.reason = eCsrScanBgScan;
                }
                else if(eCSR_SCAN_HO_PROBE_SCAN == pScanRequest->requestType)
                {
                    pScanCmd->u.scanCmd.reason = eCsrScanProbeBss;
                }
#if defined WLAN_FEATURE_P2P
                else if(eCSR_SCAN_P2P_FIND_PEER == pScanRequest->requestType)
                {
                    pScanCmd->u.scanCmd.reason = eCsrScanP2PFindPeer;
                }
#endif                
                else
                {
                    pScanCmd->u.scanCmd.reason = eCsrScanIdleScan;
                }
                if(pScanRequest->minChnTime == 0 && pScanRequest->maxChnTime == 0)
                {
                    //                                                      
                    if(pScanRequest->scanType == eSIR_ACTIVE_SCAN)
                    {
                        pScanRequest->maxChnTime = pMac->roam.configParam.nActiveMaxChnTime;
                        pScanRequest->minChnTime = pMac->roam.configParam.nActiveMinChnTime;
                    }
                    else
                    {
                        pScanRequest->maxChnTime = pMac->roam.configParam.nPassiveMaxChnTime;
                        pScanRequest->minChnTime = pMac->roam.configParam.nPassiveMinChnTime;
                    }
                }
                 /*                                                        
                                                                                                                               
                                                                                                                                       
                                                      
                                                                                                                           
                               */
                status = csrRoamGetConnectState(pMac,sessionId,&ConnectState);
                if(pMac->btc.fA2DPUp && 
                   (eCSR_ASSOC_STATE_TYPE_INFRA_ASSOCIATED != ConnectState) &&
                   (eCSR_ASSOC_STATE_TYPE_IBSS_CONNECTED != ConnectState))
                {
                    pScanRequest->maxChnTime = pScanRequest->maxChnTime << 1;
                    pScanRequest->minChnTime = pScanRequest->minChnTime << 1;
                }  
                //                                 
                pScanCmd->u.scanCmd.scanID = pMac->scan.nextScanID++; //                  
                
                if(pScanRequestID)
                {
                    *pScanRequestID = pScanCmd->u.scanCmd.scanID; 
                }

                //                                                                              
                //                                                                       
                //                                   
                if(((0 == pScanCmd->u.scanCmd.scanID)
                   && (eCSR_SCAN_REQUEST_11D_SCAN != pScanRequest->requestType))
#ifdef SOFTAP_CHANNEL_RANGE
                   && (eCSR_SCAN_SOFTAP_CHANNEL_RANGE != pScanRequest->requestType)
#endif                   
                   && (eANI_BOOLEAN_FALSE == pMac->scan.fEnableBypass11d)
                   )
                {
                    tSmeCmd *p11dScanCmd;
                    tCsrScanRequest scanReq;
                    tCsrChannelInfo *pChnInfo = &scanReq.ChannelInfo;

                    palZeroMemory(pMac->hHdd, &scanReq, sizeof(tCsrScanRequest));

                    p11dScanCmd = csrGetCommandBuffer(pMac);
                    if(p11dScanCmd)
                    {
                        tANI_U32 numChn = pMac->scan.baseChannels.numChannels;

                        palZeroMemory(pMac->hHdd, &p11dScanCmd->u.scanCmd, sizeof(tScanCmd));
                        status = palAllocateMemory( pMac->hHdd, (void **)&pChnInfo->ChannelList, numChn );
                        if( !HAL_STATUS_SUCCESS( status ) )
                        {
                            break;
                        }
                        status = palCopyMemory( pMac->hHdd, pChnInfo->ChannelList, 
                                    pMac->scan.baseChannels.channelList, numChn );
                        if( !HAL_STATUS_SUCCESS( status ) )
                        {
                            palFreeMemory( pMac->hHdd, pChnInfo->ChannelList );
                            pChnInfo->ChannelList = NULL;
                            break;
                        }
                        pChnInfo->numOfChannels = (tANI_U8)numChn;
                        p11dScanCmd->command = eSmeCommandScan;
                        p11dScanCmd->u.scanCmd.callback = NULL;
                        p11dScanCmd->u.scanCmd.pContext = NULL;
                        p11dScanCmd->u.scanCmd.scanID = pMac->scan.nextScanID++;                
                        scanReq.BSSType = eCSR_BSS_TYPE_ANY;

                        if ( csrIs11dSupported(pMac) )
                        {
                            scanReq.scanType = eSIR_PASSIVE_SCAN;
                            scanReq.requestType = eCSR_SCAN_REQUEST_11D_SCAN;
                            p11dScanCmd->u.scanCmd.reason = eCsrScan11d1;
                            scanReq.maxChnTime = pMac->roam.configParam.nPassiveMaxChnTime;
                            scanReq.minChnTime = pMac->roam.configParam.nPassiveMinChnTime;
                        }
                        else
                        {
                            scanReq.scanType = eSIR_ACTIVE_SCAN;
                            scanReq.requestType = eCSR_SCAN_IDLE_MODE_SCAN;
                            p11dScanCmd->u.scanCmd.reason = eCsrScanIdleScan;
                            scanReq.maxChnTime = pMac->roam.configParam.nActiveMaxChnTime;
                            scanReq.minChnTime = pMac->roam.configParam.nActiveMinChnTime;
                        }

                        status = csrScanCopyRequest(pMac, &p11dScanCmd->u.scanCmd.u.scanRequest, &scanReq);
                        //                     
                        palFreeMemory( pMac->hHdd, pChnInfo->ChannelList );

                        if(HAL_STATUS_SUCCESS(status))
                        {
                            //                         
#ifdef WLAN_AP_STA_CONCURRENCY
                            status = csrQueueScanRequest(pMac, p11dScanCmd);
#else
                            status = csrQueueSmeCommand(pMac, p11dScanCmd, eANI_BOOLEAN_FALSE);
#endif                   
                            if( !HAL_STATUS_SUCCESS( status ) )
                            {
                                smsLog( pMac, LOGE, FL(" fail to send message status = %d\n"), status );
                                break;
                            }
                        }
                        else 
                        {
                            break;
                        }
                    }
                    else
                    {
                        //     
                        break;
                    }
                }

                //                                        
                //                                                
                //                    
                if(pMac->scan.fFirstScanOnly2GChnl)
                {
                    csrScan2GOnyRequest(pMac, pScanCmd, pScanRequest);
                }

                status = csrScanCopyRequest(pMac, &pScanCmd->u.scanCmd.u.scanRequest, pScanRequest);
                if(HAL_STATUS_SUCCESS(status))
                {
                    //                         
#ifdef WLAN_AP_STA_CONCURRENCY
                    status = csrQueueScanRequest(pMac,pScanCmd); 
#else
                    status = csrQueueSmeCommand(pMac, pScanCmd, eANI_BOOLEAN_FALSE);                   
#endif
                    if( !HAL_STATUS_SUCCESS( status ) )
                    {
                        smsLog( pMac, LOGE, FL(" fail to send message status = %d\n"), status );
                        break;
                    }
                }
                else 
                {
                    smsLog( pMac, LOGE, FL(" fail to copy request status = %d\n"), status );
                    break;
                }
            }
            else 
            {
                //         
                break;
            }
        }
    } while(0);
    if(!HAL_STATUS_SUCCESS(status) && pScanCmd)
    {
        if( eCsrScanIdleScan == pScanCmd->u.scanCmd.reason )
        {
            //                                          
            pMac->scan.fRestartIdleScan = eANI_BOOLEAN_TRUE;
        }
        csrReleaseCommandScan(pMac, pScanCmd);
    }

    return (status);
}


eHalStatus csrScanRequestResult(tpAniSirGlobal pMac)
{
    eHalStatus status = eHAL_STATUS_SUCCESS;
    tSmeCmd *pScanCmd;
    
    if(pMac->scan.fScanEnable)
    {
        pScanCmd = csrGetCommandBuffer(pMac);
        if(pScanCmd)
        {
            pScanCmd->command = eSmeCommandScan;
            palZeroMemory(pMac->hHdd, &pScanCmd->u.scanCmd, sizeof(tScanCmd));
            pScanCmd->u.scanCmd.callback = NULL;
            pScanCmd->u.scanCmd.pContext = NULL;
            pScanCmd->u.scanCmd.reason = eCsrScanGetResult;
            //                                 
            pScanCmd->u.scanCmd.scanID = pMac->scan.nextScanID++; //                  
            status = csrQueueSmeCommand(pMac, pScanCmd, eANI_BOOLEAN_FALSE);
            if( !HAL_STATUS_SUCCESS( status ) )
            {
                smsLog( pMac, LOGE, FL(" fail to send message status = %d\n"), status );
                csrReleaseCommandScan(pMac, pScanCmd);
            }
        }
        else 
        {
            //         
            smsLog(pMac, LOGE, FL("can not obtain a common buffer\n"));
            status = eHAL_STATUS_RESOURCES;
        }
    }
    
    return (status);
}


eHalStatus csrScanAllChannels(tpAniSirGlobal pMac, eCsrRequestType reqType)
{
    eHalStatus status = eHAL_STATUS_SUCCESS;
    tANI_U32 scanId;
    tCsrScanRequest scanReq;

    palZeroMemory(pMac->hHdd, &scanReq, sizeof(tCsrScanRequest));
    scanReq.BSSType = eCSR_BSS_TYPE_ANY;
    scanReq.scanType = eSIR_ACTIVE_SCAN;
    scanReq.requestType = reqType;
    scanReq.maxChnTime = pMac->roam.configParam.nActiveMaxChnTime;
    scanReq.minChnTime = pMac->roam.configParam.nActiveMinChnTime;
    //                             
    //                                                              
    status = csrScanRequest(pMac, CSR_SESSION_ID_INVALID, &scanReq, 
                            &scanId, NULL, NULL);

    return (status);
}




eHalStatus csrIssueRoamAfterLostlinkScan(tpAniSirGlobal pMac, tANI_U32 sessionId, eCsrRoamReason reason)
{
    eHalStatus status = eHAL_STATUS_FAILURE;
    tScanResultHandle hBSSList = NULL;
    tCsrScanResultFilter *pScanFilter = NULL;
    tANI_U32 roamId = 0;
    tCsrRoamProfile *pProfile = NULL;
    tCsrRoamSession *pSession = CSR_GET_SESSION( pMac, sessionId );

    if(!pSession)
    {
        smsLog(pMac, LOGE, FL("  session %d not found "), sessionId);
        return eHAL_STATUS_FAILURE;
    }

    do
    {
        smsLog(pMac, LOG1, " csrIssueRoamAfterLostlinkScan called\n");
        if(pSession->fCancelRoaming)
        {
            smsLog(pMac, LOGW, " lostlink roaming is cancelled\n");
            csrScanStartIdleScan(pMac);
            status = eHAL_STATUS_SUCCESS;
            break;
        }
        //                                         
        status = palAllocateMemory(pMac->hHdd, (void **)&pScanFilter, sizeof(tCsrScanResultFilter));
        if(!HAL_STATUS_SUCCESS(status))
            break;
        palZeroMemory(pMac->hHdd, pScanFilter, sizeof(tCsrScanResultFilter));
        if(NULL == pSession->pCurRoamProfile)
        {
            pScanFilter->EncryptionType.numEntries = 1;
            pScanFilter->EncryptionType.encryptionType[0] = eCSR_ENCRYPT_TYPE_NONE;
        }
        else
        {
            //                                                                                            
            status = palAllocateMemory(pMac->hHdd, (void **)&pProfile, sizeof(tCsrRoamProfile));
            if(!HAL_STATUS_SUCCESS(status))
                break;
            palZeroMemory(pMac->hHdd, pProfile, sizeof(tCsrRoamProfile));
            status = csrRoamCopyProfile(pMac, pProfile, pSession->pCurRoamProfile);
            if(!HAL_STATUS_SUCCESS(status))
                break;
            status = csrRoamPrepareFilterFromProfile(pMac, pProfile, pScanFilter);
        }//                 
        roamId = GET_NEXT_ROAM_ID(&pMac->roam);
        if(HAL_STATUS_SUCCESS(status))
        {
            status = csrScanGetResult(pMac, pScanFilter, &hBSSList);
            if(HAL_STATUS_SUCCESS(status))
            {
                if(eCsrLostLink1 == reason)
                {
                    //                                                                        
                    csrMoveBssToHeadFromBSSID(pMac, &pSession->connectedProfile.bssid, hBSSList);
                }
                status = csrRoamIssueConnect(pMac, sessionId, pProfile, hBSSList, reason, 
                                                roamId, eANI_BOOLEAN_TRUE, eANI_BOOLEAN_TRUE);
                if(!HAL_STATUS_SUCCESS(status))
                {
                    csrScanResultPurge(pMac, hBSSList);
                }
            }//                
        }
    }while(0);
    if(pScanFilter)
    {
        //                                                   
        csrFreeScanFilter(pMac, pScanFilter);
        palFreeMemory(pMac->hHdd, pScanFilter);
    }
    if(NULL != pProfile)
    {
        csrReleaseProfile(pMac, pProfile);
        palFreeMemory(pMac->hHdd, (void *)pProfile);
    }

    return (status);
}


eHalStatus csrScanGetScanChnInfo(tpAniSirGlobal pMac, void *callback, void *pContext)
{
    eHalStatus status = eHAL_STATUS_SUCCESS;
    tSmeCmd *pScanCmd;
    
    if(pMac->scan.fScanEnable)
    {
        pScanCmd = csrGetCommandBuffer(pMac);
        if(pScanCmd)
        {
            pScanCmd->command = eSmeCommandScan;
            palZeroMemory(pMac->hHdd, &pScanCmd->u.scanCmd, sizeof(tScanCmd));
            pScanCmd->u.scanCmd.callback = callback;
            pScanCmd->u.scanCmd.pContext = pContext;
            pScanCmd->u.scanCmd.reason = eCsrScanGetScanChnInfo;
            //                                 
            pScanCmd->u.scanCmd.scanID = pMac->scan.nextScanID++; //                  
            status = csrQueueSmeCommand(pMac, pScanCmd, eANI_BOOLEAN_FALSE);
            if( !HAL_STATUS_SUCCESS( status ) )
            {
                smsLog( pMac, LOGE, FL(" fail to send message status = %d\n"), status );
                csrReleaseCommandScan(pMac, pScanCmd);
            }
        }
        else 
        {
            //         
            smsLog(pMac, LOGE, FL("can not obtain a common buffer\n"));
            status = eHAL_STATUS_RESOURCES;
        }
    }
    
    return (status);
}


eHalStatus csrScanHandleFailedLostlink1(tpAniSirGlobal pMac, tANI_U32 sessionId)
{
    eHalStatus status = eHAL_STATUS_FAILURE;
    tCsrRoamSession *pSession = CSR_GET_SESSION( pMac, sessionId );

    if(!pSession)
    {
        smsLog(pMac, LOGE, FL("  session %d not found "), sessionId);
        return eHAL_STATUS_FAILURE;
    }

    smsLog(pMac, LOGW, "  Lostlink scan 1 failed\n");
    if(pSession->fCancelRoaming)
    {
        csrScanStartIdleScan(pMac);
    }
    else if(pSession->pCurRoamProfile)
    {
        //                                                                                                      
        if(pSession->pCurRoamProfile->SSIDs.numOfSSIDs == 0 ||
            pSession->pCurRoamProfile->SSIDs.numOfSSIDs > 1)
        {
            //                  
            status = csrScanRequestLostLink2(pMac, sessionId);
        }
        else if(!pSession->pCurRoamProfile->ChannelInfo.ChannelList || 
                pSession->pCurRoamProfile->ChannelInfo.ChannelList[0] == 0)
        {
            //                             
            status = csrScanRequestLostLink3(pMac, sessionId);
        }
        else
        {
            //                         
            if(csrRoamCompleteRoaming(pMac, sessionId, eANI_BOOLEAN_FALSE, eCSR_ROAM_RESULT_FAILURE))
            {
                csrScanStartIdleScan(pMac);
            }
            status = eHAL_STATUS_SUCCESS;
        }
    }
    else
    {
        status = csrScanRequestLostLink3(pMac, sessionId);
    }

    return (status);    
}



eHalStatus csrScanHandleFailedLostlink2(tpAniSirGlobal pMac, tANI_U32 sessionId)
{
    eHalStatus status = eHAL_STATUS_FAILURE;
    tCsrRoamSession *pSession = CSR_GET_SESSION( pMac, sessionId );

    if(!pSession)
    {
        smsLog(pMac, LOGE, FL("  session %d not found "), sessionId);
        return eHAL_STATUS_FAILURE;
    }

    smsLog(pMac, LOGW, "  Lostlink scan 2 failed\n");
    if(pSession->fCancelRoaming)
    {
        csrScanStartIdleScan(pMac);
    }
    else if(!pSession->pCurRoamProfile || !pSession->pCurRoamProfile->ChannelInfo.ChannelList || 
                pSession->pCurRoamProfile->ChannelInfo.ChannelList[0] == 0)
    {
        //                  
        status = csrScanRequestLostLink3(pMac, sessionId);
    }
    else
    {
        //                         
        if(csrRoamCompleteRoaming(pMac, sessionId, eANI_BOOLEAN_FALSE, eCSR_ROAM_RESULT_FAILURE))
        {
            csrScanStartIdleScan(pMac);
        }
    }

    return (status);    
}



eHalStatus csrScanHandleFailedLostlink3(tpAniSirGlobal pMac, tANI_U32 sessionId)
{
    eHalStatus status = eHAL_STATUS_SUCCESS;

    smsLog(pMac, LOGW, "  Lostlink scan 3 failed\n");
    if(eANI_BOOLEAN_TRUE == csrRoamCompleteRoaming(pMac, sessionId, eANI_BOOLEAN_FALSE, eCSR_ROAM_RESULT_FAILURE))
    {
        //                         
        csrScanStartIdleScan(pMac);
    }
    
    return (status);    
}




//                                                                                                 
//                                                            
eHalStatus csrScanRequestLostLink1( tpAniSirGlobal pMac, tANI_U32 sessionId )
{
    eHalStatus status = eHAL_STATUS_SUCCESS;
    tSmeCmd *pCommand = NULL;
    tANI_U8 bAddr[6] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
    tCsrScanResultFilter *pScanFilter = NULL;
    tScanResultHandle hBSSList = NULL;
    tCsrScanResultInfo *pScanResult = NULL;
    tCsrRoamSession *pSession = CSR_GET_SESSION( pMac, sessionId );

    if(!pSession)
    {
        smsLog(pMac, LOGE, FL("  session %d not found "), sessionId);
        return eHAL_STATUS_FAILURE;
    }

    smsLog(pMac, LOGW, FL(" called\n"));
    do
    {
        pCommand = csrGetCommandBuffer(pMac);
        if(!pCommand)
        {
            status = eHAL_STATUS_RESOURCES;
            break;
        }
        palZeroMemory(pMac->hHdd, &pCommand->u.scanCmd, sizeof(tScanCmd));
        pCommand->command = eSmeCommandScan;
        pCommand->sessionId = (tANI_U8)sessionId;
        pCommand->u.scanCmd.reason = eCsrScanLostLink1;
        pCommand->u.scanCmd.callback = NULL;
        pCommand->u.scanCmd.pContext = NULL;
        pCommand->u.scanCmd.u.scanRequest.maxChnTime = pMac->roam.configParam.nActiveMaxChnTime;
        pCommand->u.scanCmd.u.scanRequest.minChnTime = pMac->roam.configParam.nActiveMinChnTime;
        pCommand->u.scanCmd.u.scanRequest.scanType = eSIR_ACTIVE_SCAN;
        if(pSession->connectedProfile.SSID.length)
        {
            status = palAllocateMemory(pMac->hHdd, (void **)&pCommand->u.scanCmd.u.scanRequest.SSIDs.SSIDList, sizeof(tCsrSSIDInfo));
            if(!HAL_STATUS_SUCCESS(status))
            {
                break;
            }
            pCommand->u.scanCmd.u.scanRequest.SSIDs.numOfSSIDs = 1;
            palCopyMemory(pMac->hHdd, &pCommand->u.scanCmd.u.scanRequest.SSIDs.SSIDList[0].SSID, 
                                &pSession->connectedProfile.SSID, sizeof(tSirMacSSid));
        }
        else
        {
            pCommand->u.scanCmd.u.scanRequest.SSIDs.numOfSSIDs = 0;
        }
        if(pSession->pCurRoamProfile)
        {
            status = palAllocateMemory(pMac->hHdd, (void **)&pScanFilter, sizeof(tCsrScanResultFilter));
            if(!HAL_STATUS_SUCCESS(status))
            {
                break;
            }
            palZeroMemory(pMac->hHdd, pScanFilter, sizeof(tCsrScanResultFilter));
            status = csrRoamPrepareFilterFromProfile(pMac, pSession->pCurRoamProfile, pScanFilter);
            if(!HAL_STATUS_SUCCESS(status))
            {
                break;
            }
            //                                                                                                   
            //                                                                                                
            if(HAL_STATUS_SUCCESS((csrScanGetResult(pMac, pScanFilter, &hBSSList))) && hBSSList)
            {
                tANI_U8 i, nChn = 0;
                status = palAllocateMemory(pMac->hHdd, (void **)&pCommand->u.scanCmd.u.scanRequest.ChannelInfo.ChannelList,
                            WNI_CFG_VALID_CHANNEL_LIST_LEN);
                if(!HAL_STATUS_SUCCESS(status))
                {
                    break;
                }
                while(((pScanResult = csrScanResultGetNext(pMac, hBSSList)) != NULL) &&
                    nChn < WNI_CFG_VALID_CHANNEL_LIST_LEN)
                {
                    for(i = 0; i < nChn; i++)
                    {
                        if(pCommand->u.scanCmd.u.scanRequest.ChannelInfo.ChannelList[i] == 
                                        pScanResult->BssDescriptor.channelId)
                        {
                            break;
                        }
                    }
                    if(i == nChn)
                    {
                        pCommand->u.scanCmd.u.scanRequest.ChannelInfo.ChannelList[nChn++] = pScanResult->BssDescriptor.channelId;
                    }
                }
                //                                       
                if(csrRoamIsChannelValid(pMac, pSession->connectedProfile.operationChannel))
                {
                    for(i = 0; i < nChn; i++)
                    {
                        if(pCommand->u.scanCmd.u.scanRequest.ChannelInfo.ChannelList[i] == 
                                        pSession->connectedProfile.operationChannel)
                        {
                            break;
                        }
                    }
                    if(i == nChn)
                    {
                        pCommand->u.scanCmd.u.scanRequest.ChannelInfo.ChannelList[nChn++] = pSession->connectedProfile.operationChannel;
                    }
                }
                pCommand->u.scanCmd.u.scanRequest.ChannelInfo.numOfChannels = nChn;
            }
            else
            {
                if(csrRoamIsChannelValid(pMac, pSession->connectedProfile.operationChannel))
                {
                    status = palAllocateMemory(pMac->hHdd, (void **)&pCommand->u.scanCmd.u.scanRequest.ChannelInfo.ChannelList,
                                1);
                    //                                   
                    if(HAL_STATUS_SUCCESS(status))
                    {
                        pCommand->u.scanCmd.u.scanRequest.ChannelInfo.ChannelList[0] = pSession->connectedProfile.operationChannel;
                        pCommand->u.scanCmd.u.scanRequest.ChannelInfo.numOfChannels = 1;
                    }
                    else 
                    {
                        break;
                    }
                }
            }
        }
        palCopyMemory(pMac->hHdd, &pCommand->u.scanCmd.u.scanRequest.bssid, bAddr, sizeof(tCsrBssid));
        status = csrQueueSmeCommand(pMac, pCommand, eANI_BOOLEAN_FALSE);
        if( !HAL_STATUS_SUCCESS( status ) )
        {
            smsLog( pMac, LOGE, FL(" fail to send message status = %d\n"), status );
            break;
        }
    } while( 0 );

    if(!HAL_STATUS_SUCCESS(status))
    {
        smsLog(pMac, LOGW, " csrScanRequestLostLink1 failed with status %d\n", status);
        if(pCommand)
        {
            csrReleaseCommandScan(pMac, pCommand);
        }
        status = csrScanHandleFailedLostlink1( pMac, sessionId );
    }
    if(pScanFilter)
    {
        csrFreeScanFilter(pMac, pScanFilter);
        palFreeMemory(pMac->hHdd, pScanFilter);
    }
    if(hBSSList)
    {
        csrScanResultPurge(pMac, hBSSList);
    }

    return( status );
}


//                                                                                                           
//                                                                                     
eHalStatus csrScanRequestLostLink2( tpAniSirGlobal pMac, tANI_U32 sessionId )
{
    eHalStatus status = eHAL_STATUS_SUCCESS;
    tANI_U8 bAddr[6] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
    tCsrScanResultFilter *pScanFilter = NULL;
    tScanResultHandle hBSSList = NULL;
    tCsrScanResultInfo *pScanResult = NULL;
    tSmeCmd *pCommand = NULL;
    tCsrRoamSession *pSession = CSR_GET_SESSION( pMac, sessionId );

    if(!pSession)
    {
        smsLog(pMac, LOGE, FL("  session %d not found "), sessionId);
        return eHAL_STATUS_FAILURE;
    }

    smsLog(pMac, LOGW, FL(" called\n"));
    do
    {
        pCommand = csrGetCommandBuffer(pMac);
        if(!pCommand)
        {
            status = eHAL_STATUS_RESOURCES;
            break;
        }
        palZeroMemory(pMac->hHdd, &pCommand->u.scanCmd, sizeof(tScanCmd));
        pCommand->command = eSmeCommandScan;
        pCommand->sessionId = (tANI_U8)sessionId;
        pCommand->u.scanCmd.reason = eCsrScanLostLink2;
        pCommand->u.scanCmd.callback = NULL;
        pCommand->u.scanCmd.pContext = NULL;
        pCommand->u.scanCmd.u.scanRequest.maxChnTime = pMac->roam.configParam.nActiveMaxChnTime;
        pCommand->u.scanCmd.u.scanRequest.minChnTime = pMac->roam.configParam.nActiveMinChnTime;
        pCommand->u.scanCmd.u.scanRequest.scanType = eSIR_ACTIVE_SCAN;
        if(pSession->pCurRoamProfile)
        {
            status = palAllocateMemory(pMac->hHdd, (void **)&pScanFilter, sizeof(tCsrScanResultFilter));
            if(!HAL_STATUS_SUCCESS(status))
            {
                break;
            }
            palZeroMemory(pMac->hHdd, pScanFilter, sizeof(tCsrScanResultFilter));
            status = csrRoamPrepareFilterFromProfile(pMac, pSession->pCurRoamProfile, pScanFilter);
            if(!HAL_STATUS_SUCCESS(status))
            {
                break;
            }
            status = csrScanGetResult(pMac, pScanFilter, &hBSSList);
            if(!HAL_STATUS_SUCCESS(status))
            {
                break;
            }
            if(hBSSList)
            {
                tANI_U8 i, nChn = 0;
                status = palAllocateMemory(pMac->hHdd, (void **)&pCommand->u.scanCmd.u.scanRequest.ChannelInfo.ChannelList,
                            WNI_CFG_VALID_CHANNEL_LIST_LEN);
                if(!HAL_STATUS_SUCCESS(status))
                {
                    break;
                }
                while(((pScanResult = csrScanResultGetNext(pMac, hBSSList)) != NULL) &&
                    nChn < WNI_CFG_VALID_CHANNEL_LIST_LEN)
                {
                    for(i = 0; i < nChn; i++)
                    {
                        if(pCommand->u.scanCmd.u.scanRequest.ChannelInfo.ChannelList[i] == 
                                        pScanResult->BssDescriptor.channelId)
                        {
                            break;
                        }
                    }
                    if(i == nChn)
                    {
                        pCommand->u.scanCmd.u.scanRequest.ChannelInfo.ChannelList[nChn++] = pScanResult->BssDescriptor.channelId;
                    }
                }
                pCommand->u.scanCmd.u.scanRequest.ChannelInfo.numOfChannels = nChn;
            }
        }
        palCopyMemory(pMac->hHdd, &pCommand->u.scanCmd.u.scanRequest.bssid, bAddr, sizeof(tCsrBssid));
        //                                
        status = csrQueueSmeCommand(pMac, pCommand, eANI_BOOLEAN_TRUE);
        if( !HAL_STATUS_SUCCESS( status ) )
        {
            smsLog( pMac, LOGE, FL(" fail to send message status = %d\n"), status );
            break;
        }
    } while( 0 );

    if(!HAL_STATUS_SUCCESS(status))
    {
        smsLog(pMac, LOGW, " csrScanRequestLostLink2 failed with status %d\n", status);
        if(pCommand)
        {
            csrReleaseCommandScan(pMac, pCommand);
        }
        status = csrScanHandleFailedLostlink2( pMac, sessionId );
    }
    if(pScanFilter)
    {
        csrFreeScanFilter(pMac, pScanFilter);
        palFreeMemory(pMac->hHdd, pScanFilter);
    }
    if(hBSSList)
    {
        csrScanResultPurge(pMac, hBSSList);
    }

    return( status );
}


//                                   
eHalStatus csrScanRequestLostLink3( tpAniSirGlobal pMac, tANI_U32 sessionId )
{
    eHalStatus status = eHAL_STATUS_SUCCESS;
    tSmeCmd *pCommand;
    tANI_U8 bAddr[6] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};

    smsLog(pMac, LOGW, FL(" called\n"));
    do
    {
        pCommand = csrGetCommandBuffer(pMac);
        if(!pCommand)
        {
            status = eHAL_STATUS_RESOURCES;
            break;
        }
        palZeroMemory(pMac->hHdd, &pCommand->u.scanCmd, sizeof(tScanCmd));
        pCommand->command = eSmeCommandScan;
        pCommand->sessionId = (tANI_U8)sessionId;
        pCommand->u.scanCmd.reason = eCsrScanLostLink3;
        pCommand->u.scanCmd.callback = NULL;
        pCommand->u.scanCmd.pContext = NULL;
        pCommand->u.scanCmd.u.scanRequest.maxChnTime = pMac->roam.configParam.nActiveMaxChnTime;
        pCommand->u.scanCmd.u.scanRequest.minChnTime = pMac->roam.configParam.nActiveMinChnTime;
        pCommand->u.scanCmd.u.scanRequest.scanType = eSIR_ACTIVE_SCAN;
        palCopyMemory(pMac->hHdd, &pCommand->u.scanCmd.u.scanRequest.bssid, bAddr, sizeof(tCsrBssid));
        //                                
        status = csrQueueSmeCommand(pMac, pCommand, eANI_BOOLEAN_TRUE);
        if( !HAL_STATUS_SUCCESS( status ) )
        {
            smsLog( pMac, LOGE, FL(" fail to send message status = %d\n"), status );
            break;
        }
    } while( 0 );
    if(!HAL_STATUS_SUCCESS(status))
    {
        smsLog(pMac, LOGW, " csrScanRequestLostLink3 failed with status %d\n", status);
        if(csrRoamCompleteRoaming(pMac, sessionId, eANI_BOOLEAN_FALSE, eCSR_ROAM_RESULT_FAILURE))
        {
            csrScanStartIdleScan(pMac);
        }
        if(pCommand)
        {
            csrReleaseCommandScan(pMac, pCommand);
        }
    }

    return( status );
}


eHalStatus csrScanHandleSearchForSSID(tpAniSirGlobal pMac, tSmeCmd *pCommand)
{
    eHalStatus status = eHAL_STATUS_FAILURE;
    tScanResultHandle hBSSList = CSR_INVALID_SCANRESULT_HANDLE;
    tCsrScanResultFilter *pScanFilter = NULL;
    tCsrRoamProfile *pProfile = pCommand->u.scanCmd.pToRoamProfile;
    tANI_U32 sessionId = pCommand->sessionId;
#ifdef FEATURE_WLAN_BTAMP_UT_RF
    tCsrRoamSession *pSession = CSR_GET_SESSION( pMac, sessionId );
#endif
    do
    {
        //                                                                                                       
        if(csrIsRoamCommandWaitingForSession(pMac, sessionId))
        {
            smsLog(pMac, LOGW, FL(" aborts because roam command waiting\n"));
            break;
        }
        if(pProfile == NULL)
            break;
        status = palAllocateMemory(pMac->hHdd, (void **)&pScanFilter, sizeof(tCsrScanResultFilter));
        if(!HAL_STATUS_SUCCESS(status))
            break;
        palZeroMemory(pMac->hHdd, pScanFilter, sizeof(tCsrScanResultFilter));
        status = csrRoamPrepareFilterFromProfile(pMac, pProfile, pScanFilter);
        if(!HAL_STATUS_SUCCESS(status))
            break;
        status = csrScanGetResult(pMac, pScanFilter, &hBSSList);
        if(!HAL_STATUS_SUCCESS(status))
            break;
        status = csrRoamIssueConnect(pMac, sessionId, pProfile, hBSSList, eCsrHddIssued, 
                                    pCommand->u.scanCmd.roamId, eANI_BOOLEAN_TRUE, eANI_BOOLEAN_TRUE);
        if(!HAL_STATUS_SUCCESS(status))
        {
            break;
        }
    }while(0);
    if(!HAL_STATUS_SUCCESS(status))
    {
        if(CSR_INVALID_SCANRESULT_HANDLE != hBSSList)
        {
            csrScanResultPurge(pMac, hBSSList);
        }
        //                                        
        csrRoamCallCallback(pMac, sessionId, NULL, pCommand->u.scanCmd.roamId,
                     eCSR_ROAM_ASSOCIATION_FAILURE, eCSR_ROAM_RESULT_FAILURE);
        //                                                     
        if(csrIsConnStateDisconnected(pMac, sessionId) && !csrIsRoamCommandWaiting(pMac))
        {
            status = csrScanStartIdleScan(pMac);
        }
#ifdef FEATURE_WLAN_BTAMP_UT_RF
        //                                     
        if( CSR_IS_WDS_STA(pProfile) )
        {
            //                                     
            csrFreeRoamProfile( pMac, sessionId );
            if (HAL_STATUS_SUCCESS(palAllocateMemory(pMac->hHdd, 
                                  (void **)&pSession->pCurRoamProfile,
                                  sizeof(tCsrRoamProfile))))
            {
                palZeroMemory(pMac->hHdd, pSession->pCurRoamProfile, sizeof(tCsrRoamProfile));
                csrRoamCopyProfile(pMac, pSession->pCurRoamProfile, pProfile);
            }
            csrRoamStartJoinRetryTimer(pMac, sessionId, CSR_JOIN_RETRY_TIMEOUT_PERIOD);
        }
#endif
    }
    if(pScanFilter)
    {
        csrFreeScanFilter(pMac, pScanFilter);
        palFreeMemory(pMac->hHdd, pScanFilter);
    }

    return (status);
}


eHalStatus csrScanHandleSearchForSSIDFailure(tpAniSirGlobal pMac, tSmeCmd *pCommand)
{
    eHalStatus status = eHAL_STATUS_SUCCESS;
    tANI_U32 sessionId = pCommand->sessionId;
    tCsrRoamProfile *pProfile = pCommand->u.scanCmd.pToRoamProfile;
    tCsrRoamSession *pSession = CSR_GET_SESSION( pMac, sessionId );

    if(!pSession)
    {
        smsLog(pMac, LOGE, FL("  session %d not found "), sessionId);
        return eHAL_STATUS_FAILURE;
    }

#if defined(WLAN_DEBUG)
    if(pCommand->u.scanCmd.u.scanRequest.SSIDs.numOfSSIDs == 1)
    {
        char str[36];
        palCopyMemory(pMac->hHdd, str, pCommand->u.scanCmd.u.scanRequest.SSIDs.SSIDList[0].SSID.ssId,
            pCommand->u.scanCmd.u.scanRequest.SSIDs.SSIDList[0].SSID.length);
        str[pCommand->u.scanCmd.u.scanRequest.SSIDs.SSIDList[0].SSID.length] = 0;
        smsLog(pMac, LOGW, FL(" SSID = %s\n"), str);
    }
#endif
    //                                                                                  
    if(pProfile && CSR_IS_START_IBSS(pProfile))
    {
        status = csrRoamIssueConnect(pMac, sessionId, pProfile, NULL, eCsrHddIssued, 
                                        pCommand->u.scanCmd.roamId, eANI_BOOLEAN_TRUE, eANI_BOOLEAN_TRUE);
        if(!HAL_STATUS_SUCCESS(status))
        {
            smsLog(pMac, LOGE, FL("failed to issue startIBSS command with status = 0x%08X\n"), status);
            csrRoamCallCallback(pMac, sessionId, NULL, pCommand->u.scanCmd.roamId, eCSR_ROAM_FAILED, eCSR_ROAM_RESULT_FAILURE);
        }
    }
    else 
    {
        eCsrRoamResult roamResult = eCSR_ROAM_RESULT_FAILURE;

        if(csrIsConnStateDisconnected(pMac, sessionId) &&
          !csrIsRoamCommandWaitingForSession(pMac, sessionId))
        {
            status = csrScanStartIdleScan(pMac);
        }
        if((NULL == pProfile) || !csrIsBssTypeIBSS(pProfile->BSSType))
        {
            //                                                          
            if(pSession->bRefAssocStartCnt > 0)
            {
                tCsrRoamInfo *pRoamInfo = NULL, roamInfo;
                palZeroMemory(pMac->hHdd, &roamInfo, sizeof(tCsrRoamInfo));
                pRoamInfo = &roamInfo;
                if(pCommand->u.roamCmd.pRoamBssEntry)
                {
                    tCsrScanResult *pScanResult = 
                                GET_BASE_ADDR(pCommand->u.roamCmd.pRoamBssEntry,
                                tCsrScanResult, Link);
                    roamInfo.pBssDesc = &pScanResult->Result.BssDescriptor;
                }
                roamInfo.statusCode = pSession->joinFailStatusCode.statusCode;
                roamInfo.reasonCode = pSession->joinFailStatusCode.reasonCode;
                pSession->bRefAssocStartCnt--;
                csrRoamCallCallback(pMac, sessionId, pRoamInfo,
                                    pCommand->u.scanCmd.roamId,
                                    eCSR_ROAM_ASSOCIATION_COMPLETION,
                                    eCSR_ROAM_RESULT_FAILURE);
            }
            else
            {
                csrRoamCallCallback(pMac, sessionId, NULL,
                                    pCommand->u.scanCmd.roamId,
                                    eCSR_ROAM_ASSOCIATION_FAILURE,
                                    eCSR_ROAM_RESULT_FAILURE);
            }
#ifdef FEATURE_WLAN_BTAMP_UT_RF
            //                                     
            if( CSR_IS_WDS_STA(pProfile) )
            {
                //                                     
                csrFreeRoamProfile( pMac, sessionId );
                if (HAL_STATUS_SUCCESS(palAllocateMemory(pMac->hHdd, 
                                      (void **)&pSession->pCurRoamProfile,
                                      sizeof(tCsrRoamProfile))))
                {
                    palZeroMemory(pMac->hHdd, pSession->pCurRoamProfile, sizeof(tCsrRoamProfile));
                    csrRoamCopyProfile(pMac, pSession->pCurRoamProfile, pProfile);
                }
                csrRoamStartJoinRetryTimer(pMac, sessionId, CSR_JOIN_RETRY_TIMEOUT_PERIOD);
            }
#endif
        }
        else
        {
            roamResult = eCSR_ROAM_RESULT_IBSS_START_FAILED;
        }
        csrRoamCompletion(pMac, sessionId, NULL, pCommand, roamResult, eANI_BOOLEAN_FALSE);
    }

    return (status);
}


//                                                                                                                
eHalStatus csrScanHandleCapChangeScanComplete(tpAniSirGlobal pMac, tANI_U32 sessionId)
{
    eHalStatus status = eHAL_STATUS_FAILURE;
    tScanResultHandle hBSSList = NULL;
    tCsrScanResultFilter *pScanFilter = NULL;
    tANI_U32 roamId = 0;
    tCsrRoamProfile *pProfile = NULL;
    tCsrRoamSession *pSession = CSR_GET_SESSION( pMac, sessionId );

    do
    {
        //                                         
        status = palAllocateMemory(pMac->hHdd, (void **)&pScanFilter, sizeof(tCsrScanResultFilter));
        if(!HAL_STATUS_SUCCESS(status))
            break;
        palZeroMemory(pMac->hHdd, pScanFilter, sizeof(tCsrScanResultFilter));
        if(NULL == pSession) break;
        if(NULL == pSession->pCurRoamProfile)
        {
            pScanFilter->EncryptionType.numEntries = 1;
            pScanFilter->EncryptionType.encryptionType[0] = eCSR_ENCRYPT_TYPE_NONE;
        }
        else
        {
            //                                                                                            
            status = palAllocateMemory(pMac->hHdd, (void **)&pProfile, sizeof(tCsrRoamProfile));
            if(!HAL_STATUS_SUCCESS(status))
                break;
            status = csrRoamCopyProfile(pMac, pProfile, pSession->pCurRoamProfile);
            if(!HAL_STATUS_SUCCESS(status))
                break;
            status = csrRoamPrepareFilterFromProfile(pMac, pProfile, pScanFilter);
        }//                 
        roamId = GET_NEXT_ROAM_ID(&pMac->roam);
        if(HAL_STATUS_SUCCESS(status))
        {
            status = csrScanGetResult(pMac, pScanFilter, &hBSSList);
            if(HAL_STATUS_SUCCESS(status))
            {
                //                                                                        
                csrMoveBssToHeadFromBSSID(pMac, &pSession->connectedProfile.bssid, hBSSList);
                status = csrRoamIssueConnect(pMac, sessionId, pProfile, hBSSList, 
                                            eCsrCapsChange, 0, eANI_BOOLEAN_TRUE, eANI_BOOLEAN_TRUE);
                if(!HAL_STATUS_SUCCESS(status))
                {
                    csrScanResultPurge(pMac, hBSSList);
                }
            }//                
            else
            {
                smsLog(pMac, LOGW, FL("cannot find matching BSS of %02X-%02X-%02X-%02X-%02X-%02X\n"), 
                        pSession->connectedProfile.bssid[0],
                        pSession->connectedProfile.bssid[1],
                        pSession->connectedProfile.bssid[2],
                        pSession->connectedProfile.bssid[3],
                        pSession->connectedProfile.bssid[4],
                        pSession->connectedProfile.bssid[5]);
                //          
                csrRoamDisconnectInternal(pMac, sessionId, eCSR_DISCONNECT_REASON_UNSPECIFIED);
            }
        }
    }while(0);
    if(pScanFilter)
    {
        csrFreeScanFilter(pMac, pScanFilter);
        palFreeMemory(pMac->hHdd, pScanFilter);
    }
    if(NULL != pProfile)
    {
        csrReleaseProfile(pMac, pProfile);
        palFreeMemory(pMac->hHdd, pProfile);
    }

    return (status);
}



eHalStatus csrScanResultPurge(tpAniSirGlobal pMac, tScanResultHandle hScanList)
{
    eHalStatus status = eHAL_STATUS_INVALID_PARAMETER;
    tScanResultList *pScanList = (tScanResultList *)hScanList;
     
    if(pScanList)
    {
        status = csrLLScanPurgeResult(pMac, &pScanList->List);
        csrLLClose(&pScanList->List);
        palFreeMemory(pMac->hHdd, pScanList);
    }
    return (status);
}


static tANI_U32 csrGetBssPreferValue(tpAniSirGlobal pMac, int rssi)
{
    tANI_U32 ret = 0;
    int i = CSR_NUM_RSSI_CAT - 1;

    while(i >= 0)
    {
        if(rssi >= pMac->roam.configParam.RSSICat[i])
        {
            ret = pMac->roam.configParam.BssPreferValue[i];
            break;
        }
        i--;
    };

    return (ret);
}


//                                                   
static tANI_U32 csrGetBssCapValue(tpAniSirGlobal pMac, tSirBssDescription *pBssDesc, tDot11fBeaconIEs *pIes)
{
    tANI_U32 ret = CSR_BSS_CAP_VALUE_NONE;

    if( pIes )
    {
        //                                 
        if(pIes->HTCaps.present)
        {
            ret += CSR_BSS_CAP_VALUE_HT;
        }
        if(CSR_IS_QOS_BSS(pIes))
        {
            ret += CSR_BSS_CAP_VALUE_WMM;
            //                       
            if(CSR_IS_UAPSD_BSS(pIes))
            {
                ret += CSR_BSS_CAP_VALUE_UAPSD;
            }
        }
    }

    return (ret);
}


//                                          
static tANI_BOOLEAN csrIsBetterBss(tCsrScanResult *pBss1, tCsrScanResult *pBss2)
{
    tANI_BOOLEAN ret;

    if(CSR_IS_BETTER_PREFER_VALUE(pBss1->preferValue, pBss2->preferValue))
    {
        ret = eANI_BOOLEAN_TRUE;
    }
    else if(CSR_IS_EQUAL_PREFER_VALUE(pBss1->preferValue, pBss2->preferValue))
    {
        if(CSR_IS_BETTER_CAP_VALUE(pBss1->capValue, pBss2->capValue))
        {
            ret = eANI_BOOLEAN_TRUE;
        }
        else
        {
            ret = eANI_BOOLEAN_FALSE;
        }
    }
    else
    {
        ret = eANI_BOOLEAN_FALSE;
    }

    return (ret);
}


#ifdef FEATURE_WLAN_LFR 
//                                             
static void csrScanAddToOccupiedChannels(
        tpAniSirGlobal pMac, 
        tCsrScanResult *pResult, 
        tCsrChannel *pOccupiedChannels, 
        tDot11fBeaconIEs *pIes)
{
    eHalStatus status;
    tANI_U8   channel;
    tANI_U8 numOccupiedChannels = pOccupiedChannels->numChannels;
    tANI_U8 *pOccupiedChannelList = pOccupiedChannels->channelList;

    channel = pResult->Result.BssDescriptor.channelId;

    if (!csrIsChannelPresentInList(pOccupiedChannelList, numOccupiedChannels, channel)
        && csrNeighborRoamIsSsidCandidateMatch(pMac, pIes)) 
    {
        status = csrAddToChannelListFront(pOccupiedChannelList, numOccupiedChannels, channel); 
        if(HAL_STATUS_SUCCESS(status))
        { 
            pOccupiedChannels->numChannels++;
            if (pOccupiedChannels->numChannels > CSR_BG_SCAN_OCCUPIED_CHANNEL_LIST_LEN) 
                pOccupiedChannels->numChannels = CSR_BG_SCAN_OCCUPIED_CHANNEL_LIST_LEN; 
        } 
    }
}
#endif

//                                     
//                    
static void csrScanAddResult(tpAniSirGlobal pMac, tCsrScanResult *pResult, tDot11fBeaconIEs *pIes)
{
    pResult->preferValue = csrGetBssPreferValue(pMac, (int)pResult->Result.BssDescriptor.rssi);
    pResult->capValue = csrGetBssCapValue(pMac, &pResult->Result.BssDescriptor, pIes);
    csrLLInsertTail( &pMac->scan.scanResultList, &pResult->Link, LL_ACCESS_LOCK );
#ifdef FEATURE_WLAN_LFR 
    csrScanAddToOccupiedChannels(pMac, pResult, &pMac->scan.occupiedChannels, pIes);
#endif
}


eHalStatus csrScanGetResult(tpAniSirGlobal pMac, tCsrScanResultFilter *pFilter, tScanResultHandle *phResult)
{
    eHalStatus status;
    tScanResultList *pRetList;
    tCsrScanResult *pResult, *pBssDesc;
    tANI_U32 count = 0;
    tListElem *pEntry;
    tANI_U32 bssLen, allocLen;
    eCsrEncryptionType uc = eCSR_ENCRYPT_TYPE_NONE, mc = eCSR_ENCRYPT_TYPE_NONE;
    eCsrAuthType auth = eCSR_AUTH_TYPE_OPEN_SYSTEM;
    tDot11fBeaconIEs *pIes, *pNewIes;
    tANI_BOOLEAN fMatch;
    
    if(phResult)
    {
        *phResult = CSR_INVALID_SCANRESULT_HANDLE;
    }
    status = palAllocateMemory(pMac->hHdd, (void **)&pRetList, sizeof(tScanResultList));
    if(HAL_STATUS_SUCCESS(status))
    {
        palZeroMemory(pMac->hHdd, pRetList, sizeof(tScanResultList));
        csrLLOpen(pMac->hHdd, &pRetList->List);
        pRetList->pCurEntry = NULL;
        
        csrLLLock(&pMac->scan.scanResultList);
        pEntry = csrLLPeekHead( &pMac->scan.scanResultList, LL_ACCESS_NOLOCK );
        while( pEntry ) 
        {
            pBssDesc = GET_BASE_ADDR( pEntry, tCsrScanResult, Link );
            pIes = (tDot11fBeaconIEs *)( pBssDesc->Result.pvIes );
            //                                                                                      
            //                                                            
            //     
            fMatch = eANI_BOOLEAN_FALSE;
            pNewIes = NULL;

            if(pFilter)
            {
                fMatch = csrMatchBSS(pMac, &pBssDesc->Result.BssDescriptor, pFilter, &auth, &uc, &mc, &pIes);
                if( NULL != pIes )
                {
                    //                          
                    if(fMatch)
                    {
                        if( !pBssDesc->Result.pvIes )
                        {
                            //                                                                      
                            pNewIes = pIes;
                        }
                        else
                        {
                            //                                                  
                            //                                                                           
                            //                                                      
                            status = palAllocateMemory(pMac->hHdd, (void **)&pNewIes, sizeof(tDot11fBeaconIEs));
                            if( HAL_STATUS_SUCCESS( status ) )
                            {
                                palCopyMemory( pMac->hHdd, pNewIes, pIes, sizeof( tDot11fBeaconIEs ) );
                            }
                            else
                            {
                                smsLog(pMac, LOGE, FL(" fail to allocate memory for IEs\n"));
                                //                                            
                                if( !pBssDesc->Result.pvIes )
                                {
                                    palFreeMemory(pMac->hHdd, pIes);
                                }
                                break;
                            }
                        }
                    }//      
                    else if( !pBssDesc->Result.pvIes )
                    {
                        palFreeMemory(pMac->hHdd, pIes);
                    }
                }
            }
            if(NULL == pFilter || fMatch)
            {
                bssLen = pBssDesc->Result.BssDescriptor.length + sizeof(pBssDesc->Result.BssDescriptor.length);
                allocLen = sizeof( tCsrScanResult ) + bssLen;
                status = palAllocateMemory(pMac->hHdd, (void **)&pResult, allocLen);
                if(!HAL_STATUS_SUCCESS(status))
                {
                    smsLog(pMac, LOGE, FL("  fail to allocate memory for scan result, len=%d\n"), allocLen);
                    if(pNewIes)
                    {
                        palFreeMemory(pMac->hHdd, pNewIes);
                    }
                    break;
                }
                palZeroMemory(pMac->hHdd, pResult, allocLen);
                pResult->capValue = pBssDesc->capValue;
                pResult->preferValue = pBssDesc->preferValue;
                pResult->ucEncryptionType = uc;
                pResult->mcEncryptionType = mc;
                pResult->authType = auth;
                pResult->Result.ssId = pBssDesc->Result.ssId;
                pResult->Result.timer = 0;
                //                           
                pResult->Result.pvIes = pNewIes;
                //                    
                status = palCopyMemory(pMac->hHdd, &pResult->Result.BssDescriptor, &pBssDesc->Result.BssDescriptor, bssLen);
                if(!HAL_STATUS_SUCCESS(status))
                {
                    smsLog(pMac, LOGE, FL("  fail to copy memory for scan result\n"));
                    palFreeMemory(pMac->hHdd, pResult);
                    if(pNewIes)
                    {
                        palFreeMemory(pMac->hHdd, pNewIes);
                    }
                    break;
                }
                //                                                                                                  
                if(csrLLIsListEmpty(&pRetList->List, LL_ACCESS_NOLOCK))
                {
                    csrLLInsertTail(&pRetList->List, &pResult->Link, LL_ACCESS_NOLOCK);
                }
                else
                {
                    //                
                    tListElem *pTmpEntry;
                    tCsrScanResult *pTmpResult;
                    
                    pTmpEntry = csrLLPeekHead(&pRetList->List, LL_ACCESS_NOLOCK);
                    while(pTmpEntry)
                    {
                        pTmpResult = GET_BASE_ADDR( pTmpEntry, tCsrScanResult, Link );
                        if(csrIsBetterBss(pResult, pTmpResult))
                        {
                            csrLLInsertEntry(&pRetList->List, pTmpEntry, &pResult->Link, LL_ACCESS_NOLOCK);
                            //                       
                            pResult = NULL;
                            break;
                        }
                        pTmpEntry = csrLLNext(&pRetList->List, pTmpEntry, LL_ACCESS_NOLOCK);
                    }
                    if(pResult != NULL)
                    {
                        //                                   
                        csrLLInsertTail(&pRetList->List, &pResult->Link, LL_ACCESS_NOLOCK);
                    }
                }
                count++;
            }
            pEntry = csrLLNext( &pMac->scan.scanResultList, pEntry, LL_ACCESS_NOLOCK );
        }//     
        csrLLUnlock(&pMac->scan.scanResultList);
        
        smsLog(pMac, LOG2, FL("return %d BSS\n"), csrLLCount(&pRetList->List));
        
        if( !HAL_STATUS_SUCCESS(status) || (phResult == NULL) )
        {
            //                                
            csrScanResultPurge(pMac, (tScanResultHandle)pRetList);
        }
        else
        {
            if(0 == count)
            {
                //                                         
                csrLLClose(&pRetList->List);
                palFreeMemory(pMac->hHdd, pRetList);
                status = eHAL_STATUS_E_NULL_VALUE;
            }
            else if(phResult)
            {
                *phResult = pRetList;
            }
        }
    }//                  
    
    return (status);
}

/*
                                            
                                               
                                                  
                                
                                           
                                          
                                                
                                              
                                                   
                                             
 */
tANI_U8 csrScanFlushDenied(tpAniSirGlobal pMac)
{
    switch(pMac->roam.neighborRoamInfo.neighborRoamState) {
        case eCSR_NEIGHBOR_ROAM_STATE_REPORT_SCAN:
        case eCSR_NEIGHBOR_ROAM_STATE_PREAUTHENTICATING:
        case eCSR_NEIGHBOR_ROAM_STATE_PREAUTH_DONE:
        case eCSR_NEIGHBOR_ROAM_STATE_REASSOCIATING:
            return (pMac->roam.neighborRoamInfo.neighborRoamState);
        default:
            return 0;
    }
}

eHalStatus csrScanFlushResult(tpAniSirGlobal pMac)
{
    tANI_U8 isFlushDenied = csrScanFlushDenied(pMac);
    if (isFlushDenied) {
        smsLog(pMac, LOGW, "%s: scan flush denied in roam state %d",
                __func__, isFlushDenied);
        return eHAL_STATUS_FAILURE;
    }
    return ( csrLLScanPurgeResult(pMac, &pMac->scan.scanResultList) );
}

eHalStatus csrScanFlushP2PResult(tpAniSirGlobal pMac)
{
        eHalStatus status = eHAL_STATUS_SUCCESS;
        tListElem *pEntry,*pFreeElem;
        tCsrScanResult *pBssDesc;
        tDblLinkList *pList = &pMac->scan.scanResultList;

        csrLLLock(pList);

        pEntry = csrLLPeekHead( pList, LL_ACCESS_NOLOCK );
        while( pEntry != NULL)
        {
                pBssDesc = GET_BASE_ADDR( pEntry, tCsrScanResult, Link );
                if( vos_mem_compare( pBssDesc->Result.ssId.ssId, "DIRECT-", 7) )
                {
                        pFreeElem = pEntry;
                        pEntry = csrLLNext(pList, pEntry, LL_ACCESS_NOLOCK);
                        csrLLRemoveEntry(pList, pFreeElem, LL_ACCESS_NOLOCK);
                        csrFreeScanResultEntry( pMac, pBssDesc );
                        continue;
                }
                pEntry = csrLLNext(pList, pEntry, LL_ACCESS_NOLOCK);
        }

        csrLLUnlock(pList);

        return (status);
}

/* 
                     
  
           
                                                                   
                                                  
  
        
                                                         
  
              
  
  
       
  
                                        
                                                 
                                                           
  
                 
 */

eHalStatus csrCheck11dChannel(tANI_U8 channelId, tANI_U8 *pChannelList, tANI_U32 numChannels)
{
    eHalStatus status = eHAL_STATUS_FAILURE;
    tANI_U8 i = 0;

    for (i = 0; i < numChannels; i++)
    {
        if(pChannelList[ i ] == channelId)
        {
            status = eHAL_STATUS_SUCCESS;
            break;
        }
    }
    return status;
}

/* 
                         
  
           
                                                                       
                                                         
  
        
                                                                      
                                                                       
                                                      
  
              
  
  
       
  
                                                      
  
                 
 */

eHalStatus csrScanFilter11dResult(tpAniSirGlobal pMac)
{
    eHalStatus status = eHAL_STATUS_SUCCESS;
    tListElem *pEntry,*pTempEntry;
    tCsrScanResult *pBssDesc;
    tANI_U32 len = sizeof(pMac->roam.validChannelList);

    /*                                  */
    if (!HAL_STATUS_SUCCESS(csrGetCfgValidChannels(pMac,
                                      pMac->roam.validChannelList, &len)))
    {
        smsLog( pMac, LOG1, "Failed to get Channel list from CFG");
    }

    pEntry = csrLLPeekHead( &pMac->scan.scanResultList, LL_ACCESS_LOCK );
    while( pEntry )
    {
        pBssDesc = GET_BASE_ADDR( pEntry, tCsrScanResult, Link );
        pTempEntry = csrLLNext( &pMac->scan.scanResultList, pEntry, 
                                                            LL_ACCESS_LOCK );
        if(csrCheck11dChannel(pBssDesc->Result.BssDescriptor.channelId,
                                              pMac->roam.validChannelList, len))
        {
            /*                                                    */
            if( csrLLRemoveEntry( &pMac->scan.scanResultList, pEntry,
                                                              LL_ACCESS_LOCK ))
            {
                csrFreeScanResultEntry( pMac, pBssDesc );
            }
        }
        pEntry = pTempEntry;
    }
    return status;
}


eHalStatus csrScanCopyResultList(tpAniSirGlobal pMac, tScanResultHandle hIn, tScanResultHandle *phResult)
{
    eHalStatus status = eHAL_STATUS_SUCCESS;
    tScanResultList *pRetList, *pInList = (tScanResultList *)hIn;
    tCsrScanResult *pResult, *pScanResult;
    tANI_U32 count = 0;
    tListElem *pEntry;
    tANI_U32 bssLen, allocLen;
    
    if(phResult)
    {
        *phResult = CSR_INVALID_SCANRESULT_HANDLE;
    }
    status = palAllocateMemory(pMac->hHdd, (void **)&pRetList, sizeof(tScanResultList));
    if(HAL_STATUS_SUCCESS(status))
    {
        palZeroMemory(pMac->hHdd, pRetList, sizeof(tScanResultList));
        csrLLOpen(pMac->hHdd, &pRetList->List);
        pRetList->pCurEntry = NULL;
        csrLLLock(&pMac->scan.scanResultList);
        csrLLLock(&pInList->List);
        
        pEntry = csrLLPeekHead( &pInList->List, LL_ACCESS_NOLOCK );
        while( pEntry ) 
        {
            pScanResult = GET_BASE_ADDR( pEntry, tCsrScanResult, Link );
            bssLen = pScanResult->Result.BssDescriptor.length + sizeof(pScanResult->Result.BssDescriptor.length);
            allocLen = sizeof( tCsrScanResult ) + bssLen;
            status = palAllocateMemory(pMac->hHdd, (void **)&pResult, allocLen);
            if(!HAL_STATUS_SUCCESS(status))
            {
                csrScanResultPurge(pMac, (tScanResultHandle *)pRetList);
                count = 0;
                break;
            }
            palZeroMemory(pMac->hHdd, pResult, allocLen);
            status = palCopyMemory(pMac->hHdd, &pResult->Result.BssDescriptor, &pScanResult->Result.BssDescriptor, bssLen);
            if(!HAL_STATUS_SUCCESS(status))
            {
                csrScanResultPurge(pMac, (tScanResultHandle *)pRetList);
                count = 0;
                break;
            }
            if( pScanResult->Result.pvIes )
            {
                status = palAllocateMemory(pMac->hHdd, (void **)&pResult->Result.pvIes, sizeof( tDot11fBeaconIEs ));
                if(!HAL_STATUS_SUCCESS(status))
                {
                    //                                       
                    palFreeMemory( pMac->hHdd, pResult );
                    csrScanResultPurge(pMac, (tScanResultHandle *)pRetList);
                    count = 0;
                    break;
                }
                status = palCopyMemory(pMac->hHdd, pResult->Result.pvIes, 
                                pScanResult->Result.pvIes, sizeof( tDot11fBeaconIEs ));
                if(!HAL_STATUS_SUCCESS(status))
                {
                    //                                       
                    palFreeMemory( pMac->hHdd, pResult );
                    csrScanResultPurge(pMac, (tScanResultHandle *)pRetList);
                    count = 0;
                    break;
                }
            }
            csrLLInsertTail(&pRetList->List, &pResult->Link, LL_ACCESS_LOCK);
            count++;
            pEntry = csrLLNext( &pInList->List, pEntry, LL_ACCESS_NOLOCK );
        }//     
        csrLLUnlock(&pInList->List);
        csrLLUnlock(&pMac->scan.scanResultList);
        
        if(HAL_STATUS_SUCCESS(status))
        {
            if(0 == count)
            {
                csrLLClose(&pRetList->List);
                palFreeMemory(pMac->hHdd, pRetList);
                status = eHAL_STATUS_E_NULL_VALUE;
            }
            else if(phResult)
            {
                *phResult = pRetList;
            }
        }
    }//                  
    
    return (status);
}


 
eHalStatus csrScanningStateMsgProcessor( tpAniSirGlobal pMac, void *pMsgBuf )
{
    eHalStatus status = eHAL_STATUS_SUCCESS;
    tSirMbMsg *pMsg = (tSirMbMsg *)pMsgBuf;

    if((eWNI_SME_SCAN_RSP == pMsg->type) || (eWNI_SME_GET_SCANNED_CHANNEL_RSP == pMsg->type))
    {
        status = csrScanSmeScanResponse( pMac, pMsgBuf );
    }
    else
    {
        if( csrIsAnySessionInConnectState( pMac ) )
        {
            //                                                                            
            //                                          
            csrRoamCheckForLinkStatusChange( pMac, ( tSirSmeRsp * )pMsgBuf );
        }
        else
        {
            smsLog( pMac, LOGW, "Message [0x%04x] received in state, when expecting Scan Response\n", pMsg->type );
        }
    }

    return (status);
}



void csrCheckNSaveWscIe(tpAniSirGlobal pMac, tSirBssDescription *pNewBssDescr, tSirBssDescription *pOldBssDescr)
{
    int idx, len;
    tANI_U8 *pbIe;

    //                                                  
    if((pNewBssDescr->fProbeRsp != pOldBssDescr->fProbeRsp) &&
       (0 == pNewBssDescr->WscIeLen))
    {
        idx = 0;
        len = pOldBssDescr->length - sizeof(tSirBssDescription) + 
                sizeof(tANI_U16) + sizeof(tANI_U32) - DOT11F_IE_WSCPROBERES_MIN_LEN - 2;
        pbIe = (tANI_U8 *)pOldBssDescr->ieFields;
        //                        
        pNewBssDescr->WscIeLen = 0;
        while(idx < len)
        {
            if((DOT11F_EID_WSCPROBERES == pbIe[0]) &&
                (0x00 == pbIe[2]) && (0x50 == pbIe[3]) && (0xf2 == pbIe[4]) && (0x04 == pbIe[5]))
            {
                //         
                if((DOT11F_IE_WSCPROBERES_MAX_LEN - 2) >= pbIe[1])
                {
                    palCopyMemory(pMac->hHdd, pNewBssDescr->WscIeProbeRsp,
                                   pbIe, pbIe[1] + 2);
                    pNewBssDescr->WscIeLen = pbIe[1] + 2;
                }
                break;
            }
            idx += pbIe[1] + 2;
            pbIe += pbIe[1] + 2;
        }
    }
}



//                
tANI_BOOLEAN csrRemoveDupBssDescription( tpAniSirGlobal pMac, tSirBssDescription *pSirBssDescr,
                                         tDot11fBeaconIEs *pIes, tAniSSID *pSsid , v_TIME_t *timer ) 
{
    tListElem *pEntry;

    tCsrScanResult *pBssDesc;
    tANI_BOOLEAN fRC = FALSE;

    //                                                                                        
    //                                                                                     
    //                                                                                            
    pEntry = csrLLPeekHead( &pMac->scan.scanResultList, LL_ACCESS_LOCK );

    while( pEntry ) 
    {
        pBssDesc = GET_BASE_ADDR( pEntry, tCsrScanResult, Link );

        //                                                                                
        //        
        if ( csrIsDuplicateBssDescription( pMac, &pBssDesc->Result.BssDescriptor, 
                                                        pSirBssDescr, pIes ) )
        {
            pSirBssDescr->rssi = (tANI_S8)( (((tANI_S32)pSirBssDescr->rssi * CSR_SCAN_RESULT_RSSI_WEIGHT ) +
                                             ((tANI_S32)pBssDesc->Result.BssDescriptor.rssi * (100 - CSR_SCAN_RESULT_RSSI_WEIGHT) )) / 100 );
            //                                         
            if( csrLLRemoveEntry( &pMac->scan.scanResultList, pEntry, LL_ACCESS_LOCK ) )
            {
                //                                                      
                //                                                  
                *pSsid = pBssDesc->Result.ssId;
                *timer = pBssDesc->Result.timer;
                csrCheckNSaveWscIe(pMac, pSirBssDescr, &pBssDesc->Result.BssDescriptor);
                
                csrFreeScanResultEntry( pMac, pBssDesc );
            }
            else
            {
                smsLog( pMac, LOGW, FL( "  fail to remove entry\n" ) );
            }
            fRC = TRUE;

            //                                                           
            break;
        }

        pEntry = csrLLNext( &pMac->scan.scanResultList, pEntry, LL_ACCESS_LOCK );
    }

    return fRC;
}


eHalStatus csrAddPMKIDCandidateList( tpAniSirGlobal pMac, tANI_U32 sessionId,
                                     tSirBssDescription *pBssDesc, tDot11fBeaconIEs *pIes )
{
    eHalStatus status = eHAL_STATUS_FAILURE;
    tCsrRoamSession *pSession = CSR_GET_SESSION( pMac, sessionId );

    if(!pSession)
    {
        smsLog(pMac, LOGE, FL("  session %d not found "), sessionId);
        return eHAL_STATUS_FAILURE;
    }

    smsLog(pMac, LOGW, "csrAddPMKIDCandidateList called pMac->scan.NumPmkidCandidate = %d\n", pSession->NumPmkidCandidate);
    if( pIes )
    {
        //                           
        if( pIes->RSN.present )
        {
            //                                                        
            if( pSession->NumPmkidCandidate < CSR_MAX_PMKID_ALLOWED )
            {

#ifdef FEATURE_WLAN_DIAG_SUPPORT_CSR
                {
                    WLAN_VOS_DIAG_EVENT_DEF(secEvent, vos_event_wlan_security_payload_type);
                    palZeroMemory(pMac->hHdd, &secEvent, sizeof(vos_event_wlan_security_payload_type));
                    secEvent.eventId = WLAN_SECURITY_EVENT_PMKID_CANDIDATE_FOUND;
                    secEvent.encryptionModeMulticast = 
                        (v_U8_t)diagEncTypeFromCSRType(pSession->connectedProfile.mcEncryptionType);
                    secEvent.encryptionModeUnicast = 
                        (v_U8_t)diagEncTypeFromCSRType(pSession->connectedProfile.EncryptionType);
                    palCopyMemory( pMac->hHdd, secEvent.bssid, pSession->connectedProfile.bssid, 6 );
                    secEvent.authMode = 
                        (v_U8_t)diagAuthTypeFromCSRType(pSession->connectedProfile.AuthType);
                    WLAN_VOS_DIAG_EVENT_REPORT(&secEvent, EVENT_WLAN_SECURITY);
                }
#endif//                                    

                //                                       
                status = palCopyMemory(pMac->hHdd, pSession->PmkidCandidateInfo[pSession->NumPmkidCandidate].BSSID, 
                                            pBssDesc->bssId, WNI_CFG_BSSID_LEN);
            
                if( HAL_STATUS_SUCCESS( status ) )
                {
                    if ( pIes->RSN.preauth )
                    {
                        pSession->PmkidCandidateInfo[pSession->NumPmkidCandidate].preAuthSupported = eANI_BOOLEAN_TRUE;
                    }
                    else
                    {
                        pSession->PmkidCandidateInfo[pSession->NumPmkidCandidate].preAuthSupported = eANI_BOOLEAN_FALSE;
                    }
                    pSession->NumPmkidCandidate++;
                }
            }
            else
            {
                status = eHAL_STATUS_FAILURE;
            }
        }
    }

    return (status);
}

//                                                                              
//                                                                         
tANI_U32 csrProcessBSSDescForPMKIDList(tpAniSirGlobal pMac, 
                                           tSirBssDescription *pBssDesc,
                                           tDot11fBeaconIEs *pIes)
{
    tANI_U32 i, bRet = CSR_SESSION_ID_INVALID;
    tCsrRoamSession *pSession;
    tDot11fBeaconIEs *pIesLocal = pIes;

    if( pIesLocal || HAL_STATUS_SUCCESS(csrGetParsedBssDescriptionIEs(pMac, pBssDesc, &pIesLocal)) )
    {
        for( i = 0; i < CSR_ROAM_SESSION_MAX; i++ )
        {
            if( CSR_IS_SESSION_VALID( pMac, i ) )
            {
                pSession = CSR_GET_SESSION( pMac, i );
                if( csrIsConnStateConnectedInfra( pMac, i ) && 
                    ( eCSR_AUTH_TYPE_RSN == pSession->connectedProfile.AuthType ) )
                {
                    if(csrMatchBSSToConnectProfile(pMac, &pSession->connectedProfile, pBssDesc, pIesLocal))
                    {
                        //                                               
                        if(HAL_STATUS_SUCCESS(csrAddPMKIDCandidateList(pMac, i, pBssDesc, pIesLocal)))
                        {
                            bRet = i;
                        }
                        break;
                    }
                }
            }
        }
        if( !pIes )
        {
            palFreeMemory(pMac->hHdd, pIesLocal);
        }
    }

    return (tANI_U8)bRet;
}

#ifdef FEATURE_WLAN_WAPI
eHalStatus csrAddBKIDCandidateList( tpAniSirGlobal pMac, tANI_U32 sessionId,
                                    tSirBssDescription *pBssDesc, tDot11fBeaconIEs *pIes )
{
    eHalStatus status = eHAL_STATUS_FAILURE;
    tCsrRoamSession *pSession = CSR_GET_SESSION( pMac, sessionId );

    if(!pSession)
    {
        smsLog(pMac, LOGE, FL("  session %d not found "), sessionId);
        return eHAL_STATUS_FAILURE;
    }

    smsLog(pMac, LOGW, "csrAddBKIDCandidateList called pMac->scan.NumBkidCandidate = %d\n", pSession->NumBkidCandidate);
    if( pIes )
    {
        //                            
        if( pIes->WAPI.present )
        {
            //                                                        
            if( pSession->NumBkidCandidate < CSR_MAX_BKID_ALLOWED )
            {

                //                                      
                status = palCopyMemory(pMac->hHdd, pSession->BkidCandidateInfo[pSession->NumBkidCandidate].BSSID, 
                                            pBssDesc->bssId, WNI_CFG_BSSID_LEN);
            
                if( HAL_STATUS_SUCCESS( status ) )
                {
                    if ( pIes->WAPI.preauth )
                    {
                        pSession->BkidCandidateInfo[pSession->NumBkidCandidate].preAuthSupported = eANI_BOOLEAN_TRUE;
                    }
                    else
                    {
                        pSession->BkidCandidateInfo[pSession->NumBkidCandidate].preAuthSupported = eANI_BOOLEAN_FALSE;
                    }
                    pSession->NumBkidCandidate++;
                }
            }
            else
            {
                status = eHAL_STATUS_FAILURE;
            }
        }
    }

    return (status);
}

//                                                                              
//                              
tANI_BOOLEAN csrProcessBSSDescForBKIDList(tpAniSirGlobal pMac, tSirBssDescription *pBssDesc,
                                          tDot11fBeaconIEs *pIes)
{
    tANI_BOOLEAN fRC = FALSE;
    tDot11fBeaconIEs *pIesLocal = pIes;
    tANI_U32 sessionId;
    tCsrRoamSession *pSession;

    if( pIesLocal || HAL_STATUS_SUCCESS(csrGetParsedBssDescriptionIEs(pMac, pBssDesc, &pIesLocal)) )
    {
        for( sessionId = 0; sessionId < CSR_ROAM_SESSION_MAX; sessionId++ )
        {
            if( CSR_IS_SESSION_VALID( pMac, sessionId) )
            {
                pSession = CSR_GET_SESSION( pMac, sessionId );
                if( csrIsConnStateConnectedInfra( pMac, sessionId ) && 
                    eCSR_AUTH_TYPE_WAPI_WAI_CERTIFICATE == pSession->connectedProfile.AuthType)
                {
                    if(csrMatchBSSToConnectProfile(pMac, &pSession->connectedProfile,pBssDesc, pIesLocal))
                    {
                        //                                               
                        if(HAL_STATUS_SUCCESS(csrAddBKIDCandidateList(pMac, sessionId, pBssDesc, pIesLocal)))
                        {
                            fRC = TRUE;
                        }
                    }
                }
            }
        }
        if(!pIes)
        {
            palFreeMemory(pMac->hHdd, pIesLocal);
        }

    }
    return fRC;
}

#endif


static void csrMoveTempScanResultsToMainList( tpAniSirGlobal pMac )
{
    tListElem *pEntry;
    tCsrScanResult *pBssDescription;
    tANI_S8         cand_Bss_rssi;
    tANI_BOOLEAN    fDupBss;
#ifdef FEATURE_WLAN_WAPI
    tANI_BOOLEAN fNewWapiBSSForCurConnection = eANI_BOOLEAN_FALSE;
#endif /*                   */
    tDot11fBeaconIEs *pIesLocal = NULL;
    tANI_U32 sessionId = CSR_SESSION_ID_INVALID;
    tAniSSID tmpSsid;
    v_TIME_t timer=0;

    tmpSsid.length = 0;
    cand_Bss_rssi = -128; //                           

    //                                                
    while( ( pEntry = csrLLRemoveTail( &pMac->scan.tempScanResults, LL_ACCESS_LOCK ) ) != NULL)
    {
        pBssDescription = GET_BASE_ADDR( pEntry, tCsrScanResult, Link );

        smsLog( pMac, LOG2, "...Bssid= %02x-%02x-%02x-%02x-%02x-%02x chan= %d, rssi = -%d\n",
                      pBssDescription->Result.BssDescriptor.bssId[ 0 ], pBssDescription->Result.BssDescriptor.bssId[ 1 ],
                      pBssDescription->Result.BssDescriptor.bssId[ 2 ], pBssDescription->Result.BssDescriptor.bssId[ 3 ],
                      pBssDescription->Result.BssDescriptor.bssId[ 4 ], pBssDescription->Result.BssDescriptor.bssId[ 5 ],
                      pBssDescription->Result.BssDescriptor.channelId,
                pBssDescription->Result.BssDescriptor.rssi * (-1) );

        //                                                       
        pIesLocal = (tDot11fBeaconIEs *)( pBssDescription->Result.pvIes );
        if( !pIesLocal && (!HAL_STATUS_SUCCESS(csrGetParsedBssDescriptionIEs(pMac, &pBssDescription->Result.BssDescriptor, &pIesLocal))) )
        {
            smsLog(pMac, LOGE, FL("  Cannot pared IEs\n"));
            csrFreeScanResultEntry(pMac, pBssDescription);
            continue;
        }
        fDupBss = csrRemoveDupBssDescription( pMac, &pBssDescription->Result.BssDescriptor, pIesLocal, &tmpSsid , &timer );
        //                                     
        if( CSR_SCAN_IS_OVER_BSS_LIMIT(pMac) )
        {
            //           
            smsLog(pMac, LOGW, FL("  BSS limit reached\n"));
            //                  
            if( (pBssDescription->Result.pvIes == NULL) && pIesLocal )
            {
                palFreeMemory(pMac->hHdd, pIesLocal);
            }
            csrFreeScanResultEntry(pMac, pBssDescription);
            //                                            
            continue;
        }
        //                                 
        if ( !fDupBss )
        {
            //               
            sessionId = csrProcessBSSDescForPMKIDList(pMac, 
                             &pBssDescription->Result.BssDescriptor, pIesLocal);
            if( CSR_SESSION_ID_INVALID != sessionId)
            {
                csrRoamCallCallback(pMac, sessionId, NULL, 0, 
                           eCSR_ROAM_SCAN_FOUND_NEW_BSS, eCSR_ROAM_RESULT_NONE);
            }
        }
        else
        {
            //                                                                             
            if( (0 == pBssDescription->Result.ssId.length) && tmpSsid.length )
            {
                //                                                                       
                //                                                                       
                //                                                                                   
                //                                                                   
                //                                                                      
                //                                           
                if( (vos_timer_get_system_time() - timer) <= HIDDEN_TIMER)
                {
                   pBssDescription->Result.timer = timer;
                   pBssDescription->Result.ssId = tmpSsid;
                }
            }
        }

        //                                 
        if( csrIs11dSupported( pMac ) )
        {
            if(cand_Bss_rssi < pBssDescription->Result.BssDescriptor.rssi)
            {
                //                                                
                if(pIesLocal->Country.present)
                {
                    cand_Bss_rssi = pBssDescription->Result.BssDescriptor.rssi;
                    //                          
                    csrLearnCountryInformation( pMac, &pBssDescription->Result.BssDescriptor, 
                             pIesLocal, eANI_BOOLEAN_FALSE );
                }

            }
        }

        else if( csrIs11hSupported( pMac ) && pIesLocal->Country.present && 
                 !pMac->roam.configParam.fSupplicantCountryCodeHasPriority )
        {
             /*                                                                             */
            csrSaveToChannelPower2G_5G( pMac, pIesLocal->Country.num_triplets * sizeof(tSirMacChanInfo), 
                                        (tSirMacChanInfo *)(&pIesLocal->Country.triplets[0]) );
        }

        //                    
        csrScanAddResult(pMac, pBssDescription, pIesLocal);
        if( (pBssDescription->Result.pvIes == NULL) && pIesLocal )
        {
            palFreeMemory(pMac->hHdd, pIesLocal);
        }
    }

    //                                                                        
    //                                                                         
    //                         
    if(pMac->scan.fAmbiguous11dInfoFound) 
    {
      if((pMac->scan.fCurrent11dInfoMatch) || (cand_Bss_rssi != -128))
      {
        pMac->scan.fAmbiguous11dInfoFound = eANI_BOOLEAN_FALSE;
      }
    }

#ifdef FEATURE_WLAN_WAPI
    if(fNewWapiBSSForCurConnection)
    {
        //                 
        csrRoamCallCallback(pMac, sessionId, NULL, 0, eCSR_ROAM_SCAN_FOUND_NEW_BSS, eCSR_ROAM_RESULT_NEW_WAPI_BSS);
    }
#endif /*                   */

    return;
}


static tCsrScanResult *csrScanSaveBssDescription( tpAniSirGlobal pMac, tSirBssDescription *pBSSDescription,
                                                  tDot11fBeaconIEs *pIes)
{
    tCsrScanResult *pCsrBssDescription = NULL;
    tANI_U32 cbBSSDesc;
    tANI_U32 cbAllocated;
    eHalStatus halStatus;

    //                                                                        
    //                                              
    cbBSSDesc = pBSSDescription->length + sizeof( pBSSDescription->length );

    cbAllocated = sizeof( tCsrScanResult ) + cbBSSDesc;

    halStatus = palAllocateMemory( pMac->hHdd, (void **)&pCsrBssDescription, cbAllocated );
    if ( HAL_STATUS_SUCCESS(halStatus) )
    {
        palZeroMemory( pMac->hHdd, pCsrBssDescription, cbAllocated );
        pCsrBssDescription->AgingCount = (tANI_S32)pMac->roam.configParam.agingCount;
        palCopyMemory(pMac->hHdd, &pCsrBssDescription->Result.BssDescriptor, pBSSDescription, cbBSSDesc );
#if defined(VOSS_ENSBALED)
        VOS_ASSERT( pCsrBssDescription->Result.pvIes == NULL );
#endif
        csrScanAddResult(pMac, pCsrBssDescription, pIes);
    }

    return( pCsrBssDescription );
}

//                            
tCsrScanResult *csrScanAppendBssDescription( tpAniSirGlobal pMac, 
                                             tSirBssDescription *pSirBssDescription, 
                                             tDot11fBeaconIEs *pIes )
{
    tCsrScanResult *pCsrBssDescription = NULL;
    tAniSSID tmpSsid;
    v_TIME_t timer = 0;
    int result;

    tmpSsid.length = 0;
    result = csrRemoveDupBssDescription( pMac, pSirBssDescription, pIes, &tmpSsid, &timer );
    pCsrBssDescription = csrScanSaveBssDescription( pMac, pSirBssDescription, pIes );
    if (result && (pCsrBssDescription != NULL))
    {
        //                                                                             
        if( (0 == pCsrBssDescription->Result.ssId.length) && tmpSsid.length )
        {
            //                                                                      
            //                                                                       
            //                                                                                   
            //                                                                  
            //                                                                     
            //                                           
            if((vos_timer_get_system_time()-timer) <= HIDDEN_TIMER)
            { 
              pCsrBssDescription->Result.ssId = tmpSsid;
              pCsrBssDescription->Result.timer = timer;
            }
        }
    }


    return( pCsrBssDescription );
}



void csrPurgeChannelPower( tpAniSirGlobal pMac, tDblLinkList *pChannelList )
{
    tCsrChannelPowerInfo *pChannelSet;
    tListElem *pEntry;

    csrLLLock(pChannelList); 
    //                                                                            
    while( ( pEntry = csrLLRemoveHead( pChannelList, LL_ACCESS_NOLOCK ) ) != NULL)
    {
        pChannelSet = GET_BASE_ADDR( pEntry, tCsrChannelPowerInfo, link );
        if( pChannelSet )
        {
            palFreeMemory( pMac->hHdd, pChannelSet );
        }
    }
    csrLLUnlock(pChannelList);
    return;
}


/*
                                                                                
                                                                                                                                
 */
eHalStatus csrSaveToChannelPower2G_5G( tpAniSirGlobal pMac, tANI_U32 tableSize, tSirMacChanInfo *channelTable )
{
    tANI_U32 i = tableSize / sizeof( tSirMacChanInfo );
    tSirMacChanInfo *pChannelInfo;
    tCsrChannelPowerInfo *pChannelSet;
    tANI_BOOLEAN f2GHzInfoFound = FALSE;
    tANI_BOOLEAN f2GListPurged = FALSE, f5GListPurged = FALSE;
    eHalStatus halStatus;

    pChannelInfo = channelTable;
    //                                                              
    while ( i-- )
    {
        halStatus = palAllocateMemory( pMac->hHdd, (void **)&pChannelSet, sizeof(tCsrChannelPowerInfo) );
        if ( eHAL_STATUS_SUCCESS == halStatus )
        {
            palZeroMemory(pMac->hHdd, pChannelSet, sizeof(tCsrChannelPowerInfo));
            pChannelSet->firstChannel = pChannelInfo->firstChanNum;
            pChannelSet->numChannels = pChannelInfo->numChannels;

            //                                                                                     
            if( (CSR_IS_CHANNEL_24GHZ(pChannelSet->firstChannel)) &&
                    ((pChannelSet->firstChannel + (pChannelSet->numChannels - 1)) <= CSR_MAX_24GHz_CHANNEL_NUMBER) )

            {
                pChannelSet->interChannelOffset = 1;
                f2GHzInfoFound = TRUE;
            }
            else if ( (CSR_IS_CHANNEL_5GHZ(pChannelSet->firstChannel)) &&
                ((pChannelSet->firstChannel + ((pChannelSet->numChannels - 1) * 4)) <= CSR_MAX_5GHz_CHANNEL_NUMBER) )
            {
                pChannelSet->interChannelOffset = 4;
                f2GHzInfoFound = FALSE;
            }
            else
            {
                smsLog( pMac, LOGW, FL("Invalid Channel %d Present in Country IE"),
                        pChannelSet->firstChannel);
                palFreeMemory(pMac->hHdd, pChannelSet);
                return eHAL_STATUS_FAILURE;
            }

            pChannelSet->txPower = CSR_ROAM_MIN( pChannelInfo->maxTxPower, pMac->roam.configParam.nTxPowerCap );

            if( f2GHzInfoFound )
            {
                if( !f2GListPurged )
                {
                    //                                    
                    csrPurgeChannelPower( pMac, &pMac->scan.channelPowerInfoList24 );
                    f2GListPurged = TRUE;
                }

                if(CSR_IS_OPERATING_BG_BAND(pMac))
                {
                    //                                        
                    csrLLInsertTail( &pMac->scan.channelPowerInfoList24, &pChannelSet->link, LL_ACCESS_LOCK );
                }
                else {
                    smsLog( pMac, LOGW, FL("Adding 11B/G channels in 11A mode -- First Channel is %d"), 
                                pChannelSet->firstChannel);
                    palFreeMemory(pMac->hHdd, pChannelSet);
                }
            }
            else
            {
                //                
                if( !f5GListPurged )
                {
                    //                                    
                    csrPurgeChannelPower( pMac, &pMac->scan.channelPowerInfoList5G );
                    f5GListPurged = TRUE;
                }

                if(CSR_IS_OPERATING_A_BAND(pMac))
                {
                    //                                     
                    csrLLInsertTail( &pMac->scan.channelPowerInfoList5G, &pChannelSet->link, LL_ACCESS_LOCK );
                }
                else {
                    smsLog( pMac, LOGW, FL("Adding 11A channels in B/G mode -- First Channel is %d"), 
                                pChannelSet->firstChannel);
                    palFreeMemory(pMac->hHdd, pChannelSet);
                }
            }
        }

        pChannelInfo++;                //                   
    }

    return eHAL_STATUS_SUCCESS;
}



void csrApplyPower2Current( tpAniSirGlobal pMac )
{
    smsLog( pMac, LOG3, FL(" Updating Cfg with power settings\n"));
    csrSaveTxPowerToCfg( pMac, &pMac->scan.channelPowerInfoList24, WNI_CFG_MAX_TX_POWER_2_4 );
    csrSaveTxPowerToCfg( pMac, &pMac->scan.channelPowerInfoList5G, WNI_CFG_MAX_TX_POWER_5 );
}


void csrApplyChannelPowerCountryInfo( tpAniSirGlobal pMac, tCsrChannel *pChannelList, tANI_U8 *countryCode)
{
    int i;
    eNVChannelEnabledType channelEnabledType;
    tANI_U8 numChannels = 0;
    tANI_U8 tempNumChannels = 0;
    tCsrChannel ChannelList;
    if( pChannelList->numChannels )
    {
        tempNumChannels = CSR_MIN(pChannelList->numChannels, WNI_CFG_VALID_CHANNEL_LIST_LEN);
        /*                                                                   
                              */
        for(i = 0; i< tempNumChannels; i++)
        {
             if(FALSE == pMac->scan.fEnableDFSChnlScan)
             {
                 channelEnabledType = 
                     vos_nv_getChannelEnabledState(pChannelList->channelList[i]);
             }
             else
             {
                channelEnabledType = NV_CHANNEL_ENABLE;
             }
             if( NV_CHANNEL_ENABLE ==  channelEnabledType)
             {
                //                                                  
                if ( vos_mem_compare(countryCode, "ID", VOS_COUNTRY_CODE_LEN ) 
                      && ( pChannelList->channelList[i] == 165 )
                      && ( pMac->scan.fIgnore_chan165 == VOS_TRUE ))
                 {
                     continue;
                 }
                 else
                 {
                     ChannelList.channelList[numChannels] = pChannelList->channelList[i];
                     numChannels++;
                 }
             }
        }
        ChannelList.numChannels = numChannels;   
   
        csrSetCfgValidChannelList(pMac, ChannelList.channelList, ChannelList.numChannels);
        //                       
        csrSetCfgScanControlList(pMac, countryCode, &ChannelList);     //                                                                              
#ifdef FEATURE_WLAN_SCAN_PNO
        //                                  
        //                                                                         
#endif //                      
    }
    else
    {
        smsLog( pMac, LOGE, FL("  11D channel list is empty\n"));
    }
    csrApplyPower2Current( pMac );     //                                                       
    csrSetCfgCountryCode(pMac, countryCode);
}


void csrResetCountryInformation( tpAniSirGlobal pMac, tANI_BOOLEAN fForce )
{
    if( fForce || (csrIs11dSupported( pMac ) && (!pMac->scan.f11dInfoReset)))
    {

#ifdef FEATURE_WLAN_DIAG_SUPPORT_CSR
    {
        vos_log_802_11d_pkt_type *p11dLog;
        int Index;

        WLAN_VOS_DIAG_LOG_ALLOC(p11dLog, vos_log_802_11d_pkt_type, LOG_WLAN_80211D_C);
        if(p11dLog)
        {
            p11dLog->eventId = WLAN_80211D_EVENT_RESET;
            palCopyMemory(pMac->hHdd, p11dLog->countryCode, pMac->scan.countryCodeCurrent, 3);
            p11dLog->numChannel = pMac->scan.base20MHzChannels.numChannels;
            if(p11dLog->numChannel <= VOS_LOG_MAX_NUM_CHANNEL)
            {
                palCopyMemory(pMac->hHdd, p11dLog->Channels, pMac->scan.base20MHzChannels.channelList,
                                p11dLog->numChannel);
                for (Index=0; Index < pMac->scan.base20MHzChannels.numChannels; Index++)
                {
                    p11dLog->TxPwr[Index] = CSR_ROAM_MIN( pMac->scan.defaultPowerTable[Index].pwr, pMac->roam.configParam.nTxPowerCap );
                }
            }
            if(!pMac->roam.configParam.Is11dSupportEnabled)
            {
                p11dLog->supportMultipleDomain = WLAN_80211D_DISABLED;
            }
            else if(pMac->roam.configParam.fEnforceDefaultDomain)
            {
                p11dLog->supportMultipleDomain = WLAN_80211D_NOT_SUPPORT_MULTI_DOMAIN;
            }
            else
            {
                p11dLog->supportMultipleDomain = WLAN_80211D_SUPPORT_MULTI_DOMAIN;
            }
            WLAN_VOS_DIAG_LOG_REPORT(p11dLog);
        }
    }
#endif //                                    

        //                                                 
        if( csrIs11dSupported( pMac ) )
        {
            pMac->scan.curScanType = eSIR_PASSIVE_SCAN;
        }

        csrPruneChannelListForMode(pMac, &pMac->scan.baseChannels);
        csrPruneChannelListForMode(pMac, &pMac->scan.base20MHzChannels);

        csrSaveChannelPowerForBand(pMac, eANI_BOOLEAN_FALSE);
        csrSaveChannelPowerForBand(pMac, eANI_BOOLEAN_TRUE);
        //                                                                      
        csrApplyChannelPowerCountryInfo( pMac, &pMac->scan.base20MHzChannels, pMac->scan.countryCodeCurrent );
        //                           
        palZeroMemory( pMac->hHdd, &pMac->scan.channels11d, sizeof(pMac->scan.channels11d) );
        pMac->scan.f11dInfoReset = eANI_BOOLEAN_TRUE;
        pMac->scan.f11dInfoApplied = eANI_BOOLEAN_FALSE;
    }

    return;
}


eHalStatus csrResetCountryCodeInformation(tpAniSirGlobal pMac, tANI_BOOLEAN *pfRestartNeeded)
{
    eHalStatus status = eHAL_STATUS_SUCCESS;
    tANI_BOOLEAN fRestart = eANI_BOOLEAN_FALSE;

    //                                           
    palCopyMemory(pMac->hHdd, pMac->scan.countryCodeCurrent, pMac->scan.countryCodeDefault, WNI_CFG_COUNTRY_CODE_LEN);
    csrSetRegulatoryDomain(pMac, pMac->scan.domainIdCurrent, &fRestart);
    if( ((eANI_BOOLEAN_FALSE == fRestart) || (pfRestartNeeded == NULL) )
          && !csrIsInfraConnected(pMac))
    {
        //                                                       
        csrResetCountryInformation(pMac, eANI_BOOLEAN_TRUE);
    }
    if(pfRestartNeeded)
    {
        *pfRestartNeeded = fRestart;
    }

    return (status);
}


eHalStatus csrSetCountryCode(tpAniSirGlobal pMac, tANI_U8 *pCountry, tANI_BOOLEAN *pfRestartNeeded)
{
    eHalStatus status = eHAL_STATUS_INVALID_PARAMETER;
    v_REGDOMAIN_t domainId;

    if(pCountry)
    {
        status = csrGetRegulatoryDomainForCountry(pMac, pCountry, &domainId);
        if(HAL_STATUS_SUCCESS(status))
        {
            status = csrSetRegulatoryDomain(pMac, domainId, pfRestartNeeded);
            if(HAL_STATUS_SUCCESS(status))
            {
                //                                                                                  
                //                                                                                
                palCopyMemory(pMac->hHdd, pMac->scan.countryCodeCurrent, pCountry, WNI_CFG_COUNTRY_CODE_LEN);
                if((pfRestartNeeded == NULL) || !(*pfRestartNeeded))
                {
                    //                                                                             
                    csrSetCfgCountryCode(pMac, pCountry);
                }
            }
        }
    }

    return (status);
}



//                                                     
//                                                             
//                                                          
void csrGetChannelPowerInfo( tpAniSirGlobal pMac, tDblLinkList *pList,
                             tANI_U32 *pNumChn, tChannelListWithPower *pChnPowerInfo)
{
    tListElem *pEntry;
    tANI_U32 chnIdx = 0, idx;
    tCsrChannelPowerInfo *pChannelSet;

    //                
    pEntry = csrLLPeekHead( pList, LL_ACCESS_LOCK );
    while( pEntry && (chnIdx < *pNumChn) )
    {
        pChannelSet = GET_BASE_ADDR( pEntry, tCsrChannelPowerInfo, link );
        if ( 1 != pChannelSet->interChannelOffset )
        {
            for( idx = 0; (idx < pChannelSet->numChannels) && (chnIdx < *pNumChn); idx++ )
            {
                pChnPowerInfo[chnIdx].chanId = (tANI_U8)(pChannelSet->firstChannel + ( idx * pChannelSet->interChannelOffset ));
                pChnPowerInfo[chnIdx++].pwr = pChannelSet->txPower;
            }
        }
        else
        {
            for( idx = 0; (idx < pChannelSet->numChannels) && (chnIdx < *pNumChn); idx++ )
            {
                pChnPowerInfo[chnIdx].chanId = (tANI_U8)(pChannelSet->firstChannel + idx);
                pChnPowerInfo[chnIdx++].pwr = pChannelSet->txPower;
            }
        }

        pEntry = csrLLNext( pList, pEntry, LL_ACCESS_LOCK );
    }
    *pNumChn = chnIdx;

    return ;
}



void csrApplyCountryInformation( tpAniSirGlobal pMac, tANI_BOOLEAN fForce )
{
    v_REGDOMAIN_t domainId;
    eHalStatus status = eHAL_STATUS_SUCCESS;

    do
    {
        if( !csrIs11dSupported( pMac ) || 0 == pMac->scan.channelOf11dInfo) break;
        if( pMac->scan.fAmbiguous11dInfoFound )
        {
            //                     
            //                                 
            if(HAL_STATUS_SUCCESS(csrGetRegulatoryDomainForCountry( pMac, pMac->scan.countryCodeCurrent, &domainId )))
            {
                pMac->scan.domainIdCurrent = domainId;
            }
            else
            {
                smsLog(pMac, LOGE, FL(" failed to get domain from currentCountryCode %02X%02X\n"), 
                    pMac->scan.countryCodeCurrent[0], pMac->scan.countryCodeCurrent[1]);
            }
            csrResetCountryInformation( pMac, eANI_BOOLEAN_FALSE );
            break;
        }
        if ( pMac->scan.f11dInfoApplied && !fForce ) break;
        if(HAL_STATUS_SUCCESS(csrGetRegulatoryDomainForCountry( pMac, pMac->scan.countryCode11d, &domainId )))
        {
            //                                               
            if( ( !pMac->roam.configParam.fEnforceDefaultDomain ) ||
                (pMac->scan.domainIdCurrent == domainId) )
            {

#ifdef FEATURE_WLAN_DIAG_SUPPORT_CSR
                {
                    vos_log_802_11d_pkt_type *p11dLog;
                    tChannelListWithPower chnPwrInfo[WNI_CFG_VALID_CHANNEL_LIST_LEN];
                    tANI_U32 nChnInfo = WNI_CFG_VALID_CHANNEL_LIST_LEN, nTmp;

                    WLAN_VOS_DIAG_LOG_ALLOC(p11dLog, vos_log_802_11d_pkt_type, LOG_WLAN_80211D_C);
                    if(p11dLog)
                    {
                        p11dLog->eventId = WLAN_80211D_EVENT_COUNTRY_SET;
                        palCopyMemory(pMac->hHdd, p11dLog->countryCode, pMac->scan.countryCode11d, 3);
                        p11dLog->numChannel = pMac->scan.channels11d.numChannels;
                        if(p11dLog->numChannel <= VOS_LOG_MAX_NUM_CHANNEL)
                        {
                            palCopyMemory(pMac->hHdd, p11dLog->Channels, pMac->scan.channels11d.channelList,
                                            p11dLog->numChannel);
                            csrGetChannelPowerInfo(pMac, &pMac->scan.channelPowerInfoList24,
                                                    &nChnInfo, chnPwrInfo);
                            nTmp = nChnInfo;
                            nChnInfo = WNI_CFG_VALID_CHANNEL_LIST_LEN - nTmp;
                            csrGetChannelPowerInfo(pMac, &pMac->scan.channelPowerInfoList5G,
                                                    &nChnInfo, &chnPwrInfo[nTmp]);
                            for(nTmp = 0; nTmp < p11dLog->numChannel; nTmp++)
                            {
                                for(nChnInfo = 0; nChnInfo < WNI_CFG_VALID_CHANNEL_LIST_LEN; nChnInfo++)
                                {
                                    if(p11dLog->Channels[nTmp] == chnPwrInfo[nChnInfo].chanId)
                                    {
                                        p11dLog->TxPwr[nTmp] = chnPwrInfo[nChnInfo].pwr;
                                        break;
                                    }
                                }
                            }
                        }
                        if(!pMac->roam.configParam.Is11dSupportEnabled)
                        {
                            p11dLog->supportMultipleDomain = WLAN_80211D_DISABLED;
                        }
                        else if(pMac->roam.configParam.fEnforceDefaultDomain)
                        {
                            p11dLog->supportMultipleDomain = WLAN_80211D_NOT_SUPPORT_MULTI_DOMAIN;
                        }
                        else
                        {
                            p11dLog->supportMultipleDomain = WLAN_80211D_SUPPORT_MULTI_DOMAIN;
                        }
                        WLAN_VOS_DIAG_LOG_REPORT(p11dLog);
                    }
                }
#endif //                                    
                if(pMac->scan.domainIdCurrent != domainId)
                {
                   /*                                                   
                                                                       
                                  
                      */
                   smsLog(pMac, LOGW, FL("Domain Changed Old %d, new %d"),
                                      pMac->scan.domainIdCurrent, domainId);
                   csrScanFilter11dResult(pMac);
                }
                status = WDA_SetRegDomain(pMac, domainId);
                if (status != eHAL_STATUS_SUCCESS)
                {
                    smsLog( pMac, LOGE, FL("  fail to set regId %d\n"), domainId );
                }
                pMac->scan.domainIdCurrent = domainId;
                csrApplyChannelPowerCountryInfo( pMac, &pMac->scan.channels11d, pMac->scan.countryCode11d );
                //                                                   
                pMac->scan.curScanType = eSIR_ACTIVE_SCAN;
                pMac->scan.f11dInfoApplied = eANI_BOOLEAN_TRUE;
                pMac->scan.f11dInfoReset = eANI_BOOLEAN_FALSE;
            }
        }

    } while( 0 );

    return;
}



tANI_BOOLEAN csrSave11dCountryString( tpAniSirGlobal pMac, tANI_U8 *pCountryCode,
                     tANI_BOOLEAN fForce)
{
    tANI_BOOLEAN fCountryStringChanged = FALSE, fUnknownCountryCode = FALSE;
    tANI_U32 i;

    //                                                                              
    for( i = 0; i < 3; i++ )
    {
        pCountryCode[ i ] = (tANI_U8)csrToUpper( pCountryCode[ i ] );
    }

    //                                                                                                 
    //                                                                                          
    //                                                                                                   
    //                                                                                                 
    //                                              
    if ( !HAL_STATUS_SUCCESS(csrGetRegulatoryDomainForCountry( pMac, pCountryCode, NULL ) ) )
    {
        //                            
        if( pMac->roam.configParam.fEnforceDefaultDomain || pMac->roam.configParam.fEnforceCountryCodeMatch )
        {
            fUnknownCountryCode = TRUE;
        }
        else
        {
            pCountryCode[ 0 ] = 'U';
            pCountryCode[ 1 ] = 'S';
        }
    }

    //                                                                                            
    //                                                                                             
    //                                                                 
    if ( 0 == pCountryCode[ 2 ] )
    {
        pCountryCode[ 2 ] = ' ';
    }

    if( !fUnknownCountryCode )
    {
        fCountryStringChanged = (!palEqualMemory( pMac->hHdd,
              pMac->scan.countryCode11d, pCountryCode, 2));


        if(( 0 == pMac->scan.countryCode11d[ 0 ] && 0 == pMac->scan.countryCode11d[ 1 ] )
             || (fForce))
        {
            //                                   
            palCopyMemory( pMac->hHdd, pMac->scan.countryCode11d, pCountryCode, sizeof( pMac->scan.countryCode11d ) );
        }
    }

    return( fCountryStringChanged );
}


void csrSaveChannelPowerForBand( tpAniSirGlobal pMac, tANI_BOOLEAN fPopulate5GBand )
{
    tANI_U32 Index, count=0;
    tSirMacChanInfo *pChanInfo;
    tSirMacChanInfo *pChanInfoStart;

    if(HAL_STATUS_SUCCESS(palAllocateMemory(pMac->hHdd, (void **)&pChanInfo, sizeof(tSirMacChanInfo) * WNI_CFG_VALID_CHANNEL_LIST_LEN)))
    {
        palZeroMemory(pMac->hHdd, pChanInfo, sizeof(tSirMacChanInfo) * WNI_CFG_VALID_CHANNEL_LIST_LEN);
        pChanInfoStart = pChanInfo;
        for (Index=0; Index < pMac->scan.base20MHzChannels.numChannels; Index++)
        {
            if ((fPopulate5GBand && (CSR_IS_CHANNEL_5GHZ(pMac->scan.defaultPowerTable[Index].chanId))) ||
                (!fPopulate5GBand && (CSR_IS_CHANNEL_24GHZ(pMac->scan.defaultPowerTable[Index].chanId))) )
            {
                pChanInfo->firstChanNum = pMac->scan.defaultPowerTable[Index].chanId;
                pChanInfo->numChannels  = 1;
                pChanInfo->maxTxPower   = CSR_ROAM_MIN( pMac->scan.defaultPowerTable[Index].pwr, pMac->roam.configParam.nTxPowerCap );
                pChanInfo++;
                count++;
            }
        }
        if(count)
        {
            csrSaveToChannelPower2G_5G( pMac, count * sizeof(tSirMacChanInfo), pChanInfoStart );
        }
        palFreeMemory(pMac->hHdd, pChanInfoStart);
    }
}


void csrSetOppositeBandChannelInfo( tpAniSirGlobal pMac )
{
    tANI_BOOLEAN fPopulate5GBand = FALSE;

    do 
    {
        //                                                                           
        //                                                                           
        if ( !CSR_IS_OPEARTING_DUAL_BAND( pMac ) ) break;
        //                                                
        if ( CSR_IS_CHANNEL_5GHZ( pMac->scan.channelOf11dInfo ) )
        {
            //                                                              
            if ( !csrLLIsListEmpty( &pMac->scan.channelPowerInfoList24, LL_ACCESS_LOCK ) ) break;
            fPopulate5GBand = FALSE;
        }
        else
        {
            //                                                                           
            //                                                 
            if ( !csrLLIsListEmpty( &pMac->scan.channelPowerInfoList5G, LL_ACCESS_LOCK ) ) break;
            fPopulate5GBand = TRUE;
        }
        csrSaveChannelPowerForBand( pMac, fPopulate5GBand );

    } while( 0 );
}


tANI_BOOLEAN csrIsSupportedChannel(tpAniSirGlobal pMac, tANI_U8 channelId)
{
    tANI_BOOLEAN fRet = eANI_BOOLEAN_FALSE;
    tANI_U32 i;

    //                                                        
    for ( i = 0; i < pMac->scan.baseChannels.numChannels; i++ )
    {
        if ( channelId == pMac->scan.baseChannels.channelList[i] )
        {
            fRet = eANI_BOOLEAN_TRUE;
            break;
        }
    }

    //                                                  
    if( fRet && pMac->roam.configParam.fEnforce11dChannels )
    {
        fRet = eANI_BOOLEAN_FALSE;
        for ( i = 0; i < pMac->scan.base20MHzChannels.numChannels; i++ )
        {
            if ( channelId == pMac->scan.base20MHzChannels.channelList[i] )
            {
                fRet = eANI_BOOLEAN_TRUE;
                break;
            }
        }
    }

    return (fRet);
}



//                                             
tANI_U8 csrGetChannelListFromChannelSet( tpAniSirGlobal pMac, tANI_U8 *pChannelList, tANI_U8 bSize, tCsrChannelPowerInfo *pChannelSet )
{
    tANI_U8 i, j = 0, chnId;

    bSize = CSR_MIN(bSize, pChannelSet->numChannels);
    for( i = 0; i < bSize; i++ )
    {
        chnId = (tANI_U8)(pChannelSet->firstChannel + ( i * pChannelSet->interChannelOffset ));
        if ( csrIsSupportedChannel( pMac, chnId ) )
        {
            pChannelList[j++] = chnId;
        }
    }

    return (j);
}



//                                                
void csrConstructCurrentValidChannelList( tpAniSirGlobal pMac, tDblLinkList *pChannelSetList, 
                                            tANI_U8 *pChannelList, tANI_U8 bSize, tANI_U8 *pNumChannels )
{
    tListElem *pEntry;
    tCsrChannelPowerInfo *pChannelSet;
    tANI_U8 numChannels;
    tANI_U8 *pChannels;

    if( pChannelSetList && pChannelList && pNumChannels )
    {
        pChannels = pChannelList;
        *pNumChannels = 0;
        pEntry = csrLLPeekHead( pChannelSetList, LL_ACCESS_LOCK );
        while( pEntry )
        {
            pChannelSet = GET_BASE_ADDR( pEntry, tCsrChannelPowerInfo, link );
            numChannels = csrGetChannelListFromChannelSet( pMac, pChannels, bSize, pChannelSet );
            pChannels += numChannels;
            *pNumChannels += numChannels;
            pEntry = csrLLNext( pChannelSetList, pEntry, LL_ACCESS_LOCK );
        }
    }
}


/*
                                                                                                    
*/
tANI_BOOLEAN csrLearnCountryInformation( tpAniSirGlobal pMac, tSirBssDescription *pSirBssDesc,
                                         tDot11fBeaconIEs *pIes, tANI_BOOLEAN fForce)
{
    tANI_U8 Num2GChannels, bMaxNumChn;
    eHalStatus status;
    tANI_BOOLEAN fRet = eANI_BOOLEAN_FALSE;
    v_REGDOMAIN_t domainId;
    tDot11fBeaconIEs *pIesLocal = pIes;

#ifdef WLAN_SOFTAP_FEATURE
    if (VOS_STA_SAP_MODE == vos_get_conparam ())
        return eHAL_STATUS_SUCCESS;
#endif

    do
    {
        //                                 
        if( !csrIs11dSupported( pMac ) ) break;
        if( !pIesLocal && (!HAL_STATUS_SUCCESS(csrGetParsedBssDescriptionIEs(pMac, pSirBssDesc, &pIesLocal))) )
        {
            break;
        }
        //                                                
        if(!pIesLocal->Country.present)
        {
            //               
            break;
        }

        if( csrSave11dCountryString( pMac, pIesLocal->Country.country, fForce ) )
        {
            //                                               
            //                                                         
            //                                                                                
            tANI_U32 i;
            tCsrRoamSession *pSession;

            for( i = 0; i < CSR_ROAM_SESSION_MAX; i++ )
            {
                if( CSR_IS_SESSION_VALID( pMac, i ) )
                {
                    pSession = CSR_GET_SESSION( pMac, i );
                    if(pSession->pCurRoamProfile)
                    {
                        tCsrScanResultFilter filter;

                        palZeroMemory(pMac->hHdd, &filter, sizeof(tCsrScanResultFilter));
                        status = csrRoamPrepareFilterFromProfile(pMac, pSession->pCurRoamProfile, &filter);
                        if(HAL_STATUS_SUCCESS(status))
                        {
                            tANI_BOOLEAN fMatch = csrMatchBSS(pMac, pSirBssDesc, &filter, NULL, NULL, NULL, NULL);
                            //                       
                            csrFreeScanFilter( pMac, &filter );
                            if(fMatch)
                            {
                                smsLog(pMac, LOGW, "   Matching roam profile BSSID %02X-%02X-%02X-%02X-%02X-%02X causing ambiguous domain info\n",
                                    pSirBssDesc->bssId[0], pSirBssDesc->bssId[1], pSirBssDesc->bssId[2], 
                                    pSirBssDesc->bssId[3], pSirBssDesc->bssId[4], pSirBssDesc->bssId[5]);
                                pMac->scan.fAmbiguous11dInfoFound = eANI_BOOLEAN_TRUE;
                                break;
                            }
                        }
                    }
                    else if( csrIsConnStateConnected(pMac, i))
                    {
                        //                                                           
                        //                                                       
                        if(csrMatchBSSToConnectProfile(pMac, &pSession->connectedProfile, pSirBssDesc, pIesLocal))
                        {
                            smsLog(pMac, LOGW, "   Matching connect profile BSSID %02X-%02X-%02X-%02X-%02X-%02X causing ambiguous domain info\n",
                                pSirBssDesc->bssId[0], pSirBssDesc->bssId[1], pSirBssDesc->bssId[2],
                                pSirBssDesc->bssId[3], pSirBssDesc->bssId[4], pSirBssDesc->bssId[5]);
                            //    
                            pMac->scan.fAmbiguous11dInfoFound = eANI_BOOLEAN_TRUE;
                            if(csrIsBssidMatch(pMac, (tCsrBssid *)&pSirBssDesc->bssId, 
                                                &pSession->connectedProfile.bssid))
                            {
                                //                                              
                                pMac->scan.fAmbiguous11dInfoFound = eANI_BOOLEAN_FALSE;
                                fRet = eANI_BOOLEAN_TRUE;
                            }
                            break;
                        }
                    }
                } //             
            } //   
            if ( i == CSR_ROAM_SESSION_MAX ) 
            {
                //                                                       
                if( !pMac->roam.configParam.fEnforceDefaultDomain )
                {
                    pMac->scan.fAmbiguous11dInfoFound = eANI_BOOLEAN_TRUE;
                }
                else 
                {
                    VOS_ASSERT( pMac->scan.domainIdCurrent == pMac->scan.domainIdDefault );
                    if( HAL_STATUS_SUCCESS(csrGetRegulatoryDomainForCountry( 
                                pMac, pIesLocal->Country.country, &domainId )) &&
                                ( domainId == pMac->scan.domainIdCurrent ) )
                    {
                        //                                
                    }
                }
            }
        }
        else //    
        {
            pMac->scan.fCurrent11dInfoMatch = eANI_BOOLEAN_TRUE;
        }

        //                                                                                 
        if(CSR_IS_CHANNEL_5GHZ(pSirBssDesc->channelId))
        {
            tANI_U8 iC;
            tSirMacChanInfo* pMacChnSet = (tSirMacChanInfo *)(&pIesLocal->Country.triplets[0]);

            for(iC = 0; iC < pIesLocal->Country.num_triplets; iC++)
            {
                if(CSR_IS_CHANNEL_24GHZ(pMacChnSet[iC].firstChanNum))
                {
                    pMacChnSet[iC].firstChanNum += 200; //                              
                }
            }
        }
        smsLog(pMac, LOG3, FL("  %d sets each one is %d\n"), pIesLocal->Country.num_triplets, sizeof(tSirMacChanInfo));
        //                                                        
        //                                   
        if (eHAL_STATUS_SUCCESS != csrSaveToChannelPower2G_5G( pMac, pIesLocal->Country.num_triplets * sizeof(tSirMacChanInfo),
                    (tSirMacChanInfo *)(&pIesLocal->Country.triplets[0]) ))
        {
            fRet = eANI_BOOLEAN_FALSE;
            return fRet;
        }

        //                                                                   
        pMac->scan.channelOf11dInfo = pSirBssDesc->channelId;
        //                                                                                      
        csrSetOppositeBandChannelInfo( pMac );
        bMaxNumChn = WNI_CFG_VALID_CHANNEL_LIST_LEN;
        //                                  
        csrConstructCurrentValidChannelList( pMac, &pMac->scan.channelPowerInfoList24, pMac->scan.channels11d.channelList, 
                                                bMaxNumChn, &Num2GChannels );
        //                                
        if(bMaxNumChn > Num2GChannels)
        {
            csrConstructCurrentValidChannelList( pMac, &pMac->scan.channelPowerInfoList5G, pMac->scan.channels11d.channelList + Num2GChannels,
                                                 bMaxNumChn - Num2GChannels,
                                                 &pMac->scan.channels11d.numChannels );
        }

        pMac->scan.channels11d.numChannels += Num2GChannels;
        fRet = eANI_BOOLEAN_TRUE;

    } while( 0 );
    
    if( !pIes && pIesLocal )
    {
        //                 
        palFreeMemory(pMac->hHdd, pIesLocal);
    }

    return( fRet );
}


static void csrSaveScanResults( tpAniSirGlobal pMac )
{
    //                                                                         
    //                                                          
    pMac->scan.channelOf11dInfo = 0;
    //                                                                       
    pMac->scan.fAmbiguous11dInfoFound = eANI_BOOLEAN_FALSE;
    //    
    //                                                                        
    //                                                                       
    pMac->scan.fCurrent11dInfoMatch = eANI_BOOLEAN_FALSE;
    //                                                              
    csrMoveTempScanResultsToMainList( pMac );

    //                                                                 
    //                                                                 
    if( csrIs11dSupported(pMac) )
    {
        csrApplyCountryInformation( pMac, FALSE );
    }
    else if( csrIs11hSupported(pMac) && !pMac->roam.configParam.fSupplicantCountryCodeHasPriority) 
    {
        //                                                                              
        csrApplyPower2Current( pMac );
    }     
}


void csrReinitScanCmd(tpAniSirGlobal pMac, tSmeCmd *pCommand)
{
    switch (pCommand->u.scanCmd.reason)
    {
    case eCsrScanSetBGScanParam:
    case eCsrScanAbortBgScan:
        if(pCommand->u.scanCmd.u.bgScanRequest.ChannelInfo.ChannelList)
        {
            palFreeMemory(pMac->hHdd, pCommand->u.scanCmd.u.bgScanRequest.ChannelInfo.ChannelList);
            pCommand->u.scanCmd.u.bgScanRequest.ChannelInfo.ChannelList = NULL;
        }
        break;
    case eCsrScanBGScanAbort:
    case eCsrScanBGScanEnable:
    case eCsrScanGetScanChnInfo:
        break;
    case eCsrScanAbortNormalScan:
    default:
        csrScanFreeRequest(pMac, &pCommand->u.scanCmd.u.scanRequest);
        break;
    }
    if(pCommand->u.scanCmd.pToRoamProfile)
    {
        csrReleaseProfile(pMac, pCommand->u.scanCmd.pToRoamProfile);
        palFreeMemory(pMac->hHdd, pCommand->u.scanCmd.pToRoamProfile);
    }
    palZeroMemory(pMac->hHdd, &pCommand->u.scanCmd, sizeof(tScanCmd));
}


tANI_BOOLEAN csrGetRemainingChannelsFor11dScan( tpAniSirGlobal pMac, tANI_U8 *pChannels, tANI_U8 *pcChannels )
{
    tANI_U32 index11dChannels, index;
    tANI_U32 indexCurrentChannels;
    tANI_BOOLEAN fChannelAlreadyScanned;
    tANI_U32 len = sizeof(pMac->roam.validChannelList);

    *pcChannels = 0;
    if ( CSR_IS_11D_INFO_FOUND(pMac) && csrRoamIsChannelValid(pMac, pMac->scan.channelOf11dInfo) )
    {
        if (HAL_STATUS_SUCCESS(csrGetCfgValidChannels(pMac, (tANI_U8 *)pMac->roam.validChannelList, &len)))
        {
            //                                                  
            for(index = 0; index < len; index++)
            {
                if(pMac->scan.channelOf11dInfo == pMac->roam.validChannelList[index])
                    break;
            }
            //                                        
            if(index < len)
            {
                //                                                                                                  
                //                                                                                              
                //                                                                                                           
                //           
                for ( index11dChannels = 0; index11dChannels < pMac->scan.channels11d.numChannels; index11dChannels++ )
                {
                    fChannelAlreadyScanned = eANI_BOOLEAN_FALSE;

                    for( indexCurrentChannels = 0; indexCurrentChannels < index; indexCurrentChannels++ )
                    {
                        if ( pMac->roam.validChannelList[ indexCurrentChannels ] == pMac->scan.channels11d.channelList[ index11dChannels ] )
                        {
                            fChannelAlreadyScanned = eANI_BOOLEAN_TRUE;
                            break;
                        }
                    }

                    if ( !fChannelAlreadyScanned )
                    {
                        pChannels[ *pcChannels ] = pMac->scan.channels11d.channelList[ index11dChannels ];
                        ( *pcChannels )++;
                    }
                }
            }
        }//      
    }
    return( *pcChannels );
}


eCsrScanCompleteNextCommand csrScanGetNextCommandState( tpAniSirGlobal pMac, tSmeCmd *pCommand, tANI_BOOLEAN fSuccess )
{
    eCsrScanCompleteNextCommand NextCommand = eCsrNextScanNothing;
    
    switch( pCommand->u.scanCmd.reason )
    {
        case eCsrScan11d1:
            NextCommand = (fSuccess) ? eCsrNext11dScan1Success : eCsrNext11dScan1Failure;
            break;
        case eCsrScan11d2:
            NextCommand = (fSuccess) ? eCsrNext11dScan2Success : eCsrNext11dScan2Failure;
            break;    
        case eCsrScan11dDone:
            NextCommand = eCsrNext11dScanComplete;
            break;
        case eCsrScanLostLink1:
            NextCommand = (fSuccess) ? eCsrNextLostLinkScan1Success : eCsrNextLostLinkScan1Failed;
            break;
        case eCsrScanLostLink2:
            NextCommand = (fSuccess) ? eCsrNextLostLinkScan2Success : eCsrNextLostLinkScan2Failed;
            break;
        case eCsrScanLostLink3:
            NextCommand = (fSuccess) ? eCsrNextLostLinkScan3Success : eCsrNextLostLinkScan3Failed;
            break;
        case eCsrScanForSsid:
            NextCommand = (fSuccess) ? eCsrNexteScanForSsidSuccess : eCsrNexteScanForSsidFailure;
            break;
        case eCsrScanForCapsChange:
            NextCommand = eCsrNextCapChangeScanComplete;    //                         
            break;
        case eCsrScanIdleScan:
            NextCommand = eCsrNextIdleScanComplete;
            break;
        default:
            NextCommand = eCsrNextScanNothing;
            break;
    }
    return( NextCommand );
}


//                                        
tANI_BOOLEAN csrHandleScan11d1Failure(tpAniSirGlobal pMac, tSmeCmd *pCommand)
{
    tANI_BOOLEAN fRet = eANI_BOOLEAN_TRUE;
    
    //                                                                
    csrResetCountryInformation(pMac, eANI_BOOLEAN_FALSE);
    pCommand->u.scanCmd.reason = eCsrScan11d2;
    if(HAL_STATUS_SUCCESS(csrScanChannels(pMac, pCommand)))
    {
        fRet = eANI_BOOLEAN_FALSE;
    }
    
    return (fRet);
}


tANI_BOOLEAN csrHandleScan11dSuccess(tpAniSirGlobal pMac, tSmeCmd *pCommand)
{
    tANI_BOOLEAN fRet = eANI_BOOLEAN_TRUE;
    tANI_U8 *pChannels;
    tANI_U8 cChannels;
    
    if(HAL_STATUS_SUCCESS(palAllocateMemory(pMac->hHdd, (void **)&pChannels, WNI_CFG_VALID_CHANNEL_LIST_LEN)))
    {
        palZeroMemory(pMac->hHdd, pChannels, WNI_CFG_VALID_CHANNEL_LIST_LEN);
        if ( csrGetRemainingChannelsFor11dScan( pMac, pChannels, &cChannels ) )
        {
            pCommand->u.scanCmd.reason = eCsrScan11dDone;
            if(pCommand->u.scanCmd.u.scanRequest.ChannelInfo.ChannelList)
            {
                palFreeMemory(pMac->hHdd, pCommand->u.scanCmd.u.scanRequest.ChannelInfo.ChannelList); 
            }
            if(HAL_STATUS_SUCCESS(palAllocateMemory(pMac->hHdd, (void **)&pCommand->u.scanCmd.u.scanRequest.ChannelInfo.ChannelList, cChannels)))
            {
                palCopyMemory(pMac->hHdd, pCommand->u.scanCmd.u.scanRequest.ChannelInfo.ChannelList, pChannels, cChannels);
                pCommand->u.scanCmd.u.scanRequest.ChannelInfo.numOfChannels = cChannels;
                pCommand->u.scanCmd.u.scanRequest.requestType = eCSR_SCAN_REQUEST_FULL_SCAN;
                pCommand->u.scanCmd.u.scanRequest.scanType = eSIR_ACTIVE_SCAN;
                if(HAL_STATUS_SUCCESS(csrScanChannels(pMac, pCommand)))
                {
                    //                             
                    fRet = eANI_BOOLEAN_FALSE;
                }
            }
        }
        palFreeMemory(pMac->hHdd, pChannels);
    }
    
    return (fRet);
}

//                                            
tANI_BOOLEAN csrScanComplete( tpAniSirGlobal pMac, tSirSmeScanRsp *pScanRsp )
{
    eCsrScanCompleteNextCommand NextCommand = eCsrNextScanNothing;
    tListElem *pEntry;
    tSmeCmd *pCommand;
    tANI_BOOLEAN fRemoveCommand = eANI_BOOLEAN_TRUE;
    tANI_BOOLEAN fSuccess;

    pEntry = csrLLPeekHead( &pMac->sme.smeCmdActiveList, LL_ACCESS_LOCK );

    if ( pEntry )
    {
        pCommand = GET_BASE_ADDR( pEntry, tSmeCmd, Link );

        //                                                                    
        //                                
        if ( eSmeCommandScan == pCommand->command )
        {     
            tANI_U32 sessionId = pCommand->sessionId;

            if(eSIR_SME_SUCCESS != pScanRsp->statusCode)
            {
                fSuccess = eANI_BOOLEAN_FALSE;
            }
            else
            {
                //                                                                        
                //                                                                       
                fSuccess = (!csrLLIsListEmpty(&pMac->scan.tempScanResults, LL_ACCESS_LOCK));
            }
            csrSaveScanResults(pMac);

#ifdef FEATURE_WLAN_DIAG_SUPPORT_CSR
            {
                vos_log_scan_pkt_type *pScanLog = NULL;
                tScanResultHandle hScanResult;
                tCsrScanResultInfo *pScanResult;
                tDot11fBeaconIEs *pIes;
                int n = 0, c = 0;

                WLAN_VOS_DIAG_LOG_ALLOC(pScanLog, vos_log_scan_pkt_type, LOG_WLAN_SCAN_C);
                if(pScanLog)
                {
                    if(eCsrScanBgScan == pCommand->u.scanCmd.reason || 
                        eCsrScanProbeBss == pCommand->u.scanCmd.reason ||
                        eCsrScanSetBGScanParam == pCommand->u.scanCmd.reason)
                    {
                        pScanLog->eventId = WLAN_SCAN_EVENT_HO_SCAN_RSP;
                    }
                    else
                    {
                        if( eSIR_PASSIVE_SCAN != pMac->scan.curScanType )
                        {
                            pScanLog->eventId = WLAN_SCAN_EVENT_ACTIVE_SCAN_RSP;
                        }
                        else
                        {
                            pScanLog->eventId = WLAN_SCAN_EVENT_PASSIVE_SCAN_RSP;
                        }
                    }
                    if(eSIR_SME_SUCCESS == pScanRsp->statusCode)
                    {
                        if(HAL_STATUS_SUCCESS(csrScanGetResult(pMac, NULL, &hScanResult)))
                        {
                            while(((pScanResult = csrScanResultGetNext(pMac, hScanResult)) != NULL))
                            {
                                if( n < VOS_LOG_MAX_NUM_BSSID )
                                {
                                    if(!HAL_STATUS_SUCCESS(csrGetParsedBssDescriptionIEs(pMac, &pScanResult->BssDescriptor, &pIes)))
                                    {
                                        smsLog(pMac, LOGE, FL(" fail to parse IEs\n"));
                                        break;
                                    }
                                    palCopyMemory(pMac->hHdd, pScanLog->bssid[n], pScanResult->BssDescriptor.bssId, 6);
                                    if(pIes && pIes->SSID.present && VOS_LOG_MAX_SSID_SIZE >= pIes->SSID.num_ssid)
                                    {
                                        palCopyMemory(pMac->hHdd, pScanLog->ssid[n], 
                                                pIes->SSID.ssid, pIes->SSID.num_ssid);
                                    }
                                    palFreeMemory(pMac->hHdd, pIes);
                                    n++;
                                }
                                c++;
                            }
                            pScanLog->numSsid = (v_U8_t)n;
                            pScanLog->totalSsid = (v_U8_t)c;
                            csrScanResultPurge(pMac, hScanResult);
                        }
                    }
                    else
                    {
                        pScanLog->status = WLAN_SCAN_STATUS_FAILURE;
                    }
                    WLAN_VOS_DIAG_LOG_REPORT(pScanLog);
                }
            }
#endif //                                    

            NextCommand = csrScanGetNextCommandState(pMac, pCommand, fSuccess);
            //                                                       
            switch(NextCommand)
            {
            case eCsrNext11dScan1Success:
            case eCsrNext11dScan2Success:
                smsLog( pMac, LOG2, FL("11dScan1/3 produced results.  Reissue Active scan...\n"));
                //                                                                                
                fRemoveCommand = eANI_BOOLEAN_TRUE;
                NextCommand = eCsrNext11dScanComplete;
                break;
            case eCsrNext11dScan1Failure:
                //                                                                                           
                //                                                                                                
                //                                                        
                fRemoveCommand = csrHandleScan11d1Failure(pMac, pCommand);
                if(fRemoveCommand)
                {
                    NextCommand = eCsrNext11dScanComplete;
                } 
                break;
            case eCsrNextLostLinkScan1Success:
                if(!HAL_STATUS_SUCCESS(csrIssueRoamAfterLostlinkScan(pMac, sessionId, eCsrLostLink1)))
                {
                    csrScanHandleFailedLostlink1(pMac, sessionId);
                }
                break;
            case eCsrNextLostLinkScan2Success:
                if(!HAL_STATUS_SUCCESS(csrIssueRoamAfterLostlinkScan(pMac, sessionId, eCsrLostLink2)))
                {
                    csrScanHandleFailedLostlink2(pMac, sessionId);
                }
                break;
            case eCsrNextLostLinkScan3Success:
                if(!HAL_STATUS_SUCCESS(csrIssueRoamAfterLostlinkScan(pMac, sessionId, eCsrLostLink3)))
                {
                    csrScanHandleFailedLostlink3(pMac, sessionId);
                }
                break;
            case eCsrNextLostLinkScan1Failed:
                csrScanHandleFailedLostlink1(pMac, sessionId);
                break;
            case eCsrNextLostLinkScan2Failed:
                csrScanHandleFailedLostlink2(pMac, sessionId);
                break;
            case eCsrNextLostLinkScan3Failed:
                csrScanHandleFailedLostlink3(pMac, sessionId);
                break;    
            case eCsrNexteScanForSsidSuccess:
                csrScanHandleSearchForSSID(pMac, pCommand);
                break;
            case eCsrNexteScanForSsidFailure:
                csrScanHandleSearchForSSIDFailure(pMac, pCommand);
                break;
            case eCsrNextIdleScanComplete:
                pMac->scan.fRestartIdleScan = eANI_BOOLEAN_TRUE;
                break;
            case eCsrNextCapChangeScanComplete:
                csrScanHandleCapChangeScanComplete(pMac, sessionId);
                break;
            default:

                break;
            }
        }
        else
        {
            smsLog( pMac, LOGW, FL("Scan Completion called but SCAN command is not ACTIVE ...\n"));
            fRemoveCommand = eANI_BOOLEAN_FALSE;
        }
    }
    else
    {
        smsLog( pMac, LOGW, FL("Scan Completion called but NO commands are ACTIVE ...\n"));
        fRemoveCommand = eANI_BOOLEAN_FALSE;
    }
   
    return( fRemoveCommand );
}



static void csrScanRemoveDupBssDescriptionFromInterimList( tpAniSirGlobal pMac, 
                                                           tSirBssDescription *pSirBssDescr,
                                                           tDot11fBeaconIEs *pIes)
{
    tListElem *pEntry;
    tCsrScanResult *pCsrBssDescription;

    //                                                                                        
    //                                                                                     
    //                                                                                            
    pEntry = csrLLPeekHead( &pMac->scan.tempScanResults, LL_ACCESS_LOCK );
    while( pEntry ) 
    {
        pCsrBssDescription = GET_BASE_ADDR( pEntry, tCsrScanResult, Link );

        //                                                                                
        //        

        if ( csrIsDuplicateBssDescription( pMac, &pCsrBssDescription->Result.BssDescriptor, 
                                             pSirBssDescr, pIes ) )
        {
            pSirBssDescr->rssi = (tANI_S8)( (((tANI_S32)pSirBssDescr->rssi * CSR_SCAN_RESULT_RSSI_WEIGHT ) +
                                    ((tANI_S32)pCsrBssDescription->Result.BssDescriptor.rssi * (100 - CSR_SCAN_RESULT_RSSI_WEIGHT) )) / 100 );

            //                                         
            if( csrLLRemoveEntry( &pMac->scan.tempScanResults, pEntry, LL_ACCESS_LOCK ) )
            {
                csrCheckNSaveWscIe(pMac, pSirBssDescr, &pCsrBssDescription->Result.BssDescriptor);
                //                                                     
                csrFreeScanResultEntry( pMac, pCsrBssDescription );
            }
            
            //                                                           
            break;
        }

        pEntry = csrLLNext( &pMac->scan.tempScanResults, pEntry, LL_ACCESS_LOCK );
    }
}



//                                                                           
//                                           
tCsrScanResult *csrScanSaveBssDescriptionToInterimList( tpAniSirGlobal pMac, 
                                                        tSirBssDescription *pBSSDescription,
                                                        tDot11fBeaconIEs *pIes)
{
    tCsrScanResult *pCsrBssDescription = NULL;
    tANI_U32 cbBSSDesc;
    tANI_U32 cbAllocated;
    eHalStatus halStatus;
    
    //                                                                        
    //                                              
    cbBSSDesc = pBSSDescription->length + sizeof( pBSSDescription->length );

    cbAllocated = sizeof( tCsrScanResult ) + cbBSSDesc;

    halStatus = palAllocateMemory( pMac->hHdd, (void **)&pCsrBssDescription, cbAllocated );
    if ( HAL_STATUS_SUCCESS(halStatus) )
    {
        palZeroMemory(pMac->hHdd, pCsrBssDescription, cbAllocated);
        pCsrBssDescription->AgingCount = (tANI_S32)pMac->roam.configParam.agingCount;
        palCopyMemory(pMac->hHdd, &pCsrBssDescription->Result.BssDescriptor, pBSSDescription, cbBSSDesc );
        //                                  
        if( pIes->SSID.present && !csrIsNULLSSID(pIes->SSID.ssid, pIes->SSID.num_ssid) )
        {
            //               
            tANI_U32 len = pIes->SSID.num_ssid;;
            if (len > SIR_MAC_MAX_SSID_LENGTH)
            {
               //                              
               len = SIR_MAC_MAX_SSID_LENGTH;
            }
            pCsrBssDescription->Result.ssId.length = len;
            pCsrBssDescription->Result.timer = vos_timer_get_system_time();
            palCopyMemory(pMac->hHdd, pCsrBssDescription->Result.ssId.ssId, 
                pIes->SSID.ssid, len );
        }
        csrLLInsertTail( &pMac->scan.tempScanResults, &pCsrBssDescription->Link, LL_ACCESS_LOCK );
    }

    return( pCsrBssDescription );
}


    

tANI_BOOLEAN csrIsDuplicateBssDescription( tpAniSirGlobal pMac, tSirBssDescription *pSirBssDesc1, 
                                           tSirBssDescription *pSirBssDesc2, tDot11fBeaconIEs *pIes2 )
{
    tANI_BOOLEAN fMatch = FALSE;
    tSirMacCapabilityInfo *pCap1, *pCap2;
    tDot11fBeaconIEs *pIes1 = NULL;
    tDot11fBeaconIEs *pIesTemp = pIes2;

    pCap1 = (tSirMacCapabilityInfo *)&pSirBssDesc1->capabilityInfo;
    pCap2 = (tSirMacCapabilityInfo *)&pSirBssDesc2->capabilityInfo;
    if(pCap1->ess == pCap2->ess)
    {
        if (pCap1->ess && 
                csrIsMacAddressEqual( pMac, (tCsrBssid *)pSirBssDesc1->bssId, (tCsrBssid *)pSirBssDesc2->bssId)&&
                (pSirBssDesc1->channelId == pSirBssDesc2->channelId))
        {
            fMatch = TRUE;
            //                                
            do
            {
                if(!HAL_STATUS_SUCCESS(csrGetParsedBssDescriptionIEs(pMac, pSirBssDesc1, &pIes1)))
                {
                    break;
                }
                if( NULL == pIesTemp )
                {
                    if(!HAL_STATUS_SUCCESS(csrGetParsedBssDescriptionIEs(pMac, pSirBssDesc2, &pIesTemp)))
                    {
                        break;
                    }
                }
                if(pIes1->SSID.present && pIesTemp->SSID.present)
                {
                    fMatch = csrIsSsidMatch(pMac, pIes1->SSID.ssid, pIes1->SSID.num_ssid, 
                                            pIesTemp->SSID.ssid, pIesTemp->SSID.num_ssid, eANI_BOOLEAN_TRUE);
                } 
            }while(0);

        }
        else if (pCap1->ibss && (pSirBssDesc1->channelId == pSirBssDesc2->channelId))
        {

            do
            {
                if(!HAL_STATUS_SUCCESS(csrGetParsedBssDescriptionIEs(pMac, pSirBssDesc1, &pIes1)))
                {
                    break;
                }
                if( NULL == pIesTemp )
                {
                    if(!HAL_STATUS_SUCCESS(csrGetParsedBssDescriptionIEs(pMac, pSirBssDesc2, &pIesTemp)))
                    {
                        break;
                    }
                }
                //                                                     
                if(pIes1->SSID.present && pIesTemp->SSID.present)
                {
                    fMatch = csrIsSsidMatch(pMac, pIes1->SSID.ssid, pIes1->SSID.num_ssid, 
                                            pIesTemp->SSID.ssid, pIesTemp->SSID.num_ssid, eANI_BOOLEAN_TRUE);
                }
            }while(0);
        }
#if defined WLAN_FEATURE_P2P
        /*                                                          */
        else if (!pCap1->ess && 
                csrIsMacAddressEqual( pMac, (tCsrBssid *)pSirBssDesc1->bssId, (tCsrBssid *)pSirBssDesc2->bssId))
        {
            fMatch = TRUE;
        }
#endif
    }

    if(pIes1)
    {
        palFreeMemory(pMac->hHdd, pIes1);
    }
    
    if( (NULL == pIes2) && pIesTemp )
    {
        //                 
        palFreeMemory(pMac->hHdd, pIesTemp);
    }

    return( fMatch );
}


tANI_BOOLEAN csrIsNetworkTypeEqual( tSirBssDescription *pSirBssDesc1, tSirBssDescription *pSirBssDesc2 )
{
    return( pSirBssDesc1->nwType == pSirBssDesc2->nwType );
}


//                                              
static tANI_BOOLEAN csrScanIsBssAllowed(tpAniSirGlobal pMac, tSirBssDescription *pBssDesc, 
                                        tDot11fBeaconIEs *pIes)
{
    tANI_BOOLEAN fAllowed = eANI_BOOLEAN_FALSE;
    eCsrPhyMode phyMode;

    if(HAL_STATUS_SUCCESS(csrGetPhyModeFromBss(pMac, pBssDesc, &phyMode, pIes)))
    {
        switch(pMac->roam.configParam.phyMode)
        {
        case eCSR_DOT11_MODE_11b:
            fAllowed = (tANI_BOOLEAN)(eCSR_DOT11_MODE_11a != phyMode);
            break;
        case eCSR_DOT11_MODE_11g:
            fAllowed = (tANI_BOOLEAN)(eCSR_DOT11_MODE_11a != phyMode);
            break;
        case eCSR_DOT11_MODE_11g_ONLY:
            fAllowed = (tANI_BOOLEAN)(eCSR_DOT11_MODE_11g == phyMode);
            break;
        case eCSR_DOT11_MODE_11a:
            fAllowed = (tANI_BOOLEAN)((eCSR_DOT11_MODE_11b != phyMode) && (eCSR_DOT11_MODE_11g != phyMode));
            break;
        case eCSR_DOT11_MODE_11n_ONLY:
            fAllowed = (tANI_BOOLEAN)((eCSR_DOT11_MODE_11n == phyMode) || (eCSR_DOT11_MODE_TAURUS == phyMode));
            break;

#ifdef WLAN_FEATURE_11AC
         case eCSR_DOT11_MODE_11ac_ONLY:
             fAllowed = (tANI_BOOLEAN)((eCSR_DOT11_MODE_11ac == phyMode) || (eCSR_DOT11_MODE_TAURUS == phyMode));
             break;
#endif
        case eCSR_DOT11_MODE_11b_ONLY:
            fAllowed = (tANI_BOOLEAN)(eCSR_DOT11_MODE_11b == phyMode);
            break;
        case eCSR_DOT11_MODE_11a_ONLY:
            fAllowed = (tANI_BOOLEAN)(eCSR_DOT11_MODE_11a == phyMode);
            break;
        case eCSR_DOT11_MODE_11n:
#ifdef WLAN_FEATURE_11AC
        case eCSR_DOT11_MODE_11ac:
#endif
        case eCSR_DOT11_MODE_TAURUS:
        default:
            fAllowed = eANI_BOOLEAN_TRUE;
            break;
        }
    }

    return (fAllowed);
}



//                                                         
static tANI_BOOLEAN csrScanValidateScanResult( tpAniSirGlobal pMac, tANI_U8 *pChannels, 
                                               tANI_U8 numChn, tSirBssDescription *pBssDesc, 
                                               tDot11fBeaconIEs **ppIes )
{
    tANI_BOOLEAN fValidChannel = FALSE;
    tDot11fBeaconIEs *pIes = NULL;
    tANI_U8 index;

    for( index = 0; index < numChn; index++ )
    {
        //                                                                                
        //                                                                                       
        //                                                                                     
        //                                                                                     
        //                                             
        if ( pChannels[ index ] == pBssDesc->channelId ) 
        {
           fValidChannel = TRUE;
           break;
        }
    }
    *ppIes = NULL;
    if(fValidChannel)
    {
        if( HAL_STATUS_SUCCESS( csrGetParsedBssDescriptionIEs(pMac, pBssDesc, &pIes) ) )
        {
            fValidChannel = csrScanIsBssAllowed(pMac, pBssDesc, pIes);
            if( fValidChannel )
            {
                *ppIes = pIes;
            }
            else
            {
                palFreeMemory( pMac->hHdd, pIes );
            }
        }
        else
        {
            fValidChannel = FALSE;
        }
    }

    return( fValidChannel );   
}


//                                           
static tANI_BOOLEAN csrScanProcessScanResults( tpAniSirGlobal pMac, tSmeCmd *pCommand, 
                                                tSirSmeScanRsp *pScanRsp, tANI_BOOLEAN *pfRemoveCommand )
{
    tANI_BOOLEAN fRet = eANI_BOOLEAN_FALSE, fRemoveCommand = eANI_BOOLEAN_FALSE;
    tDot11fBeaconIEs *pIes = NULL;
    tANI_U32 cbParsed;
    tSirBssDescription *pSirBssDescription;
    tANI_U32 cbBssDesc;
    tANI_U32 cbScanResult = GET_FIELD_OFFSET( tSirSmeScanRsp, bssDescription ) 
                            + sizeof(tSirBssDescription);    //                       

    //                                                                                          
    //                                                                                          
    //                                                                                       
    //                    
    do
    {
        if ( ( cbScanResult <= pScanRsp->length ) && 
             (( eSIR_SME_SUCCESS == pScanRsp->statusCode ) ||
              ( eSIR_SME_MORE_SCAN_RESULTS_FOLLOW == pScanRsp->statusCode ) ) )
        {
            tANI_U8 *pChannelList = NULL;
            tANI_U8 cChannels = 0;

            //                                                                   
            if( eCsrScanSetBGScanParam == pCommand->u.scanCmd.reason )
            {
                //                                               
                tCsrBGScanRequest *pBgScanReq = &pCommand->u.scanCmd.u.bgScanRequest;

                cChannels = pBgScanReq->ChannelInfo.numOfChannels;
                pChannelList = pBgScanReq->ChannelInfo.ChannelList;
            }
            else
            {
                //                                 
                cChannels = pCommand->u.scanCmd.u.scanRequest.ChannelInfo.numOfChannels;
                pChannelList = pCommand->u.scanCmd.u.scanRequest.ChannelInfo.ChannelList;
            }

            //                                                                                    
            //                                                                                         
            //                            
            //
            //                                                                                        
            //                               
            
            //                                                                                                 
            //                                                                                              
            //                       
            if ( 0 == cChannels ) 
            {
                tANI_U32 len = sizeof(pMac->roam.validChannelList);
                
                if (HAL_STATUS_SUCCESS(csrGetCfgValidChannels(pMac, (tANI_U8 *)pMac->roam.validChannelList, &len)))
                {
                    pChannelList = pMac->roam.validChannelList;
                    cChannels = (tANI_U8)len; 
                }
                else
                {
                    //               
                    smsLog( pMac, LOGE, "CSR: Processing internal SCAN results...csrGetCfgValidChannels failed\n" );
                    break;
                }
            }

            smsLog( pMac, LOG2, "CSR: Processing internal SCAN results..." );
            cbParsed = GET_FIELD_OFFSET( tSirSmeScanRsp, bssDescription );
            pSirBssDescription = pScanRsp->bssDescription;
            while( cbParsed < pScanRsp->length )
            {
                if ( csrScanValidateScanResult( pMac, pChannelList, cChannels, pSirBssDescription, &pIes ) ) 
                {
                    csrScanRemoveDupBssDescriptionFromInterimList(pMac, pSirBssDescription, pIes);
                    csrScanSaveBssDescriptionToInterimList( pMac, pSirBssDescription, pIes );
                    if( eSIR_PASSIVE_SCAN == pMac->scan.curScanType )
                    {
                        if( csrIs11dSupported( pMac) )
                        {
                            //                                                                     
                            if( csrMatchCountryCode( pMac, NULL, pIes ) )
                            {
                                //                                                     
                                if( csrIsSupportedChannel( pMac, pSirBssDescription->channelId ) )
                                {
                                    pMac->scan.curScanType = eSIR_ACTIVE_SCAN;
                                }
                            }
                        }
                        else
                        {
                            pMac->scan.curScanType = eSIR_ACTIVE_SCAN;
                        }
                    }
                    //                 
                    palFreeMemory( pMac->hHdd, pIes );
                }
                //                                                 
                cbBssDesc = pSirBssDescription->length + sizeof( pSirBssDescription->length );

                cbParsed += cbBssDesc;
                pSirBssDescription = (tSirBssDescription *)((tANI_U8 *)pSirBssDescription + cbBssDesc );

            } //     
        }
        else
        {
            smsLog( pMac, LOGW, " Scanrsp fail (0x%08X), length = %d\n", pScanRsp->statusCode, pScanRsp->length );
            //                                                   
            if(eCsrScanBgScan == pCommand->u.scanCmd.reason ||
               eCsrScanProbeBss == pCommand->u.scanCmd.reason ||
               eCsrScanSetBGScanParam == pCommand->u.scanCmd.reason)
            {
                fRemoveCommand = eANI_BOOLEAN_TRUE;
            }
        }
    }while(0);
    if ( eSIR_SME_MORE_SCAN_RESULTS_FOLLOW != pScanRsp->statusCode )
    {
        smsLog(pMac, LOG1, " Scan received %d unique BSS scan reason is %d\n", csrLLCount(&pMac->scan.tempScanResults), pCommand->u.scanCmd.reason);
        fRemoveCommand = csrScanComplete( pMac, pScanRsp );
        fRet = eANI_BOOLEAN_TRUE;
    }//                                                                
    if(pfRemoveCommand)
    {
        *pfRemoveCommand = fRemoveCommand;
    }

#ifdef WLAN_AP_STA_CONCURRENCY
    if (!csrLLIsListEmpty( &pMac->scan.scanCmdPendingList, LL_ACCESS_LOCK ))
    {
         palTimerStart(pMac->hHdd, pMac->scan.hTimerStaApConcTimer, 
                 CSR_SCAN_STAAP_CONC_INTERVAL, eANI_BOOLEAN_FALSE);
    }
#endif
    return (fRet);
}


tANI_BOOLEAN csrScanIsWildCardScan( tpAniSirGlobal pMac, tSmeCmd *pCommand )
{
    tANI_U8 bssid[WNI_CFG_BSSID_LEN] = {0, 0, 0, 0, 0, 0};
    tANI_BOOLEAN f = palEqualMemory( pMac->hHdd, pCommand->u.scanCmd.u.scanRequest.bssid, 
        bssid, sizeof(tCsrBssid) );

    //                                                                                     
    return ((tANI_BOOLEAN)( (f || (0xff == pCommand->u.scanCmd.u.scanRequest.bssid[0])) &&
        (pCommand->u.scanCmd.u.scanRequest.SSIDs.numOfSSIDs != 1) ));
}


eHalStatus csrScanSmeScanResponse( tpAniSirGlobal pMac, void *pMsgBuf )
{
    eHalStatus status = eHAL_STATUS_SUCCESS;
    tListElem *pEntry;
    tSmeCmd *pCommand;
    eCsrScanStatus scanStatus;
    tSirSmeScanRsp *pScanRsp = (tSirSmeScanRsp *)pMsgBuf;
    tSmeGetScanChnRsp *pScanChnInfo;
    tANI_BOOLEAN fRemoveCommand = eANI_BOOLEAN_TRUE;
    eCsrScanReason reason = eCsrScanOther;

    pEntry = csrLLPeekHead( &pMac->sme.smeCmdActiveList, LL_ACCESS_LOCK );

    if ( pEntry )
    {
        pCommand = GET_BASE_ADDR( pEntry, tSmeCmd, Link );
        if ( eSmeCommandScan == pCommand->command )
        {
            scanStatus = (eSIR_SME_SUCCESS == pScanRsp->statusCode) ? eCSR_SCAN_SUCCESS : eCSR_SCAN_FAILURE;
            reason = pCommand->u.scanCmd.reason;
            switch(pCommand->u.scanCmd.reason)
            {
            case eCsrScanAbortBgScan:
            case eCsrScanAbortNormalScan:
            case eCsrScanBGScanAbort:
            case eCsrScanBGScanEnable:
                break;
            case eCsrScanGetScanChnInfo:
                pScanChnInfo = (tSmeGetScanChnRsp *)pMsgBuf;
                csrScanAgeResults(pMac, pScanChnInfo);
                break;
            case eCsrScanForCapsChange:
                csrScanProcessScanResults( pMac, pCommand, pScanRsp, &fRemoveCommand );
                break;
#if WLAN_FEATURE_P2P
            case eCsrScanP2PFindPeer:
              scanStatus = ((eSIR_SME_SUCCESS == pScanRsp->statusCode) && (pScanRsp->length > 50)) ? eCSR_SCAN_FOUND_PEER : eCSR_SCAN_FAILURE;
              csrScanProcessScanResults( pMac, pCommand, pScanRsp, NULL );
              break;
#endif                
            case eCsrScanSetBGScanParam:
            default:
                if(csrScanProcessScanResults( pMac, pCommand, pScanRsp, &fRemoveCommand ))
                {
                    //                                                                  
                    //                                                   
                    if( csrScanIsWildCardScan( pMac, pCommand ) && (!pCommand->u.scanCmd.u.scanRequest.p2pSearch) )
                    {
                        //                                
                       if( pCommand->u.scanCmd.reason != eCsrScanUserRequest)
                       {
                           csrScanGetScanChnInfo(pMac, NULL, NULL);
                       }
                       else
                       {
                           csrScanGetScanChnInfo(pMac, pCommand->u.scanCmd.callback, pCommand->u.scanCmd.pContext);
                           pCommand->u.scanCmd.callback = NULL;
                       }
                    }
                }
                break;
            }//      
            if(fRemoveCommand)
            {

                csrReleaseScanCommand(pMac, pCommand, scanStatus);

            }
            smeProcessPendingQueue( pMac );
        }
        else
        {
            smsLog( pMac, LOGW, "CSR: Scan Completion called but SCAN command is not ACTIVE ..." );
            status = eHAL_STATUS_FAILURE;
        }
    }
    else
    {
        smsLog( pMac, LOGW, "CSR: Scan Completion called but NO commands are ACTIVE ..." );
        status = eHAL_STATUS_FAILURE;
    }
    
    return (status);
}




tCsrScanResultInfo *csrScanResultGetFirst(tpAniSirGlobal pMac, tScanResultHandle hScanResult)
{
    tListElem *pEntry;
    tCsrScanResult *pResult;
    tCsrScanResultInfo *pRet = NULL;
    tScanResultList *pResultList = (tScanResultList *)hScanResult;
    
    if(pResultList)
    {
        csrLLLock(&pResultList->List);
        pEntry = csrLLPeekHead(&pResultList->List, LL_ACCESS_NOLOCK);
        if(pEntry)
        {
            pResult = GET_BASE_ADDR(pEntry, tCsrScanResult, Link);
            pRet = &pResult->Result;
        }
        pResultList->pCurEntry = pEntry;
        csrLLUnlock(&pResultList->List);
    }
    
    return pRet;
}


tCsrScanResultInfo *csrScanResultGetNext(tpAniSirGlobal pMac, tScanResultHandle hScanResult)
{
    tListElem *pEntry = NULL;
    tCsrScanResult *pResult = NULL;
    tCsrScanResultInfo *pRet = NULL;
    tScanResultList *pResultList = (tScanResultList *)hScanResult;
    
    if(pResultList)
    {
        csrLLLock(&pResultList->List);
        if(NULL == pResultList->pCurEntry)
        {
            pEntry = csrLLPeekHead(&pResultList->List, LL_ACCESS_NOLOCK);
        }
        else
        {
            pEntry = csrLLNext(&pResultList->List, pResultList->pCurEntry, LL_ACCESS_NOLOCK);
        }
        if(pEntry)
        {
            pResult = GET_BASE_ADDR(pEntry, tCsrScanResult, Link);
            pRet = &pResult->Result;
        }
        pResultList->pCurEntry = pEntry;
        csrLLUnlock(&pResultList->List);
    }
    
    return pRet;
}


//                                                                                  
eHalStatus csrMoveBssToHeadFromBSSID(tpAniSirGlobal pMac, tCsrBssid *bssid, tScanResultHandle hScanResult)
{
    eHalStatus status = eHAL_STATUS_FAILURE;
    tScanResultList *pResultList = (tScanResultList *)hScanResult;
    tCsrScanResult *pResult = NULL;
    tListElem *pEntry = NULL;
   
    if(pResultList && bssid)
    {
        csrLLLock(&pResultList->List);
        pEntry = csrLLPeekHead(&pResultList->List, LL_ACCESS_NOLOCK);
        while(pEntry)
        {
            pResult = GET_BASE_ADDR(pEntry, tCsrScanResult, Link);
            if(palEqualMemory(pMac->hHdd, bssid, pResult->Result.BssDescriptor.bssId, sizeof(tCsrBssid)))
            {
                status = eHAL_STATUS_SUCCESS;
                csrLLRemoveEntry(&pResultList->List, pEntry, LL_ACCESS_NOLOCK);
                csrLLInsertHead(&pResultList->List, pEntry, LL_ACCESS_NOLOCK);
                break;
            }
            pEntry = csrLLNext(&pResultList->List, pResultList->pCurEntry, LL_ACCESS_NOLOCK);
        }
        csrLLUnlock(&pResultList->List);
    }
    
    return (status);
}


//                           
//                                                               
//                                                                                      
tANI_BOOLEAN csrScanAgeOutBss(tpAniSirGlobal pMac, tCsrScanResult *pResult)
{
    tANI_BOOLEAN fRet = eANI_BOOLEAN_FALSE;
    tANI_U32 i;
    tCsrRoamSession *pSession;

    for( i = 0; i < CSR_ROAM_SESSION_MAX; i++ )
    {
        if( CSR_IS_SESSION_VALID( pMac, i ) )
        {
            pSession = CSR_GET_SESSION( pMac, i );
            //                                          
            if(csrIsConnStateDisconnected(pMac, i) || (NULL == pSession->pConnectBssDesc) ||
              (!csrIsDuplicateBssDescription(pMac, &pResult->Result.BssDescriptor, 
                                                      pSession->pConnectBssDesc, NULL))
              )
            {
                smsLog(pMac, LOG2, "Aging out BSS %02X-%02X-%02X-%02X-%02X-%02X Channel %d\n",
                                          pResult->Result.BssDescriptor.bssId[0],
                                          pResult->Result.BssDescriptor.bssId[1],
                                          pResult->Result.BssDescriptor.bssId[2],
                                          pResult->Result.BssDescriptor.bssId[3],
                                          pResult->Result.BssDescriptor.bssId[4],
                                          pResult->Result.BssDescriptor.bssId[5],
                                          pResult->Result.BssDescriptor.channelId);
                //                                                                                               
                if( csrLLRemoveEntry(&pMac->scan.scanResultList, &pResult->Link, LL_ACCESS_NOLOCK) )
                {
                    csrFreeScanResultEntry(pMac, pResult);
                }
                fRet = eANI_BOOLEAN_TRUE;
                break;
            }
        } //             
    } //   
    if( CSR_ROAM_SESSION_MAX == i )
    {
        //                                               
        pResult->AgingCount = (tANI_S32)pMac->roam.configParam.agingCount;
        pResult->Result.BssDescriptor.nReceivedTime = (tANI_TIMESTAMP)palGetTickCount(pMac->hHdd);
    }

    return (fRet);
}


eHalStatus csrScanAgeResults(tpAniSirGlobal pMac, tSmeGetScanChnRsp *pScanChnInfo)
{
    eHalStatus status = eHAL_STATUS_SUCCESS;
    tListElem *pEntry, *tmpEntry;
    tCsrScanResult *pResult;
    tLimScanChn *pChnInfo;
    tANI_U8 i;

    csrLLLock(&pMac->scan.scanResultList);
    for(i = 0; i < pScanChnInfo->numChn; i++)
    {
        pChnInfo = &pScanChnInfo->scanChn[i];
        pEntry = csrLLPeekHead( &pMac->scan.scanResultList, LL_ACCESS_NOLOCK );
        while( pEntry ) 
        {
            tmpEntry = csrLLNext(&pMac->scan.scanResultList, pEntry, LL_ACCESS_NOLOCK);
            pResult = GET_BASE_ADDR( pEntry, tCsrScanResult, Link );
            if(pResult->Result.BssDescriptor.channelId == pChnInfo->channelId)
            {
                if(pResult->AgingCount <= 0)
                {
                    smsLog(pMac, LOGW, " age out due to ref count");
                    csrScanAgeOutBss(pMac, pResult);
                }
                else
                {
                    pResult->AgingCount--;
                }
            }
            pEntry = tmpEntry;
        }
    }
    csrLLUnlock(&pMac->scan.scanResultList);

    return (status);
}


eHalStatus csrSendMBScanReq( tpAniSirGlobal pMac, tANI_U16 sessionId, 
                    tCsrScanRequest *pScanReq, tScanReqParam *pScanReqParam )
{
    eHalStatus status = eHAL_STATUS_SUCCESS;
    tSirSmeScanReq *pMsg;
    tANI_U16 msgLen;
    tANI_U8 bssid[WNI_CFG_BSSID_LEN] = {0, 0, 0, 0, 0, 0};
    tSirScanType scanType = pScanReq->scanType;
    tANI_U32 minChnTime;    //                        
    tANI_U32 maxChnTime;    //                        
    tANI_U32 i;
    tANI_U8 selfMacAddr[WNI_CFG_BSSID_LEN];
    tANI_U8 *pSelfMac = NULL;

    msgLen = (tANI_U16)(sizeof( tSirSmeScanReq ) - sizeof( pMsg->channelList.channelNumber ) + 
                        ( sizeof( pMsg->channelList.channelNumber ) * pScanReq->ChannelInfo.numOfChannels )) +
                   ( pScanReq->uIEFieldLen ) ;

    status = palAllocateMemory(pMac->hHdd, (void **)&pMsg, msgLen);
    if(HAL_STATUS_SUCCESS(status))
    {
        do
        {
            palZeroMemory(pMac->hHdd, pMsg, msgLen);
            pMsg->messageType = pal_cpu_to_be16((tANI_U16)eWNI_SME_SCAN_REQ);
            pMsg->length = pal_cpu_to_be16(msgLen);
            //                                                                   
            pMsg->sessionId = 0;
            pMsg->transactionId = 0;
            pMsg->dot11mode = (tANI_U8) csrTranslateToWNICfgDot11Mode(pMac, csrFindBestPhyMode( pMac, pMac->roam.configParam.phyMode ));
            pMsg->bssType = pal_cpu_to_be32(csrTranslateBsstypeToMacType(pScanReq->BSSType));

            if ( CSR_IS_SESSION_VALID( pMac, sessionId ) )
            {
              pSelfMac = (tANI_U8 *)&pMac->roam.roamSession[sessionId].selfMacAddr;
            }
            else
            {
              //                                                                                          
              //                              
              for( i = 0; i < CSR_ROAM_SESSION_MAX; i++ )
              {
                if( CSR_IS_SESSION_VALID( pMac, i ) )
                {
                  pSelfMac = (tANI_U8 *)&pMac->roam.roamSession[i].selfMacAddr;
                  break;
                }
              }
              if( CSR_ROAM_SESSION_MAX == i )
              {
                tANI_U32 len = WNI_CFG_BSSID_LEN;
                pSelfMac = selfMacAddr;
                status = ccmCfgGetStr( pMac, WNI_CFG_STA_ID, pSelfMac, &len );
                if( !HAL_STATUS_SUCCESS( status ) || 
                    ( len < WNI_CFG_BSSID_LEN ) )
                {
                  smsLog( pMac, LOGE, FL(" Can not get self MAC address from CFG status = %d"), status );
                  //                   
                  status = eHAL_STATUS_FAILURE;
                  break;
                }
              }
            }
            palCopyMemory( pMac->hHdd, (tANI_U8 *)pMsg->selfMacAddr, pSelfMac, sizeof(tSirMacAddr) );

            //              
            palCopyMemory( pMac->hHdd, (tANI_U8 *)pMsg->bssId, (tANI_U8 *)&pScanReq->bssid, sizeof(tSirMacAddr) );
            if( palEqualMemory( pMac->hHdd, pScanReq->bssid, bssid, sizeof(tCsrBssid) ) )
            {
                palFillMemory( pMac->hHdd, pMsg->bssId, sizeof(tSirMacAddr), 0xff );
            }
            else
            {
                palCopyMemory(pMac->hHdd, pMsg->bssId, pScanReq->bssid, WNI_CFG_BSSID_LEN); 
            }
            minChnTime = pScanReq->minChnTime;
            maxChnTime = pScanReq->maxChnTime;

            //                                                                                
            //                     
            /*                                                                    
                                                                            
                             */
            if( (eSIR_PASSIVE_SCAN != scanType) && (eCSR_SCAN_P2P_DISCOVERY != pScanReq->requestType)
                && (eCSR_BSS_TYPE_WDS_STA != pScanReq->BSSType)
                && (eANI_BOOLEAN_FALSE == pMac->scan.fEnableBypass11d))
            {
                scanType = pMac->scan.curScanType;
                if(eSIR_PASSIVE_SCAN == pMac->scan.curScanType)
                {
                    if(minChnTime < pMac->roam.configParam.nPassiveMinChnTime) 
                    {
                        minChnTime = pMac->roam.configParam.nPassiveMinChnTime;
                    }
                    if(maxChnTime < pMac->roam.configParam.nPassiveMaxChnTime)
                    {
                        maxChnTime = pMac->roam.configParam.nPassiveMaxChnTime;
                    }
                }
            }
            pMsg->scanType = pal_cpu_to_be32(scanType);

        pMsg->numSsid = (pScanReq->SSIDs.numOfSSIDs < SIR_SCAN_MAX_NUM_SSID) ? pScanReq->SSIDs.numOfSSIDs :
                                                        SIR_SCAN_MAX_NUM_SSID;
            if((pScanReq->SSIDs.numOfSSIDs != 0) && ( eSIR_PASSIVE_SCAN != scanType ))
            {
            for (i = 0; i < pMsg->numSsid; i++)
            {
                palCopyMemory(pMac->hHdd, &pMsg->ssId[i], &pScanReq->SSIDs.SSIDList[i].SSID, sizeof(tSirMacSSid));
            }
            }
            else
            {
                //                                                          
            for (i = 0; i < SIR_SCAN_MAX_NUM_SSID; i++)
            {
                pMsg->ssId[i].length = 0;
            }
            }

//                                                                              
//                                               
#if defined (ANI_CHIPSET_VIRGO) || defined (LIBRA_FPGA)|| defined (VOLANS_FPGA)
            pMsg->minChannelTime = pal_cpu_to_be32(minChnTime * 8);
            pMsg->maxChannelTime = pal_cpu_to_be32(maxChnTime * 8);
#elif defined (ANI_CHIPSET_TAURUS) || defined(ANI_CHIPSET_LIBRA) || defined(ANI_CHIPSET_VOLANS)
            pMsg->minChannelTime = pal_cpu_to_be32(minChnTime);
            pMsg->maxChannelTime = pal_cpu_to_be32(maxChnTime);
#else
#error unknown chipset
#endif
            //                  
            pMsg->hiddenSsid = pScanReqParam->hiddenSsid;
            //         
            //                                    
            pMsg->returnAfterFirstMatch = pScanReqParam->bReturnAfter1stMatch;
            //                                                     
            //                                                      
            //                                             
            pMsg->returnFreshResults = pScanReqParam->freshScan;
            //                            
            pMsg->returnUniqueResults = pScanReqParam->fUniqueResult;
            pMsg->channelList.numChannels = (tANI_U8)pScanReq->ChannelInfo.numOfChannels;
            if(pScanReq->ChannelInfo.numOfChannels)
            {
                //                                              
                status = palCopyMemory(pMac->hHdd, pMsg->channelList.channelNumber, pScanReq->ChannelInfo.ChannelList, 
                                        pScanReq->ChannelInfo.numOfChannels);
            }

            pMsg->uIEFieldLen = (tANI_U16) pScanReq->uIEFieldLen;
            pMsg->uIEFieldOffset = (tANI_U16)(sizeof( tSirSmeScanReq ) - sizeof( pMsg->channelList.channelNumber ) + 
                  ( sizeof( pMsg->channelList.channelNumber ) * pScanReq->ChannelInfo.numOfChannels )) ;
            if(pScanReq->uIEFieldLen != 0) 
            {
                palCopyMemory(pMac->hHdd, (tANI_U8 *)pMsg+pMsg->uIEFieldOffset,
                                    pScanReq->pIEField, pScanReq->uIEFieldLen );
            }
#ifdef WLAN_FEATURE_P2P
            pMsg->p2pSearch = pScanReq->p2pSearch;
            pMsg->skipDfsChnlInP2pSearch = pScanReq->skipDfsChnlInP2pSearch;
#endif

            if (pScanReq->requestType == eCSR_SCAN_HO_BG_SCAN) 
            {
                pMsg->backgroundScanMode = eSIR_ROAMING_SCAN;
            } 

        }while(0);
        if(HAL_STATUS_SUCCESS(status))
        {
            status = palSendMBMessage(pMac->hHdd, pMsg);
        }
        else {
            palFreeMemory(pMac->hHdd, pMsg);
        }
    }//                        


    return( status );
}

eHalStatus csrSendMBScanResultReq( tpAniSirGlobal pMac, tScanReqParam *pScanReqParam )
{
    eHalStatus status = eHAL_STATUS_SUCCESS;
    tSirSmeScanReq *pMsg;
    tANI_U16 msgLen;

    msgLen = (tANI_U16)(sizeof( tSirSmeScanReq ));
    status = palAllocateMemory(pMac->hHdd, (void **)&pMsg, msgLen);
    if(HAL_STATUS_SUCCESS(status))
    {
        palZeroMemory(pMac->hHdd, pMsg, msgLen);
        pMsg->messageType = pal_cpu_to_be16((tANI_U16)eWNI_SME_SCAN_REQ);
        pMsg->length = pal_cpu_to_be16(msgLen);
        pMsg->sessionId = 0;
        pMsg->returnFreshResults = pScanReqParam->freshScan;
        //                            
        pMsg->returnUniqueResults = pScanReqParam->fUniqueResult;
        pMsg->returnAfterFirstMatch = pScanReqParam->bReturnAfter1stMatch;
        status = palSendMBMessage(pMac->hHdd, pMsg);
    }                             

    return( status );
}



eHalStatus csrScanChannels( tpAniSirGlobal pMac, tSmeCmd *pCommand )
{
    eHalStatus status = eHAL_STATUS_FAILURE;
    tScanReqParam scanReq;
    
    do
    {    
        scanReq.freshScan = CSR_SME_SCAN_FLAGS_DELETE_CACHE | TRUE;
        scanReq.fUniqueResult = TRUE;
        scanReq.hiddenSsid = SIR_SCAN_NO_HIDDEN_SSID;
        if(eCsrScanForSsid == pCommand->u.scanCmd.reason)
        {
            scanReq.bReturnAfter1stMatch = CSR_SCAN_RETURN_AFTER_FIRST_MATCH;
        }
        else
        {
            //                                                              
            scanReq.bReturnAfter1stMatch = CSR_SCAN_RETURN_AFTER_ALL_CHANNELS;
        }
        if((eCsrScanBgScan == pCommand->u.scanCmd.reason)||
           (eCsrScanProbeBss == pCommand->u.scanCmd.reason))
        {
            scanReq.hiddenSsid = SIR_SCAN_HIDDEN_SSID_PE_DECISION;
        }

#ifdef FEATURE_WLAN_DIAG_SUPPORT_CSR
        {
            vos_log_scan_pkt_type *pScanLog = NULL;

            WLAN_VOS_DIAG_LOG_ALLOC(pScanLog, vos_log_scan_pkt_type, LOG_WLAN_SCAN_C);
            if(pScanLog)
            {
                if(eCsrScanBgScan == pCommand->u.scanCmd.reason || 
                    eCsrScanProbeBss == pCommand->u.scanCmd.reason)
                {
                    pScanLog->eventId = WLAN_SCAN_EVENT_HO_SCAN_REQ;
                }
                else
                {
                    if( (eSIR_PASSIVE_SCAN != pCommand->u.scanCmd.u.scanRequest.scanType) && 
                        (eSIR_PASSIVE_SCAN != pMac->scan.curScanType) )
                    {
                        pScanLog->eventId = WLAN_SCAN_EVENT_ACTIVE_SCAN_REQ;
                    }
                    else
                    {
                        pScanLog->eventId = WLAN_SCAN_EVENT_PASSIVE_SCAN_REQ;
                    }
                }
                pScanLog->minChnTime = (v_U8_t)pCommand->u.scanCmd.u.scanRequest.minChnTime;
                pScanLog->maxChnTime = (v_U8_t)pCommand->u.scanCmd.u.scanRequest.maxChnTime;
                pScanLog->numChannel = (v_U8_t)pCommand->u.scanCmd.u.scanRequest.ChannelInfo.numOfChannels;
                if(pScanLog->numChannel && (pScanLog->numChannel < VOS_LOG_MAX_NUM_CHANNEL))
                {
                    palCopyMemory(pMac->hHdd, pScanLog->channels,
                                  pCommand->u.scanCmd.u.scanRequest.ChannelInfo.ChannelList,
                                  pScanLog->numChannel);
                }
                WLAN_VOS_DIAG_LOG_REPORT(pScanLog);
            }
        }
#endif //                                    


        status = csrSendMBScanReq(pMac, pCommand->sessionId,
                                &pCommand->u.scanCmd.u.scanRequest, &scanReq);
    }while(0);
    
    return( status );
}


eHalStatus csrScanRetrieveResult(tpAniSirGlobal pMac)
{
    eHalStatus status = eHAL_STATUS_FAILURE;
    tScanReqParam scanReq;
    
    do
    {    
        //                
        scanReq.freshScan = CSR_SME_SCAN_FLAGS_DELETE_CACHE;
        scanReq.fUniqueResult = TRUE;
        scanReq.bReturnAfter1stMatch = CSR_SCAN_RETURN_AFTER_ALL_CHANNELS;
        status = csrSendMBScanResultReq(pMac, &scanReq);
    }while(0);
    
    return (status);
}



eHalStatus csrProcessScanCommand( tpAniSirGlobal pMac, tSmeCmd *pCommand )
{
    eHalStatus status = eHAL_STATUS_SUCCESS;
    tCsrChannelInfo newChannelInfo = {0, NULL};
    int i, j;
    tANI_U8 *pChannel = NULL;
    tANI_U32 len = 0;

    //                                
    for( i = 0; i < CSR_ROAM_SESSION_MAX; i++ )
    {
      pCommand->u.scanCmd.lastRoamState[i] = csrRoamStateChange( pMac, eCSR_ROAMING_STATE_SCANNING, i);
      smsLog( pMac, LOG3, "starting SCAN command from %d state.... reason is %d\n", pCommand->u.scanCmd.lastRoamState[i], pCommand->u.scanCmd.reason );
    }

    switch(pCommand->u.scanCmd.reason)
    {
    case eCsrScanGetResult:
    case eCsrScanForCapsChange:     //                                                
        status = csrScanRetrieveResult(pMac);
        break;
    case eCsrScanSetBGScanParam:
        status = csrProcessSetBGScanParam(pMac, pCommand);
        break;
    case eCsrScanBGScanAbort:
        status = csrSetCfgBackgroundScanPeriod(pMac, 0);
        break;
    case eCsrScanBGScanEnable:
        status = csrSetCfgBackgroundScanPeriod(pMac, pMac->roam.configParam.bgScanInterval);
        break;
    case eCsrScanGetScanChnInfo:
        status = csrScanGetScanChannelInfo(pMac);
        break;
    case eCsrScanUserRequest:
        if(pMac->roam.configParam.fScanTwice)
        {
            //                         
            if(pCommand->u.scanCmd.u.scanRequest.ChannelInfo.numOfChannels &&
               (NULL != pCommand->u.scanCmd.u.scanRequest.ChannelInfo.ChannelList))
            {
                len = pCommand->u.scanCmd.u.scanRequest.ChannelInfo.numOfChannels;
                //                          
                newChannelInfo.ChannelList = (tANI_U8 *)vos_mem_malloc(newChannelInfo.numOfChannels * 2);
                pChannel = pCommand->u.scanCmd.u.scanRequest.ChannelInfo.ChannelList;
            }
            else
            {
                //                                       
                len = sizeof(pMac->roam.validChannelList);

                if (HAL_STATUS_SUCCESS(csrGetCfgValidChannels(pMac, (tANI_U8 *)pMac->roam.validChannelList, &len)))
                {
                    //                          
                    newChannelInfo.ChannelList = (tANI_U8 *)vos_mem_malloc(len * 2);
                    pChannel = pMac->roam.validChannelList;
                }
            }
            if(NULL == newChannelInfo.ChannelList)
            {
                newChannelInfo.numOfChannels = 0;
            }
            else
            {
                j = 0;
                for(i = 0; i < len; i++)
                {
                    newChannelInfo.ChannelList[j++] = pChannel[i];
                    if(CSR_MAX_24GHz_CHANNEL_NUMBER >= pChannel[i])
                    {
                        newChannelInfo.ChannelList[j++] = pChannel[i];
                    }
                }
                if(NULL != pCommand->u.scanCmd.u.scanRequest.ChannelInfo.ChannelList)
                {
                    //                                                             
                    vos_mem_free(pCommand->u.scanCmd.u.scanRequest.ChannelInfo.ChannelList);
                }
                pCommand->u.scanCmd.u.scanRequest.ChannelInfo.numOfChannels = j;
                pCommand->u.scanCmd.u.scanRequest.ChannelInfo.ChannelList = newChannelInfo.ChannelList;
            }
        } //                                     

        status = csrScanChannels(pMac, pCommand);

        break;
    default:
        status = csrScanChannels(pMac, pCommand);
        break;
    }    
    
    if(!HAL_STATUS_SUCCESS(status))
    {
        csrReleaseScanCommand(pMac, pCommand, eCSR_SCAN_FAILURE);
    }
    
    return (status);
}


eHalStatus csrScanSetBGScanparams(tpAniSirGlobal pMac, tCsrBGScanRequest *pScanReq)
{
    eHalStatus status = eHAL_STATUS_SUCCESS;
    tSmeCmd *pCommand = NULL;
    
    if(pScanReq)
    {
        do
        {
            pCommand = csrGetCommandBuffer(pMac);
            if(!pCommand)
            {
                status = eHAL_STATUS_RESOURCES;
                break;
            }
            palZeroMemory(pMac->hHdd, &pCommand->u.scanCmd, sizeof(tScanCmd));
            pCommand->command = eSmeCommandScan;
            pCommand->u.scanCmd.reason = eCsrScanSetBGScanParam;
            pCommand->u.scanCmd.callback = NULL;
            pCommand->u.scanCmd.pContext = NULL;
            palCopyMemory(pMac->hHdd, &pCommand->u.scanCmd.u.bgScanRequest, pScanReq, sizeof(tCsrBGScanRequest));
            //                        
            if(pScanReq->ChannelInfo.numOfChannels == 0)
            {
                pCommand->u.scanCmd.u.bgScanRequest.ChannelInfo.ChannelList = NULL;
            }
            else
            {
                status = palAllocateMemory(pMac->hHdd, (void **)&pCommand->u.scanCmd.u.bgScanRequest.ChannelInfo.ChannelList,
                                             pScanReq->ChannelInfo.numOfChannels);
                if(HAL_STATUS_SUCCESS(status))
                {
                    palCopyMemory(pMac->hHdd, pCommand->u.scanCmd.u.bgScanRequest.ChannelInfo.ChannelList,
                                    pScanReq->ChannelInfo.ChannelList, pScanReq->ChannelInfo.numOfChannels); 
                }
                else
                {
                    smsLog(pMac, LOGE, FL("ran out of memory\n"));
                    csrReleaseCommandScan(pMac, pCommand);
                    break;
                }
            }

            //                 
            if(pScanReq->SSID.length)
            {
               palCopyMemory(pMac->hHdd, 
                             pCommand->u.scanCmd.u.bgScanRequest.SSID.ssId,
                             pScanReq->SSID.ssId, 
                             pScanReq->SSID.length);
               pCommand->u.scanCmd.u.bgScanRequest.SSID.length = pScanReq->SSID.length;

            }
            pCommand->u.scanCmd.u.bgScanRequest.maxChnTime= pScanReq->maxChnTime;
            pCommand->u.scanCmd.u.bgScanRequest.minChnTime = pScanReq->minChnTime;
            pCommand->u.scanCmd.u.bgScanRequest.scanInterval = pScanReq->scanInterval;


            status = csrQueueSmeCommand(pMac, pCommand, eANI_BOOLEAN_FALSE);
            if( !HAL_STATUS_SUCCESS( status ) )
            {
                smsLog( pMac, LOGE, FL(" fail to send message status = %d\n"), status );
                csrReleaseCommandScan( pMac, pCommand );
                break;
            }
        }while(0);
    }
    
    return (status);
}

eHalStatus csrScanBGScanAbort( tpAniSirGlobal pMac )
{
    eHalStatus status = eHAL_STATUS_SUCCESS;
    tSmeCmd *pCommand = NULL;
    
    do
    {
        pCommand = csrGetCommandBuffer(pMac);
        if(!pCommand)
        {
            status = eHAL_STATUS_RESOURCES;
            break;
        }
        palZeroMemory(pMac->hHdd, &pCommand->u.scanCmd, sizeof(tScanCmd));
        pCommand->command = eSmeCommandScan; 
        pCommand->u.scanCmd.reason = eCsrScanBGScanAbort;
        pCommand->u.scanCmd.callback = NULL;
        pCommand->u.scanCmd.pContext = NULL;
        status = csrQueueSmeCommand(pMac, pCommand, eANI_BOOLEAN_FALSE);
        if( !HAL_STATUS_SUCCESS( status ) )
        {
            smsLog( pMac, LOGE, FL(" fail to send message status = %d\n"), status );
            csrReleaseCommandScan( pMac, pCommand );
            break;
        }
    }while(0);
    
    return (status);
}


//                                                                
eHalStatus csrScanBGScanEnable(tpAniSirGlobal pMac)
{
    eHalStatus status = eHAL_STATUS_SUCCESS;
    tSmeCmd *pCommand = NULL;
    
    if(pMac->roam.configParam.bgScanInterval)
    {
        do
        {
            pCommand = csrGetCommandBuffer(pMac);
            if(!pCommand)
            {
                status = eHAL_STATUS_RESOURCES;
                break;
            }
            palZeroMemory(pMac->hHdd, &pCommand->u.scanCmd, sizeof(tScanCmd));
            pCommand->command = eSmeCommandScan; 
            pCommand->u.scanCmd.reason = eCsrScanBGScanEnable;
            pCommand->u.scanCmd.callback = NULL;
            pCommand->u.scanCmd.pContext = NULL;
            status = csrQueueSmeCommand(pMac, pCommand, eANI_BOOLEAN_FALSE);
            if( !HAL_STATUS_SUCCESS( status ) )
            {
                smsLog( pMac, LOGE, FL(" fail to send message status = %d\n"), status );
                csrReleaseCommandScan( pMac, pCommand );
                break;
            }
        }while(0);
        //                                                                              
        //                                        
        //                                 
        csrScanStartResultAgingTimer(pMac);
    }
    else
    {
        //                                             
        csrScanStopResultAgingTimer(pMac);
        smsLog(pMac, LOGE, FL("cannot continue because the bgscan interval is 0\n"));
        status = eHAL_STATUS_INVALID_PARAMETER;
    }
    
    return (status);
}


eHalStatus csrScanCopyRequest(tpAniSirGlobal pMac, tCsrScanRequest *pDstReq, tCsrScanRequest *pSrcReq)
{
    eHalStatus status = eHAL_STATUS_SUCCESS;
    tANI_U32 len = sizeof(pMac->roam.validChannelList);
    tANI_U32 index = 0;
    tANI_U32 new_index = 0;

    do
    {
        status = csrScanFreeRequest(pMac, pDstReq);
        if(HAL_STATUS_SUCCESS(status))
        {
            status = palCopyMemory(pMac->hHdd, pDstReq, pSrcReq, sizeof(tCsrScanRequest));
            if(pSrcReq->uIEFieldLen == 0)
            {
                pDstReq->pIEField = NULL;
            }
            else
            {
                status = palAllocateMemory(pMac->hHdd, (void **)&pDstReq->pIEField, pSrcReq->uIEFieldLen);
                if(HAL_STATUS_SUCCESS(status))
                {
                    palCopyMemory(pMac->hHdd, pDstReq->pIEField, pSrcReq->pIEField, pSrcReq->uIEFieldLen);
                    pDstReq->uIEFieldLen = pSrcReq->uIEFieldLen;
                }
                else
                {
                    smsLog(pMac, LOGE, "No memory for scanning IE fields\n");
                    break;
                }
            }//                            
            {
                if(pSrcReq->ChannelInfo.numOfChannels == 0)
                {
                    pDstReq->ChannelInfo.ChannelList = NULL;
                        pDstReq->ChannelInfo.numOfChannels = 0;
                }
                else
                {
                    status = palAllocateMemory(pMac->hHdd, (void **)&pDstReq->ChannelInfo.ChannelList, 
                                        pSrcReq->ChannelInfo.numOfChannels * sizeof(*pDstReq->ChannelInfo.ChannelList));
                    if(!HAL_STATUS_SUCCESS(status))
                    {
                        pDstReq->ChannelInfo.numOfChannels = 0;
                        smsLog(pMac, LOGE, "No memory for scanning Channel List\n");
                        break;
                    }

                    if((pSrcReq->scanType == eSIR_PASSIVE_SCAN) && (pSrcReq->requestType == eCSR_SCAN_REQUEST_11D_SCAN))
                    {
                       for ( index = 0; index < pSrcReq->ChannelInfo.numOfChannels ; index++ )
                       {
                          pDstReq->ChannelInfo.ChannelList[new_index] =
                                             pSrcReq->ChannelInfo.ChannelList[index];
                          new_index++;
                    }
                       pDstReq->ChannelInfo.numOfChannels = new_index;
                    }
                    else if(HAL_STATUS_SUCCESS(csrGetCfgValidChannels(pMac, pMac->roam.validChannelList, &len)))
                    {
                        new_index = 0;
                        pMac->roam.numValidChannels = len;
                        for ( index = 0; index < pSrcReq->ChannelInfo.numOfChannels ; index++ )
                        {
                            /*                                   
                                                                                       
                                                                                       
                                                                      
                                                                                
                             */
                            if((csrRoamIsValidChannel(pMac, pSrcReq->ChannelInfo.ChannelList[index])) || 
                               ((eCSR_SCAN_P2P_DISCOVERY == pSrcReq->requestType) && 
                                CSR_IS_SOCIAL_CHANNEL(pSrcReq->ChannelInfo.ChannelList[index])))
                            {
                                pDstReq->ChannelInfo.ChannelList[new_index] =
                                    pSrcReq->ChannelInfo.ChannelList[index];
                                new_index++;
                            }
                        }
                        pDstReq->ChannelInfo.numOfChannels = new_index;
                    }
                    else
                    {
                        smsLog(pMac, LOGE, "Couldn't get the valid Channel List, keeping requester's list\n");
                        palCopyMemory(pMac->hHdd, pDstReq->ChannelInfo.ChannelList, pSrcReq->ChannelInfo.ChannelList, 
                                        pSrcReq->ChannelInfo.numOfChannels * sizeof(*pDstReq->ChannelInfo.ChannelList));
                        pDstReq->ChannelInfo.numOfChannels = pSrcReq->ChannelInfo.numOfChannels;
                    }
                }//                                
            }
            if(pSrcReq->SSIDs.numOfSSIDs == 0)
            {
                pDstReq->SSIDs.numOfSSIDs = 0;
                pDstReq->SSIDs.SSIDList = NULL;
            }
            else
            {
                status = palAllocateMemory(pMac->hHdd, (void **)&pDstReq->SSIDs.SSIDList, 
                                    pSrcReq->SSIDs.numOfSSIDs * sizeof(*pDstReq->SSIDs.SSIDList));
                if(HAL_STATUS_SUCCESS(status))
                {
                    pDstReq->SSIDs.numOfSSIDs = pSrcReq->SSIDs.numOfSSIDs;
                    palCopyMemory(pMac->hHdd, pDstReq->SSIDs.SSIDList, pSrcReq->SSIDs.SSIDList, 
                                    pSrcReq->SSIDs.numOfSSIDs * sizeof(*pDstReq->SSIDs.SSIDList));
                }
                else
                {
                    pDstReq->SSIDs.numOfSSIDs = 0;
                    smsLog(pMac, LOGE, "No memory for scanning SSID List\n");
                    break;
                }
            }//                             
#ifdef WLAN_FEATURE_P2P
            pDstReq->p2pSearch = pSrcReq->p2pSearch;
            pDstReq->skipDfsChnlInP2pSearch = pSrcReq->skipDfsChnlInP2pSearch;
#endif

        }
    }while(0);
    
    if(!HAL_STATUS_SUCCESS(status))
    {
        csrScanFreeRequest(pMac, pDstReq);
    }
    
    return (status);
}


eHalStatus csrScanFreeRequest(tpAniSirGlobal pMac, tCsrScanRequest *pReq)
{
    eHalStatus status = eHAL_STATUS_SUCCESS;
    
    if(pReq->ChannelInfo.ChannelList)
    {
        status = palFreeMemory(pMac->hHdd, pReq->ChannelInfo.ChannelList);
        pReq->ChannelInfo.ChannelList = NULL;
    }
    pReq->ChannelInfo.numOfChannels = 0;
    if(pReq->pIEField)
    {
        status = palFreeMemory(pMac->hHdd, pReq->pIEField);
        pReq->pIEField = NULL;
    }
    pReq->uIEFieldLen = 0;
    if(pReq->SSIDs.SSIDList)
    {
        palFreeMemory(pMac->hHdd, pReq->SSIDs.SSIDList);
        pReq->SSIDs.SSIDList = NULL;
    }
    pReq->SSIDs.numOfSSIDs = 0;
    
    return (status);
}


void csrScanCallCallback(tpAniSirGlobal pMac, tSmeCmd *pCommand, eCsrScanStatus scanStatus)
{
    if(pCommand->u.scanCmd.callback)
    {
//                                            
        pCommand->u.scanCmd.callback(pMac, pCommand->u.scanCmd.pContext, pCommand->u.scanCmd.scanID, scanStatus); 
//                                            
    } else {
        smsLog( pMac, LOG2, "%s:%d - Callback NULL!!!\n", __FUNCTION__, __LINE__);
    }
}


void csrScanStopTimers(tpAniSirGlobal pMac)
{
    csrScanStopResultAgingTimer(pMac);
    csrScanStopIdleScanTimer(pMac);
    csrScanStopGetResultTimer(pMac);
}


eHalStatus csrScanStartGetResultTimer(tpAniSirGlobal pMac)
{
    eHalStatus status;
    
    if(pMac->scan.fScanEnable)
    {
        status = palTimerStart(pMac->hHdd, pMac->scan.hTimerGetResult, CSR_SCAN_GET_RESULT_INTERVAL, eANI_BOOLEAN_TRUE);
    }
    else
    {
        status = eHAL_STATUS_FAILURE;
    }
    
    return (status);
}


eHalStatus csrScanStopGetResultTimer(tpAniSirGlobal pMac)
{
    return (palTimerStop(pMac->hHdd, pMac->scan.hTimerGetResult));
}


void csrScanGetResultTimerHandler(void *pv)
{
    tpAniSirGlobal pMac = PMAC_STRUCT( pv );
    
    csrScanRequestResult(pMac);
}

#ifdef WLAN_AP_STA_CONCURRENCY
static void csrStaApConcTimerHandler(void *pv)
{
    tpAniSirGlobal pMac = PMAC_STRUCT( pv );
    tListElem *pEntry;
    tSmeCmd *pScanCmd;

    csrLLLock(&pMac->scan.scanCmdPendingList);

    if ( NULL != ( pEntry = csrLLPeekHead( &pMac->scan.scanCmdPendingList, LL_ACCESS_NOLOCK) ) )
    {    
        tCsrScanRequest scanReq;
        tSmeCmd *pSendScanCmd = NULL;
        tANI_U8 numChn = 0;
        tANI_U8 i;
        tCsrChannelInfo *pChnInfo = &scanReq.ChannelInfo;
        tANI_U8    channelToScan[WNI_CFG_VALID_CHANNEL_LIST_LEN];
        eHalStatus status;
       

        pScanCmd = GET_BASE_ADDR( pEntry, tSmeCmd, Link );
        numChn = pScanCmd->u.scanCmd.u.scanRequest.ChannelInfo.numOfChannels;
        if (numChn > 1)
        {
             palZeroMemory(pMac->hHdd, &scanReq, sizeof(tCsrScanRequest));

             pSendScanCmd = csrGetCommandBuffer(pMac); //                                          
             if (!pSendScanCmd)
             {
                 smsLog( pMac, LOGE, FL(" Failed to get Queue command buffer\n") );
                 csrLLUnlock(&pMac->scan.scanCmdPendingList);
                 return;
             }
             pSendScanCmd->command = pScanCmd->command; 
             pSendScanCmd->sessionId = pScanCmd->sessionId;
             pSendScanCmd->u.scanCmd.callback = NULL;
             pSendScanCmd->u.scanCmd.pContext = pScanCmd->u.scanCmd.pContext;
             pSendScanCmd->u.scanCmd.reason = pScanCmd->u.scanCmd.reason;
             pSendScanCmd->u.scanCmd.scanID = pMac->scan.nextScanID++; //                  

             pChnInfo->numOfChannels = 1;
             palCopyMemory(pMac->hHdd, &channelToScan[0], &pScanCmd->u.scanCmd.u.scanRequest.ChannelInfo.ChannelList[0], 
                          1 * sizeof(tANI_U8)); //                     
             pChnInfo->ChannelList = &channelToScan[0];

             for (i = 0; i < (numChn-2); i++)
             {
                 pScanCmd->u.scanCmd.u.scanRequest.ChannelInfo.ChannelList[i] = 
                 pScanCmd->u.scanCmd.u.scanRequest.ChannelInfo.ChannelList[i+1]; //                              
             }
          
             pScanCmd->u.scanCmd.u.scanRequest.ChannelInfo.numOfChannels = numChn -1; //                                              

             scanReq.BSSType = eCSR_BSS_TYPE_ANY;
             //                                                
             scanReq.scanType = eSIR_ACTIVE_SCAN;
             scanReq.maxChnTime = CSR_MIN(pScanCmd->u.scanCmd.u.scanRequest.maxChnTime,CSR_ACTIVE_MAX_CHANNEL_TIME_CONC);
             scanReq.minChnTime =  CSR_MIN(pScanCmd->u.scanCmd.u.scanRequest.minChnTime,CSR_ACTIVE_MIN_CHANNEL_TIME_CONC);

             status = csrScanCopyRequest(pMac, &pSendScanCmd->u.scanCmd.u.scanRequest, &scanReq);
             if(!HAL_STATUS_SUCCESS(status))
             {
                 smsLog( pMac, LOGE, FL(" Failed to get copy csrScanRequest = %d\n"), status );
                 csrLLUnlock(&pMac->scan.scanCmdPendingList);
                 return;
             }       
        }
        else
        {    //                                                 
             //                              
             pSendScanCmd = pScanCmd;
             //                                      
             if (csrLLRemoveHead( &pMac->scan.scanCmdPendingList, LL_ACCESS_NOLOCK) == NULL)
             { //                                                                           
                 smsLog( pMac, LOGE, FL(" Failed to remove entry from scanCmdPendingList\n"));
             }
            
        }               
        csrQueueSmeCommand(pMac, pSendScanCmd, eANI_BOOLEAN_FALSE);

    }

    if (!csrLLIsListEmpty( &pMac->scan.scanCmdPendingList, LL_ACCESS_NOLOCK ))
    {
         palTimerStart(pMac->hHdd, pMac->scan.hTimerStaApConcTimer, 
                 CSR_SCAN_STAAP_CONC_INTERVAL, eANI_BOOLEAN_FALSE);
    }
    csrLLUnlock(&pMac->scan.scanCmdPendingList);
    
}
#endif

eHalStatus csrScanStartResultAgingTimer(tpAniSirGlobal pMac)
{
    eHalStatus status;
    
    if(pMac->scan.fScanEnable)
    {
        status = palTimerStart(pMac->hHdd, pMac->scan.hTimerResultAging, CSR_SCAN_RESULT_AGING_INTERVAL, eANI_BOOLEAN_TRUE);
    }
    else
    {
        status = eHAL_STATUS_FAILURE;
    }
    
    return (status);
}


eHalStatus csrScanStopResultAgingTimer(tpAniSirGlobal pMac)
{
    return (palTimerStop(pMac->hHdd, pMac->scan.hTimerResultAging));
}


//                                                                           
//                                                            
//                    
//                      
//                            
//                              
//                                            
//                                              
//                               
tANI_U32 csrScanGetAgeOutTime(tpAniSirGlobal pMac)
{
    tANI_U32 nRet;

    if(pMac->scan.nAgingCountDown)
    {
        //                                                   
        nRet = pMac->scan.nLastAgeTimeOut * pMac->scan.nAgingCountDown;
        pMac->scan.nAgingCountDown--;
    }
    else
    {
        if( csrIsAllSessionDisconnected( pMac ) )
        {
            if(pmcIsPowerSaveEnabled(pMac, ePMC_IDLE_MODE_POWER_SAVE))
            {
                nRet = pMac->roam.configParam.scanAgeTimeNCPS;
            }
            else
            {
                nRet = pMac->roam.configParam.scanAgeTimeNCNPS;
            }
        }
        else
        {
            if(pmcIsPowerSaveEnabled(pMac, ePMC_BEACON_MODE_POWER_SAVE))
            {
                nRet = pMac->roam.configParam.scanAgeTimeCPS;
            }
            else
            {
                nRet = pMac->roam.configParam.scanAgeTimeCNPS;
            }
        }
        //                                                                                    
        //                                                                                        
        if(pMac->scan.nLastAgeTimeOut > nRet)
        {
            if(nRet)
            {
                pMac->scan.nAgingCountDown = (pMac->scan.nLastAgeTimeOut / nRet);
            }
            pMac->scan.nLastAgeTimeOut = nRet;
            nRet *= pMac->scan.nAgingCountDown;
        }
        else
        {
            pMac->scan.nLastAgeTimeOut = nRet;
        }
    }

    return (nRet);
}


void csrScanResultAgingTimerHandler(void *pv)
{
    tpAniSirGlobal pMac = PMAC_STRUCT( pv );
    tANI_BOOLEAN fDisconnected = csrIsAllSessionDisconnected(pMac);
    
    //                 
    if(pMac->scan.fScanEnable && 
        (((eANI_BOOLEAN_FALSE == fDisconnected) && pMac->roam.configParam.bgScanInterval)    
        || (fDisconnected && (pMac->scan.fCancelIdleScan == eANI_BOOLEAN_FALSE)))
        )
    {
        tListElem *pEntry, *tmpEntry;
        tCsrScanResult *pResult;
        tANI_TIMESTAMP ageOutTime = (tANI_TIMESTAMP)(csrScanGetAgeOutTime(pMac) * PAL_TICKS_PER_SECOND); //                       
        tANI_TIMESTAMP curTime = (tANI_TIMESTAMP)palGetTickCount(pMac->hHdd);

        csrLLLock(&pMac->scan.scanResultList);
        pEntry = csrLLPeekHead( &pMac->scan.scanResultList, LL_ACCESS_NOLOCK );
        while( pEntry ) 
        {
            tmpEntry = csrLLNext(&pMac->scan.scanResultList, pEntry, LL_ACCESS_NOLOCK);
            pResult = GET_BASE_ADDR( pEntry, tCsrScanResult, Link );
            if((curTime - pResult->Result.BssDescriptor.nReceivedTime) > ageOutTime)
            {
                smsLog(pMac, LOGW, " age out due to time out");
                csrScanAgeOutBss(pMac, pResult);
            }
            pEntry = tmpEntry;
        }
        csrLLUnlock(&pMac->scan.scanResultList);
    }
}


eHalStatus csrScanStartIdleScanTimer(tpAniSirGlobal pMac, tANI_U32 interval)
{
    eHalStatus status;
    
    smsLog(pMac, LOG1, " csrScanStartIdleScanTimer \n ");
    if((pMac->scan.fScanEnable) && (eANI_BOOLEAN_FALSE == pMac->scan.fCancelIdleScan) && interval)
    {
        pMac->scan.nIdleScanTimeGap += interval;
        palTimerStop(pMac->hHdd, pMac->scan.hTimerIdleScan);
        status = palTimerStart(pMac->hHdd, pMac->scan.hTimerIdleScan, interval, eANI_BOOLEAN_FALSE);
        if( !HAL_STATUS_SUCCESS(status) )
        {
            smsLog(pMac, LOGE, "  Fail to start Idle scan timer. status = %d interval = %d\n", status, interval);
            //                                                             
            pMac->scan.fRestartIdleScan = eANI_BOOLEAN_TRUE;
        }
    }
    else
    {
        if( pMac->scan.fScanEnable && (eANI_BOOLEAN_FALSE == pMac->scan.fCancelIdleScan) )
        {
            pMac->scan.fRestartIdleScan = eANI_BOOLEAN_TRUE;
        }
        status = eHAL_STATUS_FAILURE;
    }
    
    return (status);
}


eHalStatus csrScanStopIdleScanTimer(tpAniSirGlobal pMac)
{
    return (palTimerStop(pMac->hHdd, pMac->scan.hTimerIdleScan));
}


//                                                                          
void csrScanSuspendIMPS( tpAniSirGlobal pMac )
{
    csrScanCancelIdleScan(pMac);
}


//                                                                                          
//                            
void csrScanResumeIMPS( tpAniSirGlobal pMac )
{
    csrScanStartIdleScan( pMac );
}


void csrScanIMPSCallback(void *callbackContext, eHalStatus status)
{
    tpAniSirGlobal pMac = PMAC_STRUCT( callbackContext );

    if(eANI_BOOLEAN_FALSE == pMac->scan.fCancelIdleScan)
    {
        if(pMac->roam.configParam.IsIdleScanEnabled) 
        {
            if(HAL_STATUS_SUCCESS(status))
            {
                if(csrIsAllSessionDisconnected(pMac) && !csrIsRoamCommandWaiting(pMac))
                {
                    smsLog(pMac, LOGW, FL("starts idle mode full scan\n"));
                    csrScanAllChannels(pMac, eCSR_SCAN_IDLE_MODE_SCAN);
                }
                else
                {
                    smsLog(pMac, LOGW, FL("cannot start idle mode full scan\n"));
                    //                                                                               
                    //                       
                    csrScanStopIdleScanTimer(pMac);
                }
            }
            else
            {
                smsLog(pMac, LOGE, FL("sees not success status (%d)\n"), status);
            }
        }
        else
        {//                                                                       
       
            tANI_U32 nTime = 0;

            pMac->scan.fRestartIdleScan = eANI_BOOLEAN_FALSE;
            if(!HAL_STATUS_SUCCESS(csrScanTriggerIdleScan(pMac, &nTime)))
            {
                csrScanStartIdleScanTimer(pMac, nTime);
            }
        }
    }
}


//                                                                                                           
//                        
//                                                                                                
eHalStatus csrScanTriggerIdleScan(tpAniSirGlobal pMac, tANI_U32 *pTimeInterval)
{
    eHalStatus status = eHAL_STATUS_CSR_WRONG_STATE;

    //                                          
    if (vos_concurrent_sessions_running() && csrIsAnySessionInConnectState(pMac))
    {
        smsLog( pMac, LOG1, FL("Cannot request IMPS because Concurrent Sessions Running\n") );
        return (status);
    }

    if(pTimeInterval)
    {
        *pTimeInterval = 0;
    }

    smsLog(pMac, LOG3, FL("called\n"));
    if( smeCommandPending( pMac ) )
    {
        smsLog( pMac, LOG1, FL("  Cannot request IMPS because command pending\n") );
        //                                         
        if(pTimeInterval)
        {
            *pTimeInterval = 0;
        }
        //                  
        pMac->scan.fRestartIdleScan = eANI_BOOLEAN_TRUE;

        return (status);
    }

    if((pMac->scan.fScanEnable) && (eANI_BOOLEAN_FALSE == pMac->scan.fCancelIdleScan) 
    /*                                       */)
    {
        //                                                                  
        csrScanStopGetResultTimer(pMac);
        if(pTimeInterval)
        {
            *pTimeInterval = pMac->roam.configParam.impsSleepTime;
        }
        //                                                 
        status = pmcRequestImps(pMac, pMac->roam.configParam.impsSleepTime / PAL_TIMER_TO_MS_UNIT, csrScanIMPSCallback, pMac);
        if(!HAL_STATUS_SUCCESS(status))
        {
            if(eHAL_STATUS_PMC_ALREADY_IN_IMPS != status)
            {
                //                                                    
                if( !csrCheckPSReady( pMac ) )
                {
                    if(pTimeInterval)
                    {
                    *pTimeInterval = 0;
                }
                    //                                                    
                    //                                                          
                    pMac->scan.fRestartIdleScan = eANI_BOOLEAN_TRUE;
                }
                else
                {
                    //                                  
                    if(pTimeInterval)
                    {
                    *pTimeInterval = CSR_IDLE_SCAN_WAIT_TIME;
                }
            }
                smsLog(pMac, LOGW, FL("call pmcRequestImps and it returns status code (%d)\n"), status);
            }
            else
            {
                smsLog(pMac, LOGW, FL("already in IMPS\n"));
                //                                                                                       
                //                                                                    
                status = eHAL_STATUS_SUCCESS;
                pMac->scan.nIdleScanTimeGap = 0;
            }
        }
        else
        {
            //                                  
            pMac->scan.nIdleScanTimeGap = 0;
        }
    }

    return (status);
}


eHalStatus csrScanStartIdleScan(tpAniSirGlobal pMac)
{
    eHalStatus status = eHAL_STATUS_CSR_WRONG_STATE;
    tANI_U32 nTime = 0;

    smsLog(pMac, LOGW, FL("called\n"));
    if(pMac->roam.configParam.IsIdleScanEnabled)
    {
        //                  
        csrScanBGScanAbort(pMac);
        //                                                                  
        csrScanStopGetResultTimer(pMac);
        //                                              
        csrScanStartResultAgingTimer(pMac);
    }
    pMac->scan.fCancelIdleScan = eANI_BOOLEAN_FALSE;
    status = csrScanTriggerIdleScan(pMac, &nTime);
    if(!HAL_STATUS_SUCCESS(status))
    {
        csrScanStartIdleScanTimer(pMac, nTime);
    }

    return (status);
}


void csrScanCancelIdleScan(tpAniSirGlobal pMac)
{
    if(eANI_BOOLEAN_FALSE == pMac->scan.fCancelIdleScan)
    {
#ifdef WLAN_SOFTAP_FEATURE
        if (vos_concurrent_sessions_running()) {
            return;
        }
#endif
        smsLog(pMac, LOG1, "  csrScanCancelIdleScan\n");
        pMac->scan.fCancelIdleScan = eANI_BOOLEAN_TRUE;
        //                                                       
        pMac->scan.fRestartIdleScan = eANI_BOOLEAN_TRUE;
        csrScanStopIdleScanTimer(pMac);
        csrScanRemoveNotRoamingScanCommand(pMac);
    }
}


void csrScanIdleScanTimerHandler(void *pv)
{
    tpAniSirGlobal pMac = PMAC_STRUCT( pv );
    eHalStatus status;
    tANI_U32 nTime = 0;

    smsLog(pMac, LOGW, "  csrScanIdleScanTimerHandler called  ");
    status = csrScanTriggerIdleScan(pMac, &nTime);
    if(!HAL_STATUS_SUCCESS(status) && (eANI_BOOLEAN_FALSE == pMac->scan.fCancelIdleScan))
    {
        //                                                    
        if(pMac->scan.nIdleScanTimeGap >= pMac->roam.configParam.impsSleepTime)
        {
            pMac->scan.nIdleScanTimeGap = 0;
            csrScanIMPSCallback(pMac, eHAL_STATUS_SUCCESS);
        }
        else
        {
            csrScanStartIdleScanTimer(pMac, nTime);
        }
    }
}




tANI_BOOLEAN csrScanRemoveNotRoamingScanCommand(tpAniSirGlobal pMac)
{
    tANI_BOOLEAN fRet = eANI_BOOLEAN_FALSE;
    tListElem *pEntry, *pEntryTmp;
    tSmeCmd *pCommand;
    tDblLinkList localList;

    vos_mem_zero(&localList, sizeof(tDblLinkList));
    if(!HAL_STATUS_SUCCESS(csrLLOpen(pMac->hHdd, &localList)))
    {
        smsLog(pMac, LOGE, FL(" failed to open list"));
        return fRet;
    }

    csrLLLock(&pMac->sme.smeCmdPendingList);
    pEntry = csrLLPeekHead(&pMac->sme.smeCmdPendingList, LL_ACCESS_NOLOCK);
    while(pEntry)
    {
        pEntryTmp = csrLLNext(&pMac->sme.smeCmdPendingList, pEntry, LL_ACCESS_NOLOCK);
        pCommand = GET_BASE_ADDR(pEntry, tSmeCmd, Link);
        if( eSmeCommandScan == pCommand->command )
        {
            switch( pCommand->u.scanCmd.reason )
            {
            case eCsrScanIdleScan:
                if( csrLLRemoveEntry(&pMac->sme.smeCmdPendingList, pEntry, LL_ACCESS_NOLOCK) )
                {
                    csrLLInsertTail(&localList, pEntry, LL_ACCESS_NOLOCK);
                }
                fRet = eANI_BOOLEAN_TRUE;
                break;

            default:
                break;
            } //      
        }
        pEntry = pEntryTmp;
    }

    csrLLUnlock(&pMac->sme.smeCmdPendingList);

    while( (pEntry = csrLLRemoveHead(&localList, LL_ACCESS_NOLOCK)) )
    {
        pCommand = GET_BASE_ADDR(pEntry, tSmeCmd, Link);
        csrReleaseCommandScan( pMac, pCommand );
    }

    csrLLClose(&localList);

    return (fRet);
}


tANI_BOOLEAN csrScanRemoveFreshScanCommand(tpAniSirGlobal pMac, tANI_U8 sessionId)
{
    tANI_BOOLEAN fRet = eANI_BOOLEAN_FALSE;
    tListElem *pEntry, *pEntryTmp;
    tSmeCmd *pCommand;
    tDblLinkList localList;

    vos_mem_zero(&localList, sizeof(tDblLinkList));
    if(!HAL_STATUS_SUCCESS(csrLLOpen(pMac->hHdd, &localList)))
    {
        smsLog(pMac, LOGE, FL(" failed to open list"));
        return fRet;
    }

    csrLLLock(&pMac->sme.smeCmdPendingList);
    pEntry = csrLLPeekHead(&pMac->sme.smeCmdPendingList, LL_ACCESS_NOLOCK);
    while(pEntry)
    {
        pEntryTmp = csrLLNext(&pMac->sme.smeCmdPendingList, pEntry, LL_ACCESS_NOLOCK);
        pCommand = GET_BASE_ADDR(pEntry, tSmeCmd, Link);
        if( (eSmeCommandScan == pCommand->command) && (sessionId == pCommand->sessionId) )
        {
            switch(pCommand->u.scanCmd.reason)
            {
            case eCsrScanGetResult:
            case eCsrScanSetBGScanParam:
            case eCsrScanBGScanAbort:
            case eCsrScanBGScanEnable:
            case eCsrScanGetScanChnInfo:
                break;
            default:
                 smsLog (pMac, LOGW, "%s: -------- abort scan command reason = %d\n",
                    __FUNCTION__, pCommand->u.scanCmd.reason);
                //                                
                if( csrLLRemoveEntry(&pMac->sme.smeCmdPendingList, pEntry, LL_ACCESS_NOLOCK) )
                {
                    csrLLInsertTail(&localList, pEntry, LL_ACCESS_NOLOCK);
                }
                fRet = eANI_BOOLEAN_TRUE;
                break;
            }
        }
        pEntry = pEntryTmp;
    }

    csrLLUnlock(&pMac->sme.smeCmdPendingList);

    while( (pEntry = csrLLRemoveHead(&localList, LL_ACCESS_NOLOCK)) )
    {
        pCommand = GET_BASE_ADDR(pEntry, tSmeCmd, Link);
        if (pCommand->u.scanCmd.callback)
        {
            /*                               
                                                                            */
            pCommand->u.scanCmd.callback(pMac, 
                     pCommand->u.scanCmd.pContext, 
                     pCommand->u.scanCmd.scanID, 
                     eCSR_SCAN_ABORT);
        }
        csrReleaseCommandScan( pMac, pCommand );
    }
    csrLLClose(&localList);

    return (fRet);
}


void csrReleaseScanCommand(tpAniSirGlobal pMac, tSmeCmd *pCommand, eCsrScanStatus scanStatus)
{
    eCsrScanReason reason = pCommand->u.scanCmd.reason;
    tANI_U32 i;
    for( i = 0; i < CSR_ROAM_SESSION_MAX; i++ )
    {
         csrRoamStateChange( pMac, pCommand->u.scanCmd.lastRoamState[i], i);
    }

        csrScanCallCallback(pMac, pCommand, scanStatus);

    smsLog(pMac, LOG3, "   Remove Scan command reason = %d\n", reason);
    if( csrLLRemoveEntry( &pMac->sme.smeCmdActiveList, &pCommand->Link, LL_ACCESS_LOCK ) )
    {
        csrReleaseCommandScan( pMac, pCommand );
    }
    else
    {
        smsLog(pMac, LOGE, " ********csrReleaseScanCommand cannot release command reason %d\n", pCommand->u.scanCmd.reason );
    }
}


eHalStatus csrScanGetPMKIDCandidateList(tpAniSirGlobal pMac, tANI_U32 sessionId,
                                        tPmkidCandidateInfo *pPmkidList, tANI_U32 *pNumItems )
{
    eHalStatus status = eHAL_STATUS_SUCCESS;
    tCsrRoamSession *pSession = CSR_GET_SESSION( pMac, sessionId );

    if(!pSession)
    {
        smsLog(pMac, LOGE, FL("  session %d not found "), sessionId);
        return eHAL_STATUS_FAILURE;
    }

    smsLog(pMac, LOGW, "  pMac->scan.NumPmkidCandidate = %d\n ", pSession->NumPmkidCandidate);
    csrResetPMKIDCandidateList(pMac, sessionId);
    if(csrIsConnStateConnected(pMac, sessionId) && pSession->pCurRoamProfile)
    {
        tCsrScanResultFilter *pScanFilter;
        tCsrScanResultInfo *pScanResult;
        tScanResultHandle hBSSList;
        tANI_U32 nItems = *pNumItems;

        *pNumItems = 0;
        status = palAllocateMemory(pMac->hHdd, (void **)&pScanFilter, sizeof(tCsrScanResultFilter));
        if(HAL_STATUS_SUCCESS(status))
        {
            palZeroMemory(pMac->hHdd, pScanFilter, sizeof(tCsrScanResultFilter));
            //                                         
            status = csrRoamPrepareFilterFromProfile(pMac, pSession->pCurRoamProfile, pScanFilter);
            if(HAL_STATUS_SUCCESS(status))
            {
                status = csrScanGetResult(pMac, pScanFilter, &hBSSList);
                if(HAL_STATUS_SUCCESS(status))
                {
                    while(((pScanResult = csrScanResultGetNext(pMac, hBSSList)) != NULL) && ( pSession->NumPmkidCandidate < nItems))
                    {
                        //                              
                        csrProcessBSSDescForPMKIDList(pMac, &pScanResult->BssDescriptor, 
                                                      (tDot11fBeaconIEs *)( pScanResult->pvIes ));
                    }
                    if(pSession->NumPmkidCandidate)
                    {
                        *pNumItems = pSession->NumPmkidCandidate;
                        palCopyMemory(pMac->hHdd, pPmkidList, pSession->PmkidCandidateInfo, 
                                      pSession->NumPmkidCandidate * sizeof(tPmkidCandidateInfo));
                    }
                    csrScanResultPurge(pMac, hBSSList);
                }//                
                csrFreeScanFilter(pMac, pScanFilter);
            }
            palFreeMemory(pMac->hHdd, pScanFilter);
        }
    }

    return (status);
}



#ifdef FEATURE_WLAN_WAPI
eHalStatus csrScanGetBKIDCandidateList(tpAniSirGlobal pMac, tANI_U32 sessionId,
                                       tBkidCandidateInfo *pBkidList, tANI_U32 *pNumItems )
{
    eHalStatus status = eHAL_STATUS_SUCCESS;
    tCsrRoamSession *pSession = CSR_GET_SESSION( pMac, sessionId );

    if(!pSession)
    {
        smsLog(pMac, LOGE, FL("  session %d not found "), sessionId);
        return eHAL_STATUS_FAILURE;
    }

    smsLog(pMac, LOGW, "  pMac->scan.NumBkidCandidate = %d\n ", pSession->NumBkidCandidate);
    csrResetBKIDCandidateList(pMac, sessionId);
    if(csrIsConnStateConnected(pMac, sessionId) && pSession->pCurRoamProfile)
    {
        tCsrScanResultFilter *pScanFilter;
        tCsrScanResultInfo *pScanResult;
        tScanResultHandle hBSSList;
        tANI_U32 nItems = *pNumItems;
        *pNumItems = 0;
        status = palAllocateMemory(pMac->hHdd, (void **)&pScanFilter, sizeof(tCsrScanResultFilter));
        if(HAL_STATUS_SUCCESS(status))
        {
            palZeroMemory(pMac->hHdd, pScanFilter, sizeof(tCsrScanResultFilter));
            //                                         
            status = csrRoamPrepareFilterFromProfile(pMac, pSession->pCurRoamProfile, pScanFilter);
            if(HAL_STATUS_SUCCESS(status))
            {
                status = csrScanGetResult(pMac, pScanFilter, &hBSSList);
                if(HAL_STATUS_SUCCESS(status))
                {
                    while(((pScanResult = csrScanResultGetNext(pMac, hBSSList)) != NULL) && ( pSession->NumBkidCandidate < nItems))
                    {
                        //                                        
                        csrProcessBSSDescForBKIDList(pMac, &pScanResult->BssDescriptor,
                              (tDot11fBeaconIEs *)( pScanResult->pvIes ));

                    }
                    if(pSession->NumBkidCandidate)
                    {
                        *pNumItems = pSession->NumBkidCandidate;
                        palCopyMemory(pMac->hHdd, pBkidList, pSession->BkidCandidateInfo, pSession->NumBkidCandidate * sizeof(tBkidCandidateInfo));
                    }
                    csrScanResultPurge(pMac, hBSSList);
                }//                
            }
            palFreeMemory(pMac->hHdd, pScanFilter);
        }
    }

    return (status);
}
#endif /*                   */



//                                                                           
//                                
eHalStatus csrScanForSSID(tpAniSirGlobal pMac, tANI_U32 sessionId, tCsrRoamProfile *pProfile, tANI_U32 roamId)
{
    eHalStatus status = eHAL_STATUS_INVALID_PARAMETER;
    tSmeCmd *pScanCmd = NULL;
    tANI_U8 bAddr[6] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff}; 
    tANI_U8  index = 0;
    tANI_U32 numSsid = pProfile->SSIDs.numOfSSIDs;

    smsLog(pMac, LOG2, FL("called\n"));
    //                                                                
    if( CSR_IS_WDS_STA( pProfile ) && numSsid )
    {
        numSsid = 1;
    }
    if(pMac->scan.fScanEnable && ( numSsid == 1 ) )
    {
        do
        {
            pScanCmd = csrGetCommandBuffer(pMac);
            if(!pScanCmd)
            {
                smsLog(pMac, LOGE, FL("failed to allocate command buffer\n"));
                break;
            }
            palZeroMemory(pMac->hHdd, &pScanCmd->u.scanCmd, sizeof(tScanCmd));
            status = palAllocateMemory(pMac->hHdd, (void **)&pScanCmd->u.scanCmd.pToRoamProfile, sizeof(tCsrRoamProfile));
            if(!HAL_STATUS_SUCCESS(status))
                break;
            status = csrRoamCopyProfile(pMac, pScanCmd->u.scanCmd.pToRoamProfile, pProfile);
            if(!HAL_STATUS_SUCCESS(status))
                break;
            pScanCmd->u.scanCmd.roamId = roamId;
            pScanCmd->command = eSmeCommandScan;
            pScanCmd->sessionId = (tANI_U8)sessionId;
            pScanCmd->u.scanCmd.callback = NULL;
            pScanCmd->u.scanCmd.pContext = NULL;
            pScanCmd->u.scanCmd.reason = eCsrScanForSsid;
            pScanCmd->u.scanCmd.scanID = pMac->scan.nextScanID++; //                  
            palZeroMemory(pMac->hHdd, &pScanCmd->u.scanCmd.u.scanRequest, sizeof(tCsrScanRequest));
            pScanCmd->u.scanCmd.u.scanRequest.scanType = eSIR_ACTIVE_SCAN;
            pScanCmd->u.scanCmd.u.scanRequest.BSSType = pProfile->BSSType;
            //                                                                               
            if(VOS_P2P_CLIENT_MODE == pProfile->csrPersona)
            {
                pScanCmd->u.scanCmd.u.scanRequest.p2pSearch = 1;
            }
            if(pProfile->pAddIEScan)
            {
                status = palAllocateMemory(pMac->hHdd,
                                (void **)&pScanCmd->u.scanCmd.u.scanRequest.pIEField,
                                pProfile->nAddIEScanLength);
                palZeroMemory(pMac->hHdd, pScanCmd->u.scanCmd.u.scanRequest.pIEField, pProfile->nAddIEScanLength);
                if(HAL_STATUS_SUCCESS(status))
                {
                    palCopyMemory(pMac->hHdd, pScanCmd->u.scanCmd.u.scanRequest.pIEField, pProfile->pAddIEScan, pProfile->nAddIEScanLength);
                    pScanCmd->u.scanCmd.u.scanRequest.uIEFieldLen = pProfile->nAddIEScanLength;
                }
                else
                {
                    smsLog(pMac, LOGE, "No memory for scanning IE fields\n");
                }
            } //                            
            else
            {
                pScanCmd->u.scanCmd.u.scanRequest.uIEFieldLen = 0;
            }
            /*                                                               */
            if(  1 == pProfile->ChannelInfo.numOfChannels )
            {
                 pScanCmd->u.scanCmd.u.scanRequest.maxChnTime = MAX_ACTIVE_SCAN_FOR_ONE_CHANNEL;
                 pScanCmd->u.scanCmd.u.scanRequest.minChnTime = MIN_ACTIVE_SCAN_FOR_ONE_CHANNEL;
            }
            else
            {
                 pScanCmd->u.scanCmd.u.scanRequest.maxChnTime = pMac->roam.configParam.nActiveMaxChnTime;
                 pScanCmd->u.scanCmd.u.scanRequest.minChnTime = pMac->roam.configParam.nActiveMinChnTime;
            }
            if(pProfile->BSSIDs.numOfBSSIDs == 1)
            {
                palCopyMemory(pMac->hHdd, pScanCmd->u.scanCmd.u.scanRequest.bssid, pProfile->BSSIDs.bssid, sizeof(tCsrBssid));
            }
            else
            {
                palCopyMemory(pMac->hHdd, pScanCmd->u.scanCmd.u.scanRequest.bssid, bAddr, 6);
            }
            if(pProfile->ChannelInfo.numOfChannels)
            {
               status = palAllocateMemory(pMac->hHdd, (void **)&pScanCmd->u.scanCmd.u.scanRequest.ChannelInfo.ChannelList, sizeof(*pScanCmd->u.scanCmd.u.scanRequest.ChannelInfo.ChannelList) * pProfile->ChannelInfo.numOfChannels);
               pScanCmd->u.scanCmd.u.scanRequest.ChannelInfo.numOfChannels = 0;
               if(HAL_STATUS_SUCCESS(status))
                {
                  csrRoamIsChannelValid(pMac, pProfile->ChannelInfo.ChannelList[0]);
                  for(index = 0; index < pProfile->ChannelInfo.numOfChannels; index++)
                  {
                     if(csrRoamIsValidChannel(pMac, pProfile->ChannelInfo.ChannelList[index]))
                     {
                        pScanCmd->u.scanCmd.u.scanRequest.ChannelInfo.ChannelList[pScanCmd->u.scanCmd.u.scanRequest.ChannelInfo.numOfChannels] 
                           = pProfile->ChannelInfo.ChannelList[index];
                        pScanCmd->u.scanCmd.u.scanRequest.ChannelInfo.numOfChannels++;
                     }
                     else 
                     {
                         smsLog(pMac, LOGW, FL("process a channel (%d) that is invalid\n"), pProfile->ChannelInfo.ChannelList[index]);
                     }

                  }
               }
               else
                {
                    break;
                }

            }
            else
            {
                pScanCmd->u.scanCmd.u.scanRequest.ChannelInfo.numOfChannels = 0;
            }
            if(pProfile->SSIDs.numOfSSIDs)
            {
                status = palAllocateMemory(pMac->hHdd, (void **)&pScanCmd->u.scanCmd.u.scanRequest.SSIDs.SSIDList, 
                                            pProfile->SSIDs.numOfSSIDs * sizeof(tCsrSSIDInfo)); 
                if(!HAL_STATUS_SUCCESS(status))
                {
                    break;
                }
                pScanCmd->u.scanCmd.u.scanRequest.SSIDs.numOfSSIDs = 1;
                palCopyMemory(pMac->hHdd, pScanCmd->u.scanCmd.u.scanRequest.SSIDs.SSIDList, pProfile->SSIDs.SSIDList,
                                sizeof(tCsrSSIDInfo));
            }
            //                         
            status = csrQueueSmeCommand(pMac, pScanCmd, eANI_BOOLEAN_FALSE);
            if( !HAL_STATUS_SUCCESS( status ) )
            {
                smsLog( pMac, LOGE, FL(" fail to send message status = %d\n"), status );
                break;
            }
        }while(0);
        if(!HAL_STATUS_SUCCESS(status))
        {
            if(pScanCmd)
            {
                csrReleaseCommandScan(pMac, pScanCmd);
                //                                                       
            }
            csrRoamCallCallback(pMac, sessionId, NULL, roamId, eCSR_ROAM_FAILED, eCSR_ROAM_RESULT_FAILURE);
        }
    }//     
    else
    {
        smsLog(pMac, LOGE, FL("cannot scan because scanEnable (%d) or numSSID (%d) is invalid\n"),
                pMac->scan.fScanEnable, pProfile->SSIDs.numOfSSIDs);
    }
    
    return (status);
}


//                                                  
//                                                                      
//                                                                 
eHalStatus csrScanForCapabilityChange(tpAniSirGlobal pMac, tSirSmeApNewCaps *pNewCaps)
{
    eHalStatus status = eHAL_STATUS_INVALID_PARAMETER;
    tSmeCmd *pScanCmd = NULL;

    if(pNewCaps)
    {
        do
        {
            pScanCmd = csrGetCommandBuffer(pMac);
            if(!pScanCmd)
            {
                smsLog(pMac, LOGE, FL("failed to allocate command buffer\n"));
                status = eHAL_STATUS_RESOURCES;
                break;
            }
            palZeroMemory(pMac->hHdd, &pScanCmd->u.scanCmd, sizeof(tScanCmd));
            status = eHAL_STATUS_SUCCESS;
            pScanCmd->u.scanCmd.roamId = 0;
            pScanCmd->command = eSmeCommandScan; 
            pScanCmd->u.scanCmd.callback = NULL;
            pScanCmd->u.scanCmd.pContext = NULL;
            pScanCmd->u.scanCmd.reason = eCsrScanForCapsChange;
            pScanCmd->u.scanCmd.scanID = pMac->scan.nextScanID++; //                  
            status = csrQueueSmeCommand(pMac, pScanCmd, eANI_BOOLEAN_FALSE);
            if( !HAL_STATUS_SUCCESS( status ) )
            {
                smsLog( pMac, LOGE, FL(" fail to send message status = %d\n"), status );
                break;
            }
        }while(0);
        if(!HAL_STATUS_SUCCESS(status))
        {
            if(pScanCmd)
            {
                csrReleaseCommandScan(pMac, pScanCmd);
            }
        }    
    }

    return (status);
}



void csrInitBGScanChannelList(tpAniSirGlobal pMac)
{
    tANI_U32 len = CSR_MIN(sizeof(pMac->roam.validChannelList), sizeof(pMac->scan.bgScanChannelList));

    palZeroMemory(pMac->hHdd, pMac->scan.bgScanChannelList, len);
    pMac->scan.numBGScanChannel = 0;

    if(HAL_STATUS_SUCCESS(csrGetCfgValidChannels(pMac, pMac->roam.validChannelList, &len)))
    {
        pMac->roam.numValidChannels = len;
        pMac->scan.numBGScanChannel = (tANI_U8)CSR_MIN(len, WNI_CFG_BG_SCAN_CHANNEL_LIST_LEN);
        palCopyMemory(pMac->hHdd, pMac->scan.bgScanChannelList, pMac->roam.validChannelList, pMac->scan.numBGScanChannel);
        csrSetBGScanChannelList(pMac, pMac->scan.bgScanChannelList, pMac->scan.numBGScanChannel);
    }
}


//                                                                       
//                                                               
tANI_BOOLEAN csrAdjustBGScanChannelList(tpAniSirGlobal pMac, tANI_U8 *pChannelList, tANI_U8 NumChannels,
                                        tANI_U8 *pAdjustChannels, tANI_U8 *pNumAdjustChannels)
{
    tANI_BOOLEAN fRet = eANI_BOOLEAN_FALSE;
    tANI_U8 i, j, count = *pNumAdjustChannels;

    i = 0;
    while(i < count)
    {
        for(j = 0; j < NumChannels; j++)
        {
            if(pChannelList[j] == pAdjustChannels[i])
                break;
        }
        if(j == NumChannels)
        {
            //                                          
            fRet = eANI_BOOLEAN_TRUE;
            count--;
            if(count - i)
            {
                palCopyMemory(pMac->hHdd, &pAdjustChannels[i], &pAdjustChannels[i+1], count - i);
            }
            else
            {
                //                                  
                break;
            }
        }
        else
        {
            i++;
        }
    }//              
    *pNumAdjustChannels = count;

    return (fRet);
}


//                                                                
eHalStatus csrScanGetSupportedChannels( tpAniSirGlobal pMac )
{
    eHalStatus status = eHAL_STATUS_SUCCESS;
    int n = WNI_CFG_VALID_CHANNEL_LIST_LEN;

    status = vos_nv_getSupportedChannels( pMac->scan.baseChannels.channelList, &n, NULL, NULL );
    if( HAL_STATUS_SUCCESS(status) )
    {
        pMac->scan.baseChannels.numChannels = (tANI_U8)n;
    }
    else
    {
        smsLog( pMac, LOGE, FL(" failed\n") );
        pMac->scan.baseChannels.numChannels = 0;
    }
    
    return ( status );
}

//                                                                                   
eHalStatus csrSetBGScanChannelList( tpAniSirGlobal pMac, tANI_U8 *pAdjustChannels, tANI_U8 NumAdjustChannels)
{
    tANI_U32 dataLen = sizeof( tANI_U8 ) * NumAdjustChannels;

    return (ccmCfgSetStr(pMac, WNI_CFG_BG_SCAN_CHANNEL_LIST, pAdjustChannels, dataLen, NULL, eANI_BOOLEAN_FALSE));
}


void csrSetCfgValidChannelList( tpAniSirGlobal pMac, tANI_U8 *pChannelList, tANI_U8 NumChannels )
{
    tANI_U32 dataLen = sizeof( tANI_U8 ) * NumChannels;


    ccmCfgSetStr(pMac, WNI_CFG_VALID_CHANNEL_LIST, pChannelList, dataLen, NULL, eANI_BOOLEAN_FALSE);

    return;
}



/*
                                                             
 */
void csrSaveTxPowerToCfg( tpAniSirGlobal pMac, tDblLinkList *pList, tANI_U32 cfgId )
{
    tListElem *pEntry;
    tANI_U32 cbLen = 0, dataLen;
    tCsrChannelPowerInfo *pChannelSet;
    tANI_U32 idx;
    tSirMacChanInfo *pChannelPowerSet;
    tANI_U8 *pBuf = NULL;

    //                                       
    dataLen = WNI_CFG_VALID_CHANNEL_LIST_LEN * sizeof(tSirMacChanInfo);
    if(HAL_STATUS_SUCCESS(palAllocateMemory(pMac->hHdd, (void **)&pBuf, dataLen)))
    {
        palZeroMemory(pMac->hHdd, pBuf, dataLen);
        pChannelPowerSet = (tSirMacChanInfo *)(pBuf);

        pEntry = csrLLPeekHead( pList, LL_ACCESS_LOCK );
        //                                                                                                 
        while( pEntry )
        {
            pChannelSet = GET_BASE_ADDR( pEntry, tCsrChannelPowerInfo, link );
            if ( 1 != pChannelSet->interChannelOffset )
            {
                //                                                                                       
                //                                                                                      
                //                     
                if ((cbLen + (pChannelSet->numChannels * sizeof(tSirMacChanInfo))) >= dataLen)
                {
                    //                                                  
                    smsLog(pMac, LOGE,
                           "%s: Buffer overflow, start %d, num %d, offset %d",
                           __FUNCTION__,
                           pChannelSet->firstChannel,
                           pChannelSet->numChannels,
                           pChannelSet->interChannelOffset);
                    break;
                }

                for( idx = 0; idx < pChannelSet->numChannels; idx++ )
                {
                    pChannelPowerSet->firstChanNum = (tSirMacChanNum)(pChannelSet->firstChannel + ( idx * pChannelSet->interChannelOffset ));
                    smsLog(pMac, LOG3, " Setting Channel Number %d\n", pChannelPowerSet->firstChanNum);
                    pChannelPowerSet->numChannels  = 1;
#ifdef WLAN_SOFTAP_FEATURE
                    pChannelPowerSet->maxTxPower = CSR_ROAM_MIN( pChannelSet->txPower, pMac->roam.configParam.nTxPowerCap );
#else
                    pChannelPowerSet->maxTxPower = pChannelSet->txPower;
#endif
                    smsLog(pMac, LOG3, " Setting Max Transmit Power %d\n", pChannelPowerSet->maxTxPower);
                    cbLen += sizeof( tSirMacChanInfo );
                    pChannelPowerSet++;
                }
            }
            else
            {
                if (cbLen >= dataLen)
                {
                    //                                        
                    smsLog(pMac, LOGE,
                           "%s: Buffer overflow, start %d, num %d, offset %d",
                           __FUNCTION__,
                           pChannelSet->firstChannel,
                           pChannelSet->numChannels,
                           pChannelSet->interChannelOffset);
                    break;
                }
                pChannelPowerSet->firstChanNum = pChannelSet->firstChannel;
                smsLog(pMac, LOG3, " Setting Channel Number %d\n", pChannelPowerSet->firstChanNum);
                pChannelPowerSet->numChannels = pChannelSet->numChannels;
#ifdef WLAN_SOFTAP_FEATURE
                pChannelPowerSet->maxTxPower = CSR_ROAM_MIN( pChannelSet->txPower, pMac->roam.configParam.nTxPowerCap );
#else
                pChannelPowerSet->maxTxPower = pChannelSet->txPower;
#endif
                smsLog(pMac, LOG3, " Setting Max Transmit Power %d, nTxPower %d\n", pChannelPowerSet->maxTxPower,pMac->roam.configParam.nTxPowerCap );


                cbLen += sizeof( tSirMacChanInfo );
                pChannelPowerSet++;
            }

            pEntry = csrLLNext( pList, pEntry, LL_ACCESS_LOCK );
        }

        if(cbLen)
        {
            ccmCfgSetStr(pMac, cfgId, (tANI_U8 *)pBuf, cbLen, NULL, eANI_BOOLEAN_FALSE); 
        }
        palFreeMemory( pMac->hHdd, pBuf );
    }//               
}


void csrSetCfgCountryCode( tpAniSirGlobal pMac, tANI_U8 *countryCode )
{
    tANI_U8 cc[WNI_CFG_COUNTRY_CODE_LEN];
    //                        
    
    smsLog( pMac, LOG3, "Setting Country Code in Cfg from csrSetCfgCountryCode %s\n",countryCode );
    palCopyMemory( pMac->hHdd, cc, countryCode, WNI_CFG_COUNTRY_CODE_LEN );

    //                                                                                       
    //                                                                      
    if ( ( 'K'  == countryCode[ 0 ] && '1' == countryCode[ 1 ]  ) ||
         ( 'K'  == countryCode[ 0 ] && '2' == countryCode[ 1 ]  ) ||
         ( 'K'  == countryCode[ 0 ] && '3' == countryCode[ 1 ]  ) ||
         ( 'K'  == countryCode[ 0 ] && '4' == countryCode[ 1 ]  )    )
    {
        //                                                                              
        cc[ 1 ] = 'R';
    }
    ccmCfgSetStr(pMac, WNI_CFG_COUNTRY_CODE, cc, WNI_CFG_COUNTRY_CODE_LEN, NULL, eANI_BOOLEAN_FALSE);

    //                                                                      
    //                                       
    /*                                                                             
     
                                           
     */
}



eHalStatus csrGetCountryCode(tpAniSirGlobal pMac, tANI_U8 *pBuf, tANI_U8 *pbLen)
{
    eHalStatus status = eHAL_STATUS_INVALID_PARAMETER;
    tANI_U32 len;

    if(pBuf && pbLen && (*pbLen >= WNI_CFG_COUNTRY_CODE_LEN))
    {
        len = *pbLen;
        status = ccmCfgGetStr(pMac, WNI_CFG_COUNTRY_CODE, pBuf, &len);
        if(HAL_STATUS_SUCCESS(status))
        {
            *pbLen = (tANI_U8)len;
        }
    }
    
    return (status);
}


void csrSetCfgScanControlList( tpAniSirGlobal pMac, tANI_U8 *countryCode, tCsrChannel *pChannelList  )
{   
    tANI_U8 i, j;
    tANI_BOOLEAN found=FALSE;  
    tANI_U8 *pControlList = NULL;
    tANI_U32 len = WNI_CFG_SCAN_CONTROL_LIST_LEN;

    if(HAL_STATUS_SUCCESS(palAllocateMemory(pMac->hHdd, (void **)&pControlList, WNI_CFG_SCAN_CONTROL_LIST_LEN)))
    {
        palZeroMemory(pMac->hHdd, (void *)pControlList, WNI_CFG_SCAN_CONTROL_LIST_LEN);
        if(HAL_STATUS_SUCCESS(ccmCfgGetStr(pMac, WNI_CFG_SCAN_CONTROL_LIST, pControlList, &len)))
        {
            for (i = 0; i < pChannelList->numChannels; i++)
            {
                for (j = 0; j < len; j += 2) 
                {
                    if (pControlList[j] == pChannelList->channelList[i]) 
                    {
                        found = TRUE;
                        break;
                    }
                }
                   
                if (found)    //                              
                {
                    if (CSR_IS_CHANNEL_5GHZ(pControlList[j]))
                    {
                        pControlList[j+1] = csrGetScanType(pMac, pControlList[j]);     
                    }
                    else  
                    {
                        pControlList[j+1]  = eSIR_ACTIVE_SCAN;  
                    }

                    found = FALSE;  //               
                }
                       
            }            

            ccmCfgSetStr(pMac, WNI_CFG_SCAN_CONTROL_LIST, pControlList, len, NULL, eANI_BOOLEAN_FALSE);
        }//                                      
        palFreeMemory(pMac->hHdd, pControlList);
    }//              
}


//                                                                         
eHalStatus csrSetCfgBackgroundScanPeriod(tpAniSirGlobal pMac, tANI_U32 bgPeriod)
{
    return (ccmCfgSetInt(pMac, WNI_CFG_BACKGROUND_SCAN_PERIOD, bgPeriod, (tCcmCfgSetCallback) csrScanCcmCfgSetCallback, eANI_BOOLEAN_FALSE));
}
    

void csrScanCcmCfgSetCallback(tHalHandle hHal, tANI_S32 result)
{
    tListElem *pEntry = NULL;
    tSmeCmd *pCommand = NULL;
    tpAniSirGlobal pMac = PMAC_STRUCT( hHal );
    
    pEntry = csrLLPeekHead( &pMac->sme.smeCmdActiveList, LL_ACCESS_LOCK );
    if ( pEntry )
    {
        pCommand = GET_BASE_ADDR( pEntry, tSmeCmd, Link );
        if ( eSmeCommandScan == pCommand->command )
        {
            eCsrScanStatus scanStatus = (CCM_IS_RESULT_SUCCESS(result)) ? eCSR_SCAN_SUCCESS : eCSR_SCAN_FAILURE;
            csrReleaseScanCommand(pMac, pCommand, scanStatus);
        }
        else
        {
            smsLog( pMac, LOGW, "CSR: Scan Completion called but SCAN command is not ACTIVE ...\n" );
        }
    }   
    smeProcessPendingQueue( pMac );
}

eHalStatus csrProcessSetBGScanParam(tpAniSirGlobal pMac, tSmeCmd *pCommand)
{
    eHalStatus status;
    tCsrBGScanRequest *pScanReq = &pCommand->u.scanCmd.u.bgScanRequest;
    tANI_U32 dataLen = sizeof( tANI_U8 ) * pScanReq->ChannelInfo.numOfChannels;
        
    //                                          
    status = ccmCfgSetInt(pMac, WNI_CFG_ACTIVE_MINIMUM_CHANNEL_TIME, pScanReq->minChnTime, NULL, eANI_BOOLEAN_FALSE);
    status = ccmCfgSetInt(pMac, WNI_CFG_ACTIVE_MAXIMUM_CHANNEL_TIME, pScanReq->maxChnTime, NULL, eANI_BOOLEAN_FALSE);
    //                                                                                                        
    if(!csrIsAllSessionDisconnected(pMac))
    {
        //                                                        
        if(pScanReq->scanInterval == 0)
        {
            //                                             
            csrScanStopResultAgingTimer(pMac);
        }

#ifdef FEATURE_WLAN_DIAG_SUPPORT_CSR
        {
            vos_log_scan_pkt_type *pScanLog = NULL;

            WLAN_VOS_DIAG_LOG_ALLOC(pScanLog, vos_log_scan_pkt_type, LOG_WLAN_SCAN_C);
            if(pScanLog)
            {
                pScanLog->eventId = WLAN_SCAN_EVENT_HO_SCAN_REQ;
                pScanLog->minChnTime = (v_U8_t)pScanReq->minChnTime;
                pScanLog->maxChnTime = (v_U8_t)pScanReq->maxChnTime;
                pScanLog->timeBetweenBgScan = (v_U8_t)pScanReq->scanInterval;
                pScanLog->numChannel = pScanReq->ChannelInfo.numOfChannels;
                if(pScanLog->numChannel && (pScanLog->numChannel < VOS_LOG_MAX_NUM_CHANNEL))
                {
                    palCopyMemory(pMac->hHdd, pScanLog->channels, pScanReq->ChannelInfo.ChannelList,
                        pScanLog->numChannel);
                }
                WLAN_VOS_DIAG_LOG_REPORT(pScanLog);
            }
        }
#endif //                                    

        status = ccmCfgSetInt(pMac, WNI_CFG_BACKGROUND_SCAN_PERIOD, pScanReq->scanInterval, NULL, eANI_BOOLEAN_FALSE);
    }
    else
    {
        //                                                        
        status = ccmCfgSetInt(pMac, WNI_CFG_BACKGROUND_SCAN_PERIOD, 0, NULL, eANI_BOOLEAN_FALSE);
    }
    
    if(pScanReq->SSID.length > WNI_CFG_SSID_LEN)
    {
        pScanReq->SSID.length = WNI_CFG_SSID_LEN;
    }
    
    status = ccmCfgSetStr(pMac, WNI_CFG_BG_SCAN_CHANNEL_LIST, pScanReq->ChannelInfo.ChannelList, dataLen, NULL, eANI_BOOLEAN_FALSE);
    status = ccmCfgSetStr(pMac, WNI_CFG_SSID, (tANI_U8 *)pScanReq->SSID.ssId, pScanReq->SSID.length, NULL, eANI_BOOLEAN_FALSE);



    return (status);
}


eHalStatus csrScanAbortMacScan(tpAniSirGlobal pMac)
{
    eHalStatus status = eHAL_STATUS_SUCCESS;
    tSirMbMsg *pMsg;
    tANI_U16 msgLen;
    tListElem *pEntry;
    tSmeCmd *pCommand;

#ifdef WLAN_AP_STA_CONCURRENCY
    csrLLLock(&pMac->scan.scanCmdPendingList);
    while( NULL != ( pEntry = csrLLRemoveHead( &pMac->scan.scanCmdPendingList, LL_ACCESS_NOLOCK) ) )
    {

        pCommand = GET_BASE_ADDR( pEntry, tSmeCmd, Link );
        csrAbortCommand( pMac, pCommand, eANI_BOOLEAN_FALSE);
    }
    csrLLUnlock(&pMac->scan.scanCmdPendingList);
#endif

    pMac->scan.fDropScanCmd = eANI_BOOLEAN_TRUE;
    csrRemoveCmdFromPendingList( pMac, &pMac->roam.roamCmdPendingList, eSmeCommandScan);
    csrRemoveCmdFromPendingList( pMac, &pMac->sme.smeCmdPendingList, eSmeCommandScan);
    pMac->scan.fDropScanCmd = eANI_BOOLEAN_FALSE;

    //                                             
    if(NULL != (pEntry = csrLLPeekHead(&pMac->sme.smeCmdActiveList, LL_ACCESS_LOCK)))
    {
        pCommand = GET_BASE_ADDR( pEntry, tSmeCmd, Link );
        if(eSmeCommandScan == pCommand->command)
        {
            msgLen = (tANI_U16)(sizeof( tSirMbMsg ));
            status = palAllocateMemory(pMac->hHdd, (void **)&pMsg, msgLen);
            if(HAL_STATUS_SUCCESS(status))
            {
                palZeroMemory(pMac->hHdd, (void *)pMsg, msgLen);
                pMsg->type = pal_cpu_to_be16((tANI_U16)eWNI_SME_SCAN_ABORT_IND);
                pMsg->msgLen = pal_cpu_to_be16(msgLen);
                status = palSendMBMessage(pMac->hHdd, pMsg);
            }
        }
    }

    return( status );
}

void csrRemoveCmdFromPendingList(tpAniSirGlobal pMac, tDblLinkList *pList,
                                 eSmeCommandType commandType )
{
    tDblLinkList localList;
    tListElem *pEntry;
    tSmeCmd   *pCommand;
    tListElem  *pEntryToRemove;

    vos_mem_zero(&localList, sizeof(tDblLinkList));
    if(!HAL_STATUS_SUCCESS(csrLLOpen(pMac->hHdd, &localList)))
    {
        smsLog(pMac, LOGE, FL(" failed to open list"));
        return;
    }

    csrLLLock(pList);
    if( !csrLLIsListEmpty( pList, LL_ACCESS_NOLOCK ) )
    {
        pEntry = csrLLPeekHead( pList, LL_ACCESS_NOLOCK);

        //                                                                         
        //                                          
        while( pEntry )
        {
            pEntryToRemove = pEntry;
            pEntry = csrLLNext(pList, pEntry, LL_ACCESS_NOLOCK);
            pCommand = GET_BASE_ADDR( pEntryToRemove, tSmeCmd, Link );
            if ( pCommand->command == commandType )
            {
                //                       
                if(csrLLRemoveEntry( pList, pEntryToRemove, LL_ACCESS_NOLOCK))
                {
                    csrLLInsertTail(&localList, pEntryToRemove, LL_ACCESS_NOLOCK);
                }
            }
        }


    }
    csrLLUnlock(pList);

    while( (pEntry = csrLLRemoveHead(&localList, LL_ACCESS_NOLOCK)) )
    {
        pCommand = GET_BASE_ADDR( pEntry, tSmeCmd, Link );
        csrAbortCommand( pMac, pCommand, eANI_BOOLEAN_FALSE);
    }
    csrLLClose(&localList);

}


eHalStatus csrScanAbortMacScanNotForConnect(tpAniSirGlobal pMac)
{
    eHalStatus status = eHAL_STATUS_SUCCESS;

    if( !csrIsScanForRoamCommandActive( pMac ) )
    {
        //                                                                    
        status = csrScanAbortMacScan(pMac);
    }

    return (status);
}


eHalStatus csrScanGetScanChannelInfo(tpAniSirGlobal pMac)
{
    eHalStatus status = eHAL_STATUS_SUCCESS;
    tSirMbMsg *pMsg;
    tANI_U16 msgLen;

    msgLen = (tANI_U16)(sizeof( tSirMbMsg ));
    status = palAllocateMemory(pMac->hHdd, (void **)&pMsg, msgLen);
    if(HAL_STATUS_SUCCESS(status))
    {
        palZeroMemory(pMac->hHdd, pMsg, msgLen);
        pMsg->type = eWNI_SME_GET_SCANNED_CHANNEL_REQ;
        pMsg->msgLen = msgLen;
        status = palSendMBMessage(pMac->hHdd, pMsg);
    }                             

    return( status );
}

tANI_BOOLEAN csrRoamIsValidChannel( tpAniSirGlobal pMac, tANI_U8 channel )
{
    tANI_BOOLEAN fValid = FALSE;
    tANI_U32 idxValidChannels;
    tANI_U32 len = pMac->roam.numValidChannels;
    
    for ( idxValidChannels = 0; ( idxValidChannels < len ); idxValidChannels++ )
    {
       if ( channel == pMac->roam.validChannelList[ idxValidChannels ] )
       {
          fValid = TRUE;
          break;
       }
    }
        
    return fValid;
}


