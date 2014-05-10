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


                       W L A N _ Q C T _ WDA _ DS . C

  OVERVIEW:

  This software unit holds the implementation of WLAN Data Abtraction APIs
  for the WLAN Transport Layer.

  DEPENDENCIES:

  Are listed for each API below.


  Copyright (c) 2010-2011 QUALCOMM Incorporated.
  All Rights Reserved.
  Qualcomm Confidential and Proprietary
===========================================================================*/

/*                                                                           

                                           


                                                                       
                                                                


                                   


                                         
                                                                      
                                                                        
                                                                          
                                                                            */

#include "wlan_qct_wda.h"
#include "wlan_qct_tl.h"
#include "wlan_qct_tli.h"
#include "tlDebug.h"
#if defined( FEATURE_WLAN_NON_INTEGRATED_SOC )
#include "wlan_bal_misc.h"
#endif

#define WDA_DS_DXE_RES_COUNT   WDA_TLI_MIN_RES_DATA + 20

#define VOS_TO_WPAL_PKT(_vos_pkt) ((wpt_packet*)_vos_pkt)

#if defined( FEATURE_WLAN_INTEGRATED_SOC )
#define WDA_HI_FLOW_MASK 0xF0
#define WDA_LO_FLOW_MASK 0x0F

static v_VOID_t 
WDA_DS_TxCompleteCB
(
 v_PVOID_t pvosGCtx, 
 v_PVOID_t pFrameDataBuff
);
#endif

#if defined( FEATURE_WLAN_NON_INTEGRATED_SOC )
/*                                                                          
                                     

             
                                                                  

              
                                                   

            

     
                                     
                                           

      
                                                   
                                                                
                                                       
                                 

              
              

              

                                                                            */
void
WDA_DS_PrepareBDHeader
(
  vos_pkt_t*      vosDataBuff,
  v_PVOID_t*      ppvBDHeader,
  v_MACADDR_t*    pvDestMacAddr,
  v_U8_t          ucDisableFrmXtl,
  VOS_STATUS*     pvosStatus,
  v_U16_t*        pusPktLen,
  v_U8_t          ucQosEnabled,
  v_U8_t          ucWDSEnabled,
  v_U8_t          extraHeadSpace
)
{
  v_U8_t      ucHeaderOffset;
  v_U8_t      ucHeaderLen;
#ifndef WLAN_SOFTAP_FEATURE
  v_PVOID_t   pvPeekData;
#endif
  v_U8_t      ucBDHeaderLen = WLANTL_BD_HEADER_LEN(ucDisableFrmXtl);

  /*                                                                       */
  /*                                                                         
                                
                                                   
                                                                            */
  vos_pkt_get_packet_length( vosDataBuff, pusPktLen);

  if ( WLANTL_MAC_HEADER_LEN(ucDisableFrmXtl) > *pusPktLen )
  {
    TLLOGE(VOS_TRACE( VOS_MODULE_ID_TL, VOS_TRACE_LEVEL_ERROR,
               "WLAN TL: Length of the packet smaller than expected network"
               " header %d", *pusPktLen ));

    *pvosStatus = VOS_STATUS_E_INVAL;
    return;
  }

  vos_pkt_reserve_head( vosDataBuff, ppvBDHeader,
                        ucBDHeaderLen );
  if ( NULL == *ppvBDHeader )
  {
    TLLOGE(VOS_TRACE( VOS_MODULE_ID_TL, VOS_TRACE_LEVEL_ERROR,
                "WLAN TL:VOSS packet corrupted on Attach BD header"));
    *pvosStatus = VOS_STATUS_E_INVAL;
    return;
  }

  /*                                                                       
                       
                                                                          */
#ifdef WLAN_SOFTAP_FEATURE
  {
   v_SIZE_t usMacAddrSize = VOS_MAC_ADDR_SIZE;
   *pvosStatus = vos_pkt_extract_data( vosDataBuff,
                                     ucBDHeaderLen +
                                     WLANTL_MAC_ADDR_ALIGN(ucDisableFrmXtl),
                                     (v_PVOID_t)pvDestMacAddr,
                                     &usMacAddrSize );
  }
#else
  *pvosStatus = vos_pkt_peek_data( vosDataBuff,
                                     ucBDHeaderLen +
                                     WLANTL_MAC_ADDR_ALIGN(ucDisableFrmXtl),
                                     (v_PVOID_t)&pvPeekData,
                                     VOS_MAC_ADDR_SIZE );

  /*      */
  vos_copy_macaddr(pvDestMacAddr, (v_MACADDR_t*)pvPeekData);
#endif
  if ( VOS_STATUS_SUCCESS != *pvosStatus )
  {
     TLLOGE(VOS_TRACE( VOS_MODULE_ID_TL, VOS_TRACE_LEVEL_ERROR,
                "WLAN TL:Failed while attempting to extract MAC Addr %d",
                *pvosStatus));
  }
  else
  {
    /*                                                                     
                              
                                  
                                    
                              
                                                              
                                                                        */
    ucHeaderOffset = ucBDHeaderLen;
    ucHeaderLen    = WLANTL_MAC_HEADER_LEN(ucDisableFrmXtl);

    if ( 0 != ucDisableFrmXtl )
    {
      if ( 0 != ucQosEnabled )
      {
        ucHeaderLen += WLANTL_802_11_HEADER_QOS_CTL;
      }

      //                                                  
      if ( ucWDSEnabled != 0 )
      {
        //                                     
        ucHeaderLen    += WLANTL_802_11_HEADER_ADDR4_LEN;
      }
      if ( extraHeadSpace != 0 )
      {
        //                                                                   
        *pusPktLen = *pusPktLen - extraHeadSpace;
      }
    }

    WLANHAL_TX_BD_SET_MPDU_HEADER_LEN( *ppvBDHeader, ucHeaderLen);
    WLANHAL_TX_BD_SET_MPDU_HEADER_OFFSET( *ppvBDHeader, ucHeaderOffset);
    WLANHAL_TX_BD_SET_MPDU_DATA_OFFSET( *ppvBDHeader,
                                          ucHeaderOffset + ucHeaderLen + extraHeadSpace);
    WLANHAL_TX_BD_SET_MPDU_LEN( *ppvBDHeader, *pusPktLen);

    TLLOG2(VOS_TRACE( VOS_MODULE_ID_TL, VOS_TRACE_LEVEL_INFO_HIGH,
                "WLAN TL: VALUES ARE HLen=%x Hoff=%x doff=%x len=%x ex=%d",
                ucHeaderLen, ucHeaderOffset, 
                (ucHeaderOffset + ucHeaderLen + extraHeadSpace), 
                *pusPktLen, extraHeadSpace));
  }/*                    */

}/*                        */
#endif /*                                 */

#ifdef WLAN_PERF
/*                                                                          
                                        

              
                                                          

                                                                            
                                                                              
                                                                            
                                                                           
                                                                             
                                                         

                                                                         
                                                    

               
     
             

     
                               
                                      
                                           
                                         
    
      
                                   
                                                                          
   
              
                 

               
  
                                                                            */
void WDA_TLI_FastHwFwdDataFrame
(
  v_PVOID_t     pvosGCtx,
  vos_pkt_t*    vosDataBuff,
  VOS_STATUS*   pvosStatus,
  v_U32_t*       puFastFwdOK,
  WLANTL_MetaInfoType*  pMetaInfo,
  WLAN_STADescType*  pStaInfo
)
{
#if defined( FEATURE_WLAN_INTEGRATED_SOC )
  /*                                           
                         
   */
#else /*                             */
   v_PVOID_t   pvPeekData;
   v_U8_t      ucDxEBDWLANHeaderLen = WLANTL_BD_HEADER_LEN(0) + sizeof(WLANBAL_sDXEHeaderType); 
   v_U8_t      ucIsUnicast;
   WLANBAL_sDXEHeaderType  *pDxEHeader;
   v_PVOID_t   pvBDHeader;
   v_PVOID_t   pucBuffPtr;
   v_U16_t      usPktLen;

   /*                                                                       
                         
                                                                          */

   vos_pkt_get_packet_length( vosDataBuff, &usPktLen);

   /*                                                                       
                       
                                                                           */
   *pvosStatus = vos_pkt_peek_data( vosDataBuff, 
                                 WLANTL_MAC_ADDR_ALIGN(0), 
                                 (v_PVOID_t)&pvPeekData, 
                                 VOS_MAC_ADDR_SIZE );

   if ( VOS_STATUS_SUCCESS != *pvosStatus ) 
   {
      TLLOGE(VOS_TRACE( VOS_MODULE_ID_TL, VOS_TRACE_LEVEL_ERROR,
                 "WLAN TL:Failed while attempting to extract MAC Addr %d", 
                 *pvosStatus));
      *pvosStatus = VOS_STATUS_E_INVAL;
      return;
   }

   /*                                                                       
                                                         
                                                                           */

   vos_pkt_reserve_head( vosDataBuff, &pucBuffPtr, 
                        ucDxEBDWLANHeaderLen );
   if ( NULL == pucBuffPtr )
   {
       TLLOGE(VOS_TRACE( VOS_MODULE_ID_TL, VOS_TRACE_LEVEL_ERROR,
                    "WLAN TL:No enough space in VOSS packet %p for DxE/BD/WLAN header", vosDataBuff));
      *pvosStatus = VOS_STATUS_E_INVAL;
       return;
   }
   pDxEHeader = (WLANBAL_sDXEHeaderType  *)pucBuffPtr;
   pvBDHeader = (v_PVOID_t) &pDxEHeader[1];

   /*                                 
                                                                          
                                                                      
                                                                       
                                         
    */
   if(pStaInfo->wSTAType == WLAN_STA_IBSS)
      ucIsUnicast = !(((tANI_U8 *)pvPeekData)[0] & 0x01);
   else
      ucIsUnicast = 1;
 
   *puFastFwdOK = (v_U32_t) WLANHAL_TxBdFastFwd(pvosGCtx, pvPeekData, pMetaInfo->ucTID, ucIsUnicast, pvBDHeader, usPktLen );
    
   /*                                                                       */
   if(! *puFastFwdOK){
       vos_pkt_trim_head(vosDataBuff, ucDxEBDWLANHeaderLen);
   }else{
      /*                                                                                       
       */
      v_U32_t uPacketSize = WLANTL_BD_HEADER_LEN(0) + usPktLen;
      vos_pkt_set_user_data_ptr( vosDataBuff, VOS_PKT_USER_DATA_ID_BAL, 
                       (v_PVOID_t)uPacketSize);
      pDxEHeader->size  = SWAP_ENDIAN_UINT32(uPacketSize);
   }

   *pvosStatus = VOS_STATUS_SUCCESS;
   return;
#endif /*                             */
}
#endif /*         */

/*                                                                          
                             

              
                                                                         
                                      


                                                                        
                                           

       
                                                                        
                                                                      
                                    

               
     
             

     
                               
                                                                           
                                               
                                               
                                                                           
                                                                    
                                                                       
                                                              
      
                                                        
                                                                   
   
              
                                                          
                                              

               
  
                                                                            */
VOS_STATUS WDA_DS_Register 
( 
  v_PVOID_t                 pvosGCtx, 
  WDA_DS_TxCompleteCallback pfnTxCompleteCallback,
  WDA_DS_RxPacketCallback   pfnRxPacketCallback, 
  WDA_DS_TxPacketCallback   pfnTxPacketCallback,
  WDA_DS_ResourceCB         pfnResourceCB,
  v_U32_t                   uResTheshold,
  v_PVOID_t                 pCallbackContext,
  v_U32_t                   *uAvailableTxBuf
)
{
#if defined( FEATURE_WLAN_INTEGRATED_SOC )
  tWDA_CbContext      *wdaContext = NULL;
  WDI_Status          wdiStatus;

  VOS_TRACE( VOS_MODULE_ID_TL, VOS_TRACE_LEVEL_INFO_HIGH,
             "WLAN WDA: WDA_DS_Register" );

  /*                                                                        
                
                                                                           */
  if ( ( NULL == pvosGCtx ) ||
       ( NULL == pfnTxPacketCallback ) ||
       ( NULL == pfnTxCompleteCallback ) ||
       ( NULL == pfnRxPacketCallback ) ||
       ( NULL == pfnResourceCB) )
  {
    VOS_TRACE( VOS_MODULE_ID_WDA, VOS_TRACE_LEVEL_ERROR,
               "WLAN WDA:Invalid pointers on WDA_DS_Register" );
    return VOS_STATUS_E_FAULT;
  }

  /*                                                                        
                       
                                                                           */
  wdaContext = (tWDA_CbContext *)vos_get_context( VOS_MODULE_ID_WDA, pvosGCtx );
  if ( NULL == wdaContext )
  {
    VOS_TRACE( VOS_MODULE_ID_WDA, VOS_TRACE_LEVEL_ERROR,
               "WLAN WDA:Invalid wda context pointer "
               "from pvosGCtx on WDA_DS_Register" );
    return VOS_STATUS_E_FAULT;
  }

  /*                                                                        
                                               
                                                                          */
  VOS_TRACE( VOS_MODULE_ID_TL, VOS_TRACE_LEVEL_INFO_HIGH, "WDA: WDA_DS_Register");

  wdaContext->pfnTxResourceCB       = pfnResourceCB;
  wdaContext->pfnTxCompleteCallback = pfnTxCompleteCallback;

  wdiStatus = WDI_DS_Register( wdaContext->pWdiContext,
                               (WDI_DS_TxCompleteCallback)WDA_DS_TxCompleteCB,
                               (WDI_DS_RxPacketCallback)pfnRxPacketCallback,
                               WDA_DS_TxFlowControlCallback,
                               pvosGCtx );

  if ( WDI_STATUS_SUCCESS != wdiStatus )
  {
    VOS_TRACE( VOS_MODULE_ID_TL, VOS_TRACE_LEVEL_ERROR,
               "WLAN TL:TL failed to register with DAL, Err: %d", wdiStatus );
    return VOS_STATUS_E_FAILURE;
  }
   
  /*                                                                           
                                                            */
  *uAvailableTxBuf = WDA_DS_DXE_RES_COUNT; 

  return VOS_STATUS_SUCCESS;
#else /*                             */
  VOS_STATUS          vosStatus;
  WLANBAL_TlRegType   tlReg;

  VOS_TRACE( VOS_MODULE_ID_TL, VOS_TRACE_LEVEL_INFO_HIGH,
             "WLAN WDA: WDA_DS_Register" );

  /*                                                                        
                
                                                                           */
  if ( ( NULL == pvosGCtx ) ||
       ( NULL == pfnTxPacketCallback ) ||
       ( NULL == pfnTxCompleteCallback ) ||
       ( NULL == pfnRxPacketCallback ) ||
       ( NULL == pfnResourceCB ) )
  {
    VOS_TRACE( VOS_MODULE_ID_TL, VOS_TRACE_LEVEL_ERROR,
               "WLAN WDA:Invalid pointers on WDA_DS_Register" );
    return VOS_STATUS_E_FAULT;
  }

  /*                                                                        
                                               
                                                                          */
  tlReg.receiveFrameCB = pfnRxPacketCallback;
  tlReg.getTXFrameCB   = pfnTxPacketCallback;
  tlReg.txCompleteCB   = pfnTxCompleteCallback;
  tlReg.txResourceCB   = pfnResourceCB;
  tlReg.txResourceThreashold = uResTheshold;
  tlReg.tlUsrData      = pvosGCtx;

  vosStatus = WLANBAL_RegTlCbFunctions( pvosGCtx, &tlReg );

  if ( VOS_STATUS_SUCCESS != vosStatus )
  {
    VOS_TRACE( VOS_MODULE_ID_TL, VOS_TRACE_LEVEL_ERROR, 
               "WLAN TL: TL failed to register with BAL, Err: %d", vosStatus );
    return vosStatus;
  }

  /*                                                                        
                                     
                                                                          */
  vosStatus = WLANBAL_GetTxResources( pvosGCtx, uAvailableTxBuf );

  if ( VOS_STATUS_SUCCESS != vosStatus )
  {
    VOS_TRACE( VOS_MODULE_ID_TL, VOS_TRACE_LEVEL_ERROR,
               "WLAN TL:TL failed to get resources from BAL, Err: %d",
               vosStatus );
    return vosStatus;
  }

  return vosStatus;
#endif
}

/*                                                                          
                              

              
                                               

                                                                      
                     

                                                                 

               
     
             

     
                               
   
              
                                                          
                                              

               
  
                                                                            */
VOS_STATUS
WDA_DS_StartXmit
(
  v_PVOID_t pvosGCtx
)
{
#if defined( FEATURE_WLAN_INTEGRATED_SOC )
  vos_msg_t                    sMessage;
/*                                                                         */

  if ( NULL == pvosGCtx )
  {
    VOS_TRACE( VOS_MODULE_ID_TL, VOS_TRACE_LEVEL_ERROR,
               "WLAN WDA:Invalid pointers on WDA_DS_StartXmit" );
    return VOS_STATUS_E_FAULT;
  }

  /*                      */
  VOS_TRACE( VOS_MODULE_ID_TL, VOS_TRACE_LEVEL_INFO_HIGH,
             "Serializing WDA TX Start Xmit event" );

  vos_mem_zero( &sMessage, sizeof(vos_msg_t) );

  sMessage.bodyptr = NULL;
  sMessage.type    = WDA_DS_TX_START_XMIT;

  return vos_tx_mq_serialize(VOS_MQ_ID_TL, &sMessage);
#else  /*                             */
  return WLANBAL_StartXmit( pvosGCtx );
#endif /*                             */
}


/*                                                                          
                              

              
                                                                     

               
     
             

     
                                                            
                                                           
   
              
                                  

               
  
                                                                            */
VOS_STATUS
WDA_DS_FinishULA
(
 void (*callbackRoutine) (void *callbackContext),
 void  *callbackContext
)
{
  vos_msg_t                    sMessage;
/*                                                                         */

  /*                      */
  VOS_TRACE( VOS_MODULE_ID_TL, VOS_TRACE_LEVEL_INFO_HIGH,
             "Serializing WDA_DS_FinishULA event" );

  vos_mem_zero( &sMessage, sizeof(vos_msg_t) );

  sMessage.bodyval  = (v_U32_t)callbackContext;
  sMessage.bodyptr  = callbackRoutine;
  sMessage.type     = WDA_DS_FINISH_ULA;

  return vos_tx_mq_serialize(VOS_MQ_ID_TL, &sMessage);
}/*                */

/*                                                                          
                                       

             
                                          
    
                                                                           
                            
    
              

            

     
                                
                                    
                                                  
                                           
                                    
                                    
                                                                         
                                  
                                                                  
                              
                        
          
             
             
        

      
                                  

              
                                                          
                                              

              

                                                                            */
VOS_STATUS
WDA_DS_BuildTxPacketInfo
(
  v_PVOID_t       pvosGCtx,
  vos_pkt_t*      vosDataBuff,
  v_MACADDR_t*    pvDestMacAddr,
  v_U8_t          ucDisableFrmXtl,
  v_U16_t*        pusPktLen,
  v_U8_t          ucQosEnabled,
  v_U8_t          ucWDSEnabled,
  v_U8_t          extraHeadSpace,
  v_U8_t          typeSubtype,
  v_PVOID_t       pAddr2,
  v_U8_t          uTid,
  v_U8_t          txFlag,
  v_U32_t         timeStamp,
  v_U8_t          ucIsEapol,
  v_U8_t          ucUP
)
{
#if defined( FEATURE_WLAN_INTEGRATED_SOC )
  VOS_STATUS             vosStatus;
  WDI_DS_TxMetaInfoType* pTxMetaInfo = NULL;
  v_SIZE_t               usMacAddrSize;
  /*                                                                       */

  /*                                                                        
                
                            
                                                                           */
  if ( ( NULL == pvosGCtx ) || ( NULL == vosDataBuff ) || ( NULL == pvDestMacAddr ) )
  {
    VOS_TRACE( VOS_MODULE_ID_WDA, VOS_TRACE_LEVEL_ERROR,
               "WDA:Invalid parameter sent on WDA_DS_BuildTxPacketInfo" );
    return VOS_STATUS_E_FAULT;
  }

  /*                                                                        
                                                
                                                                           */
  pTxMetaInfo = WDI_DS_ExtractTxMetaData( VOS_TO_WPAL_PKT(vosDataBuff)  );
  if ( NULL == pTxMetaInfo )
  {
    VOS_TRACE( VOS_MODULE_ID_TL, VOS_TRACE_LEVEL_ERROR,
        "WLAN TL:Invalid RxMetaInfo pointer from PAL packet on WLANTL_RxFrames");
    VOS_ASSERT( 0 );
    return VOS_STATUS_E_FAULT;
  }

  /*                  */
  vos_mem_zero( pTxMetaInfo, sizeof( WDI_DS_TxMetaInfoType ) );

  /*                       */
  pTxMetaInfo->txFlags    = txFlag;
  pTxMetaInfo->qosEnabled = ucQosEnabled;
  pTxMetaInfo->fenableWDS = ucWDSEnabled;
  pTxMetaInfo->ac         = ucUP;
  pTxMetaInfo->fUP        = uTid;
  pTxMetaInfo->isEapol    = ucIsEapol;
  pTxMetaInfo->fdisableFrmXlt = ucDisableFrmXtl;
  pTxMetaInfo->frmType     = ( ( typeSubtype & 0x30 ) >> 4 );
  pTxMetaInfo->typeSubtype = typeSubtype;

  /*                               */
  vos_pkt_get_packet_length( vosDataBuff, pusPktLen);
  pTxMetaInfo->fPktlen = *pusPktLen;

  //            
  usMacAddrSize = VOS_MAC_ADDR_SIZE;
  vosStatus = vos_pkt_extract_data( vosDataBuff,
                    WLANTL_MAC_ADDR_ALIGN( ucDisableFrmXtl ),
                    (v_PVOID_t)pvDestMacAddr,
                    &usMacAddrSize );
  if ( VOS_STATUS_SUCCESS != vosStatus )
  {
    VOS_TRACE( VOS_MODULE_ID_TL, VOS_TRACE_LEVEL_ERROR,
               "WDA:Failed while attempting to extract MAC Addr %d",
                vosStatus );
    VOS_ASSERT( 0 );
    return VOS_STATUS_E_FAULT;
  }

  VOS_ASSERT(usMacAddrSize == VOS_MAC_ADDR_SIZE);

  vos_copy_macaddr( (v_MACADDR_t*)pTxMetaInfo->fSTAMACAddress, pvDestMacAddr );

  //      
  vos_copy_macaddr( (v_MACADDR_t*)pTxMetaInfo->addr2MACAddress, pAddr2 );

  /*                                  */
  VOS_TRACE( VOS_MODULE_ID_TL, VOS_TRACE_LEVEL_INFO_LOW,
             "WLAN TL: Dump TX meta info: "
             "txFlags:%d, qosEnabled:%d, ac:%d, "
             "isEapol:%d, fdisableFrmXlt:%d" "frmType%d",
             pTxMetaInfo->txFlags, ucQosEnabled, pTxMetaInfo->ac,
             pTxMetaInfo->isEapol, pTxMetaInfo->fdisableFrmXlt, pTxMetaInfo->frmType );

  return VOS_STATUS_SUCCESS;
#else  /*                             */
  VOS_STATUS   vosStatus;
  v_PVOID_t    pvBDHeader;

  WDA_DS_PrepareBDHeader( vosDataBuff, &pvBDHeader, pvDestMacAddr, ucDisableFrmXtl,
                  &vosStatus, pusPktLen, ucQosEnabled, ucWDSEnabled, extraHeadSpace );

  if ( VOS_STATUS_SUCCESS != vosStatus )
  {
    VOS_TRACE( VOS_MODULE_ID_TL, VOS_TRACE_LEVEL_ERROR,
               "WLAN TL:Failed while attempting to prepare BD %d", vosStatus );
    return vosStatus;
  }

  vosStatus = WLANHAL_FillTxBd( pvosGCtx, typeSubtype, pvDestMacAddr, pAddr2,
                    &uTid, ucDisableFrmXtl, pvBDHeader, txFlag, timeStamp );

  if ( VOS_STATUS_SUCCESS != vosStatus )
  {
    VOS_TRACE( VOS_MODULE_ID_TL, VOS_TRACE_LEVEL_ERROR,
               "WLAN TL:Failed while attempting to fill BD %d", vosStatus );
    return vosStatus;
  }

  return VOS_STATUS_SUCCESS;

#endif /*                             */
}


/*                                                                          
                                      

             
                                                    
                                       
    
              

            

     
                                    

              
                                                          
                                              

              

                                                                            */
VOS_STATUS
WDA_DS_TrimRxPacketInfo
( 
  vos_pkt_t *vosDataBuff
)
{
#if defined( FEATURE_WLAN_INTEGRATED_SOC )
  /*                
                */

  return VOS_STATUS_SUCCESS;
#else  /*                             */
  VOS_STATUS vosStatus = VOS_STATUS_SUCCESS;
  v_U16_t  usPktLen;
  v_U8_t   ucMPDUHOffset;
  v_U16_t  usMPDUDOffset;
  v_U16_t  usMPDULen;
  v_U8_t   ucMPDUHLen = 0;
  v_U8_t   aucBDHeader[WLANHAL_RX_BD_HEADER_SIZE];

  vos_pkt_pop_head( vosDataBuff, aucBDHeader, WLANHAL_RX_BD_HEADER_SIZE);

  ucMPDUHOffset = (v_U8_t)WLANHAL_RX_BD_GET_MPDU_H_OFFSET(aucBDHeader);
  usMPDUDOffset = (v_U16_t)WLANHAL_RX_BD_GET_MPDU_D_OFFSET(aucBDHeader);
  usMPDULen     = (v_U16_t)WLANHAL_RX_BD_GET_MPDU_LEN(aucBDHeader);
  ucMPDUHLen    = (v_U8_t)WLANHAL_RX_BD_GET_MPDU_H_LEN(aucBDHeader);
  
  VOS_TRACE( VOS_MODULE_ID_TL, VOS_TRACE_LEVEL_INFO_HIGH,
       "WLAN TL:BD header processing data: HO %d DO %d Len %d HLen %d"
       " Tid %d BD %d",
       ucMPDUHOffset, usMPDUDOffset, usMPDULen, ucMPDUHLen,
       WLANHAL_RX_BD_HEADER_SIZE );

  vos_pkt_get_packet_length( vosDataBuff, &usPktLen);

  if (( ucMPDUHOffset >= WLANHAL_RX_BD_HEADER_SIZE ) &&
      ( usMPDUDOffset >  ucMPDUHOffset ) &&
      ( usMPDULen     >= ucMPDUHLen ) &&
      ( usPktLen >= usMPDULen ))
  {
    if((ucMPDUHOffset - WLANHAL_RX_BD_HEADER_SIZE) > 0)
    {
      vos_pkt_trim_head( vosDataBuff, ucMPDUHOffset - WLANHAL_RX_BD_HEADER_SIZE);
    }
    else
    {
      /*                
                    */
    }
    vosStatus = VOS_STATUS_SUCCESS;
  }
  else
  {
    vosStatus = VOS_STATUS_E_FAILURE;
  }

  return vosStatus;
#endif /*                             */
}



/*                                                                          
                                      

             
                                                      
    
                                                
    
              

            

     
                                    

                                                  
                                                                       
                                                       
      
                                        

              
                                                          
                                              

              

                                                                            */
VOS_STATUS
WDA_DS_PeekRxPacketInfo
(
  vos_pkt_t *vosDataBuff,
  v_PVOID_t *ppRxHeader,
  v_BOOL_t  bSwap
)
{
#if defined( FEATURE_WLAN_INTEGRATED_SOC )
  /*                                                                        
                
                                                                           */
  if (  NULL == vosDataBuff )
  {
    VOS_TRACE( VOS_MODULE_ID_WDA, VOS_TRACE_LEVEL_ERROR,
               "WDA:Invalid parameter sent on WDA_DS_PeekRxPacketInfo" );
    return VOS_STATUS_E_FAULT;
  }

  *ppRxHeader = WDI_DS_ExtractRxMetaData( (wpt_packet*)vosDataBuff );

  if ( NULL == *ppRxHeader )
  {
    VOS_TRACE( VOS_MODULE_ID_WDA, VOS_TRACE_LEVEL_ERROR,
               "WDA:Failed to get RX meta data from WDI" );
     return VOS_STATUS_E_FAILURE;
  }
     
  return VOS_STATUS_SUCCESS;
#else  /*                             */
  VOS_STATUS vosStatus;

  vosStatus = vos_pkt_peek_data( vosDataBuff, 0, (v_PVOID_t)ppRxHeader,
                                   WLANHAL_RX_BD_HEADER_SIZE);

  if ( ( VOS_STATUS_SUCCESS != vosStatus ) || ( NULL == (v_PVOID_t)ppRxHeader ) )
  {
    VOS_TRACE( VOS_MODULE_ID_TL, VOS_TRACE_LEVEL_ERROR,
               "WDA :Cannot extract BD header" );
    return VOS_STATUS_E_FAILURE;
  }

  if ( VOS_TRUE == bSwap )
  {
    WLANHAL_SwapRxBd( *ppRxHeader );
  }

  return VOS_STATUS_SUCCESS;
#endif /*                             */
}



/*                                                                          
                                         

             
                                                                    
                                                                        
                                                                    
             

                                                   
                                                                    

              

            

     
                                
                                    
                                                      

      
                                

              
                                                          
                                              

              

                                                                            */
VOS_STATUS
WDA_DS_GetFrameTypeSubType
(
  v_PVOID_t pvosGCtx,
  vos_pkt_t *vosDataBuff,
  v_PVOID_t pRxHeader,
  v_U8_t    *ucTypeSubtype
)
{
#if defined( FEATURE_WLAN_INTEGRATED_SOC )
  /*                                                                        
                
                                                                           */
  if ( NULL == pRxHeader )
  {
    VOS_TRACE( VOS_MODULE_ID_WDA, VOS_TRACE_LEVEL_ERROR,
               "WDA:Invalid parameter sent on WDA_DS_GetFrameTypeSubType" );
    return VOS_STATUS_E_FAULT;
  }

  *ucTypeSubtype = ( WDA_GET_RX_TYPE( pRxHeader ) << 4 ) | WDA_GET_RX_SUBTYPE( pRxHeader );

  return VOS_STATUS_SUCCESS;
#else  /*                             */
  v_PVOID_t           pvBDHeader = pRxHeader;
  v_U16_t             usFrmCtrl  = 0; 
  v_U8_t              ucFrmType;
  v_SIZE_t            usFrmCtrlSize = sizeof(usFrmCtrl); 
  VOS_STATUS          vosStatus;

  /*                                                                     
                                                              
                                 
                                                                       */
  vosStatus = vos_pkt_extract_data( vosDataBuff, 
                       ( 0 == WLANHAL_RX_BD_GET_FT(pvBDHeader) ) ?
                       WLANHAL_RX_BD_GET_MPDU_H_OFFSET(pvBDHeader):
                       WLANHAL_RX_BD_HEADER_SIZE,
                       &usFrmCtrl, &usFrmCtrlSize );

  if (( VOS_STATUS_SUCCESS != vosStatus ) || 
      ( sizeof(usFrmCtrl) != usFrmCtrlSize ))
  {
    VOS_TRACE( VOS_MODULE_ID_TL, VOS_TRACE_LEVEL_ERROR,
               "WLAN TL: Cannot extract Frame Control Field" );
    return VOS_STATUS_E_FAILURE;
  }


  ucFrmType = (v_U8_t)WLANHAL_RxBD_GetFrameTypeSubType( pvBDHeader, 
                                                        usFrmCtrl);
  WLANHAL_RX_BD_SET_TYPE_SUBTYPE(pvBDHeader, ucFrmType);

  *ucTypeSubtype = ucFrmType;
  
  return VOS_STATUS_SUCCESS;
#endif /*                             */
}


/*                                                                          
                                  

             
                                                                           
                     

                                       

              

            

     
                                
                                      

      

              
                                                          
                                              

              

                                                                            */
VOS_STATUS
WDA_DS_RxAmsduBdFix
(
  v_PVOID_t pvosGCtx,
  v_PVOID_t pvBDHeader
)
{
#if defined( FEATURE_WLAN_INTEGRATED_SOC )
  /*                      */
  return VOS_STATUS_SUCCESS;
#else  /*                             */
  /*                 
                                                              
                              */
  WLANHAL_RxAmsduBdFix(pvosGCtx, pvBDHeader);
  return VOS_STATUS_SUCCESS;
#endif /*                             */
}

/*                                                                          
                             

             
             

                                                                            
                                

              

            

     
                                    

      
                         

              
                                                          
                                              

              

                                                                            */
VOS_STATUS
WDA_DS_GetRssi
(
  v_PVOID_t pvosGCtx,
  v_S7_t*   puRssi
)
{
#if defined( FEATURE_WLAN_INTEGRATED_SOC )
  VOS_TRACE( VOS_MODULE_ID_TL, VOS_TRACE_LEVEL_ERROR,
         "WDA:halPS_GetRssi no longer supported. Need replacement");

  *puRssi = -30;

  return VOS_STATUS_SUCCESS;
#else  /*                             */
  halPS_GetRssi(vos_get_context(VOS_MODULE_ID_SME, pvosGCtx), puRssi);
  return VOS_STATUS_SUCCESS;
#endif /*                             */
}

/*                                                                          
                                    

             
                                                                   
                                                                

                                                                 
    
              

            

     
                                    
   
      
                                                 

              
                                                          
                                              

              

                                                                            */
VOS_STATUS
WDA_DS_GetTxResources
( 
  v_PVOID_t pvosGCtx,
  v_U32_t*  puResCount
)
{
#if defined( FEATURE_WLAN_INTEGRATED_SOC )
  /*                                                                    */
  /*                                   */
  *puResCount = WDA_TLI_BD_PDU_RESERVE_THRESHOLD + 50;
  return VOS_STATUS_SUCCESS;
#else  /*                             */
  return WLANBAL_GetTxResources( pvosGCtx, puResCount );
#endif /*                             */
}



/*                                                                          
                                      

             
                                                        

              

            

     
                                                      

      

              
                  

              

                                                                            */
v_U64_t
WDA_DS_GetReplayCounter
(
  v_PVOID_t pRxHeader
)
{
#if defined( FEATURE_WLAN_INTEGRATED_SOC )
  return WDA_GET_RX_REPLAY_COUNT( pRxHeader );
#else  /*                             */
   v_U8_t *pucRxBDHeader = pRxHeader;

/*                                            
                                 
                  
                  
                  
                  
                  

                  
                   */

#ifdef ANI_BIG_BYTE_ENDIAN
    v_U64_t ullcurrentReplayCounter = 0;
    /*                                              */
    ullcurrentReplayCounter = WLANHAL_RX_BD_GET_PMICMD_20TO23(pucRxBDHeader); 
    ullcurrentReplayCounter <<= 16;
    ullcurrentReplayCounter |= (( WLANHAL_RX_BD_GET_PMICMD_24TO25(pucRxBDHeader) & 0xFFFF0000) >> 16);
    return ullcurrentReplayCounter;
#else
    v_U64_t ullcurrentReplayCounter = 0;
    /*                                              */
    ullcurrentReplayCounter = (WLANHAL_RX_BD_GET_PMICMD_24TO25(pucRxBDHeader) & 0x0000FFFF); 
    ullcurrentReplayCounter <<= 32; 
    ullcurrentReplayCounter |= WLANHAL_RX_BD_GET_PMICMD_20TO23(pucRxBDHeader); 
    return ullcurrentReplayCounter;
#endif

#endif /*                             */
}

#if defined( FEATURE_WLAN_INTEGRATED_SOC )
/*                                                                          
                              

             
                                                                   
                         

                                                                    

      
                                                         

              

            

     
                                

      

              
                                                          
                                              

              

                                                                            */

VOS_STATUS
WDA_DS_TxFrames
( 
  v_PVOID_t pvosGCtx 
)
{
  VOS_STATUS vosStatus;
  vos_pkt_t  *pTxMgmtChain = NULL;
  vos_pkt_t  *pTxDataChain = NULL;
  vos_pkt_t  *pTxPacket = NULL;
  v_BOOL_t   bUrgent;
  wpt_uint32  ucTxResReq;
  WDI_Status wdiStatus;
  tWDA_CbContext *wdaContext = NULL;
  v_U32_t     uMgmtAvailRes;
  v_U32_t     uDataAvailRes;
  WLANTL_TxCompCBType  pfnTxComp = NULL;

  wdaContext = (tWDA_CbContext *)vos_get_context(VOS_MODULE_ID_WDA, pvosGCtx);
  if ( NULL == wdaContext )
  {
    VOS_TRACE( VOS_MODULE_ID_WDA, VOS_TRACE_LEVEL_ERROR,
               "WDA:Invalid wda context pointer from pvosGCtx on WDA_DS_TxFrames" );
    return VOS_STATUS_E_FAULT;
  }

  /*                                                                         
                                                                         
                                                               
                                                                           */
  /*       */
  uMgmtAvailRes = WDI_GetAvailableResCount(wdaContext->pWdiContext, 
                                           WDI_MGMT_POOL_ID);
  
  ucTxResReq = WLANTL_GetFrames( pvosGCtx, 
                              &pTxMgmtChain, 
                               uMgmtAvailRes, 
                              (wdaContext->uTxFlowMask & WDA_HI_FLOW_MASK),
                              &bUrgent );

  //                                                                   
  //                                                                       
  //                                                                  
  //                                                    
  vosStatus = VOS_STATUS_SUCCESS;
      
  while ( NULL != pTxMgmtChain )
  {
    /*                                       */
    pTxPacket = pTxMgmtChain;
    vosStatus = vos_pkt_walk_packet_chain( pTxMgmtChain, &pTxMgmtChain, VOS_TRUE );

    if( (VOS_STATUS_SUCCESS != vosStatus) &&
        (VOS_STATUS_E_EMPTY != vosStatus) )
    {
      VOS_TRACE( VOS_MODULE_ID_WDA, VOS_TRACE_LEVEL_ERROR,
                 "WDA Walking packet chain returned status : %d", vosStatus );
      VOS_ASSERT( 0 );
      vosStatus = VOS_STATUS_E_FAILURE;
      break;
    }

    if ( VOS_STATUS_E_EMPTY == vosStatus )
    {
       vosStatus = VOS_STATUS_SUCCESS;
    }

    wdiStatus = WDI_DS_TxPacket( wdaContext->pWdiContext, 
                                 (wpt_packet*)pTxPacket, 
                                 0 /*      */ );
    if ( WDI_STATUS_SUCCESS != wdiStatus )
    {
      VOS_TRACE( VOS_MODULE_ID_TL, VOS_TRACE_LEVEL_ERROR,
                   "WDA : Pushing a packet to WDI failed.");
      VOS_ASSERT( wdiStatus != WDI_STATUS_E_NOT_ALLOWED );
      //                               
      vos_pkt_get_user_data_ptr(pTxPacket, VOS_PKT_USER_DATA_ID_TL, (void **)&pfnTxComp);
      if(pfnTxComp)
      {
         pfnTxComp(pvosGCtx, pTxPacket, VOS_STATUS_E_FAILURE);
      }
    }

  };

  /*       */
  uDataAvailRes = WDI_GetAvailableResCount(wdaContext->pWdiContext, 
                                           WDI_DATA_POOL_ID);

  ucTxResReq = WLANTL_GetFrames( pvosGCtx, 
                              &pTxDataChain, 
                              /*                    */ uDataAvailRes, 
                              (wdaContext->uTxFlowMask & WDA_LO_FLOW_MASK),
                              &bUrgent );

  //                                                                   
  //                                                                       
  //                                                                  
  //                                                    
  vosStatus = VOS_STATUS_SUCCESS;

  while ( NULL != pTxDataChain )
  {
    /*                                       */
    pTxPacket = pTxDataChain;
    vosStatus = vos_pkt_walk_packet_chain( pTxDataChain, &pTxDataChain, VOS_TRUE );

    if( (VOS_STATUS_SUCCESS != vosStatus) &&
        (VOS_STATUS_E_EMPTY != vosStatus) )
    {
      VOS_TRACE( VOS_MODULE_ID_WDA, VOS_TRACE_LEVEL_ERROR,
                 "WDA Walking packet chain returned status : %d", vosStatus );
      VOS_ASSERT( 0 );
      vosStatus = VOS_STATUS_E_FAILURE;
      break;
    }

    if ( VOS_STATUS_E_EMPTY == vosStatus )
    {
       vosStatus = VOS_STATUS_SUCCESS;
    }

    wdiStatus = WDI_DS_TxPacket( wdaContext->pWdiContext, 
                                 (wpt_packet*)pTxPacket, 
                                 0 /*      */ );
    if ( WDI_STATUS_SUCCESS != wdiStatus )
    {
      VOS_TRACE( VOS_MODULE_ID_TL, VOS_TRACE_LEVEL_ERROR,
                   "WDA : Pushing a packet to WDI failed.");
      VOS_ASSERT( wdiStatus != WDI_STATUS_E_NOT_ALLOWED );
      //                               
      vos_pkt_get_user_data_ptr(pTxPacket, VOS_PKT_USER_DATA_ID_TL, (void **)&pfnTxComp);
      if(pfnTxComp)
      {
         pfnTxComp(pvosGCtx, pTxPacket, VOS_STATUS_E_FAILURE);
      }
    }

  };

  WDI_DS_TxComplete(wdaContext->pWdiContext, ucTxResReq);

  return vosStatus;
}
#endif /*                             */

#if defined( FEATURE_WLAN_INTEGRATED_SOC )
/*                                                                          
                                           

             
                                      

              

            

     
                                
                                                     
                                                  

      

              

              

                                                                            */
v_VOID_t
WDA_DS_TxFlowControlCallback
(
   v_PVOID_t pvosGCtx,
   v_U8_t    ucFlowMask
)
{
   tWDA_CbContext* wdaContext = NULL;
    v_U8_t          ucOldFlowMask;
   /*                                                                       */

   /*                                                                        
                 
                                                                            */
   if ( NULL == pvosGCtx )
   {
      VOS_TRACE( VOS_MODULE_ID_WDA, VOS_TRACE_LEVEL_ERROR,
                 "WDA: Invalid parameter sent on WDA_DS_TxFlowControlCallback" );
      return;
   }

   wdaContext = (tWDA_CbContext *)vos_get_context( VOS_MODULE_ID_WDA, pvosGCtx );
   if ( NULL == wdaContext )
   {
      VOS_TRACE( VOS_MODULE_ID_WDA, VOS_TRACE_LEVEL_ERROR,
                 "WDA: Invalid context on WDA_DS_TxFlowControlCallback" );
      return;
   }

   /*                          
                                                                                      
                                                              
   */


   /*               */
   ucOldFlowMask           = wdaContext->uTxFlowMask; 
   wdaContext->uTxFlowMask = ucFlowMask;

   /*                                              
    
                                                      
    
                         
                          
                          
                          
                          
    
                                                    
    
                                                        
                                                        
                                                              
   */

   if ( ucFlowMask > ucOldFlowMask  )
   {
     WDA_DS_StartXmit(pvosGCtx);
   }

}
#endif /*                             */

/*                                                                          
                                   

             
                                     

              

            

     
                                

      
                                                     
                                                  

              
                                                          
                                              

              

                                                                            */
VOS_STATUS
WDA_DS_GetTxFlowMask
(
 v_PVOID_t pvosGCtx,
 v_U8_t*   puFlowMask
)
{
#if defined( FEATURE_WLAN_INTEGRATED_SOC )
   tWDA_CbContext* wdaContext = NULL;
   /*                                                                       */

   /*                                                                        
                 
                                                                            */
   if ( ( NULL == pvosGCtx ) || ( NULL == puFlowMask ) )
   {
      VOS_TRACE( VOS_MODULE_ID_WDA, VOS_TRACE_LEVEL_ERROR,
                 "WDA: Invalid parameter sent on WDA_DS_GetTxFlowMask" );
      return VOS_STATUS_E_INVAL;
   }

   wdaContext = (tWDA_CbContext *)vos_get_context( VOS_MODULE_ID_WDA, pvosGCtx );
   if ( NULL == wdaContext )
   {
      VOS_TRACE( VOS_MODULE_ID_WDA, VOS_TRACE_LEVEL_ERROR,
                 "WDA: Invalid context on WDA_DS_GetTxFlowMask" );
      return VOS_STATUS_E_INVAL;
   }

   *puFlowMask = wdaContext->uTxFlowMask;

   return VOS_STATUS_SUCCESS;
#else
   *puFlowMask = WDA_TXFLOWMASK;
   return VOS_STATUS_SUCCESS;
#endif  /*                             */
}

#if defined( FEATURE_WLAN_INTEGRATED_SOC )
v_VOID_t 
WDA_DS_TxCompleteCB
(
 v_PVOID_t pvosGCtx, 
 v_PVOID_t pFrameDataBuff
)
{
  tWDA_CbContext*        wdaContext = NULL;
  WDI_DS_TxMetaInfoType* pTxMetadata;
  VOS_STATUS             vosStatus;
  /*                                                                       */

  /*                                                                        
                
                                                                            */

  if ( ( NULL == pvosGCtx ) || ( NULL == pFrameDataBuff ) )
  {
    VOS_TRACE( VOS_MODULE_ID_WDA, VOS_TRACE_LEVEL_ERROR,
               "WDA: Invalid parameter sent on WDA_DS_TxCompleteCB" );
    return;
  }

  wdaContext = (tWDA_CbContext *)vos_get_context( VOS_MODULE_ID_WDA, pvosGCtx );
  if ( NULL == wdaContext )
  {
    VOS_TRACE( VOS_MODULE_ID_WDA, VOS_TRACE_LEVEL_ERROR,
               "WDA: Invalid context on WDA_DS_TxCompleteCB" );
    return;
  }

  //                                 
  pTxMetadata = WDI_DS_ExtractTxMetaData( (wpt_packet*)pFrameDataBuff );
  
  if ( eWLAN_PAL_STATUS_SUCCESS == pTxMetadata->txCompleteStatus )
    vosStatus = VOS_STATUS_SUCCESS;
  else 
    vosStatus = VOS_STATUS_E_FAILURE;

  wdaContext->pfnTxCompleteCallback( pvosGCtx, pFrameDataBuff, vosStatus );
}
#endif  /*                             */
