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

#include "wniApi.h"
#ifdef FEATURE_WLAN_NON_INTEGRATED_SOC
#include "halDataStruct.h"
#endif
#if (WNI_POLARIS_FW_PRODUCT == AP)
#include "wniCfgAp.h"
#else
#include "wniCfgSta.h"
#endif
#include "aniGlobal.h"
#include "cfgApi.h"

#include "utilsApi.h"
#include "limUtils.h"
#include "limAssocUtils.h"
#include "limSecurityUtils.h"
#include "limSerDesUtils.h"
#ifdef WLAN_FEATURE_VOWIFI_11R
#include "limFT.h"
#endif
#include "vos_utils.h"


/* 
              
  
           
                                                                       
                   
  
        
                                                        
                                                
                                                 
                                                                                      
                                                       
    
  
              
  
       
  
                                                       
  
                         
 */


static inline unsigned int isAuthValid(tpAniSirGlobal pMac, tpSirMacAuthFrameBody auth,tpPESession sessionEntry) {
    unsigned int valid;
    valid=1;

    if (  ((auth->authTransactionSeqNumber==SIR_MAC_AUTH_FRAME_1)||
           (auth->authTransactionSeqNumber==SIR_MAC_AUTH_FRAME_3)) &&
          ((sessionEntry->limSystemRole == eLIM_STA_ROLE)||(sessionEntry->limSystemRole == eLIM_BT_AMP_STA_ROLE)))
        valid=0;

    if ( ((auth->authTransactionSeqNumber==SIR_MAC_AUTH_FRAME_2)||(auth->authTransactionSeqNumber==SIR_MAC_AUTH_FRAME_4))&&
         ((sessionEntry->limSystemRole == eLIM_AP_ROLE)||(sessionEntry->limSystemRole == eLIM_BT_AMP_AP_ROLE)))
        valid=0;

    if ( ((auth->authTransactionSeqNumber==SIR_MAC_AUTH_FRAME_3)||(auth->authTransactionSeqNumber==SIR_MAC_AUTH_FRAME_4))&&
         (auth->type!=SIR_MAC_CHALLENGE_TEXT_EID)&&(auth->authAlgoNumber != eSIR_SHARED_KEY))
        valid=0;

    return valid;
}


/* 
                      
  
           
                                                                          
                   
  
        
                                                                     
                                                                       
                                                        
  
              
  
       
                                                                       
                                        
                                                                       
                                                                         
                                                          
                                
                                                                      
                                                                          
                                                                       
                                                                     
                                                                        
                                          
                                                                
                                                                    
                                    
  
                                                 
                                                                 
               
 */

void
limProcessAuthFrame(tpAniSirGlobal pMac, tANI_U8 *pRxPacketInfo, tpPESession psessionEntry)
{
    tANI_U8                 *pBody, keyId, cfgPrivacyOptImp,
                            defaultKey[SIR_MAC_KEY_LENGTH],
                            encrAuthFrame[LIM_ENCR_AUTH_BODY_LEN],
                            plainBody[256];
    tANI_U16                frameLen;
    //                                                           
    tANI_U32                maxNumPreAuth, val;
    tSirMacAuthFrameBody    *pRxAuthFrameBody, rxAuthFrame, authFrame;
    tpSirMacMgmtHdr         pHdr;
    tCfgWepKeyEntry         *pKeyMapEntry = NULL;
    struct tLimPreAuthNode  *pAuthNode;
    tLimMlmAuthInd          mlmAuthInd;
    tANI_U8                 decryptResult;
    tANI_U8                 *pChallenge;
    tANI_U32                key_length=8;
    tANI_U8                 challengeTextArray[SIR_MAC_AUTH_CHALLENGE_LENGTH];
#ifdef WLAN_SOFTAP_FEATURE
    tpDphHashNode           pStaDs = NULL;
    tANI_U16                assocId = 0;
#endif
    /*                           */
    //                                                    
 

    pHdr = WDA_GET_RX_MAC_HEADER(pRxPacketInfo);
    frameLen = WDA_GET_RX_PAYLOAD_LEN(pRxPacketInfo);
    

    if (!frameLen)
    {
        //          
        limLog(pMac, LOGE,
               FL("received Authentication frame with no body from "));
        limPrintMacAddr(pMac, pHdr->sa, LOGE);

        return;
    }

    if (limIsGroupAddr(pHdr->sa))
    {
        //                                         
        //                        
        PELOG1(limLog(pMac, LOG1,
               FL("received Auth frame from a BC/MC address - "));)
       PELOG1( limPrintMacAddr(pMac, pHdr->sa, LOG1);)

        return;
    }

    pBody = WDA_GET_RX_MPDU_DATA(pRxPacketInfo);

    //                                                                                                                  


   
    //                                                              
    if (pHdr->fc.wep)
    {
        /* 
                                              
         */

#ifdef WLAN_SOFTAP_FEATURE
        //                                                               
        if ((psessionEntry->bTkipCntrMeasActive) && (psessionEntry->limSystemRole == eLIM_AP_ROLE))
        {
            PELOGE( limLog(pMac, LOGE,
               FL("Tkip counter measures Enabled, sending Deauth frame to")); )
            limPrintMacAddr(pMac, pHdr->sa, LOGE);

            limSendDeauthMgmtFrame( pMac, eSIR_MAC_MIC_FAILURE_REASON,
                                    pHdr->sa, psessionEntry );
            return;
        }
#endif

        //                                                       

        keyId = (*(pBody + 3)) >> 6;

        /* 
                                                                       
                                                                     
                                                                          
                                                                         
                                                                 
                                                            
         */

        if (psessionEntry->limSystemRole == eLIM_STA_ROLE || psessionEntry->limSystemRole == eLIM_BT_AMP_STA_ROLE)
        {
            authFrame.authAlgoNumber = eSIR_SHARED_KEY;
            authFrame.authTransactionSeqNumber = SIR_MAC_AUTH_FRAME_4;
            authFrame.authStatusCode = eSIR_MAC_CHALLENGE_FAILURE_STATUS;

            limSendAuthMgmtFrame(pMac, &authFrame,
                                 pHdr->sa,
                                 LIM_NO_WEP_IN_FC,psessionEntry);
            //          
            PELOGE(limLog(pMac, LOGE,
                   FL("received Authentication frame with wep bit set on role=%d "MAC_ADDRESS_STR),
                   psessionEntry->limSystemRole, MAC_ADDR_ARRAY(pHdr->sa) );)

            return;
        }

        if (frameLen < LIM_ENCR_AUTH_BODY_LEN)
        {
            //          
            limLog(pMac, LOGE,
                   FL("Not enough size [%d] to decrypt received Auth frame"),
                   frameLen);
            limPrintMacAddr(pMac, pHdr->sa, LOGE);

            return;
        }
#ifdef WLAN_SOFTAP_FEATURE
        if(psessionEntry->limSystemRole == eLIM_AP_ROLE)
        {
            val = psessionEntry->privacy; 
        } 
        else 
#endif
        //                                                           
        if (wlan_cfgGetInt(pMac, WNI_CFG_PRIVACY_ENABLED,
                      &val) != eSIR_SUCCESS)
        {
            /* 
                                           
                                   
             */
            limLog(pMac, LOGP, FL("could not retrieve Privacy option\n"));
        }

        cfgPrivacyOptImp = (tANI_U8)val;
        if (cfgPrivacyOptImp)
        {
            /* 
                                             
                                                            
                                                                
                                                                  
             */
            pAuthNode = limSearchPreAuthList(pMac, pHdr->sa);

            if (pAuthNode == NULL)
            {
                /* 
                                                                      
                                                           
                                                                       
                 */
                authFrame.authAlgoNumber = eSIR_SHARED_KEY;
                authFrame.authTransactionSeqNumber =
                SIR_MAC_AUTH_FRAME_4;
                authFrame.authStatusCode =
                eSIR_MAC_AUTH_FRAME_OUT_OF_SEQ_STATUS;

                limSendAuthMgmtFrame(pMac, &authFrame,
                                     pHdr->sa,
                                     LIM_NO_WEP_IN_FC,psessionEntry);

                //          
                PELOGE(limLog(pMac, LOGE,
                       FL("received Authentication frame from peer that has "
                       "no preauth context with WEP bit set "MAC_ADDRESS_STR),
                       MAC_ADDR_ARRAY(pHdr->sa));)

                return;
            }
            else
            {
                //                                  
                limDeactivateAndChangePerStaIdTimer(pMac,
                                                    eLIM_AUTH_RSP_TIMER,
                                                    pAuthNode->authNodeIdx);

                //                                  
                if ((pAuthNode->mlmState !=
                     eLIM_MLM_WT_AUTH_FRAME3_STATE) &&
                    (pAuthNode->mlmState !=
                     eLIM_MLM_AUTH_RSP_TIMEOUT_STATE))
                {
                    /* 
                                                                    
                                                              
                                                                 
                                                              
                     */

                    authFrame.authAlgoNumber = eSIR_SHARED_KEY;
                    authFrame.authTransactionSeqNumber =
                    SIR_MAC_AUTH_FRAME_4;
                    authFrame.authStatusCode =
                    eSIR_MAC_AUTH_FRAME_OUT_OF_SEQ_STATUS;

                    limSendAuthMgmtFrame(pMac, &authFrame,
                                         pHdr->sa,
                                         LIM_NO_WEP_IN_FC,psessionEntry);

                    //          
                    PELOGE(limLog(pMac, LOGE,
                           FL("received Authentication frame from peer that is in state %d "
                           MAC_ADDRESS_STR), pAuthNode->mlmState, MAC_ADDR_ARRAY(pHdr->sa));)

                    return;
                }
            }

            /* 
                                                       
                                                         
             */
            pKeyMapEntry = limLookUpKeyMappings(pHdr->sa);

            if (pKeyMapEntry)
            {
                if (!pKeyMapEntry->wepOn)
                {
                    /* 
                                                      
                                                
                                                         
                     */
                    authFrame.authAlgoNumber = eSIR_SHARED_KEY;
                    authFrame.authTransactionSeqNumber =
                    SIR_MAC_AUTH_FRAME_4;
                    authFrame.authStatusCode =
                    eSIR_MAC_CHALLENGE_FAILURE_STATUS;

                    limSendAuthMgmtFrame(pMac, &authFrame,
                                         pHdr->sa,
                                         LIM_NO_WEP_IN_FC,psessionEntry);

                    //          
                    PELOGE(limLog(pMac, LOGE,
                           FL("received Auth frame3 from peer that has NULL key map entry "
                           MAC_ADDRESS_STR),MAC_ADDR_ARRAY(pHdr->sa));)

                    return;
                } //                          
                else
                {
                    decryptResult = limDecryptAuthFrame(pMac, pKeyMapEntry->key,
                                                        pBody,
                                                        plainBody,
                                                        key_length,
                                                        (tANI_U16) (frameLen-SIR_MAC_WEP_IV_LENGTH));
                    if (decryptResult == LIM_DECRYPT_ICV_FAIL)
                    {
                        //             
                        PELOGW(limLog(pMac, LOGW, FL("=====> decryptResult == LIM_DECRYPT_ICV_FAIL ..."));)
                        limDeletePreAuthNode(pMac,
                                             pHdr->sa);
                        authFrame.authAlgoNumber = eSIR_SHARED_KEY;
                        authFrame.authTransactionSeqNumber =
                        SIR_MAC_AUTH_FRAME_4;
                        authFrame.authStatusCode =
                        eSIR_MAC_CHALLENGE_FAILURE_STATUS;

                        limSendAuthMgmtFrame(
                                            pMac, &authFrame,
                                            pHdr->sa,
                                            LIM_NO_WEP_IN_FC,psessionEntry);

                        //          
                        PELOGE(limLog(pMac, LOGE,
                               FL("received Authentication frame from peer that failed decryption, Addr "
                               MAC_ADDRESS_STR), MAC_ADDR_ARRAY(pHdr->sa));)

                        return;
                    }

                    if ((sirConvertAuthFrame2Struct(pMac, plainBody, frameLen-8, &rxAuthFrame)!=eSIR_SUCCESS)||(!isAuthValid(pMac, &rxAuthFrame,psessionEntry)))
                        return;


                } //                                   
            } //                         
            else
            {

                val = SIR_MAC_KEY_LENGTH;

#ifdef WLAN_SOFTAP_FEATURE  
                if(psessionEntry->limSystemRole == eLIM_AP_ROLE)
                {   
                    tpSirKeys pKey;
                    pKey =  &psessionEntry->WEPKeyMaterial[keyId].key[0];              
                    palCopyMemory( pMac->hHdd, defaultKey, pKey->key, pKey->keyLength);
                    val = pKey->keyLength;
                }                   
                else                              
#endif                                    
                if (wlan_cfgGetStr(pMac, (tANI_U16) (WNI_CFG_WEP_DEFAULT_KEY_1 + keyId),
                              defaultKey, &val) != eSIR_SUCCESS)
                {
                    //                                     
                    //          
                    limLog(pMac, LOGP,
                           FL("could not retrieve Default key\n"));

                    /* 
                                                
                                                         
                     */

                    authFrame.authAlgoNumber = eSIR_SHARED_KEY;
                    authFrame.authTransactionSeqNumber =
                    SIR_MAC_AUTH_FRAME_4;
                    authFrame.authStatusCode =
                    eSIR_MAC_CHALLENGE_FAILURE_STATUS;

                    limSendAuthMgmtFrame(pMac, &authFrame,
                                         pHdr->sa,
                                         LIM_NO_WEP_IN_FC,psessionEntry);

                    return;
                }

                    key_length=val;

                    decryptResult = limDecryptAuthFrame(pMac, defaultKey,
                                                        pBody,
                                                        plainBody,
                                                        key_length,
                                                        (tANI_U16) (frameLen-SIR_MAC_WEP_IV_LENGTH));
                    if (decryptResult == LIM_DECRYPT_ICV_FAIL)
                    {
                        PELOGW(limLog(pMac, LOGW, FL("=====> decryptResult == LIM_DECRYPT_ICV_FAIL ...\n"));)
                        //             
                        limDeletePreAuthNode(pMac,
                                             pHdr->sa);
                        authFrame.authAlgoNumber = eSIR_SHARED_KEY;
                        authFrame.authTransactionSeqNumber =
                        SIR_MAC_AUTH_FRAME_4;
                        authFrame.authStatusCode =
                        eSIR_MAC_CHALLENGE_FAILURE_STATUS;

                        limSendAuthMgmtFrame(
                                            pMac, &authFrame,
                                            pHdr->sa,
                                            LIM_NO_WEP_IN_FC,psessionEntry);

                        //          
                        PELOGE(limLog(pMac, LOGE,
                               FL("received Authentication frame from peer that failed decryption: "
                               MAC_ADDRESS_STR), MAC_ADDR_ARRAY(pHdr->sa));)

                        return;
                    }
                    if ((sirConvertAuthFrame2Struct(pMac, plainBody, frameLen-8, &rxAuthFrame)!=eSIR_SUCCESS)||(!isAuthValid(pMac, &rxAuthFrame,psessionEntry)))
                        return;

            } //                                                  
        }
        else
        {
            /* 
                                                 
                                                           
                                                          
                                                            
                                                             
                                                                    
             */

            authFrame.authAlgoNumber = eSIR_SHARED_KEY;
            authFrame.authTransactionSeqNumber =
            SIR_MAC_AUTH_FRAME_4;
            authFrame.authStatusCode =
            eSIR_MAC_CHALLENGE_FAILURE_STATUS;

            limSendAuthMgmtFrame(pMac, &authFrame,
                                 pHdr->sa,
                                 LIM_NO_WEP_IN_FC,psessionEntry);

            //          
            PELOGE(limLog(pMac, LOGE,
                   FL("received Authentication frame3 from peer that while privacy option is turned OFF "
                   MAC_ADDRESS_STR), MAC_ADDR_ARRAY(pHdr->sa));)

            return;
        } //                                                         
    } //            
    else
    {


        if ((sirConvertAuthFrame2Struct(pMac, pBody, frameLen, &rxAuthFrame)!=eSIR_SUCCESS)||(!isAuthValid(pMac, &rxAuthFrame,psessionEntry)))
            return;
    }


    pRxAuthFrameBody = &rxAuthFrame;

   PELOGW(limLog(pMac, LOGW,
           FL("Received Auth frame with type=%d seqnum=%d, status=%d (%d)\n"),
           (tANI_U32) pRxAuthFrameBody->authAlgoNumber,
           (tANI_U32) pRxAuthFrameBody->authTransactionSeqNumber,
           (tANI_U32) pRxAuthFrameBody->authStatusCode,(tANI_U32)pMac->lim.gLimNumPreAuthContexts);)

    switch (pRxAuthFrameBody->authTransactionSeqNumber)
    {
        case SIR_MAC_AUTH_FRAME_1:
            //            

            //                                                     
            pAuthNode = limSearchPreAuthList(pMac, pHdr->sa);
            if (pAuthNode)
            {
                //                                     
                if (pHdr->fc.retry == 0)
                {
                    /* 
                                                                 
                                                                  
                                                                                          
                                                             
                     */
                    if(pAuthNode->fTimerStarted)
                    {
                        limDeactivateAndChangePerStaIdTimer(pMac,
                                                    eLIM_AUTH_RSP_TIMER,
                                                    pAuthNode->authNodeIdx);
                    }
                    PELOGE(limLog(pMac, LOGE, FL("STA is initiating brand-new Authentication ...\n"));)
                    limDeletePreAuthNode(pMac,
                                         pHdr->sa);
#ifdef WLAN_SOFTAP_FEATURE                    
                    /* 
                                                              
                                                              
                                                             
                                     
                     */  

                    for (assocId = 0; assocId < psessionEntry->dph.dphHashTable.size; assocId++)//                             
                    {
                        pStaDs = dphGetHashEntry(pMac, assocId, &psessionEntry->dph.dphHashTable);

                        if (NULL == pStaDs)
                             continue;

                        if (pStaDs->valid)
                        {
                             if (palEqualMemory( pMac->hHdd,(tANI_U8 *) &pStaDs->staAddr,
                                      (tANI_U8 *) &(pHdr->sa), (tANI_U8) (sizeof(tSirMacAddr))) )
                                  break;
                        }
                    }

                    if (NULL != pStaDs)
                    {
                        PELOGE(limLog(pMac, LOGE, FL("lim Delete Station Context (staId: %d, assocId: %d) \n"),pStaDs->staIndex, assocId);)
                        limSendDeauthMgmtFrame(pMac,
                               eSIR_MAC_UNSPEC_FAILURE_REASON, (tANI_U8 *) pAuthNode->peerMacAddr,psessionEntry);
                        limTriggerSTAdeletion(pMac, pStaDs, psessionEntry);
                        return;
                    }
#endif
                }
                else
                {
                    /* 
                                                                                  
                                                                                        
                                            
                       */
                    PELOGE(limLog(pMac, LOGE, FL("STA is initiating Authentication after ACK lost...\n"));)
                    return;
                }
            }
            if (wlan_cfgGetInt(pMac, WNI_CFG_MAX_NUM_PRE_AUTH,
                          (tANI_U32 *) &maxNumPreAuth) != eSIR_SUCCESS)
            {
                /* 
                                              
                                       
                 */
                limLog(pMac, LOGP,
                       FL("could not retrieve MaxNumPreAuth\n"));
            }
#ifdef ANI_AP_SDK_OPT
            if(maxNumPreAuth > SIR_SDK_OPT_MAX_NUM_PRE_AUTH)
                maxNumPreAuth = SIR_SDK_OPT_MAX_NUM_PRE_AUTH;
#endif //               
            if (pMac->lim.gLimNumPreAuthContexts == maxNumPreAuth)
            {
                /* 
                                                      
                                                     
                                           
                 */
                authFrame.authAlgoNumber =
                pRxAuthFrameBody->authAlgoNumber;
                authFrame.authTransactionSeqNumber =
                pRxAuthFrameBody->authTransactionSeqNumber + 1;
                authFrame.authStatusCode =
                eSIR_MAC_UNSPEC_FAILURE_STATUS;

                limSendAuthMgmtFrame(pMac, &authFrame,
                                     pHdr->sa,
                                     LIM_NO_WEP_IN_FC,psessionEntry);

                return;
            }
            //                                         
#ifdef WLAN_SOFTAP_FEATURE
            if (limIsAuthAlgoSupported(
                                      pMac,
                                      (tAniAuthType)
                                      pRxAuthFrameBody->authAlgoNumber, psessionEntry))
#else
            if (limIsAuthAlgoSupported(
                                      pMac,
                                      (tAniAuthType)
                                      pRxAuthFrameBody->authAlgoNumber))

#endif
            {
                switch (pRxAuthFrameBody->authAlgoNumber)
                {
                    case eSIR_OPEN_SYSTEM:
                        PELOGW(limLog(pMac, LOGW, FL("=======> eSIR_OPEN_SYSTEM  ...\n"));)
                        //                                            
                        pAuthNode = limAcquireFreePreAuthNode(pMac, &pMac->lim.gLimPreAuthTimerTable);
                        if (pAuthNode == NULL)
                        {
                            //          
                            limLog(pMac, LOGW,
                                   FL("Max pre-auth nodes reached "));
                            limPrintMacAddr(pMac, pHdr->sa, LOGW);

                            return;
                        }

                        PELOG1(limLog(pMac, LOG1, FL("Alloc new data: %x peer \n"), pAuthNode);
                        limPrintMacAddr(pMac, pHdr->sa, LOG1);)

                        palCopyMemory( pMac->hHdd,
                                     (tANI_U8 *) pAuthNode->peerMacAddr,
                                     pHdr->sa,
                                     sizeof(tSirMacAddr));

                        pAuthNode->mlmState =
                        eLIM_MLM_AUTHENTICATED_STATE;
                        pAuthNode->authType = (tAniAuthType)
                                              pRxAuthFrameBody->authAlgoNumber;
                        pAuthNode->fSeen = 0;
                        pAuthNode->fTimerStarted = 0;
                        limAddPreAuthNode(pMac, pAuthNode);

                        /* 
                                                                
                                       
                         */

                        authFrame.authAlgoNumber =
                        pRxAuthFrameBody->authAlgoNumber;
                        authFrame.authTransactionSeqNumber =
                        pRxAuthFrameBody->authTransactionSeqNumber + 1;
                        authFrame.authStatusCode = eSIR_MAC_SUCCESS_STATUS;
                        limSendAuthMgmtFrame(
                                            pMac, &authFrame,
                                            pHdr->sa,
                                            LIM_NO_WEP_IN_FC,psessionEntry);

                        //                             

                        palCopyMemory( pMac->hHdd,
                                     (tANI_U8 *) mlmAuthInd.peerMacAddr,
                                     (tANI_U8 *) pHdr->sa,
                                     sizeof(tSirMacAddr));
                        mlmAuthInd.authType = (tAniAuthType)
                                              pRxAuthFrameBody->authAlgoNumber;
                        mlmAuthInd.sessionId = psessionEntry->smeSessionId;

                        limPostSmeMessage(pMac,
                                          LIM_MLM_AUTH_IND,
                                          (tANI_U32 *) &mlmAuthInd);
                        break;

                    case eSIR_SHARED_KEY:
                        PELOGW(limLog(pMac, LOGW, FL("=======> eSIR_SHARED_KEY  ...\n"));)
#ifdef WLAN_SOFTAP_FEATURE
                        if(psessionEntry->limSystemRole == eLIM_AP_ROLE)
                        {
                            val = psessionEntry->privacy;
                        }
                        else   
#endif
                        if (wlan_cfgGetInt(pMac, WNI_CFG_PRIVACY_ENABLED,
                                      &val) != eSIR_SUCCESS)
                        {
                            /* 
                                                           
                                                   
                             */
                            limLog(pMac, LOGP,
                                   FL("could not retrieve Privacy option\n"));
                        }
                        cfgPrivacyOptImp = (tANI_U8)val;
                        if (!cfgPrivacyOptImp)
                        {
                            /* 
                                                              
                                           
                                                                     
                                                                       
                                    
                             */

                            authFrame.authAlgoNumber =
                            pRxAuthFrameBody->authAlgoNumber;
                            authFrame.authTransactionSeqNumber =
                            pRxAuthFrameBody->authTransactionSeqNumber + 1;
                            authFrame.authStatusCode =
                            eSIR_MAC_AUTH_ALGO_NOT_SUPPORTED_STATUS;

                            limSendAuthMgmtFrame(
                                                pMac, &authFrame,
                                                pHdr->sa,
                                                LIM_NO_WEP_IN_FC,psessionEntry);

                            //          
                            PELOGE(limLog(pMac, LOGE,
                                   FL("received Auth frame for unsupported auth algorithm %d "
                                   MAC_ADDRESS_STR), pRxAuthFrameBody->authAlgoNumber, 
                                   MAC_ADDR_ARRAY(pHdr->sa));)

                            return;
                        }
                        else
                        {
                            //                          
                            //                
                            pAuthNode = limAcquireFreePreAuthNode(pMac, &pMac->lim.gLimPreAuthTimerTable);
                            if (pAuthNode == NULL)
                            {
                                //          
                                limLog(pMac, LOGW,
                                       FL("Max pre-auth nodes reached "));
                                limPrintMacAddr(pMac, pHdr->sa, LOGW);

                                return;
                            }

                            palCopyMemory( pMac->hHdd,
                                         (tANI_U8 *) pAuthNode->peerMacAddr,
                                         pHdr->sa,
                                         sizeof(tSirMacAddr));

                            pAuthNode->mlmState =
                            eLIM_MLM_WT_AUTH_FRAME3_STATE;
                            pAuthNode->authType =
                            (tAniAuthType)
                            pRxAuthFrameBody->authAlgoNumber;
                            pAuthNode->fSeen = 0;
                            pAuthNode->fTimerStarted = 0;
                            limAddPreAuthNode(pMac, pAuthNode);

                            PELOG1(limLog(pMac, LOG1, FL("Alloc new data: %x id %d peer \n"),
                                          pAuthNode, pAuthNode->authNodeIdx);)
                            PELOG1(limPrintMacAddr(pMac, pHdr->sa, LOG1);)

                            //                                         
                            if (tx_timer_change_context(&pAuthNode->timer, pAuthNode->authNodeIdx) != TX_SUCCESS)
                            {
                                //                                      
                                //          
                                limLog(pMac, LOGP,
                                   FL("Unable to chg context auth response timer for peer "));
                                limPrintMacAddr(pMac, pHdr->sa, LOGP);

                                /* 
                                                                
                                                                   
                                 */

                                authFrame.authAlgoNumber =
                                        pRxAuthFrameBody->authAlgoNumber;
                                authFrame.authTransactionSeqNumber =
                                        pRxAuthFrameBody->authTransactionSeqNumber + 1;
                                authFrame.authStatusCode =
                                        eSIR_MAC_UNSPEC_FAILURE_STATUS;

                                limSendAuthMgmtFrame(pMac, &authFrame,
                                                     pHdr->sa,
                                                     LIM_NO_WEP_IN_FC,psessionEntry);

                                limDeletePreAuthNode(pMac, pHdr->sa);
                                return;
                            }

                            limActivateAuthRspTimer(pMac, pAuthNode);

                            pAuthNode->fTimerStarted = 1;

                            //                            
                            //               
                            //     
                            //                                                                                                                     
                            {
                               limLog(pMac, LOGE,FL("Challenge text preparation failed in limProcessAuthFrame"));
                            }
                            
                            pChallenge = pAuthNode->challengeText;

                            palCopyMemory( pMac->hHdd,
                                           pChallenge,
                                          (tANI_U8 *) challengeTextArray,
                                          sizeof(challengeTextArray));

                            /* 
                                                                          
                             */

                            authFrame.authAlgoNumber =
                            pRxAuthFrameBody->authAlgoNumber;
                            authFrame.authTransactionSeqNumber =
                            pRxAuthFrameBody->authTransactionSeqNumber + 1;
                            authFrame.authStatusCode =
                            eSIR_MAC_SUCCESS_STATUS;
                            authFrame.type   = SIR_MAC_CHALLENGE_TEXT_EID;
                            authFrame.length = SIR_MAC_AUTH_CHALLENGE_LENGTH;
                            palCopyMemory( pMac->hHdd,
                                         authFrame.challengeText,
                                         pAuthNode->challengeText,
                                         SIR_MAC_AUTH_CHALLENGE_LENGTH);

                            limSendAuthMgmtFrame(
                                                pMac, &authFrame,
                                                pHdr->sa,
                                                LIM_NO_WEP_IN_FC,psessionEntry);
                        } //                                                    

                        break;

                    default:
                        /* 
                                                                
                                                                
                                         
                                                                 
                                                                        
                         */

                        authFrame.authAlgoNumber =
                        pRxAuthFrameBody->authAlgoNumber;
                        authFrame.authTransactionSeqNumber =
                        pRxAuthFrameBody->authTransactionSeqNumber + 1;
                        authFrame.authStatusCode =
                        eSIR_MAC_AUTH_ALGO_NOT_SUPPORTED_STATUS;

                        limSendAuthMgmtFrame(
                                            pMac, &authFrame,
                                            pHdr->sa,
                                            LIM_NO_WEP_IN_FC,psessionEntry);

                        //          
                       PELOGE( limLog(pMac, LOGE,
                               FL("received Auth frame for unsupported auth algorithm %d "
                               MAC_ADDRESS_STR), pRxAuthFrameBody->authAlgoNumber, 
                               MAC_ADDR_ARRAY(pHdr->sa));)

                        return;
                } //                                             
            } //                                                              
            else
            {
                /* 
                                                        
                                                                       
                                                            
                 */
                authFrame.authAlgoNumber =
                pRxAuthFrameBody->authAlgoNumber;
                authFrame.authTransactionSeqNumber =
                pRxAuthFrameBody->authTransactionSeqNumber + 1;
                authFrame.authStatusCode =
                eSIR_MAC_AUTH_ALGO_NOT_SUPPORTED_STATUS;

                limSendAuthMgmtFrame(pMac, &authFrame,
                                     pHdr->sa,
                                     LIM_NO_WEP_IN_FC,psessionEntry);

                //          
                PELOGE(limLog(pMac, LOGE,
                       FL("received Authentication frame for unsupported auth algorithm %d "
                       MAC_ADDRESS_STR), pRxAuthFrameBody->authAlgoNumber, 
                       MAC_ADDR_ARRAY(pHdr->sa));)
                return;
            } //                                                                 
            break;

        case SIR_MAC_AUTH_FRAME_2:
            //            

            if (psessionEntry->limMlmState != eLIM_MLM_WT_AUTH_FRAME2_STATE)
            {
                /* 
                                                                         
                                                  
                 */

                //          
                PELOG1(limLog(pMac, LOG1,
                       FL("received Auth frame2 from peer in state %d, addr "),
                       psessionEntry->limMlmState);)
                PELOG1(limPrintMacAddr(pMac, pHdr->sa, LOG1);)

                return;
            }

            if ( !palEqualMemory( pMac->hHdd,(tANI_U8 *) pHdr->sa,
                          (tANI_U8 *) &pMac->lim.gpLimMlmAuthReq->peerMacAddr,
                          sizeof(tSirMacAddr)) )
            {
                /* 
                                                               
                                                        
                                                             
                 */

                //          
                PELOGW(limLog(pMac, LOGW,
                       FL("received Auth frame2 from unexpected peer "MAC_ADDRESS_STR),
                       MAC_ADDR_ARRAY(pHdr->sa));)

                break;
            }

            if (pRxAuthFrameBody->authStatusCode ==
                eSIR_MAC_AUTH_ALGO_NOT_SUPPORTED_STATUS)
            {
                /* 
                                                                             
                                                                  
                                                                         
                                                                     
                                                      
                 */
                if (pRxAuthFrameBody->authAlgoNumber !=
                    pMac->lim.gpLimMlmAuthReq->authType)
                {
                    pRxAuthFrameBody->authAlgoNumber =
                    pMac->lim.gpLimMlmAuthReq->authType;
                }
            }

            if (pRxAuthFrameBody->authAlgoNumber !=
                pMac->lim.gpLimMlmAuthReq->authType)
            {
                /* 
                                                             
                                                      
                                                             
                 */

                //          
                PELOGW(limLog(pMac, LOGW,
                       FL("received Auth frame2 for unexpected auth algo number %d "
                       MAC_ADDRESS_STR), pRxAuthFrameBody->authAlgoNumber, 
                       MAC_ADDR_ARRAY(pHdr->sa));)

                break;
            }

            if (pRxAuthFrameBody->authStatusCode ==
                eSIR_MAC_SUCCESS_STATUS)
            {
                if (pRxAuthFrameBody->authAlgoNumber ==
                    eSIR_OPEN_SYSTEM)
                {
                    psessionEntry->limCurrentAuthType = eSIR_OPEN_SYSTEM;

                    pAuthNode = limAcquireFreePreAuthNode(pMac, &pMac->lim.gLimPreAuthTimerTable);

                    if (pAuthNode == NULL)
                    {
                        //          
                        limLog(pMac, LOGW,
                               FL("Max pre-auth nodes reached "));
                        limPrintMacAddr(pMac, pHdr->sa, LOGW);

                        return;
                    }

                    PELOG1(limLog(pMac, LOG1, FL("Alloc new data: %x peer \n"), pAuthNode);)
                    PELOG1(limPrintMacAddr(pMac, pHdr->sa, LOG1);)

                    palCopyMemory( pMac->hHdd,
                                 (tANI_U8 *) pAuthNode->peerMacAddr,
                                 pMac->lim.gpLimMlmAuthReq->peerMacAddr,
                                 sizeof(tSirMacAddr));
                    pAuthNode->fTimerStarted = 0;
                    pAuthNode->authType = pMac->lim.gpLimMlmAuthReq->authType;
                    limAddPreAuthNode(pMac, pAuthNode);

                    limRestoreFromAuthState(pMac, eSIR_SME_SUCCESS,
                                            pRxAuthFrameBody->authStatusCode,psessionEntry);
                } //                                                          
                else
                {
                    //                          

#ifdef WLAN_SOFTAP_FEATURE
                    if(psessionEntry->limSystemRole == eLIM_AP_ROLE)
                    {
                        val = psessionEntry->privacy;
                    }
                    else   
#endif
                    if (wlan_cfgGetInt(pMac, WNI_CFG_PRIVACY_ENABLED,
                                  &val) != eSIR_SUCCESS)
                    {
                        /* 
                                                       
                                               
                         */
                        limLog(pMac, LOGP,
                               FL("could not retrieve Privacy option\n"));
                    }
                    cfgPrivacyOptImp = (tANI_U8)val;
                    if (!cfgPrivacyOptImp)
                    {
                        /* 
                                                                        
                                                                           
                                                                          
                         */

                        //          
                       PELOGE( limLog(pMac, LOGE,
                               FL("received Auth frame from peer for unsupported auth algo %d "
                               MAC_ADDRESS_STR), pRxAuthFrameBody->authAlgoNumber, 
                               MAC_ADDR_ARRAY(pHdr->sa));)

                        authFrame.authAlgoNumber =
                        pRxAuthFrameBody->authAlgoNumber;
                        authFrame.authTransactionSeqNumber =
                        pRxAuthFrameBody->authTransactionSeqNumber + 1;
                        authFrame.authStatusCode =
                        eSIR_MAC_AUTH_ALGO_NOT_SUPPORTED_STATUS;

                        limSendAuthMgmtFrame(pMac, &authFrame,
                                            pHdr->sa,
                                            LIM_NO_WEP_IN_FC,psessionEntry);
                        return;
                    }
                    else
                    {

                        if (pRxAuthFrameBody->type !=
                            SIR_MAC_CHALLENGE_TEXT_EID)
                        {
                            //          
                            PELOGE(limLog(pMac, LOGE,
                                   FL("received Auth frame with invalid challenge text IE\n"));)

                            return;
                        }

                        /* 
                                                                   
                                                                     
                         */
                        pKeyMapEntry = limLookUpKeyMappings(
                                                           pHdr->sa);

                        if (pKeyMapEntry)
                        {
                            if (pKeyMapEntry->key == NULL)
                            {
                                /* 
                                                                  
                                                       
                                                                
                                 */
                                authFrame.authAlgoNumber =
                                pRxAuthFrameBody->authAlgoNumber;
                                authFrame.authTransactionSeqNumber =
                                pRxAuthFrameBody->authTransactionSeqNumber + 1;
                                authFrame.authStatusCode =
                                eSIR_MAC_CHALLENGE_FAILURE_STATUS;

                                limSendAuthMgmtFrame(pMac, &authFrame,
                                                     pHdr->sa,
                                                     LIM_NO_WEP_IN_FC,psessionEntry);

                                //          
                                PELOGE(limLog(pMac, LOGE,
                                       FL("received Auth frame from peer when key mapping key is NULL"
                                       MAC_ADDRESS_STR),MAC_ADDR_ARRAY(pHdr->sa));)

                                limRestoreFromAuthState(pMac, eSIR_SME_NO_KEY_MAPPING_KEY_FOR_PEER,
                                                              eSIR_MAC_UNSPEC_FAILURE_REASON,psessionEntry);

                                return;
                            } //                               
                            else
                            {
                                ((tpSirMacAuthFrameBody) plainBody)->authAlgoNumber =
                                sirSwapU16ifNeeded(pRxAuthFrameBody->authAlgoNumber);
                                ((tpSirMacAuthFrameBody) plainBody)->authTransactionSeqNumber =
                                sirSwapU16ifNeeded((tANI_U16) (pRxAuthFrameBody->authTransactionSeqNumber + 1));
                                ((tpSirMacAuthFrameBody) plainBody)->authStatusCode = eSIR_MAC_SUCCESS_STATUS;
                                ((tpSirMacAuthFrameBody) plainBody)->type   = SIR_MAC_CHALLENGE_TEXT_EID;
                                ((tpSirMacAuthFrameBody) plainBody)->length = SIR_MAC_AUTH_CHALLENGE_LENGTH;
                                palCopyMemory( pMac->hHdd, (tANI_U8 *) ((tpSirMacAuthFrameBody) plainBody)->challengeText,
                                              pRxAuthFrameBody->challengeText,
                                              SIR_MAC_AUTH_CHALLENGE_LENGTH);

                                limEncryptAuthFrame(pMac, 0,
                                                    pKeyMapEntry->key,
                                                    plainBody,
                                                    encrAuthFrame,key_length);

                                psessionEntry->limMlmState = eLIM_MLM_WT_AUTH_FRAME4_STATE;
                                MTRACE(macTrace(pMac, TRACE_CODE_MLM_STATE, psessionEntry->peSessionId, psessionEntry->limMlmState));

                                limSendAuthMgmtFrame(pMac,
                                                     (tpSirMacAuthFrameBody) encrAuthFrame,
                                                     pHdr->sa,
                                                     LIM_WEP_IN_FC,psessionEntry);

                                break;
                            } //                                   
                        } //                  
                        else
                        {
                            if (wlan_cfgGetInt(pMac, WNI_CFG_WEP_DEFAULT_KEYID,
                                          &val) != eSIR_SUCCESS)
                            {
                                /* 
                                                              
                                                       
                                 */
                                limLog(pMac, LOGP,
                                       FL("could not retrieve Default keyId\n"));
                            }
                            keyId = (tANI_U8)val;

                            val = SIR_MAC_KEY_LENGTH;

#ifdef WLAN_SOFTAP_FEATURE  
                            if(psessionEntry->limSystemRole == eLIM_AP_ROLE)
                            {
                                tpSirKeys pKey;
                                pKey =  &psessionEntry->WEPKeyMaterial[keyId].key[0];
                                palCopyMemory( pMac->hHdd, defaultKey, pKey->key, pKey->keyLength);
                            }
                            else
#endif
                            if (wlan_cfgGetStr(pMac, (tANI_U16) (WNI_CFG_WEP_DEFAULT_KEY_1 + keyId),
                                          defaultKey,
                                          &val)
                                != eSIR_SUCCESS)
                            {
                                //                                     
                                //          
                                limLog(pMac, LOGP,
                                       FL("could not retrieve Default key\n"));

                                authFrame.authAlgoNumber =
                                pRxAuthFrameBody->authAlgoNumber;
                                authFrame.authTransactionSeqNumber =
                                pRxAuthFrameBody->authTransactionSeqNumber + 1;
                                authFrame.authStatusCode =
                                eSIR_MAC_CHALLENGE_FAILURE_STATUS;

                                limSendAuthMgmtFrame(
                                                    pMac, &authFrame,
                                                    pHdr->sa,
                                                    LIM_NO_WEP_IN_FC,psessionEntry);

                                limRestoreFromAuthState(pMac, eSIR_SME_INVALID_WEP_DEFAULT_KEY,
                                                              eSIR_MAC_UNSPEC_FAILURE_REASON,psessionEntry);

                                break;
                            }
                                key_length=val;
                                ((tpSirMacAuthFrameBody) plainBody)->authAlgoNumber =
                                sirSwapU16ifNeeded(pRxAuthFrameBody->authAlgoNumber);
                                ((tpSirMacAuthFrameBody) plainBody)->authTransactionSeqNumber =
                                sirSwapU16ifNeeded((tANI_U16) (pRxAuthFrameBody->authTransactionSeqNumber + 1));
                                ((tpSirMacAuthFrameBody) plainBody)->authStatusCode = eSIR_MAC_SUCCESS_STATUS;
                                ((tpSirMacAuthFrameBody) plainBody)->type   = SIR_MAC_CHALLENGE_TEXT_EID;
                                ((tpSirMacAuthFrameBody) plainBody)->length = SIR_MAC_AUTH_CHALLENGE_LENGTH;
                                palCopyMemory( pMac->hHdd, (tANI_U8 *) ((tpSirMacAuthFrameBody) plainBody)->challengeText,
                                              pRxAuthFrameBody->challengeText,
                                              SIR_MAC_AUTH_CHALLENGE_LENGTH);

                                limEncryptAuthFrame(pMac, keyId,
                                                    defaultKey,
                                                    plainBody,
                                                    encrAuthFrame,key_length);

                                psessionEntry->limMlmState =
                                eLIM_MLM_WT_AUTH_FRAME4_STATE;
                                MTRACE(macTrace(pMac, TRACE_CODE_MLM_STATE, psessionEntry->peSessionId, psessionEntry->limMlmState));

                                limSendAuthMgmtFrame(pMac,
                                                     (tpSirMacAuthFrameBody) encrAuthFrame,
                                                     pHdr->sa,
                                                     LIM_WEP_IN_FC,psessionEntry);

                                break;
                        } //                      
                    } //                                                         
                } //                                                              
            } //                                                                 
            else
            {
                /* 
                                          
                                                                        
                 */

                //          
                PELOGE(limLog(pMac, LOGE,
                       FL("received Auth frame from peer with failure code %d "
                       MAC_ADDRESS_STR), pRxAuthFrameBody->authStatusCode, 
                       MAC_ADDR_ARRAY(pHdr->sa));)

                limRestoreFromAuthState(pMac, eSIR_SME_AUTH_REFUSED,
                                              pRxAuthFrameBody->authStatusCode,psessionEntry);
            } //                                                                     

            break;

        case SIR_MAC_AUTH_FRAME_3:
            //            

            if (pRxAuthFrameBody->authAlgoNumber != eSIR_SHARED_KEY)
            {
                /* 
                                                                           
                                                                          
                                                      
                 */
                authFrame.authAlgoNumber = eSIR_SHARED_KEY;
                authFrame.authTransactionSeqNumber =
                SIR_MAC_AUTH_FRAME_4;
                authFrame.authStatusCode =
                eSIR_MAC_AUTH_FRAME_OUT_OF_SEQ_STATUS;

                limSendAuthMgmtFrame(pMac, &authFrame,
                                     pHdr->sa,
                                     LIM_NO_WEP_IN_FC,psessionEntry);

                //          
                PELOGE(limLog(pMac, LOGE,
                       FL("received Auth frame3 from peer with auth algo number %d "
                       MAC_ADDRESS_STR), pRxAuthFrameBody->authAlgoNumber, 
                       MAC_ADDR_ARRAY(pHdr->sa));)

                return;
            }

            if (psessionEntry->limSystemRole == eLIM_AP_ROLE || psessionEntry->limSystemRole == eLIM_BT_AMP_AP_ROLE ||
                psessionEntry->limSystemRole == eLIM_STA_IN_IBSS_ROLE)
            {
                /* 
                                                              
                                                         
                                                   
                 */
                if (!pHdr->fc.wep)
                {
                    //                                         
                    authFrame.authAlgoNumber = eSIR_SHARED_KEY;
                    authFrame.authTransactionSeqNumber =
                    SIR_MAC_AUTH_FRAME_4;
                    authFrame.authStatusCode =
                    eSIR_MAC_CHALLENGE_FAILURE_STATUS;

                    limSendAuthMgmtFrame(pMac, &authFrame,
                                         pHdr->sa,
                                         LIM_NO_WEP_IN_FC,psessionEntry);

                    //          
                    PELOGE(limLog(pMac, LOGE,
                           FL("received Auth frame3 from peer with no WEP bit set "MAC_ADDRESS_STR),
                           MAC_ADDR_ARRAY(pHdr->sa));)

                    return;
                }

                pAuthNode = limSearchPreAuthList(pMac,
                                                pHdr->sa);
                if (pAuthNode == NULL)
                {
                    /* 
                                                       
                                                           
                              
                                                              
                                   
                     */
                    authFrame.authAlgoNumber = eSIR_SHARED_KEY;
                    authFrame.authTransactionSeqNumber =
                    SIR_MAC_AUTH_FRAME_4;
                    authFrame.authStatusCode =
                    eSIR_MAC_AUTH_FRAME_OUT_OF_SEQ_STATUS;

                    limSendAuthMgmtFrame(pMac, &authFrame,
                                         pHdr->sa,
                                         LIM_NO_WEP_IN_FC,psessionEntry);

                    //          
                    PELOGE(limLog(pMac, LOGW,
                           FL("received AuthFrame3 from peer that has no preauth context "
                           MAC_ADDRESS_STR), MAC_ADDR_ARRAY(pHdr->sa));)

                    return;
                }

                if (pAuthNode->mlmState == eLIM_MLM_AUTH_RSP_TIMEOUT_STATE)
                {
                    /* 
                                                                        
                                                         
                                                        
                     */
                    authFrame.authAlgoNumber = eSIR_SHARED_KEY;
                    authFrame.authTransactionSeqNumber =
                    SIR_MAC_AUTH_FRAME_4;
                    authFrame.authStatusCode =
                    eSIR_MAC_AUTH_RSP_TIMEOUT_STATUS;

                    limSendAuthMgmtFrame(
                                        pMac, &authFrame,
                                        pHdr->sa,
                                        LIM_NO_WEP_IN_FC,psessionEntry);

                    //          
                    limLog(pMac, LOGW,
                           FL("auth response timer timedout for peer "));
                    limPrintMacAddr(pMac, pHdr->sa, LOGW);

                    //                                
                    limDeletePreAuthNode(pMac,
                                         pHdr->sa);

                    return;
                } //                                 

                if (pRxAuthFrameBody->authStatusCode != eSIR_MAC_SUCCESS_STATUS)
                {
                    /* 
                                                                        
                                                                           
                                             
                     */

                    //          
                    PELOGE(limLog(pMac, LOGE,
                           FL("received Auth frame3 from peer with status code %d "
                           MAC_ADDRESS_STR), pRxAuthFrameBody->authStatusCode, 
                           MAC_ADDR_ARRAY(pHdr->sa));)

                    return;
                }

                /* 
                                                                          
                                        
                 */

                if (palEqualMemory( pMac->hHdd,pRxAuthFrameBody->challengeText,
                              pAuthNode->challengeText,
                              SIR_MAC_AUTH_CHALLENGE_LENGTH))
                {
                    //                                        

                    //                                                 
                    limDeactivateAndChangePerStaIdTimer(pMac,
                                                        eLIM_AUTH_RSP_TIMER,
                                                        pAuthNode->authNodeIdx);

                    pAuthNode->fTimerStarted = 0;
                    pAuthNode->mlmState = eLIM_MLM_AUTHENTICATED_STATE;

                    /* 
                                                                             
                     */
                    authFrame.authAlgoNumber = eSIR_SHARED_KEY;
                    authFrame.authTransactionSeqNumber =
                    SIR_MAC_AUTH_FRAME_4;
                    authFrame.authStatusCode = eSIR_MAC_SUCCESS_STATUS;

                    limSendAuthMgmtFrame(pMac, &authFrame,
                                         pHdr->sa,
                                         LIM_NO_WEP_IN_FC,psessionEntry);

                    //                             
                    palCopyMemory( pMac->hHdd,
                                 (tANI_U8 *) mlmAuthInd.peerMacAddr,
                                 (tANI_U8 *) pHdr->sa,
                                 sizeof(tSirMacAddr));
                    mlmAuthInd.authType = (tAniAuthType)
                                          pRxAuthFrameBody->authAlgoNumber;
                    mlmAuthInd.sessionId = psessionEntry->smeSessionId;

                    limPostSmeMessage(pMac,
                                      LIM_MLM_AUTH_IND,
                                      (tANI_U32 *) &mlmAuthInd);

                    break;
                }
                else
                {
                    /* 
                                         
                                                                          
                                                                          
                                          
                     */

                    authFrame.authAlgoNumber =
                    pRxAuthFrameBody->authAlgoNumber;
                    authFrame.authTransactionSeqNumber =
                    SIR_MAC_AUTH_FRAME_4;
                    authFrame.authStatusCode =
                    eSIR_MAC_CHALLENGE_FAILURE_STATUS;

                    limSendAuthMgmtFrame(pMac, &authFrame,
                                         pHdr->sa,
                                         LIM_NO_WEP_IN_FC,psessionEntry);

                    //          
                   PELOGE( limLog(pMac, LOGW,
                           FL("Challenge failure for peer "MAC_ADDRESS_STR), 
						   MAC_ADDR_ARRAY(pHdr->sa));)
                    return;
                }
            } //                                                    

            break;

        case SIR_MAC_AUTH_FRAME_4:
            //            
            if (psessionEntry->limMlmState != eLIM_MLM_WT_AUTH_FRAME4_STATE)
            {
                /* 
                                                                         
                                                  
                 */

                //          
                PELOG1(limLog(pMac, LOG1,
                       FL("received unexpected Auth frame4 from peer in state %d, addr "),
                       psessionEntry->limMlmState);)
               PELOG1( limPrintMacAddr(pMac, pHdr->sa, LOG1);)

                return;
            }

            if (pRxAuthFrameBody->authAlgoNumber != eSIR_SHARED_KEY)
            {
                /* 
                                                                           
                                                  
                                                                           
                                  
                 */

                //          
                PELOGE(limLog(pMac, LOGE,
                       FL("received Auth frame4 from peer with invalid auth algo %d "
                       MAC_ADDRESS_STR), pRxAuthFrameBody->authAlgoNumber, 
                       MAC_ADDR_ARRAY(pHdr->sa));)

                return;
            }

            if ( !palEqualMemory( pMac->hHdd,(tANI_U8 *) pHdr->sa,
                          (tANI_U8 *) &pMac->lim.gpLimMlmAuthReq->peerMacAddr,
                          sizeof(tSirMacAddr)) )
            {
                /* 
                                                               
                                                                 
                                                             
                 */

                //          
                PELOGE(limLog(pMac, LOGW,
                       FL("received Auth frame4 from unexpected peer "
                       MAC_ADDRESS_STR), MAC_ADDR_ARRAY(pHdr->sa));)

                break;
            }

            if (pRxAuthFrameBody->authAlgoNumber !=
                pMac->lim.gpLimMlmAuthReq->authType)
            {
                /* 
                                                                       
                                            
                                                             
                 */

                PELOGE(limLog(pMac, LOGE,
                       FL("received Authentication frame from peer with invalid auth seq number %d "
                       MAC_ADDRESS_STR), pRxAuthFrameBody->authTransactionSeqNumber, 
                       MAC_ADDR_ARRAY(pHdr->sa));)

                break;
            }

            if (pRxAuthFrameBody->authStatusCode ==
                eSIR_MAC_SUCCESS_STATUS)
            {
                /* 
                                           
                                      
                 */
                psessionEntry->limCurrentAuthType = eSIR_SHARED_KEY;

                pAuthNode = limAcquireFreePreAuthNode(pMac, &pMac->lim.gLimPreAuthTimerTable);
                if (pAuthNode == NULL)
                {
                    //          
                    limLog(pMac, LOGW,
                           FL("Max pre-auth nodes reached "));
                    limPrintMacAddr(pMac, pHdr->sa, LOGW);

                    return;
                }
                PELOG1(limLog(pMac, LOG1, FL("Alloc new data: %x peer \n"), pAuthNode);
                limPrintMacAddr(pMac, pHdr->sa, LOG1);)

                palCopyMemory( pMac->hHdd,
                             (tANI_U8 *) pAuthNode->peerMacAddr,
                             pMac->lim.gpLimMlmAuthReq->peerMacAddr,
                             sizeof(tSirMacAddr));
                pAuthNode->fTimerStarted = 0;
                pAuthNode->authType = pMac->lim.gpLimMlmAuthReq->authType;
                limAddPreAuthNode(pMac, pAuthNode);

                limRestoreFromAuthState(pMac, eSIR_SME_SUCCESS,
                                              pRxAuthFrameBody->authStatusCode,psessionEntry);

            } //                                                                 
            else
            {
                /* 
                                          
                                                                        
                 */

                //          
                PELOGE(limLog(pMac, LOGE, FL("Authentication failure from peer "
                       MAC_ADDRESS_STR), MAC_ADDR_ARRAY(pHdr->sa));)

                limRestoreFromAuthState(pMac, eSIR_SME_AUTH_REFUSED,
                                              pRxAuthFrameBody->authStatusCode,psessionEntry);
            } //                                       

            break;

        default:
            //                                                   

            //          
            PELOGE(limLog(pMac, LOGE,
                   FL("received Auth frame from peer with invalid auth seq number %d "
                   MAC_ADDRESS_STR), pRxAuthFrameBody->authTransactionSeqNumber, 
                   MAC_ADDR_ARRAY(pHdr->sa));)

            break;
    } //                                                        
} /*                               */





#ifdef WLAN_FEATURE_VOWIFI_11R

/*                                                                      
  
                                                                       
                                            
                                       
  
                                                                             
                                                                   
                                                        
                                                                        
 */
int limProcessAuthFrameNoSession(tpAniSirGlobal pMac, tANI_U8 *pBd, void *body)
{
    tpSirMacMgmtHdr pHdr;
    tpPESession psessionEntry = NULL;
    tANI_U8 *pBody;
    tANI_U16  frameLen;
    tSirMacAuthFrameBody rxAuthFrame;
    tSirMacAuthFrameBody *pRxAuthFrameBody = NULL;
    int ret_status = eSIR_FAILURE;

    pHdr = WDA_GET_RX_MAC_HEADER(pBd);
    pBody = WDA_GET_RX_MPDU_DATA(pBd);
    frameLen = WDA_GET_RX_PAYLOAD_LEN(pBd);

    //                                                                    
    psessionEntry = pMac->ft.ftPEContext.psavedsessionEntry;
    if (psessionEntry == NULL) 
    {
        limLog(pMac, LOGW, FL("Error: Unable to find session id while in pre-auth phase for FT"));
        return eSIR_FAILURE;
    }

    if (pMac->ft.ftPEContext.pFTPreAuthReq == NULL)
    {
        //                   
        return eSIR_FAILURE;
    }

    if (frameLen == 0) 
    {
        return eSIR_FAILURE;
    }
#ifdef WLAN_FEATURE_VOWIFI_11R_DEBUG
    limPrintMacAddr(pMac, pHdr->bssId, LOGE);
    limPrintMacAddr(pMac, pMac->ft.ftPEContext.pFTPreAuthReq->preAuthbssId, LOGE);
    limLog(pMac,LOG2,FL("seqControl 0x%X\n"), 
            ((pHdr->seqControl.seqNumHi << 8) | 
            (pHdr->seqControl.seqNumLo << 4) |
            (pHdr->seqControl.fragNum)));
#endif

    //                                                  
    if (!palEqualMemory( pMac->hHdd, pMac->ft.ftPEContext.pFTPreAuthReq->preAuthbssId,
        pHdr->bssId, sizeof( tSirMacAddr )))
    {
        //                                            
        //                           
        return eSIR_FAILURE;
    }

    if (eANI_BOOLEAN_TRUE ==
        pMac->ft.ftPEContext.pFTPreAuthReq->bPreAuthRspProcessed)
    {
        /*
                                                                    
                                                                   
                
                                                              
                                                               
                                                              
                                                                   
                                                                  
                                                                 
                                                                 
                                                                
                                          
                                                                  
                                                                    
                    
         */
        PELOGE(limLog(pMac,LOGE,"Auth rsp already posted to SME"
               " (session %p, FT session %p)\n", psessionEntry,
               pMac->ft.ftPEContext.pftSessionEntry););
        return eSIR_SUCCESS;
    }
    else
    {
        PELOGE(limLog(pMac,LOGE,"Auth rsp not yet posted to SME"
               " (session %p, FT session %p)\n", psessionEntry,
               pMac->ft.ftPEContext.pftSessionEntry););
        pMac->ft.ftPEContext.pFTPreAuthReq->bPreAuthRspProcessed =
            eANI_BOOLEAN_TRUE;
    }

#ifdef WLAN_FEATURE_VOWIFI_11R_DEBUG
    limLog(pMac, LOGE, FL("Pre-Auth response received from neighbor"));
    limLog(pMac, LOGE, FL("Pre-Auth done state"));
#endif
    //                                                         
    //               
    limDeactivateAndChangeTimer(pMac, eLIM_FT_PREAUTH_RSP_TIMER);


    //                        
    if ((sirConvertAuthFrame2Struct(pMac, pBody, frameLen, &rxAuthFrame) != eSIR_SUCCESS))
    {
        limHandleFTPreAuthRsp(pMac, eSIR_FAILURE, NULL, 0, psessionEntry);
        return eSIR_FAILURE;
    }
    pRxAuthFrameBody = &rxAuthFrame;

#ifdef WLAN_FEATURE_VOWIFI_11R_DEBUG
    PELOGE(limLog(pMac, LOGE,
           FL("Received Auth frame with type=%d seqnum=%d, status=%d (%d)\n"),
           (tANI_U32) pRxAuthFrameBody->authAlgoNumber,
           (tANI_U32) pRxAuthFrameBody->authTransactionSeqNumber,
           (tANI_U32) pRxAuthFrameBody->authStatusCode,(tANI_U32)pMac->lim.gLimNumPreAuthContexts);)
#endif

    switch (pRxAuthFrameBody->authTransactionSeqNumber)
    {
        case SIR_MAC_AUTH_FRAME_2:
            if (pRxAuthFrameBody->authStatusCode != eSIR_MAC_SUCCESS_STATUS)
            {
#ifdef WLAN_FEATURE_VOWIFI_11R_DEBUG
                PELOGE(limLog( pMac, LOGE, "Auth status code received is  %d\n", 
                    (tANI_U32) pRxAuthFrameBody->authStatusCode);)
#endif
            }
            else 
            {
                ret_status = eSIR_SUCCESS;
            }
            break;

        default:
#ifdef WLAN_FEATURE_VOWIFI_11R_DEBUG
            PELOGE(limLog( pMac, LOGE, "Seq. no incorrect expected 2 received %d\n", 
                (tANI_U32) pRxAuthFrameBody->authTransactionSeqNumber);)
#endif
            break;
    }

    //                              
    limHandleFTPreAuthRsp(pMac, ret_status, pBody, frameLen, psessionEntry);

    return ret_status;
}

#endif /*                         */

