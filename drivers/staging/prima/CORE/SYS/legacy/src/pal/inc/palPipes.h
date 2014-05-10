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

/** ------------------------------------------------------------------------- *
    ------------------------------------------------------------------------- *


    \file palPipes.h

    \brief Defines Pal pipes to transfer frames/memory to/from the device

    $Id$


    Copyright (C) 2006 Airgo Networks, Incorporated

   ========================================================================== */

#ifndef PALPIPES_H
#define PALPIPES_H

#include "halTypes.h"


typedef enum
{
    CSR_PIPE =  0,  //                                                      

    PIPE_TX_1 = 1,  //               
    PIPE_TX_2 = 2,
    PIPE_TX_3 = 3,
    PIPE_TX_4 = 4,
    PIPE_TX_5 = 5,
    PIPE_TX_6 = 6,
    PIPE_TX_7 = 7,

    PIPE_RX_1 = 8,  //               
    PIPE_RX_2 = 9,
    PIPE_RX_3 = 10,
    PIPE_RX_4 = 11,
    PIPE_RX_5 = 12,
    PIPE_RX_6 = 13,
    PIPE_RX_7 = 14,

    MAX_PIPE = PIPE_RX_7,
    ANY_PIPE
}ePipes;


/*                                                                              
                                          
*/
typedef struct tagHddMemSegment
{
    struct tagHddMemSegment *next;      //                                         
    void *hddContext;                   //                                                        
    void *addr;                         //                                                                                                               
                                        //                                                                                  
    tANI_U32 length;                    //                                                  
}sHddMemSegment;



/*                                                                                
                                                               
*/
typedef struct tagTxFrameTransfer
{
    struct tagTxFrameTransfer *next;   //                                                                
    sHddMemSegment *bd;                //                                                             
    sHddMemSegment *segment;           //                                                 
    tANI_U32 descCount;                 //                                        
}sTxFrameTransfer;




#define PAL_RX_BUFFER_SIZE  (16 * 1024)
typedef tANI_U8 *tRxBuffer;

typedef struct
{
    tANI_U32 nBytes;                    //                                         
    tRxBuffer pBbuffer;                 //                        
    tANI_BUFFER_ADDR physAddr;               //                         
                                        //                                                                                  
}sRxFrame;



/*                                                                              
                                  
*/
typedef struct tagRxFrameTransfer
{
    struct tagRxFrameTransfer *next;    //                                                                
    ePipes pipe;                        //                                                                           
    sRxFrame *rxFrame;                  //                                        
}sRxFrameTransfer;



typedef union
{
    sTxFrameTransfer txFrame;
    sRxFrameTransfer rxFrame;
}uFrameTransfer;


typedef eHalStatus (*palXfrCompleteFPtr)(tHalHandle hHal, ePipes pipe, uFrameTransfer *frame);


typedef struct
{
    tANI_U32 nDescs;                    //                                                                                         
    tANI_U32 nRxBuffers;                //                                                                           
    tANI_BOOLEAN preferCircular;        //                                                                   
    tANI_BOOLEAN bdPresent;             //                                       
    palXfrCompleteFPtr callback;        //                                               
    palXfrCompleteFPtr rxCallback;      //                                                  
    tANI_U32 refWQ;             //                                    
    tANI_U32 xfrType;           //                                          
    tANI_U32 chPriority;        //                                        
    tANI_BOOLEAN bCfged;        //                                    
    tANI_U32 indexEP;     //                                                                            
    tANI_U32 bmuThreshold; //              
    //                       
    void *pReserved1;
    void *pReserved2;

    tANI_BOOLEAN use_lower_4g;  /*                           */
    tANI_BOOLEAN use_short_desc_fmt;
}sPipeCfg;




#define PIPE_MEM_XFR_LIMIT  (16 * 1024)   //                            




//                       
#define palIsPipeCfg(hHdd, pipeNum) ((hHdd)->PipeCfgInfo[(pipeNum)].bCfged)
eHalStatus palPipeCfg(tHddHandle, ePipes pipe, sPipeCfg *pPipeCfg);
eHalStatus palWriteFrame(tHddHandle, ePipes pipe, sTxFrameTransfer *frame);
eHalStatus palFreeRxFrame(tHddHandle, sRxFrameTransfer *frame);

#ifdef FEATURE_WLAN_NON_INTEGRATED_SOC
#include "halInterrupts.h"
eHalStatus palTxPipeIsr(tHalHandle hHal, eHalIntSources intSource);
eHalStatus palRxPipeIsr(tHalHandle hHal, eHalIntSources intSource);
#endif

#endif
