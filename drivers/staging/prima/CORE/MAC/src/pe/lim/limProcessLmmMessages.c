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

#include "aniGlobal.h"
#include "wniApi.h"
#if (WNI_POLARIS_FW_PRODUCT == AP)
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
#include "limSerDesUtils.h"
#include "limPropExtsUtils.h"
#include "limSession.h"


#if (WNI_POLARIS_FW_PACKAGE == ADVANCED) && (WNI_POLARIS_FW_PRODUCT == AP)
/* 
                                
  
           
                                                          
                                 
  
        
                                                         
  
              
  
       
  
                                                                
                                                                   
                              
                              
 */

inline static tANI_BOOLEAN
limIsSmeMeasurementReqValid(tpAniSirGlobal pMac, tpSirSmeMeasurementReq pMeasReq)
{
#ifdef ANI_AP_SDK
    if (!pMeasReq->channelList.numChannels ||
        ((pMeasReq->measControl.periodicMeasEnabled) &&
         (!pMeasReq->measIndPeriod)) ||
        (pMeasReq->measIndPeriod &&
         ((pMeasReq->measIndPeriod < 1))) ||
        !pMeasReq->measDuration.shortTermPeriod ||
        ((pMeasReq->measDuration.shortTermPeriod < 1)) ||
        !pMeasReq->measDuration.averagingPeriod ||
        (pMeasReq->measDuration.averagingPeriod <
         pMeasReq->measDuration.shortTermPeriod) ||
        !pMeasReq->measDuration.shortChannelScanDuration ||
        ((pMeasReq->measDuration.shortChannelScanDuration <
          1)) ||
        !pMeasReq->measDuration.longChannelScanDuration ||
        (pMeasReq->measDuration.longChannelScanDuration <
         pMeasReq->measDuration.shortChannelScanDuration) ||
        ((pMeasReq->measDuration.longChannelScanDuration <
          1)))
#else
    if (!pMeasReq->channelList.numChannels ||
        ((pMeasReq->measControl.periodicMeasEnabled) &&
         (!pMeasReq->measIndPeriod)) ||
        (pMeasReq->measIndPeriod &&
         ((pMeasReq->measIndPeriod < SYS_TICK_DUR_MS))) ||
        !pMeasReq->measDuration.shortTermPeriod ||
        ((pMeasReq->measDuration.shortTermPeriod < SYS_TICK_DUR_MS)) ||
        !pMeasReq->measDuration.averagingPeriod ||
        (pMeasReq->measDuration.averagingPeriod <
         pMeasReq->measDuration.shortTermPeriod) ||
        !pMeasReq->measDuration.shortChannelScanDuration ||
        ((pMeasReq->measDuration.shortChannelScanDuration <
          SYS_TICK_DUR_MS)) ||
        !pMeasReq->measDuration.longChannelScanDuration ||
        (pMeasReq->measDuration.longChannelScanDuration <
         pMeasReq->measDuration.shortChannelScanDuration) ||
        ((pMeasReq->measDuration.longChannelScanDuration <
          SYS_TICK_DUR_MS)))


#endif
    {
        limLog(pMac, LOGW,
               FL("Received MEASUREMENT_REQ with invalid data\n"));

        return eANI_BOOLEAN_FALSE;
    }
    else
        return eANI_BOOLEAN_TRUE;

} /*                                       */


/* 
                         
  
           
                                                          
                                                           
                                                           
                                             
  
        
  
              
  
       
  
                                                  
               
               
 */

inline static tSirRetStatus
limInitMeasResources(tpAniSirGlobal pMac)
{
    tANI_U32    val;
    tANI_U32    beaconInterval;

    
    //                                     
    //                                  
    if (pMac->lim.gpLimMeasReq->measControl.periodicMeasEnabled)
    {
        val = SYS_MS_TO_TICKS(pMac->lim.gpLimMeasReq->measIndPeriod);
        if (tx_timer_create(
                        &pMac->lim.gLimMeasParams.measurementIndTimer,
                        "Meas Ind TIMEOUT",
                        limTimerHandler,
                        SIR_LIM_MEASUREMENT_IND_TIMEOUT,
                        val,
                        val,
                        TX_NO_ACTIVATE) != TX_SUCCESS)
        {
            //                                 
            //          
            limLog(pMac, LOGP, FL("call to create MeasInd timer failed\n"));

            return eSIR_SYS_TX_TIMER_CREATE_FAILED;
        }
        pMac->lim.gLimMeasParams.isMeasIndTimerActive = 0;
       PELOG3(limLog(pMac, LOG3, FL("MeasurementIndication timer initialized, period = %d\n"), 
                                                    pMac->lim.gpLimMeasReq->measIndPeriod);)

#if defined(ANI_OS_TYPE_RTAI_LINUX)
    tx_timer_set_expiry_list(
             &pMac->lim.gLimMeasParams.measurementIndTimer,
             LIM_TIMER_EXPIRY_LIST);
#endif
    }

    tANI_U32 learnInterval =
            pMac->lim.gpLimMeasReq->measDuration.shortTermPeriod /
            pMac->lim.gpLimMeasReq->channelList.numChannels;
    if (tx_timer_create(&pMac->lim.gLimMeasParams.learnIntervalTimer,
                        "Learn interval TIMEOUT",
                        limTimerHandler,
                        SIR_LIM_LEARN_INTERVAL_TIMEOUT,
                        SYS_MS_TO_TICKS(learnInterval),
                        0,
                        TX_NO_ACTIVATE) != TX_SUCCESS)
    {
        //                                       
        //          
        limLog(pMac, LOGP, FL("call to create learnInterval timer failed\n"));

        return eSIR_SYS_TX_TIMER_CREATE_FAILED;
    }

#if defined(ANI_OS_TYPE_RTAI_LINUX)
    tx_timer_set_expiry_list(
             &pMac->lim.gLimMeasParams.learnIntervalTimer,
             LIM_TIMER_EXPIRY_LIST);
#endif

    val = SYS_MS_TO_TICKS(pMac->lim.gpLimMeasReq->measDuration.shortChannelScanDuration);
    if (tx_timer_create(
          &pMac->lim.gLimMeasParams.learnDurationTimer,
          "Learn duration TIMEOUT",
          limTimerHandler,
          SIR_LIM_LEARN_DURATION_TIMEOUT,
          val,
          0,
          TX_NO_ACTIVATE) != TX_SUCCESS)
    {
        //                                       
        //          
        limLog(pMac, LOGP,
               FL("call to create LearnDuration timer failed\n"));

        return eSIR_SYS_TX_TIMER_CREATE_FAILED;
    }

#if defined(ANI_OS_TYPE_RTAI_LINUX)
    tx_timer_set_expiry_list(
             &pMac->lim.gLimMeasParams.learnDurationTimer,
             LIM_TIMER_EXPIRY_LIST);
#endif

    #if 0
    if (wlan_cfgGetInt(pMac, WNI_CFG_BEACON_INTERVAL, &beaconInterval) != eSIR_SUCCESS)
    {
        limLog(pMac, LOGP, FL("Can't read beacon interval\n"));
        return eSIR_FAILURE;
    }
    #endif //                  

     /*                                             */
     beaconInterval = psessionEntry->beaconParams.beaconInterval;
   
    if ((learnInterval > ( 2 * beaconInterval)) &&
            (pMac->lim.gLimSystemRole == eLIM_AP_ROLE))
    { 
        //                                            
        val = SYS_MS_TO_TICKS(learnInterval - (2 * beaconInterval));
    //                       
    if( TX_SUCCESS !=
        tx_timer_create( &pMac->lim.limTimers.gLimQuietBssTimer,
          "QUIET BSS TIMER",
          limQuietBssTimerHandler,
          SIR_LIM_QUIET_BSS_TIMEOUT,
            val, //              
          0, //                 
          TX_NO_ACTIVATE ))
    {
      limLog( pMac, LOGP,
          FL( "Failed to create gLimQuietBssTimer!\n" ));
      return eSIR_SYS_TX_TIMER_CREATE_FAILED;
    }

#if defined(ANI_OS_TYPE_RTAI_LINUX)
    tx_timer_set_expiry_list(
        &pMac->lim.limTimers.gLimQuietBssTimer,
        LIM_TIMER_EXPIRY_LIST );
#endif
        pMac->lim.gLimSpecMgmt.fQuietEnabled = eANI_BOOLEAN_TRUE;
    }

    pMac->lim.gLimMeasParams.shortDurationCount = 0;
    pMac->lim.gLimMeasParams.nextLearnChannelId = 0;

    pMac->lim.gLimMeasParams.rssiAlpha =
           (100 * pMac->lim.gpLimMeasReq->measDuration.shortTermPeriod) /
            pMac->lim.gpLimMeasReq->measDuration.averagingPeriod;

    if (!pMac->lim.gLimMeasParams.rssiAlpha)
        pMac->lim.gLimMeasParams.rssiAlpha = 1;

    pMac->lim.gLimMeasParams.chanUtilAlpha =
           (100 * learnInterval) / pMac->lim.gpLimMeasReq->measIndPeriod;

    if (!pMac->lim.gLimMeasParams.chanUtilAlpha)
        pMac->lim.gLimMeasParams.chanUtilAlpha = 1;

    return eSIR_SUCCESS;
} /*                                */


/*                                                
                               
                                                
                        
            
                  
            
                                                 */
static void
__limFreeMeasAndSendRsp(tpAniSirGlobal pMac, tSirResultCodes resultCode)
{
    if (pMac->lim.gpLimMeasReq != NULL)
    {
        palFreeMemory( pMac->hHdd, pMac->lim.gpLimMeasReq);
        pMac->lim.gpLimMeasReq  = NULL;
    }

    if (pMac->lim.gpLimMeasData != NULL)
    {
        palFreeMemory( pMac->hHdd, pMac->lim.gpLimMeasData);
        pMac->lim.gpLimMeasData = NULL;
    }

    //                              
    limSendSmeRsp(pMac, eWNI_SME_MEASUREMENT_RSP, resultCode);
}

/* 
                                
  
           
                                                          
                                          
  
        
  
              
  
       
  
                                                    
                                                        
  
               
 */

static void
limProcessSmeMeasurementReq(tpAniSirGlobal pMac, tANI_U32 *pMsgBuf)
{
    PELOG1(limLog(pMac, LOG1, FL("SME State = %d\n"), pMac->lim.gLimSmeState);)
    switch (pMac->lim.gLimSmeState)
    {
        case eLIM_SME_OFFLINE_STATE:
        case eLIM_SME_IDLE_STATE:
        case eLIM_SME_JOIN_FAILURE_STATE:
        case eLIM_SME_NORMAL_STATE:
        case eLIM_SME_LINK_EST_STATE:
        case eLIM_SME_CHANNEL_SCAN_STATE:
        case eLIM_SME_NORMAL_CHANNEL_SCAN_STATE:
            break;

        default:
            limLog(pMac, LOGE,
               FL("Received unexpected MeasReq message in state %X\n"),
               pMac->lim.gLimSmeState);

            //                               
            limSendSmeRsp(
                       pMac,
                       eWNI_SME_MEASUREMENT_RSP,
                       eSIR_SME_UNEXPECTED_REQ_RESULT_CODE);
            return;
    } //                                    

    if (pMac->lim.gpLimMeasReq)
    {
        //                                             
        //                                              
        limDeleteMeasTimers(pMac);
        limCleanupMeasData(pMac);
    }
    else
    {
        //                                              
        //                                               
        if (eHAL_STATUS_SUCCESS !=
            palAllocateMemory(pMac->hHdd,
                              (void **)&pMac->lim.gpLimMeasData,
                              sizeof(tLimMeasData)))
        {
            //          
            limLog(pMac, LOGE,
                   FL("memory allocate failed for MeasData\n"));

            //                               
            limSendSmeRsp(pMac, eWNI_SME_MEASUREMENT_RSP,
                          eSIR_SME_RESOURCES_UNAVAILABLE);
            return;
        }

        palZeroMemory(pMac->hHdd, (void *)pMac->lim.gpLimMeasData,
                      sizeof(tLimMeasData));
        pMac->lim.gpLimMeasData->duration = 120;
    }

    if (eHAL_STATUS_SUCCESS !=
        palAllocateMemory(pMac->hHdd,
                          (void **)&pMac->lim.gpLimMeasReq,
                          (sizeof(tSirSmeMeasurementReq) +
                           SIR_MAX_NUM_CHANNELS)))
    {
        //          
        PELOGE(limLog(pMac, LOGE, FL("memory allocate failed for MeasReq\n"));)

        __limFreeMeasAndSendRsp(pMac, eSIR_SME_RESOURCES_UNAVAILABLE);
        return;
    }

    if ((limMeasurementReqSerDes(
                          pMac,
                          pMac->lim.gpLimMeasReq,
                          (tANI_U8 *) pMsgBuf) == eSIR_FAILURE) ||
        !limIsSmeMeasurementReqValid(pMac,pMac->lim.gpLimMeasReq))
    {
        limLog(pMac, LOGE,
               FL("Rx'ed MeasReq message with invalid parameters\n"));

        __limFreeMeasAndSendRsp(pMac, eSIR_SME_INVALID_PARAMETERS);
        return;
    }
#ifdef ANI_AP_SDK
    /*                                 */
    limConvertScanDuration(pMac);
#endif /*            */

    //                                         
    if (limInitMeasResources(pMac) != eSIR_SUCCESS)
    {
        __limFreeMeasAndSendRsp(pMac, eSIR_SME_RESOURCES_UNAVAILABLE);
        return;
    }

   PELOG3(limLog(pMac, LOG3,
       FL("NumChannels=%d, shortDuration=%d, shortInterval=%d, longInterval=%d\n"),
       pMac->lim.gpLimMeasReq->channelList.numChannels,
       pMac->lim.gpLimMeasReq->measDuration.shortTermPeriod,
       pMac->lim.gpLimMeasReq->measDuration.shortChannelScanDuration,
       pMac->lim.gpLimMeasReq->measDuration.longChannelScanDuration);)

    limRadarInit(pMac);

    /* 
                                         
                                      
                       
     */
    limReEnableLearnMode(pMac);

    //                                                
    limSendSmeRsp(pMac, eWNI_SME_MEASUREMENT_RSP,
                  eSIR_SME_SUCCESS);
    PELOG2(limLog(pMac, LOG2, FL("Sending succes response to SME\n"));)
    
    if (pMac->lim.gpLimMeasReq->channelList.numChannels == 1)
        limLog(pMac, LOGE, FL("Starting Channel Availability Check on Channel %d... Wait\n"),
                *pMac->lim.gpLimMeasReq->channelList.channelNumber);
} /*                                       */


/* 
                               
  
           
                                                          
                                           
  
        
  
              
  
       
  
                                                    
                                                        
  
               
 */

static void
limProcessSmeSetWdsInfoReq(tpAniSirGlobal pMac, tANI_U32 *pMsgBuf)
{
    tANI_U16                     i;
    tSirSmeSetWdsInfoReq    wdsInfoReq;

    pMac->lim.gLimNumWdsInfoSet++;

    switch (pMac->lim.gLimSmeState)
    {
        case eLIM_SME_NORMAL_STATE:
            break;

        default:
            limLog(pMac, LOGE,
               FL("Rx'ed unexp SetWdsInfoReq message in state %X\n"),
               pMac->lim.gLimSmeState);

            //                               
            limSendSmeRsp(
                       pMac,
                       eWNI_SME_SET_WDS_INFO_RSP,
                       eSIR_SME_UNEXPECTED_REQ_RESULT_CODE);
            return;
    } //                                    

    if ((limWdsReqSerDes( pMac,
                                                  &wdsInfoReq,
                          (tANI_U8 *) pMsgBuf) == eSIR_FAILURE))
    {
        limLog(pMac, LOGW,
           FL("Rx'ed SetWdsInfoReq message with invalid parameters\n"));

        //                              
        limSendSmeRsp(pMac, eWNI_SME_SET_WDS_INFO_RSP,
                      eSIR_SME_INVALID_PARAMETERS);

        return;
    }

    //                                                  
    if ((wdsInfoReq.wdsInfo.wdsLength ==
        psessionEntry->pLimStartBssReq->wdsInfo.wdsLength) &&
        (palEqualMemory( pMac->hHdd,wdsInfoReq.wdsInfo.wdsBytes,
                   psessionEntry->pLimStartBssReq->wdsInfo.wdsBytes,
                   psessionEntry->pLimStartBssReq->wdsInfo.wdsLength) ) )
    {
        //                              
        limSendSmeRsp(pMac,
                      eWNI_SME_SET_WDS_INFO_RSP,
                      eSIR_SME_SUCCESS);

        return;
    }

    //              
    psessionEntry->pLimStartBssReq->wdsInfo.wdsLength =
            wdsInfoReq.wdsInfo.wdsLength;
    for (i=0; i<wdsInfoReq.wdsInfo.wdsLength; i++)
        psessionEntry->pLimStartBssReq->wdsInfo.wdsBytes[i] =
            wdsInfoReq.wdsInfo.wdsBytes[i];

    schSetFixedBeaconFields(pMac,psessionEntry);

    //                              
    limSendSmeRsp(pMac, eWNI_SME_SET_WDS_INFO_RSP,
                  eSIR_SME_SUCCESS);

} /*                                       */


/* 
                                   
  
           
                                                          
                                    
  
        
  
              
  
       
  
                                                    
                                                        
  
               
 */

static void
limProcessLearnDurationTimeout(tpAniSirGlobal pMac, tANI_U32 *pMsgBuf)
{
    if ( pMac->lim.gLimHalScanState == eLIM_HAL_IDLE_SCAN_STATE)
    {
        limSendHalInitScanReq(pMac, eLIM_HAL_INIT_LEARN_WAIT_STATE, eSIR_DONT_CHECK_LINK_TRAFFIC_BEFORE_SCAN );
        return;
    }

    //                                
    if (pMac->lim.gLimMeasParams.nextLearnChannelId ==
        pMac->lim.gpLimMeasReq->channelList.numChannels - 1)
    {
        //                                                       
        //                                                               
        peSetResumeChannel(pMac, 0, 0);
        //                                   
        limSendHalFinishScanReq(pMac, eLIM_HAL_FINISH_LEARN_WAIT_STATE);
    }
    else
    {
        pMac->lim.gLimMeasParams.nextLearnChannelId++;

        if (pMac->lim.gLimSystemRole == eLIM_UNKNOWN_ROLE)
        {
            //                                 
            //                                                
            //                                   
            limSendHalEndScanReq(pMac, (tANI_U8)pMac->lim.gLimMeasParams.nextLearnChannelId, 
                                 eLIM_HAL_END_LEARN_WAIT_STATE);
        }
        else
        {
            //                                                       
            //                                                               
            peSetResumeChannel(pMac, 0, 0);
            //                                      
            limSendHalFinishScanReq(pMac, eLIM_HAL_FINISH_LEARN_WAIT_STATE);
        }
    }
} /*                                          */

/* 
                                   
  
           
                                    
                                                     
  
        
  
              
  
       
  
                                                    
  
               
 */

void
limProcessLearnIntervalTimeout(tpAniSirGlobal pMac)
{
    
#ifdef GEN6_TODO
    //                                             
    //                 
    tpPESession sessionEntry;

    if((sessionEntry = peFindSessionBySessionId(pMac, pMac->lim.gLimMeasParams.learnIntervalTimer.sessionId))== NULL) 
    {
        limLog(pMac, LOGP,FL("Session Does not exist for given sessionID\n"));
        return;
    }
#endif

    PELOG2(limLog(pMac, LOG2, FL("SME state = %d\n"), pMac->lim.gLimSmeState);)
    if (!pMac->sys.gSysEnableLearnMode)
    {
        PELOG3(limLog(pMac, LOG3,
                      FL("Ignoring LEARN_INTERVAL_TIMEOUT because gSysEnableLearnMode is disabled...\n"));)
        limReEnableLearnMode(pMac);
        return;
    }

    if (pMac->lim.gLimSystemInScanLearnMode)
    {
      limLog(pMac, LOGE,
          FL("Sending START_SCAN from LIM while one req is pending\n"));
      return;
    }

    pMac->lim.gLimPrevSmeState = pMac->lim.gLimSmeState;
    if ((pMac->lim.gLimSmeState == eLIM_SME_OFFLINE_STATE) ||
        (pMac->lim.gLimSmeState == eLIM_SME_IDLE_STATE) ||
        (pMac->lim.gLimSmeState == eLIM_SME_JOIN_FAILURE_STATE))
        pMac->lim.gLimSmeState = eLIM_SME_CHANNEL_SCAN_STATE;
    else if (pMac->lim.gLimSmeState == eLIM_SME_NORMAL_STATE)
        pMac->lim.gLimSmeState = eLIM_SME_NORMAL_CHANNEL_SCAN_STATE;
    else if (pMac->lim.gLimSmeState == eLIM_SME_LINK_EST_STATE)
        pMac->lim.gLimSmeState = eLIM_SME_LINK_EST_WT_SCAN_STATE;
    else
        return;
    MTRACE(macTrace(pMac, TRACE_CODE_SME_STATE, NO_SESSION, pMac->lim.gLimSmeState));

    /*                                                                                       
                                                                                               
                                                                                                     
                                                                                                     
                                                                                                           
     */
//                                                  
    {
        pMac->lim.gLimPrevMlmState = pMac->lim.gLimMlmState;
        pMac->lim.gLimMlmState     = eLIM_MLM_LEARN_STATE;
        MTRACE(macTrace(pMac, TRACE_CODE_MLM_STATE, NO_SESSION, pMac->lim.gLimMlmState));
        pMac->lim.gLimSystemInScanLearnMode = eANI_BOOLEAN_TRUE;
    }
#if 0
    /* 
                                               
     */
    if (pMac->lim.gLimSystemRole == eLIM_STA_ROLE)
    {
        tLimMlmScanReq     *pMlmScanReq;
        tANI_U32            len;
        
        if( eHAL_STATUS_SUCCESS != palAllocateMemory( pMac->hHdd, (void **)&pMlmScanReq,
                                        (sizeof(tLimMlmScanReq) + WNI_CFG_VALID_CHANNEL_LIST_LEN)))
        {
            limLog(pMac, LOGP,
                FL("call to palAllocateMemory failed for mlmScanReq\n"));
            return;
        }
        palZeroMemory( pMac->hHdd, (tANI_U8 *) pMlmScanReq,
                              (tANI_U32)(sizeof(tLimMlmScanReq) + WNI_CFG_VALID_CHANNEL_LIST_LEN ));

        len = WNI_CFG_VALID_CHANNEL_LIST_LEN;
        if (wlan_cfgGetStr(pMac, WNI_CFG_VALID_CHANNEL_LIST,
                        pMlmScanReq->channelList.channelNumber,
                        &len) != eSIR_SUCCESS)
        {
            limLog(pMac, LOGP,
                  FL("could not retrieve Valid channel list\n"));
        }
        pMlmScanReq->channelList.numChannels = (tANI_U8) len;
        
        palFillMemory(pMac->hHdd, &pMlmScanReq->bssId, sizeof(tSirMacAddr), 0xff);
        pMlmScanReq->bssType = eSIR_AUTO_MODE;
        pMlmScanReq->scanType = eSIR_ACTIVE_SCAN;
        pMlmScanReq->backgroundScanMode = 0;
        pMlmScanReq->maxChannelTime = 40;
        pMlmScanReq->minChannelTime = 20;
        limPostMlmMessage(pMac, LIM_MLM_SCAN_REQ, (tANI_U32 *) pMlmScanReq);
    }
    else
#endif        
        limSetLearnMode(pMac);
} 


/* 
                          
  
           
                                                            
                                                           
                            
  
        
                                                                
          
  
              
  
       
  
                                                    
                                                   
                                                        
  
               
 */

void
limProcessLmmMessages(tpAniSirGlobal pMac, tANI_U32 msgType, tANI_U32 *pMsgBuf)
{
    switch (msgType)
    {
        case eWNI_SME_MEASUREMENT_REQ:
            PELOG1(limLog(pMac, LOG1, FL("Received MEASUREMENT_REQ message\n"));)
            limProcessSmeMeasurementReq(pMac, pMsgBuf);

            break;

        case eWNI_SME_SET_WDS_INFO_REQ:

            limProcessSmeSetWdsInfoReq(pMac, pMsgBuf);

            break;

        case SIR_LIM_MEASUREMENT_IND_TIMEOUT:
            //                                           
            limSendSmeMeasurementInd(pMac);

            break;

        case SIR_LIM_LEARN_INTERVAL_TIMEOUT:
            limProcessLearnIntervalTimeout(pMac);
            break;

        case SIR_LIM_LEARN_DURATION_TIMEOUT:
            limProcessLearnDurationTimeout(pMac, pMsgBuf);

            break;

        default:

            break;
    } //                 

    return;
} /*                                 */

#endif
