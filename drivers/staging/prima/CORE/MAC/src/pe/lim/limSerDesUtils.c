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

#include "aniSystemDefs.h"
#include "utilsApi.h"
#include "limTypes.h"
#include "limUtils.h"
#include "limSerDesUtils.h"



/* 
                            
  
           
                                                                
           
  
        
                                               
  
              
     
  
       
     
  
                                             
                                                               
 */

static inline tSirRetStatus
limCheckRemainingLength(tpAniSirGlobal pMac, tANI_S16 len)
{
    if (len > 0)
        return eSIR_SUCCESS;
    else
    {
        limLog(pMac, LOGW,
           FL("Received SME message with invalid rem length=%d\n"),
           len);
        return eSIR_FAILURE;
    }
} /*                                         */

#if (WNI_POLARIS_FW_PACKAGE == ADVANCED) && defined(ANI_PRODUCT_TYPE_AP)
#else
/* 
                         
  
           
                                                           
                                                                       
  
        
  
              
     
  
       
     
  
                                                                      
                                                        
                                                                    
                                                                     
                                                           
                                                   
 */

static tSirRetStatus
limGetBssDescription( tpAniSirGlobal pMac, tSirBssDescription *pBssDescription,
                     tANI_S16 rLen, tANI_S16 *lenUsed, tANI_U8 *pBuf)
{
    tANI_S16 len = 0;

    pBssDescription->length = limGetU16(pBuf);
    pBuf += sizeof(tANI_U16);
    len   = pBssDescription->length;

    if (rLen < (tANI_S16) (len + sizeof(tANI_U16)))
        return eSIR_FAILURE;

    *lenUsed = len + sizeof(tANI_U16);

    //              
    palCopyMemory( pMac->hHdd, (tANI_U8 *) pBssDescription->bssId,
                  pBuf, sizeof(tSirMacAddr));
    pBuf += sizeof(tSirMacAddr);
    len  -= sizeof(tSirMacAddr);
    if (limCheckRemainingLength(pMac, len) == eSIR_FAILURE)
        return eSIR_FAILURE;

    //              
    palCopyMemory( pMac->hHdd, (tANI_U8 *) (&pBssDescription->scanSysTimeMsec),
                  pBuf, sizeof(v_TIME_t));
    pBuf += sizeof(v_TIME_t);
    len  -= sizeof(v_TIME_t);
    if (limCheckRemainingLength(pMac, len) == eSIR_FAILURE)
        return eSIR_FAILURE;

    //                  
    palCopyMemory( pMac->hHdd, (tANI_U8 *) pBssDescription->timeStamp,
                  pBuf, sizeof(tSirMacTimeStamp));
    pBuf += sizeof(tSirMacTimeStamp);
    len  -= sizeof(tSirMacTimeStamp);
    if (limCheckRemainingLength(pMac, len) == eSIR_FAILURE)
        return eSIR_FAILURE;

    //                       
    pBssDescription->beaconInterval = limGetU16(pBuf);
    pBuf += sizeof(tANI_U16);
    len  -= sizeof(tANI_U16);
    if (limCheckRemainingLength(pMac, len) == eSIR_FAILURE)
        return eSIR_FAILURE;

    //                       
    pBssDescription->capabilityInfo = limGetU16(pBuf);
    pBuf += sizeof(tANI_U16);
    len  -= sizeof(tANI_U16);
    if (limCheckRemainingLength(pMac, len) == eSIR_FAILURE)
        return eSIR_FAILURE;

    //               
    pBssDescription->nwType = (tSirNwType) limGetU32(pBuf);
    pBuf += sizeof(tSirNwType);
    len  -= sizeof(tSirNwType);
    if (limCheckRemainingLength(pMac, len) == eSIR_FAILURE)
        return eSIR_FAILURE;

    //                     
    pBssDescription->aniIndicator = *pBuf++;
    len --;

    //             
    pBssDescription->rssi = (tANI_S8) *pBuf++;
    len --;

    //             
    pBssDescription->sinr = (tANI_S8) *pBuf++;
    len --;

    //                  
    pBssDescription->channelId = *pBuf++;
    len --;

    //                      
    pBssDescription->channelIdSelf = *pBuf++;
    len --;
    if (limCheckRemainingLength(pMac, len) == eSIR_FAILURE)
        return eSIR_FAILURE;
 
    //                                
    pBuf += sizeof(pBssDescription->sSirBssDescriptionRsvd);
    len -= sizeof(pBssDescription->sSirBssDescriptionRsvd);
    if (limCheckRemainingLength(pMac, len) == eSIR_FAILURE)
        return eSIR_FAILURE;

    //                  
    pBssDescription->nReceivedTime = limGetU32( pBuf );
    pBuf += sizeof(tANI_TIMESTAMP);
    len -= sizeof(tANI_TIMESTAMP);

#if defined WLAN_FEATURE_VOWIFI
    //                                         
    pBssDescription->parentTSF = limGetU32( pBuf );
    pBuf += sizeof(tANI_U32);
    len -= sizeof(tANI_U32);

    //                                                         
    pBssDescription->startTSF[0] = limGetU32( pBuf );
    pBuf += sizeof(tANI_U32);
    len -= sizeof(tANI_U32);

    //                                                         
    pBssDescription->startTSF[1] = limGetU32( pBuf );
    pBuf += sizeof(tANI_U32);
    len -= sizeof(tANI_U32);
#endif
#ifdef WLAN_FEATURE_VOWIFI_11R
    //               
    pBssDescription->mdiePresent = *pBuf++;
    len --;
    pBssDescription->mdie[0] = *pBuf++;
    len --;
    pBssDescription->mdie[1] = *pBuf++;
    len --;
    pBssDescription->mdie[2] = *pBuf++;
    len --;
#ifdef WLAN_FEATURE_VOWIFI_11R_DEBUG
    PELOGE(limLog(pMac, LOG1, FL("mdie=%02x %02x %02x\n"), 
        pBssDescription->mdie[0],
        pBssDescription->mdie[1],
        pBssDescription->mdie[2]);)
#endif
#endif

#ifdef FEATURE_WLAN_CCX
    pBssDescription->QBSSLoad_present = limGetU16(pBuf);
    pBuf += sizeof(tANI_U16);
    len  -= sizeof(tANI_U16);
    if (limCheckRemainingLength(pMac, len) == eSIR_FAILURE)
        return eSIR_FAILURE;

    //                       
    pBssDescription->QBSSLoad_avail = limGetU16(pBuf);
    pBuf += sizeof(tANI_U16);
    len  -= sizeof(tANI_U16);
    if (limCheckRemainingLength(pMac, len) == eSIR_FAILURE)
        return eSIR_FAILURE;
#endif
    pBssDescription->fProbeRsp = *pBuf++;
    len  -= sizeof(tANI_U8);
    if (limCheckRemainingLength(pMac, len) == eSIR_FAILURE)
        return eSIR_FAILURE;

    /*                              */
    pBuf += (3 * sizeof(tANI_U8));
    len  -= 3;

    pBssDescription->WscIeLen = limGetU32( pBuf );
    pBuf += sizeof(tANI_U32);
    len  -= sizeof(tANI_U32);
    if (limCheckRemainingLength(pMac, len) == eSIR_FAILURE)
        return eSIR_FAILURE;
    
    if (WSCIE_PROBE_RSP_LEN < len)
    {
        /*                          
                                                                   
                                                    
                                */
        palCopyMemory( pMac->hHdd, (tANI_U8 *) pBssDescription->WscIeProbeRsp,
                       pBuf,
                       WSCIE_PROBE_RSP_LEN);

    }
    else
    {
        limLog(pMac, LOGE,
                     FL("remaining bytes len %d is less than WSCIE_PROBE_RSP_LEN\n"),
                     pBssDescription->WscIeLen);
        return eSIR_FAILURE;
    }

    /*                         */
    pBuf += (WSCIE_PROBE_RSP_LEN + 1);
    len -= (WSCIE_PROBE_RSP_LEN + 1);

    if (len > 0)
    {
        palCopyMemory( pMac->hHdd, (tANI_U8 *) pBssDescription->ieFields,
                       pBuf,
                       len);
    }
    else if (len < 0)
    {
        limLog(pMac, LOGE, 
                     FL("remaining length is negative. len = %d, actual length = %d\n"), 
                     len, pBssDescription->length);
        return eSIR_FAILURE;
    }    

    return eSIR_SUCCESS;
} /*                                */
#endif



/* 
                          
  
           
                                                           
                                      
  
        
  
              
     
  
       
     
  
                                                             
                                                                      
                                                            
 */

tANI_U16
limCopyBssDescription(tpAniSirGlobal pMac, tANI_U8 *pBuf, tSirBssDescription *pBssDescription)
{
    tANI_U16 len = 0;

    limCopyU16(pBuf, pBssDescription->length);
    pBuf       += sizeof(tANI_U16);
    len        += sizeof(tANI_U16);

    palCopyMemory( pMac->hHdd, pBuf,
                  (tANI_U8 *) pBssDescription->bssId,
                  sizeof(tSirMacAddr));
    pBuf       += sizeof(tSirMacAddr);
    len        += sizeof(tSirMacAddr);

   PELOG3(limLog(pMac, LOG3,
       FL("Copying BSSdescr:channel is %d, aniInd is %d, bssId is "),
       pBssDescription->channelId, pBssDescription->aniIndicator);
    limPrintMacAddr(pMac, pBssDescription->bssId, LOG3);)

    palCopyMemory( pMac->hHdd, pBuf,
                  (tANI_U8 *) (&pBssDescription->scanSysTimeMsec),
                  sizeof(v_TIME_t));
    pBuf       += sizeof(v_TIME_t);
    len        += sizeof(v_TIME_t);

    limCopyU32(pBuf, pBssDescription->timeStamp[0]);
    pBuf       += sizeof(tANI_U32);
    len        += sizeof(tANI_U32);

    limCopyU32(pBuf, pBssDescription->timeStamp[1]);
    pBuf       += sizeof(tANI_U32);
    len        += sizeof(tANI_U32);

    limCopyU16(pBuf, pBssDescription->beaconInterval);
    pBuf       += sizeof(tANI_U16);
    len        += sizeof(tANI_U16);

    limCopyU16(pBuf, pBssDescription->capabilityInfo);
    pBuf       += sizeof(tANI_U16);
    len        += sizeof(tANI_U16);

    limCopyU32(pBuf, pBssDescription->nwType);
    pBuf       += sizeof(tANI_U32);
    len        += sizeof(tANI_U32);

    *pBuf++ = pBssDescription->aniIndicator;
    len++;

    *pBuf++ = pBssDescription->rssi;
    len++;

    *pBuf++ = pBssDescription->sinr;
    len++;

    *pBuf++ = pBssDescription->channelId;
    len++;

    palCopyMemory( pMac->hHdd, pBuf, (tANI_U8 *) &(pBssDescription->ieFields),
                  limGetIElenFromBssDescription(pBssDescription));

    return (len + sizeof(tANI_U16));
} /*                                 */



#if (WNI_POLARIS_FW_PACKAGE == ADVANCED)
/* 
                
  
           
                                                           
                                                  
  
        
  
              
     
  
       
     
  
                                                     
                                                           
  
               
 */

static void
limCopyLoad(tANI_U8 *pBuf, tSirLoad load)
{
    limCopyU16(pBuf, load.numStas);
    pBuf += sizeof(tANI_U16);

    limCopyU16(pBuf, load.channelUtilization);
    pBuf += sizeof(tANI_U16);
} /*                       */



/* 
               
  
           
                                                           
                                                  
  
        
  
              
     
  
       
     
  
                                                   
                                                
  
               
 */

static void
limGetLoad(tSirLoad *pLoad, tANI_U8 *pBuf)
{
    pLoad->numStas = limGetU16(pBuf);
    pBuf += sizeof(tANI_U16);

    pLoad->channelUtilization = limGetU16(pBuf);
    pBuf += sizeof(tANI_U16);
} /*                      */



#if (WNI_POLARIS_FW_PACKAGE == ADVANCED) && defined(ANI_PRODUCT_TYPE_AP)
/* 
                   
  
           
                                                           
                                                     
  
        
  
              
     
  
       
     
  
                                                      
                                                               
  
                                    
 */

static tANI_U16
limCopyWdsInfo(tpAniSirGlobal pMac, tANI_U8 *pBuf, tpSirWdsInfo pWdsInfo)
{
    limCopyU16(pBuf, pWdsInfo->wdsLength);
    pBuf += sizeof(tANI_U16);

    palCopyMemory( pMac->hHdd, pBuf, (tANI_U8 *) pWdsInfo->wdsBytes, pWdsInfo->wdsLength);
    pBuf += pWdsInfo->wdsLength;

    return ((tANI_U16) (sizeof(tANI_U16) + pWdsInfo->wdsLength));
} /*                          */



/* 
                  
  
           
                                                           
                                                                    
  
        
  
              
     
  
       
     
  
                                                       
                                                
  
                                                               
 */

static tANI_U8
limGetWdsInfo(tpAniSirGlobal pMac, tpSirWdsInfo pWdsInfo, tANI_U8 *pBuf)
{
    pWdsInfo->wdsLength = limGetU16(pBuf);
    pBuf += sizeof(tANI_U16);

    if (pWdsInfo->wdsLength > ANI_WDS_INFO_MAX_LENGTH)
        return false;

    palCopyMemory( pMac->hHdd, pWdsInfo->wdsBytes, pBuf, pWdsInfo->wdsLength);

    return true;
} /*                         */



/* 
                             
  
           
                                                           
                                                               
                         
  
        
  
              
     
  
       
     
  
                                                             
                                                    
  
                                                           
 */

static tANI_U16
limGetAlternateRadioList(tpAniSirGlobal pMac, tpSirMultipleAlternateRadioInfo pRadioList,
                         tANI_U8 *pBuf)
{
    tANI_U8 i;
    tANI_U16                        len = sizeof(tANI_U8);
    tpSirAlternateRadioInfo    pRadioInfo;

    pRadioList->numBss = *pBuf++;

    if (pRadioList->numBss > SIR_MAX_NUM_ALTERNATE_RADIOS)
        pRadioList->numBss = SIR_MAX_NUM_ALTERNATE_RADIOS;

    pRadioInfo = pRadioList->alternateRadio;
    for (i = 0; i < pRadioList->numBss; i++)
    {
        palCopyMemory( pMac->hHdd, pRadioInfo->bssId, pBuf, sizeof(tSirMacAddr));
        pBuf += sizeof(tSirMacAddr);
        pRadioInfo->channelId = *pBuf++;
       PELOG3(limLog(pMac, LOG3, FL("Alternate radio[%d] channelId=%d, BssId is \n"),
               i, pRadioInfo->channelId);
        limPrintMacAddr(pMac, pRadioInfo->bssId, LOG3);)

        pRadioInfo++;
        len += sizeof(tSirMacAddr) + sizeof(tANI_U8);
    }

    return len;
} /*                                    */
#endif



/* 
                           
  
           
                                                           
                                                   
  
        
  
              
     
  
       
     
  
                                                        
                                                             
                                            
  
                                               
 */

tANI_U32
limCopyNeighborBssInfo(tpAniSirGlobal pMac, tANI_U8 *pBuf, tpSirNeighborBssInfo pBssInfo)
{
    tANI_U32 bssInfoLen = 0;

    palCopyMemory( pMac->hHdd, pBuf,
                  (tANI_U8 *) pBssInfo->bssId,
                  sizeof(tSirMacAddr));
    pBuf       += sizeof(tSirMacAddr);
    bssInfoLen += sizeof(tSirMacAddr);
   PELOG3(limLog(pMac, LOG3,
       FL("Copying new NeighborWds node:channel is %d, wniIndicator is %d, bssType is %d, bssId is "),
       pBssInfo->channelId, pBssInfo->wniIndicator, pBssInfo->bssType);
    limPrintMacAddr(pMac, pBssInfo->bssId, LOG3);)

    *pBuf++ = pBssInfo->channelId;
    bssInfoLen++;

    limCopyU32(pBuf, pBssInfo->wniIndicator);
    pBuf       += sizeof(tANI_U32);
    bssInfoLen += sizeof(tANI_U32);

    limCopyU32(pBuf, pBssInfo->bssType);
    pBuf       += sizeof(tANI_U32);
    bssInfoLen += sizeof(tANI_U32);

    *pBuf++ = pBssInfo->sinr;
    bssInfoLen++;
    *pBuf++ = pBssInfo->rssi;
    bssInfoLen++;

    limCopyLoad(pBuf, pBssInfo->load);
    pBuf       += sizeof(tSirLoad);
    bssInfoLen += sizeof(tSirLoad);

    palCopyMemory( pMac->hHdd, pBuf, (tANI_U8 *) &(pBssInfo->ssId),
                  pBssInfo->ssId.length + 1);

    bssInfoLen += pBssInfo->ssId.length + 1;
    pBuf       += pBssInfo->ssId.length + 1;

    palCopyMemory( pMac->hHdd, pBuf, (tANI_U8 *) &(pBssInfo->apName),
                  pBssInfo->apName.length + 1);

    bssInfoLen += pBssInfo->apName.length + 1;
    pBuf       += pBssInfo->apName.length + 1;

    limCopyU16(pBuf, pBssInfo->rsnIE.length);
    pBuf       += sizeof(tANI_U16);
    bssInfoLen += sizeof(tANI_U16);
    palCopyMemory( pMac->hHdd, pBuf, (tANI_U8 *) &(pBssInfo->rsnIE.rsnIEdata),
                  pBssInfo->rsnIE.length);

    bssInfoLen += pBssInfo->rsnIE.length;
    pBuf       += pBssInfo->rsnIE.length;

    limCopyU32(pBuf, pBssInfo->nwType);
    pBuf       += sizeof(tANI_U32);
    bssInfoLen += sizeof(tANI_U32);

    //                                                
    limCopyU16(pBuf, pBssInfo->capabilityInfo);
    pBuf       += sizeof(tANI_U16);
    bssInfoLen += sizeof(tANI_U16);

    palCopyMemory( pMac->hHdd, pBuf, (tANI_U8 *) &(pBssInfo->operationalRateSet),
                  pBssInfo->operationalRateSet.numRates + 1);
    bssInfoLen += pBssInfo->operationalRateSet.numRates + 1;
    pBuf       += pBssInfo->operationalRateSet.numRates + 1;

    palCopyMemory( pMac->hHdd, pBuf, (tANI_U8 *) &(pBssInfo->extendedRateSet),
                  pBssInfo->extendedRateSet.numRates + 1);
    bssInfoLen += pBssInfo->extendedRateSet.numRates + 1;
    pBuf       += pBssInfo->extendedRateSet.numRates + 1;

    limCopyU16(pBuf, pBssInfo->beaconInterval);
    pBuf += sizeof(tANI_U16);
    bssInfoLen += sizeof(tANI_U16);
   
    *pBuf++ = pBssInfo->dtimPeriod;
    bssInfoLen++;
    *pBuf++ = pBssInfo->HTCapsPresent;
    bssInfoLen++;
    *pBuf++ = pBssInfo->HTInfoPresent;
    bssInfoLen++;
    *pBuf++ = pBssInfo->wmeInfoPresent;
    bssInfoLen++;
    *pBuf++ = pBssInfo->wmeEdcaPresent;
    bssInfoLen++;
    *pBuf++ = pBssInfo->wsmCapablePresent;
    bssInfoLen++;
    *pBuf++ = pBssInfo->hcfEnabled;
    bssInfoLen++;
    
    limCopyU16(pBuf, pBssInfo->propIECapability);
    pBuf += sizeof(tANI_U16);
    bssInfoLen += sizeof(tANI_U16);
    
    limCopyU32(pBuf, pBssInfo->localPowerConstraints);
    pBuf += sizeof(tANI_S32);
    bssInfoLen += sizeof(tANI_S32);
    
    limCopyU32(pBuf, pBssInfo->aggrRssi);
    pBuf += sizeof(tANI_S32);
    bssInfoLen += sizeof(tANI_S32);
    
    limCopyU32(pBuf, pBssInfo->dataCount);
    pBuf += sizeof(tANI_U32);
    bssInfoLen += sizeof(tANI_U32);
    
    limCopyU32(pBuf, pBssInfo->totalPackets);
    pBuf += sizeof(tANI_U32);
    bssInfoLen += sizeof(tANI_U32);
    
    return bssInfoLen;
} /*                                  */


/* 
                        
  
           
                                                           
                                                   
  
        
  
              
     
  
       
     
  
                                                           
                                                        
                                            
                                                               
  
                                                     
 */

static tANI_U32
limCopyNeighborList(tpAniSirGlobal pMac, tANI_U8 *pBuf, tpSirNeighborBssInfo pBssInfo, tANI_U32 numBss)
{
    tANI_U32  bssInfoLen = 0, listLen = 0;
    tANI_U8  i;
    tANI_U8  *pTemp = (tANI_U8 *) pBssInfo;

    PELOG3(limLog(pMac, LOG3, FL("Going to copy BssInfoList:\n"));)
    PELOG3(sirDumpBuf(pMac, SIR_LIM_MODULE_ID, LOG3,
               pTemp, numBss*sizeof(tSirNeighborBssInfo));)

    for (i = 0; i < numBss; i++, bssInfoLen = 0)
    {
        bssInfoLen = limCopyNeighborBssInfo(pMac,
                                   pBuf,
                                   (tpSirNeighborBssInfo) &pBssInfo[i]);

        pBuf     += bssInfoLen;
        listLen  += bssInfoLen;
    }

    return listLen;
} /*                                     */


/* 
                          
  
           
                                                          
                                                   
  
        
  
              
     
  
       
     
  
                                                                
                                                   
  
                                                    
 */

tANI_U32
limGetNeighborBssInfo(tpAniSirGlobal pMac, tpSirNeighborBssInfo pBssInfo, tANI_U8 *pBuf)
{
    tANI_U32 len = 0;

    palCopyMemory( pMac->hHdd, (tANI_U8 *) pBssInfo->bssId,
                  pBuf, sizeof(tSirMacAddr));
    pBuf += sizeof(tSirMacAddr);
    len  += sizeof(tSirMacAddr);

    pBssInfo->channelId = *pBuf++;
    len++;

    pBssInfo->wniIndicator = (tAniBool) limGetU32(pBuf);
    pBuf += sizeof(tAniBool);
    len  += sizeof(tAniBool);

    pBssInfo->bssType = (tSirBssType) limGetU32(pBuf);
    pBuf += sizeof(tSirBssType);
    len  += sizeof(tSirBssType);

    pBssInfo->sinr = *pBuf++;
    len++;
    pBssInfo->rssi = *pBuf++;
    len++;

    limGetLoad(&pBssInfo->load, pBuf);
    pBuf += sizeof(tSirLoad);
    len  += sizeof(tSirLoad);

    pBssInfo->ssId.length = *pBuf++;
    palCopyMemory( pMac->hHdd, pBssInfo->ssId.ssId, pBuf, pBssInfo->ssId.length);
    pBuf += pBssInfo->ssId.length;
    len  += pBssInfo->ssId.length + 1;
    
    pBssInfo->apName.length = *pBuf++;
    palCopyMemory( pMac->hHdd, pBssInfo->apName.name, pBuf, pBssInfo->apName.length);
    pBuf += pBssInfo->apName.length;
    len  += pBssInfo->apName.length + 1;
    
    pBssInfo->rsnIE.length = limGetU16(pBuf);
    pBuf += sizeof(tANI_U16);
    palCopyMemory( pMac->hHdd, pBssInfo->rsnIE.rsnIEdata, pBuf,
                  pBssInfo->rsnIE.length);
    pBuf += pBssInfo->rsnIE.length;
    len  += pBssInfo->rsnIE.length + 2;

   PELOG2(limLog(pMac, LOG2, FL("BSS type %d channel %d wniInd %d RSN len %d\n"),
           pBssInfo->bssType, pBssInfo->channelId, pBssInfo->wniIndicator,
           pBssInfo->rsnIE.length);
    limPrintMacAddr(pMac, pBssInfo->bssId, LOG2);)


    pBssInfo->nwType = (tSirNwType) limGetU32(pBuf);
    pBuf += sizeof(tSirNwType);
    len  += sizeof(tSirNwType);
    
    pBssInfo->capabilityInfo = limGetU16(pBuf);
    pBuf += sizeof(tANI_U16);
    len  += sizeof(tANI_U16);

    pBssInfo->operationalRateSet.numRates = *pBuf++;
    palCopyMemory( pMac->hHdd, pBssInfo->operationalRateSet.rate, pBuf,
                  pBssInfo->operationalRateSet.numRates);
    pBuf += pBssInfo->operationalRateSet.numRates;
    len  += pBssInfo->operationalRateSet.numRates + 1;

    pBssInfo->extendedRateSet.numRates = *pBuf++;
    palCopyMemory( pMac->hHdd, pBssInfo->extendedRateSet.rate, pBuf,
                  pBssInfo->extendedRateSet.numRates);
    pBuf += pBssInfo->extendedRateSet.numRates;
    len  += pBssInfo->extendedRateSet.numRates + 1;

    pBssInfo->beaconInterval = limGetU16(pBuf);
    pBuf += sizeof(tANI_U16);
    len  += sizeof(tANI_U16);

    pBssInfo->dtimPeriod = *pBuf++;
    pBssInfo->HTCapsPresent = *pBuf++;
    pBssInfo->HTInfoPresent = *pBuf++;
    pBssInfo->wmeInfoPresent = *pBuf++;
    pBssInfo->wmeEdcaPresent = *pBuf++;
    pBssInfo->wsmCapablePresent = *pBuf++;
    pBssInfo->hcfEnabled = *pBuf++;
    pBssInfo->propIECapability = limGetU16(pBuf);
    pBuf += sizeof(tANI_U16);
    pBssInfo->localPowerConstraints = limGetU32(pBuf);
    pBuf += sizeof(tANI_U32);
    len += 13;
    
   PELOG2(limLog(pMac, LOG2, FL("rsnIELen %d operRateLen %d extendRateLen %d total %d\n"),
        pBssInfo->rsnIE.length, pBssInfo->operationalRateSet.numRates,
        pBssInfo->extendedRateSet.numRates, len);)
    
    
    return len;
} /*                                 */



#if defined(ANI_PRODUCT_TYPE_AP)
/* 
                          
  
           
                                                          
                                                   
  
        
  
              
     
  
       
     
  
                                                                        
                                                        
                                                                    
                                                                     
                                                           
                                                   
 */

static tSirRetStatus
limGetNeighborBssList(tpAniSirGlobal pMac,
                      tSirNeighborBssList *pNeighborList,
                      tANI_S16 rLen, tANI_S16 *lenUsed, tANI_U8 *pBuf)
{
    tANI_U8     *pBssInfo = (tANI_U8 *) pNeighborList->bssList;
    tANI_U32    i, bssInfoLen = 0, len = 0, numBss, numBssList;

    numBssList = numBss = limGetU32(pBuf);

    if (!numBss)
    {
        PELOG1(limLog(pMac, LOG1, FL("No Neighbor BSS present in Neighbor list\n"));)

        return eSIR_FAILURE;
    }
   PELOG2(limLog(pMac, LOG2,
           FL("Num of Neighbor BSS present in Neighbor list is %d\n"),
           numBss);)

    pBuf += sizeof(tANI_U32);
    len  += sizeof(tANI_U32);
    rLen -= sizeof(tANI_U32);
    if (limCheckRemainingLength(pMac, rLen) == eSIR_FAILURE)
        return eSIR_FAILURE;

    //                                                               
    bssInfoLen = limGetNeighborBssInfo(pMac, (tSirNeighborBssInfo *) pBssInfo,
                                       pBuf);
    PELOG1(limLog(pMac, LOG1,
           FL("BSSinfo len to be joined is %d rem %d, numBss = %d\n"),
           bssInfoLen, rLen, numBss - 1);)
    pBuf  += bssInfoLen;
    len   += bssInfoLen;
    rLen  -= (tANI_S16) bssInfoLen;
    numBss--;
    numBssList--;

    if (numBss > 0)
    {
        //                                  
        if (numBss > SIR_MAX_NUM_NEIGHBOR_BSS)
        {
            //                                      
          PELOG2(limLog(pMac, LOG2,
                  FL("Pruning number of neighbors to %d from %d\n"),
                  SIR_MAX_NUM_NEIGHBOR_BSS, numBss);)
            numBss = SIR_MAX_NUM_NEIGHBOR_BSS;
        }

        pBssInfo = (tANI_U8 *) pMac->lim.gLimNeighborBssList.bssList;
        for (i = numBss; i > 0; i--)
        {
            PELOG3(limLog(pMac, LOG3, FL("pMac = %p, pBssInfo = %p, pBuf = %p\n"), pMac, pBssInfo, pBuf);)
            bssInfoLen = limGetNeighborBssInfo(pMac,
                                   (tSirNeighborBssInfo *) pBssInfo,
                                   pBuf);

            pBssInfo += sizeof(tSirNeighborBssInfo);
            pBuf     += bssInfoLen;
            len      += bssInfoLen;
            rLen     -= (tANI_S16) bssInfoLen;
            numBssList--;

            PELOG1(limLog(pMac, LOG1, FL("BSS info[%d] len %d rem %d\n"),
                   i, bssInfoLen, rLen);)
        }

        while (numBssList > 0)
        {
            tSirNeighborBssInfo pTemp;
            bssInfoLen = limGetNeighborBssInfo(pMac, &pTemp, pBuf);
            pBuf     += bssInfoLen;
            len      += bssInfoLen;
            rLen     -= (tANI_S16) bssInfoLen;
            numBssList--;
            PELOG1(limLog(pMac, LOG1, FL("BSS info[%d] len %d rem %d\n"),
                   numBssList, bssInfoLen, rLen);)
        }
    }
    *lenUsed = len;

    pMac->lim.gLimNeighborBssList.numBss = pNeighborList->numBss
                                         = numBss;

    return eSIR_SUCCESS;
} /*                                       */



/* 
                           
  
           
                                                           
                                                   
  
        
  
              
     
  
       
     
  
                                                           
                                                                     
                                              
 */

static tANI_U16
limCopyNeighborWdsInfo(tpAniSirGlobal pMac, tANI_U8 *pBuf, tpSirNeighborBssWdsInfo pInfo)
{
    tANI_U16    len = 0;

    len   = (tANI_U16) limCopyNeighborBssInfo(pMac, pBuf, &pInfo->neighborBssInfo);
    pBuf += len;
    len  += limCopyWdsInfo(pMac, pBuf, &pInfo->wdsInfo);

    return len;
} /*                                  */



/* 
                          
  
           
                                                           
                                                        
  
        
  
              
     
  
       
     
  
                                                           
                                                                 
                                              
 */

static tANI_U16
limCopyMeasMatrixInfo(tANI_U8 *pBuf, tpLimMeasMatrixNode pInfo)
{
    tANI_U16 len = 0;
    
    *pBuf++ = pInfo->matrix.channelNumber;
    len++;
    *pBuf++ = pInfo->matrix.compositeRssi;
    len++;
    limCopyU32(pBuf, pInfo->matrix.aggrRssi);
    pBuf += sizeof(tANI_S32);
    len += sizeof(tANI_S32);
    limCopyU32(pBuf, pInfo->matrix.totalPackets);
    len += sizeof(tANI_U32);

    return len;
} /*                                 */



/* 
                          
  
           
                                                           
                                                        
  
        
  
              
     
  
       
     
  
                                                  
                                                    
                                       
 */

static tANI_U16
limCopyMeasMatrixList(tpAniSirGlobal pMac, tANI_U8 *pBuf)
{
    tANI_U16    len = 0, nodeLen = 0;
    tANI_U8     numNodes = pMac->lim.gpLimMeasData->numMatrixNodes;

    *pBuf++ = numNodes;
    len++;

    if (numNodes)
    {
        tpLimMeasMatrixNode pInfo =
                         pMac->lim.gpLimMeasData->pMeasMatrixInfo;
        while (numNodes-- && pInfo) //                             
        {
            nodeLen = limCopyMeasMatrixInfo(pBuf, pInfo);
            pBuf   += nodeLen;
            len    += nodeLen;

            if (pInfo->next)
                pInfo = pInfo->next;
            else
                break;
        }

        return len;
    }
    else
    {
        *pBuf = 0;
        return 1;
    }
} /*                                 */



/* 
                           
  
           
                                                           
                                                   
  
        
  
              
     
  
       
     
  
                                                
                                                  
                                       
 */

static tANI_U16
limCopyNeighborWdsList(tpAniSirGlobal pMac, tANI_U8 *pBuf)
{
    tANI_U16    len = 0, nodeLen = 0;

    if (pMac->lim.gpLimMeasData->numBssWds)
    {
        limCopyU32(pBuf, pMac->lim.gpLimMeasData->numBssWds);
        pBuf += sizeof(tANI_U32);
        len  += sizeof(tANI_U32);

        tpLimNeighborBssWdsNode pInfo =
                        pMac->lim.gpLimMeasData->pNeighborWdsInfo;
        while (pInfo)
        {
            nodeLen = limCopyNeighborWdsInfo(pMac, pBuf, &pInfo->info);
            pBuf   += nodeLen;
            len    += nodeLen;

            if (pInfo->next)
                pInfo = pInfo->next;
            else
                break;
        }

        return len;
    }
    else
    {
        limCopyU32(pBuf, 0);
        return (sizeof(tANI_U32));
    }
} /*                                  */
#endif
#endif


/* 
                   
  
           
                                                           
                                                             
  
        
  
              
     
  
       
     
  
                                                         
                                                  
  
                                       
 */

static tANI_U32
limGetKeysInfo(tpAniSirGlobal pMac, tpSirKeys pKeyInfo, tANI_U8 *pBuf)
{
    tANI_U32 len = 0;

    pKeyInfo->keyId        = *pBuf++;
    len++;
    pKeyInfo->unicast      = *pBuf++;
    len++;
    pKeyInfo->keyDirection = (tAniKeyDirection) limGetU32(pBuf);
    len  += sizeof(tAniKeyDirection);
    pBuf += sizeof(tAniKeyDirection);

    palCopyMemory( pMac->hHdd, pKeyInfo->keyRsc, pBuf, WLAN_MAX_KEY_RSC_LEN);
    pBuf += WLAN_MAX_KEY_RSC_LEN;
    len  += WLAN_MAX_KEY_RSC_LEN;

    pKeyInfo->paeRole      = *pBuf++;
    len++;

    pKeyInfo->keyLength    = limGetU16(pBuf);
    pBuf += sizeof(tANI_U16);
    len  += sizeof(tANI_U16);
    palCopyMemory( pMac->hHdd, pKeyInfo->key, pBuf, pKeyInfo->keyLength);
    pBuf += pKeyInfo->keyLength;
    len  += pKeyInfo->keyLength;

   PELOG3(limLog(pMac, LOG3,
           FL("Extracted keyId=%d, keyLength=%d, Key is :\n"),
           pKeyInfo->keyId, pKeyInfo->keyLength);
    sirDumpBuf(pMac, SIR_LIM_MODULE_ID, LOG3,
               pKeyInfo->key, pKeyInfo->keyLength);)

    return len;
} /*                          */



/* 
                         
  
           
                                                                    
                              
  
         
  
        
  
              
     
  
       
     
  
                                                                      
                                                     
                                                                  
                                                        
                                           
 */

tSirRetStatus
limStartBssReqSerDes(tpAniSirGlobal pMac, tpSirSmeStartBssReq pStartBssReq, tANI_U8 *pBuf)
{
    tANI_S16 len = 0;
#if (WNI_POLARIS_FW_PACKAGE == ADVANCED) && defined(ANI_PRODUCT_TYPE_AP)
    tANI_U8 i;
#endif

#ifdef PE_DEBUG_LOG1
    tANI_U8  *pTemp = pBuf;
#endif

    if (!pStartBssReq || !pBuf)
        return eSIR_FAILURE;

    pStartBssReq->messageType = limGetU16(pBuf);
    pBuf += sizeof(tANI_U16);

    len = pStartBssReq->length = limGetU16(pBuf);
    pBuf += sizeof(tANI_U16);

    PELOG1(limLog(pMac, LOG1, FL("SME_START_BSS_REQ length %d bytes is:\n"), len);)
    PELOG1(sirDumpBuf(pMac, SIR_LIM_MODULE_ID, LOG1, pTemp, len);)

    if (len < (tANI_S16) sizeof(tANI_U32))
        return eSIR_FAILURE;

    len -= sizeof(tANI_U32); //                    
    if (limCheckRemainingLength(pMac, len) == eSIR_FAILURE)
        return eSIR_FAILURE;

    //                  
    pStartBssReq->sessionId = *pBuf++;
    len--;
    if (limCheckRemainingLength(pMac, len) == eSIR_FAILURE)
        return eSIR_FAILURE;

    //                      
    pStartBssReq->transactionId = limGetU16( pBuf );
    pBuf += sizeof( tANI_U16 );
    len -= sizeof( tANI_U16 );
    if (limCheckRemainingLength(pMac, len) == eSIR_FAILURE)
        return eSIR_FAILURE;

    //              
    palCopyMemory( pMac->hHdd, (tANI_U8 *) pStartBssReq->bssId, pBuf, sizeof(tSirMacAddr));
    pBuf += sizeof(tSirMacAddr);
    len  -= sizeof(tSirMacAddr);
    if (limCheckRemainingLength(pMac, len) == eSIR_FAILURE)
        return eSIR_FAILURE;

    //                    
    palCopyMemory( pMac->hHdd, (tANI_U8 *) pStartBssReq->selfMacAddr, pBuf, sizeof(tSirMacAddr));
    pBuf += sizeof(tSirMacAddr);
    len  -= sizeof(tSirMacAddr);
    if (limCheckRemainingLength(pMac, len) == eSIR_FAILURE)
        return eSIR_FAILURE;

    //                       
    pStartBssReq->beaconInterval = limGetU16( pBuf );
    pBuf += sizeof( tANI_U16 );
    len -= sizeof( tANI_U16 );
    if (limCheckRemainingLength(pMac, len) == eSIR_FAILURE)
        return eSIR_FAILURE;

    //                  
    pStartBssReq->dot11mode = *pBuf++;
    len --;
    if (limCheckRemainingLength(pMac, len) == eSIR_FAILURE)
        return eSIR_FAILURE;

    //                
    pStartBssReq->bssType = (tSirBssType) limGetU32(pBuf);
    pBuf += sizeof(tANI_U32);
    len  -= sizeof(tANI_U32);
    if (limCheckRemainingLength(pMac, len) == eSIR_FAILURE)
        return eSIR_FAILURE;

    //             
    if (*pBuf > SIR_MAC_MAX_SSID_LENGTH)
    {
        //                                              
        PELOGW(limLog(pMac, LOGW, FL("Invalid SSID length, len=%d\n"), *pBuf);)
        return eSIR_FAILURE;
    }

    pStartBssReq->ssId.length = *pBuf++;
    len--;
    if (len < pStartBssReq->ssId.length)
    {
        limLog(pMac, LOGW,
           FL("SSID length is longer that the remaining length. SSID len=%d, remaining len=%d\n"),
           pStartBssReq->ssId.length, len);
        return eSIR_FAILURE;
    }

    palCopyMemory( pMac->hHdd, (tANI_U8 *) pStartBssReq->ssId.ssId,
                  pBuf,
                  pStartBssReq->ssId.length);
    pBuf += pStartBssReq->ssId.length;
    len  -= pStartBssReq->ssId.length;
    if (limCheckRemainingLength(pMac, len) == eSIR_FAILURE)
        return eSIR_FAILURE;

    //                  
    pStartBssReq->channelId = *pBuf++;
    len--;

    //                                  
    pStartBssReq->cbMode = (ePhyChanBondState)limGetU32( pBuf );
    pBuf += sizeof( tANI_U32 );
    len -= sizeof( tANI_U32 );

    if (limCheckRemainingLength(pMac, len) == eSIR_FAILURE)
        return eSIR_FAILURE;

#if (WNI_POLARIS_FW_PACKAGE == ADVANCED) && defined(ANI_PRODUCT_TYPE_AP)
    tANI_U16 paramLen = 0;

    //                           
    pStartBssReq->alternateRadioList.numBss = *pBuf;
    paramLen = limGetAlternateRadioList(pMac,
                              &pMac->lim.gLimAlternateRadioList,
                              pBuf);
    pBuf += paramLen;
    len  -= paramLen;
    if (limCheckRemainingLength(pMac, len) == eSIR_FAILURE)
        return eSIR_FAILURE;

    //                   
    pStartBssReq->powerLevel = *pBuf++;
    len--;
    if (limCheckRemainingLength(pMac, len) == eSIR_FAILURE)
        return eSIR_FAILURE;

    //                
    if (limGetWdsInfo(pMac, &pStartBssReq->wdsInfo, pBuf) == false)
    {
        limLog(pMac, LOGW, FL("Invalid WDS length %d in SME_START_BSS_REQ\n"),
               pStartBssReq->wdsInfo.wdsLength);
        return eSIR_FAILURE;
    }

    pBuf += sizeof(tANI_U16) + pStartBssReq->wdsInfo.wdsLength;
    len  -= (sizeof(tANI_U16) + pStartBssReq->wdsInfo.wdsLength);
    if (limCheckRemainingLength(pMac, len) == eSIR_FAILURE)
        return eSIR_FAILURE;
#endif

#ifdef WLAN_SOFTAP_FEATURE
    //                        
    pStartBssReq->privacy = *pBuf++;
    len--;
    if (limCheckRemainingLength(pMac, len) == eSIR_FAILURE)
        return eSIR_FAILURE;

    //                     
    pStartBssReq->apUapsdEnable = *pBuf++;
    len--;
    if (limCheckRemainingLength(pMac, len) == eSIR_FAILURE)
        return eSIR_FAILURE;

    //                   
    pStartBssReq->ssidHidden = *pBuf++;
    len--;
    if (limCheckRemainingLength(pMac, len) == eSIR_FAILURE)
        return eSIR_FAILURE;

    pStartBssReq->fwdWPSPBCProbeReq = *pBuf++;
    len--;
    if (limCheckRemainingLength(pMac, len) == eSIR_FAILURE)
        return eSIR_FAILURE;
    
    //                             
    pStartBssReq->protEnabled = *pBuf++;
    len--;
    if (limCheckRemainingLength(pMac, len) == eSIR_FAILURE)
       return eSIR_FAILURE;

    //                               
    pStartBssReq->obssProtEnabled = *pBuf++;
    len--;
    if (limCheckRemainingLength(pMac, len) == eSIR_FAILURE)
       return eSIR_FAILURE;

    pStartBssReq->ht_capab = limGetU16(pBuf);
    pBuf += sizeof(tANI_U16);
    len  -= sizeof(tANI_U16);
    if (limCheckRemainingLength(pMac, len) == eSIR_FAILURE)
        return eSIR_FAILURE;

    //                 
    pStartBssReq->authType = (tSirBssType) limGetU32(pBuf);
    pBuf += sizeof(tANI_U32);
    len  -= sizeof(tANI_U32);
    if (limCheckRemainingLength(pMac, len) == eSIR_FAILURE)
        return eSIR_FAILURE;

    //                   
    pStartBssReq->dtimPeriod = limGetU32(pBuf);
    pBuf += sizeof(tANI_U32);
    len  -= sizeof(tANI_U32);
    if (limCheckRemainingLength(pMac, len) == eSIR_FAILURE)
        return eSIR_FAILURE;

    //                  
    pStartBssReq->wps_state = *pBuf++;
    len--;
    if (limCheckRemainingLength(pMac, len) == eSIR_FAILURE)
        return eSIR_FAILURE;

#endif
    //                   
    pStartBssReq->bssPersona = *pBuf++;
    len--;
    if (limCheckRemainingLength(pMac, len) == eSIR_FAILURE)
        return eSIR_FAILURE;

    //              
    pStartBssReq->rsnIE.length = limGetU16(pBuf);
    pBuf += sizeof(tANI_U16);

    //                                                               
    if (pStartBssReq->rsnIE.length > SIR_MAC_MAX_IE_LENGTH + 2)
    {
        limLog(pMac, LOGW,
               FL("Invalid RSN IE length %d in SME_START_BSS_REQ\n"),
               pStartBssReq->rsnIE.length);
        return eSIR_FAILURE;
    }

    palCopyMemory( pMac->hHdd, pStartBssReq->rsnIE.rsnIEdata,
                  pBuf, pStartBssReq->rsnIE.length);

    len  -= (sizeof(tANI_U16) + pStartBssReq->rsnIE.length);
    pBuf += pStartBssReq->rsnIE.length;
    if (limCheckRemainingLength(pMac, len) == eSIR_FAILURE)
        return eSIR_FAILURE;

    //               
    pStartBssReq->nwType = (tSirNwType) limGetU32(pBuf);
    pBuf += sizeof(tSirNwType);
    len  -= sizeof(tSirNwType);
    if (limCheckRemainingLength(pMac, len) == eSIR_FAILURE)
        return eSIR_FAILURE;

    //                           
    pStartBssReq->operationalRateSet.numRates = *pBuf++;

    //                          
    if (pStartBssReq->operationalRateSet.numRates >
        SIR_MAC_MAX_NUMBER_OF_RATES)
    {
        limLog(pMac, LOGW, FL("Invalid numRates %d in SME_START_BSS_REQ\n"),
               pStartBssReq->operationalRateSet.numRates);
        return eSIR_FAILURE;
    }

    len--;
    if (len < pStartBssReq->operationalRateSet.numRates)
        return eSIR_FAILURE;

    palCopyMemory( pMac->hHdd, (tANI_U8 *) pStartBssReq->operationalRateSet.rate,
                  pBuf,
                  pStartBssReq->operationalRateSet.numRates);
    pBuf += pStartBssReq->operationalRateSet.numRates;
    len  -= pStartBssReq->operationalRateSet.numRates;

    //                        
    if ((pStartBssReq->nwType == eSIR_11G_NW_TYPE) ||
        (pStartBssReq->nwType == eSIR_11N_NW_TYPE ))
    {
        pStartBssReq->extendedRateSet.numRates = *pBuf++;
        len--;
        palCopyMemory( pMac->hHdd, pStartBssReq->extendedRateSet.rate,
                       pBuf, pStartBssReq->extendedRateSet.numRates);
        pBuf += pStartBssReq->extendedRateSet.numRates;
        len  -= pStartBssReq->extendedRateSet.numRates;
    }

#if (WNI_POLARIS_FW_PACKAGE == ADVANCED) && defined(ANI_PRODUCT_TYPE_AP)
    if (limCheckRemainingLength(pMac, len) == eSIR_FAILURE)
        return eSIR_FAILURE;

        limLog(pMac,
               LOGW,
               FL("Going to parse numSSID  in the START_BSS_REQ, len=%d\n"), len);
    if (len < 2)
    {
        //                                          
        limLog(pMac,
               LOGW,
               FL("No numSSID  in the START_BSS_REQ, len=%d\n"), len);
        return eSIR_FAILURE;
    }

    //                
    pStartBssReq->numSSID = *pBuf++;
    len--;

    //                   
    for (i = 0; (i < pStartBssReq->numSSID) && len; i++)
    {
        pStartBssReq->ssIdList[i].length = *pBuf++;
        len--;
        if (len < pStartBssReq->ssIdList[i].length)
        {
            limLog(pMac, LOGW,
               FL("SSID length[%d] is more than the rem length[%d]\n"),
               pStartBssReq->ssIdList[i].length, len);
            return eSIR_FAILURE;
        }

        if (pStartBssReq->ssIdList[i].length > SIR_MAC_MAX_SSID_LENGTH)
        {
            //                                              
            limLog(pMac,
                   LOGW,
                   FL("Invalid SSID length in the list, len=%d\n"),
                   pStartBssReq->ssIdList[i].length);
            return eSIR_FAILURE;
        }

        palCopyMemory( pMac->hHdd, (tANI_U8 *) pStartBssReq->ssIdList[i].ssId,
                      pBuf,
                      pStartBssReq->ssIdList[i].length);
        pBuf += pStartBssReq->ssIdList[i].length;
        len  -= pStartBssReq->ssIdList[i].length;
    }
#endif

    if (len)
    {
        limLog(pMac, LOGW, FL("Extra bytes left in SME_START_BSS_REQ, len=%d\n"), len);
    }

    return eSIR_SUCCESS;
} /*                                */



/* 
                        
  
           
                                                                    
                             
  
         
  
        
  
              
     
  
       
     
  
                                                                     
                                                     
                                                                  
                                                        
                                           
 */

tSirRetStatus
limStopBssReqSerDes(tpAniSirGlobal pMac, tpSirSmeStopBssReq pStopBssReq, tANI_U8 *pBuf)
{
    tANI_S16 len = 0;

#ifdef PE_DEBUG_LOG1
    tANI_U8  *pTemp = pBuf;
#endif

    pStopBssReq->messageType = limGetU16(pBuf);
    pBuf += sizeof(tANI_U16);

    len = pStopBssReq->length = limGetU16(pBuf);
    pBuf += sizeof(tANI_U16);

    PELOG1(limLog(pMac, LOG1, FL("SME_STOP_BSS_REQ length %d bytes is:\n"), len);)
    PELOG1(sirDumpBuf(pMac, SIR_LIM_MODULE_ID, LOG1, pTemp, len);)

    if (len < (tANI_S16) sizeof(tANI_U32))
        return eSIR_FAILURE;

    len -= sizeof(tANI_U32); //                    
    if (limCheckRemainingLength(pMac, len) == eSIR_FAILURE)
        return eSIR_FAILURE;

    //                  
    pStopBssReq->sessionId = *pBuf++;
    len--;
    if (limCheckRemainingLength(pMac, len) == eSIR_FAILURE)
        return eSIR_FAILURE;

    //                      
    pStopBssReq->transactionId = limGetU16(pBuf);
    pBuf += sizeof(tANI_U16);
    len -= sizeof(tANI_U16); 

    //                   
    pStopBssReq->reasonCode = (tSirResultCodes) limGetU32(pBuf);
    pBuf += sizeof(tSirResultCodes);
    len -= sizeof(tSirResultCodes);

    //              
    palCopyMemory( pMac->hHdd, (tANI_U8 *) pStopBssReq->bssId, pBuf, sizeof(tSirMacAddr));
    len  -= sizeof(tSirMacAddr);
  
    if (len)
        return eSIR_FAILURE;
    else
        return eSIR_SUCCESS;

} /*                               */



/* 
                     
  
           
                                                                    
                         
  
         
  
        
  
              
     
  
       
     
  
                                                              
                                                 
                                                              
                                                    
                                       
 */

tSirRetStatus
limJoinReqSerDes(tpAniSirGlobal pMac, tpSirSmeJoinReq pJoinReq, tANI_U8 *pBuf)
{
    tANI_S16 len = 0;
    tANI_S16 lenUsed = 0;

#ifdef PE_DEBUG_LOG1
     tANI_U8  *pTemp = pBuf;
#endif

    if (!pJoinReq || !pBuf)
    {
        PELOGE(limLog(pMac, LOGE, FL("NULL ptr received\n"));)
        return eSIR_FAILURE;
    }

    //                    
    pJoinReq->messageType = limGetU16(pBuf);
    pBuf += sizeof(tANI_U16);

    //               
    len = pJoinReq->length = limGetU16(pBuf);
    pBuf += sizeof(tANI_U16);

    if (pJoinReq->messageType == eWNI_SME_JOIN_REQ)
        PELOG1(limLog(pMac, LOG3, FL("SME_JOIN_REQ length %d bytes is:\n"), len);)
    else
        PELOG1(limLog(pMac, LOG3, FL("SME_REASSOC_REQ length %d bytes is:\n"), len);)

    PELOG1(sirDumpBuf(pMac, SIR_LIM_MODULE_ID, LOG3, pTemp, len);)

    if (len < (tANI_S16) sizeof(tANI_U32))
    {
        PELOGE(limLog(pMac, LOGE, FL("len too short %d\n"), len);)
        return eSIR_FAILURE;
    }

    len -= sizeof(tANI_U32); //                    
    if (limCheckRemainingLength(pMac, len) == eSIR_FAILURE)
        return eSIR_FAILURE;

    //                  
    pJoinReq->sessionId = *pBuf++;
    len--;
    if (limCheckRemainingLength(pMac, len) == eSIR_FAILURE)
        return eSIR_FAILURE;

    //                      
    pJoinReq->transactionId = limGetU16(pBuf);
    pBuf += sizeof(tANI_U16);
    len  -= sizeof(tANI_U16);
    if (limCheckRemainingLength(pMac, len) == eSIR_FAILURE)
        return eSIR_FAILURE;

    //             
    pJoinReq->ssId.length = *pBuf++;
    len--;
    palCopyMemory( pMac->hHdd, (tANI_U8 *) pJoinReq->ssId.ssId, pBuf, pJoinReq->ssId.length);
    pBuf += pJoinReq->ssId.length;
    len -= pJoinReq->ssId.length;
    if (limCheckRemainingLength(pMac, len) == eSIR_FAILURE)
        return eSIR_FAILURE;

    //                    
    palCopyMemory( pMac->hHdd, pJoinReq->selfMacAddr, pBuf, sizeof(tSirMacAddr));
    pBuf += sizeof(tSirMacAddr);
    len -= sizeof(tSirMacAddr);
    if (limCheckRemainingLength(pMac, len) == eSIR_FAILURE)
        return eSIR_FAILURE;

    //                
    pJoinReq->bsstype = (tSirBssType) limGetU32(pBuf);
    pBuf += sizeof(tANI_U32);
    len  -= sizeof(tANI_U32);
    if (limCheckRemainingLength(pMac, len) == eSIR_FAILURE)
        return eSIR_FAILURE;

    //                  
    pJoinReq->dot11mode= *pBuf++;
    len--;
    if (limCheckRemainingLength(pMac, len) == eSIR_FAILURE)
        return eSIR_FAILURE;

    //                   
    pJoinReq->staPersona = *pBuf++;
    len--;
    if (limCheckRemainingLength(pMac, len) == eSIR_FAILURE)
        return eSIR_FAILURE;

    //               
    pJoinReq->cbMode = *pBuf++;
    len--;
    if (limCheckRemainingLength(pMac, len) == eSIR_FAILURE)
        return eSIR_FAILURE;

    //                          
    pJoinReq->uapsdPerAcBitmask = *pBuf++;
    len--;
    if (limCheckRemainingLength(pMac, len) == eSIR_FAILURE)
        return eSIR_FAILURE;

#if (WNI_POLARIS_FW_PACKAGE == ADVANCED)
    //                  
    pJoinReq->assocType = (tSirAssocType) limGetU32(pBuf);
    pBuf += sizeof(tSirAssocType);
    len  -= sizeof(tSirAssocType); 
    if (limCheckRemainingLength(pMac, len) == eSIR_FAILURE)
        return eSIR_FAILURE;
#endif

    //                           
    pJoinReq->operationalRateSet.numRates= *pBuf++;
    len--;
    if (pJoinReq->operationalRateSet.numRates)
    {
        palCopyMemory( pMac->hHdd, (tANI_U8 *) pJoinReq->operationalRateSet.rate, pBuf, pJoinReq->operationalRateSet.numRates);
        pBuf += pJoinReq->operationalRateSet.numRates;
        len -= pJoinReq->operationalRateSet.numRates;
        if (limCheckRemainingLength(pMac, len) == eSIR_FAILURE)
            return eSIR_FAILURE;
    }

    //                        
    pJoinReq->extendedRateSet.numRates = *pBuf++;
    len--;
    if (pJoinReq->extendedRateSet.numRates)
    {
        palCopyMemory( pMac->hHdd, pJoinReq->extendedRateSet.rate, pBuf, pJoinReq->extendedRateSet.numRates);
        pBuf += pJoinReq->extendedRateSet.numRates;
        len  -= pJoinReq->extendedRateSet.numRates;
        if (limCheckRemainingLength(pMac, len) == eSIR_FAILURE)
            return eSIR_FAILURE;
    }

    //               
    pJoinReq->rsnIE.length = limGetU16(pBuf);
    pBuf += sizeof(tANI_U16);
    len -= sizeof(tANI_U16);

    if (pJoinReq->rsnIE.length)
    {
        //                                                                
        if ((pJoinReq->rsnIE.length > SIR_MAC_MAX_IE_LENGTH + 2) ||
             (pJoinReq->rsnIE.length != 2 + *(pBuf + 1)))
        {
            limLog(pMac, LOGW,
                   FL("Invalid RSN IE length %d in SME_JOIN_REQ\n"),
                   pJoinReq->rsnIE.length);
            return eSIR_FAILURE;
        }
        palCopyMemory( pMac->hHdd, (tANI_U8 *) pJoinReq->rsnIE.rsnIEdata,
                      pBuf, pJoinReq->rsnIE.length);
        pBuf += pJoinReq->rsnIE.length;
        len  -= pJoinReq->rsnIE.length; //            
        if (limCheckRemainingLength(pMac, len) == eSIR_FAILURE)
            return eSIR_FAILURE;
    }

#ifdef FEATURE_WLAN_CCX
    //                
    pJoinReq->cckmIE.length = limGetU16(pBuf);
    pBuf += sizeof(tANI_U16);
    len -= sizeof(tANI_U16);
    if (pJoinReq->cckmIE.length)
    {
        //                                                                 
        if ((pJoinReq->cckmIE.length > SIR_MAC_MAX_IE_LENGTH) ||
             (pJoinReq->cckmIE.length != (2 + *(pBuf + 1))))
        {
            limLog(pMac, LOGW,
                   FL("Invalid CCKM IE length %d/%d in SME_JOIN/REASSOC_REQ\n"),
                   pJoinReq->cckmIE.length, 2 + *(pBuf + 1));
            return eSIR_FAILURE;
        }
        palCopyMemory( pMac->hHdd, (tANI_U8 *) pJoinReq->cckmIE.cckmIEdata,
                      pBuf, pJoinReq->cckmIE.length);
        pBuf += pJoinReq->cckmIE.length;
        len  -= pJoinReq->cckmIE.length; //             
        if (limCheckRemainingLength(pMac, len) == eSIR_FAILURE)
            return eSIR_FAILURE;
    }
#endif

    //                        
    pJoinReq->addIEScan.length = limGetU16(pBuf);
    pBuf += sizeof(tANI_U16);
    len -= sizeof(tANI_U16);

    if (pJoinReq->addIEScan.length)
    {
        //                                                            
        if (pJoinReq->addIEScan.length > SIR_MAC_MAX_IE_LENGTH + 2)
        {
            limLog(pMac, LOGE,
                   FL("Invalid addIE Scan length %d in SME_JOIN_REQ\n"),
                   pJoinReq->addIEScan.length);
            return eSIR_FAILURE;
        }
        //                                                                
        palCopyMemory( pMac->hHdd, (tANI_U8 *) pJoinReq->addIEScan.addIEdata,
                      pBuf, pJoinReq->addIEScan.length);
        pBuf += pJoinReq->addIEScan.length;
        len  -= pJoinReq->addIEScan.length; //            
        if (limCheckRemainingLength(pMac, len) == eSIR_FAILURE)
            return eSIR_FAILURE;
    }

    pJoinReq->addIEAssoc.length = limGetU16(pBuf);
    pBuf += sizeof(tANI_U16);
    len -= sizeof(tANI_U16);

    //                         
    if (pJoinReq->addIEAssoc.length)
    {
        //                                                            
        if (pJoinReq->addIEAssoc.length > SIR_MAC_MAX_IE_LENGTH + 2)
        {
            limLog(pMac, LOGE,
                   FL("Invalid addIE Assoc length %d in SME_JOIN_REQ\n"),
                   pJoinReq->addIEAssoc.length);
            return eSIR_FAILURE;
        }
        //                                                                
        palCopyMemory( pMac->hHdd, (tANI_U8 *) pJoinReq->addIEAssoc.addIEdata,
                      pBuf, pJoinReq->addIEAssoc.length);
        pBuf += pJoinReq->addIEAssoc.length;
        len  -= pJoinReq->addIEAssoc.length; //            
        if (limCheckRemainingLength(pMac, len) == eSIR_FAILURE)
            return eSIR_FAILURE;
    }

    pJoinReq->MCEncryptionType = limGetU32(pBuf);
    pBuf += sizeof(tANI_U32);
    len -= sizeof(tANI_U32);
    if (limCheckRemainingLength(pMac, len) == eSIR_FAILURE)
        return eSIR_FAILURE;    
    
    pJoinReq->UCEncryptionType = limGetU32(pBuf);
    pBuf += sizeof(tANI_U32);
    len -= sizeof(tANI_U32);
    if (limCheckRemainingLength(pMac, len) == eSIR_FAILURE)
        return eSIR_FAILURE;    
    
#ifdef WLAN_FEATURE_VOWIFI_11R
    //                
    pJoinReq->is11Rconnection = (tAniBool)limGetU32(pBuf);
    pBuf += sizeof(tAniBool);
    len -= sizeof(tAniBool);
    if (limCheckRemainingLength(pMac, len) == eSIR_FAILURE)
        return eSIR_FAILURE;    
#endif

#ifdef FEATURE_WLAN_CCX
    //                
    pJoinReq->isCCXconnection = (tAniBool)limGetU32(pBuf);
    pBuf += sizeof(tAniBool);
    len -= sizeof(tAniBool);
    if (limCheckRemainingLength(pMac, len) == eSIR_FAILURE)
        return eSIR_FAILURE;    

    //                  
    pJoinReq->ccxTspecInfo.numTspecs = *pBuf++;
    len -= sizeof(tANI_U8);
    palCopyMemory(pMac->hHdd, (void*)&pJoinReq->ccxTspecInfo.tspec[0], pBuf, (sizeof(tTspecInfo)* pJoinReq->ccxTspecInfo.numTspecs));
    pBuf += sizeof(tTspecInfo)*SIR_CCX_MAX_TSPEC_IES;
    len  -= sizeof(tTspecInfo)*SIR_CCX_MAX_TSPEC_IES;
    if (limCheckRemainingLength(pMac, len) == eSIR_FAILURE)
        return eSIR_FAILURE;
#endif
    
#if defined WLAN_FEATURE_VOWIFI_11R || defined FEATURE_WLAN_CCX || defined(FEATURE_WLAN_LFR)
    //                        
    pJoinReq->isFastTransitionEnabled = (tAniBool)limGetU32(pBuf);
    pBuf += sizeof(tAniBool);
    len -= sizeof(tAniBool);
    if (limCheckRemainingLength(pMac, len) == eSIR_FAILURE)
        return eSIR_FAILURE;    
#endif

#ifdef FEATURE_WLAN_LFR
    //                            
    pJoinReq->isFastRoamIniFeatureEnabled = (tAniBool)limGetU32(pBuf);
    pBuf += sizeof(tAniBool);
    len -= sizeof(tAniBool);
    if (limCheckRemainingLength(pMac, len) == eSIR_FAILURE)
        return eSIR_FAILURE;    
#endif

#if (WNI_POLARIS_FW_PACKAGE == ADVANCED) && defined(ANI_PRODUCT_TYPE_AP)
    //                     
    pJoinReq->bpIndicator = (tAniBool) limGetU32(pBuf);
    pBuf += sizeof(tAniBool);
    len  -= sizeof(tAniBool); 
    if (limCheckRemainingLength(pMac, len) == eSIR_FAILURE)
        return eSIR_FAILURE;

    //                
    pJoinReq->bpType = (tSirBpIndicatorType) limGetU32(pBuf);
    pBuf += sizeof(tSirBpIndicatorType);
    len  -= sizeof(tSirBpIndicatorType); 
    if (limCheckRemainingLength(pMac, len) == eSIR_FAILURE)
        return eSIR_FAILURE;

    //                          
    if (limGetNeighborBssList(pMac, &pJoinReq->neighborBssList,
                              len, &lenUsed, pBuf) == eSIR_FAILURE)
    {
        PELOGE(limLog(pMac, LOGE, FL("get neighbor bss list failed\n"));)
        return eSIR_FAILURE;
    }
    pBuf += lenUsed;
    len -= lenUsed;
    PELOG1(limLog(pMac, LOG1, FL("Assoc Type %d RSN len %d bp %d type %d bss RSN len %d\n"),
           pJoinReq->assocType, pJoinReq->rsnIE.length, pJoinReq->bpIndicator,
           pJoinReq->bpType, pJoinReq->neighborBssList.bssList->rsnIE.length);)
#endif

    //                                   
    pJoinReq->cbNeighbors.cbBssFoundPri = *pBuf;
    pBuf++;
    pJoinReq->cbNeighbors.cbBssFoundSecUp = *pBuf;
    pBuf++;
    pJoinReq->cbNeighbors.cbBssFoundSecDown = *pBuf;
    pBuf++;
    len -= 3;

    //                               
    pJoinReq->spectrumMgtIndicator = (tAniBool) limGetU32(pBuf);
    pBuf += sizeof(tAniBool);       
    len -= sizeof(tAniBool);

    pJoinReq->powerCap.minTxPower = *pBuf++;
    pJoinReq->powerCap.maxTxPower = *pBuf++;
    len -=2;
    limLog(pMac, LOG1, FL("Power Caps: Min power = %d, Max power = %d\n"), pJoinReq->powerCap.minTxPower, pJoinReq->powerCap.maxTxPower);

    pJoinReq->supportedChannels.numChnl = *pBuf++;
    len--;
    palCopyMemory( pMac->hHdd, (tANI_U8 *) pJoinReq->supportedChannels.channelList,
                      pBuf, pJoinReq->supportedChannels.numChnl);
    pBuf += pJoinReq->supportedChannels.numChnl;
    len-= pJoinReq->supportedChannels.numChnl;

    PELOG2(limLog(pMac, LOG2,
            FL("spectrumInd ON: minPower %d, maxPower %d , numChnls %d\n"),
            pJoinReq->powerCap.minTxPower,
            pJoinReq->powerCap.maxTxPower,
            pJoinReq->supportedChannels.numChnl);)

    //                          
    pJoinReq->uapsdPerAcBitmask = *pBuf++;
    len--;
    if (limCheckRemainingLength(pMac, len) == eSIR_FAILURE)
        return eSIR_FAILURE;

#if (WNI_POLARIS_FW_PRODUCT == WLAN_STA)
    //
    //                                                  
    //                                                   
    //                                            
    //
    if (limGetBssDescription( pMac, &pJoinReq->bssDescription,
                             len, &lenUsed, pBuf) == eSIR_FAILURE)
    {
        PELOGE(limLog(pMac, LOGE, FL("get bss description failed\n"));)
        return eSIR_FAILURE;
    }
    PELOG3(sirDumpBuf(pMac, SIR_LIM_MODULE_ID, LOG3, (tANI_U8 *) &(pJoinReq->bssDescription), pJoinReq->bssDescription.length + 2);)
    pBuf += lenUsed;
    len -= lenUsed;
#endif

    return eSIR_SUCCESS;
} /*                            */


/*                                                                
                         
                                                              
                                                                
                      
 
           
                                                          
                                          
                                                  
 
            
                                                                  */
void
limAssocIndSerDes(tpAniSirGlobal pMac, tpLimMlmAssocInd pAssocInd, tANI_U8 *pBuf, tpPESession psessionEntry)
{
    tANI_U8  *pLen  = pBuf;
    tANI_U16 mLen = 0;

#ifdef PE_DEBUG_LOG1
    tANI_U8  *pTemp = pBuf;
#endif

#if (WNI_POLARIS_FW_PACKAGE == ADVANCED)
    tANI_U32 len = 0;
#endif

    mLen   = sizeof(tANI_U32);
    mLen   += sizeof(tANI_U8);
    pBuf  += sizeof(tANI_U16);
    *pBuf = psessionEntry->smeSessionId;
    pBuf   += sizeof(tANI_U8);

     //                    
    palCopyMemory( pMac->hHdd, pBuf, pAssocInd->peerMacAddr, sizeof(tSirMacAddr));
    pBuf += sizeof(tSirMacAddr);
    mLen += sizeof(tSirMacAddr);

    //            
    limCopyU16(pBuf, pAssocInd->aid);
    pBuf += sizeof(tANI_U16);
    mLen += sizeof(tANI_U16);

   //              
    palCopyMemory( pMac->hHdd, pBuf, psessionEntry->bssId, sizeof(tSirMacAddr));
    pBuf += sizeof(tSirMacAddr);
    mLen += sizeof(tSirMacAddr);

    //              
    limCopyU16(pBuf, psessionEntry->staId);
    pBuf += sizeof(tANI_U16);
    mLen += sizeof(tANI_U16);

    //                 
    limCopyU32(pBuf, pAssocInd->authType);
    pBuf += sizeof(tANI_U32);
    mLen += sizeof(tANI_U32);

    //             
    palCopyMemory( pMac->hHdd, pBuf, (tANI_U8 *) &(pAssocInd->ssId), pAssocInd->ssId.length + 1);
    pBuf += (1 + pAssocInd->ssId.length);
    mLen += (1 + pAssocInd->ssId.length);

    //              
    limCopyU16(pBuf, pAssocInd->rsnIE.length);
    pBuf += sizeof(tANI_U16);
    mLen += sizeof(tANI_U16);
    palCopyMemory( pMac->hHdd, pBuf, (tANI_U8 *) &(pAssocInd->rsnIE.rsnIEdata),
                  pAssocInd->rsnIE.length);
    pBuf += pAssocInd->rsnIE.length;
    mLen += pAssocInd->rsnIE.length;

#if (WNI_POLARIS_FW_PACKAGE == ADVANCED)

    limCopyU16(pBuf, pAssocInd->seqNum);
    pBuf += sizeof(tANI_U16);
    mLen += sizeof(tANI_U16);

    limCopyU32(pBuf, pAssocInd->wniIndicator);
    pBuf += sizeof(tAniBool);
    mLen += sizeof(tAniBool);

    limCopyU32(pBuf, pAssocInd->bpIndicator);
    pBuf += sizeof(tAniBool);
    mLen += sizeof(tAniBool);

    limCopyU32(pBuf, pAssocInd->bpType);
    pBuf += sizeof(tSirBpIndicatorType);
    mLen += sizeof(tSirBpIndicatorType);

    limCopyU32(pBuf, pAssocInd->assocType);
    pBuf += sizeof(tANI_U32);
    mLen += sizeof(tANI_U32);

    limCopyLoad(pBuf, pAssocInd->load);
    pBuf += sizeof(tSirLoad);
    mLen += sizeof(tSirLoad);

    limCopyU32(pBuf, pAssocInd->numBss);
    pBuf += sizeof(tANI_U32);
    mLen += sizeof(tANI_U32);

    if (pAssocInd->numBss)
    {
        len   = limCopyNeighborList(pMac,
                           pBuf,
                           pAssocInd->neighborList, pAssocInd->numBss);
        pBuf += len;
        mLen += (tANI_U16) len;
    }

    //                                      

    limCopyU16(pBuf, *(tANI_U16 *)&pAssocInd->capabilityInfo);
    pBuf += sizeof(tANI_U16); //               
    mLen += sizeof(tANI_U16);

    limCopyU32(pBuf, *(tANI_U32 *)&pAssocInd->nwType);
    pBuf += sizeof(tANI_U32); //       
    mLen += sizeof(tANI_U32);

#endif

    limCopyU32(pBuf, pAssocInd->spectrumMgtIndicator);
    pBuf += sizeof(tAniBool);
    mLen += sizeof(tAniBool);

    if (pAssocInd->spectrumMgtIndicator == eSIR_TRUE)
    {
        *pBuf = pAssocInd->powerCap.minTxPower;
        pBuf++;
        *pBuf = pAssocInd->powerCap.maxTxPower;
        pBuf++;
        mLen += sizeof(tSirMacPowerCapInfo);

        *pBuf = pAssocInd->supportedChannels.numChnl;
        pBuf++;
        mLen++;

      palCopyMemory( pMac->hHdd, pBuf,
                     (tANI_U8 *) &(pAssocInd->supportedChannels.channelList),
                     pAssocInd->supportedChannels.numChnl);


        pBuf += pAssocInd->supportedChannels.numChnl;
        mLen += pAssocInd->supportedChannels.numChnl;
    }
#ifdef WLAN_SOFTAP_FEATURE
    limCopyU32(pBuf, pAssocInd->WmmStaInfoPresent);
    pBuf += sizeof(tANI_U32);
    mLen += sizeof(tANI_U32);
#endif
     //                                        
    limCopyU16(pLen, mLen);

    PELOG1(limLog(pMac, LOG1, FL("Sending SME_ASSOC_IND length %d bytes:\n"), mLen);)
    PELOG1(sirDumpBuf(pMac, SIR_LIM_MODULE_ID, LOG1, pTemp, mLen);)
} /*                             */



/* 
                      
  
           
                                                          
                                                            
                        
  
         
  
        
  
              
     
  
       
     
  
                                                                     
                                                  
                                                               
                                                     
                                        
 */

tSirRetStatus
limAssocCnfSerDes(tpAniSirGlobal pMac, tpSirSmeAssocCnf pAssocCnf, tANI_U8 *pBuf)
{
#ifdef PE_DEBUG_LOG1
    tANI_U8  *pTemp = pBuf;
#endif

    if (!pAssocCnf || !pBuf)
        return eSIR_FAILURE;

    pAssocCnf->messageType = limGetU16(pBuf);
    pBuf += sizeof(tANI_U16);

    pAssocCnf->length = limGetU16(pBuf);
    pBuf += sizeof(tANI_U16);

    if (pAssocCnf->messageType == eWNI_SME_ASSOC_CNF)
    {
        PELOG1(limLog(pMac, LOG1, FL("SME_ASSOC_CNF length %d bytes is:\n"), pAssocCnf->length);)
    }
    else
    {
        PELOG1(limLog(pMac, LOG1, FL("SME_REASSOC_CNF length %d bytes is:\n"), pAssocCnf->length);)
    }
    PELOG1(sirDumpBuf(pMac, SIR_LIM_MODULE_ID, LOG1, pTemp, pAssocCnf->length);)

    //            
    pAssocCnf->statusCode = (tSirResultCodes) limGetU32(pBuf);
    pBuf += sizeof(tSirResultCodes);

    //      
    palCopyMemory( pMac->hHdd, pAssocCnf->bssId, pBuf, sizeof(tSirMacAddr));
    pBuf += sizeof(tSirMacAddr);

    //            
    palCopyMemory( pMac->hHdd, pAssocCnf->peerMacAddr, pBuf, sizeof(tSirMacAddr));
    pBuf += sizeof(tSirMacAddr);


    pAssocCnf->aid = limGetU16(pBuf);
    pBuf += sizeof(tANI_U16);
    //               
    palCopyMemory( pMac->hHdd, pAssocCnf->alternateBssId, pBuf, sizeof(tSirMacAddr));
    pBuf += sizeof(tSirMacAddr);

    //                   
    pAssocCnf->alternateChannelId = *pBuf;
    pBuf++;

    return eSIR_SUCCESS;
} /*                             */



/* 
                         
  
           
                                                          
                                                                  
  
         
  
        
  
              
     
  
       
     
  
                                                            
                                       
                                                     
                                                                  
                                                        
                                           
 */

tSirRetStatus
limDisassocCnfSerDes(tpAniSirGlobal pMac, tpSirSmeDisassocCnf pDisassocCnf, tANI_U8 *pBuf)
{
#ifdef  PE_DEBUG_LOG1
    tANI_U8  *pTemp = pBuf;
#endif

    if (!pDisassocCnf || !pBuf)
        return eSIR_FAILURE;

    pDisassocCnf->messageType = limGetU16(pBuf);
    pBuf += sizeof(tANI_U16);

    pDisassocCnf->length = limGetU16(pBuf);
    pBuf += sizeof(tANI_U16);

    PELOG1(limLog(pMac, LOG1, FL("SME_DISASSOC_CNF length %d bytes is:\n"), pDisassocCnf->length);)
    PELOG1(sirDumpBuf(pMac, SIR_LIM_MODULE_ID, LOG1, pTemp, pDisassocCnf->length);)

    pDisassocCnf->statusCode = (tSirResultCodes) limGetU32(pBuf);
    pBuf += sizeof(tSirResultCodes);

    palCopyMemory( pMac->hHdd, pDisassocCnf->bssId, pBuf, sizeof(tSirMacAddr));
    pBuf += sizeof(tSirMacAddr);
 
    palCopyMemory( pMac->hHdd, pDisassocCnf->peerMacAddr, pBuf, sizeof(tSirMacAddr));

#if defined(ANI_PRODUCT_TYPE_AP)
    pBuf += sizeof(tSirMacAddr);
    pDisassocCnf->aid = limGetU16(pBuf);
#endif
    
    return eSIR_SUCCESS;
} /*                                */



#if (WNI_POLARIS_FW_PACKAGE == ADVANCED) && defined(ANI_PRODUCT_TYPE_AP)
/* 
                            
  
           
                                                          
                                                                     
  
         
  
        
  
              
     
  
       
     
  
                                                        
                                   
                                                 
                                                              
                                                    
                                       
 */

tSirRetStatus
limMeasurementReqSerDes(tpAniSirGlobal pMac, tpSirSmeMeasurementReq pMeasReq, tANI_U8 *pBuf)
{
    tANI_S16 len = 0;
    PELOG3(tANI_U8  *pTemp = pBuf;)

    if (!pMeasReq || !pBuf)
        return eSIR_FAILURE;

    pMeasReq->messageType = limGetU16(pBuf);
    pBuf += sizeof(tANI_U16);

    len = pMeasReq->length = limGetU16(pBuf);
    pBuf += sizeof(tANI_U16);

    PELOG3(limLog(pMac, LOG3, FL("SME_MEASUREMENT_REQ length %d bytes is:\n"), len);
    sirDumpBuf(pMac, SIR_LIM_MODULE_ID, LOG3, pTemp, len);)

    if (len < (tANI_S16) sizeof(tANI_U32))
        return eSIR_FAILURE;

    len -= sizeof(tANI_U32); //                    
    if (limCheckRemainingLength(pMac, len) == eSIR_FAILURE)
        return eSIR_FAILURE;

    pMeasReq->measControl.periodicMeasEnabled = (tAniBool)
                                                limGetU32(pBuf);

    pBuf += sizeof(tAniBool);
    len  -= sizeof(tAniBool);
    if (limCheckRemainingLength(pMac, len) == eSIR_FAILURE)
        return eSIR_FAILURE;

    pMeasReq->measControl.involveSTAs = (tAniBool)
                                        limGetU32(pBuf);
    pBuf += sizeof(tAniBool);
    len  -= sizeof(tAniBool);
    if (limCheckRemainingLength(pMac, len) == eSIR_FAILURE)
        return eSIR_FAILURE;

    pMeasReq->measControl.metricsType = *pBuf++;
    len--;
    if (limCheckRemainingLength(pMac, len) == eSIR_FAILURE)
        return eSIR_FAILURE;

    pMeasReq->measControl.scanType = (tSirScanType)
                                     limGetU32(pBuf);
    pBuf += sizeof(tSirScanType);
    len  -= sizeof(tSirScanType);
    if (limCheckRemainingLength(pMac, len) == eSIR_FAILURE)
        return eSIR_FAILURE;

    pMeasReq->measControl.longChannelScanPeriodicity = *pBuf++;
    len--;
    if (limCheckRemainingLength(pMac, len) == eSIR_FAILURE)
        return eSIR_FAILURE;

    pMeasReq->measControl.cb11hEnabled = (tAniBool)limGetU32(pBuf);
    pBuf += sizeof(tAniBool);
    len  -= sizeof(tAniBool);

    if (limCheckRemainingLength(pMac, len) == eSIR_FAILURE)
        return eSIR_FAILURE;

    pMeasReq->measDuration.shortTermPeriod = limGetU32(pBuf);
    pBuf += sizeof(tANI_U32);

    pMeasReq->measDuration.averagingPeriod = limGetU32(pBuf);
    pBuf += sizeof(tANI_U32);

    pMeasReq->measDuration.shortChannelScanDuration = limGetU32(pBuf);
    pBuf += sizeof(tANI_U32);

    pMeasReq->measDuration.longChannelScanDuration = limGetU32(pBuf);
    pBuf += sizeof(tANI_U32);

    len  -= sizeof(tSirMeasDuration);
    if (limCheckRemainingLength(pMac, len) == eSIR_FAILURE)
        return eSIR_FAILURE;

    pMeasReq->measIndPeriod = limGetU32(pBuf);

    pBuf += sizeof(tANI_U32);
    len  -= sizeof(tANI_U32);
    if (limCheckRemainingLength(pMac, len) == eSIR_FAILURE)
        return eSIR_FAILURE;

    pMeasReq->channelList.numChannels = *pBuf++;
    palCopyMemory( pMac->hHdd, (tANI_U8 *) pMeasReq->channelList.channelNumber,
                  pBuf, pMeasReq->channelList.numChannels);

    return eSIR_SUCCESS;
} /*                                   */


/* 
                    
  
           
                                                          
                                                                      
  
         
  
        
  
              
     
  
       
     
  
                                                                
                                                 
                                                              
                                                    
                                       
 */

tSirRetStatus
limWdsReqSerDes(tpAniSirGlobal pMac, tpSirSmeSetWdsInfoReq pWdsInfoReq, tANI_U8 *pBuf)
{
    tANI_S16 len = 0;
    PELOG1(tANI_U8  *pTemp = pBuf;)

    if (!pWdsInfoReq || !pBuf)
        return eSIR_FAILURE;

    //                    
    pWdsInfoReq->messageType = limGetU16(pBuf);
    pBuf += sizeof(tANI_U16);

    //               
    len = pWdsInfoReq->length = limGetU16(pBuf);
    pBuf += sizeof(tANI_U16);

    PELOG1(limLog(pMac, LOG1, FL("SME_SET_WDS_INFO_REQ length %d bytes is:\n"), len);)
    PELOG1(sirDumpBuf(pMac, SIR_LIM_MODULE_ID, LOG1, pTemp, len);)

    if (len < (tANI_S16) sizeof(tANI_U32))
        return eSIR_FAILURE;

    len -= sizeof(tANI_U32); //                    

    //                      
    pWdsInfoReq->transactionId = limGetU16(pBuf);
    pBuf += sizeof(tANI_U16);
    len -= sizeof(tANI_U16);

    //                
    pWdsInfoReq->wdsInfo.wdsLength = limGetU16(pBuf);
    len -= sizeof(tANI_U16);
    pBuf += sizeof(tANI_U16);

    if (pWdsInfoReq->wdsInfo.wdsLength > ANI_WDS_INFO_MAX_LENGTH)
        return eSIR_FAILURE;

    if (limCheckRemainingLength(pMac, len) == eSIR_FAILURE)
        return eSIR_FAILURE;

    palCopyMemory( pMac->hHdd, (tANI_U8 *) pWdsInfoReq->wdsInfo.wdsBytes,
        pBuf,
        pWdsInfoReq->wdsInfo.wdsLength);

    return eSIR_SUCCESS;

} /*                           */


/* 
                            
  
           
                                                           
                                              
  
        
  
              
  
       
  
                                                     
                                                                 
               
 */

void
limMeasurementIndSerDes(tpAniSirGlobal pMac, tANI_U8 *pBuf)
{
    tANI_U8          *pLen;
    tANI_U16         len = 0, infoLen = 0;
    tSirLoad    load;
    PELOG3(tANI_U8  *pTemp = pBuf;)

    limCopyU16(pBuf, eWNI_SME_MEASUREMENT_IND);
    pBuf += sizeof(tANI_U16);

    pLen  = pBuf;
    pBuf += sizeof(tANI_U16);

    limCopyU32(pBuf, pMac->lim.gpLimMeasData->duration);
    pBuf += sizeof(tANI_U32);
    len += sizeof(tANI_U32);

    load.numStas            = pMac->lim.gLimNumOfCurrentSTAs;
    load.channelUtilization =
                        pMac->lim.gpLimMeasData->avgChannelUtilization;
    limCopyLoad(pBuf, load);
    pBuf += sizeof(tSirLoad);
    len  += sizeof(tSirLoad);

    infoLen = limCopyMeasMatrixList(pMac, pBuf);
    pBuf   += infoLen;
    len    += infoLen;

    infoLen = limCopyNeighborWdsList(pMac, pBuf);
    pBuf   += infoLen;
    len    += infoLen;

    limCopyU16(pLen, len+4);

    PELOG3(limLog(pMac, LOG3, FL("Sending MEAS_IND length %d bytes:\n"), len);
    sirDumpBuf(pMac, SIR_LIM_MODULE_ID, LOG3, pTemp, len);)
} /*                                   */
#endif


/*                                                                
                           
                                                                
                                                                  
                        
 
           
                                                              
                                          
                                                  
 
            
                                                                  */
void
limReassocIndSerDes(tpAniSirGlobal pMac, tpLimMlmReassocInd pReassocInd, tANI_U8 *pBuf, tpPESession psessionEntry)
{
    tANI_U8  *pLen  = pBuf;
    tANI_U16 mLen = 0;

#ifdef  PE_DEBUG_LOG1
    tANI_U8  *pTemp = pBuf;
#endif

#if (WNI_POLARIS_FW_PACKAGE == ADVANCED)
    tANI_U32 len = 0;
#endif

    mLen   = sizeof(tANI_U32);
    pBuf  += sizeof(tANI_U16);
    *pBuf++ = psessionEntry->smeSessionId;
    mLen += sizeof(tANI_U8);

    //                    
    palCopyMemory( pMac->hHdd, pBuf, pReassocInd->peerMacAddr, sizeof(tSirMacAddr));
    pBuf += sizeof(tSirMacAddr);
    mLen += sizeof(tSirMacAddr);

    //                   
    palCopyMemory( pMac->hHdd, pBuf, pReassocInd->currentApAddr, sizeof(tSirMacAddr));
    pBuf += sizeof(tSirMacAddr);
    mLen += sizeof(tSirMacAddr);

    //            
    limCopyU16(pBuf, pReassocInd->aid);
    pBuf += sizeof(tANI_U16);
    mLen += sizeof(tANI_U16);
 
    //              
    palCopyMemory( pMac->hHdd, pBuf, psessionEntry->bssId, sizeof(tSirMacAddr));
    pBuf += sizeof(tSirMacAddr);
    mLen += sizeof(tSirMacAddr);

    //              
    limCopyU16(pBuf, psessionEntry->staId);
    pBuf += sizeof(tANI_U16);
    mLen += sizeof(tANI_U16);

    //                 
    limCopyU32(pBuf, pReassocInd->authType);
    pBuf += sizeof(tAniAuthType);
    mLen += sizeof(tAniAuthType);

    //             
    palCopyMemory( pMac->hHdd, pBuf, (tANI_U8 *) &(pReassocInd->ssId),
                  pReassocInd->ssId.length + 1);
    pBuf += 1 + pReassocInd->ssId.length;
    mLen += pReassocInd->ssId.length + 1;

    //              
    limCopyU16(pBuf, pReassocInd->rsnIE.length);
    pBuf += sizeof(tANI_U16);
    mLen += sizeof(tANI_U16);
    palCopyMemory( pMac->hHdd, pBuf, (tANI_U8 *) &(pReassocInd->rsnIE.rsnIEdata),
                  pReassocInd->rsnIE.length);
    pBuf += pReassocInd->rsnIE.length;
    mLen += pReassocInd->rsnIE.length;

    //              
    limCopyU16(pBuf, pReassocInd->addIE.length);
    pBuf += sizeof(tANI_U16);
    mLen += sizeof(tANI_U16);
    palCopyMemory( pMac->hHdd, pBuf, (tANI_U8*) &(pReassocInd->addIE.addIEdata),
                   pReassocInd->addIE.length);
    pBuf += pReassocInd->addIE.length;
    mLen += pReassocInd->addIE.length;

#if (WNI_POLARIS_FW_PACKAGE == ADVANCED)

    limCopyU16(pBuf, pReassocInd->seqNum);
    pBuf += sizeof(tANI_U16);
    mLen += sizeof(tANI_U16);

    limCopyU32(pBuf, pReassocInd->wniIndicator);
    pBuf += sizeof(tAniBool);
    mLen += sizeof(tAniBool);

    limCopyU32(pBuf, pReassocInd->bpIndicator);
    pBuf += sizeof(tAniBool);
    mLen += sizeof(tAniBool);

    limCopyU32(pBuf, pReassocInd->bpType);
    pBuf += sizeof(tSirBpIndicatorType);
    mLen += sizeof(tSirBpIndicatorType);

    limCopyU32(pBuf, pReassocInd->reassocType);
    pBuf += sizeof(tSirAssocType);
    mLen += sizeof(tSirAssocType);

    limCopyLoad(pBuf, pReassocInd->load);
    pBuf += sizeof(tSirLoad);
    mLen += sizeof(tSirLoad);

    limCopyU32(pBuf, pReassocInd->numBss);
    pBuf += sizeof(tANI_U32);
    mLen += sizeof(tANI_U32);

    if (pReassocInd->numBss)
    {
        len   = limCopyNeighborList(pMac,
                       pBuf,
                       pReassocInd->neighborList, pReassocInd->numBss);
        pBuf += len;
        mLen += (tANI_U16) len;
    }

    //                                      
    limCopyU16(pBuf, *(tANI_U16 *)&pReassocInd->capabilityInfo);
    pBuf += sizeof(tANI_U16); //               
    mLen += sizeof(tANI_U16);

    limCopyU32(pBuf, *(tANI_U32 *)&pReassocInd->nwType);
    pBuf += sizeof(tANI_U32); //       
    mLen += sizeof(tANI_U32);
#endif

    limCopyU32(pBuf, pReassocInd->spectrumMgtIndicator);
    pBuf += sizeof(tAniBool);
    mLen += sizeof(tAniBool);

    if (pReassocInd->spectrumMgtIndicator == eSIR_TRUE)
    {
        *pBuf = pReassocInd->powerCap.minTxPower;
        pBuf++;
        *pBuf = pReassocInd->powerCap.maxTxPower;
        pBuf++;
        mLen += sizeof(tSirMacPowerCapInfo);

        *pBuf = pReassocInd->supportedChannels.numChnl;
        pBuf++;
        mLen++;

        palCopyMemory( pMac->hHdd, pBuf,
                       (tANI_U8 *) &(pReassocInd->supportedChannels.channelList),
                       pReassocInd->supportedChannels.numChnl);

        pBuf += pReassocInd->supportedChannels.numChnl;
        mLen += pReassocInd->supportedChannels.numChnl;
    }
#ifdef WLAN_SOFTAP_FEATURE
    limCopyU32(pBuf, pReassocInd->WmmStaInfoPresent);
    pBuf += sizeof(tANI_U32);
    mLen += sizeof(tANI_U32);
#endif

    //                                          
    limCopyU16(pLen, mLen);

    PELOG1(limLog(pMac, LOG1, FL("Sending SME_REASSOC_IND length %d bytes:\n"), mLen);)
    PELOG1(sirDumpBuf(pMac, SIR_LIM_MODULE_ID, LOG1, pTemp, mLen);)
} /*                               */


/* 
                     
  
           
                                                                  
                       
  
         
  
        
  
              
     
  
       
     
  
                                                                 
                                                    
  
               
 */

void
limAuthIndSerDes(tpAniSirGlobal pMac, tpLimMlmAuthInd pAuthInd, tANI_U8 *pBuf)
{
    tANI_U8  *pLen  = pBuf;
    tANI_U16 mLen = 0;

#ifdef  PE_DEBUG_LOG1
    tANI_U8  *pTemp = pBuf;
#endif

    mLen   = sizeof(tANI_U32);
    pBuf  += sizeof(tANI_U16);
    *pBuf++ = pAuthInd->sessionId;
    mLen += sizeof(tANI_U8);

    //                           
    palZeroMemory(pMac->hHdd, pBuf, sizeof(tSirMacAddr));
    pBuf += sizeof(tSirMacAddr);
    mLen += sizeof(tSirMacAddr);

    palCopyMemory( pMac->hHdd, pBuf, pAuthInd->peerMacAddr, sizeof(tSirMacAddr));
    pBuf += sizeof(tSirMacAddr);
    mLen += sizeof(tSirMacAddr);

    limCopyU32(pBuf, pAuthInd->authType);
    pBuf += sizeof(tAniAuthType);
    mLen += sizeof(tAniAuthType);
  
    limCopyU16(pLen, mLen);

    PELOG1(limLog(pMac, LOG1, FL("Sending SME_AUTH_IND length %d bytes:\n"), mLen);)
    PELOG1(sirDumpBuf(pMac, SIR_LIM_MODULE_ID, LOG1, pTemp, mLen);)
} /*                            */



/* 
                           
  
           
                                                                    
                               
  
         
  
        
  
              
     
  
       
     
  
                                                                
                                    
                                                       
  
                                                                    
                                                          
                                             
 */

tSirRetStatus
limSetContextReqSerDes(tpAniSirGlobal pMac, tpSirSmeSetContextReq pSetContextReq, tANI_U8 *pBuf)
{
    tANI_S16 len = 0;
    tANI_U16 totalKeySize = sizeof(tANI_U8); //                              
    tANI_U8  numKeys;
    tANI_U8 *pKeys;

#ifdef  PE_DEBUG_LOG1
    tANI_U8  *pTemp = pBuf;
#endif
    if (!pSetContextReq || !pBuf)
        return eSIR_FAILURE;

    pSetContextReq->messageType = limGetU16(pBuf);
    pBuf += sizeof(tANI_U16);

    len = pSetContextReq->length = limGetU16(pBuf);
    pBuf += sizeof(tANI_U16);

    PELOG1(limLog(pMac, LOG1, FL("SME_SETCONTEXT_REQ length %d bytes is:\n"), len);)
    PELOG1(sirDumpBuf(pMac, SIR_LIM_MODULE_ID, LOG3, pTemp, len);)

    if (len < (tANI_S16) sizeof(tANI_U32))
        return eSIR_FAILURE;

    len -= sizeof(tANI_U32); //                    
    if (limCheckRemainingLength(pMac, len) == eSIR_FAILURE)
        return eSIR_FAILURE;

    //                  
    pSetContextReq->sessionId = *pBuf++;
    len--;
    if (limCheckRemainingLength(pMac, len) == eSIR_FAILURE)
        return eSIR_FAILURE;

    //                      
    pSetContextReq->transactionId = sirReadU16N(pBuf);
    pBuf += sizeof(tANI_U16);
    len  -= sizeof(tANI_U16);
    if (limCheckRemainingLength(pMac, len) == eSIR_FAILURE)
        return eSIR_FAILURE;
    palCopyMemory( pMac->hHdd, (tANI_U8 *) pSetContextReq->peerMacAddr,
                   pBuf, sizeof(tSirMacAddr));
    pBuf += sizeof(tSirMacAddr);
    len  -= sizeof(tSirMacAddr);
    if (limCheckRemainingLength(pMac, len) == eSIR_FAILURE)
        return eSIR_FAILURE;
    palCopyMemory( pMac->hHdd, pSetContextReq->bssId, pBuf, sizeof(tSirMacAddr));
    pBuf += sizeof(tSirMacAddr);
    len  -= sizeof(tSirMacAddr);
    if (limCheckRemainingLength(pMac, len) == eSIR_FAILURE)
        return eSIR_FAILURE;

#if defined(ANI_PRODUCT_TYPE_AP)
    pSetContextReq->aid = limGetU16(pBuf);
    pBuf += sizeof(tANI_U16);
    len  -= sizeof(tANI_U16);
    if (limCheckRemainingLength(pMac, len) == eSIR_FAILURE)
        return eSIR_FAILURE;
#endif

//                                                     
//                             

//                                       
//     
//                                                          
//                    
//     

    pSetContextReq->keyMaterial.length = limGetU16(pBuf);
    pBuf += sizeof(tANI_U16);
    len  -= sizeof(tANI_U16);
    if (limCheckRemainingLength(pMac, len) == eSIR_FAILURE)
        return eSIR_FAILURE;

    pSetContextReq->keyMaterial.edType = (tAniEdType) limGetU32(pBuf);
    pBuf += sizeof(tAniEdType);
    len  -= sizeof(tAniEdType);
    if (limCheckRemainingLength(pMac, len) == eSIR_FAILURE)
        return eSIR_FAILURE;

    numKeys = pSetContextReq->keyMaterial.numKeys = *pBuf++;
    len  -= sizeof(numKeys);

    if (numKeys > SIR_MAC_MAX_NUM_OF_DEFAULT_KEYS)
        return eSIR_FAILURE;

    /*                                                                            */
    if( limCheckRemainingLength(pMac, len) == eSIR_FAILURE) {
        tpSirKeys pKeyinfo = pSetContextReq->keyMaterial.key;

        pKeyinfo->keyId = 0;
        pKeyinfo->keyDirection = eSIR_TX_RX; 
        pKeyinfo->keyLength = 0;
            
        if (!limIsAddrBC(pSetContextReq->peerMacAddr))
            pKeyinfo->unicast = 1;
        else
            pKeyinfo->unicast = 0;             
    }else {
        pKeys  = (tANI_U8 *) pSetContextReq->keyMaterial.key;
        do {
            tANI_U32 keySize   = limGetKeysInfo(pMac, (tpSirKeys) pKeys,
                                       pBuf);
            pBuf         += keySize;
            pKeys        += sizeof(tSirKeys);
            totalKeySize += (tANI_U16) keySize;
            if (numKeys == 0)
                break;
            numKeys--;            
        }while (numKeys);
    }
    return eSIR_SUCCESS;
} /*                                  */

/* 
                          
  
           
                                                                    
                              
  
         
  
        
  
              
     
  
       
     
  
                                                              
                                    
                                                       
  
                                                                    
                                                          
                                             
 */

tSirRetStatus
limRemoveKeyReqSerDes(tpAniSirGlobal pMac, tpSirSmeRemoveKeyReq pRemoveKeyReq, tANI_U8 *pBuf)
{
    tANI_S16 len = 0;

#ifdef    PE_DEBUG_LOG1
    tANI_U8  *pTemp = pBuf;
#endif
    if (!pRemoveKeyReq || !pBuf)
        return eSIR_FAILURE;

    pRemoveKeyReq->messageType = limGetU16(pBuf);
    pBuf += sizeof(tANI_U16);

    len = pRemoveKeyReq->length = limGetU16(pBuf);
    pBuf += sizeof(tANI_U16);

    PELOG1(limLog(pMac, LOG1, FL("SME_REMOVEKEY_REQ length %d bytes is:\n"), len);)
    PELOG1(sirDumpBuf(pMac, SIR_LIM_MODULE_ID, LOG1, pTemp, len);)

    if (len < (tANI_S16) sizeof(tANI_U32))
        return eSIR_FAILURE;

    len -= sizeof(tANI_U32); //                    
    if (limCheckRemainingLength(pMac, len) == eSIR_FAILURE)
        return eSIR_FAILURE;

    palCopyMemory( pMac->hHdd, (tANI_U8 *) pRemoveKeyReq->peerMacAddr,
                  pBuf, sizeof(tSirMacAddr));
    pBuf += sizeof(tSirMacAddr);
    len  -= sizeof(tSirMacAddr);
    if (limCheckRemainingLength(pMac, len) == eSIR_FAILURE)
        return eSIR_FAILURE;

#if defined(ANI_PRODUCT_TYPE_AP)
    pRemoveKeyReq->aid = limGetU16(pBuf);
    pBuf += sizeof(tANI_U16);
    len  -= sizeof(tANI_U16);
    if (limCheckRemainingLength(pMac, len) == eSIR_FAILURE)
        return eSIR_FAILURE;
#endif

    pRemoveKeyReq->edType = *pBuf;
    pBuf += sizeof(tANI_U8);
    len -= sizeof(tANI_U8);
    if (limCheckRemainingLength(pMac, len) == eSIR_FAILURE)
        return eSIR_FAILURE;

    pRemoveKeyReq->wepType = *pBuf;

    pBuf += sizeof(tANI_U8);
    len -= sizeof(tANI_U8);
    if (limCheckRemainingLength(pMac, len) == eSIR_FAILURE)
        return eSIR_FAILURE;

    pRemoveKeyReq->keyId = *pBuf;
    
    pBuf += sizeof(tANI_U8);
    len -= sizeof(tANI_U8);
    if (limCheckRemainingLength(pMac, len) == eSIR_FAILURE)
        return eSIR_FAILURE;

    pRemoveKeyReq->unicast = *pBuf;
    len--;
    if (limCheckRemainingLength(pMac, len) == eSIR_FAILURE)
        return eSIR_FAILURE;

    //              
    palCopyMemory( pMac->hHdd, pRemoveKeyReq->bssId, pBuf, sizeof(tSirMacAddr));
    pBuf += sizeof(tSirMacAddr);
    len -= sizeof(tSirMacAddr);
    if (limCheckRemainingLength(pMac, len) == eSIR_FAILURE)
        return eSIR_FAILURE;

    //                  
    pRemoveKeyReq->sessionId = *pBuf++;
    len--;
    if (limCheckRemainingLength(pMac, len) == eSIR_FAILURE)
        return eSIR_FAILURE;

    //                      
    pRemoveKeyReq->transactionId = sirReadU16N(pBuf);
    pBuf += sizeof(tANI_U16);
    len  -= sizeof(tANI_U16);
    if (limCheckRemainingLength(pMac, len) == eSIR_FAILURE)
        return eSIR_FAILURE;
    
    return eSIR_SUCCESS;
} /*                                 */



/* 
                         
  
           
                                                                    
                             
  
         
  
        
  
              
     
  
       
     
  
                                                                      
                                                     
  
                                                                  
                                                        
                                           
 */

tSirRetStatus
limDisassocReqSerDes(tpAniSirGlobal pMac, tSirSmeDisassocReq *pDisassocReq, tANI_U8 *pBuf)
{
    tANI_S16 len = 0;
#ifdef PE_DEBUG_LOG1
    tANI_U8  *pTemp = pBuf;
#endif

    if (!pDisassocReq || !pBuf)
        return eSIR_FAILURE;

    pDisassocReq->messageType = limGetU16(pBuf);
    pBuf += sizeof(tANI_U16);

    len = pDisassocReq->length = limGetU16(pBuf);
    pBuf += sizeof(tANI_U16);

    PELOG1(limLog(pMac, LOG1, FL("SME_DISASSOC_REQ length %d bytes is:\n"), len);)
    PELOG1(sirDumpBuf(pMac, SIR_LIM_MODULE_ID, LOG1, pTemp, len);)

    if (len < (tANI_S16) sizeof(tANI_U32))
        return eSIR_FAILURE;

    len -= sizeof(tANI_U32); //                    
    if (limCheckRemainingLength(pMac, len) == eSIR_FAILURE)
        return eSIR_FAILURE;

    //                  
    pDisassocReq->sessionId = *pBuf;
    pBuf += sizeof(tANI_U8);
    len -= sizeof(tANI_U8);
    if (limCheckRemainingLength(pMac, len) == eSIR_FAILURE)
        return eSIR_FAILURE;

    //                      
    pDisassocReq->transactionId = limGetU16(pBuf);
    pBuf += sizeof(tANI_U16);
    len -= sizeof(tANI_U16);
    if (limCheckRemainingLength(pMac, len) == eSIR_FAILURE)
        return eSIR_FAILURE;

    //              
    palCopyMemory( pMac->hHdd, (tANI_U8 *) pDisassocReq->bssId, pBuf, sizeof(tSirMacAddr));
    pBuf += sizeof(tSirMacAddr);
    len  -= sizeof(tSirMacAddr);
    if (limCheckRemainingLength(pMac, len) == eSIR_FAILURE)
        return eSIR_FAILURE;

    //                    
    palCopyMemory( pMac->hHdd, pDisassocReq->peerMacAddr, pBuf, sizeof(tSirMacAddr));
    pBuf += sizeof(tSirMacAddr);
    len  -= sizeof(tSirMacAddr);
    if (limCheckRemainingLength(pMac, len) == eSIR_FAILURE)
        return eSIR_FAILURE;

    //                   
    pDisassocReq->reasonCode = limGetU16(pBuf);
    pBuf += sizeof(tANI_U16);
    len  -= sizeof(tANI_U16);
    if (limCheckRemainingLength(pMac, len) == eSIR_FAILURE)
        return eSIR_FAILURE;

    pDisassocReq->doNotSendOverTheAir = *pBuf;
    pBuf += sizeof(tANI_U8);
    len -= sizeof(tANI_U8);

#if defined(ANI_PRODUCT_TYPE_AP)
    pDisassocReq->aid = limGetU16(pBuf);
    pBuf += sizeof(tANI_U16);
    len  -= sizeof(tANI_U16);
    if (limCheckRemainingLength(pMac, len) == eSIR_FAILURE)
        return eSIR_FAILURE;

#if (WNI_POLARIS_FW_PACKAGE == ADVANCED)
    pDisassocReq->seqNum = limGetU16(pBuf);
#endif
#endif

    return eSIR_SUCCESS;
} /*                                */



/* 
                       
  
           
                                                                    
                           
  
         
  
        
  
              
     
  
       
     
  
                                                                           
                                                     
  
                                                                  
                                                        
                                           
 */
tSirRetStatus
limDeauthReqSerDes(tpAniSirGlobal pMac, tSirSmeDeauthReq *pDeauthReq, tANI_U8 *pBuf)
{
    tANI_S16 len = 0;

#ifdef  PE_DEBUG_LOG1
    tANI_U8  *pTemp = pBuf;
#endif

    if (!pDeauthReq || !pBuf)
        return eSIR_FAILURE;

    pDeauthReq->messageType = limGetU16(pBuf);
    pBuf += sizeof(tANI_U16);

    len = pDeauthReq->length = limGetU16(pBuf);
    pBuf += sizeof(tANI_U16);

    PELOG1(limLog(pMac, LOG1, FL("SME_DEAUTH_REQ length %d bytes is:\n"), len);)
    PELOG1(sirDumpBuf(pMac, SIR_LIM_MODULE_ID, LOG1, pTemp, len);)

    if (len < (tANI_S16) sizeof(tANI_U32))
        return eSIR_FAILURE;

    len -= sizeof(tANI_U32); //                    
    if (limCheckRemainingLength(pMac, len) == eSIR_FAILURE)
        return eSIR_FAILURE;

    //                  
    pDeauthReq->sessionId = *pBuf++;
    len--;
    if (limCheckRemainingLength(pMac, len) == eSIR_FAILURE)
        return eSIR_FAILURE;

    //                      
    pDeauthReq->transactionId = limGetU16(pBuf);
    pBuf += sizeof(tANI_U16);
    len  -= sizeof(tANI_U16);
    if (limCheckRemainingLength(pMac, len) == eSIR_FAILURE)
        return eSIR_FAILURE;

    //              
    palCopyMemory( pMac->hHdd, pDeauthReq->bssId, pBuf, sizeof(tSirMacAddr));
    pBuf += sizeof(tSirMacAddr);
    len -= sizeof(tSirMacAddr);
    if (limCheckRemainingLength(pMac, len) == eSIR_FAILURE)
        return eSIR_FAILURE;

    //                    
    palCopyMemory( pMac->hHdd, pDeauthReq->peerMacAddr, pBuf, sizeof(tSirMacAddr));
    pBuf += sizeof(tSirMacAddr);
    len -= sizeof(tSirMacAddr);
    if (limCheckRemainingLength(pMac, len) == eSIR_FAILURE)
        return eSIR_FAILURE;

    //                   
    pDeauthReq->reasonCode = limGetU16(pBuf);
    pBuf += sizeof(tANI_U16);
    len  -= sizeof(tANI_U16);

#if defined(ANI_PRODUCT_TYPE_AP)
    if (limCheckRemainingLength(pMac, len) == eSIR_FAILURE)
        return eSIR_FAILURE;
    pDeauthReq->aid = limGetU16(pBuf);
#endif
    
    return eSIR_SUCCESS;
} /*                                */




#if (WNI_POLARIS_FW_PACKAGE == ADVANCED) && defined(ANI_PRODUCT_TYPE_AP)
/* 
                             
  
           
                                                                    
                                                                   
                                                     
  
         
  
        
  
              
     
  
       
     
  
                                                             
  
               
 */

void
limCopyNeighborInfoToCfg(tpAniSirGlobal pMac, tSirNeighborBssInfo neighborBssInfo, tpPESession psessionEntry)
{
    tANI_U32    localPowerConstraints = 0;
    tANI_U16    caps;
    tANI_U8     qosCap = 0;

    if(psessionEntry)
    {
        psessionEntry->gLimPhyMode = neighborBssInfo.nwType;
    }
    else
    {
        pMac->lim.gLimPhyMode = neighborBssInfo.nwType;
    }

    cfgSetCapabilityInfo(pMac, neighborBssInfo.capabilityInfo);

    if (cfgSetStr(pMac, WNI_CFG_SSID, (tANI_U8 *) &neighborBssInfo.ssId.ssId,
                  neighborBssInfo.ssId.length) != eSIR_SUCCESS)
    {
        //                                          
        limLog(pMac, LOGP, FL("could not update SSID at CFG\n"));
    }

    #if 0
    if (cfgSetStr(
           pMac, WNI_CFG_OPERATIONAL_RATE_SET,
           (tANI_U8 *) &neighborBssInfo.operationalRateSet.rate,
           neighborBssInfo.operationalRateSet.numRates) != eSIR_SUCCESS)
    {
        //                                                         
        limLog(pMac, LOGP,
               FL("could not update Operational Rateset at CFG\n"));
    }
    #endif //                  

    if (neighborBssInfo.nwType == WNI_CFG_PHY_MODE_11G)
    {
        if (cfgSetStr(
            pMac, WNI_CFG_EXTENDED_OPERATIONAL_RATE_SET,
            (tANI_U8 *) &neighborBssInfo.extendedRateSet.rate,
            neighborBssInfo.extendedRateSet.numRates) != eSIR_SUCCESS)
        {
            //                                                      
            limLog(pMac, LOGP,
                   FL("could not update Extended Rateset at CFG\n"));
        }
    }
#if (WNI_POLARIS_FW_PACKAGE == ADVANCED)
    if (neighborBssInfo.hcfEnabled)
        LIM_BSS_CAPS_SET(HCF, qosCap);
#endif
    if (neighborBssInfo.wmeInfoPresent || neighborBssInfo.wmeEdcaPresent)
        LIM_BSS_CAPS_SET(WME, qosCap);
    if (LIM_BSS_CAPS_GET(WME, qosCap) && neighborBssInfo.wsmCapablePresent)
        LIM_BSS_CAPS_SET(WSM, qosCap);
    
    pMac->lim.gLimCurrentBssQosCaps = qosCap;
    
    if (neighborBssInfo.wniIndicator)
        pMac->lim.gLimCurrentBssPropCap = neighborBssInfo.propIECapability;
    else
        pMac->lim.gLimCurrentBssPropCap = 0;

    if (neighborBssInfo.HTCapsPresent)
        pMac->lim.htCapabilityPresentInBeacon = 1;
    else
        pMac->lim.htCapabilityPresentInBeacon = 0;
    if (neighborBssInfo.localPowerConstraints && pSessionEntry->lim11hEnable)
    {
        localPowerConstraints = neighborBssInfo.localPowerConstraints;
    }
    if (cfgSetInt(pMac, WNI_CFG_LOCAL_POWER_CONSTRAINT, localPowerConstraints) != eSIR_SUCCESS)
    {
        limLog(pMac, LOGP, FL("Could not update local power constraint to cfg.\n"));
    }
} /*                                    */
#endif


/* 
                  
  
           
                                                                   
                                                 
  
         
  
        
  
              
     
  
       
     
  
                                                              
                                                  
  
               
 */

void
limStatSerDes(tpAniSirGlobal pMac, tpAniStaStatStruct pStat, tANI_U8 *pBuf)
{
#ifdef  PE_DEBUG_LOG1
    tANI_U8  *pTemp = pBuf;
#endif

    limCopyU32(pBuf, pStat->sentAesBlksUcastHi);
    pBuf += sizeof(tANI_U32);

    limCopyU32(pBuf, pStat->sentAesBlksUcastLo);
    pBuf += sizeof(tANI_U32);

    limCopyU32(pBuf, pStat->recvAesBlksUcastHi);
    pBuf += sizeof(tANI_U32);

    limCopyU32(pBuf, pStat->recvAesBlksUcastLo);
    pBuf += sizeof(tANI_U32);

    limCopyU32(pBuf, pStat->aesFormatErrorUcastCnts);
    pBuf += sizeof(tANI_U32);

    limCopyU32(pBuf, pStat->aesReplaysUcast);
    pBuf += sizeof(tANI_U32);

    limCopyU32(pBuf, pStat->aesDecryptErrUcast);
    pBuf += sizeof(tANI_U32);

    limCopyU32(pBuf, pStat->singleRetryPkts);
    pBuf += sizeof(tANI_U32);

    limCopyU32(pBuf, pStat->failedTxPkts);
    pBuf += sizeof(tANI_U32);

    limCopyU32(pBuf, pStat->ackTimeouts);
    pBuf += sizeof(tANI_U32);

    limCopyU32(pBuf, pStat->multiRetryPkts);
    pBuf += sizeof(tANI_U32);

    limCopyU32(pBuf, pStat->fragTxCntsHi);
    pBuf += sizeof(tANI_U32);

    limCopyU32(pBuf, pStat->fragTxCntsLo);
    pBuf += sizeof(tANI_U32);

    limCopyU32(pBuf, pStat->transmittedPktsHi);
    pBuf += sizeof(tANI_U32);

    limCopyU32(pBuf, pStat->transmittedPktsLo);
    pBuf += sizeof(tANI_U32);

    limCopyU32(pBuf, pStat->phyStatHi);
    pBuf += sizeof(tANI_U32);

    limCopyU32(pBuf, pStat->phyStatLo);
    pBuf += sizeof(tANI_U32);

    limCopyU32(pBuf, pStat->uplinkRssi);
    pBuf += sizeof(tANI_U32);

    limCopyU32(pBuf, pStat->uplinkSinr);
    pBuf += sizeof(tANI_U32);

    limCopyU32(pBuf, pStat->uplinkRate);
    pBuf += sizeof(tANI_U32);

    limCopyU32(pBuf, pStat->downlinkRssi);
    pBuf += sizeof(tANI_U32);

    limCopyU32(pBuf, pStat->downlinkSinr);
    pBuf += sizeof(tANI_U32);

    limCopyU32(pBuf, pStat->downlinkRate);
    pBuf += sizeof(tANI_U32);

    limCopyU32(pBuf, pStat->nRcvBytes);
    pBuf += sizeof(tANI_U32);

    limCopyU32(pBuf, pStat->nXmitBytes);
    pBuf += sizeof(tANI_U32);

    PELOG1(limLog(pMac, LOG1, FL("STAT: length %d bytes is:\n"), sizeof(tAniStaStatStruct));)
    PELOG1(sirDumpBuf(pMac, SIR_LIM_MODULE_ID, LOG1, pTemp,  sizeof(tAniStaStatStruct));)

} /*                         */


#if (WNI_POLARIS_FW_PACKAGE == ADVANCED) && defined(ANI_PRODUCT_TYPE_AP)
/* 
                                  
  
           
                                                           
                                                         
               
  
        
  
              
     
  
       
     
  
                                                  
                                                            
                                                               
                                                       
                                                                         
                                                                               
 */

tSirRetStatus
limSmeWmStatusChangeHeaderSerDes(tpAniSirGlobal pMac,
                                 tSirSmeStatusChangeCode statusChangeCode,
                                 tANI_U8 *pBuf,
                                 tANI_U16 *len,
                                 tANI_U32 buflen,
                                 tANI_U8 sessionId)
{
    if (buflen < ((sizeof(tANI_U16) * 2) + sizeof(tANI_U32)))
    {
        limLog(pMac, LOGE,
               FL("limSmeWmStatusChangeHeaderSerDes: no enough space (buf %d) \n"), buflen);
        return eSIR_FAILURE;
    }

    *len = 0;

    limCopyU16(pBuf, eWNI_SME_WM_STATUS_CHANGE_NTF);
    pBuf += sizeof(tANI_U16);
    *len += sizeof(tANI_U16);

    //                                          
    pBuf += sizeof(tANI_U16);
    *len += sizeof(tANI_U16);
    
    *pBuf++ = sessionId;
    *len += sizeof(tANI_U8);

    limCopyU32(pBuf, statusChangeCode);
    pBuf += sizeof(tANI_U32);
    *len += sizeof(tANI_U32);

    return eSIR_SUCCESS;
} /*                                            */



/* 
                       
  
           
                                                           
                                     
  
        
  
              
     
  
       
     
  
                                                 
                                                     
                                                                         
                                         
  
                                                                  
                                                                        
 */

tSirRetStatus
limRadioInfoSerDes(tpAniSirGlobal pMac, tpSirRadarInfo pRadarInfo,
                              tANI_U8 *pBuf, tANI_U16 *len, tANI_U32 buflen)
{

    if (buflen < sizeof(tSirRadarInfo))
    {
        limLog(pMac, LOGE,
               FL("no enough space (buf %d) \n"), buflen);
        return eSIR_FAILURE;
    }

    *pBuf = pRadarInfo->channelNumber;
    pBuf += sizeof(tANI_U8);
    *len += sizeof(tANI_U8);

    limCopyU16(pBuf, pRadarInfo->radarPulseWidth);
    pBuf += sizeof(tANI_U16);
    *len += sizeof(tANI_U16);

    limCopyU16(pBuf, pRadarInfo->numRadarPulse);
    pBuf += sizeof(tANI_U16);
    *len += sizeof(tANI_U16);

    return eSIR_SUCCESS;
} /*                              */
#endif


/* 
                               
  
           
                                                           
                                             
  
 */
void
limPackBkgndScanFailNotify(tpAniSirGlobal pMac,
                           tSirSmeStatusChangeCode statusChangeCode,
                           tpSirBackgroundScanInfo pScanInfo,
                           tSirSmeWmStatusChangeNtf *pSmeNtf,
                           tANI_U8 sessionId)
{

    tANI_U16    length = (sizeof(tANI_U16) * 2) + sizeof(tANI_U8) +
                    sizeof(tSirSmeStatusChangeCode) +
                    sizeof(tSirBackgroundScanInfo);

#if defined (ANI_PRODUCT_TYPE_AP) && defined(ANI_LITTLE_BYTE_ENDIAN)
        sirStoreU16N((tANI_U8*)&pSmeNtf->messageType, eWNI_SME_WM_STATUS_CHANGE_NTF );
        sirStoreU16N((tANI_U8*)&pSmeNtf->length, length);
        pSmeNtf->sessionId = sessionId;
        sirStoreU32N((tANI_U8*)&pSmeNtf->statusChangeCode, statusChangeCode);

        sirStoreU32N((tANI_U8*)&pSmeNtf->statusChangeInfo.bkgndScanInfo.numOfScanSuccess,
                     pScanInfo->numOfScanSuccess);
        sirStoreU32N((tANI_U8*)&pSmeNtf->statusChangeInfo.bkgndScanInfo.numOfScanFailure,
                     pScanInfo->numOfScanFailure);
        sirStoreU32N((tANI_U8*)&pSmeNtf->statusChangeInfo.bkgndScanInfo.reserved,
                     pScanInfo->reserved);
#else
        pSmeNtf->messageType = eWNI_SME_WM_STATUS_CHANGE_NTF;
        pSmeNtf->statusChangeCode = statusChangeCode;
        pSmeNtf->length = length;
        pSmeNtf->sessionId = sessionId;
        pSmeNtf->statusChangeInfo.bkgndScanInfo.numOfScanSuccess = pScanInfo->numOfScanSuccess;
        pSmeNtf->statusChangeInfo.bkgndScanInfo.numOfScanFailure = pScanInfo->numOfScanFailure;
        pSmeNtf->statusChangeInfo.bkgndScanInfo.reserved = pScanInfo->reserved;
#endif
}

#if (WNI_POLARIS_FW_PACKAGE == ADVANCED) && defined(ANI_PRODUCT_TYPE_AP)

/* 
                           
  
           
                                                           
                                     
                                       
                                                   
  
        
  
              
     
  
       
     
  
                                                            
                                                          
                                        
                                         
                                        
  
                                                                 
                                                                       
 */

tSirRetStatus nonTitanBssFoundSerDes( tpAniSirGlobal pMac,
    tpSirNeighborBssWdsInfo pNewBssInfo,
    tANI_U8 *pBuf,
    tANI_U16 *len,
    tANI_U8 sessionId )
{
tANI_U8 *pTemp = pBuf;
tANI_U16 length = 0;
tANI_U32 bufLen = sizeof( tSirSmeWmStatusChangeNtf );

  //                                                 
  if( eSIR_SUCCESS != limSmeWmStatusChangeHeaderSerDes(
        pMac,
        eSIR_SME_CB_LEGACY_BSS_FOUND_BY_AP,
        pBuf,
        &length,
        bufLen,
        sessionId))
  {
    limLog( pMac, LOGE,
        FL("Header SerDes failed \n"));
    return eSIR_FAILURE;
  }
  pBuf += length;
  bufLen -= length;

  //                                 
  length = limCopyNeighborWdsInfo( pMac,
      pBuf,
      pNewBssInfo );
  pBuf += length;
  bufLen -= length;

  //                                       
  pBuf = pTemp;
  pBuf += sizeof(tANI_U16);
  limCopyU16(pBuf, length);

  //                                       
  *len = length;

  return eSIR_SUCCESS;
} /*                                  */

/* 
                                  
  
           
                                                             
                                                         
  
        
                                                         
  
              
  
       
  
                                                                       
                                                                   
                                                                    
                                 
 */

tANI_BOOLEAN
limIsSmeSwitchChannelReqValid(tpAniSirGlobal pMac, 
                              tANI_U8 *pBuf, 
                              tpSirSmeSwitchChannelReq pSmeMsg)
{
    pSmeMsg->messageType = limGetU16(pBuf);
    pBuf += sizeof(tANI_U16);  

    pSmeMsg->length = limGetU16(pBuf); 
    pBuf += sizeof(tANI_U16);

    pSmeMsg->transactionId = limGetU16(pBuf); 
    pBuf += sizeof(tANI_U16);

    pSmeMsg->channelId = *pBuf;
    pBuf++;
  
    pSmeMsg->cbMode = limGetU32(pBuf);
    pBuf += sizeof(tANI_U32);

    pSmeMsg->dtimFactor = limGetU32(pBuf);
    pBuf += sizeof(tANI_U32);

    if (pSmeMsg->length != SIR_SME_CHANNEL_SWITCH_SIZE)
    {
        PELOGE(limLog(pMac, LOGE, FL("Invalid length %d \n"), pSmeMsg->length);)
        return eANI_BOOLEAN_FALSE;
    }

    //                                   
    if (pSmeMsg->dtimFactor > SIR_MAX_DTIM_FACTOR)
    {
        PELOGE(limLog(pMac, LOGE, FL("Invalid dtimFactor %d \n"), pSmeMsg->dtimFactor);)
        return eANI_BOOLEAN_FALSE;
    }

    return eANI_BOOLEAN_TRUE;
}

#endif

#ifdef WLAN_SOFTAP_FEATURE
/* 
                                 
  
           
                                                             
                                                             
  
        
                                                         
  
              
  
       
  
                                                                           
                                                                  
                                                                        
                                 
 */
tANI_BOOLEAN
limIsSmeGetAssocSTAsReqValid(tpAniSirGlobal pMac, tpSirSmeGetAssocSTAsReq pGetAssocSTAsReq, tANI_U8 *pBuf)
{
    tANI_S16 len = 0;

    pGetAssocSTAsReq->messageType = limGetU16(pBuf);
    pBuf += sizeof(tANI_U16);

    len = pGetAssocSTAsReq->length = limGetU16(pBuf);
    pBuf += sizeof(tANI_U16);

    if (len < (tANI_S16) sizeof(tANI_U32))
        return eSIR_FAILURE;

    len -= sizeof(tANI_U32); //                    
    if (limCheckRemainingLength(pMac, len) == eSIR_FAILURE)
        return eSIR_FAILURE;

    //              
    palCopyMemory( pMac->hHdd, (tANI_U8 *) pGetAssocSTAsReq->bssId, pBuf, sizeof(tSirMacAddr));
    pBuf += sizeof(tSirMacAddr);
    len  -= sizeof(tSirMacAddr);
    if (limCheckRemainingLength(pMac, len) == eSIR_FAILURE)
        return eSIR_FAILURE;

    //              
    pGetAssocSTAsReq->modId = limGetU16(pBuf);
    pBuf += sizeof(tANI_U16);
    len  -= sizeof(tANI_U16);
    if (limCheckRemainingLength(pMac, len) == eSIR_FAILURE)
        return eSIR_FAILURE;

    //                    
    pGetAssocSTAsReq->pUsrContext = (void *)limGetU32(pBuf);
    pBuf += sizeof(tANI_U32);
    len  -= sizeof(tANI_U32);
    if (limCheckRemainingLength(pMac, len) == eSIR_FAILURE)
        return eSIR_FAILURE;

    //                          
    pGetAssocSTAsReq->pSapEventCallback = (void *)limGetU32(pBuf);
    pBuf += sizeof(tANI_U32);
    len  -= sizeof(tANI_U32);
    if (limCheckRemainingLength(pMac, len) == eSIR_FAILURE)
        return eSIR_FAILURE;

    //                        
    pGetAssocSTAsReq->pAssocStasArray = (void *)limGetU32(pBuf);
    pBuf += sizeof(tANI_U32);
    len  -= sizeof(tANI_U32);

    PELOG1(limLog(pMac, LOG1, FL("SME_GET_ASSOC_STAS_REQ length consumed %d bytes \n"), len);)

    if (len < 0)
    {
        PELOGE(limLog(pMac, LOGE, FL("SME_GET_ASSOC_STAS_REQ invalid length\n"));)
        return eANI_BOOLEAN_FALSE;
    }

    return eANI_BOOLEAN_TRUE;
}

/* 
                             
  
           
                                                                    
                                            
  
         
  
        
  
              
     
  
       
     
  
                                                                                     
                                                                
                                                                             
                                                                   
                                                      
 */
tSirRetStatus
limTkipCntrMeasReqSerDes(tpAniSirGlobal pMac, tpSirSmeTkipCntrMeasReq  pTkipCntrMeasReq, tANI_U8 *pBuf)
{
    tANI_S16 len = 0;

#ifdef PE_DEBUG_LOG1
    tANI_U8  *pTemp = pBuf;
#endif

    pTkipCntrMeasReq->messageType = limGetU16(pBuf);
    pBuf += sizeof(tANI_U16);

    len = pTkipCntrMeasReq->length = limGetU16(pBuf);
    pBuf += sizeof(tANI_U16);

    PELOG1(limLog(pMac, LOG1, FL("SME_TKIP_CNTR_MEAS_REQ length %d bytes is:\n"), len);)
    PELOG1(sirDumpBuf(pMac, SIR_LIM_MODULE_ID, LOG1, pTemp, len);)

    if (len < (tANI_S16) sizeof(tANI_U32))
        return eSIR_FAILURE;

    len -= sizeof(tANI_U32); //                    
    if (limCheckRemainingLength(pMac, len) == eSIR_FAILURE)
        return eSIR_FAILURE;

    //                  
    pTkipCntrMeasReq->sessionId = *pBuf++;
    len--;
    if (limCheckRemainingLength(pMac, len) == eSIR_FAILURE)
        return eSIR_FAILURE;

    //                      
    pTkipCntrMeasReq->transactionId = limGetU16(pBuf);
    pBuf += sizeof(tANI_U16);
    len -= sizeof(tANI_U16); 
    if (limCheckRemainingLength(pMac, len) == eSIR_FAILURE)
        return eSIR_FAILURE;

    //              
    palCopyMemory( pMac->hHdd, (tANI_U8 *) pTkipCntrMeasReq->bssId, pBuf, sizeof(tSirMacAddr));
    pBuf += sizeof(tSirMacAddr);
    len -= sizeof(tSirMacAddr);
    if (limCheckRemainingLength(pMac, len) == eSIR_FAILURE)
        return eSIR_FAILURE;

    //                
    pTkipCntrMeasReq->bEnable = *pBuf++;
    len --;

    PELOG1(limLog(pMac, LOG1, FL("SME_TKIP_CNTR_MEAS_REQ length consumed %d bytes \n"), len);)
    
    if (len)
    {
        PELOGE(limLog(pMac, LOGE, FL("SME_TKIP_CNTR_MEAS_REQ invalid \n"));)
        return eSIR_FAILURE;
    }
    else
        return eSIR_SUCCESS;
}

/* 
                                      
  
           
                                                                   
                                                                        
  
        
                                                         
  
              
  
       
  
                                                                               
                                                                                     
                                                                            
                                 
 */
 
tSirRetStatus
limIsSmeGetWPSPBCSessionsReqValid(tpAniSirGlobal pMac, tSirSmeGetWPSPBCSessionsReq *pGetWPSPBCSessionsReq, tANI_U8 *pBuf)
{
    tANI_S16 len = 0;

    PELOG1(sirDumpBuf(pMac, SIR_LIM_MODULE_ID, LOG1, pBuf, sizeof(tSirSmeGetWPSPBCSessionsReq));)
    
    pGetWPSPBCSessionsReq->messageType = limGetU16(pBuf);
    pBuf += sizeof(tANI_U16);

    len = pGetWPSPBCSessionsReq->length = limGetU16(pBuf);
    pBuf += sizeof(tANI_U16);

    if (len < (tANI_S16) sizeof(tANI_U32))
        return eSIR_FAILURE;

    len -= sizeof(tANI_U32); //                    
    if (limCheckRemainingLength(pMac, len) == eSIR_FAILURE)
        return eSIR_FAILURE;

   //                    
    pGetWPSPBCSessionsReq->pUsrContext = (void *)limGetU32(pBuf);
    pBuf += sizeof(tANI_U32);
    len  -= sizeof(tANI_U32);
    if (limCheckRemainingLength(pMac, len) == eSIR_FAILURE)
        return eSIR_FAILURE;

    //                          
    pGetWPSPBCSessionsReq->pSapEventCallback = (void *)limGetU32(pBuf);
    pBuf += sizeof(tANI_U32);
    len  -= sizeof(tANI_U32);
    if (limCheckRemainingLength(pMac, len) == eSIR_FAILURE)
        return eSIR_FAILURE;

    //              
    palCopyMemory( pMac->hHdd, (tANI_U8 *) pGetWPSPBCSessionsReq->bssId, pBuf, sizeof(tSirMacAddr));
    pBuf += sizeof(tSirMacAddr);
    len  -= sizeof(tSirMacAddr);
 
    //                                             
    palCopyMemory( pMac->hHdd, (tANI_U8 *) pGetWPSPBCSessionsReq->pRemoveMac, pBuf, sizeof(tSirMacAddr));
    pBuf += sizeof(tSirMacAddr);
    len  -= sizeof(tSirMacAddr);
    
    PELOG1(limLog(pMac, LOG1, FL("SME_GET_ASSOC_STAS_REQ length consumed %d bytes \n"), len);)

    if (len < 0)
    {
        PELOGE(limLog(pMac, LOGE, FL("SME_GET_WPSPBC_SESSION_REQ invalid length\n"));)
        return eSIR_FAILURE;
    }

    return eSIR_SUCCESS;
}

#endif

/*                                                                
                         
                                                             
                                                              
                     
                                 
                                   
                               
                                   
                                             
                                                     
                                                 
                                                     
            
                                                                  */
void
limGetSessionInfo(tpAniSirGlobal pMac, tANI_U8 *pBuf, tANI_U8 *sessionId, tANI_U16 *transactionId)
{
    if (!pBuf)
    {
        limLog(pMac, LOGE, FL("NULL ptr received. \n"));
        return;
    }

    pBuf += sizeof(tANI_U16);   //                   
    pBuf += sizeof(tANI_U16);   //                    

    *sessionId = *pBuf;            //              
    pBuf++;    
    *transactionId = limGetU16(pBuf);  //                  

    return;
}

#ifdef WLAN_SOFTAP_FEATURE

/* 
                               
  
           
                                                         
  
         
  
        
  
              
     
  
       
     
  
                                                                     
                                    
                                                       
  
                                                                    
                                                          
                                             
 */

tSirRetStatus
limUpdateAPWPSIEsReqSerDes(tpAniSirGlobal pMac, tpSirUpdateAPWPSIEsReq pUpdateAPWPSIEsReq, tANI_U8 *pBuf)
{
    tANI_S16 len = 0;

    PELOG1(sirDumpBuf(pMac, SIR_LIM_MODULE_ID, LOG1, pBuf, sizeof(tSirUpdateAPWPSIEsReq));)

    if (!pUpdateAPWPSIEsReq || !pBuf)
        return eSIR_FAILURE;

    pUpdateAPWPSIEsReq->messageType = limGetU16(pBuf);
    pBuf += sizeof(tANI_U16);

    len = pUpdateAPWPSIEsReq->length = limGetU16(pBuf);
    pBuf += sizeof(tANI_U16);

    if (len < (tANI_S16) sizeof(tANI_U32))
        return eSIR_FAILURE;

    len -= sizeof(tANI_U32); //                    
    if (limCheckRemainingLength(pMac, len) == eSIR_FAILURE)
        return eSIR_FAILURE;
        
    //                      
    pUpdateAPWPSIEsReq->transactionId = limGetU16( pBuf );
    pBuf += sizeof( tANI_U16 );
    len -= sizeof( tANI_U16 );
    if (limCheckRemainingLength(pMac, len) == eSIR_FAILURE)
        return eSIR_FAILURE;

    //              
    palCopyMemory( pMac->hHdd, (tANI_U8 *) pUpdateAPWPSIEsReq->bssId, pBuf, sizeof(tSirMacAddr));
    pBuf += sizeof(tSirMacAddr);
    len  -= sizeof(tSirMacAddr);
    if (limCheckRemainingLength(pMac, len) == eSIR_FAILURE)
        return eSIR_FAILURE;

    //                  
    pUpdateAPWPSIEsReq->sessionId = *pBuf++;
    len--;
    if (limCheckRemainingLength(pMac, len) == eSIR_FAILURE)
        return eSIR_FAILURE;

    //                 
    palCopyMemory( pMac->hHdd, (tSirAPWPSIEs *) &pUpdateAPWPSIEsReq->APWPSIEs, pBuf, sizeof(tSirAPWPSIEs));
    pBuf += sizeof(tSirAPWPSIEs);
    len  -= sizeof(tSirAPWPSIEs);

    PELOG1(limLog(pMac, LOG1, FL("SME_UPDATE_APWPSIE_REQ length consumed %d bytes \n"), len);)

    if (len < 0)
    {
        PELOGE(limLog(pMac, LOGE, FL("SME_UPDATE_APWPSIE_REQ invalid length\n"));)
        return eSIR_FAILURE;
    }

    return eSIR_SUCCESS;
} /*                                  */

/* 
                                   
  
           
                                                         
  
         
  
        
  
              
     
  
       
     
  
                                                                            
                                    
                                                       
  
                                                                    
                                                          
                                             
 */

tSirRetStatus
limUpdateAPWPARSNIEsReqSerDes(tpAniSirGlobal pMac, tpSirUpdateAPWPARSNIEsReq pUpdateAPWPARSNIEsReq, tANI_U8 *pBuf)
{
    tANI_S16 len = 0;

    PELOG1(sirDumpBuf(pMac, SIR_LIM_MODULE_ID, LOG1, pBuf, sizeof(tSirUpdateAPWPARSNIEsReq));)

    if (!pUpdateAPWPARSNIEsReq || !pBuf)
        return eSIR_FAILURE;

    pUpdateAPWPARSNIEsReq->messageType = limGetU16(pBuf);
    pBuf += sizeof(tANI_U16);

    len = pUpdateAPWPARSNIEsReq->length = limGetU16(pBuf);
    pBuf += sizeof(tANI_U16);

    if (len < (tANI_S16) sizeof(tANI_U32))
        return eSIR_FAILURE;

    len -= sizeof(tANI_U32); //                    
    if (limCheckRemainingLength(pMac, len) == eSIR_FAILURE)
        return eSIR_FAILURE;

    //                      
    pUpdateAPWPARSNIEsReq->transactionId = limGetU16( pBuf );
    pBuf += sizeof(tANI_U16);
    len -= sizeof( tANI_U16 );
    if (limCheckRemainingLength(pMac, len) == eSIR_FAILURE)
        return eSIR_FAILURE;

    //              
    palCopyMemory( pMac->hHdd, (tANI_U8 *) pUpdateAPWPARSNIEsReq->bssId, pBuf, sizeof(tSirMacAddr));
    pBuf += sizeof(tSirMacAddr);
    len  -= sizeof(tSirMacAddr);
    if (limCheckRemainingLength(pMac, len) == eSIR_FAILURE)
        return eSIR_FAILURE;

    //                  
    pUpdateAPWPARSNIEsReq->sessionId = *pBuf++;
    len--;
    if (limCheckRemainingLength(pMac, len) == eSIR_FAILURE)
        return eSIR_FAILURE;

    //                    
    palCopyMemory( pMac->hHdd, (tSirRSNie *) &pUpdateAPWPARSNIEsReq->APWPARSNIEs, pBuf, sizeof(tSirRSNie));
    pBuf += sizeof(tSirRSNie);
    len  -= sizeof(tSirRSNie);

    if (len < 0)
    {
        PELOGE(limLog(pMac, LOGE, FL("SME_GET_WPSPBC_SESSION_REQ invalid length\n"));)
        return eSIR_FAILURE;
    }

    return eSIR_SUCCESS;
} /*                                         */

#endif
