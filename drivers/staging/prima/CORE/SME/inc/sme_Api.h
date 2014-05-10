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

#if !defined( __SME_API_H )
#define __SME_API_H


/**=========================================================================
  
  \file  smeApi.h
  
  \brief prototype for SME APIs
  
   Copyright 2008 (c) Qualcomm, Incorporated.  All Rights Reserved.
   
   Qualcomm Confidential and Proprietary.
  
  ========================================================================*/

/*          */

/*                                                                          
               
                                                                          */
#include "ccmApi.h"
#include "csrApi.h"
#include "pmcApi.h"
#include "vos_mq.h"
#include "vos_lock.h"
#include "halTypes.h"
#include "sirApi.h"
#include "btcApi.h"
#include "vos_nvitem.h"
#include "p2p_Api.h"
#ifdef FEATURE_WLAN_NON_INTEGRATED_SOC
#include "halFw.h"
#endif

#ifdef FEATURE_OEM_DATA_SUPPORT
#include "oemDataApi.h"
#endif

#if defined WLAN_FEATURE_VOWIFI
#include "smeRrmInternal.h"
#endif

/*                                                                           
                                        
                                                                          */

#define SME_SUMMARY_STATS         1
#define SME_GLOBAL_CLASSA_STATS   2
#define SME_GLOBAL_CLASSB_STATS   4
#define SME_GLOBAL_CLASSC_STATS   8
#define SME_GLOBAL_CLASSD_STATS  16
#define SME_PER_STA_STATS        32

#define SME_INVALID_COUNTRY_CODE "XX"

/*                                                                           
                   
                                                                          */
typedef struct _smeConfigParams
{
   tCsrConfigParam  csrConfig;
#if defined WLAN_FEATURE_VOWIFI
   tRrmConfigParam  rrmConfig;
#endif
#if defined FEATURE_WLAN_LFR
    tANI_U8   isFastRoamIniFeatureEnabled;
#endif
#if defined FEATURE_WLAN_CCX
    tANI_U8   isCcxIniFeatureEnabled;
#endif
#if defined WLAN_FEATURE_P2P_INTERNAL
   tP2PConfigParam  p2pConfig;
#endif
#if  defined (WLAN_FEATURE_VOWIFI_11R) || defined (FEATURE_WLAN_CCX) || defined(FEATURE_WLAN_LFR)
    tANI_U8   isFastTransitionEnabled;
    tANI_U8   RoamRssiDiff;
#endif
} tSmeConfigParams, *tpSmeConfigParams;


/*                                                                          
                                        
                                                                          */

/*                                                                          
  
                                                                          
  
                                                                              
                                                                    

                                                                    
                                       
  
                                               
  
                                                                
  
                                                                 
     
  
                                                                            */
eHalStatus sme_Open(tHalHandle hHal);

/*                                                                          
  
                                                                   
  
                                                                      
                                          

                                                               
                                           
  
                                               
  
                                                          
  
                                                                               
                                                      
     
  
                                                                            */
eHalStatus sme_Close(tHalHandle hHal);

/*                                                                          
  
                                                          
  
                                                                     
                                                    

                                               
  
                                             
  
                                                         
     
  
                                                                            */
eHalStatus sme_Start(tHalHandle hHal);

/*                                                                          
  
                                                                     
  
                                                                    
                                                       

  
                                               

                                                                   
  
                                               
  
                                                                                    
                         
     
  
                                                                            */
eHalStatus sme_Stop(tHalHandle hHal, tANI_BOOLEAN pmcFlag);


/*                                                                          
  
                                                                     
  
                            

  
                                               
                                                                                                 
                                                    
                                                                                      
                                                                                   
  
                                                                      
  
                                                                 
                                                            
     
  
                                                                            */
eHalStatus sme_OpenSession(tHalHandle hHal, csrRoamCompleteCallback callback, void *pContext, 
                           tANI_U8 *pSelfMacAddr, tANI_U8 *pbSessionId);


/*                                                                          
  
                                                                      
  
                            

  
                                               

                                                    
  
                                                   
  
                                                                 
                                                                 
     
  
                                                                            */
eHalStatus sme_CloseSession(tHalHandle hHal, tANI_U8 sessionId,
                         csrRoamSessionCloseCallback callback, void *pContext);



/*                                                                          
  
                                                                       
  
                                                                           
                                     
   
                                                                         

  
                                               
                                                                      
                                  
  
                                                                              
  
                                                                           
     
  
                                                                            */
eHalStatus sme_UpdateConfig(tHalHandle hHal, tpSmeConfigParams pSmeConfigParams);

#ifdef FEATURE_WLAN_SCAN_PNO
/*                                                                          

                                                                          
 
                                                                      
                 

                             

                                               
  
                                                                           

                                                                        
     

                                                                            */
eHalStatus sme_UpdateChannelConfig(tHalHandle hHal);

#endif //                       
#ifdef WLAN_SOFTAP_FEATURE
/*                                                                          
  
                                                                        
                                                
                                       

                             

                                               
                                                                     
                                  

                                                                              

                                                                           
     
                                                                          */

eHalStatus sme_set11dinfo(tHalHandle hHal,  tpSmeConfigParams pSmeConfigParams);

/*                                                                          

                                                                             

                             

                                                      
                                                                            

                                                                       

                                                                          
     
                                                                          */

eHalStatus sme_getSoftApDomain(tHalHandle hHal,  v_REGDOMAIN_t *domainIdSoftAp);

eHalStatus sme_setRegInfo(tHalHandle hHal,  tANI_U8 *apCntryCode);

#endif

/*                                                                            
                              
                                                                              
                                 
    
                                                                          
                                                                              
                                                                            
                                                                         

                               
    
                                                 

          
                                                                               
                                                                              
                                                                               
                                                                         

                           
                                                                             */
eHalStatus sme_ChangeConfigParams(tHalHandle hHal, 
                                 tCsrUpdateConfigParam *pUpdateConfigParam);

/*                                                                          
  
                                                                                          
                   
  
                                                                                                
                    
  
  
                                               
  
                                                                                  
  
                                                                  
     
  
                                                                            */
eHalStatus sme_HDDReadyInd(tHalHandle hHal);


/*                                                                          
  
                                                               
  
                                                                           
                    
  
  
                                               
                                                                   
  
                                                                     
  
                                                               
     
  
                                                                            */
eHalStatus sme_ProcessMsg(tHalHandle hHal, vos_msg_t* pMsg);

v_VOID_t sme_FreeMsg( tHalHandle hHal, vos_msg_t* pMsg );

/*                                                                            
                       
                                                                     
                                                                           
                                                                                
                                                               
                                                          
                           
                                                                             */
eHalStatus sme_ScanRequest(tHalHandle hHal, tANI_U8 sessionId, tCsrScanRequest *, 
                           tANI_U32 *pScanRequestID, 
                           csrScanCompleteCallback callback, void *pContext);


/*                                                                            
                               
                                                                        
                                               
                           
                                                                             */
eHalStatus sme_ScanSetBGScanparams(tHalHandle hHal, tANI_U8 sessionId, tCsrBGScanRequest *pScanReq);


/*                                                                            
                         
                                                               
                                                                        
                                               
                           
                                                                             */
eHalStatus sme_ScanGetResult(tHalHandle hHal, tANI_U8 sessionId, tCsrScanResultFilter *pFilter, 
                            tScanResultHandle *phResult);


/*                                                                            
                           
                                                                   
                           
                                                                             */
eHalStatus sme_ScanFlushResult(tHalHandle hHal, tANI_U8 sessionId);
eHalStatus sme_ScanFlushP2PResult(tHalHandle hHal, tANI_U8 sessionId);

/*                                                                            
                              
                                                                             
                       
                                                       
                                                         
                                                                             */
tCsrScanResultInfo *sme_ScanResultGetFirst(tHalHandle, 
                                          tScanResultHandle hScanResult);

/*                                                                            
                             
                                                                            
                                                                               
                
                                                       
                                                   
                                                                             */
tCsrScanResultInfo *sme_ScanResultGetNext(tHalHandle, 
                                          tScanResultHandle hScanResult);

/*                                                                            
                           
                                                                                 
                                                    
                                                                        
                                            
                                                                
                           
                                                                             */
eHalStatus sme_ScanResultPurge(tHalHandle hHal, tScanResultHandle hScanResult);

/*                                                                            
                                     
                                                                
                                                                     
                                           
                                                                    
                                                                             
                                                                            
                                 
                                                                                 
                                                 
                                          
                                                         
                                                      
                                                                             */
eHalStatus sme_ScanGetPMKIDCandidateList(tHalHandle hHal, tANI_U8 sessionId,
                                        tPmkidCandidateInfo *pPmkidList, 
                                        tANI_U32 *pNumItems );


/*                                                                            
                                            

        
                                                                              
                            

                                                  
                                                                    

                                                         

                                              
                             
  
                              

                      
                                                                             */
eHalStatus sme_RoamRegisterLinkQualityIndCallback(tHalHandle hHal, tANI_U8 sessionId,
                                                  csrRoamLinkQualityIndCallback   callback,  
                                                  void                           *pContext);


/*                                                                            
                       
                                                                         
                                                                 
                                                          
                                               
                           
                                                                             */
eHalStatus sme_RoamConnect(tHalHandle hHal, tANI_U8 sessionId, tCsrRoamProfile *pProfile, 
                           tANI_U32 *pRoamId);

/*                                                                            
                       
                                                                           
                                                                              
                                                                                
             
                                                                                
                                                                               
                           
                                               
                           
                                                                                 */
eHalStatus sme_RoamReassoc(tHalHandle hHal, tANI_U8 sessionId, tCsrRoamProfile *pProfile,
                          tCsrRoamModifyProfileFields modProfileFields,
                          tANI_U32 *pRoamId);

/*                                                                            
                                    
                                                                              
                           
                                                                   
                                                                             */
eHalStatus sme_RoamConnectToLastProfile(tHalHandle hHal, tANI_U8 sessionId);

/*                                                                            
                          
                                                                         
                                                                               
                                                                 
                           
                                                                             */
eHalStatus sme_RoamDisconnect(tHalHandle hHal, tANI_U8 sessionId, eCsrRoamDisconnectReason reason);

#ifdef WLAN_SOFTAP_FEATURE
/*                                                                            
                       
                                                        
                                             
                           
                                                                             */
eHalStatus sme_RoamStopBss(tHalHandle hHal, tANI_U8 sessionId);

/*                                                                            
                                 
                                                                                       
                                 
                                             
                                                                                        
                                                                                                        
                                            
                                                           
                                                                                           
                      
                                                                                 */
eHalStatus sme_RoamGetAssociatedStas(tHalHandle hHal, tANI_U8 sessionId,
                                        VOS_MODULE_ID modId, void *pUsrContext,
                                        void *pfnSapEventCallback, tANI_U8 *pAssocStasBuf);

/*                                                                            
                             
                                                                  
                                                                                        
                                                                                        
                                                                                 */
eHalStatus sme_RoamDisconnectSta(tHalHandle hHal, tANI_U8 sessionId, tANI_U8 *pPeerMacAddr);

/*                                                                            
                         
                                                                  
                                  
                                          
                                                                                        
                                                                                            
                                                                                 */
eHalStatus sme_RoamDeauthSta(tHalHandle hHal, tANI_U8 sessionId,
                                tANI_U8 *pPeerMacAddr);

/*                                                                            
                                   
                                                                               
                                          
                                                            
                                                                                            
                                                                                 */
eHalStatus sme_RoamTKIPCounterMeasures(tHalHandle hHal, tANI_U8 sessionId, tANI_BOOLEAN bEnable);

/*                                                                            
                                    
                                                          
                                 
                                             
                                            
                                                           
                      
                                                                                 */
eHalStatus sme_RoamGetWpsSessionOverlap(tHalHandle hHal, tANI_U8 sessionId,
                                        void *pUsrContext, void *pfnSapEventCallback,
                                        v_MACADDR_t pRemoveMac);
#endif

/*                                                                            
                               
                                                                                 
                     
                           
                                                                             */
eHalStatus sme_RoamGetConnectState(tHalHandle hHal, tANI_U8 sessionId, eCsrConnectState *pState);

/*                                                                            
                                 
                                                                           
                                                                                
                                                                      
                                                              
                                              
                                                     
                                                                             */
eHalStatus sme_RoamGetConnectProfile(tHalHandle hHal, tANI_U8 sessionId,
                                     tCsrRoamConnectedProfile *pProfile);

/*                                                                            
                                  
                                                                          
                                                                   
                                                              
                                              
                             
                                                                             */
eHalStatus sme_RoamFreeConnectProfile(tHalHandle hHal, 
                                      tCsrRoamConnectedProfile *pProfile);

/*                                                                            
                             
                                                                               
                                                                      
                                        
                                                                        
                                                                                 
                                                             
                                                                                 
                                                                    
                                         
                                                     
                                                  
                                                                             */
eHalStatus sme_RoamSetPMKIDCache( tHalHandle hHal, tANI_U8 sessionId, tPmkidCacheInfo *pPMKIDCache, 
                                  tANI_U32 numItems );

/*                                                                            
                                
                                                                                    
                                                            
                                
                                                   
                                                                                
                                                                           
                                                                             
                             
                                                                                     
                                                                                 
                                   
                                                                             */
eHalStatus sme_RoamGetSecurityReqIE(tHalHandle hHal, tANI_U8 sessionId, tANI_U32 *pLen,
                                  tANI_U8 *pBuf, eCsrSecurityType secType);

/*                                                                            
                                
                                                                                      
                                               
                                                   
                                                                                
                                                                           
                                                                             
                             
                                                                                                          
                                                                                 
                                   
                                                                             */
eHalStatus sme_RoamGetSecurityRspIE(tHalHandle hHal, tANI_U8 sessionId, tANI_U32 *pLen,
                                  tANI_U8 *pBuf, eCsrSecurityType secType);


/*                                                                            
                                
                                                                             
                  
                                                        
                                                                             */
tANI_U32 sme_RoamGetNumPMKIDCache(tHalHandle hHal, tANI_U8 sessionId);

/*                                                                            
                             
                                                                           
                                                                              
                                                                              
                                                               
                                                                               
                                         
                                                                                 
                                   
                                                                             */
eHalStatus sme_RoamGetPMKIDCache(tHalHandle hHal, tANI_U8 sessionId, tANI_U32 *pNum, 
                                 tPmkidCacheInfo *pPmkidCache);

/*                                                                            
                          
                                                                        
                                        
                                           
                           
                                                                             */
eHalStatus sme_GetConfigParam(tHalHandle hHal, tSmeConfigParams *pParam);

/*                                                                            
                         
                                                                              
                                             
    
                                                                                     
                                                                                          
                                                                        
                               

                              
                                   
                                   
                                   
                                   
                             

                                                                           
                                                                                 
                                     
                                                          
                                                                      
                                                                            
                           
                                                                             */
eHalStatus sme_GetStatistics(tHalHandle hHal, eCsrStatsRequesterType requesterId, 
                             tANI_U32 statsMask, 
                             tCsrStatsCallback callback, 
                             tANI_U32 periodicity, tANI_BOOLEAN cache, 
                             tANI_U8 staId, void *pContext);

eHalStatus sme_GetRssi(tHalHandle hHal, 
                             tCsrRssiCallback callback, 
                             tANI_U8 staId, tCsrBssid bssId, void *pContext, void* pVosContext);

/*                                                                            
                     
                                                                       
                                                              
                                                                           
                                                    
                                                                               
                                                                                
                                                                   
                                                               
                           
                                                                             */
eHalStatus sme_CfgSetInt(tHalHandle hHal, tANI_U32 cfgId, tANI_U32 ccmValue, 
                         tCcmCfgSetCallback callback, eAniBoolean toBeSaved) ;

/*                                                                            
                     
                                                                       
                                                              
                                                                                
                                    
                                                                    
                                                                               
                                                                                
                                                                   
                                                               
                           
                                                                             */
eHalStatus sme_CfgSetStr(tHalHandle hHal, tANI_U32 cfgId, tANI_U8 *pStr, 
                         tANI_U32 length, tCcmCfgSetCallback callback, 
                         eAniBoolean toBeSaved) ;


/*                                                                            
                                  
                                                                             
                                                               
                                                                                
                                                                           
                                                                             
                                                                             
            
                                                                          
                                    

                           
                                                                                 */
eHalStatus sme_GetModifyProfileFields(tHalHandle hHal, tANI_U8 sessionId, 
                                     tCsrRoamModifyProfileFields * pModifyProfileFields);


/*                                                                          
                              
                                                                              
                                                                              
                                                                              
                                                                             
                                          
                                                  
                                                     
                                                                          
                                                                                 
                         
                                                                            */
eHalStatus sme_SetConfigPowerSave(tHalHandle hHal, tPmcPowerSavingMode psMode,
                                  void *pConfigParams);

/*                                                                          
                              
                                                                               
                                                  
                                      
                                                                          
                                                                                 
                         
                                                                            */
eHalStatus sme_GetConfigPowerSave(tHalHandle hHal, tPmcPowerSavingMode psMode,
                                  void *pConfigParams);

/*                                                                            
                            
                                                                            
                                          
                                                  
                                               
                           
                                                                             */
extern eHalStatus sme_SignalPowerEvent (
   tHalHandle hHal,
   tPmcPowerEvent event);

/*                                                                            
                           
                                                                            
                                                                    
                                                  
                                                     
                           
                                                                             */
extern eHalStatus sme_EnablePowerSave (
   tHalHandle hHal,
   tPmcPowerSavingMode psMode);

/*                                                                            
                            
                                                                            
                                                                             
                                        
                                                  
                                                       
                           
                                                                             */
extern eHalStatus sme_DisablePowerSave (
   tHalHandle hHal,
   tPmcPowerSavingMode psMode);

/*                                                                            
                              
                                                                     
                                                                                  
                                                             
                                                  
                           
                                                                             */
extern eHalStatus sme_StartAutoBmpsTimer ( tHalHandle hHal);

/*                                                                            
                             
                                                                                   
                                                                                   
                                                                                    
                                                  
                           
                                                                             */
extern eHalStatus sme_StopAutoBmpsTimer ( tHalHandle hHal);

/*                                                                            
                           
                                                          
                                                  
                                                                  
                                                                              
                           
                                                                             */
extern eHalStatus sme_QueryPowerState (
   tHalHandle hHal,
   tPmcPowerState *pPowerState,
   tPmcSwitchState *pSwWlanSwitchState);

/*                                                                            
                              
                                                                              
                                                                          
                                                  
                                         
                           
                                                                             */
extern tANI_BOOLEAN sme_IsPowerSaveEnabled(
   tHalHandle hHal,
   tPmcPowerSavingMode psMode);

/*                                                                            
                            
                                                                   
                                                                          
                                                                                    
                                                                                        
                                                                                        
                                                                                        
                                                                                        
                                
                                                                                         
                                                                                         
                     
                                                                                     
                                               
                                       
                                      
                                               
                                               
                                                                                         
                                                           
                                                                                
                                                  
                                                  
                                                                                 
                                                                         
                                                                                  
                                                      
                                
                                                             
                                                                       
                                                                           
                                                                             */
extern eHalStatus sme_RequestFullPower (
   tHalHandle hHal,
   void (*callbackRoutine) (void *callbackContext, eHalStatus status),
   void *callbackContext,
   tRequestFullPowerReason fullPowerReason);

/*                                                                            
                       
                                                                          
                                                                              
                                                    
                                                  
                                                                                 
                                                                         
                      
                                                   
                                                                  
                                                                     
                                                                             */
extern eHalStatus sme_RequestBmps (
   tHalHandle hHal,
   void (*callbackRoutine) (void *callbackContext, eHalStatus status),
   void *callbackContext);

/*                                                                            
                                       
                                                                             
                        
                                                  
                                                                             */
void  sme_SetDHCPTillPowerActiveFlag(tHalHandle hHal, tANI_U8 flag);


/*                                                                            
                      
                                                                              
                                                                            
                 
                                                  
                                                                                 
                                                                         
                                                    
                                                                   
                                                                      
                                                                           
                           
                                                                             */
extern eHalStatus sme_StartUapsd (
   tHalHandle hHal,
   void (*callbackRoutine) (void *callbackContext, eHalStatus status),
   void *callbackContext);

/*                                                                            
                     
                                                                             
                                                                                  
                                             
                                                  
                        
                                                                             
                                                                       
                                                                             */
extern eHalStatus sme_StopUapsd (tHalHandle hHal);

/*                                                                            
                          
                                                                                 
                                                                               
                                                                               
                     
                                                  
                                                                                 
                                                                         
                        
                                                     
                                                                
                                                                   
                                                                             */
extern eHalStatus sme_RequestStandby (
   tHalHandle hHal,
   void (*callbackRoutine) (void *callbackContext, eHalStatus status),
   void *callbackContext);

/*                                                                            
                                  
                                                                       
                                                                     
                                                  
                                                                     
                                                                       
                      
                                                         
                                                               
                                                                             */
extern eHalStatus sme_RegisterPowerSaveCheck (
   tHalHandle hHal, 
   tANI_BOOLEAN (*checkRoutine) (void *checkContext), void *checkContext);

/*                                                                            
                                    
                                                 
                                                  
                                                                       
                      
                                                           
                                                                 
                                                                             */
extern eHalStatus sme_DeregisterPowerSaveCheck (
   tHalHandle hHal, 
   tANI_BOOLEAN (*checkRoutine) (void *checkContext));

/*                                                                            
                                        
                                                               
                                                                          
                                                  
                                                                
                                                                       
                      
                                                         
                                                               
                                                                             */
extern eHalStatus sme_RegisterDeviceStateUpdateInd (
   tHalHandle hHal, 
   void (*callbackRoutine) (void *callbackContext, tPmcState pmcState),
   void *callbackContext);

/*                                                                            
                                          
                                                                             
                                                  
                                                                  
                      
                                                           
                                                                 
                                                                             */
extern eHalStatus sme_DeregisterDeviceStateUpdateInd (
   tHalHandle hHal, 
   void (*callbackRoutine) (void *callbackContext, tPmcState pmcState));

/*                                                                            
                               
                                                                               
                                                                                 
                 
                                                  
                                          
                      
                                                   
                                                   
                                                                             */
extern eHalStatus sme_WowlAddBcastPattern (
   tHalHandle hHal, 
   tpSirWowlAddBcastPtrn pattern);

/*                                                                            
                               
                                                                      
                                                  
                                            
                      
                                                      
                                                   
                                                                             */
extern eHalStatus sme_WowlDelBcastPattern (
   tHalHandle hHal, 
   tpSirWowlDelBcastPtrn pattern);

/*                                                                            
                     
                                                             
                                                                                 
                                                                                
                                                                                 
                                
                                                                                     
                                                                                         
                                                                        
                                                                                             
                                                                                      
                                                                                    
                                                                                    
                                                                                        
                                      
                                                                                     
                                                                     
            
                                                  
                                                                         
                                                                           
                                                                                         
                                                      
                                                                
                                                                     
                                                                                       
                                                      
                      
                                                                 
                                                                 
                                                                                     
                                                      
                                                                             */
extern eHalStatus sme_EnterWowl (
    tHalHandle hHal,
    void (*enterWowlCallbackRoutine) (void *callbackContext, eHalStatus status),
    void *enterWowlCallbackContext,
#ifdef WLAN_WAKEUP_EVENTS
    void (*wakeReasonIndCB) (void *callbackContext, tpSirWakeReasonInd pWakeReasonInd),
    void *wakeReasonIndCBContext,
#endif //                   
    tpSirSmeWowlEnterParams wowlEnterParams);

/*                                                                            
                    
                                                                                 
                                                                                   
                                                                               
                                                  
                      
                                                                                
                                                                                   
                                                     
                                                                       
                                                                             */
extern eHalStatus sme_ExitWowl (tHalHandle hHal);

/*                                                                            

                      

                                                                                    
                                

                                                                                   

                                                                                                              

                                                                                       

                                                                           

                                                                                 */
eHalStatus sme_RoamSetKey(tHalHandle, tANI_U8 sessionId, tCsrRoamSetKey *pSetKey, tANI_U32 *pRoamId);

/*                                                                            

                         

                                                              

                                                                                 

                                                                                                              

                                                                                       

                                                                           

                                                                                 */
eHalStatus sme_RoamRemoveKey(tHalHandle, tANI_U8 sessionId, tCsrRoamRemoveKey *pRemoveKey, tANI_U32 *pRoamId);


/*                                                                            

                          

                                                                                                      
                            
                                                                                                         
                              

                                                                                 

                                                                            
                                                                                           
                                                                   
                                                                                     

                                

                                                                           

                                                                                 */
eHalStatus sme_GetCountryCode(tHalHandle hHal, tANI_U8 *pBuf, tANI_U8 *pbLen);

/*                                                                            

                          

                                                       
                                                       
                              

                                                                                

                                                                                                      
                                

                                

                                                                           

                                                                                 */
eHalStatus sme_SetCountryCode(tHalHandle hHal, tANI_U8 *pCountry, tANI_BOOLEAN *pfRestartNeeded);

/*                                                                            
                                       
                                                                                               
                                                                            
                                                                                                               
                                           
                           
                                                                                 */
eHalStatus sme_ResetCountryCodeInformation(tHalHandle hHal, tANI_BOOLEAN *pfRestartNeeded);

/*                                                                            
                                   
                                                                                     
                                                                                      
                                                                                      
                                               
                                                                                                    
                                                                                                
                           
                                                                                 */
eHalStatus sme_GetSupportedCountryCode(tHalHandle hHal, tANI_U8 *pBuf, tANI_U32 *pbLen);

/*                                                                            
                                      
                                                                                            
                                                                                                          
                                                         
                                                                          
                                

                                                                                
                                                                                 */
eHalStatus sme_GetCurrentRegulatoryDomain(tHalHandle hHal, v_REGDOMAIN_t *pDomain);

/*                                                                            
                               
                                                                 
                                                                                                          
                                   
                                                                                    
                                    
                                                                                                               
                                           
                           
                                                                                 */
eHalStatus sme_SetRegulatoryDomain(tHalHandle hHal, v_REGDOMAIN_t domainId, tANI_BOOLEAN *pfRestartNeeded);

/*                                                                            

                                         

                                                                                           

                                                                                  

                                                                                       
                                                                          

                                

                                                                           

                                                                                 */
eHalStatus sme_GetRegulatoryDomainForCountry(tHalHandle hHal, tANI_U8 *pCountry, v_REGDOMAIN_t *pDomainId);



/*                                                                            

                                         

                                                                                       

                                                                                           

                                                                                                
                                                                                                      
                                                                                                       
                                                                                           

                                

                                                                           

                                                                                 */
eHalStatus sme_GetSupportedRegulatoryDomains(tHalHandle hHal, v_REGDOMAIN_t *pDomains, tANI_U32 *pNumDomains);

//                      
tANI_BOOLEAN sme_Is11dSupported(tHalHandle hHal);
tANI_BOOLEAN sme_Is11hSupported(tHalHandle hHal);
tANI_BOOLEAN sme_IsWmmSupported(tHalHandle hHal); 
//                                                                                        
eHalStatus sme_ScanGetBaseChannels( tHalHandle hHal, tCsrChannelInfo * pChannelInfo );

typedef void ( *tSmeChangeCountryCallback)(void *pContext);
/*                                                                            

                             

                                                                            
                                     

                                                 

                                           

                                

                                                                           

                                                                                 */
eHalStatus sme_ChangeCountryCode( tHalHandle hHal,
                                  tSmeChangeCountryCallback callback,
                                  tANI_U8 *pCountry,
                                  void *pContext,
                                  void* pVosContext );


/*                                                                            
                            
                                                                               
                                                                              
                                                                              
                              
                                                  
                                                                                   
                                                                                    
                      
                                                                             
                                                                                    
                                                                  
                                                         
                                                                             */
VOS_STATUS sme_BtcSignalBtEvent (tHalHandle hHal, tpSmeBtEvent pBtcBtEvent);

/*                                                                            
                        
                                                                               
                                                                                  
                                                            
                                                  
                                                                        
                                                                                    
                                           
                      
                                                            
                                                     
                                                                             */
VOS_STATUS sme_BtcSetConfig (tHalHandle hHal, tpSmeBtcConfig pSmeBtcConfig);

/*                                                                            
                        
                                                                                 
                                                  
                                                                                       
                                                                                     
                      
                                          
                                       
                                                                             */
VOS_STATUS sme_BtcGetConfig (tHalHandle hHal, tpSmeBtcConfig pSmeBtcConfig);

/*                                                                            
                         
                                                   
                                                  
                                                
                                                                  
                            
                
                                                                             */
void sme_SetCfgPrivacy(tHalHandle hHal, tCsrRoamProfile *pProfile, tANI_BOOLEAN fPrivacy);

#if defined WLAN_FEATURE_VOWIFI
/*                                                                            
                                 
                                           
                                                  
                                                                          
                                                                                      
                                           
                      
                                          
                                       
                                                                             */
VOS_STATUS sme_NeighborReportRequest (tHalHandle hHal, tANI_U8 sessionId, 
                tpRrmNeighborReq pRrmNeighborReq, tpRrmNeighborRspCallbackInfo callbackInfo);
#endif

//                                                                         
//                                                                          
//                                                                   

//                   
//                                         
VOS_STATUS sme_DbgReadRegister(tHalHandle hHal, v_U32_t regAddr, v_U32_t *pRegValue);

//                    
//                                         
VOS_STATUS sme_DbgWriteRegister(tHalHandle hHal, v_U32_t regAddr, v_U32_t regValue);

//                 
//                                         
//                                                   
VOS_STATUS sme_DbgReadMemory(tHalHandle hHal, v_U32_t memAddr, v_U8_t *pBuf, v_U32_t nLen);

//                  
//                                         
VOS_STATUS sme_DbgWriteMemory(tHalHandle hHal, v_U32_t memAddr, v_U8_t *pBuf, v_U32_t nLen);

#ifdef FEATURE_WLAN_NON_INTEGRATED_SOC
//                
VOS_STATUS sme_GetFwVersion (tHalHandle hHal,FwVersionInfo *pVersion);
#endif
#ifdef FEATURE_WLAN_INTEGRATED_SOC
VOS_STATUS sme_GetWcnssWlanCompiledVersion(tHalHandle hHal,
                                           tSirVersionType *pVersion);
VOS_STATUS sme_GetWcnssWlanReportedVersion(tHalHandle hHal,
                                           tSirVersionType *pVersion);
VOS_STATUS sme_GetWcnssSoftwareVersion(tHalHandle hHal,
                                       tANI_U8 *pVersion,
                                       tANI_U32 versionBufferSize);
VOS_STATUS sme_GetWcnssHardwareVersion(tHalHandle hHal,
                                       tANI_U8 *pVersion,
                                       tANI_U32 versionBufferSize);
#endif
eHalStatus sme_RoamRegisterCallback(tHalHandle hHal,
                                    csrRoamCompleteCallback callback,
                                    void *pContext);

#ifdef FEATURE_WLAN_WAPI
/*                                                                            
                            
                                                                          
                   
                                                                                 
                                      
                                                                                   
                                                                                 
                                                                   
                                                                                 
                                                              
                                                                             */
eHalStatus sme_RoamSetBKIDCache( tHalHandle hHal, tANI_U32 sessionId, tBkidCacheInfo *pBKIDCache,
                                 tANI_U32 numItems );

/*                                                                            
                            
                                                                                
               
                                                                                 
                                      
                                                                              
                                                                                   
                 
                                                                                  
               
                                                                                 
               
                                                                             */
eHalStatus sme_RoamGetBKIDCache(tHalHandle hHal, tANI_U32 *pNum,
                                tBkidCacheInfo *pBkidCache);

/*                                                                            
                               
                                                                                
                                 
                                                                                 
                                      
                                                        
                                                                             */
tANI_U32 sme_RoamGetNumBKIDCache(tHalHandle hHal, tANI_U32 sessionId);

/*                                                                            
                                    
                                                               
                                                                    
                                          
                                                                    
                                                                            
                                                                            
                                 
                                                                                 
                                                 
                                         
                                                        
                                                     
                                                                             */
eHalStatus sme_ScanGetBKIDCandidateList(tHalHandle hHal, tANI_U32 sessionId,
                                        tBkidCandidateInfo *pBkidList, 
                                        tANI_U32 *pNumItems );
#endif /*                   */

#ifdef FEATURE_OEM_DATA_SUPPORT
/*                                                                                           
                                
                                                                                            */
/*                                                                            
                      
                                                                         
                                                                          
                                                                    
                                                          
                           
                                                                             */
eHalStatus sme_OemDataReq(tHalHandle hHal, 
                                       tANI_U8 sessionId,
                                       tOemDataReqConfig *, 
                                       tANI_U32 *pOemDataReqID, 
                                       oemData_OemDataReqCompleteCallback callback, 
                                       void *pContext);

/*                                                                            
                         
                                                         
                                                                          
                           
                                                                             */
eHalStatus sme_getOemDataRsp(tHalHandle hHal, 
                                         tOemDataRsp **pOemDataRsp);

#endif /*                        */


#ifdef WLAN_SOFTAP_FEATURE

/*                                                                            

                             

                                                                                                
                                

                                                                               

                                                                                           

                                                                             

                                                                                 */

eHalStatus sme_RoamUpdateAPWPSIE(tHalHandle, tANI_U8 sessionId, tSirAPWPSIEs *pAPWPSIES);
/*                                                                            

                                 

                                                                                                     
                                

                                                                                           

                                     

                                                                             

                                                                                 */
eHalStatus sme_RoamUpdateAPWPARSNIEs(tHalHandle hHal, tANI_U8 sessionId, tSirRSNie * pAPSirRSNie);

#endif

/*                                                                            
                        
                                         
                                                
                                   
                                  

                                                                             

                                                                            */

eHalStatus sme_sendBTAmpEvent(tHalHandle hHal, tSmeBtAmpEvent btAmpEvent);



/*                                                                            
                          
                                                
                                                  
                                                       
                      
                                                                              */
eHalStatus sme_SetHostOffload (tHalHandle hHal, tANI_U8 sessionId,
                                    tpSirHostOffloadReq pRequest);

/*                                                                            
                        
                                              
                                                  
                                                          
                      
                                                                             */
eHalStatus sme_SetKeepAlive (tHalHandle hHal, tANI_U8 sessionId,
                                  tpSirKeepAliveReq pRequest);


/*                                                                            
                        
                                   
                                                  
                      
                                          
                                       
                                                                             */
eHalStatus sme_AbortMacScan(tHalHandle hHal);

/*                                                                             
                              
                                                           
                                                                                 
                                                         
                               
                               
                                                                               */
eHalStatus sme_GetOperationChannel(tHalHandle hHal, tANI_U32 *pChannel, tANI_U8 sessionId);

#ifdef WLAN_FEATURE_P2P
/*                                                                            

                            

                                                                      
                                                                        
                                                                           
                               
                                                 
                       
                                                                                 */
eHalStatus sme_RegisterMgmtFrame(tHalHandle hHal, tANI_U8 sessionId, 
                     tANI_U16 frameType, tANI_U8* matchData, tANI_U16 matchLen);

/*                                                                            

                              

                                                                         
                                                                        
                                                                           
                               
                                                 
                       
                                                                                 */
eHalStatus sme_DeregisterMgmtFrame(tHalHandle hHal, tANI_U8 sessionId, 
                     tANI_U16 frameType, tANI_U8* matchData, tANI_U16 matchLen);
#endif

/*                                                                            

                               

         
                                                                              
                                  

         

                                           
 
                                                                          
                                                                               
                                                                              
                               
                         
                         
                                   

   
                        
  
  
                                                                            */
eHalStatus sme_ConfigureRxpFilter( tHalHandle hHal, 
                              tpSirWlanSetRxpFilters  wlanRxpFilterParam);

/*                                                                            

                                        

         
                                                                             
                                                                     
                                     

         

                                           
 
                                                  

   
                        
  
  
                                                                            */
eHalStatus sme_ConfigureAppsCpuWakeupState( tHalHandle hHal, tANI_BOOLEAN  isAppsAwake);

#ifdef FEATURE_WLAN_INTEGRATED_SOC
/*                                                                            

                                

         
                                                                               
                

         

                                           
 
                                                     

   
                        
  
  
                                                                            */
eHalStatus sme_ConfigureSuspendInd( tHalHandle hHal, 
                             tpSirWlanSuspendParam  wlanSuspendParam);

/*                                                                            

                               

         
                                                                               
              

         

                                           
 
                                                   

   
                        
  
  
                                                                            */
eHalStatus sme_ConfigureResumeReq( tHalHandle hHal, 
                             tpSirWlanResumeParam  wlanResumeParam);

#endif

/*                                                                            

                             

                                                                
                              

                                                 

                                                           

                                                                                 */
tANI_S8 sme_GetInfraSessionId(tHalHandle hHal);

/*                                                                            

                                    

                                                                       
                              

                                                 
                                                                 

                                                                  

                                                                                 */
tANI_U8 sme_GetInfraOperationChannel( tHalHandle hHal, tANI_U8 sessionId);
/*                                                                            

                                         

                                                                                   
                              

                                                 
                                                              

                                                                  

                                                                                 */
tANI_U8 sme_GetConcurrentOperationChannel( tHalHandle hHal );

/*                                                                            
                        
                                   
                                                  
                      
                                          
                                       
                                                                             */
eHalStatus sme_AbortMacScan(tHalHandle hHal);

/*                                                                            
                               
                                         
                                                  
                                                               
                                            
                      
                                                                             */
eHalStatus sme_GetCfgValidChannels(tHalHandle hHal, tANI_U8 *aValidChannels, tANI_U32 *len);

#ifdef FEATURE_WLAN_SCAN_PNO

/*                                                                            
                                   
                                                                  
                                                  
                                                       
                      
                                                                             */
eHalStatus sme_SetPreferredNetworkList (tHalHandle hHal, tpSirPNOScanReq pRequest, tANI_U8 sessionId, preferredNetworkFoundIndCallback callbackRoutine, void *callbackContext );

/*                                                                            
                         
                                           
                                                  
                                                       
                      
                                                                             */
eHalStatus sme_SetRSSIFilter(tHalHandle hHal, v_U8_t rssiThreshold);

/*                                                                             
 
                                    
 
              
                                               
 
             
                                 
                                     
  
                     
 
                                                                             */
eHalStatus sme_PreferredNetworkFoundInd (tHalHandle hHal, void* pMsg);
#endif //                      

/*                                                                            
                          
                                        
                                                  
                                                                  
                      
                                                                             */
eHalStatus sme_SetPowerParams(tHalHandle hHal, tSirSetPowerParamsReq* pwParams);

/*                                                                            
                            
                                                        
                                                  
                                                                 
                           
                                                                             */
eHalStatus sme_SetTxPerTracking (
   tHalHandle hHal,
   void (*pCallbackfn) (void *pCallbackContext),
   void *pCallbackContext,
   tpSirTxPerTrackingParam pTxPerTrackingParam);

#ifdef WLAN_FEATURE_PACKET_FILTERING
/*                                                                            
                                  
                                                  
                                                  
                                                                   
                         
                                                                             */
eHalStatus sme_8023MulticastList(tHalHandle hHal, tpSirRcvFltMcAddrList pMulticastAddrs);

/*                                                                            
                                  
                                            
                                                  
                                                              
                         
                                                                             */
eHalStatus sme_ReceiveFilterSetFilter(tHalHandle hHal, tpSirRcvPktFilterCfgType pRcvPktFilterCfg,
                                           tANI_U8 sessionId);

/*                                                                            
                               
                                               
                                                  
                                                                                                      
                                                                       
                         
                                                                             */
eHalStatus sme_GetFilterMatchCount(tHalHandle hHal, 
                                   FilterMatchCountCallback callbackRoutine, 
                                   void *callbackContext );

/*                                                                            
                                    
                                              
                                                  
                                                                        
                         
                                                                             */
eHalStatus sme_ReceiveFilterClearFilter(tHalHandle hHal,
                                        tpSirRcvFltPktClearParam pRcvFltPktClearParam,
                                        tANI_U8  sessionId);
#endif //                              
/*                                                                            

                          
                                                                            
                              

                                                 
                                      

                                                

                                                                                 */
tANI_BOOLEAN sme_IsChannelValid(tHalHandle hHal, tANI_U8 channel);

/*                                                                            
                       
                                       
                
                                       
                        
                                                                             */
eHalStatus sme_SetFreqBand(tHalHandle hHal, eCsrBand eBand);

/*                                                                            
                       
                                                  
                
                                                  
                        
                                                                             */
eHalStatus sme_GetFreqBand(tHalHandle hHal, eCsrBand *pBand);

/*                                                                            

                            
                                                        
                                                  
                                                                 
                           
                                                                             */
eHalStatus sme_SetTxPerTracking (
   tHalHandle hHal,
   void (*pCallbackfn) (void *pCallbackContext),
   void *pCallbackContext,
   tpSirTxPerTrackingParam pTxPerTrackingParam);

#ifdef WLAN_FEATURE_GTK_OFFLOAD
/*                                                                            
                         
                                           
                                                  
                                                           
                      
                                                                             */
eHalStatus sme_SetGTKOffload (tHalHandle hHal, tpSirGtkOffloadParams pRequest);

/*                                                                            
                         
                                               
                                                  
                                                            
                      
                                                                             */
eHalStatus sme_GetGTKOffload (tHalHandle hHal, GTKOffloadGetInfoCallback callbackRoutine, void *callbackContext );
#endif //                         

#ifdef WLAN_WAKEUP_EVENTS
eHalStatus sme_WakeReasonIndCallback (tHalHandle hHal, void* pMsg);
#endif //                   

/*                                                                            
                            
                                                        
                                                  
                                                                 
                           
                                                                             */
eHalStatus sme_SetTxPerTracking (
   tHalHandle hHal,
   void (*pCallbackfn) (void *pCallbackContext),
   void *pCallbackContext,
   tpSirTxPerTrackingParam pTxPerTrackingParam);


//                                         
tANI_U16 sme_ChnToFreq(tANI_U8 chanNum);

tANI_BOOLEAN sme_IsChannelValid(tHalHandle hHal, tANI_U8 channel);

#if defined WLAN_FEATURE_P2P_INTERNAL

eHalStatus sme_p2pResetSession(tHalHandle hHal, tANI_U8 HDDSessionId);

/*                                                                            
                              
                                                      
                                                  
                                                             
                           
                                                                             */
eHalStatus sme_p2pFlushDeviceList(tHalHandle hHal, tANI_U8 HDDSessionId);

eHalStatus sme_p2pGetResultFilter(tHalHandle hHal, tANI_U8 HDDSessionId,
                              tCsrScanResultFilter *pFilter);

#endif //                                     
   
/*                                                                            
                         
                                                                          
                                         
                
                                                 
                                                   
                                     
                        
                                                                             */
eHalStatus sme_SetMaxTxPower(tHalHandle hHal, tSirMacAddr pBssid, 
                             tSirMacAddr pSelfMacAddress, v_S7_t dB);

#ifdef WLAN_SOFTAP_FEATURE
/*                                                                            

                    

                                                                           
                             

               
                     
                                                                    
                        

                                                                                 */
eHalStatus sme_HideSSID(tHalHandle hHal, v_U8_t sessionId, v_U8_t ssidHidden);
#endif

/*                                                                            

                      
                                                
                                                  
                                                     
                                                              
                           
                                                                             */
eHalStatus sme_SetTmLevel(tHalHandle hHal, v_U16_t newTMLevel, v_U16_t tmMode);

/*                                                                           

                                                                                   
              

                                      

              

                                                                           */
void sme_featureCapsExchange(tHalHandle hHal);

/*                                                                           

                                                                             
                     
              

                                      
                                            

                           

                                                                             */
eHalStatus sme_GetDefaultCountryCodeFrmNv(tHalHandle hHal, tANI_U8 *pCountry);

/*                                                                           

                                                                                 
                       

                                      
                                            

                            

                                                                             */
eHalStatus sme_GetCurrentCountryCode(tHalHandle hHal, tANI_U8 *pCountry);

/*                                                                            
                          
                                                     
                                                                               
                                                                     
                                                         
                                                                               
                                                                   
                  
                                                                             */
void sme_transportDebug
(
   v_BOOL_t  displaySnapshot,
   v_BOOL_t  toggleStallDetect
);
#endif //                           
