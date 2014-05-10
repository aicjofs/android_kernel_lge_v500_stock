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

               W L A N   S Y S T E M   M O D U L E


DESCRIPTION
  This file contains the system module that implements the 'exectution model'
  in the Gen6 host software.


  Copyright (c) 2008 QUALCOMM Incorporated. All Rights Reserved.
  Qualcomm Confidential and Proprietary
===========================================================================*/

/*                                                                           

                                           


                                                                       
                                                                


                                   


                                        
                                                                             
                                                                            
                                               
                                                                            
                                                                
                                       

                                                                           */


#include <wlan_qct_sys.h>
#include <vos_api.h>

#include <sirTypes.h>   //                         
#include <sirParams.h>  //                     
#include <sirApi.h>     //                                 
#include <wniApi.h>     //                                 
#include "aniGlobal.h"
#include "wlan_qct_wda.h"
#ifdef FEATURE_WLAN_NON_INTEGRATED_SOC
#include <halCommonApi.h>  //                              
#endif
#include "sme_Api.h"
#include "macInitApi.h"

#ifndef WLAN_FTM_STUB
VOS_STATUS WLANFTM_McProcessMsg (v_VOID_t *message);

#endif


//                                                                        
//                                                                       
//                      
#ifndef WLAN_FTM_STUB
#define SYS_MSG_ID_FTM_RSP      11
#define FTM_SYS_MSG_COOKIE      0xFACE
#endif

#define SYS_MSG_COOKIE ( 0xFACE )

//                                                 
#ifndef FIELD_OFFSET
#define FIELD_OFFSET(x,y) offsetof(x,y)
#endif

VOS_STATUS sys_SendSmeStartReq( v_CONTEXT_t pVosContext );

//                                      
typedef struct
{
   sysResponseCback mcStartCB;
   v_VOID_t *       mcStartUserData;

} sysContextData;

#if defined( FEATURE_WLAN_NON_INTEGRATED_SOC )
//                                                                              
//                                                                            
//                                
static sysContextData gSysContext;
#endif

static vos_event_t gStopEvt;

VOS_STATUS sysBuildMessageHeader( SYS_MSG_ID sysMsgId, vos_msg_t *pMsg )
{
   pMsg->type     = sysMsgId;
   pMsg->reserved = SYS_MSG_COOKIE;

   return( VOS_STATUS_SUCCESS );
}


VOS_STATUS sysOpen( v_CONTEXT_t pVosContext )
{
   return( VOS_STATUS_SUCCESS );
}


#if defined( FEATURE_WLAN_NON_INTEGRATED_SOC )
VOS_STATUS sysMcStart( v_CONTEXT_t pVosContext, sysResponseCback userCallback, v_VOID_t *pUserData )
{
   VOS_STATUS vosStatus = VOS_STATUS_SUCCESS;
   vos_msg_t sysMsg;

   sysBuildMessageHeader( SYS_MSG_ID_MC_START, &sysMsg );

   //                                                                     
   //                                      
   //          
   sysMsg.bodyptr = (void *)userCallback;
   sysMsg.bodyval = (v_U32_t)pUserData;

   //                   
   vosStatus = vos_mq_post_message( VOS_MQ_ID_SYS, &sysMsg );
   if ( !VOS_IS_STATUS_SUCCESS(vosStatus) )
   {
      vosStatus = VOS_STATUS_E_BADMSG;
   }

   return( vosStatus );
}
#endif  /*                                 */

v_VOID_t sysStopCompleteCb
(
  v_VOID_t *pUserData
)
{
  vos_event_t* pStopEvt = (vos_event_t *) pUserData;
  VOS_STATUS vosStatus;
/*                                                                         */

  vosStatus = vos_event_set( pStopEvt );
  VOS_ASSERT( VOS_IS_STATUS_SUCCESS ( vosStatus ) );

} /*                               */

VOS_STATUS sysStop( v_CONTEXT_t pVosContext )
{
   VOS_STATUS vosStatus = VOS_STATUS_SUCCESS;
   vos_msg_t sysMsg;
   v_U8_t evtIndex;

   /*                           */
   vosStatus = vos_event_init( &gStopEvt );

   if(! VOS_IS_STATUS_SUCCESS( vosStatus ))
   {
      return vosStatus;
   }

   /*                                                        */
   sysBuildMessageHeader( SYS_MSG_ID_MC_STOP, &sysMsg );

   //                                                                     
   //                                      
   //          
   sysMsg.bodyptr = (void *)sysStopCompleteCb;
   sysMsg.bodyval = (v_U32_t) &gStopEvt;

   //                   
   vosStatus = vos_mq_post_message( VOS_MQ_ID_SYS, &sysMsg );
   if ( !VOS_IS_STATUS_SUCCESS(vosStatus) )
   {
      vosStatus = VOS_STATUS_E_BADMSG;
   }

   vosStatus = vos_wait_events( &gStopEvt, 1, 0, &evtIndex );
   VOS_ASSERT( VOS_IS_STATUS_SUCCESS ( vosStatus ) );

   vosStatus = vos_event_destroy( &gStopEvt );
   VOS_ASSERT( VOS_IS_STATUS_SUCCESS ( vosStatus ) );

   return( vosStatus );
}


VOS_STATUS sysClose( v_CONTEXT_t pVosContext )
{
   return( VOS_STATUS_SUCCESS );
}


#if defined( FEATURE_WLAN_NON_INTEGRATED_SOC )
static VOS_STATUS sys_PostMcThreadProbeMsg( v_CONTEXT_t pVosContext, sysResponseCback userCallback,
                                          v_VOID_t *pUserData, SYS_MSG_ID sysMsgId )
{
   VOS_STATUS vosStatus = VOS_STATUS_SUCCESS;
   vos_msg_t sysMsg;
   tSirRetStatus sirStatus = eSIR_SUCCESS;
 /*                                                                                        */

   sysBuildMessageHeader( sysMsgId, &sysMsg );


   //                                                                     
   //                                      
   sysMsg.bodyptr = (void *)userCallback;
   sysMsg.bodyval = (v_U32_t)pUserData;

   //                    
   vosStatus = vos_mq_post_message( VOS_MQ_ID_SYS, &sysMsg );

   if ( eSIR_SUCCESS != sirStatus )
   {
      vosStatus = VOS_STATUS_E_BADMSG;
   }

   return( vosStatus );
}


static VOS_STATUS sys_PostTxThreadProbeMsg( v_CONTEXT_t pVosContext, sysResponseCback userCallback,
                                          v_VOID_t *pUserData, SYS_MSG_ID sysMsgId )
{
   VOS_STATUS vosStatus = VOS_STATUS_SUCCESS;
   vos_msg_t sysMsg;
   tSirRetStatus sirStatus = eSIR_SUCCESS;
 /*                                                                                        */

   sysBuildMessageHeader( sysMsgId, &sysMsg );


   //                                                                     
   //                                      
   sysMsg.bodyptr = (void *)userCallback;
   sysMsg.bodyval = (v_U32_t)pUserData;

   //                    
   vosStatus = vos_tx_mq_serialize( VOS_MQ_ID_SYS, &sysMsg );

   if ( eSIR_SUCCESS != sirStatus )
   {
      vosStatus = VOS_STATUS_E_BADMSG;
   }

   return( vosStatus );
}


v_VOID_t sysMcThreadProbe( v_CONTEXT_t pVosContext, sysResponseCback userCallback,
                           v_VOID_t *pUserData )
{
   VOS_STATUS vosStatus;

   vosStatus = sys_PostMcThreadProbeMsg( pVosContext, userCallback, pUserData,
                                       SYS_MSG_ID_MC_THR_PROBE );

   if (VOS_IS_STATUS_SUCCESS( vosStatus ))
   {
      //                                                               
      //                                                  
      VOS_ASSERT( 1 );
   }
   //                     
}


v_VOID_t sysTxThreadProbe( v_CONTEXT_t pVosContext, sysResponseCback userCallback,
                           v_VOID_t *pUserData )
{
   VOS_STATUS vosStatus;

   vosStatus = sys_PostTxThreadProbeMsg( pVosContext, userCallback, pUserData,
                                       SYS_MSG_ID_TX_THR_PROBE );

   if (VOS_IS_STATUS_SUCCESS( vosStatus ))
   {
      //                                                               
      //                                                  
      VOS_ASSERT( 1 );
   }
   //                     
}
#endif /*                                 */



#if defined( FEATURE_WLAN_NON_INTEGRATED_SOC )
static VOS_STATUS sys_SendHalInitStartReqMsg( v_CONTEXT_t pVosContext )
{
   VOS_STATUS vosStatus = VOS_STATUS_SUCCESS;
   tSirMbMsg msg;

   tSirRetStatus sirStatus = eSIR_SUCCESS;
   tSirMbMsg *pMsg = &msg;
   v_VOID_t *hHal;

   do
   {
      //                       
      hHal = vos_get_context( VOS_MODULE_ID_HAL, pVosContext );
      if ( NULL == hHal ) break;

     //                                                         
     //                                                     
     pMsg->type = WDA_INIT_START_REQ;
     pMsg->data[0] = (tANI_U32)hHal;

     //                                                      
     //                                                  
     pMsg->msgLen = sizeof( tHalHandle ) + FIELD_OFFSET( tSirMbMsg, data );

     //                                           
     sirStatus = uMacPostCtrlMsg( hHal, pMsg );
     if ( eSIR_SUCCESS != sirStatus )
     {
        vosStatus = VOS_STATUS_E_FAILURE;
     }

  } while( 0 );

   return( vosStatus );
}
#endif /*                                 */

#if defined(__ANI_COMPILER_PRAGMA_PACK_STACK)
#pragma pack( push )
#pragma pack( 1 )
#elif defined(__ANI_COMPILER_PRAGMA_PACK)
#pragma pack( 1 )
#endif

typedef struct sPolFileVersion
{
  unsigned char  MajorVersion;
  unsigned char  MinorVersion;
  unsigned char  Suffix;
  unsigned char  Build;

} tPolFileVersion;


typedef struct sPolFileHeader
{
  tPolFileVersion FileVersion;
  tPolFileVersion HWCapabilities;
  unsigned long   FileLength;
  unsigned long   NumDirectoryEntries;

} tPolFileHeader;


typedef enum ePolFileDirTypes
{
  ePOL_DIR_TYPE_BOOTLOADER = 0,
  ePOL_DIR_TYPE_STA_FIRMWARE,
  ePOL_DIR_TYPE_AP_FIRMWARE,
  ePOL_DIR_TYPE_DIAG_FIRMWARE,
  ePOL_DIR_TYPE_STA_CONFIG,
  ePOL_DIR_TYPE_AP_CONFIG

} tPolFileDirTypes;


typedef struct sPolFileDirEntry
{
  unsigned long DirEntryType;
  unsigned long DirEntryFileOffset;
  unsigned long DirEntryLength;

} tPolFileDirEntry;

#if defined(__ANI_COMPILER_PRAGMA_PACK_STACK)
#pragma pack( pop )
#endif


static unsigned short polFileChkSum( unsigned short *FileData, unsigned long NumWords )
{
  unsigned long Sum;

  for ( Sum = 0; NumWords > 0; NumWords-- )
  {
    Sum += *FileData++;
  }

  Sum  = (Sum >> 16) + (Sum & 0xffff); //          
  Sum += (Sum >> 16);                  //                          

  return( (unsigned short)( ~Sum ) );
}

#ifdef FEATURE_WLAN_NON_INTEGRATED_SOC
static v_BOOL_t sys_validateStaConfig( void *pImage, unsigned long cbFile,
   void **ppStaConfig, v_SIZE_t *pcbStaConfig )
#else
v_BOOL_t sys_validateStaConfig( void *pImage, unsigned long cbFile,
   void **ppStaConfig, v_SIZE_t *pcbStaConfig )
#endif
{
   v_BOOL_t fFound = VOS_FALSE;
   tPolFileHeader   *pFileHeader = NULL;
   tPolFileDirEntry *pDirEntry = NULL;
   v_U32_t idx;

   do
   {
      //                                                 
      if ( polFileChkSum( ( v_U16_t *)pImage, cbFile / sizeof( v_U16_t ) ) )
      {
         VOS_TRACE( VOS_MODULE_ID_SYS, VOS_TRACE_LEVEL_ERROR,
                    "Failed to validate the checksum for CFG binary"  );
         break;
      }

      pFileHeader = (tPolFileHeader *)pImage;

      *ppStaConfig = NULL;
      *pcbStaConfig = 0;

      pDirEntry = ( tPolFileDirEntry* ) ( pFileHeader + 1 );

      for ( idx = 0; idx < pFileHeader->NumDirectoryEntries; ++idx )
      {
         if ( ePOL_DIR_TYPE_STA_CONFIG == pDirEntry[ idx ].DirEntryType )
         {
            *ppStaConfig = pDirEntry[ idx ].DirEntryFileOffset + ( v_U8_t * )pFileHeader;

            *pcbStaConfig = pDirEntry[ idx ].DirEntryLength;

            break;
         }

      } //                                        

      if ( NULL != *ppStaConfig  )
      {
         VOS_TRACE( VOS_MODULE_ID_SYS, VOS_TRACE_LEVEL_INFO_LOW,
                    "Found the Station CFG in the CFG binary!!" );

         fFound = VOS_TRUE;
      }
      else
      {
         VOS_TRACE( VOS_MODULE_ID_SYS, VOS_TRACE_LEVEL_ERROR,
                    "Failed to find Station CFG in the CFG binary" );
      }

   } while( 0 );

   VOS_ASSERT( VOS_TRUE == fFound );

   return( fFound );
}

#if defined( FEATURE_WLAN_NON_INTEGRATED_SOC )
static v_U8_t _vImageArray[15000];

static VOS_STATUS sys_getCfgBinaryMsgBuffer( tSirMbMsg **ppMsg, v_SIZE_t *pcbCfgBinary )
{
   VOS_STATUS vosStatus = VOS_STATUS_E_FAILURE;

   v_VOID_t *pFileImage = NULL;
   v_SIZE_t cbFileImage = 0;

   v_VOID_t *pCfgBinary = NULL;
   v_SIZE_t cbCfgBinary = 0;

   v_BOOL_t bStatus = VOS_FALSE;

   do
   {
      //                                             
      vosStatus = vos_get_binary_blob( VOS_BINARY_ID_CONFIG, NULL, &cbFileImage );
      if ( VOS_STATUS_E_NOMEM != vosStatus )
      {
         VOS_TRACE( VOS_MODULE_ID_SYS, VOS_TRACE_LEVEL_ERROR,
                    "Error obtaining binary size" );
         break;
      }

      //                                                          
      //                                           
      pFileImage = (v_VOID_t*)_vImageArray;

      if ( NULL == pFileImage )
      {
         VOS_TRACE( VOS_MODULE_ID_SYS, VOS_TRACE_LEVEL_ERROR,
                    "Unable to allocate memory for the CFG binary [size= %d bytes]",
                    cbFileImage );

         vosStatus = VOS_STATUS_E_NOMEM;
         break;
      }

      //                                 
      vosStatus = vos_get_binary_blob( VOS_BINARY_ID_CONFIG, pFileImage, &cbFileImage );
      if ( !VOS_IS_STATUS_SUCCESS( vosStatus ) )
      {
         VOS_TRACE( VOS_MODULE_ID_SYS, VOS_TRACE_LEVEL_ERROR,
                    "Error: Cannot retrieve CFG fine image from vOSS. [size= %d bytes]",
                    cbFileImage );
         break;
      }

      //                                                                           
      //                                                              
      bStatus = sys_validateStaConfig( pFileImage, cbFileImage,
                                      &pCfgBinary, &cbCfgBinary );
      if ( VOS_FALSE == bStatus )
      {
         VOS_TRACE( VOS_MODULE_ID_SYS, VOS_TRACE_LEVEL_ERROR,
                    "Error: Cannot find STA CFG in binary image fileze" );
         vosStatus = VOS_STATUS_E_FAILURE;
         break;
      }

      //                                                             
      *pcbCfgBinary = cbCfgBinary;

      //                                                                            
      //                                                                             
      //                                    
      //
      //                                                            
      *ppMsg = vos_mem_malloc( cbCfgBinary + FIELD_OFFSET( tSirMbMsg, data ) );
      if ( NULL == *ppMsg )
      {
         VOS_TRACE( VOS_MODULE_ID_SYS, VOS_TRACE_LEVEL_ERROR,
                    "Unable to allocate memory for the CFG download message "
                    "buffer [size= %d bytes]",
                    cbCfgBinary + FIELD_OFFSET( tSirMbMsg, data ) );

         vosStatus = VOS_STATUS_E_NOMEM;
         break;
      }

      //                                                                 
      vos_mem_copy( (v_VOID_t *)( ( *ppMsg )->data ), pCfgBinary , cbCfgBinary );

      //                                   
      vosStatus = VOS_STATUS_SUCCESS;

   } while( 0 );

   //                                                               
   //                    
   if ( NULL == pFileImage )
   {
     //                            
   }

   //                                                               
   //                                                                  
   //                                                                
   //             
   if ( ( !VOS_IS_STATUS_SUCCESS( vosStatus ) ) && ( NULL != *ppMsg ) )
   {
      vos_mem_free( *ppMsg );
      *ppMsg = NULL;
      *pcbCfgBinary = 0;
   }

   return( vosStatus );
}
#endif /*                                 */






#if defined( FEATURE_WLAN_NON_INTEGRATED_SOC )
static VOS_STATUS sys_SendWniCfgDnldMsg( v_CONTEXT_t pVosContext )
{
   VOS_STATUS vosStatus = VOS_STATUS_E_FAILURE;

   tSirRetStatus sirStatus = eSIR_SUCCESS;
   tSirMbMsg *pMsg = NULL;
   v_VOID_t *hHal  = NULL;

   v_SIZE_t cbCfgBinary = 0;

   do
   {
      hHal = vos_get_context( VOS_MODULE_ID_HAL, pVosContext );
      if ( NULL == hHal ) break;

      vosStatus = sys_getCfgBinaryMsgBuffer( &pMsg, &cbCfgBinary );
      if ( !VOS_IS_STATUS_SUCCESS( vosStatus ) ) break;

      pMsg->type = WNI_CFG_DNLD_RSP;

      //                                                      
      //                                                  
      pMsg->msgLen = (tANI_U16)( cbCfgBinary + FIELD_OFFSET( tSirMbMsg, data ) );

      sirStatus = halMmhForwardMBmsg( hHal, pMsg );
      if ( eSIR_SUCCESS != sirStatus )
      {
         vosStatus = VOS_STATUS_E_FAILURE;
      }
      vos_mem_free(pMsg);
   } while( 0 );

   return( vosStatus );
}
#endif /*                                 */


#if defined( FEATURE_WLAN_NON_INTEGRATED_SOC )
VOS_STATUS sys_SendSmeStartReq( v_CONTEXT_t pVosContext )
{
   VOS_STATUS vosStatus = VOS_STATUS_SUCCESS;
   tSirSmeStartReq msg;

   tSirRetStatus sirStatus = eSIR_SUCCESS;
   tSirSmeStartReq *pMsg = &msg;
   v_VOID_t *hHal;

   do
   {
      //                       
      hHal = vos_get_context( VOS_MODULE_ID_HAL, pVosContext );
      if ( NULL == hHal ) break;

      //                                                        
      //                                                     
      pMsg->messageType      = eWNI_SME_START_REQ;
      pMsg->length           = sizeof( msg );
      pMsg->roamingAtPolaris = 0;
      pMsg->sendNewBssInd    = 0;

     //                                           
     sirStatus = halMmhForwardMBmsg( hHal, (tSirMbMsg *)pMsg );
     if ( eSIR_SUCCESS != sirStatus )
     {
        vosStatus = VOS_STATUS_E_FAILURE;
     }

  } while( 0 );

   return( vosStatus );
}
#endif /*                                 */

VOS_STATUS sysMcProcessMsg( v_CONTEXT_t pVosContext, vos_msg_t *pMsg )
{
   VOS_STATUS vosStatus = VOS_STATUS_SUCCESS;
   v_VOID_t *hHal;

   VOS_ASSERT( pMsg );

   //                                                                  
   //                                                                 
   //                                                                
   //                                                                   
   //                                                
   if ( SYS_MSG_COOKIE == pMsg->reserved )
   {
      //                                   
      switch( pMsg->type )
      {
         case SYS_MSG_ID_MC_START:
         {
#if defined( FEATURE_WLAN_NON_INTEGRATED_SOC )
            //                                                       
            //     
            gSysContext.mcStartCB = (sysResponseCback)pMsg->bodyptr;
            gSysContext.mcStartUserData= (v_VOID_t *)pMsg->bodyval;

            //                                                 
            //                                                  
            vosStatus = sys_SendHalInitStartReqMsg( pVosContext );
#else
            /*                                                       
                                        */
            VOS_TRACE( VOS_MODULE_ID_SYS, VOS_TRACE_LEVEL_ERROR,
                       " Received SYS_MSG_ID_MC_START message msgType= %d [0x%08lx]",
                       pMsg->type, pMsg->type );
            VOS_ASSERT(0);
#endif
            break;
         }

         case SYS_MSG_ID_MC_STOP:
         {
            VOS_TRACE( VOS_MODULE_ID_SYS, VOS_TRACE_LEVEL_INFO,
                       "Processing SYS MC STOP" );

            //                       
            hHal = vos_get_context( VOS_MODULE_ID_PE, pVosContext );
            if (NULL == hHal)
            {
               VOS_TRACE( VOS_MODULE_ID_SYS, VOS_TRACE_LEVEL_ERROR,
                          "%s: Invalid hHal", __FUNCTION__ );
            }
            else
            {
               vosStatus = sme_Stop( hHal, TRUE );
               VOS_ASSERT( VOS_IS_STATUS_SUCCESS( vosStatus ) );

               vosStatus = macStop( hHal, HAL_STOP_TYPE_SYS_DEEP_SLEEP );
               VOS_ASSERT( VOS_IS_STATUS_SUCCESS( vosStatus ) );

               ((sysResponseCback)pMsg->bodyptr)((v_VOID_t *)pMsg->bodyval);

               vosStatus = VOS_STATUS_SUCCESS;
            }
            break;
         }

         //                                               
         //                                 
         case SYS_MSG_ID_MC_THR_PROBE:
         {
#if defined( FEATURE_WLAN_NON_INTEGRATED_SOC )
            sysResponseCback callback;

            //                                                  
            //                                                
            //                    
            callback = (sysResponseCback)pMsg->bodyptr;

            callback( (v_VOID_t *)pMsg->bodyval );
#else
            /*                                                       
                                        */
            VOS_TRACE( VOS_MODULE_ID_SYS, VOS_TRACE_LEVEL_ERROR,
                       " Received SYS_MSG_ID_MC_THR_PROBE message msgType= %d [0x%08lx]",
                       pMsg->type, pMsg->type );
            VOS_ASSERT(0);
#endif
            break;
         }

         case SYS_MSG_ID_MC_TIMER:
         {
            vos_timer_callback_t timerCB;
            //                       
            //                                                  
            //                                                      
            //                 
            timerCB = (vos_timer_callback_t)pMsg->bodyptr;

            //                                          
            timerCB( (v_VOID_t *)pMsg->bodyval );

            break;
         }
#ifndef WLAN_FTM_STUB
         case SYS_MSG_ID_FTM_RSP:
         {
             WLANFTM_McProcessMsg((v_VOID_t *)pMsg->bodyptr);
             break;
         }
#endif

         default:
         {
            VOS_TRACE( VOS_MODULE_ID_SYS, VOS_TRACE_LEVEL_ERROR,
                       "Unknown message type in sysMcProcessMsg() msgType= %d [0x%08lx]",
                       pMsg->type, pMsg->type );
            break;
        }

      }   //                           

   }   //                  
   else
   {
      //                              
      switch( pMsg->type )
      {
#if defined( FEATURE_WLAN_NON_INTEGRATED_SOC )
         /*                                                                        
                                                                     */
         case WDA_APP_SETUP_NTF:
         {
            //                                                             
            //                                                               
            //                                                                 
            //                                     
            break;
         }

         case WNI_CFG_DNLD_REQ:
         {
            //                                                               
            //                                                    
            vosStatus = sys_SendWniCfgDnldMsg( pVosContext );
            if (pMsg->bodyptr) 
               vos_mem_free(pMsg->bodyptr); 
            break;
         }

         case WNI_CFG_DNLD_CNF:
         {
            tSirMbMsg *pSirMsg = (tSirMbMsg *)pMsg->bodyptr;

            VOS_TRACE( VOS_MODULE_ID_SYS, VOS_TRACE_LEVEL_INFO,
                       "WNI_CFG_DNLD_CNF message received with status code= %d [0x%08lX]",
                       pSirMsg->data[0], pSirMsg->data[0] );

            //                                                                       
            //                                                                    
            //                                                         

            if ( WNI_CFG_SUCCESS == pSirMsg->data[ 0 ] )
            {
               VOS_TRACE( VOS_MODULE_ID_SYS, VOS_TRACE_LEVEL_INFO,
                          "WNI_CFG_DNLD_CNF received with successful status" );
            }
            else
            {
               //                                                             
               //                                                           
               //                                                     
               VOS_ASSERT( 0 );

               VOS_TRACE( VOS_MODULE_ID_SYS, VOS_TRACE_LEVEL_INFO,
                          "WNI_CFG_DNLD_CNF received with status= %d [0x%08lX]",
                          pSirMsg->data[ 0 ], pSirMsg->data[ 0 ] );
            }
            if (pMsg->bodyptr) 
               vos_mem_free(pMsg->bodyptr); 
            break;
         }

         case WDA_NIC_OPER_NTF:
         {
            VOS_TRACE( VOS_MODULE_ID_SYS, VOS_TRACE_LEVEL_INFO,
                       "WDA_NIC_OPER_NTF message received" );

            //                                                             
            //                                                                  
            //                                                        
            //                                                         
            //                                                                
            //                                                   
            //                                                    
            vosStatus = sys_SendSmeStartReq( pVosContext );
            if (pMsg->bodyptr) 
                vos_mem_free(pMsg->bodyptr); 
            break;
         }

         case eWNI_SME_START_RSP:
         {
            tSirSmeRsp *pSirSmeRspMsg = (tSirSmeRsp *)pMsg->bodyptr;
#ifdef WLAN_DEBUG
            VOS_TRACE( VOS_MODULE_ID_SYS, VOS_TRACE_LEVEL_INFO,
                       "eWNI_SME_START_RSP received with status code= %d [0x%08lX]",
                       pSirSmeRspMsg->statusCode, pSirSmeRspMsg->statusCode );
#endif
            //                                                                   
            //                                                                  
            //                                                  
            if ( gSysContext.mcStartCB )
            {
               gSysContext.mcStartCB( gSysContext.mcStartUserData );
            }
            if (pMsg->bodyptr) 
                vos_mem_free(pMsg->bodyptr); 
            break;
         }
#endif  /*                                 */

         default:
         {
            VOS_ASSERT( 0 );

            VOS_TRACE( VOS_MODULE_ID_SYS, VOS_TRACE_LEVEL_ERROR,
                       "Received SYS message cookie with unidentified "
                       "MC message type= %d [0x%08lX]", pMsg->type, pMsg->type );

            vosStatus = VOS_STATUS_E_BADMSG;
            if (pMsg->bodyptr) 
               vos_mem_free(pMsg->bodyptr);
            break;
         }
      }   //                         
   }   //         

   return( vosStatus );
}




VOS_STATUS sysTxProcessMsg( v_CONTEXT_t pVosContext, vos_msg_t *pMsg )
{
   VOS_STATUS vosStatus = VOS_STATUS_SUCCESS;

   VOS_ASSERT( pMsg );

   //                                                                  
   //                                                                 
   //                                                                
   //                                                                   
   //                                                
   if ( SYS_MSG_COOKIE == pMsg->reserved )
   {
      //                                   
      switch( pMsg->type )
      {
         //                                               
         //                                 
         case SYS_MSG_ID_TX_THR_PROBE:
         {
#if defined( FEATURE_WLAN_NON_INTEGRATED_SOC )
            sysResponseCback callback;

            //                                                  
            //                                                
            //                    
            callback = (sysResponseCback)pMsg->bodyptr;

            callback( (v_VOID_t *)pMsg->bodyval );
#else
           /*                                                       
                                        */
            VOS_TRACE( VOS_MODULE_ID_SYS, VOS_TRACE_LEVEL_ERROR,
                       " Received SYS_MSG_ID_TX_THR_PROBE message msgType= %d [0x%08lx]",
                       pMsg->type, pMsg->type );
            VOS_ASSERT(0);

#endif
            break;
         }

         case SYS_MSG_ID_TX_TIMER:
         {
            vos_timer_callback_t timerCB;

            //                       
            //                                                  
            //                                                      
            //                 
            timerCB = (vos_timer_callback_t)pMsg->bodyptr;

            //                                          
            timerCB( (v_VOID_t *)pMsg->bodyval );

            break;
         }

         default:
         {
            VOS_TRACE( VOS_MODULE_ID_SYS, VOS_TRACE_LEVEL_ERROR,
                       "Unknown message type in sysTxProcessMsg() msgType= %d [0x%08lx]",
                       pMsg->type, pMsg->type );
            break;
        }

      }   //                           
   }   //                  
   else
   {
      VOS_ASSERT( 0 );

      VOS_TRACE( VOS_MODULE_ID_SYS, VOS_TRACE_LEVEL_ERROR,
                 "Received SYS message cookie with unidentified TX message "
                 " type= %d [0x%08lX]", pMsg->type, pMsg->type );

      vosStatus = VOS_STATUS_E_BADMSG;
   }   //         

   return( vosStatus );
}


#ifdef FEATURE_WLAN_INTEGRATED_SOC
VOS_STATUS sysRxProcessMsg( v_CONTEXT_t pVosContext, vos_msg_t *pMsg )
{
   VOS_STATUS vosStatus = VOS_STATUS_SUCCESS;

   VOS_ASSERT( pMsg );

   //                                                                  
   //                                                                 
   //                                                                
   //                                                                   
   //                                                
   if ( SYS_MSG_COOKIE == pMsg->reserved )
   {
      //                                   
      switch( pMsg->type )
      {
         case SYS_MSG_ID_RX_TIMER:
         {
            vos_timer_callback_t timerCB;

            //                       
            //                                                  
            //                                                      
            //                 
            timerCB = (vos_timer_callback_t)pMsg->bodyptr;

            //                                          
            timerCB( (v_VOID_t *)pMsg->bodyval );

            break;
         }

         default:
         {
            VOS_TRACE( VOS_MODULE_ID_SYS, VOS_TRACE_LEVEL_ERROR,
                       "Unknown message type in sysRxProcessMsg() msgType= %d [0x%08lx]",
                       pMsg->type, pMsg->type );
            break;
        }

      }   //                           
   }   //                  
   else
   {
      VOS_ASSERT( 0 );

      VOS_TRACE( VOS_MODULE_ID_SYS, VOS_TRACE_LEVEL_ERROR,
                 "Received SYS message cookie with unidentified RX message "
                 " type= %d [0x%08lX]", pMsg->type, pMsg->type );

      vosStatus = VOS_STATUS_E_BADMSG;
   }   //         

   return( vosStatus );
}

#endif

v_VOID_t sysMcFreeMsg( v_CONTEXT_t pVContext, vos_msg_t* pMsg )
{
   return;
}


v_VOID_t sysTxFreeMsg( v_CONTEXT_t pVContext, vos_msg_t* pMsg )
{
   return;
}


void
SysProcessMmhMsg
(
  tpAniSirGlobal pMac,
  tSirMsgQ* pMsg
)
{
  VOS_MQ_ID   targetMQ = VOS_MQ_ID_SYS;
/*                                                                         */
  /*
                                         
                                              
                                      
  */

  VOS_ASSERT(NULL != pMsg);


  switch (pMsg->type)
  {
    /*
                                           
    */
    case WNI_CFG_DNLD_REQ:
    case WNI_CFG_DNLD_CNF:
    case WDA_APP_SETUP_NTF:
    case WDA_NIC_OPER_NTF:
    case WDA_RESET_REQ:
    case eWNI_SME_START_RSP:
    {
      /*                                        */
      targetMQ = VOS_MQ_ID_SYS;

#if defined( FEATURE_WLAN_INTEGRATED_SOC )
      VOS_TRACE( VOS_MODULE_ID_SYS, VOS_TRACE_LEVEL_ERROR,
                 "Handling for the Message ID %d is removed in SYS\r\n",
                 pMsg->type);

      VOS_ASSERT(0);
#endif /*                             */
      break;
    }


    /*
                                           
    */
    case WNI_CFG_DNLD_RSP:
    case WDA_INIT_START_REQ:
    {
      /*                                        */
      targetMQ = VOS_MQ_ID_WDA;

#if defined ( FEATURE_WLAN_INTEGRATED_SOC )
      VOS_TRACE( VOS_MODULE_ID_SYS, VOS_TRACE_LEVEL_ERROR,
                 "Handling for the Message ID %d is removed as there is no HAL \r\n",
                 pMsg->type);

      VOS_ASSERT(0);
#endif /*                             */
      break;
    }

    case eWNI_SME_START_REQ:
    case WNI_CFG_GET_REQ:
    case WNI_CFG_SET_REQ:
    case WNI_CFG_SET_REQ_NO_RSP:
    case eWNI_SME_SYS_READY_IND:
    {
       /*                                       */
      targetMQ = VOS_MQ_ID_PE;
      break;
    }


    case WNI_CFG_GET_RSP:
    case WNI_CFG_SET_CNF:
/*                              
                               
                                
                                  
                                
                                 
                                
                                
                                 
                                
                                  
                                 
                                 
                                
                                  */ //                                       
    {
       /*                                        */
      targetMQ = VOS_MQ_ID_SME;
      break;
    }

    default:
    {

      if ( ( pMsg->type >= eWNI_SME_MSG_TYPES_BEGIN )  &&  ( pMsg->type <= eWNI_SME_MSG_TYPES_END ) )
      {
         targetMQ = VOS_MQ_ID_SME;
         break;
      }

      VOS_TRACE( VOS_MODULE_ID_SYS, VOS_TRACE_LEVEL_ERROR,
                 "Message of ID %d is not yet handled by SYS\r\n",
                 pMsg->type);

      VOS_ASSERT(0);
    }

  }


  /*
                                                                
  */
  if(VOS_STATUS_SUCCESS != vos_mq_post_message(targetMQ, (vos_msg_t*)pMsg))
  {
    //                                                                           
    /*                         */
    if(pMsg->bodyptr)
    {
      palFreeMemory( pMac->hHdd, pMsg->bodyptr);
    }
  }

} /*                    */

#ifndef WLAN_FTM_STUB
/*                                                                          
                              

             
                                                                            
                                                                          

              
                                                                 

            

      
                                                                        
                                                                   
                                                   


              
                                                            

                       

              
          
                                                                            */

void wlan_sys_ftm(void *pMsgPtr)
{
    vos_msg_t  vosMessage;



    vosMessage.reserved = FTM_SYS_MSG_COOKIE;
    vosMessage.type     = SYS_MSG_ID_FTM_RSP;
    vosMessage.bodyptr  = pMsgPtr;

    vos_mq_post_message(VOS_MQ_ID_SYS, &vosMessage);

    return;
}

#endif /*               */


