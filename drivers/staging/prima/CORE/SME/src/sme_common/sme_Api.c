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

/**=========================================================================

  \file  smeApi.c

  \brief Definitions for SME APIs

   Copyright 2008 (c) Qualcomm, Incorporated.  All Rights Reserved.

   Qualcomm Confidential and Proprietary.

  ========================================================================*/

/*                                                                           

                                           


                                                                       
                                                                



                                                     
                                                                                            
                                                                   
                                                               

                                                                           */

/*                                                                          
               
                                                                          */

#ifdef FEATURE_WLAN_NON_INTEGRATED_SOC
#include "halInternal.h"
#endif

#include "smsDebug.h"
#include "sme_Api.h"
#include "csrInsideApi.h"
#include "smeInside.h"
#include "csrInternal.h"
#include "wlan_qct_wda.h"
#include "halMsgApi.h"

#ifdef WLAN_SOFTAP_FEATURE
#include "sapApi.h"
#endif



#if !defined(FEATURE_WLAN_NON_INTEGRATED_SOC)
extern tSirRetStatus uMacPostCtrlMsg(void* pSirGlobal, tSirMbMsg* pMb);

#include <wlan_qct_pal_api.h>
#endif

//               
extern eHalStatus pmcPrepareCommand( tpAniSirGlobal pMac, eSmeCommandType cmdType, void *pvParam,
                            tANI_U32 size, tSmeCmd **ppCmd );
extern void pmcReleaseCommand( tpAniSirGlobal pMac, tSmeCmd *pCommand );
extern void qosReleaseCommand( tpAniSirGlobal pMac, tSmeCmd *pCommand );
#if defined WLAN_FEATURE_P2P
extern eHalStatus p2pProcessRemainOnChannelCmd(tpAniSirGlobal pMac, tSmeCmd *p2pRemainonChn);
extern eHalStatus sme_remainOnChnRsp( tpAniSirGlobal pMac, tANI_U8 *pMsg);
extern eHalStatus sme_mgmtFrmInd( tHalHandle hHal, tpSirSmeMgmtFrameInd pSmeMgmtFrm);
extern eHalStatus sme_remainOnChnReady( tHalHandle hHal, tANI_U8* pMsg);
extern eHalStatus sme_sendActionCnf( tHalHandle hHal, tANI_U8* pMsg);
extern eHalStatus p2pProcessNoAReq(tpAniSirGlobal pMac, tSmeCmd *pNoACmd);
#endif

static eHalStatus initSmeCmdList(tpAniSirGlobal pMac);
static void smeAbortCommand( tpAniSirGlobal pMac, tSmeCmd *pCommand, tANI_BOOLEAN fStopping );

eCsrPhyMode sme_GetPhyMode(tHalHandle hHal);

eHalStatus sme_HandleChangeCountryCode(tpAniSirGlobal pMac,  void *pMsgBuf);

eHalStatus sme_HandlePreChannelSwitchInd(tHalHandle hHal);

eHalStatus sme_HandlePostChannelSwitchInd(tHalHandle hHal);

#ifdef FEATURE_WLAN_LFR
tANI_BOOLEAN csrIsScanAllowed(tpAniSirGlobal pMac);
#endif

//                 
eHalStatus sme_AcquireGlobalLock( tSmeStruct *psSme)
{
    eHalStatus status = eHAL_STATUS_INVALID_PARAMETER;

    if(psSme)
    {
        if( VOS_IS_STATUS_SUCCESS( vos_lock_acquire( &psSme->lkSmeGlobalLock) ) )
        {
            status = eHAL_STATUS_SUCCESS;
        }
    }

    return (status);
}


eHalStatus sme_ReleaseGlobalLock( tSmeStruct *psSme)
{
    eHalStatus status = eHAL_STATUS_INVALID_PARAMETER;

    if(psSme)
    {
        if( VOS_IS_STATUS_SUCCESS( vos_lock_release( &psSme->lkSmeGlobalLock) ) )
        {
            status = eHAL_STATUS_SUCCESS;
        }
    }

    return (status);
}



static eHalStatus initSmeCmdList(tpAniSirGlobal pMac)
{
    eHalStatus status;
    tSmeCmd *pCmd;

    pMac->sme.totalSmeCmd = SME_TOTAL_COMMAND;
    if(HAL_STATUS_SUCCESS(status = csrLLOpen(pMac->hHdd, &pMac->sme.smeCmdActiveList)))
    {
        if(HAL_STATUS_SUCCESS(status = csrLLOpen(pMac->hHdd, &pMac->sme.smeCmdPendingList)))
        {
            if(HAL_STATUS_SUCCESS(status = csrLLOpen(pMac->hHdd, &pMac->sme.smeCmdFreeList)))
            {
                status = palAllocateMemory(pMac->hHdd, (void **)&pCmd, sizeof(tSmeCmd) * pMac->sme.totalSmeCmd);
                if(HAL_STATUS_SUCCESS(status))
                {
                    tANI_U32 c;

                    palZeroMemory(pMac->hHdd, pCmd, sizeof(tSmeCmd) * pMac->sme.totalSmeCmd);
                    pMac->sme.pSmeCmdBufAddr = pCmd;
                    for(c = 0; c < pMac->sme.totalSmeCmd; c++)
                    {
                        csrLLInsertTail(&pMac->sme.smeCmdFreeList, &pCmd[c].Link, LL_ACCESS_LOCK);
                    }
                }
            }
        }
    }

    return (status);
}


void smeReleaseCommand(tpAniSirGlobal pMac, tSmeCmd *pCmd)
{
    pCmd->command = eSmeNoCommand;
    csrLLInsertTail(&pMac->sme.smeCmdFreeList, &pCmd->Link, LL_ACCESS_LOCK);
}



static void smeReleaseCmdList(tpAniSirGlobal pMac, tDblLinkList *pList)
{
    tListElem *pEntry;
    tSmeCmd *pCommand;

    while((pEntry = csrLLRemoveHead(pList, LL_ACCESS_LOCK)) != NULL)
    {
        //                                                    
        //                                                          
        pCommand = GET_BASE_ADDR( pEntry, tSmeCmd, Link );
        smeAbortCommand(pMac, pCommand, eANI_BOOLEAN_TRUE);
    }
}

static void purgeSmeCmdList(tpAniSirGlobal pMac)
{
    //                                                           
    smeReleaseCmdList(pMac, &pMac->sme.smeCmdPendingList);
    smeReleaseCmdList(pMac, &pMac->sme.smeCmdActiveList);
}

void purgeSmeSessionCmdList(tpAniSirGlobal pMac, tANI_U32 sessionId)
{
    //                                                           
    tListElem *pEntry, *pNext;
    tSmeCmd *pCommand;
    tDblLinkList *pList = &pMac->sme.smeCmdPendingList;
    tDblLinkList localList;

    vos_mem_zero(&localList, sizeof(tDblLinkList));
    if(!HAL_STATUS_SUCCESS(csrLLOpen(pMac->hHdd, &localList)))
    {
        smsLog(pMac, LOGE, FL(" failed to open list"));
        return;
    }

    csrLLLock(pList);
    pEntry = csrLLPeekHead(pList, LL_ACCESS_NOLOCK);
    while(pEntry != NULL)
    {
        pNext = csrLLNext(pList, pEntry, LL_ACCESS_NOLOCK);
        pCommand = GET_BASE_ADDR( pEntry, tSmeCmd, Link );
        if(pCommand->sessionId == sessionId)
        {
            if(csrLLRemoveEntry(pList, pEntry, LL_ACCESS_NOLOCK))
            {
                csrLLInsertTail(&localList, pEntry, LL_ACCESS_NOLOCK);
            }
        }
        pEntry = pNext;
    }
    csrLLUnlock(pList);

    while( (pEntry = csrLLRemoveHead(&localList, LL_ACCESS_NOLOCK)) )
    {
        pCommand = GET_BASE_ADDR( pEntry, tSmeCmd, Link );
        smeAbortCommand(pMac, pCommand, eANI_BOOLEAN_TRUE);
    }
    csrLLClose(&localList);

}


static eHalStatus freeSmeCmdList(tpAniSirGlobal pMac)
{
    eHalStatus status = eHAL_STATUS_SUCCESS;

    purgeSmeCmdList(pMac);
    csrLLClose(&pMac->sme.smeCmdPendingList);
    csrLLClose(&pMac->sme.smeCmdActiveList);
    csrLLClose(&pMac->sme.smeCmdFreeList);

    if(NULL != pMac->sme.pSmeCmdBufAddr)
    {
        status = palFreeMemory(pMac->hHdd, pMac->sme.pSmeCmdBufAddr);
        pMac->sme.pSmeCmdBufAddr = NULL;
    }

    return (status);
}


void dumpCsrCommandInfo(tpAniSirGlobal pMac, tSmeCmd *pCmd)
{
#ifdef WLAN_DEBUG
    switch( pCmd->command )
    {
    case eSmeCommandScan:
        smsLog( pMac, LOGE, " scan command reason is %d", pCmd->u.scanCmd.reason );
        break;

    case eSmeCommandRoam:
        smsLog( pMac, LOGE, " roam command reason is %d", pCmd->u.roamCmd.roamReason );
        break;

    case eSmeCommandWmStatusChange:
        smsLog( pMac, LOGE, " WMStatusChange command type is %d", pCmd->u.wmStatusChangeCmd.Type );
        break;

    case eSmeCommandSetKey:
        smsLog( pMac, LOGE, " setKey command auth(%d) enc(%d)", 
                        pCmd->u.setKeyCmd.authType, pCmd->u.setKeyCmd.encType );
        break;

    case eSmeCommandRemoveKey:
        smsLog( pMac, LOGE, " removeKey command auth(%d) enc(%d)", 
                        pCmd->u.removeKeyCmd.authType, pCmd->u.removeKeyCmd.encType );
        break;

    default:
        break;
    }
#endif  //                 
}

tSmeCmd *smeGetCommandBuffer( tpAniSirGlobal pMac )
{
    tSmeCmd *pRetCmd = NULL, *pTempCmd = NULL;
    tListElem *pEntry;

    pEntry = csrLLRemoveHead( &pMac->sme.smeCmdFreeList, LL_ACCESS_LOCK );

    //                                                                
    //                                                               
    //                                        
    if ( pEntry )
    {
        pRetCmd = GET_BASE_ADDR( pEntry, tSmeCmd, Link );
    }
    else {
        int idx = 1;

        //                                                          
        pEntry = csrLLPeekHead( &pMac->sme.smeCmdActiveList, LL_ACCESS_LOCK );
        if( pEntry )
        {
           pTempCmd = GET_BASE_ADDR( pEntry, tSmeCmd, Link );
        }
        smsLog( pMac, LOGE, "Out of command buffer.... command (0x%X) stuck\n", 
           (pTempCmd) ? pTempCmd->command : eSmeNoCommand );
        if(pTempCmd)
        {
            if( eSmeCsrCommandMask & pTempCmd->command )
            {
                //                                                                  
                dumpCsrCommandInfo(pMac, pTempCmd);
            }
        } //            

        //                                 
        csrLLLock(&pMac->sme.smeCmdPendingList);
        pEntry = csrLLPeekHead( &pMac->sme.smeCmdPendingList, LL_ACCESS_NOLOCK );
        while(pEntry)
        {
            pTempCmd = GET_BASE_ADDR( pEntry, tSmeCmd, Link );
            smsLog( pMac, LOGE, "Out of command buffer.... SME pending command #%d (0x%X)\n",
                    idx++, pTempCmd->command );
            if( eSmeCsrCommandMask & pTempCmd->command )
            {
                //                                                                  
                dumpCsrCommandInfo(pMac, pTempCmd);
            }
            pEntry = csrLLNext( &pMac->sme.smeCmdPendingList, pEntry, LL_ACCESS_NOLOCK );
        }
        csrLLUnlock(&pMac->sme.smeCmdPendingList);

        //                                                         
        csrLLLock(&pMac->roam.roamCmdPendingList);
        pEntry = csrLLPeekHead( &pMac->roam.roamCmdPendingList, LL_ACCESS_NOLOCK );
        while(pEntry)
        {
            pTempCmd = GET_BASE_ADDR( pEntry, tSmeCmd, Link );
            smsLog( pMac, LOGE, "Out of command buffer.... CSR pending command #%d (0x%X)\n",
                    idx++, pTempCmd->command );
            dumpCsrCommandInfo(pMac, pTempCmd);
            pEntry = csrLLNext( &pMac->roam.roamCmdPendingList, pEntry, LL_ACCESS_NOLOCK );
        }
        csrLLUnlock(&pMac->roam.roamCmdPendingList);
    }

    return( pRetCmd );
}


void smePushCommand( tpAniSirGlobal pMac, tSmeCmd *pCmd, tANI_BOOLEAN fHighPriority )
{
    if ( fHighPriority )
    {
        csrLLInsertHead( &pMac->sme.smeCmdPendingList, &pCmd->Link, LL_ACCESS_LOCK );
    }
    else
    {
        csrLLInsertTail( &pMac->sme.smeCmdPendingList, &pCmd->Link, LL_ACCESS_LOCK );
    }

    //                             
    smeProcessPendingQueue( pMac );

    return;
}


static eSmeCommandType smeIsFullPowerNeeded( tpAniSirGlobal pMac, tSmeCmd *pCommand )
{
    eSmeCommandType pmcCommand = eSmeNoCommand;
    tANI_BOOLEAN fFullPowerNeeded = eANI_BOOLEAN_FALSE;
    tPmcState pmcState;
    eHalStatus status;

    do
    {
        pmcState = pmcGetPmcState(pMac);

        status = csrIsFullPowerNeeded( pMac, pCommand, NULL, &fFullPowerNeeded );
        if( !HAL_STATUS_SUCCESS(status) )
        {
            //                                               
            return ( eSmeDropCommand );
        }
        if( fFullPowerNeeded  ) break;
        fFullPowerNeeded = ( ( eSmeCommandAddTs == pCommand->command ) ||
                    ( eSmeCommandDelTs ==  pCommand->command ) );
        if( fFullPowerNeeded ) break;
#ifdef FEATURE_OEM_DATA_SUPPORT
        fFullPowerNeeded = (pmcState == IMPS && 
                                       eSmeCommandOemDataReq == pCommand->command);
        if(fFullPowerNeeded) break;
#endif
#ifdef WLAN_FEATURE_P2P
        fFullPowerNeeded = (pmcState == IMPS && 
                            eSmeCommandRemainOnChannel == pCommand->command);
        if(fFullPowerNeeded) break;
#endif
    } while(0);

    if( fFullPowerNeeded )
    {
        switch( pmcState )
        {
        case IMPS:
        case STANDBY:
            pmcCommand = eSmeCommandExitImps;
            break;

        case BMPS:
            pmcCommand = eSmeCommandExitBmps;
            break;

        case UAPSD:
            pmcCommand = eSmeCommandExitUapsd;
            break;

        case WOWL:
            pmcCommand = eSmeCommandExitWowl;
            break;

        default:
            break;
        }
    }

    return ( pmcCommand );
}


//                                           
static void smeAbortCommand( tpAniSirGlobal pMac, tSmeCmd *pCommand, tANI_BOOLEAN fStopping )
{
    if( eSmePmcCommandMask & pCommand->command )
    {
        pmcAbortCommand( pMac, pCommand, fStopping );
    }
    else if ( eSmeCsrCommandMask & pCommand->command )
    {
        csrAbortCommand( pMac, pCommand, fStopping );
    }
    else
    {
        switch( pCommand->command )
        {
#ifdef WLAN_FEATURE_P2P
            case eSmeCommandRemainOnChannel:
                if (NULL != pCommand->u.remainChlCmd.callback)
                {
                    remainOnChanCallback callback = 
                                            pCommand->u.remainChlCmd.callback;
                    /*                 */
                    if( callback )
                    {
                        callback(pMac, pCommand->u.remainChlCmd.callbackCtx, 
                                            eCSR_SCAN_ABORT );
                    }
                }
                smeReleaseCommand( pMac, pCommand );
                break;
#endif
            default:
                smeReleaseCommand( pMac, pCommand );
                break;
        }
    }
}



tANI_BOOLEAN smeProcessCommand( tpAniSirGlobal pMac )
{
    tANI_BOOLEAN fContinue = eANI_BOOLEAN_FALSE;
    eHalStatus status = eHAL_STATUS_SUCCESS;
    tListElem *pEntry;
    tSmeCmd *pCommand;
    eSmeCommandType pmcCommand = eSmeNoCommand;

    //                                                                       
    //                   
    //                                                   
    csrLLLock( &pMac->sme.smeCmdActiveList );
    if ( csrLLIsListEmpty( &pMac->sme.smeCmdActiveList, LL_ACCESS_NOLOCK ) )
    {
        if(!csrLLIsListEmpty(&pMac->sme.smeCmdPendingList, LL_ACCESS_LOCK))
        {
            //                
            pEntry = csrLLPeekHead( &pMac->sme.smeCmdPendingList, LL_ACCESS_LOCK );
            if( pEntry )
            {
                pCommand = GET_BASE_ADDR( pEntry, tSmeCmd, Link );
                //                                                                            
                if( CSR_IS_WAIT_FOR_KEY( pMac, pCommand->sessionId ) )
                {
                    if( !CSR_IS_SET_KEY_COMMAND( pCommand ) )
                    {
                        csrLLUnlock( &pMac->sme.smeCmdActiveList );
                        smsLog(pMac, LOGE, "  Cannot process command(%d) while waiting for key\n", pCommand->command);
                        return ( eANI_BOOLEAN_FALSE );
                    }
                }
                pmcCommand = smeIsFullPowerNeeded( pMac, pCommand );
                if( eSmeDropCommand == pmcCommand )
                {
                    //                                            
                    if( csrLLRemoveEntry( &pMac->sme.smeCmdPendingList, pEntry, LL_ACCESS_LOCK ) )
                    {
                        smeAbortCommand( pMac, pCommand, eANI_BOOLEAN_FALSE );
                    }
                    csrLLUnlock( &pMac->sme.smeCmdActiveList );
                    //                       
                    return (eANI_BOOLEAN_TRUE);
                }
                else if( eSmeNoCommand != pmcCommand )
                {
                    tExitBmpsInfo exitBmpsInfo;
                    void *pv = NULL;
                    tANI_U32 size = 0;
                    tSmeCmd *pPmcCmd = NULL;

                    if( eSmeCommandExitBmps == pmcCommand )
                    {
                        exitBmpsInfo.exitBmpsReason = eSME_REASON_OTHER;
                        pv = (void *)&exitBmpsInfo;
                        size = sizeof(tExitBmpsInfo);
                    }
                    //                                                       
                    status = pmcPrepareCommand( pMac, pmcCommand, pv, size, &pPmcCmd );
                    if( HAL_STATUS_SUCCESS( status ) && pPmcCmd )
                    {
                        //                                      
                        csrLLInsertHead( &pMac->sme.smeCmdActiveList, &pPmcCmd->Link, LL_ACCESS_NOLOCK );
                        csrLLUnlock( &pMac->sme.smeCmdActiveList );
                        fContinue = pmcProcessCommand( pMac, pPmcCmd );
                        if( fContinue )
                        {
                            //                             
                            if( csrLLRemoveEntry( &pMac->sme.smeCmdActiveList, &pPmcCmd->Link, LL_ACCESS_NOLOCK ) )
                            {
                                pmcReleaseCommand( pMac, pPmcCmd );
                            }
                        }
                    }
                    else
                    {
                        csrLLUnlock( &pMac->sme.smeCmdActiveList );
                        smsLog( pMac, LOGE, FL(  "Cannot issue command(0x%X) to wake up the chip. Status = %d\n"), pmcCommand, status );
                        //            
                        fContinue = eANI_BOOLEAN_TRUE;
                    }
                    return fContinue;
                }
                if ( csrLLRemoveEntry( &pMac->sme.smeCmdPendingList, pEntry, LL_ACCESS_LOCK ) )
                {
                    //                          

                    //                                          
                    csrLLInsertHead( &pMac->sme.smeCmdActiveList, &pCommand->Link, LL_ACCESS_NOLOCK );

                    //                              

                    switch ( pCommand->command )
                    {

                        case eSmeCommandScan:
                            csrLLUnlock( &pMac->sme.smeCmdActiveList );
                            status = csrProcessScanCommand( pMac, pCommand );
                            break;

                        case eSmeCommandRoam:
                            csrLLUnlock( &pMac->sme.smeCmdActiveList );
                            status  = csrRoamProcessCommand( pMac, pCommand );
                            break;

                        case eSmeCommandWmStatusChange:
                            csrLLUnlock( &pMac->sme.smeCmdActiveList );
                            csrRoamProcessWmStatusChangeCommand(pMac, pCommand);
                            break;

                        case eSmeCommandSetKey:
                            csrLLUnlock( &pMac->sme.smeCmdActiveList );
                            status = csrRoamProcessSetKeyCommand( pMac, pCommand );
                            if(!HAL_STATUS_SUCCESS(status))
                            {
                                if( csrLLRemoveEntry( &pMac->sme.smeCmdActiveList, 
                                            &pCommand->Link, LL_ACCESS_LOCK ) )
                                {
                                    csrReleaseCommandSetKey( pMac, pCommand );
                                }
                            }
                            break;

                        case eSmeCommandRemoveKey:
                            csrLLUnlock( &pMac->sme.smeCmdActiveList );
                            status = csrRoamProcessRemoveKeyCommand( pMac, pCommand );
                            if(!HAL_STATUS_SUCCESS(status))
                            {
                                if( csrLLRemoveEntry( &pMac->sme.smeCmdActiveList, 
                                            &pCommand->Link, LL_ACCESS_LOCK ) )
                                {
                                    csrReleaseCommandRemoveKey( pMac, pCommand );
                                }
                            }
                            break;

                        case eSmeCommandAddStaSession:
                            csrLLUnlock( &pMac->sme.smeCmdActiveList );
                            csrProcessAddStaSessionCommand( pMac, pCommand );
                            break;
                        case eSmeCommandDelStaSession:    
                            csrLLUnlock( &pMac->sme.smeCmdActiveList );
                            csrProcessDelStaSessionCommand( pMac, pCommand );
                            break;

#ifdef FEATURE_OEM_DATA_SUPPORT
                        case eSmeCommandOemDataReq:
                            csrLLUnlock(&pMac->sme.smeCmdActiveList);
                            oemData_ProcessOemDataReqCommand(pMac, pCommand);
                            break;
#endif
#if defined WLAN_FEATURE_P2P
                        case eSmeCommandRemainOnChannel:
                            csrLLUnlock(&pMac->sme.smeCmdActiveList);
                            p2pProcessRemainOnChannelCmd(pMac, pCommand);
                            break;
                        case eSmeCommandNoAUpdate:
                            csrLLUnlock( &pMac->sme.smeCmdActiveList );
                            p2pProcessNoAReq(pMac,pCommand);
#endif
                        case eSmeCommandEnterImps:
                        case eSmeCommandExitImps:
                        case eSmeCommandEnterBmps:
                        case eSmeCommandExitBmps:
                        case eSmeCommandEnterUapsd:
                        case eSmeCommandExitUapsd:
                        case eSmeCommandEnterWowl:
                        case eSmeCommandExitWowl:
                            csrLLUnlock( &pMac->sme.smeCmdActiveList );
                            fContinue = pmcProcessCommand( pMac, pCommand );
                            if( fContinue )
                            {
                                //                             
                                if( csrLLRemoveEntry( &pMac->sme.smeCmdActiveList,
                                            &pCommand->Link, LL_ACCESS_LOCK ) )
                                {
                                    pmcReleaseCommand( pMac, pCommand );
                                }
                            }
                            break;

                        //                                                                       
                        //                                  
                        case eSmeCommandEnterStandby:
                            if( csrIsConnStateDisconnected( pMac, pCommand->sessionId ) )
                            {
                                //               
                                csrLLUnlock( &pMac->sme.smeCmdActiveList );
                                fContinue = pmcProcessCommand( pMac, pCommand );
                                if( fContinue )
                                {
                                    //                             
                                    if( csrLLRemoveEntry( &pMac->sme.smeCmdActiveList,
                                                &pCommand->Link, LL_ACCESS_LOCK ) )
                                    {
                                        pmcReleaseCommand( pMac, pCommand );
                                    }
                                }
                            }
                            else
                            {
                                //                                                               
                                tSmeCmd *pNewCmd;

                                //                             
                                fContinue = eANI_BOOLEAN_TRUE;
                                //                                  
                                if( csrLLRemoveEntry( &pMac->sme.smeCmdActiveList,
                                                &pCommand->Link, LL_ACCESS_NOLOCK ) )
                                {
                                    csrLLUnlock( &pMac->sme.smeCmdActiveList );
                                    //                                                             
                                    //                 
                                    pNewCmd = csrGetCommandBuffer( pMac );
                                    if( NULL != pNewCmd )
                                    {
                                        //                                                             
                                        csrLLInsertHead( &pMac->sme.smeCmdPendingList, &pCommand->Link,
                                                        LL_ACCESS_LOCK );
                                        pNewCmd->command = eSmeCommandRoam;
                                        pNewCmd->u.roamCmd.roamReason = eCsrForcedDisassoc;
                                        //                                                   
                                        csrLLInsertHead( &pMac->sme.smeCmdPendingList, &pNewCmd->Link,
                                                        LL_ACCESS_LOCK );
                                    }
                                    else
                                    {
                                        //                         
                                        fContinue = pmcProcessCommand( pMac, pCommand );
                                        if( fContinue )
                                        {
                                            //                             
                                            if( csrLLRemoveEntry( &pMac->sme.smeCmdActiveList,
                                                        &pCommand->Link, LL_ACCESS_LOCK ) )
                                            {
                                                pmcReleaseCommand( pMac, pCommand );
                                            }
                                        }
                                    }
                                }
                                else
                                {
                                    csrLLUnlock( &pMac->sme.smeCmdActiveList );
                                    smsLog( pMac, LOGE, FL(" failed to remove standby command\n") );
                                    VOS_ASSERT(0);
                                }
                            }
                            break;

                        case eSmeCommandAddTs:
                        case eSmeCommandDelTs:
                            csrLLUnlock( &pMac->sme.smeCmdActiveList );
#ifndef WLAN_MDM_CODE_REDUCTION_OPT
                            fContinue = qosProcessCommand( pMac, pCommand );
                            if( fContinue )
                            {
                                //                             
                                if( csrLLRemoveEntry( &pMac->sme.smeCmdActiveList,
                                            &pCommand->Link, LL_ACCESS_NOLOCK ) )
                                {
//                                   
                                    qosReleaseCommand( pMac, pCommand );
//                                       
                                }
                            }
#endif
                            break;

                        default:
                            //                  
                            //                              
                            smsLog(pMac, LOGE, " csrProcessCommand processes an unknown command %d\n", pCommand->command);
                            pEntry = csrLLRemoveHead( &pMac->sme.smeCmdActiveList, LL_ACCESS_NOLOCK );
                            csrLLUnlock( &pMac->sme.smeCmdActiveList );
                            pCommand = GET_BASE_ADDR( pEntry, tSmeCmd, Link );
                            smeReleaseCommand( pMac, pCommand );
                            status = eHAL_STATUS_FAILURE;
                            break;
                    }
                    if(!HAL_STATUS_SUCCESS(status))
                    {
                        fContinue = eANI_BOOLEAN_TRUE;
                    }
                }//          
                else
                {
                    //                                                
                    csrLLUnlock( &pMac->sme.smeCmdActiveList );
                }
            }
            else
            {
                csrLLUnlock( &pMac->sme.smeCmdActiveList );
            }
        }
        else
        {
            //                  
            csrLLUnlock( &pMac->sme.smeCmdActiveList );
            //                                                                                                   
            if(pMac->scan.fRestartIdleScan && eANI_BOOLEAN_FALSE == pMac->scan.fCancelIdleScan)
            {
                tANI_U32 nTime = 0;

                pMac->scan.fRestartIdleScan = eANI_BOOLEAN_FALSE;
                if(!HAL_STATUS_SUCCESS(csrScanTriggerIdleScan(pMac, &nTime)))
                {
                    csrScanStartIdleScanTimer(pMac, nTime);
                }
            }
        }
    }
    else {
        csrLLUnlock( &pMac->sme.smeCmdActiveList );
    }

    return ( fContinue );
}

void smeProcessPendingQueue( tpAniSirGlobal pMac )
{
    while( smeProcessCommand( pMac ) );
}


tANI_BOOLEAN smeCommandPending(tpAniSirGlobal pMac)
{
    return ( !csrLLIsListEmpty( &pMac->sme.smeCmdActiveList, LL_ACCESS_NOLOCK ) ||
        !csrLLIsListEmpty(&pMac->sme.smeCmdPendingList, LL_ACCESS_NOLOCK) );
}



//           

/*                                                                          

                                                                          

                                                                             
                                                                    

                                                                   
                                       
                             
                                               

                                                                

                                                            
     

                                                                            */
eHalStatus sme_Open(tHalHandle hHal)
{
   eHalStatus status = eHAL_STATUS_FAILURE;
   tpAniSirGlobal pMac = PMAC_STRUCT( hHal );

   do {
      pMac->sme.state = SME_STATE_STOP;
      if( !VOS_IS_STATUS_SUCCESS( vos_lock_init( &pMac->sme.lkSmeGlobalLock ) ) )
      {
          smsLog( pMac, LOGE, "sme_Open failed init lock\n" );
          status = eHAL_STATUS_FAILURE;
          break;
      }

      status = ccmOpen(hHal);
      if ( ! HAL_STATUS_SUCCESS( status ) ) {
         smsLog( pMac, LOGE,
                 "ccmOpen failed during initialization with status=%d", status );
         break;
      }

      status = csrOpen(pMac);
      if ( ! HAL_STATUS_SUCCESS( status ) ) {
         smsLog( pMac, LOGE,
                 "csrOpen failed during initialization with status=%d", status );
         break;
      }

      status = pmcOpen(hHal);
      if ( ! HAL_STATUS_SUCCESS( status ) ) {
         smsLog( pMac, LOGE,
                 "pmcOpen failed during initialization with status=%d", status );
         break;
      }

#ifndef WLAN_MDM_CODE_REDUCTION_OPT
      status = sme_QosOpen(pMac);
      if ( ! HAL_STATUS_SUCCESS( status ) ) {
         smsLog( pMac, LOGE,
                 "Qos open failed during initialization with status=%d", status );
         break;
      }

      status = btcOpen(pMac);
      if ( ! HAL_STATUS_SUCCESS( status ) ) {
         smsLog( pMac, LOGE,
                 "btcOpen open failed during initialization with status=%d", status );
         break;
      }
#endif
#ifdef FEATURE_OEM_DATA_SUPPORT
      status = oemData_OemDataReqOpen(pMac);
      if ( ! HAL_STATUS_SUCCESS( status ) ) {
         smsLog(pMac, LOGE,
                "oemData_OemDataReqOpen failed during initialization with status=%d", status );
         break;
      }
#endif

      if(!HAL_STATUS_SUCCESS((status = initSmeCmdList(pMac))))
          break;

#ifdef WLAN_SOFTAP_FEATURE
      {
         v_PVOID_t pvosGCtx = vos_get_global_context(VOS_MODULE_ID_SAP, NULL);
         if ( NULL == pvosGCtx ){
            smsLog( pMac, LOGE, "WLANSAP_Open open failed during initialization");
            status = eHAL_STATUS_FAILURE;
            break;
         }

         status = WLANSAP_Open( pvosGCtx );
         if ( ! HAL_STATUS_SUCCESS( status ) ) {
             smsLog( pMac, LOGE,
                     "WLANSAP_Open open failed during initialization with status=%d", status );
             break;
         }
      }
#endif
#if defined WLAN_FEATURE_VOWIFI
      status = rrmOpen(pMac);
      if ( ! HAL_STATUS_SUCCESS( status ) ) {
         smsLog( pMac, LOGE,
                 "rrmOpen open failed during initialization with status=%d", status );
         break;
      }
#endif

#if defined WLAN_FEATURE_VOWIFI_11R
      sme_FTOpen(pMac);
#endif
#if defined WLAN_FEATURE_P2P
      sme_p2pOpen(pMac);
#endif

   }while (0);

   return status;
}

#ifdef WLAN_SOFTAP_FEATURE
/*                                                                          

                                                                        
                                               
                                       

                             

                                               
                                                                     
                                  

                                                                              

                                                                           
     
                                                                          */

eHalStatus sme_set11dinfo(tHalHandle hHal,  tpSmeConfigParams pSmeConfigParams)
{
   eHalStatus status = eHAL_STATUS_FAILURE;
   tpAniSirGlobal pMac = PMAC_STRUCT( hHal );

   if (NULL == pSmeConfigParams ) {
      smsLog( pMac, LOGE,
              "Empty config param structure for SME, nothing to update");
      return status;
   }

   status = csrSetChannels(hHal, &pSmeConfigParams->csrConfig );
   if ( ! HAL_STATUS_SUCCESS( status ) ) {
      smsLog( pMac, LOGE, "csrChangeDefaultConfigParam failed with status=%d\n",
              status );
   }
    return status;
}

/*                                                                          

                                                                             

                             

                                                      
                                                                            

                                                                       

                                                                          
     
                                                                          */

eHalStatus sme_getSoftApDomain(tHalHandle hHal,  v_REGDOMAIN_t *domainIdSoftAp)
{
   eHalStatus status = eHAL_STATUS_FAILURE;
   tpAniSirGlobal pMac = PMAC_STRUCT( hHal );

   if (NULL == domainIdSoftAp ) {
      smsLog( pMac, LOGE, "Uninitialized domain Id");
      return status;
   }    

   *domainIdSoftAp = pMac->scan.domainIdCurrent;
   status = eHAL_STATUS_SUCCESS;
    
   return status;
}


eHalStatus sme_setRegInfo(tHalHandle hHal,  tANI_U8 *apCntryCode)
{
   eHalStatus status = eHAL_STATUS_FAILURE;
   tpAniSirGlobal pMac = PMAC_STRUCT( hHal );

   if (NULL == apCntryCode ) {
      smsLog( pMac, LOGE, "Empty Country Code, nothing to update");
      return status;
   }

   status = csrSetRegInfo(hHal, apCntryCode );
   if ( ! HAL_STATUS_SUCCESS( status ) ) {
      smsLog( pMac, LOGE, "csrSetRegInfo failed with status=%d\n",
              status );
   }
    return status;
}

#endif
#ifdef FEATURE_WLAN_SCAN_PNO
/*                                                                          

                                                                          
 
                                                                      
                 

                             

                                               
  
                                                                           

                                                                        
     

                                                                            */
eHalStatus sme_UpdateChannelConfig(tHalHandle hHal)
{
  tpAniSirGlobal pMac = PMAC_STRUCT( hHal );

  pmcUpdateScanParams( pMac, &(pMac->roam.configParam), 
                      &pMac->scan.base20MHzChannels, FALSE);
  return eHAL_STATUS_SUCCESS;
}
#endif //                       

/*                                                                          

                                                                       

                                                                           
                                  

                                                                         

                             

                                               
                                                                     
                                  

                                                                              

                                                                           
     

                                                                            */
eHalStatus sme_UpdateConfig(tHalHandle hHal, tpSmeConfigParams pSmeConfigParams)
{
   eHalStatus status = eHAL_STATUS_FAILURE;
   tpAniSirGlobal pMac = PMAC_STRUCT( hHal );

   if (NULL == pSmeConfigParams ) {
      smsLog( pMac, LOGE,
              "Empty config param structure for SME, nothing to update");
      return status;
   }

   status = csrChangeDefaultConfigParam(pMac, &pSmeConfigParams->csrConfig);

   if ( ! HAL_STATUS_SUCCESS( status ) ) {
      smsLog( pMac, LOGE, "csrChangeDefaultConfigParam failed with status=%d\n",
              status );
   }
#if defined WLAN_FEATURE_P2P_INTERNAL
   status = p2pChangeDefaultConfigParam(pMac, &pSmeConfigParams->p2pConfig);

   if ( ! HAL_STATUS_SUCCESS( status ) ) {
      smsLog( pMac, LOGE, "p2pChangeDefaultConfigParam failed with status=%d\n",
              status );
   }
#endif
#if defined WLAN_FEATURE_VOWIFI
   status = rrmChangeDefaultConfigParam(hHal, &pSmeConfigParams->rrmConfig);

   if ( ! HAL_STATUS_SUCCESS( status ) ) {
      smsLog( pMac, LOGE, "rrmChangeDefaultConfigParam failed with status=%d\n",
              status );
   }
#endif
   //                                
   //                                                                                          
   if(
#ifdef FEATURE_WLAN_NON_INTEGRATED_SOC
      //                                                                      
      //                                
      SME_IS_READY(pMac) && 
#endif
      csrIsAllSessionDisconnected( pMac) )
   {
       csrSetGlobalCfgs(pMac);
   }

   return status;
}

/*                                                                            
                              
                                                                             
                                

                                                                         
                                                                              
                                                                            
                                                                         

                               

                                                 

          
                                                                              
                                                                             
                                                                              
                                                                        

                      
                                                                             */
eHalStatus sme_ChangeConfigParams(tHalHandle hHal,
                                 tCsrUpdateConfigParam *pUpdateConfigParam)
{
   eHalStatus status = eHAL_STATUS_FAILURE;
   tpAniSirGlobal pMac = PMAC_STRUCT( hHal );

   if (NULL == pUpdateConfigParam ) {
      smsLog( pMac, LOGE,
              "Empty config param structure for SME, nothing to reset\n");
      return status;
   }

   status = csrChangeConfigParams(pMac, pUpdateConfigParam);

   if ( ! HAL_STATUS_SUCCESS( status ) ) {
      smsLog( pMac, LOGE, "csrUpdateConfigParam failed with status=%d\n",
              status );
   }

   return status;

}

/*                                                                          

                                                                             
                                

                                                                                
                                  

                             
                                               

                                                                    
                                             

                                                                  
     

                                                                            */
eHalStatus sme_HDDReadyInd(tHalHandle hHal)
{
   tSirSmeReadyReq Msg;
   eHalStatus status = eHAL_STATUS_FAILURE;
   tPmcPowerState powerState;
   tPmcSwitchState hwWlanSwitchState;
   tPmcSwitchState swWlanSwitchState;
   tpAniSirGlobal pMac = PMAC_STRUCT( hHal );

   do
   {
#ifdef FEATURE_WLAN_NON_INTEGRATED_SOC
      csrSetGlobalCfgs( pMac );
#endif

      Msg.messageType = eWNI_SME_SYS_READY_IND;
      Msg.length      = sizeof( tSirSmeReadyReq );

      if (eSIR_FAILURE != uMacPostCtrlMsg( hHal, (tSirMbMsg*)&Msg ))
      {
         status = eHAL_STATUS_SUCCESS;
      }
      else
      {
         smsLog( pMac, LOGE,
                 "uMacPostCtrlMsg failed to send eWNI_SME_SYS_READY_IND");
         break;
      }

      status = pmcQueryPowerState( hHal, &powerState,
                                &hwWlanSwitchState, &swWlanSwitchState );
      if ( ! HAL_STATUS_SUCCESS( status ) )
      {
         smsLog( pMac, LOGE, "pmcQueryPowerState failed with status=%d\n",
                 status );
         break;
      }

      if ( (ePMC_SWITCH_OFF != hwWlanSwitchState) &&
           (ePMC_SWITCH_OFF != swWlanSwitchState) )
      {
         status = csrReady(pMac);
         if ( ! HAL_STATUS_SUCCESS( status ) )
         {
            smsLog( pMac, LOGE, "csrReady failed with status=%d\n", status );
            break;
         }
         status = pmcReady(hHal);
         if ( ! HAL_STATUS_SUCCESS( status ) )
         {
             smsLog( pMac, LOGE, "pmcReady failed with status=%d\n", status );
             break;
         }
#ifndef WLAN_MDM_CODE_REDUCTION_OPT
         if(VOS_STATUS_SUCCESS != btcReady(hHal))
         {
             status = eHAL_STATUS_FAILURE;
             smsLog( pMac, LOGE, "btcReady failed\n");
             break;
         }
#endif

#if defined WLAN_FEATURE_VOWIFI
         if(VOS_STATUS_SUCCESS != rrmReady(hHal))
         {
             status = eHAL_STATUS_FAILURE;
             smsLog( pMac, LOGE, "rrmReady failed\n");
             break;
         }
#endif
      }
      pMac->sme.state = SME_STATE_READY;
   } while( 0 );

   return status;
}

/*                                                                          

                                                          

                                                                    
                                                    
                             
                                               

                                             

                                                   
     

                                                                            */
eHalStatus sme_Start(tHalHandle hHal)
{
   eHalStatus status = eHAL_STATUS_FAILURE;
   tpAniSirGlobal pMac = PMAC_STRUCT( hHal );

   do
   {
#ifdef FEATURE_WLAN_NON_INTEGRATED_SOC 
      ccmStart(hHal);
#endif
      status = csrStart(pMac);
      if ( ! HAL_STATUS_SUCCESS( status ) ) {
         smsLog( pMac, LOGE, "csrStart failed during smeStart with status=%d\n",
                 status );
         break;
      }

      status = pmcStart(hHal);
      if ( ! HAL_STATUS_SUCCESS( status ) ) {
         smsLog( pMac, LOGE, "pmcStart failed during smeStart with status=%d\n",
                 status );
         break;
      }

#ifdef WLAN_SOFTAP_FEATURE
      status = WLANSAP_Start(vos_get_global_context(VOS_MODULE_ID_SAP, NULL));
      if ( ! HAL_STATUS_SUCCESS( status ) ) {
         smsLog( pMac, LOGE, "WLANSAP_Start failed during smeStart with status=%d\n",
                 status );
         break;
      }
#endif
      pMac->sme.state = SME_STATE_START;
   }while (0);

   return status;
}


#ifdef WLAN_FEATURE_PACKET_FILTERING
/*                                                                             
 
                                             
 
              
                                                                 
 
             
                                 
                                                   
 
                     
 
                                                                             */
eHalStatus sme_PCFilterMatchCountResponseHandler(tHalHandle hHal, void* pMsg)
{
    tpAniSirGlobal pMac = PMAC_STRUCT(hHal);
    eHalStatus status = eHAL_STATUS_SUCCESS;
    tpSirRcvFltPktMatchRsp pRcvFltPktMatchRsp = (tpSirRcvFltPktMatchRsp)pMsg;

    if (NULL == pMsg)
    {
        smsLog(pMac, LOGE, "in %s msg ptr is NULL\n", __FUNCTION__);
        status = eHAL_STATUS_FAILURE;
    }
    else
    {
        smsLog(pMac, LOG2, "SME: entering "
            "sme_FilterMatchCountResponseHandler\n");

        /*                                                             */
        if (pMac->pmc.FilterMatchCountCB != NULL)
           pMac->pmc.FilterMatchCountCB(pMac->pmc.FilterMatchCountCBContext,
                                          pRcvFltPktMatchRsp);

        smsLog(pMac, LOG1, "%s: status=0x%x", __FUNCTION__,
               pRcvFltPktMatchRsp->status);

        pMac->pmc.FilterMatchCountCB = NULL;
        pMac->pmc.FilterMatchCountCBContext = NULL;
    }

    return(status);
}
#endif //                              


/*                                                                          

                                                               

                                                                          
                   

                             
                                               
                                                                   

                                                                     

                                                                      
     

                                                                            */
eHalStatus sme_ProcessMsg(tHalHandle hHal, vos_msg_t* pMsg)
{
   eHalStatus status = eHAL_STATUS_FAILURE;
   tpAniSirGlobal pMac = PMAC_STRUCT( hHal );

   if (pMsg == NULL) {
      smsLog( pMac, LOGE, "Empty message for SME, nothing to process\n");
      return status;
   }

   status = sme_AcquireGlobalLock( &pMac->sme );
   if ( HAL_STATUS_SUCCESS( status ) )
   {
      if( SME_IS_START(pMac) )
      {
          switch (pMsg->type) { //                                                                                          
          case eWNI_PMC_ENTER_BMPS_RSP:
          case eWNI_PMC_EXIT_BMPS_RSP:
          case eWNI_PMC_EXIT_BMPS_IND:
          case eWNI_PMC_ENTER_IMPS_RSP:
          case eWNI_PMC_EXIT_IMPS_RSP:
          case eWNI_PMC_SMPS_STATE_IND:
          case eWNI_PMC_ENTER_UAPSD_RSP:
          case eWNI_PMC_EXIT_UAPSD_RSP:
          case eWNI_PMC_ENTER_WOWL_RSP:
          case eWNI_PMC_EXIT_WOWL_RSP:
             //   
             if (pMsg->bodyptr)
             {
                pmcMessageProcessor(hHal, pMsg->bodyptr);
                status = eHAL_STATUS_SUCCESS;
                vos_mem_free( pMsg->bodyptr );
             } else {
                smsLog( pMac, LOGE, "Empty rsp message for PMC, nothing to process\n");
             }
             break;

          case WNI_CFG_SET_CNF:
          case WNI_CFG_DNLD_CNF:
          case WNI_CFG_GET_RSP:
          case WNI_CFG_ADD_GRP_ADDR_CNF:
          case WNI_CFG_DEL_GRP_ADDR_CNF:
             //   
             if (pMsg->bodyptr)
             {
                ccmCfgCnfMsgHandler(hHal, pMsg->bodyptr);
                status = eHAL_STATUS_SUCCESS;
                vos_mem_free( pMsg->bodyptr );
             } else {
                smsLog( pMac, LOGE, "Empty rsp message for CCM, nothing to process\n");
             }
             break;

          case eWNI_SME_ADDTS_RSP:
          case eWNI_SME_DELTS_RSP:
          case eWNI_SME_DELTS_IND:
#ifdef WLAN_FEATURE_VOWIFI_11R
          case eWNI_SME_FT_AGGR_QOS_RSP:
#endif             
             //   
             if (pMsg->bodyptr)
             {
#ifndef WLAN_MDM_CODE_REDUCTION_OPT
                status = sme_QosMsgProcessor(pMac, pMsg->type, pMsg->bodyptr);
                vos_mem_free( pMsg->bodyptr );
#endif
             } else {
                smsLog( pMac, LOGE, "Empty rsp message for QoS, nothing to process\n");
             }
             break;
#if defined WLAN_FEATURE_VOWIFI
          case eWNI_SME_NEIGHBOR_REPORT_IND:
          case eWNI_SME_BEACON_REPORT_REQ_IND:
#if defined WLAN_VOWIFI_DEBUG
             smsLog( pMac, LOGE, "Received RRM message. Message Id = %d\n", pMsg->type );
#endif
             if ( pMsg->bodyptr )
             {
                status = sme_RrmMsgProcessor( pMac, pMsg->type, pMsg->bodyptr );
                vos_mem_free( pMsg->bodyptr );
             }
             else
             {
                smsLog( pMac, LOGE, "Empty message for RRM, nothing to process\n");
             }
             break;
#endif

#ifdef FEATURE_OEM_DATA_SUPPORT
          //                                 
          case eWNI_SME_OEM_DATA_RSP:
                if(pMsg->bodyptr)
                {
                        status = sme_HandleOemDataRsp(pMac, pMsg->bodyptr);
                        vos_mem_free(pMsg->bodyptr);
                }
                else
                {
                        smsLog( pMac, LOGE, "Empty rsp message for oemData_ (eWNI_SME_OEM_DATA_RSP), nothing to process\n");
                }
                smeProcessPendingQueue( pMac );
                break;
#endif

          case eWNI_SME_ADD_STA_SELF_RSP:
                if(pMsg->bodyptr)
                {
                   status = csrProcessAddStaSessionRsp(pMac, pMsg->bodyptr);
                   vos_mem_free(pMsg->bodyptr);
                }
                else
                {
                   smsLog( pMac, LOGE, "Empty rsp message for meas (eWNI_SME_ADD_STA_SELF_RSP), nothing to process\n");
                }
                break;
          case eWNI_SME_DEL_STA_SELF_RSP:
                if(pMsg->bodyptr)
                {
                   status = csrProcessDelStaSessionRsp(pMac, pMsg->bodyptr);
                   vos_mem_free(pMsg->bodyptr);
                }
                else
                {
                   smsLog( pMac, LOGE, "Empty rsp message for meas (eWNI_SME_DEL_STA_SELF_RSP), nothing to process\n");
                }
                break;
#ifdef WLAN_FEATURE_P2P
          case eWNI_SME_REMAIN_ON_CHN_RSP:
                if(pMsg->bodyptr)
                {
                    status = sme_remainOnChnRsp(pMac, pMsg->bodyptr);
                    vos_mem_free(pMsg->bodyptr);
                }
                else
                {
                    smsLog( pMac, LOGE, "Empty rsp message for meas (eWNI_SME_REMAIN_ON_CHN_RSP), nothing to process\n");
                }
                break;
          case eWNI_SME_REMAIN_ON_CHN_RDY_IND:
                if(pMsg->bodyptr)
                {
                    status = sme_remainOnChnReady(pMac, pMsg->bodyptr);
                    vos_mem_free(pMsg->bodyptr);
                }
                else
                {
                    smsLog( pMac, LOGE, "Empty rsp message for meas (eWNI_SME_REMAIN_ON_CHN_RDY_IND), nothing to process\n");
                }
                break;
           case eWNI_SME_MGMT_FRM_IND:
                if(pMsg->bodyptr)
                {
                    sme_mgmtFrmInd(pMac, pMsg->bodyptr);
                    vos_mem_free(pMsg->bodyptr);
                }
                else
                { 
                    smsLog( pMac, LOGE, "Empty rsp message for meas (eWNI_SME_MGMT_FRM_IND), nothing to process\n");
                }
                break;
           case eWNI_SME_ACTION_FRAME_SEND_CNF:
                if(pMsg->bodyptr)
                {
                    status = sme_sendActionCnf(pMac, pMsg->bodyptr);
                    vos_mem_free(pMsg->bodyptr);
                }
                else
                {
                    smsLog( pMac, LOGE, "Empty rsp message for meas (eWNI_SME_ACTION_FRAME_SEND_CNF), nothing to process\n");
                }
                break;
#endif
          case eWNI_SME_COEX_IND:
                if(pMsg->bodyptr)
                {
                   status = btcHandleCoexInd((void *)pMac, pMsg->bodyptr);
                   vos_mem_free(pMsg->bodyptr);
                }
                else
                {
                   smsLog(pMac, LOGE, "Empty rsp message for meas (eWNI_SME_COEX_IND), nothing to process\n");
                }
                break;    

#ifdef FEATURE_WLAN_SCAN_PNO
          case eWNI_SME_PREF_NETWORK_FOUND_IND:
                if(pMsg->bodyptr)
                {
                   status = sme_PreferredNetworkFoundInd((void *)pMac, pMsg->bodyptr);
                   vos_mem_free(pMsg->bodyptr);
                }
                else
                {
                   smsLog(pMac, LOGE, "Empty rsp message for meas (eWNI_SME_PREF_NETWORK_FOUND_IND), nothing to process\n");
                }
                break;
#endif //                      
          
          case eWNI_SME_TX_PER_HIT_IND:
                if (pMac->sme.pTxPerHitCallback)
                {
                   pMac->sme.pTxPerHitCallback(pMac->sme.pTxPerHitCbContext);
                }
                break;

          case eWNI_SME_CHANGE_COUNTRY_CODE:
                if(pMsg->bodyptr)
                {
                   status = sme_HandleChangeCountryCode((void *)pMac, pMsg->bodyptr);
                   vos_mem_free(pMsg->bodyptr);
                }
                else
                {
                   smsLog(pMac, LOGE, "Empty rsp message for meas (eWNI_SME_COEX_IND), nothing to process\n");
                }
                break;

#ifdef WLAN_FEATURE_PACKET_FILTERING
          case eWNI_PMC_PACKET_COALESCING_FILTER_MATCH_COUNT_RSP:
                if(pMsg->bodyptr)
                {
                   status = sme_PCFilterMatchCountResponseHandler((void *)pMac, pMsg->bodyptr);
                   vos_mem_free(pMsg->bodyptr);
                }
                else
                {
                   smsLog(pMac, LOGE, "Empty rsp message for meas "
                          "(PACKET_COALESCING_FILTER_MATCH_COUNT_RSP), nothing to process\n");
                }
                break;
#endif //                              
          case eWNI_SME_PRE_SWITCH_CHL_IND:
             {
                status = sme_HandlePreChannelSwitchInd(pMac);
                break;
             }
                
          case eWNI_SME_POST_SWITCH_CHL_IND:
             {
                status = sme_HandlePostChannelSwitchInd(pMac);
                break;
             }

#ifdef WLAN_WAKEUP_EVENTS
          case eWNI_SME_WAKE_REASON_IND:
                if(pMsg->bodyptr)
                {
                   status = sme_WakeReasonIndCallback((void *)pMac, pMsg->bodyptr);
                   vos_mem_free(pMsg->bodyptr);
                }
                else
                {
                   smsLog(pMac, LOGE, "Empty rsp message for meas (eWNI_SME_WAKE_REASON_IND), nothing to process\n");
                }
                break;
#endif //                   

          default:

             if ( ( pMsg->type >= eWNI_SME_MSG_TYPES_BEGIN )
                  &&  ( pMsg->type <= eWNI_SME_MSG_TYPES_END ) )
             {
                //   
                if (pMsg->bodyptr)
                {
                   status = csrMsgProcessor(hHal, pMsg->bodyptr);
                   vos_mem_free( pMsg->bodyptr );
                }
                else
                {
                   smsLog( pMac, LOGE, "Empty rsp message for CSR, nothing to process\n");
                }
             }
             else
             {
                smsLog( pMac, LOGW, "Unknown message type %d, nothing to process\n",
                        pMsg->type);
                if (pMsg->bodyptr)
                {
                   vos_mem_free( pMsg->bodyptr );
                }
             }
          }//      
      } //            
      else
      {
         smsLog( pMac, LOGW, "message type %d in stop state ignored\n", pMsg->type);
         if (pMsg->bodyptr)
         {
            vos_mem_free( pMsg->bodyptr );
         }
      }
      sme_ReleaseGlobalLock( &pMac->sme );
   }
   else
   {
      smsLog( pMac, LOGW, "Locking failed, bailing out\n");
      if (pMsg->bodyptr)
      {
          vos_mem_free( pMsg->bodyptr );
      }
   }

   return status;
}


//                                                                             
//               
v_VOID_t sme_FreeMsg( tHalHandle hHal, vos_msg_t* pMsg )
{
   if( pMsg )
   {
      if (pMsg->bodyptr)
      {
         vos_mem_free( pMsg->bodyptr );
      }
   }

}


/*                                                                          

                                                                     

                                                                   
                                                       

                             
                                              

                                               

                                                                          
                                  
     

                                                                            */
eHalStatus sme_Stop(tHalHandle hHal, tANI_BOOLEAN pmcFlag)
{
   eHalStatus status = eHAL_STATUS_FAILURE;
   eHalStatus fail_status = eHAL_STATUS_SUCCESS;
   tpAniSirGlobal pMac = PMAC_STRUCT( hHal );

#ifdef WLAN_SOFTAP_FEATURE
   status = WLANSAP_Stop(vos_get_global_context(VOS_MODULE_ID_SAP, NULL));
   if ( ! HAL_STATUS_SUCCESS( status ) ) {
      smsLog( pMac, LOGE, "WLANSAP_Stop failed during smeStop with status=%d\n",
                          status );
      fail_status = status;
   }
#endif

   p2pStop(hHal);

   if(pmcFlag)
   {
      status = pmcStop(hHal);
      if ( ! HAL_STATUS_SUCCESS( status ) ) {
         smsLog( pMac, LOGE, "pmcStop failed during smeStop with status=%d\n",
                 status );
         fail_status = status;
      }
   }

   status = csrStop(pMac);
   if ( ! HAL_STATUS_SUCCESS( status ) ) {
      smsLog( pMac, LOGE, "csrStop failed during smeStop with status=%d\n",
              status );
      fail_status = status;
   }

   ccmStop(hHal);

   purgeSmeCmdList(pMac);

   if (!HAL_STATUS_SUCCESS( fail_status )) {
      status = fail_status;
   }

   pMac->sme.state = SME_STATE_STOP;

   return status;
}

/*                                                                          

                                                                   

                                                                     
                                          

                                                            
                                          
                             
                                              

                                                          

                                                                               
                                                      
     

                                                                            */
eHalStatus sme_Close(tHalHandle hHal)
{
   eHalStatus status = eHAL_STATUS_FAILURE;
   eHalStatus fail_status = eHAL_STATUS_SUCCESS;
   tpAniSirGlobal pMac = PMAC_STRUCT( hHal );

   status = csrClose(pMac);
   if ( ! HAL_STATUS_SUCCESS( status ) ) {
      smsLog( pMac, LOGE, "csrClose failed during sme close with status=%d\n",
              status );
      fail_status = status;
   }

#ifdef WLAN_SOFTAP_FEATURE
   status = WLANSAP_Close(vos_get_global_context(VOS_MODULE_ID_SAP, NULL));
   if ( ! HAL_STATUS_SUCCESS( status ) ) {
      smsLog( pMac, LOGE, "WLANSAP_close failed during sme close with status=%d\n",
              status );
      fail_status = status;
   }
#endif

#ifndef WLAN_MDM_CODE_REDUCTION_OPT
   status = btcClose(hHal);
   if ( ! HAL_STATUS_SUCCESS( status ) ) {
      smsLog( pMac, LOGE, "BTC close failed during sme close with status=%d\n",
              status );
      fail_status = status;
   }

   status = sme_QosClose(pMac);
   if ( ! HAL_STATUS_SUCCESS( status ) ) {
      smsLog( pMac, LOGE, "Qos close failed during sme close with status=%d\n",
              status );
      fail_status = status;
   }
#endif
#ifdef FEATURE_OEM_DATA_SUPPORT
   status = oemData_OemDataReqClose(hHal);
   if ( ! HAL_STATUS_SUCCESS( status ) ) {
       smsLog( pMac, LOGE, "OEM DATA REQ close failed during sme close with status=%d\n", 
              status );
      fail_status = status;
   }
#endif

   status = ccmClose(hHal);
         if ( ! HAL_STATUS_SUCCESS( status ) ) {
      smsLog( pMac, LOGE, "ccmClose failed during sme close with status=%d\n",
                 status );
             fail_status = status;
         }

   status = pmcClose(hHal);
   if ( ! HAL_STATUS_SUCCESS( status ) ) {
      smsLog( pMac, LOGE, "pmcClose failed during sme close with status=%d\n",
              status );
      fail_status = status;
   }
#if defined WLAN_FEATURE_VOWIFI
   status = rrmClose(hHal);
   if ( ! HAL_STATUS_SUCCESS( status ) ) {
      smsLog( pMac, LOGE, "RRM close failed during sme close with status=%d\n",
              status );
      fail_status = status;
   }
#endif

#if defined WLAN_FEATURE_VOWIFI_11R
   sme_FTClose(hHal);
#endif
#if defined WLAN_FEATURE_P2P
   sme_p2pClose(hHal);
#endif

   freeSmeCmdList(pMac);

   if( !VOS_IS_STATUS_SUCCESS( vos_lock_destroy( &pMac->sme.lkSmeGlobalLock ) ) )
   {
       fail_status = eHAL_STATUS_FAILURE;
   }

   if (!HAL_STATUS_SUCCESS( fail_status )) {
      status = fail_status;
   }

   pMac->sme.state = SME_STATE_STOP;

   return status;
}
#ifdef FEATURE_WLAN_LFR
tANI_BOOLEAN csrIsScanAllowed(tpAniSirGlobal pMac)
{
#if 0
        switch(pMac->roam.neighborRoamInfo.neighborRoamState) {
                case eCSR_NEIGHBOR_ROAM_STATE_REPORT_SCAN:
                case eCSR_NEIGHBOR_ROAM_STATE_PREAUTHENTICATING:
                case eCSR_NEIGHBOR_ROAM_STATE_PREAUTH_DONE:
                case eCSR_NEIGHBOR_ROAM_STATE_REASSOCIATING:
                        return eANI_BOOLEAN_FALSE;
                default:
                        return eANI_BOOLEAN_TRUE;
        }
#else
        /*
                                                 
                                                 
                                             
         */
        return eANI_BOOLEAN_TRUE;
#endif
}
#endif
/*                                                                            
                       
                                                                     
                                 
                                                                           
                                                                               
                                                               
                                                          
                      
                                                                             */
eHalStatus sme_ScanRequest(tHalHandle hHal, tANI_U8 sessionId, tCsrScanRequest *pscanReq, 
                           tANI_U32 *pScanRequestID, 
                           csrScanCompleteCallback callback, void *pContext)
{
    eHalStatus status = eHAL_STATUS_FAILURE;
    tpAniSirGlobal pMac = PMAC_STRUCT( hHal );

    smsLog(pMac, LOG2, FL("enter"));
    do
    {
        if(pMac->scan.fScanEnable)
        {
            status = sme_AcquireGlobalLock( &pMac->sme );
            if ( HAL_STATUS_SUCCESS( status ) )
            {
                {
#ifdef FEATURE_WLAN_LFR
                    if(csrIsScanAllowed(pMac)) {
#endif
                            status = csrScanRequest( hHal, sessionId, pscanReq,
                                                     pScanRequestID, callback, pContext );
#ifdef FEATURE_WLAN_LFR
                    } else {
                            /*                                                   */
                            status = eHAL_STATUS_RESOURCES;
                    }
#endif
                }
                  
                sme_ReleaseGlobalLock( &pMac->sme );
            } //                             
        } //                          
    } while( 0 );

    return (status);


}

/*                                                                            
                         
                                                               
                               
                                                                        
                                               
                      
                                                                             */
eHalStatus sme_ScanGetResult(tHalHandle hHal, tANI_U8 sessionId, tCsrScanResultFilter *pFilter,
                            tScanResultHandle *phResult)
{
   eHalStatus status = eHAL_STATUS_FAILURE;
   tpAniSirGlobal pMac = PMAC_STRUCT( hHal );

   smsLog(pMac, LOG2, FL("enter"));
   status = sme_AcquireGlobalLock( &pMac->sme );
   if ( HAL_STATUS_SUCCESS( status ) )
   {
       status = csrScanGetResult( hHal, pFilter, phResult );
       sme_ReleaseGlobalLock( &pMac->sme );
   }
   smsLog(pMac, LOG2, FL("exit status %d"), status);

   return (status);
}


/*                                                                            
                           
                                                                   
                               
                      
                                                                             */
eHalStatus sme_ScanFlushResult(tHalHandle hHal, tANI_U8 sessionId)
{
   eHalStatus status = eHAL_STATUS_FAILURE;
   tpAniSirGlobal pMac = PMAC_STRUCT( hHal );

   status = sme_AcquireGlobalLock( &pMac->sme );
   if ( HAL_STATUS_SUCCESS( status ) )
   {
       status = csrScanFlushResult( hHal );
       sme_ReleaseGlobalLock( &pMac->sme );
   }

   return (status);
}

eHalStatus sme_ScanFlushP2PResult(tHalHandle hHal, tANI_U8 sessionId)
{
        eHalStatus status = eHAL_STATUS_FAILURE;
        tpAniSirGlobal pMac = PMAC_STRUCT( hHal );

        status = sme_AcquireGlobalLock( &pMac->sme );
        if ( HAL_STATUS_SUCCESS( status ) )
        {
                status = csrScanFlushP2PResult( hHal );
                sme_ReleaseGlobalLock( &pMac->sme );
        }

        return (status);
}

/*                                                                            
                              
                                                                            
                       
                               
                                                       
                                                    
                                                                             */
tCsrScanResultInfo *sme_ScanResultGetFirst(tHalHandle hHal,
                                          tScanResultHandle hScanResult)
{
   eHalStatus status = eHAL_STATUS_FAILURE;
   tpAniSirGlobal pMac = PMAC_STRUCT( hHal );
   tCsrScanResultInfo *pRet = NULL;

   status = sme_AcquireGlobalLock( &pMac->sme );
   if ( HAL_STATUS_SUCCESS( status ) )
   {
       pRet = csrScanResultGetFirst( pMac, hScanResult );
       sme_ReleaseGlobalLock( &pMac->sme );
   }

   return (pRet);
}


/*                                                                            
                             
                                                                           
                                                                              
                
                               
                                                       
                                              
                                                                             */
tCsrScanResultInfo *sme_ScanResultGetNext(tHalHandle hHal,
                                          tScanResultHandle hScanResult)
{
    eHalStatus status = eHAL_STATUS_FAILURE;
    tpAniSirGlobal pMac = PMAC_STRUCT( hHal );
    tCsrScanResultInfo *pRet = NULL;

    status = sme_AcquireGlobalLock( &pMac->sme );
    if ( HAL_STATUS_SUCCESS( status ) )
    {
        pRet = csrScanResultGetNext( pMac, hScanResult );
        sme_ReleaseGlobalLock( &pMac->sme );
    }

    return (pRet);
}


/*                                                                            
                               
                                                                        
                               
                                               
                      
                                                                             */
eHalStatus sme_ScanSetBGScanparams(tHalHandle hHal, tANI_U8 sessionId, tCsrBGScanRequest *pScanReq)
{
    eHalStatus status = eHAL_STATUS_FAILURE;
    tpAniSirGlobal pMac = PMAC_STRUCT( hHal );

    if( NULL != pScanReq )
    {
        status = sme_AcquireGlobalLock( &pMac->sme );
        if ( HAL_STATUS_SUCCESS( status ) )
        {
            status = csrScanSetBGScanparams( hHal, pScanReq );
            sme_ReleaseGlobalLock( &pMac->sme );
        }
    }

    return (status);
}


/*                                                                            
                           
                                                                                
                                                    
                               
                                                                       
                                           
                                                                
                      
                                                                             */
eHalStatus sme_ScanResultPurge(tHalHandle hHal, tScanResultHandle hScanResult)
{
   eHalStatus status = eHAL_STATUS_FAILURE;
   tpAniSirGlobal pMac = PMAC_STRUCT( hHal );

   status = sme_AcquireGlobalLock( &pMac->sme );
   if ( HAL_STATUS_SUCCESS( status ) )
   {
       status = csrScanResultPurge( hHal, hScanResult );
       sme_ReleaseGlobalLock( &pMac->sme );
   }

   return (status);
}

/*                                                                            
                                     
                                                                
                               
                                                                    
                                           
                                                                   
                                                                            
                                                                           
                                 
                                                                                
                                                 
                                          
                                                        
                                                      
                                                                             */
eHalStatus sme_ScanGetPMKIDCandidateList(tHalHandle hHal, tANI_U8 sessionId,
                                        tPmkidCandidateInfo *pPmkidList, 
                                        tANI_U32 *pNumItems )
{
   eHalStatus status = eHAL_STATUS_FAILURE;
   tpAniSirGlobal pMac = PMAC_STRUCT( hHal );

   status = sme_AcquireGlobalLock( &pMac->sme );
   if ( HAL_STATUS_SUCCESS( status ) )
   {
       status = csrScanGetPMKIDCandidateList( pMac, sessionId, pPmkidList, pNumItems );
       sme_ReleaseGlobalLock( &pMac->sme );
   }

   return (status);
}

/*                                                                            
                                            

        
                                                                             
                           

                                                  
                                                                    

                                                         

                                              
                             

                              

                    
                                                                             */
eHalStatus sme_RoamRegisterLinkQualityIndCallback(tHalHandle hHal, tANI_U8 sessionId,
                                                  csrRoamLinkQualityIndCallback   callback,
                                                  void                           *pContext)
{
   return(csrRoamRegisterLinkQualityIndCallback((tpAniSirGlobal)hHal, callback, pContext));
}

/*                                                                            
                                
                                                                           
                                                                       
                                                                                 
                                                          
                      
                                                                             */
eHalStatus sme_RoamRegisterCallback(tHalHandle hHal,
                                    csrRoamCompleteCallback callback,
                                    void *pContext)
{
   return(csrRoamRegisterCallback((tpAniSirGlobal)hHal, callback, pContext));
}

eCsrPhyMode sme_GetPhyMode(tHalHandle hHal)
{
    tpAniSirGlobal pMac = PMAC_STRUCT(hHal);
    return pMac->roam.configParam.phyMode;
}

/*                                                                            
                       
                                                                         
                                  
                                                                 
                                                                    
                                                                           
                                             
                                               
                      
                                                                             */
eHalStatus sme_RoamConnect(tHalHandle hHal, tANI_U8 sessionId, tCsrRoamProfile *pProfile,
                           tANI_U32 *pRoamId)
{
    eHalStatus status = eHAL_STATUS_FAILURE;
    tpAniSirGlobal pMac = PMAC_STRUCT( hHal );

    smsLog(pMac, LOG2, FL("enter"));
    status = sme_AcquireGlobalLock( &pMac->sme );
    if ( HAL_STATUS_SUCCESS( status ) )
    {
        if( CSR_IS_SESSION_VALID( pMac, sessionId ) )
        {
            status = csrRoamConnect( pMac, sessionId, pProfile, NULL, pRoamId );
        }
        else
        {
            status = eHAL_STATUS_INVALID_PARAMETER;
        }
        sme_ReleaseGlobalLock( &pMac->sme );
    }

    return (status);
}

/*                                                                            
                       
                                                                           
                                                                             
                                                                                
           
                                                                               
                                                                              
                           
                                               
                      
                                                                                 */
eHalStatus sme_RoamReassoc(tHalHandle hHal, tANI_U8 sessionId, tCsrRoamProfile *pProfile,
                          tCsrRoamModifyProfileFields modProfileFields,
                          tANI_U32 *pRoamId)
{
    eHalStatus status = eHAL_STATUS_FAILURE;
    tpAniSirGlobal pMac = PMAC_STRUCT( hHal );

    smsLog(pMac, LOG2, FL("enter"));
    status = sme_AcquireGlobalLock( &pMac->sme );
    if ( HAL_STATUS_SUCCESS( status ) )
    {
        if( CSR_IS_SESSION_VALID( pMac, sessionId ) )
        {
            status = csrRoamReassoc( pMac, sessionId, pProfile, modProfileFields, pRoamId );
        }
        else
        {
            status = eHAL_STATUS_INVALID_PARAMETER;
        }
        sme_ReleaseGlobalLock( &pMac->sme );
    }

    return (status);
}

/*                                                                            
                                    
                                                                             
                           
                                  
                                                              
                                                                             */
eHalStatus sme_RoamConnectToLastProfile(tHalHandle hHal, tANI_U8 sessionId)
{
   eHalStatus status = eHAL_STATUS_FAILURE;
   tpAniSirGlobal pMac = PMAC_STRUCT( hHal );

   status = sme_AcquireGlobalLock( &pMac->sme );
   if ( HAL_STATUS_SUCCESS( status ) )
   {
      if( CSR_IS_SESSION_VALID( pMac, sessionId ) )
      {
         status = csrRoamConnectToLastProfile( pMac, sessionId );
      }
      else
      {
          status = eHAL_STATUS_INVALID_PARAMETER;
      }
      sme_ReleaseGlobalLock( &pMac->sme );
   }

   return (status);
}

/*                                                                            
                          
                                                                         
                                  
                                                                              
                                                                 
                      
                                                                             */
eHalStatus sme_RoamDisconnect(tHalHandle hHal, tANI_U8 sessionId, eCsrRoamDisconnectReason reason)
{
   eHalStatus status = eHAL_STATUS_FAILURE;
   tpAniSirGlobal pMac = PMAC_STRUCT( hHal );

   smsLog(pMac, LOG2, FL("enter"));
   status = sme_AcquireGlobalLock( &pMac->sme );
   if ( HAL_STATUS_SUCCESS( status ) )
   {
      if( CSR_IS_SESSION_VALID( pMac, sessionId ) )
      {
         status = csrRoamDisconnect( pMac, sessionId, reason );
      }
      else
      {
          status = eHAL_STATUS_INVALID_PARAMETER;
      }
      sme_ReleaseGlobalLock( &pMac->sme );
   }

   return (status);
}

#ifdef WLAN_SOFTAP_FEATURE
/*                                                                            
                       
                                                                
                                  
                                          
                                                                                        
                                                                                 */
eHalStatus sme_RoamStopBss(tHalHandle hHal, tANI_U8 sessionId)
{
   eHalStatus status = eHAL_STATUS_FAILURE;
   tpAniSirGlobal pMac = PMAC_STRUCT( hHal );

   smsLog(pMac, LOG2, FL("enter"));
   status = sme_AcquireGlobalLock( &pMac->sme );
   if ( HAL_STATUS_SUCCESS( status ) )
   {
      if( CSR_IS_SESSION_VALID( pMac, sessionId ) )
      {
         status = csrRoamIssueStopBssCmd( pMac, sessionId, eANI_BOOLEAN_TRUE );
      }
      else
      {
          status = eHAL_STATUS_INVALID_PARAMETER;
      }
      sme_ReleaseGlobalLock( &pMac->sme );
   }

   return (status);
}

/*                                                                            
                             
                                                                  
                                  
                                          
                                                                                        
                                                                                        
                                                                                 */
eHalStatus sme_RoamDisconnectSta(tHalHandle hHal, tANI_U8 sessionId,
                                tANI_U8 *pPeerMacAddr)
{
   eHalStatus status = eHAL_STATUS_FAILURE;
   tpAniSirGlobal pMac = PMAC_STRUCT( hHal );

   if ( NULL == pMac )
   {
     VOS_ASSERT(0);
     return status;
   }

   status = sme_AcquireGlobalLock( &pMac->sme );
   if ( HAL_STATUS_SUCCESS( status ) )
   {
      if( CSR_IS_SESSION_VALID( pMac, sessionId ) )
      {
         status = csrRoamIssueDisassociateStaCmd( pMac, sessionId, pPeerMacAddr, 
                                                  eSIR_MAC_UNSPEC_FAILURE_REASON);
      }
      else
      {
         status = eHAL_STATUS_INVALID_PARAMETER;
      }
      sme_ReleaseGlobalLock( &pMac->sme );
   }

   return (status);
}

/*                                                                            
                         
                                                                  
                                  
                                          
                                                                                        
                                                                                        
                                                                                 */
eHalStatus sme_RoamDeauthSta(tHalHandle hHal, tANI_U8 sessionId,
                                tANI_U8 *pPeerMacAddr)
{
   eHalStatus status = eHAL_STATUS_FAILURE;
   tpAniSirGlobal pMac = PMAC_STRUCT( hHal );

   if ( NULL == pMac )
   {
     VOS_ASSERT(0);
     return status;
   }

   status = sme_AcquireGlobalLock( &pMac->sme );
   if ( HAL_STATUS_SUCCESS( status ) )
   {
      if( CSR_IS_SESSION_VALID( pMac, sessionId ) )
      {
         status = csrRoamIssueDeauthStaCmd( pMac, sessionId, pPeerMacAddr, 
                     eSIR_MAC_UNSPEC_FAILURE_REASON);
      }
      else
      {
         status = eHAL_STATUS_INVALID_PARAMETER;
      }
      sme_ReleaseGlobalLock( &pMac->sme );
   }

   return (status);
}

/*                                                                            
                                   
                                                                               
                                          
                                                                                        
                      
                                                                                 */
eHalStatus sme_RoamTKIPCounterMeasures(tHalHandle hHal, tANI_U8 sessionId,
                                        tANI_BOOLEAN bEnable)
{
   eHalStatus status = eHAL_STATUS_FAILURE;
   tpAniSirGlobal pMac = PMAC_STRUCT( hHal );

   if ( NULL == pMac )
   {
     VOS_ASSERT(0);
     return status;
   }

   status = sme_AcquireGlobalLock( &pMac->sme );
   if ( HAL_STATUS_SUCCESS( status ) )
   {
        if( CSR_IS_SESSION_VALID( pMac, sessionId ) )
        {
            status = csrRoamIssueTkipCounterMeasures( pMac, sessionId, bEnable);
        }
        else
        {
            status = eHAL_STATUS_INVALID_PARAMETER;
        }
        sme_ReleaseGlobalLock( &pMac->sme );
   }

   return (status);
}

/*                                                                            
                                 
                                                                                       
                                 
                                             
                                                                                        
                                                                                                        
                                            
                                                           
                                                                                           
                      
                                                                                 */
eHalStatus sme_RoamGetAssociatedStas(tHalHandle hHal, tANI_U8 sessionId,
                                        VOS_MODULE_ID modId, void *pUsrContext,
                                        void *pfnSapEventCallback, tANI_U8 *pAssocStasBuf)
{
   eHalStatus status = eHAL_STATUS_FAILURE;
   tpAniSirGlobal pMac = PMAC_STRUCT( hHal );

   if ( NULL == pMac )
   {
     VOS_ASSERT(0);
     return status;
   }

   status = sme_AcquireGlobalLock( &pMac->sme );
   if ( HAL_STATUS_SUCCESS( status ) )
   {
        if( CSR_IS_SESSION_VALID( pMac, sessionId ) )
        {
            status = csrRoamGetAssociatedStas( pMac, sessionId, modId, pUsrContext, pfnSapEventCallback, pAssocStasBuf );
        }
        else
        {
            status = eHAL_STATUS_INVALID_PARAMETER;
        }
        sme_ReleaseGlobalLock( &pMac->sme );
   }

   return (status);
}

/*                                                                            
                                    
                                                          
                                 
                                             
                                            
                                                           
                                                                               
                      
                                                                                 */
eHalStatus sme_RoamGetWpsSessionOverlap(tHalHandle hHal, tANI_U8 sessionId,
                                        void *pUsrContext, void 
                                        *pfnSapEventCallback, v_MACADDR_t pRemoveMac)
{
   eHalStatus status = eHAL_STATUS_FAILURE;
   tpAniSirGlobal pMac = PMAC_STRUCT( hHal );

   if ( NULL == pMac )
   {
     VOS_ASSERT(0);
     return status;
   }

   status = sme_AcquireGlobalLock( &pMac->sme );
   if ( HAL_STATUS_SUCCESS( status ) )
   {
        if( CSR_IS_SESSION_VALID( pMac, sessionId ) )
        {
            status = csrRoamGetWpsSessionOverlap( pMac, sessionId, pUsrContext, pfnSapEventCallback, pRemoveMac);
        }
        else
        {
            status = eHAL_STATUS_INVALID_PARAMETER;
        }
        sme_ReleaseGlobalLock( &pMac->sme );
   }

   return (status);
}

#endif

/*                                                                            
                               
                                                                                
                     
                                
                      
                                                                             */
eHalStatus sme_RoamGetConnectState(tHalHandle hHal, tANI_U8 sessionId, eCsrConnectState *pState)
{
    eHalStatus status = eHAL_STATUS_FAILURE;
    tpAniSirGlobal pMac = PMAC_STRUCT( hHal );

    status = sme_AcquireGlobalLock( &pMac->sme );
    if ( HAL_STATUS_SUCCESS( status ) )
    {
       if( CSR_IS_SESSION_VALID( pMac, sessionId ) )
       {
          status = csrRoamGetConnectState( pMac, sessionId, pState );
       }
       else
       {
           status = eHAL_STATUS_INVALID_PARAMETER;
       }
       sme_ReleaseGlobalLock( &pMac->sme );
    }

    return (status);
}

/*                                                                            
                                 
                                                                          
                                                                               
                                                                      
                                
                                                             
                                              
                                                
                                                                             */
eHalStatus sme_RoamGetConnectProfile(tHalHandle hHal, tANI_U8 sessionId,
                                     tCsrRoamConnectedProfile *pProfile)
{
   eHalStatus status = eHAL_STATUS_FAILURE;
   tpAniSirGlobal pMac = PMAC_STRUCT( hHal );

   status = sme_AcquireGlobalLock( &pMac->sme );
   if ( HAL_STATUS_SUCCESS( status ) )
   {
      if( CSR_IS_SESSION_VALID( pMac, sessionId ) )
      {
         status = csrRoamGetConnectProfile( pMac, sessionId, pProfile );
      }
      else
      {
          status = eHAL_STATUS_INVALID_PARAMETER;
      }
      sme_ReleaseGlobalLock( &pMac->sme );
   }

   return (status);
}

/*                                                                            
                                  
                                                                         
                                                                   
                                
                                                             
                                              
                       
                                                                             */
eHalStatus sme_RoamFreeConnectProfile(tHalHandle hHal,
                                      tCsrRoamConnectedProfile *pProfile)
{
   eHalStatus status = eHAL_STATUS_FAILURE;
   tpAniSirGlobal pMac = PMAC_STRUCT( hHal );

   status = sme_AcquireGlobalLock( &pMac->sme );
   if ( HAL_STATUS_SUCCESS( status ) )
   {
       status = csrRoamFreeConnectProfile( pMac, pProfile );
       sme_ReleaseGlobalLock( &pMac->sme );
   }

   return (status);
}

/*                                                                            
                             
                                                                               
                                
                                                                     
                                        
                                                                       
                                                                                
                                                             
                                                                                
                                                                   
                                         
                                                    
                                                  
                                                                             */
eHalStatus sme_RoamSetPMKIDCache( tHalHandle hHal, tANI_U8 sessionId, tPmkidCacheInfo *pPMKIDCache,
                                  tANI_U32 numItems )
{
   eHalStatus status = eHAL_STATUS_FAILURE;
   tpAniSirGlobal pMac = PMAC_STRUCT( hHal );

   status = sme_AcquireGlobalLock( &pMac->sme );
   if ( HAL_STATUS_SUCCESS( status ) )
   {
      if( CSR_IS_SESSION_VALID( pMac, sessionId ) )
      {
         status = csrRoamSetPMKIDCache( pMac, sessionId, pPMKIDCache, numItems );
      }
      else
      {
          status = eHAL_STATUS_INVALID_PARAMETER;
      }
      sme_ReleaseGlobalLock( &pMac->sme );
   }

   return (status);
}

/*                                                                            
                                
                                                                                    
                                                            
                                
                                                                               
                                                                           
                                                                            
                             
                                                                   
                                                                                
                                   
                                                                             */
eHalStatus sme_RoamGetSecurityReqIE(tHalHandle hHal, tANI_U8 sessionId, tANI_U32 *pLen,
                                  tANI_U8 *pBuf, eCsrSecurityType secType)
{
    eHalStatus status = eHAL_STATUS_FAILURE;
    tpAniSirGlobal pMac = PMAC_STRUCT( hHal );

    status = sme_AcquireGlobalLock( &pMac->sme );
    if ( HAL_STATUS_SUCCESS( status ) )
    {
        if( CSR_IS_SESSION_VALID( pMac, sessionId ) )
        {
           status = csrRoamGetWpaRsnReqIE( hHal, sessionId, pLen, pBuf );
        }
        else
        {
           status = eHAL_STATUS_INVALID_PARAMETER;
        }
        sme_ReleaseGlobalLock( &pMac->sme );
    }

    return (status);
}

/*                                                                            
                                
                                                                                     
                                               
                                
                                                                               
                                                                           
                                                                            
                             
                                                                   
                                                                                
                                   
                                                                             */
eHalStatus sme_RoamGetSecurityRspIE(tHalHandle hHal, tANI_U8 sessionId, tANI_U32 *pLen,
                                  tANI_U8 *pBuf, eCsrSecurityType secType)
{
    eHalStatus status = eHAL_STATUS_FAILURE;
    tpAniSirGlobal pMac = PMAC_STRUCT( hHal );

    status = sme_AcquireGlobalLock( &pMac->sme );
    if ( HAL_STATUS_SUCCESS( status ) )
    {
        if( CSR_IS_SESSION_VALID( pMac, sessionId ) )
        {
           status = csrRoamGetWpaRsnRspIE( pMac, sessionId, pLen, pBuf );
        }
        else
        {
           status = eHAL_STATUS_INVALID_PARAMETER;
        }
        sme_ReleaseGlobalLock( &pMac->sme );
    }

    return (status);

}


/*                                                                            
                                
                                                                            
                  
                                
                                                        
                                                                             */
tANI_U32 sme_RoamGetNumPMKIDCache(tHalHandle hHal, tANI_U8 sessionId)
{
    eHalStatus status = eHAL_STATUS_FAILURE;
    tpAniSirGlobal pMac = PMAC_STRUCT( hHal );
    tANI_U32 numPmkidCache = 0;

    status = sme_AcquireGlobalLock( &pMac->sme );
    if ( HAL_STATUS_SUCCESS( status ) )
    {
        if( CSR_IS_SESSION_VALID( pMac, sessionId ) )
        {
           numPmkidCache = csrRoamGetNumPMKIDCache( pMac, sessionId );
           status = eHAL_STATUS_SUCCESS;
        }
        else
        {
           status = eHAL_STATUS_INVALID_PARAMETER;
        }
        sme_ReleaseGlobalLock( &pMac->sme );
    }

    return (numPmkidCache);
}

/*                                                                            
                             
                                                                           
                                
                                                                             
                                                                             
                                                               
                                                                              
                                         
                                                                                
                                   
                                                                             */
eHalStatus sme_RoamGetPMKIDCache(tHalHandle hHal, tANI_U8 sessionId, tANI_U32 *pNum,
                                 tPmkidCacheInfo *pPmkidCache)
{
   eHalStatus status = eHAL_STATUS_FAILURE;
   tpAniSirGlobal pMac = PMAC_STRUCT( hHal );

   status = sme_AcquireGlobalLock( &pMac->sme );
   if ( HAL_STATUS_SUCCESS( status ) )
   {
       if( CSR_IS_SESSION_VALID( pMac, sessionId ) )
       {
          status = csrRoamGetPMKIDCache( pMac, sessionId, pNum, pPmkidCache );
       }
       else
       {
          status = eHAL_STATUS_INVALID_PARAMETER;
       }
       sme_ReleaseGlobalLock( &pMac->sme );
   }

   return (status);
}


/*                                                                            
                          
                                                                       
                                       
                                
                                           
                      
                                                                             */
eHalStatus sme_GetConfigParam(tHalHandle hHal, tSmeConfigParams *pParam)
{
   eHalStatus status = eHAL_STATUS_FAILURE;
   tpAniSirGlobal pMac = PMAC_STRUCT( hHal );

   status = sme_AcquireGlobalLock( &pMac->sme );
   if ( HAL_STATUS_SUCCESS( status ) )
   {
      status = csrGetConfigParam(pMac, &pParam->csrConfig);
      if (status != eHAL_STATUS_SUCCESS)
      {
         smsLog( pMac, LOGE, "%s csrGetConfigParam failed\n", __FUNCTION__);
         sme_ReleaseGlobalLock( &pMac->sme );
         return status;
      }
#if defined WLAN_FEATURE_P2P_INTERNAL
      status = p2pGetConfigParam(pMac, &pParam->p2pConfig);
      if (status != eHAL_STATUS_SUCCESS)
      {
         smsLog( pMac, LOGE, "%s p2pGetConfigParam failed\n", __FUNCTION__);
         sme_ReleaseGlobalLock( &pMac->sme );
         return status;
      }
#endif
      sme_ReleaseGlobalLock( &pMac->sme );
   }

   return (status);
}

/*                                                                            
                     
                                                                      
                                
                                                             
                                                                           
                                                    
                                                                              
                                                                               
                                                                  
                                                               
                      
                                                                             */
eHalStatus sme_CfgSetInt(tHalHandle hHal, tANI_U32 cfgId, tANI_U32 ccmValue,
                         tCcmCfgSetCallback callback, eAniBoolean toBeSaved)
{
   return(ccmCfgSetInt(hHal, cfgId, ccmValue, callback, toBeSaved));
}

/*                                                                            
                     
                                                                      
                                
                                                             
                                                                               
                                    
                                                  
                                                                              
                                                                               
                                                                  
                                                               
                      
                                                                             */
eHalStatus sme_CfgSetStr(tHalHandle hHal, tANI_U32 cfgId, tANI_U8 *pStr,
                         tANI_U32 length, tCcmCfgSetCallback callback,
                         eAniBoolean toBeSaved)
{
   return(ccmCfgSetStr(hHal, cfgId, pStr, length, callback, toBeSaved));
}

/*                                                                            
                                  
                                                                            
                                                               
                                                                                
                                                                           
                                                                             
                                                                             
            
                                                                         
                                    

                      
                                                                                 */
eHalStatus sme_GetModifyProfileFields(tHalHandle hHal, tANI_U8 sessionId,
                                     tCsrRoamModifyProfileFields * pModifyProfileFields)
{
   eHalStatus status = eHAL_STATUS_FAILURE;
   tpAniSirGlobal pMac = PMAC_STRUCT( hHal );

   status = sme_AcquireGlobalLock( &pMac->sme );
   if ( HAL_STATUS_SUCCESS( status ) )
   {
       if( CSR_IS_SESSION_VALID( pMac, sessionId ) )
       {
          status = csrGetModifyProfileFields(pMac, sessionId, pModifyProfileFields);
       }
       else
       {
          status = eHAL_STATUS_INVALID_PARAMETER;
       }
       sme_ReleaseGlobalLock( &pMac->sme );
   }

   return (status);
}

/*                                                                          
                              
                                                                              
                                                                              
                                                                              
                                                                            
                                          
                                                  
                                                     
                                                                          
                                                                                
                      
                                                                            */
eHalStatus sme_SetConfigPowerSave(tHalHandle hHal, tPmcPowerSavingMode psMode,
                                  void *pConfigParams)
{
   eHalStatus status = eHAL_STATUS_FAILURE;
   tpAniSirGlobal pMac = PMAC_STRUCT( hHal );

   if (NULL == pConfigParams ) {
      smsLog( pMac, LOGE, "Empty config param structure for PMC, "
              "nothing to update");
      return eHAL_STATUS_FAILURE;
   }

   status = sme_AcquireGlobalLock( &pMac->sme );
   if ( HAL_STATUS_SUCCESS( status ) )
   {
       status = pmcSetConfigPowerSave(hHal, psMode, pConfigParams);
       sme_ReleaseGlobalLock( &pMac->sme );
   }

   return (status);
}

/*                                                                          
                              
                                                                               
                                                  
                                      
                                                                          
                                                                                
                      
                                                                            */
eHalStatus sme_GetConfigPowerSave(tHalHandle hHal, tPmcPowerSavingMode psMode,
                                  void *pConfigParams)
{
   eHalStatus status = eHAL_STATUS_FAILURE;
   tpAniSirGlobal pMac = PMAC_STRUCT( hHal );

   if (NULL == pConfigParams ) {
      smsLog( pMac, LOGE, "Empty config param structure for PMC, "
              "nothing to update");
      return eHAL_STATUS_FAILURE;
   }

   status = sme_AcquireGlobalLock( &pMac->sme );
   if ( HAL_STATUS_SUCCESS( status ) )
   {
       status = pmcGetConfigPowerSave(hHal, psMode, pConfigParams);
       sme_ReleaseGlobalLock( &pMac->sme );
   }

   return (status);
}

/*                                                                            
                            
                                                                            
                                          
                                                  
                                               
                      
                                                                             */
eHalStatus sme_SignalPowerEvent (tHalHandle hHal, tPmcPowerEvent event)
{
   eHalStatus status = eHAL_STATUS_FAILURE;
   tpAniSirGlobal pMac = PMAC_STRUCT( hHal );

   status = sme_AcquireGlobalLock( &pMac->sme );
   if ( HAL_STATUS_SUCCESS( status ) )
   {
       status = pmcSignalPowerEvent(hHal, event);
       sme_ReleaseGlobalLock( &pMac->sme );
   }

   return (status);
}

/*                                                                            
                           
                                                  
                                                  
                                                                             
                                                                              
                                                                             
                            
                      
                                                                             */
eHalStatus sme_EnablePowerSave (tHalHandle hHal, tPmcPowerSavingMode psMode)
{
   eHalStatus status = eHAL_STATUS_FAILURE;
   tpAniSirGlobal pMac = PMAC_STRUCT( hHal );

   status = sme_AcquireGlobalLock( &pMac->sme );
   if ( HAL_STATUS_SUCCESS( status ) )
   {
       status =  pmcEnablePowerSave(hHal, psMode);
       sme_ReleaseGlobalLock( &pMac->sme );
   }

   return (status);
}

/*                                                                            
                            
                                                    
                                                  
                                                                               
                                                                                 
                                                   
                      
                                                                             */
eHalStatus sme_DisablePowerSave (tHalHandle hHal, tPmcPowerSavingMode psMode)
{
   eHalStatus status = eHAL_STATUS_FAILURE;
   tpAniSirGlobal pMac = PMAC_STRUCT( hHal );

   status = sme_AcquireGlobalLock( &pMac->sme );
   if ( HAL_STATUS_SUCCESS( status ) )
   {
       status = pmcDisablePowerSave(hHal, psMode);
       sme_ReleaseGlobalLock( &pMac->sme );
   }

   return (status);
 }

/*                                                                            
                              
                                                                     
                                                                                  
                                                             
                                                  
                      
                                                                             */
eHalStatus sme_StartAutoBmpsTimer ( tHalHandle hHal)
{
   eHalStatus status = eHAL_STATUS_FAILURE;
   tpAniSirGlobal pMac = PMAC_STRUCT( hHal );

   status = sme_AcquireGlobalLock( &pMac->sme );
   if ( HAL_STATUS_SUCCESS( status ) )
   {
       status = pmcStartAutoBmpsTimer(hHal);
       sme_ReleaseGlobalLock( &pMac->sme );
   }

   return (status);
}
/*                                                                            
                             
                                                                                   
                                                                                   
                                                                                    
                                                  
                      
                                                                             */
eHalStatus sme_StopAutoBmpsTimer ( tHalHandle hHal)
{
   eHalStatus status = eHAL_STATUS_FAILURE;
   tpAniSirGlobal pMac = PMAC_STRUCT( hHal );

   status = sme_AcquireGlobalLock( &pMac->sme );
   if ( HAL_STATUS_SUCCESS( status ) )
   {
       status = pmcStopAutoBmpsTimer(hHal);
       sme_ReleaseGlobalLock( &pMac->sme );
   }

   return (status);
}
/*                                                                            
                           
                                                          
                                                  
                                                                                
                                                                              
                      
                                                                             */
eHalStatus sme_QueryPowerState (
   tHalHandle hHal,
   tPmcPowerState *pPowerState,
   tPmcSwitchState *pSwWlanSwitchState)
{
   eHalStatus status = eHAL_STATUS_FAILURE;
   tpAniSirGlobal pMac = PMAC_STRUCT( hHal );

   status = sme_AcquireGlobalLock( &pMac->sme );
   if ( HAL_STATUS_SUCCESS( status ) )
   {
       status = pmcQueryPowerState (hHal, pPowerState, NULL, pSwWlanSwitchState);
       sme_ReleaseGlobalLock( &pMac->sme );
   }

   return (status);
}

/*                                                                            
                              
                                                                              
                                                                          
                                                  
                                         
                      
                                                                             */
tANI_BOOLEAN sme_IsPowerSaveEnabled (tHalHandle hHal, tPmcPowerSavingMode psMode)
{
   eHalStatus status = eHAL_STATUS_FAILURE;
   tpAniSirGlobal pMac = PMAC_STRUCT( hHal );
   tANI_BOOLEAN result = false;

   status = sme_AcquireGlobalLock( &pMac->sme );
   if ( HAL_STATUS_SUCCESS( status ) )
   {
       result = pmcIsPowerSaveEnabled(hHal, psMode);
       sme_ReleaseGlobalLock( &pMac->sme );
       return result;
   }

   return false;
}

/*                                                                            
                            
                                                                            
                                                                                
                                                                             
                                              
                                  
                                                                                
                                                  
                                                  
                                                                                 
                               
                                                             
                                                                       
                                                                           
                                                                             */
eHalStatus sme_RequestFullPower (
   tHalHandle hHal,
   void (*callbackRoutine) (void *callbackContext, eHalStatus status),
   void *callbackContext,
   tRequestFullPowerReason fullPowerReason)
{
   eHalStatus status = eHAL_STATUS_FAILURE;
   tpAniSirGlobal pMac = PMAC_STRUCT( hHal );

   status = sme_AcquireGlobalLock( &pMac->sme );
   if ( HAL_STATUS_SUCCESS( status ) )
   {
       status = pmcRequestFullPower(hHal, callbackRoutine, callbackContext, fullPowerReason);
       sme_ReleaseGlobalLock( &pMac->sme );
   }

   return (status);
}

/*                                                                            
                       
                                                                         
                                                                              
                   
                                                  
                                                                                 
                      
                                                   
                                                                  
                                                                     
                                                                             */
eHalStatus sme_RequestBmps (
   tHalHandle hHal,
   void (*callbackRoutine) (void *callbackContext, eHalStatus status),
   void *callbackContext)
{
   eHalStatus status = eHAL_STATUS_FAILURE;
   tpAniSirGlobal pMac = PMAC_STRUCT( hHal );

   status = sme_AcquireGlobalLock( &pMac->sme );
   if ( HAL_STATUS_SUCCESS( status ) )
   {
       status = pmcRequestBmps(hHal, callbackRoutine, callbackContext);
       sme_ReleaseGlobalLock( &pMac->sme );
   }

   return (status);
}


/*                                                                            
                                       
                                                                             
                         
                                                  
                                                                             */
void  sme_SetDHCPTillPowerActiveFlag(tHalHandle hHal, tANI_U8 flag)
{
   tpAniSirGlobal pMac = PMAC_STRUCT( hHal );

   //                                                                          
   pMac->pmc.remainInPowerActiveTillDHCP = flag;
}


/*                                                                            
                      
                                                                              
                                                                            
                 
                                                  
                                                                                 
                                                    
                                                                   
                                                                      
                                                                           
                      
                                                                             */
eHalStatus sme_StartUapsd (
   tHalHandle hHal,
   void (*callbackRoutine) (void *callbackContext, eHalStatus status),
   void *callbackContext)
{
   eHalStatus status = eHAL_STATUS_FAILURE;
   tpAniSirGlobal pMac = PMAC_STRUCT( hHal );

   status = sme_AcquireGlobalLock( &pMac->sme );
   if ( HAL_STATUS_SUCCESS( status ) )
   {
       status = pmcStartUapsd(hHal, callbackRoutine, callbackContext);
       sme_ReleaseGlobalLock( &pMac->sme );
   }

   return (status);
 }

/*                                                                            
                     
                                                                             
                                                            
                                                  
                      
                                                                             
                                                                       
                                                                             */
eHalStatus sme_StopUapsd (tHalHandle hHal)
{
   eHalStatus status = eHAL_STATUS_FAILURE;
   tpAniSirGlobal pMac = PMAC_STRUCT( hHal );

   status = sme_AcquireGlobalLock( &pMac->sme );
   if ( HAL_STATUS_SUCCESS( status ) )
   {
       status = pmcStopUapsd(hHal);
       sme_ReleaseGlobalLock( &pMac->sme );
   }

   return (status);
}

/*                                                                            
                          
                                                                                 
                                                                            
                     
                                                  
                                                                                 
                      
                                                     
                                                                
                                                                   
                                                                             */
eHalStatus sme_RequestStandby (
   tHalHandle hHal,
   void (*callbackRoutine) (void *callbackContext, eHalStatus status),
   void *callbackContext)
{
   eHalStatus status = eHAL_STATUS_FAILURE;
   tpAniSirGlobal pMac = PMAC_STRUCT( hHal );

   smsLog( pMac, LOG1, FL("") );
   status = sme_AcquireGlobalLock( &pMac->sme );
   if ( HAL_STATUS_SUCCESS( status ) )
   {
       status = pmcRequestStandby(hHal, callbackRoutine, callbackContext);
       sme_ReleaseGlobalLock( &pMac->sme );
   }

   return (status);
}

/*                                                                            
                                  
                                                                       
                                                                     
                                                  
                                                                     
                      
                                                         
                                                             
                                                                             */
eHalStatus sme_RegisterPowerSaveCheck (
   tHalHandle hHal,
   tANI_BOOLEAN (*checkRoutine) (void *checkContext), void *checkContext)
{
   eHalStatus status = eHAL_STATUS_FAILURE;
   tpAniSirGlobal pMac = PMAC_STRUCT( hHal );

   status = sme_AcquireGlobalLock( &pMac->sme );
   if ( HAL_STATUS_SUCCESS( status ) )
   {
       status = pmcRegisterPowerSaveCheck (hHal, checkRoutine, checkContext);
       sme_ReleaseGlobalLock( &pMac->sme );
   }

   return (status);
}

/*                                                                            
                                    
                                                 
                                                  
                                                                       
                      
                                                           
                                                               
                                                                             */
eHalStatus sme_DeregisterPowerSaveCheck (
   tHalHandle hHal,
   tANI_BOOLEAN (*checkRoutine) (void *checkContext))
{
   eHalStatus status = eHAL_STATUS_FAILURE;
   tpAniSirGlobal pMac = PMAC_STRUCT( hHal );

   status = sme_AcquireGlobalLock( &pMac->sme );
   if ( HAL_STATUS_SUCCESS( status ) )
   {
       status = pmcDeregisterPowerSaveCheck (hHal, checkRoutine);
       sme_ReleaseGlobalLock( &pMac->sme );
   }

   return (status);
}

/*                                                                            
                                        
                                                               
                                                                          
                                                  
                                                                
                                                                       
                      
                                                         
                                                             
                                                                             */
eHalStatus sme_RegisterDeviceStateUpdateInd (
   tHalHandle hHal,
   void (*callbackRoutine) (void *callbackContext, tPmcState pmcState),
   void *callbackContext)
{
   eHalStatus status = eHAL_STATUS_FAILURE;
   tpAniSirGlobal pMac = PMAC_STRUCT( hHal );

   status = sme_AcquireGlobalLock( &pMac->sme );
   if ( HAL_STATUS_SUCCESS( status ) )
   {
       status = pmcRegisterDeviceStateUpdateInd (hHal, callbackRoutine, callbackContext);
       sme_ReleaseGlobalLock( &pMac->sme );
   }

   return (status);
}

/*                                                                            
                                          
                                                                             
                                                  
                                                                  
                      
                                                           
                                                               
                                                                             */
eHalStatus sme_DeregisterDeviceStateUpdateInd (
   tHalHandle hHal,
   void (*callbackRoutine) (void *callbackContext, tPmcState pmcState))
{
   eHalStatus status = eHAL_STATUS_FAILURE;
   tpAniSirGlobal pMac = PMAC_STRUCT( hHal );

   status = sme_AcquireGlobalLock( &pMac->sme );
   if ( HAL_STATUS_SUCCESS( status ) )
   {
       status = pmcDeregisterDeviceStateUpdateInd (hHal, callbackRoutine);
       sme_ReleaseGlobalLock( &pMac->sme );
   }

   return (status);
}

/*                                                                            
                               
                                                                               
                                                                                 
                                                                          
                                                             
                                                  
                                          
                      
                                                   
                                                  
                                                                             */
eHalStatus sme_WowlAddBcastPattern (
   tHalHandle hHal,
   tpSirWowlAddBcastPtrn pattern)
{
    eHalStatus status = eHAL_STATUS_FAILURE;
    tpAniSirGlobal pMac = PMAC_STRUCT( hHal );
    status = sme_AcquireGlobalLock( &pMac->sme );
    if ( HAL_STATUS_SUCCESS( status ) )
    {
       status = pmcWowlAddBcastPattern (hHal, pattern);
       sme_ReleaseGlobalLock( &pMac->sme );
    }

    return (status);
}

/*                                                                            
                               
                                                                      
                                                  
                                            
                      
                                                      
                                                  
                                                                             */
eHalStatus sme_WowlDelBcastPattern (
   tHalHandle hHal,
   tpSirWowlDelBcastPtrn pattern)
{
    eHalStatus status = eHAL_STATUS_FAILURE;
    tpAniSirGlobal pMac = PMAC_STRUCT( hHal );
    status = sme_AcquireGlobalLock( &pMac->sme );
    if ( HAL_STATUS_SUCCESS( status ) )
    {
       status = pmcWowlDelBcastPattern (hHal, pattern);
       sme_ReleaseGlobalLock( &pMac->sme );
    }

    return (status);
}

/*                                                                            
                     
                                                                                
                                                                                
                                                                              
                                                                           
                                 
                                                                          
                                                                                
                                                                               
                                                                                    
                                                                                    
                                                                                  
                                            

                                                  
                                                                         
                                                                           
                                                                                         
                                                      
                                                                
                                                                     
                                                                                       
                                                      
                      
                                                                 
                                                                 
                                                                                 
                                                           
                                                                             */
eHalStatus sme_EnterWowl (
    tHalHandle hHal,
    void (*enterWowlCallbackRoutine) (void *callbackContext, eHalStatus status),
    void *enterWowlCallbackContext,
#ifdef WLAN_WAKEUP_EVENTS
    void (*wakeIndicationCB) (void *callbackContext, tpSirWakeReasonInd pWakeReasonInd),
    void *wakeIndicationCBContext,
#endif //                   
    tpSirSmeWowlEnterParams wowlEnterParams)
{
    eHalStatus status = eHAL_STATUS_FAILURE;
    tpAniSirGlobal pMac = PMAC_STRUCT( hHal );
    status = sme_AcquireGlobalLock( &pMac->sme );
    if ( HAL_STATUS_SUCCESS( status ) )
    {
       status = pmcEnterWowl (hHal, enterWowlCallbackRoutine, enterWowlCallbackContext, 
#ifdef WLAN_WAKEUP_EVENTS
                              wakeIndicationCB, wakeIndicationCBContext, 
#endif //                   
                              wowlEnterParams);
       sme_ReleaseGlobalLock( &pMac->sme );
    }

    return (status);
}
/*                                                                            
                    
                                                                                
                                                                                  
                 
                                                  
                      
                                                                
                                                                      
                                                                             */
eHalStatus sme_ExitWowl (tHalHandle hHal)
{
    eHalStatus status = eHAL_STATUS_FAILURE;
    tpAniSirGlobal pMac = PMAC_STRUCT( hHal );
    status = sme_AcquireGlobalLock( &pMac->sme );
    if ( HAL_STATUS_SUCCESS( status ) )
    {
       status = pmcExitWowl (hHal);
       sme_ReleaseGlobalLock( &pMac->sme );
    }

    return (status);
}

/*                                                                            

                      

                                                                                    
                                

                                                                                   

                                                                                                              

                                                                                       

                                                                           

                                                                                 */
eHalStatus sme_RoamSetKey(tHalHandle hHal, tANI_U8 sessionId, tCsrRoamSetKey *pSetKey, tANI_U32 *pRoamId)
{
   eHalStatus status = eHAL_STATUS_FAILURE;
   tpAniSirGlobal pMac = PMAC_STRUCT( hHal );
   tANI_U32 roamId;
#ifdef WLAN_SOFTAP_FEATURE
   tANI_U32 i;
   tCsrRoamSession *pSession = NULL;
#endif

   status = sme_AcquireGlobalLock( &pMac->sme );
   if ( HAL_STATUS_SUCCESS( status ) )
   {
      roamId = GET_NEXT_ROAM_ID(&pMac->roam);
      if(pRoamId)
      {
         *pRoamId = roamId;
      }

#ifdef WLAN_SOFTAP_FEATURE
      smsLog(pMac, LOG2, FL("keyLength\n"), pSetKey->keyLength);

      for(i=0; i<pSetKey->keyLength; i++)
          smsLog(pMac, LOG2, FL("%02x"), pSetKey->Key[i]); 

      smsLog(pMac, LOG2, "\n sessionId=%d roamId=%d\n", sessionId, roamId);       

      pSession = CSR_GET_SESSION(pMac, sessionId);

      if(!pSession)
      {
         smsLog(pMac, LOGE, FL("  session %d not found "), sessionId);
         return eHAL_STATUS_FAILURE;
      }

      if(CSR_IS_INFRA_AP(&pSession->connectedProfile))
      {
         if(pSetKey->keyDirection == eSIR_TX_DEFAULT)
         {
            if ( ( eCSR_ENCRYPT_TYPE_WEP40 == pSetKey->encType ) ||
                 ( eCSR_ENCRYPT_TYPE_WEP40_STATICKEY == pSetKey->encType ))
            {
               pSession->pCurRoamProfile->negotiatedUCEncryptionType = eCSR_ENCRYPT_TYPE_WEP40_STATICKEY;
            }
            if ( ( eCSR_ENCRYPT_TYPE_WEP104 == pSetKey->encType ) ||
                 ( eCSR_ENCRYPT_TYPE_WEP104_STATICKEY == pSetKey->encType ))
            {
               pSession->pCurRoamProfile->negotiatedUCEncryptionType = eCSR_ENCRYPT_TYPE_WEP104_STATICKEY;
            }
         }
      }
#endif

      status = csrRoamSetKey ( pMac, sessionId, pSetKey, roamId );
      sme_ReleaseGlobalLock( &pMac->sme );
   }

   return (status);
}


/*                                                                            

                         

                                                              

                                                                                 

                                                                                                              

                                                                                       

                                                                           

                                                                                 */
eHalStatus sme_RoamRemoveKey(tHalHandle hHal, tANI_U8 sessionId,
                             tCsrRoamRemoveKey *pRemoveKey, tANI_U32 *pRoamId)
{
   eHalStatus status = eHAL_STATUS_FAILURE;
   tpAniSirGlobal pMac = PMAC_STRUCT( hHal );
   tANI_U32 roamId;

   status = sme_AcquireGlobalLock( &pMac->sme );
   if ( HAL_STATUS_SUCCESS( status ) )
   {
      roamId = GET_NEXT_ROAM_ID(&pMac->roam);
      if(pRoamId)
      {
         *pRoamId = roamId;
      }
      status = csrRoamIssueRemoveKeyCommand( pMac, sessionId, pRemoveKey, roamId );
      sme_ReleaseGlobalLock( &pMac->sme );
   }

   return (status);
}

/*                                                                            
                   
                                                                                  

                                                                           
                                                                      
                                                                            
                                    
                      
                                                                             */
eHalStatus sme_GetRssi(tHalHandle hHal,
                             tCsrRssiCallback callback,
                             tANI_U8 staId, tCsrBssid bssId, 
                             void *pContext, void* pVosContext)
{
   eHalStatus status = eHAL_STATUS_FAILURE;
   tpAniSirGlobal pMac = PMAC_STRUCT( hHal );

   status = sme_AcquireGlobalLock( &pMac->sme );
   if ( HAL_STATUS_SUCCESS( status ) )
   {
      status = csrGetRssi( pMac, callback,
                                 staId, bssId, pContext, pVosContext);
      sme_ReleaseGlobalLock( &pMac->sme );
   }
   return (status);
}

/*                                                                            
                         
                                                                             
                                            

                                                                                     
                                                                                          
                                                                           
                                                                                
                                                 
                                                          
                                                                      
                                                                            
                      
                                                                             */
eHalStatus sme_GetStatistics(tHalHandle hHal, eCsrStatsRequesterType requesterId,
                             tANI_U32 statsMask,
                             tCsrStatsCallback callback,
                             tANI_U32 periodicity, tANI_BOOLEAN cache,
                             tANI_U8 staId, void *pContext)
{
   eHalStatus status = eHAL_STATUS_FAILURE;
   tpAniSirGlobal pMac = PMAC_STRUCT( hHal );

   status = sme_AcquireGlobalLock( &pMac->sme );
   if ( HAL_STATUS_SUCCESS( status ) )
   {
      status = csrGetStatistics( pMac, requesterId , statsMask, callback,
                                 periodicity, cache, staId, pContext);
      sme_ReleaseGlobalLock( &pMac->sme );
   }

   return (status);

}

/*                                                                            

                          

                                                                                                     
                            
                                                                                                         
                              

                                                                                 

                                                                            
                                                                                           
                                                                   
                                                                                     

                                

                                                                           

                                                                                 */
eHalStatus sme_GetCountryCode(tHalHandle hHal, tANI_U8 *pBuf, tANI_U8 *pbLen)
{
    tpAniSirGlobal pMac = PMAC_STRUCT( hHal );

    return ( csrGetCountryCode( pMac, pBuf, pbLen ) );
}


/*                                                                            

                          

                                                      
                                                       
                              

                                                                                

                                                                                                      
                                

                                

                                                                           

                                                                                 */
eHalStatus sme_SetCountryCode(tHalHandle hHal, tANI_U8 *pCountry, tANI_BOOLEAN *pfRestartNeeded)
{
    tpAniSirGlobal pMac = PMAC_STRUCT( hHal );

    return ( csrSetCountryCode( pMac, pCountry, pfRestartNeeded ) );
}


/*                                                                            
                                       
                                                                                               
                                                                            
                                                                                                              
                                           
                      
                                                                                 */
eHalStatus sme_ResetCountryCodeInformation(tHalHandle hHal, tANI_BOOLEAN *pfRestartNeeded)
{
    tpAniSirGlobal pMac = PMAC_STRUCT( hHal );

    return ( csrResetCountryCodeInformation( pMac, pfRestartNeeded ) );
}


/*                                                                            
                                   
                                                                                     
                                                                                     
                                                                                      
                                               
                                                                                                    
                                                                                                
                      
                                                                                 */
eHalStatus sme_GetSupportedCountryCode(tHalHandle hHal, tANI_U8 *pBuf, tANI_U32 *pbLen)
{
    tpAniSirGlobal pMac = PMAC_STRUCT( hHal );

    return ( csrGetSupportedCountryCode( pMac, pBuf, pbLen ) );
}


/*                                                                            
                                      
                                                                                            
                                                                                                          
                                                         
                                                                          
                                

                                                                           
                                                                                 */
eHalStatus sme_GetCurrentRegulatoryDomain(tHalHandle hHal, v_REGDOMAIN_t *pDomain)
{
    tpAniSirGlobal pMac = PMAC_STRUCT( hHal );
    eHalStatus status = eHAL_STATUS_INVALID_PARAMETER;

    if( pDomain )
    {
        if( csrIs11dSupported( pMac ) )
        {
            *pDomain = csrGetCurrentRegulatoryDomain( pMac );
            status = eHAL_STATUS_SUCCESS;
        }
        else
        {
            status = eHAL_STATUS_FAILURE;
        }
    }

    return ( status );
}


/*                                                                            
                               
                                                                 
                                                                                                          
                                   
                                                                                  
                                    
                                                                                                              
                                           
                      
                                                                                 */
eHalStatus sme_SetRegulatoryDomain(tHalHandle hHal, v_REGDOMAIN_t domainId, tANI_BOOLEAN *pfRestartNeeded)
{
    tpAniSirGlobal pMac = PMAC_STRUCT( hHal );

    return ( csrSetRegulatoryDomain( pMac, domainId, pfRestartNeeded ) );
}


/*                                                                            

                                         

                                                                                           

                                                                                  

                                                                                       
                                                                          

                                

                                                                           

                                                                                 */
eHalStatus sme_GetRegulatoryDomainForCountry(tHalHandle hHal, tANI_U8 *pCountry, v_REGDOMAIN_t *pDomainId)
{
    tpAniSirGlobal pMac = PMAC_STRUCT( hHal );

    return ( csrGetRegulatoryDomainForCountry( pMac, pCountry, pDomainId ) );
}




/*                                                                            

                                         

                                                                                       

                                                                                           

                                                                                                
                                                                                                      
                                                                                                       
                                                                                           

                                

                                                                           

                                                                                 */
eHalStatus sme_GetSupportedRegulatoryDomains(tHalHandle hHal, v_REGDOMAIN_t *pDomains, tANI_U32 *pNumDomains)
{
    eHalStatus status = eHAL_STATUS_INVALID_PARAMETER;

    //                              
    if( pNumDomains )
    {
        if( NUM_REG_DOMAINS <= *pNumDomains )
        {
            status = eHAL_STATUS_SUCCESS;
        }
        *pNumDomains = NUM_REG_DOMAINS;
    }
    if( HAL_STATUS_SUCCESS( status ) )
    {
        if( pDomains )
        {
            pDomains[0] = REGDOMAIN_FCC;
            pDomains[1] = REGDOMAIN_ETSI;
            pDomains[2] = REGDOMAIN_JAPAN;
            pDomains[3] = REGDOMAIN_WORLD;
            pDomains[4] = REGDOMAIN_N_AMER_EXC_FCC;
            pDomains[5] = REGDOMAIN_APAC;
            pDomains[6] = REGDOMAIN_KOREA;
            pDomains[7] = REGDOMAIN_HI_5GHZ;
            pDomains[8] = REGDOMAIN_NO_5GHZ;
        }
        else
        {
            status = eHAL_STATUS_INVALID_PARAMETER;
        }
    }

    return ( status );
}


//                      
tANI_BOOLEAN sme_Is11dSupported(tHalHandle hHal)
{
    tpAniSirGlobal pMac = PMAC_STRUCT( hHal );

    return ( csrIs11dSupported( pMac ) );
}


tANI_BOOLEAN sme_Is11hSupported(tHalHandle hHal)
{
    tpAniSirGlobal pMac = PMAC_STRUCT( hHal );

    return ( csrIs11hSupported( pMac ) );
}


tANI_BOOLEAN sme_IsWmmSupported(tHalHandle hHal)
{
    tpAniSirGlobal pMac = PMAC_STRUCT( hHal );

    return ( csrIsWmmSupported( pMac ) );
}

//                                                                                        
eHalStatus sme_ScanGetBaseChannels( tHalHandle hHal, tCsrChannelInfo * pChannelInfo )
{
   tpAniSirGlobal pMac = PMAC_STRUCT( hHal );

   return(csrScanGetBaseChannels(pMac,pChannelInfo) );
}

/*                                                                            

                             

                                                                            
                                     

                                                 

                                           

                                

                                                                           

                                                                                 */
eHalStatus sme_ChangeCountryCode( tHalHandle hHal,
                                          tSmeChangeCountryCallback callback,
                                          tANI_U8 *pCountry,
                                          void *pContext,
                                          void* pVosContext )
{
   eHalStatus                status = eHAL_STATUS_FAILURE;
   tpAniSirGlobal            pMac = PMAC_STRUCT( hHal );
   vos_msg_t                 msg;
   tAniChangeCountryCodeReq *pMsg;

   status = sme_AcquireGlobalLock( &pMac->sme );
   if ( HAL_STATUS_SUCCESS( status ) )
   {
      smsLog(pMac, LOG1, FL(" called\n"));      
      status = palAllocateMemory(pMac->hHdd, (void **)&pMsg, sizeof(tAniChangeCountryCodeReq));
      if ( !HAL_STATUS_SUCCESS(status) ) 
      {
         smsLog(pMac, LOGE, " csrChangeCountryCode: failed to allocate mem for req \n");
         return status;
      }

      pMsg->msgType = pal_cpu_to_be16((tANI_U16)eWNI_SME_CHANGE_COUNTRY_CODE);
      pMsg->msgLen = (tANI_U16)sizeof(tAniChangeCountryCodeReq);
      palCopyMemory(pMac->hHdd, pMsg->countryCode, pCountry, 3);
      pMsg->changeCCCallback = callback;
      pMsg->pDevContext = pContext;
      pMsg->pVosContext = pVosContext;

      msg.type = eWNI_SME_CHANGE_COUNTRY_CODE;
      msg.bodyptr = pMsg;
      msg.reserved = 0;

      if(VOS_STATUS_SUCCESS != vos_mq_post_message(VOS_MQ_ID_SME, &msg))
      {
          smsLog(pMac, LOGE, " sme_ChangeCountryCode failed to post msg to self \n");   
          palFreeMemory(pMac->hHdd, (void *)pMsg);
          status = eHAL_STATUS_FAILURE;
      }
      smsLog(pMac, LOG1, FL(" returned\n"));      
      sme_ReleaseGlobalLock( &pMac->sme );
   }

   return (status);
}

/*                                                                            
                            
                                                                               
                                                                              
                                                                              
                              
                                                  
                                                                                
                                                                                 
                      
                                                                             
                                                                                
                                                   
                                                        
                                                                             */
VOS_STATUS sme_BtcSignalBtEvent (tHalHandle hHal, tpSmeBtEvent pBtEvent)
{
    VOS_STATUS status = VOS_STATUS_E_FAILURE;
#ifndef WLAN_MDM_CODE_REDUCTION_OPT
    tpAniSirGlobal pMac = PMAC_STRUCT( hHal );

    if ( eHAL_STATUS_SUCCESS == sme_AcquireGlobalLock( &pMac->sme ) )
    {
        status = btcSignalBTEvent (hHal, pBtEvent);
        sme_ReleaseGlobalLock( &pMac->sme );
    }
#endif
    return (status);
}

/*                                                                            
                        
                                                                               
                                                                                  
                                                            
                                                  
                                                                                       
                                                                                     
                      
                                                           
                                                    
                                                                             */
VOS_STATUS sme_BtcSetConfig (tHalHandle hHal, tpSmeBtcConfig pSmeBtcConfig)
{
    VOS_STATUS status = VOS_STATUS_E_FAILURE;
#ifndef WLAN_MDM_CODE_REDUCTION_OPT
    tpAniSirGlobal pMac = PMAC_STRUCT( hHal );
    if ( eHAL_STATUS_SUCCESS == sme_AcquireGlobalLock( &pMac->sme ) )
    {
        status = btcSetConfig (hHal, pSmeBtcConfig);
        sme_ReleaseGlobalLock( &pMac->sme );
    }
#endif
    return (status);
}

/*                                                                            
                        
                                                                                 
                                                  
                                                                        
                                                                                    
                                           
                      
                                          
                                       
                                                                             */
VOS_STATUS sme_BtcGetConfig (tHalHandle hHal, tpSmeBtcConfig pSmeBtcConfig)
{
    VOS_STATUS status = VOS_STATUS_E_FAILURE;
#ifndef WLAN_MDM_CODE_REDUCTION_OPT
    tpAniSirGlobal pMac = PMAC_STRUCT( hHal );

    if ( eHAL_STATUS_SUCCESS == sme_AcquireGlobalLock( &pMac->sme ) )
    {
        status = btcGetConfig (hHal, pSmeBtcConfig);
        sme_ReleaseGlobalLock( &pMac->sme );
    }
#endif
    return (status);
}
/*                                                                            
                         
                                                   
                                                  
                                                
                                                                  

                
                                                                             */
void sme_SetCfgPrivacy( tHalHandle hHal,
                        tCsrRoamProfile *pProfile,
                        tANI_BOOLEAN fPrivacy
                        )
{
    tpAniSirGlobal pMac = PMAC_STRUCT( hHal );
    if ( eHAL_STATUS_SUCCESS == sme_AcquireGlobalLock( &pMac->sme ) )
    {
        csrSetCfgPrivacy(pMac, pProfile, fPrivacy);
        sme_ReleaseGlobalLock( &pMac->sme );
    }
}

#if defined WLAN_FEATURE_VOWIFI
/*                                                                            
                                 
                                           
                                                  
                                                                          
                                                                                      
                                           
                      
                                          
                                       
                                                                             */
VOS_STATUS sme_NeighborReportRequest (tHalHandle hHal, tANI_U8 sessionId,
                                    tpRrmNeighborReq pRrmNeighborReq, tpRrmNeighborRspCallbackInfo callbackInfo)
{
    VOS_STATUS status = VOS_STATUS_E_FAILURE;
    tpAniSirGlobal pMac = PMAC_STRUCT( hHal );

    if ( eHAL_STATUS_SUCCESS == sme_AcquireGlobalLock( &pMac->sme ) )
    {
        status = sme_RrmNeighborReportRequest (hHal, sessionId, pRrmNeighborReq, callbackInfo);
        sme_ReleaseGlobalLock( &pMac->sme );
    }

    return (status);
}
#endif

//                                                                         
//                                                                          
//                                                                   

//                   
//                                         
VOS_STATUS sme_DbgReadRegister(tHalHandle hHal, v_U32_t regAddr, v_U32_t *pRegValue)
{
   VOS_STATUS   status = VOS_STATUS_E_FAILURE;
   tpAniSirGlobal pMac = PMAC_STRUCT(hHal);
#if defined(FEATURE_WLAN_NON_INTEGRATED_SOC)
   tHddHandle     hHdd = pMac->hHdd;
#endif
   tPmcPowerState PowerState;
   tANI_U32  sessionId = 0;

   /*                                                                         */

   if(eDRIVER_TYPE_MFG == pMac->gDriverType)
   {
#if defined(FEATURE_WLAN_NON_INTEGRATED_SOC)
      if (HAL_STATUS_SUCCESS(palReadRegister(hHdd, regAddr, pRegValue)))
#elif defined(FEATURE_WLAN_INTEGRATED_SOC)
      if (eWLAN_PAL_STATUS_SUCCESS == wpalDbgReadRegister(regAddr, pRegValue))
#endif
      {
         return VOS_STATUS_SUCCESS;
      }
      return VOS_STATUS_E_FAILURE;
   }

   /*                                                                        */

   /*                         */
   if (eHAL_STATUS_SUCCESS != sme_AcquireGlobalLock(&pMac->sme))
   {
      return VOS_STATUS_E_FAILURE;
   }

   if(HAL_STATUS_SUCCESS(pmcQueryPowerState(pMac, &PowerState, NULL, NULL)))
   {
      /*                                                                 */
      if(!csrIsConnStateDisconnected(pMac, sessionId) || (ePMC_LOW_POWER != PowerState))
      {
#if defined(FEATURE_WLAN_NON_INTEGRATED_SOC)
         if (HAL_STATUS_SUCCESS(palReadRegister(hHdd, regAddr, pRegValue )))
#elif defined(FEATURE_WLAN_INTEGRATED_SOC)
         if (eWLAN_PAL_STATUS_SUCCESS == wpalDbgReadRegister(regAddr, pRegValue))
#endif
         {
            status = VOS_STATUS_SUCCESS;
         }
         else
         {  
            status = VOS_STATUS_E_FAILURE;
         }
      }
      else
      {
         status = VOS_STATUS_E_FAILURE;
      }
   }

   /*                                       
                                                                                  */
   if ( VOS_STATUS_SUCCESS != status )
   {
      *pRegValue = 0xDEADBEEF;
       status = VOS_STATUS_SUCCESS;
   }
        
   /*                         */
   sme_ReleaseGlobalLock(&pMac->sme);
    
   return (status);
}


//                    
//                                         
VOS_STATUS sme_DbgWriteRegister(tHalHandle hHal, v_U32_t regAddr, v_U32_t regValue)
{
   VOS_STATUS    status = VOS_STATUS_E_FAILURE;
   tpAniSirGlobal  pMac = PMAC_STRUCT(hHal);
#if defined(FEATURE_WLAN_NON_INTEGRATED_SOC)
   tHddHandle      hHdd = pMac->hHdd;
#endif
   tPmcPowerState PowerState;
   tANI_U32   sessionId = 0;

   /*                                                                         */

   if(eDRIVER_TYPE_MFG == pMac->gDriverType)
   {
#if defined(FEATURE_WLAN_NON_INTEGRATED_SOC)
      if (HAL_STATUS_SUCCESS(palWriteRegister(hHdd, regAddr, regValue)))
#elif defined(FEATURE_WLAN_INTEGRATED_SOC)
      if (eWLAN_PAL_STATUS_SUCCESS == wpalDbgWriteRegister(regAddr, regValue))
#endif
      {
         return VOS_STATUS_SUCCESS;
      }
      return VOS_STATUS_E_FAILURE;
   }

   /*                                                                        */

   /*                         */
   if (eHAL_STATUS_SUCCESS != sme_AcquireGlobalLock(&pMac->sme))
   {
      return VOS_STATUS_E_FAILURE;
   }

   if(HAL_STATUS_SUCCESS(pmcQueryPowerState(pMac, &PowerState, NULL, NULL)))
   {
      /*                                                                 */
      if(!csrIsConnStateDisconnected(pMac, sessionId) || (ePMC_LOW_POWER != PowerState))
      {
#if defined(FEATURE_WLAN_NON_INTEGRATED_SOC)
         if (HAL_STATUS_SUCCESS(palWriteRegister(hHdd, regAddr, regValue)))
#elif defined(FEATURE_WLAN_INTEGRATED_SOC)
         if (eWLAN_PAL_STATUS_SUCCESS == wpalDbgWriteRegister(regAddr, regValue))
#endif
         {
            status = VOS_STATUS_SUCCESS;
         }
         else
         {
            status = VOS_STATUS_E_FAILURE;
         }
      }
      else
      {
         status = VOS_STATUS_E_FAILURE;
      }
   }
        
   /*                         */
   sme_ReleaseGlobalLock(&pMac->sme);
    
   return (status);
}



//                 
//                                         
//                                                   
VOS_STATUS sme_DbgReadMemory(tHalHandle hHal, v_U32_t memAddr, v_U8_t *pBuf, v_U32_t nLen)
{
   VOS_STATUS  status  = VOS_STATUS_E_FAILURE;
   tpAniSirGlobal pMac = PMAC_STRUCT( hHal );
#if defined(FEATURE_WLAN_NON_INTEGRATED_SOC)
   v_PVOID_t  pvosGCTx = vos_get_global_context(VOS_MODULE_ID_PE, (v_VOID_t *)hHal);
   tHddHandle     hHdd = pMac->hHdd;
#endif
   tPmcPowerState PowerState;
   tANI_U32 sessionId  = 0;

   /*                                                                         */

   if(eDRIVER_TYPE_MFG == pMac->gDriverType)
   {
#if defined(FEATURE_WLAN_NON_INTEGRATED_SOC)
      if (HAL_STATUS_SUCCESS(palReadDeviceMemory(hHdd, memAddr, (void *)pBuf, nLen)))
#elif defined(FEATURE_WLAN_INTEGRATED_SOC)
      if (eWLAN_PAL_STATUS_SUCCESS == wpalDbgReadMemory(memAddr, (void *)pBuf, nLen))
#endif
      {
         return VOS_STATUS_SUCCESS;
      }
      return VOS_STATUS_E_FAILURE;
   }

   /*                                                                        */

   /*                         */
   if (eHAL_STATUS_SUCCESS != sme_AcquireGlobalLock(&pMac->sme))
   {
      return VOS_STATUS_E_FAILURE;
   }

   if(HAL_STATUS_SUCCESS(pmcQueryPowerState(pMac, &PowerState, NULL, NULL)))
   {
      /*                                                                 */
      if(!csrIsConnStateDisconnected(pMac, sessionId) || (ePMC_LOW_POWER != PowerState))
      {
#if defined(FEATURE_WLAN_NON_INTEGRATED_SOC)
         if (HAL_STATUS_SUCCESS(palReadDeviceMemory(pvosGCTx, memAddr, (void *)pBuf, nLen)))
#elif defined(FEATURE_WLAN_INTEGRATED_SOC)
         if (eWLAN_PAL_STATUS_SUCCESS == wpalDbgReadMemory(memAddr, (void *)pBuf, nLen)) 
#endif
         {
            status = VOS_STATUS_SUCCESS;
         }
         else
         {
            status = VOS_STATUS_E_FAILURE;
         }
      }
      else
      {
         status = VOS_STATUS_E_FAILURE;
      }
   }

   /*                                       
                                                                                  */
   if (VOS_STATUS_SUCCESS != status)
   {
      vos_mem_set(pBuf, nLen, 0xCD);
      status = VOS_STATUS_SUCCESS;
      smsLog(pMac, LOGE, FL(" filled with 0xCD because it cannot access the hardware\n"));
   }

   /*                  */
   sme_ReleaseGlobalLock(&pMac->sme);
    
   return (status);
}


//                  
//                                         
VOS_STATUS sme_DbgWriteMemory(tHalHandle hHal, v_U32_t memAddr, v_U8_t *pBuf, v_U32_t nLen)
{
   VOS_STATUS    status = VOS_STATUS_E_FAILURE;
   tpAniSirGlobal  pMac = PMAC_STRUCT(hHal);
#if defined(FEATURE_WLAN_NON_INTEGRATED_SOC)
   tHddHandle      hHdd = pMac->hHdd;
#endif
   tPmcPowerState PowerState;
   tANI_U32   sessionId = 0;

   /*                                                                         */

   if(eDRIVER_TYPE_MFG == pMac->gDriverType)
   {
#if defined(FEATURE_WLAN_NON_INTEGRATED_SOC)
      if (HAL_STATUS_SUCCESS(palWriteDeviceMemory(hHdd, memAddr, (void *)pBuf, nLen)))
#elif defined(FEATURE_WLAN_NON_INTEGRATED_SOC)
      if (eWLAN_PAL_STATUS_SUCCESS == wpalDbgWriteMemory( memAddr, (void *)pBuf, nLen))
#endif
      {
         return VOS_STATUS_SUCCESS;
      }
      return VOS_STATUS_E_FAILURE;
   }

   /*                                                                        */

   /*                         */
   if (eHAL_STATUS_SUCCESS != sme_AcquireGlobalLock(&pMac->sme))
   {
      return VOS_STATUS_E_FAILURE;
   }

   if(HAL_STATUS_SUCCESS(pmcQueryPowerState(pMac, &PowerState, NULL, NULL)))
   {
      /*                                                                 */
      if(!csrIsConnStateDisconnected(pMac, sessionId) || (ePMC_LOW_POWER != PowerState))
      {
#if defined(FEATURE_WLAN_NON_INTEGRATED_SOC)
         if (HAL_STATUS_SUCCESS(palWriteDeviceMemory(hHdd, memAddr, (void *)pBuf, nLen)))
#elif defined(FEATURE_WLAN_INTEGRATED_SOC)
         if (eWLAN_PAL_STATUS_SUCCESS == wpalDbgWriteMemory(memAddr, (void *)pBuf, nLen))
#endif
         {
            status = VOS_STATUS_SUCCESS;
         }
         else
         {
            status = VOS_STATUS_E_FAILURE;
         }
      }
      else
      {
         status = VOS_STATUS_E_FAILURE;
      }
   }

   /*                     */
   sme_ReleaseGlobalLock(&pMac->sme);
    
   return (status);
}


void smsLog(tpAniSirGlobal pMac, tANI_U32 loglevel, const char *pString,...) 
{
#ifdef WLAN_DEBUG
    //                                 
    if ( loglevel > pMac->utils.gLogDbgLevel[LOG_INDEX_FOR_MODULE( SIR_SMS_MODULE_ID )] )
        return;
    else
    {
        va_list marker;

        va_start( marker, pString );     /*                                */

        logDebug(pMac, SIR_SMS_MODULE_ID, loglevel, pString, marker);

        va_end( marker );              /*                                */
    }
#endif
}
#ifdef FEATURE_WLAN_NON_INTEGRATED_SOC  
/*                                                                            
                        
                                                  
                                                  
                                                            
                      
                                        
                                       
                                                                             */
VOS_STATUS sme_GetFwVersion (tHalHandle hHal,FwVersionInfo *pVersion)
{
    VOS_STATUS status = VOS_STATUS_SUCCESS;
    tpAniSirGlobal pMac = PMAC_STRUCT( hHal );

    if ( eHAL_STATUS_SUCCESS == sme_AcquireGlobalLock( &pMac->sme ) )
    {
        if( pVersion != NULL ) {
            vos_mem_copy((v_VOID_t*)pVersion,(v_VOID_t*)&pMac->hal.FwParam.fwVersion, sizeof(FwVersionInfo));
        }
        else {
            status = VOS_STATUS_E_INVAL;
        }
        sme_ReleaseGlobalLock( &pMac->sme );
    }

    return (status);
}
#endif

#ifdef FEATURE_WLAN_INTEGRATED_SOC
/*                                                                            
                                       
                                                                   
                                           
                                                  
                                                                   
                      
                                        
                                       
                                                                             */
VOS_STATUS sme_GetWcnssWlanCompiledVersion(tHalHandle hHal,
                                           tSirVersionType *pVersion)
{
    VOS_STATUS status = VOS_STATUS_SUCCESS;
    tpAniSirGlobal pMac = PMAC_STRUCT( hHal );
    v_CONTEXT_t vosContext = vos_get_global_context(VOS_MODULE_ID_SME, NULL);

    if ( eHAL_STATUS_SUCCESS == sme_AcquireGlobalLock( &pMac->sme ) )
    {
        if( pVersion != NULL )
        {
            status = WDA_GetWcnssWlanCompiledVersion(vosContext, pVersion);
        }
        else
        {
            status = VOS_STATUS_E_INVAL;
        }
        sme_ReleaseGlobalLock( &pMac->sme );
    }

    return (status);
}


/*                                                                            
                                       
                                                                   
                                                       
                                                  
                                                                   
                      
                                        
                                       
                                                                             */
VOS_STATUS sme_GetWcnssWlanReportedVersion(tHalHandle hHal,
                                           tSirVersionType *pVersion)
{
    VOS_STATUS status = VOS_STATUS_SUCCESS;
    tpAniSirGlobal pMac = PMAC_STRUCT( hHal );
    v_CONTEXT_t vosContext = vos_get_global_context(VOS_MODULE_ID_SME, NULL);

    if ( eHAL_STATUS_SUCCESS == sme_AcquireGlobalLock( &pMac->sme ) )
    {
        if( pVersion != NULL )
        {
            status = WDA_GetWcnssWlanReportedVersion(vosContext, pVersion);
        }
        else
        {
            status = VOS_STATUS_E_INVAL;
        }
        sme_ReleaseGlobalLock( &pMac->sme );
    }

    return (status);
}


/*                                                                            
                                   
                                                                   
                                                  
                                                                       
                                                                     
                      
                                        
                                       
                                                                             */
VOS_STATUS sme_GetWcnssSoftwareVersion(tHalHandle hHal,
                                       tANI_U8 *pVersion,
                                       tANI_U32 versionBufferSize)
{
    VOS_STATUS status = VOS_STATUS_SUCCESS;
    tpAniSirGlobal pMac = PMAC_STRUCT( hHal );
    v_CONTEXT_t vosContext = vos_get_global_context(VOS_MODULE_ID_SME, NULL);

    if ( eHAL_STATUS_SUCCESS == sme_AcquireGlobalLock( &pMac->sme ) )
    {
        if( pVersion != NULL )
        {
            status = WDA_GetWcnssSoftwareVersion(vosContext, pVersion,
                                                 versionBufferSize);
        }
        else
        {
            status = VOS_STATUS_E_INVAL;
        }
        sme_ReleaseGlobalLock( &pMac->sme );
    }

    return (status);
}


/*                                                                            
                                   
                                                                     
                                                  
                                                                       
                                                                     
                      
                                        
                                       
                                                                             */
VOS_STATUS sme_GetWcnssHardwareVersion(tHalHandle hHal,
                                       tANI_U8 *pVersion,
                                       tANI_U32 versionBufferSize)
{
    VOS_STATUS status = VOS_STATUS_SUCCESS;
    tpAniSirGlobal pMac = PMAC_STRUCT( hHal );
    v_CONTEXT_t vosContext = vos_get_global_context(VOS_MODULE_ID_SME, NULL);

    if ( eHAL_STATUS_SUCCESS == sme_AcquireGlobalLock( &pMac->sme ) )
    {
        if( pVersion != NULL )
        {
            status = WDA_GetWcnssHardwareVersion(vosContext, pVersion,
                                                 versionBufferSize);
        }
        else
        {
            status = VOS_STATUS_E_INVAL;
        }
        sme_ReleaseGlobalLock( &pMac->sme );
    }

    return (status);
}
#endif

#ifdef FEATURE_WLAN_WAPI
/*                                                                            
                            
                                                                         
                   
                                                                                
                                      
                                                                                   
                                                                                
                                                                   
                                                                                
                                                              
                                                                             */
eHalStatus sme_RoamSetBKIDCache( tHalHandle hHal, tANI_U32 sessionId, tBkidCacheInfo *pBKIDCache,
                                 tANI_U32 numItems )
{
   eHalStatus status = eHAL_STATUS_FAILURE;
   tpAniSirGlobal pMac = PMAC_STRUCT( hHal );

   status = sme_AcquireGlobalLock( &pMac->sme );
   if ( HAL_STATUS_SUCCESS( status ) )
   {
       status = csrRoamSetBKIDCache( pMac, sessionId, pBKIDCache, numItems );
       sme_ReleaseGlobalLock( &pMac->sme );
   }

   return (status);
}

/*                                                                            
                            
                                                                               
               
                                                                                
                                      
                                                                             
                                                                                  
                 
                                                                                 
               
                                                                                
               
                                                                             */
eHalStatus sme_RoamGetBKIDCache(tHalHandle hHal, tANI_U32 *pNum,
                                tBkidCacheInfo *pBkidCache)
{
   eHalStatus status = eHAL_STATUS_FAILURE;
   tpAniSirGlobal pMac = PMAC_STRUCT( hHal );

   status = sme_AcquireGlobalLock( &pMac->sme );
   if ( HAL_STATUS_SUCCESS( status ) )
   {
       smsLog(pMac, LOGE, FL(" !!!!!!!!!!!!!!!!!!SessionId is hardcoded\n"));
       status = csrRoamGetBKIDCache( pMac, 0, pNum, pBkidCache );
       sme_ReleaseGlobalLock( &pMac->sme );
   }

   return (status);
}

/*                                                                            
                               
                                                                               
                                 
                                                                                
                                      
                                                        
                                                                             */
tANI_U32 sme_RoamGetNumBKIDCache(tHalHandle hHal, tANI_U32 sessionId)
{
   eHalStatus status = eHAL_STATUS_FAILURE;
   tpAniSirGlobal pMac = PMAC_STRUCT( hHal );
   tANI_U32 numBkidCache = 0;

   status = sme_AcquireGlobalLock( &pMac->sme );
   if ( HAL_STATUS_SUCCESS( status ) )
   {
       numBkidCache = csrRoamGetNumBKIDCache( pMac, sessionId );
       sme_ReleaseGlobalLock( &pMac->sme );
   }

   return (numBkidCache);
}

/*                                                                            
                                    
                                                               
                                                                   
                                          
                                                                   
                                                                           
                                                                           
                                 
                                                                                
                                                 
                                         
                                                       
                                                     
                                                                             */
eHalStatus sme_ScanGetBKIDCandidateList(tHalHandle hHal, tANI_U32 sessionId,
                                        tBkidCandidateInfo *pBkidList,
                                        tANI_U32 *pNumItems )
{
    eHalStatus status = eHAL_STATUS_FAILURE;
    tpAniSirGlobal pMac = PMAC_STRUCT( hHal );

    status = sme_AcquireGlobalLock( &pMac->sme );
    if ( HAL_STATUS_SUCCESS( status ) )
    {
        status = csrScanGetBKIDCandidateList( pMac, sessionId, pBkidList, pNumItems );
        sme_ReleaseGlobalLock( &pMac->sme );
    }

    return (status);
}
#endif /*                   */

#ifdef FEATURE_OEM_DATA_SUPPORT

/*                                                                            
                                                      
                                                                             */

/*                                                                            
                         
                                                        
                                                         
                                                          
                           
                                                                             */
eHalStatus sme_getOemDataRsp(tHalHandle hHal, 
        tOemDataRsp **pOemDataRsp)
{
    eHalStatus status = eHAL_STATUS_SUCCESS;
    tpAniSirGlobal pMac = PMAC_STRUCT(hHal);

    do
    {
        //                                   
        status = sme_AcquireGlobalLock(&pMac->sme);

        if(!HAL_STATUS_SUCCESS(status))
        {
            break;
        }

        if(pMac->oemData.pOemDataRsp != NULL)
        {
            *pOemDataRsp = pMac->oemData.pOemDataRsp;
        }
        else
        {
            status = eHAL_STATUS_FAILURE;
        }

        //                                   
        sme_ReleaseGlobalLock( &pMac->sme );

    } while(0);

    return status;
}

/*                                                                            
                      
                                              
                                             
                                                                          
                                                                    
                                                          
                           
                                                                             */
eHalStatus sme_OemDataReq(tHalHandle hHal, 
        tANI_U8 sessionId,
        tOemDataReqConfig *pOemDataReqConfig, 
        tANI_U32 *pOemDataReqID, 
        oemData_OemDataReqCompleteCallback callback, 
        void *pContext)
{
    eHalStatus status = eHAL_STATUS_SUCCESS;
    tpAniSirGlobal pMac = PMAC_STRUCT(hHal);

    do
    {
        //                                   
        status = sme_AcquireGlobalLock(&pMac->sme);
        if(HAL_STATUS_SUCCESS(status))
        {
            tANI_U32 lOemDataReqId = pMac->oemData.oemDataReqID++; //                  

            if(pOemDataReqID)
            {
               *pOemDataReqID = lOemDataReqId;
            }
            else
               return eHAL_STATUS_FAILURE;

            status = oemData_OemDataReq(hHal, sessionId, pOemDataReqConfig, pOemDataReqID, callback, pContext);

            //                                   
            sme_ReleaseGlobalLock( &pMac->sme );
        }
    } while(0);

    smsLog(pMac, LOGW, "exiting function %s\n", __FUNCTION__);

    return(status);
}

#endif /*                        */

/*                                                                          

                                                                    

                            


                                               
                                                                                                 
                                                    
                                                                                      
                                                                                   

                                                                      

                                                               
                                                            
     

                                                                            */
eHalStatus sme_OpenSession(tHalHandle hHal, csrRoamCompleteCallback callback, void *pContext,
                           tANI_U8 *pSelfMacAddr, tANI_U8 *pbSessionId)
{
   eHalStatus status;
   tpAniSirGlobal pMac = PMAC_STRUCT( hHal );

   if( NULL == pbSessionId )
   {
      status = eHAL_STATUS_INVALID_PARAMETER;
   }
   else
   {
      status = sme_AcquireGlobalLock( &pMac->sme );
      if ( HAL_STATUS_SUCCESS( status ) )
      {
         status = csrRoamOpenSession( pMac, callback, pContext, pSelfMacAddr, pbSessionId );

         sme_ReleaseGlobalLock( &pMac->sme );
      }
   }

   return ( status );
}


/*                                                                          

                                                                     

                            


                                               

                                                    

                                                  

                                                               
                                                                
     

                                                                            */
eHalStatus sme_CloseSession(tHalHandle hHal, tANI_U8 sessionId,
                          csrRoamSessionCloseCallback callback, void *pContext)
{
   eHalStatus status;
   tpAniSirGlobal pMac = PMAC_STRUCT( hHal );

   status = sme_AcquireGlobalLock( &pMac->sme );
   if ( HAL_STATUS_SUCCESS( status ) )
   {
      status = csrRoamCloseSession( pMac, sessionId, FALSE, 
                                    callback, pContext );

      sme_ReleaseGlobalLock( &pMac->sme );
   }

   return ( status );
}

#ifdef WLAN_SOFTAP_FEATURE
/*                                                                            

                             

                                                                                                
                                

                                                                           

                                    

                                                                             

                                                                                 */
eHalStatus sme_RoamUpdateAPWPSIE(tHalHandle hHal, tANI_U8 sessionId, tSirAPWPSIEs *pAPWPSIES)
{

   eHalStatus status = eHAL_STATUS_FAILURE;
   tpAniSirGlobal pMac = PMAC_STRUCT( hHal );

   status = sme_AcquireGlobalLock( &pMac->sme );
   if ( HAL_STATUS_SUCCESS( status ) )
   {

      status = csrRoamUpdateAPWPSIE( pMac, sessionId, pAPWPSIES );

      sme_ReleaseGlobalLock( &pMac->sme );
   }

   return (status);
}
/*                                                                            

                                 

                                                                                                     
                                

                                                                                           

                                    

                                                                             

                                                                                 */
eHalStatus sme_RoamUpdateAPWPARSNIEs(tHalHandle hHal, tANI_U8 sessionId, tSirRSNie * pAPSirRSNie)
{

   eHalStatus status = eHAL_STATUS_FAILURE;
   tpAniSirGlobal pMac = PMAC_STRUCT( hHal );

   status = sme_AcquireGlobalLock( &pMac->sme );
   if ( HAL_STATUS_SUCCESS( status ) )
   {

      status = csrRoamUpdateWPARSNIEs( pMac, sessionId, pAPSirRSNie);

      sme_ReleaseGlobalLock( &pMac->sme );
   }

   return (status);
}
#endif


/*                                                                                

                        

                                                            
                                       

                                

                                                                   

                                         

                                                                               */

eHalStatus sme_sendBTAmpEvent(tHalHandle hHal, tSmeBtAmpEvent btAmpEvent)
{
  vos_msg_t msg;
  tpSmeBtAmpEvent ptrSmeBtAmpEvent = NULL;
  eHalStatus status = eHAL_STATUS_FAILURE;

  ptrSmeBtAmpEvent = vos_mem_malloc(sizeof(tpSmeBtAmpEvent));
  if (NULL == ptrSmeBtAmpEvent)
     {
        VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_ERROR, "%s: "
           "Not able to allocate memory for BTAmp event", __FUNCTION__);
        return status;
   }

  vos_mem_copy(ptrSmeBtAmpEvent, (void*)&btAmpEvent, sizeof(tSmeBtAmpEvent));
  msg.type = WDA_SIGNAL_BTAMP_EVENT;
  msg.reserved = 0;
  msg.bodyptr = ptrSmeBtAmpEvent;

  //                                               

  if(VOS_STATUS_SUCCESS != vos_mq_post_message(VOS_MODULE_ID_WDA, &msg))
  {
    VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_ERROR, "%s: "
           "Not able to post SIR_HAL_SIGNAL_BTAMP_EVENT message to HAL", __FUNCTION__);
    vos_mem_free(ptrSmeBtAmpEvent);
    return status;
  }

  return eHAL_STATUS_SUCCESS;

}

/*                                                                            
                          
                                                
                                                  
                                                       
                      
                                                                             */
eHalStatus sme_SetHostOffload (tHalHandle hHal, tANI_U8 sessionId, 
                                    tpSirHostOffloadReq pRequest)
{
    tpAniSirGlobal pMac = PMAC_STRUCT( hHal );
    eHalStatus status = eHAL_STATUS_FAILURE;
    tCsrRoamSession *pSession = CSR_GET_SESSION( pMac, sessionId );

    if(pSession == NULL )
    {
        VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_ERROR, "%s: "
           "Unable to find the csrSession", __FUNCTION__);
        return status;
    }

    if ( eHAL_STATUS_SUCCESS == ( status = sme_AcquireGlobalLock( &pMac->sme ) ) )
    {
#ifdef WLAN_NS_OFFLOAD
        if(SIR_IPV6_NS_OFFLOAD == pRequest->offloadType)
        {
            status = pmcSetNSOffload( hHal, pRequest ,pSession->connectedProfile.bssid);
        }
        else
#endif //               
        {
            status = pmcSetHostOffload (hHal, pRequest, pSession->connectedProfile.bssid);
        }
        sme_ReleaseGlobalLock( &pMac->sme );
    }

    return (status);
}

#ifdef WLAN_FEATURE_GTK_OFFLOAD
/*                                                                            
                         
                                               
                                                  
                                                           
                      
                                                                             */
eHalStatus sme_SetGTKOffload (tHalHandle hHal, tpSirGtkOffloadParams pRequest)
{
    tpAniSirGlobal pMac = PMAC_STRUCT( hHal );
    eHalStatus status;

    if ( eHAL_STATUS_SUCCESS == ( status = sme_AcquireGlobalLock( &pMac->sme ) ) )
    {
        status = pmcSetGTKOffload( hHal, pRequest );
        sme_ReleaseGlobalLock( &pMac->sme );
    }

    return (status);
}

/*                                                                            
                         
                                               
                                                  
                                                            
                      
                                                                             */
eHalStatus sme_GetGTKOffload (tHalHandle hHal, GTKOffloadGetInfoCallback callbackRoutine, void *callbackContext )
{
    tpAniSirGlobal pMac = PMAC_STRUCT( hHal );
    eHalStatus status;

    if ( eHAL_STATUS_SUCCESS == ( status = sme_AcquireGlobalLock( &pMac->sme ) ) )
    {
        pmcGetGTKOffload(hHal, callbackRoutine, callbackContext);
        sme_ReleaseGlobalLock( &pMac->sme );
    }

    return (status);
}
#endif //                         

/*                                                                            
                        
                                              
                                                  
                                                          
                      
                                                                             */
eHalStatus sme_SetKeepAlive (tHalHandle hHal, tANI_U8 sessionId, 
                                 tpSirKeepAliveReq pRequest)
{
    tpAniSirGlobal pMac = PMAC_STRUCT( hHal );
    eHalStatus status;
    tCsrRoamSession *pSession = CSR_GET_SESSION( pMac, sessionId );

    VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_ERROR, "%s: "
           "setting Keep alive in SME TP %d", __FUNCTION__,pRequest->timePeriod);

    if(pSession == NULL )
    {
		VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_ERROR, "%s: "
           " Session not Found", __FUNCTION__);
        return eHAL_STATUS_FAILURE;
    }

    if ( eHAL_STATUS_SUCCESS == ( status = sme_AcquireGlobalLock( &pMac->sme ) ) )
    {
        status = pmcSetKeepAlive (hHal, pRequest, pSession->connectedProfile.bssid);
        sme_ReleaseGlobalLock( &pMac->sme );
    }

    return (status);
}

#ifdef FEATURE_WLAN_SCAN_PNO
/*                                                                            
                                   
                                                                  
                                                  
                                                       
                      
                                                                             */
eHalStatus sme_SetPreferredNetworkList (tHalHandle hHal, tpSirPNOScanReq pRequest, tANI_U8 sessionId, void (*callbackRoutine) (void *callbackContext, tSirPrefNetworkFoundInd *pPrefNetworkFoundInd), void *callbackContext )
{
    tpAniSirGlobal pMac = PMAC_STRUCT( hHal );
    eHalStatus status;

    if ( eHAL_STATUS_SUCCESS == ( status = sme_AcquireGlobalLock( &pMac->sme ) ) )
    {
        pmcSetPreferredNetworkList(hHal, pRequest, sessionId, callbackRoutine, callbackContext);
        sme_ReleaseGlobalLock( &pMac->sme );
    }

    return (status);
}

eHalStatus sme_SetRSSIFilter(tHalHandle hHal, v_U8_t rssiThreshold)
{
    tpAniSirGlobal pMac = PMAC_STRUCT( hHal );
    eHalStatus status;

    if ( eHAL_STATUS_SUCCESS == ( status = sme_AcquireGlobalLock( &pMac->sme ) ) )
    {
        pmcSetRssiFilter(hHal, rssiThreshold);
        sme_ReleaseGlobalLock( &pMac->sme );
    }

    return (status);
}

#endif //                      

eHalStatus sme_SetPowerParams(tHalHandle hHal, tSirSetPowerParamsReq* pwParams)
{
    tpAniSirGlobal pMac = PMAC_STRUCT( hHal );
    eHalStatus status;

    if ( eHAL_STATUS_SUCCESS == ( status = sme_AcquireGlobalLock( &pMac->sme ) ) )
    {
        pmcSetPowerParams(hHal, pwParams);
        sme_ReleaseGlobalLock( &pMac->sme );
    }

    return (status);
}

/*                                                                            
                        
                                   
                                                  
                      
                                          
                                       
                                                                             */
eHalStatus sme_AbortMacScan(tHalHandle hHal)
{
    eHalStatus status;
    tpAniSirGlobal pMac = PMAC_STRUCT( hHal );

    status = sme_AcquireGlobalLock( &pMac->sme );
    if ( HAL_STATUS_SUCCESS( status ) )
    {
       status = csrScanAbortMacScan(pMac);
    
       sme_ReleaseGlobalLock( &pMac->sme );
    }
    
    return ( status );
}

/*                                                                             
                                   
                                                                
                                                                                     
                                                             
                                    
                                   
                                                                               */
eHalStatus sme_GetOperationChannel(tHalHandle hHal, tANI_U32 *pChannel, tANI_U8 sessionId)
{
    tpAniSirGlobal pMac = PMAC_STRUCT( hHal );
    tCsrRoamSession *pSession;

    if (CSR_IS_SESSION_VALID( pMac, sessionId ))
    {
       pSession = CSR_GET_SESSION( pMac, sessionId );

       if(( pSession->connectedProfile.BSSType == eCSR_BSS_TYPE_INFRASTRUCTURE ) || 
          ( pSession->connectedProfile.BSSType == eCSR_BSS_TYPE_IBSS ) ||
#ifdef WLAN_SOFTAP_FEATURE	  
          ( pSession->connectedProfile.BSSType == eCSR_BSS_TYPE_INFRA_AP ) ||
#endif
          ( pSession->connectedProfile.BSSType == eCSR_BSS_TYPE_START_IBSS ))
       {
           *pChannel =pSession->connectedProfile.operationChannel;
           return eHAL_STATUS_SUCCESS;
       }
    }
    return eHAL_STATUS_FAILURE;
}//                                  

#ifdef WLAN_FEATURE_P2P
/*                                                                            

                            

                                                                      
                                                                        
                                                                           
                               
                                                 
                       
                                                                                 */
eHalStatus sme_RegisterMgmtFrame(tHalHandle hHal, tANI_U8 sessionId, 
                     tANI_U16 frameType, tANI_U8* matchData, tANI_U16 matchLen)
{
    eHalStatus status = eHAL_STATUS_SUCCESS;
    tpAniSirGlobal pMac = PMAC_STRUCT(hHal);

    if ( eHAL_STATUS_SUCCESS == ( status = sme_AcquireGlobalLock( &pMac->sme ) ) )
    {
        tSirRegisterMgmtFrame *pMsg;
        tANI_U16 len;
        tCsrRoamSession *pSession = CSR_GET_SESSION( pMac, sessionId );

        if(!pSession)
        {
            smsLog(pMac, LOGE, FL("  session %d not found "), sessionId);
            return eHAL_STATUS_FAILURE;
        }
        
        if( !pSession->sessionActive )
        {
            VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_ERROR,
                        "%s Invalid Sessionid", __FUNCTION__);
            sme_ReleaseGlobalLock( &pMac->sme );
            return eHAL_STATUS_FAILURE;
        }
        
        len = sizeof(tSirRegisterMgmtFrame) + matchLen;
        
        status = palAllocateMemory(pMac->hHdd, (void**)&pMsg, len );
        if(HAL_STATUS_SUCCESS(status))
        {
            palZeroMemory(pMac->hHdd, pMsg, len);
            pMsg->messageType     = eWNI_SME_REGISTER_MGMT_FRAME_REQ;
            pMsg->length          = len;
            pMsg->sessionId       = sessionId;
            pMsg->registerFrame   = VOS_TRUE;
            pMsg->frameType       = frameType;
            pMsg->matchLen        = matchLen;
            palCopyMemory( pMac, pMsg->matchData, matchData, matchLen); 
            status = palSendMBMessage(pMac->hHdd, pMsg);
        }
        sme_ReleaseGlobalLock( &pMac->sme );
    }
    return status;
}

/*                                                                            

                              

                                                                         
                                                                        
                                                                           
                               
                                                 
                       
                                                                                 */
eHalStatus sme_DeregisterMgmtFrame(tHalHandle hHal, tANI_U8 sessionId, 
                     tANI_U16 frameType, tANI_U8* matchData, tANI_U16 matchLen)
{
    eHalStatus status = eHAL_STATUS_SUCCESS;
    tpAniSirGlobal pMac = PMAC_STRUCT(hHal);

    if ( eHAL_STATUS_SUCCESS == ( status = sme_AcquireGlobalLock( &pMac->sme ) ) )
    {
        tSirRegisterMgmtFrame *pMsg;
        tANI_U16 len;
        tCsrRoamSession *pSession = CSR_GET_SESSION( pMac, sessionId );

        if(!pSession)
        {
            smsLog(pMac, LOGE, FL("  session %d not found "), sessionId);
            return eHAL_STATUS_FAILURE;
        }
        
        if( !pSession->sessionActive ) 
        {
            VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_ERROR,
                        "%s Invalid Sessionid", __FUNCTION__);
            sme_ReleaseGlobalLock( &pMac->sme );
            return eHAL_STATUS_FAILURE;
        }

        len = sizeof(tSirRegisterMgmtFrame) + matchLen;
        
        status = palAllocateMemory(pMac->hHdd, (void**)&pMsg, len );
        if(HAL_STATUS_SUCCESS(status))
        {
            palZeroMemory(pMac->hHdd, pMsg, len);
            pMsg->messageType     = eWNI_SME_REGISTER_MGMT_FRAME_REQ;
            pMsg->length          = len; 
            pMsg->registerFrame   = VOS_FALSE;
            pMsg->frameType       = frameType;
            pMsg->matchLen        = matchLen;
            palCopyMemory( pMac, pMsg->matchData, matchData, matchLen); 
            status = palSendMBMessage(pMac->hHdd, pMsg);
        }
        sme_ReleaseGlobalLock( &pMac->sme );
    }
    return status;
}

/*                                                                            
                           
                                                                                          
                                                  
                               
                                     
                                                                           
                                       
                      
                                                                             */

eHalStatus sme_RemainOnChannel(tHalHandle hHal, tANI_U8 sessionId,
                               tANI_U8 channel, tANI_U32 duration,
                               remainOnChanCallback callback, 
                               void *pContext)
{
  eHalStatus status = eHAL_STATUS_SUCCESS;
  tpAniSirGlobal pMac = PMAC_STRUCT(hHal);

  if ( eHAL_STATUS_SUCCESS == ( status = sme_AcquireGlobalLock( &pMac->sme ) ) )
  {
    status = p2pRemainOnChannel (hHal, sessionId, channel, duration, callback, pContext
#ifdef WLAN_FEATURE_P2P_INTERNAL
                                , eP2PRemainOnChnReasonUnknown
#endif
                                );
    sme_ReleaseGlobalLock( &pMac->sme );
  }
  return(status);
}

/*                                                                            
                          
                                                                        
                                                  
                                                                                             
                      
                                                                             */

#ifndef WLAN_FEATURE_CONCURRENT_P2P
eHalStatus sme_ReportProbeReq(tHalHandle hHal, tANI_U8 flag)
{
    eHalStatus status = eHAL_STATUS_SUCCESS;
    tpAniSirGlobal pMac = PMAC_STRUCT(hHal);

    do
    {
        //                                   
        status = sme_AcquireGlobalLock(&pMac->sme);
        if(HAL_STATUS_SUCCESS(status))
        {
            /*                     */
            pMac->p2pContext.probeReqForwarding = flag;
            //                                   
            sme_ReleaseGlobalLock( &pMac->sme );
        }
    } while(0);

    smsLog(pMac, LOGW, "exiting function %s\n", __FUNCTION__);

    return(status);
}

/*                                                                            
                       
                                                
                                                  
                                           
                                       
                      
                                                                             */

eHalStatus sme_updateP2pIe(tHalHandle hHal, void *p2pIe, tANI_U32 p2pIeLength)
{
    eHalStatus status = eHAL_STATUS_SUCCESS;
    tpAniSirGlobal pMac = PMAC_STRUCT(hHal);

    //                                   
    status = sme_AcquireGlobalLock(&pMac->sme);
    if(HAL_STATUS_SUCCESS(status))
    {
        if(NULL != pMac->p2pContext.probeRspIe){
            vos_mem_free(pMac->p2pContext.probeRspIe);
            pMac->p2pContext.probeRspIeLength = 0;
        }

        pMac->p2pContext.probeRspIe = vos_mem_malloc(p2pIeLength);
        if (NULL == pMac->p2pContext.probeRspIe)
        {
            smsLog(pMac, LOGE, "%s: Unable to allocate P2P IE", __FUNCTION__);
            pMac->p2pContext.probeRspIeLength = 0;
            status = eHAL_STATUS_FAILURE;
        }
        else
        {
            pMac->p2pContext.probeRspIeLength = p2pIeLength;

            sirDumpBuf( pMac, SIR_LIM_MODULE_ID, LOG2,
                        pMac->p2pContext.probeRspIe,
                        pMac->p2pContext.probeRspIeLength ); 
            vos_mem_copy((tANI_U8 *)pMac->p2pContext.probeRspIe, p2pIe,
                         p2pIeLength);
        }

        //                                   
        sme_ReleaseGlobalLock( &pMac->sme );
    }
   
    smsLog(pMac, LOG2, "exiting function %s\n", __FUNCTION__);

    return(status);
}
#endif

/*                                                                            
                      
                                                     
                                                  
                      
                                                                             */

eHalStatus sme_sendAction(tHalHandle hHal, tANI_U8 sessionId,
                          const tANI_U8 *pBuf, tANI_U32 len,
                          tANI_U16 wait, tANI_BOOLEAN noack)
{
    eHalStatus status = eHAL_STATUS_SUCCESS;
    tpAniSirGlobal pMac = PMAC_STRUCT(hHal);

    //                                   
    status = sme_AcquireGlobalLock(&pMac->sme);
    if(HAL_STATUS_SUCCESS(status))
    {
        p2pSendAction(hHal, sessionId, pBuf, len, wait, noack);
        //                                   
        sme_ReleaseGlobalLock( &pMac->sme );
    }

    smsLog(pMac, LOGW, "exiting function %s\n", __FUNCTION__);

    return(status);
}

eHalStatus sme_CancelRemainOnChannel(tHalHandle hHal, tANI_U8 sessionId )
{
  eHalStatus status = eHAL_STATUS_SUCCESS;
  tpAniSirGlobal pMac = PMAC_STRUCT(hHal);

  if ( eHAL_STATUS_SUCCESS == ( status = sme_AcquireGlobalLock( &pMac->sme ) ) )
  {
    status = p2pCancelRemainOnChannel (hHal, sessionId);
    sme_ReleaseGlobalLock( &pMac->sme );
  }
  return(status);
}

//                  
eHalStatus sme_p2pSetPs(tHalHandle hHal, tP2pPsConfig * data)
{
  eHalStatus status = eHAL_STATUS_SUCCESS;
  tpAniSirGlobal pMac = PMAC_STRUCT(hHal);

  if ( eHAL_STATUS_SUCCESS == ( status = sme_AcquireGlobalLock( &pMac->sme ) ) )
  {
    status = p2pSetPs (hHal, data);
    sme_ReleaseGlobalLock( &pMac->sme );
  }
  return(status);
}

#endif

/*                                                                            

                               

         
                                                                              
                                  

         

                                           
 
                                                                          
                                                                               
                                                                              
                               
                         
                         
                                   

   
                        
  
  
                                                                            */
eHalStatus sme_ConfigureRxpFilter( tHalHandle hHal, 
                            tpSirWlanSetRxpFilters  wlanRxpFilterParam)
{
    eHalStatus status = eHAL_STATUS_SUCCESS;
    VOS_STATUS vosStatus = VOS_STATUS_SUCCESS;
    tpAniSirGlobal pMac = PMAC_STRUCT(hHal);
    vos_msg_t       vosMessage;

    if ( eHAL_STATUS_SUCCESS == ( status = sme_AcquireGlobalLock( &pMac->sme ) ) )
    {
        /*                                     */
        vosMessage.bodyptr = wlanRxpFilterParam;
        vosMessage.type         = WDA_CFG_RXP_FILTER_REQ;
        vosStatus = vos_mq_post_message( VOS_MQ_ID_WDA, &vosMessage );
        if ( !VOS_IS_STATUS_SUCCESS(vosStatus) )
        {
           status = eHAL_STATUS_FAILURE;
        }
        sme_ReleaseGlobalLock( &pMac->sme );
    }
    return(status);
}

#ifdef FEATURE_WLAN_INTEGRATED_SOC
/*                                                                            

                                

         
                                                                               
                

         

                                           
 
                                                     

   
                        
  
  
                                                                            */
eHalStatus sme_ConfigureSuspendInd( tHalHandle hHal, 
                          tpSirWlanSuspendParam  wlanSuspendParam)
{
    eHalStatus status = eHAL_STATUS_SUCCESS;
    VOS_STATUS vosStatus = VOS_STATUS_SUCCESS;
    tpAniSirGlobal pMac = PMAC_STRUCT(hHal);
    vos_msg_t       vosMessage;

    if ( eHAL_STATUS_SUCCESS == ( status = sme_AcquireGlobalLock( &pMac->sme ) ) )
    {
        /*                                     */
        vosMessage.bodyptr = wlanSuspendParam;
        vosMessage.type    = WDA_WLAN_SUSPEND_IND;
        vosStatus = vos_mq_post_message( VOS_MQ_ID_WDA, &vosMessage );
        if ( !VOS_IS_STATUS_SUCCESS(vosStatus) )
        {
           status = eHAL_STATUS_FAILURE;
        }
        sme_ReleaseGlobalLock( &pMac->sme );
    }
    return(status);
}

/*                                                                            

                               

         
                                                                               
              

         

                                           
 
                                                   

   
                        
  
  
                                                                            */
eHalStatus sme_ConfigureResumeReq( tHalHandle hHal, 
                             tpSirWlanResumeParam  wlanResumeParam)
{
    eHalStatus status = eHAL_STATUS_SUCCESS;
    VOS_STATUS vosStatus = VOS_STATUS_SUCCESS;
    tpAniSirGlobal pMac = PMAC_STRUCT(hHal);
    vos_msg_t       vosMessage;

    if ( eHAL_STATUS_SUCCESS == ( status = sme_AcquireGlobalLock( &pMac->sme ) ) )
    {
        /*                                     */
        vosMessage.bodyptr = wlanResumeParam;
        vosMessage.type    = WDA_WLAN_RESUME_REQ;
        vosStatus = vos_mq_post_message( VOS_MQ_ID_WDA, &vosMessage );
        if ( !VOS_IS_STATUS_SUCCESS(vosStatus) )
        {
           status = eHAL_STATUS_FAILURE;
        }
        sme_ReleaseGlobalLock( &pMac->sme );
    }
    return(status);
}

#endif
/*                                                                            

                             

                                                                
                              

                                                 

                                                           

                                                                                 */
tANI_S8 sme_GetInfraSessionId(tHalHandle hHal)
{
   eHalStatus status = eHAL_STATUS_FAILURE;
   tANI_S8 sessionid = -1;
   tpAniSirGlobal pMac = PMAC_STRUCT( hHal );
                   
   status = sme_AcquireGlobalLock( &pMac->sme );
   if ( HAL_STATUS_SUCCESS( status ) )
   {

      sessionid = csrGetInfraSessionId( pMac);

      sme_ReleaseGlobalLock( &pMac->sme );
   }

   return (sessionid);
}

/*                                                                            

                                    

                                                                       
                              

                                                 
                                                                 

                                                                  

                                                                                 */
tANI_U8 sme_GetInfraOperationChannel( tHalHandle hHal, tANI_U8 sessionId)
{
   eHalStatus status = eHAL_STATUS_FAILURE;
   tpAniSirGlobal pMac = PMAC_STRUCT( hHal );
   tANI_U8 channel = 0;
   status = sme_AcquireGlobalLock( &pMac->sme );
   if ( HAL_STATUS_SUCCESS( status ) )
   {

      channel = csrGetInfraOperationChannel( pMac, sessionId);

      sme_ReleaseGlobalLock( &pMac->sme );
   }

   return (channel);
}

//                                                                                                           
//                                                         
tANI_U8 sme_GetConcurrentOperationChannel( tHalHandle hHal )
{
   eHalStatus status = eHAL_STATUS_FAILURE;
   tpAniSirGlobal pMac = PMAC_STRUCT( hHal );   
   tANI_U8 channel = 0;
   status = sme_AcquireGlobalLock( &pMac->sme );
   if ( HAL_STATUS_SUCCESS( status ) )
   {

      channel = csrGetConcurrentOperationChannel( pMac );
      VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_INFO_HIGH, "%s: "
           " Other Concurrent Channel = %d", __FUNCTION__,channel);
      sme_ReleaseGlobalLock( &pMac->sme );
   }

   return (channel);
}

#ifdef FEATURE_WLAN_SCAN_PNO
/*                                                                             
 
                                    
 
              
                                               
 
             
                                 
                                     
 
                     
 
                                                                             */
eHalStatus sme_PreferredNetworkFoundInd (tHalHandle hHal, void* pMsg)
{
   tpAniSirGlobal pMac = PMAC_STRUCT(hHal);
   eHalStatus status = eHAL_STATUS_SUCCESS;
   tSirPrefNetworkFoundInd *pPrefNetworkFoundInd = (tSirPrefNetworkFoundInd *)pMsg;

   if (NULL == pMsg)
   {
      smsLog(pMac, LOGE, "in %s msg ptr is NULL\n", __FUNCTION__);
      status = eHAL_STATUS_FAILURE;
   }
   else
   {
      if (pPrefNetworkFoundInd->ssId.length > 0)
      {
          smsLog(pMac, LOG1, "Preferred Network Found Indication in %s(), SSID=%s", 
                 __FUNCTION__, pPrefNetworkFoundInd->ssId.ssId);


         /*                                                           */
         if (pMac->pmc.prefNetwFoundCB != NULL)
         {    
               pMac->pmc.prefNetwFoundCB(pMac->pmc.preferredNetworkFoundIndCallbackContext, pPrefNetworkFoundInd);
         }

      }
      else
      {
         smsLog(pMac, LOGE, "%s: callback failed - SSID is NULL\n", __FUNCTION__);
         status = eHAL_STATUS_FAILURE;
      }
   }


   return(status);
}

#endif //                      


eHalStatus sme_GetCfgValidChannels(tHalHandle hHal, tANI_U8 *aValidChannels, tANI_U32 *len)
{
    eHalStatus status = eHAL_STATUS_FAILURE;
    tpAniSirGlobal pMac = PMAC_STRUCT( hHal );
           
    status = sme_AcquireGlobalLock( &pMac->sme );
    if ( HAL_STATUS_SUCCESS( status ) )
    {
        status = csrGetCfgValidChannels(pMac, aValidChannels, len);
        sme_ReleaseGlobalLock( &pMac->sme );
    }
    
    return (status);
}


/*                                                                            

                            

                                                       

                                                 
                                                               

                      

                                                                                 */
eHalStatus sme_SetTxPerTracking(tHalHandle hHal, 
                                void (*pCallbackfn) (void *pCallbackContext), 
                                void *pCallbackContext,
                                tpSirTxPerTrackingParam pTxPerTrackingParam)
{
    vos_msg_t msg;
    tpSirTxPerTrackingParam pTxPerTrackingParamReq = NULL;
    tpAniSirGlobal pMac = PMAC_STRUCT( hHal );

    if ( eHAL_STATUS_SUCCESS == sme_AcquireGlobalLock( &pMac->sme ) )
    {
        pMac->sme.pTxPerHitCallback = pCallbackfn;
        pMac->sme.pTxPerHitCbContext = pCallbackContext;
        sme_ReleaseGlobalLock( &pMac->sme );
    }
    
    //                                                                  
    pTxPerTrackingParamReq = vos_mem_malloc(sizeof(tSirTxPerTrackingParam));
    if (NULL == pTxPerTrackingParamReq)
    {
        VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_ERROR, "%s: Not able to allocate memory for tSirTxPerTrackingParam", __FUNCTION__);
        return eHAL_STATUS_FAILURE;
    }

    vos_mem_copy(pTxPerTrackingParamReq, (void*)pTxPerTrackingParam, sizeof(tSirTxPerTrackingParam));
    msg.type = WDA_SET_TX_PER_TRACKING_REQ;
    msg.reserved = 0;
    msg.bodyptr = pTxPerTrackingParamReq;

    if(VOS_STATUS_SUCCESS != vos_mq_post_message(VOS_MODULE_ID_WDA, &msg))
    {
        VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_ERROR, "%s: Not able to post WDA_SET_TX_PER_TRACKING_REQ message to WDA", __FUNCTION__);
        vos_mem_free(pTxPerTrackingParamReq);
        return eHAL_STATUS_FAILURE;
    }

    return eHAL_STATUS_SUCCESS;
}

/*                                                                            

                                   

                                                           

                                  
                                     
                                     
                                                                
                                                                   
                                                                           
                                                                
                                                                         
                                                         
                                                                                       
                       
                                                                                                         

                                                 
                               

                      

                                                                                 */
eHalStatus sme_HandleChangeCountryCode(tpAniSirGlobal pMac,  void *pMsgBuf)
{
   eHalStatus  status = eHAL_STATUS_SUCCESS;
   tAniChangeCountryCodeReq *pMsg;
   v_REGDOMAIN_t domainIdIoctl; 
   VOS_STATUS vosStatus = VOS_STATUS_SUCCESS;
   static uNvTables nvTables;
   pMsg = (tAniChangeCountryCodeReq *)pMsgBuf;


   /*                                                                                                             */
   if( VOS_TRUE == vos_mem_compare(pMsg->countryCode, SME_INVALID_COUNTRY_CODE, 2) )
   {
       pMac->roam.configParam.Is11dSupportEnabled = pMac->roam.configParam.Is11dSupportEnabledOriginal;

       vosStatus = vos_nv_readDefaultCountryTable( &nvTables );

       /*                                          */
       if ( VOS_IS_STATUS_SUCCESS(vosStatus) )
       {
           palCopyMemory( pMac->hHdd, pMsg->countryCode , nvTables.defaultCountryTable.countryCode, WNI_CFG_COUNTRY_CODE_LEN );
       }
       else
       {
           status = eHAL_STATUS_FAILURE;
           return status;
       }
   }
   else
   {
       /*                                                      */
       if(pMac->roam.configParam.fSupplicantCountryCodeHasPriority)
       {
           pMac->roam.configParam.Is11dSupportEnabled = eANI_BOOLEAN_FALSE;
       }
   }

   /*                            
                              
                                     
                                         
                                       
                             */
   if(pMac->roam.configParam.Is11dSupportEnabled &&
      pMac->roam.configParam.fEnforce11dChannels &&
      pMac->roam.configParam.fEnforceCountryCodeMatch &&
      pMac->roam.configParam.fEnforceDefaultDomain &&
      !csrSave11dCountryString(pMac, pMsg->countryCode, eANI_BOOLEAN_TRUE))
   {
      /*                                            
                                      
                                                                 */
      return eHAL_STATUS_SUCCESS;
   }

   /*                                                        */
   status = csrSetCountryCode(pMac, pMsg->countryCode, NULL);
   if(eHAL_STATUS_SUCCESS != status)
   {
       /*                                                      */
       pMac->roam.configParam.Is11dSupportEnabled = pMac->roam.configParam.Is11dSupportEnabledOriginal;
       smsLog(pMac, LOGE, "Set Country Code Fail %d", status);
       return status;  
   }

   /*                           
                                                                                                                          
                                               
   */
   csrScanFlushResult(pMac);

   /*                                    */
   palCopyMemory(pMac->hHdd, pMac->scan.countryCodeDefault, pMac->scan.countryCodeCurrent, WNI_CFG_COUNTRY_CODE_LEN);

   /*                                 */
   status = csrGetRegulatoryDomainForCountry( pMac, pMac->scan.countryCodeCurrent,(v_REGDOMAIN_t *) &domainIdIoctl );
   if ( status != eHAL_STATUS_SUCCESS )
   {
       smsLog( pMac, LOGE, FL("  fail to get regId %d\n"), domainIdIoctl );
       return status;
   }

   status = WDA_SetRegDomain(pMac, domainIdIoctl);

   if ( status != eHAL_STATUS_SUCCESS )
   {
       smsLog( pMac, LOGE, FL("  fail to set regId %d\n"), domainIdIoctl );
       return status;
   }

   /*                          */
   pMac->scan.domainIdDefault = pMac->scan.domainIdCurrent;

   /*                                  */
   status = csrInitGetChannels( pMac );

   if ( status != eHAL_STATUS_SUCCESS )
   {
       smsLog( pMac, LOGE, FL("  fail to get Channels \n"));
       return status;
   }

   /*                                             */
   csrResetCountryInformation(pMac, eANI_BOOLEAN_TRUE);
   if( pMsg->changeCCCallback )
   {
      ((tSmeChangeCountryCallback)(pMsg->changeCCCallback))((void *)pMsg->pDevContext);
   }

   return eHAL_STATUS_SUCCESS;
}

#ifdef WLAN_FEATURE_PACKET_FILTERING
eHalStatus sme_8023MulticastList (tHalHandle hHal, tpSirRcvFltMcAddrList pMulticastAddrs)
{
    tpSirRcvFltMcAddrList   pRequestBuf;
    vos_msg_t               msg;
    tpAniSirGlobal          pMac = PMAC_STRUCT(hHal);
    tANI_U8                 sessionId = 0;
    tCsrRoamSession         *pSession = NULL;

    VOS_TRACE( VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_INFO, "%s: "
               "ulMulticastAddrCnt=%d, multicastAddr[0]=%d", __FUNCTION__,
               pMulticastAddrs->ulMulticastAddrCnt,
               pMulticastAddrs->multicastAddr[0]);

    /*
                                                             
     */
     for( sessionId = 0; sessionId < CSR_ROAM_SESSION_MAX; sessionId++ )
     {
         if( CSR_IS_SESSION_VALID( pMac, sessionId ) && 
           ( csrIsConnStateInfra( pMac, sessionId )) )
         {
            pSession = CSR_GET_SESSION( pMac, sessionId );
         }
     }

    if(pSession == NULL )
    {
        VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_ERROR, "%s: Unable to find "
            "the right session", __FUNCTION__);
        return eHAL_STATUS_FAILURE;
    }
    
    pRequestBuf = vos_mem_malloc(sizeof(tSirRcvFltMcAddrList));
    if (NULL == pRequestBuf)
    {
        VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_ERROR, "%s: Not able to "
            "allocate memory for 8023 Multicast List request", __FUNCTION__);
        return eHAL_STATUS_FAILED_ALLOC;
    }
    vos_mem_copy(pRequestBuf, pMulticastAddrs, sizeof(tSirRcvFltMcAddrList));

    vos_mem_copy(pRequestBuf->selfMacAddr, pSession->selfMacAddr, sizeof(tSirMacAddr));
    vos_mem_copy(pRequestBuf->bssId, pSession->connectedProfile.bssid, 
                 sizeof(tSirMacAddr));

    msg.type = WDA_8023_MULTICAST_LIST_REQ;
    msg.reserved = 0;
    msg.bodyptr = pRequestBuf;
    if(VOS_STATUS_SUCCESS != vos_mq_post_message(VOS_MODULE_ID_WDA, &msg))
    {
        VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_ERROR, "%s: Not able to "
            "post WDA_8023_MULTICAST_LIST message to WDA", __FUNCTION__);
        vos_mem_free(pRequestBuf);
        return eHAL_STATUS_FAILURE;
    }

    return eHAL_STATUS_SUCCESS;
}

eHalStatus sme_ReceiveFilterSetFilter(tHalHandle hHal, tpSirRcvPktFilterCfgType pRcvPktFilterCfg, 
                                           tANI_U8 sessionId)
{
    tpSirRcvPktFilterCfgType    pRequestBuf;
    v_SINT_t                allocSize;
    vos_msg_t               msg;
    tpAniSirGlobal          pMac = PMAC_STRUCT(hHal);
    tCsrRoamSession         *pSession = CSR_GET_SESSION( pMac, sessionId );
    v_U8_t   idx=0;

    VOS_TRACE( VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_INFO, "%s: filterType=%d, "
               "filterId = %d", __FUNCTION__,
               pRcvPktFilterCfg->filterType, pRcvPktFilterCfg->filterId);
  
    allocSize = sizeof(tSirRcvPktFilterCfgType) + 
                      ((pRcvPktFilterCfg->numFieldParams - 1) * 
                      sizeof(tSirRcvPktFilterFieldParams));
    pRequestBuf = vos_mem_malloc(allocSize);
    if ((NULL == pRequestBuf) || (NULL == pSession))
    {
        VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_ERROR, "%s: Not able to "
            "allocate memory for Receive Filter Set Filter request", __FUNCTION__);
        return eHAL_STATUS_FAILED_ALLOC;
    }
    vos_mem_copy(pRequestBuf, pRcvPktFilterCfg, allocSize);

    vos_mem_copy( pRequestBuf->selfMacAddr, pSession->selfMacAddr, sizeof(tSirMacAddr));
    vos_mem_copy( pRequestBuf->bssId, pSession->connectedProfile.bssid, 
                          sizeof(tSirMacAddr));

    msg.type = WDA_RECEIVE_FILTER_SET_FILTER_REQ;
    msg.reserved = 0;
    msg.bodyptr = pRequestBuf;

    VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_INFO, "Pkt Flt Req : "
           "FT %d FID %d ", 
           pRequestBuf->filterType, pRequestBuf->filterId);    

    VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_INFO, "Pkt Flt Req : "
           "params %d CT %d", 
           pRequestBuf->numFieldParams, pRequestBuf->coalesceTime);    

    for (idx=0; idx<pRequestBuf->numFieldParams; idx++)
    {

      VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_INFO,
           "Proto %d Comp Flag %d \n",
           pRequestBuf->paramsData[idx].protocolLayer,
           pRequestBuf->paramsData[idx].cmpFlag);

      VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_INFO,
           "Data Offset %d Data Len %d\n",
           pRequestBuf->paramsData[idx].dataOffset,
           pRequestBuf->paramsData[idx].dataLength);

      VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_INFO,
          "CData: %d:%d:%d:%d:%d:%d\n",
           pRequestBuf->paramsData[idx].compareData[0],
           pRequestBuf->paramsData[idx].compareData[1], 
           pRequestBuf->paramsData[idx].compareData[2],
           pRequestBuf->paramsData[idx].compareData[3],
           pRequestBuf->paramsData[idx].compareData[4],
           pRequestBuf->paramsData[idx].compareData[5]);

      VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_INFO, 
           "MData: %d:%d:%d:%d:%d:%d\n",
           pRequestBuf->paramsData[idx].dataMask[0],
           pRequestBuf->paramsData[idx].dataMask[1], 
           pRequestBuf->paramsData[idx].dataMask[2],
           pRequestBuf->paramsData[idx].dataMask[3],
           pRequestBuf->paramsData[idx].dataMask[4],
           pRequestBuf->paramsData[idx].dataMask[5]);

    }

    if(VOS_STATUS_SUCCESS != vos_mq_post_message(VOS_MODULE_ID_WDA, &msg))
    {
        VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_ERROR, "%s: Not able to post "
            "WDA_RECEIVE_FILTER_SET_FILTER message to WDA", __FUNCTION__);
        vos_mem_free(pRequestBuf);
        return eHAL_STATUS_FAILURE;
    }

    return eHAL_STATUS_SUCCESS;
}

eHalStatus sme_GetFilterMatchCount(tHalHandle hHal, 
                                   FilterMatchCountCallback callbackRoutine,
                                   void *callbackContext )
{
    tpAniSirGlobal pMac = PMAC_STRUCT( hHal );
    eHalStatus status;

    VOS_TRACE( VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_INFO, "+%s", __FUNCTION__);

    if ( eHAL_STATUS_SUCCESS == ( status = sme_AcquireGlobalLock( &pMac->sme)))
    {
        pmcGetFilterMatchCount(hHal, callbackRoutine, callbackContext);
        sme_ReleaseGlobalLock( &pMac->sme );
    }

    VOS_TRACE( VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_INFO, "-%s", __FUNCTION__);

    return (status);
}

eHalStatus sme_ReceiveFilterClearFilter(tHalHandle hHal, tpSirRcvFltPktClearParam pRcvFltPktClearParam, 
                                             tANI_U8 sessionId)
{
    tpSirRcvFltPktClearParam pRequestBuf;
    vos_msg_t               msg;
    tpAniSirGlobal          pMac = PMAC_STRUCT(hHal);
    tCsrRoamSession         *pSession = CSR_GET_SESSION( pMac, sessionId );

    VOS_TRACE( VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_INFO, "%s: filterId = %d", __FUNCTION__,
               pRcvFltPktClearParam->filterId);
  
    pRequestBuf = vos_mem_malloc(sizeof(tSirRcvFltPktClearParam));
    if ((NULL == pRequestBuf) || (NULL == pSession))
    {
        VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_ERROR,
            "%s: Not able to allocate memory for Receive Filter "
            "Clear Filter request", __FUNCTION__);
        return eHAL_STATUS_FAILED_ALLOC;
    }
    vos_mem_copy(pRequestBuf, pRcvFltPktClearParam, sizeof(tSirRcvFltPktClearParam));

    vos_mem_copy( pRequestBuf->selfMacAddr, pSession->selfMacAddr, sizeof(tSirMacAddr));
    vos_mem_copy( pRequestBuf->bssId, pSession->connectedProfile.bssid, 
                          sizeof(tSirMacAddr));

    msg.type = WDA_RECEIVE_FILTER_CLEAR_FILTER_REQ;
    msg.reserved = 0;
    msg.bodyptr = pRequestBuf;
    if(VOS_STATUS_SUCCESS != vos_mq_post_message(VOS_MODULE_ID_WDA, &msg))
    {
        VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_ERROR, "%s: Not able to post "
            "WDA_RECEIVE_FILTER_CLEAR_FILTER message to WDA", __FUNCTION__);
        vos_mem_free(pRequestBuf);
        return eHAL_STATUS_FAILURE;
    }

    return eHAL_STATUS_SUCCESS;
}
#endif //                              

/*                                                                            
                                          
                                                                             
                            
                          
                 
                                                                             */
void sme_PreChannelSwitchIndFullPowerCB(void *callbackContext, 
                eHalStatus status)
{
    tpAniSirGlobal pMac = (tpAniSirGlobal)callbackContext;
    tSirMbMsg *pMsg;
    tANI_U16 msgLen;

    msgLen = (tANI_U16)(sizeof( tSirMbMsg ));
    status = palAllocateMemory(pMac->hHdd, (void **)&pMsg, msgLen);
    if(HAL_STATUS_SUCCESS(status))
    {
        palZeroMemory(pMac->hHdd, (void *)pMsg, msgLen);
        pMsg->type = pal_cpu_to_be16((tANI_U16)eWNI_SME_PRE_CHANNEL_SWITCH_FULL_POWER);
        pMsg->msgLen = pal_cpu_to_be16(msgLen);
        status = palSendMBMessage(pMac->hHdd, pMsg);
    }                             

    return;
}

/*                                                                            
                                     
                                                                   
               
                                                        
                                                                             */
eHalStatus sme_HandlePreChannelSwitchInd(tHalHandle hHal)
{
   eHalStatus status = eHAL_STATUS_FAILURE;
   tpAniSirGlobal pMac = PMAC_STRUCT( hHal );
   status = sme_AcquireGlobalLock( &pMac->sme );
   if ( HAL_STATUS_SUCCESS( status ) )
   {
       status = pmcRequestFullPower(hHal, sme_PreChannelSwitchIndFullPowerCB, 
                            pMac, eSME_FULL_PWR_NEEDED_BY_CHANNEL_SWITCH); 
       sme_ReleaseGlobalLock( &pMac->sme );
   }

   return (status);
}

/*                                                                            
                                      
                                                                    
               
                                                        
                                                                             */
eHalStatus sme_HandlePostChannelSwitchInd(tHalHandle hHal)
{
   eHalStatus status = eHAL_STATUS_FAILURE;
   tpAniSirGlobal pMac = PMAC_STRUCT( hHal );

   status = sme_AcquireGlobalLock( &pMac->sme );
   if ( HAL_STATUS_SUCCESS( status ) )
   {
       status = pmcRequestBmps(hHal, NULL, NULL);
       sme_ReleaseGlobalLock( &pMac->sme );
   }

   return (status);
}

/*                                                                            

                          

                                                                            
                              

                                                 
                                      

                                                

                                                                                 */
tANI_BOOLEAN sme_IsChannelValid(tHalHandle hHal, tANI_U8 channel)
{
   eHalStatus status = eHAL_STATUS_FAILURE;
   tANI_BOOLEAN valid = FALSE;
   tpAniSirGlobal pMac = PMAC_STRUCT( hHal );
                   
   status = sme_AcquireGlobalLock( &pMac->sme );
   if ( HAL_STATUS_SUCCESS( status ) )
   {

      valid = csrRoamIsChannelValid( pMac, channel);

      sme_ReleaseGlobalLock( &pMac->sme );
   }

   return (valid);
}

/*                                                                            
                       
                                       
                
                                       
                        
                                                                             */
eHalStatus sme_SetFreqBand(tHalHandle hHal, eCsrBand eBand)
{
   eHalStatus status = eHAL_STATUS_FAILURE;
   tpAniSirGlobal pMac = PMAC_STRUCT( hHal );

   status = sme_AcquireGlobalLock( &pMac->sme );
   if ( HAL_STATUS_SUCCESS( status ) )
   {
      status = csrSetBand(hHal, eBand);
      sme_ReleaseGlobalLock( &pMac->sme );
   }
   return status;
}

/*                                                                            
                       
                                                  
                
                                      
                        
                                                                             */
eHalStatus sme_GetFreqBand(tHalHandle hHal, eCsrBand *pBand)
{
   eHalStatus status = eHAL_STATUS_FAILURE;
   tpAniSirGlobal pMac = PMAC_STRUCT( hHal );

   status = sme_AcquireGlobalLock( &pMac->sme );
   if ( HAL_STATUS_SUCCESS( status ) )
   {
      *pBand = csrGetCurrentBand( hHal );
      sme_ReleaseGlobalLock( &pMac->sme );
   }
   return status;
}

#ifdef WLAN_WAKEUP_EVENTS
/*                                                                             
                               

        
                                                                                      

                                     
                                                                                       

                      
                                                                             */
eHalStatus sme_WakeReasonIndCallback (tHalHandle hHal, void* pMsg)
{
   tpAniSirGlobal pMac = PMAC_STRUCT(hHal);
   eHalStatus status = eHAL_STATUS_SUCCESS;
   tSirWakeReasonInd *pWakeReasonInd = (tSirWakeReasonInd *)pMsg;

   if (NULL == pMsg)
   {
      smsLog(pMac, LOGE, "in %s msg ptr is NULL\n", __FUNCTION__);
      status = eHAL_STATUS_FAILURE;
   }
   else
   {
      smsLog(pMac, LOG2, "SME: entering sme_WakeReasonIndCallback\n");

      /*                                               */
      if (pMac->pmc.wakeReasonIndCB != NULL)
          pMac->pmc.wakeReasonIndCB(pMac->pmc.wakeReasonIndCBContext, pWakeReasonInd);
        
      pMac->pmc.wakeReasonIndCB = NULL;
      pMac->pmc.wakeReasonIndCBContext = NULL;

      smsLog(pMac, LOG1, "Wake Reason Indication in %s(), reason=%d", __FUNCTION__, pWakeReasonInd->ulReason);
   }

   return(status);
}
#endif //                   


/*                                                                            

                         

                                                                              
                             

                
                                                 
                                                   
                                     
                        

                                                                                 */
eHalStatus sme_SetMaxTxPower(tHalHandle hHal, tSirMacAddr pBssid, 
                             tSirMacAddr pSelfMacAddress, v_S7_t dB)
{
    vos_msg_t msg;
    tpMaxTxPowerParams pMaxTxParams = NULL;

    pMaxTxParams = vos_mem_malloc(sizeof(tMaxTxPowerParams));
    if (NULL == pMaxTxParams)
    {
        VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_ERROR, "%s: Not able to allocate memory for pMaxTxParams", __FUNCTION__);
        return eHAL_STATUS_FAILURE;
    }

    vos_mem_copy(pMaxTxParams->bssId, pBssid, SIR_MAC_ADDR_LENGTH);
    vos_mem_copy(pMaxTxParams->selfStaMacAddr , pSelfMacAddress, 
                 SIR_MAC_ADDR_LENGTH);
    pMaxTxParams->power = dB;

    msg.type = WDA_SET_MAX_TX_POWER_REQ;
    msg.reserved = 0;
    msg.bodyptr = pMaxTxParams;

    if(VOS_STATUS_SUCCESS != vos_mq_post_message(VOS_MODULE_ID_WDA, &msg))
    {
        VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_ERROR, "%s: Not able to post WDA_SET_MAX_TX_POWER_REQ message to WDA", __FUNCTION__);
        vos_mem_free(pMaxTxParams);
        return eHAL_STATUS_FAILURE;
    }

    return eHAL_STATUS_SUCCESS;
}

#ifdef WLAN_SOFTAP_FEATURE
/*                                                                            

                    

                                                              
                             

               
                    
                                                                    
                        

                                                                                 */
eHalStatus sme_HideSSID(tHalHandle hHal, v_U8_t sessionId, v_U8_t ssidHidden)
{
    eHalStatus status   = eHAL_STATUS_SUCCESS;
    tpAniSirGlobal pMac = PMAC_STRUCT(hHal);
    tANI_U16 len;

    if ( eHAL_STATUS_SUCCESS == ( status = sme_AcquireGlobalLock( &pMac->sme ) ) )
    {
        tpSirUpdateParams pMsg;
        tCsrRoamSession *pSession = CSR_GET_SESSION( pMac, sessionId );

        if(!pSession)
        {
            smsLog(pMac, LOGE, FL("  session %d not found "), sessionId);
            return eHAL_STATUS_FAILURE;
        }
        
        if( !pSession->sessionActive ) 
            VOS_ASSERT(0);

        /*                                    */
        len = sizeof(tSirUpdateParams); 
        status = palAllocateMemory( pMac->hHdd, (void **)&pMsg, len );
        if(HAL_STATUS_SUCCESS(status))
        {
            palZeroMemory(pMac->hHdd, pMsg, sizeof(tSirUpdateParams) );
            pMsg->messageType     = eWNI_SME_HIDE_SSID_REQ;
            pMsg->length          = len;
            /*                       */
            pMsg->sessionId       = sessionId;
            pMsg->ssidHidden      = ssidHidden; 
            status = palSendMBMessage(pMac->hHdd, pMsg);
        }
        sme_ReleaseGlobalLock( &pMac->sme );
    }
   return status;
}
#endif

/*                                                                            

                      
                                                
                                                  
                                                     
                                                              
                           
                                                                             */
eHalStatus sme_SetTmLevel(tHalHandle hHal, v_U16_t newTMLevel, v_U16_t tmMode)
{
    eHalStatus          status    = eHAL_STATUS_SUCCESS;
    VOS_STATUS          vosStatus = VOS_STATUS_SUCCESS;
    tpAniSirGlobal      pMac      = PMAC_STRUCT(hHal);
    vos_msg_t           vosMessage;
    tAniSetTmLevelReq  *setTmLevelReq = NULL;

    if ( eHAL_STATUS_SUCCESS == ( status = sme_AcquireGlobalLock( &pMac->sme ) ) )
    {
        setTmLevelReq = (tAniSetTmLevelReq *)vos_mem_malloc(sizeof(tAniSetTmLevelReq));
        if(NULL == setTmLevelReq)
        {
           VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_ERROR,
                     "%s: Not able to allocate memory for sme_SetTmLevel", __FUNCTION__);
           return eHAL_STATUS_FAILURE;
        }

        setTmLevelReq->tmMode     = tmMode;
        setTmLevelReq->newTmLevel = newTMLevel;

        /*                                     */
        vosMessage.bodyptr = setTmLevelReq;
        vosMessage.type    = WDA_SET_TM_LEVEL_REQ;
        vosStatus = vos_mq_post_message( VOS_MQ_ID_WDA, &vosMessage );
        if ( !VOS_IS_STATUS_SUCCESS(vosStatus) )
        {
           VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_ERROR,
                     "%s: Post Set TM Level MSG fail", __FUNCTION__);
           vos_mem_free(setTmLevelReq);
           status = eHAL_STATUS_FAILURE;
        }
        sme_ReleaseGlobalLock( &pMac->sme );
    }
    return(status);
}

/*                                                                           

                                                                                   
              

                                      

              

                                                                           */
void sme_featureCapsExchange( tHalHandle hHal)
{
    v_CONTEXT_t vosContext = vos_get_global_context(VOS_MODULE_ID_SME, NULL);
    WDA_featureCapsExchange(vosContext);
}


/*                                                                            

                                 

                                               

                
                    
                        

                                                                                 */
eHalStatus sme_GetDefaultCountryCodeFrmNv(tHalHandle hHal, tANI_U8 *pCountry)
{
    tpAniSirGlobal pMac = PMAC_STRUCT( hHal );
    return csrGetDefaultCountryCodeFrmNv(pMac, pCountry);
}

/*                                                                            

                                 

                                       

                
                    
                        

                                                                                 */
eHalStatus sme_GetCurrentCountryCode(tHalHandle hHal, tANI_U8 *pCountry)
{
    tpAniSirGlobal pMac = PMAC_STRUCT( hHal );
    return csrGetCurrentCountryCode(pMac, pCountry);
}

/*                                                                            
                          
                                                     
                                                                               
                                                                     
                                                         
                                                                               
                                                                   
                  
                                                                             */
void sme_transportDebug
(
   v_BOOL_t  displaySnapshot,
   v_BOOL_t  toggleStallDetect
)
{
   WDA_TransportChannelDebug(displaySnapshot, toggleStallDetect);
}
