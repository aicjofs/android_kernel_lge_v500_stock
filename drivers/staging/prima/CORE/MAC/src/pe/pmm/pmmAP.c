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

#include "sirCommon.h"

#include "aniGlobal.h"

#include "schApi.h"
#include "limApi.h"
#include "cfgApi.h"
#include "wniCfgAp.h"

#include "pmmApi.h"
#include "pmmDebug.h"

#define PMM_TIM_BITS_LIMIT        10
#define PMM_CF_POLLABLE_SCH_LIMIT 100

//                                                                       
//                                     
#define PMM_PSPOLL_PERSISTENCE    2

#if (WNI_POLARIS_FW_PRODUCT==AP)
//                                                                     
/* 
                             
  
                                                                     
  
         
  
               
  
        
  
                                 
                                            
                                       
                                                                                
                                                                                                 
 */

static tANI_BOOLEAN 
__isTimChanged(tpAniSirGlobal pMac, tpPmmTim pNewTim, tpPmmTim pPrevTim)
{
    if ( pNewTim->dtimCount != pPrevTim->dtimCount ||
         pNewTim->maxAssocId != pPrevTim->maxAssocId ||
         pNewTim->minAssocId != pPrevTim->minAssocId ||
         pNewTim->numStaWithData != pPrevTim->numStaWithData)         
        return eANI_BOOLEAN_TRUE;
    
    if (!palEqualMemory(pMac->hHdd, pNewTim->pTim, pPrevTim->pTim, sizeof(*pPrevTim->pTim) * pMac->lim.maxStation))
        return eANI_BOOLEAN_TRUE;
 
    return eANI_BOOLEAN_FALSE;
}

/* 
                                
  
                                             
  
         
  
               
  
        
  
                                 
                                                                    
                                  
               
 */
static /*       */
void    __updatePmmGlobal( tpAniSirGlobal pMac, tpPmmTim pNewTim)
{
    tpPmmTim    pGlobalTim = &pMac->pmm.gPmmTim;
    palCopyMemory(pMac->hHdd, pGlobalTim->pTim,  pNewTim->pTim,
                                                        sizeof(*pNewTim->pTim) * pMac->lim.maxStation);

    pGlobalTim->dtimCount = pNewTim->dtimCount;
    pGlobalTim->maxAssocId = pNewTim->maxAssocId;
    pGlobalTim->minAssocId = pNewTim->minAssocId;
    pGlobalTim->numStaWithData = pNewTim->numStaWithData;    
}


void pmmUpdatePSPollState(tpAniSirGlobal pMac)
{
    tANI_U16 psFlag = 0;
    tANI_U32 sta;

    //                                                                       
    if (pMac->pmm.gPmmPsPollUpdate == 0)
        return;

    /*
                                                                           
                                                                              
                                                                       
                  
                                                                                
                                                                            
     */
    for (sta = 1; sta < pMac->lim.maxStation; sta++)
    {
        tANI_U8 ps = 0;

        if (pMac->pmm.gpPmmStaState[sta].psPollUpdate == 0)
            continue;

        if (pMac->pmm.gpPmmStaState[sta].psPollUpdate == 1)
        {
            pmmUpdatePMMode(pMac, sta, ps);
        }
        pMac->pmm.gpPmmStaState[sta].psPollUpdate--;
        psFlag++;
        PELOG4(pmmLog(pMac, LOG4, FL("UpdatePSPollState: sta %d, state %d (count %d)\n"), sta, ps, pMac->pmm.gpPmmStaState[sta].psPollUpdate););
    }

    if (psFlag == 0)
        pMac->pmm.gPmmPsPollUpdate = 0;
    PELOG3(pmmLog(pMac, LOG3, FL("UpdatePSPollState: %d ps-poll flagged sta's\n"), psFlag);)
}

/* 
                                              
   
                                                      
  
         
                                            
                                                                                      
                         
  
        
                                 
                 
  */
 
void pmmHandleTimBasedDisassociation (tpAniSirGlobal pMac, tpPESession psessionEntry)
{
    tpPmmStaInfo pmmStaInfo = pMac->pmm.gPmmTim.pStaInfo;
    tANI_U32    staCnt;
    tANI_U32    staWithData = pMac->pmm.gPmmTim.numStaWithData;
    

    /*                                           */
    for (staCnt = 0; staCnt < pMac->pmm.gPmmNumSta; staCnt++) {
        tpDphHashNode pSta = dphGetHashEntry(pMac, pmmStaInfo[staCnt].assocId);

        if ((pSta == NULL) ||(!pSta->valid )) {
            pmmLog(pMac, LOGE, FL("Invalid ASSOCID / associd points to notvalid STA assocId [%d] NumSta [%d]\n"),pmmStaInfo[staCnt].assocId, 
                pMac->pmm.gPmmNumSta );
            continue;
        }
        
        if (staCnt >= staWithData) /*                                                       */
            pSta->timWaitCount = GET_TIM_WAIT_COUNT(pSta->mlmStaContext.listenInterval);   /*                        */
        else {  /*                                            */
            if (pSta->curTxMpduCnt == pmmStaInfo[staCnt].staTxAckCnt) {
                /*                                                                    */
                if ( !(--pSta->timWaitCount)) { 
                    PELOGE(pmmLog(pMac, LOGE, FL(" STA with AID %d did not respond to TIM, deleting..."),pmmStaInfo[staCnt].assocId);)
                    limTriggerSTAdeletion(pMac, pSta, psessionEntry);
                    continue;
                }
            }else /*                                            */
                pSta->timWaitCount = GET_TIM_WAIT_COUNT(pSta->mlmStaContext.listenInterval);  /*                        */
        }
        pSta->curTxMpduCnt = pmmStaInfo[staCnt].staTxAckCnt;
    }

    return;
 }

#endif

#ifdef WLAN_SOFTAP_FEATURE
/* 
                 
  
            
               
  
         
  
               
  
        
  
                                              
                                                                           
                                                                      
               
 */
void pmmGenerateTIM(tpAniSirGlobal pMac, tANI_U8 **pPtr, tANI_U16 *timLength, tANI_U8 dtimPeriod)
{
    tANI_U8 *ptr = *pPtr;
    tANI_U32 val = 0;
    tANI_U32 minAid = 1; //                                   
    tANI_U32 maxAid = HAL_NUM_STA;


    //                                
    tANI_U8 N1 = minAid / 8;
    tANI_U8 N2 = maxAid / 8;
    if (N1 & 1) N1--;

    *timLength = N2 - N1 + 4;
    val = dtimPeriod;

    /* 
                                       
                                                                              
                                                                                        
                                                                                          
                                                                                        
     */
    *ptr++ = SIR_MAC_TIM_EID;
    *ptr++ = (tANI_U8)(*timLength);
    *ptr++ = 0xFF; //                                                 
    *ptr++ = (tANI_U8)val;

    *ptr++ = 0xFF; //                                                      
    ptr += (N2 - N1 + 1);

    PELOG2(sirDumpBuf(pMac, SIR_PMM_MODULE_ID, LOG2, *pPtr, (*timLength)+2);)
    *pPtr = ptr;
}

#endif
#ifdef ANI_PRODUCT_TYPE_AP
/* 
               
  
                                                                                  
                                                                  
  
         
  
               
  
        
  
                                              
                                                                                                 
               
 */

void pmmUpdateTIM(tpAniSirGlobal pMac, tpBeaconGenParams pBeaconGenParams)
{
    tANI_U16 i;
    tANI_U16 assocId;
    tPmmTim curTim;
    tPmmTim *pPmmTim = &pMac->pmm.gPmmTim;
    tANI_U16 totalSta;
    tpBeaconGenStaInfo pStaInfo;

    totalSta = pBeaconGenParams->numOfSta + pBeaconGenParams->numOfStaWithoutData;

    if(totalSta > pMac->lim.maxStation)
    {
        PELOGE(pmmLog(pMac, LOGE, FL("Error in TIM Update: Total STA count %d exceeds the MAX Allowed %d \n"), totalSta, pMac->lim.maxStation);)
        return;
    }
    palZeroMemory(pMac->hHdd, &curTim,  sizeof(curTim));
    palZeroMemory(pMac->hHdd, pMac->pmm.gpPmmPSState, 
        sizeof(*pMac->pmm.gpPmmPSState) * pMac->lim.maxStation);

    if( eHAL_STATUS_SUCCESS != palAllocateMemory( pMac->hHdd,
                  (void **) &curTim.pTim, sizeof(tANI_U8)*pMac->lim.maxStation))
    {
        pmmLog( pMac, LOGE, FL( "Unable to PAL allocate memory\n" ));
        return;                 
    }
    palZeroMemory(pMac->hHdd, curTim.pTim, sizeof(tANI_U8) * pMac->lim.maxStation);
    pPmmTim->numStaWithData = curTim.numStaWithData = pBeaconGenParams->numOfSta;
    pMac->pmm.gPmmNumSta = totalSta;
    curTim.minAssocId= pMac->lim.maxStation - 1;
    curTim.maxAssocId= 0;
    curTim.dtimCount = pBeaconGenParams->dtimCount;
    pStaInfo = (tpBeaconGenStaInfo)((tANI_U8 *)pBeaconGenParams + sizeof(*pBeaconGenParams));

    if (totalSta > 0) 
        palCopyMemory(pMac->hHdd, pPmmTim->pStaInfo, pStaInfo, 
                                                                sizeof(*pPmmTim->pStaInfo) * totalSta);
    
    for(i=0; i<totalSta; i++)
    {
        assocId = pStaInfo[i].assocId;      
        if(assocId < pMac->lim.maxStation)
        {
            //                                                           
            if(i<pBeaconGenParams->numOfSta)
            {
                PELOG2(pmmLog(pMac, LOG2, FL("Setting TIM for assocId: %d\n"), assocId);)
                curTim.pTim[assocId] = 1;
                if (assocId < curTim.minAssocId)
                    curTim.minAssocId= assocId;
                if (assocId > curTim.maxAssocId)
                    curTim.maxAssocId= assocId;
            }
            pMac->pmm.gpPmmPSState[assocId] = 1;
                    
        }    
    }

    if (!curTim.maxAssocId)
        curTim.minAssocId = 0;

    if(pBeaconGenParams->fBroadcastTrafficPending)
        curTim.pTim[0] = 1;

    //                                                            
    if (!__isTimChanged(pMac, &curTim, pPmmTim)) {
        palFreeMemory(pMac->hHdd, curTim.pTim);
        return;
    }

    //                         
    __updatePmmGlobal(pMac, &curTim);
    
    pMac->sch.schObject.fBeaconChanged = 1;
    palFreeMemory(pMac->hHdd, curTim.pTim);
    
    return;
}




//                                                                     
/* 
                   
  
            
                              
  
         
  
               
  
        
  
                                                                  
               
 */

void pmmProcessPSPoll(tpAniSirGlobal pMac, tpHalBufDesc pBd)
{

    tpSirMacMgmtHdr mh = SIR_MAC_BD_TO_MPDUHEADER(pBd);


    //                               
    tANI_U16 aid = mh->durationLo + ((mh->durationHi & 0x3f) << 8);

    PELOG1(pmmLog(pMac, LOG1, FL("Received PS Poll AID 0x%x\n"), aid);)
    if (aid == 0 || aid >= pMac->lim.maxStation)
    {
       PELOG1(pmmLog(pMac, LOG1, FL("STA id %d in update PM, should be [1,%d]\n"),
               aid, pMac->lim.maxStation);)
        return;
    }

    tpDphHashNode pSta = dphGetHashEntry(pMac, aid);
    if (pSta && pSta->valid)
        pSta->numTimSent = 0;

    /*
                                                                          
                                                                            
      
                                                                           
                                                                 
                                                                     
     */
    pMac->pmm.gpPmmStaState[aid].psPollUpdate = PMM_PSPOLL_PERSISTENCE;
    pMac->pmm.gPmmPsPollUpdate = 1;
    if (mh->fc.powerMgmt != pMac->pmm.gpPmmStaState[aid].powerSave)
    {
        tANI_U8 ps = 0;
        if (pMac->pmm.gpPmmStaState[aid].powerSave != ps)
        {
           PELOG1(pmmLog(pMac, LOG1, FL("Sta %d: PS Poll processing PS-state %d -> %d\n"),
                   aid, pMac->pmm.gpPmmStaState[aid].powerSave, ps);)
            pmmUpdatePMMode(pMac, aid, ps);
        }
    }

#ifdef PMM_APSD
    /*
                                          
                                        
      
                                                                            
                                                                             
                                                                            
                                                                            
                                                                        
                                  
     */

    {
        tANI_U8 uapsdFlag = (*((tANI_U8*) &pSta->qos.capability.qosInfo)) & 0xF;

        if ((uapsdFlag & 0xf) == 0xf)
        {
            /*
                                                    
             */
            apsdSendQosNull(pMac, pSta->assocId, 0, pSta->staAddr);
        }
    }

#endif

    /*
                                                       
                                    
                                        
     */

    //                                           
}

void pmmProcessRxActivity(tpAniSirGlobal pMac, tANI_U16 staId, tANI_U8 pmMode)
{
    if (staId == 0 || staId >= pMac->lim.maxStation)
    {
       PELOG1(pmmLog(pMac, LOG1, FL("STA id %d in RxActProc, should be [1,%d]\n"),
               staId, pMac->lim.maxStation);)
        return;
    }

    PELOG1(pmmLog(pMac, LOG1, FL("STA id %d;) pmMode %d\n"), staId, pmMode);)

    //                                                               
    if (pMac->pmm.gpPmmStaState[staId].psPollUpdate == 0)
    {
        pmmUpdatePMMode(pMac, staId, pmMode);
    }
}

//                                                                     
/* 
                  
  
            
                            
  
         
  
               
  
        
  
                                     
                                             
               
 */

void pmmUpdatePMMode(tpAniSirGlobal pMac, tANI_U16 staId, tANI_U8 pmMode)
{
    if ((staId == 0) ||
        (staId == 1) ||
        (staId >= pMac->lim.maxStation))
    {
        pmmLog(pMac, LOGE, FL("STA id %d in update PM, should be [2,%d]\n"),
               staId, pMac->lim.maxStation);
        return;
    }

    //                                              
    tpDphHashNode pSta = dphGetHashEntry(pMac, staId);
    if (pSta == NULL || pSta->valid == 0)
        return;

    if (pMac->pmm.gpPmmStaState[staId].powerSave != pmMode)
    {
       PELOG1(pmmLog(pMac, LOG1, FL("STA id %d, PM mode %d\n"),
               staId, pmMode);)
#ifdef WMM_APSD
        pmmLog(pMac, LOGE, FL("pm mode change: STA id %d, PM mode %d\n"),
               staId, pmMode);
#endif

        //                          
        pMac->pmm.gpPmmStaState[staId].powerSave = pmMode;

        //                                                       
        tANI_U16 i = (staId == 2) ? pMac->lim.maxStation-1 : staId-1;
        while (i != staId &&
               !(pMac->pmm.gpPmmStaState[i].powerSave == 1 &&
                 pMac->pmm.gpPmmStaState[i].cfPollable == pMac->pmm.gpPmmStaState[staId].cfPollable)
              )
        {
            i = (i == 2) ? pMac->lim.maxStation-1 : i-1;
        }

        //                                     
        if (pMac->pmm.gpPmmStaState[staId].cfPollable)
            pmmUpdatePollablePMMode(pMac, staId, pmMode, i);
        else
            pmmUpdateNonPollablePMMode(pMac, staId, pmMode, i);

        //                                         
        if (pmMode)
            pMac->pmm.gPmmNumSta++;
        else
        {
            pMac->pmm.gPmmNumSta--;

            //                                                       
            pSta->numTimSent = 0;
        }

        //                                           
        tANI_U8 multicastPS = pMac->pmm.gPmmNumSta ? 1 : 0;
        if (pMac->pmm.gpPmmStaState[0].powerSave != multicastPS)
        {
            pMac->pmm.gpPmmStaState[0].powerSave = multicastPS;
            //                                    
        }
    }
}

//                                                                     
/* 
                             
  
            
                            
  
         
  
               
  
        
  
                                     
                                             
                                                        
               
 */

void pmmUpdateNonPollablePMMode(tpAniSirGlobal pMac, tANI_U16 staId, tANI_U8 pmMode, tANI_U16 i)
{
    if (i == staId)
    {
        //                                         
        pMac->pmm.gpPmmStaState[staId].nextPS = (tANI_U8)staId;
        if (pmMode)
        {
            //                                               
            pMac->pmm.gPmmNextSta = staId;
        }
        else
        {
            //                                              
            pMac->pmm.gPmmNextSta = 0;
        }
    }
    else
    {
        //                                                   
        if (pmMode)
        {
            //                                                            
            pMac->pmm.gpPmmStaState[staId].nextPS = pMac->pmm.gpPmmStaState[i].nextPS;
            pMac->pmm.gpPmmStaState[i].nextPS = (tANI_U8)staId;
        }
        else
        {
            //                                                             
            pMac->pmm.gpPmmStaState[i].nextPS = pMac->pmm.gpPmmStaState[staId].nextPS;

            //                                                        
            if (staId == pMac->pmm.gPmmNextSta)
                pMac->pmm.gPmmNextSta = pMac->pmm.gpPmmStaState[staId].nextPS;
        }
    }
}

//                                                                     
/* 
                          
  
            
                            
  
         
  
               
  
        
  
                                     
                                             
                                                        
               
 */

void pmmUpdatePollablePMMode(tpAniSirGlobal pMac, tANI_U16 staId, tANI_U8 pmMode, tANI_U16 i)
{
    if (i == staId)
    {
        //                                     
        pMac->pmm.gpPmmStaState[staId].nextPS = (tANI_U8)staId;
        if (pmMode)
        {
            //                                           
            pMac->pmm.gPmmNextCFPSta = staId;
        }
        else
        {
            //                                          
            pMac->pmm.gPmmNextCFPSta = 0;
        }
    }
    else
    {
        //                                               
        if (pmMode)
        {
            //                                                        
            pMac->pmm.gpPmmStaState[staId].nextPS = pMac->pmm.gpPmmStaState[i].nextPS;
            pMac->pmm.gpPmmStaState[i].nextPS = (tANI_U8)staId;
        }
        else
        {
            //                                                         
            pMac->pmm.gpPmmStaState[i].nextPS = pMac->pmm.gpPmmStaState[staId].nextPS;

            //                                                        
            if (staId == pMac->pmm.gPmmNextCFPSta)
                pMac->pmm.gPmmNextCFPSta = pMac->pmm.gpPmmStaState[staId].nextPS;
        }
    }
}



#endif //                             

//                                                                     

