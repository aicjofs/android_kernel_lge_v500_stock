#ifndef __REFCODE_H__
#define __REFCODE_H__

//                       

#define F54_Porting

#ifdef F54_Porting
#include <linux/kernel.h>	//      
#include <linux/string.h>	//      
extern void device_I2C_read(unsigned char add, unsigned char *value, unsigned short len);
extern void device_I2C_write(unsigned char add, unsigned char *value, unsigned short len);
extern void InitPage(void);
extern void SetPage(unsigned char page);
extern void readRMI(unsigned short add, unsigned char *value, unsigned short len);
extern void writeRMI(unsigned short add, unsigned char *value, unsigned short len);
extern void delayMS(int val);
extern void cleanExit(int code);
extern int waitATTN(int code, int time);
extern void write_log(char *data);
extern int get_limit( unsigned char Tx, unsigned char Rx);
extern int LimitFile[30][46*2];
#endif

//                  
#define _F54_TEST_
#define _FW_TESTING_
//                                 
//                                           
#define _DS4_3_2_	//                               

//                                                                                    
//                                       
//                                                       
//                                               
//                                        
//                                                               
#if defined(CONFIG_MACH_APQ8064_GK_KR) || defined(CONFIG_MACH_APQ8064_GKATT)
#define NoiseMitigation 0x155	//            
#else
#define NoiseMitigation 0x157	//            
#endif
//                                               
//                                                               
#if defined(CONFIG_MACH_APQ8064_GK_KR) || defined(CONFIG_MACH_APQ8064_GKATT)
#define F54_CBCPolarity 0x16E	//            
#else
#define F54_CBCPolarity 0x170	//            
#endif

#ifdef _DS4_3_2_
#define F55_PhysicalRx_Addr 0x301	//                               
#endif

#ifdef _F54_TEST_
unsigned char F54_FullRawCap(int);
unsigned char F54_RxToRxReport(void);
unsigned char F54_TxToGndReport(void);
unsigned char F54_TxToTxReport(void);
unsigned char F54_TxOpenReport(void);
unsigned char F54_RxOpenReport(void);
unsigned char F54_HighResistance(void);
#endif

#ifdef _BUTTON_DELTA_IMAGE_TEST_
unsigned char F54_ButtonDeltaImage();
#endif

#ifdef _FW_TESTING_
void HostImplementationTesting( void );
#endif

#ifdef _F34_TEST_
void CompleteReflash_OmitLockdown();
void CompleteReflash();
void CompleteReflash_Lockdown();
void ConfigBlockReflash();
#endif

void FirmwareCheck( void );
void AttentionTest( void );
void FirmwareCheck_temp( void );

#endif

