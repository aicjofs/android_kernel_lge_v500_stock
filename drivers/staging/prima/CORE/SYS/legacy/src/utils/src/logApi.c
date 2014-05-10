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
#define WNI_PRINT_DEBUG

#include <sirCommon.h>
#include <sirDebug.h>
#include <utilsApi.h>
#if defined(FEATURE_WLAN_NON_INTEGRATED_SOC)
#include <halCommonApi.h>
#endif
#include <wlan_qct_wda.h>

#include <stdarg.h>
#include "sirWrapper.h"
#include "utilsGlobal.h"
#include "macInitApi.h"
#include "palApi.h"

#include "vos_trace.h"

#ifdef ANI_OS_TYPE_ANDROID
#include <linux/kernel.h>
#endif


//                                                     
//                                   
//                     
#ifndef ANI_OS_TYPE_WINDOWS
#ifndef _vsnprintf
#define _vsnprintf vsnprintf
#endif
#endif //            

#define dbgTraceInfo(_Mask, _InParams)                 \
  {                                                      \
    KdPrint (_InParams) ;                                 \
  }

#define utilLogLogDebugMessage(HddAdapter, _LogBuffer)   \
  {                                                      \
    VOS_TRACE(VOS_MODULE_ID_SYS, VOS_TRACE_LEVEL_INFO,   \
              _LogBuffer);                               \
  }
  

//                                                                      
/* 
             
  
            
                                                          
  
         
  
               
        
  
        
  
                                                                    
               
 */
tSirRetStatus 
logInit(tpAniSirGlobal pMac)
{
    tANI_U32    i;

    //                                                   
    //                            
    for (i = 0; i < LOG_ENTRY_NUM; i++)
    {
#ifdef SIR_DEBUG
        pMac->utils.gLogEvtLevel[i] = pMac->utils.gLogDbgLevel[i] = LOG1;
#else
#ifdef LX5280
        pMac->utils.gLogEvtLevel[i] = pMac->utils.gLogDbgLevel[i] = LOGE;
#else
        pMac->utils.gLogEvtLevel[i] = pMac->utils.gLogDbgLevel[i] = LOGW;
#endif
#endif
    }
    return eSIR_SUCCESS;

} /*               */

void
logDeinit(tpAniSirGlobal pMac)
{
    return;
}

/* 
            
  
           
                                                  
  
         
  
        
  
              
        
  
       
  
                                                                    
                                  
                                                       
                                               
               
 */

#if defined(ANI_OS_TYPE_OSX)
#if defined ANI_FIREWIRE_LOG
#include <IOKit/firewire/FireLog.h>
#define printk          FireLog
#else
#define printk          printf
#endif
#define tx_time_get()   (0)
#endif

void logDbg(tpAniSirGlobal pMac, tANI_U8 modId, tANI_U32 debugLevel, const char *pStr,...)
{
#ifdef WLAN_DEBUG
    if ( debugLevel > pMac->utils.gLogDbgLevel[LOG_INDEX_FOR_MODULE( modId )] )
        return;
    else
    {
        va_list marker;

        va_start( marker, pStr );     /*                                */

        logDebug(pMac, modId, debugLevel, pStr, marker);
        
        va_end( marker );              /*                                */
    }      
#endif
}

#ifdef VOSS_ENABLED
static inline VOS_TRACE_LEVEL getVosDebugLevel(tANI_U32 debugLevel)
{   
    switch(debugLevel)
    {
        case LOGP:
            return VOS_TRACE_LEVEL_FATAL;
        case LOGE:
            return VOS_TRACE_LEVEL_ERROR;
        case LOGW:
            return VOS_TRACE_LEVEL_WARN;
        case LOG1:
            return VOS_TRACE_LEVEL_INFO;
        case LOG2:
            return VOS_TRACE_LEVEL_INFO_HIGH;
        case LOG3:
            return VOS_TRACE_LEVEL_INFO_MED;
        case LOG4:
            return VOS_TRACE_LEVEL_INFO_LOW;
        default:
            return VOS_TRACE_LEVEL_INFO_LOW;
    }
}

static inline VOS_MODULE_ID getVosModuleId(tANI_U8 modId)
{
    switch(modId)
    {
        case SIR_HAL_MODULE_ID:
        case SIR_PHY_MODULE_ID:
            return VOS_MODULE_ID_WDA;

        case SIR_LIM_MODULE_ID:
        case SIR_SCH_MODULE_ID:
        case SIR_PMM_MODULE_ID:
        case SIR_CFG_MODULE_ID:
        case SIR_MNT_MODULE_ID:
        case SIR_DPH_MODULE_ID:
        case SIR_DBG_MODULE_ID:
            return VOS_MODULE_ID_PE;

        case SIR_SYS_MODULE_ID:
            return VOS_MODULE_ID_SYS;
    
        case SIR_SMS_MODULE_ID:
            return VOS_MODULE_ID_SME;

        default:
            return VOS_MODULE_ID_SYS;
    }
}
#endif //             

#define LOG_SIZE 256
void logDebug(tpAniSirGlobal pMac, tANI_U8 modId, tANI_U32 debugLevel, const char *pStr, va_list marker)
{
    VOS_TRACE_LEVEL  vosDebugLevel;
    VOS_MODULE_ID    vosModuleId;
    char             logBuffer[LOG_SIZE];

    vosDebugLevel = getVosDebugLevel(debugLevel);
    vosModuleId = getVosModuleId(modId);

#ifdef ANI_OS_TYPE_ANDROID
    vsnprintf(logBuffer, LOG_SIZE - 1, pStr, marker);
#else

#ifdef WINDOWS_DT
    RtlStringCbVPrintfA( &logBuffer[ 0 ], LOG_SIZE - 1, pStr, marker );
#else
    _vsnprintf(logBuffer, LOG_SIZE - 1, (char *)pStr, marker);
#endif

#endif
    VOS_TRACE(vosModuleId, vosDebugLevel, "%s", logBuffer);

    //                               
    VOS_ASSERT( ( debugLevel <= pMac->utils.gLogDbgLevel[LOG_INDEX_FOR_MODULE( modId )] ) && ( LOGP != debugLevel ) );
} /*                    */
