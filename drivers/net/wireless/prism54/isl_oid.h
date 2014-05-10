/*
 *  Copyright (C) 2003 Herbert Valerio Riedel <hvr@gnu.org>
 *  Copyright (C) 2004 Luis R. Rodriguez <mcgrof@ruslug.rutgers.edu>
 *  Copyright (C) 2004 Aurelien Alleaume <slts@free.fr>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#if !defined(_ISL_OID_H)
#define _ISL_OID_H

/*
                                                                   
                           
 */

struct obj_ssid {
	u8 length;
	char octets[33];
} __packed;

struct obj_key {
	u8 type;		/*              */
	u8 length;
	char key[32];
} __packed;

struct obj_mlme {
	u8 address[6];
	u16 id;
	u16 state;
	u16 code;
} __packed;

struct obj_mlmeex {
	u8 address[6];
	u16 id;
	u16 state;
	u16 code;
	u16 size;
	u8 data[0];
} __packed;

struct obj_buffer {
	u32 size;
	u32 addr;		/*                   */
} __packed;

struct obj_bss {
	u8 address[6];
	int:16;			/*         */

	char state;
	char reserved;
	short age;

	char quality;
	char rssi;

	struct obj_ssid ssid;
	short channel;
	char beacon_period;
	char dtim_period;
	short capinfo;
	short rates;
	short basic_rates;
	int:16;			/*         */
} __packed;

struct obj_bsslist {
	u32 nr;
	struct obj_bss bsslist[0];
} __packed;

struct obj_frequencies {
	u16 nr;
	u16 mhz[0];
} __packed;

struct obj_attachment {
	char type;
	char reserved;
	short id;
	short size;
	char data[0];
} __packed;

/*
                                                              
                                    
 */
static inline void
__bug_on_wrong_struct_sizes(void)
{
	BUILD_BUG_ON(sizeof (struct obj_ssid) != 34);
	BUILD_BUG_ON(sizeof (struct obj_key) != 34);
	BUILD_BUG_ON(sizeof (struct obj_mlme) != 12);
	BUILD_BUG_ON(sizeof (struct obj_mlmeex) != 14);
	BUILD_BUG_ON(sizeof (struct obj_buffer) != 8);
	BUILD_BUG_ON(sizeof (struct obj_bss) != 60);
	BUILD_BUG_ON(sizeof (struct obj_bsslist) != 4);
	BUILD_BUG_ON(sizeof (struct obj_frequencies) != 2);
}

enum dot11_state_t {
	DOT11_STATE_NONE = 0,
	DOT11_STATE_AUTHING = 1,
	DOT11_STATE_AUTH = 2,
	DOT11_STATE_ASSOCING = 3,

	DOT11_STATE_ASSOC = 5,
	DOT11_STATE_IBSS = 6,
	DOT11_STATE_WDS = 7
};

enum dot11_bsstype_t {
	DOT11_BSSTYPE_NONE = 0,
	DOT11_BSSTYPE_INFRA = 1,
	DOT11_BSSTYPE_IBSS = 2,
	DOT11_BSSTYPE_ANY = 3
};

enum dot11_auth_t {
	DOT11_AUTH_NONE = 0,
	DOT11_AUTH_OS = 1,
	DOT11_AUTH_SK = 2,
	DOT11_AUTH_BOTH = 3
};

enum dot11_mlme_t {
	DOT11_MLME_AUTO = 0,
	DOT11_MLME_INTERMEDIATE = 1,
	DOT11_MLME_EXTENDED = 2
};

enum dot11_priv_t {
	DOT11_PRIV_WEP = 0,
	DOT11_PRIV_TKIP = 1
};

/*                                                     
                                                          
                                                                     
                                                               
                               
                
                                                                                   
 */
enum dot11_maxframeburst_t {
	/*                                    */
	DOT11_MAXFRAMEBURST_OFF = 0, /*                       */
	DOT11_MAXFRAMEBURST_MIXED_SAFE = 650, /*                   */
	DOT11_MAXFRAMEBURST_IDEAL = 1300, /*                         */
	DOT11_MAXFRAMEBURST_MAX = 5000, /*                 
                                                       
                                               
                                                                  
                                                 
  */
};

/*                                                   
                                                     
                                                     
  
                                         
                                             
                         */
enum dot11_preamblesettings_t {
	DOT11_PREAMBLESETTING_LONG = 0,
		/*                                     */
	DOT11_PREAMBLESETTING_SHORT = 1,
		/*                                      */
	DOT11_PREAMBLESETTING_DYNAMIC = 2
		/*                     */
};

/*                                                       
  
                                            
                                             */
enum dot11_slotsettings_t {
	DOT11_SLOTSETTINGS_LONG = 0,
		/*                                        */
	DOT11_SLOTSETTINGS_SHORT = 1,
		/*                                        */
	DOT11_SLOTSETTINGS_DYNAMIC = 2
		/*                     */
};

/*                                                  
                                                                     
                               
                                  
                                  
                       
  
                                            
                                                              
              
 */
enum do11_nonerpstatus_t {
	DOT11_ERPSTAT_NONEPRESENT = 0,
	DOT11_ERPSTAT_USEPROTECTION = 1
};

/*                                                             
                                                       
                                              
                                                      
                                  */
enum dot11_nonerpprotection_t {
	DOT11_NONERP_NEVER = 0,
	DOT11_NONERP_ALWAYS = 1,
	DOT11_NONERP_DYNAMIC = 2
};

/*                                          
                                                     
                            */
enum dot11_profile_t { /*                        */
	/*                                       */
	DOT11_PROFILE_B_ONLY = 0,
		/*                                   
                                                              
                        
                                                  
                                                    
   */
	DOT11_PROFILE_MIXED_G_WIFI = 1,
		/*                                                                
                                                              
                        
                                                  
                                                       
   */
	DOT11_PROFILE_MIXED_LONG = 2, /*              */
		/*                              */
	DOT11_PROFILE_G_ONLY = 3,
		/*                              */
	DOT11_PROFILE_TEST = 4,
		/*                                     
                                                            
                                                
                                                     
   */
	DOT11_PROFILE_B_WIFI = 5,
		/*                        */
	DOT11_PROFILE_A_ONLY = 6,
		/*                                     
                                                     
   */
	DOT11_PROFILE_MIXED_SHORT = 7
		/*                      */
};


/*                                                                        
                                          */
enum oid_inl_conformance_t {
	OID_INL_CONFORMANCE_NONE = 0,	/*                         */
	OID_INL_CONFORMANCE_STRICT = 1,	/*                            */
	OID_INL_CONFORMANCE_FLEXIBLE = 2,	/*                           
                                                             
                                 */
};

enum oid_inl_mode_t {
	INL_MODE_NONE = -1,
	INL_MODE_PROMISCUOUS = 0,
	INL_MODE_CLIENT = 1,
	INL_MODE_AP = 2,
	INL_MODE_SNIFFER = 3
};

enum oid_inl_config_t {
	INL_CONFIG_NOTHING = 0x00,
	INL_CONFIG_MANUALRUN = 0x01,
	INL_CONFIG_FRAMETRAP = 0x02,
	INL_CONFIG_RXANNEX = 0x04,
	INL_CONFIG_TXANNEX = 0x08,
	INL_CONFIG_WDS = 0x10
};

enum oid_inl_phycap_t {
	INL_PHYCAP_2400MHZ = 1,
	INL_PHYCAP_5000MHZ = 2,
	INL_PHYCAP_FAA = 0x80000000,	/*                                    */
};


enum oid_num_t {
	GEN_OID_MACADDRESS = 0,
	GEN_OID_LINKSTATE,
	GEN_OID_WATCHDOG,
	GEN_OID_MIBOP,
	GEN_OID_OPTIONS,
	GEN_OID_LEDCONFIG,

	/*        */
	DOT11_OID_BSSTYPE,
	DOT11_OID_BSSID,
	DOT11_OID_SSID,
	DOT11_OID_STATE,
	DOT11_OID_AID,
	DOT11_OID_COUNTRYSTRING,
	DOT11_OID_SSIDOVERRIDE,

	DOT11_OID_MEDIUMLIMIT,
	DOT11_OID_BEACONPERIOD,
	DOT11_OID_DTIMPERIOD,
	DOT11_OID_ATIMWINDOW,
	DOT11_OID_LISTENINTERVAL,
	DOT11_OID_CFPPERIOD,
	DOT11_OID_CFPDURATION,

	DOT11_OID_AUTHENABLE,
	DOT11_OID_PRIVACYINVOKED,
	DOT11_OID_EXUNENCRYPTED,
	DOT11_OID_DEFKEYID,
	DOT11_OID_DEFKEYX,	/*                                        */
	DOT11_OID_STAKEY,
	DOT11_OID_REKEYTHRESHOLD,
	DOT11_OID_STASC,

	DOT11_OID_PRIVTXREJECTED,
	DOT11_OID_PRIVRXPLAIN,
	DOT11_OID_PRIVRXFAILED,
	DOT11_OID_PRIVRXNOKEY,

	DOT11_OID_RTSTHRESH,
	DOT11_OID_FRAGTHRESH,
	DOT11_OID_SHORTRETRIES,
	DOT11_OID_LONGRETRIES,
	DOT11_OID_MAXTXLIFETIME,
	DOT11_OID_MAXRXLIFETIME,
	DOT11_OID_AUTHRESPTIMEOUT,
	DOT11_OID_ASSOCRESPTIMEOUT,

	DOT11_OID_ALOFT_TABLE,
	DOT11_OID_ALOFT_CTRL_TABLE,
	DOT11_OID_ALOFT_RETREAT,
	DOT11_OID_ALOFT_PROGRESS,
	DOT11_OID_ALOFT_FIXEDRATE,
	DOT11_OID_ALOFT_RSSIGRAPH,
	DOT11_OID_ALOFT_CONFIG,

	DOT11_OID_VDCFX,
	DOT11_OID_MAXFRAMEBURST,

	DOT11_OID_PSM,
	DOT11_OID_CAMTIMEOUT,
	DOT11_OID_RECEIVEDTIMS,
	DOT11_OID_ROAMPREFERENCE,

	DOT11_OID_BRIDGELOCAL,
	DOT11_OID_CLIENTS,
	DOT11_OID_CLIENTSASSOCIATED,
	DOT11_OID_CLIENTX,	/*                                           */

	DOT11_OID_CLIENTFIND,
	DOT11_OID_WDSLINKADD,
	DOT11_OID_WDSLINKREMOVE,
	DOT11_OID_EAPAUTHSTA,
	DOT11_OID_EAPUNAUTHSTA,
	DOT11_OID_DOT1XENABLE,
	DOT11_OID_MICFAILURE,
	DOT11_OID_REKEYINDICATE,

	DOT11_OID_MPDUTXSUCCESSFUL,
	DOT11_OID_MPDUTXONERETRY,
	DOT11_OID_MPDUTXMULTIPLERETRIES,
	DOT11_OID_MPDUTXFAILED,
	DOT11_OID_MPDURXSUCCESSFUL,
	DOT11_OID_MPDURXDUPS,
	DOT11_OID_RTSSUCCESSFUL,
	DOT11_OID_RTSFAILED,
	DOT11_OID_ACKFAILED,
	DOT11_OID_FRAMERECEIVES,
	DOT11_OID_FRAMEERRORS,
	DOT11_OID_FRAMEABORTS,
	DOT11_OID_FRAMEABORTSPHY,

	DOT11_OID_SLOTTIME,
	DOT11_OID_CWMIN, /*                 */
	DOT11_OID_CWMAX, /*                 */
	DOT11_OID_ACKWINDOW,
	DOT11_OID_ANTENNARX,
	DOT11_OID_ANTENNATX,
	DOT11_OID_ANTENNADIVERSITY,
	DOT11_OID_CHANNEL,
	DOT11_OID_EDTHRESHOLD,
	DOT11_OID_PREAMBLESETTINGS,
	DOT11_OID_RATES,
	DOT11_OID_CCAMODESUPPORTED,
	DOT11_OID_CCAMODE,
	DOT11_OID_RSSIVECTOR,
	DOT11_OID_OUTPUTPOWERTABLE,
	DOT11_OID_OUTPUTPOWER,
	DOT11_OID_SUPPORTEDRATES,
	DOT11_OID_FREQUENCY,
	DOT11_OID_SUPPORTEDFREQUENCIES,
	DOT11_OID_NOISEFLOOR,
	DOT11_OID_FREQUENCYACTIVITY,
	DOT11_OID_IQCALIBRATIONTABLE,
	DOT11_OID_NONERPPROTECTION,
	DOT11_OID_SLOTSETTINGS,
	DOT11_OID_NONERPTIMEOUT,
	DOT11_OID_PROFILES,
	DOT11_OID_EXTENDEDRATES,

	DOT11_OID_DEAUTHENTICATE,
	DOT11_OID_AUTHENTICATE,
	DOT11_OID_DISASSOCIATE,
	DOT11_OID_ASSOCIATE,
	DOT11_OID_SCAN,
	DOT11_OID_BEACON,
	DOT11_OID_PROBE,
	DOT11_OID_DEAUTHENTICATEEX,
	DOT11_OID_AUTHENTICATEEX,
	DOT11_OID_DISASSOCIATEEX,
	DOT11_OID_ASSOCIATEEX,
	DOT11_OID_REASSOCIATE,
	DOT11_OID_REASSOCIATEEX,

	DOT11_OID_NONERPSTATUS,

	DOT11_OID_STATIMEOUT,
	DOT11_OID_MLMEAUTOLEVEL,
	DOT11_OID_BSSTIMEOUT,
	DOT11_OID_ATTACHMENT,
	DOT11_OID_PSMBUFFER,

	DOT11_OID_BSSS,
	DOT11_OID_BSSX,		/*                                   */
	DOT11_OID_BSSFIND,
	DOT11_OID_BSSLIST,

	OID_INL_TUNNEL,
	OID_INL_MEMADDR,
	OID_INL_MEMORY,
	OID_INL_MODE,
	OID_INL_COMPONENT_NR,
	OID_INL_VERSION,
	OID_INL_INTERFACE_ID,
	OID_INL_COMPONENT_ID,
	OID_INL_CONFIG,
	OID_INL_DOT11D_CONFORMANCE,
	OID_INL_PHYCAPABILITIES,
	OID_INL_OUTPUTPOWER,

	OID_NUM_LAST
};

#define OID_FLAG_CACHED		0x80
#define OID_FLAG_TYPE		0x7f

#define OID_TYPE_U32		0x01
#define OID_TYPE_SSID		0x02
#define OID_TYPE_KEY		0x03
#define OID_TYPE_BUFFER		0x04
#define OID_TYPE_BSS		0x05
#define OID_TYPE_BSSLIST	0x06
#define OID_TYPE_FREQUENCIES	0x07
#define OID_TYPE_MLME		0x08
#define OID_TYPE_MLMEEX		0x09
#define OID_TYPE_ADDR		0x0A
#define OID_TYPE_RAW		0x0B
#define OID_TYPE_ATTACH		0x0C

/*                                                                          
                                                   
 */

struct oid_t {
	enum oid_num_t oid;
	short range;		/*                          */
	short size;		/*                                 */
	char flags;
};

union oid_res_t {
	void *ptr;
	u32 u;
};

#define	IWMAX_BITRATES	20
#define	IWMAX_BSS	24
#define IWMAX_FREQ	30
#define PRIV_STR_SIZE	1024

#endif				/*                      */
/*     */
