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
#include "aniGlobal.h"

#include "utilsApi.h"
#include "limTypes.h"
#include "limUtils.h"
#include "limAssocUtils.h"
#include "limSecurityUtils.h"
#include "limSerDesUtils.h"
#include "schApi.h"
#include "limSendMessages.h"



/* 
                        
  
           
                                                           
                                    
  
        
  
              
  
       
  
                                                 
                                                                            
               
 */

void
limProcessDeauthFrame(tpAniSirGlobal pMac, tANI_U8 *pRxPacketInfo, tpPESession psessionEntry)
{
    tANI_U8           *pBody;
    tANI_U16          aid, reasonCode;
    tpSirMacMgmtHdr   pHdr;
    tLimMlmAssocCnf   mlmAssocCnf;
    tLimMlmDeauthInd  mlmDeauthInd;
    tpDphHashNode     pStaDs;


    pHdr = WDA_GET_RX_MAC_HEADER(pRxPacketInfo);

    pBody = WDA_GET_RX_MPDU_DATA(pRxPacketInfo);


    if ((eLIM_STA_ROLE == psessionEntry->limSystemRole) && (eLIM_SME_WT_DEAUTH_STATE == psessionEntry->limSmeState))
    {
       PELOGE(limLog(pMac, LOGE,
        FL("received Deauth frame in DEAUTH_WT_STATE(already processing previously received DEAUTH frame).. Dropping this..\n "));)
       return;
    }

    if (limIsGroupAddr(pHdr->sa))
    {
        //                                           
        //                        
        PELOG1(limLog(pMac, LOG1,
               FL("received Deauth frame from a BC/MC address\n"));)

        return;
    }

    if (limIsGroupAddr(pHdr->da) && !limIsAddrBC(pHdr->da))
    {
        //                                       
        //                        
        PELOG1(limLog(pMac, LOG1,
               FL("received Deauth frame for a MC address\n"));)

        return;
    }
    //                                                
    reasonCode = sirReadU16(pBody);

    PELOGE(limLog(pMac, LOGE,
        FL("received Deauth frame (mlm state = %s) with reason code %d from "),
        limMlmStateStr(psessionEntry->limMlmState), reasonCode);
    limPrintMacAddr(pMac, pHdr->sa, LOGE);)
      
    if ( (psessionEntry->limSystemRole == eLIM_AP_ROLE )||(psessionEntry->limSystemRole == eLIM_BT_AMP_AP_ROLE) )
    {
        switch (reasonCode)
        {
            case eSIR_MAC_UNSPEC_FAILURE_REASON:
            case eSIR_MAC_DEAUTH_LEAVING_BSS_REASON:
                //                                                    
                break;

            default:
                //                                                      
                //                               
                PELOG1(limLog(pMac, LOG1,
                   FL("received Deauth frame with invalid reasonCode %d from \n"),
                   reasonCode);
                limPrintMacAddr(pMac, pHdr->sa, LOG1);)

                break;
        }
    }
    else if (psessionEntry->limSystemRole == eLIM_STA_ROLE ||psessionEntry->limSystemRole == eLIM_BT_AMP_STA_ROLE)
    {
        switch (reasonCode)
        {
            case eSIR_MAC_UNSPEC_FAILURE_REASON:
            case eSIR_MAC_PREV_AUTH_NOT_VALID_REASON:
            case eSIR_MAC_DEAUTH_LEAVING_BSS_REASON:
            case eSIR_MAC_CLASS2_FRAME_FROM_NON_AUTH_STA_REASON:
            case eSIR_MAC_CLASS3_FRAME_FROM_NON_ASSOC_STA_REASON:
            case eSIR_MAC_STA_NOT_PRE_AUTHENTICATED_REASON:
                //                                          
                break;

            default:
                //                                            
                //                               
                PELOG1(limLog(pMac, LOG1,
                   FL("received Deauth frame with invalid reasonCode %d from \n"),
                   reasonCode);
                limPrintMacAddr(pMac, pHdr->sa, LOG1);)

                break;
        }
    }
    else
    {
        //                                     
        //                                          
        limLog(pMac, LOGE,
           FL("received Deauth frame with reasonCode %d in role %d from \n"),
           reasonCode, psessionEntry->limSystemRole);
          limPrintMacAddr(pMac, pHdr->sa, LOGE);

        return;
    }

    /*                                                                
                                                                       
                             
                          
                                                                     
                            
                            
                          
      
                      
                                                                      
                                                                      
                                                                         
                                                                     
                                                                    
                                                                    
                                          
     */
    if (limIsReassocInProgress(pMac,psessionEntry)) {
        if (!IS_REASSOC_BSSID(pMac,pHdr->sa,psessionEntry)) {
            PELOGE(limLog(pMac, LOGE, FL("Rcv Deauth from unknown/different AP while ReAssoc. Ignore \n"));)
            limPrintMacAddr(pMac, pHdr->sa, LOGE);
            limPrintMacAddr(pMac, psessionEntry->limReAssocbssId, LOGE);
            return;
        }

        /*                                                                   
                                                                                 
         */
        if (!IS_CURRENT_BSSID(pMac, pHdr->sa,psessionEntry)) {
            PELOGE(limLog(pMac, LOGE, FL("received DeAuth from the New AP to which ReAssoc is sent \n"));)
            limPrintMacAddr(pMac, pHdr->sa, LOGE);
            limPrintMacAddr(pMac, psessionEntry->bssId, LOGE);
            limRestorePreReassocState(pMac,
                                  eSIR_SME_REASSOC_REFUSED, reasonCode,psessionEntry);
            return;
        }
    }

    
    /*                                                                        
                                                                         
     */
#ifdef WLAN_SOFTAP_FEATURE
    if(psessionEntry->limSystemRole != eLIM_AP_ROLE ){
#endif
        if (!IS_CURRENT_BSSID(pMac, pHdr->sa,psessionEntry)) 
        {
            PELOGE(limLog(pMac, LOGE, FL("received DeAuth from an AP other than we're trying to join. Ignore. \n"));)
            if (limSearchPreAuthList(pMac, pHdr->sa))
            {
                PELOGE(limLog(pMac, LOGE, FL("Preauth entry exist. Deleting... \n"));)
                limDeletePreAuthNode(pMac, pHdr->sa);
            }
            return;
        }
#ifdef WLAN_SOFTAP_FEATURE
    }
#endif

        //                           
        switch (psessionEntry->limSystemRole)
        {
            case eLIM_STA_ROLE:
            case eLIM_BT_AMP_STA_ROLE:
                switch (psessionEntry->limMlmState)
                {
                    case eLIM_MLM_WT_AUTH_FRAME2_STATE:
                        /* 
                                                             
                                                               
                                  
                         */

                        //          
                        PELOG1(limLog(pMac, LOG1,
                           FL("received Deauth frame with failure code %d from "),
                           reasonCode);
                        limPrintMacAddr(pMac, pHdr->sa, LOG1);)

                        limRestoreFromAuthState(pMac, eSIR_SME_DEAUTH_WHILE_JOIN,
                                                reasonCode,psessionEntry);

                        return;

                    case eLIM_MLM_AUTHENTICATED_STATE:
                        //                                 
                        palCopyMemory( pMac->hHdd,
                               (tANI_U8 *) &mlmDeauthInd.peerMacAddr,
                               pHdr->sa,
                               sizeof(tSirMacAddr));
                        mlmDeauthInd.reasonCode = reasonCode;

                        psessionEntry->limMlmState = eLIM_MLM_IDLE_STATE;
                        MTRACE(macTrace(pMac, TRACE_CODE_MLM_STATE, psessionEntry->peSessionId, psessionEntry->limMlmState));

                        
                        limPostSmeMessage(pMac,
                                          LIM_MLM_DEAUTH_IND,
                                          (tANI_U32 *) &mlmDeauthInd);
                        return;

                    case eLIM_MLM_WT_ASSOC_RSP_STATE:
                        /* 
                                                              
                                                                 
                                                             
                         */
                        if (limSearchPreAuthList(pMac, pHdr->sa))
                            limDeletePreAuthNode(pMac, pHdr->sa);

                       if (psessionEntry->pLimMlmJoinReq)
                        {
                            palFreeMemory( pMac->hHdd, psessionEntry->pLimMlmJoinReq);
                            psessionEntry->pLimMlmJoinReq = NULL;
                        }

                        mlmAssocCnf.resultCode = eSIR_SME_DEAUTH_WHILE_JOIN;
                        mlmAssocCnf.protStatusCode = reasonCode;
                        
                        /*              */
                        mlmAssocCnf.sessionId = psessionEntry->peSessionId;

                        psessionEntry->limMlmState =
                                   psessionEntry->limPrevMlmState;
                        MTRACE(macTrace(pMac, TRACE_CODE_MLM_STATE, psessionEntry->peSessionId, psessionEntry->limMlmState));

                        //                                      
                        limDeactivateAndChangeTimer(
                                     pMac,
                                     eLIM_ASSOC_FAIL_TIMER);

                        limPostSmeMessage(
                            pMac,
                            LIM_MLM_ASSOC_CNF,
                            (tANI_U32 *) &mlmAssocCnf);
                        
                        return;

                    case eLIM_MLM_IDLE_STATE:
                    case eLIM_MLM_LINK_ESTABLISHED_STATE:
                        /* 
                                                                    
                                                                  
                                                                     
                         */
                        if (limSearchPreAuthList(pMac, pHdr->sa))
                           limDeletePreAuthNode(pMac, pHdr->sa);

                        break;

                    case eLIM_MLM_WT_REASSOC_RSP_STATE:
                        break;

                    case eLIM_MLM_WT_FT_REASSOC_RSP_STATE:
                        PELOGE(limLog(pMac, LOGE,
                           FL("received Deauth frame in FT state %X with reasonCode=%d from "),
                           psessionEntry->limMlmState, reasonCode);)
                        limPrintMacAddr(pMac, pHdr->sa, LOGE);
                        break;

                    default:
                        PELOG1(limLog(pMac, LOG1,
                           FL("received Deauth frame in state %X with reasonCode=%d from "),
                           psessionEntry->limMlmState, reasonCode);)
                        limPrintMacAddr(pMac, pHdr->sa, LOG1);
                        return;
                }
                break;

            case eLIM_STA_IN_IBSS_ROLE:
                break;

#ifdef WLAN_SOFTAP_FEATURE
            case eLIM_AP_ROLE:
                break;
#endif 

            default: //                                    

#if (WNI_POLARIS_FW_PRODUCT == AP)
                //                                                     
                if (limSearchPreAuthList(pMac, pHdr->sa) == NULL)
                {
                    /* 
                                                                           
                                                                   
                                                         
                     */
                    PELOG1(limLog(pMac, LOG1,
                       FL("received Deauth frame from peer that does not have context, addr "));
                    limPrintMacAddr(pMac, pHdr->sa, LOG1);)
                }
                else
                {
                    //                                   
                    limDeletePreAuthNode(pMac,
                                         pHdr->sa);

                    palCopyMemory( pMac->hHdd,
                           (tANI_U8 *) &mlmDeauthInd.peerMacAddr,
                           pHdr->sa,
                           sizeof(tSirMacAddr));
                    mlmDeauthInd.reasonCode = reasonCode;
                    mlmDeauthInd.aid        = 0;

                    limPostSmeMessage(pMac,
                                      LIM_MLM_DEAUTH_IND,
                                      (tANI_U32 *) &mlmDeauthInd);
                }
#endif

                return;
        } //                                      


        
    /* 
                                                    
                                                    
     */
    if( (pStaDs = dphLookupHashEntry(pMac, pHdr->sa, &aid, &psessionEntry->dph.dphHashTable)) == NULL)
        return;


    if ((pStaDs->mlmStaContext.mlmState == eLIM_MLM_WT_DEL_STA_RSP_STATE) ||
        (pStaDs->mlmStaContext.mlmState == eLIM_MLM_WT_DEL_BSS_RSP_STATE))
    {
        /* 
                                                                  
                                                               
         */
        PELOG1(limLog(pMac, LOG1,
           FL("received Deauth frame from peer that is in state %X, addr "),
           pStaDs->mlmStaContext.mlmState);
        limPrintMacAddr(pMac, pHdr->sa, LOG1);)
        return;
    } 
    pStaDs->mlmStaContext.disassocReason = (tSirMacReasonCodes)reasonCode;
    pStaDs->mlmStaContext.cleanupTrigger = eLIM_PEER_ENTITY_DEAUTH;

    //                                 
    palCopyMemory( pMac->hHdd, (tANI_U8 *) &mlmDeauthInd.peerMacAddr,
                  pStaDs->staAddr,
                  sizeof(tSirMacAddr));
#if (WNI_POLARIS_FW_PRODUCT == AP)
    mlmDeauthInd.aid           = pStaDs->assocId;
#endif
    mlmDeauthInd.reasonCode    = (tANI_U8) pStaDs->mlmStaContext.disassocReason;
    mlmDeauthInd.deauthTrigger = eLIM_PEER_ENTITY_DEAUTH;


    /* 
                                                                  
                                                                   
                                                               
                                                           
     */
    if (limIsReassocInProgress(pMac,psessionEntry)) {
        /* 
                                              
                                                 
                                               
         */
        if (limSearchPreAuthList(pMac, pHdr->sa))
            limDeletePreAuthNode(pMac, pHdr->sa);

        if (psessionEntry->limAssocResponseData) {
            palFreeMemory(pMac->hHdd, psessionEntry->limAssocResponseData);
            psessionEntry->limAssocResponseData = NULL;                            
        }

        PELOGE(limLog(pMac, LOGE, FL("Rcv Deauth from ReAssoc AP. Issue REASSOC_CNF. \n"));)
       /*
                                                                          
                                                                         
                                                                     
                                                                 
        */
       limPostReassocFailure(pMac, eSIR_SME_FT_REASSOC_TIMEOUT_FAILURE,
               eSIR_MAC_UNSPEC_FAILURE_STATUS, psessionEntry);
        return;
    }

    //                                       
    limPostSmeMessage(pMac, LIM_MLM_DEAUTH_IND, (tANI_U32 *) &mlmDeauthInd);

    //                                  
    limSendSmeDeauthInd(pMac, pStaDs, psessionEntry);
    return;

} /*                                 */

