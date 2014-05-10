/*
 * Copyright (c) 2012-2013, The Linux Foundation. All rights reserved.
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


#if !defined( __WLAN_QCT_PAL_PACKET_H )
#define __WLAN_QCT_PAL_PACKET_H

/*                                                                          
  
                              
  
                                                 
               
                                        
  
                                                                          */

#include "wlan_qct_pal_type.h"
#include "wlan_qct_pal_status.h"

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
//                                      
//                                                                
//                                       
//                             

//                                                            
//                                               
//                                                   
#define VPKT_SIZE_BUFFER  ((30 * 128) + 32)

typedef enum
{
   //                                                     
   eWLAN_PAL_PKT_TYPE_TX_802_11_MGMT,
   //                                               
   eWLAN_PAL_PKT_TYPE_TX_802_11_DATA,
   //                                              
   eWLAN_PAL_PKT_TYPE_TX_802_3_DATA,
   //                                                       
   eWLAN_PAL_PKT_TYPE_RX_RAW
} wpt_packet_type;


typedef struct swpt_packet
{
   /*
                                            
                                            
                                                                         
                       
   */
   void *pBD;
   //                                             
   void *pBDPhys; 
   //                                                     
   void *pOSStruct;  
   void *pktMetaInfo;
   wpt_packet_type pktType;
   //                                                                
   wpt_uint16 BDLength;

   //                                                      
   void *pInternalData; 
} wpt_packet;

typedef struct swpt_iterator
{
   void *pNext;  
   void *pCur;
   void *pContext;
} wpt_iterator;


//                               
#define WPAL_PACKET_SET_BD_POINTER(pPkt, pBd)   ( (pPkt)->pBD = (pBd) )
#define WPAL_PACKET_GET_BD_POINTER(pPkt)  ( (pPkt)->pBD )
//                                 
#define WPAL_PACKET_SET_BD_PHYS(pPkt, pBdPhys)   ( (pPkt)->pBDPhys = (pBdPhys) )
#define WPAL_PACKET_GET_BD_PHYS(pPkt)  ( (pPkt)->pBDPhys )
#define WPAL_PACKET_SET_BD_LENGTH(pPkt, len)   ( (pPkt)->BDLength = (len) )
#define WPAL_PACKET_GET_BD_LENGTH(pPkt)  ( (pPkt)->BDLength )
#define WPAL_PACKET_SET_METAINFO_POINTER(pPkt, p) ( (pPkt)->pktMetaInfo = (p) )
#define WPAL_PACKET_GET_METAINFO_POINTER(pPkt) ( (pPkt)->pktMetaInfo )
#define WPAL_PACKET_SET_TYPE(pPkt, type)  ( (pPkt)->pktType = (type) )
#define WPAL_PACKET_GET_TYPE(pPkt)  ( (pPkt)->pktType )
#define WPAL_PACKET_SET_OS_STRUCT_POINTER(pPkt, pStruct)   ( (pPkt)->pOSStruct = (pStruct) )
#define WPAL_PACKET_GET_OS_STRUCT_POINTER(pPkt)  ( (pPkt)->pOSStruct )
#define WPAL_PACKET_IS_FLAT_BUF(pktType) ( (eWLAN_PAL_PKT_TYPE_RX_RAW == (pktType)) || \
                                           (eWLAN_PAL_PKT_TYPE_TX_802_11_MGMT == (pktType)) )

/*                                                                  */
typedef void ( *wpalPacketLowPacketCB )( wpt_packet *pPacket, void *usrData );


/*                                                                           
                                                                                                
                                                           
                                                                                 
           
                                          
           
                                           
                                                                           */
wpt_status wpalPacketInit(void *pPalContext);

/*                                                                           
                                                                    
                                                                                 
           
                                          
           
                                           
                                                                           */
wpt_status wpalPacketClose(void *pPalContext);


/*                                                                           
                                                     
           
                                          
                                                            
                                                                 
           
                                                     
                                                                           */
wpt_packet * wpalPacketAlloc(wpt_packet_type pktType, wpt_uint32 nPktSize,
                             wpalPacketLowPacketCB rxLowCB, void *usrdata);

/*                                                                           
                                                                     
                                                   
           
                                          
                                               
           
                                          
                                                                           */
wpt_status wpalPacketFree(wpt_packet *pPkt);

/*                                                                           
                                                              
           
                                                        
                                               
           
                                                                                        
                                                
                                                                           */
wpt_uint32 wpalPacketGetLength(wpt_packet *pPkt);

/*                                                                           
                                                                              
                                                         
                                                                  
           
                                       
                                                    
           
                                                       
                                                                           */
wpt_status wpalPacketRawTrimHead(wpt_packet *pPkt, wpt_uint32 size);

/*                                                                           
                                                                              
                                                                       
                                                                              
           
                                       
                                                           
           
                                                           
                                                                           */
wpt_status wpalPacketRawTrimTail(wpt_packet *pPkt, wpt_uint32 size);


/*                                                                           
                                                                                              
                                                                       
           
                                       
           
                    
                                                           
                                                                           */
extern wpt_uint8 *wpalPacketGetRawBuf(wpt_packet *pPkt);


/*                                                                           
                                                                                         
                                                                                           
                            
           
                                       
           
                    
                                                           
                                                                           */
extern wpt_status wpalPacketSetRxLength(wpt_packet *pPkt, wpt_uint32 len);


/*                                                                           
                                                                              
           
                                                          
                                         
           
                                          
                                                                           */
wpt_status wpalIteratorInit(wpt_iterator *pIter, wpt_packet *pPacket);

/*                                                                           
                                                        
           
                                                          
                                         
                                                                                                                                            
                                                         
           
                                          
                                                                           */
wpt_status wpalIteratorNext(wpt_iterator *pIter, wpt_packet *pPacket, void **ppAddr, wpt_uint32 *pLen);


/*                                                                           
                                                                                
                                                                          
                              
 
           
                                         
 
           
                                          
                                                                           */
wpt_status wpalLockPacketForTransfer( wpt_packet *pPacket);

/*                                                                           
                                                                             
                                                                      
           
                                         
 
           
                                          
                                                                           */
wpt_status wpalUnlockPacket( wpt_packet *pPacket);

/*                                                                           
                                                                   
                                  
           
                                         
 
           
                                         
                                                                           */
wpt_int32 wpalPacketGetFragCount(wpt_packet *pPkt);

/*                                                                           
                                                                     
           
                                         
 
           
                                
                                  
                                
                                                                           */
wpt_status wpalIsPacketLocked( wpt_packet *pPacket);

/*                                                                           
                                                                    
         
                                              

          
                               
                                                                           */
wpt_status wpalGetNumRxRawPacket(wpt_uint32 *numRxResource);

#endif //                        
