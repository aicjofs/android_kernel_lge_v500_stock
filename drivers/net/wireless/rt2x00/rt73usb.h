/*
	Copyright (C) 2004 - 2009 Ivo van Doorn <IvDoorn@gmail.com>
	<http://rt2x00.serialmonkey.com>

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the
	Free Software Foundation, Inc.,
	59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

/*
                
                                                                
                                      
 */

#ifndef RT73USB_H
#define RT73USB_H

/*
                   
 */
#define RF5226				0x0001
#define RF2528				0x0002
#define RF5225				0x0003
#define RF2527				0x0004

/*
                      
                                                          
 */
#define DEFAULT_RSSI_OFFSET		120

/*
                               
 */
#define CSR_REG_BASE			0x3000
#define CSR_REG_SIZE			0x04b0
#define EEPROM_BASE			0x0000
#define EEPROM_SIZE			0x0100
#define BBP_BASE			0x0000
#define BBP_SIZE			0x0080
#define RF_BASE				0x0004
#define RF_SIZE				0x0010

/*
                       
 */
#define NUM_TX_QUEUES			4

/*
                 
 */

/*
                                          
 */
#define MCU_LEDCS_LED_MODE		FIELD16(0x001f)
#define MCU_LEDCS_RADIO_STATUS		FIELD16(0x0020)
#define MCU_LEDCS_LINK_BG_STATUS	FIELD16(0x0040)
#define MCU_LEDCS_LINK_A_STATUS		FIELD16(0x0080)
#define MCU_LEDCS_POLARITY_GPIO_0	FIELD16(0x0100)
#define MCU_LEDCS_POLARITY_GPIO_1	FIELD16(0x0200)
#define MCU_LEDCS_POLARITY_GPIO_2	FIELD16(0x0400)
#define MCU_LEDCS_POLARITY_GPIO_3	FIELD16(0x0800)
#define MCU_LEDCS_POLARITY_GPIO_4	FIELD16(0x1000)
#define MCU_LEDCS_POLARITY_ACT		FIELD16(0x2000)
#define MCU_LEDCS_POLARITY_READY_BG	FIELD16(0x4000)
#define MCU_LEDCS_POLARITY_READY_A	FIELD16(0x8000)

/*
                       
 */
#define FIRMWARE_RT2571			"rt73.bin"
#define FIRMWARE_IMAGE_BASE		0x0800

/*
                             
                                          
                                            
                                              
 */
#define SHARED_KEY_TABLE_BASE		0x1000
#define PAIRWISE_KEY_TABLE_BASE		0x1200
#define PAIRWISE_TA_TABLE_BASE		0x1a00

#define SHARED_KEY_ENTRY(__idx) \
	( SHARED_KEY_TABLE_BASE + \
		((__idx) * sizeof(struct hw_key_entry)) )
#define PAIRWISE_KEY_ENTRY(__idx) \
	( PAIRWISE_KEY_TABLE_BASE + \
		((__idx) * sizeof(struct hw_key_entry)) )
#define PAIRWISE_TA_ENTRY(__idx) \
	( PAIRWISE_TA_TABLE_BASE + \
		((__idx) * sizeof(struct hw_pairwise_ta_entry)) )

struct hw_key_entry {
	u8 key[16];
	u8 tx_mic[8];
	u8 rx_mic[8];
} __packed;

struct hw_pairwise_ta_entry {
	u8 address[6];
	u8 cipher;
	u8 reserved;
} __packed;

/*
                                                  
                                                     
 */
#define HW_DEBUG_SETTING_BASE		0x2bf0

/*
                              
 */
#define HW_BEACON_BASE0			0x2400
#define HW_BEACON_BASE1			0x2500
#define HW_BEACON_BASE2			0x2600
#define HW_BEACON_BASE3			0x2700

#define HW_BEACON_OFFSET(__index) \
	( HW_BEACON_BASE0 + (__index * 0x0100) )

/*
                                     
                                                      
 */

/*
                                  
 */
#define MAC_CSR0			0x3000
#define MAC_CSR0_REVISION		FIELD32(0x0000000f)
#define MAC_CSR0_CHIPSET		FIELD32(0x000ffff0)

/*
                                     
                                                       
                                 
                                                            
 */
#define MAC_CSR1			0x3004
#define MAC_CSR1_SOFT_RESET		FIELD32(0x00000001)
#define MAC_CSR1_BBP_RESET		FIELD32(0x00000002)
#define MAC_CSR1_HOST_READY		FIELD32(0x00000004)

/*
                                
 */
#define MAC_CSR2			0x3008
#define MAC_CSR2_BYTE0			FIELD32(0x000000ff)
#define MAC_CSR2_BYTE1			FIELD32(0x0000ff00)
#define MAC_CSR2_BYTE2			FIELD32(0x00ff0000)
#define MAC_CSR2_BYTE3			FIELD32(0xff000000)

/*
                                
                      
                                                       
                                                    
                                                
                                           
 */
#define MAC_CSR3			0x300c
#define MAC_CSR3_BYTE4			FIELD32(0x000000ff)
#define MAC_CSR3_BYTE5			FIELD32(0x0000ff00)
#define MAC_CSR3_UNICAST_TO_ME_MASK	FIELD32(0x00ff0000)

/*
                              
 */
#define MAC_CSR4			0x3010
#define MAC_CSR4_BYTE0			FIELD32(0x000000ff)
#define MAC_CSR4_BYTE1			FIELD32(0x0000ff00)
#define MAC_CSR4_BYTE2			FIELD32(0x00ff0000)
#define MAC_CSR4_BYTE3			FIELD32(0xff000000)

/*
                              
               
                                                              
                                                             
                                            
                                   
                                             
                                             
                                                 
 */
#define MAC_CSR5			0x3014
#define MAC_CSR5_BYTE4			FIELD32(0x000000ff)
#define MAC_CSR5_BYTE5			FIELD32(0x0000ff00)
#define MAC_CSR5_BSS_ID_MASK		FIELD32(0x00ff0000)

/*
                                           
 */
#define MAC_CSR6			0x3018
#define MAC_CSR6_MAX_FRAME_UNIT		FIELD32(0x00000fff)

/*
                     
 */
#define MAC_CSR7			0x301c

/*
                                
                       
 */
#define MAC_CSR8			0x3020
#define MAC_CSR8_SIFS			FIELD32(0x000000ff)
#define MAC_CSR8_SIFS_AFTER_RX_OFDM	FIELD32(0x0000ff00)
#define MAC_CSR8_EIFS			FIELD32(0xffff0000)

/*
                                       
                                                      
                                                       
                                                          
                                                                     
 */
#define MAC_CSR9			0x3024
#define MAC_CSR9_SLOT_TIME		FIELD32(0x000000ff)
#define MAC_CSR9_CWMIN			FIELD32(0x00000f00)
#define MAC_CSR9_CWMAX			FIELD32(0x0000f000)
#define MAC_CSR9_CW_SELECT		FIELD32(0x00010000)

/*
                                        
 */
#define MAC_CSR10			0x3028

/*
                                                    
                                                             
                                                      
                                 
 */
#define MAC_CSR11			0x302c
#define MAC_CSR11_DELAY_AFTER_TBCN	FIELD32(0x000000ff)
#define MAC_CSR11_TBCN_BEFORE_WAKEUP	FIELD32(0x00007f00)
#define MAC_CSR11_AUTOWAKE		FIELD32(0x00008000)
#define MAC_CSR11_WAKEUP_LATENCY	FIELD32(0x000f0000)

/*
                                                                             
                                   
                                                            
                                                 
 */
#define MAC_CSR12			0x3030
#define MAC_CSR12_CURRENT_STATE		FIELD32(0x00000001)
#define MAC_CSR12_PUT_TO_SLEEP		FIELD32(0x00000002)
#define MAC_CSR12_FORCE_WAKEUP		FIELD32(0x00000004)
#define MAC_CSR12_BBP_CURRENT_STATE	FIELD32(0x00000008)

/*
                   
 */
#define MAC_CSR13			0x3034
#define MAC_CSR13_BIT0			FIELD32(0x00000001)
#define MAC_CSR13_BIT1			FIELD32(0x00000002)
#define MAC_CSR13_BIT2			FIELD32(0x00000004)
#define MAC_CSR13_BIT3			FIELD32(0x00000008)
#define MAC_CSR13_BIT4			FIELD32(0x00000010)
#define MAC_CSR13_BIT5			FIELD32(0x00000020)
#define MAC_CSR13_BIT6			FIELD32(0x00000040)
#define MAC_CSR13_BIT7			FIELD32(0x00000080)
#define MAC_CSR13_BIT8			FIELD32(0x00000100)
#define MAC_CSR13_BIT9			FIELD32(0x00000200)
#define MAC_CSR13_BIT10			FIELD32(0x00000400)
#define MAC_CSR13_BIT11			FIELD32(0x00000800)
#define MAC_CSR13_BIT12			FIELD32(0x00001000)

/*
                                   
                                      
                                        
                                                       
                                  
                                                  
 */
#define MAC_CSR14			0x3038
#define MAC_CSR14_ON_PERIOD		FIELD32(0x000000ff)
#define MAC_CSR14_OFF_PERIOD		FIELD32(0x0000ff00)
#define MAC_CSR14_HW_LED		FIELD32(0x00010000)
#define MAC_CSR14_SW_LED		FIELD32(0x00020000)
#define MAC_CSR14_HW_LED_POLARITY	FIELD32(0x00040000)
#define MAC_CSR14_SW_LED2		FIELD32(0x00080000)

/*
                          
 */
#define MAC_CSR15			0x303c

/*
                          
                                                      
 */

/*
                                           
                             
                                                                   
                                 
                            
                                      
                                    
                                                
                                          
                                                
                                         
                                         
                                           
 */
#define TXRX_CSR0			0x3040
#define TXRX_CSR0_RX_ACK_TIMEOUT	FIELD32(0x000001ff)
#define TXRX_CSR0_TSF_OFFSET		FIELD32(0x00007e00)
#define TXRX_CSR0_AUTO_TX_SEQ		FIELD32(0x00008000)
#define TXRX_CSR0_DISABLE_RX		FIELD32(0x00010000)
#define TXRX_CSR0_DROP_CRC		FIELD32(0x00020000)
#define TXRX_CSR0_DROP_PHYSICAL		FIELD32(0x00040000)
#define TXRX_CSR0_DROP_CONTROL		FIELD32(0x00080000)
#define TXRX_CSR0_DROP_NOT_TO_ME	FIELD32(0x00100000)
#define TXRX_CSR0_DROP_TO_DS		FIELD32(0x00200000)
#define TXRX_CSR0_DROP_VERSION_ERROR	FIELD32(0x00400000)
#define TXRX_CSR0_DROP_MULTICAST	FIELD32(0x00800000)
#define TXRX_CSR0_DROP_BROADCAST	FIELD32(0x01000000)
#define TXRX_CSR0_DROP_ACK_CTS		FIELD32(0x02000000)
#define TXRX_CSR0_TX_WITHOUT_WAITING	FIELD32(0x04000000)

/*
            
 */
#define TXRX_CSR1			0x3044
#define TXRX_CSR1_BBP_ID0		FIELD32(0x0000007f)
#define TXRX_CSR1_BBP_ID0_VALID		FIELD32(0x00000080)
#define TXRX_CSR1_BBP_ID1		FIELD32(0x00007f00)
#define TXRX_CSR1_BBP_ID1_VALID		FIELD32(0x00008000)
#define TXRX_CSR1_BBP_ID2		FIELD32(0x007f0000)
#define TXRX_CSR1_BBP_ID2_VALID		FIELD32(0x00800000)
#define TXRX_CSR1_BBP_ID3		FIELD32(0x7f000000)
#define TXRX_CSR1_BBP_ID3_VALID		FIELD32(0x80000000)

/*
            
 */
#define TXRX_CSR2			0x3048
#define TXRX_CSR2_BBP_ID0		FIELD32(0x0000007f)
#define TXRX_CSR2_BBP_ID0_VALID		FIELD32(0x00000080)
#define TXRX_CSR2_BBP_ID1		FIELD32(0x00007f00)
#define TXRX_CSR2_BBP_ID1_VALID		FIELD32(0x00008000)
#define TXRX_CSR2_BBP_ID2		FIELD32(0x007f0000)
#define TXRX_CSR2_BBP_ID2_VALID		FIELD32(0x00800000)
#define TXRX_CSR2_BBP_ID3		FIELD32(0x7f000000)
#define TXRX_CSR2_BBP_ID3_VALID		FIELD32(0x80000000)

/*
            
 */
#define TXRX_CSR3			0x304c
#define TXRX_CSR3_BBP_ID0		FIELD32(0x0000007f)
#define TXRX_CSR3_BBP_ID0_VALID		FIELD32(0x00000080)
#define TXRX_CSR3_BBP_ID1		FIELD32(0x00007f00)
#define TXRX_CSR3_BBP_ID1_VALID		FIELD32(0x00008000)
#define TXRX_CSR3_BBP_ID2		FIELD32(0x007f0000)
#define TXRX_CSR3_BBP_ID2_VALID		FIELD32(0x00800000)
#define TXRX_CSR3_BBP_ID3		FIELD32(0x7f000000)
#define TXRX_CSR3_BBP_ID3_VALID		FIELD32(0x80000000)

/*
                                               
                                                  
                               
                                                              
                                                                               
 */
#define TXRX_CSR4			0x3050
#define TXRX_CSR4_TX_ACK_TIMEOUT	FIELD32(0x000000ff)
#define TXRX_CSR4_CNTL_ACK_POLICY	FIELD32(0x00000700)
#define TXRX_CSR4_ACK_CTS_PSM		FIELD32(0x00010000)
#define TXRX_CSR4_AUTORESPOND_ENABLE	FIELD32(0x00020000)
#define TXRX_CSR4_AUTORESPOND_PREAMBLE	FIELD32(0x00040000)
#define TXRX_CSR4_OFDM_TX_RATE_DOWN	FIELD32(0x00080000)
#define TXRX_CSR4_OFDM_TX_RATE_STEP	FIELD32(0x00300000)
#define TXRX_CSR4_OFDM_TX_FALLBACK_CCK	FIELD32(0x00400000)
#define TXRX_CSR4_LONG_RETRY_LIMIT	FIELD32(0x0f000000)
#define TXRX_CSR4_SHORT_RETRY_LIMIT	FIELD32(0xf0000000)

/*
            
 */
#define TXRX_CSR5			0x3054

/*
                                           
 */
#define TXRX_CSR6			0x3058

/*
                                                                    
 */
#define TXRX_CSR7			0x305c
#define TXRX_CSR7_ACK_CTS_6MBS		FIELD32(0x000000ff)
#define TXRX_CSR7_ACK_CTS_9MBS		FIELD32(0x0000ff00)
#define TXRX_CSR7_ACK_CTS_12MBS		FIELD32(0x00ff0000)
#define TXRX_CSR7_ACK_CTS_18MBS		FIELD32(0xff000000)

/*
                                                                      
 */
#define TXRX_CSR8			0x3060
#define TXRX_CSR8_ACK_CTS_24MBS		FIELD32(0x000000ff)
#define TXRX_CSR8_ACK_CTS_36MBS		FIELD32(0x0000ff00)
#define TXRX_CSR8_ACK_CTS_48MBS		FIELD32(0x00ff0000)
#define TXRX_CSR8_ACK_CTS_54MBS		FIELD32(0xff000000)

/*
                                               
                                       
                                         
                                                                   
                                       
 */
#define TXRX_CSR9			0x3064
#define TXRX_CSR9_BEACON_INTERVAL	FIELD32(0x0000ffff)
#define TXRX_CSR9_TSF_TICKING		FIELD32(0x00010000)
#define TXRX_CSR9_TSF_SYNC		FIELD32(0x00060000)
#define TXRX_CSR9_TBTT_ENABLE		FIELD32(0x00080000)
#define TXRX_CSR9_BEACON_GEN		FIELD32(0x00100000)
#define TXRX_CSR9_TIMESTAMP_COMPENSATE	FIELD32(0xff000000)

/*
                                
 */
#define TXRX_CSR10			0x3068

/*
                        
 */
#define TXRX_CSR11			0x306c

/*
                          
 */
#define TXRX_CSR12			0x3070
#define TXRX_CSR12_LOW_TSFTIMER		FIELD32(0xffffffff)

/*
                           
 */
#define TXRX_CSR13			0x3074
#define TXRX_CSR13_HIGH_TSFTIMER	FIELD32(0xffffffff)

/*
                          
 */
#define TXRX_CSR14			0x3078

/*
                                                 
 */
#define TXRX_CSR15			0x307c

/*
                         
                                                      
 */

/*
                           
 */
#define PHY_CSR0			0x3080
#define PHY_CSR0_PA_PE_BG		FIELD32(0x00010000)
#define PHY_CSR0_PA_PE_A		FIELD32(0x00020000)

/*
           
 */
#define PHY_CSR1			0x3084
#define PHY_CSR1_RF_RPI			FIELD32(0x00010000)

/*
                                
 */
#define PHY_CSR2			0x3088

/*
                                         
                                             
                                  
                                           
                                                 
 */
#define PHY_CSR3			0x308c
#define PHY_CSR3_VALUE			FIELD32(0x000000ff)
#define PHY_CSR3_REGNUM			FIELD32(0x00007f00)
#define PHY_CSR3_READ_CONTROL		FIELD32(0x00008000)
#define PHY_CSR3_BUSY			FIELD32(0x00010000)

/*
                                       
                                                                        
                                                                     
                                                               
                            
                                                
 */
#define PHY_CSR4			0x3090
#define PHY_CSR4_VALUE			FIELD32(0x00ffffff)
#define PHY_CSR4_NUMBER_OF_BITS		FIELD32(0x1f000000)
#define PHY_CSR4_IF_SELECT		FIELD32(0x20000000)
#define PHY_CSR4_PLL_LD			FIELD32(0x40000000)
#define PHY_CSR4_BUSY			FIELD32(0x80000000)

/*
                                                   
 */
#define PHY_CSR5			0x3094
#define PHY_CSR5_IQ_FLIP		FIELD32(0x00000004)

/*
                                            
 */
#define PHY_CSR6			0x3098
#define PHY_CSR6_IQ_FLIP		FIELD32(0x00000004)

/*
                                             
 */
#define PHY_CSR7			0x309c

/*
                             
 */

/*
                                      
 */
#define SEC_CSR0			0x30a0
#define SEC_CSR0_BSS0_KEY0_VALID	FIELD32(0x00000001)
#define SEC_CSR0_BSS0_KEY1_VALID	FIELD32(0x00000002)
#define SEC_CSR0_BSS0_KEY2_VALID	FIELD32(0x00000004)
#define SEC_CSR0_BSS0_KEY3_VALID	FIELD32(0x00000008)
#define SEC_CSR0_BSS1_KEY0_VALID	FIELD32(0x00000010)
#define SEC_CSR0_BSS1_KEY1_VALID	FIELD32(0x00000020)
#define SEC_CSR0_BSS1_KEY2_VALID	FIELD32(0x00000040)
#define SEC_CSR0_BSS1_KEY3_VALID	FIELD32(0x00000080)
#define SEC_CSR0_BSS2_KEY0_VALID	FIELD32(0x00000100)
#define SEC_CSR0_BSS2_KEY1_VALID	FIELD32(0x00000200)
#define SEC_CSR0_BSS2_KEY2_VALID	FIELD32(0x00000400)
#define SEC_CSR0_BSS2_KEY3_VALID	FIELD32(0x00000800)
#define SEC_CSR0_BSS3_KEY0_VALID	FIELD32(0x00001000)
#define SEC_CSR0_BSS3_KEY1_VALID	FIELD32(0x00002000)
#define SEC_CSR0_BSS3_KEY2_VALID	FIELD32(0x00004000)
#define SEC_CSR0_BSS3_KEY3_VALID	FIELD32(0x00008000)

/*
                                                     
 */
#define SEC_CSR1			0x30a4
#define SEC_CSR1_BSS0_KEY0_CIPHER_ALG	FIELD32(0x00000007)
#define SEC_CSR1_BSS0_KEY1_CIPHER_ALG	FIELD32(0x00000070)
#define SEC_CSR1_BSS0_KEY2_CIPHER_ALG	FIELD32(0x00000700)
#define SEC_CSR1_BSS0_KEY3_CIPHER_ALG	FIELD32(0x00007000)
#define SEC_CSR1_BSS1_KEY0_CIPHER_ALG	FIELD32(0x00070000)
#define SEC_CSR1_BSS1_KEY1_CIPHER_ALG	FIELD32(0x00700000)
#define SEC_CSR1_BSS1_KEY2_CIPHER_ALG	FIELD32(0x07000000)
#define SEC_CSR1_BSS1_KEY3_CIPHER_ALG	FIELD32(0x70000000)

/*
                                             
                                               
                                               
 */
#define SEC_CSR2			0x30a8
#define SEC_CSR3			0x30ac

/*
                                               
 */
#define SEC_CSR4			0x30b0
#define SEC_CSR4_ENABLE_BSS0		FIELD32(0x00000001)
#define SEC_CSR4_ENABLE_BSS1		FIELD32(0x00000002)
#define SEC_CSR4_ENABLE_BSS2		FIELD32(0x00000004)
#define SEC_CSR4_ENABLE_BSS3		FIELD32(0x00000008)

/*
                                                     
 */
#define SEC_CSR5			0x30b4
#define SEC_CSR5_BSS2_KEY0_CIPHER_ALG	FIELD32(0x00000007)
#define SEC_CSR5_BSS2_KEY1_CIPHER_ALG	FIELD32(0x00000070)
#define SEC_CSR5_BSS2_KEY2_CIPHER_ALG	FIELD32(0x00000700)
#define SEC_CSR5_BSS2_KEY3_CIPHER_ALG	FIELD32(0x00007000)
#define SEC_CSR5_BSS3_KEY0_CIPHER_ALG	FIELD32(0x00070000)
#define SEC_CSR5_BSS3_KEY1_CIPHER_ALG	FIELD32(0x00700000)
#define SEC_CSR5_BSS3_KEY2_CIPHER_ALG	FIELD32(0x07000000)
#define SEC_CSR5_BSS3_KEY3_CIPHER_ALG	FIELD32(0x70000000)

/*
                         
 */

/*
                                                      
 */
#define STA_CSR0			0x30c0
#define STA_CSR0_FCS_ERROR		FIELD32(0x0000ffff)
#define STA_CSR0_PLCP_ERROR		FIELD32(0xffff0000)

/*
                                                      
 */
#define STA_CSR1			0x30c4
#define STA_CSR1_PHYSICAL_ERROR		FIELD32(0x0000ffff)
#define STA_CSR1_FALSE_CCA_ERROR	FIELD32(0xffff0000)

/*
                                                        
 */
#define STA_CSR2			0x30c8
#define STA_CSR2_RX_FIFO_OVERFLOW_COUNT	FIELD32(0x0000ffff)
#define STA_CSR2_RX_OVERFLOW_COUNT	FIELD32(0xffff0000)

/*
                             
 */
#define STA_CSR3			0x30cc
#define STA_CSR3_TX_BEACON_COUNT	FIELD32(0x0000ffff)

/*
                            
 */
#define STA_CSR4			0x30d0
#define STA_CSR4_TX_NO_RETRY_COUNT	FIELD32(0x0000ffff)
#define STA_CSR4_TX_ONE_RETRY_COUNT	FIELD32(0xffff0000)

/*
                            
 */
#define STA_CSR5			0x30d4
#define STA_CSR4_TX_MULTI_RETRY_COUNT	FIELD32(0x0000ffff)
#define STA_CSR4_TX_RETRY_FAIL_COUNT	FIELD32(0xffff0000)

/*
                         
 */

/*
                                              
 */
#define QOS_CSR1			0x30e4
#define QOS_CSR1_BYTE4			FIELD32(0x000000ff)
#define QOS_CSR1_BYTE5			FIELD32(0x0000ff00)

/*
                                          
 */
#define QOS_CSR2			0x30e8

/*
                                      
                                         
                                         
 */
#define QOS_CSR3			0x30ec
#define QOS_CSR4			0x30f0

/*
                                                     
 */
#define QOS_CSR5			0x30f4

/*
                         
 */

/*
                                     
                     
                     
                     
                     
 */
#define AIFSN_CSR			0x0400
#define AIFSN_CSR_AIFSN0		FIELD32(0x0000000f)
#define AIFSN_CSR_AIFSN1		FIELD32(0x000000f0)
#define AIFSN_CSR_AIFSN2		FIELD32(0x00000f00)
#define AIFSN_CSR_AIFSN3		FIELD32(0x0000f000)

/*
                                     
                     
                     
                     
                     
 */
#define CWMIN_CSR			0x0404
#define CWMIN_CSR_CWMIN0		FIELD32(0x0000000f)
#define CWMIN_CSR_CWMIN1		FIELD32(0x000000f0)
#define CWMIN_CSR_CWMIN2		FIELD32(0x00000f00)
#define CWMIN_CSR_CWMIN3		FIELD32(0x0000f000)

/*
                                     
                     
                     
                     
                     
 */
#define CWMAX_CSR			0x0408
#define CWMAX_CSR_CWMAX0		FIELD32(0x0000000f)
#define CWMAX_CSR_CWMAX1		FIELD32(0x000000f0)
#define CWMAX_CSR_CWMAX2		FIELD32(0x00000f00)
#define CWMAX_CSR_CWMAX3		FIELD32(0x0000f000)

/*
                                           
                                         
                                         
 */
#define AC_TXOP_CSR0			0x040c
#define AC_TXOP_CSR0_AC0_TX_OP		FIELD32(0x0000ffff)
#define AC_TXOP_CSR0_AC1_TX_OP		FIELD32(0xffff0000)

/*
                                           
                                         
                                         
 */
#define AC_TXOP_CSR1			0x0410
#define AC_TXOP_CSR1_AC2_TX_OP		FIELD32(0x0000ffff)
#define AC_TXOP_CSR1_AC3_TX_OP		FIELD32(0xffff0000)

/*
                 
                                     
 */

/*
     
 */
#define BBP_R2_BG_MODE			FIELD8(0x20)

/*
     
 */
#define BBP_R3_SMART_MODE		FIELD8(0x01)

/*
                         
                                                                          
 */

/*
                                       
                                                                     
                           
 */
#define BBP_R4_RX_ANTENNA_CONTROL	FIELD8(0x03)
#define BBP_R4_RX_FRAME_END		FIELD8(0x20)

/*
      
 */
#define BBP_R77_RX_ANTENNA		FIELD8(0x03)

/*
               
 */

/*
       
 */
#define RF3_TXPOWER			FIELD32(0x00003e00)

/*
       
 */
#define RF4_FREQ_OFFSET			FIELD32(0x0003f000)

/*
                  
                                         
 */

/*
                  
 */
#define EEPROM_MAC_ADDR_0		0x0002
#define EEPROM_MAC_ADDR_BYTE0		FIELD16(0x00ff)
#define EEPROM_MAC_ADDR_BYTE1		FIELD16(0xff00)
#define EEPROM_MAC_ADDR1		0x0003
#define EEPROM_MAC_ADDR_BYTE2		FIELD16(0x00ff)
#define EEPROM_MAC_ADDR_BYTE3		FIELD16(0xff00)
#define EEPROM_MAC_ADDR_2		0x0004
#define EEPROM_MAC_ADDR_BYTE4		FIELD16(0x00ff)
#define EEPROM_MAC_ADDR_BYTE5		FIELD16(0xff00)

/*
                  
                                   
                                                        
                                                        
                                                                      
                                     
                                                            
                                    
 */
#define EEPROM_ANTENNA			0x0010
#define EEPROM_ANTENNA_NUM		FIELD16(0x0003)
#define EEPROM_ANTENNA_TX_DEFAULT	FIELD16(0x000c)
#define EEPROM_ANTENNA_RX_DEFAULT	FIELD16(0x0030)
#define EEPROM_ANTENNA_FRAME_TYPE	FIELD16(0x0040)
#define EEPROM_ANTENNA_DYN_TXAGC	FIELD16(0x0200)
#define EEPROM_ANTENNA_HARDWARE_RADIO	FIELD16(0x0400)
#define EEPROM_ANTENNA_RF_TYPE		FIELD16(0xf800)

/*
                     
                              
 */
#define EEPROM_NIC			0x0011
#define EEPROM_NIC_EXTERNAL_LNA		FIELD16(0x0010)

/*
                    
                                                    
                                     
 */
#define EEPROM_GEOGRAPHY		0x0012
#define EEPROM_GEOGRAPHY_GEO_A		FIELD16(0x00ff)
#define EEPROM_GEOGRAPHY_GEO		FIELD16(0xff00)

/*
              
 */
#define EEPROM_BBP_START		0x0013
#define EEPROM_BBP_SIZE			16
#define EEPROM_BBP_VALUE		FIELD16(0x00ff)
#define EEPROM_BBP_REG_ID		FIELD16(0xff00)

/*
                         
 */
#define EEPROM_TXPOWER_G_START		0x0023
#define EEPROM_TXPOWER_G_SIZE		7
#define EEPROM_TXPOWER_G_1		FIELD16(0x00ff)
#define EEPROM_TXPOWER_G_2		FIELD16(0xff00)

/*
                   
 */
#define EEPROM_FREQ			0x002f
#define EEPROM_FREQ_OFFSET		FIELD16(0x00ff)
#define EEPROM_FREQ_SEQ_MASK		FIELD16(0xff00)
#define EEPROM_FREQ_SEQ			FIELD16(0x0300)

/*
              
                                          
                                          
                                      
                                           
                                           
                                           
                                           
                                           
                      
 */
#define EEPROM_LED			0x0030
#define EEPROM_LED_POLARITY_RDY_G	FIELD16(0x0001)
#define EEPROM_LED_POLARITY_RDY_A	FIELD16(0x0002)
#define EEPROM_LED_POLARITY_ACT		FIELD16(0x0004)
#define EEPROM_LED_POLARITY_GPIO_0	FIELD16(0x0008)
#define EEPROM_LED_POLARITY_GPIO_1	FIELD16(0x0010)
#define EEPROM_LED_POLARITY_GPIO_2	FIELD16(0x0020)
#define EEPROM_LED_POLARITY_GPIO_3	FIELD16(0x0040)
#define EEPROM_LED_POLARITY_GPIO_4	FIELD16(0x0080)
#define EEPROM_LED_LED_MODE		FIELD16(0x1f00)

/*
                         
 */
#define EEPROM_TXPOWER_A_START		0x0031
#define EEPROM_TXPOWER_A_SIZE		12
#define EEPROM_TXPOWER_A_1		FIELD16(0x00ff)
#define EEPROM_TXPOWER_A_2		FIELD16(0xff00)

/*
                              
 */
#define EEPROM_RSSI_OFFSET_BG		0x004d
#define EEPROM_RSSI_OFFSET_BG_1		FIELD16(0x00ff)
#define EEPROM_RSSI_OFFSET_BG_2		FIELD16(0xff00)

/*
                             
 */
#define EEPROM_RSSI_OFFSET_A		0x004e
#define EEPROM_RSSI_OFFSET_A_1		FIELD16(0x00ff)
#define EEPROM_RSSI_OFFSET_A_2		FIELD16(0xff00)

/*
                          
 */
#define TXD_DESC_SIZE			( 6 * sizeof(__le32) )
#define TXINFO_SIZE			( 6 * sizeof(__le32) )
#define RXD_DESC_SIZE			( 6 * sizeof(__le32) )

/*
                                                     
 */

/*
        
                                                   
                                                   
                                                
             
                                              
                                    
                                    
                                     
                                      
                                                                  
 */
#define TXD_W0_BURST			FIELD32(0x00000001)
#define TXD_W0_VALID			FIELD32(0x00000002)
#define TXD_W0_MORE_FRAG		FIELD32(0x00000004)
#define TXD_W0_ACK			FIELD32(0x00000008)
#define TXD_W0_TIMESTAMP		FIELD32(0x00000010)
#define TXD_W0_OFDM			FIELD32(0x00000020)
#define TXD_W0_IFS			FIELD32(0x00000040)
#define TXD_W0_RETRY_MODE		FIELD32(0x00000080)
#define TXD_W0_TKIP_MIC			FIELD32(0x00000100)
#define TXD_W0_KEY_TABLE		FIELD32(0x00000200)
#define TXD_W0_KEY_INDEX		FIELD32(0x0000fc00)
#define TXD_W0_DATABYTE_COUNT		FIELD32(0x0fff0000)
#define TXD_W0_BURST2			FIELD32(0x10000000)
#define TXD_W0_CIPHER_ALG		FIELD32(0xe0000000)

/*
        
                                 
                                                         
                                               
 */
#define TXD_W1_HOST_Q_ID		FIELD32(0x0000000f)
#define TXD_W1_AIFSN			FIELD32(0x000000f0)
#define TXD_W1_CWMIN			FIELD32(0x00000f00)
#define TXD_W1_CWMAX			FIELD32(0x0000f000)
#define TXD_W1_IV_OFFSET		FIELD32(0x003f0000)
#define TXD_W1_HW_SEQUENCE		FIELD32(0x10000000)
#define TXD_W1_BUFFER_COUNT		FIELD32(0xe0000000)

/*
                          
 */
#define TXD_W2_PLCP_SIGNAL		FIELD32(0x000000ff)
#define TXD_W2_PLCP_SERVICE		FIELD32(0x0000ff00)
#define TXD_W2_PLCP_LENGTH_LOW		FIELD32(0x00ff0000)
#define TXD_W2_PLCP_LENGTH_HIGH		FIELD32(0xff000000)

/*
        
 */
#define TXD_W3_IV			FIELD32(0xffffffff)

/*
        
 */
#define TXD_W4_EIV			FIELD32(0xffffffff)

/*
        
                                                                            
                                                                            
                                                  
                                          
 */
#define TXD_W5_FRAME_OFFSET		FIELD32(0x000000ff)
#define TXD_W5_PACKET_ID		FIELD32(0x0000ff00)
#define TXD_W5_TX_POWER			FIELD32(0x00ff0000)
#define TXD_W5_WAITING_DMA_DONE_INT	FIELD32(0x01000000)

/*
                                    
 */

/*
        
                                                         
                                           
 */
#define RXD_W0_OWNER_NIC		FIELD32(0x00000001)
#define RXD_W0_DROP			FIELD32(0x00000002)
#define RXD_W0_UNICAST_TO_ME		FIELD32(0x00000004)
#define RXD_W0_MULTICAST		FIELD32(0x00000008)
#define RXD_W0_BROADCAST		FIELD32(0x00000010)
#define RXD_W0_MY_BSS			FIELD32(0x00000020)
#define RXD_W0_CRC_ERROR		FIELD32(0x00000040)
#define RXD_W0_OFDM			FIELD32(0x00000080)
#define RXD_W0_CIPHER_ERROR		FIELD32(0x00000300)
#define RXD_W0_KEY_INDEX		FIELD32(0x0000fc00)
#define RXD_W0_DATABYTE_COUNT		FIELD32(0x0fff0000)
#define RXD_W0_CIPHER_ALG		FIELD32(0xe0000000)

/*
        
                                            
                              
 */
#define RXD_W1_SIGNAL			FIELD32(0x000000ff)
#define RXD_W1_RSSI_AGC			FIELD32(0x00001f00)
#define RXD_W1_RSSI_LNA			FIELD32(0x00006000)
#define RXD_W1_FRAME_OFFSET		FIELD32(0x7f000000)

/*
        
                                           
 */
#define RXD_W2_IV			FIELD32(0xffffffff)

/*
        
                                             
 */
#define RXD_W3_EIV			FIELD32(0xffffffff)

/*
        
                                             
                                                               
 */
#define RXD_W4_ICV			FIELD32(0xffffffff)

/*
                                                                       
                                 
                                                              
                                      
 */

/*
        
 */
#define RXD_W5_RESERVED			FIELD32(0xffffffff)

/*
                                                              
                                             
 */
#define MIN_TXPOWER	0
#define MAX_TXPOWER	31
#define DEFAULT_TXPOWER	24

#define TXPOWER_FROM_DEV(__txpower) \
	(((u8)(__txpower)) > MAX_TXPOWER) ? DEFAULT_TXPOWER : (__txpower)

#define TXPOWER_TO_DEV(__txpower) \
	clamp_t(char, __txpower, MIN_TXPOWER, MAX_TXPOWER)

#endif /*           */
