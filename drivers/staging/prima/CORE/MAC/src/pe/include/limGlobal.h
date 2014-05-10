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
#ifndef __LIM_GLOBAL_H
#define __LIM_GLOBAL_H

#include "wniApi.h"
#include "sirApi.h"
#include "sirMacProtDef.h"
#include "sirMacPropExts.h"
#include "sirCommon.h"
#include "sirDebug.h"
#include "wniCfgAp.h"
#ifdef WLAN_SOFTAP_FEATURE
#include "csrApi.h"
#include "sapApi.h"
#endif

//                                           
#define LIM_MAX_NUM_OF_SCAN_RESULTS 256

//                                                          
#define LINK_TEST_STATUS_SUCCESS                0x1
#define LINK_TEST_STATUS_UNSUPPORTED_RATE       0x2
#define LINK_TEST_STATUS_INVALID_ADDR           0x3

//                                                    
//                          
#define LIM_RHP_WORK_AROUND_DURATION 1000000

//                                             
#define LIM_MAX_QUIET_DURATION 32

#define LIM_TX_WQ_EMPTY_SLEEP_NS                100000

//                                      
#define LIM_SEND_DISASSOC_FRAME_THRESHOLD       2
#define LIM_HASH_MISS_TIMER_MS                  10000

//                              
#define MAX_DEFERRED_QUEUE_LEN                  20

//                    
#define LIM_MAX_BUF_SIZE                        8192

//                                                                 
#define LIM_TIM_WAIT_COUNT_FACTOR          5

/*                                                                                            */
#define LIM_MIN_TIM_WAIT_COUNT          50  

#define GET_TIM_WAIT_COUNT(LIntrvl)        ((LIntrvl * LIM_TIM_WAIT_COUNT_FACTOR) > LIM_MIN_TIM_WAIT_COUNT ? \
                                                                    (LIntrvl * LIM_TIM_WAIT_COUNT_FACTOR) : LIM_MIN_TIM_WAIT_COUNT)

//                                     

//                        
typedef enum eLimSystemRole
{
    eLIM_UNKNOWN_ROLE,
    eLIM_AP_ROLE,
    eLIM_STA_IN_IBSS_ROLE,
    eLIM_STA_ROLE,
    eLIM_BT_AMP_STA_ROLE,
    eLIM_BT_AMP_AP_ROLE
#ifdef WLAN_FEATURE_P2P
    ,eLIM_P2P_DEVICE_ROLE
    ,eLIM_P2P_DEVICE_GO
    ,eLIM_P2P_DEVICE_CLINET
#endif
} tLimSystemRole;

/* 
                                                             
                                                  
                                     
                                                    
                                                               
 */
typedef enum eLimSmeStates
{
    eLIM_SME_OFFLINE_STATE,
    eLIM_SME_IDLE_STATE,
    eLIM_SME_SUSPEND_STATE,
    eLIM_SME_WT_SCAN_STATE,
    eLIM_SME_WT_JOIN_STATE,
    eLIM_SME_WT_AUTH_STATE,
    eLIM_SME_WT_ASSOC_STATE,
    eLIM_SME_WT_REASSOC_STATE,
    eLIM_SME_WT_REASSOC_LINK_FAIL_STATE,
    eLIM_SME_JOIN_FAILURE_STATE,
    eLIM_SME_ASSOCIATED_STATE,
    eLIM_SME_REASSOCIATED_STATE,
    eLIM_SME_LINK_EST_STATE,
    eLIM_SME_LINK_EST_WT_SCAN_STATE,
    eLIM_SME_WT_PRE_AUTH_STATE,
    eLIM_SME_WT_DISASSOC_STATE,
    eLIM_SME_WT_DEAUTH_STATE,
    eLIM_SME_WT_START_BSS_STATE,
    eLIM_SME_WT_STOP_BSS_STATE,
    eLIM_SME_NORMAL_STATE,
    eLIM_SME_CHANNEL_SCAN_STATE,
    eLIM_SME_NORMAL_CHANNEL_SCAN_STATE
} tLimSmeStates;

/* 
                        
                                                      
                                                  
                           
 */
typedef enum eLimMlmStates
{
    eLIM_MLM_OFFLINE_STATE,
    eLIM_MLM_IDLE_STATE,
    eLIM_MLM_WT_PROBE_RESP_STATE,
    eLIM_MLM_PASSIVE_SCAN_STATE,
    eLIM_MLM_WT_JOIN_BEACON_STATE,
    eLIM_MLM_JOINED_STATE,
    eLIM_MLM_BSS_STARTED_STATE,
    eLIM_MLM_WT_AUTH_FRAME2_STATE,
    eLIM_MLM_WT_AUTH_FRAME3_STATE,
    eLIM_MLM_WT_AUTH_FRAME4_STATE,
    eLIM_MLM_AUTH_RSP_TIMEOUT_STATE,
    eLIM_MLM_AUTHENTICATED_STATE,
    eLIM_MLM_WT_ASSOC_RSP_STATE,
    eLIM_MLM_WT_REASSOC_RSP_STATE,
    eLIM_MLM_ASSOCIATED_STATE,
    eLIM_MLM_REASSOCIATED_STATE,
    eLIM_MLM_LINK_ESTABLISHED_STATE,
    eLIM_MLM_WT_ASSOC_CNF_STATE,
    eLIM_MLM_LEARN_STATE,
    eLIM_MLM_WT_ADD_BSS_RSP_STATE,
    eLIM_MLM_WT_DEL_BSS_RSP_STATE,
    eLIM_MLM_WT_ADD_BSS_RSP_ASSOC_STATE,
    eLIM_MLM_WT_ADD_BSS_RSP_REASSOC_STATE,
    eLIM_MLM_WT_ADD_BSS_RSP_PREASSOC_STATE,
    eLIM_MLM_WT_ADD_STA_RSP_STATE,
    eLIM_MLM_WT_DEL_STA_RSP_STATE,
    //                                                                                       
    //                                                                                   
    //                                                                                     
    //                                        
    eLIM_MLM_WT_ASSOC_DEL_STA_RSP_STATE,
    eLIM_MLM_WT_SET_BSS_KEY_STATE,
    eLIM_MLM_WT_SET_STA_KEY_STATE,
    eLIM_MLM_WT_SET_STA_BCASTKEY_STATE,    
    eLIM_MLM_WT_ADDBA_RSP_STATE,
    eLIM_MLM_WT_REMOVE_BSS_KEY_STATE,
    eLIM_MLM_WT_REMOVE_STA_KEY_STATE,
    eLIM_MLM_WT_SET_MIMOPS_STATE,
#if defined WLAN_FEATURE_VOWIFI_11R
    eLIM_MLM_WT_ADD_BSS_RSP_FT_REASSOC_STATE,
    eLIM_MLM_WT_FT_REASSOC_RSP_STATE,
#endif
#ifdef WLAN_FEATURE_P2P
    eLIM_MLM_P2P_LISTEN_STATE,
#endif
} tLimMlmStates;

//                         
/*                                                    
                                                              
                                                       
                                                  
                                                    
                                                            
                                          
 */
typedef enum eLimQuietStates
{
    eLIM_QUIET_INIT,
    eLIM_QUIET_BEGIN,
    eLIM_QUIET_CHANGED,
    eLIM_QUIET_RUNNING,
    eLIM_QUIET_END
} tLimQuietStates;

//                          
/*                                                     
                          
                                       
                                                           
                                                         
 */
typedef enum eLimDot11hChanSwStates
{
    eLIM_11H_CHANSW_INIT,
    eLIM_11H_CHANSW_RUNNING,
    eLIM_11H_CHANSW_END
} tLimDot11hChanSwStates;

#ifdef GEN4_SCAN

//                         
typedef void (*SUSPEND_RESUME_LINK_CALLBACK)(tpAniSirGlobal pMac, eHalStatus status, tANI_U32 *data);

//                                                    
typedef enum eLimHalScanState
{
  eLIM_HAL_IDLE_SCAN_STATE,
  eLIM_HAL_INIT_SCAN_WAIT_STATE,
  eLIM_HAL_START_SCAN_WAIT_STATE,
  eLIM_HAL_END_SCAN_WAIT_STATE,
  eLIM_HAL_FINISH_SCAN_WAIT_STATE,
  eLIM_HAL_INIT_LEARN_WAIT_STATE,
  eLIM_HAL_START_LEARN_WAIT_STATE,
  eLIM_HAL_END_LEARN_WAIT_STATE,
  eLIM_HAL_FINISH_LEARN_WAIT_STATE,
  eLIM_HAL_SCANNING_STATE,
//                         
  eLIM_HAL_SUSPEND_LINK_WAIT_STATE,
  eLIM_HAL_SUSPEND_LINK_STATE,
  eLIM_HAL_RESUME_LINK_WAIT_STATE,
//                             
} tLimLimHalScanState;
#endif //          

//                                
//                                                                
typedef enum eLimBAState
{
  eLIM_BA_STATE_IDLE, //                                          
  eLIM_BA_STATE_WT_ADD_RSP, //                                        
  eLIM_BA_STATE_WT_DEL_RSP //                                           
} tLimBAState;

#if (WNI_POLARIS_FW_PRODUCT == AP) && (WNI_POLARIS_FW_PACKAGE == ADVANCED)
typedef struct sLimMeasParams
{
    TX_TIMER    measurementIndTimer;
    TX_TIMER    learnIntervalTimer;
    TX_TIMER    learnDurationTimer;
    tANI_U32    rssiAlpha;
    tANI_U32    chanUtilAlpha;
    tANI_U8     shortDurationCount;
    tANI_U8     isMeasIndTimerActive:1;
    tANI_U8     disableMeasurements:1;
    tANI_U8     rsvd:6;
    tANI_U8     nextLearnChannelId;
} tLimMeasParams, *tpLimMeasParams;

typedef struct tLimMeasMatrixNode tLimMeasMatrixNode,
                                  *tpLimMeasMatrixNode;
struct tLimMeasMatrixNode
{
    tSirMeasMatrixInfo     matrix;
    tANI_U32                    avgRssi;
    tpLimMeasMatrixNode    next;
};

typedef struct tLimNeighborBssWdsNode tLimNeighborBssWdsNode,
                                      *tpLimNeighborBssWdsNode;
struct tLimNeighborBssWdsNode
{
    tSirNeighborBssWdsInfo     info;
    tpLimNeighborBssWdsNode    next;
};

typedef struct sLimMeasData
{
    tANI_U32                duration;  
    tANI_U32                prevTsfLo;
    tANI_U32                prevChannelUtilization;
    tANI_U16                avgChannelUtilization;
    tANI_U8                 numMatrixNodes;
    tpLimMeasMatrixNode     pMeasMatrixInfo;
    tANI_U32                numBssWds;
    tANI_U16                totalBssSize;
    tpLimNeighborBssWdsNode pNeighborWdsInfo;
} tLimMeasData, *tpLimMeasData;
#endif

#if (WNI_POLARIS_FW_PACKAGE == ADVANCED)
//                                   
typedef struct sSirMultipleAlternateRadioInfo
{
    tANI_U8                       numBss;
    tSirAlternateRadioInfo   alternateRadio[SIR_MAX_NUM_ALTERNATE_RADIOS];
} tSirMultipleAlternateRadioInfo, *tpSirMultipleAlternateRadioInfo;

//                                  
typedef struct sSirMultipleNeighborBssInfo
{
    tANI_U32                  numBss;
    tSirNeighborBssInfo  bssList[SIR_MAX_NUM_NEIGHBOR_BSS];
} tSirMultipleNeighborBssInfo, *tpSirMultipleNeighborBssInfo;
#endif


#if (WNI_POLARIS_FW_PRODUCT == AP)
/* 
                                                      
                                                    
                
 */
typedef struct sLimAIDtbr
{
    tANI_U8 tbr:1;
    tANI_U8 seen:1;
} tLimAIDtbr;
#endif

//                                  
typedef struct sLimMlmAuthReq
{
    tSirMacAddr    peerMacAddr;
    tAniAuthType   authType;
    tANI_U32       authFailureTimeout;
    tANI_U8        sessionId; 
} tLimMlmAuthReq, *tpLimMlmAuthReq;

typedef struct sLimMlmJoinReq
{
    tANI_U32               joinFailureTimeout;
    tSirMacRateSet         operationalRateSet;
    tANI_U8                 sessionId;
    tSirBssDescription     bssDescription;
} tLimMlmJoinReq, *tpLimMlmJoinReq;

typedef struct sLimMlmScanReq
{
    tSirBssType        bssType;
    tSirMacAddr        bssId;
    tSirMacSSid        ssId[SIR_SCAN_MAX_NUM_SSID];
    tSirScanType       scanType;
    tANI_U32           minChannelTime;
    tANI_U32           maxChannelTime;
    tSirBackgroundScanMode  backgroundScanMode;
    tANI_U32 dot11mode;
    /*                                             */
    tANI_U8            numSsid;

#ifdef WLAN_FEATURE_P2P
    tANI_BOOLEAN   p2pSearch;
    tANI_BOOLEAN   skipDfsChnlInP2pSearch;
#endif
    tANI_U16           uIEFieldLen;
    tANI_U16           uIEFieldOffset;

    //                                                    
    tSirChannelList    channelList;
    /*                             
                        
                                   
                  
                                   
                                        
                                        
                                        
                                        
                                        
                                        
                                        
                                        
                                        
                                 
                                  
                                   */
} tLimMlmScanReq, *tpLimMlmScanReq;

typedef struct tLimScanResultNode tLimScanResultNode;
struct tLimScanResultNode
{
    tLimScanResultNode *next;
    tSirBssDescription bssDescription;
};

#ifdef FEATURE_OEM_DATA_SUPPORT

#ifndef OEM_DATA_REQ_SIZE 
#define OEM_DATA_REQ_SIZE 70
#endif
#ifndef OEM_DATA_RSP_SIZE
#define OEM_DATA_RSP_SIZE 968
#endif

//                                       
typedef struct sLimMlmOemDataReq
{
    tSirMacAddr           selfMacAddr;
    tANI_U8               oemDataReq[OEM_DATA_REQ_SIZE];
} tLimMlmOemDataReq, *tpLimMlmOemDataReq;

typedef struct sLimMlmOemDataRsp
{
   tANI_U8                oemDataRsp[OEM_DATA_RSP_SIZE];
} tLimMlmOemDataRsp, *tpLimMlmOemDataRsp;
#endif

//                                        
typedef struct tLimPreAuthNode
{
    struct tLimPreAuthNode     *next;
    tSirMacAddr         peerMacAddr;
    tAniAuthType        authType;
    tLimMlmStates       mlmState;
    tANI_U8             authNodeIdx;
    tANI_U8             challengeText[SIR_MAC_AUTH_CHALLENGE_LENGTH];
    tANI_U8             fTimerStarted:1;
    tANI_U8             fSeen:1;
    tANI_U8             fFree:1;
    tANI_U8             rsvd:5;
    TX_TIMER            timer;
}tLimPreAuthNode, *tpLimPreAuthNode;

//                                    
typedef struct tLimPreAuthTable
{
    tANI_U32        numEntry;
    tpLimPreAuthNode pTable;
}tLimPreAuthTable, *tpLimPreAuthTable;

//                                      
typedef struct sLimMlmStaContext
{
    tLimMlmStates           mlmState;
    tAniAuthType            authType;
    tANI_U16                listenInterval;
    tSirMacCapabilityInfo   capabilityInfo;
    tSirMacPropRateSet      propRateSet;
    tSirMacReasonCodes      disassocReason;
    tANI_U16                cleanupTrigger;

    tSirResultCodes resultCode;
    tANI_U16 protStatusCode;
    
    tANI_U8                 subType:1; //                                   
    tANI_U8                 updateContext:1;
    tANI_U8                 schClean:1;
    //                                                          
    tANI_U8                 htCapability:1;
#ifdef WLAN_FEATURE_11AC
    tANI_U8                 vhtCapability:1;
#endif
} tLimMlmStaContext, *tpLimMlmStaContext;

//                                                                
typedef struct sLimDeferredMsgQParams
{
    tSirMsgQ    deferredQueue[MAX_DEFERRED_QUEUE_LEN];
    tANI_U16         size;
    tANI_U16         read;
    tANI_U16         write;
} tLimDeferredMsgQParams, *tpLimDeferredMsgQParams;

typedef struct sLimTraceQ
{
    tANI_U32                type;
    tLimSmeStates      smeState;
    tLimMlmStates      mlmState;
    tANI_U32                value;
    tANI_U32                value2;
} tLimTraceQ;

typedef struct sLimTraceParams
{
    tLimTraceQ    traceQueue[1024];
    tANI_U16           write;
    tANI_U16           enabled;
} tLimTraceParams;

typedef struct sCfgProtection
{
    tANI_U32 overlapFromlla:1;
    tANI_U32 overlapFromllb:1;
    tANI_U32 overlapFromllg:1;
    tANI_U32 overlapHt20:1;
    tANI_U32 overlapNonGf:1;
    tANI_U32 overlapLsigTxop:1;
    tANI_U32 overlapRifs:1;
    tANI_U32 overlapOBSS:1; /*                */
    tANI_U32 fromlla:1;
    tANI_U32 fromllb:1;
    tANI_U32 fromllg:1;
    tANI_U32 ht20:1;
    tANI_U32 nonGf:1;
    tANI_U32 lsigTxop:1;
    tANI_U32 rifs:1;
    tANI_U32 obss:1; /*                */
}tCfgProtection, *tpCfgProtection;

typedef enum eLimProtStaCacheType
{
    eLIM_PROT_STA_CACHE_TYPE_INVALID,
    eLIM_PROT_STA_CACHE_TYPE_llB,
    eLIM_PROT_STA_CACHE_TYPE_llG,  
    eLIM_PROT_STA_CACHE_TYPE_HT20
}tLimProtStaCacheType;

typedef struct sCacheParams
{
    tANI_U8        active;
    tSirMacAddr   addr;    
    tLimProtStaCacheType protStaCacheType;
    
} tCacheParams, *tpCacheParams;

#ifdef ANI_PRODUCT_TYPE_AP
#define LIM_PROT_STA_OVERLAP_CACHE_SIZE     10
#define LIM_PROT_STA_CACHE_SIZE 256
#else
#ifdef WLAN_SOFTAP_FEATURE
#define LIM_PROT_STA_OVERLAP_CACHE_SIZE    HAL_NUM_ASSOC_STA
#define LIM_PROT_STA_CACHE_SIZE            HAL_NUM_ASSOC_STA
#else
#define LIM_PROT_STA_OVERLAP_CACHE_SIZE    5
#define LIM_PROT_STA_CACHE_SIZE            5
#endif
#endif

typedef struct sLimProtStaParams
{
    tANI_U8               numSta;
    tANI_U8               protectionEnabled;
} tLimProtStaParams, *tpLimProtStaParams;


typedef struct sLimNoShortParams
{
    tANI_U8           numNonShortPreambleSta;
    tCacheParams      staNoShortCache[LIM_PROT_STA_CACHE_SIZE];
} tLimNoShortParams, *tpLimNoShortParams;

typedef struct sLimNoShortSlotParams
{
    tANI_U8           numNonShortSlotSta;
    tCacheParams      staNoShortSlotCache[LIM_PROT_STA_CACHE_SIZE];
} tLimNoShortSlotParams, *tpLimNoShortSlotParams;


typedef struct tLimIbssPeerNode tLimIbssPeerNode;
struct tLimIbssPeerNode
{
    tLimIbssPeerNode         *next;
    tSirMacAddr              peerMacAddr;
    tANI_U8                       aniIndicator:1;
    tANI_U8                       extendedRatesPresent:1;
    tANI_U8                       edcaPresent:1;
    tANI_U8                       wmeEdcaPresent:1;
    tANI_U8                       wmeInfoPresent:1;
    tANI_U8                       htCapable:1;
    tANI_U8                       rsvd:2;
    tSirMacCapabilityInfo    capabilityInfo;
    tSirMacRateSet           supportedRates;
    tSirMacRateSet           extendedRates;
    tANI_U8                   supportedMCSSet[SIZE_OF_SUPPORTED_MCS_SET];
    tSirMacEdcaParamSetIE    edcaParams;
    tANI_U16 propCapability;
    tANI_U8  erpIePresent;

    //                            
    tANI_U8 htGreenfield;
    tANI_U8 htShortGI40Mhz;
    tANI_U8 htShortGI20Mhz;

    //                                          
    tANI_U8 htDsssCckRate40MHzSupport;

    //                
    tSirMacHTMIMOPowerSaveState htMIMOPSState;

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
    tANI_U8 htAMpduDensity;

    //                         
    tANI_U8 htMaxRxAMpduFactor;

    //                        
    //                        
    tANI_U8 htMaxAmsduLength;

    //
    //                         
    //                                         
    //                       
    //
    tANI_U8 htSupportedChannelWidthSet;

    tANI_U8 beaconHBCount;
    tANI_U8 heartbeatFailure;

    tANI_U8 *beacon; //                                 
    tANI_U16 beaconLen;
    
};

//                                  
typedef enum eLimChannelSwitchState
{
    eLIM_CHANNEL_SWITCH_IDLE,
    eLIM_CHANNEL_SWITCH_PRIMARY_ONLY,
    eLIM_CHANNEL_SWITCH_SECONDARY_ONLY,
    eLIM_CHANNEL_SWITCH_PRIMARY_AND_SECONDARY
} tLimChannelSwitchState;


//                    
typedef struct sLimChannelSwitchInfo
{
    tLimChannelSwitchState   state;
    tANI_U8                  primaryChannel;
    ePhyChanBondState        secondarySubBand;
    tANI_U32                 switchCount;
    tANI_U32                 switchTimeoutValue;
    tANI_U8                  switchMode;
} tLimChannelSwitchInfo, *tpLimChannelSwitchInfo;

//                                 
typedef enum eLimQuietTxMode
{
    eLIM_TX_ALL = 0,       /*                                                                       */
    eLIM_TX_STA,           /*                                                                          */
    eLIM_TX_BSS,           /*                                                          */
    eLIM_TX_BSS_BUT_BEACON /*                                                                    
                                                                                                                        
                                                                                                                    
                                                                            */
} tLimQuietTxMode;

typedef enum eLimControlTx
{
    eLIM_RESUME_TX = 0,
    eLIM_STOP_TX
} tLimControlTx;

#ifdef ANI_AP_SDK
typedef struct sLimScanDurationConvert
{
    tANI_U32 shortChannelScanDuration_tick; /*                                                                                  */
    tANI_U32 shortChannelScanDuration_TU; /*                                                  */
    tANI_U32 longChannelScanDuration_tick; /*                                                                                  */
    tANI_U32 longChannelScanDuration_TU; /*                                                  */
} tLimScanDurationConvert, *tpLimScanDurationConvert;
#endif /*            */

//                                                                     

typedef __ani_attr_pre_packed struct sLimTspecInfo
{
    tANI_U8          inuse;       //                   
    tANI_U8          idx;         //              
    tANI_U8          staAddr[6];
    tANI_U16         assocId;
    tSirMacTspecIE   tspec;
    tANI_U8          numTclas; //                         
    tSirTclasInfo    tclasInfo[SIR_MAC_TCLASIE_MAXNUM];
    tANI_U8          tclasProc;
    tANI_U8          tclasProcPresent:1; //                                             
} __ani_attr_packed tLimTspecInfo, *tpLimTspecInfo;

typedef struct sLimAdmitPolicyInfo
{
    tANI_U8          type;      //                          
    tANI_U8          bw_factor; //                                                     
                              //                                        
} tLimAdmitPolicyInfo, *tpLimAdmitPolicyInfo;


typedef enum eLimWscEnrollState
{
    eLIM_WSC_ENROLL_NOOP,
    eLIM_WSC_ENROLL_BEGIN,
    eLIM_WSC_ENROLL_IN_PROGRESS,
    eLIM_WSC_ENROLL_END
    
} tLimWscEnrollState;

#define WSC_PASSWD_ID_PUSH_BUTTON         (0x0004)

typedef struct sLimWscIeInfo
{
    tANI_BOOLEAN       apSetupLocked;
    tANI_BOOLEAN       selectedRegistrar;
    tANI_U16           selectedRegistrarConfigMethods;
    tLimWscEnrollState wscEnrollmentState;
    tLimWscEnrollState probeRespWscEnrollmentState;
    tANI_U8            reqType;
    tANI_U8            respType;
} tLimWscIeInfo, *tpLimWscIeInfo;

//                                    
#define LIM_NUM_TSPEC_MAX      4


//                                       
typedef struct sLimSpecMgmtInfo
{
    tLimQuietStates    quietState;
    tANI_U32           quietCount;
    tANI_U32           quietDuration;    /*                                  */
    tANI_U32           quietDuration_TU; /*                                                       */
    tANI_U32           quietTimeoutValue; /*                                         */
    tANI_BOOLEAN       fQuietEnabled;    /*                                                 */

    tLimDot11hChanSwStates dot11hChanSwState;
        
    tANI_BOOLEAN       fRadarDetCurOperChan; /*                                       */
    tANI_BOOLEAN       fRadarIntrConfigured; /*                                             */
}tLimSpecMgmtInfo, *tpLimSpecMgmtInfo;
#endif
