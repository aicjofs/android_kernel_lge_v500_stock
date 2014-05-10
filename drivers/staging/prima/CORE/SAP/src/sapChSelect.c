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

                      s a p C h S e l e c t . C
                                               
  OVERVIEW:
  
  This software unit holds the implementation of the WLAN SAP modules
  functions for channel selection.  

  DEPENDENCIES: 

  Are listed for each API below. 
  
  
  Copyright (c) 2010 QUALCOMM Incorporated.
  All Rights Reserved.
  Qualcomm Confidential and Proprietary
===========================================================================*/

/*                                                                           

                                           


                                                                       
                                                                



                                        
                                                                                
                                      

                                                                           */


/*                                                                          
               
                                                                        */
#include "vos_trace.h"
#include "csrApi.h"
#include "sme_Api.h"
#include "sapChSelect.h"
#include "sapInternal.h"

/*                                                                          
                      
                                                                          */

/*                                                                          
         
                                                                          */
#define SAP_DEBUG

/*                                                                          
                            

              
                                                                
                                                           

               
        

             

      
                                                                
   
              
                              
  
               
                                                                            */
v_BOOL_t sapChanSelInit(tHalHandle halHandle, tSapChSelSpectInfo *pSpectInfoParams)
{
    tSapSpectChInfo *pSpectCh = NULL;
    v_U8_t *pChans = NULL;
    v_U16_t channelnum = 0;
    tpAniSirGlobal pMac = PMAC_STRUCT(halHandle);

    VOS_TRACE(VOS_MODULE_ID_SAP, VOS_TRACE_LEVEL_INFO_HIGH, "In %s", __FUNCTION__);

    //                              
    //                                                                           
    pSpectInfoParams->numSpectChans = pMac->scan.base20MHzChannels.numChannels;
       
    //                                                 
    pSpectCh = (tSapSpectChInfo *)vos_mem_malloc((pSpectInfoParams->numSpectChans) * sizeof(*pSpectCh));

    if(pSpectCh == NULL) {
        VOS_TRACE(VOS_MODULE_ID_SAP, VOS_TRACE_LEVEL_ERROR, "In %s, VOS_MALLOC_ERR", __FUNCTION__);
        return eSAP_FALSE;
    }

    vos_mem_zero(pSpectCh, (pSpectInfoParams->numSpectChans) * sizeof(*pSpectCh));

    //                                                                 
    pSpectInfoParams->pSpectCh = pSpectCh;

    pChans = pMac->scan.base20MHzChannels.channelList;

    //                                                                   
    for (channelnum = 0; channelnum < pSpectInfoParams->numSpectChans; channelnum++) {

        if(*pChans == 14 ) //                                          
            continue;
        pSpectCh->chNum = *pChans;
        pSpectCh->valid = eSAP_TRUE;
        pSpectCh->rssiAgr = SOFTAP_MIN_RSSI;//                            
        pSpectCh++;
        pChans++;
    }
    return eSAP_TRUE;
}

/*                                                                          
                                

              
                                                                      
                                                    
         
               
        

             

      
                                                                          
                                                       
   
              
                                                              
  
               
                                                                            */
v_U32_t sapweightRssiCount(v_S7_t rssi, v_U16_t count)
{
    v_S31_t     rssiWeight=0;
    v_S31_t     countWeight=0;
    v_U32_t     rssicountWeight=0;
    
    //                 
    rssiWeight = SOFTAP_RSSI_WEIGHT * (rssi - SOFTAP_MIN_RSSI)
                 /(SOFTAP_MAX_RSSI - SOFTAP_MIN_RSSI);
                 
    if(rssiWeight > SOFTAP_RSSI_WEIGHT)
        rssiWeight = SOFTAP_RSSI_WEIGHT;
    else if (rssiWeight < 0)
        rssiWeight = 0;

    //                       
    countWeight = SOFTAP_COUNT_WEIGHT * (count - SOFTAP_MIN_COUNT)
                  /(SOFTAP_MAX_COUNT - SOFTAP_MIN_COUNT);
                      
    if(countWeight > SOFTAP_COUNT_WEIGHT)
        countWeight = SOFTAP_COUNT_WEIGHT;
    else if (countWeight < 0)
        countWeight = 0;
        
    rssicountWeight =  rssiWeight + countWeight;      

    VOS_TRACE( VOS_MODULE_ID_SAP, VOS_TRACE_LEVEL_INFO_HIGH, "In %s, rssiWeight=%d, countWeight=%d, rssicountWeight=%d",
              __FUNCTION__, rssiWeight, countWeight, rssicountWeight);
    
    return(rssicountWeight);
}

/*                                                                          
                                   

              
                                                                 
                                                                
                     
    
               
        

             

      
                                                                      
                                                  
                                                         
   
              
                   
  
               
                                                                            */
void sapComputeSpectWeight( tSapChSelSpectInfo* pSpectInfoParams, 
                                 tHalHandle halHandle, tScanResultHandle pResult)
{
    v_S7_t rssi = 0;
    v_U8_t chn_num = 0;
    v_U8_t channel_id = 0;

    tCsrScanResultInfo *pScanResult;
    tSapSpectChInfo *pSpectCh   = pSpectInfoParams->pSpectCh;

    VOS_TRACE( VOS_MODULE_ID_SAP, VOS_TRACE_LEVEL_INFO_HIGH, "In %s, Computing spectral weight", __FUNCTION__);

    /* 
                                                                   
    */

    pScanResult = sme_ScanResultGetFirst(halHandle, pResult);    

    while (pScanResult) {
        pSpectCh = pSpectInfoParams->pSpectCh;
        //                                                                        
        for (chn_num = 0; chn_num < pSpectInfoParams->numSpectChans; chn_num++) {

            /*
                                                                        
                                         
             */
            if(pScanResult->BssDescriptor.channelId == 0)
                channel_id = pScanResult->BssDescriptor.channelIdSelf;
            else
                channel_id = pScanResult->BssDescriptor.channelId;

            if (channel_id == pSpectCh->chNum) {        
                if (pSpectCh->rssiAgr < pScanResult->BssDescriptor.rssi)
                    pSpectCh->rssiAgr = pScanResult->BssDescriptor.rssi;

                ++pSpectCh->bssCount; //                           

                VOS_TRACE(VOS_MODULE_ID_SAP, VOS_TRACE_LEVEL_INFO_HIGH,
                   "In %s, bssdes.ch_self=%d, bssdes.ch_ID=%d, bssdes.rssi=%d, SpectCh.bssCount=%d, pScanResult=0x%x",
                  __FUNCTION__, pScanResult->BssDescriptor.channelIdSelf, pScanResult->BssDescriptor.channelId, 
                  pScanResult->BssDescriptor.rssi, pSpectCh->bssCount, pScanResult);
                         
                 pSpectCh++;
                 break;
           } else {
             pSpectCh++;
           }
        }

        pScanResult = sme_ScanResultGetNext(halHandle, pResult);
    }

    //                                                                
    pSpectCh = pSpectInfoParams->pSpectCh;

    VOS_TRACE(VOS_MODULE_ID_SAP, VOS_TRACE_LEVEL_INFO_HIGH, "In %s, Spectrum Channels Weight", __FUNCTION__);

    for (chn_num = 0; chn_num < (pSpectInfoParams->numSpectChans); chn_num++) {
    
        /*
                                                                               
                                                  
        */

        rssi = (v_S7_t)pSpectCh->rssiAgr;

        pSpectCh->weight = SAPDFS_NORMALISE_1000 * sapweightRssiCount(rssi, pSpectCh->bssCount);

        //                         
        VOS_TRACE(VOS_MODULE_ID_SAP, VOS_TRACE_LEVEL_INFO_HIGH, "In %s, Chan=%d Weight= %d rssiAgr=%d bssCount=%d", __FUNCTION__, pSpectCh->chNum,
            pSpectCh->weight, pSpectCh->rssiAgr, pSpectCh->bssCount);
        //                         
        pSpectCh++;
    }
}

/*                                                                          
                            

              
                                                                  
                                               
    
               
        

             

      
                                                                        
   
              
                   
  
               
                                                                            */
void sapChanSelExit( tSapChSelSpectInfo *pSpectInfoParams )
{
    //                              
    vos_mem_free(pSpectInfoParams->pSpectCh);
}

/*                                                                          
                              

              
                                                            
    
               
        

             

      
                                                                        
   
              
                   
  
               
                                                                            */
void sapSortChlWeight(tSapChSelSpectInfo *pSpectInfoParams)
{
    tSapSpectChInfo temp;

    tSapSpectChInfo *pSpectCh = NULL;
    v_U32_t i = 0, j = 0, minWeightIndex = 0;

    pSpectCh = pSpectInfoParams->pSpectCh;
#ifdef SOFTAP_CHANNEL_RANGE
    //                                    
    for (i = 0; i < pSpectInfoParams->numSpectChans; i++) {
        minWeightIndex = i;
        for( j = i + 1; j < pSpectInfoParams->numSpectChans; j++) {
            if(pSpectCh[j].weight < pSpectCh[minWeightIndex].weight) {
                minWeightIndex = j; 
            }
        }
        if(minWeightIndex != i) {
            vos_mem_copy(&temp, &pSpectCh[minWeightIndex], sizeof(*pSpectCh));
            vos_mem_copy(&pSpectCh[minWeightIndex], &pSpectCh[i], sizeof(*pSpectCh));
            vos_mem_copy(&pSpectCh[i], &temp, sizeof(*pSpectCh));
        }
    }
#else
    //                                    
    for (i = 0; i < SPECT_24GHZ_CH_COUNT; i++) {
        minWeightIndex = i;
        for( j = i + 1; j < SPECT_24GHZ_CH_COUNT; j++) {
            if(pSpectCh[j].weight < pSpectCh[minWeightIndex].weight) {
                minWeightIndex = j; 
            }
        }
        if(minWeightIndex != i) {
            vos_mem_copy(&temp, &pSpectCh[minWeightIndex], sizeof(*pSpectCh));
            vos_mem_copy(&pSpectCh[minWeightIndex], &pSpectCh[i], sizeof(*pSpectCh));
            vos_mem_copy(&pSpectCh[i], &temp, sizeof(*pSpectCh));
        }
    }
#endif

    /*             */
    VOS_TRACE(VOS_MODULE_ID_SAP, VOS_TRACE_LEVEL_INFO_HIGH, "In %s, Sorted Spectrum Channels Weight", __FUNCTION__);
    pSpectCh = pSpectInfoParams->pSpectCh;
    for (j = 0; j < (pSpectInfoParams->numSpectChans); j++) {
        VOS_TRACE(VOS_MODULE_ID_SAP, VOS_TRACE_LEVEL_INFO_HIGH, "In %s, Channel=%d Weight= %d rssi=%d bssCount=%d",
                    __FUNCTION__, pSpectCh->chNum, pSpectCh->weight, pSpectCh->rssiAgr, pSpectCh->bssCount);
        pSpectCh++;
    }

}

/*                                                                          
                              

              
                                                                           
                                      
    
               
        

             

      
                                           
                                                  
   
              
                                                           
  
               
                                                                            */
v_U8_t sapSelectChannel(tHalHandle halHandle, tScanResultHandle pScanResult)
{
    //                                                      
    tSapChSelSpectInfo oSpectInfoParams = {NULL,0}; 
    tSapChSelSpectInfo *pSpectInfoParams = &oSpectInfoParams; //               

    v_U8_t bestChNum = 0;
#ifdef SOFTAP_CHANNEL_RANGE
    v_U32_t startChannelNum;
    v_U32_t endChannelNum;
    v_U32_t operatingBand;
    v_U8_t  count = 0;
#endif    
    VOS_TRACE(VOS_MODULE_ID_SAP, VOS_TRACE_LEVEL_INFO_HIGH, "In %s, Running SAP Ch Select", __FUNCTION__);

    //                            
    //                                                      

    //                                                     
    if(sapChanSelInit( halHandle, pSpectInfoParams) != eSAP_TRUE ) {
        VOS_TRACE(VOS_MODULE_ID_SAP, VOS_TRACE_LEVEL_ERROR, "In %s, Ch Select initialization failed", __FUNCTION__);
        return SAP_CHANNEL_NOT_SELECTED;
    }

    //                                                                
    sapComputeSpectWeight( pSpectInfoParams, halHandle, pScanResult);

    //                                                                            
    sapSortChlWeight(pSpectInfoParams);

#ifdef SOFTAP_CHANNEL_RANGE
    ccmCfgGetInt( halHandle, WNI_CFG_SAP_CHANNEL_SELECT_START_CHANNEL, &startChannelNum);
    ccmCfgGetInt( halHandle, WNI_CFG_SAP_CHANNEL_SELECT_END_CHANNEL, &endChannelNum);
    ccmCfgGetInt( halHandle, WNI_CFG_SAP_CHANNEL_SELECT_OPERATING_BAND, &operatingBand);

    /*                                                  */
    for(count=0; count < pSpectInfoParams->numSpectChans ; count++)
    {
        if((startChannelNum <= pSpectInfoParams->pSpectCh[count].chNum)&&
          ( endChannelNum >= pSpectInfoParams->pSpectCh[count].chNum))
        {
            bestChNum = (v_U8_t)pSpectInfoParams->pSpectCh[count].chNum;
            break;
        }
    }

#else
    //                                                          
    bestChNum = (v_U8_t)pSpectInfoParams->pSpectCh[0].chNum;

#endif

    //                              
    sapChanSelExit(pSpectInfoParams);

    VOS_TRACE(VOS_MODULE_ID_SAP, VOS_TRACE_LEVEL_INFO_HIGH, "In %s, Running SAP Ch select Completed, Ch=%d",
                __FUNCTION__, bestChNum);

    if (bestChNum > 0 && bestChNum <= 252)
        return bestChNum;
    else
        return SAP_CHANNEL_NOT_SELECTED;
}

