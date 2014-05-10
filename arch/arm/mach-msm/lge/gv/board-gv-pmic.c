/* Copyright (c) 2011-2012, Code Aurora Forum. All rights reserved.
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

#include <linux/init.h>
#include <linux/ioport.h>
#include <linux/gpio.h>
#include <linux/platform_device.h>
#include <linux/bootmem.h>
#include <linux/mfd/pm8xxx/pm8921.h>
#include <linux/leds.h>
#include <linux/leds-pm8xxx.h>
#include <linux/mfd/pm8xxx/pm8xxx-adc.h>
#include <asm/mach-types.h>
#include <asm/mach/mmc.h>
#include <mach/msm_bus_board.h>
#include <mach/board.h>
#include <mach/gpio.h>
#include <mach/gpiomux.h>
#include <mach/restart.h>
#include <mach/socinfo.h>
#include <mach/board_lge.h>
#include <linux/i2c.h>
#include "devices.h"
#include "board-gv.h"
#ifdef CONFIG_BATTERY_MAX17047
#include <linux/max17047_battery.h>
#endif

struct pm8xxx_gpio_init {
	unsigned			gpio;
	struct pm_gpio			config;
};

struct pm8xxx_mpp_init {
	unsigned			mpp;
	struct pm8xxx_mpp_config_data	config;
};

#define PM8921_GPIO_INIT(_gpio, _dir, _buf, _val, _pull, _vin, _out_strength, \
			_func, _inv, _disable) \
{ \
	.gpio	= PM8921_GPIO_PM_TO_SYS(_gpio), \
	.config	= { \
		.direction	= _dir, \
		.output_buffer	= _buf, \
		.output_value	= _val, \
		.pull		= _pull, \
		.vin_sel	= _vin, \
		.out_strength	= _out_strength, \
		.function	= _func, \
		.inv_int_pol	= _inv, \
		.disable_pin	= _disable, \
	} \
}

#define PM8921_MPP_INIT(_mpp, _type, _level, _control) \
{ \
	.mpp	= PM8921_MPP_PM_TO_SYS(_mpp), \
	.config	= { \
		.type		= PM8XXX_MPP_TYPE_##_type, \
		.level		= _level, \
		.control	= PM8XXX_MPP_##_control, \
	} \
}

#define PM8821_MPP_INIT(_mpp, _type, _level, _control) \
{ \
	.mpp	= PM8821_MPP_PM_TO_SYS(_mpp), \
	.config	= { \
		.type		= PM8XXX_MPP_TYPE_##_type, \
		.level		= _level, \
		.control	= PM8XXX_MPP_##_control, \
	} \
}

#define PM8921_GPIO_DISABLE(_gpio) \
	PM8921_GPIO_INIT(_gpio, PM_GPIO_DIR_IN, 0, 0, 0, PM_GPIO_VIN_S4, \
			 0, 0, 0, 1)

#define PM8921_GPIO_OUTPUT(_gpio, _val, _strength) \
	PM8921_GPIO_INIT(_gpio, PM_GPIO_DIR_OUT, PM_GPIO_OUT_BUF_CMOS, _val, \
			PM_GPIO_PULL_NO, PM_GPIO_VIN_S4, \
			PM_GPIO_STRENGTH_##_strength, \
			PM_GPIO_FUNC_NORMAL, 0, 0)

#define PM8921_GPIO_OUTPUT_BUFCONF(_gpio, _val, _strength, _bufconf) \
	PM8921_GPIO_INIT(_gpio, PM_GPIO_DIR_OUT,\
			PM_GPIO_OUT_BUF_##_bufconf, _val, \
			PM_GPIO_PULL_NO, PM_GPIO_VIN_S4, \
			PM_GPIO_STRENGTH_##_strength, \
			PM_GPIO_FUNC_NORMAL, 0, 0)

#define PM8921_GPIO_INPUT(_gpio, _pull) \
	PM8921_GPIO_INIT(_gpio, PM_GPIO_DIR_IN, PM_GPIO_OUT_BUF_CMOS, 0, \
			_pull, PM_GPIO_VIN_S4, \
			PM_GPIO_STRENGTH_NO, \
			PM_GPIO_FUNC_NORMAL, 0, 0)

#define PM8921_GPIO_OUTPUT_FUNC(_gpio, _val, _func) \
	PM8921_GPIO_INIT(_gpio, PM_GPIO_DIR_OUT, PM_GPIO_OUT_BUF_CMOS, _val, \
			PM_GPIO_PULL_NO, PM_GPIO_VIN_S4, \
			PM_GPIO_STRENGTH_HIGH, \
			_func, 0, 0)

#define PM8921_GPIO_OUTPUT_VIN(_gpio, _val, _vin) \
	PM8921_GPIO_INIT(_gpio, PM_GPIO_DIR_OUT, PM_GPIO_OUT_BUF_CMOS, _val, \
			PM_GPIO_PULL_NO, _vin, \
			PM_GPIO_STRENGTH_HIGH, \
			PM_GPIO_FUNC_NORMAL, 0, 0)

/*                                    */
static struct pm8xxx_gpio_init pm8921_gpios[] __initdata = {
//                       
//                                      
//                               
#if 0	//    
#if defined(CONFIG_LGE_BROADCAST_ONESEG) && defined(CONFIG_LGE_BROADCAST_ONESEG_FC8150) //       

#if 1	//                               
	PM8921_GPIO_OUTPUT(11, 0, HIGH),
	PM8921_GPIO_OUTPUT(12, 1, HIGH),
#else 	/*                            */
	PM8921_GPIO_OUTPUT(11, 1, HIGH),
	PM8921_GPIO_OUTPUT(12, 0, HIGH),
#endif		//                               
#endif //                                                                                   
#endif	//    


#if defined(CONFIG_LGE_BROADCAST_ONESEG) && defined(CONFIG_MACH_APQ8064_GVDCM) /*             */
	PM8921_GPIO_OUTPUT(11, 0, HIGH), /*                                         */
#endif	/*                             */
//                       
	PM8921_GPIO_OUTPUT(14, 1, HIGH),	/*                   */
	PM8921_GPIO_OUTPUT(13, 0, HIGH), /*          */
	PM8921_GPIO_OUTPUT_BUFCONF(25, 0, LOW, CMOS), /*              */
//                                                                 
#ifdef CONFIG_LGE_BLUESLEEP
	PM8921_GPIO_OUTPUT(26, 0, HIGH),
#else
//         
	PM8921_GPIO_OUTPUT_FUNC(26, 0, PM_GPIO_FUNC_2), /*                   */


#endif
//                                                                 
	PM8921_GPIO_OUTPUT_VIN(30, 1, PM_GPIO_VIN_VPH), /*                  */
	PM8921_GPIO_OUTPUT_BUFCONF(36, 1, LOW, OPEN_DRAIN),
#ifdef CONFIG_BCMDHD
	PM8921_GPIO_OUTPUT_FUNC(43, 0, PM_GPIO_FUNC_1),
#else
	PM8921_GPIO_OUTPUT_FUNC(44, 0, PM_GPIO_FUNC_2),
#endif
	PM8921_GPIO_OUTPUT(33, 0, HIGH),
	PM8921_GPIO_OUTPUT(20, 0, HIGH),
	PM8921_GPIO_OUTPUT(35, 0, HIGH), /*           */
//                                           
	PM8921_GPIO_INPUT(38, PM_GPIO_PULL_UP_30),
	/*                   */
	PM8921_GPIO_OUTPUT(34, 1, HIGH),
	PM8921_GPIO_OUTPUT(13, 0, HIGH),               /*                 */
	PM8921_GPIO_INPUT(12, PM_GPIO_PULL_UP_30),     /*             */
#if defined(CONFIG_LGE_IRDA)
	PM8921_GPIO_OUTPUT(17,1,HIGH),
#endif
#if defined(CONFIG_LGE_IRRC)
	PM8921_GPIO_OUTPUT(18, 0, HIGH), /*                  */
	PM8921_GPIO_OUTPUT(21, 0, HIGH), /*                   */
#endif
#ifdef CONFIG_BCMDHD
	//              
	PM8921_GPIO_OUTPUT(29, 0, HIGH),
#endif
#ifdef CONFIG_FB_MSM_MIPI_LGIT_VIDEO_FHD_INVERSE_PT
    PM8921_GPIO_OUTPUT(42, 1, HIGH),
    PM8921_GPIO_OUTPUT(22, 1, HIGH),
    PM8921_GPIO_OUTPUT(23, 1, HIGH),    
#endif

};

#ifdef CONFIG_SWITCH_FSA8008
static struct pm8xxx_gpio_init pm8921_fsa8008_gpios[] __initdata = {
	PM8921_GPIO_OUTPUT(31, 0, HIGH), /*                           */
	PM8921_GPIO_INPUT(32, PM_GPIO_PULL_UP_1P5), /*                              */
};
#endif

#ifdef CONFIG_SWITCH_MAX1462X
static struct pm8xxx_gpio_init pm8921_max1462x_gpios[] __initdata = {
	PM8921_GPIO_OUTPUT(31, 0, HIGH), /*                            */
	PM8921_GPIO_OUTPUT(32, 0, LOW), /*                            */
};
#endif

static struct pm8xxx_gpio_init pm8921_mtp_kp_gpios[] __initdata = {
	PM8921_GPIO_INPUT(3, PM_GPIO_PULL_UP_30),
	PM8921_GPIO_INPUT(4, PM_GPIO_PULL_UP_30),
};

static struct pm8xxx_gpio_init pm8921_cdp_kp_gpios[] __initdata = {
	PM8921_GPIO_INPUT(27, PM_GPIO_PULL_UP_30),
	PM8921_GPIO_INPUT(42, PM_GPIO_PULL_UP_30),
	PM8921_GPIO_INPUT(17, PM_GPIO_PULL_UP_1P5),	/*       */
};

static struct pm8xxx_gpio_init pm8921_mpq8064_hrd_gpios[] __initdata = {
	PM8921_GPIO_OUTPUT(37, 0, LOW),	/*          */
};

/*                                    */
static struct pm8xxx_gpio_init pm8917_gpios[] __initdata = {
        PM8921_GPIO_OUTPUT(14, 1, HIGH),        /*                   */
        PM8921_GPIO_OUTPUT(23, 0, HIGH),        /*                       */
        PM8921_GPIO_OUTPUT_BUFCONF(25, 0, LOW, CMOS), /*              */
        PM8921_GPIO_OUTPUT(26, 1, HIGH), /*               */
        PM8921_GPIO_OUTPUT_BUFCONF(36, 1, LOW, OPEN_DRAIN),
        PM8921_GPIO_OUTPUT_FUNC(38, 0, PM_GPIO_FUNC_2),
        PM8921_GPIO_OUTPUT(33, 0, HIGH),
        PM8921_GPIO_OUTPUT(20, 0, HIGH),
        PM8921_GPIO_INPUT(35, PM_GPIO_PULL_UP_30),
        PM8921_GPIO_INPUT(30, PM_GPIO_PULL_UP_30),
        /*                   */
        PM8921_GPIO_OUTPUT(34, 1, MED),
        PM8921_GPIO_OUTPUT(13, 0, HIGH),               /*                 */
        PM8921_GPIO_INPUT(12, PM_GPIO_PULL_UP_30),     /*             */
};

/*                                        */
static struct pm8xxx_gpio_init pm8917_cdp_kp_gpios[] __initdata = {
        PM8921_GPIO_INPUT(27, PM_GPIO_PULL_UP_30),
        PM8921_GPIO_INPUT(8, PM_GPIO_PULL_UP_30),
        PM8921_GPIO_INPUT(17, PM_GPIO_PULL_UP_1P5),     /*       */
};

static struct pm8xxx_gpio_init pm8921_mpq_gpios[] __initdata = {
	PM8921_GPIO_INIT(27, PM_GPIO_DIR_IN, PM_GPIO_OUT_BUF_CMOS, 0,
			PM_GPIO_PULL_NO, PM_GPIO_VIN_VPH, PM_GPIO_STRENGTH_NO,
			PM_GPIO_FUNC_NORMAL, 0, 0),
};

/*                                   */
static struct pm8xxx_mpp_init pm8xxx_mpps[] __initdata = {
	//                                                                      
	PM8921_MPP_INIT(8, D_OUTPUT, PM8921_MPP_DIG_LEVEL_S4, DOUT_CTRL_LOW),
#ifndef CONFIG_SWITCH_MAX1462X
	/*                                                                   */
	PM8921_MPP_INIT(9, D_INPUT, PM8921_MPP_DIG_LEVEL_S4, DIN_TO_INT),
	/*              */
	PM8921_MPP_INIT(1, D_OUTPUT, PM8921_MPP_DIG_LEVEL_VPH, DOUT_CTRL_HIGH),
#endif
};

#if defined(CONFIG_MACH_LGE)
static struct pm8xxx_gpio_init pm8921_disabled_gpios[] __initdata= {
	PM8921_GPIO_DISABLE(3),
	PM8921_GPIO_DISABLE(4),
	PM8921_GPIO_DISABLE(5),
	PM8921_GPIO_DISABLE(6),
	PM8921_GPIO_DISABLE(7),
	PM8921_GPIO_DISABLE(8),
	PM8921_GPIO_DISABLE(12),
	PM8921_GPIO_DISABLE(14),
	PM8921_GPIO_DISABLE(15),
	PM8921_GPIO_DISABLE(16),
	PM8921_GPIO_DISABLE(23),
	PM8921_GPIO_DISABLE(25),
	PM8921_GPIO_DISABLE(30),
	PM8921_GPIO_DISABLE(32),
	PM8921_GPIO_DISABLE(36),
	PM8921_GPIO_DISABLE(38),
	PM8921_GPIO_DISABLE(40),
	PM8921_GPIO_DISABLE(41),
	PM8921_GPIO_DISABLE(44),

};

static struct pm8xxx_mpp_init pm8xxx_disabled_mpps[] __initdata = {
		PM8921_MPP_INIT(2, SINK, PM8XXX_MPP_CS_OUT_5MA, CS_CTRL_DISABLE),
		PM8921_MPP_INIT(11, SINK, PM8XXX_MPP_CS_OUT_5MA, CS_CTRL_DISABLE),
};
#endif

#ifdef CONFIG_SWITCH_MAX1462X
static struct pm8xxx_mpp_init pm8xxx_max1462x_mpps[] __initdata = {
	/*                                        */
	PM8921_MPP_INIT(9, D_INPUT, PM8921_MPP_DIG_LEVEL_S4, DIN_TO_INT),
	PM8921_MPP_INIT(10, D_INPUT, PM8921_MPP_DIG_LEVEL_S4, DIN_TO_INT),
	/*                                                      */
	PM8921_MPP_INIT(1, D_INPUT, PM8921_MPP_DIG_LEVEL_S4, DIN_TO_INT),
};
#endif
/*                                                                    */
#if defined (CONFIG_IMX132) || defined(CONFIG_CE1702)
struct pm_gpio pm_gpio_cam_rst_ctrl = {
	.direction = PM_GPIO_DIR_OUT,
	.output_buffer = PM_GPIO_OUT_BUF_CMOS,
	.output_value = 0,
	.pull = PM_GPIO_PULL_NO,
	.vin_sel = PM_GPIO_VIN_S4,
	.out_strength = PM_GPIO_STRENGTH_HIGH,
	.function = PM_GPIO_FUNC_NORMAL,
	.inv_int_pol = 0,
	.disable_pin = 0
};
#endif
/*                                                                  */

void __init apq8064_configure_gpios(struct pm8xxx_gpio_init *data, int len)
{
	int i, rc;

	for (i = 0; i < len; i++) {
		rc = pm8xxx_gpio_config(data[i].gpio, &data[i].config);
		if (rc)
			pr_err("%s: pm8xxx_gpio_config(%u) failed: rc=%d\n",
				__func__, data[i].gpio, rc);
	}
}

void __init apq8064_pm8xxx_gpio_mpp_init(void)
{
	int i, rc;

	if (socinfo_get_pmic_model() != PMIC_MODEL_PM8917)
		apq8064_configure_gpios(pm8921_gpios, ARRAY_SIZE(pm8921_gpios));
	else
		apq8064_configure_gpios(pm8917_gpios, ARRAY_SIZE(pm8917_gpios));

#ifdef CONFIG_SWITCH_FSA8008
	if(lge_get_board_revno() == HW_REV_A){
		printk("[%s] CONFIG_SWITCH_FSA8008 pm8921 init\n", __func__);
		apq8064_configure_gpios(pm8921_fsa8008_gpios, ARRAY_SIZE(pm8921_fsa8008_gpios));
	}
#endif

#ifdef CONFIG_SWITCH_MAX1462X
	if(lge_get_board_revno() >= HW_REV_B){
		printk("[%s] CONFIG_SWITCH_MAX1462X pm8921 init\n", __func__);
		apq8064_configure_gpios(pm8921_max1462x_gpios, ARRAY_SIZE(pm8921_max1462x_gpios));
	}
#endif

	if (machine_is_apq8064_cdp() || machine_is_apq8064_liquid()) {
		if (socinfo_get_pmic_model() != PMIC_MODEL_PM8917)
			apq8064_configure_gpios(pm8921_cdp_kp_gpios,
					ARRAY_SIZE(pm8921_cdp_kp_gpios));
		else
			apq8064_configure_gpios(pm8917_cdp_kp_gpios,
					ARRAY_SIZE(pm8917_cdp_kp_gpios));
	}

	if (machine_is_apq8064_mtp())
		apq8064_configure_gpios(pm8921_mtp_kp_gpios,
					ARRAY_SIZE(pm8921_mtp_kp_gpios));

	if (machine_is_mpq8064_cdp() || machine_is_mpq8064_hrd()
	    || machine_is_mpq8064_dtv())
		apq8064_configure_gpios(pm8921_mpq_gpios,
					ARRAY_SIZE(pm8921_mpq_gpios));

	if (machine_is_mpq8064_hrd())
		apq8064_configure_gpios(pm8921_mpq8064_hrd_gpios,
					ARRAY_SIZE(pm8921_mpq8064_hrd_gpios));

	for (i = 0; i < ARRAY_SIZE(pm8xxx_mpps); i++) {
		rc = pm8xxx_mpp_config(pm8xxx_mpps[i].mpp,
					&pm8xxx_mpps[i].config);
		if (rc) {
			pr_err("%s: pm8xxx_mpp_config: rc=%d\n", __func__, rc);
			break;
		}
	}

#ifdef CONFIG_SWITCH_MAX1462X
	if(lge_get_board_revno() >= HW_REV_C)
	{
		for (i = 0; i < ARRAY_SIZE(pm8xxx_max1462x_mpps); i++) {
			rc = pm8xxx_mpp_config(pm8xxx_max1462x_mpps[i].mpp,
						&pm8xxx_max1462x_mpps[i].config);
			if (rc) {
				pr_err("%s: pm8xxx_mpp_max1462x_config: rc=%d\n", __func__, rc);
				break;
			}
		}
	}
#endif

#if defined(CONFIG_MACH_LGE)
	if(lge_get_board_revno() >= HW_REV_D){
		apq8064_configure_gpios(pm8921_disabled_gpios, ARRAY_SIZE(pm8921_disabled_gpios));

		for (i = 0; i < ARRAY_SIZE(pm8xxx_disabled_mpps); i++) {
			rc = pm8xxx_mpp_config(pm8xxx_disabled_mpps[i].mpp,
						&pm8xxx_disabled_mpps[i].config);
			if (rc) {
				pr_err("%s: pm8xxx_mpp_config: rc=%d\n", __func__, rc);
				break;
			}
		}	
	}
#endif

}

static struct pm8xxx_pwrkey_platform_data apq8064_pm8921_pwrkey_pdata = {
	.pull_up		= 1,
	.kpd_trigger_delay_us	= 15625,
	.wakeup			= 1,
};

static struct pm8xxx_misc_platform_data apq8064_pm8921_misc_pdata = {
	.priority		= 0,
};

#define PM8921_LC_LED_MAX_CURRENT	4	/*         */
#define PM8921_KEY_LED_MAX_CURRENT	4	/*         */
#define PM8921_LC_LED_LOW_CURRENT	1	/*         */
#define PM8XXX_LED_PWM_PERIOD		1000
#define PM8XXX_LED_PWM_DUTY_MS		20
/* 
                                                              
                            
 */
#define PM8XXX_PWM_CHANNEL_NONE		-1

static struct led_info pm8921_led_info_rev_a[] = {
#if defined(CONFIG_LGE_PM)
	[0] = {
		.name			= "led:red",
			},
#else
			[0] = {
				.name			= "led:red",
		.default_trigger	= "ac-online",
	},
#endif
#if defined(CONFIG_MACH_LGE)	/*                               */
			[1] = {
				.name			= "button-backlight",
			},
 #if !defined(CONFIG_MACH_APQ8064_J1A)
			[2] = {
				.name			= "led:green",
			},
#endif
#endif
};

static struct led_platform_data pm8921_led_core_pdata_rev_a = {
	.num_leds = ARRAY_SIZE(pm8921_led_info_rev_a),
	.leds = pm8921_led_info_rev_a,
};

static struct led_info pm8921_led_info_rev_b[] = {
#if defined(CONFIG_LGE_PM)
	[0] = {
		.name			= "led:red",
			},
#else
			[0] = {
				.name			= "led:red",
		.default_trigger	= "ac-online",
	},
#endif
#if defined(CONFIG_MACH_LGE)	/*                               */
			[1] = {
				.name			= "button-backlight",
			},
 #if !defined(CONFIG_MACH_APQ8064_J1A)
			[2] = {
				.name			= "led:green",
			},
#endif
#endif
};

static struct led_platform_data pm8921_led_core_pdata_rev_b = {
	.num_leds = ARRAY_SIZE(pm8921_led_info_rev_b),
	.leds = pm8921_led_info_rev_b,
};

static struct led_info pm8921_led_info[] = {
			[0] = {
				.name			= "button-backlight",
			},
};

static struct led_platform_data pm8921_led_core_pdata = {
	.num_leds = ARRAY_SIZE(pm8921_led_info),
	.leds = pm8921_led_info,
};


#if defined(CONFIG_LGE_PM)
#else
static int pm8921_led0_pwm_duty_pcts[56] = {
	1, 4, 8, 12, 16, 20, 24, 28, 32, 36,
	40, 44, 46, 52, 56, 60, 64, 68, 72, 76,
	80, 84, 88, 92, 96, 100, 100, 100, 98, 95,
	92, 88, 84, 82, 78, 74, 70, 66, 62, 58,
	58, 54, 50, 48, 42, 38, 34, 30, 26, 22,
	14, 10, 6, 4, 1
};
#endif

/*
                                                               
                                                               
                                   
 */
#if defined(CONFIG_LGE_PM)
#else
static struct pm8xxx_pwm_duty_cycles pm8921_led0_pwm_duty_cycles = {
	.duty_pcts = (int *)&pm8921_led0_pwm_duty_pcts,
	.num_duty_pcts = ARRAY_SIZE(pm8921_led0_pwm_duty_pcts),
	.duty_ms = PM8XXX_LED_PWM_DUTY_MS,
	.start_idx = 1,
};
#endif

static struct pm8xxx_led_config pm8921_led_configs_rev_a[] = {
#if defined(CONFIG_LGE_PM)
	[0] = {
		.id = PM8XXX_ID_LED_1,
				.mode = PM8XXX_LED_MODE_MANUAL,
				.max_current = PM8921_LC_LED_MAX_CURRENT,
			},
#else
			[0] = {
				.id = PM8XXX_ID_LED_1,
		.mode = PM8XXX_LED_MODE_PWM2,
		.max_current = PM8921_LC_LED_MAX_CURRENT,
		.pwm_channel = 5,
		.pwm_period_us = PM8XXX_LED_PWM_PERIOD,
		.pwm_duty_cycles = &pm8921_led0_pwm_duty_cycles,
	},
#endif

#if defined(CONFIG_MACH_LGE)	/*                               */
			[1] = {
				.id = PM8XXX_ID_LED_KB_LIGHT,
				.mode = PM8XXX_LED_MODE_MANUAL,
				.max_current = PM8921_KEY_LED_MAX_CURRENT,
			},
 #if !defined(CONFIG_MACH_APQ8064_J1A)
			[2] = {
				.id = PM8XXX_ID_LED_2,
				.mode = PM8XXX_LED_MODE_MANUAL,
				.max_current = PM8921_LC_LED_MAX_CURRENT,
			},
#endif
#endif

};

static struct pm8xxx_led_platform_data apq8064_pm8921_leds_pdata_rev_a = {
		.led_core = &pm8921_led_core_pdata_rev_a,
		.configs = pm8921_led_configs_rev_a,
		.num_configs = ARRAY_SIZE(pm8921_led_configs_rev_a),
};

static struct pm8xxx_led_config pm8921_led_configs_rev_b[] = {
#if defined(CONFIG_LGE_PM)
	[0] = {
		.id = PM8XXX_ID_LED_0,
				.mode = PM8XXX_LED_MODE_MANUAL,
				.max_current = PM8921_LC_LED_MAX_CURRENT,
			},
#else
			[0] = {
				.id = PM8XXX_ID_LED_0,
		.mode = PM8XXX_LED_MODE_PWM2,
		.max_current = PM8921_LC_LED_MAX_CURRENT,
		.pwm_channel = 5,
		.pwm_period_us = PM8XXX_LED_PWM_PERIOD,
		.pwm_duty_cycles = &pm8921_led0_pwm_duty_cycles,
	},
#endif

#if defined(CONFIG_MACH_LGE)	/*                               */
			[1] = {
				.id = PM8XXX_ID_LED_1,
				.mode = PM8XXX_LED_MODE_MANUAL,
				.max_current = PM8921_KEY_LED_MAX_CURRENT,
			},
 #if !defined(CONFIG_MACH_APQ8064_J1A)
			[2] = {
				.id = PM8XXX_ID_LED_2,
				.mode = PM8XXX_LED_MODE_MANUAL,
				.max_current = PM8921_LC_LED_MAX_CURRENT,
			},
#endif
#endif

};

static struct pm8xxx_led_platform_data apq8064_pm8921_leds_pdata_rev_b = {
		.led_core = &pm8921_led_core_pdata_rev_b,
		.configs = pm8921_led_configs_rev_b,
		.num_configs = ARRAY_SIZE(pm8921_led_configs_rev_b),
};

static struct pm8xxx_led_config pm8921_led_configs[] = {
			[0] = {
				.id = PM8XXX_ID_LED_0,
				.mode = PM8XXX_LED_MODE_MANUAL,
				.max_current = PM8921_KEY_LED_MAX_CURRENT,
			},
};

static struct pm8xxx_led_platform_data apq8064_pm8921_leds_pdata = {
		.led_core = &pm8921_led_core_pdata,
		.configs = pm8921_led_configs,
		.num_configs = ARRAY_SIZE(pm8921_led_configs),
};

static struct pm8xxx_adc_amux apq8064_pm8921_adc_channels_data[] = {
	{"vcoin", CHANNEL_VCOIN, CHAN_PATH_SCALING2, AMUX_RSV1,
		ADC_DECIMATION_TYPE2, ADC_SCALE_DEFAULT},
	{"vbat", CHANNEL_VBAT, CHAN_PATH_SCALING2, AMUX_RSV1,
		ADC_DECIMATION_TYPE2, ADC_SCALE_DEFAULT},
	{"dcin", CHANNEL_DCIN, CHAN_PATH_SCALING4, AMUX_RSV1,
		ADC_DECIMATION_TYPE2, ADC_SCALE_DEFAULT},
	{"ichg", CHANNEL_ICHG, CHAN_PATH_SCALING1, AMUX_RSV1,
		ADC_DECIMATION_TYPE2, ADC_SCALE_DEFAULT},
	{"vph_pwr", CHANNEL_VPH_PWR, CHAN_PATH_SCALING2, AMUX_RSV1,
		ADC_DECIMATION_TYPE2, ADC_SCALE_DEFAULT},
	{"ibat", CHANNEL_IBAT, CHAN_PATH_SCALING1, AMUX_RSV1,
		ADC_DECIMATION_TYPE2, ADC_SCALE_DEFAULT},
	{"batt_therm", CHANNEL_BATT_THERM, CHAN_PATH_SCALING1, AMUX_RSV2,
		ADC_DECIMATION_TYPE2, ADC_SCALE_BATT_THERM},
	{"batt_id", CHANNEL_BATT_ID, CHAN_PATH_SCALING1, AMUX_RSV1,
		ADC_DECIMATION_TYPE2, ADC_SCALE_DEFAULT},
	{"usbin", CHANNEL_USBIN, CHAN_PATH_SCALING3, AMUX_RSV1,
		ADC_DECIMATION_TYPE2, ADC_SCALE_DEFAULT},
	{"pmic_therm", CHANNEL_DIE_TEMP, CHAN_PATH_SCALING1, AMUX_RSV1,
		ADC_DECIMATION_TYPE2, ADC_SCALE_PMIC_THERM},
	{"625mv", CHANNEL_625MV, CHAN_PATH_SCALING1, AMUX_RSV1,
		ADC_DECIMATION_TYPE2, ADC_SCALE_DEFAULT},
	{"125v", CHANNEL_125V, CHAN_PATH_SCALING1, AMUX_RSV1,
		ADC_DECIMATION_TYPE2, ADC_SCALE_DEFAULT},
	{"chg_temp", CHANNEL_CHG_TEMP, CHAN_PATH_SCALING1, AMUX_RSV1,
		ADC_DECIMATION_TYPE2, ADC_SCALE_DEFAULT},
	{"xo_therm", CHANNEL_MUXOFF, CHAN_PATH_SCALING1, AMUX_RSV0,
		ADC_DECIMATION_TYPE2, ADC_SCALE_XOTHERM},
	{"pa_therm0", ADC_MPP_1_AMUX3, CHAN_PATH_SCALING1, AMUX_RSV1,
		ADC_DECIMATION_TYPE2, ADC_SCALE_APQ_THERM},
	{"usb_id_device", ADC_MPP_1_AMUX6, CHAN_PATH_SCALING1, AMUX_RSV1,
		ADC_DECIMATION_TYPE2, ADC_SCALE_DEFAULT},
};

static struct pm8xxx_adc_properties apq8064_pm8921_adc_data = {
	.adc_vdd_reference	= 1800, /*                            */
	.bitresolution		= 15,
	.bipolar                = 0,
};

static struct pm8xxx_adc_platform_data apq8064_pm8921_adc_pdata = {
	.adc_channel		= apq8064_pm8921_adc_channels_data,
	.adc_num_board_channel	= ARRAY_SIZE(apq8064_pm8921_adc_channels_data),
	.adc_prop		= &apq8064_pm8921_adc_data,
	.adc_mpp_base		= PM8921_MPP_PM_TO_SYS(1),
};

static struct pm8xxx_mpp_platform_data
apq8064_pm8921_mpp_pdata __devinitdata = {
	.mpp_base	= PM8921_MPP_PM_TO_SYS(1),
};

static struct pm8xxx_gpio_platform_data
apq8064_pm8921_gpio_pdata __devinitdata = {
	.gpio_base	= PM8921_GPIO_PM_TO_SYS(1),
};

static struct pm8xxx_irq_platform_data
apq8064_pm8921_irq_pdata __devinitdata = {
	.irq_base		= PM8921_IRQ_BASE,
	.devirq			= MSM_GPIO_TO_INT(74),
	.irq_trigger_flag	= IRQF_TRIGGER_LOW,
	.dev_id			= 0,
};

static struct pm8xxx_rtc_platform_data
apq8064_pm8921_rtc_pdata = {
	.rtc_write_enable       = false,
	.rtc_alarm_powerup      = false,
};

static int apq8064_pm8921_therm_mitigation[] = {
	1100,
	700,
	600,
	325,
};

#ifdef CONFIG_LGE_PM
/*
                            
                                            
                                                      
                               
 */
#define MAX_VOLTAGE_MV		4350
#define CHG_TERM_MA		150
static struct pm8921_charger_platform_data apq8064_pm8921_chg_pdata __devinitdata = {

	/*                                                                                 */
	.safety_time		= 512, /*                                        */
	.update_time		= 30000,
	.max_voltage		= MAX_VOLTAGE_MV,

	/*                                                                      
                                                            */
	.min_voltage		= 3200,
	/*                                                                                           */
	.resume_voltage_delta	= 50, 
	.resume_charge_percent	= 99,
	.term_current		= CHG_TERM_MA,

	/*                                 */
	.vin_min			= 4400,

#ifdef CONFIG_LGE_CHARGER_TEMP_SCENARIO
	/*                                                                   */
	.cool_temp		= 0, /*    */	/*                   */
	.warm_temp		= 0, /*    */	/*                   */
	.cool_bat_chg_current	= 350,	/*                         */
	.warm_bat_chg_current	= 350,
	.temp_level_1		= 510,
	.temp_level_2		= 450,
	.temp_level_3		= 420,
	.temp_level_4		= -50,
	.temp_level_5		= -100,
	/*                                                          */
	.cold_thr		= 1,	/*     */
	.hot_thr		= 0,	/*     */
#else /*                         */
	.cool_temp		= 10,
	.warm_temp		= 40,
	.cool_bat_chg_current	= 350,
	.warm_bat_chg_current	= 350,
#endif

	.temp_check_period	= 1,

#ifdef CONFIG_LGE_PM_LOW_BATT_CHG
	.weak_voltage = 3000,
	.trkl_voltage = 2800,	//                       
	.trkl_current = 200,	//                      
#endif

	/*                                                       */
	/*                      
                                              
                                          
  */
	.max_bat_chg_current	= 1500,

	.cool_bat_voltage	= 4100,
	.warm_bat_voltage	= 4100,
	.thermal_mitigation	= apq8064_pm8921_therm_mitigation,
	.thermal_levels		= ARRAY_SIZE(apq8064_pm8921_therm_mitigation),
	/*                         */
	.led_src_config		= LED_SRC_MIN_VPH_5V,
/*                     */
	.rconn_mohm    = 10,
	.enable_tcxo_warmup_delay = true,	
};
#else /*                        */
#define MAX_VOLTAGE_MV          4200
#define CHG_TERM_MA		100
static struct pm8921_charger_platform_data
apq8064_pm8921_chg_pdata __devinitdata = {
	.safety_time		= 180,
	.update_time		= 60000,
	.max_voltage		= MAX_VOLTAGE_MV,
	.min_voltage		= 3200,
	.uvd_thresh_voltage	= 4050,
	.resume_voltage_delta	= 100,
	.term_current		= CHG_TERM_MA,
	.cool_temp		= 10,
	.warm_temp		= 40,
	.temp_check_period	= 1,
	.max_bat_chg_current	= 1100,
	.cool_bat_chg_current	= 350,
	.warm_bat_chg_current	= 350,
	.cool_bat_voltage	= 4100,
	.warm_bat_voltage	= 4100,
	.thermal_mitigation	= apq8064_pm8921_therm_mitigation,
	.thermal_levels		= ARRAY_SIZE(apq8064_pm8921_therm_mitigation),
	.rconn_mohm		= 18,
	.enable_tcxo_warmup_delay = true,	
};
#endif

static struct pm8xxx_ccadc_platform_data
apq8064_pm8xxx_ccadc_pdata = {
	.r_sense		= 10,
	.calib_delay_ms		= 600000,
};

/*                         
                                                
                                                                           
                                    
                                                                                   
               
                
                      
                     
                                               
*/
#define BMS_2100MAH_OFF3500
static struct pm8921_bms_platform_data
apq8064_pm8921_bms_pdata __devinitdata = {
	.battery_type	= BATT_2200_LGE,
#ifdef BMS_2100MAH_OFF3500
	.battery_type	= BATT_2100_LGE,
#endif
	.ignore_shutdown_soc = 0,
	.r_sense			= 10,
	.v_cutoff			= 3500, //     
	.max_voltage_uv			= MAX_VOLTAGE_MV * 1000,
	.rconn_mohm			= 44, //   
	.shutdown_soc_valid_limit	= 20,
	.adjust_soc_low_threshold	= 25,
	.chg_term_ua			= CHG_TERM_MA * 1000,
};

static unsigned int keymap_rev_a[] = {
	KEY(1, 0, KEY_VOLUMEUP),
	KEY(1, 1, KEY_VOLUMEDOWN),
	KEY(0, 1, KEY_HOMEPAGE),
};

static struct matrix_keymap_data keymap_data_rev_a = {
	.keymap_size    = ARRAY_SIZE(keymap_rev_a),
	.keymap         = keymap_rev_a,
};

static unsigned int keymap[] = {
	KEY(0, 0, KEY_VOLUMEUP),
	KEY(0, 1, KEY_VOLUMEDOWN),
	KEY(1, 1, KEY_HOMEPAGE),
};

static struct matrix_keymap_data keymap_data = {
	.keymap_size    = ARRAY_SIZE(keymap),
	.keymap         = keymap,
};

static struct pm8xxx_keypad_platform_data keypad_data = {
	.input_name             = "gv-keypad-8064",
	.input_phys_device      = "gv-keypad-8064/input0",
	.num_rows               = 2,
	.num_cols               = 5,
	.rows_gpio_start	= PM8921_GPIO_PM_TO_SYS(9),
	.cols_gpio_start	= PM8921_GPIO_PM_TO_SYS(1),
	.debounce_ms            = 15,
	.scan_delay_ms          = 32,
	.row_hold_ns            = 91500,
	.wakeup                 = 1,
	.keymap_data            = &keymap_data,
};
static struct pm8921_platform_data
apq8064_pm8921_platform_data __devinitdata = {
	.regulator_pdatas	= msm8064_pm8921_regulator_pdata,
	.irq_pdata		= &apq8064_pm8921_irq_pdata,
	.gpio_pdata		= &apq8064_pm8921_gpio_pdata,
	.mpp_pdata		= &apq8064_pm8921_mpp_pdata,
	.rtc_pdata		= &apq8064_pm8921_rtc_pdata,
	.pwrkey_pdata		= &apq8064_pm8921_pwrkey_pdata,
	.keypad_pdata		= &keypad_data,
	.misc_pdata		= &apq8064_pm8921_misc_pdata,
	.leds_pdata		= &apq8064_pm8921_leds_pdata,
	.adc_pdata		= &apq8064_pm8921_adc_pdata,
	.charger_pdata		= &apq8064_pm8921_chg_pdata,
	.bms_pdata		= &apq8064_pm8921_bms_pdata,
	.ccadc_pdata		= &apq8064_pm8xxx_ccadc_pdata,
};

static struct pm8xxx_irq_platform_data
apq8064_pm8821_irq_pdata __devinitdata = {
	.irq_base		= PM8821_IRQ_BASE,
	.devirq			= PM8821_SEC_IRQ_N,
	.irq_trigger_flag	= IRQF_TRIGGER_HIGH,
	.dev_id			= 1,
};

static struct pm8xxx_mpp_platform_data
apq8064_pm8821_mpp_pdata __devinitdata = {
	.mpp_base	= PM8821_MPP_PM_TO_SYS(1),
};

static struct pm8821_platform_data
apq8064_pm8821_platform_data __devinitdata = {
	.irq_pdata	= &apq8064_pm8821_irq_pdata,
	.mpp_pdata	= &apq8064_pm8821_mpp_pdata,
};

static struct msm_ssbi_platform_data apq8064_ssbi_pm8921_pdata __devinitdata = {
	.controller_type = MSM_SBI_CTRL_PMIC_ARBITER,
	.slave	= {
		.name		= "pm8921-core",
		.platform_data	= &apq8064_pm8921_platform_data,
	},
};

static struct msm_ssbi_platform_data apq8064_ssbi_pm8821_pdata __devinitdata = {
	.controller_type = MSM_SBI_CTRL_PMIC_ARBITER,
	.slave	= {
		.name		= "pm8821-core",
		.platform_data	= &apq8064_pm8821_platform_data,
	},
};

#ifdef CONFIG_BATTERY_MAX17047

#define I2C_SURF 1
#define I2C_FFA  (1 << 1)
#define I2C_RUMI (1 << 2)
#define I2C_SIM  (1 << 3)
#define I2C_LIQUID (1 << 4)

struct i2c_registry {
	u8                     machs;
	int                    bus;
	struct i2c_board_info *info;
	int                    len;
};


static struct i2c_board_info max17047_i2c_info[] = {
	{
		I2C_BOARD_INFO("max17047", MAX17047_FUELGAUGE_I2C_ADDR),
		//                                                 
	}
};

static struct i2c_registry gv_i2c_pm_subsystem __initdata = {
		I2C_SURF | I2C_FFA | I2C_LIQUID | I2C_RUMI,
		APQ_8064_GSBI1_QUP_I2C_BUS_ID,
		max17047_i2c_info,
		ARRAY_SIZE(max17047_i2c_info),
};

void __init lge_add_i2c_pm_subsystem_devices(void)
{
	/*           
                                      
                                                 
  */

	/*                                                  */
	i2c_register_board_info(gv_i2c_pm_subsystem.bus,
				gv_i2c_pm_subsystem.info,
				gv_i2c_pm_subsystem.len);
}

#endif //                        

void __init apq8064_init_pmic(void)
{
	pmic_reset_irq = PM8921_IRQ_BASE + PM8921_RESOUT_IRQ;


	apq8064_device_ssbi_pmic1.dev.platform_data =
						&apq8064_ssbi_pm8921_pdata;
	apq8064_device_ssbi_pmic2.dev.platform_data =
				&apq8064_ssbi_pm8821_pdata;

	if (socinfo_get_pmic_model() != PMIC_MODEL_PM8917) {
		apq8064_pm8921_platform_data.regulator_pdatas
			= msm8064_pm8921_regulator_pdata;
		apq8064_pm8921_platform_data.num_regulators
			= msm8064_pm8921_regulator_pdata_len;
	} else {
		apq8064_pm8921_platform_data.regulator_pdatas
			= msm8064_pm8917_regulator_pdata;
		apq8064_pm8921_platform_data.num_regulators
			= msm8064_pm8917_regulator_pdata_len;
	}

	if (lge_get_board_revno() == HW_REV_A){
		printk("[%s] lge_get_board_revno() == HW_REV_A, LEDS PORT CHANGE\n", __func__);
		apq8064_pm8921_platform_data.leds_pdata
			= &apq8064_pm8921_leds_pdata_rev_a;
		}else if(lge_get_board_revno() == HW_REV_B){
		printk("[%s] lge_get_board_revno() == HW_REV_B, LEDS PORT CHANGE\n", __func__);
		apq8064_pm8921_platform_data.leds_pdata
			= &apq8064_pm8921_leds_pdata_rev_b;
		}

/*                                           */
	if(lge_get_board_revno() < HW_REV_B) {
		keypad_data.keymap_data = &keymap_data_rev_a;
	}

#ifndef CONFIG_MACH_LGE
/*                                                             */
	if (machine_is_apq8064_mtp()) {
		apq8064_pm8921_bms_pdata.battery_type = BATT_PALLADIUM;
	} else if (machine_is_apq8064_liquid()) {
		apq8064_pm8921_bms_pdata.battery_type = BATT_DESAY;
	} else if (machine_is_apq8064_cdp()) {
		apq8064_pm8921_chg_pdata.has_dc_supply = true;
	}
/*                                                             */
#endif

	apq8064_pm8921_adc_pdata.apq_therm = true;
}
