/* Copyright (c) 2012, The Linux Foundation. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#ifndef __MSM_CAM_IRQROUTER_H__
#define __MSM_CAM_IRQROUTER_H__

#include <linux/bitops.h>

/*                                            */
/*                                            
                                               
                                                       
                                                        
                                                        
                                                       */

/*                                        */
#define CAMSS_HW_VERSION			0x00000000

/*                                                          
                                                               */
#define CAMSS_TESTBUS_SEL			0x00000004

/*                                                                  
                                                  
                                                                   
                                                                */
#define CAMSS_GPIO_MUX_SEL			0x00000008

/*                                                             
                                                          */
#define CAMSS_AHB_ARB_CTL			0x0000000C

/*           */
#define CAMSS_XPU2_STATUS			0x00000010

/*                                                       */
#define CAMSS_CSI_PIX_CLK_MUX_SEL		0x00000020
#define CAMSS_CSI_PIX_CLK_CGC_EN		0x00000024

/*                                                     */
#define CAMSS_CSI_RDI_CLK_MUX_SEL		0x00000028
#define CAMSS_CSI_RDI_CLK_CGC_EN		0x0000002C

/*                                               */
#define CAMSS_CSI_PHY_0_CLK_MUX_SEL		0x00000030
#define CAMSS_CSI_PHY_0_CLK_CGC_EN		0x00000034

/*                                               */
#define CAMSS_CSI_PHY_1_CLK_MUX_SEL		0x00000038
#define CAMSS_CSI_PHY_1_CLK_CGC_EN		0x0000003C

/*                                               */
#define CAMSS_CSI_PHY_2_CLK_MUX_SEL		0x00000040
#define CAMSS_CSI_PHY_2_CLK_CGC_EN		0x00000044
/*                                          */

/*                                      */
/*                                             
                                           */
#define CAMSS_IRQ_COMPOSITE_RESET_CTRL		0x00000060

/*                                              
                                              
                   */
#define CAMSS_IRQ_COMPOSITE_MASK_0		0x00000064

/*                                              
                                            
                   */
#define CAMSS_IRQ_COMPOSITE_MASK_1		0x00000068

/*                                              
                                              
                   */
#define CAMSS_IRQ_COMPOSITE_MASK_2		0x0000006C

/*                                              
                                              
                   */
#define CAMSS_IRQ_COMPOSITE_MASK_3		0x00000070

/*                                              
                                              
                   */
#define CAMSS_IRQ_COMPOSITE_MASK_4		0x00000074

/*                                              
                                              
                   */
#define CAMSS_IRQ_COMPOSITE_MASK_5		0x00000078

/*                                              
                                              
                   */
#define CAMSS_IRQ_COMPOSITE_MASK_6		0x0000007C

/*                                              
                                            
                   */
#define CAMSS_IRQ_COMPOSITE_MASK_7		0x00000080

/*                                              
                                              
                   */
#define CAMSS_IRQ_COMPOSITE_MASK_8		0x00000084

/*                                              
                                              
                   */
#define CAMSS_IRQ_COMPOSITE_MASK_9		0x00000088

/*                                              
                                               
                   */
#define CAMSS_IRQ_COMPOSITE_MASK_10		0x0000008C

/*                                              
                                               
                   */
#define CAMSS_IRQ_COMPOSITE_MASK_11		0x00000090

/*                                              
                                               
                   */
#define CAMSS_IRQ_COMPOSITE_MASK_12		0x00000094

/*                                                 
                                              
                                                 */
#define CAMSS_IRQ_COMPOSITE_MICRO_MASK_0	0x000000A4
#define CAMSS_IRQ_COMPOSITE_MICRO_MASK_1	0x000000A8
#define CAMSS_IRQ_COMPOSITE_MICRO_MASK_2	0x000000AC
#define CAMSS_IRQ_COMPOSITE_MICRO_MASK_3	0x000000B0
#define CAMSS_IRQ_COMPOSITE_MICRO_MASK_4	0x000000B4
#define CAMSS_IRQ_COMPOSITE_MICRO_MASK_5	0x000000B8
/*                                   */

/*                                               
                                    */
#define CAMSS_IRQ_COMPOSITE_RESET_MASK		0x003F1FFF

/*                                             
                       */
#define ENABLE_MICRO_IRQ_IN_COMPOSITE		BIT(0)
/*                                           
                       */
#define ENABLE_CCI_IRQ_IN_COMPOSITE		BIT(1)
/*                                            
                       */
#define ENABLE_CSI0_IRQ_IN_COMPOSITE		BIT(2)
/*                                            
                       */
#define ENABLE_CSI1_IRQ_IN_COMPOSITE		BIT(3)
/*                                            
                       */
#define ENABLE_CSI2_IRQ_IN_COMPOSITE		BIT(4)
/*                                            
                       */
#define ENABLE_CSI3_IRQ_IN_COMPOSITE		BIT(5)
/*                                             
                       */
#define ENABLE_ISPIF_IRQ_IN_COMPOSITE		BIT(6)
/*                                           
                       */
#define ENABLE_CPP_IRQ_IN_COMPOSITE		BIT(7)
/*                                            
                       */
#define ENABLE_VFE0_IRQ_IN_COMPOSITE		BIT(8)
/*                                            
                       */
#define ENABLE_VFE1_IRQ_IN_COMPOSITE		BIT(9)
/*                                             
                       */
#define ENABLE_JPEG0_IRQ_IN_COMPOSITE		BIT(10)
/*                                             
                       */
#define ENABLE_JPEG1_IRQ_IN_COMPOSITE		BIT(11)
/*                                             
                       */
#define ENABLE_JPEG2_IRQ_IN_COMPOSITE		BIT(12)

struct irqrouter_ctrl_type {
	/*             */
	struct v4l2_subdev subdev;
	struct platform_device *pdev;

	void __iomem *irqr_dev_base;

	struct resource	*irqr_dev_mem;
	struct resource *irqr_dev_io;
	atomic_t active;
	struct msm_cam_server_irqmap_entry def_hw_irqmap[CAMERA_SS_IRQ_MAX];
};

#endif /*                         */
