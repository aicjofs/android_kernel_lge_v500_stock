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
/** ------------------------------------------------------------------------- * 
    ------------------------------------------------------------------------- *  

  
    \file csrNeighborRoam.c
  
    Implementation for the simple roaming algorithm for 802.11r Fast transitions and Legacy roaming for Android platform.
  
    Copyright (C) 2010 Qualcomm, Incorporated
  
 
   ========================================================================== */

/*                                                                           

                                           


                                                                       
                                                                



                                                     
                                                                                            
                                            

                                                                           */
#ifdef WLAN_FEATURE_NEIGHBOR_ROAMING
#include "wlan_qct_wda.h"
#include "palApi.h"
#include "csrInsideApi.h"
#include "smsDebug.h"
#include "logDump.h"
#include "smeQosInternal.h"
#include "wlan_qct_tl.h"
#include "smeInside.h"
#include "vos_diag_core_event.h"
#include "vos_diag_core_log.h"
#include "csrApi.h"
#include "wlan_qct_tl.h"
#include "sme_Api.h"
#include "csrNeighborRoam.h"
#ifdef FEATURE_WLAN_CCX
#include "csrCcx.h"
#endif

#define WLAN_FEATURE_NEIGHBOR_ROAMING_DEBUG 1
#ifdef WLAN_FEATURE_NEIGHBOR_ROAMING_DEBUG
#define NEIGHBOR_ROAM_DEBUG smsLog
#else
#define NEIGHBOR_ROAM_DEBUG(x...)
#endif

static void csrNeighborRoamResetChannelInfo(tpCsrNeighborRoamChannelInfo rChInfo);
static void csrNeighborRoamResetCfgListChanScanControlInfo(tpAniSirGlobal pMac);
static void csrNeighborRoamResetPreauthControlInfo(tpAniSirGlobal pMac);
static void csrNeighborRoamDeregAllRssiIndication(tpAniSirGlobal pMac);

VOS_STATUS csrNeighborRoamNeighborLookupUPCallback (v_PVOID_t pAdapter, v_U8_t rssiNotification,
                                                                               v_PVOID_t pUserCtxt);
VOS_STATUS csrNeighborRoamNeighborLookupDOWNCallback (v_PVOID_t pAdapter, v_U8_t rssiNotification,
                                                                               v_PVOID_t pUserCtxt);
void csrNeighborRoamRRMNeighborReportResult(void *context, VOS_STATUS vosStatus);
eHalStatus csrRoamCopyConnectedProfile(tpAniSirGlobal pMac, tANI_U32 sessionId, tCsrRoamProfile *pDstProfile );

#ifdef WLAN_FEATURE_VOWIFI_11R
static eHalStatus csrNeighborRoamIssuePreauthReq(tpAniSirGlobal pMac);
VOS_STATUS csrNeighborRoamIssueNeighborRptRequest(tpAniSirGlobal pMac);
#endif

/*                        */
#define CSR_NEIGHBOR_ROAM_STATE_TRANSITION(newState)\
{\
    pMac->roam.neighborRoamInfo.prevNeighborRoamState = pMac->roam.neighborRoamInfo.neighborRoamState;\
    pMac->roam.neighborRoamInfo.neighborRoamState = newState;\
    smsLog(pMac, LOG1, FL("Neighbor Roam Transition from state %d ==> %d"), pMac->roam.neighborRoamInfo.prevNeighborRoamState, newState);\
}

/*                                                                            

                                              

                                                                                    
                                          

                                                  
                                                                    

                

                                                                           */
void csrNeighborRoamFreeNeighborRoamBSSNode(tpAniSirGlobal pMac, tpCsrNeighborRoamBSSInfo neighborRoamBSSNode)
{
    if (neighborRoamBSSNode)
    {
        if (neighborRoamBSSNode->pBssDescription)
        {
            vos_mem_free(neighborRoamBSSNode->pBssDescription);
            neighborRoamBSSNode->pBssDescription = NULL;
        }
        vos_mem_free(neighborRoamBSSNode);
        neighborRoamBSSNode = NULL;
    }

    return;
}

/*                                                                            

                                                

                                                                   

                                                  
                                                                   
                                                                 

                                                    

                                                                           */
tANI_BOOLEAN csrNeighborRoamRemoveRoamableAPListEntry(tpAniSirGlobal pMac, 
                                                tDblLinkList *pList, tpCsrNeighborRoamBSSInfo pNeighborEntry)
{
    if(pList)
    {
        return csrLLRemoveEntry(pList, &pNeighborEntry->List, LL_ACCESS_LOCK);
    }

    smsLog(pMac, LOGE, FL("Removing neighbor BSS node from list failed. Current count = %d\n"), csrLLCount(pList));

    return eANI_BOOLEAN_FALSE;
}

/*                                                                            

                                                 

                                                                                                            

                                                  
                                                                    
                                                                                       

                                                 

                                                                           */
tpCsrNeighborRoamBSSInfo csrNeighborRoamGetRoamableAPListNextEntry(tpAniSirGlobal pMac, 
                                        tDblLinkList *pList, tpCsrNeighborRoamBSSInfo pNeighborEntry)
{
    tListElem *pEntry = NULL;
    tpCsrNeighborRoamBSSInfo pResult = NULL;
    
    if(pList)
    {
        if(NULL == pNeighborEntry)
        {
            pEntry = csrLLPeekHead(pList, LL_ACCESS_LOCK);
        }
        else
        {
            pEntry = csrLLNext(pList, &pNeighborEntry->List, LL_ACCESS_LOCK);
        }
        if(pEntry)
        {
            pResult = GET_BASE_ADDR(pEntry, tCsrNeighborRoamBSSInfo, List);
        }
    }
    
    return pResult;
}

/*                                                                            

                                          

                                                                     

                                                  
                                                        

                

                                                                           */
void csrNeighborRoamFreeRoamableBSSList(tpAniSirGlobal pMac, tDblLinkList *pList)
{
    tpCsrNeighborRoamBSSInfo pResult = NULL;

    NEIGHBOR_ROAM_DEBUG(pMac, LOG2, FL("Emptying the BSS list. Current count = %d\n"), csrLLCount(pList));

    /*                                                                        */
    while ((pResult = csrNeighborRoamGetRoamableAPListNextEntry(pMac, pList, NULL)) != NULL)
    {
        csrNeighborRoamRemoveRoamableAPListEntry(pMac, pList, pResult);
        csrNeighborRoamFreeNeighborRoamBSSNode(pMac, pResult);
    }
    return;
}

/*                                                                            

                                         

                                                                                        
                                                              
                                                                                           

                                  
                                                      
                                                                                                      

                

                                                                           */
VOS_STATUS csrNeighborRoamReassocIndCallback(v_PVOID_t pAdapter, 
                               v_U8_t trafficStatus, 
                               v_PVOID_t pUserCtxt)
{
    tpAniSirGlobal pMac = PMAC_STRUCT( pUserCtxt );
    tpCsrNeighborRoamControlInfo    pNeighborRoamInfo = &pMac->roam.neighborRoamInfo;
    VOS_STATUS vosStatus = VOS_STATUS_SUCCESS;   
 
    smsLog(pMac, LOG1, FL("Reassoc indication callback called"));


    //                                                                                                               

    NEIGHBOR_ROAM_DEBUG(pMac, LOG2, FL("Deregistering DOWN event reassoc callback with TL. RSSI = %d"), pNeighborRoamInfo->cfgParams.neighborReassocThreshold * (-1));


    vosStatus = WLANTL_DeregRSSIIndicationCB(pMac->roam.gVosContext, (v_S7_t)pNeighborRoamInfo->cfgParams.neighborReassocThreshold * (-1),
                                                        WLANTL_HO_THRESHOLD_DOWN, 
                                                        csrNeighborRoamReassocIndCallback,
                                                        VOS_MODULE_ID_SME);
                        
    if(!VOS_IS_STATUS_SUCCESS(vosStatus))
    {
        //       
        smsLog(pMac, LOGW, FL(" Couldn't deregister csrNeighborRoamReassocIndCallback with TL: Status = %d\n"), vosStatus);
    }
    
    NEIGHBOR_ROAM_DEBUG(pMac, LOG2, FL("Deregistering UP event neighbor lookup callback with TL. RSSI = %d"), pNeighborRoamInfo->cfgParams.neighborLookupThreshold * (-1));
    /*                                                   */
    /*          */
    vosStatus = WLANTL_DeregRSSIIndicationCB(pMac->roam.gVosContext, (v_S7_t)pNeighborRoamInfo->cfgParams.neighborLookupThreshold * (-1),
                                                        WLANTL_HO_THRESHOLD_DOWN, 
                                                        csrNeighborRoamNeighborLookupUPCallback,
                                                        VOS_MODULE_ID_SME);
                        
    if(!VOS_IS_STATUS_SUCCESS(vosStatus))
    {
        //       
        smsLog(pMac, LOGW, FL(" Couldn't deregister csrNeighborRoamReassocIndCallback with TL: Status = %d\n"), vosStatus);
    }

    /*                                          */
    palTimerStop(pMac->hHdd, pNeighborRoamInfo->neighborResultsRefreshTimer);

#ifdef WLAN_FEATURE_VOWIFI_11R
    if (pNeighborRoamInfo->is11rAssoc)
    {
        if (eCSR_NEIGHBOR_ROAM_STATE_REPORT_SCAN == pNeighborRoamInfo->neighborRoamState)
        {
            csrNeighborRoamIssuePreauthReq(pMac);
        }
        else
        {
            smsLog(pMac, LOGE, FL("11R Reassoc indication received in unexpected state %d"), pNeighborRoamInfo->neighborRoamState);
            VOS_ASSERT(0);
        }
    }
    else
#endif

#ifdef FEATURE_WLAN_CCX
    if (pNeighborRoamInfo->isCCXAssoc)
    {
        if (eCSR_NEIGHBOR_ROAM_STATE_REPORT_SCAN == pNeighborRoamInfo->neighborRoamState)
        {
            csrNeighborRoamIssuePreauthReq(pMac);
        }
        else
        {
            smsLog(pMac, LOGE, FL("CCX Reassoc indication received in unexpected state %d"), pNeighborRoamInfo->neighborRoamState);
            VOS_ASSERT(0);
        }
    }
    else
#endif
#ifdef FEATURE_WLAN_LFR
    if (csrRoamIsFastRoamEnabled(pMac))
    {
        if (eCSR_NEIGHBOR_ROAM_STATE_REPORT_SCAN == pNeighborRoamInfo->neighborRoamState)
        {
            csrNeighborRoamIssuePreauthReq(pMac);
        }
        else
        {
            smsLog(pMac, LOGE, FL("LFR Reassoc indication received in unexpected state %d"), pNeighborRoamInfo->neighborRoamState);
            VOS_ASSERT(0);
        }
    }
    else
#endif
    {
        if (eCSR_NEIGHBOR_ROAM_STATE_CFG_CHAN_LIST_SCAN == pNeighborRoamInfo->neighborRoamState)
        {
            csrNeighborRoamRequestHandoff(pMac);
        }
        else
        {
            smsLog(pMac, LOGE, FL("Non-11R Reassoc indication received in unexpected state %d"), pNeighborRoamInfo->neighborRoamState);
            VOS_ASSERT(0);
        }
    }
    return VOS_STATUS_SUCCESS;
}

/*                */
static void csrNeighborRoamResetChannelInfo(tpCsrNeighborRoamChannelInfo rChInfo)
{
        if ((rChInfo->IAPPNeighborListReceived == FALSE) &&
                        (rChInfo->currentChannelListInfo.numOfChannels))
    {
                rChInfo->currentChanIndex = CSR_NEIGHBOR_ROAM_INVALID_CHANNEL_INDEX;
                rChInfo->currentChannelListInfo.numOfChannels = 0;

                if (rChInfo->currentChannelListInfo.ChannelList)
                        vos_mem_free(rChInfo->currentChannelListInfo.ChannelList);
    
                rChInfo->currentChannelListInfo.ChannelList = NULL;
                rChInfo->chanListScanInProgress = eANI_BOOLEAN_FALSE;
    }
    else 
    {
                rChInfo->currentChanIndex = 0;
                rChInfo->chanListScanInProgress = eANI_BOOLEAN_TRUE;
        }
    }

static void csrNeighborRoamResetCfgListChanScanControlInfo(tpAniSirGlobal pMac)
{
        tpCsrNeighborRoamControlInfo pNeighborRoamInfo = &pMac->roam.neighborRoamInfo;

        /*                          */
    palTimerStop(pMac->hHdd, pNeighborRoamInfo->neighborScanTimer);

        /*                                          */
        palTimerStop(pMac->hHdd, pNeighborRoamInfo->neighborResultsRefreshTimer);

        /*                        */
    if (eANI_BOOLEAN_TRUE == pNeighborRoamInfo->scanRspPending)
    {
                csrScanAbortMacScan(pMac);
        }
        pNeighborRoamInfo->scanRspPending = eANI_BOOLEAN_FALSE;

        /*                                     */
        csrNeighborRoamResetChannelInfo(&pNeighborRoamInfo->roamChannelInfo);
    }

static void csrNeighborRoamResetPreauthControlInfo(tpAniSirGlobal pMac)
{
        tpCsrNeighborRoamControlInfo pNeighborRoamInfo = &pMac->roam.neighborRoamInfo;
    
#if  defined (WLAN_FEATURE_VOWIFI_11R) || defined (FEATURE_WLAN_CCX) || defined(FEATURE_WLAN_LFR)
        pNeighborRoamInfo->is11rAssoc = eANI_BOOLEAN_FALSE;
        pNeighborRoamInfo->FTRoamInfo.preAuthRspWaitTimerInfo.pMac = pMac;
        pNeighborRoamInfo->FTRoamInfo.preAuthRspWaitTimerInfo.sessionId = 
                CSR_SESSION_ID_INVALID;
        /*                          */
        csrNeighborRoamPurgePreauthFailedList(pMac);
#endif

        pNeighborRoamInfo->FTRoamInfo.preauthRspPending = eANI_BOOLEAN_FALSE;
        pNeighborRoamInfo->FTRoamInfo.numPreAuthRetries = 0;
#ifdef WLAN_FEATURE_VOWIFI_11R
    /*                                           */
    pNeighborRoamInfo->FTRoamInfo.currentNeighborRptRetryNum = 0;
    pNeighborRoamInfo->FTRoamInfo.neighborRptPending = eANI_BOOLEAN_FALSE;
    pNeighborRoamInfo->FTRoamInfo.numBssFromNeighborReport = 0;
    vos_mem_zero(pNeighborRoamInfo->FTRoamInfo.neighboReportBssInfo, sizeof(tCsrNeighborReportBssInfo) * MAX_BSS_IN_NEIGHBOR_RPT);
    palTimerStop(pMac->hHdd, pNeighborRoamInfo->FTRoamInfo.preAuthRspWaitTimer);
#endif    
}

static void csrNeighborRoamDeregAllRssiIndication(tpAniSirGlobal pMac)
{
    tpCsrNeighborRoamControlInfo pNeighborRoamInfo = &pMac->roam.neighborRoamInfo;
    VOS_STATUS                    vosStatus = VOS_STATUS_SUCCESS;

        NEIGHBOR_ROAM_DEBUG(pMac, LOG2, 
                        FL("Deregister neighbor lookup UP callback with TL. RSSI = %d"), 
                        pNeighborRoamInfo->cfgParams.neighborLookupThreshold * (-1));

        /*                                                   */
        vosStatus = WLANTL_DeregRSSIIndicationCB(pMac->roam.gVosContext, 
                        (v_S7_t)pNeighborRoamInfo->cfgParams.neighborLookupThreshold * (-1),
                        WLANTL_HO_THRESHOLD_UP, 
                        csrNeighborRoamNeighborLookupUPCallback,
                        VOS_MODULE_ID_SME);
    
        if(!VOS_IS_STATUS_SUCCESS(vosStatus))
        {
                smsLog(pMac, LOGW, 
                                FL("Couldn't deregister csrNeighborRoamNeighborLookupUPCallback "
                                        "with TL: Status = %d\n"), vosStatus);
        }

        NEIGHBOR_ROAM_DEBUG(pMac, LOG2, 
                        FL("Deregistering reassoc DOWN callback with TL. RSSI = %d"), 
                        pNeighborRoamInfo->cfgParams.neighborReassocThreshold * (-1));

    /*                                                   */
        vosStatus = WLANTL_DeregRSSIIndicationCB(pMac->roam.gVosContext, 
                        (v_S7_t)pNeighborRoamInfo->cfgParams.neighborReassocThreshold * (-1),
                                                        WLANTL_HO_THRESHOLD_DOWN, 
                                                        csrNeighborRoamReassocIndCallback,
                                                        VOS_MODULE_ID_SME);
                        
    if(!VOS_IS_STATUS_SUCCESS(vosStatus))
    {
                smsLog(pMac, LOGW, 
                                FL(" Couldn't deregister csrNeighborRoamReassocIndCallback with "
                                        "TL: Status = %d\n"), vosStatus);
    }

        NEIGHBOR_ROAM_DEBUG(pMac, LOG2, 
                        FL("Deregistering neighborLookup DOWN callback with TL. RSSI = %d"), 
                        pNeighborRoamInfo->currentNeighborLookupThreshold * (-1));

    /*                                                           */
        vosStatus = WLANTL_DeregRSSIIndicationCB(pMac->roam.gVosContext, 
                        (v_S7_t)pNeighborRoamInfo->currentNeighborLookupThreshold * (-1),
                                                        WLANTL_HO_THRESHOLD_DOWN, 
                                                        csrNeighborRoamNeighborLookupDOWNCallback,
                                                        VOS_MODULE_ID_SME);
                        
    if(!VOS_IS_STATUS_SUCCESS(vosStatus))
    {
                smsLog(pMac, LOGW, 
                                FL(" Couldn't deregister csrNeighborRoamNeighborLookupDOWNCallback "
                                        "with TL: Status = %d\n"), vosStatus);
    }

        /*                                                              */
        pNeighborRoamInfo->currentNeighborLookupThreshold = 
                pNeighborRoamInfo->cfgParams.neighborLookupThreshold;
        pNeighborRoamInfo->currentScanResultsRefreshPeriod = 
            NEIGHBOR_SCAN_RESULTS_REFRESH_PERIOD_MIN;
}
                        
/*                                                                            

                                                     

                                                                                     
                                                                                     
                                           

                                                  

                

                                                                           */
void csrNeighborRoamResetConnectedStateControlInfo(tpAniSirGlobal pMac)
    {
    tpCsrNeighborRoamControlInfo    pNeighborRoamInfo = &pMac->roam.neighborRoamInfo;

    csrNeighborRoamResetChannelInfo(&pNeighborRoamInfo->roamChannelInfo);
    csrNeighborRoamFreeRoamableBSSList(pMac, &pNeighborRoamInfo->roamableAPList);
    
 /*                                          */
    palTimerStop(pMac->hHdd, pNeighborRoamInfo->neighborResultsRefreshTimer);    

#ifdef WLAN_FEATURE_VOWIFI_11R
    /*                                           */
    pNeighborRoamInfo->FTRoamInfo.currentNeighborRptRetryNum = 0;
    pNeighborRoamInfo->FTRoamInfo.neighborRptPending = eANI_BOOLEAN_FALSE;
    pNeighborRoamInfo->FTRoamInfo.numPreAuthRetries = 0;
    pNeighborRoamInfo->FTRoamInfo.numBssFromNeighborReport = 0;
    pNeighborRoamInfo->FTRoamInfo.preauthRspPending = 0;
    vos_mem_zero(pNeighborRoamInfo->FTRoamInfo.neighboReportBssInfo, sizeof(tCsrNeighborReportBssInfo) * MAX_BSS_IN_NEIGHBOR_RPT);
    palTimerStop(pMac->hHdd, pNeighborRoamInfo->FTRoamInfo.preAuthRspWaitTimer);
#endif

    }
    
void csrNeighborRoamResetReportScanStateControlInfo(tpAniSirGlobal pMac)
{
    tpCsrNeighborRoamControlInfo pNeighborRoamInfo = &pMac->roam.neighborRoamInfo;
    pNeighborRoamInfo->csrSessionId            =   CSR_SESSION_ID_INVALID;
    vos_mem_set(pNeighborRoamInfo->currAPbssid, sizeof(tCsrBssid), 0);
    pNeighborRoamInfo->neighborScanTimerInfo.pMac = pMac;
    pNeighborRoamInfo->neighborScanTimerInfo.sessionId = CSR_SESSION_ID_INVALID;
#ifdef FEATURE_WLAN_CCX
    pNeighborRoamInfo->isCCXAssoc = eANI_BOOLEAN_FALSE;
    pNeighborRoamInfo->isVOAdmitted = eANI_BOOLEAN_FALSE;
    pNeighborRoamInfo->MinQBssLoadRequired = 0;
#endif

    /*                         */
    palTimerStop(pMac->hHdd, pNeighborRoamInfo->neighborResultsRefreshTimer);
     /*                        */
       csrNeighborRoamFreeRoamableBSSList(pMac, &pNeighborRoamInfo->roamableAPList); 
    return;
}

/*                                                                            

                                                

                                                                                     
                                                                                     
                      

                                                  

                

                                                                           */
void csrNeighborRoamResetInitStateControlInfo(tpAniSirGlobal pMac)
{
    csrNeighborRoamResetConnectedStateControlInfo(pMac);

    /*                                                                                              */
    csrNeighborRoamResetReportScanStateControlInfo(pMac);
}



#ifdef WLAN_FEATURE_VOWIFI_11R
/*                                                                            

                                      

                                                                             
                                                                            
                                                                                     
                                                                                       
                                          

                                                  
                                                               

                                                                                  
                          

                                                                           */
static eHalStatus csrNeighborRoamBssIdScanFilter(tpAniSirGlobal pMac, tCsrScanResultFilter *pScanFilter)
{
    tpCsrNeighborRoamControlInfo    pNeighborRoamInfo = &pMac->roam.neighborRoamInfo;
    tANI_U8 i = 0;

    VOS_ASSERT(pScanFilter != NULL);
    vos_mem_zero(pScanFilter, sizeof(tCsrScanResultFilter));

    pScanFilter->BSSIDs.numOfBSSIDs = pNeighborRoamInfo->FTRoamInfo.numBssFromNeighborReport;
    pScanFilter->BSSIDs.bssid = vos_mem_malloc(sizeof(tSirMacAddr) * pScanFilter->BSSIDs.numOfBSSIDs);
    if (NULL == pScanFilter->BSSIDs.bssid)
    {
        smsLog(pMac, LOGE, FL("Scan Filter BSSID mem alloc failed"));
        return eHAL_STATUS_FAILED_ALLOC;
    }

    vos_mem_zero(pScanFilter->BSSIDs.bssid, sizeof(tSirMacAddr) * pScanFilter->BSSIDs.numOfBSSIDs);

    /*                                                                         */
    for (i = 0; i < pScanFilter->BSSIDs.numOfBSSIDs; i++)
    {
        vos_mem_copy(&pScanFilter->BSSIDs.bssid[i], 
                pNeighborRoamInfo->FTRoamInfo.neighboReportBssInfo[i].neighborBssId, sizeof(tSirMacAddr));
    }

    /*                                       */
    return csrNeighborRoamPrepareScanProfileFilter(pMac, pScanFilter);
}

/*                                                                            

                                           

                                                       

                                                  

                

                                                                           */
void csrNeighborRoamPurgePreauthFailList(tpAniSirGlobal pMac)
{
    tpCsrNeighborRoamControlInfo    pNeighborRoamInfo = &pMac->roam.neighborRoamInfo;

    NEIGHBOR_ROAM_DEBUG(pMac, LOGE, FL("Purging the preauth fail list"));
    while (pNeighborRoamInfo->FTRoamInfo.preAuthFailList.numMACAddress)
    {
        vos_mem_zero(pNeighborRoamInfo->FTRoamInfo.preAuthFailList.macAddress[pNeighborRoamInfo->FTRoamInfo.preAuthFailList.numMACAddress-1],
                                    sizeof(tSirMacAddr));
        pNeighborRoamInfo->FTRoamInfo.preAuthFailList.numMACAddress--;
    }
    return;
}

/*                                                                            

                                                

                                                                       

                                                  
                                                              

                                                                         

                                                                           */
eHalStatus csrNeighborRoamAddBssIdToPreauthFailList(tpAniSirGlobal pMac, tSirMacAddr bssId)
{
    tpCsrNeighborRoamControlInfo    pNeighborRoamInfo = &pMac->roam.neighborRoamInfo;

    NEIGHBOR_ROAM_DEBUG(pMac, LOGE, FL(" Added BSSID %02x:%02x:%02x:%02x:%02x:%02x to Preauth failed list\n"), 
                        bssId[0], bssId[1], bssId[2], bssId[3], bssId[4], bssId[5]);


    if ((pNeighborRoamInfo->FTRoamInfo.preAuthFailList.numMACAddress + 1) >
            MAX_NUM_PREAUTH_FAIL_LIST_ADDRESS)
    {
        smsLog(pMac, LOGE, FL("Preauth fail list already full.. Cannot add new one"));
        return eHAL_STATUS_FAILURE;
    }
    vos_mem_copy(pNeighborRoamInfo->FTRoamInfo.preAuthFailList.macAddress[pNeighborRoamInfo->FTRoamInfo.preAuthFailList.numMACAddress],
                        bssId, sizeof(tSirMacAddr));
    pNeighborRoamInfo->FTRoamInfo.preAuthFailList.numMACAddress++;
    
    return eHAL_STATUS_SUCCESS;
}

/*                                                                            

                                         

                                                                          
                                                                               

                                                  

                                                                                

                                                                           */
tANI_BOOLEAN csrNeighborRoamIsPreauthCandidate(tpAniSirGlobal pMac, tSirMacAddr bssId)
{
    tANI_U8 i = 0;
    tpCsrNeighborRoamControlInfo    pNeighborRoamInfo = &pMac->roam.neighborRoamInfo;

    if (0 == pNeighborRoamInfo->FTRoamInfo.preAuthFailList.numMACAddress)
        return eANI_BOOLEAN_TRUE;
    
    for (i = 0; i < pNeighborRoamInfo->FTRoamInfo.preAuthFailList.numMACAddress; i++)
    {
        if (VOS_TRUE == vos_mem_compare(pNeighborRoamInfo->FTRoamInfo.preAuthFailList.macAddress[i],
                                                                        bssId, sizeof(tSirMacAddr)))
        {
            NEIGHBOR_ROAM_DEBUG(pMac, LOGE, FL("BSSID %02x:%02x:%02x:%02x:%02x:%02x already present in preauth fail list"),
                                                bssId[0], bssId[1], bssId[2], bssId[3], bssId[4], bssId[5]);
            return eANI_BOOLEAN_FALSE;
        }
    }

    return eANI_BOOLEAN_TRUE;
}

/*                                                                            

                                      

                                                                                    
                            

                                                  

                                                                         

                                                                           */
static eHalStatus csrNeighborRoamIssuePreauthReq(tpAniSirGlobal pMac)
{
    tpCsrNeighborRoamControlInfo    pNeighborRoamInfo = &pMac->roam.neighborRoamInfo;
    eHalStatus status = eHAL_STATUS_SUCCESS;
    tpCsrNeighborRoamBSSInfo    pNeighborBssNode;
    
    /*                            */
    VOS_ASSERT(pNeighborRoamInfo->FTRoamInfo.preauthRspPending == eANI_BOOLEAN_FALSE);

    /*                                  */
    /*                                                                                                    */
    /*                                                                                                 */
    /*                                                                      */
    pNeighborBssNode = csrNeighborRoamGetRoamableAPListNextEntry(pMac, &pNeighborRoamInfo->roamableAPList, NULL);

    if (NULL == pNeighborBssNode)
    {
        smsLog(pMac, LOG1, FL("Roamable AP list is empty.. "));
        return eHAL_STATUS_FAILURE;
    }
    else
    {
        status = csrRoamEnqueuePreauth(pMac, pNeighborRoamInfo->csrSessionId, pNeighborBssNode->pBssDescription,
                eCsrPerformPreauth, eANI_BOOLEAN_TRUE);
        if (eHAL_STATUS_SUCCESS != status)
        {
            smsLog(pMac, LOGE, FL("Send Preauth request to PE failed with status %d\n"), status);
            return status;
        }
    }
    
    pNeighborRoamInfo->FTRoamInfo.preauthRspPending = eANI_BOOLEAN_TRUE;

    /*                                   */
    pNeighborRoamInfo->FTRoamInfo.numPreAuthRetries++;
    
    /*                                           */
    CSR_NEIGHBOR_ROAM_STATE_TRANSITION(eCSR_NEIGHBOR_ROAM_STATE_PREAUTHENTICATING)
#if 0    
    /*                             */
    status = palTimerStart(pMac->hHdd, pNeighborRoamInfo->FTRoamInfo.preAuthRspWaitTimer, 
                   CSR_NEIGHBOR_ROAM_PREAUTH_RSP_WAIT_MULTIPLIER * pNeighborRoamInfo->cfgParams.neighborScanPeriod * PAL_TIMER_TO_MS_UNIT,
                   eANI_BOOLEAN_FALSE);
    if (eHAL_STATUS_SUCCESS != status)
    {
        smsLog(pMac, LOGE, FL("Preauth response wait timer start failed with status %d\n"), status);
        return status;
    }
#endif
    
    return status;
}

/*                                                                            

                                        

                                                             
                                                                                
                                                                             
                                                                                            
                                                
                                                             

                                                  
                                                                         

                                                                     
                                         

                                                                           */
eHalStatus csrNeighborRoamPreauthRspHandler(tpAniSirGlobal pMac, VOS_STATUS vosStatus)
{
    tpCsrNeighborRoamControlInfo    pNeighborRoamInfo = &pMac->roam.neighborRoamInfo;
    eHalStatus  status = eHAL_STATUS_SUCCESS;
    eHalStatus  preauthProcessed = eHAL_STATUS_SUCCESS;
    tpCsrNeighborRoamBSSInfo pPreauthRspNode = NULL;
    
    if (eANI_BOOLEAN_FALSE == pNeighborRoamInfo->FTRoamInfo.preauthRspPending)
    {

            /*                                               
                                                                  
                                                             
                                                                
             */
            NEIGHBOR_ROAM_DEBUG(pMac, LOGW, 
                                FL("Unexpected pre-auth response in state %d\n"), 
                                pNeighborRoamInfo->neighborRoamState);
            preauthProcessed = eHAL_STATUS_FAILURE;
            goto DEQ_PREAUTH;
    }    

    //                                     
    if ((pNeighborRoamInfo->neighborRoamState != eCSR_NEIGHBOR_ROAM_STATE_PREAUTHENTICATING) &&
        (pNeighborRoamInfo->neighborRoamState != eCSR_NEIGHBOR_ROAM_STATE_REPORT_SCAN))
    {
        NEIGHBOR_ROAM_DEBUG(pMac, LOGW, FL("Preauth response received in state %d\n"), 
                            pNeighborRoamInfo->neighborRoamState);
        preauthProcessed = eHAL_STATUS_FAILURE;
        goto DEQ_PREAUTH;
    }

    if (VOS_STATUS_E_TIMEOUT != vosStatus)
    {
#if 0
        /*                                                              */
        status = palTimerStop(pMac->hHdd, pNeighborRoamInfo->FTRoamInfo.preAuthRspWaitTimer);
#endif
        pNeighborRoamInfo->FTRoamInfo.preauthRspPending = eANI_BOOLEAN_FALSE;
    }

    if (VOS_STATUS_SUCCESS == vosStatus)
    {
        pPreauthRspNode = csrNeighborRoamGetRoamableAPListNextEntry(pMac, &pNeighborRoamInfo->roamableAPList, NULL);
    }
    if ((VOS_STATUS_SUCCESS == vosStatus) && (NULL != pPreauthRspNode))
    {
        NEIGHBOR_ROAM_DEBUG(pMac, LOGE, FL("Preauth completed successfully after %d tries\n"), pNeighborRoamInfo->FTRoamInfo.numPreAuthRetries);

        /*                                                                                            */
        csrNeighborRoamRemoveRoamableAPListEntry(pMac, &pNeighborRoamInfo->roamableAPList, pPreauthRspNode);
        csrLLInsertTail(&pNeighborRoamInfo->FTRoamInfo.preAuthDoneList, &pPreauthRspNode->List, LL_ACCESS_LOCK);

        /*                                                                   */
        CSR_NEIGHBOR_ROAM_STATE_TRANSITION(eCSR_NEIGHBOR_ROAM_STATE_PREAUTH_DONE)
        pNeighborRoamInfo->FTRoamInfo.numPreAuthRetries = 0;

        /*                                                                                               
                                                                                                       */
    }
    else
    {
        tpCsrNeighborRoamBSSInfo    pNeighborBssNode = NULL;
        tListElem                   *pEntry;

        smsLog(pMac, LOGE, FL("Preauth failed retry number %d, status = %d\n"), pNeighborRoamInfo->FTRoamInfo.numPreAuthRetries, vosStatus);
        
        /*                                                                                                                */
        if (pNeighborRoamInfo->FTRoamInfo.numPreAuthRetries >=  CSR_NEIGHBOR_ROAM_MAX_NUM_PREAUTH_RETRIES)
        {
            /*                                                                                            */
            pNeighborRoamInfo->FTRoamInfo.numPreAuthRetries = 0;

            /*                                                                                        */
            pEntry = csrLLRemoveHead(&pNeighborRoamInfo->roamableAPList, LL_ACCESS_LOCK);
            if(pEntry)
            {
                pNeighborBssNode = GET_BASE_ADDR(pEntry, tCsrNeighborRoamBSSInfo, List);
            /*                                     */
            status = csrNeighborRoamAddBssIdToPreauthFailList(pMac, pNeighborBssNode->pBssDescription->bssId);
            /*                           */
            csrNeighborRoamFreeNeighborRoamBSSNode(pMac, pNeighborBssNode);
            }
        }

        /*                                               */
        if (eHAL_STATUS_SUCCESS == csrNeighborRoamIssuePreauthReq(pMac))
        goto DEQ_PREAUTH; 

        CSR_NEIGHBOR_ROAM_STATE_TRANSITION(eCSR_NEIGHBOR_ROAM_STATE_REPORT_SCAN)

        /*                                                                                                    */
        status = palTimerStart(pMac->hHdd, pNeighborRoamInfo->neighborResultsRefreshTimer, 
                        pNeighborRoamInfo->cfgParams.neighborResultsRefreshPeriod * PAL_TIMER_TO_MS_UNIT, 
                        eANI_BOOLEAN_FALSE);
        if (eHAL_STATUS_SUCCESS != status)
        {
            smsLog(pMac, LOGE, FL("Neighbor results refresh timer start failed with status %d\n"), status);
        }
    }

DEQ_PREAUTH:
    csrRoamDequeuePreauth(pMac);
    return preauthProcessed;
}
#endif  /*                               */

/*                                                                            

                                               

                                                                                         
                                                           

                                                  
                                                                              

                                                                         

                                                                           */
eHalStatus csrNeighborRoamPrepareScanProfileFilter(tpAniSirGlobal pMac, tCsrScanResultFilter *pScanFilter)
{
    tpCsrNeighborRoamControlInfo    pNeighborRoamInfo = &pMac->roam.neighborRoamInfo;
    tANI_U8 sessionId   = (tANI_U8)pNeighborRoamInfo->csrSessionId;
    tCsrRoamConnectedProfile *pCurProfile = &pMac->roam.roamSession[sessionId].connectedProfile;
    tANI_U8 i = 0;
    
    VOS_ASSERT(pScanFilter != NULL);

    vos_mem_zero(pScanFilter, sizeof(tCsrScanResultFilter));

    /*                                        */
    pScanFilter->BSSIDs.numOfBSSIDs = 0;

    /*                                                         */
    pScanFilter->SSIDs.numOfSSIDs = 1;  
    pScanFilter->SSIDs.SSIDList = vos_mem_malloc(sizeof(tCsrSSIDInfo));
    if (NULL == pScanFilter->SSIDs.SSIDList)
    {
        smsLog(pMac, LOGE, FL("Scan Filter SSID mem alloc failed"));
        return eHAL_STATUS_FAILED_ALLOC;
    }
    pScanFilter->SSIDs.SSIDList->handoffPermitted = 1;
    pScanFilter->SSIDs.SSIDList->ssidHidden = 0;
    pScanFilter->SSIDs.SSIDList->SSID.length =  pCurProfile->SSID.length;
    vos_mem_copy((void *)pScanFilter->SSIDs.SSIDList->SSID.ssId, (void *)pCurProfile->SSID.ssId, pCurProfile->SSID.length); 

    NEIGHBOR_ROAM_DEBUG(pMac, LOGE, FL("Filtering for SSID %s from scan results.. SSID Length = %d\n"), 
                        pScanFilter->SSIDs.SSIDList->SSID.ssId, pScanFilter->SSIDs.SSIDList->SSID.length);
    pScanFilter->authType.numEntries = 1;
    pScanFilter->authType.authType[0] = pCurProfile->AuthType;

    pScanFilter->EncryptionType.numEntries = 1; //              
    pScanFilter->EncryptionType.encryptionType[0] = pCurProfile->EncryptionType;

    pScanFilter->mcEncryptionType.numEntries = 1;
    pScanFilter->mcEncryptionType.encryptionType[0] = pCurProfile->mcEncryptionType;

    pScanFilter->BSSType = pCurProfile->BSSType;

    /*                                                                        */
    pScanFilter->ChannelInfo.numOfChannels = pNeighborRoamInfo->roamChannelInfo.currentChannelListInfo.numOfChannels;
    pScanFilter->ChannelInfo.ChannelList = vos_mem_malloc(pScanFilter->ChannelInfo.numOfChannels * sizeof(tANI_U8));
    if (NULL == pScanFilter->ChannelInfo.ChannelList)
    {
        smsLog(pMac, LOGE, FL("Scan Filter Channel list mem alloc failed"));
        vos_mem_free(pScanFilter->SSIDs.SSIDList);
        pScanFilter->SSIDs.SSIDList = NULL;
        return eHAL_STATUS_FAILED_ALLOC;
    }
    for (i = 0; i < pScanFilter->ChannelInfo.numOfChannels; i++)
    {
        pScanFilter->ChannelInfo.ChannelList[i] = pNeighborRoamInfo->roamChannelInfo.currentChannelListInfo.ChannelList[i];
    }

#ifdef WLAN_FEATURE_VOWIFI_11R
    if (pNeighborRoamInfo->is11rAssoc)
    {
        /*                                                                                              */
        pScanFilter->MDID.mdiePresent = pCurProfile->MDID.mdiePresent;
        pScanFilter->MDID.mobilityDomain = pCurProfile->MDID.mobilityDomain;
    }
#endif

    return eHAL_STATUS_SUCCESS;
}

tANI_U32 csrGetCurrentAPRssi(tpAniSirGlobal pMac, tScanResultHandle *pScanResultList)
{
        tCsrScanResultInfo *pScanResult;
        tpCsrNeighborRoamControlInfo    pNeighborRoamInfo = &pMac->roam.neighborRoamInfo;
        tANI_U32 CurrAPRssi = -125; /*                                                                        
                                                                                                                   
                                                                                                                     */

        while (NULL != (pScanResult = csrScanResultGetNext(pMac, *pScanResultList)))
        {

                if (VOS_TRUE == vos_mem_compare(pScanResult->BssDescriptor.bssId,
                                                pNeighborRoamInfo->currAPbssid, sizeof(tSirMacAddr)))
                {
                        /*                                                 
                                                                              
                                                                                                          
                                                                                                              
                                                                                                                      */

                         CurrAPRssi = (int)pScanResult->BssDescriptor.rssi * (-1) ;

                } else {
                        continue;
                }
        }

        return CurrAPRssi;

}

/*                                                                            

                                         

                                                                                             
                                                                                   

                                                  
                                                                                 

                

                                                                           */

static void csrNeighborRoamProcessScanResults(tpAniSirGlobal pMac, tScanResultHandle *pScanResultList)
{
    tCsrScanResultInfo *pScanResult;
    tpCsrNeighborRoamControlInfo    pNeighborRoamInfo = &pMac->roam.neighborRoamInfo;
    tpCsrNeighborRoamBSSInfo    pBssInfo;
    tANI_U32 CurrAPRssi;
    tANI_U8 RoamRssiDiff = pMac->roam.configParam.RoamRssiDiff;

    /*                                                              
                                                                 
                                                        
                                                             
                  
                                                                   */
    CurrAPRssi = csrGetCurrentAPRssi(pMac, pScanResultList);

    /*                                                                               */
    /*                                                                                                                               */
    /*                                                                                                                
                                                                                                    */
    /*                                                                                                                                     
                                                                           */

    while (NULL != (pScanResult = csrScanResultGetNext(pMac, *pScanResultList)))
    {
        NEIGHBOR_ROAM_DEBUG(pMac, LOGE, 
            FL("Scan result: BSSID %02x:%02x:%02x:%02x:%02x:%02x (Rssi %d)"), 
            pScanResult->BssDescriptor.bssId[0],
            pScanResult->BssDescriptor.bssId[1],
            pScanResult->BssDescriptor.bssId[2],
            pScanResult->BssDescriptor.bssId[3],
            pScanResult->BssDescriptor.bssId[4],
            pScanResult->BssDescriptor.bssId[5],
            abs(pScanResult->BssDescriptor.rssi));

       if (VOS_TRUE == vos_mem_compare(pScanResult->BssDescriptor.bssId, 
                       pNeighborRoamInfo->currAPbssid, sizeof(tSirMacAddr)))
        {
            /*                                                                   */
            continue;
        }

       /*
                                                                    
        */
       if (abs(pScanResult->BssDescriptor.rssi) >
           pNeighborRoamInfo->currentNeighborLookupThreshold)
       {
           VOS_TRACE (VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_INFO,
               "%s: [INFOLOG] new ap rssi (%d) lower than lookup threshold (%d)\n",
               __func__, (int)pScanResult->BssDescriptor.rssi * (-1),
               (int)pNeighborRoamInfo->currentNeighborLookupThreshold * (-1));
           continue;
       }

       /*                                                                                                                  
                                                               */
       if(RoamRssiDiff > 0)
       {
               if (abs(CurrAPRssi) < abs(pScanResult->BssDescriptor.rssi))
               {
                       /*                                                     */
                       VOS_TRACE (VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_INFO,
                                 "%s: [INFOLOG]Current AP rssi=%d new ap rssi worse=%d\n", __func__,
                                 CurrAPRssi,
                                 (int)pScanResult->BssDescriptor.rssi * (-1) );
                       continue;
               } else {
                       /*                                                                                             
                                                                                          */
                       if (abs(abs(CurrAPRssi) - abs(pScanResult->BssDescriptor.rssi)) < RoamRssiDiff)
                       {
                          continue;
                       }
                       else {
                                 VOS_TRACE (VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_INFO,
                                            "%s: [INFOLOG]Current AP rssi=%d new ap rssi better=%d\n", __func__,
                                            CurrAPRssi,
                                            (int)pScanResult->BssDescriptor.rssi * (-1) );
                       }
               }
       }

#ifdef WLAN_FEATURE_VOWIFI_11R
        if (pNeighborRoamInfo->is11rAssoc)
        {
            if (!csrNeighborRoamIsPreauthCandidate(pMac, pScanResult->BssDescriptor.bssId))
            {
                smsLog(pMac, LOGE, FL("BSSID present in pre-auth fail list.. Ignoring"));
                continue;
            }
        }
#endif /*                         */

#ifdef FEATURE_WLAN_CCX
        if (pNeighborRoamInfo->isCCXAssoc)
        {
            if (!csrNeighborRoamIsPreauthCandidate(pMac, pScanResult->BssDescriptor.bssId))
            {
                smsLog(pMac, LOGE, FL("BSSID present in pre-auth fail list.. Ignoring"));
                continue;
            }
        }
        if ((pScanResult->BssDescriptor.QBSSLoad_present) &&
             (pScanResult->BssDescriptor.QBSSLoad_avail))
        {
            if (pNeighborRoamInfo->isVOAdmitted)
            {
                smsLog(pMac, LOG1, FL("New AP has %x BW available\n"), (unsigned int)pScanResult->BssDescriptor.QBSSLoad_avail);
                smsLog(pMac, LOG1, FL("We need %x BW available\n"),(unsigned int)pNeighborRoamInfo->MinQBssLoadRequired);
                if (pScanResult->BssDescriptor.QBSSLoad_avail < pNeighborRoamInfo->MinQBssLoadRequired) 
                {
                    VOS_TRACE (VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_INFO, 
                        "[INFOLOG]BSSID : %02x:%02x:%02x:%02x:%02x:%02x has no bandwidth ignoring..not adding to roam list\n",
                        pScanResult->BssDescriptor.bssId[0],
                        pScanResult->BssDescriptor.bssId[1],
                        pScanResult->BssDescriptor.bssId[2],
                        pScanResult->BssDescriptor.bssId[3],
                        pScanResult->BssDescriptor.bssId[4],
                        pScanResult->BssDescriptor.bssId[5]);
                    continue;
                }
            }
        }
        else
        {
            smsLog(pMac, LOGE, FL("No QBss %x %x\n"), (unsigned int)pScanResult->BssDescriptor.QBSSLoad_avail, (unsigned int)pScanResult->BssDescriptor.QBSSLoad_present);
            if (pNeighborRoamInfo->isVOAdmitted)
            {
                VOS_TRACE (VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_INFO, 
                    "[INFOLOG]BSSID : %02x:%02x:%02x:%02x:%02x:%02x has no QBSSLoad IE, ignoring..not adding to roam list\n",
                    pScanResult->BssDescriptor.bssId[0],
                    pScanResult->BssDescriptor.bssId[1],
                    pScanResult->BssDescriptor.bssId[2],
                    pScanResult->BssDescriptor.bssId[3],
                    pScanResult->BssDescriptor.bssId[4],
                    pScanResult->BssDescriptor.bssId[5]);
                continue;
            }
        }
#endif /*                  */

#ifdef FEATURE_WLAN_LFR
        //                                          
        //                                                               
        if (csrRoamIsFastRoamEnabled(pMac))
        {
            if (!csrNeighborRoamIsPreauthCandidate(pMac, pScanResult->BssDescriptor.bssId))
            {
                smsLog(pMac, LOGE, FL("BSSID present in pre-auth fail list.. Ignoring"));
                continue;
            }
        }
#endif /*                  */

        /*                                                                                               
                       */
        if (pNeighborRoamInfo->scanRequestTimeStamp >= pScanResult->BssDescriptor.nReceivedTime)
        {
            smsLog(pMac, LOGE, FL("Ignoring BSS as it is older than the scan request timestamp"));
            continue;
        }

        pBssInfo = vos_mem_malloc(sizeof(tCsrNeighborRoamBSSInfo));
        if (NULL == pBssInfo)
        {
            smsLog(pMac, LOGE, FL("Memory allocation for Neighbor Roam BSS Info failed.. Just ignoring"));
            continue;
        }

        pBssInfo->pBssDescription = vos_mem_malloc(pScanResult->BssDescriptor.length + sizeof(pScanResult->BssDescriptor.length));
        if (pBssInfo->pBssDescription != NULL)
        {
            vos_mem_copy(pBssInfo->pBssDescription, &pScanResult->BssDescriptor, 
                    pScanResult->BssDescriptor.length + sizeof(pScanResult->BssDescriptor.length));
        }
        else
        {
            smsLog(pMac, LOGE, FL("Memory allocation for Neighbor Roam BSS Descriptor failed.. Just ignoring"));
            vos_mem_free(pBssInfo);
            continue;
            
        }
        pBssInfo->apPreferenceVal = 10; //                                                                                          

        /*                                                                 */
        csrLLInsertTail(&pNeighborRoamInfo->roamableAPList, &pBssInfo->List, LL_ACCESS_LOCK);
    }

    /*                                                                                                                            */
    csrScanResultPurge(pMac, *pScanResultList);

    return;
}

/*                                                                            

                                            

                                                                               
                                                                                  
                                                                                   
                                                               
                                                                         
                                                
                                                             
                                                                             

                                                  

                                                                             

                                                                           */
static VOS_STATUS csrNeighborRoamHandleEmptyScanResult(tpAniSirGlobal pMac)
{
    tpCsrNeighborRoamControlInfo    pNeighborRoamInfo = &pMac->roam.neighborRoamInfo;
    eHalStatus  status = eHAL_STATUS_SUCCESS;

    /*                          */
    status = palTimerStop(pMac->hHdd, pNeighborRoamInfo->neighborScanTimer);
    if (eHAL_STATUS_SUCCESS != status) 
    {
        smsLog(pMac, LOGW, FL(" palTimerStop failed with status %d\n"), status);
    }

#ifdef WLAN_FEATURE_VOWIFI_11R
    /*                                           */
    vos_mem_zero(&pNeighborRoamInfo->FTRoamInfo.neighboReportBssInfo, sizeof(tCsrNeighborReportBssInfo) * MAX_BSS_IN_NEIGHBOR_RPT);
#endif

    /*                                           */
    if (eHAL_STATUS_SUCCESS != 
            palTimerStart(pMac->hHdd, pNeighborRoamInfo->neighborResultsRefreshTimer, 
            pNeighborRoamInfo->currentScanResultsRefreshPeriod * PAL_TIMER_TO_MS_UNIT, 
            eANI_BOOLEAN_FALSE)) 
    {
        smsLog(pMac, LOGE, FL("Neighbor results refresh timer failed to start (%d)"), status);
        vos_mem_free(pNeighborRoamInfo->roamChannelInfo.currentChannelListInfo.ChannelList);
        pNeighborRoamInfo->roamChannelInfo.currentChannelListInfo.ChannelList = NULL;
        return VOS_STATUS_E_FAILURE;
    }
    smsLog(pMac, LOG1, FL("Neighbor results refresh timer started (%ld ms)"), 
            (pNeighborRoamInfo->currentScanResultsRefreshPeriod * PAL_TIMER_TO_MS_UNIT));

    /*                                                                      */
    if ( (2*pNeighborRoamInfo->currentScanResultsRefreshPeriod) >
            NEIGHBOR_SCAN_RESULTS_REFRESH_PERIOD_MAX ) 
    {
        pNeighborRoamInfo->currentScanResultsRefreshPeriod = 
            NEIGHBOR_SCAN_RESULTS_REFRESH_PERIOD_MAX;
    } else {
        /*                                                                 */
        pNeighborRoamInfo->currentScanResultsRefreshPeriod *= 2;
    }

    return VOS_STATUS_SUCCESS;
}

/*                                                                            

                                          

                                                                                     
                                                                                          
                                                                                               
                                                                                              
                                                                                        
                   

                                                       
                               
                             
                             

                                                                              

                                                                           */
static eHalStatus csrNeighborRoamScanRequestCallback(tHalHandle halHandle, void *pContext,
                         tANI_U32 scanId, eCsrScanStatus status)
{
    tpAniSirGlobal                  pMac = (tpAniSirGlobal) halHandle;
    tpCsrNeighborRoamControlInfo    pNeighborRoamInfo = &pMac->roam.neighborRoamInfo;
    tANI_U8                         currentChanIndex;
    tCsrScanResultFilter    scanFilter;
    tScanResultHandle       scanResult;
    tANI_U32                tempVal = 0;
    eHalStatus              hstatus;

    pMac->roam.neighborRoamInfo.scanRspPending = eANI_BOOLEAN_FALSE;
    
    /*                                                                                                  */
    if (eCSR_NEIGHBOR_ROAM_STATE_CONNECTED == pNeighborRoamInfo->neighborRoamState)
    {
        smsLog(pMac, LOGE, FL("Received in CONNECTED state. Must be because a UP event from TL after issuing scan request. Ignore it"));
        return eHAL_STATUS_SUCCESS;
    }

    /*                                                                                                     */
    currentChanIndex = (pMac->roam.neighborRoamInfo.roamChannelInfo.currentChanIndex) ? (pMac->roam.neighborRoamInfo.roamChannelInfo.currentChanIndex - 1) : 0;

    /*                 */
    if (pMac->roam.neighborRoamInfo.roamChannelInfo.currentChannelListInfo.ChannelList) { 
        NEIGHBOR_ROAM_DEBUG(pMac, LOGW, FL("csrNeighborRoamScanRequestCallback received for Channel = %d, ChanIndex = %d"), 
                    pMac->roam.neighborRoamInfo.roamChannelInfo.currentChannelListInfo.ChannelList[currentChanIndex], currentChanIndex);
    }
    else
    {
        smsLog(pMac, LOG1, FL("Received during clean-up. Silently ignore scan completion event."));
        return eHAL_STATUS_SUCCESS;
    }

    if (eANI_BOOLEAN_FALSE == pNeighborRoamInfo->roamChannelInfo.chanListScanInProgress)
    {
        /*                                                                                       
                                                       */
        /*                                                                                                       
                                                                                                          */
        NEIGHBOR_ROAM_DEBUG(pMac, LOGW, FL("Channel list scan completed. Current chan index = %d"), currentChanIndex);
        VOS_ASSERT(pNeighborRoamInfo->roamChannelInfo.currentChanIndex == 0);

#if  defined (WLAN_FEATURE_VOWIFI_11R) || defined (FEATURE_WLAN_CCX) || defined(FEATURE_WLAN_LFR)
        /*                                                                                              
                                                                    */
        if (eCSR_NEIGHBOR_ROAM_STATE_REPORT_SCAN == pNeighborRoamInfo->neighborRoamState)
        {
            hstatus = csrNeighborRoamBssIdScanFilter(pMac, &scanFilter);
            NEIGHBOR_ROAM_DEBUG(pMac, LOGW, FL("11R or CCX Association: Prepare scan filter status  with neighbor AP = %d"), hstatus);
            tempVal = 1;
        }
        else
#endif
        {
            hstatus = csrNeighborRoamPrepareScanProfileFilter(pMac, &scanFilter);
            NEIGHBOR_ROAM_DEBUG(pMac, LOGW, FL("11R/CCX/Other Association: Prepare scan to find neighbor AP filter status  = %d"), hstatus);
        }
        if (eHAL_STATUS_SUCCESS != hstatus)
        {
            smsLog(pMac, LOGE, FL("Scan Filter preparation failed for Assoc type %d.. Bailing out.."), tempVal);
            return eHAL_STATUS_FAILURE;
        }
        hstatus = csrScanGetResult(pMac, &scanFilter, &scanResult);
        NEIGHBOR_ROAM_DEBUG(pMac, LOGE, FL("Get Scan Result status code %d"), hstatus);
        /*                                                      */
        csrNeighborRoamProcessScanResults(pMac, &scanResult);

        /*                      */
        csrFreeScanFilter(pMac, &scanFilter);

        tempVal = csrLLCount(&pNeighborRoamInfo->roamableAPList);

        switch(pNeighborRoamInfo->neighborRoamState)
        {
            case eCSR_NEIGHBOR_ROAM_STATE_CFG_CHAN_LIST_SCAN:
                if (tempVal)
                {
                    /*
                                                                
                                                               
                                  
                     */
                    pNeighborRoamInfo->currentScanResultsRefreshPeriod = 
                        NEIGHBOR_SCAN_RESULTS_REFRESH_PERIOD_MIN;
#ifdef WLAN_FEATURE_VOWIFI_11R
                    /*                                                                                                  
                                                                    */
                    if (pNeighborRoamInfo->is11rAssoc)
                    {
                        /*                                                                        */
                        CSR_NEIGHBOR_ROAM_STATE_TRANSITION(eCSR_NEIGHBOR_ROAM_STATE_REPORT_SCAN)
                    }
                    else
#endif
#ifdef FEATURE_WLAN_CCX
                    /*                                                                                                  
                                                                    */
                    if (pNeighborRoamInfo->isCCXAssoc)
                    {
                        /*                                                                        */
                        CSR_NEIGHBOR_ROAM_STATE_TRANSITION(eCSR_NEIGHBOR_ROAM_STATE_REPORT_SCAN)
                    }
                    else
#endif
#ifdef FEATURE_WLAN_LFR
                    /*                                                                                   
                                                                    */
                    if (csrRoamIsFastRoamEnabled(pMac))
                    {
                        /*                                                                        */
                        CSR_NEIGHBOR_ROAM_STATE_TRANSITION(eCSR_NEIGHBOR_ROAM_STATE_REPORT_SCAN)
                    }
                    else
#endif
                    {
                       
                        NEIGHBOR_ROAM_DEBUG(pMac, LOGE, FL("Completed scanning of CFG CHAN LIST in non-11r association. Registering reassoc callback"));
                        /*                                                                                              */
                        /*                                                                                                    
                                                                                                                       
                                                   */
                        
//                                                                                       
                    }
                }
                else
                {
                    NEIGHBOR_ROAM_DEBUG(pMac, LOGE, FL("No candidate found after scanning in state %d.. "), pNeighborRoamInfo->neighborRoamState);
                    /*                         */
                    csrNeighborRoamHandleEmptyScanResult(pMac);
                }
                break;
#ifdef WLAN_FEATURE_VOWIFI_11R                
            case eCSR_NEIGHBOR_ROAM_STATE_REPORT_SCAN:
                if (!tempVal)
                {
                    smsLog(pMac, LOGE, FL("No candidate found after scanning in state %d.. "), pNeighborRoamInfo->neighborRoamState);
                    /*                                                                                    */
                    csrNeighborRoamTransitToCFGChanScan(pMac);
                }
                break;
#endif /*                         */
            default:
                //                                                                               
                //                                                                                      
                //                                                                            
                VOS_TRACE (VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_INFO, 
                        "%s: [INFOLOG] State %d\n", __func__, (pNeighborRoamInfo->neighborRoamState));

                //                             
                return eHAL_STATUS_SUCCESS;
        }

        if (tempVal)
        {
            VOS_STATUS  vosStatus = VOS_STATUS_SUCCESS;

           /*                                                                                                                  
                                                                                                                             */
           if (eHAL_STATUS_SUCCESS != palTimerStart(pMac->hHdd, pNeighborRoamInfo->neighborResultsRefreshTimer, 
                        pNeighborRoamInfo->cfgParams.neighborResultsRefreshPeriod * PAL_TIMER_TO_MS_UNIT, 
                        eANI_BOOLEAN_FALSE))
            {
                smsLog(pMac, LOGE, FL("Neighbor results refresh timer failed to start, status = %d"), status);
                vos_mem_free(pNeighborRoamInfo->roamChannelInfo.currentChannelListInfo.ChannelList);
                pNeighborRoamInfo->roamChannelInfo.currentChannelListInfo.ChannelList = NULL;
                return eHAL_STATUS_FAILURE;
            }

            NEIGHBOR_ROAM_DEBUG(pMac, LOG2, FL("Registering DOWN event Reassoc callback with TL. RSSI = %d"), pNeighborRoamInfo->cfgParams.neighborReassocThreshold * (-1));
            /*                                        */
            vosStatus = WLANTL_RegRSSIIndicationCB(pMac->roam.gVosContext, (v_S7_t)pNeighborRoamInfo->cfgParams.neighborReassocThreshold * (-1),
                                            WLANTL_HO_THRESHOLD_DOWN, 
                                            csrNeighborRoamReassocIndCallback,
                                            VOS_MODULE_ID_SME, pMac);
            
            if(!VOS_IS_STATUS_SUCCESS(vosStatus))
            {
               //       
               smsLog(pMac, LOGW, FL(" Couldn't register csrNeighborRoamReassocIndCallback with TL: Status = %d\n"), vosStatus);
            }
 
        }
    }
    else
    {

        /*                                                                      */
        hstatus = palTimerStart(pMac->hHdd, pNeighborRoamInfo->neighborScanTimer, 
                    pNeighborRoamInfo->cfgParams.neighborScanPeriod * PAL_TIMER_TO_MS_UNIT, 
                    eANI_BOOLEAN_FALSE);
    
        if (eHAL_STATUS_SUCCESS != hstatus)
        {
            /*                                               */
            smsLog(pMac, LOGE, FL("Neighbor scan PAL Timer start failed, status = %d, Ignoring state transition"), status);
            vos_mem_free(pNeighborRoamInfo->roamChannelInfo.currentChannelListInfo.ChannelList);
            pNeighborRoamInfo->roamChannelInfo.currentChannelListInfo.ChannelList = NULL;
            return eHAL_STATUS_FAILURE;
        }
    }
    return eHAL_STATUS_SUCCESS;
}

/*                                                                            

                                         

                                                                                          
                  

                                                  
                                                                                  

                                                                              

                                                                           */
eHalStatus csrNeighborRoamIssueBgScanRequest(tpAniSirGlobal pMac, tCsrBGScanRequest *pBgScanParams)
{
    eHalStatus status = eHAL_STATUS_SUCCESS;
    tANI_U32 scanId;
    tCsrScanRequest scanReq;
    tANI_U8 channel;
    
    NEIGHBOR_ROAM_DEBUG(pMac, LOGE, FL("csrNeighborRoamIssueBgScanRequest for Channel = %d, ChanIndex = %d"), 
                    pBgScanParams->ChannelInfo.ChannelList[0], pMac->roam.neighborRoamInfo.roamChannelInfo.currentChanIndex);


    //                                                           
    palZeroMemory(pMac->hHdd, &scanReq, sizeof(tCsrScanRequest));
    /*                       */
    scanReq.SSIDs.numOfSSIDs = 1;
    scanReq.SSIDs.SSIDList = vos_mem_malloc(sizeof(tCsrSSIDInfo) * scanReq.SSIDs.numOfSSIDs);
    if(NULL == scanReq.SSIDs.SSIDList)
    {
       //       
       smsLog(pMac, LOGW, FL("Couldn't allocate memory for the SSID..Freeing memory allocated for Channel List\n"));
       return eHAL_STATUS_FAILURE;
    }
    vos_mem_zero(scanReq.SSIDs.SSIDList, sizeof(tCsrSSIDInfo) * scanReq.SSIDs.numOfSSIDs);

    scanReq.SSIDs.SSIDList[0].handoffPermitted = eANI_BOOLEAN_TRUE;
    scanReq.SSIDs.SSIDList[0].ssidHidden = eANI_BOOLEAN_TRUE;
    vos_mem_copy((void *)&scanReq.SSIDs.SSIDList[0].SSID, (void *)&pBgScanParams->SSID, sizeof(pBgScanParams->SSID));
    
    scanReq.ChannelInfo.numOfChannels = pBgScanParams->ChannelInfo.numOfChannels;
    
    channel = pBgScanParams->ChannelInfo.ChannelList[0];
    scanReq.ChannelInfo.ChannelList = &channel;    

    scanReq.BSSType = eCSR_BSS_TYPE_INFRASTRUCTURE;
    scanReq.scanType = eSIR_ACTIVE_SCAN;
    scanReq.requestType = eCSR_SCAN_HO_BG_SCAN;
    scanReq.maxChnTime = pBgScanParams->maxChnTime;
    scanReq.minChnTime = pBgScanParams->minChnTime;
    status = csrScanRequest(pMac, CSR_SESSION_ID_INVALID, &scanReq,
                        &scanId, csrNeighborRoamScanRequestCallback, NULL);
    if (eHAL_STATUS_SUCCESS != status)
    {
        smsLog(pMac, LOGE, FL("CSR Scan Request failed with status %d"), status);
        vos_mem_free(scanReq.SSIDs.SSIDList);
        return status;
    }
    pMac->roam.neighborRoamInfo.scanRspPending = eANI_BOOLEAN_TRUE;

    vos_mem_free(scanReq.SSIDs.SSIDList);
    NEIGHBOR_ROAM_DEBUG(pMac, LOGE, FL("Channel List Address = %08x, Actual index = %d"), 
                    &pMac->roam.neighborRoamInfo.roamChannelInfo.currentChannelListInfo.ChannelList[0], 
                    pMac->roam.neighborRoamInfo.roamChannelInfo.currentChanIndex);
    return status;
}

/*                                                                            

                                    

                                                                                   
                                                                                 
                                                                                   
                   

                                                  

                                                                              

                                                                           */
eHalStatus csrNeighborRoamPerformBgScan(tpAniSirGlobal pMac)
{
    eHalStatus      status = eHAL_STATUS_SUCCESS;
    tpCsrNeighborRoamControlInfo    pNeighborRoamInfo = &pMac->roam.neighborRoamInfo;
    tCsrBGScanRequest   bgScanParams;
    tANI_U8             broadcastBssid[] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };
    tANI_U8             channel = 0;

    if (pNeighborRoamInfo->roamChannelInfo.currentChannelListInfo.ChannelList)
    {
        NEIGHBOR_ROAM_DEBUG(pMac, LOGE, FL("Channel List Address = %08x"), &pNeighborRoamInfo->roamChannelInfo.currentChannelListInfo.ChannelList[0]);
    }
    else 
    {
        NEIGHBOR_ROAM_DEBUG(pMac, LOGE, FL("Channel List Empty"));
        //                                                             
        //                                                                 
        //                                                                      
        //                                
        csrNeighborRoamHandleEmptyScanResult(pMac);
        return status;
    }
    /*                                                   */
    vos_mem_copy(bgScanParams.bssid, broadcastBssid, sizeof(tCsrBssid));
    bgScanParams.SSID.length = pMac->roam.roamSession[pNeighborRoamInfo->csrSessionId].connectedProfile.SSID.length;
    vos_mem_copy(bgScanParams.SSID.ssId, pMac->roam.roamSession[pNeighborRoamInfo->csrSessionId].connectedProfile.SSID.ssId, 
                                    pMac->roam.roamSession[pNeighborRoamInfo->csrSessionId].connectedProfile.SSID.length);

    channel = pNeighborRoamInfo->roamChannelInfo.currentChannelListInfo.ChannelList[pNeighborRoamInfo->roamChannelInfo.currentChanIndex];
    bgScanParams.ChannelInfo.numOfChannels = 1;
    bgScanParams.ChannelInfo.ChannelList = &channel;
   
    bgScanParams.minChnTime = pNeighborRoamInfo->cfgParams.minChannelScanTime;
    bgScanParams.maxChnTime = pNeighborRoamInfo->cfgParams.maxChannelScanTime;

    status = csrNeighborRoamIssueBgScanRequest(pMac, &bgScanParams);
    if (eHAL_STATUS_SUCCESS != status)
    {
        smsLog(pMac, LOGE, FL("Issue of BG Scan request failed: Status = %d"), status);
        return status;
    }

    pNeighborRoamInfo->roamChannelInfo.currentChanIndex++;
    if (pNeighborRoamInfo->roamChannelInfo.currentChanIndex >= 
            pNeighborRoamInfo->roamChannelInfo.currentChannelListInfo.numOfChannels)
    {      
        NEIGHBOR_ROAM_DEBUG(pMac, LOGE, FL("Completed scanning channels in Channel List: CurrChanIndex = %d, Num Channels = %d"),
                                            pNeighborRoamInfo->roamChannelInfo.currentChanIndex, 
                                            pNeighborRoamInfo->roamChannelInfo.currentChannelListInfo.numOfChannels);
        /*                                             */
        pNeighborRoamInfo->roamChannelInfo.currentChanIndex = 0;
        /*                                                                                                      
                                              */
        if (eANI_BOOLEAN_TRUE == pNeighborRoamInfo->roamChannelInfo.chanListScanInProgress)
        {
            pNeighborRoamInfo->roamChannelInfo.chanListScanInProgress = eANI_BOOLEAN_FALSE;
        }
    }

    return status;
}

/*                                                                            

                                                

                                                                                   
                                                                        

                                                                          

                

                                                                           */
void csrNeighborRoamNeighborScanTimerCallback(void *pv)
{
    tCsrTimerInfo *pInfo = (tCsrTimerInfo *)pv;
    tpAniSirGlobal pMac = pInfo->pMac;
    tANI_U32         sessionId = pInfo->sessionId;
    tpCsrNeighborRoamControlInfo    pNeighborRoamInfo = &pMac->roam.neighborRoamInfo;

    //                                                                          
    if(eANI_BOOLEAN_TRUE == pNeighborRoamInfo->scanRspPending)
    {
       //   
       smsLog(pMac, LOGW, FL("Already BgScanRsp is Pending\n"));
       return;
    }

    VOS_ASSERT(sessionId == pNeighborRoamInfo->csrSessionId);

    switch (pNeighborRoamInfo->neighborRoamState)
    {
#ifdef WLAN_FEATURE_VOWIFI_11R
        case eCSR_NEIGHBOR_ROAM_STATE_REPORT_SCAN:
            switch(pNeighborRoamInfo->prevNeighborRoamState)
            {
                case eCSR_NEIGHBOR_ROAM_STATE_REPORT_QUERY:
                    csrNeighborRoamPerformBgScan(pMac);
                    break;
                default:
                    smsLog(pMac, LOGE, FL("Neighbor scan callback received in state %d, prev state = %d"), 
                                    pNeighborRoamInfo->neighborRoamState, pNeighborRoamInfo->prevNeighborRoamState);
                    break;
            }
            break;
#endif /*                         */
        case eCSR_NEIGHBOR_ROAM_STATE_CFG_CHAN_LIST_SCAN:     
            csrNeighborRoamPerformBgScan(pMac);
            break;
        default:
            break;
    }
    return;
}

/*                                                                            

                                                  

                                                                                   
                                                                                    
                                                                                     
                                  

                                                                               

                

                                                                           */
void csrNeighborRoamResultsRefreshTimerCallback(void *context)
{
    tCsrTimerInfo *pInfo = (tCsrTimerInfo *)context;
    tpAniSirGlobal pMac = pInfo->pMac;
    VOS_STATUS     vosStatus = VOS_STATUS_SUCCESS;
    tpCsrNeighborRoamControlInfo    pNeighborRoamInfo = &pMac->roam.neighborRoamInfo;
     
    NEIGHBOR_ROAM_DEBUG(pMac, LOG2, FL("Deregistering DOWN event reassoc callback with TL. RSSI = %d"), pNeighborRoamInfo->cfgParams.neighborReassocThreshold * (-1));

    /*                                                   */
    vosStatus = WLANTL_DeregRSSIIndicationCB(pMac->roam.gVosContext, (v_S7_t)pNeighborRoamInfo->cfgParams.neighborReassocThreshold * (-1),
                                                        WLANTL_HO_THRESHOLD_DOWN, 
                                                        csrNeighborRoamReassocIndCallback,
                                                        VOS_MODULE_ID_SME);
                        
    if(!VOS_IS_STATUS_SUCCESS(vosStatus))
    {
        //       
        smsLog(pMac, LOGW, FL(" Couldn't deregister csrNeighborRoamReassocIndCallback with TL: Status = %d\n"), vosStatus);
    }

    /*                                                             */
    csrNeighborRoamResetConnectedStateControlInfo(pMac);

#if defined WLAN_FEATURE_VOWIFI_11R && defined WLAN_FEATURE_VOWIFI
    if ((pNeighborRoamInfo->is11rAssoc) && (pMac->rrm.rrmSmeContext.rrmConfig.rrmEnabled))
    {
        NEIGHBOR_ROAM_DEBUG(pMac, LOGE, FL("11R Association:Neighbor Lookup Down event received in CONNECTED state"));
        vosStatus = csrNeighborRoamIssueNeighborRptRequest(pMac);
        if (VOS_STATUS_SUCCESS != vosStatus)
        {
            smsLog(pMac, LOGE, FL("Neighbor report request failed. status = %d\n"), vosStatus);
            return;
        }
        /*                                                                                           */
        pNeighborRoamInfo->FTRoamInfo.currentNeighborRptRetryNum++;
        pNeighborRoamInfo->FTRoamInfo.neighborRptPending = eANI_BOOLEAN_TRUE;
        CSR_NEIGHBOR_ROAM_STATE_TRANSITION(eCSR_NEIGHBOR_ROAM_STATE_REPORT_QUERY)
    }
    else
#endif      
    {
        NEIGHBOR_ROAM_DEBUG(pMac, LOGE, FL("Non 11R or CCX Association:Neighbor Lookup Down event received in CONNECTED state"));
        vosStatus = csrNeighborRoamTransitToCFGChanScan(pMac);
        if (VOS_STATUS_SUCCESS != vosStatus)
        {
            return;
        }
    }
    return;
}

#if defined WLAN_FEATURE_VOWIFI_11R && defined WLAN_FEATURE_VOWIFI
/*                                                                            

                                              

                                                                                       
                                                                                          
                                                             

                                                  

                                                                             

                                                                           */
VOS_STATUS csrNeighborRoamIssueNeighborRptRequest(tpAniSirGlobal pMac)
{
    tRrmNeighborRspCallbackInfo callbackInfo;
    tpCsrNeighborRoamControlInfo    pNeighborRoamInfo = &pMac->roam.neighborRoamInfo;
    tRrmNeighborReq neighborReq;


    neighborReq.no_ssid = 0;

    /*                  */
    neighborReq.ssid.length = pMac->roam.roamSession[pNeighborRoamInfo->csrSessionId].connectedProfile.SSID.length;
    vos_mem_copy(neighborReq.ssid.ssId, pMac->roam.roamSession[pNeighborRoamInfo->csrSessionId].connectedProfile.SSID.ssId, 
                                    pMac->roam.roamSession[pNeighborRoamInfo->csrSessionId].connectedProfile.SSID.length);
    
    callbackInfo.neighborRspCallback = csrNeighborRoamRRMNeighborReportResult;
    callbackInfo.neighborRspCallbackContext = pMac;
    callbackInfo.timeout = pNeighborRoamInfo->FTRoamInfo.neighborReportTimeout;

    return sme_NeighborReportRequest(pMac,(tANI_U8) pNeighborRoamInfo->csrSessionId, &neighborReq, &callbackInfo);
}

/*                                                                            

                                         

                                                            
                                                                      
                                                                                

                                                  
                                                                                                 
                                                                   
                                                                            
                                                                                                
                                                                                                   

                                                                             

                                                                           */
VOS_STATUS csrNeighborRoamMergeChannelLists( 
        tpAniSirGlobal pMac, 
        tANI_U8   *pInputChannelList, 
        int inputNumOfChannels,
        tANI_U8   *pOutputChannelList,
        int outputNumOfChannels,
        int *pMergedOutputNumOfChannels 
        )
{
    int i = 0;
    int j = 0;
    int numChannels = outputNumOfChannels;

    //                       
    if (!pInputChannelList) return eHAL_STATUS_E_NULL_VALUE;

    //                       
    if (!pOutputChannelList) return eHAL_STATUS_E_NULL_VALUE;

    //                                                                        
    for (i = 0; i < inputNumOfChannels; i++)
    {
        for (j = 0; j < outputNumOfChannels; j++)
        {
            if (pInputChannelList[i] == pOutputChannelList[j])
                break;
        }
        if (j == outputNumOfChannels)
        {
            if (pInputChannelList[i])
            {
                VOS_TRACE (VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_ERROR, 
                        "%s: [INFOLOG] Adding extra %d to Neighbor channel list\n", __func__, 
                        pInputChannelList[i]); 
                pOutputChannelList[numChannels] = pInputChannelList[i]; 
                numChannels++; 
            }
        }
    }

    //                                
    *pMergedOutputNumOfChannels = numChannels; 

    return eHAL_STATUS_SUCCESS;
}

/*                                                                            

                                                       

                                                                              
                                                                                    
                                                                                
                                       

                                                  

                                                                             

                                                                           */
VOS_STATUS csrNeighborRoamCreateChanListFromNeighborReport(tpAniSirGlobal pMac)
{
    tpRrmNeighborReportDesc pNeighborBssDesc;
    tpCsrNeighborRoamControlInfo    pNeighborRoamInfo = &pMac->roam.neighborRoamInfo;
    tANI_U8         numChannels = 0, i = 0;
    tANI_U8         channelList[MAX_BSS_IN_NEIGHBOR_RPT];
#if 0
    eHalStatus  status = eHAL_STATUS_SUCCESS;
#endif

    /*                                                                                           */
    pNeighborRoamInfo->FTRoamInfo.numBssFromNeighborReport = 0;

    pNeighborBssDesc = smeRrmGetFirstBssEntryFromNeighborCache(pMac);

    while (pNeighborBssDesc)
    {
        if (pNeighborRoamInfo->FTRoamInfo.numBssFromNeighborReport >= MAX_BSS_IN_NEIGHBOR_RPT) break;
        
        /*                                                      */
        pNeighborRoamInfo->FTRoamInfo.neighboReportBssInfo[pNeighborRoamInfo->FTRoamInfo.numBssFromNeighborReport].channelNum = 
                                    pNeighborBssDesc->pNeighborBssDescription->channel;
        pNeighborRoamInfo->FTRoamInfo.neighboReportBssInfo[pNeighborRoamInfo->FTRoamInfo.numBssFromNeighborReport].neighborScore = 
                                    (tANI_U8)pNeighborBssDesc->roamScore;
        vos_mem_copy(pNeighborRoamInfo->FTRoamInfo.neighboReportBssInfo[pNeighborRoamInfo->FTRoamInfo.numBssFromNeighborReport].neighborBssId,
                                     pNeighborBssDesc->pNeighborBssDescription->bssId, sizeof(tSirMacAddr));
        pNeighborRoamInfo->FTRoamInfo.numBssFromNeighborReport++;

        /*                                         */
        if (numChannels > 0)
        {
            for (i = 0; i < numChannels; i++)
            {
                if (pNeighborBssDesc->pNeighborBssDescription->channel == channelList[i])
                    break;
            }
            
        }
        if (i == numChannels)
        {
            if (pNeighborBssDesc->pNeighborBssDescription->channel)
            {
                        VOS_TRACE (VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_INFO, 
                                "%s: [INFOLOG] Adding %d to Neighbor channel list\n", __func__,
                                pNeighborBssDesc->pNeighborBssDescription->channel);
                        channelList[numChannels] = pNeighborBssDesc->pNeighborBssDescription->channel;
                        numChannels++;
            }
        }
            
        pNeighborBssDesc = smeRrmGetNextBssEntryFromNeighborCache(pMac, pNeighborBssDesc);
    }

    if (pNeighborRoamInfo->roamChannelInfo.currentChannelListInfo.ChannelList)
    {
#if 0
        //                                                                    
        //                                                            
        status = csrNeighborRoamMergeChannelLists( 
                pMac, 
                pNeighborRoamInfo->roamChannelInfo.currentChannelListInfo.ChannelList, 
                pNeighborRoamInfo->roamChannelInfo.currentChannelListInfo.numOfChannels, 
                channelList, 
                numChannels, 
                &numChannels );
#endif

        vos_mem_free(pNeighborRoamInfo->roamChannelInfo.currentChannelListInfo.ChannelList);
    }

    pNeighborRoamInfo->roamChannelInfo.currentChannelListInfo.ChannelList = NULL;
    /*                                                                        */
    if (numChannels)
        pNeighborRoamInfo->roamChannelInfo.currentChannelListInfo.ChannelList = vos_mem_malloc((numChannels) * sizeof(tANI_U8));
    if (NULL == pNeighborRoamInfo->roamChannelInfo.currentChannelListInfo.ChannelList)
    {
        smsLog(pMac, LOGE, FL("Memory allocation for Channel list failed.. TL event ignored"));
        return VOS_STATUS_E_RESOURCES;
    }

    vos_mem_copy(pNeighborRoamInfo->roamChannelInfo.currentChannelListInfo.ChannelList, 
                                            channelList, (numChannels) * sizeof(tANI_U8));
    pNeighborRoamInfo->roamChannelInfo.currentChannelListInfo.numOfChannels = numChannels;
    if (numChannels)
    {
        smsLog(pMac, LOG1, FL("IAPP Neighbor list callback received as expected in state %d."), 
            pNeighborRoamInfo->neighborRoamState);
        pNeighborRoamInfo->roamChannelInfo.IAPPNeighborListReceived = eANI_BOOLEAN_TRUE;
    }
    pNeighborRoamInfo->roamChannelInfo.currentChanIndex = 0;
    pNeighborRoamInfo->roamChannelInfo.chanListScanInProgress = eANI_BOOLEAN_TRUE;
    
    return VOS_STATUS_SUCCESS;
}

/*                                                                            

                                              

                                                                                  
                                                                                
                                                                                
                                                                 
                               

                                                     
                                                               

                

                                                                           */
void csrNeighborRoamRRMNeighborReportResult(void *context, VOS_STATUS vosStatus)
{
    tpAniSirGlobal pMac = PMAC_STRUCT(context);
    tpCsrNeighborRoamControlInfo    pNeighborRoamInfo = &pMac->roam.neighborRoamInfo;
    eHalStatus  status = eHAL_STATUS_SUCCESS;

    smsLog(pMac, LOG1, FL("Neighbor report result callback with status = %d\n"), vosStatus);
    switch (pNeighborRoamInfo->neighborRoamState)
    {
        case eCSR_NEIGHBOR_ROAM_STATE_REPORT_QUERY:
            /*                                   */
            pNeighborRoamInfo->FTRoamInfo.neighborRptPending = eANI_BOOLEAN_FALSE;
            if (VOS_STATUS_SUCCESS == vosStatus)
            {
                /*                                                                                               */
                vosStatus = csrNeighborRoamCreateChanListFromNeighborReport(pMac);
                if (VOS_STATUS_SUCCESS == vosStatus)
                {
                    NEIGHBOR_ROAM_DEBUG(pMac, LOGE, FL("Channel List created from Neighbor report, Transitioning to NEIGHBOR_SCAN state\n")); 
                }

                /*                                                                                                */
                pNeighborRoamInfo->scanRequestTimeStamp = (tANI_TIMESTAMP)palGetTickCount(pMac->hHdd);
                
                /*                                                               */
                /*                                                                                                       
                                              */
                status = palTimerStart(pMac->hHdd, pNeighborRoamInfo->neighborScanTimer, 
                                pNeighborRoamInfo->cfgParams.neighborScanPeriod * PAL_TIMER_TO_MS_UNIT, 
                                eANI_BOOLEAN_FALSE);
                if (eHAL_STATUS_SUCCESS != status)
                {
                    /*                                               */
                    smsLog(pMac, LOGE, FL("PAL Timer start for neighbor scan timer failed, status = %d, Ignoring state transition"), status);
                    vos_mem_free(pNeighborRoamInfo->roamChannelInfo.currentChannelListInfo.ChannelList);
                    pNeighborRoamInfo->roamChannelInfo.currentChannelListInfo.ChannelList = NULL;
                    return;
                }
                pNeighborRoamInfo->FTRoamInfo.currentNeighborRptRetryNum = 0;                
                /*                                                              */
                CSR_NEIGHBOR_ROAM_STATE_TRANSITION(eCSR_NEIGHBOR_ROAM_STATE_REPORT_SCAN)
            }
            else
            {
                /*                                                                                                 
                                                                                                         */
                smsLog(pMac, LOGE, FL("Neighbor report result failed after %d retries, MAX RETRIES = %d\n"), 
                     pNeighborRoamInfo->FTRoamInfo.currentNeighborRptRetryNum, pNeighborRoamInfo->cfgParams.maxNeighborRetries);
                if (pNeighborRoamInfo->FTRoamInfo.currentNeighborRptRetryNum >= 
                        pNeighborRoamInfo->cfgParams.maxNeighborRetries)
                {
                    smsLog(pMac, LOGE, FL("Bailing out to CFG Channel list scan.. \n"));
                    vosStatus = csrNeighborRoamTransitToCFGChanScan(pMac);
                    if (VOS_STATUS_SUCCESS != vosStatus)
                    {
                        smsLog(pMac, LOGE, FL("Transit to CFG Channel list scan state failed with status %d \n"), vosStatus);
                        return;
                    }
                    /*                                                                               */
                    pNeighborRoamInfo->FTRoamInfo.currentNeighborRptRetryNum = 0;
                }
                else
                {
                    vosStatus = csrNeighborRoamIssueNeighborRptRequest(pMac);
                    if (VOS_STATUS_SUCCESS != vosStatus)
                    {
                        smsLog(pMac, LOGE, FL("Neighbor report request failed. status = %d\n"), vosStatus);
                        return;
                    }
                    pNeighborRoamInfo->FTRoamInfo.neighborRptPending = eANI_BOOLEAN_TRUE;
                    /*                                                                                           */
                    pNeighborRoamInfo->FTRoamInfo.currentNeighborRptRetryNum++;
                }
            }
            break;
        default:
            smsLog(pMac, LOGE, FL("Neighbor result callback not expected in state %d, Ignoring.."), pNeighborRoamInfo->neighborRoamState);
            break;
    }
    return;
}
#endif /*                         */


#ifdef FEATURE_WLAN_LFR 
tANI_BOOLEAN csrNeighborRoamIsSsidCandidateMatch( 
        tpAniSirGlobal pMac, 
        tDot11fBeaconIEs *pIes)
{
    tpCsrNeighborRoamControlInfo    pNeighborRoamInfo = &pMac->roam.neighborRoamInfo;
    tANI_U8 sessionId   = (tANI_U8)pNeighborRoamInfo->csrSessionId;
    tCsrRoamConnectedProfile *pCurProfile;
    tANI_BOOLEAN fMatch = FALSE;

    if( !(pMac->roam.roamSession
            && CSR_IS_SESSION_VALID(pMac, sessionId)))
        return TRUE;  //                                                      

    pCurProfile = &pMac->roam.roamSession[sessionId].connectedProfile;

    if( !pCurProfile)
        return TRUE;  //                                                      

    if( pIes )
    {
        if(pIes->SSID.present)
        {
            fMatch = csrIsSsidMatch( pMac, (void *)pCurProfile->SSID.ssId, pCurProfile->SSID.length, 
                    pIes->SSID.ssid, pIes->SSID.num_ssid, 
                    eANI_BOOLEAN_TRUE ); //                                     
            //                                         
            return fMatch;  
        } else
            return FALSE;  //                                     
    } else
        return FALSE;  //                                                       
}

/*                                                                            

                                         

                                                                                     
                                                                                     
                                                                                       
                                                                                       
                                                                             

                                                  
                                                          
                                                                                
                                                                                
                                                                                           

                                                                             

                                                                           */
VOS_STATUS csrNeighborRoamReorderChannelList( 
        tpAniSirGlobal pMac, 
        tANI_U8   *pInputChannelList, 
        int numOfChannels,
        tANI_U8   *pOutputChannelList,
        int *pOutputNumOfChannels 
        )
{
    int i = 0;
    int j = 0;
    static int index = 0;
    int outputNumOfChannels  = 0; //                                    
    tANI_U8 numOccupiedChannels = pMac->scan.occupiedChannels.numChannels;
    tANI_U8 *pOccupiedChannelList = pMac->scan.occupiedChannels.channelList;


    //                                                                      
    for (i = 0; i < numOccupiedChannels; i++)
    {
        if (pOccupiedChannelList[i] != 0) 
        {
            pOutputChannelList[i] = pOccupiedChannelList[i]; 
            outputNumOfChannels++;
        }
    }

    //                                                                                                                  
    for (j = 0; j < CSR_BG_SCAN_VALID_CHANNEL_LIST_CHUNK_SIZE; j++)
    {
        if (!csrIsChannelPresentInList(pOccupiedChannelList, numOccupiedChannels, pInputChannelList[(j+index)%numOfChannels]))
        {
            pOutputChannelList[i] = pInputChannelList[(j+index)%numOfChannels]; 
            i++;
            outputNumOfChannels++;
        }
    }

    //                                                                    
    index = (index + CSR_BG_SCAN_VALID_CHANNEL_LIST_CHUNK_SIZE) % numOfChannels; 

    //                               
    smsLog(pMac, LOGE, FL("numOfChannels in the default channels list=%d. Number in the final list=%d."), numOfChannels, i);

    //                              
    *pOutputNumOfChannels = outputNumOfChannels; 

    return eHAL_STATUS_SUCCESS;
}
#endif /*                  */

/*                                                                            

                                           

                                                                                    
                                                                                     
                                                                                     
                                                                              

                                                  

                                                                             

                                                                           */
VOS_STATUS csrNeighborRoamTransitToCFGChanScan(tpAniSirGlobal pMac)
{
    tpCsrNeighborRoamControlInfo    pNeighborRoamInfo = &pMac->roam.neighborRoamInfo;
    eHalStatus  status  = eHAL_STATUS_SUCCESS;
    int i = 0;
    int numOfChannels = 0;
    tANI_U8   channelList[WNI_CFG_VALID_CHANNEL_LIST_LEN];

    if ( 
#ifdef FEATURE_WLAN_CCX
        ((pNeighborRoamInfo->isCCXAssoc) && 
                    (pNeighborRoamInfo->roamChannelInfo.IAPPNeighborListReceived == eANI_BOOLEAN_FALSE)) ||
        (pNeighborRoamInfo->isCCXAssoc == eANI_BOOLEAN_FALSE) || 
#endif //    
        pNeighborRoamInfo->roamChannelInfo.currentChannelListInfo.numOfChannels == 0)

    {
        smsLog(pMac, LOGW, FL("Falling back to CFG channel list"));


        /*                                                                                           
                                                                                                                 
                                        */
        if (NULL != pNeighborRoamInfo->roamChannelInfo.currentChannelListInfo.ChannelList)
        {
            vos_mem_free(pNeighborRoamInfo->roamChannelInfo.currentChannelListInfo.ChannelList);
            pNeighborRoamInfo->roamChannelInfo.currentChannelListInfo.ChannelList = NULL;
        }
        VOS_ASSERT( pNeighborRoamInfo->roamChannelInfo.currentChannelListInfo.ChannelList == NULL);

        //                                                                                         
        //                                                                                                         
        if (0 != pNeighborRoamInfo->cfgParams.channelInfo.numOfChannels)
        {
            //                                                                                                    
            NEIGHBOR_ROAM_DEBUG(pMac, LOGE, "Using the channel list from cfg.ini");
            status = csrNeighborRoamMergeChannelLists( 
                    pMac, 
                    pNeighborRoamInfo->cfgParams.channelInfo.ChannelList, 
                    pNeighborRoamInfo->cfgParams.channelInfo.numOfChannels, 
                    channelList, 
                    0, //                                                                
                    &numOfChannels );
        } 
        else
            {
            /*                                     */
            NEIGHBOR_ROAM_DEBUG(pMac, LOGE, "Switching to master list of valid channels");
            numOfChannels = sizeof(pMac->roam.validChannelList);
            if(HAL_STATUS_SUCCESS(csrGetCfgValidChannels(pMac, (tANI_U8 *)pMac->roam.validChannelList, (tANI_U32 *) &numOfChannels)))
            {
                //                                                                                                                   
                status = csrNeighborRoamMergeChannelLists( 
                        pMac, 
                        (tANI_U8 *)pMac->roam.validChannelList, 
                        numOfChannels,   //                                                
                        channelList, 
                        0, //                                                                
                        &numOfChannels );  //                                                                       
            }
            else
            { 
                smsLog(pMac, LOGE, FL("Could not get valid channel list, TL event ignored")); 
                return VOS_STATUS_E_FAILURE;
            }
        }

        /*                                                                                 */

        /*                                                    */
        smsLog(pMac, LOGE, FL("%d channels in the default list. Add %d occupied channels. %d is the MAX scan channel list."), 
                numOfChannels, 
                CSR_BG_SCAN_OCCUPIED_CHANNEL_LIST_LEN, 
                numOfChannels+CSR_BG_SCAN_OCCUPIED_CHANNEL_LIST_LEN );
        pNeighborRoamInfo->roamChannelInfo.currentChannelListInfo.numOfChannels = numOfChannels;
        VOS_ASSERT( pNeighborRoamInfo->roamChannelInfo.currentChannelListInfo.ChannelList == NULL);
        if (numOfChannels)
        {
            pNeighborRoamInfo->roamChannelInfo.currentChannelListInfo.ChannelList = 
                vos_mem_malloc(numOfChannels+CSR_BG_SCAN_OCCUPIED_CHANNEL_LIST_LEN );
        }
    
        if (NULL == pNeighborRoamInfo->roamChannelInfo.currentChannelListInfo.ChannelList)
        {
            smsLog(pMac, LOGE, FL("Memory allocation for Channel list failed.. TL event ignored"));
            return VOS_STATUS_E_RESOURCES;
        }
    
#ifdef FEATURE_WLAN_LFR
        /*                                                                  */
    
        status = csrNeighborRoamReorderChannelList( pMac, 
                channelList, 
                numOfChannels, 
                pNeighborRoamInfo->roamChannelInfo.currentChannelListInfo.ChannelList, 
                &numOfChannels );
        if (eHAL_STATUS_SUCCESS != status)
#endif
        {
            /*                     */
            smsLog(pMac, LOGE, FL("Cannot re-order scan channel list. (status = %d) Going to use default scan channel list."), status);
        vos_mem_copy(pNeighborRoamInfo->roamChannelInfo.currentChannelListInfo.ChannelList, 
                                channelList, numOfChannels * sizeof(tANI_U8));
        } 

        /*                                            */
        pNeighborRoamInfo->roamChannelInfo.currentChannelListInfo.numOfChannels = numOfChannels;
        for (i = 0; i < pNeighborRoamInfo->roamChannelInfo.currentChannelListInfo.numOfChannels; i++)
        {
            NEIGHBOR_ROAM_DEBUG(pMac, LOGE, "Channel List from CFG (or scan caching) = %d\n", 
                pNeighborRoamInfo->roamChannelInfo.currentChannelListInfo.ChannelList[i]);
        }
    }

    /*                                                                                                */
    pNeighborRoamInfo->scanRequestTimeStamp = (tANI_TIMESTAMP)palGetTickCount(pMac->hHdd);
    
    palTimerStop(pMac->hHdd, pNeighborRoamInfo->neighborScanTimer);
    /*                                                                                                       
                                       */
    status = palTimerStart(pMac->hHdd, pNeighborRoamInfo->neighborScanTimer, 
                    pNeighborRoamInfo->cfgParams.neighborScanPeriod * PAL_TIMER_TO_MS_UNIT, 
                    eANI_BOOLEAN_FALSE);
    
    if (eHAL_STATUS_SUCCESS != status)
    {
        /*                       */
        smsLog(pMac, LOGE, FL("Neighbor scan PAL Timer start failed, status = %d, Ignoring state transition"), status);
        vos_mem_free(pNeighborRoamInfo->roamChannelInfo.currentChannelListInfo.ChannelList);
        pNeighborRoamInfo->roamChannelInfo.currentChannelListInfo.ChannelList = NULL;
        return VOS_STATUS_E_FAILURE;
    }
    
    pNeighborRoamInfo->roamChannelInfo.currentChanIndex = 0;
    pNeighborRoamInfo->roamChannelInfo.chanListScanInProgress = eANI_BOOLEAN_TRUE;
    /*                                                                       */
    csrScanFlushResult(pMac);
    
    /*                                        */
    CSR_NEIGHBOR_ROAM_STATE_TRANSITION(eCSR_NEIGHBOR_ROAM_STATE_CFG_CHAN_LIST_SCAN)

    return VOS_STATUS_SUCCESS;
}

/*                                                                            

                                            

                                                                                  
                                                                                      
                                                             

                                                  

                                                                             

                                                                           */
VOS_STATUS  csrNeighborRoamNeighborLookupUpEvent(tpAniSirGlobal pMac)
{
    tpCsrNeighborRoamControlInfo    pNeighborRoamInfo = &pMac->roam.neighborRoamInfo;
    VOS_STATUS  vosStatus;
    csrNeighborRoamDeregAllRssiIndication(pMac);

    /*                                            */
    if (pNeighborRoamInfo->neighborRoamState != eCSR_NEIGHBOR_ROAM_STATE_CONNECTED)
        CSR_NEIGHBOR_ROAM_STATE_TRANSITION(eCSR_NEIGHBOR_ROAM_STATE_CONNECTED)

    /*                                                                                                                          */
    csrNeighborRoamResetConnectedStateControlInfo(pMac);

    
    NEIGHBOR_ROAM_DEBUG(pMac, LOG2, FL("Registering DOWN event neighbor lookup callback with TL. RSSI = %d,"), pNeighborRoamInfo->currentNeighborLookupThreshold * (-1));
    /*                                                                        */
    vosStatus = WLANTL_RegRSSIIndicationCB(pMac->roam.gVosContext, (v_S7_t)pNeighborRoamInfo->currentNeighborLookupThreshold * (-1),
                                    WLANTL_HO_THRESHOLD_DOWN, 
                                    csrNeighborRoamNeighborLookupDOWNCallback, 
                                    VOS_MODULE_ID_SME, pMac);
    if(!VOS_IS_STATUS_SUCCESS(vosStatus))
    {
       //       
       smsLog(pMac, LOGW, FL(" Couldn't register csrNeighborRoamNeighborLookupCallback DOWN event with TL: Status = %d\n"), vosStatus);
    }


    return vosStatus;
}

/*                                                                            

                                              

                                                                                  
                                                                                          
                                                                                          
                                  

                                                  

                                                                             

                                                                           */
VOS_STATUS  csrNeighborRoamNeighborLookupDownEvent(tpAniSirGlobal pMac)
{
    tpCsrNeighborRoamControlInfo    pNeighborRoamInfo = &pMac->roam.neighborRoamInfo;
    VOS_STATUS  vosStatus = VOS_STATUS_SUCCESS;
    eHalStatus  status = eHAL_STATUS_SUCCESS;

    switch (pNeighborRoamInfo->neighborRoamState)
    {
        case eCSR_NEIGHBOR_ROAM_STATE_CONNECTED:
            
            NEIGHBOR_ROAM_DEBUG(pMac, LOG2, FL("Deregistering DOWN event neighbor lookup callback with TL. RSSI = %d,"), 
                                                            pNeighborRoamInfo->currentNeighborLookupThreshold * (-1));
            /*                                                        */
            vosStatus = WLANTL_DeregRSSIIndicationCB(pMac->roam.gVosContext, (v_S7_t)pNeighborRoamInfo->currentNeighborLookupThreshold * (-1),
                                            WLANTL_HO_THRESHOLD_DOWN, 
                                            csrNeighborRoamNeighborLookupDOWNCallback, 
                                            VOS_MODULE_ID_SME);
            
            if(!VOS_IS_STATUS_SUCCESS(vosStatus))
            {
               //       
               smsLog(pMac, LOGW, FL(" Couldn't Deregister csrNeighborRoamNeighborLookupCallback DOWN event from TL: Status = %d\n"), status);
            }
            
           
#if defined WLAN_FEATURE_VOWIFI_11R && defined WLAN_FEATURE_VOWIFI
            if ((pNeighborRoamInfo->is11rAssoc) && (pMac->rrm.rrmSmeContext.rrmConfig.rrmEnabled))
            {
               
                NEIGHBOR_ROAM_DEBUG(pMac, LOGE, FL("11R Association:Neighbor Lookup Down event received in CONNECTED state"));
                vosStatus = csrNeighborRoamIssueNeighborRptRequest(pMac);
                if (VOS_STATUS_SUCCESS != vosStatus)
                {
                    smsLog(pMac, LOGE, FL("Neighbor report request failed. status = %d\n"), vosStatus);
                    return vosStatus;
                }
                /*                                                                                           */
                pNeighborRoamInfo->FTRoamInfo.currentNeighborRptRetryNum++;
                pNeighborRoamInfo->FTRoamInfo.neighborRptPending = eANI_BOOLEAN_TRUE;
                CSR_NEIGHBOR_ROAM_STATE_TRANSITION(eCSR_NEIGHBOR_ROAM_STATE_REPORT_QUERY)
            }
            else
#endif      
            {
                NEIGHBOR_ROAM_DEBUG(pMac, LOG2, FL("Non 11R or CCX Association:Neighbor Lookup Down event received in CONNECTED state"));

                vosStatus = csrNeighborRoamTransitToCFGChanScan(pMac);
                if (VOS_STATUS_SUCCESS != vosStatus)
                {
                    return vosStatus;
                }
            }
            NEIGHBOR_ROAM_DEBUG(pMac, LOG2, FL("Registering UP event neighbor lookup callback with TL. RSSI = %d,"), pNeighborRoamInfo->cfgParams.neighborLookupThreshold * (-1));
            /*                                                                      */
            vosStatus = WLANTL_RegRSSIIndicationCB(pMac->roam.gVosContext, (v_S7_t)pNeighborRoamInfo->cfgParams.neighborLookupThreshold * (-1),
                                            WLANTL_HO_THRESHOLD_UP, 
                                            csrNeighborRoamNeighborLookupUPCallback, 
                                            VOS_MODULE_ID_SME, pMac);
            if(!VOS_IS_STATUS_SUCCESS(vosStatus))
            {
               //       
               smsLog(pMac, LOGW, FL(" Couldn't register csrNeighborRoamNeighborLookupCallback UP event with TL: Status = %d\n"), status);
            }
            break;
        default:
            smsLog(pMac, LOGE, FL("DOWN event received in invalid state %d..Ignoring..."), pNeighborRoamInfo->neighborRoamState);
            break;
            
    }
    return vosStatus;
}

/*                                                                            

                                               

                                                                              
                                                              

                                  
                                                      
                                                                                                      

                                                                             

                                                                           */
VOS_STATUS csrNeighborRoamNeighborLookupUPCallback (v_PVOID_t pAdapter, v_U8_t rssiNotification,
                                                                               v_PVOID_t pUserCtxt)
{
    tpAniSirGlobal pMac = PMAC_STRUCT( pUserCtxt );
    tpCsrNeighborRoamControlInfo    pNeighborRoamInfo = &pMac->roam.neighborRoamInfo;
    VOS_STATUS  vosStatus = eHAL_STATUS_SUCCESS;

    NEIGHBOR_ROAM_DEBUG(pMac, LOGE, FL("Neighbor Lookup UP indication callback called with notification %d"), rssiNotification);

    if(!csrIsConnStateConnectedInfra(pMac, pNeighborRoamInfo->csrSessionId))
    {
       smsLog(pMac, LOGW, "Ignoring the indication as we are not connected\n");
       return VOS_STATUS_SUCCESS;
    }

    VOS_ASSERT(WLANTL_HO_THRESHOLD_UP == rssiNotification);
    vosStatus = csrNeighborRoamNeighborLookupUpEvent(pMac);
    return vosStatus;
}

/*                                                                            

                                                 

                                                                              
                                                                 

                                  
                                                      
                                                                                                      

                                                                             

                                                                           */
VOS_STATUS csrNeighborRoamNeighborLookupDOWNCallback (v_PVOID_t pAdapter, v_U8_t rssiNotification,
                                                                               v_PVOID_t pUserCtxt)
{
    tpAniSirGlobal pMac = PMAC_STRUCT( pUserCtxt );
    tpCsrNeighborRoamControlInfo    pNeighborRoamInfo = &pMac->roam.neighborRoamInfo;
    VOS_STATUS  vosStatus = eHAL_STATUS_SUCCESS;

    NEIGHBOR_ROAM_DEBUG(pMac, LOGE, FL("Neighbor Lookup DOWN indication callback called with notification %d"), rssiNotification);

    if(!csrIsConnStateConnectedInfra(pMac, pNeighborRoamInfo->csrSessionId))
    {
       smsLog(pMac, LOGW, "Ignoring the indication as we are not connected\n");
       return VOS_STATUS_SUCCESS;
    }

    VOS_ASSERT(WLANTL_HO_THRESHOLD_DOWN == rssiNotification);
    vosStatus = csrNeighborRoamNeighborLookupDownEvent(pMac);

    return vosStatus;
}

#ifdef RSSI_HACK
extern int dumpCmdRSSI;
#endif

/*                                                                            

                                         

                                                                                   
                                                                                   
                                                                                    
                                                                           

                                                  
                                                            

                                                                              

                                                                           */
eHalStatus csrNeighborRoamIndicateDisconnect(tpAniSirGlobal pMac, tANI_U8 sessionId)
{
    tpCsrNeighborRoamControlInfo    pNeighborRoamInfo = &pMac->roam.neighborRoamInfo;

    smsLog(pMac, LOGE, FL("Disconnect indication on session %d in state %d (sub-state %d)"), 
           sessionId, pNeighborRoamInfo->neighborRoamState,
           pMac->roam.curSubState[sessionId]);
 
#ifdef FEATURE_WLAN_CCX
    {
      tCsrRoamSession *pSession = CSR_GET_SESSION( pMac, sessionId);
      if (pSession->connectedProfile.isCCXAssoc)
      {
          vos_mem_copy(&pSession->prevApSSID, &pSession->connectedProfile.SSID, sizeof(tSirMacSSid));
          vos_mem_copy(pSession->prevApBssid, pSession->connectedProfile.bssid, sizeof(tSirMacAddr));
          pSession->prevOpChannel = pSession->connectedProfile.operationChannel;
          pSession->isPrevApInfoValid = TRUE;
          pSession->roamTS1 = vos_timer_get_system_time();

      }
    }
#endif
   
#ifdef RSSI_HACK
    dumpCmdRSSI = -40;
#endif
    switch (pNeighborRoamInfo->neighborRoamState)
    {
        case eCSR_NEIGHBOR_ROAM_STATE_REASSOCIATING:
            //                                       
            //                                                           
            //       
            palTimerStop(pMac->hHdd, pNeighborRoamInfo->neighborScanTimer);
            palTimerStop(pMac->hHdd, pNeighborRoamInfo->neighborResultsRefreshTimer);
            if (!CSR_IS_ROAM_SUBSTATE_DISASSOC_HO( pMac, sessionId )) {
                /*
                                                                          
                                                                            
                                                                           
                                                                             
                                                                        
                                                                          
                                                              
                 */
                CSR_NEIGHBOR_ROAM_STATE_TRANSITION(eCSR_NEIGHBOR_ROAM_STATE_INIT);
            }
            break;

        case eCSR_NEIGHBOR_ROAM_STATE_INIT:
            csrNeighborRoamResetInitStateControlInfo(pMac);
            csrNeighborRoamDeregAllRssiIndication(pMac);
            break; 

        case eCSR_NEIGHBOR_ROAM_STATE_CFG_CHAN_LIST_SCAN:
            CSR_NEIGHBOR_ROAM_STATE_TRANSITION(eCSR_NEIGHBOR_ROAM_STATE_INIT);
            csrNeighborRoamResetCfgListChanScanControlInfo(pMac);
            csrNeighborRoamDeregAllRssiIndication(pMac);
            break;

        case eCSR_NEIGHBOR_ROAM_STATE_PREAUTH_DONE:
            /*                                         */
            palTimerStop(pMac->hHdd, pMac->ft.ftSmeContext.preAuthReassocIntvlTimer);
        case eCSR_NEIGHBOR_ROAM_STATE_REPORT_SCAN:
        case eCSR_NEIGHBOR_ROAM_STATE_PREAUTHENTICATING:
            CSR_NEIGHBOR_ROAM_STATE_TRANSITION(eCSR_NEIGHBOR_ROAM_STATE_INIT)
            csrNeighborRoamResetPreauthControlInfo(pMac);
            csrNeighborRoamResetReportScanStateControlInfo(pMac);
            csrNeighborRoamDeregAllRssiIndication(pMac);
            break;

        default:
            NEIGHBOR_ROAM_DEBUG(pMac, LOGE, FL("Received disconnect event in state %d"), pNeighborRoamInfo->neighborRoamState);
            NEIGHBOR_ROAM_DEBUG(pMac, LOGE, FL("Transitioning to INIT state"));
            CSR_NEIGHBOR_ROAM_STATE_TRANSITION(eCSR_NEIGHBOR_ROAM_STATE_INIT)
            break;
    }
    return eHAL_STATUS_SUCCESS;
}

/*                                                                            

                                      

                                                                                    
                                                                              
                                                                      

                                                  
                                                         
                                                      

                                                                              

                                                                           */
eHalStatus csrNeighborRoamIndicateConnect(tpAniSirGlobal pMac, tANI_U8 sessionId, VOS_STATUS vosStatus)
{
    tpCsrNeighborRoamControlInfo    pNeighborRoamInfo = &pMac->roam.neighborRoamInfo;
    eHalStatus  status = eHAL_STATUS_SUCCESS;
    VOS_STATUS  vstatus;

#if  defined (WLAN_FEATURE_VOWIFI_11R) || defined (FEATURE_WLAN_CCX) || defined(FEATURE_WLAN_LFR)
    int  init_ft_flag = FALSE;
#endif

    smsLog(pMac, LOG2, FL("Connect indication received with session id %d in state %d"), sessionId, pNeighborRoamInfo->neighborRoamState);

    switch (pNeighborRoamInfo->neighborRoamState)
    {
        case eCSR_NEIGHBOR_ROAM_STATE_REASSOCIATING:
            if (VOS_STATUS_SUCCESS != vosStatus)
            {
                /*                                                                                                           */
                CSR_NEIGHBOR_ROAM_STATE_TRANSITION(eCSR_NEIGHBOR_ROAM_STATE_INIT)
                break;
            }
            /*                                       */
        case eCSR_NEIGHBOR_ROAM_STATE_INIT:
            /*                                    */ 
            csrNeighborRoamResetInitStateControlInfo(pMac);

            CSR_NEIGHBOR_ROAM_STATE_TRANSITION(eCSR_NEIGHBOR_ROAM_STATE_CONNECTED)

            pNeighborRoamInfo->csrSessionId = sessionId;
            vos_mem_copy(pNeighborRoamInfo->currAPbssid, 
                        pMac->roam.roamSession[sessionId].connectedProfile.bssid, sizeof(tCsrBssid));
            pNeighborRoamInfo->currAPoperationChannel = pMac->roam.roamSession[sessionId].connectedProfile.operationChannel;
            pNeighborRoamInfo->neighborScanTimerInfo.pMac = pMac;
            pNeighborRoamInfo->neighborScanTimerInfo.sessionId = sessionId;
            
#if  defined (WLAN_FEATURE_VOWIFI_11R) || defined (FEATURE_WLAN_CCX) || defined(FEATURE_WLAN_LFR)
            /*                                                                            */
            csrNeighborRoamFreeRoamableBSSList(pMac, &pMac->roam.neighborRoamInfo.FTRoamInfo.preAuthDoneList);
#endif
            
#ifdef WLAN_FEATURE_VOWIFI_11R
            //                                            
            if ( csrIsAuthType11r( pMac->roam.roamSession[sessionId].connectedProfile.AuthType ) )
            {
                if (pMac->roam.configParam.isFastTransitionEnabled)
                    init_ft_flag = TRUE;
                pNeighborRoamInfo->is11rAssoc = eANI_BOOLEAN_TRUE;
            }
            else
                pNeighborRoamInfo->is11rAssoc = eANI_BOOLEAN_FALSE;
            NEIGHBOR_ROAM_DEBUG(pMac, LOG2, FL("11rAssoc is = %d"), pNeighborRoamInfo->is11rAssoc);
#endif

#ifdef FEATURE_WLAN_CCX
            //                                            
            if (pMac->roam.roamSession[sessionId].connectedProfile.isCCXAssoc)
            {
                if (pMac->roam.configParam.isFastTransitionEnabled)
                    init_ft_flag = TRUE;
                pNeighborRoamInfo->isCCXAssoc = eANI_BOOLEAN_TRUE;
            }
            else
                pNeighborRoamInfo->isCCXAssoc = eANI_BOOLEAN_FALSE;
            NEIGHBOR_ROAM_DEBUG(pMac, LOG2, FL("isCCXAssoc is = %d ft = %d"),
                                pNeighborRoamInfo->isCCXAssoc, init_ft_flag);
                            
#endif

#ifdef FEATURE_WLAN_LFR
            //                                     
            if (csrRoamIsFastRoamEnabled(pMac))
            {
                init_ft_flag = TRUE;
            }
#endif

#if  defined (WLAN_FEATURE_VOWIFI_11R) || defined (FEATURE_WLAN_CCX) || defined(FEATURE_WLAN_LFR)
            if ( init_ft_flag == TRUE )
            {
                /*                                                                  */
                pNeighborRoamInfo->FTRoamInfo.preAuthRspWaitTimerInfo.pMac = pMac;
                pNeighborRoamInfo->FTRoamInfo.preAuthRspWaitTimerInfo.sessionId = sessionId;
                pNeighborRoamInfo->FTRoamInfo.currentNeighborRptRetryNum = 0;
                csrNeighborRoamPurgePreauthFailedList(pMac);

                NEIGHBOR_ROAM_DEBUG(pMac, LOG2, FL("Registering neighbor lookup DOWN event with TL, RSSI = %d"), pNeighborRoamInfo->currentNeighborLookupThreshold);
                /*                                                                         */
                vstatus = WLANTL_RegRSSIIndicationCB(pMac->roam.gVosContext, (v_S7_t)pNeighborRoamInfo->currentNeighborLookupThreshold * (-1),
                                            WLANTL_HO_THRESHOLD_DOWN, 
                                            csrNeighborRoamNeighborLookupDOWNCallback, 
                                            VOS_MODULE_ID_SME, pMac);
            
                if(!VOS_IS_STATUS_SUCCESS(vstatus))
                {
                   //       
                   smsLog(pMac, LOGW, FL(" Couldn't register csrNeighborRoamNeighborLookupDOWNCallback with TL: Status = %d\n"), vstatus);
                   status = eHAL_STATUS_FAILURE;
                }
            }
#endif
            break;
        default:
            smsLog(pMac, LOGE, FL("Connect event received in invalid state %d..Ignoring..."), pNeighborRoamInfo->neighborRoamState);
            break;
    }
    return status;
}


#ifdef WLAN_FEATURE_VOWIFI_11R
/*                                                                            

                                                      

                                                                                   
                                                                                   
                   

                                                                    

                

                                                                           */
void csrNeighborRoamPreAuthResponseWaitTimerHandler(void *context)
{
    tCsrTimerInfo *pTimerInfo = (tCsrTimerInfo *)context;
    tpAniSirGlobal pMac = (tpAniSirGlobal)pTimerInfo->pMac;
    tpCsrNeighborRoamControlInfo    pNeighborRoamInfo = &pMac->roam.neighborRoamInfo;

    pNeighborRoamInfo->FTRoamInfo.preauthRspPending = eANI_BOOLEAN_FALSE;

    csrNeighborRoamPreauthRspHandler(pMac, VOS_STATUS_E_TIMEOUT);
}

/*                                                                            

                                             

                                                                                

                                                  

                

                                                                           */
void csrNeighborRoamPurgePreauthFailedList(tpAniSirGlobal pMac)
{
    tANI_U8 i;

    for (i = 0; i < pMac->roam.neighborRoamInfo.FTRoamInfo.preAuthFailList.numMACAddress; i++)
    {
        vos_mem_zero(pMac->roam.neighborRoamInfo.FTRoamInfo.preAuthFailList.macAddress[i], sizeof(tSirMacAddr));
    }
    pMac->roam.neighborRoamInfo.FTRoamInfo.preAuthFailList.numMACAddress = 0;

    return;
}

/*                                                                            

                                       

                                                                               

                                                  

                                                                              

                                                                           */
eHalStatus csrNeighborRoamInit11rAssocInfo(tpAniSirGlobal pMac)
{
    eHalStatus  status;
    tpCsr11rAssocNeighborInfo   pFTRoamInfo = &pMac->roam.neighborRoamInfo.FTRoamInfo;

    pMac->roam.neighborRoamInfo.is11rAssoc = eANI_BOOLEAN_FALSE;
    pMac->roam.neighborRoamInfo.cfgParams.maxNeighborRetries = pMac->roam.configParam.neighborRoamConfig.nMaxNeighborRetries;
    pFTRoamInfo->neighborReportTimeout = CSR_NEIGHBOR_ROAM_REPORT_QUERY_TIMEOUT;
    pFTRoamInfo->PEPreauthRespTimeout = CSR_NEIGHBOR_ROAM_PREAUTH_RSP_WAIT_MULTIPLIER * pMac->roam.neighborRoamInfo.cfgParams.neighborScanPeriod;
    pFTRoamInfo->neighborRptPending = eANI_BOOLEAN_FALSE;
    pFTRoamInfo->preauthRspPending = eANI_BOOLEAN_FALSE;
    
    pFTRoamInfo->preAuthRspWaitTimerInfo.pMac = pMac;
    pFTRoamInfo->preAuthRspWaitTimerInfo.sessionId = CSR_SESSION_ID_INVALID;
    status = palTimerAlloc(pMac->hHdd, &pFTRoamInfo->preAuthRspWaitTimer, 
                    csrNeighborRoamPreAuthResponseWaitTimerHandler, (void *)&pFTRoamInfo->preAuthRspWaitTimerInfo);

    if (eHAL_STATUS_SUCCESS != status)
    {
        smsLog(pMac, LOGE, FL("Response wait Timer allocation failed"));
        return eHAL_STATUS_RESOURCES;
    }
    
    pMac->roam.neighborRoamInfo.FTRoamInfo.currentNeighborRptRetryNum = 0;
    pMac->roam.neighborRoamInfo.FTRoamInfo.numBssFromNeighborReport = 0;
    vos_mem_zero(pMac->roam.neighborRoamInfo.FTRoamInfo.neighboReportBssInfo, 
                            sizeof(tCsrNeighborReportBssInfo) * MAX_BSS_IN_NEIGHBOR_RPT);

    
    status = csrLLOpen(pMac->hHdd, &pFTRoamInfo->preAuthDoneList);
    if (eHAL_STATUS_SUCCESS != status)
    {
        smsLog(pMac, LOGE, FL("LL Open of preauth done AP List failed"));
        palTimerFree(pMac->hHdd, pFTRoamInfo->preAuthRspWaitTimer);
        return eHAL_STATUS_RESOURCES;
    }
    return status;
}
#endif /*                         */

/*                                                                            

                           

                                                                   

                                                  

                                                                              

                                                                           */
eHalStatus csrNeighborRoamInit(tpAniSirGlobal pMac)
{
    eHalStatus status;
    tpCsrNeighborRoamControlInfo    pNeighborRoamInfo = &pMac->roam.neighborRoamInfo;

    pNeighborRoamInfo->neighborRoamState       =   eCSR_NEIGHBOR_ROAM_STATE_CLOSED;
    pNeighborRoamInfo->prevNeighborRoamState   =   eCSR_NEIGHBOR_ROAM_STATE_CLOSED;
    pNeighborRoamInfo->csrSessionId            =   CSR_SESSION_ID_INVALID;
    pNeighborRoamInfo->cfgParams.maxChannelScanTime = pMac->roam.configParam.neighborRoamConfig.nNeighborScanMaxChanTime;
    pNeighborRoamInfo->cfgParams.minChannelScanTime = pMac->roam.configParam.neighborRoamConfig.nNeighborScanMinChanTime;
    pNeighborRoamInfo->cfgParams.maxNeighborRetries = 0;
    pNeighborRoamInfo->cfgParams.neighborLookupThreshold = pMac->roam.configParam.neighborRoamConfig.nNeighborLookupRssiThreshold;
    pNeighborRoamInfo->cfgParams.neighborReassocThreshold = pMac->roam.configParam.neighborRoamConfig.nNeighborReassocRssiThreshold;
    pNeighborRoamInfo->cfgParams.neighborScanPeriod = pMac->roam.configParam.neighborRoamConfig.nNeighborScanTimerPeriod;
    pNeighborRoamInfo->cfgParams.neighborResultsRefreshPeriod = pMac->roam.configParam.neighborRoamConfig.nNeighborResultsRefreshPeriod;
    
    pNeighborRoamInfo->cfgParams.channelInfo.numOfChannels   =   
                        pMac->roam.configParam.neighborRoamConfig.neighborScanChanList.numChannels;

    pNeighborRoamInfo->cfgParams.channelInfo.ChannelList = 
                vos_mem_malloc(pMac->roam.configParam.neighborRoamConfig.neighborScanChanList.numChannels);

    if (NULL == pNeighborRoamInfo->cfgParams.channelInfo.ChannelList)
    {
        smsLog(pMac, LOGE, FL("Memory Allocation for CFG Channel List failed"));
        return eHAL_STATUS_RESOURCES;
    }

    /*                                           */
    palCopyMemory(pMac->hHdd, pNeighborRoamInfo->cfgParams.channelInfo.ChannelList, 
                        pMac->roam.configParam.neighborRoamConfig.neighborScanChanList.channelList,
                        pMac->roam.configParam.neighborRoamConfig.neighborScanChanList.numChannels);

    vos_mem_set(pNeighborRoamInfo->currAPbssid, sizeof(tCsrBssid), 0);
    pNeighborRoamInfo->currentNeighborLookupThreshold = pMac->roam.neighborRoamInfo.cfgParams.neighborLookupThreshold;
    pNeighborRoamInfo->currentScanResultsRefreshPeriod = 
        NEIGHBOR_SCAN_RESULTS_REFRESH_PERIOD_MIN;
    pNeighborRoamInfo->scanRspPending = eANI_BOOLEAN_FALSE;

    pNeighborRoamInfo->neighborScanTimerInfo.pMac = pMac;
    pNeighborRoamInfo->neighborScanTimerInfo.sessionId = CSR_SESSION_ID_INVALID;
    status = palTimerAlloc(pMac->hHdd, &pNeighborRoamInfo->neighborScanTimer, 
                    csrNeighborRoamNeighborScanTimerCallback, (void *)&pNeighborRoamInfo->neighborScanTimerInfo);

    if (eHAL_STATUS_SUCCESS != status)
    {
        smsLog(pMac, LOGE, FL("Response wait Timer allocation failed"));
        vos_mem_free(pNeighborRoamInfo->cfgParams.channelInfo.ChannelList);
        pNeighborRoamInfo->cfgParams.channelInfo.ChannelList = NULL;
        return eHAL_STATUS_RESOURCES;
    }

    status = palTimerAlloc(pMac->hHdd, &pNeighborRoamInfo->neighborResultsRefreshTimer, 
                    csrNeighborRoamResultsRefreshTimerCallback, (void *)&pNeighborRoamInfo->neighborScanTimerInfo);

    if (eHAL_STATUS_SUCCESS != status)
    {
        smsLog(pMac, LOGE, FL("Response wait Timer allocation failed"));
        smsLog(pMac, LOGE, FL("LL Open of roamable AP List failed"));
        vos_mem_free(pNeighborRoamInfo->cfgParams.channelInfo.ChannelList);
        pNeighborRoamInfo->cfgParams.channelInfo.ChannelList = NULL;
        palTimerFree(pMac->hHdd, pNeighborRoamInfo->neighborScanTimer);
        return eHAL_STATUS_RESOURCES;
    }

    status = csrLLOpen(pMac->hHdd, &pNeighborRoamInfo->roamableAPList);
    if (eHAL_STATUS_SUCCESS != status)
    {
        smsLog(pMac, LOGE, FL("LL Open of roamable AP List failed"));
        vos_mem_free(pNeighborRoamInfo->cfgParams.channelInfo.ChannelList);
        pNeighborRoamInfo->cfgParams.channelInfo.ChannelList = NULL;
        palTimerFree(pMac->hHdd, pNeighborRoamInfo->neighborScanTimer);
        palTimerFree(pMac->hHdd, pNeighborRoamInfo->neighborResultsRefreshTimer);
        return eHAL_STATUS_RESOURCES;
    }

    pNeighborRoamInfo->roamChannelInfo.currentChanIndex = CSR_NEIGHBOR_ROAM_INVALID_CHANNEL_INDEX;
    pNeighborRoamInfo->roamChannelInfo.currentChannelListInfo.numOfChannels = 0;
    pNeighborRoamInfo->roamChannelInfo.currentChannelListInfo.ChannelList = NULL;
    pNeighborRoamInfo->roamChannelInfo.chanListScanInProgress = eANI_BOOLEAN_FALSE;
    pNeighborRoamInfo->roamChannelInfo.IAPPNeighborListReceived = eANI_BOOLEAN_FALSE;

#ifdef WLAN_FEATURE_VOWIFI_11R
    status = csrNeighborRoamInit11rAssocInfo(pMac);
    if (eHAL_STATUS_SUCCESS != status)
    {
        smsLog(pMac, LOGE, FL("LL Open of roamable AP List failed"));
        vos_mem_free(pNeighborRoamInfo->cfgParams.channelInfo.ChannelList);
        pNeighborRoamInfo->cfgParams.channelInfo.ChannelList = NULL;
        palTimerFree(pMac->hHdd, pNeighborRoamInfo->neighborScanTimer);
        palTimerFree(pMac->hHdd, pNeighborRoamInfo->neighborResultsRefreshTimer);        
        csrLLClose(&pNeighborRoamInfo->roamableAPList);
        return eHAL_STATUS_RESOURCES;
    }
#endif
    /*                                             */
    pNeighborRoamInfo->scanRequestTimeStamp = (tANI_TIMESTAMP)palGetTickCount(pMac->hHdd);

    CSR_NEIGHBOR_ROAM_STATE_TRANSITION(eCSR_NEIGHBOR_ROAM_STATE_INIT)
    
    return eHAL_STATUS_SUCCESS;
}

/*                                                                            

                            

                                                                            

                                                  

                

                                                                           */
void csrNeighborRoamClose(tpAniSirGlobal pMac)
{
    tpCsrNeighborRoamControlInfo    pNeighborRoamInfo = &pMac->roam.neighborRoamInfo;

    if (eCSR_NEIGHBOR_ROAM_STATE_CLOSED == pNeighborRoamInfo->neighborRoamState)
    {
        smsLog(pMac, LOGE, FL("Neighbor Roam Algorithm Already Closed\n"));
        return;
    }

    if (pNeighborRoamInfo->cfgParams.channelInfo.ChannelList)
        vos_mem_free(pNeighborRoamInfo->cfgParams.channelInfo.ChannelList);
   
    pNeighborRoamInfo->cfgParams.channelInfo.ChannelList = NULL;
    
    pNeighborRoamInfo->neighborScanTimerInfo.pMac = NULL;
    pNeighborRoamInfo->neighborScanTimerInfo.sessionId = CSR_SESSION_ID_INVALID;
    palTimerFree(pMac->hHdd, pNeighborRoamInfo->neighborScanTimer);
    palTimerFree(pMac->hHdd, pNeighborRoamInfo->neighborResultsRefreshTimer);

    /*                                                                            */
    csrNeighborRoamFreeRoamableBSSList(pMac, &pNeighborRoamInfo->roamableAPList);
    csrLLClose(&pNeighborRoamInfo->roamableAPList);
    
    if (pNeighborRoamInfo->roamChannelInfo.currentChannelListInfo.ChannelList)
    {
        vos_mem_free(pNeighborRoamInfo->roamChannelInfo.currentChannelListInfo.ChannelList);
    }

    pNeighborRoamInfo->roamChannelInfo.currentChannelListInfo.ChannelList = NULL;
    pNeighborRoamInfo->roamChannelInfo.currentChanIndex = CSR_NEIGHBOR_ROAM_INVALID_CHANNEL_INDEX;
    pNeighborRoamInfo->roamChannelInfo.currentChannelListInfo.numOfChannels = 0;
    pNeighborRoamInfo->roamChannelInfo.currentChannelListInfo.ChannelList = NULL;
    pNeighborRoamInfo->roamChannelInfo.chanListScanInProgress = eANI_BOOLEAN_FALSE;    
    pNeighborRoamInfo->roamChannelInfo.IAPPNeighborListReceived = eANI_BOOLEAN_FALSE;

    /*                    */ 
    csrReleaseProfile(pMac, &pNeighborRoamInfo->csrNeighborRoamProfile);

#ifdef WLAN_FEATURE_VOWIFI_11R
    pMac->roam.neighborRoamInfo.FTRoamInfo.currentNeighborRptRetryNum = 0;
    palTimerFree(pMac->hHdd, pMac->roam.neighborRoamInfo.FTRoamInfo.preAuthRspWaitTimer);
    pMac->roam.neighborRoamInfo.FTRoamInfo.preAuthRspWaitTimerInfo.pMac = NULL;
    pMac->roam.neighborRoamInfo.FTRoamInfo.preAuthRspWaitTimerInfo.sessionId = CSR_SESSION_ID_INVALID;
    pMac->roam.neighborRoamInfo.FTRoamInfo.numBssFromNeighborReport = 0;
    vos_mem_zero(pMac->roam.neighborRoamInfo.FTRoamInfo.neighboReportBssInfo, 
                            sizeof(tCsrNeighborReportBssInfo) * MAX_BSS_IN_NEIGHBOR_RPT);
    csrNeighborRoamFreeRoamableBSSList(pMac, &pMac->roam.neighborRoamInfo.FTRoamInfo.preAuthDoneList);
    csrLLClose(&pMac->roam.neighborRoamInfo.FTRoamInfo.preAuthDoneList);
#endif /*                         */

    CSR_NEIGHBOR_ROAM_STATE_TRANSITION(eCSR_NEIGHBOR_ROAM_STATE_CLOSED)
    
    return;
}

/*                                                                            

                                     

                                                                              
                                                                                    
                                                                   

                                                  

                

                                                                           */
void csrNeighborRoamRequestHandoff(tpAniSirGlobal pMac)
{

    tCsrRoamInfo roamInfo;
    tpCsrNeighborRoamControlInfo    pNeighborRoamInfo = &pMac->roam.neighborRoamInfo;
    tANI_U32 sessionId = pNeighborRoamInfo->csrSessionId;
    tCsrNeighborRoamBSSInfo     handoffNode;
    extern void csrRoamRoamingStateDisassocRspProcessor( tpAniSirGlobal pMac, tSirSmeDisassocRsp *pSmeDisassocRsp );
    tANI_U32 roamId = 0;

    if (pMac->roam.neighborRoamInfo.neighborRoamState != eCSR_NEIGHBOR_ROAM_STATE_PREAUTH_DONE) 
    {
        smsLog(pMac, LOGE, FL("Roam requested when Neighbor roam is in %d state"), 
            pMac->roam.neighborRoamInfo.neighborRoamState);
        return;
    }

    vos_mem_zero(&roamInfo, sizeof(tCsrRoamInfo));
    csrRoamCallCallback(pMac, pNeighborRoamInfo->csrSessionId, &roamInfo, roamId, eCSR_ROAM_FT_START, 
                eSIR_SME_SUCCESS);

    vos_mem_zero(&roamInfo, sizeof(tCsrRoamInfo));
    CSR_NEIGHBOR_ROAM_STATE_TRANSITION(eCSR_NEIGHBOR_ROAM_STATE_REASSOCIATING)
    
    csrNeighborRoamGetHandoffAPInfo(pMac, &handoffNode);
    smsLog(pMac, LOGE, FL("HANDOFF CANDIDATE BSSID %02x:%02x:%02x:%02x:%02x:%02x"),
                                                handoffNode.pBssDescription->bssId[0], 
                                                handoffNode.pBssDescription->bssId[1], 
                                                handoffNode.pBssDescription->bssId[2], 
                                                handoffNode.pBssDescription->bssId[3], 
                                                handoffNode.pBssDescription->bssId[4], 
                                                handoffNode.pBssDescription->bssId[5]);
   
    /*                                                               */ 
    csrReleaseProfile(pMac, &pNeighborRoamInfo->csrNeighborRoamProfile);
    /*                                                                                                                 
                                                     */
    csrRoamCopyConnectedProfile(pMac, pNeighborRoamInfo->csrSessionId, &pNeighborRoamInfo->csrNeighborRoamProfile);
    vos_mem_copy(pNeighborRoamInfo->csrNeighborRoamProfile.BSSIDs.bssid, handoffNode.pBssDescription->bssId, sizeof(tSirMacAddr));
    pNeighborRoamInfo->csrNeighborRoamProfile.ChannelInfo.ChannelList[0] = handoffNode.pBssDescription->channelId;
    
    NEIGHBOR_ROAM_DEBUG(pMac, LOGW, " csrRoamHandoffRequested: disassociating with current AP\n");

    if(!HAL_STATUS_SUCCESS(csrRoamIssueDisassociateCmd(pMac, sessionId, eCSR_DISCONNECT_REASON_HANDOFF)))
    {
        smsLog(pMac, LOGW, "csrRoamHandoffRequested:  fail to issue disassociate\n");
        return;
    }                       

    //                                               
    roamInfo.reasonCode = eCsrRoamReasonBetterAP;

    vos_mem_copy(roamInfo.bssid, 
                 handoffNode.pBssDescription->bssId, 
                 sizeof( tCsrBssid ));

    csrRoamCallCallback(pMac, sessionId, &roamInfo, 0, eCSR_ROAM_ROAMING_START, eCSR_ROAM_RESULT_NONE);


    return;
}

/*                                                                            

                                          

                                                                                 
                                           

                                                  
                                                                            

                                                                                  

                                                                           */
tANI_BOOLEAN csrNeighborRoamIsHandoffInProgress(tpAniSirGlobal pMac)
{
    if (eCSR_NEIGHBOR_ROAM_STATE_REASSOCIATING == pMac->roam.neighborRoamInfo.neighborRoamState)
        return eANI_BOOLEAN_TRUE;

    return eANI_BOOLEAN_FALSE;
}

#ifdef WLAN_FEATURE_VOWIFI_11R
/*                                                                            

                                 

                                                                                       

                                                  

                                                                                   

                                                                           */
tANI_BOOLEAN csrNeighborRoamIs11rAssoc(tpAniSirGlobal pMac)
{
    return pMac->roam.neighborRoamInfo.is11rAssoc;
}
#endif /*                         */


/*                                                                            

                                       

                                                                                     
                                                                                        
                                               

                                                  
                                                                         

                

                                                                           */
void csrNeighborRoamGetHandoffAPInfo(tpAniSirGlobal pMac, tpCsrNeighborRoamBSSInfo pHandoffNode)
{
    tpCsrNeighborRoamControlInfo    pNeighborRoamInfo = &pMac->roam.neighborRoamInfo;
    tpCsrNeighborRoamBSSInfo        pBssNode;
    
    VOS_ASSERT(NULL != pHandoffNode); 
        
#ifdef WLAN_FEATURE_VOWIFI_11R
    if (pNeighborRoamInfo->is11rAssoc)
    {
        /*                                                          */
        pBssNode = csrNeighborRoamGetRoamableAPListNextEntry(pMac, &pNeighborRoamInfo->FTRoamInfo.preAuthDoneList, NULL);
        NEIGHBOR_ROAM_DEBUG(pMac, LOG1, FL("Number of Handoff candidates = %d"), csrLLCount(&pNeighborRoamInfo->FTRoamInfo.preAuthDoneList));
    }
    else
#endif
#ifdef FEATURE_WLAN_CCX
    if (pNeighborRoamInfo->isCCXAssoc)
    {
        /*                                                          */
        pBssNode = csrNeighborRoamGetRoamableAPListNextEntry(pMac, &pNeighborRoamInfo->FTRoamInfo.preAuthDoneList, NULL);
        NEIGHBOR_ROAM_DEBUG(pMac, LOG1, FL("Number of Handoff candidates = %d"), csrLLCount(&pNeighborRoamInfo->FTRoamInfo.preAuthDoneList));
    }
    else
#endif
#ifdef FEATURE_WLAN_LFR
    if (csrRoamIsFastRoamEnabled(pMac))
    {
        /*                                                          */
        pBssNode = csrNeighborRoamGetRoamableAPListNextEntry(pMac, &pNeighborRoamInfo->FTRoamInfo.preAuthDoneList, NULL);
        NEIGHBOR_ROAM_DEBUG(pMac, LOG1, FL("Number of Handoff candidates = %d"), csrLLCount(&pNeighborRoamInfo->FTRoamInfo.preAuthDoneList));
    }
    else
#endif
    {
        pBssNode = csrNeighborRoamGetRoamableAPListNextEntry(pMac, &pNeighborRoamInfo->roamableAPList, NULL);
        NEIGHBOR_ROAM_DEBUG(pMac, LOG1, FL("Number of Handoff candidates = %d"), csrLLCount(&pNeighborRoamInfo->roamableAPList));
    }
    vos_mem_copy(pHandoffNode, pBssNode, sizeof(tCsrNeighborRoamBSSInfo));

    return;
}

/*                                                                            
                                                               

                                                  

                   

                                                                           */
tANI_BOOLEAN csrNeighborRoamStatePreauthDone(tpAniSirGlobal pMac)
{
    return (pMac->roam.neighborRoamInfo.neighborRoamState == 
               eCSR_NEIGHBOR_ROAM_STATE_PREAUTH_DONE);
}

/*                                                                            
                                                                    
                                                                       
         
                                                                         
                                                                      
                                                                         
           

                                                  

                   
                                                                           */
void csrNeighborRoamTranistionPreauthDoneToDisconnected(tpAniSirGlobal pMac)
{
    if (pMac->roam.neighborRoamInfo.neighborRoamState != 
               eCSR_NEIGHBOR_ROAM_STATE_PREAUTH_DONE) return;

    //           
    palTimerStop(pMac->hHdd, pMac->ft.ftSmeContext.preAuthReassocIntvlTimer);

    //                         
    CSR_NEIGHBOR_ROAM_STATE_TRANSITION(eCSR_NEIGHBOR_ROAM_STATE_INIT)
}

#endif /*                               */
