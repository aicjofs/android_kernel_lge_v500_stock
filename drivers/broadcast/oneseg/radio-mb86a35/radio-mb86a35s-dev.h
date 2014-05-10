/* 
                             
                                                      
                                                                            
*/
/* COPYRIGHT FUJITSU SEMICONDUCTOR LIMITED 2011 */

#ifndef	__RADIO_MB86A35_DEV_H__
#define	__RADIO_MB86A35_DEV_H__

#include <linux/module.h>	/*        */
#include <linux/moduleparam.h>
#include <linux/init.h>
#include <linux/device.h>
#include <linux/interrupt.h>
#include <linux/platform_device.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#include <linux/spi/spi.h>
//                             
#include <linux/fs.h>		/*                                   */
#ifndef  DEBUG_I386
#include <mach/hardware.h>
#endif
#include <asm/irq.h>
#include <asm/io.h>

#include <linux/kernel.h>	/*        */
#include <linux/proc_fs.h>
#include <linux/errno.h>
#include <linux/sched.h>	/*             */
#include <linux/timer.h>
#include <linux/time.h>
#include <linux/delay.h>	/*       */

#include <asm/io.h>
#include <asm/uaccess.h>	/*                */

#include <linux/types.h>
#include <linux/stat.h>
#include <linux/fcntl.h>
#include <linux/errno.h>

#include <media/v4l2-dev.h>	/*         */

#include <linux/dma-mapping.h>
#include <linux/gpio.h>

/*                                            */
#define	NODE_MAJOR		VIDEO_MAJOR	/*           */
#define	NODE_MINOR			64	/*           */
//                               

#include "radio-mb86a35.h"

//                                       

//                      
//                             
//                             
//                             
#define	PRINT_BASENAME		"radio-mb86a35"
#define	PRINT_LINE		__LINE__
#define	PRINT_FUNCTION		__FUNCTION__
#define	PRINT_LHEADERFMT	"%s[%d] %s "
#define	PRINT_LHEADER		PRINT_BASENAME,PRINT_LINE,PRINT_FUNCTION
#define PRINT_FORMAT		"%s[%d] %s [%05d] %s | %s\n"

#undef DEBUG

#ifdef	DEBUG
unsigned char STOPLOG = 0;

#define PRINT_DEBUG             KERN_DEBUG

#undef	DEBUG_PRINT_MEM
#endif

#ifdef	DEBUG_PRINT_MEM

unsigned char *LOGMEM = NULL;
#define	DEBUG_PRINT_MEM_SIZE		4096 * 4 + 128
int LOGCOPYSIZE = 0;

#define	INFOPRINT(format, arg...)

#ifdef	PRINT_INFO
#undef	INFOPRINT
#define INFOPRINT(format, arg...)	{\
		if(( LOGMEM != NULL ) && ( strlen( LOGMEM ) < ( DEBUG_PRINT_MEM_SIZE - 128 )))	\
			sprintf( &LOGMEM[strlen(LOGMEM)], " I:" format , ## arg );	\
		else	PRINT( PRINT_INFO format , ## arg );	\
	}
#endif

#define	ERRPRINT(format, arg...)

#ifdef	PRINT_ERROR
#undef	ERRPRINT
#define ERRPRINT(format, arg...)	{\
		if(( LOGMEM != NULL ) && ( strlen( LOGMEM ) < ( DEBUG_PRINT_MEM_SIZE - 128 )))	\
			sprintf( &LOGMEM[strlen(LOGMEM)], " E:" format , ## arg );	\
		else	PRINT( PRINT_ERROR format , ## arg );	\
	}
#endif

#define	DBGPRINT(format, arg...)

#ifdef	PRINT_DEBUG
#undef	DBGPRINT
#define DBGPRINT(format, arg...)	{\
		if(( LOGMEM != NULL ) && ( strlen( LOGMEM ) < ( DEBUG_PRINT_MEM_SIZE - 128 )))	\
			sprintf( &LOGMEM[strlen(LOGMEM)], " D:" format , ## arg );	\
		else	PRINT( PRINT_DEBUG format , ## arg );	\
	}
#endif

#else
#define	INFOPRINT(format, arg...)

#ifdef	PRINT_INFO
#undef	INFOPRINT
#define INFOPRINT(format, arg...)	\
		PRINT( PRINT_INFO format , ## arg )
#endif

#define	ERRPRINT(format, arg...)

#ifdef	PRINT_ERROR
#undef	ERRPRINT
#define ERRPRINT(format, arg...)	\
		PRINT( PRINT_ERROR format , ## arg )
#endif

#define	DBGPRINT(format, arg...)

#ifdef	PRINT_DEBUG
#undef	DBGPRINT
#define DBGPRINT(format, arg...)	\
		PRINT( PRINT_DEBUG format , ## arg )
#endif
#endif

/*                                                                                                 */
/*                                                                                                 */
/*                                                                                                 */
#define MB86A35S_SPI_WRITE_HEADER_1BYTE		0x10
#define MB86A35S_SPI_WRITE_HEADER_3BYTE		0x20
#define MB86A35S_SPI_READ_HEADER_1BYTE		0x40
#define MB86A35S_SPI_READ_HEADER_3BYTE		0x80
#define MB86A35S_SPI_WRITE_1BYTE		0x01
#define MB86A35S_SPI_WRITE_2BYTE		0x02
#define MB86A35S_SPI_WRITE_3BYTE		0x03
#define MB86A35S_SPI_READ_1BYTE			0x01
#define MB86A35S_SPI_READ_2BYTE			0x02
#define MB86A35S_SPI_READ_3BYTE			0x03
#define MB86A35S_REG_ADDR_SPIF_EN		0x90
#define MB86A35S_SET_OFDN_EN			0x01
#define MB86A35S_SET_OFDN_DIS			0x00
#define MB86A35S_SET_RF_EN			0x02
#define MB86A35S_SET_RF_DIS			0x00

#define MB86A35S_IRQ_WAIT			5000
#define MB86A35S_IRQ_DELAY			1
#define MB86A35S_SPI_WAITTIME			200	/*           */

#define MB86A35S_MAP_SIZE			(500 * 1024) /*       */
#define MB86A35S_SPIS_160PAC			0x7F80 + 4

#define MB86A35S_SPIRF_FREQ			0x14

/*                 */
#define MB86A35S_REG_ADDR_SPICTRL		0x92
#define MB86A35S_REG_ADDR_STREAMREAD		0x94
#define MB86A35S_REG_ADDR_SPIRF_FREQ		0x98
#define MB86A35S_REG_ADDR_SPI_SUBA		0x9E
#define MB86A35S_REG_ADDR_SPI_SUBD		0x9F
#define MB86A35S_REG_ADDR_RFA			0xB0
#define MB86A35S_REG_ADDR_RFD			0xB1
#define MB86A35S_REG_ADDR_IOSEL3		0xF9
#define MB86A35S_REG_ADDR_IOOUTEN		0xFC

/*                       */
#define	MB86A35_REG_SUBR_NULLCTRL		0xD0

/*                       */
#define	MB86A35S_REG_SUBR_PACBYTE		0x00
#define	MB86A35S_REG_SUBR_ERRCTRL		0x01
#define	MB86A35S_REG_SUBR_SPIS0_IRQMASK		0x1D
#define	MB86A35S_REG_SUBR_RAMDATAH		0x23
#define	MB86A35S_REG_SUBR_RAMDATAL		0x24

#ifndef FPGA
#define	MB86A35S_READ_SIZE_MAX			(160*204+3)
#else
#define	MB86A35S_READ_SIZE_MAX			(120*204+3)
#endif

#define GPIO_SPI_CLKI				54
#define GPIO_SPI_DO				52  //                                   
#define GPIO_SPI_DI				51  //                                   
#define GPIO_SPI_CSI				53
/*             */
//                            
//                           
#define GPIO_SPI_XIRQ				77 //    
//                            


#define GPIO_SPI_TEST				138
#define GPIO_SPI_TEST2				133



#define	MB86A35_I2C_ADDRESS			0x10
#define	MB86A35_I2C_ADAPTER_ID			0	/*                        */
#define	MB86A35_I2C_RFICADDRESS			( 0xC2 >> 1 )

#define	MB86A35_SELECT_UHF			1
#define	MB86A35_SELECT_VHF			0

#define MB86A35_CN_MONI_WAITTIME(n)		( 1260 * ( 2 << ( n + 1 )))/1000 + 10

#define MB86A35_MER_MONI_WAITTIME(n)		( 1260 * ( 2 << n ))/1000 + 10

#define	MB86A35_RF_CHANNEL_SET_TIMEOUTCOUNT	3

/*                                */

static inline char SEGCNT_FROM_CHNO(int chno)
{
	/*                                            */
	char SEGCNT_T[42]
	    = { 0x90, 0x90,	/*                   */
		0xA0, 0xA0, 0xA0,	/*                   */
		0xB0, 0xB0, 0xB0,	/*                   */
		0xC0, 0xC0, 0xC0,	/*                   */
		0xD0, 0xD0, 0xD0,	/*                   */
		0xE0, 0xE0, 0xE0,	/*                   */
		0xF0, 0xF0, 0xF0,	/*                   */
		0x00, 0x00, 0x00,	/*                   */
		0x10, 0x10, 0x10,	/*                   */
		0x20, 0x20, 0x20,	/*                   */
		0x30, 0x30, 0x30,	/*                   */
		0x40, 0x40, 0x40,	/*                   */
		0x50, 0x50, 0x50,	/*                   */
		0x60, 0x60, 0x60,	/*                   */
		0x90		/*                   */
	};

	if (chno > 41)
		return 0;
	return SEGCNT_T[chno];
}

									/*        */
									/*                */
#define	PARAM_FTSEGCNT_SEGCNT_ISDB_Tmm(n)		SEGCNT_FROM_CHNO(n)

/*                                                                                                 */
/*                                                                                                 */
/*                                                                                                 */

/*                              */
#define	MB86A35_REG_ADDR_REVISION		0x00		/*                */	/*             */
#define	MB86A35_REG_ADDR_IFAGC			0x03	/*                */
#define	MB86A35_REG_ADDR_SUBADR			0x04	/*                */
#define	MB86A35_REG_ADDR_SUBDAT			0x05	/*                */
#define	MB86A35_REG_ADDR_MODED_CTRL		0x06		/*                */	/*             */
#define	MB86A35_REG_ADDR_MODED_STAT		0x07		/*                */	/*             */
#define	MB86A35_REG_ADDR_CONT_CTRL		0x08		/*                */	/*             */
#define	MB86A35_REG_ADDR_STATE_INIT		0x08		/*                */	/*             */
#define	MB86A35_REG_ADDR_IQINV			0x09		/*                */	/*             */
#define	MB86A35_REG_ADDR_SYNC_STATE		0x0A		/*                */	/*             */
#define	MB86A35_REG_ADDR_CONT_CTRL3		0x1C		/*                */	/*             */
#define	MB86A35_REG_ADDR_MODED_CTRL2		0x1F		/*                */	/*             */
#define	MB86A35_REG_ADDR_SPFFT_LAYERSEL		0x39		/*                */	/*             */
#define	MB86A35_REG_ADDR_FTSEGSFT		0x44		/*                */	/*             */
#define	MB86A35_REG_ADDR_CNCNT			0x45		/*                */	/*             */
#define	MB86A35_REG_ADDR_CNDATHI		0x46		/*                */	/*             */
#define	MB86A35_REG_ADDR_CNDATLO		0x47		/*                */	/*             */
#define	MB86A35_REG_ADDR_CNCNT2			0x48		/*                */	/*             */
#define	MB86A35_REG_ADDR_FEC_SUBA		0x50		/*                */	/*             */
#define	MB86A35_REG_ADDR_FEC_SUBD		0x51		/*                */	/*             */
#define	MB86A35_REG_ADDR_VBERON			0x52		/*                */	/*             */
#define	MB86A35_REG_ADDR_VBERXRST		0x53		/*                */	/*             */
#define	MB86A35_REG_ADDR_VBERXRST_AL		0x53		/*                */	/*             */
#define	MB86A35_REG_ADDR_VBERFLG		0x54		/*                */	/*             */
#define	MB86A35_REG_ADDR_VBERFLG_AL		0x54		/*                */	/*             */
#define	MB86A35_REG_ADDR_VBERDTA0		0x55		/*                */	/*             */
#define	MB86A35_REG_ADDR_VBERDTA1		0x56		/*                */	/*             */
#define	MB86A35_REG_ADDR_VBERDTA2		0x57		/*                */	/*             */
#define	MB86A35_REG_ADDR_VBERDTB0		0x58		/*                */	/*             */
#define	MB86A35_REG_ADDR_VBERDTB1		0x59		/*                */	/*             */
#define	MB86A35_REG_ADDR_VBERDTB2		0x5A		/*                */	/*             */
#define	MB86A35_REG_ADDR_VBERDTC0		0x5B		/*                */	/*             */
#define	MB86A35_REG_ADDR_VBERDTC1		0x5C		/*                */	/*             */
#define	MB86A35_REG_ADDR_VBERDTC2		0x5D		/*                */	/*             */
#define	MB86A35_REG_ADDR_RSBERON		0x5E		/*                */	/*             */
#define	MB86A35_REG_ADDR_RSBERRST		0x5F		/*                */	/*             */
#define	MB86A35_REG_ADDR_RSBERXRST		0x5F		/*                */	/*             */
#define	MB86A35_REG_ADDR_RSBERCEFLG		0x60		/*                */	/*             */
#define	MB86A35_REG_ADDR_RSBERTHFLG		0x61		/*                */	/*             */
#define	MB86A35_REG_ADDR_RSERRFLG		0x62		/*                */	/*             */
#define	MB86A35_REG_ADDR_FECIRQ1		0x63		/*                */	/*             */
#define	MB86A35_REG_ADDR_RSBERDTA0		0x64		/*                */	/*             */
#define	MB86A35_REG_ADDR_RSBERDTA1		0x65		/*                */	/*             */
#define	MB86A35_REG_ADDR_RSBERDTA2		0x66		/*                */	/*             */
#define	MB86A35_REG_ADDR_RSBERDTB0		0x67		/*                */	/*             */
#define	MB86A35_REG_ADDR_RSBERDTB1		0x68		/*                */	/*             */
#define	MB86A35_REG_ADDR_RSBERDTB2		0x69		/*                */	/*             */
#define	MB86A35_REG_ADDR_RSBERDTC0		0x6A		/*                */	/*             */
#define	MB86A35_REG_ADDR_RSBERDTC1		0x6B		/*                */	/*             */
#define	MB86A35_REG_ADDR_RSBERDTC2		0x6C		/*                */	/*             */
#define	MB86A35_REG_ADDR_TMCC_SUBA		0x6D		/*                */	/*             */
#define	MB86A35_REG_ADDR_TMCC_SUBD		0x6E		/*                */	/*             */
#define	MB86A35_REG_ADDR_FECIRQ2		0x6F		/*                */	/*             */
#define	MB86A35_REG_ADDR_RST			0x70		/*                */	/*             */
#define	MB86A35_REG_ADDR_SEGMENT		0x71		/*                */	/*             */
#define	MB86A35_REG_ADDR_FPWDNMODE		0x72		/*                */	/*             */
#define	MB86A35_REG_ADDR_MACRO_PWDN		0xD0		/*                */	/*             */
#define	MB86A35_REG_ADDR_DACOUT			0xD5		/*                */	/*             */
#define	MB86A35_REG_ADDR_FRMLOCK_SEL		0xDD		/*                */	/*             */
#define	MB86A35_REG_ADDR_XIRQINV		0xDD		/*                */	/*             */
#define	MB86A35_REG_ADDR_SEARCH_CTRL		0xE6		/*                */	/*             */
#define	MB86A35_REG_ADDR_SEARCH_CHANNEL		0xE7		/*                */	/*             */
#define	MB86A35_REG_ADDR_SEARCH_SUBA		0xE8		/*                */	/*             */
#define	MB86A35_REG_ADDR_SEARCH_SUBD		0xE9		/*                */	/*             */
#define	MB86A35_REG_ADDR_GPIO_DAT		0xEB		/*                */	/*             */
#define	MB86A35_REG_ADDR_GPIO_OUTSEL		0xEC		/*                */	/*             */
#define	MB86A35_REG_ADDR_TUNER_IRQ		0xED		/*                */	/*             */
#define	MB86A35_REG_ADDR_TUNER_IRQCTL		0xEE		/*                */	/*             */

#define	MB86A35_REG_RFADDR_CHIPID1		0x00	/*           */	/*            */
#define	MB86A35_REG_RFADDR_CHIPID0		0x01	/*            */
#define	MB86A35_REG_RFADDR_SPLITID		0x02	/*            */
#define	MB86A35_REG_RFADDR_RFAGC		0x03	/*          */
#define	MB86A35_REG_RFADDR_GVBB			0x04	/*         */
#define	MB86A35_REG_RFADDR_LNAGAIN		0x05	/*            */

/*                             */
#define	MB86A35_REG_SUBR_IFAH			0x00	/*                 *//*                */	/*             */
#define	MB86A35_REG_SUBR_IFAL			0x01		/*                */	/*             */
#define	MB86A35_REG_SUBR_IFBH			0x02		/*                */	/*             */
#define	MB86A35_REG_SUBR_IFBL			0x03		/*                */	/*             */
#define	MB86A35_REG_SUBR_DTS			0x08		/*                */	/*             */
#define	MB86A35_REG_SUBR_IFAGCO			0x09		/*                */	/*             */
#define	MB86A35_REG_SUBR_MAXIFAGC		0x0A		/*                */	/*             */
#define	MB86A35_REG_SUBR_AGAIN			0x0B		/*                */	/*             */
#define	MB86A35_REG_SUBR_VIFREFH		0x0E		/*                */	/*             */
#define	MB86A35_REG_SUBR_VIFREFL		0x0F		/*                */	/*             */
#define	MB86A35_REG_SUBR_VIFREF2H		0x10		/*                */	/*             */
#define	MB86A35_REG_SUBR_VIFREF2L		0x11		/*                */	/*             */
#define	MB86A35_REG_SUBR_LEVELTH		0x19		/*                */	/*             */	/*             */
#define	MB86A35_REG_SUBR_IFSAMPLE		0x29		/*                */	/*             */
#define	MB86A35_REG_SUBR_OUTSAMPLE		0x32		/*                */	/*             */
#define	MB86A35_REG_SUBR_IFAGCDAC		0x3A		/*                */	/*             */

#define	MB86A35_REG_SUBR_MERCTRL		0x50	/*                     *//*             */	/*             */
#define	MB86A35_REG_SUBR_MERSTEP		0x51		/*             */	/*             */
#define	MB86A35_REG_SUBR_MERA0			0x52		/*             */	/*             */
#define	MB86A35_REG_SUBR_MERA1			0x53		/*             */	/*             */
#define	MB86A35_REG_SUBR_MERA2			0x54		/*             */	/*             */
#define	MB86A35_REG_SUBR_MERB0			0x55		/*             */	/*             */
#define	MB86A35_REG_SUBR_MERB1			0x56		/*             */	/*             */
#define	MB86A35_REG_SUBR_MERB2			0x57		/*             */	/*             */
#define	MB86A35_REG_SUBR_MERC0			0x58		/*             */	/*             */
#define	MB86A35_REG_SUBR_MERC1			0x59		/*             */	/*             */
#define	MB86A35_REG_SUBR_MERC2			0x5A		/*             */	/*             */
#define	MB86A35_REG_SUBR_MEREND			0x5B		/*             */	/*             */
#define	MB86A35_REG_SUBR_VBERSETA0		0xA7		/*                   */	/*             */
#define	MB86A35_REG_SUBR_VBERSETA1		0xA8		/*                   */	/*             */
#define	MB86A35_REG_SUBR_VBERSETA2		0xA9		/*                   */	/*             */
#define	MB86A35_REG_SUBR_VBERSETB0		0xAA		/*                   */	/*             */
#define	MB86A35_REG_SUBR_VBERSETB1		0xAB		/*                   */	/*             */
#define	MB86A35_REG_SUBR_VBERSETB2		0xAC		/*                   */	/*             */
#define	MB86A35_REG_SUBR_VBERSETC0		0xAD		/*                   */	/*             */
#define	MB86A35_REG_SUBR_VBERSETC1		0xAE		/*                   */	/*             */
#define	MB86A35_REG_SUBR_VBERSETC2		0xAF		/*                   */	/*             */
#define	MB86A35_REG_SUBR_PEREN			0xB0		/*             */	/*             */
#define	MB86A35_REG_SUBR_PERRST			0xB1		/*             */	/*             */
#define	MB86A35_REG_SUBR_PERSNUMA0		0xB2		/*                   */	/*             */
#define	MB86A35_REG_SUBR_PERSNUMA1		0xB3		/*                   */	/*             */
#define	MB86A35_REG_SUBR_PERSNUMB0		0xB4		/*                   */	/*             */
#define	MB86A35_REG_SUBR_PERSNUMB1		0xB5		/*                   */	/*             */
#define	MB86A35_REG_SUBR_PERSNUMC0		0xB6		/*                   */	/*             */
#define	MB86A35_REG_SUBR_PERSNUMC1		0xB7		/*                   */	/*             */
#define	MB86A35_REG_SUBR_PERFLG			0xB8		/*             */	/*             */
#define	MB86A35_REG_SUBR_PERERRA0		0xB9		/*                  */	/*             */
#define	MB86A35_REG_SUBR_PERERRA1		0xBA		/*                  */	/*             */
#define	MB86A35_REG_SUBR_PERERRB0		0xBB		/*                  */	/*             */
#define	MB86A35_REG_SUBR_PERERRB1		0xBC		/*                  */	/*             */
#define	MB86A35_REG_SUBR_PERERRC0		0xBD		/*                  */	/*             */
#define	MB86A35_REG_SUBR_PERERRC1		0xBE		/*                  */	/*             */
#define	MB86A35_REG_SUBR_PERERRFLG		0xB8		/*             */	/*             */
#define	MB86A35_REG_SUBR_TSMASK1		0xCC		/*             */	/*             */
#define	MB86A35_REG_SUBR_TSOUT2			0xCD		/*             */	/*             */
#define	MB86A35_REG_SUBR_PBER2			0xCE		/*             */	/*             */
#define	MB86A35_REG_SUBR_SBER2			0xCF		/*             */	/*             */
#define	MB86A35_REG_SUBR_RS0			0xD1		/*             */	/*             */
#define	MB86A35_REG_SUBR_TSMASK			0xD4		/*             */	/*             */
#define	MB86A35_REG_SUBR_TSOUT			0xD5		/*             */	/*             */
#define	MB86A35_REG_SUBR_PBER			0xD6		/*             */	/*            */
#define	MB86A35_REG_SUBR_SBER			0xD7		/*             */	/*             */
#define	MB86A35_REG_SUBR_STSSEL			0xD8		/*             */	/*             */
#define	MB86A35_REG_SUBR_TSERR			0xD9		/*             */	/*             */
#define	MB86A35_REG_SUBR_IRQMASK		0xDA		/*             */	/*             */
#define	MB86A35_REG_SUBR_IRQMASK2		0xDB		/*             */	/*             */
#define	MB86A35_REG_SUBR_SBERSETA0		0xDC		/*                   */	/*             */
#define	MB86A35_REG_SUBR_SBERSETA1		0xDD		/*                   */	/*             */
#define	MB86A35_REG_SUBR_SBERSETB0		0xDE		/*                   */	/*             */
#define	MB86A35_REG_SUBR_SBERSETB1		0xDF		/*                   */	/*             */
#define	MB86A35_REG_SUBR_SBERSETC0		0xE0		/*                   */	/*             */
#define	MB86A35_REG_SUBR_SBERSETC1		0xE1		/*                   */	/*             */

#define	MB86A35_REG_SUBR_TMCC_IRQ_MASK		0x35	/*                       *//*                */	/*             */
#define	MB86A35_REG_SUBR_TMCC_IRQ_RST		0x36		/*                 */	/*             */
#define	MB86A35_REG_SUBR_TMCC_IRQ_EMG_INV	0x38		/*            */	/*             */
#define	MB86A35_REG_SUBR_S8WAIT			0x71		/*            */	/*             */
#define	MB86A35_REG_SUBR_TMCC0			0x80	/*            */
#define	MB86A35_REG_SUBR_TMCC1			0x81	/*            */
#define	MB86A35_REG_SUBR_TMCC2			0x82	/*            */
#define	MB86A35_REG_SUBR_TMCC3			0x83	/*            */
#define	MB86A35_REG_SUBR_TMCC4			0x84	/*            */
#define	MB86A35_REG_SUBR_TMCC5			0x85	/*               */
#define	MB86A35_REG_SUBR_TMCC6			0x86	/*                     */
#define	MB86A35_REG_SUBR_TMCC7			0x87	/*                     */
#define	MB86A35_REG_SUBR_TMCC8			0x88	/*                     */
#define	MB86A35_REG_SUBR_TMCC9			0x89	/*                     */
#define	MB86A35_REG_SUBR_TMCC10			0x8A	/*                     */
#define	MB86A35_REG_SUBR_TMCC11			0x8B	/*                     */
#define	MB86A35_REG_SUBR_TMCC12			0x8C	/*                     */
#define	MB86A35_REG_SUBR_TMCC13			0x8D	/*                     */
#define	MB86A35_REG_SUBR_TMCC14			0x8E	/*                     */
#define	MB86A35_REG_SUBR_TMCC15			0x8F	/*                     */
#define	MB86A35_REG_SUBR_TMCC16			0x90	/*                       */
#define	MB86A35_REG_SUBR_TMCC17			0x91	/*                       */
#define	MB86A35_REG_SUBR_TMCC18			0x92	/*            */
#define	MB86A35_REG_SUBR_TMCC19			0x93	/*            */
#define	MB86A35_REG_SUBR_TMCC20			0x94	/*            */
#define	MB86A35_REG_SUBR_TMCC21			0x95	/*            */
#define	MB86A35_REG_SUBR_TMCC22			0x96	/*            */
#define	MB86A35_REG_SUBR_TMCC23			0x97	/*            */
#define	MB86A35_REG_SUBR_TMCC24			0x98	/*            */
#define	MB86A35_REG_SUBR_TMCC25			0x99	/*            */
#define	MB86A35_REG_SUBR_TMCC26			0x9A	/*            */
#define	MB86A35_REG_SUBR_TMCC27			0x9B	/*            */
#define	MB86A35_REG_SUBR_TMCC28			0x9C	/*            */
#define	MB86A35_REG_SUBR_TMCC29			0x9D	/*            */
#define	MB86A35_REG_SUBR_TMCC30			0x9E	/*            */
#define	MB86A35_REG_SUBR_TMCC31			0x9F	/*            */
#define	MB86A35_REG_SUBR_FEC_IN			0xA0		/*                */	/*             */
#define	MB86A35_REG_SUBR_TMCCREAD		0xA1		/*                */	/*             */
#define	MB86A35_REG_SUBR_SBER_IRQ_MASK		0xDA		/*                */	/*             */
#define	MB86A35_REG_SUBR_TSERR_IRQ_MASK		0xDB		/*                 */	/*             */
#define	MB86A35_REG_SUBR_XIRQINV		0xDD		/*                 */	/*             */
#define	MB86A35_REG_SUBR_PCHKOUT0		0xF0		/*                 */	/*             */
#define	MB86A35_REG_SUBR_PCHKOUT1		0xF1		/*                 */	/*             */
#define	MB86A35_REG_SUBR_TMCCCHK_HI		0xFA		/*                 */	/*             */
#define	MB86A35_REG_SUBR_TMCCCHK_LO		0xFB		/*                 */	/*             */
#define	MB86A35_REG_SUBR_TMCCCHK2_HI		0xF8		/*                 */	/*             */
#define	MB86A35_REG_SUBR_TMCCCHK2_LO		0xF9		/*                 */	/*             */

#define	MB86A35_REG_SUBR_CHANNEL0		0x00	/*                           */	/*                */
#define	MB86A35_REG_SUBR_CHANNEL1		0x01	/*                */
#define	MB86A35_REG_SUBR_CHANNEL2		0x02	/*                */
#define	MB86A35_REG_SUBR_CHANNEL3		0x03	/*                */
#define	MB86A35_REG_SUBR_CHANNEL4		0x04	/*                */
#define	MB86A35_REG_SUBR_CHANNEL5		0x05	/*                */
#define	MB86A35_REG_SUBR_CHANNEL6		0x06	/*                */
#define	MB86A35_REG_SUBR_CHANNEL7		0x07	/*                */
#define	MB86A35_REG_SUBR_CHANNEL8		0x10	/*                */
#define	MB86A35_REG_SUBR_SEARCH_END		0x08	/*                */

/*                        */
#define	MB86A35_REG_DATA_REVISION		0x43			/*               */	/*             */

#define	MB86A35_MASK_MODED_CTRL_M3G32		0x80	/*            */	/*               */
#define	MB86A35_MASK_MODED_CTRL_M3G16		0x40	/*            */	/*               */
#define	MB86A35_MASK_MODED_CTRL_M3G8		0x20	/*            */	/*               */
#define	MB86A35_MASK_MODED_CTRL_M3G4		0x10	/*            */	/*               */
#define	MB86A35_MASK_MODED_CTRL_M2G32		0x08	/*            */	/*               */
#define	MB86A35_MASK_MODED_CTRL_M2G16		0x04	/*            */	/*               */
#define	MB86A35_MASK_MODED_CTRL_M2G8		0x02	/*            */	/*               */
#define	MB86A35_MASK_MODED_CTRL_M2G4		0x01	/*            */	/*               */

#define	MB86A35_MASK_MODED_STAT_MDFAIL		0x40	/*            */	/*                     */
#define	MB86A35_MASK_MODED_STAT_MDDETECT	0x20	/*            */	/*                     */
#define	MB86A35_MASK_MODED_STAT_MDFIX		0x10	/*            */	/*                     */
#define	MB86A35_MASK_MODED_STAT_MODE		0x0C	/*            */	/*                       */
#define	MB86A35_MASK_MODED_STAT_GUARDE		0x03	/*            */	/*                       */

#define	MB86A35_MASK_CONT_CTRL_MDTEST		0x02	/*           */	/*                   */
#define	MB86A35_MASK_CONT_CTRL_STATEINIT	0x01	/*           */	/*                   */

#define	MB86A35_MASK_STATE_INIT_INIT		0x01	/*            */	/*              */

#define	MB86A35_MASK_IQINV			0x04	/*       */	/*               */

#define	MB86A35_MASK_STAT			0x07	/*            */	/*                     */

#define	MB86A35_MASK_CONT_CTRL3_IQSEL		0x02	/*            */	/*                   */
#define	MB86A35_MASK_CONT_CTRL3_IFSEL		0x01	/*            */	/*                   */
#define	MB86A35_MASK_CONT_CTRL3			( MB86A35_MASK_CONT_CTRL3_IQSEL | MB86A35_MASK_CONT_CTRL3_IFSEL )

#define	MB86A35_MASK_MODED_CTRL2_M1G32		0x08	/*             */	/*               */
#define	MB86A35_MASK_MODED_CTRL2_M1G16		0x04	/*             */	/*               */
#define	MB86A35_MASK_MODED_CTRL2_M1G8		0x02	/*             */	/*               */
#define	MB86A35_MASK_MODED_CTRL2_M1G4		0x01	/*             */	/*               */

#define	MB86A35_MASK_LAYERSEL			0x03	/*                */	/*                         */

#define	MB86A35_MASK_SEGCNT			0xF0	/*          */	/*                        */

#define	MB86A35_CNCNT_FLG			0x40	/*       */	/*              */
#define	MB86A35_CNCNT_LOCK			0x20	/*       */	/*              */
#define	MB86A35_CNCNT_RST			0x10	/*       */	/*              */
#define	MB86A35_MASK_CNCNT_SYMCOUNT		0x0F	/*       */	/*                         */

#define	MB86A35_MODE_AUTO			0x00	/*        */	/*              */
#define	MB86A35_MODE_MUNL			0x04	/*        */	/*              */
#define	MB86A35_MASK_MODE			0x04	/*        */	/*              */

#define	MB86A35_MERCTRL_LOCK			0x04	/*         */	/*               */
#define	MB86A35_MERCTRL_MODE			0x02	/*         */	/*               */
#define	MB86A35_MERCTRL_RST			0x01	/*         */	/*               */

#define	MB86A35_MASK_MERSTEP_SYMCOUNT		0x07	/*         */	/*                    */

#define	MB86A35_MEREND_FLG			0x01	/*        */	/*                */

#define	MB86A35_MASK_PEREN_PERENC		0x04	/*       */	/*                 */
#define	MB86A35_MASK_PEREN_PERENB		0x02	/*       */	/*                 */
#define	MB86A35_MASK_PEREN_PERENA		0x01	/*       */	/*                 */

#define	MB86A35_MASK_PERRST_PERRSTC		0x04	/*        */	/*                  */
#define	MB86A35_MASK_PERRST_PERRSTB		0x02	/*        */	/*                  */
#define	MB86A35_MASK_PERRST_PERRSTA		0x01	/*        */	/*                  */

#define	MB86A35_MASK_PERFLG_PERFLGC		0x04	/*        */	/*                  */
#define	MB86A35_MASK_PERFLG_PERFLGB		0x02	/*        */	/*                  */
#define	MB86A35_MASK_PERFLG_PERFLGA		0x01	/*        */	/*                  */

#define	MB86A35_MASK_TSMASK1_TSERRMASK		0x10	/*         */	/*                   */
#define	MB86A35_MASK_TSMASK1_TSPACMASK		0x08	/*         */	/*                   */
#define	MB86A35_MASK_TSMASK1_TSENMASK		0x04	/*         */	/*                   */
#define	MB86A35_MASK_TSMASK1_TSDTMASK		0x02	/*         */	/*                   */
#define	MB86A35_MASK_TSMASK1_TSCLKMASK		0x01	/*         */	/*                   */

#define	MB86A35_MASK_TSOUT2_TSERRINV		0x80	/*        */	/*                   */
#define	MB86A35_MASK_TSOUT2_TSENINV		0x40	/*        */	/*                   */
#define	MB86A35_MASK_TSOUT2_TSSINV		0x20	/*        */	/*                   */

#define	MB86A35_MASK_PBER2_PLAER		0x07	/*       */	/*                      */

#define	MB86A35_MASK_SBER2_SCLKSEL		0x80	/*       */	/*                   */
#define	MB86A35_MASK_SBER2_TSCLKCTRL		0x40	/*       */	/*                   */
#define	MB86A35_MASK_SBER2_SBERSEL		0x20	/*       */	/*                   */
#define	MB86A35_MASK_SBER2_SPACON		0x10	/*       */	/*                   */
#define	MB86A35_MASK_SBER2_SENON		0x08	/*       */	/*                   */
#define	MB86A35_MASK_SBER2_SLAYER		0x07	/*       */	/*                       */

#define	MB86A35_MASK_RS0_ERID			0x04	/*     */	/*                   */
#define	MB86A35_MASK_RS0_RSEN			0x02	/*     */	/*                   */

#define	MB86A35_MASK_TSMASK_TSFRMMASK		0x80	/*        */	/*                   */
#define	MB86A35_MASK_TSMASK_TSRSMASK		0x40	/*        */	/*                   */
#define	MB86A35_MASK_TSMASK_TSLAMASK		0x20	/*        */	/*                   */
#define	MB86A35_MASK_TSMASK_TSERRMASK		0x10	/*        */	/*                   */
#define	MB86A35_MASK_TSMASK_TSPACMASK		0x08	/*        */	/*                   */
#define	MB86A35_MASK_TSMASK_TSENMASK		0x04	/*        */	/*                   */
#define	MB86A35_MASK_TSMASK_TSDTMASK		0x02	/*        */	/*                   */
#define	MB86A35_MASK_TSMASK_TSCLKMASK		0x01	/*        */	/*                   */

#define	MB86A35_MASK_TSOUT_TSERRINV		0x80	/*       */	/*                    */
#define	MB86A35_MASK_TSOUT_TSENINV		0x40	/*       */	/*                    */
#define	MB86A35_MASK_TSOUT_TSSINV		0x20	/*       */	/*                    */
#define	MB86A35_MASK_TSOUT_TSPINV		0x10	/*       */	/*                    */
#define	MB86A35_MASK_TSOUT_TSRSINV		0x08	/*       */	/*                    */
#define	MB86A35_MASK_TSOUT_TSERRMASK2		0x04	/*       */	/*                    */
#define	MB86A35_MASK_TSOUT_TSERRMASK		0x02	/*       */	/*                    */

#define	MB86A35_MASK_PBER_PPACON		0x10	/*      */	/*                    */
#define	MB86A35_MASK_PBER_PENON			0x08	/*      */	/*                    */
#define	MB86A35_MASK_PBER_PLAER			0x07	/*      */	/*                      */

#define	MB86A35_MASK_SBER_SCLKSEL		0x80	/*      */	/*                    */
#define	MB86A35_MASK_SBER_TSCLKCTRL		0x40	/*      */	/*                    */
#define	MB86A35_MASK_SBER_SBERSEL		0x20	/*      */	/*                    */
#define	MB86A35_MASK_SBER_SPACON		0x10	/*      */	/*                    */
#define	MB86A35_MASK_SBER_SENON			0x08	/*      */	/*                    */
#define	MB86A35_MASK_SBER_SLAYER		0x07	/*      */	/*                       */

#define	MB86A35_MASK_STSSEL_STSSEL		0x01	/*        */	/*                    */

#define	MB86A35_MASK_TSERR_TSERR2		0x02	/*        */	/*                    */
#define	MB86A35_MASK_TSERR_TSERR1		0x01	/*        */	/*                    */

#define	MB86A35_MASK_IRQMASK_DTERIRQ		0x80	/*         */	/*                    */
#define	MB86A35_MASK_IRQMASK_THMASKC		0x20	/*         */	/*                    */
#define	MB86A35_MASK_IRQMASK_THMASKB		0x10	/*         */	/*                    */
#define	MB86A35_MASK_IRQMASK_THMASKA		0x08	/*         */	/*                    */
#define	MB86A35_MASK_IRQMASK_CEMASKC		0x04	/*         */	/*                    */
#define	MB86A35_MASK_IRQMASK_CEMASKB		0x02	/*         */	/*                    */
#define	MB86A35_MASK_IRQMASK_CEMASKA		0x01	/*         */	/*                    */

#define	MB86A35_MASK_IRQMASK2_ERRMASKC		0x04	/*          */	/*                    */
#define	MB86A35_MASK_IRQMASK2_ERRMASKB		0x02	/*          */	/*                    */
#define	MB86A35_MASK_IRQMASK2_ERRMASKA		0x01	/*          */	/*                    */

#define	MB86A35_MASK_VBERON_VBERON		0x01	/*        */	/*                    */

#define	MB86A35_MASK_VBERXRST_AL_VBERXRSTC	0x04	/*          */	/*                    */
#define	MB86A35_MASK_VBERXRST_AL_VBERXRSTB	0x02	/*          */	/*                    */
#define	MB86A35_MASK_VBERXRST_AL_VBERXRSTA	0x01	/*          */	/*                    */

#define	MB86A35_MASK_VBERFLG_AL_VBERXRSTC	0x04	/*            */	/*                    */
#define	MB86A35_MASK_VBERFLG_AL_VBERXRSTB	0x02	/*            */	/*                    */
#define	MB86A35_MASK_VBERFLG_AL_VBERXRSTA	0x01	/*            */	/*                    */

#define	MB86A35_MASK_RSBERON_RSBERAUTO		0x10	/*         */	/*                    */
#define	MB86A35_MASK_RSBERON_RSBERC		0x04	/*         */	/*                    */
#define	MB86A35_MASK_RSBERON_RSBERB		0x02	/*         */	/*                    */
#define	MB86A35_MASK_RSBERON_RSBERA		0x01	/*         */	/*                    */

#define	MB86A35_MASK_RSBERXRST_SBERXRSTC	0x04	/*           */	/*                    */
#define	MB86A35_MASK_RSBERXRST_SBERXRSTB	0x02	/*           */	/*                    */
#define	MB86A35_MASK_RSBERXRST_SBERXRSTA	0x01	/*           */	/*                    */

#define	MB86A35_MASK_RSBERCEFLG_SBERCEFC	0x04	/*            */	/*                    */
#define	MB86A35_MASK_RSBERCEFLG_SBERCEFB	0x02	/*            */	/*                    */
#define	MB86A35_MASK_RSBERCEFLG_SBERCEFA	0x01	/*            */	/*                    */

#define	MB86A35_MASK_RSBERTHFLG_SBERTHRC	0x40	/*            */	/*                    */
#define	MB86A35_MASK_RSBERTHFLG_SBERTHRB	0x20	/*            */	/*                    */
#define	MB86A35_MASK_RSBERTHFLG_SBERTHRA	0x10	/*            */	/*                    */
#define	MB86A35_MASK_RSBERTHFLG_SBERTHFC	0x04	/*            */	/*                    */
#define	MB86A35_MASK_RSBERTHFLG_SBERTHFB	0x02	/*            */	/*                    */
#define	MB86A35_MASK_RSBERTHFLG_SBERTHFA	0x01	/*            */	/*                    */

#define	MB86A35_MASK_RSERRFLG_RSERRSTC		0x40	/*          */	/*                    */
#define	MB86A35_MASK_RSERRFLG_RSERRSTB		0x20	/*          */	/*                    */
#define	MB86A35_MASK_RSERRFLG_RSERRSTA		0x10	/*          */	/*                    */
#define	MB86A35_MASK_RSERRFLG_RSERRC		0x04	/*          */	/*                    */
#define	MB86A35_MASK_RSERRFLG_RSERRB		0x02	/*          */	/*                    */
#define	MB86A35_MASK_RSERRFLG_RSERRA		0x01	/*          */	/*                    */

#define	MB86A35_MASK_FECIRQ1_TSERRC		0x40	/*         */	/*                    */
#define	MB86A35_MASK_FECIRQ1_TSERRB		0x20	/*         */	/*                    */
#define	MB86A35_MASK_FECIRQ1_TSERRA		0x10	/*         */	/*                    */
//                                                                                           
#define	MB86A35_MASK_FECIRQ1_EMG		0x04	/*         */	/*                    */
#define	MB86A35_MASK_FECIRQ1_CNT		0x02	/*         */	/*                    */
#define	MB86A35_MASK_FECIRQ1_ILL		0x01	/*         */	/*                    */

#define	MB86A35_MASK_S8WAIT_TSWAIT		0x80	/*        */	/*                    */
#define	MB86A35_MASK_S8WAIT_BERRST		0x40	/*        */	/*                    */
#define	MB86A35_MASK_S8WAIT			( MB86A35_MASK_S8WAIT_TSWAIT | MB86A35_MASK_S8WAIT_BERRST )

#define	MB86A35_MASK_FEC_IN_CORRECT		0x02	/*        */	/*                    */
#define	MB86A35_MASK_FEC_IN_VALID		0x01	/*        */	/*                    */

#define	MB86A35_MASK_TMCCREAD_TMCCLOCK		0x01	/*          */	/*                    */

//                                                                                           
#define	MB86A35_MASK_FECIRQ2_SBERCEFC		0x40	/*         */	/*                    */
#define	MB86A35_MASK_FECIRQ2_SBERCEFB		0x20	/*         */	/*                    */
#define	MB86A35_MASK_FECIRQ2_SBERCEFA		0x10	/*         */	/*                    */
//                                                                                           
#define	MB86A35_MASK_FECIRQ2_SBERTHFC		0x04	/*         */	/*                    */
#define	MB86A35_MASK_FECIRQ2_SBERTHFB		0x02	/*         */	/*                    */
#define	MB86A35_MASK_FECIRQ2_SBERTHFA		0x01	/*         */	/*                    */

#define	MB86A35_MASK_RST_I2CREG_RESET		0xF0	/*     */	/*                        */
#define	MB86A35_MASK_RST_LOGIC_RESET		0x0F	/*     */	/*                       */

#define	MB86A35_MASK_SEGMENT_BANDSEL		0xE0	/*         */	/*                     */
#define	MB86A35_MASK_SEGMENT_TMMSEL		0x04	/*         */	/*                     */
#define	MB86A35_MASK_SEGMENT_MYTYPESEL		0x03	/*         */	/*                     */

#define	MB86A35_MASK_FPWDNMODE_TSSEL_1S		0x01	/*           */	/*                     */

#define	MB86A35_MASK_MACRO_PWDN_DACPWDN		0x20	/*            */	/*                     */

#define	MB86A35_MASK_FRMLOCK_SEL_XIROINV	0x04	/*         */	/*                    */

#define	MB86A35_SEARCH_CTRL_SEARCH_AFT		0x20	/*             */	/*                    */
#define	MB86A35_SEARCH_CTRL_SEARCH_RST		0x10	/*             */	/*                    */
#define	MB86A35_SEARCH_CTRL_SEARCH		0x01	/*             */	/*                    */

#define	MB86A35_MASK_GPIO_DAT_AC_MODE		0x40	/*          */	/*                    */
#define	MB86A35_MASK_GPIO_DAT_GPIO_DAT		0x0F	/*          */	/*                    */

#define	MB86A35_MASK_GPIO_OUTSEL_GPIO_IRQOUT	0xF0	/*             */	/*                       */
#define	MB86A35_MASK_GPIO_OUTSEL_GPIO_OUTEN	0x0F	/*             */	/*                       */

#define	MB86A35_TUNER_IRQ_GPIO			0x02	/*           */	/*                 */
#define	MB86A35_TUNER_IRQ_CHEND			0x01	/*           */	/*                 */

#define	MB86A35_TUNER_IRQCTL_GPIO_RST		0x20	/*              */	/*                    */
#define	MB86A35_TUNER_IRQCTL_CHEND_RST		0x10	/*              */	/*                    */
#define	MB86A35_TUNER_IRQCTL_GPIOMASK		0x02	/*              */	/*                    */
#define	MB86A35_TUNER_IRQCTL_CHENDMASK		0x01	/*              */	/*                    */

#define	MB86A35_REG_ADDR_SUBSELECT		0xFE	/*            */	/*                */

#define	MB86A35_MASK_LNAGAIN_LNA		0x03	/*            */	/*               */

/*                                                                                                 */

#define	MB86A35_I2CMASK_STATE_INIT		0x02
#define	MB86A35_I2CMASK_SEGMENT			0x00
#define	MB86A35_I2CMASK_LAYERSEL		0x00
#define	MB86A35_I2CMASK_IQINV			0x3B
#define	MB86A35_I2CMASK_CONT_CTRL3		0x00
#define	MB86A35_I2CMASK_DACPWDN			0x00
#define	MB86A35_I2CMASK_FTSEGCNT		0x00
#define	MB86A35_I2CMASK_SUB_IFAH		0x00
#define	MB86A35_I2CMASK_SUB_IFAL		0x00
#define	MB86A35_I2CMASK_SUB_IFBH		0x00
#define	MB86A35_I2CMASK_SUB_IFBL		0x00
#define	MB86A35_I2CMASK_SUB_DTS			0x1F
#define	MB86A35_I2CMASK_SUB_IFAGCO		0x00
#define	MB86A35_I2CMASK_SUB_MAXIFAGC		0xFF
#define	MB86A35_I2CMASK_SUB_AGAIN		0xFF
#define	MB86A35_I2CMASK_SUB_VIFREFH		0x0F
#define	MB86A35_I2CMASK_SUB_VIFREFL		0xFF
#define	MB86A35_I2CMASK_SUB_VIFREF2H		0x0F
#define	MB86A35_I2CMASK_SUB_VIFREF2L		0xFF
#define	MB86A35_I2CMASK_SUB_IFSAMPLE		0xFF
#define	MB86A35_I2CMASK_SUB_OUTSAMPLE		0xFF
#define	MB86A35_I2CMASK_GPIO_DAT		0x80
#define	MB86A35_I2CMASK_GPIO_DAT_ACMODE		0x8F
#define	MB86A35_I2CMASK_TMCC_SUB_S8WAIT		0x3F
#define	MB86A35_I2CMASK_RS0			0x3D
#define	MB86A35_I2CMASK_TSOUT			0x01
#define	MB86A35_I2CMASK_TSOUT2			0x00
#define	MB86A35_I2CMASK_PBER			0x18
#define	MB86A35_I2CMASK_TSMASK0			0x60
#define	MB86A35_I2CMASK_TSMASK1			0x60
#define	MB86A35_I2CMASK_TMCC_IRQ_MASK		0xF8
#define	MB86A35_I2CMASK_SBER_IRQ_MASK		0x40
#define	MB86A35_I2CMASK_TSERR_IRQ_MASK		0x00
#define	MB86A35_I2CMASK_TMCC_IRQ_RST		0x00
#define	MB86A35_I2CMASK_EMG_INV			0x01

/*                                                                                                 */

struct MB86A35_setting_data {
	unsigned char address;	/*         */
	unsigned char data;	/*      */
};
typedef struct MB86A35_setting_data MB86A35_setting_data_t;

/*                   */
typedef struct MB86A35_setting_data MB86A35_RF_init_UHF_t;

//            
MB86A35_RF_init_UHF_t RF_INIT_UHF_DATA[] = {
	{0x22, 0xb0}, {0x23, 0x28}, {0x24, 0x88},
	{0x25, 0xbb}, {0x26, 0xcc}, {0x27, 0x2b},
	{0x28, 0xd0}, {0x29, 0xd7}, {0x2a, 0x92},
	{0x2b, 0x28}, {0x2e, 0x06}, {0x2f, 0x00},
	{0x30, 0x04}, {0x31, 0x3f}, {0x32, 0xff},
	{0x33, 0x30}, {0x34, 0x00}, {0x35, 0x00},
	{0x36, 0x40}, {0x37, 0x88}, {0x38, 0x88},
	{0x39, 0xf6}, {0x3a, 0x02}, {0x3b, 0x18},
	{0x3c, 0x11}, {0x3d, 0x38}, {0x3e, 0x1b},
	{0x3f, 0x44}, {0x40, 0x3a}, {0x41, 0x0f},
	{0x42, 0x1d}, {0x43, 0x1c}, {0x44, 0x38},
	{0x45, 0x19}, {0x46, 0x31}, {0x47, 0x62},
	{0x48, 0x38}, {0x49, 0x6f}, {0x4a, 0x62},
	{0x4b, 0x50}, {0x4c, 0x00}, {0x4d, 0x9b},
	{0x4e, 0x3b}, {0x4f, 0x3b}, {0x50, 0x3b},
	{0x51, 0x33}, {0x52, 0x40}, {0x53, 0xe7},
	{0x54, 0x01}, {0x55, 0x8b}, {0x56, 0x80},
	{0x57, 0x8f}, {0x58, 0x3f}, {0x59, 0x23},
	{0x5a, 0x08}, {0x5b, 0x30}, {0x5c, 0x55},
	{0x5d, 0x33}, {0x5e, 0x38}, {0x5f, 0x08},
	{0x60, 0xbb}, {0x61, 0x33}, {0x62, 0x0e},
	{0x63, 0x65}, {0x64, 0x07}, {0x65, 0x2F},
	{0x66, 0xcc}, {0x67, 0x80}, {0x68, 0x60},
	{0x69, 0x1f}, {0x6a, 0x11}, {0x6b, 0x5f},
	{0x6c, 0x00}, {0x6d, 0x2b}, {0x6e, 0x2b},
	{0x6f, 0x2b}, {0x70, 0x55}, {0x71, 0x59},
	{0x72, 0x5e}, {0x73, 0x40}, {0x74, 0x49},
	{0x75, 0x4a}, {0x76, 0x39}, {0x77, 0x39},
	{0x78, 0x39}, {0x79, 0x2f}, {0x7a, 0x28},
	{0x7b, 0x80}, {0x7c, 0x86}, {0x7d, 0x5e},
	{0x7e, 0x5e}, {0x7f, 0x00}, {0x80, 0x23},
	{0x81, 0x3c}, {0x82, 0x1e}, {0x83, 0xe4},
	{0x84, 0x88}, {0x85, 0x80}, {0x86, 0xff},
	{0x87, 0x7f}, {0x88, 0x00}, {0x89, 0x3c},
	{0x8a, 0x7f}, {0x8b, 0x20}, {0x8c, 0x0d},
	{0x8d, 0x00}, {0x8e, 0x60}, {0x8f, 0x56},
	{0x90, 0x50}, {0x91, 0x56}, {0x92, 0x12},
	{0x93, 0x40},
	{0xff, 0xff}
};

/*                   */
MB86A35_RF_init_UHF_t RF_INIT_VHF_DATA[] = {
	{0x22, 0x30}, {0x23, 0x28}, {0x24, 0x88},
	{0x25, 0xe8}, {0x26, 0xcc}, {0x27, 0x2b},
	{0x28, 0xd0}, {0x29, 0xd7}, {0x2a, 0x92},
	{0x2b, 0x28}, {0x2c, 0x00}, {0x2d, 0x00},
	{0x2e, 0x06}, {0x2f, 0x00}, {0x30, 0x04},
	{0x31, 0x3f}, {0x32, 0xff}, {0x33, 0x30},
	{0x34, 0x00}, {0x35, 0x00}, {0x36, 0x40},
	{0x37, 0x88}, {0x38, 0x88}, {0x39, 0xf6},
	{0x3a, 0x02}, {0x3b, 0x18}, {0x3c, 0x11},
	{0x3d, 0x38}, {0x3e, 0x1b}, {0x3f, 0x44},
	{0x40, 0x3a}, {0x41, 0x0f}, {0x42, 0x1d},
	{0x43, 0x1c}, {0x44, 0x38}, {0x45, 0x19},
	{0x46, 0x31}, {0x47, 0x62}, {0x48, 0x38},
	{0x49, 0x6f}, {0x4a, 0x62}, {0x4b, 0x50},
	{0x4c, 0x00}, {0x4d, 0xcb}, {0x4e, 0x3b},
	{0x4f, 0xbf}, {0x50, 0x3f}, {0x51, 0x33},
	{0x52, 0x40}, {0x53, 0xe7}, {0x54, 0x01},
	{0x55, 0x8b}, {0x56, 0x80}, {0x57, 0x8f},
	{0x58, 0x3f}, {0x59, 0x23}, {0x5a, 0x08},
	{0x5b, 0x30}, {0x5c, 0x55}, {0x5d, 0x33},
	{0x5e, 0x38}, {0x5f, 0x08}, {0x60, 0x88},
	{0x61, 0x03}, {0x62, 0x08}, {0x63, 0x85},
	{0x64, 0x07}, {0x65, 0x2f}, {0x66, 0xcc},
	{0x67, 0x80}, {0x68, 0x60}, {0x69, 0x1f},
	{0x6a, 0x11}, {0x6b, 0x5f}, {0x6c, 0x00},
	{0x6d, 0x2b}, {0x6e, 0x2b}, {0x6f, 0x2b},
	{0x70, 0x55}, {0x71, 0x59}, {0x72, 0x5e},
	{0x73, 0x40}, {0x74, 0x49}, {0x75, 0x4a},
	{0x76, 0x39}, {0x77, 0x39}, {0x78, 0x39},
	{0x79, 0x2f}, {0x7a, 0x28}, {0x7b, 0x80},
	{0x7c, 0x86}, {0x7d, 0x5e}, {0x7e, 0x5e},
	{0x7f, 0x00}, {0x80, 0x23}, {0x81, 0x3c},
	{0x82, 0x1e}, {0x83, 0xe4}, {0x84, 0x88},
	{0x85, 0x80}, {0x86, 0xff}, {0x87, 0x7f},
	{0x88, 0x00}, {0x89, 0x3c}, {0x8a, 0x7f},
	{0x8b, 0x20}, {0x8c, 0x0d}, {0x8d, 0x08},
	{0x8e, 0x60}, {0x8f, 0x56}, {0x90, 0x50},
	{0x91, 0x56}, {0x92, 0x12}, {0x93, 0x40},
	{0xff, 0xff}
};

//      
MB86A35_RF_init_UHF_t RF_INIT_UHF_DATA1[] = {
	{0x22, 0x80}, {0x23, 0x18}, 
	{0x25, 0xaa}, 
	
	{0x2b, 0x08},               {0x2f, 0x01},
	{0x30, 0x2e}, 
	              {0x34, 0x7f}, {0x35, 0x00},
	{0x36, 0x00}, 
	
	                            {0x3e, 0xff},
	{0x3f, 0x41}, {0x40, 0x41}, {0x41, 0x41},
	
	
	
	                            {0x4d, 0x8c},
	{0x4e, 0x08}, {0x4f, 0x8b}, {0x50, 0x09},
	{0x51, 0xc9}, {0x52, 0x97}, 
	{0x54, 0x41}, {0x55, 0x06}, {0x56, 0x00},
	              {0x58, 0xdf}, 
	{0x5a, 0x09},               {0x5c, 0x42},
	{0x5d, 0x1c}, {0x5e, 0x2a}, {0x5f, 0x22},
	{0x60, 0x49}, {0x61, 0x50}, {0x62, 0x96},
	{0x63, 0x96},               {0x65, 0xff},
	{0x66, 0xff},               {0x68, 0x90},
	{0x69, 0x90}, {0x6a, 0x81}, {0x6b, 0x42},
	{0x6c, 0x42}, 
	
	{0x72, 0xa0}, 
	{0x75, 0x07}, 
	{0x78, 0x20}, {0x79, 0xff}, {0x7a, 0xf0},
	{0x7b, 0xc0}, {0x7c, 0xf0}, 
	              {0x7f, 0xf6}, 
	
	
	{0x87, 0xc0}, {0x88, 0x20}, 
	              {0x8b, 0x13}, 
	{0x8d, 0xe8}, 
	              {0x91, 0x3a}, 
	
	{0xff, 0xff}
};

/*                   */
MB86A35_RF_init_UHF_t RF_INIT_VHF_DATA1[] = {
	{0x22, 0x00}, {0x23, 0x18}, 
	{0x25, 0xaa}, 
	
	{0x2b, 0x08}, 
	              {0x2f, 0x01}, {0x30, 0x2e},
	
	{0x34, 0x10}, {0x35, 0x3f}, {0x36, 0x00},
	                            
	
	              {0x3e, 0xe0}, {0x3f, 0x41},
	{0x40, 0x41}, {0x41, 0x41}, 
	
	
	
	              {0x4d, 0xcc}, {0x4e, 0x10},
	{0x4f, 0x8b}, {0x50, 0x00}, {0x51, 0x98},
	{0x52, 0x97},               {0x54, 0x00},
	{0x55, 0x07}, {0x56, 0x00}, 
	{0x58, 0xdf},               {0x5a, 0x06},
	              {0x5c, 0x42}, {0x5d, 0x1c},
	{0x5e, 0x2a}, {0x5f, 0x22}, {0x60, 0x49},
	{0x61, 0x63}, {0x62, 0x63}, {0x63, 0x63},
	              {0x65, 0xff}, {0x66, 0xff},
	              {0x68, 0x90}, {0x69, 0x90},
	{0x6a, 0x81}, {0x6b, 0x42}, {0x6c, 0x42},
	
	                            {0x72, 0x80},
	                            {0x75, 0x00},
	                            {0x78, 0x20},
	{0x79, 0xff}, {0x7a, 0xf0}, {0x7b, 0xc0},
	{0x7c, 0xf0}, 
	{0x7f, 0xff}, 
	
	                            {0x87, 0xc0},
	{0x88, 0x20}, 
	{0x8b, 0x13},               {0x8d, 0xe8},
	
	{0x91, 0x3a}, 
	{0xff, 0xff}
};


//          
MB86A35_RF_init_UHF_t RF_INIT_UHF_DATA2[] = {
	{0x22, 0x80}, {0x23, 0x18}, 
	{0x25, 0xaa}, 
	
	{0x2b, 0x08},               {0x2f, 0x01},
	{0x30, 0x2e}, 
	              {0x34, 0x7f}, {0x35, 0x00},
	{0x36, 0x00}, 
	
	                            {0x3e, 0xff},
        {0x3f, 0x41}, {0x40, 0x41}, {0x41, 0x41},
	
	
	
	
	                            {0x4d, 0x8c},
	{0x4e, 0x08}, {0x4f, 0x8b}, {0x50, 0x09},
	{0x51, 0xc9}, {0x52, 0x92}, 
	{0x54, 0x41}, {0x55, 0x08}, {0x56, 0x00},
	              {0x58, 0xdf}, 
	{0x5a, 0x04},               {0x5c, 0x02},
	{0x5d, 0x1c}, {0x5e, 0x2a}, {0x5f, 0x32},
	{0x60, 0x49}, {0x61, 0x50}, {0x62, 0x51},
	{0x63, 0xd4},               {0x65, 0xff},
	{0x66, 0xff},               {0x68, 0x90},
	{0x69, 0x88}, {0x6a, 0x78}, {0x6b, 0x01},
	{0x6c, 0x21}, 
	
	{0x72, 0xa0}, 
	{0x75, 0x07}, 
	{0x78, 0x20}, {0x79, 0xff}, {0x7a, 0xf0},
	{0x7b, 0xb0}, {0x7c, 0xf0}, {0x7d, 0x26},
	{0x7e, 0x26}, {0x7f, 0xf6}, 
	
	
	{0x87, 0xc0}, {0x88, 0x20}, 
	              {0x8b, 0x13}, 
	{0x8d, 0xe8}, 
	              {0x91, 0x32}, 
	
	{0xff, 0xff}
};

//                  
MB86A35_RF_init_UHF_t RF_INIT_VHF_DATA2[] = {
	{0x22, 0x00}, {0x23, 0x18}, 
	{0x25, 0xaa}, 
	
	{0x2b, 0x08}, 
	              {0x2f, 0x01}, {0x30, 0x2e},
	
	{0x34, 0x1a}, {0x35, 0xff}, {0x36, 0x5f},
	                            
	
	              {0x3e, 0xe0}, {0x3f, 0x41},
        {0x40, 0x41}, {0x41, 0x41},	
	
	
	
	              {0x4d, 0xcc}, {0x4e, 0x20},
	{0x4f, 0x8b}, {0x50, 0x00}, {0x51, 0x98},
	{0x52, 0x92},               {0x54, 0x60},
	{0x55, 0x08}, {0x56, 0x00}, 
	{0x58, 0xdf},               {0x5a, 0x04},
	              {0x5c, 0x02}, {0x5d, 0x1c},
	{0x5e, 0x2a}, {0x5f, 0x32}, {0x60, 0x49},
	{0x61, 0x63}, {0x62, 0x51}, {0x63, 0xd4},
	              {0x65, 0xff}, {0x66, 0xff},
	              {0x68, 0x90}, {0x69, 0x88},
	{0x6a, 0x78}, {0x6b, 0x01}, {0x6c, 0x20},
	
	                            {0x72, 0x80},
	                            {0x75, 0x00},
	                            {0x78, 0x20},
	{0x79, 0xff}, {0x7a, 0xf0}, {0x7b, 0xb0},
	{0x7c, 0xf0}, {0x7d, 0x26}, {0x7e, 0x26},
	{0x7f, 0xff}, 
	
	                            {0x87, 0xc0},
	{0x88, 0x20}, 
	{0x8b, 0x13},               {0x8d, 0xe8},
	
	{0x91, 0x32}, 
	{0xff, 0xff}
};





/*                     */
struct mb86a35_freq {
	u8 CH;
	//                        
	//                   
	u8 PLLN;
	u32 PLLF;
	//                      
	u8 REG28;
	//                      
	u8 REG29;
	//                      
	u8 REG2A;
};
typedef struct mb86a35_freq mb86a35_freq_t;

mb86a35_freq_t mb86a35_freq_VHF[] = {
	/*                                                                                                         */
	{ /*    */ 1, /*                          */ 25, 0xFB6DB, /*      */
	 251, /*       */ 109, /*       */ 177},
	{ /*    */ 2, /*                          */ 26, 0x9249, /*      */ 9,
	 /*       */ 36, /*       */ 145},
	{ /*    */ 3, /*                          */ 26, 0x16DB6, /*      */ 22,
	 /*       */ 219, /*       */ 97},
	{ /*    */ 4, /*                          */ 26, 0x24924, /*      */ 36,
	 /*       */ 146, /*       */ 65},
	{ /*    */ 5, /*                          */ 26, 0x32492, /*      */ 50,
	 /*       */ 73, /*       */ 33},
	{ /*    */ 6, /*                          */ 26, 0x40000, /*      */ 64,
	 /*       */ 0, /*       */ 1},
	{ /*    */ 7, /*                          */ 26, 0x4DB6D, /*      */ 77,
	 /*       */ 182, /*       */ 209},
	{ /*    */ 8, /*                          */ 26, 0x5B6DB, /*      */ 91,
	 /*       */ 109, /*       */ 177},
	{ /*    */ 9, /*                          */ 26, 0x69249, /*      */
	 105, /*       */ 36, /*       */ 145},
	{ /*    */ 10, /*                          */ 26, 0x76DB6, /*      */
	 118, /*       */ 219, /*       */ 97},
	{ /*    */ 11, /*                          */ 26, 0x84924, /*      */
	 132, /*       */ 146, /*       */ 65},
	{ /*    */ 12, /*                          */ 26, 0x92492, /*      */
	 146, /*       */ 73, /*       */ 33},
	{ /*    */ 13, /*                          */ 26, 0xA0000, /*      */
	 160, /*       */ 0, /*       */ 1},
	{ /*    */ 14, /*                          */ 26, 0xADB6D, /*      */
	 173, /*       */ 182, /*       */ 209},
	{ /*    */ 15, /*                          */ 26, 0xBB6DB, /*      */
	 187, /*       */ 109, /*       */ 177},
	{ /*    */ 16, /*                          */ 26, 0xC9249, /*      */
	 201, /*       */ 36, /*       */ 145},
	{ /*    */ 17, /*                          */ 26, 0xD6DB6, /*      */
	 214, /*       */ 219, /*       */ 97},
	{ /*    */ 18, /*                          */ 26, 0xE4924, /*      */
	 228, /*       */ 146, /*       */ 65},
	{ /*    */ 19, /*                          */ 26, 0xF2492, /*      */
	 242, /*       */ 73, /*       */ 33},
	{ /*    */ 20, /*                          */ 27, 0x0, /*      */ 0,
	 /*       */ 0, /*       */ 1},
	{ /*    */ 21, /*                          */ 27, 0xDB6D, /*      */ 13,
	 /*       */ 182, /*       */ 209},
	{ /*    */ 22, /*                          */ 27, 0x1B6DB, /*      */
	 27, /*       */ 109, /*       */ 177},
	{ /*    */ 23, /*                          */ 27, 0x29249, /*      */
	 41, /*       */ 36, /*       */ 145},
	{ /*    */ 24, /*                          */ 27, 0x36DB6, /*      */
	 54, /*       */ 219, /*       */ 97},
	{ /*    */ 25, /*                          */ 27, 0x44924, /*      */
	 68, /*       */ 146, /*       */ 65},
	{ /*    */ 26, /*                          */ 27, 0x52492, /*      */
	 82, /*       */ 73, /*       */ 33},
	{ /*    */ 27, /*                          */ 27, 0x60000, /*      */
	 96, /*       */ 0, /*       */ 1},
	{ /*    */ 28, /*                          */ 27, 0x6DB6D, /*      */
	 109, /*       */ 182, /*       */ 209},
	{ /*    */ 29, /*                          */ 27, 0x7B6DB, /*      */
	 123, /*       */ 109, /*       */ 177},
	{ /*    */ 30, /*                          */ 27, 0x89249, /*      */
	 137, /*       */ 36, /*       */ 145},
	{ /*    */ 31, /*                          */ 27, 0x96DB6, /*      */
	 150, /*       */ 219, /*       */ 97},
	{ /*    */ 32, /*                          */ 27, 0xA4924, /*      */
	 164, /*       */ 146, /*       */ 65},
	{ /*    */ 33, /*                          */ 27, 0xB2492, /*      */
	 178, /*       */ 73, /*       */ 33},
	{ /*    */ 0xFF, /*                          */ 99, 0xFFFFF, /*      */
	 255, /*       */ 255, /*       */ 255}
};

mb86a35_freq_t mb86a35_freq_UHF[] = {
	/*                                                                                                          */
	{13, /*                          */ 29, 0x92492, /*      */ 146,
	 /*      */ 73, /*      */ 34},
	{14, /*                          */ 29, 0xF2492, /*      */ 242,
	 /*      */ 73, /*      */ 34},
	{15, /*                          */ 30, 0x52492, /*      */ 82,
	 /*      */ 73, /*      */ 34},
	{16, /*                          */ 30, 0xB2492, /*      */ 178,
	 /*      */ 73, /*      */ 34},
	{17, /*                          */ 31, 0x12492, /*      */ 18,
	 /*      */ 73, /*      */ 34},
	{18, /*                          */ 31, 0x72492, /*      */ 114,
	 /*      */ 73, /*      */ 34},
	{19, /*                          */ 31, 0xD2492, /*      */ 210,
	 /*      */ 73, /*      */ 34},
	{20, /*                          */ 32, 0x32492, /*      */ 50,
	 /*      */ 73, /*      */ 34},
	{21, /*                          */ 32, 0x92492, /*      */ 146,
	 /*      */ 73, /*      */ 34},
	{22, /*                          */ 32, 0xF2492, /*      */ 242,
	 /*      */ 73, /*      */ 34},
	{23, /*                          */ 33, 0x52492, /*      */ 82,
	 /*      */ 73, /*      */ 34},
	{24, /*                          */ 33, 0xB2492, /*      */ 178,
	 /*      */ 73, /*      */ 34},
	{25, /*                          */ 34, 0x12492, /*      */ 18,
	 /*      */ 73, /*      */ 34},
	{26, /*                          */ 34, 0x72492, /*      */ 114,
	 /*      */ 73, /*      */ 34},
	{27, /*                          */ 34, 0xD2492, /*      */ 210,
	 /*      */ 73, /*      */ 34},
	{28, /*                          */ 35, 0x32492, /*      */ 50,
	 /*      */ 73, /*      */ 34},
	{29, /*                          */ 35, 0x92492, /*      */ 146,
	 /*      */ 73, /*      */ 34},
	{30, /*                          */ 35, 0xF2492, /*      */ 242,
	 /*      */ 73, /*      */ 34},
	{31, /*                          */ 36, 0x52492, /*      */ 82,
	 /*      */ 73, /*      */ 34},
	{32, /*                          */ 36, 0xB2492, /*      */ 178,
	 /*      */ 73, /*      */ 34},
	{33, /*                          */ 37, 0x12492, /*      */ 18,
	 /*      */ 73, /*      */ 34},
	{34, /*                          */ 37, 0x72492, /*      */ 114,
	 /*      */ 73, /*      */ 34},
	{35, /*                          */ 37, 0xD2492, /*      */ 210,
	 /*      */ 73, /*      */ 34},
	{36, /*                          */ 38, 0x32492, /*      */ 50,
	 /*      */ 73, /*      */ 34},
	{37, /*                          */ 38, 0x92492, /*      */ 146,
	 /*      */ 73, /*      */ 34},
	{38, /*                          */ 38, 0xF2492, /*      */ 242,
	 /*      */ 73, /*      */ 34},
	{39, /*                          */ 39, 0x52492, /*      */ 82,
	 /*      */ 73, /*      */ 34},
	{40, /*                          */ 39, 0xB2492, /*      */ 178,
	 /*      */ 73, /*      */ 34},
	{41, /*                          */ 20, 0x9249, /*      */ 9, /*      */
	 36, /*      */ 145},
	{42, /*                          */ 20, 0x39249, /*      */ 57,
	 /*      */ 36, /*      */ 145},
	{43, /*                          */ 20, 0x69249, /*      */ 105,
	 /*      */ 36, /*      */ 145},
	{44, /*                          */ 20, 0x99249, /*      */ 153,
	 /*      */ 36, /*      */ 145},
	{45, /*                          */ 20, 0xC9249, /*      */ 201,
	 /*      */ 36, /*      */ 145},
	{46, /*                          */ 20, 0xF9249, /*      */ 249,
	 /*      */ 36, /*      */ 145},
	{47, /*                          */ 21, 0x29249, /*      */ 41,
	 /*      */ 36, /*      */ 145},
	{48, /*                          */ 21, 0x59249, /*      */ 89,
	 /*      */ 36, /*      */ 145},
	{49, /*                          */ 21, 0x89249, /*      */ 137,
	 /*      */ 36, /*      */ 145},
	{50, /*                          */ 21, 0xB9249, /*      */ 185,
	 /*      */ 36, /*      */ 145},
	{51, /*                          */ 21, 0xE9249, /*      */ 233,
	 /*      */ 36, /*      */ 145},
	{52, /*                          */ 22, 0x19249, /*      */ 25,
	 /*      */ 36, /*      */ 145},
	{0xFF, /*                          */ 99, 0xFFFFF, /*      */ 255,
	 /*       */ 255, /*       */ 255}
};

/*                     */
struct mb86a35_freq1 {
	u8 CH;
	u32 FREQ;        //          

};
typedef struct mb86a35_freq1 mb86a35_freq1_t;

mb86a35_freq1_t mb86a35_freq1_VHF[] = {
	/*                             */
	{ /*    */ 1, 207857},
	{ /*    */ 2, 208285},
	{ /*    */ 3, 208714},
	{ /*    */ 4, 209142},
	{ /*    */ 5, 209571},
	{ /*    */ 6, 210000},
	{ /*    */ 7, 210428},
	{ /*    */ 8, 210857},
	{ /*    */ 9, 211285},
	{ /*    */ 10,211714},
	{ /*    */ 11,212142},
	{ /*    */ 12,212571},
	{ /*    */ 13,213000},
	{ /*    */ 14,213428},
	{ /*    */ 15,213857},
	{ /*    */ 16,214285},
	{ /*    */ 17,214714},
	{ /*    */ 18,215142},
	{ /*    */ 19,215571},
	{ /*    */ 20,216000},
	{ /*    */ 21,216428},
	{ /*    */ 22,216857},
	{ /*    */ 23,217285},
	{ /*    */ 24,217714},
	{ /*    */ 25,218142},
	{ /*    */ 26,218571},
	{ /*    */ 27,219000},
	{ /*    */ 28,219428},
	{ /*    */ 29,219857},
	{ /*    */ 30,220285},
	{ /*    */ 31,220714},
	{ /*    */ 32,221142},
	{ /*    */ 33,221571}, 
	{ /*    */ 0xFF,999999},
};

mb86a35_freq1_t mb86a35_freq1_UHF[] = {

	/*                        */
	{13,     473143},
	{14,     479143},
	{15,     485143},
	{16,     491143},
	{17,     497143},
	{18,     503143},
	{19,     509143},
	{20,     515143},
	{21,     521143},
	{22,     527143},
	{23,     533143},
	{24,     539143},
	{25,     545143},
	{26,     551143},
	{27,     557143},
	{28,     563143},
	{29,     569143},
	{30,     575143},
	{31,     581143},
	{32,     587143},
	{33,     593143},
	{34,     599143},
	{35,     605143},
	{36,     611143},
	{37,     617143},
	{38,     623143},
	{39,     629143},
	{40,     635143},
	{41,     641143},
	{42,     647143},
	{43,     653143},
	{44,     659143},
	{45,     665143},
	{46,     671143},
	{47,     677143},
	{48,     683143},
	{49,     689143},
	{50,     695143},
	{51,     701143},
	{52,     707143},
	{53,     713143},
	{54,     719143},
	{55,     725143},
	{56,     731143},
	{57,     737143},
	{58,     743143},
	{59,     749143},
	{60,     755143},
	{61,     761143},
	{62,     767143},
	{0xFF,   999999},
};

struct mb86a35_cmdcontrol {
	char mer_flg;

	ioctl_reset_t RESET;
	ioctl_init_t INIT;
	ioctl_agc_t AGC;
	ioctl_port_t GPIO;
	ioctl_seq_t SEQ;
	ioctl_ber_moni_t BER;
	ioctl_ts_t TS;
	ioctl_irq_t IRQ;
	ioctl_cn_moni_t CN;
	ioctl_mer_moni_t MER;
	ioctl_ch_search_t CHSRH;
	ioctl_rf_t RF;
	ioctl_i2c_t I2C;
	ioctl_hrm_t HRM;
	ioctl_ofdm_init_t OFDM_INIT;
	ioctl_low_up_if_t LOW_UP_IF;
	ioctl_spi_t SPI;
	ioctl_spi_config_t SPI_CONFIG;
	ioctl_stream_read_t STREAM_READ;
	ioctl_stream_read_ctrl_t STREAM_READ_CTRL;
	ioctl_ts_setup_t TS_SETUP;
	ioctl_select_antenna_t SELECT_ANTENNA;	//                              
};
typedef struct mb86a35_cmdcontrol mb86a35_cmdcontrol_t;

#endif /*                         */
