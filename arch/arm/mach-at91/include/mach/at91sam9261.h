/*
 * arch/arm/mach-at91/include/mach/at91sam9261.h
 *
 * Copyright (C) SAN People
 *
 * Common definitions.
 * Based on AT91SAM9261 datasheet revision E. (Preliminary)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#ifndef AT91SAM9261_H
#define AT91SAM9261_H

/*
                                     
 */
#define AT91SAM9261_ID_PIOA	2	/*                          */
#define AT91SAM9261_ID_PIOB	3	/*                          */
#define AT91SAM9261_ID_PIOC	4	/*                          */
#define AT91SAM9261_ID_US0	6	/*         */
#define AT91SAM9261_ID_US1	7	/*         */
#define AT91SAM9261_ID_US2	8	/*         */
#define AT91SAM9261_ID_MCI	9	/*                           */
#define AT91SAM9261_ID_UDP	10	/*                 */
#define AT91SAM9261_ID_TWI	11	/*                    */
#define AT91SAM9261_ID_SPI0	12	/*                               */
#define AT91SAM9261_ID_SPI1	13	/*                               */
#define AT91SAM9261_ID_SSC0	14	/*                                 */
#define AT91SAM9261_ID_SSC1	15	/*                                 */
#define AT91SAM9261_ID_SSC2	16	/*                                 */
#define AT91SAM9261_ID_TC0	17	/*                 */
#define AT91SAM9261_ID_TC1	18	/*                 */
#define AT91SAM9261_ID_TC2	19	/*                 */
#define AT91SAM9261_ID_UHP	20	/*               */
#define AT91SAM9261_ID_LCDC	21	/*                */
#define AT91SAM9261_ID_IRQ0	29	/*                                      */
#define AT91SAM9261_ID_IRQ1	30	/*                                      */
#define AT91SAM9261_ID_IRQ2	31	/*                                      */


/*
                                           
 */
#define AT91SAM9261_BASE_TCB0		0xfffa0000
#define AT91SAM9261_BASE_TC0		0xfffa0000
#define AT91SAM9261_BASE_TC1		0xfffa0040
#define AT91SAM9261_BASE_TC2		0xfffa0080
#define AT91SAM9261_BASE_UDP		0xfffa4000
#define AT91SAM9261_BASE_MCI		0xfffa8000
#define AT91SAM9261_BASE_TWI		0xfffac000
#define AT91SAM9261_BASE_US0		0xfffb0000
#define AT91SAM9261_BASE_US1		0xfffb4000
#define AT91SAM9261_BASE_US2		0xfffb8000
#define AT91SAM9261_BASE_SSC0		0xfffbc000
#define AT91SAM9261_BASE_SSC1		0xfffc0000
#define AT91SAM9261_BASE_SSC2		0xfffc4000
#define AT91SAM9261_BASE_SPI0		0xfffc8000
#define AT91SAM9261_BASE_SPI1		0xfffcc000


/*
                     
 */
#define AT91SAM9261_BASE_SMC	0xffffec00
#define AT91SAM9261_BASE_MATRIX	0xffffee00
#define AT91SAM9261_BASE_SDRAMC	0xffffea00
#define AT91SAM9261_BASE_DBGU	AT91_BASE_DBGU0
#define AT91SAM9261_BASE_PIOA	0xfffff400
#define AT91SAM9261_BASE_PIOB	0xfffff600
#define AT91SAM9261_BASE_PIOC	0xfffff800
#define AT91SAM9261_BASE_RSTC	0xfffffd00
#define AT91SAM9261_BASE_SHDWC	0xfffffd10
#define AT91SAM9261_BASE_RTT	0xfffffd20
#define AT91SAM9261_BASE_PIT	0xfffffd30
#define AT91SAM9261_BASE_WDT	0xfffffd40
#define AT91SAM9261_BASE_GPBR	0xfffffd50

#define AT91_USART0	AT91SAM9261_BASE_US0
#define AT91_USART1	AT91SAM9261_BASE_US1
#define AT91_USART2	AT91SAM9261_BASE_US2


/*
                   
 */
#define AT91SAM9261_SRAM_BASE	0x00300000	/*                            */
#define AT91SAM9261_SRAM_SIZE	0x00028000	/*                            */

#define AT91SAM9G10_SRAM_BASE	AT91SAM9261_SRAM_BASE	/*                            */
#define AT91SAM9G10_SRAM_SIZE	0x00004000	/*                           */

#define AT91SAM9261_ROM_BASE	0x00400000	/*                           */
#define AT91SAM9261_ROM_SIZE	SZ_32K		/*                          */

#define AT91SAM9261_UHP_BASE	0x00500000	/*                     */
#define AT91SAM9261_LCDC_BASE	0x00600000	/*                */


#endif
