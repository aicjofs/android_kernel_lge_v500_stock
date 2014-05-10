#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/i2c.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/interrupt.h>
#include <asm/uaccess.h>
#include <linux/gpio.h>
#include <linux/platform_data/mhl_device.h>
#include <linux/input.h>
#include <linux/hid.h>


#include "sii8334_mhl_tx.h"

#ifdef CONFIG_MACH_LGE
#define LGE_ADOPTER_ID 612
#endif

#define SILICON_IMAGE_ADOPTER_ID 322
#define TRANSCODER_DEVICE_ID 0x8334

#define	POWER_STATE_D3				3
#define	POWER_STATE_D0_NO_MHL		2
#define	POWER_STATE_D0_MHL			0
#define	POWER_STATE_FIRST_INIT		0xFF

#define	TIMER_FOR_MONITORING				(TIMER_0)

#define TX_HW_RESET_PERIOD		10	//       
#define TX_HW_RESET_DELAY			100

#define SII8334_I2C_DEVICE_NUMBER	5
static int sii8334_i2c_dev_index = 0;

#define MAX_I2C_MESSAGES	3		/*                                             */
DEFINE_SEMAPHORE(sii8334_irq_sem);

struct i2c_client *sii8334_mhl_i2c_client[SII8334_I2C_DEVICE_NUMBER];

//
//                                     
//
static	uint8_t	fwPowerState = POWER_STATE_FIRST_INIT;

//
//                                                                   
//                                                           
//
static	bool		mscCmdInProgress;	//                                            
//
//                               
//
static	uint8_t	dsHpdStatus = 0;

#ifdef CONFIG_MACH_LGE
static	bool		sii8334_mhl_suspended = false;
#endif

static struct mhl_platform_data *mhl_pdata[SII8334_I2C_DEVICE_NUMBER];

/*                */
static uint8_t MHLSleepStatus = 0;

void SiiRegWrite(uint16_t virtualAddr, uint8_t value)
{
	struct i2c_client* client;

	/*                */
	if(MHLSleepStatus) return;

	client = sii8334_mhl_i2c_client[(virtualAddr >> 8)];
	i2c_smbus_write_byte_data(client, (virtualAddr & 0xff), value);
}

uint8_t SiiRegRead(uint16_t virtualAddr)
{
	int value;
	struct i2c_client* client;

	/*                */
	if(MHLSleepStatus) return 0xFF;

	client = sii8334_mhl_i2c_client[(virtualAddr >> 8)];

	value = i2c_smbus_read_byte_data(client, (virtualAddr & 0xff));

	if (value < 0)
		return 0xFF;
	else
		return value;
}

void SiiRegModify(uint16_t virtualAddr, uint8_t mask, uint8_t value)
{
	uint8_t aByte;

	aByte = SiiRegRead( virtualAddr );
	aByte &= (~mask);                       //                             
	aByte |= (mask & value);                //                         
	SiiRegWrite( virtualAddr, aByte );
}

#define WriteByteCBUS(offset,value)  SiiRegWrite(TX_PAGE_CBUS | (uint16_t)offset,value)
#define ReadByteCBUS(offset)         SiiRegRead( TX_PAGE_CBUS | (uint16_t)offset)
#define	SET_BIT(offset,bitnumber)		SiiRegModify(offset,(1<<bitnumber),(1<<bitnumber))
#define	CLR_BIT(offset,bitnumber)		SiiRegModify(offset,(1<<bitnumber),0x00)
//
//
#define	DISABLE_DISCOVERY				SiiRegModify(REG_DISC_CTRL1,BIT0,0);
#define	ENABLE_DISCOVERY				SiiRegModify(REG_DISC_CTRL1,BIT0,BIT0);

#define STROBE_POWER_ON					SiiRegModify(REG_DISC_CTRL1,BIT1,0);
/*
                             
                                                 
 */

#define INTR_1_DESIRED_MASK   (BIT6 | BIT5)
#define	UNMASK_INTR_1_INTERRUPTS		SiiRegWrite(REG_INTR1_MASK, INTR_1_DESIRED_MASK)
#define	MASK_INTR_1_INTERRUPTS			SiiRegWrite(REG_INTR1_MASK, 0x00)


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

#define	INTR_4_DESIRED_MASK				(BIT0 | BIT2 | BIT3 | BIT4 | BIT5 | BIT6)
#define	UNMASK_INTR_4_INTERRUPTS		SiiRegWrite(REG_INTR4_MASK, INTR_4_DESIRED_MASK)
#define	MASK_INTR_4_INTERRUPTS			SiiRegWrite(REG_INTR4_MASK, 0x00)

//                                            
//                                 
//                                

#define	INTR_5_DESIRED_MASK				(BIT3 | BIT4)
#define	UNMASK_INTR_5_INTERRUPTS		SiiRegWrite(REG_INTR5_MASK, INTR_5_DESIRED_MASK)
#define	MASK_INTR_5_INTERRUPTS			SiiRegWrite(REG_INTR5_MASK, 0x00)

//                                                         
//                       
//                                 
//                                
//                               
//                               
//                             
//                       
//                       
#define	INTR_CBUS1_DESIRED_MASK			(BIT2 | BIT3 | BIT4 | BIT5 | BIT6)
#define	UNMASK_CBUS1_INTERRUPTS			SiiRegWrite(TX_PAGE_CBUS | 0x0009, INTR_CBUS1_DESIRED_MASK)
#define	MASK_CBUS1_INTERRUPTS			SiiRegWrite(TX_PAGE_CBUS | 0x0009, 0x00)

#ifdef CONFIG_LG_MAGIC_MOTION_REMOCON
#define	INTR_CBUS2_DESIRED_MASK			(BIT0 | BIT2 | BIT3)
#else
#define	INTR_CBUS2_DESIRED_MASK			(BIT2 | BIT3)
#endif
#define	UNMASK_CBUS2_INTERRUPTS			SiiRegWrite(TX_PAGE_CBUS | 0x001F, INTR_CBUS2_DESIRED_MASK)
#define	MASK_CBUS2_INTERRUPTS			SiiRegWrite(TX_PAGE_CBUS | 0x001F, 0x00)

#define I2C_INACCESSIBLE -1
#define I2C_ACCESSIBLE 1

void	SiiMhlTxNotifyDsHpdChange( uint8_t dsHpdStatus );
void SiiMhlTxHwReset(uint16_t hwResetPeriod,uint16_t hwResetDelay);
void MhlTxProcessEvents(void);
void SiiMhlTxNotifyRgndMhl( void );
void	SiiMhlTxNotifyConnection( bool mhlConnected );
void	SiiMhlTxGotMhlMscMsg( uint8_t subCommand, uint8_t cmdData );
void	SiiMhlTxMscCommandDone( uint8_t data1 );
void	SiiMhlTxMscWriteBurstDone( uint8_t data1 );
void	SiiMhlTxGotMhlIntr( uint8_t intr_0, uint8_t intr_1 );
void	SiiMhlTxGotMhlStatus( uint8_t status_0, uint8_t status_1 );

static void Int1Isr (void);
static int  Int4Isr (void);
static void Int5Isr (void);
static void MhlCbusIsr (void);

static void CbusReset (void);
static void SwitchToD0 (void);
static void SwitchToD3 (void);
static void WriteInitialRegisterValues (void);
static void InitCBusRegs (void);
static void ForceUsbIdSwitchOpen (void);
static void ReleaseUsbIdSwitchOpen (void);
static void MhlTxDrvProcessConnection (void);
static void MhlTxDrvProcessDisconnection (void);
static void ProcessScdtStatusChange (void);

MhlTxNotifyEventsStatus_e  AppNotifyMhlEvent(uint8_t eventCode, uint8_t eventParam);
bool SiiMhlTxRcpeSend( uint8_t rcpeErrorCode );
bool SiiMhlTxReadDevcap( uint8_t offset );
bool SiiMhlTxSetPathEn(void );
bool SiiMhlTxClrPathEn( void );
bool SiiMhlTxRcpkSend( uint8_t rcpKeyCode );
void  AppNotifyMhlDownStreamHPDStatusChange(bool connected);

bool tmdsPoweredUp;
bool mhlConnected;
uint8_t g_chipRevId;


#ifdef CONFIG_MACH_LGE

struct mhl_common_type
{
	void  (*send_uevent)(char *buf);
	void (*hdmi_hpd_on)(int on);
};

struct mhl_common_type  *mhl_common_state;

extern void hdmi_common_send_uevent(char *buf);
extern void hdmi_common_set_hpd_on(int on);


#ifdef CONFIG_LG_MAGIC_MOTION_REMOCON
#define	MAGIC_CANVAS_X	1280
#define	MAGIC_CANVAS_Y	720
#define	MHD_SCRATCHPAD_SIZE	16
#define	MHD_MAX_BUFFER_SIZE	MHD_SCRATCHPAD_SIZE	//                               
static int	tvCtl_x = MAGIC_CANVAS_X;
static int	tvCtl_y = MAGIC_CANVAS_Y;

static unsigned short kbd_key_pressed[2];

void SiiMhlTxReadScratchpad(void);
void MhlControl(void);

#endif

#endif /*                 */


//                                                                             
//
//                                     
//
//                                            
//
void SiiMhlTxDrvAcquireUpstreamHPDControl (void)
{
	//                                                
	SET_BIT(REG_INT_CTRL, 4);
}

//                                                                             
//
//                                             
//
//                                            
//
void SiiMhlTxDrvAcquireUpstreamHPDControlDriveLow (void)
{
	//                                                                              
	SiiRegModify(REG_INT_CTRL, BIT5 | BIT4, BIT4);	//                                              
}

//                                                                             
//
//                                     
//
//                                            
//
void SiiMhlTxDrvReleaseUpstreamHPDControl (void)
{
	//                                                       
	//                                             
	CLR_BIT(REG_INT_CTRL, 4);
}

static void Int1Isr(void)
{
	uint8_t regIntr1;
	regIntr1 = SiiRegRead(REG_INTR1);

	if (regIntr1)
	{
		//                                                
		SiiRegWrite(REG_INTR1,regIntr1);

		if (BIT6 & regIntr1)
		{
			uint8_t cbusStatus;
			//
			//                                                    
			//
			cbusStatus = SiiRegRead(TX_PAGE_CBUS | 0x000D);

			//                    
			if(BIT6 & (dsHpdStatus ^ cbusStatus))
			{
				uint8_t status = cbusStatus & BIT6;

				//                                               
				SiiMhlTxNotifyDsHpdChange( status );

				if (status)
				{
					SiiMhlTxDrvReleaseUpstreamHPDControl();  //                                                                
				}

				//         
				dsHpdStatus = cbusStatus;
			}
		}
	}
}


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
//                                                                            
bool SiiMhlTxChipInitialize (void)
{
	tmdsPoweredUp = false;
	mhlConnected = false;
	mscCmdInProgress = false;	//                                            
	dsHpdStatus = 0;
	fwPowerState = POWER_STATE_D0_MHL;

	g_chipRevId = SiiRegRead(TX_PAGE_L0 | 0x04);
	/*                */
	//                                               

	//                                                            
	WriteInitialRegisterValues();

	//                                                               
	SwitchToD3();

	return true;
}


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
void SiiMhlTxDeviceIsr (void)
{
	uint8_t intMStatus, i; //                 
	//
	//                                                   
	//

	i=0;

	do
	{
		if( POWER_STATE_D0_MHL != fwPowerState )
		{
			//
			//                                                                
			//                                                                 
			//
			if (I2C_INACCESSIBLE == Int4Isr())
			{
				return; //                                                        
			}
		}
		else if( POWER_STATE_D0_MHL == fwPowerState )
		{
			if (I2C_INACCESSIBLE == Int4Isr())
			{
				return; //                                                        
			}
			//                                                            
			//                                                                   
			Int5Isr();

			//                                             
			//                                                              
			MhlCbusIsr();
			Int1Isr();
		}

		if( POWER_STATE_D3 != fwPowerState )
		{
			//                                                        
			//                                                              
			MhlTxProcessEvents();
		}

		intMStatus = SiiRegRead(TX_PAGE_L0 | 0x0070);	//            
		if(0xFF == intMStatus)
		{
			intMStatus = 0;
		}
		i++;

		intMStatus &= 0x01; //             
	} while (intMStatus);
}

//                                                                             
//
//                       
//
//                                                                            
//
void SiiMhlTxDrvTmdsControl (bool enable)
{
	if( enable )
	{
		SET_BIT(REG_TMDS_CCTRL, 4);
		SiiMhlTxDrvReleaseUpstreamHPDControl();  //                           
	}
	else
	{
		CLR_BIT(REG_TMDS_CCTRL, 4);
	}
}

//                                                                             
//
//                            
//
//                                                                        
//                                                                        
//          
//
void SiiMhlTxDrvNotifyEdidChange (void)
{
	//
	//                                  
	//
	SiiMhlTxDrvAcquireUpstreamHPDControl();

	//                                               
	CLR_BIT(REG_INT_CTRL, 5);

	//           
	msleep(110);

	//                                                       
	SET_BIT(REG_INT_CTRL, 5);

	//                                                                        
	SiiMhlTxDrvReleaseUpstreamHPDControl();
}
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
//                                                                              

bool SiiMhlTxDrvSendCbusCommand (cbus_req_t *pReq)
{
	bool  success = true;

	uint8_t i, startbit;

	//
	//                                    
	//
	if( (POWER_STATE_D0_MHL != fwPowerState ) || (mscCmdInProgress))
	{
		return false;
	}
	//                        
	mscCmdInProgress	= true;

	/*                                                                                      */
	/*                                                                                      */
	/*                                                                                      */
	/*                                                                                      */

	//                                                 
	SiiRegWrite(REG_CBUS_PRI_ADDR_CMD, pReq->offsetData); 	//           
	SiiRegWrite(REG_CBUS_PRI_WR_DATA_1ST, pReq->payload_u.msgData[0]);

	startbit = 0x00;
	switch ( pReq->command )
	{
		case MHL_SET_INT:	//                                  
			startbit = MSC_START_BIT_WRITE_REG;
			break;

		case MHL_WRITE_STAT:	//                                        
			startbit = MSC_START_BIT_WRITE_REG;
			break;

		case MHL_READ_DEVCAP:	//                                           
			startbit = MSC_START_BIT_READ_REG;
			break;

		case MHL_GET_STATE:			//       
		case MHL_GET_VENDOR_ID:		//                     
		case MHL_SET_HPD:			//                                       
		case MHL_CLR_HPD:			//                                         
		case MHL_GET_SC1_ERRORCODE:		//                                        
		case MHL_GET_DDC_ERRORCODE:		//                                           
		case MHL_GET_MSC_ERRORCODE:		//                                   
		case MHL_GET_SC3_ERRORCODE:		//                                         
			SiiRegWrite(REG_CBUS_PRI_ADDR_CMD, pReq->command );
			startbit = MSC_START_BIT_MSC_CMD;
			break;

		case MHL_MSC_MSG:
			SiiRegWrite(REG_CBUS_PRI_WR_DATA_2ND, pReq->payload_u.msgData[1]);
			SiiRegWrite(REG_CBUS_PRI_ADDR_CMD, pReq->command );
			startbit = MSC_START_BIT_VS_CMD;
			break;

		case MHL_WRITE_BURST:
			SiiRegWrite(REG_MSC_WRITE_BURST_LEN, pReq->length -1 );

			//                                                  
			if (NULL == pReq->payload_u.pdatabytes)
			{
				success = false;
			}
			else
			{
				uint8_t *pData = pReq->payload_u.pdatabytes;
				for ( i = 0; i < pReq->length; i++ )
				{
					SiiRegWrite(REG_CBUS_SCRATCHPAD_0 + i, *pData++ );
				}
				startbit = MSC_START_BIT_WRITE_BURST;
			}
			break;

		default:
			success = false;
			break;
	}

	/*                                                                                      */
	/*                                                                                      */
	/*                                                                                      */

	if ( success )
	{
		SiiRegWrite(REG_CBUS_PRI_START, startbit );
	}

	return( success );
}

bool SiiMhlTxDrvCBusBusy (void)
{
	return mscCmdInProgress ? true : false;
}

//                                                                         
//                           
//
//
//                                                                         
static void WriteInitialRegisterValues (void)
{
	//         
	SiiRegWrite(TX_PAGE_L1 | 0x003D, 0x3F);			//                        
	SiiRegWrite(TX_PAGE_2 | 0x0011, 0x01);			//                   
	SiiRegWrite(TX_PAGE_2 | 0x0012, 0x11);			//                                 

	SiiRegWrite(REG_MHLTX_CTL1, 0x10); //                         
	SiiRegWrite(REG_MHLTX_CTL6, 0xBC); //                           
	SiiRegWrite(REG_MHLTX_CTL2, 0x3C); //                              
	SiiRegWrite(REG_MHLTX_CTL4, /*    */0xCA);  //                                            

	SiiRegWrite(REG_MHLTX_CTL7, 0x03); //           
	SiiRegWrite(REG_MHLTX_CTL8, 0x0A); //                                 

	//                   
	SiiRegWrite(REG_TMDS_CCTRL, 0x08);			//                                                                           
	SiiRegWrite(REG_USB_CHARGE_PUMP, 0x8C);		//                                 
	SiiRegWrite(TX_PAGE_L0 | 0x0085, 0x02);

	SiiRegWrite(TX_PAGE_2 | 0x0000, 0x00);
	SiiRegModify(REG_DVI_CTRL3, BIT5, 0);      //           
	SiiRegWrite(TX_PAGE_2 | 0x0013, 0x60);

	SiiRegWrite(TX_PAGE_2 | 0x0017, 0x03);			//              
	SiiRegWrite(TX_PAGE_2 | 0x001A, 0x20);			//        
	SiiRegWrite(TX_PAGE_2 | 0x0022, 0xE0);			//        
	SiiRegWrite(TX_PAGE_2 | 0x0023, 0xC0);			//        
	SiiRegWrite(TX_PAGE_2 | 0x0024, 0xA0);			//        
	SiiRegWrite(TX_PAGE_2 | 0x0025, 0x80);			//        
	SiiRegWrite(TX_PAGE_2 | 0x0026, 0x60);			//        
	SiiRegWrite(TX_PAGE_2 | 0x0027, 0x40);			//        
	SiiRegWrite(TX_PAGE_2 | 0x0028, 0x20);			//        
	SiiRegWrite(TX_PAGE_2 | 0x0029, 0x00);			//        

	SiiRegWrite(TX_PAGE_2 | 0x0031, 0x0A);			//                 
	SiiRegWrite(TX_PAGE_2 | 0x0045, 0x06);			//                         

	SiiRegWrite(TX_PAGE_2 | 0x004B, 0x06);
	SiiRegWrite(TX_PAGE_2 | 0x004C, 0x60);			//                    
	SiiRegWrite(TX_PAGE_2 | 0x004C, 0xE0);			//                    
	SiiRegWrite(TX_PAGE_2 | 0x004D, 0x00);			//               

	SiiRegWrite(TX_PAGE_L0 | 0x0008, 0x35);			//                                                              

	SiiRegWrite(REG_DISC_CTRL2, 0xAD);
	SiiRegWrite(REG_DISC_CTRL5, 0x57);				//                                      
	SiiRegWrite(REG_DISC_CTRL6, 0x11);				//                                                
	SiiRegWrite(REG_DISC_CTRL8, 0x82);				//            

	SiiRegWrite(REG_DISC_CTRL9, 0x24);				//                                                         

	//                                                 
	SiiRegWrite(REG_DISC_CTRL4, 0x8C);				//                                                                   
	SiiRegWrite(REG_DISC_CTRL1, 0x27);				//                      
	SiiRegWrite(REG_DISC_CTRL7, 0x20);				//                    
	SiiRegWrite(REG_DISC_CTRL3, 0x86);				//                   

	//                                            
	if (fwPowerState != TX_POWER_STATE_D3)
	{
		SiiRegModify(REG_INT_CTRL, BIT5 | BIT4, BIT4);	//                                     
	}

	SiiRegModify(REG_INT_CTRL, BIT6, 0);   //               

	SiiRegWrite(REG_SRST, 0x84); 					//                                   

	SiiRegWrite(TX_PAGE_L0 | 0x000D, 0x1C); 		//                           

	CbusReset();

	InitCBusRegs();
}

//                                                                         
//             
//
//                                                                         
static void InitCBusRegs (void)
{
	uint8_t		regval;

	SiiRegWrite(TX_PAGE_CBUS | 0x0007, 0xF2); 			//                                     
	SiiRegWrite(TX_PAGE_CBUS | 0x0036, 0x0B); 			//                                                                                                                  
	SiiRegWrite(REG_CBUS_LINK_CONTROL_8, 0x30); 		//                       
	SiiRegWrite(TX_PAGE_CBUS | 0x0040, 0x03); 			//                    

#define DEVCAP_REG(x) (TX_PAGE_CBUS | 0x0080 | DEVCAP_OFFSET_##x)
	//                 
	SiiRegWrite(DEVCAP_REG(DEV_STATE      ) ,DEVCAP_VAL_DEV_STATE       );
	SiiRegWrite(DEVCAP_REG(MHL_VERSION    ) ,DEVCAP_VAL_MHL_VERSION     );
	SiiRegWrite(DEVCAP_REG(DEV_CAT        ) ,DEVCAP_VAL_DEV_CAT         );
	SiiRegWrite(DEVCAP_REG(ADOPTER_ID_H   ) ,DEVCAP_VAL_ADOPTER_ID_H    );
	SiiRegWrite(DEVCAP_REG(ADOPTER_ID_L   ) ,DEVCAP_VAL_ADOPTER_ID_L    );
	SiiRegWrite(DEVCAP_REG(VID_LINK_MODE  ) ,DEVCAP_VAL_VID_LINK_MODE   );
	SiiRegWrite(DEVCAP_REG(AUD_LINK_MODE  ) ,DEVCAP_VAL_AUD_LINK_MODE   );
	SiiRegWrite(DEVCAP_REG(VIDEO_TYPE     ) ,DEVCAP_VAL_VIDEO_TYPE      );
	SiiRegWrite(DEVCAP_REG(LOG_DEV_MAP    ) ,DEVCAP_VAL_LOG_DEV_MAP     );
	SiiRegWrite(DEVCAP_REG(BANDWIDTH      ) ,DEVCAP_VAL_BANDWIDTH       );
	SiiRegWrite(DEVCAP_REG(FEATURE_FLAG   ) ,DEVCAP_VAL_FEATURE_FLAG    );
	SiiRegWrite(DEVCAP_REG(DEVICE_ID_H    ) ,DEVCAP_VAL_DEVICE_ID_H     );
	SiiRegWrite(DEVCAP_REG(DEVICE_ID_L    ) ,DEVCAP_VAL_DEVICE_ID_L     );
	SiiRegWrite(DEVCAP_REG(SCRATCHPAD_SIZE) ,DEVCAP_VAL_SCRATCHPAD_SIZE );
	SiiRegWrite(DEVCAP_REG(INT_STAT_SIZE  ) ,DEVCAP_VAL_INT_STAT_SIZE   );
	SiiRegWrite(DEVCAP_REG(RESERVED       ) ,DEVCAP_VAL_RESERVED        );

	//                                                                          
	regval = SiiRegRead(REG_CBUS_LINK_CONTROL_2);
	regval = (regval | 0x0C);
	SiiRegWrite(REG_CBUS_LINK_CONTROL_2, regval);

	//                                                         
	SiiRegWrite(REG_MSC_TIMEOUT_LIMIT, 0x0F);

	//              
	SiiRegWrite(REG_CBUS_LINK_CONTROL_1, 0x01);
	SiiRegModify(TX_PAGE_CBUS | 0x002E, BIT4,BIT4);  //                                  

}

//                                                                         
//
//                     
//
//                                                                         
static void ForceUsbIdSwitchOpen (void)
{
	DISABLE_DISCOVERY
		SiiRegModify(REG_DISC_CTRL6, BIT6, BIT6);				//                            
	SiiRegWrite(REG_DISC_CTRL3, 0x86);
	SiiRegModify(REG_INT_CTRL, BIT5 | BIT4, BIT4);		//                                           
}
//                                                                         
//
//                       
//
//                                                                         
static void ReleaseUsbIdSwitchOpen (void)
{
	msleep(50); //         
	SiiRegModify(REG_DISC_CTRL6, BIT6, 0x00);
	ENABLE_DISCOVERY
}
/*
                                
                                                                         
                                   
 */
void SiiMhlTxDrvProcessRgndMhl( void )
{
	SiiRegModify(REG_DISC_CTRL9, BIT0, BIT0);
}
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
void ProcessRgnd (void)
{
	uint8_t rgndImpedance;

	//
	//                                                      
	//
	rgndImpedance = SiiRegRead(REG_DISC_STAT2) & 0x03;

	printk(KERN_INFO "[MHL]Drv: RGND = 0x%02X\n", (int)rgndImpedance);

	//
	//                        
	//                            
	//
	//                                             
	if (0x02 == rgndImpedance)
	{
		printk(KERN_INFO "[MHL]MHL Device\n");
		SiiMhlTxNotifyRgndMhl(); //                                                        
	}
	else
	{
		printk(KERN_INFO "[MHL]Non-MHL Device\n");
		SiiRegModify(REG_DISC_CTRL9, BIT3, BIT3);	//                
	}
}

//                                                                  
//           
//                                                             
//
//                                                                    
//            
//                                                                  
void SwitchToD0 (void)
{
	//
	//                                                                 
	//
	printk(KERN_INFO "[MHL]Switch to D0 mode\n");
	/*                */
	MHLSleepStatus = 0;

	WriteInitialRegisterValues();

	//                        

	STROBE_POWER_ON //                        
		SiiRegModify(TPI_DEVICE_POWER_STATE_CTRL_REG, TX_POWER_STATE_MASK, 0x00);

	fwPowerState = POWER_STATE_D0_NO_MHL;
}

//                                                                  
//           
//
//                                                             
//
//                                                                  
void SwitchToD3 (void)
{
	if(POWER_STATE_D3 != fwPowerState)
	{
#ifndef	__KERNEL__ // 
		pinM2uVbusCtrlM = 1;
		pinMhlConn = 1;
		pinUsbConn = 0;
#endif	// 

		printk(KERN_INFO "[MHL]Switch to D3 mode\n");

		//                                     
		SiiMhlTxDrvAcquireUpstreamHPDControlDriveLow();

		//                                                           
		//                   


		SiiRegWrite(REG_MHLTX_CTL1, 0xD0);

		//
		//                                                        
		//                                                                         
		//                                             
		//
		/*             
                                        
                                    
                                  
    
    
                                                                         
                                                                      
    
   */
		CLR_BIT(TX_PAGE_L1 | 0x003D, 0);

		fwPowerState = POWER_STATE_D3;

		/*                */
		MHLSleepStatus = 1;
		/*
    
       
    
                                        
    
   */
	}

}

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
//                                        
//                                                                  
static int Int4Isr (void)
{
	uint8_t int4Status;

	int4Status = SiiRegRead(REG_INTR4);	//            

	//                                                                                     
	if(0xFF != int4Status)
	{
		if(int4Status)
		{
			printk(KERN_INFO "[MHL]Drv: INT4 Status = 0x%02X\n", (int) int4Status);
		}

		if(int4Status & BIT0) //                   
		{
			if (g_chipRevId < 1)
			{
				ProcessScdtStatusChange();
			}
		}

		//                          
		if(int4Status & BIT2) //            
		{
			MhlTxDrvProcessConnection();
		}

		//                          
		else if(int4Status & BIT3)
		{
			SiiRegWrite(REG_INTR4, int4Status); /*                */
			SiiRegWrite(REG_DISC_STAT2, 0x80);	//                              
			SwitchToD3();
			return I2C_INACCESSIBLE;
		}

		if (int4Status & BIT5)
		{
			MhlTxDrvProcessDisconnection();

			//                                                        
			//                                      
			MhlTxProcessEvents();
			return I2C_INACCESSIBLE;
		}

		if((POWER_STATE_D0_MHL != fwPowerState) && (int4Status & BIT6))
		{
			//                           
			SwitchToD0();

			ProcessRgnd();
		}

		//                             
		if(fwPowerState != POWER_STATE_D3)
		{
			//                        
			if (int4Status & BIT4)
			{
				ForceUsbIdSwitchOpen();
				ReleaseUsbIdSwitchOpen();
			}
		}
	}
	SiiRegWrite(REG_INTR4, int4Status); //                     
	return I2C_ACCESSIBLE;
}

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
//     
//                                                                  
static void Int5Isr (void)
{
	uint8_t int5Status;

	int5Status = SiiRegRead(REG_INTR5);

	SiiRegWrite(REG_INTR5, int5Status);	//                     
}

//                                                                         
//
//                          
//
//                                                                         
static void MhlTxDrvProcessConnection (void)
{
	if( POWER_STATE_D0_MHL == fwPowerState )
	{
		return;
	}
#ifndef	__KERNEL__ // 
	//                  
	pinM2uVbusCtrlM = 0;
	pinMhlConn = 0;
	pinUsbConn = 1;
#endif	//  

	//
	//                                     
	//
	SiiRegWrite(REG_MHLTX_CTL1, 0x10);

	fwPowerState = POWER_STATE_D0_MHL;

	//
	//                                                    
	//                      
	//
	SiiRegWrite(TX_PAGE_CBUS | 0x0007, 0xF2);


	//                                                
	ENABLE_DISCOVERY

		//                                       
		SiiMhlTxNotifyConnection(mhlConnected = true);
}

//                                                                         
//
//                             
//
//                                                                         
static void MhlTxDrvProcessDisconnection (void)
{
	//                     
	SiiRegWrite(REG_INTR4, SiiRegRead(REG_INTR4));

	SiiRegWrite(REG_MHLTX_CTL1, 0xD0);


	dsHpdStatus &= ~BIT6;  //                                           
	SiiRegWrite(TX_PAGE_CBUS | 0x000D, dsHpdStatus);
	SiiMhlTxNotifyDsHpdChange(0);

	if( POWER_STATE_D0_MHL == fwPowerState )
	{
		//                                    
		SiiMhlTxNotifyConnection(false);
	}

	//                           
	SwitchToD3();
}

//                                                                         
//
//          
//
//                                                                         
void CbusReset (void)
{
	uint8_t idx;
	SET_BIT(REG_SRST, 3);
	msleep(2);
	CLR_BIT(REG_SRST, 3);

	mscCmdInProgress = false;

	//                                                    
	UNMASK_INTR_1_INTERRUPTS;
	UNMASK_INTR_4_INTERRUPTS;
	if (g_chipRevId < 1)
	{
		UNMASK_INTR_5_INTERRUPTS;
	}
	else
	{
		//                                  
		MASK_INTR_5_INTERRUPTS;
	}

	UNMASK_CBUS1_INTERRUPTS;
	UNMASK_CBUS2_INTERRUPTS;

	for(idx=0; idx < 4; idx++)
	{
		//                                                                      
		WriteByteCBUS((0xE0 + idx), 0xFF);

		//                                                                      
		WriteByteCBUS((0xF0 + idx), 0xFF);
	}
}

//                                                                         
//
//                  
//
//
//                                                                         
static uint8_t CBusProcessErrors (uint8_t intStatus)
{
	uint8_t result          = 0;
	uint8_t mscAbortReason  = 0;
	uint8_t ddcAbortReason  = 0;

	/*                                                              */

	intStatus &= (BIT_MSC_ABORT | BIT_MSC_XFR_ABORT | BIT_DDC_ABORT);

	if ( intStatus )
	{
		//                                                       

		/*                                                                  */
		if( intStatus & BIT_DDC_ABORT )
		{
			result = ddcAbortReason = SiiRegRead(TX_PAGE_CBUS | REG_DDC_ABORT_REASON);
		}

		if ( intStatus & BIT_MSC_XFR_ABORT )
		{
			result = mscAbortReason = SiiRegRead(TX_PAGE_CBUS | REG_PRI_XFR_ABORT_REASON);

			SiiRegWrite(TX_PAGE_CBUS | REG_PRI_XFR_ABORT_REASON, 0xFF);
		}
		if ( intStatus & BIT_MSC_ABORT )
		{
			SiiRegWrite(TX_PAGE_CBUS | REG_CBUS_PRI_FWR_ABORT_REASON, 0xFF);
		}
	}
	return( result );
}

void SiiMhlTxDrvGetScratchPad (uint8_t startReg,uint8_t *pData,uint8_t length)
{
	int i;

	for (i = 0; i < length;++i,++startReg)
	{
		*pData++ = SiiRegRead(TX_PAGE_CBUS | (0xC0 + startReg));
	}
}

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
//                       
//                                 
//                                
//                               
//                               
//                             
//                       
//                       
//                                                                         
static void MhlCbusIsr (void)
{
	uint8_t		cbusInt;
	uint8_t     gotData[4];	//                                   
	uint8_t		i;
	//
	//                                         
	//
	cbusInt = SiiRegRead(TX_PAGE_CBUS | 0x0008);

	//                                                                                     
	if (cbusInt == 0xFF)
	{
		return;
	}

	if (cbusInt)
	{
		//
		//                                                                 
		//
		SiiRegWrite(TX_PAGE_CBUS | 0x0008, cbusInt);
	}

	//                  
	if ((cbusInt & BIT3))
	{
		uint8_t mscMsg[2];
		//
		//                                            
		//
		mscMsg[0] = SiiRegRead(TX_PAGE_CBUS | 0x0018);
		mscMsg[1] = SiiRegRead(TX_PAGE_CBUS | 0x0019);

		SiiMhlTxGotMhlMscMsg( mscMsg[0], mscMsg[1] );
	}
	if (cbusInt & (BIT_MSC_ABORT | BIT_MSC_XFR_ABORT | BIT_DDC_ABORT))
	{
		gotData[0] = CBusProcessErrors(cbusInt);
	}
	//                       
	if (cbusInt & BIT4)
	{
		mscCmdInProgress = false;

		//                                                        
		SiiMhlTxMscCommandDone( SiiRegRead(TX_PAGE_CBUS | 0x0016) );
	}

	//
	//                                                        
	//               
	//                                               
	//                                       
	//               
	//                                                  
	//
	cbusInt = SiiRegRead(TX_PAGE_CBUS | 0x001E);
	if(cbusInt)
	{
		//
		//                                                                 
		//
		SiiRegWrite(TX_PAGE_CBUS | 0x001E, cbusInt);

	}


	if ( BIT0 & cbusInt)
	{
		//                     
		SiiMhlTxMscWriteBurstDone( cbusInt );
	}

	if(cbusInt & BIT2)
	{
		uint8_t intr[4];
		uint16_t address;

		for(i = 0,address=0x00A0; i < 4; ++i,++address)
		{
			//                              
			intr[i] = SiiRegRead(TX_PAGE_CBUS | address);
			SiiRegWrite( (TX_PAGE_CBUS | address) , intr[i] );
		}
		//                                           
		SiiMhlTxGotMhlIntr( intr[0], intr[1] );
	}

	if (cbusInt & BIT3)
	{
		uint8_t status[4];
		uint16_t address;

		for(i = 0,address=0x00B0; i < 4; ++i,++address)
		{
			//                              
			status[i] = SiiRegRead(TX_PAGE_CBUS | address);
			SiiRegWrite( (TX_PAGE_CBUS | address), 0xFF /*                 */ );
		}
		SiiMhlTxGotMhlStatus( status[0], status[1] );
	}

}

static void ProcessScdtStatusChange(void)
{
	uint8_t scdtStatus;
	uint8_t mhlFifoStatus;

	scdtStatus = SiiRegRead(REG_TMDS_CSTAT);

	if (scdtStatus & 0x02)
	{
		mhlFifoStatus = SiiRegRead(REG_INTR5);
		if (mhlFifoStatus & 0x0C)
		{
			SiiRegWrite(REG_INTR5, 0x0C);

			SiiRegWrite(REG_SRST, 0x94);
			SiiRegWrite(REG_SRST, 0x84);
		}
	}
}

//                                                                             
//
//                        
//
//                                                                         
//                     
//
void SiiMhlTxDrvPowBitChange (bool enable)
{
	//                                   
	if (enable)
	{
		SiiRegModify(REG_DISC_CTRL8, 0x04, 0x04);
	}
}

/*
                     
 */
#define NUM_CBUS_EVENT_QUEUE_EVENTS 5
typedef struct _CBusQueue_t
{
	uint8_t head;   //                                   
	uint8_t tail;
	cbus_req_t queue[NUM_CBUS_EVENT_QUEUE_EVENTS];
}CBusQueue_t,*PCBusQueue_t;


#define QUEUE_SIZE(x) (sizeof(x.queue)/sizeof(x.queue[0]))
#define MAX_QUEUE_DEPTH(x) (QUEUE_SIZE(x) -1)
#define QUEUE_DEPTH(x) ((x.head <= x.tail)?(x.tail-x.head):(QUEUE_SIZE(x)-x.head+x.tail))
#define QUEUE_FULL(x) (QUEUE_DEPTH(x) >= MAX_QUEUE_DEPTH(x))

#define ADVANCE_QUEUE_HEAD(x) { x.head = (x.head < MAX_QUEUE_DEPTH(x))?(x.head+1):0; }
#define ADVANCE_QUEUE_TAIL(x) { x.tail = (x.tail < MAX_QUEUE_DEPTH(x))?(x.tail+1):0; }

#define RETREAT_QUEUE_HEAD(x) { x.head = (x.head > 0)?(x.head-1):MAX_QUEUE_DEPTH(x); }

//                                                               
//                                                                  
//                                
CBusQueue_t CBusQueue;

cbus_req_t *GetNextCBusTransactionImpl(void)
{
	if (0==QUEUE_DEPTH(CBusQueue))
	{
		return NULL;
	}
	else
	{
		cbus_req_t *retVal;
		retVal = &CBusQueue.queue[CBusQueue.head];
		ADVANCE_QUEUE_HEAD(CBusQueue)
			return retVal;
	}
}

#define GetNextCBusTransaction(func) GetNextCBusTransactionImpl()

bool PutNextCBusTransactionImpl(cbus_req_t *pReq)
{
	if (QUEUE_FULL(CBusQueue))
	{
		//             
		return false;
	}
	//                            
	CBusQueue.queue[CBusQueue.tail] = *pReq;
	ADVANCE_QUEUE_TAIL(CBusQueue)
		return true;
}

#define PutNextCBusTransaction(req) PutNextCBusTransactionImpl(req)

bool PutPriorityCBusTransactionImpl(cbus_req_t *pReq)
{
	if (QUEUE_FULL(CBusQueue))
	{
		//             
		return false;
	}
	//                            
	RETREAT_QUEUE_HEAD(CBusQueue)
		CBusQueue.queue[CBusQueue.head] = *pReq;
	return true;
}
#define PutPriorityCBusTransaction(pReq) PutPriorityCBusTransactionImpl(pReq)

#define IncrementCBusReferenceCount(func) {mhlTxConfig.cbusReferenceCount++; }
#define DecrementCBusReferenceCount(func) {mhlTxConfig.cbusReferenceCount--; }

#define SetMiscFlag(func,x) { mhlTxConfig.miscFlags |=  (x); }
#define ClrMiscFlag(func,x) { mhlTxConfig.miscFlags &= ~(x); }
//
//                                                             
//
//
//
//                                                           
//
static	mhlTx_config_t	mhlTxConfig={0};
//
//                           
//
static bool SiiMhlTxSetDCapRdy( void );
/*                                       */
static	bool 		SiiMhlTxRapkSend( void );

static	void		MhlTxResetStates( void );
static	bool		MhlTxSendMscMsg ( uint8_t command, uint8_t cmdData );
static void SiiMhlTxRefreshPeerDevCapEntries(void);
static void MhlTxDriveStates( void );

#define	MHL_DEV_LD_DISPLAY					(0x01 << 0)
#define	MHL_DEV_LD_VIDEO					(0x01 << 1)
#define	MHL_DEV_LD_AUDIO					(0x01 << 2)
#define	MHL_DEV_LD_MEDIA					(0x01 << 3)
#define	MHL_DEV_LD_TUNER					(0x01 << 4)
#define	MHL_DEV_LD_RECORD					(0x01 << 5)
#define	MHL_DEV_LD_SPEAKER					(0x01 << 6)
#define	MHL_DEV_LD_GUI						(0x01 << 7)


#define	MHL_MAX_RCP_KEY_CODE	(0x7F + 1)	//          

uint8_t rcpSupportTable [MHL_MAX_RCP_KEY_CODE] = {
	(MHL_DEV_LD_GUI),		//              
	(MHL_DEV_LD_GUI),		//          
	(MHL_DEV_LD_GUI),		//            
	(MHL_DEV_LD_GUI),		//            
	(MHL_DEV_LD_GUI),		//             
	0, 0, 0, 0,				//               
	(MHL_DEV_LD_GUI),		//                 
	0, 0, 0,				//               
	(MHL_DEV_LD_GUI),		//              
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,	//               
	(MHL_DEV_LD_VIDEO | MHL_DEV_LD_AUDIO | MHL_DEV_LD_MEDIA | MHL_DEV_LD_TUNER),	//                       
	(MHL_DEV_LD_VIDEO | MHL_DEV_LD_AUDIO | MHL_DEV_LD_MEDIA | MHL_DEV_LD_TUNER),
	(MHL_DEV_LD_VIDEO | MHL_DEV_LD_AUDIO | MHL_DEV_LD_MEDIA | MHL_DEV_LD_TUNER),
	(MHL_DEV_LD_VIDEO | MHL_DEV_LD_AUDIO | MHL_DEV_LD_MEDIA | MHL_DEV_LD_TUNER),
	(MHL_DEV_LD_VIDEO | MHL_DEV_LD_AUDIO | MHL_DEV_LD_MEDIA | MHL_DEV_LD_TUNER),
	(MHL_DEV_LD_VIDEO | MHL_DEV_LD_AUDIO | MHL_DEV_LD_MEDIA | MHL_DEV_LD_TUNER),
	(MHL_DEV_LD_VIDEO | MHL_DEV_LD_AUDIO | MHL_DEV_LD_MEDIA | MHL_DEV_LD_TUNER),
	(MHL_DEV_LD_VIDEO | MHL_DEV_LD_AUDIO | MHL_DEV_LD_MEDIA | MHL_DEV_LD_TUNER),
	(MHL_DEV_LD_VIDEO | MHL_DEV_LD_AUDIO | MHL_DEV_LD_MEDIA | MHL_DEV_LD_TUNER),
	(MHL_DEV_LD_VIDEO | MHL_DEV_LD_AUDIO | MHL_DEV_LD_MEDIA | MHL_DEV_LD_TUNER),
	0,						//           
	(MHL_DEV_LD_VIDEO | MHL_DEV_LD_AUDIO | MHL_DEV_LD_MEDIA | MHL_DEV_LD_TUNER),	//                 
	(MHL_DEV_LD_VIDEO | MHL_DEV_LD_AUDIO | MHL_DEV_LD_MEDIA | MHL_DEV_LD_TUNER),	//                 
	0, 0, 0,				//               
	(MHL_DEV_LD_TUNER),		//                  
	(MHL_DEV_LD_TUNER),		//                  
	(MHL_DEV_LD_TUNER),		//                        
	(MHL_DEV_LD_AUDIO),		//                    
	0,						//                    
	0,						//                        
	0,						//            
	0,						//               
	0,						//                 
	0, 0, 0, 0, 0, 0, 0,	//                   
	0,						//                 

	(MHL_DEV_LD_SPEAKER),	//                 
	(MHL_DEV_LD_SPEAKER),	//                   
	(MHL_DEV_LD_SPEAKER),	//            
	(MHL_DEV_LD_VIDEO | MHL_DEV_LD_AUDIO),	//            
	(MHL_DEV_LD_VIDEO | MHL_DEV_LD_AUDIO | MHL_DEV_LD_RECORD),	//            
	(MHL_DEV_LD_VIDEO | MHL_DEV_LD_AUDIO | MHL_DEV_LD_RECORD),	//             
	(MHL_DEV_LD_RECORD),	//              
	(MHL_DEV_LD_VIDEO | MHL_DEV_LD_AUDIO),	//              
	(MHL_DEV_LD_VIDEO | MHL_DEV_LD_AUDIO),	//                    
	(MHL_DEV_LD_MEDIA),		//             
	(MHL_DEV_LD_VIDEO | MHL_DEV_LD_AUDIO | MHL_DEV_LD_MEDIA),	//               
	(MHL_DEV_LD_VIDEO | MHL_DEV_LD_AUDIO | MHL_DEV_LD_MEDIA),	//                
	0, 0, 0,				//               
	0,						//             
	0,						//                  
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //               
	(MHL_DEV_LD_VIDEO | MHL_DEV_LD_AUDIO),	//                     
	(MHL_DEV_LD_VIDEO | MHL_DEV_LD_AUDIO |MHL_DEV_LD_RECORD),	//                               
	(MHL_DEV_LD_RECORD),	//                       
	(MHL_DEV_LD_RECORD),	//                                 
	(MHL_DEV_LD_VIDEO | MHL_DEV_LD_AUDIO | MHL_DEV_LD_RECORD),	//                     

	(MHL_DEV_LD_SPEAKER),	//                     
	(MHL_DEV_LD_SPEAKER),	//                             
	0, 0, 0, 0, 0, 0, 0, 0, 0, 	                        //                                
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 		//                                
};

bool MhlTxCBusBusy(void)
{
	return ((QUEUE_DEPTH(CBusQueue) > 0)||SiiMhlTxDrvCBusBusy() || mhlTxConfig.cbusReferenceCount)?true:false;
}
/*
                  
                              

             
                              
                                  
 */
static uint8_t QualifyPathEnable( void )
{
	uint8_t retVal = 0;  //                       
	if (MHL_STATUS_PATH_ENABLED & mhlTxConfig.status_1)
	{
		retVal = 1;
	}
	else
	{
		if (0x10 == mhlTxConfig.aucDevCapCache[DEVCAP_OFFSET_MHL_VERSION])
		{
			if (0x44 == mhlTxConfig.aucDevCapCache[DEVCAP_OFFSET_INT_STAT_SIZE])
			{
				retVal = 1;
			}
		}
	}
	return retVal;
}
//                                                                             
//                   
//
//                                                                               
//
//
static void SiiMhlTxTmdsEnable(void)
{
	if (MHL_RSEN & mhlTxConfig.mhlHpdRSENflags)
	{
		if (MHL_HPD & mhlTxConfig.mhlHpdRSENflags)
		{
			if (QualifyPathEnable())
			{
				if (RAP_CONTENT_ON & mhlTxConfig.rapFlags)
				{
					SiiMhlTxDrvTmdsControl( true );
				}
			}
		}
	}
}

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
//                                              
//                                              
//
static bool SiiMhlTxSetInt( uint8_t regToWrite,uint8_t  mask, uint8_t priorityLevel )
{
	cbus_req_t	req;
	bool retVal;

	//                                 
	//         
	req.retryCount  = 2;
	req.command     = MHL_SET_INT;
	req.offsetData  = regToWrite;
	req.payload_u.msgData[0]  = mask;
	if (0 == priorityLevel)
	{
		retVal = PutPriorityCBusTransaction(&req);
	}
	else
	{
		retVal = PutNextCBusTransaction(&req);
	}
	return retVal;
}

//                                                                             
//
//                     
//
static bool SiiMhlTxDoWriteBurst( uint8_t startReg, uint8_t *pData,uint8_t length )
{
	if (FLAGS_WRITE_BURST_PENDING & mhlTxConfig.miscFlags)
	{
		cbus_req_t	req;
		bool retVal;

		req.retryCount  = 1;
		req.command     = MHL_WRITE_BURST;
		req.length      = length;
		req.offsetData  = startReg;
		req.payload_u.pdatabytes  = pData;

		retVal = PutPriorityCBusTransaction(&req);
		ClrMiscFlag(SiiMhlTxDoWriteBurst, FLAGS_WRITE_BURST_PENDING)
			return retVal;
	}
	return false;
}

//                                                                       
//                          
//
ScratchPadStatus_e SiiMhlTxRequestWriteBurst(uint8_t startReg, uint8_t length, uint8_t *pData)
{
	ScratchPadStatus_e retVal = SCRATCHPAD_BUSY;

	if (!(MHL_FEATURE_SP_SUPPORT & mhlTxConfig.mscFeatureFlag))
	{
		retVal= SCRATCHPAD_NOT_SUPPORTED;
	}
	else
	{
		if (
				(FLAGS_SCRATCHPAD_BUSY & mhlTxConfig.miscFlags)
				||
				MhlTxCBusBusy()
			)
		{

		}
		else
		{
			bool temp;
			uint8_t i,reg;
			for (i = 0,reg=startReg; (i < length) && (reg < SCRATCHPAD_SIZE); ++i,++reg)
			{
				mhlTxConfig.localScratchPad[reg]=pData[i];
			}

			temp =  SiiMhlTxSetInt(MHL_RCHANGE_INT,MHL_INT_REQ_WRT, 1);
			retVal = temp ? SCRATCHPAD_SUCCESS: SCRATCHPAD_FAIL;
		}
	}
	return retVal;
}

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
//
//
//
//
void SiiMhlTxInitialize(uint8_t pollIntervalMs )
{
	//                                           
	CBusQueue.head = 0;
	CBusQueue.tail = 0;

	//
	//                            
	//
	mhlTxConfig.pollIntervalMs  = pollIntervalMs;

	MhlTxResetStates( );

	SiiMhlTxChipInitialize ();
}



//                                                                             
//
//                   
//
//                                                                           
//                                                                             
//                                                                                  
//       
//
void MhlTxProcessEvents(void)
{
	//                                                                  
	MhlTxDriveStates();
	if( mhlTxConfig.mhlConnectionEvent )
	{
		//                    
		mhlTxConfig.mhlConnectionEvent = false;

		//
		//                                                       
		//
		AppNotifyMhlEvent(mhlTxConfig.mhlConnected, mhlTxConfig.mscFeatureFlag);

		//                                                    
		if(MHL_TX_EVENT_DISCONNECTION == mhlTxConfig.mhlConnected)
		{
			MhlTxResetStates( );
		}
		else if (MHL_TX_EVENT_CONNECTION == mhlTxConfig.mhlConnected)
		{
			SiiMhlTxSetDCapRdy();
		}
	}
	else if( mhlTxConfig.mscMsgArrived )
	{
		//                    
		mhlTxConfig.mscMsgArrived = false;

		//
		//                               
		//
		switch( mhlTxConfig.mscMsgSubCommand )
		{
			case	MHL_MSC_MSG_RAP:
				//                           
				//
				//                                    
				//
				if( MHL_RAP_CONTENT_ON == mhlTxConfig.mscMsgData)
				{
					mhlTxConfig.rapFlags |= RAP_CONTENT_ON;
					SiiMhlTxTmdsEnable();
				}
				else if( MHL_RAP_CONTENT_OFF == mhlTxConfig.mscMsgData)
				{
					mhlTxConfig.rapFlags &= ~RAP_CONTENT_ON;
					SiiMhlTxDrvTmdsControl( false );
				}
				//                        
				SiiMhlTxRapkSend( );
				break;

			case	MHL_MSC_MSG_RCP:
				//                                                           
				//                         
				if(MHL_LOGICAL_DEVICE_MAP & rcpSupportTable [mhlTxConfig.mscMsgData & 0x7F] )
				{
					AppNotifyMhlEvent(MHL_TX_EVENT_RCP_RECEIVED, mhlTxConfig.mscMsgData);
				}
				else
				{
					//                                        
					mhlTxConfig.mscSaveRcpKeyCode = mhlTxConfig.mscMsgData; //         
					SiiMhlTxRcpeSend( RCPE_INEEFECTIVE_KEY_CODE );
				}
				break;

			case	MHL_MSC_MSG_RCPK:
				AppNotifyMhlEvent(MHL_TX_EVENT_RCPK_RECEIVED, mhlTxConfig.mscMsgData);
				DecrementCBusReferenceCount(MhlTxProcessEvents)
					mhlTxConfig.mscLastCommand = 0;
				mhlTxConfig.mscMsgLastCommand = 0;
				break;

			case	MHL_MSC_MSG_RCPE:
				AppNotifyMhlEvent(MHL_TX_EVENT_RCPE_RECEIVED, mhlTxConfig.mscMsgData);
				break;

			case	MHL_MSC_MSG_RAPK:
				//                                                                 
				DecrementCBusReferenceCount(MhlTxProcessEvents)
					mhlTxConfig.mscLastCommand = 0;
				mhlTxConfig.mscMsgLastCommand = 0;
				break;

			default:
				//                                                         
				break;
		}
	}
}

//                                                                             
//
//                 
//
//                                                                      
//                                                                            
//                 
//
static void MhlTxDriveStates( void )
{
	//                                 
	if (QUEUE_DEPTH(CBusQueue) > 0)
	{
		if (!SiiMhlTxDrvCBusBusy())
		{
			int reQueueRequest = 0;
			cbus_req_t *pReq = GetNextCBusTransaction(MhlTxDriveStates);
			//                                            
			if (MHL_SET_INT == pReq->command)
			{
				if (MHL_RCHANGE_INT == pReq->offsetData)
				{
					if (FLAGS_SCRATCHPAD_BUSY & mhlTxConfig.miscFlags)
					{
						if (MHL_INT_REQ_WRT == pReq->payload_u.msgData[0])
						{
							reQueueRequest= 1;
						}
						else if (MHL_INT_GRT_WRT == pReq->payload_u.msgData[0])
						{
							reQueueRequest= 0;
						}
					}
					else
					{
						if (MHL_INT_REQ_WRT == pReq->payload_u.msgData[0])
						{
							IncrementCBusReferenceCount(MhlTxDriveStates)
								SetMiscFlag(MhlTxDriveStates, FLAGS_SCRATCHPAD_BUSY)
								SetMiscFlag(MhlTxDriveStates, FLAGS_WRITE_BURST_PENDING)
						}
						else if (MHL_INT_GRT_WRT == pReq->payload_u.msgData[0])
						{
							SetMiscFlag(MhlTxDriveStates, FLAGS_SCRATCHPAD_BUSY)
						}
					}
				}
			}
			if (reQueueRequest)
			{
				//                                                         
				if (pReq->retryCount-- > 0)
				{
					PutNextCBusTransaction(pReq);
				}
			}
			else
			{
				if (MHL_MSC_MSG == pReq->command)
				{
					mhlTxConfig.mscMsgLastCommand = pReq->payload_u.msgData[0];
					mhlTxConfig.mscMsgLastData    = pReq->payload_u.msgData[1];
				}
				else
				{
					mhlTxConfig.mscLastOffset  = pReq->offsetData;
					mhlTxConfig.mscLastData    = pReq->payload_u.msgData[0];

				}
				mhlTxConfig.mscLastCommand = pReq->command;

				IncrementCBusReferenceCount(MhlTxDriveStates)
					SiiMhlTxDrvSendCbusCommand( pReq  );
			}
		}
	}
}


static void ExamineLocalAndPeerVidLinkMode( void )
{
	//                   
	mhlTxConfig.linkMode &= ~MHL_STATUS_CLK_MODE_MASK;
	mhlTxConfig.linkMode |= MHL_STATUS_CLK_MODE_NORMAL;

	//                                                      
	//                                     
	if (MHL_DEV_VID_LINK_SUPP_PPIXEL & mhlTxConfig.aucDevCapCache[DEVCAP_OFFSET_VID_LINK_MODE])
	{
		if (MHL_DEV_VID_LINK_SUPP_PPIXEL & DEVCAP_VAL_VID_LINK_MODE)
		{
			mhlTxConfig.linkMode &= ~MHL_STATUS_CLK_MODE_MASK;
			mhlTxConfig.linkMode |= mhlTxConfig.preferredClkMode;
		}
	}
}

//                                                                             
//
//                       
//
//                                                                               
//
//                                                                                 
//                                                                               
//
#define FLAG_OR_NOT(x) (FLAGS_HAVE_##x & mhlTxConfig.miscFlags)?#x:""
#define SENT_OR_NOT(x) (FLAGS_SENT_##x & mhlTxConfig.miscFlags)?#x:""

void	SiiMhlTxMscCommandDone( uint8_t data1 )
{
	DecrementCBusReferenceCount(SiiMhlTxMscCommandDone)
		if ( MHL_READ_DEVCAP == mhlTxConfig.mscLastCommand )
		{
			if (mhlTxConfig.mscLastOffset < sizeof(mhlTxConfig.aucDevCapCache) )
			{
				//                   
				mhlTxConfig.aucDevCapCache[mhlTxConfig.mscLastOffset] = data1;

				if(MHL_DEV_CATEGORY_OFFSET == mhlTxConfig.mscLastOffset)
				{
					uint8_t param;
					mhlTxConfig.miscFlags |= FLAGS_HAVE_DEV_CATEGORY;
					param  = data1 & MHL_DEV_CATEGORY_POW_BIT;

					//                                                    
					if (MHL_TX_EVENT_STATUS_PASSTHROUGH == AppNotifyMhlEvent(MHL_TX_EVENT_POW_BIT_CHG, param))
					{
						SiiMhlTxDrvPowBitChange((bool)param );
					}

				}
				else if(MHL_DEV_FEATURE_FLAG_OFFSET == mhlTxConfig.mscLastOffset)
				{
					mhlTxConfig.miscFlags |= FLAGS_HAVE_DEV_FEATURE_FLAGS;

					//                              
					mhlTxConfig.mscFeatureFlag	= data1;
				}
				else if (DEVCAP_OFFSET_VID_LINK_MODE == mhlTxConfig.mscLastOffset)
				{
					ExamineLocalAndPeerVidLinkMode();
				}


				if ( ++mhlTxConfig.ucDevCapCacheIndex < sizeof(mhlTxConfig.aucDevCapCache) )
				{
					//                                                                                         
					SiiMhlTxReadDevcap( mhlTxConfig.ucDevCapCacheIndex );
				}
				else
				{
					//                                                      
					AppNotifyMhlEvent(MHL_TX_EVENT_DCAP_CHG, 0);

					//                                                                
					//                        
					//                               
					mhlTxConfig.mscLastCommand = 0;
					mhlTxConfig.mscLastOffset  = 0;
				}
			}
		}
		else if(MHL_WRITE_STAT == mhlTxConfig.mscLastCommand)
		{
			if (MHL_STATUS_REG_CONNECTED_RDY == mhlTxConfig.mscLastOffset)
			{
				if (MHL_STATUS_DCAP_RDY & mhlTxConfig.mscLastData)
				{
					mhlTxConfig.miscFlags |= FLAGS_SENT_DCAP_RDY;
					SiiMhlTxSetInt(MHL_RCHANGE_INT,MHL_INT_DCAP_CHG, 0); //                 
				}
			}
			else if (MHL_STATUS_REG_LINK_MODE == mhlTxConfig.mscLastOffset)
			{
				if ( MHL_STATUS_PATH_ENABLED & mhlTxConfig.mscLastData)
				{
					mhlTxConfig.miscFlags |= FLAGS_SENT_PATH_EN;
				}
			}

			mhlTxConfig.mscLastCommand = 0;
			mhlTxConfig.mscLastOffset  = 0;
		}
		else if (MHL_MSC_MSG == mhlTxConfig.mscLastCommand)
		{
			if(MHL_MSC_MSG_RCPE == mhlTxConfig.mscMsgLastCommand)
			{
				//
				//                                                                     
				//
				if( SiiMhlTxRcpkSend( mhlTxConfig.mscSaveRcpKeyCode ) )
				{
				}
			}
			else
			{
			}
			mhlTxConfig.mscLastCommand = 0;
		}
		else if (MHL_WRITE_BURST == mhlTxConfig.mscLastCommand)
		{
			mhlTxConfig.mscLastCommand = 0;
			mhlTxConfig.mscLastOffset  = 0;
			mhlTxConfig.mscLastData    = 0;

			//                                                        
			//                                                             
			SiiMhlTxSetInt( MHL_RCHANGE_INT,MHL_INT_DSCR_CHG,0 );
		}
		else if (MHL_SET_INT == mhlTxConfig.mscLastCommand)
		{
			if (MHL_RCHANGE_INT == mhlTxConfig.mscLastOffset)
			{
				if (MHL_INT_DSCR_CHG == mhlTxConfig.mscLastData)
				{
					DecrementCBusReferenceCount(SiiMhlTxMscCommandDone)  //                                        
						ClrMiscFlag(SiiMhlTxMscCommandDone, FLAGS_SCRATCHPAD_BUSY)
				}
			}
			//                                                                   
			mhlTxConfig.mscLastCommand = 0;
			mhlTxConfig.mscLastOffset  = 0;
			mhlTxConfig.mscLastData    = 0;
		}
		else
		{
		}
	if (!(FLAGS_RCP_READY & mhlTxConfig.miscFlags))
	{
		if (FLAGS_HAVE_DEV_CATEGORY & mhlTxConfig.miscFlags)
		{
			if (FLAGS_HAVE_DEV_FEATURE_FLAGS& mhlTxConfig.miscFlags)
			{
				if (FLAGS_SENT_PATH_EN & mhlTxConfig.miscFlags)
				{
					if (FLAGS_SENT_DCAP_RDY & mhlTxConfig.miscFlags)
					{
						if (mhlTxConfig.ucDevCapCacheIndex >= sizeof(mhlTxConfig.aucDevCapCache))
						{
							mhlTxConfig.miscFlags |= FLAGS_RCP_READY;
							//                                          
							//                        
							mhlTxConfig.mhlConnectionEvent = true;
							mhlTxConfig.mhlConnected = MHL_TX_EVENT_RCP_READY;
						}
					}
				}
			}
		}
	}
}

//                                                                             
//
//                          
//
//                                                                                
//
//                                                                                 
//                                                                               
//
void	SiiMhlTxMscWriteBurstDone( uint8_t data1 )
{
#ifdef CONFIG_LG_MAGIC_MOTION_REMOCON
	SiiMhlTxReadScratchpad();
#else
#define WRITE_BURST_TEST_SIZE 16
	uint8_t temp[WRITE_BURST_TEST_SIZE];
	uint8_t i;
	data1=data1;  //                                       
	SiiMhlTxDrvGetScratchPad(0,temp,WRITE_BURST_TEST_SIZE);
	for (i = 0; i < WRITE_BURST_TEST_SIZE ; ++i)
	{
		if (temp[i]>=' ')
		{
		}
		else
		{
		}
	}
#endif
}


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
//
//                                          
//
void	SiiMhlTxGotMhlMscMsg( uint8_t subCommand, uint8_t cmdData )
{
	//                    
	mhlTxConfig.mscMsgArrived		= true;
	mhlTxConfig.mscMsgSubCommand	= subCommand;
	mhlTxConfig.mscMsgData			= cmdData;
}
//                                                                             
//
//                   
//
//                                                                               
//
//                                                                                 
//                                                                               
//
void	SiiMhlTxGotMhlIntr( uint8_t intr_0, uint8_t intr_1 )
{
	//
	//                          
	//
	if(MHL_INT_DCAP_CHG & intr_0)
	{
		if (MHL_STATUS_DCAP_RDY & mhlTxConfig.status_0)
		{
			SiiMhlTxRefreshPeerDevCapEntries();
		}
	}

	if( MHL_INT_DSCR_CHG & intr_0)
	{
		SiiMhlTxDrvGetScratchPad(0,mhlTxConfig.localScratchPad,sizeof(mhlTxConfig.localScratchPad));
		//                               
		ClrMiscFlag(SiiMhlTxGotMhlIntr, FLAGS_SCRATCHPAD_BUSY)
			AppNotifyMhlEvent(MHL_TX_EVENT_DSCR_CHG,0);
	}
	if( MHL_INT_REQ_WRT  & intr_0)
	{

		//                                        
		if (FLAGS_SCRATCHPAD_BUSY & mhlTxConfig.miscFlags)
		{
			//                                            
			//                       
			SiiMhlTxSetInt( MHL_RCHANGE_INT, MHL_INT_GRT_WRT,1);
		}
		else
		{
			SetMiscFlag(SiiMhlTxGotMhlIntr, FLAGS_SCRATCHPAD_BUSY)
				//                                                    
				//                                      
				SiiMhlTxSetInt( MHL_RCHANGE_INT, MHL_INT_GRT_WRT,0);
		}
	}
	if( MHL_INT_GRT_WRT  & intr_0)
	{
		uint8_t length =sizeof(mhlTxConfig.localScratchPad);
		SiiMhlTxDoWriteBurst(0x40, mhlTxConfig.localScratchPad, length);
	}

	//                                                                           
	if(MHL_INT_EDID_CHG & intr_1)
	{
		//                                              
		//                                                 
		SiiMhlTxDrvNotifyEdidChange ( );
	}
}
//                                                                             
//
//                     
//
//                                                                            
//
//                                                                                 
//                                                                               
//
void	SiiMhlTxGotMhlStatus( uint8_t status_0, uint8_t status_1 )
{
	//
	//                                   
	//
	uint8_t StatusChangeBitMask0,StatusChangeBitMask1;
	StatusChangeBitMask0 = status_0 ^ mhlTxConfig.status_0;
	StatusChangeBitMask1 = status_1 ^ mhlTxConfig.status_1;
	//                                                                                                                              
	mhlTxConfig.status_0 = status_0;
	mhlTxConfig.status_1 = status_1;

	if(MHL_STATUS_DCAP_RDY & StatusChangeBitMask0)
	{
		if (MHL_STATUS_DCAP_RDY & status_0)
		{
			SiiMhlTxRefreshPeerDevCapEntries();
		}
	}


	//                    
	if(MHL_STATUS_PATH_ENABLED & StatusChangeBitMask1)
	{
		if(MHL_STATUS_PATH_ENABLED & status_1)
		{
			//                                                   
			SiiMhlTxSetPathEn();
		}
		else
		{
			//                                                   
			SiiMhlTxClrPathEn();
		}
	}

}
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
//
//
bool SiiMhlTxRcpSend( uint8_t rcpKeyCode )
{
	bool retVal;
	//
	//                                                               
	//

	if((0 == (MHL_FEATURE_RCP_SUPPORT & mhlTxConfig.mscFeatureFlag))
			||
			!(FLAGS_RCP_READY & mhlTxConfig.miscFlags)
	  )
	{
		retVal=false;
	}

	retVal=MhlTxSendMscMsg ( MHL_MSC_MSG_RCP, rcpKeyCode );
	if(retVal)
	{
		IncrementCBusReferenceCount(SiiMhlTxRcpSend)
			MhlTxDriveStates();
	}
	return retVal;
}

//                                                                             
//
//                 
//
//                                             
//
bool SiiMhlTxRcpkSend( uint8_t rcpKeyCode )
{
	bool	retVal;

	retVal = MhlTxSendMscMsg(MHL_MSC_MSG_RCPK, rcpKeyCode);
	if(retVal)
	{
		MhlTxDriveStates();
	}
	return retVal;
}

//                                                                             
//
//                 
//
//                                             
//
static	bool SiiMhlTxRapkSend( void )
{
	return	( MhlTxSendMscMsg ( MHL_MSC_MSG_RAPK, 0 ) );
}

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
bool SiiMhlTxRcpeSend( uint8_t rcpeErrorCode )
{
	bool	retVal;

	retVal = MhlTxSendMscMsg(MHL_MSC_MSG_RCPE, rcpeErrorCode);
	if(retVal)
	{
		MhlTxDriveStates();
	}
	return retVal;
}

static bool SiiMhlTxSetStatus( uint8_t regToWrite, uint8_t value )
{
	cbus_req_t	req;
	bool retVal;

	//                                 
	//         
	req.retryCount  = 2;
	req.command     = MHL_WRITE_STAT;
	req.offsetData  = regToWrite;
	req.payload_u.msgData[0]  = value;

	retVal = PutNextCBusTransaction(&req);
	return retVal;
}

//                                                                             
//
//                   
//
static bool SiiMhlTxSetDCapRdy( void )
{
	mhlTxConfig.connectedReady |= MHL_STATUS_DCAP_RDY;   //                  
	return SiiMhlTxSetStatus( MHL_STATUS_REG_CONNECTED_RDY, mhlTxConfig.connectedReady);
}

//                                                                             
//
//                   
//
/*                                      
   
                                                                           
                                                                                      
   
 */
//                                                                             
//
//                      
//
static bool SiiMhlTxSendLinkMode(void)
{
	return SiiMhlTxSetStatus( MHL_STATUS_REG_LINK_MODE, mhlTxConfig.linkMode);
}

//                                                                             
//
//                  
//
bool SiiMhlTxSetPathEn(void )
{
	SiiMhlTxTmdsEnable();
	mhlTxConfig.linkMode |= MHL_STATUS_PATH_ENABLED;     //                  
	return SiiMhlTxSetStatus( MHL_STATUS_REG_LINK_MODE, mhlTxConfig.linkMode);
}

//                                                                             
//
//                  
//
bool SiiMhlTxClrPathEn( void )
{
	SiiMhlTxDrvTmdsControl( false );
	mhlTxConfig.linkMode &= ~MHL_STATUS_PATH_ENABLED;    //                  
	return SiiMhlTxSetStatus( MHL_STATUS_REG_LINK_MODE, mhlTxConfig.linkMode);
}

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
bool SiiMhlTxReadDevcap( uint8_t offset )
{
	cbus_req_t	req;
	//
	//                             
	//
	req.retryCount  = 2;
	req.command     = MHL_READ_DEVCAP;
	req.offsetData  = offset;
	req.payload_u.msgData[0]  = 0;  //                           

	return PutNextCBusTransaction(&req);
}

/*
                                   
 */

static void SiiMhlTxRefreshPeerDevCapEntries(void)
{
	//                                                 
	if (mhlTxConfig.ucDevCapCacheIndex >= sizeof(mhlTxConfig.aucDevCapCache))
	{
		mhlTxConfig.ucDevCapCacheIndex=0;
		SiiMhlTxReadDevcap( mhlTxConfig.ucDevCapCacheIndex );
	}
}

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
static bool MhlTxSendMscMsg ( uint8_t command, uint8_t cmdData )
{
	cbus_req_t	req;
	uint8_t		ccode;

	//
	//                     
	//
	//                                                  
	//
	req.retryCount  = 2;
	req.command     = MHL_MSC_MSG;
	req.payload_u.msgData[0]  = command;
	req.payload_u.msgData[1]  = cmdData;
	ccode = PutNextCBusTransaction(&req);
	return( (bool) ccode );
}
//                                                                             
//
//                         
//
//
void	SiiMhlTxNotifyConnection( bool mhlConnected )
{
	mhlTxConfig.mhlConnectionEvent = true;

	if(mhlConnected)
	{
		mhlTxConfig.rapFlags |= RAP_CONTENT_ON;
		mhlTxConfig.mhlConnected = MHL_TX_EVENT_CONNECTION;
		mhlTxConfig.mhlHpdRSENflags |= MHL_RSEN;
		SiiMhlTxTmdsEnable();
		SiiMhlTxSendLinkMode();
		MhlTxProcessEvents();
	}
	else
	{
		mhlTxConfig.rapFlags &= ~RAP_CONTENT_ON;
		mhlTxConfig.mhlConnected = MHL_TX_EVENT_DISCONNECTION;
		mhlTxConfig.mhlHpdRSENflags &= ~MHL_RSEN;
	}
}
//                                                                             
//
//                          
//                                                                             
//
//                                                 
//
void	SiiMhlTxNotifyDsHpdChange( uint8_t dsHpdStatus )
{
	if( 0 == dsHpdStatus )
	{
		printk(KERN_INFO "%s: ~MHL_HPD\n", __func__);
		mhlTxConfig.mhlHpdRSENflags &= ~MHL_HPD;
		AppNotifyMhlDownStreamHPDStatusChange(dsHpdStatus);
		SiiMhlTxDrvTmdsControl( false );
	}
	else
	{
		printk(KERN_INFO "%s: MHL_HPD\n", __func__);
		mhlTxConfig.mhlHpdRSENflags |= MHL_HPD;
		AppNotifyMhlDownStreamHPDStatusChange(dsHpdStatus);
		SiiMhlTxTmdsEnable();
	}
}
//                                                                             
//
//                 
//
//                                                                          
//                                                                      
//
static void	MhlTxResetStates( void )
{
	mhlTxConfig.mhlConnectionEvent	= false;
	mhlTxConfig.mhlConnected		= MHL_TX_EVENT_DISCONNECTION;
	mhlTxConfig.mhlHpdRSENflags     &= ~(MHL_RSEN | MHL_HPD);
	mhlTxConfig.rapFlags 			&= ~RAP_CONTENT_ON;
	mhlTxConfig.mscMsgArrived		= false;

	mhlTxConfig.status_0            = 0;
	mhlTxConfig.status_1            = 0;
	mhlTxConfig.connectedReady      = 0;
	mhlTxConfig.linkMode            = MHL_STATUS_CLK_MODE_NORMAL; //                              
	mhlTxConfig.preferredClkMode	= MHL_STATUS_CLK_MODE_NORMAL;  //                                                                                    
	mhlTxConfig.cbusReferenceCount  = 0;
	mhlTxConfig.miscFlags           = 0;
	mhlTxConfig.mscLastCommand      = 0;
	mhlTxConfig.mscMsgLastCommand   = 0;
	mhlTxConfig.ucDevCapCacheIndex  = 1 + sizeof(mhlTxConfig.aucDevCapCache);
}

/*
                          
        
                         
                     
 */
uint8_t    SiiTxReadConnectionStatus(void)
{
	return (mhlTxConfig.mhlConnected >= MHL_TX_EVENT_RCP_READY)?1:0;
}

/*
                                

                                                                      

                     
                                                               
 */
uint8_t SiiMhlTxSetPreferredPixelFormat(uint8_t clkMode)
{
	if (~MHL_STATUS_CLK_MODE_MASK & clkMode)
	{
		return 1;
	}
	else
	{
		mhlTxConfig.preferredClkMode = clkMode;

		//                                                           
		//                        
		if (mhlTxConfig.ucDevCapCacheIndex <= sizeof(mhlTxConfig.aucDevCapCache))
		{
			//                                                                  
			if (mhlTxConfig.ucDevCapCacheIndex > DEVCAP_OFFSET_VID_LINK_MODE)
			{
				ExamineLocalAndPeerVidLinkMode();
			}
		}
		return 0;
	}
}

/*
                        
                                 
                                         
        
             
           
 */
uint8_t SiiTxGetPeerDevCapEntry(uint8_t index,uint8_t *pData)
{
	if (mhlTxConfig.ucDevCapCacheIndex < sizeof(mhlTxConfig.aucDevCapCache))
	{
		//                      
		return 1;
	}
	else
	{
		*pData = mhlTxConfig.aucDevCapCache[index];
		return 0;
	}
}

/*
                       
                                                                                 
                                         
                                                                          

        
                                                  

 */
ScratchPadStatus_e SiiGetScratchPadVector(uint8_t offset,uint8_t length, uint8_t *pData)
{
	if (!(MHL_FEATURE_SP_SUPPORT & mhlTxConfig.mscFeatureFlag))
	{
		return SCRATCHPAD_NOT_SUPPORTED;
	}
	else if (FLAGS_SCRATCHPAD_BUSY & mhlTxConfig.miscFlags)
	{
		return SCRATCHPAD_BUSY;
	}
	else if ((offset >= sizeof(mhlTxConfig.localScratchPad)) ||
			(length > (sizeof(mhlTxConfig.localScratchPad)- offset)) )
	{
		return SCRATCHPAD_BAD_PARAM;
	}
	else
	{
		uint8_t i,reg;
		for (i = 0,reg=offset; (i < length) && (reg < sizeof(mhlTxConfig.localScratchPad));++i,++reg)
		{
			pData[i] = mhlTxConfig.localScratchPad[reg];
		}
		return SCRATCHPAD_SUCCESS;
	}
}

void SiiMhlTxNotifyRgndMhl( void )
{
	if (MHL_TX_EVENT_STATUS_PASSTHROUGH == AppNotifyMhlEvent(MHL_TX_EVENT_RGND_MHL, 0))
	{
		//                                                                     
		SiiMhlTxDrvProcessRgndMhl();
	}
}

/*                                                                           */
static int32_t devMajor = 0;    /*                               */
static struct cdev siiMhlCdev;
static struct class *siiMhlClass;

/*                                                                            */

MHL_DRIVER_CONTEXT_T gDriverContext;

#if defined(DEBUG)
unsigned char DebugChannelMasks[SII_OSAL_DEBUG_NUM_CHANNELS/8+1]={0xFF,0xFF,0xFF,0xFF};
//                                    
module_param_array(DebugChannelMasks, byte, NULL, S_IRUGO | S_IWUSR);

ushort DebugFormat = SII_OS_DEBUG_FORMAT_FILEINFO;
module_param(DebugFormat, ushort, S_IRUGO | S_IWUSR);
#endif


/*                                                                           */

/* 
                                           
  
                                                                              
                                         
  
                                                              
  
                                                                             */
int32_t StartMhlTxDevice(void)
{
	printk(KERN_INFO "[MHL]Starting %s\n", MHL_PART_NAME);

	/*                                                        */
	if (down_interruptible(&sii8334_irq_sem) == 0) {
		SiiMhlTxInitialize(EVENT_POLL_INTERVAL_30_MS);
		up(&sii8334_irq_sem);
	}

	return 0;
}

/*                                                                           */

/* 
                                                                     
 */
ssize_t ShowConnectionState(struct device *dev, struct device_attribute *attr,
		char *buf)
{
	if(gDriverContext.flags & MHL_STATE_FLAG_CONNECTED) {
		return scnprintf(buf, PAGE_SIZE, "connected %s_ready",
				gDriverContext.flags & MHL_STATE_FLAG_RCP_READY? "rcp" : "not_rcp");
	} else {
		return scnprintf(buf, PAGE_SIZE, "not connected");
	}
}


/* 
                                                                
 */
ssize_t ShowRcp(struct device *dev, struct device_attribute *attr,
		char *buf)
{
	int		status = 0;

	if(down_interruptible(&sii8334_irq_sem) != 0)
		return -ERESTARTSYS;

	if(gDriverContext.flags &
			(MHL_STATE_FLAG_RCP_SENT | MHL_STATE_FLAG_RCP_RECEIVED))
	{
		status = scnprintf(buf, PAGE_SIZE, "0x%02x %s",
				gDriverContext.keyCode,
				gDriverContext.flags & MHL_STATE_FLAG_RCP_SENT? "sent" : "received");
	}

	up(&sii8334_irq_sem);
	return status;
}



/* 
                                                                 
 */
ssize_t SendRcp(struct device *dev, struct device_attribute *attr,
		const char *buf, size_t count)
{
	unsigned long	keyCode;
	int				status = -EINVAL;

	if(down_interruptible(&sii8334_irq_sem) != 0)
		return -ERESTARTSYS;

	while(gDriverContext.flags & MHL_STATE_FLAG_RCP_READY) {

		if(strict_strtoul(buf, 0, &keyCode)) {
			break;
		}

		if(keyCode >= 0xFE) {
			break;
		}

		gDriverContext.flags &= ~(MHL_STATE_FLAG_RCP_RECEIVED |
				MHL_STATE_FLAG_RCP_ACK |
				MHL_STATE_FLAG_RCP_NAK);
		gDriverContext.flags |= MHL_STATE_FLAG_RCP_SENT;
		gDriverContext.keyCode = (uint8_t)keyCode;
		SiiMhlTxRcpSend((uint8_t)keyCode);
		status = count;
		break;
	}

	up(&sii8334_irq_sem);

	return status;
}


/* 
                                                             
  
                                                                
                                          
  
                                           
                                              
                                                                             
                                                       
                                                                
 */
ssize_t SendRcpAck(struct device *dev, struct device_attribute *attr,
		const char *buf, size_t count)
{
	unsigned long	keyCode = 0x100;	//                               
	unsigned long	errCode = 0x100;
	char			*pStr;
	int				status = -EINVAL;

	//                                                                   
	do {
		pStr = strstr(buf, "keycode=");
		if(pStr != NULL) {
			if(strict_strtoul(pStr + 8, 0, &keyCode)) {
				break;
			}
		} else {
			break;
		}

		pStr = strstr(buf, "errorcode=");
		if(pStr != NULL) {
			if(strict_strtoul(pStr + 10, 0, &errCode)) {
				break;
			}
		} else
			break;

	} while(false);

	if((keyCode > 0xFF) || (errCode > 0xFF))
		return status;

	if(down_interruptible(&sii8334_irq_sem) != 0)
		return -ERESTARTSYS;

	while(gDriverContext.flags & MHL_STATE_FLAG_RCP_READY) {

		if((keyCode != gDriverContext.keyCode)
				|| !(gDriverContext.flags & MHL_STATE_FLAG_RCP_RECEIVED))
			break;

		if(errCode == 0) {
			SiiMhlTxRcpkSend((uint8_t)keyCode);
		} else {
			SiiMhlTxRcpeSend((uint8_t)errCode);
		}

		status = count;
		break;
	}

	up(&sii8334_irq_sem);

	return status;
}



/* 
                                                            
  
                                                              
                                     
  
                                               
                                              
                                                                           
                                          
                                                          
                                                           
 */
ssize_t ShowRcpAck(struct device *dev, struct device_attribute *attr,
		char *buf)
{
	int				status = -EINVAL;

	if(down_interruptible(&sii8334_irq_sem) != 0)
		return -ERESTARTSYS;

	if(gDriverContext.flags & (MHL_STATE_FLAG_RCP_ACK | MHL_STATE_FLAG_RCP_NAK)) {

		status = scnprintf(buf, PAGE_SIZE, "keycode=0x%02x errorcode=0x%02x",
				gDriverContext.keyCode, gDriverContext.errCode);
	}

	up(&sii8334_irq_sem);

	return status;
}



/* 
                                                            
  
                                                                       
                                                                   
                  
  
                                                                  
                         
 */
ssize_t SelectDevCap(struct device *dev, struct device_attribute *attr,
		const char *buf, size_t count)
{
	unsigned long	devCapOffset;
	int				status = -EINVAL;

	do {

		if (strict_strtoul(buf, 0, &devCapOffset))
			break;

		if (devCapOffset >= 0x0F)
			break;

		gDriverContext.devCapOffset = (uint8_t)devCapOffset;

		status = count;

	} while(false);

	return status;
}



/* 
                                                           
  
                                                                       
                                                          
  
                                                                      
                                                                         
                                                                      
        
  
                                               
                               
                                                                 
                             
                                                                     
                            
 */
ssize_t ReadDevCap(struct device *dev, struct device_attribute *attr,
		char *buf)
{
	uint8_t		regValue;
	int			status = -EINVAL;

	if(down_interruptible(&sii8334_irq_sem) != 0)
		return -ERESTARTSYS;

	do {
		if(gDriverContext.flags & MHL_STATE_FLAG_CONNECTED) {

			status = SiiTxGetPeerDevCapEntry(gDriverContext.devCapOffset,
					&regValue);
			if(status != 0) {
				//                                                       
				//                                                       
				//                 
				status = -EAGAIN;
				break;
			}
			status = scnprintf(buf, PAGE_SIZE, "offset:0x%02x=0x%02x",
					gDriverContext.devCapOffset, regValue);
		}
	} while(false);

	up(&sii8334_irq_sem);

	return status;
}



#define MAX_EVENT_STRING_LEN 40
/*                                                                           */
/* 
                                                                     
                         
  
                                                                             */
void  AppNotifyMhlDownStreamHPDStatusChange(bool connected)
{
	char	event_string[MAX_EVENT_STRING_LEN];
	char	*envp[] = {event_string, NULL};

	snprintf(event_string, MAX_EVENT_STRING_LEN, "MHLEVENT=%s",
			connected? "HPD" : "NO_HPD");
	kobject_uevent_env(&gDriverContext.pDevice->kobj,
			KOBJ_CHANGE, envp);
}



/*                                                                           */
/* 
                                                                           
  
                                                                             */
MhlTxNotifyEventsStatus_e  AppNotifyMhlEvent(uint8_t eventCode, uint8_t eventParam)
{
	char						event_string[MAX_EVENT_STRING_LEN];
	char						*envp[] = {event_string, NULL};
	MhlTxNotifyEventsStatus_e	rtnStatus = MHL_TX_EVENT_STATUS_PASSTHROUGH;
	char rcp_buf[MAX_EVENT_STRING_LEN];

	//                                                                     
	//                                                                    
	//                                                                         
	//                     
	gDriverContext.pendingEvent = eventCode;
	gDriverContext.pendingEventData = eventParam;

	pr_info("%s: eventCode[%d]\n",__func__,eventCode);

	switch(eventCode) {

		case MHL_TX_EVENT_CONNECTION:
			gDriverContext.flags |= MHL_STATE_FLAG_CONNECTED;
			strncpy(event_string, "MHLEVENT=connected", MAX_EVENT_STRING_LEN);
			kobject_uevent_env(&gDriverContext.pDevice->kobj,
					KOBJ_CHANGE, envp);
#ifdef CONFIG_MACH_LGE
			/*                                  */
			pr_info("%s : HDMI HPD on when MHL is connected \n",__func__);
			if(mhl_common_state->hdmi_hpd_on)
				mhl_common_state->hdmi_hpd_on(1);
#endif
			break;

		case MHL_TX_EVENT_RCP_READY:
			gDriverContext.flags |= MHL_STATE_FLAG_RCP_READY;
			strncpy(event_string, "MHLEVENT=rcp_ready", MAX_EVENT_STRING_LEN);
			kobject_uevent_env(&gDriverContext.pDevice->kobj,
					KOBJ_CHANGE, envp);
			break;

		case MHL_TX_EVENT_DISCONNECTION:
			gDriverContext.flags = 0;
			gDriverContext.keyCode = 0;
			gDriverContext.errCode = 0;
			strncpy(event_string, "MHLEVENT=disconnected", MAX_EVENT_STRING_LEN);
			kobject_uevent_env(&gDriverContext.pDevice->kobj,
					KOBJ_CHANGE, envp);

#ifdef CONFIG_MACH_LGE
			/*                                       */
			pr_info("%s : HDMI HPD off when MHL is disconnected \n",__func__);
			if(mhl_common_state->hdmi_hpd_on)
				mhl_common_state->hdmi_hpd_on(0);
#endif
			break;

		case MHL_TX_EVENT_RCP_RECEIVED:
			gDriverContext.flags &= ~MHL_STATE_FLAG_RCP_SENT;
			gDriverContext.flags |= MHL_STATE_FLAG_RCP_RECEIVED;
			gDriverContext.keyCode = eventParam;
			snprintf(event_string, MAX_EVENT_STRING_LEN,
					"MHLEVENT=received_RCP key code=0x%02x", eventParam);
			sprintf(rcp_buf, "MHL_RCP=%d", eventParam);
			kobject_uevent_env(&gDriverContext.pDevice->kobj,
					KOBJ_CHANGE, envp);
#ifdef CONFIG_MACH_LGE
			pr_info("[MHL_RCP] %s\n",rcp_buf);
			if(mhl_common_state->send_uevent)
				mhl_common_state->send_uevent(rcp_buf);
#endif
			break;

		case MHL_TX_EVENT_RCPK_RECEIVED:
			if((gDriverContext.flags & MHL_STATE_FLAG_RCP_SENT)
					&& (gDriverContext.keyCode == eventParam)) {

				gDriverContext.flags |= MHL_STATE_FLAG_RCP_ACK;

				snprintf(event_string, MAX_EVENT_STRING_LEN,
						"MHLEVENT=received_RCPK key code=0x%02x", eventParam);
				sprintf(rcp_buf, "MHL_RCPK=%d", eventParam);
				kobject_uevent_env(&gDriverContext.pDevice->kobj,
						KOBJ_CHANGE, envp);
#ifdef CONFIG_MACH_LGE
				if(mhl_common_state->send_uevent)
					mhl_common_state->send_uevent(rcp_buf);
#endif
			}
			break;

		case MHL_TX_EVENT_RCPE_RECEIVED:
			if(gDriverContext.flags & MHL_STATE_FLAG_RCP_SENT) {

				gDriverContext.errCode = eventParam;
				gDriverContext.flags |= MHL_STATE_FLAG_RCP_NAK;

				snprintf(event_string, MAX_EVENT_STRING_LEN,
						"MHLEVENT=received_RCPE error code=0x%02x", eventParam);
				sprintf(rcp_buf, "MHL_RCPE=%d", eventParam);
				kobject_uevent_env(&gDriverContext.pDevice->kobj,
						KOBJ_CHANGE, envp);
#ifdef CONFIG_MACH_LGE
				if(mhl_common_state->send_uevent)
					mhl_common_state->send_uevent(rcp_buf);
#endif
			}
			break;

		case MHL_TX_EVENT_DCAP_CHG:
			snprintf(event_string, MAX_EVENT_STRING_LEN,
					"MHLEVENT=DEVCAP change");
			kobject_uevent_env(&gDriverContext.pDevice->kobj,
					KOBJ_CHANGE, envp);

			break;

		case MHL_TX_EVENT_DSCR_CHG:	//                                   
			snprintf(event_string, MAX_EVENT_STRING_LEN,
					"MHLEVENT=SCRATCHPAD change");
			kobject_uevent_env(&gDriverContext.pDevice->kobj,
					KOBJ_CHANGE, envp);
			break;

		case MHL_TX_EVENT_POW_BIT_CHG:	//                                    
			if(eventParam) {
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
				snprintf(event_string, MAX_EVENT_STRING_LEN,
						"MHLEVENT=MHL VBUS power OFF");

			} else {
				snprintf(event_string, MAX_EVENT_STRING_LEN,
						"MHLEVENT=MHL VBUS power ON");
			}

			kobject_uevent_env(&gDriverContext.pDevice->kobj,
					KOBJ_CHANGE, envp);
			break;

		case MHL_TX_EVENT_RGND_MHL:
			//                                                               
			//                                                               
			//                                                              
			//                                                           
			//                                                         
			rtnStatus = MHL_TX_EVENT_STATUS_HANDLED;
			break;
			//                                                              
			//                                                                 
			//                                          
			snprintf(event_string, MAX_EVENT_STRING_LEN,
					"MHLEVENT=MHL device detected");
			kobject_uevent_env(&gDriverContext.pDevice->kobj,
					KOBJ_CHANGE, envp);
			break;
	}
	return rtnStatus;
}

static irqreturn_t sii8334_mhl_tx_irq(int irq, void *handle)
{
	printk(KERN_INFO "%s\n",__func__);
	/*                */
	MHLSleepStatus = 0;

	if (down_interruptible(&sii8334_irq_sem) == 0) {
		SiiMhlTxDeviceIsr();
		up(&sii8334_irq_sem);
	}

	return IRQ_HANDLED;
}

/*                                     */
static bool	bTxOpen = false;
int32_t SiiMhlOpen(struct inode *pInode, struct file *pFile)
{

	if(bTxOpen)
		return -EBUSY;

	bTxOpen = true;		//                            
	return 0;
}

int32_t SiiMhlRelease(struct inode *pInode, struct file *pFile)
{
	bTxOpen = false;	//                            

	return 0;
}

long SiiMhlIoctl(struct file *pFile, unsigned int ioctlCode,
		unsigned long ioctlParam)
{
	mhlTxEventPacket_t		mhlTxEventPacket;
	mhlTxReadDevCap_t		mhlTxReadDevCap;
	mhlTxScratchPadAccess_t	mhlTxScratchPadAccess;
	ScratchPadStatus_e		scratchPadStatus;
	long					retStatus = 0;
	bool					status;

	if (down_interruptible(&sii8334_irq_sem) != 0)
		return -ERESTARTSYS;

	switch (ioctlCode)
	{
		case SII_MHL_GET_MHL_TX_EVENT:

			mhlTxEventPacket.event = gDriverContext.pendingEvent;
			mhlTxEventPacket.eventParam = gDriverContext.pendingEventData;

			if(copy_to_user((mhlTxEventPacket_t*)ioctlParam,
						&mhlTxEventPacket, sizeof(mhlTxEventPacket_t))) {
				retStatus = -EFAULT;
				break;
			}

			gDriverContext.pendingEvent = MHL_TX_EVENT_NONE;
			gDriverContext.pendingEventData = 0;
			break;

		case SII_MHL_RCP_SEND:

			gDriverContext.flags &= ~(MHL_STATE_FLAG_RCP_RECEIVED |
					MHL_STATE_FLAG_RCP_ACK |
					MHL_STATE_FLAG_RCP_NAK);
			gDriverContext.flags |= MHL_STATE_FLAG_RCP_SENT;
			gDriverContext.keyCode = (uint8_t)ioctlParam;
			status = SiiMhlTxRcpSend((uint8_t)ioctlParam);
			break;

		case SII_MHL_RCP_SEND_ACK:

			status = SiiMhlTxRcpkSend((uint8_t)ioctlParam);
			break;

		case SII_MHL_RCP_SEND_ERR_ACK:

			status = SiiMhlTxRcpeSend((uint8_t)ioctlParam);
			break;

		case SII_MHL_GET_MHL_CONNECTION_STATUS:

			if(gDriverContext.flags & MHL_STATE_FLAG_RCP_READY)
				mhlTxEventPacket.event = MHL_TX_EVENT_RCP_READY;

			else if(gDriverContext.flags & MHL_STATE_FLAG_CONNECTED)
				mhlTxEventPacket.event = MHL_TX_EVENT_CONNECTION;

			else
				mhlTxEventPacket.event = MHL_TX_EVENT_DISCONNECTION;

			mhlTxEventPacket.eventParam = 0;

			if(copy_to_user((mhlTxEventPacket_t*)ioctlParam,
						&mhlTxEventPacket, sizeof(mhlTxEventPacket_t))) {
				retStatus = -EFAULT;
			}
			break;

		case SII_MHL_GET_DEV_CAP_VALUE:

			if(copy_from_user(&mhlTxReadDevCap, (mhlTxReadDevCap_t*)ioctlParam,
						sizeof(mhlTxReadDevCap_t))) {
				retStatus = -EFAULT;
				break;
			}

			//                                                            
			//                                 
			if(!(gDriverContext.flags & MHL_STATE_FLAG_RCP_READY) ||
					(mhlTxReadDevCap.regNum > 0x0F)) {
				retStatus = -EINVAL;
				break;
			}

			retStatus = SiiTxGetPeerDevCapEntry(mhlTxReadDevCap.regNum,
					&mhlTxReadDevCap.regValue);
			if(retStatus != 0) {
				//                                                       
				//                                                       
				//                 
				retStatus = -EAGAIN;
				break;
			}

			if(copy_to_user((mhlTxReadDevCap_t*)ioctlParam,
						&mhlTxReadDevCap, sizeof(mhlTxReadDevCap_t))) {
				retStatus = -EFAULT;
			}
			break;

		case SII_MHL_WRITE_SCRATCH_PAD:

			if(copy_from_user(&mhlTxScratchPadAccess,
						(mhlTxScratchPadAccess_t*)ioctlParam,
						sizeof(mhlTxScratchPadAccess_t))) {
				retStatus = -EFAULT;
				break;
			}

			//                                                            
			//                                                           
			//                                                           
			//                                                               
			//                                                 
			if(!(gDriverContext.flags & MHL_STATE_FLAG_RCP_READY) ||
					(mhlTxScratchPadAccess.length < ADOPTER_ID_SIZE) ||
					(mhlTxScratchPadAccess.length > MAX_SCRATCH_PAD_TRANSFER_SIZE) ||
					(mhlTxScratchPadAccess.offset >
					 (MAX_SCRATCH_PAD_TRANSFER_SIZE - ADOPTER_ID_SIZE)) ||
					(mhlTxScratchPadAccess.offset + mhlTxScratchPadAccess.length >
					 MAX_SCRATCH_PAD_TRANSFER_SIZE)) {
				retStatus = -EINVAL;
				break;
			}

			scratchPadStatus =  SiiMhlTxRequestWriteBurst(mhlTxScratchPadAccess.offset,
					mhlTxScratchPadAccess.length,
					mhlTxScratchPadAccess.data);
			switch(scratchPadStatus) {
				case SCRATCHPAD_SUCCESS:
					break;

				case SCRATCHPAD_BUSY:
					retStatus = -EAGAIN;
					break;

				default:
					retStatus = -EFAULT;
					break;
			}
			break;

		case SII_MHL_READ_SCRATCH_PAD:

			if(copy_from_user(&mhlTxScratchPadAccess,
						(mhlTxScratchPadAccess_t*)ioctlParam,
						sizeof(mhlTxScratchPadAccess_t))) {
				retStatus = -EFAULT;
				break;
			}

			//                                                            
			//                                                           
			//                                                           
			//                                                               
			//                                                 
			if(!(gDriverContext.flags & MHL_STATE_FLAG_RCP_READY) ||
					(mhlTxScratchPadAccess.length > MAX_SCRATCH_PAD_TRANSFER_SIZE) ||
					(mhlTxScratchPadAccess.offset >= MAX_SCRATCH_PAD_TRANSFER_SIZE) ||
					(mhlTxScratchPadAccess.offset + mhlTxScratchPadAccess.length >
					 MAX_SCRATCH_PAD_TRANSFER_SIZE)) {
				retStatus = -EINVAL;
				break;
			}

			scratchPadStatus =  SiiGetScratchPadVector(mhlTxScratchPadAccess.offset,
					mhlTxScratchPadAccess.length,
					mhlTxScratchPadAccess.data);
			switch(scratchPadStatus) {
				case SCRATCHPAD_SUCCESS:
					break;

				case SCRATCHPAD_BUSY:
					retStatus = -EAGAIN;
					break;
				default:
					retStatus = -EFAULT;
					break;
			}
			break;

		default:
			retStatus = -ENOTTY;
	}

	up(&sii8334_irq_sem);

	return retStatus;
}

#ifdef CONFIG_MACH_LGE

#ifdef CONFIG_LG_MAGIC_MOTION_REMOCON

void SiiMhlTxReadScratchpad(void)
{
	int m_idx = 0;
	//                                          
	memset(mhlTxConfig.mscScratchpadData, 0x00, MHD_SCRATCHPAD_SIZE);

	for(m_idx=0; m_idx<(MHD_SCRATCHPAD_SIZE/2); m_idx++)
	{
		mhlTxConfig.mscScratchpadData[m_idx] = SiiRegRead( TX_PAGE_CBUS |( 0xC0+m_idx));
		//                                                                                                  
	}

	MhlControl();
}

void SiiMhlTxWriteScratchpad(uint8_t *wdata)
{
	int i;

	SiiRegWrite(REG_CBUS_PRI_START,0xff);
	SiiRegWrite(REG_CBUS_PRI_ADDR_CMD, 0x40);

	SiiRegWrite(REG_MSC_WRITE_BURST_LEN, MHD_MAX_BUFFER_SIZE -1 );

	for ( i = 0; i < MHD_MAX_BUFFER_SIZE; i++ )
	{
		SiiRegWrite(REG_CBUS_SCRATCHPAD_0 + i,wdata[i] );
	}
	SiiRegWrite(REG_CBUS_PRI_START, MSC_START_BIT_WRITE_BURST );
}

int mhl_ms_ptr(signed short x, signed short y)	//                                     
{
	return 0;
}
void mhl_ms_hide_cursor(void)	//                          
{

}
int mhl_ms_btn(int action)		//                                                       
{
	return 0;
}
int mhl_kbd_key(unsigned int tKeyCode, int value)	//                                          
{
	char mbuf[120];
	memset(mbuf, 0, sizeof(mbuf));
	sprintf(mbuf, "MHL_KEY press=%04d, keycode=%04d", value, tKeyCode);
	pr_info("[MAGIC] mhl_kbd_key %s\n",mbuf);

	if(mhl_common_state->send_uevent)
		mhl_common_state->send_uevent(mbuf);

	return 0;
}
void mhl_writeburst_uevent(unsigned short mev)
{
	char env_buf[120];

	memset(env_buf, 0, sizeof(env_buf));

	switch(mev)
	{
		case 1:
			sprintf(env_buf, "hdmi_kbd_on");
			break;

		case 2:
			sprintf(env_buf, "hdmi_kbd_off");
			break;

		default:

			break;
	}

	if(mhl_common_state->send_uevent)
		mhl_common_state->send_uevent(env_buf);

	pr_info("[MAGIC] mhl_writeburst_uevent %s\n",env_buf);
	return;
}


void MhlControl(void)
{
	uint8_t category = 0;
	uint8_t command = 0;
	uint8_t action = 0;
	uint8_t r_action = 0;
	int x_position = 0;
	int y_position = 0;
	unsigned short keyCode = 0;
	uint8_t plugResp[MHD_MAX_BUFFER_SIZE];
	signed char r_lval = 0;
	signed char r_uval = 0;
	int fdr = 3;
	int tmp_x = 0;
	int tmp_y = 0;
	char mbuf[120];

	static uint8_t pre_r_action = 0;
	static int pre_x_position = 0;
	static int pre_y_position = 0;

	category = mhlTxConfig.mscScratchpadData[0];

	if(category == 0x01)
	{
		command =  mhlTxConfig.mscScratchpadData[1];
		if(command == 0x01)			//                                                            
		{
			action = mhlTxConfig.mscScratchpadData[2];
			x_position = (int)((mhlTxConfig.mscScratchpadData[3] <<8) | mhlTxConfig.mscScratchpadData[4]);
			y_position = (int)((mhlTxConfig.mscScratchpadData[5] <<8) | mhlTxConfig.mscScratchpadData[6]);

			if((0 < tvCtl_x) && (0 < tvCtl_y))
			{
				x_position = x_position*MAGIC_CANVAS_X/tvCtl_x;
				y_position = y_position*MAGIC_CANVAS_Y/tvCtl_y;

				if(x_position>MAGIC_CANVAS_X) x_position = MAGIC_CANVAS_X;
				if(y_position>MAGIC_CANVAS_Y) y_position = MAGIC_CANVAS_Y;
			}

			if(action == 0x01)	r_action = 1;
			else if(action == 0x02) r_action = 0;
			else return;

			if(pre_r_action == r_action && pre_x_position == x_position && pre_y_position == y_position){
				pr_info("%s ->magic RC, double input, so remove this info. ",__func__);
				return;
			}
			else{
				pre_r_action = r_action;
				pre_x_position = x_position;
				pre_y_position = y_position;
			}

	

			memset(mbuf, 0, sizeof(mbuf));
			pr_info("%s ->magic RC, action:%d, (%d,%d)\n",__func__,r_action,x_position,y_position);
			sprintf(mbuf, "MHL_CTL action=%04d, x_pos=%04d, y_pos=%04d", r_action,x_position,y_position);
			if(mhl_common_state->send_uevent)
				mhl_common_state->send_uevent(mbuf);
			

		}
		else if(command == 0x02)		//                                          
		{
			action = mhlTxConfig.mscScratchpadData[2];

			r_lval = 0;
			r_uval = 0;
			r_lval = (signed char) mhlTxConfig.mscScratchpadData[4];
			r_uval = (signed char) mhlTxConfig.mscScratchpadData[3];
			x_position = (int)((r_uval<<8) | r_lval);

			r_lval = 0;
			r_uval = 0;
			r_lval = (signed char) mhlTxConfig.mscScratchpadData[6];
			r_uval = (signed char) mhlTxConfig.mscScratchpadData[5];
			y_position = (int)((r_uval<<8) | r_lval);

			pr_info("%s -> mouse, action:%d, (%d,%d) \n",__func__, action,x_position,y_position);

			if(((x_position> 100) || (x_position< -100)) ||((y_position> 100) || (y_position< -100)))
				fdr = 5;
			else if(((-30 < x_position) && (x_position < 30)) && ((-30 < y_position) && (y_position < 30)))
				fdr = 1;
			else
				fdr = 3;

			fdr = 7;
			x_position = fdr*x_position;
			y_position = fdr*y_position;

			if(action == 0x00)	//       
			{
				mhl_ms_ptr(x_position, y_position);
				return;
			}

			if((action == 0x03)) r_action = 1;
			else if((action == 0x02)) r_action = 0;
			else return;

			mhl_ms_ptr(x_position, y_position);
			mhl_ms_btn(r_action);
		}
		else if(command == 0x03)		//                                              
		{
			action = mhlTxConfig.mscScratchpadData[2];
			keyCode = ((mhlTxConfig.mscScratchpadData[3] <<8) | mhlTxConfig.mscScratchpadData[4]);

			pr_info("%s ->magic RC, action:%d, keyCode:%d \n",__func__,r_action,keyCode);

			if((keyCode == KEY_F11) || (keyCode == KEY_F12))	//                        
			{
				char mbuf[120];

				if(action == 0x01)
				{
					if(keyCode == KEY_F11) r_action = 1;
					else r_action = 0;

					x_position = 9999;
					y_position = 9999;
					memset(mbuf, 0, sizeof(mbuf));
					pr_info("%s ->magic RC, action:%d, (%d,%d) \n",__func__,r_action,x_position,y_position);
					sprintf(mbuf, "MHL_CTL action=%04d, x_pos=%04d, y_pos=%04d", r_action,x_position,y_position);

					if(mhl_common_state->send_uevent)
						mhl_common_state->send_uevent(mbuf);
				}
				return;
			}

			if(action == 0x01)
			{
				r_action = 1;
				if((kbd_key_pressed[0]>>8) == 0x00)
				{
					if(((kbd_key_pressed[1] >>8) == 0x01) && ((kbd_key_pressed[1]&0xFF) == keyCode))
					{
						pr_info("%s -> keyboard, duplicated pressed, %x \n",__func__, keyCode);
						return;
					}
					kbd_key_pressed[0] = (0x0100 | keyCode);
				}
				else
				{
					if((kbd_key_pressed[1] >>8) == 0x01)
					{
						pr_info("%s -> keyboard, duplicated pressed, %x \n",__func__, keyCode);
						return;
					}
					kbd_key_pressed[1] = (0x0100 | keyCode);
				}

				/*                               
         
                                                                              
               
         
            
         
                               
         
     */
			}
			else if(action == 0x02)
			{
				r_action = 0;

				if(((kbd_key_pressed[0]>>8) == 0x01) && ((kbd_key_pressed[0]&0xFF) == keyCode))
				{
					kbd_key_pressed[0] = 0x0000;
					mhl_kbd_key(keyCode,r_action);
					return;
				}
				if(((kbd_key_pressed[1]>>8) == 0x01) && ((kbd_key_pressed[1]&0xFF) == keyCode))
				{
					kbd_key_pressed[1] = 0x0000;
					mhl_kbd_key(keyCode,r_action);
					return;
				}

				pr_info("%s -> keyboard, duplicated released, %x \n",__func__, keyCode);
				return;

				/*                               
         
                                                                               
               
         
            
         
                               
         
     */
			}
			else return;

			mhl_kbd_key(keyCode,r_action);
		}
		else
		{
			pr_info("MhlControl : not defined message\n");
		}
	}
	else if(category == 0x02)
	{
		command =  mhlTxConfig.mscScratchpadData[1];
		action = mhlTxConfig.mscScratchpadData[2];

		if(command > 0x03)
		{
			pr_info("MhlControl : not defined message\n");
			return;
		}

		if(command == 0x01)	//                                  
		{
			if(action == 0x01)
			{
				tmp_x = (int)((mhlTxConfig.mscScratchpadData[3] <<8) | mhlTxConfig.mscScratchpadData[4]);
				tmp_y = (int)((mhlTxConfig.mscScratchpadData[5] <<8) | mhlTxConfig.mscScratchpadData[6]);

				pr_info("%s -> TV control canvas (%d, %d) \n",__func__, tmp_x, tmp_y);
				if((((MAGIC_CANVAS_X/2) < tmp_x) && (tmp_x < 2*MAGIC_CANVAS_X))
						&& (((MAGIC_CANVAS_Y/2) < tmp_y) && (tmp_y < 2*MAGIC_CANVAS_Y)))
				{
					tvCtl_x = tmp_x;
					tvCtl_y = tmp_y;
				}
				else
				{
					tvCtl_x = MAGIC_CANVAS_X;
					tvCtl_y = MAGIC_CANVAS_Y;
				}
			}
		}
		else if(command == 0x02)	//      
		{
			if(action == 0x02)
			{
				mhl_ms_hide_cursor();
				pr_info("%s -> mouse UNPLUGGED.\n",__func__);
			}
			else;
		}
		else if(command == 0x03)	//         
		{
			kbd_key_pressed[0] = 0x0000;
			kbd_key_pressed[1] = 0x0000;

			if(action == 0x01)
			{
				mhl_writeburst_uevent(1);
				pr_info("%s -> keyboard PLUGGED.\n",__func__);
			}
			else if(action == 0x02)
			{
				mhl_writeburst_uevent(2);
				pr_info("%s -> keyboard UNPLUGGED.\n",__func__);
			}
			else;
		}
		else;

		memcpy(plugResp, mhlTxConfig.mscScratchpadData, MHD_MAX_BUFFER_SIZE);
		plugResp[3] = 0x01;
		plugResp[4] = 0x00;
		plugResp[5] = 0x00;
		plugResp[6] = 0x00;
		SiiMhlTxWriteScratchpad(plugResp);
	}
	else
	{
		pr_info("MhlControl : not defined message\n");
		return;
	}
}

#endif /*                              */

#endif /*                */

/* 
                                               
 */
static const struct file_operations siiMhlFops = {
	.owner			= THIS_MODULE,
	.open			= SiiMhlOpen,
	.release		= SiiMhlRelease,
	.unlocked_ioctl	= SiiMhlIoctl
};

/*
                                                  
 */
struct device_attribute driver_attribs[] = {
	__ATTR(connection_state, 0444, ShowConnectionState, NULL),
	__ATTR(rcp_keycode, 0644, ShowRcp, SendRcp),
	__ATTR(rcp_ack, 0644, ShowRcpAck, SendRcpAck),
	__ATTR(devcap, 0644, ReadDevCap, SelectDevCap),
	__ATTR_NULL
};

int mhl_get_i2c_index(int addr)
{
	switch (addr) {
		case 0x39:
		case 0x72:
			return 0;
		case 0x3D:
		case 0x7A:
			return 1;
		case 0x49:
		case 0x92:
			return 2;
		case 0x4D:
		case 0x9A:
			return 3;
		case 0x64:
		case 0xC8:
			return 4;
	}

	return 0;
}

static int32_t sii8334_mhl_tx_probe(struct i2c_client *client,
		const struct i2c_device_id *id)
{
	int32_t	ret = 0;
	dev_t dev_no;

	printk(KERN_INFO "[MHL]%s\n",__func__);

	sii8334_mhl_i2c_client[mhl_get_i2c_index(client->addr)] = client;
	mhl_pdata[mhl_get_i2c_index(client->addr)] = client->dev.platform_data;

	++sii8334_i2c_dev_index;
	if (sii8334_i2c_dev_index < SII8334_I2C_DEVICE_NUMBER)
		return ret;

	/*                                                           
                                       
  */
	if (devMajor) {
		dev_no = MKDEV(devMajor, 0);
		ret = register_chrdev_region(dev_no, MHL_DRIVER_MINOR_MAX,
				MHL_DRIVER_NAME);
	} else {
		ret = alloc_chrdev_region(&dev_no, 0, MHL_DRIVER_MINOR_MAX,
				MHL_DRIVER_NAME);
		devMajor = MAJOR(dev_no);
	}

	if (ret) {
		printk(KERN_ERR "register_chrdev %d, %s failed, error code: %d\n",
				devMajor, MHL_DRIVER_NAME, ret);
		return ret;
	}

	cdev_init(&siiMhlCdev, &siiMhlFops);
	siiMhlCdev.owner = THIS_MODULE;
	ret = cdev_add(&siiMhlCdev, dev_no, MHL_DRIVER_MINOR_MAX);
	if (ret) {
		printk(KERN_ERR "cdev_add %s failed %d\n", MHL_DRIVER_NAME, ret);
		goto free_chrdev;
	}

	siiMhlClass = class_create(THIS_MODULE, "mhl");
	if (IS_ERR(siiMhlClass)) {
		printk(KERN_ERR "class_create failed %d\n", ret);
		ret = PTR_ERR(siiMhlClass);
		goto free_cdev;
	}

	siiMhlClass->dev_attrs = driver_attribs;

	gDriverContext.pDevice = device_create(siiMhlClass, NULL,
			MKDEV(devMajor, 0), NULL, "%s", MHL_DEVICE_NAME);
	if (IS_ERR(gDriverContext.pDevice)) {
		printk(KERN_ERR "class_device_create failed %s %d\n", MHL_DEVICE_NAME, ret);
		ret = PTR_ERR(gDriverContext.pDevice);
		goto free_class;
	}

#ifdef CONFIG_MACH_LGE
	mhl_common_state =  (struct mhl_common_type *)kmalloc(sizeof(
				struct mhl_common_type), GFP_KERNEL);
	if(!mhl_common_state){
		pr_err("%s: memory allocation fail\n",__func__);
		ret = -ENOMEM;
		goto error;
	}

	mhl_common_state->hdmi_hpd_on = hdmi_common_set_hpd_on;
	mhl_common_state->send_uevent = hdmi_common_send_uevent;

#endif	/*                 */

	sema_init(&sii8334_irq_sem, 1);

	ret = request_threaded_irq(sii8334_mhl_i2c_client[0]->irq, NULL,
			sii8334_mhl_tx_irq, IRQF_TRIGGER_LOW | IRQF_ONESHOT, MHL_PART_NAME,
			NULL);

	/*                                   */
	mhl_pdata[0]->power(1, 0);

	ret = StartMhlTxDevice();
	if(ret == 0)
		return 0;
	else
		device_destroy(siiMhlClass, MKDEV(devMajor, 0));


free_class:
	class_destroy(siiMhlClass);

free_cdev:
	cdev_del(&siiMhlCdev);

free_chrdev:
	unregister_chrdev_region(MKDEV(devMajor, 0), MHL_DRIVER_MINOR_MAX);
error:
	return ret;
}

static int32_t sii8334_mhl_tx_remove(struct i2c_client *client)
{
	sii8334_mhl_i2c_client[mhl_get_i2c_index(client->addr)] = NULL;

	device_destroy(siiMhlClass, MKDEV(devMajor, 0));
	class_destroy(siiMhlClass);
	unregister_chrdev_region(MKDEV(devMajor, 0), MHL_DRIVER_MINOR_MAX);

#ifdef CONFIG_MACH_LGE
	if(mhl_common_state){
		kfree(mhl_common_state);
		mhl_common_state = NULL;
	}
#endif

	return 0;
}

#ifdef CONFIG_MACH_LGE
static int32_t sii8334_mhl_tx_suspend(struct i2c_client *client, pm_message_t mesg)
{
	if (sii8334_mhl_suspended == true)
		return 0;
	sii8334_mhl_suspended = true;
	printk(KERN_INFO "[MHL]%s\n", __func__);
#if 0 /*              */
	mhl_pdata[0]->power(0, 1);
#endif

	disable_irq(sii8334_mhl_i2c_client[0]->irq);
	
	return 0;
}

static int32_t sii8334_mhl_tx_resume(struct i2c_client *client)
{
#if 0 /*               */
    int ret;
#endif
    if (sii8334_mhl_suspended == false)
        return 0;

	sii8334_mhl_suspended = false;
	printk(KERN_INFO "[MHL]%s\n", __func__);
#if 0 /*              */
	ret = mhl_pdata[0]->power(1, 1);

	if (ret > -1)
		StartMhlTxDevice();
#endif

	enable_irq(sii8334_mhl_i2c_client[0]->irq);

	return 0;
}
#endif

static const struct i2c_device_id sii8334_mhl_tx_id[] = {
	{ MHL_PART_NAME, 0 },
	{ }
};

static struct i2c_driver sii8334_mhl_tx_i2c_driver = {
	.driver = {
		.name = MHL_DRIVER_NAME,
	},
	.probe = sii8334_mhl_tx_probe,
	.remove = sii8334_mhl_tx_remove,
#ifdef CONFIG_MACH_LGE
	.suspend = sii8334_mhl_tx_suspend,
	.resume = sii8334_mhl_tx_resume,
#endif
	.id_table = sii8334_mhl_tx_id,
};

static int __init sii8334_mhl_tx_init(void)
{
	return i2c_add_driver(&sii8334_mhl_tx_i2c_driver);
}

static void __exit sii8334_mhl_tx_exit(void)
{
	i2c_del_driver(&sii8334_mhl_tx_i2c_driver);
}

module_init(sii8334_mhl_tx_init);
module_exit(sii8334_mhl_tx_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Silicon Image <http://www.siliconimage.com>");
MODULE_DESCRIPTION(MHL_DRIVER_DESC);
