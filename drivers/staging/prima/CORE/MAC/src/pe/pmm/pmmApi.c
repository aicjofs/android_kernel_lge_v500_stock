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
#include "wniCfgSta.h"

#include "sirCommon.h"
#include "aniGlobal.h"

#include "schApi.h"
#include "limApi.h"
#include "limSendMessages.h"
#include "cfgApi.h"
#include "limSessionUtils.h"
#include "limFT.h"


#include "pmmApi.h"
#include "pmmDebug.h"
#include "sirApi.h"
#include "wmmApsd.h"

#include "limSendSmeRspMessages.h"
#include "limTimerUtils.h"
#include "limTrace.h"
#include "limUtils.h"
#include "VossWrapper.h"
#ifdef INTEGRATION_READY
#include "vos_status.h" //          
#include "vos_mq.h"     //                     
#endif

#include "wlan_qct_wda.h"

//                                                                     
/* 
                
  
            
                        
  
         
  
               
  
        
  
              
              
               
 */

tSirRetStatus
pmmInitialize(tpAniSirGlobal pMac)
{

#if (WNI_POLARIS_FW_PRODUCT == AP)
    pMac->pmm.gPmmNextSta = 0;
    pMac->pmm.gPmmNumSta = 0;
#endif

    pmmResetStats(pMac);

    pMac->pmm.gPmmBeaconInterval = WNI_CFG_BEACON_INTERVAL_STADEF;
    pMac->pmm.gPmmState = ePMM_STATE_READY;


#ifdef ANI_PRODUCT_TYPE_AP

    palZeroMemory(pMac->hHdd,
                    pMac->pmm.gPmmTim.pTim, sizeof(tANI_U8)*pMac->lim.maxStation);

    palZeroMemory(pMac->hHdd,
                     pMac->pmm.gPmmTim.pStaInfo, sizeof(*pMac->pmm.gPmmTim.pStaInfo)*pMac->lim.maxStation);

    palZeroMemory(pMac->hHdd,
                    pMac->pmm.gpPmmStaState, sizeof(tPmmStaState)*pMac->lim.maxStation);

    palZeroMemory(pMac->hHdd,
                    pMac->pmm.gpPmmPSState, sizeof(tANI_U8)*pMac->lim.maxStation);
#endif

    pMac->pmm.inMissedBeaconScenario = FALSE;

    return eSIR_SUCCESS;
}

//                                                                     
/* 
                
  
            
                        
  
         
  
               
  
        
  
              
  
               
 */

void
pmmResetStats(void *pvMac)
{
    tpAniSirGlobal pMac = (tpAniSirGlobal)pvMac;

    pMac->pmm.BmpsmaxSleepTime = 0;
    pMac->pmm.BmpsavgSleepTime = 0;
    pMac->pmm.BmpsminSleepTime = 0;
    pMac->pmm.BmpscntSleep = 0;

    pMac->pmm.BmpsmaxTimeAwake = 0;
    pMac->pmm.BmpsavgTimeAwake = 0;
    pMac->pmm.BmpsminTimeAwake = 0;
    pMac->pmm.BmpscntAwake = 0;

    pMac->pmm.BmpsWakeupTimeStamp = 0;
    pMac->pmm.BmpsSleepTimeStamp = 0;

    pMac->pmm.BmpsHalReqFailCnt = 0;
    pMac->pmm.BmpsInitFailCnt = 0;
    pMac->pmm.BmpsInitFailCnt= 0;
    pMac->pmm.BmpsInvStateCnt= 0;
    pMac->pmm.BmpsPktDrpInSleepMode= 0;
    pMac->pmm.BmpsReqInInvalidRoleCnt= 0;
    pMac->pmm.BmpsSleeReqFailCnt= 0;
    pMac->pmm.BmpsWakeupIndCnt= 0;

    pMac->pmm.ImpsWakeupTimeStamp = 0;
    pMac->pmm.ImpsSleepTimeStamp = 0;
    pMac->pmm.ImpsMaxTimeAwake = 0;
    pMac->pmm.ImpsMinTimeAwake = 0;
    pMac->pmm.ImpsAvgTimeAwake = 0;
    pMac->pmm.ImpsCntAwake = 0;

    pMac->pmm.ImpsCntSleep = 0;
    pMac->pmm.ImpsMaxSleepTime = 0;
    pMac->pmm.ImpsMinSleepTime = 0;
    pMac->pmm.ImpsAvgSleepTime = 0;

    pMac->pmm.ImpsSleepErrCnt = 0;
    pMac->pmm.ImpsWakeupErrCnt = 0;
    pMac->pmm.ImpsLastErr = 0;
    pMac->pmm.ImpsInvalidStateCnt = 0;

    return;
}



//                                                                     
/* 
                             
  
            
                                                               
                                                                           
                                          
  
         
  
               
  
        
  
         
               
 */

void pmmInitBmpsResponseHandler(tpAniSirGlobal pMac, eHalStatus rspStatus)
{


    tPmmState nextState = pMac->pmm.gPmmState;
    tSirResultCodes retStatus = eSIR_SME_SUCCESS;
    tpPESession     psessionEntry;

    /*                                                            
                                                 
    */
    SET_LIM_PROCESS_DEFD_MESGS(pMac, true);

    if(pMac->pmm.gPmmState != ePMM_STATE_BMPS_WT_INIT_RSP)
    {
        PELOGE(pmmLog(pMac, LOGE,
            FL("pmmBmps: Received 'InitPwrSaveRsp' while in incorrect state: %d\n"),
            pMac->pmm.gPmmState);)

        retStatus = eSIR_SME_INVALID_PMM_STATE;
        pmmBmpsUpdateInvalidStateCnt(pMac);
        goto failure;
    }

    //                                                                                       
    //                                                                               
    if(rspStatus == eHAL_STATUS_SUCCESS)
    {
        PELOG2(pmmLog(pMac, LOG2,
            FL("pmmBmps: Received successful response from HAL to enter BMPS_POWER_SAVE \n"));)

        pMac->pmm.gPmmState = ePMM_STATE_BMPS_SLEEP;

        //                        
        pmmUpdatePwrSaveStats(pMac);

        //                             
        pMac->sys.gSysEnableScanMode = false;

        if (pMac->lim.gLimTimersCreated)
        {
            /*                                 */
            if(pMac->lim.limTimers.gLimHeartBeatTimer.pMac)
            {
                MTRACE(macTrace(pMac, TRACE_CODE_TIMER_DEACTIVATE, NO_SESSION, eLIM_HEART_BEAT_TIMER));
                tx_timer_deactivate(&pMac->lim.limTimers.gLimHeartBeatTimer);
            }
        }
        limSendSmeRsp(pMac, eWNI_PMC_ENTER_BMPS_RSP,  retStatus, 0, 0);
    }
    else
    {
        //                                                 
        PELOGE(pmmLog(pMac, LOGE,
            FL("pmmBmps: BMPS_INIT_PWR_SAVE_REQ failed, informing SME\n"));)

        pmmBmpsUpdateInitFailureCnt(pMac);
        nextState = ePMM_STATE_BMPS_WAKEUP;
        retStatus = eSIR_SME_BMPS_REQ_FAILED;
        goto failure;
    }
    return;

failure:
    psessionEntry = peGetValidPowerSaveSession(pMac);
    if(psessionEntry != NULL)
    {
       if (pMac->lim.gLimTimersCreated && pMac->lim.limTimers.gLimHeartBeatTimer.pMac)
       {
           if(VOS_TRUE != tx_timer_running(&pMac->lim.limTimers.gLimHeartBeatTimer))
           {
               PELOGE(pmmLog(pMac, LOGE, FL("Unexpected heartbeat timer not running"));)
               limReactivateHeartBeatTimer(pMac, psessionEntry);
           }
       }
    }

    //                                      
    pMac->pmm.gPmmState = nextState;
    pmmBmpsUpdateSleepReqFailureCnt(pMac);
    limSendSmeRsp(pMac, eWNI_PMC_ENTER_BMPS_RSP, retStatus, 0, 0);
    return;

}

//                                                                     
/* 
                            
  
            
                                                    
  
         
  
               
  
        
  
                                                

               
 */

void pmmExitBmpsRequestHandler(tpAniSirGlobal pMac, tpExitBmpsInfo pExitBmpsInfo)
{
    tSirResultCodes respStatus = eSIR_SME_SUCCESS;

    tPmmState origState = pMac->pmm.gPmmState;

#ifdef FEATURE_WLAN_DIAG_SUPPORT 
    limDiagEventReport(pMac, WLAN_PE_DIAG_EXIT_BMPS_REQ_EVENT, peGetValidPowerSaveSession(pMac), 0, (tANI_U16)pExitBmpsInfo->exitBmpsReason);
#endif //                         

    if (NULL == pExitBmpsInfo)
    {
        respStatus = eSIR_SME_BMPS_REQ_REJECT;
        PELOGW(pmmLog(pMac, LOGW, FL("pmmBmps: Rcvd EXIT_BMPS with NULL body\n"));)
        goto failure;
    }

    /*                                                      
                                                                
                                                            
                                                                     
                                                                    
     */
    if(limIsSystemInScanState(pMac))
    {
        PELOGW(pmmLog(pMac, LOGW, 
            FL("pmmBmps: Device is already awake and scanning, returning success to PMC \n"));)
        limSendSmeRsp(pMac, eWNI_PMC_EXIT_BMPS_RSP, respStatus, 0, 0);
        return;
    }

    /*                                               */
    PELOGW(pmmLog(pMac, LOGW, FL("pmmBmps: Sending eWNI_PMC_EXIT_BMPS_REQ to HAL\n"));)
    if (pMac->pmm.gPmmState == ePMM_STATE_BMPS_SLEEP)
    {
        /*                                                           
                                                                   
         */
        pMac->pmm.gPmmExitBmpsReasonCode = pExitBmpsInfo->exitBmpsReason;
        palFreeMemory( pMac->hHdd, (tANI_U8 *) pExitBmpsInfo);

        PELOGW(pmmLog(pMac, LOGW, 
            FL("pmmBmps: Rcvd EXIT_BMPS with reason code%d \n"), pMac->pmm.gPmmExitBmpsReasonCode);)


        //                                    
        pMac->pmm.gPmmState = ePMM_STATE_BMPS_WT_WAKEUP_RSP;
        if(pmmSendChangePowerSaveMsg(pMac) !=  eSIR_SUCCESS)
        {
            /*                       */
            respStatus = eSIR_SME_BMPS_REQ_REJECT;
            pmmBmpsUpdateHalReqFailureCnt(pMac);
            goto failure;
        }
        else
        {
            PELOG1(pmmLog(pMac, LOG1,
                          FL("pmmBmps: eWNI_PMC_EXIT_BMPS_REQ was successfully sent to HAL\n"));)
        }
    }
    else
    {
        PELOGE(pmmLog(pMac, LOGE, 
                      FL("pmmBmps: eWNI_PMC_EXIT_BMPS_REQ received in invalid state: %d\n"),
            pMac->pmm.gPmmState );)

        respStatus = eSIR_SME_INVALID_PMM_STATE;
        pmmBmpsUpdateInvalidStateCnt(pMac);
        goto failure;
    }
    return;

failure:
    pMac->pmm.gPmmState = origState;
    pmmBmpsUpdateWakeupReqFailureCnt(pMac);
    limSendSmeRsp(pMac, eWNI_PMC_EXIT_BMPS_RSP, respStatus, 0, 0);
}


//                                                                     
/* 
                     
  
            
                                                             
                                                           
                                                     
                                                                   
          
  
         
  
               
  
        
  
                                                             
                                                

               
 */


void pmmInitBmpsPwrSave(tpAniSirGlobal pMac)
{
    tSirRetStatus   retStatus = eSIR_SUCCESS;
    tSirResultCodes respStatus = eSIR_SME_SUCCESS;
    tpPESession     psessionEntry;

    tPmmState origState = pMac->pmm.gPmmState;

    if((psessionEntry = peGetValidPowerSaveSession(pMac))== NULL)
    {
        respStatus = eSIR_SME_BMPS_REQ_REJECT;
        goto failure;
    }
#ifdef FEATURE_WLAN_DIAG_SUPPORT 
    limDiagEventReport(pMac, WLAN_PE_DIAG_ENTER_BMPS_REQ_EVENT, psessionEntry, 0, 0);
#endif //                         

    if ( ((pMac->pmm.gPmmState != ePMM_STATE_READY) &&
         (pMac->pmm.gPmmState != ePMM_STATE_BMPS_WAKEUP)) ||
         limIsSystemInScanState(pMac) ||
         limIsChanSwitchRunning(pMac) ||
         limIsInQuietDuration(pMac) )
    {
        PELOGE(pmmLog(pMac, LOGE, 
            FL("pmmBmps: BMPS Request received in invalid state PMM=%d, SME=%d, rejecting the initpwrsave request\n"), 
            pMac->pmm.gPmmState, pMac->lim.gLimSmeState);)

        respStatus = eSIR_SME_INVALID_PMM_STATE;
        pmmBmpsUpdateInvalidStateCnt(pMac);
        goto failure;
    }

    //                                                                                                               
    if(pMac->pmm.inMissedBeaconScenario)
    {
       if (pMac->lim.gLimTimersCreated && pMac->lim.limTimers.gLimHeartBeatTimer.pMac)
       {
           if(VOS_TRUE != tx_timer_running(&pMac->lim.limTimers.gLimHeartBeatTimer))
           {
               PELOGE(pmmLog(pMac, LOGE, FL("Unexpected heartbeat timer not running"));)
               limReactivateHeartBeatTimer(pMac, psessionEntry);
           }
       }
       respStatus = eSIR_SME_BMPS_REQ_REJECT;
       goto failure;
    }

    /*                                                                         
                                                                           
                                                        
     */
     /*                                                                */
#if 0     
    if ( (pMac->sys.gSysEnableLinkMonitorMode) && (pMac->lim.limTimers.gLimHeartBeatTimer.pMac) )
    {
        if(VOS_TRUE != tx_timer_running(&pMac->lim.limTimers.gLimHeartBeatTimer)) 
        {
            PELOGE(pmmLog(pMac, LOGE, 
                FL("Reject BMPS_REQ because HeartBeatTimer is not running. \n"));)
            respStatus = eSIR_SME_BMPS_REQ_FAILED;
            goto failure;
        }
    }
#endif

    //                                                                             
    //                              

    //                                                                                      
    pMac->pmm.gPmmState = ePMM_STATE_BMPS_WT_INIT_RSP;
    if((retStatus = pmmSendInitPowerSaveMsg(pMac,psessionEntry)) != eSIR_SUCCESS)
    {
        PELOGE(pmmLog(pMac, LOGE, 
            FL("pmmBmps: Init Power Save Request Failed: Sending Response: %d\n"), 
            retStatus);)

        respStatus = eSIR_SME_BMPS_REQ_REJECT;
        pmmBmpsUpdateHalReqFailureCnt(pMac);
        goto failure;
    }
    //                              
    pMac->pmm.sessionId = psessionEntry->peSessionId;
    return;

failure:

    //                                        
    pMac->pmm.gPmmState =origState;
    limSendSmeRsp(pMac, eWNI_PMC_ENTER_BMPS_RSP, respStatus, 0, 0);
    return;
}


/* 
                              
  
           
                                                                
                                                          
  
        
  
              
     
  
       
     
  
                                                
                                                     
 */
tSirRetStatus pmmSendChangePowerSaveMsg(tpAniSirGlobal pMac)
{
    tSirRetStatus  retStatus = eSIR_SUCCESS;
    tpExitBmpsParams  pExitBmpsParams;
    tSirMsgQ msgQ;
    tpPESession psessionEntry;
    tANI_U8  currentOperatingChannel = limGetCurrentOperatingChannel(pMac);

    if( eHAL_STATUS_SUCCESS != palAllocateMemory( pMac->hHdd, (void **)&pExitBmpsParams, sizeof(*pExitBmpsParams)) )
    {
        pmmLog(pMac, LOGW, FL("palAllocateMemory() failed\n"));
        retStatus = eSIR_MEM_ALLOC_FAILED;
        return retStatus;
    }

    if((psessionEntry = peGetValidPowerSaveSession(pMac)) == NULL )
    {
        retStatus = eSIR_FAILURE;
        return retStatus;
    }

    palZeroMemory( pMac->hHdd, (tANI_U8 *)pExitBmpsParams, sizeof(*pExitBmpsParams));
    msgQ.type = WDA_EXIT_BMPS_REQ;
    msgQ.reserved = 0;
    msgQ.bodyptr = pExitBmpsParams;
    msgQ.bodyval = 0;

    /*                                                       
                                                                          
                                                           
                                         
                                                 
     */
    if ( !(SIR_IS_FULL_POWER_REASON_DISCONNECTED(pMac->pmm.gPmmExitBmpsReasonCode) ||
          ( (eSME_MISSED_BEACON_IND_RCVD == pMac->pmm.gPmmExitBmpsReasonCode) && 
             limIsconnectedOnDFSChannel(currentOperatingChannel))))
        pExitBmpsParams->sendDataNull = 1;

    pExitBmpsParams->bssIdx = psessionEntry->bssIdx;
   
    /*                                                
                                        
     */
    SET_LIM_PROCESS_DEFD_MESGS(pMac, false);
    MTRACE(macTraceMsgTx(pMac, NO_SESSION, msgQ.type));
    retStatus = wdaPostCtrlMsg( pMac, &msgQ);
    if( eSIR_SUCCESS != retStatus )
    {
        PELOGE(pmmLog( pMac, LOGE, FL("Sending WDA_EXIT_BMPS_REQ failed, reason=%X \n"), retStatus );)
        palFreeMemory(pMac->hHdd, (tANI_U8*)pExitBmpsParams);
        return retStatus;
    }

    PELOG1(pmmLog(pMac, LOG1, FL("WDA_EXIT_BMPS_REQ has been successfully sent to HAL\n"));)
    return retStatus;
}


/* 
                            
  
           
                                                                 
                                                                
                                 
  
        
  
              
     
  
       
     
  
                                                
                                             
  
                                                     
 */

tSirRetStatus  pmmSendInitPowerSaveMsg(tpAniSirGlobal pMac,tpPESession psessionEntry)
{
    tSirRetStatus   retCode = eSIR_SUCCESS;
    tSirMsgQ msgQ;
    tpEnterBmpsParams pBmpsParams = NULL;
    int         i=0;

    tANI_U32    rssiFilterPeriod = 5;
    tANI_U32    numBeaconPerRssiAverage = 5;
    tANI_U32    bRssiFilterEnable = FALSE;

    if(psessionEntry->currentBssBeaconCnt == 0)
    {
        PELOGE(pmmLog( pMac, LOGE,  FL("Beacon count is zero, can not retrieve the TSF, failing the Enter Bmps Request\n"));)
        return eSIR_FAILURE;
    }

    if (palAllocateMemory(pMac->hHdd, (void **)&pBmpsParams, sizeof(tEnterBmpsParams)) != eHAL_STATUS_SUCCESS)
    {
        pmmLog(pMac, LOGP, "PMM: Not able to allocate memory for Enter Bmps\n");
        return eSIR_FAILURE;
    }

    pMac->pmm.inMissedBeaconScenario = FALSE;
    pBmpsParams->respReqd = TRUE;

    pBmpsParams->tbtt = psessionEntry->lastBeaconTimeStamp;
    pBmpsParams->dtimCount = psessionEntry->lastBeaconDtimCount;
    pBmpsParams->dtimPeriod = psessionEntry->lastBeaconDtimPeriod;
    pBmpsParams->bssIdx = psessionEntry->bssIdx;

    if(wlan_cfgGetInt(pMac, WNI_CFG_RSSI_FILTER_PERIOD, &rssiFilterPeriod) != eSIR_SUCCESS)
        pmmLog(pMac, LOGP, FL("pmmCfg: cfgGet failed for Rssi filter period"));

    //                                                                       
    if(wlan_cfgGetInt(pMac, WNI_CFG_PS_ENABLE_RSSI_MONITOR, &bRssiFilterEnable) != eSIR_SUCCESS)
        pmmLog(pMac, LOGP, FL("pmmCfg: cfgGet failed for Rssi monitor enable flag"));
    pBmpsParams->bRssiFilterEnable = bRssiFilterEnable;


#if  defined (WLAN_FEATURE_VOWIFI_11R) || defined (FEATURE_WLAN_CCX) || defined(FEATURE_WLAN_LFR)
    //                                                                                      
    //                 
    for(i =0; i < pMac->lim.maxBssId; i++)
    {
        if (limisFastTransitionRequired(pMac, i))
        {
            if(wlan_cfgGetInt(pMac, WNI_CFG_FT_RSSI_FILTER_PERIOD, &rssiFilterPeriod) != eSIR_SUCCESS)
                pmmLog(pMac, LOGP, FL("pmmCfg: cfgGet failed for Rssi filter period"));
            //                                             
            //                                                        
            //                               

            pBmpsParams->bRssiFilterEnable = TRUE;
            break;
        }
    }
#endif

    pBmpsParams->rssiFilterPeriod = (tANI_U8)rssiFilterPeriod;

    /*                                                            
                                                               
     */
    if(wlan_cfgGetInt(pMac, WNI_CFG_NUM_BEACON_PER_RSSI_AVERAGE, &numBeaconPerRssiAverage) != eSIR_SUCCESS)
        pmmLog(pMac, LOGP, FL("pmmCfg: cfgGet failed for num beacon per rssi"));

    pBmpsParams->numBeaconPerRssiAverage = (tANI_U8) numBeaconPerRssiAverage;
    if (numBeaconPerRssiAverage > rssiFilterPeriod)
        pBmpsParams->numBeaconPerRssiAverage = 
            (tANI_U8)GET_MIN_VALUE(rssiFilterPeriod, WNI_CFG_NUM_BEACON_PER_RSSI_AVERAGE_STAMAX);

    pmmLog (pMac, LOG1,
        "%s: [INFOLOG]RssiFilterInfo..%d %x %x\n", __func__, (int)pBmpsParams->bRssiFilterEnable,
        (unsigned int)pBmpsParams->rssiFilterPeriod, (unsigned int)pBmpsParams->numBeaconPerRssiAverage);

    msgQ.type = WDA_ENTER_BMPS_REQ;
    msgQ.reserved = 0;
    msgQ.bodyptr = pBmpsParams;
    msgQ.bodyval = 0;

    PELOG2(pmmLog( pMac, LOG2,
        FL( "pmmBmps: Sending WDA_ENTER_BMPS_REQ" ));)

    /*                                                      
                                   
     */
    SET_LIM_PROCESS_DEFD_MESGS(pMac, false);

    MTRACE(macTraceMsgTx(pMac, psessionEntry->peSessionId, msgQ.type));
    if( eSIR_SUCCESS != (retCode = wdaPostCtrlMsg( pMac, &msgQ )))
    {
        palFreeMemory(pMac->hHdd, pBmpsParams);
        PELOGE(pmmLog( pMac, LOGE,
            FL("Posting WDA_ENTER_BMPS_REQ to HAL failed, reason=%X\n"),
            retCode );)
    }

    return retCode;
}

/* 
                        
  
           
                                                           
  
       
  
                                                
                                             
  
                                                     
 */
tSirRetStatus pmmSendPowerSaveCfg(tpAniSirGlobal pMac, tpSirPowerSaveCfg pUpdatedPwrSaveCfg)
{
    tSirRetStatus   retCode = eSIR_SUCCESS;
    tSirMsgQ    msgQ;
    tANI_U32    listenInterval;
    tANI_U32    HeartBeatCount = 1;
    tANI_U32    maxPsPoll;
    tANI_U32    numBeaconPerRssiAverage;
    tANI_U32    minRssiThreshold;
    tANI_U32    nthBeaconFilter;
    tANI_U32    broadcastFrameFilter;
    tANI_U32    rssiFilterPeriod;
    tANI_U32    ignoreDtim;

    if (NULL == pUpdatedPwrSaveCfg)
        goto returnFailure;

    if(pMac->lim.gLimSmeState != eLIM_SME_IDLE_STATE  )
    {
        pmmLog(pMac, LOGE,
            FL("pmmCfg: Power Save Configuration received in invalid global sme state %d"),
            pMac->lim.gLimSmeState);
        retCode = eSIR_SME_INVALID_STATE;
        goto returnFailure;
    }

    //                                        
    if(wlan_cfgGetInt(pMac, WNI_CFG_LISTEN_INTERVAL, &listenInterval) != eSIR_SUCCESS)
        pmmLog(pMac, LOGP, FL("pmmCfg: cfgGet failed for listen interval"));
    pUpdatedPwrSaveCfg->listenInterval = (tANI_U16)listenInterval;

    if(wlan_cfgGetInt(pMac, WNI_CFG_HEART_BEAT_THRESHOLD, &HeartBeatCount) != eSIR_SUCCESS)
        pmmLog(pMac, LOGP, FL("pmmCfg: cfgGet failed for heart beat thresh"));

    pMac->lim.gLimHeartBeatCount = HeartBeatCount;
    pUpdatedPwrSaveCfg->HeartBeatCount = HeartBeatCount;

    if(wlan_cfgGetInt(pMac, WNI_CFG_NTH_BEACON_FILTER, &nthBeaconFilter) != eSIR_SUCCESS)
        pmmLog(pMac, LOGP, FL("pmmCfg: cfgGet failed for Nth beacon filter"));
    pUpdatedPwrSaveCfg->nthBeaconFilter = nthBeaconFilter;

    if(wlan_cfgGetInt(pMac, WNI_CFG_MAX_PS_POLL, &maxPsPoll) != eSIR_SUCCESS)
        pmmLog(pMac, LOGP, FL("pmmCfg: cfgGet failed for max poll"));
    pUpdatedPwrSaveCfg->maxPsPoll = maxPsPoll;

    if(wlan_cfgGetInt(pMac, WNI_CFG_MIN_RSSI_THRESHOLD, &minRssiThreshold) != eSIR_SUCCESS)
        pmmLog(pMac, LOGP, FL("pmmCfg: cfgGet failed for min RSSI Threshold"));
    pUpdatedPwrSaveCfg->minRssiThreshold = minRssiThreshold;

    if(wlan_cfgGetInt(pMac, WNI_CFG_NUM_BEACON_PER_RSSI_AVERAGE, &numBeaconPerRssiAverage) != eSIR_SUCCESS)
        pmmLog(pMac, LOGP, FL("pmmCfg: cfgGet failed for num beacon per rssi"));
    pUpdatedPwrSaveCfg->numBeaconPerRssiAverage = (tANI_U8) numBeaconPerRssiAverage;

    if(wlan_cfgGetInt(pMac, WNI_CFG_RSSI_FILTER_PERIOD, &rssiFilterPeriod) != eSIR_SUCCESS)
        pmmLog(pMac, LOGP, FL("pmmCfg: cfgGet failed for Rssi filter period"));
    pUpdatedPwrSaveCfg->rssiFilterPeriod = (tANI_U8) rssiFilterPeriod;

    if(wlan_cfgGetInt(pMac, WNI_CFG_BROADCAST_FRAME_FILTER_ENABLE, &broadcastFrameFilter) != eSIR_SUCCESS)
        pmmLog(pMac, LOGP, FL("pmmCfg: cfgGet failed for Nth beacon filter"));
    pUpdatedPwrSaveCfg->broadcastFrameFilter = (tANI_U8) broadcastFrameFilter;

    if(wlan_cfgGetInt(pMac, WNI_CFG_IGNORE_DTIM, &ignoreDtim) != eSIR_SUCCESS)
        pmmLog(pMac, LOGP, FL("pmmCfg: cfgGet failed for ignoreDtim"));
    pUpdatedPwrSaveCfg->ignoreDtim = (tANI_U8) ignoreDtim;

    /*                                                            
                                                               
     */
    if (numBeaconPerRssiAverage > rssiFilterPeriod)
        pUpdatedPwrSaveCfg->numBeaconPerRssiAverage = (tANI_U8)GET_MIN_VALUE(rssiFilterPeriod, WNI_CFG_NUM_BEACON_PER_RSSI_AVERAGE_STAMAX);

    //                                             
    palCopyMemory( pMac->hHdd, (tANI_U8 *) &pMac->pmm.gPmmCfg,  pUpdatedPwrSaveCfg, sizeof(tSirPowerSaveCfg));


    msgQ.type = WDA_PWR_SAVE_CFG;
    msgQ.reserved = 0;
    msgQ.bodyptr = pUpdatedPwrSaveCfg;
    msgQ.bodyval = 0;

    PELOG1(pmmLog( pMac, LOG1, FL( "pmmBmps: Sending WDA_PWR_SAVE_CFG to HAL"));)
    MTRACE(macTraceMsgTx(pMac, NO_SESSION, msgQ.type));
    if( eSIR_SUCCESS != (retCode = wdaPostCtrlMsg( pMac, &msgQ )))
    {
        pmmLog( pMac, LOGP,
            FL("Posting WDA_PWR_SAVE_CFG to HAL failed, reason=%X"),
            retCode );
        goto returnFailure;
    }
    return retCode;

returnFailure:

    /*                                                */
    if (NULL != pUpdatedPwrSaveCfg)
    {
        palFreeMemory( pMac->hHdd, (tANI_U8 *) pUpdatedPwrSaveCfg);
    }
    return retCode;
}

/* 
                             
  
           
                                                                      
                                      
        
  
              
     
  
       
     
  
                                                
                                                                     
  
                
 */
void pmmExitBmpsResponseHandler(tpAniSirGlobal pMac,  tpSirMsgQ limMsg)
{
    tpExitBmpsParams  pExitBmpsRsp;
    eHalStatus  rspStatus;
    tANI_U8 PowersavesessionId;
    tpPESession psessionEntry;
    tSirResultCodes retStatus = eSIR_SME_SUCCESS;
    
    /*                                                     */
    PowersavesessionId = pMac->pmm.sessionId;

    /*                                                            
                                                
     */

    SET_LIM_PROCESS_DEFD_MESGS(pMac, true);

    if((psessionEntry = peFindSessionBySessionId(pMac,PowersavesessionId))==NULL)
    {
        limLog(pMac, LOGP,FL("Session Does not exist for given sessionID\n"));
        return;
    }

    

    /*                          */
    pmmUpdateWakeupStats(pMac);

    if (NULL == limMsg->bodyptr)
    {
        pmmLog(pMac, LOGE, FL("Received SIR_HAL_EXIT_BMPS_RSP with NULL "));
        return;
    }
    pExitBmpsRsp = (tpExitBmpsParams)(limMsg->bodyptr);
    rspStatus = pExitBmpsRsp->status;

    if(pMac->pmm.gPmmState != ePMM_STATE_BMPS_WT_WAKEUP_RSP)
    {
        PELOGE(pmmLog(pMac, LOGE,
            FL("Received SIR_HAL_EXIT_BMPS_RSP while in incorrect state: %d\n"),
            pMac->pmm.gPmmState);)

        retStatus = eSIR_SME_INVALID_PMM_STATE;
        pmmBmpsUpdateInvalidStateCnt(pMac);
    }
    else
    {
        PELOGW(pmmLog(pMac, LOGW, FL("Received SIR_HAL_EXIT_BMPS_RSP in correct state. \n"));)
    }

    /*                                              
                                                  
     */
    switch (rspStatus)
    {
        case eHAL_STATUS_SUCCESS:
            retStatus = eSIR_SME_SUCCESS;
            break;

        default:
            {
                /*                                                              
                                                                                     
                                                             
                 */
                retStatus = eSIR_SME_BMPS_REQ_FAILED;
            }
            break;

    }

    pMac->pmm.gPmmState = ePMM_STATE_BMPS_WAKEUP;
    pmmUpdateWakeupStats(pMac);

    //                        
    pMac->sys.gSysEnableScanMode = true;

    //                     
   if(IS_SLM_SESSIONIZATION_SUPPORTED_BY_FW )
   {
       limSendSmeRsp(pMac, eWNI_PMC_EXIT_BMPS_RSP, retStatus, 
                  psessionEntry->smeSessionId, psessionEntry->transactionId);
   }
   else
   {
       limSendSmeRsp(pMac, eWNI_PMC_EXIT_BMPS_RSP, retStatus, 0, 0);
   }

    if ( pMac->pmm.gPmmExitBmpsReasonCode == eSME_MISSED_BEACON_IND_RCVD)
    {
        PELOGW(pmmLog(pMac, LOGW, FL("Rcvd SIR_HAL_EXIT_BMPS_RSP with MISSED_BEACON\n"));)
        pmmMissedBeaconHandler(pMac);
    }
    else if(pMac->pmm.inMissedBeaconScenario)
    {
        PELOGW(pmmLog(pMac, LOGW, FL("Rcvd SIR_HAL_EXIT_BMPS_RSP in missed beacon scenario but reason code not correct"));)
        pmmMissedBeaconHandler(pMac);
    }
    else
    {
        //                       
        limReactivateHeartBeatTimer(pMac, psessionEntry);
    }
    return;
}


/* 
                           
  
           
                                                                       
                                                       
  
       
                                                
               
 */
void pmmMissedBeaconHandler(tpAniSirGlobal pMac)
{
    tANI_U8 pwrSaveSessionId;
    tANI_U32 beaconInterval = 0;
    tANI_U32 heartBeatInterval = pMac->lim.gLimHeartBeatCount;
    tpPESession psessionEntry;
    
    /*                                                     */
    pwrSaveSessionId = pMac->pmm.sessionId;

    if((psessionEntry = peFindSessionBySessionId(pMac,pwrSaveSessionId))==NULL)
    {
        limLog(pMac, LOGE,FL("Session Does not exist for given sessionID\n"));
        return;
    }


    PELOGE(pmmLog(pMac, LOG1, FL("The device woke up due to MISSED BEACON \n"));)

    /*                                            */
    if((pMac->lim.limTimers.gLimHeartBeatTimer.pMac) &&
       (pMac->lim.gLimTimersCreated))
    {
        if (wlan_cfgGetInt(pMac, WNI_CFG_BEACON_INTERVAL, &beaconInterval) != eSIR_SUCCESS)
            PELOG1(pmmLog(pMac, LOG1, FL("Fail to get BEACON_INTERVAL value\n"));)

        /*                                         */
        heartBeatInterval= SYS_MS_TO_TICKS(beaconInterval * heartBeatInterval);

        if( tx_timer_change(&pMac->lim.limTimers.gLimHeartBeatTimer,
                            (tANI_U32)heartBeatInterval, 0) != TX_SUCCESS)
        {
            PELOG1(pmmLog(pMac, LOG1, FL("Fail to change HeartBeat timer\n"));)
        }

        /*                        */
        if(LIM_IS_CONNECTION_ACTIVE(psessionEntry))
        {
            if(psessionEntry->LimRxedBeaconCntDuringHB < MAX_NO_BEACONS_PER_HEART_BEAT_INTERVAL)
                pMac->lim.gLimHeartBeatBeaconStats[psessionEntry->LimRxedBeaconCntDuringHB]++;
            else
                pMac->lim.gLimHeartBeatBeaconStats[0]++;
        }

        /*                                                                               
                                                                                    
                                       
         */
        //                                                              
        limResetHBPktCount(psessionEntry); //                                                        
        pmmSendMessageToLim(pMac, SIR_LIM_HEART_BEAT_TIMEOUT);
    }
    else
    {
        PELOGE(pmmLog(pMac, LOGE, FL("HeartBeat Timer is not created, cannot re-activate\n"));)
    }

    return;
}


/* 
                               
  
           
                                                                  
                                                                                 
             
                                                                             
                                                                                          
                                                                   
                                                                                  
                                                                          
                                                                                           
                                                                                     
                                                                                                     
                                                                                                        
                            
        
  
              
     
  
       
     
  
                                                
                                                                     
  
                
 */

void pmmExitBmpsIndicationHandler(tpAniSirGlobal pMac, tANI_U8 mode, eHalStatus rspStatus)
{

    tANI_U32 beaconInterval = 0;
    tANI_U32 heartBeatInterval = pMac->lim.gLimHeartBeatCount;
    tANI_U8  powersavesessionId;
    tpPESession psessionEntry;

    /*                                                     */
    powersavesessionId = pMac->pmm.sessionId;

    psessionEntry = peFindSessionBySessionId(pMac,powersavesessionId);

    if(psessionEntry == NULL)
    {
      PELOGE(limLog(pMac, LOGE,FL("Session does Not exist with given sessionId :%d \n"),powersavesessionId);)
      return;
    }

    /*                                                                    
                                                                             
                                                          
     */

    PELOGW(pmmLog(pMac, LOGW, 
           FL("pmmBmps: Received SIR_HAL_EXIT_BMPS_IND from HAL, Exiting BMPS sleep mode\n")); )


    pMac->pmm.gPmmState = ePMM_STATE_BMPS_WAKEUP;
    pmmUpdateWakeupStats(pMac);

    /*                         */
    pMac->sys.gSysEnableScanMode = true;

    pmmBmpsUpdateWakeupIndCnt(pMac);

    /*                                         */
    limSendSmeRsp(pMac,
                  eWNI_PMC_EXIT_BMPS_IND,
                  eSIR_SME_SUCCESS, 0, 0);

    switch(rspStatus)
    {

        /*                                                                       
                                                                                  
                                                          
         */
        case eHAL_STATUS_HEARTBEAT_TMOUT:
            {
                PELOG1(pmmLog(pMac, LOG1,
                              FL("pmmBmps: The device woke up due to HeartBeat Timeout\n"));)

                /*                                            */
                if((pMac->lim.limTimers.gLimHeartBeatTimer.pMac) &&
                   (pMac->lim.gLimTimersCreated))
                {

                    /*                                            */
                    beaconInterval = psessionEntry->beaconParams.beaconInterval;

                    /*                                         */
                    heartBeatInterval= SYS_MS_TO_TICKS(beaconInterval * heartBeatInterval);

                    if(tx_timer_change(&pMac->lim.limTimers.gLimHeartBeatTimer,
                                       (tANI_U32)heartBeatInterval, 0) != TX_SUCCESS)
                    {
                        PELOG1(pmmLog(pMac, LOG1,
                               FL("pmmBmps: Unable to change HeartBeat timer\n"));)
                    }

                    /*                        */
                    if(LIM_IS_CONNECTION_ACTIVE(psessionEntry))
                    {
                        if(psessionEntry->LimRxedBeaconCntDuringHB < MAX_NO_BEACONS_PER_HEART_BEAT_INTERVAL)
                            pMac->lim.gLimHeartBeatBeaconStats[psessionEntry->LimRxedBeaconCntDuringHB]++;
                        else
                            pMac->lim.gLimHeartBeatBeaconStats[0]++;
                    }

                    /*                                                                           
                                                                                                
                                                   
                     */
                    pmmSendMessageToLim(pMac, SIR_LIM_HEART_BEAT_TIMEOUT);

                }
                else
                {

                    PELOGE(pmmLog(pMac, LOGE, 
                           FL("pmmBmps: HeartBeat Timer is not created, cannot re-activate\n"));)
                }
            }
            break;

        case eHAL_STATUS_NTH_BEACON_DELIVERY:
            break;

        default:
            break;

    }

    return;

}


//                                                                     
/* 
                         
  
            
  
         
  
               
  
        
  
              
               
 */

void pmmProcessMessageQueue(tpAniSirGlobal pMac)
{
#if 0
    tSirMsgQ msg;

    while (1)
    {
        if (tx_queue_receive(&pMac->sys.gSirPmmMsgQ, (void *) &msg, TX_WAIT_FOREVER)
            != TX_SUCCESS)
            break;

        pmmProcessMessage(pMac, &msg);
    }
#endif
}

//                                                                     
/* 
                    
  
                                                                  
  
         
  
               
  
        
  
              
               
 */

void pmmProcessMessage(tpAniSirGlobal pMac, tpSirMsgQ pMsg)
{
    switch (pMsg->type)
    {
        case eWNI_PMC_PWR_SAVE_CFG:
        {
            tpSirPowerSaveCfg pPSCfg;
            tSirMbMsg *pMbMsg = (tSirMbMsg *)pMsg->bodyptr;

            if (palAllocateMemory(pMac->hHdd, (void **)&pPSCfg, sizeof(tSirPowerSaveCfg)) != eHAL_STATUS_SUCCESS)
            {
                pmmLog(pMac, LOGP, "PMM: Not able to allocate memory for PMC Config\n");
            }
            (void) palCopyMemory(pMac->hHdd, pPSCfg, pMbMsg->data, sizeof(tSirPowerSaveCfg));
            (void) pmmSendPowerSaveCfg(pMac, pPSCfg);

            //                                                 
            if (limSendBeaconFilterInfo(pMac) != eSIR_SUCCESS)
                pmmLog(pMac, LOGE, FL("Fail to send Beacon Filter Info \n"));
        }
            break;

        case eWNI_PMC_ENTER_BMPS_REQ:
            pmmInitBmpsPwrSave(pMac);
            break;

        case WDA_ENTER_BMPS_RSP:
            pmmInitBmpsResponseHandler(pMac, (eHalStatus)pMsg->bodyval);
            break;

        case eWNI_PMC_EXIT_BMPS_REQ:
        {
            tpExitBmpsInfo  pExitBmpsInfo;
            tSirMbMsg      *pMbMsg = (tSirMbMsg *)pMsg->bodyptr;

            if (palAllocateMemory(pMac->hHdd, (void **)&pExitBmpsInfo, sizeof(tExitBmpsInfo)) != eHAL_STATUS_SUCCESS)
            {
                pmmLog(pMac, LOGP, "PMM: Failed to allocate memory for Exit BMPS Info \n");
            }
            (void) palCopyMemory(pMac->hHdd, pExitBmpsInfo, pMbMsg->data, sizeof(tExitBmpsInfo));
            (void) pmmExitBmpsRequestHandler(pMac, pExitBmpsInfo);
        }
            break;

        case WDA_EXIT_BMPS_RSP:
            pmmExitBmpsResponseHandler(pMac, pMsg);
            break;

        case WDA_EXIT_BMPS_IND:
            pmmExitBmpsIndicationHandler(pMac, SIR_PM_ACTIVE_MODE, (eHalStatus)pMsg->bodyval);
            break;

        case eWNI_PMC_ENTER_IMPS_REQ:
            pmmEnterImpsRequestHandler(pMac);
            break;

        case WDA_ENTER_IMPS_RSP:
            pmmEnterImpsResponseHandler(pMac, (eHalStatus)pMsg->bodyval);
            break;

        case eWNI_PMC_EXIT_IMPS_REQ:
            pmmExitImpsRequestHandler(pMac);
            break;

        case WDA_EXIT_IMPS_RSP:
            pmmExitImpsResponseHandler(pMac, (eHalStatus)pMsg->bodyval);
            break;

        case eWNI_PMC_ENTER_UAPSD_REQ:
            pmmEnterUapsdRequestHandler(pMac);
            break;

        case WDA_ENTER_UAPSD_RSP:
            pmmEnterUapsdResponseHandler(pMac, pMsg);
            break;

        case eWNI_PMC_EXIT_UAPSD_REQ:
            pmmExitUapsdRequestHandler(pMac);
            break;

        case WDA_EXIT_UAPSD_RSP:
            pmmExitUapsdResponseHandler(pMac, (eHalStatus)pMsg->bodyval);
            break;

        case eWNI_PMC_WOWL_ADD_BCAST_PTRN:
            pmmSendWowlAddBcastPtrn(pMac, pMsg);
            break;

        case eWNI_PMC_WOWL_DEL_BCAST_PTRN:
            pmmSendWowlDelBcastPtrn(pMac, pMsg);
            break;

        case eWNI_PMC_ENTER_WOWL_REQ:
            pmmEnterWowlRequestHandler(pMac, pMsg);
            break;

        case WDA_WOWL_ENTER_RSP:
            pmmEnterWowlanResponseHandler(pMac, pMsg);
            break;

        case eWNI_PMC_EXIT_WOWL_REQ:
            pmmExitWowlanRequestHandler(pMac);
            break;

        case WDA_WOWL_EXIT_RSP:
            pmmExitWowlanResponseHandler(pMac, (eHalStatus)pMsg->bodyval);
            break;
#ifdef WLAN_FEATURE_PACKET_FILTERING
        case WDA_PACKET_COALESCING_FILTER_MATCH_COUNT_RSP:
            pmmFilterMatchCountResponseHandler(pMac, pMsg);
            break;
#endif //                              


#ifdef WLAN_FEATURE_GTK_OFFLOAD
        case WDA_GTK_OFFLOAD_GETINFO_RSP:
            pmmGTKOffloadGetInfoResponseHandler(pMac, pMsg);
            break;
#endif //                         

        default:
            PELOGW(pmmLog(pMac, LOGW, 
                FL("PMM: Unknown message in pmmMsgQ type %d, potential memory leak!!\n"),
                pMsg->type);)
    }

    if (NULL != pMsg->bodyptr)
    {
        palFreeMemory(pMac->hHdd, (tANI_U8 *)pMsg->bodyptr);
        pMsg->bodyptr = NULL;
    }
}






//                                                                     
/* 
                 
  
            
                                          
  
         
  
               
  
        
  
                                 
               
 */

tSirRetStatus
pmmPostMessage(tpAniSirGlobal pMac, tpSirMsgQ pMsg)
{
#if defined(VOSS_ENABLED)
    VOS_STATUS vosStatus;
    vosStatus = vos_mq_post_message(VOS_MQ_ID_PE, (vos_msg_t *) pMsg);
    if(!VOS_IS_STATUS_SUCCESS(vosStatus))
    {
        pmmLog(pMac, LOGP, FL("vos_mq_post_message failed with status code %d\n"), vosStatus);
        return eSIR_FAILURE;
    }
#elif defined(ANI_OS_TYPE_WINDOWS)
    pmmProcessMessage(pMac, pMsg);
#else
    if (tx_queue_send(&pMac->sys.gSirPmmMsgQ, pMsg, TX_NO_WAIT) != TX_SUCCESS)
        return eSIR_FAILURE;

#endif //             

    return eSIR_SUCCESS;
}





/* 
                        
  
                                                                              
  
         
  
               
  
        
  
                                               
               
 */

void pmmUpdatePwrSaveStats(tpAniSirGlobal pMac)
{
/*
                           

                                                                

                                      
                                                                                       
        
                                         

                                               
     
                                               
     

                                                                                  
     
                                               
     

                                                                                                                                          

                             
           
*/
}




/* 
                        
  
                                                            
  
         
  
               
  
        
  
                                               
               
 */

void pmmUpdateWakeupStats(tpAniSirGlobal pMac)
{
/*

                               

                                                                     
                                                                                        

                                                   
         
                                                   
         

                                                                                      
         
                                                   
         

                                                                                                                                              

                                 
               
*/
}

//                                                                     
/* 
                             
  
            
                                                                        
                                                             
  
         
  
               
       
  
        
  
                                
                
 */
void pmmEnterImpsRequestHandler (tpAniSirGlobal pMac)
{

    tSirResultCodes resultCode = eSIR_SME_SUCCESS;
    tSirRetStatus   retStatus = eSIR_SUCCESS;
    tPmmState       origState = pMac->pmm.gPmmState;

#ifdef FEATURE_WLAN_DIAG_SUPPORT 
    limDiagEventReport(pMac, WLAN_PE_DIAG_ENTER_IMPS_REQ_EVENT, peGetValidPowerSaveSession(pMac), 0, 0);
#endif //                         

    /*                                                */
    if(peIsAnySessionActive(pMac))
    {
        resultCode = eSIR_SME_INVALID_STATE;
        goto failure;
    }

    if ( ((pMac->pmm.gPmmState != ePMM_STATE_READY) &&
          (pMac->pmm.gPmmState != ePMM_STATE_IMPS_WAKEUP)) ||
         ((pMac->lim.gLimSmeState != eLIM_SME_IDLE_STATE) &&
          (pMac->lim.gLimSmeState != eLIM_SME_JOIN_FAILURE_STATE)) ||
         (pMac->lim.gLimMlmState != eLIM_MLM_IDLE_STATE) ||
         limIsChanSwitchRunning (pMac) ||
         limIsInQuietDuration (pMac) )
    {
        PELOGE(pmmLog(pMac, LOGE, 
              FL("pmmImps: PMM State = %d, Global MLM State = %d, Global SME State = %d, rejecting the sleep mode request\n"),
              pMac->pmm.gPmmState, pMac->lim.gLimMlmState, pMac->lim.gLimSmeState);)

        resultCode = eSIR_SME_INVALID_PMM_STATE;
        pmmImpsUpdateErrStateStats(pMac);
        goto failure;
    }

    //                                            
    pMac->pmm.gPmmState = ePMM_STATE_IMPS_WT_SLEEP_RSP;
    if( (retStatus = pmmImpsSendChangePwrSaveMsg(pMac, SIR_PM_SLEEP_MODE)) != eSIR_SUCCESS)
    {
        PELOGE(pmmLog(pMac, LOGE, 
               FL("pmmImps: IMPS Sleep Request failed: sending response: %x\n"), retStatus);)

        resultCode = eSIR_SME_IMPS_REQ_FAILED;
        goto failure;
    }
    else
    {
        PELOG1(pmmLog(pMac, LOG1,
               FL("pmmImps: Waiting for SoftMac response for IMPS request\n"));)
    }
    return;

failure:
    pMac->pmm.gPmmState = origState;
    pmmImpsUpdateSleepErrStats(pMac, retStatus);

    limSendSmeRsp(pMac,
                  eWNI_PMC_ENTER_IMPS_RSP,
                  resultCode, 0, 0);

}

//                                                                     
/* 
                              
  
            
                                                                       
                                                                         
                                                                       
                                                         
  
         
  
               
       
  
        
  
                                             
                
 */
void pmmEnterImpsResponseHandler (tpAniSirGlobal pMac, eHalStatus rspStatus)
{
    tPmmState nextState = pMac->pmm.gPmmState;
    tSirResultCodes resultCode = eSIR_SME_SUCCESS;

    /*                                                            
                                             
     */
    SET_LIM_PROCESS_DEFD_MESGS(pMac, true);

    if(pMac->pmm.gPmmState != ePMM_STATE_IMPS_WT_SLEEP_RSP)
    {
        PELOGE(pmmLog(pMac, LOGE, 
               FL("pmmImps: Receives IMPS sleep rsp in invalid state: %d\n"),
               pMac->pmm.gPmmState);)

        resultCode = eSIR_SME_INVALID_PMM_STATE;
        pmmImpsUpdateErrStateStats(pMac);

        goto failure;
    }

    if(eHAL_STATUS_SUCCESS == rspStatus)
    {
        //                                               
        pMac->pmm.gPmmState = ePMM_STATE_IMPS_SLEEP;

        PELOG2(pmmLog(pMac, LOG2,
            FL("pmmImps: Received successful WDA_ENTER_IMPS_RSP from HAL\n"));)

        //                            
        pmmImpsUpdatePwrSaveStats(pMac);

        limSendSmeRsp(pMac,
                      eWNI_PMC_ENTER_IMPS_RSP,
                      resultCode, 0, 0);
    }
    else
    {
        //                                            
        nextState = ePMM_STATE_IMPS_WAKEUP;
        resultCode = eSIR_SME_CANNOT_ENTER_IMPS;
        goto failure;
    }
    return;

failure:
    PELOGE(pmmLog(pMac, LOGE, 
           FL("pmmImpsSleepRsp failed, Ret Code: %d, next state will be: %d\n"),
           rspStatus,
           pMac->pmm.gPmmState);)

    pMac->pmm.gPmmState = nextState;

    limSendSmeRsp(pMac,
                  eWNI_PMC_ENTER_IMPS_RSP,
                  resultCode, 0, 0);
}


//                                                                     
/* 
                            
  
            
                                                                           
                                                              
  
         
  
               
       
  
        
  
                                
                
 */
void pmmExitImpsRequestHandler (tpAniSirGlobal pMac)
{
    tSirRetStatus retStatus = eSIR_SUCCESS;
    tSirResultCodes resultCode = eSIR_SME_SUCCESS;

    tPmmState origState = pMac->pmm.gPmmState;

#ifdef FEATURE_WLAN_DIAG_SUPPORT 
    limDiagEventReport(pMac, WLAN_PE_DIAG_EXIT_IMPS_REQ_EVENT, peGetValidPowerSaveSession(pMac), 0, 0);
#endif //                         

    if (ePMM_STATE_IMPS_SLEEP == pMac->pmm.gPmmState)
    {
        pMac->pmm.gPmmState = ePMM_STATE_IMPS_WT_WAKEUP_RSP;
        if( (retStatus = pmmImpsSendChangePwrSaveMsg(pMac, SIR_PM_ACTIVE_MODE)) !=
            eSIR_SUCCESS)
        {
            PELOGE(pmmLog(pMac, LOGE, 
                   FL("pmmImps: Wakeup request message sent to SoftMac failed\n"));)
            resultCode = eSIR_SME_IMPS_REQ_FAILED;
            goto failure;
        }
    }
    else
    {
        //                     
        PELOGE(pmmLog(pMac, LOGE, 
                      FL("pmmImps: Wakeup Req received in invalid state: %x\n"),
                      pMac->pmm.gPmmState);)

        resultCode = eSIR_SME_INVALID_PMM_STATE;
        pmmImpsUpdateErrStateStats(pMac);

        goto failure;
    }
    return;

failure:
    PELOGE(pmmLog (pMac, LOGE, 
                   FL("pmmImps: Changing to IMPS wakeup mode failed, Ret Code: %d, Next State: %d\n"),
                   retStatus, pMac->pmm.gPmmState);)

    pMac->pmm.gPmmState = origState;
    pmmImpsUpdateWakeupErrStats(pMac, retStatus);

    limSendSmeRsp(pMac,
                  eWNI_PMC_EXIT_IMPS_RSP,
                  resultCode, 0, 0);
}


//                                                                     
/* 
                             
  
            
                                                                       
                                                                         
                                                                       
                                                         
  
         
  
               
       
  
        
  
                               
               
 */
void pmmExitImpsResponseHandler(tpAniSirGlobal pMac, eHalStatus rspStatus)
{
    tSirResultCodes resultCode = eSIR_SME_SUCCESS;

    /*                                                            
                                            
     */
    SET_LIM_PROCESS_DEFD_MESGS(pMac, true);

    if (pMac->pmm.gPmmState != ePMM_STATE_IMPS_WT_WAKEUP_RSP)
    {
        PELOGE(pmmLog(pMac, LOGE, 
                      FL("pmmImps: Received 'Wakeup' response in invalid state: %d\n"), 
                      pMac->pmm.gPmmState);)

        resultCode = eSIR_SME_INVALID_PMM_STATE;
        pmmImpsUpdateErrStateStats(pMac);
    }

    switch(rspStatus)
    {
    case eHAL_STATUS_SUCCESS:
        {
            resultCode = eSIR_SME_SUCCESS;
            PELOG2(pmmLog(pMac, LOG2, 
                          FL("pmmImps: Received WDA_EXIT_IMPS_RSP with Successful response from HAL\n"));)
        }
        break;

        default:
            {
                resultCode = eSIR_SME_IMPS_REQ_FAILED;
                PELOGW(pmmLog(pMac, LOGW, 
                              FL("pmmImps: Received WDA_EXIT_IMPS_RSP with Failure Status from HAL\n"));)
            }
            break;

    }

    pMac->pmm.gPmmState = ePMM_STATE_IMPS_WAKEUP;

    //                            
    pmmImpsUpdateWakeupStats(pMac);

    limSendSmeRsp(pMac,
                  eWNI_PMC_EXIT_IMPS_RSP,
                  resultCode, 0, 0);
    return;

}

//                                                                     
/* 
                              
  
            
                                                               
                                                                
                                                                  
                                                 
                              
  
        
  
                                    
                    
 */
void pmmEnterUapsdRequestHandler (tpAniSirGlobal pMac)
{
    tSirResultCodes resultCode = eSIR_SME_SUCCESS;
    tSirRetStatus   retStatus = eSIR_SUCCESS;

    tPmmState origState = pMac->pmm.gPmmState;

#ifdef FEATURE_WLAN_DIAG_SUPPORT 
    limDiagEventReport(pMac, WLAN_PE_DIAG_ENTER_UAPSD_REQ_EVENT, peGetValidPowerSaveSession(pMac), 0, 0);
#endif //                         

    if ( (pMac->pmm.gPmmState != ePMM_STATE_BMPS_SLEEP) ||
         limIsSystemInScanState(pMac) )
    {
        PELOGE(pmmLog(pMac, LOGE,
            FL("pmmUapsd: PMM State = %d, Global MLM State = %d, Global SME State = %d, rejecting the sleep mode request\n"),
            pMac->pmm.gPmmState, pMac->lim.gLimMlmState, pMac->lim.gLimSmeState);)

        resultCode = eSIR_SME_INVALID_PMM_STATE;
        goto failure;
    }

    pMac->pmm.gPmmState = ePMM_STATE_UAPSD_WT_SLEEP_RSP;

    if( (retStatus = pmmUapsdSendChangePwrSaveMsg(pMac, SIR_PM_SLEEP_MODE)) != eSIR_SUCCESS)
    {
        PELOGE(pmmLog(pMac, LOGE,
            FL("pmmUapsd: HAL_ENTER_UAPSD_REQ failed with response: %x\n"), retStatus);)
        resultCode = eSIR_SME_UAPSD_REQ_FAILED;
        goto failure;
    }

    PELOG1(pmmLog(pMac, LOG1, FL("pmmUapsd: Waiting for WDA_ENTER_UAPSD_RSP \n"));)
    return;

failure:
    pMac->pmm.gPmmState = origState;
    limSendSmeRsp(pMac, eWNI_PMC_ENTER_UAPSD_RSP, resultCode, 0, 0);
    return;
}


//                                                                     
/* 
                               
  
            
                                                                
                                                                         
                                                
  
        
  
                 
               
 */
void pmmEnterUapsdResponseHandler(tpAniSirGlobal pMac, tpSirMsgQ limMsg)
{
    tpUapsdParams    pUapsdRspMsg;
    tSirResultCodes  retStatus = eSIR_SME_SUCCESS;

    tANI_U8 PowersavesessionId;
    tpPESession psessionEntry;

    /*                                                            
                                                  
     */
    SET_LIM_PROCESS_DEFD_MESGS(pMac, true);

    /*                                                     */
    PowersavesessionId = pMac->pmm.sessionId;

    if (NULL == limMsg->bodyptr)
    {
        PELOGE(pmmLog(pMac, LOGE, FL("pmmUapsd: Received SIR_HAL_ENTER_UAPSD_RSP with NULL "));)
        return;
    }

    pUapsdRspMsg = (tpUapsdParams)(limMsg->bodyptr);

    if((psessionEntry = peFindSessionBySessionId(pMac,PowersavesessionId))==NULL)
    {
        limLog(pMac, LOGP,FL("Session Does not exist for given sessionID\n"));
        return;
    }

    if(pMac->pmm.gPmmState != ePMM_STATE_UAPSD_WT_SLEEP_RSP)
    {
        PELOGE(pmmLog(pMac, LOGE,
            FL("pmmUapsd: Received SIR_HAL_ENTER_UAPSD_RSP while in incorrect state: %d\n"),
            pMac->pmm.gPmmState);)
        limSendSmeRsp(pMac, eWNI_PMC_ENTER_UAPSD_RSP, eSIR_SME_INVALID_PMM_STATE, 0, 0);        
        return;
    }

    if(pUapsdRspMsg->status == eHAL_STATUS_SUCCESS)
    {
        PELOGW(pmmLog(pMac, LOGW,
            FL("pmmUapsd: Received successful response from HAL to enter UAPSD mode \n"));)
        pMac->pmm.gPmmState = ePMM_STATE_UAPSD_SLEEP;
    }
    else
    {
        PELOGE(pmmLog(pMac, LOGE,
            FL("pmmUapsd: SIR_HAL_ENTER_UAPSD_RSP failed, informing SME\n"));)
        pMac->pmm.gPmmState = ePMM_STATE_BMPS_SLEEP;
        retStatus = eSIR_SME_UAPSD_REQ_FAILED;
    }

    if(IS_SLM_SESSIONIZATION_SUPPORTED_BY_FW)
    {
        limSendSmeRsp(pMac, eWNI_PMC_ENTER_UAPSD_RSP, retStatus, 
                        psessionEntry->smeSessionId, psessionEntry->transactionId);
    }
    else
    {
        limSendSmeRsp(pMac, eWNI_PMC_ENTER_UAPSD_RSP, retStatus, 0, 0);
    }

    return;
}


//                                                                     
/* 
                             
  
            
                                                              
                                                                
                                                           
                                                 
                                 
  
        
  
                                    
                    
 */
void pmmExitUapsdRequestHandler(tpAniSirGlobal pMac)
{
    tSirRetStatus retStatus = eSIR_SUCCESS;
    tSirResultCodes resultCode = eSIR_SME_SUCCESS;

    tPmmState origState = pMac->pmm.gPmmState;

#ifdef FEATURE_WLAN_DIAG_SUPPORT 
    limDiagEventReport(pMac, WLAN_PE_DIAG_EXIT_UAPSD_REQ_EVENT, peGetValidPowerSaveSession(pMac), 0, 0);
#endif //                         

    if (ePMM_STATE_UAPSD_SLEEP == pMac->pmm.gPmmState)
    {
        pMac->pmm.gPmmState = ePMM_STATE_UAPSD_WT_WAKEUP_RSP;
        if( (retStatus = pmmUapsdSendChangePwrSaveMsg(pMac, SIR_PM_ACTIVE_MODE)) !=
                                                eSIR_SUCCESS)
        {
            PELOGE(pmmLog(pMac, LOGE,
                FL("pmmUapsd: sending EXIT_UAPSD to HAL failed \n"));)
            resultCode = eSIR_SME_UAPSD_REQ_FAILED;
            goto failure;
        }
    }
    else
    {
        PELOGE(pmmLog(pMac, LOGE,
            FL("pmmUapsd: Rcv EXIT_UAPSD from PMC in invalid state: %x\n"),
            pMac->pmm.gPmmState);)

        resultCode = eSIR_SME_INVALID_PMM_STATE;
        goto failure;
    }
    return;

failure:
    pMac->pmm.gPmmState = origState;
    PELOGE(pmmLog(pMac, LOGE,
        FL("pmmUapsd: Waking up from UAPSD mode failed, Ret Code: %d, Next State: %d\n"),
        retStatus, pMac->pmm.gPmmState);)
    limSendSmeRsp(pMac, eWNI_PMC_EXIT_UAPSD_RSP, resultCode, 0, 0);
}


//                                                                     
/* 
                              
  
            
                                                                    
                                                               
                                                                
                                           
  
        
  
                                    
                    
 */
void pmmExitUapsdResponseHandler(tpAniSirGlobal pMac, eHalStatus rspStatus)
{
    tSirResultCodes resultCode = eSIR_SME_SUCCESS;
    tANI_U8 PowersavesessionId;
    tpPESession psessionEntry;

    /*                                                            
                                                 
     */
    SET_LIM_PROCESS_DEFD_MESGS(pMac, true);

    if (pMac->pmm.gPmmState != ePMM_STATE_UAPSD_WT_WAKEUP_RSP)
    {
        PELOGE(pmmLog(pMac, LOGE,
            FL("Received HAL_EXIT_UAPSD_RSP in invalid state: %d\n"),
            pMac->pmm.gPmmState);)
        limSendSmeRsp(pMac, eWNI_PMC_EXIT_UAPSD_RSP, eSIR_SME_INVALID_PMM_STATE, 0, 0);
        return;
    }

    PowersavesessionId = pMac->pmm.sessionId;
    if((psessionEntry = peFindSessionBySessionId(pMac,PowersavesessionId))==NULL)
    {
        limLog(pMac, LOGP,FL("Session Does not exist for given sessionID\n"));
        return;
    }

    switch(rspStatus)
    {
        case eHAL_STATUS_SUCCESS:
            resultCode = eSIR_SME_SUCCESS;
            PELOGW(pmmLog(pMac, LOGW,
                FL("Received SIR_HAL_EXIT_UAPSD_RSP with Successful response \n"));)
            break;
        default:
            resultCode = eSIR_SME_UAPSD_REQ_FAILED;
            PELOGE(pmmLog(pMac, LOGW,
                FL("Received SIR_HAL_EXIT_UAPSD_RSP with Failure Status\n"));)
            break;
    }

    pMac->pmm.gPmmState = ePMM_STATE_BMPS_SLEEP;

    if(IS_SLM_SESSIONIZATION_SUPPORTED_BY_FW)
    {
        limSendSmeRsp(pMac, eWNI_PMC_EXIT_UAPSD_RSP, resultCode, psessionEntry->smeSessionId,
                      psessionEntry->transactionId);
    }
    else
    {
        limSendSmeRsp(pMac, eWNI_PMC_EXIT_UAPSD_RSP, resultCode, 0, 0);
    }
    return;
}

/*                                                              
                                
                                                          
                        
                             
                             
             
                                                                */
void pmmSendWowlAddBcastPtrn(tpAniSirGlobal pMac,  tpSirMsgQ pMsg)
{
    tpSirWowlAddBcastPtrn  pBcastPtrn;
    tSirMbMsg              *pMbMsg = (tSirMbMsg *)pMsg->bodyptr;
    tSirRetStatus          retCode = eSIR_SUCCESS;
    tSirMsgQ               msgQ;

    if (palAllocateMemory(pMac->hHdd, (void **)&pBcastPtrn, sizeof(*pBcastPtrn)) != eHAL_STATUS_SUCCESS)
    {
        pmmLog(pMac, LOGP, FL("Fail to allocate memory for WoWLAN Add Bcast Pattern \n"));
        return;
    }
    (void) palCopyMemory(pMac->hHdd, pBcastPtrn, pMbMsg->data, sizeof(*pBcastPtrn));

    if (NULL == pBcastPtrn)
    {
        pmmLog(pMac, LOGE, FL("Add broadcast pattern message is NULL \n"));
        return;
    }

    msgQ.type = WDA_WOWL_ADD_BCAST_PTRN;
    msgQ.reserved = 0;
    msgQ.bodyptr = pBcastPtrn;
    msgQ.bodyval = 0;

    pmmLog(pMac, LOG1, FL( "Sending WDA_WOWL_ADD_BCAST_PTRN to HAL"));
#ifdef FEATURE_WLAN_DIAG_SUPPORT 
    limDiagEventReport(pMac, WLAN_PE_DIAG_WOWL_ADD_BCAST_PTRN_EVENT, NULL, 0, 0);
#endif //                         

    if( eSIR_SUCCESS != (retCode = wdaPostCtrlMsg( pMac, &msgQ )))
    {
        if (pBcastPtrn != NULL)
            palFreeMemory( pMac->hHdd, (tANI_U8 *) pBcastPtrn);
        pmmLog( pMac, LOGP, FL("Posting WDA_WOWL_ADD_BCAST_PTRN failed, reason=%X"), retCode );
    }
    return;
}

/*                                                              
                                
                                                          
                        
                             
                             
             
                                                                */
void pmmSendWowlDelBcastPtrn(tpAniSirGlobal pMac, tpSirMsgQ pMsg)
{
    tpSirWowlDelBcastPtrn   pDeletePtrn;
    tSirMbMsg               *pMbMsg = (tSirMbMsg *)pMsg->bodyptr;
    tSirRetStatus           retCode = eSIR_SUCCESS;
    tSirMsgQ                msgQ;

    if (palAllocateMemory(pMac->hHdd, (void **)&pDeletePtrn, sizeof(*pDeletePtrn)) != eHAL_STATUS_SUCCESS)
    {
        pmmLog(pMac, LOGP, FL("Fail to allocate memory for WoWLAN Delete Bcast Pattern \n"));
        return;
    }
    (void) palCopyMemory(pMac->hHdd, pDeletePtrn, pMbMsg->data, sizeof(*pDeletePtrn));

    if (NULL == pDeletePtrn)
    {
        pmmLog(pMac, LOGE, FL("Delete broadcast pattern message is NULL \n"));
        return;
    }

    msgQ.type = WDA_WOWL_DEL_BCAST_PTRN;
    msgQ.reserved = 0;
    msgQ.bodyptr = pDeletePtrn;
    msgQ.bodyval = 0;

    pmmLog(pMac, LOG1, FL( "Sending WDA_WOWL_DEL_BCAST_PTRN"));
#ifdef FEATURE_WLAN_DIAG_SUPPORT 
    limDiagEventReport(pMac, WLAN_PE_DIAG_WOWL_DEL_BCAST_PTRN_EVENT, NULL, 0, 0);
#endif //                         

    if( eSIR_SUCCESS != (retCode = wdaPostCtrlMsg( pMac, &msgQ )))
    {
        if (NULL != pDeletePtrn)
            palFreeMemory( pMac->hHdd, (tANI_U8 *) pDeletePtrn);
        pmmLog( pMac, LOGP, FL("Posting WDA_WOWL_DEL_BCAST_PTRN failed, reason=%X"), retCode );
    }
    return;
}

/*                                                           
                                   
                                                             
                                                  
                                           
                             
                             
             
                                                             */
void pmmEnterWowlRequestHandler(tpAniSirGlobal pMac, tpSirMsgQ pMsg)
{
    tpSirSmeWowlEnterParams  pSmeWowlParams = NULL;
    tpSirHalWowlEnterParams  pHalWowlParams = NULL;
    tSirRetStatus  retCode = eSIR_SUCCESS;
    tANI_U32  cfgValue = 0;
    tSirMbMsg *pMbMsg = (tSirMbMsg *)pMsg->bodyptr;

#ifdef FEATURE_WLAN_DIAG_SUPPORT 
    limDiagEventReport(pMac, WLAN_PE_DIAG_ENTER_WOWL_REQ_EVENT, NULL, 0, 0);
#endif //                         

    pSmeWowlParams = (tpSirSmeWowlEnterParams)(pMbMsg->data);

    if (NULL == pSmeWowlParams)
        return;
//                          
#if 0
    if (pMac->lim.gLimSmeState != eLIM_SME_LINK_EST_STATE)
    {
        pmmLog(pMac, LOGE, FL("Rcvd PMC_ENTER_WOWL_REQ when station is not associated \n"));
        limSendSmeRsp(pMac, eWNI_PMC_ENTER_WOWL_RSP, eSIR_SME_STA_NOT_ASSOCIATED, 0, 0);
        goto end;
    }
#endif


    if ((pMac->pmm.gPmmState != ePMM_STATE_BMPS_SLEEP) && (pMac->pmm.gPmmState != ePMM_STATE_WOWLAN))
    {
        pmmLog(pMac, LOGE, FL("Rcvd PMC_ENTER_WOWL_REQ in invalid Power Save state \n"));
        limSendSmeRsp(pMac, eWNI_PMC_ENTER_WOWL_RSP, eSIR_SME_INVALID_PMM_STATE, 0, 0);
        goto end;
    }

    if (palAllocateMemory(pMac->hHdd, (void **)&pHalWowlParams, sizeof(*pHalWowlParams)) != eHAL_STATUS_SUCCESS)
    {
        pmmLog(pMac, LOGP, FL("Fail to allocate memory for Enter Wowl Request \n"));
        goto end;
    }
    (void) palZeroMemory(pMac->hHdd, (tANI_U8 *)pHalWowlParams, sizeof(*pHalWowlParams) );

    //                          
    pHalWowlParams->ucMagicPktEnable = pSmeWowlParams->ucMagicPktEnable;
    pHalWowlParams->ucPatternFilteringEnable = pSmeWowlParams->ucPatternFilteringEnable;
    (void)palCopyMemory( pMac->hHdd, (tANI_U8 *)pHalWowlParams->magicPtrn, (tANI_U8 *)pSmeWowlParams->magicPtrn, sizeof(tSirMacAddr) );

#ifdef WLAN_WAKEUP_EVENTS
    pHalWowlParams->ucWoWEAPIDRequestEnable = pSmeWowlParams->ucWoWEAPIDRequestEnable;
    pHalWowlParams->ucWoWEAPOL4WayEnable = pSmeWowlParams->ucWoWEAPOL4WayEnable;
    pHalWowlParams->ucWowNetScanOffloadMatch = pSmeWowlParams->ucWowNetScanOffloadMatch;
    pHalWowlParams->ucWowGTKRekeyError = pSmeWowlParams->ucWowGTKRekeyError;
    pHalWowlParams->ucWoWBSSConnLoss = pSmeWowlParams->ucWoWBSSConnLoss;
#endif //                   

    if(wlan_cfgGetInt(pMac, WNI_CFG_WOWLAN_UCAST_PATTERN_FILTER_ENABLE, &cfgValue) != eSIR_SUCCESS)
    {
        limLog(pMac, LOGP, FL("cfgGet failed for WNI_CFG_WOWLAN_UCAST_PATTERN_FILTER_ENABLE"));
        goto end;
    }
    pHalWowlParams->ucUcastPatternFilteringEnable = (tANI_U8)cfgValue;

    if(wlan_cfgGetInt(pMac, WNI_CFG_WOWLAN_CHANNEL_SWITCH_ENABLE, &cfgValue) != eSIR_SUCCESS)
    {
        limLog(pMac, LOGP, FL("cfgGet failed for WNI_CFG_WOWLAN_CHANNEL_SWITCH_ENABLE"));
        goto end;
    }
    pHalWowlParams->ucWowChnlSwitchRcv = (tANI_U8)cfgValue;

    if(wlan_cfgGetInt(pMac, WNI_CFG_WOWLAN_DEAUTH_ENABLE, &cfgValue) != eSIR_SUCCESS)
    {
       limLog(pMac, LOGP, FL("cfgGet failed for WNI_CFG_WOWLAN_DEAUTH_ENABLE \n"));
       goto end;
    }
    pHalWowlParams->ucWowDeauthRcv = (tANI_U8)cfgValue;

    if(wlan_cfgGetInt(pMac, WNI_CFG_WOWLAN_DISASSOC_ENABLE, &cfgValue) != eSIR_SUCCESS)
    {
        limLog(pMac, LOGP, FL("cfgGet failed for WNI_CFG_WOWLAN_DEAUTH_ENABLE \n"));
        goto end;
    }
    pHalWowlParams->ucWowDisassocRcv = (tANI_U8)cfgValue;

    if(wlan_cfgGetInt(pMac, WNI_CFG_WOWLAN_MAX_MISSED_BEACON, &cfgValue) != eSIR_SUCCESS)
    {
        limLog(pMac, LOGP, FL("cfgGet failed for WNI_CFG_WOWLAN_MAX_MISSED_BEACON \n"));
        goto end;
    }
    pHalWowlParams->ucWowMaxMissedBeacons = (tANI_U8)cfgValue;

    if(wlan_cfgGetInt(pMac, WNI_CFG_WOWLAN_MAX_SLEEP_PERIOD, &cfgValue) != eSIR_SUCCESS)
    {
        limLog(pMac, LOGP, FL("cfgGet failed for WNI_CFG_WOWLAN_MAX_SLEEP_PERIOD \n"));
        goto end;
    }
    pHalWowlParams->ucWowMaxSleepUsec = (tANI_U8)cfgValue;

    //                   
    if( eSIR_SUCCESS != (retCode = pmmSendWowlEnterRequest( pMac, pHalWowlParams)))
    {
        pmmLog(pMac, LOGE, FL("Send ENTER_WOWL_REQ to HAL failed, reasonCode %d \n"), retCode);
        limSendSmeRsp(pMac, eWNI_PMC_ENTER_WOWL_RSP, eSIR_SME_WOWL_ENTER_REQ_FAILED, 0, 0);
        goto end;
    }
    return;

end:
    if (pHalWowlParams != NULL)
        palFreeMemory( pMac->hHdd, (tANI_U8 *) pHalWowlParams);
    return;
}


/*                                                              
                                
                                                           
                                                               
                                                        
                                              
                                      
                                                
                      
                                                                */
tSirRetStatus pmmSendWowlEnterRequest(tpAniSirGlobal pMac, tpSirHalWowlEnterParams pHalWowlParams)
{
    tSirRetStatus             retCode = eSIR_SUCCESS;
    tSirMsgQ                  msgQ;

    if (NULL == pHalWowlParams)
        return eSIR_FAILURE;

    msgQ.type = WDA_WOWL_ENTER_REQ;
    msgQ.reserved = 0;
    msgQ.bodyptr = pHalWowlParams;
    msgQ.bodyval = 0;

    /*                                        
                                    
     */
    SET_LIM_PROCESS_DEFD_MESGS(pMac, false);

    retCode = wdaPostCtrlMsg(pMac, &msgQ);
    if( eSIR_SUCCESS != retCode )
    {
        pmmLog( pMac, LOGE, FL("Posting WDA_WOWL_ENTER_REQ failed, reason=%X"), retCode );
        return retCode;
    }
    return retCode;
}

/*                                                           
                                      
                                                    
                                                  
                             
                               
             
                                                             */
void pmmEnterWowlanResponseHandler(tpAniSirGlobal pMac, tpSirMsgQ limMsg)
{
    tpSirHalWowlEnterParams  pHalWowlMsg;
    eHalStatus            rspStatus;
    tSirResultCodes          smeRspCode = eSIR_SME_SUCCESS;

    /*                                                      
                                                   
     */
    SET_LIM_PROCESS_DEFD_MESGS(pMac, true);

    pHalWowlMsg = (tpSirHalWowlEnterParams)(limMsg->bodyptr);
    if (NULL == pHalWowlMsg)
    {
        pmmLog(pMac, LOGE, FL("Recvd WDA_WOWL_ENTER_RSP with NULL msg "));
        smeRspCode = eSIR_SME_WOWL_ENTER_REQ_FAILED;
    }
    else
    {
        rspStatus = pHalWowlMsg->status;
        if(rspStatus == eHAL_STATUS_SUCCESS)
        {
            pmmLog(pMac, LOGW, FL("Rcv successful response from HAL to enter WOWLAN \n"));
            pMac->pmm.gPmmState = ePMM_STATE_WOWLAN;
        }
        else
        {
            pmmLog(pMac, LOGE, FL("HAL enter WOWLAN failed, informing SME\n"));
            smeRspCode = eSIR_SME_WOWL_ENTER_REQ_FAILED;
        }
    }

    limSendSmeRsp(pMac, eWNI_PMC_ENTER_WOWL_RSP, smeRspCode, 0, 0);
    return;
}

/*                                                           
                                    
                                                       
                                            
                             
             
                                                             */
void pmmExitWowlanRequestHandler(tpAniSirGlobal pMac)
{
    tSirRetStatus retStatus = eSIR_SUCCESS;
    tSirResultCodes smeRspCode = eSIR_SME_SUCCESS;

#ifdef FEATURE_WLAN_DIAG_SUPPORT 
    limDiagEventReport(pMac, WLAN_PE_DIAG_EXIT_WOWL_REQ_EVENT, NULL, 0, 0);
#endif //                         

    if ( pMac->pmm.gPmmState != ePMM_STATE_WOWLAN )
    {
        pmmLog(pMac, LOGE,
            FL("Exit WOWLAN Request received in invalid state PMM=%d \n"),
            pMac->pmm.gPmmState);
        smeRspCode = eSIR_SME_INVALID_PMM_STATE;
        goto failure;
    }

    if((retStatus = pmmSendExitWowlReq(pMac)) != eSIR_SUCCESS)
    {
        pmmLog(pMac, LOGE,
            FL("Fail to send WDA_WOWL_EXIT_REQ, reason code %d\n"),
            retStatus);
        smeRspCode = eSIR_SME_WOWL_EXIT_REQ_FAILED;
        goto failure;
    }
    return;

failure:
    limSendSmeRsp(pMac, eWNI_PMC_EXIT_WOWL_RSP, smeRspCode, 0, 0);
    return;
}

/*                                                           
                           
                                                  
                        
                             
             
                                                             */
tSirRetStatus  pmmSendExitWowlReq(tpAniSirGlobal pMac)
{
    tSirRetStatus  retCode = eSIR_SUCCESS;
    tSirMsgQ       msgQ;

    msgQ.type = WDA_WOWL_EXIT_REQ;
    msgQ.reserved = 0;
    msgQ.bodyptr = 0;
    msgQ.bodyval = 0;

    pmmLog(pMac, LOGW, FL("Sending WDA_WOWL_EXIT_REQ"));

    /*                                             
                                           
     */
    SET_LIM_PROCESS_DEFD_MESGS(pMac, false);

    if( eSIR_SUCCESS != (retCode = wdaPostCtrlMsg( pMac, &msgQ )))
        pmmLog( pMac, LOGE,
            FL("Posting WDA_WOWL_EXIT_REQ failed, reason=%X\n"),
            retCode );

    return retCode;
}

/*                                                           
                                     
                                                             
                                                      
                             
             
                                                             */
void pmmExitWowlanResponseHandler(tpAniSirGlobal pMac, eHalStatus rspStatus)
{
    /*                                                      
                                                  
     */
    SET_LIM_PROCESS_DEFD_MESGS(pMac, true);

    //                               
    pMac->pmm.gPmmState = ePMM_STATE_BMPS_SLEEP;

    if(rspStatus == eHAL_STATUS_SUCCESS)
    {
        pmmLog(pMac, LOGW, FL("Rcvd successful rsp from HAL to exit WOWLAN \n"));
        limSendSmeRsp(pMac, eWNI_PMC_EXIT_WOWL_RSP, eSIR_SME_SUCCESS, 0, 0);
    }
    else
    {
        pmmLog(pMac, LOGE, FL("Rcvd failure rsp from HAL to exit WOWLAN \n"));
        limSendSmeRsp(pMac, eWNI_PMC_EXIT_WOWL_RSP, eSIR_SME_WOWL_EXIT_REQ_FAILED, 0, 0);
    }
    return;
}


//                                                                     
/* 
                              
  
            
                                                                  
  
         
  
               
       
  
        
  
                                
                                 
                
 */

tSirRetStatus pmmImpsSendChangePwrSaveMsg(tpAniSirGlobal pMac, tANI_U8 mode)
{
    tSirRetStatus retStatus = eSIR_SUCCESS;
    tSirMsgQ msgQ;

    if (SIR_PM_SLEEP_MODE == mode)
    {
        msgQ.type = WDA_ENTER_IMPS_REQ;
        PELOG2(pmmLog (pMac, LOG2, FL("Sending WDA_ENTER_IMPS_REQ to HAL\n"));)
    }
    else
    {
        msgQ.type = WDA_EXIT_IMPS_REQ;
        PELOG2(pmmLog (pMac, LOG2, FL("Sending WDA_EXIT_IMPS_REQ to HAL\n"));)
    }

    msgQ.reserved = 0;
    msgQ.bodyptr = NULL;
    msgQ.bodyval = 0;

    /*                                                      
                                                        
     */
    SET_LIM_PROCESS_DEFD_MESGS(pMac, false);
    MTRACE(macTraceMsgTx(pMac, NO_SESSION, msgQ.type));
    retStatus = wdaPostCtrlMsg(pMac, &msgQ);
    if ( eSIR_SUCCESS != retStatus )
    {
        PELOGE(pmmLog(pMac, LOGE, 
            FL("WDA_ENTER/EXIT_IMPS_REQ to HAL failed, reason=%X\n"), retStatus);)
    }

    return retStatus;
}

//                                                                     
/* 
                               
  
            
                                                             
                                
  
        
  
                                         
                                          
                         
 */
tSirRetStatus pmmUapsdSendChangePwrSaveMsg (tpAniSirGlobal pMac, tANI_U8 mode)
{
    tSirRetStatus retStatus = eSIR_SUCCESS;
    tpUapsdParams pUapsdParams = NULL;
    tANI_U8  uapsdDeliveryMask = 0;
    tANI_U8  uapsdTriggerMask = 0;
    tSirMsgQ msgQ;
    tpPESession pSessionEntry;

    if (SIR_PM_SLEEP_MODE == mode)
    {
        if( eHAL_STATUS_SUCCESS != palAllocateMemory( pMac->hHdd, (void **)&pUapsdParams, sizeof(tUapsdParams)) )
        {
            PELOGW(pmmLog(pMac, LOGW, FL("pmmUapsd :palAllocateMemory() failed\n"));)
            retStatus = eSIR_MEM_ALLOC_FAILED;
            return retStatus;
        }

        if((pSessionEntry = peGetValidPowerSaveSession(pMac)) == NULL )
        {
            PELOGW(pmmLog(pMac, LOGW, FL("pmmUapsd :palAllocateMemory() failed\n"));)
            retStatus = eSIR_FAILURE;
            return retStatus;
        }
        palZeroMemory( pMac->hHdd, (tANI_U8 *)pUapsdParams, sizeof(tUapsdParams));
        msgQ.type = WDA_ENTER_UAPSD_REQ;
        msgQ.bodyptr = pUapsdParams;

        uapsdDeliveryMask = (pMac->lim.gUapsdPerAcBitmask | pMac->lim.gUapsdPerAcDeliveryEnableMask);
        uapsdTriggerMask = (pMac->lim.gUapsdPerAcBitmask | pMac->lim.gUapsdPerAcTriggerEnableMask);

        pUapsdParams->bkDeliveryEnabled = LIM_UAPSD_GET(ACBK, uapsdDeliveryMask);
        pUapsdParams->beDeliveryEnabled = LIM_UAPSD_GET(ACBE, uapsdDeliveryMask);
        pUapsdParams->viDeliveryEnabled = LIM_UAPSD_GET(ACVI, uapsdDeliveryMask);
        pUapsdParams->voDeliveryEnabled = LIM_UAPSD_GET(ACVO, uapsdDeliveryMask);
        pUapsdParams->bkTriggerEnabled = LIM_UAPSD_GET(ACBK, uapsdTriggerMask);
        pUapsdParams->beTriggerEnabled = LIM_UAPSD_GET(ACBE, uapsdTriggerMask);
        pUapsdParams->viTriggerEnabled = LIM_UAPSD_GET(ACVI, uapsdTriggerMask);
        pUapsdParams->voTriggerEnabled = LIM_UAPSD_GET(ACVO, uapsdTriggerMask);
        pUapsdParams->bssIdx = pSessionEntry->bssIdx;

        PELOGE(pmmLog(pMac, LOGE, 
                      FL("UAPSD Mask:  static = 0x%x, DeliveryEnabled = 0x%x, TriggerEnabled = 0x%x \n"),
            pMac->lim.gUapsdPerAcBitmask,
            pMac->lim.gUapsdPerAcDeliveryEnableMask,
            pMac->lim.gUapsdPerAcTriggerEnableMask);)

        PELOG1(pmmLog(pMac, LOG1, FL("Delivery Enabled: BK=%d, BE=%d, Vi=%d, Vo=%d \n"), 
            pUapsdParams->bkDeliveryEnabled, 
            pUapsdParams->beDeliveryEnabled, 
            pUapsdParams->viDeliveryEnabled, 
            pUapsdParams->voDeliveryEnabled);)

        PELOG1(pmmLog(pMac, LOG1, FL("Trigger Enabled: BK=%d, BE=%d, Vi=%d, Vo=%d \n"), 
            pUapsdParams->bkTriggerEnabled, 
            pUapsdParams->beTriggerEnabled, 
            pUapsdParams->viTriggerEnabled, 
            pUapsdParams->voTriggerEnabled);)

        PELOGW(pmmLog (pMac, LOGW, FL("pmmUapsd: Sending WDA_ENTER_UAPSD_REQ to HAL\n"));)
    }
    else
    {
        msgQ.type = WDA_EXIT_UAPSD_REQ;
        msgQ.bodyptr = NULL;
        PELOGW(pmmLog (pMac, LOGW, FL("pmmUapsd: Sending WDA_EXIT_UAPSD_REQ to HAL\n"));)
    }

    /*                                                      
                                         
     */
    SET_LIM_PROCESS_DEFD_MESGS(pMac, false);

    msgQ.reserved = 0;
    msgQ.bodyval = 0;
    MTRACE(macTraceMsgTx(pMac, NO_SESSION, msgQ.type));
    retStatus = wdaPostCtrlMsg(pMac, &msgQ);
    if ( eSIR_SUCCESS != retStatus )
    {
        PELOGE(pmmLog(pMac, LOGE, 
            FL("pmmUapsd: WDA_ENTER/EXIT_UAPSD_REQ to HAL failed, reason=%X\n"),
            retStatus);)
        if (SIR_PM_SLEEP_MODE == mode)
            palFreeMemory(pMac->hHdd, (tANI_U8*)pUapsdParams);
    }

    return retStatus;
}


//                                                                     
/* 
                            
  
            
                                                                     
                           
  
         
  
               
       
  
        
  
                                
                
 */

void pmmImpsUpdatePwrSaveStats(tpAniSirGlobal pMac)
{
/*
                           

                                                                

                                      
     
                                                                                         
     
        
     
                      
     

                                               
     
                                               
     

                                                                                  
     
                                               
     

                                                                                                                                    

                               

           
*/
}


//                                                                     
/* 
                           
  
            
                                                                  
                           
  
         
  
               
       
  
        
  
                                
                
 */

void pmmImpsUpdateWakeupStats (tpAniSirGlobal pMac)
{
/*
                           

                                                                 

                                                                                    

                                               
     
                                               
     

                                                                                  
     
                                               
     

                                                                                                                                        

                               

           
*/
}

//                                                                  
void pmmImpsUpdateSleepErrStats(tpAniSirGlobal pMac,
                                tSirRetStatus retStatus)
{
    pMac->pmm.ImpsSleepErrCnt++;
    pMac->pmm.ImpsLastErr = retStatus;
    return;
}

//                                                                        
void pmmImpsUpdateWakeupErrStats(tpAniSirGlobal pMac,
                                 tSirRetStatus retStatus)
{
    pMac->pmm.ImpsWakeupErrCnt++;
    pMac->pmm.ImpsLastErr = retStatus;
    return;
}


//                                                            
//                             
void pmmImpsUpdateErrStateStats(tpAniSirGlobal pMac)
{
    pMac->pmm.ImpsInvalidStateCnt++;
    return;
}

//                                                      
void pmmImpsUpdatePktDropStats(tpAniSirGlobal pMac)
{

    pMac->pmm.ImpsPktDrpInSleepMode++;
    return;
}

//                                                      
void pmmBmpsUpdatePktDropStats(tpAniSirGlobal pMac)
{

    pMac->pmm.BmpsPktDrpInSleepMode++;
    return;
}

//                                                         
void pmmBmpsUpdateInitFailureCnt(tpAniSirGlobal pMac)
{

    pMac->pmm.BmpsInitFailCnt++;
    return;
}

//                                                             
void pmmBmpsUpdateSleepReqFailureCnt(tpAniSirGlobal pMac)
{

    pMac->pmm.BmpsSleeReqFailCnt++;
    return;
}

//                                                              
void pmmBmpsUpdateWakeupReqFailureCnt(tpAniSirGlobal pMac)
{

    pMac->pmm.BmpsWakeupReqFailCnt++;
    return;
}

//                                                                                     
void pmmBmpsUpdateInvalidStateCnt(tpAniSirGlobal pMac)
{

    pMac->pmm.BmpsInvStateCnt++;
    return;
}

//                                                                    
void pmmBmpsUpdateWakeupIndCnt(tpAniSirGlobal pMac)
{
    pMac->pmm.BmpsWakeupIndCnt++;
    return;
}

//                                                                    
void pmmBmpsUpdateHalReqFailureCnt(tpAniSirGlobal pMac)
{
    pMac->pmm.BmpsHalReqFailCnt++;
    return;
}

//                                                                      
//                    
void pmmBmpsUpdateReqInInvalidRoleCnt(tpAniSirGlobal pMac)
{
    pMac->pmm.BmpsReqInInvalidRoleCnt++;
    return;
}

#if 0
//                            
void pmmUpdateDroppedPktStats(tpAniSirGlobal pMac)
{
    switch (pMac->pmm.gPmmState)
    {
    case ePMM_STATE_BMPS_SLEEP:
        pmmBmpsUpdatePktDropStats(pMac);
        break;

    case ePMM_STATE_IMPS_SLEEP:
        pmmImpsUpdatePktDropStats(pMac);
        break;

    default:
        break;
    }
    return;

}
#endif

//                                  
void pmmResetPmmState(tpAniSirGlobal pMac)
{
    pMac->pmm.gPmmState = ePMM_STATE_READY;
    
    pMac->pmm.inMissedBeaconScenario = FALSE;
    return;
}

/*                                           */
void pmmSendMessageToLim(tpAniSirGlobal pMac,
                         tANI_U32 msgId)
{
    tSirMsgQ limMsg;
    tANI_U32 statusCode;

    limMsg.type = (tANI_U16) msgId;
    limMsg.bodyptr = NULL;
    limMsg.bodyval = 0;

    if ((statusCode = limPostMsgApi(pMac, &limMsg)) != eSIR_SUCCESS)
    {
          PELOGW(pmmLog(pMac, LOGW,
            FL("posting message %X to LIM failed, reason=%d\n"),
            limMsg.type, statusCode);)
    }
}

#ifdef WLAN_FEATURE_PACKET_FILTERING
void pmmFilterMatchCountResponseHandler(tpAniSirGlobal pMac, tpSirMsgQ limMsg)
{
    tpSirRcvFltPktMatchRsp  pRcvFltPktMatchCntRsp;
    eHalStatus              rspStatus;
    tSirResultCodes         smeRspCode = eSIR_SME_SUCCESS;

    /*                                                      
                                                                             
     */
    SET_LIM_PROCESS_DEFD_MESGS(pMac, true);

    pRcvFltPktMatchCntRsp = (tpSirRcvFltPktMatchRsp)(limMsg->bodyptr);
    if (NULL == pRcvFltPktMatchCntRsp)
    {
        pmmLog(pMac, LOGE, FL("Received "
            "WDA_PACKET_COALESCING_FILTER_MATCH_COUNT_RSP with NULL msg "));
        smeRspCode = eSIR_SME_PC_FILTER_MATCH_COUNT_REQ_FAILED;
    }
    else
    {
        rspStatus = pRcvFltPktMatchCntRsp->status;
        if (eHAL_STATUS_SUCCESS == rspStatus)
        {
            pmmLog(pMac, LOGE, FL("Rcv successful response from HAL to get "
                "Packet Coalescing Filter Match Count\n"));
        }
        else
        {
            pmmLog(pMac, LOGE, FL("HAL failed to get Packet Coalescing "
                "Filter Match Count, informing SME\n"));
            smeRspCode = eSIR_SME_PC_FILTER_MATCH_COUNT_REQ_FAILED;
        }
    }

    limSendSmeRsp(pMac, eWNI_PMC_PACKET_COALESCING_FILTER_MATCH_COUNT_RSP, 
                  smeRspCode, 0, 0);
    return;
}
#endif //                              

#ifdef WLAN_FEATURE_GTK_OFFLOAD
void pmmGTKOffloadGetInfoResponseHandler(tpAniSirGlobal pMac, tpSirMsgQ limMsg)
{
    tpSirGtkOffloadGetInfoRspParams  pGtkOffloadGetInfoRspParams;
    eHalStatus                       rspStatus;
    tSirResultCodes                  smeRspCode = eSIR_SME_SUCCESS;

    /*                                                      
                                                            
     */
    SET_LIM_PROCESS_DEFD_MESGS(pMac, true);

    pGtkOffloadGetInfoRspParams = (tpSirGtkOffloadGetInfoRspParams)(limMsg->bodyptr);
    if (NULL == pGtkOffloadGetInfoRspParams)
    {
        pmmLog(pMac, LOGE, FL("Received WDA_GTK_OFFLOAD_GETINFO_RSP with NULL msg "));
        smeRspCode = eSIR_SME_GTK_OFFLOAD_GETINFO_REQ_FAILED;
    }
    else
    {
        rspStatus = pGtkOffloadGetInfoRspParams->ulStatus;
        if(rspStatus == eHAL_STATUS_SUCCESS)
        {
            pmmLog(pMac, LOGW, FL("Rcv successful response from HAL to get GTK Offload Information\n"));
        }
        else
        {
            pmmLog(pMac, LOGE, FL("HAL failed to get GTK Offload Information, informing SME\n"));
            smeRspCode = eSIR_SME_GTK_OFFLOAD_GETINFO_REQ_FAILED;
        }
    }

    limSendSmeRsp(pMac, eWNI_PMC_GTK_OFFLOAD_GETINFO_RSP, smeRspCode, 0, 0);
    return;
}
#endif //                         
