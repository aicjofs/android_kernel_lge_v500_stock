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

#if (WNI_POLARIS_FW_PRODUCT == AP)
#include "wniCfgAp.h"
#else
#include "wniCfgSta.h"
#endif
#include "sirMacProtDef.h"
#include "cfgApi.h"
#include "limTypes.h"
#include "limUtils.h"
#include "limPropExtsUtils.h"
#ifdef FEATURE_WLAN_NON_INTEGRATED_SOC
#include "halCommonApi.h"
#endif
#include "schApi.h"
#include "pmmApi.h"
#if defined WLAN_FEATURE_VOWIFI
#include "rrmApi.h"
#endif

static void limUpdateConfig(tpAniSirGlobal pMac,tpPESession psessionEntry);

#if 0
/* 
                              
  
           
                                                          
  
         
  
        
  
              
     
  
       
     
  
                                             
                                                                   
 */

static tANI_U32
limGetCfgIdOfDefaultKeyid(tANI_U32 dkid)
{
    if (dkid == WNI_CFG_WEP_DEFAULT_KEYID_0)
        return WNI_CFG_WEP_DEFAULT_KEY_1;
    else if (dkid == WNI_CFG_WEP_DEFAULT_KEYID_1)
        return WNI_CFG_WEP_DEFAULT_KEY_2;
    else if (dkid == WNI_CFG_WEP_DEFAULT_KEYID_2)
        return WNI_CFG_WEP_DEFAULT_KEY_3;
    else //                                    
        return WNI_CFG_WEP_DEFAULT_KEY_4;
} /*                                     */
#endif


/* 
                              
  
           
                                                       
                                 
  
         
  
        
  
              
     
  
       
  
                                                  
               
 */

static void
limSetDefaultKeyIdAndKeys(tpAniSirGlobal pMac)
{
#ifdef FIXME_GEN6
    tANI_U32 val;
    tANI_U32 dkCfgId;

    PELOG1(limLog(pMac, LOG1, FL("Setting default keys at SP\n"));)

    if (wlan_cfgGetInt(pMac, WNI_CFG_WEP_DEFAULT_KEYID,
                  &val) != eSIR_SUCCESS)
    {
        limLog(pMac, LOGP,
               FL("Unable to retrieve defaultKeyId from CFG\n"));
    }
    dkCfgId = limGetCfgIdOfDefaultKeyid(val);
#endif

} /*                                     */

/*                                                               
                       
                                                 
                                  
                 
                                                               */
#ifdef WLAN_SOFTAP_FEATURE
void limSetCfgProtection(tpAniSirGlobal pMac, tpPESession pesessionEntry)
#else
void limSetCfgProtection(tpAniSirGlobal pMac)
#endif
{
    tANI_U32 val = 0;

#ifdef WLAN_SOFTAP_FEATURE
    if(( pesessionEntry != NULL ) && (pesessionEntry->limSystemRole == eLIM_AP_ROLE )){
        if (pesessionEntry->gLimProtectionControl == WNI_CFG_FORCE_POLICY_PROTECTION_DISABLE )
            palZeroMemory( pMac->hHdd, (void *)&pesessionEntry->cfgProtection , sizeof(tCfgProtection));
        else{
            limLog(pMac, LOG1, FL(" frm11a = %d, from11b = %d, frm11g = %d, "
                                    "ht20 = %d, nongf = %d, lsigTxop = %d, "
                                    "rifs = %d, obss = %d\n"),    
                                    pesessionEntry->cfgProtection.fromlla,
                                    pesessionEntry->cfgProtection.fromllb,
                                    pesessionEntry->cfgProtection.fromllg,
                                    pesessionEntry->cfgProtection.ht20,
                                    pesessionEntry->cfgProtection.nonGf,
                                    pesessionEntry->cfgProtection.lsigTxop,
                                    pesessionEntry->cfgProtection.rifs,
                                    pesessionEntry->cfgProtection.obss);
        }
    }
    else{
#endif
    if (wlan_cfgGetInt(pMac, WNI_CFG_FORCE_POLICY_PROTECTION, &val) != eSIR_SUCCESS)
    {
        limLog(pMac, LOGP, FL("reading WNI_CFG_FORCE_POLICY_PROTECTION cfg failed\n"));
        return;
    }
    else
        pMac->lim.gLimProtectionControl = (tANI_U8)val;

    if (wlan_cfgGetInt(pMac, WNI_CFG_PROTECTION_ENABLED, &val) != eSIR_SUCCESS)
    {
        limLog(pMac, LOGP, FL("reading protection cfg failed\n"));
        return;
    }

    if(pMac->lim.gLimProtectionControl == WNI_CFG_FORCE_POLICY_PROTECTION_DISABLE)
        palZeroMemory( pMac->hHdd, (void *)&pMac->lim.cfgProtection , sizeof(tCfgProtection));
    else
        {
#if (defined(ANI_PRODUCT_TYPE_AP) || defined(ANI_PRODUCT_TYPE_AP_SDK))
            {
                pMac->lim.cfgProtection.overlapFromlla = (val >> WNI_CFG_PROTECTION_ENABLED_OLBC_FROM_llA) & 1;
                pMac->lim.cfgProtection.overlapFromllb = (val >> WNI_CFG_PROTECTION_ENABLED_OLBC_FROM_llB) & 1;
                pMac->lim.cfgProtection.overlapFromllg = (val >> WNI_CFG_PROTECTION_ENABLED_OLBC_FROM_llG) & 1;
                pMac->lim.cfgProtection.overlapHt20 = (val >> WNI_CFG_PROTECTION_ENABLED_OLBC_HT20) & 1;
                pMac->lim.cfgProtection.overlapNonGf = (val >> WNI_CFG_PROTECTION_ENABLED_OLBC_NON_GF) & 1;
                pMac->lim.cfgProtection.overlapLsigTxop = (val >> WNI_CFG_PROTECTION_ENABLED_OLBC_LSIG_TXOP) & 1;
                pMac->lim.cfgProtection.overlapRifs = (val >> WNI_CFG_PROTECTION_ENABLED_OLBC_RIFS) & 1;
                pMac->lim.cfgProtection.overlapOBSS = (val>> WNI_CFG_PROTECTION_ENABLED_OLBC_OBSS )&1;

            }
            #endif
            pMac->lim.cfgProtection.fromlla = (val >> WNI_CFG_PROTECTION_ENABLED_FROM_llA) & 1;
            pMac->lim.cfgProtection.fromllb = (val >> WNI_CFG_PROTECTION_ENABLED_FROM_llB) & 1;
            pMac->lim.cfgProtection.fromllg = (val >> WNI_CFG_PROTECTION_ENABLED_FROM_llG) & 1;
            pMac->lim.cfgProtection.ht20 = (val >> WNI_CFG_PROTECTION_ENABLED_HT_20) & 1;
            pMac->lim.cfgProtection.nonGf = (val >> WNI_CFG_PROTECTION_ENABLED_NON_GF) & 1;
            pMac->lim.cfgProtection.lsigTxop = (val >> WNI_CFG_PROTECTION_ENABLED_LSIG_TXOP) & 1;
            pMac->lim.cfgProtection.rifs = (val >> WNI_CFG_PROTECTION_ENABLED_RIFS) & 1;
            pMac->lim.cfgProtection.obss= (val >> WNI_CFG_PROTECTION_ENABLED_OBSS) & 1;

        }
#ifdef WLAN_SOFTAP_FEATURE
}
#endif
}



/* 
                                
  
            
                                                                               
                                                                         
  
          
                                               
  
 */
static tSirRetStatus limUpdateTriggerStaBkScanFlag(tpAniSirGlobal pMac)
{
    tANI_U32 val;
    tANI_U8 flag;

    if(wlan_cfgGetInt(pMac, WNI_CFG_TRIG_STA_BK_SCAN, &val) != eSIR_SUCCESS)
    {
    PELOG1(limLog(pMac, LOG1, FL("Failed to get WNI_CFG_TRIG_STA_BK_SCAN from cfg\n"));)
    return eSIR_FAILURE;
    }

    flag = (val) ? 1 : 0;
    if(flag != pMac->lim.gLimTriggerBackgroundScanDuringQuietBss)
    {
        /*                    */
        pMac->lim.gLimTriggerBackgroundScanDuringQuietBss = flag;
        /*                                          */

        //                                                                                                                 
        limUpdateBeacon(pMac);

    }

    return eSIR_FAILURE;
}


/* 
                            
  
           
                                                     
                                                      
                                                      
  
         
  
        
  
              
     
  
       
  
                                                  
                                                       
               
 */

void
limHandleCFGparamUpdate(tpAniSirGlobal pMac, tANI_U32 cfgId)
{
    tANI_U32 val1, val2;
    tANI_U16 val16;
    tSirMacHTCapabilityInfo   *pHTCapabilityInfo;
    tSirMacHTParametersInfo *pAmpduParamInfo;

    PELOG3(limLog(pMac, LOG3, FL("Handling CFG parameter id %X update\n"), cfgId);)
    switch (cfgId)
    {
        case WNI_CFG_WEP_DEFAULT_KEYID:

            //                                                            
            //                                                        
            //                                                                      
            //                                                             
            limSetDefaultKeyIdAndKeys( pMac );

            break;

        case WNI_CFG_EXCLUDE_UNENCRYPTED:
            if (wlan_cfgGetInt(pMac, WNI_CFG_EXCLUDE_UNENCRYPTED,
                          &val1) != eSIR_SUCCESS)
            {
                limLog(pMac, LOGP,
                   FL("Unable to retrieve excludeUnencr from CFG\n"));
            }
#if 0
            halSetSpExclUndecrypted(pMac, (tHalBitVal) val);
#else
            limLog(pMac, LOGE,
                   FL("Unsupported CFG: WNI_CFG_EXCLUDE_UNENCRYPTED\n"));
#endif

            break;

        case WNI_CFG_ASSOCIATION_FAILURE_TIMEOUT:
            if (pMac->lim.gLimMlmState != eLIM_MLM_WT_ASSOC_RSP_STATE)
            {
                //                                      
                limDeactivateAndChangeTimer(pMac,
                                            eLIM_ASSOC_FAIL_TIMER);
            }

            break;

#if (WNI_POLARIS_FW_PRODUCT == WLAN_STA) || defined(ANI_AP_CLIENT_SDK)
        case WNI_CFG_BACKGROUND_SCAN_PERIOD:


            limDeactivateAndChangeTimer(pMac, eLIM_BACKGROUND_SCAN_TIMER);

            if (wlan_cfgGetInt(pMac, WNI_CFG_BACKGROUND_SCAN_PERIOD, &val1) != eSIR_SUCCESS)
            {
                limLog(pMac, LOGP,  FL("could not retrieve Background scan period value\n"));
                break;
            }
            if (val1 == 0)
                break;



            if ( (pMac->lim.gLimSystemRole == eLIM_STA_ROLE) ||
                 ( (pMac->lim.gLimSystemRole == eLIM_STA_IN_IBSS_ROLE) &&
                   (pMac->lim.gLimSmeState == eLIM_SME_NORMAL_STATE)))
            {
                //                                 
                MTRACE(macTrace(pMac, TRACE_CODE_TIMER_ACTIVATE, NO_SESSION, eLIM_BACKGROUND_SCAN_TIMER));
                if (tx_timer_activate(
                      &pMac->lim.limTimers.gLimBackgroundScanTimer) != TX_SUCCESS)
                {
                    //                                           
                    //          
                    limLog(pMac, LOGP,
                      FL("could not activate background scan timer\n"));
                    pMac->lim.gLimBackgroundScanStarted = FALSE;
                    pMac->lim.gLimBackgroundScanTerminate = TRUE;
                }
                else
                {
                    pMac->lim.gLimBackgroundScanStarted = TRUE;
                    pMac->lim.gLimBackgroundScanTerminate = FALSE;
                }
               
               PELOG3(limLog(pMac, LOG3,
                       FL("Updated Background scan period\n"));)
            }
            
            break;
#endif
#if (WNI_POLARIS_FW_PRODUCT == AP)
        case WNI_CFG_PREAUTH_CLNUP_TIMEOUT:
            if (pMac->lim.gLimSystemRole == eLIM_AP_ROLE)
            {
                limDeactivateAndChangeTimer(pMac,
                                           eLIM_PRE_AUTH_CLEANUP_TIMER);

#ifdef GEN6_TODO
                /*                                                                     
                                    
                 */
                pMac->lim.limTimers.gLimPreAuthClnupTimer.sessionId = sessionId;
#endif
                //                                  
                MTRACE(macTrace(pMac, TRACE_CODE_TIMER_ACTIVATE, NO_SESSION, eLIM_PRE_AUTH_CLEANUP_TIMER));
                if (tx_timer_activate(&pMac->lim.limTimers.gLimPreAuthClnupTimer)
                                                       != TX_SUCCESS)
                {
                    //                                            
                    //          
                    limLog(pMac, LOGP,
                      FL("could not activate preauth cleanup timer\n"));
                }
               PELOG3(limLog(pMac, LOG3,
                       FL("Updated pre-auth cleanup timeout\n"));)
            }

            break;

#endif

        case WNI_CFG_BG_SCAN_CHANNEL_LIST:
#if (WNI_POLARIS_FW_PRODUCT == WLAN_STA) || defined(ANI_AP_CLIENT_SDK)
            PELOG1(limLog(pMac, LOG1,
               FL("VALID_CHANNEL_LIST has changed, reset next bg scan channel\n"));)
            pMac->lim.gLimBackgroundScanChannelId = 0;
#endif

            break;

    case WNI_CFG_TRIG_STA_BK_SCAN:
        if(limUpdateTriggerStaBkScanFlag(pMac) != eSIR_SUCCESS)
        {
       PELOG2(limLog(pMac, LOG2,
               FL("Updating lim trigger sta bk scan global flag failed!\n"));)
        }
        break;

    case WNI_CFG_PROTECTION_ENABLED:
#ifdef WLAN_SOFTAP_FEATURE
        limSetCfgProtection(pMac, NULL);
#else
        limSetCfgProtection(pMac);
#endif
        break;
    case WNI_CFG_PROBE_RSP_BCN_ADDNIE_FLAG:
    {
        tSirMsgQ msg = {0};
        tANI_U32 status;

        msg.type = SIR_LIM_UPDATE_BEACON;

        status = limPostMsgApi(pMac, &msg);

        if (status != TX_SUCCESS)
            PELOGE(limLog(pMac, LOGE, FL("Failed limPostMsgApi\n"), status);)
        break;
    }
    case WNI_CFG_GREENFIELD_CAPABILITY:
        if (wlan_cfgGetInt(pMac, WNI_CFG_HT_CAP_INFO, &val1) != eSIR_SUCCESS) 
        {
            PELOGE(limLog(pMac, LOGE, FL("could not retrieve HT Cap Info CFG\n"));)
            break;
        }
        if (wlan_cfgGetInt(pMac, WNI_CFG_GREENFIELD_CAPABILITY, &val2) != eSIR_SUCCESS) 
        {
            PELOGE(limLog(pMac, LOGE, FL("could not retrieve GreenField CFG\n"));)
            break;
        }
        val16 = ( tANI_U16 ) val1;
        pHTCapabilityInfo = ( tSirMacHTCapabilityInfo* ) &val16;
        pHTCapabilityInfo->greenField = (tANI_U16)val2;
        if(cfgSetInt(pMac, WNI_CFG_HT_CAP_INFO, *(tANI_U16*)pHTCapabilityInfo) != eSIR_SUCCESS)
            PELOGE(limLog(pMac, LOGE, FL("could not update HT Cap Info CFG\n"));)
        break;

    case WNI_CFG_HT_RX_STBC:
        if (wlan_cfgGetInt(pMac, WNI_CFG_HT_CAP_INFO, &val1) != eSIR_SUCCESS) 
        {
            PELOGE(limLog(pMac, LOGE, FL("could not retrieve WNI_CFG_HT_CAP_INFO \n"));)
            break;
        }
        if (wlan_cfgGetInt(pMac, WNI_CFG_HT_RX_STBC, &val2) != eSIR_SUCCESS) 
        {
            PELOGE(limLog(pMac, LOGE, FL("could not retrieve WNI_CFG_HT_RX_STBC\n"));)
            break;
        }
        val16 = ( tANI_U16 ) val1;
        pHTCapabilityInfo = ( tSirMacHTCapabilityInfo* ) &val16;
        pHTCapabilityInfo->rxSTBC = (tANI_U16)val2;
        if(cfgSetInt(pMac, WNI_CFG_HT_CAP_INFO, *(tANI_U16*)pHTCapabilityInfo) != eSIR_SUCCESS)
            PELOGE(limLog(pMac, LOGE, FL("could not update HT Cap Info CFG\n"));)
        break;

    case WNI_CFG_MAX_AMSDU_LENGTH:
        if (wlan_cfgGetInt(pMac, WNI_CFG_HT_CAP_INFO, &val1) != eSIR_SUCCESS)
            {
                PELOGE(limLog(pMac, LOGE, FL("could not retrieve HT Cap Info CFG\n"));)
                break;
            }
        if (wlan_cfgGetInt(pMac, WNI_CFG_MAX_AMSDU_LENGTH, &val2) != eSIR_SUCCESS)
            {
                PELOGE(limLog(pMac, LOGE, FL("could not retrieve Max AMSDU Length CFG\n"));)
                break;
            }
        val16 = ( tANI_U16 ) val1;
        pHTCapabilityInfo = ( tSirMacHTCapabilityInfo* ) &val16;
        pHTCapabilityInfo->maximalAMSDUsize = (tANI_U16)val2;
        if(cfgSetInt(pMac, WNI_CFG_HT_CAP_INFO, *(tANI_U16*)pHTCapabilityInfo) != eSIR_SUCCESS)
            PELOGE(limLog(pMac, LOGE, FL("could not update HT Cap Info CFG\n"));)
        break;

    case WNI_CFG_SHORT_GI_20MHZ:
        if (wlan_cfgGetInt(pMac, WNI_CFG_HT_CAP_INFO, &val1) != eSIR_SUCCESS) 
            {
                PELOGE(limLog(pMac, LOGE, FL("could not retrieve HT Cap CFG\n"));)
                break;
            }
        if (wlan_cfgGetInt(pMac, WNI_CFG_SHORT_GI_20MHZ, &val2) != eSIR_SUCCESS) 
            {
                PELOGE(limLog(pMac, LOGE, FL("could not retrieve shortGI 20Mhz CFG\n"));)
                break;
            }
        val16 = ( tANI_U16 ) val1;
        pHTCapabilityInfo = ( tSirMacHTCapabilityInfo* ) &val16;
        pHTCapabilityInfo->shortGI20MHz = (tANI_U16)val2;
        if(cfgSetInt(pMac,  WNI_CFG_HT_CAP_INFO, *(tANI_U16*)pHTCapabilityInfo) != eSIR_SUCCESS)
            PELOGE(limLog(pMac, LOGE, FL("could not update HT Cap Info CFG\n"));)
        break;
    case WNI_CFG_SHORT_GI_40MHZ:
        if (wlan_cfgGetInt(pMac, WNI_CFG_HT_CAP_INFO, &val1) != eSIR_SUCCESS) 
            {
                PELOGE(limLog(pMac, LOGE, FL("could not retrieve HT Cap CFG\n"));)
                break;
            }
        if (wlan_cfgGetInt(pMac, WNI_CFG_SHORT_GI_40MHZ, &val2) != eSIR_SUCCESS) 
            {
                PELOGE(limLog(pMac, LOGE, FL("could not retrieve shortGI 40Mhz CFG\n"));)
                break;
            }
        val16 = ( tANI_U16 ) val1;
        pHTCapabilityInfo = ( tSirMacHTCapabilityInfo* ) &val16;
        pHTCapabilityInfo->shortGI40MHz = (tANI_U16)val2;
        if(cfgSetInt(pMac,  WNI_CFG_HT_CAP_INFO, *(tANI_U16*)pHTCapabilityInfo) != eSIR_SUCCESS)
            PELOGE(limLog(pMac, LOGE, FL("could not update HT Cap Info CFG\n"));)
        break;
    case WNI_CFG_MPDU_DENSITY:
        if (wlan_cfgGetInt(pMac, WNI_CFG_HT_AMPDU_PARAMS, &val1) != eSIR_SUCCESS) 
            {
                PELOGE(limLog(pMac, LOGE, FL("could not retrieve HT AMPDU Param CFG\n"));)
                break;
            }
        if (wlan_cfgGetInt(pMac, WNI_CFG_MPDU_DENSITY, &val2) != eSIR_SUCCESS) 
            {
                PELOGE(limLog(pMac, LOGE, FL("could not retrieve MPDU Density CFG\n"));)
                break;
            }
        val16 = ( tANI_U16 ) val1;
        pAmpduParamInfo = ( tSirMacHTParametersInfo* ) &val16;
        pAmpduParamInfo->mpduDensity = (tANI_U8)val2;
        if(cfgSetInt(pMac,  WNI_CFG_HT_AMPDU_PARAMS, *(tANI_U8*)pAmpduParamInfo) != eSIR_SUCCESS)
            PELOGE(limLog(pMac, LOGE, FL("could not update HT AMPDU Param CFG\n"));)

        break;
    case WNI_CFG_MAX_RX_AMPDU_FACTOR:
        if (wlan_cfgGetInt(pMac, WNI_CFG_HT_AMPDU_PARAMS, &val1) != eSIR_SUCCESS) 
            {
                PELOGE(limLog(pMac, LOGE, FL("could not retrieve HT AMPDU Param CFG\n"));)
                break;
            }
        if (wlan_cfgGetInt(pMac, WNI_CFG_MAX_RX_AMPDU_FACTOR, &val2) != eSIR_SUCCESS) 
            {
                PELOGE(limLog(pMac, LOGE, FL("could not retrieve AMPDU Factor CFG\n"));)
                break;
            }
        val16 = ( tANI_U16 ) val1;
        pAmpduParamInfo = ( tSirMacHTParametersInfo* ) &val16;
        pAmpduParamInfo->maxRxAMPDUFactor = (tANI_U8)val2;
        if(cfgSetInt(pMac,  WNI_CFG_HT_AMPDU_PARAMS, *(tANI_U8*)pAmpduParamInfo) != eSIR_SUCCESS)
            PELOGE(limLog(pMac, LOGE, FL("could not update HT AMPDU Param CFG\n"));)
        break;
  
    case WNI_CFG_HEART_BEAT_THRESHOLD:
        if (wlan_cfgGetInt(pMac, WNI_CFG_HEART_BEAT_THRESHOLD, &val1) != eSIR_SUCCESS)
        {
            PELOGE(limLog(pMac, LOGE, FL("could not retrieve WNI_CFG_HEART_BEAT_THRESHOLD CFG\n"));)
                break;
        }
        if(!val1) 
        {
            limDeactivateAndChangeTimer(pMac, eLIM_HEART_BEAT_TIMER);
            pMac->sys.gSysEnableLinkMonitorMode = 0;
            PELOGE(limLog(pMac, LOGE, "Deactivating heartbeat link monitoring\n");)
        } 
        else 
        {
            tANI_U16 sessionId;
            pMac->sys.gSysEnableLinkMonitorMode = 1;
            for(sessionId = 0; sessionId < pMac->lim.maxBssId; sessionId++)
            {
                if( (pMac->lim.gpSession[sessionId].valid )&& 
                    (eLIM_MLM_LINK_ESTABLISHED_STATE == pMac->lim.gpSession[sessionId].limMlmState) &&
                    ( pMac->pmm.gPmmState != ePMM_STATE_BMPS_SLEEP))
                {
                    limReactivateHeartBeatTimer(pMac, &pMac->lim.gpSession[sessionId]);
                }
            }
            PELOGE(limLog(pMac, LOGE, "Reactivating heartbeat link monitoring\n");)
        }        
    case WNI_CFG_MAX_PS_POLL:
    case WNI_CFG_NUM_BEACON_PER_RSSI_AVERAGE:
    case WNI_CFG_MIN_RSSI_THRESHOLD:
    case WNI_CFG_NTH_BEACON_FILTER:
    case WNI_CFG_BROADCAST_FRAME_FILTER_ENABLE:
        {
            tpSirPowerSaveCfg pPowerSaveConfig;

            /*                                                */
            if (palAllocateMemory(pMac->hHdd, (void **)&pPowerSaveConfig,
                                  sizeof(tSirPowerSaveCfg)) != eHAL_STATUS_SUCCESS)
            {
                PELOGE(limLog(pMac, LOGE, FL("LIM: Cannot allocate memory for power save configuration\n"));)
                break;
            }

            /*                                                                                             
                                                                                     
             */
            palCopyMemory(pMac->hHdd, pPowerSaveConfig, (tANI_U8 *)&pMac->pmm.gPmmCfg, sizeof(tSirPowerSaveCfg));

            if ( (pmmSendPowerSaveCfg(pMac, pPowerSaveConfig)) != eSIR_SUCCESS)
            {
                PELOGE(limLog(pMac, LOGE, FL("LIM: pmmSendPowerSaveCfg() failed \n"));)
            }
        }
        break;


     case WNI_CFG_DOT11_MODE:
        if (wlan_cfgGetInt(pMac, WNI_CFG_DOT11_MODE, &val1) != eSIR_SUCCESS) 
        {
            PELOGE(limLog(pMac, LOGE, FL("could not retrieve Dot11 Mode  CFG\n"));)
            break;
        }
        /*      */
        //                                                                                                                 
        break;
    case WNI_CFG_ADDBA_REQ_DECLINE:
        if(wlan_cfgGetInt(pMac, WNI_CFG_ADDBA_REQ_DECLINE, &val1) != eSIR_SUCCESS) {
            limLog( pMac, LOGE, FL( "Unable to get ADDBA_REQ_DECLINE cfg\n" ));
            break;
        }
        pMac->lim.gAddBA_Declined = (tANI_U8)val1;
        break;
        
    case WNI_CFG_SCAN_IN_POWERSAVE:
        if(wlan_cfgGetInt(pMac, WNI_CFG_SCAN_IN_POWERSAVE, &val1) != eSIR_SUCCESS) {
            limLog( pMac, LOGE, FL( "Unable to get WNI_CFG_SCAN_IN_POWERSAVE \n" ));
            break;
        }
        pMac->lim.gScanInPowersave = (tANI_U8)val1;
        break;


    case WNI_CFG_ASSOC_STA_LIMIT:
        if(wlan_cfgGetInt(pMac, WNI_CFG_ASSOC_STA_LIMIT, &val1) != eSIR_SUCCESS) {
            limLog( pMac, LOGE, FL( "Unable to get WNI_CFG_ASSOC_STA_LIMIT" ));
            break;
        }
        pMac->lim.gLimAssocStaLimit = (tANI_U16)val1;
        break;

    default:
            break;
    }
} /*                                   */



/* 
                          
  
           
                                                             
                                                                
  
         
  
        
  
              
     
  
       
  
                                                  
               
 */

void
limApplyConfiguration(tpAniSirGlobal pMac,tpPESession psessionEntry)
{
    tANI_U32          val=0, phyMode;

    PELOG2(limLog(pMac, LOG2, FL("Applying config\n"));)

#if (defined(ANI_PRODUCT_TYPE_AP) || defined(ANI_PRODUCT_TYPE_AP_SDK))
    limCleanupMeasResources(pMac);
#endif
    limInitWdsInfoParams(pMac);

    psessionEntry->limSentCapsChangeNtf = false;

    limGetPhyMode(pMac, &phyMode, psessionEntry);
        
    //                           
    limSetDefaultKeyIdAndKeys(pMac);

    limUpdateConfig(pMac,psessionEntry);

    if (phyMode == WNI_CFG_PHY_MODE_11A)
    {
        //                                
        //                                                                                                                           
        psessionEntry->shortSlotTimeSupported = true;
    }
    else if (phyMode == WNI_CFG_PHY_MODE_11G)
    {
        if ((psessionEntry->pePersona == VOS_STA_SAP_MODE) ||
           (psessionEntry->pePersona == VOS_P2P_GO_MODE))
        {
            val = 1;
        }

        //                                       

        if (psessionEntry->limMlmState == eLIM_MLM_WT_JOIN_BEACON_STATE)
            //             
            val = SIR_MAC_GET_SHORT_SLOT_TIME( psessionEntry->limCurrentBssCaps);
        else if (psessionEntry->limMlmState == eLIM_MLM_WT_REASSOC_RSP_STATE)
            //                       
            val = SIR_MAC_GET_SHORT_SLOT_TIME( psessionEntry->limReassocBssCaps);
        psessionEntry->shortSlotTimeSupported = val;
    }
    else //                                                                                     
    {
        //                            
        psessionEntry->shortSlotTimeSupported = false;
    }
    //                                

#ifdef WLAN_SOFTAP_FEATURE
    limSetCfgProtection(pMac, psessionEntry);    
#else
    limSetCfgProtection(pMac);    
#endif


    /*                            */
    if ( (psessionEntry->limSystemRole == eLIM_AP_ROLE) ||
         (psessionEntry->limSystemRole == eLIM_BT_AMP_AP_ROLE)||
         (psessionEntry->limSystemRole == eLIM_STA_IN_IBSS_ROLE)||
         (psessionEntry->limSystemRole == eLIM_BT_AMP_STA_ROLE) )
    {
        /*                                                                    
                                                          */

        if(psessionEntry->statypeForBss == STA_ENTRY_SELF)
        {
            PELOG1(limLog(pMac, LOG1, FL("Initializing BT-AMP beacon generation\n"));)
            schSetBeaconInterval(pMac,psessionEntry);
            schSetFixedBeaconFields(pMac,psessionEntry);
        }
    }

    if (wlan_cfgGetInt(pMac, WNI_CFG_SCAN_IN_POWERSAVE, &val) != eSIR_SUCCESS)
    {
        limLog(pMac, LOGP, FL("could not retrieve WNI_CFG_SCAN_IN_POWERSAVE\n"));
        return;
    }
    pMac->lim.gScanInPowersave = (tANI_U8) val;

} /*                                 */


/* 
                  
  
            
                                           
                                    
  
         
  
               
  
        
  
              
               
 */

static void
limUpdateConfig(tpAniSirGlobal pMac,tpPESession psessionEntry)
{
    tANI_U32 val;

    #if 0
    if (wlan_cfgGetStr(pMac, WNI_CFG_STA_ID, pMac->lim.gLimMyMacAddr, &len) != eSIR_SUCCESS)
        limLog(pMac, LOGP, FL("cfg get sta id failed\n"));
    #endif //                 
    sirCopyMacAddr(pMac->lim.gLimMyMacAddr,psessionEntry->selfMacAddr);

    if (wlan_cfgGetInt(pMac, WNI_CFG_SHORT_PREAMBLE, &val) != eSIR_SUCCESS)
        limLog(pMac, LOGP, FL("cfg get short preamble failed\n"));
    psessionEntry->beaconParams.fShortPreamble = (val) ? 1 : 0;

    if (wlan_cfgGetInt(pMac, WNI_CFG_WME_ENABLED, &val) != eSIR_SUCCESS)
        limLog(pMac, LOGP, FL("cfg get wme enabled failed\n"));
    psessionEntry->limWmeEnabled = (val) ? 1 : 0;

    if (wlan_cfgGetInt(pMac, WNI_CFG_WSM_ENABLED, &val) != eSIR_SUCCESS)
        limLog(pMac, LOGP, FL("cfg get wsm enabled failed\n"));
    psessionEntry->limWsmEnabled = (val) ? 1 : 0;

    if ((! psessionEntry->limWmeEnabled) && (psessionEntry->limWsmEnabled))
    {
        PELOGE(limLog(pMac, LOGE, FL("Can't enable WSM without WME\n"));)
        psessionEntry->limWsmEnabled = 0;
    }

    if (wlan_cfgGetInt(pMac, WNI_CFG_QOS_ENABLED, &val) != eSIR_SUCCESS)
        limLog(pMac, LOGP, FL("cfg get qos enabled failed\n"));
    psessionEntry->limQosEnabled = (val) ? 1 : 0;

    if (wlan_cfgGetInt(pMac, WNI_CFG_HCF_ENABLED, &val) != eSIR_SUCCESS)
        limLog(pMac, LOGP, FL("cfg get hcf enabled failed\n"));
    psessionEntry->limHcfEnabled = (val) ? 1 : 0;

    //                                          
    if(wlan_cfgGetInt(pMac, WNI_CFG_ADDBA_REQ_DECLINE, &val) != eSIR_SUCCESS)
        limLog( pMac, LOGP, FL( "Unable to get ADDBA_REQ_DECLINE cfg\n" ));
    pMac->lim.gAddBA_Declined = (val) ?  0xff : 0x0;

    //                                                                 
    //                                 
    if (psessionEntry->limWsmEnabled && psessionEntry->limSystemRole == eLIM_AP_ROLE)
        psessionEntry->limHcfEnabled = 1;

    if (wlan_cfgGetInt(pMac, WNI_CFG_11D_ENABLED, &val) != eSIR_SUCCESS)
        limLog(pMac, LOGP, FL("cfg get 11d enabled failed\n"));
    psessionEntry->lim11dEnabled = (val) ? 1 : 0;

    if(wlan_cfgGetInt(pMac, WNI_CFG_ASSOC_STA_LIMIT, &val) != eSIR_SUCCESS) {
        limLog( pMac, LOGP, FL( "cfg get assoc sta limit failed" ));
    }
    pMac->lim.gLimAssocStaLimit = (tANI_U16)val;

#if defined WLAN_FEATURE_VOWIFI
    rrmUpdateConfig( pMac, psessionEntry ); 
#endif
    PELOG1(limLog(pMac, LOG1, FL("Updated Lim shadow state based on CFG\n"));)

    
}


