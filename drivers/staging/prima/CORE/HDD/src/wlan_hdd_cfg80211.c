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

/**========================================================================

  \file  wlan_hdd_cfg80211.c

  \brief WLAN Host Device Driver implementation

  Copyright 2008 (c) Qualcomm, Incorporated.  All Rights Reserved.

  Qualcomm Confidential and Proprietary.

  ========================================================================*/

/*                                                                           

                        


                                                                        
                                                                 


                                      


                                              
                                                                                     
                                              

                                                                       
                             
                                                                            */

#ifdef CONFIG_CFG80211

#include <linux/version.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/wireless.h>
#include <wlan_hdd_includes.h>
#include <net/arp.h>
#include <net/cfg80211.h>
#include <linux/wireless.h>
#include <wlan_hdd_wowl.h>
#include <aniGlobal.h>
#include "ccmApi.h"
#include "sirParams.h"
#include "dot11f.h"
#include "wlan_hdd_assoc.h"
#include "wlan_hdd_wext.h"
#include "sme_Api.h"
#include "wlan_hdd_p2p.h"
#include "wlan_hdd_cfg80211.h"
#include "wlan_hdd_hostapd.h"
#include "sapInternal.h"
#include "wlan_hdd_softap_tx_rx.h"
#include "wlan_hdd_main.h"
#ifdef WLAN_BTAMP_FEATURE
#include "bap_hdd_misc.h"
#endif
#include <qc_sap_ioctl.h>

#define g_mode_rates_size (12)
#define a_mode_rates_size (8)
#define FREQ_BASE_80211G          (2407)
#define FREQ_BAND_DIFF_80211G     (5)
#define MAX_SCAN_SSID 9
#define GET_IE_LEN_IN_BSS_DESC(lenInBss) ( lenInBss + sizeof(lenInBss) - \
        ((int) OFFSET_OF( tSirBssDescription, ieFields)))

#define HDD2GHZCHAN(freq, chan, flag)   {     \
    .band =  IEEE80211_BAND_2GHZ, \
    .center_freq = (freq), \
    .hw_value = (chan),\
    .flags = (flag), \
    .max_antenna_gain = 0 ,\
    .max_power = 30, \
}

#define HDD5GHZCHAN(freq, chan, flag)   {     \
    .band =  IEEE80211_BAND_5GHZ, \
    .center_freq = (freq), \
    .hw_value = (chan),\
    .flags = (flag), \
    .max_antenna_gain = 0 ,\
    .max_power = 30, \
}

#define HDD_G_MODE_RATETAB(rate, rate_id, flag)\
{\
    .bitrate = rate, \
    .hw_value = rate_id, \
    .flags = flag, \
}

static const u32 hdd_cipher_suites[] = 
{
    WLAN_CIPHER_SUITE_WEP40,
    WLAN_CIPHER_SUITE_WEP104,
    WLAN_CIPHER_SUITE_TKIP,
#ifdef FEATURE_WLAN_CCX
#define WLAN_CIPHER_SUITE_KRK 0x004096ff /*             */
    WLAN_CIPHER_SUITE_KRK,
    WLAN_CIPHER_SUITE_CCMP,
#else
    WLAN_CIPHER_SUITE_CCMP,
#endif
#ifdef FEATURE_WLAN_WAPI
    WLAN_CIPHER_SUITE_SMS4,
#endif
};

static inline int is_broadcast_ether_addr(const u8 *addr)
{
    return ((addr[0] == 0xff) && (addr[1] == 0xff) && (addr[2] == 0xff) &&
            (addr[3] == 0xff) && (addr[4] == 0xff) && (addr[5] == 0xff));
}

static struct ieee80211_channel hdd_channels_2_4_GHZ[] =
{  
    HDD2GHZCHAN(2412, 1, 0) ,
    HDD2GHZCHAN(2417, 2, 0) ,
    HDD2GHZCHAN(2422, 3, 0) ,
    HDD2GHZCHAN(2427, 4, 0) ,
    HDD2GHZCHAN(2432, 5, 0) ,
    HDD2GHZCHAN(2437, 6, 0) ,
    HDD2GHZCHAN(2442, 7, 0) ,
    HDD2GHZCHAN(2447, 8, 0) ,
    HDD2GHZCHAN(2452, 9, 0) ,
    HDD2GHZCHAN(2457, 10, 0) ,
    HDD2GHZCHAN(2462, 11, 0) ,
    HDD2GHZCHAN(2467, 12, 0) ,
    HDD2GHZCHAN(2472, 13, 0) ,
    HDD2GHZCHAN(2484, 14, 0) ,
};

#ifdef WLAN_FEATURE_P2P
static struct ieee80211_channel hdd_social_channels_2_4_GHZ[] =
{
    HDD2GHZCHAN(2412, 1, 0) ,
    HDD2GHZCHAN(2437, 6, 0) ,
    HDD2GHZCHAN(2462, 11, 0) ,
};
#endif

static struct ieee80211_channel hdd_channels_5_GHZ[] =
{
    HDD5GHZCHAN(5180, 36, 0) ,
    HDD5GHZCHAN(5200, 40, 0) ,
    HDD5GHZCHAN(5220, 44, 0) ,
    HDD5GHZCHAN(5240, 48, 0) ,
    HDD5GHZCHAN(5260, 52, 0) ,
    HDD5GHZCHAN(5280, 56, 0) ,
    HDD5GHZCHAN(5300, 60, 0) ,
    HDD5GHZCHAN(5320, 64, 0) ,
    HDD5GHZCHAN(5500,100, 0) ,
    HDD5GHZCHAN(5520,104, 0) ,
    HDD5GHZCHAN(5540,108, 0) ,
    HDD5GHZCHAN(5560,112, 0) ,
    HDD5GHZCHAN(5580,116, 0) ,
    HDD5GHZCHAN(5600,120, 0) ,
    HDD5GHZCHAN(5620,124, 0) ,
    HDD5GHZCHAN(5640,128, 0) ,
    HDD5GHZCHAN(5660,132, 0) ,
    HDD5GHZCHAN(5680,136, 0) ,
    HDD5GHZCHAN(5700,140, 0) ,
    HDD5GHZCHAN(5745,149, 0) ,
    HDD5GHZCHAN(5765,153, 0) ,
    HDD5GHZCHAN(5785,157, 0) ,
    HDD5GHZCHAN(5805,161, 0) ,
    HDD5GHZCHAN(5825,165, 0) ,
};

static struct ieee80211_rate g_mode_rates[] =
{
    HDD_G_MODE_RATETAB(10, 0x1, 0),    
    HDD_G_MODE_RATETAB(20, 0x2, 0),    
    HDD_G_MODE_RATETAB(55, 0x4, 0),    
    HDD_G_MODE_RATETAB(110, 0x8, 0),    
    HDD_G_MODE_RATETAB(60, 0x10, 0),    
    HDD_G_MODE_RATETAB(90, 0x20, 0),    
    HDD_G_MODE_RATETAB(120, 0x40, 0),    
    HDD_G_MODE_RATETAB(180, 0x80, 0),    
    HDD_G_MODE_RATETAB(240, 0x100, 0),    
    HDD_G_MODE_RATETAB(360, 0x200, 0),    
    HDD_G_MODE_RATETAB(480, 0x400, 0),    
    HDD_G_MODE_RATETAB(540, 0x800, 0),
};   

static struct ieee80211_rate a_mode_rates[] =
{
    HDD_G_MODE_RATETAB(60, 0x10, 0),    
    HDD_G_MODE_RATETAB(90, 0x20, 0),    
    HDD_G_MODE_RATETAB(120, 0x40, 0),    
    HDD_G_MODE_RATETAB(180, 0x80, 0),    
    HDD_G_MODE_RATETAB(240, 0x100, 0),    
    HDD_G_MODE_RATETAB(360, 0x200, 0),    
    HDD_G_MODE_RATETAB(480, 0x400, 0),    
    HDD_G_MODE_RATETAB(540, 0x800, 0),
};

static struct ieee80211_supported_band wlan_hdd_band_2_4_GHZ =
{
    .channels = hdd_channels_2_4_GHZ,
    .n_channels = ARRAY_SIZE(hdd_channels_2_4_GHZ),
    .band       = IEEE80211_BAND_2GHZ,
    .bitrates = g_mode_rates,
    .n_bitrates = g_mode_rates_size,
    .ht_cap.ht_supported   = 1,
    .ht_cap.cap            =  IEEE80211_HT_CAP_SGI_20
                            | IEEE80211_HT_CAP_GRN_FLD
                            | IEEE80211_HT_CAP_DSSSCCK40
                            | IEEE80211_HT_CAP_LSIG_TXOP_PROT,
    .ht_cap.ampdu_factor   = IEEE80211_HT_MAX_AMPDU_64K,
    .ht_cap.ampdu_density  = IEEE80211_HT_MPDU_DENSITY_16,
    .ht_cap.mcs.rx_mask    = { 0xff, 0, 0, 0, 0, 0, 0, 0, 0, 0, },
    .ht_cap.mcs.rx_highest = cpu_to_le16( 72 ),
    .ht_cap.mcs.tx_params  = IEEE80211_HT_MCS_TX_DEFINED,
};

#ifdef WLAN_FEATURE_P2P
static struct ieee80211_supported_band wlan_hdd_band_p2p_2_4_GHZ =
{
    .channels = hdd_social_channels_2_4_GHZ,
    .n_channels = ARRAY_SIZE(hdd_social_channels_2_4_GHZ),
    .band       = IEEE80211_BAND_2GHZ,
    .bitrates = g_mode_rates,
    .n_bitrates = g_mode_rates_size,
    .ht_cap.ht_supported   = 1,
    .ht_cap.cap            =  IEEE80211_HT_CAP_SGI_20
                            | IEEE80211_HT_CAP_GRN_FLD
                            | IEEE80211_HT_CAP_DSSSCCK40
                            | IEEE80211_HT_CAP_LSIG_TXOP_PROT,
    .ht_cap.ampdu_factor   = IEEE80211_HT_MAX_AMPDU_64K,
    .ht_cap.ampdu_density  = IEEE80211_HT_MPDU_DENSITY_16,
    .ht_cap.mcs.rx_mask    = { 0xff, 0, 0, 0, 0, 0, 0, 0, 0, 0, },
    .ht_cap.mcs.rx_highest = cpu_to_le16( 72 ),
    .ht_cap.mcs.tx_params  = IEEE80211_HT_MCS_TX_DEFINED,
};
#endif

static struct ieee80211_supported_band wlan_hdd_band_5_GHZ =
{
    .channels = hdd_channels_5_GHZ,
    .n_channels = ARRAY_SIZE(hdd_channels_5_GHZ),
    .band     = IEEE80211_BAND_5GHZ,
    .bitrates = a_mode_rates,
    .n_bitrates = a_mode_rates_size,
    .ht_cap.ht_supported   = 1,
    .ht_cap.cap            =  IEEE80211_HT_CAP_SGI_20
                            | IEEE80211_HT_CAP_GRN_FLD
                            | IEEE80211_HT_CAP_DSSSCCK40
                            | IEEE80211_HT_CAP_LSIG_TXOP_PROT
                            | IEEE80211_HT_CAP_SGI_40
                            | IEEE80211_HT_CAP_SUP_WIDTH_20_40,
    .ht_cap.ampdu_factor   = IEEE80211_HT_MAX_AMPDU_64K,
    .ht_cap.ampdu_density  = IEEE80211_HT_MPDU_DENSITY_16,
    .ht_cap.mcs.rx_mask    = { 0xff, 0, 0, 0, 0, 0, 0, 0, 0, 0, },
    .ht_cap.mcs.rx_highest = cpu_to_le16( 72 ),
    .ht_cap.mcs.tx_params  = IEEE80211_HT_MCS_TX_DEFINED,
};

/*                                                                       
                                                */
static const struct ieee80211_txrx_stypes
wlan_hdd_txrx_stypes[NUM_NL80211_IFTYPES] = {
    [NL80211_IFTYPE_STATION] = {
        .tx = 0xffff,
        .rx = BIT(SIR_MAC_MGMT_ACTION) |
            BIT(SIR_MAC_MGMT_PROBE_REQ),
    },
    [NL80211_IFTYPE_AP] = {
        .tx = 0xffff,
        .rx = BIT(SIR_MAC_MGMT_ASSOC_REQ) |
            BIT(SIR_MAC_MGMT_REASSOC_REQ) |
            BIT(SIR_MAC_MGMT_PROBE_REQ) |
            BIT(SIR_MAC_MGMT_DISASSOC) |
            BIT(SIR_MAC_MGMT_AUTH) |
            BIT(SIR_MAC_MGMT_DEAUTH) |
            BIT(SIR_MAC_MGMT_ACTION),
    },
#ifdef WLAN_FEATURE_P2P    
    [NL80211_IFTYPE_P2P_CLIENT] = {
        .tx = 0xffff,
        .rx = BIT(SIR_MAC_MGMT_ACTION) |
            BIT(SIR_MAC_MGMT_PROBE_REQ),
    },
    [NL80211_IFTYPE_P2P_GO] = {
        /*                                 */
        .tx = 0xffff,
        .rx = BIT(SIR_MAC_MGMT_ASSOC_REQ) |
            BIT(SIR_MAC_MGMT_REASSOC_REQ) |
            BIT(SIR_MAC_MGMT_PROBE_REQ) |
            BIT(SIR_MAC_MGMT_DISASSOC) |
            BIT(SIR_MAC_MGMT_AUTH) |
            BIT(SIR_MAC_MGMT_DEAUTH) |
            BIT(SIR_MAC_MGMT_ACTION),
    },
#endif    
};

static struct cfg80211_ops wlan_hdd_cfg80211_ops;

/*                                     */
struct index_data_rate_type
{
   v_U8_t   beacon_rate_index;
   v_U16_t  supported_rate[4];
};

/*                                                         
                                  
                                                                
                                        
                                                  
                                                              
                                            
                                                                  
 */
static const struct
{
   v_U8_t   beacon_rate_index;
   v_U16_t  supported_rate[3];
} supported_data_rate[] =
{
/*                                      */
   {2,   { 10,  10, 0}},
   {4,   { 20,  20, 0}},
   {11,  { 55,  55, 0}},
   {12,  { 60,  60, 0}},
   {18,  { 90,  90, 0}},
   {22,  {110,  55, 0}},
   {24,  {120, 120, 0}},
   {36,  {180, 180, 0}},
   {44,  {220, 220, 0}},
   {48,  {240, 240, 0}},
   {66,  {330, 240, 0}},
   {72,  {360, 240, 0}},
   {96,  {480, 240, 0}},
   {108, {540, 240, 0}}
};

/*                      */
static struct index_data_rate_type supported_mcs_rate[] =
{
/*                           */
   {0,  {65,  135,  72,  150}},
   {1,  {130, 270,  144, 300}},
   {2,  {195, 405,  217, 450}},
   {3,  {260, 540,  289, 600}},
   {4,  {390, 810,  433, 900}},
   {5,  {520, 1080, 578, 1200}},
   {6,  {585, 1215, 650, 1350}},
   {7,  {650, 1350, 722, 1500}}
};

extern struct net_device_ops net_ops_struct;

/*
                                   
                                                 
                          
                                                                    
 */
struct wiphy *wlan_hdd_cfg80211_init(int priv_size)
{
    struct wiphy *wiphy;
    ENTER();

    /* 
                             
     */
    wiphy = wiphy_new(&wlan_hdd_cfg80211_ops, priv_size);

    if (!wiphy)
    {
        /*                                                         */
        hddLog(VOS_TRACE_LEVEL_ERROR, "%s: wiphy init failed", __func__);
        return NULL;
    }

    return wiphy;
}

/*
                                          
                                                         
                                         
 */
int wlan_hdd_cfg80211_update_band(struct wiphy *wiphy, eCsrBand eBand)
{
    ENTER();
    switch(eBand)
    {
        case eCSR_BAND_24:
            wiphy->bands[IEEE80211_BAND_2GHZ] = &wlan_hdd_band_2_4_GHZ;
            wiphy->bands[IEEE80211_BAND_5GHZ] = NULL;
            break;
        case eCSR_BAND_5G:
#ifdef WLAN_FEATURE_P2P
            wiphy->bands[IEEE80211_BAND_2GHZ] = &wlan_hdd_band_p2p_2_4_GHZ;
#else
            wiphy->bands[IEEE80211_BAND_2GHZ] = NULL;
#endif
            wiphy->bands[IEEE80211_BAND_5GHZ] = &wlan_hdd_band_5_GHZ;
            break;
        case eCSR_BAND_ALL:
        default:
            wiphy->bands[IEEE80211_BAND_2GHZ] = &wlan_hdd_band_2_4_GHZ;
            wiphy->bands[IEEE80211_BAND_5GHZ] = &wlan_hdd_band_5_GHZ;
    }
    return 0;
}
/*
                                   
                                                 
                          
                                                                    
 */
int wlan_hdd_cfg80211_register(struct device *dev, 
                               struct wiphy *wiphy,
                               hdd_config_t *pCfg
                               )
{
    ENTER();

    /*                                                */
    set_wiphy_dev(wiphy, dev);

    wiphy->mgmt_stypes = wlan_hdd_txrx_stypes;

    wiphy->flags |=   WIPHY_FLAG_HAVE_AP_SME 
                    | WIPHY_FLAG_AP_PROBE_RESP_OFFLOAD
                    | WIPHY_FLAG_CUSTOM_REGULATORY;

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3,4,0))
    wiphy->flags |=   WIPHY_FLAG_HAS_REMAIN_ON_CHANNEL 
                    | WIPHY_FLAG_OFFCHAN_TX;
#endif

    wiphy->max_scan_ssids = MAX_SCAN_SSID; 
    
    wiphy->max_scan_ie_len = 200 ; //                    

    /*                                           */
    wiphy->interface_modes =   BIT(NL80211_IFTYPE_STATION) 
                             | BIT(NL80211_IFTYPE_ADHOC)
#ifdef WLAN_FEATURE_P2P
                             | BIT(NL80211_IFTYPE_P2P_CLIENT)
                             | BIT(NL80211_IFTYPE_P2P_GO)
#endif                       
                             | BIT(NL80211_IFTYPE_AP);

    /*                                                               
                    */
    if( !pCfg->ShortGI20MhzEnable )
    {
        wlan_hdd_band_2_4_GHZ.ht_cap.cap     &= ~IEEE80211_HT_CAP_SGI_20;
        wlan_hdd_band_5_GHZ.ht_cap.cap       &= ~IEEE80211_HT_CAP_SGI_20;
        wlan_hdd_band_p2p_2_4_GHZ.ht_cap.cap &= ~IEEE80211_HT_CAP_SGI_20;
    }

    if( !pCfg->ShortGI40MhzEnable )
    {
        wlan_hdd_band_5_GHZ.ht_cap.cap       &= ~IEEE80211_HT_CAP_SGI_40;
    }

    if( !pCfg->nChannelBondingMode5GHz )
    {
        wlan_hdd_band_5_GHZ.ht_cap.cap &= ~IEEE80211_HT_CAP_SUP_WIDTH_20_40;
    }

    /*                          */
    switch(pCfg->nBandCapability)
    {
        case eCSR_BAND_24:
            wiphy->bands[IEEE80211_BAND_2GHZ] = &wlan_hdd_band_2_4_GHZ;
            break;
        case eCSR_BAND_5G:
#ifdef WLAN_FEATURE_P2P
            wiphy->bands[IEEE80211_BAND_2GHZ] = &wlan_hdd_band_p2p_2_4_GHZ;
#endif
            wiphy->bands[IEEE80211_BAND_5GHZ] = &wlan_hdd_band_5_GHZ;
            break;
        case eCSR_BAND_ALL:
        default:
            wiphy->bands[IEEE80211_BAND_2GHZ] = &wlan_hdd_band_2_4_GHZ;
            wiphy->bands[IEEE80211_BAND_5GHZ] = &wlan_hdd_band_5_GHZ;
    }
    /*                                             */
    wiphy->cipher_suites = hdd_cipher_suites;
    wiphy->n_cipher_suites = ARRAY_SIZE(hdd_cipher_suites);

    /*                                 */
    wiphy->signal_type = CFG80211_SIGNAL_TYPE_MBM;

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,38))
#ifdef WLAN_FEATURE_P2P
    wiphy->max_remain_on_channel_duration = 1000;
#endif
#endif

    /*                                      */
    if (0 > wiphy_register(wiphy))
    {
        /*            */
        hddLog(VOS_TRACE_LEVEL_ERROR,"%s: wiphy register failed", __func__);
        return -EIO;
    }

    EXIT();
    return 0;
}     

/*                                                               
                                                                      
                  
*/
void wlan_hdd_cfg80211_post_voss_start(hdd_adapter_t* pAdapter)
{
#ifdef WLAN_FEATURE_P2P
    tHalHandle hHal = WLAN_HDD_GET_HAL_CTX(pAdapter);
    /*                                                       */
    v_U16_t type = (SIR_MAC_MGMT_FRAME << 2) | ( SIR_MAC_MGMT_ACTION << 4);

    ENTER();

   /*                                                                
                                                                       
                                                                   */
    /*                     */
    sme_RegisterMgmtFrame(hHal, pAdapter->sessionId, type, 
                         (v_U8_t*)GAS_INITIAL_REQ, GAS_INITIAL_REQ_SIZE );

    /*                      */
    sme_RegisterMgmtFrame(hHal, pAdapter->sessionId, type,
                         (v_U8_t*)GAS_INITIAL_RSP, GAS_INITIAL_RSP_SIZE );
    
    /*                      */
    sme_RegisterMgmtFrame(hHal, pAdapter->sessionId, type,
                         (v_U8_t*)GAS_COMEBACK_REQ, GAS_COMEBACK_REQ_SIZE );

    /*                       */
    sme_RegisterMgmtFrame(hHal, pAdapter->sessionId, type,
                         (v_U8_t*)GAS_COMEBACK_RSP, GAS_COMEBACK_RSP_SIZE );

    /*                   */
    sme_RegisterMgmtFrame(hHal, pAdapter->sessionId, type,
                         (v_U8_t*)P2P_PUBLIC_ACTION_FRAME, 
                                  P2P_PUBLIC_ACTION_FRAME_SIZE );

    /*            */
    sme_RegisterMgmtFrame(hHal, pAdapter->sessionId, type,
                         (v_U8_t*)P2P_ACTION_FRAME,
                                  P2P_ACTION_FRAME_SIZE );
#endif /*                  */
}

void wlan_hdd_cfg80211_pre_voss_stop(hdd_adapter_t* pAdapter)
{
#ifdef WLAN_FEATURE_P2P
    tHalHandle hHal = WLAN_HDD_GET_HAL_CTX(pAdapter);
    /*                                                       */
    v_U16_t type = (SIR_MAC_MGMT_FRAME << 2) | ( SIR_MAC_MGMT_ACTION << 4);

    ENTER();

   /*                                                                
                                                                       
                                                                   */
    /*                     */

    sme_DeregisterMgmtFrame(hHal, pAdapter->sessionId, type,
                          (v_U8_t*)GAS_INITIAL_REQ, GAS_INITIAL_REQ_SIZE );

    /*                      */
    sme_DeregisterMgmtFrame(hHal, pAdapter->sessionId, type,
                         (v_U8_t*)GAS_INITIAL_RSP, GAS_INITIAL_RSP_SIZE );
    
    /*                      */
    sme_DeregisterMgmtFrame(hHal, pAdapter->sessionId, type,
                         (v_U8_t*)GAS_COMEBACK_REQ, GAS_COMEBACK_REQ_SIZE );

    /*                       */
    sme_DeregisterMgmtFrame(hHal, pAdapter->sessionId, type,
                         (v_U8_t*)GAS_COMEBACK_RSP, GAS_COMEBACK_RSP_SIZE );

    /*                   */
    sme_DeregisterMgmtFrame(hHal, pAdapter->sessionId, type,
                         (v_U8_t*)P2P_PUBLIC_ACTION_FRAME, 
                                  P2P_PUBLIC_ACTION_FRAME_SIZE );

    /*            */
    sme_DeregisterMgmtFrame(hHal, pAdapter->sessionId, type,
                         (v_U8_t*)P2P_ACTION_FRAME,
                                  P2P_ACTION_FRAME_SIZE );
#endif /*                  */
}

#ifdef FEATURE_WLAN_WAPI
void wlan_hdd_cfg80211_set_key_wapi(hdd_adapter_t* pAdapter, u8 key_index,
                                     const u8 *mac_addr, u8 *key , int key_Len)
{
    hdd_station_ctx_t *pHddStaCtx = WLAN_HDD_GET_STATION_CTX_PTR(pAdapter);
    tCsrRoamSetKey  setKey;
    v_BOOL_t isConnected = TRUE;
    int status = 0;
    v_U32_t roamId= 0xFF;
    tANI_U8 *pKeyPtr = NULL;
    int n = 0;

    hddLog(VOS_TRACE_LEVEL_INFO, "%s: device_mode = %d\n",
                                        __func__,pAdapter->device_mode);

    setKey.keyId = key_index; //             
    setKey.encType  = eCSR_ENCRYPT_TYPE_WPI; //                    
    setKey.keyDirection = eSIR_TX_RX;  //                              
    setKey.paeRole = 0 ; //             
    if (!mac_addr || is_broadcast_ether_addr(mac_addr))
    {
        vos_set_macaddr_broadcast( (v_MACADDR_t *)setKey.peerMac );
    }
    else
    {
        isConnected = hdd_connIsConnected(pHddStaCtx);
        vos_mem_copy(setKey.peerMac,&pHddStaCtx->conn_info.bssId,WNI_CFG_BSSID_LEN);
    }
    setKey.keyLength = key_Len;
    pKeyPtr = setKey.Key;
    memcpy( pKeyPtr, key, key_Len);

    hddLog(VOS_TRACE_LEVEL_INFO,"\n%s: WAPI KEY LENGTH:0x%04x",
                                            __func__, key_Len);
    for (n = 0 ; n < key_Len; n++)
        hddLog(VOS_TRACE_LEVEL_INFO, "%s WAPI KEY Data[%d]:%02x ",
                                           __func__,n,setKey.Key[n]);

    pHddStaCtx->roam_info.roamingState = HDD_ROAM_STATE_SETTING_KEY;
    if ( isConnected )
    {
        status= sme_RoamSetKey( WLAN_HDD_GET_HAL_CTX(pAdapter),
                             pAdapter->sessionId, &setKey, &roamId );
    }
    if ( status != 0 )
    {
        VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR,
                  "[%4d] sme_RoamSetKey returned ERROR status= %d",
                                                __LINE__, status );
        pHddStaCtx->roam_info.roamingState = HDD_ROAM_STATE_NONE;
    }
}
#endif /*                  */

#if (LINUX_VERSION_CODE < KERNEL_VERSION(3,4,0))
int wlan_hdd_cfg80211_alloc_new_beacon(hdd_adapter_t *pAdapter, 
                                       beacon_data_t **ppBeacon,
                                       struct beacon_parameters *params)
#else
int wlan_hdd_cfg80211_alloc_new_beacon(hdd_adapter_t *pAdapter, 
                                       beacon_data_t **ppBeacon,
                                       struct cfg80211_beacon_data *params,
                                       int dtim_period)
#endif
{    
    int size;
    beacon_data_t *beacon = NULL;
    beacon_data_t *old = NULL;
    int head_len,tail_len;

    ENTER();
    if (params->head && !params->head_len)
        return -EINVAL;

    old = pAdapter->sessionCtx.ap.beacon;

    if (!params->head && !old)
        return -EINVAL;

    if (params->tail && !params->tail_len)
        return -EINVAL;

#if (LINUX_VERSION_CODE <= KERNEL_VERSION(2,6,38))
    /*                                                       */
    if (!params->dtim_period)
        return -EINVAL;
#endif

    if(params->head)
        head_len = params->head_len;
    else
        head_len = old->head_len;

    if(params->tail || !old)
        tail_len = params->tail_len;
    else
        tail_len = old->tail_len;

    size = sizeof(beacon_data_t) + head_len + tail_len;

    beacon = kzalloc(size, GFP_KERNEL);

    if( beacon == NULL )
        return -ENOMEM;

#if (LINUX_VERSION_CODE < KERNEL_VERSION(3,4,0))
    if(params->dtim_period || !old )
        beacon->dtim_period = params->dtim_period;
    else
        beacon->dtim_period = old->dtim_period;
#else
    if(dtim_period || !old )
        beacon->dtim_period = dtim_period;
    else
        beacon->dtim_period = old->dtim_period;
#endif
 
    beacon->head = ((u8 *) beacon) + sizeof(beacon_data_t);
    beacon->tail = beacon->head + head_len;
    beacon->head_len = head_len;
    beacon->tail_len = tail_len;

    if(params->head) {
        memcpy (beacon->head,params->head,beacon->head_len);
    }
    else { 
        if(old)
            memcpy (beacon->head,old->head,beacon->head_len);
    }
    
    if(params->tail) {
        memcpy (beacon->tail,params->tail,beacon->tail_len);
    }
    else {
       if(old)   
           memcpy (beacon->tail,old->tail,beacon->tail_len);
    }

    *ppBeacon = beacon;

    kfree(old);

    return 0;

}

v_U8_t* wlan_hdd_cfg80211_get_ie_ptr(v_U8_t *pIes, int length, v_U8_t eid)
{
    int left = length;
    v_U8_t *ptr = pIes;
    v_U8_t elem_id,elem_len;
   
    while(left >= 2)
    {   
        elem_id  =  ptr[0];
        elem_len =  ptr[1];
        left -= 2;
        if(elem_len > left)
        {
            hddLog(VOS_TRACE_LEVEL_FATAL,
                    FL("****Invalid IEs eid = %d elem_len=%d left=%d*****"),
                                                    eid,elem_len,left);
            return NULL;
        }
        if (elem_id == eid) 
        {
            return ptr;
        }
   
        left -= elem_len;
        ptr += (elem_len + 2);
    }
    return NULL;
}

/*                                  */
static int wlan_hdd_rate_is_11g(u8 rate)
{
    u8 gRateArray[8] = {12, 18, 24, 36, 48, 72, 96, 104}; /*                 */
    u8 i;
    for (i = 0; i < 8; i++)
    {
        if(rate == gRateArray[i])
            return TRUE;
    }
    return FALSE;
}

/*                                                 */
static void wlan_hdd_check_11gmode(u8 *pIe, u8* require_ht,
                     u8* pCheckRatesfor11g, eSapPhyMode* pSapHw_mode)
{
    u8 i, num_rates = pIe[0];

    pIe += 1;
    for ( i = 0; i < num_rates; i++)
    {
        if( *pCheckRatesfor11g && (TRUE == wlan_hdd_rate_is_11g(pIe[i] & RATE_MASK)))
        {
            /*                                                       */
            *pSapHw_mode = eSAP_DOT11_MODE_11g;
            if (pIe[i] & BASIC_RATE_MASK)
            {
                /*                                                  
                                    
                 */
               *pSapHw_mode = eSAP_DOT11_MODE_11g_ONLY;
               *pCheckRatesfor11g = FALSE;
            }
        }
        else if((BASIC_RATE_MASK | WLAN_BSS_MEMBERSHIP_SELECTOR_HT_PHY) == pIe[i])
        {
            *require_ht = TRUE;
        }
    }
    return;
}

static void wlan_hdd_set_sapHwmode(hdd_adapter_t *pHostapdAdapter)
{
    tsap_Config_t *pConfig = &pHostapdAdapter->sessionCtx.ap.sapConfig;
    beacon_data_t *pBeacon = pHostapdAdapter->sessionCtx.ap.beacon;
    struct ieee80211_mgmt *pMgmt_frame = (struct ieee80211_mgmt*)pBeacon->head;
    u8 checkRatesfor11g = TRUE;
    u8 require_ht = FALSE;
    u8 *pIe=NULL;

    pConfig->SapHw_mode= eSAP_DOT11_MODE_11b;

    pIe = wlan_hdd_cfg80211_get_ie_ptr(&pMgmt_frame->u.beacon.variable[0],
                                       pBeacon->head_len, WLAN_EID_SUPP_RATES);
    if (pIe != NULL)
    {
        pIe += 1;
        wlan_hdd_check_11gmode(pIe, &require_ht, &checkRatesfor11g,
                               &pConfig->SapHw_mode);
    }

    pIe = wlan_hdd_cfg80211_get_ie_ptr(pBeacon->tail, pBeacon->tail_len,
                                WLAN_EID_EXT_SUPP_RATES);
    if (pIe != NULL)
    {

        pIe += 1;
        wlan_hdd_check_11gmode(pIe, &require_ht, &checkRatesfor11g,
                               &pConfig->SapHw_mode);
    }

    if( pConfig->channel > 14 )
    {
        pConfig->SapHw_mode= eSAP_DOT11_MODE_11a;
    }

    pIe = wlan_hdd_cfg80211_get_ie_ptr(pBeacon->tail, pBeacon->tail_len,
                                       WLAN_EID_HT_CAPABILITY);

    if(pIe) 
    {
        pConfig->SapHw_mode= eSAP_DOT11_MODE_11n;
        if(require_ht)
            pConfig->SapHw_mode= eSAP_DOT11_MODE_11n_ONLY;
    }
}

#if (LINUX_VERSION_CODE < KERNEL_VERSION(3,4,0))
static int wlan_hdd_cfg80211_update_apies(hdd_adapter_t* pHostapdAdapter,
                            struct beacon_parameters *params)
#else
static int wlan_hdd_cfg80211_update_apies(hdd_adapter_t* pHostapdAdapter,
                                     struct cfg80211_beacon_data *params)
#endif
{
    v_U8_t *genie;
    v_U8_t total_ielen = 0, ielen = 0;
    v_U8_t *pIe = NULL;
    v_U8_t addIE[1] = {0};
    beacon_data_t *pBeacon = pHostapdAdapter->sessionCtx.ap.beacon;
    int ret = 0;

    genie = vos_mem_malloc(MAX_GENIE_LEN);

    if(genie == NULL) {

        return -ENOMEM;
    }

    pIe = wlan_hdd_get_wps_ie_ptr(pBeacon->tail, pBeacon->tail_len);

    if(pIe)
    {
        /*               */
        ielen = pIe[1] + 2;
        if( ielen <=MAX_GENIE_LEN)
        {
            vos_mem_copy(genie, pIe, ielen);
        }
        else 
        {
            hddLog( VOS_TRACE_LEVEL_ERROR, "**Wps Ie Length is too big***\n");
            ret = -EINVAL;
            goto done;
        }
        total_ielen = ielen;
    }

#ifdef WLAN_FEATURE_WFD
    pIe = wlan_hdd_get_wfd_ie_ptr(pBeacon->tail,pBeacon->tail_len);

    if(pIe) 
    { 
        ielen = pIe[1] + 2;
        if(total_ielen + ielen <= MAX_GENIE_LEN) {
            vos_mem_copy(&genie[total_ielen],pIe,(pIe[1] + 2));
        }
        else {
           hddLog( VOS_TRACE_LEVEL_ERROR, "**Wps Ie + P2p Ie + Wfd Ie Length is too big***\n");
           ret = -EINVAL;
           goto done;
        }
        total_ielen += ielen; 
    }
#endif

#ifdef WLAN_FEATURE_P2P
    pIe = wlan_hdd_get_p2p_ie_ptr(pBeacon->tail,pBeacon->tail_len);

    if(pIe)
    {
        ielen = pIe[1] + 2;
        if(total_ielen + ielen <= MAX_GENIE_LEN)
        {
            vos_mem_copy(&genie[total_ielen], pIe, (pIe[1] + 2));
        }
        else
        {
            hddLog( VOS_TRACE_LEVEL_ERROR, 
                    "**Wps Ie+ P2pIE Length is too big***\n");
            ret = -EINVAL;
            goto done;
        }
        total_ielen += ielen;
    }
#endif

    if (ccmCfgSetStr((WLAN_HDD_GET_CTX(pHostapdAdapter))->hHal,
       WNI_CFG_PROBE_RSP_BCN_ADDNIE_DATA, genie, total_ielen, NULL,
               eANI_BOOLEAN_FALSE)==eHAL_STATUS_FAILURE)
    {
        hddLog(LOGE,
               "Could not pass on WNI_CFG_PROBE_RSP_BCN_ADDNIE_DATA to CCM");
        ret = -EINVAL;
        goto done;
    }

    if (ccmCfgSetInt((WLAN_HDD_GET_CTX(pHostapdAdapter))->hHal,
          WNI_CFG_PROBE_RSP_BCN_ADDNIE_FLAG, 1,NULL,
          test_bit(SOFTAP_BSS_STARTED, &pHostapdAdapter->event_flags) ?
                   eANI_BOOLEAN_TRUE : eANI_BOOLEAN_FALSE)
          ==eHAL_STATUS_FAILURE)
    {
        hddLog(LOGE,
            "Could not pass on WNI_CFG_PROBE_RSP_BCN_ADDNIE_FLAG to CCM");
        ret = -EINVAL;
        goto done;
    }

    //                     
    if ( (params->proberesp_ies != NULL) && (params->proberesp_ies_len != 0) )
    {
        u16 rem_probe_resp_ie_len = params->proberesp_ies_len;
        u8 probe_rsp_ie_len[3] = {0};
        u8 counter = 0;
        /*                                                             
                                                               
                                                                         
                                                       
        */
        while ((rem_probe_resp_ie_len > 0) && (counter < 3))
        {
            if (rem_probe_resp_ie_len > MAX_CFG_STRING_LEN)
            {
                probe_rsp_ie_len[counter++] = MAX_CFG_STRING_LEN;
                rem_probe_resp_ie_len -= MAX_CFG_STRING_LEN;
            }
            else
            {
                probe_rsp_ie_len[counter++] = rem_probe_resp_ie_len;
                rem_probe_resp_ie_len = 0;
            }
        }

        rem_probe_resp_ie_len = 0;

        if (probe_rsp_ie_len[0] > 0)
        {
            if (ccmCfgSetStr((WLAN_HDD_GET_CTX(pHostapdAdapter))->hHal,
                            WNI_CFG_PROBE_RSP_ADDNIE_DATA1,
                            (tANI_U8*)&params->proberesp_ies[rem_probe_resp_ie_len],
                            probe_rsp_ie_len[0], NULL,
                            eANI_BOOLEAN_FALSE) == eHAL_STATUS_FAILURE)
            {
                 hddLog(LOGE,
                       "Could not pass on WNI_CFG_PROBE_RSP_ADDNIE_DATA1 to CCM");
                 ret = -EINVAL;
                 goto done;
            }
            rem_probe_resp_ie_len += probe_rsp_ie_len[0];
        }

        if (probe_rsp_ie_len[1] > 0)
        {
            if (ccmCfgSetStr((WLAN_HDD_GET_CTX(pHostapdAdapter))->hHal,
                            WNI_CFG_PROBE_RSP_ADDNIE_DATA2,
                            (tANI_U8*)&params->proberesp_ies[rem_probe_resp_ie_len],
                            probe_rsp_ie_len[1], NULL,
                            eANI_BOOLEAN_FALSE) == eHAL_STATUS_FAILURE)
            {
                 hddLog(LOGE,
                       "Could not pass on WNI_CFG_PROBE_RSP_ADDNIE_DATA2 to CCM");
                 ret = -EINVAL;
                 goto done;
            }
            rem_probe_resp_ie_len += probe_rsp_ie_len[1];
        }

        if (probe_rsp_ie_len[2] > 0)
        {
            if (ccmCfgSetStr((WLAN_HDD_GET_CTX(pHostapdAdapter))->hHal,
                            WNI_CFG_PROBE_RSP_ADDNIE_DATA3,
                            (tANI_U8*)&params->proberesp_ies[rem_probe_resp_ie_len],
                            probe_rsp_ie_len[2], NULL,
                            eANI_BOOLEAN_FALSE) == eHAL_STATUS_FAILURE)
            {
                 hddLog(LOGE,
                       "Could not pass on WNI_CFG_PROBE_RSP_ADDNIE_DATA3 to CCM");
                 ret = -EINVAL;
                 goto done;
            }
            rem_probe_resp_ie_len += probe_rsp_ie_len[2];
        }

        if (probe_rsp_ie_len[1] == 0 )
        {
            if ( eHAL_STATUS_FAILURE == ccmCfgSetStr((WLAN_HDD_GET_CTX(pHostapdAdapter))->hHal,
                            WNI_CFG_PROBE_RSP_ADDNIE_DATA2, (tANI_U8*)addIE, 0, NULL,
                            eANI_BOOLEAN_FALSE) )
            {
                hddLog(LOGE,
                   "Could not pass on WNI_CFG_PROBE_RSP_ADDNIE_DATA2 to CCM\n");
            }
        }

        if (probe_rsp_ie_len[2] == 0 )
        {
            if ( eHAL_STATUS_FAILURE == ccmCfgSetStr((WLAN_HDD_GET_CTX(pHostapdAdapter))->hHal,
                            WNI_CFG_PROBE_RSP_ADDNIE_DATA3, (tANI_U8*)addIE, 0, NULL,
                            eANI_BOOLEAN_FALSE) )
            {
                hddLog(LOGE,
                   "Could not pass on WNI_CFG_PROBE_RSP_ADDNIE_DATA3 to CCM\n");
            }
        }

        if (ccmCfgSetInt((WLAN_HDD_GET_CTX(pHostapdAdapter))->hHal,
             WNI_CFG_PROBE_RSP_ADDNIE_FLAG, 1,NULL,
             test_bit(SOFTAP_BSS_STARTED, &pHostapdAdapter->event_flags) ?
                      eANI_BOOLEAN_TRUE : eANI_BOOLEAN_FALSE)
             == eHAL_STATUS_FAILURE)
        {
           hddLog(LOGE,
             "Could not pass on WNI_CFG_PROBE_RSP_ADDNIE_FLAG to CCM");
           ret = -EINVAL;
           goto done;
        }
    }
    else
    {
        //                              
        wlan_hdd_reset_prob_rspies(pHostapdAdapter);

        hddLog(VOS_TRACE_LEVEL_INFO,
               "%s: No Probe Response IE received in set beacon",
               __func__);
    }

    //                       
    if ( (params->assocresp_ies != NULL) && (params->assocresp_ies_len != 0) )
    {
       if (ccmCfgSetStr((WLAN_HDD_GET_CTX(pHostapdAdapter))->hHal,
               WNI_CFG_ASSOC_RSP_ADDNIE_DATA, (tANI_U8*)params->assocresp_ies,
               params->assocresp_ies_len, NULL,
               eANI_BOOLEAN_FALSE) == eHAL_STATUS_FAILURE)
       {
            hddLog(LOGE,
                  "Could not pass on WNI_CFG_ASSOC_RSP_ADDNIE_DATA to CCM");
            ret = -EINVAL;
            goto done;
       }

       if (ccmCfgSetInt((WLAN_HDD_GET_CTX(pHostapdAdapter))->hHal,
          WNI_CFG_ASSOC_RSP_ADDNIE_FLAG, 1, NULL,
          test_bit(SOFTAP_BSS_STARTED, &pHostapdAdapter->event_flags) ?
                   eANI_BOOLEAN_TRUE : eANI_BOOLEAN_FALSE)
          == eHAL_STATUS_FAILURE)
       {
          hddLog(LOGE,
            "Could not pass on WNI_CFG_ASSOC_RSP_ADDNIE_FLAG to CCM");
          ret = -EINVAL;
          goto done;
       }
    }
    else
    {
        hddLog(VOS_TRACE_LEVEL_INFO,
               "%s: No Assoc Response IE received in set beacon",
               __func__);

        if ( eHAL_STATUS_FAILURE == ccmCfgSetInt((WLAN_HDD_GET_CTX(pHostapdAdapter))->hHal,
                            WNI_CFG_ASSOC_RSP_ADDNIE_FLAG, 0, NULL,
                            eANI_BOOLEAN_FALSE) )
        {
            hddLog(LOGE,
               "Could not pass on WNI_CFG_ASSOC_RSP_ADDNIE_FLAG to CCM\n");
        }
    }

done:
    vos_mem_free(genie);
    return 0;
}

/* 
                                                
                                              
                                  
                                                                 
                 
 */ 
static VOS_STATUS wlan_hdd_validate_operation_channel(hdd_adapter_t *pAdapter,int channel)
{
        
    v_U32_t num_ch = 0;
    u8 valid_ch[WNI_CFG_VALID_CHANNEL_LIST_LEN];
    u32 indx = 0;
    tHalHandle hHal = WLAN_HDD_GET_HAL_CTX(pAdapter);
     
    num_ch = WNI_CFG_VALID_CHANNEL_LIST_LEN;

    if (0 != ccmCfgGetStr(hHal, WNI_CFG_VALID_CHANNEL_LIST,
            valid_ch, &num_ch))
    {
        hddLog(VOS_TRACE_LEVEL_ERROR,
                "%s: failed to get valid channel list\n", __func__);
        return VOS_STATUS_E_FAILURE;
    }

    for (indx = 0; indx < num_ch; indx++)
    {
        if (channel == valid_ch[indx])
        {
            break;
        }
    }

    if (indx >= num_ch)
    {
        hddLog(VOS_TRACE_LEVEL_ERROR,
                "%s: Invalid Channel [%d] \n", __func__, channel);
        return VOS_STATUS_E_FAILURE;
    }
    return VOS_STATUS_SUCCESS;
         
}


#if (LINUX_VERSION_CODE < KERNEL_VERSION(3,4,0))
static int wlan_hdd_cfg80211_start_bss(hdd_adapter_t *pHostapdAdapter,
                            struct beacon_parameters *params)
#else
static int wlan_hdd_cfg80211_start_bss(hdd_adapter_t *pHostapdAdapter,
                                       struct cfg80211_beacon_data *params,
                                       const u8 *ssid, size_t ssid_len,
                                       enum nl80211_hidden_ssid hidden_ssid)
#endif
{
    tsap_Config_t *pConfig;
    beacon_data_t *pBeacon = NULL;
    struct ieee80211_mgmt *pMgmt_frame;
    v_U8_t *pIe=NULL;
    v_U16_t capab_info;
    eCsrAuthType RSNAuthType;
    eCsrEncryptionType RSNEncryptType;
    eCsrEncryptionType mcRSNEncryptType;
    int status = VOS_STATUS_SUCCESS;
    tpWLAN_SAPEventCB pSapEventCallback;
    hdd_hostapd_state_t *pHostapdState;
    v_U8_t wpaRsnIEdata[(SIR_MAC_MAX_IE_LENGTH * 2)+4];  //                                                                 
    v_CONTEXT_t pVosContext = (WLAN_HDD_GET_CTX(pHostapdAdapter))->pvosContext;
    struct qc_mac_acl_entry *acl_entry = NULL;
    v_SINT_t i;

    ENTER();

    pHostapdState = WLAN_HDD_GET_HOSTAP_STATE_PTR(pHostapdAdapter);

    pConfig = &pHostapdAdapter->sessionCtx.ap.sapConfig;

    pBeacon = pHostapdAdapter->sessionCtx.ap.beacon;

    pMgmt_frame = (struct ieee80211_mgmt*)pBeacon->head;

    pConfig->beacon_int =  pMgmt_frame->u.beacon.beacon_int;

    //                                                   
    //                                          

    /*                                         */
    pConfig->protEnabled = 
           (WLAN_HDD_GET_CTX(pHostapdAdapter))->cfg_ini->apProtEnabled;

    pConfig->dtim_period = pBeacon->dtim_period;

    hddLog(VOS_TRACE_LEVEL_INFO_HIGH,"****pConfig->dtim_period=%d***\n",
                                      pConfig->dtim_period);


    if (pHostapdAdapter->device_mode == WLAN_HDD_SOFTAP)
    {
        pIe = wlan_hdd_cfg80211_get_ie_ptr(pBeacon->tail, pBeacon->tail_len,
                                       WLAN_EID_COUNTRY);
        if(pIe)
        {
            tANI_BOOLEAN restartNeeded;
            tHalHandle hHal = WLAN_HDD_GET_HAL_CTX(pHostapdAdapter);
            pConfig->ieee80211d = 1;
            vos_mem_copy(pConfig->countryCode, &pIe[2], 3);
            sme_setRegInfo(hHal, pConfig->countryCode);
            sme_ResetCountryCodeInformation(hHal, &restartNeeded);
            /*
                                                               
                                          
             */
            if( AUTO_CHANNEL_SELECT != pConfig->channel )
            {
                if(VOS_STATUS_SUCCESS != wlan_hdd_validate_operation_channel(pHostapdAdapter,pConfig->channel))
                {
                    hddLog(VOS_TRACE_LEVEL_ERROR,
                            "%s: Invalid Channel [%d] \n", __func__, pConfig->channel);
                    return -EINVAL;
                }
            }
        }
        else
        {
            pConfig->ieee80211d = 0;
        }
    }
    else
    {
        pConfig->ieee80211d = 0;
    }
    pConfig->authType = eSAP_AUTO_SWITCH;

    capab_info = pMgmt_frame->u.beacon.capab_info;
    
    pConfig->privacy = (pMgmt_frame->u.beacon.capab_info & 
                        WLAN_CAPABILITY_PRIVACY) ? VOS_TRUE : VOS_FALSE;

    (WLAN_HDD_GET_AP_CTX_PTR(pHostapdAdapter))->uPrivacy = pConfig->privacy;

    /*                             */
    pIe = wlan_hdd_get_wps_ie_ptr(pBeacon->tail, pBeacon->tail_len);

    if(pIe)
    {
        if(pIe[1] < (2 + WPS_OUI_TYPE_SIZE))
        {
            hddLog( VOS_TRACE_LEVEL_ERROR, "**Wps Ie Length is too small***\n");
            return -EINVAL;
        }
        else if(memcmp(&pIe[2], WPS_OUI_TYPE, WPS_OUI_TYPE_SIZE) == 0)
        {
             hddLog( VOS_TRACE_LEVEL_INFO, "** WPS IE(len %d) ***", (pIe[1]+2));
             /*                                                               
                         
              */
              if(SAP_WPS_ENABLED_UNCONFIGURED == pIe[15])
              {
                  pConfig->wps_state = SAP_WPS_ENABLED_UNCONFIGURED;
              } else if(SAP_WPS_ENABLED_CONFIGURED == pIe[15])
              {
                  pConfig->wps_state = SAP_WPS_ENABLED_CONFIGURED;
              }
        }
    }
    else
    {
        pConfig->wps_state = SAP_WPS_DISABLED;
    }
    pConfig->fwdWPSPBCProbeReq  = 1; //                                        

    pConfig->RSNWPAReqIELength = 0;
    pConfig->pRSNWPAReqIE = NULL;
    pIe = wlan_hdd_cfg80211_get_ie_ptr(pBeacon->tail, pBeacon->tail_len, 
                                       WLAN_EID_RSN);
    if(pIe && pIe[1])
    { 
        pConfig->RSNWPAReqIELength = pIe[1] + 2;
        memcpy(&wpaRsnIEdata[0], pIe, pConfig->RSNWPAReqIELength);
        pConfig->pRSNWPAReqIE = &wpaRsnIEdata[0];
        /*                                                             
                                                                      
                      
           */
        status = hdd_softap_unpackIE( 
                        vos_get_context( VOS_MODULE_ID_SME, pVosContext),
                        &RSNEncryptType,
                        &mcRSNEncryptType,
                        &RSNAuthType,
                        pConfig->pRSNWPAReqIE[1]+2,
                        pConfig->pRSNWPAReqIE );

        if( VOS_STATUS_SUCCESS == status )
        {
            /*                                                    
                          
               */
            pConfig->RSNEncryptType = RSNEncryptType; //                                  
            pConfig->mcRSNEncryptType = mcRSNEncryptType;
            (WLAN_HDD_GET_AP_CTX_PTR(pHostapdAdapter))->ucEncryptType
                                                              = RSNEncryptType;
            hddLog( LOG1, FL("%s: CSR AuthType = %d, "
                        "EncryptionType = %d mcEncryptionType = %d\n"),
                        RSNAuthType, RSNEncryptType, mcRSNEncryptType);
        }
    }
    
    pIe = wlan_hdd_get_vendor_oui_ie_ptr(WPA_OUI_TYPE, WPA_OUI_TYPE_SIZE,
                                         pBeacon->tail, pBeacon->tail_len);

    if(pIe && pIe[1] && (pIe[0] == DOT11F_EID_WPA))
    {
        if (pConfig->pRSNWPAReqIE)
        {
            /*                   */
            memcpy((&wpaRsnIEdata[0] + pConfig->RSNWPAReqIELength), pIe, pIe[1] + 2);
            pConfig->RSNWPAReqIELength += pIe[1] + 2;
        }
        else
        {
            pConfig->RSNWPAReqIELength = pIe[1] + 2;
            memcpy(&wpaRsnIEdata[0], pIe, pConfig->RSNWPAReqIELength);
            pConfig->pRSNWPAReqIE = &wpaRsnIEdata[0];
            status = hdd_softap_unpackIE( 
                          vos_get_context( VOS_MODULE_ID_SME, pVosContext),
                          &RSNEncryptType,
                          &mcRSNEncryptType,
                          &RSNAuthType,
                          pConfig->pRSNWPAReqIE[1]+2,
                          pConfig->pRSNWPAReqIE );

            if( VOS_STATUS_SUCCESS == status )
            {
                /*                                                    
                              
                   */
                pConfig->RSNEncryptType = RSNEncryptType; //                                  
                pConfig->mcRSNEncryptType = mcRSNEncryptType;
                (WLAN_HDD_GET_AP_CTX_PTR(pHostapdAdapter))->ucEncryptType
                                                              = RSNEncryptType;
                hddLog( LOG1, FL("%s: CSR AuthType = %d, "
                                "EncryptionType = %d mcEncryptionType = %d\n"),
                                RSNAuthType, RSNEncryptType, mcRSNEncryptType);
            }
        }
    }

    pConfig->SSIDinfo.ssidHidden = VOS_FALSE;

#if (LINUX_VERSION_CODE < KERNEL_VERSION(3,4,0))
    if (params->ssid != NULL)
    {
        memcpy(pConfig->SSIDinfo.ssid.ssId, params->ssid, params->ssid_len);
        pConfig->SSIDinfo.ssid.length = params->ssid_len;
        if (params->hidden_ssid != NL80211_HIDDEN_SSID_NOT_IN_USE)
            pConfig->SSIDinfo.ssidHidden = VOS_TRUE;
    }
#else
    if (ssid != NULL)
    {
        memcpy(pConfig->SSIDinfo.ssid.ssId, ssid, ssid_len);
        pConfig->SSIDinfo.ssid.length = ssid_len;
        if (hidden_ssid != NL80211_HIDDEN_SSID_NOT_IN_USE)
            pConfig->SSIDinfo.ssidHidden = VOS_TRUE;
    }
#endif

    vos_mem_copy(pConfig->self_macaddr.bytes, 
               pHostapdAdapter->macAddressCurrent.bytes, sizeof(v_MACADDR_t));
    
    /*               */
    pConfig->SapMacaddr_acl = eSAP_ACCEPT_UNLESS_DENIED;
    pConfig->num_accept_mac = 0;
    pConfig->num_deny_mac = 0;

    pIe = wlan_hdd_get_vendor_oui_ie_ptr(BLACKLIST_OUI_TYPE, WPA_OUI_TYPE_SIZE,
                                         pBeacon->tail, pBeacon->tail_len);

    /*                                      
                            
                            
                             
                             
                                                
                                                                                                
    */    
    if ((pIe != NULL) && (pIe[1] != 0))    
    {
        pConfig->SapMacaddr_acl = pIe[6];
        pConfig->num_deny_mac   = pIe[7];
        hddLog(VOS_TRACE_LEVEL_INFO,"acl type = %d no deny mac = %d\n",
                                     pIe[6], pIe[7]);
        if (pConfig->num_deny_mac > MAX_MAC_ADDRESS_DENIED)
            pConfig->num_deny_mac = MAX_MAC_ADDRESS_DENIED;
        acl_entry = (struct qc_mac_acl_entry *)(pIe + 8);
        for (i = 0; i < pConfig->num_deny_mac; i++)
        {
            vos_mem_copy(&pConfig->deny_mac[i], acl_entry->addr, sizeof(qcmacaddr));
            acl_entry++;
        }        
    }
    pIe = wlan_hdd_get_vendor_oui_ie_ptr(WHITELIST_OUI_TYPE, WPA_OUI_TYPE_SIZE,
                                         pBeacon->tail, pBeacon->tail_len);

    /*                                      
                            
                            
                             
                             
                                                
                                                                                                
    */    
    if ((pIe != NULL) && (pIe[1] != 0))    
    {
        pConfig->SapMacaddr_acl = pIe[6];
        pConfig->num_accept_mac   = pIe[7];
        hddLog(VOS_TRACE_LEVEL_INFO,"acl type = %d no accept mac = %d\n",
                                      pIe[6], pIe[7]);
        if (pConfig->num_accept_mac > MAX_MAC_ADDRESS_ACCEPTED)
            pConfig->num_accept_mac = MAX_MAC_ADDRESS_ACCEPTED;
        acl_entry = (struct qc_mac_acl_entry *)(pIe + 8);
        for (i = 0; i < pConfig->num_accept_mac; i++)
        {
            vos_mem_copy(&pConfig->accept_mac[i], acl_entry->addr, sizeof(qcmacaddr));
            acl_entry++;
        }
    }
    wlan_hdd_set_sapHwmode(pHostapdAdapter);

#ifdef WLAN_FEATURE_11AC
    if(((WLAN_HDD_GET_CTX(pHostapdAdapter))->cfg_ini->dot11Mode == eHDD_DOT11_MODE_AUTO) || 
       ((WLAN_HDD_GET_CTX(pHostapdAdapter))->cfg_ini->dot11Mode == eHDD_DOT11_MODE_11ac) ||
       ((WLAN_HDD_GET_CTX(pHostapdAdapter))->cfg_ini->dot11Mode == eHDD_DOT11_MODE_11ac_ONLY) )
    {
        pConfig->SapHw_mode = eSAP_DOT11_MODE_11ac;
    }
#endif

    //                                                                         
    pConfig->ht_capab =
                 (WLAN_HDD_GET_CTX(pHostapdAdapter))->cfg_ini->apProtection;

    if ( 0 != wlan_hdd_cfg80211_update_apies(pHostapdAdapter, params) )
    {
        hddLog(LOGE, FL("SAP Not able to set AP IEs"));
        return -EINVAL;
    }

    //                 
    pConfig->UapsdEnable =  
          (WLAN_HDD_GET_CTX(pHostapdAdapter))->cfg_ini->apUapsdEnabled;
    //                      
    pConfig->obssProtEnabled = 
           (WLAN_HDD_GET_CTX(pHostapdAdapter))->cfg_ini->apOBSSProtEnabled; 

    hddLog(LOGW, FL("SOftAP macaddress : "MAC_ADDRESS_STR"\n"), 
                 MAC_ADDR_ARRAY(pHostapdAdapter->macAddressCurrent.bytes));
    hddLog(LOGW,FL("ssid =%s\n"), pConfig->SSIDinfo.ssid.ssId);  
    hddLog(LOGW,FL("beaconint=%d, channel=%d\n"), (int)pConfig->beacon_int,
                                                     (int)pConfig->channel);
    hddLog(LOGW,FL("hw_mode=%x\n"),  pConfig->SapHw_mode);
    hddLog(LOGW,FL("privacy=%d, authType=%d\n"), pConfig->privacy, 
                                                 pConfig->authType); 
    hddLog(LOGW,FL("RSN/WPALen=%d, \n"),(int)pConfig->RSNWPAReqIELength);
    hddLog(LOGW,FL("Uapsd = %d\n"),pConfig->UapsdEnable); 
    hddLog(LOGW,FL("ProtEnabled = %d, OBSSProtEnabled = %d\n"),
                          pConfig->protEnabled, pConfig->obssProtEnabled);

    if(test_bit(SOFTAP_BSS_STARTED, &pHostapdAdapter->event_flags)) 
    {
        //                                 
        //                                                  
        hddLog( LOGE, "Bss Already started...Ignore the request");
        EXIT();
        return 0;
    }
    
    pConfig->persona = pHostapdAdapter->device_mode;

    pSapEventCallback = hdd_hostapd_SAPEventCB;
    if(WLANSAP_StartBss(pVosContext, pSapEventCallback, pConfig,
                 (v_PVOID_t)pHostapdAdapter->dev) != VOS_STATUS_SUCCESS)
    {
        hddLog(LOGE,FL("SAP Start Bss fail\n"));
        return -EINVAL;
    }

    hddLog(LOG1, 
           FL("Waiting for Scan to complete(auto mode) and BSS to start"));

    status = vos_wait_single_event(&pHostapdState->vosEvent, 10000);
   
    if (!VOS_IS_STATUS_SUCCESS(status))
    {  
        VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR, 
                 ("ERROR: HDD vos wait for single_event failed!!\n"));
        VOS_ASSERT(0);
    }
 
    //                                             
    set_bit(SOFTAP_BSS_STARTED, &pHostapdAdapter->event_flags);

    pHostapdState->bCommit = TRUE;
    EXIT();

   return 0;
}

#if (LINUX_VERSION_CODE < KERNEL_VERSION(3,4,0))
static int wlan_hdd_cfg80211_add_beacon(struct wiphy *wiphy, 
                                        struct net_device *dev, 
                                        struct beacon_parameters *params)
{
    hdd_adapter_t *pAdapter =  WLAN_HDD_GET_PRIV_PTR(dev);
    int status=VOS_STATUS_SUCCESS; 

    ENTER();

    hddLog(VOS_TRACE_LEVEL_INFO_HIGH, "device mode=%d\n",pAdapter->device_mode);

    if ( (WLAN_HDD_GET_CTX(pAdapter))->isLogpInProgress )
    {
        VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR,
                "%s:LOGP in Progress. Ignore!!!", __func__);
        return -EAGAIN;
    }

    if ( (pAdapter->device_mode == WLAN_HDD_SOFTAP) 
#ifdef WLAN_FEATURE_P2P
      || (pAdapter->device_mode == WLAN_HDD_P2P_GO)
#endif
       )
    {
        beacon_data_t *old,*new;

        old = pAdapter->sessionCtx.ap.beacon;
   
        if (old)
            return -EALREADY;

        status = wlan_hdd_cfg80211_alloc_new_beacon(pAdapter,&new,params);

        if(status != VOS_STATUS_SUCCESS) 
        {
             hddLog(VOS_TRACE_LEVEL_FATAL,
                   "%s:Error!!! Allocating the new beacon\n",__func__);
             return -EINVAL;
        }

        pAdapter->sessionCtx.ap.beacon = new;

        status = wlan_hdd_cfg80211_start_bss(pAdapter, params);
    }

    EXIT();
    return status;
}
 
static int wlan_hdd_cfg80211_set_beacon(struct wiphy *wiphy, 
                                        struct net_device *dev,
                                        struct beacon_parameters *params)
{
    hdd_adapter_t *pAdapter =  WLAN_HDD_GET_PRIV_PTR(dev); 
    int status=VOS_STATUS_SUCCESS;

    ENTER();

    hddLog(VOS_TRACE_LEVEL_INFO, "%s: device_mode = %d\n",
                                __func__,pAdapter->device_mode);

    if ( (WLAN_HDD_GET_CTX(pAdapter))->isLogpInProgress )
    {
        VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR,
                "%s:LOGP in Progress. Ignore!!!", __func__);
        return -EAGAIN;
    }

    if ((pAdapter->device_mode == WLAN_HDD_SOFTAP) 
#ifdef WLAN_FEATURE_P2P
     || (pAdapter->device_mode == WLAN_HDD_P2P_GO)
#endif
       ) 
    {
        beacon_data_t *old,*new;
                
        old = pAdapter->sessionCtx.ap.beacon;
        
        if (!old)
            return -ENOENT;

        status = wlan_hdd_cfg80211_alloc_new_beacon(pAdapter,&new,params);

        if(status != VOS_STATUS_SUCCESS) {
            hddLog(VOS_TRACE_LEVEL_FATAL, 
                   "%s: Error!!! Allocating the new beacon\n",__func__);
            return -EINVAL;
       }

       pAdapter->sessionCtx.ap.beacon = new;

       status = wlan_hdd_cfg80211_start_bss(pAdapter, params);
    }

    EXIT();
    return status;
}

#endif //                                            

#if (LINUX_VERSION_CODE < KERNEL_VERSION(3,4,0))
static int wlan_hdd_cfg80211_del_beacon(struct wiphy *wiphy,
                                        struct net_device *dev)
#else
static int wlan_hdd_cfg80211_stop_ap (struct wiphy *wiphy,
                                      struct net_device *dev)
#endif
{
    hdd_adapter_t *pAdapter =  WLAN_HDD_GET_PRIV_PTR(dev);
    hdd_context_t  *pHddCtx    = NULL;
    hdd_scaninfo_t *pScanInfo  = NULL;
    hdd_adapter_t  *staAdapter = NULL;
    VOS_STATUS status = 0;

    staAdapter = hdd_get_adapter(pAdapter->pHddCtx, WLAN_HDD_INFRA_STATION);

    if (!staAdapter)
    {
        staAdapter = hdd_get_adapter(pAdapter->pHddCtx, WLAN_HDD_P2P_CLIENT);
    }
    if ((WLAN_HDD_GET_CTX(pAdapter))->isLogpInProgress)
    {
        VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR,
                "%s:LOGP in Progress. Ignore!!!", __func__);
        return -EAGAIN;
    }

    if (staAdapter != NULL)
    {
        pScanInfo =  &staAdapter->scan_info;
    }

    ENTER();

    if (NULL == pAdapter)
    {
        VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_FATAL,
                   "%s: HDD adapter context is Null", __FUNCTION__);
        return -ENODEV;
    }
    if ((WLAN_HDD_GET_CTX(pAdapter))->isLogpInProgress)
    {
        VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_FATAL, "%s:LOGP in Progress. Ignore!!!",__func__);
        return -EAGAIN;
    }

    pHddCtx  =  (hdd_context_t*)pAdapter->pHddCtx;

    hddLog(VOS_TRACE_LEVEL_INFO, "%s: device_mode = %d\n",
                              __func__,pAdapter->device_mode);

    if ((pScanInfo != NULL) && pScanInfo->mScanPending)
    {
        INIT_COMPLETION(staAdapter->abortscan_event_var);
        hdd_abort_mac_scan(staAdapter->pHddCtx);
        status = wait_for_completion_interruptible_timeout(
                           &staAdapter->abortscan_event_var,
                           msecs_to_jiffies(WLAN_WAIT_TIME_ABORTSCAN));
        if (!status)
        {
            VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_FATAL,
                         "%s: Timeout occured while waiting for abortscan" ,
                         __FUNCTION__);
            VOS_ASSERT(pScanInfo->mScanPending);
            return 0;
        }
    }

    if ((pAdapter->device_mode == WLAN_HDD_SOFTAP)
#ifdef WLAN_FEATURE_P2P
     || (pAdapter->device_mode == WLAN_HDD_P2P_GO)
#endif
       )
    {
        beacon_data_t *old;

        old = pAdapter->sessionCtx.ap.beacon;

        if (!old)
            return -ENOENT;

#ifdef CONFIG_CFG80211
        hdd_cleanup_actionframe(pHddCtx, pAdapter);
#endif

        mutex_lock(&pHddCtx->sap_lock);
        if(test_bit(SOFTAP_BSS_STARTED, &pAdapter->event_flags))
        {
            if ( VOS_STATUS_SUCCESS == (status = WLANSAP_StopBss((WLAN_HDD_GET_CTX(pAdapter))->pvosContext) ) )
            {
                hdd_hostapd_state_t *pHostapdState = WLAN_HDD_GET_HOSTAP_STATE_PTR(pAdapter);

                status = vos_wait_single_event(&pHostapdState->vosEvent, 10000);

                if (!VOS_IS_STATUS_SUCCESS(status))
                {
                    VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR,
                             ("ERROR: HDD vos wait for single_event failed!!\n"));
                    VOS_ASSERT(0);
                }
            }
            clear_bit(SOFTAP_BSS_STARTED, &pAdapter->event_flags);
        }
        mutex_unlock(&pHddCtx->sap_lock);

        if(status != VOS_STATUS_SUCCESS)
        {
            hddLog(VOS_TRACE_LEVEL_FATAL,
                    "%s:Error!!! Stopping the BSS\n",__func__);
            return -EINVAL;
        }

        if (ccmCfgSetInt((WLAN_HDD_GET_CTX(pAdapter))->hHal,
            WNI_CFG_PROBE_RSP_BCN_ADDNIE_FLAG, 0,NULL, eANI_BOOLEAN_FALSE)
                                                    ==eHAL_STATUS_FAILURE)
        {
            hddLog(LOGE,
               "Could not pass on WNI_CFG_PROBE_RSP_BCN_ADDNIE_FLAG to CCM\n");
        }

        if ( eHAL_STATUS_FAILURE == ccmCfgSetInt((WLAN_HDD_GET_CTX(pAdapter))->hHal,
                            WNI_CFG_ASSOC_RSP_ADDNIE_FLAG, 0, NULL,
                            eANI_BOOLEAN_FALSE) )
        {
            hddLog(LOGE,
               "Could not pass on WNI_CFG_ASSOC_RSP_ADDNIE_FLAG to CCM\n");
        }

        //                              
        wlan_hdd_reset_prob_rspies(pAdapter);

        pAdapter->sessionCtx.ap.beacon = NULL;
        kfree(old);
    }
    EXIT();
    return status;
}

#if (LINUX_VERSION_CODE > KERNEL_VERSION(3,3,0))

static int wlan_hdd_cfg80211_start_ap(struct wiphy *wiphy, 
                                      struct net_device *dev, 
                                      struct cfg80211_ap_settings *params)
{
    hdd_adapter_t *pAdapter = WLAN_HDD_GET_PRIV_PTR(dev);
    int            status = VOS_STATUS_SUCCESS; 

    ENTER();

    hddLog(VOS_TRACE_LEVEL_INFO_HIGH, "device mode=%d\n", pAdapter->device_mode);

    if ((WLAN_HDD_GET_CTX(pAdapter))->isLogpInProgress)
    {
        VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_FATAL, "%s:LOGP in Progress. Ignore!!!",__func__);
        return -EAGAIN;
    }
    if ((pAdapter->device_mode == WLAN_HDD_SOFTAP) 
#ifdef WLAN_FEATURE_P2P
      || (pAdapter->device_mode == WLAN_HDD_P2P_GO)
#endif
       )
    {
        beacon_data_t  *old,*new;

        old = pAdapter->sessionCtx.ap.beacon;
   
        if (old)
            return -EALREADY;

        status = wlan_hdd_cfg80211_alloc_new_beacon(pAdapter, &new, &params->beacon, params->dtim_period);

        if(status != VOS_STATUS_SUCCESS) 
        {
             hddLog(VOS_TRACE_LEVEL_FATAL,
                   "%s:Error!!! Allocating the new beacon\n",__func__);
             return -EINVAL;
        }
        pAdapter->sessionCtx.ap.beacon = new;
        status = wlan_hdd_cfg80211_start_bss(pAdapter, &params->beacon, params->ssid,
                                             params->ssid_len, params->hidden_ssid);
    }

    EXIT();
    return status;
}


static int wlan_hdd_cfg80211_change_beacon(struct wiphy *wiphy, 
                                        struct net_device *dev,
                                        struct cfg80211_beacon_data *params)
{
    hdd_adapter_t *pAdapter =  WLAN_HDD_GET_PRIV_PTR(dev); 
    int status=VOS_STATUS_SUCCESS;

    ENTER();

    hddLog(VOS_TRACE_LEVEL_INFO, "%s: device_mode = %d\n",
                                __func__, pAdapter->device_mode);
    if ((WLAN_HDD_GET_CTX(pAdapter))->isLogpInProgress)
    {
        VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_FATAL, "%s:LOGP in Progress. Ignore!!!",__func__);
        return -EAGAIN;
    }

    if ((pAdapter->device_mode == WLAN_HDD_SOFTAP) 
#ifdef WLAN_FEATURE_P2P
     || (pAdapter->device_mode == WLAN_HDD_P2P_GO)
#endif
       ) 
    {
        beacon_data_t *old,*new;
                
        old = pAdapter->sessionCtx.ap.beacon;
        
        if (!old)
            return -ENOENT;

        status = wlan_hdd_cfg80211_alloc_new_beacon(pAdapter, &new, params, 0);

        if(status != VOS_STATUS_SUCCESS) {
            hddLog(VOS_TRACE_LEVEL_FATAL, 
                   "%s: Error!!! Allocating the new beacon\n",__func__);
            return -EINVAL;
       }

       pAdapter->sessionCtx.ap.beacon = new;

       status = wlan_hdd_cfg80211_start_bss(pAdapter, params, NULL, 0, 0);
    }

    EXIT();
    return status;
}

#endif //                                            


static int wlan_hdd_cfg80211_change_bss (struct wiphy *wiphy,
                                      struct net_device *dev,
                                      struct bss_parameters *params)
{
    hdd_adapter_t *pAdapter =  WLAN_HDD_GET_PRIV_PTR(dev);

    ENTER();
    
    hddLog(VOS_TRACE_LEVEL_INFO, "%s: device_mode = %d\n",
                               __func__,pAdapter->device_mode);

    if((pAdapter->device_mode == WLAN_HDD_SOFTAP)
#ifdef WLAN_FEATURE_P2P
     ||  (pAdapter->device_mode == WLAN_HDD_P2P_GO)
#endif
      ) 
    {
        /*                                                               
                                         */
        if (-1 != params->ap_isolate) 
        {
            pAdapter->sessionCtx.ap.apDisableIntraBssFwd = !!params->ap_isolate;
        } 
    }

    EXIT();
    return 0;
}

/*
                                           
                                                                         
 */
int wlan_hdd_cfg80211_change_iface( struct wiphy *wiphy,
                                    struct net_device *ndev,
                                    enum nl80211_iftype type,
                                    u32 *flags,
                                    struct vif_params *params
                                  )
{
    struct wireless_dev *wdev;
    hdd_adapter_t *pAdapter = WLAN_HDD_GET_PRIV_PTR( ndev );
    hdd_context_t *pHddCtx = WLAN_HDD_GET_CTX( pAdapter );
    tCsrRoamProfile *pRoamProfile = NULL;
    eCsrRoamBssType LastBSSType;
    hdd_config_t *pConfig = pHddCtx->cfg_ini;
    eMib_dot11DesiredBssType connectedBssType;
    VOS_STATUS status;

    ENTER();

    if ((WLAN_HDD_GET_CTX(pAdapter))->isLogpInProgress)
    {
        VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_FATAL, "%s:LOGP in Progress. Ignore!!!",__func__);
        return -EAGAIN;
    }

    hddLog(VOS_TRACE_LEVEL_INFO, "%s: device_mode = %d",
                             __func__, pAdapter->device_mode);

    wdev = ndev->ieee80211_ptr;

#ifdef WLAN_BTAMP_FEATURE
    if((NL80211_IFTYPE_P2P_CLIENT == type)||
       (NL80211_IFTYPE_ADHOC == type)||
       (NL80211_IFTYPE_AP == type)||
       (NL80211_IFTYPE_P2P_GO == type))
    {
        pHddCtx->isAmpAllowed = VOS_FALSE;
        //                 
        status = WLANBAP_StopAmp();
        if(VOS_STATUS_SUCCESS != status )
        {
            pHddCtx->isAmpAllowed = VOS_TRUE;
            hddLog(VOS_TRACE_LEVEL_FATAL,
                   "%s: Failed to stop AMP", __func__);
            return -EINVAL;
        }
    }
#endif //                  
    /*                                       */
    wlan_hdd_clear_concurrency_mode(pHddCtx, pAdapter->device_mode);

    if( (pAdapter->device_mode == WLAN_HDD_INFRA_STATION)
#ifdef WLAN_FEATURE_P2P
      || (pAdapter->device_mode == WLAN_HDD_P2P_CLIENT)
      || (pAdapter->device_mode == WLAN_HDD_P2P_DEVICE)
#endif
      )
    {
        hdd_wext_state_t *pWextState = WLAN_HDD_GET_WEXT_STATE_PTR(pAdapter);
        pRoamProfile = &pWextState->roamProfile;
        LastBSSType = pRoamProfile->BSSType;

        switch (type)
        {
            case NL80211_IFTYPE_STATION:
#ifdef WLAN_FEATURE_P2P
            case NL80211_IFTYPE_P2P_CLIENT:
#endif
                hddLog(VOS_TRACE_LEVEL_INFO,
                   "%s: setting interface Type to INFRASTRUCTURE", __func__);
                pRoamProfile->BSSType = eCSR_BSS_TYPE_INFRASTRUCTURE;
#ifdef WLAN_FEATURE_11AC
                if(pConfig->dot11Mode == eHDD_DOT11_MODE_AUTO)
                {
                    pConfig->dot11Mode = eHDD_DOT11_MODE_11ac;
                }
#endif
                pRoamProfile->phyMode = 
                hdd_cfg_xlate_to_csr_phy_mode(pConfig->dot11Mode);
                wdev->iftype = type;
#ifdef WLAN_FEATURE_P2P
                pAdapter->device_mode = (type == NL80211_IFTYPE_STATION) ?
                                WLAN_HDD_INFRA_STATION: WLAN_HDD_P2P_CLIENT;
#endif
                break;
            case NL80211_IFTYPE_ADHOC:
                hddLog(VOS_TRACE_LEVEL_INFO,
                  "%s: setting interface Type to ADHOC", __func__);
                pRoamProfile->BSSType = eCSR_BSS_TYPE_START_IBSS;
                pRoamProfile->phyMode =
                    hdd_cfg_xlate_to_csr_phy_mode(pConfig->dot11Mode);
                wdev->iftype = type;
                break;

            case NL80211_IFTYPE_AP:
#ifdef WLAN_FEATURE_P2P
            case NL80211_IFTYPE_P2P_GO:
#endif
            {
                hddLog(VOS_TRACE_LEVEL_INFO_HIGH,
                      "%s: setting interface Type to %s", __func__,
                      (type == NL80211_IFTYPE_AP) ? "SoftAP" : "P2pGo");

                //                    
                hdd_stop_adapter( pHddCtx, pAdapter );
                hdd_deinit_adapter( pHddCtx, pAdapter );
                memset(&pAdapter->sessionCtx, 0, sizeof(pAdapter->sessionCtx));
#ifdef WLAN_SOFTAP_FEATURE
#ifdef WLAN_FEATURE_P2P
                pAdapter->device_mode = (type == NL80211_IFTYPE_AP) ?
                                   WLAN_HDD_SOFTAP : WLAN_HDD_P2P_GO;
#else
                pAdapter->device_mode = WLAN_HDD_SOFTAP;
#endif

                //                                
                //                  
                if(WLAN_HDD_P2P_GO == pAdapter->device_mode)
                {
                    if(VOS_STATUS_E_FAILURE == 
                       hdd_disable_bmps_imps(pHddCtx, WLAN_HDD_P2P_GO))
                    {
                       //                 
                       VOS_ASSERT(0);
                    }
                }

                if ((WLAN_HDD_SOFTAP == pAdapter->device_mode) &&
                    (pConfig->apRandomBssidEnabled))
                {
                    /*                                                 
                                                                 */
                    get_random_bytes(&ndev->dev_addr[3], 3);
                    ndev->dev_addr[0] = 0x02;
                    ndev->dev_addr[1] = 0x1A;
                    ndev->dev_addr[2] = 0x11;
                    ndev->dev_addr[3] |= 0xF0;
                    memcpy(pAdapter->macAddressCurrent.bytes, ndev->dev_addr,
                           VOS_MAC_ADDR_SIZE);
                    pr_info("wlan: Generated HotSpot BSSID "
                            "%02x:%02x:%02x:%02x:%02x:%02x\n",
                            ndev->dev_addr[0],
                            ndev->dev_addr[1],
                            ndev->dev_addr[2],
                            ndev->dev_addr[3],
                            ndev->dev_addr[4],
                            ndev->dev_addr[5]);
                }

                hdd_set_ap_ops( pAdapter->dev );

                status = hdd_init_ap_mode(pAdapter);
                if(status != VOS_STATUS_SUCCESS)
                {
                    hddLog(VOS_TRACE_LEVEL_FATAL,
                           "%s: Error initializing the ap mode", __func__);
                    return -EINVAL;
                }
                hdd_set_conparam(1);

#endif
                /*                                                */
                if(wdev)
                {
                    wdev->iftype = type;
                    pHddCtx->change_iface = type;
                }
                else
                {
                    hddLog(VOS_TRACE_LEVEL_ERROR,
                            "%s: ERROR !!!! Wireless dev is NULL", __func__);
                    return -EINVAL;
                }
                goto done;
            }

            default:
                hddLog(VOS_TRACE_LEVEL_ERROR, "%s: Unsupported interface Type",
                        __func__);
                return -EOPNOTSUPP;
        }
    }
    else if ( (pAdapter->device_mode == WLAN_HDD_SOFTAP)
#ifdef WLAN_FEATURE_P2P
           || (pAdapter->device_mode == WLAN_HDD_P2P_GO)
#endif
            )
    {
       switch(type)
       {
           case NL80211_IFTYPE_STATION:
#ifdef WLAN_FEATURE_P2P
           case NL80211_IFTYPE_P2P_CLIENT:
#endif
           case NL80211_IFTYPE_ADHOC:
                hdd_stop_adapter( pHddCtx, pAdapter );
                hdd_deinit_adapter( pHddCtx, pAdapter );
                wdev->iftype = type;
#ifdef WLAN_FEATURE_P2P
                pAdapter->device_mode = (type == NL80211_IFTYPE_STATION) ?
                                  WLAN_HDD_INFRA_STATION: WLAN_HDD_P2P_CLIENT;
#endif
                hdd_set_conparam(0);
                pHddCtx->change_iface = type;
                memset(&pAdapter->sessionCtx, 0, sizeof(pAdapter->sessionCtx));
                hdd_set_station_ops( pAdapter->dev );
                status = hdd_init_station_mode( pAdapter );
                if( VOS_STATUS_SUCCESS != status )
                    return -EOPNOTSUPP;
                /*                                                                 
                                                                     
                 */
                hdd_enable_bmps_imps(pHddCtx);
                goto done;
            case NL80211_IFTYPE_AP:
#ifdef WLAN_FEATURE_P2P
            case NL80211_IFTYPE_P2P_GO:
#endif
                wdev->iftype = type;
#ifdef WLAN_FEATURE_P2P
                pAdapter->device_mode = (type == NL80211_IFTYPE_AP) ?
                                        WLAN_HDD_SOFTAP : WLAN_HDD_P2P_GO;
#endif
               goto done;
           default:
                hddLog(VOS_TRACE_LEVEL_ERROR, "%s: Unsupported interface Type",
                        __func__);
                return -EOPNOTSUPP;

       }

    }
    else
    {
      return -EOPNOTSUPP;
    }


    if(pRoamProfile)
    {
        if ( LastBSSType != pRoamProfile->BSSType )
        {
            /*                                                */
            wdev->iftype = type;

            /*                                                 
                                                      */
            if ( hdd_connGetConnectedBssType(
                 WLAN_HDD_GET_STATION_CTX_PTR(pAdapter), &connectedBssType ) ||
                ( eCSR_BSS_TYPE_START_IBSS == LastBSSType ) )
            {
                /*                                  */
                INIT_COMPLETION(pAdapter->disconnect_comp_var);
                if( eHAL_STATUS_SUCCESS ==
                        sme_RoamDisconnect( WLAN_HDD_GET_HAL_CTX(pAdapter),
                                pAdapter->sessionId,
                                eCSR_DISCONNECT_REASON_UNSPECIFIED ) )
                {
                    wait_for_completion_interruptible_timeout(
                              &pAdapter->disconnect_comp_var,
                              msecs_to_jiffies(WLAN_WAIT_TIME_DISCONNECT));
                }
            }
        }
    }

done:
    /*                                  */
    wlan_hdd_set_concurrency_mode(pHddCtx, pAdapter->device_mode);
#ifdef WLAN_BTAMP_FEATURE
    if((NL80211_IFTYPE_STATION == type) && (pHddCtx->concurrency_mode <= 1) && 
       (pHddCtx->no_of_sessions[WLAN_HDD_INFRA_STATION] <=1))
    {
        //                   
        pHddCtx->isAmpAllowed = VOS_TRUE;
    }
#endif //                  
    EXIT();
    return 0;
}

static int wlan_hdd_change_station(struct wiphy *wiphy,
                                         struct net_device *dev,
                                         u8 *mac,
                                         struct station_parameters *params)
{
    VOS_STATUS status = VOS_STATUS_SUCCESS;
    hdd_adapter_t *pAdapter = WLAN_HDD_GET_PRIV_PTR( dev );
    v_MACADDR_t STAMacAddress;

    ENTER();

    if ( (WLAN_HDD_GET_CTX(pAdapter))->isLogpInProgress )
    {
        VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR,
                "%s:LOGP in Progress. Ignore!!!", __func__);
        return -EAGAIN;
    }

    vos_mem_copy(STAMacAddress.bytes, mac, sizeof(v_MACADDR_t));

    if ( ( pAdapter->device_mode == WLAN_HDD_SOFTAP )
#ifdef WLAN_FEATURE_P2P
      || ( pAdapter->device_mode == WLAN_HDD_P2P_GO )
#endif
       )
    {
        if(params->sta_flags_set & BIT(NL80211_STA_FLAG_AUTHORIZED)) 
        {
            status = hdd_softap_change_STA_state( pAdapter, &STAMacAddress, 
                                                  WLANTL_STA_AUTHENTICATED);

            VOS_TRACE( VOS_MODULE_ID_HDD_SOFTAP, VOS_TRACE_LEVEL_ERROR,
                "%s: Station MAC address does not matching", __FUNCTION__);
            return -EINVAL;
        }
    }
    
    EXIT();
    return status;
}

/*
                                                   
                                                               
 */
static u8 wlan_hdd_cfg80211_get_ibss_peer_staidx(hdd_adapter_t* pAdapter)
{
    u8 idx = 0;
    u8 temp[VOS_MAC_ADDR_SIZE] = {0}; 
    ENTER();
    memset(temp, 0, VOS_MAC_ADDR_SIZE);
    for ( idx = 0; idx < HDD_MAX_NUM_IBSS_STA; idx++ )
    {
        if ( (0 != 
            (WLAN_HDD_GET_STATION_CTX_PTR(pAdapter))->conn_info.staId[idx]) 
          && memcmp((u8*)&(WLAN_HDD_GET_STATION_CTX_PTR(pAdapter))->conn_info.peerMacAddress[idx],
                                                     temp, VOS_MAC_ADDR_SIZE) 
           )
        {
            return idx;
        }
    }
    return idx;
}


/*
                                      
                                                          
 */
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,38))
static int wlan_hdd_cfg80211_add_key( struct wiphy *wiphy, 
                                      struct net_device *ndev,
                                      u8 key_index, bool pairwise,
                                      const u8 *mac_addr,
                                      struct key_params *params
                                      )
#else
static int wlan_hdd_cfg80211_add_key( struct wiphy *wiphy, 
                                      struct net_device *ndev,
                                      u8 key_index, const u8 *mac_addr,
                                      struct key_params *params
                                      )
#endif
{
    hdd_adapter_t *pAdapter = WLAN_HDD_GET_PRIV_PTR( ndev ); 
    tCsrRoamSetKey  setKey;
    u8 groupmacaddr[WNI_CFG_BSSID_LEN] = {0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};
    int status = 0;
    v_U32_t roamId= 0xFF; 
    v_CONTEXT_t pVosContext = (WLAN_HDD_GET_CTX(pAdapter))->pvosContext;  
    hdd_hostapd_state_t *pHostapdState;
    VOS_STATUS vos_status;

    ENTER();

    if ( (WLAN_HDD_GET_CTX(pAdapter))->isLogpInProgress )
    {
        VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR,
                "%s:LOGP in Progress. Ignore!!!", __func__);
        return -EAGAIN;
    }

    hddLog(VOS_TRACE_LEVEL_INFO, "%s: device_mode = %d\n",
            __func__,pAdapter->device_mode);

    if (CSR_MAX_NUM_KEY <= key_index)
    {
        hddLog(VOS_TRACE_LEVEL_ERROR, "%s: Invalid key index %d", __func__, 
                key_index);

        return -EINVAL;
    }

    hddLog(VOS_TRACE_LEVEL_INFO, 
            "%s: called with key index = %d & key length %d",
            __func__, key_index, params->key_len);

    /*                                */
    vos_mem_zero(&setKey,sizeof(tCsrRoamSetKey));
    setKey.keyId = key_index;
    setKey.keyLength = params->key_len;
    vos_mem_copy(&setKey.Key[0],params->key, params->key_len);

    switch (params->cipher) 
    {
        case WLAN_CIPHER_SUITE_WEP40:
            setKey.encType = eCSR_ENCRYPT_TYPE_WEP40_STATICKEY;
            break;

        case WLAN_CIPHER_SUITE_WEP104:
            setKey.encType = eCSR_ENCRYPT_TYPE_WEP104_STATICKEY;
            break;

        case WLAN_CIPHER_SUITE_TKIP:
            {
                u8 *pKey = &setKey.Key[0];
                setKey.encType = eCSR_ENCRYPT_TYPE_TKIP;

                vos_mem_zero(pKey, CSR_MAX_KEY_LEN);

                /*                                               

                                                        
                                                         
                                                        
                                                         

                */
                /*                                                     

                                                        
                                                         
                                                        
                                                         
                  */
                /*                               */
                vos_mem_copy(pKey, params->key,16);

                /*                     */
                vos_mem_copy(&pKey[16],&params->key[24],8); 

                /*                */
                vos_mem_copy(&pKey[24],&params->key[16],8); 


                break;
            }

        case WLAN_CIPHER_SUITE_CCMP:
            setKey.encType = eCSR_ENCRYPT_TYPE_AES;
            break;

#ifdef FEATURE_WLAN_WAPI
        case WLAN_CIPHER_SUITE_SMS4:
            {
                vos_mem_zero(&setKey,sizeof(tCsrRoamSetKey));
                wlan_hdd_cfg80211_set_key_wapi(pAdapter, key_index, mac_addr,
                        params->key, params->key_len);
                return 0;
            }
#endif
#ifdef FEATURE_WLAN_CCX
        case WLAN_CIPHER_SUITE_KRK:
            setKey.encType = eCSR_ENCRYPT_TYPE_KRK;
            break;
#endif
        default:
            hddLog(VOS_TRACE_LEVEL_ERROR, "%s: unsupported cipher type %lu",
                    __func__, params->cipher);
            return -EOPNOTSUPP;
    }

    hddLog(VOS_TRACE_LEVEL_INFO_MED, "%s: encryption type %d",
            __func__, setKey.encType);



    if ((pAdapter->device_mode == WLAN_HDD_SOFTAP)
#ifdef WLAN_FEATURE_P2P
            || (pAdapter->device_mode == WLAN_HDD_P2P_GO)
#endif
       )
    {


        if ( 
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,38))
                (!pairwise)
#else
                (!mac_addr || is_broadcast_ether_addr(mac_addr)) 
#endif
           )
        {
            /*              */
            VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_INFO,
                    "%s- %d: setting Broacast key",
                    __func__, __LINE__);
            setKey.keyDirection = eSIR_RX_ONLY;
            vos_mem_copy(setKey.peerMac,groupmacaddr,WNI_CFG_BSSID_LEN);
        }
        else
        {
            /*                 */
            VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_INFO,
                    "%s- %d: setting pairwise key",
                    __func__, __LINE__);
            setKey.keyDirection = eSIR_TX_RX;
            vos_mem_copy(setKey.peerMac, mac_addr,WNI_CFG_BSSID_LEN);
        }

        pHostapdState = WLAN_HDD_GET_HOSTAP_STATE_PTR(pAdapter);
        if( pHostapdState->bssState == BSS_START ) 
        { 
            status = WLANSAP_SetKeySta( pVosContext, &setKey);

            if ( status != eHAL_STATUS_SUCCESS )
            {
                VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR,
                        "[%4d] WLANSAP_SetKeySta returned ERROR status= %d",
                        __LINE__, status );
            }
        }

        /*                 */
        else if( eCSR_ENCRYPT_TYPE_WEP40_STATICKEY  == setKey.encType ||
                eCSR_ENCRYPT_TYPE_WEP104_STATICKEY  == setKey.encType  )
        {
            //                                                                      
            hdd_ap_ctx_t *pAPCtx = WLAN_HDD_GET_AP_CTX_PTR(pAdapter);
            vos_mem_copy(&pAPCtx->wepKey[key_index], &setKey, sizeof(tCsrRoamSetKey));
        }
        else
        {
            //                                                                  
            hdd_ap_ctx_t *pAPCtx = WLAN_HDD_GET_AP_CTX_PTR(pAdapter); 
            vos_mem_copy(&pAPCtx->groupKey, &setKey, sizeof(tCsrRoamSetKey));
        }
    }
    else if ( (pAdapter->device_mode == WLAN_HDD_INFRA_STATION) 
#ifdef WLAN_FEATURE_P2P
            || (pAdapter->device_mode == WLAN_HDD_P2P_CLIENT)
#endif
            )
    {
        hdd_wext_state_t *pWextState = WLAN_HDD_GET_WEXT_STATE_PTR(pAdapter);
        hdd_station_ctx_t *pHddStaCtx = WLAN_HDD_GET_STATION_CTX_PTR(pAdapter);

        pWextState->roamProfile.Keys.KeyLength[key_index] = (u8)params->key_len;

        vos_mem_copy(&pWextState->roamProfile.Keys.KeyMaterial[key_index][0], 
                params->key, params->key_len);

        pHddStaCtx->roam_info.roamingState = HDD_ROAM_STATE_SETTING_KEY;

    if (!(  ( IW_AUTH_KEY_MGMT_802_1X 
                    == (pWextState->authKeyMgmt & IW_AUTH_KEY_MGMT_802_1X)) 
                    && (eCSR_AUTH_TYPE_OPEN_SYSTEM == pHddStaCtx->conn_info.authType)
             )
                &&
                (  (WLAN_CIPHER_SUITE_WEP40 == params->cipher)
                   || (WLAN_CIPHER_SUITE_WEP104 == params->cipher)
                )
           )
        {
            /*                                                                
                                                                            
                                       */

            tCsrRoamProfile          *pRoamProfile = &pWextState->roamProfile;

            pWextState->roamProfile.negotiatedUCEncryptionType = 
                pHddStaCtx->conn_info.ucEncryptionType = 
                ((WLAN_CIPHER_SUITE_WEP40 == params->cipher) ?
                 eCSR_ENCRYPT_TYPE_WEP40_STATICKEY :
                 eCSR_ENCRYPT_TYPE_WEP104_STATICKEY);


            hddLog(VOS_TRACE_LEVEL_INFO_MED, 
                    "%s: Negotiated encryption type %d", __func__, 
                    pWextState->roamProfile.negotiatedUCEncryptionType);

            sme_SetCfgPrivacy((tpAniSirGlobal)WLAN_HDD_GET_HAL_CTX(pAdapter),
                    &pWextState->roamProfile, true);
            setKey.keyLength = 0;
            setKey.keyDirection =  eSIR_TX_RX;

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,38))
            if (pairwise) 
            {
#endif
                if (mac_addr)
                {
                    vos_mem_copy(setKey.peerMac, mac_addr,WNI_CFG_BSSID_LEN);
                }
                else
                {
                    /*                                                           
                                                         */
                    if (eCSR_BSS_TYPE_START_IBSS == pRoamProfile->BSSType)
                    {
                        u8 staidx = wlan_hdd_cfg80211_get_ibss_peer_staidx(pAdapter);
                        if (HDD_MAX_NUM_IBSS_STA != staidx)
                        {
                            vos_mem_copy(setKey.peerMac,
                                    &pHddStaCtx->conn_info.peerMacAddress[staidx],
                                    WNI_CFG_BSSID_LEN);

                        } 
                        else
                        {
                            hddLog(VOS_TRACE_LEVEL_ERROR, "%s: No peerMac found", 
                                    __func__);
                            return -EOPNOTSUPP;
                        } 
                    }
                    else
                    {
                        vos_mem_copy(setKey.peerMac,
                                &pHddStaCtx->conn_info.bssId[0],
                                WNI_CFG_BSSID_LEN);
                    }
                }
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,38))
            }
            else
            {
                /*              */
                VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_INFO,
                        "%s- %d: setting Group key",
                        __func__, __LINE__);
                setKey.keyDirection = eSIR_RX_ONLY;
                vos_mem_copy(setKey.peerMac, groupmacaddr, WNI_CFG_BSSID_LEN);
            }
#endif
        }
        else if (
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,38))
                (!pairwise)
#else
                (!mac_addr || is_broadcast_ether_addr(mac_addr)) 
#endif
                )
        {
            /*              */
            VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_INFO,
                    "%s- %d: setting Group key",
                    __func__, __LINE__);
            setKey.keyDirection = eSIR_RX_ONLY;
            vos_mem_copy(setKey.peerMac,groupmacaddr,WNI_CFG_BSSID_LEN);
        }
        else
        {
            /*                 */
            VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_INFO,
                    "%s- %d: setting pairwise key",
                    __func__, __LINE__);
            setKey.keyDirection = eSIR_TX_RX;
            vos_mem_copy(setKey.peerMac, mac_addr,WNI_CFG_BSSID_LEN);
        }

        hddLog(VOS_TRACE_LEVEL_INFO_MED, 
                "%s: set key for peerMac %2x:%2x:%2x:%2x:%2x:%2x, direction %d",
                __func__, setKey.peerMac[0], setKey.peerMac[1], 
                setKey.peerMac[2], setKey.peerMac[3], 
                setKey.peerMac[4], setKey.peerMac[5], 
                setKey.keyDirection);

        vos_status = wlan_hdd_check_ula_done(pAdapter);

        if ( vos_status != VOS_STATUS_SUCCESS )
        {
            VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR,
                    "[%4d] wlan_hdd_check_ula_done returned ERROR status= %d",
                    __LINE__, vos_status );

            pHddStaCtx->roam_info.roamingState = HDD_ROAM_STATE_NONE;

            return -EINVAL;

        }


        /*                             */
        status = sme_RoamSetKey( WLAN_HDD_GET_HAL_CTX(pAdapter),
                pAdapter->sessionId, &setKey, &roamId );

        if ( 0 != status )
        {
            hddLog(VOS_TRACE_LEVEL_ERROR, 
                    "%s: sme_RoamSetKey failed, returned %d", __func__, status);
            pHddStaCtx->roam_info.roamingState = HDD_ROAM_STATE_NONE;
            return -EINVAL;
        }


        /*                                                                             
                                                                                     
                             */
        if ( (eCSR_BSS_TYPE_START_IBSS == pWextState->roamProfile.BSSType) && 
            !(  ( IW_AUTH_KEY_MGMT_802_1X 
                    == (pWextState->authKeyMgmt & IW_AUTH_KEY_MGMT_802_1X)) 
                    && (eCSR_AUTH_TYPE_OPEN_SYSTEM == pHddStaCtx->conn_info.authType)
                 )
                &&
                (  (WLAN_CIPHER_SUITE_WEP40 == params->cipher)
                   || (WLAN_CIPHER_SUITE_WEP104 == params->cipher)
                )
           )
        {
            setKey.keyDirection = eSIR_RX_ONLY;
            vos_mem_copy(setKey.peerMac,groupmacaddr,WNI_CFG_BSSID_LEN);

            hddLog(VOS_TRACE_LEVEL_INFO_MED, 
                    "%s: set key peerMac %2x:%2x:%2x:%2x:%2x:%2x, direction %d",
                    __func__, setKey.peerMac[0], setKey.peerMac[1], 
                    setKey.peerMac[2], setKey.peerMac[3], 
                    setKey.peerMac[4], setKey.peerMac[5], 
                    setKey.keyDirection);

            status = sme_RoamSetKey( WLAN_HDD_GET_HAL_CTX(pAdapter), 
                    pAdapter->sessionId, &setKey, &roamId );

            if ( 0 != status )
            {
                hddLog(VOS_TRACE_LEVEL_ERROR, 
                        "%s: sme_RoamSetKey failed for group key (IBSS), returned %d", 
                        __func__, status);
                pHddStaCtx->roam_info.roamingState = HDD_ROAM_STATE_NONE;
                return -EINVAL;
            }
        }
    }

    return 0;
}

/*
                                      
                                                   
 */
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,38))
static int wlan_hdd_cfg80211_get_key( 
                        struct wiphy *wiphy, 
                        struct net_device *ndev,
                        u8 key_index, bool pairwise, 
                        const u8 *mac_addr, void *cookie,
                        void (*callback)(void *cookie, struct key_params*)
                        )
#else
static int wlan_hdd_cfg80211_get_key( 
                        struct wiphy *wiphy, 
                        struct net_device *ndev,
                        u8 key_index, const u8 *mac_addr, void *cookie,
                        void (*callback)(void *cookie, struct key_params*)
                        )
#endif
{
    hdd_adapter_t *pAdapter = WLAN_HDD_GET_PRIV_PTR( ndev ); 
    hdd_wext_state_t *pWextState= WLAN_HDD_GET_WEXT_STATE_PTR(pAdapter);
    tCsrRoamProfile *pRoamProfile = &(pWextState->roamProfile);
    struct key_params params;

    ENTER();
    
    hddLog(VOS_TRACE_LEVEL_INFO, "%s: device_mode = %d\n",
                                 __func__,pAdapter->device_mode);
    
    memset(&params, 0, sizeof(params));

    if (CSR_MAX_NUM_KEY <= key_index)
    {
        return -EINVAL;
    }   

    switch(pRoamProfile->EncryptionType.encryptionType[0])
    {
        case eCSR_ENCRYPT_TYPE_NONE:
            params.cipher = IW_AUTH_CIPHER_NONE;
            break;

        case eCSR_ENCRYPT_TYPE_WEP40_STATICKEY:
        case eCSR_ENCRYPT_TYPE_WEP40:
            params.cipher = WLAN_CIPHER_SUITE_WEP40;
            break;

        case eCSR_ENCRYPT_TYPE_WEP104_STATICKEY:
        case eCSR_ENCRYPT_TYPE_WEP104:
            params.cipher = WLAN_CIPHER_SUITE_WEP104;
            break;

        case eCSR_ENCRYPT_TYPE_TKIP:
            params.cipher = WLAN_CIPHER_SUITE_TKIP;
            break;

        case eCSR_ENCRYPT_TYPE_AES:
            params.cipher = WLAN_CIPHER_SUITE_AES_CMAC;
            break;

        default:
            params.cipher = IW_AUTH_CIPHER_NONE;
            break;
    }

    params.key_len = pRoamProfile->Keys.KeyLength[key_index];
    params.seq_len = 0;
    params.seq = NULL;
    params.key = &pRoamProfile->Keys.KeyMaterial[key_index][0];
    callback(cookie, &params);
    return 0;
}

/*
                                      
                                                      
 */
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,38))
static int wlan_hdd_cfg80211_del_key( struct wiphy *wiphy, 
                                      struct net_device *ndev,
                                      u8 key_index, 
                                      bool pairwise, 
                                      const u8 *mac_addr
                                    )
#else
static int wlan_hdd_cfg80211_del_key( struct wiphy *wiphy, 
                                      struct net_device *ndev,
                                      u8 key_index,
                                      const u8 *mac_addr
                                    )
#endif
{
    int status = 0;

    //                                                                      
    //                                                                   
    //                                             
    //                                                               
    //                                  
#if 0
    hdd_adapter_t *pAdapter = WLAN_HDD_GET_PRIV_PTR( ndev ); 
    v_CONTEXT_t pVosContext = (WLAN_HDD_GET_CTX(pAdapter))->pvosContext;  
    u8 groupmacaddr[WNI_CFG_BSSID_LEN] = {0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};
    tCsrRoamSetKey  setKey;
    v_U32_t roamId= 0xFF;
    
    ENTER();

    hddLog(VOS_TRACE_LEVEL_INFO_HIGH, "%s: device_mode = %d\n",
                                     __func__,pAdapter->device_mode);

    if (CSR_MAX_NUM_KEY <= key_index)
    {
        hddLog(VOS_TRACE_LEVEL_ERROR, "%s: Invalid key index %d", __func__, 
                key_index);

        return -EINVAL;
    }

    vos_mem_zero(&setKey,sizeof(tCsrRoamSetKey));
    setKey.keyId = key_index;

    if (mac_addr)
        vos_mem_copy(setKey.peerMac, mac_addr,WNI_CFG_BSSID_LEN);
    else
        vos_mem_copy(setKey.peerMac, groupmacaddr, WNI_CFG_BSSID_LEN);

    setKey.encType = eCSR_ENCRYPT_TYPE_NONE;

    if ((pAdapter->device_mode == WLAN_HDD_SOFTAP)
#ifdef WLAN_FEATURE_P2P
      || (pAdapter->device_mode == WLAN_HDD_P2P_GO)
#endif
       ) 
    { 
       
        hdd_hostapd_state_t *pHostapdState =  
                                  WLAN_HDD_GET_HOSTAP_STATE_PTR(pAdapter);
        if( pHostapdState->bssState == BSS_START)
        {
            status = WLANSAP_SetKeySta( pVosContext, &setKey);
    
            if ( status != eHAL_STATUS_SUCCESS )
            {
                VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR,
                     "[%4d] WLANSAP_SetKeySta returned ERROR status= %d",
                     __LINE__, status );
            }
        }
    }
    else if ( (pAdapter->device_mode == WLAN_HDD_INFRA_STATION)
#ifdef WLAN_FEATURE_P2P
           || (pAdapter->device_mode == WLAN_HDD_P2P_CLIENT) 
#endif
            )
    {
        hdd_station_ctx_t *pHddStaCtx = WLAN_HDD_GET_STATION_CTX_PTR(pAdapter);

        pHddStaCtx->roam_info.roamingState = HDD_ROAM_STATE_SETTING_KEY;   
    
        hddLog(VOS_TRACE_LEVEL_INFO_MED, 
                "%s: delete key for peerMac %2x:%2x:%2x:%2x:%2x:%2x",
                __func__, setKey.peerMac[0], setKey.peerMac[1], 
                setKey.peerMac[2], setKey.peerMac[3], 
                setKey.peerMac[4], setKey.peerMac[5]);
        if(pAdapter->sessionCtx.station.conn_info.connState == 
                                       eConnectionState_Associated) 
        {
            status = sme_RoamSetKey( WLAN_HDD_GET_HAL_CTX(pAdapter), 
                                   pAdapter->sessionId, &setKey, &roamId );
        
            if ( 0 != status )
            {
                hddLog(VOS_TRACE_LEVEL_ERROR, 
                        "%s: sme_RoamSetKey failure, returned %d",
                                                     __func__, status);
                pHddStaCtx->roam_info.roamingState = HDD_ROAM_STATE_NONE;
                return -EINVAL;
            }
        }
    }
#endif
    EXIT();
    return status;
}

/*
                                              
                                                        
 */
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,38))
static int wlan_hdd_cfg80211_set_default_key( struct wiphy *wiphy,
                                              struct net_device *ndev,
                                              u8 key_index,
                                              bool unicast, bool multicast)
#else
static int wlan_hdd_cfg80211_set_default_key( struct wiphy *wiphy,
                                              struct net_device *ndev,
                                              u8 key_index)
#endif
{
    hdd_adapter_t *pAdapter = WLAN_HDD_GET_PRIV_PTR( ndev ); 
    hdd_wext_state_t *pWextState = WLAN_HDD_GET_WEXT_STATE_PTR(pAdapter);
    int status = 0;
    hdd_station_ctx_t *pHddStaCtx = WLAN_HDD_GET_STATION_CTX_PTR(pAdapter);

    ENTER();

    hddLog(VOS_TRACE_LEVEL_INFO, "%s: device_mode = %d key_index = %d \n",
                                         __func__,pAdapter->device_mode, key_index);
   
    if (CSR_MAX_NUM_KEY <= key_index)
    {
        hddLog(VOS_TRACE_LEVEL_ERROR, "%s: Invalid key index %d", __func__, 
                key_index);

        return -EINVAL;
    }

    if ( (WLAN_HDD_GET_CTX(pAdapter))->isLogpInProgress )
    {
        VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR,
                "%s:LOGP in Progress. Ignore!!!", __func__);
        return -EAGAIN;
    }
    
    if ((pAdapter->device_mode == WLAN_HDD_INFRA_STATION)
#ifdef WLAN_FEATURE_P2P
     || (pAdapter->device_mode == WLAN_HDD_P2P_CLIENT)
#endif
       ) 
    {
        if ( (key_index != pWextState->roamProfile.Keys.defaultIndex) && 
             (eCSR_ENCRYPT_TYPE_TKIP != 
                pWextState->roamProfile.EncryptionType.encryptionType[0]) &&
             (eCSR_ENCRYPT_TYPE_AES != 
                pWextState->roamProfile.EncryptionType.encryptionType[0])
           )
        {  
            /*                                                   
                                                 */
    
            tCsrRoamSetKey  setKey;
            v_U32_t roamId= 0xFF;
            tCsrKeys *Keys = &pWextState->roamProfile.Keys;
            
            hddLog(VOS_TRACE_LEVEL_INFO_HIGH, "%s: default tx key index %d", 
                    __func__, key_index);
            
            Keys->defaultIndex = (u8)key_index;
            vos_mem_zero(&setKey,sizeof(tCsrRoamSetKey));
            setKey.keyId = key_index;
            setKey.keyLength = Keys->KeyLength[key_index];
            
            vos_mem_copy(&setKey.Key[0], 
                    &Keys->KeyMaterial[key_index][0], 
                    Keys->KeyLength[key_index]);
    
            setKey.keyDirection = eSIR_TX_ONLY;
    
            vos_mem_copy(setKey.peerMac, 
                    &pHddStaCtx->conn_info.bssId[0],
                    WNI_CFG_BSSID_LEN);
    
            setKey.encType = 
                pWextState->roamProfile.EncryptionType.encryptionType[0];
             
            /*                       */
            status = sme_RoamSetKey( WLAN_HDD_GET_HAL_CTX(pAdapter), 
                                   pAdapter->sessionId, &setKey, &roamId );
    
            if ( 0 != status )
            {
                hddLog(VOS_TRACE_LEVEL_ERROR, 
                        "%s: sme_RoamSetKey failed, returned %d", __func__, 
                        status);
                return -EINVAL;
            }
        }
    }

    /*                                                       */
    else if ( WLAN_HDD_SOFTAP == pAdapter->device_mode )
    {
        if ( (eCSR_ENCRYPT_TYPE_TKIP !=
                pWextState->roamProfile.EncryptionType.encryptionType[0]) &&
             (eCSR_ENCRYPT_TYPE_AES !=
                pWextState->roamProfile.EncryptionType.encryptionType[0])
           )
        {
            /*                                                           */
            hdd_ap_ctx_t *pAPCtx = WLAN_HDD_GET_AP_CTX_PTR(pAdapter);
            pAPCtx->wepKey[key_index].keyDirection = eSIR_TX_DEFAULT;
        }
    }
 
    return status;
}

/* 
                                          
                                                   
 */
int wlan_hdd_cfg80211_set_channel( struct wiphy *wiphy, struct net_device *dev,
                                   struct ieee80211_channel *chan,
                                   enum nl80211_channel_type channel_type
                                 )
{
    v_U32_t num_ch = 0;
    u32 channel = 0;
    hdd_adapter_t *pAdapter = WLAN_HDD_GET_PRIV_PTR( dev ); 
    int freq = chan->center_freq; /*                */ 
 
    ENTER();
    
    hddLog(VOS_TRACE_LEVEL_INFO, 
                "%s: device_mode = %d  freq = %d \n",__func__, 
                            pAdapter->device_mode, chan->center_freq);
    if ((WLAN_HDD_GET_CTX(pAdapter))->isLogpInProgress)
    {
        VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_FATAL, "%s:LOGP in Progress. Ignore!!!",__func__);
        return -EAGAIN;
    }

    /* 
                                 
                    
     */

    channel = ieee80211_frequency_to_channel(freq);
    
    /*                  */
    if ((WNI_CFG_CURRENT_CHANNEL_STAMIN > channel) || 
            (WNI_CFG_CURRENT_CHANNEL_STAMAX < channel)) 
    {
        hddLog(VOS_TRACE_LEVEL_ERROR, 
                "%s: Channel [%d] is outside valid range from %d to %d\n",
                __func__, channel, WNI_CFG_CURRENT_CHANNEL_STAMIN,
                WNI_CFG_CURRENT_CHANNEL_STAMAX);
        return -EINVAL;
    }

    num_ch = WNI_CFG_VALID_CHANNEL_LIST_LEN;

    if ((WLAN_HDD_SOFTAP != pAdapter->device_mode)
#ifdef WLAN_FEATURE_P2P
     && (WLAN_HDD_P2P_GO != pAdapter->device_mode)
#endif
      )
    {
        if(VOS_STATUS_SUCCESS != wlan_hdd_validate_operation_channel(pAdapter,channel))
        {
            hddLog(VOS_TRACE_LEVEL_ERROR,
                    "%s: Invalid Channel [%d] \n", __func__, channel);
            return -EINVAL;
        }
        hddLog(VOS_TRACE_LEVEL_INFO_HIGH, 
                "%s: set channel to [%d] for device mode =%d", 
                          __func__, channel,pAdapter->device_mode);
    }
    if( (pAdapter->device_mode == WLAN_HDD_INFRA_STATION)
#ifdef WLAN_FEATURE_P2P
     || (pAdapter->device_mode == WLAN_HDD_P2P_CLIENT)
#endif
      )
    {
        hdd_wext_state_t *pWextState = WLAN_HDD_GET_WEXT_STATE_PTR(pAdapter); 
        tCsrRoamProfile * pRoamProfile = &pWextState->roamProfile;
        hdd_station_ctx_t *pHddStaCtx = WLAN_HDD_GET_STATION_CTX_PTR(pAdapter);

        if (eConnectionState_IbssConnected == pHddStaCtx->conn_info.connState)
        {
           /*                                        */
            hddLog( VOS_TRACE_LEVEL_ERROR, 
                   "%s: IBSS Associated, can't set the channel\n", __func__);
            return -EINVAL;
        }

        num_ch = pRoamProfile->ChannelInfo.numOfChannels = 1;
        pHddStaCtx->conn_info.operationChannel = channel; 
        pRoamProfile->ChannelInfo.ChannelList = 
            &pHddStaCtx->conn_info.operationChannel; 
    }
    else if ((pAdapter->device_mode == WLAN_HDD_SOFTAP) 
#ifdef WLAN_FEATURE_P2P
        ||   (pAdapter->device_mode == WLAN_HDD_P2P_GO)
#endif
            ) 
    {
        (WLAN_HDD_GET_AP_CTX_PTR(pAdapter))->sapConfig.channel = channel;

        if ( WLAN_HDD_SOFTAP == pAdapter->device_mode )
        {
            hdd_config_t *cfg_param = (WLAN_HDD_GET_CTX(pAdapter))->cfg_ini;

            /*                                                                 
                                     
            */
            if ( cfg_param->apAutoChannelSelection )
            {
                (WLAN_HDD_GET_AP_CTX_PTR(pAdapter))->sapConfig.channel = AUTO_CHANNEL_SELECT;

                hddLog(VOS_TRACE_LEVEL_INFO_HIGH,
                       "%s: set channel to auto channel (0) for device mode =%d",
                       __func__, pAdapter->device_mode);
            }
        }
    }
    else 
    {
        hddLog(VOS_TRACE_LEVEL_FATAL, 
               "%s: Invalid device mode failed to set valid channel", __func__);
        return -EINVAL;
    }
    EXIT();
    return 0;
}



/*
                                         
                                                                        
 */
static struct cfg80211_bss* wlan_hdd_cfg80211_inform_bss(
                    hdd_adapter_t *pAdapter, tCsrRoamConnectedProfile *roamProfile)
{
    struct net_device *dev = pAdapter->dev;
    struct wireless_dev *wdev = dev->ieee80211_ptr;
    struct wiphy *wiphy = wdev->wiphy;
    tSirBssDescription *pBssDesc = roamProfile->pBssDesc;
    int chan_no;
    int ie_length;
    const char *ie;
    unsigned int freq;
    struct ieee80211_channel *chan;
    int rssi = 0;
    struct cfg80211_bss *bss = NULL;

    ENTER();

    if( NULL == pBssDesc )
    {
        hddLog(VOS_TRACE_LEVEL_FATAL, "%s: pBssDesc is NULL\n", __func__);
        return bss;
    }

    chan_no = pBssDesc->channelId;
    ie_length = GET_IE_LEN_IN_BSS_DESC( pBssDesc->length );
    ie =  ((ie_length != 0) ? (const char *)&pBssDesc->ieFields: NULL);

    if( NULL == ie )
    {
       hddLog(VOS_TRACE_LEVEL_FATAL, "%s: IE of BSS descriptor is NULL\n", __func__);
       return bss;
    }

#if (LINUX_VERSION_CODE > KERNEL_VERSION(2,6,38))
    if (chan_no <= ARRAY_SIZE(hdd_channels_2_4_GHZ))
    {
        freq = ieee80211_channel_to_frequency(chan_no, IEEE80211_BAND_2GHZ);
    }
    else
    {
        freq = ieee80211_channel_to_frequency(chan_no, IEEE80211_BAND_5GHZ);
    }
#else
    freq = ieee80211_channel_to_frequency(chan_no);
#endif

    chan = __ieee80211_get_channel(wiphy, freq);

    bss = cfg80211_get_bss(wiphy, chan, pBssDesc->bssId,
                           &roamProfile->SSID.ssId[0], roamProfile->SSID.length,
                           WLAN_CAPABILITY_ESS, WLAN_CAPABILITY_ESS);
    if (bss == NULL)
    {
        rssi = (VOS_MIN ((pBssDesc->rssi + pBssDesc->sinr), 0))*100;

        return (cfg80211_inform_bss(wiphy, chan, pBssDesc->bssId, 
                le64_to_cpu(*(__le64 *)pBssDesc->timeStamp), 
                pBssDesc->capabilityInfo,
                pBssDesc->beaconInterval, ie, ie_length,
                rssi, GFP_KERNEL ));
}
    else
    {
        return bss;
    }
}



/*
                                               
                                                                        
 */
struct cfg80211_bss*
wlan_hdd_cfg80211_inform_bss_frame( hdd_adapter_t *pAdapter,
                                    tSirBssDescription *bss_desc
                                    )
{
    /*
                                                                           
                                                                             
                                                                        
                                                                            
                                                                                
                                                                          
                                
     */
    struct net_device *dev = pAdapter->dev;
    struct wireless_dev *wdev = dev->ieee80211_ptr;
    struct wiphy *wiphy = wdev->wiphy;
    int chan_no = bss_desc->channelId;
    int ie_length = GET_IE_LEN_IN_BSS_DESC( bss_desc->length );
    const char *ie =
        ((ie_length != 0) ? (const char *)&bss_desc->ieFields: NULL);
    unsigned int freq;
    struct ieee80211_channel *chan;
    struct ieee80211_mgmt *mgmt =
        kzalloc((sizeof (struct ieee80211_mgmt) + ie_length), GFP_KERNEL);
    struct cfg80211_bss *bss_status = NULL;
    size_t frame_len = sizeof (struct ieee80211_mgmt) + ie_length;
    int rssi = 0;
#ifdef WLAN_OPEN_SOURCE
    struct timespec ts;
#endif

    ENTER();

    if (!mgmt)
        return NULL;

    memcpy(mgmt->bssid, bss_desc->bssId, ETH_ALEN);

#ifdef WLAN_OPEN_SOURCE
    /*                                                    
                                                     */
    get_monotonic_boottime(&ts);
    mgmt->u.probe_resp.timestamp =
         ((u64)ts.tv_sec * 1000000) + (ts.tv_nsec / 1000);
#else
    /*                                                 */
    memcpy(&mgmt->u.probe_resp.timestamp, bss_desc->timeStamp,
            sizeof (bss_desc->timeStamp));

#endif

    mgmt->u.probe_resp.beacon_int = bss_desc->beaconInterval;
    mgmt->u.probe_resp.capab_info = bss_desc->capabilityInfo;
    memcpy(mgmt->u.probe_resp.variable, ie, ie_length);

    mgmt->frame_control |=
        (u16)(IEEE80211_FTYPE_MGMT | IEEE80211_STYPE_PROBE_RESP);

#if (LINUX_VERSION_CODE > KERNEL_VERSION(2,6,38))
    if (chan_no <= ARRAY_SIZE(hdd_channels_2_4_GHZ) && 
        (wiphy->bands[IEEE80211_BAND_2GHZ] != NULL))
    {
        freq = ieee80211_channel_to_frequency(chan_no, IEEE80211_BAND_2GHZ);
    }
    else if ((chan_no > ARRAY_SIZE(hdd_channels_2_4_GHZ)) && 
        (wiphy->bands[IEEE80211_BAND_5GHZ] != NULL))

    {
        freq = ieee80211_channel_to_frequency(chan_no, IEEE80211_BAND_5GHZ);
    }
    else
    {
        kfree(mgmt);
        return NULL;
    }
#else
    freq = ieee80211_channel_to_frequency(chan_no);
#endif
    chan = __ieee80211_get_channel(wiphy, freq);

    /*                                                            
                                                        
      */
    if (( eConnectionState_Associated ==
             pAdapter->sessionCtx.station.conn_info.connState ) &&
             ( VOS_TRUE == vos_mem_compare(bss_desc->bssId,
                             pAdapter->sessionCtx.station.conn_info.bssId,
                             WNI_CFG_BSSID_LEN)))
    {
       /*                                                               */
       rssi = (pAdapter->rssi * 100);
    }
    else
    {
       rssi = (VOS_MIN ((bss_desc->rssi + bss_desc->sinr), 0))*100;
    }

    bss_status = cfg80211_inform_bss_frame(wiphy, chan, mgmt,
            frame_len, rssi, GFP_KERNEL);
    kfree(mgmt);
    return bss_status;
}

/*
                                            
                                                               
 */
struct cfg80211_bss* wlan_hdd_cfg80211_update_bss_db( hdd_adapter_t *pAdapter, 
                                      tCsrRoamInfo *pRoamInfo
                                      )
{
    tCsrRoamConnectedProfile roamProfile;
    tHalHandle hHal = WLAN_HDD_GET_HAL_CTX(pAdapter);
    struct cfg80211_bss *bss = NULL;

    ENTER();

    memset(&roamProfile, 0, sizeof(tCsrRoamConnectedProfile));
    sme_RoamGetConnectProfile(hHal, pAdapter->sessionId, &roamProfile);

    if (NULL != roamProfile.pBssDesc)
    {
        bss = wlan_hdd_cfg80211_inform_bss(pAdapter, 
                &roamProfile);

        if (NULL == bss)
        {
            hddLog(VOS_TRACE_LEVEL_INFO, "%s: cfg80211_inform_bss return NULL",
                    __func__);
        }

        sme_RoamFreeConnectProfile(hHal, &roamProfile);
    }
    else
    {
        hddLog(VOS_TRACE_LEVEL_ERROR, "%s:  roamProfile.pBssDesc is NULL",
                __func__);
    }
    return bss;
}

/*
                                         
 */
static int wlan_hdd_cfg80211_update_bss( struct wiphy *wiphy, 
                                         hdd_adapter_t *pAdapter 
                                        )
{   
    tHalHandle hHal = WLAN_HDD_GET_HAL_CTX(pAdapter);
    tCsrScanResultInfo *pScanResult;
    eHalStatus status = 0;
    tScanResultHandle pResult;
    struct cfg80211_bss *bss_status = NULL;

    ENTER();

    if ((WLAN_HDD_GET_CTX(pAdapter))->isLogpInProgress)
    {
      VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_FATAL, "%s:LOGP in Progress. Ignore!!!",__func__);
      return -EAGAIN;
    }

    /*
                                                                    
     */
    status = sme_ScanGetResult(hHal, pAdapter->sessionId, NULL, &pResult);

    /*                 */
    if (NULL == pResult)
    {
        hddLog(VOS_TRACE_LEVEL_INFO, "%s: No scan result\n", __func__);
        return status;
    }

    pScanResult = sme_ScanResultGetFirst(hHal, pResult);

    while (pScanResult)
    {
        /* 
                                                                           
                                                                      
                                                                             
                                                                           
                                                                          
                                                                          
                                                          
                                    
           */

        bss_status = wlan_hdd_cfg80211_inform_bss_frame(pAdapter,
                &pScanResult->BssDescriptor);
    

        if (NULL == bss_status)
        {
            hddLog(VOS_TRACE_LEVEL_INFO,
                    "%s: NULL returned by cfg80211_inform_bss\n", __func__);
        }
        else
        {
            cfg80211_put_bss(bss_status);
        }

        pScanResult = sme_ScanResultGetNext(hHal, pResult);
    }

    sme_ScanResultPurge(hHal, pResult); 

    return 0; 
}

void
hddPrintMacAddr(tCsrBssid macAddr, tANI_U8 logLevel)
{
    VOS_TRACE(VOS_MODULE_ID_HDD, logLevel, 
           "%X:%X:%X:%X:%X:%X\n",
           macAddr[0], macAddr[1], macAddr[2], macAddr[3], macAddr[4],
           macAddr[5]);
} /*                                 */

void
hddPrintPmkId(tCsrBssid pmkId, tANI_U8 logLevel)
{
    VOS_TRACE(VOS_MODULE_ID_HDD, logLevel, 
           "%X:%X:%X:%X:%X:%X:%X:%X:%X:%X:%X:%X:%X:%X:%X:%X\n",
           pmkId[0], pmkId[1], pmkId[2], pmkId[3], pmkId[4],
           pmkId[5], pmkId[6], pmkId[7], pmkId[8], pmkId[9],
           pmkId[10], pmkId[11], pmkId[12], pmkId[13], pmkId[14],
           pmkId[15]);
} /*                               */

//                                                     
//                                                

//                                                                                                 
//                                                                      

#define dump_bssid(bssid) \
    { \
        hddLog(VOS_TRACE_LEVEL_INFO, "BSSID (MAC) address:\t"); \
        hddPrintMacAddr(bssid, VOS_TRACE_LEVEL_INFO);\
        hddLog(VOS_TRACE_LEVEL_INFO, "\n"); \
    }

#define dump_pmkid(pMac, pmkid) \
    { \
        hddLog(VOS_TRACE_LEVEL_INFO, "PMKSA-ID:\t"); \
        hddPrintPmkId(pmkid, VOS_TRACE_LEVEL_INFO);\
        hddLog(VOS_TRACE_LEVEL_INFO, "\n"); \
    }

#ifdef FEATURE_WLAN_LFR
/*
                                                     
                                                                           
 */
int wlan_hdd_cfg80211_pmksa_candidate_notify(
                    hdd_adapter_t *pAdapter, tCsrRoamInfo *pRoamInfo, 
                    int index, bool preauth )
{
#ifdef FEATURE_WLAN_OKC
    struct net_device *dev = pAdapter->dev;

    ENTER();
    hddLog(VOS_TRACE_LEVEL_INFO, "%s is going to notify supplicant of:", __func__);

    if( NULL == pRoamInfo )
    {
        hddLog(VOS_TRACE_LEVEL_FATAL, "%s: pRoamInfo is NULL\n", __func__);
        return -EINVAL;
    }

    dump_bssid(pRoamInfo->bssid);
    cfg80211_pmksa_candidate_notify(dev, index,
                                    pRoamInfo->bssid, preauth, GFP_KERNEL);
#endif  /*                  */
    return 0; 
}
#endif //                

/*
                                            
                                                          
  
 */
static eHalStatus hdd_cfg80211_scan_done_callback(tHalHandle halHandle, 
        void *pContext, tANI_U32 scanId, eCsrScanStatus status)
{
    struct net_device *dev = (struct net_device *) pContext;
    //                                                   
    hdd_adapter_t *pAdapter = WLAN_HDD_GET_PRIV_PTR( dev );
    hdd_scaninfo_t *pScanInfo = &pAdapter->scan_info;
    struct cfg80211_scan_request *req = NULL;
    int ret = 0;

    ENTER();

    hddLog(VOS_TRACE_LEVEL_INFO,
            "%s called with halHandle = %p, pContext = %p,"
            "scanID = %d, returned status = %d\n", 
            __func__, halHandle, pContext, (int) scanId, (int) status);

    //                                                                 
    ret = wait_for_completion_interruptible_timeout(
                         &pScanInfo->scan_req_completion_event,
                         msecs_to_jiffies(WLAN_WAIT_TIME_SCAN_REQ));
    if (!ret)
    {
       VOS_ASSERT(pScanInfo->mScanPending);
       goto allow_suspend;
    }

    if(pScanInfo->mScanPending != VOS_TRUE)
    {
        VOS_ASSERT(pScanInfo->mScanPending);
        goto allow_suspend;
    }

    /*                  */
    if (pScanInfo->scanId != scanId) 
    {
        hddLog(VOS_TRACE_LEVEL_INFO,
                "%s called with mismatched scanId pScanInfo->scanId = %d "
                "scanId = %d \n", __func__, (int) pScanInfo->scanId, 
                (int) scanId);
    }

    ret = wlan_hdd_cfg80211_update_bss((WLAN_HDD_GET_CTX(pAdapter))->wiphy, 
                                        pAdapter);

    if (0 > ret)
        hddLog(VOS_TRACE_LEVEL_INFO, "%s: NO SCAN result", __func__);    


    /*                                            
                                      */
    if (pAdapter->scan_info.waitScanResult)
    {
        /*                                                       
                                                      */
        if(WEXT_SCAN_PENDING_DELAY == pAdapter->scan_info.scan_pending_option)
        {
            vos_event_set(&pAdapter->scan_info.scan_finished_event);
        }
        /*                            */
        else if(WEXT_SCAN_PENDING_PIGGYBACK == pAdapter->scan_info.scan_pending_option)
        {
            struct net_device *dev = pAdapter->dev;
            union iwreq_data wrqu;
            int we_event;
            char *msg;

            memset(&wrqu, '\0', sizeof(wrqu));
            we_event = SIOCGIWSCAN;
            msg = NULL;
            wireless_send_event(dev, we_event, &wrqu, msg);
        }
    }
    pAdapter->scan_info.waitScanResult = FALSE;

    /*                  */
    req = pAdapter->request;

    if (!req)
    {
        hddLog(VOS_TRACE_LEVEL_ERROR, "request is became NULL\n");
        pScanInfo->mScanPending = VOS_FALSE;
        goto allow_suspend;
    }

    /*
                                  
     */  
    req->n_ssids = 0;
    req->n_channels = 0;
    req->ie = 0;

    complete(&pAdapter->abortscan_event_var);
    pAdapter->request = NULL;
    /*                           */
    pScanInfo->mScanPending = VOS_FALSE;

    /*
                                                            
                  
     */
    cfg80211_scan_done(req, false);
#ifdef WLAN_FEATURE_P2P
    /*                                                */
    if(pScanInfo->flushP2pScanResults)
    {
        tANI_U8 sessionId = pAdapter->sessionId;
        sme_ScanFlushP2PResult(WLAN_HDD_GET_HAL_CTX(pAdapter), sessionId);
        pScanInfo->flushP2pScanResults = 0;
    }
#endif

allow_suspend:
    /*                                             */
    hdd_allow_suspend();

    EXIT();
    return 0;
}

/*
                                   
                                                                      
                                                               
 */
int wlan_hdd_cfg80211_scan( struct wiphy *wiphy, struct net_device *dev,
        struct cfg80211_scan_request *request)
{  
    hdd_adapter_t *pAdapter = WLAN_HDD_GET_PRIV_PTR( dev ); 
    hdd_context_t *pHddCtx = WLAN_HDD_GET_CTX( pAdapter );
    hdd_wext_state_t *pwextBuf = WLAN_HDD_GET_WEXT_STATE_PTR(pAdapter);
    hdd_config_t *cfg_param = (WLAN_HDD_GET_CTX(pAdapter))->cfg_ini;
    tCsrScanRequest scanRequest;
    tANI_U8 *channelList = NULL, i;
    v_U32_t scanId = 0;
    int status = 0;
    hdd_scaninfo_t *pScanInfo = &pAdapter->scan_info;
#ifdef WLAN_FEATURE_P2P
    v_U8_t* pP2pIe = NULL;
#endif

    ENTER();

    hddLog(VOS_TRACE_LEVEL_INFO, "%s: device_mode = %d\n",
                                   __func__,pAdapter->device_mode);
#ifdef WLAN_BTAMP_FEATURE
    //                                          
    if( VOS_TRUE == WLANBAP_AmpSessionOn() ) 
    {
        hddLog(VOS_TRACE_LEVEL_ERROR, 
                "%s: No scanning when AMP is on", __func__);
        return -EOPNOTSUPP;
    }
#endif
    //                                             
    if( pAdapter->device_mode == WLAN_HDD_SOFTAP ) 
    {
        hddLog(VOS_TRACE_LEVEL_ERROR, 
                "%s: Not scanning on device_mode = %d",
                                    __func__, pAdapter->device_mode);
        return -EOPNOTSUPP;
    }

    if (TRUE == pScanInfo->mScanPending)
    {
        hddLog(VOS_TRACE_LEVEL_INFO, "%s: mScanPending is TRUE", __func__);
        return -EBUSY;                  
    }

    if ((WLAN_HDD_GET_CTX(pAdapter))->isLogpInProgress)
    {
        VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_FATAL,
                  "%s:LOGP in Progress. Ignore!!!", __func__);
        return -EAGAIN;
    }

    if ((WLAN_HDD_GET_CTX(pAdapter))->isLoadUnloadInProgress)
    {
        VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_FATAL,
                  "%s:Unloading/Loading in Progress. Ignore!!!", __func__);
        return -EAGAIN;
    }
    //                                              
    //                                   
    //                                                 
    //                          
    if(0 != wlan_hdd_check_remain_on_channel(pAdapter))
    {
        hddLog(VOS_TRACE_LEVEL_INFO, "%s: Remain On Channel Pending", __func__);
        return -EBUSY;
    }

    if (mutex_lock_interruptible(&pHddCtx->tmInfo.tmOperationLock))
    {
        VOS_TRACE(VOS_MODULE_ID_HDD,VOS_TRACE_LEVEL_ERROR,
                  "%s: Aquire lock fail", __func__);
        return -EAGAIN;
    }    
    if (TRUE == pHddCtx->tmInfo.tmAction.enterImps)
    {
        hddLog(VOS_TRACE_LEVEL_WARN,
               "%s: MAX TM Level Scan not allowed", __func__);
        mutex_unlock(&pHddCtx->tmInfo.tmOperationLock);
        return -EBUSY;                  
    }
    mutex_unlock(&pHddCtx->tmInfo.tmOperationLock);

    vos_mem_zero( &scanRequest, sizeof(scanRequest));

    if (NULL != request)
    {
        hddLog(VOS_TRACE_LEVEL_INFO, "scan request for ssid = %d",
               (int)request->n_ssids);  

        /*                                                                       
                                                                                    
                                             
         */
        if (request->ssids && '\0' == request->ssids->ssid[0])
        {
            request->n_ssids = 0;
        }

        if (0 < request->n_ssids)
        {
            tCsrSSIDInfo *SsidInfo;
            int j;
            scanRequest.SSIDs.numOfSSIDs = request->n_ssids;
            /*                                          */
            SsidInfo = scanRequest.SSIDs.SSIDList =
                      ( tCsrSSIDInfo *)vos_mem_malloc(
                              request->n_ssids*sizeof(tCsrSSIDInfo));

            if(NULL == scanRequest.SSIDs.SSIDList)
            {
                hddLog(VOS_TRACE_LEVEL_ERROR,
                               "memory alloc failed SSIDInfo buffer");
                return -ENOMEM;
            }

            /*                                      */
            for(j = 0; j < request->n_ssids; j++, SsidInfo++)
            {
                /*                     */
                SsidInfo->SSID.length = request->ssids[j].ssid_len;
                vos_mem_copy(SsidInfo->SSID.ssId, &request->ssids[j].ssid[0],
                             SsidInfo->SSID.length);
                SsidInfo->SSID.ssId[SsidInfo->SSID.length] = '\0';
                hddLog(VOS_TRACE_LEVEL_INFO_HIGH, "SSID number %d:  %s",
                                                   j, SsidInfo->SSID.ssId);
            }
            /*                             */
            scanRequest.scanType = eSIR_ACTIVE_SCAN;
        }
        else if(WLAN_HDD_P2P_GO == pAdapter->device_mode)
        {
            /*                             */
            scanRequest.scanType = eSIR_ACTIVE_SCAN;
        }
        else
        {
            /*                                                            */
            scanRequest.scanType = pScanInfo->scan_mode;
        }
        scanRequest.minChnTime = cfg_param->nActiveMinChnTime; 
        scanRequest.maxChnTime = cfg_param->nActiveMaxChnTime;
    }
    else
    {
        /*                             */
        scanRequest.scanType = eSIR_ACTIVE_SCAN;
        vos_mem_set( scanRequest.bssid, sizeof( tCsrBssid ), 0xff );

        /*                                      */
        scanRequest.minChnTime = 0;
        scanRequest.maxChnTime = 0;
    }

    /*                             */
    scanRequest.BSSType = eCSR_BSS_TYPE_ANY;

    /*                                      */

    /*                                    */
    if( request )
    {
        if( request->n_channels )
        {
            channelList = vos_mem_malloc( request->n_channels );
            if( NULL == channelList )
            {
                status = -ENOMEM;
                goto free_mem;
            }

            for( i = 0 ; i < request->n_channels ; i++ )
                channelList[i] = request->channels[i]->hw_value;
        }

        scanRequest.ChannelInfo.numOfChannels = request->n_channels;
        scanRequest.ChannelInfo.ChannelList = channelList;

        /*                              */
        scanRequest.requestType = eCSR_SCAN_REQUEST_FULL_SCAN;

        if( request->ie_len )
        {
            /*                                                       */
            memset( &pScanInfo->scanAddIE, 0, sizeof(pScanInfo->scanAddIE) );
            memcpy( pScanInfo->scanAddIE.addIEdata, request->ie, request->ie_len);
            pScanInfo->scanAddIE.length = request->ie_len;

            if((WLAN_HDD_INFRA_STATION == pAdapter->device_mode) ||
                (WLAN_HDD_P2P_CLIENT == pAdapter->device_mode) ||
                (WLAN_HDD_P2P_DEVICE == pAdapter->device_mode)
              )
            {
               pwextBuf->roamProfile.pAddIEScan = pScanInfo->scanAddIE.addIEdata;
               pwextBuf->roamProfile.nAddIEScanLength = pScanInfo->scanAddIE.length;
            }

            scanRequest.uIEFieldLen = pScanInfo->scanAddIE.length;
            scanRequest.pIEField = pScanInfo->scanAddIE.addIEdata;

#ifdef WLAN_FEATURE_P2P
            pP2pIe = wlan_hdd_get_p2p_ie_ptr((v_U8_t*)request->ie,
                                                       request->ie_len);
            if (pP2pIe != NULL)
            {
                /*                                                         */
                if(TRUE == request->no_cck)
                {
                    tANI_U8 sessionId = pAdapter->sessionId;
                    hddLog(VOS_TRACE_LEVEL_INFO,
                           "%s: This is a P2P Search", __func__);
                    scanRequest.p2pSearch = 1;

                    /*                                            
                                                                          */
                    if( request->n_channels == WLAN_HDD_P2P_SOCIAL_CHANNELS )
                    {
                         pScanInfo->flushP2pScanResults = 1;
                         sme_ScanFlushP2PResult( WLAN_HDD_GET_HAL_CTX(pAdapter),
                                          sessionId );
                         /*                                  */
                         scanRequest.requestType = eCSR_SCAN_P2P_DISCOVERY;
                    }

                    /*
                                                             
                                               
                    */
                    if(cfg_param->skipDfsChnlInP2pSearch)
                    {
                       scanRequest.skipDfsChnlInP2pSearch = 1;
                    }
                    else
                    {
                       scanRequest.skipDfsChnlInP2pSearch = 0;
                    }

                }
            }
#endif
        }
    }

    INIT_COMPLETION(pScanInfo->scan_req_completion_event);

    /*                                                                    
                                    
                                                                                   
                                                                                       
                                                                              
                                                                                 
                                                    
     */
    hdd_prevent_suspend();

    status = sme_ScanRequest( WLAN_HDD_GET_HAL_CTX(pAdapter),
                              pAdapter->sessionId, &scanRequest, &scanId,
                              &hdd_cfg80211_scan_done_callback, dev );

    if (eHAL_STATUS_SUCCESS != status)
    {
        hddLog(VOS_TRACE_LEVEL_ERROR,
                "%s: sme_ScanRequest returned error %d", __func__, status);
        complete(&pScanInfo->scan_req_completion_event);
        if(eHAL_STATUS_RESOURCES == status)
        {
                hddLog(VOS_TRACE_LEVEL_INFO, "%s: HO is in progress.So defer the scan by informing busy",__func__);
                status = -EBUSY;
        } else {
                status = -EIO;
        }
        hdd_allow_suspend();
        goto free_mem;
    }

    pScanInfo->mScanPending = TRUE;
    pAdapter->request = request;
    pScanInfo->scanId = scanId;

    complete(&pScanInfo->scan_req_completion_event);

free_mem:
    if( scanRequest.SSIDs.SSIDList )
    {
        vos_mem_free(scanRequest.SSIDs.SSIDList);
    }

    if( channelList )
      vos_mem_free( channelList );

    EXIT();

    return status;
}

/*
                                            
                                                          
 */
int wlan_hdd_cfg80211_connect_start( hdd_adapter_t  *pAdapter, 
        const u8 *ssid, size_t ssid_len, const u8 *bssid, u8 operatingChannel)
{
    int status = 0;
    hdd_wext_state_t *pWextState;
    v_U32_t roamId;
    tCsrRoamProfile *pRoamProfile;
    eMib_dot11DesiredBssType connectedBssType;
    eCsrAuthType RSNAuthType;

    ENTER();

    pWextState = WLAN_HDD_GET_WEXT_STATE_PTR(pAdapter);
    
    if (SIR_MAC_MAX_SSID_LENGTH < ssid_len)
    {
        hddLog(VOS_TRACE_LEVEL_ERROR, "%s: wrong SSID len", __func__);
        return -EINVAL;
    }

    pRoamProfile = &pWextState->roamProfile;

    if (pRoamProfile) 
    {
        int ret = 0;
        hdd_station_ctx_t *pHddStaCtx;
        pHddStaCtx = WLAN_HDD_GET_STATION_CTX_PTR(pAdapter);
        hdd_connGetConnectedBssType(pHddStaCtx,&connectedBssType );

        if((eMib_dot11DesiredBssType_independent == connectedBssType) ||
           (eConnectionState_Associated == pHddStaCtx->conn_info.connState) ||
           (eConnectionState_IbssConnected == pHddStaCtx->conn_info.connState))
        {
            /*                         */
            INIT_COMPLETION(pAdapter->disconnect_comp_var);
            if( eHAL_STATUS_SUCCESS == 
                  sme_RoamDisconnect( WLAN_HDD_GET_HAL_CTX(pAdapter),
                            pAdapter->sessionId,
                            eCSR_DISCONNECT_REASON_UNSPECIFIED ) )
            {
                ret = wait_for_completion_interruptible_timeout(
                             &pAdapter->disconnect_comp_var,
                             msecs_to_jiffies(WLAN_WAIT_TIME_DISCONNECT));
                if (0 == ret)
                {
                    VOS_ASSERT(0);
                }
            }
        }
        else if(eConnectionState_Disconnecting == pHddStaCtx->conn_info.connState)
        {
            ret = wait_for_completion_interruptible_timeout(
                         &pAdapter->disconnect_comp_var,
                         msecs_to_jiffies(WLAN_WAIT_TIME_DISCONNECT));
            if (0 == ret)
            {
                VOS_ASSERT(0);
            }
        }

        if (HDD_WMM_USER_MODE_NO_QOS == 
                        (WLAN_HDD_GET_CTX(pAdapter))->cfg_ini->WmmMode)
        {
            /*                           */
            pRoamProfile->uapsd_mask = 0;
        }
        else
        {
            /*                                            */
            pRoamProfile->uapsd_mask = 
                     (WLAN_HDD_GET_CTX(pAdapter))->cfg_ini->UapsdMask;
        }

        pRoamProfile->SSIDs.numOfSSIDs = 1;
        pRoamProfile->SSIDs.SSIDList->SSID.length = ssid_len;
        vos_mem_zero(pRoamProfile->SSIDs.SSIDList->SSID.ssId,
                sizeof(pRoamProfile->SSIDs.SSIDList->SSID.ssId)); 
        vos_mem_copy((void *)(pRoamProfile->SSIDs.SSIDList->SSID.ssId),
                ssid, ssid_len);

        if (bssid)
        {
            pRoamProfile->BSSIDs.numOfBSSIDs = 1;
            vos_mem_copy((void *)(pRoamProfile->BSSIDs.bssid), bssid,
                    WNI_CFG_BSSID_LEN);
            /*                                                         
                                                                              
                                                                            
             */
            vos_mem_copy((void *)(pWextState->req_bssId), bssid,
                    WNI_CFG_BSSID_LEN);
        }

        if ((IW_AUTH_WPA_VERSION_WPA == pWextState->wpaVersion) ||
                (IW_AUTH_WPA_VERSION_WPA2 == pWextState->wpaVersion))
        { 
            /*          */
            hdd_SetGENIEToCsr(pAdapter, &RSNAuthType);
            /*        */
            hdd_set_csr_auth_type(pAdapter, RSNAuthType);
        }
        else if ( (pWextState->roamProfile.AuthType.authType[0] == 
                    eCSR_AUTH_TYPE_OPEN_SYSTEM)
                && ((pWextState->roamProfile.EncryptionType.encryptionType[0] == 
                        eCSR_ENCRYPT_TYPE_WEP40_STATICKEY) 
                    || (pWextState->roamProfile.EncryptionType.encryptionType[0] == 
                        eCSR_ENCRYPT_TYPE_WEP104_STATICKEY))
                )
        {
            /*                                                                                     
                                                                                                 
                                                                     */
            (WLAN_HDD_GET_STATION_CTX_PTR(pAdapter))->conn_info.authType = 
                                                     eCSR_AUTH_TYPE_AUTOSWITCH;
            pWextState->roamProfile.AuthType.authType[0] = 
                  (WLAN_HDD_GET_STATION_CTX_PTR(pAdapter))->conn_info.authType;
        }
#ifdef FEATURE_WLAN_WAPI
        if (pAdapter->wapi_info.nWapiMode)
        {
            hddLog(LOG1, "%s: Setting WAPI AUTH Type and Encryption Mode values", __FUNCTION__);
            switch (pAdapter->wapi_info.wapiAuthMode)
            {
                case WAPI_AUTH_MODE_PSK:
                {
                    hddLog(LOG1, "%s: WAPI AUTH TYPE: PSK: %d", __FUNCTION__,
                                                   pAdapter->wapi_info.wapiAuthMode);
                    pRoamProfile->AuthType.authType[0] = eCSR_AUTH_TYPE_WAPI_WAI_PSK;
                    break;
                }
                case WAPI_AUTH_MODE_CERT:
                {
                    hddLog(LOG1, "%s: WAPI AUTH TYPE: CERT: %d", __FUNCTION__,
                                                    pAdapter->wapi_info.wapiAuthMode);
                    pRoamProfile->AuthType.authType[0] = eCSR_AUTH_TYPE_WAPI_WAI_CERTIFICATE;
                    break;
                }
            } //              
            if ( pAdapter->wapi_info.wapiAuthMode == WAPI_AUTH_MODE_PSK ||
                pAdapter->wapi_info.wapiAuthMode == WAPI_AUTH_MODE_CERT)
            {
                hddLog(LOG1, "%s: WAPI PAIRWISE/GROUP ENCRYPTION: WPI", __FUNCTION__);
                pRoamProfile->AuthType.numEntries = 1;
                pRoamProfile->EncryptionType.numEntries = 1;
                pRoamProfile->EncryptionType.encryptionType[0] = eCSR_ENCRYPT_TYPE_WPI;
                pRoamProfile->mcEncryptionType.numEntries = 1;
                pRoamProfile->mcEncryptionType.encryptionType[0] = eCSR_ENCRYPT_TYPE_WPI;
            }
        }
#endif /*                   */
        pRoamProfile->csrPersona = pAdapter->device_mode;

        if( operatingChannel )
        {
           pRoamProfile->ChannelInfo.ChannelList = &operatingChannel;
           pRoamProfile->ChannelInfo.numOfChannels = 1;
        }

        status = sme_RoamConnect( WLAN_HDD_GET_HAL_CTX(pAdapter), 
                            pAdapter->sessionId, pRoamProfile, &roamId);

        pRoamProfile->ChannelInfo.ChannelList = NULL;
        pRoamProfile->ChannelInfo.numOfChannels = 0;
    }
    else
    {
        hddLog(VOS_TRACE_LEVEL_ERROR, "%s: No valid Roam profile", __func__);
        return -EINVAL;
    }
    EXIT(); 
    return status;
}

/*
                                            
                                                                      
  
 */
static int wlan_hdd_cfg80211_set_auth_type(hdd_adapter_t *pAdapter,
        enum nl80211_auth_type auth_type)
{
    hdd_wext_state_t *pWextState = WLAN_HDD_GET_WEXT_STATE_PTR(pAdapter);   
    hdd_station_ctx_t *pHddStaCtx = WLAN_HDD_GET_STATION_CTX_PTR(pAdapter);

    ENTER();

    /*                       */
    switch (auth_type) 
    {
        case NL80211_AUTHTYPE_OPEN_SYSTEM:
        case NL80211_AUTHTYPE_AUTOMATIC:
            hddLog(VOS_TRACE_LEVEL_INFO, 
                    "%s: set authentication type to OPEN", __func__);
            pHddStaCtx->conn_info.authType = eCSR_AUTH_TYPE_OPEN_SYSTEM;
            break;

        case NL80211_AUTHTYPE_SHARED_KEY:
            hddLog(VOS_TRACE_LEVEL_INFO, 
                    "%s: set authentication type to SHARED", __func__);
            pHddStaCtx->conn_info.authType = eCSR_AUTH_TYPE_SHARED_KEY;
            break;
#ifdef FEATURE_WLAN_CCX
        case NL80211_AUTHTYPE_NETWORK_EAP:
            hddLog(VOS_TRACE_LEVEL_INFO, 
                            "%s: set authentication type to CCKM WPA", __func__);
            pHddStaCtx->conn_info.authType = eCSR_AUTH_TYPE_CCKM_WPA;//                                                                
            break;
#endif


        default:
            hddLog(VOS_TRACE_LEVEL_ERROR, 
                    "%s: Unsupported authentication type %d", __func__, 
                    auth_type);
            pHddStaCtx->conn_info.authType = eCSR_AUTH_TYPE_UNKNOWN;
            return -EINVAL;
    }

    pWextState->roamProfile.AuthType.authType[0] = 
                                        pHddStaCtx->conn_info.authType;
    return 0;
}

/*
                                   
                                                             
  
 */
static int wlan_hdd_set_akm_suite( hdd_adapter_t *pAdapter, 
                                   u32 key_mgmt
                                   )
{
    hdd_wext_state_t *pWextState = WLAN_HDD_GET_WEXT_STATE_PTR(pAdapter);
    ENTER();
    
    /*                 */
    switch(key_mgmt)
    {
        case WLAN_AKM_SUITE_PSK:
            hddLog(VOS_TRACE_LEVEL_INFO, "%s: setting key mgmt type to PSK", 
                    __func__);
            pWextState->authKeyMgmt |= IW_AUTH_KEY_MGMT_PSK;
            break;

        case WLAN_AKM_SUITE_8021X:
            hddLog(VOS_TRACE_LEVEL_INFO, "%s: setting key mgmt type to 8021x", 
                    __func__);
            pWextState->authKeyMgmt |= IW_AUTH_KEY_MGMT_802_1X;
            break;
#ifdef FEATURE_WLAN_CCX
#define WLAN_AKM_SUITE_CCKM         0x00409600 /*                                */
#define IW_AUTH_KEY_MGMT_CCKM       8  /*                               */
        case WLAN_AKM_SUITE_CCKM:
            hddLog(VOS_TRACE_LEVEL_INFO, "%s: setting key mgmt type to CCKM",
                            __func__);
            pWextState->authKeyMgmt |= IW_AUTH_KEY_MGMT_CCKM;
            break;
#endif

        default:
            hddLog(VOS_TRACE_LEVEL_ERROR, "%s: Unsupported key mgmt type %d", 
                    __func__, key_mgmt);
            return -EINVAL;

    }
    return 0;
}

/*
                                         
                                                    
                                 
 */
static int wlan_hdd_cfg80211_set_cipher( hdd_adapter_t *pAdapter, 
                                u32 cipher, 
                                bool ucast
                                )
{
    eCsrEncryptionType encryptionType = eCSR_ENCRYPT_TYPE_NONE;
    hdd_wext_state_t *pWextState = WLAN_HDD_GET_WEXT_STATE_PTR(pAdapter);   
    hdd_station_ctx_t *pHddStaCtx = WLAN_HDD_GET_STATION_CTX_PTR(pAdapter);

    ENTER();

    if (!cipher) 
    {
        hddLog(VOS_TRACE_LEVEL_ERROR, "%s: received cipher %d - considering none", 
                __func__, cipher);
        encryptionType = eCSR_ENCRYPT_TYPE_NONE;
    }
    else
    {
    
        /*                     */
        switch (cipher) 
        {
            case IW_AUTH_CIPHER_NONE:
                encryptionType = eCSR_ENCRYPT_TYPE_NONE;
                break;
    
            case WLAN_CIPHER_SUITE_WEP40:
                if ((IW_AUTH_KEY_MGMT_802_1X == pWextState->authKeyMgmt) && 
                    (eCSR_AUTH_TYPE_OPEN_SYSTEM == pHddStaCtx->conn_info.authType))
                    encryptionType = eCSR_ENCRYPT_TYPE_WEP40;
                else
                    encryptionType = eCSR_ENCRYPT_TYPE_WEP40_STATICKEY;
                break;
    
            case WLAN_CIPHER_SUITE_WEP104:
                if ((IW_AUTH_KEY_MGMT_802_1X == pWextState->authKeyMgmt) && 
                    (eCSR_AUTH_TYPE_OPEN_SYSTEM == pHddStaCtx->conn_info.authType))
                    encryptionType = eCSR_ENCRYPT_TYPE_WEP104;
                else
                    encryptionType = eCSR_ENCRYPT_TYPE_WEP104_STATICKEY;
                break;
    
            case WLAN_CIPHER_SUITE_TKIP:
                encryptionType = eCSR_ENCRYPT_TYPE_TKIP;
                break;
    
            case WLAN_CIPHER_SUITE_CCMP:
                encryptionType = eCSR_ENCRYPT_TYPE_AES;
                break;
#ifdef FEATURE_WLAN_WAPI
        case WLAN_CIPHER_SUITE_SMS4:
            encryptionType = eCSR_ENCRYPT_TYPE_WPI;
            break;
#endif

#ifdef FEATURE_WLAN_CCX
        case WLAN_CIPHER_SUITE_KRK:
            encryptionType = eCSR_ENCRYPT_TYPE_KRK;
            break;
#endif
            default:
                hddLog(VOS_TRACE_LEVEL_ERROR, "%s: Unsupported cipher type %d", 
                        __func__, cipher);
                return -EOPNOTSUPP;
        }
    }

    if (ucast)
    {
        hddLog(VOS_TRACE_LEVEL_INFO, "%s: setting unicast cipher type to %d", 
                __func__, encryptionType);
        pHddStaCtx->conn_info.ucEncryptionType            = encryptionType;
        pWextState->roamProfile.EncryptionType.numEntries = 1;
        pWextState->roamProfile.EncryptionType.encryptionType[0] = 
                                          encryptionType;
    }
    else
    {
        hddLog(VOS_TRACE_LEVEL_INFO, "%s: setting mcast cipher type to %d", 
                __func__, encryptionType);
        pHddStaCtx->conn_info.mcEncryptionType                       = encryptionType;
        pWextState->roamProfile.mcEncryptionType.numEntries        = 1;
        pWextState->roamProfile.mcEncryptionType.encryptionType[0] = encryptionType;
    }

    return 0;
}


/*
                                     
                                               
 */
int wlan_hdd_cfg80211_set_ie( hdd_adapter_t *pAdapter, 
                              u8 *ie, 
                              size_t ie_len
                              )
{
    hdd_wext_state_t *pWextState = WLAN_HDD_GET_WEXT_STATE_PTR(pAdapter);
    u8 *genie = ie;
    v_U16_t remLen = ie_len;
#ifdef FEATURE_WLAN_WAPI
    v_U32_t akmsuite[MAX_NUM_AKM_SUITES];
    u16 *tmp;
    v_U16_t akmsuiteCount;
    int *akmlist;
#endif
    ENTER();

    /*                           */
    pWextState->assocAddIE.length = 0;
    pWextState->roamProfile.bWPSAssociation = VOS_FALSE;

    while (remLen >= 2)
    {
        v_U16_t eLen = 0;
        v_U8_t elementId;
        elementId = *genie++;
        eLen  = *genie++;
        remLen -= 2;
    
        hddLog(VOS_TRACE_LEVEL_INFO, "%s: IE[0x%X], LEN[%d]\n", 
            __func__, elementId, eLen);
         
        switch ( elementId ) 
        {
            case DOT11F_EID_WPA: 
                if ((2+4) > eLen) /*                          */
                {
                    hddLog(VOS_TRACE_LEVEL_ERROR, 
                              "%s: Invalid WPA IE", __func__);
                    return -EINVAL;
                }
                else if (0 == memcmp(&genie[0], "\x00\x50\xf2\x04", 4)) 
                {
                    v_U16_t curAddIELen = pWextState->assocAddIE.length;
                    hddLog (VOS_TRACE_LEVEL_INFO, "%s Set WPS IE(len %d)", 
                            __func__, eLen + 2);
                    
                    if( SIR_MAC_MAX_IE_LENGTH < (pWextState->assocAddIE.length + eLen) )
                    {
                       hddLog(VOS_TRACE_LEVEL_FATAL, "Cannot accomadate assocAddIE. "
                                                      "Need bigger buffer space\n");
                       VOS_ASSERT(0);
                       return -ENOMEM;
                    }
                    //                                                                                      
                    memcpy( pWextState->assocAddIE.addIEdata + curAddIELen, genie - 2, eLen + 2);
                    pWextState->assocAddIE.length += eLen + 2;
                    
                    pWextState->roamProfile.bWPSAssociation = VOS_TRUE;
                    pWextState->roamProfile.pAddIEAssoc = pWextState->assocAddIE.addIEdata;
                    pWextState->roamProfile.nAddIEAssocLength = pWextState->assocAddIE.length;
                }
                else if (0 == memcmp(&genie[0], "\x00\x50\xf2", 3)) 
                {  
                    hddLog (VOS_TRACE_LEVEL_INFO, "%s Set WPA IE (len %d)",__func__, eLen + 2);
                    memset( pWextState->WPARSNIE, 0, MAX_WPA_RSN_IE_LEN );
                    memcpy( pWextState->WPARSNIE, genie - 2, (eLen + 2) /*      */);
                    pWextState->roamProfile.pWPAReqIE = pWextState->WPARSNIE;
                    pWextState->roamProfile.nWPAReqIELength = eLen + 2;//       
                }
#ifdef WLAN_FEATURE_P2P
                else if ( (0 == memcmp(&genie[0], P2P_OUI_TYPE, 
                                                         P2P_OUI_TYPE_SIZE)) 
                        /*                                     */
                         && (WLAN_HDD_P2P_CLIENT == pAdapter->device_mode) )
                {
                    v_U16_t curAddIELen = pWextState->assocAddIE.length;
                    hddLog (VOS_TRACE_LEVEL_INFO, "%s Set P2P IE(len %d)", 
                            __func__, eLen + 2);
                    
                    if( SIR_MAC_MAX_IE_LENGTH < (pWextState->assocAddIE.length + eLen) )
                    {
                       hddLog(VOS_TRACE_LEVEL_FATAL, "Cannot accomadate assocAddIE "
                                                      "Need bigger buffer space\n");
                       VOS_ASSERT(0);
                       return -ENOMEM;
                    }
                    //                                                                                      
                    memcpy( pWextState->assocAddIE.addIEdata + curAddIELen, genie - 2, eLen + 2);
                    pWextState->assocAddIE.length += eLen + 2;
                    
                    pWextState->roamProfile.pAddIEAssoc = pWextState->assocAddIE.addIEdata;
                    pWextState->roamProfile.nAddIEAssocLength = pWextState->assocAddIE.length;
                }
#endif
#ifdef WLAN_FEATURE_WFD
                else if ( (0 == memcmp(&genie[0], WFD_OUI_TYPE, 
                                                         WFD_OUI_TYPE_SIZE)) 
                        /*                                     */
                         && (WLAN_HDD_P2P_CLIENT == pAdapter->device_mode) )
                {
                    v_U16_t curAddIELen = pWextState->assocAddIE.length;
                    hddLog (VOS_TRACE_LEVEL_INFO, "%s Set WFD IE(len %d)", 
                            __func__, eLen + 2);
                    
                    if( SIR_MAC_MAX_IE_LENGTH < (pWextState->assocAddIE.length + eLen) )
                    {
                       hddLog(VOS_TRACE_LEVEL_FATAL, "Cannot accomadate assocAddIE "
                                                      "Need bigger buffer space\n");
                       VOS_ASSERT(0);
                       return -ENOMEM;
                    }
                    //                                                                      
                    //                         
                    memcpy( pWextState->assocAddIE.addIEdata + curAddIELen, genie - 2, eLen + 2);
                    pWextState->assocAddIE.length += eLen + 2;
                    
                    pWextState->roamProfile.pAddIEAssoc = pWextState->assocAddIE.addIEdata;
                    pWextState->roamProfile.nAddIEAssocLength = pWextState->assocAddIE.length;
                }
#endif
                break;
            case DOT11F_EID_RSN:
                hddLog (VOS_TRACE_LEVEL_INFO, "%s Set RSN IE(len %d)",__func__, eLen + 2);
                memset( pWextState->WPARSNIE, 0, MAX_WPA_RSN_IE_LEN );
                memcpy( pWextState->WPARSNIE, genie - 2, (eLen + 2)/*      */);
                pWextState->roamProfile.pRSNReqIE = pWextState->WPARSNIE;
                pWextState->roamProfile.nRSNReqIELength = eLen + 2; //       
                break;
#ifdef FEATURE_WLAN_WAPI
            case WLAN_EID_WAPI:
                pAdapter->wapi_info.nWapiMode = 1;   //                         
                hddLog(VOS_TRACE_LEVEL_INFO,"WAPI MODE IS  %lu \n",
                                          pAdapter->wapi_info.nWapiMode);
                tmp = (u16 *)ie;
                tmp = tmp + 2; //                                         
                akmsuiteCount = WPA_GET_LE16(tmp);
                tmp = tmp + 1;
                akmlist = (int *)(tmp);
                if(akmsuiteCount <= MAX_NUM_AKM_SUITES)
                {
                    memcpy(akmsuite, akmlist, (4*akmsuiteCount));
                }
                else
                {
                    hddLog(VOS_TRACE_LEVEL_FATAL, "Invalid akmSuite count\n");
                    VOS_ASSERT(0);
                    return -EINVAL;
                }

                if (WAPI_PSK_AKM_SUITE == akmsuite[0])
                {
                    hddLog(VOS_TRACE_LEVEL_INFO, "%s: WAPI AUTH MODE SET TO PSK",
                                                            __FUNCTION__);
                    pAdapter->wapi_info.wapiAuthMode = WAPI_AUTH_MODE_PSK;
                }    
                if (WAPI_CERT_AKM_SUITE == akmsuite[0])
                {     
                    hddLog(VOS_TRACE_LEVEL_INFO, "%s: WAPI AUTH MODE SET TO CERTIFICATE",
                                                             __FUNCTION__);
                    pAdapter->wapi_info.wapiAuthMode = WAPI_AUTH_MODE_CERT;
                }
                break;
#endif
            default:
                hddLog (VOS_TRACE_LEVEL_ERROR, 
                        "%s Set UNKNOWN IE %X", __func__, elementId);
                return 0;
        }
        genie += eLen;
        remLen -= eLen;
    }
    EXIT();
    return 0;
}

/*
                                          
                                                    
                                       
 */
int wlan_hdd_cfg80211_set_privacy( hdd_adapter_t *pAdapter, 
                                   struct cfg80211_connect_params *req
                                   )
{
    int status = 0;
    hdd_wext_state_t *pWextState = WLAN_HDD_GET_WEXT_STATE_PTR(pAdapter);   
    ENTER();

    /*               */
    pWextState->wpaVersion = IW_AUTH_WPA_VERSION_DISABLED;

    if (req->crypto.wpa_versions) 
    {
        if ( (NL80211_WPA_VERSION_1 == req->crypto.wpa_versions)
            && ( (req->ie_len) 
           && (0 == memcmp( &req->ie[2], "\x00\x50\xf2",3) ) ) ) 
           //                                         
           /*                                                      
                                                      
              */
        {
            pWextState->wpaVersion = IW_AUTH_WPA_VERSION_WPA;
        }
        else if (NL80211_WPA_VERSION_2 == req->crypto.wpa_versions)
        {
            pWextState->wpaVersion = IW_AUTH_WPA_VERSION_WPA2;
        }
    }
    
    hddLog(VOS_TRACE_LEVEL_INFO, "%s: set wpa version to %d", __func__, 
            pWextState->wpaVersion);

    /*                       */
    status = wlan_hdd_cfg80211_set_auth_type(pAdapter, req->auth_type);

    if (0 > status)
    {
        hddLog(VOS_TRACE_LEVEL_ERROR, 
                "%s: failed to set authentication type ", __func__);
        return status;
    }

    /*                 */
    if (req->crypto.n_akm_suites)
    {
        status = wlan_hdd_set_akm_suite(pAdapter, req->crypto.akm_suites[0]);
        if (0 > status)
        {
            hddLog(VOS_TRACE_LEVEL_ERROR, "%s: failed to set akm suite", 
                    __func__);
            return status;
        }
    }

    /*                        */
    if (req->crypto.n_ciphers_pairwise)
    {
        status = wlan_hdd_cfg80211_set_cipher(pAdapter,
                                      req->crypto.ciphers_pairwise[0], true);
        if (0 > status)
        {
            hddLog(VOS_TRACE_LEVEL_ERROR, 
                    "%s: failed to set unicast cipher type", __func__);
            return status;
        }
    }
    else
    {
        /*                                   */
        status = wlan_hdd_cfg80211_set_cipher(pAdapter, 0, true);
        if (0 > status)
        {
            hddLog(VOS_TRACE_LEVEL_ERROR, 
                    "%s: failed to set unicast cipher type", __func__);
            return status;
        }
    }

    /*                     */
    status = wlan_hdd_cfg80211_set_cipher(pAdapter, req->crypto.cipher_group,
                                                                       false);

    if (0 > status)
    {
        hddLog(VOS_TRACE_LEVEL_ERROR, "%s: failed to set mcast cipher type", 
                __func__);
        return status;
    }

    /*                                                                */
    if (req->ie_len)
    {
        status = wlan_hdd_cfg80211_set_ie(pAdapter, req->ie, req->ie_len);
        if ( 0 > status)
        {
            hddLog(VOS_TRACE_LEVEL_ERROR, "%s: failed to parse the WPA/RSN IE", 
                    __func__);
            return status;
        }
    }

    /*                                         */
    if (req->key && req->key_len) 
    {
        if ( (WLAN_CIPHER_SUITE_WEP40 == req->crypto.ciphers_pairwise[0])
                || (WLAN_CIPHER_SUITE_WEP104 == req->crypto.ciphers_pairwise[0])
          )
        {
            if ( IW_AUTH_KEY_MGMT_802_1X 
                    == (pWextState->authKeyMgmt & IW_AUTH_KEY_MGMT_802_1X  ))
            {
                hddLog(VOS_TRACE_LEVEL_ERROR, "%s: Dynamic WEP not supported", 
                        __func__);
                return -EOPNOTSUPP;
            }
            else
            {
                u8 key_len = req->key_len;
                u8 key_idx = req->key_idx;

                if ((eCSR_SECURITY_WEP_KEYSIZE_MAX_BYTES >= key_len) 
                        && (CSR_MAX_NUM_KEY > key_idx)
                  )
                {
                    hddLog(VOS_TRACE_LEVEL_INFO, 
                     "%s: setting default wep key, key_idx = %hu key_len %hu", 
                            __func__, key_idx, key_len);
                    vos_mem_copy(
                       &pWextState->roamProfile.Keys.KeyMaterial[key_idx][0], 
                                  req->key, key_len);
                    pWextState->roamProfile.Keys.KeyLength[key_idx] = 
                                                               (u8)key_len;
                    pWextState->roamProfile.Keys.defaultIndex = (u8)key_idx;
                }
            }
        }
    }

    return status;
}

/*
                                          
                                                    
                                       
 */
static int wlan_hdd_cfg80211_connect( struct wiphy *wiphy, 
                                      struct net_device *ndev,
                                      struct cfg80211_connect_params *req
                                      )
{
    int status = 0;
    hdd_adapter_t *pAdapter = WLAN_HDD_GET_PRIV_PTR( ndev ); 
    VOS_STATUS exitbmpsStatus = VOS_STATUS_E_INVAL;
    hdd_context_t *pHddCtx = NULL;

    ENTER();

    hddLog(VOS_TRACE_LEVEL_INFO, 
             "%s: device_mode = %d\n",__func__,pAdapter->device_mode);

    if ( (WLAN_HDD_GET_CTX(pAdapter))->isLogpInProgress )
    {
        VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR,
                "%s:LOGP in Progress. Ignore!!!", __func__);
        return -EAGAIN;
    }

#ifdef WLAN_BTAMP_FEATURE
    //                                                   
    if( VOS_TRUE == WLANBAP_AmpSessionOn() ) 
    {
        hddLog(VOS_TRACE_LEVEL_ERROR, 
                "%s: No connection when AMP is on", __func__);
        return -1;
    }
#endif
    /*                              */
    status = wlan_hdd_cfg80211_set_privacy(pAdapter, req); 

    if ( 0 > status)
    {
        hddLog(VOS_TRACE_LEVEL_ERROR, "%s: failed to set security params", 
                __func__);
        return status;
    }

    //                                                       
    //                                                 
    if((WLAN_HDD_INFRA_STATION == pAdapter->device_mode) &&
        (vos_concurrent_sessions_running()))
    {
        v_CONTEXT_t pVosContext = vos_get_global_context( VOS_MODULE_ID_HDD, NULL );

        if (NULL != pVosContext)
        {
            pHddCtx = vos_get_context( VOS_MODULE_ID_HDD, pVosContext);
            if(NULL != pHddCtx)
            {
               exitbmpsStatus = hdd_disable_bmps_imps(pHddCtx, WLAN_HDD_INFRA_STATION);
            }
        }
    }

    if ( req->channel )
    {
        status = wlan_hdd_cfg80211_connect_start(pAdapter, req->ssid,
                                                  req->ssid_len, req->bssid,
                                                  req->channel->hw_value);
    }
    else
    {
        status = wlan_hdd_cfg80211_connect_start(pAdapter, req->ssid,
                                                  req->ssid_len, req->bssid,
                                                  0);
    }

    if (0 > status)
    {
        //                         
        if((VOS_STATUS_SUCCESS == exitbmpsStatus) &&
            (NULL != pHddCtx))
        {
           //                           
           hdd_enable_bmps_imps(pHddCtx);
        }

        hddLog(VOS_TRACE_LEVEL_ERROR, "%s: connect failed", __func__);
        return status;
    }
    (WLAN_HDD_GET_CTX(pAdapter))->isAmpAllowed = VOS_FALSE;
    EXIT();
    return status;
}


/*
                                         
                                                             
 */
static int wlan_hdd_cfg80211_disconnect( struct wiphy *wiphy, 
                                         struct net_device *dev,
                                         u16 reason
                                         )
{
    hdd_adapter_t *pAdapter = WLAN_HDD_GET_PRIV_PTR( dev ); 
    tCsrRoamProfile  *pRoamProfile = 
                    &(WLAN_HDD_GET_WEXT_STATE_PTR(pAdapter))->roamProfile;
    int status = 0;
    hdd_station_ctx_t *pHddStaCtx = WLAN_HDD_GET_STATION_CTX_PTR(pAdapter);
    
    ENTER();
    
    hddLog(VOS_TRACE_LEVEL_INFO, "%s: device_mode = %d\n", 
                                    __func__,pAdapter->device_mode);

    hddLog(VOS_TRACE_LEVEL_INFO, "%s: Disconnect called with reason code %d", 
            __func__, reason);    

    if ((WLAN_HDD_GET_CTX(pAdapter))->isLogpInProgress)
    {
        VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR,
                "%s:LOGP in Progress. Ignore!!!",__func__);
        return -EAGAIN;
    }
    if (NULL != pRoamProfile)
    {
        /*                                                                 */
        if (pHddStaCtx->conn_info.connState == eConnectionState_Associated)
        {
            eCsrRoamDisconnectReason reasonCode = 
                                       eCSR_DISCONNECT_REASON_UNSPECIFIED;
            switch(reason)
            {
                case WLAN_REASON_MIC_FAILURE:
                    reasonCode = eCSR_DISCONNECT_REASON_MIC_ERROR;
                    break;

                case WLAN_REASON_DISASSOC_DUE_TO_INACTIVITY:
                case WLAN_REASON_DISASSOC_AP_BUSY:
                case WLAN_REASON_CLASS3_FRAME_FROM_NONASSOC_STA:
                    reasonCode = eCSR_DISCONNECT_REASON_DISASSOC;
                    break;

                case WLAN_REASON_PREV_AUTH_NOT_VALID:
                case WLAN_REASON_CLASS2_FRAME_FROM_NONAUTH_STA:
                    reasonCode = eCSR_DISCONNECT_REASON_DEAUTH;
                    break;

                case WLAN_REASON_DEAUTH_LEAVING:
                default:
                    reasonCode = eCSR_DISCONNECT_REASON_UNSPECIFIED;
                    break;
            }
            pHddStaCtx->conn_info.connState = eConnectionState_NotConnected;
            (WLAN_HDD_GET_CTX(pAdapter))->isAmpAllowed = VOS_TRUE;
            INIT_COMPLETION(pAdapter->disconnect_comp_var);

            /*                */
            status = sme_RoamDisconnect( WLAN_HDD_GET_HAL_CTX(pAdapter), 
                                         pAdapter->sessionId, reasonCode);

            if ( 0 != status)
            {
                hddLog(VOS_TRACE_LEVEL_ERROR,
                        "%s csrRoamDisconnect failure, returned %d \n", 
                        __func__, (int)status );
                return -EINVAL;
            }

            wait_for_completion_interruptible_timeout(
                   &pAdapter->disconnect_comp_var,
                   msecs_to_jiffies(WLAN_WAIT_TIME_DISCONNECT));


            /*              */
            netif_tx_disable(dev);
            netif_carrier_off(dev);
        }
    }
    else
    {
        hddLog(VOS_TRACE_LEVEL_ERROR, "%s: No valid roam profile", __func__);
    }

    return status;
}

/*
                                               
                                                    
                         
 */
static int wlan_hdd_cfg80211_set_privacy_ibss(
                                         hdd_adapter_t *pAdapter, 
                                         struct cfg80211_ibss_params *params
                                         )
{
    int status = 0;
    hdd_wext_state_t *pWextState = WLAN_HDD_GET_WEXT_STATE_PTR(pAdapter);   
    eCsrEncryptionType encryptionType = eCSR_ENCRYPT_TYPE_NONE;
    hdd_station_ctx_t *pHddStaCtx = WLAN_HDD_GET_STATION_CTX_PTR(pAdapter);
    
    ENTER();

    pWextState->wpaVersion = IW_AUTH_WPA_VERSION_DISABLED;

    if (params->ie_len && ( NULL != params->ie) )
    {
        if (WLAN_EID_RSN == params->ie[0]) 
        {
            pWextState->wpaVersion = IW_AUTH_WPA_VERSION_WPA2;
            encryptionType = eCSR_ENCRYPT_TYPE_AES;
        }
        else
        {
            pWextState->wpaVersion = IW_AUTH_WPA_VERSION_WPA;
            encryptionType = eCSR_ENCRYPT_TYPE_TKIP;
        }
        status = wlan_hdd_cfg80211_set_ie(pAdapter, params->ie, params->ie_len);

        if (0 > status)
        {
            hddLog(VOS_TRACE_LEVEL_ERROR, "%s: failed to parse WPA/RSN IE", 
                    __func__);
            return status;
        }
    }

    pWextState->roamProfile.AuthType.authType[0] = 
                                pHddStaCtx->conn_info.authType = 
                                eCSR_AUTH_TYPE_OPEN_SYSTEM;

    if (params->privacy)
    {
        /*                                                                       
                                                                              
                                             
                                                                    
                                     */
        /*                                               
                                        */

        encryptionType = eCSR_ENCRYPT_TYPE_WEP40_STATICKEY;
    }

    pHddStaCtx->conn_info.ucEncryptionType                   = encryptionType;
    pWextState->roamProfile.EncryptionType.numEntries        = 1;
    pWextState->roamProfile.EncryptionType.encryptionType[0] = encryptionType;

    return status;
}

/*
                                        
                                                
 */
static int wlan_hdd_cfg80211_join_ibss( struct wiphy *wiphy, 
                                        struct net_device *dev,
                                        struct cfg80211_ibss_params *params
                                       )
{
    hdd_adapter_t *pAdapter = WLAN_HDD_GET_PRIV_PTR( dev ); 
    hdd_wext_state_t *pWextState = WLAN_HDD_GET_WEXT_STATE_PTR(pAdapter);
    tCsrRoamProfile          *pRoamProfile;
    int status;
    hdd_station_ctx_t *pHddStaCtx = WLAN_HDD_GET_STATION_CTX_PTR(pAdapter);

    ENTER();
    
    hddLog(VOS_TRACE_LEVEL_INFO, 
                  "%s: device_mode = %d\n",__func__,pAdapter->device_mode);

    if ( (WLAN_HDD_GET_CTX(pAdapter))->isLogpInProgress )
    {
        VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR,
                "%s:LOGP in Progress. Ignore!!!", __func__);
        return -EAGAIN;
    }

    if (NULL == pWextState)
    {
        hddLog (VOS_TRACE_LEVEL_ERROR, "%s ERROR: Data Storage Corruption\n", 
                __func__);
        return -EIO;
    }

    pRoamProfile = &pWextState->roamProfile;

    if ( eCSR_BSS_TYPE_START_IBSS != pRoamProfile->BSSType )
    {
        hddLog (VOS_TRACE_LEVEL_ERROR, 
                "%s Interface type is not set to IBSS \n", __func__);
        return -EINVAL;
    }

    /*             */
    if (NULL != params->channel)
    {
        u8 channelNum;
        if (IEEE80211_BAND_5GHZ == params->channel->band)
        {
            hddLog(VOS_TRACE_LEVEL_ERROR, 
                    "%s: IBSS join is called with unsupported band %d", 
                    __func__, params->channel->band);
            return -EOPNOTSUPP;
        }

        /*                    */
        channelNum = 
               ieee80211_frequency_to_channel(params->channel->center_freq);

        /*               */
        if (14 >= channelNum)
        {
            v_U32_t numChans = WNI_CFG_VALID_CHANNEL_LIST_LEN;
            v_U8_t validChan[WNI_CFG_VALID_CHANNEL_LIST_LEN];
            tHalHandle hHal = WLAN_HDD_GET_HAL_CTX(pAdapter);
            int indx;

            if (0 != ccmCfgGetStr(hHal, WNI_CFG_VALID_CHANNEL_LIST,
                        validChan, &numChans))
            {
                hddLog(VOS_TRACE_LEVEL_ERROR, "%s: No valid channel list", 
                        __func__);
                return -EOPNOTSUPP;
            }

            for (indx = 0; indx < numChans; indx++)
            {
                if (channelNum == validChan[indx])
                {
                    break;
                }
            }
            if (indx >= numChans)
            {
                hddLog(VOS_TRACE_LEVEL_ERROR, "%s: Not valid Channel %d", 
                        __func__, channelNum);
                return -EINVAL;
            }
            /*                             */
            hddLog(VOS_TRACE_LEVEL_INFO_HIGH, "%s: set channel %d", __func__, 
                    channelNum);
            pRoamProfile->ChannelInfo.numOfChannels = 1;
            pHddStaCtx->conn_info.operationChannel = channelNum;
            pRoamProfile->ChannelInfo.ChannelList = 
                &pHddStaCtx->conn_info.operationChannel;
        }
        else
        {
            hddLog(VOS_TRACE_LEVEL_ERROR, "%s: Not valid Channel %hu", 
                    __func__, channelNum);
            return -EINVAL;
        }
    }

    /*                                */
    status = wlan_hdd_cfg80211_set_privacy_ibss(pAdapter, params); 
    if (status < 0)
    {
        hddLog(VOS_TRACE_LEVEL_ERROR, "%s: failed to set security parameters", 
                __func__);
        return status;
    }

    /*                     */
    status = wlan_hdd_cfg80211_connect_start(pAdapter, params->ssid, 
            params->ssid_len, params->bssid, 0);

    if (0 > status)
    {
        hddLog(VOS_TRACE_LEVEL_ERROR, "%s: connect failed", __func__);
        return status;
    }

    return 0;
}

/*
                                         
                                          
 */
static int wlan_hdd_cfg80211_leave_ibss( struct wiphy *wiphy, 
                                         struct net_device *dev
                                         )
{
    hdd_adapter_t *pAdapter = WLAN_HDD_GET_PRIV_PTR( dev ); 
    hdd_wext_state_t *pWextState = WLAN_HDD_GET_WEXT_STATE_PTR(pAdapter);
    tCsrRoamProfile *pRoamProfile;

    ENTER();

    if ( (WLAN_HDD_GET_CTX(pAdapter))->isLogpInProgress )
    {
        VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR,
                "%s:LOGP in Progress. Ignore!!!", __func__);
        return -EAGAIN;
    }

    hddLog(VOS_TRACE_LEVEL_INFO, "%s: device_mode = %d\n",__func__,pAdapter->device_mode);
    if (NULL == pWextState)
    {
        hddLog (VOS_TRACE_LEVEL_ERROR, "%s ERROR: Data Storage Corruption\n", 
                __func__);
        return -EIO;
    }

    pRoamProfile = &pWextState->roamProfile;

    /*                                                        */
    if (eCSR_BSS_TYPE_START_IBSS != pRoamProfile->BSSType)
    {
        hddLog (VOS_TRACE_LEVEL_ERROR, "%s: BSS Type is not set to IBSS", 
                __func__);
        return -EINVAL;
    }

    /*                          */
    INIT_COMPLETION(pAdapter->disconnect_comp_var);
    sme_RoamDisconnect( WLAN_HDD_GET_HAL_CTX(pAdapter), pAdapter->sessionId,
                                  eCSR_DISCONNECT_REASON_IBSS_LEAVE);

    return 0;
}

/*
                                               
                                                  
                                                     
 */
static int wlan_hdd_cfg80211_set_wiphy_params(struct wiphy *wiphy, 
        u32 changed)
{
    hdd_context_t *pHddCtx = wiphy_priv(wiphy);
    tHalHandle hHal = pHddCtx->hHal;

    ENTER();

    if ( pHddCtx->isLogpInProgress )
    {
        VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR,
                "%s:LOGP in Progress. Ignore!!!", __func__);
        return -EAGAIN;
    }

    if (changed & WIPHY_PARAM_RTS_THRESHOLD)
    {
        u16 rts_threshold = (wiphy->rts_threshold == -1) ?
                               WNI_CFG_RTS_THRESHOLD_STAMAX :
                               wiphy->rts_threshold;

        if ((WNI_CFG_RTS_THRESHOLD_STAMIN > rts_threshold) ||
                (WNI_CFG_RTS_THRESHOLD_STAMAX < rts_threshold)) 
        {
            hddLog(VOS_TRACE_LEVEL_ERROR, 
                    "%s: Invalid RTS Threshold value %hu", 
                    __func__, rts_threshold);
            return -EINVAL;
        }

        if (0 != ccmCfgSetInt(hHal, WNI_CFG_RTS_THRESHOLD,
                    rts_threshold, ccmCfgSetCallback,
                    eANI_BOOLEAN_TRUE)) 
        {
            hddLog(VOS_TRACE_LEVEL_ERROR, 
                    "%s: ccmCfgSetInt failed for rts_threshold value %hu", 
                    __func__, rts_threshold);
            return -EIO;
        }

        hddLog(VOS_TRACE_LEVEL_INFO_MED, "%s: set rts threshold %hu", __func__, 
                rts_threshold);
    }

    if (changed & WIPHY_PARAM_FRAG_THRESHOLD)
    {
        u16 frag_threshold = (wiphy->frag_threshold == -1) ?
                                WNI_CFG_FRAGMENTATION_THRESHOLD_STAMAX :
                                wiphy->frag_threshold;

        if ((WNI_CFG_FRAGMENTATION_THRESHOLD_STAMIN > frag_threshold)||
                (WNI_CFG_FRAGMENTATION_THRESHOLD_STAMAX < frag_threshold) ) 
        {
            hddLog(VOS_TRACE_LEVEL_ERROR, 
                    "%s: Invalid frag_threshold value %hu", __func__, 
                    frag_threshold);
            return -EINVAL;
        }

        if (0 != ccmCfgSetInt(hHal, WNI_CFG_FRAGMENTATION_THRESHOLD,
                    frag_threshold, ccmCfgSetCallback,
                    eANI_BOOLEAN_TRUE)) 
        {
            hddLog(VOS_TRACE_LEVEL_ERROR, 
                    "%s: ccmCfgSetInt failed for frag_threshold value %hu", 
                    __func__, frag_threshold);
            return -EIO;
        }

        hddLog(VOS_TRACE_LEVEL_INFO_MED, "%s: set frag threshold %hu", __func__,
                frag_threshold);
    }

    if ((changed & WIPHY_PARAM_RETRY_SHORT)
            || (changed & WIPHY_PARAM_RETRY_LONG))
    {
        u8 retry_value = (changed & WIPHY_PARAM_RETRY_SHORT) ?
                         wiphy->retry_short :
                         wiphy->retry_long;

        if ((WNI_CFG_LONG_RETRY_LIMIT_STAMIN > retry_value) ||
                (WNI_CFG_LONG_RETRY_LIMIT_STAMAX < retry_value))
        {
            hddLog(VOS_TRACE_LEVEL_ERROR, "%s: Invalid Retry count %hu", 
                    __func__, retry_value);
            return -EINVAL;
        }

        if (changed & WIPHY_PARAM_RETRY_SHORT)
        {
            if (0 != ccmCfgSetInt(hHal, WNI_CFG_LONG_RETRY_LIMIT,
                        retry_value, ccmCfgSetCallback,
                        eANI_BOOLEAN_TRUE)) 
            {
                hddLog(VOS_TRACE_LEVEL_ERROR, 
                        "%s: ccmCfgSetInt failed for long retry count %hu", 
                        __func__, retry_value);
                return -EIO;
            }
            hddLog(VOS_TRACE_LEVEL_INFO_MED, "%s: set long retry count %hu", 
                    __func__, retry_value);
        }
        else if (changed & WIPHY_PARAM_RETRY_SHORT)
        {
            if (0 != ccmCfgSetInt(hHal, WNI_CFG_SHORT_RETRY_LIMIT,
                        retry_value, ccmCfgSetCallback,
                        eANI_BOOLEAN_TRUE)) 
            {
                hddLog(VOS_TRACE_LEVEL_ERROR, 
                        "%s: ccmCfgSetInt failed for short retry count %hu", 
                        __func__, retry_value);
                return -EIO;
            }
            hddLog(VOS_TRACE_LEVEL_INFO_MED, "%s: set short retry count %hu", 
                    __func__, retry_value);
        }
    }

    return 0;
}

/*
                                          
                                           
 */
static int wlan_hdd_cfg80211_set_txpower(struct wiphy *wiphy,
#if LINUX_VERSION_CODE <= KERNEL_VERSION(2,6,35)
        enum tx_power_setting type, 
#else
        enum nl80211_tx_power_setting type, 
#endif
        int dbm)
{
    hdd_context_t *pHddCtx = (hdd_context_t*) wiphy_priv(wiphy);
    tHalHandle hHal = pHddCtx->hHal;
    tSirMacAddr bssid = {0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};
    tSirMacAddr selfMac = {0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};

    ENTER();

    if (0 != ccmCfgSetInt(hHal, WNI_CFG_CURRENT_TX_POWER_LEVEL, 
                dbm, ccmCfgSetCallback, 
                eANI_BOOLEAN_TRUE)) 
    {
        hddLog(VOS_TRACE_LEVEL_ERROR, 
                "%s: ccmCfgSetInt failed for tx power %hu", __func__, dbm);
        return -EIO;
    }

    if ( pHddCtx->isLogpInProgress )
    {
        VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR,
                "%s:LOGP in Progress. Ignore!!!", __func__);
        return -EAGAIN;
    }

    hddLog(VOS_TRACE_LEVEL_INFO_MED, "%s: set tx power level %d dbm", __func__,
            dbm);

    switch(type)
    {
    case NL80211_TX_POWER_AUTOMATIC: /*                                      */
       /*              */
    case NL80211_TX_POWER_LIMITED: /*                                   */
       if( sme_SetMaxTxPower(hHal, bssid, selfMac, dbm) != eHAL_STATUS_SUCCESS )
       {
          hddLog(VOS_TRACE_LEVEL_ERROR, "%s: Setting maximum tx power failed", 
                 __func__); 
          return -EIO;          
       }
       break;
    case NL80211_TX_POWER_FIXED: /*                                 */
       hddLog(VOS_TRACE_LEVEL_ERROR, "%s: NL80211_TX_POWER_FIXED not supported", 
              __func__);
       return -EOPNOTSUPP;
       break;
    default:
       hddLog(VOS_TRACE_LEVEL_ERROR, "%s: Invalid power setting type %d", 
              __func__, type); 
       return -EIO;
    }

    return 0;
}

/*
                                          
                                            
 */
static int wlan_hdd_cfg80211_get_txpower(struct wiphy *wiphy, int *dbm)
{

    hdd_adapter_t *pAdapter;
    hdd_context_t *pHddCtx = (hdd_context_t*) wiphy_priv(wiphy);

    ENTER();

    if (NULL == pHddCtx)
    {
        hddLog(VOS_TRACE_LEVEL_FATAL,"%s: HDD context is Null",__func__);
        *dbm = 0;
        return -ENOENT;
    }

    if ( pHddCtx->isLogpInProgress )
    {
        VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR,
                "%s:LOGP in Progress. Ignore!!!", __func__);
        return -EAGAIN;
    }

    pAdapter = hdd_get_adapter(pHddCtx, WLAN_HDD_INFRA_STATION);
    if (NULL == pAdapter)
    {
        hddLog(VOS_TRACE_LEVEL_FATAL, "%s: Not in station context " ,__func__);
        return -ENOENT;
    }

    wlan_hdd_get_classAstats(pAdapter);
    *dbm = pAdapter->hdd_stats.ClassA_stat.max_pwr;

    EXIT();
    return 0;
}

static int wlan_hdd_cfg80211_get_station(struct wiphy *wiphy, struct net_device *dev,
                                   u8* mac, struct station_info *sinfo)
{
    hdd_adapter_t *pAdapter = WLAN_HDD_GET_PRIV_PTR( dev );
    hdd_station_ctx_t *pHddStaCtx = WLAN_HDD_GET_STATION_CTX_PTR(pAdapter);
    int ssidlen = pHddStaCtx->conn_info.SSID.SSID.length;
    tANI_U8 rate_flags;

    hdd_context_t *pHddCtx = (hdd_context_t*) wiphy_priv(wiphy);
    hdd_config_t  *pCfg    = pHddCtx->cfg_ini;
    tHalHandle hHal        = WLAN_HDD_GET_HAL_CTX(pAdapter);

    tANI_U8  OperationalRates[CSR_DOT11_SUPPORTED_RATES_MAX];
    tANI_U32 ORLeng = CSR_DOT11_SUPPORTED_RATES_MAX;
    tANI_U8  ExtendedRates[CSR_DOT11_EXTENDED_SUPPORTED_RATES_MAX];
    tANI_U32 ERLeng = CSR_DOT11_EXTENDED_SUPPORTED_RATES_MAX;
    tANI_U8  MCSRates[SIZE_OF_BASIC_MCS_SET];
    tANI_U32 MCSLeng = SIZE_OF_BASIC_MCS_SET;
    tANI_U16 maxRate = 0;
    tANI_U16 myRate;
    tANI_U16 currentRate = 0;
    tANI_U8  maxSpeedMCS = 0;
    tANI_U8  maxMCSIdx = 0;
    tANI_U8  rateFlag = 1;
    tANI_U8  i, j, rssidx;

    ENTER();

    if ((eConnectionState_Associated != pHddStaCtx->conn_info.connState) ||
            (0 == ssidlen))
    {
        hddLog(VOS_TRACE_LEVEL_INFO, "%s: Not associated or"
                    " Invalid ssidlen, %d", __func__, ssidlen);
        /*                 */
        return 0;
    }

    if ( (WLAN_HDD_GET_CTX(pAdapter))->isLogpInProgress )
    {
        VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR,
                "%s:LOGP in Progress. Ignore!!!", __func__);
        return -EAGAIN;
    }

    wlan_hdd_get_rssi(pAdapter, &sinfo->signal);
    sinfo->filled |= STATION_INFO_SIGNAL;

    wlan_hdd_get_station_stats(pAdapter);
    rate_flags = pAdapter->hdd_stats.ClassA_stat.tx_rate_flags;

    //                                  
    myRate = pAdapter->hdd_stats.ClassA_stat.tx_rate * 5;

#ifdef LINKSPEED_DEBUG_ENABLED
    pr_info("RSSI %d, RLMS %u, rate %d, rssi high %d, rssi low %d\n",
            sinfo->signal,
            pCfg->reportMaxLinkSpeed,
            myRate,
            (int) pCfg->linkSpeedRssiHigh,
            (int) pCfg->linkSpeedRssiLow);
#endif //                       

    if (eHDD_LINK_SPEED_REPORT_ACTUAL != pCfg->reportMaxLinkSpeed)
    {
        //                                                       
        if (eHDD_LINK_SPEED_REPORT_MAX == pCfg->reportMaxLinkSpeed)
        {
            //                              
            rssidx = 0;
        }
        else if (eHDD_LINK_SPEED_REPORT_MAX_SCALED == pCfg->reportMaxLinkSpeed)
        {
            //                                                
            if (sinfo->signal >= pCfg->linkSpeedRssiHigh)
            {
                //                              
                rssidx = 0;
            }
            else if (sinfo->signal >= pCfg->linkSpeedRssiLow)
            {
                //                    
                rssidx = 1;
            }
            else
            {
                //                    
                rssidx = 2;
            }
        }
        else
        {
            //                                             
            hddLog(VOS_TRACE_LEVEL_ERROR,
                   "%s: Invalid value for reportMaxLinkSpeed: %u",
                    __func__, pCfg->reportMaxLinkSpeed);
            rssidx = 0;
        }

        maxRate = 0;

        /*                    */
        ccmCfgGetStr(hHal, WNI_CFG_OPERATIONAL_RATE_SET, OperationalRates, &ORLeng);
        for (i = 0; i < ORLeng; i++)
        {
            for (j = 0; j < (sizeof(supported_data_rate) / sizeof(supported_data_rate[0])); j ++)
            {
                /*                   */
                if (supported_data_rate[j].beacon_rate_index == (OperationalRates[i] & 0x7F))
                {
                    currentRate = supported_data_rate[j].supported_rate[rssidx];
                    break;
                }
            }
            /*                 */
            maxRate = (currentRate > maxRate)?currentRate:maxRate;
        }

        /*                       */
        ccmCfgGetStr(hHal, WNI_CFG_EXTENDED_OPERATIONAL_RATE_SET, ExtendedRates, &ERLeng);
        for (i = 0; i < ERLeng; i++)
        {
            for (j = 0; j < (sizeof(supported_data_rate) / sizeof(supported_data_rate[0])); j ++)
            {
                if (supported_data_rate[j].beacon_rate_index == (ExtendedRates[i] & 0x7F))
                {
                    currentRate = supported_data_rate[j].supported_rate[rssidx];
                    break;
                }
            }
            /*                 */
            maxRate = (currentRate > maxRate)?currentRate:maxRate;
        }

        /*                                                        
                                                                      
                     */
        if ((0 == rssidx) || !(rate_flags & eHAL_TX_RATE_LEGACY))
        {
            ccmCfgGetStr(hHal, WNI_CFG_CURRENT_MCS_SET, MCSRates, &MCSLeng);
            rateFlag = 0;
            if (rate_flags & eHAL_TX_RATE_HT40)
            {
                rateFlag |= 1;
            }
            if (rate_flags & eHAL_TX_RATE_SGI)
            {
                rateFlag |= 2;
            }

            for (i = 0; i < MCSLeng; i++)
            {
                for (j = 0; j < (sizeof(supported_mcs_rate) / sizeof(supported_mcs_rate[0])); j++)
                {
                    if (supported_mcs_rate[j].beacon_rate_index == MCSRates[i])
                    {
                        currentRate = supported_mcs_rate[j].supported_rate[rateFlag];
                        break;
                    }
                }
                if (currentRate > maxRate)
                {
                    maxRate     = currentRate;
                    maxSpeedMCS = 1;
                    maxMCSIdx   = supported_mcs_rate[j].beacon_rate_index;
                }
            }
        }

        //                                                                
        if (maxRate < myRate)
        {
           maxRate = myRate;
           if (rate_flags & eHAL_TX_RATE_LEGACY)
           {
              maxSpeedMCS = 0;
           }
           else
           {
              maxSpeedMCS = 1;
              maxMCSIdx = pAdapter->hdd_stats.ClassA_stat.mcs_index;
           }
        }

        if (!maxSpeedMCS)
        {
            sinfo->txrate.legacy  = maxRate;
#ifdef LINKSPEED_DEBUG_ENABLED
            pr_info("Reporting legacy rate %d\n", sinfo->txrate.legacy);
#endif //                       
        }
        else
        {
            sinfo->txrate.mcs    = maxMCSIdx;
            sinfo->txrate.flags |= RATE_INFO_FLAGS_MCS;
            if (rate_flags & eHAL_TX_RATE_SGI)
            {
                sinfo->txrate.flags |= RATE_INFO_FLAGS_SHORT_GI;
            }
            if (rate_flags & eHAL_TX_RATE_HT40)
            {
                sinfo->txrate.flags |= RATE_INFO_FLAGS_40_MHZ_WIDTH;
            }
#ifdef LINKSPEED_DEBUG_ENABLED
            pr_info("Reporting MCS rate %d flags %x\n",
                    sinfo->txrate.mcs,
                    sinfo->txrate.flags );
#endif //                       
        }
    }
    else
    {
        //                                        

        if (rate_flags & eHAL_TX_RATE_LEGACY)
        {
            //                                     
            sinfo->txrate.legacy = myRate;
#ifdef LINKSPEED_DEBUG_ENABLED
            pr_info("Reporting actual legacy rate %d\n", sinfo->txrate.legacy);
#endif //                       
        }
        else
        {
            //           
            sinfo->txrate.mcs = pAdapter->hdd_stats.ClassA_stat.mcs_index;
            sinfo->txrate.flags |= RATE_INFO_FLAGS_MCS;
            if (rate_flags & eHAL_TX_RATE_SGI)
            {
                sinfo->txrate.flags |= RATE_INFO_FLAGS_SHORT_GI;
            }
            if (rate_flags & eHAL_TX_RATE_HT40)
            {
                sinfo->txrate.flags |= RATE_INFO_FLAGS_40_MHZ_WIDTH;
            }
#ifdef LINKSPEED_DEBUG_ENABLED
            pr_info("Reporting actual MCS rate %d flags %x\n",
                    sinfo->txrate.mcs,
                    sinfo->txrate.flags );
#endif //                       
        }
    }
    sinfo->filled |= STATION_INFO_TX_BITRATE;

    sinfo->tx_packets =
       pAdapter->hdd_stats.summary_stat.tx_frm_cnt[0] +
       pAdapter->hdd_stats.summary_stat.tx_frm_cnt[1] +
       pAdapter->hdd_stats.summary_stat.tx_frm_cnt[2] +
       pAdapter->hdd_stats.summary_stat.tx_frm_cnt[3];

    sinfo->tx_retries =
       pAdapter->hdd_stats.summary_stat.retry_cnt[0] +
       pAdapter->hdd_stats.summary_stat.retry_cnt[1] +
       pAdapter->hdd_stats.summary_stat.retry_cnt[2] +
       pAdapter->hdd_stats.summary_stat.retry_cnt[3];

    sinfo->tx_failed =
       pAdapter->hdd_stats.summary_stat.fail_cnt[0] +
       pAdapter->hdd_stats.summary_stat.fail_cnt[1] +
       pAdapter->hdd_stats.summary_stat.fail_cnt[2] +
       pAdapter->hdd_stats.summary_stat.fail_cnt[3];

    sinfo->filled |=
       STATION_INFO_TX_PACKETS |
       STATION_INFO_TX_RETRIES |
       STATION_INFO_TX_FAILED;

       EXIT();
       return 0;
}

static int wlan_hdd_cfg80211_set_power_mgmt(struct wiphy *wiphy,
                     struct net_device *dev, bool mode, v_SINT_t timeout)
{
    hdd_adapter_t *pAdapter = WLAN_HDD_GET_PRIV_PTR(dev);
    VOS_STATUS vos_status;

    ENTER();

    if (NULL == pAdapter)
    {
        hddLog(VOS_TRACE_LEVEL_ERROR, "%s: Adapter is NULL\n", __func__);
        return -ENODEV;
    }
    if ( (WLAN_HDD_GET_CTX(pAdapter))->isLogpInProgress )
    {
        VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR,
                "%s:LOGP in Progress. Ignore!!!", __func__);
        return -EAGAIN;
    }

    /*                                                                  
                                                  
                                                       
      */
    vos_status =  wlan_hdd_enter_bmps(pAdapter, !mode);

    EXIT();
    if (VOS_STATUS_E_FAILURE == vos_status)
    {
        return -EINVAL;
    }
    return 0;
}


#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,38))
static int wlan_hdd_set_default_mgmt_key(struct wiphy *wiphy,
                         struct net_device *netdev,
                         u8 key_index)
{
    ENTER();
    return 0;
}
#endif //                   

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3,4,0))
static int wlan_hdd_set_txq_params(struct wiphy *wiphy,
                   struct net_device *dev,
                   struct ieee80211_txq_params *params)
{
    ENTER();
    return 0;
}
#elif (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,38))
static int wlan_hdd_set_txq_params(struct wiphy *wiphy,
                   struct ieee80211_txq_params *params)
{
    ENTER();
    return 0;
}
#endif //                  

static int wlan_hdd_cfg80211_del_station(struct wiphy *wiphy,
                                         struct net_device *dev, u8 *mac)
{
    hdd_adapter_t *pAdapter = WLAN_HDD_GET_PRIV_PTR(dev);

    ENTER();
    if ( NULL == pAdapter || NULL == pAdapter->pHddCtx)
    {
        hddLog(VOS_TRACE_LEVEL_FATAL, "%s: Invalid Adapter or HDD Context " ,__func__);
        return -EINVAL;
    }

    if (((hdd_context_t*)pAdapter->pHddCtx)->isLoadUnloadInProgress)
    {
         hddLog( LOGE,
                 "%s: Wlan Load/Unload is in progress", __func__);
         return -EBUSY;
    }

    if ( (WLAN_HDD_GET_CTX(pAdapter))->isLogpInProgress )
    {
        VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR,
                "%s:LOGP in Progress. Ignore!!!", __func__);
        return -EAGAIN;
    }

    if ( (WLAN_HDD_SOFTAP == pAdapter->device_mode)
#ifdef WLAN_FEATURE_P2P
       || (WLAN_HDD_P2P_GO == pAdapter->device_mode)
#endif
       )
    {
        if( NULL == mac )
        {
            v_U16_t i;
            for(i = 0; i < WLAN_MAX_STA_COUNT; i++)
            {
                if(pAdapter->aStaInfo[i].isUsed)
                {
                    u8 *macAddr = pAdapter->aStaInfo[i].macAddrSTA.bytes;
                    hddLog(VOS_TRACE_LEVEL_INFO,
                                        "%s: Delete STA with MAC::"
                                        "%02x:%02x:%02x:%02x:%02x:%02x",
                                        __func__,
                                        macAddr[0], macAddr[1], macAddr[2],
                                        macAddr[3], macAddr[4], macAddr[5]);
                    hdd_softap_sta_deauth(pAdapter, macAddr);
                }
            }
        }
        else
        {
            hddLog(VOS_TRACE_LEVEL_INFO,
                                "%s: Delete STA with MAC::"
                                "%02x:%02x:%02x:%02x:%02x:%02x",
                                __func__,
                                mac[0], mac[1], mac[2],
                                mac[3], mac[4], mac[5]);
            hdd_softap_sta_deauth(pAdapter, mac);
        }
    }

    EXIT();

    return 0;
}

static int wlan_hdd_cfg80211_add_station(struct wiphy *wiphy,
          struct net_device *dev, u8 *mac, struct station_parameters *params)
{
    //                            
    ENTER();
    return 0;
}


#ifdef FEATURE_WLAN_LFR
static int wlan_hdd_cfg80211_set_pmksa(struct wiphy *wiphy, struct net_device *dev,
            struct cfg80211_pmksa *pmksa)
{
#define MAX_PMKSAIDS_IN_CACHE 8
    static tPmkidCacheInfo PMKIDCache[MAX_PMKSAIDS_IN_CACHE]; //                
    static tANI_U32 i = 0; //                       
    tANI_U32 j=0;  
    hdd_adapter_t *pAdapter = WLAN_HDD_GET_PRIV_PTR(dev);
    tHalHandle halHandle;
    eHalStatus result; 
    tANI_U8  BSSIDMatched = 0;
   
    ENTER();

    //                    
    if ( NULL == pAdapter || NULL == pAdapter->pHddCtx)
    {
        hddLog(VOS_TRACE_LEVEL_FATAL, "%s: Invalid Adapter or HDD Context " ,__func__);
        return -EINVAL;
    }

    if (((hdd_context_t*)pAdapter->pHddCtx)->isLoadUnloadInProgress)
    {
         hddLog( LOGE,
                 "%s: Wlan Load/Unload is in progress", __func__);
         return -EBUSY;
    }

    if ( (WLAN_HDD_GET_CTX(pAdapter))->isLogpInProgress )
    {
        VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR,
                "%s:LOGP in Progress. Ignore!!!", __func__);
        return -EAGAIN;
    }

    //                     
    halHandle = WLAN_HDD_GET_HAL_CTX(pAdapter);

    for (j = 0; j < i; j++)
    {
        if(vos_mem_compare(PMKIDCache[j].BSSID, 
                    pmksa->bssid, WNI_CFG_BSSID_LEN))
        {
            /*                                              */
            BSSIDMatched = 1;
            vos_mem_copy(PMKIDCache[j].BSSID, 
                    pmksa->bssid, WNI_CFG_BSSID_LEN);
            vos_mem_copy(PMKIDCache[j].PMKID, 
                    pmksa->pmkid,   
                    CSR_RSN_PMKID_SIZE);
            hddLog(VOS_TRACE_LEVEL_FATAL, "%s: Reusing cache entry %d.", 
                    __FUNCTION__, j );
            dump_bssid(pmksa->bssid);
            dump_pmkid(halHandle, pmksa->pmkid);
            break;
        }
    }

    /*                                                               */
    if(j == MAX_PMKSAIDS_IN_CACHE) i=0;

    if (!BSSIDMatched)
    { 
        //                                                                      
        vos_mem_copy(PMKIDCache[i].BSSID, 
                pmksa->bssid, ETHER_ADDR_LEN); 
        vos_mem_copy(PMKIDCache[i].PMKID, 
                pmksa->pmkid,   
                CSR_RSN_PMKID_SIZE);
        hddLog(VOS_TRACE_LEVEL_FATAL, "%s: Adding a new cache entry %d.", 
                __FUNCTION__, i );
        dump_bssid(pmksa->bssid);
        dump_pmkid(halHandle, pmksa->pmkid);
        //                                     
        //                                                                           
        if (i<=(MAX_PMKSAIDS_IN_CACHE-1)) i++; else i=0; 
    }


    //                                                                    
    //                                                                            
    //                          
    hddLog(VOS_TRACE_LEVEL_FATAL, "%s: Calling csrRoamSetPMKIDCache with %d cache entries.", 
            __FUNCTION__, i );
    //                                      
    result = sme_RoamSetPMKIDCache(halHandle,pAdapter->sessionId, 
                                    PMKIDCache, 
                                    i );
    return 0;
}


static int wlan_hdd_cfg80211_del_pmksa(struct wiphy *wiphy, struct net_device *dev,
            struct cfg80211_pmksa *pmksa)
{
    ENTER();
    //                            
    return 0;
}

static int wlan_hdd_cfg80211_flush_pmksa(struct wiphy *wiphy, struct net_device *dev)
{
    ENTER();
    //                            
    return 0;
}
#endif


/*              */
static struct cfg80211_ops wlan_hdd_cfg80211_ops = 
{
    .add_virtual_intf = wlan_hdd_add_virtual_intf,
    .del_virtual_intf = wlan_hdd_del_virtual_intf,
    .change_virtual_intf = wlan_hdd_cfg80211_change_iface,
    .change_station = wlan_hdd_change_station,
#if (LINUX_VERSION_CODE < KERNEL_VERSION(3,4,0))
    .add_beacon = wlan_hdd_cfg80211_add_beacon,
    .del_beacon = wlan_hdd_cfg80211_del_beacon,
    .set_beacon = wlan_hdd_cfg80211_set_beacon,
#else
    .start_ap = wlan_hdd_cfg80211_start_ap,
    .change_beacon = wlan_hdd_cfg80211_change_beacon,
    .stop_ap = wlan_hdd_cfg80211_stop_ap,
#endif
    .change_bss = wlan_hdd_cfg80211_change_bss,
    .add_key = wlan_hdd_cfg80211_add_key,
    .get_key = wlan_hdd_cfg80211_get_key,
    .del_key = wlan_hdd_cfg80211_del_key,
    .set_default_key = wlan_hdd_cfg80211_set_default_key,
    .set_channel = wlan_hdd_cfg80211_set_channel,
    .scan = wlan_hdd_cfg80211_scan,
    .connect = wlan_hdd_cfg80211_connect,
    .disconnect = wlan_hdd_cfg80211_disconnect,
    .join_ibss  = wlan_hdd_cfg80211_join_ibss,
    .leave_ibss = wlan_hdd_cfg80211_leave_ibss,
    .set_wiphy_params = wlan_hdd_cfg80211_set_wiphy_params,
    .set_tx_power = wlan_hdd_cfg80211_set_txpower,
    .get_tx_power = wlan_hdd_cfg80211_get_txpower,
#ifdef WLAN_FEATURE_P2P
    .remain_on_channel = wlan_hdd_cfg80211_remain_on_channel,
    .cancel_remain_on_channel =  wlan_hdd_cfg80211_cancel_remain_on_channel,
    .mgmt_tx =  wlan_hdd_action,
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,38))
     .mgmt_tx_cancel_wait = wlan_hdd_cfg80211_mgmt_tx_cancel_wait,
     .set_default_mgmt_key = wlan_hdd_set_default_mgmt_key,
     .set_txq_params = wlan_hdd_set_txq_params,
#endif
#endif
     .get_station = wlan_hdd_cfg80211_get_station,
     .set_power_mgmt = wlan_hdd_cfg80211_set_power_mgmt,
     .del_station  = wlan_hdd_cfg80211_del_station,
     .add_station  = wlan_hdd_cfg80211_add_station,
#ifdef FEATURE_WLAN_LFR
     .set_pmksa = wlan_hdd_cfg80211_set_pmksa,
     .del_pmksa = wlan_hdd_cfg80211_del_pmksa,
     .flush_pmksa = wlan_hdd_cfg80211_flush_pmksa,
#endif
};

#endif //                
