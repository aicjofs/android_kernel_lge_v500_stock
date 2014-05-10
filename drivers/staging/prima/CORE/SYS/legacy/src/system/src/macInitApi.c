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
/*                        */
#ifdef FEATURE_WLAN_NON_INTEGRATED_SOC

/*                                    */
#include "halInternal.h"
#include "halHddApis.h"
#include "halDebug.h"
#include "halMTU.h"
#include "halRxp.h"
#include "halPhyApi.h"

//                        
#include "halCommonApi.h"   //           
#endif
#include "cfgApi.h"         //           
#include "limApi.h"         //           
#include "sirTypes.h"
#include "sysDebug.h"
#include "sysEntryFunc.h"
#include "macInitApi.h"
#if defined(ANI_LOGDUMP)
#include "logDump.h"
#endif //                        

#ifdef TRACE_RECORD
#include "macTrace.h"
#endif

extern tSirRetStatus halDoCfgInit(tpAniSirGlobal pMac);
extern tSirRetStatus halProcessStartEvent(tpAniSirGlobal pMac);




tSirRetStatus macReset(tpAniSirGlobal pMac, tANI_U32 rc);

#ifdef FEATURE_WLAN_INTEGRATED_SOC
tSirRetStatus macPreStart(tHalHandle hHal)
{
   tSirRetStatus status = eSIR_SUCCESS;
   tANI_BOOLEAN memAllocFailed = eANI_BOOLEAN_FALSE;
   tpAniSirGlobal pMac = (tpAniSirGlobal) hHal;
   tANI_U8 i;

   for(i=0; i<MAX_DUMP_TABLE_ENTRY; i++)
   {
      if(palAllocateMemory(pMac->hHdd, ((void *)&pMac->dumpTableEntry[i]), sizeof(tDumpModuleEntry))
          != eHAL_STATUS_SUCCESS)
      {
         memAllocFailed = eANI_BOOLEAN_TRUE;
         break;
      }
      else
      {
         palZeroMemory(pMac->hHdd, pMac->dumpTableEntry[i], sizeof(tSirMbMsg));
      }
   }
   if( memAllocFailed )
   {
      while(i>0)
      {
         i--;
         palFreeMemory(pMac, pMac->dumpTableEntry[i]);
      }
      sysLog(pMac, LOGE, FL("pMac->dumpTableEntry is NULL\n"));
      status = eSIR_FAILURE;
   }

#if defined(ANI_LOGDUMP)
   //                                                   
   logDumpInit(pMac);
#endif //                        

   return status;
}

tSirRetStatus macStart(tHalHandle hHal, void* pHalMacStartParams)
{
   tSirRetStatus status = eSIR_SUCCESS;
   tpAniSirGlobal pMac = (tpAniSirGlobal) hHal;

   if (NULL == pMac)
   {
      VOS_ASSERT(0);
      status = eSIR_FAILURE;
      return status;
   }

   pMac->gDriverType = ((tHalMacStartParameters*)pHalMacStartParams)->driverType;

   sysLog(pMac, LOG2, FL("called\n"));

   do
   {

#if defined(TRACE_RECORD)
      //              
      macTraceInit(pMac);
#endif

      if (!HAL_STATUS_SUCCESS(palAllocateMemory(pMac->hHdd, ((void *)&pMac->pResetMsg), sizeof(tSirMbMsg))))
      {
         sysLog(pMac, LOGE, FL("pMac->pResetMsg is NULL\n"));
         status = eSIR_FAILURE;
         break;
      }
      else
      {
         palZeroMemory(pMac->hHdd, pMac->pResetMsg, sizeof(tSirMbMsg));
      }

      if (pMac->gDriverType != eDRIVER_TYPE_MFG)
      {
         status = peStart(pMac);
      }

   } while(0);
   pMac->sys.abort = false;

   return status;
}

#else /*                             */
tSirRetStatus macStart(tHalHandle hHal, void* pHalMacStartParams)
{
    tANI_U8 i;
    tSirRetStatus status = eSIR_SUCCESS;
    eHalStatus             halStatus;
    tpAniSirGlobal pMac = (tpAniSirGlobal) hHal;
    tANI_BOOLEAN memAllocFailed = eANI_BOOLEAN_FALSE;

    if(NULL == pMac)
    {
        VOS_ASSERT(0);
        status = eSIR_FAILURE;
        return status;
    }

    pMac->gDriverType = ((tHalMacStartParameters *)pHalMacStartParams)->driverType;

    sysLog(pMac, LOG2, FL("called\n"));

    do
    {
        for(i=0; i<MAX_DUMP_TABLE_ENTRY; i++)
        {
            if(palAllocateMemory(pMac->hHdd, ((void **)&pMac->dumpTableEntry[i]), sizeof(tDumpModuleEntry))
                != eHAL_STATUS_SUCCESS)
            {
                memAllocFailed = eANI_BOOLEAN_TRUE;
                break;
            }
            else
            {
                palZeroMemory(pMac->hHdd, pMac->dumpTableEntry[i], sizeof(tSirMbMsg));
            }
        }
        if( memAllocFailed )
        {
            while(i>0)
            {
                i--;
                palFreeMemory(pMac, pMac->dumpTableEntry[i]);
            }
            sysLog(pMac, LOGE, FL("pMac->dumpTableEntry is NULL\n"));
            status = eSIR_FAILURE;
            break;
        }
        else
        {
#if defined(ANI_LOGDUMP)
            logDumpInit(pMac);
#endif //                        
        }

#if defined(TRACE_RECORD)
        //              
        macTraceInit(pMac);
#endif
        if (!HAL_STATUS_SUCCESS(palAllocateMemory(pMac->hHdd, ((void **)&pMac->pResetMsg), sizeof(tSirMbMsg))))
        {
            sysLog(pMac, LOGE, FL("pMac->pResetMsg is NULL\n"));
            status = eSIR_FAILURE;
            break;
        }
        else
        {
            palZeroMemory(pMac->hHdd, pMac->pResetMsg, sizeof(tSirMbMsg));
        }

        halStatus = halStart(hHal, (tHalMacStartParameters*)pHalMacStartParams );

        if ( !HAL_STATUS_SUCCESS(halStatus) )
        {
            sysLog(pMac,LOGE, FL("halStart failed with error code = %d\n"), halStatus);
            status = eSIR_FAILURE;
        }
        else if(pMac->gDriverType != eDRIVER_TYPE_MFG)
        {
            peStart(pMac);
        }

    }while(0);
    pMac->sys.abort = false;

    return status;
}
#endif /*                             */

/*                                                               
           
                                                                                                      
                                                                           
                        
                   
                     
                                                               */

tSirRetStatus macStop(tHalHandle hHal, tHalStopType stopType)
{
    tANI_U8 i;
    tpAniSirGlobal pMac = (tpAniSirGlobal) hHal;
#ifdef FEATURE_WLAN_NON_INTEGRATED_SOC
    halStop(hHal, stopType);
#endif
    peStop(pMac);
    cfgCleanup( pMac );
    //                                                    
    //                                                   
    if(false == pMac->sys.abort)
    {
        palFreeMemory(pMac->hHdd, pMac->pResetMsg);
        pMac->pResetMsg = NULL;
    }
    /*                         */
    for(i=0; i<MAX_DUMP_TABLE_ENTRY; i++)
    {
        palFreeMemory(pMac, pMac->dumpTableEntry[i]);
    }

    return eSIR_SUCCESS;
}

/*                                                               
           
                                                                                             
                                                              
                              
                        
                                           
                     
                                                               */

tSirRetStatus macOpen(tHalHandle *pHalHandle, tHddHandle hHdd, tMacOpenParameters *pMacOpenParms)
{
    tpAniSirGlobal pMac = NULL;

    if(pHalHandle == NULL)
        return eSIR_FAILURE;

    /*
                                                                                         
                                  
                                                                                
                                      
     */

    /*               */
    if (palAllocateMemory(hHdd, ((void **)&pMac), sizeof(tAniSirGlobal)) != eHAL_STATUS_SUCCESS)
        return eSIR_FAILURE;

    /*                               */
    palZeroMemory(hHdd, pMac, sizeof(tAniSirGlobal));

    /*                                       */
    //                                              

#ifndef GEN6_ONWARDS
#ifdef RTL8652
    {
        //                                                                              
        extern void * rtlglue_alloc_data_scratchpad_memory(unsigned int size, char *);
        pMac->hal.pHalDxe = (tpAniHalDxe) rtlglue_alloc_data_scratchpad_memory(sizeof(tAniHalDxe),  "halDxe");
    }
    if(pMac->hal.pHalDxe){
        ;
    }else
#endif
    /*                 */
    if (palAllocateMemory(hHdd, ((void **)&pMac->hal.pHalDxe), sizeof(tAniHalDxe)) != eHAL_STATUS_SUCCESS){
        palFreeMemory(hHdd, pMac);
        return eSIR_FAILURE;
    }
    /*                                 */
    palZeroMemory(hHdd, pMac->hal.pHalDxe, sizeof(tAniHalDxe));
#endif //            

    /*
                                             
                                                                          
                 
     */
    pMac->hHdd      = hHdd;
    pMac->pAdapter  = hHdd; //                        
    *pHalHandle     = (tHalHandle)pMac;

    {
        /*                                             */
        if( eSIR_SUCCESS != logInit(pMac))
           return eSIR_FAILURE;
            
        /*                                                */
        if( eSIR_SUCCESS != cfgInit(pMac) )
            return eSIR_FAILURE;

        sysInitGlobals(pMac);

#ifdef FEATURE_WLAN_NON_INTEGRATED_SOC
        //                                                            
        //                                                            
        //                                                
        if(pMacOpenParms->frameTransRequired) {
            pMac->hal.halMac.frameTransEnabled = 1;
        } else {
            pMac->hal.halMac.frameTransEnabled = 0;
        }
#endif

        //                                                  
#if defined( VOSS_ENABLED )
        tx_voss_wrapper_init(pMac, hHdd);
#endif
    }

#ifdef FEATURE_WLAN_NON_INTEGRATED_SOC
    if (eHAL_STATUS_SUCCESS != halOpen(pMac, pHalHandle, hHdd, pMacOpenParms))
        return eSIR_FAILURE;
#endif

    return peOpen(pMac, pMacOpenParms);
}

/*                                                               
            
                                                                          
                                                                
                            
            
                                                               */

tSirRetStatus macClose(tHalHandle hHal)
{

    tpAniSirGlobal pMac = (tpAniSirGlobal) hHal;

#ifndef GEN6_ONWARDS
    if(pMac->hal.pHalDxe){
#ifdef RTL8652
        extern void * rtlglue_is_data_scratchpad_memory(void *);
        if(rtlglue_is_data_scratchpad_memory(pMac->hal.pHalDxe))
            ;
        else
#endif
            palFreeMemory(pMac, pMac->hal.pHalDxe);
    }
#endif //            

    peClose(pMac);
#ifdef FEATURE_WLAN_NON_INTEGRATED_SOC
    halClose(hHal);
#endif

    /*                                                 */
    cfgDeInit(pMac);

    logDeinit(pMac);

    //                                                   
    palFreeMemory( pMac->hHdd, pMac );

    return eSIR_SUCCESS;
}

/*                                                               
            
                                                                                                   
                            
                    
                         
                                                               */

tSirRetStatus macReset(tpAniSirGlobal pMac, tANI_U32 rc)
{
    tSirRetStatus status = eSIR_SUCCESS;
#ifdef FEATURE_WLAN_NON_INTEGRATED_SOC
    if(eHAL_STATUS_SUCCESS != halReset((tHalHandle)pMac, rc))
          status = eSIR_FAILURE;
#else
    sysLog(pMac, LOGE, FL("*************No-op. Need to call WDA reset function \n"));
#endif
    return status;
}

//                                                                       
/* 
                 
  
            
                                                                
  
         
  
               
  
  
        
  
                                                 
                                    
                                        
 */

void
macSysResetReq(tpAniSirGlobal pMac, tANI_U32 rc)
{
    sysLog(pMac, LOGE, FL("Reason Code = 0x%X\n"),rc);

    switch (rc)
    {
        case eSIR_STOP_BSS:
        case eSIR_SME_BSS_RESTART:
        case eSIR_RADIO_HW_SWITCH_STATUS_IS_OFF:
        case eSIR_CFB_FLAG_STUCK_EXCEPTION:
                //      
                //                   
                break;

        case eSIR_EOF_SOF_EXCEPTION:
        case eSIR_BMU_EXCEPTION:
        case eSIR_CP_EXCEPTION:
        case eSIR_LOW_PDU_EXCEPTION:
        case eSIR_USER_TRIG_RESET:
        case eSIR_AHB_HANG_EXCEPTION:
        default:
             macReset(pMac, rc);
            break;

    }
}

//                                                              
/* 
                        
  
            
                                                                      
  
         
  
               
  
  
        
  
                                                 
                                        
 */

void
macSysResetReqFromHDD(void *pMac, tANI_U32 rc)
{
    macSysResetReq( (tpAniSirGlobal)pMac, rc );
}

