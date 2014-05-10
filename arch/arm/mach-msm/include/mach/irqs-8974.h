/* Copyright (c) 2011-2012, The Linux Foundation. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#ifndef __ASM_ARCH_MSM_IRQS_8974_H
#define __ASM_ARCH_MSM_IRQS_8974_H

/*                            */

/*
                                                           
                                             
                                            
 */

#define GIC_PPI_START 16
#define GIC_SPI_START 32

#define AVS_SVICINT				(GIC_PPI_START + 6)
#define AVS_SVICINTSWDONE			(GIC_PPI_START + 7)
#define INT_ARMQC_PERFMON			(GIC_PPI_START + 10)
/*                  */

#define APCC_QGICL2PERFMONIRPTREQ	(GIC_SPI_START + 1)
#define SC_SICL2PERFMONIRPTREQ		APCC_QGICL2PERFMONIRPTREQ
#define TLMM_MSM_SUMMARY_IRQ		(GIC_SPI_START + 208)
#define SPS_BAM_DMA_IRQ			(GIC_SPI_START + 105)

#define NR_MSM_IRQS 1020 /*                                              */
#define NR_GPIO_IRQS 146
#define NR_QPNP_IRQS 32768 /*                                        */
#define NR_BOARD_IRQS NR_QPNP_IRQS
#define NR_TLMM_MSM_DIR_CONN_IRQ 8
#define NR_MSM_GPIOS NR_GPIO_IRQS

#endif

