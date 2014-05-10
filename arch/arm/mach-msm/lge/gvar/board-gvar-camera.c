/* Copyright (c) 2012-2013, The Linux Foundation. All rights reserved.
 * Copyright (c) 2012, LGE Inc.
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

#include <asm/mach-types.h>
#include <linux/i2c.h>
#include <linux/gpio.h>
#include <mach/board.h>
#include <mach/msm_bus_board.h>
#include <mach/gpiomux.h>
#include <mach/socinfo.h>
#include <media/msm_camera.h>
#include "devices.h"
#include "board-gvar.h"
#include <mach/board_lge.h>

#ifdef CONFIG_MSM_CAMERA
static struct gpiomux_setting cam_settings[] = {
	{
		.func = GPIOMUX_FUNC_GPIO, /*       */
		.drv = GPIOMUX_DRV_2MA,
		.pull = GPIOMUX_PULL_DOWN,
	},

	{
		.func = GPIOMUX_FUNC_1, /*        */
		.drv = GPIOMUX_DRV_2MA,
		.pull = GPIOMUX_PULL_NONE,
	},

	{
		.func = GPIOMUX_FUNC_GPIO, /*        */
		.drv = GPIOMUX_DRV_2MA,
		.pull = GPIOMUX_PULL_NONE,
	},

	{
		.func = GPIOMUX_FUNC_1, /*        */
		.drv = GPIOMUX_DRV_8MA,
		.pull = GPIOMUX_PULL_NONE,
	},

	{
		.func = GPIOMUX_FUNC_4, /*        */
		.drv = GPIOMUX_DRV_2MA,
		.pull = GPIOMUX_PULL_NONE,
	},

	{
		.func = GPIOMUX_FUNC_6, /*        */
		.drv = GPIOMUX_DRV_8MA,
		.pull = GPIOMUX_PULL_UP,
	},

	{
		.func = GPIOMUX_FUNC_2, /*        */
		.drv = GPIOMUX_DRV_2MA,
		.pull = GPIOMUX_PULL_UP,
	},

	{
		.func = GPIOMUX_FUNC_3, /*        */
		.drv = GPIOMUX_DRV_8MA,
		.pull = GPIOMUX_PULL_UP,
	},

	{
		.func = GPIOMUX_FUNC_GPIO, /*           */
		.drv = GPIOMUX_DRV_2MA,
		.pull = GPIOMUX_PULL_KEEPER,
	},

	{
		.func = GPIOMUX_FUNC_9, /*        */
		.drv = GPIOMUX_DRV_8MA,
		.pull = GPIOMUX_PULL_NONE,
	},
	{
		.func = GPIOMUX_FUNC_A, /*         */
		.drv = GPIOMUX_DRV_8MA,
		.pull = GPIOMUX_PULL_NONE,
	},
	{
		.func = GPIOMUX_FUNC_6, /*         */
		.drv = GPIOMUX_DRV_8MA,
		.pull = GPIOMUX_PULL_NONE,
	},
	{
		.func = GPIOMUX_FUNC_4, /*         */
		.drv = GPIOMUX_DRV_2MA,
		.pull = GPIOMUX_PULL_NONE,
	},

};

static struct msm_gpiomux_config apq8064_cam_common_configs[] = {
/*                                                                                                                      */
#if !defined(CONFIG_MACH_APQ8064_GVAR_CMCC)
	{
		.gpio = GPIO_CAM_FLASH_EN, /*   */
		.settings = {
			[GPIOMUX_ACTIVE]    = &cam_settings[2],
			[GPIOMUX_SUSPENDED] = &cam_settings[0],
		},
	},
#endif
/*                                                                                     */
	{
		.gpio = GPIO_CAM_MCLK0, /*   */
		.settings = {
			[GPIOMUX_ACTIVE]    = &cam_settings[1],
			[GPIOMUX_SUSPENDED] = &cam_settings[0],
		},
	},

/*                                                       */
#if 1
	{
		.gpio = GPIO_CAM_MCLK2, /*   */
		.settings = {
			[GPIOMUX_ACTIVE]    = &cam_settings[4],
			[GPIOMUX_SUSPENDED] = &cam_settings[0],
		},
	},
#else
	{
		.gpio = GPIO_CAM_MCLK1, /*   */
		.settings = {
			[GPIOMUX_ACTIVE]    = &cam_settings[1],
			[GPIOMUX_SUSPENDED] = &cam_settings[0],
		},
	},
#endif
	{
		.gpio = GPIO_CAM2_RST_N, /*    */
		.settings = {
			[GPIOMUX_ACTIVE]    = &cam_settings[2],
			[GPIOMUX_SUSPENDED] = &cam_settings[0],
		},
	},
	{
		.gpio = GPIO_CAM1_RST_N, /*    */
		.settings = {
			[GPIOMUX_ACTIVE]    = &cam_settings[2],
			[GPIOMUX_SUSPENDED] = &cam_settings[0],
		},
	},
	{
		.gpio = GPIO_CAM_I2C_SDA, /*    */
		.settings = {
			[GPIOMUX_ACTIVE]    = &cam_settings[3],
			[GPIOMUX_SUSPENDED] = &cam_settings[8],
		},
	},
	{
		.gpio = GPIO_CAM_I2C_SCL, /*    */
		.settings = {
			[GPIOMUX_ACTIVE]    = &cam_settings[3],
			[GPIOMUX_SUSPENDED] = &cam_settings[8],
		},
	},
};
/*                                                                          */
static struct msm_gpiomux_config apq8064_cam_common_configs_revE[] = {
/*                                                                                                                      */
#if !defined(CONFIG_MACH_APQ8064_GVAR_CMCC)
	{
		.gpio = GPIO_CAM_FLASH_EN, /*   */
		.settings = {
			[GPIOMUX_ACTIVE]    = &cam_settings[2],
			[GPIOMUX_SUSPENDED] = &cam_settings[0],
		},
	},
#endif
/*                                                                                     */
	{
		.gpio = GPIO_CAM_MCLK0, /*   */
		.settings = {
			[GPIOMUX_ACTIVE]    = &cam_settings[1],
			[GPIOMUX_SUSPENDED] = &cam_settings[0],
		},
	},

/*                                                       */
#if 1
	{
		.gpio = GPIO_CAM_MCLK2, /*   */
		.settings = {
			[GPIOMUX_ACTIVE]    = &cam_settings[4],
			[GPIOMUX_SUSPENDED] = &cam_settings[0],
		},
	},
#else
	{
		.gpio = GPIO_CAM_MCLK1, /*   */
		.settings = {
			[GPIOMUX_ACTIVE]    = &cam_settings[1],
			[GPIOMUX_SUSPENDED] = &cam_settings[0],
		},
	},
#endif
#if 0
	{
		.gpio = GPIO_CAM2_RST_N, /*    */
		.settings = {
			[GPIOMUX_ACTIVE]    = &cam_settings[2],
			[GPIOMUX_SUSPENDED] = &cam_settings[0],
		},
	},
	{
		.gpio = GPIO_CAM1_RST_N, /*    */
		.settings = {
			[GPIOMUX_ACTIVE]    = &cam_settings[2],
			[GPIOMUX_SUSPENDED] = &cam_settings[0],
		},
	},
#endif
	{
		.gpio = GPIO_CAM_I2C_SDA, /*    */
		.settings = {
			[GPIOMUX_ACTIVE]    = &cam_settings[3],
			[GPIOMUX_SUSPENDED] = &cam_settings[8],
		},
	},
	{
		.gpio = GPIO_CAM_I2C_SCL, /*    */
		.settings = {
			[GPIOMUX_ACTIVE]    = &cam_settings[3],
			[GPIOMUX_SUSPENDED] = &cam_settings[8],
		},
	},
};
/*                                                                        */

/*                                                                              */
#if defined(CONFIG_CE1702)
static struct msm_gpiomux_config apq8064_cam_2d_configs[] = {
};

static struct msm_bus_vectors cam_init_vectors[] = {
	{
		.src = MSM_BUS_MASTER_VFE,
		.dst = MSM_BUS_SLAVE_EBI_CH0,
		.ab  = 0,
		.ib  = 0,
	},
	{
		.src = MSM_BUS_MASTER_VPE,
		.dst = MSM_BUS_SLAVE_EBI_CH0,
		.ab  = 0,
		.ib  = 0,
	},
	{
		.src = MSM_BUS_MASTER_JPEG_ENC,
		.dst = MSM_BUS_SLAVE_EBI_CH0,
		.ab  = 0,
		.ib  = 0,
	},
};

static struct msm_bus_vectors cam_preview_vectors[] = {
	{
		.src = MSM_BUS_MASTER_VFE,
		.dst = MSM_BUS_SLAVE_EBI_CH0,
		.ab  = 194735543,//                                                                                                                                        
		.ib  = 292103315,//                                                                                                                                        
/*
                                                                                       
                                                                   
*/
	},
	{
		.src = MSM_BUS_MASTER_VPE,
		.dst = MSM_BUS_SLAVE_EBI_CH0,
		.ab  = 0,
		.ib  = 0,
	},
	{
		.src = MSM_BUS_MASTER_JPEG_ENC,
		.dst = MSM_BUS_SLAVE_EBI_CH0,
		.ab  = 0,
		.ib  = 0,
	},
};

static struct msm_bus_vectors cam_video_vectors[] = {
	{
		.src = MSM_BUS_MASTER_VFE,
		.dst = MSM_BUS_SLAVE_EBI_CH0,
		.ab  = 274406400,//                                                                                                                                         
		.ib  = 561807360,
	},
	{
		.src = MSM_BUS_MASTER_VPE,
		.dst = MSM_BUS_SLAVE_EBI_CH0,
		.ab  = 206807040,
		.ib  = 488816640,
	},
	{
		.src = MSM_BUS_MASTER_JPEG_ENC,
		.dst = MSM_BUS_SLAVE_EBI_CH0,
		.ab  = 0,
		.ib  = 0,
	},
};

static struct msm_bus_vectors cam_snapshot_vectors[] = {
	{
		.src = MSM_BUS_MASTER_VFE,
		.dst = MSM_BUS_SLAVE_EBI_CH0,
		.ab  = 274423680,
		.ib  = 1097694720,
	},
	{
		.src = MSM_BUS_MASTER_VPE,
		.dst = MSM_BUS_SLAVE_EBI_CH0,
		.ab  = 0,
		.ib  = 0,
	},
	{
		.src = MSM_BUS_MASTER_JPEG_ENC,
		.dst = MSM_BUS_SLAVE_EBI_CH0,
		.ab  = 540000000,
		.ib  = 1350000000,
	},
};

static struct msm_bus_vectors cam_zsl_vectors[] = {
	{
		.src = MSM_BUS_MASTER_VFE,
		.dst = MSM_BUS_SLAVE_EBI_CH0,
		.ab  = 302071680,
		.ib  = 1812430080,
	},
	{
		.src = MSM_BUS_MASTER_VPE,
		.dst = MSM_BUS_SLAVE_EBI_CH0,
		.ab  = 0,
		.ib  = 0,
	},
	{
		.src = MSM_BUS_MASTER_JPEG_ENC,
		.dst = MSM_BUS_SLAVE_EBI_CH0,
		.ab  = 540000000,
		.ib  = 2025000000,
	},
};

static struct msm_bus_vectors cam_video_ls_vectors[] = {
	{
		.src = MSM_BUS_MASTER_VFE,
		.dst = MSM_BUS_SLAVE_EBI_CH0,
		.ab  = 348192000,
		.ib  = 617103360,
	},
	{
		.src = MSM_BUS_MASTER_VPE,
		.dst = MSM_BUS_SLAVE_EBI_CH0,
		.ab  = 206807040,
		.ib  = 488816640,
	},
	{
		.src = MSM_BUS_MASTER_JPEG_ENC,
		.dst = MSM_BUS_SLAVE_EBI_CH0,
		.ab  = 540000000,
		.ib  = 1350000000,
	},
};

static struct msm_bus_vectors cam_dual_vectors[] = {
	{
		.src = MSM_BUS_MASTER_VFE,
		.dst = MSM_BUS_SLAVE_EBI_CH0,
		.ab  = 348192000,
		.ib  = 1208286720,
	},
	{
		.src = MSM_BUS_MASTER_VPE,
		.dst = MSM_BUS_SLAVE_EBI_CH0,
		.ab  = 206807040,
		.ib  = 488816640,
	},
	{
		.src = MSM_BUS_MASTER_JPEG_ENC,
		.dst = MSM_BUS_SLAVE_EBI_CH0,
		.ab  = 540000000,
		.ib  = 1350000000,
	},
	{
		.src = MSM_BUS_MASTER_JPEG_ENC,
		.dst = MSM_BUS_SLAVE_MM_IMEM,
		.ab  = 43200000,
		.ib  = 69120000,
	},
	{
		.src = MSM_BUS_MASTER_VFE,
		.dst = MSM_BUS_SLAVE_MM_IMEM,
		.ab  = 43200000,
		.ib  = 69120000,
	},
};

static struct msm_bus_paths cam_bus_client_config[] = {
	{
		ARRAY_SIZE(cam_init_vectors),
		cam_init_vectors,
	},
	{
		ARRAY_SIZE(cam_preview_vectors),
		cam_preview_vectors,
	},
	{
		ARRAY_SIZE(cam_video_vectors),
		cam_video_vectors,
	},
	{
		ARRAY_SIZE(cam_snapshot_vectors),
		cam_snapshot_vectors,
	},
	{
		ARRAY_SIZE(cam_zsl_vectors),
		cam_zsl_vectors,
	},
	{
		ARRAY_SIZE(cam_video_ls_vectors),
		cam_video_ls_vectors,
	},
	{
		ARRAY_SIZE(cam_dual_vectors),
		cam_dual_vectors,
	},
};

static struct msm_bus_scale_pdata cam_bus_client_pdata = {
		cam_bus_client_config,
		ARRAY_SIZE(cam_bus_client_config),
		.name = "msm_camera",
};

static struct msm_camera_device_platform_data msm_camera_csi_device_data[] = {
	{
		.csid_core = 0,
		.is_vpe    = 1,
		.cam_bus_scale_table = &cam_bus_client_pdata,
	},
	{
		.csid_core = 1,
		.is_vpe    = 1,
		.cam_bus_scale_table = &cam_bus_client_pdata,
	},
};
static struct camera_vreg_t apq_8064_back_cam_vreg[] = {
	{"cam1_vdig", REG_VS, 0, 0, 0, 0},
	{"cam1_vio", REG_VS, 0, 0, 0, 0},
	{"cam1_vana", REG_LDO, 2850000, 2850000, 85600, 0},
	{"cam1_vaf", REG_LDO, 1800000, 1800000, 150000, 0},
	{"cam1_isp_core", REG_LDO, 1150000, 1150000, 260000, 0},
	{"cam1_isp_host", REG_LDO, 1800000, 1800000, 9000, 0},
	{"cam1_isp_ram", REG_LDO, 1800000, 1800000, 115000, 0},
	{"cam1_isp_camif", REG_LDO, 1800000, 1800000, 11000, 0},
	{"cam1_isp_sys", REG_LDO, 2800000, 2800000, 2000, 0},
};


static struct gpio apq8064_common_cam_gpio[] = {
	{12, GPIOF_DIR_IN, "CAMIF_I2C_DATA"},
	{13, GPIOF_DIR_IN, "CAMIF_I2C_CLK"},
};

static struct gpio apq8064_back_cam_gpio[] = {
	{GPIO_CAM_MCLK0, GPIOF_DIR_IN, "CAMIF_MCLK"},
	{GPIO_CAM1_RST_N, GPIOF_DIR_OUT, "CAM_RESET"},
};
/*                                                                          */
static struct gpio apq8064_back_cam_gpio_revE[] = {
	{GPIO_CAM_MCLK0, GPIOF_DIR_IN, "CAMIF_MCLK"},
};
/*                                                                        */

static struct msm_gpio_set_tbl apq8064_back_cam_gpio_set_tbl[] = {
	{GPIO_CAM1_RST_N, GPIOF_OUT_INIT_LOW, 10000},
	{GPIO_CAM1_RST_N, GPIOF_OUT_INIT_HIGH, 10000},
};

static struct msm_camera_gpio_conf apq8064_back_cam_gpio_conf = {
	.cam_gpiomux_conf_tbl = apq8064_cam_2d_configs,
	.cam_gpiomux_conf_tbl_size = ARRAY_SIZE(apq8064_cam_2d_configs),
	.cam_gpio_common_tbl = apq8064_common_cam_gpio,
	.cam_gpio_common_tbl_size = ARRAY_SIZE(apq8064_common_cam_gpio),
	.cam_gpio_req_tbl = apq8064_back_cam_gpio,
	.cam_gpio_req_tbl_size = ARRAY_SIZE(apq8064_back_cam_gpio),
	.cam_gpio_set_tbl = apq8064_back_cam_gpio_set_tbl,
	.cam_gpio_set_tbl_size = ARRAY_SIZE(apq8064_back_cam_gpio_set_tbl),
};
/*                                                                          */
static struct msm_camera_gpio_conf apq8064_back_cam_gpio_conf_revE = {
	.cam_gpiomux_conf_tbl = apq8064_cam_2d_configs,
	.cam_gpiomux_conf_tbl_size = ARRAY_SIZE(apq8064_cam_2d_configs),
	.cam_gpio_common_tbl = apq8064_common_cam_gpio,
	.cam_gpio_common_tbl_size = ARRAY_SIZE(apq8064_common_cam_gpio),
	.cam_gpio_req_tbl = apq8064_back_cam_gpio_revE,
	.cam_gpio_req_tbl_size = ARRAY_SIZE(apq8064_back_cam_gpio_revE),
};
/*                                                                        */

static struct msm_camera_i2c_conf apq8064_back_cam_i2c_conf = {
	.use_i2c_mux = 1,
	.mux_dev = &msm8960_device_i2c_mux_gsbi4,
	.i2c_mux_mode = MODE_L,
};


static struct msm_camera_sensor_flash_data flash_ce1702 = {
	.flash_type	= MSM_CAMERA_FLASH_NONE,
};

static struct msm_camera_csi_lane_params ce1702_csi_lane_params = {
	.csi_lane_assign = 0xE4,
	.csi_lane_mask = 0xF,
};

static struct msm_camera_sensor_platform_info sensor_board_info_ce1702 = {
	.mount_angle	= 90,
	.cam_vreg = apq_8064_back_cam_vreg,
	.num_vreg = ARRAY_SIZE(apq_8064_back_cam_vreg),
	.gpio_conf = &apq8064_back_cam_gpio_conf,
	.i2c_conf = &apq8064_back_cam_i2c_conf,
	.csi_lane_params = &ce1702_csi_lane_params,
};
/*                                                                          */
static struct msm_camera_sensor_platform_info sensor_board_info_ce1702_revE = {
	.mount_angle	= 90,
	.cam_vreg = apq_8064_back_cam_vreg,
	.num_vreg = ARRAY_SIZE(apq_8064_back_cam_vreg),
	.gpio_conf = &apq8064_back_cam_gpio_conf_revE,
	.i2c_conf = &apq8064_back_cam_i2c_conf,
	.csi_lane_params = &ce1702_csi_lane_params,
};
/*                                                                        */

/*
                                                       
                                       
  

                                                    
                                           
                                                 
  
*/
static struct msm_camera_sensor_info msm_camera_sensor_ce1702_data = {
	.sensor_name	= "ce1702",
	.pdata	= &msm_camera_csi_device_data[0],
	.flash_data	= &flash_ce1702,
	.sensor_platform_info = &sensor_board_info_ce1702,
	.csi_if	= 1,
	.camera_type = BACK_CAMERA_2D,
	.sensor_type = YUV_SENSOR,
#ifdef CONFIG_CE1702_ACT
	//                                                     
	//                                        
	.actuator_info = &msm_act_main_cam_0_info,
	//                                                   
#endif
//                                    
};
/*                                                                          */
static struct msm_camera_sensor_info msm_camera_sensor_ce1702_data_revE = {
	.sensor_name	= "ce1702",
	.pdata	= &msm_camera_csi_device_data[0],
	.flash_data	= &flash_ce1702,
	.sensor_platform_info = &sensor_board_info_ce1702_revE,
	.csi_if	= 1,
	.camera_type = BACK_CAMERA_2D,
	.sensor_type = YUV_SENSOR,
#ifdef CONFIG_CE1702_ACT
	//                                                     
	//                                        
	.actuator_info = &msm_act_main_cam_0_info,
	//                                                   
#endif
//                                    
};
/*                                                                        */

#endif
/*                                                                            */

#if defined(CONFIG_IMX111) || defined(CONFIG_IMX091)
static struct msm_gpiomux_config apq8064_cam_2d_configs[] = {
};

static struct msm_bus_vectors cam_init_vectors[] = {
	{
		.src = MSM_BUS_MASTER_VFE,
		.dst = MSM_BUS_SLAVE_EBI_CH0,
		.ab  = 0,
		.ib  = 0,
	},
	{
		.src = MSM_BUS_MASTER_VPE,
		.dst = MSM_BUS_SLAVE_EBI_CH0,
		.ab  = 0,
		.ib  = 0,
	},
	{
		.src = MSM_BUS_MASTER_JPEG_ENC,
		.dst = MSM_BUS_SLAVE_EBI_CH0,
		.ab  = 0,
		.ib  = 0,
	},
};

static struct msm_bus_vectors cam_preview_vectors[] = {
	{
		.src = MSM_BUS_MASTER_VFE,
		.dst = MSM_BUS_SLAVE_EBI_CH0,
		.ab  = 27648000,
		.ib  = 110592000,
	},
	{
		.src = MSM_BUS_MASTER_VPE,
		.dst = MSM_BUS_SLAVE_EBI_CH0,
		.ab  = 0,
		.ib  = 0,
	},
	{
		.src = MSM_BUS_MASTER_JPEG_ENC,
		.dst = MSM_BUS_SLAVE_EBI_CH0,
		.ab  = 0,
		.ib  = 0,
	},
};

static struct msm_bus_vectors cam_video_vectors[] = {
	{
		.src = MSM_BUS_MASTER_VFE,
		.dst = MSM_BUS_SLAVE_EBI_CH0,
		.ab  = 140451840,
		.ib  = 561807360,
	},
	{
		.src = MSM_BUS_MASTER_VPE,
		.dst = MSM_BUS_SLAVE_EBI_CH0,
		.ab  = 206807040,
		.ib  = 488816640,
	},
	{
		.src = MSM_BUS_MASTER_JPEG_ENC,
		.dst = MSM_BUS_SLAVE_EBI_CH0,
		.ab  = 0,
		.ib  = 0,
	},
};

static struct msm_bus_vectors cam_snapshot_vectors[] = {
	{
		.src = MSM_BUS_MASTER_VFE,
		.dst = MSM_BUS_SLAVE_EBI_CH0,
		.ab  = 274423680,
		.ib  = 1097694720,
	},
	{
		.src = MSM_BUS_MASTER_VPE,
		.dst = MSM_BUS_SLAVE_EBI_CH0,
		.ab  = 0,
		.ib  = 0,
	},
	{
		.src = MSM_BUS_MASTER_JPEG_ENC,
		.dst = MSM_BUS_SLAVE_EBI_CH0,
		.ab  = 540000000,
		.ib  = 1350000000,
	},
};

static struct msm_bus_vectors cam_zsl_vectors[] = {
	{
		.src = MSM_BUS_MASTER_VFE,
		.dst = MSM_BUS_SLAVE_EBI_CH0,
		.ab  = 302071680,
		.ib  = 1812430080,
	},
	{
		.src = MSM_BUS_MASTER_VPE,
		.dst = MSM_BUS_SLAVE_EBI_CH0,
		.ab  = 0,
		.ib  = 0,
	},
	{
		.src = MSM_BUS_MASTER_JPEG_ENC,
		.dst = MSM_BUS_SLAVE_EBI_CH0,
		.ab  = 540000000,
		.ib  = 2025000000,
	},
};

static struct msm_bus_paths cam_bus_client_config[] = {
	{
		ARRAY_SIZE(cam_init_vectors),
		cam_init_vectors,
	},
	{
		ARRAY_SIZE(cam_preview_vectors),
		cam_preview_vectors,
	},
	{
		ARRAY_SIZE(cam_video_vectors),
		cam_video_vectors,
	},
	{
		ARRAY_SIZE(cam_snapshot_vectors),
		cam_snapshot_vectors,
	},
	{
		ARRAY_SIZE(cam_zsl_vectors),
		cam_zsl_vectors,
	},
};

static struct msm_bus_scale_pdata cam_bus_client_pdata = {
		cam_bus_client_config,
		ARRAY_SIZE(cam_bus_client_config),
		.name = "msm_camera",
};

static struct msm_camera_device_platform_data msm_camera_csi_device_data[] = {
	{
		.csid_core = 0,
		.is_vpe    = 1,
		.cam_bus_scale_table = &cam_bus_client_pdata,
	},
	{
		.csid_core = 1,
		.is_vpe    = 1,
		.cam_bus_scale_table = &cam_bus_client_pdata,
	},
};
static struct camera_vreg_t apq_8064_back_cam_vreg[] = {
	{"cam1_vdig", REG_LDO, 1200000, 1200000, 105000, 0},
	{"cam1_vio", REG_VS, 0, 0, 0, 0},
	{"cam1_vana", REG_LDO, 2850000, 2850000, 85600, 0},
#if defined(CONFIG_IMX111) || defined(CONFIG_IMX091)
	{"cam1_vaf", REG_LDO, 2800000, 2800000, 300000, 0},
#else
	{"cam1_vaf", REG_LDO, 1800000, 1800000, 150000, 0},
#endif
};
#endif

#ifdef CONFIG_IMX119
static struct camera_vreg_t apq_8064_front_cam_vreg[] = {
	{"cam2_vio", REG_VS, 0, 0, 0, 0},
	{"cam2_vana", REG_LDO, 2800000, 2850000, 85600, 0},
	{"cam2_vdig", REG_LDO, 1200000, 1200000, 105000, 0},
};
#endif
/*                                                                                     */
#if defined (CONFIG_IMX132)
static struct camera_vreg_t apq_8064_front_cam_vreg[] = {
	{"cam2_vio", REG_LDO, 1800000, 1800000, 85600, 0},
	{"cam2_vdig", REG_LDO, 1200000, 1200000, 105000, 0},
//                                                 
};
#endif
/*                                                                                     */

#if defined(CONFIG_IMX111) || defined(CONFIG_IMX091)
static struct gpio apq8064_common_cam_gpio[] = {
	{12, GPIOF_DIR_IN, "CAMIF_I2C_DATA"},
	{13, GPIOF_DIR_IN, "CAMIF_I2C_CLK"},
};

static struct gpio apq8064_back_cam_gpio[] = {
	{GPIO_CAM_MCLK0, GPIOF_DIR_IN, "CAMIF_MCLK"},
	{GPIO_CAM1_RST_N, GPIOF_DIR_OUT, "CAM_RESET"},
};

static struct msm_gpio_set_tbl apq8064_back_cam_gpio_set_tbl[] = {
	{GPIO_CAM1_RST_N, GPIOF_OUT_INIT_LOW, 10000},
	{GPIO_CAM1_RST_N, GPIOF_OUT_INIT_HIGH, 10000},
};

static struct msm_camera_gpio_conf apq8064_back_cam_gpio_conf = {
	.cam_gpiomux_conf_tbl = apq8064_cam_2d_configs,
	.cam_gpiomux_conf_tbl_size = ARRAY_SIZE(apq8064_cam_2d_configs),
	.cam_gpio_common_tbl = apq8064_common_cam_gpio,
	.cam_gpio_common_tbl_size = ARRAY_SIZE(apq8064_common_cam_gpio),
	.cam_gpio_req_tbl = apq8064_back_cam_gpio,
	.cam_gpio_req_tbl_size = ARRAY_SIZE(apq8064_back_cam_gpio),
	.cam_gpio_set_tbl = apq8064_back_cam_gpio_set_tbl,
	.cam_gpio_set_tbl_size = ARRAY_SIZE(apq8064_back_cam_gpio_set_tbl),
};
#endif

#ifdef CONFIG_IMX119
static struct gpio apq8064_front_cam_gpio[] = {
/*                                                       */
#if 1
	{GPIO_CAM_MCLK2, GPIOF_DIR_IN, "CAMIF_MCLK"},
#else
	{GPIO_CAM_MCLK1, GPIOF_DIR_IN, "CAMIF_MCLK"},
#endif
	{GPIO_CAM2_RST_N, GPIOF_DIR_OUT, "CAM_RESET"},
};

static struct msm_gpio_set_tbl apq8064_front_cam_gpio_set_tbl[] = {
	{GPIO_CAM2_RST_N, GPIOF_OUT_INIT_LOW, 10000},
	{GPIO_CAM2_RST_N, GPIOF_OUT_INIT_HIGH, 10000},
};

static struct msm_camera_gpio_conf apq8064_front_cam_gpio_conf = {
	.cam_gpiomux_conf_tbl = apq8064_cam_2d_configs,
	.cam_gpiomux_conf_tbl_size = ARRAY_SIZE(apq8064_cam_2d_configs),
	.cam_gpio_common_tbl = apq8064_common_cam_gpio,
	.cam_gpio_common_tbl_size = ARRAY_SIZE(apq8064_common_cam_gpio),
	.cam_gpio_req_tbl = apq8064_front_cam_gpio,
	.cam_gpio_req_tbl_size = ARRAY_SIZE(apq8064_front_cam_gpio),
	.cam_gpio_set_tbl = apq8064_front_cam_gpio_set_tbl,
	.cam_gpio_set_tbl_size = ARRAY_SIZE(apq8064_front_cam_gpio_set_tbl),
};
#endif

/*                                                                                     */
#if defined (CONFIG_IMX132)
static struct gpio apq8064_front_cam_gpio[] = {
	{GPIO_CAM_MCLK2, GPIOF_DIR_IN, "CAMIF_MCLK"},
	{GPIO_CAM2_RST_N, GPIOF_DIR_OUT, "CAM_RESET"},
};
/*                                                                          */
static struct gpio apq8064_front_cam_gpio_revE[] = {
	{GPIO_CAM_MCLK2, GPIOF_DIR_IN, "CAMIF_MCLK"},
};
/*                                                                        */

static struct msm_gpio_set_tbl apq8064_front_cam_gpio_set_tbl[] = {
	{GPIO_CAM2_RST_N, GPIOF_OUT_INIT_LOW, 10000},
	{GPIO_CAM2_RST_N, GPIOF_OUT_INIT_HIGH, 10000},
};

static struct msm_camera_gpio_conf apq8064_front_cam_gpio_conf = {
	.cam_gpiomux_conf_tbl = apq8064_cam_2d_configs,
	.cam_gpiomux_conf_tbl_size = ARRAY_SIZE(apq8064_cam_2d_configs),
	.cam_gpio_common_tbl = apq8064_common_cam_gpio,
	.cam_gpio_common_tbl_size = ARRAY_SIZE(apq8064_common_cam_gpio),
	.cam_gpio_req_tbl = apq8064_front_cam_gpio,
	.cam_gpio_req_tbl_size = ARRAY_SIZE(apq8064_front_cam_gpio),
	.cam_gpio_set_tbl = apq8064_front_cam_gpio_set_tbl,
	.cam_gpio_set_tbl_size = ARRAY_SIZE(apq8064_front_cam_gpio_set_tbl),
};
/*                                                                          */
static struct msm_camera_gpio_conf apq8064_front_cam_gpio_conf_revE = {
	.cam_gpiomux_conf_tbl = apq8064_cam_2d_configs,
	.cam_gpiomux_conf_tbl_size = ARRAY_SIZE(apq8064_cam_2d_configs),
	.cam_gpio_common_tbl = apq8064_common_cam_gpio,
	.cam_gpio_common_tbl_size = ARRAY_SIZE(apq8064_common_cam_gpio),
	.cam_gpio_req_tbl = apq8064_front_cam_gpio_revE,
	.cam_gpio_req_tbl_size = ARRAY_SIZE(apq8064_front_cam_gpio_revE),
};
/*                                                                        */

#endif
/*                                                                                     */

#if defined (CONFIG_IMX091) || defined (CONFIG_IMX111)
static struct msm_camera_i2c_conf apq8064_back_cam_i2c_conf = {
	.use_i2c_mux = 1,
	.mux_dev = &msm8960_device_i2c_mux_gsbi4,
	.i2c_mux_mode = MODE_L,
};
#endif
#ifdef CONFIG_IMX111_ACT
static struct i2c_board_info msm_act_main_cam_i2c_info = {
	I2C_BOARD_INFO("msm_actuator", I2C_SLAVE_ADDR_IMX111_ACT),
};

static struct msm_actuator_info msm_act_main_cam_0_info = {
	.board_info     = &msm_act_main_cam_i2c_info,
	.cam_name   = MSM_ACTUATOR_MAIN_CAM_1,
	.bus_id         = APQ_8064_GSBI4_QUP_I2C_BUS_ID,
	.vcm_pwd        = 0,
	.vcm_enable     = 0,
};
#endif

#ifdef CONFIG_IMX111
static struct msm_camera_sensor_flash_data flash_imx111 = {
	.flash_type	= MSM_CAMERA_FLASH_LED,
};

static struct msm_camera_csi_lane_params imx111_csi_lane_params = {
	.csi_lane_assign = 0xE4,
	.csi_lane_mask = 0xF,
};

static struct msm_camera_sensor_platform_info sensor_board_info_imx111 = {
	.mount_angle	= 90,
	.cam_vreg = apq_8064_back_cam_vreg,
	.num_vreg = ARRAY_SIZE(apq_8064_back_cam_vreg),
	.gpio_conf = &apq8064_back_cam_gpio_conf,
	.i2c_conf = &apq8064_back_cam_i2c_conf,
	.csi_lane_params = &imx111_csi_lane_params,
};

static struct msm_camera_sensor_info msm_camera_sensor_imx111_data = {
	.sensor_name	= "imx111",
	.pdata	= &msm_camera_csi_device_data[0],
	.flash_data	= &flash_imx111,
	.sensor_platform_info = &sensor_board_info_imx111,
	.csi_if	= 1,
	.camera_type = BACK_CAMERA_2D,
	.sensor_type = BAYER_SENSOR,
#ifdef CONFIG_IMX111_ACT
		.actuator_info = &msm_act_main_cam_0_info,

#endif
};
#endif

#ifdef CONFIG_IMX091_ACT
static struct i2c_board_info msm_act_main_cam_i2c_info = {
	I2C_BOARD_INFO("msm_actuator", I2C_SLAVE_ADDR_IMX091_ACT), /*      */
};

static struct msm_actuator_info msm_act_main_cam_0_info = {
	.board_info     = &msm_act_main_cam_i2c_info,
	.cam_name   = MSM_ACTUATOR_MAIN_CAM_1,
	.bus_id         = APQ_8064_GSBI4_QUP_I2C_BUS_ID,
	.vcm_pwd        = 0,
	.vcm_enable     = 0,
};
#endif
#ifdef CONFIG_IMX091
static struct msm_camera_sensor_flash_data flash_imx091 = {
	.flash_type	= MSM_CAMERA_FLASH_LED,
};

static struct msm_camera_csi_lane_params imx091_csi_lane_params = {
	.csi_lane_assign = 0xE4,
	.csi_lane_mask = 0xF,
};

static struct msm_camera_sensor_platform_info sensor_board_info_imx091 = {
	.mount_angle	= 90,
	.cam_vreg = apq_8064_back_cam_vreg,
	.num_vreg = ARRAY_SIZE(apq_8064_back_cam_vreg),
	.gpio_conf = &apq8064_back_cam_gpio_conf,
	.i2c_conf = &apq8064_back_cam_i2c_conf,
	.csi_lane_params = &imx091_csi_lane_params,
};

static struct i2c_board_info imx091_eeprom_i2c_info = {
	I2C_BOARD_INFO("imx091_eeprom", 0x21),
};

static struct msm_eeprom_info imx091_eeprom_info = {
	.board_info     = &imx091_eeprom_i2c_info,
	.bus_id         = APQ_8064_GSBI4_QUP_I2C_BUS_ID,
};

static struct msm_camera_sensor_info msm_camera_sensor_imx091_data = {
	.sensor_name	= "imx091",
	.pdata	= &msm_camera_csi_device_data[0],
	.flash_data	= &flash_imx091,
	.sensor_platform_info = &sensor_board_info_imx091,
	.csi_if	= 1,
	.camera_type = BACK_CAMERA_2D,
	.sensor_type = BAYER_SENSOR,
#ifdef CONFIG_IMX091_ACT
	.actuator_info = &msm_act_main_cam_0_info,
#endif
	.eeprom_info = &imx091_eeprom_info,
};
#endif

#ifdef CONFIG_IMX119
static struct msm_camera_i2c_conf apq8064_front_cam_i2c_conf = {
	.use_i2c_mux = 1,
	.mux_dev = &msm8960_device_i2c_mux_gsbi4,
	.i2c_mux_mode = MODE_L,
};
#endif

/*                                                                                     */
#if defined (CONFIG_IMX132)
static struct msm_camera_i2c_conf apq8064_front_cam_i2c_conf = {
	.use_i2c_mux = 1,
	.mux_dev = &msm8960_device_i2c_mux_gsbi4,
	.i2c_mux_mode = MODE_L,
};
#endif
/*                                                                                     */

#ifdef CONFIG_IMX135
static struct msm_camera_sensor_flash_data flash_imx135 = {
	.flash_type = MSM_CAMERA_FLASH_NONE,
};

static struct msm_camera_csi_lane_params imx135_csi_lane_params = {
	.csi_lane_assign = 0xE4,
	.csi_lane_mask = 0xF,
};

static struct msm_camera_sensor_platform_info sensor_board_info_imx135 = {
	.mount_angle    = 90,
	.cam_vreg = apq_8064_cam_vreg,
	.num_vreg = ARRAY_SIZE(apq_8064_cam_vreg),
	.gpio_conf = &apq8064_back_cam_gpio_conf,
	.i2c_conf = &apq8064_back_cam_i2c_conf,
	.csi_lane_params = &imx135_csi_lane_params,
};

static struct msm_camera_sensor_info msm_camera_sensor_imx135_data = {
	.sensor_name    = "imx135",
	.pdata  = &msm_camera_csi_device_data[0],
	.flash_data = &flash_imx135,
	.sensor_platform_info = &sensor_board_info_imx135,
	.csi_if = 1,
	.camera_type = BACK_CAMERA_2D,
	.sensor_type = BAYER_SENSOR,
	.actuator_info = &msm_act_main_cam_1_info,
};
#endif

#ifdef CONFIG_IMX119
static struct msm_camera_sensor_flash_data flash_imx119 = {
	.flash_type	= MSM_CAMERA_FLASH_NONE,
};

static struct msm_camera_csi_lane_params imx119_csi_lane_params = {
	.csi_lane_assign = 0xE4,
	.csi_lane_mask = 0x1,
};

static struct msm_camera_sensor_platform_info sensor_board_info_imx119 = {
	.mount_angle	= 270,
	.cam_vreg = apq_8064_front_cam_vreg,
	.num_vreg = ARRAY_SIZE(apq_8064_front_cam_vreg),
	.gpio_conf = &apq8064_front_cam_gpio_conf,
	.i2c_conf = &apq8064_front_cam_i2c_conf,
	.csi_lane_params = &imx119_csi_lane_params,
};

static struct msm_camera_sensor_info msm_camera_sensor_imx119_data = {
	.sensor_name	= "imx119",
	.pdata	= &msm_camera_csi_device_data[1],
	.flash_data	= &flash_imx119,
	.sensor_platform_info = &sensor_board_info_imx119,
	.csi_if	= 1,
	.camera_type = FRONT_CAMERA_2D,
	.sensor_type = BAYER_SENSOR,
};
#endif

/*                                                                                     */
#if defined (CONFIG_IMX132)
static struct msm_camera_sensor_flash_data flash_imx132 = {
	.flash_type	= MSM_CAMERA_FLASH_NONE,
};

static struct msm_camera_csi_lane_params imx132_csi_lane_params = {
	.csi_lane_assign = 0xE4,
	.csi_lane_mask = 0x1,
};

static struct msm_camera_sensor_platform_info sensor_board_info_imx132 = {
	.mount_angle	= 270,
	.cam_vreg = apq_8064_front_cam_vreg,
	.num_vreg = ARRAY_SIZE(apq_8064_front_cam_vreg),
	.gpio_conf = &apq8064_front_cam_gpio_conf,
	.i2c_conf = &apq8064_front_cam_i2c_conf,
	.csi_lane_params = &imx132_csi_lane_params,
};
/*                                                                          */
static struct msm_camera_sensor_platform_info sensor_board_info_imx132_revE = {
	.mount_angle	= 270,
	.cam_vreg = apq_8064_front_cam_vreg,
	.num_vreg = ARRAY_SIZE(apq_8064_front_cam_vreg),
	.gpio_conf = &apq8064_front_cam_gpio_conf_revE,
	.i2c_conf = &apq8064_front_cam_i2c_conf,
	.csi_lane_params = &imx132_csi_lane_params,
};
/*                                                                        */

static struct msm_camera_sensor_info msm_camera_sensor_imx132_data = {
	.sensor_name	= "imx132",
	.pdata	= &msm_camera_csi_device_data[1],
	.flash_data	= &flash_imx132,
	.sensor_platform_info = &sensor_board_info_imx132,
	.csi_if	= 1,
	.camera_type = FRONT_CAMERA_2D,
	.sensor_type = BAYER_SENSOR,
};
/*                                                                          */
static struct msm_camera_sensor_info msm_camera_sensor_imx132_data_revE = {
	.sensor_name	= "imx132",
	.pdata	= &msm_camera_csi_device_data[1],
	.flash_data	= &flash_imx132,
	.sensor_platform_info = &sensor_board_info_imx132_revE,
	.csi_if	= 1,
	.camera_type = FRONT_CAMERA_2D,
	.sensor_type = BAYER_SENSOR,
};
/*                                                                        */

#endif
/*                                                                                     */

/*                               */
struct led_flash_platform_data {
	unsigned gpio_en;
	unsigned scl_gpio;
	unsigned sda_gpio;
};

/*                                                                                                                      */
#if !defined(CONFIG_MACH_APQ8064_GVAR_CMCC)
static struct led_flash_platform_data lm3559_flash_pdata[] = {
	{
		.scl_gpio = GPIO_CAM_FLASH_I2C_SCL,
		.sda_gpio = GPIO_CAM_FLASH_I2C_SDA,
		.gpio_en = GPIO_CAM_FLASH_EN,
	}
};
#endif
/*                                                                                                                      */

static struct platform_device msm_camera_server = {
	.name = "msm_cam_server",
	.id = 0,
};

void __init apq8064_init_cam(void)
{
/*                                                                          */
	if(lge_get_board_revno() >= HW_REV_A ){
		msm_gpiomux_install(apq8064_cam_common_configs_revE,
				ARRAY_SIZE(apq8064_cam_common_configs_revE));
	}else
/*                                                                        */
	{
	/*                                                                    
                       */
	if (socinfo_get_platform_subtype() != PLATFORM_SUBTYPE_SGLTE2) {
		msm_gpiomux_install(apq8064_cam_common_configs,
				ARRAY_SIZE(apq8064_cam_common_configs));
	}
	}

	platform_device_register(&msm_camera_server);
	if (socinfo_get_platform_subtype() != PLATFORM_SUBTYPE_SGLTE2)
		platform_device_register(&msm8960_device_i2c_mux_gsbi4);
	platform_device_register(&msm8960_device_csiphy0);
	platform_device_register(&msm8960_device_csiphy1);
	platform_device_register(&msm8960_device_csid0);
	platform_device_register(&msm8960_device_csid1);
	platform_device_register(&msm8960_device_ispif);
	platform_device_register(&msm8960_device_vfe);
	platform_device_register(&msm8960_device_vpe);
}

#ifdef CONFIG_I2C
static struct i2c_board_info apq8064_camera_i2c_boardinfo[] = {
#ifdef CONFIG_IMX111
	{
		I2C_BOARD_INFO("imx111", I2C_SLAVE_ADDR_IMX111), /*      */
		.platform_data = &msm_camera_sensor_imx111_data,
	},
#endif
#ifdef CONFIG_IMX091
	{
		I2C_BOARD_INFO("imx091", I2C_SLAVE_ADDR_IMX091), /*      */
		.platform_data = &msm_camera_sensor_imx091_data,
	},
#endif
#ifdef CONFIG_IMX119
	{
		I2C_BOARD_INFO("imx119", I2C_SLAVE_ADDR_IMX119), /*      */
		.platform_data = &msm_camera_sensor_imx119_data,
	},
#endif
/*                                                                                     */
#ifdef CONFIG_CE1702
	{
	I2C_BOARD_INFO("ce1702", I2C_SLAVE_ADDR_CE1702), /*      */
	.platform_data = &msm_camera_sensor_ce1702_data,
	},
#endif
#ifdef CONFIG_IMX132
	{
	I2C_BOARD_INFO("imx132", I2C_SLAVE_ADDR_IMX132), /*      */
	.platform_data = &msm_camera_sensor_imx132_data,
	},
#endif
/*                                                                                     */
};

#ifdef CONFIG_IMX135
	{
	I2C_BOARD_INFO("imx135", 0x10),
	.platform_data = &msm_camera_sensor_imx135_data,
	},
#endif

/*                                                                          */
static struct i2c_board_info apq8064_camera_i2c_boardinfo_revE[] = {
#ifdef CONFIG_CE1702
	{
	I2C_BOARD_INFO("ce1702", I2C_SLAVE_ADDR_CE1702), /*      */
	.platform_data = &msm_camera_sensor_ce1702_data_revE,
	},
#endif
#ifdef CONFIG_IMX132
	{
	I2C_BOARD_INFO("imx132", I2C_SLAVE_ADDR_IMX132), /*      */
	.platform_data = &msm_camera_sensor_imx132_data_revE,
	},
#endif
};
/*                                                                        */

/*                               */
/*                                                                                                                      */
#if !defined(CONFIG_MACH_APQ8064_GVAR_CMCC)
static struct i2c_board_info apq8064_lge_camera_i2c_boardinfo[] = {
	{
		I2C_BOARD_INFO("lm3559", I2C_SLAVE_ADDR_FLASH),
		.platform_data = &lm3559_flash_pdata,
	},
};
#endif
/*                                                                                                                      */

struct msm_camera_board_info apq8064_camera_board_info = {
	.board_info = apq8064_camera_i2c_boardinfo,
	.num_i2c_board_info = ARRAY_SIZE(apq8064_camera_i2c_boardinfo),
};
/*                                                                          */
struct msm_camera_board_info apq8064_camera_board_info_revE = {
	.board_info = apq8064_camera_i2c_boardinfo_revE,
	.num_i2c_board_info = ARRAY_SIZE(apq8064_camera_i2c_boardinfo_revE),
};
/*                                                                        */

/*                               */
/*                                                                                                                      */
#if !defined(CONFIG_MACH_APQ8064_GVAR_CMCC)
struct msm_camera_board_info apq8064_lge_camera_board_info = {
	.board_info = apq8064_lge_camera_i2c_boardinfo,
	.num_i2c_board_info = ARRAY_SIZE(apq8064_lge_camera_i2c_boardinfo),
};
#endif
/*                                                                                                                     */
#endif
#endif
