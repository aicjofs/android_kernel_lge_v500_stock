/*
 * Copyright 2008-2010 Analog Devices Inc.
 *
 * Licensed under the ADI BSD license or the GPL-2 (or later)
 */

#ifndef _DEF_BF539_H
#define _DEF_BF539_H

#include "defBF538.h"

/*                                                      */

#define	MXVR_CONFIG	      0xFFC02700  /*                             */
#define	MXVR_PLL_CTL_0	      0xFFC02704  /*                                         */

#define	MXVR_STATE_0	      0xFFC02708  /*                       */
#define	MXVR_STATE_1	      0xFFC0270C  /*                       */

#define	MXVR_INT_STAT_0	      0xFFC02710  /*                                  */
#define	MXVR_INT_STAT_1	      0xFFC02714  /*                                  */

#define	MXVR_INT_EN_0	      0xFFC02718  /*                                  */
#define	MXVR_INT_EN_1	      0xFFC0271C  /*                                  */

#define	MXVR_POSITION	      0xFFC02720  /*                             */
#define	MXVR_MAX_POSITION     0xFFC02724  /*                                     */

#define	MXVR_DELAY	      0xFFC02728  /*                                */
#define	MXVR_MAX_DELAY	      0xFFC0272C  /*                                        */

#define	MXVR_LADDR	      0xFFC02730  /*                               */
#define	MXVR_GADDR	      0xFFC02734  /*                             */
#define	MXVR_AADDR	      0xFFC02738  /*                                 */

#define	MXVR_ALLOC_0	      0xFFC0273C  /*                                  */
#define	MXVR_ALLOC_1	      0xFFC02740  /*                                  */
#define	MXVR_ALLOC_2	      0xFFC02744  /*                                  */
#define	MXVR_ALLOC_3	      0xFFC02748  /*                                  */
#define	MXVR_ALLOC_4	      0xFFC0274C  /*                                  */
#define	MXVR_ALLOC_5	      0xFFC02750  /*                                  */
#define	MXVR_ALLOC_6	      0xFFC02754  /*                                  */
#define	MXVR_ALLOC_7	      0xFFC02758  /*                                  */
#define	MXVR_ALLOC_8	      0xFFC0275C  /*                                  */
#define	MXVR_ALLOC_9	      0xFFC02760  /*                                  */
#define	MXVR_ALLOC_10	      0xFFC02764  /*                                   */
#define	MXVR_ALLOC_11	      0xFFC02768  /*                                   */
#define	MXVR_ALLOC_12	      0xFFC0276C  /*                                   */
#define	MXVR_ALLOC_13	      0xFFC02770  /*                                   */
#define	MXVR_ALLOC_14	      0xFFC02774  /*                                   */

#define	MXVR_SYNC_LCHAN_0     0xFFC02778  /*                                                  */
#define	MXVR_SYNC_LCHAN_1     0xFFC0277C  /*                                                  */
#define	MXVR_SYNC_LCHAN_2     0xFFC02780  /*                                                  */
#define	MXVR_SYNC_LCHAN_3     0xFFC02784  /*                                                  */
#define	MXVR_SYNC_LCHAN_4     0xFFC02788  /*                                                  */
#define	MXVR_SYNC_LCHAN_5     0xFFC0278C  /*                                                  */
#define	MXVR_SYNC_LCHAN_6     0xFFC02790  /*                                                  */
#define	MXVR_SYNC_LCHAN_7     0xFFC02794  /*                                                  */

#define	MXVR_DMA0_CONFIG      0xFFC02798  /*                                     */
#define	MXVR_DMA0_START_ADDR  0xFFC0279C  /*                                            */
#define	MXVR_DMA0_COUNT	      0xFFC027A0  /*                                         */
#define	MXVR_DMA0_CURR_ADDR   0xFFC027A4  /*                                              */
#define	MXVR_DMA0_CURR_COUNT  0xFFC027A8  /*                                                 */

#define	MXVR_DMA1_CONFIG      0xFFC027AC  /*                                     */
#define	MXVR_DMA1_START_ADDR  0xFFC027B0  /*                                            */
#define	MXVR_DMA1_COUNT	      0xFFC027B4  /*                                         */
#define	MXVR_DMA1_CURR_ADDR   0xFFC027B8  /*                                              */
#define	MXVR_DMA1_CURR_COUNT  0xFFC027BC  /*                                                 */

#define	MXVR_DMA2_CONFIG      0xFFC027C0  /*                                     */
#define	MXVR_DMA2_START_ADDR  0xFFC027C4  /*                                            */
#define	MXVR_DMA2_COUNT	      0xFFC027C8  /*                                         */
#define	MXVR_DMA2_CURR_ADDR   0xFFC027CC  /*                                              */
#define	MXVR_DMA2_CURR_COUNT  0xFFC027D0  /*                                                 */

#define	MXVR_DMA3_CONFIG      0xFFC027D4  /*                                     */
#define	MXVR_DMA3_START_ADDR  0xFFC027D8  /*                                            */
#define	MXVR_DMA3_COUNT	      0xFFC027DC  /*                                         */
#define	MXVR_DMA3_CURR_ADDR   0xFFC027E0  /*                                              */
#define	MXVR_DMA3_CURR_COUNT  0xFFC027E4  /*                                                 */

#define	MXVR_DMA4_CONFIG      0xFFC027E8  /*                                     */
#define	MXVR_DMA4_START_ADDR  0xFFC027EC  /*                                            */
#define	MXVR_DMA4_COUNT	      0xFFC027F0  /*                                         */
#define	MXVR_DMA4_CURR_ADDR   0xFFC027F4  /*                                              */
#define	MXVR_DMA4_CURR_COUNT  0xFFC027F8  /*                                                 */

#define	MXVR_DMA5_CONFIG      0xFFC027FC  /*                                     */
#define	MXVR_DMA5_START_ADDR  0xFFC02800  /*                                            */
#define	MXVR_DMA5_COUNT	      0xFFC02804  /*                                         */
#define	MXVR_DMA5_CURR_ADDR   0xFFC02808  /*                                              */
#define	MXVR_DMA5_CURR_COUNT  0xFFC0280C  /*                                                 */

#define	MXVR_DMA6_CONFIG      0xFFC02810  /*                                     */
#define	MXVR_DMA6_START_ADDR  0xFFC02814  /*                                            */
#define	MXVR_DMA6_COUNT	      0xFFC02818  /*                                         */
#define	MXVR_DMA6_CURR_ADDR   0xFFC0281C  /*                                              */
#define	MXVR_DMA6_CURR_COUNT  0xFFC02820  /*                                                 */

#define	MXVR_DMA7_CONFIG      0xFFC02824  /*                                     */
#define	MXVR_DMA7_START_ADDR  0xFFC02828  /*                                            */
#define	MXVR_DMA7_COUNT	      0xFFC0282C  /*                                         */
#define	MXVR_DMA7_CURR_ADDR   0xFFC02830  /*                                              */
#define	MXVR_DMA7_CURR_COUNT  0xFFC02834  /*                                                 */

#define	MXVR_AP_CTL	      0xFFC02838  /*                                    */
#define	MXVR_APRB_START_ADDR  0xFFC0283C  /*                                                 */
#define	MXVR_APRB_CURR_ADDR   0xFFC02840  /*                                                   */
#define	MXVR_APTB_START_ADDR  0xFFC02844  /*                                                 */
#define	MXVR_APTB_CURR_ADDR   0xFFC02848  /*                                                   */

#define	MXVR_CM_CTL	      0xFFC0284C  /*                                       */
#define	MXVR_CMRB_START_ADDR  0xFFC02850  /*                                                    */
#define	MXVR_CMRB_CURR_ADDR   0xFFC02854  /*                                                */
#define	MXVR_CMTB_START_ADDR  0xFFC02858  /*                                                    */
#define	MXVR_CMTB_CURR_ADDR   0xFFC0285C  /*                                                */

#define	MXVR_RRDB_START_ADDR  0xFFC02860  /*                                             */
#define	MXVR_RRDB_CURR_ADDR   0xFFC02864  /*                                               */

#define	MXVR_PAT_DATA_0	      0xFFC02868  /*                              */
#define	MXVR_PAT_EN_0	      0xFFC0286C  /*                                */
#define	MXVR_PAT_DATA_1	      0xFFC02870  /*                              */
#define	MXVR_PAT_EN_1	      0xFFC02874  /*                                */

#define	MXVR_FRAME_CNT_0      0xFFC02878  /*                      */
#define	MXVR_FRAME_CNT_1      0xFFC0287C  /*                      */

#define	MXVR_ROUTING_0	      0xFFC02880  /*                               */
#define	MXVR_ROUTING_1	      0xFFC02884  /*                               */
#define	MXVR_ROUTING_2	      0xFFC02888  /*                               */
#define	MXVR_ROUTING_3	      0xFFC0288C  /*                               */
#define	MXVR_ROUTING_4	      0xFFC02890  /*                               */
#define	MXVR_ROUTING_5	      0xFFC02894  /*                               */
#define	MXVR_ROUTING_6	      0xFFC02898  /*                               */
#define	MXVR_ROUTING_7	      0xFFC0289C  /*                               */
#define	MXVR_ROUTING_8	      0xFFC028A0  /*                               */
#define	MXVR_ROUTING_9	      0xFFC028A4  /*                               */
#define	MXVR_ROUTING_10	      0xFFC028A8  /*                                */
#define	MXVR_ROUTING_11	      0xFFC028AC  /*                                */
#define	MXVR_ROUTING_12	      0xFFC028B0  /*                                */
#define	MXVR_ROUTING_13	      0xFFC028B4  /*                                */
#define	MXVR_ROUTING_14	      0xFFC028B8  /*                                */

#define	MXVR_PLL_CTL_1	      0xFFC028BC  /*                                         */
#define	MXVR_BLOCK_CNT	      0xFFC028C0  /*                    */
#define	MXVR_PLL_CTL_2	      0xFFC028C4  /*                                         */

#endif /*              */
