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
#ifdef ANI_BUS_TYPE_SDIO
#include <wlan_sal_misc.h>
#endif //                  
#include <vos_api.h>
#include <vos_sched.h>
#include <vos_power.h>
#include <linux/etherdevice.h>
#include <linux/firmware.h>
#ifdef ANI_BUS_TYPE_SDIO
#include <linux/mmc/sdio_func.h>
#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,32))
//                                                                   
#define dev_to_sdio_func(d)      container_of(d, struct sdio_func, dev)
#endif
#endif //                  
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

#ifdef CONFIG_CFG80211
#include <linux/wireless.h>
#include <net/cfg80211.h>
#include "wlan_hdd_cfg80211.h"
#include "wlan_hdd_p2p.h"
#endif
#include <linux/rtnetlink.h>
#ifdef ANI_MANF_DIAG
int wlan_hdd_ftm_start(hdd_context_t *pAdapter);
#endif
#ifdef WLAN_SOFTAP_FEATURE
#include "sapApi.h"
#include <linux/semaphore.h>
#include <mach/subsystem_restart.h>
#include <wlan_hdd_hostapd.h>
#include <wlan_hdd_softap_tx_rx.h>
#endif
#ifdef FEATURE_WLAN_INTEGRATED_SOC
#include "cfgApi.h"
#endif
#include "wlan_hdd_dev_pwr.h"
#ifdef WLAN_BTAMP_FEATURE
#include "bap_hdd_misc.h"
#endif
#ifdef FEATURE_WLAN_INTEGRATED_SOC
#include "wlan_qct_pal_trace.h"
#endif /*                             */
#include "qwlan_version.h"

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
static char fwpath[BUF_LEN];
#ifndef MODULE
static int wlan_hdd_inited = 0;
#endif

/*
                                                                 
                                                  
  
 */
#define WLAN_HDD_RESTART_RETRY_DELAY_MS 5000  /*          */
#define WLAN_HDD_RESTART_RETRY_MAX_CNT  5     /*           */
#if (LINUX_VERSION_CODE < KERNEL_VERSION(3,4,5))
static struct wake_lock wlan_wake_lock;
#endif
/*                                     */
static v_U8_t      isSsrRequired;

//                             
static VOS_STATUS wlan_hdd_framework_restart(hdd_context_t *pHddCtx);
static void wlan_hdd_restart_init(hdd_context_t *pHddCtx);
static void wlan_hdd_restart_deinit(hdd_context_t *pHddCtx);
void wlan_hdd_restart_timer_cb(v_PVOID_t usrDataForCallback);

v_U16_t hdd_select_queue(struct net_device *dev,
    struct sk_buff *skb);

#ifdef WLAN_FEATURE_PACKET_FILTERING
static void hdd_set_multicast_list(struct net_device *dev);
#endif

void hdd_wlan_initial_scan(hdd_adapter_t *pAdapter);

extern int hdd_setBand_helper(struct net_device *dev, tANI_U8* ptr);

static int hdd_netdev_notifier_call(struct notifier_block * nb,
                                         unsigned long state,
                                         void *ndev)
{
   struct net_device *dev = ndev;
   hdd_adapter_t *pAdapter = NULL;
#ifdef WLAN_BTAMP_FEATURE
   VOS_STATUS status;
   hdd_context_t *pHddCtx;
#endif

   //                                                       
   if((strncmp( dev->name, "wlan", 4 )) && 
      (strncmp( dev->name, "p2p", 3))
     )
      return NOTIFY_DONE;

#ifdef CONFIG_CFG80211
   if (!dev->ieee80211_ptr)
       return NOTIFY_DONE;
#endif

   pAdapter = WLAN_HDD_GET_PRIV_PTR(dev);

   if(NULL == pAdapter)
   {
      hddLog(VOS_TRACE_LEVEL_FATAL,"%s: HDD Adaptor Null Pointer", __func__);
      VOS_ASSERT(0);
      return NOTIFY_DONE;
   }

   hddLog(VOS_TRACE_LEVEL_INFO,"%s: New Net Device State = %lu", __func__, state);

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
        if( pAdapter->scan_info.mScanPending != FALSE )
        { 
           int result;
           INIT_COMPLETION(pAdapter->abortscan_event_var);
           hdd_abort_mac_scan(pAdapter->pHddCtx);
           result = wait_for_completion_interruptible_timeout(
                               &pAdapter->abortscan_event_var,
                               msecs_to_jiffies(WLAN_WAIT_TIME_ABORTSCAN));
           if(!result)
           {
              VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR,
                         "%s: Timeout occured while waiting for abortscan" ,
                          __FUNCTION__);
           }
        }
        else
        {
           VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_INFO,
               "%s: Scan is not Pending from user" , __FUNCTION__);
        }
#ifdef WLAN_BTAMP_FEATURE
        VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_INFO,"%s: disabling AMP", __FUNCTION__);
        pHddCtx = WLAN_HDD_GET_CTX( pAdapter );
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
extern int isWDresetInProgress(void);
#ifdef CONFIG_HAS_EARLYSUSPEND
extern void register_wlan_suspend(void);
extern void unregister_wlan_suspend(void);
void hdd_unregister_mcast_bcast_filter(hdd_context_t *pHddCtx);
void hdd_register_mcast_bcast_filter(hdd_context_t *pHddCtx);
#endif
//                                      
static int con_mode = 0;
#ifndef MODULE
/*                                                          
                                                                        
                                                                   
                                                                         
                                                                             
                                                        
 */
static int curr_con_mode = 0;
#endif

#ifdef FEATURE_WLAN_INTEGRATED_SOC
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
#endif /*                             */

int hdd_ioctl(struct net_device *dev, struct ifreq *ifr, int cmd)
{
   hdd_adapter_t *pAdapter = WLAN_HDD_GET_PRIV_PTR(dev);
   hdd_priv_data_t priv_data;
   tANI_U8 *command = NULL;
   int ret = 0;

   if (NULL == pAdapter)
   {
      VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_FATAL,
         "%s: HDD adapter context is Null", __FUNCTION__);
      ret = -ENODEV;
      goto exit; 
   }

   if ((!ifr) || (!ifr->ifr_data))
   {
       ret = -EINVAL;
       goto exit; 
   }

   if (copy_from_user(&priv_data, ifr->ifr_data, sizeof(hdd_priv_data_t)))
   {
       ret = -EFAULT;
       goto exit;
   }

   command = kmalloc(priv_data.total_len, GFP_KERNEL);
   if (!command)
   {
       VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_FATAL,
          "%s: failed to allocate memory\n", __FUNCTION__);
       ret = -ENOMEM;
       goto exit;
   }

   if (copy_from_user(command, priv_data.buf, priv_data.total_len))
   {
       ret = -EFAULT;
       goto exit;
   }

   if ((SIOCDEVPRIVATE + 1) == cmd)
   {
       hdd_context_t *pHddCtx = (hdd_context_t*)pAdapter->pHddCtx;

       VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_INFO,
                  "%s: Received %s cmd from Wi-Fi GUI***", __func__, command);

       if (strncmp(command, "P2P_DEV_ADDR", 12) == 0 )
       {
           if (copy_to_user(priv_data.buf, pHddCtx->p2pDeviceAddress.bytes,
                                                           sizeof(tSirMacAddr)))
           {
               VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_FATAL,
                  "%s: failed to copy data to user buffer\n", __FUNCTION__);
               ret = -EFAULT;
           }
       }
       else if(strncmp(priv_data.buf, "SETBAND", 7) == 0)
       {
           tANI_U8 *ptr = (tANI_U8*)priv_data.buf ;
           int ret = 0 ;
        
           /*                              */
   
           /*                                        
                                                  */
           VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_INFO,
                    "%s: SetBandCommand Info  comm %s UL %d, TL %d", __FUNCTION__, priv_data.buf, priv_data.used_len, priv_data.total_len);
        
           /*                              */
           ret = hdd_setBand_helper(dev, ptr);   
       } 
       else if ( strncasecmp(command, "COUNTRY", 7) == 0 )
       {
           char *country_code;

           country_code = command + 8;
           ret = (int)sme_ChangeCountryCode(pHddCtx->hHal, NULL, country_code,
                    pAdapter, pHddCtx->pvosContext);
           if( 0 != ret )
           {
               VOS_TRACE( VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_FATAL,
                       "%s: SME Change Country code fail ret=%d\n",__func__, ret);

           }
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
         "%s: HDD adapter context is Null", __FUNCTION__);
      return -ENODEV;
   }
   
   pHddCtx = (hdd_context_t*)pAdapter->pHddCtx;
   if (NULL == pHddCtx)
   {
      VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_FATAL,
         "%s: HDD context is Null", __FUNCTION__);
      return -ENODEV;
   }

   status = hdd_get_front_adapter ( pHddCtx, &pAdapterNode );
   while ( (NULL != pAdapterNode) && (VOS_STATUS_SUCCESS == status) )
   {
        if( pAdapterNode->pAdapter->event_flags & DEVICE_IFACE_OPENED)
        {
            hddLog(VOS_TRACE_LEVEL_INFO, "%s: chip already out of " 
                  "standby", __func__, pAdapter->device_mode);
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
   
   pAdapter->event_flags |= DEVICE_IFACE_OPENED;
   if (hdd_connIsConnected(WLAN_HDD_GET_STATION_CTX_PTR(pAdapter))) 
   {
       VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_INFO,
                 "%s: Enabling Tx Queues", __FUNCTION__);
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
         "%s: HDD adapter context is Null", __FUNCTION__);
      return -1;
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
         "%s: HDD adapter context is Null", __FUNCTION__);
      return -ENODEV;
   }

   pHddCtx = (hdd_context_t*)pAdapter->pHddCtx;
   if (NULL == pHddCtx)
   {
      VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_FATAL,
         "%s: HDD context is Null", __FUNCTION__);
      return -ENODEV;
   }

   pAdapter->event_flags &= ~(DEVICE_IFACE_OPENED);
   hddLog(VOS_TRACE_LEVEL_INFO, "%s: Disabling OS Tx queues", __func__);
   netif_tx_disable(pAdapter->dev);
   netif_carrier_off(pAdapter->dev);


   /*                                                 
                             */
   if ( (WLAN_HDD_SOFTAP == pAdapter->device_mode )
                 || (WLAN_HDD_MONITOR == pAdapter->device_mode )
#ifdef WLAN_FEATURE_P2P
                 || (WLAN_HDD_P2P_GO == pAdapter->device_mode )
#endif
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
        if ( pAdapterNode->pAdapter->event_flags & DEVICE_IFACE_OPENED)
        {
            hddLog(VOS_TRACE_LEVEL_INFO, "%s: Still other ifaces are up cannot "
                   "put device to sleep", __func__, pAdapter->device_mode);
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
#ifndef FEATURE_WLAN_INTEGRATED_SOC
static void hdd_set_mac_addr_cb( tHalHandle hHal, tANI_S32 result )
{
  //                                    

  VOS_ASSERT( CCM_IS_RESULT_SUCCESS( result ) );
}
#endif


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
#ifdef CONFIG_CFG80211   
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

hdd_adapter_t* hdd_alloc_station_adapter( hdd_context_t *pHddCtx, tSirMacAddr macAddr, char* name )
{
   struct net_device *pWlanDev = NULL;
   hdd_adapter_t *pAdapter = NULL;
#ifdef CONFIG_CFG80211
   /*
                                                  
    */ 
   pWlanDev = alloc_netdev_mq(sizeof( hdd_adapter_t ), name, ether_setup, NUM_TX_QUEUES);
   
#else      
   //                                                       
   pWlanDev = alloc_etherdev_mq(sizeof( hdd_adapter_t ), NUM_TX_QUEUES);

#endif

   if(pWlanDev != NULL)
   {

      //                                                     
      pAdapter = (hdd_adapter_t*) netdev_priv( pWlanDev );

#ifndef CONFIG_CFG80211
      //                             
      ether_setup(pWlanDev);
#endif

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
      init_completion(&pAdapter->abortscan_event_var);
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,38))
      init_completion(&pAdapter->offchannel_tx_event);
#endif
#ifdef CONFIG_CFG80211
      init_completion(&pAdapter->tx_action_cnf_event);
#endif
      init_completion(&pHddCtx->mc_sus_event_var);
      init_completion(&pHddCtx->tx_sus_event_var);

      init_completion(&pAdapter->scan_info.scan_req_completion_event);

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
#ifdef CONFIG_CFG80211
      pWlanDev->ieee80211_ptr = &pAdapter->wdev ;
      pAdapter->wdev.wiphy = pHddCtx->wiphy;  
      pAdapter->wdev.netdev =  pWlanDev;
#endif  
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
      if (strchr(pWlanDev->name, '%')) {
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

eHalStatus hdd_smeCloseSessionCallback(void *pContext)
{
   if(pContext != NULL)
   {
      clear_bit(SME_SESSION_OPENED, &((hdd_adapter_t*)pContext)->event_flags);

      /*                                                           
                                                                          
                                                      
       */
      flush_scheduled_work();
      complete(&((hdd_adapter_t*)pContext)->session_close_comp_var);
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

   return VOS_STATUS_SUCCESS;

error_wmm_init:
   clear_bit(INIT_TX_RX_SUCCESS, &pAdapter->event_flags);
   hdd_deinit_tx_rx(pAdapter);
error_init_txrx:
   hdd_UnregisterWext(pWlanDev);
error_register_wext:
   if(test_bit(SME_SESSION_OPENED, &pAdapter->event_flags))
   {
      INIT_COMPLETION(pAdapter->session_close_comp_var);
      if( eHAL_STATUS_SUCCESS == sme_CloseSession( pHddCtx->hHal,
                                    pAdapter->sessionId,
                                    hdd_smeCloseSessionCallback, pAdapter ) )
      {
         //                                                          
         wait_for_completion_interruptible_timeout(
                          &pAdapter->session_close_comp_var,
                           msecs_to_jiffies(WLAN_WAIT_TIME_SESSIONOPENCLOSE));
      }
}
error_sme_open:
   return status;
}

#ifdef CONFIG_CFG80211
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
#endif

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

#ifdef CONFIG_CFG80211
         hdd_cleanup_actionframe(pHddCtx, pAdapter);
#endif

         break;
      }

      case WLAN_HDD_SOFTAP:
      case WLAN_HDD_P2P_GO:
#ifdef WLAN_SOFTAP_FEATURE
      {
#ifdef CONFIG_CFG80211
         hdd_cleanup_actionframe(pHddCtx, pAdapter);
#endif

         hdd_unregister_hostapd(pAdapter);
         hdd_set_conparam( 0 );
#ifdef CONFIG_CFG80211
         wlan_hdd_set_monitor_tx_adapter( WLAN_HDD_GET_CTX(pAdapter), NULL );
#endif
         break;
      }

      case WLAN_HDD_MONITOR:
      {
#ifdef CONFIG_CFG80211
          hdd_adapter_t* pAdapterforTx = pAdapter->sessionCtx.monitor.pAdapterForTx;
#endif
         if(test_bit(INIT_TX_RX_SUCCESS, &pAdapter->event_flags))
         {
            hdd_deinit_tx_rx( pAdapter );
            clear_bit(INIT_TX_RX_SUCCESS, &pAdapter->event_flags);
         }
#ifdef CONFIG_CFG80211
         if(NULL != pAdapterforTx)
         {
            hdd_cleanup_actionframe(pHddCtx, pAdapterforTx);
         }
#endif
#endif //                   
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
#ifdef WLAN_FEATURE_P2P
       case WLAN_HDD_P2P_CLIENT:
       case WLAN_HDD_P2P_GO:
#endif
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
                                 char *iface_name, tSirMacAddr macAddr, 
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
#ifdef WLAN_FEATURE_P2P
      case WLAN_HDD_P2P_CLIENT:
      case WLAN_HDD_P2P_DEVICE:
#endif
      {
         pAdapter = hdd_alloc_station_adapter( pHddCtx, macAddr, iface_name );

         if( NULL == pAdapter )
            return NULL;

#ifdef CONFIG_CFG80211
         pAdapter->wdev.iftype = (session_type == WLAN_HDD_P2P_CLIENT) ?
                                  NL80211_IFTYPE_P2P_CLIENT:
                                  NL80211_IFTYPE_STATION;
#endif

         pAdapter->device_mode = session_type;

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

#ifdef WLAN_FEATURE_P2P
      case WLAN_HDD_P2P_GO:
#endif
      case WLAN_HDD_SOFTAP:
      {
         pAdapter = hdd_wlan_create_ap_dev( pHddCtx, macAddr, (tANI_U8 *)iface_name );
         if( NULL == pAdapter )
            return NULL;

#ifdef CONFIG_CFG80211
         pAdapter->wdev.iftype = (session_type == WLAN_HDD_SOFTAP) ?
                                  NL80211_IFTYPE_AP:
                                  NL80211_IFTYPE_P2P_GO;
#endif
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
#ifdef CONFIG_CFG80211   
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
#ifdef WLAN_FEATURE_P2P
         if (NULL == pAdapter->sessionCtx.monitor.pAdapterForTx)
         {
            pAdapter->sessionCtx.monitor.pAdapterForTx = 
                           hdd_get_adapter(pAdapter->pHddCtx, WLAN_HDD_P2P_GO);
         }
#endif
         /*                                                               
                               */
         INIT_WORK(&pAdapter->sessionCtx.monitor.pAdapterForTx->monTxWorkQueue,
                   hdd_mon_tx_work_queue);
#endif
      }
         break;
#ifdef ANI_MANF_DIAG
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
      }
         break;
#endif
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

#ifdef FEATURE_WLAN_NON_INTEGRATED_SOC
      /*                                                             
                                    
                                                                           
                                  */ 
      if (vos_concurrent_sessions_running())
      {
         WLANTL_ConfigureSwFrameTXXlationForAll(pHddCtx->pvosContext, TRUE);
      }
#endif
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
      hdd_remove_adapter( pHddCtx, pAdapterNode );
      vos_mem_free( pAdapterNode );

#ifdef FEATURE_WLAN_NON_INTEGRATED_SOC
      /*                                                                
                                */ 
      /*                                                              
                             */
      if (!vos_concurrent_sessions_running())
      {
         WLANTL_ConfigureSwFrameTXXlationForAll(pHddCtx->pvosContext, FALSE);
      }
#endif

      /*                                                                */
      if ((!vos_concurrent_sessions_running()) && 
           ((pHddCtx->no_of_sessions[VOS_STA_MODE] >= 1) || 
           (pHddCtx->no_of_sessions[VOS_P2P_CLIENT_MODE] >= 1)))
      {
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
               wait_for_completion_interruptible_timeout(
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
                         __FUNCTION__);
               }
            }
            else
            {
               hddLog(LOGE, "%s: failure in WLANSAP_StopBss", __FUNCTION__);
            }
            clear_bit(SOFTAP_BSS_STARTED, &pAdapter->event_flags);

            if (eHAL_STATUS_FAILURE ==
                ccmCfgSetInt(pHddCtx->hHal, WNI_CFG_PROBE_RSP_BCN_ADDNIE_FLAG,
                             0, NULL, eANI_BOOLEAN_FALSE))
            {
               hddLog(LOGE,
                      "%s: Failed to set WNI_CFG_PROBE_RSP_BCN_ADDNIE_FLAG",
                      __FUNCTION__);
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

      //                                
      pAdapter->sessionCtx.station.bSendDisconnect = 
            hdd_connIsConnected( WLAN_HDD_GET_STATION_CTX_PTR( pAdapter )) ?
                                                       VOS_TRUE : VOS_FALSE;

      hdd_deinit_tx_rx(pAdapter);
      hdd_wmm_adapter_close(pAdapter);

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
   v_MACADDR_t  bcastMac = VOS_MAC_ADDR_BROADCAST_INITIALIZER;

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
            hdd_init_station_mode(pAdapter);
            /*                                                 */
            pAdapter->isLinkUpSvcNeeded = FALSE; 
            pAdapter->scan_info.mScanPending = FALSE;
            pAdapter->scan_info.waitScanResult = FALSE;

            //                        
            hdd_wlan_initial_scan(pAdapter);

            //                                                                
            if(pAdapter->sessionCtx.station.bSendDisconnect)
            {
               union iwreq_data wrqu;
               memset(&wrqu, '\0', sizeof(wrqu));
               wrqu.ap_addr.sa_family = ARPHRD_ETHER;
               memset(wrqu.ap_addr.sa_data,'\0',ETH_ALEN);
               wireless_send_event(pAdapter->dev, SIOCGIWAP, &wrqu, NULL);
               pAdapter->sessionCtx.station.bSendDisconnect = VOS_FALSE;

#ifdef CONFIG_CFG80211
               /*                                        */
               cfg80211_disconnected(pAdapter->dev, WLAN_REASON_UNSPECIFIED,
                                     NULL, 0, GFP_KERNEL); 
#endif
            }
            break;

         case WLAN_HDD_SOFTAP:
            /*                       */
            break;

         case WLAN_HDD_P2P_GO:
#ifdef CONFIG_CFG80211
              hddLog(VOS_TRACE_LEVEL_ERROR, "%s [SSR] send restart supplicant",
                                                       __func__);
              /*                             */
              cfg80211_del_sta(pAdapter->dev,
                        (const u8 *)&bcastMac.bytes[0], GFP_KERNEL);
#endif
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

v_U8_t hdd_is_ssr_required( void)
{
    return isSsrRequired;
}

void hdd_set_ssr_required( v_U8_t value)
{
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

#ifdef CONFIG_CFG80211
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
#endif
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
   hdd_context_t *pHddCtx;
   int mc_count;
   int i = 0;
   struct netdev_hw_addr *ha;
   pHddCtx = (hdd_context_t*)pAdapter->pHddCtx;
   if (NULL == pHddCtx)
   {
      hddLog(VOS_TRACE_LEVEL_ERROR,
            "%s: HDD context is Null", __FUNCTION__);
      return;
   }

   if (dev->flags & IFF_ALLMULTI)
   {
      hddLog(VOS_TRACE_LEVEL_INFO,
            "%s: allow all multicast frames", __FUNCTION__);
      pHddCtx->mc_addr_list.mc_cnt = 0;
   }
   else 
   {
      mc_count = netdev_mc_count(dev);
      hddLog(VOS_TRACE_LEVEL_INFO,
            "%s: mc_count = %u", __FUNCTION__, mc_count);
      if (mc_count > WLAN_HDD_MAX_MC_ADDR_LIST)
      {
         hddLog(VOS_TRACE_LEVEL_INFO,
               "%s: No free filter available; allow all multicast frames", __FUNCTION__);
         pHddCtx->mc_addr_list.mc_cnt = 0;
         return;
      }

      pHddCtx->mc_addr_list.mc_cnt = mc_count;

      netdev_for_each_mc_addr(ha, dev) {
         if (i == mc_count)
            break;
         memset(&(pHddCtx->mc_addr_list.addr[i][0]), 0, ETH_ALEN);
         memcpy(&(pHddCtx->mc_addr_list.addr[i][0]), ha->addr, ETH_ALEN);
         hddLog(VOS_TRACE_LEVEL_INFO, "\n%s: mlist[%d] = %02x:%02x:%02x:%02x:%02x:%02x", 
               __func__, i, 
               pHddCtx->mc_addr_list.addr[i][0], pHddCtx->mc_addr_list.addr[i][1], 
               pHddCtx->mc_addr_list.addr[i][2], pHddCtx->mc_addr_list.addr[i][3], 
               pHddCtx->mc_addr_list.addr[i][4], pHddCtx->mc_addr_list.addr[i][5]);
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
            return;
         }
         vos_mem_copy(scanReq.ChannelInfo.ChannelList, channelInfo.ChannelList,
            channelInfo.numOfChannels);
         scanReq.ChannelInfo.numOfChannels = channelInfo.numOfChannels;
         vos_mem_free(channelInfo.ChannelList);
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
          "%s: context = %p, status = %d", pContext, status);

   if (NULL == callbackContext)
   {
      hddLog(VOS_TRACE_LEVEL_ERROR,
             "%s: Bad param, context [%p]",
             __FUNCTION__, callbackContext);
      return;
   }

   /*                                                                     
                                                                     
                                                                    
                                                                    */

   if (POWER_CONTEXT_MAGIC != pContext->magic)
   {
      /*                                                               */
      hddLog(VOS_TRACE_LEVEL_WARN,
             "%s: Invalid context, magic [%08x]",
              __FUNCTION__, pContext->magic);
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
#ifdef ANI_BUS_TYPE_SDIO
   struct sdio_func *sdio_func_dev = NULL;
#endif //                  
#ifdef CONFIG_CFG80211
    struct wiphy *wiphy = pHddCtx->wiphy;
#endif 
#ifdef FEATURE_WLAN_INTEGRATED_SOC
   hdd_adapter_t* pAdapter;
#endif
   struct fullPowerContext powerContext;
   long lrc;

   ENTER();

   //                                              
   wlan_hdd_restart_deinit(pHddCtx);

#ifdef CONFIG_CFG80211
#ifdef WLAN_SOFTAP_FEATURE
   if (VOS_STA_SAP_MODE != hdd_get_conparam())
#endif
   {
#ifdef ANI_MANF_DIAG
      if (VOS_FTM_MODE != hdd_get_conparam())
#endif /*               */
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
#endif

#ifdef ANI_MANF_DIAG
   if (VOS_FTM_MODE == hdd_get_conparam())
  {
    wlan_hdd_ftm_close(pHddCtx);
    goto free_hdd_ctx;
  }
#endif  
   //                            
   //                           
   //                            

#ifdef CONFIG_HAS_EARLYSUSPEND
   //                                    
   if(pHddCtx->cfg_ini->nEnableSuspend)
   {
      unregister_wlan_suspend();
   }
#endif

#ifdef FEATURE_WLAN_INTEGRATED_SOC
#ifdef WLAN_SOFTAP_FEATURE
   if (VOS_STA_SAP_MODE == hdd_get_conparam())
   {
      pAdapter = hdd_get_adapter(pHddCtx,
                                   WLAN_HDD_SOFTAP);
   }
   else
   {
#endif
#ifdef ANI_MANF_DIAG
      if (VOS_FTM_MODE != hdd_get_conparam())
#endif /*               */
      {
         pAdapter = hdd_get_adapter(pHddCtx,
                                    WLAN_HDD_INFRA_STATION);
      }
#ifdef WLAN_SOFTAP_FEATURE
   }
#endif
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
#endif //                           

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
                   __FUNCTION__, (0 == lrc) ? "timeout" : "interrupt");
            /*                                                 
                                                                      
                                                                      
                                                                     
                                                                 
                                                                     
                                               */
            msleep(50);
         }
      }
      else
      {
         hddLog(VOS_TRACE_LEVEL_ERROR,
                "%s: Request for Full Power failed, status %d",
                __FUNCTION__, halStatus);
         VOS_ASSERT(0);
         /*                                                  */
      }
   }

   //                                   
   unregister_netdevice_notifier(&hdd_netdev_notifier);
   
   hdd_stop_all_adapters( pHddCtx );

#ifdef ANI_BUS_TYPE_SDIO
   sdio_func_dev = libra_getsdio_funcdev();

   if(sdio_func_dev == NULL)
   {
        hddLog(VOS_TRACE_LEVEL_FATAL, "%s: sdio_func_dev is NULL!",__func__);
        VOS_ASSERT(0);
        return;
   }

   sd_claim_host(sdio_func_dev);

   /*                                       */
   libra_enable_sdio_irq(sdio_func_dev, 0);

   sd_release_host(sdio_func_dev);
#endif //                  

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

#ifdef ANI_BUS_TYPE_SDIO
   vosStatus = WLANBAL_Stop( pVosContext );

   hddLog(VOS_TRACE_LEVEL_ERROR,"WLAN BAL STOP\n");
   if (!VOS_IS_STATUS_SUCCESS(vosStatus))
   {
      VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_FATAL,
         "%s: Failed to stop BAL",__func__);
      VOS_ASSERT( VOS_IS_STATUS_SUCCESS( vosStatus ) );
   }

    msleep(50);
   //                                                   
   vosStatus = WLANBAL_SuspendChip( pVosContext );

   hddLog(VOS_TRACE_LEVEL_ERROR,"WLAN Suspend Chip\n");

   if (!VOS_IS_STATUS_SUCCESS(vosStatus))
   {
      VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_FATAL,
         "%s: Failed to suspend chip ",__func__);
      VOS_ASSERT( VOS_IS_STATUS_SUCCESS( vosStatus ) );
   }
   //               
   vosStatus = WLANSAL_Stop( pVosContext );
   if (!VOS_IS_STATUS_SUCCESS(vosStatus))
   {
      VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_FATAL,
         "%s: Failed to stop SAL",__func__);
      VOS_ASSERT( VOS_IS_STATUS_SUCCESS( vosStatus ) );
   }

#endif //                  

   //                                                                  
   vosStatus = vos_chipAssertDeepSleep( NULL, NULL, NULL );
   VOS_ASSERT( VOS_IS_STATUS_SUCCESS( vosStatus ) );

   //                                  
   vos_chipPowerDown(NULL, NULL, NULL);

   vos_chipVoteOffXOBuffer(NULL, NULL, NULL);

   //                          
   send_btc_nlink_msg(WLAN_MODULE_DOWN_IND, 0);
   nl_srv_exit();

   //                                                        
#ifdef CONFIG_HAS_EARLYSUSPEND
   hdd_unregister_mcast_bcast_filter(pHddCtx);
#endif

   //                                                                       
   //                                                                        
   //                      
   vosStatus = vos_sched_close( pVosContext );
   if (!VOS_IS_STATUS_SUCCESS(vosStatus))    {
      VOS_TRACE( VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_FATAL,
         "%s: Failed to close VOSS Scheduler",__func__);
      VOS_ASSERT( VOS_IS_STATUS_SUCCESS( vosStatus ) );
   }

#ifdef WLAN_FEATURE_HOLD_RX_WAKELOCK
   /*                       */
   wake_lock_destroy(&pHddCtx->rx_wake_lock);
#endif

   //          
   //                                                                                                
   vos_close(pVosContext);

#ifdef ANI_BUS_TYPE_SDIO
   vosStatus = WLANBAL_Close(pVosContext);
   if (!VOS_IS_STATUS_SUCCESS(vosStatus))
   {
      VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_FATAL, 
          "%s: Failed to close BAL",__func__);
      VOS_ASSERT( VOS_IS_STATUS_SUCCESS( vosStatus ) );
   }
   hddLog(VOS_TRACE_LEVEL_ERROR,"Returned WLAN BAL CLOSE\n\n\n\n");
#endif //                  

   //              
   if(pHddCtx->cfg_ini->fIsLogpEnabled)
      vos_watchdog_close(pVosContext);

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

#ifdef ANI_MANF_DIAG
free_hdd_ctx:   
#endif
#ifdef CONFIG_CFG80211
   wiphy_unregister(wiphy) ; 
   wiphy_free(wiphy) ;
#else
   vos_mem_free( pHddCtx );
#endif
   if (hdd_is_ssr_required())
   {
       /*                                                               */
       subsystem_restart("riva");
       msleep(5000);
   }
   hdd_set_ssr_required (VOS_FALSE);
}


/*                                                                            

                                                                         
                                                                        

                                                       

                                            

                                                                            */
static VOS_STATUS hdd_update_config_from_nv(hdd_context_t* pHddCtx)
{
#ifndef FEATURE_WLAN_INTEGRATED_SOC
   eHalStatus halStatus;
#endif

#ifdef FEATURE_WLAN_INTEGRATED_SOC
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
#endif /*                             */

#ifndef FEATURE_WLAN_INTEGRATED_SOC
#if 1 /*                             */
   //                    
   //                                                                                                           
   halStatus = ccmCfgSetStr( pHddCtx->hHal, WNI_CFG_STA_ID,
                             (v_U8_t *)&pHddCtx->cfg_ini->intfMacAddr[0],
                             sizeof( pHddCtx->cfg_ini->intfMacAddr[0]),
                             hdd_set_mac_addr_cb, VOS_FALSE );

   if (!HAL_STATUS_SUCCESS( halStatus ))
   {
      hddLog(VOS_TRACE_LEVEL_ERROR,"%s: Failed to set MAC Address. "
          "HALStatus is %08d [x%08x]",__func__, halStatus, halStatus );
      return VOS_STATUS_E_FAILURE;
   }
#endif
#endif

   return VOS_STATUS_SUCCESS;
}

/*                                                                            

                                                                         

                                         

                

                                                                            */
VOS_STATUS hdd_post_voss_start_config(hdd_context_t* pHddCtx)
{
   eHalStatus halStatus;
   v_U32_t listenInterval;

#ifdef FEATURE_WLAN_NON_INTEGRATED_SOC
   /*                                                                    
                                                          
   */

   //                             
   if (FALSE == hdd_update_config_dat(pHddCtx))
   {
      hddLog(VOS_TRACE_LEVEL_FATAL,"%s: config update failed",__func__ );
      return VOS_STATUS_E_FAILURE;
   }

   //                        
   if (VOS_STATUS_SUCCESS != hdd_update_config_from_nv(pHddCtx))
   {
      hddLog(VOS_TRACE_LEVEL_FATAL,
             "%s: config update from NV failed", __func__ );
      return VOS_STATUS_E_FAILURE;
   }
#endif //                                

   //                                                              
   //                                                            
   halStatus = sme_HDDReadyInd( pHddCtx->hHal );
   if ( !HAL_STATUS_SUCCESS( halStatus ) )
   {
      hddLog(VOS_TRACE_LEVEL_ERROR,"%S: sme_HDDReadyInd() failed with status "
          "code %08d [x%08x]",__func__, halStatus, halStatus );
      return VOS_STATUS_E_FAILURE;
   }

   //                                 
   //                                                                        
   //                    
   wlan_cfgGetInt(pHddCtx->hHal, WNI_CFG_LISTEN_INTERVAL, &listenInterval);
   pHddCtx->hdd_actual_LI_value = listenInterval;
   
   return VOS_STATUS_SUCCESS;
}

#ifdef ANI_BUS_TYPE_SDIO

#ifndef ANI_MANF_DIAG
//                              
void wlan_hdd_enable_deepsleep(v_VOID_t * pVosContext)
{
/*                                                                       
                          

                                                                   
                                                                       
                                                                      
                                                                 

                                                                             
                     */

    return;
}
#endif
#endif

/*                        */
void hdd_prevent_suspend(void)
{
#if (LINUX_VERSION_CODE < KERNEL_VERSION(3,4,5))
    wake_lock(&wlan_wake_lock);
#else
    wcnss_prevent_suspend();
#endif
}

void hdd_allow_suspend(void)
{
#if (LINUX_VERSION_CODE < KERNEL_VERSION(3,4,5))
    wake_unlock(&wlan_wake_lock);
#else
    wcnss_allow_suspend();
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
                __FUNCTION__);
         break;
      }

      vstatus = sme_GetWcnssWlanReportedVersion(pHddCtx->hHal,
                                                &versionReported);
      if (!VOS_IS_STATUS_SUCCESS(vstatus))
      {
         hddLog(VOS_TRACE_LEVEL_FATAL,
                "%s: unable to retrieve WCNSS WLAN reported version",
                __FUNCTION__);
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
                __FUNCTION__);
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
                __FUNCTION__);
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
         sme_featureCapsExchange(pHddCtx->hHal);

   } while (0);

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
#ifdef CONFIG_CFG80211
   struct wiphy *wiphy;
#endif
#ifdef ANI_BUS_TYPE_SDIO
   struct sdio_func *sdio_func_dev = dev_to_sdio_func(dev);
#endif //                 

   ENTER();
#ifdef CONFIG_CFG80211
   /*
                                
    */
   wiphy = wlan_hdd_cfg80211_init(sizeof(hdd_context_t)) ;

   if(wiphy == NULL)
   {
      hddLog(VOS_TRACE_LEVEL_ERROR,"%s: cfg80211 init failed", __func__);
      return -1;
   }

   pHddCtx = wiphy_priv(wiphy);

#else      
      
   pHddCtx = vos_mem_malloc ( sizeof( hdd_context_t ) );
   if(pHddCtx == NULL)
   {
      hddLog(VOS_TRACE_LEVEL_ERROR,"%s: cfg80211 init failed", __func__);
      return -1;
   }

#endif   
   //                                        
   vos_mem_zero(pHddCtx, sizeof( hdd_context_t ));

#ifdef CONFIG_CFG80211
   pHddCtx->wiphy = wiphy;
#endif
   hdd_prevent_suspend();
   pHddCtx->isLoadUnloadInProgress = TRUE;

   vos_set_load_unload_in_progress(VOS_MODULE_ID_VOSS, TRUE);

   /*                                              */
   pVosContext = vos_get_global_context(VOS_MODULE_ID_SYS, NULL);

   //                                                           
   pHddCtx->pvosContext = pVosContext;

   //                                                      
   ((VosContextType*)(pVosContext))->pHDDContext = (v_VOID_t*)pHddCtx;

#ifdef ANI_BUS_TYPE_SDIO
   //                                                    
   libra_sdio_setprivdata (sdio_func_dev, pHddCtx);
   atomic_set(&pHddCtx->sdio_claim_count, 0);
#endif //                  

   pHddCtx->parent_dev = dev;

   init_completion(&pHddCtx->full_pwr_comp_var);
   init_completion(&pHddCtx->standby_comp_var);
   init_completion(&pHddCtx->req_bmps_comp_var);


   hdd_list_init( &pHddCtx->hddAdapters, MAX_NUMBER_OF_ADAPTERS );

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

#ifdef CONFIG_CFG80211
   /*
                                                   
    */
   if (0 < wlan_hdd_cfg80211_register(dev, wiphy, pHddCtx->cfg_ini))
   {
      hddLog(VOS_TRACE_LEVEL_FATAL, 
              "%s: wlan_hdd_cfg80211_register return failure", __func__);
      goto err_wiphy_reg;
   }
#endif

#ifdef FEATURE_WLAN_INTEGRATED_SOC
   //                                               
   hdd_wdi_trace_enable(eWLAN_MODULE_DAL,
                        pHddCtx->cfg_ini->wdiTraceEnableDAL);
   hdd_wdi_trace_enable(eWLAN_MODULE_DAL_CTRL,
                        pHddCtx->cfg_ini->wdiTraceEnableCTL);
   hdd_wdi_trace_enable(eWLAN_MODULE_DAL_DATA,
                        pHddCtx->cfg_ini->wdiTraceEnableDAT);
   hdd_wdi_trace_enable(eWLAN_MODULE_PAL,
                        pHddCtx->cfg_ini->wdiTraceEnablePAL);
#endif /*                             */

#ifdef ANI_MANF_DIAG 
   if(VOS_FTM_MODE == hdd_get_conparam())
  {
      if ( VOS_STATUS_SUCCESS != wlan_hdd_ftm_open(pHddCtx) )
      {
          hddLog(VOS_TRACE_LEVEL_FATAL,"%s: wlan_hdd_ftm_open Failed",__func__);
          goto err_free_hdd_context;
      }
      hddLog(VOS_TRACE_LEVEL_FATAL,"%s: FTM driver loaded success fully",__func__);
      return VOS_STATUS_SUCCESS;
  }
#endif

    //                    
   if(pHddCtx->cfg_ini->fIsLogpEnabled)
   {
      status = vos_watchdog_open(pVosContext,
         &((VosContextType*)pVosContext)->vosWatchdog, sizeof(VosWatchdogContext));

      if(!VOS_IS_STATUS_SUCCESS( status ))
      {
         hddLog(VOS_TRACE_LEVEL_FATAL,"%s: vos_watchdog_open failed",__func__);
#ifdef CONFIG_CFG80211
         goto err_wiphy_reg;
#else
         goto err_config;
#endif
      }
   }

   pHddCtx->isLogpInProgress = FALSE;
   vos_set_logp_in_progress(VOS_MODULE_ID_VOSS, FALSE);

#ifdef ANI_BUS_TYPE_SDIO
   status = WLANBAL_Open(pHddCtx->pvosContext);
   if(!VOS_IS_STATUS_SUCCESS(status))
   {
     VOS_TRACE( VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR,
        "%s: Failed to open BAL",__func__);
      goto err_wdclose;
   }
#endif //                  

   status = vos_chipVoteOnXOBuffer(NULL, NULL, NULL);
   if(!VOS_IS_STATUS_SUCCESS(status))
   {
      hddLog(VOS_TRACE_LEVEL_FATAL, "%s: Failed to configure 19.2 MHz Clock", __func__);
#ifdef ANI_BUS_TYPE_SDIO
      goto err_balclose;
#else
      goto err_wdclose;
#endif
   }


#ifdef ANI_BUS_TYPE_SDIO
   status = WLANSAL_Start(pHddCtx->pvosContext);
   if (!VOS_IS_STATUS_SUCCESS(status))
   {
      hddLog(VOS_TRACE_LEVEL_FATAL, "%s: Failed to start SAL",__func__);
      goto err_clkvote;
   }

  /*           */
  status = WLANBAL_Start(pHddCtx->pvosContext);

  if (!VOS_IS_STATUS_SUCCESS(status))
   {
     VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR,
              "%s: Failed to start BAL",__func__);
     goto err_salstop;
  }
#endif //                  

#ifdef MSM_PLATFORM_7x30
   /*                                                                                              
                                             
    */
#endif

   status = vos_open( &pVosContext, 0);
   if ( !VOS_IS_STATUS_SUCCESS( status ))
   {
      hddLog(VOS_TRACE_LEVEL_FATAL,"%s: vos_open failed",__func__);
      goto err_balstop;   
   }

   /*                                 */
   pHddCtx->hHal = (tHalHandle)vos_get_context( VOS_MODULE_ID_SME, pVosContext );

   if ( NULL == pHddCtx->hHal )
   {
      hddLog(VOS_TRACE_LEVEL_FATAL,"%s: HAL context is null",__func__);      
      goto err_vosclose;
   }

#ifdef FEATURE_WLAN_INTEGRATED_SOC
      /*                         */
      /*                                                                                                           */
      status = vos_preStart( pHddCtx->pvosContext );
      if ( !VOS_IS_STATUS_SUCCESS( status ) )
      {
         hddLog(VOS_TRACE_LEVEL_FATAL,"%s: vos_preStart failed",__func__);
         goto err_vosclose;
      }
#endif

   //                                        
   status = hdd_set_sme_config( pHddCtx );

   if ( VOS_STATUS_SUCCESS != status )
   {
      hddLog(VOS_TRACE_LEVEL_FATAL,"%s: Failed hdd_set_sme_config",__func__); 
         goto err_vosclose;
      }

   //                         
   status = hdd_wmm_init(pHddCtx);
   if (!VOS_IS_STATUS_SUCCESS(status))
   {
      hddLog(VOS_TRACE_LEVEL_FATAL, "%s: hdd_wmm_init failed", __FUNCTION__);
      goto err_vosclose;
   }

#ifdef FEATURE_WLAN_INTEGRATED_SOC
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
         return VOS_STATUS_E_FAILURE;
      }
   }
#endif //                            

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

#ifdef WLAN_SOFTAP_FEATURE
   if (VOS_STA_SAP_MODE == hdd_get_conparam())
   {
     pAdapter = hdd_open_adapter( pHddCtx, WLAN_HDD_SOFTAP, "softap.%d", 
         wlan_hdd_get_intf_addr(pHddCtx), FALSE );
   }
   else
   {
#endif
     pAdapter = hdd_open_adapter( pHddCtx, WLAN_HDD_INFRA_STATION, "wlan%d",
         wlan_hdd_get_intf_addr(pHddCtx), FALSE );
     if (pAdapter != NULL)
     {
#ifdef WLAN_FEATURE_P2P
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
                    __FUNCTION__);
                goto err_close_adapter;
             }
         }

         pP2pAdapter = hdd_open_adapter( pHddCtx, WLAN_HDD_P2P_DEVICE, "p2p%d",
                           &pHddCtx->p2pDeviceAddress.bytes[0], FALSE );
         if ( NULL == pP2pAdapter )
         {
             hddLog(VOS_TRACE_LEVEL_FATAL,
                "%s: Failed to do hdd_open_adapter for P2P Device Interface",
                __FUNCTION__);
             goto err_close_adapter;
         }
#endif
     }
#ifdef WLAN_SOFTAP_FEATURE
   }
#endif

   if( pAdapter == NULL )
   {
     hddLog(VOS_TRACE_LEVEL_ERROR,"%s: hdd_open_adapter failed",__func__);
#ifdef ANI_BUS_TYPE_SDIO
     goto err_balstop;
#else
     goto err_clkvote;
#endif
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

#ifdef FEATURE_WLAN_SCAN_PNO
   /*                                                  */
   sme_UpdateChannelConfig(pHddCtx->hHal); 
#endif

#ifdef FEATURE_WLAN_INTEGRATED_SOC
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
#endif

   /*                                                    */
   if (req_riva_power_on_lock("wlan"))
   {
      hddLog(VOS_TRACE_LEVEL_FATAL,"%s: req riva power on lock failed",
                                     __func__);
      goto err_unregister_pmops;
   }

#ifdef CONFIG_HAS_EARLYSUSPEND
   //                                  
   if(pHddCtx->cfg_ini->nEnableSuspend)
   {
      register_wlan_suspend();
   }
#endif

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
      hddLog(VOS_TRACE_LEVEL_FATAL,"%S: nl_srv_init failed",__func__);
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

   //                           
   if(!hdd_init_wowl(pHddCtx))
   {
      hddLog(VOS_TRACE_LEVEL_FATAL,"%s: hdd_init_wowl failed",__func__);
      goto err_nl_srv;
   }

#ifdef CONFIG_HAS_EARLYSUSPEND
   hdd_register_mcast_bcast_filter(pHddCtx);
#endif
#ifdef CONFIG_CFG80211
#ifdef WLAN_SOFTAP_FEATURE
   if (VOS_STA_SAP_MODE != hdd_get_conparam())
#endif
   {
      wlan_hdd_cfg80211_post_voss_start(pP2pAdapter);
   }
#endif

   mutex_init(&pHddCtx->sap_lock);

   pHddCtx->isLoadUnloadInProgress = FALSE;

#ifdef WLAN_FEATURE_HOLD_RX_WAKELOCK
   /*                          */
   wake_lock_init(&pHddCtx->rx_wake_lock,
           WAKE_LOCK_SUSPEND,
           "qcom_rx_wakelock");
#endif

   vos_event_init(&pAdapter->scan_info.scan_finished_event);
   pAdapter->scan_info.scan_pending_option = WEXT_SCAN_PENDING_GIVEUP;

   vos_set_load_unload_in_progress(VOS_MODULE_ID_VOSS, FALSE);
   hdd_allow_suspend();
   
   //                             
   wlan_hdd_restart_init(pHddCtx);
  
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

err_balstop:
#ifdef ANI_BUS_TYPE_SDIO
#ifndef ANI_MANF_DIAG
       wlan_hdd_enable_deepsleep(pHddCtx->pvosContext);
#endif

   WLANBAL_Stop(pHddCtx->pvosContext);
   WLANBAL_SuspendChip(pHddCtx->pvosContext);
#endif

#ifdef ANI_BUS_TYPE_SDIO
err_salstop:
   WLANSAL_Stop(pHddCtx->pvosContext);

#endif
err_clkvote:
    vos_chipVoteOffXOBuffer(NULL, NULL, NULL);

#ifdef ANI_BUS_TYPE_SDIO
err_balclose:
   WLANBAL_Close(pHddCtx->pvosContext);
#endif //                  

err_wdclose:
   if(pHddCtx->cfg_ini->fIsLogpEnabled)
      vos_watchdog_close(pVosContext);

#ifdef CONFIG_CFG80211
err_wiphy_reg:
   wiphy_unregister(wiphy) ; 
#endif

err_config:
   kfree(pHddCtx->cfg_ini);
   pHddCtx->cfg_ini= NULL;

err_free_hdd_context:
   hdd_allow_suspend();
#ifdef CONFIG_CFG80211
   wiphy_free(wiphy) ;
   //             
#else
   vos_mem_free( pHddCtx );
#endif
   VOS_BUG(1);

   return -1;

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
#ifdef ANI_BUS_TYPE_SDIO
   struct sdio_func *sdio_func_dev = NULL;
   unsigned int attempts = 0;
#endif //                  
   struct device *dev = NULL;
   int ret_status = 0;

   ENTER();

#if (LINUX_VERSION_CODE < KERNEL_VERSION(3,4,5))
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
      return -1;
   }

#ifdef ANI_BUS_TYPE_SDIO
   //                                                                                 
   //                                                                                
   //                                                                                
   do {
      sdio_func_dev = libra_getsdio_funcdev();
      if (NULL == sdio_func_dev) {
         hddLog(VOS_TRACE_LEVEL_FATAL, "%s: Libra WLAN not detected yet.",__func__);
         attempts++;
      }
      else {
         hddLog(VOS_TRACE_LEVEL_FATAL, "%s: Libra WLAN detecton succeeded",__func__);
         dev = &sdio_func_dev->dev;
         break;
      }

      if(attempts == 7)
         break;

      msleep(250);

   }while (attempts < 7);

   //                                                                              
   //                                                   
   if (NULL == sdio_func_dev) {

      attempts = 0;

      //                                  
      vos_chipPowerDown(NULL, NULL, NULL);

      msleep(1000);

      //                                                        
      status = vos_chipPowerUp(NULL,NULL,NULL);
      if (!VOS_IS_STATUS_SUCCESS(status))
      {
         hddLog(VOS_TRACE_LEVEL_FATAL, "%s: Retry Libra WLAN not Powered Up. "
             "exiting", __func__);
         return -1;
      }

      do {
         sdio_func_dev = libra_getsdio_funcdev();
         if (NULL == sdio_func_dev) {
            hddLog(VOS_TRACE_LEVEL_FATAL, "%s: Retry Libra WLAN not detected yet.",__func__);
            attempts++;
         }
         else {
            hddLog(VOS_TRACE_LEVEL_FATAL, "%s: Retry Libra WLAN detecton succeeded",__func__);
            dev = &sdio_func_dev->dev;
            break;
         }

         if(attempts == 2)
           break;

         msleep(1000);

      }while (attempts < 3);
   }

#endif //                  

#ifdef ANI_BUS_TYPE_PCI

   dev = wcnss_wlan_get_device();

#endif //                 

#ifdef ANI_BUS_TYPE_PLATFORM
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

#ifdef ANI_BUS_TYPE_SDIO
   /*              */
   status = WLANSAL_Open(pVosContext, 0);

   if(!VOS_IS_STATUS_SUCCESS(status))
   {
         hddLog(VOS_TRACE_LEVEL_FATAL,"%s: Failed to open SAL", __func__);

      /*                                               */
      vos_preClose( &pVosContext );
         ret_status = -1;
         break;
   }
#endif //                  

#ifndef MODULE
      /*                                                                            
       */
      hdd_set_conparam((v_UINT_t)con_mode);
#endif

      //                            
      if(hdd_wlan_startup(dev)) {
         hddLog(VOS_TRACE_LEVEL_FATAL,"%s: WLAN Driver Initialization failed",
          __func__);
#ifdef ANI_BUS_TYPE_SDIO
         WLANSAL_Close(pVosContext);
#endif //                  
         vos_preClose( &pVosContext );
         ret_status = -1;
         break;
      }

      /*                               
                                                                                       
                                      
       */
      hddLog(VOS_TRACE_LEVEL_ERROR, "In module init: Ensure Force XO Core is OFF"
                                       " when  WLAN is turned ON so Core toggles"
                                       " unless we enter PS\n");
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

#if (LINUX_VERSION_CODE < KERNEL_VERSION(3,4,5))
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
      /*                                                          */
      while(isWDresetInProgress()){
         VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_FATAL, "%s:SSR in Progress; block rmmod for 1 second!!!",__func__);
         msleep(1000);
       }

      pHddCtx->isLoadUnloadInProgress = TRUE;
      vos_set_load_unload_in_progress(VOS_MODULE_ID_VOSS, TRUE);

      //                                                  
      hdd_wlan_exit(pHddCtx);
   }

#ifdef ANI_BUS_TYPE_SDIO
   WLANSAL_Close(pVosContext);
#endif //                  

   vos_preClose( &pVosContext );

#ifdef TIMER_MANAGER
   vos_timer_exit();
#endif
#ifdef MEMORY_DEBUG
   vos_mem_exit();
#endif

done:
#if (LINUX_VERSION_CODE < KERNEL_VERSION(3,4,5))
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
   /*                                   */
   return 0;
}

static int con_mode_handler(const char *kmessage,
                                 struct kernel_param *kp)
{
   return 0;
}
#else /*               */
/*                                                                            

                                                    

                                  
                                                                   
                                                                                
                                

                                               

                                                                            */
static int fwpath_changed_handler(const char *kmessage,
                                 struct kernel_param *kp)
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
static int con_mode_handler(const char *kmessage,
                                 struct kernel_param *kp)
{
   int ret = param_set_int(kmessage, kp);

   if (ret)
       return ret;

   return fwpath_changed_handler(kmessage, kp);
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

void hdd_softap_sta_deauth(hdd_adapter_t *pAdapter, v_U8_t *pDestMacAddress)
{
    v_U8_t STAId;
    v_CONTEXT_t pVosContext = (WLAN_HDD_GET_CTX(pAdapter))->pvosContext;
#ifdef FEATURE_WLAN_NON_INTEGRATED_SOC
    tHalHandle hHalHandle;
#endif

    ENTER();

    hddLog( LOGE, "hdd_softap_sta_deauth:(0x%x, false)", (WLAN_HDD_GET_CTX(pAdapter))->pvosContext);

    //                                     
    if( pDestMacAddress[0] & 0x1 )
       return;

    WLANSAP_DeauthSta(pVosContext,pDestMacAddress);

    /*                  */
#ifdef FEATURE_WLAN_NON_INTEGRATED_SOC
    hHalHandle = (tHalHandle ) vos_get_context(VOS_MODULE_ID_HAL, pVosContext);
    if (eHAL_STATUS_SUCCESS ==
        halTable_FindStaidByAddr(hHalHandle, (tANI_U8 *)pDestMacAddress,
                                 &STAId))
    {
       hdd_softap_DeregisterSTA(pAdapter, STAId);
    }
#else
    if (VOS_STATUS_SUCCESS ==
        hdd_softap_GetStaId(pAdapter, (v_MACADDR_t *)pDestMacAddress,
                            &STAId))
    {
      hdd_softap_DeregisterSTA(pAdapter, STAId);
    }
#endif

    EXIT();
}

/*                                                                            

                                             

                                                            

                                         

                                  

                

                                                                            */

void hdd_softap_sta_disassoc(hdd_adapter_t *pAdapter,v_U8_t *pDestMacAddress)
{
        v_CONTEXT_t pVosContext = (WLAN_HDD_GET_CTX(pAdapter))->pvosContext;

    ENTER();

    hddLog( LOGE, "hdd_softap_sta_disassoc:(0x%x, false)", (WLAN_HDD_GET_CTX(pAdapter))->pvosContext);

    //                                       
    if( pDestMacAddress[0] & 0x1 )
       return;

    WLANSAP_DisassocSta(pVosContext,pDestMacAddress);
}

void hdd_softap_tkip_mic_fail_counter_measure(hdd_adapter_t *pAdapter,v_BOOL_t enable)
{
    v_CONTEXT_t pVosContext = (WLAN_HDD_GET_CTX(pAdapter))->pvosContext;

    ENTER();

    hddLog( LOGE, "hdd_softap_tkip_mic_fail_counter_measure:(0x%x, false)", (WLAN_HDD_GET_CTX(pAdapter))->pvosContext);

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
    hddLog(LOGE, "%s: Invalid context", __FUNCTION__);
    return VOS_STA;
}

/*                                                      
                                                          
                                         */
v_BOOL_t hdd_is_apps_power_collapse_allowed(hdd_context_t* pHddCtx)
{
    tPmcState pmcState = pmcGetPmcState(pHddCtx->hHal);
    hdd_config_t *pConfig = pHddCtx->cfg_ini;
    hdd_adapter_list_node_t *pAdapterNode = NULL, *pNext = NULL; 
    hdd_adapter_t *pAdapter = NULL; 
    VOS_STATUS status;

#ifdef WLAN_SOFTAP_FEATURE
    if (VOS_STA_SAP_MODE == hdd_get_conparam())
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
            if ((pConfig->fIsImpsEnabled || pConfig->fIsBmpsEnabled)
                 && (pmcState != IMPS && pmcState != BMPS
                 &&  pmcState != STOPPED && pmcState != STANDBY))
            {
                return FALSE;
            }
        }
        status = hdd_get_next_adapter ( pHddCtx, pAdapterNode, &pNext );
        pAdapterNode = pNext;
    }
    return TRUE;
}

void wlan_hdd_set_concurrency_mode(hdd_context_t *pHddCtx, tVOS_CON_MODE mode)
{
   switch(mode)
   {
       case WLAN_HDD_INFRA_STATION:
#ifdef WLAN_FEATURE_P2P
       case WLAN_HDD_P2P_CLIENT:
       case WLAN_HDD_P2P_GO:
#endif
       case WLAN_HDD_SOFTAP:
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
       case WLAN_HDD_INFRA_STATION:
#ifdef WLAN_FEATURE_P2P
       case WLAN_HDD_P2P_CLIENT:
       case WLAN_HDD_P2P_GO:
#endif
       case WLAN_HDD_SOFTAP:
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
          hddLog(LOGE, FL("Failed to stop HDD restart timer\n"));
   vos_status = vos_timer_destroy(&pHddCtx->hdd_restart_timer);
   if (!VOS_IS_STATUS_SUCCESS(vos_status))
          hddLog(LOGE, FL("Failed to destroy HDD restart timer\n"));

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
            "%s: memory allocatoin failed (%d bytes)", __func__, len);
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

   /*             */
   status = wlan_hdd_framework_restart(pHddCtx);
   
   return status;
}


//                                       
module_init(hdd_module_init);
module_exit(hdd_module_exit);

MODULE_LICENSE("Dual BSD/GPL");
MODULE_AUTHOR("Qualcomm Atheros, Inc.");
MODULE_DESCRIPTION("WLAN HOST DEVICE DRIVER");

module_param_call(con_mode, con_mode_handler, param_get_int, &con_mode,
                    S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);

module_param_call(fwpath, fwpath_changed_handler, param_get_string, fwpath,
                    S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
