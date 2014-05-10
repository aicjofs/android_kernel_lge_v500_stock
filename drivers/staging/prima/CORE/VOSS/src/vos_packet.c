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

  \file        vos_packet.c

  \brief       virtual Operating System Services (vOSS) network Packet APIs

   Network Protocol packet/buffer support interfaces

   Copyright 2009 (c) Qualcomm, Incorporated.  All Rights Reserved.

   Qualcomm Confidential and Proprietary.

  ========================================================================*/

/*          */

/*                                                                          
               
                                                                          */
#include <vos_packet.h>
#include <i_vos_packet.h>
#include <vos_timer.h>
#include <vos_trace.h>
#ifdef WLAN_SOFTAP_FEATURE
#include <wlan_hdd_main.h>   
#endif

/*                                                                          
                                        
                                                                          */

/*                                                                          
                   
                                                                          */

/*                                                                           
                  
                                                                          */
static vos_pkt_context_t *gpVosPacketContext = NULL;

/*                                                                         
                                         
                                                                          */

static VOS_STATUS vos_pkti_packet_init( struct vos_pkt_t *pPkt,
                                 VOS_PKT_TYPE pktType )
{
   VOS_STATUS vosStatus = VOS_STATUS_SUCCESS;

   //             
   pPkt->packetType = pktType;
   pPkt->magic = VPKT_MAGIC_NUMBER;

   //                                       
   switch (pktType)
   {
   case VOS_PKT_TYPE_RX_RAW:
   case VOS_PKT_TYPE_TX_802_11_MGMT:
      //                            
      //                                                              
      //                                                       
      pPkt->pSkb = alloc_skb(VPKT_SIZE_BUFFER, GFP_ATOMIC);
      if (likely(pPkt->pSkb))
      {
         skb_reserve(pPkt->pSkb, VPKT_SIZE_BUFFER);
      }
      else
      {
         vosStatus = VOS_STATUS_E_NOMEM;
      }

#ifdef FEATURE_WLAN_INTEGRATED_SOC
      /*                 */
      WPAL_PACKET_SET_BD_POINTER(&(pPkt->palPacket), NULL);
      WPAL_PACKET_SET_BD_PHYS(&(pPkt->palPacket), NULL);
      WPAL_PACKET_SET_BD_LENGTH(&(pPkt->palPacket), 0);
      WPAL_PACKET_SET_OS_STRUCT_POINTER(&(pPkt->palPacket), NULL);
#endif

      break;
   default:
      //                       
      break;
   }

   return vosStatus;
}



static VOS_STATUS vos_pkti_list_destroy( struct list_head *pList )
{
   struct vos_pkt_t *pVosPacket;

   if (unlikely(NULL == pList))
   {
      //                                                
      //                                                
      VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_FATAL,
                "VPKT [%d]: NULL pList", __LINE__);
      VOS_ASSERT(0);
      return VOS_STATUS_E_INVAL;
   }

   list_for_each_entry(pVosPacket, pList, node)
   {

      //                                          
      if (unlikely(VPKT_MAGIC_NUMBER != pVosPacket->magic))
      {
         //                                                 
         //                                               
         //                    
         VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_FATAL,
                   "VPKT [%d]: Invalid magic", __LINE__);
         VOS_ASSERT(0);
         break;
      }

      //                                           
      if (pVosPacket->pSkb)
      {
         //                                   
         kfree_skb(pVosPacket->pSkb);
         pVosPacket->pSkb = NULL;
      }

      //                                                                    
      //                                                                   
      //                                        
      pVosPacket->magic = 0;

   }

   //                                                                   
   INIT_LIST_HEAD(pList);

   return VOS_STATUS_SUCCESS;
}


static void vos_pkti_replenish_raw_pool(void)
{
   struct sk_buff * pSkb;
   struct vos_pkt_t *pVosPacket;
   v_BOOL_t didOne = VOS_FALSE;
   vos_pkt_get_packet_callback callback;

   //                                                                        
   if (likely(0 == gpVosPacketContext->rxReplenishListCount))
   {
      return;
   }

   //                                                                    
   //                          
   mutex_lock(&gpVosPacketContext->mlock);


   if ((gpVosPacketContext->rxReplenishListCount < VPKT_RX_REPLENISH_THRESHOLD)
       &&
       (!list_empty(&gpVosPacketContext->rxRawFreeList)))
   {
      mutex_unlock(&gpVosPacketContext->mlock);
      return;
   }

   VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_INFO,
             "VPKT [%d]: Packet replenish activated", __LINE__);

   //                                                          
   while (gpVosPacketContext->rxReplenishListCount)
   {
      //                                                              
      //                                                       
      pSkb = alloc_skb(VPKT_SIZE_BUFFER, GFP_ATOMIC);
      if (unlikely(NULL == pSkb))
      {
         //                                    
         break;
      }
      skb_reserve(pSkb, VPKT_SIZE_BUFFER);

      //                                            
      pVosPacket = list_first_entry(&gpVosPacketContext->rxReplenishList,
                                    struct vos_pkt_t, node);
      list_del(&pVosPacket->node);
      gpVosPacketContext->rxReplenishListCount--;

      //                                 
      pVosPacket->pSkb = pSkb;

      //                               
      list_add_tail(&pVosPacket->node, &gpVosPacketContext->rxRawFreeList);

      didOne = VOS_TRUE;

      VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_INFO,
                "VPKT [%d]: [%p] Packet replenished",
                __LINE__, pVosPacket);

   }

   //                                                              
   //                         
   if ((VOS_TRUE == didOne) &&
       (gpVosPacketContext->rxRawLowResourceInfo.callback))
   {
      //                                           
      pVosPacket = list_first_entry(&gpVosPacketContext->rxRawFreeList,
                                    struct vos_pkt_t, node);
      list_del(&pVosPacket->node);

      //                                                      
      memset(&pVosPacket->pvUserData, 0, sizeof(pVosPacket->pvUserData));

      //                                        
      pVosPacket->pNext = NULL;

      //                          
      pVosPacket->timestamp = vos_timer_get_system_ticks();

      VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_INFO,
                "VPKT [%d]: [%p] Packet replenish callback",
                __LINE__, pVosPacket);

      callback = gpVosPacketContext->rxRawLowResourceInfo.callback;
      gpVosPacketContext->rxRawLowResourceInfo.callback = NULL;
      mutex_unlock(&gpVosPacketContext->mlock);
      callback(pVosPacket, gpVosPacketContext->rxRawLowResourceInfo.userData);
   }
   else
   {
      mutex_unlock(&gpVosPacketContext->mlock);
   }
}


#if defined( WLAN_DEBUG )
static char *vos_pkti_packet_type_str(VOS_PKT_TYPE pktType)
{
   switch (pktType)
   {
   case VOS_PKT_TYPE_TX_802_11_MGMT:
      return "TX_802_11_MGMT";
      break;
   
   case VOS_PKT_TYPE_TX_802_11_DATA:
      return  "TX_802_11_DATA";
      break;
       
   case VOS_PKT_TYPE_TX_802_3_DATA:
      return "TX_802_3_DATA";
      break;
   
   case VOS_PKT_TYPE_RX_RAW:
      return "RX_RAW";
      break;

   default:
      return "UNKNOWN";
      break;
   } 
}
#endif //                      

/*                                                                           

                                                              

                                                               
         

                                                          

                                                               
                                                           

                                                               
                         

                                                                 
                                              

                                                                       
                                                            

                                                                       
                               

                                                                       
                  

                                                                     
                

                        

                                                                           */
VOS_STATUS vos_packet_open( v_VOID_t *pVosContext,
                            vos_pkt_context_t *pVosPacketContext,
                            v_SIZE_t vosPacketContextSize )
{
   VOS_STATUS vosStatus = VOS_STATUS_SUCCESS;
   unsigned int freePacketIndex;
   unsigned int idx;
   struct vos_pkt_t *pPkt;
   struct list_head *pFreeList;

   VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_INFO, "Enter:%s",__FUNCTION__);

   do
   {

      if (NULL == pVosContext)
      {
         VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_FATAL,
                   "VPKT [%d]: NULL pVosContext", __LINE__);
         vosStatus = VOS_STATUS_E_INVAL;
         break;
      }

      if (NULL == pVosPacketContext)
      {
         VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_FATAL,
                   "VPKT [%d]: NULL pVosPacketContext", __LINE__);
         vosStatus = VOS_STATUS_E_INVAL;
         break;
      }

      if (sizeof(vos_pkt_context_t) != vosPacketContextSize)
      {
         VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_FATAL,
                   "VPKT [%d]: invalid vosPacketContextSize, %d vs %d",
                   __LINE__, sizeof(vos_pkt_context_t), vosPacketContextSize);
         vosStatus = VOS_STATUS_E_INVAL;
         break;
      }

      //                                                        
      //                                        
      memset(pVosPacketContext, 0, vosPacketContextSize);

      //                                                 
      gpVosPacketContext = pVosPacketContext;

      //                                                        
      pVosPacketContext->vosContext = pVosContext;

      //                                                   
      INIT_LIST_HEAD(&pVosPacketContext->rxReplenishList);
      pVosPacketContext->rxReplenishListCount = 0;

      //                                                     
      freePacketIndex = 0;

      //                                    
      pFreeList = &pVosPacketContext->rxRawFreeList;
      INIT_LIST_HEAD(pFreeList);

      //                         
      for (idx = 0; idx < VPKT_NUM_RX_RAW_PACKETS; idx++)
      {
         pPkt = &pVosPacketContext->vosPktBuffers[freePacketIndex++];
         vosStatus = vos_pkti_packet_init(pPkt, VOS_PKT_TYPE_RX_RAW);

#ifdef FEATURE_WLAN_INTEGRATED_SOC
         WPAL_PACKET_SET_METAINFO_POINTER(&(pPkt->palPacket),
                  (void*)&pVosPacketContext->rxMetaInfo[idx]);
         WPAL_PACKET_SET_TYPE(&(pPkt->palPacket), 
                              eWLAN_PAL_PKT_TYPE_RX_RAW);
#endif

         if (VOS_STATUS_SUCCESS != vosStatus)
         {
            VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_FATAL,
                      "VPKT [%d]: Packet init failure", __LINE__);
            break;
         }
         list_add_tail(&pPkt->node, pFreeList);
      }

      //                            
      if (VOS_STATUS_SUCCESS != vosStatus)
      {
         break;
      }

      //                                     
      pFreeList = &pVosPacketContext->txDataFreeList;
      INIT_LIST_HEAD(pFreeList);

      //                          
      for (idx = 0; idx < VPKT_NUM_TX_DATA_PACKETS; idx++)
      {
         pPkt = &pVosPacketContext->vosPktBuffers[freePacketIndex++];
         vosStatus = vos_pkti_packet_init(pPkt, VOS_PKT_TYPE_TX_802_3_DATA);
#ifdef FEATURE_WLAN_INTEGRATED_SOC
         WPAL_PACKET_SET_METAINFO_POINTER(&(pPkt->palPacket),
               (void*)&pVosPacketContext->txDataMetaInfo[idx]);
         WPAL_PACKET_SET_TYPE(&(pPkt->palPacket), 
                              eWLAN_PAL_PKT_TYPE_TX_802_3_DATA);
#endif
         if (VOS_STATUS_SUCCESS != vosStatus)
         {
            VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_FATAL,
                      "VPKT [%d]: Packet init failure", __LINE__);
            break;
         }
         list_add_tail(&pPkt->node, pFreeList);
#ifdef WLAN_SOFTAP_FEATURE
         pVosPacketContext->uctxDataFreeListCount++;
#endif
      }

      //                            
      if (VOS_STATUS_SUCCESS != vosStatus)
      {
         break;
      }

      //                                     
      pFreeList = &pVosPacketContext->txMgmtFreeList;
      INIT_LIST_HEAD(pFreeList);

      mutex_init(&gpVosPacketContext->mlock);
      //                          
      for (idx = 0; idx < VPKT_NUM_TX_MGMT_PACKETS; idx++)
      {
         pPkt = &pVosPacketContext->vosPktBuffers[freePacketIndex++];

         vosStatus = vos_pkti_packet_init(pPkt, VOS_PKT_TYPE_TX_802_11_MGMT);

#ifdef FEATURE_WLAN_INTEGRATED_SOC
         WPAL_PACKET_SET_METAINFO_POINTER(&(pPkt->palPacket),
               (void*)&pVosPacketContext->txMgmtMetaInfo[idx]);
         WPAL_PACKET_SET_TYPE(&(pPkt->palPacket), 
                              eWLAN_PAL_PKT_TYPE_TX_802_11_MGMT);
#endif

         if (VOS_STATUS_SUCCESS != vosStatus)
         {
            VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_FATAL,
                      "VPKT [%d]: Packet init failure", __LINE__);
            break;
         }
         list_add_tail(&pPkt->node, pFreeList);
      }

      //                            
      if (VOS_STATUS_SUCCESS != vosStatus)
      {
         break;
      }

   } while (0);

   return vosStatus;
}



/*                                                                           

                                                          

                                                                  
                                                                   
               

                                                          

                                                                     

                                                                     
                        

                                                                       

                       

                                                                             */
VOS_STATUS vos_packet_close( v_PVOID_t pVosContext )
{

   VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_INFO, "Enter:%s",__FUNCTION__);

   if (unlikely(NULL == pVosContext))
   {
      VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_FATAL,
                "VPKT [%d]: NULL pVosContext", __LINE__);
      return VOS_STATUS_E_INVAL;
   }

   if (unlikely(gpVosPacketContext->vosContext != pVosContext))
   {
      VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_FATAL,
                "VPKT [%d]: invalid pVosContext", __LINE__);
      return VOS_STATUS_E_INVAL;
   }

   mutex_lock(&gpVosPacketContext->mlock);
   (void) vos_pkti_list_destroy(&gpVosPacketContext->txMgmtFreeList);
   (void) vos_pkti_list_destroy(&gpVosPacketContext->txDataFreeList);
   (void) vos_pkti_list_destroy(&gpVosPacketContext->rxRawFreeList);
   (void) vos_pkti_list_destroy(&gpVosPacketContext->rxReplenishList);
   mutex_unlock(&gpVosPacketContext->mlock);

#ifdef WLAN_SOFTAP_FEATURE
   gpVosPacketContext->uctxDataFreeListCount = 0;
#endif

   return VOS_STATUS_SUCCESS;
}


/*                                                                           

                                                 

                                                            

                                                                        
                                                                      
                                                           

                                                                            
         
                                                                         
                                     

                                                                             
                                       
          

                                                            

                                                      

                                                                          
                                          
         
                                                                    

                                                                  
          

                                                                           
                                      

                                                                          
                                                                         
                                                                         
                   

                                                                      
                                                       

                                                                           
                                                                      

                                                                        
                                                                   
                                                                   
                                             

                                                                        
                                                                     
                                                                      
                                                                       

                                                                          
                                                                       
                                                                           
                                                                        
                                                                       
                             

                                                                        
                                                               
                                                                             

                                                                     
                                                                 
                                                                 
                                                                        
                               

                                                                         

     

                                                                          */
VOS_STATUS vos_pkt_get_packet( vos_pkt_t **ppPacket,
                               VOS_PKT_TYPE pktType,
                               v_SIZE_t dataSize,
                               v_SIZE_t numPackets,
                               v_BOOL_t zeroBuffer,
                               vos_pkt_get_packet_callback callback,
                               v_VOID_t *userData )
{
   struct list_head *pPktFreeList;
   vos_pkt_low_resource_info *pLowResourceInfo;
   struct vos_pkt_t *pVosPacket;
   //                                      
   if (unlikely(NULL == ppPacket))
   {
      VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_FATAL,
                "VPKT [%d]: NULL ppPacket", __LINE__);
      return VOS_STATUS_E_INVAL;
   }

   //                                                                
   if (unlikely(1 != numPackets))
   {
      VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR,
                "VPKT [%d]: invalid numPackets, %d", __LINE__, numPackets);
      return VOS_STATUS_E_INVAL;
   }

   //                                      
   if (unlikely((0 == dataSize) || (dataSize > VPKT_SIZE_BUFFER)))
   {
      VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR,
                "VPKT [%d]: invalid dataSize, %d", __LINE__, dataSize);
      return VOS_STATUS_E_INVAL;
   }

   //                                                                  
   //                                                      
   //                                              
   switch (pktType)
   {

   case VOS_PKT_TYPE_RX_RAW:
      pPktFreeList = &gpVosPacketContext->rxRawFreeList;
      pLowResourceInfo = &gpVosPacketContext->rxRawLowResourceInfo;

      //                                            
      vos_pkti_replenish_raw_pool();

      break;

   case VOS_PKT_TYPE_TX_802_11_MGMT:
      pPktFreeList = &gpVosPacketContext->txMgmtFreeList;
      pLowResourceInfo = &gpVosPacketContext->txMgmtLowResourceInfo;
      break;

   default:
      VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR,
                "VPKT [%d]: invalid packet type %d[%s]",
                __LINE__, pktType, vos_pkti_packet_type_str(pktType));
      return VOS_STATUS_E_INVAL;
   }

   //                                                                   
   //                                                                       
   //                                                        
   if (unlikely(pLowResourceInfo->callback))
   {
      VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR,
                "VPKT [%d]: Low resource handler already registered",
                __LINE__);
      return VOS_STATUS_E_ALREADY;
   }

   mutex_lock(&gpVosPacketContext->mlock);
   //                                                   
   if (unlikely(list_empty(pPktFreeList)))
   {
      //                  
      //                                   
      if (unlikely(NULL == callback))
      {
         VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR,
                   "VPKT [%d]: Low resource condition and no callback provided",
                   __LINE__);
         mutex_unlock(&gpVosPacketContext->mlock);

         return VOS_STATUS_E_FAILURE;
      }

      //                                                            
      //                                         
      pLowResourceInfo->callback   = callback;
      pLowResourceInfo->userData   = userData;
      VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_WARN,
                "VPKT [%d]: Low resource condition for packet type %d[%s]",
                __LINE__, pktType, vos_pkti_packet_type_str(pktType));
      mutex_unlock(&gpVosPacketContext->mlock);

      return VOS_STATUS_E_RESOURCES;
   }

   //                                           
   pVosPacket = list_first_entry(pPktFreeList, struct vos_pkt_t, node);
   list_del(&pVosPacket->node);
   mutex_unlock(&gpVosPacketContext->mlock);

   //                                                      
   memset(&pVosPacket->pvUserData, 0, sizeof(pVosPacket->pvUserData));

   //                                        
   pVosPacket->pNext = NULL;

   //                     
   pVosPacket->packetType = pktType;

   //                          
   pVosPacket->timestamp = vos_timer_get_system_ticks();

   //                                                             
   if (unlikely(zeroBuffer))
   {
      memset(pVosPacket->pSkb->head,
             0,
             skb_end_pointer(pVosPacket->pSkb) - pVosPacket->pSkb->head);
   }

   VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_INFO,
             "VPKT [%d]: [%p] Packet allocated, type %d[%s]",
             __LINE__, pVosPacket, pktType, vos_pkti_packet_type_str(pktType));

   *ppPacket = pVosPacket;
   return VOS_STATUS_SUCCESS;
}


/*                                                                           

                                                                           
                    

                                                                        
                                                     

                                                                             
                                                                          

                                                                        
                                                                      
                                                           

                                                                            
         
                                                                           
                        

                                                                             
                        
          

                                                                               
                                                                           
                                                                              
                                                             
                                                                  

                                                                           
                                      

                                                                          
                                                                        
                                                                       

                                                                         
                                                                     
                                                                            
                                                                        
                                                                             

                                                                      
                                                       

                                                                           
                                                                      

                                                                        
                                                                   
                              

                                                                        
                                                                     
                                                                      
                                                                       

                                                                          
                                                                       
                                                                           
                                                                        
                                                                       
                             

                                                                        
                                                               
                                                                             

                                                                     
                                                                 
                                                                 
                                                                        
                               

                                                                         
                   

                             

                                                                          */
VOS_STATUS vos_pkt_wrap_data_packet( vos_pkt_t **ppPacket,
                                     VOS_PKT_TYPE pktType,
                                     v_VOID_t *pOSPacket,
                                     vos_pkt_get_packet_callback callback,
                                     v_VOID_t *userData )
{
   struct list_head *pPktFreeList;
   vos_pkt_low_resource_info *pLowResourceInfo;
   struct vos_pkt_t *pVosPacket;

   //                                      
   if (unlikely(NULL == ppPacket))
   {
      VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_FATAL,
                "VPKT [%d]: NULL ppPacket", __LINE__);
      return VOS_STATUS_E_INVAL;
   }

   //                                                               
   //                                                            
   //                      
   if (unlikely(VOS_PKT_TYPE_TX_802_3_DATA != pktType))
   {
      VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR,
                "VPKT [%d]: invalid pktType", __LINE__, pktType);
      return VOS_STATUS_E_INVAL;
   }

   //                                                                  
   pPktFreeList = &gpVosPacketContext->txDataFreeList;
   pLowResourceInfo = &gpVosPacketContext->txDataLowResourceInfo;

   //                                                                   
   //                                                                       
   //                                                        
   if (unlikely(pLowResourceInfo->callback))
   {
      VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR,
                "VPKT [%d]: Low resource handler already registered",
                __LINE__);
      return VOS_STATUS_E_ALREADY;
   }

   //                                                   
   if (unlikely(list_empty(pPktFreeList)))
   {
      //                  
      //                                   
      if (unlikely(NULL == callback))
      {
         VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR,
                   "VPKT [%d]: Low resource condition and no callback provided",
                   __LINE__);
         return VOS_STATUS_E_FAILURE;
      }

      //                                                            
      //                                         
      pLowResourceInfo->callback   = callback;
      pLowResourceInfo->userData   = userData;
      VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_WARN,
                "VPKT [%d]: Low resource condition for pool %s",
                __LINE__, vos_pkti_packet_type_str(pktType));
      return VOS_STATUS_E_RESOURCES;
   }

   //                                           
   pVosPacket = list_first_entry(pPktFreeList, struct vos_pkt_t, node);
   list_del(&pVosPacket->node);
#ifdef WLAN_SOFTAP_FEATURE
   gpVosPacketContext->uctxDataFreeListCount --;
#endif

   //                                                      
   memset(&pVosPacket->pvUserData, 0, sizeof(pVosPacket->pvUserData));

   //                                        
   pVosPacket->pNext = NULL;

   //                     
   pVosPacket->packetType = pktType;

   //                    
   pVosPacket->pSkb = (struct sk_buff *) pOSPacket;

   //                          
   pVosPacket->timestamp = vos_timer_get_system_ticks();

   VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_INFO,
             "VPKT [%d]: [%p] Packet allocated, type %s",
             __LINE__, pVosPacket, vos_pkti_packet_type_str(pktType));

   *ppPacket = pVosPacket;
   return VOS_STATUS_SUCCESS;
}



/*                                                                           

                                                                         

                                                                       
                                                                         
                           

                                                                         
                                                                             
                                                                     

                                                                          
                                                                            
                                                                          
                                                                        
                                                 

                                                                

                                                                               
                                                                           
                                                                              
                                                             
                                                                  

                                                                        
                                                                          
                                                                               
                                                                  
                                       

                                                                            
                         

                                                                        
                                                       
                                                              

                                                                        

                             

                                                                             */
VOS_STATUS vos_pkt_set_os_packet( vos_pkt_t *pPacket,
                                  v_VOID_t *pOSPacket )
{
   //                                      
   if (unlikely((NULL == pPacket)||(NULL == pOSPacket)))
   {
      VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_FATAL,
                "VPKT [%d]: NULL pointer", __LINE__);
      return VOS_STATUS_E_INVAL;
   }

   //                                                    
   if (unlikely(VPKT_MAGIC_NUMBER != pPacket->magic))
   {
      VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_FATAL,
                "VPKT [%d]: Invalid magic", __LINE__);
      return VOS_STATUS_E_INVAL;
   }

   //                                                               
   //                                                            
   //                      
   if (unlikely(VOS_PKT_TYPE_TX_802_3_DATA != pPacket->packetType))
   {
      VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR,
                "VPKT [%d]: invalid packet type %d[%s]",
                __LINE__, pPacket->packetType,
                vos_pkti_packet_type_str(pPacket->packetType));
      return VOS_STATUS_E_INVAL;
   }

   //                                                                   
   if (unlikely(pPacket->pSkb))
   {
      VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_WARN,
                "VPKT [%d]: Packet previously attached", __LINE__);
   }

   //       
   pPacket->pSkb = (struct sk_buff *) pOSPacket;
   
   return VOS_STATUS_SUCCESS;
}


/*                                                                           

                                                                         

                                                                  
                                                                         
                           

                                                                

                                                                           
                                                                             
                                                                            
                         

                                                                            
                                                                            
                                                                             
                                                                          
                                                         

                                                                         
                                                                    
                     

                                                                        
                                                       
                                                              

                                                                             
                   

                                                          
                             

                                                                             */
VOS_STATUS vos_pkt_get_os_packet( vos_pkt_t *pPacket,
                                  v_VOID_t **ppOSPacket,
                                  v_BOOL_t clearOSPacket )
{
   //                                                 
   if (unlikely((NULL == pPacket)||(NULL == ppOSPacket)))
   {
      VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_FATAL,
                "VPKT [%d]: NULL pointer", __LINE__);
      return VOS_STATUS_E_INVAL;
   }

   //                                                    
   if (unlikely(VPKT_MAGIC_NUMBER != pPacket->magic))
   {
      VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_FATAL,
                "VPKT [%d]: Invalid magic", __LINE__);
      return VOS_STATUS_E_INVAL;
   }

   //                      
   *ppOSPacket = (v_VOID_t *) pPacket->pSkb;

   //          
   if (clearOSPacket)
   {
      pPacket->pSkb = NULL;
   }

   return VOS_STATUS_SUCCESS;
}

/*                                                                           

                                                                         
                                                       

                                                                      
                                                                       
                                                                        
                                                                        
            

                                                                          

                                                                          
                         

                                                                     
                                
                                        
                                               
                                                 
                                            

                                                                           
                           

                    

     

                                                                             */
v_VOID_t vos_pkt_get_user_data_ptr( vos_pkt_t *pPacket,
                                    VOS_PKT_USER_DATA_ID userID,
                                    v_VOID_t **ppUserData )
{
   //                                                 
   if (unlikely(NULL == pPacket))
   {
       VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_FATAL,
               "VPKT [%d]: NULL pointer", __LINE__);
       if (ppUserData != NULL)
       {
           *ppUserData = NULL;
       }
       return;
   }

   //                                                    
   if (unlikely(VPKT_MAGIC_NUMBER != pPacket->magic))
   {
      VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_FATAL,
                "VPKT [%d]: Invalid magic", __LINE__);
      *ppUserData = NULL;
      return;
   }

   //                
   if (unlikely(userID >= VOS_PKT_USER_DATA_ID_MAX))
   {
      VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR,
                "VPKT [%d]: Invalid user ID [%d]", __LINE__, userID);
      *ppUserData = NULL;
      return;
   }

   //                                                       
   //                         
   *ppUserData = pPacket->pvUserData[userID];

   return;
}


/*                                                                           

                                                                          
                                              

                                                                     
                                                                       
                                                                        
                                                                        
            

                                                                           
                                                                 

                                                           

                                                                          
                         

                                                                     
                                
                                        
                                               
                                                 
                                            

                                                                           
                             

                    

     

                                                                             */
v_VOID_t vos_pkt_set_user_data_ptr( vos_pkt_t *pPacket,
                                    VOS_PKT_USER_DATA_ID userID,
                                    v_VOID_t *pUserData )
{
   //                                     
   if (unlikely(NULL == pPacket))
   {
      VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_FATAL,
                "VPKT [%d]: NULL pointer", __LINE__);
      return;
   }

   //                                                       
   if (unlikely(VPKT_MAGIC_NUMBER != pPacket->magic))
   {
      VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_FATAL,
                "VPKT [%d]: Invalid magic", __LINE__);
      return;
   }

   //                
   if (unlikely(userID >= VOS_PKT_USER_DATA_ID_MAX))
   {
      VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR,
                "VPKT [%d]: Invalid user ID [%d]", __LINE__, userID);
      return;
   }

   //                                                       
   //                         
   pPacket->pvUserData[userID] = pUserData;

   return;
}

/*                                                                           

                                                                       

                                                                       

                                                                         
                                                                
                                                                        
                                                            

                                                                          
                                                       

         

     

                                                                             */
VOS_STATUS vos_pkt_return_packet( vos_pkt_t *pPacket )
{
   vos_pkt_t *pNext;
   struct list_head *pPktFreeList;
   vos_pkt_low_resource_info *pLowResourceInfo;
   vos_pkt_get_packet_callback callback;
   v_SIZE_t *pCount;
   VOS_PKT_TYPE packetType = VOS_PKT_TYPE_TX_802_3_DATA;

   //                                     
   if (unlikely(NULL == pPacket))
   {
      return VOS_STATUS_E_INVAL;
   }

   //                                        
   while (pPacket)
   {
      //                                  
      pNext = pPacket->pNext;
      pPacket->pNext = NULL;

      //                                                    
      if (unlikely(VPKT_MAGIC_NUMBER != pPacket->magic))
      {
         VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_FATAL,
                   "VPKT [%d]: Invalid magic", __LINE__);
         return VOS_STATUS_E_INVAL;
      }

      //                                                   
      if (pPacket->pSkb)
      {
         pPacket->pSkb->len = 0;
         pPacket->pSkb->data = pPacket->pSkb->head;
         skb_reset_tail_pointer(pPacket->pSkb);
         skb_reserve(pPacket->pSkb, VPKT_SIZE_BUFFER);
      }

      pCount = NULL;
      //                                                                  
      switch (pPacket->packetType)
      {
      case VOS_PKT_TYPE_RX_RAW:
         //                                                        
         //                                                          
         //               
         if (pPacket->pSkb)
         {
            pPktFreeList = &gpVosPacketContext->rxRawFreeList;
            pLowResourceInfo = &gpVosPacketContext->rxRawLowResourceInfo;
         }
         else
         {
            pPktFreeList = &gpVosPacketContext->rxReplenishList;
            pLowResourceInfo = NULL;
            pCount = &gpVosPacketContext->rxReplenishListCount;
         }
         packetType = VOS_PKT_TYPE_RX_RAW;
         break;

      case VOS_PKT_TYPE_TX_802_11_MGMT:
                
         pPktFreeList = &gpVosPacketContext->txMgmtFreeList;
         pLowResourceInfo = &gpVosPacketContext->txMgmtLowResourceInfo;
         
         break;

      case VOS_PKT_TYPE_TX_802_3_DATA:
         pPktFreeList = &gpVosPacketContext->txDataFreeList;
         pLowResourceInfo = &gpVosPacketContext->txDataLowResourceInfo;
#ifdef WLAN_SOFTAP_FEATURE
         gpVosPacketContext->uctxDataFreeListCount ++;
#endif
         break;

      default:
         VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_FATAL,
                   "VPKT [%d]: invalid packet type %d[%s]",
                   __LINE__, pPacket->packetType,
                   vos_pkti_packet_type_str(pPacket->packetType));

         return VOS_STATUS_E_INVAL;
      }


      //                                                                
      if (pLowResourceInfo && pLowResourceInfo->callback)
      {
         //        
         VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_INFO,"VPKT [%d]: recycle %p",  __LINE__, pPacket);

         //                                                             
         //                                         
         VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_INFO,
                   "VPKT [%d]: [%p] Packet recycled, type %d[%s]",
                   __LINE__, pPacket, pPacket->packetType,
                   vos_pkti_packet_type_str(pPacket->packetType));

         //                                                      
         memset(&pPacket->pvUserData, 0, sizeof(pPacket->pvUserData));

         //                                        
         pPacket->pNext = NULL;

         //                          
         pPacket->timestamp = vos_timer_get_system_ticks();

         callback = pLowResourceInfo->callback;
         pLowResourceInfo->callback = NULL;
         callback(pPacket, pLowResourceInfo->userData);
      }
      else
      {
         //                                                      
         //                                            
         VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_INFO,
                   "VPKT [%d]: [%p] Packet returned, type %d[%s]",
                   __LINE__, pPacket, pPacket->packetType,
                   vos_pkti_packet_type_str(pPacket->packetType));
         mutex_lock(&gpVosPacketContext->mlock);
         list_add_tail(&pPacket->node, pPktFreeList);
         mutex_unlock(&gpVosPacketContext->mlock);
        
         if (pCount)
         {
            (*pCount)++;
         }
      }

      //                                 
      pPacket = pNext;

   } //                

   //                                            
   if (VOS_PKT_TYPE_RX_RAW == packetType)
   {
      vos_pkti_replenish_raw_pool();   
   }
   return VOS_STATUS_SUCCESS;
}

/*                                                                           

                                                                       

                                                                         
                                                                          
               

                                                       

                                                  

                                                                        
                                      
      
                                                         
                                                             
       

         

     

                                                                             */
VOS_STATUS vos_pkt_chain_packet( vos_pkt_t *pPacket,
                                 vos_pkt_t *pChainPacket,
                                 v_BOOL_t chainAfter )
{
   //                                
   if (unlikely((NULL == pPacket) || (NULL == pChainPacket)))
   {
      VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_FATAL,
                "VPKT [%d]: NULL pointer", __LINE__);
      return VOS_STATUS_E_INVAL;
   }

   //                                                       
   if (unlikely((VPKT_MAGIC_NUMBER != pPacket->magic) ||
                (VPKT_MAGIC_NUMBER != pChainPacket->magic)))
   {
      VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_FATAL,
                "VPKT [%d]: Invalid magic", __LINE__);
      return VOS_STATUS_E_INVAL;
   }

   //                                 
   if (unlikely(VOS_FALSE == chainAfter))
   {
      vos_pkt_t *pTmp = pPacket;
      pPacket = pChainPacket;
      pChainPacket = pTmp;
   }

   //                          
   while (pPacket->pNext)
   {
      pPacket = pPacket->pNext;
   }

   //       
   pPacket->pNext = pChainPacket;

   return VOS_STATUS_SUCCESS;
}


/*                                                                           

                                                                       
                                                       

                                                                           
                                                                         
                            

                                         

                                                                        
                                                              
                                                                     
                                          

                                                                           
                                                                      
                                                                              
                                                            

        
                                                     
                                                                        
                                                                        

                                                           
                                                                             
                                                                               
                                                                              
                                                   
         

                                                                       
                                                       

         

     

                                                                             */
VOS_STATUS vos_pkt_walk_packet_chain( vos_pkt_t *pPacket,
                                      vos_pkt_t **ppChainedPacket,
                                      v_BOOL_t unchainPacket )
{
   //                                
   if (unlikely((NULL == pPacket) || (NULL == ppChainedPacket)))
   {
      VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_FATAL,
                "VPKT [%d]: NULL pointer", __LINE__);
      return VOS_STATUS_E_INVAL;
   }

   //                                                       
   if (unlikely(VPKT_MAGIC_NUMBER != pPacket->magic))
   {
      VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_FATAL,
                "VPKT [%d]: Invalid magic", __LINE__);
      return VOS_STATUS_E_INVAL;
   }

   //                
   *ppChainedPacket = pPacket->pNext;

   //                                     
   if (VOS_FALSE != unchainPacket)
   {
      pPacket->pNext = NULL;
   }

   //                                                  
   if (*ppChainedPacket)
   {
      return VOS_STATUS_SUCCESS;
   }
   else
   {
      return VOS_STATUS_E_EMPTY;
   }
}

/*                                                                           

                                                                        

                                                                         
                                                         

                                                                       
                              

                                                                              

                                                                           
                                                                             
                                                                            
                                          

                                                                        
                          

                                               

         

     

                                                                             */
VOS_STATUS vos_pkt_get_data_vector( vos_pkt_t *pPacket,
                                    vos_pkt_data_vector_t *pVector,
                                    v_SIZE_t *pNumVectors )
{
   //              

   VOS_ASSERT(0);
   return VOS_STATUS_E_FAILURE;
}


/*                                                                           

                                                                    

                                                                      
                                                                      
                                                                    

                                                        

                                                                            
                                                                   

                                                                           
                        

                                                                             
                                                                          
                                          

                                                                        
                                                                       
                                                                      
                                                             

         

     

                                                                             */
VOS_STATUS vos_pkt_extract_data( vos_pkt_t *pPacket,
                                 v_SIZE_t pktOffset,
                                 v_VOID_t *pOutputBuffer,
                                 v_SIZE_t *pOutputBufferSize )
{
   v_SIZE_t len;
   struct sk_buff *skb;

   //                                
   if (unlikely((NULL == pPacket) ||
                (NULL == pOutputBuffer) ||
                (NULL == pOutputBufferSize)))
   {
      VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_FATAL,
                "VPKT [%d]: NULL pointer", __LINE__);
      return VOS_STATUS_E_INVAL;
   }

   //                                                       
   if (unlikely(VPKT_MAGIC_NUMBER != pPacket->magic))
   {
      VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_FATAL,
                "VPKT [%d]: Invalid magic", __LINE__);
      return VOS_STATUS_E_INVAL;
   }

   //                       
   skb = pPacket->pSkb;

   //                 
   if (unlikely(NULL == skb))
   {
      VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_FATAL,
                "VPKT [%d]: NULL skb", __LINE__);
      return VOS_STATUS_E_INVAL;
   }

   //                              
   len = *pOutputBufferSize;

   //                                                                    
   //                                                                
   //                                                                 
   //                                                                     
   //                  
   //
   //                                                                
   //                                                               
   //                                                                    
   //                        
   if (0 == len)
   {
      len = skb->len - pktOffset;

      //                         
      *pOutputBufferSize = len;
   }
   else
   {
      //                                                          
      if (len > (skb->len - pktOffset))
      {
         VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR,
                   "VPKT [%d]: Request overrun, "
                   "req offset %d, req size %d, packet size %d",
                   __LINE__, pktOffset, len, skb->len);
         return VOS_STATUS_E_INVAL;
      }
   }

   //              
   memcpy(pOutputBuffer, &skb->data[pktOffset], len);

   return VOS_STATUS_SUCCESS;
}

/*                                                                           

                                                                              

                                                                        
                                                                     
                                                                     
                                         

                                                                    
                      

                                                                           
                        

                                                                          
                                                                             
                                                            

                                                                       
                                                                          
                         

                                                                           
                                 

                                                                           
                                                                           
                                                                  

                            

                                                                             */
VOS_STATUS vos_pkt_extract_data_chain( vos_pkt_t *pPacket,
                                       v_VOID_t *pOutputBuffer,
                                       v_SIZE_t *pOutputBufferSize )
{
   VOS_STATUS vosStatus;
   v_SIZE_t len;
   struct sk_buff *skb;

   //                                
   if (unlikely((NULL == pPacket) ||
                (NULL == pOutputBuffer) ||
                (NULL == pOutputBufferSize)))
   {
      VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_FATAL,
                "VPKT [%d]: NULL pointer", __LINE__);
      return VOS_STATUS_E_INVAL;
   }

   //                                                       
   if (unlikely(VPKT_MAGIC_NUMBER != pPacket->magic))
   {
      VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_FATAL,
                "VPKT [%d]: Invalid magic", __LINE__);
      return VOS_STATUS_E_INVAL;
   }

   //                                           
   vosStatus = vos_pkt_get_packet_chain_length(pPacket, &len);
   if (unlikely(VOS_STATUS_SUCCESS != vosStatus))
   {
      VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR,
                "VPKT [%d]: Unable to get packet chain length", __LINE__);
      return VOS_STATUS_E_FAILURE;
   }

   //                                                                
   //                                             
   if (len > *pOutputBufferSize)
   {
      *pOutputBufferSize = len;
      return VOS_STATUS_E_NOMEM;
   }

   //                                                        
   while (pPacket)
   {
      //                       
      skb = pPacket->pSkb;

      //                 
      if (unlikely(NULL == skb))
      {
         VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_FATAL,
                   "VPKT [%d]: NULL skb", __LINE__);
         return VOS_STATUS_E_INVAL;
      }

      memcpy(pOutputBuffer, skb->data, skb->len);
      pOutputBuffer += skb->len;

      pPacket = pPacket->pNext;
   }

   return VOS_STATUS_SUCCESS;
}

/*                                                                           

                                                                    

                                                                       
                                                                            
                                                                     
                

                                                                      
                                                                        
                                                                           
         

                                               

                                                                       

                                                                        
                                                                  

                                                                     

         

     

                                                                             */
VOS_STATUS vos_pkt_peek_data( vos_pkt_t *pPacket,
                              v_SIZE_t pktOffset,
                              v_VOID_t **ppPacketData,
                              v_SIZE_t numBytes )
{
   struct sk_buff *skb;

   //                                
   if (unlikely((NULL == pPacket) ||
                (NULL == ppPacketData)))
   {
      VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_FATAL,
                "VPKT [%d]: NULL pointer", __LINE__);
      return VOS_STATUS_E_INVAL;
   }

   //                                                       
   if (unlikely(VPKT_MAGIC_NUMBER != pPacket->magic))
   {
      VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_FATAL,
                "VPKT [%d]: Invalid magic", __LINE__);
      return VOS_STATUS_E_INVAL;
   }

   //                  
   if (unlikely(0 == numBytes))
   {
      VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_FATAL,
                "VPKT [%d]: Invalid numBytes", __LINE__);
      return VOS_STATUS_E_INVAL;
   }

   //                       
   skb = pPacket->pSkb;

   //                 
   if (unlikely(NULL == skb))
   {
      VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_FATAL,
                "VPKT [%d]: NULL skb", __LINE__);
      return VOS_STATUS_E_INVAL;
   }

   //                   
   if (unlikely((pktOffset + numBytes) > skb->len))
   {
      VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR,
                "VPKT [%d]: Packet overflow, offset %d size %d len %d",
                __LINE__, pktOffset, numBytes, skb->len);
      return VOS_STATUS_E_INVAL;
   }

   //                                     
   *ppPacketData = &skb->data[pktOffset];
   return VOS_STATUS_SUCCESS;
}


/*                                                                           

                                                                      

                                                     

                                                               

                                                                             

         

     

                                                                             */
VOS_STATUS vos_pkt_get_packet_type( vos_pkt_t *pPacket,
                                    VOS_PKT_TYPE *pPacketType )
{
   //                                
   if (unlikely((NULL == pPacket) ||
                (NULL == pPacketType)))
   {
      VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_FATAL,
                "VPKT [%d]: NULL pointer", __LINE__);
      return VOS_STATUS_E_INVAL;
   }

   //                                                       
   if (unlikely(VPKT_MAGIC_NUMBER != pPacket->magic))
   {
      VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_FATAL,
                "VPKT [%d]: Invalid magic", __LINE__);
      return VOS_STATUS_E_INVAL;
   }

   //                                 
   *pPacketType = pPacket->packetType;
   return VOS_STATUS_SUCCESS;
}



/*                                                                           

                                                                          

                                                                 

                                                                 

                                                                              
                                          
         

     

                                                                             */
VOS_STATUS vos_pkt_get_packet_length( vos_pkt_t *pPacket,
                                      v_U16_t *pPacketSize )
{
   //                                
   if (unlikely((NULL == pPacket) ||
                (NULL == pPacketSize)))
   {
      VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_FATAL,
                "VPKT [%d]: NULL pointer", __LINE__);
      return VOS_STATUS_E_INVAL;
   }

   //                                                       
   if (unlikely(VPKT_MAGIC_NUMBER != pPacket->magic))
   {
      VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_FATAL,
                "VPKT [%d]: Invalid magic", __LINE__);
      return VOS_STATUS_E_INVAL;
   }

   //                 
   if (unlikely(NULL == pPacket->pSkb))
   {
      VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_FATAL,
                "VPKT [%d]: NULL skb", __LINE__);
      return VOS_STATUS_E_INVAL;
   }

   //                                 
   *pPacketSize = pPacket->pSkb->len;
   return VOS_STATUS_SUCCESS;
}


/*                                                                           

                                                                             

                                                                       

                                                                              
                                                                           
                     

                                                                              
                                          
         

     

                                                                             */
VOS_STATUS vos_pkt_get_packet_chain_length( vos_pkt_t *pPacketChain,
                                            v_SIZE_t *pPacketChainSize )
{
   v_SIZE_t chainSize = 0;

   //                                
   if (unlikely((NULL == pPacketChain) ||
                (NULL == pPacketChainSize)))
   {
      VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_FATAL,
                "VPKT [%d]: NULL pointer", __LINE__);
      return VOS_STATUS_E_INVAL;
   }

   //                                                         
   while (pPacketChain)
   {

      //                                                       
      if (unlikely(VPKT_MAGIC_NUMBER != pPacketChain->magic))
      {
         VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_FATAL,
                   "VPKT [%d]: Invalid magic", __LINE__);
         return VOS_STATUS_E_INVAL;
      }

      //                 
      if (unlikely(NULL == pPacketChain->pSkb))
      {
         VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_FATAL,
                   "VPKT [%d]: NULL skb", __LINE__);
         return VOS_STATUS_E_INVAL;
      }

      chainSize += pPacketChain->pSkb->len;
      pPacketChain = pPacketChain->pNext;
   }

   //              
   *pPacketChainSize = chainSize;
   return VOS_STATUS_SUCCESS;
}


/*                                                                           

                                                                        

                                                                            
                                                                     

                                             

                                                                              

                                                                         
                           

         

     

                                                                             */
VOS_STATUS vos_pkt_push_head( vos_pkt_t *pPacket,
                              v_VOID_t *pData,
                              v_SIZE_t dataSize )
{
   struct sk_buff *skb;

   //                                
   if (unlikely((NULL == pPacket) || (NULL == pData)))
   {
      VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_FATAL,
                "VPKT [%d]: NULL pointer", __LINE__);
      return VOS_STATUS_E_INVAL;
   }

   //                                                       
   if (unlikely(VPKT_MAGIC_NUMBER != pPacket->magic))
   {
      VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_FATAL,
                "VPKT [%d]: Invalid magic", __LINE__);
      return VOS_STATUS_E_INVAL;
   }

   //                       
   skb = pPacket->pSkb;

   //                 
   if (unlikely(NULL == skb))
   {
      VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_FATAL,
                "VPKT [%d]: NULL skb", __LINE__);
      return VOS_STATUS_E_INVAL;
   }

   //                                                               
   //                                                                 
   //                                                
   if (unlikely(skb_headroom(skb) < dataSize))
   {
      VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_FATAL,
                "VPKT [%d]: Insufficient headroom, "
                "head[%p], data[%p], req[%d]",
                __LINE__, skb->head, skb->data, dataSize);
      return VOS_STATUS_E_INVAL;
   }

   //                       
   memcpy(skb_push(skb, dataSize), pData, dataSize);

   return VOS_STATUS_SUCCESS;
}

/*                                                                           

                                                                             

                                                                            
                                                                              
                                                                       
                                    

                                                                       
           

                                      

                                             

                                                                           
                                                                            
                                                                             
                                                 

                                                                           
                                                                           
                                                    

         

     

                                                                             */
VOS_STATUS vos_pkt_reserve_head( vos_pkt_t *pPacket,
                                 v_VOID_t **ppData,
                                 v_SIZE_t dataSize )
{
   struct sk_buff *skb;
   struct sk_buff *newskb;

   //                                
   if (unlikely((NULL == pPacket) || (NULL == ppData)))
   {
      VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_FATAL,
                "VPKT [%d]: NULL pointer", __LINE__);
      return VOS_STATUS_E_INVAL;
   }

   //                                                       
   if (unlikely(VPKT_MAGIC_NUMBER != pPacket->magic))
   {
      VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_FATAL,
                "VPKT [%d]: Invalid magic", __LINE__);
      return VOS_STATUS_E_INVAL;
   }

   //                       
   skb = pPacket->pSkb;

   //                 
   if (unlikely(NULL == skb))
   {
      VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_FATAL,
                "VPKT [%d]: NULL skb", __LINE__);
      return VOS_STATUS_E_INVAL;
   }

   //                                                               
   //                                                                 
   //                                                
   if (unlikely(skb_headroom(skb) < dataSize))
   {
      VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_WARN,
                "VPKT [%d]: Insufficient headroom, "
                "head[%p], data[%p], req[%d]",
                __LINE__, skb->head, skb->data, dataSize);
    
      if ((newskb = skb_realloc_headroom(skb, dataSize)) == NULL) {
         VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_FATAL,
                   "VPKT [%d]: Failed to realloc headroom", __LINE__);
         return VOS_STATUS_E_INVAL;
      }

      kfree_skb(skb);
      skb = newskb;

      //                    
      pPacket->pSkb = newskb;
   }

   //                               
   *ppData = skb_push(skb, dataSize);
   //                                                  
   memset( (void *)*ppData, 0, dataSize );

   return VOS_STATUS_SUCCESS;
}


/*                                                                           

                                                                                  

                                                                            
                                                                              
                                                                       
                                    

                                                                       
           
 
                                                  
 
                                      

                                             

                                                                           
                                                                            
                                                                             
                                                 

                                                                           
                                                                           
                                                    

         

     

                                                                             */
VOS_STATUS vos_pkt_reserve_head_fast( vos_pkt_t *pPacket,
                                 v_VOID_t **ppData,
                                 v_SIZE_t dataSize )
{
   struct sk_buff *skb;
   struct sk_buff *newskb;

   //                       
   skb = pPacket->pSkb;

   //                 
   if (unlikely(NULL == skb))
   {
      VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_FATAL,
                "VPKT [%d]: NULL skb", __LINE__);
      return VOS_STATUS_E_INVAL;
   }

   //                                                               
   //                                                                 
   //                                                
   if (unlikely(skb_headroom(skb) < dataSize))
   {
      VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_WARN,
                "VPKT [%d]: Insufficient headroom, "
                "head[%p], data[%p], req[%d]",
                __LINE__, skb->head, skb->data, dataSize);
    
      if ((newskb = skb_realloc_headroom(skb, dataSize)) == NULL) {
         VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_FATAL,
                   "VPKT [%d]: Failed to realloc headroom", __LINE__);
         return VOS_STATUS_E_INVAL;
      }

      kfree_skb(skb);
      skb = newskb;

      //                    
      pPacket->pSkb = newskb;
   }

   //                               
   *ppData = skb_push(skb, dataSize);
   //                                                  
   //                                       

   return VOS_STATUS_SUCCESS;
}

/*                                                                           

                                                                           

                                                                     
                                                                

                                                 

                                                                           
                                       

                                                                           
                                                                          
                                               

         

     

                                                                             */
VOS_STATUS vos_pkt_pop_head( vos_pkt_t *pPacket,
                             v_VOID_t *pData,
                             v_SIZE_t dataSize )
{
   struct sk_buff *skb;

   //                                
   if (unlikely((NULL == pPacket) || (NULL == pData)))
   {
      VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_FATAL,
                "VPKT [%d]: NULL pointer", __LINE__);
      return VOS_STATUS_E_INVAL;
   }

   //                                                       
   if (unlikely(VPKT_MAGIC_NUMBER != pPacket->magic))
   {
      VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_FATAL,
                "VPKT [%d]: Invalid magic", __LINE__);
      return VOS_STATUS_E_INVAL;
   }

   //                       
   skb = pPacket->pSkb;

   //                 
   if (unlikely(NULL == skb))
   {
      VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_FATAL,
                "VPKT [%d]: NULL skb", __LINE__);
      return VOS_STATUS_E_INVAL;
   }

   //                                      
   if (unlikely(skb->len < dataSize))
   {
      VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_FATAL,
                "VPKT [%d]: pop exceeds packet size, len[%d], req[%d]",
                __LINE__, skb->len, dataSize);
      return VOS_STATUS_E_INVAL;
   }

   //              
   memcpy(pData, skb->data, dataSize);
   skb_pull(skb, dataSize);

   return VOS_STATUS_SUCCESS;
}


/*                                                                           

                                                                            

                                                                         
                                                                            
                                                                           
                                                                            
              

                                                                

                                                 

                                                                        
                           

                                                                   
                                                                           
                                                               
                                                                     

         

     

                                                                             */
VOS_STATUS vos_pkt_trim_head( vos_pkt_t *pPacket,
                              v_SIZE_t dataSize )
{
   struct sk_buff *skb;

   //                                
   if (unlikely(NULL == pPacket))
   {
      VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_FATAL,
                "VPKT [%d]: NULL pointer", __LINE__);
      return VOS_STATUS_E_INVAL;
   }

   //                                                       
   if (unlikely(VPKT_MAGIC_NUMBER != pPacket->magic))
   {
      VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_FATAL,
                "VPKT [%d]: Invalid magic", __LINE__);
      return VOS_STATUS_E_INVAL;
   }

   //                       
   skb = pPacket->pSkb;

   //                 
   if (unlikely(NULL == skb))
   {
      VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_FATAL,
                "VPKT [%d]: NULL skb", __LINE__);
      return VOS_STATUS_E_INVAL;
   }

   //                                       
   if (unlikely(skb->len < dataSize))
   {
      VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_FATAL,
                "VPKT [%d]: trim exceeds packet size, len[%d], req[%d]",
                __LINE__, skb->len, dataSize);
      return VOS_STATUS_E_INVAL;
   }

   //               
   skb_pull(skb, dataSize);

   return VOS_STATUS_SUCCESS;
}

/*                                                                           

                                                                      

                                                                          
                                                                       

                                             

                                                                              

                                                                                 

         

     

                                                                             */
VOS_STATUS vos_pkt_push_tail( vos_pkt_t *pPacket,
                              v_VOID_t *pData,
                              v_SIZE_t dataSize )
{
   struct sk_buff *skb;

   //                                
   if (unlikely((NULL == pPacket) || (NULL == pData)))
   {
      VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_FATAL,
                "VPKT [%d]: NULL pointer", __LINE__);
      return VOS_STATUS_E_INVAL;
   }

   //                                                       
   if (unlikely(VPKT_MAGIC_NUMBER != pPacket->magic))
   {
      VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_FATAL,
                "VPKT [%d]: Invalid magic", __LINE__);
      return VOS_STATUS_E_INVAL;
   }

   //                       
   skb = pPacket->pSkb;

   //                 
   if (unlikely(NULL == skb))
   {
      VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_FATAL,
                "VPKT [%d]: NULL skb", __LINE__);
      return VOS_STATUS_E_INVAL;
   }

   //                                                               
   //                                                                
   //                                               
   if (unlikely(skb_tailroom(skb) < dataSize))
   {
      VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_FATAL,
                "VPKT [%d]: Insufficient tailroom, "
                "tail[%p], end[%p], req[%d]",
                __LINE__, skb_tail_pointer(skb),
                skb_end_pointer(skb), dataSize);
      return VOS_STATUS_E_INVAL;
   }

   //                       
   memcpy(skb_put(skb, dataSize), pData, dataSize);

   return VOS_STATUS_SUCCESS;
}


/*                                                                           

                                                                           

                                                                          
                                                                              
                                                                       
                                    

                                                                       
           

                                             

                                                                           
                                                                            
                                                                             
                                                 

                                                                           
                                                                           
                                                    

         

     

                                                                             */
VOS_STATUS vos_pkt_reserve_tail( vos_pkt_t *pPacket,
                                 v_VOID_t **ppData,
                                 v_SIZE_t dataSize )
{
   struct sk_buff *skb;

   //                                
   if (unlikely((NULL == pPacket) || (NULL == ppData)))
   {
      VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_FATAL,
                "VPKT [%d]: NULL pointer", __LINE__);
      return VOS_STATUS_E_INVAL;
   }

   //                                                       
   if (unlikely(VPKT_MAGIC_NUMBER != pPacket->magic))
   {
      VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_FATAL,
                "VPKT [%d]: Invalid magic", __LINE__);
      return VOS_STATUS_E_INVAL;
   }

   //                       
   skb = pPacket->pSkb;

   //                 
   if (unlikely(NULL == skb))
   {
      VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_FATAL,
                "VPKT [%d]: NULL skb", __LINE__);
      return VOS_STATUS_E_INVAL;
   }

   //                                                               
   //                                                                
   //                                               
   if (unlikely(skb_tailroom(skb) < dataSize))
   {
      VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_FATAL,
                "VPKT [%d]: Insufficient tailroom, "
                "tail[%p], end[%p], req[%d]",
                __LINE__, skb_tail_pointer(skb),
                skb_end_pointer(skb), dataSize);
      return VOS_STATUS_E_INVAL;
   }

   //                            
   *ppData = skb_put(skb, dataSize);

   return VOS_STATUS_SUCCESS;
}



/*                                                                           

                                                                         

                                                                   
                                                                

                                                 

                                                                           
                                       

                                                                          
                                                                          
                                               

         

     

                                                                             */
VOS_STATUS vos_pkt_pop_tail( vos_pkt_t *pPacket,
                             v_VOID_t *pData,
                             v_SIZE_t dataSize )
{
   struct sk_buff *skb;

   //                                
   if (unlikely((NULL == pPacket) || (NULL == pData)))
   {
      VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_FATAL,
                "VPKT [%d]: NULL pointer", __LINE__);
      return VOS_STATUS_E_INVAL;
   }

   //                                                       
   if (unlikely(VPKT_MAGIC_NUMBER != pPacket->magic))
   {
      VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_FATAL,
                "VPKT [%d]: Invalid magic", __LINE__);
      return VOS_STATUS_E_INVAL;
   }

   //                       
   skb = pPacket->pSkb;

   //                 
   if (unlikely(NULL == skb))
   {
      VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_FATAL,
                "VPKT [%d]: NULL skb", __LINE__);
      return VOS_STATUS_E_INVAL;
   }

   //                                      
   if (unlikely(skb->len < dataSize))
   {
      VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_FATAL,
                "VPKT [%d]: pop exceeds packet size, len[%d], req[%d]",
                __LINE__, skb->len, dataSize);
      return VOS_STATUS_E_INVAL;
   }

   //                                                          
   skb->tail -= dataSize;
   skb->len -= dataSize;

   //                       
   memcpy(pData, skb_tail_pointer(skb), dataSize);

   return VOS_STATUS_SUCCESS;
}

/*                                                                           

                                                                          

                                                                         
                                                                          
                                                                     
                                                                 

                                                                

                                                 

                                                                        
                           

                                                                   
                                                                           
                                                               
                                                                     

         

     

                                                                             */
VOS_STATUS vos_pkt_trim_tail( vos_pkt_t *pPacket,
                              v_SIZE_t dataSize )
{
   struct sk_buff *skb;

   //                                
   if (unlikely(NULL == pPacket))
   {
      VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_FATAL,
                "VPKT [%d]: NULL pointer", __LINE__);
      return VOS_STATUS_E_INVAL;
   }

   //                                                       
   if (unlikely(VPKT_MAGIC_NUMBER != pPacket->magic))
   {
      VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_FATAL,
                "VPKT [%d]: Invalid magic", __LINE__);
      return VOS_STATUS_E_INVAL;
   }

   //                       
   skb = pPacket->pSkb;

   //                 
   if (unlikely(NULL == skb))
   {
      VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_FATAL,
                "VPKT [%d]: NULL skb", __LINE__);
      return VOS_STATUS_E_INVAL;
   }

   //                                      
   if (unlikely(skb->len < dataSize))
   {
      VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_FATAL,
                "VPKT [%d]: pop exceeds packet size, len[%d], req[%d]",
                __LINE__, skb->len, dataSize);
      return VOS_STATUS_E_INVAL;
   }

   //                                                          
   skb->tail -= dataSize;
   skb->len -= dataSize;

   return VOS_STATUS_SUCCESS;
}


/*                                                                           

                                                                           
                       

                                                 

                                                       

                                                             

                                                              

     

                                                                             */
VOS_STATUS vos_pkt_get_timestamp( vos_pkt_t *pPacket,
                                  v_TIME_t* pTstamp )
{
   //                                
   if (unlikely((NULL == pPacket) ||
                (NULL == pTstamp)))
   {
      VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_FATAL,
                "VPKT [%d]: NULL pointer", __LINE__);
      return VOS_STATUS_E_INVAL;
   }

   //                                                       
   if (unlikely(VPKT_MAGIC_NUMBER != pPacket->magic))
   {
      VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_FATAL,
                "VPKT [%d]: Invalid magic", __LINE__);
      return VOS_STATUS_E_INVAL;
   }

   //                                 
   *pTstamp = pPacket->timestamp;
   return VOS_STATUS_SUCCESS;
}


/*                                                                           

                                                                      
                                                                          
                                                                         
                                                                       
                                                                             
                                                                               
                                                                             
                                                                               
                                  

                                                                    
                                                                          
                                                                 
                                                   

                                                          

                                                     

                                                   

                                                      

     

                                                                              */
VOS_STATUS vos_pkt_flatten_rx_pkt( vos_pkt_t **ppPacket )
{
   //                                                      
   return VOS_STATUS_SUCCESS;
}

/*                                                                           
  
                                                                       

                                                                                
                                                                  

                                                 

                                                                          


         

     

                                                                             */
VOS_STATUS vos_pkt_set_rx_length( vos_pkt_t *pPacket,
                                  v_SIZE_t pktLen )
{
   struct sk_buff *skb;

   //                                
   if (unlikely(NULL == pPacket))
   {
      VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_FATAL,
                "VPKT [%d]: NULL pointer", __LINE__);
      return VOS_STATUS_E_INVAL;
   }

   //                                                       
   if (unlikely(VPKT_MAGIC_NUMBER != pPacket->magic))
   {
      VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_FATAL,
                "VPKT [%d]: Invalid magic", __LINE__);
      return VOS_STATUS_E_INVAL;
   }

   //                       
   skb = pPacket->pSkb;

   //                 
   if (unlikely(NULL == skb))
   {
      VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_FATAL,
                "VPKT [%d]: NULL skb", __LINE__);
      return VOS_STATUS_E_INVAL;
   }

   //                                                          
   //                          
   skb_set_tail_pointer(skb, pktLen);
   skb->len   = pktLen;

   return VOS_STATUS_SUCCESS; 

}
/*                                                                           
  
                                                                            
                                        
                                                                              
           
   
                                                              
                                                                              
                                               
                                                                    
                                                             

                                                
  
                                                      

                                               
    
     
  
                                                                              */
VOS_STATUS vos_pkt_get_available_buffer_pool (VOS_PKT_TYPE  pktType,
                                              v_SIZE_t     *vosFreeBuffer)
{
   struct list_head *pList;
   struct list_head *pNode;
   v_SIZE_t count;
   if (NULL == vosFreeBuffer)
   {
      return VOS_STATUS_E_INVAL;
   }

   switch (pktType)
   {
   case VOS_PKT_TYPE_TX_802_11_MGMT:
      pList = &gpVosPacketContext->txMgmtFreeList;
      break;

   case VOS_PKT_TYPE_TX_802_11_DATA:
   case VOS_PKT_TYPE_TX_802_3_DATA:
#ifdef WLAN_SOFTAP_FEATURE
      if (VOS_STA_SAP_MODE == hdd_get_conparam())
      {
         *vosFreeBuffer = gpVosPacketContext->uctxDataFreeListCount;  
          return VOS_STATUS_SUCCESS;
      }
      else
#endif
      pList = &gpVosPacketContext->txDataFreeList;
      break;

   case VOS_PKT_TYPE_RX_RAW:
      //                                                            
      //                                                          
      //                                   
      vos_pkti_replenish_raw_pool();
      pList = &gpVosPacketContext->rxRawFreeList;
      break;

   default:
      return (VOS_STATUS_E_INVAL);
   }

   count = 0;
   mutex_lock(&gpVosPacketContext->mlock);
   list_for_each(pNode, pList)
   {
      count++;
   }
   mutex_unlock(&gpVosPacketContext->mlock);
   *vosFreeBuffer = count;
   return VOS_STATUS_SUCCESS;
}


#ifdef VOS_PACKET_UNIT_TEST
#include "vos_packet_test.c"
#endif
