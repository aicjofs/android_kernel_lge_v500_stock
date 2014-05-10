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

#if !defined( WLAN_HDD_MAIN_H )
#define WLAN_HDD_MAIN_H
/**===========================================================================
  
  \file  WLAN_HDD_MAIN_H.h
  
  \brief Linux HDD Adapter Type
         Copyright 2008 (c) Qualcomm, Incorporated.
         All Rights Reserved.
         Qualcomm Confidential and Proprietary.
  
  ==========================================================================*/
  
/*                                                                            
               
                                                                           */ 
  
#include <linux/netdevice.h>
#include <linux/skbuff.h>
#ifdef CONFIG_CFG80211
#include <net/cfg80211.h>
#endif
#include <vos_list.h>
#include <vos_types.h>
#include "sirMacProtDef.h"
#include "csrApi.h"
#include <wlan_hdd_assoc.h>
#include <wlan_hdd_dp_utils.h>
#include <wlan_hdd_wmm.h>
#include <wlan_hdd_cfg.h>
#include <linux/spinlock.h>
#ifdef WLAN_FEATURE_HOLD_RX_WAKELOCK
#include <linux/wakelock.h>
#endif
#ifdef ANI_MANF_DIAG
#include <wlan_hdd_ftm.h>
#endif
/*                                                                            
                                        
                                                                           */
/*                                           */
#define LIBRA_CARD_INSERT_DETECT_MAX_COUNT      5
#define LIBRA_CARD_REMOVE_DETECT_MAX_COUNT      5
/*                      */  
#define NUM_TX_QUEUES 4
/*                                                 */
#define NET_DEV_TX_QUEUE_LEN 100
/*                                                           */
#define HDD_TX_QUEUE_MAX_LEN 128
/*                                                                      
                                                                     
                               */
#define HDD_TX_QUEUE_LOW_WATER_MARK (HDD_TX_QUEUE_MAX_LEN*3/4)
/*                                   */
#define LIBRA_HW_NEEDED_HEADROOM   128
/*                        */
#ifdef LIBRA_LINUX_PC
#define HDD_TX_TIMEOUT          (8000)       
#else
#define HDD_TX_TIMEOUT          msecs_to_jiffies(5000)    
#endif
/*                  */
#define HDD_DEFAULT_MTU         (1500)
/*                                  */
#define NET_DEVICE_REGISTERED  1<<0
#define SME_SESSION_OPENED     1<<1
#define INIT_TX_RX_SUCCESS     1<<2
#define WMM_INIT_DONE          1<<3
#define SOFTAP_BSS_STARTED     1<<4
#define DEVICE_IFACE_OPENED    1<<5

/*                                                      */
#define WLAN_WAIT_TIME_DISCONNECT  500
#define WLAN_WAIT_TIME_STATS       800
#define WLAN_WAIT_TIME_POWER       800
/*                                                       
                                                                       
                         */
#define WLAN_WAIT_TIME_SESSIONOPENCLOSE  15000
#define WLAN_WAIT_TIME_ABORTSCAN  2000

/*                  */
#define WLAN_WAIT_TIME_SCAN_REQ 100

#define MAX_NUMBER_OF_ADAPTERS 4

#define MAX_CFG_STRING_LEN  255

#define MAC_ADDR_ARRAY(a) (a)[0], (a)[1], (a)[2], (a)[3], (a)[4], (a)[5]
/*                      */
#define MAC_ADDRESS_STR "%02x:%02x:%02x:%02x:%02x:%02x"
#define MAX_GENIE_LEN 255

#if defined(QC_WLAN_CHIPSET_PRIMA)
#define WLAN_CHIP_VERSION   "WCNSS"
#elif defined(ANI_CHIPSET_LIBRA)
#define WLAN_CHIP_VERSION   "WCN1312"
#elif defined(ANI_CHIPSET_VOLANS)
#define WLAN_CHIP_VERSION   "WCN1314"
#else
#define WLAN_CHIP_VERSION   "UNKNOWN"
#endif

#define hddLog(level, args...) VOS_TRACE( VOS_MODULE_ID_HDD, level, ## args)
#define ENTER() VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_INFO, "Enter:%s", __FUNCTION__)
#define EXIT()  VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_INFO, "Exit:%s", __FUNCTION__)

#define WLAN_HDD_GET_PRIV_PTR(__dev__) (hdd_adapter_t*)(netdev_priv((__dev__)))

#define MAX_EXIT_ATTEMPTS_DURING_LOGP 20

#define MAX_NO_OF_2_4_CHANNELS 14

#define WLAN_HDD_PUBLIC_ACTION_FRAME 4
#define WLAN_HDD_PUBLIC_ACTION_FRAME_OFFSET 24
#define WLAN_HDD_PUBLIC_ACTION_FRAME_TYPE_OFFSET 30
#define WLAN_HDD_P2P_SOCIAL_CHANNELS 3

#ifdef WLAN_FEATURE_HOLD_RX_WAKELOCK
#define HDD_WAKE_LOCK_DURATION 50
#endif

typedef struct hdd_tx_rx_stats_s
{
   //                 
   __u32    txXmitCalled;
   __u32    txXmitDropped;
   __u32    txXmitBackPressured;
   __u32    txXmitQueued;
   __u32    txXmitClassifiedAC[NUM_TX_QUEUES];
   __u32    txXmitDroppedAC[NUM_TX_QUEUES];
   __u32    txXmitBackPressuredAC[NUM_TX_QUEUES];
   __u32    txXmitQueuedAC[NUM_TX_QUEUES];
   //                
   __u32    txFetched;
   __u32    txFetchedAC[NUM_TX_QUEUES];
   __u32    txFetchEmpty;
   __u32    txFetchLowResources;
   __u32    txFetchDequeueError;
   __u32    txFetchDequeued;
   __u32    txFetchDequeuedAC[NUM_TX_QUEUES];
   __u32    txFetchDePressured;
   __u32    txFetchDePressuredAC[NUM_TX_QUEUES];
   //                   
   __u32    txCompleted;
   //            
   __u32    txFlushed;
   __u32    txFlushedAC[NUM_TX_QUEUES];
   //         
   __u32    rxChains;
   __u32    rxPackets;
   __u32    rxDropped;
   __u32    rxDelivered;
   __u32    rxRefused;
} hdd_tx_rx_stats_t;

typedef struct hdd_chip_reset_stats_s
{
   __u32    totalLogpResets;
   __u32    totalCMD53Failures;
   __u32    totalMutexReadFailures;
   __u32    totalMIFErrorFailures;
   __u32    totalFWHearbeatFailures;
   __u32    totalUnknownExceptions;
} hdd_chip_reset_stats_t;

typedef struct hdd_stats_s
{
   tCsrSummaryStatsInfo       summary_stat;
   tCsrGlobalClassAStatsInfo  ClassA_stat;
   tCsrGlobalClassBStatsInfo  ClassB_stat;
   tCsrGlobalClassCStatsInfo  ClassC_stat;
   tCsrGlobalClassDStatsInfo  ClassD_stat;
   tCsrPerStaStatsInfo        perStaStats;
   hdd_tx_rx_stats_t          hddTxRxStats;
   hdd_chip_reset_stats_t     hddChipResetStats;
} hdd_stats_t;

typedef enum
{
   HDD_ROAM_STATE_NONE,
   
   //                                                                
   HDD_ROAM_STATE_DISCONNECTING_POWER,
   
   //                                                                   
   //                                                                       
   //                                                               
   HDD_ROAM_STATE_SETTING_KEY,
} HDD_ROAM_STATE;

typedef enum
{
   eHDD_SUSPEND_NONE = 0,
   eHDD_SUSPEND_DEEP_SLEEP,
   eHDD_SUSPEND_STANDBY,
} hdd_ps_state_t;

typedef struct roaming_info_s
{
   HDD_ROAM_STATE roamingState;
   vos_event_t roamingEvent;
   
} roaming_info_t;

#ifdef FEATURE_WLAN_WAPI
/*                                              */
#define MAX_WPI_KEY_LENGTH    16
#define MAX_NUM_PN            16
#define MAC_ADDR_LEN           6
#define MAX_ADDR_INDEX        12
#define MAX_NUM_AKM_SUITES    16
#define MAX_NUM_UNI_SUITES    16
#define MAX_NUM_BKIDS         16
#define HDD_PAIRWISE_WAPI_KEY 0
#define HDD_GROUP_WAPI_KEY    1

/*                            */
enum _WAPIAuthMode
{
   WAPI_AUTH_MODE_OPEN = 0,
   WAPI_AUTH_MODE_PSK = 1,
   WAPI_AUTH_MODE_CERT
} __attribute__((packed));
typedef enum _WAPIAuthMode WAPIAuthMode;

/*                                      */
#define   WZC_ORIGINAL      0
#define   WAPI_EXTENTION    1

struct _WAPI_FUNCTION_MODE
{
   unsigned char wapiMode;
}__attribute__((packed));

typedef struct _WAPI_FUNCTION_MODE WAPI_FUNCTION_MODE;

typedef struct _WAPI_BKID
{
   v_U8_t   bkid[16];
}WAPI_BKID, *pWAPI_BKID;

/*                                                    */
struct _WAPI_AssocInfo
{
   v_U8_t      elementID;
   v_U8_t      length;
   v_U16_t     version;
   v_U16_t     akmSuiteCount;
   v_U32_t     akmSuite[MAX_NUM_AKM_SUITES];
   v_U16_t     unicastSuiteCount;
   v_U32_t     unicastSuite[MAX_NUM_UNI_SUITES];
   v_U32_t     multicastSuite;
   v_U16_t     wapiCability;
   v_U16_t     bkidCount;
   WAPI_BKID   bkidList[MAX_NUM_BKIDS];
} __attribute__((packed));

typedef struct _WAPI_AssocInfo WAPI_AssocInfo;
typedef struct _WAPI_AssocInfo *pWAPI_IEAssocInfo;

/*                           */
enum _WAPIKeyType
{
   PAIRWISE_KEY, // 
   GROUP_KEY     // 
}__attribute__((packed));
typedef enum _WAPIKeyType WAPIKeyType;

/*                                */
enum _KEY_DIRECTION
{
   None,
   Rx,
   Tx,
   Rx_Tx
}__attribute__((packed));

typedef enum _KEY_DIRECTION WAPI_KEY_DIRECTION;

/*                               */
struct WLAN_WAPI_KEY
{
   WAPIKeyType     keyType;
   WAPI_KEY_DIRECTION   keyDirection;  /*                       */
   v_U8_t          keyId;
   v_U8_t          addrIndex[MAX_ADDR_INDEX]; /*                       */
   int             wpiekLen;
   v_U8_t          wpiek[MAX_WPI_KEY_LENGTH];
   int             wpickLen;
   v_U8_t          wpick[MAX_WPI_KEY_LENGTH];
   v_U8_t          pn[MAX_NUM_PN];        /*                       */
}__attribute__((packed));

typedef struct WLAN_WAPI_KEY WLAN_WAPI_KEY;
typedef struct WLAN_WAPI_KEY *pWLAN_WAPI_KEY;

#define WPA_GET_LE16(a) ((u16) (((a)[1] << 8) | (a)[0]))
#define WLAN_EID_WAPI 68
#define WAPI_PSK_AKM_SUITE  0x02721400
#define WAPI_CERT_AKM_SUITE 0x01721400

/*                                     */
struct _WLAN_BKID_LIST
{
   v_U32_t          length;
   v_U32_t          BKIDCount;
   WAPI_BKID        BKID[1];
}__attribute__((packed));

typedef struct _WLAN_BKID_LIST WLAN_BKID_LIST;
typedef struct _WLAN_BKID_LIST *pWLAN_BKID_LIST;


/*                                       */
struct hdd_wapi_info_s
{
   v_U32_t     nWapiMode;
   v_BOOL_t    fIsWapiSta;
   v_MACADDR_t cachedMacAddr;
   v_UCHAR_t   wapiAuthMode;
}__attribute__((packed));
typedef struct hdd_wapi_info_s hdd_wapi_info_t;
#endif /*                   */

#ifdef CONFIG_CFG80211
typedef struct beacon_data_s {
    u8 *head, *tail;
    int head_len, tail_len;
    int dtim_period;
} beacon_data_t;
#endif

typedef enum device_mode
{  /*                                                 */
   WLAN_HDD_INFRA_STATION,
   WLAN_HDD_SOFTAP,
   WLAN_HDD_P2P_CLIENT,
   WLAN_HDD_P2P_GO,
   WLAN_HDD_MONITOR
#ifdef ANI_MANF_DIAG
   ,WLAN_HDD_FTM,
#endif
   WLAN_HDD_P2P_DEVICE
}device_mode_t;

typedef enum rem_on_channel_request_type
{
   REMAIN_ON_CHANNEL_REQUEST,
   OFF_CHANNEL_ACTION_TX,
}rem_on_channel_request_type_t;

/*                                    */
typedef enum
{
   WLAN_HDD_TM_LEVEL_0,
   WLAN_HDD_TM_LEVEL_1,
   WLAN_HDD_TM_LEVEL_2,
   WLAN_HDD_TM_LEVEL_3,
   WLAN_HDD_TM_LEVEL_4,
   WLAN_HDD_TM_LEVEL_MAX
} WLAN_TmLevelEnumType;

/*                                                           */
typedef struct
{
   v_BOOL_t  ampduEnable;
   v_BOOL_t  enterImps;
   v_U32_t   txSleepDuration;
   v_U32_t   txOperationDuration;
   v_U32_t   txBlockFrameCountThreshold;
} hdd_tmLevelAction_t;

/*                                              */
typedef struct
{
   WLAN_TmLevelEnumType currentTmLevel;
   hdd_tmLevelAction_t  tmAction;
   vos_timer_t          txSleepTimer;
   struct mutex         tmOperationLock;
   vos_event_t          setTmDoneEvent;
   v_U32_t              txFrameCount;
   v_TIME_t             lastblockTs;
   v_TIME_t             lastOpenTs;
   struct netdev_queue *blockedQueue;
} hdd_thermal_mitigation_info_t;

#if defined CONFIG_CFG80211
typedef struct hdd_remain_on_chan_ctx
{
  struct net_device *dev;
  struct ieee80211_channel chan;
  enum nl80211_channel_type chan_type;
  unsigned int duration;
  u64 cookie;
  rem_on_channel_request_type_t rem_on_chan_request;
}hdd_remain_on_chan_ctx_t;

typedef enum{
    HDD_IDLE,
    HDD_PD_REQ_ACK_PENDING,
    HDD_GO_NEG_REQ_ACK_PENDING,
    HDD_INVALID_STATE,
}eP2PActionFrameState;

typedef enum {
    WLAN_HDD_GO_NEG_REQ,
    WLAN_HDD_GO_NEG_RESP,
    WLAN_HDD_GO_NEG_CNF,
    WLAN_HDD_INVITATION_REQ,
    WLAN_HDD_INVITATION_RESP,
    WLAN_HDD_DEV_DIS_REQ,
    WLAN_HDD_DEV_DIS_RESP,
    WLAN_HDD_PROV_DIS_REQ,
    WLAN_HDD_PROV_DIS_RESP,
}tActionFrmType;

typedef struct hdd_cfg80211_state_s 
{
  tANI_U16 current_freq;
  u64 action_cookie;
  tANI_U8 *buf;
  size_t len;
  struct sk_buff *skb;
  hdd_remain_on_chan_ctx_t* remain_on_chan_ctx;
  eP2PActionFrameState actionFrmState;
}hdd_cfg80211_state_t;

#endif
struct hdd_station_ctx
{
  /*                                         */
   hdd_wext_state_t WextState;

   /*                       */
   connection_info_t conn_info;

   roaming_info_t roam_info;

   v_BOOL_t bSendDisconnect;

#if  defined (WLAN_FEATURE_VOWIFI_11R) || defined (FEATURE_WLAN_CCX) || defined(FEATURE_WLAN_LFR)
   int     ft_carrier_on;
#endif
};

#define BSS_STOP    0 
#define BSS_START   1
typedef struct hdd_hostapd_state_s
{
    int bssState;
    vos_event_t vosEvent;
    VOS_STATUS vosStatus;
    v_BOOL_t bCommit; 

} hdd_hostapd_state_t;


/*
                                                                            
*/
typedef struct {
    /*                                    */
    v_BOOL_t isUsed;

    /*                                                            
                                                                  */
    v_U8_t ucSTAId;

    /*                             */
    v_MACADDR_t macAddrSTA;

    /*                                                            
                                                               */
    WLANTL_STAStateType tlSTAState;

   /*                                              */
   hdd_list_t wmm_tx_queue[NUM_TX_QUEUES];

   /*                                                             */
   v_U16_t aTxQueueDepth[NUM_TX_QUEUES];
   
   /*                                             */
   v_BOOL_t txSuspended[NUM_TX_QUEUES];

   /*                              */
   v_BOOL_t isQosEnabled;

} hdd_station_info_t;

struct hdd_ap_ctx_s
{
   hdd_hostapd_state_t HostapdState;

   //                                       
   //                             
   //                    
   //                     
   //                                    

   /*                                                        */
   //                        

   /*                                       */
   v_U8_t uBCStaId;

   v_U8_t uPrivacy;  //                                  
   
#ifdef WLAN_SOFTAP_FEATURE   
   tSirWPSPBCProbeReq WPSPBCProbeReq;
   
   tsap_Config_t sapConfig;
#endif
   
   struct semaphore semWpsPBCOverlapInd;
   
   v_BOOL_t apDisableIntraBssFwd;
      
   vos_timer_t hdd_ap_inactivity_timer;

   v_U8_t   operatingChannel;
   
   v_BOOL_t uIsAuthenticated;

   eCsrEncryptionType ucEncryptType;
   
#ifdef CONFIG_CFG80211   
   //                                                                       
   tCsrRoamSetKey groupKey; 
   //                                                                
   tCsrRoamSetKey wepKey[CSR_MAX_NUM_KEY];

   beacon_data_t *beacon;
#endif
};

#ifdef CONFIG_CFG80211   
struct hdd_mon_ctx_s
{
   hdd_adapter_t *pAdapterForTx;
};
#endif

typedef struct hdd_scaninfo_s
{
   /*              */
   v_U32_t scanId; 

   /*                   */
   v_U32_t mScanPending;

   /*                         */
   v_U32_t waitScanResult;

#ifdef WLAN_FEATURE_P2P
  v_BOOL_t flushP2pScanResults;
#endif

   /*                        */
   tSirAddie scanAddIE; 

   /*          */
   tSirScanType scan_mode;

   /*                       */
   struct completion scan_req_completion_event;

   vos_event_t scan_finished_event;

   hdd_scan_pending_option_e scan_pending_option;

}hdd_scaninfo_t;

#define WLAN_HDD_ADAPTER_MAGIC 0x574c414e //            
struct hdd_adapter_s
{
   void *pHddCtx;

   device_mode_t device_mode; 

   /*                               */
   struct net_device *dev;
    
   //                         
#ifdef CONFIG_CFG80211
   struct wireless_dev wdev ;
   struct cfg80211_scan_request *request ; 
#endif

#ifdef WLAN_FEATURE_P2P
   /*                                                         
                                                                  
                                                                 
                              
    */
    tANI_U8  ops;
    tANI_U32 ctw;
#endif

   /*                                       */       
   v_MACADDR_t macAddressCurrent;    
      
   /*            */
   unsigned long event_flags;

   /*                        */
   struct net_device_stats stats;
   /*                */
   hdd_stats_t hdd_stats;
   /*                */
   sHddMib_t  hdd_mib;
           
   tANI_U8 sessionId;

   /*                                       */
   struct completion session_close_comp_var;

   /*                                      */
   struct completion session_open_comp_var;

   //                                                         
   /*                                              */
   struct completion disconnect_comp_var;

   /*                                    */
   struct completion change_country_code;

   /*                                      */
   struct completion linkup_event_var;


   /*                                   */
   struct completion abortscan_event_var;
#ifdef CONFIG_CFG80211
   /*                                                        */
   struct completion cancel_rem_on_chan_var;

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,38))
   /*                                                              */
   struct completion offchannel_tx_event;
#endif
   /*                                      */
   struct completion tx_action_cnf_event;
   /*                                                 */
   struct completion rem_on_chan_ready_event;
#endif

   /*                                              */
   v_BOOL_t isLinkUpSvcNeeded;

/*                                                            
             
 */
   /*                                             */
   hdd_list_t wmm_tx_queue[NUM_TX_QUEUES];
   /*                                             */
   v_BOOL_t isVosOutOfResource;
  
   /*                                             */
   v_BOOL_t isTxSuspended[NUM_TX_QUEUES];

   /*             */
   hdd_wmm_status_t hddWmmStatus;
/*                                                            
 */
/*                                                            
                         
 */
    /*                            */
   /*                        */
   spinlock_t staInfo_lock; //                                   
   hdd_station_info_t aStaInfo[WLAN_MAX_STA_COUNT];
   //                         

   v_U16_t aTxQueueLimit[NUM_TX_QUEUES];
/*                                                            
 */

#ifdef FEATURE_WLAN_WAPI
   hdd_wapi_info_t wapi_info;
#endif
   
   v_S7_t rssi;
#ifdef CONFIG_CFG80211
   struct work_struct  monTxWorkQueue;
   struct sk_buff *skb_to_tx;
#endif

   union {
      hdd_station_ctx_t station;
      hdd_ap_ctx_t  ap;
#ifdef CONFIG_CFG80211   
      hdd_mon_ctx_t monitor;
#endif
   }sessionCtx;

   hdd_scaninfo_t scan_info;
#ifdef CONFIG_CFG80211
   hdd_cfg80211_state_t cfg80211State;
#endif

   //                                            
   v_U32_t magic;
   v_BOOL_t higherDtimTransition;
};

typedef struct hdd_dynamic_mcbcfilter_s
{
    v_BOOL_t     enableCfg;
    v_U8_t       mcastBcastFilterSetting;
    v_BOOL_t     enableSuspend;
    v_U8_t       mcBcFilterSuspend;
}hdd_dynamic_mcbcfilter_t;

#define WLAN_HDD_GET_STATION_CTX_PTR(pAdapter) &(pAdapter)->sessionCtx.station
#define WLAN_HDD_GET_AP_CTX_PTR(pAdapter) &(pAdapter)->sessionCtx.ap
#define WLAN_HDD_GET_WEXT_STATE_PTR(pAdapter)  &(pAdapter)->sessionCtx.station.WextState
#define WLAN_HDD_GET_CTX(pAdapter) (hdd_context_t*)pAdapter->pHddCtx
#define WLAN_HDD_GET_HAL_CTX(pAdapter)  ((hdd_context_t*)(pAdapter->pHddCtx))->hHal
#define WLAN_HDD_GET_HOSTAP_STATE_PTR(pAdapter) &(pAdapter)->sessionCtx.ap.HostapdState
#define WLAN_HDD_GET_CFG_STATE_PTR(pAdapter)  &(pAdapter)->cfg80211State
#define WLAN_HDD_MAX_MC_ADDR_LIST 10

typedef struct hdd_adapter_list_node
{
   hdd_list_node_t node;     //                      
   hdd_adapter_t *pAdapter;
}hdd_adapter_list_node_t;

typedef struct hdd_priv_data_s
{
   tANI_U8 *buf;
   int used_len;
   int total_len;
}hdd_priv_data_t;

#ifdef WLAN_FEATURE_PACKET_FILTERING
typedef struct multicast_addr_list
{
   v_U8_t isFilterApplied;
   v_U8_t mc_cnt;
   v_U8_t addr[WLAN_HDD_MAX_MC_ADDR_LIST][ETH_ALEN];
} t_multicast_add_list;
#endif

/*                              */

struct hdd_context_s
{
   /*                      */
   v_CONTEXT_t pvosContext;

   /*               */
   tHalHandle hHal;

#ifdef CONFIG_CFG80211
   struct wiphy *wiphy ;
#endif
   //                           

   hdd_list_t hddAdapters; //                
   /*                                                            */
   hdd_adapter_t *sta_to_adapter[WLAN_MAX_STA_COUNT + 3]; //                                 

   /*                                  */
   const struct firmware *fw;
   
   /*                                 */
   const struct firmware *cfg;
   
   /*                      */
   const struct firmware *nv;
   
   /*                               */
   struct device *parent_dev;

   pid_t  pid_sdio_claimed;
   atomic_t sdio_claim_count;

   /*                                            */ 
   hdd_config_t *cfg_ini;
  #ifdef ANI_MANF_DIAG
   wlan_hdd_ftm_status_t ftm; 
  #endif
   /*                                              */
   struct completion full_pwr_comp_var;
   /*                                                */
   struct completion req_bmps_comp_var;
   
   /*                                           */
   struct completion standby_comp_var;
   
#ifdef FEATURE_WLAN_INTEGRATED_SOC
   /*                                                      */
   struct completion rx_sus_event_var;
#endif //                            

   /*                                                      */
   struct completion tx_sus_event_var;

   /*                                                      */
   struct completion mc_sus_event_var;


   v_BOOL_t isWlanSuspended;

   v_BOOL_t isTxThreadSuspended;

   v_BOOL_t isMcThreadSuspended;

#ifdef FEATURE_WLAN_INTEGRATED_SOC
   v_BOOL_t isRxThreadSuspended;
#endif

   volatile v_BOOL_t isLogpInProgress;

   v_BOOL_t isLoadUnloadInProgress;
   
   /*                                         */
   hdd_ps_state_t hdd_ps_state;
   
   /*                                             */
   v_BOOL_t hdd_mcastbcast_filter_set;

   /*                                     */
   v_BOOL_t hdd_ignore_dtim_enabled;
   v_U32_t hdd_actual_ignore_DTIM_value;
   v_U32_t hdd_actual_LI_value; 

   
   v_BOOL_t hdd_wlan_suspended;
   
   spinlock_t filter_lock;
   
   /*                                                  */
   struct mutex sap_lock;
   
   /*                */
   v_SINT_t ptt_pid;

   v_U8_t change_iface;

   /*                        */
   tVOS_CONCURRENCY_MODE concurrency_mode;

   v_U16_t no_of_sessions[VOS_MAX_NO_OF_MODE];

   hdd_chip_reset_stats_t hddChipResetStats;
   /*                                */
   v_U32_t  hddRivaResetStats;
   
   hdd_dynamic_mcbcfilter_t dynamic_mcbc_filter;
   
   /*                                      */
   v_BOOL_t isAmpAllowed;
   
#ifdef WLAN_FEATURE_P2P
   /*                                          */
   v_MACADDR_t p2pDeviceAddress;
#endif

   /*                                */
   hdd_thermal_mitigation_info_t tmInfo;
#ifdef WLAN_FEATURE_PACKET_FILTERING
   t_multicast_add_list mc_addr_list;
#endif

#ifdef WLAN_FEATURE_HOLD_RX_WAKELOCK
   struct wake_lock rx_wake_lock;
#endif

   /* 
                                            
                                                 
                                                 
                                             
     
    */
   vos_timer_t hdd_restart_timer;
   atomic_t isRestartInProgress;
   u_int8_t hdd_restart_retries;
   
};



/*                                                                            
                                        
                                                                           */ 
VOS_STATUS hdd_get_front_adapter( hdd_context_t *pHddCtx,
                                  hdd_adapter_list_node_t** ppAdapterNode);

VOS_STATUS hdd_get_next_adapter( hdd_context_t *pHddCtx,
                                 hdd_adapter_list_node_t* pAdapterNode,
                                 hdd_adapter_list_node_t** pNextAdapterNode);

VOS_STATUS hdd_remove_adapter( hdd_context_t *pHddCtx,
                               hdd_adapter_list_node_t* pAdapterNode);

VOS_STATUS hdd_remove_front_adapter( hdd_context_t *pHddCtx,
                                     hdd_adapter_list_node_t** ppAdapterNode);

VOS_STATUS hdd_add_adapter_back( hdd_context_t *pHddCtx,
                                 hdd_adapter_list_node_t* pAdapterNode);

VOS_STATUS hdd_add_adapter_front( hdd_context_t *pHddCtx,
                                  hdd_adapter_list_node_t* pAdapterNode);

hdd_adapter_t* hdd_open_adapter( hdd_context_t *pHddCtx, tANI_U8 session_type,
                                 char* name, tSirMacAddr macAddr, tANI_U8 rtnl_held );
VOS_STATUS hdd_close_adapter( hdd_context_t *pHddCtx, hdd_adapter_t *pAdapter, tANI_U8 rtnl_held );
VOS_STATUS hdd_close_all_adapters( hdd_context_t *pHddCtx );
VOS_STATUS hdd_stop_all_adapters( hdd_context_t *pHddCtx );
VOS_STATUS hdd_reset_all_adapters( hdd_context_t *pHddCtx );
VOS_STATUS hdd_start_all_adapters( hdd_context_t *pHddCtx );
VOS_STATUS hdd_reconnect_all_adapters( hdd_context_t *pHddCtx );
hdd_adapter_t * hdd_get_adapter_by_name( hdd_context_t *pHddCtx, tANI_U8 *name );
hdd_adapter_t * hdd_get_adapter_by_macaddr( hdd_context_t *pHddCtx, tSirMacAddr macAddr );
hdd_adapter_t * hdd_get_mon_adapter( hdd_context_t *pHddCtx );
VOS_STATUS hdd_init_station_mode( hdd_adapter_t *pAdapter );
hdd_adapter_t * hdd_get_adapter( hdd_context_t *pHddCtx, device_mode_t mode );
void hdd_deinit_adapter( hdd_context_t *pHddCtx, hdd_adapter_t *pAdapter );
VOS_STATUS hdd_stop_adapter( hdd_context_t *pHddCtx, hdd_adapter_t *pAdapter );
void hdd_set_station_ops( struct net_device *pWlanDev );
tANI_U8* wlan_hdd_get_intf_addr(hdd_context_t* pHddCtx);
void wlan_hdd_release_intf_addr(hdd_context_t* pHddCtx, tANI_U8* releaseAddr);
v_U8_t hdd_get_operating_channel( hdd_context_t *pHddCtx, device_mode_t mode );


#if defined(WLAN_SOFTAP_FEATURE) || defined(ANI_MANF_DIAG)
void hdd_set_conparam ( v_UINT_t newParam );
tVOS_CON_MODE hdd_get_conparam( void );
#endif

void wlan_hdd_enable_deepsleep(v_VOID_t * pVosContext);
v_BOOL_t hdd_is_apps_power_collapse_allowed(hdd_context_t* pHddCtx);
void hdd_abort_mac_scan(hdd_context_t *pHddCtx);
#ifdef CONFIG_CFG80211
void wlan_hdd_set_monitor_tx_adapter( hdd_context_t *pHddCtx, hdd_adapter_t *pAdapter );
void hdd_cleanup_actionframe( hdd_context_t *pHddCtx, hdd_adapter_t *pAdapter );
#endif
void wlan_hdd_set_concurrency_mode(hdd_context_t *pHddCtx, tVOS_CON_MODE mode);
void wlan_hdd_clear_concurrency_mode(hdd_context_t *pHddCtx, tVOS_CON_MODE mode);
void wlan_hdd_reset_prob_rspies(hdd_adapter_t* pHostapdAdapter);
void hdd_prevent_suspend(void);
void hdd_allow_suspend(void);
v_U8_t hdd_is_ssr_required(void);
void hdd_set_ssr_required(v_U8_t value);

VOS_STATUS hdd_enable_bmps_imps(hdd_context_t *pHddCtx);
VOS_STATUS hdd_disable_bmps_imps(hdd_context_t *pHddCtx, tANI_U8 session_type);

eHalStatus hdd_smeCloseSessionCallback(void *pContext);
VOS_STATUS wlan_hdd_restart_driver(hdd_context_t *pHddCtx);
void hdd_exchange_version_and_caps(hdd_context_t *pHddCtx);
#endif    //                                    
