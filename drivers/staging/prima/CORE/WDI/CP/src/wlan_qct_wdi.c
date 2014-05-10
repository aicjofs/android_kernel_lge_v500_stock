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

/*===========================================================================

                       W L A N _ Q C T _ W D I. C

  OVERVIEW:

  This software unit holds the implementation of the WLAN Device Abstraction
  Layer Interface.

  The functions externalized by this module are to be called by any upper
  MAC implementation that wishes to use the WLAN Device.

  DEPENDENCIES:

  Are listed for each API below.


  Copyright (c) 2008 QUALCOMM Incorporated.
  All Rights Reserved.
  Qualcomm Confidential and Proprietary
===========================================================================*/

/*                                                                           

                                           


                                                                       
                                                                


                             


                                      
                                                                             
                                                   
                                    

                                                                           */

/*                                                                            
                
                                                                            */
#include "wlan_qct_wdi.h"
#include "wlan_qct_wdi_i.h"
#include "wlan_qct_wdi_sta.h"
#include "wlan_qct_wdi_dp.h"

#include "wlan_qct_wdi_cts.h"

#include "wlan_qct_pal_api.h"
#include "wlan_qct_pal_type.h"
#include "wlan_qct_pal_status.h"
#include "wlan_qct_pal_sync.h"
#include "wlan_qct_pal_msg.h"
#include "wlan_qct_pal_trace.h"
#include "wlan_qct_pal_packet.h"

#include "wlan_qct_wdi_dts.h"

#include "wlan_hal_msg.h"

#ifdef ANI_MANF_DIAG
#include "pttMsgApi.h"
#endif /*               */

/*                                                                           
                                                                   
                                                                            */
#define WDI_WCTS_ACTION_TIMEOUT       2000 /*                                 */

#define MAC_ADDR_ARRAY(a) (a)[0], (a)[1], (a)[2], (a)[3], (a)[4], (a)[5]
#define MAC_ADDRESS_STR "%02x:%02x:%02x:%02x:%02x:%02x"


#ifdef FEATURE_WLAN_SCAN_PNO
#define WDI_PNO_VERSION_MASK 0x8000
#endif

/*                               */
static tWlanFeatCaps *gpHostWlanFeatCaps = NULL;
/*                             */
static tWlanFeatCaps *gpFwWlanFeatCaps = NULL;
/*                                                       
                                                               
 */
static placeHolderInCapBitmap supportEnabledFeatures[] =
   {MCC, P2P, DOT11AC, SLM_SESSIONIZATION};

/*                                                                          
                          
                                                                           */
WPT_STATIC const WDI_MainFsmEntryType wdiMainFSM[WDI_MAX_ST] =
{
  /*           */
  {{
    WDI_MainStart,              /*               */
    NULL,                       /*              */
    WDI_MainReqBusy,            /*                 */
    WDI_MainRspInit,            /*                  */
    WDI_MainClose,              /*               */
    WDI_MainShutdown            /*                  */
  }},

  /*              */
  {{
    WDI_MainStartStarted,       /*               */
    WDI_MainStopStarted,        /*              */
    WDI_MainReqStarted,         /*                 */
    WDI_MainRsp,                /*                  */
    NULL,                       /*               */
    WDI_MainShutdown            /*                  */
  }},

  /*              */
  {{
    WDI_MainStart,              /*               */
    WDI_MainStopStopped,        /*              */
    NULL,                       /*                 */
    WDI_MainRsp,                /*                  */
    WDI_MainClose,              /*               */
    NULL                        /*                  */
  }},

  /*           */
  {{
    WDI_MainStartBusy,          /*               */
    WDI_MainStopBusy,           /*              */
    WDI_MainReqBusy,            /*                 */
    WDI_MainRsp,                /*                  */
    WDI_MainCloseBusy,          /*               */
    WDI_MainShutdownBusy        /*                  */
  }}
};

/*                                                                           
                                                                          
                                                                      
          
                                                                            */
WDI_ReqProcFuncType  pfnReqProcTbl[WDI_MAX_UMAC_IND] =
{
  /*    */
  WDI_ProcessStartReq,      /*                */
  WDI_ProcessStopReq,       /*               */
  WDI_ProcessCloseReq,      /*                */

  /*    */
  WDI_ProcessInitScanReq,   /*                    */
  WDI_ProcessStartScanReq,  /*                     */
  WDI_ProcessEndScanReq,    /*                   */
  WDI_ProcessFinishScanReq, /*                      */

  /*           */
  WDI_ProcessJoinReq,       /*               */
  WDI_ProcessConfigBSSReq,  /*                     */
  WDI_ProcessDelBSSReq,     /*                  */
  WDI_ProcessPostAssocReq,  /*                     */
  WDI_ProcessDelSTAReq,     /*                  */

  /*          */
  WDI_ProcessSetBssKeyReq,        /*                      */
  WDI_ProcessRemoveBssKeyReq,     /*                      */
  WDI_ProcessSetStaKeyReq,        /*                      */
  WDI_ProcessRemoveStaKeyReq,     /*                      */

  /*                 */
  WDI_ProcessAddTSpecReq,          /*                 */
  WDI_ProcessDelTSpecReq,          /*                 */
  WDI_ProcessUpdateEDCAParamsReq,  /*                        */
  WDI_ProcessAddBASessionReq,      /*                         */
  WDI_ProcessDelBAReq,             /*                 */

  /*                            */
  WDI_ProcessChannelSwitchReq,     /*                    */
  WDI_ProcessConfigStaReq,         /*                     */
  WDI_ProcessSetLinkStateReq,      /*                      */
  WDI_ProcessGetStatsReq,          /*                    */
  WDI_ProcessUpdateCfgReq,         /*                     */

  /*       */
  WDI_ProcessAddBAReq,             /*                 */
  WDI_ProcessTriggerBAReq,         /*                     */

  /*                      */
  WDI_ProcessUpdateBeaconParamsReq, /*                       */
  WDI_ProcessSendBeaconParamsReq, /*                  */

  WDI_ProcessUpdateProbeRspTemplateReq, /*                                */
  WDI_ProcessSetStaBcastKeyReq,        /*                            */
  WDI_ProcessRemoveStaBcastKeyReq,     /*                            */
  WDI_ProcessSetMaxTxPowerReq,         /*                        */
#ifdef WLAN_FEATURE_P2P
  WDI_ProcessP2PGONOAReq,              /*                                  */
#else
  NULL,
#endif
  /*                */
  WDI_ProcessEnterImpsReq,         /*                     */
  WDI_ProcessExitImpsReq,          /*                    */
  WDI_ProcessEnterBmpsReq,         /*                     */
  WDI_ProcessExitBmpsReq,          /*                    */
  WDI_ProcessEnterUapsdReq,        /*                      */
  WDI_ProcessExitUapsdReq,         /*                     */
  WDI_ProcessSetUapsdAcParamsReq,  /*                          */
  WDI_ProcessUpdateUapsdParamsReq, /*                             */
  WDI_ProcessConfigureRxpFilterReq, /*                               */
  WDI_ProcessSetBeaconFilterReq,   /*                            */
  WDI_ProcessRemBeaconFilterReq,   /*                            */
  WDI_ProcessSetRSSIThresholdsReq, /*                              */
  WDI_ProcessHostOffloadReq,       /*                       */
  WDI_ProcessWowlAddBcPtrnReq,     /*                           */
  WDI_ProcessWowlDelBcPtrnReq,     /*                           */
  WDI_ProcessWowlEnterReq,         /*                     */
  WDI_ProcessWowlExitReq,          /*                    */
  WDI_ProcessConfigureAppsCpuWakeupStateReq, /*                                          */
  /*                */
  WDI_ProcessNvDownloadReq,      /*                    */
  WDI_ProcessFlushAcReq,           /*                   */
  WDI_ProcessBtAmpEventReq,        /*                      */
#ifdef WLAN_FEATURE_VOWIFI_11R
  WDI_ProcessAggrAddTSpecReq,      /*                     */
#else
  NULL,
#endif /*                         */
  WDI_ProcessAddSTASelfReq,         /*                      */
  WDI_ProcessDelSTASelfReq,          /*                      */
#ifdef ANI_MANF_DIAG
  WDI_ProcessFTMCommandReq,            /*                 */
#else
  NULL,
#endif /*               */

#ifdef FEATURE_OEM_DATA_SUPPORT
  WDI_ProcessStartOemDataReq,     /*                      */
#else
  NULL,
#endif /*                        */
  WDI_ProcessHostResumeReq,            /*                   */

  WDI_ProcessKeepAliveReq,       /*                    */

#ifdef FEATURE_WLAN_SCAN_PNO
  WDI_ProcessSetPreferredNetworkReq,  /*                          */
  WDI_ProcessSetRssiFilterReq,        /*                         */
  WDI_ProcessUpdateScanParamsReq,     /*                            */
#else
  NULL,
  NULL,
  NULL,
#endif /*                       */

  WDI_ProcessSetTxPerTrackingReq,     /*                              */

#ifdef WLAN_FEATURE_PACKET_FILTERING
  /*                             */
  WDI_Process8023MulticastListReq,
  /*                                   */
  WDI_ProcessReceiveFilterSetFilterReq,
  /*                                              */
  WDI_ProcessFilterMatchCountReq,
  /*                                     */
  WDI_ProcessReceiveFilterClearFilterReq,
#else
  NULL,
  NULL,
  NULL,
  NULL,
#endif //                              
  WDI_ProcessInitScanReq,               /*                       */
  WDI_ProcessHALDumpCmdReq,             /*                     */
  WDI_ProcessShutdownReq,               /*                   */

  WDI_ProcessSetPowerParamsReq,         /*                        */
#ifdef FEATURE_WLAN_CCX
  WDI_ProcessTSMStatsReq,          /*                   */
#else
  NULL,
#endif

#ifdef WLAN_FEATURE_GTK_OFFLOAD
  WDI_ProcessGTKOffloadReq,          /*                      */
  WDI_ProcessGTKOffloadGetInfoReq,   /*                              */
#else
  NULL,
  NULL,
#endif //                         

  WDI_ProcessSetTmLevelReq,             /*                    */
  WDI_ProcessFeatureCapsExchangeReq,    /*                               */
  /*                                                                         
               
                                                                           */
  WDI_ProcessHostSuspendInd,            /*                     */
};


/*                                                                           
                                                                          
                                                                      
          
                                                                            */
WDI_RspProcFuncType  pfnRspProcTbl[WDI_MAX_RESP] =
{
  /*    */
  WDI_ProcessStartRsp,            /*                 */
  WDI_ProcessStopRsp,             /*                */
  WDI_ProcessCloseRsp,            /*                 */

  /*    */
  WDI_ProcessInitScanRsp,         /*                     */
  WDI_ProcessStartScanRsp,        /*                      */
  WDI_ProcessEndScanRsp,          /*                    */
  WDI_ProcessFinishScanRsp,       /*                       */

  /*            */
  WDI_ProcessJoinRsp,             /*                */
  WDI_ProcessConfigBSSRsp,        /*                      */
  WDI_ProcessDelBSSRsp,           /*                   */
  WDI_ProcessPostAssocRsp,        /*                      */
  WDI_ProcessDelSTARsp,           /*                   */

  /*          */
  WDI_ProcessSetBssKeyRsp,        /*                       */
  WDI_ProcessRemoveBssKeyRsp,     /*                       */
  WDI_ProcessSetStaKeyRsp,        /*                       */
  WDI_ProcessRemoveStaKeyRsp,     /*                       */

  /*                 */
  WDI_ProcessAddTSpecRsp,          /*                  */
  WDI_ProcessDelTSpecRsp,          /*                  */
  WDI_ProcessUpdateEDCAParamsRsp,  /*                         */
  WDI_ProcessAddBASessionRsp,      /*                          */
  WDI_ProcessDelBARsp,             /*                  */

  /*                            */
  WDI_ProcessChannelSwitchRsp,     /*                     */
  WDI_ProcessConfigStaRsp,         /*                      */
  WDI_ProcessSetLinkStateRsp,      /*                       */
  WDI_ProcessGetStatsRsp,          /*                     */
  WDI_ProcessUpdateCfgRsp,         /*                      */

  /*        */
  WDI_ProcessAddBARsp,             /*                  */
  WDI_ProcessTriggerBARsp,         /*                      */

  /*          */
  WDI_ProcessUpdateBeaconParamsRsp, /*                       */
  WDI_ProcessSendBeaconParamsRsp,   /*                  */

  /*            */
  WDI_ProcessUpdateProbeRspTemplateRsp,/*                                */
  WDI_ProcessSetStaBcastKeyRsp,        /*                           */
  WDI_ProcessRemoveStaBcastKeyRsp,     /*                           */
  WDI_ProcessSetMaxTxPowerRsp,         /*                          */

  /*                */
  WDI_ProcessEnterImpsRsp,         /*                      */
  WDI_ProcessExitImpsRsp,          /*                     */
  WDI_ProcessEnterBmpsRsp,         /*                      */
  WDI_ProcessExitBmpsRsp,          /*                     */
  WDI_ProcessEnterUapsdRsp,        /*                       */
  WDI_ProcessExitUapsdRsp,         /*                      */
  WDI_ProcessSetUapsdAcParamsRsp,  /*                           */
  WDI_ProcessUpdateUapsdParamsRsp, /*                              */
  WDI_ProcessConfigureRxpFilterRsp,/*                                */
  WDI_ProcessSetBeaconFilterRsp,   /*                             */
  WDI_ProcessRemBeaconFilterRsp,   /*                             */
  WDI_ProcessSetRSSIThresoldsRsp,  /*                               */
  WDI_ProcessHostOffloadRsp,       /*                        */
  WDI_ProcessWowlAddBcPtrnRsp,     /*                            */
  WDI_ProcessWowlDelBcPtrnRsp,     /*                            */
  WDI_ProcessWowlEnterRsp,         /*                      */
  WDI_ProcessWowlExitRsp,          /*                     */
  WDI_ProcessConfigureAppsCpuWakeupStateRsp, /*                                           */


  WDI_ProcessNvDownloadRsp, /*                     */

  WDI_ProcessFlushAcRsp,           /*                    */
  WDI_ProcessBtAmpEventRsp,        /*                       */
#ifdef WLAN_FEATURE_VOWIFI_11R
  WDI_ProcessAggrAddTSpecRsp,       /*                       */
#else
  NULL,
#endif /*                         */
  WDI_ProcessAddSTASelfRsp,          /*                       */
  WDI_ProcessDelSTASelfRsp,          /*                       */
#ifdef FEATURE_OEM_DATA_SUPPORT
  WDI_ProcessStartOemDataRsp,     /*                       */
#else
  NULL,
#endif /*                        */
  WDI_ProcessHostResumeRsp,        /*                    */

#ifdef WLAN_FEATURE_P2P
  WDI_ProcessP2PGONOARsp,           /*                                  */
#else
  NULL,
#endif

#ifdef ANI_MANF_DIAG
  WDI_ProcessFTMCommandRsp,         /*                  */
#else
  NULL,
#endif /*               */

  WDI_ProcessKeepAliveRsp,       /*                      */

#ifdef FEATURE_WLAN_SCAN_PNO
  WDI_ProcessSetPreferredNetworkRsp,     /*                           */
  WDI_ProcessSetRssiFilterRsp,           /*                          */
  WDI_ProcessUpdateScanParamsRsp,        /*                             */
#else
  NULL,
  NULL,
  NULL,
#endif //                      

  WDI_ProcessSetTxPerTrackingRsp,      /*                               */

  /*                                                                     
               
                                                                       */
#ifdef WLAN_FEATURE_PACKET_FILTERING
  /*                              */
  WDI_Process8023MulticastListRsp,
  /*                                    */
  WDI_ProcessReceiveFilterSetFilterRsp,
  /*                                               */
  WDI_ProcessFilterMatchCountRsp,
  /*                                      */
  WDI_ProcessReceiveFilterClearFilterRsp,
#else
  NULL,
  NULL,
  NULL,
  NULL,
#endif //                              

  WDI_ProcessHALDumpCmdRsp,       /*                       */
  WDI_ProcessShutdownRsp,         /*                   */

  WDI_ProcessSetPowerParamsRsp,         /*                         */
#ifdef FEATURE_WLAN_CCX
  WDI_ProcessTsmStatsRsp,          /*                     */
#else
  NULL,
#endif

#ifdef WLAN_FEATURE_GTK_OFFLOAD
  WDI_ProcessGtkOffloadRsp,             /*                       */
  WDI_ProcessGTKOffloadGetInfoRsp,      /*                               */
#else
  NULL,
  NULL,
#endif //                         
  WDI_ProcessSetTmLevelRsp,             /*                       */
  WDI_ProcessFeatureCapsExchangeRsp,    /*                                */

  /*                                                                     
               
                                                                       */
  WDI_ProcessLowRSSIInd,            /*                                                                       */
  WDI_ProcessMissedBeaconInd,       /*                            */
  WDI_ProcessUnkAddrFrameInd,       /*                                     */
  WDI_ProcessMicFailureInd,         /*                          */
  WDI_ProcessFatalErrorInd,         /*                          */
  WDI_ProcessDelSTAInd,             /*                      */

  WDI_ProcessCoexInd,               /*                   */

  WDI_ProcessTxCompleteInd,         /*                          */

#ifdef WLAN_FEATURE_P2P
  WDI_ProcessP2pNoaAttrInd,         /*                     */
#else
  NULL,
#endif

#ifdef FEATURE_WLAN_SCAN_PNO
  WDI_ProcessPrefNetworkFoundInd,   /*                                */
#else
  NULL,
#endif //                      

#ifdef WLAN_WAKEUP_EVENTS
  WDI_ProcessWakeReasonInd,          /*                     */
#else //                   
  NULL,
#endif //                   

  WDI_ProcessTxPerHitInd,               /*                         */
};


/*                                                                           
                               
                                                                            */
WDI_ControlBlockType  gWDICb;
static wpt_uint8      gWDIInitialized = eWLAN_PAL_FALSE;

const wpt_uint8 szTransportChName[] = "WLAN_CTRL";

/*                                                      */
WPT_INLINE
void* WDI_GET_PAL_CTX( void )
{
  return gWDICb.pPALContext;
}/*               */

/*                                                                            
                          
                                                                             */
/*                                            */
WPT_STATIC WPT_INLINE WDI_Status
WDI_HAL_2_WDI_STATUS
(
  eHalStatus halStatus
);

/*                                              */
WPT_STATIC WPT_INLINE tHalHostMsgType
WDI_2_HAL_REQ_TYPE
(
  WDI_RequestEnumType    wdiReqType
);

/*                                                */
WPT_STATIC WPT_INLINE WDI_ResponseEnumType
HAL_2_WDI_RSP_TYPE
(
  tHalHostMsgType halMsg
);

/*                                            */
WPT_STATIC WPT_INLINE tDriverType
WDI_2_HAL_DRV_TYPE
(
  WDI_DriverType wdiDriverType
);

/*                                            */
WPT_STATIC WPT_INLINE tHalStopType
WDI_2_HAL_STOP_REASON
(
  WDI_StopType wdiStopType
);

/*                                                  */
WPT_STATIC WPT_INLINE eHalSysMode
WDI_2_HAL_SCAN_MODE
(
  WDI_ScanMode wdiScanMode
);

/*                                                     */
WPT_STATIC WPT_INLINE ePhyChanBondState
WDI_2_HAL_SEC_CH_OFFSET
(
  WDI_HTSecondaryChannelOffset wdiSecChOffset
);

/*                                      */
WPT_STATIC WPT_INLINE tSirBssType
WDI_2_HAL_BSS_TYPE
(
  WDI_BssType wdiBSSType
);

/*                                    */
WPT_STATIC WPT_INLINE tSirNwType
WDI_2_HAL_NW_TYPE
(
  WDI_NwType wdiNWType
);

/*                                                */
WPT_STATIC WPT_INLINE ePhyChanBondState
WDI_2_HAL_CB_STATE
(
  WDI_PhyChanBondState wdiCbState
);

/*                                                */
WPT_STATIC WPT_INLINE tSirMacHTOperatingMode
WDI_2_HAL_HT_OPER_MODE
(
  WDI_HTOperatingMode wdiHTOperMode
);

/*                                              */
WPT_STATIC WPT_INLINE tSirMacHTMIMOPowerSaveState
WDI_2_HAL_MIMO_PS
(
  WDI_HTMIMOPowerSaveState wdiHTOperMode
);

/*                                      */
WPT_STATIC WPT_INLINE tAniEdType
WDI_2_HAL_ENC_TYPE
(
  WDI_EncryptType wdiEncType
);

/*                                      */
WPT_STATIC WPT_INLINE tAniWepType
WDI_2_HAL_WEP_TYPE
(
  WDI_WepType  wdiWEPType
);

/*                                          */
WPT_STATIC WPT_INLINE tSirLinkState
WDI_2_HAL_LINK_STATE
(
  WDI_LinkStateType  wdiLinkState
);

/*                                         */
WPT_STATIC WPT_INLINE
void
WDI_CopyWDIStaCtxToHALStaCtx
(
  tConfigStaParams*          phalConfigSta,
  WDI_ConfigStaReqInfoType*  pwdiConfigSta
);

/*                                           */
WPT_STATIC WPT_INLINE void
WDI_CopyWDIRateSetToHALRateSet
(
  tSirMacRateSet* pHalRateSet,
  WDI_RateSet*    pwdiRateSet
);

/*                                                    */
WPT_STATIC WPT_INLINE void
WDI_CopyWDIEDCAParamsToHALEDCAParams
(
  tSirMacEdcaParamRecord* phalEdcaParam,
  WDI_EdcaParamRecord*    pWDIEdcaParam
);

/*                                                        */
WPT_STATIC WPT_INLINE void
WDI_CopyWDIMgmFrameHdrToHALMgmFrameHdr
(
  tSirMacMgmtHdr* pmacMgmtHdr,
  WDI_MacMgmtHdr* pwdiMacMgmtHdr
);

/*                                                    */
WPT_STATIC WPT_INLINE void
WDI_CopyWDIConfigBSSToHALConfigBSS
(
  tConfigBssParams*         phalConfigBSS,
  WDI_ConfigBSSReqInfoType* pwdiConfigBSS
);

/*                                                                      
                          */
WPT_STATIC WPT_INLINE void
WDI_ExtractRequestCBFromEvent
(
  WDI_EventInfoType* pEvent,
  WDI_ReqStatusCb*   ppfnReqCB,
  void**             ppUserData
);

wpt_uint8
WDI_FindEmptySession
(
  WDI_ControlBlockType*   pWDICtx,
  WDI_BSSSessionType**    ppSession
);

void
WDI_AddBcastSTAtoSTATable
(
  WDI_ControlBlockType*  pWDICtx,
  WDI_AddStaParams *     staParams,
  wpt_uint16             usBcastStaIdx
);

WDI_Status WDI_SendNvBlobReq
(
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
);

void
WDI_SetPowerStateCb
(
   wpt_status status,
   unsigned int dxePhyAddr,
   void      *pContext
);

#define CASE_RETURN_STRING( str )           \
    case ( ( str ) ): return( #str ); break \

/* 
                                                                     

                                           

     
                                    
*/
static char *WDI_getReqMsgString(wpt_uint16 wdiReqMsgId)
{
  switch (wdiReqMsgId)
  {
    CASE_RETURN_STRING( WDI_START_REQ );
    CASE_RETURN_STRING( WDI_STOP_REQ );
    CASE_RETURN_STRING( WDI_CLOSE_REQ );
    CASE_RETURN_STRING( WDI_INIT_SCAN_REQ );
    CASE_RETURN_STRING( WDI_START_SCAN_REQ );
    CASE_RETURN_STRING( WDI_END_SCAN_REQ );
    CASE_RETURN_STRING( WDI_FINISH_SCAN_REQ );
    CASE_RETURN_STRING( WDI_JOIN_REQ );
    CASE_RETURN_STRING( WDI_CONFIG_BSS_REQ );
    CASE_RETURN_STRING( WDI_DEL_BSS_REQ );
    CASE_RETURN_STRING( WDI_POST_ASSOC_REQ );
    CASE_RETURN_STRING( WDI_DEL_STA_REQ );
    CASE_RETURN_STRING( WDI_SET_BSS_KEY_REQ );
    CASE_RETURN_STRING( WDI_RMV_BSS_KEY_REQ );
    CASE_RETURN_STRING( WDI_SET_STA_KEY_REQ );
    CASE_RETURN_STRING( WDI_RMV_STA_KEY_REQ );
    CASE_RETURN_STRING( WDI_ADD_TS_REQ );
    CASE_RETURN_STRING( WDI_DEL_TS_REQ );
    CASE_RETURN_STRING( WDI_UPD_EDCA_PRMS_REQ );
    CASE_RETURN_STRING( WDI_ADD_BA_SESSION_REQ );
    CASE_RETURN_STRING( WDI_DEL_BA_REQ );
    CASE_RETURN_STRING( WDI_CH_SWITCH_REQ );
    CASE_RETURN_STRING( WDI_CONFIG_STA_REQ );
    CASE_RETURN_STRING( WDI_SET_LINK_ST_REQ );
    CASE_RETURN_STRING( WDI_GET_STATS_REQ );
    CASE_RETURN_STRING( WDI_UPDATE_CFG_REQ );
    CASE_RETURN_STRING( WDI_ADD_BA_REQ );
    CASE_RETURN_STRING( WDI_TRIGGER_BA_REQ );
    CASE_RETURN_STRING( WDI_UPD_BCON_PRMS_REQ );
    CASE_RETURN_STRING( WDI_SND_BCON_REQ );
    CASE_RETURN_STRING( WDI_UPD_PROBE_RSP_TEMPLATE_REQ );
    CASE_RETURN_STRING( WDI_SET_STA_BCAST_KEY_REQ );
    CASE_RETURN_STRING( WDI_RMV_STA_BCAST_KEY_REQ );
    CASE_RETURN_STRING( WDI_SET_MAX_TX_POWER_REQ );
    CASE_RETURN_STRING( WDI_P2P_GO_NOTICE_OF_ABSENCE_REQ );
    CASE_RETURN_STRING( WDI_ENTER_IMPS_REQ );
    CASE_RETURN_STRING( WDI_EXIT_IMPS_REQ );
    CASE_RETURN_STRING( WDI_ENTER_BMPS_REQ );
    CASE_RETURN_STRING( WDI_EXIT_BMPS_REQ );
    CASE_RETURN_STRING( WDI_ENTER_UAPSD_REQ );
    CASE_RETURN_STRING( WDI_EXIT_UAPSD_REQ );
    CASE_RETURN_STRING( WDI_SET_UAPSD_PARAM_REQ );
    CASE_RETURN_STRING( WDI_UPDATE_UAPSD_PARAM_REQ );
    CASE_RETURN_STRING( WDI_CONFIGURE_RXP_FILTER_REQ );
    CASE_RETURN_STRING( WDI_SET_BEACON_FILTER_REQ);
    CASE_RETURN_STRING( WDI_REM_BEACON_FILTER_REQ );
    CASE_RETURN_STRING( WDI_SET_RSSI_THRESHOLDS_REQ );
    CASE_RETURN_STRING( WDI_HOST_OFFLOAD_REQ );
    CASE_RETURN_STRING( WDI_WOWL_ADD_BC_PTRN_REQ );
    CASE_RETURN_STRING( WDI_WOWL_DEL_BC_PTRN_REQ );
    CASE_RETURN_STRING( WDI_WOWL_ENTER_REQ );
    CASE_RETURN_STRING( WDI_WOWL_EXIT_REQ );
    CASE_RETURN_STRING( WDI_CONFIGURE_APPS_CPU_WAKEUP_STATE_REQ );
    CASE_RETURN_STRING( WDI_NV_DOWNLOAD_REQ );
    CASE_RETURN_STRING( WDI_FLUSH_AC_REQ );
    CASE_RETURN_STRING( WDI_BTAMP_EVENT_REQ );
    CASE_RETURN_STRING( WDI_AGGR_ADD_TS_REQ );
    CASE_RETURN_STRING( WDI_ADD_STA_SELF_REQ );
    CASE_RETURN_STRING( WDI_DEL_STA_SELF_REQ );
    CASE_RETURN_STRING( WDI_FTM_CMD_REQ );
    CASE_RETURN_STRING( WDI_START_OEM_DATA_REQ );
    CASE_RETURN_STRING( WDI_HOST_RESUME_REQ );
    CASE_RETURN_STRING( WDI_KEEP_ALIVE_REQ);
  #ifdef FEATURE_WLAN_SCAN_PNO
    CASE_RETURN_STRING( WDI_SET_PREF_NETWORK_REQ );
    CASE_RETURN_STRING( WDI_SET_RSSI_FILTER_REQ );
    CASE_RETURN_STRING( WDI_UPDATE_SCAN_PARAMS_REQ );
  #endif
    CASE_RETURN_STRING( WDI_SET_TX_PER_TRACKING_REQ );
    CASE_RETURN_STRING( WDI_8023_MULTICAST_LIST_REQ );
    CASE_RETURN_STRING( WDI_RECEIVE_FILTER_SET_FILTER_REQ );
    CASE_RETURN_STRING( WDI_PACKET_COALESCING_FILTER_MATCH_COUNT_REQ );
    CASE_RETURN_STRING( WDI_RECEIVE_FILTER_CLEAR_FILTER_REQ );
    CASE_RETURN_STRING( WDI_INIT_SCAN_CON_REQ );
    CASE_RETURN_STRING( WDI_HAL_DUMP_CMD_REQ );
    CASE_RETURN_STRING( WDI_SHUTDOWN_REQ );
    CASE_RETURN_STRING( WDI_SET_POWER_PARAMS_REQ );
    default:
        return "Unknown WDI MessageId";
  }
}



/* 
                                                                      

                                             

     
                                    
*/
static char *WDI_getRespMsgString(wpt_uint16 wdiRespMsgId)
{
  switch (wdiRespMsgId)
  {
    CASE_RETURN_STRING( WDI_START_RESP );
    CASE_RETURN_STRING( WDI_STOP_RESP );
    CASE_RETURN_STRING( WDI_CLOSE_RESP );
    CASE_RETURN_STRING( WDI_INIT_SCAN_RESP );
    CASE_RETURN_STRING( WDI_START_SCAN_RESP );
    CASE_RETURN_STRING( WDI_END_SCAN_RESP );
    CASE_RETURN_STRING( WDI_FINISH_SCAN_RESP );
    CASE_RETURN_STRING( WDI_JOIN_RESP );
    CASE_RETURN_STRING( WDI_CONFIG_BSS_RESP );
    CASE_RETURN_STRING( WDI_DEL_BSS_RESP );
    CASE_RETURN_STRING( WDI_POST_ASSOC_RESP );
    CASE_RETURN_STRING( WDI_DEL_STA_RESP );
    CASE_RETURN_STRING( WDI_SET_BSS_KEY_RESP );
    CASE_RETURN_STRING( WDI_RMV_BSS_KEY_RESP );
    CASE_RETURN_STRING( WDI_SET_STA_KEY_RESP );
    CASE_RETURN_STRING( WDI_RMV_STA_KEY_RESP );
    CASE_RETURN_STRING( WDI_ADD_TS_RESP );
    CASE_RETURN_STRING( WDI_DEL_TS_RESP );
    CASE_RETURN_STRING( WDI_UPD_EDCA_PRMS_RESP );
    CASE_RETURN_STRING( WDI_ADD_BA_SESSION_RESP );
    CASE_RETURN_STRING( WDI_DEL_BA_RESP );
    CASE_RETURN_STRING( WDI_CH_SWITCH_RESP );
    CASE_RETURN_STRING( WDI_CONFIG_STA_RESP );
    CASE_RETURN_STRING( WDI_SET_LINK_ST_RESP );
    CASE_RETURN_STRING( WDI_GET_STATS_RESP );
    CASE_RETURN_STRING( WDI_UPDATE_CFG_RESP );
    CASE_RETURN_STRING( WDI_ADD_BA_RESP );
    CASE_RETURN_STRING( WDI_TRIGGER_BA_RESP );
    CASE_RETURN_STRING( WDI_UPD_BCON_PRMS_RESP );
    CASE_RETURN_STRING( WDI_SND_BCON_RESP );
    CASE_RETURN_STRING( WDI_UPD_PROBE_RSP_TEMPLATE_RESP );
    CASE_RETURN_STRING( WDI_SET_STA_BCAST_KEY_RESP );
    CASE_RETURN_STRING( WDI_RMV_STA_BCAST_KEY_RESP );
    CASE_RETURN_STRING( WDI_SET_MAX_TX_POWER_RESP );
    CASE_RETURN_STRING( WDI_P2P_GO_NOTICE_OF_ABSENCE_RESP );
    CASE_RETURN_STRING( WDI_ENTER_IMPS_RESP );
    CASE_RETURN_STRING( WDI_EXIT_IMPS_RESP );
    CASE_RETURN_STRING( WDI_ENTER_BMPS_RESP );
    CASE_RETURN_STRING( WDI_EXIT_BMPS_RESP );
    CASE_RETURN_STRING( WDI_ENTER_UAPSD_RESP );
    CASE_RETURN_STRING( WDI_EXIT_UAPSD_RESP );
    CASE_RETURN_STRING( WDI_SET_UAPSD_PARAM_RESP );
    CASE_RETURN_STRING( WDI_UPDATE_UAPSD_PARAM_RESP );
    CASE_RETURN_STRING( WDI_CONFIGURE_RXP_FILTER_RESP );
    CASE_RETURN_STRING( WDI_SET_BEACON_FILTER_RESP);
    CASE_RETURN_STRING( WDI_REM_BEACON_FILTER_RESP );
    CASE_RETURN_STRING( WDI_SET_RSSI_THRESHOLDS_RESP );
    CASE_RETURN_STRING( WDI_HOST_OFFLOAD_RESP );
    CASE_RETURN_STRING( WDI_WOWL_ADD_BC_PTRN_RESP );
    CASE_RETURN_STRING( WDI_WOWL_DEL_BC_PTRN_RESP );
    CASE_RETURN_STRING( WDI_WOWL_ENTER_RESP );
    CASE_RETURN_STRING( WDI_WOWL_EXIT_RESP );
    CASE_RETURN_STRING( WDI_CONFIGURE_APPS_CPU_WAKEUP_STATE_RESP );
    CASE_RETURN_STRING( WDI_NV_DOWNLOAD_RESP );
    CASE_RETURN_STRING( WDI_FLUSH_AC_RESP );
    CASE_RETURN_STRING( WDI_BTAMP_EVENT_RESP );
    CASE_RETURN_STRING( WDI_AGGR_ADD_TS_RESP );
    CASE_RETURN_STRING( WDI_ADD_STA_SELF_RESP );
    CASE_RETURN_STRING( WDI_DEL_STA_SELF_RESP );
    CASE_RETURN_STRING( WDI_FTM_CMD_RESP );
    CASE_RETURN_STRING( WDI_START_OEM_DATA_RESP );
    CASE_RETURN_STRING( WDI_HOST_RESUME_RESP );
    CASE_RETURN_STRING( WDI_KEEP_ALIVE_RESP);
  #ifdef FEATURE_WLAN_SCAN_PNO
    CASE_RETURN_STRING( WDI_SET_PREF_NETWORK_RESP );
    CASE_RETURN_STRING( WDI_SET_RSSI_FILTER_RESP );
    CASE_RETURN_STRING( WDI_UPDATE_SCAN_PARAMS_RESP );
  #endif
    CASE_RETURN_STRING( WDI_SET_TX_PER_TRACKING_RESP );
    CASE_RETURN_STRING( WDI_8023_MULTICAST_LIST_RESP );
    CASE_RETURN_STRING( WDI_RECEIVE_FILTER_SET_FILTER_RESP );
    CASE_RETURN_STRING( WDI_PACKET_COALESCING_FILTER_MATCH_COUNT_RESP );
    CASE_RETURN_STRING( WDI_RECEIVE_FILTER_CLEAR_FILTER_RESP );
    CASE_RETURN_STRING( WDI_HAL_DUMP_CMD_RESP );
    CASE_RETURN_STRING( WDI_SHUTDOWN_RESP );
    CASE_RETURN_STRING( WDI_SET_POWER_PARAMS_RESP );
    default:
        return "Unknown WDI MessageId";
  }
}

/* 
                                                                  

                                  

     
                                    
*/
static char *WDI_getHALStatusMsgString(wpt_uint16 halStatusId)
{
  switch (halStatusId)
  {
    CASE_RETURN_STRING( eHAL_STATUS_SUCCESS );
    CASE_RETURN_STRING( PAL_STATUS_INVAL );
    CASE_RETURN_STRING( PAL_STATUS_ALREADY );
    CASE_RETURN_STRING( PAL_STATUS_EMPTY );
    CASE_RETURN_STRING( PAL_STATUS_FAILURE );
    CASE_RETURN_STRING( eHAL_STATUS_FAILURE );
    CASE_RETURN_STRING( eHAL_STATUS_INVALID_PARAMETER );
    CASE_RETURN_STRING( eHAL_STATUS_INVALID_STAIDX );
    CASE_RETURN_STRING( eHAL_STATUS_DPU_DESCRIPTOR_TABLE_FULL );
    CASE_RETURN_STRING( eHAL_STATUS_NO_INTERRUPTS );
    CASE_RETURN_STRING( eHAL_STATUS_INTERRUPT_PRESENT );
    CASE_RETURN_STRING( eHAL_STATUS_STA_TABLE_FULL );
    CASE_RETURN_STRING( eHAL_STATUS_DUPLICATE_STA );
    CASE_RETURN_STRING( eHAL_STATUS_BSSID_INVALID );
    CASE_RETURN_STRING( eHAL_STATUS_STA_INVALID );
    CASE_RETURN_STRING( eHAL_STATUS_DUPLICATE_BSSID );
    CASE_RETURN_STRING( eHAL_STATUS_INVALID_BSSIDX );
    CASE_RETURN_STRING( eHAL_STATUS_BSSID_TABLE_FULL );
    CASE_RETURN_STRING( eHAL_STATUS_INVALID_SIGNATURE );
    CASE_RETURN_STRING( eHAL_STATUS_INVALID_KEYID );
    CASE_RETURN_STRING( eHAL_STATUS_SET_CHAN_ALREADY_ON_REQUESTED_CHAN );
    CASE_RETURN_STRING( eHAL_STATUS_UMA_DESCRIPTOR_TABLE_FULL );
    CASE_RETURN_STRING( eHAL_STATUS_DPU_MICKEY_TABLE_FULL );
    CASE_RETURN_STRING( eHAL_STATUS_BA_RX_BUFFERS_FULL );
    CASE_RETURN_STRING( eHAL_STATUS_BA_RX_MAX_SESSIONS_REACHED );
    CASE_RETURN_STRING( eHAL_STATUS_BA_RX_INVALID_SESSION_ID );
    CASE_RETURN_STRING( eHAL_STATUS_TIMER_START_FAILED );
    CASE_RETURN_STRING( eHAL_STATUS_TIMER_STOP_FAILED );
    CASE_RETURN_STRING( eHAL_STATUS_FAILED_ALLOC );
    CASE_RETURN_STRING( eHAL_STATUS_NOTIFY_BSS_FAIL );
    CASE_RETURN_STRING( eHAL_STATUS_DEL_STA_SELF_IGNORED_REF_COUNT_NOT_ZERO );
    CASE_RETURN_STRING( eHAL_STATUS_ADD_STA_SELF_IGNORED_REF_COUNT_NOT_ZERO );
    CASE_RETURN_STRING( eHAL_STATUS_FW_SEND_MSG_FAILED );
    default:
        return "Unknown HAL status";
  }
}

/*                                                                        

                                                

                                                                          */

/* 
                                               

                                                                             
                                                                           
                                             

                                                                  
                                                    
                                                        
                                                              

                                    
*/
WDI_Status
WDI_Init
(
  void*                      pOSContext,
  void**                     ppWDIGlobalCtx,
  WDI_DeviceCapabilityType*  pWdiDevCapability,
  unsigned int               driverType
)
{
  wpt_uint8               i;
  wpt_status              wptStatus;
  WDI_Status              wdiStatus;
  WCTS_TransportCBsType   wctsCBs;
  /*                                                                       */

  /*                                                                     
                
                                                                       */
  if (( NULL == ppWDIGlobalCtx ) || ( NULL == pWdiDevCapability ))
  {
     WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_ERROR,
              "Invalid input parameters in WDI_Init");

    return WDI_STATUS_E_FAILURE;
  }

  /*                                                                     
                                                                  
                                                                       */
  if ( eWLAN_PAL_FALSE != gWDIInitialized )
  {
    WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_WARN,
              "WDI module already initialized - return");

    return WDI_STATUS_SUCCESS;
  }

  /*                                                                         
                              
                                                                      
                                                                             */
  gWDIInitialized = eWLAN_PAL_TRUE;

  /*                        */
  WDI_CleanCB(&gWDICb);
  gWDICb.pOSContext = pOSContext;

  /*                   */
  wdiStatus = WDI_STATableInit(&gWDICb);
  if ( WDI_STATUS_SUCCESS != wdiStatus )
  {
    WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_FATAL,
               "%s: Failure while initializing STA Table, status %d",
               __FUNCTION__, wdiStatus);
    goto fail_STATableInit;
  }

  /*                                                                        
                
                                                                           */
  wptStatus =  wpalOpen(&gWDICb.pPALContext, pOSContext);
  if ( eWLAN_PAL_STATUS_SUCCESS != wptStatus )
  {
    WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_FATAL,
               "%s: Failed to open PAL, status %d",
               __FUNCTION__, wptStatus);
    goto fail_wpalOpen;
  }

  /*                                                                      
                             */
  wptStatus =  wpalMutexInit(&gWDICb.wptMutex);
  if ( eWLAN_PAL_STATUS_SUCCESS !=  wptStatus )
  {
    WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_FATAL,
               "%s: Failed to init mutex, status %d",
               __FUNCTION__, wptStatus);
    goto fail_mutex;
  }

  /*                                                                    
                                    */
  wptStatus = wpalTimerInit( &gWDICb.wptResponseTimer,
                             WDI_ResponseTimerCB,
                             &gWDICb);
  if ( eWLAN_PAL_STATUS_SUCCESS != wptStatus )
  {
    WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_FATAL,
              "%s: Failed to init response timer, status %d",
               __FUNCTION__, wptStatus);
    goto fail_timer;
  }

  /*                                          */
  wptStatus = wpal_list_init(&(gWDICb.wptPendingQueue));
  if ( eWLAN_PAL_STATUS_SUCCESS != wptStatus )
  {
    WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_FATAL,
              "%s: Failed to init pending request queue, status %d",
               __FUNCTION__, wptStatus);
    goto fail_pend_queue;
  }

  /*                                */
  wptStatus = wpal_list_init(&(gWDICb.wptPendingAssocSessionIdQueue));
  if ( eWLAN_PAL_STATUS_SUCCESS !=  wptStatus )
  {
    WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_FATAL,
              "%s: Failed to init assoc session queue, status %d",
               __FUNCTION__, wptStatus);
    goto fail_assoc_queue;
  }

  /*                                          */
  for ( i = 0; i < WDI_MAX_BSS_SESSIONS; i++ )
  {
    wptStatus = wpal_list_init(&(gWDICb.aBSSSessions[i].wptPendingQueue));
    if ( eWLAN_PAL_STATUS_SUCCESS !=  wptStatus )
    {
      WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_FATAL,
                 "%s: Failed to init BSS %d pending queue, status %d",
                 __FUNCTION__, i, wptStatus);
      goto fail_bss_queue;
    }
  }

  /*                                                                    */
  gWDICb.magic = WDI_CONTROL_BLOCK_MAGIC;

  /*                                                                        
                                           
                                                                           */
  wdiStatus = WDI_DP_UtilsInit(&gWDICb);
  if ( WDI_STATUS_SUCCESS != wdiStatus )
  {
    WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_FATAL,
               "%s: Failed to initialize the DP Util Module, status %d",
               __FUNCTION__, wdiStatus);
    goto fail_dp_util_init;
  }

  /*                            */
  wptStatus = wpalEventInit(&gWDICb.setPowerStateEvent);
  if ( eWLAN_PAL_STATUS_SUCCESS != wptStatus )
  {
     WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_FATAL,
                "%s: Failed to initialize power state event, status %d",
                __FUNCTION__, wptStatus);
     goto fail_power_event;
  }

  /*                        */
  wptStatus = wpalEventInit(&gWDICb.wctsActionEvent);
  if ( eWLAN_PAL_STATUS_SUCCESS !=  wptStatus )
  {
     WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_FATAL,
                "%s: Failed to initialize WCTS action event, status %d",
                __FUNCTION__, wptStatus);
     goto fail_wcts_event;
  }

  /*                                                                        
                                                    
                                                                           */
  wctsCBs.wctsNotifyCB      = WDI_NotifyMsgCTSCB;
  wctsCBs.wctsNotifyCBData  = &gWDICb;
  wctsCBs.wctsRxMsgCB       = WDI_RXMsgCTSCB;
  wctsCBs.wctsRxMsgCBData   = &gWDICb;

  gWDICb.bCTOpened          = eWLAN_PAL_FALSE;
  gWDICb.wctsHandle = WCTS_OpenTransport( szTransportChName ,
                                          WDI_CT_CHANNEL_SIZE,
                                          &wctsCBs );

  if ( NULL == gWDICb.wctsHandle )
  {
     WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_FATAL,
                "%s: Failed to open WCTS transport", __FUNCTION__);
     goto fail_wcts_open;
  }

  gWDICb.driverMode = (tDriverType)driverType;
  /*                                            */
  if(eDRIVER_TYPE_MFG != (tDriverType)driverType)
  {
    /*                                                                        
                            
                                                                             */
    if(eWLAN_PAL_STATUS_SUCCESS != WDTS_openTransport(&gWDICb))
    {
      WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_FATAL,
                 "%s: Failed to open the DT Transport", __FUNCTION__);
      goto fail_wdts_open;
    }
  }

  /*                                           */
  gWDICb.uGlobalState = WDI_INIT_ST;

  /*                                                         */
  *ppWDIGlobalCtx = &gWDICb;

  /*                               */
  pWdiDevCapability->bFrameXtlSupported = eWLAN_PAL_FALSE;
  pWdiDevCapability->ucMaxSTASupported  = gWDICb.ucMaxStations;
  pWdiDevCapability->ucMaxBSSSupported  = gWDICb.ucMaxBssids;
  return WDI_STATUS_SUCCESS;

  /*               
                                                 */

 fail_wdts_open:
  {
     wpt_status             eventStatus;

     /*                                                              
                                                                    
                                                                      
                                                   
     */

     eventStatus = wpalEventReset(&gWDICb.wctsActionEvent);
     if ( eWLAN_PAL_STATUS_SUCCESS != eventStatus )
     {
        WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_WARN,
                   "%s: Failed to reset WCTS action event", __FUNCTION__);
     }

     WCTS_CloseTransport(gWDICb.wctsHandle);

     /*                                                               
                                                               
                                                                      
                                 */
     if ( eWLAN_PAL_STATUS_SUCCESS == eventStatus )
     {
        eventStatus = wpalEventWait(&gWDICb.wctsActionEvent,
                                    WDI_WCTS_ACTION_TIMEOUT);
        if ( eWLAN_PAL_STATUS_SUCCESS != eventStatus )
        {
           WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_WARN,
                      "%s: Failed to wait on WCTS action event", __FUNCTION__);
        }
     }
     else
     {
        wpalSleep(WDI_WCTS_ACTION_TIMEOUT);
     }
  }
 fail_wcts_open:
  wpalEventDelete(&gWDICb.wctsActionEvent);
 fail_wcts_event:
  wpalEventDelete(&gWDICb.setPowerStateEvent);
 fail_power_event:
  WDI_DP_UtilsExit(&gWDICb);
 fail_dp_util_init:
  gWDICb.magic = 0;
 fail_bss_queue:
  /*                                                  */
  while (0 < i)
  {
     i--;
     wpal_list_destroy(&(gWDICb.aBSSSessions[i].wptPendingQueue));
  }
  wpal_list_destroy(&(gWDICb.wptPendingAssocSessionIdQueue));
 fail_assoc_queue:
  wpal_list_destroy(&(gWDICb.wptPendingQueue));
 fail_pend_queue:
  wpalTimerDelete(&gWDICb.wptResponseTimer);
 fail_timer:
  wpalMutexDelete(&gWDICb.wptMutex);
 fail_mutex:
  wpalClose(gWDICb.pPALContext);
 fail_wpalOpen:
  WDI_STATableClose(&gWDICb);
 fail_STATableInit:
  gWDIInitialized = eWLAN_PAL_FALSE;

  return WDI_STATUS_E_FAILURE;

}/*        */;

/* 
                                                               
                                                              
                                                               
                                                               
                                                           

                                                             
                                                              
                                                            
                        

                                

                                                            
                                          

                                                                
                                                    

                                                         
                

               
                                    
*/
WDI_Status
WDI_Start
(
  WDI_StartReqParamsType*  pwdiStartParams,
  WDI_StartRspCb           wdiStartRspCb,
  void*                    pUserData
)
{
  WDI_EventInfoType      wdiEventData;
  /*                                                                        */

  /*                                                                        
                
                                                                          */
  if ( eWLAN_PAL_FALSE == gWDIInitialized )
  {
    WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_ERROR,
              "WDI API call before module is initialized - Fail request");

    return WDI_STATUS_E_NOT_ALLOWED;
  }

  /*                                                                        
                                               
                                                                          */
  wdiEventData.wdiRequest      = WDI_START_REQ;
  wdiEventData.pEventData      = pwdiStartParams;
  wdiEventData.uEventDataSize  = sizeof(*pwdiStartParams);
  wdiEventData.pCBfnc          = wdiStartRspCb;
  wdiEventData.pUserData       = pUserData;

  return WDI_PostMainEvent(&gWDICb, WDI_REQUEST_EVENT, &wdiEventData);

}/*         */

/* 
                                                              
                                                              
                                                              
                                                               
                         

                                                   

                                                         

                                 

                                                          
                                          

                                                               
                                                   

                                                         
                

               
                                    
*/
WDI_Status
WDI_Stop
(
  WDI_StopReqParamsType*  pwdiStopParams,
  WDI_StopRspCb           wdiStopRspCb,
  void*                   pUserData
)
{
  WDI_EventInfoType      wdiEventData;
  WDI_ControlBlockType*  pWDICtx = &gWDICb;
  /*                                                                        */

  /*                                                                        
                
                                                                          */
  if ( eWLAN_PAL_FALSE == gWDIInitialized )
  {
    WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_ERROR,
              "WDI API call before module is initialized - Fail request");

    return WDI_STATUS_E_NOT_ALLOWED;
  }

  /*                                                          */
  wpalMutexAcquire(&pWDICtx->wptMutex);

  /*                         */
  WDI_ClearPendingRequests(pWDICtx);

  /*                                     */
  wpalMutexRelease(&pWDICtx->wptMutex);

  /*                           */
  wpalMemoryFree(gpHostWlanFeatCaps);
  wpalMemoryFree(gpFwWlanFeatCaps);
  gpHostWlanFeatCaps = NULL;
  gpFwWlanFeatCaps = NULL;

  /*                                                                        
                                               
                                                                          */
  wdiEventData.wdiRequest      = WDI_STOP_REQ;
  wdiEventData.pEventData      = pwdiStopParams;
  wdiEventData.uEventDataSize  = sizeof(*pwdiStopParams);
  wdiEventData.pCBfnc          = wdiStopRspCb;
  wdiEventData.pUserData       = pUserData;

  return WDI_PostMainEvent(&gWDICb, WDI_STOP_EVENT, &wdiEventData);

}/*        */



/* 
                                                             
                                                             
              

                                             

                                

            

              
                                    
*/
WDI_Status
WDI_Close
(
  void
)
{
  wpt_uint8              i;
  WDI_EventInfoType      wdiEventData;
  wpt_status             wptStatus;
  wpt_status             eventStatus;
  /*                                                                        */

  /*                                                                        
                
                                                                          */
  if ( eWLAN_PAL_FALSE == gWDIInitialized )
  {
    WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_ERROR,
              "WDI API call before module is initialized - Fail request");

    return WDI_STATUS_E_NOT_ALLOWED;
  }

  /*                                                          
                                                                  
                                */
  eventStatus = wpalEventReset(&gWDICb.wctsActionEvent);
  if ( eWLAN_PAL_STATUS_SUCCESS != eventStatus )
  {
     WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_WARN,
                "%s: Failed to reset WCTS action event", __FUNCTION__);
     /*                                                    */
  }

  /*                                                                        
                                               
                                                                          */
  wdiEventData.wdiRequest      = WDI_CLOSE_REQ;
  wdiEventData.pEventData      = NULL;
  wdiEventData.uEventDataSize  = 0;
  wdiEventData.pCBfnc          = NULL;
  wdiEventData.pUserData       = NULL;

  gWDIInitialized = eWLAN_PAL_FALSE;

  wptStatus = WDI_PostMainEvent(&gWDICb, WDI_CLOSE_EVENT, &wdiEventData);

  /*                                            
                                                     */
  if ( eWLAN_PAL_STATUS_SUCCESS == eventStatus )
  {
     eventStatus = wpalEventWait(&gWDICb.wctsActionEvent,
                                 WDI_WCTS_ACTION_TIMEOUT);
     if ( eWLAN_PAL_STATUS_SUCCESS != eventStatus )
     {
        WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_WARN,
                   "%s: Failed to wait on WCTS action event", __FUNCTION__);
     }
  }

  /*                               */
  wptStatus = wpalEventDelete(&gWDICb.wctsActionEvent);
  if ( eWLAN_PAL_STATUS_SUCCESS !=  wptStatus )
  {
     WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_WARN,
                "WDI Close failed to destroy an event");
     WDI_ASSERT(0);
  }

   /*                                   */
   wptStatus = wpalEventDelete(&gWDICb.setPowerStateEvent);
   if ( eWLAN_PAL_STATUS_SUCCESS !=  wptStatus )
   {
      WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_WARN,
                "WDI Close failed to destroy an event");

      WDI_ASSERT(0);
   }

  /*                                                                        
                                       
                                                                           */
  if ( WDI_STATUS_SUCCESS != WDI_DP_UtilsExit(&gWDICb))
  {
    WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_WARN,
              "WDI Init failed to close the DP Util Module");

    WDI_ASSERT(0);
  }

  /*                                       */
  for ( i = 0; i < WDI_MAX_BSS_SESSIONS; i++ )
  {
    wpal_list_destroy(&(gWDICb.aBSSSessions[i].wptPendingQueue));
  }

  /*                                               */
  wpal_list_destroy(&(gWDICb.wptPendingAssocSessionIdQueue));

  /*                                      */
  wpal_list_destroy(&(gWDICb.wptPendingQueue));

  /*                           */
  wptStatus = wpalTimerDelete( &gWDICb.wptResponseTimer);

  /*                                  */
  wptStatus = wpalMutexDelete(&gWDICb.wptMutex);
  if ( eWLAN_PAL_STATUS_SUCCESS !=  wptStatus )
  {
     WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_ERROR,
                "Failed to delete mutex %d", wptStatus);
     WDI_ASSERT(0);
  }

  /*                                                           
                                                 */
  WDI_CleanCB(&gWDICb);

  return wptStatus;

}/*         */

/* 
                                                                     
                                                  
                                                         

                                                            
                            

                                         


                                 

                                                              

                                    
*/
WDI_Status
WDI_Shutdown
(
 wpt_boolean closeTransport
)
{
   WDI_EventInfoType      wdiEventData;
   wpt_status             wptStatus;
   int                    i = 0;
   /*                                                                        */

   /*                                                                        
                 
                                                                             */
   if ( eWLAN_PAL_FALSE == gWDIInitialized )
   {
      WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_ERROR,
            "WDI API call before module is initialized - Fail request");

      return WDI_STATUS_E_NOT_ALLOWED;
   }

   /*                                                                        
                                                
                                                                             */
   wdiEventData.wdiRequest      = WDI_SHUTDOWN_REQ;
   wdiEventData.pEventData      = NULL;
   wdiEventData.uEventDataSize  = 0;

   /*                                                                
                                               */
   wptStatus = WDI_PostMainEvent(&gWDICb, WDI_SHUTDOWN_EVENT, &wdiEventData);
   if ( eWLAN_PAL_STATUS_SUCCESS != wptStatus )
   {
      WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_FATAL,
            "%s: Failed to process shutdown event", __FUNCTION__);
   }
   /*                                   */
   wptStatus = wpalEventDelete(&gWDICb.setPowerStateEvent);
   if ( eWLAN_PAL_STATUS_SUCCESS !=  wptStatus )
   {
      WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_WARN,
            "WDI Close failed to destroy an event");

      WDI_ASSERT(0);
   }
   /*                                                                        
                                        
                                                                             */
   if ( WDI_STATUS_SUCCESS != WDI_DP_UtilsExit(&gWDICb))
   {
      WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_WARN,
            "WDI Init failed to close the DP Util Module");

      WDI_ASSERT(0);
   }
   if ( closeTransport )
   {
      /*                                                    */
      WCTS_CloseTransport(gWDICb.wctsHandle);
   }
   else
   {
      /*                                                           
                                                      */
      WCTS_ClearPendingQueue(gWDICb.wctsHandle);
   }
   /*                                       */
   for ( i = 0; i < WDI_MAX_BSS_SESSIONS; i++ )
   {
      wpal_list_destroy(&(gWDICb.aBSSSessions[i].wptPendingQueue));
   }

   /*                                               */
   wpal_list_destroy(&(gWDICb.wptPendingAssocSessionIdQueue));
   /*                                      */
   wpal_list_destroy(&(gWDICb.wptPendingQueue));
   /*                           */
   wptStatus = wpalTimerDelete( &gWDICb.wptResponseTimer);

   /*                                  */
   wptStatus = wpalMutexDelete(&gWDICb.wptMutex);
   if ( eWLAN_PAL_STATUS_SUCCESS !=  wptStatus )
   {
      WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_ERROR,
            "%s: Failed to delete mutex %d",  __FUNCTION__, wptStatus);
      WDI_ASSERT(0);
   }
   /*                           */
   wpalMemoryFree(gpHostWlanFeatCaps);
   wpalMemoryFree(gpFwWlanFeatCaps);
   gpHostWlanFeatCaps = NULL;
   gpFwWlanFeatCaps = NULL;
   /*                                                           
                                                  */
   WDI_CleanCB(&gWDICb);
   return wptStatus;

}/*            */


/*                                                                        

                                      

                                                                          */

/* 
                                                               
                                                               
                                                              
                                                       
                                              

                                                                
                                      

                                 

                                                                
                                             

                                                                
                                                           

                                                         
                

               
                                    
*/
WDI_Status
WDI_InitScanReq
(
  WDI_InitScanReqParamsType*  pwdiInitScanParams,
  WDI_InitScanRspCb           wdiInitScanRspCb,
  void*                       pUserData
)
{
  WDI_EventInfoType      wdiEventData;
  /*                                                                        */

  /*                                                                        
                
                                                                          */
  if ( eWLAN_PAL_FALSE == gWDIInitialized )
  {
    WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_ERROR,
              "WDI API call before module is initialized - Fail request");

    return WDI_STATUS_E_NOT_ALLOWED;
  }

  /*                                                                        
                                               
                                                                          */
  wdiEventData.wdiRequest      = WDI_INIT_SCAN_REQ;
  wdiEventData.pEventData      = pwdiInitScanParams;
  wdiEventData.uEventDataSize  = sizeof(*pwdiInitScanParams);
  wdiEventData.pCBfnc          = wdiInitScanRspCb;
  wdiEventData.pUserData       = pUserData;

  return WDI_PostMainEvent(&gWDICb, WDI_REQUEST_EVENT, &wdiEventData);

}/*               */

/* 
                                                          
                                                             
                                                            
                                                               
                                              

                                                                
                                      

                                       

                                                        
                                                       

                                                        
                                                              
              

                                                         
                

                     
                                    
*/
WDI_Status
WDI_StartScanReq
(
  WDI_StartScanReqParamsType*  pwdiStartScanParams,
  WDI_StartScanRspCb           wdiStartScanRspCb,
  void*                        pUserData
)
{
  WDI_EventInfoType      wdiEventData;
  /*                                                                        */

  /*                                                                        
                
                                                                          */
  if ( eWLAN_PAL_FALSE == gWDIInitialized )
  {
    WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_ERROR,
              "WDI API call before module is initialized - Fail request");

    return WDI_STATUS_E_NOT_ALLOWED;
  }

  /*                                                                        
                                               
                                                                          */
  wdiEventData.wdiRequest      = WDI_START_SCAN_REQ;
  wdiEventData.pEventData      = pwdiStartScanParams;
  wdiEventData.uEventDataSize  = sizeof(*pwdiStartScanParams);
  wdiEventData.pCBfnc          = wdiStartScanRspCb;
  wdiEventData.pUserData       = pUserData;

  return WDI_PostMainEvent(&gWDICb, WDI_REQUEST_EVENT, &wdiEventData);

}/*                */


/* 
                                                           
                                                              
                                                                
                                                            
                                                             
                                              

                                                                
                                      

                                        

                                                              
                                             

                                                               
                                                          

                                                         
                

                      
                                    
*/
WDI_Status
WDI_EndScanReq
(
  WDI_EndScanReqParamsType* pwdiEndScanParams,
  WDI_EndScanRspCb          wdiEndScanRspCb,
  void*                     pUserData
)
{
  WDI_EventInfoType      wdiEventData;
  /*                                                                        */

  /*                                                                        
                
                                                                          */
  if ( eWLAN_PAL_FALSE == gWDIInitialized )
  {
    WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_ERROR,
              "WDI API call before module is initialized - Fail request");

    return WDI_STATUS_E_NOT_ALLOWED;
  }

  /*                                                                        
                                               
                                                                          */
  wdiEventData.wdiRequest      = WDI_END_SCAN_REQ;
  wdiEventData.pEventData      = pwdiEndScanParams;
  wdiEventData.uEventDataSize  = sizeof(*pwdiEndScanParams);
  wdiEventData.pCBfnc          = wdiEndScanRspCb;
  wdiEventData.pUserData       = pUserData;

  return WDI_PostMainEvent(&gWDICb, WDI_REQUEST_EVENT, &wdiEventData);

}/*              */


/* 
                                                               
                                                               
                                                              
                                                             
                                              

                                                                
                                      

                                       

                                                           
                                                       

                                                         
                                                               
              

                                                         
                

                     
                                    
*/
WDI_Status
WDI_FinishScanReq
(
  WDI_FinishScanReqParamsType* pwdiFinishScanParams,
  WDI_FinishScanRspCb          wdiFinishScanRspCb,
  void*                        pUserData
)
{
  WDI_EventInfoType      wdiEventData;
  /*                                                                        */

  /*                                                                        
                
                                                                          */
  if ( eWLAN_PAL_FALSE == gWDIInitialized )
  {
    WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_ERROR,
              "WDI API call before module is initialized - Fail request");

    return WDI_STATUS_E_NOT_ALLOWED;
  }

  /*                                                                        
                                               
                                                                          */
  wdiEventData.wdiRequest      = WDI_FINISH_SCAN_REQ;
  wdiEventData.pEventData      = pwdiFinishScanParams;
  wdiEventData.uEventDataSize  = sizeof(*pwdiFinishScanParams);
  wdiEventData.pCBfnc          = wdiFinishScanRspCb;
  wdiEventData.pUserData       = pUserData;

  return WDI_PostMainEvent(&gWDICb, WDI_REQUEST_EVENT, &wdiEventData);

}/*                 */

/*                                                                        

                                          

                                                                          */

/* 
                                                              
                                                            
                                                              
                                                            
                                

                                                                
                                      

                                 

                                                          
                                          

                                                               
                                                   

                                                         
                

               
                                    
*/
WDI_Status
WDI_JoinReq
(
  WDI_JoinReqParamsType* pwdiJoinParams,
  WDI_JoinRspCb          wdiJoinRspCb,
  void*                  pUserData
)
{
  WDI_EventInfoType      wdiEventData;
  /*                                                                        */

  /*                                                                        
                
                                                                          */
  if ( eWLAN_PAL_FALSE == gWDIInitialized )
  {
    WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_ERROR,
              "WDI API call before module is initialized - Fail request");

    return WDI_STATUS_E_NOT_ALLOWED;
  }

  /*                                                                        
                                               
                                                                          */
  wdiEventData.wdiRequest      = WDI_JOIN_REQ;
  wdiEventData.pEventData      = pwdiJoinParams;
  wdiEventData.uEventDataSize  = sizeof(*pwdiJoinParams);
  wdiEventData.pCBfnc          = wdiJoinRspCb;
  wdiEventData.pUserData       = pUserData;

  return WDI_PostMainEvent(&gWDICb, WDI_REQUEST_EVENT, &wdiEventData);

}/*           */

/* 
                                                          
                                                               
                                                                
                                                                
                                                               
                

                                                                
                                      

                                   

                                                        
                                                       

                                                        
                                                              
              

                                                         
                

                 
                                    
*/
WDI_Status
WDI_ConfigBSSReq
(
  WDI_ConfigBSSReqParamsType* pwdiConfigBSSParams,
  WDI_ConfigBSSRspCb          wdiConfigBSSRspCb,
  void*                       pUserData
)
{
  WDI_EventInfoType      wdiEventData;
  /*                                                                        */

  /*                                                                        
                
                                                                          */
  if ( eWLAN_PAL_FALSE == gWDIInitialized )
  {
    WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_ERROR,
              "WDI API call before module is initialized - Fail request");

    return WDI_STATUS_E_NOT_ALLOWED;
  }

  /*                                                                        
                                               
                                                                          */
  wdiEventData.wdiRequest      = WDI_CONFIG_BSS_REQ;
  wdiEventData.pEventData      = pwdiConfigBSSParams;
  wdiEventData.uEventDataSize  = sizeof(*pwdiConfigBSSParams);
  wdiEventData.pCBfnc          = wdiConfigBSSRspCb;
  wdiEventData.pUserData       = pUserData;

  return WDI_PostMainEvent(&gWDICb, WDI_REQUEST_EVENT, &wdiEventData);

}/*                */

/* 
                                                          
                                                            
                                                            
                                                            
                                              

                                                                
                                      

                                                            

                                                               
                                          

                                                              
                                                         

                                                         
                

                                        
                                    
*/
WDI_Status
WDI_DelBSSReq
(
  WDI_DelBSSReqParamsType* pwdiDelBSSParams,
  WDI_DelBSSRspCb          wdiDelBSSRspCb,
  void*                    pUserData
)
{
  WDI_EventInfoType      wdiEventData;
  /*                                                                        */

  /*                                                                        
                
                                                                          */
  if ( eWLAN_PAL_FALSE == gWDIInitialized )
  {
    WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_ERROR,
              "WDI API call before module is initialized - Fail request");

    return WDI_STATUS_E_NOT_ALLOWED;
  }

  /*                                                                        
                                               
                                                                          */
  wdiEventData.wdiRequest      = WDI_DEL_BSS_REQ;
  wdiEventData.pEventData      = pwdiDelBSSParams;
  wdiEventData.uEventDataSize  = sizeof(*pwdiDelBSSParams);
  wdiEventData.pCBfnc          = wdiDelBSSRspCb;
  wdiEventData.pUserData       = pUserData;

  return WDI_PostMainEvent(&gWDICb, WDI_REQUEST_EVENT, &wdiEventData);

}/*             */

/* 
                                                              
                                                          
                                                                
                                                              
                                                             

                                                                
                                      

                                   

                                                                
                                             

                                                        
                                                              
              

                                                         
                

                 
                                    
*/
WDI_Status
WDI_PostAssocReq
(
  WDI_PostAssocReqParamsType* pwdiPostAssocReqParams,
  WDI_PostAssocRspCb          wdiPostAssocRspCb,
  void*                       pUserData
)
{
  WDI_EventInfoType      wdiEventData;
  /*                                                                        */

  /*                                                                        
                
                                                                          */
  if ( eWLAN_PAL_FALSE == gWDIInitialized )
  {
    WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_ERROR,
              "WDI API call before module is initialized - Fail request");

    return WDI_STATUS_E_NOT_ALLOWED;
  }

  /*                                                                        
                                               
                                                                          */
  wdiEventData.wdiRequest      = WDI_POST_ASSOC_REQ;
  wdiEventData.pEventData      = pwdiPostAssocReqParams;
  wdiEventData.uEventDataSize  = sizeof(*pwdiPostAssocReqParams);
  wdiEventData.pCBfnc          = wdiPostAssocRspCb;
  wdiEventData.pUserData       = pUserData;

  return WDI_PostMainEvent(&gWDICb, WDI_REQUEST_EVENT, &wdiEventData);

}/*                */

/* 
                                                               
                                                              
                                                              
                                                         
                                                               
                

                                                                
                                      

                                        

                                                               
                                          

                                                              
                                                         

                                                         
                

                      
                                    
*/
WDI_Status
WDI_DelSTAReq
(
  WDI_DelSTAReqParamsType* pwdiDelSTAParams,
  WDI_DelSTARspCb          wdiDelSTARspCb,
  void*                    pUserData
)
{
  WDI_EventInfoType      wdiEventData;
  /*                                                                        */

  /*                                                                        
                
                                                                          */
  if ( eWLAN_PAL_FALSE == gWDIInitialized )
  {
    WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_ERROR,
              "WDI API call before module is initialized - Fail request");

    return WDI_STATUS_E_NOT_ALLOWED;
  }

  /*                                                                        
                                               
                                                                          */
  wdiEventData.wdiRequest      = WDI_DEL_STA_REQ;
  wdiEventData.pEventData      = pwdiDelSTAParams;
  wdiEventData.uEventDataSize  = sizeof(*pwdiDelSTAParams);
  wdiEventData.pCBfnc          = wdiDelSTARspCb;
  wdiEventData.pUserData       = pUserData;

  return WDI_PostMainEvent(&gWDICb, WDI_REQUEST_EVENT, &wdiEventData);

}/*             */

/*                                                                        

                                          

                                                                          */

/* 
                                                                   
                                                                     
                                                                 
                                                               
                

                                                                
                                      

                                        

                                                         
                                                       

                                                        
                                                               
              

                                                         
                

                      
                                    
*/
WDI_Status
WDI_SetBSSKeyReq
(
  WDI_SetBSSKeyReqParamsType* pwdiSetBSSKeyParams,
  WDI_SetBSSKeyRspCb          wdiSetBSSKeyRspCb,
  void*                       pUserData
)
{
  WDI_EventInfoType      wdiEventData;
  /*                                                                        */

  /*                                                                        
                
                                                                          */
  if ( eWLAN_PAL_FALSE == gWDIInitialized )
  {
    WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_ERROR,
              "WDI API call before module is initialized - Fail request");

    return WDI_STATUS_E_NOT_ALLOWED;
  }

  /*                                                                        
                                               
                                                                          */
  wdiEventData.wdiRequest      = WDI_SET_BSS_KEY_REQ;
  wdiEventData.pEventData      = pwdiSetBSSKeyParams;
  wdiEventData.uEventDataSize  = sizeof(*pwdiSetBSSKeyParams);
  wdiEventData.pCBfnc          = wdiSetBSSKeyRspCb;
  wdiEventData.pUserData       = pUserData;

  return WDI_PostMainEvent(&gWDICb, WDI_REQUEST_EVENT, &wdiEventData);

}/*                */

/* 
                                                                      
                                                                  
                                                                
                                                               
                

                                                                
                                      

                                        

                                                               
                                                       

                                                           
                                                              
                  

                                                         
                

                      
                                    
*/
WDI_Status
WDI_RemoveBSSKeyReq
(
  WDI_RemoveBSSKeyReqParamsType* pwdiRemoveBSSKeyParams,
  WDI_RemoveBSSKeyRspCb          wdiRemoveBSSKeyRspCb,
  void*                          pUserData
)
{
  WDI_EventInfoType      wdiEventData;
  /*                                                                        */

  /*                                                                        
                
                                                                          */
  if ( eWLAN_PAL_FALSE == gWDIInitialized )
  {
    WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_ERROR,
              "WDI API call before module is initialized - Fail request");

    return WDI_STATUS_E_NOT_ALLOWED;
  }

  /*                                                                        
                                               
                                                                          */
  wdiEventData.wdiRequest      = WDI_RMV_BSS_KEY_REQ;
  wdiEventData.pEventData      = pwdiRemoveBSSKeyParams;
  wdiEventData.uEventDataSize  = sizeof(*pwdiRemoveBSSKeyParams);
  wdiEventData.pCBfnc          = wdiRemoveBSSKeyRspCb;
  wdiEventData.pUserData       = pUserData;

  return WDI_PostMainEvent(&gWDICb, WDI_REQUEST_EVENT, &wdiEventData);

}/*                   */


/* 
                                                             
                                                              
                                                              
                                                         
                                              

                                                                
                                      

                                        

                                                         
                                                       

                                                        
                                                               
              

                                                         
                

                      
                                    
*/
WDI_Status
WDI_SetSTAKeyReq
(
  WDI_SetSTAKeyReqParamsType* pwdiSetSTAKeyParams,
  WDI_SetSTAKeyRspCb          wdiSetSTAKeyRspCb,
  void*                       pUserData
)
{
  WDI_EventInfoType      wdiEventData;
  /*                                                                        */

  /*                                                                        
                
                                                                          */
  if ( eWLAN_PAL_FALSE == gWDIInitialized )
  {
    WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_ERROR,
              "WDI API call before module is initialized - Fail request");

    return WDI_STATUS_E_NOT_ALLOWED;
  }

  /*                                                                        
                                               
                                                                          */
  wdiEventData.wdiRequest      = WDI_SET_STA_KEY_REQ;
  wdiEventData.pEventData      = pwdiSetSTAKeyParams;
  wdiEventData.uEventDataSize  = sizeof(*pwdiSetSTAKeyParams);
  wdiEventData.pCBfnc          = wdiSetSTAKeyRspCb;
  wdiEventData.pUserData       = pUserData;

  return WDI_PostMainEvent(&gWDICb, WDI_REQUEST_EVENT, &wdiEventData);

}/*                */


/* 
                                                             
                                                               
                                                              
                                                            
                                              

                                                                
                                      

                                        

                                                               
                                                       

                                                           
                                                              
                  

                                                         
                

                      
                                    
*/
WDI_Status
WDI_RemoveSTAKeyReq
(
  WDI_RemoveSTAKeyReqParamsType* pwdiRemoveSTAKeyParams,
  WDI_RemoveSTAKeyRspCb          wdiRemoveSTAKeyRspCb,
  void*                          pUserData
)
{
  WDI_EventInfoType      wdiEventData;
  /*                                                                        */

  /*                                                                        
                
                                                                          */
  if ( eWLAN_PAL_FALSE == gWDIInitialized )
  {
    WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_ERROR,
              "WDI API call before module is initialized - Fail request");

    return WDI_STATUS_E_NOT_ALLOWED;
  }

  /*                                                                        
                                               
                                                                          */
  wdiEventData.wdiRequest      = WDI_RMV_STA_KEY_REQ;
  wdiEventData.pEventData      = pwdiRemoveSTAKeyParams;
  wdiEventData.uEventDataSize  = sizeof(*pwdiRemoveSTAKeyParams);
  wdiEventData.pCBfnc          = wdiRemoveSTAKeyRspCb;
  wdiEventData.pUserData       = pUserData;

  return WDI_PostMainEvent(&gWDICb, WDI_REQUEST_EVENT, &wdiEventData);

}/*                   */


/* 
                                                               
                                                              
                                                            
                                                          
                                              

                                                                
                                      

                                        

                                                               
                                                       

                                                             
                                                               
              

                                                         
                

                      
                                    
*/
WDI_Status
WDI_SetSTABcastKeyReq
(
  WDI_SetSTAKeyReqParamsType* pwdiSetSTABcastKeyParams,
  WDI_SetSTAKeyRspCb          wdiSetSTABcastKeyRspCb,
  void*                       pUserData
)

{
  WDI_EventInfoType      wdiEventData;
  /*                                                                        */

  /*                                                                        
                
                                                                          */
  if ( eWLAN_PAL_FALSE == gWDIInitialized )
  {
    WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_ERROR,
              "WDI API call before module is initialized - Fail request");

    return WDI_STATUS_E_NOT_ALLOWED;
  }

  /*                                                                        
                                               
                                                                          */
  wdiEventData.wdiRequest      = WDI_SET_STA_BCAST_KEY_REQ;
  wdiEventData.pEventData      = pwdiSetSTABcastKeyParams;
  wdiEventData.uEventDataSize  = sizeof(*pwdiSetSTABcastKeyParams);
  wdiEventData.pCBfnc          = wdiSetSTABcastKeyRspCb;
  wdiEventData.pUserData       = pUserData;

  return WDI_PostMainEvent(&gWDICb, WDI_REQUEST_EVENT, &wdiEventData);

}/*                     */

/* 
                                                              
                                                                
                                                              
                                                              
                                              

                                                                
                                      

                                             

                                                       
                                                           
                               

                                                                
                                                               
                       

                                                         
                

                           
                                    
*/
WDI_Status
WDI_RemoveSTABcastKeyReq
(
  WDI_RemoveSTAKeyReqParamsType* pwdiRemoveSTABcastKeyParams,
  WDI_RemoveSTAKeyRspCb          wdiRemoveSTABcastKeyRspCb,
  void*                          pUserData
)
{
  WDI_EventInfoType      wdiEventData;
  /*                                                                        */

  /*                                                                        
                
                                                                          */
  if ( eWLAN_PAL_FALSE == gWDIInitialized )
  {
    WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_ERROR,
              "WDI API call before module is initialized - Fail request");

    return WDI_STATUS_E_NOT_ALLOWED;
  }

  /*                                                                        
                                               
                                                                          */
  wdiEventData.wdiRequest      = WDI_RMV_STA_BCAST_KEY_REQ;
  wdiEventData.pEventData      = pwdiRemoveSTABcastKeyParams;
  wdiEventData.uEventDataSize  = sizeof(*pwdiRemoveSTABcastKeyParams);
  wdiEventData.pCBfnc          = wdiRemoveSTABcastKeyRspCb;
  wdiEventData.pUserData       = pUserData;

  return WDI_PostMainEvent(&gWDICb, WDI_REQUEST_EVENT, &wdiEventData);

}/*                        */

/* 
                                                          
                                                     
                                                              
                                                              
                                              

                                                                
                                      

                                             

                                                       
                                                           
                               

                                                                
                                                               
                       

                                                         
                

                          
                                    
*/
WDI_Status
WDI_SetMaxTxPowerReq
(
  WDI_SetMaxTxPowerParamsType*   pwdiSetMaxTxPowerParams,
  WDA_SetMaxTxPowerRspCb         wdiReqStatusCb,
  void*                          pUserData
)
{
  WDI_EventInfoType      wdiEventData;
  /*                                                                        */

  /*                                                                        
                
                                                                          */
  if ( eWLAN_PAL_FALSE == gWDIInitialized )
  {
    WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_ERROR,
              "WDI API call before module is initialized - Fail request");

    return WDI_STATUS_E_NOT_ALLOWED;
  }

  /*                                                                        
                                               
                                                                          */
  wdiEventData.wdiRequest      = WDI_SET_MAX_TX_POWER_REQ;
  wdiEventData.pEventData      = pwdiSetMaxTxPowerParams;
  wdiEventData.uEventDataSize  = sizeof(*pwdiSetMaxTxPowerParams);
  wdiEventData.pCBfnc          = wdiReqStatusCb;
  wdiEventData.pUserData       = pUserData;

  return WDI_PostMainEvent(&gWDICb, WDI_REQUEST_EVENT, &wdiEventData);
}

#ifdef FEATURE_WLAN_CCX
WDI_Status
WDI_TSMStatsReq
(
   WDI_TSMStatsReqParamsType*        pwdiTsmReqParams,
   WDI_TsmRspCb                 wdiReqStatusCb,
   void*                        pUserData
)
{
  WDI_EventInfoType wdiEventData;
  /*                                                                        */
  /*                                                                        
                
                                                                          */
  if ( eWLAN_PAL_FALSE == gWDIInitialized )
  {
    WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_ERROR,
              "WDI API call before module is initialized - Fail request");

    return WDI_STATUS_E_NOT_ALLOWED;
  }

  /*                                                                        
                                               
                                                                          */
  wdiEventData.wdiRequest      = WDI_TSM_STATS_REQ;
  wdiEventData.pEventData      = pwdiTsmReqParams;
  wdiEventData.uEventDataSize  = sizeof(*pwdiTsmReqParams);
  wdiEventData.pCBfnc          = wdiReqStatusCb;
  wdiEventData.pUserData       = pUserData;

  return WDI_PostMainEvent(&gWDICb, WDI_REQUEST_EVENT, &wdiEventData);

}
#endif

/*                                                                        

                                           

                                                                          */

/* 
                                                                
                                                            
                                                              
                                                            
                                                              
                                                              
                                

                                                                
                                      

                                        

                                                                
                                          

                                                                
                                                     

                                                         
                

                      
                                    
*/
WDI_Status
WDI_AddTSReq
(
  WDI_AddTSReqParamsType* pwdiAddTsReqParams,
  WDI_AddTsRspCb          wdiAddTsRspCb,
  void*                   pUserData
)
{
  WDI_EventInfoType      wdiEventData;
  /*                                                                        */

  /*                                                                        
                
                                                                          */
  if ( eWLAN_PAL_FALSE == gWDIInitialized )
  {
    WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_ERROR,
              "WDI API call before module is initialized - Fail request");

    return WDI_STATUS_E_NOT_ALLOWED;
  }

  /*                                                                        
                                               
                                                                          */
  wdiEventData.wdiRequest      = WDI_ADD_TS_REQ;
  wdiEventData.pEventData      = pwdiAddTsReqParams;
  wdiEventData.uEventDataSize  = sizeof(*pwdiAddTsReqParams);
  wdiEventData.pCBfnc          = wdiAddTsRspCb;
  wdiEventData.pUserData       = pUserData;

  return WDI_PostMainEvent(&gWDICb, WDI_REQUEST_EVENT, &wdiEventData);

}/*            */



/* 
                                                                
                                                             
                                                             
                                                             
                                                              
                         

                                                                
                                      

                                    

                                                                
                                          

                                                                
                                                     

                                                         
                

                  
                                    
*/
WDI_Status
WDI_DelTSReq
(
  WDI_DelTSReqParamsType* pwdiDelTsReqParams,
  WDI_DelTsRspCb          wdiDelTsRspCb,
  void*                   pUserData
)
{
  WDI_EventInfoType      wdiEventData;
  /*                                                                        */

  /*                                                                        
                
                                                                          */
  if ( eWLAN_PAL_FALSE == gWDIInitialized )
  {
    WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_ERROR,
              "WDI API call before module is initialized - Fail request");

    return WDI_STATUS_E_NOT_ALLOWED;
  }

  /*                                                                        
                                               
                                                                          */
  wdiEventData.wdiRequest      = WDI_DEL_TS_REQ;
  wdiEventData.pEventData      = pwdiDelTsReqParams;
  wdiEventData.uEventDataSize  = sizeof(*pwdiDelTsReqParams);
  wdiEventData.pCBfnc          = wdiDelTsRspCb;
  wdiEventData.pUserData       = pUserData;

  return WDI_PostMainEvent(&gWDICb, WDI_REQUEST_EVENT, &wdiEventData);

}/*            */



/* 
                                                              
                                                               
                                                            
                                                           
                                                               
                

                                                                
                                      

                                        

                                                              
                                             

                                                               
                                                                

                                                         
                

                      
                                    
*/
WDI_Status
WDI_UpdateEDCAParams
(
  WDI_UpdateEDCAParamsType*    pwdiUpdateEDCAParams,
  WDI_UpdateEDCAParamsRspCb    wdiUpdateEDCAParamsRspCb,
  void*                        pUserData
)
{
  WDI_EventInfoType      wdiEventData;
  /*                                                                        */

  /*                                                                        
                
                                                                          */
  if ( eWLAN_PAL_FALSE == gWDIInitialized )
  {
    WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_ERROR,
              "WDI API call before module is initialized - Fail request");

    return WDI_STATUS_E_NOT_ALLOWED;
  }

  /*                                                                        
                                               
                                                                          */
  wdiEventData.wdiRequest      = WDI_UPD_EDCA_PRMS_REQ;
  wdiEventData.pEventData      = pwdiUpdateEDCAParams;
  wdiEventData.uEventDataSize  = sizeof(*pwdiUpdateEDCAParams);
  wdiEventData.pCBfnc          = wdiUpdateEDCAParamsRspCb;
  wdiEventData.pUserData       = pUserData;

  return WDI_PostMainEvent(&gWDICb, WDI_REQUEST_EVENT, &wdiEventData);

}/*                    */


/* 
                                                                       
                                                                
                                                                
                                                             
                                                              
                         

                                                                
                                      

                                        

                                                                
                                          

                                                                
                                                     

                                                         
                

                      
                                    
*/
WDI_Status
WDI_AddBASessionReq
(
  WDI_AddBASessionReqParamsType* pwdiAddBASessionReqParams,
  WDI_AddBASessionRspCb          wdiAddBASessionRspCb,
  void*                          pUserData
)
{
  WDI_EventInfoType      wdiEventData;
  /*                                                                        */

  /*                                                                        
                
                                                                          */
  if ( eWLAN_PAL_FALSE == gWDIInitialized )
  {
    WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_ERROR,
              "WDI API call before module is initialized - Fail request");

    return WDI_STATUS_E_NOT_ALLOWED;
  }

  /*                                                                        
                                               
                                                                          */
  wdiEventData.wdiRequest      = WDI_ADD_BA_SESSION_REQ;
  wdiEventData.pEventData      = pwdiAddBASessionReqParams;
  wdiEventData.uEventDataSize  = sizeof(*pwdiAddBASessionReqParams);
  wdiEventData.pCBfnc          = wdiAddBASessionRspCb;
  wdiEventData.pUserData       = pUserData;

  return WDI_PostMainEvent(&gWDICb, WDI_REQUEST_EVENT, &wdiEventData);

}/*                   */

/* 
                                                               
                                                             
                                                            
                                                               
                                                   

                                                                
                                      

                                    

                                                                
                                          

                                                                
                                                     

                                                         
                

                  
                                    
*/
WDI_Status
WDI_DelBAReq
(
  WDI_DelBAReqParamsType* pwdiDelBAReqParams,
  WDI_DelBARspCb          wdiDelBARspCb,
  void*                   pUserData
)
{
  WDI_EventInfoType      wdiEventData;
  /*                                                                        */

  /*                                                                        
                
                                                                          */
  if ( eWLAN_PAL_FALSE == gWDIInitialized )
  {
    WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_ERROR,
              "WDI API call before module is initialized - Fail request");

    return WDI_STATUS_E_NOT_ALLOWED;
  }

  /*                                                                        
                                               
                                                                          */
  wdiEventData.wdiRequest      = WDI_DEL_BA_REQ;
  wdiEventData.pEventData      = pwdiDelBAReqParams;
  wdiEventData.uEventDataSize  = sizeof(*pwdiDelBAReqParams);
  wdiEventData.pCBfnc          = wdiDelBARspCb;
  wdiEventData.pUserData       = pUserData;

  return WDI_PostMainEvent(&gWDICb, WDI_REQUEST_EVENT, &wdiEventData);

}/*            */

/*                                                                        

                                           

                                                                          */

/* 
                                                              
                                                             
                                                               
                                                              
                                                             

                                                                
                                      

                                 

                                                          
                                                       

                                                         
                                                             
                       

                                                         
                

               
                                    
*/
WDI_Status
WDI_SetPwrSaveCfgReq
(
  WDI_UpdateCfgReqParamsType*   pwdiPowerSaveCfg,
  WDI_SetPwrSaveCfgCb     wdiSetPwrSaveCfgCb,
  void*                   pUserData
)
{
  WDI_EventInfoType      wdiEventData;
  /*                                                                        */

  /*                                                                        
                
                                                                          */
  if ( eWLAN_PAL_FALSE == gWDIInitialized )
  {
    WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_ERROR,
              "WDI API call before module is initialized - Fail request");

    return WDI_STATUS_E_NOT_ALLOWED;
  }

  /*                                                                        
                                               
                                                                          */
  wdiEventData.wdiRequest      = WDI_UPDATE_CFG_REQ;
  wdiEventData.pEventData      = pwdiPowerSaveCfg;
  wdiEventData.uEventDataSize  = sizeof(*pwdiPowerSaveCfg);
  wdiEventData.pCBfnc          = wdiSetPwrSaveCfgCb;
  wdiEventData.pUserData       = pUserData;

  return WDI_PostMainEvent(&gWDICb, WDI_REQUEST_EVENT, &wdiEventData);

}/*                    */

/* 
                                                             
                                                             
                                                               
                                                                
                            

                                                                
                                      


                                                        
                                                              
              

                                                         
                

               
                                    
*/
WDI_Status
WDI_EnterImpsReq
(
   WDI_EnterImpsRspCb  wdiEnterImpsRspCb,
   void*                   pUserData
)
{
  WDI_EventInfoType      wdiEventData;
  /*                                                                        */

  /*                                                                        
                
                                                                          */
  if ( eWLAN_PAL_FALSE == gWDIInitialized )
  {
    WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_ERROR,
              "WDI API call before module is initialized - Fail request");

    return WDI_STATUS_E_NOT_ALLOWED;
  }

  /*                                                                        
                                               
                                                                          */
  wdiEventData.wdiRequest      = WDI_ENTER_IMPS_REQ;
  wdiEventData.pEventData      = NULL;
  wdiEventData.uEventDataSize  = 0;
  wdiEventData.pCBfnc          = wdiEnterImpsRspCb;
  wdiEventData.pUserData       = pUserData;

  return WDI_PostMainEvent(&gWDICb, WDI_REQUEST_EVENT, &wdiEventData);

}/*                */

/* 
                                                            
                                                               
                                                              
                                                                
                            

                                                                
                                      



                                                                
                                                           

                                                         
                

               
                                    
*/
WDI_Status
WDI_ExitImpsReq
(
   WDI_ExitImpsRspCb  wdiExitImpsRspCb,
   void*                   pUserData
)
{
  WDI_EventInfoType      wdiEventData;
  /*                                                                        */

  /*                                                                        
                
                                                                          */
  if ( eWLAN_PAL_FALSE == gWDIInitialized )
  {
    WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_ERROR,
              "WDI API call before module is initialized - Fail request");

    return WDI_STATUS_E_NOT_ALLOWED;
  }

  /*                                                                        
                                               
                                                                          */
  wdiEventData.wdiRequest      = WDI_EXIT_IMPS_REQ;
  wdiEventData.pEventData      = NULL;
  wdiEventData.uEventDataSize  = 0;
  wdiEventData.pCBfnc          = wdiExitImpsRspCb;
  wdiEventData.pUserData       = pUserData;

  return WDI_PostMainEvent(&gWDICb, WDI_REQUEST_EVENT, &wdiEventData);

}/*               */

/* 
                                                             
                                                             
                                                              
                                                        
                                              

                                                                
                                      

                                        

                                                            
                                                       

                                                        
                                                              
              

                                                         
                

                      
                                    
*/
WDI_Status
WDI_EnterBmpsReq
(
   WDI_EnterBmpsReqParamsType *pwdiEnterBmpsReqParams,
   WDI_EnterBmpsRspCb  wdiEnterBmpsRspCb,
   void*                   pUserData
)
{
  WDI_EventInfoType      wdiEventData;
  /*                                                                        */

  /*                                                                        
                
                                                                          */
  if ( eWLAN_PAL_FALSE == gWDIInitialized )
  {
    WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_ERROR,
              "WDI API call before module is initialized - Fail request");

    return WDI_STATUS_E_NOT_ALLOWED;
  }

  /*                                                                        
                                               
                                                                          */
  wdiEventData.wdiRequest      = WDI_ENTER_BMPS_REQ;
  wdiEventData.pEventData      = pwdiEnterBmpsReqParams;
  wdiEventData.uEventDataSize  = sizeof(*pwdiEnterBmpsReqParams);
  wdiEventData.pCBfnc          = wdiEnterBmpsRspCb;
  wdiEventData.pUserData       = pUserData;

  return WDI_PostMainEvent(&gWDICb, WDI_REQUEST_EVENT, &wdiEventData);

}/*                */

/* 
                                                            
                                                               
                                                              
                                                       
                                              

                                                                
                                      

                                        

                                                          
                                                       

                                                                
                                                           

                                                         
                

                      
                                    
*/
WDI_Status
WDI_ExitBmpsReq
(
   WDI_ExitBmpsReqParamsType *pwdiExitBmpsReqParams,
   WDI_ExitBmpsRspCb  wdiExitBmpsRspCb,
   void*                   pUserData
)
{
  WDI_EventInfoType      wdiEventData;
  /*                                                                        */

  /*                                                                        
                
                                                                          */
  if ( eWLAN_PAL_FALSE == gWDIInitialized )
  {
    WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_ERROR,
              "WDI API call before module is initialized - Fail request");

    return WDI_STATUS_E_NOT_ALLOWED;
  }

  /*                                                                        
                                               
                                                                          */
  wdiEventData.wdiRequest      = WDI_EXIT_BMPS_REQ;
  wdiEventData.pEventData      = pwdiExitBmpsReqParams;
  wdiEventData.uEventDataSize  = sizeof(*pwdiExitBmpsReqParams);
  wdiEventData.pCBfnc          = wdiExitBmpsRspCb;
  wdiEventData.pUserData       = pUserData;

  return WDI_PostMainEvent(&gWDICb, WDI_REQUEST_EVENT, &wdiEventData);

}/*               */

/* 
                                                              
                                                              
                                                              
                                                         
                                              

                                                                
                                      

                                        
                                               

                                                              
                                                       

                                                         
                                                               
              

                                                         
                

                                               
                                    
*/
WDI_Status
WDI_EnterUapsdReq
(
   WDI_EnterUapsdReqParamsType *pwdiEnterUapsdReqParams,
   WDI_EnterUapsdRspCb  wdiEnterUapsdRspCb,
   void*                   pUserData
)
{
  WDI_EventInfoType      wdiEventData;
  /*                                                                        */

  /*                                                                        
                
                                                                          */
  if ( eWLAN_PAL_FALSE == gWDIInitialized )
  {
    WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_ERROR,
              "WDI API call before module is initialized - Fail request");

    return WDI_STATUS_E_NOT_ALLOWED;
  }

  /*                                                                        
                                               
                                                                          */
  wdiEventData.wdiRequest      = WDI_ENTER_UAPSD_REQ;
  wdiEventData.pEventData      = pwdiEnterUapsdReqParams;
  wdiEventData.uEventDataSize  = sizeof(*pwdiEnterUapsdReqParams);
  wdiEventData.pCBfnc          = wdiEnterUapsdRspCb;
  wdiEventData.pUserData       = pUserData;

  return WDI_PostMainEvent(&gWDICb, WDI_REQUEST_EVENT, &wdiEventData);

}/*                 */

/* 
                                                             
                                                                
                                                              
                                                             
                                

                                                                
                                      

                                        

                                                        
                                                              
              

                                                         
                

                      
                                    
*/
WDI_Status
WDI_ExitUapsdReq
(
   WDI_ExitUapsdRspCb  wdiExitUapsdRspCb,
   void*                   pUserData
)
{
  WDI_EventInfoType      wdiEventData;
  /*                                                                        */

  /*                                                                        
                
                                                                          */
  if ( eWLAN_PAL_FALSE == gWDIInitialized )
  {
    WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_ERROR,
              "WDI API call before module is initialized - Fail request");

    return WDI_STATUS_E_NOT_ALLOWED;
  }

  /*                                                                        
                                               
                                                                          */
  wdiEventData.wdiRequest      = WDI_EXIT_UAPSD_REQ;
  wdiEventData.pEventData      = NULL;
  wdiEventData.uEventDataSize  = 0;
  wdiEventData.pCBfnc          = wdiExitUapsdRspCb;
  wdiEventData.pUserData       = pUserData;

  return WDI_PostMainEvent(&gWDICb, WDI_REQUEST_EVENT, &wdiEventData);

}/*                */

/* 
                                                              
                                                         
                                                                
                                                                
                                                             
                                                             

                                                                
                                      

                                        

                                                      
                                                          

                                                             
                                                              
                       

                                                         
                

                      
                                    
*/
WDI_Status
WDI_UpdateUapsdParamsReq
(
   WDI_UpdateUapsdReqParamsType *pwdiUpdateUapsdReqParams,
   WDI_UpdateUapsdParamsCb  wdiUpdateUapsdParamsCb,
   void*                   pUserData
)
{
  WDI_EventInfoType      wdiEventData;
  /*                                                                        */

  /*                                                                        
                
                                                                          */
  if ( eWLAN_PAL_FALSE == gWDIInitialized )
  {
    WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_ERROR,
              "WDI API call before module is initialized - Fail request");

    return WDI_STATUS_E_NOT_ALLOWED;
  }

  /*                                                                        
                                               
                                                                          */
  wdiEventData.wdiRequest      = WDI_UPDATE_UAPSD_PARAM_REQ;
  wdiEventData.pEventData      = pwdiUpdateUapsdReqParams;
  wdiEventData.uEventDataSize  = sizeof(*pwdiUpdateUapsdReqParams);;
  wdiEventData.pCBfnc          = wdiUpdateUapsdParamsCb;
  wdiEventData.pUserData       = pUserData;

  return WDI_PostMainEvent(&gWDICb, WDI_REQUEST_EVENT, &wdiEventData);

}/*                        */

/* 
                                                             
                                                                
                                                          
                                                                
                                                          
                                                             

                                                                
                                      

                                        

                                                           
                                          

                                                            
                                                                
                  

                                                         
                

                      
                                    
*/
WDI_Status
WDI_SetUapsdAcParamsReq
(
  WDI_SetUapsdAcParamsReqParamsType*      pwdiUapsdInfo,
  WDI_SetUapsdAcParamsCb  wdiSetUapsdAcParamsCb,
  void*                   pUserData
)
{
  WDI_EventInfoType      wdiEventData;
  /*                                                                        */

  /*                                                                        
                
                                                                          */
  if ( eWLAN_PAL_FALSE == gWDIInitialized )
  {
    WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_ERROR,
              "WDI API call before module is initialized - Fail request");

    return WDI_STATUS_E_NOT_ALLOWED;
  }

  /*                                                                        
                                               
                                                                          */
  wdiEventData.wdiRequest      = WDI_SET_UAPSD_PARAM_REQ;
  wdiEventData.pEventData      = pwdiUapsdInfo;
  wdiEventData.uEventDataSize  = sizeof(*pwdiUapsdInfo);
  wdiEventData.pCBfnc          = wdiSetUapsdAcParamsCb;
  wdiEventData.pUserData       = pUserData;

  return WDI_PostMainEvent(&gWDICb, WDI_REQUEST_EVENT, &wdiEventData);

}/*                       */

/* 
                                                               
                                                                
                                                                       
                                                              
                                  

                                                                
                                      


                                                
                                                       
                               

                                                              
                                                               
                       

                                                         
                

                                    
*/
WDI_Status
WDI_ConfigureRxpFilterReq
(
   WDI_ConfigureRxpFilterReqParamsType *pwdiConfigureRxpFilterReqParams,
   WDI_ConfigureRxpFilterCb             wdiConfigureRxpFilterCb,
   void*                                pUserData
)
{
   WDI_EventInfoType      wdiEventData;
   /*                                                                        */

   /*                                                                        
                 
                                                                           */
   if ( eWLAN_PAL_FALSE == gWDIInitialized )
   {
     WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_ERROR,
               "WDI API call before module is initialized - Fail request");

     return WDI_STATUS_E_NOT_ALLOWED;
   }

   /*                                                                        
                                                
                                                                           */
   wdiEventData.wdiRequest      = WDI_CONFIGURE_RXP_FILTER_REQ;
   wdiEventData.pEventData      = pwdiConfigureRxpFilterReqParams;
   wdiEventData.uEventDataSize  = sizeof(*pwdiConfigureRxpFilterReqParams);
   wdiEventData.pCBfnc          = wdiConfigureRxpFilterCb;
   wdiEventData.pUserData       = pUserData;

   return WDI_PostMainEvent(&gWDICb, WDI_REQUEST_EVENT, &wdiEventData);
}/*                         */

/* 
                                                                
                                                            
                                                            
                                                   
                              

                                                                
                                      


                                             
                                                       
                               

                                                        
                                                            
                       

                                                         
                

                                    
*/
WDI_Status
WDI_SetBeaconFilterReq
(
   WDI_BeaconFilterReqParamsType   *pwdiBeaconFilterReqParams,
   WDI_SetBeaconFilterCb            wdiBeaconFilterCb,
   void*                            pUserData
)
{
   WDI_EventInfoType      wdiEventData;
   /*                                                                        */

   /*                                                                        
                 
                                                                           */
   if ( eWLAN_PAL_FALSE == gWDIInitialized )
   {
     WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_ERROR,
               "WDI API call before module is initialized - Fail request");

     return WDI_STATUS_E_NOT_ALLOWED;
   }

   /*                                                                        
                                                
                                                                           */
   wdiEventData.wdiRequest      = WDI_SET_BEACON_FILTER_REQ;
   wdiEventData.pEventData      = pwdiBeaconFilterReqParams;
   wdiEventData.uEventDataSize  = sizeof(*pwdiBeaconFilterReqParams);;
   wdiEventData.pCBfnc          = wdiBeaconFilterCb;
   wdiEventData.pUserData       = pUserData;

   return WDI_PostMainEvent(&gWDICb, WDI_REQUEST_EVENT, &wdiEventData);
}/*                      */

/* 
                                                                
                                                           
                                                               
                                                             
                                             

                                                                
                                      


                                             
                                                       
                               

                                                        
                                                               
                       

                                                         
                

                                    
*/
WDI_Status
WDI_RemBeaconFilterReq
(
   WDI_RemBeaconFilterReqParamsType *pwdiBeaconFilterReqParams,
   WDI_RemBeaconFilterCb             wdiBeaconFilterCb,
   void*                             pUserData
)
{
   WDI_EventInfoType      wdiEventData;
   /*                                                                        */

   /*                                                                        
                 
                                                                           */
   if ( eWLAN_PAL_FALSE == gWDIInitialized )
   {
     WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_ERROR,
               "WDI API call before module is initialized - Fail request");

     return WDI_STATUS_E_NOT_ALLOWED;
   }

   /*                                                                        
                                                
                                                                           */
   wdiEventData.wdiRequest      = WDI_REM_BEACON_FILTER_REQ;
   wdiEventData.pEventData      = pwdiBeaconFilterReqParams;
   wdiEventData.uEventDataSize  = sizeof(*pwdiBeaconFilterReqParams);;
   wdiEventData.pCBfnc          = wdiBeaconFilterCb;
   wdiEventData.pUserData       = pUserData;

   return WDI_PostMainEvent(&gWDICb, WDI_REQUEST_EVENT, &wdiEventData);
}/*                      */

/* 
                                                              
                                                    
                                                            
                                                               
                                                    
                                              

                                                                
                                      

                                        

                                                           
                                          

                                                            
                                                                
                  

                                                         
                

                      
                                    
*/
WDI_Status
WDI_SetRSSIThresholdsReq
(
  WDI_SetRSSIThresholdsReqParamsType*      pwdiRSSIThresholdsParams,
  WDI_SetRSSIThresholdsCb                  wdiSetRSSIThresholdsCb,
  void*                                    pUserData
)
{
   WDI_EventInfoType      wdiEventData;
   /*                                                                        */

   /*                                                                        
                 
                                                                           */
   if ( eWLAN_PAL_FALSE == gWDIInitialized )
   {
     WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_ERROR,
               "WDI API call before module is initialized - Fail request");

     return WDI_STATUS_E_NOT_ALLOWED;
   }

   /*                                                                        
                                                
                                                                           */
   wdiEventData.wdiRequest      = WDI_SET_RSSI_THRESHOLDS_REQ;
   wdiEventData.pEventData      = pwdiRSSIThresholdsParams;
   wdiEventData.uEventDataSize  = sizeof(*pwdiRSSIThresholdsParams);;
   wdiEventData.pCBfnc          = wdiSetRSSIThresholdsCb;
   wdiEventData.pUserData       = pUserData;

   return WDI_PostMainEvent(&gWDICb, WDI_REQUEST_EVENT, &wdiEventData);
}/*                         */

/* 
                                                            
                                                            
                                                            
                                                            
                                                      
                                                         

                                                                
                                      

                                        

                                                            
                                             

                                                                
                                                       
              

                                                         
                

                      
                                    
*/
WDI_Status
WDI_HostOffloadReq
(
  WDI_HostOffloadReqParamsType*      pwdiHostOffloadParams,
  WDI_HostOffloadCb                  wdiHostOffloadCb,
  void*                              pUserData
)
{
   WDI_EventInfoType      wdiEventData;
   /*                                                                        */

   /*                                                                        
                 
                                                                           */
   if ( eWLAN_PAL_FALSE == gWDIInitialized )
   {
     WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_ERROR,
               "WDI API call before module is initialized - Fail request");

     return WDI_STATUS_E_NOT_ALLOWED;
   }

   /*                                                                        
                                                
                                                                           */
   wdiEventData.wdiRequest      = WDI_HOST_OFFLOAD_REQ;
   wdiEventData.pEventData      = pwdiHostOffloadParams;
   wdiEventData.uEventDataSize  = sizeof(*pwdiHostOffloadParams);;
   wdiEventData.pCBfnc          = wdiHostOffloadCb;
   wdiEventData.pUserData       = pUserData;

   return WDI_PostMainEvent(&gWDICb, WDI_REQUEST_EVENT, &wdiEventData);
}/*                  */

/* 
                                                          
                                                                         
                                                                         
                                                            
                                                    
                                                         

                                                                
                                      

                                        

                                                        
                                             

                                                              
                                                     
              

                                                         
                

                      
                                    
*/
WDI_Status
WDI_KeepAliveReq
(
  WDI_KeepAliveReqParamsType*        pwdiKeepAliveParams,
  WDI_KeepAliveCb                    wdiKeepAliveCb,
  void*                              pUserData
)
{
    WDI_EventInfoType      wdiEventData;
    /*                                                                        */

    /*                                                                        
                 
                                                                            */
    if ( eWLAN_PAL_FALSE == gWDIInitialized )
    {
         WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_ERROR,
                    "WDI_KeepAliveReq: WDI API call before module "
                    "is initialized - Fail request");

     return WDI_STATUS_E_NOT_ALLOWED;
    }

    /*                                                                        
                                                
                                                                            */
    wdiEventData.wdiRequest      = WDI_KEEP_ALIVE_REQ;
    wdiEventData.pEventData      = pwdiKeepAliveParams;
    wdiEventData.uEventDataSize  = sizeof(*pwdiKeepAliveParams);
    wdiEventData.pCBfnc          = wdiKeepAliveCb;
    wdiEventData.pUserData       = pUserData;

    return WDI_PostMainEvent(&gWDICb, WDI_REQUEST_EVENT, &wdiEventData);
}/*                */

/* 
                                                              
                                                                
                                                           
                                                              
                                                             
                                                         

                                                                
                                      

                                        

                                                       
                                                       

                                                         
                                                              
                       

                                                         
                

                      
                                    
*/
WDI_Status
WDI_WowlAddBcPtrnReq
(
  WDI_WowlAddBcPtrnReqParamsType*    pwdiWowlAddBcPtrnParams,
  WDI_WowlAddBcPtrnCb                wdiWowlAddBcPtrnCb,
  void*                              pUserData
)
{
   WDI_EventInfoType      wdiEventData;
   /*                                                                        */

   /*                                                                        
                 
                                                                           */
   if ( eWLAN_PAL_FALSE == gWDIInitialized )
   {
     WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_ERROR,
               "WDI API call before module is initialized - Fail request");

     return WDI_STATUS_E_NOT_ALLOWED;
   }

   /*                                                                        
                                                
                                                                           */
   wdiEventData.wdiRequest      = WDI_WOWL_ADD_BC_PTRN_REQ;
   wdiEventData.pEventData      = pwdiWowlAddBcPtrnParams;
   wdiEventData.uEventDataSize  = sizeof(*pwdiWowlAddBcPtrnParams);;
   wdiEventData.pCBfnc          = wdiWowlAddBcPtrnCb;
   wdiEventData.pUserData       = pUserData;

   return WDI_PostMainEvent(&gWDICb, WDI_REQUEST_EVENT, &wdiEventData);
}/*                    */

/* 
                                                              
                                                            
                                                             
                                                         
                                              

                                                                
                                      

                                            

                                                       
                                                       

                                                         
                                                              
                       

                                                         
                

                          
                                    
*/
WDI_Status
WDI_WowlDelBcPtrnReq
(
  WDI_WowlDelBcPtrnReqParamsType*    pwdiWowlDelBcPtrnParams,
  WDI_WowlDelBcPtrnCb                wdiWowlDelBcPtrnCb,
  void*                              pUserData
)
{
   WDI_EventInfoType      wdiEventData;
   /*                                                                        */

   /*                                                                        
                 
                                                                           */
   if ( eWLAN_PAL_FALSE == gWDIInitialized )
   {
     WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_ERROR,
               "WDI API call before module is initialized - Fail request");

     return WDI_STATUS_E_NOT_ALLOWED;
   }

   /*                                                                        
                                                
                                                                           */
   wdiEventData.wdiRequest      = WDI_WOWL_DEL_BC_PTRN_REQ;
   wdiEventData.pEventData      = pwdiWowlDelBcPtrnParams;
   wdiEventData.uEventDataSize  = sizeof(*pwdiWowlDelBcPtrnParams);;
   wdiEventData.pCBfnc          = wdiWowlDelBcPtrnCb;
   wdiEventData.pUserData       = pUserData;

   return WDI_PostMainEvent(&gWDICb, WDI_REQUEST_EVENT, &wdiEventData);
}/*                    */

/* 
                                                          
                                                             
                                                              
                                                            
                                                              
                         

                                                                
                                      

                                        

                                                      
                                                       

                                                        
                                                              
              

                                                         
                

                      
                                    
*/
WDI_Status
WDI_WowlEnterReq
(
  WDI_WowlEnterReqParamsType*    pwdiWowlEnterParams,
  WDI_WowlEnterReqCb             wdiWowlEnterCb,
  void*                          pUserData
)
{
   WDI_EventInfoType      wdiEventData;
   /*                                                                        */

   /*                                                                        
                 
                                                                           */
   if ( eWLAN_PAL_FALSE == gWDIInitialized )
   {
     WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_ERROR,
               "WDI API call before module is initialized - Fail request");

     return WDI_STATUS_E_NOT_ALLOWED;
   }

   /*                                                                        
                                                
                                                                           */
   wdiEventData.wdiRequest      = WDI_WOWL_ENTER_REQ;
   wdiEventData.pEventData      = pwdiWowlEnterParams;
   wdiEventData.uEventDataSize  = sizeof(*pwdiWowlEnterParams);;
   wdiEventData.pCBfnc          = wdiWowlEnterCb;
   wdiEventData.pUserData       = pUserData;

   return WDI_PostMainEvent(&gWDICb, WDI_REQUEST_EVENT, &wdiEventData);
}/*                */

/* 
                                                         
                                                               
                                                               
                                                               
                

                                                                
                                      

                                        

                                                    
                                                       

                                                                
                                                           

                                                         
                

                      
                                    
*/
WDI_Status
WDI_WowlExitReq
(
  WDI_WowlExitReqCb              wdiWowlExitCb,
  void*                          pUserData
)
{
   WDI_EventInfoType      wdiEventData;
   /*                                                                        */

   /*                                                                        
                 
                                                                           */
   if ( eWLAN_PAL_FALSE == gWDIInitialized )
   {
     WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_ERROR,
               "WDI API call before module is initialized - Fail request");

     return WDI_STATUS_E_NOT_ALLOWED;
   }

   /*                                                                        
                                                
                                                                           */
   wdiEventData.wdiRequest      = WDI_WOWL_EXIT_REQ;
   wdiEventData.pEventData      = NULL;
   wdiEventData.uEventDataSize  = 0;
   wdiEventData.pCBfnc          = wdiWowlExitCb;
   wdiEventData.pUserData       = pUserData;

   return WDI_PostMainEvent(&gWDICb, WDI_REQUEST_EVENT, &wdiEventData);
}/*               */

/* 
                                                              
                                                             
                                                              
                                                         
                                                              
                              

                                                                
                                      


                                                     
                                                               
                                      

                                                              
                                                                
                                          

                                                         
                

                                    
*/
WDI_Status
WDI_ConfigureAppsCpuWakeupStateReq
(
   WDI_ConfigureAppsCpuWakeupStateReqParamsType *pwdiConfigureAppsCpuWakeupStateReqParams,
   WDI_ConfigureAppsCpuWakeupStateCb             wdiConfigureAppsCpuWakeupStateCb,
   void*                                         pUserData
)
{
   WDI_EventInfoType      wdiEventData;
   /*                                                                        */

   /*                                                                        
                 
                                                                           */
   if ( eWLAN_PAL_FALSE == gWDIInitialized )
   {
     WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_ERROR,
               "WDI API call before module is initialized - Fail request");

     return WDI_STATUS_E_NOT_ALLOWED;
   }

   /*                                                                        
                                                
                                                                           */
   wdiEventData.wdiRequest      = WDI_CONFIGURE_APPS_CPU_WAKEUP_STATE_REQ;
   wdiEventData.pEventData      = pwdiConfigureAppsCpuWakeupStateReqParams;
   wdiEventData.uEventDataSize  = sizeof(*pwdiConfigureAppsCpuWakeupStateReqParams);
   wdiEventData.pCBfnc          = wdiConfigureAppsCpuWakeupStateCb;
   wdiEventData.pUserData       = pUserData;

   return WDI_PostMainEvent(&gWDICb, WDI_REQUEST_EVENT, &wdiEventData);
}/*                                  */
/* 
                                                              
                                                               
                                                              
                                                                
                                

                                                                
                                      

                                    

                                                        
                                                       

                                                               
                                                          

                                                         
                

                  
                                    
*/
WDI_Status
WDI_FlushAcReq
(
  WDI_FlushAcReqParamsType* pwdiFlushAcReqParams,
  WDI_FlushAcRspCb          wdiFlushAcRspCb,
  void*                     pUserData
)
{
   WDI_EventInfoType      wdiEventData;
   /*                                                                        */

   /*                                                                        
                 
                                                                           */
   if ( eWLAN_PAL_FALSE == gWDIInitialized )
   {
     WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_ERROR,
               "WDI API call before module is initialized - Fail request");

     return WDI_STATUS_E_NOT_ALLOWED;
   }

   /*                                                                        
                                                
                                                                           */
   wdiEventData.wdiRequest      = WDI_FLUSH_AC_REQ;
   wdiEventData.pEventData      = pwdiFlushAcReqParams;
   wdiEventData.uEventDataSize  = sizeof(*pwdiFlushAcReqParams);
   wdiEventData.pCBfnc          = wdiFlushAcRspCb;
   wdiEventData.pUserData       = pUserData;

   return WDI_PostMainEvent(&gWDICb, WDI_REQUEST_EVENT, &wdiEventData);

}/*              */

/* 
                                                           
                                                                
                                                               
                                                              
                                                          
                                              

                                                                
                                      


                                                              
                                                       

                                                         
                                                                
              

                                                         
                

                                    
*/
WDI_Status
WDI_BtAmpEventReq
(
  WDI_BtAmpEventParamsType* pwdiBtAmpEventReqParams,
  WDI_BtAmpEventRspCb       wdiBtAmpEventRspCb,
  void*                     pUserData
)
{
   WDI_EventInfoType      wdiEventData;
   /*                                                                        */

   /*                                                                        
                 
                                                                           */
   if ( eWLAN_PAL_FALSE == gWDIInitialized )
   {
     WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_ERROR,
               "WDI API call before module is initialized - Fail request");

     return WDI_STATUS_E_NOT_ALLOWED;
   }

   /*                                                                        
                                                
                                                                           */
   wdiEventData.wdiRequest      = WDI_BTAMP_EVENT_REQ;
   wdiEventData.pEventData      = pwdiBtAmpEventReqParams;
   wdiEventData.uEventDataSize  = sizeof(*pwdiBtAmpEventReqParams);
   wdiEventData.pCBfnc          = wdiBtAmpEventRspCb;
   wdiEventData.pUserData       = pUserData;

   return WDI_PostMainEvent(&gWDICb, WDI_REQUEST_EVENT, &wdiEventData);

}/*                 */

#ifdef FEATURE_OEM_DATA_SUPPORT
/* 
                                                                
                                                                  
                                                              
                                                                
                                              

                                                                
                                      



                                                 
                                         

                                                           
                                                      
              

                                                         
                

                                    
*/
WDI_Status
WDI_StartOemDataReq
(
  WDI_oemDataReqParamsType*         pwdiOemDataReqParams,
  WDI_oemDataRspCb                  wdiOemDataRspCb,
  void*                             pUserData
)
{
   WDI_EventInfoType      wdiEventData;
   /*                                                                        */

   /*                                                                        
                 
                                                                           */
   if ( eWLAN_PAL_FALSE == gWDIInitialized )
   {
     WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_ERROR,
               "WDI API call before module is initialized - Fail request");

     return WDI_STATUS_E_NOT_ALLOWED;
   }

   /*                                                                        
                                                
                                                                           */
   wdiEventData.wdiRequest      = WDI_START_OEM_DATA_REQ;
   wdiEventData.pEventData      = pwdiOemDataReqParams;
   wdiEventData.uEventDataSize  = sizeof(*pwdiOemDataReqParams);
   wdiEventData.pCBfnc          = wdiOemDataRspCb;
   wdiEventData.pUserData       = pUserData;

   return WDI_PostMainEvent(&gWDICb, WDI_REQUEST_EVENT, &wdiEventData);


}

#endif


/*                                                                        

                                         

                                                                          */
/* 
                                                               
                                                               
                                                            
                                                          
                                              

                                                                
                                      

                                 

                                                         
                                                       

                                                                
                                                           

                                                         
                

               
                                    
*/
WDI_Status
WDI_SwitchChReq
(
  WDI_SwitchChReqParamsType* pwdiSwitchChReqParams,
  WDI_SwitchChRspCb          wdiSwitchChRspCb,
  void*                      pUserData
)
{
  WDI_EventInfoType      wdiEventData;
  /*                                                                        */

  /*                                                                        
                
                                                                          */
  if ( eWLAN_PAL_FALSE == gWDIInitialized )
  {
    WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_ERROR,
              "WDI API call before module is initialized - Fail request");

    return WDI_STATUS_E_NOT_ALLOWED;
  }

  /*                                                                        
                                               
                                                                          */
  wdiEventData.wdiRequest      = WDI_CH_SWITCH_REQ;
  wdiEventData.pEventData      = pwdiSwitchChReqParams;
  wdiEventData.uEventDataSize  = sizeof(*pwdiSwitchChReqParams);
  wdiEventData.pCBfnc          = wdiSwitchChRspCb;
  wdiEventData.pUserData       = pUserData;

  return WDI_PostMainEvent(&gWDICb, WDI_REQUEST_EVENT, &wdiEventData);

}/*               */


/* 
                                                          
                                                             
                                                            
                                                               
                                

                                                                
                                      

                                 

                                                           
                                                       

                                                        
                                                              
              

                                                         
                

               
                                    
*/
WDI_Status
WDI_ConfigSTAReq
(
  WDI_ConfigSTAReqParamsType* pwdiConfigSTAReqParams,
  WDI_ConfigSTARspCb          wdiConfigSTARspCb,
  void*                       pUserData
)
{
  WDI_EventInfoType      wdiEventData;
  /*                                                                        */

  /*                                                                        
                
                                                                          */
  if ( eWLAN_PAL_FALSE == gWDIInitialized )
  {
    WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_ERROR,
              "WDI API call before module is initialized - Fail request");

    return WDI_STATUS_E_NOT_ALLOWED;
  }

  /*                                                                        
                                               
                                                                          */
  wdiEventData.wdiRequest      = WDI_CONFIG_STA_REQ;
  wdiEventData.pEventData      = pwdiConfigSTAReqParams;
  wdiEventData.uEventDataSize  = sizeof(*pwdiConfigSTAReqParams);
  wdiEventData.pCBfnc          = wdiConfigSTARspCb;
  wdiEventData.pUserData       = pUserData;

  return WDI_PostMainEvent(&gWDICb, WDI_REQUEST_EVENT, &wdiEventData);

}/*                */

/* 
                                                             
                                                              
                                                              
                                                       
                                              

                                                                
                                      

                                        

                                                               
                                                          

                                                           
                                                              
                  

                                                         
                

                      
                                    
*/
WDI_Status
WDI_SetLinkStateReq
(
  WDI_SetLinkReqParamsType* pwdiSetLinkStateReqParams,
  WDI_SetLinkStateRspCb     wdiSetLinkStateRspCb,
  void*                     pUserData
)
{
  WDI_EventInfoType      wdiEventData;
  /*                                                                        */

  /*                                                                        
                
                                                                          */
  if ( eWLAN_PAL_FALSE == gWDIInitialized )
  {
    WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_ERROR,
              "WDI API call before module is initialized - Fail request");

    return WDI_STATUS_E_NOT_ALLOWED;
  }

  /*                                                                        
                                               
                                                                          */
  wdiEventData.wdiRequest      = WDI_SET_LINK_ST_REQ;
  wdiEventData.pEventData      = pwdiSetLinkStateReqParams;
  wdiEventData.uEventDataSize  = sizeof(*pwdiSetLinkStateReqParams);
  wdiEventData.pCBfnc          = wdiSetLinkStateRspCb;
  wdiEventData.pUserData       = pUserData;

  return WDI_PostMainEvent(&gWDICb, WDI_REQUEST_EVENT, &wdiEventData);

}/*                   */


/* 
                                                               
                                                              
                                                              
                                                              
                                   

                                                                
                                      

                                 

                                                            
                                                       

                                                                
                                                           

                                                         
                

               
                                    
*/
WDI_Status
WDI_GetStatsReq
(
  WDI_GetStatsReqParamsType* pwdiGetStatsReqParams,
  WDI_GetStatsRspCb          wdiGetStatsRspCb,
  void*                      pUserData
)
{
  WDI_EventInfoType      wdiEventData;
  /*                                                                        */

  /*                                                                        
                
                                                                          */
  if ( eWLAN_PAL_FALSE == gWDIInitialized )
  {
    WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_ERROR,
              "WDI API call before module is initialized - Fail request");

    return WDI_STATUS_E_NOT_ALLOWED;
  }

  /*                                                                        
                                               
                                                                          */
  wdiEventData.wdiRequest      = WDI_GET_STATS_REQ;
  wdiEventData.pEventData      = pwdiGetStatsReqParams;
  wdiEventData.uEventDataSize  = sizeof(*pwdiGetStatsReqParams);
  wdiEventData.pCBfnc          = wdiGetStatsRspCb;
  wdiEventData.pUserData       = pUserData;

  return WDI_PostMainEvent(&gWDICb, WDI_REQUEST_EVENT, &wdiEventData);

}/*               */


/* 
                                                               
                                                         
                                                                
                                                              
                                                   

                                                                
                                      

                                 

                                                           
                                                       

                                                         
                                                              
              

                                                         
                

               
                                    
*/
WDI_Status
WDI_UpdateCfgReq
(
  WDI_UpdateCfgReqParamsType* pwdiUpdateCfgReqParams,
  WDI_UpdateCfgRspCb          wdiUpdateCfgsRspCb,
  void*                       pUserData
)
{
  WDI_EventInfoType      wdiEventData;
  /*                                                                        */

  /*                                                                        
                
                                                                          */
  if ( eWLAN_PAL_FALSE == gWDIInitialized )
  {
    WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_ERROR,
              "WDI API call before module is initialized - Fail request");

    return WDI_STATUS_E_NOT_ALLOWED;
  }

  /*                                                                        
                                               
                                                                          */
  wdiEventData.wdiRequest      = WDI_UPDATE_CFG_REQ;
  wdiEventData.pEventData      = pwdiUpdateCfgReqParams;
  wdiEventData.uEventDataSize  = sizeof(*pwdiUpdateCfgReqParams);
  wdiEventData.pCBfnc          = wdiUpdateCfgsRspCb;
  wdiEventData.pUserData       = pUserData;

  return WDI_PostMainEvent(&gWDICb, WDI_REQUEST_EVENT, &wdiEventData);

}/*                */



/* 
                                                                
                                                                
                                                                
                                                             
                                                              
                         

                                                                
                                      

                                        

                                                                
                                          

                                                                
                                                     

                                                         
                

                      
                                    
*/
WDI_Status
WDI_AddBAReq
(
  WDI_AddBAReqParamsType* pwdiAddBAReqParams,
  WDI_AddBARspCb          wdiAddBARspCb,
  void*                   pUserData
)
{
  WDI_EventInfoType      wdiEventData;
  /*                                                                        */

  /*                                                                        
                
                                                                          */
  if ( eWLAN_PAL_FALSE == gWDIInitialized )
  {
    WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_ERROR,
              "WDI API call before module is initialized - Fail request");

    return WDI_STATUS_E_NOT_ALLOWED;
  }

  /*                                                                        
                                               
                                                                          */
  wdiEventData.wdiRequest      = WDI_ADD_BA_REQ;
  wdiEventData.pEventData      = pwdiAddBAReqParams;
  wdiEventData.uEventDataSize  = sizeof(*pwdiAddBAReqParams);
  wdiEventData.pCBfnc          = wdiAddBARspCb;
  wdiEventData.pUserData       = pUserData;

  return WDI_PostMainEvent(&gWDICb, WDI_REQUEST_EVENT, &wdiEventData);

}/*            */


/* 
                                                                    
                                                                
                                                                
                                                             
                                                              
                         

                                                                
                                      

                                        

                                                                
                                          

                                                                
                                                     

                                                         
                

                      
                                    
*/
WDI_Status
WDI_TriggerBAReq
(
  WDI_TriggerBAReqParamsType* pwdiTriggerBAReqParams,
  WDI_TriggerBARspCb          wdiTriggerBARspCb,
  void*                       pUserData
)
{
  WDI_EventInfoType      wdiEventData;
  /*                                                                        */

  /*                                                                        
                
                                                                          */
  if ( eWLAN_PAL_FALSE == gWDIInitialized )
  {
    WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_ERROR,
              "WDI API call before module is initialized - Fail request");

    return WDI_STATUS_E_NOT_ALLOWED;
  }

  /*                                                                        
                                               
                                                                          */
  wdiEventData.wdiRequest      = WDI_TRIGGER_BA_REQ;
  wdiEventData.pEventData      = pwdiTriggerBAReqParams;
  wdiEventData.uEventDataSize  = sizeof(*pwdiTriggerBAReqParams);
  wdiEventData.pCBfnc          = wdiTriggerBARspCb;
  wdiEventData.pUserData       = pUserData;

  return WDI_PostMainEvent(&gWDICb, WDI_REQUEST_EVENT, &wdiEventData);

}/*            */

/* 
                                                                   
                                                                 
                                                                                                    
                                                               
                

                                                                
                                      

                                        

                                                                 
                                             

                                                                 
                                                                

                                                         
                

                      
                                    
*/
WDI_Status
WDI_UpdateBeaconParamsReq
(
  WDI_UpdateBeaconParamsType*    pwdiUpdateBeaconParams,
  WDI_UpdateBeaconParamsRspCb    wdiUpdateBeaconParamsRspCb,
  void*                        pUserData
)
{
  WDI_EventInfoType      wdiEventData;
  /*                                                                        */

  /*                                                                        
                
                                                                          */
  if ( eWLAN_PAL_FALSE == gWDIInitialized )
  {
    WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_ERROR,
              "WDI API call before module is initialized - Fail request");

    return WDI_STATUS_E_NOT_ALLOWED;
  }

  /*                                                                        
                                               
                                                                          */
  wdiEventData.wdiRequest      = WDI_UPD_BCON_PRMS_REQ;
  wdiEventData.pEventData      = pwdiUpdateBeaconParams;
  wdiEventData.uEventDataSize  = sizeof(*pwdiUpdateBeaconParams);
  wdiEventData.pCBfnc          = wdiUpdateBeaconParamsRspCb;
  wdiEventData.pUserData       = pUserData;

  return WDI_PostMainEvent(&gWDICb, WDI_REQUEST_EVENT, &wdiEventData);

}/*                         */

/* 
                                                                 
                                                         
                                                                                                      
                                                               
                

                                                                
                                      

                                        

                                                               
                                             

                                                               
                                                                

                                                         
                

                      
                                    
*/
WDI_Status
WDI_SendBeaconParamsReq
(
  WDI_SendBeaconParamsType*    pwdiSendBeaconParams,
  WDI_SendBeaconParamsRspCb    wdiSendBeaconParamsRspCb,
  void*                        pUserData
)
{
  WDI_EventInfoType      wdiEventData;
  /*                                                                        */

  /*                                                                        
                
                                                                          */
  if ( eWLAN_PAL_FALSE == gWDIInitialized )
  {
    WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_ERROR,
              "WDI API call before module is initialized - Fail request");

    return WDI_STATUS_E_NOT_ALLOWED;
  }

  /*                                                                        
                                               
                                                                          */
  wdiEventData.wdiRequest      = WDI_SND_BCON_REQ;
  wdiEventData.pEventData      = pwdiSendBeaconParams;
  wdiEventData.uEventDataSize  = sizeof(*pwdiSendBeaconParams);
  wdiEventData.pCBfnc          = wdiSendBeaconParamsRspCb;
  wdiEventData.pUserData       = pUserData;

  return WDI_PostMainEvent(&gWDICb, WDI_REQUEST_EVENT, &wdiEventData);

}/*                       */

/* 
                                                             
                                                                
                                 
                                                    
                                                            
                                                         

                                                                
                                      


                                                                 
                                                       

                                                               
                                                             
                       

                                                         
                

                  
                                    
*/

WDI_Status
WDI_UpdateProbeRspTemplateReq
(
  WDI_UpdateProbeRspTemplateParamsType*    pwdiUpdateProbeRspParams,
  WDI_UpdateProbeRspTemplateRspCb          wdiUpdateProbeRspParamsRspCb,
  void*                                    pUserData
)
{
  WDI_EventInfoType      wdiEventData;
  /*                                                                        */

  /*                                                                        
                
                                                                          */
  if ( eWLAN_PAL_FALSE == gWDIInitialized )
  {
    WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_ERROR,
              "WDI API call before module is initialized - Fail request");

    return WDI_STATUS_E_NOT_ALLOWED;
  }

  /*                                                                        
                                               
                                                                          */
  wdiEventData.wdiRequest      = WDI_UPD_PROBE_RSP_TEMPLATE_REQ;
  wdiEventData.pEventData      = pwdiUpdateProbeRspParams;
  wdiEventData.uEventDataSize  = sizeof(*pwdiUpdateProbeRspParams);
  wdiEventData.pCBfnc          = wdiUpdateProbeRspParamsRspCb;
  wdiEventData.pUserData       = pUserData;

  return WDI_PostMainEvent(&gWDICb, WDI_REQUEST_EVENT, &wdiEventData);

}/*                             */

/* 
                                                                            
                         


                                                                          
                                          

                                                                     
                                                          

                                                         
                

                      
                                    
*/
WDI_Status
WDI_NvDownloadReq
(
  WDI_NvDownloadReqParamsType* pwdiNvDownloadReqParams,
  WDI_NvDownloadRspCb        wdiNvDownloadRspCb,
  void*                   pUserData
)
{
  WDI_EventInfoType      wdiEventData;

  /*                                                                        
                
                                                                          */
  if ( eWLAN_PAL_FALSE == gWDIInitialized )
  {
    WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_ERROR,
              "WDI API call before module is initialized - Fail request");

    return WDI_STATUS_E_NOT_ALLOWED;
  }

  /*                                                                        
                                               
                                                                          */
  wdiEventData.wdiRequest      = WDI_NV_DOWNLOAD_REQ;
  wdiEventData.pEventData      = (void *)pwdiNvDownloadReqParams;
  wdiEventData.uEventDataSize  = sizeof(*pwdiNvDownloadReqParams);
  wdiEventData.pCBfnc          = wdiNvDownloadRspCb;
  wdiEventData.pUserData       = pUserData;

  return WDI_PostMainEvent(&gWDICb, WDI_START_EVENT, &wdiEventData);

}/*                 */

#ifdef WLAN_FEATURE_P2P
/* 
                                                  
                                                 
                                                    
                                                            
                                                         

                                                                
                                      


                                                                 
                                                       

                                                               
                                                             
                       

                                                         
                

                  
                                    
*/
WDI_Status
WDI_SetP2PGONOAReq
(
  WDI_SetP2PGONOAReqParamsType*    pwdiP2PGONOAReqParams,
  WDI_SetP2PGONOAReqParamsRspCb    wdiP2PGONOAReqParamsRspCb,
  void*                            pUserData
)
{
  WDI_EventInfoType      wdiEventData;
  /*                                                                        */

  /*                                                                        
                
                                                                          */
  if ( eWLAN_PAL_FALSE == gWDIInitialized )
  {
    WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_ERROR,
              "WDI API call before module is initialized - Fail request");

    return WDI_STATUS_E_NOT_ALLOWED;
  }

  /*                                                                        
                                               
                                                                          */
  wdiEventData.wdiRequest      = WDI_P2P_GO_NOTICE_OF_ABSENCE_REQ;
  wdiEventData.pEventData      = pwdiP2PGONOAReqParams;
  wdiEventData.uEventDataSize  = sizeof(*pwdiP2PGONOAReqParams);
  wdiEventData.pCBfnc          = wdiP2PGONOAReqParamsRspCb;
  wdiEventData.pUserData       = pUserData;

  return WDI_PostMainEvent(&gWDICb, WDI_REQUEST_EVENT, &wdiEventData);

}/*                  */
#endif

/* 
                                                 
                                                                 
                                                                
                                      


                                                            
                                                       

                                                         
                

     
                                    
*/
WDI_Status
WDI_AddSTASelfReq
(
  WDI_AddSTASelfReqParamsType* pwdiAddSTASelfReqParams,
  WDI_AddSTASelfParamsRspCb    wdiAddSTASelfReqParamsRspCb,
  void*                        pUserData
)
{
  WDI_EventInfoType      wdiEventData;
  /*                                                                        */

  /*                                                                        
                
                                                                          */
  if ( eWLAN_PAL_FALSE == gWDIInitialized )
  {
    WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_ERROR,
              "WDI API call before module is initialized - Fail request");

    return WDI_STATUS_E_NOT_ALLOWED;
  }

  /*                                                                        
                                               
                                                                          */
  wdiEventData.wdiRequest      = WDI_ADD_STA_SELF_REQ;
  wdiEventData.pEventData      = pwdiAddSTASelfReqParams;
  wdiEventData.uEventDataSize  = sizeof(*pwdiAddSTASelfReqParams);
  wdiEventData.pCBfnc          = wdiAddSTASelfReqParamsRspCb;
  wdiEventData.pUserData       = pUserData;

  return WDI_PostMainEvent(&gWDICb, WDI_REQUEST_EVENT, &wdiEventData);

}/*                 */


#ifdef WLAN_FEATURE_VOWIFI_11R
/* 
                                                                    
                                                            
                                                              
                                                            
                                                              
                                                              
                                

                                                                
                                      

                                        

                                                                
                                          

                                                                
                                                     

                                                         
                

                      
                                    
*/
WDI_Status
WDI_AggrAddTSReq
(
  WDI_AggrAddTSReqParamsType* pwdiAggrAddTsReqParams,
  WDI_AggrAddTsRspCb          wdiAggrAddTsRspCb,
  void*                   pUserData
)
{
  WDI_EventInfoType      wdiEventData;
  /*                                                                        */

  /*                                                                        
                
                                                                          */
  if ( eWLAN_PAL_FALSE == gWDIInitialized )
  {
    WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_ERROR,
              "WDI API call before module is initialized - Fail request");

    return WDI_STATUS_E_NOT_ALLOWED;
  }

  /*                                                                        
                                               
                                                                          */
  wdiEventData.wdiRequest      = WDI_AGGR_ADD_TS_REQ;
  wdiEventData.pEventData      = pwdiAggrAddTsReqParams;
  wdiEventData.uEventDataSize  = sizeof(*pwdiAggrAddTsReqParams);
  wdiEventData.pCBfnc          = wdiAggrAddTsRspCb;
  wdiEventData.pUserData       = pUserData;

  return WDI_PostMainEvent(&gWDICb, WDI_REQUEST_EVENT, &wdiEventData);

}/*                */

#endif /*                         */

#ifdef ANI_MANF_DIAG
/* 
                         
                              

                                          
                                                  
                                     

     
                                    
*/
WDI_Status
WDI_FTMCommandReq
(
  WDI_FTMCommandReqType *ftmCommandReq,
  WDI_FTMCommandRspCb    ftmCommandRspCb,
  void                  *pUserData
)
{
  WDI_EventInfoType      wdiEventData;
  /*                                                                        */

  /*                                                                        
                
                                                                          */
  if ( eWLAN_PAL_FALSE == gWDIInitialized )
  {
    WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_ERROR,
              "WDI API call before module is initialized - Fail request");

    return WDI_STATUS_E_NOT_ALLOWED;
  }

  /*                                                                        
                                               
                                                                          */
  wdiEventData.wdiRequest     = WDI_FTM_CMD_REQ;
  wdiEventData.pEventData     = (void *)ftmCommandReq;
  wdiEventData.uEventDataSize = ftmCommandReq->bodyLength + sizeof(wpt_uint32);
  wdiEventData.pCBfnc         = ftmCommandRspCb;
  wdiEventData.pUserData      = pUserData;

  return WDI_PostMainEvent(&gWDICb, WDI_REQUEST_EVENT, &wdiEventData);
}
#endif /*               */
/* 
                                        

                                                                
                                      


                                             
                                          

                                                                    
                                                

                                                         
                

     
                                    
*/
WDI_Status
WDI_HostResumeReq
(
  WDI_ResumeParamsType*            pwdiResumeReqParams,
  WDI_HostResumeEventRspCb         wdiResumeReqRspCb,
  void*                            pUserData
)
{
  WDI_EventInfoType      wdiEventData;
  /*                                                                        */

  /*                                                                        
                
                                                                          */
  if ( eWLAN_PAL_FALSE == gWDIInitialized )
  {
    WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_ERROR,
              "WDI API call before module is initialized - Fail request");

    return WDI_STATUS_E_NOT_ALLOWED;
  }

  /*                                                                        
                                               
                                                                          */
  wdiEventData.wdiRequest      = WDI_HOST_RESUME_REQ;
  wdiEventData.pEventData      = pwdiResumeReqParams;
  wdiEventData.uEventDataSize  = sizeof(*pwdiResumeReqParams);
  wdiEventData.pCBfnc          = wdiResumeReqRspCb;
  wdiEventData.pUserData       = pUserData;

  return WDI_PostMainEvent(&gWDICb, WDI_REQUEST_EVENT, &wdiEventData);

}/*                 */

/* 
                                        

                                                                
                                      


                                                 
                                          

                                                                     
                                                     

                                                         
                

                      
                                    
*/
WDI_Status
WDI_DelSTASelfReq
(
  WDI_DelSTASelfReqParamsType*      pwdiDelStaSelfReqParams,
  WDI_DelSTASelfRspCb               wdiDelStaSelfRspCb,
  void*                             pUserData
)
{
  WDI_EventInfoType      wdiEventData;
  /*                                                                        */

  /*                                                                        
                
                                                                          */
  if ( eWLAN_PAL_FALSE == gWDIInitialized )
  {
    WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_ERROR,
              "WDI API call before module is initialized - Fail request");

    return WDI_STATUS_E_NOT_ALLOWED;
  }

  /*                                                                        
                                               
                                                                          */
  wdiEventData.wdiRequest      = WDI_DEL_STA_SELF_REQ;
  wdiEventData.pEventData      = pwdiDelStaSelfReqParams;
  wdiEventData.uEventDataSize  = sizeof(*pwdiDelStaSelfReqParams);
  wdiEventData.pCBfnc          = wdiDelStaSelfRspCb;
  wdiEventData.pUserData       = pUserData;

  return WDI_PostMainEvent(&gWDICb, WDI_REQUEST_EVENT, &wdiEventData);

}/*                */

/* 
                                                                 
                                                        
                                                        
                                                                 
                                                         

                                                                
                                      

                                                                                
                                                       

                                                                
                                                                             
                       

                                                         
                

                                    
*/
WDI_Status
WDI_SetTxPerTrackingReq
(
  WDI_SetTxPerTrackingReqParamsType*      pwdiSetTxPerTrackingReqParams,
  WDI_SetTxPerTrackingRspCb               pwdiSetTxPerTrackingRspCb,
  void*                                   pUserData
)
{
   WDI_EventInfoType      wdiEventData;
   /*                                                                        */

   /*                                                                        
                 
                                                                           */
   if ( eWLAN_PAL_FALSE == gWDIInitialized )
   {
     WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_ERROR,
               "WDI API call before module is initialized - Fail request");

     return WDI_STATUS_E_NOT_ALLOWED;
   }

   /*                                                                        
                                                
                                                                           */
   wdiEventData.wdiRequest      = WDI_SET_TX_PER_TRACKING_REQ;
   wdiEventData.pEventData      = pwdiSetTxPerTrackingReqParams;
   wdiEventData.uEventDataSize  = sizeof(*pwdiSetTxPerTrackingReqParams);
   wdiEventData.pCBfnc          = pwdiSetTxPerTrackingRspCb;
   wdiEventData.pUserData       = pUserData;

   return WDI_PostMainEvent(&gWDICb, WDI_REQUEST_EVENT, &wdiEventData);

}/*                       */

/* 
                         
                                                                         
                             

                                                            

                                     

                                                       
                

                                    
*/
WDI_Status
WDI_SetTmLevelReq
(
   WDI_SetTmLevelReqType        *pwdiSetTmLevelReq,
   WDI_SetTmLevelCb              pwdiSetTmLevelRspCb,
   void                         *usrData
)
{
   WDI_EventInfoType      wdiEventData;
   /*                                                                        */

   /*                                                                        
                 
                                                                           */
   if ( eWLAN_PAL_FALSE == gWDIInitialized )
   {
     WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_ERROR,
               "WDI API call before module is initialized - Fail request");

     return WDI_STATUS_E_NOT_ALLOWED;
   }

   /*                                                                        
                                                
                                                                           */
   wdiEventData.wdiRequest      = WDI_SET_TM_LEVEL_REQ;
   wdiEventData.pEventData      = pwdiSetTmLevelReq;
   wdiEventData.uEventDataSize  = sizeof(*pwdiSetTmLevelReq);
   wdiEventData.pCBfnc          = pwdiSetTmLevelRspCb;
   wdiEventData.pUserData       = usrData;

   return WDI_PostMainEvent(&gWDICb, WDI_REQUEST_EVENT, &wdiEventData);
}

/* 
                          

                                                            
                   

                                      

     

                              
*/
WDI_Status
WDI_HostSuspendInd
(
  WDI_SuspendParamsType*    pwdiSuspendIndParams
)
{

  WDI_EventInfoType      wdiEventData;
  /*                                                                        */

  /*                                                                        
                
                                                                          */
  if ( eWLAN_PAL_FALSE == gWDIInitialized )
  {
    WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_ERROR,
              "WDI API call before module is initialized - Fail request");

    return WDI_STATUS_E_NOT_ALLOWED;
  }

  /*                                                                        
                                               
                                                                          */
  wdiEventData.wdiRequest      = WDI_HOST_SUSPEND_IND;
  wdiEventData.pEventData      = pwdiSuspendIndParams;
  wdiEventData.uEventDataSize  = sizeof(*pwdiSuspendIndParams);
  wdiEventData.pCBfnc          = NULL;
  wdiEventData.pUserData       = NULL;

  return WDI_PostMainEvent(&gWDICb, WDI_REQUEST_EVENT, &wdiEventData);

}/*                  */

/* 
                         
                                   

                                                     
                                                       
                                     

     
                                    
*/
WDI_Status WDI_HALDumpCmdReq
(
  WDI_HALDumpCmdReqParamsType *halDumpCmdReqParams,
  WDI_HALDumpCmdRspCb    halDumpCmdRspCb,
  void                  *pUserData
)
{
  WDI_EventInfoType      wdiEventData;
  /*                                                                        */

  /*                                                                        
                
                                                                          */
  if ( eWLAN_PAL_FALSE == gWDIInitialized )
  {
    WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_ERROR,
              "WDI API call before module is initialized - Fail request");

    return WDI_STATUS_E_NOT_ALLOWED;
  }

  /*                                                                        
                                               
                                                                          */
  wdiEventData.wdiRequest     =   WDI_HAL_DUMP_CMD_REQ;
  wdiEventData.pEventData     =   (void *)halDumpCmdReqParams;
  wdiEventData.uEventDataSize =   sizeof(WDI_HALDumpCmdReqParamsType);
  wdiEventData.pCBfnc         =   halDumpCmdRspCb;
  wdiEventData.pUserData      =   pUserData;

  return WDI_PostMainEvent(&gWDICb, WDI_REQUEST_EVENT, &wdiEventData);
}

/*                                                                            

                                                             

                                                                             */

/* 
                                                          


                                                         
                                                          
                                                          
                  

     
                                    
*/
WDI_Status
WDI_PostMainEvent
(
  WDI_ControlBlockType*  pWDICtx,
  WDI_MainEventType      wdiEV,
  WDI_EventInfoType*     pEventData

)
{
  WDI_Status         wdiStatus;
  WDI_MainFuncType   pfnWDIMainEvHdlr;
  WDI_MainStateType  wdiOldState;
  /*                                                                      */

  /*                                                                         
                 
                                                                            */
  if (( pWDICtx->uGlobalState >= WDI_MAX_ST ) ||
      ( wdiEV >= WDI_MAX_EVENT ))
  {
     WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_ERROR,
               "Invalid state or event in Post Main Ev function ST: %d EV: %d",
               pWDICtx->uGlobalState, wdiEV);
     return WDI_STATUS_E_NOT_ALLOWED;
  }

  /*                                          */
  wpalMutexAcquire(&pWDICtx->wptMutex);

  /*                             */
  pfnWDIMainEvHdlr = wdiMainFSM[pWDICtx->uGlobalState].pfnMainTbl[wdiEV];

  wdiOldState = pWDICtx->uGlobalState;

  /*
                                                                       
                                                        
                                                                                      
                                                                                        
                                                                              
       
                         */
  if ( WDI_RESPONSE_EVENT != wdiEV)
  {
    /*                                                                         
                                                                           
                                                     */
    WDI_STATE_TRANSITION( pWDICtx, WDI_BUSY_ST);
  }
  /*                                                                        
                                       */
  if ( NULL != pfnWDIMainEvHdlr )
  {
    WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_WARN,
              "Posting event %d in state: %d to the Main FSM",
              wdiEV, wdiOldState);
    wdiStatus = pfnWDIMainEvHdlr( pWDICtx, pEventData);
  }
  else
  {
    WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_ERROR,
              "Unexpected event %d in state: %d",
              wdiEV, wdiOldState);
    wdiStatus = WDI_STATUS_E_NOT_ALLOWED;
  }

  /*                                                                     
            
                                                                        
                                                                        
                                                          

                                                                              
                                                                          
                         

                                                                         
                                                                          
                                                                   */
  if (( WDI_STATUS_SUCCESS != wdiStatus )&&
      ( WDI_STATUS_PENDING != wdiStatus ))
  {
    if ( WDI_RESPONSE_EVENT != wdiEV)
    {
      /*                                                                     
                                                                              
                                                                           
                                    */
      WDI_STATE_TRANSITION( pWDICtx, wdiOldState);
    }
    WDI_DequeuePendingReq(pWDICtx);

  }

  /*                                        */
  wpalMutexRelease(&pWDICtx->wptMutex);

  return wdiStatus;

}/*                 */


/*                                                                          
                      
                                                                          */
/* 
                                                          


                                                         
                                                                    

     
                                    
*/
WDI_Status
WDI_MainStart
(
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
)
{

  /*                                                                    
                 
                                                                        */
  if (( NULL ==  pWDICtx ) || ( NULL == pEventData ))
  {
     WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_WARN,
               "Invalid parameters on Main Start %x %x",
               pWDICtx, pEventData);
     return WDI_STATUS_E_FAILURE;
  }

  wpalMutexAcquire(&pWDICtx->wptMutex);

  /*                                                                    
                                                   
                                                                        */
  if ( eWLAN_PAL_FALSE == pWDICtx->bCTOpened )
  {
     WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_WARN,
               "Control Transport not yet Open - queueing the request");

     WDI_STATE_TRANSITION( pWDICtx, WDI_INIT_ST);
     WDI_QueuePendingReq( pWDICtx, pEventData);

     wpalMutexRelease(&pWDICtx->wptMutex);
     return WDI_STATUS_PENDING;
  }

  wpalMutexRelease(&pWDICtx->wptMutex);

  /*              */
  return WDI_ProcessRequest( pWDICtx, pEventData );

}/*             */

/* 
                                                 


                                                         
                                                                    

     
                                    
*/
WDI_Status
WDI_MainRspInit
(
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
)
{
  /*                                                                        
                                                                 
                                                                          */
  WDI_ASSERT(0);

  /*              */
  return WDI_STATUS_E_NOT_ALLOWED;
}/*                 */

/* 
                                                          


                                                         
                                                                    

     
                                    
*/
WDI_Status
WDI_MainClose
(
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
)
{

  /*                                                                    
                 
                                                                        */
  if (( NULL ==  pWDICtx ) || ( NULL == pEventData ))
  {
     WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_WARN,
               "Invalid parameters on Main Close %x %x",
               pWDICtx, pEventData);
     return WDI_STATUS_E_FAILURE;
  }

  /*              */
  return WDI_ProcessRequest( pWDICtx, pEventData );

}/*             */
/*                                                                          
                         
                                                                          */
/* 
                                                 


                                                         
                                                                    

     
                                    
*/
WDI_Status
WDI_MainStartStarted
(
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
)
{
  WDI_StartRspCb           wdiStartRspCb = NULL;
  /*                                                                        */

  /*                                                                    
                 
                                                                        */
  if (( NULL ==  pWDICtx ) || ( NULL == pEventData ))
  {
     WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_WARN,
               "Invalid parameters on Main Start %x %x",
               pWDICtx, pEventData);
     return WDI_STATUS_E_FAILURE;
  }

  /*                                                                    
                                                
                                                                        */
  WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_WARN,
    "Received start while transport was already started - nothing to do");

  wpalMutexAcquire(&pWDICtx->wptMutex);

  /*                                                                       
        */
  WDI_STATE_TRANSITION( pWDICtx, WDI_STARTED_ST);

  /*                                      */
  WDI_DequeuePendingReq(pWDICtx);

  wpalMutexRelease(&pWDICtx->wptMutex);

  /*                 */
  wdiStartRspCb = (WDI_StartRspCb)pEventData->pCBfnc;

   /*           */
  wdiStartRspCb( &pWDICtx->wdiCachedStartRspParams, pWDICtx->pRspCBUserData);

  /*              */
  return WDI_STATUS_SUCCESS;

}/*                    */

/* 
                                                


                                                         
                                                                    

     
                                    
*/
WDI_Status
WDI_MainStopStarted
(
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
)
{
  /*                                                                    
                 
                                                                        */
  if (( NULL ==  pWDICtx ) || ( NULL == pEventData ))
  {
     WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_ERROR,
               "Invalid parameters on Main Start %x %x",
               pWDICtx, pEventData);
     return WDI_STATUS_E_FAILURE;
  }

  /*                                                                        
                                                                     */

  WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_INFO,
            "Processing stop request in FSM");

  /*              */
  return WDI_ProcessRequest( pWDICtx, pEventData );

}/*                   */
/* 
                                                   


                                                         
                                                                    

     
                                    
*/
WDI_Status
WDI_MainReqStarted
(
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
)
{

  /*                                                                    
                 
                                                                        */
  if (( NULL ==  pWDICtx ) || ( NULL == pEventData ))
  {
     WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_WARN,
               "Invalid parameters on Main Req Started %x %x",
               pWDICtx, pEventData);
     return WDI_STATUS_E_FAILURE;
  }

  /*                                                                        
                                                                     */

  /*              */
  return WDI_ProcessRequest( pWDICtx, pEventData );

}/*                  */

/* 
                                                             


                                                         
                                                                    

     
                                    
*/
WDI_Status
WDI_MainRsp
(
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
)
{
  WDI_Status  wdiStatus;
  wpt_boolean expectedResponse;

  /*                                                                    
                 
                                                                        */
  if (( NULL ==  pWDICtx ) || ( NULL == pEventData ))
  {
     WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_WARN,
               "Invalid parameters on Main Response %x %x",
               pWDICtx, pEventData);
     return WDI_STATUS_E_FAILURE;
  }

  if ( pEventData->wdiResponse ==  pWDICtx->wdiExpectedResponse )
  {
    /*                                  */
    expectedResponse = eWLAN_PAL_TRUE;

    /*                                                                  */
    pWDICtx->ucExpectedStateTransition = WDI_STARTED_ST;

    /*                                       */
     pWDICtx->wdiExpectedResponse = WDI_MAX_RESP;
  }
  else
  {
    /*                                                  */
    expectedResponse = eWLAN_PAL_FALSE;
    /*                                                              
                                                                       
                               */
    pWDICtx->ucExpectedStateTransition = pWDICtx->uGlobalState;
  }

  /*                                    */
  wdiStatus = WDI_ProcessResponse( pWDICtx, pEventData );

  /*                                                    */
  wpalMutexAcquire(&pWDICtx->wptMutex);

  /*                                                              
                                                                      
                                          
                                                      
                           */
  WDI_STATE_TRANSITION( pWDICtx, pWDICtx->ucExpectedStateTransition);

  /*                                                                       
             */
  if ( expectedResponse )
  {
     WDI_DequeuePendingReq(pWDICtx);
  }

  wpalMutexRelease(&pWDICtx->wptMutex);

  /*                        */
  return WDI_STATUS_SUCCESS;

}/*           */

/*                                                                          
                         
                                                                          */
/* 
                                                


                                                         
                                                                    

     
                                    
*/
WDI_Status
WDI_MainStopStopped
(
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
)
{
  /*                                                                    
                 
                                                                        */
  if (( NULL ==  pWDICtx ) || ( NULL == pEventData ))
  {
     WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_ERROR,
               "Invalid parameters on Main Stop Stopped %x %x",
               pWDICtx, pEventData);
     return WDI_STATUS_E_FAILURE;
  }

  /*                                                                   
                                                                     
                                                                      
                                                                      
                                                              */
  WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_ERROR,
            "Processing stop request while stopped in FSM");

  /*              */
  return WDI_ProcessRequest( pWDICtx, pEventData );

}/*                   */

/*                                                                          
                      
                                                                          */
/* 
                                              


                                                         
                                                                    

     
                                    
*/
WDI_Status
WDI_MainStartBusy
(
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
)
{
  /*                                                                    
                 
                                                                        */
  if (( NULL ==  pWDICtx ) || ( NULL == pEventData ))
  {
     WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_WARN,
               "Invalid parameters on Main Start in BUSY %x %x",
               pWDICtx, pEventData);
     return WDI_STATUS_E_FAILURE;
  }

  /*                                                                    
                                                   
                                                                        */
  WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_WARN,
           "WDI Busy state - queue start request");

  /*                       */
  WDI_QueuePendingReq( pWDICtx, pEventData);

  /*              */
  return WDI_STATUS_PENDING;
}/*                 */

/* 
                                             


                                                         
                                                                    

     
                                    
*/
WDI_Status
WDI_MainStopBusy
(
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
)
{
  /*                                                                    
                 
                                                                        */
  if (( NULL ==  pWDICtx ) || ( NULL == pEventData ))
  {
     WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_WARN,
               "Invalid parameters on Main Stop in BUSY %x %x",
               pWDICtx, pEventData);
     return WDI_STATUS_E_FAILURE;
  }

  /*                                                                    
                                                   
                                                                        */
  WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_WARN,
           "WDI Busy state - queue stop request");

  WDI_QueuePendingReq( pWDICtx, pEventData);
  return WDI_STATUS_PENDING;

}/*                */

/* 
                                                


                                                         
                                                                    

     
                                    
*/
WDI_Status
WDI_MainReqBusy
(
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
)
{
  /*                                                                    
                 
                                                                        */
  if (( NULL ==  pWDICtx ) || ( NULL == pEventData ))
  {
     WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_WARN,
               "Invalid parameters on Main Request in BUSY %x %x",
               pWDICtx, pEventData);
     return WDI_STATUS_E_FAILURE;
  }

  /*                                                                    
                                                   
                                                                        */
  WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_WARN,
           "WDI Busy state - queue request %d because waiting for response %d",
             pEventData->wdiRequest, pWDICtx->wdiExpectedResponse);

  WDI_QueuePendingReq( pWDICtx, pEventData);
  return WDI_STATUS_PENDING;

}/*               */
/* 
                                              


                                                         
                                                                    

     
                                    
*/
WDI_Status
WDI_MainCloseBusy
(
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
)
{
  /*                                                                    
                 
                                                                        */
  if (( NULL ==  pWDICtx ) || ( NULL == pEventData ))
  {
     WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_WARN,
               "Invalid parameters on Main Close in BUSY %x %x",
               pWDICtx, pEventData);
     return WDI_STATUS_E_FAILURE;
  }

  /*                                                                    
                                                   
                                                                        */
  WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_WARN,
           "WDI Busy state - queue close request");

  WDI_QueuePendingReq( pWDICtx, pEventData);
  return WDI_STATUS_PENDING;

}/*                 */

/* 
                                                            


                                                         
                                                                    

     
                                    
*/
WDI_Status
WDI_MainShutdown
(
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
)
{
  /*                                                                    
                 
                                                                        */
  if (( NULL ==  pWDICtx ) || ( NULL == pEventData ))
  {
     WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_ERROR,
               "Invalid parameters on Main Start %x %x",
               pWDICtx, pEventData);
     return WDI_STATUS_E_FAILURE;
  }

  /*                                                                        
                                                                     */

  WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_FATAL,
            "Processing shutdown request in FSM");

  /*              */
  return WDI_ProcessRequest( pWDICtx, pEventData );

}/*                */

/* 
                                                 


                                                         
                                                                    

     
                                    
*/
WDI_Status
WDI_MainShutdownBusy
(
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
)
{
  /*                                                                    
                 
                                                                        */
  if (( NULL ==  pWDICtx ) || ( NULL == pEventData ))
  {
     WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_ERROR,
               "Invalid parameters on Main Start %x %x",
               pWDICtx, pEventData);
     return WDI_STATUS_E_FAILURE;
  }

  /*                                                                 
                                                       
   */
  wpalTimerStop(&gWDICb.wptResponseTimer);

  WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_FATAL,
            "Processing shutdown request in FSM: Busy state ");

  return WDI_ProcessRequest( pWDICtx, pEventData );

}/*                    */


/*                                                                       

                                                          

                                                                        */

/*                                                                        
                                                      
                                                                        */
/* 
                                                            
                  

                                                         
                                                                    

     
                                    
*/
WDI_Status
WDI_ProcessStartReq
(
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
)
{
  WDI_StartReqParamsType* pwdiStartParams    = NULL;
  WDI_StartRspCb          wdiStartRspCb      = NULL;
  wpt_uint8*              pSendBuffer        = NULL;
  wpt_uint16              usDataOffset       = 0;
  wpt_uint16              usSendSize         = 0;

  tHalMacStartReqMsg      halStartReq;
  wpt_uint16              usLen              = 0;
  /*                                                                        */

  /*                                                                         
                
                                                                           */
  if (( NULL == pEventData ) ||
      ( NULL == (pwdiStartParams = (WDI_StartReqParamsType*)pEventData->pEventData)) ||
      ( NULL == (wdiStartRspCb   = (WDI_StartRspCb)pEventData->pCBfnc)))
  {
     WPAL_TRACE( eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_FATAL,
                 "%s: Invalid parameters", __FUNCTION__);
     WDI_ASSERT(0);
     return WDI_STATUS_E_FAILURE;
  }

  /*                                                                       
                      
                                                                         */
  usLen = sizeof(halStartReq.startReqParams) +
          pwdiStartParams->usConfigBufferLen;

  if (( WDI_STATUS_SUCCESS != WDI_GetMessageBuffer( pWDICtx, WDI_START_REQ,
                        usLen,
                        &pSendBuffer, &usDataOffset, &usSendSize))||
      ( usSendSize < (usDataOffset + usLen )))
  {
     WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_FATAL,
              "Unable to get send buffer in start req %x %x %x",
                pEventData, pwdiStartParams, wdiStartRspCb);
     WDI_ASSERT(0);
     return WDI_STATUS_E_FAILURE;
  }

  /*                                                                       
                       
                                                                         */
  halStartReq.startReqParams.driverType =
     WDI_2_HAL_DRV_TYPE(pwdiStartParams->wdiDriverType);

  halStartReq.startReqParams.uConfigBufferLen =
                  pwdiStartParams->usConfigBufferLen;
  wpalMemoryCopy( pSendBuffer+usDataOffset,
                  &halStartReq.startReqParams,
                  sizeof(halStartReq.startReqParams));

  usDataOffset  += sizeof(halStartReq.startReqParams);
  wpalMemoryCopy( pSendBuffer+usDataOffset,
                  pwdiStartParams->pConfigBuffer,
                  pwdiStartParams->usConfigBufferLen);

  pWDICtx->wdiReqStatusCB     = pwdiStartParams->wdiReqStatusCB;
  pWDICtx->pReqStatusUserData = pwdiStartParams->pUserData;

  /*                                                                        
                                                      */
  pWDICtx->wdiLowLevelIndCB   = pwdiStartParams->wdiLowLevelIndCB;
  pWDICtx->pIndUserData       = pwdiStartParams->pIndUserData;

  pWDICtx->bFrameTransEnabled = pwdiStartParams->bFrameTransEnabled;
  /*                                                                         
                             
                                                                           */
  return  WDI_SendMsg( pWDICtx, pSendBuffer, usSendSize,
                       wdiStartRspCb, pEventData->pUserData, WDI_START_RESP);


}/*                   */

/* 
                                                           
                  

                                                         
                                                                    

     
                                    
*/
WDI_Status
WDI_ProcessStopReq
(
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
)
{
  WDI_StopReqParamsType* pwdiStopParams      = NULL;
  WDI_StopRspCb          wdiStopRspCb        = NULL;
  wpt_uint8*             pSendBuffer         = NULL;
  wpt_uint16             usDataOffset        = 0;
  wpt_uint16             usSendSize          = 0;
  wpt_status             status;
  tHalMacStopReqMsg      halStopReq;
  /*                                                                        */

 /*                                                                         
                
                                                                           */
  if (( NULL == pEventData ) ||
      ( NULL == (pwdiStopParams = (WDI_StopReqParamsType*)pEventData->pEventData)) ||
      ( NULL == (wdiStopRspCb   = (WDI_StopRspCb)pEventData->pCBfnc)))
  {
     WPAL_TRACE( eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_WARN,
                 "%s: Invalid parameters", __FUNCTION__);
     WDI_ASSERT(0);
     return WDI_STATUS_E_FAILURE;
  }

  /*                                                                       
                      
                                                                         */
  if (( WDI_STATUS_SUCCESS != WDI_GetMessageBuffer( pWDICtx, WDI_STOP_REQ,
                        sizeof(halStopReq.stopReqParams),
                        &pSendBuffer, &usDataOffset, &usSendSize))||
      ( usSendSize < (usDataOffset + sizeof(halStopReq.stopReqParams) )))
  {
     WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_WARN,
              "Unable to get send buffer in stop req %x %x %x",
                pEventData, pwdiStopParams, wdiStopRspCb);
     WDI_ASSERT(0);
     return WDI_STATUS_E_FAILURE;
  }

  /*                                                                       
                       
                                                                         */
  halStopReq.stopReqParams.reason = WDI_2_HAL_STOP_REASON(
                                          pwdiStopParams->wdiStopReason);

  wpalMemoryCopy( pSendBuffer+usDataOffset,
                  &halStopReq.stopReqParams,
                  sizeof(halStopReq.stopReqParams));

  pWDICtx->wdiReqStatusCB     = pwdiStopParams->wdiReqStatusCB;
  pWDICtx->pReqStatusUserData = pwdiStopParams->pUserData;

  /*                                */
  if ( eDRIVER_TYPE_MFG != pWDICtx->driverMode )
  {
     /*                                              
                                                                                 */
     WDI_STATableStop(pWDICtx);

     /*                                     */
     status = wpalEventReset(&pWDICtx->setPowerStateEvent);
     if (eWLAN_PAL_STATUS_SUCCESS != status)
     {
        WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_WARN,
                  "WDI Init failed to reset power state event");

        WDI_ASSERT(0);
        return VOS_STATUS_E_FAILURE;
     }
     /*                            */
     WDTS_SetPowerState(pWDICtx, WDTS_POWER_STATE_DOWN, WDI_SetPowerStateCb);
     /*
                                                                     
      */
     status = wpalEventWait(&pWDICtx->setPowerStateEvent,
                            WDI_SET_POWER_STATE_TIMEOUT);
     if (eWLAN_PAL_STATUS_SUCCESS != status)
     {
        WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_WARN,
                  "WDI Init failed to wait on an event");

        WDI_ASSERT(0);
        return VOS_STATUS_E_FAILURE;
      }
  }

  /*                                                                         
                            
                                                                           */
  return  WDI_SendMsg( pWDICtx, pSendBuffer, usSendSize,
                       wdiStopRspCb, pEventData->pUserData, WDI_STOP_RESP);

}/*                  */

/* 
                                                            
                  

                                                         
                                                                    

     
                                    
*/
WDI_Status
WDI_ProcessCloseReq
(
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
)
{
   wpt_status              wptStatus;
   /*                                                                        */

   /*                              */
   wpalMutexAcquire(&pWDICtx->wptMutex);

   /*                         */
   WDI_ClearPendingRequests(pWDICtx);

   /*                        */
   WCTS_CloseTransport(pWDICtx->wctsHandle);

   /*                     */
   /*                                  */
   if ( eDRIVER_TYPE_MFG != pWDICtx->driverMode )
   {
      WDTS_Close(pWDICtx);
   }

   /*                                               */
   WDI_STATableClose(pWDICtx);

   /*              */
   wptStatus = wpalClose(pWDICtx->pPALContext);
   if ( eWLAN_PAL_STATUS_SUCCESS !=  wptStatus )
   {
     WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_ERROR,
               "Failed to wpal Close %d", wptStatus);
     WDI_ASSERT(0);
   }

   /*                             */
   WDI_STATE_TRANSITION( pWDICtx, WDI_INIT_ST);

   wpalMutexRelease(&pWDICtx->wptMutex);

   /*                                                          */
   pWDICtx->ucExpectedStateTransition = WDI_INIT_ST;

   return WDI_STATUS_SUCCESS;
}/*                   */


/*                                                                           
                                                
                                                                           */

/* 
                                                                
                  

                                                         
                                                                    

     
                                    
*/
WDI_Status
WDI_ProcessInitScanReq
(
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
)
{
  WDI_InitScanReqParamsType*  pwdiInitScanParams    = NULL;
  WDI_InitScanRspCb           wdiInitScanRspCb      = NULL;
  wpt_uint8*                  pSendBuffer           = NULL;
  wpt_uint16                  usDataOffset          = 0;
  wpt_uint16                  usSendSize            = 0;
  wpt_uint8                   i = 0;

  tHalInitScanReqMsg          halInitScanReqMsg;

  /*                      
                                                               */
  tHalInitScanConReqMsg       halInitScanConReqMsg;

  /*                                                                        */

  /*                                                                         
                
                                                                             */
  if (( NULL == pEventData ) ||
      ( NULL == (pwdiInitScanParams = (WDI_InitScanReqParamsType*)pEventData->pEventData)) ||
      ( NULL == (wdiInitScanRspCb   = (WDI_InitScanRspCb)pEventData->pCBfnc)))
  {
    WPAL_TRACE( eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_WARN,
        "%s: Invalid parameters", __FUNCTION__);
    WDI_ASSERT(0);
    return WDI_STATUS_E_FAILURE;
  }

#if 0
  wpalMutexAcquire(&pWDICtx->wptMutex);
  /*                                                                       
                                                                      
                                    
                                      
                                                                           */
  if ( pWDICtx->bScanInProgress )
  {
    WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_ERROR,
        "Scan is already in progress - subsequent scan is not allowed"
        " until the first scan completes");

    wpalMutexRelease(&pWDICtx->wptMutex);
    return WDI_STATUS_E_NOT_ALLOWED;
  }

  pWDICtx->bScanInProgress = eWLAN_PAL_TRUE;
  pWDICtx->uScanState      = WDI_SCAN_INITIALIZED_ST;

  wpalMutexRelease(&pWDICtx->wptMutex);
#endif

  if (pwdiInitScanParams->wdiReqInfo.bUseNOA)
  {
    /*                      
                                                                 */
    /*                                                                       
                        
                                                                             */
    if (( WDI_STATUS_SUCCESS != WDI_GetMessageBuffer( pWDICtx, WDI_INIT_SCAN_CON_REQ,
            sizeof(halInitScanConReqMsg.initScanParams),
            &pSendBuffer, &usDataOffset, &usSendSize))||
        ( usSendSize < (usDataOffset + sizeof(halInitScanConReqMsg.initScanParams) )))
    {
      WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_WARN,
          "Unable to get send buffer in init scan req %x %x %x",
          pEventData, pwdiInitScanParams, wdiInitScanRspCb);
      WDI_ASSERT(0);
      return WDI_STATUS_E_FAILURE;
    }


    /*                                                                       
                         
                                                                             */
    halInitScanConReqMsg.initScanParams.scanMode =
      WDI_2_HAL_SCAN_MODE(pwdiInitScanParams->wdiReqInfo.wdiScanMode);

    wpalMemoryCopy(halInitScanConReqMsg.initScanParams.bssid,
        pwdiInitScanParams->wdiReqInfo.macBSSID, WDI_MAC_ADDR_LEN);

    halInitScanConReqMsg.initScanParams.notifyBss =
      pwdiInitScanParams->wdiReqInfo.bNotifyBSS;
    halInitScanConReqMsg.initScanParams.frameType =
      pwdiInitScanParams->wdiReqInfo.ucFrameType;
    halInitScanConReqMsg.initScanParams.frameLength =
      pwdiInitScanParams->wdiReqInfo.ucFrameLength;

    WDI_CopyWDIMgmFrameHdrToHALMgmFrameHdr( &halInitScanConReqMsg.initScanParams.macMgmtHdr,
        &pwdiInitScanParams->wdiReqInfo.wdiMACMgmtHdr);

#ifdef WLAN_FEATURE_P2P
    halInitScanConReqMsg.initScanParams.useNoA = pwdiInitScanParams->wdiReqInfo.bUseNOA;
    halInitScanConReqMsg.initScanParams.scanDuration = pwdiInitScanParams->wdiReqInfo.scanDuration;
#endif

    halInitScanConReqMsg.initScanParams.scanEntry.activeBSScnt =
      pwdiInitScanParams->wdiReqInfo.wdiScanEntry.activeBSScnt;

    for (i=0; i < pwdiInitScanParams->wdiReqInfo.wdiScanEntry.activeBSScnt; i++)
    {
      halInitScanConReqMsg.initScanParams.scanEntry.bssIdx[i] =
        pwdiInitScanParams->wdiReqInfo.wdiScanEntry.bssIdx[i];
    }

    wpalMemoryCopy( pSendBuffer+usDataOffset,
        &halInitScanConReqMsg.initScanParams,
        sizeof(halInitScanConReqMsg.initScanParams));
  }
  else
  {
    /*                                                                       
                        
                                                                             */
    if (( WDI_STATUS_SUCCESS != WDI_GetMessageBuffer( pWDICtx, WDI_INIT_SCAN_REQ,
            sizeof(halInitScanReqMsg.initScanParams),
            &pSendBuffer, &usDataOffset, &usSendSize))||
        ( usSendSize < (usDataOffset + sizeof(halInitScanReqMsg.initScanParams) )))
    {
      WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_WARN,
          "Unable to get send buffer in init scan req %x %x %x",
          pEventData, pwdiInitScanParams, wdiInitScanRspCb);
      WDI_ASSERT(0);
      return WDI_STATUS_E_FAILURE;
    }


    /*                                                                       
                         
                                                                             */
    halInitScanReqMsg.initScanParams.scanMode =
      WDI_2_HAL_SCAN_MODE(pwdiInitScanParams->wdiReqInfo.wdiScanMode);

    wpalMemoryCopy(halInitScanReqMsg.initScanParams.bssid,
        pwdiInitScanParams->wdiReqInfo.macBSSID, WDI_MAC_ADDR_LEN);

    halInitScanReqMsg.initScanParams.notifyBss =
      pwdiInitScanParams->wdiReqInfo.bNotifyBSS;
    halInitScanReqMsg.initScanParams.frameType =
      pwdiInitScanParams->wdiReqInfo.ucFrameType;
    halInitScanReqMsg.initScanParams.frameLength =
      pwdiInitScanParams->wdiReqInfo.ucFrameLength;

    WDI_CopyWDIMgmFrameHdrToHALMgmFrameHdr( &halInitScanReqMsg.initScanParams.macMgmtHdr,
        &pwdiInitScanParams->wdiReqInfo.wdiMACMgmtHdr);

    halInitScanReqMsg.initScanParams.scanEntry.activeBSScnt =
      pwdiInitScanParams->wdiReqInfo.wdiScanEntry.activeBSScnt;

    for (i=0; i < pwdiInitScanParams->wdiReqInfo.wdiScanEntry.activeBSScnt; i++)
    {
      halInitScanReqMsg.initScanParams.scanEntry.bssIdx[i] =
        pwdiInitScanParams->wdiReqInfo.wdiScanEntry.bssIdx[i];
    }

    wpalMemoryCopy( pSendBuffer+usDataOffset,
        &halInitScanReqMsg.initScanParams,
        sizeof(halInitScanReqMsg.initScanParams));
  }

  pWDICtx->wdiReqStatusCB     = pwdiInitScanParams->wdiReqStatusCB;
  pWDICtx->pReqStatusUserData = pwdiInitScanParams->pUserData;

  /*                                                                         
                                 
                                                                             */
  return  WDI_SendMsg( pWDICtx, pSendBuffer, usSendSize,
      wdiInitScanRspCb, pEventData->pUserData, WDI_INIT_SCAN_RESP);

}/*                      */

/* 
                                                             
                      

                                                         
                                                                    

     
                                    
*/
WDI_Status
WDI_ProcessStartScanReq
(
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
)
{
  WDI_StartScanReqParamsType*  pwdiStartScanParams    = NULL;
  WDI_StartScanRspCb           wdiStartScanRspCb      = NULL;
  wpt_uint8*                   pSendBuffer            = NULL;
  wpt_uint16                   usDataOffset           = 0;
  wpt_uint16                   usSendSize             = 0;

  tHalStartScanReqMsg          halStartScanReqMsg;
  /*                                                                        */

  /*                                                                         
                
                                                                           */
  if (( NULL == pEventData ) ||
      ( NULL == (pwdiStartScanParams = (WDI_StartScanReqParamsType*)pEventData->pEventData)) ||
      ( NULL == (wdiStartScanRspCb   = (WDI_StartScanRspCb)pEventData->pCBfnc)))
  {
     WPAL_TRACE( eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_WARN,
                 "%s: Invalid parameters", __FUNCTION__);
     WDI_ASSERT(0);
     return WDI_STATUS_E_FAILURE;
  }

#if 0
  wpalMutexAcquire(&pWDICtx->wptMutex);
  /*                                                                       
                                                                    
                                                                      
                         
                                                                         */
  if (( !pWDICtx->bScanInProgress ) ||
      (( WDI_SCAN_INITIALIZED_ST != pWDICtx->uScanState ) &&
       ( WDI_SCAN_ENDED_ST != pWDICtx->uScanState )))
  {
    WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_ERROR,
              "Scan start not allowed in this state %d %d",
               pWDICtx->bScanInProgress, pWDICtx->uScanState);

    wpalMutexRelease(&pWDICtx->wptMutex);
    return WDI_STATUS_E_NOT_ALLOWED;
  }

  pWDICtx->uScanState      = WDI_SCAN_STARTED_ST;

  wpalMutexRelease(&pWDICtx->wptMutex);
#endif

  /*                                                                       
                      
                                                                         */
  if (( WDI_STATUS_SUCCESS != WDI_GetMessageBuffer( pWDICtx, WDI_START_SCAN_REQ,
                        sizeof(halStartScanReqMsg.startScanParams),
                        &pSendBuffer, &usDataOffset, &usSendSize))||
      ( usSendSize < (usDataOffset + sizeof(halStartScanReqMsg.startScanParams) )))
  {
     WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_WARN,
              "Unable to get send buffer in start scan req %x %x %x",
                pEventData, pwdiStartScanParams, wdiStartScanRspCb);
     WDI_ASSERT(0);
     return WDI_STATUS_E_FAILURE;
  }

  halStartScanReqMsg.startScanParams.scanChannel =
                              pwdiStartScanParams->ucChannel;
  wpalMemoryCopy( pSendBuffer+usDataOffset,
                  &halStartScanReqMsg.startScanParams,
                  sizeof(halStartScanReqMsg.startScanParams));

  pWDICtx->wdiReqStatusCB     = pwdiStartScanParams->wdiReqStatusCB;
  pWDICtx->pReqStatusUserData = pwdiStartScanParams->pUserData;

  /*                                                                         
                                  
                                                                           */
  return  WDI_SendMsg( pWDICtx, pSendBuffer, usSendSize,
                       wdiStartScanRspCb, pEventData->pUserData, WDI_START_SCAN_RESP);
}/*                       */


/* 
                                                               
                  

                                                         
                                                                    

     
                                    
*/
WDI_Status
WDI_ProcessEndScanReq
(
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
)
{
  WDI_EndScanReqParamsType*  pwdiEndScanParams    = NULL;
  WDI_EndScanRspCb           wdiEndScanRspCb      = NULL;
  wpt_uint8*                 pSendBuffer          = NULL;
  wpt_uint16                 usDataOffset         = 0;
  wpt_uint16                 usSendSize           = 0;

  tHalEndScanReqMsg          halEndScanReqMsg;
  /*                                                                        */

  /*                                                                         
                
                                                                           */
  if (( NULL == pEventData ) ||
      ( NULL == (pwdiEndScanParams = (WDI_EndScanReqParamsType*)pEventData->pEventData)) ||
      ( NULL == (wdiEndScanRspCb   = (WDI_EndScanRspCb)pEventData->pCBfnc)))
  {
     WPAL_TRACE( eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_WARN,
                 "%s: Invalid parameters", __FUNCTION__);
     WDI_ASSERT(0);
     return WDI_STATUS_E_FAILURE;
  }

  /*                                                                        
                                                                  
                                       */
#if 0
  wpalMutexAcquire(&pWDICtx->wptMutex);
  /*                                                                       
                                                                  
                                                                      
              
                                                                         */
  if (( !pWDICtx->bScanInProgress ) ||
      ( WDI_SCAN_STARTED_ST != pWDICtx->uScanState ))
  {
    WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_WARN,
              "End start not allowed in this state %d %d",
               pWDICtx->bScanInProgress, pWDICtx->uScanState);

    wpalMutexRelease(&pWDICtx->wptMutex);
    return WDI_STATUS_E_NOT_ALLOWED;
  }

  pWDICtx->uScanState      = WDI_SCAN_ENDED_ST;

  wpalMutexRelease(&pWDICtx->wptMutex);
#endif

  /*                                                                       
                      
                                                                         */
  if (( WDI_STATUS_SUCCESS != WDI_GetMessageBuffer( pWDICtx, WDI_END_SCAN_REQ,
                        sizeof(halEndScanReqMsg.endScanParams),
                        &pSendBuffer, &usDataOffset, &usSendSize))||
      ( usSendSize < (usDataOffset + sizeof(halEndScanReqMsg.endScanParams) )))
  {
     WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_WARN,
              "Unable to get send buffer in start scan req %x %x %x",
                pEventData, pwdiEndScanParams, wdiEndScanRspCb);
     WDI_ASSERT(0);
     return WDI_STATUS_E_FAILURE;
  }

  halEndScanReqMsg.endScanParams.scanChannel = pwdiEndScanParams->ucChannel;

  wpalMemoryCopy( pSendBuffer+usDataOffset,
                  &halEndScanReqMsg.endScanParams,
                  sizeof(halEndScanReqMsg.endScanParams));

  pWDICtx->wdiReqStatusCB     = pwdiEndScanParams->wdiReqStatusCB;
  pWDICtx->pReqStatusUserData = pwdiEndScanParams->pUserData;

  /*                                                                         
                                
                                                                           */
  return  WDI_SendMsg( pWDICtx, pSendBuffer, usSendSize,
                       wdiEndScanRspCb, pEventData->pUserData, WDI_END_SCAN_RESP);
}/*                     */


/* 
                                                              
                      

                                                         
                                                                    

     
                                    
*/
WDI_Status
WDI_ProcessFinishScanReq
(
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
)
{
  WDI_FinishScanReqParamsType*  pwdiFinishScanParams;
  WDI_FinishScanRspCb           wdiFinishScanRspCb;
  wpt_uint8*                    pSendBuffer          = NULL;
  wpt_uint16                    usDataOffset         = 0;
  wpt_uint16                    usSendSize           = 0;
  wpt_uint8                     i                    = 0;

  tHalFinishScanReqMsg          halFinishScanReqMsg;
  /*                                                                        */

  /*                                                                         
                
                                                                           */
  if (( NULL == pEventData ) ||
      ( NULL == pEventData->pEventData) ||
      ( NULL == pEventData->pCBfnc))
  {
     WPAL_TRACE( eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_WARN,
                 "%s: Invalid parameters", __FUNCTION__);
     WDI_ASSERT(0);
     return WDI_STATUS_E_FAILURE;
  }

  pwdiFinishScanParams = (WDI_FinishScanReqParamsType*)pEventData->pEventData;
  wdiFinishScanRspCb   = (WDI_FinishScanRspCb)pEventData->pCBfnc;
  /*                                                                        
                                                                  
                                       */
#if 0
  wpalMutexAcquire(&pWDICtx->wptMutex);
   /*                                                                       
                                               
                                                            
                                       
                                                                         */
  if ( !pWDICtx->bScanInProgress )
  {
    WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_WARN,
              "Finish start not allowed in this state %d",
               pWDICtx->bScanInProgress );

    wpalMutexRelease(&pWDICtx->wptMutex);
    return WDI_STATUS_E_NOT_ALLOWED;
  }

  /*                                                                       
                                                                            
                                                              
                                                                         */
  pWDICtx->uScanState      = WDI_SCAN_FINISHED_ST;
  pWDICtx->bScanInProgress = eWLAN_PAL_FALSE;
  wpalMutexRelease(&pWDICtx->wptMutex);
#endif

  if ( pWDICtx->bInBmps )
  {
     //                                     
     WDTS_SetPowerState(pWDICtx, WDTS_POWER_STATE_BMPS, NULL);
  }

  /*                                                                       
                      
                                                                         */
  if (( WDI_STATUS_SUCCESS != WDI_GetMessageBuffer( pWDICtx, WDI_FINISH_SCAN_REQ,
                        sizeof(halFinishScanReqMsg.finishScanParams),
                        &pSendBuffer, &usDataOffset, &usSendSize))||
      ( usSendSize < (usDataOffset + sizeof(halFinishScanReqMsg.finishScanParams) )))
  {
     WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_WARN,
              "Unable to get send buffer in start scan req %x %x %x",
                pEventData, pwdiFinishScanParams, wdiFinishScanRspCb);
     WDI_ASSERT(0);
     return WDI_STATUS_E_FAILURE;
  }

  halFinishScanReqMsg.finishScanParams.scanMode =
    WDI_2_HAL_SCAN_MODE(pwdiFinishScanParams->wdiReqInfo.wdiScanMode);

  halFinishScanReqMsg.finishScanParams.currentOperChannel =
    pwdiFinishScanParams->wdiReqInfo.ucCurrentOperatingChannel;

  halFinishScanReqMsg.finishScanParams.cbState =
    WDI_2_HAL_CB_STATE(pwdiFinishScanParams->wdiReqInfo.wdiCBState);

  wpalMemoryCopy(halFinishScanReqMsg.finishScanParams.bssid,
                 pwdiFinishScanParams->wdiReqInfo.macBSSID, WDI_MAC_ADDR_LEN);

  halFinishScanReqMsg.finishScanParams.notifyBss   =
                              pwdiFinishScanParams->wdiReqInfo.bNotifyBSS ;
  halFinishScanReqMsg.finishScanParams.frameType   =
                              pwdiFinishScanParams->wdiReqInfo.ucFrameType ;
  halFinishScanReqMsg.finishScanParams.frameLength =
                              pwdiFinishScanParams->wdiReqInfo.ucFrameLength ;

  halFinishScanReqMsg.finishScanParams.scanEntry.activeBSScnt =
                   pwdiFinishScanParams->wdiReqInfo.wdiScanEntry.activeBSScnt ;

  for (i = 0; i < pwdiFinishScanParams->wdiReqInfo.wdiScanEntry.activeBSScnt; i++)
  {
    halFinishScanReqMsg.finishScanParams.scanEntry.bssIdx[i] =
               pwdiFinishScanParams->wdiReqInfo.wdiScanEntry.bssIdx[i] ;
  }

  WDI_CopyWDIMgmFrameHdrToHALMgmFrameHdr( &halFinishScanReqMsg.finishScanParams.macMgmtHdr,
                              &pwdiFinishScanParams->wdiReqInfo.wdiMACMgmtHdr);

  wpalMemoryCopy( pSendBuffer+usDataOffset,
                  &halFinishScanReqMsg.finishScanParams,
                  sizeof(halFinishScanReqMsg.finishScanParams));

  pWDICtx->wdiReqStatusCB     = pwdiFinishScanParams->wdiReqStatusCB;
  pWDICtx->pReqStatusUserData = pwdiFinishScanParams->pUserData;

  /*                                                                         
                                   
                                                                           */
  return  WDI_SendMsg( pWDICtx, pSendBuffer, usSendSize,
                       wdiFinishScanRspCb, pEventData->pUserData, WDI_FINISH_SCAN_RESP);
}/*                        */


/*                                                                          
                                           
                                                                          */
/* 
                                            

                                                         
                                                                    

     
                                    
*/
WDI_Status
WDI_ProcessBSSSessionJoinReq
(
  WDI_ControlBlockType*   pWDICtx,
  WDI_JoinReqParamsType*  pwdiJoinParams,
  WDI_JoinRspCb           wdiJoinRspCb,
  void*                   pUserData
)
{
  WDI_BSSSessionType*     pBSSSes             = NULL;
  wpt_uint8*              pSendBuffer         = NULL;
  wpt_uint16              usDataOffset        = 0;
  wpt_uint16              usSendSize          = 0;
  wpt_uint8               ucCurrentBSSSesIdx  = 0;

  tHalJoinReqMsg          halJoinReqMsg;
  /*                                                                        */

  /*                                                                        
                                                                          
              
                                                                          */
  ucCurrentBSSSesIdx = WDI_FindAssocSession( pWDICtx,
                                   pwdiJoinParams->wdiReqInfo.macBSSID,
                                  &pBSSSes);

  if ( NULL != pBSSSes )
  {
    WPAL_TRACE( eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_ERROR,
              "Association for this BSSID is already in place");

    return WDI_STATUS_E_NOT_ALLOWED;
  }

  wpalMutexAcquire(&pWDICtx->wptMutex);
  /*                                                                        
                                
                                                                          */
  ucCurrentBSSSesIdx = WDI_FindEmptySession( pWDICtx, &pBSSSes);
  if ( NULL == pBSSSes )
  {

    WPAL_TRACE( eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_ERROR,
              "DAL has no free sessions - cannot run another join");

    wpalMutexRelease(&pWDICtx->wptMutex);
    return WDI_STATUS_RES_FAILURE;
  }

  /*                     */
  pBSSSes->bInUse = eWLAN_PAL_TRUE;
  wpalMemoryCopy( pBSSSes->macBSSID, pwdiJoinParams->wdiReqInfo.macBSSID,
                  WDI_MAC_ADDR_LEN);

  /*                           */
  pBSSSes->wdiAssocState      = WDI_ASSOC_JOINING_ST;
  pWDICtx->ucCurrentBSSSesIdx = ucCurrentBSSSesIdx;

  wpalMutexRelease(&pWDICtx->wptMutex);

  /*                                                                       
                      
                                                                         */
  if (( WDI_STATUS_SUCCESS != WDI_GetMessageBuffer( pWDICtx, WDI_JOIN_REQ,
                        sizeof(halJoinReqMsg.joinReqParams),
                        &pSendBuffer, &usDataOffset, &usSendSize))||
      ( usSendSize < (usDataOffset + sizeof(halJoinReqMsg.joinReqParams) )))
  {
     WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_WARN,
              "Unable to get send buffer in join req %x %x %x",
                pUserData, pwdiJoinParams, wdiJoinRspCb);
     WDI_ASSERT(0);
     return WDI_STATUS_E_FAILURE;
  }

  wpalMemoryCopy(halJoinReqMsg.joinReqParams.bssId,
                 pwdiJoinParams->wdiReqInfo.macBSSID, WDI_MAC_ADDR_LEN);

  wpalMemoryCopy(halJoinReqMsg.joinReqParams.selfStaMacAddr,
                 pwdiJoinParams->wdiReqInfo.macSTASelf,
                 WDI_MAC_ADDR_LEN);

  halJoinReqMsg.joinReqParams.ucChannel =
    pwdiJoinParams->wdiReqInfo.wdiChannelInfo.ucChannel;

  halJoinReqMsg.joinReqParams.linkState = pwdiJoinParams->wdiReqInfo.linkState;

#ifdef WLAN_FEATURE_VOWIFI
  halJoinReqMsg.joinReqParams.maxTxPower =
    pwdiJoinParams->wdiReqInfo.wdiChannelInfo.cMaxTxPower;
#else
  halJoinReqMsg.joinReqParams.ucLocalPowerConstraint =
    pwdiJoinParams->wdiReqInfo.wdiChannelInfo.ucLocalPowerConstraint;
#endif

  halJoinReqMsg.joinReqParams.secondaryChannelOffset =
     WDI_2_HAL_SEC_CH_OFFSET(pwdiJoinParams->wdiReqInfo.wdiChannelInfo.
                             wdiSecondaryChannelOffset);

  wpalMemoryCopy( pSendBuffer+usDataOffset,
                  &halJoinReqMsg.joinReqParams,
                  sizeof(halJoinReqMsg.joinReqParams));

  pWDICtx->wdiReqStatusCB     = pwdiJoinParams->wdiReqStatusCB;
  pWDICtx->pReqStatusUserData = pwdiJoinParams->pUserData;

  /*                                                                         
                            
                                                                           */
  return  WDI_SendMsg( pWDICtx, pSendBuffer, usSendSize,
                       wdiJoinRspCb, pUserData, WDI_JOIN_RESP);

}/*                            */

/* 
                                                           
                  

                                                         
                                                                    

     
                                    
*/
WDI_Status
WDI_ProcessJoinReq
(
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
)
{
  WDI_Status              wdiStatus          = WDI_STATUS_SUCCESS;
  WDI_JoinReqParamsType*  pwdiJoinParams     = NULL;
  WDI_JoinRspCb           wdiJoinRspCb       = NULL;
  /*                                                                        */

  /*                                                                         
                
                                                                           */
  if (( NULL == pEventData ) ||
      ( NULL == (pwdiJoinParams = (WDI_JoinReqParamsType*)pEventData->pEventData)) ||
      ( NULL == (wdiJoinRspCb   = (WDI_JoinRspCb)pEventData->pCBfnc)))
  {
     WPAL_TRACE( eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_WARN,
                 "%s: Invalid parameters", __FUNCTION__);
     WDI_ASSERT(0);
     return WDI_STATUS_E_FAILURE;
  }

  /*                                                                         
                                                                           
                                              
                                                                           */
  wpalMutexAcquire(&pWDICtx->wptMutex);

  if ( eWLAN_PAL_FALSE != pWDICtx->bAssociationInProgress )
  {
    WPAL_TRACE( eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_WARN,
              "Association is currently in progress, queueing new join req");

    /*                                              */
    wdiStatus = WDI_QueueNewAssocRequest(pWDICtx, pEventData,
                             pwdiJoinParams->wdiReqInfo.macBSSID);

    wpalMutexRelease(&pWDICtx->wptMutex);

    return wdiStatus;
  }

  /*                           */
  pWDICtx->bAssociationInProgress = eWLAN_PAL_TRUE;
  wpalMutexRelease(&pWDICtx->wptMutex);

  /*                        */
  return WDI_ProcessBSSSessionJoinReq( pWDICtx, pwdiJoinParams,
                                       wdiJoinRspCb,pEventData->pUserData);

}/*                  */


/* 
                                                             
                      

                                                         
                                                                    

     
                                    
*/
WDI_Status
WDI_ProcessConfigBSSReq
(
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
)
{
  WDI_ConfigBSSReqParamsType*  pwdiConfigBSSParams;
  WDI_ConfigBSSRspCb           wdiConfigBSSRspCb;
  wpt_uint8                    ucCurrentBSSSesIdx  = 0;
  WDI_BSSSessionType*          pBSSSes             = NULL;
  wpt_uint16                   uMsgSize            = 0;
  wpt_uint8*                   pSendBuffer         = NULL;
  wpt_uint16                   usDataOffset        = 0;
  wpt_uint16                   usSendSize          = 0;
  WDI_Status                   wdiStatus           = WDI_STATUS_SUCCESS;

  tConfigBssReqMsg             halConfigBssReqMsg;
  /*                                                                        */

  /*                                                                         
                
                                                                           */
  if (( NULL == pEventData ) ||
      ( NULL == pEventData->pEventData ) ||
      ( NULL == pEventData->pCBfnc ))
  {
     WPAL_TRACE( eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_WARN,
                 "%s: Invalid parameters", __FUNCTION__);
     WDI_ASSERT(0);
     return WDI_STATUS_E_FAILURE;
  }

  pwdiConfigBSSParams = (WDI_ConfigBSSReqParamsType*)pEventData->pEventData;
  wdiConfigBSSRspCb   = (WDI_ConfigBSSRspCb)pEventData->pCBfnc;
  /*                                                                         
                                                                           
                                              
                                                                           */
  wpalMutexAcquire(&pWDICtx->wptMutex);

  /*                                                                        
                                              
                                                                          */
  ucCurrentBSSSesIdx = WDI_FindAssocSession( pWDICtx,
                                 pwdiConfigBSSParams->wdiReqInfo.macBSSID,
                                 &pBSSSes);

  if ( NULL == pBSSSes )
  {
#ifdef WLAN_FEATURE_VOWIFI_11R
      /*                                                                        
                                    
                                                                              */
      ucCurrentBSSSesIdx = WDI_FindEmptySession( pWDICtx, &pBSSSes);
      if ( NULL == pBSSSes )
      {

        WPAL_TRACE( eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_ERROR,
                  "DAL has no free sessions - cannot run another join");

        wpalMutexRelease(&pWDICtx->wptMutex);
        return WDI_STATUS_RES_FAILURE;
      }

      /*                     */
      pBSSSes->bInUse = eWLAN_PAL_TRUE;
      wpalMemoryCopy( pBSSSes->macBSSID, pwdiConfigBSSParams->wdiReqInfo.macBSSID,
                      WDI_MAC_ADDR_LEN);

      /*                           */
      pBSSSes->wdiAssocState      = WDI_ASSOC_JOINING_ST;
      pWDICtx->ucCurrentBSSSesIdx = ucCurrentBSSSesIdx;
#else
    /*                                                                    
                              */
    if((pwdiConfigBSSParams->wdiReqInfo.wdiBSSType == WDI_IBSS_MODE) ||
       (pwdiConfigBSSParams->wdiReqInfo.wdiBSSType == WDI_INFRA_AP_MODE) ||
       (pwdiConfigBSSParams->wdiReqInfo.wdiBSSType == WDI_BTAMP_AP_MODE) ||
       (pwdiConfigBSSParams->wdiReqInfo.wdiBSSType == WDI_BTAMP_STA_MODE))
    {
      /*                                                                        
                                    
                                                                              */
      ucCurrentBSSSesIdx = WDI_FindEmptySession( pWDICtx, &pBSSSes);
      if ( NULL == pBSSSes )
      {

        WPAL_TRACE( eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_ERROR,
                  "DAL has no free sessions - cannot run another join");

        wpalMutexRelease(&pWDICtx->wptMutex);
        return WDI_STATUS_RES_FAILURE;
      }

      /*                     */
      pBSSSes->bInUse = eWLAN_PAL_TRUE;
      wpalMemoryCopy( pBSSSes->macBSSID, pwdiConfigBSSParams->wdiReqInfo.macBSSID,
                      WDI_MAC_ADDR_LEN);

      /*                           */
      pBSSSes->wdiAssocState      = WDI_ASSOC_JOINING_ST;
      pWDICtx->ucCurrentBSSSesIdx = ucCurrentBSSSesIdx;
    }
    else
    {
      WPAL_TRACE( eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_ERROR,
                "%s: Association sequence for this BSS does not yet exist." MAC_ADDRESS_STR "wdiBssType %d",
                __func__, MAC_ADDR_ARRAY(pwdiConfigBSSParams->wdiReqInfo.macBSSID),
                pwdiConfigBSSParams->wdiReqInfo.wdiBSSType);

      /*                  */
      wpalMutexRelease(&pWDICtx->wptMutex);
      return WDI_STATUS_E_NOT_ALLOWED;
    }
#endif
  }

  /*                                                                        
                                                             
                                             
                                                                          */
  if ( eWLAN_PAL_TRUE == pBSSSes->bAssocReqQueued )
  {
    WPAL_TRACE( eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_WARN,
              "%s: Association sequence for this BSS exists but currently queued. " MAC_ADDRESS_STR " bssIdx %d",
              __func__, MAC_ADDR_ARRAY(pwdiConfigBSSParams->wdiReqInfo.macBSSID),
              ucCurrentBSSSesIdx);

    wdiStatus = WDI_QueueAssocRequest( pWDICtx, pBSSSes, pEventData);

    wpalMutexRelease(&pWDICtx->wptMutex);

    return wdiStatus;
  }

  /*                                           */
  wpalMemoryCopy(&pWDICtx->wdiCachedConfigBssReq,
                 pwdiConfigBSSParams,
                 sizeof(pWDICtx->wdiCachedConfigBssReq));

  wpalMutexRelease(&pWDICtx->wptMutex);

  /*                                                               */
#ifdef WLAN_FEATURE_11AC
  if (WDI_getFwWlanFeatCaps(DOT11AC))
	  uMsgSize = sizeof(halConfigBssReqMsg.uBssParams.configBssParams_V1); //                     
  else
#endif
	  uMsgSize = sizeof(halConfigBssReqMsg.uBssParams.configBssParams); //                              

  /*                                                                       
                      
                                                                         */
  if (( WDI_STATUS_SUCCESS != WDI_GetMessageBuffer( pWDICtx, WDI_CONFIG_BSS_REQ,
                    uMsgSize, &pSendBuffer, &usDataOffset, &usSendSize))||
      ( usSendSize < (usDataOffset + uMsgSize )))
  {
     WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_WARN,
              "Unable to get send buffer in config bss req %x %x %x",
                pEventData, pwdiConfigBSSParams, wdiConfigBSSRspCb);
     WDI_ASSERT(0);
     return WDI_STATUS_E_FAILURE;
  }

  /*                     */
#ifdef WLAN_FEATURE_11AC
  if (WDI_getFwWlanFeatCaps(DOT11AC))
    WDI_CopyWDIConfigBSSToHALConfigBSS( (tConfigBssParams*)&halConfigBssReqMsg.uBssParams.configBssParams_V1,
                                        &pwdiConfigBSSParams->wdiReqInfo);
  else
#endif
  WDI_CopyWDIConfigBSSToHALConfigBSS( &halConfigBssReqMsg.uBssParams.configBssParams,
                                      &pwdiConfigBSSParams->wdiReqInfo);

  /*                                                                          
                              */
  halConfigBssReqMsg.uBssParams.configBssParams.staContext.staIdx = WDI_STA_INVALID_IDX;

  /*                               */
  halConfigBssReqMsg.uBssParams.configBssParams.staContext.bssIdx = pBSSSes->ucBSSIdx;

  wpalMemoryCopy( pSendBuffer+usDataOffset,
                  &halConfigBssReqMsg.uBssParams.configBssParams,
                  uMsgSize);

  pWDICtx->wdiReqStatusCB     = pwdiConfigBSSParams->wdiReqStatusCB;
  pWDICtx->pReqStatusUserData = pwdiConfigBSSParams->pUserData;

  /*                                                                         
                                  
                                                                           */
  return  WDI_SendMsg( pWDICtx, pSendBuffer, usSendSize,
                       wdiConfigBSSRspCb, pEventData->pUserData,
                       WDI_CONFIG_BSS_RESP);

}/*                       */


/* 
                                                              
                  

                                                         
                                                                    

     
                                    
*/
WDI_Status
WDI_ProcessDelBSSReq
(
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
)
{
  WDI_DelBSSReqParamsType*  pwdiDelBSSParams    = NULL;
  WDI_DelBSSRspCb           wdiDelBSSRspCb      = NULL;
  wpt_uint8                 ucCurrentBSSSesIdx  = 0;
  WDI_BSSSessionType*       pBSSSes             = NULL;
  wpt_uint8*                pSendBuffer         = NULL;
  wpt_uint16                usDataOffset        = 0;
  wpt_uint16                usSendSize          = 0;
  WDI_Status                wdiStatus           = WDI_STATUS_SUCCESS;

  tDeleteBssReqMsg          halBssReqMsg;
  /*                                                                        */

  /*                                                                         
                
                                                                           */
  if (( NULL == pEventData ) ||
      ( NULL == (pwdiDelBSSParams = (WDI_DelBSSReqParamsType*)pEventData->pEventData)) ||
      ( NULL == (wdiDelBSSRspCb   = (WDI_DelBSSRspCb)pEventData->pCBfnc)))
  {
     WPAL_TRACE( eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_WARN,
                 "%s: Invalid parameters", __FUNCTION__);
     WDI_ASSERT(0);
     return WDI_STATUS_E_FAILURE;
  }

  /*                                                                         
                                                                           
                                              
                                                                           */
  wpalMutexAcquire(&pWDICtx->wptMutex);

  /*                                                                        
                                              
                                                                          */
  ucCurrentBSSSesIdx = WDI_FindAssocSessionByBSSIdx( pWDICtx,
                                             pwdiDelBSSParams->ucBssIdx,
                                            &pBSSSes);

  if ( NULL == pBSSSes )
  {
    WPAL_TRACE( eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_ERROR,
        "%s: BSS does not yet exist. ucBssIdx %d",
        __func__, pwdiDelBSSParams->ucBssIdx);

    wpalMutexRelease(&pWDICtx->wptMutex);

    return WDI_STATUS_E_NOT_ALLOWED;
  }

  /*                                                                        
                                                             
                                             
                                                                          */
  if ( eWLAN_PAL_TRUE == pBSSSes->bAssocReqQueued )
  {
    WPAL_TRACE( eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_WARN,
              "%s: Association sequence for this BSS exists but currently queued. ucBssIdx %d",
              __func__, pwdiDelBSSParams->ucBssIdx);

    wdiStatus = WDI_QueueAssocRequest( pWDICtx, pBSSSes, pEventData);

    wpalMutexRelease(&pWDICtx->wptMutex);

    return wdiStatus;
  }

  /*                                                                       
                                                                         
                                                                         
                                                                        
                                                                          
                                                                         */
  if ( ucCurrentBSSSesIdx == pWDICtx->ucCurrentBSSSesIdx )
  {
    /*                                                                      
                                                                            
                                             */
     pWDICtx->bAssociationInProgress = eWLAN_PAL_FALSE;

     /*                                                               
                  */
     WDI_DequeueAssocRequest(pWDICtx);
  }

  wpalMutexRelease(&pWDICtx->wptMutex);
  /*                                                                       
                      
                                                                         */
  if (( WDI_STATUS_SUCCESS != WDI_GetMessageBuffer( pWDICtx, WDI_DEL_BSS_REQ,
                        sizeof(halBssReqMsg.deleteBssParams),
                        &pSendBuffer, &usDataOffset, &usSendSize))||
      ( usSendSize < (usDataOffset + sizeof(halBssReqMsg.deleteBssParams) )))
  {
     WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_WARN,
              "Unable to get send buffer in start req %x %x %x",
                pEventData, pwdiDelBSSParams, wdiDelBSSRspCb);
     WDI_ASSERT(0);
     return WDI_STATUS_E_FAILURE;
  }

  /*                                     */

  /*                                                                  */
  halBssReqMsg.deleteBssParams.bssIdx = pBSSSes->ucBSSIdx;

  wpalMemoryCopy( pSendBuffer+usDataOffset,
                  &halBssReqMsg.deleteBssParams,
                  sizeof(halBssReqMsg.deleteBssParams));

  pWDICtx->wdiReqStatusCB     = pwdiDelBSSParams->wdiReqStatusCB;
  pWDICtx->pReqStatusUserData = pwdiDelBSSParams->pUserData;


  /*                                                                         
                               
                                                                           */
  return  WDI_SendMsg( pWDICtx, pSendBuffer, usSendSize,
                       wdiDelBSSRspCb, pEventData->pUserData, WDI_DEL_BSS_RESP);


}/*                    */

/* 
                                                             
                      

                                                         
                                                                    

     
                                    
*/
WDI_Status
WDI_ProcessPostAssocReq
(
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
)
{
  WDI_PostAssocReqParamsType* pwdiPostAssocParams   = NULL;
  WDI_PostAssocRspCb          wdiPostAssocRspCb     = NULL;
  wpt_uint8                   ucCurrentBSSSesIdx    = 0;
  WDI_BSSSessionType*         pBSSSes               = NULL;
  wpt_uint8*                  pSendBuffer           = NULL;
  wpt_uint16                  usDataOffset          = 0;
  wpt_uint16                  usSendSize            = 0;
  wpt_uint16                  uMsgSize              = 0;
  wpt_uint16                  uOffset               = 0;
  WDI_Status                  wdiStatus             = WDI_STATUS_SUCCESS;

  tPostAssocReqMsg            halPostAssocReqMsg;
  /*                                                                        */

  /*                                                                         
                
                                                                           */
  if (( NULL == pEventData ) ||
      ( NULL == (pwdiPostAssocParams = (WDI_PostAssocReqParamsType*)pEventData->pEventData)) ||
      ( NULL == (wdiPostAssocRspCb   = (WDI_PostAssocRspCb)pEventData->pCBfnc)))
  {
     WPAL_TRACE( eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_WARN,
                 "%s: Invalid parameters", __FUNCTION__);
     WDI_ASSERT(0);
     return WDI_STATUS_E_FAILURE;
  }

  /*                                                                         
                                                                           
                                              
                                                                           */
  wpalMutexAcquire(&pWDICtx->wptMutex);

  /*                                                                        
                                              
                                                                          */
  ucCurrentBSSSesIdx = WDI_FindAssocSession( pWDICtx,
                              pwdiPostAssocParams->wdiBSSParams.macBSSID,
                              &pBSSSes);

  if ( NULL == pBSSSes )
  {
    WPAL_TRACE( eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_ERROR,
              "%s: Association sequence for this BSS does not yet exist - "
              "operation not allowed. macBSSID " MAC_ADDRESS_STR,
              __func__, MAC_ADDR_ARRAY(pwdiPostAssocParams->wdiBSSParams.macBSSID));

    wpalMutexRelease(&pWDICtx->wptMutex);
    return WDI_STATUS_E_NOT_ALLOWED;
  }

  /*                                                                        
                                                             
                                             
                                                                          */
  if ( eWLAN_PAL_TRUE == pBSSSes->bAssocReqQueued )
  {
    WPAL_TRACE( eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_WARN,
              "%s: Association sequence for this BSS exists but currently queued. macBSSID " MAC_ADDRESS_STR,
              __func__, MAC_ADDR_ARRAY(pwdiPostAssocParams->wdiBSSParams.macBSSID));

    wdiStatus = WDI_QueueAssocRequest( pWDICtx, pBSSSes, pEventData);

    wpalMutexRelease(&pWDICtx->wptMutex);

    return wdiStatus;
  }

  /*                                                                       
                                                                     
                  
                                                                           */
  if (( ucCurrentBSSSesIdx != pWDICtx->ucCurrentBSSSesIdx ) ||
      ( eWLAN_PAL_FALSE == pWDICtx->bAssociationInProgress ))
  {
    WPAL_TRACE( eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_ERROR,
              "Association sequence for this BSS association no longer in "
              "progress - not allowed");

    wpalMutexRelease(&pWDICtx->wptMutex);
    return WDI_STATUS_E_NOT_ALLOWED;
  }

  /*                                                                       
                                                       
                                                                         */
  if ( WDI_ASSOC_JOINING_ST != pBSSSes->wdiAssocState)
  {
    WPAL_TRACE( eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_ERROR,
              "Post Assoc not allowed before JOIN - failing request");

    wpalMutexRelease(&pWDICtx->wptMutex);
    return WDI_STATUS_E_NOT_ALLOWED;
  }

  wpalMutexRelease(&pWDICtx->wptMutex);

  uMsgSize = sizeof(halPostAssocReqMsg.postAssocReqParams.configStaParams) +
             sizeof(halPostAssocReqMsg.postAssocReqParams.configBssParams) ;
  /*                                                                       
                                    
                                                                         */
  if (( WDI_STATUS_SUCCESS != WDI_GetMessageBuffer( pWDICtx, WDI_POST_ASSOC_REQ,
                        uMsgSize,&pSendBuffer, &usDataOffset, &usSendSize))||
      ( usSendSize < (usDataOffset + uMsgSize )))
  {
     WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_WARN,
              "Unable to get send buffer in start req %x %x %x",
                pEventData, pwdiPostAssocParams, wdiPostAssocRspCb);
     WDI_ASSERT(0);
     return WDI_STATUS_E_FAILURE;
  }

  /*                        */
  WDI_CopyWDIStaCtxToHALStaCtx(&halPostAssocReqMsg.postAssocReqParams.configStaParams,
                               &pwdiPostAssocParams->wdiSTAParams );

  /*                                    */
  if ( WDI_STATUS_SUCCESS !=
       WDI_STATableFindStaidByAddr(pWDICtx,
                                   pwdiPostAssocParams->wdiSTAParams.macSTA,
                                   (wpt_uint8*)&halPostAssocReqMsg.postAssocReqParams.configStaParams.staIdx ))
  {
    WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_WARN,
             "This station does not exist in the WDI Station Table %d");
    wpalMutexRelease(&pWDICtx->wptMutex);
    return WDI_STATUS_E_FAILURE;
  }

  /*                               */
  halPostAssocReqMsg.postAssocReqParams.configStaParams.bssIdx =
     pBSSSes->ucBSSIdx;

  /*                        */
  WDI_CopyWDIConfigBSSToHALConfigBSS( &halPostAssocReqMsg.postAssocReqParams.configBssParams,
                                      &pwdiPostAssocParams->wdiBSSParams);

  /*                                           */
  if ( WDI_STATUS_SUCCESS !=
       WDI_STATableFindStaidByAddr(pWDICtx,
                                   pwdiPostAssocParams->wdiBSSParams.wdiSTAContext.macSTA,
                                   (wpt_uint8*)&halPostAssocReqMsg.postAssocReqParams.configBssParams.staContext.staIdx))
  {
    WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_WARN,
             "This station does not exist in the WDI Station Table %d");
    wpalMutexRelease(&pWDICtx->wptMutex);
    return WDI_STATUS_E_FAILURE;
  }

  /*                               */
  halPostAssocReqMsg.postAssocReqParams.configStaParams.bssIdx =
     pBSSSes->ucBSSIdx;


  wpalMemoryCopy( pSendBuffer+usDataOffset,
                  &halPostAssocReqMsg.postAssocReqParams.configStaParams,
                  sizeof(halPostAssocReqMsg.postAssocReqParams.configStaParams));

  uOffset = sizeof(halPostAssocReqMsg.postAssocReqParams.configStaParams);

  wpalMemoryCopy( pSendBuffer+usDataOffset + uOffset,
                  &halPostAssocReqMsg.postAssocReqParams.configBssParams,
                  sizeof(halPostAssocReqMsg.postAssocReqParams.configBssParams));


  pWDICtx->wdiReqStatusCB     = pwdiPostAssocParams->wdiReqStatusCB;
  pWDICtx->pReqStatusUserData = pwdiPostAssocParams->pUserData;


  wpalMemoryCopy( &pWDICtx->wdiCachedPostAssocReq,
                  pwdiPostAssocParams,
                  sizeof(pWDICtx->wdiCachedPostAssocReq));

  /*                                                                         
                                  
                                                                           */
  return  WDI_SendMsg( pWDICtx, pSendBuffer, usSendSize,
                       wdiPostAssocRspCb, pEventData->pUserData, WDI_POST_ASSOC_RESP);


}/*                       */

/* 
                                                              
                  

                                                         
                                                                    

     
                                    
*/
WDI_Status
WDI_ProcessDelSTAReq
(
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
)
{
  WDI_DelSTAReqParamsType*  pwdiDelSTAParams;
  WDI_DelSTARspCb           wdiDelSTARspCb;
  wpt_uint8                 ucCurrentBSSSesIdx  = 0;
  WDI_BSSSessionType*       pBSSSes             = NULL;
  wpt_uint8*                pSendBuffer         = NULL;
  wpt_uint16                usDataOffset        = 0;
  wpt_uint16                usSendSize          = 0;
  wpt_macAddr               macBSSID;
  WDI_Status                wdiStatus           = WDI_STATUS_SUCCESS;

  tDeleteStaReqMsg          halDelStaReqMsg;
  /*                                                                        */

  /*                                                                         
                
                                                                           */
  if (( NULL == pEventData ) || ( NULL == pEventData->pEventData ) ||
      ( NULL == pEventData->pCBfnc ))
  {
     WPAL_TRACE( eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_WARN,
                 "%s: Invalid parameters", __FUNCTION__);
     WDI_ASSERT(0);
     return WDI_STATUS_E_FAILURE;
  }

  pwdiDelSTAParams = (WDI_DelSTAReqParamsType*)pEventData->pEventData;
  wdiDelSTARspCb   = (WDI_DelSTARspCb)pEventData->pCBfnc;
  /*                                                                         
                                                                           
                                              
                                                                           */
  wpalMutexAcquire(&pWDICtx->wptMutex);

  /*                                                                        
                                                                       
                                                                          */
  if ( WDI_STATUS_SUCCESS != WDI_STATableGetStaBSSIDAddr(pWDICtx,
                                                         pwdiDelSTAParams->ucSTAIdx,
                                                         &macBSSID))
  {
    WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_WARN,
             "This station does not exist in the WDI Station Table %d");
    wpalMutexRelease(&pWDICtx->wptMutex);
    return WDI_STATUS_E_FAILURE;
  }

  ucCurrentBSSSesIdx = WDI_FindAssocSession( pWDICtx, macBSSID, &pBSSSes);
  if ( NULL == pBSSSes )
  {
    WPAL_TRACE( eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_ERROR,
              "%s: Association sequence for this BSS does not yet exist. macBSSID " MAC_ADDRESS_STR,
              __func__, MAC_ADDR_ARRAY(macBSSID));

    wpalMutexRelease(&pWDICtx->wptMutex);
    return WDI_STATUS_E_NOT_ALLOWED;
  }

  /*                                                                        
                                                             
                                             
                                                                          */
  if ( eWLAN_PAL_TRUE == pBSSSes->bAssocReqQueued )
  {
    WPAL_TRACE( eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_WARN,
              "%s: Association sequence for this BSS exists but currently queued. macBSSID " MAC_ADDRESS_STR,
              __func__, MAC_ADDR_ARRAY(macBSSID));

    wdiStatus = WDI_QueueAssocRequest( pWDICtx, pBSSSes, pEventData);
    wpalMutexRelease(&pWDICtx->wptMutex);
    return wdiStatus;
  }

  wpalMutexRelease(&pWDICtx->wptMutex);
  /*                                                                       
                      
                                                                         */
  if (( WDI_STATUS_SUCCESS != WDI_GetMessageBuffer( pWDICtx, WDI_DEL_STA_REQ,
                        sizeof(halDelStaReqMsg.delStaParams),
                        &pSendBuffer, &usDataOffset, &usSendSize))||
      ( usSendSize < (usDataOffset + sizeof(halDelStaReqMsg.delStaParams) )))
  {
     WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_WARN,
              "Unable to get send buffer in start req %x %x %x",
                pEventData, pwdiDelSTAParams, wdiDelSTARspCb);
     WDI_ASSERT(0);
     return WDI_STATUS_E_FAILURE;
  }

  halDelStaReqMsg.delStaParams.staIdx = pwdiDelSTAParams->ucSTAIdx;
  wpalMemoryCopy( pSendBuffer+usDataOffset,
                  &halDelStaReqMsg.delStaParams,
                  sizeof(halDelStaReqMsg.delStaParams));

  pWDICtx->wdiReqStatusCB     = pwdiDelSTAParams->wdiReqStatusCB;
  pWDICtx->pReqStatusUserData = pwdiDelSTAParams->pUserData;

  /*                                                                         
                               
                                                                           */
  return  WDI_SendMsg( pWDICtx, pSendBuffer, usSendSize,
                       wdiDelSTARspCb, pEventData->pUserData, WDI_DEL_STA_RESP);

}/*                    */


/*                                                                          
                                                
                                                                          */
/* 
                                                                  
                  

                                                         
                                                                    

     
                                    
*/
WDI_Status
WDI_ProcessSetBssKeyReq
(
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
)
{
  WDI_SetBSSKeyReqParamsType*  pwdiSetBSSKeyParams;
  WDI_SetBSSKeyRspCb           wdiSetBSSKeyRspCb;
  wpt_uint8                    ucCurrentBSSSesIdx  = 0;
  WDI_BSSSessionType*          pBSSSes             = NULL;
  wpt_uint8*                   pSendBuffer         = NULL;
  wpt_uint16                   usDataOffset        = 0;
  wpt_uint16                   usSendSize          = 0;
  WDI_Status                   wdiStatus           = WDI_STATUS_SUCCESS;
  tSetBssKeyReqMsg             halSetBssKeyReqMsg  = {{0}};
  wpt_uint8                    keyIndex            = 0;

  /*                                                                        */

  /*                                                                         
                
                                                                           */
  if (( NULL == pEventData ) || ( NULL == pEventData->pEventData ) ||
      ( NULL == pEventData->pCBfnc ))
  {
     WPAL_TRACE( eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_WARN,
                 "%s: Invalid parameters", __FUNCTION__);
     WDI_ASSERT(0);
     return WDI_STATUS_E_FAILURE;
  }

  pwdiSetBSSKeyParams = (WDI_SetBSSKeyReqParamsType*)pEventData->pEventData;
  wdiSetBSSKeyRspCb   = (WDI_SetBSSKeyRspCb)pEventData->pCBfnc;
  /*                                                                         
                                                                           
                                              
                                                                           */
  wpalMutexAcquire(&pWDICtx->wptMutex);

  /*                                                                        
                                              
                                                                          */
  ucCurrentBSSSesIdx = WDI_FindAssocSessionByBSSIdx( pWDICtx,
                           pwdiSetBSSKeyParams->wdiBSSKeyInfo.ucBssIdx,
                          &pBSSSes);

  if ( NULL == pBSSSes )
  {
    WPAL_TRACE( eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_ERROR,
              "%s: Association sequence for this BSS does not yet exist. ucBssIdx %d",
              __func__, pwdiSetBSSKeyParams->wdiBSSKeyInfo.ucBssIdx);

    wpalMutexRelease(&pWDICtx->wptMutex);
    return WDI_STATUS_E_NOT_ALLOWED;
  }

  /*                                                                        
                                                             
                                             
                                                                          */
  if ( eWLAN_PAL_TRUE == pBSSSes->bAssocReqQueued )
  {
    WPAL_TRACE( eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_WARN,
              "%s: Association sequence for this BSS exists but currently queued. ucBssIdx %d",
              __func__, pwdiSetBSSKeyParams->wdiBSSKeyInfo.ucBssIdx);

    wdiStatus = WDI_QueueAssocRequest( pWDICtx, pBSSSes, pEventData);
    wpalMutexRelease(&pWDICtx->wptMutex);
    return wdiStatus;
  }


  wpalMutexRelease(&pWDICtx->wptMutex);
  /*                                                                       
                      
                                                                         */
  if (( WDI_STATUS_SUCCESS != WDI_GetMessageBuffer( pWDICtx, WDI_SET_BSS_KEY_REQ,
                        sizeof(halSetBssKeyReqMsg.setBssKeyParams),
                        &pSendBuffer, &usDataOffset, &usSendSize))||
      ( usSendSize < (usDataOffset + sizeof(halSetBssKeyReqMsg.setBssKeyParams) )))
  {
     WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_WARN,
              "Unable to get send buffer in set bss key req %x %x %x",
                pEventData, pwdiSetBSSKeyParams, wdiSetBSSKeyRspCb);
     WDI_ASSERT(0);
     return WDI_STATUS_E_FAILURE;
  }

  /*                                                                       
                                                
                                                                         */

  halSetBssKeyReqMsg.setBssKeyParams.bssIdx = ucCurrentBSSSesIdx;

  halSetBssKeyReqMsg.setBssKeyParams.encType =
             WDI_2_HAL_ENC_TYPE (pwdiSetBSSKeyParams->wdiBSSKeyInfo.wdiEncType);

  halSetBssKeyReqMsg.setBssKeyParams.numKeys =
                                  pwdiSetBSSKeyParams->wdiBSSKeyInfo.ucNumKeys;

  for(keyIndex = 0; keyIndex < pwdiSetBSSKeyParams->wdiBSSKeyInfo.ucNumKeys ;
                                                                 keyIndex++)
  {
    halSetBssKeyReqMsg.setBssKeyParams.key[keyIndex].keyId =
                      pwdiSetBSSKeyParams->wdiBSSKeyInfo.aKeys[keyIndex].keyId;
    halSetBssKeyReqMsg.setBssKeyParams.key[keyIndex].unicast =
                     pwdiSetBSSKeyParams->wdiBSSKeyInfo.aKeys[keyIndex].unicast;
    halSetBssKeyReqMsg.setBssKeyParams.key[keyIndex].keyDirection =
                pwdiSetBSSKeyParams->wdiBSSKeyInfo.aKeys[keyIndex].keyDirection;
    wpalMemoryCopy(halSetBssKeyReqMsg.setBssKeyParams.key[keyIndex].keyRsc,
                     pwdiSetBSSKeyParams->wdiBSSKeyInfo.aKeys[keyIndex].keyRsc,
                     WDI_MAX_KEY_RSC_LEN);
    halSetBssKeyReqMsg.setBssKeyParams.key[keyIndex].paeRole =
                     pwdiSetBSSKeyParams->wdiBSSKeyInfo.aKeys[keyIndex].paeRole;
    halSetBssKeyReqMsg.setBssKeyParams.key[keyIndex].keyLength =
                   pwdiSetBSSKeyParams->wdiBSSKeyInfo.aKeys[keyIndex].keyLength;
    wpalMemoryCopy(halSetBssKeyReqMsg.setBssKeyParams.key[keyIndex].key,
                         pwdiSetBSSKeyParams->wdiBSSKeyInfo.aKeys[keyIndex].key,
                        WDI_MAX_KEY_LENGTH);
   }

  wpalMemoryCopy( pSendBuffer+usDataOffset,
                    &halSetBssKeyReqMsg.setBssKeyParams,
                    sizeof(halSetBssKeyReqMsg.setBssKeyParams));

  pWDICtx->wdiReqStatusCB     = pwdiSetBSSKeyParams->wdiReqStatusCB;
  pWDICtx->pReqStatusUserData = pwdiSetBSSKeyParams->pUserData;

  /*                                                                         
                                   
                                                                           */
  return  WDI_SendMsg( pWDICtx, pSendBuffer, usSendSize,
                       wdiSetBSSKeyRspCb, pEventData->pUserData,
                       WDI_SET_BSS_KEY_RESP);

}/*                       */

/* 
                                                                 
                      

                                                         
                                                                    

     
                                    
*/
WDI_Status
WDI_ProcessRemoveBssKeyReq
(
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
)
{
  WDI_RemoveBSSKeyReqParamsType*  pwdiRemoveBSSKeyParams;
  WDI_RemoveBSSKeyRspCb           wdiRemoveBSSKeyRspCb;
  wpt_uint8                       ucCurrentBSSSesIdx     = 0;
  WDI_BSSSessionType*             pBSSSes                = NULL;
  wpt_uint8*                      pSendBuffer            = NULL;
  wpt_uint16                      usDataOffset           = 0;
  wpt_uint16                      usSendSize             = 0;
  WDI_Status                      wdiStatus              = WDI_STATUS_SUCCESS;
  tRemoveBssKeyReqMsg             halRemoveBssKeyReqMsg  = {{0}};
  /*                                                                        */

  /*                                                                         
                
                                                                           */
  if (( NULL == pEventData ) || ( NULL == pEventData->pEventData ) ||
      ( NULL == pEventData->pCBfnc ))
  {
     WPAL_TRACE( eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_WARN,
                 "%s: Invalid parameters", __FUNCTION__);
     WDI_ASSERT(0);
     return WDI_STATUS_E_FAILURE;
  }

  pwdiRemoveBSSKeyParams = (WDI_RemoveBSSKeyReqParamsType*)pEventData->pEventData;
  wdiRemoveBSSKeyRspCb   = (WDI_RemoveBSSKeyRspCb)pEventData->pCBfnc;
  /*                                                                         
                                                                           
                                              
                                                                           */
  wpalMutexAcquire(&pWDICtx->wptMutex);

  /*                                                                        
                                              
                                                                          */
  ucCurrentBSSSesIdx = WDI_FindAssocSessionByBSSIdx( pWDICtx,
                           pwdiRemoveBSSKeyParams->wdiKeyInfo.ucBssIdx,
                          &pBSSSes);

  if ( NULL == pBSSSes )
  {
    WPAL_TRACE( eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_ERROR,
              "%s: Association sequence for this BSS does not yet exist. ucBssIdx %d",
              __func__, pwdiRemoveBSSKeyParams->wdiKeyInfo.ucBssIdx);

    wpalMutexRelease(&pWDICtx->wptMutex);
    return WDI_STATUS_E_NOT_ALLOWED;
  }

  /*                                                                        
                                                             
                                             
                                                                          */
  if ( eWLAN_PAL_TRUE == pBSSSes->bAssocReqQueued )
  {
    WPAL_TRACE( eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_WARN,
              "%s: Association sequence for this BSS exists but currently queued. ucBssIdx %d",
              __func__, pwdiRemoveBSSKeyParams->wdiKeyInfo.ucBssIdx);

    wdiStatus = WDI_QueueAssocRequest( pWDICtx, pBSSSes, pEventData);
    wpalMutexRelease(&pWDICtx->wptMutex);
    return wdiStatus;
  }


  wpalMutexRelease(&pWDICtx->wptMutex);

  /*                                                                       
                      
                                                                         */
  if (( WDI_STATUS_SUCCESS != WDI_GetMessageBuffer( pWDICtx, WDI_RMV_BSS_KEY_REQ,
                        sizeof(halRemoveBssKeyReqMsg.removeBssKeyParams),
                        &pSendBuffer, &usDataOffset, &usSendSize))||
      ( usSendSize < (usDataOffset + sizeof(halRemoveBssKeyReqMsg.removeBssKeyParams) )))
  {
     WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_WARN,
              "Unable to get send buffer in set bss key req %x %x %x",
                pEventData, pwdiRemoveBSSKeyParams, wdiRemoveBSSKeyRspCb);
     WDI_ASSERT(0);
     return WDI_STATUS_E_FAILURE;
  }
  /*                                                                       
                                                
                                                                         */
  halRemoveBssKeyReqMsg.removeBssKeyParams.bssIdx = ucCurrentBSSSesIdx;

  halRemoveBssKeyReqMsg.removeBssKeyParams.encType =
      WDI_2_HAL_ENC_TYPE (pwdiRemoveBSSKeyParams->wdiKeyInfo.wdiEncType);

  halRemoveBssKeyReqMsg.removeBssKeyParams.keyId = pwdiRemoveBSSKeyParams->wdiKeyInfo.ucKeyId;

  halRemoveBssKeyReqMsg.removeBssKeyParams.wepType =
      WDI_2_HAL_WEP_TYPE(pwdiRemoveBSSKeyParams->wdiKeyInfo.wdiWEPType);

  wpalMemoryCopy( pSendBuffer+usDataOffset,
                    &halRemoveBssKeyReqMsg.removeBssKeyParams,
                    sizeof(halRemoveBssKeyReqMsg.removeBssKeyParams));

  pWDICtx->wdiReqStatusCB     = pwdiRemoveBSSKeyParams->wdiReqStatusCB;
  pWDICtx->pReqStatusUserData = pwdiRemoveBSSKeyParams->pUserData;

  /*                                                                         
                                      
                                                                           */
  return  WDI_SendMsg( pWDICtx, pSendBuffer, usSendSize,
                       wdiRemoveBSSKeyRspCb, pEventData->pUserData,
                       WDI_RMV_BSS_KEY_RESP);
}/*                          */

/* 
                                                                 
                  

                                                         
                                                                    

     
                                    
*/
WDI_Status
WDI_ProcessSetStaKeyReq
(
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
)
{
  WDI_SetSTAKeyReqParamsType*  pwdiSetSTAKeyParams;
  WDI_SetSTAKeyRspCb           wdiSetSTAKeyRspCb;
  WDI_BSSSessionType*          pBSSSes             = NULL;
  wpt_uint8*                   pSendBuffer         = NULL;
  wpt_uint16                   usDataOffset        = 0;
  wpt_uint16                   usSendSize          = 0;
  WDI_Status                   wdiStatus           = WDI_STATUS_SUCCESS;
  wpt_macAddr                  macBSSID;
  wpt_uint8                    ucCurrentBSSSesIdx;
  tSetStaKeyReqMsg             halSetStaKeyReqMsg  = {{0}};
  wpt_uint8                    keyIndex            = 0;

  /*                                                                        */

  /*                                                                         
                
                                                                           */
  if (( NULL == pEventData ) || ( NULL == pEventData->pEventData ) ||
      ( NULL == pEventData->pCBfnc ))
  {
     WPAL_TRACE( eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_WARN,
                 "%s: Invalid parameters", __FUNCTION__);
     WDI_ASSERT(0);
     return WDI_STATUS_E_FAILURE;
  }

   pwdiSetSTAKeyParams = (WDI_SetSTAKeyReqParamsType*)pEventData->pEventData;
   wdiSetSTAKeyRspCb   = (WDI_SetSTAKeyRspCb)pEventData->pCBfnc;
  /*                                                                         
                                                                           
                                              
                                                                           */
  wpalMutexAcquire(&pWDICtx->wptMutex);

  /*                                                                        
                                                                       
                                                                          */
  if ( WDI_STATUS_SUCCESS != WDI_STATableGetStaBSSIDAddr(pWDICtx,
                                  pwdiSetSTAKeyParams->wdiKeyInfo.ucSTAIdx,
                                  &macBSSID))
  {
    WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_WARN,
             "This station does not exist in the WDI Station Table %d");
    wpalMutexRelease(&pWDICtx->wptMutex);
    return WDI_STATUS_E_FAILURE;
  }

  ucCurrentBSSSesIdx = WDI_FindAssocSession( pWDICtx, macBSSID, &pBSSSes);
  if ( NULL == pBSSSes )
  {
    WPAL_TRACE( eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_ERROR,
              "%s: Association sequence for this BSS does not yet exist. macBSSID " MAC_ADDRESS_STR,
              __func__, MAC_ADDR_ARRAY(macBSSID));

    wpalMutexRelease(&pWDICtx->wptMutex);
    return WDI_STATUS_E_NOT_ALLOWED;
  }

  /*                                                                        
                                                             
                                             
                                                                          */
  if ( eWLAN_PAL_TRUE == pBSSSes->bAssocReqQueued )
  {
    WPAL_TRACE( eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_WARN,
              "%s: Association sequence for this BSS exists but currently queued. macBSSID " MAC_ADDRESS_STR,
              __func__, MAC_ADDR_ARRAY(macBSSID));

    wdiStatus = WDI_QueueAssocRequest( pWDICtx, pBSSSes, pEventData);
    wpalMutexRelease(&pWDICtx->wptMutex);
    return wdiStatus;
  }


  wpalMutexRelease(&pWDICtx->wptMutex);
  /*                                                                       
                      
                                                                         */
  if (( WDI_STATUS_SUCCESS != WDI_GetMessageBuffer( pWDICtx, WDI_SET_STA_KEY_REQ,
                        sizeof(halSetStaKeyReqMsg.setStaKeyParams),
                        &pSendBuffer, &usDataOffset, &usSendSize))||
      ( usSendSize < (usDataOffset + sizeof(halSetStaKeyReqMsg.setStaKeyParams) )))
  {
     WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_WARN,
              "Unable to get send buffer in set bss key req %x %x %x",
                pEventData, pwdiSetSTAKeyParams, wdiSetSTAKeyRspCb);
     WDI_ASSERT(0);
     return WDI_STATUS_E_FAILURE;
  }
  /*                                                                       
                                                    
                                                                         */
  halSetStaKeyReqMsg.setStaKeyParams.encType =
      WDI_2_HAL_ENC_TYPE (pwdiSetSTAKeyParams->wdiKeyInfo.wdiEncType);

  halSetStaKeyReqMsg.setStaKeyParams.wepType =
      WDI_2_HAL_WEP_TYPE (pwdiSetSTAKeyParams->wdiKeyInfo.wdiWEPType );

  halSetStaKeyReqMsg.setStaKeyParams.staIdx = pwdiSetSTAKeyParams->wdiKeyInfo.ucSTAIdx;

  halSetStaKeyReqMsg.setStaKeyParams.defWEPIdx = pwdiSetSTAKeyParams->wdiKeyInfo.ucDefWEPIdx;

  halSetStaKeyReqMsg.setStaKeyParams.singleTidRc = pwdiSetSTAKeyParams->wdiKeyInfo.ucSingleTidRc;

#ifdef WLAN_SOFTAP_FEATURE
  for(keyIndex = 0; keyIndex < pwdiSetSTAKeyParams->wdiKeyInfo.ucNumKeys ;
                                                                 keyIndex++)
  {
    halSetStaKeyReqMsg.setStaKeyParams.key[keyIndex].keyId =
                      pwdiSetSTAKeyParams->wdiKeyInfo.wdiKey[keyIndex].keyId;
    halSetStaKeyReqMsg.setStaKeyParams.key[keyIndex].unicast =
                     pwdiSetSTAKeyParams->wdiKeyInfo.wdiKey[keyIndex].unicast;
    halSetStaKeyReqMsg.setStaKeyParams.key[keyIndex].keyDirection =
                pwdiSetSTAKeyParams->wdiKeyInfo.wdiKey[keyIndex].keyDirection;
    wpalMemoryCopy(halSetStaKeyReqMsg.setStaKeyParams.key[keyIndex].keyRsc,
                     pwdiSetSTAKeyParams->wdiKeyInfo.wdiKey[keyIndex].keyRsc,
                     WDI_MAX_KEY_RSC_LEN);
    halSetStaKeyReqMsg.setStaKeyParams.key[keyIndex].paeRole =
                     pwdiSetSTAKeyParams->wdiKeyInfo.wdiKey[keyIndex].paeRole;
    halSetStaKeyReqMsg.setStaKeyParams.key[keyIndex].keyLength =
                   pwdiSetSTAKeyParams->wdiKeyInfo.wdiKey[keyIndex].keyLength;
    wpalMemoryCopy(halSetStaKeyReqMsg.setStaKeyParams.key[keyIndex].key,
                         pwdiSetSTAKeyParams->wdiKeyInfo.wdiKey[keyIndex].key,
                        WDI_MAX_KEY_LENGTH);
   }
#else
  halSetStaKeyReqMsg.setStaKeyParams.key.keyId =
                      pwdiSetSTAKeyParams->wdiKeyInfo.wdiKey[0].keyId;
  halSetStaKeyReqMsg.setStaKeyParams.key.unicast =
                     pwdiSetSTAKeyParams->wdiKeyInfo.wdiKey[0].unicast;
  halSetStaKeyReqMsg.setStaKeyParams.key.keyDirection =
                pwdiSetSTAKeyParams->wdiKeyInfo.wdiKey[0].keyDirection;
  wpalMemoryCopy(halSetStaKeyReqMsg.setStaKeyParams.key.keyRsc,
                     pwdiSetSTAKeyParams->wdiKeyInfo.wdiKey[0].keyRsc,
                     WDI_MAX_KEY_RSC_LEN);
  halSetStaKeyReqMsg.setStaKeyParams.key.paeRole =
                     pwdiSetSTAKeyParams->wdiKeyInfo.wdiKey[0].paeRole;
  halSetStaKeyReqMsg.setStaKeyParams.key.keyLength =
                   pwdiSetSTAKeyParams->wdiKeyInfo.wdiKey[0].keyLength;
  wpalMemoryCopy(halSetStaKeyReqMsg.setStaKeyParams.key.key,
                         pwdiSetSTAKeyParams->wdiKeyInfo.wdiKey[0].key,
                        WDI_MAX_KEY_LENGTH);
#endif

  wpalMemoryCopy( pSendBuffer+usDataOffset,
                    &halSetStaKeyReqMsg.setStaKeyParams,
                    sizeof(halSetStaKeyReqMsg.setStaKeyParams));

  pWDICtx->wdiReqStatusCB     = pwdiSetSTAKeyParams->wdiReqStatusCB;
  pWDICtx->pReqStatusUserData = pwdiSetSTAKeyParams->pUserData;

  /*                                                                         
                                   
                                                                           */
  return  WDI_SendMsg( pWDICtx, pSendBuffer, usSendSize,
                       wdiSetSTAKeyRspCb, pEventData->pUserData,
                       WDI_SET_STA_KEY_RESP);

}/*                       */

/* 
                                                            
                           

                                                         
                                                                    

     
                                    
*/
WDI_Status
WDI_ProcessRemoveStaKeyReq
(
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
)
{
  WDI_RemoveSTAKeyReqParamsType*  pwdiRemoveSTAKeyParams;
  WDI_RemoveSTAKeyRspCb           wdiRemoveSTAKeyRspCb;
  WDI_BSSSessionType*             pBSSSes                = NULL;
  wpt_uint8*                      pSendBuffer            = NULL;
  wpt_uint16                      usDataOffset           = 0;
  wpt_uint16                      usSendSize             = 0;
  WDI_Status                      wdiStatus              = WDI_STATUS_SUCCESS;
  wpt_macAddr                     macBSSID;
  wpt_uint8                       ucCurrentBSSSesIdx;
  tRemoveStaKeyReqMsg             halRemoveStaKeyReqMsg  = {{0}};
  /*                                                                        */

  /*                                                                         
                
                                                                           */
 if (( NULL == pEventData ) || ( NULL == pEventData->pEventData ) ||
      ( NULL == pEventData->pCBfnc ))
  {
     WPAL_TRACE( eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_WARN,
                 "%s: Invalid parameters", __FUNCTION__);
     WDI_ASSERT(0);
     return WDI_STATUS_E_FAILURE;
  }

  pwdiRemoveSTAKeyParams = (WDI_RemoveSTAKeyReqParamsType*)pEventData->pEventData;
  wdiRemoveSTAKeyRspCb   = (WDI_RemoveSTAKeyRspCb)pEventData->pCBfnc;
  /*                                                                         
                                                                           
                                              
                                                                           */
  wpalMutexAcquire(&pWDICtx->wptMutex);

  /*                                                                        
                                                                       
                                                                          */
  if ( WDI_STATUS_SUCCESS != WDI_STATableGetStaBSSIDAddr(pWDICtx,
                             pwdiRemoveSTAKeyParams->wdiKeyInfo.ucSTAIdx,
                             &macBSSID))
  {
    WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_WARN,
             "This station does not exist in the WDI Station Table %d");
    wpalMutexRelease(&pWDICtx->wptMutex);
    return WDI_STATUS_E_FAILURE;
  }

  ucCurrentBSSSesIdx = WDI_FindAssocSession( pWDICtx, macBSSID, &pBSSSes);
  if ( NULL == pBSSSes )
  {
    WPAL_TRACE( eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_ERROR,
              "%s: Association sequence for this BSS does not yet exist. macBSSID " MAC_ADDRESS_STR,
              __func__, MAC_ADDR_ARRAY(macBSSID));

    wpalMutexRelease(&pWDICtx->wptMutex);
    return WDI_STATUS_E_NOT_ALLOWED;
  }

  /*                                                                        
                                                             
                                             
                                                                          */
  if ( eWLAN_PAL_TRUE == pBSSSes->bAssocReqQueued )
  {
    WPAL_TRACE( eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_WARN,
              "%s: Association sequence for this BSS exists but currently queued. macBSSID " MAC_ADDRESS_STR,
              __func__, MAC_ADDR_ARRAY(macBSSID));

    wdiStatus = WDI_QueueAssocRequest( pWDICtx, pBSSSes, pEventData);
    wpalMutexRelease(&pWDICtx->wptMutex);
    return wdiStatus;
  }



  wpalMutexRelease(&pWDICtx->wptMutex);
  /*                                                                       
                      
                                                                         */
  if (( WDI_STATUS_SUCCESS != WDI_GetMessageBuffer( pWDICtx, WDI_RMV_STA_KEY_REQ,
                        sizeof(halRemoveStaKeyReqMsg.removeStaKeyParams),
                        &pSendBuffer, &usDataOffset, &usSendSize))||
      ( usSendSize < (usDataOffset + sizeof(halRemoveStaKeyReqMsg.removeStaKeyParams) )))
  {
     WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_WARN,
              "Unable to get send buffer in set bss key req %x %x %x",
                pEventData, pwdiRemoveSTAKeyParams, wdiRemoveSTAKeyRspCb);
     WDI_ASSERT(0);
     return WDI_STATUS_E_FAILURE;
  }

  /*                                                                       
                                                
                                                                         */

  halRemoveStaKeyReqMsg.removeStaKeyParams.staIdx =
      pwdiRemoveSTAKeyParams->wdiKeyInfo.ucSTAIdx;

  halRemoveStaKeyReqMsg.removeStaKeyParams.encType =
      WDI_2_HAL_ENC_TYPE (pwdiRemoveSTAKeyParams->wdiKeyInfo.wdiEncType);

  halRemoveStaKeyReqMsg.removeStaKeyParams.keyId =
      pwdiRemoveSTAKeyParams->wdiKeyInfo.ucKeyId;

  halRemoveStaKeyReqMsg.removeStaKeyParams.unicast =
      pwdiRemoveSTAKeyParams->wdiKeyInfo.ucUnicast;

  wpalMemoryCopy( pSendBuffer+usDataOffset,
                    &halRemoveStaKeyReqMsg.removeStaKeyParams,
                    sizeof(halRemoveStaKeyReqMsg.removeStaKeyParams));

  pWDICtx->wdiReqStatusCB     = pwdiRemoveSTAKeyParams->wdiReqStatusCB;
  pWDICtx->pReqStatusUserData = pwdiRemoveSTAKeyParams->pUserData;

  /*                                                                         
                                      
                                                                           */
  return  WDI_SendMsg( pWDICtx, pSendBuffer, usSendSize,
                       wdiRemoveSTAKeyRspCb, pEventData->pUserData,
                       WDI_RMV_STA_KEY_RESP);

}/*                          */

/* 
                                                                 
                  

                                                         
                                                                    

     
                                    
*/
WDI_Status
WDI_ProcessSetStaBcastKeyReq
(
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
)
{
  WDI_SetSTAKeyReqParamsType*  pwdiSetSTAKeyParams;
  WDI_SetSTAKeyRspCb           wdiSetSTAKeyRspCb;
  WDI_BSSSessionType*          pBSSSes             = NULL;
  wpt_uint8*                   pSendBuffer         = NULL;
  wpt_uint16                   usDataOffset        = 0;
  wpt_uint16                   usSendSize          = 0;
  WDI_Status                   wdiStatus           = WDI_STATUS_SUCCESS;
  wpt_macAddr                  macBSSID;
  wpt_uint8                    ucCurrentBSSSesIdx;
  tSetStaKeyReqMsg             halSetStaKeyReqMsg  = {{0}};
  wpt_uint8                    keyIndex            = 0;

  /*                                                                        */

  /*                                                                         
                
                                                                           */
  if (( NULL == pEventData ) || ( NULL == pEventData->pEventData ) ||
      ( NULL == pEventData->pCBfnc ))
  {
     WPAL_TRACE( eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_WARN,
                 "%s: Invalid parameters", __FUNCTION__);
     WDI_ASSERT(0);
     return WDI_STATUS_E_FAILURE;
  }

   pwdiSetSTAKeyParams = (WDI_SetSTAKeyReqParamsType*)pEventData->pEventData;
   wdiSetSTAKeyRspCb   = (WDI_SetSTAKeyRspCb)pEventData->pCBfnc;
  /*                                                                         
                                                                           
                                              
                                                                           */
  wpalMutexAcquire(&pWDICtx->wptMutex);

  /*                                                                        
                                                                       
                                                                          */
  if ( WDI_STATUS_SUCCESS != WDI_STATableGetStaBSSIDAddr(pWDICtx,
                                  pwdiSetSTAKeyParams->wdiKeyInfo.ucSTAIdx,
                                  &macBSSID))
  {
    WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_WARN,
             "This station does not exist in the WDI Station Table %d");
    wpalMutexRelease(&pWDICtx->wptMutex);
    return WDI_STATUS_E_FAILURE;
  }

  ucCurrentBSSSesIdx = WDI_FindAssocSession( pWDICtx, macBSSID, &pBSSSes);
  if ( NULL == pBSSSes )
  {
    WPAL_TRACE( eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_ERROR,
              "Association sequence for this BSS does not yet exist. macBSSID " MAC_ADDRESS_STR,
              __func__, MAC_ADDR_ARRAY(macBSSID));

    wpalMutexRelease(&pWDICtx->wptMutex);
    return WDI_STATUS_E_NOT_ALLOWED;
  }

  /*                                                                        
                                                             
                                             
                                                                          */
  if ( eWLAN_PAL_TRUE == pBSSSes->bAssocReqQueued )
  {
    WPAL_TRACE( eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_WARN,
              "%s: Association sequence for this BSS exists but currently queued. macBSSID " MAC_ADDRESS_STR,
              __func__, MAC_ADDR_ARRAY(macBSSID));

    wdiStatus = WDI_QueueAssocRequest( pWDICtx, pBSSSes, pEventData);
    wpalMutexRelease(&pWDICtx->wptMutex);
    return wdiStatus;
  }


  wpalMutexRelease(&pWDICtx->wptMutex);
  /*                                                                       
                      
                                                                         */
  if (( WDI_STATUS_SUCCESS != WDI_GetMessageBuffer( pWDICtx, WDI_SET_STA_KEY_REQ,
                        sizeof(halSetStaKeyReqMsg.setStaKeyParams),
                        &pSendBuffer, &usDataOffset, &usSendSize))||
      ( usSendSize < (usDataOffset + sizeof(halSetStaKeyReqMsg.setStaKeyParams) )))
  {
     WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_WARN,
              "Unable to get send buffer in set bss key req %x %x %x",
                pEventData, pwdiSetSTAKeyParams, wdiSetSTAKeyRspCb);
     WDI_ASSERT(0);
     return WDI_STATUS_E_FAILURE;
  }
  /*                                                                       
                                                    
                                                                         */
  halSetStaKeyReqMsg.setStaKeyParams.encType =
      WDI_2_HAL_ENC_TYPE (pwdiSetSTAKeyParams->wdiKeyInfo.wdiEncType);

  halSetStaKeyReqMsg.setStaKeyParams.wepType =
      WDI_2_HAL_WEP_TYPE (pwdiSetSTAKeyParams->wdiKeyInfo.wdiWEPType );

  halSetStaKeyReqMsg.setStaKeyParams.staIdx = pwdiSetSTAKeyParams->wdiKeyInfo.ucSTAIdx;

  halSetStaKeyReqMsg.setStaKeyParams.defWEPIdx = pwdiSetSTAKeyParams->wdiKeyInfo.ucDefWEPIdx;

  halSetStaKeyReqMsg.setStaKeyParams.singleTidRc = pwdiSetSTAKeyParams->wdiKeyInfo.ucSingleTidRc;

#ifdef WLAN_SOFTAP_FEATURE
  for(keyIndex = 0; keyIndex < pwdiSetSTAKeyParams->wdiKeyInfo.ucNumKeys ;
                                                                 keyIndex++)
  {
    halSetStaKeyReqMsg.setStaKeyParams.key[keyIndex].keyId =
                      pwdiSetSTAKeyParams->wdiKeyInfo.wdiKey[keyIndex].keyId;
    halSetStaKeyReqMsg.setStaKeyParams.key[keyIndex].unicast =
                     pwdiSetSTAKeyParams->wdiKeyInfo.wdiKey[keyIndex].unicast;
    halSetStaKeyReqMsg.setStaKeyParams.key[keyIndex].keyDirection =
                pwdiSetSTAKeyParams->wdiKeyInfo.wdiKey[keyIndex].keyDirection;
    wpalMemoryCopy(halSetStaKeyReqMsg.setStaKeyParams.key[keyIndex].keyRsc,
                     pwdiSetSTAKeyParams->wdiKeyInfo.wdiKey[keyIndex].keyRsc,
                     WDI_MAX_KEY_RSC_LEN);
    halSetStaKeyReqMsg.setStaKeyParams.key[keyIndex].paeRole =
                     pwdiSetSTAKeyParams->wdiKeyInfo.wdiKey[keyIndex].paeRole;
    halSetStaKeyReqMsg.setStaKeyParams.key[keyIndex].keyLength =
                   pwdiSetSTAKeyParams->wdiKeyInfo.wdiKey[keyIndex].keyLength;
    wpalMemoryCopy(halSetStaKeyReqMsg.setStaKeyParams.key[keyIndex].key,
                         pwdiSetSTAKeyParams->wdiKeyInfo.wdiKey[keyIndex].key,
                        WDI_MAX_KEY_LENGTH);
   }
#else
  halSetStaKeyReqMsg.setStaKeyParams.key.keyId =
                      pwdiSetSTAKeyParams->wdiKeyInfo.wdiKey[0].keyId;
  halSetStaKeyReqMsg.setStaKeyParams.key.unicast =
                     pwdiSetSTAKeyParams->wdiKeyInfo.wdiKey[0].unicast;
  halSetStaKeyReqMsg.setStaKeyParams.key.keyDirection =
                pwdiSetSTAKeyParams->wdiKeyInfo.wdiKey[0].keyDirection;
  wpalMemoryCopy(halSetStaKeyReqMsg.setStaKeyParams.key.keyRsc,
                     pwdiSetSTAKeyParams->wdiKeyInfo.wdiKey[0].keyRsc,
                     WDI_MAX_KEY_RSC_LEN);
  halSetStaKeyReqMsg.setStaKeyParams.key.paeRole =
                     pwdiSetSTAKeyParams->wdiKeyInfo.wdiKey[0].paeRole;
  halSetStaKeyReqMsg.setStaKeyParams.key.keyLength =
                   pwdiSetSTAKeyParams->wdiKeyInfo.wdiKey[0].keyLength;
  wpalMemoryCopy(halSetStaKeyReqMsg.setStaKeyParams.key.key,
                         pwdiSetSTAKeyParams->wdiKeyInfo.wdiKey[0].key,
                        WDI_MAX_KEY_LENGTH);
#endif

  wpalMemoryCopy( pSendBuffer+usDataOffset,
                    &halSetStaKeyReqMsg.setStaKeyParams,
                    sizeof(halSetStaKeyReqMsg.setStaKeyParams));

  pWDICtx->wdiReqStatusCB     = pwdiSetSTAKeyParams->wdiReqStatusCB;
  pWDICtx->pReqStatusUserData = pwdiSetSTAKeyParams->pUserData;

  /*                                                                         
                                   
                                                                           */
  return  WDI_SendMsg( pWDICtx, pSendBuffer, usSendSize,
                       wdiSetSTAKeyRspCb, pEventData->pUserData,
                       WDI_SET_STA_KEY_RESP);

}/*                            */

/* 
                                                            
                           

                                                         
                                                                    

     
                                    
*/
WDI_Status
WDI_ProcessRemoveStaBcastKeyReq
(
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
)
{
  WDI_RemoveSTAKeyReqParamsType*  pwdiRemoveSTABcastKeyParams;
  WDI_RemoveSTAKeyRspCb           wdiRemoveSTAKeyRspCb;
  WDI_BSSSessionType*             pBSSSes                = NULL;
  wpt_uint8*                      pSendBuffer            = NULL;
  wpt_uint16                      usDataOffset           = 0;
  wpt_uint16                      usSendSize             = 0;
  WDI_Status                      wdiStatus              = WDI_STATUS_SUCCESS;
  wpt_macAddr                     macBSSID;
  wpt_uint8                       ucCurrentBSSSesIdx;
  tRemoveStaKeyReqMsg             halRemoveStaBcastKeyReqMsg = {{0}};
  /*                                                                        */

  /*                                                                         
                
                                                                           */
 if (( NULL == pEventData ) || ( NULL == pEventData->pEventData ) ||
      ( NULL == pEventData->pCBfnc ))
  {
     WPAL_TRACE( eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_WARN,
                 "%s: Invalid parameters", __FUNCTION__);
     WDI_ASSERT(0);
     return WDI_STATUS_E_FAILURE;
  }

  pwdiRemoveSTABcastKeyParams = (WDI_RemoveSTAKeyReqParamsType*)pEventData->pEventData;
  wdiRemoveSTAKeyRspCb   = (WDI_RemoveSTAKeyRspCb)pEventData->pCBfnc;
  /*                                                                         
                                                                           
                                              
                                                                           */
  wpalMutexAcquire(&pWDICtx->wptMutex);

  /*                                                                        
                                                                       
                                                                          */
  if ( WDI_STATUS_SUCCESS != WDI_STATableGetStaBSSIDAddr(pWDICtx,
                             pwdiRemoveSTABcastKeyParams->wdiKeyInfo.ucSTAIdx,
                             &macBSSID))
  {
    WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_WARN,
             "This station does not exist in the WDI Station Table %d");
    wpalMutexRelease(&pWDICtx->wptMutex);
    return WDI_STATUS_E_FAILURE;
  }

  ucCurrentBSSSesIdx = WDI_FindAssocSession( pWDICtx, macBSSID, &pBSSSes);
  if ( NULL == pBSSSes )
  {
    WPAL_TRACE( eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_ERROR,
              "%s: Association sequence for this BSS does not yet exist. macBSSID " MAC_ADDRESS_STR,
              __func__, MAC_ADDR_ARRAY(macBSSID));

    wpalMutexRelease(&pWDICtx->wptMutex);
    return WDI_STATUS_E_NOT_ALLOWED;
  }

  /*                                                                        
                                                             
                                             
                                                                          */
  if ( eWLAN_PAL_TRUE == pBSSSes->bAssocReqQueued )
  {
    WPAL_TRACE( eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_WARN,
              "%s: Association sequence for this BSS exists but currently queued. macBSSID " MAC_ADDRESS_STR,
               __func__, MAC_ADDR_ARRAY(macBSSID));

    wdiStatus = WDI_QueueAssocRequest( pWDICtx, pBSSSes, pEventData);
    wpalMutexRelease(&pWDICtx->wptMutex);
    return wdiStatus;
  }



  wpalMutexRelease(&pWDICtx->wptMutex);
  /*                                                                       
                      
                                                                         */
  if (( WDI_STATUS_SUCCESS != WDI_GetMessageBuffer( pWDICtx, WDI_RMV_STA_BCAST_KEY_REQ,
                        sizeof(halRemoveStaBcastKeyReqMsg.removeStaKeyParams),
                        &pSendBuffer, &usDataOffset, &usSendSize))||
      ( usSendSize < (usDataOffset + sizeof(halRemoveStaBcastKeyReqMsg.removeStaKeyParams) )))
  {
     WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_WARN,
              "Unable to get send buffer in set bss key req %x %x %x",
                pEventData, pwdiRemoveSTABcastKeyParams, wdiRemoveSTAKeyRspCb);
     WDI_ASSERT(0);
     return WDI_STATUS_E_FAILURE;
  }

  /*                                                                       
                                                
                                                                         */

  halRemoveStaBcastKeyReqMsg.removeStaKeyParams.staIdx =
      pwdiRemoveSTABcastKeyParams->wdiKeyInfo.ucSTAIdx;

  halRemoveStaBcastKeyReqMsg.removeStaKeyParams.encType =
      WDI_2_HAL_ENC_TYPE (pwdiRemoveSTABcastKeyParams->wdiKeyInfo.wdiEncType);

  halRemoveStaBcastKeyReqMsg.removeStaKeyParams.keyId =
      pwdiRemoveSTABcastKeyParams->wdiKeyInfo.ucKeyId;

  halRemoveStaBcastKeyReqMsg.removeStaKeyParams.unicast =
      pwdiRemoveSTABcastKeyParams->wdiKeyInfo.ucUnicast;

  wpalMemoryCopy( pSendBuffer+usDataOffset,
                    &halRemoveStaBcastKeyReqMsg.removeStaKeyParams,
                    sizeof(halRemoveStaBcastKeyReqMsg.removeStaKeyParams));

  pWDICtx->wdiReqStatusCB     = pwdiRemoveSTABcastKeyParams->wdiReqStatusCB;
  pWDICtx->pReqStatusUserData = pwdiRemoveSTABcastKeyParams->pUserData;

  /*                                                                         
                                      
                                                                           */
  return  WDI_SendMsg( pWDICtx, pSendBuffer, usSendSize,
                       wdiRemoveSTAKeyRspCb, pEventData->pUserData,
                       WDI_RMV_STA_KEY_RESP);

}/*                               */

/*                                                                          
                                                    
                                                                          */
/* 
                                                                
                  

                                                         
                                                                    

     
                                    
*/
WDI_Status
WDI_ProcessAddTSpecReq
(
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
)
{
  WDI_AddTSReqParamsType*  pwdiAddTSParams;
  WDI_AddTsRspCb           wdiAddTSRspCb;
  wpt_uint8                ucCurrentBSSSesIdx  = 0;
  WDI_BSSSessionType*      pBSSSes             = NULL;
  wpt_uint8*               pSendBuffer         = NULL;
  wpt_uint16               usDataOffset        = 0;
  wpt_uint16               usSendSize          = 0;
  WDI_Status               wdiStatus           = WDI_STATUS_SUCCESS;
  wpt_macAddr              macBSSID;
  tAddTsParams             halAddTsParams      = {0};

  /*                                                                        */

  /*                                                                         
                
                                                                           */
  if (( NULL == pEventData ) || ( NULL == pEventData->pEventData ) ||
      ( NULL == pEventData->pCBfnc ))
  {
     WPAL_TRACE( eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_WARN,
                 "%s: Invalid parameters", __FUNCTION__);
     WDI_ASSERT(0);
     return WDI_STATUS_E_FAILURE;
  }

  pwdiAddTSParams = (WDI_AddTSReqParamsType*)pEventData->pEventData;
  wdiAddTSRspCb   = (WDI_AddTsRspCb)pEventData->pCBfnc;
  /*                                                                         
                                                                           
                                              
                                                                           */
  wpalMutexAcquire(&pWDICtx->wptMutex);

  /*                                                                        
                                                                       
                                                                          */
  if ( WDI_STATUS_SUCCESS != WDI_STATableGetStaBSSIDAddr(pWDICtx,
                                        pwdiAddTSParams->wdiTsInfo.ucSTAIdx,
                                        &macBSSID))
  {
    WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_WARN,
             "This station does not exist in the WDI Station Table %d");
    wpalMutexRelease(&pWDICtx->wptMutex);
    return WDI_STATUS_E_FAILURE;
  }

  ucCurrentBSSSesIdx = WDI_FindAssocSession( pWDICtx, macBSSID, &pBSSSes);
  if ( NULL == pBSSSes )
  {
    WPAL_TRACE( eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_ERROR,
              "%s: Association sequence for this BSS does not yet exist. macBSSID " MAC_ADDRESS_STR,
              __func__, MAC_ADDR_ARRAY(macBSSID));

    wpalMutexRelease(&pWDICtx->wptMutex);
    return WDI_STATUS_E_NOT_ALLOWED;
  }

  /*                                                                        
                                                             
                                             
                                                                          */
  if ( eWLAN_PAL_TRUE == pBSSSes->bAssocReqQueued )
  {
    WPAL_TRACE( eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_WARN,
              "%s: Association sequence for this BSS exists but currently queued. macBSSID " MAC_ADDRESS_STR,
              __func__, MAC_ADDR_ARRAY(macBSSID));

    wdiStatus = WDI_QueueAssocRequest( pWDICtx, pBSSSes, pEventData);
    wpalMutexRelease(&pWDICtx->wptMutex);
    return wdiStatus;
  }

  wpalMutexRelease(&pWDICtx->wptMutex);
  /*                                                                       
                      
                                                                   
                                                                         */
  if (( WDI_STATUS_SUCCESS != WDI_GetMessageBuffer( pWDICtx, WDI_ADD_TS_REQ,
                                                    sizeof(halAddTsParams),
                                                    &pSendBuffer, &usDataOffset,
                                                    &usSendSize))||
      ( usSendSize < (usDataOffset + sizeof(halAddTsParams) )))
  {
     WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_WARN,
              "Unable to get send buffer in set bss key req %x %x %x",
                pEventData, pwdiAddTSParams, wdiAddTSRspCb);
     WDI_ASSERT(0);
     return WDI_STATUS_E_FAILURE;
  }

  halAddTsParams.staIdx = pwdiAddTSParams->wdiTsInfo.ucSTAIdx;
  halAddTsParams.tspecIdx = pwdiAddTSParams->wdiTsInfo.ucTspecIdx;

  //        
  halAddTsParams.tspec.type = pwdiAddTSParams->wdiTsInfo.wdiTspecIE.ucType;
  halAddTsParams.tspec.length = pwdiAddTSParams->wdiTsInfo.wdiTspecIE.ucLength;
  halAddTsParams.tspec.nomMsduSz =
     pwdiAddTSParams->wdiTsInfo.wdiTspecIE.usNomMsduSz;
  halAddTsParams.tspec.maxMsduSz =
     pwdiAddTSParams->wdiTsInfo.wdiTspecIE.usMaxMsduSz;
  halAddTsParams.tspec.minSvcInterval =
     pwdiAddTSParams->wdiTsInfo.wdiTspecIE.uMinSvcInterval;
  halAddTsParams.tspec.maxSvcInterval =
     pwdiAddTSParams->wdiTsInfo.wdiTspecIE.uMaxSvcInterval;
  halAddTsParams.tspec.inactInterval =
     pwdiAddTSParams->wdiTsInfo.wdiTspecIE.uInactInterval;
  halAddTsParams.tspec.suspendInterval =
     pwdiAddTSParams->wdiTsInfo.wdiTspecIE.uSuspendInterval;
  halAddTsParams.tspec.svcStartTime =
     pwdiAddTSParams->wdiTsInfo.wdiTspecIE.uSvcStartTime;
  halAddTsParams.tspec.minDataRate =
     pwdiAddTSParams->wdiTsInfo.wdiTspecIE.uMinDataRate;
  halAddTsParams.tspec.meanDataRate =
     pwdiAddTSParams->wdiTsInfo.wdiTspecIE.uMeanDataRate;
  halAddTsParams.tspec.peakDataRate =
     pwdiAddTSParams->wdiTsInfo.wdiTspecIE.uPeakDataRate;
  halAddTsParams.tspec.maxBurstSz =
     pwdiAddTSParams->wdiTsInfo.wdiTspecIE.uMaxBurstSz;
  halAddTsParams.tspec.delayBound =
     pwdiAddTSParams->wdiTsInfo.wdiTspecIE.uDelayBound;
  halAddTsParams.tspec.minPhyRate =
     pwdiAddTSParams->wdiTsInfo.wdiTspecIE.uMinPhyRate;
  halAddTsParams.tspec.surplusBw =
     pwdiAddTSParams->wdiTsInfo.wdiTspecIE.usSurplusBw;
  halAddTsParams.tspec.mediumTime =
     pwdiAddTSParams->wdiTsInfo.wdiTspecIE.usMediumTime;

  //                            
  halAddTsParams.tspec.tsinfo.traffic.ackPolicy =
     pwdiAddTSParams->wdiTsInfo.wdiTspecIE.wdiTSinfo.wdiTraffic.accessPolicy;
  halAddTsParams.tspec.tsinfo.traffic.userPrio =
     pwdiAddTSParams->wdiTsInfo.wdiTspecIE.wdiTSinfo.wdiTraffic.userPrio;
  halAddTsParams.tspec.tsinfo.traffic.psb =
     pwdiAddTSParams->wdiTsInfo.wdiTspecIE.wdiTSinfo.wdiTraffic.psb;
  halAddTsParams.tspec.tsinfo.traffic.aggregation =
     pwdiAddTSParams->wdiTsInfo.wdiTspecIE.wdiTSinfo.wdiTraffic.aggregation;
  halAddTsParams.tspec.tsinfo.traffic.direction =
     pwdiAddTSParams->wdiTsInfo.wdiTspecIE.wdiTSinfo.wdiTraffic.direction;
  halAddTsParams.tspec.tsinfo.traffic.tsid =
     pwdiAddTSParams->wdiTsInfo.wdiTspecIE.wdiTSinfo.wdiTraffic.tsid;
  halAddTsParams.tspec.tsinfo.traffic.trafficType =
     pwdiAddTSParams->wdiTsInfo.wdiTspecIE.wdiTSinfo.wdiTraffic.trafficType;

  //                             
  halAddTsParams.tspec.tsinfo.schedule.rsvd =
     pwdiAddTSParams->wdiTsInfo.wdiTspecIE.wdiTSinfo.wdiSchedule.rsvd;
  halAddTsParams.tspec.tsinfo.schedule.schedule =
     pwdiAddTSParams->wdiTsInfo.wdiTspecIE.wdiTSinfo.wdiSchedule.schedule;

  wpalMemoryCopy( pSendBuffer+usDataOffset,
                  &halAddTsParams,
                  sizeof(halAddTsParams));

  pWDICtx->wdiReqStatusCB     = pwdiAddTSParams->wdiReqStatusCB;
  pWDICtx->pReqStatusUserData = pwdiAddTSParams->pUserData;

  /*                                                                         
                              
                                                                           */
  return  WDI_SendMsg( pWDICtx, pSendBuffer, usSendSize,
                       wdiAddTSRspCb, pEventData->pUserData,
                       WDI_ADD_TS_RESP);
}/*                      */


/* 
                                                                
                  

                                                         
                                                                    

     
                                    
*/
WDI_Status
WDI_ProcessDelTSpecReq
(
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
)
{
  WDI_DelTSReqParamsType*      pwdiDelTSParams;
  WDI_DelTsRspCb               wdiDelTSRspCb;
  wpt_uint8                    ucCurrentBSSSesIdx  = 0;
  WDI_BSSSessionType*          pBSSSes             = NULL;
  wpt_uint8*                   pSendBuffer         = NULL;
  wpt_uint16                   usDataOffset        = 0;
  wpt_uint16                   usSendSize          = 0;
  WDI_Status                   wdiStatus           = WDI_STATUS_SUCCESS;
  /*                                                                        */

  /*                                                                         
                
                                                                           */
  if (( NULL == pEventData ) || ( NULL == pEventData->pEventData ) ||
      ( NULL == pEventData->pCBfnc ))
  {
     WPAL_TRACE( eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_WARN,
                 "%s: Invalid parameters", __FUNCTION__);
     WDI_ASSERT(0);
     return WDI_STATUS_E_FAILURE;
  }

  pwdiDelTSParams = (WDI_DelTSReqParamsType*)pEventData->pEventData;
  wdiDelTSRspCb   = (WDI_DelTsRspCb)pEventData->pCBfnc;

  /*                                                                         
                                                                           
                                              
                                                                           */
  wpalMutexAcquire(&pWDICtx->wptMutex);

  /*                                                                        
                                              
                                                                          */
  ucCurrentBSSSesIdx = WDI_FindAssocSession( pWDICtx,
                           pwdiDelTSParams->wdiDelTSInfo.macBSSID,
                          &pBSSSes);

  if ( NULL == pBSSSes )
  {
    WPAL_TRACE( eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_ERROR,
            "%s: Association sequence for this BSS does not yet exist. macBSSID " MAC_ADDRESS_STR,
            __func__, MAC_ADDR_ARRAY(pwdiDelTSParams->wdiDelTSInfo.macBSSID));

    wpalMutexRelease(&pWDICtx->wptMutex);
    return WDI_STATUS_E_NOT_ALLOWED;
  }

  /*                                                                        
                                                             
                                             
                                                                          */
  if ( eWLAN_PAL_TRUE == pBSSSes->bAssocReqQueued )
  {
    WPAL_TRACE( eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_WARN,
              "%s: Association sequence for this BSS exists but currently queued. macBSSID " MAC_ADDRESS_STR,
              __func__, MAC_ADDR_ARRAY(pwdiDelTSParams->wdiDelTSInfo.macBSSID));

    wdiStatus = WDI_QueueAssocRequest( pWDICtx, pBSSSes, pEventData);
    wpalMutexRelease(&pWDICtx->wptMutex);
    return wdiStatus;
  }


  wpalMutexRelease(&pWDICtx->wptMutex);
  /*                                                                       
                      
                                                                   
                                                                         */
  if (( WDI_STATUS_SUCCESS != WDI_GetMessageBuffer( pWDICtx, WDI_DEL_TS_REQ,
                        sizeof(pwdiDelTSParams->wdiDelTSInfo),
                        &pSendBuffer, &usDataOffset, &usSendSize))||
      ( usSendSize < (usDataOffset + sizeof(pwdiDelTSParams->wdiDelTSInfo) )))
  {
     WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_WARN,
              "Unable to get send buffer in set bss key req %x %x %x",
                pEventData, pwdiDelTSParams, wdiDelTSRspCb);
     WDI_ASSERT(0);
     return WDI_STATUS_E_FAILURE;
  }

  wpalMemoryCopy( pSendBuffer+usDataOffset,
                  &pwdiDelTSParams->wdiDelTSInfo,
                  sizeof(pwdiDelTSParams->wdiDelTSInfo));

  pWDICtx->wdiReqStatusCB     = pwdiDelTSParams->wdiReqStatusCB;
  pWDICtx->pReqStatusUserData = pwdiDelTSParams->pUserData;

  /*                                                                         
                              
                                                                           */
  return  WDI_SendMsg( pWDICtx, pSendBuffer, usSendSize,
                       wdiDelTSRspCb, pEventData->pUserData, WDI_DEL_TS_RESP);
}/*                      */

/* 
                                                                
                           

                                                         
                                                                    

     
                                    
*/
WDI_Status
WDI_ProcessUpdateEDCAParamsReq
(
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
)
{
  WDI_UpdateEDCAParamsType*    pwdiUpdateEDCAParams;
  WDI_UpdateEDCAParamsRspCb    wdiUpdateEDCARspCb;
  wpt_uint8                    ucCurrentBSSSesIdx  = 0;
  WDI_BSSSessionType*          pBSSSes             = NULL;
  wpt_uint8*                   pSendBuffer         = NULL;
  wpt_uint16                   usDataOffset         = 0;
  wpt_uint16                   usSendSize           = 0;
  WDI_Status                   wdiStatus;
  /*                                                                        */

  /*                                                                         
                
                                                                           */
  if (( NULL == pEventData ) || ( NULL == pEventData->pEventData ) ||
      ( NULL == pEventData->pCBfnc ))
  {
     WPAL_TRACE( eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_WARN,
                 "%s: Invalid parameters", __FUNCTION__);
     WDI_ASSERT(0);
     return WDI_STATUS_E_FAILURE;
  }

  pwdiUpdateEDCAParams = (WDI_UpdateEDCAParamsType*)pEventData->pEventData;
  wdiUpdateEDCARspCb   = (WDI_UpdateEDCAParamsRspCb)pEventData->pCBfnc;
  /*                                                                         
                                                                           
                                              
                                                                           */
  wpalMutexAcquire(&pWDICtx->wptMutex);

  /*                                                                        
                                              
                                                                          */
  ucCurrentBSSSesIdx = WDI_FindAssocSessionByBSSIdx( pWDICtx,
                           pwdiUpdateEDCAParams->wdiEDCAInfo.ucBssIdx,
                          &pBSSSes);

  if ( NULL == pBSSSes )
  {
    WPAL_TRACE( eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_ERROR,
            "%s: Association sequence for this BSS does not yet exist. ucBssIdx %d",
            __func__, pwdiUpdateEDCAParams->wdiEDCAInfo.ucBssIdx);

    wpalMutexRelease(&pWDICtx->wptMutex);
    return WDI_STATUS_E_NOT_ALLOWED;
  }

  /*                                                                        
                                                             
                                             
                                                                          */
  if ( eWLAN_PAL_TRUE == pBSSSes->bAssocReqQueued )
  {
    WPAL_TRACE( eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_WARN,
              "%s: Association sequence for this BSS exists but currently queued. ucBssIdx %d",
              __func__, pwdiUpdateEDCAParams->wdiEDCAInfo.ucBssIdx);

    wdiStatus = WDI_QueueAssocRequest( pWDICtx, pBSSSes, pEventData);
    wpalMutexRelease(&pWDICtx->wptMutex);
    return wdiStatus;
  }


  wpalMutexRelease(&pWDICtx->wptMutex);
  /*                                                                       
                      
                                                                   
                                                                         */
  if (( WDI_STATUS_SUCCESS != WDI_GetMessageBuffer( pWDICtx, WDI_UPD_EDCA_PRMS_REQ,
                        sizeof(pwdiUpdateEDCAParams->wdiEDCAInfo),
                        &pSendBuffer, &usDataOffset, &usSendSize))||
      ( usSendSize < (usDataOffset + sizeof(pwdiUpdateEDCAParams->wdiEDCAInfo) )))
  {
     WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_WARN,
              "Unable to get send buffer in set bss key req %x %x %x",
                pEventData, pwdiUpdateEDCAParams, wdiUpdateEDCARspCb);
     WDI_ASSERT(0);
     return WDI_STATUS_E_FAILURE;
  }

  wpalMemoryCopy( pSendBuffer+usDataOffset,
                  &pwdiUpdateEDCAParams->wdiEDCAInfo,
                  sizeof(pwdiUpdateEDCAParams->wdiEDCAInfo));

  pWDICtx->wdiReqStatusCB     = pwdiUpdateEDCAParams->wdiReqStatusCB;
  pWDICtx->pReqStatusUserData = pwdiUpdateEDCAParams->pUserData;

  /*                                                                         
                                          
                                                                           */
  return  WDI_SendMsg( pWDICtx, pSendBuffer, usSendSize,
                       wdiUpdateEDCARspCb, pEventData->pUserData,
                       WDI_UPD_EDCA_PRMS_RESP);
}/*                              */

/* 
                                                             
                  

                                                         
                                                                    

     
                                    
*/
WDI_Status
WDI_ProcessAddBASessionReq
(
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
)
{
  WDI_AddBASessionReqParamsType*  pwdiAddBASessionParams;
  WDI_AddBASessionRspCb           wdiAddBASessionRspCb;
  wpt_uint8                       ucCurrentBSSSesIdx  = 0;
  WDI_BSSSessionType*             pBSSSes             = NULL;
  wpt_uint8*                      pSendBuffer         = NULL;
  wpt_uint16                      usDataOffset        = 0;
  wpt_uint16                      usSendSize          = 0;
  WDI_Status                      wdiStatus           = WDI_STATUS_SUCCESS;
  wpt_macAddr                     macBSSID;

  tAddBASessionReqMsg             halAddBASessionReq;
  /*                                                                        */

  /*                                                                         
                
                                                                           */
  if (( NULL == pEventData ) || ( NULL == pEventData->pEventData ) ||
      ( NULL == pEventData->pCBfnc ))
  {
     WPAL_TRACE( eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_WARN,
                 "%s: Invalid parameters", __FUNCTION__);
     WDI_ASSERT(0);
     return WDI_STATUS_E_FAILURE;
  }

  pwdiAddBASessionParams =
                  (WDI_AddBASessionReqParamsType*)pEventData->pEventData;
  wdiAddBASessionRspCb =
                  (WDI_AddBASessionRspCb)pEventData->pCBfnc;
  /*                                                                         
                                                                           
                                              
                                                                           */
  wpalMutexAcquire(&pWDICtx->wptMutex);

  /*                                                                        
                                              
                                                                          */
  if ( WDI_STATUS_SUCCESS != WDI_STATableGetStaBSSIDAddr(pWDICtx,
                   pwdiAddBASessionParams->wdiBASessionInfoType.ucSTAIdx,
                   &macBSSID))
  {
    WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_WARN,
             "This station does not exist in the WDI Station Table %d");
    wpalMutexRelease(&pWDICtx->wptMutex);
    return WDI_STATUS_E_FAILURE;
  }


  ucCurrentBSSSesIdx = WDI_FindAssocSession( pWDICtx, macBSSID, &pBSSSes);

  if ( NULL == pBSSSes )
  {
    WPAL_TRACE( eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_ERROR,
          "%s: Association sequence for this BSS does not yet exist. macBSSID " MAC_ADDRESS_STR,
          __func__, MAC_ADDR_ARRAY(macBSSID));

    wpalMutexRelease(&pWDICtx->wptMutex);
    return WDI_STATUS_E_NOT_ALLOWED;
  }

  /*                                                                        
                                                             
                                             
                                                                          */
  if ( eWLAN_PAL_TRUE == pBSSSes->bAssocReqQueued )
  {
    WPAL_TRACE( eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_WARN,
              "%s: Association sequence for this BSS exists but currently queued. macBSSID " MAC_ADDRESS_STR,
               __func__, MAC_ADDR_ARRAY(macBSSID));

    wdiStatus = WDI_QueueAssocRequest( pWDICtx, pBSSSes, pEventData);
    wpalMutexRelease(&pWDICtx->wptMutex);
    return wdiStatus;
  }


  wpalMutexRelease(&pWDICtx->wptMutex);
  /*                                                                       
                      
                                                                         */
  if (( WDI_STATUS_SUCCESS != WDI_GetMessageBuffer( pWDICtx,
                        WDI_ADD_BA_SESSION_REQ,
                        sizeof(halAddBASessionReq.addBASessionParams),
                        &pSendBuffer, &usDataOffset, &usSendSize))||
      ( usSendSize <
            (usDataOffset + sizeof(halAddBASessionReq.addBASessionParams) )))
  {
     WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_WARN,
              "Unable to get send buffer in Add BA session req %x %x %x",
                pEventData, pwdiAddBASessionParams, wdiAddBASessionRspCb);
     WDI_ASSERT(0);
     return WDI_STATUS_E_FAILURE;
  }

  halAddBASessionReq.addBASessionParams.staIdx =
                  pwdiAddBASessionParams->wdiBASessionInfoType.ucSTAIdx;
  wpalMemoryCopy(halAddBASessionReq.addBASessionParams.peerMacAddr,
                  pwdiAddBASessionParams->wdiBASessionInfoType.macPeerAddr,
                  WDI_MAC_ADDR_LEN);
  halAddBASessionReq.addBASessionParams.baTID =
                  pwdiAddBASessionParams->wdiBASessionInfoType.ucBaTID;
  halAddBASessionReq.addBASessionParams.baPolicy =
                  pwdiAddBASessionParams->wdiBASessionInfoType.ucBaPolicy;
  halAddBASessionReq.addBASessionParams.baBufferSize =
                  pwdiAddBASessionParams->wdiBASessionInfoType.usBaBufferSize;
  halAddBASessionReq.addBASessionParams.baTimeout =
                  pwdiAddBASessionParams->wdiBASessionInfoType.usBaTimeout;
  halAddBASessionReq.addBASessionParams.baSSN =
                  pwdiAddBASessionParams->wdiBASessionInfoType.usBaSSN;
  halAddBASessionReq.addBASessionParams.baDirection =
                  pwdiAddBASessionParams->wdiBASessionInfoType.ucBaDirection;

  wpalMemoryCopy( pSendBuffer+usDataOffset,
                  &halAddBASessionReq.addBASessionParams,
                  sizeof(halAddBASessionReq.addBASessionParams));

  pWDICtx->wdiReqStatusCB     = pwdiAddBASessionParams->wdiReqStatusCB;
  pWDICtx->pReqStatusUserData = pwdiAddBASessionParams->pUserData;

  /*                                                                         
                             
                                                                           */
  return  WDI_SendMsg( pWDICtx, pSendBuffer, usSendSize,
                        wdiAddBASessionRspCb, pEventData->pUserData,
                        WDI_ADD_BA_SESSION_RESP);
}/*                          */

/* 
                                                             
                  

                                                         
                                                                    

     
                                    
*/
WDI_Status
WDI_ProcessDelBAReq
(
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
)
{
  WDI_DelBAReqParamsType*      pwdiDelBAParams;
  WDI_DelBARspCb               wdiDelBARspCb;
  wpt_uint8                    ucCurrentBSSSesIdx  = 0;
  WDI_BSSSessionType*          pBSSSes             = NULL;
  wpt_uint8*                   pSendBuffer         = NULL;
  wpt_uint16                   usDataOffset        = 0;
  wpt_uint16                   usSendSize          = 0;
  WDI_Status                   wdiStatus           = WDI_STATUS_SUCCESS;
  wpt_macAddr                  macBSSID;
  tDelBAParams                 halDelBAparam;
  /*                                                                        */

  /*                                                                         
                
                                                                           */
  if (( NULL == pEventData ) || ( NULL == pEventData->pEventData ) ||
      ( NULL == pEventData->pCBfnc ))
  {
     WPAL_TRACE( eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_WARN,
                 "%s: Invalid parameters", __FUNCTION__);
     WDI_ASSERT(0);
     return WDI_STATUS_E_FAILURE;
  }

  pwdiDelBAParams = (WDI_DelBAReqParamsType*)pEventData->pEventData;
  wdiDelBARspCb   = (WDI_DelBARspCb)pEventData->pCBfnc;
  /*                                                                         
                                                                           
                                              
                                                                           */
  wpalMutexAcquire(&pWDICtx->wptMutex);

  /*                                                                        
                                              
                                                                          */
  if ( WDI_STATUS_SUCCESS != WDI_STATableGetStaBSSIDAddr(pWDICtx,
                                     pwdiDelBAParams->wdiBAInfo.ucSTAIdx,
                                     &macBSSID))
  {
    WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_WARN,
             "This station does not exist in the WDI Station Table %d");
    wpalMutexRelease(&pWDICtx->wptMutex);
    return WDI_STATUS_E_FAILURE;
  }

  ucCurrentBSSSesIdx = WDI_FindAssocSession( pWDICtx, macBSSID, &pBSSSes);

  if ( NULL == pBSSSes )
  {
    WPAL_TRACE( eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_ERROR,
            "%s: Association sequence for this BSS does not yet exist. macBSSID " MAC_ADDRESS_STR,
            __func__, MAC_ADDR_ARRAY(macBSSID));

    wpalMutexRelease(&pWDICtx->wptMutex);
    return WDI_STATUS_E_NOT_ALLOWED;
  }

  /*                                                                        
                                                             
                                             
                                                                          */
  if ( eWLAN_PAL_TRUE == pBSSSes->bAssocReqQueued )
  {
    WPAL_TRACE( eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_WARN,
              "%s: Association sequence for this BSS exists but currently queued. macBSSID " MAC_ADDRESS_STR,
              __func__, MAC_ADDR_ARRAY(macBSSID));

    wdiStatus = WDI_QueueAssocRequest( pWDICtx, pBSSSes, pEventData);
    wpalMutexRelease(&pWDICtx->wptMutex);
    return wdiStatus;
  }

  wpalMutexRelease(&pWDICtx->wptMutex);
  if (( WDI_STATUS_SUCCESS != WDI_GetMessageBuffer( pWDICtx, WDI_DEL_BA_REQ,
                        sizeof(halDelBAparam),
                        &pSendBuffer, &usDataOffset, &usSendSize))||
      ( usSendSize < (usDataOffset + sizeof(halDelBAparam) )))
  {
     WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_WARN,
              "Unable to get send buffer for DEL BA req %x %x %x",
                pEventData, pwdiDelBAParams, wdiDelBARspCb);
     WDI_ASSERT(0);
     return WDI_STATUS_E_FAILURE;
  }

  halDelBAparam.staIdx = pwdiDelBAParams->wdiBAInfo.ucSTAIdx;
  halDelBAparam.baTID = pwdiDelBAParams->wdiBAInfo.ucBaTID;
  halDelBAparam.baDirection = pwdiDelBAParams->wdiBAInfo.ucBaDirection;

  wpalMemoryCopy( pSendBuffer+usDataOffset,
                  &halDelBAparam,
                  sizeof(halDelBAparam));

  pWDICtx->wdiReqStatusCB     = pwdiDelBAParams->wdiReqStatusCB;
  pWDICtx->pReqStatusUserData = pwdiDelBAParams->pUserData;

  /*                                                                         
                             
                                                                           */
  return  WDI_SendMsg( pWDICtx, pSendBuffer, usSendSize,
                       wdiDelBARspCb, pEventData->pUserData, WDI_DEL_BA_RESP);
}/*                   */

#ifdef FEATURE_WLAN_CCX

WDI_Status
WDI_ProcessTSMStatsReq
(
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
)
{
  WDI_TSMStatsReqParamsType*  pwdiTSMParams;
  WDI_TsmRspCb                wdiTSMRspCb;
  wpt_uint8                   ucCurrentBSSSesIdx   = 0;
  WDI_BSSSessionType*         pBSSSes              = NULL;
  wpt_uint8*                  pSendBuffer          = NULL;
  wpt_uint16                  usDataOffset         = 0;
  wpt_uint16                  usSendSize           = 0;
  WDI_Status                  wdiStatus            = WDI_STATUS_SUCCESS;
  tTsmStatsParams             halTsmStatsReqParams = {0};

  /*                                                                        */

  /*                                                                         
                   
                                                                           */
  if (( NULL == pEventData ) || ( NULL == pEventData->pEventData ) ||
      ( NULL == pEventData->pCBfnc ))
  {
     WPAL_TRACE( eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_ERROR,
                 "%s: Invalid parameters", __FUNCTION__);
     WDI_ASSERT(0);
     return WDI_STATUS_E_FAILURE;
  }

  pwdiTSMParams = (WDI_TSMStatsReqParamsType*)pEventData->pEventData;
  wdiTSMRspCb   = (WDI_TsmRspCb)pEventData->pCBfnc;
  /*                                                                         
                                                                           
                                              
                                                                           */
  wpalMutexAcquire(&pWDICtx->wptMutex);

  ucCurrentBSSSesIdx = WDI_FindAssocSession( pWDICtx, pwdiTSMParams->wdiTsmStatsParamsInfo.bssid, &pBSSSes);
  if ( NULL == pBSSSes )
  {
    WPAL_TRACE( eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_ERROR,
            "%s: Association sequence for this BSS does not yet exist. macBSSID " MAC_ADDRESS_STR,
            __func__, MAC_ADDR_ARRAY(pwdiTSMParams->wdiTsmStatsParamsInfo.bssid));

    wpalMutexRelease(&pWDICtx->wptMutex);
    return WDI_STATUS_E_NOT_ALLOWED;
  }

  /*                                                                        
                                                             
                                             
                                                                          */
  if ( eWLAN_PAL_TRUE == pBSSSes->bAssocReqQueued )
  {
    WPAL_TRACE( eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_WARN,
              "s: Association sequence for this BSS exists but currently queued. macBSSID " MAC_ADDRESS_STR,
              __func__, MAC_ADDR_ARRAY(pwdiTSMParams->wdiTsmStatsParamsInfo.bssid));

    wdiStatus = WDI_QueueAssocRequest( pWDICtx, pBSSSes, pEventData);
    wpalMutexRelease(&pWDICtx->wptMutex);
    return wdiStatus;
  }

  wpalMutexRelease(&pWDICtx->wptMutex);
  /*                                                                       
                      
                                                                   
                                                                         */
  if (( WDI_STATUS_SUCCESS != WDI_GetMessageBuffer( pWDICtx, WDI_TSM_STATS_REQ,
                                                    sizeof(halTsmStatsReqParams),
                                                    &pSendBuffer, &usDataOffset,                                                     &usSendSize))||
      ( usSendSize < (usDataOffset + sizeof(halTsmStatsReqParams) )))
  {
     WPAL_TRACE( eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_ERROR,
              "Unable to get send buffer in set bss key req %x %x %x",
                pEventData, pwdiTSMParams, wdiTSMRspCb);
     WDI_ASSERT(0);
     return WDI_STATUS_E_FAILURE;
  }

  halTsmStatsReqParams.tsmTID = pwdiTSMParams->wdiTsmStatsParamsInfo.ucTid;
  wpalMemoryCopy(halTsmStatsReqParams.bssId,
                 pwdiTSMParams->wdiTsmStatsParamsInfo.bssid,
                 WDI_MAC_ADDR_LEN);
  wpalMemoryCopy( pSendBuffer+usDataOffset,
                  &halTsmStatsReqParams,
                  sizeof(halTsmStatsReqParams));

  pWDICtx->wdiReqStatusCB     = pwdiTSMParams->wdiReqStatusCB;
  pWDICtx->pReqStatusUserData = pwdiTSMParams->pUserData;

  /*                                                                         
                                 
                                                                           */
  return  WDI_SendMsg( pWDICtx, pSendBuffer, usSendSize,
                       wdiTSMRspCb, pEventData->pUserData,
                       WDI_TSM_STATS_RESP);
}/*                      */

#endif


/* 
                                                               
                  

                                                         
                                                                    

     
                                    
*/
WDI_Status
WDI_ProcessFlushAcReq
(
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
)
{
   WDI_FlushAcReqParamsType*    pwdiFlushAcParams = NULL;
   WDI_FlushAcRspCb             wdiFlushAcRspCb;
   wpt_uint8*                   pSendBuffer         = NULL;
   wpt_uint16                   usDataOffset        = 0;
   wpt_uint16                   usSendSize          = 0;
   /*                                                                        */

   /*                                                                         
                 
                                                                            */
   if (( NULL == pEventData ) || ( NULL == pEventData->pEventData ) ||
       ( NULL == pEventData->pCBfnc ))
   {
      WPAL_TRACE( eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_WARN,
                  "%s: Invalid parameters", __FUNCTION__);
      WDI_ASSERT(0);
      return WDI_STATUS_E_FAILURE;
   }

   pwdiFlushAcParams = (WDI_FlushAcReqParamsType*)pEventData->pEventData;
   wdiFlushAcRspCb   = (WDI_FlushAcRspCb)pEventData->pCBfnc;
   /*                                                                       
                       
                                                                    
                                                                          */
   if (( WDI_STATUS_SUCCESS != WDI_GetMessageBuffer( pWDICtx, WDI_FLUSH_AC_REQ,
                         sizeof(pwdiFlushAcParams->wdiFlushAcInfo),
                         &pSendBuffer, &usDataOffset, &usSendSize))||
       ( usSendSize < (usDataOffset + sizeof(pwdiFlushAcParams->wdiFlushAcInfo) )))
   {
      WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_WARN,
               "Unable to get send buffer in set bss key req %x %x %x",
                 pEventData, pwdiFlushAcParams, wdiFlushAcRspCb);
      WDI_ASSERT(0);
      return WDI_STATUS_E_FAILURE;
   }

   wpalMemoryCopy( pSendBuffer+usDataOffset,
                   &pwdiFlushAcParams->wdiFlushAcInfo,
                   sizeof(pwdiFlushAcParams->wdiFlushAcInfo));

   pWDICtx->wdiReqStatusCB     = pwdiFlushAcParams->wdiReqStatusCB;
   pWDICtx->pReqStatusUserData = pwdiFlushAcParams->pUserData;

   /*                                                                         
                              
                                                                            */
   return  WDI_SendMsg( pWDICtx, pSendBuffer, usSendSize,
                        wdiFlushAcRspCb, pEventData->pUserData, WDI_FLUSH_AC_RESP);
}/*                     */

/* 
                                                               
                      

                                                         
                                                                    

     
                                    
*/
WDI_Status
WDI_ProcessBtAmpEventReq
(
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
)
{
   WDI_BtAmpEventParamsType*    pwdiBtAmpEventParams = NULL;
   WDI_BtAmpEventRspCb          wdiBtAmpEventRspCb;
   wpt_uint8*                   pSendBuffer         = NULL;
   wpt_uint16                   usDataOffset        = 0;
   wpt_uint16                   usSendSize          = 0;

   tBtAmpEventMsg               haltBtAmpEventMsg;
   /*                                                                        */

   /*                                                                         
                 
                                                                            */
   if (( NULL == pEventData ) || ( NULL == pEventData->pEventData ) ||
       ( NULL == pEventData->pCBfnc ))
   {
      WPAL_TRACE( eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_WARN,
                  "%s: Invalid parameters", __FUNCTION__);
      WDI_ASSERT(0);
      return WDI_STATUS_E_FAILURE;
   }

   pwdiBtAmpEventParams = (WDI_BtAmpEventParamsType*)pEventData->pEventData;
   wdiBtAmpEventRspCb   = (WDI_BtAmpEventRspCb)pEventData->pCBfnc;
   /*                                                                       
                       
                                                                    
                                                                          */
   if (( WDI_STATUS_SUCCESS != WDI_GetMessageBuffer( pWDICtx, WDI_BTAMP_EVENT_REQ,
                         sizeof(haltBtAmpEventMsg.btAmpEventParams),
                         &pSendBuffer, &usDataOffset, &usSendSize))||
       ( usSendSize < (usDataOffset + sizeof(haltBtAmpEventMsg.btAmpEventParams) )))
   {
      WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_WARN,
               "Unable to get send buffer in BT AMP event req %x %x %x",
                 pEventData, pwdiBtAmpEventParams, wdiBtAmpEventRspCb);
      WDI_ASSERT(0);
      return WDI_STATUS_E_FAILURE;
   }

   haltBtAmpEventMsg.btAmpEventParams.btAmpEventType =
      pwdiBtAmpEventParams->wdiBtAmpEventInfo.ucBtAmpEventType;
   wpalMemoryCopy( pSendBuffer+usDataOffset,
                   &haltBtAmpEventMsg.btAmpEventParams,
                   sizeof(haltBtAmpEventMsg.btAmpEventParams));

   pWDICtx->wdiReqStatusCB     = pwdiBtAmpEventParams->wdiReqStatusCB;
   pWDICtx->pReqStatusUserData = pwdiBtAmpEventParams->pUserData;

   /*                                                                         
                              
                                                                            */
   return  WDI_SendMsg( pWDICtx, pSendBuffer, usSendSize,
                        wdiBtAmpEventRspCb, pEventData->pUserData, WDI_BTAMP_EVENT_RESP);
}/*                        */

/* 
                                                                   
                  

                                                         
                                                                    

     
                                    
*/
WDI_Status
WDI_ProcessAddSTASelfReq
(
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
)
{
  WDI_AddSTASelfReqParamsType*          pwdiAddSTASelfReqParams;
  WDI_AddSTASelfParamsRspCb             wdiAddSTASelfReqRspCb;
  wpt_uint8*                            pSendBuffer         = NULL;
  wpt_uint16                            usDataOffset        = 0;
  wpt_uint16                            usSendSize          = 0;
  tAddStaSelfParams                     halAddSTASelfParams;
  /*                                                                        */

  /*                                                                         
                
                                                                           */
  if (( NULL == pEventData ) ||
      ( NULL == pEventData->pEventData) ||
      ( NULL == pEventData->pCBfnc))
  {
     WPAL_TRACE( eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_WARN,
                 "%s: Invalid parameters", __FUNCTION__);
     WDI_ASSERT(0);
     return WDI_STATUS_E_FAILURE;
  }

  pwdiAddSTASelfReqParams =
    (WDI_AddSTASelfReqParamsType*)pEventData->pEventData;
  wdiAddSTASelfReqRspCb =
    (WDI_AddSTASelfParamsRspCb)pEventData->pCBfnc;
  /*                                                                       
                      
                                                                         */
  if (( WDI_STATUS_SUCCESS != WDI_GetMessageBuffer( pWDICtx,
                        WDI_ADD_STA_SELF_REQ,
                        sizeof(tAddStaSelfParams),
                        &pSendBuffer, &usDataOffset, &usSendSize))||
      ( usSendSize < (usDataOffset + sizeof(tAddStaSelfParams) )))
  {
     WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_WARN,
              "Unable to get send buffer in ADD STA SELF REQ %x %x %x",
     pEventData, pwdiAddSTASelfReqParams, wdiAddSTASelfReqRspCb);
     WDI_ASSERT(0);
     return WDI_STATUS_E_FAILURE;
  }

  /*                                           */
  wpalMemoryCopy(&pWDICtx->wdiCacheAddSTASelfReq, pwdiAddSTASelfReqParams,
                 sizeof(pWDICtx->wdiCacheAddSTASelfReq));

  wpalMemoryCopy(halAddSTASelfParams.selfMacAddr,
                   pwdiAddSTASelfReqParams->wdiAddSTASelfInfo.selfMacAddr, 6) ;

  wpalMemoryCopy( pSendBuffer+usDataOffset, &halAddSTASelfParams,
                                         sizeof(tAddStaSelfParams));

  pWDICtx->wdiReqStatusCB     = pwdiAddSTASelfReqParams->wdiReqStatusCB;
  pWDICtx->pReqStatusUserData = pwdiAddSTASelfReqParams->pUserData;

  /*                                                                         
                                                  
                                                                           */
  return  WDI_SendMsg( pWDICtx, pSendBuffer, usSendSize,
                       wdiAddSTASelfReqRspCb, pEventData->pUserData,
                       WDI_ADD_STA_SELF_RESP);
}/*                        */



/* 
                                                               
                      

                                                         
                                                                    

     
                                    
*/
WDI_Status
WDI_ProcessDelSTASelfReq
(
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
)
{
  WDI_DelSTASelfReqParamsType*      pwdiDelStaSelfReqParams;
  WDI_DelSTASelfRspCb               wdiDelStaSelfRspCb;
  wpt_uint8*                   pSendBuffer         = NULL;
  wpt_uint16                   usDataOffset        = 0;
  wpt_uint16                   usSendSize          = 0;
  tDelStaSelfParams            halSetDelSelfSTAParams;
  /*                                                                        */

  /*                                                                         
                
                                                                           */
  if (( NULL == pEventData ) || ( NULL == pEventData->pEventData ) ||
      ( NULL == pEventData->pCBfnc ))
  {
     WPAL_TRACE( eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_WARN,
                 "%s: Invalid parameters", __FUNCTION__);
     WDI_ASSERT(0);
     return WDI_STATUS_E_FAILURE;
  }

  pwdiDelStaSelfReqParams =
                 (WDI_DelSTASelfReqParamsType*)pEventData->pEventData;
  wdiDelStaSelfRspCb      = (WDI_DelSTASelfRspCb)pEventData->pCBfnc;

   /*                                                                       
                       
                                                                    
                                                                          */
  if (( WDI_STATUS_SUCCESS != WDI_GetMessageBuffer( pWDICtx, WDI_DEL_STA_SELF_REQ,
                         sizeof(pwdiDelStaSelfReqParams->wdiDelStaSelfInfo),
                         &pSendBuffer, &usDataOffset, &usSendSize))||
    ( usSendSize <
         (usDataOffset + sizeof(pwdiDelStaSelfReqParams->wdiDelStaSelfInfo) )))
   {
      WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_WARN,
               "Unable to get send buffer in Del Sta Self req %x %x %x",
                 pEventData, pwdiDelStaSelfReqParams, wdiDelStaSelfRspCb);
      WDI_ASSERT(0);
      return WDI_STATUS_E_FAILURE;
  }

  wpalMemoryCopy(halSetDelSelfSTAParams.selfMacAddr,
                   pwdiDelStaSelfReqParams->wdiDelStaSelfInfo.selfMacAddr, 6) ;

  wpalMemoryCopy( pSendBuffer+usDataOffset, &halSetDelSelfSTAParams,
                                         sizeof(tDelStaSelfParams));

  pWDICtx->wdiReqStatusCB     = pwdiDelStaSelfReqParams->wdiReqStatusCB;
  pWDICtx->pReqStatusUserData = pwdiDelStaSelfReqParams->pUserData;

  /*                                                                         
                              
                                                                            */
  return  WDI_SendMsg( pWDICtx, pSendBuffer, usSendSize,
                        wdiDelStaSelfRspCb, pEventData->pUserData,
                                                     WDI_DEL_STA_SELF_RESP);

}

#ifdef FEATURE_OEM_DATA_SUPPORT
/* 
                                                                 
                      

                                                         
                                                                    

     
                                    
*/
WDI_Status
WDI_ProcessStartOemDataReq
(
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
)
{
  WDI_oemDataReqParamsType*    pwdiOemDataReqParams = NULL;
  WDI_oemDataRspCb             wdiOemDataRspCb;
  wpt_uint8*                   pSendBuffer         = NULL;
  wpt_uint16                   usDataOffset        = 0;
  wpt_uint16                   usSendSize          = 0;
  wpt_uint16                   reqLen;
  tStartOemDataReqParams*      halStartOemDataReqParams;

  /*                                                                         
              
                                                                           */
  if (( NULL == pEventData ) || ( NULL == pEventData->pEventData ) ||
      ( NULL == pEventData->pCBfnc ))
  {
      WPAL_TRACE( eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_WARN,
                  "%s: Invalid parameters", __FUNCTION__);
      WDI_ASSERT(0);
      return WDI_STATUS_E_FAILURE;
  }

  pwdiOemDataReqParams = (WDI_oemDataReqParamsType*)pEventData->pEventData;
  wdiOemDataRspCb   = (WDI_oemDataRspCb)pEventData->pCBfnc;

  /*                                                                       
                       
                                                                          */

  reqLen = sizeof(tStartOemDataReqParams);

  if (( WDI_STATUS_SUCCESS != WDI_GetMessageBuffer( pWDICtx,
                         WDI_START_OEM_DATA_REQ, reqLen,
                              &pSendBuffer, &usDataOffset, &usSendSize))||
        (usSendSize < (usDataOffset + reqLen)))
  {
      WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_WARN,
               "Unable to get send buffer in Start Oem Data req %x %x %x",
                 pEventData, pwdiOemDataReqParams, wdiOemDataRspCb);
      WDI_ASSERT(0);
      return WDI_STATUS_E_FAILURE;
  }

  //                                                
  halStartOemDataReqParams = (tStartOemDataReqParams *)(pSendBuffer + usDataOffset );

  wpalMemoryCopy(&halStartOemDataReqParams->selfMacAddr, &pwdiOemDataReqParams->wdiOemDataReqInfo.selfMacAddr, sizeof(wpt_macAddr));
  wpalMemoryCopy(&halStartOemDataReqParams->oemDataReq, &pwdiOemDataReqParams->wdiOemDataReqInfo.oemDataReq, OEM_DATA_REQ_SIZE);

  pWDICtx->wdiReqStatusCB     = pwdiOemDataReqParams->wdiReqStatusCB;
  pWDICtx->pReqStatusUserData = pwdiOemDataReqParams->pUserData;

  /*                                                                         
                             
                                                                           */
  return  WDI_SendMsg( pWDICtx, pSendBuffer, usSendSize,
                        wdiOemDataRspCb, pEventData->pUserData,
                                            WDI_START_OEM_DATA_RESP);
}/*                          */
#endif

/* 
                                                              
                      

                                                         
                                                                    

     
                                    
*/
WDI_Status
WDI_ProcessHostResumeReq
(
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
)
{
  WDI_ResumeParamsType*          pwdiHostResumeParams = NULL;
  WDI_HostResumeEventRspCb       wdiHostResumeRspCb;
  wpt_uint8*                     pSendBuffer         = NULL;
  wpt_uint16                     usDataOffset        = 0;
  wpt_uint16                     usSendSize          = 0;
  tHalWlanHostResumeReqParam     halResumeReqParams;

  /*                                                                        */

  /*                                                                         
              
                                                                           */
  if (( NULL == pEventData ) || ( NULL == pEventData->pEventData ) ||
      ( NULL == pEventData->pCBfnc ))
  {
      WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_WARN,
               "%s: Invalid parameters ",__FUNCTION__);
      WDI_ASSERT(0);
      return WDI_STATUS_E_FAILURE;
  }

   pwdiHostResumeParams = (WDI_ResumeParamsType*)pEventData->pEventData;
   wdiHostResumeRspCb   = (WDI_HostResumeEventRspCb)pEventData->pCBfnc;

  /*                                                                       
                       
                                                                          */
  if (( WDI_STATUS_SUCCESS != WDI_GetMessageBuffer( pWDICtx,
                         WDI_HOST_RESUME_REQ, sizeof(halResumeReqParams),
                              &pSendBuffer, &usDataOffset, &usSendSize))||
        (usSendSize < (usDataOffset + sizeof(halResumeReqParams))))
  {
      WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_WARN,
               "Unable to get send buffer in Start Oem Data req %x %x %x",
                 pEventData, pwdiHostResumeParams, wdiHostResumeRspCb);
      WDI_ASSERT(0);
      return WDI_STATUS_E_FAILURE;
  }

  halResumeReqParams.configuredMcstBcstFilterSetting =
     pwdiHostResumeParams->wdiResumeParams.ucConfiguredMcstBcstFilterSetting;

  wpalMemoryCopy( pSendBuffer+usDataOffset,
                  &halResumeReqParams,
                  sizeof(halResumeReqParams));

  pWDICtx->wdiReqStatusCB     = pwdiHostResumeParams->wdiReqStatusCB;
  pWDICtx->pReqStatusUserData = pwdiHostResumeParams->pUserData;

  /*                                                                         
                             
                                                                           */
  return  WDI_SendMsg( pWDICtx, pSendBuffer, usSendSize,
                        wdiHostResumeRspCb, pEventData->pUserData,
                                            WDI_HOST_RESUME_RESP);
}/*                        */

/* 
                                                                       
                                

                                                         
                                                                    

     
                                    
*/
WDI_Status
WDI_ProcessSetTxPerTrackingReq
(
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
)
{
   WDI_SetTxPerTrackingReqParamsType* pwdiSetTxPerTrackingReqParams = NULL;
   WDI_SetTxPerTrackingRspCb          pwdiSetTxPerTrackingRspCb = NULL;
   wpt_uint8*               pSendBuffer         = NULL;
   wpt_uint16               usDataOffset        = 0;
   wpt_uint16               usSendSize          = 0;
   tHalTxPerTrackingReqParam     halTxPerTrackingReqParam;
   /*                                                                        */

   /*                                                                         
                 
                                                                            */
   if (( NULL == pEventData ) || ( NULL == pEventData->pEventData ) ||
       ( NULL == pEventData->pCBfnc ))
   {
       WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_WARN,
                "%s: Invalid parameters ",__FUNCTION__);
       WDI_ASSERT(0);
       return WDI_STATUS_E_FAILURE;
   }

   pwdiSetTxPerTrackingReqParams = (WDI_SetTxPerTrackingReqParamsType*)pEventData->pEventData;
   pwdiSetTxPerTrackingRspCb   = (WDI_SetTxPerTrackingRspCb)pEventData->pCBfnc;

   /*                                                                       
                       
                                                                    
                                                                          */
   if (( WDI_STATUS_SUCCESS != WDI_GetMessageBuffer( pWDICtx, WDI_SET_TX_PER_TRACKING_REQ,
                         sizeof(halTxPerTrackingReqParam),
                         &pSendBuffer, &usDataOffset, &usSendSize))||
       ( usSendSize < (usDataOffset + sizeof(halTxPerTrackingReqParam) )))
   {
      WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_WARN,
                  "Unable to get send buffer in set tx per tracking req %x %x %x",
                  pEventData, pwdiSetTxPerTrackingReqParams, pwdiSetTxPerTrackingRspCb);
      WDI_ASSERT(0);
      return WDI_STATUS_E_FAILURE;
   }

   halTxPerTrackingReqParam.ucTxPerTrackingEnable = pwdiSetTxPerTrackingReqParams->wdiTxPerTrackingParam.ucTxPerTrackingEnable;
   halTxPerTrackingReqParam.ucTxPerTrackingPeriod = pwdiSetTxPerTrackingReqParams->wdiTxPerTrackingParam.ucTxPerTrackingPeriod;
   halTxPerTrackingReqParam.ucTxPerTrackingRatio = pwdiSetTxPerTrackingReqParams->wdiTxPerTrackingParam.ucTxPerTrackingRatio;
   halTxPerTrackingReqParam.uTxPerTrackingWatermark = pwdiSetTxPerTrackingReqParams->wdiTxPerTrackingParam.uTxPerTrackingWatermark;

   wpalMemoryCopy( pSendBuffer+usDataOffset,
                   &halTxPerTrackingReqParam,
                   sizeof(halTxPerTrackingReqParam));

   pWDICtx->wdiReqStatusCB     = pwdiSetTxPerTrackingReqParams->wdiReqStatusCB;
   pWDICtx->pReqStatusUserData = pwdiSetTxPerTrackingReqParams->pUserData;

   /*                                                                         
                                
                                                                            */
   return  WDI_SendMsg( pWDICtx, pSendBuffer, usSendSize,
                        pwdiSetTxPerTrackingRspCb, pEventData->pUserData, WDI_SET_TX_PER_TRACKING_RESP);
}/*                              */

/*                                                                         
                                        
                                                                         */

/* 
                                                                             

                                                         
                                                                    

     
                                    
*/
WDI_Status
WDI_ProcessHostSuspendInd
(
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
)
{
  WDI_SuspendParamsType          *pSuspendIndParams;
  wpt_uint8*                     pSendBuffer         = NULL;
  wpt_uint16                     usDataOffset        = 0;
  wpt_uint16                     usSendSize          = 0;
  WDI_Status                     wdiStatus;
  tHalWlanHostSuspendIndParam    halWlanSuspendIndparams;
  /*                                                                        */

  /*                                                                         
                 
                                                                           */
  if (( NULL == pEventData ) || ( NULL == pEventData->pEventData ))
  {
      WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_WARN,
               "%s: Invalid parameters in Suspend ind",__FUNCTION__);
      WDI_ASSERT(0);
      return WDI_STATUS_E_FAILURE;
  }

  pSuspendIndParams = (WDI_SuspendParamsType *)pEventData->pEventData;

   /*                                                                       
                       
                                                                          */
  if (( WDI_STATUS_SUCCESS != WDI_GetMessageBuffer( pWDICtx,
                         WDI_HOST_SUSPEND_IND,
                     sizeof(halWlanSuspendIndparams),
                     &pSendBuffer, &usDataOffset, &usSendSize))||
        (usSendSize < (usDataOffset + sizeof(halWlanSuspendIndparams))))
  {
      WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_WARN,
                  "Unable to get send buffer in Suspend Ind ");
      WDI_ASSERT(0);
      return WDI_STATUS_E_FAILURE;
  }

  halWlanSuspendIndparams.configuredMcstBcstFilterSetting =
       pSuspendIndParams->wdiSuspendParams.ucConfiguredMcstBcstFilterSetting;

  halWlanSuspendIndparams.activeSessionCount =
       WDI_GetActiveSessionsCount(pWDICtx);

  wpalMemoryCopy( pSendBuffer+usDataOffset, &halWlanSuspendIndparams,
                                         sizeof(tHalWlanHostSuspendIndParam));

  /*                                                                         
                               
                                                                           */
  pWDICtx->wdiReqStatusCB     = pSuspendIndParams->wdiReqStatusCB;
  pWDICtx->pReqStatusUserData = pSuspendIndParams->pUserData;

  wdiStatus = WDI_SendIndication( pWDICtx, pSendBuffer, usSendSize);
  return  ( wdiStatus != WDI_STATUS_SUCCESS )?wdiStatus:WDI_STATUS_SUCCESS_SYNC;
}/*                         */

/*                                                                          
                                                     
                                                                          */
/* 
                                                            
                           

                                                         
                                                                    

     
                                    
*/
WDI_Status
WDI_ProcessChannelSwitchReq
(
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
)
{
  WDI_SwitchChReqParamsType*   pwdiSwitchChParams;
  WDI_SwitchChRspCb            wdiSwitchChRspCb;
  wpt_uint8*                   pSendBuffer         = NULL;
  wpt_uint16                   usDataOffset        = 0;
  wpt_uint16                   usSendSize          = 0;
  tSwitchChannelReqMsg         halSwitchChannelReq = {{0}};
  /*                                                                        */

  /*                                                                         
                
                                                                           */
  if (( NULL == pEventData ) || ( NULL == pEventData->pEventData ) ||
      ( NULL == pEventData->pCBfnc ))
  {
     WPAL_TRACE( eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_WARN,
                 "%s: Invalid parameters", __FUNCTION__);
     WDI_ASSERT(0);
     return WDI_STATUS_E_FAILURE;
  }

  pwdiSwitchChParams = (WDI_SwitchChReqParamsType*)pEventData->pEventData;
  wdiSwitchChRspCb   = (WDI_SwitchChRspCb)pEventData->pCBfnc;
  /*                                                                       
                      
                                                                   
                                                                         */
  if (( WDI_STATUS_SUCCESS != WDI_GetMessageBuffer( pWDICtx, WDI_CH_SWITCH_REQ,
                        sizeof(halSwitchChannelReq.switchChannelParams),
                        &pSendBuffer, &usDataOffset, &usSendSize))||
      ( usSendSize < (usDataOffset + sizeof(halSwitchChannelReq.switchChannelParams) )))
  {
     WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_WARN,
              "Unable to get send buffer in channel switch req %x %x %x",
                pEventData, pwdiSwitchChParams, wdiSwitchChRspCb);
     WDI_ASSERT(0);
     return WDI_STATUS_E_FAILURE;
  }

  halSwitchChannelReq.switchChannelParams.channelNumber =
                       pwdiSwitchChParams->wdiChInfo.ucChannel;
#ifndef WLAN_FEATURE_VOWIFI
  halSwitchChannelReq.switchChannelParams.localPowerConstraint =
                       pwdiSwitchChParams->wdiChInfo.ucLocalPowerConstraint;
#endif
  halSwitchChannelReq.switchChannelParams.secondaryChannelOffset =
                       pwdiSwitchChParams->wdiChInfo.wdiSecondaryChannelOffset;

#ifdef WLAN_FEATURE_VOWIFI
  halSwitchChannelReq.switchChannelParams.maxTxPower
                            = pwdiSwitchChParams->wdiChInfo.cMaxTxPower;
  wpalMemoryCopy(halSwitchChannelReq.switchChannelParams.selfStaMacAddr,
                  pwdiSwitchChParams->wdiChInfo.macSelfStaMacAddr,
                  WDI_MAC_ADDR_LEN);
  wpalMemoryCopy(halSwitchChannelReq.switchChannelParams.bssId,
                  pwdiSwitchChParams->wdiChInfo.macBSSId,
                  WDI_MAC_ADDR_LEN);
#endif
  wpalMemoryCopy( pSendBuffer+usDataOffset,
                  &halSwitchChannelReq.switchChannelParams,
                  sizeof(halSwitchChannelReq.switchChannelParams));

  pWDICtx->wdiReqStatusCB     = pwdiSwitchChParams->wdiReqStatusCB;
  pWDICtx->pReqStatusUserData = pwdiSwitchChParams->pUserData;

  /*                                                                         
                                      
                                                                           */
  return  WDI_SendMsg( pWDICtx, pSendBuffer, usSendSize,
                       wdiSwitchChRspCb, pEventData->pUserData, WDI_CH_SWITCH_RESP);
}/*                           */

/* 
                                                                 
                  

                                                         
                                                                    

     
                                    
*/
WDI_Status
WDI_ProcessConfigStaReq
(
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
)
{
  WDI_ConfigSTAReqParamsType*  pwdiConfigSTAParams;
  WDI_ConfigSTARspCb           wdiConfigSTARspCb;
  wpt_uint8                    ucCurrentBSSSesIdx  = 0;
  WDI_BSSSessionType*          pBSSSes             = NULL;
  wpt_uint8*                   pSendBuffer         = NULL;
  wpt_uint16                   usDataOffset        = 0;
  wpt_uint16                   usSendSize          = 0;
  WDI_Status                   wdiStatus           = WDI_STATUS_SUCCESS;

  tConfigStaReqMsg             halConfigStaReqMsg;
  wpt_uint16                   uMsgSize            = 0;
  /*                                                                        */

  /*                                                                         
                
                                                                           */
  if (( NULL == pEventData ) || ( NULL == pEventData->pEventData ) ||
      ( NULL == pEventData->pCBfnc ))
  {
     WPAL_TRACE( eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_WARN,
                 "%s: Invalid parameters", __FUNCTION__);
     WDI_ASSERT(0);
     return WDI_STATUS_E_FAILURE;
  }

  pwdiConfigSTAParams = (WDI_ConfigSTAReqParamsType*)pEventData->pEventData;
  wdiConfigSTARspCb   = (WDI_ConfigSTARspCb)pEventData->pCBfnc;
  /*                                                                         
                                                                           
                                              
                                                                           */
  wpalMutexAcquire(&pWDICtx->wptMutex);

  /*                                                                        
                                              
                                                                          */
  ucCurrentBSSSesIdx = WDI_FindAssocSession( pWDICtx,
                           pwdiConfigSTAParams->wdiReqInfo.macBSSID,
                          &pBSSSes);

  if ( NULL == pBSSSes )
  {
    WPAL_TRACE( eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_ERROR,
          "%s: Association sequence for this BSS does not yet exist. macBSSID " MAC_ADDRESS_STR,
          __func__, MAC_ADDR_ARRAY(pwdiConfigSTAParams->wdiReqInfo.macBSSID));

    wpalMutexRelease(&pWDICtx->wptMutex);
    return WDI_STATUS_E_NOT_ALLOWED;
  }

  /*                                                                        
                                                             
                                             
                                                                          */
  if ( eWLAN_PAL_TRUE == pBSSSes->bAssocReqQueued )
  {
    WPAL_TRACE( eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_WARN,
              "%s: Association sequence for this BSS exists but currently queued. macBSSID " MAC_ADDRESS_STR,
              __func__, MAC_ADDR_ARRAY(pwdiConfigSTAParams->wdiReqInfo.macBSSID));

    wdiStatus = WDI_QueueAssocRequest( pWDICtx, pBSSSes, pEventData);
    wpalMutexRelease(&pWDICtx->wptMutex);
    return wdiStatus;
  }

  wpalMutexRelease(&pWDICtx->wptMutex);

  /*                                                               */
#ifdef WLAN_FEATURE_11AC
  if (WDI_getFwWlanFeatCaps(DOT11AC))
	  uMsgSize = sizeof(halConfigStaReqMsg.uStaParams.configStaParams_V1); //                   
  else
#endif
	  uMsgSize = sizeof(halConfigStaReqMsg.uStaParams.configStaParams); //                  

  /*                                                                       
                      
                                                                         */
  if (( WDI_STATUS_SUCCESS != WDI_GetMessageBuffer( pWDICtx, WDI_CONFIG_STA_REQ,
                        uMsgSize,
                        &pSendBuffer, &usDataOffset, &usSendSize))||
      ( usSendSize < (usDataOffset + uMsgSize )))
  {
     WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_WARN,
              "Unable to get send buffer in config sta req %x %x %x",
                pEventData, pwdiConfigSTAParams, wdiConfigSTARspCb);
     WDI_ASSERT(0);
     return WDI_STATUS_E_FAILURE;
  }

  /*                        */
  WDI_CopyWDIStaCtxToHALStaCtx( &halConfigStaReqMsg.uStaParams.configStaParams,
                                &pwdiConfigSTAParams->wdiReqInfo);

  if(pwdiConfigSTAParams->wdiReqInfo.wdiSTAType == WDI_STA_ENTRY_SELF)
  {
    /*                                    */
    if ( WDI_STATUS_SUCCESS !=
         WDI_STATableFindStaidByAddr(pWDICtx,
                                     pwdiConfigSTAParams->wdiReqInfo.macSTA,
                                     (wpt_uint8*)&halConfigStaReqMsg.uStaParams.configStaParams.staIdx ))
    {
      WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_WARN,
               "This station does not exist in the WDI Station Table %d");
      wpalMutexRelease(&pWDICtx->wptMutex);
      return WDI_STATUS_E_FAILURE;
    }
  }
  else
  {
  /*                                                                          
                              */
    halConfigStaReqMsg.uStaParams.configStaParams.staIdx = WDI_STA_INVALID_IDX;
  }

  /*                               */
  halConfigStaReqMsg.uStaParams.configStaParams.bssIdx = pBSSSes->ucBSSIdx;
  
  wpalMemoryCopy( pSendBuffer+usDataOffset,
                  &halConfigStaReqMsg.uStaParams,
                  uMsgSize);

  pWDICtx->wdiReqStatusCB     = pwdiConfigSTAParams->wdiReqStatusCB;
  pWDICtx->pReqStatusUserData = pwdiConfigSTAParams->pUserData;

  wpalMemoryCopy( &pWDICtx->wdiCachedConfigStaReq,
                  pwdiConfigSTAParams,
                  sizeof(pWDICtx->wdiCachedConfigStaReq));

  /*                                                                         
                                  
                                                                           */
  return  WDI_SendMsg( pWDICtx, pSendBuffer, usSendSize,
                       wdiConfigSTARspCb, pEventData->pUserData, WDI_CONFIG_STA_RESP);
}/*                       */


/* 
                                                            
                           

                                                         
                                                                    

     
                                    
*/
WDI_Status
WDI_ProcessSetLinkStateReq
(
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
)
{
  WDI_SetLinkReqParamsType*    pwdiSetLinkParams;
  WDI_SetLinkStateRspCb        wdiSetLinkRspCb;
  wpt_uint8                    ucCurrentBSSSesIdx  = 0;
  WDI_BSSSessionType*          pBSSSes             = NULL;
  wpt_uint8*                   pSendBuffer         = NULL;
  wpt_uint16                   usDataOffset        = 0;
  wpt_uint16                   usSendSize          = 0;
  WDI_Status                   wdiStatus           = WDI_STATUS_SUCCESS;
  tLinkStateParams             halLinkStateReqMsg;
  /*                                                                        */

  /*                                                                         
                
                                                                           */
  if (( NULL == pEventData ) || ( NULL == pEventData->pEventData ) ||
      ( NULL == pEventData->pCBfnc ))
  {
     WPAL_TRACE( eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_WARN,
                 "%s: Invalid parameters", __FUNCTION__);
     WDI_ASSERT(0);
     return WDI_STATUS_E_FAILURE;
  }

  pwdiSetLinkParams = (WDI_SetLinkReqParamsType*)pEventData->pEventData;
  wdiSetLinkRspCb   = (WDI_SetLinkStateRspCb)pEventData->pCBfnc;
  /*                                                                         
                                                                           
                                              
                                                                           */
  wpalMutexAcquire(&pWDICtx->wptMutex);

  /*                                                                        
                                              
                                                                          */
  ucCurrentBSSSesIdx = WDI_FindAssocSession( pWDICtx,
                           pwdiSetLinkParams->wdiLinkInfo.macBSSID,
                          &pBSSSes);

  if ( NULL == pBSSSes )
  {
     WPAL_TRACE( eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_INFO, 
     "%s: Set link request received outside association session. macBSSID " MAC_ADDRESS_STR, 
     __func__, MAC_ADDR_ARRAY(pwdiSetLinkParams->wdiLinkInfo.macBSSID));
  }
  else
  {
    /*                                                                        
                                                               
                                               
                                                                            */
    if ( eWLAN_PAL_TRUE == pBSSSes->bAssocReqQueued )
    {
      WPAL_TRACE( eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_WARN,
                "%s: Association sequence for this BSS exists but currently queued. macBSSID " MAC_ADDRESS_STR,
                __func__, MAC_ADDR_ARRAY(pwdiSetLinkParams->wdiLinkInfo.macBSSID));

      wdiStatus = WDI_QueueAssocRequest( pWDICtx, pBSSSes, pEventData);
      wpalMutexRelease(&pWDICtx->wptMutex);
      return wdiStatus;
    }
  }
  /*                                                                      
                                                                   
                                               */
  wpalMemoryCopy(&pWDICtx->wdiCacheSetLinkStReq, pwdiSetLinkParams,
                 sizeof(pWDICtx->wdiCacheSetLinkStReq));

  wpalMutexRelease(&pWDICtx->wptMutex);
  /*                                                                       
                      
                                                                   
                                                                         */

  if (( WDI_STATUS_SUCCESS != WDI_GetMessageBuffer( pWDICtx, WDI_SET_LINK_ST_REQ,
                        sizeof(halLinkStateReqMsg),
                        &pSendBuffer, &usDataOffset, &usSendSize))||
      ( usSendSize < (usDataOffset + sizeof(halLinkStateReqMsg) )))
  {
     WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_WARN,
              "Unable to get send buffer in set bss key req %x %x %x",
                pEventData, pwdiSetLinkParams, wdiSetLinkRspCb);
     WDI_ASSERT(0);
     return WDI_STATUS_E_FAILURE;
  }

  wpalMemoryCopy(halLinkStateReqMsg.bssid,
                 pwdiSetLinkParams->wdiLinkInfo.macBSSID, WDI_MAC_ADDR_LEN);

  wpalMemoryCopy(halLinkStateReqMsg.selfMacAddr,
                 pwdiSetLinkParams->wdiLinkInfo.macSelfStaMacAddr, WDI_MAC_ADDR_LEN);

  halLinkStateReqMsg.state =
     WDI_2_HAL_LINK_STATE(pwdiSetLinkParams->wdiLinkInfo.wdiLinkState);

  wpalMemoryCopy( pSendBuffer+usDataOffset,
                  &halLinkStateReqMsg,
                  sizeof(halLinkStateReqMsg));

  pWDICtx->wdiReqStatusCB     = pwdiSetLinkParams->wdiReqStatusCB;
  pWDICtx->pReqStatusUserData = pwdiSetLinkParams->pUserData;

  /*                                                                         
                                      
                                                                           */
  return  WDI_SendMsg( pWDICtx, pSendBuffer, usSendSize,
                       wdiSetLinkRspCb, pEventData->pUserData, WDI_SET_LINK_ST_RESP);
}/*                          */


/* 
                                                                
                  

                                                         
                                                                    

     
                                    
*/
WDI_Status
WDI_ProcessGetStatsReq
(
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
)
{
  WDI_GetStatsReqParamsType*   pwdiGetStatsParams;
  WDI_GetStatsRspCb            wdiGetStatsRspCb;
  wpt_uint8*                   pSendBuffer         = NULL;
  wpt_uint16                   usDataOffset        = 0;
  wpt_uint16                   usSendSize          = 0;
  wpt_uint8                    ucCurrentBSSSesIdx  = 0;
  WDI_BSSSessionType*          pBSSSes             = NULL;
  wpt_macAddr                  macBSSID;
  WDI_Status                   wdiStatus           = WDI_STATUS_SUCCESS;
  tHalStatsReqMsg              halStatsReqMsg;
  /*                                                                        */

  /*                                                                         
                
                                                                           */
  if (( NULL == pEventData ) || ( NULL == pEventData->pEventData) ||
      ( NULL == pEventData->pCBfnc ) )
  {
     WPAL_TRACE( eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_WARN,
                 "%s: Invalid parameters", __FUNCTION__);
     WDI_ASSERT(0);
     return WDI_STATUS_E_FAILURE;
  }

  pwdiGetStatsParams = (WDI_GetStatsReqParamsType*)pEventData->pEventData;
  wdiGetStatsRspCb   = (WDI_GetStatsRspCb)pEventData->pCBfnc;

  /*                                                                         
                                                                           
                                              
                                                                           */
  wpalMutexAcquire(&pWDICtx->wptMutex);

  /*                                                                        
                                              
                                                                          */
  if ( WDI_STATUS_SUCCESS != WDI_STATableGetStaBSSIDAddr(pWDICtx,
                        pwdiGetStatsParams->wdiGetStatsParamsInfo.ucSTAIdx,
                        &macBSSID))
  {
    WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_WARN,
             "This station does not exist in the WDI Station Table %d");
    wpalMutexRelease(&pWDICtx->wptMutex);
    return WDI_STATUS_E_FAILURE;
  }

  ucCurrentBSSSesIdx = WDI_FindAssocSession( pWDICtx, macBSSID, &pBSSSes);
  if ( NULL == pBSSSes )
  {
    WPAL_TRACE( eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_ERROR,
        "%s: Association sequence for this BSS does not yet exist. macBSSID " MAC_ADDRESS_STR,
        __func__, MAC_ADDR_ARRAY(macBSSID));

    wpalMutexRelease(&pWDICtx->wptMutex);
    return WDI_STATUS_E_NOT_ALLOWED;
  }

  /*                                                                        
                                                             
                                             
                                                                          */
  if ( eWLAN_PAL_TRUE == pBSSSes->bAssocReqQueued )
  {
    WPAL_TRACE( eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_WARN,
              "%s: Association sequence for this BSS exists but currently queued. macBSSID " MAC_ADDRESS_STR,
              __func__, MAC_ADDR_ARRAY(macBSSID));

    wdiStatus = WDI_QueueAssocRequest( pWDICtx, pBSSSes, pEventData);
    wpalMutexRelease(&pWDICtx->wptMutex);
    return wdiStatus;
  }


  wpalMutexRelease(&pWDICtx->wptMutex);

  /*                                                                       
                      
                                                                         */
  if (( WDI_STATUS_SUCCESS != WDI_GetMessageBuffer( pWDICtx, WDI_GET_STATS_REQ,
                        sizeof(halStatsReqMsg.statsReqParams),
                        &pSendBuffer, &usDataOffset, &usSendSize))||
      ( usSendSize < (usDataOffset + sizeof(halStatsReqMsg.statsReqParams) )))
  {
     WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_WARN,
              "Unable to get send buffer in set bss key req %x %x %x",
                pEventData, pwdiGetStatsParams, wdiGetStatsRspCb);
     WDI_ASSERT(0);
     return WDI_STATUS_E_FAILURE;
  }

  halStatsReqMsg.statsReqParams.staId =
                  pwdiGetStatsParams->wdiGetStatsParamsInfo.ucSTAIdx;
  halStatsReqMsg.statsReqParams.statsMask =
                  pwdiGetStatsParams->wdiGetStatsParamsInfo.uStatsMask;
  wpalMemoryCopy( pSendBuffer+usDataOffset,
                  &halStatsReqMsg.statsReqParams,
                  sizeof(halStatsReqMsg.statsReqParams));

  pWDICtx->wdiReqStatusCB     = pwdiGetStatsParams->wdiReqStatusCB;
  pWDICtx->pReqStatusUserData = pwdiGetStatsParams->pUserData;

  /*                                                                         
                               
                                                                           */
  return  WDI_SendMsg( pWDICtx, pSendBuffer, usSendSize,
                       wdiGetStatsRspCb, pEventData->pUserData, WDI_GET_STATS_RESP);
}/*                      */

/* 
                                                             
                      

                                                         
                                                                    

     
                                    
*/
WDI_Status
WDI_ProcessUpdateCfgReq
(
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
)
{
  WDI_UpdateCfgReqParamsType*  pwdiUpdateCfgParams = NULL;
  WDI_UpdateCfgRspCb           wdiUpdateCfgRspCb = NULL;

  wpt_uint8*                   pSendBuffer         = NULL;
  wpt_uint16                   usDataOffset         = 0;
  wpt_uint16                   usSendSize          = 0;
  /*                                                                        */

  /*                                                                         
                
                                                                           */
  if (( NULL == pEventData ) || ( NULL == pEventData->pEventData ) ||
      ( NULL == pEventData->pCBfnc))
  {
     WPAL_TRACE( eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_WARN,
                 "%s: Invalid parameters", __FUNCTION__);
     WDI_ASSERT(0);
     return WDI_STATUS_E_FAILURE;
  }

  pwdiUpdateCfgParams = (WDI_UpdateCfgReqParamsType*)pEventData->pEventData;
  wdiUpdateCfgRspCb   = (WDI_UpdateCfgRspCb)pEventData->pCBfnc;

  /*                                                                       
                      
                                                                   
                                                                         */

  if (( WDI_STATUS_SUCCESS != WDI_GetMessageBuffer( pWDICtx, WDI_UPDATE_CFG_REQ,
                        pwdiUpdateCfgParams->uConfigBufferLen + sizeof(wpt_uint32),
                        &pSendBuffer, &usDataOffset, &usSendSize))||
      ( usSendSize < (usDataOffset +  pwdiUpdateCfgParams->uConfigBufferLen)))
  {
     WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_WARN,
              "Unable to get send buffer in set bss key req %x %x %x",
                pEventData, pwdiUpdateCfgParams, wdiUpdateCfgRspCb);
     WDI_ASSERT(0);
     return WDI_STATUS_E_FAILURE;
  }

  wpalMemoryCopy( pSendBuffer+usDataOffset,
                  &pwdiUpdateCfgParams->uConfigBufferLen,
                  sizeof(wpt_uint32));
  wpalMemoryCopy( pSendBuffer+usDataOffset+sizeof(wpt_uint32),
                  pwdiUpdateCfgParams->pConfigBuffer,
                  pwdiUpdateCfgParams->uConfigBufferLen);

  pWDICtx->wdiReqStatusCB     = pwdiUpdateCfgParams->wdiReqStatusCB;
  pWDICtx->pReqStatusUserData = pwdiUpdateCfgParams->pUserData;

  /*                                                                         
                                  
                                                                           */

  return  WDI_SendMsg( pWDICtx, pSendBuffer, usSendSize,
                       wdiUpdateCfgRspCb, pEventData->pUserData, WDI_UPDATE_CFG_RESP);

}/*                       */


/* 
                                                             
                  

                                                         
                                                                    

     
                                    
*/
WDI_Status
WDI_ProcessAddBAReq
(
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
)
{
  WDI_AddBAReqParamsType*  pwdiAddBAParams;
  WDI_AddBARspCb           wdiAddBARspCb;
  wpt_uint8                ucCurrentBSSSesIdx  = 0;
  WDI_BSSSessionType*      pBSSSes             = NULL;
  wpt_uint8*               pSendBuffer         = NULL;
  wpt_uint16               usDataOffset        = 0;
  wpt_uint16               usSendSize          = 0;
  WDI_Status               wdiStatus           = WDI_STATUS_SUCCESS;
  wpt_macAddr              macBSSID;

  tAddBAReqMsg             halAddBAReq;
  /*                                                                        */

  /*                                                                         
                
                                                                           */
  if (( NULL == pEventData ) ||
      ( NULL == pEventData->pEventData) ||
      ( NULL == pEventData->pCBfnc ))
  {
     WPAL_TRACE( eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_WARN,
                 "%s: Invalid parameters", __FUNCTION__);
     WDI_ASSERT(0);
     return WDI_STATUS_E_FAILURE;
  }

  pwdiAddBAParams = (WDI_AddBAReqParamsType*)pEventData->pEventData;
  wdiAddBARspCb   = (WDI_AddBARspCb)pEventData->pCBfnc;

  /*                                                                         
                                                                           
                                              
                                                                           */
  wpalMutexAcquire(&pWDICtx->wptMutex);

  /*                                                                        
                                              
                                                                          */
  if ( WDI_STATUS_SUCCESS != WDI_STATableGetStaBSSIDAddr(pWDICtx,
                                  pwdiAddBAParams->wdiBAInfoType.ucSTAIdx,
                                  &macBSSID))
  {
    WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_WARN,
             "This station does not exist in the WDI Station Table %d");
    wpalMutexRelease(&pWDICtx->wptMutex);
    return WDI_STATUS_E_FAILURE;
  }

  ucCurrentBSSSesIdx = WDI_FindAssocSession( pWDICtx, macBSSID, &pBSSSes);
  if ( NULL == pBSSSes )
  {
      WPAL_TRACE( eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_ERROR,
            "%s: Association sequence for this BSS does not yet exist. macBSSID " MAC_ADDRESS_STR,
            __func__, MAC_ADDR_ARRAY(macBSSID));

    wpalMutexRelease(&pWDICtx->wptMutex);
    return WDI_STATUS_E_NOT_ALLOWED;
  }

  /*                                                                        
                                                             
                                             
                                                                          */
  if ( eWLAN_PAL_TRUE == pBSSSes->bAssocReqQueued )
  {
    WPAL_TRACE( eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_WARN,
              "%s: Association sequence for this BSS exists but currently queued. macBSSID " MAC_ADDRESS_STR,
              __func__, MAC_ADDR_ARRAY(macBSSID));

    wdiStatus = WDI_QueueAssocRequest( pWDICtx, pBSSSes, pEventData);
    wpalMutexRelease(&pWDICtx->wptMutex);
    return wdiStatus;
  }


  wpalMutexRelease(&pWDICtx->wptMutex);
  /*                                                                       
                      
                                                                         */
  if (( WDI_STATUS_SUCCESS != WDI_GetMessageBuffer( pWDICtx, WDI_ADD_BA_REQ,
                        sizeof(halAddBAReq.addBAParams),
                        &pSendBuffer, &usDataOffset, &usSendSize))||
      ( usSendSize <
            (usDataOffset + sizeof(halAddBAReq.addBAParams) )))
  {
     WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_WARN,
              "Unable to get send buffer in Add BA req %x %x %x",
                pEventData, pwdiAddBAParams, wdiAddBARspCb);
     WDI_ASSERT(0);
     return WDI_STATUS_E_FAILURE;
  }

  halAddBAReq.addBAParams.baSessionID =
                             pwdiAddBAParams->wdiBAInfoType.ucBaSessionID;
  halAddBAReq.addBAParams.winSize = pwdiAddBAParams->wdiBAInfoType.ucWinSize;
#ifdef FEATURE_ON_CHIP_REORDERING
  halAddBAReq.addBAParams.isReorderingDoneOnChip =
                       pwdiAddBAParams->wdiBAInfoType.bIsReorderingDoneOnChip;
#endif

  wpalMemoryCopy( pSendBuffer+usDataOffset,
                  &halAddBAReq.addBAParams,
                  sizeof(halAddBAReq.addBAParams));

  pWDICtx->wdiReqStatusCB     = pwdiAddBAParams->wdiReqStatusCB;
  pWDICtx->pReqStatusUserData = pwdiAddBAParams->pUserData;

  /*                                                                         
                             
                                                                           */
  return  WDI_SendMsg( pWDICtx, pSendBuffer, usSendSize,
                        wdiAddBARspCb, pEventData->pUserData,
                        WDI_ADD_BA_RESP);
}/*                   */



/* 
                                                                 
                  

                                                         
                                                                    

     
                                    
*/
WDI_Status
WDI_ProcessTriggerBAReq
(
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
)
{
  WDI_TriggerBAReqParamsType*  pwdiTriggerBAParams;
  WDI_TriggerBARspCb           wdiTriggerBARspCb;
  wpt_uint8                    ucCurrentBSSSesIdx  = 0;
  WDI_BSSSessionType*          pBSSSes             = NULL;
  wpt_uint8*                   pSendBuffer         = NULL;
  wpt_uint16                   usDataOffset        = 0;
  wpt_uint16                   usSendSize          = 0;
  WDI_Status                   wdiStatus           = WDI_STATUS_SUCCESS;
  wpt_uint16                   index;
  wpt_macAddr                  macBSSID;

  tTriggerBAReqMsg               halTriggerBAReq;
  tTriggerBaReqCandidate*        halTriggerBACandidate;
  WDI_TriggerBAReqCandidateType* wdiTriggerBACandidate;
  /*                                                                        */

  /*                                                                         
                
                                                                           */
  if (( NULL == pEventData ) ||
      ( NULL == pEventData->pEventData ) ||
      ( NULL == pEventData->pCBfnc ))
  {
     WPAL_TRACE( eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_WARN,
                 "%s: Invalid parameters", __FUNCTION__);
     WDI_ASSERT(0);
     return WDI_STATUS_E_FAILURE;
  }

  pwdiTriggerBAParams = (WDI_TriggerBAReqParamsType*)pEventData->pEventData;
  wdiTriggerBARspCb = (WDI_TriggerBARspCb)pEventData->pCBfnc;
  /*                                                                         
                                                                           
                                              
                                                                           */
  wpalMutexAcquire(&pWDICtx->wptMutex);

  /*                                                                        
                                              
                                                                          */
  if ( WDI_STATUS_SUCCESS != WDI_STATableGetStaBSSIDAddr(pWDICtx,
                                  pwdiTriggerBAParams->wdiTriggerBAInfoType.ucSTAIdx,
                                  &macBSSID))
  {
    WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_WARN,
             "This station does not exist in the WDI Station Table %d");
    wpalMutexRelease(&pWDICtx->wptMutex);
    return WDI_STATUS_E_FAILURE;
  }

  ucCurrentBSSSesIdx = WDI_FindAssocSession( pWDICtx, macBSSID, &pBSSSes);
  if ( NULL == pBSSSes )
  {
    WPAL_TRACE( eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_ERROR,
        "%s: Association sequence for this BSS does not yet exist. macBSSID " MAC_ADDRESS_STR,
        __func__, MAC_ADDR_ARRAY(macBSSID));

    wpalMutexRelease(&pWDICtx->wptMutex);
    return WDI_STATUS_E_NOT_ALLOWED;
  }

  /*                                                                        
                                                             
                                             
                                                                          */
  if ( eWLAN_PAL_TRUE == pBSSSes->bAssocReqQueued )
  {
    WPAL_TRACE( eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_WARN,
              "%s: Association sequence for this BSS exists but currently queued. macBSSID " MAC_ADDRESS_STR,
              __func__, MAC_ADDR_ARRAY(macBSSID));

    wdiStatus = WDI_QueueAssocRequest( pWDICtx, pBSSSes, pEventData);
    wpalMutexRelease(&pWDICtx->wptMutex);
    return wdiStatus;
  }


  wpalMutexRelease(&pWDICtx->wptMutex);
  /*                                                                       
                      
                                                                         */
  if (( WDI_STATUS_SUCCESS != WDI_GetMessageBuffer( pWDICtx,
                  WDI_TRIGGER_BA_REQ,
                  sizeof(halTriggerBAReq.triggerBAParams) +
                  (sizeof(tTriggerBaReqCandidate) *
                  pwdiTriggerBAParams->wdiTriggerBAInfoType.usBACandidateCnt),
                  &pSendBuffer, &usDataOffset, &usSendSize))||
      ( usSendSize <
            (usDataOffset + sizeof(halTriggerBAReq.triggerBAParams)+
               (sizeof(tTriggerBaReqCandidate) *
               pwdiTriggerBAParams->wdiTriggerBAInfoType.usBACandidateCnt) )))
  {
     WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_WARN,
              "Unable to get send buffer in Trigger BA req %x %x %x",
                pEventData, pwdiTriggerBAParams, wdiTriggerBARspCb);
     WDI_ASSERT(0);
     return WDI_STATUS_E_FAILURE;
  }

  halTriggerBAReq.triggerBAParams.baSessionID =
                  pwdiTriggerBAParams->wdiTriggerBAInfoType.ucBASessionID;
  halTriggerBAReq.triggerBAParams.baCandidateCnt =
                  pwdiTriggerBAParams->wdiTriggerBAInfoType.usBACandidateCnt;

  wpalMemoryCopy( pSendBuffer+usDataOffset,
                  &halTriggerBAReq.triggerBAParams,
                  sizeof(halTriggerBAReq.triggerBAParams));

  wdiTriggerBACandidate =
    (WDI_TriggerBAReqCandidateType*)(pwdiTriggerBAParams + 1);
  halTriggerBACandidate = (tTriggerBaReqCandidate*)(pSendBuffer+usDataOffset+
                                 sizeof(halTriggerBAReq.triggerBAParams));

  for(index = 0 ; index < halTriggerBAReq.triggerBAParams.baCandidateCnt ;
                                                                     index++)
  {
    halTriggerBACandidate->staIdx = wdiTriggerBACandidate->ucSTAIdx;
    halTriggerBACandidate->tidBitmap = wdiTriggerBACandidate->ucTidBitmap;
    halTriggerBACandidate++;
    wdiTriggerBACandidate++;
  }

  pWDICtx->wdiReqStatusCB     = pwdiTriggerBAParams->wdiReqStatusCB;
  pWDICtx->pReqStatusUserData = pwdiTriggerBAParams->pUserData;

  /*                                                                         
                             
                                                                           */
  return  WDI_SendMsg( pWDICtx, pSendBuffer, usSendSize,
                        wdiTriggerBARspCb, pEventData->pUserData,
                        WDI_TRIGGER_BA_RESP);
}/*                       */



/* 
                                                                            
                  

                                                         
                                                                    

     
                                    
*/
WDI_Status
WDI_ProcessUpdateBeaconParamsReq
(
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
)
{
  WDI_UpdateBeaconParamsType*  pwdiUpdateBeaconParams;
  WDI_UpdateBeaconParamsRspCb  wdiUpdateBeaconParamsRspCb;
  wpt_uint8*                   pSendBuffer         = NULL;
  wpt_uint16                   usDataOffset        = 0;
  wpt_uint16                   usSendSize          = 0;
  tUpdateBeaconParams          halUpdateBeaconParams;
  /*                                                                        */

  /*                                                                         
                
                                                                           */
  if (( NULL == pEventData ) ||
      ( NULL == pEventData->pEventData) ||
      ( NULL == pEventData->pCBfnc))
  {
     WPAL_TRACE( eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_WARN,
                 "%s: Invalid parameters", __FUNCTION__);
     WDI_ASSERT(0);
     return WDI_STATUS_E_FAILURE;
  }

  pwdiUpdateBeaconParams = (WDI_UpdateBeaconParamsType*)pEventData->pEventData;
  wdiUpdateBeaconParamsRspCb = (WDI_UpdateBeaconParamsRspCb)pEventData->pCBfnc;
  /*                                                                       
                      
                                                                         */
  if (( WDI_STATUS_SUCCESS != WDI_GetMessageBuffer( pWDICtx, WDI_UPD_BCON_PRMS_REQ,
                        sizeof(halUpdateBeaconParams),
                        &pSendBuffer, &usDataOffset, &usSendSize))||
      ( usSendSize < (usDataOffset + sizeof(halUpdateBeaconParams) )))
  {
     WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_WARN,
              "Unable to get send buffer in set bss key req %x %x %x",
                pEventData, pwdiUpdateBeaconParams, wdiUpdateBeaconParamsRspCb);
     WDI_ASSERT(0);
     return WDI_STATUS_E_FAILURE;
  }

  /*                    */
  halUpdateBeaconParams.bssIdx =
    pwdiUpdateBeaconParams->wdiUpdateBeaconParamsInfo.ucBssIdx;
  /*                                                               
                         */
  halUpdateBeaconParams.fShortPreamble =
    pwdiUpdateBeaconParams->wdiUpdateBeaconParamsInfo.ucfShortPreamble;
  /*                 */
  halUpdateBeaconParams.fShortSlotTime =
    pwdiUpdateBeaconParams->wdiUpdateBeaconParamsInfo.ucfShortSlotTime;
  /*                 */
  halUpdateBeaconParams.beaconInterval =
    pwdiUpdateBeaconParams->wdiUpdateBeaconParamsInfo.usBeaconInterval;

  /*                   */
  halUpdateBeaconParams.llaCoexist =
    pwdiUpdateBeaconParams->wdiUpdateBeaconParamsInfo.ucllaCoexist;
  halUpdateBeaconParams.llbCoexist =
    pwdiUpdateBeaconParams->wdiUpdateBeaconParamsInfo.ucllbCoexist;
  halUpdateBeaconParams.llgCoexist =
    pwdiUpdateBeaconParams->wdiUpdateBeaconParamsInfo.ucllgCoexist;
  halUpdateBeaconParams.ht20MhzCoexist  =
    pwdiUpdateBeaconParams->wdiUpdateBeaconParamsInfo.ucHt20MhzCoexist;
  halUpdateBeaconParams.llnNonGFCoexist =
    pwdiUpdateBeaconParams->wdiUpdateBeaconParamsInfo.ucllnNonGFCoexist;
  halUpdateBeaconParams.fLsigTXOPProtectionFullSupport =
    pwdiUpdateBeaconParams->wdiUpdateBeaconParamsInfo.ucfLsigTXOPProtectionFullSupport;
  halUpdateBeaconParams.fRIFSMode =
    pwdiUpdateBeaconParams->wdiUpdateBeaconParamsInfo.ucfRIFSMode;
  halUpdateBeaconParams.paramChangeBitmap =
    pwdiUpdateBeaconParams->wdiUpdateBeaconParamsInfo.usChangeBitmap;

  wpalMemoryCopy( pSendBuffer+usDataOffset, &halUpdateBeaconParams,
                  sizeof(halUpdateBeaconParams));

  pWDICtx->wdiReqStatusCB     = pwdiUpdateBeaconParams->wdiReqStatusCB;
  pWDICtx->pReqStatusUserData = pwdiUpdateBeaconParams->pUserData;

  /*                                                                         
                              
                                                                           */
  return  WDI_SendMsg( pWDICtx, pSendBuffer, usSendSize,
                       wdiUpdateBeaconParamsRspCb, pEventData->pUserData, WDI_UPD_BCON_PRMS_RESP);
}/*                                */



/* 
                                                                            
                  

                                                         
                                                                    

     
                                    
*/
WDI_Status
WDI_ProcessSendBeaconParamsReq
(
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
)
{
  WDI_SendBeaconParamsType*    pwdiSendBeaconParams;
  WDI_SendBeaconParamsRspCb    wdiSendBeaconParamsRspCb;
  wpt_uint8*                   pSendBuffer         = NULL;
  wpt_uint16                   usDataOffset        = 0;
  wpt_uint16                   usSendSize          = 0;
  /*                                                                        */

  tSendBeaconReqMsg            halSendBeaconReq;
  /*                                                                         
                
                                                                           */
  if (( NULL == pEventData ) ||
      ( NULL == pEventData->pEventData ) ||
      ( NULL == pEventData->pCBfnc ))
  {
     WPAL_TRACE( eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_WARN,
                 "%s: Invalid parameters", __FUNCTION__);
     WDI_ASSERT(0);
     return WDI_STATUS_E_FAILURE;
  }

  pwdiSendBeaconParams = (WDI_SendBeaconParamsType*)pEventData->pEventData;
  wdiSendBeaconParamsRspCb   = (WDI_SendBeaconParamsRspCb)pEventData->pCBfnc;
  /*                                                                       
                      
                                                                         */
  if (( WDI_STATUS_SUCCESS != WDI_GetMessageBuffer( pWDICtx, WDI_SND_BCON_REQ,
                        sizeof(halSendBeaconReq.sendBeaconParam),
                        &pSendBuffer, &usDataOffset, &usSendSize))||
      ( usSendSize < (usDataOffset + sizeof(halSendBeaconReq.sendBeaconParam) )))
  {
     WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_WARN,
              "Unable to get send buffer in send beacon req %x %x %x",
                pEventData, pwdiSendBeaconParams, wdiSendBeaconParamsRspCb);
     WDI_ASSERT(0);
     return WDI_STATUS_E_FAILURE;
  }

  wpalMemoryCopy(halSendBeaconReq.sendBeaconParam.bssId,
                  pwdiSendBeaconParams->wdiSendBeaconParamsInfo.macBSSID,
                  WDI_MAC_ADDR_LEN);
  halSendBeaconReq.sendBeaconParam.beaconLength =
                  pwdiSendBeaconParams->wdiSendBeaconParamsInfo.beaconLength;
  wpalMemoryCopy(halSendBeaconReq.sendBeaconParam.beacon,
                  pwdiSendBeaconParams->wdiSendBeaconParamsInfo.beacon,
                  pwdiSendBeaconParams->wdiSendBeaconParamsInfo.beaconLength);
#ifdef WLAN_SOFTAP_FEATURE
  halSendBeaconReq.sendBeaconParam.timIeOffset =
                  pwdiSendBeaconParams->wdiSendBeaconParamsInfo.timIeOffset;
#endif
#ifdef WLAN_FEATURE_P2P
  halSendBeaconReq.sendBeaconParam.p2pIeOffset =
                  pwdiSendBeaconParams->wdiSendBeaconParamsInfo.usP2PIeOffset;
#endif

  wpalMemoryCopy( pSendBuffer+usDataOffset,
                  &halSendBeaconReq.sendBeaconParam,
                  sizeof(halSendBeaconReq.sendBeaconParam));

  pWDICtx->wdiReqStatusCB     = pwdiSendBeaconParams->wdiReqStatusCB;
  pWDICtx->pReqStatusUserData = pwdiSendBeaconParams->pUserData;

  /*                                                                         
                              
                                                                           */
  return  WDI_SendMsg( pWDICtx, pSendBuffer, usSendSize,
                       wdiSendBeaconParamsRspCb, pEventData->pUserData, WDI_SND_BCON_RESP);
}/*                              */

/* 
                                                                            
                  

                                                         
                                                                    

     
                                    
*/
WDI_Status
WDI_ProcessUpdateProbeRspTemplateReq
(
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
)
{
  WDI_UpdateProbeRspTemplateParamsType*  pwdiUpdateProbeRespTmplParams;
  WDI_UpdateProbeRspTemplateRspCb        wdiUpdateProbeRespTmplRspCb;
  wpt_uint8*                             pSendBuffer         = NULL;
  wpt_uint16                             usDataOffset        = 0;
  wpt_uint16                             usSendSize          = 0;
  tSendProbeRespReqParams                halUpdateProbeRspTmplParams;
  /*                                                                        */

  /*                                                                         
                
                                                                           */
  if (( NULL == pEventData ) ||
      ( NULL == pEventData->pEventData) ||
      ( NULL == pEventData->pCBfnc))
  {
     WPAL_TRACE( eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_WARN,
                 "%s: Invalid parameters", __FUNCTION__);
     WDI_ASSERT(0);
     return WDI_STATUS_E_FAILURE;
  }

  pwdiUpdateProbeRespTmplParams =
    (WDI_UpdateProbeRspTemplateParamsType*)pEventData->pEventData;
  wdiUpdateProbeRespTmplRspCb =
    (WDI_UpdateProbeRspTemplateRspCb)pEventData->pCBfnc;
  /*                                                                       
                      
                                                                         */
  if (( WDI_STATUS_SUCCESS != WDI_GetMessageBuffer( pWDICtx, WDI_UPD_PROBE_RSP_TEMPLATE_REQ,
                        sizeof(halUpdateProbeRspTmplParams),
                        &pSendBuffer, &usDataOffset, &usSendSize))||
      ( usSendSize < (usDataOffset + sizeof(halUpdateProbeRspTmplParams) )))
  {
     WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_WARN,
              "Unable to get send buffer in set bss key req %x %x %x",
     pEventData, pwdiUpdateProbeRespTmplParams, wdiUpdateProbeRespTmplRspCb);
     WDI_ASSERT(0);
     return WDI_STATUS_E_FAILURE;
  }

  wpalMemoryCopy(halUpdateProbeRspTmplParams.bssId,
                 pwdiUpdateProbeRespTmplParams->wdiProbeRspTemplateInfo.macBSSID,
                 WDI_MAC_ADDR_LEN);

  halUpdateProbeRspTmplParams.probeRespTemplateLen =
    pwdiUpdateProbeRespTmplParams->wdiProbeRspTemplateInfo.uProbeRespTemplateLen;

  wpalMemoryCopy(halUpdateProbeRspTmplParams.pProbeRespTemplate,
    pwdiUpdateProbeRespTmplParams->wdiProbeRspTemplateInfo.pProbeRespTemplate,
                 BEACON_TEMPLATE_SIZE);


  wpalMemoryCopy(halUpdateProbeRspTmplParams.ucProxyProbeReqValidIEBmap,
           pwdiUpdateProbeRespTmplParams->wdiProbeRspTemplateInfo.uaProxyProbeReqValidIEBmap,
                 WDI_PROBE_REQ_BITMAP_IE_LEN);

  wpalMemoryCopy( pSendBuffer+usDataOffset,
                  &halUpdateProbeRspTmplParams,
                  sizeof(halUpdateProbeRspTmplParams));

  pWDICtx->wdiReqStatusCB     = pwdiUpdateProbeRespTmplParams->wdiReqStatusCB;
  pWDICtx->pReqStatusUserData = pwdiUpdateProbeRespTmplParams->pUserData;

  /*                                                                         
                                                  
                                                                           */
  return  WDI_SendMsg( pWDICtx, pSendBuffer, usSendSize,
                       wdiUpdateProbeRespTmplRspCb, pEventData->pUserData,
                       WDI_UPD_PROBE_RSP_TEMPLATE_RESP);
}/*                                    */

/* 
                                                               
                  

                                                         
                                                                    

     
                                    
*/
WDI_Status
WDI_ProcessNvDownloadReq
(
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
)
{

  WDI_NvDownloadReqParamsType*  pwdiNvDownloadReqParams = NULL;
  WDI_NvDownloadRspCb      wdiNvDownloadRspCb = NULL;

  /*                                                                         
                 
                                                                            */
  if (( NULL == pEventData ) ||
      ( NULL == (pwdiNvDownloadReqParams =
                 (WDI_NvDownloadReqParamsType*)pEventData->pEventData)) ||
      ( NULL == (wdiNvDownloadRspCb =
                (WDI_NvDownloadRspCb)pEventData->pCBfnc)))
  {
    WPAL_TRACE( eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_WARN,
                "%s: Invalid parameters", __FUNCTION__);
    WDI_ASSERT(0);
    return WDI_STATUS_E_FAILURE;
  }

  /*                           */
  pWDICtx->wdiNvBlobInfo.usTotalFragment =
                TOTALFRAGMENTS(pwdiNvDownloadReqParams->wdiBlobInfo.uBlobSize);

  /*                                                               
                                                       */
  if( 0 == pWDICtx->wdiNvBlobInfo.usCurrentFragment)
  {
    wpalMemoryCopy(&pWDICtx->wdiCachedNvDownloadReq,
                 pwdiNvDownloadReqParams,
                 sizeof(pWDICtx->wdiCachedNvDownloadReq));

    pWDICtx->pfncRspCB = pEventData->pCBfnc;
    pWDICtx->pRspCBUserData = pEventData->pUserData;
  }

  return WDI_SendNvBlobReq(pWDICtx,pEventData);
}

/* 
                                                                   
                      

                                                         
                                                                    

     
                                    
*/
WDI_Status WDI_ProcessSetMaxTxPowerReq
(
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
)
{
  WDI_SetMaxTxPowerParamsType*      pwdiSetMaxTxPowerParams = NULL;
  WDA_SetMaxTxPowerRspCb            wdiSetMaxTxPowerRspCb;
  wpt_uint8*                        pSendBuffer         = NULL;
  wpt_uint16                        usDataOffset        = 0;
  wpt_uint16                        usSendSize          = 0;
  tSetMaxTxPwrReq                   halSetMaxTxPower;
  /*                                                                        */

  /*                                                                         
                
                                                                           */
  if (( NULL == pEventData ) ||
      ( NULL == pEventData->pEventData ) ||
      ( NULL == pEventData->pCBfnc ))
  {
     WPAL_TRACE( eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_ERROR,
                 "%s: Invalid parameters", __FUNCTION__);
     WDI_ASSERT(0);
     return WDI_STATUS_E_FAILURE;
  }
  pwdiSetMaxTxPowerParams =
    (WDI_SetMaxTxPowerParamsType*)pEventData->pEventData;
  wdiSetMaxTxPowerRspCb =
    (WDA_SetMaxTxPowerRspCb)pEventData->pCBfnc;

  /*                                                                       
                      
                                                                         */
if (( WDI_STATUS_SUCCESS != WDI_GetMessageBuffer( pWDICtx, WDI_SET_MAX_TX_POWER_REQ,
                        sizeof(halSetMaxTxPower.setMaxTxPwrParams),
                        &pSendBuffer, &usDataOffset, &usSendSize))||
      ( usSendSize < (usDataOffset + sizeof(halSetMaxTxPower.setMaxTxPwrParams)
)))
  {
     WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_ERROR,
              "Unable to get Set Max Tx Power req %x %x %x",
                pEventData, pwdiSetMaxTxPowerParams, wdiSetMaxTxPowerRspCb);
     WDI_ASSERT(0);
     return WDI_STATUS_E_FAILURE;
  }

  wpalMemoryCopy(halSetMaxTxPower.setMaxTxPwrParams.bssId,
                  pwdiSetMaxTxPowerParams->wdiMaxTxPowerInfo.macBSSId,
                  WDI_MAC_ADDR_LEN);

  wpalMemoryCopy(halSetMaxTxPower.setMaxTxPwrParams.selfStaMacAddr,
                  pwdiSetMaxTxPowerParams->wdiMaxTxPowerInfo.macSelfStaMacAddr,
                  WDI_MAC_ADDR_LEN);
  halSetMaxTxPower.setMaxTxPwrParams.power =
                  pwdiSetMaxTxPowerParams->wdiMaxTxPowerInfo.ucPower;

  wpalMemoryCopy( pSendBuffer+usDataOffset,
                  &halSetMaxTxPower.setMaxTxPwrParams,
                  sizeof(halSetMaxTxPower.setMaxTxPwrParams));

  pWDICtx->wdiReqStatusCB     = pwdiSetMaxTxPowerParams->wdiReqStatusCB;
  pWDICtx->pReqStatusUserData = pwdiSetMaxTxPowerParams->pUserData;

  /*                                                                         
                              
                                                                           */
  return  WDI_SendMsg( pWDICtx, pSendBuffer, usSendSize,
                       wdiSetMaxTxPowerRspCb, pEventData->pUserData,
                                                      WDI_SET_MAX_TX_POWER_RESP);

}

#ifdef WLAN_FEATURE_P2P

/* 
                                                                            
                  

                                                         
                                                                    

     
                                    
*/
WDI_Status
WDI_ProcessP2PGONOAReq
(
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
)
{
  WDI_SetP2PGONOAReqParamsType*          pwdiP2PGONOAReqParams;
  WDI_SetP2PGONOAReqParamsRspCb          wdiP2PGONOAReqRspCb;
  wpt_uint8*                             pSendBuffer         = NULL;
  wpt_uint16                             usDataOffset        = 0;
  wpt_uint16                             usSendSize          = 0;
  tSetP2PGONOAParams                     halSetP2PGONOAParams;
  /*                                                                        */

  /*                                                                         
                
                                                                           */
  if (( NULL == pEventData ) ||
      ( NULL == pEventData->pEventData) ||
      ( NULL == pEventData->pCBfnc))
  {
     WPAL_TRACE( eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_WARN,
                 "%s: Invalid parameters", __FUNCTION__);
     WDI_ASSERT(0);
     return WDI_STATUS_E_FAILURE;
  }

  pwdiP2PGONOAReqParams =
    (WDI_SetP2PGONOAReqParamsType*)pEventData->pEventData;
  wdiP2PGONOAReqRspCb =
    (WDI_SetP2PGONOAReqParamsRspCb)pEventData->pCBfnc;
  /*                                                                       
                      
                                                                         */
  if (( WDI_STATUS_SUCCESS != WDI_GetMessageBuffer( pWDICtx,
                        WDI_P2P_GO_NOTICE_OF_ABSENCE_REQ,
                        sizeof(halSetP2PGONOAParams),
                        &pSendBuffer, &usDataOffset, &usSendSize))||
      ( usSendSize < (usDataOffset + sizeof(halSetP2PGONOAParams) )))
  {
     WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_WARN,
              "Unable to get send buffer in set P2P GO NOA REQ %x %x %x",
     pEventData, pwdiP2PGONOAReqParams, wdiP2PGONOAReqRspCb);
     WDI_ASSERT(0);
     return WDI_STATUS_E_FAILURE;
  }

  halSetP2PGONOAParams.opp_ps =
                           pwdiP2PGONOAReqParams->wdiP2PGONOAInfo.ucOpp_ps;
  halSetP2PGONOAParams.ctWindow =
                           pwdiP2PGONOAReqParams->wdiP2PGONOAInfo.uCtWindow;
  halSetP2PGONOAParams.count = pwdiP2PGONOAReqParams->wdiP2PGONOAInfo.ucCount;
  halSetP2PGONOAParams.duration =
                           pwdiP2PGONOAReqParams->wdiP2PGONOAInfo.uDuration;
  halSetP2PGONOAParams.interval =
                           pwdiP2PGONOAReqParams->wdiP2PGONOAInfo.uInterval;
  halSetP2PGONOAParams.single_noa_duration =
                 pwdiP2PGONOAReqParams->wdiP2PGONOAInfo.uSingle_noa_duration;
  halSetP2PGONOAParams.psSelection =
                   pwdiP2PGONOAReqParams->wdiP2PGONOAInfo.ucPsSelection;

  wpalMemoryCopy( pSendBuffer+usDataOffset,
                  &halSetP2PGONOAParams,
                  sizeof(halSetP2PGONOAParams));

  pWDICtx->wdiReqStatusCB     = pwdiP2PGONOAReqParams->wdiReqStatusCB;
  pWDICtx->pReqStatusUserData = pwdiP2PGONOAReqParams->pUserData;

  /*                                                                         
                                                  
                                                                           */
  return  WDI_SendMsg( pWDICtx, pSendBuffer, usSendSize,
                       wdiP2PGONOAReqRspCb, pEventData->pUserData,
                       WDI_P2P_GO_NOTICE_OF_ABSENCE_RESP);
}/*                      */

#endif


/* 
                                                             
                        
               

     
             
*/
void
WDI_SetPowerStateCb
(
   wpt_status status,
   unsigned int dxePhyAddr,
   void      *pContext
)
{
   wpt_status              wptStatus;
   WDI_ControlBlockType *pCB = NULL;
   /*                                                                       */
   if(eWLAN_PAL_STATUS_E_FAILURE == status )
   {
      //                                     
   }
   /*
                                                                    
                 
*/
   if( NULL != pContext )
   {
      pCB = (WDI_ControlBlockType *)pContext;
   }
   else
   {
      //                
      pCB = &gWDICb;
   }
   pCB->dxePhyAddr = dxePhyAddr;
   wptStatus  = wpalEventSet(&pCB->setPowerStateEvent);
   if ( eWLAN_PAL_STATUS_SUCCESS !=  wptStatus )
   {
      WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_WARN,
                "Failed to set an event");

      WDI_ASSERT(0);
   }
   return;
}


/* 
                                                        
                           

                                                         
                                                                    

     
                                    
*/
WDI_Status
WDI_ProcessEnterImpsReq
(
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
)
{
   wpt_status               wptStatus;
   WDI_EnterImpsRspCb       wdiEnterImpsRspCb = NULL;
   wpt_uint8*               pSendBuffer         = NULL;
   wpt_uint16               usDataOffset        = 0;
   wpt_uint16               usSendSize          = 0;
   /*                                                                        */

   /*                                                                         
                 
                                                                            */
   if (( NULL == pEventData ) ||
       ( NULL == (wdiEnterImpsRspCb   = (WDI_EnterImpsRspCb)pEventData->pCBfnc)))
   {
      WPAL_TRACE( eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_WARN,
                  "%s: Invalid parameters", __FUNCTION__);
      WDI_ASSERT(0);
      return WDI_STATUS_E_FAILURE;
   }

   /*                                                                       
                       
                                                                    
                                                                          */
   if (( WDI_STATUS_SUCCESS != WDI_GetMessageBuffer( pWDICtx, WDI_ENTER_IMPS_REQ,
                                                     0,
                                                     &pSendBuffer, &usDataOffset, &usSendSize))||
       ( usSendSize < (usDataOffset )))
   {
      WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_WARN,
               "Unable to get send buffer in Enter IMPS req %x %x",
                 pEventData, wdiEnterImpsRspCb);
      WDI_ASSERT(0);
      return WDI_STATUS_E_FAILURE;
   }

   /*                                     */
   wptStatus = wpalEventReset(&pWDICtx->setPowerStateEvent);
   if ( eWLAN_PAL_STATUS_SUCCESS != wptStatus )
   {
      WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_WARN,
                "WDI Init failed to reset an event");

      WDI_ASSERT(0);
      return VOS_STATUS_E_FAILURE;
   }

   //                                     
   WDTS_SetPowerState(pWDICtx, WDTS_POWER_STATE_IMPS, WDI_SetPowerStateCb);

   /*
                                                                   
    */
   wptStatus = wpalEventWait(&pWDICtx->setPowerStateEvent,
                             WDI_SET_POWER_STATE_TIMEOUT);
   if ( eWLAN_PAL_STATUS_SUCCESS != wptStatus )
   {
      WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_WARN,
                "WDI Init failed to wait on an event");

      WDI_ASSERT(0);
      return VOS_STATUS_E_FAILURE;
   }

   /*                                                                         
                                
                                                                            */
   return  WDI_SendMsg( pWDICtx, pSendBuffer, usSendSize,
                        wdiEnterImpsRspCb, pEventData->pUserData, WDI_ENTER_IMPS_RESP);
}/*                       */

/* 
                                                       
                           

                                                         
                                                                    

     
                                    
*/
WDI_Status
WDI_ProcessExitImpsReq
(
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
)
{
   WDI_ExitImpsRspCb        wdiExitImpsRspCb = NULL;
   wpt_uint8*               pSendBuffer         = NULL;
   wpt_uint16               usDataOffset        = 0;
   wpt_uint16               usSendSize          = 0;
   /*                                                                        */

   /*                                                                         
                 
                                                                            */
   if (( NULL == pEventData ) ||
       ( NULL == (wdiExitImpsRspCb   = (WDI_ExitImpsRspCb)pEventData->pCBfnc)))
   {
      WPAL_TRACE( eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_WARN,
                  "%s: Invalid parameters", __FUNCTION__);
      WDI_ASSERT(0);
      return WDI_STATUS_E_FAILURE;
   }

   /*                                                                       
                       
                                                                    
                                                                          */
   if (( WDI_STATUS_SUCCESS != WDI_GetMessageBuffer( pWDICtx, WDI_EXIT_IMPS_REQ,
                                                     0,
                                                     &pSendBuffer, &usDataOffset, &usSendSize))||
       ( usSendSize < (usDataOffset )))
   {
      WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_WARN,
               "Unable to get send buffer in Exit IMPS req %x %x",
                 pEventData, wdiExitImpsRspCb);
      WDI_ASSERT(0);
      return WDI_STATUS_E_FAILURE;
   }

   /*                                                                         
                                
                                                                            */
   return  WDI_SendMsg( pWDICtx, pSendBuffer, usSendSize,
                        wdiExitImpsRspCb, pEventData->pUserData, WDI_EXIT_IMPS_RESP);
}/*                      */

/* 
                                                             
                      

                                                         
                                                                    

     
                                    
*/
WDI_Status
WDI_ProcessEnterBmpsReq
(
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
)
{
   WDI_EnterBmpsReqParamsType*  pwdiEnterBmpsReqParams = NULL;
   WDI_EnterBmpsRspCb           wdiEnterBmpsRspCb = NULL;
   wpt_uint8*               pSendBuffer         = NULL;
   wpt_uint16               usDataOffset        = 0;
   wpt_uint16               usSendSize          = 0;
   tHalEnterBmpsReqParams   enterBmpsReq;
   wpt_status               wptStatus;

   /*                                                                        */

  /*                                                                         
                 
                                                                           */
   if (( NULL == pEventData ) ||
       ( NULL == (pwdiEnterBmpsReqParams = (WDI_EnterBmpsReqParamsType*)pEventData->pEventData)) ||
       ( NULL == (wdiEnterBmpsRspCb   = (WDI_EnterBmpsRspCb)pEventData->pCBfnc)))
   {
      WPAL_TRACE( eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_WARN,
                  "%s: Invalid parameters", __FUNCTION__);
      WDI_ASSERT(0);
      return WDI_STATUS_E_FAILURE;
   }

   /*                                                                       
                       
                                                                    
                                                                          */
   if (( WDI_STATUS_SUCCESS != WDI_GetMessageBuffer( pWDICtx, WDI_ENTER_BMPS_REQ,
                         sizeof(enterBmpsReq),
                         &pSendBuffer, &usDataOffset, &usSendSize))||
       ( usSendSize < (usDataOffset + sizeof(enterBmpsReq) )))
   {
      WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_WARN,
               "Unable to get send buffer in Enter BMPS req %x %x %x",
                 pEventData, pwdiEnterBmpsReqParams, wdiEnterBmpsRspCb);
      WDI_ASSERT(0);
      return WDI_STATUS_E_FAILURE;
   }

   /*                                     */
   wptStatus = wpalEventReset(&pWDICtx->setPowerStateEvent);
   if ( eWLAN_PAL_STATUS_SUCCESS != wptStatus )
   {
      WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_WARN,
                "WDI Init failed to reset an event");

      WDI_ASSERT(0);
      return VOS_STATUS_E_FAILURE;
   }

   //                                     
   WDTS_SetPowerState(pWDICtx, WDTS_POWER_STATE_BMPS, WDI_SetPowerStateCb);

/*
                                                                   
    */
   wptStatus = wpalEventWait(&pWDICtx->setPowerStateEvent,
                             WDI_SET_POWER_STATE_TIMEOUT);
   if ( eWLAN_PAL_STATUS_SUCCESS != wptStatus )
   {
      WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_WARN,
                "WDI Init failed to wait on an event");

      WDI_ASSERT(0);
      return VOS_STATUS_E_FAILURE;
   }

   pWDICtx->bInBmps = eWLAN_PAL_TRUE;

   enterBmpsReq.bssIdx = pwdiEnterBmpsReqParams->wdiEnterBmpsInfo.ucBssIdx;
   enterBmpsReq.tbtt = pwdiEnterBmpsReqParams->wdiEnterBmpsInfo.uTbtt;
   enterBmpsReq.dtimCount = pwdiEnterBmpsReqParams->wdiEnterBmpsInfo.ucDtimCount;
   enterBmpsReq.dtimPeriod = pwdiEnterBmpsReqParams->wdiEnterBmpsInfo.ucDtimPeriod;

   //                        
   enterBmpsReq.rssiFilterPeriod = pwdiEnterBmpsReqParams->wdiEnterBmpsInfo.rssiFilterPeriod;
   enterBmpsReq.numBeaconPerRssiAverage = pwdiEnterBmpsReqParams->wdiEnterBmpsInfo.numBeaconPerRssiAverage;
   enterBmpsReq.bRssiFilterEnable = pwdiEnterBmpsReqParams->wdiEnterBmpsInfo.bRssiFilterEnable;

   wpalMemoryCopy( pSendBuffer+usDataOffset,
                   &enterBmpsReq,
                   sizeof(enterBmpsReq));

   pWDICtx->wdiReqStatusCB     = pwdiEnterBmpsReqParams->wdiReqStatusCB;
   pWDICtx->pReqStatusUserData = pwdiEnterBmpsReqParams->pUserData;

   /*                                                                         
                                
                                                                            */
   return  WDI_SendMsg( pWDICtx, pSendBuffer, usSendSize,
                        wdiEnterBmpsRspCb, pEventData->pUserData, WDI_ENTER_BMPS_RESP);
}/*                       */

/* 
                                                                
                  

                                                         
                                                                    

     
                                    
*/
WDI_Status
WDI_ProcessExitBmpsReq
(
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
)
{
   WDI_ExitBmpsReqParamsType*  pwdiExitBmpsReqParams = NULL;
   WDI_ExitBmpsRspCb           wdiExitBmpsRspCb = NULL;
   wpt_uint8*               pSendBuffer         = NULL;
   wpt_uint16               usDataOffset        = 0;
   wpt_uint16               usSendSize          = 0;
   tHalExitBmpsReqParams    exitBmpsReq;
   /*                                                                        */

   /*                                                                         
                 
                                                                            */
   if (( NULL == pEventData ) ||
       ( NULL == (pwdiExitBmpsReqParams = (WDI_ExitBmpsReqParamsType*)pEventData->pEventData)) ||
       ( NULL == (wdiExitBmpsRspCb   = (WDI_ExitBmpsRspCb)pEventData->pCBfnc)))
   {
      WPAL_TRACE( eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_WARN,
                  "%s: Invalid parameters", __FUNCTION__);
      WDI_ASSERT(0);
      return WDI_STATUS_E_FAILURE;
   }

   /*                                                                       
                       
                                                                    
                                                                          */
   if (( WDI_STATUS_SUCCESS != WDI_GetMessageBuffer( pWDICtx, WDI_EXIT_BMPS_REQ,
                         sizeof(exitBmpsReq),
                         &pSendBuffer, &usDataOffset, &usSendSize))||
       ( usSendSize < (usDataOffset + sizeof(exitBmpsReq) )))
   {
      WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_WARN,
               "Unable to get send buffer in Exit BMPS req %x %x %x",
                 pEventData, pwdiExitBmpsReqParams, wdiExitBmpsRspCb);
      WDI_ASSERT(0);
      return WDI_STATUS_E_FAILURE;
   }
   exitBmpsReq.sendDataNull = pwdiExitBmpsReqParams->wdiExitBmpsInfo.ucSendDataNull;

   exitBmpsReq.bssIdx = pwdiExitBmpsReqParams->wdiExitBmpsInfo.bssIdx;

   wpalMemoryCopy( pSendBuffer+usDataOffset,
                   &exitBmpsReq,
                   sizeof(exitBmpsReq));

   pWDICtx->wdiReqStatusCB     = pwdiExitBmpsReqParams->wdiReqStatusCB;
   pWDICtx->pReqStatusUserData = pwdiExitBmpsReqParams->pUserData;

   /*                                                                         
                                
                                                                            */
   return  WDI_SendMsg( pWDICtx, pSendBuffer, usSendSize,
                        wdiExitBmpsRspCb, pEventData->pUserData, WDI_EXIT_BMPS_RESP);
}/*                      */

/* 
                                                              
                      

                                                         
                                                                    

     
                                    
*/
WDI_Status
WDI_ProcessEnterUapsdReq
(
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
)
{
   WDI_EnterUapsdReqParamsType*  pwdiEnterUapsdReqParams = NULL;
   WDI_EnterUapsdRspCb           wdiEnterUapsdRspCb = NULL;
   wpt_uint8*               pSendBuffer         = NULL;
   wpt_uint16               usDataOffset        = 0;
   wpt_uint16               usSendSize          = 0;
   tUapsdReqParams          enterUapsdReq;
   /*                                                                        */

   /*                                                                         
                 
                                                                            */
   if (( NULL == pEventData ) ||
       ( NULL == (pwdiEnterUapsdReqParams = (WDI_EnterUapsdReqParamsType*)pEventData->pEventData)) ||
       ( NULL == (wdiEnterUapsdRspCb   = (WDI_EnterUapsdRspCb)pEventData->pCBfnc)))
   {
      WPAL_TRACE( eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_WARN,
                  "%s: Invalid parameters", __FUNCTION__);
      WDI_ASSERT(0);
      return WDI_STATUS_E_FAILURE;
   }

   /*                                                                       
                       
                                                                    
                                                                          */
   if (( WDI_STATUS_SUCCESS != WDI_GetMessageBuffer( pWDICtx, WDI_ENTER_UAPSD_REQ,
                         sizeof(enterUapsdReq),
                         &pSendBuffer, &usDataOffset, &usSendSize))||
       ( usSendSize < (usDataOffset + sizeof(enterUapsdReq) )))
   {
      WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_WARN,
               "Unable to get send buffer in Enter UAPSD req %x %x %x",
                 pEventData, pwdiEnterUapsdReqParams, wdiEnterUapsdRspCb);
      WDI_ASSERT(0);
      return WDI_STATUS_E_FAILURE;
   }

   enterUapsdReq.beDeliveryEnabled  = pwdiEnterUapsdReqParams->wdiEnterUapsdInfo.ucBeDeliveryEnabled;
   enterUapsdReq.beTriggerEnabled   = pwdiEnterUapsdReqParams->wdiEnterUapsdInfo.ucBeTriggerEnabled;
   enterUapsdReq.bkDeliveryEnabled  = pwdiEnterUapsdReqParams->wdiEnterUapsdInfo.ucBkDeliveryEnabled;
   enterUapsdReq.bkTriggerEnabled   = pwdiEnterUapsdReqParams->wdiEnterUapsdInfo.ucBkTriggerEnabled;
   enterUapsdReq.viDeliveryEnabled  = pwdiEnterUapsdReqParams->wdiEnterUapsdInfo.ucViDeliveryEnabled;
   enterUapsdReq.viTriggerEnabled   = pwdiEnterUapsdReqParams->wdiEnterUapsdInfo.ucViTriggerEnabled;
   enterUapsdReq.voDeliveryEnabled  = pwdiEnterUapsdReqParams->wdiEnterUapsdInfo.ucVoDeliveryEnabled;
   enterUapsdReq.voTriggerEnabled   = pwdiEnterUapsdReqParams->wdiEnterUapsdInfo.ucVoTriggerEnabled;
   enterUapsdReq.bssIdx             = pwdiEnterUapsdReqParams->wdiEnterUapsdInfo.bssIdx;

   wpalMemoryCopy( pSendBuffer+usDataOffset,
                   &enterUapsdReq,
                   sizeof(enterUapsdReq));

   pWDICtx->wdiReqStatusCB     = pwdiEnterUapsdReqParams->wdiReqStatusCB;
   pWDICtx->pReqStatusUserData = pwdiEnterUapsdReqParams->pUserData;

   /*                                                                         
                                
                                                                            */
   return  WDI_SendMsg( pWDICtx, pSendBuffer, usSendSize,
                        wdiEnterUapsdRspCb, pEventData->pUserData, WDI_ENTER_UAPSD_RESP);
}/*                        */

/* 
                                                        
                           

                                                         
                                                                    

     
                                    
*/
WDI_Status
WDI_ProcessExitUapsdReq
(
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
)
{
   WDI_ExitUapsdRspCb       wdiExitUapsdRspCb = NULL;
   wpt_uint8*               pSendBuffer         = NULL;
   wpt_uint16               usDataOffset        = 0;
   wpt_uint16               usSendSize          = 0;
   /*                                                                        */

   /*                                                                         
                 
                                                                            */
   if (( NULL == pEventData ) ||
       ( NULL == (wdiExitUapsdRspCb   = (WDI_ExitUapsdRspCb)pEventData->pCBfnc)))
   {
      WPAL_TRACE( eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_WARN,
                  "%s: Invalid parameters", __FUNCTION__);
      WDI_ASSERT(0);
      return WDI_STATUS_E_FAILURE;
   }

   /*                                                                       
                       
                                                                    
                                                                          */
   if (( WDI_STATUS_SUCCESS != WDI_GetMessageBuffer( pWDICtx, WDI_EXIT_UAPSD_REQ,
                                                     0,
                                                     &pSendBuffer, &usDataOffset, &usSendSize))||
       ( usSendSize < (usDataOffset )))
   {
      WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_WARN,
               "Unable to get send buffer in Exit UAPSD req %x %x",
                 pEventData, wdiExitUapsdRspCb);
      WDI_ASSERT(0);
      return WDI_STATUS_E_FAILURE;
   }

   /*                                                                         
                                
                                                                            */
  return  WDI_SendMsg( pWDICtx, pSendBuffer, usSendSize,
                        wdiExitUapsdRspCb, pEventData->pUserData, WDI_EXIT_UAPSD_RESP);
}/*                       */

/* 
                                                              
                           

                                                         
                                                                    

     
                                    
*/
WDI_Status
WDI_ProcessSetUapsdAcParamsReq
(
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
)
{
  WDI_SetUapsdAcParamsReqParamsType*  pwdiSetUapsdAcParams = NULL;
  WDI_SetUapsdAcParamsCb              wdiSetUapsdAcParamsCb = NULL;
  wpt_uint8*               pSendBuffer         = NULL;
  wpt_uint16               usDataOffset        = 0;
  wpt_uint16               usSendSize          = 0;
  tUapsdInfo               uapsdAcParamsReq;
  /*                                                                        */

  /*                                                                         
                
                                                                           */
  if (( NULL == pEventData ) ||
      ( NULL == (pwdiSetUapsdAcParams = (WDI_SetUapsdAcParamsReqParamsType*)pEventData->pEventData)) ||
      ( NULL == (wdiSetUapsdAcParamsCb   = (WDI_SetUapsdAcParamsCb)pEventData->pCBfnc)))
  {
     WPAL_TRACE( eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_WARN,
                 "%s: Invalid parameters", __FUNCTION__);
     WDI_ASSERT(0);
     return WDI_STATUS_E_FAILURE;
  }

  /*                                                                       
                      
                                                                   
                                                                         */
  if (( WDI_STATUS_SUCCESS != WDI_GetMessageBuffer( pWDICtx, WDI_SET_UAPSD_PARAM_REQ,
                        sizeof(uapsdAcParamsReq),
                        &pSendBuffer, &usDataOffset, &usSendSize))||
      ( usSendSize < (usDataOffset + sizeof(uapsdAcParamsReq) )))
  {
     WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_WARN,
              "Unable to get send buffer in Set UAPSD params req %x %x %x",
                pEventData, pwdiSetUapsdAcParams, wdiSetUapsdAcParamsCb);
     WDI_ASSERT(0);
     return WDI_STATUS_E_FAILURE;
  }

  uapsdAcParamsReq.ac = pwdiSetUapsdAcParams->wdiUapsdInfo.ucAc;
  uapsdAcParamsReq.staidx = pwdiSetUapsdAcParams->wdiUapsdInfo.ucSTAIdx;
  uapsdAcParamsReq.up = pwdiSetUapsdAcParams->wdiUapsdInfo.ucUp;
  uapsdAcParamsReq.delayInterval = pwdiSetUapsdAcParams->wdiUapsdInfo.uDelayInterval;
  uapsdAcParamsReq.srvInterval = pwdiSetUapsdAcParams->wdiUapsdInfo.uSrvInterval;
  uapsdAcParamsReq.susInterval = pwdiSetUapsdAcParams->wdiUapsdInfo.uSusInterval;

  wpalMemoryCopy( pSendBuffer+usDataOffset,
                  &uapsdAcParamsReq,
                  sizeof(uapsdAcParamsReq));

  pWDICtx->wdiReqStatusCB     = pwdiSetUapsdAcParams->wdiReqStatusCB;
  pWDICtx->pReqStatusUserData = pwdiSetUapsdAcParams->pUserData;

  /*                                                                         
                               
                                                                           */
  return  WDI_SendMsg( pWDICtx, pSendBuffer, usSendSize,
                       wdiSetUapsdAcParamsCb, pEventData->pUserData, WDI_SET_UAPSD_PARAM_RESP);
}/*                              */

/* 
                                                            
                                

                                                         
                                                                    

     
                                    
*/
WDI_Status
WDI_ProcessUpdateUapsdParamsReq
(
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
)
{
   WDI_UpdateUapsdReqParamsType*  pwdiUpdateUapsdReqParams = NULL;
   WDI_UpdateUapsdParamsCb        wdiUpdateUapsdParamsCb = NULL;
   wpt_uint8*               pSendBuffer         = NULL;
   wpt_uint16               usDataOffset        = 0;
   wpt_uint16               usSendSize          = 0;
   /*                                                                        */

   /*                                                                         
                 
                                                                            */
   if (( NULL == pEventData ) ||
       ( NULL == (pwdiUpdateUapsdReqParams = (WDI_UpdateUapsdReqParamsType*)pEventData->pEventData)) ||
       ( NULL == (wdiUpdateUapsdParamsCb   = (WDI_UpdateUapsdParamsCb)pEventData->pCBfnc)))
   {
      WPAL_TRACE( eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_WARN,
                  "%s: Invalid parameters", __FUNCTION__);
      WDI_ASSERT(0);
      return WDI_STATUS_E_FAILURE;
   }

   /*                                                                       
                       
                                                                    
                                                                          */
   if (( WDI_STATUS_SUCCESS != WDI_GetMessageBuffer( pWDICtx, WDI_UPDATE_UAPSD_PARAM_REQ,
                         sizeof(pwdiUpdateUapsdReqParams->wdiUpdateUapsdInfo),
                         &pSendBuffer, &usDataOffset, &usSendSize))||
       ( usSendSize < (usDataOffset + sizeof(pwdiUpdateUapsdReqParams->wdiUpdateUapsdInfo) )))
   {
      WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_WARN,
               "Unable to get send buffer in Update UAPSD params req %x %x %x",
                 pEventData, pwdiUpdateUapsdReqParams, wdiUpdateUapsdParamsCb);
      WDI_ASSERT(0);
      return WDI_STATUS_E_FAILURE;
   }

   wpalMemoryCopy( pSendBuffer+usDataOffset,
                   &pwdiUpdateUapsdReqParams->wdiUpdateUapsdInfo,
                   sizeof(pwdiUpdateUapsdReqParams->wdiUpdateUapsdInfo));

   pWDICtx->wdiReqStatusCB     = pwdiUpdateUapsdReqParams->wdiReqStatusCB;
   pWDICtx->pReqStatusUserData = pwdiUpdateUapsdReqParams->pUserData;

   /*                                                                         
                                
                                                                            */
   return  WDI_SendMsg( pWDICtx, pSendBuffer, usSendSize,
                        wdiUpdateUapsdParamsCb, pEventData->pUserData, WDI_UPDATE_UAPSD_PARAM_RESP);
}/*                               */

/* 
                                                             
                                

                                                         
                                                                    

     
                                    
*/
WDI_Status
WDI_ProcessConfigureRxpFilterReq
(
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
)
{
  WDI_ConfigureRxpFilterReqParamsType*  pwdiRxpFilterParams = NULL;
  WDI_ConfigureRxpFilterCb              wdiConfigureRxpFilterCb = NULL;
  wpt_uint8*               pSendBuffer         = NULL;
  wpt_uint16               usDataOffset        = 0;
  wpt_uint16               usSendSize          = 0;
  tHalConfigureRxpFilterReqParams     halRxpFilterParams;

  /*                                                                        */

  /*                                                                         
                
                                                                           */
  if (( NULL == pEventData ) ||
      ( NULL == (pwdiRxpFilterParams = (WDI_ConfigureRxpFilterReqParamsType*)pEventData->pEventData)) ||
      ( NULL == (wdiConfigureRxpFilterCb   = (WDI_ConfigureRxpFilterCb)pEventData->pCBfnc)))
  {
     WPAL_TRACE( eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_WARN,
                 "%s: Invalid parameters", __FUNCTION__);
     WDI_ASSERT(0);
     return WDI_STATUS_E_FAILURE;
  }

   /*                                                                       
                       
                                                                          */
  if (( WDI_STATUS_SUCCESS != WDI_GetMessageBuffer( pWDICtx, WDI_CONFIGURE_RXP_FILTER_REQ,
                        sizeof(halRxpFilterParams),
                        &pSendBuffer, &usDataOffset, &usSendSize))||
      ( usSendSize < (usDataOffset + sizeof(halRxpFilterParams) )))
  {
     WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_WARN,
              "Unable to get send buffer in Set UAPSD params req %x %x %x",
                pEventData, pwdiRxpFilterParams, wdiConfigureRxpFilterCb);
     WDI_ASSERT(0);
     return WDI_STATUS_E_FAILURE;
  }

  halRxpFilterParams.setMcstBcstFilterSetting =
      pwdiRxpFilterParams->wdiRxpFilterParam.ucSetMcstBcstFilterSetting;
  halRxpFilterParams.setMcstBcstFilter =
      pwdiRxpFilterParams->wdiRxpFilterParam.ucSetMcstBcstFilter;

  wpalMemoryCopy( pSendBuffer+usDataOffset,
                  &halRxpFilterParams,
                  sizeof(halRxpFilterParams));

  pWDICtx->wdiReqStatusCB     = pwdiRxpFilterParams->wdiReqStatusCB;
  pWDICtx->pReqStatusUserData = pwdiRxpFilterParams->pUserData;

  /*                                                                         
                               
                                                                           */
  return  WDI_SendMsg( pWDICtx, pSendBuffer, usSendSize,
                       wdiConfigureRxpFilterCb, pEventData->pUserData, WDI_CONFIGURE_RXP_FILTER_RESP);
}/*                                */

/* 
                                                          
                                

                                                         
                                                                    

     
                                    
*/
WDI_Status
WDI_ProcessSetBeaconFilterReq
(
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
)
{
   WDI_BeaconFilterReqParamsType*  pwdiBeaconFilterParams = NULL;
   WDI_SetBeaconFilterCb           wdiBeaconFilterCb = NULL;
   wpt_uint8*               pSendBuffer         = NULL;
   wpt_uint16               usDataOffset        = 0;
   wpt_uint16               usSendSize          = 0;
   /*                                                                        */

   /*                                                                         
                 
                                                                            */
   if (( NULL == pEventData ) ||
       ( NULL == (pwdiBeaconFilterParams = (WDI_BeaconFilterReqParamsType*)pEventData->pEventData)) ||
       ( NULL == (wdiBeaconFilterCb   = (WDI_SetBeaconFilterCb)pEventData->pCBfnc)))
   {
      WPAL_TRACE( eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_WARN,
                  "%s: Invalid parameters", __FUNCTION__);
      WDI_ASSERT(0);
      return WDI_STATUS_E_FAILURE;
   }

   /*                                                                       
                       
                                                                    
                                                                          */
   if (( WDI_STATUS_SUCCESS != WDI_GetMessageBuffer( pWDICtx, WDI_SET_BEACON_FILTER_REQ,
                         sizeof(pwdiBeaconFilterParams->wdiBeaconFilterInfo) + pwdiBeaconFilterParams->wdiBeaconFilterInfo.usIeNum * sizeof(tBeaconFilterIe),
                         &pSendBuffer, &usDataOffset, &usSendSize))||
       ( usSendSize < (usDataOffset + sizeof(pwdiBeaconFilterParams->wdiBeaconFilterInfo) )))
   {
      WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_WARN,
               "Unable to get send buffer in Set beacon filter req %x %x %x",
                 pEventData, pwdiBeaconFilterParams, wdiBeaconFilterCb);
      WDI_ASSERT(0);
      return WDI_STATUS_E_FAILURE;
   }

   wpalMemoryCopy( pSendBuffer+usDataOffset,
                   &pwdiBeaconFilterParams->wdiBeaconFilterInfo,
                   sizeof(pwdiBeaconFilterParams->wdiBeaconFilterInfo));
   wpalMemoryCopy( pSendBuffer+usDataOffset+sizeof(pwdiBeaconFilterParams->wdiBeaconFilterInfo),
                   &pwdiBeaconFilterParams->aFilters[0],
                   pwdiBeaconFilterParams->wdiBeaconFilterInfo.usIeNum * sizeof(tBeaconFilterIe));

   pWDICtx->wdiReqStatusCB     = pwdiBeaconFilterParams->wdiReqStatusCB;
   pWDICtx->pReqStatusUserData = pwdiBeaconFilterParams->pUserData;

   /*                                                                         
                                
                                                                            */
   return  WDI_SendMsg( pWDICtx, pSendBuffer, usSendSize,
                        wdiBeaconFilterCb, pEventData->pUserData, WDI_SET_BEACON_FILTER_RESP);
}/*                             */

/* 
                                                             
                                

                                                         
                                                                    

     
                                    
*/
WDI_Status
WDI_ProcessRemBeaconFilterReq
(
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
)
{
   WDI_RemBeaconFilterReqParamsType*  pwdiBeaconFilterParams = NULL;
   WDI_RemBeaconFilterCb              wdiBeaconFilterCb = NULL;
   wpt_uint8*               pSendBuffer         = NULL;
   wpt_uint16               usDataOffset        = 0;
   wpt_uint16               usSendSize          = 0;
   /*                                                                        */

   /*                                                                         
                 
                                                                            */
   if (( NULL == pEventData ) ||
       ( NULL == (pwdiBeaconFilterParams = (WDI_RemBeaconFilterReqParamsType*)pEventData->pEventData)) ||
       ( NULL == (wdiBeaconFilterCb   = (WDI_RemBeaconFilterCb)pEventData->pCBfnc)))
   {
      WPAL_TRACE( eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_WARN,
                  "%s: Invalid parameters", __FUNCTION__);
      WDI_ASSERT(0);
      return WDI_STATUS_E_FAILURE;
   }

   /*                                                                       
                       
                                                                    
                                                                          */
   if (( WDI_STATUS_SUCCESS != WDI_GetMessageBuffer( pWDICtx, WDI_REM_BEACON_FILTER_REQ,
                         sizeof(pwdiBeaconFilterParams->wdiBeaconFilterInfo),
                         &pSendBuffer, &usDataOffset, &usSendSize))||
       ( usSendSize < (usDataOffset + sizeof(pwdiBeaconFilterParams->wdiBeaconFilterInfo) )))
   {
      WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_WARN,
                  "Unable to get send buffer in remove beacon filter req %x %x %x",
                  pEventData, pwdiBeaconFilterParams, wdiBeaconFilterCb);
      WDI_ASSERT(0);
      return WDI_STATUS_E_FAILURE;
   }

   wpalMemoryCopy( pSendBuffer+usDataOffset,
                   &pwdiBeaconFilterParams->wdiBeaconFilterInfo,
                   sizeof(pwdiBeaconFilterParams->wdiBeaconFilterInfo));

   pWDICtx->wdiReqStatusCB     = pwdiBeaconFilterParams->wdiReqStatusCB;
   pWDICtx->pReqStatusUserData = pwdiBeaconFilterParams->pUserData;

   /*                                                                         
                                
                                                                            */
   return  WDI_SendMsg( pWDICtx, pSendBuffer, usSendSize,
                        wdiBeaconFilterCb, pEventData->pUserData, WDI_REM_BEACON_FILTER_RESP);
}

/* 
                                                            
                                

                                                         
                                                                    

     
                                    
*/
WDI_Status
WDI_ProcessSetRSSIThresholdsReq
(
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
)
{
   WDI_SetRSSIThresholdsReqParamsType*  pwdiRSSIThresholdsParams = NULL;
   WDI_SetRSSIThresholdsCb              wdiRSSIThresholdsCb = NULL;
   wpt_uint8*               pSendBuffer         = NULL;
   wpt_uint16               usDataOffset        = 0;
   wpt_uint16               usSendSize          = 0;
   tHalRSSIThresholds       rssiThresholdsReq;
   WDI_Status               ret_status = 0;
   /*                                                                        */

   /*                                                                         
                 
                                                                            */
   if (( NULL == pEventData ) ||
       ( NULL == (pwdiRSSIThresholdsParams = (WDI_SetRSSIThresholdsReqParamsType*)pEventData->pEventData)) ||
       ( NULL == (wdiRSSIThresholdsCb   = (WDI_SetRSSIThresholdsCb)pEventData->pCBfnc)))
   {
      WPAL_TRACE( eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_WARN,
                  "%s: Invalid parameters", __FUNCTION__);
      WDI_ASSERT(0);
      return WDI_STATUS_E_FAILURE;
   }

   /*                                                                       
                       
                                                                    
                                                                          */
   if (( WDI_STATUS_SUCCESS != WDI_GetMessageBuffer( pWDICtx, WDI_SET_RSSI_THRESHOLDS_REQ,
                         sizeof(rssiThresholdsReq),
                         &pSendBuffer, &usDataOffset, &usSendSize))||
       ( usSendSize < (usDataOffset + sizeof(rssiThresholdsReq) )))
   {
      WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_WARN,
                  "Unable to get send buffer in remove beacon filter req %x %x %x",
                  pEventData, pwdiRSSIThresholdsParams, wdiRSSIThresholdsCb);
      WDI_ASSERT(0);
      return WDI_STATUS_E_FAILURE;
   }

   rssiThresholdsReq.bReserved10 =
      pwdiRSSIThresholdsParams->wdiRSSIThresholdsInfo.bReserved10;
   rssiThresholdsReq.bRssiThres1NegNotify =
      pwdiRSSIThresholdsParams->wdiRSSIThresholdsInfo.bRssiThres1NegNotify;
   rssiThresholdsReq.bRssiThres1PosNotify =
      pwdiRSSIThresholdsParams->wdiRSSIThresholdsInfo.bRssiThres1PosNotify;
   rssiThresholdsReq.bRssiThres2NegNotify =
      pwdiRSSIThresholdsParams->wdiRSSIThresholdsInfo.bRssiThres2NegNotify;
   rssiThresholdsReq.bRssiThres2PosNotify =
      pwdiRSSIThresholdsParams->wdiRSSIThresholdsInfo.bRssiThres2PosNotify;
   rssiThresholdsReq.bRssiThres3NegNotify =
      pwdiRSSIThresholdsParams->wdiRSSIThresholdsInfo.bRssiThres3NegNotify;
   rssiThresholdsReq.bRssiThres3PosNotify =
      pwdiRSSIThresholdsParams->wdiRSSIThresholdsInfo.bRssiThres3PosNotify;
   rssiThresholdsReq.ucRssiThreshold1 =
      pwdiRSSIThresholdsParams->wdiRSSIThresholdsInfo.ucRssiThreshold1;
   rssiThresholdsReq.ucRssiThreshold2 =
      pwdiRSSIThresholdsParams->wdiRSSIThresholdsInfo.ucRssiThreshold2;
   rssiThresholdsReq.ucRssiThreshold3 =
      pwdiRSSIThresholdsParams->wdiRSSIThresholdsInfo.ucRssiThreshold3;

   wpalMemoryCopy( pSendBuffer+usDataOffset,
                   &rssiThresholdsReq,
                   sizeof(rssiThresholdsReq));

   pWDICtx->wdiReqStatusCB     = pwdiRSSIThresholdsParams->wdiReqStatusCB;
   pWDICtx->pReqStatusUserData = pwdiRSSIThresholdsParams->pUserData;

   /*                                                                         
                                  
                                                                            */
   if ((ret_status = WDI_SendMsg( pWDICtx, pSendBuffer, usSendSize,
                        wdiRSSIThresholdsCb, pEventData->pUserData, WDI_SET_RSSI_THRESHOLDS_RESP)) == WDI_STATUS_SUCCESS)
   {
      //                                                                                 
      //                                                                        
      //                                                                      
      //                                                                       
      //                  
      //                                            
      pWDICtx->ucExpectedStateTransition =  WDI_BUSY_ST;
   }

   return ret_status;
}

/* 
                                                            
                                

                                                         
                                                                    

     
                                    
*/
WDI_Status
WDI_ProcessHostOffloadReq
(
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
)
{
   WDI_HostOffloadReqParamsType*  pwdiHostOffloadParams = NULL;
   WDI_HostOffloadCb              wdiHostOffloadCb = NULL;
   wpt_uint8*               pSendBuffer         = NULL;
   wpt_uint16               usDataOffset        = 0;
   wpt_uint16               usSendSize          = 0;
   tHalHostOffloadReq       hostOffloadParams;
   tHalNSOffloadParams      nsOffloadParams;

   /*                                                                        */

   /*                                                                         
                 
                                                                            */
   if (( NULL == pEventData ) ||
       ( NULL == (pwdiHostOffloadParams = (WDI_HostOffloadReqParamsType*)pEventData->pEventData)) ||
       ( NULL == (wdiHostOffloadCb   = (WDI_HostOffloadCb)pEventData->pCBfnc)))
   {
      WPAL_TRACE( eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_WARN,
                  "%s: Invalid parameters", __FUNCTION__);
      WDI_ASSERT(0);
      return WDI_STATUS_E_FAILURE;
   }

   /*                                                                       
                       
                                                                    
                                                                          */
   if (( WDI_STATUS_SUCCESS != WDI_GetMessageBuffer( pWDICtx, WDI_HOST_OFFLOAD_REQ,
                         sizeof(hostOffloadParams)+sizeof(nsOffloadParams),
                         &pSendBuffer, &usDataOffset, &usSendSize))||
       ( usSendSize < (usDataOffset + sizeof(hostOffloadParams) + sizeof(nsOffloadParams) )))
   {
      WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_WARN,
                  "Unable to get send buffer in host offload req %x %x %x",
                  pEventData, pwdiHostOffloadParams, wdiHostOffloadCb);
      WDI_ASSERT(0);
      return WDI_STATUS_E_FAILURE;
   }

   hostOffloadParams.offloadType = pwdiHostOffloadParams->wdiHostOffloadInfo.ucOffloadType;
   hostOffloadParams.enableOrDisable = pwdiHostOffloadParams->wdiHostOffloadInfo.ucEnableOrDisable;

   if( HAL_IPV4_ARP_REPLY_OFFLOAD == hostOffloadParams.offloadType )
   {
      //            
      wpalMemoryCopy(hostOffloadParams.params.hostIpv4Addr,
                     pwdiHostOffloadParams->wdiHostOffloadInfo.params.aHostIpv4Addr,
                     4);
   }
   else
   {
      //           
      wpalMemoryCopy(hostOffloadParams.params.hostIpv6Addr,
                     pwdiHostOffloadParams->wdiHostOffloadInfo.params.aHostIpv6Addr,
                     16);

#ifdef WLAN_NS_OFFLOAD
        //                                                                    
        wpalMemoryCopy(nsOffloadParams.srcIPv6Addr,
                        pwdiHostOffloadParams->wdiNsOffloadParams.srcIPv6Addr,
                        16);
        wpalMemoryCopy(nsOffloadParams.selfIPv6Addr,
                        pwdiHostOffloadParams->wdiNsOffloadParams.selfIPv6Addr,
                        16);
        wpalMemoryCopy(nsOffloadParams.targetIPv6Addr1,
                        pwdiHostOffloadParams->wdiNsOffloadParams.targetIPv6Addr1,
                        16);
        wpalMemoryCopy(nsOffloadParams.targetIPv6Addr2,
                        pwdiHostOffloadParams->wdiNsOffloadParams.targetIPv6Addr2,
                        16);
        wpalMemoryCopy(nsOffloadParams.selfMacAddr,
                        pwdiHostOffloadParams->wdiNsOffloadParams.selfMacAddr,
                        6);
        nsOffloadParams.srcIPv6AddrValid = pwdiHostOffloadParams->wdiNsOffloadParams.srcIPv6AddrValid;
        nsOffloadParams.targetIPv6Addr1Valid = pwdiHostOffloadParams->wdiNsOffloadParams.targetIPv6Addr1Valid;
        nsOffloadParams.targetIPv6Addr2Valid = pwdiHostOffloadParams->wdiNsOffloadParams.targetIPv6Addr2Valid;

        nsOffloadParams.bssIdx = pwdiHostOffloadParams->wdiNsOffloadParams.bssIdx;

#endif //                
   }

   //                                        
   wpalMemoryCopy( pSendBuffer+usDataOffset,
                   &hostOffloadParams,
                   sizeof(hostOffloadParams));

#ifdef WLAN_NS_OFFLOAD
   if( HAL_IPV6_NS_OFFLOAD == hostOffloadParams.offloadType )
   {
       //                                      
       wpalMemoryCopy( pSendBuffer+usDataOffset+sizeof(hostOffloadParams),
                       &nsOffloadParams,
                       sizeof(nsOffloadParams));
   }
#endif //                

   pWDICtx->wdiReqStatusCB     = pwdiHostOffloadParams->wdiReqStatusCB;
   pWDICtx->pReqStatusUserData = pwdiHostOffloadParams->pUserData;

   /*                                                                         
                                
                                                                            */
   return  WDI_SendMsg( pWDICtx, pSendBuffer, usSendSize,
                        wdiHostOffloadCb, pEventData->pUserData, WDI_HOST_OFFLOAD_RESP);
}/*                         */

/* 
                                                   
                                

                                                         
                                                                    

     
                                    
*/
WDI_Status
WDI_ProcessKeepAliveReq
(
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
)
{
   WDI_KeepAliveReqParamsType*  pwdiKeepAliveParams = NULL;
   WDI_KeepAliveCb              wdiKeepAliveCb = NULL;
   wpt_uint8*               pSendBuffer         = NULL;
   wpt_uint16               usDataOffset        = 0;
   wpt_uint16               usSendSize          = 0;
   tHalKeepAliveReq         keepAliveReq;
   /*                                                                        */

   /*                                                                         
                 
                                                                            */
   if (( NULL == pEventData ) ||
       ( NULL == (pwdiKeepAliveParams = (WDI_KeepAliveReqParamsType*)pEventData->pEventData)) ||
       ( NULL == (wdiKeepAliveCb   = (WDI_KeepAliveCb)pEventData->pCBfnc)))
   {
      WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_ERROR,
               "Invalid parameters in Keep Alive req");
      WDI_ASSERT(0);
      return WDI_STATUS_E_FAILURE;
   }

   /*                                                                       
                       
                                                                    
                                                                          */
   if (( WDI_STATUS_SUCCESS != WDI_GetMessageBuffer( pWDICtx, WDI_KEEP_ALIVE_REQ,
                         sizeof(keepAliveReq),
                         &pSendBuffer, &usDataOffset, &usSendSize))||
       ( usSendSize < (usDataOffset + sizeof(keepAliveReq) )))
   {
      WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_ERROR,
                  "Unable to get send buffer in keep alive req %x %x %x",
                  pEventData, pwdiKeepAliveParams, wdiKeepAliveCb);
      WDI_ASSERT(0);
      return WDI_STATUS_E_FAILURE;
   }

   keepAliveReq.packetType = pwdiKeepAliveParams->wdiKeepAliveInfo.ucPacketType;
   keepAliveReq.timePeriod = pwdiKeepAliveParams->wdiKeepAliveInfo.ucTimePeriod;

   keepAliveReq.bssIdx = pwdiKeepAliveParams->wdiKeepAliveInfo.bssIdx;

   if(pwdiKeepAliveParams->wdiKeepAliveInfo.ucPacketType == 2)
   {
   wpalMemoryCopy(keepAliveReq.hostIpv4Addr,
                     pwdiKeepAliveParams->wdiKeepAliveInfo.aHostIpv4Addr,
                     HAL_IPV4_ADDR_LEN);
   wpalMemoryCopy(keepAliveReq.destIpv4Addr,
                     pwdiKeepAliveParams->wdiKeepAliveInfo.aDestIpv4Addr,
                     HAL_IPV4_ADDR_LEN);
   wpalMemoryCopy(keepAliveReq.destMacAddr,
                     pwdiKeepAliveParams->wdiKeepAliveInfo.aDestMacAddr,
                     HAL_MAC_ADDR_LEN);
   }

   wpalMemoryCopy( pSendBuffer+usDataOffset,
                   &keepAliveReq,
                   sizeof(keepAliveReq));

   WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_INFO,
               "Process keep alive req %d",sizeof(keepAliveReq));

   WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_INFO,
               "Process keep alive req time period %d",keepAliveReq.timePeriod);

   pWDICtx->wdiReqStatusCB     = pwdiKeepAliveParams->wdiReqStatusCB;
   pWDICtx->pReqStatusUserData = pwdiKeepAliveParams->pUserData;

  WPAL_TRACE(eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_INFO,
                  "Sending keep alive req to HAL");

   /*                                                                         
                                
                                                                            */
   return  WDI_SendMsg( pWDICtx, pSendBuffer, usSendSize,
                        wdiKeepAliveCb, pEventData->pUserData, WDI_KEEP_ALIVE_RESP);
}/*                       */


/* 
                                                         
                                

                                                         
                                                                    

     
                                    
*/
WDI_Status
WDI_ProcessWowlAddBcPtrnReq
(
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
)
{
   WDI_WowlAddBcPtrnReqParamsType*  pwdiWowlAddBcPtrnParams = NULL;
   WDI_WowlAddBcPtrnCb              wdiWowlAddBcPtrnCb = NULL;
   wpt_uint8*               pSendBuffer         = NULL;
   wpt_uint16               usDataOffset        = 0;
   wpt_uint16               usSendSize          = 0;
   tHalWowlAddBcastPtrn     wowlAddBcPtrnReq;
   /*                                                                        */

   /*                                                                         
                 
                                                                            */
   if (( NULL == pEventData ) ||
       ( NULL == (pwdiWowlAddBcPtrnParams = (WDI_WowlAddBcPtrnReqParamsType*)pEventData->pEventData)) ||
       ( NULL == (wdiWowlAddBcPtrnCb   = (WDI_WowlAddBcPtrnCb)pEventData->pCBfnc)))
   {
      WPAL_TRACE( eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_WARN,
                  "%s: Invalid parameters", __FUNCTION__);
      WDI_ASSERT(0);
      return WDI_STATUS_E_FAILURE;
   }

   /*                                                                       
                       
                                                                    
                                                                          */
   if (( WDI_STATUS_SUCCESS != WDI_GetMessageBuffer( pWDICtx, WDI_WOWL_ADD_BC_PTRN_REQ,
                         sizeof(wowlAddBcPtrnReq),
                         &pSendBuffer, &usDataOffset, &usSendSize))||
       ( usSendSize < (usDataOffset + sizeof(wowlAddBcPtrnReq) )))
   {
      WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_WARN,
                  "Unable to get send buffer in Wowl add bc ptrn req %x %x %x",
                  pEventData, pwdiWowlAddBcPtrnParams, wdiWowlAddBcPtrnCb);
      WDI_ASSERT(0);
      return WDI_STATUS_E_FAILURE;
   }

   wowlAddBcPtrnReq.ucPatternId =
      pwdiWowlAddBcPtrnParams->wdiWowlAddBcPtrnInfo.ucPatternId;
   wowlAddBcPtrnReq.ucPatternByteOffset =
      pwdiWowlAddBcPtrnParams->wdiWowlAddBcPtrnInfo.ucPatternByteOffset;
   wowlAddBcPtrnReq.ucPatternMaskSize =
      pwdiWowlAddBcPtrnParams->wdiWowlAddBcPtrnInfo.ucPatternMaskSize;
   wowlAddBcPtrnReq.ucPatternSize =
      pwdiWowlAddBcPtrnParams->wdiWowlAddBcPtrnInfo.ucPatternSize;

   if (pwdiWowlAddBcPtrnParams->wdiWowlAddBcPtrnInfo.ucPatternSize <= HAL_WOWL_BCAST_PATTERN_MAX_SIZE)
   {
       wpalMemoryCopy(wowlAddBcPtrnReq.ucPattern,
                      pwdiWowlAddBcPtrnParams->wdiWowlAddBcPtrnInfo.ucPattern,
                      pwdiWowlAddBcPtrnParams->wdiWowlAddBcPtrnInfo.ucPatternSize);
       wpalMemoryCopy(wowlAddBcPtrnReq.ucPatternMask,
                      pwdiWowlAddBcPtrnParams->wdiWowlAddBcPtrnInfo.ucPatternMask,
                      pwdiWowlAddBcPtrnParams->wdiWowlAddBcPtrnInfo.ucPatternMaskSize);
   }
   else
   {
       wpalMemoryCopy(wowlAddBcPtrnReq.ucPattern,
                      pwdiWowlAddBcPtrnParams->wdiWowlAddBcPtrnInfo.ucPattern,
                      HAL_WOWL_BCAST_PATTERN_MAX_SIZE);
       wpalMemoryCopy(wowlAddBcPtrnReq.ucPatternMask,
                      pwdiWowlAddBcPtrnParams->wdiWowlAddBcPtrnInfo.ucPatternMask,
                      HAL_WOWL_BCAST_PATTERN_MAX_SIZE);

       wpalMemoryCopy(wowlAddBcPtrnReq.ucPattern,
                      pwdiWowlAddBcPtrnParams->wdiWowlAddBcPtrnInfo.ucPattern,
                      pwdiWowlAddBcPtrnParams->wdiWowlAddBcPtrnInfo.ucPatternMaskSize - HAL_WOWL_BCAST_PATTERN_MAX_SIZE);
       wpalMemoryCopy(wowlAddBcPtrnReq.ucPatternMask,
                      pwdiWowlAddBcPtrnParams->wdiWowlAddBcPtrnInfo.ucPatternMask,
                      pwdiWowlAddBcPtrnParams->wdiWowlAddBcPtrnInfo.ucPatternMaskSize - HAL_WOWL_BCAST_PATTERN_MAX_SIZE);
   }

   wpalMemoryCopy( pSendBuffer+usDataOffset,
                   &wowlAddBcPtrnReq,
                   sizeof(wowlAddBcPtrnReq));

   pWDICtx->wdiReqStatusCB     = pwdiWowlAddBcPtrnParams->wdiReqStatusCB;
   pWDICtx->pReqStatusUserData = pwdiWowlAddBcPtrnParams->pUserData;

   /*                                                                         
                                
                                                                            */
   return  WDI_SendMsg( pWDICtx, pSendBuffer, usSendSize,
                        wdiWowlAddBcPtrnCb, pEventData->pUserData, WDI_WOWL_ADD_BC_PTRN_RESP);
}/*                           */

/* 
                                                            
                                

                                                         
                                                                    

     
                                    
*/
WDI_Status
WDI_ProcessWowlDelBcPtrnReq
(
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
)
{
   WDI_WowlDelBcPtrnReqParamsType*  pwdiWowlDelBcPtrnParams = NULL;
   WDI_WowlDelBcPtrnCb              wdiWowlDelBcPtrnCb = NULL;
   wpt_uint8*               pSendBuffer         = NULL;
   wpt_uint16               usDataOffset        = 0;
   wpt_uint16               usSendSize          = 0;
   tHalWowlDelBcastPtrn     wowlDelBcPtrnReq;
   /*                                                                        */

   /*                                                                         
                 
                                                                            */
   if (( NULL == pEventData ) ||
       ( NULL == (pwdiWowlDelBcPtrnParams = (WDI_WowlDelBcPtrnReqParamsType*)pEventData->pEventData)) ||
       ( NULL == (wdiWowlDelBcPtrnCb   = (WDI_WowlDelBcPtrnCb)pEventData->pCBfnc)))
   {
      WPAL_TRACE( eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_WARN,
                  "%s: Invalid parameters", __FUNCTION__);
      WDI_ASSERT(0);
      return WDI_STATUS_E_FAILURE;
   }

   /*                                                                       
                       
                                                                    
                                                                          */
   if (( WDI_STATUS_SUCCESS != WDI_GetMessageBuffer( pWDICtx, WDI_WOWL_DEL_BC_PTRN_REQ,
                         sizeof(wowlDelBcPtrnReq),
                         &pSendBuffer, &usDataOffset, &usSendSize))||
       ( usSendSize < (usDataOffset + sizeof(wowlDelBcPtrnReq) )))
   {
      WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_WARN,
                  "Unable to get send buffer in Wowl del bc ptrn req %x %x %x",
                  pEventData, pwdiWowlDelBcPtrnParams, wdiWowlDelBcPtrnCb);
      WDI_ASSERT(0);
      return WDI_STATUS_E_FAILURE;
   }

   wowlDelBcPtrnReq.ucPatternId =
      pwdiWowlDelBcPtrnParams->wdiWowlDelBcPtrnInfo.ucPatternId;
   wpalMemoryCopy( pSendBuffer+usDataOffset,
                   &wowlDelBcPtrnReq,
                   sizeof(wowlDelBcPtrnReq));

   pWDICtx->wdiReqStatusCB     = pwdiWowlDelBcPtrnParams->wdiReqStatusCB;
   pWDICtx->pReqStatusUserData = pwdiWowlDelBcPtrnParams->pUserData;

   /*                                                                         
                                
                                                                            */
   return  WDI_SendMsg( pWDICtx, pSendBuffer, usSendSize,
                        wdiWowlDelBcPtrnCb, pEventData->pUserData, WDI_WOWL_DEL_BC_PTRN_RESP);
}/*                           */

/* 
                                                   
                                

                                                         
                                                                    

     
                                    
*/
WDI_Status
WDI_ProcessWowlEnterReq
(
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
)
{
   WDI_WowlEnterReqParamsType*  pwdiWowlEnterParams = NULL;
   WDI_WowlEnterReqCb           wdiWowlEnterCb = NULL;
   wpt_uint8*               pSendBuffer         = NULL;
   wpt_uint16               usDataOffset        = 0;
   wpt_uint16               usSendSize          = 0;
   tHalWowlEnterParams      wowlEnterReq;
   /*                                                                        */

   /*                                                                         
                 
                                                                            */
   if (( NULL == pEventData ) ||
       ( NULL == (pwdiWowlEnterParams = (WDI_WowlEnterReqParamsType*)pEventData->pEventData)) ||
       ( NULL == (wdiWowlEnterCb   = (WDI_WowlEnterReqCb)pEventData->pCBfnc)))
   {
      WPAL_TRACE( eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_WARN,
                  "%s: Invalid parameters", __FUNCTION__);
      WDI_ASSERT(0);
      return WDI_STATUS_E_FAILURE;
   }

   /*                                                                       
                       
                                                                    
                                                                          */
   if (( WDI_STATUS_SUCCESS != WDI_GetMessageBuffer( pWDICtx, WDI_WOWL_ENTER_REQ,
                         sizeof(wowlEnterReq),
                         &pSendBuffer, &usDataOffset, &usSendSize))||
       ( usSendSize < (usDataOffset + sizeof(wowlEnterReq) )))
   {
      WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_WARN,
                  "Unable to get send buffer in Wowl enter req %x %x %x",
                  pEventData, pwdiWowlEnterParams, wdiWowlEnterCb);
      WDI_ASSERT(0);
      return WDI_STATUS_E_FAILURE;
   }

   wowlEnterReq.ucMagicPktEnable =
      pwdiWowlEnterParams->wdiWowlEnterInfo.ucMagicPktEnable;
   wowlEnterReq.ucPatternFilteringEnable =
      pwdiWowlEnterParams->wdiWowlEnterInfo.ucPatternFilteringEnable;
   wowlEnterReq.ucUcastPatternFilteringEnable =
      pwdiWowlEnterParams->wdiWowlEnterInfo.ucUcastPatternFilteringEnable;
   wowlEnterReq.ucWowChnlSwitchRcv =
      pwdiWowlEnterParams->wdiWowlEnterInfo.ucWowChnlSwitchRcv;
   wowlEnterReq.ucWowDeauthRcv =
      pwdiWowlEnterParams->wdiWowlEnterInfo.ucWowDeauthRcv;
   wowlEnterReq.ucWowDisassocRcv =
      pwdiWowlEnterParams->wdiWowlEnterInfo.ucWowDisassocRcv;
   wowlEnterReq.ucWowMaxMissedBeacons =
      pwdiWowlEnterParams->wdiWowlEnterInfo.ucWowMaxMissedBeacons;
   wowlEnterReq.ucWowMaxSleepUsec =
      pwdiWowlEnterParams->wdiWowlEnterInfo.ucWowMaxSleepUsec;

#ifdef WLAN_WAKEUP_EVENTS
   wowlEnterReq.ucWoWEAPIDRequestEnable =
      pwdiWowlEnterParams->wdiWowlEnterInfo.ucWoWEAPIDRequestEnable;

   wowlEnterReq.ucWoWEAPOL4WayEnable =
      pwdiWowlEnterParams->wdiWowlEnterInfo.ucWoWEAPOL4WayEnable;

   wowlEnterReq.ucWowNetScanOffloadMatch =
      pwdiWowlEnterParams->wdiWowlEnterInfo.ucWowNetScanOffloadMatch;

   wowlEnterReq.ucWowGTKRekeyError =
      pwdiWowlEnterParams->wdiWowlEnterInfo.ucWowGTKRekeyError;

   wowlEnterReq.ucWoWBSSConnLoss =
      pwdiWowlEnterParams->wdiWowlEnterInfo.ucWoWBSSConnLoss;
#endif //                   

   wpalMemoryCopy(wowlEnterReq.magicPtrn,
                  pwdiWowlEnterParams->wdiWowlEnterInfo.magicPtrn,
                  sizeof(tSirMacAddr));

   wpalMemoryCopy( pSendBuffer+usDataOffset,
                   &wowlEnterReq,
                   sizeof(wowlEnterReq));

   pWDICtx->wdiReqStatusCB     = pwdiWowlEnterParams->wdiReqStatusCB;
   pWDICtx->pReqStatusUserData = pwdiWowlEnterParams->pUserData;

   /*                                                                         
                                
                                                                            */
   return  WDI_SendMsg( pWDICtx, pSendBuffer, usSendSize,
                        wdiWowlEnterCb, pEventData->pUserData, WDI_WOWL_ENTER_RESP);
}/*                       */

/* 
                                                                
                  

                                                         
                                                                    

     
                                    
*/
WDI_Status
WDI_ProcessWowlExitReq
(
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
)
{
   WDI_WowlExitReqCb           wdiWowlExitCb = NULL;
   wpt_uint8*               pSendBuffer         = NULL;
   wpt_uint16               usDataOffset        = 0;
   wpt_uint16               usSendSize          = 0;
   /*                                                                        */

   /*                                                                         
                 
                                                                            */
   if (( NULL == pEventData ) ||
       ( NULL == (wdiWowlExitCb   = (WDI_WowlExitReqCb)pEventData->pCBfnc)))
   {
      WPAL_TRACE( eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_WARN,
                  "%s: Invalid parameters", __FUNCTION__);
      WDI_ASSERT(0);
      return WDI_STATUS_E_FAILURE;
   }

   /*                                                                       
                       
                                                                    
                                                                          */
   if (( WDI_STATUS_SUCCESS != WDI_GetMessageBuffer( pWDICtx, WDI_WOWL_EXIT_REQ,
                                                     0,
                                                     &pSendBuffer, &usDataOffset, &usSendSize))||
       ( usSendSize < (usDataOffset )))
   {
      WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_WARN,
               "Unable to get send buffer in Wowl Exit req %x %x",
                 pEventData, wdiWowlExitCb);
      WDI_ASSERT(0);
      return WDI_STATUS_E_FAILURE;
   }

   /*                                                                         
                                
                                                                            */
   return  WDI_SendMsg( pWDICtx, pSendBuffer, usSendSize,
                        wdiWowlExitCb, pEventData->pUserData, WDI_WOWL_EXIT_RESP);
}/*                      */

/* 
                                                                
                                        

                                                         
                                                                    

     
                                    
*/
WDI_Status
WDI_ProcessConfigureAppsCpuWakeupStateReq
(
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
)
{
   WDI_ConfigureAppsCpuWakeupStateReqParamsType*  pwdiAppsCpuWakeupStateParams = NULL;
   WDI_ConfigureAppsCpuWakeupStateCb              wdiConfigureAppsCpuWakeupStateCb = NULL;
   wpt_uint8*               pSendBuffer         = NULL;
   wpt_uint16               usDataOffset        = 0;
   wpt_uint16               usSendSize          = 0;
   tHalConfigureAppsCpuWakeupStateReqParams  halCfgAppsCpuWakeupStateReqParams;
   /*                                                                        */

   /*                                                                         
                 
                                                                            */
   if (( NULL == pEventData ) ||
       ( NULL == (pwdiAppsCpuWakeupStateParams = (WDI_ConfigureAppsCpuWakeupStateReqParamsType*)pEventData->pEventData)) ||
       ( NULL == (wdiConfigureAppsCpuWakeupStateCb   = (WDI_ConfigureAppsCpuWakeupStateCb)pEventData->pCBfnc)))
   {
      WPAL_TRACE( eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_WARN,
                  "%s: Invalid parameters", __FUNCTION__);
      WDI_ASSERT(0);
      return WDI_STATUS_E_FAILURE;
   }

   /*                                                                       
                       
                                                                          */
   if (( WDI_STATUS_SUCCESS != WDI_GetMessageBuffer( pWDICtx, WDI_CONFIGURE_APPS_CPU_WAKEUP_STATE_REQ,
                         sizeof(halCfgAppsCpuWakeupStateReqParams),
                         &pSendBuffer, &usDataOffset, &usSendSize))||
       ( usSendSize < (usDataOffset + sizeof(pwdiAppsCpuWakeupStateParams->bIsAppsAwake) )))
   {
      WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_WARN,
               "Unable to get send buffer in Apps CPU Wakeup State req %x %x %x",
                 pEventData, pwdiAppsCpuWakeupStateParams, wdiConfigureAppsCpuWakeupStateCb);
      WDI_ASSERT(0);
      return WDI_STATUS_E_FAILURE;
   }

   halCfgAppsCpuWakeupStateReqParams.isAppsCpuAwake =
                           pwdiAppsCpuWakeupStateParams->bIsAppsAwake;

   wpalMemoryCopy( pSendBuffer+usDataOffset,
                   &halCfgAppsCpuWakeupStateReqParams,
                   sizeof(halCfgAppsCpuWakeupStateReqParams));

   pWDICtx->wdiReqStatusCB     = pwdiAppsCpuWakeupStateParams->wdiReqStatusCB;
   pWDICtx->pReqStatusUserData = pwdiAppsCpuWakeupStateParams->pUserData;

   /*                                                                         
                                
                                                                            */
   return  WDI_SendMsg( pWDICtx, pSendBuffer, usSendSize,
                        wdiConfigureAppsCpuWakeupStateCb, pEventData->pUserData,
                        WDI_CONFIGURE_APPS_CPU_WAKEUP_STATE_RESP);
}/*                                         */

#ifdef WLAN_FEATURE_VOWIFI_11R
/* 
                                                                           
                  

                                                         
                                                                    

     
                                    
*/
WDI_Status
WDI_ProcessAggrAddTSpecReq
(
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
)
{
  WDI_AggrAddTSReqParamsType*  pwdiAggrAddTSParams;
  WDI_AggrAddTsRspCb           wdiAggrAddTSRspCb;
  wpt_uint8                ucCurrentBSSSesIdx  = 0;
  WDI_BSSSessionType*      pBSSSes             = NULL;
  wpt_uint8*               pSendBuffer         = NULL;
  wpt_uint16               usDataOffset        = 0;
  wpt_uint16               usSendSize          = 0;
  WDI_Status               wdiStatus           = WDI_STATUS_SUCCESS;
  wpt_macAddr              macBSSID;
  tAggrAddTsReq            halAggrAddTsReq;
  int i;
  /*                                                                        */

  /*                                                                         
                
                                                                           */
  if (( NULL == pEventData ) || ( NULL == pEventData->pEventData ) ||
      ( NULL == pEventData->pCBfnc ))
  {
     WPAL_TRACE( eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_WARN,
                 "%s: Invalid parameters", __FUNCTION__);
     WDI_ASSERT(0);
     return WDI_STATUS_E_FAILURE;
  }
  wpalMemoryFill( &halAggrAddTsReq, sizeof(tAggrAddTsReq), 0 );
  pwdiAggrAddTSParams = (WDI_AggrAddTSReqParamsType*)pEventData->pEventData;
  wdiAggrAddTSRspCb   = (WDI_AggrAddTsRspCb)pEventData->pCBfnc;
  /*                                                                         
                                                                           
                                              
                                                                           */
  wpalMutexAcquire(&pWDICtx->wptMutex);

  /*                                                                        
                                                                       
                                                                          */
  if ( WDI_STATUS_SUCCESS != WDI_STATableGetStaBSSIDAddr(pWDICtx,
                                        pwdiAggrAddTSParams->wdiAggrTsInfo.ucSTAIdx,
                                        &macBSSID))
  {
    WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_WARN,
             "This station does not exist in the WDI Station Table %d");
    wpalMutexRelease(&pWDICtx->wptMutex);
    return WDI_STATUS_E_FAILURE;
  }

  ucCurrentBSSSesIdx = WDI_FindAssocSession( pWDICtx, macBSSID, &pBSSSes);
  if ( NULL == pBSSSes )
  {
    WPAL_TRACE( eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_ERROR,
        "%s: Association sequence for this BSS does not yet exist. macBSSID " MAC_ADDRESS_STR,
        __func__, MAC_ADDR_ARRAY(macBSSID));

    wpalMutexRelease(&pWDICtx->wptMutex);
    return WDI_STATUS_E_NOT_ALLOWED;
  }

  /*                                                                        
                                                             
                                             
                                                                          */
  if ( eWLAN_PAL_TRUE == pBSSSes->bAssocReqQueued )
  {
    WPAL_TRACE( eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_WARN,
              "%s: Association sequence for this BSS exists but currently queued. macBSSID " MAC_ADDRESS_STR,
              __func__, MAC_ADDR_ARRAY(macBSSID));

    wdiStatus = WDI_QueueAssocRequest( pWDICtx, pBSSSes, pEventData);
    wpalMutexRelease(&pWDICtx->wptMutex);
    return wdiStatus;
  }

  wpalMutexRelease(&pWDICtx->wptMutex);
  /*                                                                       
                      
                                                                   
                                                                         */
  if (( WDI_STATUS_SUCCESS != WDI_GetMessageBuffer( pWDICtx, WDI_AGGR_ADD_TS_REQ,
                        sizeof(tAggrAddTsParams),
                        &pSendBuffer, &usDataOffset, &usSendSize))||
      ( usSendSize < (usDataOffset + sizeof(tAggrAddTsParams) )))
  {
     WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_WARN,
              "Unable to get send buffer in set bss key req %x %x %x",
                pEventData, pwdiAggrAddTSParams, wdiAggrAddTSRspCb);
     WDI_ASSERT(0);
     return WDI_STATUS_E_FAILURE;
  }

  halAggrAddTsReq.aggrAddTsParam.staIdx =
     pwdiAggrAddTSParams->wdiAggrTsInfo.ucSTAIdx;
  halAggrAddTsReq.aggrAddTsParam.tspecIdx =
     pwdiAggrAddTSParams->wdiAggrTsInfo.ucTspecIdx;

  for( i = 0; i < WLAN_HAL_MAX_AC; i++ )
  {
     halAggrAddTsReq.aggrAddTsParam.tspec[i].type =
        pwdiAggrAddTSParams->wdiAggrTsInfo.wdiTspecIE[i].ucType;
     halAggrAddTsReq.aggrAddTsParam.tspec[i].length =
        pwdiAggrAddTSParams->wdiAggrTsInfo.wdiTspecIE[i].ucLength;
     halAggrAddTsReq.aggrAddTsParam.tspec[i].tsinfo.traffic.ackPolicy =
        pwdiAggrAddTSParams->wdiAggrTsInfo.wdiTspecIE[i].wdiTSinfo.wdiTraffic.
        ackPolicy;
     halAggrAddTsReq.aggrAddTsParam.tspec[i].tsinfo.traffic.accessPolicy =
        pwdiAggrAddTSParams->wdiAggrTsInfo.wdiTspecIE[i].wdiTSinfo.wdiTraffic.
        accessPolicy;
     halAggrAddTsReq.aggrAddTsParam.tspec[i].tsinfo.traffic.userPrio =
        pwdiAggrAddTSParams->wdiAggrTsInfo.wdiTspecIE[i].wdiTSinfo.wdiTraffic.
        userPrio;
     halAggrAddTsReq.aggrAddTsParam.tspec[i].tsinfo.traffic.psb =
        pwdiAggrAddTSParams->wdiAggrTsInfo.wdiTspecIE[i].wdiTSinfo.wdiTraffic.
        psb;
     halAggrAddTsReq.aggrAddTsParam.tspec[i].tsinfo.traffic.aggregation =
        pwdiAggrAddTSParams->wdiAggrTsInfo.wdiTspecIE[i].wdiTSinfo.wdiTraffic.
        aggregation;
     halAggrAddTsReq.aggrAddTsParam.tspec[i].tsinfo.traffic.direction =
        pwdiAggrAddTSParams->wdiAggrTsInfo.wdiTspecIE[i].wdiTSinfo.wdiTraffic.
        direction;
     halAggrAddTsReq.aggrAddTsParam.tspec[i].tsinfo.traffic.tsid =
        pwdiAggrAddTSParams->wdiAggrTsInfo.wdiTspecIE[i].wdiTSinfo.wdiTraffic.
        trafficType;
     halAggrAddTsReq.aggrAddTsParam.tspec[i].tsinfo.traffic.tsid =
        pwdiAggrAddTSParams->wdiAggrTsInfo.wdiTspecIE[i].wdiTSinfo.wdiTraffic.
        trafficType;
     halAggrAddTsReq.aggrAddTsParam.tspec[i].tsinfo.schedule.rsvd =
        pwdiAggrAddTSParams->wdiAggrTsInfo.wdiTspecIE[i].wdiTSinfo.wdiSchedule.rsvd;
     halAggrAddTsReq.aggrAddTsParam.tspec[i].tsinfo.schedule.schedule =
        pwdiAggrAddTSParams->wdiAggrTsInfo.wdiTspecIE[i].wdiTSinfo.wdiSchedule.schedule;


     halAggrAddTsReq.aggrAddTsParam.tspec[i].nomMsduSz =
        pwdiAggrAddTSParams->wdiAggrTsInfo.wdiTspecIE[i].usNomMsduSz;
     halAggrAddTsReq.aggrAddTsParam.tspec[i].maxMsduSz =
        pwdiAggrAddTSParams->wdiAggrTsInfo.wdiTspecIE[i].usMaxMsduSz;
     halAggrAddTsReq.aggrAddTsParam.tspec[i].minSvcInterval =
        pwdiAggrAddTSParams->wdiAggrTsInfo.wdiTspecIE[i].uMinSvcInterval;
     halAggrAddTsReq.aggrAddTsParam.tspec[i].maxSvcInterval =
        pwdiAggrAddTSParams->wdiAggrTsInfo.wdiTspecIE[i].uMaxSvcInterval;
     halAggrAddTsReq.aggrAddTsParam.tspec[i].inactInterval =
        pwdiAggrAddTSParams->wdiAggrTsInfo.wdiTspecIE[i].uInactInterval;
     halAggrAddTsReq.aggrAddTsParam.tspec[i].suspendInterval =
        pwdiAggrAddTSParams->wdiAggrTsInfo.wdiTspecIE[i].uSuspendInterval;
     halAggrAddTsReq.aggrAddTsParam.tspec[i].svcStartTime =
        pwdiAggrAddTSParams->wdiAggrTsInfo.wdiTspecIE[i].uSvcStartTime;
     halAggrAddTsReq.aggrAddTsParam.tspec[i].minDataRate =
        pwdiAggrAddTSParams->wdiAggrTsInfo.wdiTspecIE[i].uMinDataRate;
     halAggrAddTsReq.aggrAddTsParam.tspec[i].meanDataRate =
        pwdiAggrAddTSParams->wdiAggrTsInfo.wdiTspecIE[i].uMeanDataRate;
     halAggrAddTsReq.aggrAddTsParam.tspec[i].peakDataRate =
        pwdiAggrAddTSParams->wdiAggrTsInfo.wdiTspecIE[i].uPeakDataRate;
     halAggrAddTsReq.aggrAddTsParam.tspec[i].maxBurstSz =
        pwdiAggrAddTSParams->wdiAggrTsInfo.wdiTspecIE[i].uMaxBurstSz;
     halAggrAddTsReq.aggrAddTsParam.tspec[i].delayBound =
        pwdiAggrAddTSParams->wdiAggrTsInfo.wdiTspecIE[i].uDelayBound;
     halAggrAddTsReq.aggrAddTsParam.tspec[i].minPhyRate =
        pwdiAggrAddTSParams->wdiAggrTsInfo.wdiTspecIE[i].uMinPhyRate;
     halAggrAddTsReq.aggrAddTsParam.tspec[i].surplusBw =
        pwdiAggrAddTSParams->wdiAggrTsInfo.wdiTspecIE[i].usSurplusBw;
     halAggrAddTsReq.aggrAddTsParam.tspec[i].mediumTime =
        pwdiAggrAddTSParams->wdiAggrTsInfo.wdiTspecIE[i].usMediumTime;
  }

  wpalMemoryCopy( pSendBuffer+usDataOffset,
                  &halAggrAddTsReq,
                  sizeof(halAggrAddTsReq));

  pWDICtx->wdiReqStatusCB     = pwdiAggrAddTSParams->wdiReqStatusCB;
  pWDICtx->pReqStatusUserData = pwdiAggrAddTSParams->pUserData;

  /*                                                                         
                              
                                                                           */
  return  WDI_SendMsg( pWDICtx, pSendBuffer, usSendSize,
                       wdiAggrAddTSRspCb, pEventData->pUserData,
                       WDI_AGGR_ADD_TS_RESP);
}/*                          */
#endif /*                         */

/* 
                                                               
                  

                                                         
                                                                    

     
                                    
*/
WDI_Status
WDI_ProcessShutdownReq
(
 WDI_ControlBlockType*  pWDICtx,
 WDI_EventInfoType*     pEventData
 )
{
   wpt_status              wptStatus;


   /*                                                                        */

   /*                                                                         
                 
                                                                              */
   if ( NULL == pEventData )
   {
      WPAL_TRACE( eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_WARN,
            "%s: Invalid parameters", __FUNCTION__);
      WDI_ASSERT(0);
      return WDI_STATUS_E_FAILURE;
   }

   wpalMutexAcquire(&pWDICtx->wptMutex);


   gWDIInitialized = eWLAN_PAL_FALSE;
   /*                                */
   if ( eDRIVER_TYPE_MFG != pWDICtx->driverMode )
   {
      /*                                              
                                                                                   */
      WDI_STATableStop(pWDICtx);

      /*                            */
      WDTS_Stop(pWDICtx);
   }

   /*                         */
   WDI_ClearPendingRequests(pWDICtx);
   /*                     */
   /*                                  */
   if ( eDRIVER_TYPE_MFG != pWDICtx->driverMode )
   {
      WDTS_Close(pWDICtx);
   }
   /*                                               */
   WDI_STATableClose(pWDICtx);
   /*              */
   wptStatus = wpalClose(pWDICtx->pPALContext);
   if ( eWLAN_PAL_STATUS_SUCCESS !=  wptStatus )
   {
      WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_ERROR,
            "Failed to wpal Close %d", wptStatus);
      WDI_ASSERT(0);
   }

   /*                             */
   WDI_STATE_TRANSITION( pWDICtx, WDI_INIT_ST);

   wpalMutexRelease(&pWDICtx->wptMutex);

   /*                                                          */
   pWDICtx->ucExpectedStateTransition = WDI_INIT_ST;


   return WDI_STATUS_SUCCESS;
}/*                      */

/*                                                                        
                                                       
                                                                        */

/* 
                                                               
                                                

                                                         
                                                                    

     
                                    
*/
WDI_Status
WDI_ProcessStartRsp
(
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
)
{
  WDI_StartRspParamsType   wdiRspParams;
  WDI_StartRspCb           wdiStartRspCb = NULL;

  tHalMacStartRspParams*   startRspParams;

#ifndef HAL_SELF_STA_PER_BSS
  WDI_AddStaParams         wdiAddSTAParam = {0};
#endif
  /*                                                                        */

  wdiStartRspCb = (WDI_StartRspCb)pWDICtx->pfncRspCB;
  /*                                                                         
                
                                                                           */
  if (( NULL == pEventData ) ||
      ( NULL == pEventData->pEventData) ||
      ( NULL == wdiStartRspCb ))
  {
     WPAL_TRACE( eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_WARN,
                 "%s: Invalid parameters", __FUNCTION__);
     WDI_ASSERT(0);
     return WDI_STATUS_E_FAILURE;
  }

  /*                                                                         
                                        
                                                                           */
  if ( sizeof(tHalMacStartRspParams) > pEventData->uEventDataSize )
  {
     //                             
     WPAL_TRACE( eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_FATAL,
                 "Invalid response length in Start Resp Expect %x Rcvd %x",
                 sizeof(tHalMacStartRspParams), pEventData->uEventDataSize);
     WDI_ASSERT(0);
     return WDI_STATUS_E_FAILURE;
  }

  /*                                                                         
                                                                         
                                    
                                                                           */
  startRspParams = (tHalMacStartRspParams *) pEventData->pEventData;

  wdiRspParams.ucMaxBssids   = startRspParams->ucMaxBssids;
  wdiRspParams.ucMaxStations = startRspParams->ucMaxStations;
  wdiRspParams.wlanCompiledVersion.major = WLAN_HAL_VER_MAJOR;
  wdiRspParams.wlanCompiledVersion.minor = WLAN_HAL_VER_MINOR;
  wdiRspParams.wlanCompiledVersion.version = WLAN_HAL_VER_VERSION;
  wdiRspParams.wlanCompiledVersion.revision = WLAN_HAL_VER_REVISION;
  wdiRspParams.wlanReportedVersion.major =
                               startRspParams->wcnssWlanVersion.major;
  wdiRspParams.wlanReportedVersion.minor =
                               startRspParams->wcnssWlanVersion.minor;
  wdiRspParams.wlanReportedVersion.version =
                               startRspParams->wcnssWlanVersion.version;
  wdiRspParams.wlanReportedVersion.revision =
                               startRspParams->wcnssWlanVersion.revision;
  wpalMemoryCopy(wdiRspParams.wcnssSoftwareVersion,
          startRspParams->wcnssCrmVersionString,
          sizeof(wdiRspParams.wcnssSoftwareVersion));
  wpalMemoryCopy(wdiRspParams.wcnssHardwareVersion,
          startRspParams->wcnssWlanVersionString,
          sizeof(wdiRspParams.wcnssHardwareVersion));
  wdiRspParams.wdiStatus     = WDI_HAL_2_WDI_STATUS(startRspParams->status);

  wpalMutexAcquire(&pWDICtx->wptMutex);
  if ( WDI_STATUS_SUCCESS == wdiRspParams.wdiStatus  )
  {
    pWDICtx->ucExpectedStateTransition =  WDI_STARTED_ST;

    /*                                        */
    wpalMemoryCopy( &pWDICtx->wdiCachedStartRspParams ,
                  &wdiRspParams,
                  sizeof(pWDICtx->wdiCachedStartRspParams));

  }
  else
  {
    WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_ERROR,
               "Failed to start device with status %s(%d)",
               WDI_getHALStatusMsgString(startRspParams->status),
               startRspParams->status);

    /*                                                                    
            */
    pWDICtx->ucExpectedStateTransition =  WDI_STOPPED_ST;

    wpalMutexRelease(&pWDICtx->wptMutex);

     /*           */
    wdiStartRspCb( &wdiRspParams, pWDICtx->pRspCBUserData);

    WDI_DetectedDeviceError(pWDICtx, wdiRspParams.wdiStatus);

    /*                                                                    
                                                                               */
    return WDI_STATUS_SUCCESS;
  }

  wpalMutexRelease(&pWDICtx->wptMutex);

  if(eDRIVER_TYPE_MFG == pWDICtx->driverMode)
  {
    /*                                         */
    /*             */
    wdiStartRspCb( &wdiRspParams, pWDICtx->pRspCBUserData);
    return WDI_STATUS_SUCCESS;
  }

  /*                          */
  WDTS_startTransport(pWDICtx);

  /*                                             */
  WDI_STATableStart(pWDICtx);

#ifndef HAL_SELF_STA_PER_BSS
  /*                          */
  pWDICtx->ucSelfStaId = halStartRspMsg.startRspParams.selfStaIdx;

  pWDICtx->usSelfStaDpuId = wdiRspParams.usSelfStaDpuId;
  wpalMemoryCopy(pWDICtx->macSelfSta, wdiRspParams.macSelfSta,
                 WDI_MAC_ADDR_LEN);

  /*                                */

  /*                                              */
  /*                                          */
  /*                               */
  /*                                     */
  /*                                    */
  /*                                     */

  //                                         
  wdiAddSTAParam.bcastDpuIndex = wdiRspParams.usSelfStaDpuId;
  wdiAddSTAParam.bcastMgmtDpuIndex = wdiRspParams.usSelfStaDpuId;
  wdiAddSTAParam.dpuIndex = wdiRspParams.usSelfStaDpuId;;
  wpalMemoryCopy(wdiAddSTAParam.staMacAddr, wdiRspParams.macSelfSta,
                 WDI_MAC_ADDR_LEN);
  wdiAddSTAParam.ucStaType = WDI_STA_ENTRY_SELF; /*          */
  wdiAddSTAParam.ucSTAIdx = halStartRspMsg.startRspParams.selfStaIdx;

  /*                                                                         
                                                                        
                                                                                
             */
  (void)WDI_STATableAddSta(pWDICtx,&wdiAddSTAParam);
#endif

  /*           */
  wdiStartRspCb( &wdiRspParams, pWDICtx->pRspCBUserData);

  return WDI_STATUS_SUCCESS;
}/*                   */


/* 
                                                              
                                                

                                                         
                                                                    

     
                                    
*/
WDI_Status
WDI_ProcessStopRsp
(
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
)
{
  WDI_Status          wdiStatus;
  WDI_StopRspCb       wdiStopRspCb = NULL;

  tHalMacStopRspMsg   halMacStopRspMsg;
  /*                                                                        */

  wdiStopRspCb = (WDI_StopRspCb)pWDICtx->pfncRspCB;
  /*                                                                         
                
                                                                           */
  if (( NULL == pEventData ) ||
      ( NULL == pEventData->pEventData) ||
      ( NULL == wdiStopRspCb ))
  {
     WPAL_TRACE( eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_WARN,
                 "%s: Invalid parameters", __FUNCTION__);
     WDI_ASSERT(0);
     return WDI_STATUS_E_FAILURE;
  }

  /*                                                                         
                                        
                                                                           */
  if ( sizeof(halMacStopRspMsg) < pEventData->uEventDataSize )
  {
     WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_WARN,
              "Invalid response length in Stop Resp %x %x",
                pEventData->uEventDataSize);
     WDI_ASSERT(0);
     return WDI_STATUS_E_FAILURE;
  }

  /*                                                                         
                                                                         
                                    
                                                                           */
  wpalMemoryCopy( &halMacStopRspMsg.stopRspParams,
                  pEventData->pEventData,
                  sizeof(halMacStopRspMsg.stopRspParams));

  wdiStatus   =   WDI_HAL_2_WDI_STATUS(halMacStopRspMsg.stopRspParams.status);

  wpalMutexAcquire(&pWDICtx->wptMutex);

  /*                                                                          
                                    
                                                                            */
  if ( WDI_STATUS_SUCCESS != wdiStatus  )
  {
    WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_ERROR,
               "Failed to stop the device with status %s (%d)",
               WDI_getHALStatusMsgString(halMacStopRspMsg.stopRspParams.status),
               halMacStopRspMsg.stopRspParams.status);

    WDI_DetectedDeviceError( pWDICtx, WDI_ERR_BASIC_OP_FAILURE);

    wpalMutexRelease(&pWDICtx->wptMutex);
    return WDI_STATUS_E_FAILURE;
  }

  pWDICtx->ucExpectedStateTransition = WDI_STOPPED_ST;

  /*                                                                 
                                                                          
                        */
  WDI_STATE_TRANSITION( pWDICtx, pWDICtx->ucExpectedStateTransition);
  wpalMutexRelease(&pWDICtx->wptMutex);

  /*                                   */

  /*           */
  wdiStopRspCb( wdiStatus, pWDICtx->pRspCBUserData);

  return WDI_STATUS_SUCCESS;
}/*                  */

/* 
                                                          
                                                

                                                         
                                                                    

     
                                    
*/
WDI_Status
WDI_ProcessCloseRsp
(
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
)
{
  /*                                                                    
           */
  WDI_ASSERT(0);
  return WDI_STATUS_SUCCESS;
}/*                   */


/*                                                                            
                                                  
                                                                            */

/* 
                                                              
                                                

                                                         
                                                                    

     
                                    
*/
WDI_Status
WDI_ProcessInitScanRsp
(
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
)
{
  WDI_Status            wdiStatus;
  WDI_InitScanRspCb     wdiInitScanRspCb;
  tHalInitScanRspMsg    halInitScanRspMsg;
  /*                                                                        */

  /*                                                                         
                
                                                                           */
  if (( NULL == pWDICtx ) || ( NULL == pEventData ) ||
      ( NULL == pEventData->pEventData))
  {
     WPAL_TRACE( eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_WARN,
                 "%s: Invalid parameters", __FUNCTION__);
     WDI_ASSERT(0);
     return WDI_STATUS_E_FAILURE;
  }

  wdiInitScanRspCb = (WDI_InitScanRspCb)pWDICtx->pfncRspCB;
  if( NULL == wdiInitScanRspCb)
  {
     WPAL_TRACE( eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_FATAL,
                 "%s: call back function is NULL", __FUNCTION__);
     WDI_ASSERT(0);
     return WDI_STATUS_E_FAILURE;
  }

  /*                                                                         
                                                                         
                                    
                                                                           */
  wpalMemoryCopy( &halInitScanRspMsg.initScanRspParams,
                  pEventData->pEventData,
                  sizeof(halInitScanRspMsg.initScanRspParams));

  wdiStatus   =   WDI_HAL_2_WDI_STATUS(halInitScanRspMsg.initScanRspParams.status);

  if ( pWDICtx->bInBmps )
  {
     //                                           
     WDTS_SetPowerState(pWDICtx, WDTS_POWER_STATE_FULL, NULL);
  }

  /*           */
  wdiInitScanRspCb( wdiStatus, pWDICtx->pRspCBUserData);

  return WDI_STATUS_SUCCESS;
}/*                      */


/* 
                                                               
                                                

                                                         
                                                                    

     
                                    
*/
WDI_Status
WDI_ProcessStartScanRsp
(
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
)
{
  WDI_StartScanRspParamsType   wdiStartScanParams;
  WDI_StartScanRspCb           wdiStartScanRspCb;

  tHalStartScanRspMsg          halStartScanRspMsg;
  /*                                                                        */

  /*                                                                         
                
                                                                           */
  if (( NULL == pWDICtx ) || ( NULL == pEventData ) ||
      ( NULL == pEventData->pEventData))
  {
     WPAL_TRACE( eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_WARN,
                 "%s: Invalid parameters", __FUNCTION__);
     WDI_ASSERT(0);
     return WDI_STATUS_E_FAILURE;
  }

  wdiStartScanRspCb = (WDI_StartScanRspCb)pWDICtx->pfncRspCB;
  if( NULL == wdiStartScanRspCb)
  {
     WPAL_TRACE( eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_FATAL,
                 "%s: call back function is NULL", __FUNCTION__);
     WDI_ASSERT(0);
     return WDI_STATUS_E_FAILURE;
  }

  /*                                                                         
                                        
                                                                           */
  wpalMemoryCopy( &halStartScanRspMsg.startScanRspParams,
                  pEventData->pEventData,
                  sizeof(halStartScanRspMsg.startScanRspParams));

  wdiStartScanParams.wdiStatus   =   WDI_HAL_2_WDI_STATUS(
                             halStartScanRspMsg.startScanRspParams.status);
#ifdef WLAN_FEATURE_VOWIFI
  wdiStartScanParams.ucTxMgmtPower =
                             halStartScanRspMsg.startScanRspParams.txMgmtPower;
  wpalMemoryCopy( wdiStartScanParams.aStartTSF,
                  halStartScanRspMsg.startScanRspParams.startTSF,
                  2);
#endif

  if ( eHAL_STATUS_SUCCESS != halStartScanRspMsg.startScanRspParams.status )
  {
     WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_ERROR,
              "Start scan failed with status %s (%d)",
              WDI_getHALStatusMsgString(halStartScanRspMsg.startScanRspParams.status),
              halStartScanRspMsg.startScanRspParams.status);
     /*                                                */
  }

  /*           */
  wdiStartScanRspCb( &wdiStartScanParams, pWDICtx->pRspCBUserData);

  return WDI_STATUS_SUCCESS;

}/*                       */


/* 
                                                         
                                                         

                                                         
                                                                    

     
                                    
*/
WDI_Status
WDI_ProcessEndScanRsp
(
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
)
{
  WDI_Status            wdiStatus;
  tHalEndScanRspMsg     halEndScanRspMsg;
  WDI_EndScanRspCb      wdiEndScanRspCb;
  /*                                                                        */

  /*                                                                         
                
                                                                           */
  if (( NULL == pWDICtx ) || ( NULL == pEventData ) ||
      ( NULL == pEventData->pEventData))
  {
     WPAL_TRACE( eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_WARN,
                 "%s: Invalid parameters", __FUNCTION__);
     WDI_ASSERT(0);
     return WDI_STATUS_E_FAILURE;
  }

  wdiEndScanRspCb = (WDI_EndScanRspCb)pWDICtx->pfncRspCB;

  /*                                                                         
                                        
                                                                           */
  wpalMemoryCopy( &halEndScanRspMsg.endScanRspParams,
                  pEventData->pEventData,
                  sizeof(halEndScanRspMsg.endScanRspParams));

  wdiStatus   =   WDI_HAL_2_WDI_STATUS(halEndScanRspMsg.endScanRspParams.status);

  if ( eHAL_STATUS_SUCCESS != halEndScanRspMsg.endScanRspParams.status )
  {
     WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_ERROR,
              "End Scan failed with status %s (%d )",
              WDI_getHALStatusMsgString(halEndScanRspMsg.endScanRspParams.status),
              halEndScanRspMsg.endScanRspParams.status);
     /*                                                */
  }

  /*           */
  wdiEndScanRspCb( wdiStatus, pWDICtx->pRspCBUserData);

  return WDI_STATUS_SUCCESS;
}/*                     */


/* 
                                                            
                                                         

                                                         
                                                                    

     
                                    
*/
WDI_Status
WDI_ProcessFinishScanRsp
(
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
)
{
  WDI_Status            wdiStatus;
  WDI_FinishScanRspCb   wdiFinishScanRspCb;

  tHalFinishScanRspMsg  halFinishScanRspMsg;
  /*                                                                        */

  /*                                                                         
                
                                                                           */
  if (( NULL == pWDICtx ) || ( NULL == pEventData ) ||
      ( NULL == pEventData->pEventData))
  {
     WPAL_TRACE( eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_WARN,
                 "%s: Invalid parameters", __FUNCTION__);
     WDI_ASSERT(0);
     return WDI_STATUS_E_FAILURE;
  }

  wdiFinishScanRspCb = (WDI_FinishScanRspCb)pWDICtx->pfncRspCB;

  /*                                                                         
                                        
                                                                           */
  wpalMemoryCopy( (void *)&halFinishScanRspMsg.finishScanRspParams.status,
                  pEventData->pEventData,
                  sizeof(halFinishScanRspMsg.finishScanRspParams.status));

  wdiStatus   =   WDI_HAL_2_WDI_STATUS(halFinishScanRspMsg.finishScanRspParams.status);

  WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_INFO_LOW,
              "Finish scan response reported status: %d",
              halFinishScanRspMsg.finishScanRspParams.status);

  if (( eHAL_STATUS_SUCCESS != halFinishScanRspMsg.finishScanRspParams.status )&&
      ( eHAL_STATUS_NOTIFY_BSS_FAIL  != halFinishScanRspMsg.finishScanRspParams.status ))
  {
     WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_ERROR,
              "Finish Scan failed with status %s (%d)",
              WDI_getHALStatusMsgString(halFinishScanRspMsg.finishScanRspParams.status),
              halFinishScanRspMsg.finishScanRspParams.status);
     /*                                                */
  }

  /*           */
  wdiFinishScanRspCb( wdiStatus, pWDICtx->pRspCBUserData);

  return WDI_STATUS_SUCCESS;
}/*                        */

/* 
                                                              
                                                

                                                         
                                                                    

     
                                    
*/
WDI_Status
WDI_ProcessJoinRsp
(
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
)
{
  WDI_Status                    wdiStatus;
  WDI_JoinRspCb                 wdiJoinRspCb;
  WDI_BSSSessionType*           pBSSSes             = NULL;

  tHalJoinRspMsg                halJoinRspMsg;
  /*                                                                        */

  /*                                                                         
                
                                                                           */
  if (( NULL == pWDICtx ) ||
      ( NULL == pWDICtx->pfncRspCB ) ||
      ( NULL == pEventData ) ||
      ( NULL == pEventData->pEventData))
  {
     WPAL_TRACE( eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_WARN,
                 "%s: Invalid parameters", __FUNCTION__);
     WDI_ASSERT(0);
     return WDI_STATUS_E_FAILURE;
  }

  wdiJoinRspCb = (WDI_JoinRspCb)pWDICtx->pfncRspCB;

  /*                                                                         
                                        
                                                                           */
  wpalMemoryCopy( &halJoinRspMsg.joinRspParams,
                  pEventData->pEventData,
                  sizeof(halJoinRspMsg.joinRspParams));

  wdiStatus   =   WDI_HAL_2_WDI_STATUS(halJoinRspMsg.joinRspParams.status);

  wpalMutexAcquire(&pWDICtx->wptMutex);

  /*                                                                       
                                                                 
                              
                                                                           */
  if (( !WDI_VALID_SESSION_IDX(pWDICtx->ucCurrentBSSSesIdx )) ||
      ( eWLAN_PAL_FALSE == pWDICtx->bAssociationInProgress ))
  {
    WPAL_TRACE( eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_ERROR,
              "%s: Association sequence for this BSS does not yet exist (bssIdx %d) or "
              "association no longer in progress %d - mysterious HAL response",
              __func__, pWDICtx->ucCurrentBSSSesIdx, pWDICtx->bAssociationInProgress);

    WDI_DetectedDeviceError( pWDICtx, WDI_ERR_BASIC_OP_FAILURE);
    wpalMutexRelease(&pWDICtx->wptMutex);
    return WDI_STATUS_E_NOT_ALLOWED;
  }

  pBSSSes = &pWDICtx->aBSSSessions[pWDICtx->ucCurrentBSSSesIdx];

  /*                                                                       
                                               
                                                                         */
  if ( WDI_ASSOC_JOINING_ST != pBSSSes->wdiAssocState)
  {
    WPAL_TRACE( eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_ERROR,
              "Join only allowed in Joining state - failure state is %d "
              "strange HAL response", pBSSSes->wdiAssocState);

    WDI_DetectedDeviceError( pWDICtx, WDI_ERR_BASIC_OP_FAILURE);

    wpalMutexRelease(&pWDICtx->wptMutex);
    return WDI_STATUS_E_NOT_ALLOWED;
  }


  /*                                                                       
                                                           
                                                                         */
  if ( WDI_STATUS_SUCCESS != wdiStatus )
  {
    /*                                              */
    WDI_DeleteSession(pWDICtx, pBSSSes);

    /*                                   */
    pWDICtx->bAssociationInProgress = eWLAN_PAL_FALSE;

    /*                                                                        */
    WDI_DequeueAssocRequest(pWDICtx);

  }
  else
  {
    /*                                                                      
                                                       */
    pBSSSes->wdiAssocState = WDI_ASSOC_JOINING_ST;
  }

  wpalMutexRelease(&pWDICtx->wptMutex);

  /*           */
  wdiJoinRspCb( wdiStatus, pWDICtx->pRspCBUserData);

  return WDI_STATUS_SUCCESS;
}/*                  */


/* 
                                                           
                                                         

                                                         
                                                                    

     
                                    
*/
WDI_Status
WDI_ProcessConfigBSSRsp
(
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
)
{
  WDI_ConfigBSSRspParamsType    wdiConfigBSSParams;
  WDI_ConfigBSSRspCb            wdiConfigBSSRspCb;
  wpt_uint8                     ucCurrentBSSSesIdx  = 0;
  WDI_BSSSessionType*           pBSSSes             = NULL;

  tConfigBssRspMsg              halConfigBssRspMsg;
  WDI_AddStaParams              wdiBcastAddSTAParam = {0};
  WDI_AddStaParams              wdiAddSTAParam = {0};

  /*                                                                        */

  /*                                                                         
                
                                                                           */
  if (( NULL == pEventData ) ||
      ( NULL == pEventData->pEventData))
  {
     WPAL_TRACE( eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_WARN,
                 "%s: Invalid parameters", __FUNCTION__);
     WDI_ASSERT(0);
     return WDI_STATUS_E_FAILURE;
  }

  wdiConfigBSSRspCb = (WDI_ConfigBSSRspCb)pWDICtx->pfncRspCB;

  /*                                                                         
                                        
                                                                           */
  wpalMemoryCopy( &halConfigBssRspMsg.configBssRspParams,
                   pEventData->pEventData,
                   sizeof(halConfigBssRspMsg.configBssRspParams));

  wdiConfigBSSParams.wdiStatus = WDI_HAL_2_WDI_STATUS(
                            halConfigBssRspMsg.configBssRspParams.status);
  if(WDI_STATUS_SUCCESS == wdiConfigBSSParams.wdiStatus)
  {
    wpalMemoryCopy( wdiConfigBSSParams.macBSSID,
                    pWDICtx->wdiCachedConfigBssReq.wdiReqInfo.macBSSID,
                    WDI_MAC_ADDR_LEN);

    wdiConfigBSSParams.ucBSSIdx = halConfigBssRspMsg.configBssRspParams.bssIdx;

    wdiConfigBSSParams.ucBcastSig =
       halConfigBssRspMsg.configBssRspParams.bcastDpuSignature;

    wdiConfigBSSParams.ucUcastSig =
       halConfigBssRspMsg.configBssRspParams.ucastDpuSignature;

    wdiConfigBSSParams.ucSTAIdx = halConfigBssRspMsg.configBssRspParams.bssStaIdx;

  #ifdef WLAN_FEATURE_VOWIFI
    wdiConfigBSSParams.ucTxMgmtPower =
                               halConfigBssRspMsg.configBssRspParams.txMgmtPower;
  #endif
     wpalMemoryCopy( wdiConfigBSSParams.macSTA,
                     halConfigBssRspMsg.configBssRspParams.staMac,
                     WDI_MAC_ADDR_LEN );

    wpalMutexAcquire(&pWDICtx->wptMutex);
    /*                                                                        
                                                
                                                                            */
    ucCurrentBSSSesIdx = WDI_FindAssocSession( pWDICtx,
                                               wdiConfigBSSParams.macBSSID,
                                              &pBSSSes);

    /*                                                                       
                                                                         
                                
                                                                             */
    if ( NULL == pBSSSes )
    {
      WPAL_TRACE( eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_ERROR,
                "Association sequence for this BSS does not yet exist "
                "- mysterious HAL response");

      WDI_DetectedDeviceError( pWDICtx, WDI_ERR_BASIC_OP_FAILURE);

      wpalMutexRelease(&pWDICtx->wptMutex);
      return WDI_STATUS_E_NOT_ALLOWED;
    }

    /*                      */
    pBSSSes->wdiBssType = pWDICtx->wdiCachedConfigBssReq.wdiReqInfo.wdiBSSType;
    pBSSSes->ucBSSIdx = halConfigBssRspMsg.configBssRspParams.bssIdx;
    pBSSSes->bcastDpuIndex     =
      halConfigBssRspMsg.configBssRspParams.bcastDpuDescIndx;
    pBSSSes->bcastDpuSignature =
      halConfigBssRspMsg.configBssRspParams.bcastDpuSignature;
    pBSSSes->bcastMgmtDpuIndex =
      halConfigBssRspMsg.configBssRspParams.mgmtDpuDescIndx;
    pBSSSes->bcastMgmtDpuSignature =
      halConfigBssRspMsg.configBssRspParams.mgmtDpuSignature;
    pBSSSes->ucRmfEnabled      =
      pWDICtx->wdiCachedConfigBssReq.wdiReqInfo.ucRMFEnabled;
    pBSSSes->bcastStaIdx =
       halConfigBssRspMsg.configBssRspParams.bssBcastStaIdx;

    /*                                                                     */

    /*                                                                         
                    
                                                                               */
    wdiAddSTAParam.ucSTAIdx = halConfigBssRspMsg.configBssRspParams.bssStaIdx;
    wdiAddSTAParam.dpuIndex = halConfigBssRspMsg.configBssRspParams.dpuDescIndx;
    wdiAddSTAParam.dpuSig   = halConfigBssRspMsg.configBssRspParams.ucastDpuSignature;

     /*                                                          */
    wdiAddSTAParam.ucWmmEnabled =
        pWDICtx->wdiCachedConfigBssReq.wdiReqInfo.wdiSTAContext.ucWMMEnabled;
    wdiAddSTAParam.ucHTCapable  =
        pWDICtx->wdiCachedConfigBssReq.wdiReqInfo.wdiSTAContext.ucHTCapable;
    wdiAddSTAParam.ucStaType    =
        pWDICtx->wdiCachedConfigBssReq.wdiReqInfo.wdiSTAContext.wdiSTAType;

     /*                    */
    wpalMemoryCopy(wdiAddSTAParam.staMacAddr,
                   halConfigBssRspMsg.configBssRspParams.staMac,
                   WDI_MAC_ADDR_LEN);

    wpalMemoryCopy(wdiAddSTAParam.macBSSID,
                   pWDICtx->wdiCachedConfigBssReq.wdiReqInfo.wdiSTAContext.macBSSID ,
                   WDI_MAC_ADDR_LEN);

    /*                           */
    wdiAddSTAParam.bcastMgmtDpuIndex     =
        halConfigBssRspMsg.configBssRspParams.mgmtDpuDescIndx;
    wdiAddSTAParam.bcastMgmtDpuSignature =
        halConfigBssRspMsg.configBssRspParams.mgmtDpuSignature;
    wdiAddSTAParam.bcastDpuIndex         =
        halConfigBssRspMsg.configBssRspParams.bcastDpuDescIndx;
    wdiAddSTAParam.bcastDpuSignature     =
        halConfigBssRspMsg.configBssRspParams.bcastDpuSignature;
    wdiAddSTAParam.ucRmfEnabled          =
        pWDICtx->wdiCachedConfigBssReq.wdiReqInfo.ucRMFEnabled;
    wdiAddSTAParam.ucBSSIdx =
       halConfigBssRspMsg.configBssRspParams.bssIdx;

    WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_INFO,
                "Add STA to the table index: %d", wdiAddSTAParam.ucSTAIdx );

    WDI_STATableAddSta(pWDICtx,&wdiAddSTAParam);
    /*                                                                         
                                         
                                                                               */
    if( pWDICtx->wdiCachedConfigBssReq.wdiReqInfo.ucOperMode ==
        WDI_BSS_OPERATIONAL_MODE_AP )
    {
       WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_INFO,
                  "Add BCAST STA to table for index: %d",
                  halConfigBssRspMsg.configBssRspParams.bssBcastStaIdx );

       wpalMemoryCopy( &wdiBcastAddSTAParam, &wdiAddSTAParam,
                       sizeof(WDI_AddStaParams) );

       WDI_AddBcastSTAtoSTATable( pWDICtx, &wdiBcastAddSTAParam,
                                  halConfigBssRspMsg.configBssRspParams.bssBcastStaIdx );
    }
    wpalMutexRelease(&pWDICtx->wptMutex);
  }
  else
  {
    WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_ERROR,
                  "Config BSS RSP failed with status : %s(%d)",
                  WDI_getHALStatusMsgString(
                  halConfigBssRspMsg.configBssRspParams.status),
                  halConfigBssRspMsg.configBssRspParams.status);


    /*                                              */
    WDI_DeleteSession(pWDICtx, pBSSSes);

    /*                                   */
    pWDICtx->bAssociationInProgress = eWLAN_PAL_FALSE;

    /*                                                                        */
    WDI_DequeueAssocRequest(pWDICtx);

  }

  /*           */
  wdiConfigBSSRspCb( &wdiConfigBSSParams, pWDICtx->pRspCBUserData);

  return WDI_STATUS_SUCCESS;
}/*                       */


/* 
                                                                 
                                                

                                                         
                                                                    

     
                                    
*/
WDI_Status
WDI_ProcessDelBSSRsp
(
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
)
{
  WDI_DelBSSRspParamsType       wdiDelBSSParams;
  WDI_DelBSSRspCb               wdiDelBSSRspCb;
  wpt_uint8                     ucCurrentBSSSesIdx  = 0;
  WDI_BSSSessionType*           pBSSSes             = NULL;

  tDeleteBssRspMsg              halDelBssRspMsg;
  /*                                                                        */

  /*                                                                         
                
                                                                           */
  if (( NULL == pEventData ) ||
      ( NULL == pEventData->pEventData))
  {
     WPAL_TRACE( eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_WARN,
                 "%s: Invalid parameters", __FUNCTION__);
     WDI_ASSERT(0);
     return WDI_STATUS_E_FAILURE;
  }

  wdiDelBSSRspCb = (WDI_DelBSSRspCb)pWDICtx->pfncRspCB;

  /*                                                                         
                                        
                                                                           */
  wpalMemoryCopy( &halDelBssRspMsg.deleteBssRspParams,
                  pEventData->pEventData,
                  sizeof(halDelBssRspMsg.deleteBssRspParams));


  wdiDelBSSParams.wdiStatus   =   WDI_HAL_2_WDI_STATUS(
                                 halDelBssRspMsg.deleteBssRspParams.status);

  wpalMutexAcquire(&pWDICtx->wptMutex);

  /*                                                                        
                                              
                                                                          */
  ucCurrentBSSSesIdx = WDI_FindAssocSessionByBSSIdx( pWDICtx,
                             halDelBssRspMsg.deleteBssRspParams.bssIdx,
                             &pBSSSes);

  /*                                                                       
                                                                    
                              
                                                                           */
  if ( NULL == pBSSSes )
  {
    WPAL_TRACE( eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_ERROR,
              "Association sequence for this BSS does not yet exist or "
              "association no longer in progress - mysterious HAL response");

    WDI_DetectedDeviceError( pWDICtx, WDI_ERR_BASIC_OP_FAILURE);

    wpalMutexRelease(&pWDICtx->wptMutex);
    return WDI_STATUS_E_NOT_ALLOWED;
  }

  /*                                      */
  wpalMemoryCopy(wdiDelBSSParams.macBSSID,
                 pBSSSes->macBSSID, WDI_MAC_ADDR_LEN);

  wdiDelBSSParams.ucBssIdx = halDelBssRspMsg.deleteBssRspParams.bssIdx;

  /*                                                                       
                                       
                                                                         */
  WDI_DeleteSession(pWDICtx, pBSSSes);


  /*                                                                            */
  if(WDI_INFRA_AP_MODE == pBSSSes->wdiBssType)
  {
    (void)WDI_STATableDelSta( pWDICtx, pBSSSes->bcastStaIdx );
  }

   /*                              */
  WDI_STATableBSSDelSta(pWDICtx, halDelBssRspMsg.deleteBssRspParams.bssIdx);

  wpalMutexRelease(&pWDICtx->wptMutex);

  /*           */
  wdiDelBSSRspCb( &wdiDelBSSParams, pWDICtx->pRspCBUserData);

  return WDI_STATUS_SUCCESS;
}/*                    */

/* 
                                                               
                                                

                                                         
                                                                    

     
                                    
*/
WDI_Status
WDI_ProcessPostAssocRsp
(
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
)
{
  WDI_PostAssocRspParamsType    wdiPostAssocParams;
  WDI_PostAssocRspCb            wdiPostAssocRspCb;
  wpt_uint8                     ucCurrentBSSSesIdx     = 0;
  WDI_BSSSessionType*           pBSSSes                = NULL;
  tPostAssocRspMsg              halPostAssocRspMsg;
  /*                                                                        */

  /*                                                                         
                
                                                                           */
  if (( NULL == pEventData ) ||
      ( NULL == pEventData->pEventData))
  {
     WPAL_TRACE( eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_WARN,
                 "%s: Invalid parameters", __FUNCTION__);
     WDI_ASSERT(0);
     return WDI_STATUS_E_FAILURE;
  }

  wdiPostAssocRspCb = (WDI_PostAssocRspCb)pWDICtx->pfncRspCB;

  /*                                                                         
                                        
                                                                           */
  wpalMemoryCopy( &halPostAssocRspMsg.postAssocRspParams,
                   pEventData->pEventData,
                   sizeof(halPostAssocRspMsg.postAssocRspParams));

  /*                                  */

  wdiPostAssocParams.staParams.ucSTAIdx   =
    halPostAssocRspMsg.postAssocRspParams.configStaRspParams.staIdx;
  wdiPostAssocParams.staParams.ucUcastSig =
    halPostAssocRspMsg.postAssocRspParams.configStaRspParams.ucUcastSig;
  wdiPostAssocParams.staParams.ucBcastSig =
    halPostAssocRspMsg.postAssocRspParams.configStaRspParams.ucBcastSig;

 wdiPostAssocParams.wdiStatus =
    WDI_HAL_2_WDI_STATUS(halPostAssocRspMsg.postAssocRspParams.configStaRspParams.status);

 /*                                                                            
                            */
  wpalMemoryCopy( wdiPostAssocParams.staParams.macSTA,
                  pWDICtx->wdiCachedPostAssocReq.wdiSTAParams.macSTA,
                  WDI_MAC_ADDR_LEN);

  /*                               */

  wpalMemoryCopy( wdiPostAssocParams.bssParams.macBSSID,
                  pWDICtx->wdiCachedPostAssocReq.wdiBSSParams.macBSSID,
                  WDI_MAC_ADDR_LEN);

  /*                                                                            
                            */
  wpalMemoryCopy( wdiPostAssocParams.bssParams.macSTA,
                  pWDICtx->wdiCachedPostAssocReq.wdiBSSParams.wdiSTAContext
                  .macSTA, WDI_MAC_ADDR_LEN);

  wdiPostAssocParams.bssParams.ucBcastSig =
     halPostAssocRspMsg.postAssocRspParams.configStaRspParams.ucBcastSig;

  wdiPostAssocParams.bssParams.ucUcastSig =
     halPostAssocRspMsg.postAssocRspParams.configStaRspParams.ucUcastSig;

  wdiPostAssocParams.bssParams.ucBSSIdx =
     halPostAssocRspMsg.postAssocRspParams.configBssRspParams.bssIdx;

  wdiPostAssocParams.bssParams.ucSTAIdx =
     halPostAssocRspMsg.postAssocRspParams.configBssRspParams.bssStaIdx;

  wpalMutexAcquire(&pWDICtx->wptMutex);

  /*                                                                        
                                              
                                                                          */
  ucCurrentBSSSesIdx = WDI_FindAssocSession( pWDICtx,
                                             wdiPostAssocParams.bssParams.
                                             macBSSID, &pBSSSes);

  /*                                                                       
                                                                       
                              
                                                                           */
  if (( NULL == pBSSSes ) ||
      ( ucCurrentBSSSesIdx != pWDICtx->ucCurrentBSSSesIdx ) ||
      ( eWLAN_PAL_FALSE == pWDICtx->bAssociationInProgress ))
  {
    WPAL_TRACE( eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_ERROR,
              "Association sequence for this BSS does not yet exist or "
              "association no longer in progress - mysterious HAL response");

    WDI_DetectedDeviceError( pWDICtx, WDI_ERR_BASIC_OP_FAILURE);

    wpalMutexRelease(&pWDICtx->wptMutex);
    return WDI_STATUS_E_NOT_ALLOWED;
  }

  /*                                                                       
                                                       
                                                                         */
  if ( WDI_ASSOC_JOINING_ST != pBSSSes->wdiAssocState)
  {
    WPAL_TRACE( eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_ERROR,
              "Post Assoc not allowed before JOIN - failing request "
              "strange HAL response");

    WDI_DetectedDeviceError( pWDICtx, WDI_ERR_BASIC_OP_FAILURE);

    wpalMutexRelease(&pWDICtx->wptMutex);
    return WDI_STATUS_E_NOT_ALLOWED;
  }

  /*                                                                       
                                                           
                                                                         */
  if ( WDI_STATUS_SUCCESS != wdiPostAssocParams.wdiStatus )
  {
    /*                                              */
    WDI_DeleteSession(pWDICtx, pBSSSes);
  }
  else
  {
    /*                              */
    pBSSSes->wdiAssocState = WDI_ASSOC_POST_ST;

    /*             */
    pBSSSes->bcastMgmtDpuIndex     =
      halPostAssocRspMsg.postAssocRspParams.configBssRspParams.mgmtDpuDescIndx;
    pBSSSes->bcastMgmtDpuSignature =
      halPostAssocRspMsg.postAssocRspParams.configBssRspParams.mgmtDpuSignature;
    pBSSSes->bcastDpuIndex         =
      halPostAssocRspMsg.postAssocRspParams.configBssRspParams.bcastDpuDescIndx;
    pBSSSes->bcastDpuSignature     =
      halPostAssocRspMsg.postAssocRspParams.configBssRspParams.bcastDpuSignature;

    pBSSSes->ucBSSIdx              =
      halPostAssocRspMsg.postAssocRspParams.configBssRspParams.bssIdx;
  }

  /*                                   */
  pWDICtx->bAssociationInProgress = eWLAN_PAL_FALSE;

  /*                                                                        */
  WDI_DequeueAssocRequest(pWDICtx);

  wpalMutexRelease(&pWDICtx->wptMutex);

  /*           */
  wdiPostAssocRspCb( &wdiPostAssocParams, pWDICtx->pRspCBUserData);

  return WDI_STATUS_SUCCESS;
}/*                       */

/* 
                                                               
                                             

                                                         
                                                                    

     
                                    
*/
WDI_Status
WDI_ProcessDelSTARsp
(
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
)
{
  WDI_DelSTARspParamsType   wdiDelSTARsp;
  WDI_DelSTARspCb           wdiDelSTARspCb;
  wpt_uint8                 staType;
  tDeleteStaRspMsg          halDelStaRspMsg;
  /*                                                                        */

  /*                                                                         
                
                                                                           */
  if (( NULL == pWDICtx ) || ( NULL == pEventData ) ||
      ( NULL == pEventData->pEventData))
  {
     WPAL_TRACE( eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_WARN,
                 "%s: Invalid parameters", __FUNCTION__);
     WDI_ASSERT(0);
     return WDI_STATUS_E_FAILURE;
  }

  wdiDelSTARspCb = (WDI_DelSTARspCb)pWDICtx->pfncRspCB;

  /*                                                                         
                                        
                                                                           */
  wpalMemoryCopy( &halDelStaRspMsg.delStaRspParams,
                  pEventData->pEventData,
                  sizeof(halDelStaRspMsg.delStaRspParams));

  wdiDelSTARsp.ucSTAIdx    = halDelStaRspMsg.delStaRspParams.staId;
  wdiDelSTARsp.wdiStatus   =
    WDI_HAL_2_WDI_STATUS(halDelStaRspMsg.delStaRspParams.status);

  WDI_STATableGetStaType(pWDICtx, wdiDelSTARsp.ucSTAIdx, &staType);

  /*                                                                                                     */
  if(staType == WDI_STA_ENTRY_SELF)
  {
    WDI_StaStruct* pSTATable = (WDI_StaStruct*) pWDICtx->staTable;

    /*                                */

    /*                                     */
    /*                                    */
    /*                                     */

#define WDI_DPU_SELF_STA_DEFAULT_IDX 0
#define WDI_DPU_SELF_STA_DEFAULT_SIG 0

    //                                         
    pSTATable[wdiDelSTARsp.ucSTAIdx].dpuIndex = WDI_DPU_SELF_STA_DEFAULT_IDX;
    pSTATable[wdiDelSTARsp.ucSTAIdx].bcastDpuIndex = WDI_DPU_SELF_STA_DEFAULT_IDX;
    pSTATable[wdiDelSTARsp.ucSTAIdx].bcastMgmtDpuIndex = WDI_DPU_SELF_STA_DEFAULT_IDX;
    pSTATable[wdiDelSTARsp.ucSTAIdx].bcastDpuSignature = WDI_DPU_SELF_STA_DEFAULT_SIG;
    pSTATable[wdiDelSTARsp.ucSTAIdx].bcastMgmtDpuSignature = WDI_DPU_SELF_STA_DEFAULT_SIG;
    pSTATable[wdiDelSTARsp.ucSTAIdx].dpuSig = WDI_DPU_SELF_STA_DEFAULT_SIG;

    pSTATable[wdiDelSTARsp.ucSTAIdx].bssIdx = WDI_BSS_INVALID_IDX;
  }
  else
  {
    //                               
    WDI_STATableDelSta( pWDICtx, wdiDelSTARsp.ucSTAIdx);
  }

  /*           */
  wdiDelSTARspCb( &wdiDelSTARsp, pWDICtx->pRspCBUserData);

  return WDI_STATUS_SUCCESS;
}/*                    */


/*                                                                          
                                                         
                                                                          */

/* 
                                                                
                                                

                                                         
                                                                    

     
                                    
*/
WDI_Status
WDI_ProcessSetBssKeyRsp
(
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
)
{
  WDI_Status            wdiStatus;
  eHalStatus            halStatus;
  WDI_SetBSSKeyRspCb    wdiSetBSSKeyRspCb;
  /*                                                                        */

  /*                                                                         
                
                                                                           */
  if (( NULL == pWDICtx ) || ( NULL == pEventData ) ||
      ( NULL == pEventData->pEventData))
  {
     WPAL_TRACE( eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_WARN,
                 "%s: Invalid parameters", __FUNCTION__);
     WDI_ASSERT(0);
     return WDI_STATUS_E_FAILURE;
  }

  wdiSetBSSKeyRspCb = (WDI_SetBSSKeyRspCb)pWDICtx->pfncRspCB;

  /*                                                                         
                                        
                                                                           */
  halStatus = *((eHalStatus*)pEventData->pEventData);
  wdiStatus   =   WDI_HAL_2_WDI_STATUS(halStatus);

  if ( eHAL_STATUS_SUCCESS != halStatus )
  {
     WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_ERROR,
              "Set BSS Key failed with status %s (%d)",
              WDI_getHALStatusMsgString(halStatus),
              halStatus);
     /*                                                */
  }

  /*           */
  wdiSetBSSKeyRspCb( wdiStatus, pWDICtx->pRspCBUserData);

  return WDI_STATUS_SUCCESS;
}/*                       */

/* 
                                                                   
                                                

                                                         
                                                                    

     
                                    
*/
WDI_Status
WDI_ProcessRemoveBssKeyRsp
(
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
)
{
  WDI_Status              wdiStatus;
  eHalStatus              halStatus;
  WDI_RemoveBSSKeyRspCb   wdiRemoveBSSKeyRspCb;
  /*                                                                        */

  /*                                                                         
                
                                                                           */
  if (( NULL == pWDICtx ) || ( NULL == pEventData ) ||
      ( NULL == pEventData->pEventData))
  {
     WPAL_TRACE( eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_WARN,
                 "%s: Invalid parameters", __FUNCTION__);
     WDI_ASSERT(0);
     return WDI_STATUS_E_FAILURE;
  }

  wdiRemoveBSSKeyRspCb = (WDI_RemoveBSSKeyRspCb)pWDICtx->pfncRspCB;

  /*                                                                         
                                        
                                                                           */
  halStatus = *((eHalStatus*)pEventData->pEventData);
  wdiStatus   =   WDI_HAL_2_WDI_STATUS(halStatus);

  if ( eHAL_STATUS_SUCCESS != halStatus )
  {
     WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_ERROR,
              "Remove BSS Key failed with status %s (%d )",
              WDI_getHALStatusMsgString(halStatus),
              halStatus);
     /*                                                */
  }

  /*           */
  wdiRemoveBSSKeyRspCb( wdiStatus, pWDICtx->pRspCBUserData);

  return WDI_STATUS_SUCCESS;
}/*                       */


/* 
                                                                
                                                

                                                         
                                                                    

     
                                    
*/
WDI_Status
WDI_ProcessSetStaKeyRsp
(
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
)
{
  WDI_Status            wdiStatus;
  eHalStatus            halStatus;
  WDI_SetSTAKeyRspCb    wdiSetSTAKeyRspCb;
  /*                                                                        */

  /*                                                                         
                
                                                                           */
  if (( NULL == pWDICtx ) || ( NULL == pEventData ) ||
      ( NULL == pEventData->pEventData))
  {
     WPAL_TRACE( eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_WARN,
                 "%s: Invalid parameters", __FUNCTION__);
     WDI_ASSERT(0);
     return WDI_STATUS_E_FAILURE;
  }

  wdiSetSTAKeyRspCb = (WDI_SetSTAKeyRspCb)pWDICtx->pfncRspCB;

  /*                                                                         
                                        
                                                                           */
  halStatus = *((eHalStatus*)pEventData->pEventData);
  wdiStatus   =   WDI_HAL_2_WDI_STATUS(halStatus);

  if ( eHAL_STATUS_SUCCESS != halStatus )
  {
     WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_ERROR,
              "Set STA Key failed with status %s (%d)",
              WDI_getHALStatusMsgString(halStatus),
              halStatus);
     /*                                                */
  }

  /*           */
  wdiSetSTAKeyRspCb( wdiStatus, pWDICtx->pRspCBUserData);

  return WDI_STATUS_SUCCESS;
}/*                       */

/* 
                                                          
                                                         

                                                         
                                                                    

     
                                    
*/
WDI_Status
WDI_ProcessRemoveStaKeyRsp
(
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
)
{
  WDI_Status              wdiStatus;
  eHalStatus              halStatus;
  WDI_RemoveSTAKeyRspCb   wdiRemoveSTAKeyRspCb;
  /*                                                                        */

  /*                                                                         
                
                                                                           */
  if (( NULL == pWDICtx ) || ( NULL == pEventData ) ||
      ( NULL == pEventData->pEventData))
  {
     WPAL_TRACE( eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_WARN,
                 "%s: Invalid parameters", __FUNCTION__);
     WDI_ASSERT(0);
     return WDI_STATUS_E_FAILURE;
  }

  wdiRemoveSTAKeyRspCb = (WDI_RemoveSTAKeyRspCb)pWDICtx->pfncRspCB;

  /*                                                                         
                                        
                                                                           */
  halStatus = *((eHalStatus*)pEventData->pEventData);
  wdiStatus   =   WDI_HAL_2_WDI_STATUS(halStatus);

  if ( eHAL_STATUS_SUCCESS != halStatus )
  {
     WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_ERROR,
              "Remove STA Key failed with status %s (%d)",
              WDI_getHALStatusMsgString(halStatus),
              halStatus);
     /*                                                */
  }

  /*           */
  wdiRemoveSTAKeyRspCb( wdiStatus, pWDICtx->pRspCBUserData);

  return WDI_STATUS_SUCCESS;
}/*                          */

/* 
                                                             
                                                         

                                                         
                                                                    

     
                                    
*/
WDI_Status
WDI_ProcessSetStaBcastKeyRsp
(
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
)
{
  WDI_Status            wdiStatus;
  eHalStatus            halStatus;
  WDI_SetSTAKeyRspCb    wdiSetSTABcastKeyRspCb;
  /*                                                                        */

  /*                                                                         
                
                                                                           */
  if (( NULL == pWDICtx ) || ( NULL == pEventData ) ||
      ( NULL == pEventData->pEventData))
  {
     WPAL_TRACE( eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_WARN,
                 "%s: Invalid parameters", __FUNCTION__);
     WDI_ASSERT(0);
     return WDI_STATUS_E_FAILURE;
  }

  wdiSetSTABcastKeyRspCb = (WDI_SetSTAKeyRspCb)pWDICtx->pfncRspCB;

  /*                                                                         
                                        
                                                                           */
  wpalMemoryCopy( &halStatus,
                  pEventData->pEventData,
                  sizeof(halStatus));

  wdiStatus   =   WDI_HAL_2_WDI_STATUS(halStatus);

  if ( eHAL_STATUS_SUCCESS != halStatus )
  {
     WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_ERROR,
              "Set STA Key failed with status %s (%d)",
              WDI_getHALStatusMsgString(halStatus),
              halStatus);
     /*                                                */
  }

  /*           */
  wdiSetSTABcastKeyRspCb( wdiStatus, pWDICtx->pRspCBUserData);

  return WDI_STATUS_SUCCESS;
}/*                            */

/* 
                                                                
                                                         

                                                         
                                                                    

     
                                    
*/
WDI_Status
WDI_ProcessRemoveStaBcastKeyRsp
(
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
)
{
  WDI_Status              wdiStatus;
  eHalStatus              halStatus;
  WDI_RemoveSTAKeyRspCb   wdiRemoveSTABcastKeyRspCb;
  /*                                                                        */

  /*                                                                         
                
                                                                           */
  if (( NULL == pWDICtx ) || ( NULL == pEventData ) ||
      ( NULL == pEventData->pEventData))
  {
     WPAL_TRACE( eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_WARN,
                 "%s: Invalid parameters", __FUNCTION__);
     WDI_ASSERT(0);
     return WDI_STATUS_E_FAILURE;
  }

  wdiRemoveSTABcastKeyRspCb = (WDI_RemoveSTAKeyRspCb)pWDICtx->pfncRspCB;

  /*                                                                         
                                        
                                                                           */
  wpalMemoryCopy( &halStatus,
                  pEventData->pEventData,
                  sizeof(halStatus));

  wdiStatus   =   WDI_HAL_2_WDI_STATUS(halStatus);

  if ( eHAL_STATUS_SUCCESS != halStatus )
  {
     WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_ERROR,
              "Remove STA Key failed with status %s (%d)",
              WDI_getHALStatusMsgString(halStatus),
              halStatus);
     /*                                                */
  }

  /*           */
  wdiRemoveSTABcastKeyRspCb( wdiStatus, pWDICtx->pRspCBUserData);

  return WDI_STATUS_SUCCESS;
}/*                               */


/*                                                                          
                                                           
                                                                          */

/* 
                                                              
                                                

                                                         
                                                                    

     
                                    
*/
WDI_Status
WDI_ProcessAddTSpecRsp
(
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
)
{
  WDI_Status       wdiStatus;
  eHalStatus       halStatus;
  WDI_AddTsRspCb   wdiAddTsRspCb;
  /*                                                                        */

  /*                                                                         
                
                                                                           */
  if (( NULL == pWDICtx ) || ( NULL == pEventData ) ||
      ( NULL == pEventData->pEventData))
  {
     WPAL_TRACE( eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_WARN,
                 "%s: Invalid parameters", __FUNCTION__);
     WDI_ASSERT(0);
     return WDI_STATUS_E_FAILURE;
  }

  wdiAddTsRspCb = (WDI_AddTsRspCb)pWDICtx->pfncRspCB;

  /*                                                                         
                                        
                                                                           */
  halStatus = *((eHalStatus*)pEventData->pEventData);
  wdiStatus   =   WDI_HAL_2_WDI_STATUS(halStatus);

  /*           */
  wdiAddTsRspCb( wdiStatus, pWDICtx->pRspCBUserData);

  return WDI_STATUS_SUCCESS;
}/*                      */


/* 
                                                              
                                                

                                                         
                                                                    

     
                                    
*/
WDI_Status
WDI_ProcessDelTSpecRsp
(
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
)
{
  WDI_Status       wdiStatus;
  eHalStatus       halStatus;
  WDI_DelTsRspCb   wdiDelTsRspCb;
  /*                                                                        */

  /*                                                                         
                
                                                                           */
  if (( NULL == pWDICtx ) || ( NULL == pEventData ) ||
      ( NULL == pEventData->pEventData))
  {
     WPAL_TRACE( eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_WARN,
                 "%s: Invalid parameters", __FUNCTION__);
     WDI_ASSERT(0);
     return WDI_STATUS_E_FAILURE;
  }

  wdiDelTsRspCb = (WDI_DelTsRspCb)pWDICtx->pfncRspCB;

  /*                                                                         
                                        
                                                                           */
  halStatus = *((eHalStatus*)pEventData->pEventData);
  wdiStatus   =   WDI_HAL_2_WDI_STATUS(halStatus);

  /*           */
  wdiDelTsRspCb( wdiStatus, pWDICtx->pRspCBUserData);

  return WDI_STATUS_SUCCESS;
}/*                      */

/* 
                                                                  
                                                         

                                                         
                                                                    

     
                                    
*/
WDI_Status
WDI_ProcessUpdateEDCAParamsRsp
(
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
)
{
  WDI_Status       wdiStatus;
  eHalStatus       halStatus;
  WDI_UpdateEDCAParamsRspCb   wdiUpdateEDCAParamsRspCb;
  /*                                                                        */

  /*                                                                         
                
                                                                           */
  if (( NULL == pWDICtx ) || ( NULL == pEventData ) ||
      ( NULL == pEventData->pEventData))
  {
     WPAL_TRACE( eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_WARN,
                 "%s: Invalid parameters", __FUNCTION__);
     WDI_ASSERT(0);
     return WDI_STATUS_E_FAILURE;
  }

  wdiUpdateEDCAParamsRspCb = (WDI_UpdateEDCAParamsRspCb)pWDICtx->pfncRspCB;

  /*                                                                         
                                        
                                                                           */
  halStatus = *((eHalStatus*)pEventData->pEventData);
  wdiStatus   =   WDI_HAL_2_WDI_STATUS(halStatus);

  /*           */
  wdiUpdateEDCAParamsRspCb( wdiStatus, pWDICtx->pRspCBUserData);

  return WDI_STATUS_SUCCESS;
}/*                              */


/* 
                                                           
                                                

                                                         
                                                                    

     
                                    
*/
WDI_Status
WDI_ProcessAddBASessionRsp
(
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
)
{
  WDI_AddBASessionRspCb   wdiAddBASessionRspCb;

  tAddBASessionRspParams        halBASessionRsp;
  WDI_AddBASessionRspParamsType wdiBASessionRsp;


  /*                                                                        */

  /*                                                                         
                
                                                                           */
  if (( NULL == pWDICtx ) || ( NULL == pEventData ) ||
      ( NULL == pEventData->pEventData))
  {
     WPAL_TRACE( eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_WARN,
                 "%s: Invalid parameters", __FUNCTION__);
     WDI_ASSERT(0);
     return WDI_STATUS_E_FAILURE;
  }

  wdiAddBASessionRspCb = (WDI_AddBASessionRspCb)pWDICtx->pfncRspCB;

  /*                                                                         
                                        
                                                                           */
  wpalMemoryCopy( &halBASessionRsp,
                  pEventData->pEventData,
                  sizeof(halBASessionRsp));

  wdiBASessionRsp.wdiStatus = WDI_HAL_2_WDI_STATUS(halBASessionRsp.status);

  if ( WDI_STATUS_SUCCESS == wdiBASessionRsp.wdiStatus )
  {
    wdiBASessionRsp.ucBaDialogToken = halBASessionRsp.baDialogToken;
    wdiBASessionRsp.ucBaTID = halBASessionRsp.baTID;
    wdiBASessionRsp.ucBaBufferSize = halBASessionRsp.baBufferSize;
    wdiBASessionRsp.usBaSessionID = halBASessionRsp.baSessionID;
    wdiBASessionRsp.ucWinSize = halBASessionRsp.winSize;
    wdiBASessionRsp.ucSTAIdx = halBASessionRsp.STAID;
    wdiBASessionRsp.usBaSSN = halBASessionRsp.SSN;
  }

  /*           */
  wdiAddBASessionRspCb( &wdiBASessionRsp, pWDICtx->pRspCBUserData);

  return WDI_STATUS_SUCCESS;
}/*                          */


/* 
                                                           
                                                

                                                         
                                                                    

     
                                    
*/
WDI_Status
WDI_ProcessDelBARsp
(
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
)
{
  WDI_Status       wdiStatus;
  eHalStatus       halStatus;
  WDI_DelBARspCb   wdiDelBARspCb;
  /*                                                                        */

  /*                                                                         
                
                                                                           */
  if (( NULL == pWDICtx ) || ( NULL == pEventData ) ||
      ( NULL == pEventData->pEventData))
  {
     WPAL_TRACE( eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_WARN,
                 "%s: Invalid parameters", __FUNCTION__);
     WDI_ASSERT(0);
     return WDI_STATUS_E_FAILURE;
  }

  wdiDelBARspCb = (WDI_DelBARspCb)pWDICtx->pfncRspCB;

  /*                                                                         
                                        
                                                                           */
  halStatus = *((eHalStatus*)pEventData->pEventData);
  wdiStatus   =   WDI_HAL_2_WDI_STATUS(halStatus);

  if ( eHAL_STATUS_SUCCESS == halStatus )
  {
    /*                                                                  */
  }

  /*           */
  wdiDelBARspCb( wdiStatus, pWDICtx->pRspCBUserData);

  return WDI_STATUS_SUCCESS;
}/*                   */

#ifdef FEATURE_WLAN_CCX
/* 
                                                              
                                                

                                                         
                                                                    

     
                                    
*/
WDI_Status
WDI_ProcessTsmStatsRsp
(
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
)
{
  WDI_TsmRspCb          wdiTsmStatsRspCb;
  tTsmStatsRspMsg       halTsmStatsRspMsg;
  WDI_TSMStatsRspParamsType  wdiTsmStatsRspParams;
  /*                                                                        */

  /*                                                                         
                
                                                                           */
  if (( NULL == pWDICtx ) || ( NULL == pEventData ) ||
      ( NULL == pEventData->pEventData))
  {
     WPAL_TRACE( eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_ERROR,
                 "%s: Invalid parameters", __FUNCTION__);
     WDI_ASSERT(0);
     return WDI_STATUS_E_FAILURE;
  }

  wdiTsmStatsRspCb = (WDI_TsmRspCb)pWDICtx->pfncRspCB;

  /*                                                                         
                                                                         
                                    
                                                                           */
  wpalMemoryCopy( &halTsmStatsRspMsg.tsmStatsRspParams,
                  pEventData->pEventData,
                  sizeof(halTsmStatsRspMsg.tsmStatsRspParams));

  wdiTsmStatsRspParams.UplinkPktQueueDly = halTsmStatsRspMsg.tsmStatsRspParams.UplinkPktQueueDly;
  wpalMemoryCopy( wdiTsmStatsRspParams.UplinkPktQueueDlyHist,
                  halTsmStatsRspMsg.tsmStatsRspParams.UplinkPktQueueDlyHist,
                  sizeof(halTsmStatsRspMsg.tsmStatsRspParams.UplinkPktQueueDlyHist)/
                  sizeof(halTsmStatsRspMsg.tsmStatsRspParams.UplinkPktQueueDlyHist[0]));
  wdiTsmStatsRspParams.UplinkPktTxDly = halTsmStatsRspMsg.tsmStatsRspParams.UplinkPktTxDly;
  wdiTsmStatsRspParams.UplinkPktLoss = halTsmStatsRspMsg.tsmStatsRspParams.UplinkPktLoss;
  wdiTsmStatsRspParams.UplinkPktCount = halTsmStatsRspMsg.tsmStatsRspParams.UplinkPktCount;
  wdiTsmStatsRspParams.RoamingCount = halTsmStatsRspMsg.tsmStatsRspParams.RoamingCount;
  wdiTsmStatsRspParams.RoamingDly = halTsmStatsRspMsg.tsmStatsRspParams.RoamingDly;
  wdiTsmStatsRspParams.wdiStatus   =   WDI_HAL_2_WDI_STATUS(
                             halTsmStatsRspMsg.tsmStatsRspParams.status);

  /*           */
  wdiTsmStatsRspCb( &wdiTsmStatsRspParams, pWDICtx->pRspCBUserData);

  return WDI_STATUS_SUCCESS;
}/*                      */

#endif



/* 
                                                             
                                                

                                                         
                                                                    

     
                                    
*/
WDI_Status
WDI_ProcessFlushAcRsp
(
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
)
{
  WDI_Status       wdiStatus;
  eHalStatus       halStatus;
  WDI_FlushAcRspCb wdiFlushAcRspCb;
  /*                                                                        */

  /*                                                                         
                
                                                                           */
  if (( NULL == pWDICtx ) || ( NULL == pEventData ) ||
      ( NULL == pEventData->pEventData))
  {
     WPAL_TRACE( eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_WARN,
                 "%s: Invalid parameters", __FUNCTION__);
     WDI_ASSERT(0);
     return WDI_STATUS_E_FAILURE;
  }

  wdiFlushAcRspCb = (WDI_FlushAcRspCb)pWDICtx->pfncRspCB;

  /*                                                                         
                                        
                                                                           */
  wpalMemoryCopy( &halStatus,
                  pEventData->pEventData,
                  sizeof(halStatus));

  wdiStatus   =   WDI_HAL_2_WDI_STATUS(halStatus);

  /*           */
  wdiFlushAcRspCb( wdiStatus, pWDICtx->pRspCBUserData);

  return WDI_STATUS_SUCCESS;
}/*                     */

/* 
                                                        
                                                         

                                                         
                                                                    

     
                                    
*/
WDI_Status
WDI_ProcessBtAmpEventRsp
(
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
)
{
   WDI_Status       wdiStatus;
   eHalStatus       halStatus;
   WDI_BtAmpEventRspCb wdiBtAmpEventRspCb;
   /*                                                                        */

   /*                                                                         
                 
                                                                            */
   if (( NULL == pWDICtx ) || ( NULL == pEventData ) ||
       ( NULL == pEventData->pEventData))
   {
      WPAL_TRACE( eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_WARN,
                  "%s: Invalid parameters", __FUNCTION__);
      WDI_ASSERT(0);
      return WDI_STATUS_E_FAILURE;
   }

   wdiBtAmpEventRspCb = (WDI_BtAmpEventRspCb)pWDICtx->pfncRspCB;

   /*                                                                         
                                         
                                                                            */
   wpalMemoryCopy( &halStatus,
                   pEventData->pEventData,
                   sizeof(halStatus));

   wdiStatus   =   WDI_HAL_2_WDI_STATUS(halStatus);

   /*           */
   wdiBtAmpEventRspCb( wdiStatus, pWDICtx->pRspCBUserData);

   return WDI_STATUS_SUCCESS;
}/*                        */


/* 
                                                 
                                                                

                                                         
                                                                    

     
                                    
*/
WDI_Status
WDI_ProcessAddSTASelfRsp
(
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
)
{
  WDI_AddSTASelfRspParamsType  wdiAddSTASelfParams;
  WDI_AddSTASelfParamsRspCb    wdiAddSTASelfReqParamsRspCb;
  tAddStaSelfRspMsg            halAddStaSelfRsp;
  WDI_AddStaParams             wdiAddSTAParam = {0};
  /*                                                                        */

  /*                                                                         
                
                                                                           */
  if (( NULL == pWDICtx ) || ( NULL == pEventData ) ||
      ( NULL == pEventData->pEventData))
  {
     WPAL_TRACE( eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_WARN,
                 "%s: Invalid parameters", __FUNCTION__);
     WDI_ASSERT(0);
     return WDI_STATUS_E_FAILURE;
  }

  wdiAddSTASelfReqParamsRspCb =
                         (WDI_AddSTASelfParamsRspCb)pWDICtx->pfncRspCB;

  /*                                                                         
                                        
                                                                           */
  wpalMemoryCopy( &halAddStaSelfRsp.addStaSelfRspParams,
                  pEventData->pEventData,
                  sizeof(halAddStaSelfRsp.addStaSelfRspParams));


  wdiAddSTASelfParams.wdiStatus   =
    WDI_HAL_2_WDI_STATUS(halAddStaSelfRsp.addStaSelfRspParams.status);

  wdiAddSTASelfParams.ucSTASelfIdx   =
    halAddStaSelfRsp.addStaSelfRspParams.selfStaIdx;
  wdiAddSTASelfParams.dpuIdx =
    halAddStaSelfRsp.addStaSelfRspParams.dpuIdx;
  wdiAddSTASelfParams.dpuSignature =
    halAddStaSelfRsp.addStaSelfRspParams.dpuSignature;

  wpalMemoryCopy(wdiAddSTASelfParams.macSelfSta,
                 pWDICtx->wdiCacheAddSTASelfReq.wdiAddSTASelfInfo.selfMacAddr,
                 WDI_MAC_ADDR_LEN);


#ifdef HAL_SELF_STA_PER_BSS

  /*                                */

  /*                                     */
  /*                                    */
  /*                                     */

  //                                         

  /*               */
  wdiAddSTAParam.dpuIndex              = wdiAddSTASelfParams.dpuIdx;
  wdiAddSTAParam.dpuSig                = wdiAddSTASelfParams.dpuSignature;
  wdiAddSTAParam.bcastDpuSignature     = wdiAddSTASelfParams.dpuSignature;
  wdiAddSTAParam.bcastMgmtDpuSignature = wdiAddSTASelfParams.dpuSignature;
  wdiAddSTAParam.bcastDpuIndex         = wdiAddSTASelfParams.dpuIdx;
  wdiAddSTAParam.bcastMgmtDpuIndex     = wdiAddSTASelfParams.dpuIdx;

  wpalMemoryCopy(wdiAddSTAParam.staMacAddr, wdiAddSTASelfParams.macSelfSta,
                 WDI_MAC_ADDR_LEN);

  wdiAddSTAParam.ucStaType = WDI_STA_ENTRY_SELF; /*          */
  wdiAddSTAParam.ucSTAIdx = wdiAddSTASelfParams.ucSTASelfIdx;

  if(halAddStaSelfRsp.addStaSelfRspParams.status
     != eHAL_STATUS_ADD_STA_SELF_IGNORED_REF_COUNT_NOT_ZERO)
  {
     (void)WDI_STATableAddSta(pWDICtx,&wdiAddSTAParam);
  }
#endif

  /*           */
  wdiAddSTASelfReqParamsRspCb( &wdiAddSTASelfParams, pWDICtx->pRspCBUserData);

  return WDI_STATUS_SUCCESS;
}/*                        */



/* 
                                                        
                                                         

                                                         
                                                                    

     
                                    
*/
WDI_Status
WDI_ProcessDelSTASelfRsp
(
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
)
{
  WDI_DelSTASelfRspParamsType     wdiDelStaSelfRspParams;
  WDI_DelSTASelfRspCb             wdiDelStaSelfRspCb;
  tDelStaSelfRspParams            delStaSelfRspParams;
  wpt_uint8                       ucStaIdx;

  /*                                                                        */

  /*                                                                         
                
                                                                             */
  if (( NULL == pWDICtx ) || ( NULL == pEventData ) ||
      ( NULL == pEventData->pEventData))
  {
    WPAL_TRACE( eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_WARN,
                "%s: Invalid parameters", __FUNCTION__);
    WDI_ASSERT(0);
    return WDI_STATUS_E_FAILURE;
  }

  wdiDelStaSelfRspCb = (WDI_DelSTASelfRspCb)pWDICtx->pfncRspCB;

  /*                                                                         
                                        
                                                                             */

  wpalMemoryCopy( &delStaSelfRspParams,
                        (wpt_uint8*)pEventData->pEventData,
                              sizeof(tDelStaSelfRspParams));

  wdiDelStaSelfRspParams.wdiStatus   =
    WDI_HAL_2_WDI_STATUS(delStaSelfRspParams.status);

  /*                                  
                                                      */
  if( eHAL_STATUS_SUCCESS == delStaSelfRspParams.status )
  {
    WDI_Status wdiStatus;
    wdiStatus = WDI_STATableFindStaidByAddr(pWDICtx,
                               delStaSelfRspParams.selfMacAddr,
                               &ucStaIdx);
    if(WDI_STATUS_E_FAILURE == wdiStatus)
    {
      WPAL_TRACE( eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_ERROR,
                 "%s: Unable to extract the STA Idx ", __FUNCTION__);
      WDI_ASSERT(0);
      return WDI_STATUS_E_FAILURE;
    }
    WDI_STATableDelSta(pWDICtx, ucStaIdx);
  }

  /*           */
  wdiDelStaSelfRspCb(&wdiDelStaSelfRspParams, (void*) pWDICtx->pRspCBUserData);

  return WDI_STATUS_SUCCESS;
}

#ifdef FEATURE_OEM_DATA_SUPPORT
/* 
                                                  
                                                         

                                                         
                                                                    

     
                                    
*/
#define OFFSET_OF(structType,fldName)   (&((structType*)0)->fldName)

WDI_Status
WDI_ProcessStartOemDataRsp
(
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
)
{
  WDI_oemDataRspCb           wdiOemDataRspCb;
  WDI_oemDataRspParamsType*  wdiOemDataRspParams;
  tStartOemDataRspParams*    halStartOemDataRspParams;

  /*                                                                         
                
                                                                           */
  if (( NULL == pWDICtx ) || ( NULL == pEventData ) ||
      ( NULL == pEventData->pEventData))
  {
     WPAL_TRACE( eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_WARN,
                 "%s: Invalid parameters", __FUNCTION__);
     WDI_ASSERT(0);
     return WDI_STATUS_E_FAILURE;
  }

  wdiOemDataRspCb = (WDI_oemDataRspCb)pWDICtx->pfncRspCB;

   /*                                                                         
                                         
                                                                            */
  halStartOemDataRspParams = (tStartOemDataRspParams *)pEventData->pEventData;


  //                                                                     
  //           

  //                                              
  wdiOemDataRspParams = wpalMemoryAllocate(sizeof(WDI_oemDataRspParamsType)) ;

  if(NULL == wdiOemDataRspParams)
  {
    WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_WARN,
            "Failed to allocate memory in OEM DATA Response %x %x %x ",
                pWDICtx, pEventData, pEventData->pEventData);
    WDI_ASSERT(0);
    return WDI_STATUS_E_FAILURE;
  }

  /*                                */
  wpalMemoryCopy(wdiOemDataRspParams->oemDataRsp, halStartOemDataRspParams->oemDataRsp, OEM_DATA_RSP_SIZE);

  /*           */
  wdiOemDataRspCb(wdiOemDataRspParams, pWDICtx->pRspCBUserData);

  //                                                         
  wpalMemoryFree(wdiOemDataRspParams);

  return WDI_STATUS_SUCCESS;
}/*                          */
#endif

/*                                                                           
                                                        
                                                                           */

/* 
                                                                   
                                                

                                                         
                                                                    

     
                                    
*/
WDI_Status
WDI_ProcessChannelSwitchRsp
(
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
)
{
  WDI_SwitchCHRspParamsType   wdiSwitchChRsp;
  WDI_SwitchChRspCb           wdiChSwitchRspCb;
  tSwitchChannelRspParams     halSwitchChannelRsp;
  /*                                                                        */

  /*                                                                         
                
                                                                           */
  if (( NULL == pWDICtx ) || ( NULL == pEventData ) ||
      ( NULL == pEventData->pEventData))
  {
     WPAL_TRACE( eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_WARN,
                 "%s: Invalid parameters", __FUNCTION__);
     WDI_ASSERT(0);
     return WDI_STATUS_E_FAILURE;
  }

  wdiChSwitchRspCb = (WDI_SwitchChRspCb)pWDICtx->pfncRspCB;

  /*                                                                         
                                        
                                                                           */
  wpalMemoryCopy( &halSwitchChannelRsp,
                  (wpt_uint8*)pEventData->pEventData,
                  sizeof(halSwitchChannelRsp));

  wdiSwitchChRsp.wdiStatus   =
               WDI_HAL_2_WDI_STATUS(halSwitchChannelRsp.status);
  wdiSwitchChRsp.ucChannel = halSwitchChannelRsp.channelNumber;

#ifdef WLAN_FEATURE_VOWIFI
  wdiSwitchChRsp.ucTxMgmtPower =  halSwitchChannelRsp.txMgmtPower;
#endif

  /*           */
  wdiChSwitchRspCb( &wdiSwitchChRsp, pWDICtx->pRspCBUserData);

  return WDI_STATUS_SUCCESS;
}/*                           */


/* 
                                                               
                                                

                                                         
                                                                    

     
                                    
*/
WDI_Status
WDI_ProcessConfigStaRsp
(
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
)
{
  WDI_ConfigSTARspParamsType    wdiCfgSTAParams;
  WDI_ConfigSTARspCb            wdiConfigSTARspCb;
  WDI_AddStaParams              wdiAddSTAParam;

  WDI_BSSSessionType*           pBSSSes             = NULL;
  wpt_uint8                     ucCurrentBSSSesIdx  = 0;

  tConfigStaRspMsg              halConfigStaRsp;
  /*                                                                        */

  /*                                                                         
                
                                                                           */
  if (( NULL == pWDICtx ) || ( NULL == pEventData ) ||
      ( NULL == pEventData->pEventData))
  {
     WPAL_TRACE( eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_WARN,
                 "%s: Invalid parameters", __FUNCTION__);
     WDI_ASSERT(0);
     return WDI_STATUS_E_FAILURE;
  }

  wdiConfigSTARspCb = (WDI_ConfigSTARspCb)pWDICtx->pfncRspCB;

  /*                                                                         
                                        
                                                                           */
  wpalMemoryCopy( &halConfigStaRsp.configStaRspParams,
                  pEventData->pEventData,
                  sizeof(halConfigStaRsp.configStaRspParams));


  wdiCfgSTAParams.ucSTAIdx    = halConfigStaRsp.configStaRspParams.staIdx;
  wdiCfgSTAParams.ucBssIdx    = halConfigStaRsp.configStaRspParams.bssIdx;
  wdiCfgSTAParams.ucUcastSig  = halConfigStaRsp.configStaRspParams.ucUcastSig;
  wdiCfgSTAParams.ucBcastSig  = halConfigStaRsp.configStaRspParams.ucBcastSig;
  wdiCfgSTAParams.ucMgmtSig   = halConfigStaRsp.configStaRspParams.ucMgmtSig;

   /*                                                                     
           */
   wpalMemoryCopy( wdiCfgSTAParams.macSTA,
          pWDICtx->wdiCachedConfigStaReq.wdiReqInfo.macSTA,
          WDI_MAC_ADDR_LEN);

  wdiCfgSTAParams.wdiStatus   =
    WDI_HAL_2_WDI_STATUS(halConfigStaRsp.configStaRspParams.status);

  wdiCfgSTAParams.ucDpuIndex = halConfigStaRsp.configStaRspParams.dpuIndex;
  wdiCfgSTAParams.ucBcastDpuIndex = halConfigStaRsp.configStaRspParams.bcastDpuIndex;
  wdiCfgSTAParams.ucBcastMgmtDpuIdx = halConfigStaRsp.configStaRspParams.bcastMgmtDpuIdx;

  if ( WDI_STATUS_SUCCESS == wdiCfgSTAParams.wdiStatus )
  {
    if ( WDI_ADD_STA == pWDICtx->wdiCachedConfigStaReq.wdiReqInfo.wdiAction )
    {
      /*                  */
      wdiAddSTAParam.ucSTAIdx = halConfigStaRsp.configStaRspParams.staIdx;
      wdiAddSTAParam.dpuSig   = halConfigStaRsp.configStaRspParams.ucUcastSig;
      wdiAddSTAParam.dpuIndex = halConfigStaRsp.configStaRspParams.dpuIndex;

      /*                                                          */
      wdiAddSTAParam.ucWmmEnabled =
        pWDICtx->wdiCachedConfigStaReq.wdiReqInfo.ucWMMEnabled;
      wdiAddSTAParam.ucHTCapable  =
        pWDICtx->wdiCachedConfigStaReq.wdiReqInfo.ucHTCapable;
      wdiAddSTAParam.ucStaType    =
        pWDICtx->wdiCachedConfigStaReq.wdiReqInfo.wdiSTAType;

      /*                    */
      wpalMemoryCopy(wdiAddSTAParam.staMacAddr,
                     pWDICtx->wdiCachedConfigStaReq.wdiReqInfo.macSTA,
                     WDI_MAC_ADDR_LEN);

      wpalMemoryCopy(wdiAddSTAParam.macBSSID,
                     pWDICtx->wdiCachedConfigStaReq.wdiReqInfo.macBSSID ,
                     WDI_MAC_ADDR_LEN);

      ucCurrentBSSSesIdx = WDI_FindAssocSession( pWDICtx,
                    pWDICtx->wdiCachedConfigStaReq.wdiReqInfo.macBSSID,
                    &pBSSSes);

      if ( NULL == pBSSSes )
      {
        WPAL_TRACE( eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_ERROR,
                  "Association for this BSSID is not in place");

        WDI_ASSERT(0);
        return WDI_STATUS_E_NOT_ALLOWED;
      }

      /*                           */
      wdiAddSTAParam.bcastMgmtDpuIndex =
         halConfigStaRsp.configStaRspParams.bcastMgmtDpuIdx;
      wdiAddSTAParam.bcastMgmtDpuSignature =
         halConfigStaRsp.configStaRspParams.ucMgmtSig;
      wdiAddSTAParam.bcastDpuIndex  =
         halConfigStaRsp.configStaRspParams.bcastDpuIndex;
      wdiAddSTAParam.bcastDpuSignature =
         halConfigStaRsp.configStaRspParams.ucBcastSig;
      wdiAddSTAParam.ucRmfEnabled          = pBSSSes->ucRmfEnabled;
      wdiAddSTAParam.ucBSSIdx              = ucCurrentBSSSesIdx;

      WDI_STATableAddSta(pWDICtx,&wdiAddSTAParam);
    }
    if( WDI_UPDATE_STA == pWDICtx->wdiCachedConfigStaReq.wdiReqInfo.wdiAction )
    {
       WDI_StaStruct* pSTATable = (WDI_StaStruct*) pWDICtx->staTable;

       pSTATable[halConfigStaRsp.configStaRspParams.staIdx].bcastDpuIndex =
          halConfigStaRsp.configStaRspParams.bcastDpuIndex;
       pSTATable[halConfigStaRsp.configStaRspParams.staIdx].bcastDpuSignature =
          halConfigStaRsp.configStaRspParams.ucBcastSig;
       pSTATable[halConfigStaRsp.configStaRspParams.staIdx].bcastMgmtDpuIndex =
          halConfigStaRsp.configStaRspParams.bcastMgmtDpuIdx;
       pSTATable[halConfigStaRsp.configStaRspParams.staIdx].bcastMgmtDpuSignature =
          halConfigStaRsp.configStaRspParams.ucMgmtSig;
       pSTATable[halConfigStaRsp.configStaRspParams.staIdx].bssIdx =
          halConfigStaRsp.configStaRspParams.bssIdx;
       pSTATable[halConfigStaRsp.configStaRspParams.staIdx].dpuIndex =
          halConfigStaRsp.configStaRspParams.dpuIndex;
       pSTATable[halConfigStaRsp.configStaRspParams.staIdx].dpuSig =
          halConfigStaRsp.configStaRspParams.ucUcastSig;
    }
  }

  /*           */
  wdiConfigSTARspCb( &wdiCfgSTAParams, pWDICtx->pRspCBUserData);

  return WDI_STATUS_SUCCESS;
}/*                       */


/* 
                                                          
                                                         

                                                         
                                                                    

     
                                    
*/
WDI_Status
WDI_ProcessSetLinkStateRsp
(
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
)
{
  WDI_Status              wdiStatus;
  eHalStatus              halStatus;
  WDI_SetLinkStateRspCb   wdiSetLinkStateRspCb;

  WDI_BSSSessionType*     pBSSSes              = NULL;
  wpt_uint8               ucCurrentBSSSesIdx   = 0;
  /*                                                                        */

  /*                                                                         
                
                                                                           */
  if (( NULL == pWDICtx ) || ( NULL == pEventData ) ||
      ( NULL == pEventData->pEventData))
  {
     WPAL_TRACE( eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_WARN,
                 "%s: Invalid parameters", __FUNCTION__);
     WDI_ASSERT(0);
     return WDI_STATUS_E_FAILURE;
  }

  wdiSetLinkStateRspCb = (WDI_SetLinkStateRspCb)pWDICtx->pfncRspCB;

  wpalMutexAcquire(&pWDICtx->wptMutex);

  /*                                                                    
                                                                     
                                   */
  if ( WDI_LINK_IDLE_STATE ==
       pWDICtx->wdiCacheSetLinkStReq.wdiLinkInfo.wdiLinkState )
  {
    /*                                                                        
                                                
                                                                            */
    ucCurrentBSSSesIdx = WDI_FindAssocSession( pWDICtx,
                        pWDICtx->wdiCacheSetLinkStReq.wdiLinkInfo.macBSSID,
                        &pBSSSes);

    /*                                                                       
                                                                      
                                
                                                                             */
    if ( NULL == pBSSSes )
    {
      WPAL_TRACE( eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_INFO,
                "Set link response received outside association session");
    }
    else
    {
      /*                                                                      
                                                                               
              */
      if(( WDI_BTAMP_STA_MODE != pBSSSes->wdiBssType )&&
         ( WDI_BTAMP_AP_MODE != pBSSSes->wdiBssType ))
      {
         /*                                                                       
                                              
                                                                                */
         WDI_DeleteSession(pWDICtx, pBSSSes);

         /*                                                                       
                                                                              
                                 
                                                                                */
         if ( ucCurrentBSSSesIdx == pWDICtx->ucCurrentBSSSesIdx )
         {
           /*                                   */
           pWDICtx->bAssociationInProgress = eWLAN_PAL_FALSE;
           /*                                                                        */
           WDI_DequeueAssocRequest(pWDICtx);
         }
      }
    }
  }
  /*                                                                        
                    */
  if ( WDI_LINK_POSTASSOC_STATE ==
       pWDICtx->wdiCacheSetLinkStReq.wdiLinkInfo.wdiLinkState )
  {
     pWDICtx->bAssociationInProgress = eWLAN_PAL_FALSE;
     WDI_DequeueAssocRequest(pWDICtx);
  }

  wpalMutexRelease(&pWDICtx->wptMutex);

  /*                                                                         
                                        
                                                                           */
  wpalMemoryCopy( &halStatus,
                  pEventData->pEventData,
                  sizeof(halStatus));

  wdiStatus   =   WDI_HAL_2_WDI_STATUS(halStatus);

  /*           */
  wdiSetLinkStateRspCb( wdiStatus, pWDICtx->pRspCBUserData);

  return WDI_STATUS_SUCCESS;
}/*                          */

/* 
                                                                 
                                             

                                                         
                                                                    

     
                                    
*/
WDI_Status
WDI_ProcessGetStatsRsp
(
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
)
{
  WDI_GetStatsRspParamsType   *wdiGetStatsRsp;
  WDI_GetStatsRspCb           wdiGetStatsRspCb;
  tHalStatsRspParams*         pHalStatsRspParams;

  /*                                                                        */

  /*                                                                         
                
                                                                           */
  if (( NULL == pWDICtx ) || ( NULL == pEventData ) ||
      ( NULL == pEventData->pEventData))
  {
     WPAL_TRACE( eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_WARN,
                 "%s: Invalid parameters", __FUNCTION__);
     WDI_ASSERT(0);
     return WDI_STATUS_E_FAILURE;
  }

  /*                                                                         
                                        
                                                                           */
  pHalStatsRspParams = (tHalStatsRspParams *)pEventData->pEventData;

  /*                                   */
  wdiGetStatsRsp = (WDI_GetStatsRspParamsType *)wpalMemoryAllocate(
              pHalStatsRspParams->msgLen - sizeof(tHalStatsRspParams)
              + sizeof(WDI_GetStatsRspParamsType));

  if(NULL == wdiGetStatsRsp)
  {
    WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_ERROR,
                "Failed to allocate memory in Get Stats Response %x %x %x ",
                 pWDICtx, pEventData, pEventData->pEventData);
    WDI_ASSERT(0);
    return WDI_STATUS_E_FAILURE;
  }

  wdiGetStatsRspCb = (WDI_GetStatsRspCb)pWDICtx->pfncRspCB;

  wpalMemoryZero(wdiGetStatsRsp, pHalStatsRspParams->msgLen);
  wdiGetStatsRsp->usMsgType  = pHalStatsRspParams->msgType;
  wdiGetStatsRsp->usMsgLen   = pHalStatsRspParams->msgLen;
  wdiGetStatsRsp->wdiStatus  = WDI_HAL_2_WDI_STATUS(pHalStatsRspParams->status);
  wdiGetStatsRsp->ucSTAIdx   = pHalStatsRspParams->staId;
  wdiGetStatsRsp->uStatsMask = pHalStatsRspParams->statsMask;

  /*                                                                         */
  wpalMemoryCopy(wdiGetStatsRsp + 1,
   (wpt_uint8*)pEventData->pEventData + sizeof(tHalStatsRspParams),
   pHalStatsRspParams->msgLen - sizeof(tHalStatsRspParams));

  /*           */
  wdiGetStatsRspCb( wdiGetStatsRsp, pWDICtx->pRspCBUserData);

  wpalMemoryFree(wdiGetStatsRsp);

  return WDI_STATUS_SUCCESS;
}/*                      */


/* 
                                                                  
                                             

                                                         
                                                                    

     
                                    
*/
WDI_Status
WDI_ProcessUpdateCfgRsp
(
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
)
{
  WDI_Status           wdiStatus;
  eHalStatus           halStatus;
  WDI_UpdateCfgRspCb   wdiUpdateCfgRspCb;
  /*                                                                        */

  /*                                                                         
                
                                                                           */
  if (( NULL == pWDICtx ) || ( NULL == pEventData ) ||
      ( NULL == pEventData->pEventData))
  {
     WPAL_TRACE( eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_WARN,
                 "%s: Invalid parameters", __FUNCTION__);
     WDI_ASSERT(0);
     return WDI_STATUS_E_FAILURE;
  }

  wdiUpdateCfgRspCb = (WDI_UpdateCfgRspCb)pWDICtx->pfncRspCB;

  /*                                                                         
                                        
                                                                           */
  halStatus = *((eHalStatus*)pEventData->pEventData);
  wdiStatus   =   WDI_HAL_2_WDI_STATUS(halStatus);

  /*           */
  wdiUpdateCfgRspCb( wdiStatus, pWDICtx->pRspCBUserData);

  return WDI_STATUS_SUCCESS;
}/*                       */



/* 
                                                           
                                                

                                                         
                                                                    

     
                                    
*/
WDI_Status
WDI_ProcessAddBARsp
(
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
)
{
  WDI_AddBARspCb          wdiAddBARspCb;

  tAddBARspParams         halAddBARsp;
  WDI_AddBARspinfoType    wdiAddBARsp;

  /*                                                                        */

  /*                                                                         
                
                                                                           */
  if (( NULL == pWDICtx ) || ( NULL == pEventData ) ||
      ( NULL == pEventData->pEventData))
  {
     WPAL_TRACE( eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_WARN,
                 "%s: Invalid parameters", __FUNCTION__);
     WDI_ASSERT(0);
     return WDI_STATUS_E_FAILURE;
  }

  wdiAddBARspCb = (WDI_AddBARspCb)pWDICtx->pfncRspCB;

  /*                                                                         
                                        
                                                                           */
  wpalMemoryCopy( &halAddBARsp,
                  pEventData->pEventData,
                  sizeof(halAddBARsp));

  wdiAddBARsp.wdiStatus = WDI_HAL_2_WDI_STATUS(halAddBARsp.status);

  if ( WDI_STATUS_SUCCESS == wdiAddBARsp.wdiStatus )
  {
    wdiAddBARsp.ucBaDialogToken = halAddBARsp.baDialogToken;
  }

  /*           */
  wdiAddBARspCb( &wdiAddBARsp, pWDICtx->pRspCBUserData);

  return WDI_STATUS_SUCCESS;
}/*                          */

/* 
                                                           
                                                

                                                         
                                                                    

     
                                    
*/
WDI_Status
WDI_ProcessTriggerBARsp
(
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
)
{
  WDI_TriggerBARspCb      wdiTriggerBARspCb;

  tTriggerBARspParams*           halTriggerBARsp;
  tTriggerBaRspCandidate*        halBaCandidate;
  WDI_TriggerBARspParamsType*    wdiTriggerBARsp;
  WDI_TriggerBARspCandidateType* wdiTriggerBARspCandidate;
  wpt_uint16                     index;
  wpt_uint16                     TidIndex;

  /*                                                                        */

  /*                                                                         
                
                                                                           */
  if (( NULL == pWDICtx ) || ( NULL == pEventData ) ||
      ( NULL == pEventData->pEventData))
  {
     WPAL_TRACE( eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_WARN,
                 "%s: Invalid parameters", __FUNCTION__);
     WDI_ASSERT(0);
     return WDI_STATUS_E_FAILURE;
  }

  wdiTriggerBARspCb = (WDI_TriggerBARspCb)pWDICtx->pfncRspCB;

  /*                                                                         
                                        
                                                                           */
  halTriggerBARsp = (tTriggerBARspParams *)pEventData->pEventData;

  wdiTriggerBARsp = wpalMemoryAllocate(sizeof(WDI_TriggerBARspParamsType) +
                      halTriggerBARsp->baCandidateCnt *
                      sizeof(WDI_TriggerBARspCandidateType));
  if(NULL == wdiTriggerBARsp)
  {
    WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_WARN,
                "Failed to allocate memory in Trigger BA Response %x %x %x ",
                 pWDICtx, pEventData, pEventData->pEventData);
    wpalMemoryFree(halTriggerBARsp);
    WDI_ASSERT(0);
    return WDI_STATUS_E_FAILURE;
  }

  wdiTriggerBARsp->wdiStatus = WDI_HAL_2_WDI_STATUS(halTriggerBARsp->status);

  if ( WDI_STATUS_SUCCESS == wdiTriggerBARsp->wdiStatus)
  {
    wdiTriggerBARsp->usBaCandidateCnt = halTriggerBARsp->baCandidateCnt;
    wpalMemoryCopy(wdiTriggerBARsp->macBSSID,
                                 halTriggerBARsp->bssId , WDI_MAC_ADDR_LEN);

    wdiTriggerBARspCandidate = (WDI_TriggerBARspCandidateType*)(wdiTriggerBARsp + 1);
    halBaCandidate = (tTriggerBaRspCandidate*)(halTriggerBARsp + 1);

    for(index = 0; index < wdiTriggerBARsp->usBaCandidateCnt; index++)
    {
      wpalMemoryCopy(wdiTriggerBARspCandidate->macSTA,
                                  halBaCandidate->staAddr, WDI_MAC_ADDR_LEN);
      for(TidIndex = 0; TidIndex < STA_MAX_TC; TidIndex++)
      {
        wdiTriggerBARspCandidate->wdiBAInfo[TidIndex].fBaEnable =
                            halBaCandidate->baInfo[TidIndex].fBaEnable;
        wdiTriggerBARspCandidate->wdiBAInfo[TidIndex].startingSeqNum =
                            halBaCandidate->baInfo[TidIndex].startingSeqNum;
      }
      wdiTriggerBARspCandidate++;
      halBaCandidate++;
    }
  }

  /*           */
  wdiTriggerBARspCb( wdiTriggerBARsp, pWDICtx->pRspCBUserData);

  wpalMemoryFree(wdiTriggerBARsp);
  return WDI_STATUS_SUCCESS;
}/*                          */

/* 
                                                                         
                                                

                                                         
                                                                    

     
                                    
*/
WDI_Status
WDI_ProcessUpdateBeaconParamsRsp
(
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
)
{
  WDI_Status       wdiStatus;
  eHalStatus       halStatus;
  WDI_UpdateBeaconParamsRspCb   wdiUpdateBeaconParamsRspCb;
  /*                                                                        */

  /*                                                                         
                
                                                                           */
  if (( NULL == pWDICtx ) || ( NULL == pEventData ) ||
      ( NULL == pEventData->pEventData))
  {
     WPAL_TRACE( eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_WARN,
                 "%s: Invalid parameters", __FUNCTION__);
     WDI_ASSERT(0);
     return WDI_STATUS_E_FAILURE;
  }

  wdiUpdateBeaconParamsRspCb = (WDI_UpdateBeaconParamsRspCb)pWDICtx->pfncRspCB;

  /*                                                                         
                                        
                                                                           */
  wpalMemoryCopy( &halStatus,
                  pEventData->pEventData,
                  sizeof(halStatus));

  wdiStatus   =   WDI_HAL_2_WDI_STATUS(halStatus);

  /*           */
  wdiUpdateBeaconParamsRspCb( wdiStatus, pWDICtx->pRspCBUserData);

  return WDI_STATUS_SUCCESS;
}/*                                */

/* 
                                                                         
                                                

                                                         
                                                                    

     
                                    
*/
WDI_Status
WDI_ProcessSendBeaconParamsRsp
(
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
)
{
  WDI_Status       wdiStatus;
  eHalStatus       halStatus;
  WDI_SendBeaconParamsRspCb   wdiSendBeaconParamsRspCb;
  /*                                                                        */

  /*                                                                         
                
                                                                           */
  if (( NULL == pWDICtx ) || ( NULL == pEventData ) ||
      ( NULL == pEventData->pEventData))
  {
     WPAL_TRACE( eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_WARN,
                 "%s: Invalid parameters", __FUNCTION__);
     WDI_ASSERT(0);
     return WDI_STATUS_E_FAILURE;
  }

  wdiSendBeaconParamsRspCb = (WDI_SendBeaconParamsRspCb)pWDICtx->pfncRspCB;

  /*                                                                         
                                        
                                                                           */
  wpalMemoryCopy( &halStatus,
                  pEventData->pEventData,
                  sizeof(halStatus));

  wdiStatus   =   WDI_HAL_2_WDI_STATUS(halStatus);

  /*           */
  wdiSendBeaconParamsRspCb( wdiStatus, pWDICtx->pRspCBUserData);

  return WDI_STATUS_SUCCESS;
}/*                              */


/* 
                                                               
                                                                

                                                         
                                                                    

     
                                    
*/
WDI_Status
WDI_ProcessUpdateProbeRspTemplateRsp
(
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
)
{
  WDI_Status       wdiStatus;
  eHalStatus       halStatus;
  WDI_UpdateProbeRspTemplateRspCb   wdiUpdProbeRspTemplRspCb;
  /*                                                                        */

  /*                                                                         
                
                                                                           */
  if (( NULL == pWDICtx ) || ( NULL == pEventData ) ||
      ( NULL == pEventData->pEventData))
  {
     WPAL_TRACE( eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_WARN,
                 "%s: Invalid parameters", __FUNCTION__);
     WDI_ASSERT(0);
     return WDI_STATUS_E_FAILURE;
  }

  wdiUpdProbeRspTemplRspCb = (WDI_UpdateProbeRspTemplateRspCb)pWDICtx->pfncRspCB;

  /*                                                                         
                                        
                                                                           */
  wpalMemoryCopy( &halStatus,
                  pEventData->pEventData,
                  sizeof(halStatus));

  wdiStatus   =   WDI_HAL_2_WDI_STATUS(halStatus);

  /*           */
  wdiUpdProbeRspTemplRspCb( wdiStatus, pWDICtx->pRspCBUserData);

  return WDI_STATUS_SUCCESS;
}/*                                    */

  /* 
                                                                     
                                                

                                                         
                                                                    

     
                                    
*/
WDI_Status
WDI_ProcessSetMaxTxPowerRsp
(
  WDI_ControlBlockType*          pWDICtx,
  WDI_EventInfoType*             pEventData
)
{
  tSetMaxTxPwrRspMsg             halTxpowerrsp;

  WDI_SetMaxTxPowerRspMsg        wdiSetMaxTxPowerRspMsg;

  WDA_SetMaxTxPowerRspCb         wdiReqStatusCb;
  /*                                                                        */

  /*                                                                         
                
                                                                           */
  if (( NULL == pWDICtx ) || ( NULL == pEventData ) ||
      ( NULL == pEventData->pEventData))
  {
     WPAL_TRACE( eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_WARN,
                 "%s: Invalid parameters", __FUNCTION__);
     WDI_ASSERT(0);
     return WDI_STATUS_E_FAILURE;
  }

  wdiReqStatusCb = (WDA_SetMaxTxPowerRspCb)pWDICtx->pfncRspCB;

  /*                                                                         
                                        
                                                                           */
  wpalMemoryCopy( &halTxpowerrsp.setMaxTxPwrRspParams,
                           pEventData->pEventData,
                           sizeof(halTxpowerrsp.setMaxTxPwrRspParams));

  if ( eHAL_STATUS_SUCCESS != halTxpowerrsp.setMaxTxPwrRspParams.status )
  {
     WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_ERROR,
              "Error status returned in Set Max Tx Power Response ");
     WDI_DetectedDeviceError( pWDICtx, WDI_ERR_BASIC_OP_FAILURE);
     return WDI_STATUS_E_FAILURE;
  }

  wdiSetMaxTxPowerRspMsg.wdiStatus =
         WDI_HAL_2_WDI_STATUS(halTxpowerrsp.setMaxTxPwrRspParams.status);
  wdiSetMaxTxPowerRspMsg.ucPower  = halTxpowerrsp.setMaxTxPwrRspParams.power;

  /*           */
  wdiReqStatusCb( &wdiSetMaxTxPowerRspMsg, pWDICtx->pRspCBUserData);

  return WDI_STATUS_SUCCESS;
}

#ifdef WLAN_FEATURE_P2P
/* 
                                                                      
                                                                

                                                         
                                                                    

     
                                    
*/
WDI_Status
WDI_ProcessP2PGONOARsp
(
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
)
{
  WDI_Status       wdiStatus;
  eHalStatus       halStatus;
  WDI_SetP2PGONOAReqParamsRspCb   wdiP2PGONOAReqParamsRspCb;
  /*                                                                        */

  /*                                                                         
                
                                                                           */
  if (( NULL == pWDICtx ) || ( NULL == pEventData ) ||
      ( NULL == pEventData->pEventData))
  {
     WPAL_TRACE( eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_WARN,
                 "%s: Invalid parameters", __FUNCTION__);
     WDI_ASSERT(0);
     return WDI_STATUS_E_FAILURE;
  }

  wdiP2PGONOAReqParamsRspCb = (WDI_SetP2PGONOAReqParamsRspCb)pWDICtx->pfncRspCB;

  /*                                                                         
                                        
                                                                           */
  wpalMemoryCopy( &halStatus,
                  pEventData->pEventData,
                  sizeof(halStatus));

  wdiStatus   =   WDI_HAL_2_WDI_STATUS(halStatus);

  /*           */
  wdiP2PGONOAReqParamsRspCb( wdiStatus, pWDICtx->pRspCBUserData);

  return WDI_STATUS_SUCCESS;
}/*                      */
#endif
/* 
                                                               
                                                

                                                         
                                                                    

     
                                    
*/
WDI_Status
WDI_ProcessEnterImpsRsp
(
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
)
{
  WDI_Status           wdiStatus;
  eHalStatus           halStatus;
  WDI_EnterImpsRspCb   wdiEnterImpsRspCb;
  /*                                                                        */

  /*                                                                         
                
                                                                           */
  if (( NULL == pWDICtx ) || ( NULL == pEventData ) ||
      ( NULL == pEventData->pEventData))
  {
     WPAL_TRACE( eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_WARN,
                 "%s: Invalid parameters", __FUNCTION__);
     WDI_ASSERT(0);
     return WDI_STATUS_E_FAILURE;
  }

  wdiEnterImpsRspCb = (WDI_EnterImpsRspCb)pWDICtx->pfncRspCB;

  /*                                                                         
                                        
                                                                           */
  halStatus = *((eHalStatus*)pEventData->pEventData);

  wdiStatus   =   WDI_HAL_2_WDI_STATUS(halStatus);

  /*                                                                           
                                               
                                                                                                 
   */
  if (wdiStatus != WDI_STATUS_SUCCESS) {

	  WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_FATAL,
		 "WDI PRocess Enter IMPS RSP failed With HAL Status Code: %d",halStatus);
	  /*                                                            
                                                   */
	  WDTS_SetPowerState(pWDICtx, WDTS_POWER_STATE_FULL, NULL);
  }
  /*           */
  wdiEnterImpsRspCb( wdiStatus, pWDICtx->pRspCBUserData);

  return WDI_STATUS_SUCCESS;
}/*                       */

/* 
                                                              
                                                

                                                         
                                                                    

     
                                    
*/
WDI_Status
WDI_ProcessExitImpsRsp
(
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
)
{
  WDI_Status           wdiStatus;
  eHalStatus           halStatus;
  WDI_ExitImpsRspCb    wdiExitImpsRspCb;
  /*                                                                        */

  /*                                                                         
                
                                                                           */
  if (( NULL == pWDICtx ) || ( NULL == pEventData ) ||
      ( NULL == pEventData->pEventData))
  {
     WPAL_TRACE( eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_WARN,
                 "%s: Invalid parameters", __FUNCTION__);
     WDI_ASSERT(0);
     return WDI_STATUS_E_FAILURE;
  }

  wdiExitImpsRspCb = (WDI_ExitImpsRspCb)pWDICtx->pfncRspCB;

  /*                                                                         
                                        
                                                                           */
  halStatus = *((eHalStatus*)pEventData->pEventData);
  wdiStatus   =   WDI_HAL_2_WDI_STATUS(halStatus);

  //                                           
  WDTS_SetPowerState(pWDICtx, WDTS_POWER_STATE_FULL, NULL);

  /*           */
  wdiExitImpsRspCb( wdiStatus, pWDICtx->pRspCBUserData);

  return WDI_STATUS_SUCCESS;
}/*                      */

/* 
                                                               
                                                

                                                         
                                                                    

     
                                    
*/
WDI_Status
WDI_ProcessEnterBmpsRsp
(
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
)
{
  WDI_Status           wdiStatus;
  eHalStatus           halStatus;
  WDI_EnterBmpsRspCb   wdiEnterBmpsRspCb;
  /*                                                                        */

  /*                                                                         
                
                                                                           */
  if (( NULL == pWDICtx ) || ( NULL == pEventData ) ||
      ( NULL == pEventData->pEventData))
  {
     WPAL_TRACE( eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_WARN,
                 "%s: Invalid parameters", __FUNCTION__);
     WDI_ASSERT(0);
     return WDI_STATUS_E_FAILURE;
  }

  wdiEnterBmpsRspCb = (WDI_EnterBmpsRspCb)pWDICtx->pfncRspCB;

  /*                                                                         
                                        
                                                                           */
  halStatus = *((eHalStatus*)pEventData->pEventData);
  wdiStatus   =   WDI_HAL_2_WDI_STATUS(halStatus);

  /*                                                                           
                                               
                                                                                                 
   */
   if (wdiStatus != WDI_STATUS_SUCCESS) {

	  WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_FATAL,
		 "WDI PRocess Enter BMPS RSP failed With HAL Status Code: %d",halStatus);
	  /*                                                            
                                                   */
	  WDTS_SetPowerState(pWDICtx, WDTS_POWER_STATE_FULL, NULL);
	  pWDICtx->bInBmps = eWLAN_PAL_FALSE;
   }

  /*           */
  wdiEnterBmpsRspCb( wdiStatus, pWDICtx->pRspCBUserData);

  return WDI_STATUS_SUCCESS;
}/*                       */

/* 
                                                              
                                                

                                                         
                                                                    

     
                                    
*/
WDI_Status
WDI_ProcessExitBmpsRsp
(
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
)
{
  WDI_Status           wdiStatus;
  eHalStatus           halStatus;
  WDI_ExitBmpsRspCb   wdiExitBmpsRspCb;
  /*                                                                        */

  /*                                                                         
                
                                                                           */
  if (( NULL == pWDICtx ) || ( NULL == pEventData ) ||
      ( NULL == pEventData->pEventData))
  {
     WPAL_TRACE( eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_WARN,
                 "%s: Invalid parameters", __FUNCTION__);
     WDI_ASSERT(0);
     return WDI_STATUS_E_FAILURE;
  }

  wdiExitBmpsRspCb = (WDI_ExitBmpsRspCb)pWDICtx->pfncRspCB;

  /*                                                                         
                                        
                                                                           */
  halStatus = *((eHalStatus*)pEventData->pEventData);
  wdiStatus   =   WDI_HAL_2_WDI_STATUS(halStatus);

  //                                           
  WDTS_SetPowerState(pWDICtx, WDTS_POWER_STATE_FULL, NULL);

  pWDICtx->bInBmps = eWLAN_PAL_FALSE;

  /*           */
  wdiExitBmpsRspCb( wdiStatus, pWDICtx->pRspCBUserData);

  return WDI_STATUS_SUCCESS;
}/*                      */

/* 
                                                                
                                                

                                                         
                                                                    

     
                                    
*/
WDI_Status
WDI_ProcessEnterUapsdRsp
(
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
)
{
  WDI_Status           wdiStatus;
  eHalStatus           halStatus;
  WDI_EnterUapsdRspCb   wdiEnterUapsdRspCb;
  /*                                                                        */

  /*                                                                         
                
                                                                           */
  if (( NULL == pWDICtx ) || ( NULL == pEventData ) ||
      ( NULL == pEventData->pEventData))
  {
     WPAL_TRACE( eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_WARN,
                 "%s: Invalid parameters", __FUNCTION__);
     WDI_ASSERT(0);
     return WDI_STATUS_E_FAILURE;
  }

  wdiEnterUapsdRspCb = (WDI_EnterUapsdRspCb)pWDICtx->pfncRspCB;

  /*                                                                         
                                        
                                                                           */
  halStatus = *((eHalStatus*)pEventData->pEventData);
  wdiStatus   =   WDI_HAL_2_WDI_STATUS(halStatus);

  if(WDI_STATUS_SUCCESS == wdiStatus)
  {
   //                                                                             
   //                                                                         
   //                                                                                  
   //                                    
   pWDICtx->ucDpuRF = BMUWQ_FW_DPU_TX;

#ifdef WLAN_PERF
   //                                                                   
   pWDICtx->uBdSigSerialNum++;
#endif
  }

  /*           */
  wdiEnterUapsdRspCb( wdiStatus, pWDICtx->pRspCBUserData);

  return WDI_STATUS_SUCCESS;
}/*                        */

/* 
                                                               
                                                

                                                         
                                                                    

     
                                    
*/
WDI_Status
WDI_ProcessExitUapsdRsp
(
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
)
{
  WDI_Status           wdiStatus;
  eHalStatus           halStatus;
  WDI_ExitUapsdRspCb   wdiExitUapsdRspCb;
  /*                                                                        */

  /*                                                                         
                
                                                                           */
  if (( NULL == pWDICtx ) || ( NULL == pEventData ) ||
      ( NULL == pEventData->pEventData))
  {
     WPAL_TRACE( eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_WARN,
                 "%s: Invalid parameters", __FUNCTION__);
     WDI_ASSERT(0);
     return WDI_STATUS_E_FAILURE;
  }

  wdiExitUapsdRspCb = (WDI_ExitUapsdRspCb)pWDICtx->pfncRspCB;

  /*                                                                         
                                        
                                                                           */
  halStatus = *((eHalStatus*)pEventData->pEventData);
  wdiStatus   =   WDI_HAL_2_WDI_STATUS(halStatus);

   //                                                                                 
   //                               
   pWDICtx->ucDpuRF = BMUWQ_BTQM_TX_MGMT;

#ifdef WLAN_PERF
   //                                                                   
   pWDICtx->uBdSigSerialNum++;
#endif

  /*           */
  wdiExitUapsdRspCb( wdiStatus, pWDICtx->pRspCBUserData);

  return WDI_STATUS_SUCCESS;
}/*                       */

/* 
                                                            
                                                         

                                                         
                                                                    

     
                                    
*/
WDI_Status
WDI_ProcessSetUapsdAcParamsRsp
(
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
)
{
  WDI_Status           wdiStatus;
  eHalStatus           halStatus;
  WDI_SetUapsdAcParamsCb   wdiSetUapsdAcParamsCb;
  /*                                                                        */

  /*                                                                         
                
                                                                           */
  if (( NULL == pWDICtx ) || ( NULL == pEventData ) ||
      ( NULL == pEventData->pEventData))
  {
     WPAL_TRACE( eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_WARN,
                 "%s: Invalid parameters", __FUNCTION__);
     WDI_ASSERT(0);
     return WDI_STATUS_E_FAILURE;
  }

  wdiSetUapsdAcParamsCb = (WDI_SetUapsdAcParamsCb)pWDICtx->pfncRspCB;

  /*                                                                         
                                        
                                                                           */
  halStatus = *((eHalStatus*)pEventData->pEventData);
  wdiStatus   =   WDI_HAL_2_WDI_STATUS(halStatus);

  /*           */
  wdiSetUapsdAcParamsCb( wdiStatus, pWDICtx->pRspCBUserData);

  return WDI_STATUS_SUCCESS;
}/*                              */

/* 
                                                               
                                                         

                                                         
                                                                    

     
                                    
*/
WDI_Status
WDI_ProcessUpdateUapsdParamsRsp
(
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
)
{
  WDI_Status           wdiStatus;
  eHalStatus           halStatus;
  WDI_UpdateUapsdParamsCb   wdiUpdateUapsdParamsCb;
  /*                                                                        */

  /*                                                                         
                
                                                                           */
  if (( NULL == pWDICtx ) || ( NULL == pEventData ) ||
      ( NULL == pEventData->pEventData))
  {
     WPAL_TRACE( eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_WARN,
                 "%s: Invalid parameters", __FUNCTION__);
     WDI_ASSERT(0);
     return WDI_STATUS_E_FAILURE;
  }

  wdiUpdateUapsdParamsCb = (WDI_UpdateUapsdParamsCb)pWDICtx->pfncRspCB;

  /*                                                                         
                                        
                                                                           */
  halStatus = *((eHalStatus*)pEventData->pEventData);
  wdiStatus   =   WDI_HAL_2_WDI_STATUS(halStatus);

  /*           */
  wdiUpdateUapsdParamsCb( wdiStatus, pWDICtx->pRspCBUserData);

  return WDI_STATUS_SUCCESS;
}/*                               */

/* 
                                                                
                                                         

                                                         
                                                                    

     
                                    
*/
WDI_Status
WDI_ProcessConfigureRxpFilterRsp
(
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
)
{
  WDI_Status           wdiStatus;
  eHalStatus           halStatus;
  WDI_ConfigureRxpFilterCb   wdiConfigureRxpFilterCb;
  /*                                                                        */

  /*                                                                         
                
                                                                           */
  if (( NULL == pWDICtx ) || ( NULL == pEventData ) ||
      ( NULL == pEventData->pEventData))
  {
     WPAL_TRACE( eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_WARN,
                 "%s: Invalid parameters", __FUNCTION__);
     WDI_ASSERT(0);
     return WDI_STATUS_E_FAILURE;
  }

  wdiConfigureRxpFilterCb = (WDI_ConfigureRxpFilterCb)pWDICtx->pfncRspCB;

  /*                                                                         
                                        
                                                                           */
  halStatus = *((eHalStatus*)pEventData->pEventData);
  wdiStatus   =   WDI_HAL_2_WDI_STATUS(halStatus);

  /*           */
  wdiConfigureRxpFilterCb( wdiStatus, pWDICtx->pRspCBUserData);

  return WDI_STATUS_SUCCESS;
}/*                                */

/* 
                                                             
                                                         

                                                         
                                                                    

     
                                    
*/
WDI_Status
WDI_ProcessSetBeaconFilterRsp
(
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
)
{
   WDI_Status           wdiStatus;
   eHalStatus           halStatus;
   WDI_SetBeaconFilterCb   wdiBeaconFilterCb;
   /*                                                                        */

   /*                                                                         
                 
                                                                            */
   if (( NULL == pWDICtx ) || ( NULL == pEventData ) ||
       ( NULL == pEventData->pEventData))
   {
      WPAL_TRACE( eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_WARN,
                  "%s: Invalid parameters", __FUNCTION__);
      WDI_ASSERT(0);
      return WDI_STATUS_E_FAILURE;
   }

   wdiBeaconFilterCb = (WDI_SetBeaconFilterCb)pWDICtx->pfncRspCB;

   /*                                                                         
                                         
                                                                            */
   halStatus = *((eHalStatus*)pEventData->pEventData);
   wdiStatus   =   WDI_HAL_2_WDI_STATUS(halStatus);

   /*           */
   wdiBeaconFilterCb( wdiStatus, pWDICtx->pRspCBUserData);

   return WDI_STATUS_SUCCESS;
}/*                             */

/* 
                                                                
                                                         

                                                         
                                                                    

     
                                    
*/
WDI_Status
WDI_ProcessRemBeaconFilterRsp
(
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
)
{
   WDI_Status           wdiStatus;
   eHalStatus           halStatus;
   WDI_RemBeaconFilterCb   wdiBeaconFilterCb;
   /*                                                                        */

   /*                                                                         
                 
                                                                            */
   if (( NULL == pWDICtx ) || ( NULL == pEventData ) ||
       ( NULL == pEventData->pEventData))
   {
      WPAL_TRACE( eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_WARN,
                  "%s: Invalid parameters", __FUNCTION__);
      WDI_ASSERT(0);
      return WDI_STATUS_E_FAILURE;
   }

   wdiBeaconFilterCb = (WDI_RemBeaconFilterCb)pWDICtx->pfncRspCB;

   /*                                                                         
                                         
                                                                            */
   halStatus = *((eHalStatus*)pEventData->pEventData);
   wdiStatus   =   WDI_HAL_2_WDI_STATUS(halStatus);

   /*           */
   wdiBeaconFilterCb( wdiStatus, pWDICtx->pRspCBUserData);

   return WDI_STATUS_SUCCESS;
}/*                             */

/* 
                                                               
                                                         

                                                         
                                                                    

     
                                    
*/
WDI_Status
WDI_ProcessSetRSSIThresoldsRsp
(
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
)
{
   WDI_Status           wdiStatus;
   eHalStatus           halStatus;
   WDI_SetRSSIThresholdsCb   wdiRSSIThresholdsCb;
   /*                                                                        */

   /*                                                                         
                 
                                                                            */
   if (( NULL == pWDICtx ) || ( NULL == pEventData ) ||
       ( NULL == pEventData->pEventData))
   {
      WPAL_TRACE( eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_WARN,
                  "%s: Invalid parameters", __FUNCTION__);
      WDI_ASSERT(0);
      return WDI_STATUS_E_FAILURE;
   }

   wdiRSSIThresholdsCb = (WDI_SetRSSIThresholdsCb)pWDICtx->pfncRspCB;

   /*                                                                         
                                         
                                                                            */
   halStatus = *((eHalStatus*)pEventData->pEventData);
   wdiStatus   =   WDI_HAL_2_WDI_STATUS(halStatus);

   /*           */
   wdiRSSIThresholdsCb( wdiStatus, pWDICtx->pRspCBUserData);

   return WDI_STATUS_SUCCESS;
}/*                              */

/* 
                                                        
                                                         

                                                         
                                                                    

     
                                    
*/
WDI_Status
WDI_ProcessHostOffloadRsp
(
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
)
{
   WDI_Status           wdiStatus;
   eHalStatus           halStatus;
   WDI_HostOffloadCb    wdiHostOffloadCb;
   /*                                                                        */

   /*                                                                         
                 
                                                                            */
   if (( NULL == pWDICtx ) || ( NULL == pEventData ) ||
       ( NULL == pEventData->pEventData))
   {
      WPAL_TRACE( eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_WARN,
                  "%s: Invalid parameters", __FUNCTION__);
      WDI_ASSERT(0);
      return WDI_STATUS_E_FAILURE;
   }

   wdiHostOffloadCb = (WDI_HostOffloadCb)pWDICtx->pfncRspCB;

   /*                                                                         
                                         
                                                                            */
   halStatus = *((eHalStatus*)pEventData->pEventData);
   wdiStatus   =   WDI_HAL_2_WDI_STATUS(halStatus);

   /*           */
   wdiHostOffloadCb( wdiStatus, pWDICtx->pRspCBUserData);

   return WDI_STATUS_SUCCESS;
}/*                         */

/* 
                                                      
                                                         

                                                         
                                                                    

     
                                    
*/
WDI_Status
WDI_ProcessKeepAliveRsp
(
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
)
{
   WDI_Status           wdiStatus;
   eHalStatus           halStatus;
   WDI_KeepAliveCb      wdiKeepAliveCb;
   /*                                                                        */
   WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_ERROR,
               "Received WDI_ProcessKeepAliveRsp Callback from HAL");


   /*                                                                         
                 
                                                                            */
   if (( NULL == pWDICtx ) || ( NULL == pEventData ) ||
       ( NULL == pEventData->pEventData))
   {
      WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_WARN,
                  "%s: Invalid parameters", __FUNCTION__);
      WDI_ASSERT(0);
      return WDI_STATUS_E_FAILURE;
   }

   wdiKeepAliveCb = (WDI_KeepAliveCb)pWDICtx->pfncRspCB;

   /*                                                                         
                                         
                                                                            */
   halStatus = *((eHalStatus*)pEventData->pEventData);
   wdiStatus   =   WDI_HAL_2_WDI_STATUS(halStatus);

   /*           */
   wdiKeepAliveCb( wdiStatus, pWDICtx->pRspCBUserData);

   return WDI_STATUS_SUCCESS;
}/*                       */

/* 
                                                         
                                                         

                                                         
                                                                    

     
                                    
*/
WDI_Status
WDI_ProcessWowlAddBcPtrnRsp
(
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
)
{
   WDI_Status           wdiStatus;
   eHalStatus           halStatus;
   WDI_WowlAddBcPtrnCb    wdiWowlAddBcPtrnCb;
   /*                                                                        */

   /*                                                                         
                 
                                                                            */
   if (( NULL == pWDICtx ) || ( NULL == pEventData ) ||
       ( NULL == pEventData->pEventData))
   {
      WPAL_TRACE( eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_WARN,
                  "%s: Invalid parameters", __FUNCTION__);
      WDI_ASSERT(0);
      return WDI_STATUS_E_FAILURE;
   }

   wdiWowlAddBcPtrnCb = (WDI_WowlAddBcPtrnCb)pWDICtx->pfncRspCB;

   /*                                                                         
                                         
                                                                            */
   halStatus = *((eHalStatus*)pEventData->pEventData);
   wdiStatus   =   WDI_HAL_2_WDI_STATUS(halStatus);

   /*           */
   wdiWowlAddBcPtrnCb( wdiStatus, pWDICtx->pRspCBUserData);

   return WDI_STATUS_SUCCESS;
}/*                           */

/* 
                                                            
                                                         

                                                         
                                                                    

     
                                    
*/
WDI_Status
WDI_ProcessWowlDelBcPtrnRsp
(
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
)
{
   WDI_Status           wdiStatus;
   eHalStatus           halStatus;
   WDI_WowlDelBcPtrnCb    wdiWowlDelBcPtrnCb;
   /*                                                                        */

   /*                                                                         
                 
                                                                            */
   if (( NULL == pWDICtx ) || ( NULL == pEventData ) ||
       ( NULL == pEventData->pEventData))
   {
      WPAL_TRACE( eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_WARN,
                  "%s: Invalid parameters", __FUNCTION__);
      WDI_ASSERT(0);
      return WDI_STATUS_E_FAILURE;
   }

   wdiWowlDelBcPtrnCb = (WDI_WowlDelBcPtrnCb)pWDICtx->pfncRspCB;

   /*                                                                         
                                         
                                                                            */
   halStatus = *((eHalStatus*)pEventData->pEventData);
   wdiStatus   =   WDI_HAL_2_WDI_STATUS(halStatus);

   /*           */
   wdiWowlDelBcPtrnCb( wdiStatus, pWDICtx->pRspCBUserData);

   return WDI_STATUS_SUCCESS;
}/*                           */

/* 
                                                               
                                                

                                                         
                                                                    

     
                                    
*/
WDI_Status
WDI_ProcessWowlEnterRsp
(
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
)
{
   WDI_Status           wdiStatus;
   eHalStatus           halStatus;
   WDI_WowlEnterReqCb   wdiWowlEnterCb;
   /*                                                                        */

   /*                                                                         
                 
                                                                            */
   if (( NULL == pWDICtx ) || ( NULL == pEventData ) ||
       ( NULL == pEventData->pEventData))
   {
      WPAL_TRACE( eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_WARN,
                  "%s: Invalid parameters", __FUNCTION__);
      WDI_ASSERT(0);
      return WDI_STATUS_E_FAILURE;
   }

   wdiWowlEnterCb = (WDI_WowlEnterReqCb)pWDICtx->pfncRspCB;

   /*                                                                         
                                         
                                                                            */
   halStatus = *((eHalStatus*)pEventData->pEventData);
   wdiStatus   =   WDI_HAL_2_WDI_STATUS(halStatus);

   /*           */
   wdiWowlEnterCb( wdiStatus, pWDICtx->pRspCBUserData);

   return WDI_STATUS_SUCCESS;
}/*                       */

/* 
                                                              
                                                

                                                         
                                                                    

     
                                    
*/
WDI_Status
WDI_ProcessWowlExitRsp
(
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
)
{
   WDI_Status           wdiStatus;
   eHalStatus           halStatus;
   WDI_WowlExitReqCb   wdiWowlExitCb;
   /*                                                                        */

   /*                                                                         
                 
                                                                            */
   if (( NULL == pWDICtx ) || ( NULL == pEventData ) ||
       ( NULL == pEventData->pEventData))
   {
      WPAL_TRACE( eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_WARN,
                  "%s: Invalid parameters", __FUNCTION__);
      WDI_ASSERT(0);
      return WDI_STATUS_E_FAILURE;
   }

   wdiWowlExitCb = (WDI_WowlExitReqCb)pWDICtx->pfncRspCB;

   /*                                                                         
                                         
                                                                            */
   halStatus = *((eHalStatus*)pEventData->pEventData);
   wdiStatus   =   WDI_HAL_2_WDI_STATUS(halStatus);

   /*           */
   wdiWowlExitCb( wdiStatus, pWDICtx->pRspCBUserData);

   return WDI_STATUS_SUCCESS;
}/*                      */

/* 
                                                            
                                                              
                 

                                                         
                                                                    

     
                                    
*/
WDI_Status
WDI_ProcessConfigureAppsCpuWakeupStateRsp
(
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
)
{
   WDI_Status           wdiStatus;
   eHalStatus           halStatus;
   WDI_ConfigureAppsCpuWakeupStateCb   wdiConfigureAppsCpuWakeupStateCb;
   /*                                                                        */

   /*                                                                         
                 
                                                                            */
   if (( NULL == pWDICtx ) || ( NULL == pEventData ) ||
       ( NULL == pEventData->pEventData))
   {
      WPAL_TRACE( eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_WARN,
                  "%s: Invalid parameters", __FUNCTION__);
      WDI_ASSERT(0);
      return WDI_STATUS_E_FAILURE;
   }

   wdiConfigureAppsCpuWakeupStateCb = (WDI_ConfigureAppsCpuWakeupStateCb)pWDICtx->pfncRspCB;

   /*                                                                         
                                         
                                                                            */
   halStatus = *((eHalStatus*)pEventData->pEventData);
   wdiStatus   =   WDI_HAL_2_WDI_STATUS(halStatus);

   /*           */
   wdiConfigureAppsCpuWakeupStateCb( wdiStatus, pWDICtx->pRspCBUserData);

   return WDI_STATUS_SUCCESS;
}/*                                         */


/* 
                                                  
                                                                               

                                                         
                                                                    

     
                                    
*/
WDI_Status
WDI_ProcessNvDownloadRsp
(
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
)
{

  WDI_NvDownloadRspCb    wdiNvDownloadRspCb;
  tHalNvImgDownloadRspParams halNvDownloadRsp;
  WDI_NvDownloadRspInfoType wdiNvDownloadRsp;

  /*                                                                         
               
                                                                           */
  if (( NULL == pWDICtx ) || ( NULL == pEventData ) ||
    ( NULL == pEventData->pEventData))
  {
    WPAL_TRACE( eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_WARN,
                "%s: Invalid parameters", __FUNCTION__);
    WDI_ASSERT(0);
    return WDI_STATUS_E_FAILURE;
  }

  /*                                                                         
                                        
                                                                            */
  wpalMemoryCopy( &halNvDownloadRsp,
                  pEventData->pEventData,
                  sizeof(halNvDownloadRsp));

  wdiNvDownloadRsp.wdiStatus = WDI_HAL_2_WDI_STATUS(halNvDownloadRsp.status);

  if((wdiNvDownloadRsp.wdiStatus == WDI_STATUS_SUCCESS) &&
    (pWDICtx->wdiNvBlobInfo.usCurrentFragment !=
         pWDICtx->wdiNvBlobInfo.usTotalFragment ))
  {
    WDI_NvDownloadReq(&pWDICtx->wdiCachedNvDownloadReq,
       (WDI_NvDownloadRspCb)pWDICtx->pfncRspCB, pWDICtx->pRspCBUserData);
  }
  else
  {
    /*                                                                   */
    pWDICtx->wdiNvBlobInfo.usTotalFragment = 0;
    pWDICtx->wdiNvBlobInfo.usFragmentSize = 0;
    pWDICtx->wdiNvBlobInfo.usCurrentFragment = 0;
    /*                                             */
    wdiNvDownloadRspCb = (WDI_NvDownloadRspCb)pWDICtx->pfncRspCB;
    wdiNvDownloadRspCb( &wdiNvDownloadRsp, pWDICtx->pRspCBUserData);
  }

  return WDI_STATUS_SUCCESS;
}
#ifdef WLAN_FEATURE_VOWIFI_11R
/* 
                                                              
                                                

                                                         
                                                                    

     
                                    
*/
WDI_Status
WDI_ProcessAggrAddTSpecRsp
(
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
)
{
  WDI_Status            wdiStatus;
  tAggrAddTsRspParams   aggrAddTsRsp;
  WDI_AggrAddTsRspCb    wdiAggrAddTsRspCb;
  /*                                                                        */

  /*                                                                         
                
                                                                           */
  if (( NULL == pWDICtx ) || ( NULL == pEventData ) ||
      ( NULL == pEventData->pEventData))
  {
     WPAL_TRACE( eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_WARN,
                 "%s: Invalid parameters", __FUNCTION__);
     WDI_ASSERT(0);
     return WDI_STATUS_E_FAILURE;
  }

  wdiAggrAddTsRspCb = (WDI_AddTsRspCb)pWDICtx->pfncRspCB;

  /*                                                                         
                                        
                                                                           */
  wpalMemoryCopy( &aggrAddTsRsp,
                  pEventData->pEventData,
                  sizeof(aggrAddTsRsp));

  /*                                                     */
  wdiStatus   =   WDI_HAL_2_WDI_STATUS(aggrAddTsRsp.status0);

  /*           */
  wdiAggrAddTsRspCb( wdiStatus, pWDICtx->pRspCBUserData);

  return WDI_STATUS_SUCCESS;
}/*                      */
#endif /*                         */

/* 
                                                        
                                                         

                                                         
                                                                    

     
                                    
*/
WDI_Status
WDI_ProcessHostResumeRsp
(
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
)
{
  WDI_SuspendResumeRspParamsType     wdiResumeRspParams;
  WDI_HostResumeEventRspCb           wdiHostResumeRspCb;
  tHalHostResumeRspParams            hostResumeRspMsg;
  /*                                                                        */

  /*                                                                         
                
                                                                             */
  if (( NULL == pWDICtx ) || ( NULL == pEventData ) ||
      ( NULL == pEventData->pEventData))
  {
    WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_WARN,
                "%s: Invalid parameters", __FUNCTION__);
    WDI_ASSERT(0);
    return WDI_STATUS_E_FAILURE;
  }

  wdiHostResumeRspCb = (WDI_HostResumeEventRspCb)pWDICtx->pfncRspCB;

  /*                                                                         
                                        
                                                                             */

  wpalMemoryCopy( &hostResumeRspMsg,
      (wpt_uint8*)pEventData->pEventData,
      sizeof(hostResumeRspMsg));

  wdiResumeRspParams.wdiStatus   =
    WDI_HAL_2_WDI_STATUS(hostResumeRspMsg.status);

  /*           */
  wdiHostResumeRspCb(&wdiResumeRspParams, (void*) pWDICtx->pRspCBUserData);

  return WDI_STATUS_SUCCESS;
}

/* 
                                                               
                                                

                                                         
                                                                    

     
                                    
*/
WDI_Status
WDI_ProcessSetTxPerTrackingRsp
(
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
)
{
  WDI_Status           wdiStatus;
  eHalStatus           halStatus;
  WDI_SetTxPerTrackingRspCb   pwdiSetTxPerTrackingRspCb;
  /*                                                                        */

  /*                                                                         
                
                                                                           */
  if (( NULL == pWDICtx ) || ( NULL == pEventData ) ||
      ( NULL == pEventData->pEventData))
  {
     WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_WARN,
                 "%s: Invalid parameters", __FUNCTION__);
     WDI_ASSERT(0);
     return WDI_STATUS_E_FAILURE;
  }

  pwdiSetTxPerTrackingRspCb = (WDI_SetTxPerTrackingRspCb)pWDICtx->pfncRspCB;

  /*                                                                         
                                        
                                                                           */
  halStatus = *((eHalStatus*)pEventData->pEventData);
  wdiStatus   =   WDI_HAL_2_WDI_STATUS(halStatus);

  /*           */
  pwdiSetTxPerTrackingRspCb( wdiStatus, pWDICtx->pRspCBUserData);

  return WDI_STATUS_SUCCESS;
}/*                              */

/*                                                                          
                                            
                                                                           */
/* 
                                                            
                                                              
                 

                                                         
                                                                    

     
                                    
*/
WDI_Status
WDI_ProcessLowRSSIInd
(
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
)
{
  WDI_LowLevelIndType  wdiInd;
  tHalRSSINotificationIndMsg   halRSSINotificationIndMsg;
  /*                                                                        */

  /*                                                                         
                
                                                                           */
  if (( NULL == pWDICtx ) || ( NULL == pEventData ) ||
      ( NULL == pEventData->pEventData))
  {
     WPAL_TRACE( eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_WARN,
                 "%s: Invalid parameters", __FUNCTION__);
     WDI_ASSERT(0);
     return WDI_STATUS_E_FAILURE;
  }

  /*                                                                         
                                          
                                                                           */
  wpalMemoryCopy( (void *)&halRSSINotificationIndMsg.rssiNotificationParams,
                  pEventData->pEventData,
                  sizeof(tHalRSSINotification));

  /*                                 */
  wdiInd.wdiIndicationType = WDI_RSSI_NOTIFICATION_IND;
  wdiInd.wdiIndicationData.wdiLowRSSIInfo.bRssiThres1PosCross =
     halRSSINotificationIndMsg.rssiNotificationParams.bRssiThres1PosCross;
  wdiInd.wdiIndicationData.wdiLowRSSIInfo.bRssiThres1NegCross =
     halRSSINotificationIndMsg.rssiNotificationParams.bRssiThres1NegCross;
  wdiInd.wdiIndicationData.wdiLowRSSIInfo.bRssiThres2PosCross =
     halRSSINotificationIndMsg.rssiNotificationParams.bRssiThres2PosCross;
  wdiInd.wdiIndicationData.wdiLowRSSIInfo.bRssiThres2NegCross =
     halRSSINotificationIndMsg.rssiNotificationParams.bRssiThres2NegCross;
  wdiInd.wdiIndicationData.wdiLowRSSIInfo.bRssiThres3PosCross =
     halRSSINotificationIndMsg.rssiNotificationParams.bRssiThres3PosCross;
  wdiInd.wdiIndicationData.wdiLowRSSIInfo.bRssiThres3NegCross =
     halRSSINotificationIndMsg.rssiNotificationParams.bRssiThres3NegCross;

  /*           */
  pWDICtx->wdiLowLevelIndCB( &wdiInd, pWDICtx->pIndUserData );

  return WDI_STATUS_SUCCESS;
}/*                     */


/* 
                                                              
                                                             
                     

                                                         
                                                                    

     
                                    
*/
WDI_Status
WDI_ProcessMissedBeaconInd
(
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
)
{
  WDI_Status           wdiStatus;
  eHalStatus           halStatus;
  WDI_LowLevelIndType  wdiInd;
  /*                                                                        */

  /*                                                                         
                
                                                                           */
  if (( NULL == pWDICtx ) || ( NULL == pEventData ) ||
      ( NULL == pEventData->pEventData))
  {
     WPAL_TRACE( eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_WARN,
                 "%s: Invalid parameters", __FUNCTION__);
     WDI_ASSERT(0);
     return WDI_STATUS_E_FAILURE;
  }

  /*                                                                         
                                          
                                                                           */
  /*                                                               */
  halStatus = *((eHalStatus*)pEventData->pEventData);
  wdiStatus   =   WDI_HAL_2_WDI_STATUS(halStatus);

  /*                                 */
  wdiInd.wdiIndicationType = WDI_MISSED_BEACON_IND;

  /*           */
  pWDICtx->wdiLowLevelIndCB( &wdiInd, pWDICtx->pIndUserData );

  return WDI_STATUS_SUCCESS;
}/*                          */


/* 
                                                               
                                                             
                     

                                                         
                                                                    

     
                                    
*/
WDI_Status
WDI_ProcessUnkAddrFrameInd
(
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
)
{
  WDI_Status           wdiStatus;
  eHalStatus           halStatus;
  WDI_LowLevelIndType  wdiInd;
  /*                                                                        */

  /*                                                                         
                
                                                                           */
  if (( NULL == pWDICtx ) || ( NULL == pEventData ) ||
      ( NULL == pEventData->pEventData))
  {
     WPAL_TRACE( eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_WARN,
                 "%s: Invalid parameters", __FUNCTION__);
     WDI_ASSERT(0);
     return WDI_STATUS_E_FAILURE;
  }

  /*                                                                         
                                          
                                                                           */
  /*                                                               */
  halStatus = *((eHalStatus*)pEventData->pEventData);
  wdiStatus   =   WDI_HAL_2_WDI_STATUS(halStatus);

  /*                                 */
  wdiInd.wdiIndicationType = WDI_UNKNOWN_ADDR2_FRAME_RX_IND;
  /*                                   
                                               */

  /*           */
  pWDICtx->wdiLowLevelIndCB( &wdiInd, pWDICtx->pIndUserData );

  return WDI_STATUS_SUCCESS;
}/*                          */


/* 
                                                               
                                                              
                 

                                                         
                                                                    

     
                                    
*/
WDI_Status
WDI_ProcessMicFailureInd
(
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
)
{
  WDI_LowLevelIndType  wdiInd;
  tpSirMicFailureInd   pHalMicFailureInd;

  /*                                                                        */

  /*                                                                         
                
                                                                           */
  if (( NULL == pWDICtx ) || ( NULL == pEventData ) ||
      ( NULL == pEventData->pEventData))
  {
     WPAL_TRACE( eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_WARN,
                 "%s: Invalid parameters", __FUNCTION__);
     WDI_ASSERT(0);
     return WDI_STATUS_E_FAILURE;
  }

  pHalMicFailureInd = (tpSirMicFailureInd)pEventData->pEventData;
  /*                                                                         
                                          
                                                                           */

  /*                                 */
  wdiInd.wdiIndicationType = WDI_MIC_FAILURE_IND;
  wpalMemoryCopy(wdiInd.wdiIndicationData.wdiMICFailureInfo.bssId,
                 pHalMicFailureInd->bssId, WDI_MAC_ADDR_LEN);
  wpalMemoryCopy(wdiInd.wdiIndicationData.wdiMICFailureInfo.macSrcAddr,
                 pHalMicFailureInd->info.srcMacAddr, WDI_MAC_ADDR_LEN);
  wpalMemoryCopy(wdiInd.wdiIndicationData.wdiMICFailureInfo.macTaAddr,
                 pHalMicFailureInd->info.taMacAddr, WDI_MAC_ADDR_LEN);
  wpalMemoryCopy(wdiInd.wdiIndicationData.wdiMICFailureInfo.macDstAddr,
                 pHalMicFailureInd->info.dstMacAddr, WDI_MAC_ADDR_LEN);
  wdiInd.wdiIndicationData.wdiMICFailureInfo.ucMulticast =
                 pHalMicFailureInd->info.multicast;
  wdiInd.wdiIndicationData.wdiMICFailureInfo.ucIV1 =
                 pHalMicFailureInd->info.IV1;
  wdiInd.wdiIndicationData.wdiMICFailureInfo.keyId=
                 pHalMicFailureInd->info.keyId;
  wpalMemoryCopy(wdiInd.wdiIndicationData.wdiMICFailureInfo.TSC,
                 pHalMicFailureInd->info.TSC,WDI_CIPHER_SEQ_CTR_SIZE);
  wpalMemoryCopy(wdiInd.wdiIndicationData.wdiMICFailureInfo.macRxAddr,
                 pHalMicFailureInd->info.rxMacAddr, WDI_MAC_ADDR_LEN);
  /*           */
  pWDICtx->wdiLowLevelIndCB( &wdiInd, pWDICtx->pIndUserData );

  return WDI_STATUS_SUCCESS;
}/*                        */


/* 
                                                              
                                                             
                     

                                                         
                                                                    

     
                                    
*/
WDI_Status
WDI_ProcessFatalErrorInd
(
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
)
{
  WDI_Status           wdiStatus;
  eHalStatus           halStatus;
  WDI_LowLevelIndType  wdiInd;
  /*                                                                        */

  /*                                                                         
                
                                                                           */
  if (( NULL == pWDICtx ) || ( NULL == pEventData ) ||
      ( NULL == pEventData->pEventData))
  {
     WPAL_TRACE( eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_WARN,
                 "%s: Invalid parameters", __FUNCTION__);
     WDI_ASSERT(0);
     return WDI_STATUS_E_FAILURE;
  }

  /*                                                                         
                                          
                                                                           */

  /*                                                               */
  halStatus = *((eHalStatus*)pEventData->pEventData);
  wdiStatus   =   WDI_HAL_2_WDI_STATUS(halStatus);

  WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_ERROR,
              "Fatal failure received from device %d ", halStatus );

  /*                                 */
  wdiInd.wdiIndicationType             = WDI_FATAL_ERROR_IND;
  wdiInd.wdiIndicationData.usErrorCode = WDI_ERR_DEV_INTERNAL_FAILURE;

  /*           */
  pWDICtx->wdiLowLevelIndCB( &wdiInd, pWDICtx->pIndUserData );

  return WDI_STATUS_SUCCESS;
}/*                        */

/* 
                                                           
                                                             
                     

                                                         
                                                                    

     
                                    
*/
WDI_Status
WDI_ProcessDelSTAInd
(
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
)
{
  tDeleteStaContextParams    halDelSTACtx;
  WDI_LowLevelIndType        wdiInd;
  /*                                                                        */

  /*                                                                         
                
                                                                           */
  if (( NULL == pWDICtx ) || ( NULL == pEventData ) ||
      ( NULL == pEventData->pEventData))
  {
     WPAL_TRACE( eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_WARN,
                 "%s: Invalid parameters", __FUNCTION__);
     WDI_ASSERT(0);
     return WDI_STATUS_E_FAILURE;
  }

  /*                                                                         
                                          
                                                                           */

  /*                                                       */
  wpalMemoryCopy( &halDelSTACtx,
                  pEventData->pEventData,
                  sizeof(halDelSTACtx));

  /*                                 */
  wdiInd.wdiIndicationType             = WDI_DEL_STA_IND;

  wpalMemoryCopy(wdiInd.wdiIndicationData.wdiDeleteSTAIndType.macADDR2,
                 halDelSTACtx.addr2, WDI_MAC_ADDR_LEN);
  wpalMemoryCopy(wdiInd.wdiIndicationData.wdiDeleteSTAIndType.macBSSID,
                 halDelSTACtx.bssId, WDI_MAC_ADDR_LEN);

  wdiInd.wdiIndicationData.wdiDeleteSTAIndType.usAssocId =
    halDelSTACtx.assocId;
  wdiInd.wdiIndicationData.wdiDeleteSTAIndType.ucSTAIdx  =
    halDelSTACtx.staId;
  wdiInd.wdiIndicationData.wdiDeleteSTAIndType.wptReasonCode =
    halDelSTACtx.reasonCode;

  /*           */
  pWDICtx->wdiLowLevelIndCB( &wdiInd, pWDICtx->pIndUserData );

  return WDI_STATUS_SUCCESS;
}/*                    */

/* 
                                                     
                                                             
                     

                                                         
                                                                    

     
                                    
*/
WDI_Status
WDI_ProcessCoexInd
(
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
)
{
  WDI_LowLevelIndType  wdiInd;
  tCoexIndMsg          halCoexIndMsg;
  wpt_uint32           index;
  /*                                                                        */

  /*                                                                         
                
                                                                           */
  if (( NULL == pWDICtx ) || ( NULL == pEventData ) ||
      ( NULL == pEventData->pEventData ))
  {
     WPAL_TRACE( eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_WARN,
                 "%s: Invalid parameters", __FUNCTION__);
     WDI_ASSERT( 0 );
     return WDI_STATUS_E_FAILURE;
  }

  /*                                                                         
                                          
                                                                           */
  wpalMemoryCopy( &halCoexIndMsg.coexIndParams,
                  pEventData->pEventData,
                  sizeof(halCoexIndMsg.coexIndParams) );

  /*                                 */
  wdiInd.wdiIndicationType = WDI_COEX_IND;
  wdiInd.wdiIndicationData.wdiCoexInfo.coexIndType = halCoexIndMsg.coexIndParams.coexIndType;
  for (index = 0; index < WDI_COEX_IND_DATA_SIZE; index++)
  {
    wdiInd.wdiIndicationData.wdiCoexInfo.coexIndData[index] = halCoexIndMsg.coexIndParams.coexIndData[index];
  }

  //      
  WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_INFO,
              "[COEX WDI] Coex Ind Type (%x) data (%x %x %x %x)",
              wdiInd.wdiIndicationData.wdiCoexInfo.coexIndType,
              wdiInd.wdiIndicationData.wdiCoexInfo.coexIndData[0],
              wdiInd.wdiIndicationData.wdiCoexInfo.coexIndData[1],
              wdiInd.wdiIndicationData.wdiCoexInfo.coexIndData[2],
              wdiInd.wdiIndicationData.wdiCoexInfo.coexIndData[3] );

  /*           */
  pWDICtx->wdiLowLevelIndCB( &wdiInd, pWDICtx->pIndUserData );

  return WDI_STATUS_SUCCESS;
}/*                  */

/* 
                                                            
                                                             
                     

                                                         
                                                                    

     
                                    
*/
WDI_Status
WDI_ProcessTxCompleteInd
(
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
)
{
  WDI_LowLevelIndType  wdiInd;
  tTxComplIndMsg       halTxComplIndMsg;
  /*                                                                        */

  /*                                                                         
                
                                                                           */
  if (( NULL == pWDICtx ) || ( NULL == pEventData ) ||
      ( NULL == pEventData->pEventData ))
  {
     WPAL_TRACE( eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_WARN,
                 "%s: Invalid parameters", __FUNCTION__);
     WDI_ASSERT( 0 );
     return WDI_STATUS_E_FAILURE;
  }

  /*                                                                         
                                          
                                                                           */
  wpalMemoryCopy( &halTxComplIndMsg.txComplParams,
                  pEventData->pEventData,
                  sizeof(halTxComplIndMsg.txComplParams) );

  /*                                 */
  wdiInd.wdiIndicationType = WDI_TX_COMPLETE_IND;
  wdiInd.wdiIndicationData.tx_complete_status
                               = halTxComplIndMsg.txComplParams.status;

  /*           */
  pWDICtx->wdiLowLevelIndCB( &wdiInd, pWDICtx->pIndUserData );

  return WDI_STATUS_SUCCESS;
}/*                        */

#ifdef WLAN_FEATURE_P2P
/* 
                                                         
                                                             
                     

                                                         
                                                                    

     
                                    
*/
WDI_Status
WDI_ProcessP2pNoaAttrInd
(
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
)
{
  WDI_LowLevelIndType  wdiInd;
  tNoaAttrIndMsg       halNoaAttrIndMsg;
  /*                                                                        */

  /*                                                                         
                
                                                                           */
  if (( NULL == pWDICtx ) || ( NULL == pEventData ) ||
      ( NULL == pEventData->pEventData ))
  {
     WPAL_TRACE( eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_WARN,
                 "%s: Invalid parameters", __FUNCTION__);
     WDI_ASSERT( 0 );
     return WDI_STATUS_E_FAILURE;
  }

  /*                                                                         
                                          
                                                                           */
  wpalMemoryCopy( &halNoaAttrIndMsg.noaAttrIndParams,
                  pEventData->pEventData,
                  sizeof(halNoaAttrIndMsg.noaAttrIndParams) );

  /*                                 */
  wdiInd.wdiIndicationType = WDI_P2P_NOA_ATTR_IND;

  wdiInd.wdiIndicationData.wdiP2pNoaAttrInfo.status
                          = halNoaAttrIndMsg.noaAttrIndParams.status;

  wdiInd.wdiIndicationData.wdiP2pNoaAttrInfo.ucIndex
                          = halNoaAttrIndMsg.noaAttrIndParams.index;
  wdiInd.wdiIndicationData.wdiP2pNoaAttrInfo.ucOppPsFlag
                          = halNoaAttrIndMsg.noaAttrIndParams.oppPsFlag;
  wdiInd.wdiIndicationData.wdiP2pNoaAttrInfo.usCtWin
                          = halNoaAttrIndMsg.noaAttrIndParams.ctWin;

  wdiInd.wdiIndicationData.wdiP2pNoaAttrInfo.usNoa1IntervalCnt
                          = halNoaAttrIndMsg.noaAttrIndParams.uNoa1IntervalCnt;
  wdiInd.wdiIndicationData.wdiP2pNoaAttrInfo.uslNoa1Duration
                          = halNoaAttrIndMsg.noaAttrIndParams.uNoa1Duration;
  wdiInd.wdiIndicationData.wdiP2pNoaAttrInfo.uslNoa1Interval
                             = halNoaAttrIndMsg.noaAttrIndParams.uNoa1Interval;
  wdiInd.wdiIndicationData.wdiP2pNoaAttrInfo.uslNoa1StartTime
                          = halNoaAttrIndMsg.noaAttrIndParams.uNoa1StartTime;

  wdiInd.wdiIndicationData.wdiP2pNoaAttrInfo.usNoa2IntervalCnt
                          = halNoaAttrIndMsg.noaAttrIndParams.uNoa2IntervalCnt;
  wdiInd.wdiIndicationData.wdiP2pNoaAttrInfo.uslNoa2Duration
                          = halNoaAttrIndMsg.noaAttrIndParams.uNoa2Duration;
  wdiInd.wdiIndicationData.wdiP2pNoaAttrInfo.uslNoa2Interval
                          = halNoaAttrIndMsg.noaAttrIndParams.uNoa2Interval;
  wdiInd.wdiIndicationData.wdiP2pNoaAttrInfo.uslNoa2StartTime
                          = halNoaAttrIndMsg.noaAttrIndParams.uNoa2StartTime;

  /*           */
  pWDICtx->wdiLowLevelIndCB( &wdiInd, pWDICtx->pIndUserData );

  return WDI_STATUS_SUCCESS;
}/*                     */
#endif

/* 
                                                           
                                                             
                     

                                                         
                                                                    

     
                                    
*/
WDI_Status
WDI_ProcessTxPerHitInd
(
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
)
{
  WDI_LowLevelIndType  wdiInd;
  /*                                                                        */

  /*                                                                         
                                          
                                                                           */
  /*                                 */
  wdiInd.wdiIndicationType = WDI_TX_PER_HIT_IND;

  /*           */
  pWDICtx->wdiLowLevelIndCB( &wdiInd, pWDICtx->pIndUserData );

  return WDI_STATUS_SUCCESS;
}/*                      */

#ifdef ANI_MANF_DIAG
/* 
                                
                                                

                                                         
                                                                    

     
                                    
*/
WDI_Status
WDI_ProcessFTMCommandReq
(
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
)
{
  WDI_FTMCommandReqType  *ftmCommandReq = NULL;
  wpt_uint8              *ftmCommandBuffer = NULL;
  wpt_uint16              dataOffset;
  wpt_uint16              bufferSize;
  /*                                                                         
                
                                                                           */
  if (( NULL == pWDICtx ) || ( NULL == pEventData ) ||
      ( NULL == pEventData->pEventData))

  {
     WPAL_TRACE( eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_WARN,
                 "%s: Invalid parameters", __FUNCTION__);
     WDI_ASSERT(0);
     return WDI_STATUS_E_FAILURE;
  }

  ftmCommandReq = (WDI_FTMCommandReqType *)pEventData->pEventData;

  /*                */
  WDI_GetMessageBuffer(pWDICtx,
                       WDI_FTM_CMD_REQ,
                       ftmCommandReq->bodyLength,
                       &ftmCommandBuffer,
                       &dataOffset,
                       &bufferSize);

  wpalMemoryCopy(ftmCommandBuffer + dataOffset,
                 ftmCommandReq->FTMCommandBody,
                 ftmCommandReq->bodyLength);

  /*          */
  return WDI_SendMsg(pWDICtx,
                     ftmCommandBuffer,
                     bufferSize,
                     pEventData->pCBfnc,
                     pEventData->pUserData,
                     WDI_FTM_CMD_RESP);
}

/* 
                                
                                                                      

                                                         
                                                                    

     
                                    
*/
WDI_Status
WDI_ProcessFTMCommandRsp
(
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
)
{
  WDI_FTMCommandRspCb   ftmCMDRspCb;
  tProcessPttRspParams *ftmCMDRspData = NULL;
  /*                                                                        */

  /*                                                                         
                
                                                                           */
  if (( NULL == pWDICtx ) || ( NULL == pEventData ) ||
      ( NULL == pEventData->pEventData))
  {
     WPAL_TRACE( eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_WARN,
                 "%s: Invalid parameters", __FUNCTION__);
     WDI_ASSERT(0);
     return WDI_STATUS_E_FAILURE;
  }

  ftmCMDRspCb = (WDI_FTMCommandRspCb)pWDICtx->pfncRspCB;

  ftmCMDRspData = (tProcessPttRspParams *)pEventData->pEventData;

  wpalMemoryCopy((void *)pWDICtx->ucFTMCommandRspBuffer,
                 (void *)&ftmCMDRspData->pttMsgBuffer,
                 ftmCMDRspData->pttMsgBuffer.msgBodyLength);

  /*           */
  ftmCMDRspCb((void *)pWDICtx->ucFTMCommandRspBuffer, pWDICtx->pRspCBUserData);

  return WDI_STATUS_SUCCESS;
}
#endif /*               */
/* 
                                
                                                     

                                                         
                                                                    

     
                                    
*/
WDI_Status
WDI_ProcessHALDumpCmdReq
(
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
)
{
  WDI_HALDumpCmdReqParamsType*  pwdiHALDumpCmdParams = NULL;
  WDI_HALDumpCmdRspCb           wdiHALDumpCmdRspCb = NULL;
  wpt_uint16               usDataOffset        = 0;
  wpt_uint16               usSendSize          = 0;
  tHalDumpCmdReqMsg        halDumpCmdReqMsg;
  wpt_uint8*               pSendBuffer         = NULL;

  /*                                                                         
                
                                                                           */
  if (( NULL == pEventData ) ||
      ( NULL == pEventData->pEventData) ||
      ( NULL == pEventData->pCBfnc ))
  {
     WPAL_TRACE( eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_WARN,
                 "%s: Invalid parameters", __FUNCTION__);
     WDI_ASSERT(0);
     return WDI_STATUS_E_FAILURE;
  }

  pwdiHALDumpCmdParams = (WDI_HALDumpCmdReqParamsType*)pEventData->pEventData;
  wdiHALDumpCmdRspCb   = (WDI_HALDumpCmdRspCb)pEventData->pCBfnc;

  /*                                                       */
  halDumpCmdReqMsg.dumpCmdReqParams.argument1 =
                pwdiHALDumpCmdParams->wdiHALDumpCmdInfoType.command;
  halDumpCmdReqMsg.dumpCmdReqParams.argument2 =
                pwdiHALDumpCmdParams->wdiHALDumpCmdInfoType.argument1;
  halDumpCmdReqMsg.dumpCmdReqParams.argument3 =
                pwdiHALDumpCmdParams->wdiHALDumpCmdInfoType.argument2;
  halDumpCmdReqMsg.dumpCmdReqParams.argument4 =
                pwdiHALDumpCmdParams->wdiHALDumpCmdInfoType.argument3;
  halDumpCmdReqMsg.dumpCmdReqParams.argument5 =
                pwdiHALDumpCmdParams->wdiHALDumpCmdInfoType.argument4;

  /*                                                                       
                      
                                                                         */
  if (( WDI_STATUS_SUCCESS != WDI_GetMessageBuffer( pWDICtx, WDI_HAL_DUMP_CMD_REQ,
                        sizeof(halDumpCmdReqMsg.dumpCmdReqParams),
                        &pSendBuffer, &usDataOffset, &usSendSize))||
      ( usSendSize <
            (usDataOffset + sizeof(halDumpCmdReqMsg.dumpCmdReqParams) )))
  {
     WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_WARN,
              "Unable to get send buffer in HAL Dump Command req %x %x %x",
                pEventData, pwdiHALDumpCmdParams, wdiHALDumpCmdRspCb);
     WDI_ASSERT(0);
     return WDI_STATUS_E_FAILURE;
  }

  wpalMemoryCopy( pSendBuffer+usDataOffset,
                  &halDumpCmdReqMsg.dumpCmdReqParams,
                  sizeof(halDumpCmdReqMsg.dumpCmdReqParams));

  pWDICtx->wdiReqStatusCB     = pwdiHALDumpCmdParams->wdiReqStatusCB;
  pWDICtx->pReqStatusUserData = pwdiHALDumpCmdParams->pUserData;

  /*                                                                         
                             
                                                                           */
  return  WDI_SendMsg( pWDICtx, pSendBuffer, usSendSize,
                        wdiHALDumpCmdRspCb, pEventData->pUserData,
                        WDI_HAL_DUMP_CMD_RESP);
}

/* 
                                
                                                                       

                                                         
                                                                    

     
                                    
*/
WDI_Status
WDI_ProcessHALDumpCmdRsp
(
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
)
{
  WDI_HALDumpCmdRspCb     wdiHALDumpCmdRspCb;
  tpHalDumpCmdRspParams   halDumpCmdRspParams;
  WDI_HALDumpCmdRspParamsType wdiHALDumpCmdRsp;

  /*                                                                         
                
                                                                           */
  if (( NULL == pWDICtx ) || ( NULL == pEventData ) ||
      ( NULL == pEventData->pEventData))
  {
     WPAL_TRACE( eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_WARN,
                 "%s: Invalid parameters", __FUNCTION__);
     WDI_ASSERT(0);
     return WDI_STATUS_E_FAILURE;
  }

  wdiHALDumpCmdRspCb = (WDI_HALDumpCmdRspCb)pWDICtx->pfncRspCB;

  /*                                      */
  wdiHALDumpCmdRsp.usBufferLen = 0;
  wdiHALDumpCmdRsp.pBuffer = NULL;

  halDumpCmdRspParams = (tHalDumpCmdRspParams *)pEventData->pEventData;

  wdiHALDumpCmdRsp.wdiStatus   =
              WDI_HAL_2_WDI_STATUS(halDumpCmdRspParams->status);

  if (( wdiHALDumpCmdRsp.wdiStatus  ==  WDI_STATUS_SUCCESS) &&
      (halDumpCmdRspParams->rspLength != 0))
  {
      /*                        */
      wdiHALDumpCmdRsp.usBufferLen = halDumpCmdRspParams->rspLength;
      wdiHALDumpCmdRsp.pBuffer = wpalMemoryAllocate(halDumpCmdRspParams->rspLength);
      wpalMemoryCopy( &halDumpCmdRspParams->rspBuffer,
                  wdiHALDumpCmdRsp.pBuffer,
                  sizeof(wdiHALDumpCmdRsp.usBufferLen));
  }

  /*           */
  wdiHALDumpCmdRspCb(&wdiHALDumpCmdRsp, pWDICtx->pRspCBUserData);

  if(wdiHALDumpCmdRsp.pBuffer != NULL)
  {
    /*                           */
    wpalMemoryFree(wdiHALDumpCmdRsp.pBuffer);
  }
  return WDI_STATUS_SUCCESS;
}

/*                                                                          
                                                 

                                                                           
                             
                                                                          */
/* 
                                                            
                                                         
                   

       

                                                             
                                              
                                                   

                         

             
*/
void
WDI_NotifyMsgCTSCB
(
  WCTS_HandleType        wctsHandle,
  WCTS_NotifyEventType   wctsEvent,
  void*                  wctsNotifyCBData
)
{
  WDI_ControlBlockType*  pWDICtx = (WDI_ControlBlockType*)wctsNotifyCBData;
  /*                                                                      */

  if (NULL == pWDICtx )
  {
    WPAL_TRACE( eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_WARN,
                "%s: Invalid parameters", __FUNCTION__);
    WDI_ASSERT(0);
    return;
  }

  if (WDI_CONTROL_BLOCK_MAGIC != pWDICtx->magic)
  {
    /*                                          */
    WPAL_TRACE( eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_WARN,
                "%s: Invalid control block", __FUNCTION__);
    return;
  }

  if ( WCTS_EVENT_OPEN == wctsEvent )
  {
    /*                                                                  
               */
    wpalMutexAcquire(&pWDICtx->wptMutex);
    pWDICtx->bCTOpened   = eWLAN_PAL_TRUE;

    /*                                                                   
                                                    */
    WDI_DequeuePendingReq(pWDICtx);
    wpalMutexRelease(&pWDICtx->wptMutex);
  }
  else if  ( WCTS_EVENT_CLOSE == wctsEvent )
  {
    /*                                                                  
               */
    wpalMutexAcquire(&pWDICtx->wptMutex);
    pWDICtx->bCTOpened   = eWLAN_PAL_FALSE;

    /*                                                         */
    WDI_ClearPendingRequests(pWDICtx);
    wpalMutexRelease(&pWDICtx->wptMutex);

    /*                                          */
    wpalEventSet(&pWDICtx->wctsActionEvent);
  }

}/*                  */


/* 
                                                            
                                                               
               

       

                                                        
                           
                                  
                                                  

                         

             
*/
void
WDI_RXMsgCTSCB
(
  WCTS_HandleType       wctsHandle,
  void*                 pMsg,
  wpt_uint32            uLen,
  void*                 wctsRxMsgCBData
)
{
  tHalMsgHeader          *pHalMsgHeader;
  WDI_EventInfoType      wdiEventData;
  WDI_ControlBlockType*  pWDICtx = (WDI_ControlBlockType*)wctsRxMsgCBData;
  /*                                                                     */

  /*                                                                        
                
                                                                          */
  if ((NULL == pWDICtx ) || ( NULL == pMsg ) ||
      ( uLen < sizeof(tHalMsgHeader)))
  {
    WPAL_TRACE( eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_WARN,
                "%s: Invalid parameters", __FUNCTION__);
    WDI_ASSERT(0);
    return;
  }

  if (WDI_CONTROL_BLOCK_MAGIC != pWDICtx->magic)
  {
    /*                                          */
    WPAL_TRACE( eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_WARN,
                "%s: Invalid control block", __FUNCTION__);
    return;
  }

  /*                                                                         
                                                   
                                */

  pHalMsgHeader = (tHalMsgHeader *)pMsg;

  if ( uLen != pHalMsgHeader->msgLen )
  {
    WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_ERROR,
              "Invalid packet received from HAL - catastrophic failure");
    WDI_DetectedDeviceError( pWDICtx, WDI_ERR_INVALID_RSP_FMT);
    return;
  }

  wdiEventData.wdiResponse = HAL_2_WDI_RSP_TYPE( pHalMsgHeader->msgType );

  /*                                          */
  wdiEventData.pEventData     = (wpt_uint8*)pMsg + sizeof(tHalMsgHeader);
  wdiEventData.uEventDataSize = pHalMsgHeader->msgLen - sizeof(tHalMsgHeader);
  wdiEventData.pCBfnc         = gWDICb.pfncRspCB;
  wdiEventData.pUserData      = gWDICb.pRspCBUserData;


  if ( wdiEventData.wdiResponse ==  pWDICtx->wdiExpectedResponse )
  {
    /*                                            */
    /*                                                                    */
    wpalTimerStop(&pWDICtx->wptResponseTimer);
  }
  /*                                                              */
  else if ( wdiEventData.wdiResponse < WDI_HAL_IND_MIN )
  {
    WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_FATAL,
               "Received response %s (%d) when expecting %s (%d) - catastrophic failure",
               WDI_getRespMsgString(wdiEventData.wdiResponse),
               wdiEventData.wdiResponse,
               WDI_getRespMsgString(pWDICtx->wdiExpectedResponse),
               pWDICtx->wdiExpectedResponse);
    /*                                                             */
    return;
  }

  WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_INFO,
           "Rx smth from HAL: %d", wdiEventData.wdiResponse);

  /*                                        */
  WDI_PostMainEvent(pWDICtx, WDI_RESPONSE_EVENT, &wdiEventData);

}/*              */


/*                                                                        
                                 
                                                                        */

/* 
                                                               
                              

                                              

                                    
*/
WPT_INLINE WDI_Status
WDI_CleanCB
(
  WDI_ControlBlockType*  pWDICtx
)
{
  /*                                                                       */

  /*                           */
  wpalMemoryZero( pWDICtx, sizeof(*pWDICtx));

  pWDICtx->uGlobalState  = WDI_MAX_ST;
  pWDICtx->ucMaxBssids   = WDI_MAX_SUPPORTED_BSS;
  pWDICtx->ucMaxStations = WDI_MAX_SUPPORTED_STAS;

  WDI_ResetAssocSessions( pWDICtx );

  return WDI_STATUS_SUCCESS;
}/*           */


/* 
                                       


                                                         
                                                                    

     
                                    
*/
WPT_INLINE WDI_Status
WDI_ProcessRequest
(
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
)
{
  /*                                                                        */

  /*                                                                    
                                  */

  if (( pEventData->wdiRequest < WDI_MAX_UMAC_IND ) &&
      ( NULL != pfnReqProcTbl[pEventData->wdiRequest] ))
  {
    WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_INFO,
              "Calling request processing function for req %s (%d) %x",
              WDI_getReqMsgString(pEventData->wdiRequest),
              pEventData->wdiRequest, pfnReqProcTbl[pEventData->wdiRequest]);
    return pfnReqProcTbl[pEventData->wdiRequest](pWDICtx, pEventData);
  }
  else
  {
    WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_ERROR,
              "Operation %d is not yet implemented ",
               pEventData->wdiRequest);
    return WDI_STATUS_E_NOT_IMPLEMENT;
  }
}/*                  */


/* 
                                                               
                                                             
                                              

                                                         
                                                        
                                            
                                                    
                                                                
                                           
                                                                
             

     
                                    
*/
WDI_Status
WDI_GetMessageBuffer
(
  WDI_ControlBlockType*  pWDICtx,
  WDI_RequestEnumType    wdiReqType,
  wpt_uint16             usBufferLen,
  wpt_uint8**            pMsgBuffer,
  wpt_uint16*            pusDataOffset,
  wpt_uint16*            pusBufferSize
)
{
  tHalMsgHeader  halMsgHeader;
  /*                                                                        */

  /*                                                                        
         */

  /*                                                                         
                                            
                                                                           */
  *pusBufferSize = sizeof(halMsgHeader) + usBufferLen;
  *pMsgBuffer   = (wpt_uint8*)wpalMemoryAllocate(*pusBufferSize);
  if ( NULL ==  *pMsgBuffer )
  {
     WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_ERROR,
               "Unable to allocate message buffer for req %s (%d)",
               WDI_getReqMsgString(wdiReqType),
               wdiReqType);
     WDI_ASSERT(0);
     return WDI_STATUS_MEM_FAILURE;
  }

  /*                                                                         
                               
                                                                           */
  halMsgHeader.msgType = WDI_2_HAL_REQ_TYPE(wdiReqType);
  /*                 */
#ifdef WLAN_FEATURE_11AC
  if (WDI_getFwWlanFeatCaps(DOT11AC))
	  halMsgHeader.msgVersion = WLAN_HAL_MSG_VERSION1;
  else
#endif
	  halMsgHeader.msgVersion = WLAN_HAL_MSG_VERSION0;

  halMsgHeader.msgLen  = sizeof(halMsgHeader) + usBufferLen;
  *pusDataOffset       = sizeof(halMsgHeader);
  wpalMemoryCopy(*pMsgBuffer, &halMsgHeader, sizeof(halMsgHeader));

  return WDI_STATUS_SUCCESS;
}/*                    */


/* 
                                                               
                                                             
              

                                                         
                                           

                                                          
                                                               
           
                                                         
                 
                                                              
                                              

     
                                    
*/
WDI_Status
WDI_SendMsg
(
  WDI_ControlBlockType*  pWDICtx,
  wpt_uint8*             pSendBuffer,
  wpt_uint32             usSendSize,
  void*                  pRspCb,
  void*                  pUserData,
  WDI_ResponseEnumType   wdiExpectedResponse
)
{
  WDI_Status wdiStatus = WDI_STATUS_SUCCESS;
  /*                                                                       */

  /*                                                                        
                              
                                                                          */
  pWDICtx->pRspCBUserData      = pUserData;
  pWDICtx->pfncRspCB           = pRspCb;
  pWDICtx->wdiExpectedResponse = wdiExpectedResponse;

   /*                                                                       
                                                                     
                               
                                                            
                                                                          */
   if ( 0 != WCTS_SendMessage( pWDICtx->wctsHandle, (void*)pSendBuffer, usSendSize ))
   {
     WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_FATAL,
                "Failed to send message over the bus - catastrophic failure");

     wdiStatus = WDI_STATUS_E_FAILURE;
   }

  /*                                                      */
   if ( NULL != pWDICtx->wdiReqStatusCB )
   {
     /*                                                     */
     WDI_ReqStatusCb callback = pWDICtx->wdiReqStatusCB;
     void *callbackContext = pWDICtx->pReqStatusUserData;
     pWDICtx->wdiReqStatusCB = NULL;
     pWDICtx->pReqStatusUserData = NULL;
     callback(wdiStatus, callbackContext);

     /*                                                          
                                                                   
                                                                       
                                                                   
                              */
     if (wdiStatus == WDI_STATUS_E_FAILURE)
       wdiStatus = WDI_STATUS_PENDING;
   }

  if ( wdiStatus == WDI_STATUS_SUCCESS )
  {
   /*                                      */
   wpalTimerStart(&pWDICtx->wptResponseTimer, WDI_RESPONSE_TIMEOUT);
  }
  else
  {
     /*                                                              */
     WDI_DetectedDeviceError(pWDICtx, WDI_ERR_TRANSPORT_FAILURE);
  }

  return wdiStatus;

}/*           */



/* 
                                                              
                                                               
                 

                                                         
                                           
                                                  

     
                                    
*/
WDI_Status
WDI_SendIndication
(
  WDI_ControlBlockType*  pWDICtx,
  wpt_uint8*             pSendBuffer,
  wpt_uint32             usSendSize
)
{
   wpt_uint32 uStatus ;
   /*                                                                       */

   /*                                                                       
                                           
                                                            
                                                                          */
   uStatus = WCTS_SendMessage( pWDICtx->wctsHandle,
                               (void*)pSendBuffer, usSendSize );

   /*                                                 */
   if ( NULL != pWDICtx->wdiReqStatusCB )
   {
      WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_INFO,
                "Send indication status : %d", uStatus);

      pWDICtx->wdiReqStatusCB( (uStatus != 0 ) ? WDI_STATUS_E_FAILURE:
                                                 WDI_STATUS_SUCCESS,
                               pWDICtx->pReqStatusUserData);
   }

   /*                                                                    
                                       */
   if ( 0 != uStatus)
   {
      WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_FATAL,
                "Failed to send indication over the bus - catastrophic failure");

      WDI_DetectedDeviceError( pWDICtx, WDI_ERR_TRANSPORT_FAILURE);
      return WDI_STATUS_E_FAILURE;
   }

   return WDI_STATUS_SUCCESS;
}/*                  */


/* 
                                                               
                                               

                                                        
                                                               
                                 

     
             
*/
void
WDI_DetectedDeviceError
(
  WDI_ControlBlockType*  pWDICtx,
  wpt_uint16             usErrorCode
)
{
  WDI_LowLevelIndType  wdiInd;
  /*                                                                       */

  WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_ERROR,
            "Device Error detected code: %d - transitioning to stopped state",
            usErrorCode);

  wpalMutexAcquire(&pWDICtx->wptMutex);

  WDI_STATableStop(pWDICtx);

  WDI_ResetAssocSessions(pWDICtx);

  /*                                                                 
                       */
  pWDICtx->ucExpectedStateTransition =  WDI_STOPPED_ST;

  /*                                                                       */
  WDI_STATE_TRANSITION( pWDICtx, WDI_STOPPED_ST);

  WDI_ClearPendingRequests(pWDICtx);

  /*                                                             */

  wpalMutexRelease(&pWDICtx->wptMutex);

  /*                                                                        
                                          
                                                                          */
  if (pWDICtx->wdiLowLevelIndCB)
  {
     wdiInd.wdiIndicationType             = WDI_FATAL_ERROR_IND;
     wdiInd.wdiIndicationData.usErrorCode = usErrorCode;

     pWDICtx->wdiLowLevelIndCB( &wdiInd,  pWDICtx->pIndUserData);
  }
}/*                       */

/* 
                                                                
                                                              
                                                             
                                               
       

                                                            

     
             
*/
void
WDI_ResponseTimerCB
(
  void *pUserData
)
{
  WDI_ControlBlockType*  pWDICtx = (WDI_ControlBlockType*)pUserData;
  /*                                                                      */

  if (NULL == pWDICtx )
  {
    WPAL_TRACE( eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_WARN,
                "%s: Invalid parameters", __FUNCTION__);
    WDI_ASSERT(0);
    return;
  }

  if ( WDI_MAX_RESP != pWDICtx->wdiExpectedResponse )
  {

  WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_ERROR,
            "Timeout occurred while waiting for %s (%d) message from device "
            " - catastrophic failure",
            WDI_getRespMsgString(pWDICtx->wdiExpectedResponse),
            pWDICtx->wdiExpectedResponse);
  /*                                                                      
                                                                       
                                  */
  wpalRivaSubystemRestart();
  }
  else
  {
     WPAL_TRACE( eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_ERROR,
                 "Timeout occurred but not waiting for any response %d",
                 pWDICtx->wdiExpectedResponse);
  }

  return;

}/*                   */


/* 
                                        


                                                         
                                                                    

     
                                    
*/
WPT_INLINE WDI_Status
WDI_ProcessResponse
(
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
)
{
  /*                                                                        */

  /*                                                                 
                                  
                                */
  if (( pEventData->wdiResponse < WDI_MAX_RESP ) &&
      ( NULL != pfnRspProcTbl[pEventData->wdiResponse] ))
  {
    WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_INFO,
              "Calling response processing function for resp %s (%d) %x",
              WDI_getRespMsgString(pEventData->wdiResponse),
              pEventData->wdiResponse, pfnRspProcTbl[pEventData->wdiResponse]);
    return pfnRspProcTbl[pEventData->wdiResponse](pWDICtx, pEventData);
  }
  else
  {
    WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_ERROR,
              "Operation %d is not yet implemented ",
              pEventData->wdiResponse);
    return WDI_STATUS_E_NOT_IMPLEMENT;
  }
}/*                   */


/*                                                                         
                                                  
                                                                         */

/* 
                                                                
                                                       
       

                                               
                                                           
          

     
                                
*/
WDI_Status
WDI_QueuePendingReq
(
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
)
{
  wpt_list_node*      pNode;
  WDI_EventInfoType*  pEventDataQueue = wpalMemoryAllocate(sizeof(*pEventData));
  void*               pEventInfo = NULL;
  /*                                                                        */

  if ( NULL ==  pEventDataQueue )
  {
    WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_ERROR,
              "Cannot allocate memory for queueing");
    WDI_ASSERT(0);
    return WDI_STATUS_MEM_FAILURE;
  }

  pEventDataQueue->pCBfnc          = pEventData->pCBfnc;
  pEventDataQueue->pUserData       = pEventData->pUserData;
  pEventDataQueue->uEventDataSize  = pEventData->uEventDataSize;
  pEventDataQueue->wdiRequest      = pEventData->wdiRequest;
  pEventDataQueue->wdiResponse     = pEventData->wdiResponse;

  if( pEventData->uEventDataSize != 0 && pEventData->pEventData != NULL )
  {
     pEventInfo = wpalMemoryAllocate(pEventData->uEventDataSize);

     if ( NULL ==  pEventInfo )
     {
       WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_ERROR,
                 "Cannot allocate memory for queueing event data info");
       WDI_ASSERT(0);
       wpalMemoryFree(pEventDataQueue);
       return WDI_STATUS_MEM_FAILURE;
     }

     wpalMemoryCopy(pEventInfo, pEventData->pEventData, pEventData->uEventDataSize);

  }
  pEventDataQueue->pEventData = pEventInfo;

  /*                                                                          */
  pNode = (wpt_list_node*)pEventDataQueue;

  wpal_list_insert_back(&(pWDICtx->wptPendingQueue), pNode);

  return WDI_STATUS_SUCCESS;
}/*                   */

/* 
                                                           
                                            
       

                                 

     
                                
*/
void
WDI_PALCtrlMsgCB
(
 wpt_msg *pMsg
)
{
  WDI_EventInfoType*     pEventData = NULL;
  WDI_ControlBlockType*  pWDICtx    = NULL;
  WDI_Status             wdiStatus;
  WDI_ReqStatusCb        pfnReqStatusCB;
  void*                  pUserData;
  /*                                                                        */

  if (( NULL == pMsg )||
      ( NULL == (pEventData = (WDI_EventInfoType*)pMsg->ptr)) ||
      ( NULL == (pWDICtx  = (WDI_ControlBlockType*)pMsg->pContext )))
  {
    WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_ERROR,
              "Invalid message received on serialize ctrl context API");
    WDI_ASSERT(0);
    return;
  }

  /*                                                             
                                                                       
                                                                           
                                         */

  WDI_STATE_TRANSITION( pWDICtx, pMsg->val);

  /*                                                                       
                                                       
                                                        
                                                                         */
  switch ( pEventData->wdiRequest )
  {

  case WDI_STOP_REQ:
      wdiStatus = WDI_PostMainEvent(&gWDICb, WDI_STOP_EVENT, pEventData);
      break;

  case WDI_NV_DOWNLOAD_REQ:
      //                                                                                              
      //                                                                                                                 
      if (pWDICtx->uGlobalState == WDI_STARTED_ST)
      {
        WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_INFO,
                 "%s: WDI_NV_DOWNLOAD_REQ called in WDI_STARTED_ST - send with WDI_REQUEST_EVENT", __FUNCTION__);
        wdiStatus = WDI_PostMainEvent(&gWDICb, WDI_REQUEST_EVENT, pEventData);
      }
      else
      {
        wdiStatus = WDI_PostMainEvent(&gWDICb, WDI_START_EVENT, pEventData);
      }

      break;

  default:
    wdiStatus = WDI_PostMainEvent(&gWDICb, WDI_REQUEST_EVENT, pEventData);
    break;
  }/*                                 */

  if ( WDI_STATUS_SUCCESS != wdiStatus  )
  {
    WDI_ExtractRequestCBFromEvent(pEventData, &pfnReqStatusCB, &pUserData);

    if ( NULL != pfnReqStatusCB )
    {
      /*                */
      pfnReqStatusCB( wdiStatus, pUserData);
    }
  }

  /*                                             */
  if( pEventData != NULL )
  {
     if( pEventData->pEventData != NULL )
     {
        wpalMemoryFree(pEventData->pEventData);
     }
     wpalMemoryFree(pEventData);
  }

  if( pMsg != NULL )
  {
     wpalMemoryFree(pMsg);
  }

}/*                */

/* 
                                                                
                                                       
       

                                               
                                                           
          

     
                                
*/
WDI_Status
WDI_DequeuePendingReq
(
  WDI_ControlBlockType*  pWDICtx
)
{
  wpt_list_node*      pNode      = NULL;
  WDI_EventInfoType*  pEventData;
  wpt_msg*            palMsg;
  /*                                                                        */

  wpal_list_remove_front(&(pWDICtx->wptPendingQueue), &pNode);

  if ( NULL ==  pNode )
  {
    WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_WARN,
              "List is empty - return");
    return WDI_STATUS_SUCCESS;
  }

  /*                                                                          
                                   */
  pEventData = (WDI_EventInfoType*)pNode;

  /*                                          
                                                                          
                    */
  palMsg = wpalMemoryAllocate(sizeof(wpt_msg));

  if ( NULL ==  palMsg )
  {
    WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_ERROR,
               "WDI_DequeuePendingReq: Cannot allocate memory for palMsg.");
    WDI_ASSERT(0);
    return WDI_STATUS_MEM_FAILURE;
  }
  palMsg->pContext = pWDICtx;
  palMsg->callback = WDI_PALCtrlMsgCB;
  palMsg->ptr      = pEventData;

  /*                                                     */
  palMsg->val      = pWDICtx->uGlobalState;

  /*                                                             */
  WDI_STATE_TRANSITION( pWDICtx, WDI_BUSY_ST);

  wpalPostCtrlMsg(pWDICtx->pPALContext, palMsg);

  return WDI_STATUS_PENDING;
}/*                     */


/* 
                                                              
                                                                
                                                            
       

                                               
                                                                
                   

     
                                
*/
WDI_Status
WDI_QueueNewAssocRequest
(
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData,
  wpt_macAddr            macBSSID
)
{
  wpt_uint8 i;
  WDI_BSSSessionType*     pSession = NULL;
  wpt_list_node*          pNode;
  WDI_EventInfoType*      pEventDataQueue;
  void*                   pEventInfo;
  WDI_NextSessionIdType*  pSessionIdElement;
  /*                                                                        */


  /*                                                                        
                                                 
                                                                            */
  for ( i = 0; i < WDI_MAX_BSS_SESSIONS; i++ )
  {
     if ( eWLAN_PAL_FALSE == pWDICtx->aBSSSessions[i].bInUse )
     {
       /*                      */
       pSession = &pWDICtx->aBSSSessions[i];
       break;
     }
  }

  if ( i >=  WDI_MAX_BSS_SESSIONS )
  {
    /*                              */
    return WDI_STATUS_E_FAILURE;
  }

  /*                                                                        
                                                             
                                                                          */
  wpalMemoryCopy(pWDICtx->aBSSSessions[i].macBSSID, macBSSID, WDI_MAC_ADDR_LEN);
  pWDICtx->aBSSSessions[i].bInUse = eWLAN_PAL_TRUE;

  /*                                                                        
                                                      
                                                                          */
  pEventDataQueue = (WDI_EventInfoType*)wpalMemoryAllocate(sizeof(WDI_EventInfoType));
  if ( NULL == pEventDataQueue )
  {
    WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_ERROR,
               "%s: Cannot allocate memory for queue node", __FUNCTION__);
    WDI_ASSERT(0);
    return WDI_STATUS_MEM_FAILURE;
  }

  pSessionIdElement = (WDI_NextSessionIdType*)wpalMemoryAllocate(sizeof(WDI_NextSessionIdType));
  if ( NULL == pSessionIdElement )
  {
    WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_ERROR,
               "%s: Cannot allocate memory for session ID", __FUNCTION__);
    WDI_ASSERT(0);
    wpalMemoryFree(pEventDataQueue);
    return WDI_STATUS_MEM_FAILURE;
  }

  pEventInfo = wpalMemoryAllocate(pEventData->uEventDataSize);
  if ( NULL == pEventInfo )
  {
    WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_ERROR,
               "%s: Cannot allocate memory for event data info", __FUNCTION__);
    WDI_ASSERT(0);
    wpalMemoryFree(pSessionIdElement);
    wpalMemoryFree(pEventDataQueue);
    return WDI_STATUS_MEM_FAILURE;
  }

  pEventDataQueue->pCBfnc          = pEventData->pCBfnc;
  pEventDataQueue->pUserData       = pEventData->pUserData;
  pEventDataQueue->uEventDataSize  = pEventData->uEventDataSize;
  pEventDataQueue->wdiRequest      = pEventData->wdiRequest;
  pEventDataQueue->wdiResponse     = pEventData->wdiResponse;

  wpalMemoryCopy(pEventInfo, pEventData->pEventData, pEventData->uEventDataSize);
  pEventDataQueue->pEventData = pEventInfo;

  /*                                                                          */
  pNode = (wpt_list_node*)pEventDataQueue;

  /*                                          */
  pSession->bAssocReqQueued = eWLAN_PAL_TRUE;

  wpal_list_insert_back(&(pSession->wptPendingQueue), pNode);

  /*                                                                          
                                                                           
                                         */
  pSessionIdElement->ucIndex = i;
  pNode = (wpt_list_node*)pSessionIdElement;

  WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_INFO,
       "Queueing up new assoc session : %d ", pSessionIdElement->ucIndex);
  wpal_list_insert_back(&pWDICtx->wptPendingAssocSessionIdQueue, pNode);

  /*                                                                        
               */
  return WDI_STATUS_PENDING;
}/*                        */

/* 
                                                              
                                                                
                                                            
       

                                               
                                         
                                                          
          

     
                                
*/
WDI_Status
WDI_QueueAssocRequest
(
  WDI_ControlBlockType*  pWDICtx,
  WDI_BSSSessionType*    pSession,
  WDI_EventInfoType*     pEventData
)
{
  wpt_list_node*      pNode;
  WDI_EventInfoType*  pEventDataQueue;
  void*               pEventInfo;
  /*                                                                        */

  /*                                                                        
                  
                                                                            */
  if (( NULL == pSession ) || ( NULL == pWDICtx ))
  {
    WPAL_TRACE( eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_WARN,
                "%s: Invalid parameters", __FUNCTION__);

    return WDI_STATUS_E_FAILURE;
  }

  /*                                                                        
                                                      
                                                                          */
  pEventDataQueue = (WDI_EventInfoType*)wpalMemoryAllocate(sizeof(WDI_EventInfoType));
  if ( NULL ==  pEventDataQueue )
  {
    WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_ERROR,
               "%s: Cannot allocate memory for queueing", __FUNCTION__);
    WDI_ASSERT(0);
    return WDI_STATUS_MEM_FAILURE;
  }

  pEventInfo = wpalMemoryAllocate(pEventData->uEventDataSize);
  if ( NULL ==  pEventInfo )
  {
    WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_ERROR,
               "%s: Cannot allocate memory for queueing event data info",
               __FUNCTION__);
    WDI_ASSERT(0);
    wpalMemoryFree(pEventDataQueue);
    return WDI_STATUS_MEM_FAILURE;
  }

  pEventDataQueue->pCBfnc          = pEventData->pCBfnc;
  pEventDataQueue->pUserData       = pEventData->pUserData;
  pEventDataQueue->uEventDataSize  = pEventData->uEventDataSize;
  pEventDataQueue->wdiRequest      = pEventData->wdiRequest;
  pEventDataQueue->wdiResponse     = pEventData->wdiResponse;
  pEventDataQueue->pEventData      = pEventInfo;

  wpalMemoryCopy(pEventInfo, pEventData->pEventData, pEventData->uEventDataSize);

  /*                                                                          */
  pNode = (wpt_list_node*)pEventDataQueue;

  /*                                          */
  pSession->bAssocReqQueued = eWLAN_PAL_TRUE;

  wpal_list_insert_back(&(pSession->wptPendingQueue), pNode);

  /*                                                                    
                                                              */
  return WDI_STATUS_PENDING;
}/*                     */

/* 
                                                                
                                                  
                                                             
                                                
       

                                               


     
                                
*/
WDI_Status
WDI_DequeueAssocRequest
(
  WDI_ControlBlockType*  pWDICtx
)
{
  wpt_list_node*          pNode = NULL;
  WDI_NextSessionIdType*  pSessionIdElement;
  WDI_BSSSessionType*     pSession;
  /*                                                                        */

  /*                                                                        
                  
                                                                            */
  if ( NULL == pWDICtx )
  {
    WPAL_TRACE( eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_WARN,
                "%s: Invalid parameters", __FUNCTION__);

    return WDI_STATUS_E_FAILURE;
  }

  /*                                                                        
                                                                    
                                                         
                                                                     
              
                                                                       
                                                                        
                                                         
                                                                          */
  wpal_list_remove_front(&(pWDICtx->wptPendingAssocSessionIdQueue), &pNode);

  if ( NULL ==  pNode )
  {
    WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_WARN,
              "List is empty - return");
    return WDI_STATUS_SUCCESS;
  }

  /*                                                                          
                                   */
  pSessionIdElement = (WDI_NextSessionIdType*)pNode;

  WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_INFO,
       "Dequeueing new assoc session : %d ", pSessionIdElement->ucIndex);

  if ( pSessionIdElement->ucIndex < WDI_MAX_BSS_SESSIONS )
  {
      pSession = &pWDICtx->aBSSSessions[pSessionIdElement->ucIndex];

      /*                                                        
                                                                  */
      wpal_list_remove_back(&(pSession->wptPendingQueue), &pNode);
      while ( NULL !=  pNode )
      {
        /*                                          */
        wpal_list_insert_front( &(pWDICtx->wptPendingQueue), &pNode);
        wpal_list_remove_back(&(pSession->wptPendingQueue), &pNode);
      }
      pSession->bAssocReqQueued = eWLAN_PAL_FALSE;
  }
  else
  {
     WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_FATAL,
              "Invalid session id queued up for assoc");
     WPAL_ASSERT(0);
     wpalMemoryFree(pSessionIdElement);
     return WDI_STATUS_E_FAILURE;
  }

  /*                                                                       */
  wpalMemoryFree(pSessionIdElement);
  return WDI_STATUS_SUCCESS;
}/*                       */

/* 
                                                             
                                                            
                                                 
       

                                               

     
                                
*/
WDI_Status
WDI_ClearPendingRequests
(
  WDI_ControlBlockType*  pWDICtx
)
{
  wpt_list_node*      pNode = NULL;
  WDI_EventInfoType*  pEventDataQueue = NULL;
  WDI_ReqStatusCb     pfnReqStatusCB;
  void*               pUserData;
  /*                                                                        */

  wpal_list_remove_front(&(pWDICtx->wptPendingQueue), &pNode);

  /*                                                                        
                                                                        
                                                                           
                                                      
                                                                          */
  while( pNode )
  {
      /*                                                                          
                                   */
    pEventDataQueue = (WDI_EventInfoType*)pNode;

    WDI_ExtractRequestCBFromEvent(pEventDataQueue, &pfnReqStatusCB, &pUserData);
    if ( NULL != pfnReqStatusCB )
    {
      /*                */
      pfnReqStatusCB( WDI_STATUS_E_FAILURE, pUserData);
    }
    /*                                              */
    if ( pEventDataQueue->pEventData != NULL )
    {
      wpalMemoryFree(pEventDataQueue->pEventData);
    }
    wpalMemoryFree(pEventDataQueue);

    if (wpal_list_remove_front(&(pWDICtx->wptPendingQueue), &pNode) !=  eWLAN_PAL_STATUS_SUCCESS)
    {
        break;
    }
  }

  return WDI_STATUS_SUCCESS;
}/*                        */

/* 
                                                                             


                                                       

     
*/
void
WDI_ResetAssocSessions
(
  WDI_ControlBlockType*   pWDICtx
)
{
  wpt_uint8 i;
  /*                                                                       */

  /*                                                                         
                   
                                                                           */
  for ( i = 0; i < WDI_MAX_BSS_SESSIONS; i++ )
  {
    wpalMemoryZero( &pWDICtx->aBSSSessions[i], sizeof(WDI_BSSSessionType) );
    pWDICtx->aBSSSessions[i].wdiAssocState = WDI_ASSOC_INIT_ST;
    pWDICtx->aBSSSessions[i].bcastStaIdx = WDI_STA_INVALID_IDX;
    pWDICtx->aBSSSessions[i].ucBSSIdx = WDI_BSS_INVALID_IDX;
  }
}/*                      */

/* 
                                                                


                                                       
                                            
                                                         

     
                                          
*/
wpt_uint8
WDI_FindAssocSession
(
  WDI_ControlBlockType*   pWDICtx,
  wpt_macAddr             macBSSID,
  WDI_BSSSessionType**    ppSession
)
{
  wpt_uint8 i;
  /*                                                                       */

  /*                                                                         
                
                                                                           */
  if ( NULL == ppSession )
  {
    WPAL_TRACE( eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_WARN,
                "%s: Invalid parameters", __FUNCTION__);
     return WDI_MAX_BSS_SESSIONS;
  }

  *ppSession = NULL;

  /*                                                                        
                                                 
                                                                            */
  for ( i = 0; i < WDI_MAX_BSS_SESSIONS; i++ )
  {
     if ( eWLAN_PAL_TRUE ==
          wpalMemoryCompare(pWDICtx->aBSSSessions[i].macBSSID, macBSSID, WDI_MAC_ADDR_LEN) )
     {
       /*                 */
       *ppSession = &pWDICtx->aBSSSessions[i];
       return i;
     }
  }

  return i;
}/*                    */

/* 
                                                                


                                                   
                                            
                                                         

     
                                          
*/
wpt_uint8
WDI_FindAssocSessionByBSSIdx
(
  WDI_ControlBlockType*   pWDICtx,
  wpt_uint16              ucBSSIdx,
  WDI_BSSSessionType**    ppSession
)
{
  wpt_uint8 i;
  /*                                                                       */

  /*                                                                         
                
                                                                           */
  if ( NULL == ppSession )
  {
    WPAL_TRACE( eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_WARN,
                "%s: Invalid parameters", __FUNCTION__);
     return WDI_MAX_BSS_SESSIONS;
  }

  *ppSession = NULL;

  /*                                                                        
                                                 
                                                                            */
  for ( i = 0; i < WDI_MAX_BSS_SESSIONS; i++ )
  {
     if ( ucBSSIdx == pWDICtx->aBSSSessions[i].ucBSSIdx )
     {
       /*                 */
       *ppSession = &pWDICtx->aBSSSessions[i];
       return i;
     }
  }

  return i;
}/*                            */

/* 
                                                                


                                                   
                                            
                                                         

     
                                          
*/
wpt_uint8
WDI_FindAssocSessionByIdx
(
  WDI_ControlBlockType*   pWDICtx,
  wpt_uint16              usIdx,
  WDI_BSSSessionType**    ppSession
)
{
  /*                                                                       */

  /*                                                                         
                
                                                                           */
  if ( NULL == ppSession || usIdx >= WDI_MAX_BSS_SESSIONS )
  {
    WPAL_TRACE( eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_WARN,
                "%s: Invalid parameters", __FUNCTION__);
     return WDI_MAX_BSS_SESSIONS;
  }

  /*                 */
  *ppSession = &pWDICtx->aBSSSessions[usIdx];

  return usIdx;

}/*                            */

/* 
                                                               
          


                                                       
                                                         

     
                                          
*/
wpt_uint8
WDI_FindEmptySession
(
  WDI_ControlBlockType*   pWDICtx,
  WDI_BSSSessionType**    ppSession
)
{
  wpt_uint8 i;
  /*                                                                       */
   /*                                                                         
                
                                                                           */
  if ( NULL == ppSession )
  {
    WPAL_TRACE( eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_WARN,
                "%s: Invalid parameters", __FUNCTION__);
     return WDI_MAX_BSS_SESSIONS;
  }

  *ppSession = NULL;

  /*                                                                        
                                                
                                                                            */
  for ( i = 0; i < WDI_MAX_BSS_SESSIONS; i++ )
  {
     if ( ! pWDICtx->aBSSSessions[i].bInUse )
     {
       /*               */
       *ppSession = &pWDICtx->aBSSSessions[i];
       return i;
     }
  }

  return i;
}/*                    */


/* 
                                                            
                


                                                       

     
                                  
*/
wpt_uint8
WDI_GetActiveSessionsCount
(
  WDI_ControlBlockType*   pWDICtx
)
{
  wpt_uint8 i, ucCount = 0;
  /*                                                                       */

  /*                                                                        
                               
                                                                            */
  for ( i = 0; i < WDI_MAX_BSS_SESSIONS; i++ )
  {
     if ( pWDICtx->aBSSSessions[i].bInUse )
     {
       ucCount++;
     }
  }

  return ucCount;
}/*                          */

/* 
                                                        
          


                                                       
                                                         

     
                                          
*/
void
WDI_DeleteSession
(
  WDI_ControlBlockType*   pWDICtx,
  WDI_BSSSessionType*     ppSession
)
{
   /*                                                                         
                
                                                                           */
  if ( NULL == ppSession )
  {
    WPAL_TRACE( eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_WARN,
                "%s: Invalid parameters", __FUNCTION__);
     return ;
  }

  /*                                                                        
                                   
                                                                            */
  wpal_list_destroy(&ppSession->wptPendingQueue);
  wpalMemoryZero(ppSession,  sizeof(*ppSession));
  ppSession->wdiAssocState = WDI_ASSOC_INIT_ST;
  ppSession->bInUse        = eWLAN_PAL_FALSE;
  ppSession->wdiBssType    = WDI_INFRASTRUCTURE_MODE;
  wpal_list_init(&ppSession->wptPendingQueue);

}/*                 */

/* 
                                                                          
                                                       
       

                                                         
                                                      

     
             
*/
void
WDI_AddBcastSTAtoSTATable
(
  WDI_ControlBlockType*  pWDICtx,
  WDI_AddStaParams *     staParams,
  wpt_uint16             usBcastStaIdx
)
{
  WDI_AddStaParams              wdiAddSTAParam = {0};
  wpt_macAddr  bcastMacAddr = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
  /*                                                                       */

  /*                                                                     
                
                                                                       */
  if ( NULL == staParams )
  {
    WPAL_TRACE( eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_WARN,
                "%s: Invalid parameters", __FUNCTION__);

    return;
  }

  wdiAddSTAParam.bcastDpuIndex = staParams->bcastDpuIndex;
  wdiAddSTAParam.bcastDpuSignature = staParams->bcastDpuSignature;
  wdiAddSTAParam.bcastMgmtDpuIndex = staParams->bcastMgmtDpuIndex;
  wdiAddSTAParam.bcastMgmtDpuSignature = staParams->bcastMgmtDpuSignature;
  wdiAddSTAParam.dpuIndex = staParams->dpuIndex;
  wdiAddSTAParam.dpuSig = staParams->dpuSig;
  wpalMemoryCopy( wdiAddSTAParam.macBSSID, staParams->macBSSID,
                  WDI_MAC_ADDR_LEN );
  wpalMemoryCopy( wdiAddSTAParam.staMacAddr, bcastMacAddr, WDI_MAC_ADDR_LEN );
  wdiAddSTAParam.ucBSSIdx = staParams->ucBSSIdx;
  wdiAddSTAParam.ucHTCapable = staParams->ucHTCapable;
  wdiAddSTAParam.ucRmfEnabled = staParams->ucRmfEnabled;
  wdiAddSTAParam.ucStaType = WDI_STA_ENTRY_BCAST;
  wdiAddSTAParam.ucWmmEnabled = staParams->ucWmmEnabled;
  wdiAddSTAParam.ucSTAIdx = usBcastStaIdx;

  (void)WDI_STATableAddSta(pWDICtx,&wdiAddSTAParam);
}

/* 
                                                              
            

                                                         
                                                                    

     
                                    
 */

WDI_Status WDI_SendNvBlobReq
(
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
)
{

  tHalNvImgDownloadReqMsg    halNvImgDownloadParam;
  wpt_uint8*                 pSendBuffer   = NULL;
  wpt_uint16                 usDataOffset  = 0;
  wpt_uint16                 usSendSize    = 0;
  wpt_uint16                 usCurrentFragmentSize =0;
  wpt_uint8*                 pSrcBuffer = NULL;
  WDI_NvDownloadReqParamsType*  pwdiNvDownloadReqParams =NULL ;
  WDI_NvDownloadRspCb      wdiNvDownloadRspCb;

  wdiNvDownloadRspCb = (WDI_NvDownloadRspCb)pEventData->pCBfnc;
  WDI_ASSERT(NULL != wdiNvDownloadRspCb);
  pwdiNvDownloadReqParams = (WDI_NvDownloadReqParamsType*)pEventData->pEventData;

  /*                                    */
  pSrcBuffer =(wpt_uint8 *) pwdiNvDownloadReqParams->wdiBlobInfo.pBlobAddress;

  /*                                     */
  pWDICtx->wdiNvBlobInfo.usCurrentFragment += 1;

  /*                             */
  /*                                                                                           */
  halNvImgDownloadParam.nvImageReqParams.fragNumber =
                                     pWDICtx->wdiNvBlobInfo.usCurrentFragment-1;

  /*                                                                                
                                                                                        
                                */

  if(pWDICtx->wdiNvBlobInfo.usTotalFragment
                         == pWDICtx->wdiNvBlobInfo.usCurrentFragment)
  {
    /*                                      */
    if( !(usCurrentFragmentSize =
                 pwdiNvDownloadReqParams->wdiBlobInfo.uBlobSize%FRAGMENT_SIZE ))
      usCurrentFragmentSize = FRAGMENT_SIZE;

    /*                             */
    halNvImgDownloadParam.nvImageReqParams.isLastFragment = 1;
    halNvImgDownloadParam.nvImageReqParams.nvImgBufferSize= usCurrentFragmentSize;

  }
  else
  {
    usCurrentFragmentSize = FRAGMENT_SIZE;

    /*                             */
    halNvImgDownloadParam.nvImageReqParams.isLastFragment =0;
    halNvImgDownloadParam.nvImageReqParams.nvImgBufferSize = usCurrentFragmentSize;
  }

  /*                                                                       
                     
                                                                          */
  if (( WDI_STATUS_SUCCESS != WDI_GetMessageBuffer( pWDICtx,WDI_NV_DOWNLOAD_REQ,
         sizeof(halNvImgDownloadParam.nvImageReqParams)+ usCurrentFragmentSize,
                    &pSendBuffer, &usDataOffset, &usSendSize))||
      ( usSendSize <
           (usDataOffset + sizeof(halNvImgDownloadParam.nvImageReqParams) + usCurrentFragmentSize )))
  {
    WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_WARN,
         "Unable to get send buffer in NV Download req %x %x ",
         pEventData, pwdiNvDownloadReqParams);
    WDI_ASSERT(0);
    return WDI_STATUS_E_FAILURE;
  }

  /*                                           */
  wpalMemoryCopy(pSendBuffer + usDataOffset ,
    &halNvImgDownloadParam.nvImageReqParams ,sizeof(tHalNvImgDownloadReqParams));

  /*                                 */
  wpalMemoryCopy(pSendBuffer + usDataOffset + sizeof(tHalNvImgDownloadReqParams),
        (void *)(pSrcBuffer + halNvImgDownloadParam.nvImageReqParams.fragNumber * FRAGMENT_SIZE),
                  usCurrentFragmentSize);

  pWDICtx->wdiReqStatusCB     = pwdiNvDownloadReqParams->wdiReqStatusCB;
  pWDICtx->pReqStatusUserData = pwdiNvDownloadReqParams->pUserData;

  return  WDI_SendMsg( pWDICtx, pSendBuffer, usSendSize,
                           wdiNvDownloadRspCb, pEventData->pUserData,
                           WDI_NV_DOWNLOAD_RESP);

}
/*                                                                            
                             
                                                                             */
/* 
                                                                
                                                    
                                                      

     
        
*/
WPT_INLINE void
WDI_DS_AssignDatapathContext (void *pContext, void *pDPContext)
{
   WDI_ControlBlockType *pCB = (WDI_ControlBlockType *)pContext;

   pCB->pDPContext = pDPContext;
   return;
}

/* 
                                                                


                                                    

     
                                    
*/
WPT_INLINE void *
WDI_DS_GetDatapathContext (void *pContext)
{
   WDI_ControlBlockType *pCB = (WDI_ControlBlockType *)pContext;
   return pCB->pDPContext;
}
/* 
                                                                


                                                    
                                                                    

     
             
*/
WPT_INLINE void
WDT_AssignTransportDriverContext (void *pContext, void *pDTDriverContext)
{
   WDI_ControlBlockType *pCB = (WDI_ControlBlockType *)pContext;

   pCB->pDTDriverContext = pDTDriverContext;
   return;
}

/* 
                                                                


                                                   

     
                                    
*/
WPT_INLINE void *
WDT_GetTransportDriverContext (void *pContext)
{
   WDI_ControlBlockType *pCB = (WDI_ControlBlockType *)pContext;
   return(pCB->pDTDriverContext);
}

/*                                                                            
                          
                                                                             */
/*                                            */
WPT_STATIC WPT_INLINE WDI_Status
WDI_HAL_2_WDI_STATUS
(
  eHalStatus halStatus
)
{
  /*                                                                          
                                  */
  switch(  halStatus )
  {
  case eHAL_STATUS_SUCCESS:
  case eHAL_STATUS_ADD_STA_SELF_IGNORED_REF_COUNT_NOT_ZERO:
  case eHAL_STATUS_DEL_STA_SELF_IGNORED_REF_COUNT_NOT_ZERO:
    return WDI_STATUS_SUCCESS;
  case eHAL_STATUS_FAILURE:
    return WDI_STATUS_E_FAILURE;
  case eHAL_STATUS_FAILED_ALLOC:
    return WDI_STATUS_MEM_FAILURE;
   /*                                                                    
                                                          */
  default:
    return WDI_STATUS_DEV_INTERNAL_FAILURE;
  }

  return WDI_STATUS_E_FAILURE;
}/*                    */

/*                                              */
WPT_STATIC WPT_INLINE tHalHostMsgType
WDI_2_HAL_REQ_TYPE
(
  WDI_RequestEnumType    wdiReqType
)
{
   /*                                                                          
                                  */
  switch(  wdiReqType )
  {
  case WDI_START_REQ:
    return WLAN_HAL_START_REQ;
  case WDI_STOP_REQ:
    return WLAN_HAL_STOP_REQ;
  case WDI_INIT_SCAN_REQ:
    return WLAN_HAL_INIT_SCAN_REQ;
  case WDI_START_SCAN_REQ:
    return WLAN_HAL_START_SCAN_REQ;
  case WDI_END_SCAN_REQ:
    return WLAN_HAL_END_SCAN_REQ;
  case WDI_FINISH_SCAN_REQ:
    return WLAN_HAL_FINISH_SCAN_REQ;
  case WDI_JOIN_REQ:
    return WLAN_HAL_JOIN_REQ;
  case WDI_CONFIG_BSS_REQ:
    return WLAN_HAL_CONFIG_BSS_REQ;
  case WDI_DEL_BSS_REQ:
    return WLAN_HAL_DELETE_BSS_REQ;
  case WDI_POST_ASSOC_REQ:
    return WLAN_HAL_POST_ASSOC_REQ;
  case WDI_DEL_STA_REQ:
    return WLAN_HAL_DELETE_STA_REQ;
  case WDI_SET_BSS_KEY_REQ:
    return WLAN_HAL_SET_BSSKEY_REQ;
  case WDI_RMV_BSS_KEY_REQ:
    return WLAN_HAL_RMV_BSSKEY_REQ;
  case WDI_SET_STA_KEY_REQ:
    return WLAN_HAL_SET_STAKEY_REQ;
  case WDI_RMV_STA_KEY_REQ:
    return WLAN_HAL_RMV_STAKEY_REQ;
  case WDI_SET_STA_BCAST_KEY_REQ:
    return WLAN_HAL_SET_BCASTKEY_REQ;
  case WDI_RMV_STA_BCAST_KEY_REQ:
    //                                                                             
    return WLAN_HAL_RMV_STAKEY_REQ;
  case WDI_ADD_TS_REQ:
    return WLAN_HAL_ADD_TS_REQ;
  case WDI_DEL_TS_REQ:
    return WLAN_HAL_DEL_TS_REQ;
  case WDI_UPD_EDCA_PRMS_REQ:
    return WLAN_HAL_UPD_EDCA_PARAMS_REQ;
  case WDI_ADD_BA_REQ:
    return WLAN_HAL_ADD_BA_REQ;
  case WDI_DEL_BA_REQ:
    return WLAN_HAL_DEL_BA_REQ;
#ifdef FEATURE_WLAN_CCX
  case WDI_TSM_STATS_REQ:
    return WLAN_HAL_TSM_STATS_REQ;
#endif
  case WDI_CH_SWITCH_REQ:
    return WLAN_HAL_CH_SWITCH_REQ;
  case WDI_CONFIG_STA_REQ:
    return WLAN_HAL_CONFIG_STA_REQ;
  case WDI_SET_LINK_ST_REQ:
    return WLAN_HAL_SET_LINK_ST_REQ;
  case WDI_GET_STATS_REQ:
    return WLAN_HAL_GET_STATS_REQ;
  case WDI_UPDATE_CFG_REQ:
    return WLAN_HAL_UPDATE_CFG_REQ;
  case WDI_ADD_BA_SESSION_REQ:
    return WLAN_HAL_ADD_BA_SESSION_REQ;
  case WDI_TRIGGER_BA_REQ:
    return WLAN_HAL_TRIGGER_BA_REQ;
  case WDI_UPD_BCON_PRMS_REQ:
    return WLAN_HAL_UPDATE_BEACON_REQ;
  case WDI_SND_BCON_REQ:
    return WLAN_HAL_SEND_BEACON_REQ;
  case WDI_UPD_PROBE_RSP_TEMPLATE_REQ:
    return WLAN_HAL_UPDATE_PROBE_RSP_TEMPLATE_REQ;
   case WDI_SET_MAX_TX_POWER_REQ:
    return WLAN_HAL_SET_MAX_TX_POWER_REQ;
#ifdef WLAN_FEATURE_P2P
  case WDI_P2P_GO_NOTICE_OF_ABSENCE_REQ:
    return WLAN_HAL_SET_P2P_GONOA_REQ;
#endif
  case WDI_ENTER_IMPS_REQ:
    return WLAN_HAL_ENTER_IMPS_REQ;
  case WDI_EXIT_IMPS_REQ:
    return WLAN_HAL_EXIT_IMPS_REQ;
  case WDI_ENTER_BMPS_REQ:
    return WLAN_HAL_ENTER_BMPS_REQ;
  case WDI_EXIT_BMPS_REQ:
    return WLAN_HAL_EXIT_BMPS_REQ;
  case WDI_ENTER_UAPSD_REQ:
    return WLAN_HAL_ENTER_UAPSD_REQ;
  case WDI_EXIT_UAPSD_REQ:
    return WLAN_HAL_EXIT_UAPSD_REQ;
  case WDI_SET_UAPSD_PARAM_REQ:
    return WLAN_HAL_SET_UAPSD_AC_PARAMS_REQ;
  case WDI_UPDATE_UAPSD_PARAM_REQ:
    return WLAN_HAL_UPDATE_UAPSD_PARAM_REQ;
  case WDI_CONFIGURE_RXP_FILTER_REQ:
    return WLAN_HAL_CONFIGURE_RXP_FILTER_REQ;
  case WDI_SET_BEACON_FILTER_REQ:
    return WLAN_HAL_ADD_BCN_FILTER_REQ;
  case WDI_REM_BEACON_FILTER_REQ:
    return WLAN_HAL_REM_BCN_FILTER_REQ;
  case WDI_SET_RSSI_THRESHOLDS_REQ:
    return WLAN_HAL_SET_RSSI_THRESH_REQ;
  case WDI_HOST_OFFLOAD_REQ:
    return WLAN_HAL_HOST_OFFLOAD_REQ;
  case WDI_WOWL_ADD_BC_PTRN_REQ:
    return WLAN_HAL_ADD_WOWL_BCAST_PTRN;
  case WDI_WOWL_DEL_BC_PTRN_REQ:
    return WLAN_HAL_DEL_WOWL_BCAST_PTRN;
  case WDI_WOWL_ENTER_REQ:
    return WLAN_HAL_ENTER_WOWL_REQ;
  case WDI_WOWL_EXIT_REQ:
    return WLAN_HAL_EXIT_WOWL_REQ;
  case WDI_CONFIGURE_APPS_CPU_WAKEUP_STATE_REQ:
    return WLAN_HAL_CONFIGURE_APPS_CPU_WAKEUP_STATE_REQ;
   case WDI_NV_DOWNLOAD_REQ:
    return WLAN_HAL_DOWNLOAD_NV_REQ;
  case WDI_FLUSH_AC_REQ:
    return WLAN_HAL_TL_HAL_FLUSH_AC_REQ;
  case WDI_BTAMP_EVENT_REQ:
    return WLAN_HAL_SIGNAL_BTAMP_EVENT_REQ;
#ifdef WLAN_FEATURE_VOWIFI_11R
  case WDI_AGGR_ADD_TS_REQ:
     return WLAN_HAL_AGGR_ADD_TS_REQ;
#endif /*                         */
#ifdef ANI_MANF_DIAG
  case WDI_FTM_CMD_REQ:
    return WLAN_HAL_PROCESS_PTT_REQ;
#endif /*               */
  case WDI_ADD_STA_SELF_REQ:
    return WLAN_HAL_ADD_STA_SELF_REQ;
  case WDI_DEL_STA_SELF_REQ:
    return WLAN_HAL_DEL_STA_SELF_REQ;
#ifdef FEATURE_OEM_DATA_SUPPORT
  case WDI_START_OEM_DATA_REQ:
    return WLAN_HAL_START_OEM_DATA_REQ;
#endif /*                          */
  case WDI_HOST_RESUME_REQ:
    return WLAN_HAL_HOST_RESUME_REQ;
  case WDI_HOST_SUSPEND_IND:
    return WLAN_HAL_HOST_SUSPEND_IND;
  case WDI_KEEP_ALIVE_REQ:
    return WLAN_HAL_KEEP_ALIVE_REQ;

#ifdef FEATURE_WLAN_SCAN_PNO
  case WDI_SET_PREF_NETWORK_REQ:
    return WLAN_HAL_SET_PREF_NETWORK_REQ;
  case WDI_SET_RSSI_FILTER_REQ:
    return WLAN_HAL_SET_RSSI_FILTER_REQ;
  case WDI_UPDATE_SCAN_PARAMS_REQ:
    return WLAN_HAL_UPDATE_SCAN_PARAM_REQ;
#endif //                      
  case WDI_SET_TX_PER_TRACKING_REQ:
    return WLAN_HAL_SET_TX_PER_TRACKING_REQ;
#ifdef WLAN_FEATURE_PACKET_FILTERING
  case WDI_8023_MULTICAST_LIST_REQ:
    return WLAN_HAL_8023_MULTICAST_LIST_REQ;
  case WDI_RECEIVE_FILTER_SET_FILTER_REQ:
    return WLAN_HAL_SET_PACKET_FILTER_REQ;
  case WDI_PACKET_COALESCING_FILTER_MATCH_COUNT_REQ:
    return WLAN_HAL_PACKET_FILTER_MATCH_COUNT_REQ;
  case WDI_RECEIVE_FILTER_CLEAR_FILTER_REQ:
    return WLAN_HAL_CLEAR_PACKET_FILTER_REQ;
#endif //                              
  case WDI_HAL_DUMP_CMD_REQ:
    return WLAN_HAL_DUMP_COMMAND_REQ;
#ifdef WLAN_FEATURE_GTK_OFFLOAD
  case WDI_GTK_OFFLOAD_REQ:
    return WLAN_HAL_GTK_OFFLOAD_REQ;
  case WDI_GTK_OFFLOAD_GETINFO_REQ:
    return WLAN_HAL_GTK_OFFLOAD_GETINFO_REQ;
#endif /*                          */

  case WDI_INIT_SCAN_CON_REQ:
    return WLAN_HAL_INIT_SCAN_CON_REQ;
  case WDI_SET_POWER_PARAMS_REQ:
    return WLAN_HAL_SET_POWER_PARAMS_REQ;
  case WDI_SET_TM_LEVEL_REQ:
    return WLAN_HAL_SET_THERMAL_MITIGATION_REQ;
  case WDI_FEATURE_CAPS_EXCHANGE_REQ:
    return WLAN_HAL_FEATURE_CAPS_EXCHANGE_REQ;
  default:
    return WLAN_HAL_MSG_MAX;
  }

}/*                  */

/*                                                */
WPT_STATIC WPT_INLINE WDI_ResponseEnumType
HAL_2_WDI_RSP_TYPE
(
  tHalHostMsgType halMsg
)
{
  /*                                                                          
                                  */
  switch(  halMsg )
  {
  case WLAN_HAL_START_RSP:
    return WDI_START_RESP;
  case WLAN_HAL_STOP_RSP:
    return WDI_STOP_RESP;
  case WLAN_HAL_INIT_SCAN_RSP:
    return WDI_INIT_SCAN_RESP;
  case WLAN_HAL_START_SCAN_RSP:
    return WDI_START_SCAN_RESP;
  case WLAN_HAL_END_SCAN_RSP:
    return WDI_END_SCAN_RESP;
  case WLAN_HAL_FINISH_SCAN_RSP:
    return WDI_FINISH_SCAN_RESP;
  case WLAN_HAL_CONFIG_STA_RSP:
    return WDI_CONFIG_STA_RESP;
  case WLAN_HAL_DELETE_STA_RSP:
    return WDI_DEL_STA_RESP;
  case WLAN_HAL_CONFIG_BSS_RSP:
    return WDI_CONFIG_BSS_RESP;
  case WLAN_HAL_DELETE_BSS_RSP:
    return WDI_DEL_BSS_RESP;
  case WLAN_HAL_JOIN_RSP:
    return WDI_JOIN_RESP;
  case WLAN_HAL_POST_ASSOC_RSP:
    return WDI_POST_ASSOC_RESP;
  case WLAN_HAL_SET_BSSKEY_RSP:
    return WDI_SET_BSS_KEY_RESP;
  case WLAN_HAL_SET_STAKEY_RSP:
    return WDI_SET_STA_KEY_RESP;
  case WLAN_HAL_RMV_BSSKEY_RSP:
    return WDI_RMV_BSS_KEY_RESP;
  case WLAN_HAL_RMV_STAKEY_RSP:
    return WDI_RMV_STA_KEY_RESP;
  case WLAN_HAL_SET_BCASTKEY_RSP:
    return WDI_SET_STA_BCAST_KEY_RESP;
  //                                                                           
  //                                    
  case WLAN_HAL_ADD_TS_RSP:
    return WDI_ADD_TS_RESP;
  case WLAN_HAL_DEL_TS_RSP:
    return WDI_DEL_TS_RESP;
  case WLAN_HAL_UPD_EDCA_PARAMS_RSP:
    return WDI_UPD_EDCA_PRMS_RESP;
  case WLAN_HAL_ADD_BA_RSP:
    return WDI_ADD_BA_RESP;
  case WLAN_HAL_DEL_BA_RSP:
    return WDI_DEL_BA_RESP;
#ifdef FEATURE_WLAN_CCX
  case WLAN_HAL_TSM_STATS_RSP:
    return WDI_TSM_STATS_RESP;
#endif
  case WLAN_HAL_CH_SWITCH_RSP:
    return WDI_CH_SWITCH_RESP;
  case WLAN_HAL_SET_LINK_ST_RSP:
    return WDI_SET_LINK_ST_RESP;
  case WLAN_HAL_GET_STATS_RSP:
    return WDI_GET_STATS_RESP;
  case WLAN_HAL_UPDATE_CFG_RSP:
    return WDI_UPDATE_CFG_RESP;
  case WLAN_HAL_ADD_BA_SESSION_RSP:
    return WDI_ADD_BA_SESSION_RESP;
  case WLAN_HAL_TRIGGER_BA_RSP:
    return WDI_TRIGGER_BA_RESP;
  case WLAN_HAL_UPDATE_BEACON_RSP:
    return WDI_UPD_BCON_PRMS_RESP;
  case WLAN_HAL_SEND_BEACON_RSP:
    return WDI_SND_BCON_RESP;
  case WLAN_HAL_UPDATE_PROBE_RSP_TEMPLATE_RSP:
    return WDI_UPD_PROBE_RSP_TEMPLATE_RESP;
  /*           */
  case WLAN_HAL_RSSI_NOTIFICATION_IND:
    return WDI_HAL_RSSI_NOTIFICATION_IND;
  case WLAN_HAL_MISSED_BEACON_IND:
    return WDI_HAL_MISSED_BEACON_IND;
  case WLAN_HAL_UNKNOWN_ADDR2_FRAME_RX_IND:
    return WDI_HAL_UNKNOWN_ADDR2_FRAME_RX_IND;
  case WLAN_HAL_MIC_FAILURE_IND:
    return WDI_HAL_MIC_FAILURE_IND;
  case WLAN_HAL_FATAL_ERROR_IND:
    return WDI_HAL_FATAL_ERROR_IND;
  case WLAN_HAL_DELETE_STA_CONTEXT_IND:
    return WDI_HAL_DEL_STA_IND;
  case WLAN_HAL_COEX_IND:
    return WDI_HAL_COEX_IND;
  case WLAN_HAL_OTA_TX_COMPL_IND:
    return WDI_HAL_TX_COMPLETE_IND;
#ifdef WLAN_FEATURE_P2P
  case WLAN_HAL_P2P_NOA_ATTR_IND:
    return WDI_HAL_P2P_NOA_ATTR_IND;
#endif
  case WLAN_HAL_TX_PER_HIT_IND:
    return WDI_HAL_TX_PER_HIT_IND;
  case WLAN_HAL_SET_MAX_TX_POWER_RSP:
    return WDI_SET_MAX_TX_POWER_RESP;
#ifdef WLAN_FEATURE_P2P
  case WLAN_HAL_SET_P2P_GONOA_RSP:
    return WDI_P2P_GO_NOTICE_OF_ABSENCE_RESP;
#endif
  case WLAN_HAL_ENTER_IMPS_RSP:
    return WDI_ENTER_IMPS_RESP;
  case WLAN_HAL_EXIT_IMPS_RSP:
    return WDI_EXIT_IMPS_RESP;
  case WLAN_HAL_ENTER_BMPS_RSP:
    return WDI_ENTER_BMPS_RESP;
  case WLAN_HAL_EXIT_BMPS_RSP:
    return WDI_EXIT_BMPS_RESP;
  case WLAN_HAL_ENTER_UAPSD_RSP:
    return WDI_ENTER_UAPSD_RESP;
  case WLAN_HAL_EXIT_UAPSD_RSP:
    return WDI_EXIT_UAPSD_RESP;
  case WLAN_HAL_SET_UAPSD_AC_PARAMS_RSP:
    return WDI_SET_UAPSD_PARAM_RESP;
  case WLAN_HAL_UPDATE_UAPSD_PARAM_RSP:
    return WDI_UPDATE_UAPSD_PARAM_RESP;
  case WLAN_HAL_CONFIGURE_RXP_FILTER_RSP:
    return WDI_CONFIGURE_RXP_FILTER_RESP;
  case WLAN_HAL_ADD_BCN_FILTER_RSP:
    return WDI_SET_BEACON_FILTER_RESP;
  case WLAN_HAL_REM_BCN_FILTER_RSP:
    return WDI_REM_BEACON_FILTER_RESP;
  case WLAN_HAL_SET_RSSI_THRESH_RSP:
    return WDI_SET_RSSI_THRESHOLDS_RESP;
  case WLAN_HAL_HOST_OFFLOAD_RSP:
    return WDI_HOST_OFFLOAD_RESP;
  case WLAN_HAL_ADD_WOWL_BCAST_PTRN_RSP:
    return WDI_WOWL_ADD_BC_PTRN_RESP;
  case WLAN_HAL_DEL_WOWL_BCAST_PTRN_RSP:
    return WDI_WOWL_DEL_BC_PTRN_RESP;
  case WLAN_HAL_ENTER_WOWL_RSP:
    return WDI_WOWL_ENTER_RESP;
  case WLAN_HAL_EXIT_WOWL_RSP:
    return WDI_WOWL_EXIT_RESP;
  case WLAN_HAL_CONFIGURE_APPS_CPU_WAKEUP_STATE_RSP:
    return WDI_CONFIGURE_APPS_CPU_WAKEUP_STATE_RESP;
  case WLAN_HAL_DOWNLOAD_NV_RSP:
    return WDI_NV_DOWNLOAD_RESP;
  case WLAN_HAL_TL_HAL_FLUSH_AC_RSP:
    return WDI_FLUSH_AC_RESP;
  case WLAN_HAL_SIGNAL_BTAMP_EVENT_RSP:
    return WDI_BTAMP_EVENT_RESP;
#ifdef ANI_MANF_DIAG
  case WLAN_HAL_PROCESS_PTT_RSP:
    return  WDI_FTM_CMD_RESP;
#endif /*               */
  case WLAN_HAL_ADD_STA_SELF_RSP:
    return WDI_ADD_STA_SELF_RESP;
case WLAN_HAL_DEL_STA_SELF_RSP:
    return WDI_DEL_STA_SELF_RESP;
#ifdef FEATURE_OEM_DATA_SUPPORT
  case WLAN_HAL_START_OEM_DATA_RSP:
    return WDI_START_OEM_DATA_RESP;
#endif /*                          */
  case WLAN_HAL_HOST_RESUME_RSP:
    return WDI_HOST_RESUME_RESP;
  case WLAN_HAL_KEEP_ALIVE_RSP:
    return WDI_KEEP_ALIVE_RESP;
#ifdef FEATURE_WLAN_SCAN_PNO
  case WLAN_HAL_SET_PREF_NETWORK_RSP:
    return WDI_SET_PREF_NETWORK_RESP;
  case WLAN_HAL_SET_RSSI_FILTER_RSP:
    return WDI_SET_RSSI_FILTER_RESP;
  case WLAN_HAL_UPDATE_SCAN_PARAM_RSP:
    return WDI_UPDATE_SCAN_PARAMS_RESP;
  case WLAN_HAL_PREF_NETW_FOUND_IND:
    return WDI_HAL_PREF_NETWORK_FOUND_IND;
#endif //                      
  case WLAN_HAL_SET_TX_PER_TRACKING_RSP:
    return WDI_SET_TX_PER_TRACKING_RESP;
#ifdef WLAN_FEATURE_PACKET_FILTERING
  case WLAN_HAL_8023_MULTICAST_LIST_RSP:
    return WDI_8023_MULTICAST_LIST_RESP;
  case WLAN_HAL_SET_PACKET_FILTER_RSP:
    return WDI_RECEIVE_FILTER_SET_FILTER_RESP;
  case WLAN_HAL_PACKET_FILTER_MATCH_COUNT_RSP:
    return WDI_PACKET_COALESCING_FILTER_MATCH_COUNT_RESP;
  case WLAN_HAL_CLEAR_PACKET_FILTER_RSP:
    return WDI_RECEIVE_FILTER_CLEAR_FILTER_RESP;
#endif //                              

  case WLAN_HAL_DUMP_COMMAND_RSP:
    return WDI_HAL_DUMP_CMD_RESP;
  case WLAN_HAL_SET_POWER_PARAMS_RSP:
    return WDI_SET_POWER_PARAMS_RESP;
#ifdef WLAN_FEATURE_VOWIFI_11R
  case WLAN_HAL_AGGR_ADD_TS_RSP:
    return WDI_AGGR_ADD_TS_RESP;
#endif

#ifdef WLAN_FEATURE_GTK_OFFLOAD
  case WLAN_HAL_GTK_OFFLOAD_RSP:
    return WDI_GTK_OFFLOAD_RESP;
  case WLAN_HAL_GTK_OFFLOAD_GETINFO_RSP:
    return WDI_GTK_OFFLOAD_GETINFO_RESP;
#endif /*                          */
#ifdef WLAN_WAKEUP_EVENTS
  case WLAN_HAL_WAKE_REASON_IND:
    return WDI_HAL_WAKE_REASON_IND;
#endif //                   

  case WLAN_HAL_SET_THERMAL_MITIGATION_RSP:
    return WDI_SET_TM_LEVEL_RESP;
  case WLAN_HAL_FEATURE_CAPS_EXCHANGE_RSP:
      return WDI_FEATURE_CAPS_EXCHANGE_RESP;
  default:
    return eDRIVER_TYPE_MAX;
  }

}/*                  */


/*                                            */
WPT_STATIC WPT_INLINE tDriverType
WDI_2_HAL_DRV_TYPE
(
  WDI_DriverType wdiDriverType
)
{
  /*                                                                          
                                  */
  switch(  wdiDriverType )
  {
  case WDI_DRIVER_TYPE_PRODUCTION:
    return eDRIVER_TYPE_PRODUCTION;
  case WDI_DRIVER_TYPE_MFG:
    return eDRIVER_TYPE_MFG;
  case WDI_DRIVER_TYPE_DVT:
    return eDRIVER_TYPE_DVT;
  }

  return eDRIVER_TYPE_MAX;
}/*                  */


/*                                            */
WPT_STATIC WPT_INLINE tHalStopType
WDI_2_HAL_STOP_REASON
(
  WDI_StopType wdiDriverType
)
{
  /*                                                                          
                                  */
  switch(  wdiDriverType )
  {
  case WDI_STOP_TYPE_SYS_RESET:
    return HAL_STOP_TYPE_SYS_RESET;
  case WDI_DRIVER_TYPE_MFG:
    return WDI_STOP_TYPE_SYS_DEEP_SLEEP;
  case WDI_STOP_TYPE_RF_KILL:
    return HAL_STOP_TYPE_RF_KILL;
  }

  return HAL_STOP_TYPE_MAX;
}/*                     */


/*                                                  */
WPT_STATIC WPT_INLINE eHalSysMode
WDI_2_HAL_SCAN_MODE
(
  WDI_ScanMode wdiScanMode
)
{
  /*                                                                          
                                  */
  switch(  wdiScanMode )
  {
  case WDI_SCAN_MODE_NORMAL:
    return eHAL_SYS_MODE_NORMAL;
  case WDI_SCAN_MODE_LEARN:
    return eHAL_SYS_MODE_LEARN;
  case WDI_SCAN_MODE_SCAN:
    return eHAL_SYS_MODE_SCAN;
  case WDI_SCAN_MODE_PROMISC:
    return eHAL_SYS_MODE_PROMISC;
  case WDI_SCAN_MODE_SUSPEND_LINK:
    return eHAL_SYS_MODE_SUSPEND_LINK;
  case WDI_SCAN_MODE_ROAM_SCAN:
    return eHAL_SYS_MODE_ROAM_SCAN;
  case WDI_SCAN_MODE_ROAM_SUSPEND_LINK:
    return eHAL_SYS_MODE_ROAM_SUSPEND_LINK;
  }

  return eHAL_SYS_MODE_MAX;
}/*                   */

/*                                                     */
WPT_STATIC WPT_INLINE ePhyChanBondState
WDI_2_HAL_SEC_CH_OFFSET
(
  WDI_HTSecondaryChannelOffset wdiSecChOffset
)
{
  /*                                                                          
                                  */
  switch(  wdiSecChOffset )
  {
  case WDI_SECONDARY_CHANNEL_OFFSET_NONE:
    return PHY_SINGLE_CHANNEL_CENTERED;
  case WDI_SECONDARY_CHANNEL_OFFSET_UP:
    return PHY_DOUBLE_CHANNEL_LOW_PRIMARY;
  case WDI_SECONDARY_CHANNEL_OFFSET_DOWN:
    return PHY_DOUBLE_CHANNEL_HIGH_PRIMARY;
#ifdef WLAN_FEATURE_11AC
  case WDI_CHANNEL_20MHZ_LOW_40MHZ_CENTERED:
    return PHY_QUADRUPLE_CHANNEL_20MHZ_LOW_40MHZ_CENTERED;
  case WDI_CHANNEL_20MHZ_CENTERED_40MHZ_CENTERED:
    return PHY_QUADRUPLE_CHANNEL_20MHZ_LOW_40MHZ_CENTERED;
  case WDI_CHANNEL_20MHZ_HIGH_40MHZ_CENTERED:
    return PHY_QUADRUPLE_CHANNEL_20MHZ_HIGH_40MHZ_CENTERED;
  case WDI_CHANNEL_20MHZ_LOW_40MHZ_LOW:
    return PHY_QUADRUPLE_CHANNEL_20MHZ_LOW_40MHZ_LOW;
  case WDI_CHANNEL_20MHZ_HIGH_40MHZ_LOW:
    return PHY_QUADRUPLE_CHANNEL_20MHZ_LOW_40MHZ_LOW;
  case WDI_CHANNEL_20MHZ_LOW_40MHZ_HIGH:
    return PHY_QUADRUPLE_CHANNEL_20MHZ_LOW_40MHZ_HIGH;
  case WDI_CHANNEL_20MHZ_HIGH_40MHZ_HIGH:
     return PHY_QUADRUPLE_CHANNEL_20MHZ_HIGH_40MHZ_HIGH;
#endif
  default:
      break;
  }

  return PHY_CHANNEL_BONDING_STATE_MAX;
}/*                       */

/*                                      */
WPT_STATIC WPT_INLINE tSirBssType
WDI_2_HAL_BSS_TYPE
(
  WDI_BssType wdiBSSType
)
{
  /*                                                                          
                                  */
  switch(  wdiBSSType )
  {
  case WDI_INFRASTRUCTURE_MODE:
    return eSIR_INFRASTRUCTURE_MODE;
  case WDI_INFRA_AP_MODE:
    return eSIR_INFRA_AP_MODE;
  case WDI_IBSS_MODE:
    return eSIR_IBSS_MODE;
  case WDI_BTAMP_STA_MODE:
    return eSIR_BTAMP_STA_MODE;
  case WDI_BTAMP_AP_MODE:
    return eSIR_BTAMP_AP_MODE;
  case WDI_BSS_AUTO_MODE:
    return eSIR_AUTO_MODE;
  }

  return eSIR_DONOT_USE_BSS_TYPE;
}/*                  */

/*                                    */
WPT_STATIC WPT_INLINE tSirNwType
WDI_2_HAL_NW_TYPE
(
  WDI_NwType wdiNWType
)
{
  /*                                                                          
                                  */
  switch(  wdiNWType )
  {
  case WDI_11A_NW_TYPE:
    return eSIR_11A_NW_TYPE;
  case WDI_11B_NW_TYPE:
    return eSIR_11B_NW_TYPE;
  case WDI_11G_NW_TYPE:
    return eSIR_11G_NW_TYPE;
  case WDI_11N_NW_TYPE:
    return eSIR_11N_NW_TYPE;
  }

  return eSIR_DONOT_USE_NW_TYPE;
}/*                 */

/*                                                */
WPT_STATIC WPT_INLINE ePhyChanBondState
WDI_2_HAL_CB_STATE
(
  WDI_PhyChanBondState wdiCbState
)
{
  /*                                                                          
                                  */
  switch ( wdiCbState )
  {
  case WDI_PHY_SINGLE_CHANNEL_CENTERED:
    return PHY_SINGLE_CHANNEL_CENTERED;
  case WDI_PHY_DOUBLE_CHANNEL_LOW_PRIMARY:
    return PHY_DOUBLE_CHANNEL_LOW_PRIMARY;
  case WDI_PHY_DOUBLE_CHANNEL_CENTERED:
    return PHY_DOUBLE_CHANNEL_CENTERED;
  case WDI_PHY_DOUBLE_CHANNEL_HIGH_PRIMARY:
    return PHY_DOUBLE_CHANNEL_HIGH_PRIMARY;
#ifdef WLAN_FEATURE_11AC
  case WDI_QUADRUPLE_CHANNEL_20MHZ_LOW_40MHZ_CENTERED:
    return PHY_QUADRUPLE_CHANNEL_20MHZ_LOW_40MHZ_CENTERED;
  case WDI_QUADRUPLE_CHANNEL_20MHZ_CENTERED_40MHZ_CENTERED:
    return PHY_QUADRUPLE_CHANNEL_20MHZ_CENTERED_40MHZ_CENTERED;
  case WDI_QUADRUPLE_CHANNEL_20MHZ_HIGH_40MHZ_CENTERED:
    return PHY_QUADRUPLE_CHANNEL_20MHZ_HIGH_40MHZ_CENTERED;
  case WDI_QUADRUPLE_CHANNEL_20MHZ_LOW_40MHZ_LOW:
    return PHY_QUADRUPLE_CHANNEL_20MHZ_LOW_40MHZ_LOW;
  case WDI_QUADRUPLE_CHANNEL_20MHZ_HIGH_40MHZ_LOW:
    return PHY_QUADRUPLE_CHANNEL_20MHZ_HIGH_40MHZ_LOW;
  case WDI_QUADRUPLE_CHANNEL_20MHZ_LOW_40MHZ_HIGH:
    return PHY_QUADRUPLE_CHANNEL_20MHZ_LOW_40MHZ_HIGH;
  case WDI_QUADRUPLE_CHANNEL_20MHZ_HIGH_40MHZ_HIGH:
    return PHY_QUADRUPLE_CHANNEL_20MHZ_HIGH_40MHZ_HIGH;
#endif
  case WDI_MAX_CB_STATE:
  default:
    break;
  }

  return PHY_CHANNEL_BONDING_STATE_MAX;
}/*                  */

/*                                                */
WPT_STATIC WPT_INLINE tSirMacHTOperatingMode
WDI_2_HAL_HT_OPER_MODE
(
  WDI_HTOperatingMode wdiHTOperMode
)
{
  /*                                                                          
                                  */
  switch ( wdiHTOperMode )
  {
  case WDI_HT_OP_MODE_PURE:
    return eSIR_HT_OP_MODE_PURE;
  case WDI_HT_OP_MODE_OVERLAP_LEGACY:
    return eSIR_HT_OP_MODE_OVERLAP_LEGACY;
  case WDI_HT_OP_MODE_NO_LEGACY_20MHZ_HT:
    return eSIR_HT_OP_MODE_NO_LEGACY_20MHZ_HT;
  case WDI_HT_OP_MODE_MIXED:
    return eSIR_HT_OP_MODE_MIXED;
  }

  return eSIR_HT_OP_MODE_MAX;
}/*                      */

/*                                              */
WPT_STATIC WPT_INLINE tSirMacHTMIMOPowerSaveState
WDI_2_HAL_MIMO_PS
(
  WDI_HTMIMOPowerSaveState wdiHTOperMode
)
{
  /*                                                                          
                                  */
  switch ( wdiHTOperMode )
  {
  case WDI_HT_MIMO_PS_STATIC:
    return eSIR_HT_MIMO_PS_STATIC;
  case WDI_HT_MIMO_PS_DYNAMIC:
    return eSIR_HT_MIMO_PS_DYNAMIC;
  case WDI_HT_MIMO_PS_NA:
    return eSIR_HT_MIMO_PS_NA;
  case WDI_HT_MIMO_PS_NO_LIMIT:
    return eSIR_HT_MIMO_PS_NO_LIMIT;
  }

  return eSIR_HT_MIMO_PS_MAX;
}/*                 */

/*                                      */
WPT_STATIC WPT_INLINE tAniEdType
WDI_2_HAL_ENC_TYPE
(
  WDI_EncryptType wdiEncType
)
{
  /*                                                                          
                                  */
  switch ( wdiEncType )
  {
  case WDI_ENCR_NONE:
    return eSIR_ED_NONE;

  case WDI_ENCR_WEP40:
    return eSIR_ED_WEP40;

  case WDI_ENCR_WEP104:
    return eSIR_ED_WEP104;

  case WDI_ENCR_TKIP:
    return eSIR_ED_TKIP;

  case WDI_ENCR_CCMP:
    return eSIR_ED_CCMP;

  case WDI_ENCR_AES_128_CMAC:
    return eSIR_ED_AES_128_CMAC;
#if defined(FEATURE_WLAN_WAPI)
  case WDI_ENCR_WPI:
    return eSIR_ED_WPI;
#endif
  default:
    return eSIR_ED_NOT_IMPLEMENTED;
  }

}/*                  */

/*                                      */
WPT_STATIC WPT_INLINE tAniWepType
WDI_2_HAL_WEP_TYPE
(
  WDI_WepType  wdiWEPType
)
{
  /*                                                                          
                                  */
  switch ( wdiWEPType )
  {
  case WDI_WEP_STATIC:
    return eSIR_WEP_STATIC;

  case WDI_WEP_DYNAMIC:
    return eSIR_WEP_DYNAMIC;
  }

  return eSIR_WEP_MAX;
}/*                  */

WPT_STATIC WPT_INLINE tSirLinkState
WDI_2_HAL_LINK_STATE
(
  WDI_LinkStateType  wdiLinkState
)
{
  /*                                                                          
                                  */
  switch ( wdiLinkState )
  {
  case WDI_LINK_IDLE_STATE:
    return eSIR_LINK_IDLE_STATE;

  case WDI_LINK_PREASSOC_STATE:
    return eSIR_LINK_PREASSOC_STATE;

  case WDI_LINK_POSTASSOC_STATE:
    return eSIR_LINK_POSTASSOC_STATE;

  case WDI_LINK_AP_STATE:
    return eSIR_LINK_AP_STATE;

  case WDI_LINK_IBSS_STATE:
    return eSIR_LINK_IBSS_STATE;

  case WDI_LINK_BTAMP_PREASSOC_STATE:
    return eSIR_LINK_BTAMP_PREASSOC_STATE;

  case WDI_LINK_BTAMP_POSTASSOC_STATE:
    return eSIR_LINK_BTAMP_POSTASSOC_STATE;

  case WDI_LINK_BTAMP_AP_STATE:
    return eSIR_LINK_BTAMP_AP_STATE;

  case WDI_LINK_BTAMP_STA_STATE:
    return eSIR_LINK_BTAMP_STA_STATE;

  case WDI_LINK_LEARN_STATE:
    return eSIR_LINK_LEARN_STATE;

  case WDI_LINK_SCAN_STATE:
    return eSIR_LINK_SCAN_STATE;

  case WDI_LINK_FINISH_SCAN_STATE:
    return eSIR_LINK_FINISH_SCAN_STATE;

  case WDI_LINK_INIT_CAL_STATE:
    return eSIR_LINK_INIT_CAL_STATE;

  case WDI_LINK_FINISH_CAL_STATE:
    return eSIR_LINK_FINISH_CAL_STATE;

#ifdef WLAN_FEATURE_P2P
  case WDI_LINK_LISTEN_STATE:
    return eSIR_LINK_LISTEN_STATE;
#endif

  default:
    return eSIR_LINK_MAX;
  }
}

/*                                         */
WPT_STATIC WPT_INLINE
void
WDI_CopyWDIStaCtxToHALStaCtx
(
  tConfigStaParams*          phalConfigSta,
  WDI_ConfigStaReqInfoType*  pwdiConfigSta
)
{
   wpt_uint8 i;
#ifdef WLAN_FEATURE_11AC
   /*                           */
   tConfigStaParams_V1* phalConfigSta_V1 = NULL;
   if (WDI_getFwWlanFeatCaps(DOT11AC))
   {
	   phalConfigSta_V1 = (tConfigStaParams_V1*)phalConfigSta;
   }
#endif
   /*                                                                          
                                  */

  wpalMemoryCopy(phalConfigSta->bssId,
                  pwdiConfigSta->macBSSID, WDI_MAC_ADDR_LEN);

  wpalMemoryCopy(phalConfigSta->staMac,
                  pwdiConfigSta->macSTA, WDI_MAC_ADDR_LEN);

  phalConfigSta->assocId                 = pwdiConfigSta->usAssocId;
  phalConfigSta->staType                 = pwdiConfigSta->wdiSTAType;
  phalConfigSta->shortPreambleSupported  = pwdiConfigSta->ucShortPreambleSupported;
  phalConfigSta->listenInterval          = pwdiConfigSta->usListenInterval;
  phalConfigSta->wmmEnabled              = pwdiConfigSta->ucWMMEnabled;
  phalConfigSta->htCapable               = pwdiConfigSta->ucHTCapable;
  phalConfigSta->txChannelWidthSet       = pwdiConfigSta->ucTXChannelWidthSet;
  phalConfigSta->rifsMode                = pwdiConfigSta->ucRIFSMode;
  phalConfigSta->lsigTxopProtection      = pwdiConfigSta->ucLSIGTxopProtection;
  phalConfigSta->maxAmpduSize            = pwdiConfigSta->ucMaxAmpduSize;
  phalConfigSta->maxAmpduDensity         = pwdiConfigSta->ucMaxAmpduDensity;
  phalConfigSta->maxAmsduSize            = pwdiConfigSta->ucMaxAmsduSize;
  phalConfigSta->fShortGI40Mhz           = pwdiConfigSta->ucShortGI40Mhz;
  phalConfigSta->fShortGI20Mhz           = pwdiConfigSta->ucShortGI20Mhz;
  phalConfigSta->rmfEnabled              = pwdiConfigSta->ucRMFEnabled;
  phalConfigSta->action                  = pwdiConfigSta->wdiAction;
  phalConfigSta->uAPSD                   = pwdiConfigSta->ucAPSD;
  phalConfigSta->maxSPLen                = pwdiConfigSta->ucMaxSPLen;
  phalConfigSta->greenFieldCapable       = pwdiConfigSta->ucGreenFieldCapable;
  phalConfigSta->delayedBASupport        = pwdiConfigSta->ucDelayedBASupport;
  phalConfigSta->us32MaxAmpduDuration    = pwdiConfigSta->us32MaxAmpduDuratio;
  phalConfigSta->fDsssCckMode40Mhz       = pwdiConfigSta->ucDsssCckMode40Mhz;
  phalConfigSta->encryptType             = pwdiConfigSta->ucEncryptType;

  phalConfigSta->mimoPS = WDI_2_HAL_MIMO_PS(pwdiConfigSta->wdiMIMOPS);

  phalConfigSta->supportedRates.opRateMode =
                          pwdiConfigSta->wdiSupportedRates.opRateMode;
  for(i = 0; i < SIR_NUM_11B_RATES; i ++)
  {
     phalConfigSta->supportedRates.llbRates[i] =
                          pwdiConfigSta->wdiSupportedRates.llbRates[i];
  }
  for(i = 0; i < SIR_NUM_11A_RATES; i ++)
  {
     phalConfigSta->supportedRates.llaRates[i] =
                          pwdiConfigSta->wdiSupportedRates.llaRates[i];
  }
  for(i = 0; i < SIR_NUM_POLARIS_RATES; i ++)
  {
     phalConfigSta->supportedRates.aniLegacyRates[i] =
                          pwdiConfigSta->wdiSupportedRates.aLegacyRates[i];
  }
  phalConfigSta->supportedRates.aniEnhancedRateBitmap =
                          pwdiConfigSta->wdiSupportedRates.uEnhancedRateBitmap;
  for(i = 0; i < SIR_MAC_MAX_SUPPORTED_MCS_SET; i ++)
  {
     phalConfigSta->supportedRates.supportedMCSSet[i] =
                          pwdiConfigSta->wdiSupportedRates.aSupportedMCSSet[i];
  }
  phalConfigSta->supportedRates.rxHighestDataRate =
                          pwdiConfigSta->wdiSupportedRates.aRxHighestDataRate;

#ifdef WLAN_FEATURE_11AC
  if(phalConfigSta_V1 != NULL)
  {
	  phalConfigSta_V1->supportedRates.vhtRxMCSMap = pwdiConfigSta->wdiSupportedRates.vhtRxMCSMap;
	  phalConfigSta_V1->supportedRates.vhtRxHighestDataRate = pwdiConfigSta->wdiSupportedRates.vhtRxHighestDataRate;
	  phalConfigSta_V1->supportedRates.vhtTxMCSMap = pwdiConfigSta->wdiSupportedRates.vhtTxMCSMap;
	  phalConfigSta_V1->supportedRates.vhtTxHighestDataRate = pwdiConfigSta->wdiSupportedRates.vhtTxHighestDataRate;
  }
#endif

#ifdef WLAN_FEATURE_P2P
  phalConfigSta->p2pCapableSta = pwdiConfigSta->ucP2pCapableSta ;
#endif

#ifdef WLAN_FEATURE_11AC
  if(phalConfigSta_V1 != NULL)
  {
	  phalConfigSta_V1->vhtCapable = pwdiConfigSta->ucVhtCapableSta;
	  phalConfigSta_V1->vhtTxChannelWidthSet = pwdiConfigSta->ucVhtTxChannelWidthSet;
  }
#endif
}/*                            */;

/*                                           */
WPT_STATIC WPT_INLINE void
WDI_CopyWDIRateSetToHALRateSet
(
  tSirMacRateSet* pHalRateSet,
  WDI_RateSet*    pwdiRateSet
)
{
  wpt_uint8 i;
  /*                                                                        */

  pHalRateSet->numRates = ( pwdiRateSet->ucNumRates <= SIR_MAC_RATESET_EID_MAX )?
                            pwdiRateSet->ucNumRates:SIR_MAC_RATESET_EID_MAX;

  for ( i = 0; i < pHalRateSet->numRates; i++ )
  {
    pHalRateSet->rate[i] = pwdiRateSet->aRates[i];
  }

}/*                              */


/*                                                    */
WPT_STATIC WPT_INLINE void
WDI_CopyWDIEDCAParamsToHALEDCAParams
(
  tSirMacEdcaParamRecord* phalEdcaParam,
  WDI_EdcaParamRecord*    pWDIEdcaParam
)
{
  /*                                                                          
                                  */

  phalEdcaParam->aci.rsvd   = pWDIEdcaParam->wdiACI.rsvd;
  phalEdcaParam->aci.aci    = pWDIEdcaParam->wdiACI.aci;
  phalEdcaParam->aci.acm    = pWDIEdcaParam->wdiACI.acm;
  phalEdcaParam->aci.aifsn  = pWDIEdcaParam->wdiACI.aifsn;

  phalEdcaParam->cw.max     = pWDIEdcaParam->wdiCW.max;
  phalEdcaParam->cw.min     = pWDIEdcaParam->wdiCW.min;
  phalEdcaParam->txoplimit  = pWDIEdcaParam->usTXOPLimit;
}/*                                    */


/*                                                        */
WPT_STATIC WPT_INLINE void
WDI_CopyWDIMgmFrameHdrToHALMgmFrameHdr
(
  tSirMacMgmtHdr* pmacMgmtHdr,
  WDI_MacMgmtHdr* pwdiMacMgmtHdr
)
{
  pmacMgmtHdr->fc.protVer    =  pwdiMacMgmtHdr->fc.protVer;
  pmacMgmtHdr->fc.type       =  pwdiMacMgmtHdr->fc.type;
  pmacMgmtHdr->fc.subType    =  pwdiMacMgmtHdr->fc.subType;
  pmacMgmtHdr->fc.toDS       =  pwdiMacMgmtHdr->fc.toDS;
  pmacMgmtHdr->fc.fromDS     =  pwdiMacMgmtHdr->fc.fromDS;
  pmacMgmtHdr->fc.moreFrag   =  pwdiMacMgmtHdr->fc.moreFrag;
  pmacMgmtHdr->fc.retry      =  pwdiMacMgmtHdr->fc.retry;
  pmacMgmtHdr->fc.powerMgmt  =  pwdiMacMgmtHdr->fc.powerMgmt;
  pmacMgmtHdr->fc.moreData   =  pwdiMacMgmtHdr->fc.moreData;
  pmacMgmtHdr->fc.wep        =  pwdiMacMgmtHdr->fc.wep;
  pmacMgmtHdr->fc.order      =  pwdiMacMgmtHdr->fc.order;

  pmacMgmtHdr->durationLo =  pwdiMacMgmtHdr->durationLo;
  pmacMgmtHdr->durationHi =  pwdiMacMgmtHdr->durationHi;

  wpalMemoryCopy(pmacMgmtHdr->da,
                 pwdiMacMgmtHdr->da, 6);
  wpalMemoryCopy(pmacMgmtHdr->sa,
                 pwdiMacMgmtHdr->sa, 6);
  wpalMemoryCopy(pmacMgmtHdr->bssId,
                 pwdiMacMgmtHdr->bssId, 6);

  pmacMgmtHdr->seqControl.fragNum  =  pwdiMacMgmtHdr->seqControl.fragNum;
  pmacMgmtHdr->seqControl.seqNumLo =  pwdiMacMgmtHdr->seqControl.seqNumLo;
  pmacMgmtHdr->seqControl.seqNumHi =  pwdiMacMgmtHdr->seqControl.seqNumHi;

}/*                                      */


/*                                                    */
WPT_STATIC WPT_INLINE void
WDI_CopyWDIConfigBSSToHALConfigBSS
(
  tConfigBssParams*         phalConfigBSS,
  WDI_ConfigBSSReqInfoType* pwdiConfigBSS
)
{

  wpt_uint8 keyIndex = 0;
#ifdef WLAN_FEATURE_11AC
  /*                           */
  tConfigBssParams_V1* phalConfigBSS_V1 = NULL;
  if (WDI_getFwWlanFeatCaps(DOT11AC))
	  phalConfigBSS_V1 = (tConfigBssParams_V1*)phalConfigBSS;
#endif

  wpalMemoryCopy( phalConfigBSS->bssId,
                  pwdiConfigBSS->macBSSID,
                  WDI_MAC_ADDR_LEN);

#ifdef HAL_SELF_STA_PER_BSS
  wpalMemoryCopy( phalConfigBSS->selfMacAddr,
                  pwdiConfigBSS->macSelfAddr,
                  WDI_MAC_ADDR_LEN);
#endif

  phalConfigBSS->bssType  = WDI_2_HAL_BSS_TYPE(pwdiConfigBSS->wdiBSSType);

  phalConfigBSS->operMode = pwdiConfigBSS->ucOperMode;
  phalConfigBSS->nwType   = WDI_2_HAL_NW_TYPE(pwdiConfigBSS->wdiNWType);

  phalConfigBSS->shortSlotTimeSupported =
     pwdiConfigBSS->ucShortSlotTimeSupported;
  phalConfigBSS->llaCoexist         = pwdiConfigBSS->ucllaCoexist;
  phalConfigBSS->llbCoexist         = pwdiConfigBSS->ucllbCoexist;
  phalConfigBSS->llgCoexist         = pwdiConfigBSS->ucllgCoexist;
  phalConfigBSS->ht20Coexist        = pwdiConfigBSS->ucHT20Coexist;
  phalConfigBSS->llnNonGFCoexist    = pwdiConfigBSS->ucllnNonGFCoexist;
  phalConfigBSS->fLsigTXOPProtectionFullSupport =
    pwdiConfigBSS->ucTXOPProtectionFullSupport;
  phalConfigBSS->fRIFSMode          = pwdiConfigBSS->ucRIFSMode;
  phalConfigBSS->beaconInterval     = pwdiConfigBSS->usBeaconInterval;
  phalConfigBSS->dtimPeriod         = pwdiConfigBSS->ucDTIMPeriod;
  phalConfigBSS->txChannelWidthSet  = pwdiConfigBSS->ucTXChannelWidthSet;
  phalConfigBSS->currentOperChannel = pwdiConfigBSS->ucCurrentOperChannel;
  phalConfigBSS->currentExtChannel  = pwdiConfigBSS->ucCurrentExtChannel;
  phalConfigBSS->action             = pwdiConfigBSS->wdiAction;
  phalConfigBSS->htCapable          = pwdiConfigBSS->ucHTCapable;
  phalConfigBSS->obssProtEnabled    = pwdiConfigBSS->ucObssProtEnabled;
  phalConfigBSS->rmfEnabled         = pwdiConfigBSS->ucRMFEnabled;

  phalConfigBSS->htOperMode =
    WDI_2_HAL_HT_OPER_MODE(pwdiConfigBSS->wdiHTOperMod);

  phalConfigBSS->dualCTSProtection        = pwdiConfigBSS->ucDualCTSProtection;
  phalConfigBSS->ucMaxProbeRespRetryLimit = pwdiConfigBSS->ucMaxProbeRespRetryLimit;
  phalConfigBSS->bHiddenSSIDEn            = pwdiConfigBSS->bHiddenSSIDEn;
  phalConfigBSS->bProxyProbeRespEn        = pwdiConfigBSS->bProxyProbeRespEn;

#ifdef WLAN_FEATURE_VOWIFI
  phalConfigBSS->maxTxPower               = pwdiConfigBSS->cMaxTxPower;
#endif

  /*                                                                             
                                         */
  phalConfigBSS->ssId.length =
    (pwdiConfigBSS->wdiSSID.ucLength <= 32)?
    pwdiConfigBSS->wdiSSID.ucLength : 32;
  wpalMemoryCopy(phalConfigBSS->ssId.ssId,
                 pwdiConfigBSS->wdiSSID.sSSID,
                 phalConfigBSS->ssId.length);

  WDI_CopyWDIStaCtxToHALStaCtx( &phalConfigBSS->staContext,
                                &pwdiConfigBSS->wdiSTAContext);

  WDI_CopyWDIRateSetToHALRateSet( &phalConfigBSS->rateSet,
                                  &pwdiConfigBSS->wdiRateSet);

  phalConfigBSS->edcaParamsValid = pwdiConfigBSS->ucEDCAParamsValid;

  if(phalConfigBSS->edcaParamsValid)
  {
     WDI_CopyWDIEDCAParamsToHALEDCAParams( &phalConfigBSS->acbe,
                                        &pwdiConfigBSS->wdiBEEDCAParams);
     WDI_CopyWDIEDCAParamsToHALEDCAParams( &phalConfigBSS->acbk,
                                           &pwdiConfigBSS->wdiBKEDCAParams);
     WDI_CopyWDIEDCAParamsToHALEDCAParams( &phalConfigBSS->acvi,
                                           &pwdiConfigBSS->wdiVIEDCAParams);
     WDI_CopyWDIEDCAParamsToHALEDCAParams( &phalConfigBSS->acvo,
                                           &pwdiConfigBSS->wdiVOEDCAParams);
  }

  phalConfigBSS->halPersona = pwdiConfigBSS->ucPersona;

  phalConfigBSS->bSpectrumMgtEnable = pwdiConfigBSS->bSpectrumMgtEn;

#ifdef WLAN_FEATURE_VOWIFI_11R

  phalConfigBSS->extSetStaKeyParamValid =
     pwdiConfigBSS->bExtSetStaKeyParamValid;

  if( phalConfigBSS->extSetStaKeyParamValid )
  {
     /*                                                                       
                                                       
                                                                            */
     phalConfigBSS->extSetStaKeyParam.encType =
        WDI_2_HAL_ENC_TYPE (pwdiConfigBSS->wdiExtSetKeyParam.wdiEncType);

     phalConfigBSS->extSetStaKeyParam.wepType =
        WDI_2_HAL_WEP_TYPE (pwdiConfigBSS->wdiExtSetKeyParam.wdiWEPType );

     phalConfigBSS->extSetStaKeyParam.staIdx = pwdiConfigBSS->wdiExtSetKeyParam.ucSTAIdx;

     phalConfigBSS->extSetStaKeyParam.defWEPIdx = pwdiConfigBSS->wdiExtSetKeyParam.ucDefWEPIdx;

     phalConfigBSS->extSetStaKeyParam.singleTidRc = pwdiConfigBSS->wdiExtSetKeyParam.ucSingleTidRc;

#ifdef WLAN_SOFTAP_FEATURE
     for(keyIndex = 0; keyIndex < pwdiConfigBSS->wdiExtSetKeyParam.ucNumKeys ;
          keyIndex++)
     {
        phalConfigBSS->extSetStaKeyParam.key[keyIndex].keyId =
           pwdiConfigBSS->wdiExtSetKeyParam.wdiKey[keyIndex].keyId;
        phalConfigBSS->extSetStaKeyParam.key[keyIndex].unicast =
           pwdiConfigBSS->wdiExtSetKeyParam.wdiKey[keyIndex].unicast;
        phalConfigBSS->extSetStaKeyParam.key[keyIndex].keyDirection =
           pwdiConfigBSS->wdiExtSetKeyParam.wdiKey[keyIndex].keyDirection;
        wpalMemoryCopy(phalConfigBSS->extSetStaKeyParam.key[keyIndex].keyRsc,
                       pwdiConfigBSS->wdiExtSetKeyParam.wdiKey[keyIndex].keyRsc,
                       WDI_MAX_KEY_RSC_LEN);
        phalConfigBSS->extSetStaKeyParam.key[keyIndex].paeRole =
           pwdiConfigBSS->wdiExtSetKeyParam.wdiKey[keyIndex].paeRole;
        phalConfigBSS->extSetStaKeyParam.key[keyIndex].keyLength =
           pwdiConfigBSS->wdiExtSetKeyParam.wdiKey[keyIndex].keyLength;
        wpalMemoryCopy(phalConfigBSS->extSetStaKeyParam.key[keyIndex].key,
                       pwdiConfigBSS->wdiExtSetKeyParam.wdiKey[keyIndex].key,
                       WDI_MAX_KEY_LENGTH);
     }
#else
     phalConfigBSS->extSetStaKeyParam.key.keyId =
        pwdiConfigBSS->wdiExtSetKeyParam.wdiKey[0].keyId;
     phalConfigBSS->extSetStaKeyParam.key.unicast =
        pwdiConfigBSS->wdiExtSetKeyParam.wdiKey[0].unicast;
     phalConfigBSS->extSetStaKeyParam.key.keyDirection =
        pwdiConfigBSS->wdiExtSetKeyParam.wdiKey[0].keyDirection;
     wpalMemoryCopy(phalConfigBSS->extSetStaKeyParam.key.keyRsc,
                    pwdiConfigBSS->wdiExtSetKeyParam.wdiKey[0].keyRsc,
                    WDI_MAX_KEY_RSC_LEN);
     phalConfigBSS->extSetStaKeyParam.key.paeRole =
        pwdiConfigBSS->wdiExtSetKeyParam.wdiKey[0].paeRole;
     phalConfigBSS->extSetStaKeyParam.key.keyLength =
        pwdiConfigBSS->wdiExtSetKeyParam.wdiKey[0].keyLength;
     wpalMemoryCopy(phalConfigBSS->extSetStaKeyParam.key.key,
                    pwdiConfigBSS->wdiExtSetKeyParam.wdiKey[0].key,
                    WDI_MAX_KEY_LENGTH);
#endif
  }
  else/*                                                  */
  {
     wpalMemoryZero( &phalConfigBSS->extSetStaKeyParam,
                     sizeof(phalConfigBSS->extSetStaKeyParam) );
  }

#endif /*                       */

#ifdef WLAN_FEATURE_11AC
  if(phalConfigBSS_V1 != NULL)
  {
      phalConfigBSS_V1->vhtCapable = pwdiConfigBSS->ucVhtCapableSta;
      phalConfigBSS_V1->vhtTxChannelWidthSet = pwdiConfigBSS->ucVhtTxChannelWidthSet;
  }
#endif

}/*                                  */


/*                                                                      
                          */
WPT_STATIC WPT_INLINE void
WDI_ExtractRequestCBFromEvent
(
  WDI_EventInfoType* pEvent,
  WDI_ReqStatusCb*   ppfnReqCB,
  void**             ppUserData
)
{
  /*                                                                  */
  switch ( pEvent->wdiRequest )
  {
  case WDI_START_REQ:
    *ppfnReqCB   =  ((WDI_StartReqParamsType*)pEvent->pEventData)->wdiReqStatusCB;
    *ppUserData  =  ((WDI_StartReqParamsType*)pEvent->pEventData)->pUserData;
    break;
  case WDI_STOP_REQ:
    *ppfnReqCB   =  ((WDI_StopReqParamsType*)pEvent->pEventData)->wdiReqStatusCB;
    *ppUserData  =  ((WDI_StopReqParamsType*)pEvent->pEventData)->pUserData;
    break;
  case WDI_INIT_SCAN_REQ:
    *ppfnReqCB   =  ((WDI_InitScanReqParamsType*)pEvent->pEventData)->wdiReqStatusCB;
    *ppUserData  =  ((WDI_InitScanReqParamsType*)pEvent->pEventData)->pUserData;
    break;
  case WDI_START_SCAN_REQ:
    *ppfnReqCB   =  ((WDI_StartScanReqParamsType*)pEvent->pEventData)->wdiReqStatusCB;
    *ppUserData  =  ((WDI_StartScanReqParamsType*)pEvent->pEventData)->pUserData;
    break;
  case WDI_END_SCAN_REQ:
    *ppfnReqCB   =  ((WDI_EndScanReqParamsType*)pEvent->pEventData)->wdiReqStatusCB;
    *ppUserData  =  ((WDI_EndScanReqParamsType*)pEvent->pEventData)->pUserData;
    break;
  case WDI_FINISH_SCAN_REQ:
    *ppfnReqCB   =  ((WDI_FinishScanReqParamsType*)pEvent->pEventData)->wdiReqStatusCB;
    *ppUserData  =  ((WDI_FinishScanReqParamsType*)pEvent->pEventData)->pUserData;
    break;
  case WDI_JOIN_REQ:
    *ppfnReqCB   =  ((WDI_JoinReqParamsType*)pEvent->pEventData)->wdiReqStatusCB;
    *ppUserData  =  ((WDI_JoinReqParamsType*)pEvent->pEventData)->pUserData;
    break;
  case WDI_CONFIG_BSS_REQ:
    *ppfnReqCB   =  ((WDI_ConfigBSSReqParamsType*)pEvent->pEventData)->wdiReqStatusCB;
    *ppUserData  =  ((WDI_ConfigBSSReqParamsType*)pEvent->pEventData)->pUserData;
    break;
  case WDI_DEL_BSS_REQ:
    *ppfnReqCB   =  ((WDI_DelBSSReqParamsType*)pEvent->pEventData)->wdiReqStatusCB;
    *ppUserData  =  ((WDI_DelBSSReqParamsType*)pEvent->pEventData)->pUserData;
    break;
  case WDI_POST_ASSOC_REQ:
    *ppfnReqCB   =  ((WDI_PostAssocReqParamsType*)pEvent->pEventData)->wdiReqStatusCB;
    *ppUserData  =  ((WDI_PostAssocReqParamsType*)pEvent->pEventData)->pUserData;
    break;
  case WDI_DEL_STA_REQ:
    *ppfnReqCB   =  ((WDI_DelSTAReqParamsType*)pEvent->pEventData)->wdiReqStatusCB;
    *ppUserData  =  ((WDI_DelSTAReqParamsType*)pEvent->pEventData)->pUserData;
    break;
  case WDI_DEL_STA_SELF_REQ:
    *ppfnReqCB   =  ((WDI_DelSTASelfReqParamsType*)pEvent->pEventData)->wdiReqStatusCB;
    *ppUserData  =  ((WDI_DelSTASelfReqParamsType*)pEvent->pEventData)->pUserData;
    break;

  case WDI_SET_BSS_KEY_REQ:
    *ppfnReqCB   =  ((WDI_SetBSSKeyReqParamsType*)pEvent->pEventData)->wdiReqStatusCB;
    *ppUserData  =  ((WDI_SetBSSKeyReqParamsType*)pEvent->pEventData)->pUserData;
    break;
  case WDI_RMV_BSS_KEY_REQ:
    *ppfnReqCB   =  ((WDI_RemoveBSSKeyReqParamsType*)pEvent->pEventData)->wdiReqStatusCB;
    *ppUserData  =  ((WDI_RemoveBSSKeyReqParamsType*)pEvent->pEventData)->pUserData;
    break;
  case WDI_SET_STA_KEY_REQ:
    *ppfnReqCB   =  ((WDI_SetSTAKeyReqParamsType*)pEvent->pEventData)->wdiReqStatusCB;
    *ppUserData  =  ((WDI_SetSTAKeyReqParamsType*)pEvent->pEventData)->pUserData;
    break;
  case WDI_RMV_STA_KEY_REQ:
    *ppfnReqCB   =  ((WDI_RemoveSTAKeyReqParamsType*)pEvent->pEventData)->wdiReqStatusCB;
    *ppUserData  =  ((WDI_RemoveSTAKeyReqParamsType*)pEvent->pEventData)->pUserData;
    break;
  case WDI_ADD_TS_REQ:
    *ppfnReqCB   =  ((WDI_AddTSReqParamsType*)pEvent->pEventData)->wdiReqStatusCB;
    *ppUserData  =  ((WDI_AddTSReqParamsType*)pEvent->pEventData)->pUserData;
    break;
  case WDI_DEL_TS_REQ:
    *ppfnReqCB   =  ((WDI_DelTSReqParamsType*)pEvent->pEventData)->wdiReqStatusCB;
    *ppUserData  =  ((WDI_DelTSReqParamsType*)pEvent->pEventData)->pUserData;
    break;
  case WDI_UPD_EDCA_PRMS_REQ:
    *ppfnReqCB   =  ((WDI_UpdateEDCAParamsType*)pEvent->pEventData)->wdiReqStatusCB;
    *ppUserData  =  ((WDI_UpdateEDCAParamsType*)pEvent->pEventData)->pUserData;
    break;
  case WDI_ADD_BA_SESSION_REQ:
    *ppfnReqCB   =  ((WDI_AddBASessionReqParamsType*)pEvent->pEventData)->wdiReqStatusCB;
    *ppUserData  =  ((WDI_AddBASessionReqParamsType*)pEvent->pEventData)->pUserData;
    break;
  case WDI_DEL_BA_REQ:
    *ppfnReqCB   =  ((WDI_DelBAReqParamsType*)pEvent->pEventData)->wdiReqStatusCB;
    *ppUserData  =  ((WDI_DelBAReqParamsType*)pEvent->pEventData)->pUserData;
    break;
#ifdef FEATURE_WLAN_CCX
   case WDI_TSM_STATS_REQ:
    *ppfnReqCB   =  ((WDI_TSMStatsReqParamsType*)pEvent->pEventData)->wdiReqStatusCB;
    *ppUserData  =  ((WDI_TSMStatsReqParamsType*)pEvent->pEventData)->pUserData;
     break;
#endif
  case WDI_CH_SWITCH_REQ:
    *ppfnReqCB   =  ((WDI_SwitchChReqParamsType*)pEvent->pEventData)->wdiReqStatusCB;
    *ppUserData  =  ((WDI_SwitchChReqParamsType*)pEvent->pEventData)->pUserData;
    break;
  case WDI_CONFIG_STA_REQ:
    *ppfnReqCB   =  ((WDI_ConfigSTAReqParamsType*)pEvent->pEventData)->wdiReqStatusCB;
    *ppUserData  =  ((WDI_ConfigSTAReqParamsType*)pEvent->pEventData)->pUserData;
    break;
  case WDI_SET_LINK_ST_REQ:
    *ppfnReqCB   =  ((WDI_SetLinkReqParamsType*)pEvent->pEventData)->wdiReqStatusCB;
    *ppUserData  =  ((WDI_SetLinkReqParamsType*)pEvent->pEventData)->pUserData;
    break;
  case WDI_GET_STATS_REQ:
    *ppfnReqCB   =  ((WDI_GetStatsReqParamsType*)pEvent->pEventData)->wdiReqStatusCB;
    *ppUserData  =  ((WDI_GetStatsReqParamsType*)pEvent->pEventData)->pUserData;
    break;
  case WDI_UPDATE_CFG_REQ:
    *ppfnReqCB   =  ((WDI_UpdateCfgReqParamsType*)pEvent->pEventData)->wdiReqStatusCB;
    *ppUserData  =  ((WDI_UpdateCfgReqParamsType*)pEvent->pEventData)->pUserData;
    break;
  case WDI_ADD_BA_REQ:
    *ppfnReqCB   =  ((WDI_AddBAReqParamsType*)pEvent->pEventData)->wdiReqStatusCB;
    *ppUserData  =  ((WDI_AddBAReqParamsType*)pEvent->pEventData)->pUserData;
    break;
  case WDI_TRIGGER_BA_REQ:
    *ppfnReqCB   =  ((WDI_TriggerBAReqParamsType*)pEvent->pEventData)->wdiReqStatusCB;
    *ppUserData  =  ((WDI_TriggerBAReqParamsType*)pEvent->pEventData)->pUserData;
    break;
  case  WDI_UPD_BCON_PRMS_REQ:
    *ppfnReqCB   =  ((WDI_UpdateBeaconParamsType*)pEvent->pEventData)->wdiReqStatusCB;
    *ppUserData  =  ((WDI_UpdateBeaconParamsType*)pEvent->pEventData)->pUserData;
     break;
  case  WDI_SND_BCON_REQ:
    *ppfnReqCB   =  ((WDI_SendBeaconParamsType*)pEvent->pEventData)->wdiReqStatusCB;
    *ppUserData  =  ((WDI_SendBeaconParamsType*)pEvent->pEventData)->pUserData;
     break;
  case  WDI_ENTER_BMPS_REQ:
    *ppfnReqCB   =  ((WDI_EnterBmpsReqParamsType*)pEvent->pEventData)->wdiReqStatusCB;
    *ppUserData  =  ((WDI_EnterBmpsReqParamsType*)pEvent->pEventData)->pUserData;
     break;
  case  WDI_EXIT_BMPS_REQ:
    *ppfnReqCB   =  ((WDI_ExitBmpsReqParamsType*)pEvent->pEventData)->wdiReqStatusCB;
    *ppUserData  =  ((WDI_ExitBmpsReqParamsType*)pEvent->pEventData)->pUserData;
     break;
  case  WDI_ENTER_UAPSD_REQ:
    *ppfnReqCB   =  ((WDI_EnterUapsdReqParamsType*)pEvent->pEventData)->wdiReqStatusCB;
    *ppUserData  =  ((WDI_EnterUapsdReqParamsType*)pEvent->pEventData)->pUserData;
     break;
  case  WDI_UPDATE_UAPSD_PARAM_REQ:
    *ppfnReqCB   =  ((WDI_UpdateUapsdReqParamsType*)pEvent->pEventData)->wdiReqStatusCB;
    *ppUserData  =  ((WDI_UpdateUapsdReqParamsType*)pEvent->pEventData)->pUserData;
     break;
  case  WDI_CONFIGURE_RXP_FILTER_REQ:
    *ppfnReqCB   =  ((WDI_ConfigureRxpFilterReqParamsType*)pEvent->pEventData)->wdiReqStatusCB;
    *ppUserData  =  ((WDI_ConfigureRxpFilterReqParamsType*)pEvent->pEventData)->pUserData;
     break;
  case  WDI_SET_BEACON_FILTER_REQ:
    *ppfnReqCB   =  ((WDI_BeaconFilterReqParamsType*)pEvent->pEventData)->wdiReqStatusCB;
    *ppUserData  =  ((WDI_BeaconFilterReqParamsType*)pEvent->pEventData)->pUserData;
     break;
  case  WDI_REM_BEACON_FILTER_REQ:
    *ppfnReqCB   =  ((WDI_RemBeaconFilterReqParamsType*)pEvent->pEventData)->wdiReqStatusCB;
    *ppUserData  =  ((WDI_RemBeaconFilterReqParamsType*)pEvent->pEventData)->pUserData;
     break;
  case  WDI_SET_RSSI_THRESHOLDS_REQ:
    *ppfnReqCB   =  ((WDI_SetRSSIThresholdsReqParamsType*)pEvent->pEventData)->wdiReqStatusCB;
    *ppUserData  =  ((WDI_SetRSSIThresholdsReqParamsType*)pEvent->pEventData)->pUserData;
     break;
  case  WDI_HOST_OFFLOAD_REQ:
    *ppfnReqCB   =  ((WDI_HostOffloadReqParamsType*)pEvent->pEventData)->wdiReqStatusCB;
    *ppUserData  =  ((WDI_HostOffloadReqParamsType*)pEvent->pEventData)->pUserData;
     break;
  case  WDI_WOWL_ADD_BC_PTRN_REQ:
    *ppfnReqCB   =  ((WDI_WowlAddBcPtrnReqParamsType*)pEvent->pEventData)->wdiReqStatusCB;
    *ppUserData  =  ((WDI_WowlAddBcPtrnReqParamsType*)pEvent->pEventData)->pUserData;
     break;
  case  WDI_WOWL_DEL_BC_PTRN_REQ:
    *ppfnReqCB   =  ((WDI_WowlDelBcPtrnReqParamsType*)pEvent->pEventData)->wdiReqStatusCB;
    *ppUserData  =  ((WDI_WowlDelBcPtrnReqParamsType*)pEvent->pEventData)->pUserData;
     break;
  case  WDI_WOWL_ENTER_REQ:
    *ppfnReqCB   =  ((WDI_WowlEnterReqParamsType*)pEvent->pEventData)->wdiReqStatusCB;
    *ppUserData  =  ((WDI_WowlEnterReqParamsType*)pEvent->pEventData)->pUserData;
     break;
  case  WDI_CONFIGURE_APPS_CPU_WAKEUP_STATE_REQ:
    *ppfnReqCB   =  ((WDI_ConfigureAppsCpuWakeupStateReqParamsType*)pEvent->pEventData)->wdiReqStatusCB;
    *ppUserData  =  ((WDI_ConfigureAppsCpuWakeupStateReqParamsType*)pEvent->pEventData)->pUserData;
     break;
  case WDI_FLUSH_AC_REQ:
    *ppfnReqCB   =  ((WDI_FlushAcReqParamsType*)pEvent->pEventData)->wdiReqStatusCB;
    *ppUserData  =  ((WDI_FlushAcReqParamsType*)pEvent->pEventData)->pUserData;
    break;
  case WDI_BTAMP_EVENT_REQ:
    *ppfnReqCB   =  ((WDI_BtAmpEventParamsType*)pEvent->pEventData)->wdiReqStatusCB;
    *ppUserData  =  ((WDI_BtAmpEventParamsType*)pEvent->pEventData)->pUserData;
    break;
  case WDI_KEEP_ALIVE_REQ:
    *ppfnReqCB   =  ((WDI_KeepAliveReqParamsType*)pEvent->pEventData)->wdiReqStatusCB;
    *ppUserData  =  ((WDI_KeepAliveReqParamsType*)pEvent->pEventData)->pUserData;
    break;
  case WDI_SET_TX_PER_TRACKING_REQ:
    *ppfnReqCB   =  ((WDI_SetTxPerTrackingReqParamsType*)pEvent->pEventData)->wdiReqStatusCB;
    *ppUserData  =  ((WDI_SetTxPerTrackingReqParamsType*)pEvent->pEventData)->pUserData;
  default:
    *ppfnReqCB   =  NULL;
    *ppUserData  =  NULL;
      break;
  }
}/*                             */


/* 
                                                             
                                                  

                                        

                          

                      
                                    
*/
wpt_boolean
WDI_IsHwFrameTxTranslationCapable
(
  wpt_uint8 uSTAIdx
)
{
  /*                                                                       
             */
  /*                                                                        
                
                                                                          */
  if ( eWLAN_PAL_FALSE == gWDIInitialized )
  {
    WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_ERROR,
              "WDI API call before module is initialized - Fail request");

    return WDI_STATUS_E_NOT_ALLOWED;
  }


  return gWDICb.bFrameTransEnabled;
}/*                                 */

#ifdef FEATURE_WLAN_SCAN_PNO
/* 
                                   

                                                      
                                             

                                                            
                                                  
              

                                                         
                

                                    
*/
WDI_Status
WDI_SetPreferredNetworkReq
(
  WDI_PNOScanReqParamsType* pwdiPNOScanReqParams,
  WDI_PNOScanCb             wdiPNOScanCb,
  void*                      pUserData
)
{
   WDI_EventInfoType      wdiEventData = {{0}};
   /*                                                                        */

   /*                                                                        
                 
                                                                           */
   if ( eWLAN_PAL_FALSE == gWDIInitialized )
   {
     WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_ERROR,
               "WDI API call before module is initialized - Fail request");

     return WDI_STATUS_E_NOT_ALLOWED;
   }

   /*                                                                        
                                                
                                                                           */
   wdiEventData.wdiRequest      = WDI_SET_PREF_NETWORK_REQ;
   wdiEventData.pEventData      = pwdiPNOScanReqParams;
   wdiEventData.uEventDataSize  = sizeof(*pwdiPNOScanReqParams);
   wdiEventData.pCBfnc          = wdiPNOScanCb;
   wdiEventData.pUserData       = pUserData;

   return WDI_PostMainEvent(&gWDICb, WDI_REQUEST_EVENT, &wdiEventData);
}


/* 
                            

                                                       
                                                       

                                                               
                                                          
              

                                                         
                

                                    
*/
WDI_Status
WDI_SetRssiFilterReq
(
  WDI_SetRssiFilterReqParamsType* pwdiRssiFilterReqParams,
  WDI_RssiFilterCb                wdiRssiFilterCb,
  void*                           pUserData
)
{
   WDI_EventInfoType      wdiEventData = {{0}};
   /*                                                                        */

   /*                                                                        
                 
                                                                           */
   if ( eWLAN_PAL_FALSE == gWDIInitialized )
   {
     WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_ERROR,
               "WDI API call before module is initialized - Fail request");

     return WDI_STATUS_E_NOT_ALLOWED;
   }

   /*                                                                        
                                                
                                                                           */
   wdiEventData.wdiRequest      = WDI_SET_RSSI_FILTER_REQ;
   wdiEventData.pEventData      = pwdiRssiFilterReqParams;
   wdiEventData.uEventDataSize  = sizeof(*pwdiRssiFilterReqParams);
   wdiEventData.pCBfnc          = wdiRssiFilterCb;
   wdiEventData.pUserData       = pUserData;

   return WDI_PostMainEvent(&gWDICb, WDI_REQUEST_EVENT, &wdiEventData);
}/*                    */

/* 
                               

                                                                         
                                             

                                                                     
                                                  
              

                                                         
                

                                    
*/
WDI_Status
WDI_UpdateScanParamsReq
(
  WDI_UpdateScanParamsInfoType* pwdiUpdateScanParamsInfoType,
  WDI_UpdateScanParamsCb        wdiUpdateScanParamsCb,
  void*                         pUserData
)
{
   WDI_EventInfoType      wdiEventData = {{0}};
   /*                                                                        */

   /*                                                                        
                 
                                                                           */
   if ( eWLAN_PAL_FALSE == gWDIInitialized )
   {
     WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_ERROR,
               "WDI API call before module is initialized - Fail request");

     return WDI_STATUS_E_NOT_ALLOWED;
   }

   /*                                                                        
                                                
                                                                           */
   wdiEventData.wdiRequest      = WDI_UPDATE_SCAN_PARAMS_REQ;
   wdiEventData.pEventData      = pwdiUpdateScanParamsInfoType;
   wdiEventData.uEventDataSize  = sizeof(*pwdiUpdateScanParamsInfoType);
   wdiEventData.pCBfnc          = wdiUpdateScanParamsCb;
   wdiEventData.pUserData       = pUserData;

   return WDI_PostMainEvent(&gWDICb, WDI_REQUEST_EVENT, &wdiEventData);
}

/* 
                                                          
                          

                                                         
                                                                
                          
                                                                
                     

                                    
*/

WDI_Status
WDI_PackPreferredNetworkList
(
  WDI_ControlBlockType*      pWDICtx,
  WDI_PNOScanReqParamsType*  pwdiPNOScanReqParams,
  wpt_uint8**                ppSendBuffer,
  wpt_uint16*                pSize
)
{
   wpt_uint8*                 pSendBuffer           = NULL;
   wpt_uint16                 usDataOffset          = 0;
   wpt_uint16                 usSendSize            = 0;
   tpPrefNetwListParams       pPrefNetwListParams   = NULL;
   wpt_uint8 i;
   /*                                                                       
                       
                                                                          */
   if (( WDI_STATUS_SUCCESS != WDI_GetMessageBuffer( pWDICtx, WDI_SET_PREF_NETWORK_REQ,
                         sizeof(tPrefNetwListParams),
                         &pSendBuffer, &usDataOffset, &usSendSize))||
       ( usSendSize < (usDataOffset + sizeof(tPrefNetwListParams) )))
   {
      WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_WARN,
                  "Unable to get send buffer in Set PNO req %x ",
                   pwdiPNOScanReqParams);
      WDI_ASSERT(0);
      return WDI_STATUS_E_FAILURE;
   }

   pPrefNetwListParams = (tpPrefNetwListParams)(pSendBuffer + usDataOffset);

   /*                                                                         
                                                                      
                                                                            */
   pPrefNetwListParams->enable  = 
     pwdiPNOScanReqParams->wdiPNOScanInfo.bEnable;
   pPrefNetwListParams->modePNO = 
     pwdiPNOScanReqParams->wdiPNOScanInfo.wdiModePNO;

   pPrefNetwListParams->ucNetworksCount = 
     (pwdiPNOScanReqParams->wdiPNOScanInfo.ucNetworksCount <
      WLAN_HAL_PNO_MAX_SUPP_NETWORKS)?
     pwdiPNOScanReqParams->wdiPNOScanInfo.ucNetworksCount :
      WLAN_HAL_PNO_MAX_SUPP_NETWORKS;

   WPAL_TRACE( eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_INFO,
               "WDI SET PNO: Enable %d, Mode %d, Netw Count %d",
               pwdiPNOScanReqParams->wdiPNOScanInfo.bEnable,
               pwdiPNOScanReqParams->wdiPNOScanInfo.wdiModePNO,
               pwdiPNOScanReqParams->wdiPNOScanInfo.ucNetworksCount);

   for ( i = 0; i < pPrefNetwListParams->ucNetworksCount; i++ )
   {
     /*               */
     pPrefNetwListParams->aNetworks[i].ssId.length
        = pwdiPNOScanReqParams->wdiPNOScanInfo.aNetworks[i].ssId.ucLength;

     wpalMemoryCopy( pPrefNetwListParams->aNetworks[i].ssId.ssId,
          pwdiPNOScanReqParams->wdiPNOScanInfo.aNetworks[i].ssId.sSSID,
          pPrefNetwListParams->aNetworks[i].ssId.length);

     /*                                   */
     pPrefNetwListParams->aNetworks[i].authentication = 
       (tAuthType)pwdiPNOScanReqParams->wdiPNOScanInfo.aNetworks[i].wdiAuth;

     /*                               */
     pPrefNetwListParams->aNetworks[i].encryption = 
       (tEdType)pwdiPNOScanReqParams->wdiPNOScanInfo.aNetworks[i].wdiEncryption;

     /*                                                      
                           */
     pPrefNetwListParams->aNetworks[i].ucChannelCount =
       (pwdiPNOScanReqParams->wdiPNOScanInfo.aNetworks[i].ucChannelCount <
        WLAN_HAL_PNO_MAX_NETW_CHANNELS)?
       pwdiPNOScanReqParams->wdiPNOScanInfo.aNetworks[i].ucChannelCount :
        WLAN_HAL_PNO_MAX_NETW_CHANNELS;

     wpalMemoryCopy(pPrefNetwListParams->aNetworks[i].aChannels,
                    pwdiPNOScanReqParams->wdiPNOScanInfo.aNetworks[i].aChannels,
                    pPrefNetwListParams->aNetworks[i].ucChannelCount);

     /*                                                             */
     pPrefNetwListParams->aNetworks[i].rssiThreshold =
       pwdiPNOScanReqParams->wdiPNOScanInfo.aNetworks[i].rssiThreshold;

     WPAL_TRACE( eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_ERROR,
               "WDI SET PNO: SSID %d %s",
               pPrefNetwListParams->aNetworks[i].ssId.length,
               pPrefNetwListParams->aNetworks[i].ssId.ssId);
   }

   pPrefNetwListParams->scanTimers.ucScanTimersCount = 
     (pwdiPNOScanReqParams->wdiPNOScanInfo.scanTimers.ucScanTimersCount <
      WLAN_HAL_PNO_MAX_SCAN_TIMERS)?
     pwdiPNOScanReqParams->wdiPNOScanInfo.scanTimers.ucScanTimersCount :
      WLAN_HAL_PNO_MAX_SCAN_TIMERS;

   WPAL_TRACE( eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_ERROR,
               "WDI SET PNO: Scan timers count %d 24G P %d 5G Probe %d",
               pPrefNetwListParams->scanTimers.ucScanTimersCount,
               pwdiPNOScanReqParams->wdiPNOScanInfo.us24GProbeSize,
               pwdiPNOScanReqParams->wdiPNOScanInfo.us5GProbeSize);

   for ( i = 0; i < pPrefNetwListParams->scanTimers.ucScanTimersCount; i++   )
   {
     pPrefNetwListParams->scanTimers.aTimerValues[i].uTimerValue  = 
       pwdiPNOScanReqParams->wdiPNOScanInfo.scanTimers.aTimerValues[i].uTimerValue;
     pPrefNetwListParams->scanTimers.aTimerValues[i].uTimerRepeat = 
       pwdiPNOScanReqParams->wdiPNOScanInfo.scanTimers.aTimerValues[i].uTimerRepeat;
   }

   /*                       */
   pPrefNetwListParams->us24GProbeSize = 
     (pwdiPNOScanReqParams->wdiPNOScanInfo.us24GProbeSize<
     WLAN_HAL_PNO_MAX_PROBE_SIZE)?
     pwdiPNOScanReqParams->wdiPNOScanInfo.us24GProbeSize:
     WLAN_HAL_PNO_MAX_PROBE_SIZE;

   wpalMemoryCopy(pPrefNetwListParams->a24GProbeTemplate, 
                  pwdiPNOScanReqParams->wdiPNOScanInfo.a24GProbeTemplate,
                  pPrefNetwListParams->us24GProbeSize); 

   pPrefNetwListParams->us5GProbeSize = 
     (pwdiPNOScanReqParams->wdiPNOScanInfo.us5GProbeSize <
     WLAN_HAL_PNO_MAX_PROBE_SIZE)?
     pwdiPNOScanReqParams->wdiPNOScanInfo.us5GProbeSize:
     WLAN_HAL_PNO_MAX_PROBE_SIZE;

   wpalMemoryCopy(pPrefNetwListParams->a5GProbeTemplate, 
                  pwdiPNOScanReqParams->wdiPNOScanInfo.a5GProbeTemplate,
                  pPrefNetwListParams->us5GProbeSize); 

   /*                     */
   *ppSendBuffer = pSendBuffer;
   *pSize        = usSendSize;

   return WDI_STATUS_SUCCESS;
}/*                            */

/* 
                                                          
                          

                                                         
                                                                
                          
                                                                
                     

                                    
*/

WDI_Status
WDI_PackPreferredNetworkListNew
(
  WDI_ControlBlockType*      pWDICtx,
  WDI_PNOScanReqParamsType*  pwdiPNOScanReqParams,
  wpt_uint8**                ppSendBuffer,
  wpt_uint16*                pSize
)
{
   wpt_uint8*                 pSendBuffer           = NULL;
   wpt_uint16                 usDataOffset          = 0;
   wpt_uint16                 usSendSize            = 0;
   tpPrefNetwListParamsNew    pPrefNetwListParams;
   wpt_uint8 i;

   /*                                                                       
                       
                                                                          */
   if (( WDI_STATUS_SUCCESS != WDI_GetMessageBuffer( pWDICtx, WDI_SET_PREF_NETWORK_REQ,
                         sizeof(tPrefNetwListParamsNew),
                         &pSendBuffer, &usDataOffset, &usSendSize))||
       ( usSendSize < (usDataOffset + sizeof(tPrefNetwListParamsNew) )))
   {
      WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_WARN,
                  "Unable to get send buffer in Set PNO req %x  ",
                   pwdiPNOScanReqParams);
      WDI_ASSERT(0);
      return WDI_STATUS_E_FAILURE;
   }

   pPrefNetwListParams = (tpPrefNetwListParamsNew)(pSendBuffer + usDataOffset);

   /*                                                                         
                                                                      
                                                                            */
   pPrefNetwListParams->enable  = 
     pwdiPNOScanReqParams->wdiPNOScanInfo.bEnable;
   pPrefNetwListParams->modePNO = 
     pwdiPNOScanReqParams->wdiPNOScanInfo.wdiModePNO;

   pPrefNetwListParams->ucNetworksCount = 
     (pwdiPNOScanReqParams->wdiPNOScanInfo.ucNetworksCount <
      WLAN_HAL_PNO_MAX_SUPP_NETWORKS)?
     pwdiPNOScanReqParams->wdiPNOScanInfo.ucNetworksCount :
      WLAN_HAL_PNO_MAX_SUPP_NETWORKS;

   WPAL_TRACE( eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_INFO,
               "WDI SET PNO: Enable %d, Mode %d, Netw Count %d",
               pwdiPNOScanReqParams->wdiPNOScanInfo.bEnable,
               pwdiPNOScanReqParams->wdiPNOScanInfo.wdiModePNO,
               pwdiPNOScanReqParams->wdiPNOScanInfo.ucNetworksCount);

   for ( i = 0; i < pPrefNetwListParams->ucNetworksCount; i++ )
   {
     /*               */
     pPrefNetwListParams->aNetworks[i].ssId.length
        = pwdiPNOScanReqParams->wdiPNOScanInfo.aNetworks[i].ssId.ucLength;

     wpalMemoryCopy( pPrefNetwListParams->aNetworks[i].ssId.ssId,
          pwdiPNOScanReqParams->wdiPNOScanInfo.aNetworks[i].ssId.sSSID,
          pPrefNetwListParams->aNetworks[i].ssId.length);

     /*                                   */
     pPrefNetwListParams->aNetworks[i].authentication = 
       (tAuthType)pwdiPNOScanReqParams->wdiPNOScanInfo.aNetworks[i].wdiAuth;

     /*                               */
     pPrefNetwListParams->aNetworks[i].encryption = 
       (tEdType)pwdiPNOScanReqParams->wdiPNOScanInfo.aNetworks[i].wdiEncryption;

     /*                               */
     pPrefNetwListParams->aNetworks[i].bcastNetworkType = 
       (tSSIDBcastType)pwdiPNOScanReqParams->wdiPNOScanInfo.aNetworks[i].wdiBcastNetworkType;

     /*                                                      
                           */
     pPrefNetwListParams->aNetworks[i].ucChannelCount = 
       pwdiPNOScanReqParams->wdiPNOScanInfo.aNetworks[i].ucChannelCount;

     wpalMemoryCopy(pPrefNetwListParams->aNetworks[i].aChannels,
                    pwdiPNOScanReqParams->wdiPNOScanInfo.aNetworks[i].aChannels,
                    pPrefNetwListParams->aNetworks[i].ucChannelCount);

     /*                                                             */
     pPrefNetwListParams->aNetworks[i].rssiThreshold =
       pwdiPNOScanReqParams->wdiPNOScanInfo.aNetworks[i].rssiThreshold;

     WPAL_TRACE( eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_INFO,
               "WDI SET PNO: SSID %d %s",
               pPrefNetwListParams->aNetworks[i].ssId.length,
               pPrefNetwListParams->aNetworks[i].ssId.ssId);
   }

   pPrefNetwListParams->scanTimers.ucScanTimersCount = 
     (pwdiPNOScanReqParams->wdiPNOScanInfo.scanTimers.ucScanTimersCount <
      WLAN_HAL_PNO_MAX_SCAN_TIMERS)?
     pwdiPNOScanReqParams->wdiPNOScanInfo.scanTimers.ucScanTimersCount :
      WLAN_HAL_PNO_MAX_SCAN_TIMERS;

   WPAL_TRACE( eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_INFO,
               "WDI SET PNO: Scan timers count %d 24G P %d 5G Probe %d",
               pPrefNetwListParams->scanTimers.ucScanTimersCount,
               pwdiPNOScanReqParams->wdiPNOScanInfo.us24GProbeSize,
               pwdiPNOScanReqParams->wdiPNOScanInfo.us5GProbeSize);

   for ( i = 0; i < pPrefNetwListParams->scanTimers.ucScanTimersCount; i++   )
   {
     pPrefNetwListParams->scanTimers.aTimerValues[i].uTimerValue  = 
       pwdiPNOScanReqParams->wdiPNOScanInfo.scanTimers.aTimerValues[i].uTimerValue;
     pPrefNetwListParams->scanTimers.aTimerValues[i].uTimerRepeat = 
       pwdiPNOScanReqParams->wdiPNOScanInfo.scanTimers.aTimerValues[i].uTimerRepeat;
   }

   /*                       */
   pPrefNetwListParams->us24GProbeSize = 
     (pwdiPNOScanReqParams->wdiPNOScanInfo.us24GProbeSize<
     WLAN_HAL_PNO_MAX_PROBE_SIZE)?
     pwdiPNOScanReqParams->wdiPNOScanInfo.us24GProbeSize:
     WLAN_HAL_PNO_MAX_PROBE_SIZE;

   wpalMemoryCopy(pPrefNetwListParams->a24GProbeTemplate, 
                  pwdiPNOScanReqParams->wdiPNOScanInfo.a24GProbeTemplate,
                  pPrefNetwListParams->us24GProbeSize); 

   pPrefNetwListParams->us5GProbeSize = 
     (pwdiPNOScanReqParams->wdiPNOScanInfo.us5GProbeSize <
     WLAN_HAL_PNO_MAX_PROBE_SIZE)?
     pwdiPNOScanReqParams->wdiPNOScanInfo.us5GProbeSize:
     WLAN_HAL_PNO_MAX_PROBE_SIZE;

   wpalMemoryCopy(pPrefNetwListParams->a5GProbeTemplate, 
                  pwdiPNOScanReqParams->wdiPNOScanInfo.a5GProbeTemplate,
                  pPrefNetwListParams->us5GProbeSize); 


   /*                     */
   *ppSendBuffer = pSendBuffer;
   *pSize        = usSendSize;

   return WDI_STATUS_SUCCESS;
}/*                               */

/* 
                                                           

                                                         
                                                                    

                                    
*/
WDI_Status
WDI_ProcessSetPreferredNetworkReq
(
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
)
{
   WDI_PNOScanReqParamsType*  pwdiPNOScanReqParams  = NULL;
   WDI_PNOScanCb              wdiPNOScanCb          = NULL;
   wpt_uint8*                 pSendBuffer           = NULL;
   wpt_uint16                 usSendSize            = 0;
   WDI_Status                 wdiStatus;

   /*                                                                         
                 
                                                                            */
   if (( NULL == pEventData ) ||
       ( NULL == (pwdiPNOScanReqParams = (WDI_PNOScanReqParamsType*)pEventData->pEventData)) ||
       ( NULL == (wdiPNOScanCb   = (WDI_PNOScanCb)pEventData->pCBfnc)))
   {
      WPAL_TRACE( eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_WARN,
                  "%s: Invalid parameters", __FUNCTION__);
      WDI_ASSERT(0);
      return WDI_STATUS_E_FAILURE;
   }

   /*                                                                         
                                                    
                                                                            */
   if ( pWDICtx->wdiPNOVersion > 0 )
   {
     WPAL_TRACE( eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_INFO,
                  "%s: PNO new version %d ", __FUNCTION__,
                  pWDICtx->wdiPNOVersion);

     wdiStatus = WDI_PackPreferredNetworkListNew( pWDICtx, pwdiPNOScanReqParams,
                                      &pSendBuffer, &usSendSize);
   }
   else
   {
     WPAL_TRACE( eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_INFO,
                  "%s: PNO old version %d ", __FUNCTION__,
                  pWDICtx->wdiPNOVersion);

     wdiStatus = WDI_PackPreferredNetworkList( pWDICtx, pwdiPNOScanReqParams,
                                      &pSendBuffer, &usSendSize);
   }

   if (( WDI_STATUS_SUCCESS != wdiStatus )||
       ( NULL == pSendBuffer )||( 0 == usSendSize ))
   {
      WPAL_TRACE( eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_WARN,
                  "%s: failed to pack request parameters", __FUNCTION__);
      WDI_ASSERT(0);
      return wdiStatus;
   }

   pWDICtx->wdiReqStatusCB     = pwdiPNOScanReqParams->wdiReqStatusCB;
   pWDICtx->pReqStatusUserData = pwdiPNOScanReqParams->pUserData;

   /*                                                                         
                                
                                                                            */
   return  WDI_SendMsg( pWDICtx, pSendBuffer, usSendSize,
               wdiPNOScanCb, pEventData->pUserData, WDI_SET_PREF_NETWORK_RESP);
}

/* 
                                                

                                                         
                                                                    

     
                                    
*/
WDI_Status
WDI_ProcessSetRssiFilterReq
(
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
)
{
   WDI_SetRssiFilterReqParamsType* pwdiRssiFilterReqParams = NULL;
   WDI_RssiFilterCb                wdiRssiFilterCb         = NULL;
   wpt_uint8*                      pSendBuffer             = NULL;
   wpt_uint16                      usDataOffset            = 0;
   wpt_uint16                      usSendSize              = 0;
   wpt_uint8                       ucRssiThreshold;

   /*                                                                         
                 
                                                                            */
   if (( NULL == pEventData ) ||
       ( NULL == (pwdiRssiFilterReqParams = (WDI_SetRssiFilterReqParamsType*)pEventData->pEventData)) ||
       ( NULL == (wdiRssiFilterCb   = (WDI_RssiFilterCb)pEventData->pCBfnc)))
   {
      WPAL_TRACE( eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_WARN,
                  "%s: Invalid parameters", __FUNCTION__);
      WDI_ASSERT(0);
      return WDI_STATUS_E_FAILURE;
   }

   /*                                                                       
                       
                                                                          */
   if (( WDI_STATUS_SUCCESS != WDI_GetMessageBuffer( pWDICtx, WDI_SET_PREF_NETWORK_REQ,
                         sizeof(ucRssiThreshold),
                         &pSendBuffer, &usDataOffset, &usSendSize))||
       ( usSendSize < (usDataOffset + sizeof(ucRssiThreshold) )))
   {
      WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_WARN,
                  "Unable to get send buffer in Set PNO req %x %x %x",
                  pEventData, pwdiRssiFilterReqParams, wdiRssiFilterCb);
      WDI_ASSERT(0);
      return WDI_STATUS_E_FAILURE;
   }

   ucRssiThreshold = pwdiRssiFilterReqParams->rssiThreshold;

   wpalMemoryCopy( pSendBuffer+usDataOffset,
                   &ucRssiThreshold,
                   sizeof(ucRssiThreshold));

   pWDICtx->wdiReqStatusCB     = pwdiRssiFilterReqParams->wdiReqStatusCB;
   pWDICtx->pReqStatusUserData = pwdiRssiFilterReqParams->pUserData;

   /*                                                                         
                                
                                                                            */
   return  WDI_SendMsg( pWDICtx, pSendBuffer, usSendSize,
                        wdiRssiFilterCb, pEventData->pUserData, WDI_SET_RSSI_FILTER_RESP);
}


/* 
                                           

                                                         
                                                                    

     
                                    
*/
WDI_Status
WDI_ProcessUpdateScanParamsReq
(
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
)
{
   WDI_UpdateScanParamsInfoType* pwdiUpdateScanParams  = NULL;
   WDI_UpdateScanParamsCb        wdiUpdateScanParamsCb = NULL;
   wpt_uint8*                    pSendBuffer           = NULL;
   wpt_uint16                    usDataOffset          = 0;
   wpt_uint16                    usSendSize            = 0;
   tUpdateScanParams             updateScanParams = {0};


   /*                                                                         
                 
                                                                            */
   if (( NULL == pEventData ) ||
       ( NULL == (pwdiUpdateScanParams = (WDI_UpdateScanParamsInfoType*)pEventData->pEventData)) ||
       ( NULL == (wdiUpdateScanParamsCb   = (WDI_UpdateScanParamsCb)pEventData->pCBfnc)))
   {
      WPAL_TRACE( eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_WARN,
                  "%s: Invalid parameters", __FUNCTION__);
      WDI_ASSERT(0);
      return WDI_STATUS_E_FAILURE;
   }

   WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_INFO,
               "Begin WDI Update Scan Parameters");
   /*                                                                       
                       
                                                                          */
   if (( WDI_STATUS_SUCCESS != WDI_GetMessageBuffer( pWDICtx, WDI_UPDATE_SCAN_PARAMS_REQ,
                         sizeof(updateScanParams),
                         &pSendBuffer, &usDataOffset, &usSendSize))||
       ( usSendSize < (usDataOffset + sizeof(updateScanParams) )))
   {
      WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_WARN,
                  "Unable to get send buffer in Update Scan Params req %x %x %x",
                  pEventData, pwdiUpdateScanParams, wdiUpdateScanParamsCb);
      WDI_ASSERT(0);
      return WDI_STATUS_E_FAILURE;
   }

   //
   //                                                                         
   //

   updateScanParams.b11dEnabled    = pwdiUpdateScanParams->wdiUpdateScanParamsInfo.b11dEnabled;
   updateScanParams.b11dResolved   = pwdiUpdateScanParams->wdiUpdateScanParamsInfo.b11dResolved;
   updateScanParams.ucChannelCount =
     (pwdiUpdateScanParams->wdiUpdateScanParamsInfo.ucChannelCount <
     WLAN_HAL_PNO_MAX_NETW_CHANNELS)?
     pwdiUpdateScanParams->wdiUpdateScanParamsInfo.ucChannelCount :
     WLAN_HAL_PNO_MAX_NETW_CHANNELS;

   wpalMemoryCopy( updateScanParams.aChannels,
                   pwdiUpdateScanParams->wdiUpdateScanParamsInfo.aChannels,
                   updateScanParams.ucChannelCount);

   updateScanParams.usActiveMinChTime  = pwdiUpdateScanParams->wdiUpdateScanParamsInfo.usActiveMinChTime;
   updateScanParams.usActiveMaxChTime  = pwdiUpdateScanParams->wdiUpdateScanParamsInfo.usActiveMaxChTime;
   updateScanParams.usPassiveMinChTime = pwdiUpdateScanParams->wdiUpdateScanParamsInfo.usPassiveMinChTime;
   updateScanParams.usPassiveMaxChTime = pwdiUpdateScanParams->wdiUpdateScanParamsInfo.usPassiveMaxChTime;
   updateScanParams.cbState            = pwdiUpdateScanParams->wdiUpdateScanParamsInfo.cbState;

   wpalMemoryCopy( pSendBuffer+usDataOffset,
                   &updateScanParams,
                   sizeof(updateScanParams));

   pWDICtx->wdiReqStatusCB     = pwdiUpdateScanParams->wdiReqStatusCB;
   pWDICtx->pReqStatusUserData = pwdiUpdateScanParams->pUserData;

   WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_INFO,
               "End Update Scan Parameters");
   /*                                                                         
                                
                                                                            */
   return  WDI_SendMsg( pWDICtx, pSendBuffer, usSendSize,
                        wdiUpdateScanParamsCb, pEventData->pUserData, WDI_UPDATE_SCAN_PARAMS_RESP);
}

/* 
                                                           

                                                         
                                                                    

     
                                    
*/
WDI_Status
WDI_ProcessPrefNetworkFoundInd
(
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
)
{
  WDI_LowLevelIndType  wdiInd;
  tPrefNetwFoundInd    prefNetwFoundInd = {{0}};


  /*                                                                         
                
                                                                           */
  if (( NULL == pWDICtx ) || ( NULL == pEventData ) ||
      ( NULL == pEventData->pEventData ))
  {
     WPAL_TRACE( eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_WARN,
                 "%s: Invalid parameters", __FUNCTION__);
     WDI_ASSERT( 0 );
     return WDI_STATUS_E_FAILURE;
  }

  /*                                                                         
                                          
                                                                           */
  wpalMemoryCopy( (void *)&prefNetwFoundInd.prefNetwFoundParams,
                  pEventData->pEventData,
                  sizeof(tPrefNetwFoundParams));

  /*                                 */
  wdiInd.wdiIndicationType = WDI_PREF_NETWORK_FOUND_IND;

  wpalMemoryZero(wdiInd.wdiIndicationData.wdiPrefNetworkFoundInd.ssId.sSSID,32);

  wdiInd.wdiIndicationData.wdiPrefNetworkFoundInd.ssId.ucLength =
     (prefNetwFoundInd.prefNetwFoundParams.ssId.length < 31 )?
      prefNetwFoundInd.prefNetwFoundParams.ssId.length : 31;

  wpalMemoryCopy( wdiInd.wdiIndicationData.wdiPrefNetworkFoundInd.ssId.sSSID,
                  prefNetwFoundInd.prefNetwFoundParams.ssId.ssId,
                  wdiInd.wdiIndicationData.wdiPrefNetworkFoundInd.ssId.ucLength);

  wdiInd.wdiIndicationData.wdiPrefNetworkFoundInd.rssi =
     prefNetwFoundInd.prefNetwFoundParams.rssi;

  //      
  WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_FATAL,
              "[PNO WDI] PREF_NETWORK_FOUND_IND Type (%x) data (SSID=%s, RSSI=%d)",
              wdiInd.wdiIndicationType,
              wdiInd.wdiIndicationData.wdiPrefNetworkFoundInd.ssId.sSSID,
              wdiInd.wdiIndicationData.wdiPrefNetworkFoundInd.rssi );

  /*           */
  pWDICtx->wdiLowLevelIndCB( &wdiInd, pWDICtx->pIndUserData );

  return WDI_STATUS_SUCCESS;
}

/* 
                                               
                                                         

                                                         
                                                                    

     
                                    
*/
WDI_Status
WDI_ProcessSetPreferredNetworkRsp
(
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
)
{
   WDI_Status           wdiStatus;
   eHalStatus           halStatus;
   WDI_PNOScanCb       wdiPNOScanCb   = NULL;
   /*                                                                        */

   /*                                                                         
                 
                                                                            */
   if (( NULL == pWDICtx ) || ( NULL == pEventData ) ||
       ( NULL == pEventData->pEventData ))
   {
      WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_WARN,
                  "%s: Invalid parameters", __FUNCTION__);
      WDI_ASSERT(0);
      return WDI_STATUS_E_FAILURE;
   }


   wdiPNOScanCb = (WDI_PNOScanCb)pWDICtx->pfncRspCB;

   /*                                                                         
                                         
                                                                            */
   halStatus = *((eHalStatus*)pEventData->pEventData);
   wdiStatus   =   WDI_HAL_2_WDI_STATUS(halStatus);

   /*           */
   wdiPNOScanCb(wdiStatus, pWDICtx->pRspCBUserData);

   return WDI_STATUS_SUCCESS;
}/*                                 */

/* 
                                                       
                                                         

                                                         
                                                                    

     
                                    
*/
WDI_Status
WDI_ProcessSetRssiFilterRsp
(
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
)
{
   WDI_Status           wdiStatus;
   eHalStatus           halStatus;
   WDI_RssiFilterCb     wdiRssiFilterCb;
   /*                                                                        */

   /*                                                                         
                 
                                                                            */
   if (( NULL == pWDICtx ) || ( NULL == pEventData ) ||
       ( NULL == pEventData->pEventData ))
   {
      WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_WARN,
                  "%s: Invalid parameters", __FUNCTION__);
      WDI_ASSERT(0);
      return WDI_STATUS_E_FAILURE;
   }

   wdiRssiFilterCb = (WDI_RssiFilterCb)pWDICtx->pfncRspCB;

   /*                                                                         
                                         
                                                                            */
   halStatus = *((eHalStatus*)pEventData->pEventData);
   wdiStatus   =   WDI_HAL_2_WDI_STATUS(halStatus);

   /*           */
   wdiRssiFilterCb(wdiStatus, pWDICtx->pRspCBUserData);

   return WDI_STATUS_SUCCESS;
}/*                           */

/* 
                                                              
                                                         

                                                         
                                                                    

     
                                    
*/
WDI_Status
WDI_ProcessUpdateScanParamsRsp
(
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
)
{
   WDI_Status             wdiStatus;
   tUpdateScanParamsResp  halUpdScanParams;
   WDI_UpdateScanParamsCb wdiUpdateScanParamsCb   = NULL;
   wpt_uint32             uStatus;
   /*                                                                        */

   /*                                                                         
                 
                                                                            */
   if (( NULL == pWDICtx ) || ( NULL == pEventData ) ||
       ( NULL == pEventData->pEventData ))
   {
      WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_WARN,
                  "%s: Invalid parameters", __FUNCTION__);
      WDI_ASSERT(0);
      return WDI_STATUS_E_FAILURE;
   }

   WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_INFO,
                  "Process UPD scan params ptr : %x", __FUNCTION__);

  wdiUpdateScanParamsCb = (WDI_UpdateScanParamsCb)pWDICtx->pfncRspCB;

  /*                                                                         
                                         
                                                                            */
  wpalMemoryCopy( (void *)&halUpdScanParams.status,
                  pEventData->pEventData,
                  sizeof(halUpdScanParams.status));

  uStatus  = halUpdScanParams.status;

  /*                                            */
  pWDICtx->wdiPNOVersion = (uStatus & WDI_PNO_VERSION_MASK)? 1:0;

  /*                  */
  uStatus = uStatus & ( ~(WDI_PNO_VERSION_MASK));

  wdiStatus   =   WDI_HAL_2_WDI_STATUS(uStatus);

  WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_INFO,
              "UPD Scan Parameters rsp with status: %d",
              halUpdScanParams.status);

  /*           */
  wdiUpdateScanParamsCb(wdiStatus, pWDICtx->pRspCBUserData);

  return WDI_STATUS_SUCCESS;
}
#endif //                      

#ifdef WLAN_FEATURE_PACKET_FILTERING
WDI_Status
WDI_8023MulticastListReq
(
  WDI_RcvFltPktSetMcListReqParamsType*  pwdiRcvFltPktSetMcListReqInfo,
  WDI_8023MulticastListCb               wdi8023MulticastListCallback,
  void*                                 pUserData
)
{
   WDI_EventInfoType      wdiEventData;
   /*                                                                        */

   WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_INFO,
             "%s", __FUNCTION__);

   /*                                                                        
                 
                                                                           */
   if ( eWLAN_PAL_FALSE == gWDIInitialized )
   {
     WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_ERROR,
               "WDI API call before module is initialized - Fail request");

     return WDI_STATUS_E_NOT_ALLOWED;
   }

   /*                                                                        
                                                
                                                                           */
   wdiEventData.wdiRequest      = WDI_8023_MULTICAST_LIST_REQ;
   wdiEventData.pEventData      = pwdiRcvFltPktSetMcListReqInfo;
   wdiEventData.uEventDataSize  = sizeof(*pwdiRcvFltPktSetMcListReqInfo);
   wdiEventData.pCBfnc          = wdi8023MulticastListCallback;
   wdiEventData.pUserData       = pUserData;

   return WDI_PostMainEvent(&gWDICb, WDI_REQUEST_EVENT, &wdiEventData);
}

WDI_Status
WDI_ReceiveFilterSetFilterReq
(
  WDI_SetRcvPktFilterReqParamsType* pwdiSetRcvPktFilterReqInfo,
  WDI_ReceiveFilterSetFilterCb      wdiReceiveFilterSetFilterCallback,
  void*                             pUserData
)
{
   WDI_EventInfoType      wdiEventData;
   /*                                                                        */

   WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_INFO,
             "%s",__FUNCTION__);

   /*                                                                        
                 
                                                                           */
   if ( eWLAN_PAL_FALSE == gWDIInitialized )
   {
     WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_ERROR,
               "WDI API call before module is initialized - Fail request");

     return WDI_STATUS_E_NOT_ALLOWED;
   }

   /*                                                                        
                                                
                                                                           */
   wdiEventData.wdiRequest      = WDI_RECEIVE_FILTER_SET_FILTER_REQ;
   wdiEventData.pEventData      = pwdiSetRcvPktFilterReqInfo;
   wdiEventData.uEventDataSize  = sizeof(*pwdiSetRcvPktFilterReqInfo) +
                                  (pwdiSetRcvPktFilterReqInfo->wdiPktFilterCfg.numFieldParams
                                  * sizeof(WDI_RcvPktFilterFieldParams) - 1);
   wdiEventData.pCBfnc          = wdiReceiveFilterSetFilterCallback;
   wdiEventData.pUserData       = pUserData;


   return WDI_PostMainEvent(&gWDICb, WDI_REQUEST_EVENT, &wdiEventData);
}

WDI_Status
WDI_FilterMatchCountReq
(
  WDI_RcvFltPktMatchCntReqParamsType* pwdiRcvFltPktMatchCntReqInfo,
  WDI_FilterMatchCountCb              wdiFilterMatchCountCallback,
  void*                               pUserData
)
{
   WDI_EventInfoType      wdiEventData;
   /*                                                                        */

   WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_INFO,
             "%s",__FUNCTION__);

   /*                                                                        
                 
                                                                           */
   if ( eWLAN_PAL_FALSE == gWDIInitialized )
   {
     WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_ERROR,
               "WDI API call before module is initialized - Fail request");

     return WDI_STATUS_E_NOT_ALLOWED;
   }

   /*                                                                        
                                                
                                                                           */
   wdiEventData.wdiRequest      = WDI_PACKET_COALESCING_FILTER_MATCH_COUNT_REQ;
   wdiEventData.pEventData      = pwdiRcvFltPktMatchCntReqInfo;
   wdiEventData.uEventDataSize  = sizeof(*pwdiRcvFltPktMatchCntReqInfo);
   wdiEventData.pCBfnc          = wdiFilterMatchCountCallback;
   wdiEventData.pUserData       = pUserData;


   return WDI_PostMainEvent(&gWDICb, WDI_REQUEST_EVENT, &wdiEventData);
}

WDI_Status
WDI_ReceiveFilterClearFilterReq
(
  WDI_RcvFltPktClearReqParamsType*  pwdiRcvFltPktClearReqInfo,
  WDI_ReceiveFilterClearFilterCb    wdiReceiveFilterClearFilterCallback,
  void*                             pUserData
)
{
   WDI_EventInfoType      wdiEventData;
   /*                                                                        */

   WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_INFO,
             "%s",__FUNCTION__);

   /*                                                                        
                 
                                                                           */
   if ( eWLAN_PAL_FALSE == gWDIInitialized )
   {
     WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_ERROR,
               "WDI API call before module is initialized - Fail request");

     return WDI_STATUS_E_NOT_ALLOWED;
   }

   /*                                                                        
                                                
                                                                           */
   wdiEventData.wdiRequest      = WDI_RECEIVE_FILTER_CLEAR_FILTER_REQ;
   wdiEventData.pEventData      = pwdiRcvFltPktClearReqInfo;
   wdiEventData.uEventDataSize  = sizeof(*pwdiRcvFltPktClearReqInfo);
   wdiEventData.pCBfnc          = wdiReceiveFilterClearFilterCallback;
   wdiEventData.pUserData       = pUserData;


   return WDI_PostMainEvent(&gWDICb, WDI_REQUEST_EVENT, &wdiEventData);
}

/* 
                                                    

                                                         
                                                                    

     
                                    
*/
WDI_Status
WDI_Process8023MulticastListReq
(
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
)
{
   WDI_RcvFltPktSetMcListReqParamsType* pwdiFltPktSetMcListReqParamsType  = NULL;
   WDI_8023MulticastListCb    wdi8023MulticastListCb = NULL;
   wpt_uint8*                 pSendBuffer           = NULL;
   wpt_uint16                 usDataOffset          = 0;
   wpt_uint16                 usSendSize            = 0;
   tpHalRcvFltMcAddrListType  pRcvFltMcAddrListType;
   wpt_uint8                  i;
   wpt_uint8                  ucCurrentBSSSesIdx = 0;
   WDI_BSSSessionType*        pBSSSes = NULL;

   WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_INFO,
             "%s",__FUNCTION__);

   pRcvFltMcAddrListType = wpalMemoryAllocate(sizeof(tHalRcvFltMcAddrListType)) ;
   if( NULL == pRcvFltMcAddrListType )
   {
     WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_WARN,
                 "Failed to alloc in WDI_Process8023MulticastListReq");
     return WDI_STATUS_E_FAILURE; 
   }

   /*                                                                         
                 
                                                                            */
   if (( NULL == pEventData ) ||
       ( NULL == (pwdiFltPktSetMcListReqParamsType =
       (WDI_RcvFltPktSetMcListReqParamsType*)pEventData->pEventData)) ||
       ( NULL == (wdi8023MulticastListCb =
       (WDI_8023MulticastListCb)pEventData->pCBfnc)))
   {
      WPAL_TRACE( eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_WARN,
                  "%s: Invalid parameters", __FUNCTION__);
      wpalMemoryFree(pRcvFltMcAddrListType);
      WDI_ASSERT(0);
      return WDI_STATUS_E_FAILURE;
   }

   ucCurrentBSSSesIdx = WDI_FindAssocSession( pWDICtx,
                            pwdiFltPktSetMcListReqParamsType->mcAddrList.bssId,
                            &pBSSSes);
   if ( NULL == pBSSSes )
   {
       WPAL_TRACE( eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_ERROR,
                 " %s : Association for this BSSID does not exist",__FUNCTION__);
       wpalMemoryFree(pRcvFltMcAddrListType);   
       return WDI_STATUS_E_FAILURE; 
   }

   /*                                                                       
                       
                                                                          */
   if (( WDI_STATUS_SUCCESS != WDI_GetMessageBuffer( pWDICtx,
                         WDI_8023_MULTICAST_LIST_REQ,
                         sizeof(tHalRcvFltMcAddrListType),
                         &pSendBuffer, &usDataOffset, &usSendSize))||
       ( usSendSize < (usDataOffset + sizeof(tHalRcvFltMcAddrListType))))
   {
      WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_WARN,
                  "Unable to get send buffer in "
                  "WDI_Process8023MulticastListReq() %x %x %x",
                  pEventData, pwdiFltPktSetMcListReqParamsType,
                  wdi8023MulticastListCb);
      WDI_ASSERT(0);
      return WDI_STATUS_E_FAILURE;
   }

   pRcvFltMcAddrListType->cMulticastAddr = 
       pwdiFltPktSetMcListReqParamsType->mcAddrList.ulMulticastAddrCnt;
   for( i = 0; i < pRcvFltMcAddrListType->cMulticastAddr; i++ )
   {
      wpalMemoryCopy(pRcvFltMcAddrListType->multicastAddr[i],
                 pwdiFltPktSetMcListReqParamsType->mcAddrList.multicastAddr[i],
                 sizeof(tSirMacAddr));
   }

   pRcvFltMcAddrListType->bssIdx = pBSSSes->ucBSSIdx;
   wpalMemoryCopy( pSendBuffer+usDataOffset, 
                   pRcvFltMcAddrListType, 
                   sizeof(tHalRcvFltMcAddrListType)); 

   pWDICtx->wdiReqStatusCB     = pwdiFltPktSetMcListReqParamsType->wdiReqStatusCB;
   pWDICtx->pReqStatusUserData = pwdiFltPktSetMcListReqParamsType->pUserData;


   wpalMemoryFree(pRcvFltMcAddrListType);
   /*                                                                         
                                
                                                                            */
   return  WDI_SendMsg( pWDICtx, pSendBuffer, usSendSize,
                        wdi8023MulticastListCb, pEventData->pUserData,
                        WDI_8023_MULTICAST_LIST_RESP);
}

/* 
                                                          

                                                         
                                                                    

     
                                    
*/
WDI_Status
WDI_ProcessReceiveFilterSetFilterReq
(
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
)
{
   WDI_SetRcvPktFilterReqParamsType* pwdiSetRcvPktFilterReqInfo  = NULL;
   WDI_ReceiveFilterSetFilterCb      wdiReceiveFilterSetFilterCb = NULL;
   wpt_uint8*                 pSendBuffer           = NULL;
   wpt_uint16                 usDataOffset          = 0;
   wpt_uint16                 usSendSize            = 0;
   wpt_uint32                 usRcvPktFilterCfgSize;
   tHalRcvPktFilterCfgType    *pRcvPktFilterCfg;
   wpt_uint8                  i;
   wpt_uint8                  ucCurrentBSSSesIdx = 0;
   WDI_BSSSessionType*        pBSSSes = NULL;

   WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_INFO,
             "%s",__FUNCTION__);

   /*                                                                         
                 
                                                                            */
   if (( NULL == pEventData ) ||
       ( NULL == (pwdiSetRcvPktFilterReqInfo =
       (WDI_SetRcvPktFilterReqParamsType*)pEventData->pEventData)) ||
       ( NULL == (wdiReceiveFilterSetFilterCb =
       (WDI_ReceiveFilterSetFilterCb)pEventData->pCBfnc)))
   {
      WPAL_TRACE( eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_WARN,
                  "%s: Invalid parameters", __FUNCTION__);
      WDI_ASSERT(0);
      return WDI_STATUS_E_FAILURE;
   }

   ucCurrentBSSSesIdx = WDI_FindAssocSession( pWDICtx,
                            pwdiSetRcvPktFilterReqInfo->wdiPktFilterCfg.bssId,
                            &pBSSSes);
   if ( NULL == pBSSSes )
   {
       WPAL_TRACE( eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_ERROR,
                 " %s : Association for this BSSID does not exist",__FUNCTION__);
       return WDI_STATUS_E_FAILURE;
   }

   usRcvPktFilterCfgSize = sizeof(tHalRcvPktFilterCfgType) +
       ((pwdiSetRcvPktFilterReqInfo->wdiPktFilterCfg.numFieldParams - 1)
        * sizeof(tHalRcvPktFilterParams));

  pRcvPktFilterCfg = (tHalRcvPktFilterCfgType *)wpalMemoryAllocate(
                                              usRcvPktFilterCfgSize);

  if(NULL == pRcvPktFilterCfg)
  {
    WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_WARN,
            "%s: Failed to allocate memory for "
            "tHalRcvPktFilterCfgType: %x %x %x ",
            __FUNCTION__, pWDICtx, pEventData, pEventData->pEventData);
    WDI_ASSERT(0);
    return WDI_STATUS_E_FAILURE;
  }

  wpalMemoryZero(pRcvPktFilterCfg, usRcvPktFilterCfgSize);

   /*                                                                       
                       

                                                                          */
   if (( WDI_STATUS_SUCCESS != WDI_GetMessageBuffer( pWDICtx, WDI_RECEIVE_FILTER_SET_FILTER_REQ,
                         usRcvPktFilterCfgSize,
                         &pSendBuffer, &usDataOffset, &usSendSize))||
       ( usSendSize < (usDataOffset + usRcvPktFilterCfgSize)))
   {
      WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_WARN,
                  "Unable to get send buffer in "
                  "WDI_ProcessReceiveFilterSetFilterReq() %x %x %x",
                  pEventData, pwdiSetRcvPktFilterReqInfo,
                  wdiReceiveFilterSetFilterCb);
      WDI_ASSERT(0);
      wpalMemoryFree(pRcvPktFilterCfg);
      return WDI_STATUS_E_FAILURE;
   }

   WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_INFO,
              "UsData Off %d UsSend %d cfg %d",usDataOffset,
              usSendSize,usRcvPktFilterCfgSize);

   pRcvPktFilterCfg->filterId = pwdiSetRcvPktFilterReqInfo->wdiPktFilterCfg.filterId;
   pRcvPktFilterCfg->filterType = pwdiSetRcvPktFilterReqInfo->wdiPktFilterCfg.filterType;
   pRcvPktFilterCfg->numParams = pwdiSetRcvPktFilterReqInfo->wdiPktFilterCfg.numFieldParams;
   pRcvPktFilterCfg->coalesceTime = pwdiSetRcvPktFilterReqInfo->wdiPktFilterCfg.coalesceTime;

   //                                             

   WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_INFO,
              "Out: FID %d FT %d",pRcvPktFilterCfg->filterId,
              pRcvPktFilterCfg->filterType);
   WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_INFO,
              "NParams %d CT %d",pRcvPktFilterCfg->numParams,
              pRcvPktFilterCfg->coalesceTime);

   for ( i = 0; i < pRcvPktFilterCfg->numParams; i++ )
   {
       pRcvPktFilterCfg->paramsData[i].protocolLayer =
           pwdiSetRcvPktFilterReqInfo->wdiPktFilterCfg.paramsData[i].protocolLayer;
       pRcvPktFilterCfg->paramsData[i].cmpFlag =
           pwdiSetRcvPktFilterReqInfo->wdiPktFilterCfg.paramsData[i].cmpFlag;
       pRcvPktFilterCfg->paramsData[i].dataOffset =
           pwdiSetRcvPktFilterReqInfo->wdiPktFilterCfg.paramsData[i].dataOffset;
        pRcvPktFilterCfg->paramsData[i].dataLength =
            pwdiSetRcvPktFilterReqInfo->wdiPktFilterCfg.paramsData[i].dataLength;

       wpalMemoryCopy(&pRcvPktFilterCfg->paramsData[i].compareData,
                    &pwdiSetRcvPktFilterReqInfo->wdiPktFilterCfg.paramsData[i].compareData,
                    8);
       wpalMemoryCopy(&pRcvPktFilterCfg->paramsData[i].dataMask,
                    &pwdiSetRcvPktFilterReqInfo->wdiPktFilterCfg.paramsData[i].dataMask,
                    8);

      WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_INFO,
           "Out:Proto %d Comp Flag %d \n",
           pRcvPktFilterCfg->paramsData[i].protocolLayer,
           pRcvPktFilterCfg->paramsData[i].cmpFlag);

      WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_INFO,
           "Data Offset %d Data Len %d\n",
           pRcvPktFilterCfg->paramsData[i].dataOffset,
           pRcvPktFilterCfg->paramsData[i].dataLength);

      WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_INFO,
           "CData: %d:%d:%d:%d:%d:%d\n",
           pRcvPktFilterCfg->paramsData[i].compareData[0],
           pRcvPktFilterCfg->paramsData[i].compareData[1],
           pRcvPktFilterCfg->paramsData[i].compareData[2],
           pRcvPktFilterCfg->paramsData[i].compareData[3],
           pRcvPktFilterCfg->paramsData[i].compareData[4],
           pRcvPktFilterCfg->paramsData[i].compareData[5]);

      WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_INFO,
           "MData: %d:%d:%d:%d:%d:%d\n",
           pRcvPktFilterCfg->paramsData[i].dataMask[0],
           pRcvPktFilterCfg->paramsData[i].dataMask[1],
           pRcvPktFilterCfg->paramsData[i].dataMask[2],
           pRcvPktFilterCfg->paramsData[i].dataMask[3],
           pRcvPktFilterCfg->paramsData[i].dataMask[4],
           pRcvPktFilterCfg->paramsData[i].dataMask[5]);
   }

   wpalMemoryCopy( pSendBuffer+usDataOffset,
                   pRcvPktFilterCfg,
                   usRcvPktFilterCfgSize);


   pWDICtx->wdiReqStatusCB     = pwdiSetRcvPktFilterReqInfo->wdiReqStatusCB;
   pWDICtx->pReqStatusUserData = pwdiSetRcvPktFilterReqInfo->pUserData;

   WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_INFO,
             "%s",__FUNCTION__);
   wpalMemoryFree(pRcvPktFilterCfg);

   /*                                                                         
                                
                                                                            */
   return  WDI_SendMsg( pWDICtx, pSendBuffer, usSendSize,
                        wdiReceiveFilterSetFilterCb, pEventData->pUserData,
                        WDI_RECEIVE_FILTER_SET_FILTER_RESP);
}

/* 
                                                          

                                                         
                                                                    

     
                                    
*/
WDI_Status
WDI_ProcessFilterMatchCountReq
(
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
)
{
   WDI_RcvFltPktMatchCntReqParamsType*    pwdiRcvFltPktMatchCntReqParamsType =
                                                                         NULL;
   WDI_FilterMatchCountCb                 wdiFilterMatchCountCb              =
                                                                         NULL;
   wpt_uint8*                             pSendBuffer           = NULL;
   wpt_uint16                             usDataOffset          = 0;
   wpt_uint16                             usSendSize            = 0;

   WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_INFO,
             "%s",__FUNCTION__);

   /*                                                                         
                 
                                                                            */
   if (( NULL == pEventData ) ||
       ( NULL == (pwdiRcvFltPktMatchCntReqParamsType =
       (WDI_RcvFltPktMatchCntReqParamsType*)pEventData->pEventData)) ||
       ( NULL == (wdiFilterMatchCountCb =
       (WDI_FilterMatchCountCb)pEventData->pCBfnc)))
   {
      WPAL_TRACE( eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_WARN,
                  "%s: Invalid parameters", __FUNCTION__);
      WDI_ASSERT(0);
      return WDI_STATUS_E_FAILURE;
   }

   /*                                                                       
                       
                                                                          */
   if (( WDI_STATUS_SUCCESS != WDI_GetMessageBuffer( pWDICtx,
                         WDI_PACKET_COALESCING_FILTER_MATCH_COUNT_REQ,
                         0,
                         &pSendBuffer, &usDataOffset, &usSendSize))||
       ( usSendSize < usDataOffset))
   {
      WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_WARN,
                  "Unable to get send buffer in "
                  "WDI_ProcessFilterMatchCountReq() %x %x %x",
                  pEventData, pwdiRcvFltPktMatchCntReqParamsType,
                  wdiFilterMatchCountCb);
      WDI_ASSERT(0);
      return WDI_STATUS_E_FAILURE;
   }

   //
   //                                                 
   //
   pWDICtx->wdiReqStatusCB     = pwdiRcvFltPktMatchCntReqParamsType->wdiReqStatusCB;
   pWDICtx->pReqStatusUserData = pwdiRcvFltPktMatchCntReqParamsType->pUserData;


   /*                                                                         
                                
                                                                            */
   return  WDI_SendMsg( pWDICtx, pSendBuffer, usSendSize,
                        wdiFilterMatchCountCb,
                        pEventData->pUserData,
                        WDI_PACKET_COALESCING_FILTER_MATCH_COUNT_RESP);
}

/* 
                                                            

                                                         
                                                                    

     
                                    
*/
WDI_Status
WDI_ProcessReceiveFilterClearFilterReq
(
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
)
{
   WDI_RcvFltPktClearReqParamsType* pwdiRcvFltPktClearReqParamsType = NULL;
   WDI_ReceiveFilterClearFilterCb   wdiRcvFltPktClearFilterCb       = NULL;
   wpt_uint8*                       pSendBuffer           = NULL;
   wpt_uint16                       usDataOffset          = 0;
   wpt_uint16                       usSendSize            = 0;
   tHalRcvFltPktClearParam          rcvFltPktClearParam;
   wpt_uint8                        ucCurrentBSSSesIdx = 0;
   WDI_BSSSessionType*              pBSSSes = NULL;

   WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_INFO,
             "%s",__FUNCTION__);

   /*                                                                         
                 
                                                                            */
   if (( NULL == pEventData ) ||
       ( NULL == (pwdiRcvFltPktClearReqParamsType =
       (WDI_RcvFltPktClearReqParamsType*)pEventData->pEventData)) ||
       ( NULL == (wdiRcvFltPktClearFilterCb =
       (WDI_ReceiveFilterClearFilterCb)pEventData->pCBfnc)))
   {
      WPAL_TRACE( eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_WARN,
                  "%s: Invalid parameters", __FUNCTION__);
      WDI_ASSERT(0);
      return WDI_STATUS_E_FAILURE;
   }

   ucCurrentBSSSesIdx = WDI_FindAssocSession( pWDICtx,
                            pwdiRcvFltPktClearReqParamsType->filterClearParam.bssId,
                            &pBSSSes);
   if ( NULL == pBSSSes )
   {
       WPAL_TRACE( eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_ERROR,
                 " %s : Association for this BSSID does not exist",__FUNCTION__);
       return WDI_STATUS_E_FAILURE;
   }

   /*                                                                       
                       
                                                                          */
   if (( WDI_STATUS_SUCCESS != WDI_GetMessageBuffer( pWDICtx,
                         WDI_RECEIVE_FILTER_CLEAR_FILTER_REQ,
                         sizeof(tHalRcvFltPktClearParam),
                         &pSendBuffer, &usDataOffset, &usSendSize))||
       ( usSendSize < (usDataOffset + sizeof(tHalRcvFltPktClearParam))))
   {
      WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_WARN,
                  "Unable to get send buffer in "
                  "WDI_ProcessReceiveFilterClearFilterReq() %x %x %x",
                  pEventData, pwdiRcvFltPktClearReqParamsType,
                  wdiRcvFltPktClearFilterCb);
      WDI_ASSERT(0);
      return WDI_STATUS_E_FAILURE;
   }


   rcvFltPktClearParam.status = pwdiRcvFltPktClearReqParamsType->
                                                    filterClearParam.status;
   rcvFltPktClearParam.filterId = pwdiRcvFltPktClearReqParamsType->
                                                    filterClearParam.filterId;

   rcvFltPktClearParam.bssIdx = pBSSSes->ucBSSIdx;
   wpalMemoryCopy( pSendBuffer+usDataOffset,
                   &rcvFltPktClearParam,
                   sizeof(rcvFltPktClearParam));

   pWDICtx->wdiReqStatusCB     = pwdiRcvFltPktClearReqParamsType->wdiReqStatusCB;
   pWDICtx->pReqStatusUserData = pwdiRcvFltPktClearReqParamsType->pUserData;


   /*                                                                         
                                
                                                                            */
   return  WDI_SendMsg( pWDICtx, pSendBuffer, usSendSize,
                        wdiRcvFltPktClearFilterCb, pEventData->pUserData,
                        WDI_RECEIVE_FILTER_CLEAR_FILTER_RESP);
}

/* 
                                                     

                                                         
                                                                    

     
                                    
*/
WDI_Status
WDI_Process8023MulticastListRsp
(
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
)
{
   WDI_Status           wdiStatus;
   eHalStatus           halStatus;
   WDI_8023MulticastListCb wdi8023MulticastListCb;
   /*                                                                        */

   WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_INFO,
             "%s",__FUNCTION__);

   /*                                                                         
                 
                                                                            */
   if (( NULL == pWDICtx ) || ( NULL == pEventData ) ||
       ( NULL == pEventData->pEventData ))
   {
      WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_WARN,
                  "%s: Invalid parameters", __FUNCTION__);
      WDI_ASSERT(0);
      return WDI_STATUS_E_FAILURE;
   }

   wdi8023MulticastListCb = (WDI_8023MulticastListCb)pWDICtx->pfncRspCB;

   /*                                                                         
                                         
                                                                            */
   halStatus = *((eHalStatus*)pEventData->pEventData);
   wdiStatus   =   WDI_HAL_2_WDI_STATUS(halStatus);

   /*           */
   wdi8023MulticastListCb(wdiStatus, pWDICtx->pRspCBUserData);

   return WDI_STATUS_SUCCESS;
}

/* 
                                               
                                                         

                                                         
                                                                    

     
                                    
*/
WDI_Status
WDI_ProcessReceiveFilterSetFilterRsp
(
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
)
{
   WDI_Status           wdiStatus;
   eHalStatus           halStatus;
   WDI_ReceiveFilterSetFilterCb wdiReceiveFilterSetFilterCb;
   /*                                                                        */

   WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_INFO,
          "%s",__FUNCTION__);

   /*                                                                         
                 
                                                                            */
   if (( NULL == pWDICtx ) || ( NULL == pEventData ) ||
       ( NULL == pEventData->pEventData ))
   {
      WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_WARN,
                  "%s: Invalid parameters", __FUNCTION__);
      WDI_ASSERT(0);
      return WDI_STATUS_E_FAILURE;
   }

   wdiReceiveFilterSetFilterCb = (WDI_ReceiveFilterSetFilterCb)pWDICtx->
                                                                   pfncRspCB;

   /*                                                                         
                                         
                                                                            */
   halStatus = *((eHalStatus*)pEventData->pEventData);
   wdiStatus   =   WDI_HAL_2_WDI_STATUS(halStatus);

   /*           */
   wdiReceiveFilterSetFilterCb(wdiStatus, pWDICtx->pRspCBUserData);

   return WDI_STATUS_SUCCESS;
}

/* 
                                                           

                                                         
                                                                    

     
                                    
*/
WDI_Status
WDI_ProcessFilterMatchCountRsp
(
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
)
{
   WDI_Status           wdiStatus;
   eHalStatus           halStatus;

   WDI_FilterMatchCountCb   wdiFilterMatchCountCb;

   /*                                                                        */

   WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_INFO,
             "%s",__FUNCTION__);

   /*                                                                         
                 
                                                                            */
   if (( NULL == pWDICtx ) || ( NULL == pEventData ) ||
       ( NULL == pEventData->pEventData ))
   {
      WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_WARN,
                  "%s: Invalid parameters", __FUNCTION__);
      WDI_ASSERT(0);
      return WDI_STATUS_E_FAILURE;
   }

   wdiFilterMatchCountCb = (WDI_FilterMatchCountCb)pWDICtx->pfncRspCB;

   /*                                                                         
                                         
                                                                            */
   halStatus = *((eHalStatus*)pEventData->pEventData);
   wdiStatus   =   WDI_HAL_2_WDI_STATUS(halStatus);

   /*           */
   wdiFilterMatchCountCb(wdiStatus, pWDICtx->pRspCBUserData);

   return WDI_STATUS_SUCCESS;
}

/* 
                                                             

                                                         
                                                                    

     
                                    
*/
WDI_Status
WDI_ProcessReceiveFilterClearFilterRsp
(
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
)
{
   WDI_Status           wdiStatus;
   eHalStatus           halStatus;
   WDI_ReceiveFilterClearFilterCb wdiReceiveFilterClearFilterCb;
   /*                                                                        */

   WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_INFO,
             "%s",__FUNCTION__);

   /*                                                                         
                 
                                                                            */
   if (( NULL == pWDICtx ) || ( NULL == pEventData ) ||
       ( NULL == pEventData->pEventData ))
   {
      WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_WARN,
                  "%s: Invalid parameters", __FUNCTION__);
      WDI_ASSERT(0);
      return WDI_STATUS_E_FAILURE;
   }

   wdiReceiveFilterClearFilterCb = (WDI_ReceiveFilterClearFilterCb)pWDICtx->
                                                                 pfncRspCB;

   /*                                                                         
                                         
                                                                            */
   halStatus = *((eHalStatus*)pEventData->pEventData);
   wdiStatus   =   WDI_HAL_2_WDI_STATUS(halStatus);

   /*           */
   wdiReceiveFilterClearFilterCb(wdiStatus, pWDICtx->pRspCBUserData);

   return WDI_STATUS_SUCCESS;
}
#endif //                              

/* 
                                     
                                                      
                                         

                                                         
                                                                    

     
                                    
*/
WDI_Status
WDI_ProcessShutdownRsp
(
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
)
{
  /*                                                                       
           */
  WDI_ASSERT(0);
  return WDI_STATUS_SUCCESS;
}/*                      */

/* 
                             

                                                         
                                                       

                                                                
                                                           
              

                                                         
                

                                    
*/
WDI_Status
WDI_SetPowerParamsReq
(
  WDI_SetPowerParamsReqParamsType* pwdiPowerParamsReqParams,
  WDI_SetPowerParamsCb             wdiPowerParamsCb,
  void*                            pUserData
)
{
   WDI_EventInfoType      wdiEventData;
   /*                                                                        */

   /*                                                                        
                 
                                                                           */
   if ( eWLAN_PAL_FALSE == gWDIInitialized )
   {
     WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_ERROR,
               "WDI API call before module is initialized - Fail request");

     return WDI_STATUS_E_NOT_ALLOWED;
   }

   /*                                                                        
                                                
                                                                           */
   wdiEventData.wdiRequest      = WDI_SET_POWER_PARAMS_REQ;
   wdiEventData.pEventData      = pwdiPowerParamsReqParams;
   wdiEventData.uEventDataSize  = sizeof(*pwdiPowerParamsReqParams);
   wdiEventData.pCBfnc          = wdiPowerParamsCb;
   wdiEventData.pUserData       = pUserData;

   return WDI_PostMainEvent(&gWDICb, WDI_REQUEST_EVENT, &wdiEventData);
}/*                     */

/* 
                                                 

                                                         
                                                                    

     
                                    
*/
WDI_Status
WDI_ProcessSetPowerParamsReq
(
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
)
{
   WDI_SetPowerParamsReqParamsType* pwdiPowerParamsReqParams = NULL;
   WDI_SetPowerParamsCb             wdiPowerParamsCb         = NULL;
   wpt_uint8*                       pSendBuffer              = NULL;
   wpt_uint16                       usDataOffset             = 0;
   wpt_uint16                       usSendSize               = 0;
   tSetPowerParamsType              powerParams;

   /*                                                                         
                 
                                                                            */
   if (( NULL == pEventData ) ||
       ( NULL == (pwdiPowerParamsReqParams = (WDI_SetPowerParamsReqParamsType*)pEventData->pEventData)) ||
       ( NULL == (wdiPowerParamsCb   = (WDI_SetPowerParamsCb)pEventData->pCBfnc)))
   {
      WPAL_TRACE( eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_WARN,
                  "%s: Invalid parameters", __FUNCTION__);
      WDI_ASSERT(0);
      return WDI_STATUS_E_FAILURE;
   }

   /*                                                                       
                       
                                                                          */
   if (( WDI_STATUS_SUCCESS != WDI_GetMessageBuffer( pWDICtx, WDI_SET_POWER_PARAMS_REQ,
                         sizeof(powerParams),
                         &pSendBuffer, &usDataOffset, &usSendSize))||
       ( usSendSize < (usDataOffset + sizeof(powerParams) )))
   {
      WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_WARN,
                  "Unable to get send buffer in Set PNO req %x %x %x",
                  pEventData, pwdiPowerParamsReqParams, wdiPowerParamsCb);
      WDI_ASSERT(0);
      return WDI_STATUS_E_FAILURE;
   }

  /*              */
  powerParams.uIgnoreDTIM =
    pwdiPowerParamsReqParams->wdiSetPowerParamsInfo.uIgnoreDTIM;

  /*           */
  powerParams.uDTIMPeriod =
    pwdiPowerParamsReqParams->wdiSetPowerParamsInfo.uDTIMPeriod;

  /*                 */
  powerParams.uListenInterval=
    pwdiPowerParamsReqParams->wdiSetPowerParamsInfo.uListenInterval;

  /*                            */
  powerParams.uBcastMcastFilter =
    pwdiPowerParamsReqParams->wdiSetPowerParamsInfo.uBcastMcastFilter;

  /*                          */
  powerParams.uEnableBET =
    pwdiPowerParamsReqParams->wdiSetPowerParamsInfo.uEnableBET;

  /*                                   */
  powerParams.uBETInterval =
    pwdiPowerParamsReqParams->wdiSetPowerParamsInfo.uBETInterval;


   wpalMemoryCopy( pSendBuffer+usDataOffset,
                   &powerParams,
                   sizeof(powerParams));

   pWDICtx->wdiReqStatusCB     = pwdiPowerParamsReqParams->wdiReqStatusCB;
   pWDICtx->pReqStatusUserData = pwdiPowerParamsReqParams->pUserData;

   /*                                                                         
                                
                                                                            */
   return  WDI_SendMsg( pWDICtx, pSendBuffer, usSendSize,
                        wdiPowerParamsCb, pEventData->pUserData, WDI_SET_POWER_PARAMS_RESP);
}

/* 
                                                        
                                                         

                                                         
                                                                    

     
                                    
*/
WDI_Status
WDI_ProcessSetPowerParamsRsp
(
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
)
{
   WDI_Status           wdiStatus;
   eHalStatus           halStatus;
   WDI_SetPowerParamsCb             wdiPowerParamsCb;
   /*                                                                        */

   /*                                                                         
                 
                                                                            */
   if (( NULL == pWDICtx ) || ( NULL == pEventData ) ||
       ( NULL == pEventData->pEventData ))
   {
      WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_WARN,
                  "%s: Invalid parameters", __FUNCTION__);
      WDI_ASSERT(0);
      return WDI_STATUS_E_FAILURE;
   }

   wdiPowerParamsCb = (WDI_SetPowerParamsCb)pWDICtx->pfncRspCB;

   /*                                                                         
                                         
                                                                            */
   halStatus = *((eHalStatus*)pEventData->pEventData);
   wdiStatus   =   WDI_HAL_2_WDI_STATUS(halStatus);

   /*           */
   wdiPowerParamsCb(wdiStatus, pWDICtx->pRspCBUserData);

   return WDI_STATUS_SUCCESS;
}/*                            */

#ifdef WLAN_FEATURE_GTK_OFFLOAD
/* 
                                                           
                                                                
                                                              
                                                         
                                              

                                                                
                                      

                                        

                                                          
                                             

                                                               
                                                             

                                                         
                

                      
                                    
*/
WDI_Status
WDI_GTKOffloadReq
(
  WDI_GtkOffloadReqMsg*      pwdiGtkOffloadReqMsg,
  WDI_GtkOffloadCb           wdiGtkOffloadCb,
  void*                      pUserData
)
{
   WDI_EventInfoType      wdiEventData = {0};
   /*                                                                        */

   /*                                                                        
                 
                                                                           */
   if ( eWLAN_PAL_FALSE == gWDIInitialized )
   {
     WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_ERROR,
               "WDI API call before module is initialized - Fail request");

     return WDI_STATUS_E_NOT_ALLOWED;
   }

   /*                                                                        
                                                
                                                                           */
   wdiEventData.wdiRequest      = WDI_GTK_OFFLOAD_REQ;
   wdiEventData.pEventData      = pwdiGtkOffloadReqMsg;
   wdiEventData.uEventDataSize  = sizeof(*pwdiGtkOffloadReqMsg);;
   wdiEventData.pCBfnc          = wdiGtkOffloadCb;
   wdiEventData.pUserData       = pUserData;

   return WDI_PostMainEvent(&gWDICb, WDI_REQUEST_EVENT, &wdiEventData);
}


/* 
                                                              
                                                                 
                                                              
                                                                  
                                              

                                                                
                                      

                                        

                                                    
                                                               
                                        

                                                               
                                                                 
                

                                                         
                

                      
                                    
*/
WDI_Status
WDI_GTKOffloadGetInfoReq
(
  WDI_GtkOffloadGetInfoReqMsg*  pwdiGtkOffloadGetInfoReqMsg,
  WDI_GtkOffloadGetInfoCb       wdiGtkOffloadGetInfoCb,
  void*                          pUserData
)
{
   WDI_EventInfoType      wdiEventData = {0};
   /*                                                                        */

   /*                                                                        
                 
                                                                           */
   if ( eWLAN_PAL_FALSE == gWDIInitialized )
   {
     WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_ERROR,
               "WDI API call before module is initialized - Fail request");

     return WDI_STATUS_E_NOT_ALLOWED;
   }

   /*                                                                        
                                                
                                                                           */
   wdiEventData.wdiRequest      = WDI_GTK_OFFLOAD_GETINFO_REQ;
   wdiEventData.pEventData      = pwdiGtkOffloadGetInfoReqMsg;
   wdiEventData.uEventDataSize  = sizeof(*pwdiGtkOffloadGetInfoReqMsg);
   wdiEventData.pCBfnc          = wdiGtkOffloadGetInfoCb;
   wdiEventData.pUserData       = pUserData;

   return WDI_PostMainEvent(&gWDICb, WDI_REQUEST_EVENT, &wdiEventData);
}


/* 
                                                

                                                         
                                                                    

     
                                    
*/
WDI_Status
WDI_ProcessGTKOffloadReq
(
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
)
{
   WDI_GtkOffloadReqMsg*    pwdiGtkOffloadReqMsg = NULL;
   WDI_GtkOffloadCb         wdiGtkOffloadCb      = NULL;
   wpt_uint8*               pSendBuffer          = NULL;
   wpt_uint16               usDataOffset         = 0;
   wpt_uint16               usSendSize           = 0;
   tHalGtkOffloadReqParams  gtkOffloadReqParams = {0};
   /*                                                                        */

   /*                                                                         
                 
                                                                            */
   if (( NULL == pEventData ) ||
       ( NULL == (pwdiGtkOffloadReqMsg = (WDI_GtkOffloadReqMsg*)pEventData->pEventData)) ||
       ( NULL == (wdiGtkOffloadCb   = (WDI_GtkOffloadCb)pEventData->pCBfnc)))
   {
      WPAL_TRACE( eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_WARN,
                  "%s: Invalid parameters", __FUNCTION__);
      WDI_ASSERT(0);
      return WDI_STATUS_E_FAILURE;
   }

   /*                                                                       
                       
                                                                          */
   if (( WDI_STATUS_SUCCESS != WDI_GetMessageBuffer( pWDICtx, WDI_GTK_OFFLOAD_REQ,
                         sizeof(gtkOffloadReqParams),
                         &pSendBuffer, &usDataOffset, &usSendSize))||
       ( usSendSize < (usDataOffset + sizeof(gtkOffloadReqParams) )))
   {
      WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_WARN,
                  "Unable to get send buffer in GTK offload req %x %x %x",
                  pEventData, pwdiGtkOffloadReqMsg, wdiGtkOffloadCb);
      WDI_ASSERT(0);
      return WDI_STATUS_E_FAILURE;
   }

   //
   //                                                                        
   //
   gtkOffloadReqParams.ulFlags = pwdiGtkOffloadReqMsg->gtkOffloadReqParams.ulFlags;
   //         
   wpalMemoryCopy(&(gtkOffloadReqParams.aKCK[0]), &(pwdiGtkOffloadReqMsg->gtkOffloadReqParams.aKCK[0]), 16);
   //         
   wpalMemoryCopy(&(gtkOffloadReqParams.aKEK[0]), &(pwdiGtkOffloadReqMsg->gtkOffloadReqParams.aKEK[0]), 16);
   //                      
   wpalMemoryCopy(&(gtkOffloadReqParams.ullKeyReplayCounter), &(pwdiGtkOffloadReqMsg->gtkOffloadReqParams.ullKeyReplayCounter), sizeof(v_U64_t));

   wpalMemoryCopy( pSendBuffer+usDataOffset,
                   &gtkOffloadReqParams,
                   sizeof(gtkOffloadReqParams));

   pWDICtx->wdiReqStatusCB     = pwdiGtkOffloadReqMsg->wdiReqStatusCB;
   pWDICtx->pReqStatusUserData = pwdiGtkOffloadReqMsg->pUserData;

   /*                                                                         
                                
                                                                            */
   return  WDI_SendMsg( pWDICtx, pSendBuffer, usSendSize,
                        wdiGtkOffloadCb, pEventData->pUserData, WDI_GTK_OFFLOAD_RESP);
}


/* 
                                                            

                                                         
                                                                    

     
                                    
*/
WDI_Status
WDI_ProcessGTKOffloadGetInfoReq
(
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
)
{
   WDI_GtkOffloadGetInfoReqMsg*     pwdiGtkOffloadGetInfoReqMsg = NULL;
   WDI_GtkOffloadGetInfoCb          wdiGtkOffloadGetInfoCb      = NULL;
   wpt_uint8*                       pSendBuffer           = NULL;
   wpt_uint16                       usDataOffset          = 0;
   wpt_uint16                       usSendSize            = 0;

   /*                                                                         
                 
                                                                            */
   if (( NULL == pEventData ) ||
       ( NULL == (pwdiGtkOffloadGetInfoReqMsg = (WDI_GtkOffloadGetInfoReqMsg*)pEventData->pEventData)) ||
       ( NULL == (wdiGtkOffloadGetInfoCb = (WDI_GtkOffloadGetInfoCb)pEventData->pCBfnc)))
   {
      WPAL_TRACE( eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_WARN,
                  "%s: Invalid parameters", __FUNCTION__);
      WDI_ASSERT(0);
      return WDI_STATUS_E_FAILURE;
   }

   /*                                                                       
                       
                                                                          */
   if (( WDI_STATUS_SUCCESS != WDI_GetMessageBuffer( pWDICtx, WDI_GTK_OFFLOAD_GETINFO_REQ,
                         0,
                         &pSendBuffer, &usDataOffset, &usSendSize))||
       ( usSendSize < usDataOffset))
   {
      WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_WARN,
                  "Unable to get send buffer in WDI_ProcessGTKOffloadGetInfoReq() %x %x %x",
                  pEventData, pwdiGtkOffloadGetInfoReqMsg, wdiGtkOffloadGetInfoCb);
      WDI_ASSERT(0);
      return WDI_STATUS_E_FAILURE;
   }

   //
   //                                                 
   //

   pWDICtx->wdiReqStatusCB     = pwdiGtkOffloadGetInfoReqMsg->wdiReqStatusCB;
   pWDICtx->pReqStatusUserData = pwdiGtkOffloadGetInfoReqMsg->pUserData;

   /*                                                                         
                                
                                                                            */
   return  WDI_SendMsg( pWDICtx, pSendBuffer, usSendSize,
                        wdiGtkOffloadGetInfoCb, pEventData->pUserData, WDI_GTK_OFFLOAD_GETINFO_RESP);
}

/* 
                                                        
                                                         

                                                         
                                                                    

     
                                    
*/
WDI_Status
WDI_ProcessGtkOffloadRsp
(
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
)
{
   WDI_Status           wdiStatus;
   eHalStatus           halStatus;
   WDI_GtkOffloadCb    wdiGtkOffloadCb   = NULL;
   /*                                                                        */

   wdiGtkOffloadCb = (WDI_GtkOffloadCb)pWDICtx->pfncRspCB;

   /*                                                                         
                 
                                                                            */
   if (( NULL == pWDICtx ) || ( NULL == pEventData ) ||
       ( NULL == pEventData->pEventData))
   {
      WPAL_TRACE( eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_WARN,
                  "%s: Invalid parameters", __FUNCTION__);
      WDI_ASSERT(0);
      return WDI_STATUS_E_FAILURE;
   }

   /*                                                                         
                                         
                                                                            */
   halStatus = *((eHalStatus*)pEventData->pEventData);
   wdiStatus   =   WDI_HAL_2_WDI_STATUS(halStatus);

   /*           */
   wdiGtkOffloadCb( wdiStatus, pWDICtx->pRspCBUserData);

   return WDI_STATUS_SUCCESS;
}

/* 
                                                             

                                                         
                                                                    

     
                                    
*/
WDI_Status
WDI_ProcessGTKOffloadGetInfoRsp
(
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
)
{
   WDI_Status           wdiStatus;
   eHalStatus           halStatus;

   WDI_GtkOffloadGetInfoCb   wdiGtkOffloadGetInfoCb = NULL;

   /*                                                                        */

   wdiGtkOffloadGetInfoCb = (WDI_GtkOffloadGetInfoCb)pWDICtx->pfncRspCB;

   /*                                                                         
                 
                                                                            */
   if (( NULL == pWDICtx ) || ( NULL == pEventData ) ||
       ( NULL == pEventData->pEventData ))
   {
      WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_WARN,
                  "%s: Invalid parameters", __FUNCTION__);
      WDI_ASSERT(0);
      return WDI_STATUS_E_FAILURE;
   }

   /*                                                                         
                                         
                                                                            */
   halStatus = *((eHalStatus*)pEventData->pEventData);
   wdiStatus   =   WDI_HAL_2_WDI_STATUS(halStatus);

   /*           */
   //                                                          
   //                                                                  
   wdiGtkOffloadGetInfoCb(wdiStatus, pWDICtx->pRspCBUserData);

   return WDI_STATUS_SUCCESS;
}
#endif //                         

#ifdef WLAN_WAKEUP_EVENTS
WDI_Status
WDI_ProcessWakeReasonInd
(
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
)
{
  WDI_LowLevelIndType *pWdiInd;
  tpWakeReasonParams pWakeReasonParams;
  wpt_uint32 allocSize = 0;

  WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_INFO,
            "+%s", __FUNCTION__);

  /*                                                                         
                
                                                                           */
  if (( NULL == pWDICtx ) || ( NULL == pEventData ) ||
      ( NULL == pEventData->pEventData ))
  {
     WPAL_TRACE( eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_WARN,
                 "%s: Invalid parameters", __FUNCTION__);
     WDI_ASSERT( 0 );
     return WDI_STATUS_E_FAILURE;
  }

  /*                                                                         
                                          
                                                                           */
  pWakeReasonParams = (tpWakeReasonParams)(pEventData->pEventData);

  allocSize = sizeof(WDI_LowLevelIndType) + (pWakeReasonParams->ulStoredDataLen - 1);

  //                                                   
  pWdiInd = wpalMemoryAllocate(allocSize) ;

  if(NULL == pWdiInd)
  {
    WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_WARN,
            "%s: Failed to allocate memory for WDI_WakeReasonIndType: %x %x %x ",
                __FUNCTION__, pWDICtx, pEventData, pEventData->pEventData);
    WDI_ASSERT(0);
    return WDI_STATUS_E_FAILURE;
  }

  wpalMemoryZero(pWdiInd, allocSize);

  /*                                  */
  //                                                                                          
  pWdiInd->wdiIndicationType = WDI_WAKE_REASON_IND;
  pWdiInd->wdiIndicationData.wdiWakeReasonInd.ulReason = pWakeReasonParams->ulReason;
  pWdiInd->wdiIndicationData.wdiWakeReasonInd.ulReasonArg = pWakeReasonParams->ulReasonArg;
  pWdiInd->wdiIndicationData.wdiWakeReasonInd.ulStoredDataLen = pWakeReasonParams->ulStoredDataLen;
  pWdiInd->wdiIndicationData.wdiWakeReasonInd.ulActualDataLen = pWakeReasonParams->ulActualDataLen;
  wpalMemoryCopy( (void *)&(pWdiInd->wdiIndicationData.wdiWakeReasonInd.aDataStart[0]),
                  &(pWakeReasonParams->aDataStart[0]),
                  pWakeReasonParams->ulStoredDataLen);

  /*           */
  pWDICtx->wdiLowLevelIndCB( pWdiInd, pWDICtx->pIndUserData );

  //                                                         
  wpalMemoryFree(pWdiInd);

  WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_INFO,
            "-%s", __FUNCTION__);

  return WDI_STATUS_SUCCESS;
}
#endif //                   

void WDI_GetWcnssCompiledApiVersion
(
  WDI_WlanVersionType     *pWcnssApiVersion
)
{
    pWcnssApiVersion->major    = WLAN_HAL_VER_MAJOR;
    pWcnssApiVersion->minor    = WLAN_HAL_VER_MINOR;
    pWcnssApiVersion->version  = WLAN_HAL_VER_VERSION;
    pWcnssApiVersion->revision = WLAN_HAL_VER_REVISION;
}

/* 
                                                        
                                                         

                                                         
                                                                    

     
                                    
*/
WDI_Status
WDI_ProcessSetTmLevelRsp
(
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
)
{
   WDI_Status           wdiStatus;
   eHalStatus           halStatus;
   WDI_SetTmLevelCb     wdiSetTmLevelCb;
   /*                                                                        */

   /*                                                                         
                 
                                                                            */
   if (( NULL == pWDICtx ) || ( NULL == pEventData ) ||
       ( NULL == pEventData->pEventData ))
   {
      WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_WARN,
                  "%s: Invalid parameters", __FUNCTION__);
      WDI_ASSERT(0);
      return WDI_STATUS_E_FAILURE;
   }

   wdiSetTmLevelCb = (WDI_SetPowerParamsCb)pWDICtx->pfncRspCB;

   /*                                                                         
                                         
                                                                            */
   halStatus = *((eHalStatus*)pEventData->pEventData);
   wdiStatus   =   WDI_HAL_2_WDI_STATUS(halStatus);

   /*           */
   wdiSetTmLevelCb(wdiStatus, pWDICtx->pRspCBUserData);

   return WDI_STATUS_SUCCESS;
}/*                        */

/* 
                                                            

                                                         
                                                                    

     
                                    
*/
WDI_Status
WDI_ProcessSetTmLevelReq
(
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
)
{
   WDI_SetTmLevelReqType           *pwdiSetTmLevelReq = NULL;
   WDI_SetTmLevelCb                 wdiSetTmLevelCb   = NULL;
   wpt_uint8*                       pSendBuffer       = NULL;
   wpt_uint16                       usDataOffset      = 0;
   wpt_uint16                       usSendSize        = 0;
   tSetThermalMitgationType         halTmMsg;

   /*                                                                         
                 
                                                                            */
   if (( NULL == pEventData ) ||
       ( NULL == (pwdiSetTmLevelReq = (WDI_SetTmLevelReqType*)pEventData->pEventData)) ||
       ( NULL == (wdiSetTmLevelCb   = (WDI_SetTmLevelCb)pEventData->pCBfnc)))
   {
      WPAL_TRACE( eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_WARN,
                  "%s: Invalid parameters", __FUNCTION__);
      WDI_ASSERT(0);
      return WDI_STATUS_E_FAILURE;
   }

   /*                                                                       
                       
                                                                          */
   if (( WDI_STATUS_SUCCESS != WDI_GetMessageBuffer( pWDICtx, WDI_SET_TM_LEVEL_REQ,
                         sizeof(halTmMsg),
                         &pSendBuffer, &usDataOffset, &usSendSize))||
       ( usSendSize < (usDataOffset + sizeof(halTmMsg) )))
   {
      WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_WARN,
                  "Unable to get send buffer in Set PNO req %x %x %x",
                  pEventData, pwdiSetTmLevelReq, wdiSetTmLevelCb);
      WDI_ASSERT(0);
      return WDI_STATUS_E_FAILURE;
   }

   halTmMsg.thermalMitMode = pwdiSetTmLevelReq->tmMode;
   halTmMsg.thermalMitLevel = pwdiSetTmLevelReq->tmLevel;

   wpalMemoryCopy( pSendBuffer+usDataOffset,
                   &halTmMsg,
                   sizeof(halTmMsg));

   pWDICtx->pReqStatusUserData = pwdiSetTmLevelReq->pUserData;
   pWDICtx->pfncRspCB = NULL;
   /*                                                                         
                                
                                                                            */
   return  WDI_SendMsg( pWDICtx, pSendBuffer, usSendSize,
                        wdiSetTmLevelCb, pEventData->pUserData, WDI_SET_TM_LEVEL_RESP);
}

/*                                                                                 
                
 */
static void
FillAllFeatureCaps(tWlanFeatCaps *fCaps, placeHolderInCapBitmap *enabledFeat, wpt_int8 len)
{
   wpt_int8 i;
   for (i=0; i<len; i++)
   {
      setFeatCaps(fCaps, enabledFeat[i]);
   }
}

/* 
                                  
                                                      
                                                               
                                                                     

       

                                                                          
                                                                            
                                                       

                                                         
                

     
                                    
*/
WDI_Status
WDI_featureCapsExchangeReq
(
  WDI_featureCapsExchangeCb     wdiFeatureCapsExchangeCb,
  void*                         pUserData
)
{
   WDI_EventInfoType   wdiEventData;
   wpt_int32           fCapsStructSize;

   /*                                                                        
                 
                                                                           */
   if ( eWLAN_PAL_FALSE == gWDIInitialized )
   {
      WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_ERROR,
               "WDI API call before module is initialized - Fail request");

      return WDI_STATUS_E_NOT_ALLOWED;
   }

   /*                                                                 */
   fCapsStructSize = sizeof(tWlanFeatCaps);
   gpHostWlanFeatCaps = wpalMemoryAllocate(fCapsStructSize);
   if ( NULL ==  gpHostWlanFeatCaps )
   {
      WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_ERROR,
               "Cannot allocate memory for host capability info\n");
      WDI_ASSERT(0);
      return WDI_STATUS_MEM_FAILURE;
   }

   wpalMemoryZero(gpHostWlanFeatCaps, fCapsStructSize);

   /*                                                                        
                                              
                                                                           */
   FillAllFeatureCaps(gpHostWlanFeatCaps, supportEnabledFeatures,
      (sizeof(supportEnabledFeatures)/sizeof(supportEnabledFeatures[0])));
   WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_INFO,
      "Host caps %x %x %x %x\n",
      gpHostWlanFeatCaps->featCaps[0],
      gpHostWlanFeatCaps->featCaps[1],
      gpHostWlanFeatCaps->featCaps[2],
      gpHostWlanFeatCaps->featCaps[3]
   );

   wdiEventData.wdiRequest      = WDI_FEATURE_CAPS_EXCHANGE_REQ;
   wdiEventData.pEventData      = gpHostWlanFeatCaps;
   wdiEventData.uEventDataSize  = fCapsStructSize;
   wdiEventData.pCBfnc          = wdiFeatureCapsExchangeCb;
   wdiEventData.pUserData       = pUserData;

   return WDI_PostMainEvent(&gWDICb, WDI_REQUEST_EVENT, &wdiEventData);
}

/* 
                                                            

                                                         
                                                                    

     
                                    
*/
WDI_Status
WDI_ProcessFeatureCapsExchangeReq
(
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
)
{
  wpt_uint8*              pSendBuffer        = NULL;
  wpt_uint16              usDataOffset       = 0;
  wpt_uint16              usSendSize         = 0;
  wpt_uint16              usLen              = 0;

  /*                                                                        */

  WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_INFO,
         "%s", __FUNCTION__);

  /*                                                                         
                
                                                                           */
  /*                                                                    */
  if (( NULL == pEventData ) ||
      ( NULL == (tWlanFeatCaps *)pEventData->pEventData))
  {
     WPAL_TRACE( eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_FATAL,
                 "%s: Invalid parameters", __FUNCTION__);
     WDI_ASSERT(0);
     return WDI_STATUS_E_FAILURE;
  }

  /*                                                                       
                      
                                                                         */
  usLen = sizeof(tWlanFeatCaps);

  if (( WDI_STATUS_SUCCESS != WDI_GetMessageBuffer( pWDICtx,
                        WDI_FEATURE_CAPS_EXCHANGE_REQ,
                        usLen,
                        &pSendBuffer, &usDataOffset, &usSendSize))||
      ( usSendSize < (usDataOffset + usLen )))
  {
     WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_FATAL,
              "Unable to get send buffer in feat caps exchange req %x %x",
                pEventData, (tWlanFeatCaps *)pEventData->pEventData);
     WDI_ASSERT(0);
     return WDI_STATUS_E_FAILURE;
  }

  WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_INFO,
       "Host Caps - %x %x %x %x\n",
      ((tWlanFeatCaps *)pEventData->pEventData)->featCaps[0],
      ((tWlanFeatCaps *)pEventData->pEventData)->featCaps[1],
      ((tWlanFeatCaps *)pEventData->pEventData)->featCaps[2],
      ((tWlanFeatCaps *)pEventData->pEventData)->featCaps[3]
    );

  /*                                                    */
  wpalMemoryCopy( pSendBuffer+usDataOffset,
                  (tWlanFeatCaps *)pEventData->pEventData,
                  usLen);

  /*                                                                         
                             
                                                                           */
  return  WDI_SendMsg( pWDICtx, pSendBuffer, usSendSize,
                       (WDI_StartRspCb)pEventData->pCBfnc,
                       pEventData->pUserData, WDI_FEATURE_CAPS_EXCHANGE_RESP);

}/*                                 */

/* 
                                                             

                                                         
                                                                    

     
                                    
*/
WDI_Status
WDI_ProcessFeatureCapsExchangeRsp
(
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
)
{
   WDI_featureCapsExchangeCb    wdiFeatureCapsExchangeCb;
   wpt_int32                   fCapsStructSize;
   /*                                                                        */

   WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_INFO,
          "%s", __FUNCTION__);

   /*                                                                         
                 
                                                                            */
   if (( NULL == pWDICtx ) || ( NULL == pEventData ) ||
       ( NULL == pEventData->pEventData ))
   {
      /*                                                                                */
      WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_WARN,
                  "%s: Invalid parameters", __FUNCTION__);
      WDI_ASSERT(0);
      return WDI_STATUS_E_FAILURE;
   }

   /*                                                                 */
   fCapsStructSize = sizeof(tWlanFeatCaps);
   gpFwWlanFeatCaps = wpalMemoryAllocate(fCapsStructSize);
   if ( NULL ==  gpFwWlanFeatCaps )
   {
      WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_ERROR,
               "Cannot allocate memory for host capability info\n");
      WDI_ASSERT(0);
      return WDI_STATUS_MEM_FAILURE;
   }

   /*                                                                         
                                                                          
                                     
                                                                            */
   /*                                                                         
                                         
                                                                            */

   wpalMemoryCopy(gpFwWlanFeatCaps,(tWlanFeatCaps *) pEventData -> pEventData,
                    fCapsStructSize);
   WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_INFO,
      "FW caps %x %x %x %x\n",
      gpFwWlanFeatCaps->featCaps[0],
      gpFwWlanFeatCaps->featCaps[1],
      gpFwWlanFeatCaps->featCaps[2],
      gpFwWlanFeatCaps->featCaps[3]
     );

   wdiFeatureCapsExchangeCb = (WDI_featureCapsExchangeCb) pWDICtx -> pfncRspCB;

   /*                                                                               */
   if (wdiFeatureCapsExchangeCb != NULL)
      wdiFeatureCapsExchangeCb(NULL, NULL);

   return WDI_STATUS_SUCCESS;
}

/* 
                               
                                                                              
                                                                                
                                                                                 
                                                                                
                                                  

       

                                                                                                   

     
        
                                                   
                                                                 
*/
wpt_uint8 WDI_getHostWlanFeatCaps(wpt_uint8 feat_enum_value)
{
   wpt_uint8 featSupported = 0;
   if (gpHostWlanFeatCaps != NULL)
   {
      getFeatCaps(gpHostWlanFeatCaps, feat_enum_value, featSupported);
   }
   else
   {
      WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_ERROR,
        "Caps exchange feature NOT supported. Return NOT SUPPORTED for %u feature", feat_enum_value);
   }
   return featSupported;
}

/* 
                             
                                                                              
                                                                              
                                                                                 
                                                                                
                                                  

       

                                                                                
                                                      

     
        
                                                 
                                                               
*/
wpt_uint8 WDI_getFwWlanFeatCaps(wpt_uint8 feat_enum_value)
{
    wpt_uint8 featSupported = 0;
    if (gpFwWlanFeatCaps != NULL)
    {
      getFeatCaps(gpFwWlanFeatCaps, feat_enum_value, featSupported);
    }
    else
    {
       WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_ERROR,
         "Caps exchange feature NOT supported. Return NOT SUPPORTED for %u feature", feat_enum_value);
    }
    return featSupported;
}

/* 
                                   
                                             
                                                    
                                                            

                                                      
                                                        
                                                                         
                                                             
     
             
*/
void WDI_TransportChannelDebug
(
   wpt_boolean  displaySnapshot,
   wpt_boolean  toggleStallDetect
)
{
   WDTS_ChannelDebug(displaySnapshot, toggleStallDetect);
   return;
}