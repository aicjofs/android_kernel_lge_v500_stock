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
  
  \file  vos_api.c

  \brief Stub file for all virtual Operating System Services (vOSS) APIs
               
   Copyright 2008 (c) Qualcomm, Incorporated.  All Rights Reserved.
   
   Qualcomm Confidential and Proprietary.
  
  ========================================================================*/
 /*                                                                            

                                             
   
   
                                                                        
                                                                 
   
   
                                    
   
   
                                      
                                                                             
                                         
                                                                           */

/*                                                                          
               
                                                                          */
#ifdef FEATURE_WLAN_NON_INTEGRATED_SOC
#include "aniGlobal.h"
#include "halTypes.h"
#include "wlan_qct_sal.h"
#include "wlan_qct_bal.h"
#endif
#include <vos_mq.h>
#include "vos_sched.h"
#include <vos_api.h>
#include "sirTypes.h"
#include "sirApi.h"
#include "sirMacProtDef.h"
#include "sme_Api.h"
#include "macInitApi.h"
#include "wlan_qct_sys.h"
#include "wlan_qct_tl.h"
#include "wlan_hdd_misc.h"
#include "i_vos_packet.h"
#include "vos_nvitem.h"
#include "wlan_qct_wda.h"
#include "wlan_hdd_main.h"
#include <linux/vmalloc.h>


#ifdef WLAN_SOFTAP_FEATURE
#include "sapApi.h"
#endif



#ifdef WLAN_BTAMP_FEATURE
#include "bapApi.h"
#include "bapInternal.h"
#include "bap_hdd_main.h"
#endif //                  


/*                                                                           
                                         
                                                                           */
/*                                                                    
                                                                       
                                                                         
                                  */
#define VOS_WDA_TIMEOUT 15000

/*                                                        */
#define VOS_WDA_STOP_TIMEOUT WDA_STOP_TIMEOUT 

/*                                                                           
                   
                                                                           */
static VosContextType  gVosContext;
static pVosContextType gpVosContext;

/*                                                                           
                      
                                                                           */
v_VOID_t vos_sys_probe_thread_cback ( v_VOID_t *pUserData );

#ifndef FEATURE_WLAN_INTEGRATED_SOC
v_VOID_t vos_sys_start_complete_cback  ( v_VOID_t *pUserData );
#endif
v_VOID_t vos_core_return_msg(v_PVOID_t pVContext, pVosMsgWrapper pMsgWrapper);

v_VOID_t vos_fetch_tl_cfg_parms ( WLANTL_ConfigInfoType *pTLConfig, 
    hdd_config_t * pConfig );
#ifndef FEATURE_WLAN_INTEGRATED_SOC
VOS_STATUS vos_get_fwbinary( v_VOID_t **ppBinary, v_SIZE_t *pNumBytes );
#endif


/*                                                                           
  
                                                  
    
                                                                           
                                                                    
                
                                                                   
                                                                
                       
  
                                                                   
 
  
                                                                          
                              
              
                                                                        
          
                
  
                                                                           */
VOS_STATUS vos_preOpen ( v_CONTEXT_t *pVosContext )
{
   if ( pVosContext == NULL)
      return VOS_STATUS_E_FAILURE;

   /*                          */
   *pVosContext = NULL;
   gpVosContext = &gVosContext;

   if (NULL == gpVosContext)
   {
     /*                                          */
      VOS_TRACE( VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_FATAL,
                 "%s: Failed to allocate VOS Context", __func__);
      VOS_ASSERT(0);
      return VOS_STATUS_E_RESOURCES;
   }

   vos_mem_zero(gpVosContext, sizeof(VosContextType));

   *pVosContext = gpVosContext;

   return VOS_STATUS_SUCCESS;

} /*              */

  
/*                                                                           
  
                                                    
    
                                                       
  
                                                                     
 
  
                                                
                  
          
                    
                 
                                                                           */
VOS_STATUS vos_preClose( v_CONTEXT_t *pVosContext )
{

   VOS_TRACE( VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_INFO,
                "%s: De-allocating the VOS Context", __func__);

   if (( pVosContext == NULL) || (*pVosContext == NULL)) 
   {
      VOS_TRACE( VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR,
                "%s: vOS Context is Null", __func__);
      return VOS_STATUS_E_FAILURE;
   }

   if (gpVosContext != *pVosContext)
   {
      VOS_TRACE( VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR,
                "%s: Context mismatch", __func__);
      return VOS_STATUS_E_FAILURE;
   }

   *pVosContext = gpVosContext = NULL;

   return VOS_STATUS_SUCCESS;

} /*               */

/*                                                                           
  
                                            
    
                                                     
                                 
  
                                                      
     
                                           
     
                                                                        
                                 
      
  
                                                              
 
  
                                                                          
                              
  
                                                                        
                                                    

          
                                                                        
          
                   
  
                                                                           */
VOS_STATUS vos_open( v_CONTEXT_t *pVosContext, v_SIZE_t hddContextSize )

{
   VOS_STATUS vStatus      = VOS_STATUS_SUCCESS;
   int iter                = 0;
   tSirRetStatus sirStatus = eSIR_SUCCESS;
   tMacOpenParameters macOpenParms;
   WLANTL_ConfigInfoType TLConfig;

   VOS_TRACE( VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_INFO_HIGH,
               "%s: Opening VOSS", __func__);

   if (NULL == gpVosContext)
   {
      VOS_TRACE( VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_FATAL,
                    "%s: Trying to open VOSS without a PreOpen", __func__);
      VOS_ASSERT(0);
      return VOS_STATUS_E_FAILURE;
   }

   /*                             */
   vos_timer_module_init();

   /*                            */
   if (vos_event_init(&gpVosContext->ProbeEvent) != VOS_STATUS_SUCCESS)
   {
      VOS_TRACE( VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_FATAL,
                    "%s: Unable to init probeEvent", __func__);
      VOS_ASSERT(0);
      return VOS_STATUS_E_FAILURE;
   }
#ifdef FEATURE_WLAN_INTEGRATED_SOC
   if (vos_event_init( &(gpVosContext->wdaCompleteEvent) ) != VOS_STATUS_SUCCESS )
   {
      VOS_TRACE( VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_FATAL,
                  "%s: Unable to init wdaCompleteEvent", __func__);
      VOS_ASSERT(0);
    
      goto err_probe_event;
   }

   /*                        */
   /*                                                           
                                              */
 
   VOS_TRACE( VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_INFO,
               "%s: HDD context is saved successfully", __func__);
   
#endif

   /*                                   */
   vStatus = vos_mq_init(&gpVosContext->freeVosMq);
   if (! VOS_IS_STATUS_SUCCESS(vStatus))
   {

      /*                                            */
      VOS_TRACE( VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_FATAL,
                "%s: Failed to initialize VOS free message queue", __func__);
      VOS_ASSERT(0);
#ifndef FEATURE_WLAN_INTEGRATED_SOC
      goto err_probe_event;
#else
      goto err_wda_complete_event;
#endif
   }

   for (iter = 0; iter < VOS_CORE_MAX_MESSAGES; iter++)
   {
      (gpVosContext->aMsgWrappers[iter]).pVosMsg = 
         &(gpVosContext->aMsgBuffers[iter]); 
      INIT_LIST_HEAD(&gpVosContext->aMsgWrappers[iter].msgNode);
      vos_mq_put(&gpVosContext->freeVosMq, &(gpVosContext->aMsgWrappers[iter]));
   }
#ifndef FEATURE_WLAN_INTEGRATED_SOC
   /*                                           */
   vStatus = vos_packet_open( gpVosContext, &gpVosContext->vosPacket,
                              sizeof( vos_pkt_context_t ) );

   if ( !VOS_IS_STATUS_SUCCESS( vStatus ) )
   {
      /*                                            */
      VOS_TRACE( VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_FATAL,
                "%s: Failed to open VOS Packet Module", __func__);
      VOS_ASSERT(0);
      goto err_msg_queue;
   }
#endif   

   /*                            */
   vStatus= vos_sched_open(gpVosContext, &gpVosContext->vosSched,
                           sizeof(VosSchedContext));

   if (!VOS_IS_STATUS_SUCCESS(vStatus))
   {
      /*                                            */
      VOS_TRACE( VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_FATAL,
                "%s: Failed to open VOS Scheduler", __func__);
      VOS_ASSERT(0);
#ifndef FEATURE_WLAN_INTEGRATED_SOC
      goto err_packet_close;
#else
      goto err_msg_queue;
#endif
   }

#ifdef FEATURE_WLAN_INTEGRATED_SOC
   /*
                                                                            
                                                                            
   */

   /*                    */
   vos_mem_set(&macOpenParms, sizeof(macOpenParms), 0);
   /*                                                
                                        
   */
   macOpenParms.frameTransRequired = 1;
   macOpenParms.driverType         = eDRIVER_TYPE_PRODUCTION;
   vStatus = WDA_open( gpVosContext, gpVosContext->pHDDContext, &macOpenParms );

   if (!VOS_IS_STATUS_SUCCESS(vStatus))
   {
      /*                                            */
      VOS_TRACE( VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_FATAL,
                "%s: Failed to open WDA module", __func__);
      VOS_ASSERT(0);
      goto err_sched_close;
   }

   /*                                           */
   vStatus = vos_packet_open( gpVosContext, &gpVosContext->vosPacket,
                              sizeof( vos_pkt_context_t ) );

   if ( !VOS_IS_STATUS_SUCCESS( vStatus ) )
   {
      /*                                            */
      VOS_TRACE( VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_FATAL,
                "%s: Failed to open VOS Packet Module", __func__);
      VOS_ASSERT(0);
      goto err_wda_close;
   }
#endif

   /*                     */
   vStatus = sysOpen(gpVosContext);

   if (!VOS_IS_STATUS_SUCCESS(vStatus))
   {
      /*                                            */
      VOS_TRACE( VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_FATAL,
                "%s: Failed to open SYS module", __func__);
      VOS_ASSERT(0);
#ifndef FEATURE_WLAN_INTEGRATED_SOC
      goto err_sched_close;
#else
      goto err_packet_close;
#endif
   }


   /*                          */
   vStatus = vos_nv_open();
   if (!VOS_IS_STATUS_SUCCESS(vStatus))
   {
     //                                
     VOS_TRACE( VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_FATAL,
                "%s: Failed to initialize the NV module", __func__);
     goto err_sys_close;
   }
#ifdef FEATURE_WLAN_NON_INTEGRATED_SOC
   /*                     */
   sysMcThreadProbe(gpVosContext, 
                    &vos_sys_probe_thread_cback,
                    gpVosContext);

   if (vos_wait_single_event(&gpVosContext->ProbeEvent, 0)!= VOS_STATUS_SUCCESS)
   {
      VOS_TRACE( VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_FATAL,
                "%s: Failed to probe MC Thread", __func__);
      VOS_ASSERT(0);
      goto err_nv_close;
   }

   /*                        */
   sysTxThreadProbe(gpVosContext, 
                    &vos_sys_probe_thread_cback,
                    gpVosContext);
   
   if (vos_wait_single_event(&gpVosContext->ProbeEvent, 0)!= VOS_STATUS_SUCCESS)
   {
      VOS_TRACE( VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_FATAL,
                "%s: Failed to probe TX Thread", __func__);
      VOS_ASSERT(0);
      goto err_nv_close;
   }
#endif

   /*                                                                */
   
   /*                             */

   /*                                                
                                         */
   macOpenParms.frameTransRequired = 1;
   sirStatus = macOpen(&(gpVosContext->pMACContext), gpVosContext->pHDDContext,
                         &macOpenParms);
   
   if (eSIR_SUCCESS != sirStatus)
   {
     /*                                            */
     VOS_TRACE( VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_FATAL,
               "%s: Failed to open MAC", __func__);
     VOS_ASSERT(0);
     goto err_nv_close;
   }

   /*                             */
   vStatus = sme_Open(gpVosContext->pMACContext);
   if (!VOS_IS_STATUS_SUCCESS(vStatus))
   {
     /*                                            */
     VOS_TRACE( VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_FATAL,
               "%s: Failed to open SME", __func__);
     VOS_ASSERT(0);
     goto err_mac_close;
   }

   /*                                              */
   vos_fetch_tl_cfg_parms ( &TLConfig, 
       ((hdd_context_t*)(gpVosContext->pHDDContext))->cfg_ini);

   vStatus = WLANTL_Open(gpVosContext, &TLConfig);
   if (!VOS_IS_STATUS_SUCCESS(vStatus))
   {
     /*                                            */
     VOS_TRACE( VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_FATAL,
               "%s: Failed to open TL", __func__);
     VOS_ASSERT(0);
     goto err_sme_close;
   }

   VOS_TRACE( VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_INFO_HIGH,
               "%s: VOSS successfully Opened", __func__);

   *pVosContext = gpVosContext;

   return VOS_STATUS_SUCCESS;


err_sme_close:
   sme_Close(gpVosContext->pMACContext);

err_mac_close:
   macClose(gpVosContext->pMACContext);

err_nv_close:
   vos_nv_close();
   
err_sys_close:   
   sysClose(gpVosContext);

#ifdef FEATURE_WLAN_INTEGRATED_SOC
err_packet_close:
   vos_packet_close( gpVosContext );

err_wda_close:
   WDA_close(gpVosContext);
#endif

err_sched_close:   
   vos_sched_close(gpVosContext);

#ifndef FEATURE_WLAN_INTEGRATED_SOC
err_packet_close:
   vos_packet_close( gpVosContext );
#endif

err_msg_queue:
   vos_mq_deinit(&gpVosContext->freeVosMq);

#ifdef FEATURE_WLAN_INTEGRATED_SOC
err_wda_complete_event:
   vos_event_destroy( &gpVosContext->wdaCompleteEvent );
#endif

err_probe_event:
   vos_event_destroy(&gpVosContext->ProbeEvent);

   return VOS_STATUS_E_FAILURE;

} /*            */

#ifdef FEATURE_WLAN_INTEGRATED_SOC
/*                                                                           

                         

                                                 
            
                

                                      


                                      


                                                                         
                              

                                                                       
                                                    


                                                                     

               

                                                                           */
VOS_STATUS vos_preStart( v_CONTEXT_t vosContext )
{
   VOS_STATUS vStatus          = VOS_STATUS_SUCCESS;
   pVosContextType pVosContext = (pVosContextType)vosContext;
   
   VOS_TRACE(VOS_MODULE_ID_SYS, VOS_TRACE_LEVEL_INFO,
             "vos prestart");

   VOS_ASSERT(gpVosContext == pVosContext);

   VOS_ASSERT( NULL != pVosContext->pMACContext);

   VOS_ASSERT( NULL != pVosContext->pWDAContext);

   /*                  */
   vStatus = macPreStart(gpVosContext->pMACContext);
   if ( !VOS_IS_STATUS_SUCCESS(vStatus) )
   {
      VOS_TRACE(VOS_MODULE_ID_SYS, VOS_TRACE_LEVEL_FATAL,
             "Failed at macPreStart ");
      return VOS_STATUS_E_FAILURE;
   }

   /*               */
   ccmStart(gpVosContext->pMACContext);

   /*                      */
   vos_event_reset(&gpVosContext->wdaCompleteEvent);   
    

   /*                  */
   vStatus = WDA_preStart(gpVosContext);
   if (!VOS_IS_STATUS_SUCCESS(vStatus))
   {
      VOS_TRACE(VOS_MODULE_ID_SYS, VOS_TRACE_LEVEL_FATAL,
             "Failed to WDA prestart");
      macStop(gpVosContext->pMACContext, HAL_STOP_TYPE_SYS_DEEP_SLEEP);
      ccmStop(gpVosContext->pMACContext);
      VOS_ASSERT(0);
      return VOS_STATUS_E_FAILURE;
   }

   /*                                     */
   vStatus = vos_wait_single_event( &gpVosContext->wdaCompleteEvent,
                                    VOS_WDA_TIMEOUT );
   if ( vStatus != VOS_STATUS_SUCCESS )
   {
      if ( vStatus == VOS_STATUS_E_TIMEOUT )
      {
         VOS_TRACE( VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR,
          "%s: Timeout occurred before WDA complete\n", __func__);
      }
      else
      {
         VOS_TRACE( VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR,
           "%s: WDA_preStart reporting other error", __func__);
      }
      macStop(gpVosContext->pMACContext, HAL_STOP_TYPE_SYS_DEEP_SLEEP);
      ccmStop(gpVosContext->pMACContext);
      VOS_ASSERT( 0 );
      return VOS_STATUS_E_FAILURE;
   }

   return VOS_STATUS_SUCCESS;
}
#endif

/*                                                                           
  
                                                  
    
                                                                       
            
                                         
      
                            
      
           
      
          
      
                                      
  
  
                                      
 
  
                                                                          
                              
  
                                                                        
                                                    

          
                                                                        
          
                    
                
  
                                                                           */
VOS_STATUS vos_start( v_CONTEXT_t vosContext )
{
  VOS_STATUS vStatus          = VOS_STATUS_SUCCESS;
  tSirRetStatus sirStatus     = eSIR_SUCCESS;
  pVosContextType pVosContext = (pVosContextType)vosContext;
  tHalMacStartParameters halStartParams;
#ifndef FEATURE_WLAN_INTEGRATED_SOC
  v_VOID_t *pFwBinary = NULL;
  v_SIZE_t  numFwBytes = 0;
#endif

  VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_INFO,
            "%s: Starting Libra SW", __func__);

  /*                                         */
  if (gpVosContext != pVosContext)
  {
     VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR,
           "%s: mismatch in context", __FUNCTION__);
     return VOS_STATUS_E_FAILURE;
  }

#ifndef FEATURE_WLAN_INTEGRATED_SOC
  if (( pVosContext->pBALContext == NULL) || ( pVosContext->pMACContext == NULL)
     || ( pVosContext->pTLContext == NULL))
  {
     if (pVosContext->pBALContext == NULL)
        VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR, 
            "%s: BAL NULL context", __FUNCTION__);
     else if (pVosContext->pMACContext == NULL)
        VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR, 
            "%s: MAC NULL context", __FUNCTION__);
     else
        VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR, 
            "%s: TL NULL context", __FUNCTION__);
     
     return VOS_STATUS_E_FAILURE;
  }
#else
  if (( pVosContext->pWDAContext == NULL) || ( pVosContext->pMACContext == NULL)
     || ( pVosContext->pTLContext == NULL))
  {
     if (pVosContext->pWDAContext == NULL)
        VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR, 
            "%s: WDA NULL context", __FUNCTION__);
     else if (pVosContext->pMACContext == NULL)
        VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR, 
            "%s: MAC NULL context", __FUNCTION__);
     else
        VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR, 
            "%s: TL NULL context", __FUNCTION__);
     
     return VOS_STATUS_E_FAILURE;
  }

  /*                                                              
                                                                         */

  /*                             */

  vos_event_reset( &(gpVosContext->wdaCompleteEvent) );

  vStatus = WDA_NVDownload_Start(pVosContext);

  if ( vStatus != VOS_STATUS_SUCCESS )
  {
     VOS_TRACE( VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR,
                 "%s: Failed to start NV Download", __func__);
     return VOS_STATUS_E_FAILURE;
  }

  vStatus = vos_wait_single_event( &(gpVosContext->wdaCompleteEvent),
                                   VOS_WDA_TIMEOUT );

  if ( vStatus != VOS_STATUS_SUCCESS )
  {
     if ( vStatus == VOS_STATUS_E_TIMEOUT )
     {
        VOS_TRACE( VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR,
         "%s: Timeout occurred before WDA_NVDownload_start complete", __func__);
     }
     else
     {
        VOS_TRACE( VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR,
         "%s: WDA_NVDownload_start reporting other error", __func__);
     }
     VOS_ASSERT(0);
     goto err_wda_stop;   
  }

  VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_INFO,
            "%s: WDA_NVDownload_start correctly started", __func__);

  /*               */
  vStatus = WDA_start(pVosContext);
  if ( vStatus != VOS_STATUS_SUCCESS )
  {
     VOS_TRACE( VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR,
                 "%s: Failed to start WDA", __func__);
     return VOS_STATUS_E_FAILURE;
  }
  VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_INFO,
            "%s: WDA correctly started", __func__);

#endif
  /*               */
  vos_mem_zero((v_PVOID_t)&halStartParams, sizeof(tHalMacStartParameters));

#ifndef FEATURE_WLAN_INTEGRATED_SOC
  /*                                                                      
                               */
  vStatus = vos_get_fwbinary(&pFwBinary, &numFwBytes);
  
  if ( !VOS_IS_STATUS_SUCCESS( vStatus ) )
  {
    VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR,
             "%s: Failed to get firmware binary", __func__);
    return VOS_STATUS_E_FAILURE;
  }

  VOS_TRACE( VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_INFO,
             "%s: Firmware binary file found", __func__);
   

  //                                                                      
  //                                                                          
  //                                                  

  //                                                        
  //                                                           
  //                                                      
  //                                                   
  //                                                           
  //                                                                       
  halStartParams.FW.cbImage = numFwBytes;
  halStartParams.FW.pImage = pFwBinary;

 //                                                  
 halStartParams.driverType = eDRIVER_TYPE_PRODUCTION;
#endif
  /*               */
  sirStatus = macStart(pVosContext->pMACContext,(v_PVOID_t)&halStartParams);

#ifndef FEATURE_WLAN_INTEGRATED_SOC
  hdd_release_firmware(WLAN_FW_FILE, pVosContext->pHDDContext);

  halStartParams.FW.pImage = NULL;
  halStartParams.FW.cbImage = 0;
#endif 
  if (eSIR_SUCCESS != sirStatus)
  {
    VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_FATAL,
              "%s: Failed to start MAC", __func__);
#ifndef FEATURE_WLAN_INTEGRATED_SOC
    return VOS_STATUS_E_FAILURE;
#else
    goto err_wda_stop;
#endif
  }
   
  VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_INFO,
            "%s: MAC correctly started", __func__);

  /*           */
  vStatus = sme_Start(pVosContext->pMACContext);

  if (!VOS_IS_STATUS_SUCCESS(vStatus))
  {
    VOS_TRACE( VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_FATAL,
               "%s: Failed to start SME", __func__);
    goto err_mac_stop;
  }

  VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_INFO,
            "%s: SME correctly started", __func__);

  /*           */
  vStatus = WLANTL_Start(pVosContext);
  if (!VOS_IS_STATUS_SUCCESS(vStatus))
  {
    VOS_TRACE( VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_FATAL,
               "%s: Failed to start TL", __func__);
    goto err_sme_stop;
  }

  VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_INFO,
            "TL correctly started");
#ifndef FEATURE_WLAN_INTEGRATED_SOC  
  /*                                               */
  sysMcStart(pVosContext, vos_sys_start_complete_cback, pVosContext);

  if (vos_wait_single_event(&gpVosContext->ProbeEvent, 0)!= VOS_STATUS_SUCCESS)
  {
     VOS_TRACE( VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_FATAL,
               "%s: Failed to start SYS module", __func__);
     goto err_tl_stop;
  }


   /* 
                                                                                     
                                                                                
                                                                                   
               
                                                                                          
                                                                                              
                                              
                                                    
                                                                                               
   */
  {
   sFreqFor1p3VSupply freq;
   vStatus = vos_nv_read( NV_TABLE_FREQUENCY_FOR_1_3V_SUPPLY, &freq, NULL,
         sizeof(freq) );
   if (VOS_STATUS_SUCCESS != vStatus)
    freq.freqFor1p3VSupply = VOS_NV_FREQUENCY_FOR_1_3V_SUPPLY_3P2MH;

    if (vos_chipVoteFreqFor1p3VSupply(NULL, NULL, NULL, freq.freqFor1p3VSupply) != VOS_STATUS_SUCCESS)
        VOS_TRACE( VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR,
                   "%s: Failed to set the freq %d for 1.3V Supply",
                   __func__, freq.freqFor1p3VSupply );
  }

#endif
  VOS_TRACE( VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_INFO,
            "%s: VOSS Start is successful!!", __func__);

  return VOS_STATUS_SUCCESS;

#ifndef FEATURE_WLAN_INTEGRATED_SOC
err_tl_stop:
  WLANTL_Stop(pVosContext);
#endif

err_sme_stop:
  sme_Stop(pVosContext->pMACContext, TRUE);
    
err_mac_stop:
  macStop( pVosContext->pMACContext, HAL_STOP_TYPE_SYS_RESET );

#ifdef FEATURE_WLAN_INTEGRATED_SOC
err_wda_stop:   
  vos_event_reset( &(gpVosContext->wdaCompleteEvent) );
  WDA_stop( pVosContext, HAL_STOP_TYPE_RF_KILL);
  vStatus = vos_wait_single_event( &(gpVosContext->wdaCompleteEvent),
                                   VOS_WDA_TIMEOUT );
  if( vStatus != VOS_STATUS_SUCCESS )
  {
     if( vStatus == VOS_STATUS_E_TIMEOUT )
     {
        VOS_TRACE( VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_FATAL,
         "%s: Timeout occurred before WDA_stop complete", __func__);

     }
     else
     {
        VOS_TRACE( VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_FATAL,
         "%s: WDA_stop reporting other error", __func__);
     }
     VOS_ASSERT( 0 );
  }
#endif

  return VOS_STATUS_E_FAILURE;
   
} /*             */


/*                   */
VOS_STATUS vos_stop( v_CONTEXT_t vosContext )
{
  VOS_STATUS vosStatus;

#ifdef FEATURE_WLAN_INTEGRATED_SOC
  /*                                                                  
                                                                               */
  vos_event_reset( &(gpVosContext->wdaCompleteEvent) );

  vosStatus = WDA_stop( vosContext, HAL_STOP_TYPE_RF_KILL );

  if (!VOS_IS_STATUS_SUCCESS(vosStatus))
  {
     VOS_TRACE( VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR,
         "%s: Failed to stop WDA", __func__);
     VOS_ASSERT( VOS_IS_STATUS_SUCCESS( vosStatus ) );
  }

  vosStatus = vos_wait_single_event( &(gpVosContext->wdaCompleteEvent),
                                     VOS_WDA_STOP_TIMEOUT );
   
  if ( vosStatus != VOS_STATUS_SUCCESS )
  {
     if ( vosStatus == VOS_STATUS_E_TIMEOUT )
     {
        VOS_TRACE( VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR,
         "%s: Timeout occurred before WDA complete", __func__);
     }
     else
     {
        VOS_TRACE( VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR,
         "%s: WDA_stop reporting other error", __func__ );
     }
     WDA_stopFailed(vosContext);
  }
#endif

  /*                                */
  vosStatus = sysStop( vosContext);
  if (!VOS_IS_STATUS_SUCCESS(vosStatus))
  {
     VOS_TRACE( VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR,
         "%s: Failed to stop SYS", __func__);
     VOS_ASSERT( VOS_IS_STATUS_SUCCESS( vosStatus ) );
  }

  vosStatus = WLANTL_Stop( vosContext );
  if (!VOS_IS_STATUS_SUCCESS(vosStatus))
  {
     VOS_TRACE( VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR,
         "%s: Failed to stop TL", __func__);
     VOS_ASSERT( VOS_IS_STATUS_SUCCESS( vosStatus ) );
  }

#ifndef FEATURE_WLAN_INTEGRATED_SOC
   /* 
                                                                                  
                                                                                
                                                                                   
               
                                                                                               
   */
   if (vos_chipVoteFreqFor1p3VSupply(NULL, NULL, NULL, VOS_NV_FREQUENCY_FOR_1_3V_SUPPLY_1P6MH) != VOS_STATUS_SUCCESS)
        VOS_TRACE( VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR,
               "%s: Failed to set the freq to 1.6Mhz for 1.3V Supply", __func__ );
#endif

  return VOS_STATUS_SUCCESS;
}


/*                    */
VOS_STATUS vos_close( v_CONTEXT_t vosContext )
{
  VOS_STATUS vosStatus;

#ifdef WLAN_BTAMP_FEATURE
  vosStatus = WLANBAP_Close(vosContext);
  if (!VOS_IS_STATUS_SUCCESS(vosStatus))
  {
     VOS_TRACE( VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR,
         "%s: Failed to close BAP", __func__);
     VOS_ASSERT( VOS_IS_STATUS_SUCCESS( vosStatus ) );
  }
#endif //                   


  vosStatus = WLANTL_Close(vosContext);
  if (!VOS_IS_STATUS_SUCCESS(vosStatus))
  {
     VOS_TRACE( VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR,
         "%s: Failed to close TL", __func__);
     VOS_ASSERT( VOS_IS_STATUS_SUCCESS( vosStatus ) );
  }
   
  vosStatus = sme_Close( ((pVosContextType)vosContext)->pMACContext);
  if (!VOS_IS_STATUS_SUCCESS(vosStatus))
  {
     VOS_TRACE( VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR,
         "%s: Failed to close SME", __func__);
     VOS_ASSERT( VOS_IS_STATUS_SUCCESS( vosStatus ) );
  }

  vosStatus = macClose( ((pVosContextType)vosContext)->pMACContext);
  if (!VOS_IS_STATUS_SUCCESS(vosStatus))
  {
     VOS_TRACE( VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR,
         "%s: Failed to close MAC", __func__);
     VOS_ASSERT( VOS_IS_STATUS_SUCCESS( vosStatus ) );
  }

  ((pVosContextType)vosContext)->pMACContext = NULL;

  vosStatus = vos_nv_close();
  if (!VOS_IS_STATUS_SUCCESS(vosStatus))
  {
     VOS_TRACE( VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR,
         "%s: Failed to close NV", __func__);
     VOS_ASSERT( VOS_IS_STATUS_SUCCESS( vosStatus ) );
  }


  vosStatus = sysClose( vosContext );
  if (!VOS_IS_STATUS_SUCCESS(vosStatus))
  {
     VOS_TRACE( VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR,
         "%s: Failed to close SYS", __func__);
     VOS_ASSERT( VOS_IS_STATUS_SUCCESS( vosStatus ) );
  }

#ifdef FEATURE_WLAN_INTEGRATED_SOC
  if ( TRUE == WDA_needShutdown(vosContext ))
  {
     /*                                                          */
     vosStatus = WDA_shutdown( vosContext, VOS_TRUE );
     if (VOS_IS_STATUS_SUCCESS( vosStatus ) )
     {
        hdd_set_ssr_required( VOS_TRUE );
     }
     else
     {
        VOS_TRACE( VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_FATAL,
                               "%s: Failed to shutdown WDA", __func__ );
        VOS_ASSERT( VOS_IS_STATUS_SUCCESS( vosStatus ) );
     }
  } 
  else 
  {
     vosStatus = WDA_close( vosContext );
     if (!VOS_IS_STATUS_SUCCESS(vosStatus))
     {
        VOS_TRACE( VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR,
            "%s: Failed to close WDA", __func__);
        VOS_ASSERT( VOS_IS_STATUS_SUCCESS( vosStatus ) );
     }
  }
  
  /*                                                             */
  vosStatus = vos_packet_close( vosContext );
  if (!VOS_IS_STATUS_SUCCESS(vosStatus))
  {
     VOS_TRACE( VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR,
         "%s: Failed to close VOSS Packet", __func__);
     VOS_ASSERT( VOS_IS_STATUS_SUCCESS( vosStatus ) );
  }
#endif

#ifndef FEATURE_WLAN_INTEGRATED_SOC
  vosStatus = vos_packet_close( vosContext );
  if (!VOS_IS_STATUS_SUCCESS(vosStatus))
  {
     VOS_TRACE( VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR,
         "%s: Failed to close VOSS Packet", __func__);
     VOS_ASSERT( VOS_IS_STATUS_SUCCESS( vosStatus ) );
  }
#endif

  vos_mq_deinit(&((pVosContextType)vosContext)->freeVosMq);

#ifdef FEATURE_WLAN_INTEGRATED_SOC
  vosStatus = vos_event_destroy(&gpVosContext->wdaCompleteEvent);
  if (!VOS_IS_STATUS_SUCCESS(vosStatus))
  {
     VOS_TRACE( VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR,
         "%s: failed to destroy wdaCompleteEvent", __func__);
     VOS_ASSERT( VOS_IS_STATUS_SUCCESS( vosStatus ) );
  }
#endif

  vosStatus = vos_event_destroy(&gpVosContext->ProbeEvent);
  if (!VOS_IS_STATUS_SUCCESS(vosStatus))
  {
     VOS_TRACE( VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR,
         "%s: failed to destroy ProbeEvent", __func__);
     VOS_ASSERT( VOS_IS_STATUS_SUCCESS( vosStatus ) );
  }

  return VOS_STATUS_SUCCESS;
}
                  

/*                                                                            
  
                                                  
  
                                                                       
                                                                         
                                                             

                                                
  
                                                                            
                      
                                             
  
                                                                       
                      
              
                                                                            */
v_VOID_t* vos_get_context( VOS_MODULE_ID moduleId, 
                           v_CONTEXT_t pVosContext )
{
  v_PVOID_t pModContext = NULL;

  if (pVosContext == NULL)
  {
    VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR,
        "%s: vos context pointer is null", __FUNCTION__);
    return NULL;
  }

  if (gpVosContext != pVosContext)
  {
    VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR, 
        "%s: pVosContext != gpVosContext", __FUNCTION__);
    return NULL;
  }

  switch(moduleId)
  {
    case VOS_MODULE_ID_TL:  
    {
      pModContext = gpVosContext->pTLContext;
      break;
    }

#ifndef FEATURE_WLAN_INTEGRATED_SOC
    case VOS_MODULE_ID_BAL:
    {
      pModContext = gpVosContext->pBALContext;
      break;
    }   

    case VOS_MODULE_ID_SAL:
    {
      pModContext = gpVosContext->pSALContext;
      break;
    }   

    case VOS_MODULE_ID_SSC:   
    {
      pModContext = gpVosContext->pSSCContext;
      break;
    }
#endif
#ifdef WLAN_BTAMP_FEATURE
    case VOS_MODULE_ID_BAP:
    {
        pModContext = gpVosContext->pBAPContext;
        break;
    }    
#endif //                  

#ifdef WLAN_SOFTAP_FEATURE
    case VOS_MODULE_ID_SAP:
    {
      pModContext = gpVosContext->pSAPContext;
      break;
    }

    case VOS_MODULE_ID_HDD_SOFTAP:
    {
      pModContext = gpVosContext->pHDDSoftAPContext;
      break;
    }
#endif

    case VOS_MODULE_ID_HDD:
    {
      pModContext = gpVosContext->pHDDContext;
      break;
    }

    case VOS_MODULE_ID_SME:
#ifndef FEATURE_WLAN_INTEGRATED_SOC
    case VOS_MODULE_ID_HAL:
#endif
    case VOS_MODULE_ID_PE:
    {
      /* 
                                                           
      */
      pModContext = gpVosContext->pMACContext;
      break;
    }

#ifdef FEATURE_WLAN_INTEGRATED_SOC
    case VOS_MODULE_ID_WDA:
    {
      /*                */
      pModContext = gpVosContext->pWDAContext;
      break;
    }
#endif

    case VOS_MODULE_ID_VOSS:
    {
      /*                           */
      pModContext = gpVosContext;
      break;
    }

    default:
    {
      VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR,"%s: Module ID %i "
          "does not have its context maintained by VOSS", __func__, moduleId);
      VOS_ASSERT(0);
      return NULL;
    }
  }

  if (pModContext == NULL )
  {
    VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR,"%s: Module ID %i "
          "context is Null", __func__, moduleId);
  }

  return pModContext;

} /*                  */


/*                                                                            
  
                                                           
  
                                                                       
                             
  
                                                         
  
                                                                    
                       
                      
                                              
  
                                                                         
              
                                                                            */
v_CONTEXT_t vos_get_global_context( VOS_MODULE_ID moduleId, 
                                    v_VOID_t *moduleContext )
{
  if (gpVosContext == NULL)
  {
    VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR, 
        "%s: global voss context is NULL", __FUNCTION__);
  }

  return gpVosContext;

} /*                          */


v_U8_t vos_is_logp_in_progress(VOS_MODULE_ID moduleId, v_VOID_t *moduleContext)
{
  if (gpVosContext == NULL)
  {
    VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR, 
        "%s: global voss context is NULL", __FUNCTION__);
    return 1;
  }

   return gpVosContext->isLogpInProgress;
}

void vos_set_logp_in_progress(VOS_MODULE_ID moduleId, v_U8_t value)
{
  if (gpVosContext == NULL)
  {
    VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR, 
        "%s: global voss context is NULL", __FUNCTION__);
    return;
  }

   gpVosContext->isLogpInProgress = value;
}

v_U8_t vos_is_load_unload_in_progress(VOS_MODULE_ID moduleId, v_VOID_t *moduleContext)
{
  if (gpVosContext == NULL)
  {
    VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR, 
        "%s: global voss context is NULL", __FUNCTION__);
    return 0; 
  }

   return gpVosContext->isLoadUnloadInProgress;
}

void vos_set_load_unload_in_progress(VOS_MODULE_ID moduleId, v_U8_t value)
{
  if (gpVosContext == NULL)
  {
    VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR, 
        "%s: global voss context is NULL", __FUNCTION__);
    return;
  }

   gpVosContext->isLoadUnloadInProgress = value;
}

/*                                                                            
  
                                                                                
  
                                                                      
                       
  
                                                        
  
                                                                        
  
                                                                        
                                                                     
                                                                  
                                                     
  
                                                             
                      
                                                                        
                                                                    
                                               
                                                                  
                                                              
                                                    

                                                                     
                                                                    

                                                                           
                                                                         
                                                           
            
                                                                       
                                     
              
                                           
  
                                                                            */
VOS_STATUS vos_alloc_context( v_VOID_t *pVosContext, VOS_MODULE_ID moduleID, 
                              v_VOID_t **ppModuleContext, v_SIZE_t size )
{
  v_VOID_t ** pGpModContext = NULL;

  if ( pVosContext == NULL) {
    VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR, 
        "%s: vos context is null", __FUNCTION__);
    return VOS_STATUS_E_FAILURE;
  }

  if (( gpVosContext != pVosContext) || ( ppModuleContext == NULL)) {
    VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR, 
        "%s: context mismatch or null param passed", __FUNCTION__);
    return VOS_STATUS_E_FAILURE;
  }

  switch(moduleID)
  {
    case VOS_MODULE_ID_TL:  
    {
      pGpModContext = &(gpVosContext->pTLContext); 
      break;
    }

#ifndef FEATURE_WLAN_INTEGRATED_SOC
    case VOS_MODULE_ID_BAL:
    {
      pGpModContext = &(gpVosContext->pBALContext);
      break;
    }

    case VOS_MODULE_ID_SAL:
    {
      pGpModContext = &(gpVosContext->pSALContext);
      break;
    }   

    case VOS_MODULE_ID_SSC:
    {
      pGpModContext = &(gpVosContext->pSSCContext); 
      break;
    }
#endif
#ifdef WLAN_BTAMP_FEATURE
    case VOS_MODULE_ID_BAP:
    {
        pGpModContext = &(gpVosContext->pBAPContext);
        break;
    }    
#endif //                  

#ifdef WLAN_SOFTAP_FEATURE
    case VOS_MODULE_ID_SAP:
    {
      pGpModContext = &(gpVosContext->pSAPContext);
      break;
    }
#endif

#ifdef FEATURE_WLAN_INTEGRATED_SOC
    case VOS_MODULE_ID_WDA:
    {
      pGpModContext = &(gpVosContext->pWDAContext);
      break;
    }
#endif
    case VOS_MODULE_ID_SME:
#ifndef FEATURE_WLAN_INTEGRATED_SOC
    case VOS_MODULE_ID_HAL:
#endif
    case VOS_MODULE_ID_PE:
    case VOS_MODULE_ID_HDD:
#ifdef WLAN_SOFTAP_FEATURE
    case VOS_MODULE_ID_HDD_SOFTAP:
#endif
    default:
    {     
      VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR, "%s: Module ID %i "
          "does not have its context allocated by VOSS", __func__, moduleID);
      VOS_ASSERT(0);
      return VOS_STATUS_E_INVAL;
    }
  }

  if ( NULL != *pGpModContext)
  {
    /*
                                          
                                
    */
    VOS_TRACE( VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR,
               "%s: Module ID %i context has already been allocated",
                __func__, moduleID);
    return VOS_STATUS_E_EXISTS;
  }
  
  /*
                                                
  */
  
  *ppModuleContext = kmalloc(size, GFP_KERNEL);

  
  if ( *ppModuleContext == NULL)
  {
    VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR,"%s: Failed to "
        "allocate Context for module ID %i", __func__, moduleID);
    VOS_ASSERT(0);
    return VOS_STATUS_E_NOMEM;
  }
  
  if (moduleID==VOS_MODULE_ID_TL)
  {
     vos_mem_zero(*ppModuleContext, size);
  }

  *pGpModContext = *ppModuleContext;

  return VOS_STATUS_SUCCESS;

} /*                     */


/*                                                                            
  
                                                                     
                                                  
  
                                                                  
                       
  
                                                        
  
                                                                  
  
                                                                      
                      
                                                                        
                                                                   
                      
            
                                                                       
                           
                                 
                                                                     
                                                                 
            
                                                                   
                                                                      
                                            
              
                                     
  
                                                                            */
VOS_STATUS vos_free_context( v_VOID_t *pVosContext, VOS_MODULE_ID moduleID,
                             v_VOID_t *pModuleContext )
{
  v_VOID_t ** pGpModContext = NULL;

  if (( pVosContext == NULL) || ( gpVosContext != pVosContext) ||
      ( pModuleContext == NULL))
  {
    VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR,
        "%s: Null params or context mismatch", __func__);
    return VOS_STATUS_E_FAILURE;
  }
  

  switch(moduleID)
  {
    case VOS_MODULE_ID_TL:  
    {
      pGpModContext = &(gpVosContext->pTLContext); 
      break;
    }

#ifndef FEATURE_WLAN_INTEGRATED_SOC
    case VOS_MODULE_ID_BAL:
    {
      pGpModContext = &(gpVosContext->pBALContext);
      break;
    }   

    case VOS_MODULE_ID_SAL:
    {
      pGpModContext = &(gpVosContext->pSALContext);
      break;
    }   

    case VOS_MODULE_ID_SSC:
    {
      pGpModContext = &(gpVosContext->pSSCContext); 
      break;
    }
#endif
#ifdef WLAN_BTAMP_FEATURE
    case VOS_MODULE_ID_BAP:
    {
        pGpModContext = &(gpVosContext->pBAPContext);
        break;
    }
#endif //                  
 
#ifdef WLAN_SOFTAP_FEATURE
    case VOS_MODULE_ID_SAP:
    {
      pGpModContext = &(gpVosContext->pSAPContext); 
      break;
    }
#endif

#ifdef FEATURE_WLAN_INTEGRATED_SOC
    case VOS_MODULE_ID_WDA:
    {
      pGpModContext = &(gpVosContext->pWDAContext);
      break;
    }
#endif
    case VOS_MODULE_ID_HDD:
    case VOS_MODULE_ID_SME:
#ifndef FEATURE_WLAN_INTEGRATED_SOC
    case VOS_MODULE_ID_HAL:
#endif
    case VOS_MODULE_ID_PE:
#ifdef WLAN_SOFTAP_FEATURE
    case VOS_MODULE_ID_HDD_SOFTAP:
#endif
    default:
    {     
      VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR, "%s: Module ID %i "
          "does not have its context allocated by VOSS", __func__, moduleID);
      VOS_ASSERT(0);
      return VOS_STATUS_E_INVAL;
    }
  }

  if ( NULL == *pGpModContext)
  {
    /*
                                                       
    */
    VOS_TRACE( VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR,"%s: Module ID %i "
        "context has not been allocated or freed already", __func__,moduleID);
    return VOS_STATUS_E_FAILURE;
  }
  
  if (*pGpModContext != pModuleContext)
  {
    VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR, 
        "%s: pGpModContext != pModuleContext", __FUNCTION__);
    return VOS_STATUS_E_FAILURE;
  } 
  
  if(pModuleContext != NULL)
      kfree(pModuleContext);

  *pGpModContext = NULL;

  return VOS_STATUS_SUCCESS;

} /*                    */
                                                 

/*                                                                            
  
                                                                  

                                                                              
                                                
  
      
            
           
            
           
        
  
                                                                         
                        
         
                                                                           
                                                                           
                                                                         
                                                                              
                                                         
  
                                                                       
                               
          
                                                                          
                                            
          
                                                                   
          
                                                                       
                             

     
  
                                                                            */
VOS_STATUS vos_mq_post_message( VOS_MQ_ID msgQueueId, vos_msg_t *pMsg )
{
  pVosMqType      pTargetMq   = NULL;
  pVosMsgWrapper  pMsgWrapper = NULL;

  if ((gpVosContext == NULL) || (pMsg == NULL))
  {
    VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR,
        "%s: Null params or global vos context is null", __func__);
    VOS_ASSERT(0);
    return VOS_STATUS_E_FAILURE;
  }

  switch (msgQueueId)
  {
    //                                             
    case  VOS_MQ_ID_SME: 
    {
       pTargetMq = &(gpVosContext->vosSched.smeMcMq);
       break;
    }

    //                                            
    case VOS_MQ_ID_PE:  
    {
       pTargetMq = &(gpVosContext->vosSched.peMcMq);
       break;
    }

#ifndef FEATURE_WLAN_INTEGRATED_SOC
    //                                             
    case VOS_MQ_ID_HAL: 
    {
       pTargetMq = &(gpVosContext->vosSched.halMcMq);
       break;
    }
#else
    //                                             
    case VOS_MQ_ID_WDA: 
    {
       pTargetMq = &(gpVosContext->vosSched.wdaMcMq);
       break;
    }

    //                                             
    case VOS_MQ_ID_WDI:
    {
       pTargetMq = &(gpVosContext->vosSched.wdiMcMq);
       break;
    }
#endif

    //                                            
    case VOS_MQ_ID_TL: 
    {
       pTargetMq = &(gpVosContext->vosSched.tlMcMq);
       break;
    }

    //                                                        
    case VOS_MQ_ID_SYS:
    {
       pTargetMq = &(gpVosContext->vosSched.sysMcMq);
       break;
    }

    default:

    VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR,
              ("%s: Trying to queue msg into unknown MC Msg queue ID %d"),
              __func__, msgQueueId);

    return VOS_STATUS_E_FAILURE;
  }

  VOS_ASSERT(NULL !=pTargetMq);
  if (pTargetMq == NULL)
  {
     VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR, 
         "%s: pTargetMq == NULL", __FUNCTION__);
     return VOS_STATUS_E_FAILURE;
  } 

  /*
                                   
  */
  pMsgWrapper = vos_mq_get(&gpVosContext->freeVosMq);

  if (NULL == pMsgWrapper)
  {
    VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR,
              "%s: VOS Core run out of message wrapper", __func__);

    return VOS_STATUS_E_RESOURCES;
  }
  
  /*
                         
  */
  vos_mem_copy( (v_VOID_t*)pMsgWrapper->pVosMsg, 
                (v_VOID_t*)pMsg, sizeof(vos_msg_t));

  vos_mq_put(pTargetMq, pMsgWrapper);

  set_bit(MC_POST_EVENT_MASK, &gpVosContext->vosSched.mcEventFlag);
  wake_up_interruptible(&gpVosContext->vosSched.mcWaitQueue);

  return VOS_STATUS_SUCCESS;

} /*                      */


/*                                                                            
  
                                                                             

                                                                           
                                                                           
                         
  
      
           
                
       
  
                                                                         
                        
         
                                                                                
                                                                           
                                                                              
                                                                             
                                                                
  
                                                                       
                               
          
                                                                          
                                            
          
                                                                   
          
                                                                       
                             

     
  
                                                                            */
VOS_STATUS vos_tx_mq_serialize( VOS_MQ_ID msgQueueId, vos_msg_t *pMsg )
{
  pVosMqType      pTargetMq   = NULL;
  pVosMsgWrapper  pMsgWrapper = NULL;

  if ((gpVosContext == NULL) || (pMsg == NULL))
  {
    VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR,
        "%s: Null params or global vos context is null", __func__);
    VOS_ASSERT(0);
    return VOS_STATUS_E_FAILURE;
  }

  switch (msgQueueId)
  {
    //                                             
    case  VOS_MQ_ID_TL: 
    {
       pTargetMq = &(gpVosContext->vosSched.tlTxMq);
       break;
    }

#ifndef FEATURE_WLAN_INTEGRATED_SOC
    //                                             
    case VOS_MQ_ID_SSC:  
    {
       pTargetMq = &(gpVosContext->vosSched.sscTxMq);
       break;
    }
#else
    //                                             
    case VOS_MQ_ID_WDI:  
    {
       pTargetMq = &(gpVosContext->vosSched.wdiTxMq);
       break;
    }
#endif 
    
    //                                                        
    case VOS_MQ_ID_SYS:
    {
       pTargetMq = &(gpVosContext->vosSched.sysTxMq);
       break;
    }

    default:

    VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR,
              "Trying to queue msg into unknown Tx Msg queue ID %d",
               __func__, msgQueueId);

    return VOS_STATUS_E_FAILURE;
  }

  if (pTargetMq == NULL)
  {
     VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR, 
         "%s: pTargetMq == NULL", __FUNCTION__);
     return VOS_STATUS_E_FAILURE;
  } 
    

  /*
                                   
  */
  pMsgWrapper = vos_mq_get(&gpVosContext->freeVosMq);

  if (NULL == pMsgWrapper)
  {
    VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_FATAL,
              "%s: VOS Core run out of message wrapper", __func__);

    return VOS_STATUS_E_RESOURCES;
  }

  /*
                         
  */
  vos_mem_copy( (v_VOID_t*)pMsgWrapper->pVosMsg, 
                (v_VOID_t*)pMsg, sizeof(vos_msg_t));

  vos_mq_put(pTargetMq, pMsgWrapper);

  set_bit(TX_POST_EVENT_MASK, &gpVosContext->vosSched.txEventFlag);
  wake_up_interruptible(&gpVosContext->vosSched.txWaitQueue);

  return VOS_STATUS_SUCCESS;

} /*                      */

#ifdef FEATURE_WLAN_INTEGRATED_SOC
/*                                                                            

                                                                             

                                                                          
                                                                          
                         

      
           
            
       

                                                                         
                        

                                                                               
                                                                           
                                                                             
                                                                            
                                                                

                                                                       
                               

                                                                         
                                            

                                                              

                                                                       
                             

     

                                                                            */

VOS_STATUS vos_rx_mq_serialize( VOS_MQ_ID msgQueueId, vos_msg_t *pMsg )
{
  pVosMqType      pTargetMq   = NULL;
  pVosMsgWrapper  pMsgWrapper = NULL;
  if ((gpVosContext == NULL) || (pMsg == NULL))
  {
    VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR,
        "%s: Null params or global vos context is null", __func__);
    VOS_ASSERT(0);
    return VOS_STATUS_E_FAILURE;
  }

  switch (msgQueueId)
  {

    case VOS_MQ_ID_SYS:
    {
       pTargetMq = &(gpVosContext->vosSched.sysRxMq);
       break;
    }

    //                                             
    case VOS_MQ_ID_WDI:
    {
       pTargetMq = &(gpVosContext->vosSched.wdiRxMq);
       break;
    }

    default:

    VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR,
              "Trying to queue msg into unknown Rx Msg queue ID %d",
               __func__, msgQueueId);

    return VOS_STATUS_E_FAILURE;
  }

  if (pTargetMq == NULL)
  {
     VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR,
         "%s: pTargetMq == NULL", __FUNCTION__);
     return VOS_STATUS_E_FAILURE;
  }


  /*
                                   
  */
  pMsgWrapper = vos_mq_get(&gpVosContext->freeVosMq);

  if (NULL == pMsgWrapper)
  {
    VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR,
              "%s: VOS Core run out of message wrapper", __func__);

    return VOS_STATUS_E_RESOURCES;
  }

  /*
                         
  */
  vos_mem_copy( (v_VOID_t*)pMsgWrapper->pVosMsg,
                (v_VOID_t*)pMsg, sizeof(vos_msg_t));

  vos_mq_put(pTargetMq, pMsgWrapper);

  set_bit(RX_POST_EVENT_MASK, &gpVosContext->vosSched.rxEventFlag);
  wake_up_interruptible(&gpVosContext->vosSched.rxWaitQueue);

  return VOS_STATUS_SUCCESS;

} /*                      */

#endif
v_VOID_t 
vos_sys_probe_thread_cback 
( 
  v_VOID_t *pUserData 
)
{
  if (gpVosContext != pUserData)
  {
     VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR, 
         "%s: gpVosContext != pUserData", __FUNCTION__);
     return;
  } 

  if (vos_event_set(&gpVosContext->ProbeEvent)!= VOS_STATUS_SUCCESS)
  {
     VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR, 
         "%s: vos_event_set failed", __FUNCTION__);
     return;
  }

} /*                              */

#ifndef FEATURE_WLAN_INTEGRATED_SOC
v_VOID_t vos_sys_start_complete_cback
( 
  v_VOID_t *pUserData
)
{

  if (gpVosContext != pUserData)
  {
     VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR, 
         "%s: gpVosContext != pUserData", __FUNCTION__);
     return;
  } 

  if (vos_event_set(&gpVosContext->ProbeEvent)!= VOS_STATUS_SUCCESS)
  {
     VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR, 
         "%s: vos_event_set failed", __FUNCTION__);
     return;
  }

} /*                                */
#else
v_VOID_t vos_WDAComplete_cback
(
  v_VOID_t *pUserData
)
{

  if (gpVosContext != pUserData)
  {
     VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR,
         "%s: gpVosContext != pUserData", __FUNCTION__);
     return;
  }

  if (vos_event_set(&gpVosContext->wdaCompleteEvent)!= VOS_STATUS_SUCCESS)
  {
     VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR,
         "%s: vos_event_set failed", __FUNCTION__);
     return;
  }

} /*                         */
#endif

v_VOID_t vos_core_return_msg
(
  v_PVOID_t      pVContext, 
  pVosMsgWrapper pMsgWrapper
)
{
  pVosContextType pVosContext = (pVosContextType) pVContext;
  
  VOS_ASSERT( gpVosContext == pVosContext);

  if (gpVosContext != pVosContext)
  {
     VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR, 
         "%s: gpVosContext != pVosContext", __FUNCTION__);
     return;
  } 

  VOS_ASSERT( NULL !=pMsgWrapper );

  if (pMsgWrapper == NULL)
  {
     VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR, 
         "%s: pMsgWrapper == NULL in function", __FUNCTION__);
     return;
  } 
  
  /*
                                                 
  */
  INIT_LIST_HEAD(&pMsgWrapper->msgNode);
  vos_mq_put(&pVosContext->freeVosMq, pMsgWrapper);

} /*                       */


/* 
                                                                          
                                    
   
                                                      

          
      

*/
v_VOID_t 
vos_fetch_tl_cfg_parms 
( 
  WLANTL_ConfigInfoType *pTLConfig,
  hdd_config_t * pConfig
)
{
  if (pTLConfig == NULL)
  {
   VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR, "%s NULL ptr passed in!", __FUNCTION__);
   return;
  }

  pTLConfig->ucAcWeights[0] = pConfig->WfqBkWeight;
  pTLConfig->ucAcWeights[1] = pConfig->WfqBeWeight;
  pTLConfig->ucAcWeights[2] = pConfig->WfqViWeight;
  pTLConfig->ucAcWeights[3] = pConfig->WfqVoWeight;
  pTLConfig->uDelayedTriggerFrmInt = pConfig->DelayedTriggerFrmInt;
#ifdef WLAN_SOFTAP_FEATURE
  pTLConfig->uMinFramesProcThres = pConfig->MinFramesProcThres;
#endif

}

v_BOOL_t vos_is_apps_power_collapse_allowed(void* pHddCtx)
{
  return hdd_is_apps_power_collapse_allowed((hdd_context_t*) pHddCtx);
}

void vos_abort_mac_scan(void)
{
    hdd_context_t *pHddCtx = NULL;
    v_CONTEXT_t pVosContext        = NULL;

    /*                             */
    pVosContext = vos_get_global_context(VOS_MODULE_ID_SYS, NULL);
    if(!pVosContext) {
       hddLog(VOS_TRACE_LEVEL_FATAL, "%s: Global VOS context is Null", __func__);
       return;
    }
    
    /*                     */
    pHddCtx = (hdd_context_t *)vos_get_context(VOS_MODULE_ID_HDD, pVosContext );
    if(!pHddCtx) {
       hddLog(VOS_TRACE_LEVEL_FATAL, "%s: HDD context is Null", __func__);
       return;
    }

    hdd_abort_mac_scan(pHddCtx);
    return;
}

/*                                                                           

                                      

                                     

                                                                        
                            


                                        


                                                                      

                                                 

                                                                           */
VOS_STATUS vos_shutdown(v_CONTEXT_t vosContext)
{
  VOS_STATUS vosStatus;

#ifdef WLAN_BTAMP_FEATURE
  vosStatus = WLANBAP_Close(vosContext);
  if (!VOS_IS_STATUS_SUCCESS(vosStatus))
  {
     VOS_TRACE( VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR,
         "%s: Failed to close BAP", __func__);
     VOS_ASSERT( VOS_IS_STATUS_SUCCESS( vosStatus ) );
  }
#endif //                   

  vosStatus = WLANTL_Close(vosContext);
  if (!VOS_IS_STATUS_SUCCESS(vosStatus))
  {
     VOS_TRACE( VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR,
         "%s: Failed to close TL", __func__);
     VOS_ASSERT( VOS_IS_STATUS_SUCCESS( vosStatus ) );
  }

  vosStatus = sme_Close( ((pVosContextType)vosContext)->pMACContext);
  if (!VOS_IS_STATUS_SUCCESS(vosStatus))
  {
     VOS_TRACE( VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR,
         "%s: Failed to close SME", __func__);
     VOS_ASSERT( VOS_IS_STATUS_SUCCESS( vosStatus ) );
  }

  vosStatus = macClose( ((pVosContextType)vosContext)->pMACContext);
  if (!VOS_IS_STATUS_SUCCESS(vosStatus))
  {
     VOS_TRACE( VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR,
         "%s: Failed to close MAC", __func__);
     VOS_ASSERT( VOS_IS_STATUS_SUCCESS( vosStatus ) );
  }

  ((pVosContextType)vosContext)->pMACContext = NULL;

  vosStatus = vos_nv_close();
  if (!VOS_IS_STATUS_SUCCESS(vosStatus))
  {
     VOS_TRACE( VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR,
         "%s: Failed to close NV", __func__);
     VOS_ASSERT( VOS_IS_STATUS_SUCCESS( vosStatus ) );
  }

  vosStatus = sysClose( vosContext );
  if (!VOS_IS_STATUS_SUCCESS(vosStatus))
  {
     VOS_TRACE( VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR,
         "%s: Failed to close SYS", __func__);
     VOS_ASSERT( VOS_IS_STATUS_SUCCESS( vosStatus ) );
  }

 /*                                                             */
  vosStatus = vos_packet_close( vosContext );
  if (!VOS_IS_STATUS_SUCCESS(vosStatus))
  {
     VOS_TRACE( VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR,
         "%s: Failed to close VOSS Packet", __func__);
     VOS_ASSERT( VOS_IS_STATUS_SUCCESS( vosStatus ) );
  }

  vos_mq_deinit(&((pVosContextType)vosContext)->freeVosMq);

  vosStatus = vos_event_destroy(&gpVosContext->wdaCompleteEvent);
  if (!VOS_IS_STATUS_SUCCESS(vosStatus))
  {
     VOS_TRACE( VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR,
         "%s: failed to destroy wdaCompleteEvent", __func__);
     VOS_ASSERT( VOS_IS_STATUS_SUCCESS( vosStatus ) );
  }

  vosStatus = vos_event_destroy(&gpVosContext->ProbeEvent);
  if (!VOS_IS_STATUS_SUCCESS(vosStatus))
  {
     VOS_TRACE( VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR,
         "%s: failed to destroy ProbeEvent", __func__);
     VOS_ASSERT( VOS_IS_STATUS_SUCCESS( vosStatus ) );
  }

  return VOS_STATUS_SUCCESS;
}

/*                                                                           

                                                           

                       

                                        


                                                    

                                                 

                                                                           */
VOS_STATUS vos_wda_shutdown(v_CONTEXT_t vosContext)
{
  VOS_STATUS vosStatus;
  vosStatus = WDA_shutdown(vosContext, VOS_FALSE);

  if (!VOS_IS_STATUS_SUCCESS(vosStatus))
  {
     VOS_TRACE( VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR,
         "%s: failed to shutdown WDA", __func__);
     VOS_ASSERT( VOS_IS_STATUS_SUCCESS( vosStatus ) );
  }
  return vosStatus;
}
/* 
                                                                

                                                                     
                                                              
                                                                    
                                                      
                         
        
           
         
                                                               
                                               

*/
VOS_STATUS vos_wlanShutdown(void)
{
   VOS_STATUS vstatus;
   vstatus = vos_watchdog_wlan_shutdown();
   return vstatus;
}
/* 
                                                             

                                                                     
                                                              
                             
                                                                       
                                                                        
                                             
        
           
         
                                                               
                                               

*/
VOS_STATUS vos_wlanReInit(void)
{
   VOS_STATUS vstatus;
   vstatus = vos_watchdog_wlan_re_init();
   return vstatus;
}
/* 
                                                              

                                                                  
                                                                          
                                                                  

                                                                         

        
           
         
                                                               
                                               
                                                     
                                                         


*/
VOS_STATUS vos_wlanRestart(void)
{
   VOS_STATUS vstatus;
   hdd_context_t *pHddCtx = NULL;
   v_CONTEXT_t pVosContext        = NULL;

   /*                                                 */
   if(vos_is_load_unload_in_progress( VOS_MODULE_ID_VOSS, NULL)) 
   {
      VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR, 
               "%s: Driver load/unload is in progress, retry later.", __func__);
      return VOS_STATUS_E_AGAIN;
   }

   /*                             */
   pVosContext = vos_get_global_context(VOS_MODULE_ID_VOSS, NULL);
   if(!pVosContext) {
      VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_FATAL, 
               "%s: Global VOS context is Null", __func__);
      return VOS_STATUS_E_FAILURE;
   }
    
   /*                     */
   pHddCtx = (hdd_context_t *)vos_get_context(VOS_MODULE_ID_HDD, pVosContext );
   if(!pHddCtx) {
      VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_FATAL, 
               "%s: HDD context is Null", __func__);
      return VOS_STATUS_E_FAILURE;
   }

   /*                   */
   vstatus = wlan_hdd_restart_driver(pHddCtx);
   return vstatus;
}
