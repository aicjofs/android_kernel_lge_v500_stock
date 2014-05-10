/* Copyright (c) 2011-2012, The Linux Foundation. All rights reserved.
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
#include <mach/gpiomux.h>
#include <mach/restart.h>
#include <mach/socinfo.h>
#if defined(CONFIG_MACH_LGE)
#include <mach/board_lge.h>
#include <linux/i2c.h>
#include "devices.h"
#include "board-j1.h"
#else
#include "board-8064.h"
#endif //                            

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

#ifdef CONFIG_LGE_PM
/*                                    */
static struct pm8xxx_gpio_init pm8921_gpios[] __initdata = {
//                       
//                                             
#if defined(CONFIG_LGE_BROADCAST_TDMB)
	PM8921_GPIO_OUTPUT(11, 0, HIGH), /*                            */
	PM8921_GPIO_OUTPUT(12, 1, HIGH), /*                            */
#endif

//                               
#if 0
#if defined(CONFIG_LGE_BROADCAST_ONESEG) && defined(CONFIG_LGE_BROADCAST_ONESEG_FC8150) //       

#if 1 	/*                               */
	PM8921_GPIO_OUTPUT(11, 0, HIGH),
	PM8921_GPIO_OUTPUT(12, 1, HIGH),
#else 	/*                            */
	PM8921_GPIO_OUTPUT(11, 1, HIGH),
	PM8921_GPIO_OUTPUT(12, 0, HIGH),
#endif
#endif //                                                                                   
#endif


#if defined(CONFIG_LGE_BROADCAST_ONESEG) && defined(CONFIG_LGE_BROADCAST_ONESEG_MB86A35S) //      
	PM8921_GPIO_OUTPUT(11, 1, HIGH), /*                            */
#endif //                                                                                     
//                       

#if defined(CONFIG_LGE_IRDA)
	PM8921_GPIO_OUTPUT(17, 0, HIGH), /*               */
	PM8921_GPIO_OUTPUT(18, 0, HIGH), /*            */
#else
	PM8921_GPIO_OUTPUT(17, 0, HIGH), /*            */
#endif
	PM8921_GPIO_OUTPUT(19, 0, HIGH), /*            */
	PM8921_GPIO_OUTPUT(31, 0, HIGH), /*                           */
	PM8921_GPIO_INPUT(32, PM_GPIO_PULL_UP_1P5), /*                              */
	PM8921_GPIO_OUTPUT(33, 0, HIGH), /*           */
	PM8921_GPIO_OUTPUT(34, 0, HIGH), /*             */

	#if defined(CONFIG_MACH_APQ8064_J1D) || defined(CONFIG_MACH_APQ8064_J1KD)
	PM8921_GPIO_OUTPUT(15, 0, HIGH),
	#endif
};
#else //                 
/*                                    */
static struct pm8xxx_gpio_init pm8921_gpios[] __initdata = {
	PM8921_GPIO_OUTPUT(14, 1, HIGH),	/*                   */
	PM8921_GPIO_OUTPUT_BUFCONF(25, 0, LOW, CMOS), /*              */
	PM8921_GPIO_OUTPUT_FUNC(26, 0, PM_GPIO_FUNC_2), /*                   */
	PM8921_GPIO_OUTPUT_VIN(30, 1, PM_GPIO_VIN_VPH), /*                  */
	PM8921_GPIO_OUTPUT_BUFCONF(36, 1, LOW, OPEN_DRAIN),
	PM8921_GPIO_OUTPUT_FUNC(44, 0, PM_GPIO_FUNC_2),
	PM8921_GPIO_OUTPUT(33, 0, HIGH),
	PM8921_GPIO_OUTPUT(20, 0, HIGH),
	PM8921_GPIO_INPUT(35, PM_GPIO_PULL_UP_30),
	PM8921_GPIO_INPUT(38, PM_GPIO_PULL_UP_30),
	/*                   */
	PM8921_GPIO_OUTPUT(34, 0, MED),
	PM8921_GPIO_OUTPUT(13, 0, HIGH),               /*                 */
        PM8921_GPIO_INPUT(12, PM_GPIO_PULL_UP_30),     /*             */
};
#endif //                            

#ifndef CONFIG_LGE_PM
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
	PM8921_GPIO_OUTPUT(14, 1, HIGH),	/*                   */
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
	PM8921_GPIO_INPUT(17, PM_GPIO_PULL_UP_1P5),	/*       */
};

static struct pm8xxx_gpio_init pm8921_8917_cdp_ts_gpios[] __initdata = {
	PM8921_GPIO_OUTPUT(23, 0, HIGH),	/*                       */
};

static struct pm8xxx_gpio_init pm8921_mpq_gpios[] __initdata = {
	PM8921_GPIO_INIT(27, PM_GPIO_DIR_IN, PM_GPIO_OUT_BUF_CMOS, 0,
			PM_GPIO_PULL_NO, PM_GPIO_VIN_VPH, PM_GPIO_STRENGTH_NO,
			PM_GPIO_FUNC_NORMAL, 0, 0),
};

/*                                   */
static struct pm8xxx_mpp_init pm8xxx_mpps[] __initdata = {
	PM8921_MPP_INIT(3, D_OUTPUT, PM8921_MPP_DIG_LEVEL_VPH, DOUT_CTRL_LOW),
	/*                                                                    */
	PM8921_MPP_INIT(7, D_OUTPUT, PM8921_MPP_DIG_LEVEL_VPH, DOUT_CTRL_LOW),
	PM8921_MPP_INIT(8, D_OUTPUT, PM8921_MPP_DIG_LEVEL_S4, DOUT_CTRL_LOW),
	/*                                                                   */
	PM8921_MPP_INIT(9, D_INPUT, PM8921_MPP_DIG_LEVEL_S4, DIN_TO_INT),
	/*              */
	PM8921_MPP_INIT(1, D_OUTPUT, PM8921_MPP_DIG_LEVEL_VPH, DOUT_CTRL_HIGH),
};
#endif

#ifndef CONFIG_LGE_PM
static struct pm8xxx_gpio_init pm8921_sglte2_gpios[] __initdata = {
	PM8921_GPIO_OUTPUT(23, 1, HIGH),		/*                   */
	PM8921_GPIO_OUTPUT(21, 1, HIGH),		/*                  */
};
#endif

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
#ifndef CONFIG_LGE_PM
	int i, rc;
#endif

	if (socinfo_get_pmic_model() != PMIC_MODEL_PM8917)
		apq8064_configure_gpios(pm8921_gpios, ARRAY_SIZE(pm8921_gpios));
#ifndef CONFIG_LGE_PM
	else
		apq8064_configure_gpios(pm8917_gpios, ARRAY_SIZE(pm8917_gpios));
#endif

#ifndef CONFIG_LGE_PM
	if (machine_is_apq8064_cdp() || machine_is_apq8064_liquid()) {
		if (socinfo_get_pmic_model() != PMIC_MODEL_PM8917)
			apq8064_configure_gpios(pm8921_cdp_kp_gpios,
					ARRAY_SIZE(pm8921_cdp_kp_gpios));
		else
			apq8064_configure_gpios(pm8917_cdp_kp_gpios,
					ARRAY_SIZE(pm8917_cdp_kp_gpios));

		apq8064_configure_gpios(pm8921_8917_cdp_ts_gpios,
				ARRAY_SIZE(pm8921_8917_cdp_ts_gpios));
	}

	if (machine_is_apq8064_mtp()) {
		apq8064_configure_gpios(pm8921_mtp_kp_gpios,
					ARRAY_SIZE(pm8921_mtp_kp_gpios));
		if (socinfo_get_platform_subtype() ==
					PLATFORM_SUBTYPE_SGLTE2) {
			apq8064_configure_gpios(pm8921_sglte2_gpios,
					ARRAY_SIZE(pm8921_sglte2_gpios));
		}
	}

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

#if defined(CONFIG_MACH_APQ8064_J1D)||defined(CONFIG_MACH_APQ8064_J1KD)
#define PM8921_LC_LED_MAX_CURRENT	2	/*         */
#define PM8921_KEY_LED_MAX_CURRENT	8	/*         */
#elif (defined(CONFIG_MACH_APQ8064_J1A)) || (defined(CONFIG_MACH_APQ8064_J1U)) || defined(CONFIG_MACH_APQ8064_J1SP)
#define PM8921_LC_LED_MAX_CURRENT	12	/*          */
#define PM8921_KEY_LED_MAX_CURRENT	6	/*         */
#else
#define PM8921_LC_LED_MAX_CURRENT	12	/*          */
#define PM8921_KEY_LED_MAX_CURRENT	4	/*         */
#endif
#define PM8921_LC_LED_LOW_CURRENT	1	/*         */

#ifdef CONFIG_LGE_PM_PWM_LED
#if defined(CONFIG_MACH_APQ8064_J1SP) || defined(CONFIG_MACH_APQ8064_J1A)
#define PM8XXX_LED_PWM_DUTY_MS0		50
#define PM8XXX_LED_PWM_DUTY_MS1		512
#else
#define PM8XXX_LED_PWM_DUTY_MS0		50
#define PM8XXX_LED_PWM_DUTY_MS1		50
#endif
#define PM8XXX_LED_PWM_PERIOD		1000
#else
#define PM8XXX_LED_PWM_DUTY_MS		20
#define PM8XXX_LED_PWM_PERIOD		1000
#endif

/* 
                                                              
                            
 */
#define PM8XXX_PWM_CHANNEL_NONE		-1

static struct led_info pm8921_led_info[] = {
#if defined(CONFIG_LGE_PM)
	[0] = {
		.name			= "led:red",
	},
#else

	[0] = {
		.name			= "led:red",
		.default_trigger	= "battery-charging",
	},
#endif
#if defined(CONFIG_MACH_LGE)
	[1] = {
		.name			= "button-backlight",
	},
#if !defined(CONFIG_MACH_APQ8064_J1A)
	[2] = {
		.name			= "led:green",
	},
#endif
#else
	[1] = {
		.name			= "led:green",
		.default_trigger	= "battery-full",
	},
#endif
};

static struct led_platform_data pm8921_led_core_pdata = {
	.num_leds = ARRAY_SIZE(pm8921_led_info),
	.leds = pm8921_led_info,
};

#if !defined(CONFIG_LGE_PM)

static int pm8921_led0_pwm_duty_pcts[56] = {
	1, 4, 8, 12, 16, 20, 24, 28, 32, 36,
	40, 44, 46, 52, 56, 60, 64, 68, 72, 76,
	80, 84, 88, 92, 96, 100, 100, 100, 98, 95,
	92, 88, 84, 82, 78, 74, 70, 66, 62, 58,
	58, 54, 50, 48, 42, 38, 34, 30, 26, 22,
	14, 10, 6, 4, 1
};

/*
                                                               
                                                               
                                   
 */
static struct pm8xxx_pwm_duty_cycles pm8921_led0_pwm_duty_cycles = {
	.duty_pcts = (int *)&pm8921_led0_pwm_duty_pcts,
	.num_duty_pcts = ARRAY_SIZE(pm8921_led0_pwm_duty_pcts),
	.duty_ms = PM8XXX_LED_PWM_DUTY_MS,
	.start_idx = 0,
};
#endif

#ifdef CONFIG_LGE_PM_PWM_LED
static int pm8921_led0_pwm_duty_pcts0[60] = {
	1, 2, 8, 10, 14, 18, 20, 24, 30, 34,
	36, 40, 42, 48, 50, 55, 58, 60, 62, 64,
	66, 68, 71, 73, 76, 80, 80, 80, 76, 73,
	71, 68, 66, 64, 62, 60, 58, 56, 54, 52,
	50, 48, 46, 44, 40, 36, 34, 30, 24, 20,
	18, 16, 14, 12, 10, 8, 6, 4, 1
};

#if defined(CONFIG_MACH_APQ8064_J1SP) || defined(CONFIG_MACH_APQ8064_J1A)
static int pm8921_led0_pwm_duty_pcts1[60] = {
	60, 80, 60, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
};
#else
//                   
static int pm8921_led0_pwm_duty_pcts1[60] = {
	60, 65, 70, 75, 80, 80, 75, 70, 65, 60,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
};
#endif

static struct pm8xxx_pwm_duty_cycles pm8921_led0_pwm_duty_cycles = {
	.duty_pcts0 = (int *)&pm8921_led0_pwm_duty_pcts0,
	.duty_pcts1 = (int *)&pm8921_led0_pwm_duty_pcts1,
	.num_duty_pcts0 = ARRAY_SIZE(pm8921_led0_pwm_duty_pcts0),
	.num_duty_pcts1 = ARRAY_SIZE(pm8921_led0_pwm_duty_pcts1),
	.duty_ms0 = PM8XXX_LED_PWM_DUTY_MS0,
	.duty_ms1 = PM8XXX_LED_PWM_DUTY_MS1,
	.start_idx = 0,
};
#endif

static struct pm8xxx_led_config pm8921_led_configs[] = {
#if defined(CONFIG_LGE_PM)
	[0] = {
		.id = PM8XXX_ID_LED_0,
#ifdef CONFIG_LGE_PM_PWM_LED
		.mode = PM8XXX_LED_MODE_PWM2,
		.pwm_channel = 5,
		.pwm_period_us = PM8XXX_LED_PWM_PERIOD,
		.pwm_duty_cycles = &pm8921_led0_pwm_duty_cycles,
#else
		.mode = PM8XXX_LED_MODE_MANUAL,
#endif
		.max_current = PM8921_LC_LED_MAX_CURRENT,
	},
#else //             
	[0] = {
		.id = PM8XXX_ID_LED_0,
		.mode = PM8XXX_LED_MODE_PWM2,
		.max_current = PM8921_LC_LED_MAX_CURRENT,
		.pwm_channel = 5,
		.pwm_period_us = PM8XXX_LED_PWM_PERIOD,
		.pwm_duty_cycles = &pm8921_led0_pwm_duty_cycles,
	},
#endif
#if defined(CONFIG_MACH_LGE)
	[1] = {
		.id = PM8XXX_ID_LED_1,
		.mode = PM8XXX_LED_MODE_MANUAL,
		.max_current = PM8921_KEY_LED_MAX_CURRENT,

	},
#if !defined(CONFIG_MACH_APQ8064_J1A)
	[2] = {
		.id = PM8XXX_ID_LED_2,
#ifdef CONFIG_LGE_PM_PWM_LED
		.mode = PM8XXX_LED_MODE_PWM1,
		.pwm_channel = 4,
		.pwm_period_us = PM8XXX_LED_PWM_PERIOD,
		.pwm_duty_cycles = &pm8921_led0_pwm_duty_cycles,
#else
		.mode = PM8XXX_LED_MODE_MANUAL,
#endif
		.max_current = PM8921_LC_LED_MAX_CURRENT,
	},
#endif
#else //            
	[1] = {
		.id = PM8XXX_ID_LED_1,
		.mode = PM8XXX_LED_MODE_PWM1,
		.max_current = PM8921_LC_LED_MAX_CURRENT,
		.pwm_channel = 4,
		.pwm_period_us = PM8XXX_LED_PWM_PERIOD,
		.pwm_duty_cycles = &pm8921_led0_pwm_duty_cycles,
	},
#endif
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
/*                                                          */
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
#define MAX_VOLTAGE_MV		4360
#define CHG_TERM_MA		100
#ifdef CONFIG_LGE_PM
	/*            */
#define EOC_CHECK_SOC	1
#endif

static struct pm8921_charger_platform_data apq8064_pm8921_chg_pdata __devinitdata = {

	/*                                                                                 */
	//                                                                 
	.update_time		= 60000,
	.max_voltage		= MAX_VOLTAGE_MV,

	/*                                                                      
                                                            */
	.min_voltage		= 3200,
	/*                                                                                           */
	.resume_voltage_delta	= 60, //         
	.resume_charge_percent	= 99,
	.term_current		= CHG_TERM_MA,

	/*                                 */
	.vin_min			= 4400,
#ifdef CONFIG_LGE_CHARGER_TEMP_SCENARIO
	/*                                                                   */
/*                                                                         */
	.cool_temp		= 0, /*    */	/*                   */
	.warm_temp		= 0, /*    */	/*                   */
	.cool_bat_chg_current	= 350,	/*                         */
	.warm_bat_chg_current	= 350,
/*                                                                       */
#if defined(CONFIG_MACH_APQ8064_J1SP)
	.temp_level_1		= 520, //          
/*                                         */
	.temp_level_1_1 	= 500,
	.temp_level_2		= 490, //          
	.temp_level_3		= 420,
	.temp_level_4		= -50,
	.temp_level_5		= -60, //          
#else
	.temp_level_1		= 550,
	.temp_level_2		= 450,
	.temp_level_3		= 420,
	.temp_level_4		= -50,
	.temp_level_5		= -100,
#endif
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
#if defined(CONFIG_MACH_APQ8064_J1D)||defined(CONFIG_MACH_APQ8064_J1KD)
	.max_bat_chg_current	= 1500,
#elif defined(CONFIG_MACH_APQ8064_GKSK) || defined(CONFIG_MACH_APQ8064_GKKT) || defined(CONFIG_MACH_APQ8064_GKU) || defined(CONFIG_MACH_APQ8064_GKATT) || defined(CONFIG_MACH_APQ8064_GKGLOBAL)
	.max_bat_chg_current	= 1800,
#else
	.max_bat_chg_current	= 1350,
#endif

	.cool_bat_voltage	= 4100,
	.warm_bat_voltage	= 4100,
	.thermal_mitigation	= apq8064_pm8921_therm_mitigation,
	.thermal_levels		= ARRAY_SIZE(apq8064_pm8921_therm_mitigation),
	/*                         */
	.led_src_config = LED_SRC_5V,
	/*                                                         */
/*                     */
	.rconn_mohm    = 18,
#ifdef CONFIG_LGE_PM
	/*            */
	.eoc_check_soc	= EOC_CHECK_SOC,
#endif
	.enable_tcxo_warmup_delay = true,
};
#else /*                        */
#define MAX_VOLTAGE_MV          4200
#define CHG_TERM_MA		100
static struct pm8921_charger_platform_data
apq8064_pm8921_chg_pdata __devinitdata = {
	.update_time		= 60000,
	.max_voltage		= MAX_VOLTAGE_MV,
	.min_voltage		= 3200,
	.uvd_thresh_voltage	= 4050,
	.resume_voltage_delta	= 100,
	.term_current		= CHG_TERM_MA,
	.cool_temp		= 10,
	.warm_temp		= 45,
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
	.r_sense_uohm		= 10000,
	.calib_delay_ms		= 600000,
};

/*                         
                                                
                                                                           
                                    
                                                                                   
               
                
                      
                     
                                               
*/
#define BMS_2100MAH_OFF3500


static struct pm8921_bms_platform_data
apq8064_pm8921_bms_pdata __devinitdata = {
#if defined(CONFIG_MACH_APQ8064_J1D)||defined(CONFIG_MACH_APQ8064_J1KD)
	.battery_type	= BATT_2200_LGE,
	.v_cutoff		= 3500,
#else
	.battery_type	= BATT_2100_LGE,
	.v_cutoff		= 3500,
#endif
	.ignore_shutdown_soc = 0,
	.r_sense_uohm			= 10000,
	.max_voltage_uv		= MAX_VOLTAGE_MV * 1000,
	.shutdown_soc_valid_limit = 20,
	.adjust_soc_low_threshold = 25,
	.rconn_mohm    = 44,
	.chg_term_ua   = CHG_TERM_MA * 1000,
	.normal_voltage_calc_ms		= 20000,
	.low_voltage_calc_ms		= 1000,
	.alarm_low_mv			= 3400,
	.alarm_high_mv			= 4000,
	.high_ocv_correction_limit_uv	= 50,
	.low_ocv_correction_limit_uv	= 100,
	.hold_soc_est			= 3,
	.enable_fcc_learning		= 1,
	.min_fcc_learning_soc		= 20,
	.min_fcc_ocv_pc			= 30,
	.min_fcc_learning_samples	= 5,
#ifdef CONFIG_LGE_PM
	/*            */
	.first_fixed_iavg_ma = 500,
	.eoc_check_soc  = EOC_CHECK_SOC,
#endif
};

#ifdef CONFIG_LGE_PM
/*                                                */
static unsigned int keymap[] = {
	KEY(0, 0, KEY_VOLUMEUP),
	KEY(0, 1, KEY_VOLUMEDOWN),
	//                               
	//                            
#if defined(CONFIG_MACH_APQ8064_GKKT)||defined(CONFIG_MACH_APQ8064_GKSK)||defined(CONFIG_MACH_APQ8064_GKU)||defined(CONFIG_MACH_APQ8064_GKATT) || defined(CONFIG_MACH_APQ8064_GKGLOBAL)
	KEY(1, 1, KEY_HOMEPAGE),
	KEY(1, 0, KEY_BACK), //                                                    
#endif
};

static struct matrix_keymap_data keymap_data = {
	.keymap_size    = ARRAY_SIZE(keymap),
	.keymap         = keymap,
};

static struct pm8xxx_keypad_platform_data keypad_data = {
	.input_name             = "keypad_8064",
	.input_phys_device      = "keypad_8064/input0",
#if defined(CONFIG_MACH_APQ8064_GKKT)||defined(CONFIG_MACH_APQ8064_GKSK)||defined(CONFIG_MACH_APQ8064_GKU)||defined(CONFIG_MACH_APQ8064_GKATT) || defined(CONFIG_MACH_APQ8064_GKGLOBAL)
	.num_rows				= 2,
	.num_cols				= 5,
#else
	.num_rows               = 1,
	.num_cols               = 5,
#endif
	.rows_gpio_start	= PM8921_GPIO_PM_TO_SYS(9),
	.cols_gpio_start	= PM8921_GPIO_PM_TO_SYS(1),
	.debounce_ms            = 15,
	.scan_delay_ms          = 32,
	.row_hold_ns            = 91500,
	.wakeup                 = 1,
	.keymap_data            = &keymap_data,
};
#endif //                            

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

void __init apq8064_init_pmic(void)
{
	pmic_reset_irq = PM8921_IRQ_BASE + PM8921_RESOUT_IRQ;
#if defined(CONFIG_MACH_APQ8064_J1D) || defined(CONFIG_MACH_APQ8064_J1KD)
	if (lge_get_board_revno() == HW_REV_B) {
		keymap[0] = KEY(0, 0, KEY_VOLUMEDOWN);
		keymap[1] = KEY(0, 1, KEY_VOLUMEUP);
	}
#endif
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

#if !defined(CONFIG_MACH_LGE)
	if (machine_is_apq8064_mtp()) {
		apq8064_pm8921_bms_pdata.battery_type = BATT_PALLADIUM;
	} else if (machine_is_apq8064_liquid()) {
		apq8064_pm8921_bms_pdata.battery_type = BATT_DESAY;
	} else if (machine_is_apq8064_cdp()) {
		apq8064_pm8921_chg_pdata.has_dc_supply = true;
	}
#endif

	apq8064_pm8921_adc_pdata.apq_therm = true;

	if (!machine_is_apq8064_mtp() && !machine_is_apq8064_liquid())
		apq8064_pm8921_chg_pdata.battery_less_hardware = 1;

	if (machine_is_mpq8064_hrd())
		apq8064_pm8921_chg_pdata.disable_chg_rmvl_wrkarnd = 1;
}
