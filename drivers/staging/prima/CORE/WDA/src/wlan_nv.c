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

#ifndef WLAN_NV_C
#define WLAN_NV_C

#ifndef FEATURE_WLAN_INTEGRATED_SOC

#include "wlan_nv.h"

const sHalNv nvDefaults =
{
    {
        0,                                                              //                     
        0,                                                              //                        
        1,                                                              //                                                            
        1,                                                              //                         
        2,                                                              //                         
        { 0x00, 0xDE, 0xAD, 0xBE, 0xEF, 0x00 },                         //                                           
        { "\0" }
    }, //      

    {
        //                             
        {
            //                           
            //                                                   
            //                                                
            {
                    //             
                {2150},    //                              
                {2150},    //                              
                {2150},    //                                
                {2150},    //                               
                {2150},    //                               
                {2150},    //                                 
                {2150},    //                                

                    //         
                {2150},    //                            
                {2150},    //                           

                    //               
                {1750},    //                         
                {1750},    //                         
                {1750},    //                          
                {1750},    //                          
                {1750},    //                          
                {1750},    //                          
                {1750},    //                          
                {1750},    //                          

                    //                       
                {1750},    //                                
                {1750},    //                               
                {1750},    //                                 
                {1750},    //                               
                {1750},    //                               
                {1750},    //                               
                {1650},    //                                 
                {1550},    //                               
                {1750},    //                                      
                {1750},    //                                       
                {1750},    //                                       
                {1750},    //                                       
                {1750},    //                                       
                {1750},    //                                       
                {1650},    //                                     
                {1550}     //                                       
            },  //                       
        },


        //                            
        {
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


            {   //                     
                { //                        
                    //                 
                                       //           
                    {eANI_BOOLEAN_TRUE, 23},           //          
                    {eANI_BOOLEAN_TRUE, 23},           //          
                    {eANI_BOOLEAN_TRUE, 23},           //          
                    {eANI_BOOLEAN_TRUE, 23},           //          
                    {eANI_BOOLEAN_TRUE, 23},           //          
                    {eANI_BOOLEAN_TRUE, 23},           //          
                    {eANI_BOOLEAN_TRUE, 23},           //          
                    {eANI_BOOLEAN_TRUE, 23},           //          
                    {eANI_BOOLEAN_TRUE, 23},           //          
                    {eANI_BOOLEAN_TRUE, 22},           //           
                    {eANI_BOOLEAN_TRUE, 22},           //           
                    {eANI_BOOLEAN_FALSE, 30},           //           
                    {eANI_BOOLEAN_FALSE, 30},           //           
                    {eANI_BOOLEAN_FALSE, 30},           //           

                }, //                      
                {
                    { 0 },  //                   
                },

                { //                       
                    //           
                    { 0 },                       //          
                    { 0 },                       //          
                    { 0 },                       //          
                    { 0 },                       //          
                    { 0 },                       //          
                    { 0 },                       //          
                    { 0 },                       //          
                    { 0 },                       //          
                    { 0 },                       //          
                    { 0 },                       //           
                    { 0 },                       //           
                    { 0 },                       //           
                    { 0 },                       //           
                    { 0 },                       //           
                }, //                     

                { //                        
                    //           
                    { 0 },                       //          
                    { 0 },                       //          
                    { 0 },                       //          
                    { 0 },                       //          
                    { 0 },                       //          
                    { 0 },                       //          
                    { 0 },                       //          
                    { 0 },                       //          
                    { 0 },                       //          
                    { 0 },                       //           
                    { 0 },                       //           
                    { 0 },                       //           
                    { 0 },                       //           
                    { 0 },                       //           
                } //                      
            }, //                   

            {   //                      
                { //                        
                    //                 
                                       //           
                    {eANI_BOOLEAN_TRUE, 20},           //          
                    {eANI_BOOLEAN_TRUE, 20},           //          
                    {eANI_BOOLEAN_TRUE, 20},           //          
                    {eANI_BOOLEAN_TRUE, 20},           //          
                    {eANI_BOOLEAN_TRUE, 20},           //          
                    {eANI_BOOLEAN_TRUE, 20},           //          
                    {eANI_BOOLEAN_TRUE, 20},           //          
                    {eANI_BOOLEAN_TRUE, 20},           //          
                    {eANI_BOOLEAN_TRUE, 20},           //          
                    {eANI_BOOLEAN_TRUE, 20},           //           
                    {eANI_BOOLEAN_TRUE, 20},           //           
                    {eANI_BOOLEAN_TRUE, 19},           //           
                    {eANI_BOOLEAN_TRUE, 19},           //           
                    {eANI_BOOLEAN_FALSE, 30},           //           

                }, //                      
                {
                    { 0 },  //                   
                },

                { //                       
                    //           
                    { 0 },                       //          
                    { 0 },                       //          
                    { 0 },                       //          
                    { 0 },                       //          
                    { 0 },                       //          
                    { 0 },                       //          
                    { 0 },                       //          
                    { 0 },                       //          
                    { 0 },                       //          
                    { 0 },                       //           
                    { 0 },                       //           
                    { 0 },                       //           
                    { 0 },                       //           
                    { 0 },                       //           
                }, //                     

                { //                        
                    //           
                    { 0 },                       //          
                    { 0 },                       //          
                    { 0 },                       //          
                    { 0 },                       //          
                    { 0 },                       //          
                    { 0 },                       //          
                    { 0 },                       //          
                    { 0 },                       //          
                    { 0 },                       //          
                    { 0 },                       //           
                    { 0 },                       //           
                    { 0 },                       //           
                    { 0 },                       //           
                    { 0 },                       //           
                } //                      
            }, //                    

            {   //                       
                { //                        
                    //                 
                                       //           
                    {eANI_BOOLEAN_TRUE, 20},           //          
                    {eANI_BOOLEAN_TRUE, 20},           //          
                    {eANI_BOOLEAN_TRUE, 20},           //          
                    {eANI_BOOLEAN_TRUE, 20},           //          
                    {eANI_BOOLEAN_TRUE, 20},           //          
                    {eANI_BOOLEAN_TRUE, 20},           //          
                    {eANI_BOOLEAN_TRUE, 20},           //          
                    {eANI_BOOLEAN_TRUE, 20},           //          
                    {eANI_BOOLEAN_TRUE, 20},           //          
                    {eANI_BOOLEAN_TRUE, 20},           //           
                    {eANI_BOOLEAN_TRUE, 20},           //           
                    {eANI_BOOLEAN_TRUE, 20},           //           
                    {eANI_BOOLEAN_TRUE, 20},           //           
                    {eANI_BOOLEAN_TRUE, 18},           //           

                }, //                      
                {
                    { 0 },  //                   
                },

                { //                       
                    //           
                    { 0 },                       //          
                    { 0 },                       //          
                    { 0 },                       //          
                    { 0 },                       //          
                    { 0 },                       //          
                    { 0 },                       //          
                    { 0 },                       //          
                    { 0 },                       //          
                    { 0 },                       //          
                    { 0 },                       //           
                    { 0 },                       //           
                    { 0 },                       //           
                    { 0 },                       //           
                    { 0 },                       //           
                }, //                     

                { //                        
                    //           
                    { 0 },                       //          
                    { 0 },                       //          
                    { 0 },                       //          
                    { 0 },                       //          
                    { 0 },                       //          
                    { 0 },                       //          
                    { 0 },                       //          
                    { 0 },                       //          
                    { 0 },                       //          
                    { 0 },                       //           
                    { 0 },                       //           
                    { 0 },                       //           
                    { 0 },                       //           
                    { 0 },                       //           
                } //                      
            }, //                     

            {   //                       
                { //                        
                    //                 
                                       //           
                    {eANI_BOOLEAN_TRUE, 16},           //          
                    {eANI_BOOLEAN_TRUE, 16},           //          
                    {eANI_BOOLEAN_TRUE, 16},           //          
                    {eANI_BOOLEAN_TRUE, 16},           //          
                    {eANI_BOOLEAN_TRUE, 16},           //          
                    {eANI_BOOLEAN_TRUE, 16},           //          
                    {eANI_BOOLEAN_TRUE, 16},           //          
                    {eANI_BOOLEAN_TRUE, 16},           //          
                    {eANI_BOOLEAN_TRUE, 16},           //          
                    {eANI_BOOLEAN_TRUE, 16},           //           
                    {eANI_BOOLEAN_TRUE, 16},           //           
                    {eANI_BOOLEAN_FALSE, 0},           //           
                    {eANI_BOOLEAN_FALSE, 0},           //           
                    {eANI_BOOLEAN_FALSE, 0},           //           

                }, //                      
                {
                    { 0 },  //                   
                },

                { //                       
                    //           
                    { 0 },                       //          
                    { 0 },                       //          
                    { 0 },                       //          
                    { 0 },                       //          
                    { 0 },                       //          
                    { 0 },                       //          
                    { 0 },                       //          
                    { 0 },                       //          
                    { 0 },                       //          
                    { 0 },                       //           
                    { 0 },                       //           
                    { 0 },                       //           
                    { 0 },                       //           
                    { 0 },                       //           
                }, //                     

                { //                        
                    //           
                    { 0 },                       //          
                    { 0 },                       //          
                    { 0 },                       //          
                    { 0 },                       //          
                    { 0 },                       //          
                    { 0 },                       //          
                    { 0 },                       //          
                    { 0 },                       //          
                    { 0 },                       //          
                    { 0 },                       //           
                    { 0 },                       //           
                    { 0 },                       //           
                    { 0 },                       //           
                    { 0 },                       //           
                } //                      
            }, //                     

            {   //                                
                { //                        
                    //                 
                                       //           
                    {eANI_BOOLEAN_TRUE, 16},           //          
                    {eANI_BOOLEAN_TRUE, 16},           //          
                    {eANI_BOOLEAN_TRUE, 16},           //          
                    {eANI_BOOLEAN_TRUE, 16},           //          
                    {eANI_BOOLEAN_TRUE, 16},           //          
                    {eANI_BOOLEAN_TRUE, 16},           //          
                    {eANI_BOOLEAN_TRUE, 16},           //          
                    {eANI_BOOLEAN_TRUE, 16},           //          
                    {eANI_BOOLEAN_TRUE, 16},           //          
                    {eANI_BOOLEAN_TRUE, 16},           //           
                    {eANI_BOOLEAN_TRUE, 16},           //           
                    {eANI_BOOLEAN_FALSE, 30},           //           
                    {eANI_BOOLEAN_FALSE, 30},           //           
                    {eANI_BOOLEAN_FALSE, 30},           //           

                }, //                      
                {
                    { 0 },  //                   
                },

                { //                       
                    //           
                    { 0 },                       //          
                    { 0 },                       //          
                    { 0 },                       //          
                    { 0 },                       //          
                    { 0 },                       //          
                    { 0 },                       //          
                    { 0 },                       //          
                    { 0 },                       //          
                    { 0 },                       //          
                    { 0 },                       //           
                    { 0 },                       //           
                    { 0 },                       //           
                    { 0 },                       //           
                    { 0 },                       //           
                }, //                     

                { //                        
                    //           
                    { 0 },                       //          
                    { 0 },                       //          
                    { 0 },                       //          
                    { 0 },                       //          
                    { 0 },                       //          
                    { 0 },                       //          
                    { 0 },                       //          
                    { 0 },                       //          
                    { 0 },                       //          
                    { 0 },                       //           
                    { 0 },                       //           
                    { 0 },                       //           
                    { 0 },                       //           
                    { 0 },                       //           
                } //                      
            },   //                              

            {   //                      
                { //                        
                    //                 
                                       //           
                    {eANI_BOOLEAN_TRUE, 26},           //          
                    {eANI_BOOLEAN_TRUE, 26},           //          
                    {eANI_BOOLEAN_TRUE, 26},           //          
                    {eANI_BOOLEAN_TRUE, 26},           //          
                    {eANI_BOOLEAN_TRUE, 26},           //          
                    {eANI_BOOLEAN_TRUE, 26},           //          
                    {eANI_BOOLEAN_TRUE, 26},           //          
                    {eANI_BOOLEAN_TRUE, 26},           //          
                    {eANI_BOOLEAN_TRUE, 26},           //          
                    {eANI_BOOLEAN_TRUE, 26},           //           
                    {eANI_BOOLEAN_TRUE, 26},           //           
                    {eANI_BOOLEAN_TRUE, 26},           //           
                    {eANI_BOOLEAN_TRUE, 26},           //           
                    {eANI_BOOLEAN_FALSE, 0},           //           

                }, //                      
                {
                    { 0 },  //                   
                },

                { //                       
                    //           
                    { 0 },                       //          
                    { 0 },                       //          
                    { 0 },                       //          
                    { 0 },                       //          
                    { 0 },                       //          
                    { 0 },                       //          
                    { 0 },                       //          
                    { 0 },                       //          
                    { 0 },                       //          
                    { 0 },                       //           
                    { 0 },                       //           
                    { 0 },                       //           
                    { 0 },                       //           
                    { 0 },                       //           
                }, //                     

                { //                        
                    //           
                    { 0 },                       //          
                    { 0 },                       //          
                    { 0 },                       //          
                    { 0 },                       //          
                    { 0 },                       //          
                    { 0 },                       //          
                    { 0 },                       //          
                    { 0 },                       //          
                    { 0 },                       //          
                    { 0 },                       //           
                    { 0 },                       //           
                    { 0 },                       //           
                    { 0 },                       //           
                    { 0 },                       //           
                } //                      
            }, //                    

            {   //                       
                { //                        
                    //                 
                                       //           
                    {eANI_BOOLEAN_TRUE, 15},           //          
                    {eANI_BOOLEAN_TRUE, 15},           //          
                    {eANI_BOOLEAN_TRUE, 15},           //          
                    {eANI_BOOLEAN_TRUE, 15},           //          
                    {eANI_BOOLEAN_TRUE, 15},           //          
                    {eANI_BOOLEAN_TRUE, 15},           //          
                    {eANI_BOOLEAN_TRUE, 15},           //          
                    {eANI_BOOLEAN_TRUE, 15},           //          
                    {eANI_BOOLEAN_TRUE, 15},           //          
                    {eANI_BOOLEAN_TRUE, 15},           //           
                    {eANI_BOOLEAN_TRUE, 15},           //           
                    {eANI_BOOLEAN_TRUE, 15},           //           
                    {eANI_BOOLEAN_TRUE, 15},           //           
                    {eANI_BOOLEAN_FALSE, 0},           //           

                }, //                      
                {
                    { 0 },  //                   
                },

                { //                       
                    //           
                    { 0 },                       //          
                    { 0 },                       //          
                    { 0 },                       //          
                    { 0 },                       //          
                    { 0 },                       //          
                    { 0 },                       //          
                    { 0 },                       //          
                    { 0 },                       //          
                    { 0 },                       //          
                    { 0 },                       //           
                    { 0 },                       //           
                    { 0 },                       //           
                    { 0 },                       //           
                    { 0 },                       //           
                }, //                     

                { //                        
                    //           
                    { 0 },                       //          
                    { 0 },                       //          
                    { 0 },                       //          
                    { 0 },                       //          
                    { 0 },                       //          
                    { 0 },                       //          
                    { 0 },                       //          
                    { 0 },                       //          
                    { 0 },                       //          
                    { 0 },                       //           
                    { 0 },                       //           
                    { 0 },                       //           
                    { 0 },                       //           
                    { 0 },                       //           
                } //                      
            }, //                     

            {   //                         
                { //                        
                    //                 
                                       //           
                    {eANI_BOOLEAN_TRUE, 14},           //          
                    {eANI_BOOLEAN_TRUE, 14},           //          
                    {eANI_BOOLEAN_TRUE, 14},           //          
                    {eANI_BOOLEAN_TRUE, 14},           //          
                    {eANI_BOOLEAN_TRUE, 14},           //          
                    {eANI_BOOLEAN_TRUE, 14},           //          
                    {eANI_BOOLEAN_TRUE, 14},           //          
                    {eANI_BOOLEAN_TRUE, 14},           //          
                    {eANI_BOOLEAN_TRUE, 14},           //          
                    {eANI_BOOLEAN_TRUE, 14},           //           
                    {eANI_BOOLEAN_TRUE, 14},           //           
                    {eANI_BOOLEAN_TRUE, 14},           //           
                    {eANI_BOOLEAN_TRUE, 14},           //           
                    {eANI_BOOLEAN_FALSE, 0},           //           

                }, //                      
                {
                    { 0 },  //                   
                },

                { //                       
                    //           
                    { 0 },                       //          
                    { 0 },                       //          
                    { 0 },                       //          
                    { 0 },                       //          
                    { 0 },                       //          
                    { 0 },                       //          
                    { 0 },                       //          
                    { 0 },                       //          
                    { 0 },                       //          
                    { 0 },                       //           
                    { 0 },                       //           
                    { 0 },                       //           
                    { 0 },                       //           
                    { 0 },                       //           
                }, //                     

                { //                        
                    //           
                    { 0 },                       //          
                    { 0 },                       //          
                    { 0 },                       //          
                    { 0 },                       //          
                    { 0 },                       //          
                    { 0 },                       //          
                    { 0 },                       //          
                    { 0 },                       //          
                    { 0 },                       //          
                    { 0 },                       //           
                    { 0 },                       //           
                    { 0 },                       //           
                    { 0 },                       //           
                    { 0 },                       //           
                } //                      
            }, //                       

            {   //                         
                { //                        
                    //                 
                                       //           
                    {eANI_BOOLEAN_TRUE, 12},           //          
                    {eANI_BOOLEAN_TRUE, 12},           //          
                    {eANI_BOOLEAN_TRUE, 12},           //          
                    {eANI_BOOLEAN_TRUE, 12},           //          
                    {eANI_BOOLEAN_TRUE, 12},           //          
                    {eANI_BOOLEAN_TRUE, 12},           //          
                    {eANI_BOOLEAN_TRUE, 12},           //          
                    {eANI_BOOLEAN_TRUE, 12},           //          
                    {eANI_BOOLEAN_TRUE, 12},           //          
                    {eANI_BOOLEAN_TRUE, 12},           //           
                    {eANI_BOOLEAN_TRUE, 12},           //           
                    {eANI_BOOLEAN_TRUE, 12},           //           
                    {eANI_BOOLEAN_TRUE, 12},           //           
                    {eANI_BOOLEAN_FALSE, 0},           //           

                }, //                      
                {
                    { 0 },  //                   
                },

                { //                       
                    //           
                    { 0 },                       //          
                    { 0 },                       //          
                    { 0 },                       //          
                    { 0 },                       //          
                    { 0 },                       //          
                    { 0 },                       //          
                    { 0 },                       //          
                    { 0 },                       //          
                    { 0 },                       //          
                    { 0 },                       //           
                    { 0 },                       //           
                    { 0 },                       //           
                    { 0 },                       //           
                    { 0 },                       //           
                }, //                     

                { //                        
                    //           
                    { 0 },                       //          
                    { 0 },                       //          
                    { 0 },                       //          
                    { 0 },                       //          
                    { 0 },                       //          
                    { 0 },                       //          
                    { 0 },                       //          
                    { 0 },                       //          
                    { 0 },                       //          
                    { 0 },                       //           
                    { 0 },                       //           
                    { 0 },                       //           
                    { 0 },                       //           
                    { 0 },                       //           
                } //                      
            } //                       
        },

        //                         
        {
            //               
            //  
            //                                                                                
            //                                                                             
            //                  

            0,                  //          
            { 'U', 'S', 'I' }   //            
        },

        //                        
        {
            {
                {
                    0  , // 
                    41 , // 
                    43 , // 
                    45 , // 
                    47 , // 
                    49 , // 
                    51 , // 
                    53 , // 
                    55 , // 
                    56 , // 
                    58 , //  
                    59 , //  
                    60 , //  
                    62 , //  
                    63 , //  
                    64 , //  
                    65 , //  
                    67 , //  
                    68 , //  
                    69 , //  
                    70 , //  
                    71 , //  
                    72 , //  
                    73 , //  
                    74 , //  
                    75 , //  
                    75 , //  
                    76 , //  
                    77 , //  
                    78 , //  
                    78 , //  
                    79 , //  
                    80 , //  
                    81 , //  
                    82 , //  
                    82 , //  
                    83 , //  
                    83 , //  
                    84 , //  
                    85 , //  
                    86 , //  
                    86 , //  
                    87 , //  
                    88 , //  
                    89 , //  
                    89 , //  
                    90 , //  
                    91 , //  
                    91 , //  
                    92 , //  
                    92 , //  
                    93 , //  
                    93 , //  
                    94 , //  
                    94 , //  
                    95 , //  
                    95 , //  
                    95 , //  
                    96 , //  
                    96 , //  
                    97 , //  
                    97 , //  
                    98 , //  
                    98 , //  
                    98 , //  
                    99 , //  
                    99 , //  
                    99 , //  
                    100, //  
                    100, //  
                    100, //  
                    101, //  
                    101, //  
                    102, //  
                    102, //  
                    102, //  
                    102, //  
                    103, //  
                    103, //  
                    103, //  
                    103, //  
                    104, //  
                    104, //  
                    104, //  
                    104, //  
                    105, //  
                    105, //  
                    105, //  
                    105, //  
                    105, //  
                    106, //  
                    106, //  
                    106, //  
                    106, //  
                    106, //  
                    106, //  
                    106, //  
                    106, //  
                    106, //  
                    106, //  
                    106, //   
                    106, //   
                    106, //   
                    106, //   
                    106, //   
                    106, //   
                    107, //   
                    107, //   
                    107, //   
                    107, //   
                    107, //   
                    107, //   
                    107, //   
                    107, //   
                    107, //   
                    107, //   
                    107, //   
                    107, //   
                    107, //   
                    107, //   
                    107, //   
                    107, //   
                    107, //   
                    107, //   
                    107, //   
                    107, //   
                    107, //   
                    107, //   
                }
            }, //         
            {
                {
                    0  , // 
                    41 , // 
                    43 , // 
                    45 , // 
                    47 , // 
                    49 , // 
                    51 , // 
                    52 , // 
                    54 , // 
                    56 , // 
                    57 , //  
                    59 , //  
                    60 , //  
                    61 , //  
                    62 , //  
                    64 , //  
                    65 , //  
                    66 , //  
                    67 , //  
                    68 , //  
                    69 , //  
                    70 , //  
                    71 , //  
                    72 , //  
                    73 , //  
                    74 , //  
                    75 , //  
                    75 , //  
                    76 , //  
                    77 , //  
                    78 , //  
                    79 , //  
                    79 , //  
                    80 , //  
                    81 , //  
                    82 , //  
                    82 , //  
                    83 , //  
                    84 , //  
                    85 , //  
                    85 , //  
                    86 , //  
                    87 , //  
                    88 , //  
                    88 , //  
                    89 , //  
                    89 , //  
                    90 , //  
                    91 , //  
                    91 , //  
                    92 , //  
                    92 , //  
                    93 , //  
                    93 , //  
                    94 , //  
                    94 , //  
                    95 , //  
                    95 , //  
                    96 , //  
                    96 , //  
                    96 , //  
                    97 , //  
                    97 , //  
                    98 , //  
                    98 , //  
                    98 , //  
                    99 , //  
                    99 , //  
                    99 , //  
                    100, //  
                    100, //  
                    101, //  
                    101, //  
                    101, //  
                    101, //  
                    102, //  
                    102, //  
                    102, //  
                    103, //  
                    103, //  
                    103, //  
                    104, //  
                    104, //  
                    104, //  
                    104, //  
                    105, //  
                    105, //  
                    105, //  
                    105, //  
                    105, //  
                    106, //  
                    106, //  
                    106, //  
                    106, //  
                    106, //  
                    106, //  
                    106, //  
                    106, //  
                    106, //  
                    106, //  
                    106, //   
                    106, //   
                    106, //   
                    107, //   
                    107, //   
                    107, //   
                    107, //   
                    107, //   
                    107, //   
                    107, //   
                    107, //   
                    107, //   
                    107, //   
                    107, //   
                    107, //   
                    107, //   
                    107, //   
                    107, //   
                    107, //   
                    107, //   
                    107, //   
                    107, //   
                    107, //   
                    107, //   
                    107, //   
                    107, //   
                    107, //   
                    107, //   
                }
            }, //         
                {
                    {
                        0  , // 
                        41 , // 
                        43 , // 
                        45 , // 
                        47 , // 
                        49 , // 
                        51 , // 
                        52 , // 
                        54 , // 
                        55 , // 
                        57 , //  
                        58 , //  
                        60 , //  
                        61 , //  
                        62 , //  
                        64 , //  
                        65 , //  
                        66 , //  
                        67 , //  
                        68 , //  
                        69 , //  
                        70 , //  
                        71 , //  
                        72 , //  
                        73 , //  
                        74 , //  
                        75 , //  
                        75 , //  
                        76 , //  
                        77 , //  
                        78 , //  
                        78 , //  
                        79 , //  
                        80 , //  
                        81 , //  
                        82 , //  
                        82 , //  
                        83 , //  
                        84 , //  
                        84 , //  
                        85 , //  
                        86 , //  
                        87 , //  
                        87 , //  
                        88 , //  
                        89 , //  
                        89 , //  
                        90 , //  
                        90 , //  
                        91 , //  
                        91 , //  
                        92 , //  
                        93 , //  
                        93 , //  
                        94 , //  
                        94 , //  
                        94 , //  
                        95 , //  
                        95 , //  
                        96 , //  
                        96 , //  
                        97 , //  
                        97 , //  
                        97 , //  
                        98 , //  
                        98 , //  
                        99 , //  
                        99 , //  
                        99 , //  
                        100, //  
                        100, //  
                        100, //  
                        101, //  
                        101, //  
                        101, //  
                        102, //  
                        102, //  
                        102, //  
                        103, //  
                        103, //  
                        103, //  
                        103, //  
                        104, //  
                        104, //  
                        104, //  
                        104, //  
                        104, //  
                        105, //  
                        105, //  
                        105, //  
                        105, //  
                        105, //  
                        105, //  
                        105, //  
                        105, //  
                        105, //  
                        105, //  
                        105, //  
                        105, //  
                        106, //  
                        106, //   
                        106, //   
                        106, //   
                        106, //   
                        106, //   
                        106, //   
                        106, //   
                        106, //   
                        106, //   
                        106, //   
                        106, //   
                        106, //   
                        106, //   
                        106, //   
                        106, //   
                        106, //   
                        106, //   
                        106, //   
                        106, //   
                        106, //   
                        106, //   
                        106, //   
                        106, //   
                        106, //   
                        106, //   
                        106, //   
                        106, //   
                        106, //   
                    }
                }, //         
                {
                    {
                        0  , // 
                        42 , // 
                        44 , // 
                        46 , // 
                        48 , // 
                        49 , // 
                        51 , // 
                        53 , // 
                        55 , // 
                        57 , // 
                        58 , //  
                        60 , //  
                        61 , //  
                        62 , //  
                        63 , //  
                        64 , //  
                        66 , //  
                        67 , //  
                        68 , //  
                        69 , //  
                        70 , //  
                        71 , //  
                        72 , //  
                        73 , //  
                        74 , //  
                        75 , //  
                        75 , //  
                        76 , //  
                        77 , //  
                        78 , //  
                        78 , //  
                        79 , //  
                        80 , //  
                        81 , //  
                        82 , //  
                        82 , //  
                        83 , //  
                        84 , //  
                        84 , //  
                        85 , //  
                        86 , //  
                        87 , //  
                        87 , //  
                        88 , //  
                        88 , //  
                        89 , //  
                        90 , //  
                        90 , //  
                        91 , //  
                        91 , //  
                        92 , //  
                        92 , //  
                        93 , //  
                        93 , //  
                        94 , //  
                        94 , //  
                        95 , //  
                        95 , //  
                        95 , //  
                        96 , //  
                        96 , //  
                        97 , //  
                        97 , //  
                        98 , //  
                        98 , //  
                        98 , //  
                        99 , //  
                        99 , //  
                        99 , //  
                        100, //  
                        100, //  
                        100, //  
                        101, //  
                        101, //  
                        101, //  
                        102, //  
                        102, //  
                        102, //  
                        103, //  
                        103, //  
                        103, //  
                        103, //  
                        104, //  
                        104, //  
                        104, //  
                        104, //  
                        104, //  
                        104, //  
                        104, //  
                        104, //  
                        105, //  
                        105, //  
                        105, //  
                        105, //  
                        105, //  
                        105, //  
                        105, //  
                        105, //  
                        105, //  
                        105, //  
                        105, //   
                        105, //   
                        105, //   
                        105, //   
                        105, //   
                        106, //   
                        106, //   
                        106, //   
                        106, //   
                        106, //   
                        106, //   
                        106, //   
                        106, //   
                        106, //   
                        106, //   
                        106, //   
                        106, //   
                        106, //   
                        106, //   
                        106, //   
                        106, //   
                        106, //   
                        106, //   
                        106, //   
                        106, //   
                        106, //   
                        106, //   
                        106, //   
                    }
                }, //         
                {
                    {
                        0  , // 
                        41 , // 
                        43 , // 
                        45 , // 
                        47 , // 
                        49 , // 
                        51 , // 
                        53 , // 
                        54 , // 
                        56 , // 
                        57 , //  
                        59 , //  
                        60 , //  
                        62 , //  
                        63 , //  
                        65 , //  
                        66 , //  
                        67 , //  
                        68 , //  
                        69 , //  
                        69 , //  
                        71 , //  
                        72 , //  
                        72 , //  
                        73 , //  
                        74 , //  
                        75 , //  
                        76 , //  
                        77 , //  
                        78 , //  
                        79 , //  
                        79 , //  
                        80 , //  
                        81 , //  
                        82 , //  
                        83 , //  
                        83 , //  
                        84 , //  
                        85 , //  
                        86 , //  
                        87 , //  
                        87 , //  
                        88 , //  
                        89 , //  
                        89 , //  
                        90 , //  
                        91 , //  
                        91 , //  
                        92 , //  
                        92 , //  
                        93 , //  
                        93 , //  
                        94 , //  
                        94 , //  
                        95 , //  
                        95 , //  
                        96 , //  
                        96 , //  
                        96 , //  
                        97 , //  
                        97 , //  
                        98 , //  
                        98 , //  
                        98 , //  
                        99 , //  
                        99 , //  
                        100, //  
                        100, //  
                        100, //  
                        101, //  
                        101, //  
                        101, //  
                        102, //  
                        102, //  
                        102, //  
                        103, //  
                        103, //  
                        103, //  
                        103, //  
                        104, //  
                        104, //  
                        104, //  
                        104, //  
                        105, //  
                        105, //  
                        105, //  
                        105, //  
                        105, //  
                        105, //  
                        105, //  
                        105, //  
                        105, //  
                        106, //  
                        106, //  
                        106, //  
                        106, //  
                        106, //  
                        106, //  
                        106, //  
                        106, //  
                        106, //   
                        106, //   
                        106, //   
                        106, //   
                        106, //   
                        106, //   
                        106, //   
                        106, //   
                        106, //   
                        106, //   
                        106, //   
                        106, //   
                        106, //   
                        106, //   
                        106, //   
                        106, //   
                        106, //   
                        106, //   
                        106, //   
                        106, //   
                        106, //   
                        106, //   
                        106, //   
                        106, //   
                        106, //   
                        106, //   
                        106, //   
                        106, //   
                    }
                }, //         
                {
                    {
                        0  , // 
                        41 , // 
                        43 , // 
                        45 , // 
                        47 , // 
                        49 , // 
                        51 , // 
                        53 , // 
                        55 , // 
                        56 , // 
                        58 , //  
                        59 , //  
                        61 , //  
                        62 , //  
                        63 , //  
                        64 , //  
                        65 , //  
                        66 , //  
                        68 , //  
                        69 , //  
                        70 , //  
                        71 , //  
                        72 , //  
                        73 , //  
                        74 , //  
                        75 , //  
                        76 , //  
                        77 , //  
                        77 , //  
                        78 , //  
                        79 , //  
                        80 , //  
                        80 , //  
                        81 , //  
                        82 , //  
                        83 , //  
                        83 , //  
                        84 , //  
                        85 , //  
                        86 , //  
                        87 , //  
                        87 , //  
                        88 , //  
                        89 , //  
                        89 , //  
                        90 , //  
                        91 , //  
                        91 , //  
                        92 , //  
                        92 , //  
                        93 , //  
                        93 , //  
                        94 , //  
                        94 , //  
                        95 , //  
                        95 , //  
                        96 , //  
                        96 , //  
                        97 , //  
                        97 , //  
                        98 , //  
                        98 , //  
                        98 , //  
                        99 , //  
                        99 , //  
                        100, //  
                        100, //  
                        100, //  
                        101, //  
                        101, //  
                        101, //  
                        102, //  
                        102, //  
                        102, //  
                        103, //  
                        103, //  
                        103, //  
                        103, //  
                        104, //  
                        104, //  
                        104, //  
                        104, //  
                        104, //  
                        105, //  
                        105, //  
                        105, //  
                        105, //  
                        105, //  
                        105, //  
                        105, //  
                        106, //  
                        106, //  
                        106, //  
                        106, //  
                        106, //  
                        106, //  
                        106, //  
                        106, //  
                        106, //  
                        106, //  
                        106, //   
                        106, //   
                        106, //   
                        106, //   
                        106, //   
                        106, //   
                        106, //   
                        106, //   
                        106, //   
                        106, //   
                        106, //   
                        107, //   
                        107, //   
                        107, //   
                        107, //   
                        107, //   
                        107, //   
                        107, //   
                        107, //   
                        107, //   
                        107, //   
                        107, //   
                        107, //   
                        107, //   
                        107, //   
                        107, //   
                        107, //   
                        107, //   
                    }
                }, //         
                {
                    {
                        0  , // 
                        41 , // 
                        43 , // 
                        45 , // 
                        47 , // 
                        49 , // 
                        51 , // 
                        53 , // 
                        55 , // 
                        56 , // 
                        58 , //  
                        60 , //  
                        61 , //  
                        62 , //  
                        63 , //  
                        64 , //  
                        66 , //  
                        67 , //  
                        68 , //  
                        69 , //  
                        70 , //  
                        71 , //  
                        72 , //  
                        73 , //  
                        74 , //  
                        75 , //  
                        76 , //  
                        77 , //  
                        77 , //  
                        78 , //  
                        79 , //  
                        80 , //  
                        80 , //  
                        81 , //  
                        82 , //  
                        83 , //  
                        84 , //  
                        84 , //  
                        85 , //  
                        86 , //  
                        87 , //  
                        87 , //  
                        88 , //  
                        88 , //  
                        89 , //  
                        90 , //  
                        90 , //  
                        91 , //  
                        91 , //  
                        92 , //  
                        92 , //  
                        93 , //  
                        93 , //  
                        94 , //  
                        94 , //  
                        95 , //  
                        95 , //  
                        96 , //  
                        96 , //  
                        97 , //  
                        97 , //  
                        97 , //  
                        98 , //  
                        98 , //  
                        99 , //  
                        99 , //  
                        99 , //  
                        100, //  
                        100, //  
                        100, //  
                        101, //  
                        101, //  
                        101, //  
                        102, //  
                        102, //  
                        102, //  
                        103, //  
                        103, //  
                        103, //  
                        103, //  
                        104, //  
                        104, //  
                        104, //  
                        104, //  
                        104, //  
                        104, //  
                        105, //  
                        105, //  
                        105, //  
                        105, //  
                        105, //  
                        105, //  
                        105, //  
                        105, //  
                        105, //  
                        105, //  
                        105, //  
                        105, //  
                        106, //  
                        106, //  
                        106, //   
                        106, //   
                        106, //   
                        106, //   
                        106, //   
                        106, //   
                        106, //   
                        106, //   
                        106, //   
                        106, //   
                        106, //   
                        106, //   
                        106, //   
                        106, //   
                        106, //   
                        106, //   
                        106, //   
                        106, //   
                        106, //   
                        106, //   
                        106, //   
                        106, //   
                        106, //   
                        106, //   
                        106, //   
                        106, //   
                        106, //   
                        106, //   
                    }
                }, //         
                {
                    {
                        0  , // 
                        40 , // 
                        42 , // 
                        45 , // 
                        47 , // 
                        49 , // 
                        51 , // 
                        52 , // 
                        54 , // 
                        56 , // 
                        58 , //  
                        59 , //  
                        61 , //  
                        62 , //  
                        63 , //  
                        65 , //  
                        66 , //  
                        67 , //  
                        68 , //  
                        69 , //  
                        70 , //  
                        71 , //  
                        72 , //  
                        73 , //  
                        74 , //  
                        75 , //  
                        76 , //  
                        77 , //  
                        77 , //  
                        78 , //  
                        79 , //  
                        80 , //  
                        81 , //  
                        81 , //  
                        82 , //  
                        83 , //  
                        84 , //  
                        85 , //  
                        86 , //  
                        86 , //  
                        87 , //  
                        88 , //  
                        89 , //  
                        89 , //  
                        90 , //  
                        91 , //  
                        91 , //  
                        92 , //  
                        92 , //  
                        93 , //  
                        93 , //  
                        94 , //  
                        94 , //  
                        95 , //  
                        95 , //  
                        96 , //  
                        96 , //  
                        97 , //  
                        97 , //  
                        97 , //  
                        98 , //  
                        98 , //  
                        99 , //  
                        99 , //  
                        99 , //  
                        100, //  
                        100, //  
                        100, //  
                        101, //  
                        101, //  
                        102, //  
                        102, //  
                        102, //  
                        103, //  
                        103, //  
                        103, //  
                        104, //  
                        104, //  
                        104, //  
                        104, //  
                        105, //  
                        105, //  
                        105, //  
                        105, //  
                        105, //  
                        105, //  
                        105, //  
                        105, //  
                        106, //  
                        106, //  
                        106, //  
                        106, //  
                        106, //  
                        106, //  
                        106, //  
                        106, //  
                        106, //  
                        106, //  
                        106, //  
                        106, //  
                        106, //   
                        106, //   
                        106, //   
                        106, //   
                        106, //   
                        107, //   
                        107, //   
                        107, //   
                        107, //   
                        107, //   
                        107, //   
                        107, //   
                        107, //   
                        107, //   
                        107, //   
                        107, //   
                        107, //   
                        107, //   
                        107, //   
                        107, //   
                        107, //   
                        107, //   
                        107, //   
                        107, //   
                        107, //   
                        107, //   
                        107, //   
                        107, //   
                    }
                }, //         
                {
                    {
                        0  , // 
                        41 , // 
                        44 , // 
                        46 , // 
                        48 , // 
                        50 , // 
                        52 , // 
                        54 , // 
                        56 , // 
                        58 , // 
                        59 , //  
                        60 , //  
                        62 , //  
                        63 , //  
                        64 , //  
                        66 , //  
                        67 , //  
                        68 , //  
                        69 , //  
                        70 , //  
                        71 , //  
                        72 , //  
                        73 , //  
                        74 , //  
                        75 , //  
                        76 , //  
                        77 , //  
                        78 , //  
                        79 , //  
                        79 , //  
                        80 , //  
                        81 , //  
                        82 , //  
                        83 , //  
                        83 , //  
                        84 , //  
                        85 , //  
                        86 , //  
                        87 , //  
                        87 , //  
                        88 , //  
                        89 , //  
                        89 , //  
                        90 , //  
                        91 , //  
                        91 , //  
                        92 , //  
                        92 , //  
                        93 , //  
                        93 , //  
                        94 , //  
                        94 , //  
                        95 , //  
                        95 , //  
                        96 , //  
                        96 , //  
                        97 , //  
                        97 , //  
                        98 , //  
                        98 , //  
                        98 , //  
                        99 , //  
                        99 , //  
                        100, //  
                        100, //  
                        100, //  
                        101, //  
                        101, //  
                        101, //  
                        102, //  
                        102, //  
                        103, //  
                        103, //  
                        103, //  
                        104, //  
                        104, //  
                        104, //  
                        104, //  
                        105, //  
                        105, //  
                        105, //  
                        105, //  
                        105, //  
                        105, //  
                        106, //  
                        106, //  
                        106, //  
                        106, //  
                        106, //  
                        106, //  
                        106, //  
                        106, //  
                        106, //  
                        106, //  
                        106, //  
                        106, //  
                        106, //  
                        106, //  
                        106, //  
                        107, //  
                        107, //   
                        107, //   
                        107, //   
                        107, //   
                        107, //   
                        107, //   
                        107, //   
                        107, //   
                        107, //   
                        107, //   
                        107, //   
                        107, //   
                        107, //   
                        107, //   
                        107, //   
                        107, //   
                        107, //   
                        107, //   
                        107, //   
                        107, //   
                        107, //   
                        107, //   
                        107, //   
                        107, //   
                        107, //   
                        107, //   
                        107, //   
                        107, //   
                    }
                }, //         
                {
                    {
                        0  , // 
                        41 , // 
                        43 , // 
                        47 , // 
                        48 , // 
                        50 , // 
                        52 , // 
                        53 , // 
                        55 , // 
                        57 , // 
                        58 , //  
                        60 , //  
                        62 , //  
                        63 , //  
                        64 , //  
                        65 , //  
                        67 , //  
                        68 , //  
                        69 , //  
                        70 , //  
                        71 , //  
                        72 , //  
                        73 , //  
                        74 , //  
                        75 , //  
                        76 , //  
                        77 , //  
                        77 , //  
                        78 , //  
                        79 , //  
                        80 , //  
                        81 , //  
                        82 , //  
                        83 , //  
                        84 , //  
                        85 , //  
                        85 , //  
                        86 , //  
                        87 , //  
                        88 , //  
                        89 , //  
                        89 , //  
                        90 , //  
                        90 , //  
                        91 , //  
                        92 , //  
                        92 , //  
                        93 , //  
                        94 , //  
                        94 , //  
                        95 , //  
                        95 , //  
                        96 , //  
                        96 , //  
                        96 , //  
                        97 , //  
                        97 , //  
                        98 , //  
                        98 , //  
                        99 , //  
                        99 , //  
                        99 , //  
                        100, //  
                        100, //  
                        101, //  
                        101, //  
                        102, //  
                        102, //  
                        102, //  
                        103, //  
                        103, //  
                        103, //  
                        104, //  
                        104, //  
                        104, //  
                        105, //  
                        105, //  
                        105, //  
                        105, //  
                        105, //  
                        106, //  
                        106, //  
                        106, //  
                        106, //  
                        106, //  
                        106, //  
                        106, //  
                        106, //  
                        106, //  
                        107, //  
                        107, //  
                        107, //  
                        107, //  
                        107, //  
                        107, //  
                        107, //  
                        107, //  
                        107, //  
                        107, //  
                        107, //  
                        107, //   
                        107, //   
                        107, //   
                        107, //   
                        107, //   
                        107, //   
                        107, //   
                        107, //   
                        107, //   
                        107, //   
                        107, //   
                        107, //   
                        107, //   
                        107, //   
                        107, //   
                        107, //   
                        107, //   
                        107, //   
                        107, //   
                        107, //   
                        107, //   
                        107, //   
                        107, //   
                        107, //   
                        107, //   
                        107, //   
                        107, //   
                        107, //   
                    }
                }, //          
                {
                    {
                        0  , // 
                        42 , // 
                        44 , // 
                        47 , // 
                        49 , // 
                        51 , // 
                        52 , // 
                        54 , // 
                        55 , // 
                        57 , // 
                        58 , //  
                        60 , //  
                        61 , //  
                        63 , //  
                        64 , //  
                        65 , //  
                        66 , //  
                        67 , //  
                        69 , //  
                        70 , //  
                        71 , //  
                        72 , //  
                        73 , //  
                        74 , //  
                        75 , //  
                        76 , //  
                        77 , //  
                        77 , //  
                        78 , //  
                        79 , //  
                        80 , //  
                        81 , //  
                        82 , //  
                        82 , //  
                        83 , //  
                        84 , //  
                        85 , //  
                        86 , //  
                        86 , //  
                        87 , //  
                        88 , //  
                        89 , //  
                        90 , //  
                        90 , //  
                        91 , //  
                        91 , //  
                        92 , //  
                        92 , //  
                        93 , //  
                        93 , //  
                        94 , //  
                        94 , //  
                        95 , //  
                        96 , //  
                        96 , //  
                        97 , //  
                        97 , //  
                        97 , //  
                        98 , //  
                        98 , //  
                        99 , //  
                        99 , //  
                        100, //  
                        100, //  
                        100, //  
                        101, //  
                        101, //  
                        101, //  
                        102, //  
                        102, //  
                        102, //  
                        103, //  
                        103, //  
                        103, //  
                        103, //  
                        103, //  
                        103, //  
                        104, //  
                        104, //  
                        104, //  
                        104, //  
                        104, //  
                        104, //  
                        104, //  
                        104, //  
                        104, //  
                        104, //  
                        104, //  
                        105, //  
                        105, //  
                        105, //  
                        105, //  
                        105, //  
                        105, //  
                        105, //  
                        105, //  
                        105, //  
                        105, //  
                        105, //  
                        105, //  
                        105, //   
                        105, //   
                        105, //   
                        105, //   
                        105, //   
                        105, //   
                        105, //   
                        105, //   
                        105, //   
                        105, //   
                        105, //   
                        105, //   
                        105, //   
                        105, //   
                        105, //   
                        105, //   
                        105, //   
                        105, //   
                        105, //   
                        105, //   
                        105, //   
                        105, //   
                        105, //   
                        105, //   
                        105, //   
                        105, //   
                        105, //   
                        105, //   
                    }
                }, //          
                {
                    {
                        0  , // 
                        41 , // 
                        44 , // 
                        46 , // 
                        48 , // 
                        50 , // 
                        52 , // 
                        54 , // 
                        56 , // 
                        57 , // 
                        59 , //  
                        60 , //  
                        61 , //  
                        63 , //  
                        64 , //  
                        65 , //  
                        66 , //  
                        67 , //  
                        69 , //  
                        70 , //  
                        71 , //  
                        72 , //  
                        73 , //  
                        74 , //  
                        75 , //  
                        76 , //  
                        77 , //  
                        77 , //  
                        78 , //  
                        79 , //  
                        80 , //  
                        80 , //  
                        81 , //  
                        82 , //  
                        83 , //  
                        83 , //  
                        84 , //  
                        85 , //  
                        86 , //  
                        86 , //  
                        87 , //  
                        88 , //  
                        88 , //  
                        89 , //  
                        90 , //  
                        90 , //  
                        91 , //  
                        92 , //  
                        92 , //  
                        93 , //  
                        93 , //  
                        94 , //  
                        94 , //  
                        95 , //  
                        95 , //  
                        96 , //  
                        96 , //  
                        96 , //  
                        97 , //  
                        97 , //  
                        98 , //  
                        98 , //  
                        99 , //  
                        99 , //  
                        99 , //  
                        100, //  
                        100, //  
                        100, //  
                        101, //  
                        101, //  
                        101, //  
                        102, //  
                        102, //  
                        102, //  
                        103, //  
                        103, //  
                        103, //  
                        103, //  
                        103, //  
                        103, //  
                        103, //  
                        104, //  
                        104, //  
                        104, //  
                        104, //  
                        104, //  
                        104, //  
                        104, //  
                        104, //  
                        104, //  
                        104, //  
                        104, //  
                        104, //  
                        104, //  
                        105, //  
                        105, //  
                        105, //  
                        105, //  
                        105, //  
                        105, //  
                        105, //   
                        105, //   
                        105, //   
                        105, //   
                        105, //   
                        105, //   
                        105, //   
                        105, //   
                        105, //   
                        105, //   
                        105, //   
                        105, //   
                        105, //   
                        105, //   
                        105, //   
                        105, //   
                        105, //   
                        105, //   
                        105, //   
                        105, //   
                        105, //   
                        105, //   
                        105, //   
                        105, //   
                        105, //   
                        105, //   
                        105, //   
                        105, //   
                    }
                }, //          
                {
                    {
                        0  , // 
                        42 , // 
                        44 , // 
                        46 , // 
                        48 , // 
                        50 , // 
                        52 , // 
                        54 , // 
                        56 , // 
                        58 , // 
                        59 , //  
                        60 , //  
                        61 , //  
                        63 , //  
                        64 , //  
                        65 , //  
                        66 , //  
                        68 , //  
                        69 , //  
                        70 , //  
                        71 , //  
                        72 , //  
                        73 , //  
                        74 , //  
                        75 , //  
                        75 , //  
                        76 , //  
                        77 , //  
                        78 , //  
                        79 , //  
                        80 , //  
                        80 , //  
                        81 , //  
                        82 , //  
                        83 , //  
                        83 , //  
                        84 , //  
                        85 , //  
                        86 , //  
                        86 , //  
                        87 , //  
                        88 , //  
                        89 , //  
                        89 , //  
                        90 , //  
                        91 , //  
                        91 , //  
                        92 , //  
                        93 , //  
                        93 , //  
                        94 , //  
                        94 , //  
                        95 , //  
                        95 , //  
                        96 , //  
                        96 , //  
                        97 , //  
                        97 , //  
                        97 , //  
                        98 , //  
                        98 , //  
                        99 , //  
                        99 , //  
                        100, //  
                        100, //  
                        100, //  
                        101, //  
                        101, //  
                        101, //  
                        102, //  
                        102, //  
                        102, //  
                        103, //  
                        103, //  
                        103, //  
                        103, //  
                        103, //  
                        103, //  
                        104, //  
                        104, //  
                        104, //  
                        104, //  
                        104, //  
                        104, //  
                        104, //  
                        104, //  
                        104, //  
                        104, //  
                        104, //  
                        104, //  
                        105, //  
                        105, //  
                        105, //  
                        105, //  
                        105, //  
                        105, //  
                        105, //  
                        105, //  
                        105, //  
                        105, //  
                        105, //   
                        105, //   
                        105, //   
                        105, //   
                        105, //   
                        105, //   
                        105, //   
                        105, //   
                        105, //   
                        105, //   
                        105, //   
                        105, //   
                        105, //   
                        105, //   
                        105, //   
                        105, //   
                        105, //   
                        105, //   
                        105, //   
                        105, //   
                        105, //   
                        105, //   
                        105, //   
                        105, //   
                        105, //   
                        105, //   
                        105, //   
                        105, //   
                    }
                }, //          
                {
                    {
                       0,  // 
                       40,  // 
                       43,  // 
                       45,  // 
                       47,  // 
                       49,  // 
                       50,  // 
                       52,  // 
                       54,  // 
                       56,  // 
                       57,  //  
                       58,  //  
                       59,  //  
                       60,  //  
                       62,  //  
                       63,  //  
                       64,  //  
                       65,  //  
                       66,  //  
                       67,  //  
                       68,  //  
                       69,  //  
                       70,  //  
                       71,  //  
                       72,  //  
                       73,  //  
                       74,  //  
                       74,  //  
                       75,  //  
                       76,  //  
                       77,  //  
                       78,  //  
                       78,  //  
                       79,  //  
                       80,  //  
                       81,  //  
                       82,  //  
                       83,  //  
                       83,  //  
                       84,  //  
                       85,  //  
                       85,  //  
                       86,  //  
                       87,  //  
                       87,  //  
                       88,  //  
                       89,  //  
                       89,  //  
                       90,  //  
                       90,  //  
                       91,  //  
                       91,  //  
                       92,  //  
                       92,  //  
                       93,  //  
                       93,  //  
                       94,  //  
                       94,  //  
                       95,  //  
                       95,  //  
                       96,  //  
                       96,  //  
                       96,  //  
                       97,  //  
                       97,  //  
                       97,  //  
                       98,  //  
                       98,  //  
                       98,  //  
                       98,  //  
                       99,  //  
                       99,  //  
                       99,  //  
                       99,  //  
                       99,  //  
                       99,  //  
                       99,  //  
                       99,  //  
                       99,  //  
                       99,  //  
                       100,  //  
                       100,  //  
                       100,  //  
                       100,  //  
                       100,  //  
                       100,  //  
                       100,  //  
                       100,  //  
                       100,  //  
                       100,  //  
                       100,  //  
                       100,  //  
                       100,  //  
                       100,  //  
                       100,  //  
                       100,  //  
                       100,  //  
                       100,  //  
                       100, //  
                       100, //  
                       100, //   
                       100, //   
                       100, //   
                       100, //   
                       100, //   
                       100, //   
                       100, //   
                       100, //   
                       100, //   
                       100, //   
                       100, //   
                       100, //   
                       100, //   
                       100, //   
                       100, //   
                       100, //   
                       100, //   
                       100, //   
                       100, //   
                       100, //   
                       100, //   
                       100, //   
                       100, //   
                       100, //   
                       100, //   
                       100, //   
                       100, //   
                       100, //   
                    }
                }, //          
        },

        //                          
        {
            98,  //          
            101,  //          
            101,  //          
            100,  //          
            98,  //          
            97,  //          
            94,  //          
            94,  //          
            92,  //          
            90,  //           
            94,  //           
            95,  //           
            97,  //           
            104   //           
        },
#if 0
        //                   
        {
            0x7FFF,      //                             
            0x00,        //                           
            0x00,        //                       

            0x00,        //                          
            0x00,        //                          
            0x00,        //                     
            0x00,        //                        

            0x00,        //                    
            0x00,        //                    
            { 0x00, 0x00 },        //                         

            0x00,        //                    
            0x00,        //                    
            0x00,        //                    
            0x00,        //                    

            0x7FFF,      //                        
            0x00,        //                       
            0x00,        //                        

            0x00,        //                             
            0x00,        //                         
            0x00,        //                          
            0x00,        //                          

            0x00,        //                         
            0x00,        //                          
            0x00,        //                          
            0x00,        //                             

            0x00,        //                           
            0x00,        //                                              
            0x00,        //                                 
            0x00,        //                                

            0x00,        //                           
            0x00,        //                           
            0x00,        //                           
            0x00,        //                           

            0x00,        //                             
            0x00,        //                             
            0x00,        //                             
            0x00,        //                             

            0x7FFF,        //                       
            0x7FFF,        //                        

            { //               
                {
                    { 0x00, 0x00 }, //               
                    { 0x00, 0x00 }, //               
                    { 0x00, 0x00 }, //               
                    { 0x00, 0x00 }, //               
                    { 0x00, 0x00 }, //               
                    { 0x00, 0x00 }, //               
                    { 0x00, 0x00 }, //               
                    { 0x00, 0x00 }, //               
                    { 0x00, 0x00 }, //               
                    { 0x00, 0x00 }, //               
                    { 0x00, 0x00 }, //                
                    { 0x00, 0x00 }, //                
                    { 0x00, 0x00 }, //                
                    { 0x00, 0x00 }, //                
                    { 0x00, 0x00 }, //                
                    { 0x00, 0x00 }  //                
                }, //         
                {
                    { 0x00, 0x00 }, //               
                    { 0x00, 0x00 }, //               
                    { 0x00, 0x00 }, //               
                    { 0x00, 0x00 }, //               
                    { 0x00, 0x00 }, //               
                    { 0x00, 0x00 }, //               
                    { 0x00, 0x00 }, //               
                    { 0x00, 0x00 }, //               
                    { 0x00, 0x00 }, //               
                    { 0x00, 0x00 }, //               
                    { 0x00, 0x00 }, //                
                    { 0x00, 0x00 }, //                
                    { 0x00, 0x00 }, //                
                    { 0x00, 0x00 }, //                
                    { 0x00, 0x00 }, //                
                    { 0x00, 0x00 }  //                
                }, //         
                {
                    { 0x00, 0x00 }, //               
                    { 0x00, 0x00 }, //               
                    { 0x00, 0x00 }, //               
                    { 0x00, 0x00 }, //               
                    { 0x00, 0x00 }, //               
                    { 0x00, 0x00 }, //               
                    { 0x00, 0x00 }, //               
                    { 0x00, 0x00 }, //               
                    { 0x00, 0x00 }, //               
                    { 0x00, 0x00 }, //               
                    { 0x00, 0x00 }, //                
                    { 0x00, 0x00 }, //                
                    { 0x00, 0x00 }, //                
                    { 0x00, 0x00 }, //                
                    { 0x00, 0x00 }, //                
                    { 0x00, 0x00 }  //                
                }, //         
                {
                    { 0x00, 0x00 }, //               
                    { 0x00, 0x00 }, //               
                    { 0x00, 0x00 }, //               
                    { 0x00, 0x00 }, //               
                    { 0x00, 0x00 }, //               
                    { 0x00, 0x00 }, //               
                    { 0x00, 0x00 }, //               
                    { 0x00, 0x00 }, //               
                    { 0x00, 0x00 }, //               
                    { 0x00, 0x00 }, //               
                    { 0x00, 0x00 }, //                
                    { 0x00, 0x00 }, //                
                    { 0x00, 0x00 }, //                
                    { 0x00, 0x00 }, //                
                    { 0x00, 0x00 }, //                
                    { 0x00, 0x00 }  //                
                }, //         
                {
                    { 0x00, 0x00 }, //               
                    { 0x00, 0x00 }, //               
                    { 0x00, 0x00 }, //               
                    { 0x00, 0x00 }, //               
                    { 0x00, 0x00 }, //               
                    { 0x00, 0x00 }, //               
                    { 0x00, 0x00 }, //               
                    { 0x00, 0x00 }, //               
                    { 0x00, 0x00 }, //               
                    { 0x00, 0x00 }, //               
                    { 0x00, 0x00 }, //                
                    { 0x00, 0x00 }, //                
                    { 0x00, 0x00 }, //                
                    { 0x00, 0x00 }, //                
                    { 0x00, 0x00 }, //                
                    { 0x00, 0x00 }  //                
                }, //         
                {
                    { 0x00, 0x00 }, //               
                    { 0x00, 0x00 }, //               
                    { 0x00, 0x00 }, //               
                    { 0x00, 0x00 }, //               
                    { 0x00, 0x00 }, //               
                    { 0x00, 0x00 }, //               
                    { 0x00, 0x00 }, //               
                    { 0x00, 0x00 }, //               
                    { 0x00, 0x00 }, //               
                    { 0x00, 0x00 }, //               
                    { 0x00, 0x00 }, //                
                    { 0x00, 0x00 }, //                
                    { 0x00, 0x00 }, //                
                    { 0x00, 0x00 }, //                
                    { 0x00, 0x00 }, //                
                    { 0x00, 0x00 }  //                
                }, //         
                {
                    { 0x00, 0x00 }, //               
                    { 0x00, 0x00 }, //               
                    { 0x00, 0x00 }, //               
                    { 0x00, 0x00 }, //               
                    { 0x00, 0x00 }, //               
                    { 0x00, 0x00 }, //               
                    { 0x00, 0x00 }, //               
                    { 0x00, 0x00 }, //               
                    { 0x00, 0x00 }, //               
                    { 0x00, 0x00 }, //               
                    { 0x00, 0x00 }, //                
                    { 0x00, 0x00 }, //                
                    { 0x00, 0x00 }, //                
                    { 0x00, 0x00 }, //                
                    { 0x00, 0x00 }, //                
                    { 0x00, 0x00 }  //                
                }, //         
                {
                    { 0x00, 0x00 }, //               
                    { 0x00, 0x00 }, //               
                    { 0x00, 0x00 }, //               
                    { 0x00, 0x00 }, //               
                    { 0x00, 0x00 }, //               
                    { 0x00, 0x00 }, //               
                    { 0x00, 0x00 }, //               
                    { 0x00, 0x00 }, //               
                    { 0x00, 0x00 }, //               
                    { 0x00, 0x00 }, //               
                    { 0x00, 0x00 }, //                
                    { 0x00, 0x00 }, //                
                    { 0x00, 0x00 }, //                
                    { 0x00, 0x00 }, //                
                    { 0x00, 0x00 }, //                
                    { 0x00, 0x00 }  //                
                }, //         
                {
                    { 0x00, 0x00 }, //               
                    { 0x00, 0x00 }, //               
                    { 0x00, 0x00 }, //               
                    { 0x00, 0x00 }, //               
                    { 0x00, 0x00 }, //               
                    { 0x00, 0x00 }, //               
                    { 0x00, 0x00 }, //               
                    { 0x00, 0x00 }, //               
                    { 0x00, 0x00 }, //               
                    { 0x00, 0x00 }, //               
                    { 0x00, 0x00 }, //                
                    { 0x00, 0x00 }, //                
                    { 0x00, 0x00 }, //                
                    { 0x00, 0x00 }, //                
                    { 0x00, 0x00 }, //                
                    { 0x00, 0x00 }  //                
                }, //         
                {
                    { 0x00, 0x00 }, //               
                    { 0x00, 0x00 }, //               
                    { 0x00, 0x00 }, //               
                    { 0x00, 0x00 }, //               
                    { 0x00, 0x00 }, //               
                    { 0x00, 0x00 }, //               
                    { 0x00, 0x00 }, //               
                    { 0x00, 0x00 }, //               
                    { 0x00, 0x00 }, //               
                    { 0x00, 0x00 }, //               
                    { 0x00, 0x00 }, //                
                    { 0x00, 0x00 }, //                
                    { 0x00, 0x00 }, //                
                    { 0x00, 0x00 }, //                
                    { 0x00, 0x00 }, //                
                    { 0x00, 0x00 }  //                
                }, //          
                {
                    { 0x00, 0x00 }, //               
                    { 0x00, 0x00 }, //               
                    { 0x00, 0x00 }, //               
                    { 0x00, 0x00 }, //               
                    { 0x00, 0x00 }, //               
                    { 0x00, 0x00 }, //               
                    { 0x00, 0x00 }, //               
                    { 0x00, 0x00 }, //               
                    { 0x00, 0x00 }, //               
                    { 0x00, 0x00 }, //               
                    { 0x00, 0x00 }, //                
                    { 0x00, 0x00 }, //                
                    { 0x00, 0x00 }, //                
                    { 0x00, 0x00 }, //                
                    { 0x00, 0x00 }, //                
                    { 0x00, 0x00 }  //                
                }, //          
                {
                    { 0x00, 0x00 }, //               
                    { 0x00, 0x00 }, //               
                    { 0x00, 0x00 }, //               
                    { 0x00, 0x00 }, //               
                    { 0x00, 0x00 }, //               
                    { 0x00, 0x00 }, //               
                    { 0x00, 0x00 }, //               
                    { 0x00, 0x00 }, //               
                    { 0x00, 0x00 }, //               
                    { 0x00, 0x00 }, //               
                    { 0x00, 0x00 }, //                
                    { 0x00, 0x00 }, //                
                    { 0x00, 0x00 }, //                
                    { 0x00, 0x00 }, //                
                    { 0x00, 0x00 }, //                
                    { 0x00, 0x00 }  //                
                }, //          
                {
                    { 0x00, 0x00 }, //               
                    { 0x00, 0x00 }, //               
                    { 0x00, 0x00 }, //               
                    { 0x00, 0x00 }, //               
                    { 0x00, 0x00 }, //               
                    { 0x00, 0x00 }, //               
                    { 0x00, 0x00 }, //               
                    { 0x00, 0x00 }, //               
                    { 0x00, 0x00 }, //               
                    { 0x00, 0x00 }, //               
                    { 0x00, 0x00 }, //                
                    { 0x00, 0x00 }, //                
                    { 0x00, 0x00 }, //                
                    { 0x00, 0x00 }, //                
                    { 0x00, 0x00 }, //                
                    { 0x00, 0x00 }  //                
                }, //          
                {
                    { 0x00, 0x00 }, //               
                    { 0x00, 0x00 }, //               
                    { 0x00, 0x00 }, //               
                    { 0x00, 0x00 }, //               
                    { 0x00, 0x00 }, //               
                    { 0x00, 0x00 }, //               
                    { 0x00, 0x00 }, //               
                    { 0x00, 0x00 }, //               
                    { 0x00, 0x00 }, //               
                    { 0x00, 0x00 }, //               
                    { 0x00, 0x00 }, //                
                    { 0x00, 0x00 }, //                
                    { 0x00, 0x00 }, //                
                    { 0x00, 0x00 }, //                
                    { 0x00, 0x00 }, //                
                    { 0x00, 0x00 }  //                
                }  //          
            },        //                                

            { //            
                {
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //                
                    { 0x0000, 0x0000, 0x0000 }, //                
                    { 0x0000, 0x0000, 0x0000 }, //                
                    { 0x0000, 0x0000, 0x0000 }, //                
                    { 0x0000, 0x0000, 0x0000 }, //                
                    { 0x0000, 0x0000, 0x0000 }  //                
                }, //         
                {
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //                
                    { 0x0000, 0x0000, 0x0000 }, //                
                    { 0x0000, 0x0000, 0x0000 }, //                
                    { 0x0000, 0x0000, 0x0000 }, //                
                    { 0x0000, 0x0000, 0x0000 }, //                
                    { 0x0000, 0x0000, 0x0000 }  //                
                }, //         
                {
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //                
                    { 0x0000, 0x0000, 0x0000 }, //                
                    { 0x0000, 0x0000, 0x0000 }, //                
                    { 0x0000, 0x0000, 0x0000 }, //                
                    { 0x0000, 0x0000, 0x0000 }, //                
                    { 0x0000, 0x0000, 0x0000 }  //                
                }, //         
                {
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //                
                    { 0x0000, 0x0000, 0x0000 }, //                
                    { 0x0000, 0x0000, 0x0000 }, //                
                    { 0x0000, 0x0000, 0x0000 }, //                
                    { 0x0000, 0x0000, 0x0000 }, //                
                    { 0x0000, 0x0000, 0x0000 }  //                
                }, //         
                {
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //                
                    { 0x0000, 0x0000, 0x0000 }, //                
                    { 0x0000, 0x0000, 0x0000 }, //                
                    { 0x0000, 0x0000, 0x0000 }, //                
                    { 0x0000, 0x0000, 0x0000 }, //                
                    { 0x0000, 0x0000, 0x0000 }  //                
                }, //         
                {
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //                
                    { 0x0000, 0x0000, 0x0000 }, //                
                    { 0x0000, 0x0000, 0x0000 }, //                
                    { 0x0000, 0x0000, 0x0000 }, //                
                    { 0x0000, 0x0000, 0x0000 }, //                
                    { 0x0000, 0x0000, 0x0000 }  //                
                }, //         
                {
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //                
                    { 0x0000, 0x0000, 0x0000 }, //                
                    { 0x0000, 0x0000, 0x0000 }, //                
                    { 0x0000, 0x0000, 0x0000 }, //                
                    { 0x0000, 0x0000, 0x0000 }, //                
                    { 0x0000, 0x0000, 0x0000 }  //                
                }, //         
                {
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //                
                    { 0x0000, 0x0000, 0x0000 }, //                
                    { 0x0000, 0x0000, 0x0000 }, //                
                    { 0x0000, 0x0000, 0x0000 }, //                
                    { 0x0000, 0x0000, 0x0000 }, //                
                    { 0x0000, 0x0000, 0x0000 }  //                
                }, //         
                {
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //                
                    { 0x0000, 0x0000, 0x0000 }, //                
                    { 0x0000, 0x0000, 0x0000 }, //                
                    { 0x0000, 0x0000, 0x0000 }, //                
                    { 0x0000, 0x0000, 0x0000 }, //                
                    { 0x0000, 0x0000, 0x0000 }  //                
                }, //         
                {
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //                
                    { 0x0000, 0x0000, 0x0000 }, //                
                    { 0x0000, 0x0000, 0x0000 }, //                
                    { 0x0000, 0x0000, 0x0000 }, //                
                    { 0x0000, 0x0000, 0x0000 }, //                
                    { 0x0000, 0x0000, 0x0000 }  //                
                }, //          
                {
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //                
                    { 0x0000, 0x0000, 0x0000 }, //                
                    { 0x0000, 0x0000, 0x0000 }, //                
                    { 0x0000, 0x0000, 0x0000 }, //                
                    { 0x0000, 0x0000, 0x0000 }, //                
                    { 0x0000, 0x0000, 0x0000 }  //                
                }, //          
                {
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //                
                    { 0x0000, 0x0000, 0x0000 }, //                
                    { 0x0000, 0x0000, 0x0000 }, //                
                    { 0x0000, 0x0000, 0x0000 }, //                
                    { 0x0000, 0x0000, 0x0000 }, //                
                    { 0x0000, 0x0000, 0x0000 }  //                
                }, //          
                {
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //                
                    { 0x0000, 0x0000, 0x0000 }, //                
                    { 0x0000, 0x0000, 0x0000 }, //                
                    { 0x0000, 0x0000, 0x0000 }, //                
                    { 0x0000, 0x0000, 0x0000 }, //                
                    { 0x0000, 0x0000, 0x0000 }  //                
                }, //          
                {
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //                
                    { 0x0000, 0x0000, 0x0000 }, //                
                    { 0x0000, 0x0000, 0x0000 }, //                
                    { 0x0000, 0x0000, 0x0000 }, //                
                    { 0x0000, 0x0000, 0x0000 }, //                
                    { 0x0000, 0x0000, 0x0000 }  //                
                }  //          
            },        //                                

            { //            
                {
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //                
                    { 0x0000, 0x0000, 0x0000 }, //                
                    { 0x0000, 0x0000, 0x0000 }, //                
                    { 0x0000, 0x0000, 0x0000 }, //                
                    { 0x0000, 0x0000, 0x0000 }, //                
                    { 0x0000, 0x0000, 0x0000 }  //                
                }, //         
                {
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //                
                    { 0x0000, 0x0000, 0x0000 }, //                
                    { 0x0000, 0x0000, 0x0000 }, //                
                    { 0x0000, 0x0000, 0x0000 }, //                
                    { 0x0000, 0x0000, 0x0000 }, //                
                    { 0x0000, 0x0000, 0x0000 }  //                
                }, //         
                {
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //                
                    { 0x0000, 0x0000, 0x0000 }, //                
                    { 0x0000, 0x0000, 0x0000 }, //                
                    { 0x0000, 0x0000, 0x0000 }, //                
                    { 0x0000, 0x0000, 0x0000 }, //                
                    { 0x0000, 0x0000, 0x0000 }  //                
                }, //         
                {
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //                
                    { 0x0000, 0x0000, 0x0000 }, //                
                    { 0x0000, 0x0000, 0x0000 }, //                
                    { 0x0000, 0x0000, 0x0000 }, //                
                    { 0x0000, 0x0000, 0x0000 }, //                
                    { 0x0000, 0x0000, 0x0000 }  //                
                }, //         
                {
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //                
                    { 0x0000, 0x0000, 0x0000 }, //                
                    { 0x0000, 0x0000, 0x0000 }, //                
                    { 0x0000, 0x0000, 0x0000 }, //                
                    { 0x0000, 0x0000, 0x0000 }, //                
                    { 0x0000, 0x0000, 0x0000 }  //                
                }, //         
                {
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //                
                    { 0x0000, 0x0000, 0x0000 }, //                
                    { 0x0000, 0x0000, 0x0000 }, //                
                    { 0x0000, 0x0000, 0x0000 }, //                
                    { 0x0000, 0x0000, 0x0000 }, //                
                    { 0x0000, 0x0000, 0x0000 }  //                
                }, //         
                {
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //                
                    { 0x0000, 0x0000, 0x0000 }, //                
                    { 0x0000, 0x0000, 0x0000 }, //                
                    { 0x0000, 0x0000, 0x0000 }, //                
                    { 0x0000, 0x0000, 0x0000 }, //                
                    { 0x0000, 0x0000, 0x0000 }  //                
                }, //         
                {
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //                
                    { 0x0000, 0x0000, 0x0000 }, //                
                    { 0x0000, 0x0000, 0x0000 }, //                
                    { 0x0000, 0x0000, 0x0000 }, //                
                    { 0x0000, 0x0000, 0x0000 }, //                
                    { 0x0000, 0x0000, 0x0000 }  //                
                }, //         
                {
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //                
                    { 0x0000, 0x0000, 0x0000 }, //                
                    { 0x0000, 0x0000, 0x0000 }, //                
                    { 0x0000, 0x0000, 0x0000 }, //                
                    { 0x0000, 0x0000, 0x0000 }, //                
                    { 0x0000, 0x0000, 0x0000 }  //                
                }, //         
                {
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //                
                    { 0x0000, 0x0000, 0x0000 }, //                
                    { 0x0000, 0x0000, 0x0000 }, //                
                    { 0x0000, 0x0000, 0x0000 }, //                
                    { 0x0000, 0x0000, 0x0000 }, //                
                    { 0x0000, 0x0000, 0x0000 }  //                
                }, //          
                {
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //                
                    { 0x0000, 0x0000, 0x0000 }, //                
                    { 0x0000, 0x0000, 0x0000 }, //                
                    { 0x0000, 0x0000, 0x0000 }, //                
                    { 0x0000, 0x0000, 0x0000 }, //                
                    { 0x0000, 0x0000, 0x0000 }  //                
                }, //          
                {
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //                
                    { 0x0000, 0x0000, 0x0000 }, //                
                    { 0x0000, 0x0000, 0x0000 }, //                
                    { 0x0000, 0x0000, 0x0000 }, //                
                    { 0x0000, 0x0000, 0x0000 }, //                
                    { 0x0000, 0x0000, 0x0000 }  //                
                }, //          
                {
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //                
                    { 0x0000, 0x0000, 0x0000 }, //                
                    { 0x0000, 0x0000, 0x0000 }, //                
                    { 0x0000, 0x0000, 0x0000 }, //                
                    { 0x0000, 0x0000, 0x0000 }, //                
                    { 0x0000, 0x0000, 0x0000 }  //                
                }, //          
                {
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //                
                    { 0x0000, 0x0000, 0x0000 }, //                
                    { 0x0000, 0x0000, 0x0000 }, //                
                    { 0x0000, 0x0000, 0x0000 }, //                
                    { 0x0000, 0x0000, 0x0000 }, //                
                    { 0x0000, 0x0000, 0x0000 }  //                
                }  //          
            },        //                                

            { //                                               
                {
                    {
                        {
                            { 0x00, 0x00 }, //           
                            { 0x00, 0x00 }, //           
                            { 0x00, 0x00 }, //           
                            { 0x00, 0x00 }, //           
                            { 0x00, 0x00 }, //           
                            { 0x00, 0x00 }, //           
                            { 0x00, 0x00 }, //           
                            { 0x00, 0x00 }  //           
                        } //               
                    } //          
                }, //          
                {
                    {
                        {
                            { 0x00, 0x00 }, //           
                            { 0x00, 0x00 }, //           
                            { 0x00, 0x00 }, //           
                            { 0x00, 0x00 }, //           
                            { 0x00, 0x00 }, //           
                            { 0x00, 0x00 }, //           
                            { 0x00, 0x00 }, //           
                            { 0x00, 0x00 }  //           
                        } //               
                    } //          
                }, //          
                {
                    {
                        {
                            { 0x00, 0x00 }, //           
                            { 0x00, 0x00 }, //           
                            { 0x00, 0x00 }, //           
                            { 0x00, 0x00 }, //           
                            { 0x00, 0x00 }, //           
                            { 0x00, 0x00 }, //           
                            { 0x00, 0x00 }, //           
                            { 0x00, 0x00 }  //           
                        } //               
                    } //          
                }, //          
                {
                    {
                        {
                            { 0x00, 0x00 }, //           
                            { 0x00, 0x00 }, //           
                            { 0x00, 0x00 }, //           
                            { 0x00, 0x00 }, //           
                            { 0x00, 0x00 }, //           
                            { 0x00, 0x00 }, //           
                            { 0x00, 0x00 }, //           
                            { 0x00, 0x00 }  //           
                        } //               
                    } //          
                }, //          
                {
                    {
                        {
                            { 0x00, 0x00 }, //           
                            { 0x00, 0x00 }, //           
                            { 0x00, 0x00 }, //           
                            { 0x00, 0x00 }, //           
                            { 0x00, 0x00 }, //           
                            { 0x00, 0x00 }, //           
                            { 0x00, 0x00 }, //           
                            { 0x00, 0x00 }  //           
                        } //               
                    } //          
                }, //          
                {
                    {
                        {
                            { 0x00, 0x00 }, //           
                            { 0x00, 0x00 }, //           
                            { 0x00, 0x00 }, //           
                            { 0x00, 0x00 }, //           
                            { 0x00, 0x00 }, //           
                            { 0x00, 0x00 }, //           
                            { 0x00, 0x00 }, //           
                            { 0x00, 0x00 }  //           
                        } //               
                    } //          
                }, //          
                {
                    {
                        {
                            { 0x00, 0x00 }, //           
                            { 0x00, 0x00 }, //           
                            { 0x00, 0x00 }, //           
                            { 0x00, 0x00 }, //           
                            { 0x00, 0x00 }, //           
                            { 0x00, 0x00 }, //           
                            { 0x00, 0x00 }, //           
                            { 0x00, 0x00 }  //           
                        } //               
                    } //          
                }, //          
                {
                    {
                        {
                            { 0x00, 0x00 }, //           
                            { 0x00, 0x00 }, //           
                            { 0x00, 0x00 }, //           
                            { 0x00, 0x00 }, //           
                            { 0x00, 0x00 }, //           
                            { 0x00, 0x00 }, //           
                            { 0x00, 0x00 }, //           
                            { 0x00, 0x00 }  //           
                        } //               
                    } //          
                }, //          
                {
                    {
                        {
                            { 0x00, 0x00 }, //           
                            { 0x00, 0x00 }, //           
                            { 0x00, 0x00 }, //           
                            { 0x00, 0x00 }, //           
                            { 0x00, 0x00 }, //           
                            { 0x00, 0x00 }, //           
                            { 0x00, 0x00 }, //           
                            { 0x00, 0x00 }  //           
                        } //               
                    } //          
                }, //          
                {
                    {
                        {
                            { 0x00, 0x00 }, //           
                            { 0x00, 0x00 }, //           
                            { 0x00, 0x00 }, //           
                            { 0x00, 0x00 }, //           
                            { 0x00, 0x00 }, //           
                            { 0x00, 0x00 }, //           
                            { 0x00, 0x00 }, //           
                            { 0x00, 0x00 }  //           
                        } //               
                    } //          
                }, //           
                {
                    {
                        {
                            { 0x00, 0x00 }, //           
                            { 0x00, 0x00 }, //           
                            { 0x00, 0x00 }, //           
                            { 0x00, 0x00 }, //           
                            { 0x00, 0x00 }, //           
                            { 0x00, 0x00 }, //           
                            { 0x00, 0x00 }, //           
                            { 0x00, 0x00 }  //           
                        } //               
                    } //          
                }, //           
                {
                    {
                        {
                            { 0x00, 0x00 }, //           
                            { 0x00, 0x00 }, //           
                            { 0x00, 0x00 }, //           
                            { 0x00, 0x00 }, //           
                            { 0x00, 0x00 }, //           
                            { 0x00, 0x00 }, //           
                            { 0x00, 0x00 }, //           
                            { 0x00, 0x00 }  //           
                        } //               
                    } //          
                }, //           
                {
                    {
                        {
                            { 0x00, 0x00 }, //           
                            { 0x00, 0x00 }, //           
                            { 0x00, 0x00 }, //           
                            { 0x00, 0x00 }, //           
                            { 0x00, 0x00 }, //           
                            { 0x00, 0x00 }, //           
                            { 0x00, 0x00 }, //           
                            { 0x00, 0x00 }  //           
                        } //               
                    } //          
                },  //           
                {
                    {
                        {
                            { 0x00, 0x00 }, //           
                            { 0x00, 0x00 }, //           
                            { 0x00, 0x00 }, //           
                            { 0x00, 0x00 }, //           
                            { 0x00, 0x00 }, //           
                            { 0x00, 0x00 }, //           
                            { 0x00, 0x00 }, //           
                            { 0x00, 0x00 }  //           
                        } //               
                    } //          
                }  //           
            },        //                                                

            {
                { 0x0000, { 0x00, 0x00 } }, //                                     
                { 0x0000, { 0x00, 0x00 } }, //                                     
                { 0x0000, { 0x00, 0x00 } }, //                                     
                { 0x0000, { 0x00, 0x00 } }, //                                     
                { 0x0000, { 0x00, 0x00 } }, //                                     
                { 0x0000, { 0x00, 0x00 } }, //                                     
                { 0x0000, { 0x00, 0x00 } }, //                                     
                { 0x0000, { 0x00, 0x00 } }, //                                     
                { 0x0000, { 0x00, 0x00 } }, //                                     
                { 0x0000, { 0x00, 0x00 } }, //                                      
                { 0x0000, { 0x00, 0x00 } }, //                                      
                { 0x0000, { 0x00, 0x00 } }, //                                      
                { 0x0000, { 0x00, 0x00 } }, //                                      
                { 0x0000, { 0x00, 0x00 } }  //                                      
            }        //                                                  

        }, //                   

        //                   
        {
            0xFF,        //                     
            0xFF,        //                    
            0xFF,        //                      
            0xFF,        //                    
            0xFF,        //                    
            0xFF,        //                    
            0xFF,        //                             
            0xFF,        //                             
            0xFF,        //                  
            0xFF,        //                  
            0xFF,        //                   
            0xFF,        //                  
            0xFF,        //                     
            0xFF,        //                           
            0xFF,        //                           
            0xFF,        //                              
            0xFF,        //                         
            0xFF,        //                  
            0xFF,        //                              
            0xFF,        //                   
            0xFF,        //                         
            0xFF,        //                                
            0xFF,        //                             
            {
                0x00,
                0x00,
                0x00,
                0x00,
                0x00,
                0x00,
                0x00,
                0x00,
                0x00
            }        //                       
        },
#endif
        //                             
        {
            //              
            {
                //           
                {300, 300, 300, 300, 300, 300, 300, 300, 300, 300, 300, 300, 300, 300},

                //            
                {300, 300, 300, 300, 300, 300, 300, 300, 300, 300, 300, 300, 300, 300}
            },
            //    
            {
                //           
                {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},

                //            
                {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
            }
        },

        //                      
        {
            //              
            // 
            //                                       
            //                     
            //              

            0,               //         
            {
                0,           //                
                0,           //              
                0,           //          

                0,           //             
                0,           //             
                0,           //        
                0,           //           

                0,           //       
                0,           //       
                0,           //          
                0,           //           

                0,           //       
                0,           //       
                0,           //       
                0,           //       

                0,           //           
                0,           //          
                0,           //           

                0,           //                
                0,           //            
                0,           //             
                0,           //             

                0,           //            
                0,           //             
                0,           //             
                0,           //                

                0,           //              
                0,           //                                 
                0,           //                    
                0,           //                   

                0,           //              
                0,           //              
                0,           //        

                0,           //                
                0,           //                
                0,           //                
                0,           //                

                0,           //          
                0,           //         

                0,           //               
                0,           //                    
                0            //          
            }
        },

        //                          
        {
            280,  //          
            270,  //          
            270,  //          
            270,  //          
            270,  //          
            270,  //          
            280,  //          
            280,  //          
            290,  //          
            300,  //           
            300,  //           
            310,  //           
            310,  //           
            310   //           
        },

        //                                 
        {
            {
                2150,  //          
                2150,  //          
                2150,  //          
                2150,  //          
                2150,  //          
                2150,  //          
                2150,  //          
                2150,  //          
                2150,  //          
                2150,  //           
                2150,  //           
                2150,  //           
                2150,  //           
                2150   //           
            },//            

            {
                1850,  //          
                1950,  //          
                1950,  //          
                1950,  //          
                1950,  //          
                1950,  //          
                1950,  //          
                1950,  //          
                1950,  //          
                1950,  //           
                1750,  //           
                1950,  //           
                1950,  //           
                1950   //           
            },//             

            {
                1750,  //          
                1750,  //          
                1750,  //          
                1750,  //          
                1750,  //          
                1750,  //          
                1750,  //          
                1750,  //          
                1750,  //          
                1750,  //           
                1750,  //           
                1750,  //           
                1750,  //           
                1750   //           
            },//             
        },

        //                            
        {
            0, 0
        },

        //                          
        {
            0
        }

    } //       
};

#else

#include "palTypes.h"
#include "wlan_nv.h"
//                         

const sHalNv nvDefaults =
{
    {
        0,                                                              //                     
        1,                                                              //                        
        2,                                                              //                                                            
        1,                                                              //                         
        1,                                                              //                         
        { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },                         //                                           
        { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },                         //                                           
        { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },                         //                                           
        { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },                         //                                           
        { "\0" },
        0,                                                              //                                                     
        0                                                               //                                
    }, //      

    {
        //                             
        {
            //                           
            //                                                   
            //                                                
            {
                    //             
                {1900},    //                              
                {1900},    //                              
                {1900},    //                                
                {1900},    //                               
                {1900},    //                               
                {1900},    //                                 
                {1900},    //                                

                //               
                {1700},    //                         
                {1700},    //                         
                {1700},    //                          
                {1650},    //                          
                {1600},    //                          
                {1550},    //                          
                {1550},    //                          
                {1550},    //                          

                //                   
                {1700},    //                             
                {1700},    //                             
                {1700},    //                              
                {1650},    //                              
                {1600},    //                              
                {1550},    //                              
                {1550},    //                              
                {1500},    //                              

                //                        
                {1700},    //                                
                {1700},    //                               
                {1650},    //                                 
                {1600},    //                               
                {1550},    //                               
                {1550},    //                               
                {1400},    //                                 
                {1300},    //                               
                {1700},    //                                      
                {1700},    //                                       
                {1650},    //                                       
                {1600},    //                                       
                {1550},    //                                       
                {1500},    //                                       
                {1400},    //                                     
                {1300},     //                                       

                //                         
                {1700},    //                                    
                {1700},    //                                  
                {1650},    //                                    
                {1600},    //                                  
                {1550},    //                                  
                {1500},    //                                   
                {1400},    //                                     
                {1300},    //                                   
                {1700},    //                                        
                {1700},    //                                        
                {1650},    //                                        
                {1600},    //                                        
                {1550},    //                                        
                {1500},    //                                         
                {1400},    //                                         
                {1350},    //                                         
#ifdef WLAN_FEATURE_11AC
                //          
               //                         
                {1700},    //                              
                {1700},    //                              
                {1700},    //                               
                {1650},    //                               
                {1600},    //                               
                {1550},    //                               
                {1550},    //                               
                {1500},    //                               

               //                 
                {0000},    //                                                 
                {0000},    //                                                
                {0000},    //                                                  
                {0000},    //                                                
                {0000},    //                                                
                {0000},    //                                                
                {0000},    //                                                  
                {0000},    //                                                
                {0000},    //                                                
                {0000},    //                                                 
                {0000},    //                                                  
                {0000},    //                                                  
                {0000},    //                                                  
                {0000},    //                                                  
                {0000},    //                                                  
                {0000},    //                                                
                {0000},    //                                                  
                {0000},    //                                                  


               //                 
                {0000},    //                                                  
                {0000},    //                                                
                {0000},    //                                                  
                {0000},    //                                                
                {0000},    //                                                
                {0000},    //                                                 
                {0000},    //                                                   
                {0000},    //                                                 
                {0000},    //                                                 
                {0000},    //                                                 
                {0000},    //                                                
                {0000},    //                                                
                {0000},    //                                                
                {0000},    //                                                
                {0000},    //                                                
                {0000},    //                                                 
                {0000},    //                                                 
                {0000},    //                                                 
                {0000},    //                                                 
                {0000},    //                                                 


               //                 
                {0000},    //                                                  
                {0000},    //                                                  
                {0000},    //                                                  
                {0000},    //                                                 
                {0000},    //                                                   
                {0000},    //                                                 
                {0000},    //                                                   
                {0000},    //                                                   
                {0000},    //                                                 
                {0000},    //                                                 
                {0000},    //                                                  
                {0000},    //                                                
                {0000},    //                                                  
                {0000},    //                                                 
                {0000},    //                                                 
                {0000},    //                                                 
                {0000},    //                                                   
                {0000},    //                                                 
                {0000},    //                                                 
                {0000},    //                                                   
#endif //                 
                },  //                      

                {
                //             
                {0},    //                              
                {0},    //                              
                {0},    //                                
                {0},    //                               
                {0},    //                               
                {0},    //                                 
                {0},    //                                

                //               
                {1600},    //                         
                {1600},    //                         
                {1600},    //                          
                {1550},    //                          
                {1550},    //                          
                {1450},    //                          
                {1400},    //                          
                {1400},    //                          

                //                   
                {1600},    //                             
                {1600},    //                             
                {1600},    //                              
                {1650},    //                              
                {1550},    //                              
                {1450},    //                              
                {1400},    //                              
                {1400},    //                              

                //                        
                {1600},    //                                
                {1600},    //                               
                {1550},    //                                 
                {1500},    //                               
                {1450},    //                               
                {1400},    //                               
                {1350},    //                                 
                {1200},    //                               
                {1600},    //                                      
                {1600},    //                                       
                {1550},    //                                       
                {1550},    //                                       
                {1450},    //                                       
                {1400},    //                                       
                {1300},    //                                     
                {1200},    //                                       

                //                         
                {1600},    //                                    
                {1600},    //                                  
                {1550},    //                                    
                {1500},    //                                  
                {1450},    //                                  
                {1400},    //                                   
                {1300},    //                                     
                {1200},    //                                   
                {1600},    //                                        
                {1600},    //                                        
                {1500},    //                                        
                {1500},    //                                        
                {1450},    //                                        
                {1400},    //                                         
                {1300},    //                                         
                {1200},    //                                         
#ifdef WLAN_FEATURE_11AC
                //          
               //                         
                {1700},    //                              
                {1700},    //                              
                {1700},    //                               
                {1650},    //                               
                {1600},    //                               
                {1550},    //                               
                {1550},    //                               
                {1500},    //                               

               //                 
                {1400},    //                                                 
                {1400},    //                                                
                {1350},    //                                                  
                {1300},    //                                                
                {1250},    //                                                
                {1250},    //                                                
                {1100},    //                                                  
                {1000},    //                                                
                {1000},    //                                                
                {1400},    //                                                 
                {1400},    //                                                  
                {1350},    //                                                  
                {1300},    //                                                  
                {1250},    //                                                  
                {1200},    //                                                  
                {1100},    //                                                
                {1000},    //                                                  
                {1000},    //                                                  


               //                 
                {1400},    //                                                  
                {1300},    //                                                
                {1250},    //                                                  
                {1250},    //                                                
                {1000},    //                                                
                {1000},    //                                                 
                {1000},    //                                                   
                {1000},    //                                                 
                {1000},    //                                                 
                {1000},    //                                                 
                {1400},    //                                                
                {1300},    //                                                
                {1250},    //                                                
                {1100},    //                                                
                {1000},    //                                                
                {1000},    //                                                 
                {1000},    //                                                 
                {1000},    //                                                 
                {1000},    //                                                 
                {1000},    //                                                 


               //                 
                {1300},    //                                                  
                {1100},    //                                                  
                {1000},    //                                                  
                {1000},    //                                                 
                {1000},    //                                                   
                {1000},    //                                                 
                {1000},    //                                                   
                {1000},    //                                                   
                {1000},    //                                                 
                {1000},    //                                                 
                {1300},    //                                                  
                {1100},    //                                                
                {1000},    //                                                  
                {1000},    //                                                 
                {1000},    //                                                 
                {1000},    //                                                 
                {1000},    //                                                   
                {1000},    //                                                 
                {1000},    //                                                 
                {1000},    //                                                   
#endif //                 
                },  //                        

                //       
                {
                //             
                {0},    //                              
                {0},    //                              
                {0},    //                                
                {0},    //                               
                {0},    //                               
                {0},    //                                 
                {0},    //                                

                //               
                {1600},    //                         
                {1600},    //                         
                {1600},    //                          
                {1550},    //                          
                {1550},    //                          
                {1450},    //                          
                {1400},    //                          
                {1400},    //                          

                //                   
                {1600},    //                             
                {1600},    //                             
                {1600},    //                              
                {1650},    //                              
                {1550},    //                              
                {1450},    //                              
                {1400},    //                              
                {1400},    //                              

                //                        
                {1600},    //                                
                {1600},    //                               
                {1550},    //                                 
                {1500},    //                               
                {1450},    //                               
                {1400},    //                               
                {1350},    //                                 
                {1200},    //                               
                {1600},    //                                      
                {1600},    //                                       
                {1550},    //                                       
                {1550},    //                                       
                {1450},    //                                       
                {1400},    //                                       
                {1300},    //                                     
                {1200},    //                                       

                //                         
                {1600},    //                                    
                {1600},    //                                  
                {1550},    //                                    
                {1500},    //                                  
                {1450},    //                                  
                {1400},    //                                   
                {1300},    //                                     
                {1200},    //                                   
                {1600},    //                                        
                {1600},    //                                        
                {1500},    //                                        
                {1500},    //                                        
                {1450},    //                                        
                {1400},    //                                         
                {1300},    //                                         
                {1200},    //                                         
#ifdef WLAN_FEATURE_11AC
                //          
               //                         
                {1700},    //                              
                {1700},    //                              
                {1700},    //                               
                {1650},    //                               
                {1600},    //                               
                {1550},    //                               
                {1550},    //                               
                {1500},    //                               

               //                 
                {1400},    //                                                 
                {1400},    //                                                
                {1350},    //                                                  
                {1300},    //                                                
                {1250},    //                                                
                {1250},    //                                                
                {1100},    //                                                  
                {1000},    //                                                
                {1000},    //                                                
                {1400},    //                                                 
                {1400},    //                                                  
                {1350},    //                                                  
                {1300},    //                                                  
                {1250},    //                                                  
                {1200},    //                                                  
                {1100},    //                                                
                {1000},    //                                                  
                {1000},    //                                                  


               //                 
                {1400},    //                                                  
                {1300},    //                                                
                {1250},    //                                                  
                {1250},    //                                                
                {1000},    //                                                
                {1000},    //                                                 
                {1000},    //                                                   
                {1000},    //                                                 
                {1000},    //                                                 
                {1000},    //                                                 
                {1400},    //                                                
                {1300},    //                                                
                {1250},    //                                                
                {1100},    //                                                
                {1000},    //                                                
                {1000},    //                                                 
                {1000},    //                                                 
                {1000},    //                                                 
                {1000},    //                                                 
                {1000},    //                                                 


               //                 
                {1300},    //                                                  
                {1100},    //                                                  
                {1000},    //                                                  
                {1000},    //                                                 
                {1000},    //                                                   
                {1000},    //                                                 
                {1000},    //                                                   
                {1000},    //                                                   
                {1000},    //                                                 
                {1000},    //                                                 
                {1300},    //                                                  
                {1100},    //                                                
                {1000},    //                                                  
                {1000},    //                                                 
                {1000},    //                                                 
                {1000},    //                                                 
                {1000},    //                                                   
                {1000},    //                                                 
                {1000},    //                                                 
                {1000},    //                                                   
#endif //                 
                },  //                               
                //        
                {
                //             
                {0},    //                              
                {0},    //                              
                {0},    //                                
                {0},    //                               
                {0},    //                               
                {0},    //                                 
                {0},    //                                

                //               
                {1600},    //                         
                {1600},    //                         
                {1600},    //                          
                {1550},    //                          
                {1550},    //                          
                {1450},    //                          
                {1400},    //                          
                {1400},    //                          

                //                   
                {1600},    //                             
                {1600},    //                             
                {1600},    //                              
                {1650},    //                              
                {1550},    //                              
                {1450},    //                              
                {1400},    //                              
                {1400},    //                              

                //                        
                {1600},    //                                
                {1600},    //                               
                {1550},    //                                 
                {1500},    //                               
                {1450},    //                               
                {1400},    //                               
                {1350},    //                                 
                {1200},    //                               
                {1600},    //                                      
                {1600},    //                                       
                {1550},    //                                       
                {1550},    //                                       
                {1450},    //                                       
                {1400},    //                                       
                {1300},    //                                     
                {1200},    //                                       

                //                         
                {1600},    //                                    
                {1600},    //                                  
                {1550},    //                                    
                {1500},    //                                  
                {1450},    //                                  
                {1400},    //                                   
                {1300},    //                                     
                {1200},    //                                   
                {1600},    //                                        
                {1600},    //                                        
                {1500},    //                                        
                {1500},    //                                        
                {1450},    //                                        
                {1400},    //                                         
                {1300},    //                                         
                {1200},    //                                         
#ifdef WLAN_FEATURE_11AC
                //          
               //                         
                {1700},    //                              
                {1700},    //                              
                {1700},    //                               
                {1650},    //                               
                {1600},    //                               
                {1550},    //                               
                {1550},    //                               
                {1500},    //                               

               //                 
                {1400},    //                                                 
                {1400},    //                                                
                {1350},    //                                                  
                {1300},    //                                                
                {1250},    //                                                
                {1250},    //                                                
                {1100},    //                                                  
                {1000},    //                                                
                {1000},    //                                                
                {1400},    //                                                 
                {1400},    //                                                  
                {1350},    //                                                  
                {1300},    //                                                  
                {1250},    //                                                  
                {1200},    //                                                  
                {1100},    //                                                
                {1000},    //                                                  
                {1000},    //                                                  


               //                 
                {1400},    //                                                  
                {1300},    //                                                
                {1250},    //                                                  
                {1250},    //                                                
                {1000},    //                                                
                {1000},    //                                                 
                {1000},    //                                                   
                {1000},    //                                                 
                {1000},    //                                                 
                {1000},    //                                                 
                {1400},    //                                                
                {1300},    //                                                
                {1250},    //                                                
                {1100},    //                                                
                {1000},    //                                                
                {1000},    //                                                 
                {1000},    //                                                 
                {1000},    //                                                 
                {1000},    //                                                 
                {1000},    //                                                 


               //                 
                {1300},    //                                                  
                {1100},    //                                                  
                {1000},    //                                                  
                {1000},    //                                                 
                {1000},    //                                                   
                {1000},    //                                                 
                {1000},    //                                                   
                {1000},    //                                                   
                {1000},    //                                                 
                {1000},    //                                                 
                {1300},    //                                                  
                {1100},    //                                                
                {1000},    //                                                  
                {1000},    //                                                 
                {1000},    //                                                 
                {1000},    //                                                 
                {1000},    //                                                   
                {1000},    //                                                 
                {1000},    //                                                 
                {1000},    //                                                   
#endif //                 
                },  //                          
                //     

                {
                //             
                {0},    //                              
                {0},    //                              
                {0},    //                                
                {0},    //                               
                {0},    //                               
                {0},    //                                 
                {0},    //                                

                //               
                {1600},    //                         
                {1600},    //                         
                {1600},    //                          
                {1550},    //                          
                {1550},    //                          
                {1450},    //                          
                {1400},    //                          
                {1400},    //                          

                //                   
                {1600},    //                             
                {1600},    //                             
                {1600},    //                              
                {1650},    //                              
                {1550},    //                              
                {1450},    //                              
                {1400},    //                              
                {1400},    //                              

                //                        
                {1600},    //                                
                {1600},    //                               
                {1550},    //                                 
                {1500},    //                               
                {1450},    //                               
                {1400},    //                               
                {1350},    //                                 
                {1200},    //                               
                {1600},    //                                      
                {1600},    //                                       
                {1550},    //                                       
                {1550},    //                                       
                {1450},    //                                       
                {1400},    //                                       
                {1300},    //                                     
                {1200},    //                                       

                //                         
                {1600},    //                                    
                {1600},    //                                  
                {1550},    //                                    
                {1500},    //                                  
                {1450},    //                                  
                {1400},    //                                   
                {1300},    //                                     
                {1200},    //                                   
                {1600},    //                                        
                {1600},    //                                        
                {1500},    //                                        
                {1500},    //                                        
                {1450},    //                                        
                {1400},    //                                         
                {1300},    //                                         
                {1200},    //                                         
#ifdef WLAN_FEATURE_11AC
                //          
               //                         
                {1700},    //                              
                {1700},    //                              
                {1700},    //                               
                {1650},    //                               
                {1600},    //                               
                {1550},    //                               
                {1550},    //                               
                {1500},    //                               

               //                 
                {0000},    //                                              
                {0000},    //                                             
                {0000},    //                                               
                {0000},    //                                             
                {0000},    //                                             
                {0000},    //                                             
                {0000},    //                                               
                {0000},    //                                             
                {0000},    //                                             
                {0000},    //                                              
                {0000},    //                                               
                {0000},    //                                               
                {0000},    //                                               
                {0000},    //                                               
                {0000},    //                                               
                {0000},    //                                             
                {0000},    //                                               
                {0000},    //                                               


               //                 
                {0000},    //                                                  
                {0000},    //                                                
                {0000},    //                                                  
                {0000},    //                                                
                {0000},    //                                                
                {0000},    //                                                 
                {0000},    //                                                   
                {0000},    //                                                 
                {0000},    //                                                 
                {0000},    //                                                 
                {0000},    //                                                
                {0000},    //                                                
                {0000},    //                                                
                {0000},    //                                                
                {0000},    //                                                
                {0000},    //                                                 
                {0000},    //                                                 
                {0000},    //                                                 
                {0000},    //                                                 
                {0000},    //                                                 


               //                 
                {0000},    //                                                  
                {0000},    //                                                  
                {0000},    //                                                  
                {0000},    //                                                 
                {0000},    //                                                   
                {0000},    //                                                 
                {0000},    //                                                   
                {0000},    //                                                   
                {0000},    //                                                 
                {0000},    //                                                 
                {0000},    //                                                  
                {0000},    //                                                
                {0000},    //                                                  
                {0000},    //                                                 
                {0000},    //                                                 
                {0000},    //                                                 
                {0000},    //                                                   
                {0000},    //                                                 
                {0000},    //                                                 
                {0000},    //                                                   
#endif //                 
                },  //                      
        },

        //                            
        {
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


            {   //                     
                { //                        
                    //                 
                    //                    
                    {NV_CHANNEL_ENABLE, 23},           //          
                    {NV_CHANNEL_ENABLE, 23},           //          
                    {NV_CHANNEL_ENABLE, 23},           //          
                    {NV_CHANNEL_ENABLE, 23},           //          
                    {NV_CHANNEL_ENABLE, 23},           //          
                    {NV_CHANNEL_ENABLE, 23},           //          
                    {NV_CHANNEL_ENABLE, 23},           //          
                    {NV_CHANNEL_ENABLE, 23},           //          
                    {NV_CHANNEL_ENABLE, 23},           //          
                    {NV_CHANNEL_ENABLE, 22},           //           
                    {NV_CHANNEL_ENABLE, 22},           //           
                    {NV_CHANNEL_DISABLE, 30},           //           
                    {NV_CHANNEL_DISABLE, 30},           //           
                    {NV_CHANNEL_DISABLE, 30},           //           

                    //                    
                    {NV_CHANNEL_DISABLE, 23},            //            
                    {NV_CHANNEL_DISABLE, 23},            //            
                    {NV_CHANNEL_DISABLE, 23},            //            
                    {NV_CHANNEL_DISABLE, 23},            //            
                    {NV_CHANNEL_DISABLE, 23},            //            
                    {NV_CHANNEL_DISABLE, 23},            //            
                    {NV_CHANNEL_DISABLE, 23},            //            

                    //                                  
                    {NV_CHANNEL_ENABLE, 17},             //           
                    {NV_CHANNEL_ENABLE, 17},             //           
                    {NV_CHANNEL_ENABLE, 17},             //           
                    {NV_CHANNEL_ENABLE, 17},             //           
                    {NV_CHANNEL_DFS, 24},                //           
                    {NV_CHANNEL_DFS, 24},                //           
                    {NV_CHANNEL_DFS, 24},                //           
                    {NV_CHANNEL_DFS, 24},                //           

                    //                             
                    {NV_CHANNEL_DFS, 22},                //            
                    {NV_CHANNEL_DFS, 24},                //            
                    {NV_CHANNEL_DFS, 24},                //            
                    {NV_CHANNEL_DFS, 24},                //            
                    {NV_CHANNEL_DFS, 24},                //            
                    {NV_CHANNEL_DISABLE, 0},             //            
                    {NV_CHANNEL_DISABLE, 0},             //            
                    {NV_CHANNEL_DISABLE, 0},             //            
                    {NV_CHANNEL_DFS, 24},                //            
                    {NV_CHANNEL_DFS, 24},                //            
                    {NV_CHANNEL_DFS, 24},                //            

                    //                             
                    {NV_CHANNEL_ENABLE, 30},            //            
                    {NV_CHANNEL_ENABLE, 30},            //            
                    {NV_CHANNEL_ENABLE, 30},            //            
                    {NV_CHANNEL_ENABLE, 30},            //            
                    {NV_CHANNEL_ENABLE, 0},             //            

                    //                                    
                    {NV_CHANNEL_ENABLE, 22},            //               
                    {NV_CHANNEL_ENABLE, 30},            //               
                    {NV_CHANNEL_ENABLE, 30},            //               
                    {NV_CHANNEL_ENABLE, 30},            //               
                    {NV_CHANNEL_ENABLE, 30},            //               
                    {NV_CHANNEL_ENABLE, 30},            //               
                    {NV_CHANNEL_ENABLE, 22},            //               
                    {NV_CHANNEL_ENABLE, 0},            //                
                    {NV_CHANNEL_ENABLE, 0},            //                

                    //                                     
                    {NV_CHANNEL_DISABLE, 0},            //                 
                    {NV_CHANNEL_DISABLE, 0},            //                 
                    {NV_CHANNEL_DISABLE, 0},            //                 
                    {NV_CHANNEL_DISABLE, 0},            //                 
                    {NV_CHANNEL_DISABLE, 0},            //                 

                    //                                                  
                    {NV_CHANNEL_ENABLE, 20},            //                
                    {NV_CHANNEL_ENABLE, 20},            //                
                    {NV_CHANNEL_ENABLE, 20},            //                
                    {NV_CHANNEL_ENABLE, 27},            //                
                    {NV_CHANNEL_ENABLE, 27},            //                
                    {NV_CHANNEL_ENABLE, 27},            //                
                    {NV_CHANNEL_ENABLE, 25},            //                

                    //                                             
                    {NV_CHANNEL_DISABLE, 24},            //                
                    {NV_CHANNEL_DISABLE, 27},            //                
                    {NV_CHANNEL_DISABLE, 27},            //                
                    {NV_CHANNEL_DISABLE, 0},            //                
                    {NV_CHANNEL_DISABLE, 0},            //                
                    {NV_CHANNEL_DISABLE, 0},            //                
                    {NV_CHANNEL_DISABLE, 0},            //                
                    {NV_CHANNEL_DISABLE, 0},            //                
                    {NV_CHANNEL_DISABLE, 27},            //                
                    {NV_CHANNEL_DISABLE, 27},            //                

                    //                                              
                    {NV_CHANNEL_DISABLE, 30},            //                 
                    {NV_CHANNEL_DISABLE, 30},            //                 
                    {NV_CHANNEL_DISABLE, 30},            //                 
                    {NV_CHANNEL_DISABLE, 0},             //                
                }, //                      

                {
                    { 0 },  //                   
                    {0},   //                     
                    {0},   //                     
                    {0},   //                      
                    {0}    //                   
                },

                { //                       
                    //           
                    { 0 },                       //          
                    { 0 },                       //          
                    { 0 },                       //          
                    { 0 },                       //          
                    { 0 },                       //          
                    { 0 },                       //          
                    { 0 },                       //          
                    { 0 },                       //          
                    { 0 },                       //          
                    { 0 },                       //           
                    { 0 },                       //           
                    { 0 },                       //           
                    { 0 },                       //           
                    { 0 },                       //           
                }, //                     

                { //                        
                    //                                
                    { 0 },                       //          
                    { 0 },                       //          
                    { 0 },                       //          
                    { 0 },                       //          
                    { 0 },                       //          
                    { 0 },                       //          
                    { 0 },                       //          
                    { 0 },                       //          
                    { 0 },                       //          
                    { 0 },                       //           
                    { 0 },                       //           
                    { 0 },                       //           
                    { 0 },                       //           
                    { 0 },                       //           
                } //                      
            }, //                   

            {   //                      
                { //                        
                    //                 
                    //                    
                    {NV_CHANNEL_ENABLE, 20},           //          
                    {NV_CHANNEL_ENABLE, 20},           //          
                    {NV_CHANNEL_ENABLE, 20},           //          
                    {NV_CHANNEL_ENABLE, 20},           //          
                    {NV_CHANNEL_ENABLE, 20},           //          
                    {NV_CHANNEL_ENABLE, 20},           //          
                    {NV_CHANNEL_ENABLE, 20},           //          
                    {NV_CHANNEL_ENABLE, 20},           //          
                    {NV_CHANNEL_ENABLE, 20},           //          
                    {NV_CHANNEL_ENABLE, 20},           //           
                    {NV_CHANNEL_ENABLE, 20},           //           
                    {NV_CHANNEL_ENABLE, 19},           //           
                    {NV_CHANNEL_ENABLE, 19},           //           
                    {NV_CHANNEL_DISABLE, 0},           //           

                    //                    
                    {NV_CHANNEL_DISABLE, 0},            //            
                    {NV_CHANNEL_DISABLE, 0},            //            
                    {NV_CHANNEL_DISABLE, 0},            //            
                    {NV_CHANNEL_DISABLE, 0},            //            
                    {NV_CHANNEL_DISABLE, 0},            //            
                    {NV_CHANNEL_DISABLE, 0},            //            
                    {NV_CHANNEL_DISABLE, 0},            //            

                    //                                  
                    {NV_CHANNEL_ENABLE, 23},            //           
                    {NV_CHANNEL_ENABLE, 23},            //           
                    {NV_CHANNEL_ENABLE, 23},            //           
                    {NV_CHANNEL_ENABLE, 23},            //           
                    {NV_CHANNEL_DISABLE, 23},            //           
                    {NV_CHANNEL_DISABLE, 23},            //           
                    {NV_CHANNEL_DISABLE, 23},            //           
                    {NV_CHANNEL_DISABLE, 23},            //           

                    //                             
                    {NV_CHANNEL_DISABLE, 23},            //            
                    {NV_CHANNEL_DISABLE, 23},            //            
                    {NV_CHANNEL_DISABLE, 23},            //            
                    {NV_CHANNEL_DISABLE, 23},            //            
                    {NV_CHANNEL_DISABLE, 23},            //            
                    {NV_CHANNEL_DISABLE, 0},            //            
                    {NV_CHANNEL_DISABLE, 0},            //            
                    {NV_CHANNEL_DISABLE, 0},            //            
                    {NV_CHANNEL_DISABLE, 23},            //            
                    {NV_CHANNEL_DISABLE, 23},            //            
                    {NV_CHANNEL_DISABLE, 23},            //            

                    //                             
                    {NV_CHANNEL_DISABLE, 0},            //            
                    {NV_CHANNEL_DISABLE, 0},            //            
                    {NV_CHANNEL_DISABLE, 0},            //            
                    {NV_CHANNEL_DISABLE, 0},            //            
                    {NV_CHANNEL_DISABLE, 0},            //            

                    //                                    
                    {NV_CHANNEL_ENABLE, 20},            //               
                    {NV_CHANNEL_ENABLE, 20},            //               
                    {NV_CHANNEL_ENABLE, 20},            //               
                    {NV_CHANNEL_ENABLE, 20},            //               
                    {NV_CHANNEL_ENABLE, 20},            //               
                    {NV_CHANNEL_ENABLE, 20},            //               
                    {NV_CHANNEL_ENABLE, 20},            //               
                    {NV_CHANNEL_ENABLE, 20},            //                
                    {NV_CHANNEL_ENABLE, 20},            //                

                    //                                     
                    {NV_CHANNEL_DISABLE, 0},            //                 
                    {NV_CHANNEL_DISABLE, 0},            //                 
                    {NV_CHANNEL_DISABLE, 0},            //                 
                    {NV_CHANNEL_DISABLE, 0},            //                 
                    {NV_CHANNEL_DISABLE, 0},            //                 

                    //                                                  
                    {NV_CHANNEL_ENABLE, 23},            //                
                    {NV_CHANNEL_ENABLE, 23},            //                
                    {NV_CHANNEL_ENABLE, 23},            //                
                    {NV_CHANNEL_ENABLE, 23},            //                
                    {NV_CHANNEL_ENABLE, 23},            //                
                    {NV_CHANNEL_ENABLE, 23},            //                
                    {NV_CHANNEL_ENABLE, 23},            //                

                    //                                             
                    {NV_CHANNEL_DISABLE, 23},            //                
                    {NV_CHANNEL_DISABLE, 23},            //                
                    {NV_CHANNEL_DISABLE, 23},            //                
                    {NV_CHANNEL_DISABLE, 0},            //                
                    {NV_CHANNEL_DISABLE, 0},            //                
                    {NV_CHANNEL_DISABLE, 0},            //                
                    {NV_CHANNEL_DISABLE, 0},            //                
                    {NV_CHANNEL_DISABLE, 0},            //                
                    {NV_CHANNEL_DISABLE, 23},            //                
                    {NV_CHANNEL_DISABLE, 23},            //                

                    //                                              
                    {NV_CHANNEL_DISABLE, 0},            //                 
                    {NV_CHANNEL_DISABLE, 0},            //                 
                    {NV_CHANNEL_DISABLE, 0},            //                 
                    {NV_CHANNEL_DISABLE, 0},             //                
                }, //                      

                {
                    { 0 },  //                   
                    {0},   //                     
                    {0},   //                     
                    {0},   //                      
                    {0}    //                   
                },

                { //                       
                    //           
                    { 0 },                       //          
                    { 0 },                       //          
                    { 0 },                       //          
                    { 0 },                       //          
                    { 0 },                       //          
                    { 0 },                       //          
                    { 0 },                       //          
                    { 0 },                       //          
                    { 0 },                       //          
                    { 0 },                       //           
                    { 0 },                       //           
                    { 0 },                       //           
                    { 0 },                       //           
                    { 0 },                       //           
                }, //                     

                { //                        
                    //                                
                    { 0 },                       //          
                    { 0 },                       //          
                    { 0 },                       //          
                    { 0 },                       //          
                    { 0 },                       //          
                    { 0 },                       //          
                    { 0 },                       //          
                    { 0 },                       //          
                    { 0 },                       //          
                    { 0 },                       //           
                    { 0 },                       //           
                    { 0 },                       //           
                    { 0 },                       //           
                    { 0 },                       //           
                } //                      
            }, //                    

            {   //                       
                { //                        
                    //                 
                    //                    
                    {NV_CHANNEL_ENABLE, 20},           //          
                    {NV_CHANNEL_ENABLE, 20},           //          
                    {NV_CHANNEL_ENABLE, 20},           //          
                    {NV_CHANNEL_ENABLE, 20},           //          
                    {NV_CHANNEL_ENABLE, 20},           //          
                    {NV_CHANNEL_ENABLE, 20},           //          
                    {NV_CHANNEL_ENABLE, 20},           //          
                    {NV_CHANNEL_ENABLE, 20},           //          
                    {NV_CHANNEL_ENABLE, 20},           //          
                    {NV_CHANNEL_ENABLE, 20},           //           
                    {NV_CHANNEL_ENABLE, 20},           //           
                    {NV_CHANNEL_ENABLE, 20},           //           
                    {NV_CHANNEL_ENABLE, 20},           //           
                    {NV_CHANNEL_ENABLE, 18},           //           

                    //                    
                    {NV_CHANNEL_DISABLE, 23},            //            
                    {NV_CHANNEL_DISABLE, 23},            //            
                    {NV_CHANNEL_DISABLE, 23},            //            
                    {NV_CHANNEL_DISABLE, 23},            //            
                    {NV_CHANNEL_DISABLE, 23},            //            
                    {NV_CHANNEL_DISABLE, 23},            //            
                    {NV_CHANNEL_DISABLE, 23},            //            

                    //                                  
                    {NV_CHANNEL_ENABLE, 17},            //           
                    {NV_CHANNEL_ENABLE, 17},            //           
                    {NV_CHANNEL_ENABLE, 17},            //           
                    {NV_CHANNEL_ENABLE, 17},            //           
                    {NV_CHANNEL_DFS, 24},               //           
                    {NV_CHANNEL_DFS, 24},               //           
                    {NV_CHANNEL_DFS, 24},               //           
                    {NV_CHANNEL_DFS, 24},               //           

                    //                             
                    {NV_CHANNEL_DFS, 22},               //            
                    {NV_CHANNEL_DFS, 24},               //            
                    {NV_CHANNEL_DFS, 24},               //            
                    {NV_CHANNEL_DFS, 24},               //            
                    {NV_CHANNEL_DFS, 24},               //            
                    {NV_CHANNEL_DFS, 0},                //            
                    {NV_CHANNEL_DFS, 0},                //            
                    {NV_CHANNEL_DFS, 0},                //            
                    {NV_CHANNEL_DFS, 24},               //            
                    {NV_CHANNEL_DFS, 24},               //            
                    {NV_CHANNEL_DFS, 24},               //            

                    //                             
                    {NV_CHANNEL_DISABLE, 30},            //            
                    {NV_CHANNEL_DISABLE, 30},            //            
                    {NV_CHANNEL_DISABLE, 30},            //            
                    {NV_CHANNEL_DISABLE, 30},            //            
                    {NV_CHANNEL_DISABLE, 0},            //            

                    //                                    
                    {NV_CHANNEL_ENABLE, 22},            //               
                    {NV_CHANNEL_ENABLE, 30},            //               
                    {NV_CHANNEL_ENABLE, 30},            //               
                    {NV_CHANNEL_ENABLE, 30},            //               
                    {NV_CHANNEL_ENABLE, 30},            //               
                    {NV_CHANNEL_ENABLE, 30},            //               
                    {NV_CHANNEL_ENABLE, 22},            //               
                    {NV_CHANNEL_ENABLE, 0},            //                
                    {NV_CHANNEL_ENABLE, 0},            //                

                    //                                     
                    {NV_CHANNEL_DISABLE, 0},            //                 
                    {NV_CHANNEL_DISABLE, 0},            //                 
                    {NV_CHANNEL_DISABLE, 0},            //                 
                    {NV_CHANNEL_DISABLE, 0},            //                 
                    {NV_CHANNEL_DISABLE, 0},            //                 

                    //                                                  
                    {NV_CHANNEL_ENABLE, 20},            //                
                    {NV_CHANNEL_ENABLE, 20},            //                
                    {NV_CHANNEL_ENABLE, 20},            //                
                    {NV_CHANNEL_ENABLE, 27},            //                
                    {NV_CHANNEL_ENABLE, 27},            //                
                    {NV_CHANNEL_ENABLE, 27},            //                
                    {NV_CHANNEL_ENABLE, 25},            //                

                    //                                             
                    {NV_CHANNEL_DISABLE, 24},            //                
                    {NV_CHANNEL_DISABLE, 27},            //                
                    {NV_CHANNEL_DISABLE, 27},            //                
                    {NV_CHANNEL_DISABLE, 0},            //                
                    {NV_CHANNEL_DISABLE, 0},            //                
                    {NV_CHANNEL_DISABLE, 0},            //                
                    {NV_CHANNEL_DISABLE, 0},            //                
                    {NV_CHANNEL_DISABLE, 0},            //                
                    {NV_CHANNEL_DISABLE, 27},            //                
                    {NV_CHANNEL_DISABLE, 27},            //                

                    //                                              
                    {NV_CHANNEL_DISABLE, 30},            //                 
                    {NV_CHANNEL_DISABLE, 30},            //                 
                    {NV_CHANNEL_DISABLE, 30},            //                 
                    {NV_CHANNEL_DISABLE, 0},             //                
                }, //                      

                {
                    { 0 },  //                   
                    {0},   //                     
                    {0},   //                     
                    {0},   //                      
                    {0}    //                   
                },

                { //                       
                    //           
                    { 0 },                       //          
                    { 0 },                       //          
                    { 0 },                       //          
                    { 0 },                       //          
                    { 0 },                       //          
                    { 0 },                       //          
                    { 0 },                       //          
                    { 0 },                       //          
                    { 0 },                       //          
                    { 0 },                       //           
                    { 0 },                       //           
                    { 0 },                       //           
                    { 0 },                       //           
                    { 0 },                       //           
                }, //                     

                { //                        
                    //                                
                    { 0 },                       //          
                    { 0 },                       //          
                    { 0 },                       //          
                    { 0 },                       //          
                    { 0 },                       //          
                    { 0 },                       //          
                    { 0 },                       //          
                    { 0 },                       //          
                    { 0 },                       //          
                    { 0 },                       //           
                    { 0 },                       //           
                    { 0 },                       //           
                    { 0 },                       //           
                    { 0 },                       //           
                } //                      
            }, //                     

            {   //                       
                { //                        
                    //                 
                                       //           
                    {NV_CHANNEL_ENABLE, 16},           //          
                    {NV_CHANNEL_ENABLE, 16},           //          
                    {NV_CHANNEL_ENABLE, 16},           //          
                    {NV_CHANNEL_ENABLE, 16},           //          
                    {NV_CHANNEL_ENABLE, 16},           //          
                    {NV_CHANNEL_ENABLE, 16},           //          
                    {NV_CHANNEL_ENABLE, 16},           //          
                    {NV_CHANNEL_ENABLE, 16},           //          
                    {NV_CHANNEL_ENABLE, 16},           //          
                    {NV_CHANNEL_ENABLE, 16},           //           
                    {NV_CHANNEL_ENABLE, 16},           //           
                    {NV_CHANNEL_ENABLE, 16},           //           
                    {NV_CHANNEL_ENABLE, 16},           //           
                    {NV_CHANNEL_DISABLE, 0},           //           

                    //                    
                    {NV_CHANNEL_DISABLE, 23},            //            
                    {NV_CHANNEL_DISABLE, 23},            //            
                    {NV_CHANNEL_DISABLE, 23},            //            
                    {NV_CHANNEL_DISABLE, 23},            //            
                    {NV_CHANNEL_DISABLE, 23},            //            
                    {NV_CHANNEL_DISABLE, 23},            //            
                    {NV_CHANNEL_DISABLE, 23},            //            

                    //                                  
                    {NV_CHANNEL_ENABLE, 17},            //           
                    {NV_CHANNEL_ENABLE, 17},            //           
                    {NV_CHANNEL_ENABLE, 17},            //           
                    {NV_CHANNEL_ENABLE, 17},            //           
                    {NV_CHANNEL_DFS, 24},               //           
                    {NV_CHANNEL_DFS, 24},               //           
                    {NV_CHANNEL_DFS, 24},               //           
                    {NV_CHANNEL_DFS, 24},               //           

                    //                             
                    {NV_CHANNEL_DFS, 22},               //            
                    {NV_CHANNEL_DFS, 24},               //            
                    {NV_CHANNEL_DFS, 24},               //            
                    {NV_CHANNEL_DFS, 24},               //            
                    {NV_CHANNEL_DFS, 24},               //            
                    {NV_CHANNEL_DFS, 0},                //            
                    {NV_CHANNEL_DFS, 0},                //            
                    {NV_CHANNEL_DFS, 0},                //            
                    {NV_CHANNEL_DFS, 24},               //            
                    {NV_CHANNEL_DFS, 24},               //            
                    {NV_CHANNEL_DFS, 24},               //            

                    //                             
                    {NV_CHANNEL_DISABLE, 30},            //            
                    {NV_CHANNEL_DISABLE, 30},            //            
                    {NV_CHANNEL_DISABLE, 30},            //            
                    {NV_CHANNEL_DISABLE, 30},            //            
                    {NV_CHANNEL_DISABLE, 0},            //            

                    //                                    
                    {NV_CHANNEL_ENABLE, 22},            //               
                    {NV_CHANNEL_ENABLE, 30},            //               
                    {NV_CHANNEL_ENABLE, 30},            //               
                    {NV_CHANNEL_ENABLE, 30},            //               
                    {NV_CHANNEL_ENABLE, 30},            //               
                    {NV_CHANNEL_ENABLE, 30},            //               
                    {NV_CHANNEL_ENABLE, 22},            //               
                    {NV_CHANNEL_ENABLE, 0},            //                
                    {NV_CHANNEL_ENABLE, 0},            //                

                    //                                     
                    {NV_CHANNEL_DISABLE, 0},            //                 
                    {NV_CHANNEL_DISABLE, 0},            //                 
                    {NV_CHANNEL_DISABLE, 0},            //                 
                    {NV_CHANNEL_DISABLE, 0},            //                 
                    {NV_CHANNEL_DISABLE, 0},            //                 

                    //                                                  
                    {NV_CHANNEL_ENABLE, 20},            //                
                    {NV_CHANNEL_ENABLE, 20},            //                
                    {NV_CHANNEL_ENABLE, 20},            //                
                    {NV_CHANNEL_ENABLE, 27},            //                
                    {NV_CHANNEL_ENABLE, 27},            //                
                    {NV_CHANNEL_ENABLE, 27},            //                
                    {NV_CHANNEL_ENABLE, 25},            //                

                    //                                             
                    {NV_CHANNEL_DISABLE, 24},            //                
                    {NV_CHANNEL_DISABLE, 27},            //                
                    {NV_CHANNEL_DISABLE, 27},            //                
                    {NV_CHANNEL_DISABLE, 0},            //                
                    {NV_CHANNEL_DISABLE, 0},            //                
                    {NV_CHANNEL_DISABLE, 0},            //                
                    {NV_CHANNEL_DISABLE, 0},            //                
                    {NV_CHANNEL_DISABLE, 0},            //                
                    {NV_CHANNEL_DISABLE, 27},            //                
                    {NV_CHANNEL_DISABLE, 27},            //                

                    //                                              
                    {NV_CHANNEL_DISABLE, 30},            //                 
                    {NV_CHANNEL_DISABLE, 30},            //                 
                    {NV_CHANNEL_DISABLE, 30},            //                 
                    {NV_CHANNEL_DISABLE, 0},             //                
                }, //                      

                {
                    { 0 },  //                   
                    {0},   //                     
                    {0},   //                     
                    {0},   //                      
                    {0}    //                   
                },

                { //                       
                    //           
                    { 0 },                       //          
                    { 0 },                       //          
                    { 0 },                       //          
                    { 0 },                       //          
                    { 0 },                       //          
                    { 0 },                       //          
                    { 0 },                       //          
                    { 0 },                       //          
                    { 0 },                       //          
                    { 0 },                       //           
                    { 0 },                       //           
                    { 0 },                       //           
                    { 0 },                       //           
                    { 0 },                       //           
                }, //                     

                { //                        
                    //                                
                    { 0 },                       //          
                    { 0 },                       //          
                    { 0 },                       //          
                    { 0 },                       //          
                    { 0 },                       //          
                    { 0 },                       //          
                    { 0 },                       //          
                    { 0 },                       //          
                    { 0 },                       //          
                    { 0 },                       //           
                    { 0 },                       //           
                    { 0 },                       //           
                    { 0 },                       //           
                    { 0 },                       //           
                } //                      
            }, //                     

            {   //                                
                { //                        
                    //                 
                    //                    
                    {NV_CHANNEL_ENABLE, 16},           //          
                    {NV_CHANNEL_ENABLE, 16},           //          
                    {NV_CHANNEL_ENABLE, 16},           //          
                    {NV_CHANNEL_ENABLE, 16},           //          
                    {NV_CHANNEL_ENABLE, 16},           //          
                    {NV_CHANNEL_ENABLE, 16},           //          
                    {NV_CHANNEL_ENABLE, 16},           //          
                    {NV_CHANNEL_ENABLE, 16},           //          
                    {NV_CHANNEL_ENABLE, 16},           //          
                    {NV_CHANNEL_ENABLE, 16},           //           
                    {NV_CHANNEL_ENABLE, 16},           //           
                    {NV_CHANNEL_DISABLE, 30},           //           
                    {NV_CHANNEL_DISABLE, 30},           //           
                    {NV_CHANNEL_DISABLE, 30},           //           

                    //                    
                    {NV_CHANNEL_DISABLE, 23},            //            
                    {NV_CHANNEL_DISABLE, 23},            //            
                    {NV_CHANNEL_DISABLE, 23},            //            
                    {NV_CHANNEL_DISABLE, 23},            //            
                    {NV_CHANNEL_DISABLE, 23},            //            
                    {NV_CHANNEL_DISABLE, 23},            //            
                    {NV_CHANNEL_DISABLE, 23},            //            

                    //                                  
                    {NV_CHANNEL_ENABLE, 17},            //           
                    {NV_CHANNEL_ENABLE, 17},            //           
                    {NV_CHANNEL_ENABLE, 17},            //           
                    {NV_CHANNEL_ENABLE, 17},            //           
                    {NV_CHANNEL_DISABLE, 24},            //           
                    {NV_CHANNEL_DISABLE, 24},            //           
                    {NV_CHANNEL_DISABLE, 24},            //           
                    {NV_CHANNEL_DISABLE, 24},            //           

                    //                             
                    {NV_CHANNEL_DISABLE, 22},            //            
                    {NV_CHANNEL_DISABLE, 24},            //            
                    {NV_CHANNEL_DISABLE, 24},            //            
                    {NV_CHANNEL_DISABLE, 24},            //            
                    {NV_CHANNEL_DISABLE, 24},            //            
                    {NV_CHANNEL_DISABLE, 0},            //            
                    {NV_CHANNEL_DISABLE, 0},            //            
                    {NV_CHANNEL_DISABLE, 0},            //            
                    {NV_CHANNEL_DISABLE, 24},            //            
                    {NV_CHANNEL_DISABLE, 24},            //            
                    {NV_CHANNEL_DISABLE, 24},            //            

                    //                             
                    {NV_CHANNEL_ENABLE, 30},            //            
                    {NV_CHANNEL_ENABLE, 30},            //            
                    {NV_CHANNEL_ENABLE, 30},            //            
                    {NV_CHANNEL_ENABLE, 30},            //            
                    {NV_CHANNEL_ENABLE, 0},            //            

                    //                                    
                    {NV_CHANNEL_ENABLE, 22},            //               
                    {NV_CHANNEL_ENABLE, 30},            //               
                    {NV_CHANNEL_ENABLE, 30},            //               
                    {NV_CHANNEL_ENABLE, 30},            //               
                    {NV_CHANNEL_ENABLE, 30},            //               
                    {NV_CHANNEL_ENABLE, 30},            //               
                    {NV_CHANNEL_ENABLE, 22},            //               
                    {NV_CHANNEL_ENABLE, 0},            //                
                    {NV_CHANNEL_ENABLE, 0},            //                

                    //                                     
                    {NV_CHANNEL_DISABLE, 0},            //                 
                    {NV_CHANNEL_DISABLE, 0},            //                 
                    {NV_CHANNEL_DISABLE, 0},            //                 
                    {NV_CHANNEL_DISABLE, 0},            //                 
                    {NV_CHANNEL_DISABLE, 0},            //                 

                    //                                                  
                    {NV_CHANNEL_ENABLE, 20},            //                
                    {NV_CHANNEL_ENABLE, 20},            //                
                    {NV_CHANNEL_ENABLE, 20},            //                
                    {NV_CHANNEL_ENABLE, 27},            //                
                    {NV_CHANNEL_ENABLE, 27},            //                
                    {NV_CHANNEL_ENABLE, 27},            //                
                    {NV_CHANNEL_ENABLE, 25},            //                

                    //                                             
                    {NV_CHANNEL_DISABLE, 24},            //                
                    {NV_CHANNEL_DISABLE, 27},            //                
                    {NV_CHANNEL_DISABLE, 27},            //                
                    {NV_CHANNEL_DISABLE, 0},            //                
                    {NV_CHANNEL_DISABLE, 0},            //                
                    {NV_CHANNEL_DISABLE, 0},            //                
                    {NV_CHANNEL_DISABLE, 0},            //                
                    {NV_CHANNEL_DISABLE, 0},            //                
                    {NV_CHANNEL_DISABLE, 27},            //                
                    {NV_CHANNEL_DISABLE, 27},            //                

                    //                                              
                    {NV_CHANNEL_DISABLE, 30},            //                 
                    {NV_CHANNEL_DISABLE, 30},            //                 
                    {NV_CHANNEL_DISABLE, 30},            //                 
                    {NV_CHANNEL_DISABLE, 0},             //                
                }, //                      

                {
                    { 0 },  //                   
                    {0},   //                     
                    {0},   //                     
                    {0},   //                      
                    {0}    //                   
                },

                { //                       
                    //           
                    { 0 },                       //          
                    { 0 },                       //          
                    { 0 },                       //          
                    { 0 },                       //          
                    { 0 },                       //          
                    { 0 },                       //          
                    { 0 },                       //          
                    { 0 },                       //          
                    { 0 },                       //          
                    { 0 },                       //           
                    { 0 },                       //           
                    { 0 },                       //           
                    { 0 },                       //           
                    { 0 },                       //           
                }, //                     

                { //                        
                    //                                
                    { 0 },                       //          
                    { 0 },                       //          
                    { 0 },                       //          
                    { 0 },                       //          
                    { 0 },                       //          
                    { 0 },                       //          
                    { 0 },                       //          
                    { 0 },                       //          
                    { 0 },                       //          
                    { 0 },                       //           
                    { 0 },                       //           
                    { 0 },                       //           
                    { 0 },                       //           
                    { 0 },                       //           
                } //                      
            },   //                              

            {   //                      
                { //                        
                    //                 
                    //                    
                    {NV_CHANNEL_ENABLE, 26},           //          
                    {NV_CHANNEL_ENABLE, 26},           //          
                    {NV_CHANNEL_ENABLE, 26},           //          
                    {NV_CHANNEL_ENABLE, 26},           //          
                    {NV_CHANNEL_ENABLE, 26},           //          
                    {NV_CHANNEL_ENABLE, 26},           //          
                    {NV_CHANNEL_ENABLE, 26},           //          
                    {NV_CHANNEL_ENABLE, 26},           //          
                    {NV_CHANNEL_ENABLE, 26},           //          
                    {NV_CHANNEL_ENABLE, 26},           //           
                    {NV_CHANNEL_ENABLE, 26},           //           
                    {NV_CHANNEL_ENABLE, 26},           //           
                    {NV_CHANNEL_ENABLE, 16},           //           
                    {NV_CHANNEL_DISABLE, 0},           //           

                    //                    
                    {NV_CHANNEL_DISABLE, 23},            //            
                    {NV_CHANNEL_DISABLE, 23},            //            
                    {NV_CHANNEL_DISABLE, 23},            //            
                    {NV_CHANNEL_DISABLE, 23},            //            
                    {NV_CHANNEL_DISABLE, 23},            //            
                    {NV_CHANNEL_DISABLE, 23},            //            
                    {NV_CHANNEL_DISABLE, 23},            //            

                    //                                  
                    {NV_CHANNEL_ENABLE, 17},            //           
                    {NV_CHANNEL_ENABLE, 17},            //           
                    {NV_CHANNEL_ENABLE, 17},            //           
                    {NV_CHANNEL_ENABLE, 17},            //           
                    {NV_CHANNEL_DISABLE, 24},            //           
                    {NV_CHANNEL_DISABLE, 24},            //           
                    {NV_CHANNEL_DISABLE, 24},            //           
                    {NV_CHANNEL_DISABLE, 24},            //           

                    //                             
                    {NV_CHANNEL_DISABLE, 22},            //            
                    {NV_CHANNEL_DISABLE, 24},            //            
                    {NV_CHANNEL_DISABLE, 24},            //            
                    {NV_CHANNEL_DISABLE, 24},            //            
                    {NV_CHANNEL_DISABLE, 24},            //            
                    {NV_CHANNEL_DISABLE, 0},            //            
                    {NV_CHANNEL_DISABLE, 0},            //            
                    {NV_CHANNEL_DISABLE, 0},            //            
                    {NV_CHANNEL_DISABLE, 24},            //            
                    {NV_CHANNEL_DISABLE, 24},            //            
                    {NV_CHANNEL_DISABLE, 24},            //            

                    //                             
                    {NV_CHANNEL_ENABLE, 30},            //            
                    {NV_CHANNEL_ENABLE, 30},            //            
                    {NV_CHANNEL_ENABLE, 30},            //            
                    {NV_CHANNEL_ENABLE, 30},            //            
                    {NV_CHANNEL_ENABLE, 0},            //            

                    //                                    
                    {NV_CHANNEL_ENABLE, 22},            //               
                    {NV_CHANNEL_ENABLE, 30},            //               
                    {NV_CHANNEL_ENABLE, 30},            //               
                    {NV_CHANNEL_ENABLE, 30},            //               
                    {NV_CHANNEL_ENABLE, 30},            //               
                    {NV_CHANNEL_ENABLE, 30},            //               
                    {NV_CHANNEL_ENABLE, 22},            //               
                    {NV_CHANNEL_ENABLE, 0},            //                
                    {NV_CHANNEL_ENABLE, 0},            //                

                    //                                     
                    {NV_CHANNEL_DISABLE, 0},            //                 
                    {NV_CHANNEL_DISABLE, 0},            //                 
                    {NV_CHANNEL_DISABLE, 0},            //                 
                    {NV_CHANNEL_DISABLE, 0},            //                 
                    {NV_CHANNEL_DISABLE, 0},            //                 

                    //                                                  
                    {NV_CHANNEL_ENABLE, 20},            //                
                    {NV_CHANNEL_ENABLE, 20},            //                
                    {NV_CHANNEL_ENABLE, 20},            //                
                    {NV_CHANNEL_ENABLE, 27},            //                
                    {NV_CHANNEL_ENABLE, 27},            //                
                    {NV_CHANNEL_ENABLE, 27},            //                
                    {NV_CHANNEL_ENABLE, 25},            //                

                    //                                             
                    {NV_CHANNEL_DISABLE, 24},            //                
                    {NV_CHANNEL_DISABLE, 27},            //                
                    {NV_CHANNEL_DISABLE, 27},            //                
                    {NV_CHANNEL_DISABLE, 0},            //                
                    {NV_CHANNEL_DISABLE, 0},            //                
                    {NV_CHANNEL_DISABLE, 0},            //                
                    {NV_CHANNEL_DISABLE, 0},            //                
                    {NV_CHANNEL_DISABLE, 0},            //                
                    {NV_CHANNEL_DISABLE, 27},            //                
                    {NV_CHANNEL_DISABLE, 27},            //                

                    //                                              
                    {NV_CHANNEL_DISABLE, 30},            //                 
                    {NV_CHANNEL_DISABLE, 30},            //                 
                    {NV_CHANNEL_DISABLE, 30},            //                 
                    {NV_CHANNEL_DISABLE, 0},             //                
                }, //                      

                {
                    { 0 },  //                   
                    {0},   //                     
                    {0},   //                     
                    {0},   //                      
                    {0}    //                   
                },

                { //                       
                    //           
                    { 0 },                       //          
                    { 0 },                       //          
                    { 0 },                       //          
                    { 0 },                       //          
                    { 0 },                       //          
                    { 0 },                       //          
                    { 0 },                       //          
                    { 0 },                       //          
                    { 0 },                       //          
                    { 0 },                       //           
                    { 0 },                       //           
                    { 0 },                       //           
                    { 0 },                       //           
                    { 0 },                       //           
                }, //                     

                { //                        
                    //                                
                    { 0 },                       //          
                    { 0 },                       //          
                    { 0 },                       //          
                    { 0 },                       //          
                    { 0 },                       //          
                    { 0 },                       //          
                    { 0 },                       //          
                    { 0 },                       //          
                    { 0 },                       //          
                    { 0 },                       //           
                    { 0 },                       //           
                    { 0 },                       //           
                    { 0 },                       //           
                    { 0 },                       //           
                } //                      
            }, //                    

            {   //                       
                { //                        
                    //                 
                    //                    
                    {NV_CHANNEL_ENABLE, 15},           //          
                    {NV_CHANNEL_ENABLE, 15},           //          
                    {NV_CHANNEL_ENABLE, 15},           //          
                    {NV_CHANNEL_ENABLE, 15},           //          
                    {NV_CHANNEL_ENABLE, 15},           //          
                    {NV_CHANNEL_ENABLE, 15},           //          
                    {NV_CHANNEL_ENABLE, 15},           //          
                    {NV_CHANNEL_ENABLE, 15},           //          
                    {NV_CHANNEL_ENABLE, 15},           //          
                    {NV_CHANNEL_ENABLE, 15},           //           
                    {NV_CHANNEL_ENABLE, 15},           //           
                    {NV_CHANNEL_ENABLE, 15},           //           
                    {NV_CHANNEL_ENABLE, 15},           //           
                    {NV_CHANNEL_DISABLE, 0},           //           

                    //                    
                    {NV_CHANNEL_DISABLE, 23},            //            
                    {NV_CHANNEL_DISABLE, 23},            //            
                    {NV_CHANNEL_DISABLE, 23},            //            
                    {NV_CHANNEL_DISABLE, 23},            //            
                    {NV_CHANNEL_DISABLE, 23},            //            
                    {NV_CHANNEL_DISABLE, 23},            //            
                    {NV_CHANNEL_DISABLE, 23},            //            

                    //                                  
                    {NV_CHANNEL_ENABLE, 17},            //           
                    {NV_CHANNEL_ENABLE, 17},            //           
                    {NV_CHANNEL_ENABLE, 17},            //           
                    {NV_CHANNEL_ENABLE, 17},            //           
                    {NV_CHANNEL_DISABLE, 24},            //           
                    {NV_CHANNEL_DISABLE, 24},            //           
                    {NV_CHANNEL_DISABLE, 24},            //           
                    {NV_CHANNEL_DISABLE, 24},            //           

                    //                             
                    {NV_CHANNEL_DISABLE, 22},            //            
                    {NV_CHANNEL_DISABLE, 24},            //            
                    {NV_CHANNEL_DISABLE, 24},            //            
                    {NV_CHANNEL_DISABLE, 24},            //            
                    {NV_CHANNEL_DISABLE, 24},            //            
                    {NV_CHANNEL_DISABLE, 0},            //            
                    {NV_CHANNEL_DISABLE, 0},            //            
                    {NV_CHANNEL_DISABLE, 0},            //            
                    {NV_CHANNEL_DISABLE, 24},            //            
                    {NV_CHANNEL_DISABLE, 24},            //            
                    {NV_CHANNEL_DISABLE, 24},            //            

                    //                             
                    {NV_CHANNEL_ENABLE, 30},            //            
                    {NV_CHANNEL_ENABLE, 30},            //            
                    {NV_CHANNEL_ENABLE, 30},            //            
                    {NV_CHANNEL_ENABLE, 30},            //            
                    {NV_CHANNEL_ENABLE, 0},            //            

                    //                                    
                    {NV_CHANNEL_ENABLE, 22},            //               
                    {NV_CHANNEL_ENABLE, 30},            //               
                    {NV_CHANNEL_ENABLE, 30},            //               
                    {NV_CHANNEL_ENABLE, 30},            //               
                    {NV_CHANNEL_ENABLE, 30},            //               
                    {NV_CHANNEL_ENABLE, 30},            //               
                    {NV_CHANNEL_ENABLE, 22},            //               
                    {NV_CHANNEL_ENABLE, 0},            //                
                    {NV_CHANNEL_ENABLE, 0},            //                

                    //                                     
                    {NV_CHANNEL_DISABLE, 0},            //                 
                    {NV_CHANNEL_DISABLE, 0},            //                 
                    {NV_CHANNEL_DISABLE, 0},            //                 
                    {NV_CHANNEL_DISABLE, 0},            //                 
                    {NV_CHANNEL_DISABLE, 0},            //                 

                    //                                                  
                    {NV_CHANNEL_ENABLE, 20},            //                
                    {NV_CHANNEL_ENABLE, 20},            //                
                    {NV_CHANNEL_ENABLE, 20},            //                
                    {NV_CHANNEL_ENABLE, 27},            //                
                    {NV_CHANNEL_ENABLE, 27},            //                
                    {NV_CHANNEL_ENABLE, 27},            //                
                    {NV_CHANNEL_ENABLE, 25},            //                

                    //                                             
                    {NV_CHANNEL_DISABLE, 24},            //                
                    {NV_CHANNEL_DISABLE, 27},            //                
                    {NV_CHANNEL_DISABLE, 27},            //                
                    {NV_CHANNEL_DISABLE, 0},            //                
                    {NV_CHANNEL_DISABLE, 0},            //                
                    {NV_CHANNEL_DISABLE, 0},            //                
                    {NV_CHANNEL_DISABLE, 0},            //                
                    {NV_CHANNEL_DISABLE, 0},            //                
                    {NV_CHANNEL_DISABLE, 27},            //                
                    {NV_CHANNEL_DISABLE, 27},            //                

                    //                                              
                    {NV_CHANNEL_DISABLE, 30},            //                 
                    {NV_CHANNEL_DISABLE, 30},            //                 
                    {NV_CHANNEL_DISABLE, 30},            //                 
                    {NV_CHANNEL_DISABLE, 0},             //                
                }, //                      

                {
                    { 0 },  //                   
                    {0},   //                     
                    {0},   //                     
                    {0},   //                      
                    {0}    //                   
                },

                { //                       
                    //           
                    { 0 },                       //          
                    { 0 },                       //          
                    { 0 },                       //          
                    { 0 },                       //          
                    { 0 },                       //          
                    { 0 },                       //          
                    { 0 },                       //          
                    { 0 },                       //          
                    { 0 },                       //          
                    { 0 },                       //           
                    { 0 },                       //           
                    { 0 },                       //           
                    { 0 },                       //           
                    { 0 },                       //           
                }, //                     

                { //                        
                    //                                
                    { 0 },                       //          
                    { 0 },                       //          
                    { 0 },                       //          
                    { 0 },                       //          
                    { 0 },                       //          
                    { 0 },                       //          
                    { 0 },                       //          
                    { 0 },                       //          
                    { 0 },                       //          
                    { 0 },                       //           
                    { 0 },                       //           
                    { 0 },                       //           
                    { 0 },                       //           
                    { 0 },                       //           
                } //                      
            }, //                     

            {   //                         
                { //                        
                    //                 
                    //                    
                    {NV_CHANNEL_ENABLE, 14},           //          
                    {NV_CHANNEL_ENABLE, 14},           //          
                    {NV_CHANNEL_ENABLE, 14},           //          
                    {NV_CHANNEL_ENABLE, 14},           //          
                    {NV_CHANNEL_ENABLE, 14},           //          
                    {NV_CHANNEL_ENABLE, 14},           //          
                    {NV_CHANNEL_ENABLE, 14},           //          
                    {NV_CHANNEL_ENABLE, 14},           //          
                    {NV_CHANNEL_ENABLE, 14},           //          
                    {NV_CHANNEL_ENABLE, 14},           //           
                    {NV_CHANNEL_ENABLE, 14},           //           
                    {NV_CHANNEL_ENABLE, 14},           //           
                    {NV_CHANNEL_ENABLE, 14},           //           
                    {NV_CHANNEL_DISABLE, 0},           //           

                    //                    
                    {NV_CHANNEL_DISABLE, 23},            //            
                    {NV_CHANNEL_DISABLE, 23},            //            
                    {NV_CHANNEL_DISABLE, 23},            //            
                    {NV_CHANNEL_DISABLE, 23},            //            
                    {NV_CHANNEL_DISABLE, 23},            //            
                    {NV_CHANNEL_DISABLE, 23},            //            
                    {NV_CHANNEL_DISABLE, 23},            //            

                    //                                  
                    {NV_CHANNEL_ENABLE, 17},            //           
                    {NV_CHANNEL_ENABLE, 17},            //           
                    {NV_CHANNEL_ENABLE, 17},            //           
                    {NV_CHANNEL_ENABLE, 17},            //           
                    {NV_CHANNEL_DISABLE, 24},            //           
                    {NV_CHANNEL_DISABLE, 24},            //           
                    {NV_CHANNEL_DISABLE, 24},            //           
                    {NV_CHANNEL_DISABLE, 24},            //           

                    //                             
                    {NV_CHANNEL_DISABLE, 22},            //            
                    {NV_CHANNEL_DISABLE, 24},            //            
                    {NV_CHANNEL_DISABLE, 24},            //            
                    {NV_CHANNEL_DISABLE, 24},            //            
                    {NV_CHANNEL_DISABLE, 24},            //            
                    {NV_CHANNEL_DISABLE, 0},            //            
                    {NV_CHANNEL_DISABLE, 0},            //            
                    {NV_CHANNEL_DISABLE, 0},            //            
                    {NV_CHANNEL_DISABLE, 24},            //            
                    {NV_CHANNEL_DISABLE, 24},            //            
                    {NV_CHANNEL_DISABLE, 24},            //            

                    //                             
                    {NV_CHANNEL_ENABLE, 30},            //            
                    {NV_CHANNEL_ENABLE, 30},            //            
                    {NV_CHANNEL_ENABLE, 30},            //            
                    {NV_CHANNEL_ENABLE, 30},            //            
                    {NV_CHANNEL_ENABLE, 0},            //            

                    //                                    
                    {NV_CHANNEL_ENABLE, 22},            //               
                    {NV_CHANNEL_ENABLE, 30},            //               
                    {NV_CHANNEL_ENABLE, 30},            //               
                    {NV_CHANNEL_ENABLE, 30},            //               
                    {NV_CHANNEL_ENABLE, 30},            //               
                    {NV_CHANNEL_ENABLE, 30},            //               
                    {NV_CHANNEL_ENABLE, 22},            //               
                    {NV_CHANNEL_ENABLE, 0},            //                
                    {NV_CHANNEL_ENABLE, 0},            //                

                    //                                     
                    {NV_CHANNEL_DISABLE, 0},            //                 
                    {NV_CHANNEL_DISABLE, 0},            //                 
                    {NV_CHANNEL_DISABLE, 0},            //                 
                    {NV_CHANNEL_DISABLE, 0},            //                 
                    {NV_CHANNEL_DISABLE, 0},            //                 

                    //                                                  
                    {NV_CHANNEL_ENABLE, 20},            //                
                    {NV_CHANNEL_ENABLE, 20},            //                
                    {NV_CHANNEL_ENABLE, 20},            //                
                    {NV_CHANNEL_ENABLE, 27},            //                
                    {NV_CHANNEL_ENABLE, 27},            //                
                    {NV_CHANNEL_ENABLE, 27},            //                
                    {NV_CHANNEL_ENABLE, 25},            //                

                    //                                             
                    {NV_CHANNEL_DISABLE, 24},            //                
                    {NV_CHANNEL_DISABLE, 27},            //                
                    {NV_CHANNEL_DISABLE, 27},            //                
                    {NV_CHANNEL_DISABLE, 0},            //                
                    {NV_CHANNEL_DISABLE, 0},            //                
                    {NV_CHANNEL_DISABLE, 0},            //                
                    {NV_CHANNEL_DISABLE, 0},            //                
                    {NV_CHANNEL_DISABLE, 0},            //                
                    {NV_CHANNEL_DISABLE, 27},            //                
                    {NV_CHANNEL_DISABLE, 27},            //                

                    //                                              
                    {NV_CHANNEL_DISABLE, 30},            //                 
                    {NV_CHANNEL_DISABLE, 30},            //                 
                    {NV_CHANNEL_DISABLE, 30},            //                 
                    {NV_CHANNEL_DISABLE, 0},             //                
                }, //                      

                {
                    { 0 },  //                   
                    {0},   //                     
                    {0},   //                     
                    {0},   //                      
                    {0}    //                   
                },

                { //                       
                    //           
                    { 0 },                       //          
                    { 0 },                       //          
                    { 0 },                       //          
                    { 0 },                       //          
                    { 0 },                       //          
                    { 0 },                       //          
                    { 0 },                       //          
                    { 0 },                       //          
                    { 0 },                       //          
                    { 0 },                       //           
                    { 0 },                       //           
                    { 0 },                       //           
                    { 0 },                       //           
                    { 0 },                       //           
                }, //                     

                { //                        
                    //                                
                    { 0 },                       //          
                    { 0 },                       //          
                    { 0 },                       //          
                    { 0 },                       //          
                    { 0 },                       //          
                    { 0 },                       //          
                    { 0 },                       //          
                    { 0 },                       //          
                    { 0 },                       //          
                    { 0 },                       //           
                    { 0 },                       //           
                    { 0 },                       //           
                    { 0 },                       //           
                    { 0 },                       //           
                } //                      
            }, //                       

            {   //                         
                { //                        
                    //                 
                                       //           
                    {NV_CHANNEL_ENABLE, 12},           //          
                    {NV_CHANNEL_ENABLE, 12},           //          
                    {NV_CHANNEL_ENABLE, 12},           //          
                    {NV_CHANNEL_ENABLE, 12},           //          
                    {NV_CHANNEL_ENABLE, 12},           //          
                    {NV_CHANNEL_ENABLE, 12},           //          
                    {NV_CHANNEL_ENABLE, 12},           //          
                    {NV_CHANNEL_ENABLE, 12},           //          
                    {NV_CHANNEL_ENABLE, 12},           //          
                    {NV_CHANNEL_ENABLE, 12},           //           
                    {NV_CHANNEL_ENABLE, 12},           //           
                    {NV_CHANNEL_ENABLE, 12},           //           
                    {NV_CHANNEL_ENABLE, 12},           //           
                    {NV_CHANNEL_DISABLE, 0},           //           
                }, //                      

                {
                    { 0 },  //                   
                    {0},   //                     
                    {0},   //                     
                    {0},   //                      
                    {0}    //                   
                },

                { //                       
                    //           
                    { 0 },                       //          
                    { 0 },                       //          
                    { 0 },                       //          
                    { 0 },                       //          
                    { 0 },                       //          
                    { 0 },                       //          
                    { 0 },                       //          
                    { 0 },                       //          
                    { 0 },                       //          
                    { 0 },                       //           
                    { 0 },                       //           
                    { 0 },                       //           
                    { 0 },                       //           
                    { 0 },                       //           
                }, //                     

                { //                        
                    //                                
                    { 0 },                       //          
                    { 0 },                       //          
                    { 0 },                       //          
                    { 0 },                       //          
                    { 0 },                       //          
                    { 0 },                       //          
                    { 0 },                       //          
                    { 0 },                       //          
                    { 0 },                       //          
                    { 0 },                       //           
                    { 0 },                       //           
                    { 0 },                       //           
                    { 0 },                       //           
                    { 0 },                       //           
                } //                      
            } //                       
        },

        //                         
        {
            //               
            //  
            //                                                                                
            //                                                                             
            //                  

            0,                  //          
            { 'U', 'S', 'I' }   //            
        },

        //                        
        {
            {
                {
                    0  , // 
                    41 , // 
                    43 , // 
                    45 , // 
                    47 , // 
                    49 , // 
                    51 , // 
                    53 , // 
                    55 , // 
                    56 , // 
                    58 , //  
                    59 , //  
                    60 , //  
                    62 , //  
                    63 , //  
                    64 , //  
                    65 , //  
                    67 , //  
                    68 , //  
                    69 , //  
                    70 , //  
                    71 , //  
                    72 , //  
                    73 , //  
                    74 , //  
                    75 , //  
                    75 , //  
                    76 , //  
                    77 , //  
                    78 , //  
                    78 , //  
                    79 , //  
                    80 , //  
                    81 , //  
                    82 , //  
                    82 , //  
                    83 , //  
                    83 , //  
                    84 , //  
                    85 , //  
                    86 , //  
                    86 , //  
                    87 , //  
                    88 , //  
                    89 , //  
                    89 , //  
                    90 , //  
                    91 , //  
                    91 , //  
                    92 , //  
                    92 , //  
                    93 , //  
                    93 , //  
                    94 , //  
                    94 , //  
                    95 , //  
                    95 , //  
                    95 , //  
                    96 , //  
                    96 , //  
                    97 , //  
                    97 , //  
                    98 , //  
                    98 , //  
                    98 , //  
                    99 , //  
                    99 , //  
                    99 , //  
                    100, //  
                    100, //  
                    100, //  
                    101, //  
                    101, //  
                    102, //  
                    102, //  
                    102, //  
                    102, //  
                    103, //  
                    103, //  
                    103, //  
                    103, //  
                    104, //  
                    104, //  
                    104, //  
                    104, //  
                    105, //  
                    105, //  
                    105, //  
                    105, //  
                    105, //  
                    106, //  
                    106, //  
                    106, //  
                    106, //  
                    106, //  
                    106, //  
                    106, //  
                    106, //  
                    106, //  
                    106, //  
                    106, //   
                    106, //   
                    106, //   
                    106, //   
                    106, //   
                    106, //   
                    107, //   
                    107, //   
                    107, //   
                    107, //   
                    107, //   
                    107, //   
                    107, //   
                    107, //   
                    107, //   
                    107, //   
                    107, //   
                    107, //   
                    107, //   
                    107, //   
                    107, //   
                    107, //   
                    107, //   
                    107, //   
                    107, //   
                    107, //   
                    107, //   
                    107, //   
                    107,
                }
            }, //         
            {
                {
                    0  , // 
                    41 , // 
                    43 , // 
                    45 , // 
                    47 , // 
                    49 , // 
                    51 , // 
                    52 , // 
                    54 , // 
                    56 , // 
                    57 , //  
                    59 , //  
                    60 , //  
                    61 , //  
                    62 , //  
                    64 , //  
                    65 , //  
                    66 , //  
                    67 , //  
                    68 , //  
                    69 , //  
                    70 , //  
                    71 , //  
                    72 , //  
                    73 , //  
                    74 , //  
                    75 , //  
                    75 , //  
                    76 , //  
                    77 , //  
                    78 , //  
                    79 , //  
                    79 , //  
                    80 , //  
                    81 , //  
                    82 , //  
                    82 , //  
                    83 , //  
                    84 , //  
                    85 , //  
                    85 , //  
                    86 , //  
                    87 , //  
                    88 , //  
                    88 , //  
                    89 , //  
                    89 , //  
                    90 , //  
                    91 , //  
                    91 , //  
                    92 , //  
                    92 , //  
                    93 , //  
                    93 , //  
                    94 , //  
                    94 , //  
                    95 , //  
                    95 , //  
                    96 , //  
                    96 , //  
                    96 , //  
                    97 , //  
                    97 , //  
                    98 , //  
                    98 , //  
                    98 , //  
                    99 , //  
                    99 , //  
                    99 , //  
                    100, //  
                    100, //  
                    101, //  
                    101, //  
                    101, //  
                    101, //  
                    102, //  
                    102, //  
                    102, //  
                    103, //  
                    103, //  
                    103, //  
                    104, //  
                    104, //  
                    104, //  
                    104, //  
                    105, //  
                    105, //  
                    105, //  
                    105, //  
                    105, //  
                    106, //  
                    106, //  
                    106, //  
                    106, //  
                    106, //  
                    106, //  
                    106, //  
                    106, //  
                    106, //  
                    106, //  
                    106, //   
                    106, //   
                    106, //   
                    107, //   
                    107, //   
                    107, //   
                    107, //   
                    107, //   
                    107, //   
                    107, //   
                    107, //   
                    107, //   
                    107, //   
                    107, //   
                    107, //   
                    107, //   
                    107, //   
                    107, //   
                    107, //   
                    107, //   
                    107, //   
                    107, //   
                    107, //   
                    107, //   
                    107, //   
                    107, //   
                    107, //   
                    107, //   
                    107,
                }
            }, //         
                {
                    {
                        0  , // 
                        41 , // 
                        43 , // 
                        45 , // 
                        47 , // 
                        49 , // 
                        51 , // 
                        52 , // 
                        54 , // 
                        55 , // 
                        57 , //  
                        58 , //  
                        60 , //  
                        61 , //  
                        62 , //  
                        64 , //  
                        65 , //  
                        66 , //  
                        67 , //  
                        68 , //  
                        69 , //  
                        70 , //  
                        71 , //  
                        72 , //  
                        73 , //  
                        74 , //  
                        75 , //  
                        75 , //  
                        76 , //  
                        77 , //  
                        78 , //  
                        78 , //  
                        79 , //  
                        80 , //  
                        81 , //  
                        82 , //  
                        82 , //  
                        83 , //  
                        84 , //  
                        84 , //  
                        85 , //  
                        86 , //  
                        87 , //  
                        87 , //  
                        88 , //  
                        89 , //  
                        89 , //  
                        90 , //  
                        90 , //  
                        91 , //  
                        91 , //  
                        92 , //  
                        93 , //  
                        93 , //  
                        94 , //  
                        94 , //  
                        94 , //  
                        95 , //  
                        95 , //  
                        96 , //  
                        96 , //  
                        97 , //  
                        97 , //  
                        97 , //  
                        98 , //  
                        98 , //  
                        99 , //  
                        99 , //  
                        99 , //  
                        100, //  
                        100, //  
                        100, //  
                        101, //  
                        101, //  
                        101, //  
                        102, //  
                        102, //  
                        102, //  
                        103, //  
                        103, //  
                        103, //  
                        103, //  
                        104, //  
                        104, //  
                        104, //  
                        104, //  
                        104, //  
                        105, //  
                        105, //  
                        105, //  
                        105, //  
                        105, //  
                        105, //  
                        105, //  
                        105, //  
                        105, //  
                        105, //  
                        105, //  
                        105, //  
                        106, //  
                        106, //   
                        106, //   
                        106, //   
                        106, //   
                        106, //   
                        106, //   
                        106, //   
                        106, //   
                        106, //   
                        106, //   
                        106, //   
                        106, //   
                        106, //   
                        106, //   
                        106, //   
                        106, //   
                        106, //   
                        106, //   
                        106, //   
                        106, //   
                        106, //   
                        106, //   
                        106, //   
                        106, //   
                        106, //   
                        106, //   
                        106, //   
                        106, //   
                        107,
                    }
                }, //         
                {
                    {
                        0  , // 
                        42 , // 
                        44 , // 
                        46 , // 
                        48 , // 
                        49 , // 
                        51 , // 
                        53 , // 
                        55 , // 
                        57 , // 
                        58 , //  
                        60 , //  
                        61 , //  
                        62 , //  
                        63 , //  
                        64 , //  
                        66 , //  
                        67 , //  
                        68 , //  
                        69 , //  
                        70 , //  
                        71 , //  
                        72 , //  
                        73 , //  
                        74 , //  
                        75 , //  
                        75 , //  
                        76 , //  
                        77 , //  
                        78 , //  
                        78 , //  
                        79 , //  
                        80 , //  
                        81 , //  
                        82 , //  
                        82 , //  
                        83 , //  
                        84 , //  
                        84 , //  
                        85 , //  
                        86 , //  
                        87 , //  
                        87 , //  
                        88 , //  
                        88 , //  
                        89 , //  
                        90 , //  
                        90 , //  
                        91 , //  
                        91 , //  
                        92 , //  
                        92 , //  
                        93 , //  
                        93 , //  
                        94 , //  
                        94 , //  
                        95 , //  
                        95 , //  
                        95 , //  
                        96 , //  
                        96 , //  
                        97 , //  
                        97 , //  
                        98 , //  
                        98 , //  
                        98 , //  
                        99 , //  
                        99 , //  
                        99 , //  
                        100, //  
                        100, //  
                        100, //  
                        101, //  
                        101, //  
                        101, //  
                        102, //  
                        102, //  
                        102, //  
                        103, //  
                        103, //  
                        103, //  
                        103, //  
                        104, //  
                        104, //  
                        104, //  
                        104, //  
                        104, //  
                        104, //  
                        104, //  
                        104, //  
                        105, //  
                        105, //  
                        105, //  
                        105, //  
                        105, //  
                        105, //  
                        105, //  
                        105, //  
                        105, //  
                        105, //  
                        105, //   
                        105, //   
                        105, //   
                        105, //   
                        105, //   
                        106, //   
                        106, //   
                        106, //   
                        106, //   
                        106, //   
                        106, //   
                        106, //   
                        106, //   
                        106, //   
                        106, //   
                        106, //   
                        106, //   
                        106, //   
                        106, //   
                        106, //   
                        106, //   
                        106, //   
                        106, //   
                        106, //   
                        106, //   
                        106, //   
                        106, //   
                        106, //   
                        106,
                    }
                }, //         
                {
                    {
                        0  , // 
                        41 , // 
                        43 , // 
                        45 , // 
                        47 , // 
                        49 , // 
                        51 , // 
                        53 , // 
                        54 , // 
                        56 , // 
                        57 , //  
                        59 , //  
                        60 , //  
                        62 , //  
                        63 , //  
                        65 , //  
                        66 , //  
                        67 , //  
                        68 , //  
                        69 , //  
                        69 , //  
                        71 , //  
                        72 , //  
                        72 , //  
                        73 , //  
                        74 , //  
                        75 , //  
                        76 , //  
                        77 , //  
                        78 , //  
                        79 , //  
                        79 , //  
                        80 , //  
                        81 , //  
                        82 , //  
                        83 , //  
                        83 , //  
                        84 , //  
                        85 , //  
                        86 , //  
                        87 , //  
                        87 , //  
                        88 , //  
                        89 , //  
                        89 , //  
                        90 , //  
                        91 , //  
                        91 , //  
                        92 , //  
                        92 , //  
                        93 , //  
                        93 , //  
                        94 , //  
                        94 , //  
                        95 , //  
                        95 , //  
                        96 , //  
                        96 , //  
                        96 , //  
                        97 , //  
                        97 , //  
                        98 , //  
                        98 , //  
                        98 , //  
                        99 , //  
                        99 , //  
                        100, //  
                        100, //  
                        100, //  
                        101, //  
                        101, //  
                        101, //  
                        102, //  
                        102, //  
                        102, //  
                        103, //  
                        103, //  
                        103, //  
                        103, //  
                        104, //  
                        104, //  
                        104, //  
                        104, //  
                        105, //  
                        105, //  
                        105, //  
                        105, //  
                        105, //  
                        105, //  
                        105, //  
                        105, //  
                        105, //  
                        106, //  
                        106, //  
                        106, //  
                        106, //  
                        106, //  
                        106, //  
                        106, //  
                        106, //  
                        106, //   
                        106, //   
                        106, //   
                        106, //   
                        106, //   
                        106, //   
                        106, //   
                        106, //   
                        106, //   
                        106, //   
                        106, //   
                        106, //   
                        106, //   
                        106, //   
                        106, //   
                        106, //   
                        106, //   
                        106, //   
                        106, //   
                        106, //   
                        106, //   
                        106, //   
                        106, //   
                        106, //   
                        106, //   
                        106, //   
                        106, //   
                        106, //   
                        106,
                    }
                }, //         
                {
                    {
                        0  , // 
                        41 , // 
                        43 , // 
                        45 , // 
                        47 , // 
                        49 , // 
                        51 , // 
                        53 , // 
                        55 , // 
                        56 , // 
                        58 , //  
                        59 , //  
                        61 , //  
                        62 , //  
                        63 , //  
                        64 , //  
                        65 , //  
                        66 , //  
                        68 , //  
                        69 , //  
                        70 , //  
                        71 , //  
                        72 , //  
                        73 , //  
                        74 , //  
                        75 , //  
                        76 , //  
                        77 , //  
                        77 , //  
                        78 , //  
                        79 , //  
                        80 , //  
                        80 , //  
                        81 , //  
                        82 , //  
                        83 , //  
                        83 , //  
                        84 , //  
                        85 , //  
                        86 , //  
                        87 , //  
                        87 , //  
                        88 , //  
                        89 , //  
                        89 , //  
                        90 , //  
                        91 , //  
                        91 , //  
                        92 , //  
                        92 , //  
                        93 , //  
                        93 , //  
                        94 , //  
                        94 , //  
                        95 , //  
                        95 , //  
                        96 , //  
                        96 , //  
                        97 , //  
                        97 , //  
                        98 , //  
                        98 , //  
                        98 , //  
                        99 , //  
                        99 , //  
                        100, //  
                        100, //  
                        100, //  
                        101, //  
                        101, //  
                        101, //  
                        102, //  
                        102, //  
                        102, //  
                        103, //  
                        103, //  
                        103, //  
                        103, //  
                        104, //  
                        104, //  
                        104, //  
                        104, //  
                        104, //  
                        105, //  
                        105, //  
                        105, //  
                        105, //  
                        105, //  
                        105, //  
                        105, //  
                        106, //  
                        106, //  
                        106, //  
                        106, //  
                        106, //  
                        106, //  
                        106, //  
                        106, //  
                        106, //  
                        106, //  
                        106, //   
                        106, //   
                        106, //   
                        106, //   
                        106, //   
                        106, //   
                        106, //   
                        106, //   
                        106, //   
                        106, //   
                        106, //   
                        107, //   
                        107, //   
                        107, //   
                        107, //   
                        107, //   
                        107, //   
                        107, //   
                        107, //   
                        107, //   
                        107, //   
                        107, //   
                        107, //   
                        107, //   
                        107, //   
                        107, //   
                        107, //   
                        107, //   
                        107,
                    }
                }, //         
                {
                    {
                        0  , // 
                        41 , // 
                        43 , // 
                        45 , // 
                        47 , // 
                        49 , // 
                        51 , // 
                        53 , // 
                        55 , // 
                        56 , // 
                        58 , //  
                        60 , //  
                        61 , //  
                        62 , //  
                        63 , //  
                        64 , //  
                        66 , //  
                        67 , //  
                        68 , //  
                        69 , //  
                        70 , //  
                        71 , //  
                        72 , //  
                        73 , //  
                        74 , //  
                        75 , //  
                        76 , //  
                        77 , //  
                        77 , //  
                        78 , //  
                        79 , //  
                        80 , //  
                        80 , //  
                        81 , //  
                        82 , //  
                        83 , //  
                        84 , //  
                        84 , //  
                        85 , //  
                        86 , //  
                        87 , //  
                        87 , //  
                        88 , //  
                        88 , //  
                        89 , //  
                        90 , //  
                        90 , //  
                        91 , //  
                        91 , //  
                        92 , //  
                        92 , //  
                        93 , //  
                        93 , //  
                        94 , //  
                        94 , //  
                        95 , //  
                        95 , //  
                        96 , //  
                        96 , //  
                        97 , //  
                        97 , //  
                        97 , //  
                        98 , //  
                        98 , //  
                        99 , //  
                        99 , //  
                        99 , //  
                        100, //  
                        100, //  
                        100, //  
                        101, //  
                        101, //  
                        101, //  
                        102, //  
                        102, //  
                        102, //  
                        103, //  
                        103, //  
                        103, //  
                        103, //  
                        104, //  
                        104, //  
                        104, //  
                        104, //  
                        104, //  
                        104, //  
                        105, //  
                        105, //  
                        105, //  
                        105, //  
                        105, //  
                        105, //  
                        105, //  
                        105, //  
                        105, //  
                        105, //  
                        105, //  
                        105, //  
                        106, //  
                        106, //  
                        106, //   
                        106, //   
                        106, //   
                        106, //   
                        106, //   
                        106, //   
                        106, //   
                        106, //   
                        106, //   
                        106, //   
                        106, //   
                        106, //   
                        106, //   
                        106, //   
                        106, //   
                        106, //   
                        106, //   
                        106, //   
                        106, //   
                        106, //   
                        106, //   
                        106, //   
                        106, //   
                        106, //   
                        106, //   
                        106, //   
                        106, //   
                        106, //   
                        106,
                    }
                }, //         
                {
                    {
                        0  , // 
                        40 , // 
                        42 , // 
                        45 , // 
                        47 , // 
                        49 , // 
                        51 , // 
                        52 , // 
                        54 , // 
                        56 , // 
                        58 , //  
                        59 , //  
                        61 , //  
                        62 , //  
                        63 , //  
                        65 , //  
                        66 , //  
                        67 , //  
                        68 , //  
                        69 , //  
                        70 , //  
                        71 , //  
                        72 , //  
                        73 , //  
                        74 , //  
                        75 , //  
                        76 , //  
                        77 , //  
                        77 , //  
                        78 , //  
                        79 , //  
                        80 , //  
                        81 , //  
                        81 , //  
                        82 , //  
                        83 , //  
                        84 , //  
                        85 , //  
                        86 , //  
                        86 , //  
                        87 , //  
                        88 , //  
                        89 , //  
                        89 , //  
                        90 , //  
                        91 , //  
                        91 , //  
                        92 , //  
                        92 , //  
                        93 , //  
                        93 , //  
                        94 , //  
                        94 , //  
                        95 , //  
                        95 , //  
                        96 , //  
                        96 , //  
                        97 , //  
                        97 , //  
                        97 , //  
                        98 , //  
                        98 , //  
                        99 , //  
                        99 , //  
                        99 , //  
                        100, //  
                        100, //  
                        100, //  
                        101, //  
                        101, //  
                        102, //  
                        102, //  
                        102, //  
                        103, //  
                        103, //  
                        103, //  
                        104, //  
                        104, //  
                        104, //  
                        104, //  
                        105, //  
                        105, //  
                        105, //  
                        105, //  
                        105, //  
                        105, //  
                        105, //  
                        105, //  
                        106, //  
                        106, //  
                        106, //  
                        106, //  
                        106, //  
                        106, //  
                        106, //  
                        106, //  
                        106, //  
                        106, //  
                        106, //  
                        106, //  
                        106, //   
                        106, //   
                        106, //   
                        106, //   
                        106, //   
                        107, //   
                        107, //   
                        107, //   
                        107, //   
                        107, //   
                        107, //   
                        107, //   
                        107, //   
                        107, //   
                        107, //   
                        107, //   
                        107, //   
                        107, //   
                        107, //   
                        107, //   
                        107, //   
                        107, //   
                        107, //   
                        107, //   
                        107, //   
                        107, //   
                        107, //   
                        107, //   
                        107,
                    }
                }, //         
                {
                    {
                        0  , // 
                        41 , // 
                        44 , // 
                        46 , // 
                        48 , // 
                        50 , // 
                        52 , // 
                        54 , // 
                        56 , // 
                        58 , // 
                        59 , //  
                        60 , //  
                        62 , //  
                        63 , //  
                        64 , //  
                        66 , //  
                        67 , //  
                        68 , //  
                        69 , //  
                        70 , //  
                        71 , //  
                        72 , //  
                        73 , //  
                        74 , //  
                        75 , //  
                        76 , //  
                        77 , //  
                        78 , //  
                        79 , //  
                        79 , //  
                        80 , //  
                        81 , //  
                        82 , //  
                        83 , //  
                        83 , //  
                        84 , //  
                        85 , //  
                        86 , //  
                        87 , //  
                        87 , //  
                        88 , //  
                        89 , //  
                        89 , //  
                        90 , //  
                        91 , //  
                        91 , //  
                        92 , //  
                        92 , //  
                        93 , //  
                        93 , //  
                        94 , //  
                        94 , //  
                        95 , //  
                        95 , //  
                        96 , //  
                        96 , //  
                        97 , //  
                        97 , //  
                        98 , //  
                        98 , //  
                        98 , //  
                        99 , //  
                        99 , //  
                        100, //  
                        100, //  
                        100, //  
                        101, //  
                        101, //  
                        101, //  
                        102, //  
                        102, //  
                        103, //  
                        103, //  
                        103, //  
                        104, //  
                        104, //  
                        104, //  
                        104, //  
                        105, //  
                        105, //  
                        105, //  
                        105, //  
                        105, //  
                        105, //  
                        106, //  
                        106, //  
                        106, //  
                        106, //  
                        106, //  
                        106, //  
                        106, //  
                        106, //  
                        106, //  
                        106, //  
                        106, //  
                        106, //  
                        106, //  
                        106, //  
                        106, //  
                        107, //  
                        107, //   
                        107, //   
                        107, //   
                        107, //   
                        107, //   
                        107, //   
                        107, //   
                        107, //   
                        107, //   
                        107, //   
                        107, //   
                        107, //   
                        107, //   
                        107, //   
                        107, //   
                        107, //   
                        107, //   
                        107, //   
                        107, //   
                        107, //   
                        107, //   
                        107, //   
                        107, //   
                        107, //   
                        107, //   
                        107, //   
                        107, //   
                        107, //   
                        107,
                    }
                }, //         
                {
                    {
                        0  , // 
                        41 , // 
                        43 , // 
                        47 , // 
                        48 , // 
                        50 , // 
                        52 , // 
                        53 , // 
                        55 , // 
                        57 , // 
                        58 , //  
                        60 , //  
                        62 , //  
                        63 , //  
                        64 , //  
                        65 , //  
                        67 , //  
                        68 , //  
                        69 , //  
                        70 , //  
                        71 , //  
                        72 , //  
                        73 , //  
                        74 , //  
                        75 , //  
                        76 , //  
                        77 , //  
                        77 , //  
                        78 , //  
                        79 , //  
                        80 , //  
                        81 , //  
                        82 , //  
                        83 , //  
                        84 , //  
                        85 , //  
                        85 , //  
                        86 , //  
                        87 , //  
                        88 , //  
                        89 , //  
                        89 , //  
                        90 , //  
                        90 , //  
                        91 , //  
                        92 , //  
                        92 , //  
                        93 , //  
                        94 , //  
                        94 , //  
                        95 , //  
                        95 , //  
                        96 , //  
                        96 , //  
                        96 , //  
                        97 , //  
                        97 , //  
                        98 , //  
                        98 , //  
                        99 , //  
                        99 , //  
                        99 , //  
                        100, //  
                        100, //  
                        101, //  
                        101, //  
                        102, //  
                        102, //  
                        102, //  
                        103, //  
                        103, //  
                        103, //  
                        104, //  
                        104, //  
                        104, //  
                        105, //  
                        105, //  
                        105, //  
                        105, //  
                        105, //  
                        106, //  
                        106, //  
                        106, //  
                        106, //  
                        106, //  
                        106, //  
                        106, //  
                        106, //  
                        106, //  
                        107, //  
                        107, //  
                        107, //  
                        107, //  
                        107, //  
                        107, //  
                        107, //  
                        107, //  
                        107, //  
                        107, //  
                        107, //  
                        107, //   
                        107, //   
                        107, //   
                        107, //   
                        107, //   
                        107, //   
                        107, //   
                        107, //   
                        107, //   
                        107, //   
                        107, //   
                        107, //   
                        107, //   
                        107, //   
                        107, //   
                        107, //   
                        107, //   
                        107, //   
                        107, //   
                        107, //   
                        107, //   
                        107, //   
                        107, //   
                        107, //   
                        107, //   
                        107, //   
                        107, //   
                        107, //   
                        107,
                    }
                }, //          
                {
                    {
                        0  , // 
                        42 , // 
                        44 , // 
                        47 , // 
                        49 , // 
                        51 , // 
                        52 , // 
                        54 , // 
                        55 , // 
                        57 , // 
                        58 , //  
                        60 , //  
                        61 , //  
                        63 , //  
                        64 , //  
                        65 , //  
                        66 , //  
                        67 , //  
                        69 , //  
                        70 , //  
                        71 , //  
                        72 , //  
                        73 , //  
                        74 , //  
                        75 , //  
                        76 , //  
                        77 , //  
                        77 , //  
                        78 , //  
                        79 , //  
                        80 , //  
                        81 , //  
                        82 , //  
                        82 , //  
                        83 , //  
                        84 , //  
                        85 , //  
                        86 , //  
                        86 , //  
                        87 , //  
                        88 , //  
                        89 , //  
                        90 , //  
                        90 , //  
                        91 , //  
                        91 , //  
                        92 , //  
                        92 , //  
                        93 , //  
                        93 , //  
                        94 , //  
                        94 , //  
                        95 , //  
                        96 , //  
                        96 , //  
                        97 , //  
                        97 , //  
                        97 , //  
                        98 , //  
                        98 , //  
                        99 , //  
                        99 , //  
                        100, //  
                        100, //  
                        100, //  
                        101, //  
                        101, //  
                        101, //  
                        102, //  
                        102, //  
                        102, //  
                        103, //  
                        103, //  
                        103, //  
                        103, //  
                        103, //  
                        103, //  
                        104, //  
                        104, //  
                        104, //  
                        104, //  
                        104, //  
                        104, //  
                        104, //  
                        104, //  
                        104, //  
                        104, //  
                        104, //  
                        105, //  
                        105, //  
                        105, //  
                        105, //  
                        105, //  
                        105, //  
                        105, //  
                        105, //  
                        105, //  
                        105, //  
                        105, //  
                        105, //  
                        105, //   
                        105, //   
                        105, //   
                        105, //   
                        105, //   
                        105, //   
                        105, //   
                        105, //   
                        105, //   
                        105, //   
                        105, //   
                        105, //   
                        105, //   
                        105, //   
                        105, //   
                        105, //   
                        105, //   
                        105, //   
                        105, //   
                        105, //   
                        105, //   
                        105, //   
                        105, //   
                        105, //   
                        105, //   
                        105, //   
                        105, //   
                        105, //   
                    }
                }, //          
                {
                    {
                        0  , // 
                        41 , // 
                        44 , // 
                        46 , // 
                        48 , // 
                        50 , // 
                        52 , // 
                        54 , // 
                        56 , // 
                        57 , // 
                        59 , //  
                        60 , //  
                        61 , //  
                        63 , //  
                        64 , //  
                        65 , //  
                        66 , //  
                        67 , //  
                        69 , //  
                        70 , //  
                        71 , //  
                        72 , //  
                        73 , //  
                        74 , //  
                        75 , //  
                        76 , //  
                        77 , //  
                        77 , //  
                        78 , //  
                        79 , //  
                        80 , //  
                        80 , //  
                        81 , //  
                        82 , //  
                        83 , //  
                        83 , //  
                        84 , //  
                        85 , //  
                        86 , //  
                        86 , //  
                        87 , //  
                        88 , //  
                        88 , //  
                        89 , //  
                        90 , //  
                        90 , //  
                        91 , //  
                        92 , //  
                        92 , //  
                        93 , //  
                        93 , //  
                        94 , //  
                        94 , //  
                        95 , //  
                        95 , //  
                        96 , //  
                        96 , //  
                        96 , //  
                        97 , //  
                        97 , //  
                        98 , //  
                        98 , //  
                        99 , //  
                        99 , //  
                        99 , //  
                        100, //  
                        100, //  
                        100, //  
                        101, //  
                        101, //  
                        101, //  
                        102, //  
                        102, //  
                        102, //  
                        103, //  
                        103, //  
                        103, //  
                        103, //  
                        103, //  
                        103, //  
                        103, //  
                        104, //  
                        104, //  
                        104, //  
                        104, //  
                        104, //  
                        104, //  
                        104, //  
                        104, //  
                        104, //  
                        104, //  
                        104, //  
                        104, //  
                        104, //  
                        105, //  
                        105, //  
                        105, //  
                        105, //  
                        105, //  
                        105, //  
                        105, //   
                        105, //   
                        105, //   
                        105, //   
                        105, //   
                        105, //   
                        105, //   
                        105, //   
                        105, //   
                        105, //   
                        105, //   
                        105, //   
                        105, //   
                        105, //   
                        105, //   
                        105, //   
                        105, //   
                        105, //   
                        105, //   
                        105, //   
                        105, //   
                        105, //   
                        105, //   
                        105, //   
                        105, //   
                        105, //   
                        105, //   
                        105, //   
                        105,
                    }
                }, //          
                {
                    {
                        0  , // 
                        42 , // 
                        44 , // 
                        46 , // 
                        48 , // 
                        50 , // 
                        52 , // 
                        54 , // 
                        56 , // 
                        58 , // 
                        59 , //  
                        60 , //  
                        61 , //  
                        63 , //  
                        64 , //  
                        65 , //  
                        66 , //  
                        68 , //  
                        69 , //  
                        70 , //  
                        71 , //  
                        72 , //  
                        73 , //  
                        74 , //  
                        75 , //  
                        75 , //  
                        76 , //  
                        77 , //  
                        78 , //  
                        79 , //  
                        80 , //  
                        80 , //  
                        81 , //  
                        82 , //  
                        83 , //  
                        83 , //  
                        84 , //  
                        85 , //  
                        86 , //  
                        86 , //  
                        87 , //  
                        88 , //  
                        89 , //  
                        89 , //  
                        90 , //  
                        91 , //  
                        91 , //  
                        92 , //  
                        93 , //  
                        93 , //  
                        94 , //  
                        94 , //  
                        95 , //  
                        95 , //  
                        96 , //  
                        96 , //  
                        97 , //  
                        97 , //  
                        97 , //  
                        98 , //  
                        98 , //  
                        99 , //  
                        99 , //  
                        100, //  
                        100, //  
                        100, //  
                        101, //  
                        101, //  
                        101, //  
                        102, //  
                        102, //  
                        102, //  
                        103, //  
                        103, //  
                        103, //  
                        103, //  
                        103, //  
                        103, //  
                        104, //  
                        104, //  
                        104, //  
                        104, //  
                        104, //  
                        104, //  
                        104, //  
                        104, //  
                        104, //  
                        104, //  
                        104, //  
                        104, //  
                        105, //  
                        105, //  
                        105, //  
                        105, //  
                        105, //  
                        105, //  
                        105, //  
                        105, //  
                        105, //  
                        105, //  
                        105, //   
                        105, //   
                        105, //   
                        105, //   
                        105, //   
                        105, //   
                        105, //   
                        105, //   
                        105, //   
                        105, //   
                        105, //   
                        105, //   
                        105, //   
                        105, //   
                        105, //   
                        105, //   
                        105, //   
                        105, //   
                        105, //   
                        105, //   
                        105, //   
                        105, //   
                        105, //   
                        105, //   
                        105, //   
                        105, //   
                        105, //   
                        105, //   
                        105,
                    }
                }, //          
                {
                    {
                       0,  // 
                       40,  // 
                       43,  // 
                       45,  // 
                       47,  // 
                       49,  // 
                       50,  // 
                       52,  // 
                       54,  // 
                       56,  // 
                       57,  //  
                       58,  //  
                       59,  //  
                       60,  //  
                       62,  //  
                       63,  //  
                       64,  //  
                       65,  //  
                       66,  //  
                       67,  //  
                       68,  //  
                       69,  //  
                       70,  //  
                       71,  //  
                       72,  //  
                       73,  //  
                       74,  //  
                       74,  //  
                       75,  //  
                       76,  //  
                       77,  //  
                       78,  //  
                       78,  //  
                       79,  //  
                       80,  //  
                       81,  //  
                       82,  //  
                       83,  //  
                       83,  //  
                       84,  //  
                       85,  //  
                       85,  //  
                       86,  //  
                       87,  //  
                       87,  //  
                       88,  //  
                       89,  //  
                       89,  //  
                       90,  //  
                       90,  //  
                       91,  //  
                       91,  //  
                       92,  //  
                       92,  //  
                       93,  //  
                       93,  //  
                       94,  //  
                       94,  //  
                       95,  //  
                       95,  //  
                       96,  //  
                       96,  //  
                       96,  //  
                       97,  //  
                       97,  //  
                       97,  //  
                       98,  //  
                       98,  //  
                       98,  //  
                       98,  //  
                       99,  //  
                       99,  //  
                       99,  //  
                       99,  //  
                       99,  //  
                       99,  //  
                       99,  //  
                       99,  //  
                       99,  //  
                       99,  //  
                       100,  //  
                       100,  //  
                       100,  //  
                       100,  //  
                       100,  //  
                       100,  //  
                       100,  //  
                       100,  //  
                       100,  //  
                       100,  //  
                       100,  //  
                       100,  //  
                       100,  //  
                       100,  //  
                       100,  //  
                       100,  //  
                       100,  //  
                       100,  //  
                       100, //  
                       100, //  
                       100, //   
                       100, //   
                       100, //   
                       100, //   
                       100, //   
                       100, //   
                       100, //   
                       100, //   
                       100, //   
                       100, //   
                       100, //   
                       100, //   
                       100, //   
                       100, //   
                       100, //   
                       100, //   
                       100, //   
                       100, //   
                       100, //   
                       100, //   
                       100, //   
                       100, //   
                       100, //   
                       100, //   
                       100, //   
                       100, //   
                       100, //   
                       100, //   
                       100,
                    }
                }, //          
        },

        //                          
        {
            98,  //          
            101,  //          
            101,  //          
            100,  //          
            98,  //          
            97,  //          
            94,  //          
            94,  //          
            92,  //          
            90,  //           
            94,  //           
            95,  //           
            97,  //           
            104,   //           
            100,   //            
            100,   //            
            100,   //            
            100,   //            
            100,   //            
            100,   //            
            100,   //            
            100,   //           
            100,   //           
            100,   //           
            100,   //           
            100,   //           
            100,   //           
            100,   //           
            100,   //           
            100,   //            
            100,   //            
            100,   //            
            100,   //            
            100,   //            
            100,   //            
            100,   //            
            100,   //            
            100,   //            
            100,   //            
            100,   //            
            100,   //            
            100,   //            
            100,   //            
            100,   //            
            100,   //            
            //                       
            100,   //               
            100,   //               
            100,   //               
            100,   //               
            100,   //               
            100,   //               
            100,   //               
            100,   //                
            100,   //                
            100,   //                 
            100,   //                 
            100,   //                 
            100,   //                 
            100,   //                 
            100,   //                
            100,   //                
            100,   //                
            100,   //                
            100,   //                
            100,   //                
            100,   //                
            100,   //                 
            100,   //                 
            100,   //                 
            100,   //                 
            100,   //                 
            100,   //                 
            100,   //                 
            100,   //                 
            100,   //                 
            100,   //                 
            100,   //                 
            100,   //                 
            100,   //                 
            100,   //                 
        },

        //                     
        //                            
        //                                                   
        //                                                
        {
            //                
            {
                //             
                {100},    //                           
                {100},    //                                
                {100},    //                                        
                {100},    //                                   
                {100},    //                                          
                {100},    //          
                {100},    //          
                {100},    //          
                {100},    //          
                {100},    //          
                {100},    //          
                {100},    //          
                {100},    //          
            },
            //                  
            {
                //             
                {100},    //                           
                {100},    //                                
                {100},    //                                        
                {100},    //                                   
                {100},    //                                          
                {100},    //          
                {100},    //          
                {100},    //          
                {100},    //          
                {100},    //          
                {100},    //          
                {100},    //          
                {100},    //          
            },
            //                     
            {
                //             
                {100},    //                           
                {100},    //                                
                {100},    //                                        
                {100},    //                                   
                {100},    //                                          
                {100},    //          
                {100},    //          
                {100},    //          
                {100},    //          
                {100},    //          
                {100},    //          
                {100},    //          
                {100},    //          
            },
            //                   
            {
                //             
                {100},    //                           
                {100},    //                                
                {100},    //                                        
                {100},    //                                   
                {100},    //                                          
                {100},    //          
                {100},    //          
                {100},    //          
                {100},    //          
                {100},    //          
                {100},    //          
                {100},    //          
                {100},    //          
            },
            //                
            {
                //             
                {100},    //                           
                {100},    //                                
                {100},    //                                        
                {100},    //                                   
                {100},    //                                          
                {100},    //          
                {100},    //          
                {100},    //          
                {100},    //          
                {100},    //          
                {100},    //          
                {100},    //          
                {100},    //          
            }
        },

#if 0 //           
        //                   
        {
            0x7FFF,      //                             
            0x00,        //                           
            0x00,        //                       

            0x00,        //                          
            0x00,        //                          
            0x00,        //                     
            0x00,        //                        

            0x00,        //                    
            0x00,        //                    
            { 0x00, 0x00 },        //                         

            0x00,        //                    
            0x00,        //                    
            0x00,        //                    
            0x00,        //                    

            0x7FFF,      //                        
            0x00,        //                       
            0x00,        //                        

            0x00,        //                             
            0x00,        //                         
            0x00,        //                          
            0x00,        //                          

            0x00,        //                         
            0x00,        //                          
            0x00,        //                          
            0x00,        //                             

            0x00,        //                           
            0x00,        //                                              
            0x00,        //                                 
            0x00,        //                                

            0x00,        //                           
            0x00,        //                           
            0x00,        //                           
            0x00,        //                           

            0x00,        //                             
            0x00,        //                             
            0x00,        //                             
            0x00,        //                             

            0x7FFF,        //                       
            0x7FFF,        //                        

            { //               
                {
                    { 0x00, 0x00 }, //               
                    { 0x00, 0x00 }, //               
                    { 0x00, 0x00 }, //               
                    { 0x00, 0x00 }, //               
                    { 0x00, 0x00 }, //               
                    { 0x00, 0x00 }, //               
                    { 0x00, 0x00 }, //               
                    { 0x00, 0x00 }, //               
                    { 0x00, 0x00 }, //               
                    { 0x00, 0x00 }, //               
                    { 0x00, 0x00 }, //                
                    { 0x00, 0x00 }, //                
                    { 0x00, 0x00 }, //                
                    { 0x00, 0x00 }, //                
                    { 0x00, 0x00 }, //                
                    { 0x00, 0x00 }  //                
                }, //         
                {
                    { 0x00, 0x00 }, //               
                    { 0x00, 0x00 }, //               
                    { 0x00, 0x00 }, //               
                    { 0x00, 0x00 }, //               
                    { 0x00, 0x00 }, //               
                    { 0x00, 0x00 }, //               
                    { 0x00, 0x00 }, //               
                    { 0x00, 0x00 }, //               
                    { 0x00, 0x00 }, //               
                    { 0x00, 0x00 }, //               
                    { 0x00, 0x00 }, //                
                    { 0x00, 0x00 }, //                
                    { 0x00, 0x00 }, //                
                    { 0x00, 0x00 }, //                
                    { 0x00, 0x00 }, //                
                    { 0x00, 0x00 }  //                
                }, //         
                {
                    { 0x00, 0x00 }, //               
                    { 0x00, 0x00 }, //               
                    { 0x00, 0x00 }, //               
                    { 0x00, 0x00 }, //               
                    { 0x00, 0x00 }, //               
                    { 0x00, 0x00 }, //               
                    { 0x00, 0x00 }, //               
                    { 0x00, 0x00 }, //               
                    { 0x00, 0x00 }, //               
                    { 0x00, 0x00 }, //               
                    { 0x00, 0x00 }, //                
                    { 0x00, 0x00 }, //                
                    { 0x00, 0x00 }, //                
                    { 0x00, 0x00 }, //                
                    { 0x00, 0x00 }, //                
                    { 0x00, 0x00 }  //                
                }, //         
                {
                    { 0x00, 0x00 }, //               
                    { 0x00, 0x00 }, //               
                    { 0x00, 0x00 }, //               
                    { 0x00, 0x00 }, //               
                    { 0x00, 0x00 }, //               
                    { 0x00, 0x00 }, //               
                    { 0x00, 0x00 }, //               
                    { 0x00, 0x00 }, //               
                    { 0x00, 0x00 }, //               
                    { 0x00, 0x00 }, //               
                    { 0x00, 0x00 }, //                
                    { 0x00, 0x00 }, //                
                    { 0x00, 0x00 }, //                
                    { 0x00, 0x00 }, //                
                    { 0x00, 0x00 }, //                
                    { 0x00, 0x00 }  //                
                }, //         
                {
                    { 0x00, 0x00 }, //               
                    { 0x00, 0x00 }, //               
                    { 0x00, 0x00 }, //               
                    { 0x00, 0x00 }, //               
                    { 0x00, 0x00 }, //               
                    { 0x00, 0x00 }, //               
                    { 0x00, 0x00 }, //               
                    { 0x00, 0x00 }, //               
                    { 0x00, 0x00 }, //               
                    { 0x00, 0x00 }, //               
                    { 0x00, 0x00 }, //                
                    { 0x00, 0x00 }, //                
                    { 0x00, 0x00 }, //                
                    { 0x00, 0x00 }, //                
                    { 0x00, 0x00 }, //                
                    { 0x00, 0x00 }  //                
                }, //         
                {
                    { 0x00, 0x00 }, //               
                    { 0x00, 0x00 }, //               
                    { 0x00, 0x00 }, //               
                    { 0x00, 0x00 }, //               
                    { 0x00, 0x00 }, //               
                    { 0x00, 0x00 }, //               
                    { 0x00, 0x00 }, //               
                    { 0x00, 0x00 }, //               
                    { 0x00, 0x00 }, //               
                    { 0x00, 0x00 }, //               
                    { 0x00, 0x00 }, //                
                    { 0x00, 0x00 }, //                
                    { 0x00, 0x00 }, //                
                    { 0x00, 0x00 }, //                
                    { 0x00, 0x00 }, //                
                    { 0x00, 0x00 }  //                
                }, //         
                {
                    { 0x00, 0x00 }, //               
                    { 0x00, 0x00 }, //               
                    { 0x00, 0x00 }, //               
                    { 0x00, 0x00 }, //               
                    { 0x00, 0x00 }, //               
                    { 0x00, 0x00 }, //               
                    { 0x00, 0x00 }, //               
                    { 0x00, 0x00 }, //               
                    { 0x00, 0x00 }, //               
                    { 0x00, 0x00 }, //               
                    { 0x00, 0x00 }, //                
                    { 0x00, 0x00 }, //                
                    { 0x00, 0x00 }, //                
                    { 0x00, 0x00 }, //                
                    { 0x00, 0x00 }, //                
                    { 0x00, 0x00 }  //                
                }, //         
                {
                    { 0x00, 0x00 }, //               
                    { 0x00, 0x00 }, //               
                    { 0x00, 0x00 }, //               
                    { 0x00, 0x00 }, //               
                    { 0x00, 0x00 }, //               
                    { 0x00, 0x00 }, //               
                    { 0x00, 0x00 }, //               
                    { 0x00, 0x00 }, //               
                    { 0x00, 0x00 }, //               
                    { 0x00, 0x00 }, //               
                    { 0x00, 0x00 }, //                
                    { 0x00, 0x00 }, //                
                    { 0x00, 0x00 }, //                
                    { 0x00, 0x00 }, //                
                    { 0x00, 0x00 }, //                
                    { 0x00, 0x00 }  //                
                }, //         
                {
                    { 0x00, 0x00 }, //               
                    { 0x00, 0x00 }, //               
                    { 0x00, 0x00 }, //               
                    { 0x00, 0x00 }, //               
                    { 0x00, 0x00 }, //               
                    { 0x00, 0x00 }, //               
                    { 0x00, 0x00 }, //               
                    { 0x00, 0x00 }, //               
                    { 0x00, 0x00 }, //               
                    { 0x00, 0x00 }, //               
                    { 0x00, 0x00 }, //                
                    { 0x00, 0x00 }, //                
                    { 0x00, 0x00 }, //                
                    { 0x00, 0x00 }, //                
                    { 0x00, 0x00 }, //                
                    { 0x00, 0x00 }  //                
                }, //         
                {
                    { 0x00, 0x00 }, //               
                    { 0x00, 0x00 }, //               
                    { 0x00, 0x00 }, //               
                    { 0x00, 0x00 }, //               
                    { 0x00, 0x00 }, //               
                    { 0x00, 0x00 }, //               
                    { 0x00, 0x00 }, //               
                    { 0x00, 0x00 }, //               
                    { 0x00, 0x00 }, //               
                    { 0x00, 0x00 }, //               
                    { 0x00, 0x00 }, //                
                    { 0x00, 0x00 }, //                
                    { 0x00, 0x00 }, //                
                    { 0x00, 0x00 }, //                
                    { 0x00, 0x00 }, //                
                    { 0x00, 0x00 }  //                
                }, //          
                {
                    { 0x00, 0x00 }, //               
                    { 0x00, 0x00 }, //               
                    { 0x00, 0x00 }, //               
                    { 0x00, 0x00 }, //               
                    { 0x00, 0x00 }, //               
                    { 0x00, 0x00 }, //               
                    { 0x00, 0x00 }, //               
                    { 0x00, 0x00 }, //               
                    { 0x00, 0x00 }, //               
                    { 0x00, 0x00 }, //               
                    { 0x00, 0x00 }, //                
                    { 0x00, 0x00 }, //                
                    { 0x00, 0x00 }, //                
                    { 0x00, 0x00 }, //                
                    { 0x00, 0x00 }, //                
                    { 0x00, 0x00 }  //                
                }, //          
                {
                    { 0x00, 0x00 }, //               
                    { 0x00, 0x00 }, //               
                    { 0x00, 0x00 }, //               
                    { 0x00, 0x00 }, //               
                    { 0x00, 0x00 }, //               
                    { 0x00, 0x00 }, //               
                    { 0x00, 0x00 }, //               
                    { 0x00, 0x00 }, //               
                    { 0x00, 0x00 }, //               
                    { 0x00, 0x00 }, //               
                    { 0x00, 0x00 }, //                
                    { 0x00, 0x00 }, //                
                    { 0x00, 0x00 }, //                
                    { 0x00, 0x00 }, //                
                    { 0x00, 0x00 }, //                
                    { 0x00, 0x00 }  //                
                }, //          
                {
                    { 0x00, 0x00 }, //               
                    { 0x00, 0x00 }, //               
                    { 0x00, 0x00 }, //               
                    { 0x00, 0x00 }, //               
                    { 0x00, 0x00 }, //               
                    { 0x00, 0x00 }, //               
                    { 0x00, 0x00 }, //               
                    { 0x00, 0x00 }, //               
                    { 0x00, 0x00 }, //               
                    { 0x00, 0x00 }, //               
                    { 0x00, 0x00 }, //                
                    { 0x00, 0x00 }, //                
                    { 0x00, 0x00 }, //                
                    { 0x00, 0x00 }, //                
                    { 0x00, 0x00 }, //                
                    { 0x00, 0x00 }  //                
                }, //          
                {
                    { 0x00, 0x00 }, //               
                    { 0x00, 0x00 }, //               
                    { 0x00, 0x00 }, //               
                    { 0x00, 0x00 }, //               
                    { 0x00, 0x00 }, //               
                    { 0x00, 0x00 }, //               
                    { 0x00, 0x00 }, //               
                    { 0x00, 0x00 }, //               
                    { 0x00, 0x00 }, //               
                    { 0x00, 0x00 }, //               
                    { 0x00, 0x00 }, //                
                    { 0x00, 0x00 }, //                
                    { 0x00, 0x00 }, //                
                    { 0x00, 0x00 }, //                
                    { 0x00, 0x00 }, //                
                    { 0x00, 0x00 }  //                
                }  //          
            },        //                                

            { //            
                {
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //                
                    { 0x0000, 0x0000, 0x0000 }, //                
                    { 0x0000, 0x0000, 0x0000 }, //                
                    { 0x0000, 0x0000, 0x0000 }, //                
                    { 0x0000, 0x0000, 0x0000 }, //                
                    { 0x0000, 0x0000, 0x0000 }  //                
                }, //         
                {
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //                
                    { 0x0000, 0x0000, 0x0000 }, //                
                    { 0x0000, 0x0000, 0x0000 }, //                
                    { 0x0000, 0x0000, 0x0000 }, //                
                    { 0x0000, 0x0000, 0x0000 }, //                
                    { 0x0000, 0x0000, 0x0000 }  //                
                }, //         
                {
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //                
                    { 0x0000, 0x0000, 0x0000 }, //                
                    { 0x0000, 0x0000, 0x0000 }, //                
                    { 0x0000, 0x0000, 0x0000 }, //                
                    { 0x0000, 0x0000, 0x0000 }, //                
                    { 0x0000, 0x0000, 0x0000 }  //                
                }, //         
                {
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //                
                    { 0x0000, 0x0000, 0x0000 }, //                
                    { 0x0000, 0x0000, 0x0000 }, //                
                    { 0x0000, 0x0000, 0x0000 }, //                
                    { 0x0000, 0x0000, 0x0000 }, //                
                    { 0x0000, 0x0000, 0x0000 }  //                
                }, //         
                {
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //                
                    { 0x0000, 0x0000, 0x0000 }, //                
                    { 0x0000, 0x0000, 0x0000 }, //                
                    { 0x0000, 0x0000, 0x0000 }, //                
                    { 0x0000, 0x0000, 0x0000 }, //                
                    { 0x0000, 0x0000, 0x0000 }  //                
                }, //         
                {
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //                
                    { 0x0000, 0x0000, 0x0000 }, //                
                    { 0x0000, 0x0000, 0x0000 }, //                
                    { 0x0000, 0x0000, 0x0000 }, //                
                    { 0x0000, 0x0000, 0x0000 }, //                
                    { 0x0000, 0x0000, 0x0000 }  //                
                }, //         
                {
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //                
                    { 0x0000, 0x0000, 0x0000 }, //                
                    { 0x0000, 0x0000, 0x0000 }, //                
                    { 0x0000, 0x0000, 0x0000 }, //                
                    { 0x0000, 0x0000, 0x0000 }, //                
                    { 0x0000, 0x0000, 0x0000 }  //                
                }, //         
                {
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //                
                    { 0x0000, 0x0000, 0x0000 }, //                
                    { 0x0000, 0x0000, 0x0000 }, //                
                    { 0x0000, 0x0000, 0x0000 }, //                
                    { 0x0000, 0x0000, 0x0000 }, //                
                    { 0x0000, 0x0000, 0x0000 }  //                
                }, //         
                {
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //                
                    { 0x0000, 0x0000, 0x0000 }, //                
                    { 0x0000, 0x0000, 0x0000 }, //                
                    { 0x0000, 0x0000, 0x0000 }, //                
                    { 0x0000, 0x0000, 0x0000 }, //                
                    { 0x0000, 0x0000, 0x0000 }  //                
                }, //         
                {
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //                
                    { 0x0000, 0x0000, 0x0000 }, //                
                    { 0x0000, 0x0000, 0x0000 }, //                
                    { 0x0000, 0x0000, 0x0000 }, //                
                    { 0x0000, 0x0000, 0x0000 }, //                
                    { 0x0000, 0x0000, 0x0000 }  //                
                }, //          
                {
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //                
                    { 0x0000, 0x0000, 0x0000 }, //                
                    { 0x0000, 0x0000, 0x0000 }, //                
                    { 0x0000, 0x0000, 0x0000 }, //                
                    { 0x0000, 0x0000, 0x0000 }, //                
                    { 0x0000, 0x0000, 0x0000 }  //                
                }, //          
                {
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //                
                    { 0x0000, 0x0000, 0x0000 }, //                
                    { 0x0000, 0x0000, 0x0000 }, //                
                    { 0x0000, 0x0000, 0x0000 }, //                
                    { 0x0000, 0x0000, 0x0000 }, //                
                    { 0x0000, 0x0000, 0x0000 }  //                
                }, //          
                {
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //                
                    { 0x0000, 0x0000, 0x0000 }, //                
                    { 0x0000, 0x0000, 0x0000 }, //                
                    { 0x0000, 0x0000, 0x0000 }, //                
                    { 0x0000, 0x0000, 0x0000 }, //                
                    { 0x0000, 0x0000, 0x0000 }  //                
                }, //          
                {
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //                
                    { 0x0000, 0x0000, 0x0000 }, //                
                    { 0x0000, 0x0000, 0x0000 }, //                
                    { 0x0000, 0x0000, 0x0000 }, //                
                    { 0x0000, 0x0000, 0x0000 }, //                
                    { 0x0000, 0x0000, 0x0000 }  //                
                }  //          
            },        //                                

            { //            
                {
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //                
                    { 0x0000, 0x0000, 0x0000 }, //                
                    { 0x0000, 0x0000, 0x0000 }, //                
                    { 0x0000, 0x0000, 0x0000 }, //                
                    { 0x0000, 0x0000, 0x0000 }, //                
                    { 0x0000, 0x0000, 0x0000 }  //                
                }, //         
                {
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //                
                    { 0x0000, 0x0000, 0x0000 }, //                
                    { 0x0000, 0x0000, 0x0000 }, //                
                    { 0x0000, 0x0000, 0x0000 }, //                
                    { 0x0000, 0x0000, 0x0000 }, //                
                    { 0x0000, 0x0000, 0x0000 }  //                
                }, //         
                {
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //                
                    { 0x0000, 0x0000, 0x0000 }, //                
                    { 0x0000, 0x0000, 0x0000 }, //                
                    { 0x0000, 0x0000, 0x0000 }, //                
                    { 0x0000, 0x0000, 0x0000 }, //                
                    { 0x0000, 0x0000, 0x0000 }  //                
                }, //         
                {
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //                
                    { 0x0000, 0x0000, 0x0000 }, //                
                    { 0x0000, 0x0000, 0x0000 }, //                
                    { 0x0000, 0x0000, 0x0000 }, //                
                    { 0x0000, 0x0000, 0x0000 }, //                
                    { 0x0000, 0x0000, 0x0000 }  //                
                }, //         
                {
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //                
                    { 0x0000, 0x0000, 0x0000 }, //                
                    { 0x0000, 0x0000, 0x0000 }, //                
                    { 0x0000, 0x0000, 0x0000 }, //                
                    { 0x0000, 0x0000, 0x0000 }, //                
                    { 0x0000, 0x0000, 0x0000 }  //                
                }, //         
                {
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //                
                    { 0x0000, 0x0000, 0x0000 }, //                
                    { 0x0000, 0x0000, 0x0000 }, //                
                    { 0x0000, 0x0000, 0x0000 }, //                
                    { 0x0000, 0x0000, 0x0000 }, //                
                    { 0x0000, 0x0000, 0x0000 }  //                
                }, //         
                {
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //                
                    { 0x0000, 0x0000, 0x0000 }, //                
                    { 0x0000, 0x0000, 0x0000 }, //                
                    { 0x0000, 0x0000, 0x0000 }, //                
                    { 0x0000, 0x0000, 0x0000 }, //                
                    { 0x0000, 0x0000, 0x0000 }  //                
                }, //         
                {
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //                
                    { 0x0000, 0x0000, 0x0000 }, //                
                    { 0x0000, 0x0000, 0x0000 }, //                
                    { 0x0000, 0x0000, 0x0000 }, //                
                    { 0x0000, 0x0000, 0x0000 }, //                
                    { 0x0000, 0x0000, 0x0000 }  //                
                }, //         
                {
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //                
                    { 0x0000, 0x0000, 0x0000 }, //                
                    { 0x0000, 0x0000, 0x0000 }, //                
                    { 0x0000, 0x0000, 0x0000 }, //                
                    { 0x0000, 0x0000, 0x0000 }, //                
                    { 0x0000, 0x0000, 0x0000 }  //                
                }, //         
                {
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //                
                    { 0x0000, 0x0000, 0x0000 }, //                
                    { 0x0000, 0x0000, 0x0000 }, //                
                    { 0x0000, 0x0000, 0x0000 }, //                
                    { 0x0000, 0x0000, 0x0000 }, //                
                    { 0x0000, 0x0000, 0x0000 }  //                
                }, //          
                {
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //                
                    { 0x0000, 0x0000, 0x0000 }, //                
                    { 0x0000, 0x0000, 0x0000 }, //                
                    { 0x0000, 0x0000, 0x0000 }, //                
                    { 0x0000, 0x0000, 0x0000 }, //                
                    { 0x0000, 0x0000, 0x0000 }  //                
                }, //          
                {
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //                
                    { 0x0000, 0x0000, 0x0000 }, //                
                    { 0x0000, 0x0000, 0x0000 }, //                
                    { 0x0000, 0x0000, 0x0000 }, //                
                    { 0x0000, 0x0000, 0x0000 }, //                
                    { 0x0000, 0x0000, 0x0000 }  //                
                }, //          
                {
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //                
                    { 0x0000, 0x0000, 0x0000 }, //                
                    { 0x0000, 0x0000, 0x0000 }, //                
                    { 0x0000, 0x0000, 0x0000 }, //                
                    { 0x0000, 0x0000, 0x0000 }, //                
                    { 0x0000, 0x0000, 0x0000 }  //                
                }, //          
                {
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //               
                    { 0x0000, 0x0000, 0x0000 }, //                
                    { 0x0000, 0x0000, 0x0000 }, //                
                    { 0x0000, 0x0000, 0x0000 }, //                
                    { 0x0000, 0x0000, 0x0000 }, //                
                    { 0x0000, 0x0000, 0x0000 }, //                
                    { 0x0000, 0x0000, 0x0000 }  //                
                }  //          
            },        //                                

            { //                                               
                {
                    {
                        {
                            { 0x00, 0x00 }, //           
                            { 0x00, 0x00 }, //           
                            { 0x00, 0x00 }, //           
                            { 0x00, 0x00 }, //           
                            { 0x00, 0x00 }, //           
                            { 0x00, 0x00 }, //           
                            { 0x00, 0x00 }, //           
                            { 0x00, 0x00 }  //           
                        } //               
                    } //          
                }, //          
                {
                    {
                        {
                            { 0x00, 0x00 }, //           
                            { 0x00, 0x00 }, //           
                            { 0x00, 0x00 }, //           
                            { 0x00, 0x00 }, //           
                            { 0x00, 0x00 }, //           
                            { 0x00, 0x00 }, //           
                            { 0x00, 0x00 }, //           
                            { 0x00, 0x00 }  //           
                        } //               
                    } //          
                }, //          
                {
                    {
                        {
                            { 0x00, 0x00 }, //           
                            { 0x00, 0x00 }, //           
                            { 0x00, 0x00 }, //           
                            { 0x00, 0x00 }, //           
                            { 0x00, 0x00 }, //           
                            { 0x00, 0x00 }, //           
                            { 0x00, 0x00 }, //           
                            { 0x00, 0x00 }  //           
                        } //               
                    } //          
                }, //          
                {
                    {
                        {
                            { 0x00, 0x00 }, //           
                            { 0x00, 0x00 }, //           
                            { 0x00, 0x00 }, //           
                            { 0x00, 0x00 }, //           
                            { 0x00, 0x00 }, //           
                            { 0x00, 0x00 }, //           
                            { 0x00, 0x00 }, //           
                            { 0x00, 0x00 }  //           
                        } //               
                    } //          
                }, //          
                {
                    {
                        {
                            { 0x00, 0x00 }, //           
                            { 0x00, 0x00 }, //           
                            { 0x00, 0x00 }, //           
                            { 0x00, 0x00 }, //           
                            { 0x00, 0x00 }, //           
                            { 0x00, 0x00 }, //           
                            { 0x00, 0x00 }, //           
                            { 0x00, 0x00 }  //           
                        } //               
                    } //          
                }, //          
                {
                    {
                        {
                            { 0x00, 0x00 }, //           
                            { 0x00, 0x00 }, //           
                            { 0x00, 0x00 }, //           
                            { 0x00, 0x00 }, //           
                            { 0x00, 0x00 }, //           
                            { 0x00, 0x00 }, //           
                            { 0x00, 0x00 }, //           
                            { 0x00, 0x00 }  //           
                        } //               
                    } //          
                }, //          
                {
                    {
                        {
                            { 0x00, 0x00 }, //           
                            { 0x00, 0x00 }, //           
                            { 0x00, 0x00 }, //           
                            { 0x00, 0x00 }, //           
                            { 0x00, 0x00 }, //           
                            { 0x00, 0x00 }, //           
                            { 0x00, 0x00 }, //           
                            { 0x00, 0x00 }  //           
                        } //               
                    } //          
                }, //          
                {
                    {
                        {
                            { 0x00, 0x00 }, //           
                            { 0x00, 0x00 }, //           
                            { 0x00, 0x00 }, //           
                            { 0x00, 0x00 }, //           
                            { 0x00, 0x00 }, //           
                            { 0x00, 0x00 }, //           
                            { 0x00, 0x00 }, //           
                            { 0x00, 0x00 }  //           
                        } //               
                    } //          
                }, //          
                {
                    {
                        {
                            { 0x00, 0x00 }, //           
                            { 0x00, 0x00 }, //           
                            { 0x00, 0x00 }, //           
                            { 0x00, 0x00 }, //           
                            { 0x00, 0x00 }, //           
                            { 0x00, 0x00 }, //           
                            { 0x00, 0x00 }, //           
                            { 0x00, 0x00 }  //           
                        } //               
                    } //          
                }, //          
                {
                    {
                        {
                            { 0x00, 0x00 }, //           
                            { 0x00, 0x00 }, //           
                            { 0x00, 0x00 }, //           
                            { 0x00, 0x00 }, //           
                            { 0x00, 0x00 }, //           
                            { 0x00, 0x00 }, //           
                            { 0x00, 0x00 }, //           
                            { 0x00, 0x00 }  //           
                        } //               
                    } //          
                }, //           
                {
                    {
                        {
                            { 0x00, 0x00 }, //           
                            { 0x00, 0x00 }, //           
                            { 0x00, 0x00 }, //           
                            { 0x00, 0x00 }, //           
                            { 0x00, 0x00 }, //           
                            { 0x00, 0x00 }, //           
                            { 0x00, 0x00 }, //           
                            { 0x00, 0x00 }  //           
                        } //               
                    } //          
                }, //           
                {
                    {
                        {
                            { 0x00, 0x00 }, //           
                            { 0x00, 0x00 }, //           
                            { 0x00, 0x00 }, //           
                            { 0x00, 0x00 }, //           
                            { 0x00, 0x00 }, //           
                            { 0x00, 0x00 }, //           
                            { 0x00, 0x00 }, //           
                            { 0x00, 0x00 }  //           
                        } //               
                    } //          
                }, //           
                {
                    {
                        {
                            { 0x00, 0x00 }, //           
                            { 0x00, 0x00 }, //           
                            { 0x00, 0x00 }, //           
                            { 0x00, 0x00 }, //           
                            { 0x00, 0x00 }, //           
                            { 0x00, 0x00 }, //           
                            { 0x00, 0x00 }, //           
                            { 0x00, 0x00 }  //           
                        } //               
                    } //          
                },  //           
                {
                    {
                        {
                            { 0x00, 0x00 }, //           
                            { 0x00, 0x00 }, //           
                            { 0x00, 0x00 }, //           
                            { 0x00, 0x00 }, //           
                            { 0x00, 0x00 }, //           
                            { 0x00, 0x00 }, //           
                            { 0x00, 0x00 }, //           
                            { 0x00, 0x00 }  //           
                        } //               
                    } //          
                }  //           
            },        //                                                

            {
                { 0x0000, { 0x00, 0x00 } }, //                                     
                { 0x0000, { 0x00, 0x00 } }, //                                     
                { 0x0000, { 0x00, 0x00 } }, //                                     
                { 0x0000, { 0x00, 0x00 } }, //                                     
                { 0x0000, { 0x00, 0x00 } }, //                                     
                { 0x0000, { 0x00, 0x00 } }, //                                     
                { 0x0000, { 0x00, 0x00 } }, //                                     
                { 0x0000, { 0x00, 0x00 } }, //                                     
                { 0x0000, { 0x00, 0x00 } }, //                                     
                { 0x0000, { 0x00, 0x00 } }, //                                      
                { 0x0000, { 0x00, 0x00 } }, //                                      
                { 0x0000, { 0x00, 0x00 } }, //                                      
                { 0x0000, { 0x00, 0x00 } }, //                                      
                { 0x0000, { 0x00, 0x00 } }  //                                      
            }        //                                                  

        }, //                   
#endif
        //                   
        {
            0xFF,        //                     
            0xFF,        //                    
            0xFF,        //                      
            0xFF,        //                    
            0xFF,        //                    
            0xFF,        //                    
            0xFF,        //                             
            0xFF,        //                             
            0xFF,        //                  
            0xFF,        //                  
            0xFF,        //                   
            0xFF,        //                  
            0xFF,        //                     
            0xFF,        //                           
            0xFF,        //                           
            0xFF,        //                              
            0xFF,        //                         
            0xFF,        //                  
            0xFF,        //                              
            0xFF,        //                   
            0xFF,        //                         
            0xFF,        //                                
            0xFF,        //                             
            0xFF,        //                        
            0xFF,        //                        
            0xFF,        //                      
            {
                0x00,
                0x00,
                0x00,
                0x00,
                0x00,
                0x00
            }        //                       
        },

        //                             
        {
            //              
            {
                //           
                {300}, //                      

                //            
                {300}  //                      
            },
            //    
            {
                //           
                {0},   //                      

                //            
                {0}    //                      
            }
        },

        //                      
        {
            //              
            // 
            //                                       
            //                     
            //              

            0,               //         
            {
                0,           //                
                0,           //              
                0,           //          

                0,           //             
                0,           //             
                0,           //        
                0,           //           

                0,           //       
                0,           //       
                0,           //          
                0,           //           

                0,           //       
                0,           //       
                0,           //       
                0,           //       

                0,           //           
                0,           //          
                0,           //           

                0,           //                
                0,           //            
                0,           //             
                0,           //             

                0,           //            
                0,           //             
                0,           //             
                0,           //                

                0,           //              
                0,           //                                 
                0,           //                    
                0,           //                   

                0,           //              
                0,           //              
                0,           //        

                0,           //                
                0,           //                
                0,           //                
                0,           //                

                0,           //          
                0,           //         

                0,           //               
                0,           //                    
                0            //          

            }
        },

        //                          
        {
            280,  //          
            270,  //          
            270,  //          
            270,  //          
            270,  //          
            270,  //          
            280,  //          
            280,  //          
            290,  //          
            300,  //           
            300,  //           
            310,  //           
            310,  //           
            310,   //           
            280,  //            
            280,  //            
            280,   //            
            280,   //            
            280,   //            
            280,   //            
            280,   //            
            280,   //           
            280,   //           
            280,   //           
            280,   //           
            280,   //           
            280,   //           
            280,   //           
            280,   //           
            280,   //            
            280,   //            
            280,   //            
            280,   //            
            280,   //            
            280,   //            
            280,   //            
            280,   //            
            280,   //            
            280,   //            
            280,   //            
            280,   //            
            280,   //            
            280,   //            
            280,   //            
            280,   //            
            //                       
            280,   //               
            280,   //               
            280,   //               
            280,   //               
            280,   //               
            280,   //               
            280,   //               
            280,   //                
            280,   //                
            280,   //                 
            280,   //                 
            280,   //                 
            280,   //                 
            280,   //                 
            280,   //                
            280,   //                
            280,   //                
            280,   //                
            280,   //                
            280,   //                
            280,   //                
            280,   //                 
            280,   //                 
            280,   //                 
            280,   //                 
            280,   //                 
            280,   //                 
            280,   //                 
            280,   //                 
            280,   //                 
            280,   //                 
            280,   //                 
            280,   //                 
            280,   //                 
            280,   //                 
        },

        //                                 
        {
            { 2150 }, //                                      
            { 1850 }, //                                      
            { 1750 }  //                                     
        },

        //                            
        {
            0, 0
        },

        //                          
        {
            0
        }

    } //       
};

#endif
#endif


