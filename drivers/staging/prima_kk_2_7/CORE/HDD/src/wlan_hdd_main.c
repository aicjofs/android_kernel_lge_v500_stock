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

/*========================================================================

  \file  wlan_hdd_main.c

  \brief WLAN Host Device Driver implementation

   Copyright 2008 (c) Qualcomm, Incorporated.  All Rights Reserved.

   Qualcomm Confidential and Proprietary.

  ========================================================================*/

/*                                                                          

                                            


                                                                       
                                                                


                                     


                                     
                                                                             
                                            
                                                                        
                                                                              
                                                                            */

/*                                                                          
               
                                                                          */
//                            
#include <wlan_hdd_includes.h>
#include <vos_api.h>
#include <vos_sched.h>
#include <vos_power.h>
#include <linux/etherdevice.h>
#include <linux/firmware.h>
#ifdef ANI_BUS_TYPE_PLATFORM
#include <linux/wcnss_wlan.h>
#endif //                     
#ifdef ANI_BUS_TYPE_PCI
#include "wcnss_wlan.h"
#endif /*                  */
#include <wlan_hdd_tx_rx.h>
#include <palTimer.h>
#include <wniApi.h>
#include <wlan_nlink_srv.h>
#include <wlan_btc_svc.h>
#include <wlan_hdd_cfg.h>
#include <wlan_ptt_sock_svc.h>
#include <wlan_hdd_wowl.h>
#include <wlan_hdd_misc.h>
#include <wlan_hdd_wext.h>
#ifdef WLAN_BTAMP_FEATURE
#include <bap_hdd_main.h>
#include <bapInternal.h>
#endif //                   

#include <linux/wireless.h>
#include <net/cfg80211.h>
#include "wlan_hdd_cfg80211.h"
#include "wlan_hdd_p2p.h"
#include <linux/rtnetlink.h>
int wlan_hdd_ftm_start(hdd_context_t *pAdapter);
#include "sapApi.h"
#include <linux/semaphore.h>
#include <linux/ctype.h>
#include <mach/subsystem_restart.h>
#include <wlan_hdd_hostapd.h>
#include <wlan_hdd_softap_tx_rx.h>
#include "cfgApi.h"
#include "wlan_hdd_dev_pwr.h"
#ifdef WLAN_BTAMP_FEATURE
#include "bap_hdd_misc.h"
#endif
#include "wlan_qct_pal_trace.h"
#include "qwlan_version.h"
#include "wlan_qct_wda.h"
#ifdef FEATURE_WLAN_TDLS
#include "wlan_hdd_tdls.h"
#endif

#ifdef MODULE
#define WLAN_MODULE_NAME  module_name(THIS_MODULE)
#else
#define WLAN_MODULE_NAME  "wlan"
#endif

#ifdef TIMER_MANAGER
#define TIMER_MANAGER_STR " +TIMER_MANAGER"
#else
#define TIMER_MANAGER_STR ""
#endif

#ifdef MEMORY_DEBUG
#define MEMORY_DEBUG_STR " +MEMORY_DEBUG"
#else
#define MEMORY_DEBUG_STR ""
#endif

/*                                                                      */
#define BUF_LEN 20
static char fwpath_buffer[BUF_LEN];
static struct kparam_string fwpath = {
   .string = fwpath_buffer,
   .maxlen = BUF_LEN,
};
#ifndef MODULE
static int wlan_hdd_inited;
#endif

/*
                                                                 
                                                  
  
 */
#define WLAN_HDD_RESTART_RETRY_DELAY_MS 5000  /*          */
#define WLAN_HDD_RESTART_RETRY_MAX_CNT  5     /*           */

/*
                                                  
 */
#define SIZE_OF_SETROAMMODE             11    /*                     */
#define SIZE_OF_GETROAMMODE             11    /*                     */

/*
                                        
                   
 */
#define WLAN_HDD_DRIVER_MIRACAST_CFG_MIN_VAL 0
#define WLAN_HDD_DRIVER_MIRACAST_CFG_MAX_VAL 2

#ifdef WLAN_OPEN_SOURCE
static struct wake_lock wlan_wake_lock;
#endif
/*                                     */
static e_hdd_ssr_required isSsrRequired = HDD_SSR_NOT_REQUIRED;

//                             
static VOS_STATUS wlan_hdd_framework_restart(hdd_context_t *pHddCtx);
static void wlan_hdd_restart_init(hdd_context_t *pHddCtx);
static void wlan_hdd_restart_deinit(hdd_context_t *pHddCtx);
void wlan_hdd_restart_timer_cb(v_PVOID_t usrDataForCallback);
void hdd_set_wlan_suspend_mode(bool suspend);

v_U16_t hdd_select_queue(struct net_device *dev,
    struct sk_buff *skb);

#ifdef WLAN_FEATURE_PACKET_FILTERING
static void hdd_set_multicast_list(struct net_device *dev);
#endif

void hdd_wlan_initial_scan(hdd_adapter_t *pAdapter);
int isWDresetInProgress(void);

extern int hdd_setBand_helper(struct net_device *dev, tANI_U8* ptr);
#if  defined (WLAN_FEATURE_VOWIFI_11R) || defined (FEATURE_WLAN_CCX) || defined(FEATURE_WLAN_LFR)
void hdd_getBand_helper(hdd_context_t *pHddCtx, int *pBand);
static VOS_STATUS hdd_parse_channellist(tANI_U8 *pValue, tANI_U8 *pChannelList, tANI_U8 *pNumChannels);
static VOS_STATUS hdd_parse_countryrev(tANI_U8 *pValue, tANI_U8 *pChannelList, tANI_U8 *pNumChannels);
static VOS_STATUS hdd_parse_send_action_frame_data(tANI_U8 *pValue, tANI_U8 *pTargetApBssid,
                              tANI_U8 *pChannel, tANI_U8 *pDwellTime,
                              tANI_U8 **pBuf, tANI_U8 *pBufLen);
static VOS_STATUS hdd_parse_reassoc_command_data(tANI_U8 *pValue,
                                                 tANI_U8 *pTargetApBssid,
                                                 tANI_U8 *pChannel);
#endif
static int hdd_netdev_notifier_call(struct notifier_block * nb,
                                         unsigned long state,
                                         void *ndev)
{
   struct net_device *dev = ndev;
   hdd_adapter_t *pAdapter = WLAN_HDD_GET_PRIV_PTR(dev);
   hdd_context_t *pHddCtx;
#ifdef WLAN_BTAMP_FEATURE
   VOS_STATUS status;
   hdd_context_t *pHddCtx;
#endif

   //                                                       
   if ((strncmp(dev->name, "wlan", 4)) &&
      (strncmp(dev->name, "p2p", 3)))
      return NOTIFY_DONE;

   if (isWDresetInProgress())
      return NOTIFY_DONE;

   if (!dev->ieee80211_ptr)
      return NOTIFY_DONE;

   if (NULL == pAdapter)
   {
      hddLog(VOS_TRACE_LEVEL_FATAL,"%s: HDD Adapter Null Pointer", __func__);
      VOS_ASSERT(0);
      return NOTIFY_DONE;
   }

   pHddCtx = WLAN_HDD_GET_CTX(pAdapter);
   if (NULL == pHddCtx)
   {
      hddLog(VOS_TRACE_LEVEL_FATAL,"%s: HDD Context Null Pointer", __func__);
      VOS_ASSERT(0);
      return NOTIFY_DONE;
   }

   hddLog(VOS_TRACE_LEVEL_INFO, "%s: %s New Net Device State = %lu",
          __func__, dev->name, state);

   switch (state) {
   case NETDEV_REGISTER:
        break;

   case NETDEV_UNREGISTER:
        break;

   case NETDEV_UP:
        break;

   case NETDEV_DOWN:
        break;

   case NETDEV_CHANGE:
        if(TRUE == pAdapter->isLinkUpSvcNeeded)
           complete(&pAdapter->linkup_event_var);
        break;

   case NETDEV_GOING_DOWN:
        if( pHddCtx->scan_info.mScanPending != FALSE )
        { 
           int result;
           INIT_COMPLETION(pHddCtx->scan_info.abortscan_event_var);
           hdd_abort_mac_scan(pAdapter->pHddCtx);
           result = wait_for_completion_interruptible_timeout(
                               &pHddCtx->scan_info.abortscan_event_var,
                               msecs_to_jiffies(WLAN_WAIT_TIME_ABORTSCAN));
           if(!result)
           {
              VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR,
                         "%s: Timeout occurred while waiting for abortscan" ,
                          __func__);
           }
        }
        else
        {
           VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_INFO,
               "%s: Scan is not Pending from user" , __func__);
        }
#ifdef WLAN_BTAMP_FEATURE
        VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_INFO,"%s: disabling AMP", __func__);
        status = WLANBAP_StopAmp();
        if(VOS_STATUS_SUCCESS != status )
        {
           pHddCtx->isAmpAllowed = VOS_TRUE;
           hddLog(VOS_TRACE_LEVEL_FATAL,
                  "%s: Failed to stop AMP", __func__);
        }
        else
        {
           //                                                            
           msleep(500);
           if ( pHddCtx->isAmpAllowed )
           {
                WLANBAP_DeregisterFromHCI();
                pHddCtx->isAmpAllowed = VOS_FALSE;
           }
        }
#endif //                  
        break;

   default:
        break;
   }

   return NOTIFY_DONE;
}

struct notifier_block hdd_netdev_notifier = {
   .notifier_call = hdd_netdev_notifier_call,
};

/*                                                                           
                         
                                                                           */
void hdd_unregister_mcast_bcast_filter(hdd_context_t *pHddCtx);
void hdd_register_mcast_bcast_filter(hdd_context_t *pHddCtx);
//                                      
static int con_mode;
#ifndef MODULE
/*                                                          
                                                                        
                                                                   
                                                                         
                                                                             
                                                        
 */
static int curr_con_mode;
#endif

/*                                                                            

                                                                    

                                                                    
                           

                                                                   
                                                          

                

                                                                            */
static void hdd_vos_trace_enable(VOS_MODULE_ID moduleId, v_U32_t bitmask)
{
   wpt_tracelevel level;

   /*                                                            
                                                                
                                                      */
   if (CFG_VOS_TRACE_ENABLE_DEFAULT == bitmask)
   {
      return;
   }

   /*                                                       */
   vos_trace_setValue(moduleId, VOS_TRACE_LEVEL_NONE, 0);

   /*                                                                 */
   level = VOS_TRACE_LEVEL_FATAL;
   while (0 != bitmask)
   {
      if (bitmask & 1)
      {
         vos_trace_setValue(moduleId, level, 1);
      }
      level++;
      bitmask >>= 1;
   }
}


/*                                                                            

                                                                    

                                                                    
                                      

                                                                   
                                                         

                

                                                                            */
static void hdd_wdi_trace_enable(wpt_moduleid moduleId, v_U32_t bitmask)
{
   wpt_tracelevel level;

   /*                                                            
                                                                
                                                      */
   if (CFG_WDI_TRACE_ENABLE_DEFAULT == bitmask)
   {
      return;
   }

   /*                                                       */
   wpalTraceSetLevel(moduleId, eWLAN_PAL_TRACE_LEVEL_NONE, 0);

   /*                                                                 */
   level = eWLAN_PAL_TRACE_LEVEL_FATAL;
   while (0 != bitmask)
   {
      if (bitmask & 1)
      {
         wpalTraceSetLevel(moduleId, level, 1);
      }
      level++;
      bitmask >>= 1;
   }
}

/*
                                      
                                                 
 */
int wlan_hdd_validate_context(hdd_context_t *pHddCtx)
{
    ENTER();

    if (NULL == pHddCtx || NULL == pHddCtx->cfg_ini)
    {
        VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR,
                "%s: HDD context is Null", __func__);
        return -ENODEV;
    }

    if (pHddCtx->isLogpInProgress)
    {
        VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR,
                  "%s: LOGP in Progress. Ignore!!!", __func__);
        return -EAGAIN;
    }

    if (pHddCtx->isLoadUnloadInProgress)
    {
        VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR,
                  "%s: Unloading/Loading in Progress. Ignore!!!", __func__);
        return -EAGAIN;
    }
    return 0;
}

void hdd_checkandupdate_phymode( hdd_adapter_t *pAdapter, char *country_code)
{
    hdd_station_ctx_t *pHddStaCtx = WLAN_HDD_GET_STATION_CTX_PTR(pAdapter);
    hdd_context_t *pHddCtx = WLAN_HDD_GET_CTX(pAdapter);
    hdd_config_t *cfg_param;
    eCsrPhyMode phyMode;

    if (NULL == pHddCtx)
    {
        VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_FATAL,
                "HDD Context is null !!");
        return ;
    }

    cfg_param = pHddCtx->cfg_ini;

    if (NULL == cfg_param)
    {
        VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_FATAL,
                "cfg_params not available !!");
        return ;
    }

    phyMode = sme_GetPhyMode(WLAN_HDD_GET_HAL_CTX(pAdapter));

    if (NULL != strstr(cfg_param->listOfNon11acCountryCode, country_code))
    {
        if ((eCSR_DOT11_MODE_AUTO == phyMode) ||
            (eCSR_DOT11_MODE_11ac == phyMode) ||
            (eCSR_DOT11_MODE_11ac_ONLY == phyMode))
        {
             VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_INFO,
                     "Setting phymode to 11n!!");
            sme_SetPhyMode(WLAN_HDD_GET_HAL_CTX(pAdapter), eCSR_DOT11_MODE_11n);
        }
    }
    else
    {
        /*                                                                */
        sme_SetPhyMode(WLAN_HDD_GET_HAL_CTX(pAdapter),
              hdd_cfg_xlate_to_csr_phy_mode(cfg_param->dot11Mode));
        return ;
    }

    if ((eConnectionState_Associated == pHddStaCtx->conn_info.connState) &&
        ((eCSR_CFG_DOT11_MODE_11AC_ONLY == pHddStaCtx->conn_info.dot11Mode) ||
         (eCSR_CFG_DOT11_MODE_11AC == pHddStaCtx->conn_info.dot11Mode)))
    {
        VOS_STATUS vosStatus;

        //                                   
        INIT_COMPLETION(pAdapter->disconnect_comp_var);
        vosStatus = sme_RoamDisconnect(WLAN_HDD_GET_HAL_CTX(pAdapter),
                           pAdapter->sessionId,
                           eCSR_DISCONNECT_REASON_UNSPECIFIED );

        if (VOS_STATUS_SUCCESS == vosStatus)
            wait_for_completion_interruptible_timeout(&pAdapter->disconnect_comp_var,
                  msecs_to_jiffies(WLAN_WAIT_TIME_DISCONNECT));

    }
}

void hdd_checkandupdate_dfssetting( hdd_adapter_t *pAdapter, char *country_code)
{
    hdd_context_t *pHddCtx = WLAN_HDD_GET_CTX(pAdapter);
    hdd_config_t *cfg_param;

    if (NULL == pHddCtx)
    {
        VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_FATAL,
                "HDD Context is null !!");
        return ;
    }

    cfg_param = pHddCtx->cfg_ini;

    if (NULL == cfg_param)
    {
        VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_FATAL,
                "cfg_params not available !!");
        return ;
    }

    if (NULL != strstr(cfg_param->listOfNonDfsCountryCode, country_code))
    {
       /*                                */
       sme_UpdateDfsSetting(WLAN_HDD_GET_HAL_CTX(pAdapter), 0);
    }
    else
    {
       /*                                                               */
       sme_UpdateDfsSetting(WLAN_HDD_GET_HAL_CTX(pAdapter), cfg_param->enableDFSChnlScan);
    }

}

int hdd_ioctl(struct net_device *dev, struct ifreq *ifr, int cmd)
{
   hdd_adapter_t *pAdapter = WLAN_HDD_GET_PRIV_PTR(dev);
   hdd_priv_data_t priv_data;
   tANI_U8 *command = NULL;
   int ret = 0;

   if (NULL == pAdapter)
   {
      VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_FATAL,
         "%s: HDD adapter context is Null", __func__);
      ret = -ENODEV;
      goto exit; 
   }

   if ((!ifr) || (!ifr->ifr_data))
   {
       ret = -EINVAL;
       goto exit; 
   }

   if ((WLAN_HDD_GET_CTX(pAdapter))->isLogpInProgress)
   {
       VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_INFO,
                                  "%s:LOGP in Progress. Ignore!!!", __func__);
       ret = -EBUSY;
       goto exit;
   }

   if (copy_from_user(&priv_data, ifr->ifr_data, sizeof(hdd_priv_data_t)))
   {
       ret = -EFAULT;
       goto exit;
   }

   if (priv_data.total_len <= 0)
   {
       VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_WARN,
                 "%s:invalid priv_data.total_len(%d)!!!", __func__,
                 priv_data.total_len);
       ret = -EINVAL;
       goto exit;
   }

   /*                      */
   command = kmalloc(priv_data.total_len + 1, GFP_KERNEL);
   if (!command)
   {
       VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_FATAL,
          "%s: failed to allocate memory\n", __func__);
       ret = -ENOMEM;
       goto exit;
   }

   if (copy_from_user(command, priv_data.buf, priv_data.total_len))
   {
       ret = -EFAULT;
       goto exit;
   }

   /*                                           */
   command[priv_data.total_len] = '\0';

   if ((SIOCDEVPRIVATE + 1) == cmd)
   {
       hdd_context_t *pHddCtx = (hdd_context_t*)pAdapter->pHddCtx;
       struct wiphy *wiphy = pHddCtx->wiphy;

       VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_INFO,
                  "%s: Received %s cmd from Wi-Fi GUI***", __func__, command);

       if (strncmp(command, "P2P_DEV_ADDR", 12) == 0 )
       {
           if (copy_to_user(priv_data.buf, pHddCtx->p2pDeviceAddress.bytes,
                                                           sizeof(tSirMacAddr)))
           {
               VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_FATAL,
                  "%s: failed to copy data to user buffer\n", __func__);
               ret = -EFAULT;
           }
       }
       else if(strncmp(command, "SETBAND", 7) == 0)
       {
           tANI_U8 *ptr = command ;
           int ret = 0 ;

           /*                              */

           /*                                       
                                                  */
           VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_INFO,
                    "%s: SetBandCommand Info  comm %s UL %d, TL %d", __func__, command, priv_data.used_len, priv_data.total_len);
           /*                              */
           ret = hdd_setBand_helper(dev, ptr);
       }
       else if(strncmp(command, "SETWMMPS", 8) == 0)
       {
           tANI_U8 *ptr = command;
           ret = hdd_wmmps_helper(pAdapter, ptr);
       }
       else if ( strncasecmp(command, "COUNTRY", 7) == 0 )
       {
           char *country_code;

           country_code = command + 8;

           hdd_checkandupdate_dfssetting(pAdapter, country_code);
           hdd_checkandupdate_phymode(pAdapter, country_code);
           ret = (int)sme_ChangeCountryCode(pHddCtx->hHal, NULL, country_code,
                    pAdapter, pHddCtx->pvosContext, eSIR_TRUE);
           if( 0 != ret )
           {
               VOS_TRACE( VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_FATAL,
                       "%s: SME Change Country code fail ret=%d\n",__func__, ret);

           }
           /*                                                             
                                                                      
                          */
           if (memcmp(pHddCtx->cfg_ini->crdaDefaultCountryCode,
                            CFG_CRDA_DEFAULT_COUNTRY_CODE_DEFAULT , 2) == 0)
           {
              regulatory_hint(wiphy, "00");
           }
       }
#if  defined (WLAN_FEATURE_VOWIFI_11R) || defined (FEATURE_WLAN_CCX) || defined(FEATURE_WLAN_LFR)
       else if ( strncasecmp(command, "SETCOUNTRYREV", 13) == 0 )
       {
           tANI_U8 *value = command;
           tANI_U8 countryCode[WNI_CFG_COUNTRY_CODE_LEN] = {0};
           tANI_U8 revision = 0;
           eHalStatus status = eHAL_STATUS_SUCCESS;
           v_REGDOMAIN_t regId;

           status = hdd_parse_countryrev(value, countryCode, &revision);
           if (eHAL_STATUS_SUCCESS != status)
           {
               VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR,
                  "%s: Failed to parse country revision information", __func__);
               ret = -EINVAL;
               goto exit;
           }

           /*                       */
           status = sme_GetRegulatoryDomainForCountry(pHddCtx->hHal, countryCode, &regId);
           if (eHAL_STATUS_SUCCESS != status)
           {
               VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR,
                  "%s: Invalid country code %s", __func__, countryCode);
               ret = -EINVAL;
               goto exit;
           }

           /*                   */
           if ((SME_KR_3 != revision) && (SME_KR_24 != revision) && (SME_KR_25 != revision))
           {
               VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR,
                  "%s: Invalid revision %d", __func__, revision);
               ret = -EINVAL;
               goto exit;
           }

           hdd_checkandupdate_dfssetting(pAdapter, countryCode);
           hdd_checkandupdate_phymode(pAdapter, countryCode);
           ret = (int)sme_ChangeCountryCode(pHddCtx->hHal, NULL, countryCode,
                    pAdapter, pHddCtx->pvosContext, eSIR_TRUE);
           if (0 != ret)
           {
               VOS_TRACE( VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR,
                       "%s: SME Change Country code fail ret=%d", __func__, ret);
               ret = -EINVAL;
               goto exit;
           }

           if (0 == strncmp(countryCode, "KR", 2))
           {
               status = sme_ChangeCountryValidChannelListByRevision((tHalHandle)(pHddCtx->hHal),
                                                       revision);
               if (eHAL_STATUS_SUCCESS != status)
               {
                   VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR,
                             "%s: Failed to build valid channel list", __func__);
                   ret = -EINVAL;
                   goto exit;
               }
           }
       }
#endif
       /*
                                                  
                                                                                   
       */
       else if(strncmp(command, "SET_SAP_CHANNEL_LIST", 20) == 0)
       {
           tANI_U8 *ptr = command;

           VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_INFO,
                      " Received Command to Set Preferred Channels for SAP in %s", __func__);

           ret = sapSetPreferredChannel(ptr);
       }
       else if(strncmp(command, "SETSUSPENDMODE", 14) == 0)
       {
           int suspend = 0;
           tANI_U8 *ptr = (tANI_U8*)command + 15;

           suspend = *ptr - '0';
           hdd_set_wlan_suspend_mode(suspend);
       }
#ifdef WLAN_FEATURE_NEIGHBOR_ROAMING
       else if (strncmp(command, "SETROAMTRIGGER", 14) == 0)
       {
           tANI_U8 *value = command;
           tANI_S8 rssi = 0;
           tANI_U8 lookUpThreshold = CFG_NEIGHBOR_LOOKUP_RSSI_THRESHOLD_DEFAULT;
           eHalStatus status = eHAL_STATUS_SUCCESS;

           /*                                                    */
           value = value + 15;

           /*                                         */
           ret = kstrtos8(value, 10, &rssi);
           if (ret < 0)
           {
               /*                                                                    
                                 */
               VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR,
                      "%s: kstrtou8 failed Input value may be out of range[%d - %d]",
                      CFG_NEIGHBOR_LOOKUP_RSSI_THRESHOLD_MIN,
                      CFG_NEIGHBOR_LOOKUP_RSSI_THRESHOLD_MAX);
               ret = -EINVAL;
               goto exit;
           }

           lookUpThreshold = abs(rssi);

           if ((lookUpThreshold < CFG_NEIGHBOR_LOOKUP_RSSI_THRESHOLD_MIN) ||
               (lookUpThreshold > CFG_NEIGHBOR_LOOKUP_RSSI_THRESHOLD_MAX))
           {
               VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR,
                      "Neighbor lookup threshold value %d is out of range"
                      " (Min: %d Max: %d)", lookUpThreshold,
                      CFG_NEIGHBOR_LOOKUP_RSSI_THRESHOLD_MIN,
                      CFG_NEIGHBOR_LOOKUP_RSSI_THRESHOLD_MAX);
               ret = -EINVAL;
               goto exit;
           }

           VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_INFO,
                      "%s: Received Command to Set Roam trigger"
                      " (Neighbor lookup threshold) = %d", __func__, lookUpThreshold);

           pHddCtx->cfg_ini->nNeighborLookupRssiThreshold = lookUpThreshold;
           status = sme_setNeighborLookupRssiThreshold((tHalHandle)(pHddCtx->hHal), lookUpThreshold);
           if (eHAL_STATUS_SUCCESS != status)
           {
               VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR,
                  "%s: Failed to set roam trigger, try again", __func__);
               ret = -EPERM;
               goto exit;
           }

           /*                                                          */
           sme_setNeighborReassocRssiThreshold((tHalHandle)(pHddCtx->hHal), lookUpThreshold + 5);
       }
       else if (strncmp(command, "GETROAMTRIGGER", 14) == 0)
       {
           tANI_U8 lookUpThreshold = sme_getNeighborLookupRssiThreshold((tHalHandle)(pHddCtx->hHal));
           int rssi = (-1) * lookUpThreshold;
           char extra[32];
           tANI_U8 len = 0;

           len = scnprintf(extra, sizeof(extra), "%s %d", command, rssi);
           if (copy_to_user(priv_data.buf, &extra, len + 1))
           {
               VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR,
                  "%s: failed to copy data to user buffer", __func__);
               ret = -EFAULT;
               goto exit;
           }
       }
       else if (strncmp(command, "SETROAMSCANPERIOD", 17) == 0)
       {
           tANI_U8 *value = command;
           tANI_U8 roamScanPeriod = 0;
           tANI_U16 neighborEmptyScanRefreshPeriod = CFG_EMPTY_SCAN_REFRESH_PERIOD_DEFAULT;

           /*                                             */
           /*                                                       */
           value = value + 18;
           /*                                         */
           ret = kstrtou8(value, 10, &roamScanPeriod);
           if (ret < 0)
           {
               /*                                                                    
                                 */
               VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR,
                      "%s: kstrtou8 failed Input value may be out of range[%d - %d]",
                      __func__,
                      (CFG_EMPTY_SCAN_REFRESH_PERIOD_MIN/1000),
                      (CFG_EMPTY_SCAN_REFRESH_PERIOD_MAX/1000));
               ret = -EINVAL;
               goto exit;
           }

           if ((roamScanPeriod < (CFG_EMPTY_SCAN_REFRESH_PERIOD_MIN/1000)) ||
               (roamScanPeriod > (CFG_EMPTY_SCAN_REFRESH_PERIOD_MAX/1000)))
           {
               VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR,
                      "Roam scan period value %d is out of range"
                      " (Min: %d Max: %d)", roamScanPeriod,
                      (CFG_EMPTY_SCAN_REFRESH_PERIOD_MIN/1000),
                      (CFG_EMPTY_SCAN_REFRESH_PERIOD_MAX/1000));
               ret = -EINVAL;
               goto exit;
           }
           neighborEmptyScanRefreshPeriod = roamScanPeriod * 1000;

           VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_INFO,
                      "%s: Received Command to Set roam scan period"
                      " (Empty Scan refresh period) = %d", __func__, roamScanPeriod);

           pHddCtx->cfg_ini->nEmptyScanRefreshPeriod = neighborEmptyScanRefreshPeriod;
           sme_UpdateEmptyScanRefreshPeriod((tHalHandle)(pHddCtx->hHal), neighborEmptyScanRefreshPeriod);
       }
       else if (strncmp(command, "GETROAMSCANPERIOD", 17) == 0)
       {
           tANI_U16 nEmptyScanRefreshPeriod = sme_getEmptyScanRefreshPeriod((tHalHandle)(pHddCtx->hHal));
           char extra[32];
           tANI_U8 len = 0;

           len = scnprintf(extra, sizeof(extra), "%s %d", "GETROAMSCANPERIOD", (nEmptyScanRefreshPeriod/1000));
           /*                                       */
           if (copy_to_user(priv_data.buf, &extra, len + 1))
           {
               VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR,
                  "%s: failed to copy data to user buffer", __func__);
               ret = -EFAULT;
               goto exit;
           }
       }
       else if (strncmp(command, "SETROAMSCANREFRESHPERIOD", 24) == 0)
       {
           tANI_U8 *value = command;
           tANI_U8 roamScanRefreshPeriod = 0;
           tANI_U16 neighborScanRefreshPeriod = CFG_NEIGHBOR_SCAN_RESULTS_REFRESH_PERIOD_DEFAULT;

           /*                                             */
           /*                                                              */
           value = value + 25;

           /*                                         */
           ret = kstrtou8(value, 10, &roamScanRefreshPeriod);
           if (ret < 0)
           {
               /*                                                                    
                                 */
               VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR,
                      "%s: kstrtou8 failed Input value may be out of range[%d - %d]",
                      __func__,
                      (CFG_NEIGHBOR_SCAN_RESULTS_REFRESH_PERIOD_MIN/1000),
                      (CFG_NEIGHBOR_SCAN_RESULTS_REFRESH_PERIOD_MAX/1000));
               ret = -EINVAL;
               goto exit;
           }

           if ((roamScanRefreshPeriod < (CFG_NEIGHBOR_SCAN_RESULTS_REFRESH_PERIOD_MIN/1000)) ||
               (roamScanRefreshPeriod > (CFG_NEIGHBOR_SCAN_RESULTS_REFRESH_PERIOD_MAX/1000)))
           {
               VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR,
                      "Neighbor scan results refresh period value %d is out of range"
                      " (Min: %d Max: %d)", roamScanRefreshPeriod,
                      (CFG_NEIGHBOR_SCAN_RESULTS_REFRESH_PERIOD_MIN/1000),
                      (CFG_NEIGHBOR_SCAN_RESULTS_REFRESH_PERIOD_MAX/1000));
               ret = -EINVAL;
               goto exit;
           }
           neighborScanRefreshPeriod = roamScanRefreshPeriod * 1000;

           VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_INFO,
                      "%s: Received Command to Set roam scan refresh period"
                      " (Scan refresh period) = %d", __func__, roamScanRefreshPeriod);

           pHddCtx->cfg_ini->nNeighborResultsRefreshPeriod = neighborScanRefreshPeriod;
           sme_setNeighborScanRefreshPeriod((tHalHandle)(pHddCtx->hHal), neighborScanRefreshPeriod);
       }
       else if (strncmp(command, "GETROAMSCANREFRESHPERIOD", 24) == 0)
       {
           tANI_U16 value = sme_getNeighborScanRefreshPeriod((tHalHandle)(pHddCtx->hHal));
           char extra[32];
           tANI_U8 len = 0;

           len = scnprintf(extra, sizeof(extra), "%s %d", "GETROAMSCANREFRESHPERIOD", (value/1000));
           /*                                       */
           if (copy_to_user(priv_data.buf, &extra, len + 1))
           {
               VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR,
                  "%s: failed to copy data to user buffer", __func__);
               ret = -EFAULT;
               goto exit;
           }
       }
#ifdef FEATURE_WLAN_LFR
       /*             */
       else if (strncmp(command, "SETROAMMODE", SIZE_OF_SETROAMMODE) == 0)
       {
           tANI_U8 *value = command;
	   tANI_BOOLEAN roamMode = CFG_LFR_FEATURE_ENABLED_DEFAULT;

	   /*                                                 */
	   value = value + SIZE_OF_SETROAMMODE + 1;

	   /*                                         */
	   ret = kstrtou8(value, SIZE_OF_SETROAMMODE, &roamMode);
	   if (ret < 0)
	   {
	      /*                                                                    
                   */
	      VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR,
		   "%s: kstrtou8 failed range [%d - %d]", __func__,
		   CFG_LFR_FEATURE_ENABLED_MIN,
		   CFG_LFR_FEATURE_ENABLED_MAX);
              ret = -EINVAL;
	      goto exit;
	   }
           if ((roamMode < CFG_LFR_FEATURE_ENABLED_MIN) ||
	       (roamMode > CFG_LFR_FEATURE_ENABLED_MAX))
           {
              VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR,
			"Roam Mode value %d is out of range"
			" (Min: %d Max: %d)", roamMode,
			CFG_LFR_FEATURE_ENABLED_MIN,
			CFG_LFR_FEATURE_ENABLED_MAX);
	      ret = -EINVAL;
	      goto exit;
	   }

	   VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_INFO,
		   "%s: Received Command to Set Roam Mode = %d", __func__, roamMode);
           /*
                
                                               
                                               
                                                                      
                                                                                         
     */
	   if (CFG_LFR_FEATURE_ENABLED_MIN == roamMode)
	       roamMode = CFG_LFR_FEATURE_ENABLED_MAX;    /*             */
	   else
	       roamMode = CFG_LFR_FEATURE_ENABLED_MIN;    /*              */

	   pHddCtx->cfg_ini->isFastRoamIniFeatureEnabled = roamMode;
           sme_UpdateIsFastRoamIniFeatureEnabled((tHalHandle)(pHddCtx->hHal), roamMode);
       }
       /*             */
       else if (strncmp(priv_data.buf, "GETROAMMODE", SIZE_OF_GETROAMMODE) == 0)
       {
	   tANI_BOOLEAN roamMode = sme_getIsLfrFeatureEnabled((tHalHandle)(pHddCtx->hHal));
	   char extra[32];
	   tANI_U8 len = 0;

           /*
                                                                                  
            */
           if (CFG_LFR_FEATURE_ENABLED_MIN == roamMode)
	       roamMode = CFG_LFR_FEATURE_ENABLED_MAX;
           else
	       roamMode = CFG_LFR_FEATURE_ENABLED_MIN;

	   len = scnprintf(extra, sizeof(extra), "%s %d", command, roamMode);
	   if (copy_to_user(priv_data.buf, &extra, len + 1))
	   {
               VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR,
                  "%s: failed to copy data to user buffer", __func__);
               ret = -EFAULT;
               goto exit;
	   }
       }
#endif
#endif
#if  defined (WLAN_FEATURE_VOWIFI_11R) || defined (FEATURE_WLAN_CCX) || defined(FEATURE_WLAN_LFR)
       else if (strncmp(command, "SETROAMDELTA", 12) == 0)
       {
           tANI_U8 *value = command;
           tANI_U8 roamRssiDiff = CFG_ROAM_RSSI_DIFF_DEFAULT;

           /*                                                  */
           value = value + 13;
           /*                                         */
           ret = kstrtou8(value, 10, &roamRssiDiff);
           if (ret < 0)
           {
               /*                                                                    
                                 */
               VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR,
                      "%s: kstrtou8 failed range [%d - %d]", __func__,
                      CFG_ROAM_RSSI_DIFF_MIN,
                      CFG_ROAM_RSSI_DIFF_MAX);
               ret = -EINVAL;
               goto exit;
           }

           if ((roamRssiDiff < CFG_ROAM_RSSI_DIFF_MIN) ||
               (roamRssiDiff > CFG_ROAM_RSSI_DIFF_MAX))
           {
               VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR,
                      "Roam rssi diff value %d is out of range"
                      " (Min: %d Max: %d)", roamRssiDiff,
                      CFG_ROAM_RSSI_DIFF_MIN,
                      CFG_ROAM_RSSI_DIFF_MAX);
               ret = -EINVAL;
               goto exit;
           }

           VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_INFO,
                      "%s: Received Command to Set roam rssi diff = %d", __func__, roamRssiDiff);

           pHddCtx->cfg_ini->RoamRssiDiff = roamRssiDiff;
           sme_UpdateRoamRssiDiff((tHalHandle)(pHddCtx->hHal), roamRssiDiff);
       }
       else if (strncmp(priv_data.buf, "GETROAMDELTA", 12) == 0)
       {
           tANI_U8 roamRssiDiff = sme_getRoamRssiDiff((tHalHandle)(pHddCtx->hHal));
           char extra[32];
           tANI_U8 len = 0;

           len = scnprintf(extra, sizeof(extra), "%s %d", command, roamRssiDiff);
           if (copy_to_user(priv_data.buf, &extra, len + 1))
           {
               VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR,
                  "%s: failed to copy data to user buffer", __func__);
               ret = -EFAULT;
               goto exit;
           }
       }
#endif
#if  defined (WLAN_FEATURE_VOWIFI_11R) || defined (FEATURE_WLAN_CCX) || defined(FEATURE_WLAN_LFR)
       else if (strncmp(command, "GETBAND", 7) == 0)
       {
           int band = -1;
           char extra[32];
           tANI_U8 len = 0;
           hdd_getBand_helper(pHddCtx, &band);

           len = scnprintf(extra, sizeof(extra), "%s %d", command, band);
           if (copy_to_user(priv_data.buf, &extra, len + 1))
           {
               VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR,
                  "%s: failed to copy data to user buffer", __func__);
               ret = -EFAULT;
               goto exit;
           }
       }
       else if (strncmp(command, "GETCOUNTRYREV", 13) == 0)
       {
           tANI_U8 pBuf[WNI_CFG_COUNTRY_CODE_LEN];
           tANI_U8 uBufLen = WNI_CFG_COUNTRY_CODE_LEN;
           tANI_U8 revision = 0;
           /*                                                                  
                                                                                                 */
           char extra[32] = {0};
           tANI_U8 len = 0;

           if (eHAL_STATUS_SUCCESS != sme_GetCountryCode( (tHalHandle)(pHddCtx->hHal), pBuf, &uBufLen ))
           {
               VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_FATAL,
                  "%s: failed to get country code", __func__);
               ret = -EFAULT;
               goto exit;
           }
           pBuf[uBufLen] = '\0';
           sme_GetCountryRevision((tHalHandle)(pHddCtx->hHal), &revision);

           if (0 == strncmp(pBuf, "KR", 2))
               len = scnprintf(extra, sizeof(extra), "%s %s %u", command, pBuf, revision);
           else
               len = scnprintf(extra, sizeof(extra), "%s %s", command, pBuf);

           if (copy_to_user(priv_data.buf, &extra, len + 1))
           {
               VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR,
                  "%s: failed to copy data to user buffer", __func__);
               ret = -EFAULT;
               goto exit;
           }
       }
       else if (strncmp(command, "SETROAMSCANCHANNELS", 19) == 0)
       {
           tANI_U8 *value = command;
           tANI_U8 ChannelList[WNI_CFG_VALID_CHANNEL_LIST_LEN] = {0};
           tANI_U8 numChannels = 0;
           eHalStatus status = eHAL_STATUS_SUCCESS;

           status = hdd_parse_channellist(value, ChannelList, &numChannels);
           if (eHAL_STATUS_SUCCESS != status)
           {
               VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR,
                  "%s: Failed to parse channel list information", __func__);
               ret = -EINVAL;
               goto exit;
           }

           if (numChannels > WNI_CFG_VALID_CHANNEL_LIST_LEN)
           {
               VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR,
                  "%s: number of channels (%d) supported exceeded max (%d)", __func__,
                   numChannels, WNI_CFG_VALID_CHANNEL_LIST_LEN);
               ret = -EINVAL;
               goto exit;
           }
           status = sme_ChangeRoamScanChannelList((tHalHandle)(pHddCtx->hHal), ChannelList,
                                                  numChannels);
           if (eHAL_STATUS_SUCCESS != status)
           {
               VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR,
                  "%s: Failed to update channel list information", __func__);
               ret = -EINVAL;
               goto exit;
           }
       }
       else if (strncmp(command, "GETROAMSCANCHANNELS", 19) == 0)
       {
           tANI_U8 ChannelList[WNI_CFG_VALID_CHANNEL_LIST_LEN] = {0};
           tANI_U8 numChannels = 0;
           tANI_U8 len = 0, j = 0;
           char extra[128] = {0};

           if (eHAL_STATUS_SUCCESS != sme_getRoamScanChannelList( (tHalHandle)(pHddCtx->hHal),
                                              ChannelList, &numChannels ))
           {
               VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_FATAL,
                  "%s: failed to get roam scan channel list", __func__);
               ret = -EFAULT;
               goto exit;
           }
           /*                                     
                                                                 */
           /*                                              */
           len = scnprintf(extra, sizeof(extra), "%s %d", command, numChannels);
           for (j = 0; (j < numChannels); j++)
           {
               len += scnprintf(extra + len, sizeof(extra) - len, " %d", ChannelList[j]);
           }

           if (copy_to_user(priv_data.buf, &extra, len + 1))
           {
               VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR,
                  "%s: failed to copy data to user buffer", __func__);
               ret = -EFAULT;
               goto exit;
           }
       }
       else if (strncmp(command, "SENDACTIONFRAME", 15) == 0)
       {
           tANI_U8 *value = command;
           tANI_U8 channel = 0;
           tANI_U8 dwellTime = 0;
           tANI_U8 bufLen = 0;
           tANI_U8 *buf = NULL;
           tSirMacAddr targetApBssid;
           eHalStatus status = eHAL_STATUS_SUCCESS;
           struct ieee80211_channel chan;
           tANI_U8 finalLen = 0;
           tANI_U8 *finalBuf = NULL;
           tANI_U8 temp = 0;
           u64 cookie;
           hdd_station_ctx_t *pHddStaCtx = NULL;
           pHddStaCtx = WLAN_HDD_GET_STATION_CTX_PTR(pAdapter);

           /*                                                 */
           if (eConnectionState_Associated != pHddStaCtx->conn_info.connState)
           {
               VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_INFO, "%s:Not associated!",__func__);
               ret = -EINVAL;
               goto exit;
           }

           status = hdd_parse_send_action_frame_data(value, targetApBssid, &channel,
                                                     &dwellTime, &buf, &bufLen);
           if (eHAL_STATUS_SUCCESS != status)
           {
               VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR,
                  "%s: Failed to parse send action frame data", __func__);
               ret = -EINVAL;
               goto exit;
           }

           /*                                                               
                                                */
           if (VOS_TRUE != vos_mem_compare(targetApBssid,
                                           pHddStaCtx->conn_info.bssId, sizeof(tSirMacAddr)))
           {
               VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_INFO, "%s:STA is not associated to this AP!",__func__);
               ret = -EINVAL;
               goto exit;
           }

           /*                                                               
                                           */
           if (channel != pHddStaCtx->conn_info.operationChannel)
           {
               VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_INFO,
                         "%s: channel(%d) is different from operating channel(%d)",
                         __func__, channel, pHddStaCtx->conn_info.operationChannel);
               ret = -EINVAL;
               goto exit;
           }
           chan.center_freq = sme_ChnToFreq(channel);

           finalLen = bufLen + 24;
           finalBuf = vos_mem_malloc(finalLen);
           if (NULL == finalBuf)
           {
               VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR, "%s:memory allocation failed",__func__);
               ret = -ENOMEM;
               goto exit;
           }
           vos_mem_zero(finalBuf, finalLen);

           /*              */
           temp = SIR_MAC_MGMT_ACTION << 4;
           vos_mem_copy(finalBuf + 0, &temp, sizeof(temp));

           /*           */
           temp = SIR_MAC_MGMT_FRAME;
           vos_mem_copy(finalBuf + 2, &temp, sizeof(temp));

           /*                                            */
           vos_mem_copy(finalBuf + 4, targetApBssid, sizeof(targetApBssid));

           /*                                       */
           vos_mem_copy(finalBuf + 10, pAdapter->macAddressCurrent.bytes, sizeof(pAdapter->macAddressCurrent.bytes));

           /*                             */
           vos_mem_copy(finalBuf + 16, targetApBssid, sizeof(targetApBssid));

           /*                                        */
           vos_mem_copy(finalBuf + 24, buf, bufLen);

           wlan_hdd_action( NULL, dev, &chan, 0, NL80211_CHAN_HT20,
                       1, dwellTime, finalBuf, finalLen,  1,
                       1, &cookie );
           vos_mem_free(finalBuf);
       }
       else if (strncmp(command, "SETWESMODE", 10) == 0)
       {
           tANI_U8 *value = command;
           tANI_BOOLEAN wesMode = CFG_ENABLE_WES_MODE_NAME_DEFAULT;

           /*                                                */
           value = value + 11;
           /*                                         */
           ret = kstrtou8(value, 10, &wesMode);

           if (ret < 0)
           {
               /*                                                                    
                                 */
               VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR,
                      "%s: kstrtou8 failed range [%d - %d]", __func__,
                      CFG_ENABLE_WES_MODE_NAME_MIN,
                      CFG_ENABLE_WES_MODE_NAME_MAX);
               ret = -EINVAL;
               goto exit;
           }

           if ((wesMode < CFG_ENABLE_WES_MODE_NAME_MIN) ||
               (wesMode > CFG_ENABLE_WES_MODE_NAME_MAX))
           {
               VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR,
                      "WES Mode value %d is out of range"
                      " (Min: %d Max: %d)", wesMode,
                      CFG_ENABLE_WES_MODE_NAME_MIN,
                      CFG_ENABLE_WES_MODE_NAME_MAX);
               ret = -EINVAL;
               goto exit;
           }
           VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_INFO,
                      "%s: Received Command to Set WES Mode rssi diff = %d", __func__, wesMode);

           pHddCtx->cfg_ini->isWESModeEnabled = wesMode;
           sme_UpdateWESMode((tHalHandle)(pHddCtx->hHal), wesMode);
       }
       else if (strncmp(command, "GETCCXMODE", 10) == 0)
       {
           tANI_BOOLEAN ccxMode = sme_getIsCcxFeatureEnabled((tHalHandle)(pHddCtx->hHal));
           char extra[32];
           tANI_U8 len = 0;

           len = scnprintf(extra, sizeof(extra), "%s %d", "GETCCXMODE", ccxMode);
           if (copy_to_user(priv_data.buf, &extra, len + 1))
           {
               VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR,
                  "%s: failed to copy data to user buffer", __func__);
               ret = -EFAULT;
               goto exit;
           }
       }
       else if (strncmp(command, "GETOKCMODE", 10) == 0)
       {
           tANI_BOOLEAN okcMode = hdd_is_okc_mode_enabled(pHddCtx);
           char extra[32];
           tANI_U8 len = 0;

           len = scnprintf(extra, sizeof(extra), "%s %d", "GETOKCMODE", okcMode);
           if (copy_to_user(priv_data.buf, &extra, len + 1))
           {
               VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR,
                  "%s: failed to copy data to user buffer", __func__);
               ret = -EFAULT;
               goto exit;
           }
       }
       else if (strncmp(command, "GETFASTROAM", 11) == 0)
       {
           tANI_BOOLEAN lfrMode = sme_getIsLfrFeatureEnabled((tHalHandle)(pHddCtx->hHal));
           char extra[32];
           tANI_U8 len = 0;

           len = scnprintf(extra, sizeof(extra), "%s %d", "GETFASTROAM", lfrMode);
           if (copy_to_user(priv_data.buf, &extra, len + 1))
           {
               VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR,
                  "%s: failed to copy data to user buffer", __func__);
               ret = -EFAULT;
               goto exit;
           }
       }
       else if (strncmp(command, "GETFASTTRANSITION", 17) == 0)
       {
           tANI_BOOLEAN ft = sme_getIsFtFeatureEnabled((tHalHandle)(pHddCtx->hHal));
           char extra[32];
           tANI_U8 len = 0;

           len = scnprintf(extra, sizeof(extra), "%s %d", "GETFASTTRANSITION", ft);
           if (copy_to_user(priv_data.buf, &extra, len + 1))
           {
               VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR,
                  "%s: failed to copy data to user buffer", __func__);
               ret = -EFAULT;
               goto exit;
           }
       }
       else if (strncmp(command, "SETROAMSCANCHANNELMINTIME", 25) == 0)
       {
           tANI_U8 *value = command;
           tANI_U8 minTime = CFG_NEIGHBOR_SCAN_MIN_CHAN_TIME_DEFAULT;

           /*                                                               */
           value = value + 26;
           /*                                         */
           ret = kstrtou8(value, 10, &minTime);
           if (ret < 0)
           {
               /*                                                                    
                                 */
               VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR,
                      "%s: kstrtou8 failed range [%d - %d]", __func__,
                      CFG_NEIGHBOR_SCAN_MIN_CHAN_TIME_MIN,
                      CFG_NEIGHBOR_SCAN_MIN_CHAN_TIME_MAX);
               ret = -EINVAL;
               goto exit;
           }

           if ((minTime < CFG_NEIGHBOR_SCAN_MIN_CHAN_TIME_MIN) ||
               (minTime > CFG_NEIGHBOR_SCAN_MIN_CHAN_TIME_MAX))
           {
               VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR,
                      "scan min channel time value %d is out of range"
                      " (Min: %d Max: %d)", minTime,
                      CFG_NEIGHBOR_SCAN_MIN_CHAN_TIME_MIN,
                      CFG_NEIGHBOR_SCAN_MIN_CHAN_TIME_MAX);
               ret = -EINVAL;
               goto exit;
           }

           VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_INFO,
                      "%s: Received Command to change channel min time = %d", __func__, minTime);

           pHddCtx->cfg_ini->nNeighborScanMinChanTime = minTime;
           sme_setNeighborScanMinChanTime((tHalHandle)(pHddCtx->hHal), minTime);
       }
       else if (strncmp(command, "GETROAMSCANCHANNELMINTIME", 25) == 0)
       {
           tANI_U16 val = sme_getNeighborScanMinChanTime((tHalHandle)(pHddCtx->hHal));
           char extra[32];
           tANI_U8 len = 0;

           /*                          */
           len = scnprintf(extra, sizeof(extra), "%s %d", "GETROAMSCANCHANNELMINTIME", val);
           if (copy_to_user(priv_data.buf, &extra, len + 1))
           {
               VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR,
                  "%s: failed to copy data to user buffer", __func__);
               ret = -EFAULT;
               goto exit;
           }
       }
       else if (strncmp(command, "SETSCANCHANNELTIME", 18) == 0)
       {
           tANI_U8 *value = command;
           tANI_U8 maxTime = CFG_NEIGHBOR_SCAN_MAX_CHAN_TIME_DEFAULT;

           /*                                                        */
           value = value + 19;
           /*                                         */
           ret = kstrtou8(value, 10, &maxTime);
           if (ret < 0)
           {
               /*                                                                    
                                 */
               VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR,
                      "%s: kstrtou8 failed range [%d - %d]", __func__,
                      CFG_NEIGHBOR_SCAN_MAX_CHAN_TIME_MIN,
                      CFG_NEIGHBOR_SCAN_MAX_CHAN_TIME_MAX);
               ret = -EINVAL;
               goto exit;
           }

           if ((maxTime < CFG_NEIGHBOR_SCAN_MAX_CHAN_TIME_MIN) ||
               (maxTime > CFG_NEIGHBOR_SCAN_MAX_CHAN_TIME_MAX))
           {
               VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR,
                      "lfr mode value %d is out of range"
                      " (Min: %d Max: %d)", maxTime,
                      CFG_NEIGHBOR_SCAN_MAX_CHAN_TIME_MIN,
                      CFG_NEIGHBOR_SCAN_MAX_CHAN_TIME_MAX);
               ret = -EINVAL;
               goto exit;
           }

           VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_INFO,
                      "%s: Received Command to change channel max time = %d", __func__, maxTime);

           pHddCtx->cfg_ini->nNeighborScanMaxChanTime = maxTime;
           sme_setNeighborScanMaxChanTime((tHalHandle)(pHddCtx->hHal), maxTime);
       }
       else if (strncmp(command, "GETSCANCHANNELTIME", 18) == 0)
       {
           tANI_U16 val = sme_getNeighborScanMaxChanTime((tHalHandle)(pHddCtx->hHal));
           char extra[32];
           tANI_U8 len = 0;

           /*                          */
           len = scnprintf(extra, sizeof(extra), "%s %d", "GETSCANCHANNELTIME", val);
           if (copy_to_user(priv_data.buf, &extra, len + 1))
           {
               VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR,
                  "%s: failed to copy data to user buffer", __func__);
               ret = -EFAULT;
               goto exit;
           }
       }
       else if (strncmp(command, "SETSCANHOMETIME", 15) == 0)
       {
           tANI_U8 *value = command;
           tANI_U16 val = CFG_NEIGHBOR_SCAN_TIMER_PERIOD_DEFAULT;

           /*                                                     */
           value = value + 16;
           /*                                         */
           ret = kstrtou16(value, 10, &val);
           if (ret < 0)
           {
               /*                                                                    
                                  */
               VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR,
                      "%s: kstrtou16 failed range [%d - %d]", __func__,
                      CFG_NEIGHBOR_SCAN_TIMER_PERIOD_MIN,
                      CFG_NEIGHBOR_SCAN_TIMER_PERIOD_MAX);
               ret = -EINVAL;
               goto exit;
           }

           if ((val < CFG_NEIGHBOR_SCAN_TIMER_PERIOD_MIN) ||
               (val > CFG_NEIGHBOR_SCAN_TIMER_PERIOD_MAX))
           {
               VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR,
                      "scan home time value %d is out of range"
                      " (Min: %d Max: %d)", val,
                      CFG_NEIGHBOR_SCAN_TIMER_PERIOD_MIN,
                      CFG_NEIGHBOR_SCAN_TIMER_PERIOD_MAX);
               ret = -EINVAL;
               goto exit;
           }

           VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_INFO,
                      "%s: Received Command to change scan home time = %d", __func__, val);

           pHddCtx->cfg_ini->nNeighborScanPeriod = val;
           sme_setNeighborScanPeriod((tHalHandle)(pHddCtx->hHal), val);
       }
       else if (strncmp(command, "GETSCANHOMETIME", 15) == 0)
       {
           tANI_U16 val = sme_getNeighborScanPeriod((tHalHandle)(pHddCtx->hHal));
           char extra[32];
           tANI_U8 len = 0;

           /*                          */
           len = scnprintf(extra, sizeof(extra), "%s %d", "GETSCANHOMETIME", val);
           if (copy_to_user(priv_data.buf, &extra, len + 1))
           {
               VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR,
                  "%s: failed to copy data to user buffer", __func__);
               ret = -EFAULT;
               goto exit;
           }
       }
       else if (strncmp(command, "SETROAMINTRABAND", 16) == 0)
       {
           tANI_U8 *value = command;
           tANI_U8 val = CFG_ROAM_INTRA_BAND_DEFAULT;

           /*                                                      */
           value = value + 17;
           /*                                         */
           ret = kstrtou8(value, 10, &val);
           if (ret < 0)
           {
               /*                                                                    
                                 */
               VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR,
                      "%s: kstrtou8 failed range [%d - %d]", __func__,
                      CFG_ROAM_INTRA_BAND_MIN,
                      CFG_ROAM_INTRA_BAND_MAX);
               ret = -EINVAL;
               goto exit;
           }

           if ((val < CFG_ROAM_INTRA_BAND_MIN) ||
               (val > CFG_ROAM_INTRA_BAND_MAX))
           {
               VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR,
                      "intra band mode value %d is out of range"
                      " (Min: %d Max: %d)", val,
                      CFG_ROAM_INTRA_BAND_MIN,
                      CFG_ROAM_INTRA_BAND_MAX);
               ret = -EINVAL;
               goto exit;
           }

           VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_INFO,
                      "%s: Received Command to change intra band = %d", __func__, val);

           pHddCtx->cfg_ini->nRoamIntraBand = val;
           sme_setRoamIntraBand((tHalHandle)(pHddCtx->hHal), val);
       }
       else if (strncmp(command, "GETROAMINTRABAND", 16) == 0)
       {
           tANI_U16 val = sme_getRoamIntraBand((tHalHandle)(pHddCtx->hHal));
           char extra[32];
           tANI_U8 len = 0;

           /*                          */
           len = scnprintf(extra, sizeof(extra), "%s %d", "GETROAMINTRABAND", val);
           if (copy_to_user(priv_data.buf, &extra, len + 1))
           {
               VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR,
                  "%s: failed to copy data to user buffer", __func__);
               ret = -EFAULT;
               goto exit;
           }
       }

       else if (strncmp(command, "SETSCANNPROBES", 14) == 0)
       {
           tANI_U8 *value = command;
           tANI_U8 nProbes = CFG_ROAM_SCAN_N_PROBES_DEFAULT;

           /*                                                    */
           value = value + 15;
           /*                                         */
           ret = kstrtou8(value, 10, &nProbes);
           if (ret < 0)
           {
               /*                                                                    
                                 */
               VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR,
                      "%s: kstrtou8 failed range [%d - %d]", __func__,
                      CFG_ROAM_SCAN_N_PROBES_MIN,
                      CFG_ROAM_SCAN_N_PROBES_MAX);
               ret = -EINVAL;
               goto exit;
           }

           if ((nProbes < CFG_ROAM_SCAN_N_PROBES_MIN) ||
               (nProbes > CFG_ROAM_SCAN_N_PROBES_MAX))
           {
               VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR,
                      "NProbes value %d is out of range"
                      " (Min: %d Max: %d)", nProbes,
                      CFG_ROAM_SCAN_N_PROBES_MIN,
                      CFG_ROAM_SCAN_N_PROBES_MAX);
               ret = -EINVAL;
               goto exit;
           }

           VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_INFO,
                      "%s: Received Command to Set nProbes = %d", __func__, nProbes);

           pHddCtx->cfg_ini->nProbes = nProbes;
           sme_UpdateRoamScanNProbes((tHalHandle)(pHddCtx->hHal), nProbes);
       }
       else if (strncmp(priv_data.buf, "GETSCANNPROBES", 14) == 0)
       {
           tANI_U8 val = sme_getRoamScanNProbes((tHalHandle)(pHddCtx->hHal));
           char extra[32];
           tANI_U8 len = 0;

           len = scnprintf(extra, sizeof(extra), "%s %d", command, val);
           if (copy_to_user(priv_data.buf, &extra, len + 1))
           {
               VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR,
                  "%s: failed to copy data to user buffer", __func__);
               ret = -EFAULT;
               goto exit;
           }
       }
       else if (strncmp(command, "SETSCANHOMEAWAYTIME", 19) == 0)
       {
           tANI_U8 *value = command;
           tANI_U16 homeAwayTime = CFG_ROAM_SCAN_HOME_AWAY_TIME_DEFAULT;

           /*                                                         */
           /*                                 */
           value = value + 20;
           /*                                         */
           ret = kstrtou16(value, 10, &homeAwayTime);
           if (ret < 0)
           {
               /*                                                                    
                                 */
               VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR,
                      "%s: kstrtou8 failed range [%d - %d]", __func__,
                      CFG_ROAM_SCAN_HOME_AWAY_TIME_MIN,
                      CFG_ROAM_SCAN_HOME_AWAY_TIME_MAX);
               ret = -EINVAL;
               goto exit;
           }

           /*                                              
                                                         
                                                             
                                                    */
           if (0 != homeAwayTime)
           if ((homeAwayTime < CFG_ROAM_SCAN_HOME_AWAY_TIME_MIN) ||
               (homeAwayTime > CFG_ROAM_SCAN_HOME_AWAY_TIME_MAX))
           {
               VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR,
                      "homeAwayTime value %d is out of range"
                      " (Min: %d Max: %d)", homeAwayTime,
                      CFG_ROAM_SCAN_HOME_AWAY_TIME_MIN,
                      CFG_ROAM_SCAN_HOME_AWAY_TIME_MAX);
               ret = -EINVAL;
               goto exit;
           }

           VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_INFO,
                      "%s: Received Command to Set scan away time = %d", __func__, homeAwayTime);

           pHddCtx->cfg_ini->nRoamScanHomeAwayTime = homeAwayTime;
           sme_UpdateRoamScanHomeAwayTime((tHalHandle)(pHddCtx->hHal), homeAwayTime);
       }
       else if (strncmp(priv_data.buf, "GETSCANHOMEAWAYTIME", 19) == 0)
       {
           tANI_U16 val = sme_getRoamScanHomeAwayTime((tHalHandle)(pHddCtx->hHal));
           char extra[32];
           tANI_U8 len = 0;

           len = scnprintf(extra, sizeof(extra), "%s %d", command, val);
           if (copy_to_user(priv_data.buf, &extra, len + 1))
           {
               VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR,
                  "%s: failed to copy data to user buffer", __func__);
               ret = -EFAULT;
               goto exit;
           }
       }
       else if (strncmp(command, "REASSOC", 7) == 0)
       {
           tANI_U8 *value = command;
           tANI_U8 channel = 0;
           tSirMacAddr targetApBssid;
           eHalStatus status = eHAL_STATUS_SUCCESS;

           hdd_station_ctx_t *pHddStaCtx = NULL;
           tANI_BOOLEAN wesMode = eANI_BOOLEAN_FALSE;
           pHddStaCtx = WLAN_HDD_GET_STATION_CTX_PTR(pAdapter);

           wesMode = sme_GetWESMode((tHalHandle)(pHddCtx->hHal));

           /*                                                        */
           if (!wesMode)
           {
               VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_INFO, "%s:WES Mode is not enabled(%d)",__func__, wesMode);
               ret = -EINVAL;
               goto exit;
           }

           /*                                                    */
           if (eConnectionState_Associated != pHddStaCtx->conn_info.connState)
           {
               VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_INFO, "%s:Not associated!",__func__);
               ret = -EINVAL;
               goto exit;
           }

           status = hdd_parse_reassoc_command_data(value, targetApBssid, &channel);
           if (eHAL_STATUS_SUCCESS != status)
           {
               VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR,
                  "%s: Failed to parse reassoc command data", __func__);
               ret = -EINVAL;
               goto exit;
           }

           /*                                                        
                                                   */
           if (VOS_TRUE == vos_mem_compare(targetApBssid,
                                           pHddStaCtx->conn_info.bssId, sizeof(tSirMacAddr)))
           {
               VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_INFO, "%s:Reassoc BSSID is same as currently associated AP bssid",__func__);
               ret = -EINVAL;
               goto exit;
           }

           /*                                                */
           if(VOS_STATUS_SUCCESS !=
                         wlan_hdd_validate_operation_channel(pAdapter, channel))
           {
               hddLog(VOS_TRACE_LEVEL_ERROR,
                      "%s: Invalid Channel [%d] \n", __func__, channel);
               return -EINVAL;
           }

           /*                      */
       }
#endif
#ifdef FEATURE_WLAN_LFR
       else if (strncmp(command, "SETFASTROAM", 11) == 0)
       {
           tANI_U8 *value = command;
           tANI_U8 lfrMode = CFG_LFR_FEATURE_ENABLED_DEFAULT;

           /*                                                 */
           value = value + 12;
           /*                                         */
           ret = kstrtou8(value, 10, &lfrMode);
           if (ret < 0)
           {
               /*                                                                    
                                 */
               VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR,
                      "%s: kstrtou8 failed range [%d - %d]", __func__,
                      CFG_LFR_FEATURE_ENABLED_MIN,
                      CFG_LFR_FEATURE_ENABLED_MAX);
               ret = -EINVAL;
               goto exit;
           }

           if ((lfrMode < CFG_LFR_FEATURE_ENABLED_MIN) ||
               (lfrMode > CFG_LFR_FEATURE_ENABLED_MAX))
           {
               VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR,
                      "lfr mode value %d is out of range"
                      " (Min: %d Max: %d)", lfrMode,
                      CFG_LFR_FEATURE_ENABLED_MIN,
                      CFG_LFR_FEATURE_ENABLED_MAX);
               ret = -EINVAL;
               goto exit;
           }

           VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_INFO,
                      "%s: Received Command to change lfr mode = %d", __func__, lfrMode);

           pHddCtx->cfg_ini->isFastRoamIniFeatureEnabled = lfrMode;
           sme_UpdateIsFastRoamIniFeatureEnabled((tHalHandle)(pHddCtx->hHal), lfrMode);
       }
#endif
#ifdef WLAN_FEATURE_VOWIFI_11R
       else if (strncmp(command, "SETFASTTRANSITION", 17) == 0)
       {
           tANI_U8 *value = command;
           tANI_U8 ft = CFG_FAST_TRANSITION_ENABLED_NAME_DEFAULT;

           /*                                                 */
           value = value + 18;
           /*                                         */
           ret = kstrtou8(value, 10, &ft);
           if (ret < 0)
           {
               /*                                                                    
                                 */
               VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR,
                      "%s: kstrtou8 failed range [%d - %d]", __func__,
                      CFG_FAST_TRANSITION_ENABLED_NAME_MIN,
                      CFG_FAST_TRANSITION_ENABLED_NAME_MAX);
               ret = -EINVAL;
               goto exit;
           }

           if ((ft < CFG_FAST_TRANSITION_ENABLED_NAME_MIN) ||
               (ft > CFG_FAST_TRANSITION_ENABLED_NAME_MAX))
           {
               VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR,
                      "ft mode value %d is out of range"
                      " (Min: %d Max: %d)", ft,
                      CFG_FAST_TRANSITION_ENABLED_NAME_MIN,
                      CFG_FAST_TRANSITION_ENABLED_NAME_MAX);
               ret = -EINVAL;
               goto exit;
           }

           VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_INFO,
                      "%s: Received Command to change ft mode = %d", __func__, ft);

           pHddCtx->cfg_ini->isFastTransitionEnabled = ft;
           sme_UpdateFastTransitionEnabled((tHalHandle)(pHddCtx->hHal), ft);
       }
#endif
#ifdef FEATURE_WLAN_CCX
       else if (strncmp(command, "SETCCXMODE", 10) == 0)
       {
           tANI_U8 *value = command;
           tANI_U8 ccxMode = CFG_CCX_FEATURE_ENABLED_DEFAULT;

           /*                                                */
           value = value + 11;
           /*                                         */
           ret = kstrtou8(value, 10, &ccxMode);
           if (ret < 0)
           {
               /*                                                                    
                                 */
               VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR,
                      "%s: kstrtou8 failed range [%d - %d]", __func__,
                      CFG_CCX_FEATURE_ENABLED_MIN,
                      CFG_CCX_FEATURE_ENABLED_MAX);
               ret = -EINVAL;
               goto exit;
           }

           if ((ccxMode < CFG_CCX_FEATURE_ENABLED_MIN) ||
               (ccxMode > CFG_CCX_FEATURE_ENABLED_MAX))
           {
               VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR,
                      "Ccx mode value %d is out of range"
                      " (Min: %d Max: %d)", ccxMode,
                      CFG_CCX_FEATURE_ENABLED_MIN,
                      CFG_CCX_FEATURE_ENABLED_MAX);
               ret = -EINVAL;
               goto exit;
           }

           VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_INFO,
                      "%s: Received Command to change ccx mode = %d", __func__, ccxMode);

           pHddCtx->cfg_ini->isCcxIniFeatureEnabled = ccxMode;
           sme_UpdateIsCcxFeatureEnabled((tHalHandle)(pHddCtx->hHal), ccxMode);
       }
#endif
#ifdef FEATURE_WLAN_OKC
       else if (strncmp(command, "SETOKCMODE", 10) == 0)
       {
           tANI_U8 *value = command;
           tANI_U8 okcMode = CFG_OKC_FEATURE_ENABLED_DEFAULT;

           /*                                                */
           value = value + 11;
           /*                                         */
           ret = kstrtou8(value, 10, &okcMode);
           if (ret < 0)
           {
               /*                                                                    
                                 */
               VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR,
                      "%s: kstrtou8 failed range [%d - %d]", __func__,
                      CFG_OKC_FEATURE_ENABLED_MIN,
                      CFG_OKC_FEATURE_ENABLED_MAX);
               ret = -EINVAL;
               goto exit;
           }

           if ((okcMode < CFG_OKC_FEATURE_ENABLED_MIN) ||
               (okcMode > CFG_OKC_FEATURE_ENABLED_MAX))
           {
               VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR,
                      "Okc mode value %d is out of range"
                      " (Min: %d Max: %d)", okcMode,
                      CFG_OKC_FEATURE_ENABLED_MIN,
                      CFG_OKC_FEATURE_ENABLED_MAX);
               ret = -EINVAL;
               goto exit;
           }

           VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_INFO,
                      "%s: Received Command to change okc mode = %d", __func__, okcMode);

           pHddCtx->cfg_ini->isOkcIniFeatureEnabled = okcMode;
       }
#endif
       else if (strncmp(command, "SETROAMSCANCONTROL", 18) == 0)
       {
           tANI_U8 *value = command;
           tANI_BOOLEAN roamScanControl = 0;

           /*                                                        */
           value = value + 19;
           /*                                         */
           ret = kstrtou8(value, 10, &roamScanControl);
           if (ret < 0)
           {
               /*                                                                    
                                 */
               VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR,
                      "%s: kstrtou8 failed ", __func__);
               ret = -EINVAL;
               goto exit;
           }

           if (0 != roamScanControl)
           {
               VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR,
                      "roam scan control invalid value = %d",
                      roamScanControl);
               ret = -EINVAL;
               goto exit;
           }
           VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_INFO,
                      "%s: Received Command to Set roam scan control = %d", __func__, roamScanControl);

           sme_SetRoamScanControl((tHalHandle)(pHddCtx->hHal), roamScanControl);
       }
       else if (strncmp(priv_data.buf, "GETROAMSCANCONTROL", 18) == 0)
       {
           tANI_BOOLEAN roamScanControl = sme_GetRoamScanControl((tHalHandle)(pHddCtx->hHal));
           char extra[32];
           tANI_U8 len = 0;

           len = scnprintf(extra, sizeof(extra), "%s %d", command, roamScanControl);
           if (copy_to_user(priv_data.buf, &extra, len + 1))
           {
               VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR,
                  "%s: failed to copy data to user buffer", __func__);
               ret = -EFAULT;
               goto exit;
           }
       }
#ifdef WLAN_FEATURE_PACKET_FILTERING
       else if (strncmp(command, "ENABLE_PKTFILTER_IPV6", 21) == 0)
       {
           tANI_U8 filterType = 0;
           tANI_U8 *value = command;

           /*                                                           */
           value = value + 22;

           /*                                         */
           ret = kstrtou8(value, 10, &filterType);
           if (ret < 0)
           {
               /*                                                          
                                          
                */
               VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR,
                      "%s: kstrtou8 failed range ", __func__);
               ret = -EINVAL;
               goto exit;
           }

           if (filterType != 0 && filterType != 1)
           {
               VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR,
                      "%s: Accepted Values are 0 and 1 ", __func__);
               ret = -EINVAL;
               goto exit;
           }
           wlan_hdd_setIPv6Filter(WLAN_HDD_GET_CTX(pAdapter), filterType,
                   pAdapter->sessionId);
       }
#endif
       else if (strncmp(command, "GETDWELLTIME", 12) == 0)
       {
           hdd_config_t *pCfg = (WLAN_HDD_GET_CTX(pAdapter))->cfg_ini;
           char extra[32];
           tANI_U8 len = 0;

           len = scnprintf(extra, sizeof(extra), "GETDWELLTIME %u\n",
                  (int)pCfg->nActiveMaxChnTime);
           if (copy_to_user(priv_data.buf, &extra, len + 1))
           {
               VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR,
                  "%s: failed to copy data to user buffer", __func__);
               ret = -EFAULT;
               goto exit;
           }
           ret = len;
       }
       else if (strncmp(command, "SETDWELLTIME", 12) == 0)
       {
           tANI_U8 *value = command;
           hdd_config_t *pCfg = (WLAN_HDD_GET_CTX(pAdapter))->cfg_ini;
           int val = 0, temp;

           value = value + 13;
           temp = kstrtou32(value, 10, &val);
           if ( temp != 0 || val < CFG_ACTIVE_MAX_CHANNEL_TIME_MIN ||
                             val > CFG_ACTIVE_MAX_CHANNEL_TIME_MAX )
           {
               VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR,
                "%s: argument passed for SETDWELLTIME is incorrect", __func__);
               ret = -EFAULT;
               goto exit;
           }
           pCfg->nActiveMaxChnTime = val;
       }
       else if (strncmp(command, "SCAN-ACTIVE", 11) == 0)
       {
          pHddCtx->scan_info.scan_mode = eSIR_ACTIVE_SCAN;
       }
       else if (strncmp(command, "SCAN-PASSIVE", 12) == 0)
       {
          pHddCtx->scan_info.scan_mode = eSIR_PASSIVE_SCAN;
       }
       else if ( strncasecmp(command, "MIRACAST", 8) == 0 )
       {
           tANI_U8 filterType = 0;
           tANI_U8 *value;
           value = command + 9;

           /*                                         */
           ret = kstrtou8(value, 10, &filterType);
           if (ret < 0)
           {
               /*                                                          
                                          
                */
              VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR,
                              "%s: kstrtou8 failed range ", __func__);
              ret = -EINVAL;
              goto exit;
           }
           if ((filterType < WLAN_HDD_DRIVER_MIRACAST_CFG_MIN_VAL ) ||
               (filterType > WLAN_HDD_DRIVER_MIRACAST_CFG_MAX_VAL))
           {
               VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR,
                      "%s: Accepted Values are 0 to 2. 0-Disabled, 1-Source,"
                      " 2-Sink ", __func__);
               ret = -EINVAL;
               goto exit;
           }
           //                                                              
           pHddCtx->drvr_miracast = filterType;
           hdd_tx_rx_pkt_cnt_stat_timer_handler(pHddCtx);
        }
       else {
           hddLog( VOS_TRACE_LEVEL_WARN, "%s: Unsupported GUI command %s",
                   __func__, command);
       }

   }
exit:
   if (command)
   {
       kfree(command);
   }
   return ret;
}

#if  defined (WLAN_FEATURE_VOWIFI_11R) || defined (FEATURE_WLAN_CCX) || defined(FEATURE_WLAN_LFR)
void hdd_getBand_helper(hdd_context_t *pHddCtx, int *pBand)
{
    eCsrBand band = -1;
    sme_GetFreqBand((tHalHandle)(pHddCtx->hHal), &band);
    switch (band)
    {
        case eCSR_BAND_ALL:
            *pBand = WLAN_HDD_UI_BAND_AUTO;
            break;

        case eCSR_BAND_24:
            *pBand = WLAN_HDD_UI_BAND_2_4_GHZ;
            break;

        case eCSR_BAND_5G:
            *pBand = WLAN_HDD_UI_BAND_5_GHZ;
            break;

        default:
            hddLog( VOS_TRACE_LEVEL_WARN, "%s: Invalid Band %d", __func__, band);
            *pBand = -1;
            break;
    }
}

/*                                                                            

                                                                              

                                                                      
                                                                              

                                                         
                                                     
                                                     
                                                                                              
                                
                                          

                                              

                                                                            */
VOS_STATUS hdd_parse_send_action_frame_data(tANI_U8 *pValue, tANI_U8 *pTargetApBssid, tANI_U8 *pChannel,
                                            tANI_U8 *pDwellTime, tANI_U8 **pBuf, tANI_U8 *pBufLen)
{
    tANI_U8 *inPtr = pValue;
    tANI_U8 *dataEnd;
    int tempInt;
    int j = 0;
    int i = 0;
    int v = 0;
    tANI_U8 tempBuf[32];
    tANI_U8 tempByte = 0;
    /*                                  */
    tANI_U8 macAddress[17];

    inPtr = strnchr(pValue, strlen(pValue), SPACE_ASCII_VALUE);
    /*                             */
    if (NULL == inPtr)
    {
        return -EINVAL;
    }

    /*                          */
    else if (SPACE_ASCII_VALUE != *inPtr)
    {
        return -EINVAL;
    }

    /*                     */
    while ((SPACE_ASCII_VALUE  == *inPtr) && ('\0' !=  *inPtr) ) inPtr++;

    /*                              */
    if ('\0' == *inPtr)
    {
        return -EINVAL;
    }

    v = sscanf(inPtr, "%17s", macAddress);
    if (!((1 == v) && hdd_is_valid_mac_address(macAddress)))
    {
        VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR,
             "Invalid MAC address or All hex inputs are not read (%d)", v);
        return -EINVAL;
    }

    pTargetApBssid[0] = hdd_parse_hex(macAddress[0]) << 4 | hdd_parse_hex(macAddress[1]);
    pTargetApBssid[1] = hdd_parse_hex(macAddress[3]) << 4 | hdd_parse_hex(macAddress[4]);
    pTargetApBssid[2] = hdd_parse_hex(macAddress[6]) << 4 | hdd_parse_hex(macAddress[7]);
    pTargetApBssid[3] = hdd_parse_hex(macAddress[9]) << 4 | hdd_parse_hex(macAddress[10]);
    pTargetApBssid[4] = hdd_parse_hex(macAddress[12]) << 4 | hdd_parse_hex(macAddress[13]);
    pTargetApBssid[5] = hdd_parse_hex(macAddress[15]) << 4 | hdd_parse_hex(macAddress[16]);

    /*                            */
    inPtr = strnchr(inPtr, strlen(inPtr), SPACE_ASCII_VALUE);
    /*                             */
    if (NULL == inPtr) return -EINVAL;

    /*                     */
    while ((SPACE_ASCII_VALUE  == *inPtr) && ('\0' !=  *inPtr) ) inPtr++;

    /*                              */
    if ('\0' == *inPtr)
    {
        return -EINVAL;
    }

    /*                                                */
    v = sscanf(inPtr, "%32s ", tempBuf);
    if (1 != v) return -EINVAL;

    v = kstrtos32(tempBuf, 10, &tempInt);
    if ( v < 0 || tempInt <= 0 || tempInt > WNI_CFG_CURRENT_CHANNEL_STAMAX )
     return -EINVAL;

    *pChannel = tempInt;

    /*                            */
    inPtr = strnchr(inPtr, strlen(inPtr), SPACE_ASCII_VALUE);
    /*                             */
    if (NULL == inPtr) return -EINVAL;
    /*                     */
    while ((SPACE_ASCII_VALUE  == *inPtr) && ('\0' !=  *inPtr) ) inPtr++;

    /*                              */
    if ('\0' == *inPtr)
    {
        return -EINVAL;
    }

    /*                                            */
    v = sscanf(inPtr, "%32s ", tempBuf);
    if (1 != v) return -EINVAL;

    v = kstrtos32(tempBuf, 10, &tempInt);
    if ( v < 0 || tempInt < 0) return -EINVAL;

    *pDwellTime = tempInt;

    /*                            */
    inPtr = strnchr(inPtr, strlen(inPtr), SPACE_ASCII_VALUE);
    /*                             */
    if (NULL == inPtr) return -EINVAL;
    /*                     */
    while ((SPACE_ASCII_VALUE  == *inPtr) && ('\0' !=  *inPtr) ) inPtr++;

    /*                              */
    if ('\0' == *inPtr)
    {
        return -EINVAL;
    }

    /*                         */
    dataEnd = inPtr;
    while(('\0' !=  *dataEnd) )
    {
        dataEnd++;
    }
    *pBufLen = dataEnd - inPtr ;
    if ( *pBufLen <= 0)  return -EINVAL;

    /*                                                                     
                                 
                                                                         
                                                                            
                               
                                                                    
                                                                     */
    *pBuf = vos_mem_malloc((*pBufLen + 1)/2);
    if (NULL == *pBuf)
    {
        hddLog(VOS_TRACE_LEVEL_FATAL,
           "%s: vos_mem_alloc failed ", __func__);
        return -EINVAL;
    }

    /*                                                              
                                                                                      
                                                                                    
                              */
    for (i = 0, j = 0; j < *pBufLen; j += 2)
    {
        if( j+1 == *pBufLen)
        {
             tempByte = hdd_parse_hex(inPtr[j]);
        }
        else
        {
              tempByte = (hdd_parse_hex(inPtr[j]) << 4) | (hdd_parse_hex(inPtr[j + 1]));
        }
        (*pBuf)[i++] = tempByte;
    }
    *pBufLen = i;
    return VOS_STATUS_SUCCESS;
}

/*                                                                            

                                                                 

                                                                     
                                                   

                                                         
                                                                             
                                                              

                                              

                                                                            */
VOS_STATUS hdd_parse_countryrev(tANI_U8 *pValue, tANI_U8 *pCountryCode, tANI_U8 *pRevision)
{
    tANI_U8 *inPtr = pValue;
    int tempInt;

    inPtr = strnchr(pValue, strlen(pValue), SPACE_ASCII_VALUE);
    /*                             */
    if (NULL == inPtr)
    {
        return -EINVAL;
    }

    /*                          */
    else if (SPACE_ASCII_VALUE != *inPtr)
    {
        return -EINVAL;
    }

    /*                     */
    while ((SPACE_ASCII_VALUE  == *inPtr)&& ('\0' !=  *inPtr) ) inPtr++;

    /*                              */
    if ('\0' == *inPtr)
    {
        return -EINVAL;
    }

    /*                                               */
    sscanf(inPtr, "%3s ", pCountryCode);

    VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_INFO_HIGH,
               "Country code is : %s", pCountryCode);

    /*                                                                  */
    inPtr = strpbrk( inPtr, " " );
    /*                                                   */
    if (NULL == inPtr)
    {
        return -EINVAL;
    }

    inPtr++;

    /*                    */
    while ((SPACE_ASCII_VALUE == *inPtr) && ('\0' != *inPtr) ) inPtr++;

    /*                                                                */
    if (0 == strncmp(pCountryCode, "KR", 2))
    {
        if ('\0' == *inPtr)
        {
            return -EINVAL;
        }

        sscanf(inPtr, "%d", &tempInt);
        *pRevision = tempInt;
    }
    else
    {
        VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_INFO_HIGH,
               "Revision input is required only for Country KR");
        return -EINVAL;
    }
    return VOS_STATUS_SUCCESS;
}

/*                                                                            

                                                         

                                                            
                                                                                                

                                                
                                                                            
                                                                

                                              

                                                                            */
VOS_STATUS hdd_parse_channellist(tANI_U8 *pValue, tANI_U8 *pChannelList, tANI_U8 *pNumChannels)
{
    tANI_U8 *inPtr = pValue;
    int tempInt;
    int j = 0;
    int v = 0;
    char buf[32];

    inPtr = strnchr(pValue, strlen(pValue), SPACE_ASCII_VALUE);
    /*                             */
    if (NULL == inPtr)
    {
        return -EINVAL;
    }

    /*                          */
    else if (SPACE_ASCII_VALUE != *inPtr)
    {
        return -EINVAL;
    }

    /*                     */
    while ((SPACE_ASCII_VALUE  == *inPtr) && ('\0' !=  *inPtr)) inPtr++;

    /*                              */
    if ('\0' == *inPtr)
    {
        return -EINVAL;
    }

    /*                                                    */
    v = sscanf(inPtr, "%32s ", buf);
    if (1 != v) return -EINVAL;

    v = kstrtos32(buf, 10, &tempInt);
    if ((v < 0) ||
        (tempInt <= 0) ||
        (tempInt > WNI_CFG_VALID_CHANNEL_LIST_LEN))
    {
       return -EINVAL;
    }

    *pNumChannels = tempInt;

    VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_INFO_HIGH,
               "Number of channels are: %d", *pNumChannels);

    for (j = 0; j < (*pNumChannels); j++)
    {
        /*                                                                       */
        inPtr = strpbrk( inPtr, " " );
        /*                                                     */
        if (NULL == inPtr)
        {
            if (0 != j)
            {
                *pNumChannels = j;
                return VOS_STATUS_SUCCESS;
            }
            else
            {
                return -EINVAL;
            }
        }

        /*                    */
        while ((SPACE_ASCII_VALUE == *inPtr) && ('\0' != *inPtr)) inPtr++;

        /*                                                                */
        if ( '\0' == *inPtr )
        {
            if (0 != j)
            {
                *pNumChannels = j;
                return VOS_STATUS_SUCCESS;
            }
            else
            {
                return -EINVAL;
            }
        }

        v = sscanf(inPtr, "%32s ", buf);
        if (1 != v) return -EINVAL;

        v = kstrtos32(buf, 10, &tempInt);
        if ((v < 0) ||
            (tempInt <= 0) ||
            (tempInt > WNI_CFG_CURRENT_CHANNEL_STAMAX))
        {
           return -EINVAL;
        }
        pChannelList[j] = tempInt;

        VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_INFO_HIGH,
                   "Channel %d added to preferred channel list",
                   pChannelList[j] );
    }

    return VOS_STATUS_SUCCESS;
}


/*                                                                            

                                                                          

                                                                   
                                       

                                                                      
                                                     
                                                     

                                              

                                                                            */
VOS_STATUS hdd_parse_reassoc_command_data(tANI_U8 *pValue,
                            tANI_U8 *pTargetApBssid, tANI_U8 *pChannel)
{
    tANI_U8 *inPtr = pValue;
    int tempInt;
    int v = 0;
    tANI_U8 tempBuf[32];
    /*                                  */
    tANI_U8 macAddress[17];

    inPtr = strnchr(pValue, strlen(pValue), SPACE_ASCII_VALUE);
    /*                             */
    if (NULL == inPtr)
    {
        return -EINVAL;
    }

    /*                          */
    else if (SPACE_ASCII_VALUE != *inPtr)
    {
        return -EINVAL;
    }

    /*                     */
    while ((SPACE_ASCII_VALUE  == *inPtr) && ('\0' !=  *inPtr) ) inPtr++;

    /*                              */
    if ('\0' == *inPtr)
    {
        return -EINVAL;
    }

    v = sscanf(inPtr, "%17s", macAddress);
    if (!((1 == v) && hdd_is_valid_mac_address(macAddress)))
    {
        VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR,
             "Invalid MAC address or All hex inputs are not read (%d)", v);
        return -EINVAL;
    }

    pTargetApBssid[0] = hdd_parse_hex(macAddress[0]) << 4 | hdd_parse_hex(macAddress[1]);
    pTargetApBssid[1] = hdd_parse_hex(macAddress[3]) << 4 | hdd_parse_hex(macAddress[4]);
    pTargetApBssid[2] = hdd_parse_hex(macAddress[6]) << 4 | hdd_parse_hex(macAddress[7]);
    pTargetApBssid[3] = hdd_parse_hex(macAddress[9]) << 4 | hdd_parse_hex(macAddress[10]);
    pTargetApBssid[4] = hdd_parse_hex(macAddress[12]) << 4 | hdd_parse_hex(macAddress[13]);
    pTargetApBssid[5] = hdd_parse_hex(macAddress[15]) << 4 | hdd_parse_hex(macAddress[16]);

    /*                            */
    inPtr = strnchr(inPtr, strlen(inPtr), SPACE_ASCII_VALUE);
    /*                             */
    if (NULL == inPtr) return -EINVAL;

    /*                     */
    while ((SPACE_ASCII_VALUE  == *inPtr) && ('\0' !=  *inPtr) ) inPtr++;

    /*                              */
    if ('\0' == *inPtr)
    {
        return -EINVAL;
    }

    /*                                                */
    v = sscanf(inPtr, "%32s ", tempBuf);
    if (1 != v) return -EINVAL;

    v = kstrtos32(tempBuf, 10, &tempInt);
    if ((v < 0) ||
        (tempInt <= 0) ||
        (tempInt > WNI_CFG_CURRENT_CHANNEL_STAMAX))
    {
        return -EINVAL;
    }

    *pChannel = tempInt;
    return VOS_STATUS_SUCCESS;
}

#endif

/*                                                                            

                                                          

                                                                       
                                                         
                                                                  
                                                                  

                                                                                 
                                                                
                               

                                                     
                                          

                                                                            */

v_BOOL_t hdd_is_valid_mac_address(const tANI_U8 *pMacAddr)
{
    int xdigit = 0;
    int separator = 0;
    while (*pMacAddr)
    {
        if (isxdigit(*pMacAddr))
        {
            xdigit++;
        }
        else if (':' == *pMacAddr)
        {
            if (0 == xdigit || ((xdigit / 2) - 1) != separator)
                break;

            ++separator;
        }
        else
        {
            separator = -1;
            /*                   */
            return 0;
        }
        ++pMacAddr;
    }
    return (xdigit == 12 && (separator == 5 || separator == 0));
}

/*                                                                            

                                       

                                           

                                               

                                              

                                                                            */
int hdd_open (struct net_device *dev)
{
   hdd_adapter_t *pAdapter = WLAN_HDD_GET_PRIV_PTR(dev);
   hdd_context_t *pHddCtx;
   hdd_adapter_list_node_t *pAdapterNode = NULL, *pNext = NULL;
   VOS_STATUS status;
   v_BOOL_t in_standby = TRUE;

   if (NULL == pAdapter) 
   {
      VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_FATAL,
         "%s: HDD adapter context is Null", __func__);
      return -ENODEV;
   }
   
   pHddCtx = (hdd_context_t*)pAdapter->pHddCtx;
   if (NULL == pHddCtx)
   {
      VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_FATAL,
         "%s: HDD context is Null", __func__);
      return -ENODEV;
   }

   status = hdd_get_front_adapter ( pHddCtx, &pAdapterNode );
   while ( (NULL != pAdapterNode) && (VOS_STATUS_SUCCESS == status) )
   {
      if (test_bit(DEVICE_IFACE_OPENED, &pAdapterNode->pAdapter->event_flags))
      {
         hddLog(VOS_TRACE_LEVEL_INFO, "%s: chip already out of standby",
                __func__);
         in_standby = FALSE;
         break;
      }
      else
      {
         status = hdd_get_next_adapter ( pHddCtx, pAdapterNode, &pNext );
         pAdapterNode = pNext;
      }
   }
 
   if (TRUE == in_standby)
   {
       if (VOS_STATUS_SUCCESS != wlan_hdd_exit_lowpower(pHddCtx, pAdapter))
       {
           hddLog(VOS_TRACE_LEVEL_ERROR, "%s: Failed to bring " 
                   "wlan out of power save", __func__);
           return -EINVAL;
       }
   }
   
   set_bit(DEVICE_IFACE_OPENED, &pAdapter->event_flags);
   if (hdd_connIsConnected(WLAN_HDD_GET_STATION_CTX_PTR(pAdapter))) 
   {
       VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_INFO,
                 "%s: Enabling Tx Queues", __func__);
       /*                                             */
       netif_tx_start_all_queues(dev);
   }

   return 0;
}

int hdd_mon_open (struct net_device *dev)
{
   hdd_adapter_t *pAdapter = WLAN_HDD_GET_PRIV_PTR(dev);

   if(pAdapter == NULL) {
      VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_FATAL,
         "%s: HDD adapter context is Null", __func__);
      return -EINVAL;
   }

   netif_start_queue(dev);

   return 0;
}
/*                                                                            

                                       

                                             

                                               

                                              

                                                                            */

int hdd_stop (struct net_device *dev)
{
   hdd_adapter_t *pAdapter = WLAN_HDD_GET_PRIV_PTR(dev);
   hdd_context_t *pHddCtx;
   hdd_adapter_list_node_t *pAdapterNode = NULL, *pNext = NULL;
   VOS_STATUS status;
   v_BOOL_t enter_standby = TRUE;
   
   ENTER();

   if (NULL == pAdapter)
   {
      VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_FATAL,
         "%s: HDD adapter context is Null", __func__);
      return -ENODEV;
   }

   pHddCtx = (hdd_context_t*)pAdapter->pHddCtx;
   if (NULL == pHddCtx)
   {
      VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_FATAL,
         "%s: HDD context is Null", __func__);
      return -ENODEV;
   }

   clear_bit(DEVICE_IFACE_OPENED, &pAdapter->event_flags);
   hddLog(VOS_TRACE_LEVEL_INFO, "%s: Disabling OS Tx queues", __func__);
   netif_tx_disable(pAdapter->dev);
   netif_carrier_off(pAdapter->dev);


   /*                                                 
                             */
   if ( (WLAN_HDD_SOFTAP == pAdapter->device_mode )
                 || (WLAN_HDD_MONITOR == pAdapter->device_mode )
                 || (WLAN_HDD_P2P_GO == pAdapter->device_mode )
      )
   {
      /*                                  */
      EXIT();
      return 0;
   }

   /*                             
                                                                          */
   status = hdd_get_front_adapter ( pHddCtx, &pAdapterNode );
   while ( (NULL != pAdapterNode) && (VOS_STATUS_SUCCESS == status) )
   {
      if (test_bit(DEVICE_IFACE_OPENED, &pAdapterNode->pAdapter->event_flags))
      {
         hddLog(VOS_TRACE_LEVEL_INFO, "%s: Still other ifaces are up cannot "
                "put device to sleep", __func__);
         enter_standby = FALSE;
         break;
      }
      else
      {
         status = hdd_get_next_adapter ( pHddCtx, pAdapterNode, &pNext );
         pAdapterNode = pNext;
      }
   }

   if (TRUE == enter_standby)
   {
       hddLog(VOS_TRACE_LEVEL_INFO, "%s: All Interfaces are Down " 
                 "entering standby", __func__);
       if (VOS_STATUS_SUCCESS != wlan_hdd_enter_lowpower(pHddCtx))
       {
           /*                      */
           hddLog(VOS_TRACE_LEVEL_ERROR, "%s: Failed to put "
                   "wlan in power save", __func__);
       }
   }
   
   EXIT();
   return 0;
}

/*                                                                            

                                           

                                                                      
                          

                                               

                

                                                                            */
static void hdd_uninit (struct net_device *dev)
{
   hdd_adapter_t *pAdapter = WLAN_HDD_GET_PRIV_PTR(dev);

   ENTER();

   do
   {
      if (NULL == pAdapter)
      {
         hddLog(VOS_TRACE_LEVEL_FATAL,
                "%s: NULL pAdapter", __func__);
         break;
      }

      if (WLAN_HDD_ADAPTER_MAGIC != pAdapter->magic)
      {
         hddLog(VOS_TRACE_LEVEL_FATAL,
                "%s: Invalid magic", __func__);
         break;
      }

      if (NULL == pAdapter->pHddCtx)
      {
         hddLog(VOS_TRACE_LEVEL_FATAL,
                "%s: NULL pHddCtx", __func__);
         break;
      }

      if (dev != pAdapter->dev)
      {
         hddLog(VOS_TRACE_LEVEL_FATAL,
                "%s: Invalid device reference", __func__);
         /*                                                       */
      }

      hdd_deinit_adapter(pAdapter->pHddCtx, pAdapter);

      /*                                                            */
      pAdapter->dev = NULL;
      pAdapter->magic = 0;
   } while (0);

   EXIT();
}

/*                                                                            

                                 

                                                                            

                                               
                                                 


                                               

                                                                            */

VOS_STATUS hdd_release_firmware(char *pFileName,v_VOID_t *pCtx)
{
   VOS_STATUS status = VOS_STATUS_SUCCESS;
   hdd_context_t *pHddCtx = (hdd_context_t*)pCtx;
   ENTER();


   if (!strcmp(WLAN_FW_FILE, pFileName)) {
   
       hddLog(VOS_TRACE_LEVEL_INFO_HIGH,"%s: Loaded firmware file is %s",__func__,pFileName);

       if(pHddCtx->fw) {
          release_firmware(pHddCtx->fw);
          pHddCtx->fw = NULL;
       }
       else
          status = VOS_STATUS_E_FAILURE;
   }
   else if (!strcmp(WLAN_NV_FILE,pFileName)) {
       if(pHddCtx->nv) {
          release_firmware(pHddCtx->nv);
          pHddCtx->nv = NULL;
       }
       else
          status = VOS_STATUS_E_FAILURE;

   }

   EXIT();
   return status;
}

/*                                                                            

                                 

                                                                   
                                                                    

                                                 
                                               
                                                                        
                                                 

                                                                            

                                                                            */


VOS_STATUS hdd_request_firmware(char *pfileName,v_VOID_t *pCtx,v_VOID_t **ppfw_data, v_SIZE_t *pSize)
{
   int status;
   VOS_STATUS retval = VOS_STATUS_SUCCESS;
   hdd_context_t *pHddCtx = (hdd_context_t*)pCtx;
   ENTER();

   if( (!strcmp(WLAN_FW_FILE, pfileName)) ) {

       status = request_firmware(&pHddCtx->fw, pfileName, pHddCtx->parent_dev);

       if(status || !pHddCtx->fw || !pHddCtx->fw->data) {
           hddLog(VOS_TRACE_LEVEL_FATAL, "%s: Firmware %s download failed",
                  __func__, pfileName);
           retval = VOS_STATUS_E_FAILURE;
       }

       else {
         *ppfw_data = (v_VOID_t *)pHddCtx->fw->data;
         *pSize = pHddCtx->fw->size;
          hddLog(VOS_TRACE_LEVEL_INFO, "%s: Firmware size = %d",
                 __func__, *pSize);
       }
   }
   else if(!strcmp(WLAN_NV_FILE, pfileName)) {

       status = request_firmware(&pHddCtx->nv, pfileName, pHddCtx->parent_dev);

       if(status || !pHddCtx->nv || !pHddCtx->nv->data) {
           hddLog(VOS_TRACE_LEVEL_FATAL, "%s: nv %s download failed",
                  __func__, pfileName);
           retval = VOS_STATUS_E_FAILURE;
       }

       else {
         *ppfw_data = (v_VOID_t *)pHddCtx->nv->data;
         *pSize = pHddCtx->nv->size;
          hddLog(VOS_TRACE_LEVEL_INFO, "%s: nv file size = %d",
                 __func__, *pSize);
       }
   }

   EXIT();
   return retval;
}
/*                                                                            
                                                                

                                                                              
                           

                                                  
                                         

                   

                                                                          */
void hdd_full_pwr_cbk(void *callbackContext, eHalStatus status)
{
   hdd_context_t *pHddCtx = (hdd_context_t*)callbackContext;

   hddLog(VOS_TRACE_LEVEL_INFO_HIGH,"HDD full Power callback status = %d", status);
   if(&pHddCtx->full_pwr_comp_var)
   {
      complete(&pHddCtx->full_pwr_comp_var);
   }
}

/*                                                                            

                                                                  

                                                                     
                          

                                                 
                                         

                  

                                                                          */
void hdd_req_bmps_cbk(void *callbackContext, eHalStatus status)
{

   struct completion *completion_var = (struct completion*) callbackContext;

   hddLog(VOS_TRACE_LEVEL_ERROR, "HDD BMPS request Callback, status = %d\n", status);
   if(completion_var != NULL)
   {
      complete(completion_var);
   }
}

/*                                                                            

                                  

                                                                        
                                               

                                           
                                                     
                                                      

                                               

                                                                            */

VOS_STATUS hdd_get_cfg_file_size(v_VOID_t *pCtx, char *pFileName, v_SIZE_t *pBufSize)
{
   int status;
   hdd_context_t *pHddCtx = (hdd_context_t*)pCtx;

   ENTER();

   status = request_firmware(&pHddCtx->fw, pFileName, pHddCtx->parent_dev);

   if(status || !pHddCtx->fw || !pHddCtx->fw->data) {
      hddLog(VOS_TRACE_LEVEL_FATAL,"%s: CFG download failed",__func__);
      status = VOS_STATUS_E_FAILURE;
   }
   else {
      *pBufSize = pHddCtx->fw->size;
      hddLog(VOS_TRACE_LEVEL_INFO, "%s: CFG size = %d", __func__, *pBufSize);
      release_firmware(pHddCtx->fw);
      pHddCtx->fw = NULL;
   }

   EXIT();
   return VOS_STATUS_SUCCESS;
}

/*                                                                            

                              

                                                                        
                                                                              

                                           
                                                     
                                                     
                                                      

                                               

                                                                            */

VOS_STATUS hdd_read_cfg_file(v_VOID_t *pCtx, char *pFileName,
    v_VOID_t *pBuffer, v_SIZE_t *pBufSize)
{
   int status;
   hdd_context_t *pHddCtx = (hdd_context_t*)pCtx;

   ENTER();

   status = request_firmware(&pHddCtx->fw, pFileName, pHddCtx->parent_dev);

   if(status || !pHddCtx->fw || !pHddCtx->fw->data) {
      hddLog(VOS_TRACE_LEVEL_FATAL,"%s: CFG download failed",__func__);
      return VOS_STATUS_E_FAILURE;
   }
   else {
      if(*pBufSize != pHddCtx->fw->size) {
         hddLog(VOS_TRACE_LEVEL_ERROR, "%s: Caller sets invalid CFG "
             "file size", __func__);
         release_firmware(pHddCtx->fw);
         pHddCtx->fw = NULL;
         return VOS_STATUS_E_FAILURE;
      }
        else {
         if(pBuffer) {
            vos_mem_copy(pBuffer,pHddCtx->fw->data,*pBufSize);
         }
         release_firmware(pHddCtx->fw);
         pHddCtx->fw = NULL;
        }
   }

   EXIT();

   return VOS_STATUS_SUCCESS;
}

/*                                                                            

                                

                                                          
                                                    

                                            
                                               
                                               

                                                                            */

static int hdd_set_mac_address(struct net_device *dev, void *addr)
{
   hdd_adapter_t *pAdapter = WLAN_HDD_GET_PRIV_PTR(dev);
   struct sockaddr *psta_mac_addr = addr;
   eHalStatus halStatus = eHAL_STATUS_SUCCESS;

   ENTER();

   memcpy(&pAdapter->macAddressCurrent, psta_mac_addr->sa_data, ETH_ALEN);

#ifdef HDD_SESSIONIZE 
   //                                           
   halStatus = ccmCfgSetStr( pAdapter->hHal, WNI_CFG_STA_ID,
                             (v_U8_t *)&pAdapter->macAddressCurrent,
                             sizeof( pAdapter->macAddressCurrent ),
                             hdd_set_mac_addr_cb, VOS_FALSE );
#endif

   memcpy(dev->dev_addr, psta_mac_addr->sa_data, ETH_ALEN);

   EXIT();
   return halStatus;
}

tANI_U8* wlan_hdd_get_intf_addr(hdd_context_t* pHddCtx)
{
   int i;
   for ( i = 0; i < VOS_MAX_CONCURRENCY_PERSONA; i++)
   {
      if( 0 == (pHddCtx->cfg_ini->intfAddrMask >> i))
         break;
   }

   if( VOS_MAX_CONCURRENCY_PERSONA == i)
      return NULL;

   pHddCtx->cfg_ini->intfAddrMask |= (1 << i);
   return &pHddCtx->cfg_ini->intfMacAddr[i].bytes[0];
}

void wlan_hdd_release_intf_addr(hdd_context_t* pHddCtx, tANI_U8* releaseAddr)
{
   int i;
   for ( i = 0; i < VOS_MAX_CONCURRENCY_PERSONA; i++)
   {
      if ( !memcmp(releaseAddr, &pHddCtx->cfg_ini->intfMacAddr[i].bytes[0], 6) )
      {
         pHddCtx->cfg_ini->intfAddrMask &= ~(1 << i);
         break;
      } 
   }
   return;
}

#if (LINUX_VERSION_CODE > KERNEL_VERSION(2,6,29))
  static struct net_device_ops wlan_drv_ops = {
      .ndo_open = hdd_open,
      .ndo_stop = hdd_stop,
      .ndo_uninit = hdd_uninit,
      .ndo_start_xmit = hdd_hard_start_xmit,
      .ndo_tx_timeout = hdd_tx_timeout,
      .ndo_get_stats = hdd_stats,
      .ndo_do_ioctl = hdd_ioctl,
      .ndo_set_mac_address = hdd_set_mac_address,
      .ndo_select_queue    = hdd_select_queue,
#ifdef WLAN_FEATURE_PACKET_FILTERING
#if (LINUX_VERSION_CODE > KERNEL_VERSION(3,1,0))
      .ndo_set_rx_mode = hdd_set_multicast_list,
#else
      .ndo_set_multicast_list = hdd_set_multicast_list,
#endif //                  
#endif
 };
 static struct net_device_ops wlan_mon_drv_ops = {
      .ndo_open = hdd_mon_open,
      .ndo_stop = hdd_stop,
      .ndo_uninit = hdd_uninit,
      .ndo_start_xmit = hdd_mon_hard_start_xmit,  
      .ndo_tx_timeout = hdd_tx_timeout,
      .ndo_get_stats = hdd_stats,
      .ndo_do_ioctl = hdd_ioctl,
      .ndo_set_mac_address = hdd_set_mac_address,
 };

#endif

void hdd_set_station_ops( struct net_device *pWlanDev )
{
#if (LINUX_VERSION_CODE > KERNEL_VERSION(2,6,29))
      pWlanDev->tx_queue_len = NET_DEV_TX_QUEUE_LEN,
      pWlanDev->netdev_ops = &wlan_drv_ops;
#else
      pWlanDev->open = hdd_open;
      pWlanDev->stop = hdd_stop;
      pWlanDev->uninit = hdd_uninit;
      pWlanDev->hard_start_xmit = NULL;
      pWlanDev->tx_timeout = hdd_tx_timeout;
      pWlanDev->get_stats = hdd_stats;
      pWlanDev->do_ioctl = hdd_ioctl;
      pWlanDev->tx_queue_len = NET_DEV_TX_QUEUE_LEN;
      pWlanDev->set_mac_address = hdd_set_mac_address;
#endif
}

static hdd_adapter_t* hdd_alloc_station_adapter( hdd_context_t *pHddCtx, tSirMacAddr macAddr, const char* name )
{
   struct net_device *pWlanDev = NULL;
   hdd_adapter_t *pAdapter = NULL;
   /*
                                                  
    */ 
   pWlanDev = alloc_netdev_mq(sizeof( hdd_adapter_t ), name, ether_setup, NUM_TX_QUEUES);
   
   if(pWlanDev != NULL)
   {

      //                                                     
      pAdapter = (hdd_adapter_t*) netdev_priv( pWlanDev );

      vos_mem_zero( pAdapter, sizeof( hdd_adapter_t ) );

      pAdapter->dev = pWlanDev;
      pAdapter->pHddCtx = pHddCtx; 
      pAdapter->magic = WLAN_HDD_ADAPTER_MAGIC;

      init_completion(&pAdapter->session_open_comp_var);
      init_completion(&pAdapter->session_close_comp_var);
      init_completion(&pAdapter->disconnect_comp_var);
      init_completion(&pAdapter->linkup_event_var);
      init_completion(&pAdapter->cancel_rem_on_chan_var);
      init_completion(&pAdapter->rem_on_chan_ready_event);
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,38))
      init_completion(&pAdapter->offchannel_tx_event);
#endif
      init_completion(&pAdapter->tx_action_cnf_event);
#ifdef FEATURE_WLAN_TDLS
      init_completion(&pAdapter->tdls_add_station_comp);
      init_completion(&pAdapter->tdls_del_station_comp);
      init_completion(&pAdapter->tdls_mgmt_comp);
#endif
      init_completion(&pHddCtx->mc_sus_event_var);
      init_completion(&pHddCtx->tx_sus_event_var);
      init_completion(&pHddCtx->rx_sus_event_var);
      init_completion(&pAdapter->ula_complete);

      pAdapter->isLinkUpSvcNeeded = FALSE; 
      pAdapter->higherDtimTransition = eANI_BOOLEAN_TRUE;
      //                             
      strlcpy(pWlanDev->name, name, IFNAMSIZ);

      vos_mem_copy(pWlanDev->dev_addr, (void *)macAddr, sizeof(tSirMacAddr));
      vos_mem_copy( pAdapter->macAddressCurrent.bytes, macAddr, sizeof(tSirMacAddr));
      pWlanDev->watchdog_timeo = HDD_TX_TIMEOUT;
      pWlanDev->hard_header_len += LIBRA_HW_NEEDED_HEADROOM;

      hdd_set_station_ops( pAdapter->dev );

      pWlanDev->destructor = free_netdev;
      pWlanDev->ieee80211_ptr = &pAdapter->wdev ;
      pAdapter->wdev.wiphy = pHddCtx->wiphy;  
      pAdapter->wdev.netdev =  pWlanDev;
      /*                                            */
      SET_NETDEV_DEV(pWlanDev, pHddCtx->parent_dev);
   }

   return pAdapter;
}

VOS_STATUS hdd_register_interface( hdd_adapter_t *pAdapter, tANI_U8 rtnl_lock_held )
{
   struct net_device *pWlanDev = pAdapter->dev;
   //                                                              
   //                                                      
   //                                           

   if( rtnl_lock_held )
   {
     if (strnchr(pWlanDev->name, strlen(pWlanDev->name), '%')) {
         if( dev_alloc_name(pWlanDev, pWlanDev->name) < 0 )
         {
            hddLog(VOS_TRACE_LEVEL_ERROR,"%s:Failed:dev_alloc_name",__func__);
            return VOS_STATUS_E_FAILURE;            
         }
      }
      if (register_netdevice(pWlanDev))
      {
         hddLog(VOS_TRACE_LEVEL_ERROR,"%s:Failed:register_netdev",__func__);
         return VOS_STATUS_E_FAILURE;         
      }
   }
   else
   {
      if(register_netdev(pWlanDev))
      {
         hddLog(VOS_TRACE_LEVEL_ERROR,"%s: Failed:register_netdev",__func__);
         return VOS_STATUS_E_FAILURE;         
      }
   }
   set_bit(NET_DEVICE_REGISTERED, &pAdapter->event_flags);

   return VOS_STATUS_SUCCESS;
}

static eHalStatus hdd_smeCloseSessionCallback(void *pContext)
{
   hdd_adapter_t *pAdapter = pContext;

   if (NULL == pAdapter)
   {
      hddLog(VOS_TRACE_LEVEL_FATAL, "%s: NULL pAdapter", __func__);
      return eHAL_STATUS_INVALID_PARAMETER;
   }

   if (WLAN_HDD_ADAPTER_MAGIC != pAdapter->magic)
   {
      hddLog(VOS_TRACE_LEVEL_FATAL, "%s: Invalid magic", __func__);
      return eHAL_STATUS_NOT_INITIALIZED;
   }

   clear_bit(SME_SESSION_OPENED, &pAdapter->event_flags);

#ifndef WLAN_OPEN_SOURCE
   /* need to make sure all of our scheduled work has completed.
    * This callback is called from MC thread context, so it is safe to
    * to call below flush workqueue API from here.
    *
    * Even though this is called from MC thread context, if there is a faulty
    * work item in the system, that can hang this call forever.  So flushing
    * this global work queue is not safe; and now we make sure that
    * individual work queues are stopped correctly. But the cancel work queue
    * is a GPL only API, so the proprietary  version of the driver would still
    * rely on the global work queue flush.
    */
   flush_scheduled_work();
#endif

   /*                                                         
                                                                     
                                                                  
                                                                   
                                         */
   if (WLAN_HDD_ADAPTER_MAGIC == pAdapter->magic)
   {
      complete(&pAdapter->session_close_comp_var);
   }

   return eHAL_STATUS_SUCCESS;
}

VOS_STATUS hdd_init_station_mode( hdd_adapter_t *pAdapter )
{
   struct net_device *pWlanDev = pAdapter->dev;
   hdd_station_ctx_t *pHddStaCtx = &pAdapter->sessionCtx.station;
   hdd_context_t *pHddCtx = WLAN_HDD_GET_CTX( pAdapter );
   eHalStatus halStatus = eHAL_STATUS_SUCCESS;
   VOS_STATUS status = VOS_STATUS_E_FAILURE;
   int rc = 0;

   INIT_COMPLETION(pAdapter->session_open_comp_var);
   //                                       
   halStatus = sme_OpenSession( pHddCtx->hHal, hdd_smeRoamCallback, pAdapter,
         (tANI_U8 *)&pAdapter->macAddressCurrent, &pAdapter->sessionId );
   if ( !HAL_STATUS_SUCCESS( halStatus ) )
   {
      hddLog(VOS_TRACE_LEVEL_FATAL,
             "sme_OpenSession() failed with status code %08d [x%08lx]",
                                                 halStatus, halStatus );
      status = VOS_STATUS_E_FAILURE;
      goto error_sme_open;
   }
   
   //                                                          
   rc = wait_for_completion_interruptible_timeout(
                        &pAdapter->session_open_comp_var,
                        msecs_to_jiffies(WLAN_WAIT_TIME_SESSIONOPENCLOSE));
   if (!rc)
   {
      hddLog(VOS_TRACE_LEVEL_FATAL,
             "Session is not opened within timeout period code %08d", rc );
      status = VOS_STATUS_E_FAILURE;
      goto error_sme_open;
   }

   //                             
   if( eHAL_STATUS_SUCCESS !=  (halStatus = hdd_register_wext(pWlanDev)))
   {
      hddLog(VOS_TRACE_LEVEL_FATAL,
              "hdd_register_wext() failed with status code %08d [x%08lx]",
                                                   halStatus, halStatus );
      status = VOS_STATUS_E_FAILURE;
      goto error_register_wext;
   }
   //                                                   
#if (LINUX_VERSION_CODE > KERNEL_VERSION(2,6,29))
   wlan_drv_ops.ndo_start_xmit = hdd_hard_start_xmit;
#else
   pWlanDev->hard_start_xmit = hdd_hard_start_xmit;
#endif

   //                                         
   pHddStaCtx->conn_info.connState = eConnectionState_NotConnected;

   //                                 
   pHddStaCtx->conn_info.operationChannel = pHddCtx->cfg_ini->OperatingChannel;

   /*                                   */
   pHddStaCtx->conn_info.authType = eCSR_AUTH_TYPE_OPEN_SYSTEM;

   if( VOS_STATUS_SUCCESS != ( status = hdd_init_tx_rx( pAdapter ) ) )
   {
      hddLog(VOS_TRACE_LEVEL_FATAL,
            "hdd_init_tx_rx() failed with status code %08d [x%08lx]",
                            status, status );
      goto error_init_txrx;
   }

   set_bit(INIT_TX_RX_SUCCESS, &pAdapter->event_flags);

   if( VOS_STATUS_SUCCESS != ( status = hdd_wmm_adapter_init( pAdapter ) ) )
   {
      hddLog(VOS_TRACE_LEVEL_FATAL,
            "hdd_wmm_adapter_init() failed with status code %08d [x%08lx]",
                            status, status );
      goto error_wmm_init;
   }

   set_bit(WMM_INIT_DONE, &pAdapter->event_flags);

#ifdef FEATURE_WLAN_TDLS
   if(0 != wlan_hdd_tdls_init(pAdapter))
   {
       status = VOS_STATUS_E_FAILURE;
       hddLog(VOS_TRACE_LEVEL_ERROR,"%s: wlan_hdd_tdls_init failed",__func__);
       goto error_tdls_init;
   }
   set_bit(TDLS_INIT_DONE, &pAdapter->event_flags);
#endif

   return VOS_STATUS_SUCCESS;

#ifdef FEATURE_WLAN_TDLS
error_tdls_init:
   clear_bit(WMM_INIT_DONE, &pAdapter->event_flags);
   hdd_wmm_adapter_close(pAdapter);
#endif
error_wmm_init:
   clear_bit(INIT_TX_RX_SUCCESS, &pAdapter->event_flags);
   hdd_deinit_tx_rx(pAdapter);
error_init_txrx:
   hdd_UnregisterWext(pWlanDev);
error_register_wext:
   if (test_bit(SME_SESSION_OPENED, &pAdapter->event_flags))
   {
      INIT_COMPLETION(pAdapter->session_close_comp_var);
      if (eHAL_STATUS_SUCCESS == sme_CloseSession(pHddCtx->hHal,
                                    pAdapter->sessionId,
                                    hdd_smeCloseSessionCallback, pAdapter))
      {
         //                                                          
         wait_for_completion_timeout(
                          &pAdapter->session_close_comp_var,
                          msecs_to_jiffies(WLAN_WAIT_TIME_SESSIONOPENCLOSE));
      }
}
error_sme_open:
   return status;
}

void hdd_cleanup_actionframe( hdd_context_t *pHddCtx, hdd_adapter_t *pAdapter )
{
   hdd_cfg80211_state_t *cfgState;

   cfgState = WLAN_HDD_GET_CFG_STATE_PTR( pAdapter );

   if( NULL != cfgState->buf )
   {
      int rc;
      INIT_COMPLETION(pAdapter->tx_action_cnf_event);
      rc = wait_for_completion_interruptible_timeout(
                     &pAdapter->tx_action_cnf_event,
                     msecs_to_jiffies(ACTION_FRAME_TX_TIMEOUT));
      if(!rc)
      {
         VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR,
              ("ERROR: HDD Wait for Action Confirmation Failed!!\n"));
      }
   }
   return;
}

void hdd_deinit_adapter( hdd_context_t *pHddCtx, hdd_adapter_t *pAdapter )
{
   ENTER();
   switch ( pAdapter->device_mode )
   {
      case WLAN_HDD_INFRA_STATION:
      case WLAN_HDD_P2P_CLIENT:
      case WLAN_HDD_P2P_DEVICE:
      {
         if(test_bit(INIT_TX_RX_SUCCESS, &pAdapter->event_flags))
         {
            hdd_deinit_tx_rx( pAdapter );
            clear_bit(INIT_TX_RX_SUCCESS, &pAdapter->event_flags);
         }

         if(test_bit(WMM_INIT_DONE, &pAdapter->event_flags))
         {
            hdd_wmm_adapter_close( pAdapter );
            clear_bit(WMM_INIT_DONE, &pAdapter->event_flags);
         }

         hdd_cleanup_actionframe(pHddCtx, pAdapter);
#ifdef FEATURE_WLAN_TDLS
         if(test_bit(TDLS_INIT_DONE, &pAdapter->event_flags))
         {
            wlan_hdd_tdls_exit(pAdapter);
            clear_bit(TDLS_INIT_DONE, &pAdapter->event_flags);
         }
#endif

         break;
      }

      case WLAN_HDD_SOFTAP:
      case WLAN_HDD_P2P_GO:
      {

         if (test_bit(WMM_INIT_DONE, &pAdapter->event_flags))
         {
            hdd_wmm_adapter_close( pAdapter );
            clear_bit(WMM_INIT_DONE, &pAdapter->event_flags);
         }

         hdd_cleanup_actionframe(pHddCtx, pAdapter);

         hdd_unregister_hostapd(pAdapter);
         hdd_set_conparam( 0 );
         wlan_hdd_set_monitor_tx_adapter( WLAN_HDD_GET_CTX(pAdapter), NULL );
         break;
      }

      case WLAN_HDD_MONITOR:
      {
          hdd_adapter_t* pAdapterforTx = pAdapter->sessionCtx.monitor.pAdapterForTx;
         if(test_bit(INIT_TX_RX_SUCCESS, &pAdapter->event_flags))
         {
            hdd_deinit_tx_rx( pAdapter );
            clear_bit(INIT_TX_RX_SUCCESS, &pAdapter->event_flags);
         }
         if(NULL != pAdapterforTx)
         {
            hdd_cleanup_actionframe(pHddCtx, pAdapterforTx);
         }
         break;
      }


      default:
      break;
   }

   EXIT();
}

void hdd_cleanup_adapter( hdd_context_t *pHddCtx, hdd_adapter_t *pAdapter, tANI_U8 rtnl_held )
{
   struct net_device *pWlanDev = pAdapter->dev;

   if(test_bit(NET_DEVICE_REGISTERED, &pAdapter->event_flags)) {
      if( rtnl_held )
      {
         unregister_netdevice(pWlanDev);
      }
      else
      {
         unregister_netdev(pWlanDev);
      }
      //                                                        
      //                                    
   }

}

void hdd_set_pwrparams(hdd_context_t *pHddCtx)
{
   VOS_STATUS status;
   hdd_adapter_t *pAdapter = NULL;
   hdd_adapter_list_node_t *pAdapterNode = NULL, *pNext = NULL;

   status =  hdd_get_front_adapter ( pHddCtx, &pAdapterNode );

   /*                          */
   while ( NULL != pAdapterNode && VOS_STATUS_SUCCESS == status )
   {
       pAdapter = pAdapterNode->pAdapter;
       if ( (WLAN_HDD_INFRA_STATION != pAdapter->device_mode)
         && (WLAN_HDD_P2P_CLIENT != pAdapter->device_mode) )

       {  //                                                                         
           status = hdd_get_next_adapter ( pHddCtx, pAdapterNode, &pNext );
           pAdapterNode = pNext;
           continue;
       }

       //                          
       //                                                    
      if ((pHddCtx->cfg_ini->enableDynamicDTIM ||
           pHddCtx->cfg_ini->enableModulatedDTIM) &&
          ((WLAN_HDD_INFRA_STATION == pAdapter->device_mode) ||
          ((WLAN_HDD_P2P_CLIENT == pAdapter->device_mode) &&
          !(pHddCtx->cfg_ini->ignoreDynamicDtimInP2pMode))) &&
          (eANI_BOOLEAN_TRUE == pAdapter->higherDtimTransition) &&
          (eConnectionState_Associated ==
          (WLAN_HDD_GET_STATION_CTX_PTR(pAdapter))->conn_info.connState) &&
          (pHddCtx->cfg_ini->fIsBmpsEnabled))
      {
           tSirSetPowerParamsReq powerRequest = { 0 };

           powerRequest.uIgnoreDTIM = 1;

           if (pHddCtx->cfg_ini->enableModulatedDTIM)
           {
               powerRequest.uDTIMPeriod = pHddCtx->cfg_ini->enableModulatedDTIM;
               powerRequest.uListenInterval = pHddCtx->hdd_actual_LI_value;
           }
           else
           {
               powerRequest.uListenInterval = pHddCtx->cfg_ini->enableDynamicDTIM;
           }

           /*                                                                  
                                                           */
            ccmCfgSetInt(pHddCtx->hHal, WNI_CFG_IGNORE_DTIM, powerRequest.uIgnoreDTIM,
                       NULL, eANI_BOOLEAN_FALSE);
            ccmCfgSetInt(pHddCtx->hHal, WNI_CFG_LISTEN_INTERVAL, powerRequest.uListenInterval,
                       NULL, eANI_BOOLEAN_FALSE);

           /*                                         */
            VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_INFO,
                      "Switch to DTIM %d", powerRequest.uListenInterval);
            sme_SetPowerParams( pHddCtx->hHal, &powerRequest, TRUE);
            break;

      }

      status = hdd_get_next_adapter ( pHddCtx, pAdapterNode, &pNext );
      pAdapterNode = pNext;
    }
}

void hdd_reset_pwrparams(hdd_context_t *pHddCtx)
{
   /*                     */
   tSirSetPowerParamsReq powerRequest = { 0 };

   powerRequest.uIgnoreDTIM = pHddCtx->hdd_actual_ignore_DTIM_value;
   powerRequest.uListenInterval = pHddCtx->hdd_actual_LI_value;

   /*                                                                 */
   ccmCfgSetInt(pHddCtx->hHal, WNI_CFG_IGNORE_DTIM, powerRequest.uIgnoreDTIM,
                    NULL, eANI_BOOLEAN_FALSE);
   ccmCfgSetInt(pHddCtx->hHal, WNI_CFG_LISTEN_INTERVAL, powerRequest.uListenInterval,
                    NULL, eANI_BOOLEAN_FALSE);

   VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_INFO,
                  "Switch to DTIM%d",powerRequest.uListenInterval);
   sme_SetPowerParams( pHddCtx->hHal, &powerRequest, TRUE);

}

VOS_STATUS hdd_enable_bmps_imps(hdd_context_t *pHddCtx)
{
   VOS_STATUS status = VOS_STATUS_SUCCESS;

   if(pHddCtx->cfg_ini->fIsBmpsEnabled)
   {
      sme_EnablePowerSave(pHddCtx->hHal, ePMC_BEACON_MODE_POWER_SAVE);
   }

   if(pHddCtx->cfg_ini->fIsAutoBmpsTimerEnabled)
   {
      sme_StartAutoBmpsTimer(pHddCtx->hHal); 
   }

   if (pHddCtx->cfg_ini->fIsImpsEnabled)
   {
      sme_EnablePowerSave (pHddCtx->hHal, ePMC_IDLE_MODE_POWER_SAVE);
   }

   return status;
}

VOS_STATUS hdd_disable_bmps_imps(hdd_context_t *pHddCtx, tANI_U8 session_type)
{
   hdd_adapter_t *pAdapter = NULL;
   eHalStatus halStatus;
   VOS_STATUS status = VOS_STATUS_E_INVAL;
   v_BOOL_t disableBmps = FALSE;
   v_BOOL_t disableImps = FALSE;
   
   switch(session_type)
   {
       case WLAN_HDD_INFRA_STATION:
       case WLAN_HDD_SOFTAP:
       case WLAN_HDD_P2P_CLIENT:
       case WLAN_HDD_P2P_GO:
          //                                                   
          pAdapter = hdd_get_adapter(pHddCtx, WLAN_HDD_INFRA_STATION);
          if((NULL != pAdapter)&&
              hdd_connIsConnected( WLAN_HDD_GET_STATION_CTX_PTR(pAdapter)))
          {
             disableBmps = TRUE;
          }

          pAdapter = hdd_get_adapter(pHddCtx, WLAN_HDD_P2P_CLIENT);
          if((NULL != pAdapter)&&
              hdd_connIsConnected( WLAN_HDD_GET_STATION_CTX_PTR(pAdapter)))
          {
             disableBmps = TRUE;
          }

          //                                             
          if((WLAN_HDD_SOFTAP == session_type) ||
              (WLAN_HDD_P2P_GO == session_type))
          {
             disableBmps = TRUE;
             disableImps = TRUE;
          }

          if(TRUE == disableImps)
          {
             if (pHddCtx->cfg_ini->fIsImpsEnabled)
             {
                sme_DisablePowerSave (pHddCtx->hHal, ePMC_IDLE_MODE_POWER_SAVE);
             }
          }

          if(TRUE == disableBmps)
          {
             if(pHddCtx->cfg_ini->fIsBmpsEnabled)
             {
                 halStatus = sme_DisablePowerSave(pHddCtx->hHal, ePMC_BEACON_MODE_POWER_SAVE);

                 if(eHAL_STATUS_SUCCESS != halStatus)
                 {
                    status = VOS_STATUS_E_FAILURE;
                    hddLog(VOS_TRACE_LEVEL_ERROR,"%s: Fail to Disable Power Save\n", __func__);
                    VOS_ASSERT(0);
                    return status;
                 }
              }

              if(pHddCtx->cfg_ini->fIsAutoBmpsTimerEnabled)
              {
                 halStatus = sme_StopAutoBmpsTimer(pHddCtx->hHal);

                 if(eHAL_STATUS_SUCCESS != halStatus)
                 {
                    status = VOS_STATUS_E_FAILURE;
                    hddLog(VOS_TRACE_LEVEL_ERROR,"%s: Fail to Stop Auto Bmps Timer\n", __func__);
                    VOS_ASSERT(0);
                    return status;
                 }
              }
          }

          if((TRUE == disableBmps) ||
              (TRUE == disableImps))
          {
              /*                                       */
              INIT_COMPLETION(pHddCtx->full_pwr_comp_var);
              halStatus = sme_RequestFullPower(pHddCtx->hHal, hdd_full_pwr_cbk,
                                   pHddCtx, eSME_FULL_PWR_NEEDED_BY_HDD);

              if(halStatus != eHAL_STATUS_SUCCESS)
              {
                 if(halStatus == eHAL_STATUS_PMC_PENDING)
                 {
                    //                                                         
                    wait_for_completion_interruptible_timeout(
                         &pHddCtx->full_pwr_comp_var, msecs_to_jiffies(1000));
                 }
                 else
                 {
                    status = VOS_STATUS_E_FAILURE;
                    hddLog(VOS_TRACE_LEVEL_ERROR,"%s: Request for Full Power failed\n", __func__);
                    VOS_ASSERT(0);
                    return status;
                 }
              }

              status = VOS_STATUS_SUCCESS;
          }

          break;
   }
   return status;
}

hdd_adapter_t* hdd_open_adapter( hdd_context_t *pHddCtx, tANI_U8 session_type,
                                 const char *iface_name, tSirMacAddr macAddr,
                                 tANI_U8 rtnl_held )
{
   hdd_adapter_t *pAdapter = NULL;
   hdd_adapter_list_node_t *pHddAdapterNode = NULL;
   VOS_STATUS status = VOS_STATUS_E_FAILURE;
   VOS_STATUS exitbmpsStatus;

   hddLog(VOS_TRACE_LEVEL_INFO_HIGH, "%s iface =%s type = %d\n",__func__,iface_name,session_type);

   //                                  
   exitbmpsStatus = hdd_disable_bmps_imps(pHddCtx, session_type);

   if(VOS_STATUS_E_FAILURE == exitbmpsStatus)
   {
      //                 
      VOS_ASSERT(0);
      return NULL;
   }

   switch(session_type)
   {
      case WLAN_HDD_INFRA_STATION:
      case WLAN_HDD_P2P_CLIENT:
      case WLAN_HDD_P2P_DEVICE:
      {
         pAdapter = hdd_alloc_station_adapter( pHddCtx, macAddr, iface_name );

         if( NULL == pAdapter )
            return NULL;

#ifdef FEATURE_WLAN_TDLS
         /*                                                                   
                                                                          
          */
         if (mutex_lock_interruptible(&pHddCtx->tdls_lock))
         {
             VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR,
                       "%s: unable to lock list", __func__);
             return NULL;
         }
#endif

         pAdapter->wdev.iftype = (session_type == WLAN_HDD_P2P_CLIENT) ?
                                  NL80211_IFTYPE_P2P_CLIENT:
                                  NL80211_IFTYPE_STATION;

         pAdapter->device_mode = session_type;
#ifdef FEATURE_WLAN_TDLS
         mutex_unlock(&pHddCtx->tdls_lock);
#endif

         status = hdd_init_station_mode( pAdapter );
         if( VOS_STATUS_SUCCESS != status )
            goto err_free_netdev;

         status = hdd_register_interface( pAdapter, rtnl_held );
         if( VOS_STATUS_SUCCESS != status )
         {
            hdd_deinit_adapter(pHddCtx, pAdapter);
            goto err_free_netdev;
         }
         //                            
         netif_tx_disable(pAdapter->dev);
         //                           
         netif_carrier_off(pAdapter->dev);

         break;
      }

      case WLAN_HDD_P2P_GO:
      case WLAN_HDD_SOFTAP:
      {
         pAdapter = hdd_wlan_create_ap_dev( pHddCtx, macAddr, (tANI_U8 *)iface_name );
         if( NULL == pAdapter )
            return NULL;

         pAdapter->wdev.iftype = (session_type == WLAN_HDD_SOFTAP) ?
                                  NL80211_IFTYPE_AP:
                                  NL80211_IFTYPE_P2P_GO;
         pAdapter->device_mode = session_type;

         status = hdd_init_ap_mode(pAdapter);
         if( VOS_STATUS_SUCCESS != status )
            goto err_free_netdev;

         status = hdd_register_hostapd( pAdapter, rtnl_held );
         if( VOS_STATUS_SUCCESS != status )
         {
            hdd_deinit_adapter(pHddCtx, pAdapter);
            goto err_free_netdev;
         }

         netif_tx_disable(pAdapter->dev);
         netif_carrier_off(pAdapter->dev);

         hdd_set_conparam( 1 );
         break;
      }
      case WLAN_HDD_MONITOR:
      {
         pAdapter = hdd_alloc_station_adapter( pHddCtx, macAddr, iface_name );
         if( NULL == pAdapter )
            return NULL;

         pAdapter->wdev.iftype = NL80211_IFTYPE_MONITOR; 
         pAdapter->device_mode = session_type;
         status = hdd_register_interface( pAdapter, rtnl_held );
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,29)
         pAdapter->dev->netdev_ops = &wlan_mon_drv_ops;
#else
         pAdapter->dev->open = hdd_mon_open;
         pAdapter->dev->hard_start_xmit = hdd_mon_hard_start_xmit;
#endif
         hdd_init_tx_rx( pAdapter );
         set_bit(INIT_TX_RX_SUCCESS, &pAdapter->event_flags);
         //                                                                    
         pAdapter->sessionCtx.monitor.pAdapterForTx = 
                           hdd_get_adapter(pAdapter->pHddCtx, WLAN_HDD_SOFTAP);
         if (NULL == pAdapter->sessionCtx.monitor.pAdapterForTx)
         {
            pAdapter->sessionCtx.monitor.pAdapterForTx = 
                           hdd_get_adapter(pAdapter->pHddCtx, WLAN_HDD_P2P_GO);
         }
         /*                                                               
                               */
         if (NULL == pAdapter->sessionCtx.monitor.pAdapterForTx) {
             hddLog(VOS_TRACE_LEVEL_ERROR,"%s:Failed:hdd_get_adapter",__func__);
             return NULL;
         }

         INIT_WORK(&pAdapter->sessionCtx.monitor.pAdapterForTx->monTxWorkQueue,
                   hdd_mon_tx_work_queue);
      }
         break;
      case WLAN_HDD_FTM:
      {
         pAdapter = hdd_alloc_station_adapter( pHddCtx, macAddr, iface_name );

         if( NULL == pAdapter )
            return NULL;
         /*                                                                          
                                                      */
         pAdapter->wdev.iftype = NL80211_IFTYPE_STATION;
         pAdapter->device_mode = session_type;
         status = hdd_register_interface( pAdapter, rtnl_held );

         hdd_init_tx_rx( pAdapter );

         //                            
         netif_tx_disable(pAdapter->dev);
         netif_carrier_off(pAdapter->dev);
      }
         break;
      default:
      {
         VOS_ASSERT(0);
         return NULL;
      }
   }


   if( VOS_STATUS_SUCCESS == status )
   {
      //                                  
      pHddAdapterNode = vos_mem_malloc( sizeof( hdd_adapter_list_node_t ) );
      if( NULL == pHddAdapterNode )
      {
         status = VOS_STATUS_E_NOMEM;
      }
      else
      {
         pHddAdapterNode->pAdapter = pAdapter;
         status = hdd_add_adapter_back ( pHddCtx, 
                                         pHddAdapterNode );
      }
   }

   if( VOS_STATUS_SUCCESS != status )
   {
      if( NULL != pAdapter )
      {
         hdd_cleanup_adapter( pHddCtx, pAdapter, rtnl_held );
         pAdapter = NULL;   
      }
      if( NULL != pHddAdapterNode )
      {
         vos_mem_free( pHddAdapterNode );
      }

      goto resume_bmps;
   }

   if(VOS_STATUS_SUCCESS == status)
   {
      wlan_hdd_set_concurrency_mode(pHddCtx, session_type);

      //                           
      if(!hdd_init_wowl(pAdapter))
      {
          hddLog(VOS_TRACE_LEVEL_FATAL,"%s: hdd_init_wowl failed",__func__);
          goto err_free_netdev;
      }
   }
   return pAdapter;

err_free_netdev:
   free_netdev(pAdapter->dev);
   wlan_hdd_release_intf_addr( pHddCtx,
                               pAdapter->macAddressCurrent.bytes );

resume_bmps:
   //                          
   if(VOS_STATUS_SUCCESS == exitbmpsStatus)
   {
       if (pHddCtx->hdd_wlan_suspended)
       {
           hdd_set_pwrparams(pHddCtx);
       }
       hdd_enable_bmps_imps(pHddCtx);
   }
   return NULL;
}

VOS_STATUS hdd_close_adapter( hdd_context_t *pHddCtx, hdd_adapter_t *pAdapter,
                              tANI_U8 rtnl_held )
{
   hdd_adapter_list_node_t *pAdapterNode, *pCurrent, *pNext;
   VOS_STATUS status;

   status = hdd_get_front_adapter ( pHddCtx, &pCurrent );
   if( VOS_STATUS_SUCCESS != status )
      return status;

   while ( pCurrent->pAdapter != pAdapter )
   {
      status = hdd_get_next_adapter ( pHddCtx, pCurrent, &pNext );
      if( VOS_STATUS_SUCCESS != status )
         break;

      pCurrent = pNext;
   }
   pAdapterNode = pCurrent;
   if( VOS_STATUS_SUCCESS == status )
   {
      wlan_hdd_clear_concurrency_mode(pHddCtx, pAdapter->device_mode);
      hdd_cleanup_adapter( pHddCtx, pAdapterNode->pAdapter, rtnl_held );

#ifdef FEATURE_WLAN_TDLS

      /*                                                                   
                                                                       
       */
       if (mutex_lock_interruptible(&pHddCtx->tdls_lock))
       {
           VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR,
                     "%s: unable to lock list", __func__);
           return VOS_STATUS_E_FAILURE;
       }
#endif

      hdd_remove_adapter( pHddCtx, pAdapterNode );
      vos_mem_free( pAdapterNode );
      pAdapterNode = NULL;

#ifdef FEATURE_WLAN_TDLS
       mutex_unlock(&pHddCtx->tdls_lock);
#endif


      /*                                                                */
      if ((!vos_concurrent_sessions_running()) && 
           ((pHddCtx->no_of_sessions[VOS_STA_MODE] >= 1) || 
           (pHddCtx->no_of_sessions[VOS_P2P_CLIENT_MODE] >= 1)))
      {
          if (pHddCtx->hdd_wlan_suspended)
          {
              hdd_set_pwrparams(pHddCtx);
          }
          hdd_enable_bmps_imps(pHddCtx);
      }

      return VOS_STATUS_SUCCESS;
   }

   return VOS_STATUS_E_FAILURE;
}

VOS_STATUS hdd_close_all_adapters( hdd_context_t *pHddCtx )
{
   hdd_adapter_list_node_t *pHddAdapterNode;
   VOS_STATUS status;

   ENTER();

   do
   {
      status = hdd_remove_front_adapter( pHddCtx, &pHddAdapterNode );
      if( pHddAdapterNode && VOS_STATUS_SUCCESS == status )
      {
         hdd_cleanup_adapter( pHddCtx, pHddAdapterNode->pAdapter, FALSE );
         vos_mem_free( pHddAdapterNode );
      }
   }while( NULL != pHddAdapterNode && VOS_STATUS_E_EMPTY != status );
   
   EXIT();

   return VOS_STATUS_SUCCESS;
}

void wlan_hdd_reset_prob_rspies(hdd_adapter_t* pHostapdAdapter)
{
    v_U8_t addIE[1] = {0};

    if ( eHAL_STATUS_FAILURE == ccmCfgSetStr((WLAN_HDD_GET_CTX(pHostapdAdapter))->hHal,
                            WNI_CFG_PROBE_RSP_ADDNIE_DATA1,(tANI_U8*)addIE, 0, NULL,
                            eANI_BOOLEAN_FALSE) )
    {
        hddLog(LOGE,
           "Could not pass on WNI_CFG_PROBE_RSP_ADDNIE_DATA1 to CCM\n");
    }

    if ( eHAL_STATUS_FAILURE == ccmCfgSetStr((WLAN_HDD_GET_CTX(pHostapdAdapter))->hHal,
                            WNI_CFG_PROBE_RSP_ADDNIE_DATA2, (tANI_U8*)addIE, 0, NULL,
                            eANI_BOOLEAN_FALSE) )
    {
        hddLog(LOGE,
           "Could not pass on WNI_CFG_PROBE_RSP_ADDNIE_DATA2 to CCM\n");
    }

    if ( eHAL_STATUS_FAILURE == ccmCfgSetStr((WLAN_HDD_GET_CTX(pHostapdAdapter))->hHal,
                            WNI_CFG_PROBE_RSP_ADDNIE_DATA3, (tANI_U8*)addIE, 0, NULL,
                            eANI_BOOLEAN_FALSE) )
    {
        hddLog(LOGE,
           "Could not pass on WNI_CFG_PROBE_RSP_ADDNIE_DATA3 to CCM\n");
    }
}

VOS_STATUS hdd_stop_adapter( hdd_context_t *pHddCtx, hdd_adapter_t *pAdapter )
{
   eHalStatus halStatus = eHAL_STATUS_SUCCESS;
   hdd_wext_state_t *pWextState = WLAN_HDD_GET_WEXT_STATE_PTR(pAdapter);
   union iwreq_data wrqu;

   ENTER();

   switch(pAdapter->device_mode)
   {
      case WLAN_HDD_INFRA_STATION:
      case WLAN_HDD_P2P_CLIENT:
      case WLAN_HDD_P2P_DEVICE:
         if( hdd_connIsConnected( WLAN_HDD_GET_STATION_CTX_PTR( pAdapter )) )
         {
            if (pWextState->roamProfile.BSSType == eCSR_BSS_TYPE_START_IBSS)
                halStatus = sme_RoamDisconnect(pHddCtx->hHal,
                                             pAdapter->sessionId,
                                             eCSR_DISCONNECT_REASON_IBSS_LEAVE);
            else
                halStatus = sme_RoamDisconnect(pHddCtx->hHal,
                                            pAdapter->sessionId, 
                                            eCSR_DISCONNECT_REASON_UNSPECIFIED);
            //                                                             
            if(halStatus == eHAL_STATUS_SUCCESS)
            {
               wait_for_completion_interruptible_timeout(&pAdapter->disconnect_comp_var,
               msecs_to_jiffies(WLAN_WAIT_TIME_DISCONNECT));
            }
            memset(&wrqu, '\0', sizeof(wrqu));
            wrqu.ap_addr.sa_family = ARPHRD_ETHER;
            memset(wrqu.ap_addr.sa_data,'\0',ETH_ALEN);
            wireless_send_event(pAdapter->dev, SIOCGIWAP, &wrqu, NULL);
         }
         else
         {
            hdd_abort_mac_scan(pHddCtx);
         }

         if (test_bit(SME_SESSION_OPENED, &pAdapter->event_flags)) 
         {
            INIT_COMPLETION(pAdapter->session_close_comp_var);
            if (eHAL_STATUS_SUCCESS ==
                sme_CloseSession(pHddCtx->hHal, pAdapter->sessionId, 
                                 hdd_smeCloseSessionCallback, pAdapter))
            {
               //                                                          
               wait_for_completion_timeout(
                          &pAdapter->session_close_comp_var, 
                          msecs_to_jiffies(WLAN_WAIT_TIME_SESSIONOPENCLOSE));
            }
         }

         break;

      case WLAN_HDD_SOFTAP:
      case WLAN_HDD_P2P_GO:
         //                                   
         mutex_lock(&pHddCtx->sap_lock);
         if (test_bit(SOFTAP_BSS_STARTED, &pAdapter->event_flags)) 
         {
            VOS_STATUS status;
            hdd_context_t *pHddCtx = WLAN_HDD_GET_CTX(pAdapter);

            //         
            status = WLANSAP_StopBss(pHddCtx->pvosContext);
            if (VOS_IS_STATUS_SUCCESS(status))
            {
               hdd_hostapd_state_t *pHostapdState = 
                  WLAN_HDD_GET_HOSTAP_STATE_PTR(pAdapter);

               status = vos_wait_single_event(&pHostapdState->vosEvent, 10000);
   
               if (!VOS_IS_STATUS_SUCCESS(status))
               {
                  hddLog(LOGE, "%s: failure waiting for WLANSAP_StopBss",
                         __func__);
               }
            }
            else
            {
               hddLog(LOGE, "%s: failure in WLANSAP_StopBss", __func__);
            }
            clear_bit(SOFTAP_BSS_STARTED, &pAdapter->event_flags);

            if (eHAL_STATUS_FAILURE ==
                ccmCfgSetInt(pHddCtx->hHal, WNI_CFG_PROBE_RSP_BCN_ADDNIE_FLAG,
                             0, NULL, eANI_BOOLEAN_FALSE))
            {
               hddLog(LOGE,
                      "%s: Failed to set WNI_CFG_PROBE_RSP_BCN_ADDNIE_FLAG",
                      __func__);
            }

            if ( eHAL_STATUS_FAILURE == ccmCfgSetInt((WLAN_HDD_GET_CTX(pAdapter))->hHal,
                     WNI_CFG_ASSOC_RSP_ADDNIE_FLAG, 0, NULL,
                     eANI_BOOLEAN_FALSE) )
            {
               hddLog(LOGE,
                     "Could not pass on WNI_CFG_ASSOC_RSP_ADDNIE_FLAG to CCM");
            }

            //                              
            wlan_hdd_reset_prob_rspies(pAdapter);
            kfree(pAdapter->sessionCtx.ap.beacon);
            pAdapter->sessionCtx.ap.beacon = NULL;
         }
         mutex_unlock(&pHddCtx->sap_lock);
         break;

      case WLAN_HDD_MONITOR:
#ifdef WLAN_OPEN_SOURCE
         cancel_work_sync(&pAdapter->sessionCtx.monitor.pAdapterForTx->monTxWorkQueue);
#endif
         break;

      default:
         break;
   }

   EXIT();
   return VOS_STATUS_SUCCESS;
}

VOS_STATUS hdd_stop_all_adapters( hdd_context_t *pHddCtx )
{
   hdd_adapter_list_node_t *pAdapterNode = NULL, *pNext = NULL;
   VOS_STATUS status;
   hdd_adapter_t      *pAdapter;

   ENTER();

   status = hdd_get_front_adapter ( pHddCtx, &pAdapterNode );

   while ( NULL != pAdapterNode && VOS_STATUS_SUCCESS == status )
   {
      pAdapter = pAdapterNode->pAdapter;
      netif_tx_disable(pAdapter->dev);
      netif_carrier_off(pAdapter->dev);

      hdd_stop_adapter( pHddCtx, pAdapter );

      status = hdd_get_next_adapter ( pHddCtx, pAdapterNode, &pNext );
      pAdapterNode = pNext;
   }

   EXIT();

   return VOS_STATUS_SUCCESS;
}

VOS_STATUS hdd_reset_all_adapters( hdd_context_t *pHddCtx )
{
   hdd_adapter_list_node_t *pAdapterNode = NULL, *pNext = NULL;
   VOS_STATUS status;
   hdd_adapter_t *pAdapter;

   ENTER();

   status =  hdd_get_front_adapter ( pHddCtx, &pAdapterNode );

   while ( NULL != pAdapterNode && VOS_STATUS_SUCCESS == status )
   {
      pAdapter = pAdapterNode->pAdapter;
      netif_tx_disable(pAdapter->dev);
      netif_carrier_off(pAdapter->dev);

      pAdapter->sessionCtx.station.hdd_ReassocScenario = VOS_FALSE;

      hdd_deinit_tx_rx(pAdapter);
      if (test_bit(WMM_INIT_DONE, &pAdapter->event_flags))
      {
          hdd_wmm_adapter_close( pAdapter );
          clear_bit(WMM_INIT_DONE, &pAdapter->event_flags);
      }

      status = hdd_get_next_adapter ( pHddCtx, pAdapterNode, &pNext );
      pAdapterNode = pNext;
   }

   EXIT();

   return VOS_STATUS_SUCCESS;
}

VOS_STATUS hdd_start_all_adapters( hdd_context_t *pHddCtx )
{
   hdd_adapter_list_node_t *pAdapterNode = NULL, *pNext = NULL;
   VOS_STATUS status;
   hdd_adapter_t      *pAdapter;
   eConnectionState  connState;

   ENTER();

   status = hdd_get_front_adapter ( pHddCtx, &pAdapterNode );

   while ( NULL != pAdapterNode && VOS_STATUS_SUCCESS == status )
   {
      pAdapter = pAdapterNode->pAdapter;

      switch(pAdapter->device_mode)
      {
         case WLAN_HDD_INFRA_STATION:
         case WLAN_HDD_P2P_CLIENT:
         case WLAN_HDD_P2P_DEVICE:

            connState = (WLAN_HDD_GET_STATION_CTX_PTR(pAdapter))->conn_info.connState;

            hdd_init_station_mode(pAdapter);
            /*                                                 */
            pAdapter->isLinkUpSvcNeeded = FALSE; 
            pHddCtx->scan_info.mScanPending = FALSE;
            pHddCtx->scan_info.waitScanResult = FALSE;

            //                        
            hdd_wlan_initial_scan(pAdapter);

            //                                                                
            if (eConnectionState_Associated == connState ||
                eConnectionState_IbssConnected == connState )
            {
               union iwreq_data wrqu;
               memset(&wrqu, '\0', sizeof(wrqu));
               wrqu.ap_addr.sa_family = ARPHRD_ETHER;
               memset(wrqu.ap_addr.sa_data,'\0',ETH_ALEN);
               wireless_send_event(pAdapter->dev, SIOCGIWAP, &wrqu, NULL);
               pAdapter->sessionCtx.station.hdd_ReassocScenario = VOS_FALSE;

               /*                                        */
               cfg80211_disconnected(pAdapter->dev, WLAN_REASON_UNSPECIFIED,
                                     NULL, 0, GFP_KERNEL); 
            }
            else if (eConnectionState_Connecting == connState)
            {
              /*
                                                                         
                                      
               */
               cfg80211_connect_result(pAdapter->dev, NULL,
                                       NULL, 0, NULL, 0,
                                       WLAN_STATUS_ASSOC_DENIED_UNSPEC,
                                       GFP_KERNEL);
            }
            break;

         case WLAN_HDD_SOFTAP:
            /*                       */
            break;

         case WLAN_HDD_P2P_GO:
            hddLog(VOS_TRACE_LEVEL_ERROR, "%s [SSR] send stop ap to supplicant",
                                                       __func__);
            cfg80211_ap_stopped(pAdapter->dev, GFP_KERNEL);
            break;

         case WLAN_HDD_MONITOR:
            /*                         */
            break;
         default:
            break;
      }

      status = hdd_get_next_adapter ( pHddCtx, pAdapterNode, &pNext );
      pAdapterNode = pNext;
   }

   EXIT();

   return VOS_STATUS_SUCCESS;
}

VOS_STATUS hdd_reconnect_all_adapters( hdd_context_t *pHddCtx )
{
   hdd_adapter_list_node_t *pAdapterNode = NULL, *pNext = NULL;
   hdd_adapter_t *pAdapter;
   VOS_STATUS status;
   v_U32_t roamId;

   ENTER();

   status = hdd_get_front_adapter ( pHddCtx, &pAdapterNode );

   while ( NULL != pAdapterNode && VOS_STATUS_SUCCESS == status )
   {
      pAdapter = pAdapterNode->pAdapter;

      if( (WLAN_HDD_INFRA_STATION == pAdapter->device_mode) ||
             (WLAN_HDD_P2P_CLIENT == pAdapter->device_mode) )
      {
         hdd_station_ctx_t *pHddStaCtx = WLAN_HDD_GET_STATION_CTX_PTR(pAdapter);
         hdd_wext_state_t *pWextState = WLAN_HDD_GET_WEXT_STATE_PTR(pAdapter);

         pHddStaCtx->conn_info.connState = eConnectionState_NotConnected;
         init_completion(&pAdapter->disconnect_comp_var);
         sme_RoamDisconnect(pHddCtx->hHal, pAdapter->sessionId,
                             eCSR_DISCONNECT_REASON_UNSPECIFIED);

         wait_for_completion_interruptible_timeout(
                                &pAdapter->disconnect_comp_var,
                                msecs_to_jiffies(WLAN_WAIT_TIME_DISCONNECT));

         pWextState->roamProfile.csrPersona = pAdapter->device_mode; 
         pHddCtx->isAmpAllowed = VOS_FALSE;
         sme_RoamConnect(pHddCtx->hHal,
                         pAdapter->sessionId, &(pWextState->roamProfile),
                         &roamId); 
      }

      status = hdd_get_next_adapter ( pHddCtx, pAdapterNode, &pNext );
      pAdapterNode = pNext;
   }

   EXIT();

   return VOS_STATUS_SUCCESS;
}

void hdd_dump_concurrency_info(hdd_context_t *pHddCtx)
{
   hdd_adapter_list_node_t *pAdapterNode = NULL, *pNext = NULL;
   VOS_STATUS status;
   hdd_adapter_t *pAdapter;
   hdd_station_ctx_t *pHddStaCtx;
   hdd_ap_ctx_t *pHddApCtx;
   hdd_hostapd_state_t * pHostapdState;
   tCsrBssid staBssid = { 0 }, p2pBssid = { 0 }, apBssid = { 0 };
   v_U8_t staChannel = 0, p2pChannel = 0, apChannel = 0;
   const char *p2pMode = "DEV";
   const char *ccMode = "Standalone";
   int n;

   status =  hdd_get_front_adapter ( pHddCtx, &pAdapterNode );
   while ( NULL != pAdapterNode && VOS_STATUS_SUCCESS == status )
   {
      pAdapter = pAdapterNode->pAdapter;
      switch (pAdapter->device_mode) {
      case WLAN_HDD_INFRA_STATION:
          pHddStaCtx = WLAN_HDD_GET_STATION_CTX_PTR(pAdapter);
          if (eConnectionState_Associated == pHddStaCtx->conn_info.connState) {
              staChannel = pHddStaCtx->conn_info.operationChannel;
              memcpy(staBssid, pHddStaCtx->conn_info.bssId, sizeof(staBssid));
          }
          break;
      case WLAN_HDD_P2P_CLIENT:
          pHddStaCtx = WLAN_HDD_GET_STATION_CTX_PTR(pAdapter);
          if (eConnectionState_Associated == pHddStaCtx->conn_info.connState) {
              p2pChannel = pHddStaCtx->conn_info.operationChannel;
              memcpy(p2pBssid, pHddStaCtx->conn_info.bssId, sizeof(p2pBssid));
              p2pMode = "CLI";
          }
          break;
      case WLAN_HDD_P2P_GO:
          pHddApCtx = WLAN_HDD_GET_AP_CTX_PTR(pAdapter);
          pHostapdState = WLAN_HDD_GET_HOSTAP_STATE_PTR(pAdapter);
          if (pHostapdState->bssState == BSS_START && pHostapdState->vosStatus==VOS_STATUS_SUCCESS) {
              p2pChannel = pHddApCtx->operatingChannel;
              memcpy(p2pBssid, pAdapter->macAddressCurrent.bytes, sizeof(p2pBssid));
          }
          p2pMode = "GO";
          break;
      case WLAN_HDD_SOFTAP:
          pHddApCtx = WLAN_HDD_GET_AP_CTX_PTR(pAdapter);
          pHostapdState = WLAN_HDD_GET_HOSTAP_STATE_PTR(pAdapter);
          if (pHostapdState->bssState == BSS_START && pHostapdState->vosStatus==VOS_STATUS_SUCCESS) {
              apChannel = pHddApCtx->operatingChannel;
              memcpy(apBssid, pAdapter->macAddressCurrent.bytes, sizeof(apBssid));
          }
          break;
      default:
          break;
      }
      status = hdd_get_next_adapter ( pHddCtx, pAdapterNode, &pNext );
      pAdapterNode = pNext;
   }
   if (staChannel > 0 && (apChannel > 0 || p2pChannel > 0)) {
       ccMode = (p2pChannel==staChannel||apChannel==staChannel) ? "SCC" : "MCC";
   }
   n = pr_info("wlan(%d) " MAC_ADDRESS_STR " %s",
                staChannel, MAC_ADDR_ARRAY(staBssid), ccMode);
   if (p2pChannel > 0) {
       n +=  pr_info("p2p-%s(%d) " MAC_ADDRESS_STR,
                     p2pMode, p2pChannel, MAC_ADDR_ARRAY(p2pBssid));
   }
   if (apChannel > 0) {
       n += pr_info("AP(%d) " MAC_ADDRESS_STR,
                     apChannel, MAC_ADDR_ARRAY(apBssid));
   }

   if (p2pChannel > 0 && apChannel > 0) {
        hddLog(VOS_TRACE_LEVEL_ERROR, "Error concurrent SAP %d and P2P %d which is not support", apChannel, p2pChannel);
   }
}

bool hdd_is_ssr_required( void)
{
    return (isSsrRequired == HDD_SSR_REQUIRED);
}

/*                                             */
void hdd_set_ssr_required( e_hdd_ssr_required value)
{
    if (HDD_SSR_DISABLED == isSsrRequired)
        return;

    isSsrRequired = value;
}

VOS_STATUS hdd_get_front_adapter( hdd_context_t *pHddCtx,
                                  hdd_adapter_list_node_t** ppAdapterNode)
{
    VOS_STATUS status;
    spin_lock(&pHddCtx->hddAdapters.lock);
    status =  hdd_list_peek_front ( &pHddCtx->hddAdapters,
                   (hdd_list_node_t**) ppAdapterNode );
    spin_unlock(&pHddCtx->hddAdapters.lock);
    return status;
}

VOS_STATUS hdd_get_next_adapter( hdd_context_t *pHddCtx,
                                 hdd_adapter_list_node_t* pAdapterNode,
                                 hdd_adapter_list_node_t** pNextAdapterNode)
{
    VOS_STATUS status;
    spin_lock(&pHddCtx->hddAdapters.lock);
    status = hdd_list_peek_next ( &pHddCtx->hddAdapters,
                                  (hdd_list_node_t*) pAdapterNode,
                                  (hdd_list_node_t**)pNextAdapterNode );

    spin_unlock(&pHddCtx->hddAdapters.lock);
    return status;
}

VOS_STATUS hdd_remove_adapter( hdd_context_t *pHddCtx,
                               hdd_adapter_list_node_t* pAdapterNode)
{
    VOS_STATUS status;
    spin_lock(&pHddCtx->hddAdapters.lock);
    status =  hdd_list_remove_node ( &pHddCtx->hddAdapters,
                                     &pAdapterNode->node );
    spin_unlock(&pHddCtx->hddAdapters.lock);
    return status;
}

VOS_STATUS hdd_remove_front_adapter( hdd_context_t *pHddCtx,
                                     hdd_adapter_list_node_t** ppAdapterNode)
{
    VOS_STATUS status;
    spin_lock(&pHddCtx->hddAdapters.lock);
    status =  hdd_list_remove_front( &pHddCtx->hddAdapters,
                   (hdd_list_node_t**) ppAdapterNode );
    spin_unlock(&pHddCtx->hddAdapters.lock);
    return status;
}

VOS_STATUS hdd_add_adapter_back( hdd_context_t *pHddCtx,
                                 hdd_adapter_list_node_t* pAdapterNode)
{
    VOS_STATUS status;
    spin_lock(&pHddCtx->hddAdapters.lock);
    status =  hdd_list_insert_back ( &pHddCtx->hddAdapters,
                   (hdd_list_node_t*) pAdapterNode );
    spin_unlock(&pHddCtx->hddAdapters.lock);
    return status;
}

VOS_STATUS hdd_add_adapter_front( hdd_context_t *pHddCtx,
                                  hdd_adapter_list_node_t* pAdapterNode)
{
    VOS_STATUS status;
    spin_lock(&pHddCtx->hddAdapters.lock);
    status =  hdd_list_insert_front ( &pHddCtx->hddAdapters,
                   (hdd_list_node_t*) pAdapterNode );
    spin_unlock(&pHddCtx->hddAdapters.lock);
    return status;
}

hdd_adapter_t * hdd_get_adapter_by_macaddr( hdd_context_t *pHddCtx,
                                            tSirMacAddr macAddr )
{
   hdd_adapter_list_node_t *pAdapterNode = NULL, *pNext = NULL;
   hdd_adapter_t *pAdapter;
   VOS_STATUS status;

   status = hdd_get_front_adapter ( pHddCtx, &pAdapterNode );

   while ( NULL != pAdapterNode && VOS_STATUS_SUCCESS == status )
   {
      pAdapter = pAdapterNode->pAdapter;

      if( pAdapter && vos_mem_compare( pAdapter->macAddressCurrent.bytes,
                                       macAddr, sizeof(tSirMacAddr) ) )
      {
         return pAdapter;
      }
      status = hdd_get_next_adapter ( pHddCtx, pAdapterNode, &pNext );
      pAdapterNode = pNext;
   }

   return NULL;

} 

hdd_adapter_t * hdd_get_adapter_by_name( hdd_context_t *pHddCtx, tANI_U8 *name )
{
   hdd_adapter_list_node_t *pAdapterNode = NULL, *pNext = NULL;
   hdd_adapter_t *pAdapter;
   VOS_STATUS status;

   status = hdd_get_front_adapter ( pHddCtx, &pAdapterNode );

   while ( NULL != pAdapterNode && VOS_STATUS_SUCCESS == status )
   {
      pAdapter = pAdapterNode->pAdapter;

      if( pAdapter && !strncmp( pAdapter->dev->name, (const char *)name,
          IFNAMSIZ ) )
      {
         return pAdapter;
      }
      status = hdd_get_next_adapter ( pHddCtx, pAdapterNode, &pNext );
      pAdapterNode = pNext;
   }

   return NULL;

} 

hdd_adapter_t * hdd_get_adapter( hdd_context_t *pHddCtx, device_mode_t mode )
{
   hdd_adapter_list_node_t *pAdapterNode = NULL, *pNext = NULL;
   hdd_adapter_t *pAdapter;
   VOS_STATUS status;

   status = hdd_get_front_adapter ( pHddCtx, &pAdapterNode );

   while ( NULL != pAdapterNode && VOS_STATUS_SUCCESS == status )
   {
      pAdapter = pAdapterNode->pAdapter;

      if( pAdapter && (mode == pAdapter->device_mode) )
      {
         return pAdapter;
      }
      status = hdd_get_next_adapter ( pHddCtx, pAdapterNode, &pNext );
      pAdapterNode = pNext;
   }

   return NULL;

} 

//                          
hdd_adapter_t * hdd_get_mon_adapter( hdd_context_t *pHddCtx )
{
   hdd_adapter_list_node_t *pAdapterNode = NULL, *pNext = NULL;
   hdd_adapter_t *pAdapter;
   VOS_STATUS status;

   status = hdd_get_front_adapter ( pHddCtx, &pAdapterNode );

   while ( NULL != pAdapterNode && VOS_STATUS_SUCCESS == status )
   {
      pAdapter = pAdapterNode->pAdapter;

      if( pAdapter && WLAN_HDD_MONITOR == pAdapter->device_mode )
      {
         return pAdapter;
      }

      status = hdd_get_next_adapter ( pHddCtx, pAdapterNode, &pNext );
      pAdapterNode = pNext;
   }

   return NULL;

} 

/*                                                                            
  
                                        

                                                                            
            
   
                                                 
                                                                       
                  
                                                                            */
void wlan_hdd_set_monitor_tx_adapter( hdd_context_t *pHddCtx, hdd_adapter_t *pAdapter )
{
   hdd_adapter_t *pMonAdapter;

   pMonAdapter = hdd_get_adapter( pHddCtx, WLAN_HDD_MONITOR );

   if( NULL != pMonAdapter )
   {
      pMonAdapter->sessionCtx.monitor.pAdapterForTx = pAdapter;
   }
}
/*                                                                            
  
                              

                                                                       
   
                                                 
                                                                          
                                                                            
                                                                  
                                                                                             
                                                                            */
v_U8_t hdd_get_operating_channel( hdd_context_t *pHddCtx, device_mode_t mode )
{
   hdd_adapter_list_node_t *pAdapterNode = NULL, *pNext = NULL;
   VOS_STATUS status;
   hdd_adapter_t      *pAdapter;
   v_U8_t operatingChannel = 0;

   status = hdd_get_front_adapter ( pHddCtx, &pAdapterNode );

   while ( NULL != pAdapterNode && VOS_STATUS_SUCCESS == status )
   {
      pAdapter = pAdapterNode->pAdapter;

      if( mode == pAdapter->device_mode )
      {
        switch(pAdapter->device_mode)
        {
          case WLAN_HDD_INFRA_STATION:
          case WLAN_HDD_P2P_CLIENT: 
            if( hdd_connIsConnected( WLAN_HDD_GET_STATION_CTX_PTR( pAdapter )) )
              operatingChannel = (WLAN_HDD_GET_STATION_CTX_PTR(pAdapter))->conn_info.operationChannel;
            break;
          case WLAN_HDD_SOFTAP:
          case WLAN_HDD_P2P_GO:
            /*                       */
            if(test_bit(SOFTAP_BSS_STARTED, &pAdapter->event_flags)) 
              operatingChannel = (WLAN_HDD_GET_AP_CTX_PTR(pAdapter))->operatingChannel;
            break;
          default:
            break;
        }

        break; //                                            
      }

      status = hdd_get_next_adapter ( pHddCtx, pAdapterNode, &pNext );
      pAdapterNode = pNext;
   }
   return operatingChannel;
}

#ifdef WLAN_FEATURE_PACKET_FILTERING
/*                                                                            

                                    

                                              

                                             
                                         
                         

                                                                            */
static void hdd_set_multicast_list(struct net_device *dev)
{
   hdd_adapter_t *pAdapter = WLAN_HDD_GET_PRIV_PTR(dev);
   int mc_count;
   int i = 0;
   struct netdev_hw_addr *ha;

   if (NULL == pAdapter)
   {
      hddLog(VOS_TRACE_LEVEL_ERROR,
            "%s: Adapter context is Null", __func__);
      return;
   }

   if (dev->flags & IFF_ALLMULTI)
   {
      hddLog(VOS_TRACE_LEVEL_INFO,
            "%s: allow all multicast frames", __func__);
      pAdapter->mc_addr_list.mc_cnt = 0;
   }
   else 
   {
      mc_count = netdev_mc_count(dev);
      hddLog(VOS_TRACE_LEVEL_INFO,
            "%s: mc_count = %u", __func__, mc_count);
      if (mc_count > WLAN_HDD_MAX_MC_ADDR_LIST)
      {
         hddLog(VOS_TRACE_LEVEL_INFO,
               "%s: No free filter available; allow all multicast frames", __func__);
         pAdapter->mc_addr_list.mc_cnt = 0;
         return;
      }

      pAdapter->mc_addr_list.mc_cnt = mc_count;

      netdev_for_each_mc_addr(ha, dev) {
         if (i == mc_count)
            break;
         memset(&(pAdapter->mc_addr_list.addr[i][0]), 0, ETH_ALEN);
         memcpy(&(pAdapter->mc_addr_list.addr[i][0]), ha->addr, ETH_ALEN);
         hddLog(VOS_TRACE_LEVEL_INFO, "\n%s: mlist[%d] = "MAC_ADDRESS_STR,
               __func__, i, 
               MAC_ADDR_ARRAY(pAdapter->mc_addr_list.addr[i]));
         i++;
      }
   }
   return;
}
#endif

/*                                                                            
  
                              

                                                            
                                           
   
                                             
                                                     
                                                                             
  
                                                                            */
v_U16_t hdd_select_queue(struct net_device *dev,
    struct sk_buff *skb)
{
   return hdd_wmm_select_queue(dev, skb);
}


/*                                                                            

                                  

                                          

                                                  

                                                                            */
void hdd_wlan_initial_scan(hdd_adapter_t *pAdapter)
{
   tCsrScanRequest scanReq;
   tCsrChannelInfo channelInfo;
   eHalStatus halStatus;
   unsigned long scanId;
   hdd_context_t *pHddCtx = WLAN_HDD_GET_CTX(pAdapter);

   vos_mem_zero(&scanReq, sizeof(tCsrScanRequest));
   vos_mem_set(&scanReq.bssid, sizeof(tCsrBssid), 0xff);
   scanReq.BSSType = eCSR_BSS_TYPE_ANY;

   if(sme_Is11dSupported(pHddCtx->hHal))
   {
      halStatus = sme_ScanGetBaseChannels( pHddCtx->hHal, &channelInfo );
      if ( HAL_STATUS_SUCCESS( halStatus ) )
      {
         scanReq.ChannelInfo.ChannelList = vos_mem_malloc(channelInfo.numOfChannels);
         if( !scanReq.ChannelInfo.ChannelList )
         {
            hddLog(VOS_TRACE_LEVEL_ERROR, "%s kmalloc failed", __func__);
            vos_mem_free(channelInfo.ChannelList);
            channelInfo.ChannelList = NULL;
            return;
         }
         vos_mem_copy(scanReq.ChannelInfo.ChannelList, channelInfo.ChannelList,
            channelInfo.numOfChannels);
         scanReq.ChannelInfo.numOfChannels = channelInfo.numOfChannels;
         vos_mem_free(channelInfo.ChannelList);
         channelInfo.ChannelList = NULL;
      }

      scanReq.scanType = eSIR_PASSIVE_SCAN;
      scanReq.requestType = eCSR_SCAN_REQUEST_11D_SCAN;
      scanReq.maxChnTime = pHddCtx->cfg_ini->nPassiveMaxChnTime;
      scanReq.minChnTime = pHddCtx->cfg_ini->nPassiveMinChnTime;
   }
   else
   {
      scanReq.scanType = eSIR_ACTIVE_SCAN;
      scanReq.requestType = eCSR_SCAN_REQUEST_FULL_SCAN;
      scanReq.maxChnTime = pHddCtx->cfg_ini->nActiveMaxChnTime;
      scanReq.minChnTime = pHddCtx->cfg_ini->nActiveMinChnTime;
   }

   halStatus = sme_ScanRequest(pHddCtx->hHal, pAdapter->sessionId, &scanReq, &scanId, NULL, NULL);
   if ( !HAL_STATUS_SUCCESS( halStatus ) )
   {
      hddLog(VOS_TRACE_LEVEL_ERROR, "%s: sme_ScanRequest failed status code %d",
         __func__, halStatus );
   }

   if(sme_Is11dSupported(pHddCtx->hHal))
        vos_mem_free(scanReq.ChannelInfo.ChannelList);
}

struct fullPowerContext
{
   struct completion completion;
   unsigned int magic;
};
#define POWER_CONTEXT_MAGIC  0x504F5752   //    

/*                                                                            

                                                                     

                                                                          
                       

                                               
                                      

                

                                                                            */
static void hdd_full_power_callback(void *callbackContext, eHalStatus status)
{
   struct fullPowerContext *pContext = callbackContext;

   hddLog(VOS_TRACE_LEVEL_INFO,
          "%s: context = %p, status = %d", __func__, pContext, status);

   if (NULL == callbackContext)
   {
      hddLog(VOS_TRACE_LEVEL_ERROR,
             "%s: Bad param, context [%p]",
             __func__, callbackContext);
      return;
   }

   /*                                                                     
                                                                     
                                                                    
                                                                    */

   if (POWER_CONTEXT_MAGIC != pContext->magic)
   {
      /*                                                               */
      hddLog(VOS_TRACE_LEVEL_WARN,
             "%s: Invalid context, magic [%08x]",
              __func__, pContext->magic);
      return;
   }

   /*                                                               
                                                                     
                                                                  */
   complete(&pContext->completion);
}

/*                                                                            

                                                 

                                                      

                                                

                

                                                                            */
void hdd_wlan_exit(hdd_context_t *pHddCtx)
{
   eHalStatus halStatus;
   v_CONTEXT_t pVosContext = pHddCtx->pvosContext;
   VOS_STATUS vosStatus;
   struct wiphy *wiphy = pHddCtx->wiphy;
   hdd_adapter_t* pAdapter;
   struct fullPowerContext powerContext;
   long lrc;

   ENTER();

   if (VOS_FTM_MODE != hdd_get_conparam())
   {
      //                                              
      wlan_hdd_restart_deinit(pHddCtx);
   }

   if (VOS_STA_SAP_MODE != hdd_get_conparam())
   {
      if (VOS_FTM_MODE != hdd_get_conparam())
      {
         hdd_adapter_t* pAdapter = hdd_get_adapter(pHddCtx,
                                      WLAN_HDD_INFRA_STATION);
         if (pAdapter == NULL)
            pAdapter = hdd_get_adapter(pHddCtx, WLAN_HDD_P2P_CLIENT);

         if (pAdapter != NULL)
         {
            wlan_hdd_cfg80211_pre_voss_stop(pAdapter);
            hdd_UnregisterWext(pAdapter->dev);
         }
      }
   }

   if (VOS_FTM_MODE == hdd_get_conparam())
   {
      wlan_hdd_ftm_close(pHddCtx);
      goto free_hdd_ctx;
   }
   //                            
   //                           
   //                            

   if (VOS_STA_SAP_MODE == hdd_get_conparam())
   {
      pAdapter = hdd_get_adapter(pHddCtx,
                                   WLAN_HDD_SOFTAP);
   }
   else
   {
      if (VOS_FTM_MODE != hdd_get_conparam())
      {
         pAdapter = hdd_get_adapter(pHddCtx,
                                    WLAN_HDD_INFRA_STATION);
      }
   }
   /*                                                              */
   vosStatus = hddDeregisterPmOps(pHddCtx);
   if ( !VOS_IS_STATUS_SUCCESS( vosStatus ) )
   {
      hddLog(VOS_TRACE_LEVEL_FATAL,"%s: hddDeregisterPmOps failed",__func__);
      VOS_ASSERT(0);
   }

   vosStatus = hddDevTmUnregisterNotifyCallback(pHddCtx);
   if ( !VOS_IS_STATUS_SUCCESS( vosStatus ) )
   {
      hddLog(VOS_TRACE_LEVEL_FATAL,"%s: hddDevTmUnregisterNotifyCallback failed",__func__);
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
   //                                                                    
   //                                                                    
   //                                                                      
   //                             
   hdd_abort_mac_scan( pHddCtx );

   //                              
   if ( VOS_TIMER_STATE_RUNNING ==
                        vos_timer_getCurrentState(&pHddCtx->tx_rx_trafficTmr))
   {
        vos_timer_stop(&pHddCtx->tx_rx_trafficTmr);
   }

   //                                  
   if (!VOS_IS_STATUS_SUCCESS(vos_timer_destroy(
                         &pHddCtx->tx_rx_trafficTmr)))
   {
       hddLog(VOS_TRACE_LEVEL_ERROR,
           "%s: Cannot deallocate Traffic monitor timer", __func__);
   }

   //                                                                 
   //                         
   sme_DisablePowerSave(pHddCtx->hHal, ePMC_IDLE_MODE_POWER_SAVE);
   sme_DisablePowerSave(pHddCtx->hHal, ePMC_BEACON_MODE_POWER_SAVE);
   sme_DisablePowerSave(pHddCtx->hHal, ePMC_UAPSD_MODE_POWER_SAVE);

   //                                                                        
   init_completion(&powerContext.completion);
   powerContext.magic = POWER_CONTEXT_MAGIC;

   halStatus = sme_RequestFullPower(pHddCtx->hHal, hdd_full_power_callback,
                                    &powerContext, eSME_FULL_PWR_NEEDED_BY_HDD);

   if (eHAL_STATUS_SUCCESS != halStatus)
   {
      if (eHAL_STATUS_PMC_PENDING == halStatus)
      {
         /*                                           */
         lrc = wait_for_completion_interruptible_timeout(
                                      &powerContext.completion,
                                      msecs_to_jiffies(WLAN_WAIT_TIME_POWER));
         /*                                          
                                                   */
         powerContext.magic = 0;
         if (lrc <= 0)
         {
            hddLog(VOS_TRACE_LEVEL_ERROR, "%s: %s while requesting full power",
                   __func__, (0 == lrc) ? "timeout" : "interrupt");
            /*                                                 
                                                                      
                                                                      
                                                                     
                                                                 
                                                                     
                                               */
            msleep(50);
         }
      }
      else
      {
         hddLog(VOS_TRACE_LEVEL_ERROR,
                "%s: Request for Full Power failed, status %d",
                __func__, halStatus);
         VOS_ASSERT(0);
         /*                                                  */
      }
   }

   //                                   
   unregister_netdevice_notifier(&hdd_netdev_notifier);
   
   hdd_stop_all_adapters( pHddCtx );

#ifdef WLAN_BTAMP_FEATURE
   vosStatus = WLANBAP_Stop(pVosContext);
   if (!VOS_IS_STATUS_SUCCESS(vosStatus))
   {
       VOS_TRACE( VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR,
               "%s: Failed to stop BAP",__func__);
   }
#endif //                  

   //                    
   vosStatus = vos_stop( pVosContext );
   if (!VOS_IS_STATUS_SUCCESS(vosStatus))
   {
      VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_FATAL,
         "%s: Failed to stop VOSS",__func__);
      VOS_ASSERT( VOS_IS_STATUS_SUCCESS( vosStatus ) );
   }

   //                                                                  
   vosStatus = vos_chipAssertDeepSleep( NULL, NULL, NULL );
   VOS_ASSERT( VOS_IS_STATUS_SUCCESS( vosStatus ) );

   //                                  
   vos_chipPowerDown(NULL, NULL, NULL);

   vos_chipVoteOffXOBuffer(NULL, NULL, NULL);

   //                                                        
   hdd_unregister_mcast_bcast_filter(pHddCtx);

   //                                                                       
   //                                                                        
   //                      
   vosStatus = vos_sched_close( pVosContext );
   if (!VOS_IS_STATUS_SUCCESS(vosStatus))    {
      VOS_TRACE( VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_FATAL,
         "%s: Failed to close VOSS Scheduler",__func__);
      VOS_ASSERT( VOS_IS_STATUS_SUCCESS( vosStatus ) );
   }
#ifdef WLAN_OPEN_SOURCE
#ifdef WLAN_FEATURE_HOLD_RX_WAKELOCK
   /*                       */
   wake_lock_destroy(&pHddCtx->rx_wake_lock);
#endif
   /*                       */
   wake_lock_destroy(&pHddCtx->sap_wake_lock);
#endif

#ifdef CONFIG_ENABLE_LINUX_REG
   vosStatus = vos_nv_close();
   if (!VOS_IS_STATUS_SUCCESS(vosStatus))
   {
      VOS_TRACE( VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR,
          "%s: Failed to close NV", __func__);
      VOS_ASSERT( VOS_IS_STATUS_SUCCESS( vosStatus ) );
   }
#endif

   //          
   //                                                                                                
   vos_close(pVosContext);

   //              
   if(pHddCtx->cfg_ini->fIsLogpEnabled)
      vos_watchdog_close(pVosContext);

   //                          
   send_btc_nlink_msg(WLAN_MODULE_DOWN_IND, 0);
   nl_srv_exit();

   /*                                 
                                                                                             
                          
    */
   hddLog(VOS_TRACE_LEVEL_WARN, "In module exit: Cancel the vote for XO Core ON"
                                    " when WLAN is turned OFF\n");
   if (vos_chipVoteXOCore(NULL, NULL, NULL, VOS_FALSE) != VOS_STATUS_SUCCESS)
   {
       hddLog(VOS_TRACE_LEVEL_ERROR, "Could not cancel the vote for XO Core ON." 
                                        " Not returning failure."
                                        " Power consumed will be high\n");
   }  

   hdd_close_all_adapters( pHddCtx );


   //                                                        
   kfree(pHddCtx->cfg_ini);
   pHddCtx->cfg_ini= NULL;

   /*                                             */
   if (free_riva_power_on_lock("wlan"))
   {
      hddLog(VOS_TRACE_LEVEL_ERROR, "%s: failed to free power on lock",
                                           __func__);
   }

free_hdd_ctx:
   /*                                   
                                                    */
   if (VOS_FTM_MODE != hdd_get_conparam())
   {
      wiphy_unregister(wiphy) ;
   }
   wiphy_free(wiphy) ;
   if (hdd_is_ssr_required())
   {
       /*                                                               */
       subsystem_restart("wcnss");
       msleep(5000);
   }
   hdd_set_ssr_required (VOS_FALSE);
}


/*                                                                            

                                                                         
                                                                        

                                                       

                                            

                                                                            */
static VOS_STATUS hdd_update_config_from_nv(hdd_context_t* pHddCtx)
{
   v_BOOL_t itemIsValid = VOS_FALSE;
   VOS_STATUS status;
   v_MACADDR_t macFromNV[VOS_MAX_CONCURRENCY_PERSONA];
   v_U8_t      macLoop;

   /*                                                                                */
   status = vos_nv_getValidity(VNV_FIELD_IMAGE, &itemIsValid);
   if(status != VOS_STATUS_SUCCESS)
   {
      hddLog(VOS_TRACE_LEVEL_ERROR," vos_nv_getValidity() failed\n ");
       return VOS_STATUS_E_FAILURE;
   }

   if (itemIsValid == VOS_TRUE) 
   {
        hddLog(VOS_TRACE_LEVEL_INFO_HIGH," Reading the Macaddress from NV\n ");
      status = vos_nv_readMultiMacAddress((v_U8_t *)&macFromNV[0].bytes[0],
                                          VOS_MAX_CONCURRENCY_PERSONA);
        if(status != VOS_STATUS_SUCCESS)
        {
         /*                                          
                                                       */
         hddLog(VOS_TRACE_LEVEL_ERROR," vos_nv_readMacAddress() failed\n ");
            return VOS_STATUS_E_FAILURE;
        }

      /*                                                          
                                                 */
      if(vos_is_macaddr_zero(&macFromNV[0]))
      {
         /*                                              */
         hddLog(VOS_TRACE_LEVEL_WARN, "Invalid MAC in NV file");
         return VOS_STATUS_E_INVAL;
   }

      /*                                          */
      for(macLoop = 0; macLoop < VOS_MAX_CONCURRENCY_PERSONA; macLoop++)
      {
         if(vos_is_macaddr_zero(&macFromNV[macLoop]))
         {
            printk(KERN_ERR "not valid MAC from NV for %d", macLoop);
            /*                               
                                                  */
         }
         else
         {
            vos_mem_copy((v_U8_t *)&pHddCtx->cfg_ini->intfMacAddr[macLoop].bytes[0],
                         (v_U8_t *)&macFromNV[macLoop].bytes[0],
                   VOS_MAC_ADDR_SIZE);
         }
      }
   }
   else
   {
      hddLog(VOS_TRACE_LEVEL_ERROR, "NV ITEM, MAC Not valid");
      return VOS_STATUS_E_FAILURE;
   }


   return VOS_STATUS_SUCCESS;
}

/*                                                                            

                                                                         

                                         

                

                                                                            */
VOS_STATUS hdd_post_voss_start_config(hdd_context_t* pHddCtx)
{
   eHalStatus halStatus;
   v_U32_t listenInterval;
   tANI_U32    ignoreDtim;


   //                                                              
   //                                                            
   halStatus = sme_HDDReadyInd( pHddCtx->hHal );
   if ( !HAL_STATUS_SUCCESS( halStatus ) )
   {
      hddLog(VOS_TRACE_LEVEL_ERROR,"%s: sme_HDDReadyInd() failed with status "
          "code %08d [x%08x]",__func__, halStatus, halStatus );
      return VOS_STATUS_E_FAILURE;
   }

   //                                                
   //                                                                        
   //                    
   wlan_cfgGetInt(pHddCtx->hHal, WNI_CFG_LISTEN_INTERVAL, &listenInterval);
   pHddCtx->hdd_actual_LI_value = listenInterval;
   wlan_cfgGetInt(pHddCtx->hHal, WNI_CFG_IGNORE_DTIM, &ignoreDtim);
   pHddCtx->hdd_actual_ignore_DTIM_value = ignoreDtim;


   return VOS_STATUS_SUCCESS;
}

/*                        */
void hdd_prevent_suspend(void)
{
#ifdef WLAN_OPEN_SOURCE
    wake_lock(&wlan_wake_lock);
#else
    wcnss_prevent_suspend();
#endif
}

void hdd_allow_suspend(void)
{
#ifdef WLAN_OPEN_SOURCE
    wake_unlock(&wlan_wake_lock);
#else
    wcnss_allow_suspend();
#endif
}

void hdd_allow_suspend_timeout(v_U32_t timeout)
{
#ifdef WLAN_OPEN_SOURCE
    wake_lock_timeout(&wlan_wake_lock, msecs_to_jiffies(timeout));
#else
    /*                                                   */
#endif
}

/*                                                                            

                                                                                          
                                                                                                  

                                           
                                                                    
                                                                      
                                      
                                                                                            
                                                       

                                            

                 

                                                                            */

void hdd_exchange_version_and_caps(hdd_context_t *pHddCtx)
{

   tSirVersionType versionCompiled;
   tSirVersionType versionReported;
   tSirVersionString versionString;
   tANI_U8 fwFeatCapsMsgSupported = 0;
   VOS_STATUS vstatus;

   /*                                                */
   do {

      vstatus = sme_GetWcnssWlanCompiledVersion(pHddCtx->hHal,
                                                &versionCompiled);
      if (!VOS_IS_STATUS_SUCCESS(vstatus))
      {
         hddLog(VOS_TRACE_LEVEL_FATAL,
                "%s: unable to retrieve WCNSS WLAN compiled version",
                __func__);
         break;
      }

      vstatus = sme_GetWcnssWlanReportedVersion(pHddCtx->hHal,
                                                &versionReported);
      if (!VOS_IS_STATUS_SUCCESS(vstatus))
      {
         hddLog(VOS_TRACE_LEVEL_FATAL,
                "%s: unable to retrieve WCNSS WLAN reported version",
                __func__);
         break;
      }

      if ((versionCompiled.major != versionReported.major) ||
          (versionCompiled.minor != versionReported.minor) ||
          (versionCompiled.version != versionReported.version) ||
          (versionCompiled.revision != versionReported.revision))
      {
         pr_err("%s: WCNSS WLAN Version %u.%u.%u.%u, "
                "Host expected %u.%u.%u.%u\n",
                WLAN_MODULE_NAME,
                (int)versionReported.major,
                (int)versionReported.minor,
                (int)versionReported.version,
                (int)versionReported.revision,
                (int)versionCompiled.major,
                (int)versionCompiled.minor,
                (int)versionCompiled.version,
                (int)versionCompiled.revision);
      }
      else
      {
         pr_info("%s: WCNSS WLAN version %u.%u.%u.%u\n",
                 WLAN_MODULE_NAME,
                 (int)versionReported.major,
                 (int)versionReported.minor,
                 (int)versionReported.version,
                 (int)versionReported.revision);
      }

      vstatus = sme_GetWcnssSoftwareVersion(pHddCtx->hHal,
                                            versionString,
                                            sizeof(versionString));
      if (!VOS_IS_STATUS_SUCCESS(vstatus))
      {
         hddLog(VOS_TRACE_LEVEL_FATAL,
                "%s: unable to retrieve WCNSS software version string",
                __func__);
         break;
      }

      pr_info("%s: WCNSS software version %s\n",
              WLAN_MODULE_NAME, versionString);

      vstatus = sme_GetWcnssHardwareVersion(pHddCtx->hHal,
                                            versionString,
                                            sizeof(versionString));
      if (!VOS_IS_STATUS_SUCCESS(vstatus))
      {
         hddLog(VOS_TRACE_LEVEL_FATAL,
                "%s: unable to retrieve WCNSS hardware version string",
                __func__);
         break;
      }

      pr_info("%s: WCNSS hardware version %s\n",
              WLAN_MODULE_NAME, versionString);

      /*                                                                                                   
                                                                               
                                                       
                                                      
                                        
                                        
                                         
       */
      if (((versionReported.major>0) || (versionReported.minor>1) || 
         ((versionReported.minor>=1) && (versionReported.version>=1)))
         && ((versionReported.major == 1) && (versionReported.minor >= 1)))
         fwFeatCapsMsgSupported = 1;
 
      if (fwFeatCapsMsgSupported)
      {
#ifdef WLAN_ACTIVEMODE_OFFLOAD_FEATURE
         if(!pHddCtx->cfg_ini->fEnableActiveModeOffload)
            sme_disableFeatureCapablity(WLANACTIVE_OFFLOAD);
#endif
         sme_featureCapsExchange(pHddCtx->hHal);
      }

   } while (0);

}

/*                                                                            

                                                                                

                                                

                                           

                                                                            */
static boolean hdd_is_5g_supported(hdd_context_t * pHddCtx)
{
   /*                                                        
                                 
   */
   if (WCNSS_XO_48MHZ == wcnss_wlan_iris_xo_mode())
   {
      hddLog(VOS_TRACE_LEVEL_INFO, "%s: Hardware supports 5Ghz", __func__);
      return true;
   }
   else
   {
      hddLog(VOS_TRACE_LEVEL_INFO, "%s: Hardware doesn't supports 5Ghz",
                    __func__);
      return false;
   }
}


/*                                                                            

                                               

                                                                               

                                                  

                                           

                                                                            */

int hdd_wlan_startup(struct device *dev )
{
   VOS_STATUS status;
   hdd_adapter_t *pAdapter = NULL;
   hdd_adapter_t *pP2pAdapter = NULL;
   hdd_context_t *pHddCtx = NULL;
   v_CONTEXT_t pVosContext= NULL;
#ifdef WLAN_BTAMP_FEATURE
   VOS_STATUS vStatus = VOS_STATUS_SUCCESS;
   WLANBAP_ConfigType btAmpConfig;
   hdd_config_t *pConfig;
#endif
   int ret;
   struct wiphy *wiphy;

   ENTER();
   /*
                                
    */
   wiphy = wlan_hdd_cfg80211_wiphy_alloc(sizeof(hdd_context_t)) ;

   if(wiphy == NULL)
   {
      hddLog(VOS_TRACE_LEVEL_ERROR,"%s: cfg80211 init failed", __func__);
      return -EIO;
   }

   pHddCtx = wiphy_priv(wiphy);

   //                                        
   vos_mem_zero(pHddCtx, sizeof( hdd_context_t ));

   pHddCtx->wiphy = wiphy;
   hdd_prevent_suspend();
   pHddCtx->isLoadUnloadInProgress = TRUE;

   vos_set_load_unload_in_progress(VOS_MODULE_ID_VOSS, TRUE);

   /*                                              */
   pVosContext = vos_get_global_context(VOS_MODULE_ID_SYS, NULL);

   if(pVosContext == NULL)
   {
      hddLog(VOS_TRACE_LEVEL_FATAL,"%s: Failed vos_get_global_context",__func__);
      goto err_free_hdd_context;
   }

   //                                                           
   pHddCtx->pvosContext = pVosContext;

   //                                                      
   ((VosContextType*)(pVosContext))->pHDDContext = (v_VOID_t*)pHddCtx;

   pHddCtx->parent_dev = dev;

   init_completion(&pHddCtx->full_pwr_comp_var);
   init_completion(&pHddCtx->standby_comp_var);
   init_completion(&pHddCtx->req_bmps_comp_var);
   init_completion(&pHddCtx->scan_info.scan_req_completion_event);
   init_completion(&pHddCtx->scan_info.abortscan_event_var);
   init_completion(&pHddCtx->wiphy_channel_update_event);

   spin_lock_init(&pHddCtx->schedScan_lock);

   hdd_list_init( &pHddCtx->hddAdapters, MAX_NUMBER_OF_ADAPTERS );

#ifdef FEATURE_WLAN_TDLS
   /*                                                               
                                                                
                                             
    */
   mutex_init(&pHddCtx->tdls_lock);
#endif

   pHddCtx->nEnableStrictRegulatoryForFCC = TRUE;
   //                                                             
   pHddCtx->cfg_ini = (hdd_config_t*) kmalloc(sizeof(hdd_config_t), GFP_KERNEL);
   if(pHddCtx->cfg_ini == NULL)
   {
      hddLog(VOS_TRACE_LEVEL_FATAL,"%s: Failed kmalloc hdd_config_t",__func__);
      goto err_free_hdd_context;
   }

   vos_mem_zero(pHddCtx->cfg_ini, sizeof( hdd_config_t ));

   //                                     
   status = hdd_parse_config_ini( pHddCtx );
   if ( VOS_STATUS_SUCCESS != status )
   {
      hddLog(VOS_TRACE_LEVEL_FATAL, "%s: error parsing %s",
             __func__, WLAN_INI_FILE);
      goto err_config;
   }

   if (false == hdd_is_5g_supported(pHddCtx))
   {
      //                      
      if (1 != pHddCtx->cfg_ini->nBandCapability)
      {
         hddLog(VOS_TRACE_LEVEL_INFO,
                "%s: Setting pHddCtx->cfg_ini->nBandCapability = 1", __func__);
         pHddCtx->cfg_ini->nBandCapability = 1;
      }
   }

   /*                                                                  
                                                       
    */
   pHddCtx->configuredMcastBcastFilter = pHddCtx->cfg_ini->mcastBcastFilterSetting;
   hddLog(VOS_TRACE_LEVEL_INFO, "Setting configuredMcastBcastFilter: %d",
                   pHddCtx->cfg_ini->mcastBcastFilterSetting);
   /*
                                   
    */
   if (0 < wlan_hdd_cfg80211_init(dev, wiphy, pHddCtx->cfg_ini))
   {
      hddLog(VOS_TRACE_LEVEL_FATAL, 
              "%s: wlan_hdd_cfg80211_init return failure", __func__);
      goto err_config;
   }

   //                                               
   hdd_vos_trace_enable(VOS_MODULE_ID_BAP,
                        pHddCtx->cfg_ini->vosTraceEnableBAP);
   hdd_vos_trace_enable(VOS_MODULE_ID_TL,
                        pHddCtx->cfg_ini->vosTraceEnableTL);
   hdd_vos_trace_enable(VOS_MODULE_ID_WDI,
                        pHddCtx->cfg_ini->vosTraceEnableWDI);
   hdd_vos_trace_enable(VOS_MODULE_ID_HDD,
                        pHddCtx->cfg_ini->vosTraceEnableHDD);
   hdd_vos_trace_enable(VOS_MODULE_ID_SME,
                        pHddCtx->cfg_ini->vosTraceEnableSME);
   hdd_vos_trace_enable(VOS_MODULE_ID_PE,
                        pHddCtx->cfg_ini->vosTraceEnablePE);
   hdd_vos_trace_enable(VOS_MODULE_ID_PMC,
                         pHddCtx->cfg_ini->vosTraceEnablePMC);
   hdd_vos_trace_enable(VOS_MODULE_ID_WDA,
                        pHddCtx->cfg_ini->vosTraceEnableWDA);
   hdd_vos_trace_enable(VOS_MODULE_ID_SYS,
                        pHddCtx->cfg_ini->vosTraceEnableSYS);
   hdd_vos_trace_enable(VOS_MODULE_ID_VOSS,
                        pHddCtx->cfg_ini->vosTraceEnableVOSS);
   hdd_vos_trace_enable(VOS_MODULE_ID_SAP,
                        pHddCtx->cfg_ini->vosTraceEnableSAP);
   hdd_vos_trace_enable(VOS_MODULE_ID_HDD_SOFTAP,
                        pHddCtx->cfg_ini->vosTraceEnableHDDSAP);

   //                                               
   hdd_wdi_trace_enable(eWLAN_MODULE_DAL,
                        pHddCtx->cfg_ini->wdiTraceEnableDAL);
   hdd_wdi_trace_enable(eWLAN_MODULE_DAL_CTRL,
                        pHddCtx->cfg_ini->wdiTraceEnableCTL);
   hdd_wdi_trace_enable(eWLAN_MODULE_DAL_DATA,
                        pHddCtx->cfg_ini->wdiTraceEnableDAT);
   hdd_wdi_trace_enable(eWLAN_MODULE_PAL,
                        pHddCtx->cfg_ini->wdiTraceEnablePAL);

   if (VOS_FTM_MODE == hdd_get_conparam())
   {
      if ( VOS_STATUS_SUCCESS != wlan_hdd_ftm_open(pHddCtx) )
      {
          hddLog(VOS_TRACE_LEVEL_FATAL,"%s: wlan_hdd_ftm_open Failed",__func__);
          goto err_free_hdd_context;
      }
      hddLog(VOS_TRACE_LEVEL_FATAL,"%s: FTM driver loaded success fully",__func__);
      return VOS_STATUS_SUCCESS;
   }

   //                    
   if(pHddCtx->cfg_ini->fIsLogpEnabled)
   {
      status = vos_watchdog_open(pVosContext,
         &((VosContextType*)pVosContext)->vosWatchdog, sizeof(VosWatchdogContext));

      if(!VOS_IS_STATUS_SUCCESS( status ))
      {
         hddLog(VOS_TRACE_LEVEL_FATAL,"%s: vos_watchdog_open failed",__func__);
         goto err_wdclose;
      }
   }

   pHddCtx->isLogpInProgress = FALSE;
   vos_set_logp_in_progress(VOS_MODULE_ID_VOSS, FALSE);

   status = vos_chipVoteOnXOBuffer(NULL, NULL, NULL);
   if(!VOS_IS_STATUS_SUCCESS(status))
   {
      hddLog(VOS_TRACE_LEVEL_FATAL, "%s: Failed to configure 19.2 MHz Clock", __func__);
      goto err_wdclose;
   }

   status = vos_open( &pVosContext, 0);
   if ( !VOS_IS_STATUS_SUCCESS( status ))
   {
      hddLog(VOS_TRACE_LEVEL_FATAL, "%s: vos_open failed", __func__);
      goto err_clkvote;
   }

   pHddCtx->hHal = (tHalHandle)vos_get_context( VOS_MODULE_ID_SME, pVosContext );

   if ( NULL == pHddCtx->hHal )
   {
      hddLog(VOS_TRACE_LEVEL_FATAL, "%s: HAL context is null", __func__);
      goto err_vosclose;
   }

   status = vos_preStart( pHddCtx->pvosContext );
   if ( !VOS_IS_STATUS_SUCCESS( status ) )
   {
      hddLog(VOS_TRACE_LEVEL_FATAL, "%s: vos_preStart failed", __func__);
      goto err_vosclose;
   }

   /*                                                                 
                                                                 
                                                                  */
   status = hdd_set_sme_config( pHddCtx );

   if ( VOS_STATUS_SUCCESS != status )
   {
      hddLog(VOS_TRACE_LEVEL_FATAL, "%s: Failed hdd_set_sme_config", __func__);
      goto err_vosclose;
   }

   //                         
   status = hdd_wmm_init(pHddCtx);
   if (!VOS_IS_STATUS_SUCCESS(status))
   {
      hddLog(VOS_TRACE_LEVEL_FATAL, "%s: hdd_wmm_init failed", __func__);
      goto err_vosclose;
   }

   /*                                                                
                                                                   
                                                                  */

   //                             
   if (FALSE == hdd_update_config_dat(pHddCtx))
   {
      hddLog(VOS_TRACE_LEVEL_FATAL,"%s: config update failed",__func__ );
      goto err_vosclose;
   }

   //                        
   /*                                       */
   if (VOS_STATUS_SUCCESS != hdd_update_config_from_nv(pHddCtx))
   {
#ifdef WLAN_AUTOGEN_MACADDR_FEATURE
      /*                                                              
                                                                         
                                                                    
                                                       */

      static const v_MACADDR_t default_address =
         {{0x00, 0x0A, 0xF5, 0x89, 0x89, 0xFF}};
      unsigned int serialno;
      int i;

      serialno = wcnss_get_serial_number();
      if ((0 != serialno) &&
          (0 == memcmp(&default_address, &pHddCtx->cfg_ini->intfMacAddr[0],
                       sizeof(default_address))))
      {
         /*                                                       */

         /*                                                         
                                                                   
                                                                   
                                                                
                                       */
         serialno &= 0x00FFFFFF;

         /*                                           */
         serialno *= VOS_MAX_CONCURRENCY_PERSONA;

         /*                       */
         for (i = 0; i < VOS_MAX_CONCURRENCY_PERSONA; i++)
         {
            /*                                       */
            pHddCtx->cfg_ini->intfMacAddr[i] = default_address;
            /*                                */
            pHddCtx->cfg_ini->intfMacAddr[i].bytes[3] = (serialno >> 16) & 0xFF;
            pHddCtx->cfg_ini->intfMacAddr[i].bytes[4] = (serialno >> 8) & 0xFF;
            pHddCtx->cfg_ini->intfMacAddr[i].bytes[5] = serialno & 0xFF;

            serialno++;
         }

         pr_info("wlan: Invalid MAC addresses in NV, autogenerated "
                MAC_ADDRESS_STR,
                MAC_ADDR_ARRAY(pHddCtx->cfg_ini->intfMacAddr[0].bytes));
      }
      else
#endif //                            
      {
         hddLog(VOS_TRACE_LEVEL_ERROR,
                "%s: Invalid MAC address in NV, using MAC from ini file "
                MAC_ADDRESS_STR, __func__,
                MAC_ADDR_ARRAY(pHddCtx->cfg_ini->intfMacAddr[0].bytes));
      }
   }
   {
      eHalStatus halStatus;
      //                    
      //                                                                                                           
      halStatus = cfgSetStr( pHddCtx->hHal, WNI_CFG_STA_ID,
                             (v_U8_t *)&pHddCtx->cfg_ini->intfMacAddr[0],
                             sizeof( pHddCtx->cfg_ini->intfMacAddr[0]) );
   
      if (!HAL_STATUS_SUCCESS( halStatus ))
      {
         hddLog(VOS_TRACE_LEVEL_ERROR,"%s: Failed to set MAC Address. "
                "HALStatus is %08d [x%08x]",__func__, halStatus, halStatus );
         goto err_vosclose;
      }
   }

   /*                                                                      
                                                                           */
   status = vos_start( pHddCtx->pvosContext );
   if ( !VOS_IS_STATUS_SUCCESS( status ) )
   {
      hddLog(VOS_TRACE_LEVEL_FATAL,"%s: vos_start failed",__func__);
      goto err_vosclose;
   }

   /*                                                                                   */
   hdd_exchange_version_and_caps(pHddCtx);

   status = hdd_post_voss_start_config( pHddCtx );
   if ( !VOS_IS_STATUS_SUCCESS( status ) )
   {
      hddLog(VOS_TRACE_LEVEL_FATAL,"%s: hdd_post_voss_start_config failed", 
         __func__);
      goto err_vosstop;
   }
   wlan_hdd_cfg80211_update_reg_info( wiphy );

   /*                                         */
   if (0 > wlan_hdd_cfg80211_register(wiphy))
   {
       hddLog(VOS_TRACE_LEVEL_ERROR,"%s: wiphy register failed", __func__);
       goto err_vosstop;
   }

   if (VOS_STA_SAP_MODE == hdd_get_conparam())
   {
     pAdapter = hdd_open_adapter( pHddCtx, WLAN_HDD_SOFTAP, "softap.%d", 
         wlan_hdd_get_intf_addr(pHddCtx), FALSE );
   }
   else
   {
     pAdapter = hdd_open_adapter( pHddCtx, WLAN_HDD_INFRA_STATION, "wlan%d",
         wlan_hdd_get_intf_addr(pHddCtx), FALSE );
     if (pAdapter != NULL)
     {
         if ( pHddCtx->cfg_ini->isP2pDeviceAddrAdministrated )
         {
               vos_mem_copy( pHddCtx->p2pDeviceAddress.bytes,
                       pHddCtx->cfg_ini->intfMacAddr[0].bytes,
                       sizeof(tSirMacAddr));

                /*                                                                
                                                                             
                */
                pHddCtx->p2pDeviceAddress.bytes[0] |= 0x02;
         }
         else
         {
             tANI_U8* p2p_dev_addr = wlan_hdd_get_intf_addr(pHddCtx);
             if (p2p_dev_addr != NULL)
             {
                 vos_mem_copy(&pHddCtx->p2pDeviceAddress.bytes[0],
                             p2p_dev_addr, VOS_MAC_ADDR_SIZE);
             }
             else
             {
                   hddLog(VOS_TRACE_LEVEL_FATAL,
                           "%s: Failed to allocate mac_address for p2p_device",
                   __func__);
                   goto err_close_adapter;
             }
         }

         pP2pAdapter = hdd_open_adapter( pHddCtx, WLAN_HDD_P2P_DEVICE, "p2p%d",
                           &pHddCtx->p2pDeviceAddress.bytes[0], FALSE );
         if ( NULL == pP2pAdapter )
         {
             hddLog(VOS_TRACE_LEVEL_FATAL,
                "%s: Failed to do hdd_open_adapter for P2P Device Interface",
                __func__);
             goto err_close_adapter;
         }
     }
   }

   if( pAdapter == NULL )
   {
      hddLog(VOS_TRACE_LEVEL_ERROR, "%s: hdd_open_adapter failed", __func__);
      goto err_close_adapter;
   }

#ifdef WLAN_BTAMP_FEATURE
   vStatus = WLANBAP_Open(pVosContext);
   if(!VOS_IS_STATUS_SUCCESS(vStatus))
   {
     VOS_TRACE( VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR,
        "%s: Failed to open BAP",__func__);
      goto err_close_adapter;
   }

   vStatus = BSL_Init(pVosContext);
   if(!VOS_IS_STATUS_SUCCESS(vStatus))
   {
     VOS_TRACE( VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR,
        "%s: Failed to Init BSL",__func__);
     goto err_bap_close;
   }
   vStatus = WLANBAP_Start(pVosContext);
   if (!VOS_IS_STATUS_SUCCESS(vStatus))
   {
       VOS_TRACE( VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR,
               "%s: Failed to start TL",__func__);
       goto err_bap_close;
   }

   pConfig = pHddCtx->cfg_ini;
   btAmpConfig.ucPreferredChannel = pConfig->preferredChannel;
   status = WLANBAP_SetConfig(&btAmpConfig);

#endif //                  

#ifdef WLAN_FEATURE_ROAM_SCAN_OFFLOAD
   if(!(IS_ROAM_SCAN_OFFLOAD_FEATURE_ENABLE))
   {
      hddLog(VOS_TRACE_LEVEL_DEBUG,"%s: ROAM_SCAN_OFFLOAD Feature not supported",__func__);
      pHddCtx->cfg_ini->isRoamOffloadScanEnabled = 0;
      sme_UpdateRoamScanOffloadEnabled((tHalHandle)(pHddCtx->hHal),
                       pHddCtx->cfg_ini->isRoamOffloadScanEnabled);
   }
#endif
#ifdef FEATURE_WLAN_SCAN_PNO
   /*                                                  */
   sme_UpdateChannelConfig(pHddCtx->hHal); 
#endif

   /*                                                            */
   status = hddRegisterPmOps(pHddCtx);
   if ( !VOS_IS_STATUS_SUCCESS( status ) )
   {
      hddLog(VOS_TRACE_LEVEL_FATAL,"%s: hddRegisterPmOps failed",__func__);
#ifdef WLAN_BTAMP_FEATURE
      goto err_bap_stop;
#else
      goto err_close_adapter; 
#endif //                  
   }

   /*                                                           */
   status = hddDevTmRegisterNotifyCallback(pHddCtx);
   if ( !VOS_IS_STATUS_SUCCESS( status ) )
   {
      hddLog(VOS_TRACE_LEVEL_FATAL,"%s: hddDevTmRegisterNotifyCallback failed",__func__);
      goto err_unregister_pmops;
   }

   /*                                                    */
   if (req_riva_power_on_lock("wlan"))
   {
      hddLog(VOS_TRACE_LEVEL_FATAL,"%s: req riva power on lock failed",
                                     __func__);
      goto err_unregister_pmops;
   }

   //                                                            
   ret = register_netdevice_notifier(&hdd_netdev_notifier);

   if(ret < 0)
   {
      hddLog(VOS_TRACE_LEVEL_ERROR,"%s: register_netdevice_notifier failed",__func__);
      goto err_free_power_on_lock;
   }

   //                            
   if(nl_srv_init() != 0)
   {
      hddLog(VOS_TRACE_LEVEL_FATAL,"%s: nl_srv_init failed", __func__);
      goto err_reg_netdev;
   }

   //                          
   if(btc_activate_service(pHddCtx) != 0)
   {
      hddLog(VOS_TRACE_LEVEL_FATAL,"%s: btc_activate_service failed",__func__);
      goto err_nl_srv;
   }

#ifdef PTT_SOCK_SVC_ENABLE
   //                          
   if(ptt_sock_activate_svc(pHddCtx) != 0)
   {
      hddLog(VOS_TRACE_LEVEL_FATAL,"%s: ptt_sock_activate_svc failed",__func__);
      goto err_nl_srv;
   }
#endif

   hdd_register_mcast_bcast_filter(pHddCtx);
   if (VOS_STA_SAP_MODE != hdd_get_conparam())
   {
      /*                                                  
                                      
       */
      wlan_hdd_cfg80211_post_voss_start(pAdapter);
   }

   mutex_init(&pHddCtx->sap_lock);

   pHddCtx->isLoadUnloadInProgress = FALSE;

#ifdef WLAN_OPEN_SOURCE
#ifdef WLAN_FEATURE_HOLD_RX_WAKELOCK
   /*                          */
   wake_lock_init(&pHddCtx->rx_wake_lock,
           WAKE_LOCK_SUSPEND,
           "qcom_rx_wakelock");
#endif
   /*                          */
   wake_lock_init(&pHddCtx->sap_wake_lock,
           WAKE_LOCK_SUSPEND,
           "qcom_sap_wakelock");
#endif

   vos_event_init(&pHddCtx->scan_info.scan_finished_event);
   pHddCtx->scan_info.scan_pending_option = WEXT_SCAN_PENDING_GIVEUP;

   vos_set_load_unload_in_progress(VOS_MODULE_ID_VOSS, FALSE);
   hdd_allow_suspend();
   
   //                             
   wlan_hdd_restart_init(pHddCtx);

   //                                      
   if ( pHddCtx->cfg_ini->dynSplitscan)
   {
       vos_timer_init(&pHddCtx->tx_rx_trafficTmr,
                     VOS_TIMER_TYPE_SW,
                     hdd_tx_rx_pkt_cnt_stat_timer_handler,
                     (void *)pHddCtx);
   }
   goto success;

err_nl_srv:
   nl_srv_exit();

err_reg_netdev:
   unregister_netdevice_notifier(&hdd_netdev_notifier);

err_free_power_on_lock:
   free_riva_power_on_lock("wlan");

err_unregister_pmops:
   hddDevTmUnregisterNotifyCallback(pHddCtx);
   hddDeregisterPmOps(pHddCtx);

#ifdef WLAN_BTAMP_FEATURE
err_bap_stop:
  WLANBAP_Stop(pVosContext);
#endif

#ifdef WLAN_BTAMP_FEATURE
err_bap_close:
   WLANBAP_Close(pVosContext);
#endif

err_close_adapter:
   hdd_close_all_adapters( pHddCtx );
   wiphy_unregister(wiphy) ;

err_vosstop:
   vos_stop(pVosContext);

err_vosclose:    
   status = vos_sched_close( pVosContext );
   if (!VOS_IS_STATUS_SUCCESS(status))    {
      VOS_TRACE( VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_FATAL,
         "%s: Failed to close VOSS Scheduler", __func__);
      VOS_ASSERT( VOS_IS_STATUS_SUCCESS( status ) );
   }
   vos_close(pVosContext ); 

err_clkvote:
   vos_chipVoteOffXOBuffer(NULL, NULL, NULL);

err_wdclose:
   if(pHddCtx->cfg_ini->fIsLogpEnabled)
      vos_watchdog_close(pVosContext);

err_config:
   kfree(pHddCtx->cfg_ini);
   pHddCtx->cfg_ini= NULL;

err_free_hdd_context:
   hdd_allow_suspend();
   wiphy_free(wiphy) ;
   //             
   VOS_BUG(1);

   if (hdd_is_ssr_required())
   {
       /*                                                             */
       subsystem_restart("wcnss");
       msleep(5000);
   }
   hdd_set_ssr_required (VOS_FALSE);

   return -EIO;

success:
   EXIT();
   return 0;
}

/*                                                                            

                                                      

                                                                          
                                                            

                

                                               

                                                                            */
static int hdd_driver_init( void)
{
   VOS_STATUS status;
   v_CONTEXT_t pVosContext = NULL;
   struct device *dev = NULL;
   int ret_status = 0;
#ifdef HAVE_WCNSS_CAL_DOWNLOAD
   int max_retries = 0;
#endif

#ifdef WCONN_TRACE_KMSG_LOG_BUFF
   vos_wconn_trace_init();
#endif

   ENTER();

#ifdef WLAN_OPEN_SOURCE
   wake_lock_init(&wlan_wake_lock, WAKE_LOCK_SUSPEND, "wlan");
#endif

   pr_info("%s: loading driver v%s\n", WLAN_MODULE_NAME,
           QWLAN_VERSIONSTR TIMER_MANAGER_STR MEMORY_DEBUG_STR);

   //                                                        
   status = vos_chipPowerUp(NULL,NULL,NULL);
   if (!VOS_IS_STATUS_SUCCESS(status))
   {
      hddLog(VOS_TRACE_LEVEL_FATAL, "%s: Libra WLAN not Powered Up. "
          "exiting", __func__);
#ifdef WLAN_OPEN_SOURCE
      wake_lock_destroy(&wlan_wake_lock);
#endif
      return -EIO;
   }

#ifdef ANI_BUS_TYPE_PCI

   dev = wcnss_wlan_get_device();

#endif //                 

#ifdef ANI_BUS_TYPE_PLATFORM

#ifdef HAVE_WCNSS_CAL_DOWNLOAD
   /*                                      */
   while (!wcnss_device_ready() && 5 >= ++max_retries) {
       msleep(1000);
   }
   if (max_retries >= 5) {
      hddLog(VOS_TRACE_LEVEL_FATAL,"%s: WCNSS driver not ready", __func__);
#ifdef WLAN_OPEN_SOURCE
      wake_lock_destroy(&wlan_wake_lock);
#endif
      return -ENODEV;
   }
#endif

   dev = wcnss_wlan_get_device();
#endif //                      


   do {
      if (NULL == dev) {
         hddLog(VOS_TRACE_LEVEL_FATAL, "%s: WLAN device not found!!",__func__);
         ret_status = -1;
         break;
   }

#ifdef MEMORY_DEBUG
      vos_mem_init();
#endif

#ifdef TIMER_MANAGER
      vos_timer_manager_init();
#endif

      /*                                                        */
      status = vos_preOpen(&pVosContext);

   if (!VOS_IS_STATUS_SUCCESS(status))
   {
         hddLog(VOS_TRACE_LEVEL_FATAL,"%s: Failed to preOpen VOSS", __func__);
         ret_status = -1;
         break;
   }

#ifndef MODULE
      /*                                                                            
       */
      hdd_set_conparam((v_UINT_t)con_mode);
#endif

      //                            
      if (hdd_wlan_startup(dev))
      {
         hddLog(VOS_TRACE_LEVEL_FATAL,"%s: WLAN Driver Initialization failed",
                __func__);
         vos_preClose( &pVosContext );
         ret_status = -1;
         break;
      }

      /*                               
                                                                                       
                                      
       */
      hddLog(VOS_TRACE_LEVEL_INFO, "In module init: Ensure Force XO Core is OFF"
                                       " when  WLAN is turned ON so Core toggles"
                                       " unless we enter PSD");
      if (vos_chipVoteXOCore(NULL, NULL, NULL, VOS_FALSE) != VOS_STATUS_SUCCESS)
      {
          hddLog(VOS_TRACE_LEVEL_ERROR, "Could not cancel XO Core ON vote. Not returning failure."
                                            " Power consumed will be high\n");
      }
   } while (0);

   if (0 != ret_status)
   {
      //                                                                  
      status = vos_chipAssertDeepSleep( NULL, NULL, NULL );
      VOS_ASSERT( VOS_IS_STATUS_SUCCESS( status) );

      //                                  
      vos_chipPowerDown(NULL, NULL, NULL);
#ifdef TIMER_MANAGER
      vos_timer_exit();
#endif
#ifdef MEMORY_DEBUG
      vos_mem_exit();
#endif

#ifdef WLAN_OPEN_SOURCE
      wake_lock_destroy(&wlan_wake_lock);
#endif
      pr_err("%s: driver load failure\n", WLAN_MODULE_NAME);
   }
   else
   {
      //                                        
      send_btc_nlink_msg(WLAN_MODULE_UP_IND, 0);

      pr_info("%s: driver loaded\n", WLAN_MODULE_NAME);
   }

   EXIT();

   return ret_status;
}

/*                                                                            

                                          

                                                                              

                

                                               

                                                                            */
#ifdef MODULE
static int __init hdd_module_init ( void)
{
   return hdd_driver_init();
}
#else /*               */
static int __init hdd_module_init ( void)
{
   /*                                                            */
   return 0;
}
#endif /*               */


/*                                                                            

                                          

                                                                            
                                       

                

                

                                                                            */
static void hdd_driver_exit(void)
{
   hdd_context_t *pHddCtx = NULL;
   v_CONTEXT_t pVosContext = NULL;
   int retry = 0;

   pr_info("%s: unloading driver v%s\n", WLAN_MODULE_NAME, QWLAN_VERSIONSTR);

   //                          
   pVosContext = vos_get_global_context(VOS_MODULE_ID_SYS, NULL);

   if(!pVosContext)
   {
      hddLog(VOS_TRACE_LEVEL_FATAL,"%s: Global VOS context is Null", __func__);
      goto done;
   }

   //                    
   pHddCtx = (hdd_context_t *)vos_get_context(VOS_MODULE_ID_HDD, pVosContext );

   if(!pHddCtx)
   {
      hddLog(VOS_TRACE_LEVEL_FATAL,"%s: module exit called before probe",__func__);
   }
   else
   {
      while(isWDresetInProgress()) {
         VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_FATAL,
              "%s:SSR in Progress; block rmmod for 1 second!!!", __func__);
         msleep(1000);

         if (retry++ == HDD_MOD_EXIT_SSR_MAX_RETRIES) {
            VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_FATAL,
              "%s:SSR never completed, fatal error", __func__);
            VOS_BUG(0);
         }
      }


      pHddCtx->isLoadUnloadInProgress = TRUE;
      vos_set_load_unload_in_progress(VOS_MODULE_ID_VOSS, TRUE);

      //                                                  
      hdd_wlan_exit(pHddCtx);
   }

   vos_preClose( &pVosContext );

#ifdef TIMER_MANAGER
   vos_timer_exit();
#endif
#ifdef MEMORY_DEBUG
   vos_mem_exit();
#endif

#ifdef WCONN_TRACE_KMSG_LOG_BUFF
   vos_wconn_trace_exit();
#endif

done:
#ifdef WLAN_OPEN_SOURCE
   wake_lock_destroy(&wlan_wake_lock);
#endif
   pr_info("%s: driver unloaded\n", WLAN_MODULE_NAME);
}

/*                                                                            

                                          

                                                                             

                

                

                                                                            */
static void __exit hdd_module_exit(void)
{
   hdd_driver_exit();
}

#ifdef MODULE
static int fwpath_changed_handler(const char *kmessage,
                                 struct kernel_param *kp)
{
   return param_set_copystring(kmessage, kp);
}

static int con_mode_handler(const char *kmessage,
                                 struct kernel_param *kp)
{
   return param_set_int(kmessage, kp);
}
#else /*               */
/*                                                                            

                         

                                 
                                                                   
                                                                              
                                                                             
                                         

                                               

                                                                            */
static int kickstart_driver(void)
{
   int ret_status;

   if (!wlan_hdd_inited) {
      ret_status = hdd_driver_init();
      wlan_hdd_inited = ret_status ? 0 : 1;
      return ret_status;
   }

   hdd_driver_exit();

   msleep(200);

   ret_status = hdd_driver_init();
   wlan_hdd_inited = ret_status ? 0 : 1;
   return ret_status;
}

/*                                                                            

                                                    

                                         

                                               

                                                                            */
static int fwpath_changed_handler(const char *kmessage,
                                  struct kernel_param *kp)
{
   int ret;

   ret = param_set_copystring(kmessage, kp);
   if (0 == ret)
      ret = kickstart_driver();
   return ret;
}

/*                                                                            

                             

                                                                            
                                 
                                                                  

           

           

                                                                            */
static int con_mode_handler(const char *kmessage, struct kernel_param *kp)
{
   int ret;

   ret = param_set_int(kmessage, kp);
   if (0 == ret)
      ret = kickstart_driver();
   return ret;
}
#endif /*               */

/*                                                                            

                             

                                                                             

                

                         

                                                                            */
tVOS_CON_MODE hdd_get_conparam ( void )
{
#ifdef MODULE
    return (tVOS_CON_MODE)con_mode;
#else
    return (tVOS_CON_MODE)curr_con_mode;
#endif
}
void hdd_set_conparam ( v_UINT_t newParam )
{
  con_mode = newParam;
#ifndef MODULE
  curr_con_mode = con_mode;
#endif
}
/*                                                                            

                                           

                                                            

                                         

                                  

                

                                                                            */

VOS_STATUS hdd_softap_sta_deauth(hdd_adapter_t *pAdapter, v_U8_t *pDestMacAddress)
{
    v_CONTEXT_t pVosContext = (WLAN_HDD_GET_CTX(pAdapter))->pvosContext;
    VOS_STATUS vosStatus = VOS_STATUS_E_FAULT;

    ENTER();

    hddLog( LOGE, "hdd_softap_sta_deauth:(%p, false)", (WLAN_HDD_GET_CTX(pAdapter))->pvosContext);

    //                                     
    if( pDestMacAddress[0] & 0x1 )
       return vosStatus;

    vosStatus = WLANSAP_DeauthSta(pVosContext,pDestMacAddress);

    EXIT();
    return vosStatus;
}

/*                                                                            

                                             

                                                            

                                         

                                  

                

                                                                            */

void hdd_softap_sta_disassoc(hdd_adapter_t *pAdapter,v_U8_t *pDestMacAddress)
{
        v_CONTEXT_t pVosContext = (WLAN_HDD_GET_CTX(pAdapter))->pvosContext;

    ENTER();

    hddLog( LOGE, "hdd_softap_sta_disassoc:(%p, false)", (WLAN_HDD_GET_CTX(pAdapter))->pvosContext);

    //                                       
    if( pDestMacAddress[0] & 0x1 )
       return;

    WLANSAP_DisassocSta(pVosContext,pDestMacAddress);
}

void hdd_softap_tkip_mic_fail_counter_measure(hdd_adapter_t *pAdapter,v_BOOL_t enable)
{
    v_CONTEXT_t pVosContext = (WLAN_HDD_GET_CTX(pAdapter))->pvosContext;

    ENTER();

    hddLog( LOGE, "hdd_softap_tkip_mic_fail_counter_measure:(%p, false)", (WLAN_HDD_GET_CTX(pAdapter))->pvosContext);

    WLANSAP_SetCounterMeasure(pVosContext, (v_BOOL_t)enable);
}

/*                                                                            
  
                                         
  
  
                   
  
                               
  
                                                                             */
tVOS_CONCURRENCY_MODE hdd_get_concurrency_mode ( void )
{
    v_CONTEXT_t pVosContext = vos_get_global_context( VOS_MODULE_ID_HDD, NULL );
    hdd_context_t *pHddCtx;

    if (NULL != pVosContext)
    {
       pHddCtx = vos_get_context( VOS_MODULE_ID_HDD, pVosContext);
       if (NULL != pHddCtx)
       {
          return (tVOS_CONCURRENCY_MODE)pHddCtx->concurrency_mode;
       }
    }

    /*                               */
    hddLog(LOGE, "%s: Invalid context", __func__);
    return VOS_STA;
}

/*                                                      
                                                          
                                         */
v_BOOL_t hdd_is_apps_power_collapse_allowed(hdd_context_t* pHddCtx)
{
    tPmcState pmcState = pmcGetPmcState(pHddCtx->hHal);
    tANI_BOOLEAN scanRspPending = csrNeighborRoamScanRspPending(pHddCtx->hHal);
    tANI_BOOLEAN inMiddleOfRoaming = csrNeighborMiddleOfRoaming(pHddCtx->hHal);
    hdd_config_t *pConfig = pHddCtx->cfg_ini;
    hdd_adapter_list_node_t *pAdapterNode = NULL, *pNext = NULL; 
    hdd_adapter_t *pAdapter = NULL; 
    VOS_STATUS status;
    tVOS_CONCURRENCY_MODE concurrent_state = 0;

    if (VOS_STA_SAP_MODE == hdd_get_conparam())
        return TRUE;

    concurrent_state = hdd_get_concurrency_mode();

#ifdef WLAN_ACTIVEMODE_OFFLOAD_FEATURE
    if(((concurrent_state == (VOS_STA | VOS_P2P_CLIENT)) || 
        (concurrent_state == (VOS_STA | VOS_P2P_GO))) && 
        (IS_ACTIVEMODE_OFFLOAD_FEATURE_ENABLE))
        return TRUE;
#endif

    /*                                                   */
    status = hdd_get_front_adapter ( pHddCtx, &pAdapterNode );
    while ( NULL != pAdapterNode && VOS_STATUS_SUCCESS == status )
    {
        pAdapter = pAdapterNode->pAdapter;
        if ( (WLAN_HDD_INFRA_STATION == pAdapter->device_mode)
          || (WLAN_HDD_P2P_CLIENT == pAdapter->device_mode) )
        {
            if (((pConfig->fIsImpsEnabled || pConfig->fIsBmpsEnabled)
                 && (pmcState != IMPS && pmcState != BMPS
                  &&  pmcState != STOPPED && pmcState != STANDBY)) ||
                 (eANI_BOOLEAN_TRUE == scanRspPending) ||
                 (eANI_BOOLEAN_TRUE == inMiddleOfRoaming))
            {
                hddLog( LOGE, "%s: do not allow APPS power collapse-"
                    "pmcState = %d scanRspPending = %d inMiddleOfRoaming = %d",
                    __func__, pmcState, scanRspPending, inMiddleOfRoaming );
                return FALSE;
            }
        }
        status = hdd_get_next_adapter ( pHddCtx, pAdapterNode, &pNext );
        pAdapterNode = pNext;
    }
    return TRUE;
}

/*                                                     
                                                  */
v_BOOL_t hdd_is_suspend_notify_allowed(hdd_context_t* pHddCtx)
{
    tPmcState pmcState = pmcGetPmcState(pHddCtx->hHal);
    hdd_config_t *pConfig = pHddCtx->cfg_ini;

    if (pConfig->fIsBmpsEnabled && (pmcState == BMPS))
    {
        return TRUE;
    }
    return FALSE;
}

void wlan_hdd_set_concurrency_mode(hdd_context_t *pHddCtx, tVOS_CON_MODE mode)
{
   switch(mode)
   {
       case VOS_STA_MODE:
       case VOS_P2P_CLIENT_MODE:
       case VOS_P2P_GO_MODE:
       case VOS_STA_SAP_MODE:
            pHddCtx->concurrency_mode |= (1 << mode);
            pHddCtx->no_of_sessions[mode]++;
            break;
       default:
            break;

   }
   VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_INFO, "%s: concurrency_mode = 0x%x NumberofSessions for mode %d = %d",
    __func__,pHddCtx->concurrency_mode,mode,pHddCtx->no_of_sessions[mode]);
}


void wlan_hdd_clear_concurrency_mode(hdd_context_t *pHddCtx, tVOS_CON_MODE mode)
{
   switch(mode)
   {
       case VOS_STA_MODE:
       case VOS_P2P_CLIENT_MODE:
       case VOS_P2P_GO_MODE:
       case VOS_STA_SAP_MODE:
    pHddCtx->no_of_sessions[mode]--;
    if (!(pHddCtx->no_of_sessions[mode]))
            pHddCtx->concurrency_mode &= (~(1 << mode));
            break;
       default:
            break;
   }
   VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_INFO, "%s: concurrency_mode = 0x%x NumberofSessions for mode %d = %d",
    __func__,pHddCtx->concurrency_mode,mode,pHddCtx->no_of_sessions[mode]);
}

/*                                                                            
  
                                 
  
                                                                       
  
                      
  
                   
               
                                                                             */

static void wlan_hdd_restart_init(hdd_context_t *pHddCtx)
{
   /*            */
   pHddCtx->hdd_restart_retries = 0;
   atomic_set(&pHddCtx->isRestartInProgress, 0);
   vos_timer_init(&pHddCtx->hdd_restart_timer, 
                     VOS_TIMER_TYPE_SW, 
                     wlan_hdd_restart_timer_cb,
                     pHddCtx);
}
/*                                                                            
  
                                   
  
                                                                      
  
                      
  
                   
               
                                                                             */

static void wlan_hdd_restart_deinit(hdd_context_t* pHddCtx)
{
 
   VOS_STATUS vos_status;
   /*                         */
   atomic_set(&pHddCtx->isRestartInProgress, 1);
   /*         */
   vos_status = vos_timer_stop( &pHddCtx->hdd_restart_timer );
   if (!VOS_IS_STATUS_SUCCESS(vos_status))
          hddLog(LOGW, FL("Failed to stop HDD restart timer"));
   vos_status = vos_timer_destroy(&pHddCtx->hdd_restart_timer);
   if (!VOS_IS_STATUS_SUCCESS(vos_status))
          hddLog(LOGW, FL("Failed to destroy HDD restart timer"));

}

/*                                                                            
  
                                      
  
                                                                          
                               
  
                                                                  
  
  
                      
  
                                          
                                                   
                                            

                                                                             */

static VOS_STATUS wlan_hdd_framework_restart(hdd_context_t *pHddCtx) 
{
   VOS_STATUS status = VOS_STATUS_SUCCESS;
   hdd_adapter_list_node_t *pAdapterNode = NULL, *pNext = NULL;
   int len = (sizeof (struct ieee80211_mgmt));
   struct ieee80211_mgmt *mgmt = NULL; 
   
   /*                                                     */
   mgmt =  kzalloc(len, GFP_KERNEL);
   if(mgmt == NULL) 
   {
      VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_FATAL, 
            "%s: memory allocation failed (%d bytes)", __func__, len);
      return VOS_STATUS_E_NOMEM;
   }
   mgmt->u.deauth.reason_code = WLAN_REASON_DISASSOC_LOW_ACK;

   /*                                   */
   status =  hdd_get_front_adapter ( pHddCtx, &pAdapterNode );
   do 
   {
      if( (status == VOS_STATUS_SUCCESS) && 
                           pAdapterNode  && 
                           pAdapterNode->pAdapter)
      {
         VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_FATAL, 
               "restarting the driver(intf:\'%s\' mode:%d :try %d)",
               pAdapterNode->pAdapter->dev->name,
               pAdapterNode->pAdapter->device_mode,
               pHddCtx->hdd_restart_retries + 1);
         /* 
                                                
            
                                                  
                                                                               
                                               
           
                                                                                 
                       
           
          */
         
         cfg80211_send_unprot_deauth(pAdapterNode->pAdapter->dev, (u_int8_t*)mgmt, len );  
      }
      status = hdd_get_next_adapter ( pHddCtx, pAdapterNode, &pNext );
      pAdapterNode = pNext;
   } while((NULL != pAdapterNode) && (VOS_STATUS_SUCCESS == status));


   /*                                     */
   kfree(mgmt);

   /*                                          */
   if(++pHddCtx->hdd_restart_retries < WLAN_HDD_RESTART_RETRY_MAX_CNT) 
      vos_timer_start(&pHddCtx->hdd_restart_timer, WLAN_HDD_RESTART_RETRY_DELAY_MS);

   return status;

}
/*                                                                            
  
                                     
  
                                                  
  
                         
  
                   
               
                                                                             */

void wlan_hdd_restart_timer_cb(v_PVOID_t usrDataForCallback)
{
   hdd_context_t *pHddCtx = usrDataForCallback;
   wlan_hdd_framework_restart(pHddCtx);
   return;

}


/*                                                                            
  
                                   
  
                                                                            
     
                                                  
  
                      
  
                                          
                                                   
                                                                

                                                                             */
VOS_STATUS wlan_hdd_restart_driver(hdd_context_t *pHddCtx) 
{
   VOS_STATUS status = VOS_STATUS_SUCCESS;

   /*                                       */
   if(atomic_xchg(&pHddCtx->isRestartInProgress, 1))
   {
      VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_WARN, 
            "%s: WLAN restart is already in progress", __func__);

      return VOS_STATUS_E_ALREADY;
   }
   /*                                        */
#ifdef HAVE_WCNSS_RESET_INTR
   wcnss_reset_intr();
#endif
 
   return status;
}

/*
                                                             
 */
VOS_STATUS hdd_issta_p2p_clientconnected(hdd_context_t *pHddCtx)
{
    return sme_isSta_p2p_clientConnected(pHddCtx->hHal);
}

//                                       
module_init(hdd_module_init);
module_exit(hdd_module_exit);

MODULE_LICENSE("Dual BSD/GPL");
MODULE_AUTHOR("Qualcomm Atheros, Inc.");
MODULE_DESCRIPTION("WLAN HOST DEVICE DRIVER");

module_param_call(con_mode, con_mode_handler, param_get_int, &con_mode,
                    S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);

module_param_call(fwpath, fwpath_changed_handler, param_get_string, &fwpath,
                    S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
