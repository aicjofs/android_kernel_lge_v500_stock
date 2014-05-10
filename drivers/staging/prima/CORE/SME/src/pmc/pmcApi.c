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

/******************************************************************************
*
* Name:  pmcApi.c
*
* Description: Routines that make up the Power Management Control (PMC) API.
*
* Copyright 2008 (c) Qualcomm, Incorporated.  
* All Rights Reserved.
* Qualcomm Confidential and Proprietary.
*
******************************************************************************/

#include "palTypes.h"
#include "aniGlobal.h"
#include "palTimer.h"
#include "csrLinkList.h"
#include "smsDebug.h"
#include "pmcApi.h"
#include "pmc.h"
#include "cfgApi.h"
#include "smeInside.h"
#include "csrInsideApi.h"
#include "wlan_ps_wow_diag.h"
#include "wlan_qct_wda.h"
#include "limSessionUtils.h"
#include "csrInsideApi.h"

extern void pmcReleaseCommand( tpAniSirGlobal pMac, tSmeCmd *pCommand );

void pmcCloseDeferredMsgList(tpAniSirGlobal pMac);
void pmcCloseDeviceStateUpdateList(tpAniSirGlobal pMac);
void pmcCloseRequestStartUapsdList(tpAniSirGlobal pMac);
void pmcCloseRequestBmpsList(tpAniSirGlobal pMac);
void pmcCloseRequestFullPowerList(tpAniSirGlobal pMac);
void pmcClosePowerSaveCheckList(tpAniSirGlobal pMac);

/*                                                                             
 
                
 
              
                                             
 
             
                                 
 
          
                                          
                                              
 
                                                                             */
eHalStatus pmcOpen (tHalHandle hHal)
{
    tpAniSirGlobal pMac = PMAC_STRUCT(hHal);

    smsLog(pMac, LOG2, FL("Entering pmcOpen\n"));

    /*                                                */
    pMac->pmc.powerSource = BATTERY_POWER;
    pMac->pmc.pmcState = STOPPED;
    pMac->pmc.pmcReady = FALSE;

    /*                             */
    pMac->pmc.impsEnabled = FALSE;
    pMac->pmc.autoBmpsEntryEnabled = FALSE;
    pMac->pmc.smpsEnabled = FALSE;
    pMac->pmc.uapsdEnabled = TRUE;
    pMac->pmc.bmpsEnabled = TRUE;
    pMac->pmc.standbyEnabled = TRUE;
    pMac->pmc.wowlEnabled = TRUE;
    pMac->pmc.rfSuppliesVotedOff= FALSE;

    palZeroMemory(pMac->hHdd, &(pMac->pmc.bmpsConfig), sizeof(tPmcBmpsConfigParams));
    palZeroMemory(pMac->hHdd, &(pMac->pmc.impsConfig), sizeof(tPmcImpsConfigParams));
    palZeroMemory(pMac->hHdd, &(pMac->pmc.smpsConfig), sizeof(tPmcSmpsConfigParams));

    /*                                    */
    if (palTimerAlloc(pMac->hHdd, &pMac->pmc.hImpsTimer, pmcImpsTimerExpired, hHal) != eHAL_STATUS_SUCCESS)
    {
        smsLog(pMac, LOGE, FL("Cannot allocate timer for IMPS\n"));
        return eHAL_STATUS_FAILURE;
    }

    /*                                                             
                                                */
    if (!VOS_IS_STATUS_SUCCESS(vos_timer_init(&pMac->pmc.hTrafficTimer, 
                                VOS_TIMER_TYPE_SW, pmcTrafficTimerExpired, hHal)))
    {
        smsLog(pMac, LOGE, FL("Cannot allocate timer for traffic measurement\n"));
        return eHAL_STATUS_FAILURE;
    }

#ifdef FEATURE_WLAN_DIAG_SUPPORT    
    /*                                                                               */
    if (palTimerAlloc(pMac->hHdd, &pMac->pmc.hDiagEvtTimer, pmcDiagEvtTimerExpired, hHal) != eHAL_STATUS_SUCCESS)
    {
        smsLog(pMac, LOGE, FL("Cannot allocate timer for diag event reporting\n"));
        return eHAL_STATUS_FAILURE;
    }
#endif

    //                                                      
    pMac->pmc.bmpsConfig.trafficMeasurePeriod = BMPS_TRAFFIC_TIMER_DEFAULT;
    pMac->pmc.bmpsConfig.bmpsPeriod = WNI_CFG_LISTEN_INTERVAL_STADEF;

    /*                                                                    */
    if (palTimerAlloc(pMac->hHdd, &pMac->pmc.hExitPowerSaveTimer,
                      pmcExitPowerSaveTimerExpired, hHal) != eHAL_STATUS_SUCCESS)
    {
        smsLog(pMac, LOGE, FL("Cannot allocate exit power save mode timer\n"));
        PMC_ABORT;
        return eHAL_STATUS_FAILURE;
    }
    
    /*                                                                                          */
    if (csrLLOpen(pMac->hHdd, &pMac->pmc.powerSaveCheckList) != eHAL_STATUS_SUCCESS)
    {
        smsLog(pMac, LOGE, FL("Cannot initialize power save check routine list\n"));
        PMC_ABORT;
        return eHAL_STATUS_FAILURE;
    }
    if (csrLLOpen(pMac->hHdd, &pMac->pmc.requestFullPowerList) != eHAL_STATUS_SUCCESS)
    {
        smsLog(pMac, LOGE, FL("Cannot initialize request full power callback routine list\n"));
        PMC_ABORT;
        return eHAL_STATUS_FAILURE;
    }

    /*                                                      */
    if (csrLLOpen(pMac->hHdd, &pMac->pmc.requestBmpsList) !=
      eHAL_STATUS_SUCCESS)
    {
        smsLog(pMac, LOGE, "PMC: cannot initialize request BMPS callback routine list\n");
        return eHAL_STATUS_FAILURE;
    }

    /*                                                             */
    if (csrLLOpen(pMac->hHdd, &pMac->pmc.requestStartUapsdList) != eHAL_STATUS_SUCCESS)
    {
        smsLog(pMac, LOGE, "PMC: cannot initialize request start UAPSD callback routine list\n");
        return eHAL_STATUS_FAILURE;
    }

    /*                                                                        */
    if (csrLLOpen(pMac->hHdd, &pMac->pmc.deviceStateUpdateIndList) != eHAL_STATUS_SUCCESS)
    {
        smsLog(pMac, LOGE, "PMC: cannot initialize device state update indication callback list\n");
        return eHAL_STATUS_FAILURE;
    }

    if (csrLLOpen(pMac->hHdd, &pMac->pmc.deferredMsgList) != eHAL_STATUS_SUCCESS)
    {
        smsLog(pMac, LOGE, FL("Cannot initialize deferred msg list\n"));
        PMC_ABORT;
        return eHAL_STATUS_FAILURE;
    }

    return eHAL_STATUS_SUCCESS;
}


/*                                                                             
 
                 
 
              
                                              
 
             
                                 
 
          
                                           
                                               
 
                                                                             */
eHalStatus pmcStart (tHalHandle hHal)
{
    tpAniSirGlobal pMac = PMAC_STRUCT(hHal);
    tSirMacHTMIMOPowerSaveState  htMimoPowerSaveState;

    smsLog(pMac, LOG2, FL("Entering pmcStart\n"));

    /*                                                */
    pMac->pmc.pmcState = FULL_POWER;
    pMac->pmc.requestFullPowerPending = FALSE;
    pMac->pmc.uapsdSessionRequired = FALSE;
    pMac->pmc.wowlModeRequired = FALSE;
    pMac->pmc.bmpsRequestedByHdd = FALSE;
    pMac->pmc.remainInPowerActiveTillDHCP = FALSE;
    pMac->pmc.remainInPowerActiveThreshold = 0;

    /*                             */
    pMac->pmc.hwWlanSwitchState = ePMC_SWITCH_ON;
    pMac->pmc.swWlanSwitchState = ePMC_SWITCH_ON;

    /*                               */
    pMac->pmc.impsCallbackRoutine = NULL;

    /*                                  */
    pMac->pmc.standbyCallbackRoutine = NULL;

    /*                               */
    pMac->pmc.enterWowlCallbackRoutine = NULL;

    /*                                 */
    pMac->pmc.cLastTxUnicastFrames = 0;
    pMac->pmc.cLastRxUnicastFrames = 0;

    /*                 */
    if (pMac->pmc.smpsEnabled && (pMac->pmc.powerSource != AC_POWER || pMac->pmc.smpsConfig.enterOnAc))
    {
        if (pMac->pmc.smpsConfig.mode == ePMC_DYNAMIC_SMPS)
            htMimoPowerSaveState = eSIR_HT_MIMO_PS_DYNAMIC;
        if (pMac->pmc.smpsConfig.mode == ePMC_STATIC_SMPS)
            htMimoPowerSaveState = eSIR_HT_MIMO_PS_STATIC;
    }
    else
        htMimoPowerSaveState = eSIR_HT_MIMO_PS_NO_LIMIT;
    
    if (pmcSendMessage(hHal, eWNI_PMC_SMPS_STATE_IND, &htMimoPowerSaveState,
                       sizeof(tSirMacHTMIMOPowerSaveState)) != eHAL_STATUS_SUCCESS)
        return eHAL_STATUS_FAILURE;

#ifdef FEATURE_WLAN_DIAG_SUPPORT 
    if (pmcStartDiagEvtTimer(hHal) != eHAL_STATUS_SUCCESS)
    {
       return eHAL_STATUS_FAILURE;
    }
#endif

#if defined(ANI_LOGDUMP)
    pmcDumpInit(hHal);
#endif

    return eHAL_STATUS_SUCCESS;
}


/*                                                                             
 
                
 
              
                                             
 
             
                                 
 
          
                                          
                                              
 
                                                                             */
eHalStatus pmcStop (tHalHandle hHal)
{
    tpAniSirGlobal pMac = PMAC_STRUCT(hHal);
    tListElem *pEntry;
    tPmcDeferredMsg *pDeferredMsg;

    smsLog(pMac, LOG2, FL("Entering pmcStop\n"));

    /*                            */
    if (palTimerStop(pMac->hHdd, pMac->pmc.hImpsTimer) != eHAL_STATUS_SUCCESS)
    {
        smsLog(pMac, LOGE, FL("Cannot cancel IMPS timer\n"));
    }

    pmcStopTrafficTimer(hHal);

#ifdef FEATURE_WLAN_DIAG_SUPPORT    
    pmcStopDiagEvtTimer(hHal);
#endif

    if (palTimerStop(pMac->hHdd, pMac->pmc.hExitPowerSaveTimer) != eHAL_STATUS_SUCCESS)
    {
        smsLog(pMac, LOGE, FL("Cannot cancel exit power save mode timer\n"));
    }

    /*                       */
    pmcDoCallbacks(hHal, eHAL_STATUS_FAILURE);
    pmcDoBmpsCallbacks(hHal, eHAL_STATUS_FAILURE);
    pMac->pmc.uapsdSessionRequired = FALSE;
    pmcDoStartUapsdCallbacks(hHal, eHAL_STATUS_FAILURE);
    pmcDoStandbyCallbacks(hHal, eHAL_STATUS_FAILURE);

    //                           
    csrLLLock( &pMac->pmc.deferredMsgList );
    while( NULL != ( pEntry = csrLLRemoveHead( &pMac->pmc.deferredMsgList, eANI_BOOLEAN_FALSE ) ) )
    {
        pDeferredMsg = GET_BASE_ADDR( pEntry, tPmcDeferredMsg, link );
        palFreeMemory( pMac->hHdd, pDeferredMsg );
    }
    csrLLUnlock( &pMac->pmc.deferredMsgList );

    /*                 */
    pMac->pmc.pmcState = STOPPED;
    pMac->pmc.pmcReady = FALSE;

    return eHAL_STATUS_SUCCESS;
}


/*                                                                             
 
                 
 
              
                                              
 
             
                                 
 
          
                                           
                                               
 
                                                                             */
eHalStatus pmcClose (tHalHandle hHal)
{
    tpAniSirGlobal pMac = PMAC_STRUCT(hHal);

    smsLog(pMac, LOG2, FL("Entering pmcClose\n"));

    /*                              */
    if (palTimerFree(pMac->hHdd, pMac->pmc.hImpsTimer) != eHAL_STATUS_SUCCESS)
    {
        smsLog(pMac, LOGE, FL("Cannot deallocate IMPS timer\n"));
    }
    if (!VOS_IS_STATUS_SUCCESS(vos_timer_destroy(&pMac->pmc.hTrafficTimer)))
    {
        smsLog(pMac, LOGE, FL("Cannot deallocate traffic timer\n"));
    }
#ifdef FEATURE_WLAN_DIAG_SUPPORT    
    if (palTimerFree(pMac->hHdd, pMac->pmc.hDiagEvtTimer) != eHAL_STATUS_SUCCESS)
    {
        smsLog(pMac, LOGE, FL("Cannot deallocate timer for diag event reporting\n"));
    }
#endif
    if (palTimerFree(pMac->hHdd, pMac->pmc.hExitPowerSaveTimer) != eHAL_STATUS_SUCCESS)
    {
        smsLog(pMac, LOGE, FL("Cannot deallocate exit power save mode timer\n"));
    }

    /*
                                                                                      
                        
    */
    pmcClosePowerSaveCheckList(pMac);
    pmcCloseRequestFullPowerList(pMac);
    pmcCloseRequestBmpsList(pMac);
    pmcCloseRequestStartUapsdList(pMac);
    pmcCloseDeviceStateUpdateList(pMac);
    pmcCloseDeferredMsgList(pMac);

    return eHAL_STATUS_SUCCESS;
}


/*                                                                             
 
                            
 
              
                                                    
 
             
                                 
                                        
 
          
                                               
                                                   
 
                                                                             */
eHalStatus pmcSignalPowerEvent (tHalHandle hHal, tPmcPowerEvent event)
{
    tpAniSirGlobal pMac = PMAC_STRUCT(hHal);
#ifndef GEN6_ONWARDS
    tSirMacHTMIMOPowerSaveState  htMimoPowerSaveState;
#endif

    smsLog(pMac, LOG2, FL("Entering pmcSignalPowerEvent, event %d\n"), event);

    /*                                                */
    switch (event)
    {
#ifndef GEN6_ONWARDS
    case ePMC_SYSTEM_HIBERNATE:
        return pmcEnterLowPowerState(hHal);

    case ePMC_SYSTEM_RESUME:
        return pmcExitLowPowerState(hHal);

    case ePMC_HW_WLAN_SWITCH_OFF:
        pMac->pmc.hwWlanSwitchState = ePMC_SWITCH_OFF;
        return pmcEnterLowPowerState(hHal);

    case ePMC_HW_WLAN_SWITCH_ON:
        pMac->pmc.hwWlanSwitchState = ePMC_SWITCH_ON;
        return pmcExitLowPowerState(hHal);

    case ePMC_SW_WLAN_SWITCH_OFF:
        pMac->pmc.swWlanSwitchState = ePMC_SWITCH_OFF;
        return pmcEnterLowPowerState(hHal);

    case ePMC_SW_WLAN_SWITCH_ON:
        pMac->pmc.swWlanSwitchState = ePMC_SWITCH_ON;
        return pmcExitLowPowerState(hHal);

    case ePMC_BATTERY_OPERATION:
        pMac->pmc.powerSource = BATTERY_POWER;

        /*               */
        if (pMac->pmc.smpsEnabled)
        {
            if (pMac->pmc.smpsConfig.mode == ePMC_DYNAMIC_SMPS)
                htMimoPowerSaveState = eSIR_HT_MIMO_PS_DYNAMIC;
            if (pMac->pmc.smpsConfig.mode == ePMC_STATIC_SMPS)
                htMimoPowerSaveState = eSIR_HT_MIMO_PS_STATIC;
            if (pmcSendMessage(hHal, eWNI_PMC_SMPS_STATE_IND, &htMimoPowerSaveState,
                               sizeof(tSirMacHTMIMOPowerSaveState)) != eHAL_STATUS_SUCCESS)   
                return eHAL_STATUS_FAILURE;
        }
        return eHAL_STATUS_SUCCESS;

    case ePMC_AC_OPERATION:
        pMac->pmc.powerSource = AC_POWER;

        /*                */
        if (!pMac->pmc.smpsConfig.enterOnAc)
        {
            htMimoPowerSaveState = eSIR_HT_MIMO_PS_NO_LIMIT;
            if (pmcSendMessage(hHal, eWNI_PMC_SMPS_STATE_IND, &htMimoPowerSaveState,
                               sizeof(tSirMacHTMIMOPowerSaveState)) != eHAL_STATUS_SUCCESS)
                return eHAL_STATUS_FAILURE;
        }
        return eHAL_STATUS_SUCCESS;
#endif //            
    default:
        smsLog(pMac, LOGE, FL("Invalid event %d\n"), event);
        PMC_ABORT;
        return eHAL_STATUS_FAILURE;
    }
}


/*                                                                             
 
                              
 
              
                                              
 
             
                                 
                                                
                                                                        
                                      
 
          
                                                   
                                                       
 
                                                                             */
eHalStatus pmcSetConfigPowerSave (tHalHandle hHal, tPmcPowerSavingMode psMode, void *pConfigParams)
{
    tpAniSirGlobal pMac = PMAC_STRUCT(hHal);

#ifdef FEATURE_WLAN_DIAG_SUPPORT
    WLAN_VOS_DIAG_EVENT_DEF(psRequest, vos_event_wlan_powersave_payload_type);
#endif

    smsLog(pMac, LOG2, FL("Entering pmcSetConfigPowerSave, power save mode %d\n"), psMode);

    /*                                            */
    switch (psMode)
    {
    
    case ePMC_IDLE_MODE_POWER_SAVE:
        pMac->pmc.impsConfig = *(tpPmcImpsConfigParams)pConfigParams;
        smsLog(pMac, LOG3, FL("IMPS configuration"));
        smsLog(pMac, LOG3, "          enter on AC: %d\n",
               pMac->pmc.impsConfig.enterOnAc);
        break;

    case ePMC_BEACON_MODE_POWER_SAVE:
        pMac->pmc.bmpsConfig = *(tpPmcBmpsConfigParams)pConfigParams;
        smsLog(pMac, LOG3, FL("BMPS configuration"));
        smsLog(pMac, LOG3, "          enter on AC: %d\n",
               pMac->pmc.bmpsConfig.enterOnAc);
        smsLog(pMac, LOG3, "          TX threshold: %d\n",
               pMac->pmc.bmpsConfig.txThreshold);
        smsLog(pMac, LOG3, "          RX threshold: %d\n",
               pMac->pmc.bmpsConfig.rxThreshold);
        smsLog(pMac, LOG3, "          traffic measurement period (ms): %d\n",
               pMac->pmc.bmpsConfig.trafficMeasurePeriod);
        smsLog(pMac, LOG3, "          BMPS period: %d\n",
               pMac->pmc.bmpsConfig.bmpsPeriod);
        smsLog(pMac, LOG3, "          beacons to forward code: %d\n",
               pMac->pmc.bmpsConfig.forwardBeacons);
        smsLog(pMac, LOG3, "          value of N: %d\n",
               pMac->pmc.bmpsConfig.valueOfN);
        smsLog(pMac, LOG3, "          use PS poll: %d\n",
               pMac->pmc.bmpsConfig.usePsPoll);
        smsLog(pMac, LOG3, "          set PM on last frame: %d\n",
               pMac->pmc.bmpsConfig.setPmOnLastFrame);
        smsLog(pMac, LOG3, "          value of enableBeaconEarlyTermination: %d\n",
               pMac->pmc.bmpsConfig.enableBeaconEarlyTermination);
        smsLog(pMac, LOG3, "          value of bcnEarlyTermWakeInterval: %d\n",
               pMac->pmc.bmpsConfig.bcnEarlyTermWakeInterval);

#ifdef FEATURE_WLAN_DIAG_SUPPORT    
        vos_mem_zero(&psRequest, sizeof(vos_event_wlan_powersave_payload_type));
        psRequest.event_subtype = WLAN_BMPS_SET_CONFIG;
        /*                                                              
                                                                  */
        psRequest.bmps_auto_timer_duration = (v_U16_t)pMac->pmc.bmpsConfig.trafficMeasurePeriod;
        psRequest.bmps_period = (v_U16_t)pMac->pmc.bmpsConfig.bmpsPeriod; 

        WLAN_VOS_DIAG_EVENT_REPORT(&psRequest, EVENT_WLAN_POWERSAVE_GENERIC);
#endif


        break;

    case ePMC_SPATIAL_MULTIPLEX_POWER_SAVE:
        pMac->pmc.smpsConfig = *(tpPmcSmpsConfigParams)pConfigParams;
        smsLog(pMac, LOG3, FL("SMPS configuration"));
        smsLog(pMac, LOG3, "          mode: %d\n", pMac->pmc.smpsConfig.mode);
        smsLog(pMac, LOG3, "          enter on AC: %d\n",
               pMac->pmc.smpsConfig.enterOnAc);
        break;

    default:
        smsLog(pMac, LOGE, FL("Invalid power save mode %d\n"), psMode);
        PMC_ABORT;
        return eHAL_STATUS_FAILURE;
    }

    //                                                                             
    //                             
    if(PMC_IS_READY(pMac) && psMode == ePMC_BEACON_MODE_POWER_SAVE)
    {
       if (pmcSendPowerSaveConfigMessage(hHal) != eHAL_STATUS_SUCCESS)
           return eHAL_STATUS_FAILURE;
    }

    return eHAL_STATUS_SUCCESS;
}

/*                                                                             
 
                              
 
              
                                                     
 
             
                                 
                                                
                                                                        
                                      
 
          
                                                   
                                                       
 
                                                                             */
eHalStatus pmcGetConfigPowerSave (tHalHandle hHal, tPmcPowerSavingMode psMode, void *pConfigParams)
{
    tpAniSirGlobal pMac = PMAC_STRUCT(hHal);

    smsLog(pMac, LOG2, FL("Entering pmcGetConfigPowerSave, power save mode %d\n"), psMode);

    /*                                            */
    switch (psMode)
    {
    
    case ePMC_IDLE_MODE_POWER_SAVE:
        *(tpPmcImpsConfigParams)pConfigParams = pMac->pmc.impsConfig;
        break;

    case ePMC_BEACON_MODE_POWER_SAVE:
        *(tpPmcBmpsConfigParams)pConfigParams = pMac->pmc.bmpsConfig;
        break;

    case ePMC_SPATIAL_MULTIPLEX_POWER_SAVE:
        *(tpPmcSmpsConfigParams)pConfigParams = pMac->pmc.smpsConfig;
        break;

    default:
        smsLog(pMac, LOGE, FL("Invalid power save mode %d\n"), psMode);
        return eHAL_STATUS_FAILURE;
    }

    return eHAL_STATUS_SUCCESS;
}
/*                                                                             
 
                           
 
              
                                           
 
             
                                 
                                             
 
          
                                               
                                                   
 
                                                                             */
eHalStatus pmcEnablePowerSave (tHalHandle hHal, tPmcPowerSavingMode psMode)
{
    tpAniSirGlobal pMac = PMAC_STRUCT(hHal);
    tSirMacHTMIMOPowerSaveState  htMimoPowerSaveState;

#ifdef FEATURE_WLAN_DIAG_SUPPORT    
    WLAN_VOS_DIAG_EVENT_DEF(psRequest, vos_event_wlan_powersave_payload_type);

    vos_mem_zero(&psRequest, sizeof(vos_event_wlan_powersave_payload_type));
    psRequest.event_subtype = WLAN_PS_MODE_ENABLE_REQ;
    psRequest.enable_disable_powersave_mode = psMode;

    WLAN_VOS_DIAG_EVENT_REPORT(&psRequest, EVENT_WLAN_POWERSAVE_GENERIC);
#endif
    
    smsLog(pMac, LOG2, FL("Entering pmcEnablePowerSave, power save mode %d\n"), psMode);

    /*                                         */
    switch (psMode)
    {

    case ePMC_IDLE_MODE_POWER_SAVE:
        pMac->pmc.impsEnabled = TRUE;
        break;

    case ePMC_BEACON_MODE_POWER_SAVE:
        pMac->pmc.bmpsEnabled = TRUE;
        break;

    case ePMC_SPATIAL_MULTIPLEX_POWER_SAVE:
        pMac->pmc.smpsEnabled = TRUE;

        /*                                            */
        if (pMac->pmc.pmcState != STOPPED)
            if (pMac->pmc.powerSource != AC_POWER ||
                pMac->pmc.smpsConfig.enterOnAc)
            {
                if (pMac->pmc.smpsConfig.mode == ePMC_DYNAMIC_SMPS)
                    htMimoPowerSaveState = eSIR_HT_MIMO_PS_DYNAMIC;
                if (pMac->pmc.smpsConfig.mode == ePMC_STATIC_SMPS)
                    htMimoPowerSaveState = eSIR_HT_MIMO_PS_STATIC;
                if (pmcSendMessage(hHal, eWNI_PMC_SMPS_STATE_IND, &htMimoPowerSaveState,
                                   sizeof(tSirMacHTMIMOPowerSaveState)) != eHAL_STATUS_SUCCESS)
                    return eHAL_STATUS_FAILURE;
            }
        break;

    case ePMC_UAPSD_MODE_POWER_SAVE:
        pMac->pmc.uapsdEnabled = TRUE;
        break;

    case ePMC_STANDBY_MODE_POWER_SAVE:
        pMac->pmc.standbyEnabled = TRUE;
        break;

    case ePMC_WOWL_MODE_POWER_SAVE:
        pMac->pmc.wowlEnabled = TRUE;
        break;

    default:
        smsLog(pMac, LOGE, FL("Invalid power save mode %d\n"), psMode);
        PMC_ABORT;
        return eHAL_STATUS_FAILURE;
    }

    return eHAL_STATUS_SUCCESS;
}
/*                                                                            
                             
                                                                     
                                                                                  
                                                             
                                                  
                           
                                                                             */
eHalStatus pmcStartAutoBmpsTimer (tHalHandle hHal) 
{
   tpAniSirGlobal pMac = PMAC_STRUCT(hHal);

#ifdef FEATURE_WLAN_DIAG_SUPPORT    
   WLAN_VOS_DIAG_EVENT_DEF(psRequest, vos_event_wlan_powersave_payload_type);

   vos_mem_zero(&psRequest, sizeof(vos_event_wlan_powersave_payload_type));
   psRequest.event_subtype = WLAN_START_BMPS_AUTO_TIMER_REQ;

   WLAN_VOS_DIAG_EVENT_REPORT(&psRequest, EVENT_WLAN_POWERSAVE_GENERIC);
#endif

   smsLog(pMac, LOG2, FL("Entering pmcStartAutoBmpsTimer\n"));

   /*                           */
   if (!pMac->pmc.bmpsEnabled)
   {
      smsLog(pMac, LOGE, "PMC: Cannot enable BMPS timer. BMPS is disabled\n");
      return eHAL_STATUS_FAILURE;
   }

   pMac->pmc.autoBmpsEntryEnabled = TRUE;

   /*                                                                                         
                                   */

   if (pmcShouldBmpsTimerRun(pMac))
   {
      if (pmcStartTrafficTimer(hHal, pMac->pmc.bmpsConfig.trafficMeasurePeriod) != eHAL_STATUS_SUCCESS)
         return eHAL_STATUS_FAILURE;
   }



   return eHAL_STATUS_SUCCESS;
}

/*                                                                            
                            
                                                                                   
                                                                                   
                                                                                   
                                                  
                           
                                                                             */
eHalStatus pmcStopAutoBmpsTimer (tHalHandle hHal)
{
   tpAniSirGlobal pMac = PMAC_STRUCT(hHal);

#ifdef FEATURE_WLAN_DIAG_SUPPORT    
   WLAN_VOS_DIAG_EVENT_DEF(psRequest, vos_event_wlan_powersave_payload_type);

   vos_mem_zero(&psRequest, sizeof(vos_event_wlan_powersave_payload_type));
   psRequest.event_subtype = WLAN_STOP_BMPS_AUTO_TIMER_REQ;

   WLAN_VOS_DIAG_EVENT_REPORT(&psRequest, EVENT_WLAN_POWERSAVE_GENERIC);
#endif

   smsLog(pMac, LOG2, FL("Entering pmcStopAutoBmpsTimer\n"));

   pMac->pmc.autoBmpsEntryEnabled = FALSE;
   /*                                                                                          */
   if (!pMac->pmc.uapsdSessionRequired && !pMac->pmc.bmpsRequestedByHdd)
      pmcStopTrafficTimer(hHal);

   return eHAL_STATUS_SUCCESS;
}

/*                                                                             
 
                            
 
              
                                            
 
             
                                 
                                              
 
          
                                                
                                                    
 
                                                                             */
eHalStatus pmcDisablePowerSave (tHalHandle hHal, tPmcPowerSavingMode psMode)
{
    tpAniSirGlobal pMac = PMAC_STRUCT(hHal);
    tSirMacHTMIMOPowerSaveState  htMimoPowerSaveState;

#ifdef FEATURE_WLAN_DIAG_SUPPORT    
    WLAN_VOS_DIAG_EVENT_DEF(psRequest, vos_event_wlan_powersave_payload_type);

    vos_mem_zero(&psRequest, sizeof(vos_event_wlan_powersave_payload_type));
    psRequest.event_subtype = WLAN_PS_MODE_DISABLE_REQ;
    psRequest.enable_disable_powersave_mode = psMode;

    WLAN_VOS_DIAG_EVENT_REPORT(&psRequest, EVENT_WLAN_POWERSAVE_GENERIC);
#endif

    smsLog(pMac, LOG2, FL("Entering pmcDisablePowerSave, power save mode %d\n"), psMode);

    /*                                          */
    switch (psMode)
    {

    case ePMC_IDLE_MODE_POWER_SAVE:
        pMac->pmc.impsEnabled = FALSE;
        break;

    case ePMC_BEACON_MODE_POWER_SAVE:
        pMac->pmc.bmpsEnabled = FALSE;
        break;

    case ePMC_SPATIAL_MULTIPLEX_POWER_SAVE:
        pMac->pmc.smpsEnabled = FALSE;

        /*                */
        htMimoPowerSaveState = eSIR_HT_MIMO_PS_NO_LIMIT;
        if (pmcSendMessage(hHal, eWNI_PMC_SMPS_STATE_IND, &htMimoPowerSaveState,
                           sizeof(tSirMacHTMIMOPowerSaveState)) != eHAL_STATUS_SUCCESS)
            return eHAL_STATUS_FAILURE;
        break;

    case ePMC_UAPSD_MODE_POWER_SAVE:
        pMac->pmc.uapsdEnabled = FALSE;
        break;

    case ePMC_STANDBY_MODE_POWER_SAVE:
        pMac->pmc.standbyEnabled = FALSE;
        break;

    case ePMC_WOWL_MODE_POWER_SAVE:
        pMac->pmc.wowlEnabled = FALSE;
        break;

    default:
        smsLog(pMac, LOGE, FL("Invalid power save mode %d\n"), psMode);
        PMC_ABORT;
        return eHAL_STATUS_FAILURE;
    }

    return eHAL_STATUS_SUCCESS;
}


/*                                                                             
 
                           
 
              
                                                   
 
             
                                 
                                                            
                                                                     
                                      
                                                                     
                                      
 
          
                                                            
                                                                
 
                                                                             */
eHalStatus pmcQueryPowerState (tHalHandle hHal, tPmcPowerState *pPowerState,
                               tPmcSwitchState *pHwWlanSwitchState, tPmcSwitchState *pSwWlanSwitchState)
{
    tpAniSirGlobal pMac = PMAC_STRUCT(hHal);

    smsLog(pMac, LOG2, FL("Entering pmcQueryPowerState\n"));

    /*                                                */
    if(pPowerState != NULL)
    {
        /*                                                */
        switch (pMac->pmc.pmcState)
        {
    
        case FULL_POWER:
            *pPowerState = ePMC_FULL_POWER;
            break;

        default:
            *pPowerState = ePMC_LOW_POWER;
            break;
        }
    }

    /*                                 */
    if(pHwWlanSwitchState != NULL)
       *pHwWlanSwitchState = pMac->pmc.hwWlanSwitchState;
    if(pSwWlanSwitchState != NULL)
       *pSwWlanSwitchState = pMac->pmc.swWlanSwitchState;

    return eHAL_STATUS_SUCCESS;
}


/*                                                                             
 
                              
 
              
                                                                       
                                                                        
                                                                      
                                                                        
                                                                      
 
             
                                 
                                   
 
          
                                                                         
 
                                                                             */
tANI_BOOLEAN pmcIsPowerSaveEnabled (tHalHandle hHal, tPmcPowerSavingMode psMode)
{
    tpAniSirGlobal pMac = PMAC_STRUCT(hHal);

    smsLog(pMac, LOG2, FL("Entering pmcIsPowerSaveEnabled, power save mode %d\n"), psMode);

    /*                                                                  */
    switch (psMode)
    {
    
    case ePMC_IDLE_MODE_POWER_SAVE:
        return pMac->pmc.impsEnabled && (pMac->pmc.powerSource != AC_POWER || pMac->pmc.impsConfig.enterOnAc);

    case ePMC_BEACON_MODE_POWER_SAVE:
        return pMac->pmc.bmpsEnabled;

    case ePMC_SPATIAL_MULTIPLEX_POWER_SAVE:
        return pMac->pmc.smpsEnabled && (pMac->pmc.powerSource != AC_POWER || pMac->pmc.smpsConfig.enterOnAc);

    case ePMC_UAPSD_MODE_POWER_SAVE:
        return pMac->pmc.uapsdEnabled;

    case ePMC_STANDBY_MODE_POWER_SAVE:
        return pMac->pmc.standbyEnabled;

    case ePMC_WOWL_MODE_POWER_SAVE:
        return pMac->pmc.wowlEnabled;
        break;

    default:
        smsLog(pMac, LOGE, FL("Invalid power save mode %d\n"), psMode);
        PMC_ABORT;
        return FALSE;
    }
}


/*                                                                             
 
                            
 
              
                                                            
 
             
                                 
                                                                         
                                                                           
                                                                           
                            
                                                                           
                                                                           
                                                                        
                                                                             
                                                                             
                                          
 
          
                                                             
                                                                       
                                                                           
                                                                            
 
                                                                             */
eHalStatus pmcRequestFullPower (tHalHandle hHal, void (*callbackRoutine) (void *callbackContext, eHalStatus status),
                                void *callbackContext, tRequestFullPowerReason fullPowerReason)
{
    tpAniSirGlobal pMac = PMAC_STRUCT(hHal);
    tpRequestFullPowerEntry pEntry;

#ifdef FEATURE_WLAN_DIAG_SUPPORT    
    WLAN_VOS_DIAG_EVENT_DEF(psRequest, vos_event_wlan_powersave_payload_type);

    vos_mem_zero(&psRequest, sizeof(vos_event_wlan_powersave_payload_type));
    psRequest.event_subtype = WLAN_ENTER_FULL_POWER_REQ;
    psRequest.full_power_request_reason = fullPowerReason;
 
    WLAN_VOS_DIAG_EVENT_REPORT(&psRequest, EVENT_WLAN_POWERSAVE_GENERIC);
#endif

    smsLog(pMac, LOG2, FL("Entering pmcRequestFullPower"));

    if( !PMC_IS_READY(pMac) )
    {
        smsLog(pMac, LOGE, FL("Requesting Full Power when PMC not ready\n"));
        smsLog(pMac, LOGE, FL("pmcReady = %d pmcState = %s\n"), 
            pMac->pmc.pmcReady, pmcGetPmcStateStr(pMac->pmc.pmcState));
        return eHAL_STATUS_FAILURE;
    }

    /*                                                                                         
                                   */
    if(SIR_IS_FULL_POWER_NEEDED_BY_HDD(fullPowerReason))
    {
        pMac->pmc.bmpsRequestedByHdd = FALSE;
        pMac->pmc.wowlModeRequired = FALSE;
    }

    /*                                        */
    if (pMac->pmc.pmcState == FULL_POWER)
        return eHAL_STATUS_SUCCESS;

    /*                                          */
    if (pMac->pmc.pmcState == IMPS)
        if (palTimerStop(pMac->hHdd, pMac->pmc.hImpsTimer) != eHAL_STATUS_SUCCESS)
        {
            smsLog(pMac, LOGE, FL("Cannot cancel IMPS timer\n"));
            return eHAL_STATUS_FAILURE;
        }

    /*                                 */
    if (pmcEnterRequestFullPowerState(hHal, fullPowerReason) != eHAL_STATUS_SUCCESS)
        return eHAL_STATUS_FAILURE;

    /*                                                                    
                                                                    */
    //                                                            
    if( callbackRoutine )
    {
        if (palAllocateMemory(pMac->hHdd, (void **)&pEntry, sizeof(tRequestFullPowerEntry)) != eHAL_STATUS_SUCCESS)
        {
            smsLog(pMac, LOGE, FL("Cannot allocate memory for request full power routine list entry\n"));
            PMC_ABORT;
            return eHAL_STATUS_FAILURE;
        }

        /*                                     */
        pEntry->callbackRoutine = callbackRoutine;
        pEntry->callbackContext = callbackContext;

        /*                    */
        csrLLInsertTail(&pMac->pmc.requestFullPowerList, &pEntry->link, TRUE);
    }

    return eHAL_STATUS_PMC_PENDING;
}


/*                                                                             
 
                       
 
              
                                                                      
                                                                        
                                                                         
                   
 
             
                                 
                                                                 
                                                                        
                                                                
                                                                           
                            
 
          
                                                 
                                                
                                                                          
                                           
                                                                         
                                        
                                                                
                                                                          
 
                                                                             */
eHalStatus pmcRequestImps (tHalHandle hHal, tANI_U32 impsPeriod,
                           void (*callbackRoutine) (void *callbackContext, eHalStatus status),
                           void *callbackContext)
{
    tpAniSirGlobal pMac = PMAC_STRUCT(hHal);
    eHalStatus status;

#ifdef FEATURE_WLAN_DIAG_SUPPORT    
    WLAN_VOS_DIAG_EVENT_DEF(psRequest, vos_event_wlan_powersave_payload_type);

    vos_mem_zero(&psRequest, sizeof(vos_event_wlan_powersave_payload_type));
    psRequest.event_subtype = WLAN_IMPS_ENTER_REQ;
    psRequest.imps_period = impsPeriod;

    WLAN_VOS_DIAG_EVENT_REPORT(&psRequest, EVENT_WLAN_POWERSAVE_GENERIC);
#endif


    smsLog(pMac, LOG2, FL("Entering pmcRequestImps"));

    status = pmcEnterImpsCheck( pMac );
    if( HAL_STATUS_SUCCESS( status ) )
    {
        /*                           */
        status = pmcEnterRequestImpsState( hHal );
        if (HAL_STATUS_SUCCESS( status ))
    {
            /*                                                           */
            pMac->pmc.impsPeriod = impsPeriod;
            pMac->pmc.impsCallbackRoutine = callbackRoutine;
            pMac->pmc.impsCallbackContext = callbackContext;

    }
        else
    {
            status = eHAL_STATUS_PMC_SYS_ERROR;
    }
    }

    return status;
}


/*                                                                             
 
                                  
 
              
                                                                             
                                                                            
                                                                           
                          
 
             
                                 
                                                                             
                                                                           
                                              
                                                                              
 
          
                                                  
                                                      
 
                                                                             */
eHalStatus pmcRegisterPowerSaveCheck (tHalHandle hHal, tANI_BOOLEAN (*checkRoutine) (void *checkContext),
                                      void *checkContext)
{
    tpAniSirGlobal pMac = PMAC_STRUCT(hHal);
    tpPowerSaveCheckEntry pEntry;

    smsLog(pMac, LOG2, FL("Entering pmcRegisterPowerSaveCheck"));

    /*                                                   */
    if (palAllocateMemory(pMac->hHdd, (void **)&pEntry, sizeof(tPowerSaveCheckEntry)) != eHAL_STATUS_SUCCESS)
    {
        smsLog(pMac, LOGE, FL("Cannot allocate memory for power save check routine list entry\n"));
        PMC_ABORT;
        return eHAL_STATUS_FAILURE;
    }

    /*                                     */
    pEntry->checkRoutine = checkRoutine;
    pEntry->checkContext = checkContext;

    /*                    */
    csrLLInsertTail(&pMac->pmc.powerSaveCheckList, &pEntry->link, FALSE);

    return eHAL_STATUS_SUCCESS;
}


/*                                                                             
 
                                    
 
              
                                                              
                               
 
             
                                 
                                         
 
          
                                                    
                                                        
 
                                                                             */
eHalStatus pmcDeregisterPowerSaveCheck (tHalHandle hHal, tANI_BOOLEAN (*checkRoutine) (void *checkContext))
{
    tpAniSirGlobal pMac = PMAC_STRUCT(hHal);
    tListElem *pEntry;
    tpPowerSaveCheckEntry pPowerSaveCheckEntry;

    smsLog(pMac, LOG2, FL("Entering pmcDeregisterPowerSaveCheck"));

    /*                                                             
                                            */
    pEntry = csrLLPeekHead(&pMac->pmc.powerSaveCheckList, FALSE);
    while (pEntry != NULL)
    {
        pPowerSaveCheckEntry = GET_BASE_ADDR(pEntry, tPowerSaveCheckEntry, link);
        if (pPowerSaveCheckEntry->checkRoutine == checkRoutine)
        {
            if (csrLLRemoveEntry(&pMac->pmc.powerSaveCheckList, pEntry, FALSE))
            {
                if (palFreeMemory(pMac->hHdd, pPowerSaveCheckEntry) != eHAL_STATUS_SUCCESS)
                {
                    smsLog(pMac, LOGE, FL("Cannot free memory for power save check routine list entry\n"));
                    PMC_ABORT;
                    return eHAL_STATUS_FAILURE;
                }
            }
            else
            {
                smsLog(pMac, LOGE, FL("Cannot remove power save check routine list entry\n"));
                return eHAL_STATUS_FAILURE;
            }
            return eHAL_STATUS_SUCCESS;
        }
        pEntry = csrLLNext(&pMac->pmc.powerSaveCheckList, pEntry, FALSE);
    }

    /*                                */
    return eHAL_STATUS_FAILURE;
}


static void pmcProcessResponse( tpAniSirGlobal pMac, tSirSmeRsp *pMsg )
{
    tListElem *pEntry = NULL;
    tSmeCmd *pCommand = NULL;
    tANI_BOOLEAN fRemoveCommand = eANI_BOOLEAN_TRUE;

    pEntry = csrLLPeekHead(&pMac->sme.smeCmdActiveList, LL_ACCESS_LOCK);
    if(pEntry)
    {
        pCommand = GET_BASE_ADDR(pEntry, tSmeCmd, Link);

        smsLog(pMac, LOG2, FL("process message = %d\n"), pMsg->messageType);

    /*                                         */
    switch (pMsg->messageType)
    {

    /*                                         */
    case eWNI_PMC_ENTER_IMPS_RSP:
        smsLog(pMac, LOG2, FL("Rcvd eWNI_PMC_ENTER_IMPS_RSP with status = %d\n"), pMsg->statusCode);
            if( (eSmeCommandEnterImps != pCommand->command) && (eSmeCommandEnterStandby != pCommand->command) )
            {
                smsLog(pMac, LOGW, FL("Rcvd eWNI_PMC_ENTER_IMPS_RSP without request\n"));
                fRemoveCommand = eANI_BOOLEAN_FALSE;
                break;
            }
        if(pMac->pmc.pmcState == REQUEST_IMPS)
        {
            /*                                                 */
            if (pMsg->statusCode == eSIR_SME_SUCCESS)
            {
                    pmcEnterImpsState(pMac);
            }

            /*                                                                                                            */
            else {
                smsLog(pMac, LOGE, FL("Response message to request to enter IMPS indicates failure, status %x\n"),
                       pMsg->statusCode);
                    pmcEnterFullPowerState(pMac);
            }
        }
        else if (pMac->pmc.pmcState == REQUEST_STANDBY)
        {
            /*                                                    */
            if (pMsg->statusCode == eSIR_SME_SUCCESS)
            {
                    pmcEnterStandbyState(pMac);
                    pmcDoStandbyCallbacks(pMac, eHAL_STATUS_SUCCESS);
            }

            /*                                                         
                                                                    */
            else
            {
                smsLog(pMac, LOGE, "PMC: response message to request to enter "
                       "standby indicates failure, status %x\n", pMsg->statusCode);
                    pmcEnterFullPowerState(pMac);
                    pmcDoStandbyCallbacks(pMac, eHAL_STATUS_FAILURE);
            }
        }
        else
        {
            smsLog(pMac, LOGE, "PMC: Enter IMPS rsp rcvd when device is "
               "in %d state\n", pMac->pmc.pmcState);
        }
        break;

    /*                                                  */
    case eWNI_PMC_EXIT_IMPS_RSP:
            smsLog(pMac, LOG2, FL("Rcvd eWNI_PMC_EXIT_IMPS_RSP with status = %d\n"), pMsg->statusCode);
            if( eSmeCommandExitImps != pCommand->command )
            {
                smsLog(pMac, LOGW, FL("Rcvd eWNI_PMC_EXIT_IMPS_RSP without request\n"));
                fRemoveCommand = eANI_BOOLEAN_FALSE;
                break;
            }
            /*                                                          */
            if (pMac->pmc.pmcState != REQUEST_FULL_POWER)
            {
                smsLog(pMac, LOGE, FL("Got Exit IMPS Response Message while in state %d\n"), pMac->pmc.pmcState);
                break;
            }

            /*                         */
            if (pMsg->statusCode != eSIR_SME_SUCCESS)
            {
                smsLog(pMac, LOGP, FL("Response message to request to exit IMPS indicates failure, status %x\n"),
                       pMsg->statusCode);
            }
            pmcEnterFullPowerState(pMac);
        break;

    /*                                         */
    case eWNI_PMC_ENTER_BMPS_RSP:
            smsLog(pMac, LOG2, FL("Rcvd eWNI_PMC_ENTER_BMPS_RSP with status = %d\n"), pMsg->statusCode);
            if( eSmeCommandEnterBmps != pCommand->command )
            {
                smsLog(pMac, LOGW, FL("Rcvd eWNI_PMC_ENTER_BMPS_RSP without request\n"));
                fRemoveCommand = eANI_BOOLEAN_FALSE;
                break;
            }
            pMac->pmc.bmpsRequestQueued = eANI_BOOLEAN_FALSE;
            /*                                                          */
            if (pMac->pmc.pmcState != REQUEST_BMPS)
            {
                smsLog(pMac, LOGE, FL("Got Enter BMPS Response Message while in state %d\n"), pMac->pmc.pmcState);
                break;
            }

        /*                                                 */
        if (pMsg->statusCode == eSIR_SME_SUCCESS)
        {
                pmcEnterBmpsState(pMac);
            /*                                                    
                                                                   
                                                 */
                pmcDoBmpsCallbacks(pMac, eHAL_STATUS_SUCCESS);
         }
        /*                                                                                                            */
        else
        {
                smsLog(pMac, LOGE, FL("Response message to request to enter BMPS indicates failure, status %x\n"),
                   pMsg->statusCode);
                pmcEnterFullPowerState(pMac);
                //                                                          
                pmcDoBmpsCallbacks(pMac, eHAL_STATUS_FAILURE);
        }
        break;

    /*                                                  */
    case eWNI_PMC_EXIT_BMPS_RSP:
            smsLog(pMac, LOG2, FL("Rcvd eWNI_PMC_EXIT_BMPS_RSP with status = %d\n"), pMsg->statusCode);
            if( eSmeCommandExitBmps != pCommand->command )
            {
                smsLog(pMac, LOGW, FL("Rcvd eWNI_PMC_EXIT_BMPS_RSP without request\n"));
                fRemoveCommand = eANI_BOOLEAN_FALSE;
                break;
            }
            /*                                                          */
            if (pMac->pmc.pmcState != REQUEST_FULL_POWER)
            {
                smsLog(pMac, LOGE, FL("Got Exit BMPS Response Message while in state %d\n"), pMac->pmc.pmcState);
                break;
            }

            /*                         */
            if (pMsg->statusCode != eSIR_SME_SUCCESS)
            {
                smsLog(pMac, LOGP, FL("Response message to request to exit BMPS indicates failure, status %x\n"),
                       pMsg->statusCode);
            }
            pmcEnterFullPowerState(pMac);
        break;

        /*                                                */
        case eWNI_PMC_ENTER_UAPSD_RSP:
            smsLog(pMac, LOG2, FL("Rcvd eWNI_PMC_ENTER_UAPSD_RSP with status = %d\n"), pMsg->statusCode);
            if( eSmeCommandEnterUapsd != pCommand->command )
        {
                smsLog(pMac, LOGW, FL("Rcvd eWNI_PMC_ENTER_UAPSD_RSP without request\n"));
                fRemoveCommand = eANI_BOOLEAN_FALSE;
                break;
            }
            /*                                                          */
            if (pMac->pmc.pmcState != REQUEST_START_UAPSD)
            {
                smsLog(pMac, LOGE, FL("Got Enter Uapsd rsp Message while in state %d\n"), pMac->pmc.pmcState);
                break;
            }

         /*                                                  */
            if (pMsg->statusCode == eSIR_SME_SUCCESS) 
            {
                pmcEnterUapsdState(pMac);
                pmcDoStartUapsdCallbacks(pMac, eHAL_STATUS_SUCCESS);
         }
         /*                                                            
                     */
            else {
                smsLog(pMac, LOGE, "PMC: response message to request to enter "
                   "UAPSD indicates failure, status %x\n", pMsg->statusCode);
                //                                                                           
                pMac->pmc.uapsdSessionRequired = FALSE;
                pmcEnterBmpsState(pMac);
                //                                                                                
                pmcDoStartUapsdCallbacks(pMac, eHAL_STATUS_FAILURE);
         }
         break;

      /*                                               */
      case eWNI_PMC_EXIT_UAPSD_RSP:
         smsLog(pMac, LOG2, FL("Rcvd eWNI_PMC_EXIT_UAPSD_RSP with status = %d\n"), pMsg->statusCode);
            if( eSmeCommandExitUapsd != pCommand->command )
            {
                smsLog(pMac, LOGW, FL("Rcvd eWNI_PMC_EXIT_UAPSD_RSP without request\n"));
                fRemoveCommand = eANI_BOOLEAN_FALSE;
                break;
            }
            /*                                                          */
            if (pMac->pmc.pmcState != REQUEST_STOP_UAPSD)
            {
                smsLog(pMac, LOGE, FL("Got Exit Uapsd rsp Message while in state %d\n"), pMac->pmc.pmcState);
                break;
            }

         /*                  */
         if (pMsg->statusCode != eSIR_SME_SUCCESS) {
            smsLog(pMac, LOGP, "PMC: response message to request to exit "
               "UAPSD indicates failure, status %x\n", pMsg->statusCode);
         }
            pmcEnterBmpsState(pMac);
         break;

      /*                                               */
      case eWNI_PMC_ENTER_WOWL_RSP:

            if( eSmeCommandEnterWowl != pCommand->command )
            {
                smsLog(pMac, LOGW, FL("Rcvd eWNI_PMC_ENTER_WOWL_RSP without request\n"));
                fRemoveCommand = eANI_BOOLEAN_FALSE;
                break;
            }
            /*                                                          */
            if (pMac->pmc.pmcState != REQUEST_ENTER_WOWL)
            {
                smsLog(pMac, LOGE, FL("Got eWNI_PMC_ENTER_WOWL_RSP while in state %s\n"), 
                    pmcGetPmcStateStr(pMac->pmc.pmcState));
                break;
            }

         /*                                                 */
         if (pMsg->statusCode == eSIR_SME_SUCCESS) {
                pmcEnterWowlState(pMac);
                pmcDoEnterWowlCallbacks(pMac, eHAL_STATUS_SUCCESS);
         }

         /*                                                            
                     */
         else {
            smsLog(pMac, LOGE, "PMC: response message to request to enter "
               "WOWL indicates failure, status %x\n", pMsg->statusCode);
                pmcEnterBmpsState(pMac);
                pmcDoEnterWowlCallbacks(pMac, eHAL_STATUS_FAILURE);
         }
         break;

      /*                                              */
      case eWNI_PMC_EXIT_WOWL_RSP:

            if( eSmeCommandExitWowl != pCommand->command )
            {
                smsLog(pMac, LOGW, FL("Rcvd eWNI_PMC_EXIT_WOWL_RSP without request\n"));
                fRemoveCommand = eANI_BOOLEAN_FALSE;
                break;
            }
            /*                                                          */
            if (pMac->pmc.pmcState != REQUEST_EXIT_WOWL)
            {
                smsLog(pMac, LOGE, FL("Got Exit WOWL rsp Message while in state %d\n"), pMac->pmc.pmcState);
                break;
            }

         /*                  */
         if (pMsg->statusCode != eSIR_SME_SUCCESS) {
            smsLog(pMac, LOGP, "PMC: response message to request to exit "
               "WOWL indicates failure, status %x\n", pMsg->statusCode);
         }
            pmcEnterBmpsState(pMac);
         break;

    default:
        smsLog(pMac, LOGE, FL("Invalid message type %d received\n"), pMsg->messageType);
        PMC_ABORT;
        break;
        }//      

        if( fRemoveCommand )
        {
            if( csrLLRemoveEntry( &pMac->sme.smeCmdActiveList, pEntry, LL_ACCESS_LOCK ) )
            {
                pmcReleaseCommand( pMac, pCommand );
                smeProcessPendingQueue( pMac );
            }
        }
    }
    else
    {
        smsLog(pMac, LOGE, FL("message type %d received but no request is found\n"), pMsg->messageType);
    }
}


/*                                                                             
 
                            
 
              
                                       
 
             
                                 
                                       
 
          
            
 
                                                                             */
void pmcMessageProcessor (tHalHandle hHal, tSirSmeRsp *pMsg)
{
    tpAniSirGlobal pMac = PMAC_STRUCT(hHal);

    smsLog(pMac, LOG2, FL("Entering pmcMessageProcessor, message type %d\n"), pMsg->messageType);

    switch( pMsg->messageType )
    {
    case eWNI_PMC_EXIT_BMPS_IND:
    //                                                                             
    {
        /*                              */
        smsLog(pMac, LOGW, FL("Rcvd eWNI_PMC_EXIT_BMPS_IND with status = %d\n"), pMsg->statusCode);
        /*                                                          */
        switch(pMac->pmc.pmcState)
        {
        case BMPS:
        case REQUEST_START_UAPSD:
        case UAPSD:
        case REQUEST_STOP_UAPSD:
        case REQUEST_ENTER_WOWL:
        case WOWL:
        case REQUEST_EXIT_WOWL:
        case REQUEST_FULL_POWER:
            smsLog(pMac, LOGW, FL("Got eWNI_PMC_EXIT_BMPS_IND while in state %d\n"), pMac->pmc.pmcState);
            break;
        default:
            smsLog(pMac, LOGE, FL("Got eWNI_PMC_EXIT_BMPS_IND while in state %d\n"), pMac->pmc.pmcState);
            PMC_ABORT;
            break;
        }

        /*                         */
        if (pMsg->statusCode != eSIR_SME_SUCCESS)
        {
            smsLog(pMac, LOGP, FL("Exit BMPS indication indicates failure, status %x\n"), pMsg->statusCode);
        }
        else
        {
            tpSirSmeExitBmpsInd pExitBmpsInd = (tpSirSmeExitBmpsInd)pMsg;
            pmcEnterRequestFullPowerState(hHal, pExitBmpsInd->exitBmpsReason);
        }
        break;
    }

    default:
        pmcProcessResponse( pMac, pMsg );
        break;
    }

}


tANI_BOOLEAN pmcValidateConnectState( tHalHandle hHal )
{
   tpAniSirGlobal pMac = PMAC_STRUCT(hHal);

   if ( !csrIsInfraConnected( pMac ) )
   {
      smsLog(pMac, LOGW, "PMC: STA not associated. BMPS cannot be entered\n");
      return eANI_BOOLEAN_FALSE;
   }

   //                         
   if ( csrIsIBSSStarted( pMac ) )
   {
      smsLog(pMac, LOGW, "PMC: IBSS started. BMPS cannot be entered\n");
      return eANI_BOOLEAN_FALSE;
   }
   if ( csrIsBTAMPStarted( pMac ) )
   {
      smsLog(pMac, LOGW, "PMC: BT-AMP exists. BMPS cannot be entered\n");
      return eANI_BOOLEAN_FALSE;
   }
   if ((vos_concurrent_sessions_running()) &&
       (csrIsConcurrentInfraConnected( pMac ) ||
       (vos_get_concurrency_mode()& VOS_SAP) ||
       (vos_get_concurrency_mode()& VOS_P2P_GO)))
   {
      smsLog(pMac, LOGW, "PMC: Multiple active sessions exists. BMPS cannot be entered\n");
      return eANI_BOOLEAN_FALSE;
   }
   return eANI_BOOLEAN_TRUE;
}

tANI_BOOLEAN pmcAllowImps( tHalHandle hHal )
{
    tpAniSirGlobal pMac = PMAC_STRUCT(hHal);

    //                                                     
    if ( csrIsIBSSStarted( pMac ) )
    {
       smsLog(pMac, LOGW, "PMC: IBSS started. IMPS cannot be entered\n");
       return eANI_BOOLEAN_FALSE;
    }
    if ( csrIsBTAMPStarted( pMac ) )
    {
       smsLog(pMac, LOGW, "PMC: BT-AMP exists. IMPS cannot be entered\n");
       return eANI_BOOLEAN_FALSE;
    }

    //                                               
    if ( !csrIsAllSessionDisconnected( pMac ) )
    {
       smsLog(pMac, LOGW, "PMC: Atleast one connected session. IMPS cannot be entered\n");
       return eANI_BOOLEAN_FALSE;
    }

    return eANI_BOOLEAN_TRUE;
}

/*                                                                             
 
                       
 
              
                                                  
 
             
                                 
                                                                          
                                                                           
                            
 
          
                                                  
                                                                 
                                                                     
 
                                                                             */
eHalStatus pmcRequestBmps (
    tHalHandle hHal,
    void (*callbackRoutine) (void *callbackContext, eHalStatus status),
    void *callbackContext)
{
   tpAniSirGlobal pMac = PMAC_STRUCT(hHal);
   tpRequestBmpsEntry pEntry;
   eHalStatus status;

#ifdef FEATURE_WLAN_DIAG_SUPPORT    
   WLAN_VOS_DIAG_EVENT_DEF(psRequest, vos_event_wlan_powersave_payload_type);

   vos_mem_zero(&psRequest, sizeof(vos_event_wlan_powersave_payload_type));
   psRequest.event_subtype = WLAN_BMPS_ENTER_REQ;

   WLAN_VOS_DIAG_EVENT_REPORT(&psRequest, EVENT_WLAN_POWERSAVE_GENERIC);
#endif

   smsLog(pMac, LOG2, "PMC: entering pmcRequestBmps");

   /*                                  */
   if (pMac->pmc.pmcState == BMPS || REQUEST_START_UAPSD == pMac->pmc.pmcState || UAPSD == pMac->pmc.pmcState)
   {
      smsLog(pMac, LOG2, "PMC: Device already in BMPS pmcState %d", pMac->pmc.pmcState);
      pMac->pmc.bmpsRequestedByHdd = TRUE;
      return eHAL_STATUS_SUCCESS;
   }
   
   status = pmcEnterBmpsCheck( pMac );
   if(HAL_STATUS_SUCCESS( status ))
   {
      status = pmcEnterRequestBmpsState(hHal);
      /*                           */
      if ( HAL_STATUS_SUCCESS( status ) )
      {
         /*                                                                    
                                                                                      
                                               */
         pMac->pmc.bmpsRequestedByHdd = TRUE;

         /*                                                              
                                                                   */
         if (palAllocateMemory(
               pMac->hHdd, (void **)&pEntry,
               sizeof(tRequestBmpsEntry)) != eHAL_STATUS_SUCCESS)
         {
            smsLog(pMac, LOGE, "PMC: cannot allocate memory for request "
                  "BMPS routine list entry\n");
            return eHAL_STATUS_FAILURE;
         }

         /*                                     */
         pEntry->callbackRoutine = callbackRoutine;
         pEntry->callbackContext = callbackContext;

         /*                    */
         csrLLInsertTail(&pMac->pmc.requestBmpsList, &pEntry->link, FALSE);

         status = eHAL_STATUS_PMC_PENDING;
      }
      else
      {
         status = eHAL_STATUS_FAILURE;
      }
   }

   return status;
}

/*                                                                             
 
                      
 
              
                                                   
 
             
                                 
                                                                          
                                                                           
                            
 
          
                                                   
                                                                  
                                                                     
                                                                          
 
                                                                             */
eHalStatus pmcStartUapsd (
    tHalHandle hHal,
    void (*callbackRoutine) (void *callbackContext, eHalStatus status),
    void *callbackContext)
{
   tpAniSirGlobal pMac = PMAC_STRUCT(hHal);
   tpStartUapsdEntry pEntry;

#ifdef FEATURE_WLAN_DIAG_SUPPORT    
   WLAN_VOS_DIAG_EVENT_DEF(psRequest, vos_event_wlan_powersave_payload_type);

   vos_mem_zero(&psRequest, sizeof(vos_event_wlan_powersave_payload_type));
   psRequest.event_subtype = WLAN_UAPSD_START_REQ;

   WLAN_VOS_DIAG_EVENT_REPORT(&psRequest, EVENT_WLAN_POWERSAVE_GENERIC);
#endif

   smsLog(pMac, LOG2, "PMC: entering pmcStartUapsd");

   if( !PMC_IS_READY(pMac) )
   {
       smsLog(pMac, LOGE, FL("Requesting UAPSD when PMC not ready\n"));
       smsLog(pMac, LOGE, FL("pmcReady = %d pmcState = %s\n"), 
           pMac->pmc.pmcReady, pmcGetPmcStateStr(pMac->pmc.pmcState));
       return eHAL_STATUS_FAILURE;
   }

   /*                           */
   if (!pMac->pmc.bmpsEnabled)
   {
      smsLog(pMac, LOGE, "PMC: Cannot enter UAPSD. BMPS is disabled\n");
      return eHAL_STATUS_PMC_DISABLED;
   }

   /*                            */
   if (!pMac->pmc.uapsdEnabled)
   {
      smsLog(pMac, LOGE, "PMC: Cannot enter UAPSD. UAPSD is disabled\n");
      return eHAL_STATUS_PMC_DISABLED;
   }

   /*                                   */
   if (pMac->pmc.pmcState == UAPSD)
      return eHAL_STATUS_SUCCESS;

   /*                               */
   if (!pmcValidateConnectState( pMac ))
   {
      smsLog(pMac, LOGE, "PMC: STA not associated with an AP. UAPSD cannot be entered\n");
      return eHAL_STATUS_FAILURE;
   }

   /*                                  */
   if (pmcEnterRequestStartUapsdState(hHal) != eHAL_STATUS_SUCCESS)
      return eHAL_STATUS_FAILURE;

   if( NULL != callbackRoutine )
   {
      /*                                                                               */
      if (palAllocateMemory(pMac->hHdd, (void **)&pEntry,
            sizeof(tStartUapsdEntry)) != eHAL_STATUS_SUCCESS)
      {
         smsLog(pMac, LOGE, "PMC: cannot allocate memory for request "
            "start UAPSD routine list entry\n");
         return eHAL_STATUS_FAILURE;
      }

      /*                                     */
      pEntry->callbackRoutine = callbackRoutine;
      pEntry->callbackContext = callbackContext;

      /*                    */
      csrLLInsertTail(&pMac->pmc.requestStartUapsdList, &pEntry->link, FALSE);
   }

   return eHAL_STATUS_PMC_PENDING;
}

/*                                                                             
 
                     
 
              
                                                       
 
             
                                 
 
          
                                                                            
                                                                      
 
                                                                             */
eHalStatus pmcStopUapsd (tHalHandle hHal)
{
   tpAniSirGlobal pMac = PMAC_STRUCT(hHal);

#ifdef FEATURE_WLAN_DIAG_SUPPORT    
   WLAN_VOS_DIAG_EVENT_DEF(psRequest, vos_event_wlan_powersave_payload_type);

   vos_mem_zero(&psRequest, sizeof(vos_event_wlan_powersave_payload_type));
   psRequest.event_subtype = WLAN_UAPSD_STOP_REQ;

   WLAN_VOS_DIAG_EVENT_REPORT(&psRequest, EVENT_WLAN_POWERSAVE_GENERIC);
#endif

   smsLog(pMac, LOG2, "PMC: entering pmcStopUapsd");

   /*                                               */
   pMac->pmc.uapsdSessionRequired = FALSE;

   /*                                                                      
                                                           */
   if (pMac->pmc.pmcState != UAPSD && pMac->pmc.pmcState != REQUEST_STOP_UAPSD)
   {
      smsLog(pMac, LOGW, "PMC: Device is already out of UAPSD "
         "state. Current state is %d\n", pMac->pmc.pmcState);
      return eHAL_STATUS_SUCCESS;
   }

   /*                               */
   if (pmcEnterRequestStopUapsdState(hHal) != eHAL_STATUS_SUCCESS)
      return eHAL_STATUS_FAILURE;

   return eHAL_STATUS_SUCCESS;
}

/*                                                                            
                         
                                                      
                                                  
                                                                                 
                                                                         
                        
                                                     
                                                                
                                                                   
                                                                             */
extern eHalStatus pmcRequestStandby (
   tHalHandle hHal,
   void (*callbackRoutine) (void *callbackContext, eHalStatus status),
   void *callbackContext)
{
   tpAniSirGlobal pMac = PMAC_STRUCT(hHal);

#ifdef FEATURE_WLAN_DIAG_SUPPORT    
   WLAN_VOS_DIAG_EVENT_DEF(psRequest, vos_event_wlan_powersave_payload_type);

   vos_mem_zero(&psRequest, sizeof(vos_event_wlan_powersave_payload_type));
   psRequest.event_subtype = WLAN_ENTER_STANDBY_REQ;

   WLAN_VOS_DIAG_EVENT_REPORT(&psRequest, EVENT_WLAN_POWERSAVE_GENERIC);
#endif

   smsLog(pMac, LOG2, "PMC: entering pmcRequestStandby");

   /*                              */
   if (!pMac->pmc.standbyEnabled)
   {
      smsLog(pMac, LOGE, "PMC: Cannot enter standby. Standby is disabled\n");
      return eHAL_STATUS_PMC_DISABLED;
   }

   if( !PMC_IS_READY(pMac) )
   {
       smsLog(pMac, LOGE, FL("Requesting standby when PMC not ready\n"));
       smsLog(pMac, LOGE, FL("pmcReady = %d pmcState = %s\n"), 
           pMac->pmc.pmcReady, pmcGetPmcStateStr(pMac->pmc.pmcState));
       return eHAL_STATUS_FAILURE;
   }

   /*                                     */
   if (pMac->pmc.pmcState == STANDBY)
      return eHAL_STATUS_SUCCESS;

   
   if (csrIsIBSSStarted(pMac) || csrIsBTAMPStarted(pMac))
   {
      VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_FATAL, 
          "WLAN: IBSS or BT-AMP session present. Cannot honor standby request");
      return eHAL_STATUS_PMC_NOT_NOW;
   }

   /*                              */
   if (pmcEnterRequestStandbyState(hHal) != eHAL_STATUS_SUCCESS)
      return eHAL_STATUS_FAILURE;

   /*                                                */
   pMac->pmc.standbyCallbackRoutine = callbackRoutine;
   pMac->pmc.standbyCallbackContext = callbackContext;

   return eHAL_STATUS_PMC_PENDING;
}

/*                                                                            
                                       
                                                               
                                                                          
                                                  
                                                                
                                                                       
                      
                                                         
                                                               
                                                                             */
extern eHalStatus pmcRegisterDeviceStateUpdateInd (tHalHandle hHal, 
   void (*callbackRoutine) (void *callbackContext, tPmcState pmcState),
   void *callbackContext)
{

    tpAniSirGlobal pMac = PMAC_STRUCT(hHal);
    tpDeviceStateUpdateIndEntry pEntry;

    smsLog(pMac, LOG2, FL("Entering pmcRegisterDeviceStateUpdateInd"));

    /*                                                          */
    if (palAllocateMemory(pMac->hHdd, (void **)&pEntry, sizeof(tDeviceStateUpdateIndEntry)) != eHAL_STATUS_SUCCESS)
    {
        smsLog(pMac, LOGE, FL("Cannot allocate memory for device power state update indication\n"));
        PMC_ABORT;
        return eHAL_STATUS_FAILURE;
    }

    /*                         */
    pEntry->callbackRoutine = callbackRoutine;
    pEntry->callbackContext = callbackContext;

    /*                    */
    csrLLInsertTail(&pMac->pmc.deviceStateUpdateIndList, &pEntry->link, FALSE);

    return eHAL_STATUS_SUCCESS;
}

/*                                                                            
                                         
                                                                             
                                                  
                                                                  
                      
                                                           
                                                                 
                                                                             */
eHalStatus pmcDeregisterDeviceStateUpdateInd (tHalHandle hHal, 
   void (*callbackRoutine) (void *callbackContext, tPmcState pmcState))
{
    tpAniSirGlobal pMac = PMAC_STRUCT(hHal);
    tListElem *pEntry;
    tpDeviceStateUpdateIndEntry pDeviceStateUpdateIndEntry;

    smsLog(pMac, LOG2, FL("Entering pmcDeregisterDeviceStateUpdateInd"));

    /*                                                              
                                            */
    pEntry = csrLLPeekHead(&pMac->pmc.deviceStateUpdateIndList, FALSE);
    while (pEntry != NULL)
    {
        pDeviceStateUpdateIndEntry = GET_BASE_ADDR(pEntry, tDeviceStateUpdateIndEntry, link);
        if (pDeviceStateUpdateIndEntry->callbackRoutine == callbackRoutine)
        {
            if (!csrLLRemoveEntry(&pMac->pmc.deviceStateUpdateIndList, pEntry, FALSE))
            {
                smsLog(pMac, LOGE, FL("Cannot remove device state update ind entry from list\n"));
                return eHAL_STATUS_FAILURE;
            }
            if (palFreeMemory(pMac->hHdd, pDeviceStateUpdateIndEntry) != eHAL_STATUS_SUCCESS)
            {
                smsLog(pMac, LOGE, FL("Cannot free memory for device state update ind routine list entry\n"));
                PMC_ABORT;
                return eHAL_STATUS_FAILURE;
            }
            return eHAL_STATUS_SUCCESS;
        }
        pEntry = csrLLNext(&pMac->pmc.deviceStateUpdateIndList, pEntry, FALSE);
    }

    /*                                */
    return eHAL_STATUS_FAILURE;
}

/*                                                                            
                
                                                                             
                                                                              
                                                                                
                                                                              
                         
                                                  
                      
                                                                             */
eHalStatus pmcReady(tHalHandle hHal)
{
    tpAniSirGlobal pMac = PMAC_STRUCT(hHal);

    smsLog(pMac, LOG2, FL("Entering pmcReady"));

    if(pMac->pmc.pmcState == STOPPED)
    {
        smsLog(pMac, LOGP, FL("pmcReady is invoked even before pmcStart"));
        return eHAL_STATUS_FAILURE;
    }

    pMac->pmc.pmcReady = TRUE;
    if (pmcSendPowerSaveConfigMessage(hHal) != eHAL_STATUS_SUCCESS)
    {
        return eHAL_STATUS_FAILURE;
    }

    return eHAL_STATUS_SUCCESS;
}

/*                                                                            
                              
                                                                               
                                                                                 
                                                                           
                                                             
                                                  
                                                         
                      
                                                   
                                                   
                                                                             */
eHalStatus pmcWowlAddBcastPattern (
    tHalHandle hHal, 
    tpSirWowlAddBcastPtrn pattern)
{
    tpAniSirGlobal pMac = PMAC_STRUCT(hHal);

#ifdef FEATURE_WLAN_DIAG_SUPPORT
    vos_log_powersave_wow_add_ptrn_pkt_type *log_ptr = NULL;
    WLAN_VOS_DIAG_LOG_ALLOC(log_ptr, vos_log_powersave_wow_add_ptrn_pkt_type, LOG_WLAN_POWERSAVE_WOW_ADD_PTRN_C);
#endif //                                

    smsLog(pMac, LOG2, "PMC: entering pmcWowlAddBcastPattern");

    if(pattern == NULL)
    {
        smsLog(pMac, LOGE, FL("Null broadcast pattern being passed\n"));
        return eHAL_STATUS_FAILURE;
    }

#ifdef FEATURE_WLAN_DIAG_SUPPORT
    if( log_ptr )
    {
       log_ptr->pattern_id = pattern->ucPatternId;
       log_ptr->pattern_byte_offset = pattern->ucPatternByteOffset;
       log_ptr->pattern_size = pattern->ucPatternSize;
       log_ptr->pattern_mask_size = pattern->ucPatternMaskSize;

       vos_mem_copy(log_ptr->pattern, pattern->ucPattern, SIR_WOWL_BCAST_PATTERN_MAX_SIZE);
       /*                                                                                    */
       vos_mem_copy(log_ptr->pattern_mask, pattern->ucPatternMask, SIR_WOWL_BCAST_PATTERN_MAX_SIZE >> 3);
    }

    WLAN_VOS_DIAG_LOG_REPORT(log_ptr);
#endif


    if(pattern->ucPatternId >= SIR_WOWL_BCAST_MAX_NUM_PATTERNS )
    {
        smsLog(pMac, LOGE, FL("Pattern Id must range from 0 to %d\n"), SIR_WOWL_BCAST_MAX_NUM_PATTERNS-1);
        return eHAL_STATUS_FAILURE;
    }

    if( pMac->pmc.pmcState == STANDBY || pMac->pmc.pmcState == REQUEST_STANDBY )
    {
        smsLog(pMac, LOGE, FL("Cannot add WoWL Pattern as chip is in %s state\n"),
           pmcGetPmcStateStr(pMac->pmc.pmcState));
        return eHAL_STATUS_FAILURE;
    }
    if( pMac->pmc.pmcState == IMPS || pMac->pmc.pmcState == REQUEST_IMPS )
    {
        //                      
        eHalStatus status = pmcDeferMsg( pMac, eWNI_PMC_WOWL_ADD_BCAST_PTRN, 
                                    pattern, sizeof(tSirWowlAddBcastPtrn) );

        if( eHAL_STATUS_PMC_PENDING == status )
        {
            return eHAL_STATUS_SUCCESS;
        }
        else 
        {
            //                                    
            if( !HAL_STATUS_SUCCESS( status ) )
            {
                return ( status );
            }
            //                                                     
        }
    }

    if (pmcSendMessage(hHal, eWNI_PMC_WOWL_ADD_BCAST_PTRN, pattern, sizeof(tSirWowlAddBcastPtrn))
        != eHAL_STATUS_SUCCESS)
    {
        smsLog(pMac, LOGE, FL("Send of eWNI_PMC_WOWL_ADD_BCAST_PTRN to PE failed\n"));
        return eHAL_STATUS_FAILURE;
    }

    return eHAL_STATUS_SUCCESS;
}

/*                                                                            
                              
                                                                      
                                                  
                                            
                      
                                                      
                                                   
                                                                             */
eHalStatus pmcWowlDelBcastPattern (
    tHalHandle hHal, 
    tpSirWowlDelBcastPtrn pattern)
{
    tpAniSirGlobal pMac = PMAC_STRUCT(hHal);

#ifdef FEATURE_WLAN_DIAG_SUPPORT    
    WLAN_VOS_DIAG_EVENT_DEF(wowRequest, vos_event_wlan_powersave_wow_payload_type);

    vos_mem_zero(&wowRequest, sizeof(vos_event_wlan_powersave_wow_payload_type));
    wowRequest.event_subtype = WLAN_WOW_DEL_PTRN_REQ;
    wowRequest.wow_del_ptrn_id = pattern->ucPatternId; 

    WLAN_VOS_DIAG_EVENT_REPORT(&wowRequest, EVENT_WLAN_POWERSAVE_WOW);
#endif

    smsLog(pMac, LOG2, "PMC: entering pmcWowlDelBcastPattern");

    if(pattern->ucPatternId >= SIR_WOWL_BCAST_MAX_NUM_PATTERNS )
    {
        smsLog(pMac, LOGE, FL("Pattern Id must range from 0 to %d\n"), 
            SIR_WOWL_BCAST_MAX_NUM_PATTERNS-1);
        return eHAL_STATUS_FAILURE;
    }

    if(pMac->pmc.pmcState == STANDBY || pMac->pmc.pmcState == REQUEST_STANDBY)
    {
        smsLog(pMac, LOGE, FL("Cannot delete WoWL Pattern as chip is in %s state\n"),
           pmcGetPmcStateStr(pMac->pmc.pmcState));
        return eHAL_STATUS_FAILURE;
    }

    if( pMac->pmc.pmcState == IMPS || pMac->pmc.pmcState == REQUEST_IMPS )
    {
        //                      
        eHalStatus status = pmcDeferMsg( pMac, eWNI_PMC_WOWL_DEL_BCAST_PTRN, 
                                    pattern, sizeof(tSirWowlDelBcastPtrn) );

        if( eHAL_STATUS_PMC_PENDING == status )
        {
            return eHAL_STATUS_SUCCESS;
        }
        else 
        {
            //                                    
            if( !HAL_STATUS_SUCCESS( status ) )
            {
                return ( status );
            }
            //                                                     
        }
    }

    if (pmcSendMessage(hHal, eWNI_PMC_WOWL_DEL_BCAST_PTRN, pattern, sizeof(tSirWowlDelBcastPtrn))
        != eHAL_STATUS_SUCCESS)
    {
        smsLog(pMac, LOGE, FL("Send of eWNI_PMC_WOWL_DEL_BCAST_PTRN to PE failed\n"));
        return eHAL_STATUS_FAILURE;
    }

    return eHAL_STATUS_SUCCESS;
}

/*                                                                            
                    
                                                                   
                                                                          
                                                                                    
                                                                                        
                                                                                        
                                                                                        
                                                                                        
                                
                                                                                         
                                                                                         
                     
                                                                                     
                                               
                                       
                                      
                                               
                                               
                                                                                         
                                                           
                                                                                
                                                  
                                                  
                                                                                          
                                                                                  
                                                                                 
                                                                                
                                                                                  
                                                      
                                
                                                             
                                                                       
                                                                           
                                                                             */
eHalStatus pmcEnterWowl ( 
    tHalHandle hHal, 
    void (*enterWowlCallbackRoutine) (void *callbackContext, eHalStatus status),
    void *enterWowlCallbackContext,
#ifdef WLAN_WAKEUP_EVENTS
    void (*wakeReasonIndCB) (void *callbackContext, tpSirWakeReasonInd pWakeReasonInd),
    void *wakeReasonIndCBContext,
#endif //                   
    tpSirSmeWowlEnterParams wowlEnterParams)
{
   tpAniSirGlobal pMac = PMAC_STRUCT(hHal);

#ifdef FEATURE_WLAN_DIAG_SUPPORT    
   WLAN_VOS_DIAG_EVENT_DEF(wowRequest, vos_event_wlan_powersave_wow_payload_type);

   vos_mem_zero(&wowRequest, sizeof(vos_event_wlan_powersave_wow_payload_type));
   wowRequest.event_subtype = WLAN_WOW_ENTER_REQ;
   wowRequest.wow_type = 0;

   if(wowlEnterParams->ucMagicPktEnable)
   {
       wowRequest.wow_type |= 1;
       vos_mem_copy(wowRequest.wow_magic_pattern, (tANI_U8 *)wowlEnterParams->magicPtrn, 6);
   }

   if(wowlEnterParams->ucPatternFilteringEnable)
   {
       wowRequest.wow_type |= 2;
   }

   WLAN_VOS_DIAG_EVENT_REPORT(&wowRequest, EVENT_WLAN_POWERSAVE_WOW);
#endif

   smsLog(pMac, LOG2, "PMC: entering pmcEnterWowl\n");

   if( !PMC_IS_READY(pMac) )
   {
       smsLog(pMac, LOGE, FL("Requesting WoWL when PMC not ready\n"));
       smsLog(pMac, LOGE, FL("pmcReady = %d pmcState = %s\n"), 
           pMac->pmc.pmcReady, pmcGetPmcStateStr(pMac->pmc.pmcState));
       return eHAL_STATUS_FAILURE;
   }

   /*                           */
   if (!pMac->pmc.bmpsEnabled)
   {
      smsLog(pMac, LOGE, "PMC: Cannot enter WoWL. BMPS is disabled\n");
      return eHAL_STATUS_PMC_DISABLED;
   }

   /*                           */
   if (!pMac->pmc.wowlEnabled)
   {
      smsLog(pMac, LOGE, "PMC: Cannot enter WoWL. WoWL is disabled\n");
      return eHAL_STATUS_PMC_DISABLED;
   }

   /*                                                   */
   if (!pmcValidateConnectState( pMac ))
   {
      smsLog(pMac, LOGE, "PMC: Cannot enable WOWL. STA not associated "
             "with an Access Point in Infra Mode with single active session\n");
      return eHAL_STATUS_FAILURE;
   }

   /*                                                                
                                                              */
   if(pMac->pmc.uapsdSessionRequired)
   {
      smsLog(pMac, LOGE, "PMC: Cannot request WOWL. Pending UAPSD request\n");
      return eHAL_STATUS_FAILURE;
   }

   /*                                                                  */
   if (pMac->pmc.pmcState == FULL_POWER && !pmcPowerSaveCheck(hHal))
   {
      smsLog(pMac, LOGE, "PMC: Power save check failed. WOWL request "
             "will not be accepted\n");
      return eHAL_STATUS_FAILURE;
   }

   //                                                                       
   /*                            */
   pMac->pmc.wowlEnterParams = *wowlEnterParams;
   pMac->pmc.enterWowlCallbackRoutine = enterWowlCallbackRoutine;
   pMac->pmc.enterWowlCallbackContext = enterWowlCallbackContext;
#ifdef WLAN_WAKEUP_EVENTS
   /*                                                       */
   pMac->pmc.wakeReasonIndCB = wakeReasonIndCB;
   pMac->pmc.wakeReasonIndCBContext = wakeReasonIndCBContext;
#endif //                   

   /*                           */
   if (pmcRequestEnterWowlState(hHal, wowlEnterParams) != eHAL_STATUS_SUCCESS)
      return eHAL_STATUS_FAILURE;

   pMac->pmc.wowlModeRequired = TRUE;

   return eHAL_STATUS_PMC_PENDING;
}

/*                                                                            
                   
                                                                                 
                                                                                   
                 
                                                  
                      
                                                                
                                                                       
                                                                             */
eHalStatus pmcExitWowl (tHalHandle hHal)
{
   tpAniSirGlobal pMac = PMAC_STRUCT(hHal);

#ifdef FEATURE_WLAN_DIAG_SUPPORT    
   WLAN_VOS_DIAG_EVENT_DEF(wowRequest, vos_event_wlan_powersave_wow_payload_type);

   vos_mem_zero(&wowRequest, sizeof(vos_event_wlan_powersave_wow_payload_type));
   wowRequest.event_subtype = WLAN_WOW_EXIT_REQ;

   WLAN_VOS_DIAG_EVENT_REPORT(&wowRequest, EVENT_WLAN_POWERSAVE_WOW);
#endif

   smsLog(pMac, LOG2, "PMC: entering pmcExitWowl");

   /*                                              */
   pMac->pmc.wowlModeRequired = FALSE;

   /*                              */
   if (pmcRequestExitWowlState(hHal) != eHAL_STATUS_SUCCESS)
      return eHAL_STATUS_FAILURE;

   /*                             */
   pMac->pmc.enterWowlCallbackRoutine = NULL;
   pMac->pmc.enterWowlCallbackContext = NULL;

   return eHAL_STATUS_SUCCESS;
}



/*                                                                            
                         
                                         
                                                  
                                                      
                      
                                                        
                                                   
                                                                             */
eHalStatus pmcSetHostOffload (tHalHandle hHal, tpSirHostOffloadReq pRequest, tANI_U8 *bssId)
{
    tpSirHostOffloadReq pRequestBuf;
    vos_msg_t msg;
    tpPESession     psessionEntry;
    tANI_U8   sessionId;
    tpAniSirGlobal pMac = PMAC_STRUCT(hHal);

    if((psessionEntry = peFindSessionByBssid( pMac, bssId,&sessionId)) == NULL )
    {
        VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_ERROR, "%s: Unable to find"
        "the psessionEntry for bssid : 0x%x:0x%x:0x%x:0x%x:0x%x:0x%x", 
        __FUNCTION__,bssId[5],bssId[4],bssId[3],bssId[2],bssId[1],bssId[0]);
        return eHAL_STATUS_FAILURE;
    }

    VOS_TRACE( VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_INFO, "%s: IP address = %d.%d.%d.%d", __FUNCTION__,
        pRequest->params.hostIpv4Addr[0], pRequest->params.hostIpv4Addr[1],
        pRequest->params.hostIpv4Addr[2], pRequest->params.hostIpv4Addr[3]);

    pRequestBuf = vos_mem_malloc(sizeof(tSirHostOffloadReq));
    if (NULL == pRequestBuf)
    {
        VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_ERROR, "%s: Not able to allocate memory for host offload request", __FUNCTION__);
        return eHAL_STATUS_FAILED_ALLOC;
    }
    vos_mem_copy(pRequestBuf, pRequest, sizeof(tSirHostOffloadReq));

    pRequestBuf->bssIdx = psessionEntry->bssIdx;

    msg.type = WDA_SET_HOST_OFFLOAD;
    msg.reserved = 0;
    msg.bodyptr = pRequestBuf;
    if(VOS_STATUS_SUCCESS != vos_mq_post_message(VOS_MODULE_ID_WDA, &msg))
    {
        VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_ERROR, "%s: Not able to post WDA_SET_HOST_OFFLOAD message to WDA", __FUNCTION__);
        vos_mem_free(pRequestBuf);
        return eHAL_STATUS_FAILURE;
    }

    return eHAL_STATUS_SUCCESS;
}

/*                                                                            
                       
                                       
                                                  
                                                 
                      
                                                          
                                                   
                                                                             */
eHalStatus pmcSetKeepAlive (tHalHandle hHal, tpSirKeepAliveReq pRequest, tANI_U8 *bssId)
{
    tpSirKeepAliveReq pRequestBuf;
    vos_msg_t msg;
    tpPESession     psessionEntry;
    tANI_U8   sessionId;
    tpAniSirGlobal pMac = PMAC_STRUCT(hHal);
    if((psessionEntry = peFindSessionByBssid( pMac, bssId,&sessionId)) == NULL )
    {
        VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_ERROR, "%s: Unable to find"
        "the psessionEntry for bssid : 0x%x:0x%x:0x%x:0x%x:0x%x:0x%x", 
        __FUNCTION__,bssId[5],bssId[4],bssId[3],bssId[2],bssId[1],bssId[0]);
        return eHAL_STATUS_FAILURE;
    }

    VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_INFO_LOW, "%s: "
                  "WDA_SET_KEEP_ALIVE message", __FUNCTION__);

    pRequestBuf = vos_mem_malloc(sizeof(tSirKeepAliveReq));
    if (NULL == pRequestBuf)
    {
        VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_ERROR, "%s: "
                  "Not able to allocate memory for keep alive request",
                  __FUNCTION__);
        return eHAL_STATUS_FAILED_ALLOC;
    }
    vos_mem_copy(pRequestBuf, pRequest, sizeof(tSirKeepAliveReq));

    pRequestBuf->bssIdx = psessionEntry->bssIdx;

    VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_INFO_LOW, "buff TP %d "
              "input TP %d ", pRequestBuf->timePeriod, pRequest->timePeriod);

    msg.type = WDA_SET_KEEP_ALIVE;
    msg.reserved = 0;
    msg.bodyptr = pRequestBuf;
    if(VOS_STATUS_SUCCESS != vos_mq_post_message(VOS_MODULE_ID_WDA, &msg))
    {
        VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_ERROR, "%s: "
                  "Not able to post WDA_SET_KEEP_ALIVE message to WDA",
                  __FUNCTION__);
        vos_mem_free(pRequestBuf);
        return eHAL_STATUS_FAILURE;
    }

    return eHAL_STATUS_SUCCESS;
}


#ifdef WLAN_NS_OFFLOAD

/*                                                                            
                       
                                         
                                                  
                                                      
                      
                                                        
                                                   
                                                                             */
eHalStatus pmcSetNSOffload (tHalHandle hHal, tpSirHostOffloadReq pRequest, tANI_U8 *bssId)
{
    tpSirHostOffloadReq pRequestBuf;
    vos_msg_t msg;
    int i;
    tpPESession     psessionEntry;
    tANI_U8   sessionId;
    tpAniSirGlobal pMac = PMAC_STRUCT(hHal);
    if((psessionEntry = peFindSessionByBssid( pMac, bssId,&sessionId)) == NULL )
    {
        VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_ERROR, "%s: Unable to find"
        "the psessionEntry for bssid : 0x%x:0x%x:0x%x:0x%x:0x%x:0x%x", 
        __FUNCTION__,bssId[5],bssId[4],bssId[3],bssId[2],bssId[1],bssId[0]);
        return eHAL_STATUS_FAILURE;
    }

    pRequestBuf = vos_mem_malloc(sizeof(tSirHostOffloadReq));
    if (NULL == pRequestBuf)
    {
        VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_ERROR, "%s: Not able to allocate memory for NS offload request", __FUNCTION__);
        return eHAL_STATUS_FAILED_ALLOC;
    }
    vos_mem_copy(pRequestBuf, pRequest, sizeof(tSirHostOffloadReq));

    pRequestBuf->bssIdx = psessionEntry->bssIdx;

    msg.type = WDA_SET_NS_OFFLOAD;
    msg.reserved = 0;
    msg.bodyptr = pRequestBuf;
    if(VOS_STATUS_SUCCESS != vos_mq_post_message(VOS_MODULE_ID_WDA, &msg))
    {
        VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_ERROR, "%s: Not able to post SIR_HAL_SET_HOST_OFFLOAD message to HAL", __FUNCTION__);
        vos_mem_free(pRequestBuf);
        return eHAL_STATUS_FAILURE;
    }

    return eHAL_STATUS_SUCCESS;
}

#endif //               


void pmcClosePowerSaveCheckList(tpAniSirGlobal pMac)
{
    tListElem *pEntry;
    tpPowerSaveCheckEntry pPowerSaveCheckEntry;

    csrLLLock(&pMac->pmc.powerSaveCheckList);
    while ( (pEntry = csrLLRemoveHead(&pMac->pmc.powerSaveCheckList, FALSE)) )
    {
        pPowerSaveCheckEntry = GET_BASE_ADDR(pEntry, tPowerSaveCheckEntry, link);
        if (palFreeMemory(pMac->hHdd, pPowerSaveCheckEntry) != eHAL_STATUS_SUCCESS)
        {
            smsLog(pMac, LOGE, FL("Cannot free memory \n"));
            PMC_ABORT;
            break;
        }
    }
    csrLLUnlock(&pMac->pmc.powerSaveCheckList);
    csrLLClose(&pMac->pmc.powerSaveCheckList);
}


void pmcCloseRequestFullPowerList(tpAniSirGlobal pMac)
{
    tListElem *pEntry;
    tpRequestFullPowerEntry pRequestFullPowerEntry;

    csrLLLock(&pMac->pmc.requestFullPowerList);
    while ( (pEntry = csrLLRemoveHead(&pMac->pmc.requestFullPowerList, FALSE)) )
    {
        pRequestFullPowerEntry = GET_BASE_ADDR(pEntry, tRequestFullPowerEntry, link);
        if (palFreeMemory(pMac->hHdd, pRequestFullPowerEntry) != eHAL_STATUS_SUCCESS)
        {
            smsLog(pMac, LOGE, FL("Cannot free memory \n"));
            PMC_ABORT;
            break;
        }
    }
    csrLLUnlock(&pMac->pmc.requestFullPowerList);
    csrLLClose(&pMac->pmc.requestFullPowerList);
}


void pmcCloseRequestBmpsList(tpAniSirGlobal pMac)
{
    tListElem *pEntry;
    tpRequestBmpsEntry pRequestBmpsEntry;

    csrLLLock(&pMac->pmc.requestBmpsList);
    while ( (pEntry = csrLLRemoveHead(&pMac->pmc.requestBmpsList, FALSE)) )
    {
        pRequestBmpsEntry = GET_BASE_ADDR(pEntry, tRequestBmpsEntry, link);
        if (palFreeMemory(pMac->hHdd, pRequestBmpsEntry) != eHAL_STATUS_SUCCESS)
        {
            smsLog(pMac, LOGE, FL("Cannot free memory \n"));
            PMC_ABORT;
            break;
        }
    }
    csrLLUnlock(&pMac->pmc.requestBmpsList);
    csrLLClose(&pMac->pmc.requestBmpsList);
}


void pmcCloseRequestStartUapsdList(tpAniSirGlobal pMac)
{
    tListElem *pEntry;
    tpStartUapsdEntry pStartUapsdEntry;

    csrLLLock(&pMac->pmc.requestStartUapsdList);
    while ( (pEntry = csrLLRemoveHead(&pMac->pmc.requestStartUapsdList, FALSE)) )
    {
        pStartUapsdEntry = GET_BASE_ADDR(pEntry, tStartUapsdEntry, link);
        if (palFreeMemory(pMac->hHdd, pStartUapsdEntry) != eHAL_STATUS_SUCCESS)
        {
            smsLog(pMac, LOGE, FL("Cannot free memory \n"));
            PMC_ABORT;
            break;
        }
    }
    csrLLUnlock(&pMac->pmc.requestStartUapsdList);
    csrLLClose(&pMac->pmc.requestStartUapsdList);
}


void pmcCloseDeviceStateUpdateList(tpAniSirGlobal pMac)
{
    tListElem *pEntry;
    tpDeviceStateUpdateIndEntry pDeviceStateUpdateIndEntry;

    csrLLLock(&pMac->pmc.deviceStateUpdateIndList);
    while ( (pEntry = csrLLRemoveHead(&pMac->pmc.deviceStateUpdateIndList, FALSE)) )
    {
        pDeviceStateUpdateIndEntry = GET_BASE_ADDR(pEntry, tDeviceStateUpdateIndEntry, link);
        if (palFreeMemory(pMac->hHdd, pDeviceStateUpdateIndEntry) != eHAL_STATUS_SUCCESS)
        {
            smsLog(pMac, LOGE, FL("Cannot free memory \n"));
            PMC_ABORT;
            break;
        }
    }
    csrLLUnlock(&pMac->pmc.deviceStateUpdateIndList);
    csrLLClose(&pMac->pmc.deviceStateUpdateIndList);
}


void pmcCloseDeferredMsgList(tpAniSirGlobal pMac)
{
    tListElem *pEntry;
    tPmcDeferredMsg *pDeferredMsg;

    csrLLLock(&pMac->pmc.deferredMsgList);
    while ( (pEntry = csrLLRemoveHead(&pMac->pmc.deferredMsgList, FALSE)) )
    {
        pDeferredMsg = GET_BASE_ADDR(pEntry, tPmcDeferredMsg, link);
        if (palFreeMemory(pMac->hHdd, pDeferredMsg) != eHAL_STATUS_SUCCESS)
        {
            smsLog(pMac, LOGE, FL("Cannot free memory \n"));
            PMC_ABORT;
            break;
        }
    }
    csrLLUnlock(&pMac->pmc.deferredMsgList);
    csrLLClose(&pMac->pmc.deferredMsgList);
}


#ifdef FEATURE_WLAN_SCAN_PNO

static tSirRetStatus 
pmcPopulateMacHeader( tpAniSirGlobal pMac,
                      tANI_U8* pBD,
                      tANI_U8 type,
                      tANI_U8 subType,
                      tSirMacAddr peerAddr ,
                      tSirMacAddr selfMacAddr)
{
    tSirRetStatus   statusCode = eSIR_SUCCESS;
    tpSirMacMgmtHdr pMacHdr;
    
    //                               
    pMacHdr = (tpSirMacMgmtHdr) (pBD);

    //           
    pMacHdr->fc.protVer = SIR_MAC_PROTOCOL_VERSION;
    pMacHdr->fc.type    = type;
    pMacHdr->fc.subType = subType;

    //                  
    palCopyMemory( pMac->hHdd,
                   (tANI_U8 *) pMacHdr->da,
                   (tANI_U8 *) peerAddr,
                   sizeof( tSirMacAddr ));

    sirCopyMacAddr(pMacHdr->sa,selfMacAddr);

    //                  
    palCopyMemory( pMac->hHdd,
                   (tANI_U8 *) pMacHdr->bssId,
                   (tANI_U8 *) peerAddr,
                   sizeof( tSirMacAddr ));
    return statusCode;
} /*                            */


static tSirRetStatus
pmcPrepareProbeReqTemplate(tpAniSirGlobal pMac,
                           tANI_U8        nChannelNum,
                           tANI_U32       dot11mode,
                           tSirMacAddr    selfMacAddr,
                           tANI_U8        *pFrame,
                           tANI_U16       *pusLen)
{
    tDot11fProbeRequest pr;
    tANI_U32            nStatus, nBytes, nPayload;
    tSirRetStatus       nSirStatus;
    /*        */
    tSirMacAddr         bssId = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
    /*                                                                */

    //                                                                 
    //                                                      
    //                                                              
    palZeroMemory( pMac->hHdd, ( tANI_U8* )&pr, sizeof( pr ) );

    PopulateDot11fSuppRates( pMac, nChannelNum, &pr.SuppRates,NULL);

    if ( WNI_CFG_DOT11_MODE_11B != dot11mode )
    {
        PopulateDot11fExtSuppRates1( pMac, nChannelNum, &pr.ExtSuppRates );
    }

    
    if (IS_DOT11_MODE_HT(dot11mode))
    {
       PopulateDot11fHTCaps( pMac, NULL, &pr.HTCaps );
    }
    
    //                                                                   
    //      
    nStatus = dot11fGetPackedProbeRequestSize( pMac, &pr, &nPayload );
    if ( DOT11F_FAILED( nStatus ) )
    {
        VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_ERROR,
                  "Failed to calculate the packed size f"
                  "or a Probe Request (0x%08x).\n", nStatus );

        //                                            
        nPayload = sizeof( tDot11fProbeRequest );
    }
    else if ( DOT11F_WARNED( nStatus ) )
    {
        VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_ERROR,
                  "There were warnings while calculating"
                  "the packed size for a Probe Request ("
                  "0x%08x).\n", nStatus );
    }

    nBytes = nPayload + sizeof( tSirMacMgmtHdr );
  
    /*                       */
    palZeroMemory( pMac->hHdd, pFrame, nBytes );

    //                                         
    nSirStatus = pmcPopulateMacHeader( pMac, pFrame, SIR_MAC_MGMT_FRAME,
                                SIR_MAC_MGMT_PROBE_REQ, bssId ,selfMacAddr);

    if ( eSIR_SUCCESS != nSirStatus )
    {
        VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_ERROR,
        "Failed to populate the buffer descriptor for a Probe Request (%d).\n",
                nSirStatus );
        return nSirStatus;      //           
    }

    //                                   
    nStatus = dot11fPackProbeRequest( pMac, &pr, pFrame +
                                      sizeof( tSirMacMgmtHdr ),
                                      nPayload, &nPayload );
    if ( DOT11F_FAILED( nStatus ) )
    {
        VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_ERROR,
                  "Failed to pack a Probe Request (0x%08x).\n", nStatus );
        return eSIR_FAILURE;    //           
    }
    else if ( DOT11F_WARNED( nStatus ) )
    {
        VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_ERROR, 
            "There were warnings while packing a Probe Request (0x%08x).\n" );
    }

    *pusLen = nPayload + sizeof(tSirMacMgmtHdr); 
    return eSIR_SUCCESS;
} //                                


eHalStatus pmcSetPreferredNetworkList
(
    tHalHandle hHal, 
    tpSirPNOScanReq pRequest, 
    tANI_U8 sessionId, 
    preferredNetworkFoundIndCallback callbackRoutine, 
    void *callbackContext
)
{
    tpSirPNOScanReq pRequestBuf;
    vos_msg_t msg;
    tpAniSirGlobal pMac = PMAC_STRUCT(hHal);
    tCsrRoamSession *pSession = CSR_GET_SESSION( pMac, sessionId );
    tANI_U8 ucDot11Mode; 

    VOS_TRACE( VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_INFO, "%s: SSID = %s, %s", __FUNCTION__,
        pRequest->aNetworks[0].ssId.ssId, pRequest->aNetworks[1].ssId.ssId);

    pRequestBuf = vos_mem_malloc(sizeof(tSirPNOScanReq));
    if (NULL == pRequestBuf)
    {
        VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_ERROR, "%s: Not able to allocate memory for PNO request", __FUNCTION__);
        return eHAL_STATUS_FAILED_ALLOC;
    }

    vos_mem_copy(pRequestBuf, pRequest, sizeof(tSirPNOScanReq));

    /*                             */
    ucDot11Mode = (tANI_U8) csrTranslateToWNICfgDot11Mode(pMac, 
                                       csrFindBestPhyMode( pMac, pMac->roam.configParam.phyMode ));

    /*                                                             */
    pmcPrepareProbeReqTemplate(pMac,SIR_PNO_24G_DEFAULT_CH, ucDot11Mode, pSession->selfMacAddr, 
                               pRequestBuf->p24GProbeTemplate, &pRequestBuf->us24GProbeTemplateLen); 

    pmcPrepareProbeReqTemplate(pMac,SIR_PNO_5G_DEFAULT_CH, ucDot11Mode, pSession->selfMacAddr, 
                               pRequestBuf->p5GProbeTemplate, &pRequestBuf->us5GProbeTemplateLen); 


    msg.type     = WDA_SET_PNO_REQ;
    msg.reserved = 0;
    msg.bodyptr  = pRequestBuf;
    if (!VOS_IS_STATUS_SUCCESS(vos_mq_post_message(VOS_MODULE_ID_WDA, &msg)))
    {
        VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_ERROR, "%s: Not able to post WDA_SET_PNO_REQ message to WDA", __FUNCTION__);
        vos_mem_free(pRequestBuf);
        return eHAL_STATUS_FAILURE;
    }

    /*                                                                   */
    pMac->pmc.prefNetwFoundCB = callbackRoutine;
    pMac->pmc.preferredNetworkFoundIndCallbackContext = callbackContext;

    VOS_TRACE( VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_INFO, "-%s", __FUNCTION__);

    return eHAL_STATUS_SUCCESS;
}

eHalStatus pmcSetRssiFilter(tHalHandle hHal,   v_U8_t        rssiThreshold)
{
    tpSirSetRSSIFilterReq pRequestBuf;
    vos_msg_t msg;


    pRequestBuf = vos_mem_malloc(sizeof(tpSirSetRSSIFilterReq));
    if (NULL == pRequestBuf)
    {
        VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_ERROR, "%s: Not able to allocate memory for PNO request", __FUNCTION__);
        return eHAL_STATUS_FAILED_ALLOC;
    }


    pRequestBuf->rssiThreshold = rssiThreshold; 

    msg.type = WDA_SET_RSSI_FILTER_REQ;
    msg.reserved = 0;
    msg.bodyptr = pRequestBuf;
    if (!VOS_IS_STATUS_SUCCESS(vos_mq_post_message(VOS_MODULE_ID_WDA, &msg)))
    {
        VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_ERROR, "%s: Not able to post WDA_SET_PNO_REQ message to WDA", __FUNCTION__);
        vos_mem_free(pRequestBuf);
        return eHAL_STATUS_FAILURE;
    }

    return eHAL_STATUS_SUCCESS;
}


eHalStatus pmcUpdateScanParams(tHalHandle hHal, tCsrConfig *pRequest, tCsrChannel *pChannelList, tANI_U8 b11dResolved)
{
    tpSirUpdateScanParams pRequestBuf;
    vos_msg_t msg;
    int i;

    VOS_TRACE( VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_INFO, "%s started", __FUNCTION__);

    pRequestBuf = vos_mem_malloc(sizeof(tSirUpdateScanParams));
    if (NULL == pRequestBuf)
    {
        VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_ERROR, "%s: Not able to allocate memory for UpdateScanParams request", __FUNCTION__);
        return eHAL_STATUS_FAILED_ALLOC;
    }

    // 
    //                                         
    //
    pRequestBuf->b11dEnabled    = pRequest->Is11eSupportEnabled;
    pRequestBuf->b11dResolved   = b11dResolved;
    pRequestBuf->ucChannelCount = 
        ( pChannelList->numChannels < SIR_PNO_MAX_NETW_CHANNELS )?
        pChannelList->numChannels:SIR_PNO_MAX_NETW_CHANNELS;

    for (i=0; i < pChannelList->numChannels; i++)
    {    
        pRequestBuf->aChannels[i] = pChannelList->channelList[i];
    }
    pRequestBuf->usPassiveMinChTime = pRequest->nPassiveMinChnTime;
    pRequestBuf->usPassiveMaxChTime = pRequest->nPassiveMaxChnTime;
    pRequestBuf->usActiveMinChTime  = pRequest->nActiveMinChnTime;
    pRequestBuf->usActiveMaxChTime  = pRequest->nActiveMaxChnTime; 
    pRequestBuf->ucCBState          = PHY_SINGLE_CHANNEL_CENTERED;

    msg.type = WDA_UPDATE_SCAN_PARAMS_REQ;
    msg.reserved = 0;
    msg.bodyptr = pRequestBuf;
    if (!VOS_IS_STATUS_SUCCESS(vos_mq_post_message(VOS_MODULE_ID_WDA, &msg)))
    {
        VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_ERROR, "%s: Not able to post WDA_UPDATE_SCAN_PARAMS message to WDA", __FUNCTION__);
        vos_mem_free(pRequestBuf);
        return eHAL_STATUS_FAILURE;
    }

    return eHAL_STATUS_SUCCESS;
}
#endif //                      

eHalStatus pmcSetPowerParams(tHalHandle hHal,   tSirSetPowerParamsReq*  pwParams)
{
    tSirSetPowerParamsReq* pRequestBuf;
    vos_msg_t msg;
    tpAniSirGlobal pMac = PMAC_STRUCT(hHal);
    tpPESession     psessionEntry;

    if((psessionEntry = peGetValidPowerSaveSession(pMac))== NULL)
    {
        return eHAL_STATUS_NOT_INITIALIZED;
    }

    pRequestBuf = vos_mem_malloc(sizeof(tSirSetPowerParamsReq));
    if (NULL == pRequestBuf)
    {
        VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_ERROR, "%s: Not able to allocate memory for Power Paramrequest", __FUNCTION__);
        return eHAL_STATUS_FAILED_ALLOC;
    }


    vos_mem_copy(pRequestBuf, pwParams, sizeof(*pRequestBuf)); 


    msg.type = WDA_SET_POWER_PARAMS_REQ;
    msg.reserved = 0;
    msg.bodyptr = pRequestBuf;

    if(VOS_STATUS_SUCCESS != vos_mq_post_message(VOS_MODULE_ID_WDA, &msg))
    {
        VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_ERROR, "%s: Not able to post WDA_SET_POWER_PARAMS_REQ message to WDA", __FUNCTION__);
        vos_mem_free(pRequestBuf);
        return eHAL_STATUS_FAILURE;
    }

    return eHAL_STATUS_SUCCESS;
}

#ifdef WLAN_FEATURE_PACKET_FILTERING
eHalStatus pmcGetFilterMatchCount
(
    tHalHandle hHal, 
    FilterMatchCountCallback callbackRoutine, 
    void *callbackContext
)
{
    tpSirRcvFltPktMatchRsp  pRequestBuf;
    vos_msg_t               msg;
    tpAniSirGlobal          pMac = PMAC_STRUCT(hHal);

    VOS_TRACE( VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_INFO, 
        "%s: filterId = %d", __FUNCTION__);

    pRequestBuf = vos_mem_malloc(sizeof(tSirRcvFltPktMatchRsp));
    if (NULL == pRequestBuf)
    {
        VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_ERROR, 
                  "%s: Not able to allocate "
                  "memory for Get PC Filter Match Count request", __FUNCTION__);
        return eHAL_STATUS_FAILED_ALLOC;
    }

    msg.type = WDA_PACKET_COALESCING_FILTER_MATCH_COUNT_REQ;
    msg.reserved = 0;
    msg.bodyptr = pRequestBuf;

    /*                                                                     */
    if (NULL != pMac->pmc.FilterMatchCountCB)
    {
        //                                                
        //                                                                      
        //                                                                          
        //                        
        //                                                      
    }

    pMac->pmc.FilterMatchCountCB = callbackRoutine;
    pMac->pmc.FilterMatchCountCBContext = callbackContext;

    if(VOS_STATUS_SUCCESS != vos_mq_post_message(VOS_MODULE_ID_WDA, &msg))
    {
        VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_ERROR, 
            "%s: Not able to post WDA_PACKET_COALESCING_FILTER_MATCH_COUNT_REQ "
            "message to WDA", __FUNCTION__);
        return eHAL_STATUS_FAILURE;
    }

    return eHAL_STATUS_SUCCESS;
}
#endif //                              

#ifdef WLAN_FEATURE_GTK_OFFLOAD
/*                                                                            
                        
                                    
                                                  
                                                             
                      
                                                        
                                                   
                                                                             */
eHalStatus pmcSetGTKOffload (tHalHandle hHal, tpSirGtkOffloadParams pGtkOffload)
{
    tpSirGtkOffloadParams pRequestBuf;
    vos_msg_t msg;
    int i;

    VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_INFO, "%s: KeyReplayCounter: %d", 
                __FUNCTION__, pGtkOffload->ullKeyReplayCounter);

    pRequestBuf = (tpSirGtkOffloadParams)vos_mem_malloc(sizeof(tSirGtkOffloadParams));
    if (NULL == pRequestBuf)
    {
        VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_ERROR, "%s: Not able to allocate "
                  "memory for GTK offload request", __FUNCTION__);
        return eHAL_STATUS_FAILED_ALLOC;
    }

    vos_mem_copy(pRequestBuf, pGtkOffload, sizeof(tSirGtkOffloadParams));
        
    msg.type = WDA_GTK_OFFLOAD_REQ;
    msg.reserved = 0;
    msg.bodyptr = pRequestBuf;
    if (!VOS_IS_STATUS_SUCCESS(vos_mq_post_message(VOS_MODULE_ID_WDA, &msg)))
    {
        VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_ERROR, "%s: Not able to post "
                  "SIR_HAL_SET_GTK_OFFLOAD message to HAL", __FUNCTION__);
        vos_mem_free(pRequestBuf);
        return eHAL_STATUS_FAILURE;
    }

    return eHAL_STATUS_SUCCESS;
}

/*                                                                            
                        
                                        
                                                  
                                                                                            
                      
                                                        
                                                   
                                                                             */
eHalStatus pmcGetGTKOffload(tHalHandle hHal, GTKOffloadGetInfoCallback callbackRoutine, void *callbackContext)
{
    tpSirGtkOffloadGetInfoRspParams  pRequestBuf;
    vos_msg_t               msg;
    tpAniSirGlobal          pMac = PMAC_STRUCT(hHal);

    VOS_TRACE( VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_INFO, "%s: filterId = %d", 
                __FUNCTION__);

    pRequestBuf = vos_mem_malloc(sizeof(tSirGtkOffloadGetInfoRspParams));
    if (NULL == pRequestBuf)
    {
        VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_ERROR, "%s: Not able to allocate "
                  "memory for Get GTK Offload request", __FUNCTION__);
        return eHAL_STATUS_FAILED_ALLOC;
    }

    msg.type = WDA_GTK_OFFLOAD_GETINFO_REQ;
    msg.reserved = 0;
    msg.bodyptr = pRequestBuf;

    /*                                                */
    if (NULL != pMac->pmc.GtkOffloadGetInfoCB)
    {
        //                                                
        //                                                                      
        //                                                                                                  
        //                                                      
    }

    pMac->pmc.GtkOffloadGetInfoCB = callbackRoutine;
    pMac->pmc.GtkOffloadGetInfoCBContext = callbackContext;

    if (!VOS_IS_STATUS_SUCCESS(vos_mq_post_message(VOS_MODULE_ID_WDA, &msg)))
    {
        VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_ERROR, "%s: Not able to post WDA_GTK_OFFLOAD_GETINFO_REQ message to WDA", 
                    __FUNCTION__);
        vos_mem_free(pRequestBuf);
        return eHAL_STATUS_FAILURE;
    }

    return eHAL_STATUS_SUCCESS;
}
#endif //                         
