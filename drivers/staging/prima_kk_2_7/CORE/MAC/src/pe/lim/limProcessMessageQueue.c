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
/*
                                                        
                                                         
                                    
                                  
                          
            
                                                         
                                                                       
  
 */
#include "palTypes.h"
#include "wniApi.h"
#include "wlan_qct_wdi_ds.h"
#include "wlan_qct_pal_packet.h"
#include "wlan_qct_wda.h"

#include "wniCfgSta.h"
#include "cfgApi.h"
#include "sirCommon.h"
#include "utilsApi.h"
#include "limTypes.h"
#include "limUtils.h"
#include "limAssocUtils.h"
#include "limPropExtsUtils.h"

#include "limAdmitControl.h"
#include "pmmApi.h"
#include "limIbssPeerMgmt.h"
#include "schApi.h"
#include "limSession.h"
#include "limSendMessages.h"

#if defined WLAN_FEATURE_VOWIFI
#include "rrmApi.h"
#endif
#if defined FEATURE_WLAN_CCX
#include "ccxApi.h"
#endif

#if defined WLAN_FEATURE_VOWIFI_11R
#include "limFT.h"
#endif

#ifdef WMM_APSD
#include "wmmApsd.h"
#endif

#include "vos_types.h"
#include "vos_packet.h"
#include "vos_memory.h"

void limLogSessionStates(tpAniSirGlobal pMac);

/*                                                               
                  
                                                                                           
                            
                             
                                     
            
                                                               */

tANI_U8 static
defMsgDecision(tpAniSirGlobal pMac, tpSirMsgQ  limMsg)
{


/*                                  */
  if(pMac->lim.gLimSmeState == eLIM_SME_OFFLINE_STATE)
  {
      //                               
      if (limDeferMsg(pMac, limMsg) != TX_SUCCESS)
      {
          PELOGW(limLog(pMac, LOGW, FL("Unable to Defer message(0x%X) %s limSmeState %d (prev sme state %d) sysRole %d mlm state %d (prev mlm state %d)"),
                   limMsg->type, limMsgStr(limMsg->type), pMac->lim.gLimSmeState,  pMac->lim.gLimPrevSmeState,
                   pMac->lim.gLimSystemRole,  pMac->lim.gLimMlmState,  pMac->lim.gLimPrevMlmState);)
          limLogSessionStates(pMac);
          limHandleDeferMsgError(pMac, limMsg);
      }
      return true;
  }

  //                                                                         
  if((!limIsSystemInScanState(pMac)) && (true != GET_LIM_PROCESS_DEFD_MESGS(pMac)) &&
      !pMac->lim.gLimSystemInScanLearnMode)
  {
      if((limMsg->type != WDA_ADD_BSS_RSP) &&
        (limMsg->type != WDA_DELETE_BSS_RSP) &&
        (limMsg->type != WDA_ADD_STA_RSP) &&
        (limMsg->type != WDA_ADD_STA_SELF_RSP) &&
        (limMsg->type != WDA_DEL_STA_SELF_RSP) &&
        (limMsg->type != WDA_DELETE_STA_RSP)&&
        (limMsg->type != WDA_SET_BSSKEY_RSP)&&
        (limMsg->type != WDA_SET_STAKEY_RSP)&&
        (limMsg->type != WDA_SET_STA_BCASTKEY_RSP) &&
        (limMsg->type != eWNI_SME_START_REQ) &&
        (limMsg->type != WDA_AGGR_QOS_RSP) &&
        (limMsg->type != WDA_REMOVE_BSSKEY_RSP) &&
        (limMsg->type != WDA_REMOVE_STAKEY_RSP) &&
        (limMsg->type != WDA_SET_MIMOPS_RSP)&&
        (limMsg->type != WDA_ADDBA_RSP) &&
        (limMsg->type != WDA_ENTER_BMPS_RSP) &&
        (limMsg->type != WDA_EXIT_BMPS_RSP) &&
        (limMsg->type != WDA_ENTER_IMPS_RSP) &&
        (limMsg->type != WDA_EXIT_IMPS_RSP) &&
        (limMsg->type != WDA_ENTER_UAPSD_RSP) &&
        (limMsg->type != WDA_EXIT_UAPSD_RSP) &&
        (limMsg->type != WDA_WOWL_ENTER_RSP) &&
        (limMsg->type != WDA_WOWL_EXIT_RSP) &&
        (limMsg->type != WDA_SWITCH_CHANNEL_RSP) &&
        (limMsg->type != WDA_P2P_NOA_ATTR_IND) &&
        (limMsg->type != WDA_P2P_NOA_START_IND) &&
#ifdef FEATURE_OEM_DATA_SUPPORT
        (limMsg->type != WDA_START_OEM_DATA_RSP) &&
#endif
        (limMsg->type != WDA_ADD_TS_RSP))
    {
        PELOG1(limLog(pMac, LOG1, FL("Defer the current message %s , gLimProcessDefdMsgs is false and system is not in scan/learn mode"),
               limMsgStr(limMsg->type));)

        //                               
        if (limDeferMsg(pMac, limMsg) != TX_SUCCESS)
        {
            PELOGW(limLog(pMac, LOGW, FL("Unable to Defer message(0x%X) %s limSmeState %d (prev sme state %d) sysRole %d mlm state %d (prev mlm state %d)"),
                   limMsg->type, limMsgStr(limMsg->type), pMac->lim.gLimSmeState,  pMac->lim.gLimPrevSmeState,
                   pMac->lim.gLimSystemRole,  pMac->lim.gLimMlmState,  pMac->lim.gLimPrevMlmState);)
            limLogSessionStates(pMac);
            limHandleDeferMsgError(pMac, limMsg);

        }
        return true;
    }
  }
  return false;
}

/*
                        
                                             
                                                                                    
                                                                                                                         
                                                                                    
                                                                                                                           
                                                                                    
                           
                                                                         
                
*/
static void
__limHandleBeacon(tpAniSirGlobal pMac, tpSirMsgQ pMsg, tpPESession psessionEntry)
{
    /*                                 */
    tANI_U8 *pRxPacketInfo;
    limGetBDfromRxPacket(pMac, pMsg->bodyptr, (tANI_U32 **)&pRxPacketInfo);

    //                                                                       
    //                                             

    if(psessionEntry == NULL)
    {
        schBeaconProcess(pMac, pRxPacketInfo, NULL);
    }
    else if( (psessionEntry->limSmeState == eLIM_SME_LINK_EST_STATE) ||
                (psessionEntry->limSmeState == eLIM_SME_NORMAL_STATE))
    {
        schBeaconProcess(pMac, pRxPacketInfo, psessionEntry);
    }
     else
        limProcessBeaconFrame(pMac, pRxPacketInfo, psessionEntry);

        return;
}


//                  
void limProcessNormalHddMsg(tpAniSirGlobal pMac, tSirMsgQ *pLimMsg, tANI_U8 fRspReqd);

/* 
                
  
           
                                                         
                    
  
        
     
  
              
     
  
       
     
  
                                                 
                                                                   
               
 */

tANI_U32
limDeferMsg(tpAniSirGlobal pMac, tSirMsgQ *pMsg)
{
    tANI_U32 retCode = TX_SUCCESS;

    retCode = limWriteDeferredMsgQ(pMac, pMsg);

    if (retCode == TX_SUCCESS)
    {
        MTRACE(macTraceMsgRx(pMac, NO_SESSION, LIM_TRACE_MAKE_RXMSG(pMsg->type, LIM_MSG_DEFERRED));)
    }
    else
    {
        MTRACE(macTraceMsgRx(pMac, NO_SESSION, LIM_TRACE_MAKE_RXMSG(pMsg->type, LIM_MSG_DROPPED));)
    }

    return retCode;
} /*                       */



/* 
                               
  
           
                                                   
                                 
  
        
     
  
              
     
  
       
     
  
                                                    
                                     
                                                              
                                                                   
               
 */

static void
limHandleFramesInScanState(tpAniSirGlobal pMac, tpSirMsgQ limMsg, tANI_U8 *pRxPacketInfo, tANI_U8 *deferMsg, tpPESession psessionEntry)
{
    tSirMacFrameCtl  fc;
    tpSirMacMgmtHdr  pHdr;

    *deferMsg = false;
    pHdr = WDA_GET_RX_MAC_HEADER(pRxPacketInfo);
    fc = pHdr->fc;
    limLog( pMac, LOG2, FL("ProtVersion %d, Type %d, Subtype %d"),
            fc.protVer, fc.type, fc.subType );

    //                                                                      
    if ((fc.type == SIR_MAC_MGMT_FRAME) && (fc.subType == SIR_MAC_MGMT_BEACON))
    {
        if (psessionEntry == NULL)
            limProcessBeaconFrameNoSession(pMac, pRxPacketInfo);
        else 
            limProcessBeaconFrame(pMac, pRxPacketInfo,psessionEntry);
    }
    else if ((fc.type == SIR_MAC_MGMT_FRAME) && (fc.subType == SIR_MAC_MGMT_PROBE_RSP))
    {
        if (psessionEntry == NULL)
            limProcessProbeRspFrameNoSession(pMac, pRxPacketInfo);
        else
            limProcessProbeRspFrame(pMac, pRxPacketInfo,psessionEntry);
    }
    else if ((fc.type == SIR_MAC_MGMT_FRAME) && (fc.subType == SIR_MAC_MGMT_PROBE_REQ))
    {
      limProcessProbeReqFrame_multiple_BSS(pMac, pRxPacketInfo, psessionEntry);
    }
    else if ((fc.type == SIR_MAC_MGMT_FRAME) && (fc.subType == SIR_MAC_MGMT_ACTION))
    {
       limProcessActionFrameNoSession( pMac, pRxPacketInfo);
    }
    else
    {
        *deferMsg = true;
        return; 
    }
 
    limPktFree(pMac, HAL_TXRX_FRM_802_11_MGMT, pRxPacketInfo, (void *) limMsg->bodyptr);
    return;

} /*                                      */

/*                                                              
                                                          
                  
                                                       
                                                          
              
 
                                                                 */
static void limHandleUnknownA2IndexFrames(tpAniSirGlobal pMac, void *pRxPacketInfo,tpPESession psessionEntry)
{
      /*                                                       
                                        
                                                                           */
       /*                                 */
       //                                
       //                                                                                                          
    //                    
    //                                    
    if( psessionEntry->limSystemRole == eLIM_P2P_DEVICE_ROLE )
        limProcessActionFrameNoSession(pMac, (tANI_U8 *) pRxPacketInfo);

#ifdef FEATURE_WLAN_TDLS
    {
        tpSirMacDataHdr3a pMacHdr;
        pMacHdr = WDA_GET_RX_MPDUHEADER3A(pRxPacketInfo);

        if (limIsGroupAddr(pMacHdr->addr2))
        {
            PELOG2(limLog(pMac, LOG2, FL("Ignoring A2 Invalid Packet received for MC/BC:"));
                    limPrintMacAddr(pMac, pMacHdr->addr2, LOG2);)

                return;
        }
        /*                                                                           
                                                  */
        if (psessionEntry->limSystemRole == eLIM_STA_ROLE)
        {
            /*                                     */
            LIM_LOG_TDLS(VOS_TRACE(VOS_MODULE_ID_PE, VOS_TRACE_LEVEL_ERROR, \
                        ("limHandleUnknownA2IndexFrames: type=0x%x, subtype=0x%x"),pMacHdr->fc.type, pMacHdr->fc.subType));
            switch (pMacHdr->fc.type)
            {
                case SIR_MAC_MGMT_FRAME:
                    {
                        switch (pMacHdr->fc.subType)
                        {
                            case SIR_MAC_MGMT_ACTION:
                                {
                                    limProcessActionFrame(pMac, pRxPacketInfo, psessionEntry) ;
                                    break ;
                                }
                            default:
                                {
                                    break ;
                                }
                        }
                    }
                default:
                    {
                        break ;
                    }
            }
        }
    }
#endif


    return;
}

/* 
                                 
  
           
                                                                            
                                                                       
  
        
  
              
  
       
  
                                                    
                                                                     
                                                                        
               
 */
static tANI_BOOLEAN
limCheckMgmtRegisteredFrames(tpAniSirGlobal pMac, tANI_U8 *pBd,
                                        tpPESession psessionEntry)
{
    tSirMacFrameCtl  fc;
    tpSirMacMgmtHdr  pHdr;
    tANI_U8          *pBody;
    tpLimMgmtFrameRegistration pLimMgmtRegistration = NULL, pNext = NULL;
    tANI_U16 frameType;
    tANI_U16 framelen;
    tANI_U8 type,subType;
    tANI_BOOLEAN match = VOS_FALSE;
    VOS_STATUS vosStatus;

    pHdr = WDA_GET_RX_MAC_HEADER(pBd);
    fc = pHdr->fc;
    frameType = (fc.type << 2 ) | (fc.subType << 4);
    pBody = WDA_GET_RX_MPDU_DATA(pBd);
    framelen = WDA_GET_RX_PAYLOAD_LEN(pBd);

    vos_list_peek_front(&pMac->lim.gLimMgmtFrameRegistratinQueue,
                        (vos_list_node_t**)&pLimMgmtRegistration);

    while(pLimMgmtRegistration != NULL)
    {
        type = (pLimMgmtRegistration->frameType >> 2) & 0x03;
        subType = (pLimMgmtRegistration->frameType >> 4) & 0x0f;
        if ( (type == SIR_MAC_MGMT_FRAME) && (fc.type == SIR_MAC_MGMT_FRAME)
              && (subType == SIR_MAC_MGMT_RESERVED15) )
        {
            limLog( pMac, LOG3, 
                FL("rcvd frame match with SIR_MAC_MGMT_RESERVED15"));
            match = VOS_TRUE;
            break;
        }

        if (pLimMgmtRegistration->frameType == frameType)
        { 
            if (pLimMgmtRegistration->matchLen > 0)
            {   
                if (pLimMgmtRegistration->matchLen <= framelen)
                {
                    if (palEqualMemory(pMac, pLimMgmtRegistration->matchData,
                                       pBody, pLimMgmtRegistration->matchLen))
                    {
                         /*              */
                         match = VOS_TRUE;
                         break;
                    }
                }
            }
            else
            {
                /*              */   
                match = VOS_TRUE;
                break;
            }
        }
             
        vosStatus = 
          vos_list_peek_next ( &pMac->lim.gLimMgmtFrameRegistratinQueue, 
                              (vos_list_node_t*) pLimMgmtRegistration, 
                              (vos_list_node_t**) &pNext );
        pLimMgmtRegistration = pNext;
        pNext = NULL;
    }
   
    if (match)
    {
        limLog( pMac, LOG1, 
                FL("rcvd frame match with registered frame params"));

        /*                      */
        limSendSmeMgmtFrameInd( pMac, pHdr->fc.subType, (tANI_U8*)pHdr, 
                     WDA_GET_RX_PAYLOAD_LEN(pBd) + sizeof(tSirMacMgmtHdr), 
                     pLimMgmtRegistration->sessionId,
                     WDA_GET_RX_CH(pBd), psessionEntry, 0);
    
        if ( (type == SIR_MAC_MGMT_FRAME) && (fc.type == SIR_MAC_MGMT_FRAME)
              && (subType == SIR_MAC_MGMT_RESERVED15) )
        {
            //                                                              
            //                                                              
            match = VOS_FALSE;
        }
    }

    return match;
} /*                                         */


/* 
                         
  
           
                                                   
                   
  
        
     
  
              
     
  
       
     
  
                                                 
                                                                   
               
 */

static void 
limHandle80211Frames(tpAniSirGlobal pMac, tpSirMsgQ limMsg, tANI_U8 *pDeferMsg)
{
    tANI_U8          *pRxPacketInfo = NULL;
    tSirMacFrameCtl  fc;
    tpSirMacMgmtHdr    pHdr=NULL;
    tpPESession         psessionEntry=NULL;
    tANI_U8             sessionId;
    tAniBool            isFrmFt = FALSE;
    tANI_U16            fcOffset = WLANHAL_RX_BD_HEADER_SIZE;

    *pDeferMsg= false;
    limGetBDfromRxPacket(pMac, limMsg->bodyptr, (tANI_U32 **)&pRxPacketInfo);

    pHdr = WDA_GET_RX_MAC_HEADER(pRxPacketInfo);
    isFrmFt = WDA_GET_RX_FT_DONE(pRxPacketInfo);
    fcOffset = (v_U8_t)WDA_GET_RX_MPDU_HEADER_OFFSET(pRxPacketInfo);
    fc = pHdr->fc;

#ifdef WLAN_DUMP_MGMTFRAMES
    limLog( pMac, LOGE, FL("ProtVersion %d, Type %d, Subtype %d rateIndex=%d"),
            fc.protVer, fc.type, fc.subType,
            WDA_GET_RX_MAC_RATE_IDX(pRxPacketInfo));
    VOS_TRACE_HEX_DUMP(VOS_MODULE_ID_PE, VOS_TRACE_LEVEL_ERROR, pHdr,
                       WDA_GET_RX_MPDU_HEADER_LEN(pRxPacketInfo));
#endif

#ifdef WLAN_FEATURE_ROAM_SCAN_OFFLOAD
    if ( WDA_GET_ROAMCANDIDATEIND(pRxPacketInfo))
    {
        limLog( pMac, LOG2, FL("Notify SME with candidate ind"));
        //                              
        limSendSmeCandidateFoundInd(pMac, 0);
        goto end;
    }
    if (WDA_GET_OFFLOADSCANLEARN(pRxPacketInfo))
    {
        if (fc.subType == SIR_MAC_MGMT_BEACON)
        {
            limLog( pMac, LOG2, FL("Save this beacon in LFR cache"));
            __limHandleBeacon(pMac, limMsg, NULL);
        }
        else if (fc.subType == SIR_MAC_MGMT_PROBE_RSP)
        {
            limLog( pMac, LOG2, FL("Save this probe rsp in LFR cache"));
            limProcessProbeRspFrameNoSession(pMac, pRxPacketInfo);
        }
        else
        {
            limLog( pMac, LOGE, FL("Wrong frame Type %d, Subtype %d for LFR"),
                    fc.type, fc.subType);
        }
        goto end;
    }
#endif //                              
#ifdef FEATURE_WLAN_CCX
    if (fc.type == SIR_MAC_DATA_FRAME && isFrmFt) 
    {
#if 0 //                     
        tpSirMacDot3Hdr pDataFrmHdr;

        pDataFrmHdr = (tpSirMacDot3Hdr)((tANI_U8 *)pBD+ WLANHAL_RX_BD_GET_MPDU_H_OFFSET(pBD));
        if((psessionEntry = peFindSessionByBssid(pMac,pDataFrmHdr->sa,&sessionId))== NULL)
        {
            limLog( pMac, LOGE, FL("Session not found for Frm type %d, subtype %d, SA: "), fc.type, fc.subType);
            limPrintMacAddr(pMac, pDataFrmHdr->sa, LOGE);
            limPktFree(pMac, HAL_TXRX_FRM_802_11_MGMT, pBD, limMsg->bodyptr);
            return;
        }

        if (!psessionEntry->isCCXconnection)
        {
            limLog( pMac, LOGE, FL("LIM received Type %d, Subtype %d in Non CCX connection"),
                    fc.type, fc.subType);
            limPktFree(pMac, HAL_TXRX_FRM_802_11_MGMT, pBD, limMsg->bodyptr);
            return;
        }
        limLog( pMac, LOGE, FL("Processing IAPP Frm from SA:"));
        limPrintMacAddr(pMac, pDataFrmHdr->sa, LOGE);
#else
        printk("%s: Need to port handling of IAPP frames to PRIMA for CCX", __func__);
#endif


    } else
#endif
    /*                          */
    if((psessionEntry = peFindSessionByBssid(pMac,pHdr->bssId,&sessionId))== NULL)
    {
#ifdef WLAN_FEATURE_VOWIFI_11R
        if (fc.subType == SIR_MAC_MGMT_AUTH) 
        {
#ifdef WLAN_FEATURE_VOWIFI_11R_DEBUG
            limLog( pMac, LOG1, FL("ProtVersion %d, Type %d, Subtype %d rateIndex=%d"),
                    fc.protVer, fc.type, fc.subType, WDA_GET_RX_MAC_RATE_IDX(pRxPacketInfo));
            limPrintMacAddr(pMac, pHdr->bssId, LOG1);
#endif
            if (limProcessAuthFrameNoSession(pMac, pRxPacketInfo, limMsg->bodyptr) == eSIR_SUCCESS)
            {
                limPktFree(pMac, HAL_TXRX_FRM_802_11_MGMT, pRxPacketInfo, limMsg->bodyptr);
                return;
            }
        }
#endif
        if((fc.subType != SIR_MAC_MGMT_PROBE_RSP )&&
            (fc.subType != SIR_MAC_MGMT_BEACON)&&
            (fc.subType != SIR_MAC_MGMT_PROBE_REQ)
            && (fc.subType != SIR_MAC_MGMT_ACTION ) //                                                                 
          )
        {

            if((psessionEntry = peFindSessionByPeerSta(pMac,pHdr->sa,&sessionId))== NULL) 
            {
               limLog(pMac, LOG1, FL("session does not exist for given bssId"));
               limPktFree(pMac, HAL_TXRX_FRM_802_11_MGMT, pRxPacketInfo, limMsg->bodyptr);
               return;
            }
        }
        //                                                         
        //                                                           
        if(fc.subType == SIR_MAC_MGMT_ACTION )
        {
           psessionEntry = peFindSessionByBssid(pMac,pHdr->da,&sessionId);
        }
    }


    /*                                     */
    if(limCheckMgmtRegisteredFrames(pMac, pRxPacketInfo, psessionEntry))
    {        
        limLog( pMac, LOG1, FL("Received frame is passed to SME"));
        limPktFree(pMac, HAL_TXRX_FRM_802_11_MGMT, pRxPacketInfo, limMsg->bodyptr);
        return;
    }



    if (fc.protVer != SIR_MAC_PROTOCOL_VERSION)
    {   //                                              
        limLog(pMac, LOGE, FL("Unexpected frame with protVersion %d received"),
           fc.protVer);
        limPktFree(pMac, HAL_TXRX_FRM_802_11_MGMT, pRxPacketInfo, (void *) limMsg->bodyptr);
#ifdef WLAN_DEBUG            
        pMac->lim.numProtErr++;
#endif
        return;
    }


    if (limIsSystemInScanState(pMac))
    {
        limHandleFramesInScanState(pMac, limMsg, pRxPacketInfo, pDeferMsg, psessionEntry);
        return;
    }

/*                                                                                             */

#if 0
    if (psessionEntry->limSystemRole == eLIM_UNKNOWN_ROLE) {
        limLog( pMac, LOGW, FL( "gLimSystemRole is %d. Exiting..." ),psessionEntry->limSystemRole );
        limPktFree(pMac, HAL_TXRX_FRM_802_11_MGMT, pRxPacketInfo, (void *) limMsg->bodyptr);

#ifdef WLAN_DEBUG        
        pMac->lim.numProtErr++;
#endif
        return;
    }
 #endif     //                             


#ifdef WLAN_DEBUG    
    pMac->lim.numMAC[fc.type][fc.subType]++;
#endif
    
    switch (fc.type)
    {
        case SIR_MAC_MGMT_FRAME:
        {
                #if 0   //                   
                if (limIsReassocInProgress( pMac,psessionEntry) && (fc.subType != SIR_MAC_MGMT_DISASSOC) &&
                                                (fc.subType != SIR_MAC_MGMT_DEAUTH) && (fc.subType != SIR_MAC_MGMT_REASSOC_RSP))
                {
                    limLog(pMac, LOGE, FL("Frame with Type - %d, Subtype - %d received in ReAssoc Wait state, dropping..."),
                                                                fc.type, fc.subType);
                    return;
            }
                #endif   //                          
            //                          
            switch (fc.subType)
            {
                case SIR_MAC_MGMT_ASSOC_REQ:
                    //                                        
                    if ((psessionEntry->limSystemRole == eLIM_BT_AMP_AP_ROLE)
                    || (psessionEntry->limSystemRole == eLIM_AP_ROLE)
                    )
                        limProcessAssocReqFrame(pMac, pRxPacketInfo, LIM_ASSOC, psessionEntry);

                    else
                    {
                        //                              
                        limLog(pMac, LOGE, FL("unexpected message received %X"),limMsg->type);
                        limPrintMsgName(pMac, LOGE, limMsg->type);
                    }
                    break;

                case SIR_MAC_MGMT_ASSOC_RSP:
                    limProcessAssocRspFrame(pMac, pRxPacketInfo, LIM_ASSOC,psessionEntry);
                    break;

                case SIR_MAC_MGMT_REASSOC_REQ:
                    //                                          
                    if ((psessionEntry->limSystemRole == eLIM_BT_AMP_AP_ROLE)
                      || (psessionEntry->limSystemRole == eLIM_AP_ROLE)
                    ){
                        limProcessAssocReqFrame(pMac, pRxPacketInfo, LIM_REASSOC, psessionEntry);
                    }
                    else
                    {
                        //                              
                        limLog(pMac, LOGE, FL("unexpected message received %X"),limMsg->type);
                        limPrintMsgName(pMac, LOGE, limMsg->type);
                    }
                    break;

                case SIR_MAC_MGMT_REASSOC_RSP:
                    limProcessAssocRspFrame(pMac, pRxPacketInfo, LIM_REASSOC,psessionEntry);
                    break;

                case SIR_MAC_MGMT_PROBE_REQ:
                    limProcessProbeReqFrame_multiple_BSS(pMac, pRxPacketInfo,psessionEntry);
                    break;

                case SIR_MAC_MGMT_PROBE_RSP:
                    if(psessionEntry == NULL)
                        limProcessProbeRspFrameNoSession(pMac, pRxPacketInfo);
                    else
                        limProcessProbeRspFrame(pMac, pRxPacketInfo, psessionEntry);
                    break;

                case SIR_MAC_MGMT_BEACON:
                    __limHandleBeacon(pMac, limMsg,psessionEntry);
                    break;

                case SIR_MAC_MGMT_DISASSOC:
                    limProcessDisassocFrame(pMac, pRxPacketInfo,psessionEntry);
                    break;

                case SIR_MAC_MGMT_AUTH:
                    limProcessAuthFrame(pMac, pRxPacketInfo,psessionEntry);
                    break;

                case SIR_MAC_MGMT_DEAUTH:
                    limProcessDeauthFrame(pMac, pRxPacketInfo,psessionEntry);
                    break;

                case SIR_MAC_MGMT_ACTION:
                   if(psessionEntry == NULL)
                      limProcessActionFrameNoSession(pMac, pRxPacketInfo);
                   else
                   {
                      if (WDA_GET_RX_UNKNOWN_UCAST(pRxPacketInfo))
                         limHandleUnknownA2IndexFrames(pMac, pRxPacketInfo,psessionEntry);
                     else
                         limProcessActionFrame(pMac, pRxPacketInfo,psessionEntry);
                   }
                   break;
                default:
                    //                                                
                    break;
            } //                    

        }
        break;
        case SIR_MAC_DATA_FRAME:
        {
#ifdef FEATURE_WLAN_TDLS_INTERNAL
            /*
                                                               
                                                                   
                                                                               
                                       
             */ 
            tANI_U8 *dataOffset = WDA_GET_RX_MPDU_DATA(pRxPacketInfo);
            tANI_U8 *rfc1042Hdr = (tANI_U8 *)(dataOffset + RFC1042_HDR_LENGTH) ;
            tANI_U16 ethType = GET_BE16(rfc1042Hdr) ;
            VOS_TRACE(VOS_MODULE_ID_PE, VOS_TRACE_LEVEL_ERROR, 
                                ("TDLS frame with 80211 Header")) ;
            if(ETH_TYPE_89_0d == ethType)
            {
                tANI_U8 payloadType = (rfc1042Hdr + ETH_TYPE_LEN)[0] ;
                if(PAYLOAD_TYPE_TDLS == payloadType)
                {
                    limProcessTdlsFrame(pMac, (tANI_U32*)pRxPacketInfo) ;
                }
            }     
#endif
#ifdef FEATURE_WLAN_CCX
             /*                                                     
                                                                 
                       
              */
             if (psessionEntry && psessionEntry->isCCXconnection) {
                 limProcessIappFrame(pMac, pRxPacketInfo, psessionEntry);
             }
#endif
        }
        break;
        default:
            //                                  
            break;

    } //                 

#ifdef WLAN_FEATURE_ROAM_SCAN_OFFLOAD
end:
#endif
    limPktFree(pMac, HAL_TXRX_FRM_802_11_MGMT, pRxPacketInfo, (void *) limMsg->bodyptr) ;
    return;
} /*                                */


/* 
                           
  
           
                                                                                  
  
  
       
  
                                                    
                                                        
               
 */
void
limProcessAbortScanInd(tpAniSirGlobal pMac)
{
#ifdef FEATURE_WLAN_DIAG_SUPPORT_LIM //                          
    limDiagEventReport(pMac, WLAN_PE_DIAG_SCAN_ABORT_IND_EVENT, NULL, 0, 0);
#endif //                         
    
    /*                                                                  
                                                                 
                                         
     */
    PELOG2(limLog(pMac, LOG2, FL("Processing AbortScan Ind"));)

    limAbortBackgroundScan(pMac);

    /*                                                 */
    if(limIsSystemInScanState(pMac))
    {
        if( (eLIM_HAL_INIT_SCAN_WAIT_STATE == pMac->lim.gLimHalScanState ) ||
            (eLIM_HAL_START_SCAN_WAIT_STATE == pMac->lim.gLimHalScanState ) ||
            (eLIM_HAL_END_SCAN_WAIT_STATE == pMac->lim.gLimHalScanState ) ||
            (eLIM_HAL_FINISH_SCAN_WAIT_STATE == pMac->lim.gLimHalScanState) )
        {
            //                              
            limLog( pMac, LOGW, FL(" waiting for HAL, simply signal abort gLimHalScanState = %d"), pMac->lim.gLimHalScanState );
            pMac->lim.abortScan = 1;
        }
        else
        {
            //           
            limLog( pMac, LOGW, FL(" Force aborting scan") );
            pMac->lim.abortScan = 0;
            limDeactivateAndChangeTimer(pMac, eLIM_MIN_CHANNEL_TIMER);
            limDeactivateAndChangeTimer(pMac, eLIM_MAX_CHANNEL_TIMER);
            //                                                       
            //                                                               
            peSetResumeChannel(pMac, 0, 0);
            limSendHalFinishScanReq(pMac, eLIM_HAL_FINISH_SCAN_WAIT_STATE);
        }
    }
    return;
}

/* 
                      
  
           
                                                                                  
                                               
                                                 
                                      
               
 */

void limMessageProcessor(tpAniSirGlobal pMac, tpSirMsgQ limMsg)
{
    if (eLIM_MLM_OFFLINE_STATE == pMac->lim.gLimMlmState)
    {
        peFreeMsg(pMac, limMsg);
        return;
    }

    if (!defMsgDecision(pMac, limMsg))
    {
        limProcessMessages(pMac, limMsg);
        //                                          
        {
            if ( (! (pMac->lim.gLimAddtsSent))
                            &&
                    (! (limIsSystemInScanState(pMac)))
                )
            {
                if (true == GET_LIM_PROCESS_DEFD_MESGS(pMac))
                  limProcessDeferredMessageQueue(pMac);
            }
        }
    }
}

#ifdef FEATURE_OEM_DATA_SUPPORT

void limOemDataRspHandleResumeLinkRsp(tpAniSirGlobal pMac, eHalStatus status, tANI_U32* mlmOemDataRsp)
{
    if(status != eHAL_STATUS_SUCCESS)
    {
        limLog(pMac, LOGE, FL("OEM Data Rsp failed to get the response for resume link"));
    }

    if(NULL != pMac->lim.gpLimMlmOemDataReq)
    {
        palFreeMemory(pMac->hHdd, pMac->lim.gpLimMlmOemDataReq);
        pMac->lim.gpLimMlmOemDataReq = NULL;
    }

    //                                                              
    //                                                                             
    //                                     
    //                        
    limPostSmeMessage(pMac, LIM_MLM_OEM_DATA_CNF, (tANI_U32*)(mlmOemDataRsp));

    return;
}

void limProcessOemDataRsp(tpAniSirGlobal pMac, tANI_U32* body)
{
    eHalStatus status = eHAL_STATUS_SUCCESS;
    tpLimMlmOemDataRsp mlmOemDataRsp = NULL;
    tpStartOemDataRsp oemDataRsp = NULL;

    //                                          
    SET_LIM_PROCESS_DEFD_MESGS(pMac, true);

    oemDataRsp = (tpStartOemDataRsp)(body);

    status = palAllocateMemory(pMac->hHdd, (void**)(&mlmOemDataRsp), sizeof(tLimMlmOemDataRsp));
    if(status != eHAL_STATUS_SUCCESS)
    {
        limLog(pMac, LOGP, FL("could not allocate memory for mlmOemDataRsp"));
        return;
    }

    //                                                                    
    //                                                                             
    palCopyMemory(pMac->hHdd, (void*)(mlmOemDataRsp), (void*)(oemDataRsp), sizeof(tLimMlmOemDataRsp));

    //                            
    palFreeMemory(pMac->hHdd, (void*)(oemDataRsp));

    limResumeLink(pMac, limOemDataRspHandleResumeLinkRsp, (tANI_U32*)mlmOemDataRsp);

    return;
}

#endif


/* 
                     
  
           
                                                                   
                                               
  
        
                                                                
                                                                
                                                                      
  
              
     
  
       
     
  
                                                 
                                      
               
 */

void
limProcessMessages(tpAniSirGlobal pMac, tpSirMsgQ  limMsg)
{
    tANI_U8  deferMsg = false;
    tLinkStateParams *linkStateParams;
#if defined WLAN_FEATURE_VOWIFI_11R
    tpPESession pSession;
#endif
#if defined(ANI_DVT_DEBUG)
    tSirMsgQ  msgQ;
#endif
    if(pMac->gDriverType == eDRIVER_TYPE_MFG)
    {
        palFreeMemory(pMac->hHdd, (tANI_U8 *)limMsg->bodyptr);
        limMsg->bodyptr = NULL;
        return;
    }
#ifdef WLAN_DEBUG    
    pMac->lim.numTot++;
#endif


   PELOG3(limLog(pMac, LOG3, FL("rcvd msgType = %s, sme state = %s, mlm state = %s"),
      limMsgStr(limMsg->type), limSmeStateStr(pMac->lim.gLimSmeState),
      limMlmStateStr(pMac->lim.gLimMlmState));)

    MTRACE(macTraceMsgRx(pMac, NO_SESSION, LIM_TRACE_MAKE_RXMSG(limMsg->type, LIM_MSG_PROCESSED));)

    switch (limMsg->type)
    {

        case SIR_LIM_UPDATE_BEACON:
            limUpdateBeacon(pMac);
            break;

        case SIR_CFG_PARAM_UPDATE_IND:
            //                       
            if (limIsSystemInScanState(pMac))
            {
                //                              
                //                               
                if (limDeferMsg(pMac, limMsg) != TX_SUCCESS)
                {
                    if(!(pMac->lim.deferredMsgCnt & 0xF))
                    {
                        PELOGE(limLog(pMac, LOGE, FL("Unable to Defer message(0x%X) limSmeState %d (prev sme state %d) sysRole %d mlm state %d (prev mlm state %d)"),
                        limMsg->type, pMac->lim.gLimSmeState,  pMac->lim.gLimPrevSmeState,
                        pMac->lim.gLimSystemRole,  pMac->lim.gLimMlmState,  pMac->lim.gLimPrevMlmState);)
                    }
                    limLogSessionStates(pMac);
                    limPrintMsgName(pMac, LOGE, limMsg->type);
                }
            }
            else
            {
                limHandleCFGparamUpdate(pMac, limMsg->bodyval);
            }

            break;

        case WDA_INIT_SCAN_RSP:
            limProcessInitScanRsp(pMac, limMsg->bodyptr);
            limMsg->bodyptr = NULL;
            break;

        case WDA_START_SCAN_RSP:
            limProcessStartScanRsp(pMac, limMsg->bodyptr);
            limMsg->bodyptr = NULL;
            break;

        case WDA_END_SCAN_RSP:
            limProcessEndScanRsp(pMac, limMsg->bodyptr);
            limMsg->bodyptr = NULL;
            break;

        case WDA_FINISH_SCAN_RSP:
            limProcessFinishScanRsp(pMac, limMsg->bodyptr);
            limMsg->bodyptr = NULL;
            break;
#ifdef FEATURE_OEM_DATA_SUPPORT
        case WDA_START_OEM_DATA_RSP:
            limProcessOemDataRsp(pMac, limMsg->bodyptr);
            limMsg->bodyptr = NULL;
            break;
#endif

        case WDA_SWITCH_CHANNEL_RSP:
            limProcessSwitchChannelRsp(pMac, limMsg->bodyptr);
            limMsg->bodyptr = NULL;
            break;
            
#ifdef ANI_SIR_IBSS_PEER_CACHING
        case WDA_IBSS_STA_ADD:
            limIbssStaAdd(pMac, limMsg->bodyptr);
            break;
#endif
        case SIR_BB_XPORT_MGMT_MSG:
            //                                         
#ifdef WLAN_DEBUG                
            pMac->lim.numBbt++;
#endif

            {
                v_U16_t     pktLen = 0;
                vos_pkt_t  *pVosPkt;
                VOS_STATUS  vosStatus;
                tSirMsgQ    limMsgNew;
#ifdef FEATURE_WLAN_TDLS_INTERNAL
                tANI_U32    *pBD = NULL ;
#endif 

                /*                                                      
                                                                                          
                                                                                             
                                                                                            
                                                     
                 */
                palCopyMemory(pMac, (tANI_U8*)&limMsgNew, (tANI_U8*)limMsg, sizeof(tSirMsgQ));
                pVosPkt = (vos_pkt_t *)limMsgNew.bodyptr;
                vos_pkt_get_packet_length(pVosPkt, &pktLen);

                vosStatus = WDA_DS_PeekRxPacketInfo( pVosPkt, (v_PVOID_t *)&limMsgNew.bodyptr, VOS_FALSE );

                if( !VOS_IS_STATUS_SUCCESS(vosStatus) )
                {
                    vos_pkt_return_packet(pVosPkt);
                    break;

                }
#ifdef FEATURE_WLAN_TDLS_INTERNAL
                /* 
                                                                    
                                           
                 */
                limGetBDfromRxPacket(pMac, limMsgNew.bodyptr, &pBD);
                if(0 != WDA_GET_RX_FT_DONE(pBD))
                {
                    /*
                                                                            
                                    
                     */
                    deferMsg = false ;

                    limProcessTdlsFrame(pMac, pBD) ;
                }
                else
#endif
                limHandle80211Frames(pMac, &limMsgNew, &deferMsg);

                if ( deferMsg == true )
                {
                    PELOG1(limLog(pMac, LOG1, FL("Defer message type=%X "), limMsg->type);)
                        if (limDeferMsg(pMac, limMsg) != TX_SUCCESS)
                        {
                            PELOGE(limLog(pMac, LOGE, FL("Unable to Defer message(0x%X) limSmeState %d (prev sme state %d) sysRole %d mlm state %d (prev mlm state %d)"),
                                limMsg->type, pMac->lim.gLimSmeState,  pMac->lim.gLimPrevSmeState,
                                pMac->lim.gLimSystemRole,  pMac->lim.gLimMlmState,  pMac->lim.gLimPrevMlmState);)
                            limLogSessionStates(pMac);
                            limPrintMsgName(pMac, LOGE, limMsg->type);
                            vos_pkt_return_packet(pVosPkt);
                        }
                }
                else
                {
                    /*                                                                          
                                                                               
                                                       
                     */
                    vos_pkt_return_packet(pVosPkt);
                }
            }
            break;

        case eWNI_SME_SCAN_REQ:
        case eWNI_SME_REMAIN_ON_CHANNEL_REQ:
        case eWNI_SME_DISASSOC_REQ:
        case eWNI_SME_DEAUTH_REQ:
        case eWNI_SME_STA_STAT_REQ:
        case eWNI_SME_AGGR_STAT_REQ:
        case eWNI_SME_GLOBAL_STAT_REQ:
        case eWNI_SME_STAT_SUMM_REQ:
        case eWNI_SME_GET_SCANNED_CHANNEL_REQ:
        case eWNI_SME_GET_STATISTICS_REQ:
#if defined WLAN_FEATURE_VOWIFI_11R || defined FEATURE_WLAN_CCX || defined(FEATURE_WLAN_LFR)
        case eWNI_SME_GET_ROAM_RSSI_REQ:
#endif
#ifdef FEATURE_OEM_DATA_SUPPORT
        case eWNI_SME_OEM_DATA_REQ:
#endif
#ifdef FEATURE_WLAN_TDLS
        case eWNI_SME_TDLS_SEND_MGMT_REQ:
        case eWNI_SME_TDLS_ADD_STA_REQ:
        case eWNI_SME_TDLS_DEL_STA_REQ:
#endif
#ifdef FEATURE_WLAN_TDLS_INTERNAL
        case eWNI_SME_TDLS_DISCOVERY_START_REQ:
        case eWNI_SME_TDLS_LINK_START_REQ:
        case eWNI_SME_TDLS_TEARDOWN_REQ:
#endif
        case eWNI_SME_RESET_AP_CAPS_CHANGED:
            //                            
            limProcessNormalHddMsg(pMac, limMsg, true);  //                       
            break;

        case eWNI_SME_SCAN_ABORT_IND:
            vos_mem_free((v_VOID_t *)limMsg->bodyptr);
            limMsg->bodyptr = NULL;
            limProcessAbortScanInd(pMac);
            break;

        case eWNI_SME_START_REQ:
        case eWNI_SME_SYS_READY_IND:
#ifndef WNI_ASKEY_NON_SUPPORT_FEATURE
        case eWNI_SME_JOIN_REQ:
#endif
        case eWNI_SME_AUTH_REQ:
        case eWNI_SME_REASSOC_REQ:
        case eWNI_SME_START_BSS_REQ:
        case eWNI_SME_STOP_BSS_REQ:
        case eWNI_SME_SWITCH_CHL_REQ:
        case eWNI_SME_SWITCH_CHL_CB_PRIMARY_REQ:
        case eWNI_SME_SWITCH_CHL_CB_SECONDARY_REQ:
        case eWNI_SME_SETCONTEXT_REQ:
        case eWNI_SME_REMOVEKEY_REQ:
#ifndef WNI_ASKEY_NON_SUPPORT_FEATURE
        case eWNI_SME_PROMISCUOUS_MODE_REQ:
#endif
        case eWNI_SME_DISASSOC_CNF:
        case eWNI_SME_DEAUTH_CNF:
        case eWNI_SME_ASSOC_CNF:
        case eWNI_SME_REASSOC_CNF:
        case eWNI_SME_ADDTS_REQ:
        case eWNI_SME_DELTS_REQ:
        case eWNI_SME_DEL_BA_PEER_IND:
        case eWNI_SME_SET_TX_POWER_REQ:
        case eWNI_SME_GET_TX_POWER_REQ:
        case eWNI_SME_GET_NOISE_REQ:
        case eWNI_SME_GET_ASSOC_STAS_REQ:
        case eWNI_SME_TKIP_CNTR_MEAS_REQ:
        case eWNI_SME_UPDATE_APWPSIE_REQ:
        case eWNI_SME_HIDE_SSID_REQ:
        case eWNI_SME_GET_WPSPBC_SESSION_REQ:
        case eWNI_SME_SET_APWPARSNIEs_REQ:
        case eWNI_SME_CHNG_MCC_BEACON_INTERVAL:
#if defined WLAN_FEATURE_VOWIFI
        case eWNI_SME_NEIGHBOR_REPORT_REQ_IND:
        case eWNI_SME_BEACON_REPORT_RESP_XMIT_IND:
#endif
#if defined FEATURE_WLAN_CCX
        case eWNI_SME_CCX_ADJACENT_AP_REPORT:
#endif
#ifdef WLAN_FEATURE_VOWIFI_11R
        case eWNI_SME_FT_UPDATE_KEY:
        case eWNI_SME_FT_PRE_AUTH_REQ:
        case eWNI_SME_FT_AGGR_QOS_REQ:
#endif
        case eWNI_SME_ADD_STA_SELF_REQ:
        case eWNI_SME_DEL_STA_SELF_REQ:
        case eWNI_SME_REGISTER_MGMT_FRAME_REQ:
        case eWNI_SME_UPDATE_NOA:
        case eWNI_SME_CLEAR_DFS_CHANNEL_LIST:
            //                            
            limProcessNormalHddMsg(pMac, limMsg, false);   //                          
            break;

        //                            
        case eWNI_PMC_PWR_SAVE_CFG:
        case eWNI_PMC_ENTER_BMPS_REQ:
        case eWNI_PMC_EXIT_BMPS_REQ:
        case eWNI_PMC_ENTER_IMPS_REQ:
        case eWNI_PMC_EXIT_IMPS_REQ:
        case eWNI_PMC_ENTER_UAPSD_REQ:
        case eWNI_PMC_EXIT_UAPSD_REQ:
        case eWNI_PMC_ENTER_WOWL_REQ:
        case eWNI_PMC_EXIT_WOWL_REQ:
        case eWNI_PMC_WOWL_ADD_BCAST_PTRN:
        case eWNI_PMC_WOWL_DEL_BCAST_PTRN:
            pmmProcessMessage(pMac, limMsg);
            break;

        case eWNI_PMC_SMPS_STATE_IND :
        {
            if(limMsg->bodyptr){
            palFreeMemory(pMac->hHdd, (tANI_U8 *)limMsg->bodyptr);
            limMsg->bodyptr = NULL;
            }
        }
            break;
        case eWNI_SME_SEND_ACTION_FRAME_IND:
            limSendP2PActionFrame(pMac, limMsg);
            palFreeMemory(pMac->hHdd, (tANI_U8 *)limMsg->bodyptr);
            limMsg->bodyptr = NULL;
            break;
        case eWNI_SME_ABORT_REMAIN_ON_CHAN_IND:
            limAbortRemainOnChan(pMac);
            palFreeMemory(pMac->hHdd, (tANI_U8 *)limMsg->bodyptr);
            limMsg->bodyptr = NULL;
            break;

        case SIR_HAL_P2P_NOA_START_IND:
        {
            tpPESession psessionEntry = &pMac->lim.gpSession[0];
            tANI_U8  i;
            tANI_U8 p2pGOExists = 0;
            
            limLog(pMac, LOG1, "LIM received NOA start %x", limMsg->type);

            /*                                                                                                */
            limDeactivateAndChangeTimer(pMac, eLIM_INSERT_SINGLESHOT_NOA_TIMER);

            for(i=0; i < pMac->lim.maxBssId; i++)
            {
                psessionEntry = &pMac->lim.gpSession[i];
                if   ( (psessionEntry != NULL) && (psessionEntry->valid) &&
                    (psessionEntry->pePersona == VOS_P2P_GO_MODE))
                { //                                                
                    p2pGOExists = 1;
                    palCopyMemory(pMac->hHdd, &psessionEntry->p2pGoPsNoaStartInd, limMsg->bodyptr, sizeof(tSirP2PNoaStart));
                    if (psessionEntry->p2pGoPsNoaStartInd.status != eHAL_STATUS_SUCCESS)
                    {
                        limLog(pMac, LOGW, FL("GO NOA start failure status %d reported by FW."
                            " - still go ahead with deferred sme req. This is just info"),
                            psessionEntry->p2pGoPsNoaStartInd.status);
                    }
                    break;
                }
            }

            if (p2pGOExists == 0)
            {
                limLog(pMac, LOGW, FL("By the time, we received NOA start, GO is already removed."
                        " - still go ahead with deferred sme req. This is just info"));
            }

            /*                                                                                                                 */
            limProcessRegdDefdSmeReqAfterNOAStart(pMac);
            palFreeMemory(pMac->hHdd, (tANI_U8 *)limMsg->bodyptr);
            limMsg->bodyptr = NULL;
         }
            break;

        case SIR_HAL_P2P_NOA_ATTR_IND:
            {
                tpPESession psessionEntry = &pMac->lim.gpSession[0];  
                tANI_U8  i;
                
                limLog(pMac, LOGW, FL("Received message Noa_ATTR %x"), limMsg->type);
                for(i=0; i < pMac->lim.maxBssId; i++)
                {
                    psessionEntry = &pMac->lim.gpSession[i];
                    if   ( (psessionEntry != NULL) && (psessionEntry->valid) && 
                        (psessionEntry->pePersona == VOS_P2P_GO_MODE))
                    { //                                      
                    
                        palCopyMemory(pMac->hHdd,&psessionEntry->p2pGoPsUpdate, limMsg->bodyptr,sizeof(tSirP2PNoaAttr));
                        
                        
                        limLog(pMac, LOG2, FL(" &psessionEntry->bssId%02x:%02x:%02x:%02x:%02x:%02x ctWin=%d oppPsFlag=%d"),
                                     psessionEntry->bssId[0],
                                     psessionEntry->bssId[1],
                                     psessionEntry->bssId[2],
                                     psessionEntry->bssId[3],
                                     psessionEntry->bssId[4],
                                     psessionEntry->bssId[5],                                     
                                     psessionEntry->p2pGoPsUpdate.ctWin,
                                     psessionEntry->p2pGoPsUpdate.oppPsFlag);

                        limLog(pMac, LOG2, FL(" uNoa1IntervalCnt=%d uNoa1Duration=%d uNoa1Interval=%d uNoa1StartTime=%d"),
                                     psessionEntry->p2pGoPsUpdate.uNoa1IntervalCnt,
                                     psessionEntry->p2pGoPsUpdate.uNoa1Duration,
                                     psessionEntry->p2pGoPsUpdate.uNoa1Interval,
                                     psessionEntry->p2pGoPsUpdate.uNoa1StartTime);

                        
                        break;
                    }
                }
                
            }
            palFreeMemory(pMac->hHdd, (tANI_U8 *)limMsg->bodyptr);
            limMsg->bodyptr = NULL;
            
            break;

            
        /*                                                               
                                                                          
                                                                              
                                                                             
                                    */
        case eWNI_SME_PRE_CHANNEL_SWITCH_FULL_POWER:
            if ( !tx_timer_running(&pMac->lim.limTimers.gLimChannelSwitchTimer) )
            {  
                limProcessChannelSwitchTimeout(pMac);
            }
            palFreeMemory(pMac->hHdd, (tANI_U8 *)limMsg->bodyptr);
            limMsg->bodyptr = NULL;
            break;

        //                                    
        case WDA_ENTER_BMPS_RSP:
        case WDA_EXIT_BMPS_RSP:
        case WDA_EXIT_BMPS_IND:
        case WDA_ENTER_IMPS_RSP:
        case WDA_EXIT_IMPS_RSP:
        case WDA_ENTER_UAPSD_RSP:
        case WDA_EXIT_UAPSD_RSP:
        case WDA_WOWL_ENTER_RSP:
        case WDA_WOWL_EXIT_RSP:
            pmmProcessMessage(pMac, limMsg);
            break;

        case WDA_LOW_RSSI_IND:
            //                          
            break;

        case WDA_BMPS_STATUS_IND:
            limHandleBmpsStatusInd(pMac);
            break;

        case WDA_MISSED_BEACON_IND:
            limHandleMissedBeaconInd(pMac, limMsg);
            palFreeMemory(pMac->hHdd, (tANI_U8 *)limMsg->bodyptr);
            limMsg->bodyptr = NULL;
            break;
        case WDA_MIC_FAILURE_IND:
           limMicFailureInd(pMac, limMsg);
           palFreeMemory(pMac->hHdd, (tANI_U8 *)limMsg->bodyptr);
           limMsg->bodyptr = NULL;
           break;
    


        case SIR_LIM_ADDTS_RSP_TIMEOUT:
            limProcessSmeReqMessages(pMac,limMsg);
            break;
#ifdef FEATURE_WLAN_CCX
        case SIR_LIM_CCX_TSM_TIMEOUT:
            limProcessTsmTimeoutHandler(pMac,limMsg);
            break;
        case WDA_TSM_STATS_RSP:
            limProcessHalCcxTsmRsp(pMac, limMsg);
            break;
#endif
        case WDA_ADD_TS_RSP:
            limProcessHalAddTsRsp(pMac, limMsg);
            break;

        case SIR_LIM_DEL_TS_IND:
          limProcessDelTsInd(pMac, limMsg);
            break;
        case SIR_LIM_ADD_BA_IND:
            limProcessAddBaInd(pMac, limMsg);
            break;
        case SIR_LIM_DEL_BA_ALL_IND:
            limDelAllBASessions(pMac);
            break;
        case SIR_LIM_DEL_BA_IND:
            limProcessMlmHalBADeleteInd( pMac, limMsg );
            break;

         case SIR_LIM_BEACON_GEN_IND: {

                if( pMac->lim.gLimSystemRole != eLIM_AP_ROLE )
                    schProcessPreBeaconInd(pMac, limMsg);    

                }
                break;

        case SIR_LIM_DELETE_STA_CONTEXT_IND:
            limDeleteStaContext(pMac, limMsg);
            break;

        case SIR_LIM_MIN_CHANNEL_TIMEOUT:
        case SIR_LIM_MAX_CHANNEL_TIMEOUT:
        case SIR_LIM_PERIODIC_PROBE_REQ_TIMEOUT:
        case SIR_LIM_JOIN_FAIL_TIMEOUT:
        case SIR_LIM_PERIODIC_JOIN_PROBE_REQ_TIMEOUT:
        case SIR_LIM_AUTH_FAIL_TIMEOUT:
        case SIR_LIM_AUTH_RSP_TIMEOUT:
        case SIR_LIM_ASSOC_FAIL_TIMEOUT:
        case SIR_LIM_REASSOC_FAIL_TIMEOUT:
#ifdef WLAN_FEATURE_VOWIFI_11R
        case SIR_LIM_FT_PREAUTH_RSP_TIMEOUT:
#endif
        case SIR_LIM_REMAIN_CHN_TIMEOUT:
        case SIR_LIM_INSERT_SINGLESHOT_NOA_TIMEOUT:
        case SIR_LIM_DISASSOC_ACK_TIMEOUT:
        case SIR_LIM_DEAUTH_ACK_TIMEOUT:
        case SIR_LIM_CONVERT_ACTIVE_CHANNEL_TO_PASSIVE:
            //                                                     

            limProcessMlmReqMessages(pMac,
                                     limMsg);

            break;

        case SIR_LIM_HEART_BEAT_TIMEOUT:
            /*                                                
                                                                     
                                     
                    */

            #if 0 
            PELOG1(limLog(pMac, LOG1, FL("Heartbeat timeout, SME %d, MLME %d, #bcn %d"),
                   pMac->lim.gLimSmeState, pMac->lim.gLimMlmState,
                   pMac->lim.gLimRxedBeaconCntDuringHB);)

            if(pMac->lim.gLimSystemRole == eLIM_STA_IN_IBSS_ROLE)
                limIbssHeartBeatHandle(pMac); //                    
            else
                /* 
                                                           
                                                          
                        */
                limHandleHeartBeatFailure(pMac);

            break;
            #endif //                 
            if (limIsSystemInScanState(pMac))
            {
                //                              
                //                               
                if (limDeferMsg(pMac, limMsg) != TX_SUCCESS)
                {
                    PELOGE(limLog(pMac, LOGE, FL("Unable to Defer message(0x%X) limSmeState %d (prev sme state %d) sysRole %d mlm state %d (prev mlm state %d)"),
                        limMsg->type, pMac->lim.gLimSmeState,  pMac->lim.gLimPrevSmeState,
                        pMac->lim.gLimSystemRole,  pMac->lim.gLimMlmState,  pMac->lim.gLimPrevMlmState);)
                    limLogSessionStates(pMac);
                }
            }
            else
            {
                 if (NULL == limMsg->bodyptr)
                 {
                     limHandleHeartBeatTimeout(pMac);
                 }
                 else
                 {
                     limHandleHeartBeatTimeoutForSession(pMac, (tpPESession)limMsg->bodyptr);
                 }
            }            
            break;

        case SIR_LIM_PROBE_HB_FAILURE_TIMEOUT:
            limHandleHeartBeatFailureTimeout(pMac);            
            break;

        case SIR_LIM_CHANNEL_SCAN_TIMEOUT:
            /* 
                                                       
                                                 
             */
             limDeactivateAndChangeTimer(pMac, eLIM_BACKGROUND_SCAN_TIMER);

            //                                             
            //                                 
            pMac->lim.gLimReportBackgroundScanResults = FALSE;
            limTriggerBackgroundScan(pMac);
            break;


        case SIR_LIM_HASH_MISS_THRES_TIMEOUT:

            /*
                                                                     
             */

            pMac->lim.gLimDisassocFrameCredit = 0;
            break;

        case SIR_LIM_CNF_WAIT_TIMEOUT:

            /*
                                                   
             */
            limHandleCnfWaitTimeout(pMac, (tANI_U16) limMsg->bodyval);

            break;

        case SIR_LIM_KEEPALIVE_TIMEOUT:
            limSendKeepAliveToPeer(pMac);

            break;

        case SIR_LIM_RETRY_INTERRUPT_MSG:
            //                                                      

            break;

        case SIR_LIM_INV_KEY_INTERRUPT_MSG:
            //                                                         

            break;

        case SIR_LIM_KEY_ID_INTERRUPT_MSG:
            //                                                    

            break;

        case SIR_LIM_REPLAY_THRES_INTERRUPT_MSG:
            //                                                      

            break;

        case SIR_LIM_CHANNEL_SWITCH_TIMEOUT:
            limProcessChannelSwitchTimeout(pMac);
            break;

        case SIR_LIM_QUIET_TIMEOUT:
            limProcessQuietTimeout(pMac);
            break;

        case SIR_LIM_QUIET_BSS_TIMEOUT:
            limProcessQuietBssTimeout(pMac);
            break;

        case SIR_LIM_UPDATE_OLBC_CACHEL_TIMEOUT:
            limHandleUpdateOlbcCache(pMac);
            break;
#if 0
        case SIR_LIM_WPS_OVERLAP_TIMEOUT:
            limProcessWPSOverlapTimeout(pMac);
            break;
#endif


#ifdef FEATURE_WLAN_TDLS_INTERNAL
        /*
                                                                            
                                                                       
                             
         */
        case SIR_LIM_TDLS_DISCOVERY_RSP_WAIT:
        {
            //                                             
            tpPESession psessionEntry = peFindSessionBySessionId(pMac, 
                         pMac->lim.limTimers.gLimTdlsDisRspWaitTimer.sessionId) ;
            if(NULL == psessionEntry) 
            {
              limLog(pMac, LOGP,FL("Session Does not exist for given sessionID %d"), pMac->lim.limTimers.gLimTdlsDisRspWaitTimer.sessionId);
              return;
            }
            
            VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR, 
                                ("Discovery Rsp timer expires ")) ;
#if 0 //                                                           
            /*                     */
            limSetLinkState(pMac, eSIR_LINK_FINISH_TDLS_DISCOVERY_STATE,
                                            psessionEntry->bssId) ;
#endif
            limSendSmeTdlsDisRsp(pMac, eSIR_SUCCESS, 
                                eWNI_SME_TDLS_DISCOVERY_START_RSP) ;
            break ;
        }

        /*
                                                                     
                                                       
         */
        case SIR_LIM_TDLS_LINK_SETUP_RSP_TIMEOUT:
        {
            tANI_U8 *peerMac = (tANI_U8 *)limMsg->bodyval ;
            tLimTdlsLinkSetupPeer *setupPeer = NULL ;
 
            VOS_TRACE(VOS_MODULE_ID_PE, VOS_TRACE_LEVEL_ERROR,
                                ("TDLS setup rsp timer expires ")) ;
            VOS_TRACE(VOS_MODULE_ID_PE, VOS_TRACE_LEVEL_INFO,
                       ("TDLS setup rsp timer expires for peer:")) ;
            VOS_TRACE(VOS_MODULE_ID_PE, VOS_TRACE_LEVEL_INFO,
                    ("%02X, %02X, %02X,%02X, %02X, %02X"),
                                    peerMac[0],
                                    peerMac[1],
                                    peerMac[2],
                                    peerMac[3],
                                    peerMac[4],
                                    peerMac[5]);

            limTdlsFindLinkPeer(pMac, peerMac, &setupPeer) ;
            if(NULL != setupPeer)
            {
                limTdlsDelLinkPeer( pMac, peerMac) ;
            }

            limSendSmeTdlsLinkStartRsp(pMac, eSIR_FAILURE, peerMac, 
                                            eWNI_SME_TDLS_LINK_START_RSP) ;
            break ;
        }
        case SIR_LIM_TDLS_LINK_SETUP_CNF_TIMEOUT:
        {
            tANI_U8 *peerMac = (tANI_U8 *)limMsg->bodyval ;
            tLimTdlsLinkSetupPeer *setupPeer = NULL ;

            VOS_TRACE(VOS_MODULE_ID_PE, VOS_TRACE_LEVEL_ERROR,
                                ("TDLS setup CNF timer expires ")) ;
            VOS_TRACE(VOS_MODULE_ID_PE, VOS_TRACE_LEVEL_INFO,
                      ("TDLS setup CNF timer expires for peer:")) ;
            VOS_TRACE(VOS_MODULE_ID_PE, VOS_TRACE_LEVEL_INFO,
                         ("%02X, %02X, %02X,%02X, %02X, %02X"),
                                    peerMac[0],
                                    peerMac[1],
                                    peerMac[2],
                                    peerMac[3],
                                    peerMac[4],
                                    peerMac[5]);
            limTdlsFindLinkPeer(pMac, peerMac, &setupPeer) ;
            if(NULL != setupPeer)
            {
                limTdlsDelLinkPeer( pMac, peerMac) ;
            }
            break ;
        }
#endif   /*                         */
        case WDA_ADD_BSS_RSP:
            limProcessMlmAddBssRsp( pMac, limMsg );
            break;

        case WDA_ADD_STA_RSP:

            //                                                                                               
            limProcessAddStaRsp(pMac,limMsg);
            break;

        case WDA_DELETE_STA_RSP:
            limProcessMlmDelStaRsp(pMac, limMsg);
            break;

        case WDA_ADD_STA_SELF_RSP:
            limProcessAddStaSelfRsp(pMac, limMsg);
            break; 
        case WDA_DEL_STA_SELF_RSP:
            limProcessDelStaSelfRsp(pMac, limMsg);
            break;

        case WDA_DELETE_BSS_RSP:
            limHandleDeleteBssRsp(pMac,limMsg); //                                              
            break;

        case WDA_SET_BSSKEY_RSP:
        case WDA_SET_STA_BCASTKEY_RSP:
            limProcessMlmSetBssKeyRsp( pMac, limMsg );
            break;
        case WDA_SET_STAKEY_RSP:
            limProcessMlmSetStaKeyRsp( pMac, limMsg );
            break;
        case WDA_REMOVE_BSSKEY_RSP:
        case WDA_REMOVE_STAKEY_RSP:
            limProcessMlmRemoveKeyRsp( pMac, limMsg );
            break;
        case WDA_ADDBA_RSP:
            limProcessMlmHalAddBARsp( pMac, limMsg );
            break;

        case WDA_STA_STAT_RSP:
        case WDA_AGGR_STAT_RSP:
        case WDA_GLOBAL_STAT_RSP:
        case WDA_STAT_SUMM_RSP:
            limSendSmeStatsRsp ( pMac, limMsg->type, (void *)limMsg->bodyptr);
            break;

        case WDA_GET_STATISTICS_RSP:
            limSendSmePEStatisticsRsp ( pMac, limMsg->type, (void *)limMsg->bodyptr);
            break;
#if defined WLAN_FEATURE_VOWIFI_11R || defined FEATURE_WLAN_CCX || defined(FEATURE_WLAN_LFR)
        case WDA_GET_ROAM_RSSI_RSP:
            limSendSmePEGetRoamRssiRsp ( pMac, limMsg->type, (void *)limMsg->bodyptr);
            break;
#endif


        case WDA_SET_MIMOPS_RSP:            //                                   
        case WDA_SET_TX_POWER_RSP:          //                                      
        case WDA_GET_TX_POWER_RSP:          //                                      
        case WDA_GET_NOISE_RSP:
            vos_mem_free((v_VOID_t*)limMsg->bodyptr);
            limMsg->bodyptr = NULL;
            //                                    
            break;

        case WDA_SET_MAX_TX_POWER_RSP:
#if defined WLAN_FEATURE_VOWIFI
            rrmSetMaxTxPowerRsp( pMac, limMsg );
#endif
            if(limMsg->bodyptr != NULL)
            {
               vos_mem_free((v_VOID_t*)limMsg->bodyptr);
               limMsg->bodyptr = NULL;
            }
            break;

       case SIR_LIM_ADDR2_MISS_IND:
       {
           limLog(pMac, LOGE,
                   FL("Addr2 mismatch interrupt received %X"),
                   limMsg->type);
           /*                                                               
                                                                        */
           //                                                            
           //                                                              

           /*                         */ 
           vos_mem_free((v_VOID_t *)(limMsg->bodyptr));
           limMsg->bodyptr = NULL;
           break;
       }

#ifdef WLAN_FEATURE_VOWIFI_11R
    case WDA_AGGR_QOS_RSP:
       limProcessFTAggrQoSRsp( pMac, limMsg );
       break;
#endif

    case WDA_SET_LINK_STATE_RSP:
       linkStateParams = (tLinkStateParams *)limMsg->bodyptr;
#if defined WLAN_FEATURE_VOWIFI_11R
       pSession = linkStateParams->session;
       if(linkStateParams->ft)
       {
          limSendReassocReqWithFTIEsMgmtFrame(pMac, 
                                              pSession->pLimMlmReassocReq,
                                              pSession); 
       }
#endif
       if( linkStateParams->callback )
       {
          linkStateParams->callback( pMac, linkStateParams->callbackArg );
       }
       vos_mem_free((v_VOID_t *)(limMsg->bodyptr));
       limMsg->bodyptr = NULL;
       break;

#ifdef WLAN_FEATURE_PACKET_FILTERING
    case WDA_PACKET_COALESCING_FILTER_MATCH_COUNT_RSP:
        pmmProcessMessage(pMac, limMsg);
        break;
#endif //                              

#ifdef WLAN_FEATURE_GTK_OFFLOAD
    case WDA_GTK_OFFLOAD_GETINFO_RSP:
        pmmProcessMessage(pMac, limMsg);
        break;
#endif //                         
    case eWNI_SME_SET_BCN_FILTER_REQ:
        {
#ifdef WLAN_ACTIVEMODE_OFFLOAD_FEATURE
            tpPESession     psessionEntry;
            tANI_U8 sessionId = (tANI_U8)limMsg->bodyval ;
            psessionEntry = &pMac->lim.gpSession[sessionId];
            if(psessionEntry != NULL && IS_ACTIVEMODE_OFFLOAD_FEATURE_ENABLE)
            {
               //                                                 
               if (limSendBeaconFilterInfo(pMac, psessionEntry) != eSIR_SUCCESS)
               {
                  limLog(pMac, LOGE, FL("Fail to send Beacon Filter Info "));
               }
            }
            vos_mem_free((v_VOID_t *)(limMsg->bodyptr));
            limMsg->bodyptr = NULL;
#endif
        }
        break;
    default:
        vos_mem_free((v_VOID_t*)limMsg->bodyptr);
        limMsg->bodyptr = NULL;
        //                  
        //          
        limLog(pMac, LOGE,
                FL("Discarding unexpected message received %X"),
                limMsg->type);
        limPrintMsgName(pMac, LOGE, limMsg->type);
        break;

    } //                      

   PELOG2(limLog(pMac, LOG2, FL("Done Processing msgType = %d, sme state = %s, mlm state = %s"),
            limMsg->type, limSmeStateStr(pMac->lim.gLimSmeState),
            limMlmStateStr(pMac->lim.gLimMlmState));)

} /*                              */



/* 
                                 
  
           
                                                          
                                                         
                                          
  
        
  
              
     
  
       
     
  
                                                 
               
 */

void
limProcessDeferredMessageQueue(tpAniSirGlobal pMac)
{
    tSirMsgQ  limMsg = { 0, 0, 0 };

    tSirMsgQ *readMsg;
    tANI_U16  size;

    /*
                                                       
     */
    size = pMac->lim.gLimDeferredMsgQ.size;
    if (size > 0)
    {
        while ((readMsg = limReadDeferredMsgQ(pMac)) != NULL)
        {
            palCopyMemory( pMac->hHdd, (tANI_U8*) &limMsg,
                    (tANI_U8*) readMsg, sizeof(tSirMsgQ));
            size--; 
            limProcessMessages(pMac, &limMsg);

            if((limIsSystemInScanState(pMac)) || (true != GET_LIM_PROCESS_DEFD_MESGS(pMac)) ||
                 (pMac->lim.gLimSystemInScanLearnMode))
                break;
        }
    }
} /*                                          */


/*
                         
                                                                                                                
                                                 
                                                      
                                                   
               
 */
void limProcessNormalHddMsg(tpAniSirGlobal pMac, tSirMsgQ *pLimMsg, tANI_U8 fRspReqd)
{
    tANI_BOOLEAN fDeferMsg = eANI_BOOLEAN_TRUE;

    /*                          */  
    if ((pMac->lim.gLimSystemRole == eLIM_AP_ROLE) ||(pMac->lim.gLimSystemRole == eLIM_BT_AMP_AP_ROLE )
                                                   ||(pMac->lim.gLimSystemRole == eLIM_BT_AMP_STA_ROLE)
                                                   ||(pMac->lim.gLimSystemRole == eLIM_UNKNOWN_ROLE))
    {
        /*                                                        
                                                                           
                                                                           
                                                                             
                                                  
                                                                        
                                                                         
                                                    
          
                                                                         
                                            
          */
        if (((pLimMsg->type == eWNI_SME_START_BSS_REQ) ||
             (pLimMsg->type == eWNI_SME_STOP_BSS_REQ) ||
             (pLimMsg->type == eWNI_SME_SWITCH_CHL_REQ) ||
             (pLimMsg->type == eWNI_SME_SWITCH_CHL_CB_SECONDARY_REQ) ||
             (pLimMsg->type == eWNI_SME_SWITCH_CHL_CB_PRIMARY_REQ)))
        {
            fDeferMsg = eANI_BOOLEAN_FALSE;
        }
    }

    /*                                                                                 */
    if (((pMac->lim.gLimAddtsSent) || (limIsSystemInScanState(pMac)) /*  
                                             */) && fDeferMsg)
    {
        //                                                         
        //                                                        
        if (limDeferMsg(pMac, pLimMsg) != TX_SUCCESS)
        {
#ifdef WLAN_DEBUG            
            pMac->lim.numSme++;
#endif
            PELOGE(limLog(pMac, LOGE, FL("Unable to Defer message(0x%X) limSmeState %d (prev sme state %d) sysRole %d mlm state %d (prev mlm state %d)"),
                        pLimMsg->type, pMac->lim.gLimSmeState,  pMac->lim.gLimPrevSmeState,
                        pMac->lim.gLimSystemRole,  pMac->lim.gLimMlmState,  pMac->lim.gLimPrevMlmState);)
            limLogSessionStates(pMac);
            limPrintMsgName(pMac, LOGE, pLimMsg->type);
            //             
            palFreeMemory( pMac->hHdd, (tANI_U8 *) pLimMsg->bodyptr);
            pLimMsg->bodyptr = NULL;
        }
    }
    else
    {
        if(fRspReqd)
        {
            //                            
            //                                           
            //                                      
            //                                      
            pMac->lim.gLimRspReqd = eANI_BOOLEAN_TRUE;
        }
#ifdef WLAN_DEBUG            
        pMac->lim.numSme++;
#endif
        if(limProcessSmeReqMessages(pMac, pLimMsg))
        {
            //             
            //                                                             
            palFreeMemory( pMac->hHdd, (tANI_U8 *) pLimMsg->bodyptr);
            pLimMsg->bodyptr = NULL;
        }
    }
}

void
handleHTCapabilityandHTInfo(struct sAniSirGlobal *pMac, tpPESession psessionEntry)
{
    tSirMacHTCapabilityInfo macHTCapabilityInfo;
    tSirMacHTParametersInfo macHTParametersInfo;
    tSirMacHTInfoField1 macHTInfoField1;
    tSirMacHTInfoField2 macHTInfoField2;
    tSirMacHTInfoField3 macHTInfoField3;
    tANI_U32  cfgValue;
    tANI_U8 *ptr;

    if (wlan_cfgGetInt(pMac, WNI_CFG_HT_CAP_INFO, &cfgValue) != eSIR_SUCCESS)
    {
        limLog(pMac, LOGP, FL("Fail to retrieve WNI_CFG_HT_CAP_INFO value"));
        return ;
    }
    ptr = (tANI_U8 *) &macHTCapabilityInfo;
    *((tANI_U16 *)ptr) =  (tANI_U16) (cfgValue & 0xffff);
    pMac->lim.gHTLsigTXOPProtection = (tANI_U8)macHTCapabilityInfo.lsigTXOPProtection;
    pMac->lim.gHTMIMOPSState = (tSirMacHTMIMOPowerSaveState) macHTCapabilityInfo.mimoPowerSave;
    pMac->lim.gHTGreenfield = (tANI_U8)macHTCapabilityInfo.greenField;
    pMac->lim.gHTMaxAmsduLength = (tANI_U8)macHTCapabilityInfo.maximalAMSDUsize;
    pMac->lim.gHTShortGI20Mhz = (tANI_U8)macHTCapabilityInfo.shortGI20MHz;
    pMac->lim.gHTShortGI40Mhz = (tANI_U8)macHTCapabilityInfo.shortGI40MHz;
    pMac->lim.gHTPSMPSupport = (tANI_U8)macHTCapabilityInfo.psmp;
    pMac->lim.gHTDsssCckRate40MHzSupport = (tANI_U8)macHTCapabilityInfo.dsssCckMode40MHz;

    if (wlan_cfgGetInt(pMac, WNI_CFG_HT_AMPDU_PARAMS, &cfgValue) != eSIR_SUCCESS)
    {
        limLog(pMac, LOGP, FL("Fail to retrieve WNI_CFG_HT_PARAM_INFO value"));
        return ;
    }
    ptr = (tANI_U8 *) &macHTParametersInfo;
    *ptr =  (tANI_U8) (cfgValue & 0xff);
    pMac->lim.gHTAMpduDensity = (tANI_U8)macHTParametersInfo.mpduDensity;
    pMac->lim.gHTMaxRxAMpduFactor = (tANI_U8)macHTParametersInfo.maxRxAMPDUFactor;

    //               
    if (wlan_cfgGetInt(pMac, WNI_CFG_HT_INFO_FIELD1, &cfgValue) != eSIR_SUCCESS)
    {
        limLog(pMac, LOGP, FL("Fail to retrieve WNI_CFG_HT_INFO_FIELD1 value"));
        return ;
    }
    ptr = (tANI_U8 *) &macHTInfoField1;
    *((tANI_U8 *)ptr) =  (tANI_U8) (cfgValue & 0xff);
    pMac->lim.gHTServiceIntervalGranularity = (tANI_U8)macHTInfoField1.serviceIntervalGranularity;
    pMac->lim.gHTControlledAccessOnly = (tANI_U8)macHTInfoField1.controlledAccessOnly;
    pMac->lim.gHTRifsMode = (tANI_U8)macHTInfoField1.rifsMode;

    if (wlan_cfgGetInt(pMac, WNI_CFG_HT_INFO_FIELD2, &cfgValue) != eSIR_SUCCESS)
    {
        limLog(pMac, LOGP, FL("Fail to retrieve WNI_CFG_HT_INFO_FIELD2 value"));
        return ;
    }
    ptr = (tANI_U8 *) &macHTInfoField2;
    *((tANI_U16 *)ptr) = (tANI_U16) (cfgValue & 0xffff);
    pMac->lim.gHTOperMode = (tSirMacHTOperatingMode) macHTInfoField2.opMode;

    if (wlan_cfgGetInt(pMac, WNI_CFG_HT_INFO_FIELD3, &cfgValue) != eSIR_SUCCESS)
    {
        limLog(pMac, LOGP, FL("Fail to retrieve WNI_CFG_HT_INFO_FIELD3 value"));
        return ;
    }
    ptr = (tANI_U8 *) &macHTInfoField3;
    *((tANI_U16 *)ptr) = (tANI_U16) (cfgValue & 0xffff);
    pMac->lim.gHTPCOActive = (tANI_U8)macHTInfoField3.pcoActive;
    pMac->lim.gHTPCOPhase = (tANI_U8)macHTInfoField3.pcoPhase;
    pMac->lim.gHTSecondaryBeacon = (tANI_U8)macHTInfoField3.secondaryBeacon;
    pMac->lim.gHTDualCTSProtection = (tANI_U8)macHTInfoField3.dualCTSProtection;
    pMac->lim.gHTSTBCBasicMCS = (tANI_U8)macHTInfoField3.basicSTBCMCS;

    /*                                                                                                                
                                                                                                         
                                                                                                                                
                                                                                                                           
     */
    if (psessionEntry != NULL)
    {
        psessionEntry->htCapability = IS_DOT11_MODE_HT(psessionEntry->dot11mode);  
        psessionEntry->beaconParams.fLsigTXOPProtectionFullSupport = (tANI_U8)macHTInfoField3.lsigTXOPProtectionFullSupport;
    }
}

void limLogSessionStates(tpAniSirGlobal pMac)
{
#ifdef WLAN_DEBUG
    int i;

    for(i = 0; i < pMac->lim.maxBssId; i++)
    {
        if(pMac->lim.gpSession[i].valid)
        {
            PELOG1(limLog(pMac, LOG1, FL("Session[%d] sysRole(%d) limSmeState %d (prev sme state %d) mlm state %d (prev mlm state %d)"),
                   i, pMac->lim.gpSession[i].limSystemRole,  pMac->lim.gpSession[i].limSmeState,  
                   pMac->lim.gpSession[i].limPrevSmeState,   pMac->lim.gpSession[i].limMlmState,  
                   pMac->lim.gpSession[i].limPrevMlmState);)
        }
    }
#endif //                
}
