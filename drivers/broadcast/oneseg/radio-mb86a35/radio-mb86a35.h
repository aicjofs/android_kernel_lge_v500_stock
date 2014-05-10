/* 
                      
                                                   
                                                                            
*/
/* COPYRIGHT FUJITSU SEMICONDUCTOR LIMITED 2011 */

#ifndef	__RADIO_MB86A35_H__
#define	__RADIO_MB86A35_H__

#ifndef __KERNEL__
typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
#endif

#ifndef	__RADIO_MB86A35_DEV_H__
#include <string.h>
#endif

#define	NODE_BASENAME		"/dev/radio"	/*                  */
#define	NODE_PATHNAME		NODE_BASENAME "0"	/*           */

/*                       */
/*
                                              
                       
                                           
                                                  
                                                                
 */

/*                                                                                                      */
/*                                                                                                      */
/*                                                                                                      */

enum ioctl_command_no {
	IOCTL_RST_SOFT 			= 0x0102,	/*                                   */
	IOCTL_RST_SYNC 			= 0x0104,	/*                                       */

	IOCTL_SET_RECVM 		= 0x0201,	/*                                 */
	IOCTL_SET_SPECT,				/*                                          */
	IOCTL_SET_IQSET 		= 0x0204,	/*                              */
	IOCTL_SET_ALOG_PDOWN 		= 0x0208,	/*                                                     */
	IOCTL_SET_SCHANNEL 		= 0x0210,	/*                                */

	IOCTL_AGC 			= 0x0300,	/*                                           */
	IOCTL_OFDM_INIT 		= 0x0400,	/*                                 */
	
	IOCTL_GPIO_SETUP 		= 0x0600,	/*                                         */
	IOCTL_GPIO_READ,				/*                                        */
	IOCTL_GPIO_WRITE,				/*                                         */
	IOCTL_ANALOG,					/*                              */

	IOCTL_SEQ_GETSTAT 		= 0x0701,	/*                                */
	IOCTL_SEQ_SETMODE,				/*                               */
	IOCTL_SEQ_GETMODE,				/*                               */
	IOCTL_SEQ_GETTMCC 		= 0x0708,	/*                                         */
	
	IOCTL_BER_MONISTAT 		= 0x1000,	/*                                                           */
	IOCTL_BER_MONICONFIG,				/*                                       */
	IOCTL_BER_MONISTART 		= 0x1010,	/*                                  */
	IOCTL_BER_MONIGET 		= 0x1020,	/*                                           */
	IOCTL_BER_MONISTOP 		= 0x10F0,	/*                                 */

	IOCTL_TS_START 			= 0x1101,	/*                                    */
	IOCTL_TS_STOP,					/*                                   */
	IOCTL_TS_CONFIG,				/*                                            */
	IOCTL_TS_PCLOCK 		= 0x1110,	/*                                                            */
	IOCTL_TS_OUTMASK,				/*                                     */
	IOCTL_TS_INVERT,				/*                                  */

	IOCTL_IRQ_GETREASON 		= 0x9000,	/*                            */
	IOCTL_IRQ_SETMASK,				/*                                */
	IOCTL_IRQ_TMCCPARAM_SET,			/*                                        */
	IOCTL_IRQ_TMCCPARAM_REASON,			/*                                             */

//                                                                
	IOCTL_CN_MONI_CONFIG_START 	= 0x2010,	/*                                              */
	IOCTL_CN_MONI_GET 		= 0x2100,	/*                                         */

//                                                                 
	IOCTL_MER_MONI_CONFIG_START 	= 0x4010,	/*                                              */
	IOCTL_MER_MONI_GET 		= 0x4100,	/*                                         */

	IOCTL_CH_SEARCH 		= 0x6000,	/*                               */

	IOCTL_RF_INIT 			= 0x0801,	/*                                        */
	IOCTL_RF_FUSEVALUE,				/*                     */
	IOCTL_RF_CALIB_DCOFF 		= 0x0804,	/*                        */
	IOCTL_RF_CALIB_CTUNE 		= 0x0808,	/*                     */
	IOCTL_RF_CHANNEL 		= 0x0810,	/*                            */
	IOCTL_RF_PDOWN 			= 0x0811,	/*                       */
	IOCTL_RF_RSSIMONI 		= 0x0812,	/*                      */

	IOCTL_POWERCTRL 		= 0xA000,	/*                  */

        IOCTL_I2C_MAIN 			= 0xF000,       /*                         */
        IOCTL_I2C_SUB,         	 		        /*                         */
        IOCTL_I2C_RF,        			        /*                          */
	IOCTL_HRM, 					/*                        */
	IOCTL_LOW_UP_IF,				/*                                         */

	IOCTL_SPI_MAIN 			= 0xF100,	/*                         */
	IOCTL_SPI_SUB,					/*                         */
	IOCTL_SPI_OFDM,					/*                              */
	IOCTL_SPI_OFDM_SUB,				/*                             */
	IOCTL_SPI_RF,					/*                          */

	IOCTL_STREAM_READ 		= 0xF200,	/*                          */	
	IOCTL_STREAM_READ_CTRL,				/*                        */
	IOCTL_SPI_CONFIG,				/*                       */
	IOCTL_TS_SETUP,					/*                */
	IOCTL_SELECT_ANTENNA, 				/*                       */
	
};

/*                                                                                                      */
/*                                                                                                      */
/*                                                                                                      */

							/*      */
							//                        
#define		PARAM_MODE_DETECT_ON		0x00
#define		PARAM_MODE_DETECT_OFF		0x01

#define		PARAM_MODE_ISDBT_13UHF		0x4D	/*                         */
#define		PARAM_MODE_ISDBT_1UHF		0x41	/*                         */
#define		PARAM_MODE_ISDBTMM_13VHF	0x8D	/*                         */
#define		PARAM_MODE_ISDBTMM_1VHF		0x81	/*                         */
#define		PARAM_MODE_ISDBTSB_1VHF		0xC1	/*                         */
#define		PARAM_MODE_ISDBTSB_3VHF		0xC3	/*                         */

							/*           */
#define		PARAM_RESET_ON			0x00	/*                    */
#define		PARAM_RESET_OFF			0xFF	/*                          */

							/*           */
#define		PARAM_STATE_INIT_ON		0x01	/*                            */
#define		PARAM_STATE_INIT_OFF		0x00	/*                                  */

							/*             */
#define		PARAM_SEG_BAND_DVBT7M		0x00	/*                  */
#define		PARAM_SEG_BAND_DVBT8M		0x20	/*                  */
#define		PARAM_SEG_BAND_DVBT6M		0x60	/*                  */
#define		PARAM_SEG_BAND_DVBT5M		0x80	/*                  */
#define		PARAM_SEG_BAND_ISDB6M		0x00	/*                      */
#define		PARAM_SEG_BAND_ISDB7M		0x20	/*                      */
#define		PARAM_SEG_BAND_ISDB8M		0x40	/*                      */

							/*            */
#define		PARAM_SEG_TMM_32		0x00	/*           */
#define		PARAM_SEG_TMM_16		0x04	/*           */

							/*               */
#define		PARAM_SEG_MYT_ISDB13S		0x00	/*                     */
#define		PARAM_SEG_MYT_DVBT		0x01	/*           */
#define		PARAM_SEG_MYT_ISDB1S		0x02	/*                        */
#define		PARAM_SEG_MYT_ISDB3S		0x03	/*                  */

							/*              */
#define		PARAM_LAYERSEL_A		0x00	/*             */
#define		PARAM_LAYERSEL_B		0x01	/*             */
#define		PARAM_LAYERSEL_C		0x02	/*             */

							/*           */
#define		PARAM_IQINV_IQINV_NORM		0x00	/*               */
#define		PARAM_IQINV_IQINV_INVT		0x04	/*            */

							/*           */
#define		PARAM_CONT_CTRL3_IQSEL_IF	0x00	/*              */
#define		PARAM_CONT_CTRL3_IQSEL_IQ	0x02	/*              */

							/*           */
#define		PARAM_CONT_CTRL3_IFSEL_16	0x00	/*           */
#define		PARAM_CONT_CTRL3_IFSEL_32	0x01	/*           */

							/*             */
#define		PARAM_MACRO_PDOWN_DACPWDN_OFF	0x00	/*                   */
#define		PARAM_MACRO_PDOWN_DACPWDN_ON	0x20	/*                */

#define		PARAM_FTSEGCNT_SEGCNT_ISDB_T	0xFF	/*            */

							/*              */
#define		PARAM_GPIO_DAT_AC_MODE_ACCLK	0x00	/*                    */
#define		PARAM_GPIO_DAT_AC_MODE_ACDT	0x40	/*                   */

							/*                 */
#define		PARAM_GPIO_OUTSEL_IRQOUT_3	0x80	/*                */
#define		PARAM_GPIO_OUTSEL_IRQOUT_2	0x40	/*                */
#define		PARAM_GPIO_OUTSEL_IRQOUT_1	0x20	/*                */
#define		PARAM_GPIO_OUTSEL_IRQOUT_0	0x10	/*                */

							/*                */
#define		PARAM_GPIO_OUTSEL_OUTEN_3	0x08	/*                  */
#define		PARAM_GPIO_OUTSEL_OUTEN_2	0x04	/*                  */
#define		PARAM_GPIO_OUTSEL_OUTEN_1	0x02	/*                  */
#define		PARAM_GPIO_OUTSEL_OUTEN_0	0x01	/*                  */

							/*               */
#define		PARAM_TUNER_IRQ_NONE		0x00	/*                   */
#define		PARAM_TUNER_IRQ_GPIO		0x02	/*                   */
#define		PARAM_TUNER_IRQ_CHEND		0x01	/*                         */
#define		PARAM_TUNER_IRQ_GPIO_CHEND	0x03	/*                                */

							/*            */
#define		PARAM_S8WAIT_TSWAIT_S8		0x00	/*                       */
#define		PARAM_S8WAIT_TSWAIT_S9		0x80	/*                       */
							/*             */
#define		PARAM_S8WAIT_BERWAIT_S8		0x00	/*                            */
#define		PARAM_S8WAIT_BERWAIT_S9		0x40	/*                            */
#define		PARAM_S8WAIT_TS8_BER8		0x00	/*                               */
#define		PARAM_S8WAIT_TS9_BER8		0x80	/*                               */
#define		PARAM_S8WAIT_TS8_BER9		0x40	/*                               */
#define		PARAM_S8WAIT_TS9_BER9		0xC0	/*                               */

							/*                */
#define		PARAM_STATE_INIT_NOR		0x00	/*                  */
#define		PARAM_STATE_INIT_INIT		0x01	/*                  */

							/*                */
#define		PARAM_SYNC_STATE_INIT		0x00	/*                  */
#define		PARAM_SYNC_STATE_INITTMR	0x01	/*                  */
#define		PARAM_SYNC_STATE_DETCTMODE	0x02	/*                     */
#define		PARAM_SYNC_STATE_FIXEDMODE	0x03	/*                */
#define		PARAM_SYNC_STATE_AFCMODE	0x04	/*                    */
#define		PARAM_SYNC_STATE_FFTSTART	0x05	/*                                */
#define		PARAM_SYNC_STATE_FRMSYNCSRH	0x06	/*                      */
#define		PARAM_SYNC_STATE_FRMSYNC	0x07	/*                */
#define		PARAM_SYNC_STATE_TMCCERRC	0x08	/*                         */
#define		PARAM_SYNC_STATE_TSOUT		0x09	/*                    */

							/*                */
#define		PARAM_MODED_CTRL_M3G32		0x80	/*           */
#define		PARAM_MODED_CTRL_M3G16		0x40	/*           */
#define		PARAM_MODED_CTRL_M3G08		0x20	/*           */
#define		PARAM_MODED_CTRL_M3G04		0x10	/*           */
#define		PARAM_MODED_CTRL_M2G32		0x08	/*           */
#define		PARAM_MODED_CTRL_M2G16		0x04	/*           */
#define		PARAM_MODED_CTRL_M2G08		0x02	/*           */
#define		PARAM_MODED_CTRL_M2G04		0x01	/*           */

							/*                 */
#define		PARAM_MODED_CTRL_M1G32		0x08	/*           */
#define		PARAM_MODED_CTRL_M1G16		0x04	/*           */
#define		PARAM_MODED_CTRL_M1G08		0x02	/*           */
#define		PARAM_MODED_CTRL_M1G04		0x01	/*           */

							/*                */
#define		PARAM_MODED_STAT_MDFAIL		0x40	/*            */
#define		PARAM_MODED_STAT_MDDETECT	0x20	/*              */
#define		PARAM_MODED_STAT_MDFIX		0x10	/*           */

							/*           */
#define		PARAM_MODED_STAT_MODE1		0x00	/*           */
#define		PARAM_MODED_STAT_MODE2		0x04	/*           */
#define		PARAM_MODED_STAT_MODE3		0x08	/*           */

							/*            */
#define		PARAM_MODED_STAT_GUARDE14	0x00	/*                */
#define		PARAM_MODED_STAT_GUARDE18	0x01	/*                */
#define		PARAM_MODED_STAT_GUARDE116	0x02	/*                 */
#define		PARAM_MODED_STAT_GUARDE132	0x03	/*                 */

							/*              */
#define		PARAM_TMCCREAD_TMCCLOCK_AUTO	0x00	/*                 */
#define		PARAM_TMCCREAD_TMCCLOCK_NOAUTO	0x01	/*                */

							/*                            */
#define		PARAM_TMCC_CORRECT_INV_NOW	0x00	/*                            */
#define		PARAM_TMCC_CORRECT_VLD_NOW	0x02	/*                            */
#define		PARAM_TMCC_CORRECT_INV_DONE	0x01	/*                            */
#define		PARAM_TMCC_CORRECT_VLD_DONE	0x03	/*                            */

							/*            */
#define		PARAM_VBERON_BEFORE		0x01	/*            */

							/*               */
#define		PARAM_VBERXRST_VBERXRSTC_C	0x00	/*                         */
#define		PARAM_VBERXRST_VBERXRSTC_E	0x04	/*                    */
							/*               */
#define		PARAM_VBERXRST_VBERXRSTB_C	0x00	/*                         */
#define		PARAM_VBERXRST_VBERXRSTB_E	0x02	/*                    */
							/*               */
#define		PARAM_VBERXRST_VBERXRSTA_C	0x00	/*                         */
#define		PARAM_VBERXRST_VBERXRSTA_E	0x01	/*                    */

							/*               */
#define		PARAM_RSBERON_RSBERAUTO_M	0x00	/*                       */
#define		PARAM_RSBERON_RSBERAUTO_A	0x10	/*                     */
							/*            */
#define		PARAM_RSBERON_RSBERC_S		0x00	/*                       */
#define		PARAM_RSBERON_RSBERC_B		0x04	/*                        */
							/*            */
#define		PARAM_RSBERON_RSBERB_S		0x00	/*                       */
#define		PARAM_RSBERON_RSBERB_B		0x02	/*                        */
							/*            */
#define		PARAM_RSBERON_RSBERA_S		0x00	/*                       */
#define		PARAM_RSBERON_RSBERA_B		0x01	/*                        */

							/*                */
#define		PARAM_RSBERXRST_RSBERXRSTC_S	0x00	/*                        */
#define		PARAM_RSBERXRST_RSBERXRSTC_B	0x04	/*                       */
							/*                */
#define		PARAM_RSBERXRST_RSBERXRSTB_S	0x00	/*                        */
#define		PARAM_RSBERXRST_RSBERXRSTB_B	0x02	/*                       */
							/*                */
#define		PARAM_RSBERXRST_RSBERXRSTA_S	0x00	/*                        */
#define		PARAM_RSBERXRST_RSBERXRSTA_B	0x01	/*                       */

							/*              */
#define		PARAM_RSBERCEFLG_SBERCEFC_E	0x04	/*                     */
							/*              */
#define		PARAM_RSBERCEFLG_SBERCEFB_E	0x02	/*                     */
							/*              */
#define		PARAM_RSBERCEFLG_SBERCEFA_E	0x01	/*                     */

							/*               */
#define		PARAM_RSBERTHFLG_RSBERTHRC_R	0x40	/*                          */
							/*               */
#define		PARAM_RSBERTHFLG_RSBERTHRB_R	0x20	/*                          */
							/*               */
#define		PARAM_RSBERTHFLG_RSBERTHRA_R	0x10	/*                          */
							/*               */
#define		PARAM_RSBERTHFLG_RSBERTHFC_F	0x04	/*                         */
							/*               */
#define		PARAM_RSBERTHFLG_RSBERTHFB_F	0x02	/*                         */
							/*               */
#define		PARAM_RSBERTHFLG_RSBERTHFA_F	0x01	/*                         */

							/*            */
#define		PARAM_PEREN_PERENC_F		0x04	/*                        */
							/*            */
#define		PARAM_PEREN_PERENB_F		0x02	/*                        */
							/*            */
#define		PARAM_PEREN_PERENA_F		0x01	/*                        */

							/*             */
#define		PARAM_PERRST_PERRSTC		0x04	/*                             */
							/*             */
#define		PARAM_PERRST_PERRSTB		0x02	/*                             */
							/*             */
#define		PARAM_PERRST_PERRSTA		0x01	/*                             */

							/*             */
#define		PARAM_VBERFLG_VBERFLGA		0x01	/*                          */
#define		PARAM_VBERFLG_VBERFLGB		0x02	/*                          */
#define		PARAM_VBERFLG_VBERFLGC		0x04	/*                          */
#define		PARAM_VBERFLG_VBERFLGAB		0x03	/*                            */
#define		PARAM_VBERFLG_VBERFLGAC		0x05	/*                            */
#define		PARAM_VBERFLG_VBERFLGBC		0x06	/*                            */
#define		PARAM_VBERFLG_VBERFLGALL	0x07	/*                            */

							/*                */
#define		PARAM_RSBERCEFLG_SBERCEFA	0x01	/*                            */
#define		PARAM_RSBERCEFLG_SBERCEFB	0x02	/*                            */
#define		PARAM_RSBERCEFLG_SBERCEFC	0x04	/*                            */
#define		PARAM_RSBERCEFLG_SBERCEFAB	0x03	/*                              */
#define		PARAM_RSBERCEFLG_SBERCEFAC	0x05	/*                              */
#define		PARAM_RSBERCEFLG_SBERCEFBC	0x06	/*                              */
#define		PARAM_RSBERCEFLG_SBERCEFALL	0x07	/*                              */

							/*            */
#define		PARAM_PERFLG_PERFLGA		0x01	/*                          */
#define		PARAM_PERFLG_PERFLGB		0x02	/*                          */
#define		PARAM_PERFLG_PERFLGC		0x04	/*                          */
#define		PARAM_PERFLG_PERFLGAB		0x03	/*                            */
#define		PARAM_PERFLG_PERFLGAC		0x05	/*                            */
#define		PARAM_PERFLG_PERFLGBC		0x06	/*                            */
#define		PARAM_PERFLG_PERFLGALL		0x07	/*                            */

//                                                                
//                                                                          
//                                                                          
							/*           */
#define		PARAM_RS0_RSEN_OFF		0x00	/*                         */
#define		PARAM_RS0_RSEN_ON		0x02	/*                         */

							/*             */
#define		PARAM_SBER_SCLKSEL_OFF		0x00	/*                     */
#define		PARAM_SBER_SCLKSEL_ON		0x80	/*                                */
							/*             */
#define		PARAM_SBER_SBERSEL_OFF		0x00	/*                         */
#define		PARAM_SBER_SBERSEL_ON		0x20	/*                              */
							/*            */
#define		PARAM_SBER_SPACON_OFF		0x00	/*                           */
#define		PARAM_SBER_SPACON_ON		0x10	/*                                   */
							/*            */
#define		PARAM_SBER_SENON_OFF		0x00	/*                                */
#define		PARAM_SBER_SENON_ON		0x08	/*                                */
							/*        *//*                      */
#define		PARAM_SBER_SLAYER_OFF		0x00	/*          */
#define		PARAM_SBER_SLAYER_A		0x01	/*             */
#define		PARAM_SBER_SLAYER_B		0x02	/*             */
#define		PARAM_SBER_SLAYER_C		0x03	/*             */
#define		PARAM_SBER_SLAYER_AB		0x04	/*               */
#define		PARAM_SBER_SLAYER_AC		0x05	/*               */
#define		PARAM_SBER_SLAYER_BC		0x06	/*               */
#define		PARAM_SBER_SLAYER_ALL		0x07	/*               */

							/*             */
#define		PARAM_SBER2_SCLKSEL_OFF		0x00	/*                     */
#define		PARAM_SBER2_SCLKSEL_ON		0x80	/*                                */
							/*             */
#define		PARAM_SBER2_SBERSEL_OFF		0x00	/*                         */
#define		PARAM_SBER2_SBERSEL_ON		0x20	/*                              */
							/*            */
#define		PARAM_SBER2_SPACON_OFF		0x00	/*                           */
#define		PARAM_SBER2_SPACON_ON		0x10	/*                                   */
							/*           */
#define		PARAM_SBER2_SENON_OFF		0x00	/*                                */
#define		PARAM_SBER2_SENON_ON		0x08	/*                                */
						 	/*        *//*                      */
#define		PARAM_SBER2_SLAYER_OFF		0x00	/*          */
#define		PARAM_SBER2_SLAYER_A		0x01	/*             */

							/*              */
#define		PARAM_TSOUT_TSERRINV_OFF	0x00	/*                         */
#define		PARAM_TSOUT_TSERRINV_ON		0x80	/*                     */
							/*             */
#define		PARAM_TSOUT_TSENINV_OFF		0x00	/*                        */
#define		PARAM_TSOUT_TSENINV_ON		0x40	/*                    */
							/*            */
#define		PARAM_TSOUT_TSSINV_MSB		0x00	/*                              */
#define		PARAM_TSOUT_TSSINV_LSB		0x20	/*                              */
							/*                */
#define		PARAM_TSOUT_TSERRMASK2_OFF	0x00	/*           */
#define		PARAM_TSOUT_TSERRMASK2_ON	0x04	/*                       */
							/*                */
#define		PARAM_TSOUT_TSERRMASK_OFF	0x00	/*           */
#define		PARAM_TSOUT_TSERRMASK_ON	0x02	/*                                 */

							/*              */
#define		PARAM_TSOUT2_TSERRINV_OFF	0x00	/*                         */
#define		PARAM_TSOUT2_TSERRINV_ON	0x80	/*                     */
							/*             */
#define		PARAM_TSOUT2_TSENINV_OFF	0x00	/*                        */
#define		PARAM_TSOUT2_TSENINV_ON		0x40	/*                    */
							/*            */
#define		PARAM_TSOUT2_TSSINV_MSB		0x00	/*                              */
#define		PARAM_TSOUT2_TSSINV_LSB		0x20	/*                              */

							/*            */
#define		PARAM_PBER_PLAYER_OFF		0x00	/*          */
#define		PARAM_PBER_PLAYER_A		0x01	/*             */
#define		PARAM_PBER_PLAYER_B		0x02	/*             */
#define		PARAM_PBER_PLAYER_C		0x03	/*             */
#define		PARAM_PBER_PLAYER_AB		0x04	/*               */
#define		PARAM_PBER_PLAYER_AC		0x05	/*               */
#define		PARAM_PBER_PLAYER_BC		0x06	/*               */
#define		PARAM_PBER_PLAYER_ALL		0x07	/*               */

							/*            */
#define		PARAM_PBER2_PLAYER_OFF		0x00	/*          */
#define		PARAM_PBER2_PLAYER_A		0x01	/*             */

							/*                */
#define		PARAM_TSMASK0_TSFRMMASK_OFF	0x00	/*                      */
#define		PARAM_TSMASK0_TSFRMMASK_ON	0x80	/*                     */
							/*                */
#define		PARAM_TSMASK0_TSERRMASK_OFF	0x00	/*                          */
#define		PARAM_TSMASK0_TSERRMASK_ON	0x10	/*                         */
							/*                */
#define		PARAM_TSMASK0_TSPACMASK_OFF	0x00	/*                   */
#define		PARAM_TSMASK0_TSPACMASK_ON	0x08	/*                  */
							/*               */
#define		PARAM_TSMASK0_TSENMASK_OFF	0x00	/*                       */
#define		PARAM_TSMASK0_TSENMASK_ON	0x04	/*                      */
							/*               */
#define		PARAM_TSMASK0_TSDTMASK_OFF	0x00	/*                     */
#define		PARAM_TSMASK0_TSDTMASK_ON	0x02	/*                    */
							/*                */
#define		PARAM_TSMASK0_TSCLKMASK_OFF	0x00	/*                      */
#define		PARAM_TSMASK0_TSCLKMASK_ON	0x01	/*                     */

							/*                */
#define		PARAM_TSMASK1_TSFRMMASK_OFF	0x00	/*                      */
#define		PARAM_TSMASK1_TSFRMMASK_ON	0x80	/*                     */
							/*                */
#define		PARAM_TSMASK1_TSERRMASK_OFF	0x00	/*                          */
#define		PARAM_TSMASK1_TSERRMASK_ON	0x10	/*                         */
							/*                */
#define		PARAM_TSMASK1_TSPACMASK_OFF	0x00	/*                   */
#define		PARAM_TSMASK1_TSPACMASK_ON	0x08	/*                  */
							/*               */
#define		PARAM_TSMASK1_TSENMASK_OFF	0x00	/*                       */
#define		PARAM_TSMASK1_TSENMASK_ON	0x04	/*                      */
							/*               */
#define		PARAM_TSMASK1_TSDTMASK_OFF	0x00	/*                     */
#define		PARAM_TSMASK1_TSDTMASK_ON	0x02	/*                    */
							/*                */
#define		PARAM_TSMASK1_TSCLKMASK_OFF	0x00	/*                      */
#define		PARAM_TSMASK1_TSCLKMASK_ON	0x01	/*                     */

							/*          */
#define		PARAM_IRQ_NON			0x00	/*                  */
#define		PARAM_IRQ_YES			0x01	/*               */

							/*           */
#define		PARAM_FECIRQ1_TSERRC		0x40	/*                           */
#define		PARAM_FECIRQ1_TSERRB		0x20	/*                           */
#define		PARAM_FECIRQ1_TSERRA		0x10	/*                           */
#define		PARAM_FECIRQ1_EMG		0x04	/*                        */
#define		PARAM_FECIRQ1_CNT		0x02	/*                             */
#define		PARAM_FECIRQ1_ILL		0x01	/*                                   */

							/*           */
#define		PARAM_FECIRQ2_SBERCEFCC		0x40	/*                                    */
#define		PARAM_FECIRQ2_SBERCEFCB		0x20	/*                                    */
#define		PARAM_FECIRQ2_SBERCEFCA		0x10	/*                                    */
#define		PARAM_FECIRQ2_SBERTHFC		0x04	/*                                        */
#define		PARAM_FECIRQ2_SBERTHFB		0x02	/*                                        */
#define		PARAM_FECIRQ2_SBERTHFA		0x01	/*                                        */

							/*               */
#define		PARAM_TUNER_IRQ_NONE		0x00	/*                   */
#define		PARAM_TUNER_IRQ_GPIO		0x02	/*                   */
#define		PARAM_TUNER_IRQ_CHEND		0x01	/*                    */
#define		PARAM_TUNER_IRQ_GPIO_CHEND	0x03	/*                          */

							/*                  */
#define		PARAM_TMCC_IRQ_MASK_EMG		0x04	/*              */
#define		PARAM_TMCC_IRQ_MASK_CNTDWON	0x02	/*                  */
#define		PARAM_TMCC_IRQ_MASK_ILL		0x01	/*              */

							/*                  */
#define		PARAM_SBER_IRQ_MASK_DTERIRQ	0x80	/*             */
#define		PARAM_SBER_IRQ_MASK_THMASKC	0x20	/*              */
#define		PARAM_SBER_IRQ_MASK_THMASKB	0x10	/*              */
#define		PARAM_SBER_IRQ_MASK_THMASKA	0x08	/*              */
#define		PARAM_SBER_IRQ_MASK_CEMASKC	0x04	/*              */
#define		PARAM_SBER_IRQ_MASK_CEMASKB	0x02	/*              */
#define		PARAM_SBER_IRQ_MASK_CEMASKA	0x01	/*              */

							/*                   */
#define		PARAM_TSERR_IRQ_MASK_C		0x04	/*                     */
#define		PARAM_TSERR_IRQ_MASK_B		0x02	/*                     */
#define		PARAM_TSERR_IRQ_MASK_A		0x01	/*                     */

							/*                  */
#define		PARAM_TMCC_IRQ_RST_EMG		0x04	/*               */
#define		PARAM_TMCC_IRQ_RST_CNTDWON	0x02	/*                  */
#define		PARAM_TMCC_IRQ_RST_ILL		0x01	/*               */

							/*              */
#define		PARAM_RSBERRST_SBERXRSTC_R	0x00	/*                        */
#define		PARAM_RSBERRST_SBERXRSTC_N	0x04	/*                        */
#define		PARAM_RSBERRST_SBERXRSTB_R	0x00	/*                        */
#define		PARAM_RSBERRST_SBERXRSTB_N	0x02	/*                        */
#define		PARAM_RSBERRST_SBERXRSTA_R	0x00	/*                        */
#define		PARAM_RSBERRST_SBERXRSTA_N	0x01	/*                        */

							/*                */
#define		PARAM_RSBERCEFLG_SBERCEFA	0x01	/*                            */
#define		PARAM_RSBERCEFLG_SBERCEFB	0x02	/*                            */
#define		PARAM_RSBERCEFLG_SBERCEFC	0x04	/*                            */
#define		PARAM_RSBERCEFLG_SBERCEFAB	0x03	/*                              */
#define		PARAM_RSBERCEFLG_SBERCEFAC	0x05	/*                              */
#define		PARAM_RSBERCEFLG_SBERCEFBC	0x06	/*                              */
#define		PARAM_RSBERCEFLG_SBERCEFALL	0x07	/*                              */

							/*                */
#define		PARAM_RSBERTHFLG_SBERTHRC_R	0x40	/*                                */
#define		PARAM_RSBERTHFLG_SBERTHRB_R	0x20	/*                                */
#define		PARAM_RSBERTHFLG_SBERTHRA_R	0x10	/*                                */
#define		PARAM_RSBERTHFLG_SBERTHFC_F	0x04	/*                                */
#define		PARAM_RSBERTHFLG_SBERTHFB_F	0x02	/*                                */
#define		PARAM_RSBERTHFLG_SBERTHFA_F	0x01	/*                                */

							/*              */
#define		PARAM_RSERRFLG_RSERRSTC_R	0x40	/*                                */
#define		PARAM_RSERRFLG_RSERRSTB_R	0x20	/*                                */
#define		PARAM_RSERRFLG_RSERRSTA_R	0x10	/*                                */
#define		PARAM_RSERRFLG_RSERRC_F		0x04	/*                          */
#define		PARAM_RSERRFLG_RSERRB_F		0x02	/*                          */
#define		PARAM_RSERRFLG_RSERRA_F		0x01	/*                          */

							/*             */
#define		PARAM_FECIRQ1_RSERRC		0x40	/*                      */
#define		PARAM_FECIRQ1_RSERRB		0x20	/*                      */
#define		PARAM_FECIRQ1_RSERRA		0x10	/*                      */
#define		PARAM_FECIRQ1_LOCK		0x08	/*              */
#define		PARAM_FECIRQ1_EMG		0x04	/*             */
#define		PARAM_FECIRQ1_CNT		0x02	/*                             */
#define		PARAM_FECIRQ1_ILL		0x01	/*                        */

							/*             */
#define		PARAM_XIRQINV_LOW		0x00	/*                */
#define		PARAM_XIRQINV_HIGH		0x04	/*                 */

							/*                                 */
#define		PARAM_TMCCCHK_14		0x40	/*                             */
#define		PARAM_TMCCCHK_13		0x20	/*                   */
#define		PARAM_TMCCCHK_12		0x10	/*                               */
#define		PARAM_TMCCCHK_11		0x08	/*                          */
#define		PARAM_TMCCCHK_10		0x04	/*                */
#define		PARAM_TMCCCHK_9			0x02	/*                                */
#define		PARAM_TMCCCHK_8			0x01	/*                              */
#define		PARAM_TMCCCHK_7			0x80	/*                          */
#define		PARAM_TMCCCHK_6			0x40	/*                         */
#define		PARAM_TMCCCHK_5			0x20	/*                          */
#define		PARAM_TMCCCHK_4			0x10	/*                                                */
#define		PARAM_TMCCCHK_3			0x08	/*                       */
#define		PARAM_TMCCCHK_2			0x04	/*                                */
#define		PARAM_TMCCCHK_1			0x02	/*                     */
#define		PARAM_TMCCCHK_0			0x01	/*                                       */

							/*             */
#define		PARAM_EMG_INV_RECV		0x00	/*                       */
#define		PARAM_EMG_INV_NORECV		0x10	/*                          */

							/*           */
#define		PARAM_CNCNT2_MODE_AUTO		0x00	/*                        */
#define		PARAM_CNCNT2_MODE_MUNL		0x04	/*                         */

							/*           */
#define		PARAM_MERCTRL_MODE_AUTO		0x00	/*                        */
#define		PARAM_MERCTRL_MODE_MUNL		0x02	/*                         */

							/*              */
#define		PARAM_TUNER_IRQCTL_GPIO_RST	0x20	/*                    */
							/*               */
#define		PARAM_TUNER_IRQCTL_CHEND_RST	0x10	/*                          */
							/*               */
#define		PARAM_TUNER_IRQCTL_GPIOMASK	0x02	/*               */
							/*               */
#define		PARAM_TUNER_IRQCTL_CHENDMASK	0x01	/*                     */

							/*                */
#define		PARAM_SEARCH_CTRL_SEARCH_AFT_B	0x00	/*                      */
#define		PARAM_SEARCH_CTRL_SEARCH_AFT_N	0x20	/*                         */
							/*                */
#define		PARAM_SEARCH_CTRL_SEARCH_RST	0x10	/*                        */
							/*            */
#define		PARAM_SEARCH_CTRL_SEARCH	0x01	/*                      */

							/*          */
#define		PARAM_SEARCH_END_END		0x01	/*                                */

#define		PARAM_I2C_MODE_SEND		0x01	/*                    */
#define		PARAM_I2C_MODE_SEND_8		0x01	/*                             */
#define		PARAM_I2C_MODE_SEND_16		0x02	/*                             */
#define		PARAM_I2C_MODE_SEND_24		0x03	/*                             */
#define		PARAM_I2C_MODE_RECV		0x10	/*                     */
#define		PARAM_I2C_MODE_RECV_8		0x10	/*                             */
#define		PARAM_I2C_MODE_RECV_16		0x12	/*                             */
#define		PARAM_I2C_MODE_RECV_24		0x13	/*                             */

#define		PARAM_SPI_MODE_SEND		0x01	/*                     */
#define		PARAM_SPI_MODE_SEND_8		0x01	/*                             */
#define		PARAM_SPI_MODE_SEND_16		0x02	/*                             */
#define		PARAM_SPI_MODE_SEND_24		0x03	/*                             */
#define		PARAM_SPI_MODE_RECV		0x11	/*                     */
#define		PARAM_SPI_MODE_RECV_8		0x11	/*                             */
#define		PARAM_SPI_MODE_RECV_16		0x12	/*                             */
#define		PARAM_SPI_MODE_RECV_24		0x13	/*                             */

#define		PARAM_HRM_ON			0x01	/*                  */
#define		PARAM_HRM_OFF			0x02	/*                  */
#define		PARAM_MODE_LOWER_IF		0x01	/*                            */
#define		PARAM_MODE_UPPER_IF		0x02	/*                            */

#define		PARAM_SPICTRL_STMODE_MASTER	0x00	/*                */
#define		PARAM_SPICTRL_STMODE_SLAVE	0x10	/*               */
#define		PARAM_SPICTRL_SPINUM_SINGLE	0x00	/*                  */
#define		PARAM_SPICTRL_SPINUM_DUAL	0x01	/*                 */
#define		PARAM_IOSEL3_SPIC_OPENDRAIN	0x00	/*                    */
#define		PARAM_IOSEL3_SPIC_CMOS		0x08	/*               */
#define		PARAM_IO_OUTEN_HIZ_EN		0x01	/*                 */
#define		PARAM_IO_OUTEN_HIZ_DIS		0x00	/*                   */

#define         PARAM_STREAMREAD_START          0x01    /*                      */
#define         PARAM_STREAMREAD_STOP           0x02    /*                     */

#define         PARAM_IRQMASK_STREAMREAD_ON     0x00    /*                          */
#define         PARAM_IRQMASK_STREAMREAD_OFF    0x40    /*                           */
#define		PARAM_SBER_TSCLKCTRL_ON		0x40	/*                     */
#define		PARAM_SBER_TSCLKCTRL_OFF	0x00	/*                  */
#define		PARAM_NULLCTRL_PNULL_ON		0x00	/*                             */
#define		PARAM_NULLCTRL_PNULL_OFF	0x02	/*                                */
#define		PARAM_NULLCTRL_SNULL_ON		0x00	/*                             */
#define		PARAM_NULLCTRL_SNULL_OFF	0x01	/*                                */

#define		PARAM_PACBYTE_204P		0x00	/*                        */
#define		PARAM_PACBYTE_188N		0x01	/*                        */
#define		PARAM_ERRCTRL_ERROUT		0x00	/*                  */
#define		PARAM_ERRCTRL_ERRNOOUT		0x01	/*                      */

/*                                                                                                      */

struct ioctl_reset {
	u8 RESET;
	u8 STATE_INIT;
};
typedef struct ioctl_reset ioctl_reset_t;

struct ioctl_init {
	u8 SEGMENT;
	u8 LAYERSEL;
	u8 IQINV;
	u8 CONT_CTRL3;
	u8 MACRO_PDOWN;
	u8 FTSEGCNT;
};
typedef struct ioctl_init ioctl_init_t;

/*     */
struct ioctl_agc {
	u8 mode;
	u16 IFA;		/*               */
	u16 IFB;		/*               */
	u8 IFAGCO;		/*        *//*           */
	u8 MAXIFAGC;		/*          *//*          */
	u16 VIFREF2;		/*                       */
	u8 IFSAMPLE;		/*             */
	/*                    */
	u8 IFAGC;		/*       *//*          */
	u8 IFAGCDAC;		/*             */
};
typedef struct ioctl_agc ioctl_agc_t;

/*      */
struct ioctl_port {
	u8 GPIO_DAT;
	u8 GPIO_OUTSEL;
	u8 DACOUT;
};
typedef struct ioctl_port ioctl_port_t;

/*                  */
struct ioctl_seq {
	u8 S8WAIT;
	u8 STATE_INIT;
	/*                    */
	u8 SYNC_STATE;
	u8 MODED_CTRL;
	u8 MODED_CTRL2;
	u8 MODE_DETECT; //                        
	u8 MODE; //                        
	u8 GUARD; //                        
	u8 MODED_STAT;
	u8 TMCCREAD;		/*             */
	/*                    */
	u8 TMCC[32];		/*      *//*                */
	u8 FEC_IN;
};
typedef struct ioctl_seq ioctl_seq_t;

/*             */
struct ioctl_ber_moni {
	u8 S8WAIT;
	u8 VBERON;
	u8 VBERXRST;
	u8 VBERXRST_SAVE;
	u32 VBERSETA;		/*                                   */
	u32 VBERSETB;		/*                                   */
	u32 VBERSETC;		/*                                   */
	u8 RSBERON;
	u8 RSBERXRST;
	u8 RSBERTHFLG;

	u16 SBERSETA;		/*                         */
	u16 SBERSETB;		/*                         */
	u16 SBERSETC;		/*                         */
	u8 PEREN;
	u8 PERRST;
	u16 PERSNUMA;		/*                         */
	u16 PERSNUMB;		/*                         */
	u16 PERSNUMC;		/*                         */
	/*                    */
	u8 VBERFLG;
	u8 RSBERCEFLG;
	u8 PERFLG;

	u32 VBERDTA;
	u32 VBERDTB;
	u32 VBERDTC;
	u32 RSBERDTA;
	u32 RSBERDTB;
	u32 RSBERDTC;
	u16 PERERRA;
	u16 PERERRB;
	u16 PERERRC;
};
typedef struct ioctl_ber_moni ioctl_ber_moni_t;

/*            */
struct ioctl_ts {
	u8 RS0;
	u8 SBER;
	u8 SBER2;
	u8 TSOUT;
	u8 TSOUT2;
	u8 PBER;
	u8 PBER2;
	u8 TSMASK0;
	u8 TSMASK1;
};
typedef struct ioctl_ts ioctl_ts_t;

/*            */
struct ioctl_irq {
	u8 irq;
	u8 FECIRQ1;
	u8 FECIRQ2;
	u8 TUNER_IRQ;
	u8 TMCC_IRQ_MASK;
	u8 SBER_IRQ_MASK;
	u8 TSERR_IRQ_MASK;
	u8 TMCC_IRQ_RST;
	u8 RSBERON;
	u8 RSBERRST;
	u8 RSBERCEFLG;
	u8 RSBERTHFLG;
	u8 RSERRFLG;
	u8 XIRQINV;
	u8 TMCCCHK_HI;
	u8 TMCCCHK_LO;
	u8 TMCCCHK2_HI;
	u8 TMCCCHK2_LO;
	u8 dummy;
	u8 PCHKOUT0;
	u8 PCHKOUT1;
	u8 EMG_INV;
};
typedef struct ioctl_irq ioctl_irq_t;

	/*             */
struct ioctl_cn_moni {
	u8 CNCNT2;
	u8 cn_count;		/*               */
	u8 CNCNT;		/*               *//*         */
	u16 CNDATA[1];		/*                          */
};
typedef struct ioctl_cn_moni ioctl_cn_moni_t;

	/*             */

struct mer_data {
	u32 A;			/*                                   */
	u32 B;			/*                                   */
	u32 C;			/*                                   */
};

struct ioctl_mer_moni {
	u8 MERCTRL;
	u8 mer_count;		/*               */
	u8 MERSTEP;		/*               *//*        */
	u8 mer_flag;		/*                            */

	u32 mer_dummy32;

	struct mer_data MER[1];
};
typedef struct ioctl_mer_moni ioctl_mer_moni_t;

/*                                    */
struct ioctl_ch_search {
	u8 search_flag;
	u8 mode;
	u8 TUNER_IRQCTL;
	u8 SEARCH_CHANNEL;	/*                    *//*         */
	u8 SEARCH_CTRL;		/*                        */
	u8 CHANNEL[9];		/*                  */
	u8 SEARCH_END;
};
typedef struct ioctl_ch_search ioctl_ch_search_t;

/*                     */
/*              */
struct ioctl_rf {
	u8 mode;
	u8 segments;		/*                               */
	u8 ch_no;		/*                          */
	u8 RFAGC;		/*                                   */
	u8 GVBB;		/*                               */
	u8 LNA;			/*                               */
};
typedef struct ioctl_rf ioctl_rf_t;

/*               */
struct ioctl_i2c {
	u8 mode;		/*                    */
	u8 address_sub;		/*                 */
	u8 data_sub;		/*                 */
	u8 address;		/*                       */
	u8 data[4];		/*                    */
};
typedef struct ioctl_i2c ioctl_i2c_t;

/*               */
/*               */
struct ioctl_spi {
	u8 mode;		/*                    */
	u8 address_sub;		/*                 */
	u8 data_sub;		/*                 */
	u8 address;		/*                       */
	u8 data[4];		/*                    */
};
typedef struct ioctl_spi ioctl_spi_t;

/*               */
struct ioctl_hrm {
	u8 hrm;         	/*              */
};
typedef struct ioctl_hrm ioctl_hrm_t;

/*                         */
struct ioctl_ofdm_init {
	u8 mode;
};
typedef struct ioctl_ofdm_init ioctl_ofdm_init_t;

/*                     */
struct ioctl_low_up_if {
	u8 mode;         	/*                    */
};
typedef struct ioctl_low_up_if ioctl_low_up_if_t;

/*              */
struct ioctl_stream_read {
	u8 SBER;
	u8 NULLCTRL;
};
typedef struct ioctl_stream_read ioctl_stream_read_t;

/*                     */
struct ioctl_stream_read_ctrl {
	u8  STREAM_CTRL;
	u16 READ_SIZE;
	u8 *BUF;
	u32 BUF_SIZE;
};
typedef struct ioctl_stream_read_ctrl ioctl_stream_read_ctrl_t;

/*             */
struct ioctl_spi_config {
	u8 SPICTRL;
	u8 IOSEL3;
	u8 IO_OUTEN;
};
typedef struct ioctl_spi_config ioctl_spi_config_t;

/*           */
struct ioctl_ts_setup {
	u8 PACBYTE;
	u8 ERRCTRL;
};
typedef struct ioctl_ts_setup ioctl_ts_setup_t;

/*                */
struct ioctl_select_antenna {
	u8 mode;         	/*                */
};
typedef struct ioctl_select_antenna ioctl_select_antenna_t;


u32 inline GET32(u32 * merdata)
{
	u32 rtncode = 0;
	memcpy(&rtncode, merdata, sizeof(u32));
	return rtncode;
}

#endif /*                     */
