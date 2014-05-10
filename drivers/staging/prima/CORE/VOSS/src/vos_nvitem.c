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

/*============================================================================
  FILE:         vos_nvitem.c
  OVERVIEW:     This source file contains definitions for vOS NV Item APIs
  DEPENDENCIES: NV, remote API client, WinCE REX
                Copyright (c) 2008 QUALCOMM Incorporated.
                All Rights Reserved.
                Qualcomm Confidential and Proprietary
============================================================================*/
/*                                                                            
                         
                                                                            */
//                                                                       
//                         

/*                                                                            
                
                                                                            */
#include "vos_types.h"
#include "aniGlobal.h"
#include "vos_nvitem.h"
#include "vos_trace.h"
#include "vos_api.h"
#include "wlan_hdd_misc.h"
#include "vos_sched.h"
 
/*                                                                            
                                         
                                                                            */
#define VALIDITY_BITMAP_NV_ID    NV_WLAN_VALIDITY_BITMAP_I
#define VALIDITY_BITMAP_SIZE     32
#define MAX_COUNTRY_COUNT        300
//                                                 
#define VOS_HARD_CODED_MAC    {0, 0x0a, 0xf5, 4, 5, 6}

#define DEFAULT_NV_VALIDITY_BITMAP 0xFFFFFFFF

/*                                                                            
                    
                                                                            */
//                                                              
//                                                                        
//                                                                      

//                                                             
typedef struct
{
   v_U8_t            regDomain;
   v_COUNTRYCODE_t   countryCode;
} CountryInfo_t;
//                                                             
typedef struct
{
   v_U16_t           countryCount;
   CountryInfo_t     countryInfo[MAX_COUNTRY_COUNT];
} CountryInfoTable_t;
/*                                                                            
                          
                                                                            */
/*                                                                            
                              
                                                                            */
//                             
//                                                
//                                             
static CountryInfoTable_t countryInfoTable =
{
    254,
    {
        { REGDOMAIN_FCC,     {'U', 'S'}},  //                                    
        { REGDOMAIN_ETSI,    {'A', 'D'}},  //       
        { REGDOMAIN_ETSI,    {'A', 'E'}},  //   
        { REGDOMAIN_N_AMER_EXC_FCC, {'A', 'F'}},  //           
        { REGDOMAIN_WORLD,   {'A', 'G'}},  //                   
        { REGDOMAIN_FCC,     {'A', 'I'}},  //        
        { REGDOMAIN_NO_5GHZ, {'A', 'L'}},  //       
        { REGDOMAIN_N_AMER_EXC_FCC, {'A', 'M'}},  //       
        { REGDOMAIN_ETSI,    {'A', 'N'}},  //                    
        { REGDOMAIN_NO_5GHZ, {'A', 'O'}},  //      
        { REGDOMAIN_WORLD,   {'A', 'Q'}},  //          
        { REGDOMAIN_WORLD,   {'A', 'R'}},  //         
        { REGDOMAIN_FCC,     {'A', 'S'}},  //              
        { REGDOMAIN_ETSI,    {'A', 'T'}},  //       
        { REGDOMAIN_APAC,    {'A', 'U'}},  //         
        { REGDOMAIN_ETSI,    {'A', 'W'}},  //     
        { REGDOMAIN_WORLD,   {'A', 'X'}},  //             
        { REGDOMAIN_N_AMER_EXC_FCC, {'A', 'Z'}},  //          
        { REGDOMAIN_ETSI,    {'B', 'A'}},  //                      
        { REGDOMAIN_APAC,    {'B', 'B'}},  //        
        { REGDOMAIN_NO_5GHZ, {'B', 'D'}},  //          
        { REGDOMAIN_ETSI,    {'B', 'E'}},  //       
        { REGDOMAIN_HI_5GHZ, {'B', 'F'}},  //            
        { REGDOMAIN_ETSI,    {'B', 'G'}},  //        
        { REGDOMAIN_APAC,    {'B', 'H'}},  //       
        { REGDOMAIN_NO_5GHZ, {'B', 'I'}},  //       
        { REGDOMAIN_NO_5GHZ, {'B', 'J'}},  //     
        { REGDOMAIN_FCC,     {'B', 'M'}},  //       
        { REGDOMAIN_APAC,    {'B', 'N'}},  //                 
        { REGDOMAIN_HI_5GHZ, {'B', 'O'}},  //       
        { REGDOMAIN_WORLD,   {'B', 'R'}},  //      
        { REGDOMAIN_APAC,    {'B', 'S'}},  //       
        { REGDOMAIN_NO_5GHZ, {'B', 'T'}},  //      
        { REGDOMAIN_WORLD,   {'B', 'V'}},  //             
        { REGDOMAIN_ETSI,    {'B', 'W'}},  //        
        { REGDOMAIN_ETSI,    {'B', 'Y'}},  //       
        { REGDOMAIN_HI_5GHZ, {'B', 'Z'}},  //      
        { REGDOMAIN_FCC,     {'C', 'A'}},  //      
        { REGDOMAIN_WORLD,   {'C', 'C'}},  //                       
        { REGDOMAIN_NO_5GHZ, {'C', 'D'}},  //                                     
        { REGDOMAIN_NO_5GHZ, {'C', 'F'}},  //                        
        { REGDOMAIN_NO_5GHZ, {'C', 'G'}},  //     
        { REGDOMAIN_ETSI,    {'C', 'H'}},  //           
        { REGDOMAIN_NO_5GHZ, {'C', 'I'}},  //             
        { REGDOMAIN_WORLD,   {'C', 'K'}},  //            
        { REGDOMAIN_APAC,    {'C', 'L'}},  //     
        { REGDOMAIN_NO_5GHZ, {'C', 'M'}},  //        
        { REGDOMAIN_HI_5GHZ, {'C', 'N'}},  //     
        { REGDOMAIN_APAC,    {'C', 'O'}},  //        
        { REGDOMAIN_APAC,    {'C', 'R'}},  //          
        { REGDOMAIN_NO_5GHZ, {'C', 'U'}},  //    
        { REGDOMAIN_ETSI,    {'C', 'V'}},  //          
        { REGDOMAIN_WORLD,   {'C', 'X'}},  //                
        { REGDOMAIN_ETSI,    {'C', 'Y'}},  //      
        { REGDOMAIN_ETSI,    {'C', 'Z'}},  //              
        { REGDOMAIN_ETSI,    {'D', 'E'}},  //       
        { REGDOMAIN_NO_5GHZ, {'D', 'J'}},  //        
        { REGDOMAIN_ETSI,    {'D', 'K'}},  //       
        { REGDOMAIN_WORLD,   {'D', 'M'}},  //        
        { REGDOMAIN_APAC,    {'D', 'O'}},  //                  
        { REGDOMAIN_NO_5GHZ, {'D', 'Z'}},  //       
        { REGDOMAIN_APAC,    {'E', 'C'}},  //       
        { REGDOMAIN_ETSI,    {'E', 'E'}},  //       
        { REGDOMAIN_N_AMER_EXC_FCC, {'E', 'G'}},  //     
        { REGDOMAIN_WORLD,   {'E', 'H'}},  //              
        { REGDOMAIN_NO_5GHZ, {'E', 'R'}},  //       
        { REGDOMAIN_ETSI,    {'E', 'S'}},  //     
        { REGDOMAIN_ETSI,    {'E', 'T'}},  //        
        { REGDOMAIN_ETSI,    {'E', 'U'}},  //              
        { REGDOMAIN_ETSI,    {'F', 'I'}},  //       
        { REGDOMAIN_NO_5GHZ, {'F', 'J'}},  //    
        { REGDOMAIN_WORLD,   {'F', 'K'}},  //                           
        { REGDOMAIN_WORLD,   {'F', 'M'}},  //                               
        { REGDOMAIN_WORLD,   {'F', 'O'}},  //             
        { REGDOMAIN_ETSI,    {'F', 'R'}},  //      
        { REGDOMAIN_NO_5GHZ, {'G', 'A'}},  //     
        { REGDOMAIN_ETSI,    {'G', 'B'}},  //              
        { REGDOMAIN_WORLD,   {'G', 'D'}},  //       
        { REGDOMAIN_ETSI,    {'G', 'E'}},  //       
        { REGDOMAIN_ETSI,    {'G', 'F'}},  //             
        { REGDOMAIN_WORLD,   {'G', 'G'}},  //        
        { REGDOMAIN_WORLD,   {'G', 'H'}},  //     
        { REGDOMAIN_WORLD,   {'G', 'I'}},  //         
        { REGDOMAIN_ETSI,    {'G', 'L'}},  //         
        { REGDOMAIN_NO_5GHZ, {'G', 'M'}},  //      
        { REGDOMAIN_NO_5GHZ, {'G', 'N'}},  //      
        { REGDOMAIN_ETSI,    {'G', 'P'}},  //          
        { REGDOMAIN_NO_5GHZ, {'G', 'Q'}},  //                 
        { REGDOMAIN_ETSI,    {'G', 'R'}},  //      
        { REGDOMAIN_WORLD,   {'G', 'S'}},  //                                            
        { REGDOMAIN_APAC,    {'G', 'T'}},  //         
        { REGDOMAIN_APAC,    {'G', 'U'}},  //    
        { REGDOMAIN_NO_5GHZ, {'G', 'W'}},  //             
        { REGDOMAIN_HI_5GHZ, {'G', 'Y'}},  //      
        { REGDOMAIN_WORLD,   {'H', 'K'}},  //        
        { REGDOMAIN_WORLD,   {'H', 'M'}},  //                                 
        { REGDOMAIN_WORLD,   {'H', 'N'}},  //        
        { REGDOMAIN_ETSI,    {'H', 'R'}},  //       
        { REGDOMAIN_ETSI,    {'H', 'T'}},  //     
        { REGDOMAIN_ETSI,    {'H', 'U'}},  //       
        { REGDOMAIN_HI_5GHZ, {'I', 'D'}},  //         
        { REGDOMAIN_ETSI,    {'I', 'E'}},  //       
        { REGDOMAIN_NO_5GHZ, {'I', 'L'}},  //      
        { REGDOMAIN_WORLD,   {'I', 'M'}},  //           
        { REGDOMAIN_APAC,    {'I', 'N'}},  //     
        { REGDOMAIN_WORLD,   {'I', 'O'}},  //                              
        { REGDOMAIN_NO_5GHZ, {'I', 'Q'}},  //    
        { REGDOMAIN_HI_5GHZ, {'I', 'R'}},  //                         
        { REGDOMAIN_ETSI,    {'I', 'S'}},  //       
        { REGDOMAIN_ETSI,    {'I', 'T'}},  //     
        { REGDOMAIN_JAPAN,   {'J', '1'}},  //                 
        { REGDOMAIN_JAPAN,   {'J', '2'}},  //                 
        { REGDOMAIN_JAPAN,   {'J', '3'}},  //                 
        { REGDOMAIN_JAPAN,   {'J', '4'}},  //                 
        { REGDOMAIN_JAPAN,   {'J', '5'}},  //                 
        { REGDOMAIN_WORLD,   {'J', 'E'}},  //      
        { REGDOMAIN_WORLD,   {'J', 'M'}},  //       
        { REGDOMAIN_WORLD,   {'J', 'O'}},  //      
        { REGDOMAIN_JAPAN,   {'J', 'P'}},  //     
        { REGDOMAIN_KOREA,   {'K', '1'}},  //                 
        { REGDOMAIN_KOREA,   {'K', '2'}},  //                 
        { REGDOMAIN_KOREA,   {'K', '3'}},  //                 
        { REGDOMAIN_KOREA,   {'K', '4'}},  //                 
        { REGDOMAIN_HI_5GHZ, {'K', 'E'}},  //     
        { REGDOMAIN_NO_5GHZ, {'K', 'G'}},  //          
        { REGDOMAIN_ETSI,    {'K', 'H'}},  //        
        { REGDOMAIN_WORLD,   {'K', 'I'}},  //        
        { REGDOMAIN_NO_5GHZ, {'K', 'M'}},  //       
        { REGDOMAIN_WORLD,   {'K', 'N'}},  //                     
        { REGDOMAIN_WORLD,   {'K', 'P'}},  //                                      
        { REGDOMAIN_KOREA,   {'K', 'R'}},  //                  
        { REGDOMAIN_N_AMER_EXC_FCC, {'K', 'W'}},  //      
        { REGDOMAIN_FCC,     {'K', 'Y'}},  //              
        { REGDOMAIN_NO_5GHZ, {'K', 'Z'}},  //          
        { REGDOMAIN_WORLD,   {'L', 'A'}},  //                                
        { REGDOMAIN_HI_5GHZ, {'L', 'B'}},  //       
        { REGDOMAIN_WORLD,   {'L', 'C'}},  //           
        { REGDOMAIN_ETSI,    {'L', 'I'}},  //             
        { REGDOMAIN_WORLD,   {'L', 'K'}},  //         
        { REGDOMAIN_WORLD,   {'L', 'R'}},  //       
        { REGDOMAIN_ETSI,    {'L', 'S'}},  //       
        { REGDOMAIN_ETSI,    {'L', 'T'}},  //         
        { REGDOMAIN_ETSI,    {'L', 'U'}},  //          
        { REGDOMAIN_ETSI,    {'L', 'V'}},  //      
        { REGDOMAIN_NO_5GHZ, {'L', 'Y'}},  //                      
        { REGDOMAIN_NO_5GHZ, {'M', 'A'}},  //       
        { REGDOMAIN_N_AMER_EXC_FCC, {'M', 'C'}},  //      
        { REGDOMAIN_ETSI,    {'M', 'D'}},  //                    
        { REGDOMAIN_ETSI,    {'M', 'E'}},  //          
        { REGDOMAIN_NO_5GHZ, {'M', 'G'}},  //          
        { REGDOMAIN_WORLD,   {'M', 'H'}},  //                
        { REGDOMAIN_ETSI,    {'M', 'K'}},  //                                          
        { REGDOMAIN_NO_5GHZ, {'M', 'L'}},  //    
        { REGDOMAIN_WORLD,   {'M', 'M'}},  //       
        { REGDOMAIN_NO_5GHZ, {'M', 'N'}},  //        
        { REGDOMAIN_APAC,    {'M', 'O'}},  //     
        { REGDOMAIN_FCC,     {'M', 'P'}},  //                        
        { REGDOMAIN_ETSI,    {'M', 'Q'}},  //          
        { REGDOMAIN_ETSI,    {'M', 'R'}},  //          
        { REGDOMAIN_ETSI,    {'M', 'S'}},  //          
        { REGDOMAIN_ETSI,    {'M', 'T'}},  //     
        { REGDOMAIN_ETSI,    {'M', 'U'}},  //         
        { REGDOMAIN_APAC,    {'M', 'V'}},  //        
        { REGDOMAIN_HI_5GHZ, {'M', 'W'}},  //      
        { REGDOMAIN_APAC,    {'M', 'X'}},  //      
        { REGDOMAIN_APAC,    {'M', 'Y'}},  //        
        { REGDOMAIN_WORLD,   {'M', 'Z'}},  //          
        { REGDOMAIN_WORLD,   {'N', 'A'}},  //       
        { REGDOMAIN_NO_5GHZ, {'N', 'C'}},  //             
        { REGDOMAIN_WORLD,   {'N', 'E'}},  //     
        { REGDOMAIN_WORLD,   {'N', 'F'}},  //              
        { REGDOMAIN_WORLD,   {'N', 'G'}},  //       
        { REGDOMAIN_WORLD,   {'N', 'I'}},  //         
        { REGDOMAIN_ETSI,    {'N', 'L'}},  //           
        { REGDOMAIN_ETSI,    {'N', 'O'}},  //      
        { REGDOMAIN_HI_5GHZ, {'N', 'P'}},  //     
        { REGDOMAIN_NO_5GHZ, {'N', 'R'}},  //     
        { REGDOMAIN_WORLD,   {'N', 'U'}},  //    
        { REGDOMAIN_APAC,    {'N', 'Z'}},  //           
        { REGDOMAIN_WORLD,   {'O', 'M'}},  //    
        { REGDOMAIN_APAC,    {'P', 'A'}},  //      
        { REGDOMAIN_HI_5GHZ, {'P', 'E'}},  //    
        { REGDOMAIN_ETSI,    {'P', 'F'}},  //                
        { REGDOMAIN_APAC,    {'P', 'G'}},  //                
        { REGDOMAIN_HI_5GHZ, {'P', 'H'}},  //           
        { REGDOMAIN_HI_5GHZ, {'P', 'K'}},  //        
        { REGDOMAIN_ETSI,    {'P', 'L'}},  //      
        { REGDOMAIN_WORLD,   {'P', 'M'}},  //                         
        { REGDOMAIN_WORLD,   {'P', 'N'}},  //             
        { REGDOMAIN_FCC,     {'P', 'R'}},  //           
        { REGDOMAIN_WORLD,   {'P', 'S'}},  //                               
        { REGDOMAIN_ETSI,    {'P', 'T'}},  //        
        { REGDOMAIN_WORLD,   {'P', 'W'}},  //     
        { REGDOMAIN_WORLD,   {'P', 'Y'}},  //        
        { REGDOMAIN_HI_5GHZ, {'Q', 'A'}},  //     
        { REGDOMAIN_ETSI,    {'R', 'E'}},  //       
        { REGDOMAIN_ETSI,    {'R', 'O'}},  //       
        { REGDOMAIN_ETSI,    {'R', 'S'}},  //      
        { REGDOMAIN_HI_5GHZ, {'R', 'U'}},  //      
        { REGDOMAIN_HI_5GHZ, {'R', 'W'}},  //      
        { REGDOMAIN_APAC,    {'S', 'A'}},  //            
        { REGDOMAIN_NO_5GHZ, {'S', 'B'}},  //               
        { REGDOMAIN_NO_5GHZ, {'S', 'C'}},  //          
        { REGDOMAIN_WORLD,   {'S', 'D'}},  //     
        { REGDOMAIN_ETSI,    {'S', 'E'}},  //      
        { REGDOMAIN_APAC,    {'S', 'G'}},  //         
        { REGDOMAIN_WORLD,   {'S', 'H'}},  //            
        { REGDOMAIN_ETSI,    {'S', 'I'}},  //         
        { REGDOMAIN_WORLD,   {'S', 'J'}},  //                      
        { REGDOMAIN_ETSI,    {'S', 'K'}},  //        
        { REGDOMAIN_WORLD,   {'S', 'L'}},  //            
        { REGDOMAIN_ETSI,    {'S', 'M'}},  //          
        { REGDOMAIN_ETSI,    {'S', 'N'}},  //       
        { REGDOMAIN_NO_5GHZ, {'S', 'O'}},  //       
        { REGDOMAIN_NO_5GHZ, {'S', 'R'}},  //        
        { REGDOMAIN_WORLD,   {'S', 'T'}},  //                     
        { REGDOMAIN_APAC,    {'S', 'V'}},  //           
        { REGDOMAIN_NO_5GHZ, {'S', 'Y'}},  //                    
        { REGDOMAIN_NO_5GHZ, {'S', 'Z'}},  //         
        { REGDOMAIN_ETSI,    {'T', 'C'}},  //                        
        { REGDOMAIN_NO_5GHZ, {'T', 'D'}},  //    
        { REGDOMAIN_ETSI,    {'T', 'F'}},  //                           
        { REGDOMAIN_NO_5GHZ, {'T', 'G'}},  //    
        { REGDOMAIN_WORLD,   {'T', 'H'}},  //        
        { REGDOMAIN_NO_5GHZ, {'T', 'J'}},  //          
        { REGDOMAIN_WORLD,   {'T', 'K'}},  //       
        { REGDOMAIN_WORLD,   {'T', 'L'}},  //           
        { REGDOMAIN_NO_5GHZ, {'T', 'M'}},  //            
        { REGDOMAIN_N_AMER_EXC_FCC, {'T', 'N'}},  //       
        { REGDOMAIN_NO_5GHZ, {'T', 'O'}},  //     
        { REGDOMAIN_N_AMER_EXC_FCC, {'T', 'R'}},  //      
        { REGDOMAIN_WORLD,   {'T', 'T'}},  //                   
        { REGDOMAIN_NO_5GHZ, {'T', 'V'}},  //      
        { REGDOMAIN_WORLD,   {'T', 'W'}},  //                         
        { REGDOMAIN_HI_5GHZ, {'T', 'Z'}},  //                            
        { REGDOMAIN_NO_5GHZ, {'U', 'A'}},  //       
        { REGDOMAIN_WORLD,   {'U', 'G'}},  //      
        { REGDOMAIN_FCC,     {'U', 'M'}},  //                                    
        { REGDOMAIN_WORLD,   {'U', 'Y'}},  //       
        { REGDOMAIN_WORLD,   {'U', 'Z'}},  //          
        { REGDOMAIN_ETSI,    {'V', 'A'}},  //                             
        { REGDOMAIN_WORLD,   {'V', 'C'}},  //                                
        { REGDOMAIN_HI_5GHZ, {'V', 'E'}},  //         
        { REGDOMAIN_ETSI,    {'V', 'G'}},  //                       
        { REGDOMAIN_FCC,     {'V', 'I'}},  //                  
        { REGDOMAIN_N_AMER_EXC_FCC, {'V', 'N'}},  //        
        { REGDOMAIN_NO_5GHZ, {'V', 'U'}},  //       
        { REGDOMAIN_WORLD,   {'W', 'F'}},  //                 
        { REGDOMAIN_N_AMER_EXC_FCC, {'W', 'S'}},  //     
        { REGDOMAIN_NO_5GHZ, {'Y', 'E'}},  //     
        { REGDOMAIN_ETSI,    {'Y', 'T'}},  //       
        { REGDOMAIN_WORLD,   {'Z', 'A'}},  //            
        { REGDOMAIN_APAC,    {'Z', 'M'}},  //      
        { REGDOMAIN_NO_5GHZ, {'Z', 'W'}},  //        
    }
};
typedef struct nvEFSTable_s
{
   v_U32_t    nvValidityBitmap;
   sHalNv     halnv;
} nvEFSTable_t;
nvEFSTable_t *gnvEFSTable=NULL;
/*                                           */ 
static nvEFSTable_t *pnvEFSTable =NULL;

const tRfChannelProps rfChannels[NUM_RF_CHANNELS] =
{
    //                  
    //                 
    { 2412, 1  , RF_SUBBAND_2_4_GHZ},        //          
    { 2417, 2  , RF_SUBBAND_2_4_GHZ},        //          
    { 2422, 3  , RF_SUBBAND_2_4_GHZ},        //          
    { 2427, 4  , RF_SUBBAND_2_4_GHZ},        //          
    { 2432, 5  , RF_SUBBAND_2_4_GHZ},        //          
    { 2437, 6  , RF_SUBBAND_2_4_GHZ},        //          
    { 2442, 7  , RF_SUBBAND_2_4_GHZ},        //          
    { 2447, 8  , RF_SUBBAND_2_4_GHZ},        //          
    { 2452, 9  , RF_SUBBAND_2_4_GHZ},        //          
    { 2457, 10 , RF_SUBBAND_2_4_GHZ},        //           
    { 2462, 11 , RF_SUBBAND_2_4_GHZ},        //           
    { 2467, 12 , RF_SUBBAND_2_4_GHZ},        //           
    { 2472, 13 , RF_SUBBAND_2_4_GHZ},        //           
    { 2484, 14 , RF_SUBBAND_2_4_GHZ},        //           
#ifdef FEATURE_WLAN_INTEGRATED_SOC
    { 0,    240, RF_SUBBAND_4_9_GHZ},        //            
    { 0,    244, RF_SUBBAND_4_9_GHZ},        //            
    { 0,    248, RF_SUBBAND_4_9_GHZ},        //            
    { 0,    252, RF_SUBBAND_4_9_GHZ},        //            
    { 0,    208, RF_SUBBAND_4_9_GHZ},        //            
    { 0,    212, RF_SUBBAND_4_9_GHZ},        //            
    { 0,    216, RF_SUBBAND_4_9_GHZ},        //            
    { 5180, 36 , RF_SUBBAND_5_LOW_GHZ},      //           
    { 5200, 40 , RF_SUBBAND_5_LOW_GHZ},      //           
    { 5220, 44 , RF_SUBBAND_5_LOW_GHZ},      //           
    { 5240, 48 , RF_SUBBAND_5_LOW_GHZ},      //           
    { 5260, 52 , RF_SUBBAND_5_LOW_GHZ},      //           
    { 5280, 56 , RF_SUBBAND_5_LOW_GHZ},      //           
    { 5300, 60 , RF_SUBBAND_5_LOW_GHZ},      //           
    { 5320, 64 , RF_SUBBAND_5_LOW_GHZ},      //           
    { 5500, 100, RF_SUBBAND_5_MID_GHZ},      //            
    { 5520, 104, RF_SUBBAND_5_MID_GHZ},      //            
    { 5540, 108, RF_SUBBAND_5_MID_GHZ},      //            
    { 5560, 112, RF_SUBBAND_5_MID_GHZ},      //            
    { 5580, 116, RF_SUBBAND_5_MID_GHZ},      //            
    { 5600, 120, RF_SUBBAND_5_MID_GHZ},      //            
    { 5620, 124, RF_SUBBAND_5_MID_GHZ},      //            
    { 5640, 128, RF_SUBBAND_5_MID_GHZ},      //            
    { 5660, 132, RF_SUBBAND_5_MID_GHZ},      //            
    { 5680, 136, RF_SUBBAND_5_MID_GHZ},      //            
    { 5700, 140, RF_SUBBAND_5_MID_GHZ},      //            
    { 5745, 149, RF_SUBBAND_5_HIGH_GHZ},     //            
    { 5765, 153, RF_SUBBAND_5_HIGH_GHZ},     //            
    { 5785, 157, RF_SUBBAND_5_HIGH_GHZ},     //            
    { 5805, 161, RF_SUBBAND_5_HIGH_GHZ},     //            
    { 5825, 165, RF_SUBBAND_5_HIGH_GHZ},     //            
    { 0   , 0  , NUM_RF_SUBBANDS},           //               
    { 0   , 0  , NUM_RF_SUBBANDS},           //               
    { 0   , 0  , NUM_RF_SUBBANDS},           //               
    { 0   , 0  , NUM_RF_SUBBANDS},           //               
    { 0   , 0  , NUM_RF_SUBBANDS},           //               
    { 0   , 0  , NUM_RF_SUBBANDS},           //               
    { 0   , 0  , NUM_RF_SUBBANDS},           //               
    { 0   , 0  , NUM_RF_SUBBANDS},           //                
    { 0   , 0  , NUM_RF_SUBBANDS},           //                
    { 0   , 0  , NUM_RF_SUBBANDS},           //                 
    { 0   , 0  , NUM_RF_SUBBANDS},           //                 
    { 0   , 0  , NUM_RF_SUBBANDS},           //                 
    { 0   , 0  , NUM_RF_SUBBANDS},           //                 
    { 0   , 0  , NUM_RF_SUBBANDS},           //                 
    { 0   , 0  , NUM_RF_SUBBANDS},           //                
    { 0   , 0  , NUM_RF_SUBBANDS},           //                
    { 0   , 0  , NUM_RF_SUBBANDS},           //                
    { 0   , 0  , NUM_RF_SUBBANDS},           //                
    { 0   , 0  , NUM_RF_SUBBANDS},           //                
    { 0   , 0  , NUM_RF_SUBBANDS},           //                
    { 0   , 0  , NUM_RF_SUBBANDS},           //                
    { 0   , 0  , NUM_RF_SUBBANDS},           //                 
    { 0   , 0  , NUM_RF_SUBBANDS},           //                 
    { 0   , 0  , NUM_RF_SUBBANDS},           //                 
    { 0   , 0  , NUM_RF_SUBBANDS},           //                 
    { 0   , 0  , NUM_RF_SUBBANDS},           //                 
    { 0   , 0  , NUM_RF_SUBBANDS},           //                 
    { 0   , 0  , NUM_RF_SUBBANDS},           //                 
    { 0   , 0  , NUM_RF_SUBBANDS},           //                 
    { 0   , 0  , NUM_RF_SUBBANDS},           //                 
    { 0   , 0  , NUM_RF_SUBBANDS},           //                 
    { 0   , 0  , NUM_RF_SUBBANDS},           //                 
    { 0   , 0  , NUM_RF_SUBBANDS},           //                 
    { 0   , 0  , NUM_RF_SUBBANDS},           //                 
    { 0   , 0  , NUM_RF_SUBBANDS},           //                 
#endif
};

extern const sHalNv nvDefaults;

const sRegulatoryChannel * regChannels = nvDefaults.tables.regDomains[0].channels;

/*                                                                            
                                         
                                                                            */
VOS_STATUS wlan_write_to_efs (v_U8_t *pData, v_U16_t data_len);
/*                                                                         
                                                 
                                                                       
                                                          
                                                                 
                                                
     
                                                                           */
VOS_STATUS vos_nv_init(void)
{
   return VOS_STATUS_SUCCESS;
}

VOS_STATUS vos_nv_open(void)
{
    VOS_STATUS status = VOS_STATUS_SUCCESS;
    v_CONTEXT_t pVosContext= NULL;
    v_SIZE_t bufSize;
    v_SIZE_t nvReadBufSize;
    v_BOOL_t itemIsValid = VOS_FALSE;
    
    /*                       */
    pVosContext = vos_get_global_context(VOS_MODULE_ID_SYS, NULL);
    bufSize = sizeof(nvEFSTable_t);
    status = hdd_request_firmware(WLAN_NV_FILE,
                                  ((VosContextType*)(pVosContext))->pHDDContext,
                                  (v_VOID_t**)&gnvEFSTable, &nvReadBufSize);

    if ( (!VOS_IS_STATUS_SUCCESS( status )) || !gnvEFSTable)
    {
         VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_FATAL,
                   "%s: unable to download NV file %s",
                   __FUNCTION__, WLAN_NV_FILE);
         return VOS_STATUS_E_RESOURCES;
    }

     /*                                                    */
    {
        /*                                    */
        pnvEFSTable = (nvEFSTable_t *)vos_mem_malloc(sizeof(nvEFSTable_t));
        if (NULL == pnvEFSTable)
        {
            VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR,
                      "%s : failed to allocate memory for NV", __FUNCTION__);
            return VOS_STATUS_E_NOMEM;
        }

        /*                        */
        vos_mem_copy(&(pnvEFSTable->halnv),&nvDefaults,sizeof(sHalNv));
       
        if ( nvReadBufSize != bufSize)
        {
            pnvEFSTable->nvValidityBitmap = DEFAULT_NV_VALIDITY_BITMAP;
            VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_FATAL,
                      "!!!WARNING: INVALID NV FILE, DRIVER IS USING DEFAULT CAL VALUES %d %d!!!",
                      nvReadBufSize, bufSize);
            return (eHAL_STATUS_SUCCESS);
        }

       pnvEFSTable->nvValidityBitmap = gnvEFSTable->nvValidityBitmap;
        /*                                                  */ 
        if (vos_nv_getValidity(VNV_FIELD_IMAGE, &itemIsValid) == 
           VOS_STATUS_SUCCESS)
        {
            if (itemIsValid == VOS_TRUE) {

                if(vos_nv_read( VNV_FIELD_IMAGE, (v_VOID_t *)&pnvEFSTable->halnv.fields,
                   NULL, sizeof(sNvFields) ) != VOS_STATUS_SUCCESS)
                   return (eHAL_STATUS_FAILURE);
            }
        }

        if (vos_nv_getValidity(VNV_RATE_TO_POWER_TABLE, &itemIsValid) == 
             VOS_STATUS_SUCCESS)
        {
            if (itemIsValid == VOS_TRUE)
            {
                if(vos_nv_read( VNV_RATE_TO_POWER_TABLE, 
                  (v_VOID_t *)&pnvEFSTable->halnv.tables.pwrOptimum[0],
                  NULL, sizeof(tRateGroupPwr) * NUM_RF_SUBBANDS ) != VOS_STATUS_SUCCESS)
               return (eHAL_STATUS_FAILURE);
            }
        }

        if (vos_nv_getValidity(VNV_REGULARTORY_DOMAIN_TABLE, &itemIsValid) == 
               VOS_STATUS_SUCCESS)
        {
    
            if (itemIsValid == VOS_TRUE)
            {
                if(vos_nv_read( VNV_REGULARTORY_DOMAIN_TABLE,
                (v_VOID_t *)&pnvEFSTable->halnv.tables.regDomains[0],
                 NULL, sizeof(sRegulatoryDomains) * NUM_REG_DOMAINS ) != VOS_STATUS_SUCCESS)
                    return (eHAL_STATUS_FAILURE);
            }
        }

        if (vos_nv_getValidity(VNV_DEFAULT_LOCATION, &itemIsValid) == 
            VOS_STATUS_SUCCESS)
        {
            if (itemIsValid == VOS_TRUE)
            {
                if(vos_nv_read( VNV_DEFAULT_LOCATION,
                (v_VOID_t *)&pnvEFSTable->halnv.tables.defaultCountryTable,
                NULL, sizeof(sDefaultCountry) ) !=  VOS_STATUS_SUCCESS)
                     return (eHAL_STATUS_FAILURE);
            }
        }
    
        if (vos_nv_getValidity(VNV_TPC_POWER_TABLE, &itemIsValid) == 
            VOS_STATUS_SUCCESS)
        {
            if (itemIsValid == VOS_TRUE)
            {
                if(vos_nv_read( VNV_TPC_POWER_TABLE, 
                  (v_VOID_t *)&pnvEFSTable->halnv.tables.plutCharacterized[0],
                  NULL, sizeof(tTpcPowerTable) * NUM_RF_CHANNELS ) != VOS_STATUS_SUCCESS)
                     return (eHAL_STATUS_FAILURE);
            }
        }
    
        if (vos_nv_getValidity(VNV_TPC_PDADC_OFFSETS, &itemIsValid) == 
            VOS_STATUS_SUCCESS)
        {
            if (itemIsValid == VOS_TRUE)
            {
                if(vos_nv_read( VNV_TPC_PDADC_OFFSETS,
                  (v_VOID_t *)&pnvEFSTable->halnv.tables.plutPdadcOffset[0],
                  NULL, sizeof(tANI_U16) * NUM_RF_CHANNELS ) != VOS_STATUS_SUCCESS)
                     return (eHAL_STATUS_FAILURE);
            }
        }
        if (vos_nv_getValidity(VNV_RSSI_CHANNEL_OFFSETS, &itemIsValid) == 
           VOS_STATUS_SUCCESS)
        {
            if (itemIsValid == VOS_TRUE)
            {
                if(vos_nv_read( VNV_RSSI_CHANNEL_OFFSETS,
                  (v_VOID_t *)&pnvEFSTable->halnv.tables.rssiChanOffsets[0],
                  NULL, sizeof(sRssiChannelOffsets) * 2 ) != VOS_STATUS_SUCCESS)
                     return (eHAL_STATUS_FAILURE);
            }
        }
    
        if (vos_nv_getValidity(VNV_RF_CAL_VALUES, &itemIsValid) == 
         VOS_STATUS_SUCCESS)
        {
            if (itemIsValid == VOS_TRUE)
            {
                if(vos_nv_read( VNV_RF_CAL_VALUES, (v_VOID_t *)&pnvEFSTable->halnv
    .tables.rFCalValues, NULL, sizeof(sRFCalValues) ) != VOS_STATUS_SUCCESS)
                     return (eHAL_STATUS_FAILURE);
            }
        }

        if (vos_nv_getValidity(VNV_ANTENNA_PATH_LOSS, &itemIsValid) == 
         VOS_STATUS_SUCCESS)
        {
            if (itemIsValid == VOS_TRUE)
            {
                if(vos_nv_read( VNV_ANTENNA_PATH_LOSS,
                  (v_VOID_t *)&pnvEFSTable->halnv.tables.antennaPathLoss[0], NULL, 
                sizeof(tANI_S16)*NUM_RF_CHANNELS ) != VOS_STATUS_SUCCESS)
                     return (eHAL_STATUS_FAILURE);
            }
        }
        if (vos_nv_getValidity(VNV_PACKET_TYPE_POWER_LIMITS, &itemIsValid) == 
         VOS_STATUS_SUCCESS)
        {
            if (itemIsValid == VOS_TRUE)
            {
                if(vos_nv_read( VNV_PACKET_TYPE_POWER_LIMITS, 
                  (v_VOID_t *)&pnvEFSTable->halnv.tables.pktTypePwrLimits[0], NULL, 
                sizeof(tANI_S16)*NUM_802_11_MODES*NUM_RF_CHANNELS ) != VOS_STATUS_SUCCESS)
                     return (eHAL_STATUS_FAILURE);
            }
        }

        if (vos_nv_getValidity(VNV_OFDM_CMD_PWR_OFFSET, &itemIsValid) == 
         VOS_STATUS_SUCCESS)
        {
            if (itemIsValid == VOS_TRUE)
            {
                if(vos_nv_read( VNV_OFDM_CMD_PWR_OFFSET, 
                  (v_VOID_t *)&pnvEFSTable->halnv.tables.ofdmCmdPwrOffset, NULL, 
                                sizeof(sOfdmCmdPwrOffset)) != VOS_STATUS_SUCCESS)
                     return (eHAL_STATUS_FAILURE);
            }
        }

        if (vos_nv_getValidity(VNV_TX_BB_FILTER_MODE, &itemIsValid) == 
         VOS_STATUS_SUCCESS)
        {
            if (itemIsValid == VOS_TRUE)
            {
               if(vos_nv_read(VNV_TX_BB_FILTER_MODE, 
                  (v_VOID_t *)&pnvEFSTable->halnv.tables.txbbFilterMode, NULL, 
                sizeof(sTxBbFilterMode)) != VOS_STATUS_SUCCESS)
                     return (eHAL_STATUS_FAILURE);
            }
        }
        if (vos_nv_getValidity(VNV_TABLE_VIRTUAL_RATE, &itemIsValid) == 
         VOS_STATUS_SUCCESS)
        {
            if (itemIsValid == VOS_TRUE)
            {
               if(vos_nv_read(VNV_TABLE_VIRTUAL_RATE, 
                  (v_VOID_t *)&pnvEFSTable->halnv.tables.pwrOptimum_virtualRate, NULL, 
                sizeof(gnvEFSTable->halnv.tables.pwrOptimum_virtualRate)) != VOS_STATUS_SUCCESS)
                     return (eHAL_STATUS_FAILURE);
            }
        }
    }

    return VOS_STATUS_SUCCESS;
}

VOS_STATUS vos_nv_close(void)
{
    VOS_STATUS status = VOS_STATUS_SUCCESS;
    v_CONTEXT_t pVosContext= NULL;
         /*                       */
    pVosContext = vos_get_global_context(VOS_MODULE_ID_SYS, NULL);
    status = hdd_release_firmware(WLAN_NV_FILE, ((VosContextType*)(pVosContext))->pHDDContext);
    if ( !VOS_IS_STATUS_SUCCESS( status ))
    {
        VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR,
                         "%s : vos_open failed\n",__func__);
        return VOS_STATUS_E_FAILURE;
    }
    vos_mem_free(pnvEFSTable);
    gnvEFSTable=NULL;
    return VOS_STATUS_SUCCESS;
}
/*                                                                         
                                                                            
                                  
                                                                              
                                                                       
                          
                                                   
                                   
                                                                               
                                                    
                                                          
                                                                          
     
                                                                           */
VOS_STATUS vos_nv_getRegDomainFromCountryCode( v_REGDOMAIN_t *pRegDomain,
      const v_COUNTRYCODE_t countryCode )
{
   int i;
   //              
   if (NULL == pRegDomain)
   {
      VOS_TRACE( VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR,
            ("Invalid reg domain pointer\r\n") );
      return VOS_STATUS_E_FAULT;
   }
   *pRegDomain = REGDOMAIN_COUNT;

   if (NULL == countryCode)
   {
      VOS_TRACE( VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR,
            ("Country code array is NULL\r\n") );
      return VOS_STATUS_E_FAULT;
   }
   if (0 == countryInfoTable.countryCount)
   {
      VOS_TRACE( VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR,
            ("Reg domain table is empty\r\n") );
      return VOS_STATUS_E_EMPTY;
   }
   //                                                                      
   //         
   for (i = 0; i < countryInfoTable.countryCount &&
         REGDOMAIN_COUNT == *pRegDomain; i++)
   {
      if (memcmp(countryCode, countryInfoTable.countryInfo[i].countryCode,
               VOS_COUNTRY_CODE_LEN) == 0)
      {
         //                      
         *pRegDomain = countryInfoTable.countryInfo[i].regDomain;
      }
   }
   if (REGDOMAIN_COUNT != *pRegDomain)
   {
      return VOS_STATUS_SUCCESS;
   }
   else
   {
      VOS_TRACE( VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_WARN,
            ("country code is not found\r\n"));
      return VOS_STATUS_E_EXISTS;
   }
}
/*                                                                         
                                                                     
               
                                                                       
                                                    
                                                                      
                                                                        
                                                                          
                                                
                                                                 
                                                                             
                                                                     
                                                                           
                                                                   
                                         
     
                                                                           */
VOS_STATUS vos_nv_getSupportedCountryCode( v_BYTE_t *pBuffer, v_SIZE_t *pBufferSize,
      v_SIZE_t paddingSize )
{
   v_SIZE_t providedBufferSize = *pBufferSize;
   int i;
   //                                                   
   *pBufferSize = countryInfoTable.countryCount * (VOS_COUNTRY_CODE_LEN + paddingSize );
   if ( NULL == pBuffer || providedBufferSize < *pBufferSize )
   {
      VOS_TRACE( VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_INFO,
            ("Insufficient memory for country code list\r\n"));
      return VOS_STATUS_E_NOMEM;
   }
   for (i = 0; i < countryInfoTable.countryCount; i++)
   {
      memcpy( pBuffer, countryInfoTable.countryInfo[i].countryCode, VOS_COUNTRY_CODE_LEN );
      pBuffer += (VOS_COUNTRY_CODE_LEN + paddingSize );
   }
   return VOS_STATUS_SUCCESS;
}
/*                                                                         
                                                                  
                                          
                                         
     
                                                                           */
VOS_STATUS vos_nv_readTxAntennaCount( v_U8_t *pTxAntennaCount )
{
   sNvFields fieldImage;
   VOS_STATUS status;
   status = vos_nv_read( VNV_FIELD_IMAGE, &fieldImage, NULL,
         sizeof(fieldImage) );
   if (VOS_STATUS_SUCCESS == status)
   {
      *pTxAntennaCount = fieldImage.numOfTxChains;
   }
   return status;
}
/*                                                                         
                                                                  
                                          
                                         
     
                                                                           */
VOS_STATUS vos_nv_readRxAntennaCount( v_U8_t *pRxAntennaCount )
{
   sNvFields fieldImage;
   VOS_STATUS status;
   status = vos_nv_read( VNV_FIELD_IMAGE, &fieldImage, NULL,
         sizeof(fieldImage) );
   if (VOS_STATUS_SUCCESS == status)
   {
      *pRxAntennaCount = fieldImage.numOfRxChains;
   }
   return status;
}

/*                                                                         
                                                         
                                  
                                         
     
                                                                           */
VOS_STATUS vos_nv_readMacAddress( v_MAC_ADDRESS_t pMacAddress )
{
   sNvFields fieldImage;
   VOS_STATUS status;
   status = vos_nv_read( VNV_FIELD_IMAGE, &fieldImage, NULL,
         sizeof(fieldImage) );
   if (VOS_STATUS_SUCCESS == status)
   {
      memcpy( pMacAddress, fieldImage.macAddr, VOS_MAC_ADDRESS_LEN );
   }
   else
   {
      //                                                          
      const v_U8_t macAddr[VOS_MAC_ADDRESS_LEN] = VOS_HARD_CODED_MAC;
      memcpy( pMacAddress, macAddr, VOS_MAC_ADDRESS_LEN );
      VOS_TRACE( VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_WARN,
          " fail to get MAC address from NV, hardcoded to %02X-%02X-%02X-%02X-%02X%02X",
          macAddr[0], macAddr[1], macAddr[2], macAddr[3], macAddr[4], macAddr[5]);
      status = VOS_STATUS_SUCCESS;
   }
   return status;
}

/*                                                                         

                                                                         

                                  
                                                                     

                                         

     

                                                                           */
VOS_STATUS vos_nv_readMultiMacAddress( v_U8_t *pMacAddress,
                                              v_U8_t  macCount )
{
   sNvFields   fieldImage;
   VOS_STATUS  status;
   v_U8_t      countLoop;
   v_U8_t     *pNVMacAddress;

   if((0 == macCount) || (VOS_MAX_CONCURRENCY_PERSONA < macCount) ||
      (NULL == pMacAddress))
   {
      VOS_TRACE( VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR,
          " Invalid Parameter from NV Client macCount %d, pMacAddress 0x%x",
          macCount, pMacAddress);
   }

   status = vos_nv_read( VNV_FIELD_IMAGE, &fieldImage, NULL,
                         sizeof(fieldImage) );
   if (VOS_STATUS_SUCCESS == status)
   {
      pNVMacAddress = fieldImage.macAddr;
      for(countLoop = 0; countLoop < macCount; countLoop++)
      {
         vos_mem_copy(pMacAddress + (countLoop * VOS_MAC_ADDRESS_LEN),
                      pNVMacAddress + (countLoop * VOS_MAC_ADDRESS_LEN),
                      VOS_MAC_ADDRESS_LEN);
      }
   }
   else
   {
      VOS_TRACE( VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR,
                 "vos_nv_readMultiMacAddress Get NV Field Fail");
   }

   return status;
}

/*                                                                         
                                                               
                                                                        
                                              
                                                                           
                                                                
                                   
                                                                   
                                                           
                                                               
                                              
     
                                                                           */
#ifndef WLAN_FTM_STUB

VOS_STATUS vos_nv_setValidity( VNV_TYPE type, v_BOOL_t itemIsValid )
{
   v_U32_t lastNvValidityBitmap;
   v_U32_t newNvValidityBitmap;
   VOS_STATUS status = VOS_STATUS_SUCCESS;

   //                                      
   if (VNV_TYPE_COUNT <= type)
   {
      VOS_TRACE( VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR,
                 ("%s: invalid type=%d"), __FUNCTION__, type );
      return VOS_STATUS_E_INVAL;
   }
   //                         
   lastNvValidityBitmap = gnvEFSTable->nvValidityBitmap;
   //                           
   if (itemIsValid)
   {
       newNvValidityBitmap = lastNvValidityBitmap | (1 << type);
              //                                               
       if (newNvValidityBitmap != lastNvValidityBitmap)
       {
           gnvEFSTable->nvValidityBitmap = newNvValidityBitmap;
       }
   }
   else
   {
       newNvValidityBitmap = lastNvValidityBitmap & (~(1 << type));
       if (newNvValidityBitmap != lastNvValidityBitmap)
       {
           gnvEFSTable->nvValidityBitmap = newNvValidityBitmap;
           status = wlan_write_to_efs((v_U8_t*)gnvEFSTable,sizeof(nvEFSTable_t));
           if (! VOS_IS_STATUS_SUCCESS(status)) {
               VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR, ("vos_nv_write_to_efs failed!!!\r\n"));
               status = VOS_STATUS_E_FAULT;
           }
       }
   }

   return status;
}
#endif
/*                                                                         
                                                               
                                                                    
                                              
                                                                           
                                                                
                                   
                                                                         
                               
                                                                  
                                                               
                                              
     
                                                                           */
VOS_STATUS vos_nv_getValidity( VNV_TYPE type, v_BOOL_t *pItemIsValid )
{
   v_U32_t nvValidityBitmap = gnvEFSTable->nvValidityBitmap;
   //                                      
   if (VNV_TYPE_COUNT <= type)
   {
      VOS_TRACE( VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR,
                 ("%s: invalid type=%d"), __FUNCTION__, type );
      return VOS_STATUS_E_INVAL;
   }
   *pItemIsValid = (v_BOOL_t)((nvValidityBitmap >> type) & 1);
   return VOS_STATUS_SUCCESS;
}
/*                                                                         
                                                           
                                                                           
                                                                      
                                                         
                                                                       
                                                          
                                                            
                             
                                       
                                        
                                         
                                                           
                                                               
                                                          
                                                      
                                              
     
                                                                           */
VOS_STATUS vos_nv_read( VNV_TYPE type, v_VOID_t *outputVoidBuffer,
      v_VOID_t *defaultBuffer, v_SIZE_t bufferSize )
{
    VOS_STATUS status             = VOS_STATUS_SUCCESS;
    v_SIZE_t itemSize;
    v_BOOL_t itemIsValid = VOS_TRUE;

    //             
    if (VNV_TYPE_COUNT <= type)
    {
       VOS_TRACE( VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR,
                 ("%s: invalid type=%d"), __FUNCTION__, type );
       return VOS_STATUS_E_INVAL;
    }
    if (NULL == outputVoidBuffer)
    {
       VOS_TRACE( VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR,
             ("Buffer provided is NULL\r\n") );
       return VOS_STATUS_E_FAULT;
    }
    if (0 == bufferSize)
    {
       VOS_TRACE( VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR,
             ("NV type=%d is invalid\r\n"), type );
       return VOS_STATUS_E_INVAL;
    }
    //                                    
    status = vos_nv_getValidity( type, &itemIsValid );
   if (!itemIsValid)
   {
       VOS_TRACE( VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_WARN,
            "NV type=%d does not have valid data\r\n", type );
       return VOS_STATUS_E_EMPTY;
   }
   switch(type)
   {
       case VNV_FIELD_IMAGE:
           itemSize = sizeof(gnvEFSTable->halnv.fields);
           if(bufferSize != itemSize) {
               VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR,
                ("type = %d buffer size=%d is less than data size=%d\r\n"),type, bufferSize,
                 itemSize);
               status = VOS_STATUS_E_INVAL;
           }
           else {
               memcpy(outputVoidBuffer,&gnvEFSTable->halnv.fields,bufferSize);
           }
           break;
       case VNV_RATE_TO_POWER_TABLE:
           itemSize = sizeof(gnvEFSTable->halnv.tables.pwrOptimum);
           if(bufferSize != itemSize) {
               VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR,
                ("type = %d buffer size=%d is less than data size=%d\r\n"),type, bufferSize,
                 itemSize);
               status = VOS_STATUS_E_INVAL;
           }
           else {
               memcpy(outputVoidBuffer,&gnvEFSTable->halnv.tables.pwrOptimum[0],bufferSize);
           }
           break;
       case VNV_REGULARTORY_DOMAIN_TABLE:
           itemSize = sizeof(gnvEFSTable->halnv.tables.regDomains);
           if(bufferSize != itemSize) {
               VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR,
                ("type = %d buffer size=%d is less than data size=%d\r\n"),type, bufferSize,
                 itemSize);
               status = VOS_STATUS_E_INVAL;
           }
           else {
               memcpy(outputVoidBuffer,&gnvEFSTable->halnv.tables.regDomains[0],bufferSize);
           }
           break;
       case VNV_DEFAULT_LOCATION:
           itemSize = sizeof(gnvEFSTable->halnv.tables.defaultCountryTable);
           if(bufferSize != itemSize) {
               VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR,
                ("type = %d buffer size=%d is less than data size=%d\r\n"),type, bufferSize,
                 itemSize);
               status = VOS_STATUS_E_INVAL;
           }
           else {
               memcpy(outputVoidBuffer,&gnvEFSTable->halnv.tables.defaultCountryTable,bufferSize);
           }
           break;
       case VNV_TPC_POWER_TABLE:
           itemSize = sizeof(gnvEFSTable->halnv.tables.plutCharacterized);
           if(bufferSize != itemSize) {
               VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR,
                ("type = %d buffer size=%d is less than data size=%d\r\n"),type, bufferSize,
                 itemSize);
               status = VOS_STATUS_E_INVAL;
           }
           else {
               memcpy(outputVoidBuffer,&gnvEFSTable->halnv.tables.plutCharacterized[0],bufferSize);
           }
           break;
       case VNV_TPC_PDADC_OFFSETS:
           itemSize = sizeof(gnvEFSTable->halnv.tables.plutPdadcOffset);
           if(bufferSize != itemSize) {
               VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR,
                ("type = %d buffer size=%d is less than data size=%d\r\n"),type, bufferSize,
                 itemSize);
               status = VOS_STATUS_E_INVAL;
           }
           else {
               memcpy(outputVoidBuffer,&gnvEFSTable->halnv.tables.plutPdadcOffset[0],bufferSize);
           }
           break;
       case VNV_RSSI_CHANNEL_OFFSETS:

           itemSize = sizeof(gnvEFSTable->halnv.tables.rssiChanOffsets);

           if(bufferSize != itemSize) {

               VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR,
                ("type = %d buffer size=%d is less than data size=%d\r\n"),type, bufferSize,
                 itemSize);
               status = VOS_STATUS_E_INVAL;
           }
           else {
               memcpy(outputVoidBuffer,&gnvEFSTable->halnv.tables.rssiChanOffsets[0],bufferSize);
           }
           break;
       case VNV_RF_CAL_VALUES:

           itemSize = sizeof(gnvEFSTable->halnv.tables.rFCalValues);

           if(bufferSize != itemSize) {

               VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR,
                ("type = %d buffer size=%d is less than data size=%d\r\n"),type, bufferSize,
                 itemSize);
               status = VOS_STATUS_E_INVAL;
           }
           else {
               memcpy(outputVoidBuffer,&gnvEFSTable->halnv.tables.rFCalValues,bufferSize);
           }
           break;
       case VNV_ANTENNA_PATH_LOSS:
           itemSize = sizeof(gnvEFSTable->halnv.tables.antennaPathLoss);
           if(bufferSize != itemSize) {
               VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR,
                ("type = %d buffer size=%d is less than data size=%d\r\n"),type, bufferSize,
                 itemSize);
               status = VOS_STATUS_E_INVAL;
           }
           else {
               memcpy(outputVoidBuffer,&gnvEFSTable->halnv.tables.antennaPathLoss[0],bufferSize);
           }
           break;
       case VNV_PACKET_TYPE_POWER_LIMITS:
           itemSize = sizeof(gnvEFSTable->halnv.tables.pktTypePwrLimits);
           if(bufferSize != itemSize) {
               VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR,
                ("type = %d buffer size=%d is less than data size=%d\r\n"),type, bufferSize,
                 itemSize);
               status = VOS_STATUS_E_INVAL;
           }
           else {
               memcpy(outputVoidBuffer,&gnvEFSTable->halnv.tables.pktTypePwrLimits[0][0],bufferSize);
           }
           break;
       case VNV_OFDM_CMD_PWR_OFFSET:
           itemSize = sizeof(gnvEFSTable->halnv.tables.ofdmCmdPwrOffset);
           if(bufferSize != itemSize) {
               VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR,
                ("type = %d buffer size=%d is less than data size=%d\r\n"),type, bufferSize,
                 itemSize);
               status = VOS_STATUS_E_INVAL;
           }
           else {
               memcpy(outputVoidBuffer,&gnvEFSTable->halnv.tables.ofdmCmdPwrOffset,bufferSize);
           }
           break;
       case VNV_TX_BB_FILTER_MODE:
           itemSize = sizeof(gnvEFSTable->halnv.tables.txbbFilterMode);
           if(bufferSize != itemSize) {
               VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR,
                ("type = %d buffer size=%d is less than data size=%d\r\n"),type, bufferSize,
                 itemSize);
               status = VOS_STATUS_E_INVAL;
           }
           else {
               memcpy(outputVoidBuffer,&gnvEFSTable->halnv.tables.txbbFilterMode,bufferSize);
           }
           break;

#ifdef FEATURE_WLAN_NON_INTEGRATED_SOC
       case VNV_FREQUENCY_FOR_1_3V_SUPPLY:
           itemSize = sizeof(gnvEFSTable->halnv.tables.freqFor1p3VSupply);
           if(bufferSize != itemSize) {
               VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR,
                ("type = %d buffer size=%d is less than data size=%d\r\n"),type, bufferSize,
                 itemSize);
               status = VOS_STATUS_E_INVAL;
           }
           else {
               memcpy(outputVoidBuffer,&gnvEFSTable->halnv.tables.freqFor1p3VSupply,bufferSize);
           }
           break;
#endif /*                                 */

       case VNV_TABLE_VIRTUAL_RATE:
           itemSize = sizeof(gnvEFSTable->halnv.tables.pwrOptimum_virtualRate);
           if(bufferSize != itemSize) {
               VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR,
                ("type = %d buffer size=%d is less than data size=%d\r\n"),type, bufferSize,
                 itemSize);
               status = VOS_STATUS_E_INVAL;
           }
           else {
               memcpy(outputVoidBuffer,&gnvEFSTable->halnv.tables.pwrOptimum_virtualRate,bufferSize);
           }
           break;

       default:
         break;
   }
   return status;
}
#ifndef WLAN_FTM_STUB

/*                                                                         
                                                                 
                                                                            
                                                            
                            
                                   
                                            
                                                           
                                                               
                                                           
                                                      
                                                
     
                                                                           */
VOS_STATUS vos_nv_write( VNV_TYPE type, v_VOID_t *inputVoidBuffer,
      v_SIZE_t bufferSize )
{
    VOS_STATUS status = VOS_STATUS_SUCCESS;
    v_SIZE_t itemSize;

    //             
    if (VNV_TYPE_COUNT <= type)
    {
       VOS_TRACE( VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR,
                  ("%s: invalid type=%d"), __FUNCTION__, type );
       return VOS_STATUS_E_INVAL;
    }
    if (NULL == inputVoidBuffer)
    {
       VOS_TRACE( VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR,
             ("Buffer provided is NULL\r\n") );
       return VOS_STATUS_E_FAULT;
    }
    if (0 == bufferSize)
    {
       VOS_TRACE( VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR,
             ("NV type=%d is invalid\r\n"), type );
       return VOS_STATUS_E_INVAL;
    }
    switch(type)
    {
        case VNV_FIELD_IMAGE:
            itemSize = sizeof(gnvEFSTable->halnv.fields);
            if(bufferSize != itemSize) {
                VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR,
                 ("type = %d buffer size=%d is less than data size=%d\r\n"),type, bufferSize,
                  itemSize);
                status = VOS_STATUS_E_INVAL;
            }
            else {
                memcpy(&gnvEFSTable->halnv.fields,inputVoidBuffer,bufferSize);
            }
            break;
        case VNV_RATE_TO_POWER_TABLE:
            itemSize = sizeof(gnvEFSTable->halnv.tables.pwrOptimum);
            if(bufferSize != itemSize) {
                VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR,
                 ("type = %d buffer size=%d is less than data size=%d\r\n"),type, bufferSize,
                  itemSize);
                status = VOS_STATUS_E_INVAL;
            }
            else {
                memcpy(&gnvEFSTable->halnv.tables.pwrOptimum[0],inputVoidBuffer,bufferSize);
            }
            break;
        case VNV_REGULARTORY_DOMAIN_TABLE:
            itemSize = sizeof(gnvEFSTable->halnv.tables.regDomains);
            if(bufferSize != itemSize) {
                VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR,
                 ("type = %d buffer size=%d is less than data size=%d\r\n"),type, bufferSize,
                  itemSize);
                status = VOS_STATUS_E_INVAL;
            }
            else {
                memcpy(&gnvEFSTable->halnv.tables.regDomains[0],inputVoidBuffer,bufferSize);
            }
            break;
        case VNV_DEFAULT_LOCATION:
            itemSize = sizeof(gnvEFSTable->halnv.tables.defaultCountryTable);
            if(bufferSize != itemSize) {
                VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR,
                 ("type = %d buffer size=%d is less than data size=%d\r\n"),type, bufferSize,
                  itemSize);
                status = VOS_STATUS_E_INVAL;
            }
            else {
                memcpy(&gnvEFSTable->halnv.tables.defaultCountryTable,inputVoidBuffer,bufferSize);
            }
            break;
        case VNV_TPC_POWER_TABLE:
            itemSize = sizeof(gnvEFSTable->halnv.tables.plutCharacterized);
            if(bufferSize != itemSize) {
                VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR,
                 ("type = %d buffer size=%d is less than data size=%d\r\n"),type, bufferSize,
                  itemSize);
                status = VOS_STATUS_E_INVAL;
            }
            else {
                memcpy(&gnvEFSTable->halnv.tables.plutCharacterized[0],inputVoidBuffer,bufferSize);
            }
            break;
        case VNV_TPC_PDADC_OFFSETS:
            itemSize = sizeof(gnvEFSTable->halnv.tables.plutPdadcOffset);
            if(bufferSize != itemSize) {
                VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR,
                 ("type = %d buffer size=%d is less than data size=%d\r\n"),type, bufferSize,
                  itemSize);
                status = VOS_STATUS_E_INVAL;
            }
            else {
                memcpy(&gnvEFSTable->halnv.tables.plutPdadcOffset[0],inputVoidBuffer,bufferSize);
            }
            break;
         case VNV_RSSI_CHANNEL_OFFSETS:

            itemSize = sizeof(gnvEFSTable->halnv.tables.rssiChanOffsets);

            if(bufferSize != itemSize) {

                VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR,
                 ("type = %d buffer size=%d is less than data size=%d\r\n"),type, bufferSize,
                  itemSize);
                status = VOS_STATUS_E_INVAL;
            }
            else {
                memcpy(&gnvEFSTable->halnv.tables.rssiChanOffsets[0],inputVoidBuffer,bufferSize);
            }
            break;
         case VNV_RF_CAL_VALUES:

            itemSize = sizeof(gnvEFSTable->halnv.tables.rFCalValues);

            if(bufferSize != itemSize) {

                VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR,
                 ("type = %d buffer size=%d is less than data size=%d\r\n"),type, bufferSize,
                  itemSize);
                status = VOS_STATUS_E_INVAL;
            }
            else {
                memcpy(&gnvEFSTable->halnv.tables.rFCalValues,inputVoidBuffer,bufferSize);
            }
            break;
        case VNV_ANTENNA_PATH_LOSS:
            itemSize = sizeof(gnvEFSTable->halnv.tables.antennaPathLoss);
            if(bufferSize != itemSize) {
                VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR,
                 ("type = %d buffer size=%d is less than data size=%d\r\n"),type, bufferSize,
                  itemSize);
                status = VOS_STATUS_E_INVAL;
            }
            else {
                memcpy(&gnvEFSTable->halnv.tables.antennaPathLoss[0],inputVoidBuffer,bufferSize);
            }
            break;

        case VNV_PACKET_TYPE_POWER_LIMITS:
            itemSize = sizeof(gnvEFSTable->halnv.tables.pktTypePwrLimits);
            if(bufferSize != itemSize) {
                VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR,
                 ("type = %d buffer size=%d is less than data size=%d\r\n"),type, bufferSize,
                  itemSize);
                status = VOS_STATUS_E_INVAL;
            }
            else {
                memcpy(&gnvEFSTable->halnv.tables.pktTypePwrLimits[0][0],inputVoidBuffer,bufferSize);
            }
            break;

        case VNV_OFDM_CMD_PWR_OFFSET:
            itemSize = sizeof(gnvEFSTable->halnv.tables.ofdmCmdPwrOffset);
            if(bufferSize != itemSize) {
                VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR,
                 ("type = %d buffer size=%d is less than data size=%d\r\n"),type, bufferSize,
                  itemSize);
                status = VOS_STATUS_E_INVAL;
            }
            else {
                memcpy(&gnvEFSTable->halnv.tables.ofdmCmdPwrOffset,inputVoidBuffer,bufferSize);
            }
            break;

        case VNV_TX_BB_FILTER_MODE:
            itemSize = sizeof(gnvEFSTable->halnv.tables.txbbFilterMode);
            if(bufferSize != itemSize) {
                VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR,
                 ("type = %d buffer size=%d is less than data size=%d\r\n"),type, bufferSize,
                  itemSize);
                status = VOS_STATUS_E_INVAL;
            }
            else {
                memcpy(&gnvEFSTable->halnv.tables.txbbFilterMode,inputVoidBuffer,bufferSize);
            }
            break;
            
#ifdef FEATURE_WLAN_NON_INTEGRATED_SOC
        case VNV_FREQUENCY_FOR_1_3V_SUPPLY:
            itemSize = sizeof(gnvEFSTable->halnv.tables.freqFor1p3VSupply);
            if(bufferSize != itemSize) {
                VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR,
                 ("type = %d buffer size=%d is less than data size=%d\r\n"),type, bufferSize,
                  itemSize);
                status = VOS_STATUS_E_INVAL;
            }
            else {
                memcpy(&gnvEFSTable->halnv.tables.freqFor1p3VSupply,inputVoidBuffer,bufferSize);
            }
            break;
#endif /*                                 */

        case VNV_TABLE_VIRTUAL_RATE:
            itemSize = sizeof(gnvEFSTable->halnv.tables.pwrOptimum_virtualRate);
            if(bufferSize != itemSize) {
                VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR,
                 ("type = %d buffer size=%d is less than data size=%d\r\n"),type, bufferSize,
                  itemSize);
                status = VOS_STATUS_E_INVAL;
            }
            else {
                memcpy(&gnvEFSTable->halnv.tables.pwrOptimum_virtualRate,inputVoidBuffer,bufferSize);
            }
            break;

        default:
          break;
    }
   if (VOS_STATUS_SUCCESS == status)
   {
      //                               
      status = vos_nv_setValidity( type, VOS_TRUE );
      if (! VOS_IS_STATUS_SUCCESS(status)) {
          VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR, ("vos_nv_setValidity failed!!!\r\n"));
          status = VOS_STATUS_E_FAULT;
      }
      status = wlan_write_to_efs((v_U8_t*)gnvEFSTable,sizeof(nvEFSTable_t));

      if (! VOS_IS_STATUS_SUCCESS(status)) {
          VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR, ("vos_nv_write_to_efs failed!!!\r\n"));
          status = VOS_STATUS_E_FAULT;
      }
   }
   return status;
}
#endif
  
/*                                                                         
                                                                          
                                                           
                                                 
                                                           
                                                 
                                                           
                                         
                                      
     
                                                                           */
VOS_STATUS vos_nv_getChannelListWithPower(tChannelListWithPower *channels20MHz /*                        */,
                                          tANI_U8 *num20MHzChannelsFound,
                                          tChannelListWithPower *channels40MHz /*                         */,
                                          tANI_U8 *num40MHzChannelsFound
                                          )
{
    VOS_STATUS status = VOS_STATUS_SUCCESS;
    int i, count;
    
    //                                                                              
    //                                          

    if( channels20MHz && num20MHzChannelsFound )
    {
        count = 0;
        for( i = 0; i <= RF_CHAN_14; i++ )
        {
            if( regChannels[i].enabled )
            {
                channels20MHz[count].chanId = rfChannels[i].channelNum;
                channels20MHz[count++].pwr  = regChannels[i].pwrLimit;
            }
        }
#ifdef FEATURE_WLAN_INTEGRATED_SOC
        for( i = RF_CHAN_36; i <= RF_CHAN_165; i++ )
        {
            if( regChannels[i].enabled )
            {
                channels20MHz[count].chanId = rfChannels[i].channelNum;
                channels20MHz[count++].pwr  = regChannels[i].pwrLimit;
            }
        }
#endif
        *num20MHzChannelsFound = (tANI_U8)count;
    }

    if( channels40MHz && num40MHzChannelsFound )
    {
        count = 0;
#ifdef FEATURE_WLAN_INTEGRATED_SOC
        //                                           
        for( i = RF_CHAN_BOND_3; i <= RF_CHAN_BOND_11; i++ )
        {
            
            if( regChannels[i].enabled )
            {
                channels40MHz[count].chanId = rfChannels[i].channelNum;
                channels40MHz[count++].pwr  = regChannels[i].pwrLimit;
            }
        }
        //                                         
        for( i = RF_CHAN_BOND_38; i <= RF_CHAN_BOND_163; i++ )
        {
            
            if( regChannels[i].enabled )
            {
                channels40MHz[count].chanId = rfChannels[i].channelNum;
                channels40MHz[count++].pwr  = regChannels[i].pwrLimit;
            }
        }
#endif
        *num40MHzChannelsFound = (tANI_U8)count;
    }
    return (status);
}

/*                                                                         
                                                                            
                                   
     
                                                                           */

v_REGDOMAIN_t vos_nv_getDefaultRegDomain( void )
{
    return countryInfoTable.countryInfo[0].regDomain;
}

/*                                                                         
                                                                       
                            
                                                 
                                                      
                                                 
                                                      
                                         
                                      
     
                                                                           */
VOS_STATUS vos_nv_getSupportedChannels( v_U8_t *p20MhzChannels, int *pNum20MhzChannels,
                                        v_U8_t *p40MhzChannels, int *pNum40MhzChannels)
{
    VOS_STATUS status = VOS_STATUS_E_INVAL;
    int i, count = 0;

    if( p20MhzChannels && pNum20MhzChannels )
    {
        if( *pNum20MhzChannels >= NUM_RF_CHANNELS )
        {
            for( i = 0; i <= RF_CHAN_14; i++ )
            {
                p20MhzChannels[count++] = rfChannels[i].channelNum;
            }
#ifdef FEATURE_WLAN_INTEGRATED_SOC
            for( i = RF_CHAN_36; i <= RF_CHAN_165; i++ )
            {
                p20MhzChannels[count++] = rfChannels[i].channelNum;
            }
#endif
            status = VOS_STATUS_SUCCESS;
        }
        *pNum20MhzChannels = count;
    }

    return (status);
}

/*                                                                         
                                                                            
                                                                          
                                         
     
                                                                           */
VOS_STATUS vos_nv_readDefaultCountryTable( uNvTables *tableData )
{
   
   VOS_STATUS status;

   status = vos_nv_read( VNV_DEFAULT_LOCATION, tableData, NULL, sizeof(sDefaultCountry) );

   return status;
}

/*                                                                         
                            
                                                
                                      
                                         
     
                                                                           */
VOS_STATUS vos_nv_getNVBuffer(v_VOID_t **pNvBuffer,v_SIZE_t *pSize)
{

   /*                                */
   *pNvBuffer = (v_VOID_t *)(&pnvEFSTable->halnv);
   *pSize = sizeof(sHalNv);

   return VOS_STATUS_SUCCESS;
}

#ifdef FEATURE_WLAN_INTEGRATED_SOC
/*                                                                         
                               
                                                         
                                           
                                         
     
                                                                           */
VOS_STATUS vos_nv_setRegDomain(void * clientCtxt, v_REGDOMAIN_t regId)
{
   /*                                            */
   if(regId >= REGDOMAIN_COUNT)
   {
      VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR,
                "VOS set reg domain, invalid REG domain ID %d", regId);
      return VOS_STATUS_E_INVAL;
   }

   /*                                                     */
   regChannels = pnvEFSTable->halnv.tables.regDomains[regId].channels;

   return VOS_STATUS_SUCCESS;
}

/*                                                                         
                                         
                                                              
                                                         
                      
                       
                  
     
                                                                           */
eNVChannelEnabledType vos_nv_getChannelEnabledState
(
   v_U32_t     rfChannel
)
{
   v_U32_t       channelLoop;
   eRfChannels   channelEnum = INVALID_RF_CHANNEL;

   for(channelLoop = 0; channelLoop <= RF_CHAN_165; channelLoop++)
   {
      if(rfChannels[channelLoop].channelNum == rfChannel)
      {
         channelEnum = (eRfChannels)channelLoop;
         break;
      }
   }

   if(INVALID_RF_CHANNEL == channelEnum)
   {
      VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR,
                "vos_nv_getChannelEnabledState, invalid cahnnel %d", rfChannel);
      return NV_CHANNEL_INVALID;
   }

   return regChannels[channelEnum].enabled;
}
#endif /*                                 */
