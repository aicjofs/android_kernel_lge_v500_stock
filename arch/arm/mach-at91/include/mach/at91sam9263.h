/*
 * arch/arm/mach-at91/include/mach/at91sam9263.h
 *
 * (C) 2007 Atmel Corporation.
 *
 * Common definitions.
 * Based on AT91SAM9263 datasheet revision B (Preliminary).
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#ifndef AT91SAM9263_H
#define AT91SAM9263_H

/*
                                     
 */
#define AT91SAM9263_ID_PIOA	2	/*                          */
#define AT91SAM9263_ID_PIOB	3	/*                          */
#define AT91SAM9263_ID_PIOCDE	4	/*                                   */
#define AT91SAM9263_ID_US0	7	/*         */
#define AT91SAM9263_ID_US1	8	/*         */
#define AT91SAM9263_ID_US2	9	/*         */
#define AT91SAM9263_ID_MCI0	10	/*                             */
#define AT91SAM9263_ID_MCI1	11	/*                             */
#define AT91SAM9263_ID_CAN	12	/*     */
#define AT91SAM9263_ID_TWI	13	/*                    */
#define AT91SAM9263_ID_SPI0	14	/*                               */
#define AT91SAM9263_ID_SPI1	15	/*                               */
#define AT91SAM9263_ID_SSC0	16	/*                                 */
#define AT91SAM9263_ID_SSC1	17	/*                                 */
#define AT91SAM9263_ID_AC97C	18	/*                 */
#define AT91SAM9263_ID_TCB	19	/*                          */
#define AT91SAM9263_ID_PWMC	20	/*                                   */
#define AT91SAM9263_ID_EMAC	21	/*          */
#define AT91SAM9263_ID_2DGE	23	/*                   */
#define AT91SAM9263_ID_UDP	24	/*                 */
#define AT91SAM9263_ID_ISI	25	/*                        */
#define AT91SAM9263_ID_LCDC	26	/*                */
#define AT91SAM9263_ID_DMA	27	/*                */
#define AT91SAM9263_ID_UHP	29	/*               */
#define AT91SAM9263_ID_IRQ0	30	/*                                      */
#define AT91SAM9263_ID_IRQ1	31	/*                                      */


/*
                                           
 */
#define AT91SAM9263_BASE_UDP		0xfff78000
#define AT91SAM9263_BASE_TCB0		0xfff7c000
#define AT91SAM9263_BASE_TC0		0xfff7c000
#define AT91SAM9263_BASE_TC1		0xfff7c040
#define AT91SAM9263_BASE_TC2		0xfff7c080
#define AT91SAM9263_BASE_MCI0		0xfff80000
#define AT91SAM9263_BASE_MCI1		0xfff84000
#define AT91SAM9263_BASE_TWI		0xfff88000
#define AT91SAM9263_BASE_US0		0xfff8c000
#define AT91SAM9263_BASE_US1		0xfff90000
#define AT91SAM9263_BASE_US2		0xfff94000
#define AT91SAM9263_BASE_SSC0		0xfff98000
#define AT91SAM9263_BASE_SSC1		0xfff9c000
#define AT91SAM9263_BASE_AC97C		0xfffa0000
#define AT91SAM9263_BASE_SPI0		0xfffa4000
#define AT91SAM9263_BASE_SPI1		0xfffa8000
#define AT91SAM9263_BASE_CAN		0xfffac000
#define AT91SAM9263_BASE_PWMC		0xfffb8000
#define AT91SAM9263_BASE_EMAC		0xfffbc000
#define AT91SAM9263_BASE_ISI		0xfffc4000
#define AT91SAM9263_BASE_2DGE		0xfffc8000

/*
                     
 */
#define AT91SAM9263_BASE_ECC0	0xffffe000
#define AT91SAM9263_BASE_SDRAMC0 0xffffe200
#define AT91SAM9263_BASE_SMC0	0xffffe400
#define AT91SAM9263_BASE_ECC1	0xffffe600
#define AT91SAM9263_BASE_SDRAMC1 0xffffe800
#define AT91SAM9263_BASE_SMC1	0xffffea00
#define AT91SAM9263_BASE_MATRIX	0xffffec00
#define AT91SAM9263_BASE_DBGU	AT91_BASE_DBGU1
#define AT91SAM9263_BASE_PIOA	0xfffff200
#define AT91SAM9263_BASE_PIOB	0xfffff400
#define AT91SAM9263_BASE_PIOC	0xfffff600
#define AT91SAM9263_BASE_PIOD	0xfffff800
#define AT91SAM9263_BASE_PIOE	0xfffffa00
#define AT91SAM9263_BASE_RSTC	0xfffffd00
#define AT91SAM9263_BASE_SHDWC	0xfffffd10
#define AT91SAM9263_BASE_RTT0	0xfffffd20
#define AT91SAM9263_BASE_PIT	0xfffffd30
#define AT91SAM9263_BASE_WDT	0xfffffd40
#define AT91SAM9263_BASE_RTT1	0xfffffd50
#define AT91SAM9263_BASE_GPBR	0xfffffd60

#define AT91_USART0	AT91SAM9263_BASE_US0
#define AT91_USART1	AT91SAM9263_BASE_US1
#define AT91_USART2	AT91SAM9263_BASE_US2

#define AT91_SMC	AT91_SMC0

/*
                   
 */
#define AT91SAM9263_SRAM0_BASE	0x00300000	/*                              */
#define AT91SAM9263_SRAM0_SIZE	(80 * SZ_1K)	/*                             */

#define AT91SAM9263_ROM_BASE	0x00400000	/*                           */
#define AT91SAM9263_ROM_SIZE	SZ_128K		/*                           */

#define AT91SAM9263_SRAM1_BASE	0x00500000	/*                              */
#define AT91SAM9263_SRAM1_SIZE	SZ_16K		/*                             */

#define AT91SAM9263_LCDC_BASE	0x00700000	/*                */
#define AT91SAM9263_DMAC_BASE	0x00800000	/*                */
#define AT91SAM9263_UHP_BASE	0x00a00000	/*                     */


#endif
