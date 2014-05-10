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

#include "aniGlobal.h"
#include "wniApi.h"

#include "sirCommon.h"
#if (WNI_POLARIS_FW_PRODUCT == AP)
#include "wniCfgAp.h"
#else
#include "wniCfgSta.h"
#endif
#include "cfgApi.h"

#ifdef FEATURE_WLAN_NON_INTEGRATED_SOC
#include "halDataStruct.h"
#include "halCommonApi.h"
#endif

#include "utilsApi.h"
#include "limUtils.h"
#include "limSecurityUtils.h"
#include "limSession.h"


#define LIM_SEED_LENGTH 16

/* 
                           
  
           
                                                            
                                                                  
         
  
        
  
              
     
  
       
     
  
                                                          
                                                        
                                                           
                                                            
                           
  
                                                        
 */
#ifdef WLAN_SOFTAP_FEATURE
tANI_U8
limIsAuthAlgoSupported(tpAniSirGlobal pMac, tAniAuthType authType, tpPESession psessionEntry)
#else
tANI_U8
limIsAuthAlgoSupported(tpAniSirGlobal pMac, tAniAuthType authType)
#endif
{
    tANI_U32 algoEnable, privacyOptImp;

    if (authType == eSIR_OPEN_SYSTEM)
    {

#ifdef WLAN_SOFTAP_FEATURE
        if(psessionEntry->limSystemRole == eLIM_AP_ROLE)
        {
           if((psessionEntry->authType == eSIR_OPEN_SYSTEM) || (psessionEntry->authType == eSIR_AUTO_SWITCH))
              return true;
           else
              return false; 
        }
#endif

        if (wlan_cfgGetInt(pMac, WNI_CFG_OPEN_SYSTEM_AUTH_ENABLE,
                      &algoEnable) != eSIR_SUCCESS)
        {
            /* 
                                                   
                                   
               */
            limLog(pMac, LOGE,
                   FL("could not retrieve AuthAlgo1 Enable value\n"));

            return false;
        }
        else
            return ( (algoEnable > 0 ? true : false) );
    }
    else
    {

#ifdef WLAN_SOFTAP_FEATURE
        if(psessionEntry->limSystemRole == eLIM_AP_ROLE)
        {
            if((psessionEntry->authType == eSIR_SHARED_KEY) || (psessionEntry->authType == eSIR_AUTO_SWITCH))
                algoEnable = true;
            else
                algoEnable = false;
            
        }
        else
#endif

        if (wlan_cfgGetInt(pMac, WNI_CFG_SHARED_KEY_AUTH_ENABLE,
                      &algoEnable) != eSIR_SUCCESS)
        {
            /* 
                                                   
                                   
             */
            limLog(pMac, LOGE,
                   FL("could not retrieve AuthAlgo2 Enable value\n"));

            return false;
        }

#ifdef WLAN_SOFTAP_FEATURE
        if(psessionEntry->limSystemRole == eLIM_AP_ROLE)
        {
            privacyOptImp = psessionEntry->privacy;
        }
        else
#endif

        if (wlan_cfgGetInt(pMac, WNI_CFG_PRIVACY_ENABLED,
                      &privacyOptImp) != eSIR_SUCCESS)
        {
            /* 
                                                           
                                   
             */
            limLog(pMac, LOGE,
               FL("could not retrieve PrivacyOptImplemented value\n"));

            return false;
        }
            return (algoEnable && privacyOptImp);
    }
} /*                                        */



/* 
                     
  
           
                                                     
                                                                    
                                                                  
                
  
        
  
              
  
       
  
                                                 
               
 */

void
limInitPreAuthList(tpAniSirGlobal pMac)
{
    pMac->lim.pLimPreAuthList = NULL;

#if (WNI_POLARIS_FW_PRODUCT == AP)
    if (pMac->lim.gLimSystemRole == eLIM_AP_ROLE )
    {
        tANI_U32 authClnupTimeout;
        //                  

        if (wlan_cfgGetInt(pMac, WNI_CFG_PREAUTH_CLNUP_TIMEOUT,
                      &authClnupTimeout) != eSIR_SUCCESS)
        {
            /* 
                                                      
                                   
             */
            limLog(pMac, LOGE,
               FL("could not retrieve PreAuthClnupTimeout value\n"));

            return;
        }
        authClnupTimeout = SYS_MS_TO_TICKS(authClnupTimeout);

        //                                                            
        if (tx_timer_create(&pMac->lim.limTimers.gLimPreAuthClnupTimer,
                            "preAuthCleanup",
                            limTimerHandler,
                            SIR_LIM_PREAUTH_CLNUP_TIMEOUT,
                            authClnupTimeout, authClnupTimeout,
                            TX_AUTO_ACTIVATE) != TX_SUCCESS)
        {
            //                                        
            //          
            limLog(pMac, LOGP, FL("could not create PreAuthCleanup timer\n"));

            return;
        }

#if defined(ANI_OS_TYPE_RTAI_LINUX)
        tx_timer_set_expiry_list(&pMac->lim.limTimers.gLimPreAuthClnupTimer,
                                 LIM_TIMER_EXPIRY_LIST);
#endif
        PELOG1(limLog(pMac, LOG1,
               FL("Created pre-auth cleanup timer\n"));)

    }
#endif
} /*                              */



/* 
                       
  
           
                                                          
                                                      
  
        
  
              
  
       
  
                                                 
               
 */

void
limDeletePreAuthList(tpAniSirGlobal pMac)
{
    struct tLimPreAuthNode    *pCurrNode, *pTempNode;

    pCurrNode = pTempNode = pMac->lim.pLimPreAuthList;
    while (pCurrNode != NULL)
    {
        pTempNode = pCurrNode->next;

        PELOG1(limLog(pMac, LOG1, FL("=====> limDeletePreAuthList \n"));)
        limReleasePreAuthNode(pMac, pCurrNode);

        pCurrNode = pTempNode;
    }
    pMac->lim.pLimPreAuthList = NULL;
} /*                                */



/* 
                       
  
           
                                                                
                                                                  
                                                            
                        
  
        
  
              
  
       
  
                                                     
                                              
  
                                                       
 */

struct tLimPreAuthNode *
limSearchPreAuthList(tpAniSirGlobal pMac, tSirMacAddr macAddr)
{
    struct tLimPreAuthNode    *pTempNode = pMac->lim.pLimPreAuthList;

    while (pTempNode != NULL)
    {
        if (palEqualMemory( pMac->hHdd,(tANI_U8 *) macAddr,
                      (tANI_U8 *) &pTempNode->peerMacAddr,
                      sizeof(tSirMacAddr)) )
            break;

        pTempNode = pTempNode->next;
    }

    return pTempNode;
} /*                                */



/* 
                    
  
           
                                                             
          
                                                                    
                        
  
        
                                                
  
              
  
       
  
                                                 
                                                                        
  
               
 */

void
limAddPreAuthNode(tpAniSirGlobal pMac, struct tLimPreAuthNode *pAuthNode)
{
    pMac->lim.gLimNumPreAuthContexts++;

    pAuthNode->next = pMac->lim.pLimPreAuthList;

    pMac->lim.pLimPreAuthList = pAuthNode;
} /*                             */


/* 
                        
  
           
                                                    
                           
  
        
  
              
  
       
  
                                                 
                                                              
               
 */

void
limReleasePreAuthNode(tpAniSirGlobal pMac, tpLimPreAuthNode pAuthNode)
{
    pAuthNode->fFree = 1;
    MTRACE(macTrace(pMac, TRACE_CODE_TIMER_DEACTIVATE, NO_SESSION, eLIM_PRE_AUTH_CLEANUP_TIMER));
    tx_timer_deactivate(&pAuthNode->timer);                
    pMac->lim.gLimNumPreAuthContexts--;
} /*                                 */


/* 
                       
  
           
                                                                
                                                               
                         
                                                                     
                                                     
  
        
  
              
  
       
  
                                                 
                                                                       
                                                 
  
               
 */

void
limDeletePreAuthNode(tpAniSirGlobal pMac, tSirMacAddr macAddr)
{
    struct tLimPreAuthNode    *pPrevNode, *pTempNode;

    pTempNode = pPrevNode = pMac->lim.pLimPreAuthList;

    if (pTempNode == NULL)
        return;

    if (palEqualMemory( pMac->hHdd,(tANI_U8 *) macAddr,
                  (tANI_U8 *) &pTempNode->peerMacAddr,
                  sizeof(tSirMacAddr)) )
    {
        //                         

        pMac->lim.pLimPreAuthList = pTempNode->next;

#if (WNI_POLARIS_FW_PRODUCT == AP)
        //                                          
        if (pTempNode->fTimerStarted)
            limDeactivateAndChangePerStaIdTimer(pMac,
                                                eLIM_AUTH_RSP_TIMER,
                                                pTempNode->authNodeIdx);

#endif

        PELOG1(limLog(pMac, LOG1, FL("=====> limDeletePreAuthNode : first node to delete\n"));)
        PELOG1(limLog(pMac, LOG1, FL("Release data entry: %x id %d peer \n"),
                        pTempNode, pTempNode->authNodeIdx);
        limPrintMacAddr(pMac, macAddr, LOG1);)
        limReleasePreAuthNode(pMac, pTempNode);

        return;
    }

    pTempNode = pTempNode->next;

    while (pTempNode != NULL)
    {
        if (palEqualMemory( pMac->hHdd,(tANI_U8 *) macAddr,
                      (tANI_U8 *) &pTempNode->peerMacAddr,
                      sizeof(tSirMacAddr)) )
        {
            //                         

            pPrevNode->next = pTempNode->next;

#if (WNI_POLARIS_FW_PRODUCT == AP)
            //                                          
            if (pTempNode->fTimerStarted)
                limDeactivateAndChangePerStaIdTimer(pMac,
                                                    eLIM_AUTH_RSP_TIMER,
                                                    pTempNode->authNodeIdx);
#endif
            PELOG1(limLog(pMac, LOG1, FL("=====> limDeletePreAuthNode : subsequent node to delete\n"));
            limLog(pMac, LOG1, FL("Release data entry: %x id %d peer \n"),
                         pTempNode, pTempNode->authNodeIdx);
            limPrintMacAddr(pMac, macAddr, LOG1);)
            limReleasePreAuthNode(pMac, pTempNode);

            return;
        }

        pPrevNode = pTempNode;
        pTempNode = pTempNode->next;
    }

    //                   
    //          
    limLog(pMac, LOGP, FL("peer not found in pre-auth list, addr= "));
    limPrintMacAddr(pMac, macAddr, LOGP);

} /*                                */


#if (WNI_POLARIS_FW_PRODUCT == AP)
/* 
                         
  
           
                                                                
                   
  
        
                                                               
                                                                 
                          
  
              
  
       
  
                                                 
               
 */

void
limPreAuthClnupHandler(tpAniSirGlobal pMac)
{
    tANI_U16              aid;
    tANI_U8             firstNode=false;
    tpDphHashNode    pStaDs;
    struct tLimPreAuthNode  *pPrevNode, *pCurrNode;

#ifdef GEN6_TODO
    //                                             
    //                 
    tpPESession sessionEntry;

    if((sessionEntry = peFindSessionBySessionId(pMac, pMac->lim.limTimers.gLimPreAuthClnupTimer.sessionId))== NULL) 
    {
        limLog(pMac, LOGP,FL("Session Does not exist for given sessionID\n"));
        return;
    }
#endif

    pCurrNode = pPrevNode = pMac->lim.pLimPreAuthList;

    while (pCurrNode != NULL)
    {
        if (pCurrNode->fSeen)
        {
            //                         

            if (pCurrNode == pMac->lim.pLimPreAuthList)
            {
                //                         
                pMac->lim.pLimPreAuthList = pPrevNode = pCurrNode->next;
                firstNode = true;
            }
            else
            {
                pPrevNode->next = pCurrNode->next;
            }

            //                                          
            if (pCurrNode->fTimerStarted)
                limDeactivateAndChangePerStaIdTimer(pMac,
                                                    eLIM_AUTH_RSP_TIMER,
                                                    pCurrNode->authNodeIdx);

            pStaDs = dphLookupHashEntry(pMac,
                                        pCurrNode->peerMacAddr,
                                        &aid);

            if (!pStaDs)
            {
                /* 
                                                        
                                                       
                                       
                 */
                limSendDeauthMgmtFrame(
                               pMac,
                               eSIR_MAC_PREV_AUTH_NOT_VALID_REASON, //  
                               pCurrNode->peerMacAddr,sessionEntry);
            }

            limLog(pMac,
                   LOG3,
                   FL("Release preAuth node during periodic cleanup\n"));
            limReleasePreAuthNode(pMac, pCurrNode);

            if (firstNode)
            {
                //                       
                if (pMac->lim.pLimPreAuthList == NULL)
                    break;

                pCurrNode = pMac->lim.pLimPreAuthList;
                firstNode = false;
            }
            else
            {
                pCurrNode = pPrevNode->next;
            }
        }
        else
        {
            //                                                   
            pCurrNode->fSeen = 1;

            pPrevNode = pCurrNode;
            pCurrNode = pCurrNode->next;
        }
    }
} /*                                  */
#endif



/* 
                             
  
           
                                                            
                                                          
            
  
        
                                                          
                                      
                                                      
                                   
  
              
  
       
  
                                                       
                                                        
               
 */

void
limRestoreFromAuthState(tpAniSirGlobal pMac, tSirResultCodes resultCode, tANI_U16 protStatusCode,tpPESession sessionEntry)
{
    tSirMacAddr     currentBssId;
    tLimMlmAuthCnf  mlmAuthCnf;

    palCopyMemory( pMac->hHdd, (tANI_U8 *) &mlmAuthCnf.peerMacAddr,
                  (tANI_U8 *) &pMac->lim.gpLimMlmAuthReq->peerMacAddr,
                  sizeof(tSirMacAddr));
    mlmAuthCnf.authType   = pMac->lim.gpLimMlmAuthReq->authType;
    mlmAuthCnf.resultCode = resultCode;
    mlmAuthCnf.protStatusCode = protStatusCode;
    
    /*                     */
    mlmAuthCnf.sessionId = sessionEntry->peSessionId;

    //                          
    //                              
    palFreeMemory( pMac->hHdd, pMac->lim.gpLimMlmAuthReq);
    pMac->lim.gpLimMlmAuthReq = NULL;

    sessionEntry->limMlmState = sessionEntry->limPrevMlmState;
    
    MTRACE(macTrace(pMac, TRACE_CODE_MLM_STATE, sessionEntry->peSessionId, sessionEntry->limMlmState));


    //                                      
    limDeactivateAndChangeTimer(pMac, eLIM_AUTH_FAIL_TIMER);

    #if 0
    if (wlan_cfgGetStr(pMac, WNI_CFG_BSSID, currentBssId, &cfg) != eSIR_SUCCESS)
    {
        //                                          
        limLog(pMac, LOGP, FL("could not retrieve BSSID\n"));
    }
    #endif //                 
    sirCopyMacAddr(currentBssId,sessionEntry->bssId);

    if (sessionEntry->limSmeState == eLIM_SME_WT_PRE_AUTH_STATE)
    {
        pMac->lim.gLimPreAuthChannelNumber = 0;
    }

    limPostSmeMessage(pMac,
                      LIM_MLM_AUTH_CNF,
                      (tANI_U32 *) &mlmAuthCnf);
} /*                                   */



/* 
                         
  
           
                                                            
                                                             
               
  
        
  
              
     
  
       
     
  
                                                                  
                                                         
  
                                                                    
 */

tCfgWepKeyEntry *
limLookUpKeyMappings(tSirMacAddr macAddr)
{
    return NULL;
} /*                                      */



/* 
                        
  
           
                                                            
                                         
  
        
  
              
     
  
       
     
  
                                                         
                                        
                                                                      
                                                             
                                                                  
                                                  
               
 */

void
limEncryptAuthFrame(tpAniSirGlobal pMac, tANI_U8 keyId, tANI_U8 *pKey, tANI_U8 *pPlainText,
                    tANI_U8 *pEncrBody, tANI_U32 keyLength)
{
    tANI_U8  seed[LIM_SEED_LENGTH], icv[SIR_MAC_WEP_ICV_LENGTH];

    keyLength += 3;

    //                        
    //                                                            
    halGetTxTSFtimer(pMac, (tSirMacTimeStamp *) &seed);

    //                         
    palCopyMemory( pMac->hHdd, (tANI_U8 *) &seed[3], pKey, keyLength - 3);

    //                                                            
    limComputeCrc32(icv, pPlainText, sizeof(tSirMacAuthFrameBody));

    palCopyMemory( pMac->hHdd, pPlainText + sizeof(tSirMacAuthFrameBody),
                  icv, SIR_MAC_WEP_ICV_LENGTH);

    //                                    
    limRC4(pEncrBody + SIR_MAC_WEP_IV_LENGTH,
           (tANI_U8 *) pPlainText, seed, keyLength,
           LIM_ENCR_AUTH_BODY_LEN - SIR_MAC_WEP_IV_LENGTH);

    //           
    pEncrBody[0] = seed[0];
    pEncrBody[1] = seed[1];
    pEncrBody[2] = seed[2];
    pEncrBody[3] = keyId << 6;
} /*                                     */



/* 
                    
  
           
                                                               
                                                           
  
        
  
              
     
  
       
     
  
                                                         
                                                      
                                                        
               
 */

void
limComputeCrc32(tANI_U8 *pDest, tANI_U8 * pSrc, tANI_U8 len)
{
    tANI_U32 crc;
    int i;

    crc = 0;
    crc = ~crc;

    while(len-- > 0)
        crc = limCrcUpdate(crc, *pSrc++);

    crc = ~crc;

    for (i=0; i < SIR_MAC_WEP_IV_LENGTH; i++)
    {
        pDest[i] = (tANI_U8)crc;
        crc >>= 8;
    }
} /*                                 */



/* 
           
  
           
                                                             
                                                
  
        
  
              
     
  
       
     
  
                                                                 
                                                         
                                                           
                                                  
                                             
  
               
 */

void
limRC4(tANI_U8 *pDest, tANI_U8 *pSrc, tANI_U8 *seed, tANI_U32 keyLength, tANI_U16 frameLen)
{
    typedef struct
    {
        tANI_U8 i, j;
        tANI_U8 sbox[256];
    } tRC4Context;

    tRC4Context ctx;

    {
        tANI_U16 i, j, k;

        //
        //                           
        //

        ctx.i = ctx.j = 0;
        for (i=0; i<256; i++)
            ctx.sbox[i] = (tANI_U8)i;

        j = 0;
        k = 0;
        for (i=0; i<256; i++)
        {
            tANI_U8 temp;

            j = (tANI_U8)(j + ctx.sbox[i] + seed[k]);
            temp = ctx.sbox[i];
            ctx.sbox[i] = ctx.sbox[j];
            ctx.sbox[j] = temp;

            if (++k >= keyLength)
                k = 0;
        }
    }

    {
        tANI_U8 i   = ctx.i;
        tANI_U8 j   = ctx.j;
        tANI_U8 len = (tANI_U8) frameLen;

        while (len-- > 0)
        {
            tANI_U8 temp1, temp2;

            i     = (tANI_U8)(i+1);
            temp1 = ctx.sbox[i];
            j     = (tANI_U8)(j + temp1);

            ctx.sbox[i] = temp2 = ctx.sbox[j];
            ctx.sbox[j] = temp1;

            temp1 = (tANI_U8)(temp1 + temp2);
            temp1 = ctx.sbox[temp1];
            temp2 = (tANI_U8)(pSrc ? *pSrc++ : 0);

            *pDest++ = (tANI_U8)(temp1 ^ temp2);
        }

        ctx.i = i;
        ctx.j = j;
    }
} /*                        */



/* 
                        
  
           
                                                            
                                                  
  
        
  
              
     
  
       
     
  
                                                    
                                                                 
                                                        
                                                  
                                             
  
                                                        
                                                                  
                                                                  
                                                           
 */

tANI_U8
limDecryptAuthFrame(tpAniSirGlobal pMac, tANI_U8 *pKey, tANI_U8 *pEncrBody,
                    tANI_U8 *pPlainBody, tANI_U32 keyLength, tANI_U16 frameLen)
{
    tANI_U8  seed[LIM_SEED_LENGTH], icv[SIR_MAC_WEP_ICV_LENGTH];
    int i;
    keyLength += 3;


    //                                 
    palCopyMemory( pMac->hHdd, (tANI_U8 *) seed, pEncrBody, SIR_MAC_WEP_IV_LENGTH - 1);

    //                         
    palCopyMemory( pMac->hHdd, (tANI_U8 *) &seed[3], pKey, keyLength - 3);

    //                                        
    limRC4(pPlainBody,
           pEncrBody + SIR_MAC_WEP_IV_LENGTH,
           seed,
           keyLength,
           frameLen);

    PELOG4(limLog(pMac, LOG4, FL("plainbody is \n"));
    sirDumpBuf(pMac, SIR_LIM_MODULE_ID, LOG4, pPlainBody, frameLen);)

    //                                                                
    limComputeCrc32(icv,
                    (tANI_U8 *) pPlainBody,
                    (tANI_U8) (frameLen - SIR_MAC_WEP_ICV_LENGTH));

    //                                 
    for (i = 0; i < SIR_MAC_WEP_ICV_LENGTH; i++)
    {
       PELOG4(limLog(pMac, LOG4, FL(" computed ICV%d[%x], rxed ICV%d[%x]\n"),
               i, icv[i], i, pPlainBody[frameLen - SIR_MAC_WEP_ICV_LENGTH + i]);)
        if (icv[i] != pPlainBody[frameLen - SIR_MAC_WEP_ICV_LENGTH + i])
            return LIM_DECRYPT_ICV_FAIL;
    }

    return eSIR_SUCCESS;
} /*                                     */

/* 
                       
  
                                               
 */
void limPostSmeSetKeysCnf( tpAniSirGlobal pMac,
    tLimMlmSetKeysReq *pMlmSetKeysReq,
    tLimMlmSetKeysCnf *mlmSetKeysCnf)
{
  //                                     
  palCopyMemory( pMac->hHdd, (tANI_U8 *) &mlmSetKeysCnf->peerMacAddr,
                (tANI_U8 *) pMlmSetKeysReq->peerMacAddr,
                sizeof(tSirMacAddr));

  palCopyMemory( pMac->hHdd, (tANI_U8 *) &mlmSetKeysCnf->peerMacAddr,
                (tANI_U8 *) pMlmSetKeysReq->peerMacAddr,
                sizeof(tSirMacAddr));

#if (WNI_POLARIS_FW_PRODUCT == AP)
  mlmSetKeysCnf->aid = pMlmSetKeysReq->aid;
#endif

  //                                            
  palFreeMemory( pMac->hHdd, (tANI_U8 *) pMlmSetKeysReq );
  pMac->lim.gpLimMlmSetKeysReq = NULL;

  limPostSmeMessage( pMac,
      LIM_MLM_SETKEYS_CNF,
      (tANI_U32 *) mlmSetKeysCnf );
}

/* 
                          
  
                                                 
 */
void limPostSmeRemoveKeyCnf( tpAniSirGlobal pMac,
    tpPESession psessionEntry,
    tLimMlmRemoveKeyReq *pMlmRemoveKeyReq,
    tLimMlmRemoveKeyCnf *mlmRemoveKeyCnf)
{
  //                                        
  palCopyMemory( pMac->hHdd, (tANI_U8 *) &mlmRemoveKeyCnf->peerMacAddr,
                (tANI_U8 *) pMlmRemoveKeyReq->peerMacAddr,
                sizeof(tSirMacAddr));

  //                                               
  palFreeMemory( pMac->hHdd, (tANI_U8 *) pMlmRemoveKeyReq );
  pMac->lim.gpLimMlmRemoveKeyReq = NULL;

  psessionEntry->limMlmState = psessionEntry->limPrevMlmState; //                  
  MTRACE(macTrace(pMac, TRACE_CODE_MLM_STATE, psessionEntry->peSessionId, psessionEntry->limMlmState));

  limPostSmeMessage( pMac,
      LIM_MLM_REMOVEKEY_CNF,
      (tANI_U32 *) mlmRemoveKeyCnf );
}

/* 
                        
  
           
                                                          
                                                    
                                 
  
        
  
              
     
  
       
     
  
                                                        
                                                          
               
 */
void limSendSetBssKeyReq( tpAniSirGlobal pMac,
    tLimMlmSetKeysReq *pMlmSetKeysReq,
    tpPESession    psessionEntry)
{
tSirMsgQ           msgQ;
tpSetBssKeyParams  pSetBssKeyParams = NULL;
tLimMlmSetKeysCnf  mlmSetKeysCnf;
tSirRetStatus      retCode;
tANI_U32 val = 0;

  if(pMlmSetKeysReq->numKeys > SIR_MAC_MAX_NUM_OF_DEFAULT_KEYS)
  {
      limLog( pMac, LOG1,
          FL( "numKeys = %d is more than SIR_MAC_MAX_NUM_OF_DEFAULT_KEYS\n" ), pMlmSetKeysReq->numKeys);
      
      //                               
      mlmSetKeysCnf.resultCode = eSIR_SME_INVALID_PARAMETERS;
      goto end;
  }

  //                                              

  if( eHAL_STATUS_SUCCESS != palAllocateMemory( pMac->hHdd,
          (void **) &pSetBssKeyParams,
         sizeof( tSetBssKeyParams )))     
  {
    limLog( pMac, LOGE,
        FL( "Unable to PAL allocate memory during SET_BSSKEY\n" ));

    //                               
    mlmSetKeysCnf.resultCode = eSIR_SME_RESOURCES_UNAVAILABLE;
    goto end;
  }
  else
    palZeroMemory( pMac->hHdd,
        (void *) pSetBssKeyParams,
         sizeof( tSetBssKeyParams ));     

  //                                         
  pSetBssKeyParams->bssIdx = psessionEntry->bssIdx;
  pSetBssKeyParams->encType = pMlmSetKeysReq->edType;


  if(eSIR_SUCCESS != wlan_cfgGetInt(pMac, WNI_CFG_SINGLE_TID_RC, &val))
  {
     limLog( pMac, LOGP, FL( "Unable to read WNI_CFG_SINGLE_TID_RC\n" ));
  }

  pSetBssKeyParams->singleTidRc = (tANI_U8)val;

  /*                     */
  pSetBssKeyParams->sessionId = psessionEntry ->peSessionId;

  if(pMlmSetKeysReq->key[0].keyId && 
     ((pMlmSetKeysReq->edType == eSIR_ED_WEP40) || 
      (pMlmSetKeysReq->edType == eSIR_ED_WEP104))
    )
  {
    /*                                                                      
                                                                                 */
    pSetBssKeyParams->numKeys = SIR_MAC_MAX_NUM_OF_DEFAULT_KEYS;
    palCopyMemory( pMac->hHdd,
      (tANI_U8 *) &pSetBssKeyParams->key[pMlmSetKeysReq->key[0].keyId],
      (tANI_U8 *) &pMlmSetKeysReq->key[0], sizeof(pMlmSetKeysReq->key[0]));

  }
  else
  {
    pSetBssKeyParams->numKeys = pMlmSetKeysReq->numKeys;
    palCopyMemory( pMac->hHdd,
      (tANI_U8 *) &pSetBssKeyParams->key,
      (tANI_U8 *) &pMlmSetKeysReq->key,
      sizeof( tSirKeys ) * pMlmSetKeysReq->numKeys );
  }

  SET_LIM_PROCESS_DEFD_MESGS(pMac, false);
  msgQ.type = WDA_SET_BSSKEY_REQ;
  //
  //           
  //                                                             
  //                                
  //
  msgQ.reserved = 0;
  msgQ.bodyptr = pSetBssKeyParams;
  msgQ.bodyval = 0;

  limLog( pMac, LOGW,
      FL( "Sending WDA_SET_BSSKEY_REQ...\n" ));
  MTRACE(macTraceMsgTx(pMac, psessionEntry->peSessionId, msgQ.type));
  if( eSIR_SUCCESS != (retCode = wdaPostCtrlMsg( pMac, &msgQ )))
  {
    limLog( pMac, LOGE,
        FL("Posting SET_BSSKEY to HAL failed, reason=%X\n"),
        retCode );

    //                                        
    mlmSetKeysCnf.resultCode = eSIR_SME_HAL_SEND_MESSAGE_FAIL;
  }
  else
    return; //                                     

end:
  limPostSmeSetKeysCnf( pMac,
      pMlmSetKeysReq,
      &mlmSetKeysCnf );

}

/* 
                                    
  
                                                                    
                                                      
                                                    
  
        
  
              
     
  
       
     
  
                                                        
                                                          
                                                                   
                                                         
               
 */
void limSendSetStaKeyReq( tpAniSirGlobal pMac,
    tLimMlmSetKeysReq *pMlmSetKeysReq,
    tANI_U16 staIdx,
    tANI_U8 defWEPIdx,
    tpPESession sessionEntry)
{
tSirMsgQ           msgQ;
tpSetStaKeyParams  pSetStaKeyParams = NULL;
tLimMlmSetKeysCnf  mlmSetKeysCnf;
tSirRetStatus      retCode;
tANI_U32 val = 0;

  //                                              
    if( eHAL_STATUS_SUCCESS != palAllocateMemory( pMac->hHdd, (void **) &pSetStaKeyParams,
                                                                                                 sizeof( tSetStaKeyParams ))) {
        limLog( pMac, LOGP, FL( "Unable to PAL allocate memory during SET_BSSKEY\n" ));
        return;
    }else
        palZeroMemory( pMac->hHdd, (void *) pSetStaKeyParams, sizeof( tSetStaKeyParams ));

  //                                         
  pSetStaKeyParams->staIdx = staIdx;
  pSetStaKeyParams->encType = pMlmSetKeysReq->edType;

  
  if(eSIR_SUCCESS != wlan_cfgGetInt(pMac, WNI_CFG_SINGLE_TID_RC, &val))
  {
     limLog( pMac, LOGP, FL( "Unable to read WNI_CFG_SINGLE_TID_RC\n" ));
  }

  pSetStaKeyParams->singleTidRc = (tANI_U8)val;

  /*                      */
  pSetStaKeyParams->sessionId = sessionEntry->peSessionId;

  /* 
                                                  
                          
                                                  
                                         
                                    
   */

  pSetStaKeyParams->defWEPIdx = defWEPIdx;
    
  /*                             */
  sessionEntry->limPrevMlmState = sessionEntry->limMlmState;
  SET_LIM_PROCESS_DEFD_MESGS(pMac, false);
    
  if(sessionEntry->limSystemRole == eLIM_STA_IN_IBSS_ROLE && !pMlmSetKeysReq->key[0].unicast) {
      sessionEntry->limMlmState = eLIM_MLM_WT_SET_STA_BCASTKEY_STATE;
      msgQ.type = WDA_SET_STA_BCASTKEY_REQ;
  }else {
      sessionEntry->limMlmState = eLIM_MLM_WT_SET_STA_KEY_STATE;
      msgQ.type = WDA_SET_STAKEY_REQ;
  }
  MTRACE(macTrace(pMac, TRACE_CODE_MLM_STATE, sessionEntry->peSessionId, sessionEntry->limMlmState));

  /* 
                                                    
                                                          
                                                             
   */
  switch( pMlmSetKeysReq->edType ) {
  case eSIR_ED_WEP40:
  case eSIR_ED_WEP104:
      //                   
      if( 0 == pMlmSetKeysReq->numKeys ) {
#ifdef WLAN_SOFTAP_FEATURE
          tANI_U32 i;

          for(i=0; i < SIR_MAC_MAX_NUM_OF_DEFAULT_KEYS ;i++)
          { 
              palCopyMemory( pMac->hHdd,
                             (tANI_U8 *) &pSetStaKeyParams->key[i],
                             (tANI_U8 *) &pMlmSetKeysReq->key[i], sizeof( tSirKeys ));
          }
#endif
          pSetStaKeyParams->wepType = eSIR_WEP_STATIC;
          sessionEntry->limMlmState = eLIM_MLM_WT_SET_STA_KEY_STATE;
          MTRACE(macTrace(pMac, TRACE_CODE_MLM_STATE, sessionEntry->peSessionId, sessionEntry->limMlmState));
      }else {
          /*                                                                   
                                                                 */
          palCopyMemory( pMac->hHdd,
                             (tANI_U8 *) &pSetStaKeyParams->key[defWEPIdx],
                             (tANI_U8 *) &pMlmSetKeysReq->key[0], sizeof( pMlmSetKeysReq->key[0] ));
          pMlmSetKeysReq->numKeys = SIR_MAC_MAX_NUM_OF_DEFAULT_KEYS;
      }
      break;
  case eSIR_ED_TKIP:
  case eSIR_ED_CCMP:
#ifdef FEATURE_WLAN_WAPI 
  case eSIR_ED_WPI: 
#endif
      {
          palCopyMemory( pMac->hHdd, (tANI_U8 *) &pSetStaKeyParams->key,
                         (tANI_U8 *) &pMlmSetKeysReq->key[0], sizeof( tSirKeys ));
      }
      break;
  default:
      break;
  }

  
  //
  //           
  //                                                             
  //                                
  //
  msgQ.reserved = 0;
  msgQ.bodyptr = pSetStaKeyParams;
  msgQ.bodyval = 0;

  limLog( pMac, LOG1, FL( "Sending WDA_SET_STAKEY_REQ...\n" ));
  MTRACE(macTraceMsgTx(pMac, sessionEntry->peSessionId, msgQ.type));
  if( eSIR_SUCCESS != (retCode = wdaPostCtrlMsg( pMac, &msgQ ))) {
      limLog( pMac, LOGE, FL("Posting SET_STAKEY to HAL failed, reason=%X\n"), retCode );
      //                                        
      mlmSetKeysCnf.resultCode = eSIR_SME_HAL_SEND_MESSAGE_FAIL;
  }else
      return; //                                     

  limPostSmeSetKeysCnf( pMac, pMlmSetKeysReq, &mlmSetKeysCnf );
}

/* 
                           
  
           
                                                         
                                                  
                                 
  
        
  
              
     
  
       
     
  
                                                        
                                                              
               
 */
void limSendRemoveBssKeyReq( tpAniSirGlobal pMac,
    tLimMlmRemoveKeyReq *pMlmRemoveKeyReq,
    tpPESession   psessionEntry)
{
tSirMsgQ           msgQ;
tpRemoveBssKeyParams  pRemoveBssKeyParams = NULL;
tLimMlmRemoveKeyCnf  mlmRemoveKeysCnf;
tSirRetStatus      retCode;

  //                                                 

  if( eHAL_STATUS_SUCCESS != palAllocateMemory( pMac->hHdd,
          (void **) &pRemoveBssKeyParams,
         sizeof( tRemoveBssKeyParams )))     
  {
    limLog( pMac, LOGE,
        FL( "Unable to PAL allocate memory during REMOVE_BSSKEY\n" ));

    //                               
    mlmRemoveKeysCnf.resultCode = eSIR_SME_RESOURCES_UNAVAILABLE;
    goto end;
  }
  else
    palZeroMemory( pMac->hHdd,
        (void *) pRemoveBssKeyParams,
         sizeof( tRemoveBssKeyParams ));     

  //                                            
  pRemoveBssKeyParams->bssIdx = psessionEntry->bssIdx;
  pRemoveBssKeyParams->encType = pMlmRemoveKeyReq->edType;
  pRemoveBssKeyParams->keyId = pMlmRemoveKeyReq->keyId;
  pRemoveBssKeyParams->wepType = pMlmRemoveKeyReq->wepType;

  /*                     */

  pRemoveBssKeyParams->sessionId = psessionEntry->peSessionId;

  msgQ.type = WDA_REMOVE_BSSKEY_REQ;
  //
  //           
  //                                                             
  //                                
  //
  msgQ.reserved = 0;
  msgQ.bodyptr = pRemoveBssKeyParams;
  msgQ.bodyval = 0;

  limLog( pMac, LOGW,
      FL( "Sending WDA_REMOVE_BSSKEY_REQ...\n" ));
  MTRACE(macTraceMsgTx(pMac, psessionEntry->peSessionId, msgQ.type));

  if( eSIR_SUCCESS != (retCode = wdaPostCtrlMsg( pMac, &msgQ )))
  {
    limLog( pMac, LOGE,
        FL("Posting REMOVE_BSSKEY to HAL failed, reason=%X\n"),
        retCode );

    //                                           
    mlmRemoveKeysCnf.resultCode = eSIR_SME_HAL_SEND_MESSAGE_FAIL;
  }
  else
    return; 

end:
  limPostSmeRemoveKeyCnf( pMac,
      psessionEntry,
      pMlmRemoveKeyReq,
      &mlmRemoveKeysCnf );

}

/* 
                           
  
           
                                                             
                                                      
                                                    
  
        
  
              
     
  
       
     
  
                                                        
                                                                
                                                                   
               
 */
void limSendRemoveStaKeyReq( tpAniSirGlobal pMac,
    tLimMlmRemoveKeyReq *pMlmRemoveKeyReq,
    tANI_U16 staIdx ,
    tpPESession psessionEntry)
{
tSirMsgQ           msgQ;
tpRemoveStaKeyParams  pRemoveStaKeyParams = NULL;
tLimMlmRemoveKeyCnf  mlmRemoveKeyCnf;
tSirRetStatus      retCode;



  if( eHAL_STATUS_SUCCESS != palAllocateMemory( pMac->hHdd,
          (void **) &pRemoveStaKeyParams,
          sizeof( tRemoveStaKeyParams )))
  {
    limLog( pMac, LOGE,
        FL( "Unable to PAL allocate memory during REMOVE_STAKEY\n" ));

    //                               
    mlmRemoveKeyCnf.resultCode = eSIR_SME_RESOURCES_UNAVAILABLE;
    goto end;
  }
  else
    palZeroMemory( pMac->hHdd,
        (void *) pRemoveStaKeyParams,
        sizeof( tRemoveStaKeyParams ));

  if( (pMlmRemoveKeyReq->edType == eSIR_ED_WEP104 || pMlmRemoveKeyReq->edType == eSIR_ED_WEP40) &&
        pMlmRemoveKeyReq->wepType == eSIR_WEP_STATIC )
  {
        PELOGE(limLog(pMac, LOGE, FL("Request to remove static WEP keys through station interface\n Should use BSS interface\n"));)
        mlmRemoveKeyCnf.resultCode = eSIR_SME_INVALID_PARAMETERS;
        goto end;
  }
 
  //                                        
  pRemoveStaKeyParams->staIdx = staIdx;
  pRemoveStaKeyParams->encType = pMlmRemoveKeyReq->edType;
  pRemoveStaKeyParams->keyId = pMlmRemoveKeyReq->keyId;
  pRemoveStaKeyParams->unicast = pMlmRemoveKeyReq->unicast;

  /*                     */
  pRemoveStaKeyParams->sessionId = psessionEntry->peSessionId;

  SET_LIM_PROCESS_DEFD_MESGS(pMac, false);
  
  msgQ.type = WDA_REMOVE_STAKEY_REQ;
  //
  //           
  //                                                             
  //                                
  //
  msgQ.reserved = 0;
  msgQ.bodyptr = pRemoveStaKeyParams;
  msgQ.bodyval = 0;

  limLog( pMac, LOGW,
      FL( "Sending WDA_REMOVE_STAKEY_REQ...\n" ));
  MTRACE(macTraceMsgTx(pMac, psessionEntry->peSessionId, msgQ.type));
  if( eSIR_SUCCESS != (retCode = wdaPostCtrlMsg( pMac, &msgQ )))
  {
    limLog( pMac, LOGE,
        FL("Posting REMOVE_STAKEY to HAL failed, reason=%X\n"),
        retCode );

    //                                          
    mlmRemoveKeyCnf.resultCode = eSIR_SME_HAL_SEND_MESSAGE_FAIL;
  }
  else
    return; 

end:
  limPostSmeRemoveKeyCnf( pMac,
      psessionEntry,
      pMlmRemoveKeyReq,
      &mlmRemoveKeyCnf );

}


