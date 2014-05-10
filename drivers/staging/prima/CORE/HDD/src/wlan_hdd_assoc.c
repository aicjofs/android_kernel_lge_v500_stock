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

/**========================================================================

  \file  wlan_hdd_assoc.c
  \brief WLAN Host Device Driver implementation
               
   Copyright 2008 (c) Qualcomm, Incorporated.  All Rights Reserved.
   
   Qualcomm Confidential and Proprietary.
  
  ========================================================================*/
/*                                                                           
                                             
   
   
                                                                        
                                                                 
   
   
                                      
   
   
                                      
                                                                             
                                              
                                                                            */
  
#include "wlan_hdd_includes.h"
#include <aniGlobal.h>
#include "dot11f.h"
#include "wlan_nlink_common.h"
#include "wlan_btc_svc.h"
#include "wlan_hdd_power.h"
#ifdef CONFIG_CFG80211
#include <linux/ieee80211.h>
#include <linux/wireless.h>
#include <net/cfg80211.h>
#include "wlan_hdd_cfg80211.h"
#include "csrInsideApi.h"
#endif
#if defined CONFIG_CFG80211
#include "wlan_hdd_p2p.h"
#endif
#include "sme_Api.h"

v_BOOL_t mibIsDot11DesiredBssTypeInfrastructure( hdd_adapter_t *pAdapter );

struct ether_addr 
{
    u_char  ether_addr_octet[6];
};
//                                                      
#define HDD_WPA_OUI_SIZE 4
v_U8_t ccpWpaOui00[ HDD_WPA_OUI_SIZE ] = { 0x00, 0x50, 0xf2, 0x00 };
v_U8_t ccpWpaOui01[ HDD_WPA_OUI_SIZE ] = { 0x00, 0x50, 0xf2, 0x01 };
v_U8_t ccpWpaOui02[ HDD_WPA_OUI_SIZE ] = { 0x00, 0x50, 0xf2, 0x02 };
v_U8_t ccpWpaOui03[ HDD_WPA_OUI_SIZE ] = { 0x00, 0x50, 0xf2, 0x03 };
v_U8_t ccpWpaOui04[ HDD_WPA_OUI_SIZE ] = { 0x00, 0x50, 0xf2, 0x04 };
v_U8_t ccpWpaOui05[ HDD_WPA_OUI_SIZE ] = { 0x00, 0x50, 0xf2, 0x05 };
#ifdef FEATURE_WLAN_CCX
v_U8_t ccpWpaOui06[ HDD_WPA_OUI_SIZE ] = { 0x00, 0x40, 0x96, 0x00 }; //     
#endif /*                  */
#define HDD_RSN_OUI_SIZE 4
v_U8_t ccpRSNOui00[ HDD_RSN_OUI_SIZE ] = { 0x00, 0x0F, 0xAC, 0x00 }; //             
v_U8_t ccpRSNOui01[ HDD_RSN_OUI_SIZE ] = { 0x00, 0x0F, 0xAC, 0x01 }; //              
v_U8_t ccpRSNOui02[ HDD_RSN_OUI_SIZE ] = { 0x00, 0x0F, 0xAC, 0x02 }; //                
v_U8_t ccpRSNOui03[ HDD_RSN_OUI_SIZE ] = { 0x00, 0x0F, 0xAC, 0x03 }; //         
v_U8_t ccpRSNOui04[ HDD_RSN_OUI_SIZE ] = { 0x00, 0x0F, 0xAC, 0x04 }; //         
v_U8_t ccpRSNOui05[ HDD_RSN_OUI_SIZE ] = { 0x00, 0x0F, 0xAC, 0x05 }; //        
#ifdef FEATURE_WLAN_CCX
v_U8_t ccpRSNOui06[ HDD_RSN_OUI_SIZE ] = { 0x00, 0x40, 0x96, 0x00 }; //     
#endif /*                  */

#if defined(WLAN_FEATURE_VOWIFI_11R) 
//                                    
#define FT_ASSOC_RSP_IES_OFFSET 6
#endif

#define BEACON_FRAME_IES_OFFSET 12

#ifdef WLAN_FEATURE_PACKET_FILTERING
extern void wlan_hdd_set_mc_addr_list(hdd_context_t *pHddCtx, v_U8_t set, v_U8_t sessionId);
#endif

void hdd_ResetCountryCodeAfterDisAssoc(hdd_adapter_t *pAdapter);

static inline v_VOID_t hdd_connSetConnectionState( hdd_station_ctx_t *pHddStaCtx, eConnectionState connState )
{         
   //                               
   pHddStaCtx->conn_info.connState = connState;
}

//                                
//                                                                                   
//                                                                                           

static inline v_BOOL_t hdd_connGetConnectionState( hdd_station_ctx_t *pHddStaCtx, 
                                    eConnectionState *pConnState ) 
{
   v_BOOL_t fConnected; 
   eConnectionState connState;
    
   //                          
   connState = pHddStaCtx->conn_info.connState;
   //                                                                   
   if ( eConnectionState_Associated == connState ||
        eConnectionState_IbssConnected == connState )
   {
      fConnected = VOS_TRUE;
   }
   else 
   {
      fConnected = VOS_FALSE;
   }
    
   if ( pConnState )
   {
      *pConnState = connState;
   }
  
   return( fConnected );
}

v_BOOL_t hdd_connIsConnected( hdd_station_ctx_t *pHddStaCtx )
{
   return( hdd_connGetConnectionState( pHddStaCtx, NULL ) );
}  

//                                         
#if 0
//
v_BOOL_t hdd_connIsConnectedInfra( hdd_adapter_t *pAdapter )
{
   v_BOOL_t fConnectedInfra = FALSE;
   eConnectionState connState;
   
   if ( hdd_connGetConnectionState( WLAN_HDD_GET_STATION_CTX_PTR(pAdapter), &connState ) )
   {   
      if ( eConnectionState_Associated == connState ) 
      {
         fConnectedInfra = TRUE;
      }   
   }
   
   return( fConnectedInfra );
}
#endif
    
static inline v_BOOL_t hdd_connGetConnectedCipherAlgo( hdd_station_ctx_t *pHddStaCtx, eCsrEncryptionType *pConnectedCipherAlgo )
{
    v_BOOL_t fConnected = VOS_FALSE;
    
    fConnected = hdd_connGetConnectionState( pHddStaCtx, NULL );
  
    if ( pConnectedCipherAlgo ) 
    {
        *pConnectedCipherAlgo = pHddStaCtx->conn_info.ucEncryptionType;
    }
    
    return( fConnected );
}
 
inline v_BOOL_t hdd_connGetConnectedBssType( hdd_station_ctx_t *pHddStaCtx, eMib_dot11DesiredBssType *pConnectedBssType )
{
    v_BOOL_t fConnected = VOS_FALSE;
    
    fConnected = hdd_connGetConnectionState( pHddStaCtx, NULL );
  
    if ( pConnectedBssType ) 
    {
        *pConnectedBssType = pHddStaCtx->conn_info.connDot11DesiredBssType;
    }
    
    return( fConnected );
}

static inline void hdd_connSaveConnectedBssType( hdd_station_ctx_t *pHddStaCtx, eCsrRoamBssType csrRoamBssType )
{
   switch( csrRoamBssType ) 
   {
      case eCSR_BSS_TYPE_INFRASTRUCTURE:
          pHddStaCtx->conn_info.connDot11DesiredBssType = eMib_dot11DesiredBssType_infrastructure;
         break;
                     
      case eCSR_BSS_TYPE_IBSS:
      case eCSR_BSS_TYPE_START_IBSS:
          pHddStaCtx->conn_info.connDot11DesiredBssType = eMib_dot11DesiredBssType_independent;
         break;
           
      /*                                                                      
                                                     */
      case eCSR_BSS_TYPE_ANY:                      
      default:
         VOS_ASSERT( 0 );
         break;      
   }                     
    
}

void hdd_connSaveConnectInfo( hdd_adapter_t *pAdapter, tCsrRoamInfo *pRoamInfo, eCsrRoamBssType eBssType )
{
   hdd_station_ctx_t *pHddStaCtx = WLAN_HDD_GET_STATION_CTX_PTR(pAdapter);
   eCsrEncryptionType encryptType = eCSR_ENCRYPT_TYPE_NONE;
 
   VOS_ASSERT( pRoamInfo );
   
   if ( pRoamInfo )   
   {
      //                                       
      if ( eCSR_BSS_TYPE_INFRASTRUCTURE == eBssType )
      {
          VOS_ASSERT( pRoamInfo->pBssDesc );
          vos_mem_copy(pHddStaCtx->conn_info.bssId, pRoamInfo->bssid,6 );

          //                                                           
          //                                                
          //                                                                             
          //                  
          if( !pRoamInfo->fReassocReq )
          {
              pHddStaCtx->conn_info.staId [0]= pRoamInfo->staId;
          }
      }
      else if ( eCSR_BSS_TYPE_IBSS == eBssType )
      {   
         vos_mem_copy(pHddStaCtx->conn_info.bssId, pRoamInfo->bssid,sizeof(pRoamInfo->bssid) );
      }   
      else
      {
         //                                                                           
         //                      
         VOS_ASSERT( 0 );
      }

      //           
      hdd_wmm_connect(pAdapter, pRoamInfo, eBssType);

      if( !pRoamInfo->u.pConnectedProfile )
      {
         VOS_ASSERT( pRoamInfo->u.pConnectedProfile );
      }
      else
      {
          //                              
          encryptType =  pRoamInfo->u.pConnectedProfile->mcEncryptionType;
          pHddStaCtx->conn_info.mcEncryptionType = encryptType;
          //                           
          encryptType =  pRoamInfo->u.pConnectedProfile->EncryptionType;
          pHddStaCtx->conn_info.ucEncryptionType = encryptType;

          pHddStaCtx->conn_info.authType =  pRoamInfo->u.pConnectedProfile->AuthType;

          pHddStaCtx->conn_info.operationChannel = pRoamInfo->u.pConnectedProfile->operationChannel;

          //                                 
          vos_mem_copy( &pHddStaCtx->conn_info.SSID.SSID, &pRoamInfo->u.pConnectedProfile->SSID, sizeof( tSirMacSSid ) );
      }
   }   
      
   //                           
   hdd_connSaveConnectedBssType( pHddStaCtx, eBssType );  
   
}

#if defined(WLAN_FEATURE_VOWIFI_11R)
/*
                                                  
                                                    
                                                         
                         
  
                                                         
                                                 
                                                  
                              
 */
static void hdd_SendFTAssocResponse(struct net_device *dev, hdd_adapter_t *pAdapter, 
                tCsrRoamInfo *pCsrRoamInfo)
{
    union iwreq_data wrqu;
    char *buff;
    unsigned int len = 0;
    u8 *pFTAssocRsp = NULL;
    
    if (pCsrRoamInfo->nAssocRspLength == 0) 
    {
        hddLog(LOGE,
            "%s: pCsrRoamInfo->nAssocRspLength=%d",
            __func__, (int)pCsrRoamInfo->nAssocRspLength);
        return;
    }

    pFTAssocRsp = (u8 *)(pCsrRoamInfo->pbFrames + pCsrRoamInfo->nBeaconLength + 
        pCsrRoamInfo->nAssocReqLength);
    if (pFTAssocRsp == NULL) 
    {
        hddLog(LOGE, "%s: AssocReq or AssocRsp is NULL", __func__); 
        return;
    }

    //                                      
    pFTAssocRsp += FT_ASSOC_RSP_IES_OFFSET;
    hddLog(LOG1, "%s: AssocRsp is now at %02x%02x", __func__,
        (unsigned int)pFTAssocRsp[0],
        (unsigned int)pFTAssocRsp[1]);

    //                                           
    buff = kmalloc(IW_GENERIC_IE_MAX, GFP_ATOMIC);
    if (buff == NULL) 
    {
        hddLog(LOGE, "%s: kmalloc unable to allocate memory", __func__); 
        return;
    }

    //                                                                 
    len = pCsrRoamInfo->nAssocRspLength - FT_ASSOC_RSP_IES_OFFSET;
    wrqu.data.length = len; 
    memset(buff, 0, IW_GENERIC_IE_MAX);
    memcpy(buff, pFTAssocRsp, len); 
    wireless_send_event(dev, IWEVASSOCRESPIE, &wrqu, buff);

    kfree(buff);
}
#endif /*                         */ 

#ifdef WLAN_FEATURE_VOWIFI_11R

/*                                                   
  
                                                        
                                               
  
                                                     
                                              
                                                  
                                        
  
                                                     
 */
void hdd_SendFTEvent(hdd_adapter_t *pAdapter)
{
    union iwreq_data wrqu;
    //                          
    char *buff;
    tANI_U16 auth_resp_len = 0;
    tANI_U32 ric_ies_length = 0;
    tANI_U16 str_len;
    hdd_context_t *pHddCtx = WLAN_HDD_GET_CTX(pAdapter);

    //                                           
    buff = kmalloc(IW_CUSTOM_MAX, GFP_ATOMIC);
    if (buff == NULL) 
    {
        hddLog(LOGE, "%s: kmalloc unable to allocate memory", __func__); 
        return;
    }
    vos_mem_zero(buff, IW_CUSTOM_MAX); 

    //                                     
    str_len = strlcpy(buff, "RIC=", IW_CUSTOM_MAX);
    sme_GetRICIEs( pHddCtx->hHal, (u8 *)&(buff[str_len]), 
                   (IW_CUSTOM_MAX - str_len), &ric_ies_length ); 
    if (ric_ies_length == 0) 
    {
        hddLog(LOGW, "%s: RIC IEs is of length 0 not sending RIC Information for now", __func__); 
    }
    else
    {
        wrqu.data.length = str_len + ric_ies_length;
        wireless_send_event(pAdapter->dev, IWEVCUSTOM, &wrqu, buff);
    }

    //                                   
    vos_mem_zero(buff, IW_CUSTOM_MAX); 
    str_len = strlcpy(buff, "AUTH=", IW_CUSTOM_MAX);
    sme_GetFTPreAuthResponse(pHddCtx->hHal, (u8 *)&buff[str_len], 
                             (IW_CUSTOM_MAX - str_len),  &auth_resp_len);

    if (auth_resp_len == 0) 
    {
        hddLog(LOGE, "%s: AuthRsp FTIES is of length 0", __func__); 
        return;
    }

    wrqu.data.length = str_len + auth_resp_len;
    wireless_send_event(pAdapter->dev, IWEVCUSTOM, &wrqu, buff);

    kfree(buff);
}

#endif /*                         */

#ifdef FEATURE_WLAN_CCX

/*
                                                                 
                                                                   
  
                                                         
 */
static void hdd_SendNewAPChannelInfo(struct net_device *dev, hdd_adapter_t *pAdapter, 
                tCsrRoamInfo *pCsrRoamInfo)
{
    union iwreq_data wrqu;
    tSirBssDescription *descriptor = pCsrRoamInfo->pBssDesc;
     

    if (descriptor == NULL) 
    {
        hddLog(LOGE,
            "%s: pCsrRoamInfo->pBssDesc=%p\n",
            __func__, descriptor);
        return;
    }

    //                                                                                      
    hddLog(LOGW, "%s: Sending up an SIOCGIWFREQ, channelId=%d\n", __func__, descriptor->channelId);
    memset(&wrqu, '\0', sizeof(wrqu));
    wrqu.freq.m = descriptor->channelId;
    wrqu.freq.e = 0;
    wrqu.freq.i = 0;
    wireless_send_event(pAdapter->dev, SIOCGIWFREQ, &wrqu, NULL);
}

#endif /*                  */

void hdd_SendUpdateBeaconIEsEvent(hdd_adapter_t *pAdapter, tCsrRoamInfo *pCsrRoamInfo)
{
    union iwreq_data wrqu;
    u8  *pBeaconIes;
    u8 currentLen = 0;
    char *buff;
    int totalIeLen = 0, currentOffset = 0, strLen;

    memset(&wrqu, '\0', sizeof(wrqu));

    if (0 == pCsrRoamInfo->nBeaconLength)
    {
        hddLog(LOGE, "%s: pCsrRoamInfo->nBeaconFrameLength = 0", __func__);
        return;
    }
    pBeaconIes = (u8 *)(pCsrRoamInfo->pbFrames + BEACON_FRAME_IES_OFFSET);
    if (pBeaconIes == NULL) 
    {
        hddLog(LOGE, "%s: Beacon IEs is NULL", __func__); 
        return;
    }

    //                                     
    hddLog(LOG1, "%s: Beacon IEs is now at %02x%02x", __func__,
        (unsigned int)pBeaconIes[0],
        (unsigned int)pBeaconIes[1]);
    hddLog(LOG1, "%s: Beacon IEs length = %d", __func__, pCsrRoamInfo->nBeaconLength - BEACON_FRAME_IES_OFFSET);
    
   //                                           
    buff = kmalloc(IW_CUSTOM_MAX, GFP_ATOMIC);
    if (buff == NULL) 
    {
        hddLog(LOGE, "%s: kmalloc unable to allocate memory", __func__); 
        return;
    }
    vos_mem_zero(buff, IW_CUSTOM_MAX); 

    strLen = strlcpy(buff,"BEACONIEs=", IW_CUSTOM_MAX);
    currentLen = strLen + 1;

    totalIeLen = pCsrRoamInfo->nBeaconLength - BEACON_FRAME_IES_OFFSET;
    do
    {
        /*                                                                                        
                                                                                             */
        vos_mem_zero(&buff[strLen + 1], IW_CUSTOM_MAX - (strLen + 1));
        currentLen = VOS_MIN(totalIeLen, IW_CUSTOM_MAX - (strLen + 1) - 1);
        vos_mem_copy(&buff[strLen + 1], pBeaconIes+currentOffset, currentLen);
        currentOffset += currentLen;
        totalIeLen -= currentLen;
        wrqu.data.length = strLen + 1 + currentLen;
        if (totalIeLen)
          buff[strLen] = 1;   //                                               
        else
          buff[strLen] = 0;   //                                          

        hddLog(LOG1, "%s: Beacon IEs length to supplicant = %d", __func__, currentLen);
        wireless_send_event(pAdapter->dev, IWEVCUSTOM, &wrqu, buff);
    } while (totalIeLen > 0);

    kfree(buff);
}

static void hdd_SendAssociationEvent(struct net_device *dev,tCsrRoamInfo *pCsrRoamInfo)
{
    hdd_adapter_t *pAdapter = WLAN_HDD_GET_PRIV_PTR(dev);
    hdd_context_t *pHddCtx = WLAN_HDD_GET_CTX(pAdapter);
    hdd_station_ctx_t *pHddStaCtx = WLAN_HDD_GET_STATION_CTX_PTR(pAdapter);
    union iwreq_data wrqu;
    int we_event;
    char *msg;
    int type = -1;

#if defined (WLAN_FEATURE_VOWIFI_11R) 
    //                                                    
    //                                              
    hdd_wext_state_t  *pWextState =  WLAN_HDD_GET_WEXT_STATE_PTR(pAdapter);   
    tCsrRoamProfile* pRoamProfile = &(pWextState->roamProfile);
#endif
 
    memset(&wrqu, '\0', sizeof(wrqu));
    wrqu.ap_addr.sa_family = ARPHRD_ETHER; 
    we_event = SIOCGIWAP;
   
    if(eConnectionState_Associated == pHddStaCtx->conn_info.connState)/*            */
    {
        memcpy(wrqu.ap_addr.sa_data, pCsrRoamInfo->pBssDesc->bssId, sizeof(pCsrRoamInfo->pBssDesc->bssId));
        type = WLAN_STA_ASSOC_DONE_IND;

        pr_info("wlan: connected to %02x:%02x:%02x:%02x:%02x:%02x\n",
                      wrqu.ap_addr.sa_data[0],
                      wrqu.ap_addr.sa_data[1],
                      wrqu.ap_addr.sa_data[2],
                      wrqu.ap_addr.sa_data[3],
                      wrqu.ap_addr.sa_data[4],
                      wrqu.ap_addr.sa_data[5]);
        hdd_SendUpdateBeaconIEsEvent(pAdapter, pCsrRoamInfo);

        /*                                                                     
                                                                           
                                 */
#ifdef WLAN_FEATURE_VOWIFI_11R
        //                                                  
        if ((pRoamProfile->AuthType.authType[0] == eCSR_AUTH_TYPE_FT_RSN_PSK) ||
            (pRoamProfile->AuthType.authType[0] == eCSR_AUTH_TYPE_FT_RSN) 
#ifdef FEATURE_WLAN_CCX
            || (pRoamProfile->AuthType.authType[0] == eCSR_AUTH_TYPE_CCKM_RSN) ||
            (pRoamProfile->AuthType.authType[0] == eCSR_AUTH_TYPE_CCKM_WPA)
#endif
        )
        {
            hdd_SendFTAssocResponse(dev, pAdapter, pCsrRoamInfo);
        }
#endif
    }
    else if (eConnectionState_IbssConnected == pHddStaCtx->conn_info.connState) //                
    {
        memcpy(wrqu.ap_addr.sa_data, pHddStaCtx->conn_info.bssId, sizeof(wrqu.ap_addr.sa_data));
        type = WLAN_STA_ASSOC_DONE_IND;
        pr_info("wlan: new IBSS connection to %02x:%02x:%02x:%02x:%02x:%02x",
                      pHddStaCtx->conn_info.bssId[0],
                      pHddStaCtx->conn_info.bssId[1],
                      pHddStaCtx->conn_info.bssId[2],
                      pHddStaCtx->conn_info.bssId[3],
                      pHddStaCtx->conn_info.bssId[4],
                      pHddStaCtx->conn_info.bssId[5]);
    }
    else /*                */
    {
        pr_info("wlan: disconnected\n");
        type = WLAN_STA_DISASSOC_DONE_IND;
        memset(wrqu.ap_addr.sa_data,'\0',ETH_ALEN);
    }

    msg = NULL;
    /*                                                                 
                                                                      */
    if(pHddCtx->isLoadUnloadInProgress != TRUE)
    {
        wireless_send_event(dev, we_event, &wrqu, msg);
#ifdef FEATURE_WLAN_CCX
        if(eConnectionState_Associated == pHddStaCtx->conn_info.connState)/*            */
        {        
            if ( (pRoamProfile->AuthType.authType[0] == eCSR_AUTH_TYPE_CCKM_RSN) || 
                (pRoamProfile->AuthType.authType[0] == eCSR_AUTH_TYPE_CCKM_WPA) ) 
            hdd_SendNewAPChannelInfo(dev, pAdapter, pCsrRoamInfo);
        }
#endif
    }
    send_btc_nlink_msg(type, 0);
}

void hdd_connRemoveConnectInfo( hdd_station_ctx_t *pHddStaCtx )
{
   //                                       
   pHddStaCtx->conn_info.staId [ 0 ] = 0;
   vos_mem_zero( &pHddStaCtx->conn_info.bssId, sizeof( v_MACADDR_t ) );
   vos_mem_zero( &pHddStaCtx->conn_info.peerMacAddress[ 0 ], sizeof( v_MACADDR_t ) );

   //                            
   pHddStaCtx->conn_info.authType         = eCSR_AUTH_TYPE_OPEN_SYSTEM;
   pHddStaCtx->conn_info.mcEncryptionType = eCSR_ENCRYPT_TYPE_NONE;
   pHddStaCtx->conn_info.ucEncryptionType = eCSR_ENCRYPT_TYPE_NONE;

   vos_mem_zero( &pHddStaCtx->conn_info.Keys, sizeof( tCsrKeys ) );

   //                        
   pHddStaCtx->conn_info.connDot11DesiredBssType = eCSR_BSS_TYPE_ANY;
   hdd_connSetConnectionState( pHddStaCtx, eConnectionState_NotConnected );  

   vos_mem_zero( &pHddStaCtx->conn_info.SSID, sizeof( tCsrSSIDInfo ) );
}
/*                                          */
static VOS_STATUS hdd_roamDeregisterSTA( hdd_adapter_t *pAdapter, tANI_U8 staId )
{
    VOS_STATUS vosStatus;
    hdd_disconnect_tx_rx(pAdapter);
    vosStatus = WLANTL_ClearSTAClient( (WLAN_HDD_GET_CTX(pAdapter))->pvosContext, staId );
    if ( !VOS_IS_STATUS_SUCCESS( vosStatus ) )
    {
        VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_WARN,
                   "%s: WLANTL_ClearSTAClient() failed to for staID %d.  "
                   "Status= %d [0x%08lX]",
                   __FUNCTION__, staId, vosStatus, vosStatus );
    }
    return( vosStatus );
}


static eHalStatus hdd_DisConnectHandler( hdd_adapter_t *pAdapter, tCsrRoamInfo *pRoamInfo,
                                            tANI_U32 roamId, eRoamCmdStatus roamStatus,
                                            eCsrRoamResult roamResult )
{
    eHalStatus status = eHAL_STATUS_SUCCESS;
    VOS_STATUS vstatus;
    struct net_device *dev = pAdapter->dev;
    hdd_context_t *pHddCtx = WLAN_HDD_GET_CTX(pAdapter);
    hdd_station_ctx_t *pHddStaCtx = WLAN_HDD_GET_STATION_CTX_PTR(pAdapter);

    //             
    if(dev == NULL)
    {
        hddLog(VOS_TRACE_LEVEL_INFO_HIGH, 
          "%s: net_dev is released return", __func__);
        return eHAL_STATUS_FAILURE;
    }

    //                                               
    netif_tx_disable(dev);
    netif_carrier_off(dev);
    
    INIT_COMPLETION(pAdapter->disconnect_comp_var);
    hdd_connSetConnectionState( pHddStaCtx, eConnectionState_Disconnecting );
    /*                        */
    if((pHddCtx->concurrency_mode <= 1) && (pHddCtx->no_of_sessions[WLAN_HDD_INFRA_STATION] <=1))
    {
        pHddCtx->isAmpAllowed = VOS_TRUE;
    }
    hdd_clearRoamProfileIe( pAdapter );

    //                                                  
    hdd_SendAssociationEvent(dev,pRoamInfo);
#ifdef CONFIG_CFG80211
    /*                                        */
    if(roamStatus != eCSR_ROAM_IBSS_LEAVE)
    {
        /*                                                                 
                                                                            */
        if(pHddCtx->isLoadUnloadInProgress != TRUE)
        {
            hddLog(VOS_TRACE_LEVEL_INFO_HIGH, 
                    "%s: sent disconnected event to nl80211", 
                    __func__);
            /*                                                        */
            if( eCSR_ROAM_LOSTLINK == roamStatus )
            {
                cfg80211_disconnected(dev, pRoamInfo->reasonCode, NULL, 0, GFP_KERNEL);
            }
            else
            {
                cfg80211_disconnected(dev, WLAN_REASON_UNSPECIFIED, NULL, 0, GFP_KERNEL); 
            }

            //                             
            //                                
            //           

            //                                                                
            //                                                    
            if(((WLAN_HDD_INFRA_STATION == pAdapter->device_mode) ||
                (WLAN_HDD_P2P_CLIENT == pAdapter->device_mode)) &&
                (vos_concurrent_sessions_running()))
            {
               //                                               
               hdd_context_t *pHddCtx = NULL;
               v_CONTEXT_t pVosContext = vos_get_global_context( VOS_MODULE_ID_HDD, NULL );    

               if (NULL != pVosContext)
               {
                   pHddCtx = vos_get_context( VOS_MODULE_ID_HDD, pVosContext);

                   if(NULL != pHddCtx)
                   {
                       //                                         
                       if((0 == pHddCtx->no_of_sessions[VOS_STA_SAP_MODE]) &&
                          (0 == pHddCtx->no_of_sessions[VOS_P2P_GO_MODE]))
                       {
                           hdd_enable_bmps_imps(pHddCtx);
                       }
                   }
               }
            }
        }
    }
#endif
    

    //                                                            
    vstatus = hdd_roamDeregisterSTA( pAdapter, pHddStaCtx->conn_info.staId [0] );
    if ( !VOS_IS_STATUS_SUCCESS(vstatus ) )
    {
        VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_WARN,
                  "hdd_roamDeregisterSTA() failed to for staID %d.  "
                  "Status= %d [0x%x]",
                    pHddStaCtx->conn_info.staId[0], status, status );

        status = eHAL_STATUS_FAILURE;
    }

    pHddCtx->sta_to_adapter[pHddStaCtx->conn_info.staId[0]] = NULL;
    //                                          
    hdd_connRemoveConnectInfo( pHddStaCtx );

    //                                                 
    complete(&pAdapter->disconnect_comp_var);
    return( status );
}
static VOS_STATUS hdd_roamRegisterSTA( hdd_adapter_t *pAdapter,
                                       tCsrRoamInfo *pRoamInfo,
                                       v_U8_t staId,
                                       v_MACADDR_t *pPeerMacAddress,
                                       tSirBssDescription *pBssDesc )
{
   VOS_STATUS vosStatus = VOS_STATUS_E_FAILURE;
   WLAN_STADescType staDesc = {0};
   eCsrEncryptionType connectedCipherAlgo;
   v_BOOL_t  fConnected;
   hdd_station_ctx_t *pHddStaCtx = WLAN_HDD_GET_STATION_CTX_PTR(pAdapter);
   hdd_context_t *pHddCtx = WLAN_HDD_GET_CTX(pAdapter);
   
   //                                                             
   staDesc.ucSTAId = staId;

   if ( pHddStaCtx->conn_info.connDot11DesiredBssType == eMib_dot11DesiredBssType_infrastructure)
   { 
      staDesc.wSTAType = WLAN_STA_INFRA;
      
      //                                                                                
      //                              
      vos_mem_copy( staDesc.vSTAMACAddress.bytes, pHddStaCtx->conn_info.bssId,sizeof(pHddStaCtx->conn_info.bssId) ); 
   }
   else 
   {
      //                                                               
      staDesc.wSTAType = WLAN_STA_IBSS;
      
      //                                                                                  
      //                                                                                
      //                                                                                  
      //                                               
      vos_mem_copy( staDesc.vSTAMACAddress.bytes, pPeerMacAddress->bytes,sizeof(pPeerMacAddress->bytes) );
      vos_mem_copy( staDesc.vBSSIDforIBSS.bytes, pHddStaCtx->conn_info.bssId,6 );
   }
      
   vos_copy_macaddr( &staDesc.vSelfMACAddress, &pAdapter->macAddressCurrent );

   //                                
   if (hdd_wmm_is_active(pAdapter))
   {
      staDesc.ucQosEnabled = 1;
   }
   else
   {
      staDesc.ucQosEnabled = 0;
   }

   fConnected = hdd_connGetConnectedCipherAlgo( pHddStaCtx, &connectedCipherAlgo );
   if ( connectedCipherAlgo != eCSR_ENCRYPT_TYPE_NONE )
   {
      staDesc.ucProtectedFrame = 1;
   }
   else
   {
      staDesc.ucProtectedFrame = 0;

   }

#ifdef FEATURE_WLAN_CCX
   staDesc.ucIsCcxSta = pRoamInfo->isCCXAssoc;
#endif //                

#ifdef VOLANS_ENABLE_SW_REPLAY_CHECK
   /*                                                            */
   if( (eCSR_ENCRYPT_TYPE_TKIP == connectedCipherAlgo) || (eCSR_ENCRYPT_TYPE_AES == connectedCipherAlgo))
   {
       /*                                      
                                                  
                               */
       staDesc.ucIsReplayCheckValid = VOS_TRUE;
       VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_INFO,
                 "HDD register TL ucIsReplayCheckValid %d: Replay check is needed for station", staDesc.ucIsReplayCheckValid);
   }
  
   else
   {
      /*                                            
                    */
        staDesc.ucIsReplayCheckValid = VOS_FALSE; 
        VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_INFO,
                 "HDD register TL ucIsReplayCheckValid %d", staDesc.ucIsReplayCheckValid);
   }
#endif

#ifdef FEATURE_WLAN_WAPI
   hddLog(LOG1, "%s: WAPI STA Registered: %d", __FUNCTION__, pAdapter->wapi_info.fIsWapiSta);
   if (pAdapter->wapi_info.fIsWapiSta)
   {
      staDesc.ucIsWapiSta = 1;
   }
   else
   {
      staDesc.ucIsWapiSta = 0;
   }
#endif /*                   */

   VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_INFO_MED,
                 "HDD register TL Sec_enabled= %d.", staDesc.ucProtectedFrame );

#ifdef FEATURE_WLAN_INTEGRATED_SOC
   //                                                 
   staDesc.ucSwFrameTXXlation = 1;
#else
   /*                                                                               */
   if (vos_concurrent_sessions_running())
   {
      staDesc.ucSwFrameTXXlation = 1;
   }
   else
   {
      staDesc.ucSwFrameTXXlation = 0;
   }
#endif
   staDesc.ucSwFrameRXXlation = 1;
   staDesc.ucAddRmvLLC = 1;
   VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_INFO_HIGH, "HDD register TL QoS_enabled=%d", 
              staDesc.ucQosEnabled );
   //                                
   staDesc.ucUcastSig  = pRoamInfo->ucastSig;
   staDesc.ucBcastSig  = pRoamInfo->bcastSig;
   staDesc.ucInitState = pRoamInfo->fAuthRequired ?
      WLANTL_STA_CONNECTED : WLANTL_STA_AUTHENTICATED;
   //                                      
   vosStatus = WLANTL_RegisterSTAClient( pHddCtx->pvosContext, 
                                         hdd_rx_packet_cbk, 
                                         hdd_tx_complete_cbk, 
                                         hdd_tx_fetch_packet_cbk, &staDesc,
                                         pBssDesc->rssi );
   
   if ( !VOS_IS_STATUS_SUCCESS( vosStatus ) )
   {
      VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_WARN, 
                 "WLANTL_RegisterSTAClient() failed to register.  Status= %d [0x%08lX]",
                 vosStatus, vosStatus );
      return vosStatus;      
   }                                            

   //                                                                             
   //                                                                                
   //                                                                                    
   //       
   
   VOS_ASSERT( fConnected );
  
   if ( !pRoamInfo->fAuthRequired )
   {
      VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_INFO_MED,
                 "open/shared auth StaId= %d.  Changing TL state to AUTHENTICATED at Join time", pHddStaCtx->conn_info.staId[ 0 ] );
   
      //                                                                      
      //                                
      vosStatus = WLANTL_ChangeSTAState( pHddCtx->pvosContext, staDesc.ucSTAId, 
                                         WLANTL_STA_AUTHENTICATED );
  
      pHddStaCtx->conn_info.uIsAuthenticated = VOS_TRUE;
   }                                            
   else
   {
      VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_INFO_MED,
                 "ULA auth StaId= %d.  Changing TL state to CONNECTED at Join time", pHddStaCtx->conn_info.staId[ 0 ] );
   
      vosStatus = WLANTL_ChangeSTAState( pHddCtx->pvosContext, staDesc.ucSTAId, 
                                         WLANTL_STA_CONNECTED );

      pHddStaCtx->conn_info.uIsAuthenticated = VOS_FALSE;
   }      
   return( vosStatus );
}

#if  defined (FEATURE_WLAN_CCX) || defined(FEATURE_WLAN_LFR)
static void hdd_SendReAssocEvent(struct net_device *dev, hdd_adapter_t *pAdapter,
    tCsrRoamInfo *pCsrRoamInfo, v_U8_t *reqRsnIe, tANI_U32 reqRsnLength)
{
    unsigned int len = 0;
    u8 *pFTAssocRsp = NULL;
    v_U8_t *rspRsnIe = kmalloc(IW_GENERIC_IE_MAX, GFP_KERNEL);
    tANI_U32 rspRsnLength = 0;
    struct ieee80211_channel *chan;

    if (!rspRsnIe)
    {
        hddLog(LOGE, "%s: Unable to allocate RSN IE", __func__);
        return;
    }

    if (pCsrRoamInfo == NULL)
    {
        hddLog(LOGE, "%s: Invalid CSR roam info", __func__);
        goto done;
    }

    if (pCsrRoamInfo->nAssocRspLength == 0)
    {
        hddLog(LOGE, "%s: Invalid assoc response length", __func__);
        goto done;
    }

    pFTAssocRsp = (u8 *)(pCsrRoamInfo->pbFrames + pCsrRoamInfo->nBeaconLength +
                    pCsrRoamInfo->nAssocReqLength);
    if (pFTAssocRsp == NULL)
        goto done;

    //                                     
    pFTAssocRsp += FT_ASSOC_RSP_IES_OFFSET;
    hddLog(LOG1, "%s: AssocRsp is now at %02x%02x\n", __func__,
                    (unsigned int)pFTAssocRsp[0],
                    (unsigned int)pFTAssocRsp[1]);

    //                                                                 
    len = pCsrRoamInfo->nAssocRspLength - FT_ASSOC_RSP_IES_OFFSET;
    rspRsnLength = len;
    memcpy(rspRsnIe, pFTAssocRsp, len);
    memset(rspRsnIe + len, 0, IW_GENERIC_IE_MAX - len);

    chan = ieee80211_get_channel(pAdapter->wdev.wiphy, (int) pCsrRoamInfo->pBssDesc->channelId);
    cfg80211_roamed(dev,chan,pCsrRoamInfo->bssid,
                    reqRsnIe, reqRsnLength,
                    rspRsnIe, rspRsnLength,GFP_KERNEL);

done:
    kfree(rspRsnIe);
}
#endif /*                  */

static eHalStatus hdd_AssociationCompletionHandler( hdd_adapter_t *pAdapter, tCsrRoamInfo *pRoamInfo, 
                                                    tANI_U32 roamId, eRoamCmdStatus roamStatus,                                                
                                                    eCsrRoamResult roamResult )
{
    struct net_device *dev = pAdapter->dev;
    hdd_context_t *pHddCtx = WLAN_HDD_GET_CTX(pAdapter);
    hdd_station_ctx_t *pHddStaCtx = WLAN_HDD_GET_STATION_CTX_PTR(pAdapter);
    VOS_STATUS vosStatus;
#if  defined (FEATURE_WLAN_CCX) || defined(FEATURE_WLAN_LFR)
    int ft_carrier_on = FALSE;
#endif
    int status;
 
    if ( eCSR_ROAM_RESULT_ASSOCIATED == roamResult )
    {
        hdd_connSetConnectionState( pHddStaCtx, eConnectionState_Associated );

        //                                     
        hdd_connSaveConnectInfo( pAdapter, pRoamInfo, eCSR_BSS_TYPE_INFRASTRUCTURE );
#ifdef FEATURE_WLAN_WAPI
        if ( pRoamInfo->u.pConnectedProfile->AuthType == eCSR_AUTH_TYPE_WAPI_WAI_CERTIFICATE ||
                pRoamInfo->u.pConnectedProfile->AuthType == eCSR_AUTH_TYPE_WAPI_WAI_PSK )
        {
            pAdapter->wapi_info.fIsWapiSta = 1;
        }
        else
        {
            pAdapter->wapi_info.fIsWapiSta = 0;
        }
#endif  /*                   */

        //                                       
        hdd_SendAssociationEvent(dev,pRoamInfo);


        //                                                 
        INIT_COMPLETION(pAdapter->linkup_event_var);

        /*
                                                                                                                 
                                                                                                                  
                                                                                                                    
                                                              
           */
#if  defined (WLAN_FEATURE_VOWIFI_11R) || defined (FEATURE_WLAN_CCX) || defined(FEATURE_WLAN_LFR)
        if (pHddStaCtx->ft_carrier_on == FALSE)
        {
#endif
            //                                                                                                           
            pAdapter->isLinkUpSvcNeeded = TRUE;

            //                                                                                                           
            pAdapter->isLinkUpSvcNeeded = TRUE;

            //                                             
            netif_carrier_on(dev);

            //                                                                                
            status = wait_for_completion_interruptible_timeout(&pAdapter->linkup_event_var,
                                                   msecs_to_jiffies(ASSOC_LINKUP_TIMEOUT));
            if(!status) 
            {
                hddLog(VOS_TRACE_LEVEL_WARN, "%s: Warning:ASSOC_LINKUP_TIMEOUT", __func__);
            }

            //                                                                                            
            pAdapter->isLinkUpSvcNeeded = FALSE;
#if  defined (WLAN_FEATURE_VOWIFI_11R) || defined (FEATURE_WLAN_CCX) || defined(FEATURE_WLAN_LFR)
        }
        else { 
            pHddStaCtx->ft_carrier_on = FALSE;
#if  defined (FEATURE_WLAN_CCX) || defined(FEATURE_WLAN_LFR)
            ft_carrier_on = TRUE;
#endif /*                  */
        }
#endif
        pHddCtx->sta_to_adapter[pRoamInfo->staId] = pAdapter;

        //                                                                                  
        //                 
        //                                                                                              
        if( !pRoamInfo->fReassocReq )
        {
#ifdef CONFIG_CFG80211
            v_U8_t reqRsnIe[DOT11F_IE_RSN_MAX_LEN];
            v_U8_t rspRsnIe[DOT11F_IE_RSN_MAX_LEN];
            tANI_U32 reqRsnLength = DOT11F_IE_RSN_MAX_LEN;
            tANI_U32 rspRsnLength = DOT11F_IE_RSN_MAX_LEN;
            struct cfg80211_bss *bss;

            /*                                  */
            bss = wlan_hdd_cfg80211_update_bss_db(pAdapter, pRoamInfo);
            if (NULL == bss)
            {
                pr_err("wlan: Not able to create BSS entry\n");
                return eHAL_STATUS_FAILURE;
            }

            /*                                                       */
            csrRoamGetWpaRsnReqIE(WLAN_HDD_GET_HAL_CTX(pAdapter),
                    pAdapter->sessionId,
                    &reqRsnLength,
                    reqRsnIe);

            csrRoamGetWpaRsnRspIE(WLAN_HDD_GET_HAL_CTX(pAdapter),
                    pAdapter->sessionId,
                    &rspRsnLength,
                    rspRsnIe);
#if  defined (FEATURE_WLAN_CCX) || defined(FEATURE_WLAN_LFR)
            if(ft_carrier_on)
                    hdd_SendReAssocEvent(dev, pAdapter, pRoamInfo, reqRsnIe, reqRsnLength);
            else
#endif /*                  */

            {
            /*                                  */
            cfg80211_connect_result(dev, pRoamInfo->bssid, 
                    reqRsnIe, reqRsnLength, 
                    rspRsnIe, rspRsnLength,
                    WLAN_STATUS_SUCCESS, 
                    GFP_KERNEL); 

            cfg80211_put_bss(bss);
            }
#endif

            //                                                 
            vosStatus = hdd_roamRegisterSTA( pAdapter,
                    pRoamInfo,
                    pHddStaCtx->conn_info.staId[ 0 ],
                    NULL,
                    pRoamInfo->pBssDesc );
        }
        else
        {
            //                    
            if( pRoamInfo->fAuthRequired )
            {
                vosStatus = WLANTL_ChangeSTAState( pHddCtx->pvosContext, pHddStaCtx->conn_info.staId[ 0 ], 
                        WLANTL_STA_CONNECTED );
                pHddStaCtx->conn_info.uIsAuthenticated = VOS_FALSE;
            }
            else
            {
                vosStatus = WLANTL_ChangeSTAState( pHddCtx->pvosContext, pHddStaCtx->conn_info.staId[ 0 ], 
                        WLANTL_STA_AUTHENTICATED );
                pHddStaCtx->conn_info.uIsAuthenticated = VOS_TRUE;
            }
        }

        if ( VOS_IS_STATUS_SUCCESS( vosStatus ) )
        {
            //                                               
            hdd_wmm_assoc(pAdapter, pRoamInfo, eCSR_BSS_TYPE_INFRASTRUCTURE);
        }
        else
        {
            VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_WARN,
                    "Cannot register STA with TL.  Failed with vosStatus = %d [%08lX]",
                    vosStatus, vosStatus );
        }

        //                
        netif_tx_wake_all_queues(dev);
    }  
    else 
    {
        hdd_context_t* pHddCtx = (hdd_context_t*)pAdapter->pHddCtx;

        hdd_wext_state_t *pWextState = WLAN_HDD_GET_WEXT_STATE_PTR(pAdapter);
        pr_info("wlan: connection failed with %02x:%02x:%02x:%02x:%02x:%02x"
                " reason:%d and Status:%d\n", pWextState->req_bssId[0],
                pWextState->req_bssId[1], pWextState->req_bssId[2],
                pWextState->req_bssId[3], pWextState->req_bssId[4],
                pWextState->req_bssId[5], roamResult, roamStatus);

        /*                             */
        hdd_connSetConnectionState( pHddStaCtx, eConnectionState_NotConnected);
        if((pHddCtx->concurrency_mode <= 1) && (pHddCtx->no_of_sessions[WLAN_HDD_INFRA_STATION] <=1))
        {
            pHddCtx->isAmpAllowed = VOS_TRUE;
        }

        //                             
        //                                
        //           

        //                                                                
        //                                                    
        if(((WLAN_HDD_INFRA_STATION == pAdapter->device_mode) ||
            (WLAN_HDD_P2P_CLIENT == pAdapter->device_mode)) &&
            (vos_concurrent_sessions_running()))
        {
           //                                               
           hdd_context_t *pHddCtx = NULL;
           v_CONTEXT_t pVosContext = vos_get_global_context( VOS_MODULE_ID_HDD, NULL );

           if (NULL != pVosContext)
           {
               pHddCtx = vos_get_context( VOS_MODULE_ID_HDD, pVosContext);

               if(NULL != pHddCtx)
               {
                   //                                         
                   if((0 == pHddCtx->no_of_sessions[VOS_STA_SAP_MODE]) &&
                      (0 == pHddCtx->no_of_sessions[VOS_P2P_GO_MODE]))
                   {
                       hdd_enable_bmps_imps(pHddCtx);
                   }
               }
           }
        }

#ifdef CONFIG_CFG80211
        /*                                             */
        if(eCSR_ROAM_RESULT_ASSOC_FAIL_CON_CHANNEL == roamResult)
        {
           cfg80211_connect_result(dev, pWextState->req_bssId,
                NULL, 0, NULL, 0,
                WLAN_STATUS_ASSOC_DENIED_UNSPEC, 
                GFP_KERNEL);
        }
        else
        {
           cfg80211_connect_result(dev, pWextState->req_bssId,
                NULL, 0, NULL, 0,
                WLAN_STATUS_UNSPECIFIED_FAILURE, 
                GFP_KERNEL);
        }
#endif 

        /*                      */
        hdd_clearRoamProfileIe( pAdapter );

        netif_tx_disable(dev);
        netif_carrier_off(dev);
        
        if (WLAN_HDD_P2P_CLIENT != pAdapter->device_mode)
        {
            /*                                                       
                                                              */
            hdd_ResetCountryCodeAfterDisAssoc(pAdapter);
        }
    }

    return eHAL_STATUS_SUCCESS;
}

/*                                                                             
  
                                                                            
                                                                              
                           
  
                                                                             */
static eHalStatus roamRoamIbssIndicationHandler( hdd_adapter_t *pAdapter, tCsrRoamInfo *pRoamInfo, 
   tANI_U32 roamId, eRoamCmdStatus roamStatus,                                                
   eCsrRoamResult roamResult )
{
   switch( roamResult )
   {
      //                                                     
      case eCSR_ROAM_RESULT_IBSS_STARTED:
      case eCSR_ROAM_RESULT_IBSS_JOIN_SUCCESS:
      {
         //                                               
         VOS_ASSERT( pRoamInfo );
        
         //                                                                       
         //                                                      
         hdd_connSetConnectionState( WLAN_HDD_GET_STATION_CTX_PTR(pAdapter), eConnectionState_IbssDisconnected );

         break;
      }
      
      case eCSR_ROAM_RESULT_IBSS_START_FAILED:
      {
         VOS_ASSERT( pRoamInfo );
         
         break;
      }
      
      default:
         break;
   }   
   
    return( eHAL_STATUS_SUCCESS );
}

/*                                                                             
  
                                                                               
                                                                    
                                            
                                                                       
               
  
                                                                             */
static int roamSaveIbssStation( hdd_station_ctx_t *pHddStaCtx, v_U8_t staId, v_MACADDR_t *peerMacAddress )
{
   int fSuccess = FALSE;
   int idx = 0;
   
   for ( idx = 0; idx < HDD_MAX_NUM_IBSS_STA; idx++ )
   {
      if ( 0 == pHddStaCtx->conn_info.staId[ idx ] )
      {
         pHddStaCtx->conn_info.staId[ idx ] = staId;
      
         vos_copy_macaddr( &pHddStaCtx->conn_info.peerMacAddress[ idx ], peerMacAddress );
         
         fSuccess = TRUE;
         break;
      }
   }
   
   return( fSuccess );   
}
/*                                                                             
  
                                                                                   
                                                                          
               
  
                                                                             */
static int roamRemoveIbssStation( hdd_station_ctx_t *pHddStaCtx, v_U8_t staId )
{
   int fSuccess = FALSE;
   int idx = 0;
   v_U8_t  valid_idx   = 0;
   v_U8_t  del_idx   = 0;
   
   for ( idx = 0; idx < HDD_MAX_NUM_IBSS_STA; idx++ )
   {
      if ( staId == pHddStaCtx->conn_info.staId[ idx ] )
      {
         pHddStaCtx->conn_info.staId[ idx ] = 0;

         vos_zero_macaddr( &pHddStaCtx->conn_info.peerMacAddress[ idx ] );

         fSuccess = TRUE;
         //                                                          
         del_idx = idx;
      }
      else
      {
         if (pHddStaCtx->conn_info.staId[idx] != 0) 
         {
            valid_idx = idx;
         }
      }
   }

   //                                                            
   if (fSuccess == TRUE)
   {
      if (del_idx == 0)
      {
         if (pHddStaCtx->conn_info.staId[valid_idx] != 0)
         {
            pHddStaCtx->conn_info.staId[0] = pHddStaCtx->conn_info.staId[valid_idx];
            vos_copy_macaddr( &pHddStaCtx->conn_info.peerMacAddress[ 0 ],
               &pHddStaCtx->conn_info.peerMacAddress[ valid_idx ]);

            pHddStaCtx->conn_info.staId[valid_idx] = 0;
            vos_zero_macaddr( &pHddStaCtx->conn_info.peerMacAddress[ valid_idx ] );
         }
      }
   }
   return( fSuccess );
}

/*                                                                             
  
                                                                        
                             
  
                                                                             */
static eHalStatus roamIbssConnectHandler( hdd_adapter_t *pAdapter, tCsrRoamInfo *pRoamInfo )
{
   VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_INFO, "IBSS Connect Indication from SME!!!" );
   //                                                                                             
   hdd_connSetConnectionState( WLAN_HDD_GET_STATION_CTX_PTR(pAdapter), eConnectionState_IbssConnected );

   //                                     
   hdd_connSaveConnectInfo( pAdapter, pRoamInfo, eCSR_BSS_TYPE_IBSS );

   //                                    
   hdd_SendAssociationEvent(pAdapter->dev, pRoamInfo);
#ifdef CONFIG_CFG80211
   /*                                  */
   wlan_hdd_cfg80211_update_bss_db(pAdapter, pRoamInfo);
   /*                                      */
   cfg80211_ibss_joined(pAdapter->dev, &pRoamInfo->bssid[0], GFP_KERNEL);
#endif

   return( eHAL_STATUS_SUCCESS );
}
/*                                                                             
  
                                                                          
  
                                                                             */
static eHalStatus hdd_RoamSetKeyCompleteHandler( hdd_adapter_t *pAdapter, tCsrRoamInfo *pRoamInfo, 
                                                 tANI_U32 roamId, eRoamCmdStatus roamStatus,                                                
                                                 eCsrRoamResult roamResult )
{
   eCsrEncryptionType connectedCipherAlgo;
   v_BOOL_t fConnected   = FALSE;
   VOS_STATUS vosStatus    = VOS_STATUS_E_FAILURE;
   hdd_context_t *pHddCtx = WLAN_HDD_GET_CTX(pAdapter);
   hdd_station_ctx_t *pHddStaCtx = WLAN_HDD_GET_STATION_CTX_PTR(pAdapter);
   ENTER();
   //                                                                     
   //                                                                                
   //                                                                                    
   //       
   fConnected = hdd_connGetConnectedCipherAlgo( pHddStaCtx, &connectedCipherAlgo );
   if( fConnected )
   {
      //                                                        
      //                                                                                       
      //                                                                         
      //                                                   
      if(( eCSR_ROAM_RESULT_AUTHENTICATED == roamResult ) && (pRoamInfo != NULL) && !pRoamInfo->fAuthRequired)
      {
         VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_INFO_MED,
                    "Key set for StaId= %d.  Changing TL state to AUTHENTICATED", pHddStaCtx->conn_info.staId[ 0 ] );
                    
         //                                                                        
         //                                                 
         vosStatus = WLANTL_ChangeSTAState( pHddCtx->pvosContext, pHddStaCtx->conn_info.staId[ 0 ], 
                                            WLANTL_STA_AUTHENTICATED );
 
         pHddStaCtx->conn_info.uIsAuthenticated = VOS_TRUE;
      }
      
      pHddStaCtx->roam_info.roamingState = HDD_ROAM_STATE_NONE;
   }
   else
   {
      //                                                                     
      pHddStaCtx->roam_info.roamingState = HDD_ROAM_STATE_NONE;
   }
   
   EXIT();
   return( eHAL_STATUS_SUCCESS );
}
/*                                                                             
  
                                                                                                         
                                                                             */
static eHalStatus hdd_RoamMicErrorIndicationHandler( hdd_adapter_t *pAdapter, tCsrRoamInfo *pRoamInfo, 
                                                 tANI_U32 roamId, eRoamCmdStatus roamStatus,                                                                              eCsrRoamResult roamResult )
{   
   hdd_station_ctx_t *pHddStaCtx = WLAN_HDD_GET_STATION_CTX_PTR(pAdapter);

   if( eConnectionState_Associated == pHddStaCtx->conn_info.connState &&
      TKIP_COUNTER_MEASURE_STOPED == pHddStaCtx->WextState.mTKIPCounterMeasures )
   {
      struct iw_michaelmicfailure msg;
      union iwreq_data wreq;
      memset(&msg, '\0', sizeof(msg));
      msg.src_addr.sa_family = ARPHRD_ETHER;
      memcpy(msg.src_addr.sa_data, pRoamInfo->u.pMICFailureInfo->taMacAddr, sizeof(pRoamInfo->u.pMICFailureInfo->taMacAddr));
      hddLog(LOG1, "MIC MAC %02x:%02x:%02x:%02x:%02x:%02x",
                                    msg.src_addr.sa_data[0],
                                    msg.src_addr.sa_data[1],
                                    msg.src_addr.sa_data[2],
                                    msg.src_addr.sa_data[3],
                                    msg.src_addr.sa_data[4],
                                    msg.src_addr.sa_data[5]);
  
      if(pRoamInfo->u.pMICFailureInfo->multicast == eSIR_TRUE)
         msg.flags = IW_MICFAILURE_GROUP;
      else 
         msg.flags = IW_MICFAILURE_PAIRWISE;
      memset(&wreq, 0, sizeof(wreq));
      wreq.data.length = sizeof(msg);
      wireless_send_event(pAdapter->dev, IWEVMICHAELMICFAILURE, &wreq, (char *)&msg);
#ifdef CONFIG_CFG80211
      /*                               */
      cfg80211_michael_mic_failure(pAdapter->dev, 
              pRoamInfo->u.pMICFailureInfo->taMacAddr,
              ((pRoamInfo->u.pMICFailureInfo->multicast == eSIR_TRUE) ?
               NL80211_KEYTYPE_GROUP :
               NL80211_KEYTYPE_PAIRWISE),
              pRoamInfo->u.pMICFailureInfo->keyId, 
              pRoamInfo->u.pMICFailureInfo->TSC, 
              GFP_KERNEL);
#endif
      
   }
   
   return( eHAL_STATUS_SUCCESS );
}

/*                                                                             
  
                                                                              
                                          
  
                                                                             */
static eHalStatus roamRoamConnectStatusUpdateHandler( hdd_adapter_t *pAdapter, tCsrRoamInfo *pRoamInfo, 
   tANI_U32 roamId, eRoamCmdStatus roamStatus,                                                
   eCsrRoamResult roamResult )
{
   VOS_STATUS vosStatus;

   hdd_context_t *pHddCtx = WLAN_HDD_GET_CTX(pAdapter);
   switch( roamResult )
   {
      case eCSR_ROAM_RESULT_IBSS_NEW_PEER:
      {
         VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_INFO,
                    "IBSS New Peer indication from SME "
                    "with peerMac %2x-%2x-%2x-%2x-%2x-%2x  and  stationID= %d",
                    pRoamInfo->peerMac[0], pRoamInfo->peerMac[1], pRoamInfo->peerMac[2],
                    pRoamInfo->peerMac[3], pRoamInfo->peerMac[4], pRoamInfo->peerMac[5], 
                    pRoamInfo->staId );
         
         if ( !roamSaveIbssStation( WLAN_HDD_GET_STATION_CTX_PTR(pAdapter), pRoamInfo->staId, (v_MACADDR_t *)pRoamInfo->peerMac ) )
         {
            VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_WARN,
                       "New IBSS peer but we already have the max we can handle.  Can't register this one" );
            break;
         }

         pHddCtx->sta_to_adapter[pRoamInfo->staId] = pAdapter;

         //                                                
         vosStatus = hdd_roamRegisterSTA( pAdapter,
                                          pRoamInfo,
                                          pRoamInfo->staId,
                                          (v_MACADDR_t *)pRoamInfo->peerMac,
                                          pRoamInfo->pBssDesc );
         if ( !VOS_IS_STATUS_SUCCESS( vosStatus ) )
         {
            VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR,
               "Cannot register STA with TL for IBSS.  Failed with vosStatus = %d [%08lX]",
               vosStatus, vosStatus );
         }
         
         netif_carrier_on(pAdapter->dev);
         netif_tx_start_all_queues(pAdapter->dev);
         break;
      }
         
      case eCSR_ROAM_RESULT_IBSS_CONNECT:
      {
      
         roamIbssConnectHandler( pAdapter, pRoamInfo );
         
         break;
      }   
      case eCSR_ROAM_RESULT_IBSS_PEER_DEPARTED:
      {

         if ( !roamRemoveIbssStation( WLAN_HDD_GET_STATION_CTX_PTR(pAdapter), pRoamInfo->staId ) )
         {
            VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_WARN,
                    "IBSS peer departed by cannot find peer in our registration table with TL" );
         }

         hdd_roamDeregisterSTA( pAdapter, pRoamInfo->staId );

         pHddCtx->sta_to_adapter[pRoamInfo->staId] = NULL;

         break;
      }
      case eCSR_ROAM_RESULT_IBSS_INACTIVE:
      {
         //                               
         netif_tx_disable(pAdapter->dev);
         netif_carrier_off(pAdapter->dev);
         hdd_connSetConnectionState( WLAN_HDD_GET_STATION_CTX_PTR(pAdapter), eConnectionState_NotConnected );
         
         //                                    
         hdd_SendAssociationEvent(pAdapter->dev, pRoamInfo);
         //                   
         hdd_disconnect_tx_rx(pAdapter);
         break;
      }
      default:
         break;
   
   }
   
   return( eHAL_STATUS_SUCCESS );
}

eHalStatus hdd_smeRoamCallback( void *pContext, tCsrRoamInfo *pRoamInfo, tANI_U32 roamId, 
                                eRoamCmdStatus roamStatus, eCsrRoamResult roamResult )
{
    eHalStatus halStatus = eHAL_STATUS_SUCCESS;
    hdd_adapter_t *pAdapter = (hdd_adapter_t *)pContext;
    hdd_wext_state_t *pWextState= WLAN_HDD_GET_WEXT_STATE_PTR(pAdapter);
    hdd_station_ctx_t *pHddStaCtx = WLAN_HDD_GET_STATION_CTX_PTR(pAdapter);
    VOS_STATUS status = VOS_STATUS_SUCCESS;

    VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_INFO_HIGH,
            "CSR Callback: status= %d result= %d roamID=%ld", 
                    roamStatus, roamResult, roamId ); 

    /*            */
    if (WLAN_HDD_ADAPTER_MAGIC != pAdapter->magic)
    {
       VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_FATAL,
          "pAdapter has invalid magic return"); 
       return eHAL_STATUS_FAILURE;
    }

    switch( roamStatus )
    {
        case eCSR_ROAM_SESSION_OPENED:
            if(pAdapter != NULL)
            {
                set_bit(SME_SESSION_OPENED, &pAdapter->event_flags);
                complete(&pAdapter->session_open_comp_var);
            }
            break;
            
#if  defined (WLAN_FEATURE_VOWIFI_11R) || defined (FEATURE_WLAN_CCX) || defined(FEATURE_WLAN_LFR)
            /*                                                        
                                                                     
                                                                    
                              */
        case eCSR_ROAM_FT_REASSOC_FAILED:
            hddLog(LOG1, FL("Reassoc Failed\n"));
            halStatus = hdd_DisConnectHandler( pAdapter, pRoamInfo, roamId, roamStatus, roamResult );
            /*                                                                     */
            if ((WLAN_HDD_GET_CTX(pAdapter))->hdd_mcastbcast_filter_set == TRUE) {
#ifdef FEATURE_WLAN_NON_INTEGRATED_SOC
#ifdef MSM_PLATFORM
                    hdd_conf_mcastbcast_filter((WLAN_HDD_GET_CTX(pAdapter)), FALSE);
#endif
#endif
                    (WLAN_HDD_GET_CTX(pAdapter))->hdd_mcastbcast_filter_set = FALSE;
            }
            pHddStaCtx->ft_carrier_on = FALSE;
            break;

        case eCSR_ROAM_FT_START:
            //                                                
            //                                                 
            //                                             
            //                                        
            //                                        
            //                                                      
            //                                                   
            //               
            {
                struct net_device *dev = pAdapter->dev;
                netif_tx_disable(dev);
                /*
                                                                        
                                                                              
                                                                     
                                                                               
                                                                            
                                                                               
                                                                               
                                                                            
                                                                              
                                                            
           */
                status = hdd_roamDeregisterSTA( pAdapter, pHddStaCtx->conn_info.staId [0] );
                if ( !VOS_IS_STATUS_SUCCESS(status ) )
                {
                    VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_WARN,
                            FL("hdd_roamDeregisterSTA() failed to for staID %d.  Status= %d [0x%x]"),
                            pHddStaCtx->conn_info.staId[0], status, status );
                    halStatus = eHAL_STATUS_FAILURE;
                }		
            }
            pHddStaCtx->ft_carrier_on = TRUE;
            break;
#endif

        case eCSR_ROAM_SHOULD_ROAM:
           //                         
            {
                struct net_device *dev = pAdapter->dev;
                hdd_station_ctx_t *pHddStaCtx = WLAN_HDD_GET_STATION_CTX_PTR(pAdapter);
                //                                               
                netif_tx_disable(dev);
#if  defined (WLAN_FEATURE_VOWIFI_11R) || defined (FEATURE_WLAN_CCX) || defined(FEATURE_WLAN_LFR)
                if (pHddStaCtx->ft_carrier_on == FALSE)
                {
#endif
                    netif_carrier_off(dev);
#if  defined (WLAN_FEATURE_VOWIFI_11R) || defined (FEATURE_WLAN_CCX) || defined(FEATURE_WLAN_LFR)
                }
#endif

#if  !(defined (WLAN_FEATURE_VOWIFI_11R) || defined (FEATURE_WLAN_CCX) || defined(FEATURE_WLAN_LFR))
                //                                                            
                status = hdd_roamDeregisterSTA( pAdapter, pHddStaCtx->conn_info.staId [0] );
                if ( !VOS_IS_STATUS_SUCCESS(status ) )
                {
                    VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_WARN,
                        FL("hdd_roamDeregisterSTA() failed to for staID %d.  Status= %d [0x%x]"),
                                        pHddStaCtx->conn_info.staId[0], status, status );
                    halStatus = eHAL_STATUS_FAILURE;
                }
#endif
                //                                          
                hdd_connRemoveConnectInfo( WLAN_HDD_GET_STATION_CTX_PTR(pAdapter) );
            }
           break;
        case eCSR_ROAM_LOSTLINK:
        case eCSR_ROAM_DISASSOCIATED:
            {
                hdd_context_t* pHddCtx = (hdd_context_t*)pAdapter->pHddCtx;
                VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_INFO,
                        "****eCSR_ROAM_DISASSOCIATED****");
                halStatus = hdd_DisConnectHandler( pAdapter, pRoamInfo, roamId, roamStatus, roamResult );
                /*                                                                     */
                if ((WLAN_HDD_GET_CTX(pAdapter))->hdd_mcastbcast_filter_set == TRUE) {
                    hdd_conf_mcastbcast_filter((WLAN_HDD_GET_CTX(pAdapter)), FALSE);
                    (WLAN_HDD_GET_CTX(pAdapter))->hdd_mcastbcast_filter_set = FALSE;
                }
#ifdef WLAN_FEATURE_PACKET_FILTERING    
                if (pHddCtx->cfg_ini->isMcAddrListFilter)
                {
                    /*                                   */
                    if(pHddCtx->mc_addr_list.isFilterApplied)
                    {
                        /*                                  */
                        /*             */
                        wlan_hdd_set_mc_addr_list(pHddCtx, FALSE, pAdapter->sessionId);
                    }
                }
#endif

                if (WLAN_HDD_P2P_CLIENT != pAdapter->device_mode)
                {
                    /*                                                                 
                                                                      */
                    hdd_ResetCountryCodeAfterDisAssoc(pAdapter);
                }
            }
            break;
        case eCSR_ROAM_IBSS_LEAVE:
            VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_INFO,
                    "****eCSR_ROAM_IBSS_LEAVE****");
            halStatus = hdd_DisConnectHandler( pAdapter, pRoamInfo, roamId, roamStatus, roamResult );
            break;
        case eCSR_ROAM_ASSOCIATION_COMPLETION:
            VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_INFO,
                    "****eCSR_ROAM_ASSOCIATION_COMPLETION****");
            if (  (roamResult != eCSR_ROAM_RESULT_ASSOCIATED)
               && (   (pWextState->roamProfile.EncryptionType.encryptionType[0] == eCSR_ENCRYPT_TYPE_WEP40_STATICKEY) 
                   || (pWextState->roamProfile.EncryptionType.encryptionType[0] == eCSR_ENCRYPT_TYPE_WEP104_STATICKEY)
                  )
               && (eCSR_AUTH_TYPE_SHARED_KEY != pWextState->roamProfile.AuthType.authType[0])
               )
            {
                v_U32_t roamId = 0;
                VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_INFO_HIGH,
                        "****WEP open authentication failed, trying with shared authentication****");
                (WLAN_HDD_GET_STATION_CTX_PTR(pAdapter))->conn_info.authType = eCSR_AUTH_TYPE_SHARED_KEY;
                pWextState->roamProfile.AuthType.authType[0] = (WLAN_HDD_GET_STATION_CTX_PTR(pAdapter))->conn_info.authType;
                pWextState->roamProfile.csrPersona = pAdapter->device_mode;
                halStatus = sme_RoamConnect( WLAN_HDD_GET_HAL_CTX(pAdapter), pAdapter->sessionId, &(pWextState->roamProfile), &roamId);
            }
            else
            {
                halStatus = hdd_AssociationCompletionHandler( pAdapter, pRoamInfo, roamId, roamStatus, roamResult );
            }

            break;
        case eCSR_ROAM_ASSOCIATION_FAILURE:
            halStatus = hdd_AssociationCompletionHandler( pAdapter, 
                    pRoamInfo, roamId, roamStatus, roamResult );
            break;
        case eCSR_ROAM_IBSS_IND:
            halStatus = roamRoamIbssIndicationHandler( pAdapter, pRoamInfo, roamId, roamStatus, roamResult );
            break;

        case eCSR_ROAM_CONNECT_STATUS_UPDATE:
            halStatus = roamRoamConnectStatusUpdateHandler( pAdapter, pRoamInfo, roamId, roamStatus, roamResult );
            break;            

        case eCSR_ROAM_MIC_ERROR_IND:
            halStatus = hdd_RoamMicErrorIndicationHandler( pAdapter, pRoamInfo, roamId, roamStatus, roamResult );
            break;

        case eCSR_ROAM_SET_KEY_COMPLETE:
            halStatus = hdd_RoamSetKeyCompleteHandler( pAdapter, pRoamInfo, roamId, roamStatus, roamResult );
            break;
#ifdef WLAN_FEATURE_VOWIFI_11R
        case eCSR_ROAM_FT_RESPONSE:
            hdd_SendFTEvent(pAdapter);
            break;
#endif
#ifdef FEATURE_WLAN_LFR
        case eCSR_ROAM_PMK_NOTIFY:
           if (eCSR_AUTH_TYPE_RSN == pHddStaCtx->conn_info.authType) 
           {
               /*                                          */
               halStatus = wlan_hdd_cfg80211_pmksa_candidate_notify(pAdapter, pRoamInfo, 1, false);
           }
           break;
#endif

#ifdef WLAN_FEATURE_P2P
        case eCSR_ROAM_INDICATE_MGMT_FRAME:
            hdd_indicateMgmtFrame( pAdapter,
                                  pRoamInfo->nFrameLength,
                                  pRoamInfo->pbFrames,
                                  pRoamInfo->frameType,
                                  pRoamInfo->rxChan );
            break;
        case eCSR_ROAM_REMAIN_CHAN_READY:
            hdd_remainChanReadyHandler( pAdapter );
            break;
        case eCSR_ROAM_SEND_ACTION_CNF:
            hdd_sendActionCnf( pAdapter,
               (roamResult == eCSR_ROAM_RESULT_NONE) ? TRUE : FALSE );
            break;
#endif
        default:
            break;
    }
    return( halStatus );
}
eCsrAuthType hdd_TranslateRSNToCsrAuthType( u_int8_t auth_suite[4]) 
{
    eCsrAuthType auth_type;
    //                            
    if ( memcmp(auth_suite , ccpRSNOui01, 4) == 0) 
    {
        auth_type = eCSR_AUTH_TYPE_RSN;
    } else 
    if (memcmp(auth_suite , ccpRSNOui02, 4) == 0) 
    {
        auth_type = eCSR_AUTH_TYPE_RSN_PSK;
    } else 
#ifdef WLAN_FEATURE_VOWIFI_11R
    if (memcmp(auth_suite , ccpRSNOui04, 4) == 0) 
    {
        //                                         
        auth_type = eCSR_AUTH_TYPE_FT_RSN_PSK;
    } else 
    if (memcmp(auth_suite , ccpRSNOui03, 4) == 0) 
    {
        //                                            
        auth_type = eCSR_AUTH_TYPE_FT_RSN;
    } else 
#endif
#ifdef FEATURE_WLAN_CCX
    if (memcmp(auth_suite , ccpRSNOui06, 4) == 0) 
    {
        auth_type = eCSR_AUTH_TYPE_CCKM_RSN;
    } else
#endif /*                  */
    { 
        auth_type = eCSR_AUTH_TYPE_UNKNOWN;
    }
    return auth_type;
} 
#ifdef WLAN_SOFTAP_FEATURE
eCsrAuthType 
hdd_TranslateWPAToCsrAuthType(u_int8_t auth_suite[4]) 
#else
static eCsrAuthType hdd_TranslateWPAToCsrAuthType(u_int8_t auth_suite[4]) 
#endif
{
    eCsrAuthType auth_type;
    //                            
    if ( memcmp(auth_suite , ccpWpaOui01, 4) == 0) 
    {
        auth_type = eCSR_AUTH_TYPE_WPA;
    } else 
    if (memcmp(auth_suite , ccpWpaOui02, 4) == 0) 
    {
        auth_type = eCSR_AUTH_TYPE_WPA_PSK;
    } else 
#ifdef FEATURE_WLAN_CCX
    if (memcmp(auth_suite , ccpWpaOui06, 4) == 0) 
    {
        auth_type = eCSR_AUTH_TYPE_CCKM_WPA;
    } else 
#endif /*                  */
    { 
        auth_type = eCSR_AUTH_TYPE_UNKNOWN;
    }
    hddLog(LOG1, FL("auth_type: %d"), auth_type);
    return auth_type;
}
#ifdef WLAN_SOFTAP_FEATURE
eCsrEncryptionType 
hdd_TranslateRSNToCsrEncryptionType(u_int8_t cipher_suite[4])
#else
static eCsrEncryptionType hdd_TranslateRSNToCsrEncryptionType(u_int8_t cipher_suite[4])                                    
#endif
{
    eCsrEncryptionType cipher_type;
    //                              
    if ( memcmp(cipher_suite , ccpRSNOui04, 4) == 0) 
    {
        cipher_type = eCSR_ENCRYPT_TYPE_AES;
    } 
    else if (memcmp(cipher_suite , ccpRSNOui02, 4) == 0) 
    {
        cipher_type = eCSR_ENCRYPT_TYPE_TKIP;
    } 
    else if (memcmp(cipher_suite , ccpRSNOui00, 4) == 0) 
    {
        cipher_type = eCSR_ENCRYPT_TYPE_NONE;
    } 
    else if (memcmp(cipher_suite , ccpRSNOui01, 4) == 0) 
    {
        cipher_type = eCSR_ENCRYPT_TYPE_WEP40_STATICKEY;
    } 
    else if (memcmp(cipher_suite , ccpRSNOui05, 4) == 0) 
    {        
        cipher_type = eCSR_ENCRYPT_TYPE_WEP104_STATICKEY; 
    } 
    else 
    { 
        cipher_type = eCSR_ENCRYPT_TYPE_FAILED;
    }
    hddLog(LOG1, FL("cipher_type: %d"), cipher_type);
    return cipher_type;
} 
/*                                    */
static tANI_U8 hdd_IsMACAddrNULL (tANI_U8 *macAddr, tANI_U8 length)
{
    int i;
    for (i = 0; i < length; i++)
    {
        if (0x00 != (macAddr[i]))
        {
            return FALSE;
        }
    }
    return TRUE;
} /*                                   */
#ifdef WLAN_SOFTAP_FEATURE
eCsrEncryptionType 
hdd_TranslateWPAToCsrEncryptionType(u_int8_t cipher_suite[4])
#else
static eCsrEncryptionType 
hdd_TranslateWPAToCsrEncryptionType(u_int8_t cipher_suite[4])                                    
#endif
{
    eCsrEncryptionType cipher_type;
    //                              
    if ( memcmp(cipher_suite , ccpWpaOui04, 4) == 0) 
    {
        cipher_type = eCSR_ENCRYPT_TYPE_AES;
    } else 
    if (memcmp(cipher_suite , ccpWpaOui02, 4) == 0) 
    {
        cipher_type = eCSR_ENCRYPT_TYPE_TKIP;
    } else 
    if (memcmp(cipher_suite , ccpWpaOui00, 4) == 0) 
    {
        cipher_type = eCSR_ENCRYPT_TYPE_NONE;
    } else 
    if (memcmp(cipher_suite , ccpWpaOui01, 4) == 0) 
    {
        cipher_type = eCSR_ENCRYPT_TYPE_WEP40_STATICKEY;
    } else 
    if (memcmp(cipher_suite , ccpWpaOui05, 4) == 0) 
    {
        cipher_type = eCSR_ENCRYPT_TYPE_WEP104_STATICKEY; 
    } else 
    { 
        cipher_type = eCSR_ENCRYPT_TYPE_FAILED;
    }
    hddLog(LOG1, FL("cipher_type: %d"), cipher_type);
    return cipher_type;
} 

static tANI_S32 hdd_ProcessGENIE(hdd_adapter_t *pAdapter, 
                struct ether_addr *pBssid, 
                eCsrEncryptionType *pEncryptType, 
                eCsrEncryptionType *mcEncryptType, 
                eCsrAuthType *pAuthType, 
                u_int16_t gen_ie_len, 
                u_int8_t *gen_ie) 
{
    tHalHandle halHandle = WLAN_HDD_GET_HAL_CTX(pAdapter);
    eHalStatus result; 
    tDot11fIERSN dot11RSNIE; 
    tDot11fIEWPA dot11WPAIE; 
    tANI_U32 i; 
    tANI_U8 *pRsnIe; 
    tANI_U16 RSNIeLen; 
    tPmkidCacheInfo PMKIDCache[4]; //                      

    /*                                                                           
                 */
    memset( &dot11WPAIE, 0 , sizeof(tDot11fIEWPA) );
    memset( &dot11RSNIE, 0 , sizeof(tDot11fIERSN) );

    //                
    if ((gen_ie_len < VOS_MIN(DOT11F_IE_RSN_MIN_LEN, DOT11F_IE_WPA_MIN_LEN)) ||  
            (gen_ie_len > VOS_MAX(DOT11F_IE_RSN_MAX_LEN, DOT11F_IE_WPA_MAX_LEN)) ) 
        return -EINVAL;
    //           
    if ( gen_ie[0] ==  DOT11F_EID_RSN) 
    {         
        //                
        if ((gen_ie_len < DOT11F_IE_RSN_MIN_LEN ) ||  
                (gen_ie_len > DOT11F_IE_RSN_MAX_LEN) )
        {
            return -EINVAL;
        }
        //                                         
        pRsnIe = gen_ie + 2; 
        RSNIeLen = gen_ie_len - 2; 
        //                   
        dot11fUnpackIeRSN((tpAniSirGlobal) halHandle, 
                            pRsnIe, 
                            RSNIeLen, 
                            &dot11RSNIE);
        //                                                  
        hddLog(LOG1, FL("%s: pairwise cipher suite count: %d"), 
                __FUNCTION__, dot11RSNIE.pwise_cipher_suite_count );
        hddLog(LOG1, FL("%s: authentication suite count: %d"), 
                __FUNCTION__, dot11RSNIE.akm_suite_count);
        /*                                           
                                                              */
        //                          
        //                              
        *pAuthType =  hdd_TranslateRSNToCsrAuthType(dot11RSNIE.akm_suites[0]); 
        //                                    
        *pEncryptType = hdd_TranslateRSNToCsrEncryptionType(dot11RSNIE.pwise_cipher_suites[0]);                     
        //                                 
        *mcEncryptType = hdd_TranslateRSNToCsrEncryptionType(dot11RSNIE.gp_cipher_suite);                     
        //                                          
        for (i=0; i<dot11RSNIE.pmkid_count; i++) 
        {
            if ( pBssid == NULL) 
            {
                break;
            }
            if ( hdd_IsMACAddrNULL( (u_char *) pBssid , sizeof( (char *) pBssid))) 
            {
                break;
            }
            //                                                                
            vos_mem_copy(PMKIDCache[i].BSSID,
                            pBssid, ETHER_ADDR_LEN);
            vos_mem_copy(PMKIDCache[i].PMKID,
                            dot11RSNIE.pmkid[i],
                            CSR_RSN_PMKID_SIZE);
        }
        //                                                                    
        hddLog(LOG1, FL("%s: Calling csrRoamSetPMKIDCache with cache entry %ld."), 
                                                                            __FUNCTION__, i );
        //                                      
        result = sme_RoamSetPMKIDCache(halHandle,pAdapter->sessionId,
                                        PMKIDCache, 
                                        dot11RSNIE.pmkid_count );
    }
    else if (gen_ie[0] == DOT11F_EID_WPA)
    {
        //                
        if ((gen_ie_len < DOT11F_IE_WPA_MIN_LEN ) ||
                    (gen_ie_len > DOT11F_IE_WPA_MAX_LEN))
        {
            return -EINVAL;
        }
        //                                                                
        pRsnIe = gen_ie + 2 + 4; 
        RSNIeLen = gen_ie_len - (2 + 4); 
        //                   
        dot11fUnpackIeWPA((tpAniSirGlobal) halHandle,
                            pRsnIe,
                            RSNIeLen,
                            &dot11WPAIE);
        //                                                 
        hddLog(LOG1, FL("%s: WPA unicast cipher suite count: %d"),
               __FUNCTION__, dot11WPAIE.unicast_cipher_count );
        hddLog(LOG1, FL("%s: WPA authentication suite count: %d"),
               __FUNCTION__, dot11WPAIE.auth_suite_count);
        //                           
        //                             
        *pAuthType =  hdd_TranslateWPAToCsrAuthType(dot11WPAIE.auth_suites[0]);
        //                               
        *pEncryptType = hdd_TranslateWPAToCsrEncryptionType(dot11WPAIE.unicast_ciphers[0]);
        //                               
        *mcEncryptType = hdd_TranslateWPAToCsrEncryptionType(dot11WPAIE.multicast_cipher);
    }
    else
    {
        hddLog(LOGW, FL("gen_ie[0]: %d"), gen_ie[0]);
        return -EINVAL; 
    }
    return 0;
}
int hdd_SetGENIEToCsr( hdd_adapter_t *pAdapter, eCsrAuthType *RSNAuthType)
{
    hdd_wext_state_t *pWextState = WLAN_HDD_GET_WEXT_STATE_PTR(pAdapter);
    v_U32_t status = 0;
    eCsrEncryptionType RSNEncryptType;
    eCsrEncryptionType mcRSNEncryptType;
    struct ether_addr   bSsid;   //                          
    //                          
    //                                                             
    vos_mem_copy(bSsid.ether_addr_octet,
            pWextState->roamProfile.BSSIDs.bssid,
            sizeof(bSsid.ether_addr_octet));
    if (pWextState->WPARSNIE[0] == DOT11F_EID_RSN || pWextState->WPARSNIE[0] == DOT11F_EID_WPA)
    {
        //        
    } 
    else
    {
        return 0;
    }
    //                                                                  
    //                                                                 
    status = hdd_ProcessGENIE(pAdapter,
            &bSsid,   //                          
            &RSNEncryptType,
            &mcRSNEncryptType,
            RSNAuthType,
            pWextState->WPARSNIE[1]+2,
            pWextState->WPARSNIE);
    if (status == 0)
    {
        //                                                              
        pWextState->roamProfile.EncryptionType.numEntries = 1;
        pWextState->roamProfile.mcEncryptionType.numEntries = 1;
        
        pWextState->roamProfile.EncryptionType.encryptionType[0] = RSNEncryptType; //                                  
        pWextState->roamProfile.mcEncryptionType.encryptionType[0] = mcRSNEncryptType;
        hddLog( LOG1, "%s: CSR AuthType = %d, EncryptionType = %d mcEncryptionType = %d", __FUNCTION__, *RSNAuthType, RSNEncryptType, mcRSNEncryptType);
    }
    return 0;
}
int hdd_set_csr_auth_type ( hdd_adapter_t  *pAdapter, eCsrAuthType RSNAuthType)
{
    hdd_wext_state_t *pWextState = WLAN_HDD_GET_WEXT_STATE_PTR(pAdapter);
    tCsrRoamProfile* pRoamProfile = &(pWextState->roamProfile);
    hdd_station_ctx_t *pHddStaCtx = WLAN_HDD_GET_STATION_CTX_PTR(pAdapter);
    ENTER();
    
    pRoamProfile->AuthType.numEntries = 1;
    hddLog( LOG1, "%s: pHddStaCtx->conn_info.authType = %d\n", __FUNCTION__, pHddStaCtx->conn_info.authType);
      
    switch( pHddStaCtx->conn_info.authType)
    {
       case eCSR_AUTH_TYPE_OPEN_SYSTEM:
#ifdef FEATURE_WLAN_CCX
       case eCSR_AUTH_TYPE_CCKM_WPA:
       case eCSR_AUTH_TYPE_CCKM_RSN:
#endif
        if (pWextState->wpaVersion & IW_AUTH_WPA_VERSION_DISABLED) {           
           
           pRoamProfile->AuthType.authType[0] = eCSR_AUTH_TYPE_OPEN_SYSTEM ;
        } else 
        if (pWextState->wpaVersion & IW_AUTH_WPA_VERSION_WPA) {
           
#ifdef FEATURE_WLAN_CCX
            if ((RSNAuthType == eCSR_AUTH_TYPE_CCKM_WPA) && 
                ((pWextState->authKeyMgmt & IW_AUTH_KEY_MGMT_802_1X) 
                 == IW_AUTH_KEY_MGMT_802_1X)) { 
                hddLog( LOG1, "%s: set authType to CCKM WPA. AKM also 802.1X.\n", __FUNCTION__);
                pRoamProfile->AuthType.authType[0] = eCSR_AUTH_TYPE_CCKM_WPA;   
            } else 
            if ((RSNAuthType == eCSR_AUTH_TYPE_CCKM_WPA)) { 
                hddLog( LOG1, "%s: Last chance to set authType to CCKM WPA.\n", __FUNCTION__);
                pRoamProfile->AuthType.authType[0] = eCSR_AUTH_TYPE_CCKM_WPA;   
            } else
#endif
            if((pWextState->authKeyMgmt & IW_AUTH_KEY_MGMT_802_1X)
                    == IW_AUTH_KEY_MGMT_802_1X) {
               pRoamProfile->AuthType.authType[0] = eCSR_AUTH_TYPE_WPA;   
            } else 
            if ((pWextState->authKeyMgmt & IW_AUTH_KEY_MGMT_PSK)
                    == IW_AUTH_KEY_MGMT_PSK) {
               pRoamProfile->AuthType.authType[0] = eCSR_AUTH_TYPE_WPA_PSK;
            } else {     
               pRoamProfile->AuthType.authType[0] = eCSR_AUTH_TYPE_WPA_NONE;
            }    
        }
        if (pWextState->wpaVersion & IW_AUTH_WPA_VERSION_WPA2) {
#ifdef FEATURE_WLAN_CCX
            if ((RSNAuthType == eCSR_AUTH_TYPE_CCKM_RSN) && 
                ((pWextState->authKeyMgmt & IW_AUTH_KEY_MGMT_802_1X) 
                 == IW_AUTH_KEY_MGMT_802_1X)) { 
                hddLog( LOG1, "%s: set authType to CCKM RSN. AKM also 802.1X.\n", __FUNCTION__);
                pRoamProfile->AuthType.authType[0] = eCSR_AUTH_TYPE_CCKM_RSN;   
            } else
            if ((RSNAuthType == eCSR_AUTH_TYPE_CCKM_RSN)) { 
                hddLog( LOG1, "%s: Last chance to set authType to CCKM RSN.\n", __FUNCTION__);
                pRoamProfile->AuthType.authType[0] = eCSR_AUTH_TYPE_CCKM_RSN;   
            } else
#endif

#ifdef WLAN_FEATURE_VOWIFI_11R
            if ((RSNAuthType == eCSR_AUTH_TYPE_FT_RSN) && 
                ((pWextState->authKeyMgmt & IW_AUTH_KEY_MGMT_802_1X) 
                 == IW_AUTH_KEY_MGMT_802_1X)) {
               pRoamProfile->AuthType.authType[0] = eCSR_AUTH_TYPE_FT_RSN;   
            }else
            if ((RSNAuthType == eCSR_AUTH_TYPE_FT_RSN_PSK) && 
                ((pWextState->authKeyMgmt & IW_AUTH_KEY_MGMT_PSK)
                 == IW_AUTH_KEY_MGMT_PSK)) {
               pRoamProfile->AuthType.authType[0] = eCSR_AUTH_TYPE_FT_RSN_PSK;   
            } else
#endif

            if( (pWextState->authKeyMgmt & IW_AUTH_KEY_MGMT_802_1X) 
                    == IW_AUTH_KEY_MGMT_802_1X) {
               pRoamProfile->AuthType.authType[0] = eCSR_AUTH_TYPE_RSN;   
            } else 
            if ( (pWextState->authKeyMgmt & IW_AUTH_KEY_MGMT_PSK)
                    == IW_AUTH_KEY_MGMT_PSK) {
               pRoamProfile->AuthType.authType[0] = eCSR_AUTH_TYPE_RSN_PSK;
            } else {             
               pRoamProfile->AuthType.authType[0] = eCSR_AUTH_TYPE_UNKNOWN;
            }    
        }
        break;     
         
       case eCSR_AUTH_TYPE_SHARED_KEY:
         
          pRoamProfile->AuthType.authType[0] = eCSR_AUTH_TYPE_SHARED_KEY;  
          break;
        default:
         
#ifdef FEATURE_WLAN_CCX
           hddLog( LOG1, "%s: In default, unknown auth type.\n", __FUNCTION__); 
#endif /*                  */
           pRoamProfile->AuthType.authType[0] = eCSR_AUTH_TYPE_UNKNOWN;
           break;
    }
   
    hddLog( LOG1, "%s Set roam Authtype to %d",
            __FUNCTION__, pWextState->roamProfile.AuthType.authType[0]);
   
   EXIT();
    return 0;
}

/*                                                                            
  
                          
                                                           
                            
   
                                            
                                                      
                                                 
                                                    
                                               
  
                                                                            */

int iw_set_essid(struct net_device *dev, 
                        struct iw_request_info *info,
                        union iwreq_data *wrqu, char *extra)
{
    v_U32_t status = 0;
    hdd_wext_state_t *pWextState;
    hdd_adapter_t *pAdapter = WLAN_HDD_GET_PRIV_PTR(dev);
    v_U32_t roamId;
    tCsrRoamProfile          *pRoamProfile;
    eMib_dot11DesiredBssType connectedBssType;
    eCsrAuthType RSNAuthType;
    tHalHandle hHal = WLAN_HDD_GET_HAL_CTX(pAdapter);
    hdd_station_ctx_t *pHddStaCtx = WLAN_HDD_GET_STATION_CTX_PTR(pAdapter);
 
    pWextState = WLAN_HDD_GET_WEXT_STATE_PTR(pAdapter); 
    
    ENTER();

    if ((WLAN_HDD_GET_CTX(pAdapter))->isLogpInProgress)
    {
        VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_FATAL,
                                  "%s:LOGP in Progress. Ignore!!!",__func__);
        return 0;
    }

    if(pWextState->mTKIPCounterMeasures == TKIP_COUNTER_MEASURE_STARTED) {
        hddLog(VOS_TRACE_LEVEL_INFO_HIGH, "%s :Counter measure is in progress", __func__);
        return -EBUSY;
    }
    if( SIR_MAC_MAX_SSID_LENGTH < wrqu->essid.length )
        return -EINVAL;
    pRoamProfile = &pWextState->roamProfile;
    if (pRoamProfile) 
    {
        if ( hdd_connGetConnectedBssType( pHddStaCtx, &connectedBssType ) ||
             ( eMib_dot11DesiredBssType_independent == pHddStaCtx->conn_info.connDot11DesiredBssType ))
        {
            VOS_STATUS vosStatus;
            //                                    
            INIT_COMPLETION(pAdapter->disconnect_comp_var);
            vosStatus = sme_RoamDisconnect( hHal, pAdapter->sessionId, eCSR_DISCONNECT_REASON_UNSPECIFIED );

            if(VOS_STATUS_SUCCESS == vosStatus)
               wait_for_completion_interruptible_timeout(&pAdapter->disconnect_comp_var,
                     msecs_to_jiffies(WLAN_WAIT_TIME_DISCONNECT));
        }
    }
    /*                                         */
    else 
    {
        return -EINVAL;
    }
    /*                                         */
#ifdef CONFIG_CFG80211
    /*                                                         
                                                             
                                                                       
                                 */
    if ( 0 == wrqu->essid.length ) {
        return 0;
    }
#endif

    status = hdd_wmm_get_uapsd_mask(pAdapter,
                                    &pWextState->roamProfile.uapsd_mask);
    if (VOS_STATUS_SUCCESS != status)
    {
       pWextState->roamProfile.uapsd_mask = 0;
    }
    pWextState->roamProfile.SSIDs.numOfSSIDs = 1;
     
    pWextState->roamProfile.SSIDs.SSIDList->SSID.length = wrqu->essid.length;
   
    vos_mem_zero(pWextState->roamProfile.SSIDs.SSIDList->SSID.ssId, sizeof(pWextState->roamProfile.SSIDs.SSIDList->SSID.ssId)); 
    vos_mem_copy((void *)(pWextState->roamProfile.SSIDs.SSIDList->SSID.ssId), extra, wrqu->essid.length);
    if (IW_AUTH_WPA_VERSION_WPA == pWextState->wpaVersion ||
        IW_AUTH_WPA_VERSION_WPA2 == pWextState->wpaVersion ) {
   
        //          
        hdd_SetGENIEToCsr(pAdapter, &RSNAuthType);

        //        
        hdd_set_csr_auth_type(pAdapter, RSNAuthType);
    }
#ifdef FEATURE_WLAN_WAPI
    hddLog(LOG1, "%s: Setting WAPI AUTH Type and Encryption Mode values", __FUNCTION__);
    if (pAdapter->wapi_info.nWapiMode)
    {
        switch (pAdapter->wapi_info.wapiAuthMode)
        {
            case WAPI_AUTH_MODE_PSK:
            {
                hddLog(LOG1, "%s: WAPI AUTH TYPE: PSK: %d", __FUNCTION__, pAdapter->wapi_info.wapiAuthMode);
                pRoamProfile->AuthType.numEntries = 1;
                pRoamProfile->AuthType.authType[0] = eCSR_AUTH_TYPE_WAPI_WAI_PSK;
                break;
            }
            case WAPI_AUTH_MODE_CERT:
            {
                hddLog(LOG1, "%s: WAPI AUTH TYPE: CERT: %d", __FUNCTION__, pAdapter->wapi_info.wapiAuthMode);
                pRoamProfile->AuthType.numEntries = 1;
                pRoamProfile->AuthType.authType[0] = eCSR_AUTH_TYPE_WAPI_WAI_CERTIFICATE;
                break;
            }
        } //              
        if ( pAdapter->wapi_info.wapiAuthMode == WAPI_AUTH_MODE_PSK ||
             pAdapter->wapi_info.wapiAuthMode == WAPI_AUTH_MODE_CERT)
        {
            hddLog(LOG1, "%s: WAPI PAIRWISE/GROUP ENCRYPTION: WPI", __FUNCTION__);
            pRoamProfile->EncryptionType.numEntries = 1;
            pRoamProfile->EncryptionType.encryptionType[0] = eCSR_ENCRYPT_TYPE_WPI;
            pRoamProfile->mcEncryptionType.numEntries = 1;
            pRoamProfile->mcEncryptionType.encryptionType[0] = eCSR_ENCRYPT_TYPE_WPI;
        }
    }
#endif /*                   */
    /*                                               */
    if (0 != pWextState->genIE.length)
    {
        memset( &pWextState->assocAddIE, 0, sizeof(pWextState->assocAddIE) );
        memcpy( pWextState->assocAddIE.addIEdata, pWextState->genIE.addIEdata,
            pWextState->genIE.length);
        pWextState->assocAddIE.length = pWextState->genIE.length;
        pWextState->roamProfile.pAddIEAssoc = pWextState->assocAddIE.addIEdata;
        pWextState->roamProfile.nAddIEAssocLength = pWextState->assocAddIE.length;

        /*                                   */
        memset( &pWextState->genIE, 0, sizeof(pWextState->genIE) );
    }

    /*                                                                                  */
    pWextState->roamProfile.bWPSAssociation = FALSE;

    if (NULL != wlan_hdd_get_wps_ie_ptr(pWextState->roamProfile.pAddIEAssoc,
        pWextState->roamProfile.nAddIEAssocLength))
        pWextState->roamProfile.bWPSAssociation = TRUE;


    //                                                  
    sme_SetDHCPTillPowerActiveFlag(WLAN_HDD_GET_HAL_CTX(pAdapter), TRUE);

    pWextState->roamProfile.csrPersona = pAdapter->device_mode; 
    (WLAN_HDD_GET_CTX(pAdapter))->isAmpAllowed = VOS_FALSE;
    status = sme_RoamConnect( hHal,pAdapter->sessionId, &(pWextState->roamProfile),&roamId);
    pRoamProfile->ChannelInfo.ChannelList = NULL; 
    pRoamProfile->ChannelInfo.numOfChannels = 0;

    EXIT();
    return status;
}
/*                                                                            
  
                          
                                                         
   
                                            
                                                      
                                                 
                                                    
                                               
  
                                                                            */
int iw_get_essid(struct net_device *dev, 
                       struct iw_request_info *info,
                       struct iw_point *dwrq, char *extra)
{
   hdd_adapter_t *pAdapter = WLAN_HDD_GET_PRIV_PTR(dev);
   hdd_wext_state_t *wextBuf = WLAN_HDD_GET_WEXT_STATE_PTR(pAdapter); 
   hdd_station_ctx_t *pHddStaCtx = WLAN_HDD_GET_STATION_CTX_PTR(pAdapter);
   ENTER();

   if((pHddStaCtx->conn_info.connState == eConnectionState_Associated &&
     wextBuf->roamProfile.SSIDs.SSIDList->SSID.length > 0) ||
      ((pHddStaCtx->conn_info.connState == eConnectionState_IbssConnected ||
        pHddStaCtx->conn_info.connState == eConnectionState_IbssDisconnected) &&
        wextBuf->roamProfile.SSIDs.SSIDList->SSID.length > 0))
   {
       dwrq->length = pHddStaCtx->conn_info.SSID.SSID.length;
       memcpy(extra, pHddStaCtx->conn_info.SSID.SSID.ssId, dwrq->length);
       dwrq->flags = 1;
   } else {
       memset(extra, 0, dwrq->length);
       dwrq->length = 0;
       dwrq->flags = 0;
   }
   EXIT();
   return 0;
}
/*                                                                            
  
                         
                                                                     
   
                                            
                                                      
                                                 
                                                    
                                               
  
                                                                            */
int iw_set_auth(struct net_device *dev,struct iw_request_info *info,
                        union iwreq_data *wrqu,char *extra)
{
   hdd_adapter_t *pAdapter = WLAN_HDD_GET_PRIV_PTR(dev);
   hdd_wext_state_t *pWextState = WLAN_HDD_GET_WEXT_STATE_PTR(pAdapter); 
   hdd_station_ctx_t *pHddStaCtx = WLAN_HDD_GET_STATION_CTX_PTR(pAdapter);
   tCsrRoamProfile *pRoamProfile = &pWextState->roamProfile;
   eCsrEncryptionType mcEncryptionType;   
   eCsrEncryptionType ucEncryptionType;
   
   ENTER();
   switch(wrqu->param.flags & IW_AUTH_INDEX)
   {
      case IW_AUTH_WPA_VERSION:
        
         pWextState->wpaVersion = wrqu->param.value;
       
         break;
   
   case IW_AUTH_CIPHER_PAIRWISE:
   {
      if(wrqu->param.value & IW_AUTH_CIPHER_NONE) {            
         ucEncryptionType = eCSR_ENCRYPT_TYPE_NONE;
      }           
      else if(wrqu->param.value & IW_AUTH_CIPHER_TKIP) {
         ucEncryptionType = eCSR_ENCRYPT_TYPE_TKIP;
      }            
      else if(wrqu->param.value & IW_AUTH_CIPHER_CCMP) {
         ucEncryptionType = eCSR_ENCRYPT_TYPE_AES;
      }    
            
     else if(wrqu->param.value & IW_AUTH_CIPHER_WEP40) {
           
         if( (IW_AUTH_KEY_MGMT_802_1X 
                     == (pWextState->authKeyMgmt & IW_AUTH_KEY_MGMT_802_1X)  ) 
                 && (eCSR_AUTH_TYPE_OPEN_SYSTEM == pHddStaCtx->conn_info.authType) )
                /*               */
             ucEncryptionType = eCSR_ENCRYPT_TYPE_WEP40;     
         else
                /*              */
             ucEncryptionType = eCSR_ENCRYPT_TYPE_WEP40_STATICKEY;              
      }      
      else if(wrqu->param.value & IW_AUTH_CIPHER_WEP104) {
           
         if( ( IW_AUTH_KEY_MGMT_802_1X 
                     == (pWextState->authKeyMgmt & IW_AUTH_KEY_MGMT_802_1X) ) 
                 && (eCSR_AUTH_TYPE_OPEN_SYSTEM == pHddStaCtx->conn_info.authType))
                  /*               */
            ucEncryptionType = eCSR_ENCRYPT_TYPE_WEP104;
         else
                /*              */
            ucEncryptionType = eCSR_ENCRYPT_TYPE_WEP104_STATICKEY;
               
         }
         else {
           
               hddLog(LOGW, "%s value %d UNKNOWN IW_AUTH_CIPHER",
                      __FUNCTION__, wrqu->param.value); 
               return -EINVAL;
         }
       
         pRoamProfile->EncryptionType.numEntries = 1;
         pRoamProfile->EncryptionType.encryptionType[0] = ucEncryptionType;
      }     
      break;
      case IW_AUTH_CIPHER_GROUP:
      {            
          if(wrqu->param.value & IW_AUTH_CIPHER_NONE) {
            mcEncryptionType = eCSR_ENCRYPT_TYPE_NONE;
      }
        
      else if(wrqu->param.value & IW_AUTH_CIPHER_TKIP) {
             mcEncryptionType = eCSR_ENCRYPT_TYPE_TKIP;
      }
        
      else if(wrqu->param.value & IW_AUTH_CIPHER_CCMP) {              
              mcEncryptionType = eCSR_ENCRYPT_TYPE_AES;
      }
        
      else if(wrqu->param.value & IW_AUTH_CIPHER_WEP40) {
           
         if( ( IW_AUTH_KEY_MGMT_802_1X 
                     == (pWextState->authKeyMgmt & IW_AUTH_KEY_MGMT_802_1X )) 
                 && (eCSR_AUTH_TYPE_OPEN_SYSTEM == pHddStaCtx->conn_info.authType))  
                                            
            mcEncryptionType = eCSR_ENCRYPT_TYPE_WEP40;
            
         else            
               mcEncryptionType = eCSR_ENCRYPT_TYPE_WEP40_STATICKEY; 
      }
        
      else if(wrqu->param.value & IW_AUTH_CIPHER_WEP104) 
      {     
             /*                                            */
         if( ( IW_AUTH_KEY_MGMT_802_1X 
                     == (pWextState->authKeyMgmt & IW_AUTH_KEY_MGMT_802_1X)) 
                 && (eCSR_AUTH_TYPE_OPEN_SYSTEM == pHddStaCtx->conn_info.authType))
         {
            mcEncryptionType = eCSR_ENCRYPT_TYPE_WEP104;
         }
         else
         {
            mcEncryptionType = eCSR_ENCRYPT_TYPE_WEP104_STATICKEY;
         }
      }
      else {
           
          hddLog(LOGW, "%s value %d UNKNOWN IW_AUTH_CIPHER",
                 __FUNCTION__, wrqu->param.value); 
          return -EINVAL;
       }
              
         pRoamProfile->mcEncryptionType.numEntries = 1;
         pRoamProfile->mcEncryptionType.encryptionType[0] = mcEncryptionType;
      }
      break;

      case IW_AUTH_80211_AUTH_ALG:
      {
           /*                                                             
                                        */
          if( wrqu->param.value & IW_AUTH_ALG_OPEN_SYSTEM)    
             pHddStaCtx->conn_info.authType = eCSR_AUTH_TYPE_OPEN_SYSTEM;
          
          else if(wrqu->param.value & IW_AUTH_ALG_SHARED_KEY)
             pHddStaCtx->conn_info.authType = eCSR_AUTH_TYPE_SHARED_KEY;

          else if(wrqu->param.value & IW_AUTH_ALG_LEAP)
            /*             */
             pHddStaCtx->conn_info.authType = eCSR_AUTH_TYPE_OPEN_SYSTEM;
          pWextState->roamProfile.AuthType.authType[0] = pHddStaCtx->conn_info.authType;
      }
      break;

      case IW_AUTH_KEY_MGMT:
      {
#ifdef FEATURE_WLAN_CCX
#define IW_AUTH_KEY_MGMT_CCKM       8  /*                               */
         /*                        */
         if ( wrqu->param.value & IW_AUTH_KEY_MGMT_CCKM) {
            //                                                                                           
            hddLog(VOS_TRACE_LEVEL_INFO,"%s: CCKM AKM Set %d\n", __FUNCTION__, wrqu->param.value);
            /*                                 */ 
            /*                                                           
                                                       
             */
            pWextState->authKeyMgmt |= IW_AUTH_KEY_MGMT_CCKM;
            /*                                */
            //                                                  
            pWextState->isCCXConnection = eANI_BOOLEAN_TRUE;
            //                                                                               
            pWextState->collectedAuthType = eCSR_AUTH_TYPE_CCKM_RSN;
         } else if ( wrqu->param.value & IW_AUTH_KEY_MGMT_PSK) {
            /*                       */
            pWextState->authKeyMgmt |= IW_AUTH_KEY_MGMT_PSK;
            //                                            
            //                                                                               
            pWextState->collectedAuthType = eCSR_AUTH_TYPE_RSN;
         } else if (!( wrqu->param.value & IW_AUTH_KEY_MGMT_802_1X)) {
            pWextState->collectedAuthType = eCSR_AUTH_TYPE_NONE; //                       
            /*                              */
            pWextState->authKeyMgmt = wrqu->param.value;
         } else { //                                   
            /*                       */
            pWextState->authKeyMgmt |= IW_AUTH_KEY_MGMT_802_1X;
            //                                            
            //                                                                               
            pWextState->collectedAuthType = eCSR_AUTH_TYPE_RSN;
         }
#else
         /*                       */
         pWextState->authKeyMgmt = wrqu->param.value;
#endif /*                  */
      }
      break;

      case IW_AUTH_TKIP_COUNTERMEASURES:
      {
         if(wrqu->param.value) {
            hddLog(VOS_TRACE_LEVEL_INFO_HIGH,
                   "Counter Measure started %d", wrqu->param.value);
            pWextState->mTKIPCounterMeasures = TKIP_COUNTER_MEASURE_STARTED;
         }
         else {
            hddLog(VOS_TRACE_LEVEL_INFO_HIGH,
                   "Counter Measure stopped=%d", wrqu->param.value);
            pWextState->mTKIPCounterMeasures = TKIP_COUNTER_MEASURE_STOPED;
         }
      }
      break;
      case IW_AUTH_DROP_UNENCRYPTED:
      case IW_AUTH_WPA_ENABLED:
      case IW_AUTH_RX_UNENCRYPTED_EAPOL:
      case IW_AUTH_ROAMING_CONTROL:
      case IW_AUTH_PRIVACY_INVOKED:
         
      default:
         
         hddLog(LOGW, "%s called with unsupported auth type %d", __FUNCTION__,
               wrqu->param.flags & IW_AUTH_INDEX);
      break;
   }
   
   EXIT();
   return 0;
}
/*                                                                            
 
                         
                                                             
   
                                            
                                                      
                                                 
                                                    
                                               
  
                                                                            */
int iw_get_auth(struct net_device *dev,struct iw_request_info *info,
                         union iwreq_data *wrqu,char *extra)
{
    hdd_adapter_t* pAdapter = WLAN_HDD_GET_PRIV_PTR(dev);
    hdd_wext_state_t *pWextState= WLAN_HDD_GET_WEXT_STATE_PTR(pAdapter); 
    tCsrRoamProfile *pRoamProfile = &pWextState->roamProfile;
    ENTER();
    switch(pRoamProfile->negotiatedAuthType)
    {
        case eCSR_AUTH_TYPE_WPA_NONE:
            wrqu->param.flags = IW_AUTH_WPA_VERSION;
            wrqu->param.value =  IW_AUTH_WPA_VERSION_DISABLED;
            break;
        case eCSR_AUTH_TYPE_WPA:
            wrqu->param.flags = IW_AUTH_WPA_VERSION;
            wrqu->param.value = IW_AUTH_WPA_VERSION_WPA;
            break;
#ifdef WLAN_FEATURE_VOWIFI_11R
        case eCSR_AUTH_TYPE_FT_RSN:
#endif
        case eCSR_AUTH_TYPE_RSN:
            wrqu->param.flags = IW_AUTH_WPA_VERSION;
            wrqu->param.value =  IW_AUTH_WPA_VERSION_WPA2;
            break;
         case eCSR_AUTH_TYPE_OPEN_SYSTEM:
             wrqu->param.value = IW_AUTH_ALG_OPEN_SYSTEM;
             break;
         case eCSR_AUTH_TYPE_SHARED_KEY:
             wrqu->param.value =  IW_AUTH_ALG_SHARED_KEY;
             break;
         case eCSR_AUTH_TYPE_UNKNOWN:
             hddLog(LOG1,"%s called with unknown auth type", __FUNCTION__);
             wrqu->param.value =  IW_AUTH_ALG_OPEN_SYSTEM;
             break;
         case eCSR_AUTH_TYPE_AUTOSWITCH:
             wrqu->param.value =  IW_AUTH_ALG_OPEN_SYSTEM;
             break;
         case eCSR_AUTH_TYPE_WPA_PSK:
             hddLog(LOG1,"%s called with unknown auth type", __FUNCTION__);
             wrqu->param.value = IW_AUTH_ALG_OPEN_SYSTEM;
             return -EIO;
#ifdef WLAN_FEATURE_VOWIFI_11R
         case eCSR_AUTH_TYPE_FT_RSN_PSK:
#endif
         case eCSR_AUTH_TYPE_RSN_PSK:
             hddLog(LOG1,"%s called with unknown auth type", __FUNCTION__);
             wrqu->param.value = IW_AUTH_ALG_OPEN_SYSTEM;
             return -EIO;
         default:
             hddLog(LOG1,"%s called with unknown auth type", __FUNCTION__);
             wrqu->param.value = IW_AUTH_ALG_OPEN_SYSTEM;
             return -EIO;
    }
    if(((wrqu->param.flags & IW_AUTH_INDEX) == IW_AUTH_CIPHER_PAIRWISE))
    {
        switch(pRoamProfile->negotiatedUCEncryptionType)
        {
            case eCSR_ENCRYPT_TYPE_NONE:
                wrqu->param.value = IW_AUTH_CIPHER_NONE;
                break;
            case eCSR_ENCRYPT_TYPE_WEP40:
            case eCSR_ENCRYPT_TYPE_WEP40_STATICKEY:
                wrqu->param.value = IW_AUTH_CIPHER_WEP40;
                break;
            case eCSR_ENCRYPT_TYPE_TKIP:
                wrqu->param.value = IW_AUTH_CIPHER_TKIP;
                break;
            case eCSR_ENCRYPT_TYPE_WEP104:
            case eCSR_ENCRYPT_TYPE_WEP104_STATICKEY:
                wrqu->param.value = IW_AUTH_CIPHER_WEP104;
                break;
            case eCSR_ENCRYPT_TYPE_AES:
                wrqu->param.value = IW_AUTH_CIPHER_CCMP;
                break;
            default:
                hddLog(LOG1, "%s called with unknown auth type", __FUNCTION__);
                return -EIO;
        }
   }

    if(((wrqu->param.flags & IW_AUTH_INDEX) == IW_AUTH_CIPHER_GROUP)) 
    {
        switch(pRoamProfile->negotiatedMCEncryptionType)
        {
        case eCSR_ENCRYPT_TYPE_NONE:
            wrqu->param.value = IW_AUTH_CIPHER_NONE;
            break;
        case eCSR_ENCRYPT_TYPE_WEP40:
        case eCSR_ENCRYPT_TYPE_WEP40_STATICKEY:
            wrqu->param.value = IW_AUTH_CIPHER_WEP40;
            break;
        case eCSR_ENCRYPT_TYPE_TKIP:
            wrqu->param.value = IW_AUTH_CIPHER_TKIP;
            break;
         case eCSR_ENCRYPT_TYPE_WEP104:
         case eCSR_ENCRYPT_TYPE_WEP104_STATICKEY:
             wrqu->param.value = IW_AUTH_CIPHER_WEP104;
             break;
         case eCSR_ENCRYPT_TYPE_AES:
             wrqu->param.value = IW_AUTH_CIPHER_CCMP;
             break;
         default:
             hddLog(LOG1, "%s called with unknown auth type", __FUNCTION__);
            return -EIO;
       }
   }

    hddLog(LOG1, "%s called with auth type %d",
           __FUNCTION__, pRoamProfile->AuthType.authType[0]);
    EXIT();
    return 0;
}
/*                                                                            
  
                               
                                                                 
                                                                       
   
                                            
                                                      
                                                 
                                                    
                                               
  
                                                                            */
int iw_set_ap_address(struct net_device *dev,
        struct iw_request_info *info,
        union iwreq_data *wrqu, char *extra)
{
    hdd_station_ctx_t *pHddStaCtx = WLAN_HDD_GET_STATION_CTX_PTR(WLAN_HDD_GET_PRIV_PTR(dev));
    v_U8_t  *pMacAddress=NULL;
    ENTER();
    pMacAddress = (v_U8_t*) wrqu->ap_addr.sa_data;
    VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_INFO, "%02x:%02x:%02x:%02x:%02x:%02x",pMacAddress[0],pMacAddress[1],
          pMacAddress[2],pMacAddress[3],pMacAddress[4],pMacAddress[5]);
    vos_mem_copy( pHddStaCtx->conn_info.bssId, pMacAddress, sizeof( tCsrBssid ));
    EXIT();
   
    return 0;
}
/*                                                                            
  
                               
                                                        
                                            
                                                      
                                                 
                                                    
                                               
  
                                                                            */
int iw_get_ap_address(struct net_device *dev,
                             struct iw_request_info *info,
                             union iwreq_data *wrqu, char *extra)
{
    //                                                     
    hdd_station_ctx_t *pHddStaCtx = WLAN_HDD_GET_STATION_CTX_PTR(WLAN_HDD_GET_PRIV_PTR(dev));

    ENTER();

    if ((pHddStaCtx->conn_info.connState == eConnectionState_Associated) ||
        (eConnectionState_IbssConnected == pHddStaCtx->conn_info.connState))
    {
        memcpy(wrqu->ap_addr.sa_data,pHddStaCtx->conn_info.bssId,sizeof(wrqu->ap_addr.sa_data));
    }
    else
    {
        memset(wrqu->ap_addr.sa_data,0,sizeof(wrqu->ap_addr.sa_data));
    }
    EXIT();
    return 0;
}


/*                                                                            

                                            
                                                 
                                             
                   

                                                                            */
void hdd_ResetCountryCodeAfterDisAssoc(hdd_adapter_t *pAdapter)
{
    hdd_context_t* pHddCtx = (hdd_context_t*)pAdapter->pHddCtx;
    tSmeConfigParams smeConfig;
    eHalStatus status = eHAL_STATUS_SUCCESS;
    tANI_U8 defaultCountryCode[3] = SME_INVALID_COUNTRY_CODE;
    tANI_U8 currentCountryCode[3] = SME_INVALID_COUNTRY_CODE;

    sme_GetConfigParam(pHddCtx->hHal, &smeConfig);

    VOS_TRACE( VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_INFO,
            "%s: 11d is %s\n",__func__,
            smeConfig.csrConfig.Is11dSupportEnabled ? "Enabled" : "Disabled");
    /*                                            
    */
    if (smeConfig.csrConfig.Is11dSupportEnabled)
    {
        sme_GetDefaultCountryCodeFrmNv(pHddCtx->hHal, &defaultCountryCode[0]);
        sme_GetCurrentCountryCode(pHddCtx->hHal, &currentCountryCode[0]);

        VOS_TRACE( VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_INFO,
                "%s: Default country code: %c%c%c, Current Country code: %c%c%c \n",
                __func__,
                defaultCountryCode[0], defaultCountryCode[1], defaultCountryCode[2],
                currentCountryCode[0], currentCountryCode[1], currentCountryCode[2]);
        /*                                                 
                                                                
         */
        if ((defaultCountryCode[0] != currentCountryCode[0]) ||
                (defaultCountryCode[1] != currentCountryCode[1]) ||
                (defaultCountryCode[2] != currentCountryCode[2]))
        {
            VOS_TRACE( VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_INFO,
                    "%s: Disconnected from the AP/Assoc failed and "
                    "resetting the country code to default\n",__func__);
            /*                                             */
            status = (int)sme_ChangeCountryCode(pHddCtx->hHal, NULL,
                    &defaultCountryCode[0], pAdapter,
                    pHddCtx->pvosContext
                    );
            if( 0 != status )
            {
                VOS_TRACE( VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR,
                        "%s: failed to Reset the Country Code\n",__func__);
            }
        }
    }
}

