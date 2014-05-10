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
 
#include "palTypes.h"
#include "wniCfgSta.h"
#include "aniGlobal.h"
#include "cfgApi.h"
#include "sirParams.h"
#include "limUtils.h"
#include "limTimerUtils.h"
#include "limSession.h"

#define LIM_START_AID   1


/* 
                   
  
           
                                                     
                                                        
                                                       
           
  
        
  
              
     
  
       
     
  
                                                 
               
 */

void
limInitAIDpool(tpAniSirGlobal pMac,tpPESession sessionEntry)
{
    tANI_U8 i;
    tANI_U8 maxAssocSta = pMac->lim.gLimAssocStaLimit;

    pMac->lim.gpLimAIDpool[0]=0;
    pMac->lim.freeAidHead=LIM_START_AID;

    for (i=pMac->lim.freeAidHead;i<maxAssocSta; i++)
    {
        pMac->lim.gpLimAIDpool[i]         = i+1;
    }
    pMac->lim.gpLimAIDpool[i]         =  0;

    pMac->lim.freeAidTail=i;

}


/* 
                 
  
           
                                                           
                                                          
  
        
  
              
     
  
       
  
                                                 
                                                 
 */

tANI_U16
limAssignAID(tpAniSirGlobal pMac)
{
    tANI_U16 aid;

    //                                                                     
    if (pMac->lim.gLimNumOfCurrentSTAs >= pMac->lim.gLimAssocStaLimit)
    {
        //                                     
        return 0;
    }

    /*                          */

    if (pMac->lim.freeAidHead)
    {
        aid=pMac->lim.freeAidHead;
        pMac->lim.freeAidHead=pMac->lim.gpLimAIDpool[pMac->lim.freeAidHead];
        if (pMac->lim.freeAidHead==0)
            pMac->lim.freeAidTail=0;
        pMac->lim.gLimNumOfCurrentSTAs++;
        //                                                                                                                                                             
        return aid;
    }

    return 0; /*                   */
}


/* 
                  
  
           
                                                        
                                                                  
                
  
        
  
              
     
  
       
  
                                                 
                                                                
  
               
 */

void
limReleaseAID(tpAniSirGlobal pMac, tANI_U16 aid)
{
    pMac->lim.gLimNumOfCurrentSTAs--;

    /*                             */
    if (pMac->lim.freeAidTail)
    {
        pMac->lim.gpLimAIDpool[pMac->lim.freeAidTail]=(tANI_U8)aid;
        pMac->lim.freeAidTail=(tANI_U8)aid;
    }
    else
    {
        pMac->lim.freeAidTail=pMac->lim.freeAidHead=(tANI_U8)aid;
    }
    pMac->lim.gpLimAIDpool[(tANI_U8)aid]=0;
    //                                                                                                                                                              

}
