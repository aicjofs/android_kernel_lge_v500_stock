/* Copyright (c) 2012, LGE Inc.
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

#include <linux/i2c.h>
#include <linux/gpio.h>
#include <linux/platform_device.h>
#include <linux/mfd/pm8xxx/pm8921.h>
#include <linux/regulator/consumer.h>
#include "devices.h"

#include "board-8064.h"

#if defined (CONFIG_SND_SOC_TPA2028D) || defined (CONFIG_SND_SOC_TPA2028D_DUAL_SPEAKER)
#include <sound/tpa2028d.h>
#endif

#include "board-awifi.h"
#ifdef CONFIG_SWITCH_FSA8008
#include <linux/platform_data/hds_fsa8008.h>
#include <mach/board_lge.h>
#endif
#ifdef CONFIG_SWITCH_MAX1462X 
#include <linux/platform_data/hds_max1462x.h>
#include <mach/board_lge.h>
#endif
#include "../../../../sound/soc/codecs/wcd9310.h"


#define TPA2028D_ADDRESS (0xB0>>1)
#define MSM_AMP_EN (PM8921_GPIO_PM_TO_SYS(19))
#define AGC_COMPRESIION_RATE        0
#define AGC_OUTPUT_LIMITER_DISABLE  1
#define AGC_FIXED_GAIN              15
//                                                                           
#ifdef CONFIG_SND_SOC_TPA2028D_DUAL_SPEAKER
#define MSM_AMP2_EN (PM8921_GPIO_PM_TO_SYS(20))
#endif
//                                                                         

#define GPIO_EAR_SENSE_N            38
#define GPIO_EAR_MIC_EN             PM8921_GPIO_PM_TO_SYS(31)
#define GPIO_EARPOL_DETECT          PM8921_GPIO_PM_TO_SYS(32)
//                                                             
#if !defined(CONFIG_MACH_APQ8064_AWIFI)
#define GPIO_MIC_BIAS_CONTROL       PM8921_GPIO_PM_TO_SYS(32)
#endif
//                                                             
#define GPIO_EAR_KEY_INT            23
//                                                                                           
#if defined(CONFIG_MACH_APQ8064_AWIFI)
#define GPIO_2V8_AUDIO_PWR_EN       PM8921_GPIO_PM_TO_SYS(22)
#define GPIO_HPH_3V0_EN             PM8921_GPIO_PM_TO_SYS(16)
#endif
//                                                                                         

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

#if defined(CONFIG_SND_SOC_TPA2028D) || defined(CONFIG_SND_SOC_TPA2028D_DUAL_SPEAKER)
int amp_enable(int on_state)
{
	int err = 0;
	static int init_status = 0;
	struct pm_gpio param = {
		.direction      = PM_GPIO_DIR_OUT,
		.output_buffer  = PM_GPIO_OUT_BUF_CMOS,
		.output_value   = 1,
		.pull           = PM_GPIO_PULL_NO,
		.vin_sel	= PM_GPIO_VIN_S4,
		.out_strength   = PM_GPIO_STRENGTH_MED,
		.function       = PM_GPIO_FUNC_NORMAL,
	};

	if (init_status == 0) {
		err = gpio_request(MSM_AMP_EN, NULL);
		if (err)
			pr_err("%s: Error requesting GPIO %d\n",
					__func__, MSM_AMP_EN);

		err = pm8xxx_gpio_config(MSM_AMP_EN, &param);
		if (err)
			pr_err("%s: Failed to configure gpio %d\n",
					__func__, MSM_AMP_EN);
		else
			init_status++;
	}

	switch (on_state) {
	case 0:
		err = gpio_direction_output(MSM_AMP_EN, 0);
		printk(KERN_INFO "%s: AMP_EN is set to 0\n", __func__);
		break;
	case 1:
		err = gpio_direction_output(MSM_AMP_EN, 1);
		printk(KERN_INFO "%s: AMP_EN is set to 1\n", __func__);
		break;
	case 2:
		printk(KERN_INFO "%s: amp enable bypass(%d)\n", __func__, on_state);
		err = 0;
		break;

	default:
		pr_err("amp enable fail\n");
		err = 1;
		break;
	}
	return err;
}

static struct audio_amp_platform_data amp_platform_data =  {
	.enable = amp_enable,
	.agc_compression_rate = AGC_COMPRESIION_RATE,
	.agc_output_limiter_disable = AGC_OUTPUT_LIMITER_DISABLE,
	.agc_fixed_gain = AGC_FIXED_GAIN,
};
#endif

//                                                                           
#ifdef CONFIG_SND_SOC_TPA2028D_DUAL_SPEAKER
int amp2_enable(int on_state)
{
	int err = 0;
	static int init_status = 0;
	struct pm_gpio param = {
		.direction      = PM_GPIO_DIR_OUT,
		.output_buffer  = PM_GPIO_OUT_BUF_CMOS,
		.output_value   = 1,
		.pull           = PM_GPIO_PULL_NO,
		.vin_sel	= PM_GPIO_VIN_S4,
		.out_strength   = PM_GPIO_STRENGTH_MED,
		.function       = PM_GPIO_FUNC_NORMAL,
	};

	if (init_status == 0) {
		err = gpio_request(MSM_AMP2_EN, NULL);
		if (err)
			pr_err("%s: Error requesting GPIO %d err(%d)\n",
					__func__, MSM_AMP2_EN, err);

		err = pm8xxx_gpio_config(MSM_AMP2_EN, &param);
		if (err)
			pr_err("%s: Failed to configure gpio %d err(%d)\n",
					__func__, MSM_AMP2_EN, err);
		else
			init_status++;
	}

	switch (on_state) {
	case 0:
		err = gpio_direction_output(MSM_AMP2_EN, 0);
		printk(KERN_INFO "%s: AMP2_EN is set to 0 err(%d)\n", __func__, err);
		break;
	case 1:
		err = gpio_direction_output(MSM_AMP2_EN, 1);
		printk(KERN_INFO "%s: AMP2_EN is set to 1 err(%d)\n", __func__, err);
		break;
	case 2:
		printk(KERN_INFO "%s: amp2 enable bypass(%d)\n", __func__, on_state);
		err = 0;
		break;

	default:
		pr_err("amp2 enable fail\n");
		err = 1;
		break;
	}
	return err;
}

static struct audio_amp_platform_data amp2_platform_data =  {
	.enable = amp2_enable,
	.agc_compression_rate = AGC_COMPRESIION_RATE,
	.agc_output_limiter_disable = AGC_OUTPUT_LIMITER_DISABLE,
	.agc_fixed_gain = AGC_FIXED_GAIN,
};

#endif
//                                                                         



static struct i2c_board_info msm_i2c_audiosubsystem_info[] = {
#if defined( CONFIG_SND_SOC_TPA2028D) || defined(CONFIG_SND_SOC_TPA2028D_DUAL_SPEAKER )
	{
		I2C_BOARD_INFO("tpa2028d_amp", TPA2028D_ADDRESS),
		.platform_data = &amp_platform_data,
	}
#endif
};

//                                                                           
#ifdef CONFIG_SND_SOC_TPA2028D_DUAL_SPEAKER
static struct i2c_board_info msm_i2c_audiosubsystem2_info[] = {
	{
		I2C_BOARD_INFO("tpa2028d2_amp", TPA2028D_ADDRESS),
		.platform_data = &amp2_platform_data, 	
	} 
};
#endif
//                                                                         

static struct i2c_registry msm_i2c_audiosubsystem __initdata = {
	/*                                  */
	I2C_SURF | I2C_FFA | I2C_RUMI | I2C_SIM | I2C_LIQUID,
	APQ_8064_GSBI1_QUP_I2C_BUS_ID,
	msm_i2c_audiosubsystem_info,
	ARRAY_SIZE(msm_i2c_audiosubsystem_info),
};

//                                                                           
#ifdef CONFIG_SND_SOC_TPA2028D_DUAL_SPEAKER
#define APQ_8064_GSBI7_QUP_I2C_BUS_ID 7

static struct i2c_registry msm_i2c_audiosubsystem2 __initdata = {
	/*                                  */
	I2C_SURF | I2C_FFA | I2C_RUMI | I2C_SIM | I2C_LIQUID,
	APQ_8064_GSBI7_QUP_I2C_BUS_ID,
	msm_i2c_audiosubsystem2_info,
	ARRAY_SIZE(msm_i2c_audiosubsystem2_info),
};
#endif
//                                                                         


static void __init lge_add_i2c_tpa2028d_devices(void)
{
	/*                                                  */
	i2c_register_board_info(msm_i2c_audiosubsystem.bus,
				msm_i2c_audiosubsystem.info,
				msm_i2c_audiosubsystem.len);

//                                                                           
#ifdef CONFIG_SND_SOC_TPA2028D_DUAL_SPEAKER
	i2c_register_board_info(msm_i2c_audiosubsystem2.bus,
				msm_i2c_audiosubsystem2.info,
				msm_i2c_audiosubsystem2.len);
	#endif
	//                                                                          
}

#ifdef CONFIG_SWITCH_FSA8008
static struct fsa8008_platform_data lge_hs_pdata_fsa8008 = {
	.switch_name = "h2w",
	.keypad_name = "hs_detect",

	.key_code = KEY_MEDIA,

	.gpio_detect = GPIO_EAR_SENSE_N,
	.gpio_mic_en = GPIO_EAR_MIC_EN,
	.gpio_jpole  = GPIO_EARPOL_DETECT,
	.gpio_key    = GPIO_EAR_KEY_INT,
	.set_headset_mic_bias = NULL,
	//                                                                                           
	#if defined(CONFIG_MACH_APQ8064_AWIFI)
	.gpio_power_en = GPIO_2V8_AUDIO_PWR_EN,
	.gpio_hph_en = GPIO_HPH_3V0_EN,
	#endif
	//                                                                                         

	.latency_for_detection = 75,
};

static struct platform_device lge_hsd_device_fsa8008 = {
	.name = "fsa8008",
	.id   = -1,
	.dev = {
		.platform_data = &lge_hs_pdata_fsa8008,
	},
};

static int __init lge_hsd_fsa8008_init(void)
{
  //                          
 //                                        
	printk(KERN_INFO "lge_hsd_fsa8008_init\n");

  //                                     


		//                                                                                                 
		#if defined(CONFIG_MACH_APQ8064_AWIFI)
		lge_hs_pdata_fsa8008.set_headset_mic_bias = NULL;
		#else
        lge_hs_pdata_fsa8008.set_headset_mic_bias = tabla_codec_micbias3_ctl;
		#endif
		//                                                                                               
   //                                                                                                                             

	return platform_device_register(&lge_hsd_device_fsa8008);
}

static void __exit lge_hsd_fsa8008_exit(void)
{
	printk(KERN_INFO "lge_hsd_fsa8008_exit\n");
	platform_device_unregister(&lge_hsd_device_fsa8008);
}
#endif

#ifdef CONFIG_SWITCH_MAX1462X
static struct max1462x_platform_data lge_hs_pdata_max1462x = {
	.switch_name = "h2w",
	.keypad_name = "hs_detect",

	.key_code = 0,	                                   //                                         

	.gpio_mode	= GPIO_EAR_MIC_EN,		              //                                
	.gpio_det	= GPIO_EAR_SENSE_N,		              //                                 
	.gpio_swd	= GPIO_EAR_KEY_INT,		       //                                           
//                                                             
#if !defined(CONFIG_MACH_APQ8064_AWIFI)
	.external_ldo_mic_bias	= GPIO_MIC_BIAS_CONTROL,	//                                  
#endif
//                                                             

	.latency_for_detection = 75,
	.latency_for_key = 50,                                 //                           

    .adc_mpp_num = PM8XXX_AMUX_MPP_1,   /*                                              */
    .adc_channel = ADC_MPP_1_AMUX6,     /*                                           */

    .set_headset_mic_bias = NULL,       /*                                               */
};

static struct platform_device lge_hsd_device_max1462x = {
	.name = "max1462x",
	.id   = -1,
	.dev = {
		.platform_data = &lge_hs_pdata_max1462x,
	},
};

static int __init lge_hsd_max1462x_init(void)
{
 //                           
//                                        
    printk(KERN_INFO "lge_hsd_max1462x_init\n");

//                                       
//                                 
#if !(defined(CONFIG_MACH_APQ8064_GKATT) || defined(CONFIG_MACH_APQ8064_GKGLOBAL) || defined(CONFIG_MACH_APQ8064_AWIFI))
        lge_hs_pdata_max1462x.set_headset_mic_bias = set_headset_mic_bias_l10;
        printk(KERN_INFO "lge_bd_rev : %d, >= bd_rev : %d\n", lge_bd_rev, bd_rev);

#else /*                                   */
    lge_hs_pdata_max1462x.set_headset_mic_bias = NULL;
    //                                                                         
#endif /*                                   */
	return platform_device_register(&lge_hsd_device_max1462x);
}

static void __exit lge_hsd_max1462x_exit(void)
{
	printk(KERN_INFO "lge_hsd_max1462x_exit\n");
	platform_device_unregister(&lge_hsd_device_max1462x);
}

#endif
void __init lge_add_sound_devices(void)
{
	lge_add_i2c_tpa2028d_devices();
//                                                             
	if (lge_get_board_revno() == HW_REV_EVB2){
#ifdef CONFIG_SWITCH_FSA8008
	lge_hsd_fsa8008_init();
#endif
	} else {
#ifdef CONFIG_SWITCH_MAX1462X
	lge_hsd_max1462x_init();
#endif
	}
//                                                             
}
