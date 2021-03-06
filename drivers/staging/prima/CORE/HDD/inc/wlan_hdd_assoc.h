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

#if !defined( HDD_CONNECTION_H__ ) 
#define HDD_CONNECTION_H__ 
#include <wlan_hdd_mib.h>
#define HDD_MAX_NUM_IBSS_STA ( 4 )
#define TKIP_COUNTER_MEASURE_STARTED 1
#define TKIP_COUNTER_MEASURE_STOPED  0 
/*                                                           */
#define ASSOC_LINKUP_TIMEOUT 60
typedef enum 
{
   /*                                                                       */
   eConnectionState_NotConnected,
   /*                                          */
   eConnectionState_Associated,

   /*                                                                           
                    */
   eConnectionState_IbssDisconnected,

   /*                                                                     */
   eConnectionState_IbssConnected,

   /*                                             */
   eConnectionState_Disconnecting
	
}eConnectionState;
/*                                                  */
typedef struct connection_info_s
{
   /*                              */
   eConnectionState connState;
   
   /*                                                                 
                                                                                
                                 */
      
   eMib_dot11DesiredBssType connDot11DesiredBssType;
   /*        */
   tCsrBssid bssId;
   
   /*           */
   tCsrSSIDInfo SSID;
   
   /*             */
   v_U8_t staId[ HDD_MAX_NUM_IBSS_STA ];
   /*                                        */
   v_MACADDR_t peerMacAddress[ HDD_MAX_NUM_IBSS_STA ];         
   /*            */
   eCsrAuthType   authType;

   /*                          */
   eCsrEncryptionType ucEncryptionType;

   /*                            */
   eCsrEncryptionType mcEncryptionType;

   /*       */
   tCsrKeys Keys;

   /*                     */
   v_U8_t operationChannel; 
   
    /*                                */
   v_U8_t uIsAuthenticated;
   
}connection_info_t;
/*                              */
typedef struct hdd_adapter_s hdd_adapter_t;
typedef struct hdd_context_s hdd_context_t;
typedef struct hdd_station_ctx hdd_station_ctx_t;
typedef struct hdd_ap_ctx_s  hdd_ap_ctx_t;
#ifdef CONFIG_CFG80211   
typedef struct hdd_mon_ctx_s  hdd_mon_ctx_t;
#endif


extern v_BOOL_t hdd_connIsConnected( hdd_station_ctx_t *pHddStaCtx );
extern eHalStatus hdd_smeRoamCallback( void *pContext, tCsrRoamInfo *pRoamInfo, v_U32_t roamId, 
                                eRoamCmdStatus roamStatus, eCsrRoamResult roamResult );

extern v_VOID_t hdd_connSaveConnectInfo( hdd_adapter_t *pAdapter, tCsrRoamInfo *pRoamInfo, eCsrRoamBssType eBssType );

inline v_BOOL_t hdd_connGetConnectedBssType( hdd_station_ctx_t *pHddCtx, 
        eMib_dot11DesiredBssType *pConnectedBssType );

int hdd_SetGENIEToCsr( hdd_adapter_t *pAdapter, eCsrAuthType *RSNAuthType );

int hdd_set_csr_auth_type( hdd_adapter_t *pAdapter, eCsrAuthType RSNAuthType );
#endif
