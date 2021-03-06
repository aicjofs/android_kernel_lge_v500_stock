/*                                                                          */

/*
 *  m520xsim.h -- ColdFire 5207/5208 System Integration Module support.
 *
 *  (C) Copyright 2005, Intec Automation (mike@steroidmicros.com)
 */

/*                                                                          */
#ifndef m520xsim_h
#define m520xsim_h
/*                                                                          */

#define	CPU_NAME		"COLDFIRE(m520x)"
#define	CPU_INSTR_PER_JIFFY	3
#define	MCF_BUSCLK		(MCF_CLK / 2)

#include <asm/m52xxacr.h>

/*
                                               
 */
#define MCFICM_INTC0        0xFC048000  /*                           */
#define MCFINTC_IPRH        0x00        /*                         */
#define MCFINTC_IPRL        0x04        /*                        */
#define MCFINTC_IMRH        0x08        /*                      */
#define MCFINTC_IMRL        0x0c        /*                     */
#define MCFINTC_INTFRCH     0x10        /*                       */
#define MCFINTC_INTFRCL     0x14        /*                      */
#define MCFINTC_SIMR        0x1c        /*                         */
#define MCFINTC_CIMR        0x1d        /*                           */
#define MCFINTC_ICR0        0x40        /*                   */

/*
                                                                        
                                                                     
                                                
 */
#define MCFINTC0_SIMR       (MCFICM_INTC0 + MCFINTC_SIMR)
#define MCFINTC0_CIMR       (MCFICM_INTC0 + MCFINTC_CIMR)
#define	MCFINTC0_ICR0       (MCFICM_INTC0 + MCFINTC_ICR0)
#define MCFINTC1_SIMR       (0)
#define MCFINTC1_CIMR       (0)
#define	MCFINTC1_ICR0       (0)

#define MCFINT_VECBASE      64
#define MCFINT_UART0        26          /*                            */
#define MCFINT_UART1        27          /*                            */
#define MCFINT_UART2        28          /*                            */
#define MCFINT_QSPI         31          /*                           */
#define MCFINT_FECRX0	    36		/*                             */
#define MCFINT_FECTX0	    40		/*                             */
#define MCFINT_FECENTC0	    42		/*                             */
#define MCFINT_PIT1         4           /*                                               */

#define MCF_IRQ_UART0	    (MCFINT_VECBASE + MCFINT_UART0)
#define MCF_IRQ_UART1	    (MCFINT_VECBASE + MCFINT_UART1)
#define MCF_IRQ_UART2	    (MCFINT_VECBASE + MCFINT_UART2)

#define MCF_IRQ_FECRX0	    (MCFINT_VECBASE + MCFINT_FECRX0)
#define MCF_IRQ_FECTX0	    (MCFINT_VECBASE + MCFINT_FECTX0)
#define MCF_IRQ_FECENTC0    (MCFINT_VECBASE + MCFINT_FECENTC0)

#define	MCF_IRQ_QSPI	    (MCFINT_VECBASE + MCFINT_QSPI)

/*
                                  
 */
#define MCFSIM_SDMR         0xFC0a8000	/*                                   */
#define MCFSIM_SDCR         0xFC0a8004	/*                        */
#define MCFSIM_SDCFG1       0xFC0a8008	/*                                */
#define MCFSIM_SDCFG2       0xFC0a800c	/*                                */
#define MCFSIM_SDCS0        0xFC0a8110	/*                                   */
#define MCFSIM_SDCS1        0xFC0a8114	/*                                   */

/*
                            
 */
#define MCFEPORT_EPPAR			0xFC088000
#define MCFEPORT_EPDDR			0xFC088002
#define MCFEPORT_EPIER			0xFC088003
#define MCFEPORT_EPDR			0xFC088004
#define MCFEPORT_EPPDR			0xFC088005
#define MCFEPORT_EPFR			0xFC088006

#define MCFGPIO_PODR_BUSCTL		0xFC0A4000
#define MCFGPIO_PODR_BE			0xFC0A4001
#define MCFGPIO_PODR_CS			0xFC0A4002
#define MCFGPIO_PODR_FECI2C		0xFC0A4003
#define MCFGPIO_PODR_QSPI		0xFC0A4004
#define MCFGPIO_PODR_TIMER		0xFC0A4005
#define MCFGPIO_PODR_UART		0xFC0A4006
#define MCFGPIO_PODR_FECH		0xFC0A4007
#define MCFGPIO_PODR_FECL		0xFC0A4008

#define MCFGPIO_PDDR_BUSCTL		0xFC0A400C
#define MCFGPIO_PDDR_BE			0xFC0A400D
#define MCFGPIO_PDDR_CS			0xFC0A400E
#define MCFGPIO_PDDR_FECI2C		0xFC0A400F
#define MCFGPIO_PDDR_QSPI		0xFC0A4010
#define MCFGPIO_PDDR_TIMER		0xFC0A4011
#define MCFGPIO_PDDR_UART		0xFC0A4012
#define MCFGPIO_PDDR_FECH		0xFC0A4013
#define MCFGPIO_PDDR_FECL		0xFC0A4014

#define MCFGPIO_PPDSDR_CS		0xFC0A401A
#define MCFGPIO_PPDSDR_FECI2C		0xFC0A401B
#define MCFGPIO_PPDSDR_QSPI		0xFC0A401C
#define MCFGPIO_PPDSDR_TIMER		0xFC0A401D
#define MCFGPIO_PPDSDR_UART		0xFC0A401E
#define MCFGPIO_PPDSDR_FECH		0xFC0A401F
#define MCFGPIO_PPDSDR_FECL		0xFC0A4020

#define MCFGPIO_PCLRR_BUSCTL		0xFC0A4024
#define MCFGPIO_PCLRR_BE		0xFC0A4025
#define MCFGPIO_PCLRR_CS		0xFC0A4026
#define MCFGPIO_PCLRR_FECI2C		0xFC0A4027
#define MCFGPIO_PCLRR_QSPI		0xFC0A4028
#define MCFGPIO_PCLRR_TIMER		0xFC0A4029
#define MCFGPIO_PCLRR_UART		0xFC0A402A
#define MCFGPIO_PCLRR_FECH		0xFC0A402B
#define MCFGPIO_PCLRR_FECL		0xFC0A402C

/*
                       
 */
#define MCFGPIO_PODR			MCFGPIO_PODR_CS
#define MCFGPIO_PDDR			MCFGPIO_PDDR_CS
#define MCFGPIO_PPDR			MCFGPIO_PPDSDR_CS
#define MCFGPIO_SETR			MCFGPIO_PPDSDR_CS
#define MCFGPIO_CLRR			MCFGPIO_PCLRR_CS

#define MCFGPIO_PIN_MAX			80
#define MCFGPIO_IRQ_MAX			8
#define MCFGPIO_IRQ_VECBASE		MCFINT_VECBASE

#define MCF_GPIO_PAR_UART		0xFC0A4036
#define MCF_GPIO_PAR_FECI2C		0xFC0A4033
#define MCF_GPIO_PAR_QSPI		0xFC0A4034
#define MCF_GPIO_PAR_FEC		0xFC0A4038

#define MCF_GPIO_PAR_UART_PAR_URXD0         (0x0001)
#define MCF_GPIO_PAR_UART_PAR_UTXD0         (0x0002)

#define MCF_GPIO_PAR_UART_PAR_URXD1         (0x0040)
#define MCF_GPIO_PAR_UART_PAR_UTXD1         (0x0080)

#define MCF_GPIO_PAR_FECI2C_PAR_SDA_URXD2   (0x02)
#define MCF_GPIO_PAR_FECI2C_PAR_SCL_UTXD2   (0x04)

/*
                     
 */
#define	MCFPIT_BASE1		0xFC080000	/*                        */
#define	MCFPIT_BASE2		0xFC084000	/*                        */

/*
                
 */
#define MCFUART_BASE0		0xFC060000	/*                       */
#define MCFUART_BASE1		0xFC064000	/*                       */
#define MCFUART_BASE2		0xFC068000	/*                       */

/*
               
 */
#define	MCFFEC_BASE0		0xFC030000	/*                      */
#define	MCFFEC_SIZE0		0x800		/*                   */

/*
                
 */
#define	MCFQSPI_BASE		0xFC05C000	/*                     */
#define	MCFQSPI_SIZE		0x40		/*                   */

#define	MCFQSPI_CS0		46
#define	MCFQSPI_CS1		47
#define	MCFQSPI_CS2		27

/*
                       
 */
#define	MCF_RCR			0xFC0A0000
#define	MCF_RSR			0xFC0A0001

#define	MCF_RCR_SWRESET		0x80		/*                    */
#define	MCF_RCR_FRCSTOUT	0x40		/*                      */

/*                                                                          */
#endif  /*            */
