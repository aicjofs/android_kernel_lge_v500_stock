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
 *
 */

#include <linux/init.h>
#include <linux/ioport.h>
#include <linux/platform_device.h>
#include <linux/gpio.h>
#include <linux/bootmem.h>
#include <asm/mach-types.h>
#include <asm/mach/mmc.h>
#include <mach/msm_bus_board.h>
#include <mach/board.h>
#include <mach/gpiomux.h>
#include "devices.h"
#include "board-gvar.h"
#include "board-storage-common-a.h"

#include <mach/board_lge.h>
#ifdef CONFIG_MMC_MSM_SDC4_SUPPORT
#include <linux/skbuff.h>
#include <linux/wlan_plat.h>
#include <linux/pm_qos.h>
#endif /*                             */


/*                                */
enum sdcc_controllers {
	SDCC1,
	SDCC2,
	SDCC3,
	SDCC4,
	MAX_SDCC_CONTROLLER
};

/*                                              */
static struct msm_mmc_reg_data mmc_vdd_reg_data[MAX_SDCC_CONTROLLER] = {
	/*                             */
	[SDCC1] = {
		.name = "sdc_vdd",
		.high_vol_level = 2950000,
		.low_vol_level = 2950000,
		.always_on = 1,
		.lpm_sup = 1,
		.lpm_uA = 9000,
		.hpm_uA = 200000, /*       */
	},
	/*                                      */
	[SDCC3] = {
		.name = "sdc_vdd",
		.high_vol_level = 2950000,
		.low_vol_level = 2950000,
#ifdef CONFIG_LGE_SD_LIFETIME_STRENGTHEN
        .always_on = 1,
#endif
		.hpm_uA = 800000, /*       */
	}
};

/*                                                        */
static struct msm_mmc_reg_data mmc_vdd_io_reg_data[MAX_SDCC_CONTROLLER] = {
	/*                             */
	[SDCC1] = {
		.name = "sdc_vdd_io",
		.always_on = 1,
		.high_vol_level = 1800000,
		.low_vol_level = 1800000,
		.hpm_uA = 200000, /*       */
	},
	/*                                      */
	[SDCC3] = {
		.name = "sdc_vdd_io",
		.high_vol_level = 2950000,
		.low_vol_level = 1850000,
		.always_on = 1,
		.lpm_sup = 1,
		/*                                       */
		.hpm_uA = 16000,
		/*
                                                            
                                                       
                  
   */
		.lpm_uA = 2000,
	}
};

static struct msm_mmc_slot_reg_data mmc_slot_vreg_data[MAX_SDCC_CONTROLLER] = {
	/*                             */
	[SDCC1] = {
		.vdd_data = &mmc_vdd_reg_data[SDCC1],
		.vdd_io_data = &mmc_vdd_io_reg_data[SDCC1],
	},
	/*                                      */
	[SDCC3] = {
		.vdd_data = &mmc_vdd_reg_data[SDCC3],
		.vdd_io_data = &mmc_vdd_io_reg_data[SDCC3],
	}
};

/*               */
static struct msm_mmc_pad_drv sdc1_pad_drv_on_cfg[] = {
	{TLMM_HDRV_SDC1_CLK, GPIO_CFG_16MA},
	{TLMM_HDRV_SDC1_CMD, GPIO_CFG_10MA},
	{TLMM_HDRV_SDC1_DATA, GPIO_CFG_10MA}
};

static struct msm_mmc_pad_drv sdc1_pad_drv_off_cfg[] = {
	{TLMM_HDRV_SDC1_CLK, GPIO_CFG_2MA},
	{TLMM_HDRV_SDC1_CMD, GPIO_CFG_2MA},
	{TLMM_HDRV_SDC1_DATA, GPIO_CFG_2MA}
};

static struct msm_mmc_pad_pull sdc1_pad_pull_on_cfg[] = {
	{TLMM_PULL_SDC1_CLK, GPIO_CFG_NO_PULL},
	{TLMM_PULL_SDC1_CMD, GPIO_CFG_PULL_UP},
	{TLMM_PULL_SDC1_DATA, GPIO_CFG_PULL_UP}
};

static struct msm_mmc_pad_pull sdc1_pad_pull_off_cfg[] = {
	{TLMM_PULL_SDC1_CLK, GPIO_CFG_NO_PULL},
	{TLMM_PULL_SDC1_CMD, GPIO_CFG_PULL_UP},
	{TLMM_PULL_SDC1_DATA, GPIO_CFG_PULL_UP}
};

/*               */
static struct msm_mmc_pad_drv sdc3_pad_drv_on_cfg[] = {
	{TLMM_HDRV_SDC3_CLK, GPIO_CFG_8MA},
	{TLMM_HDRV_SDC3_CMD, GPIO_CFG_8MA},
	{TLMM_HDRV_SDC3_DATA, GPIO_CFG_8MA}
};

static struct msm_mmc_pad_drv sdc3_pad_drv_off_cfg[] = {
	{TLMM_HDRV_SDC3_CLK, GPIO_CFG_2MA},
	{TLMM_HDRV_SDC3_CMD, GPIO_CFG_2MA},
	{TLMM_HDRV_SDC3_DATA, GPIO_CFG_2MA}
};

static struct msm_mmc_pad_pull sdc3_pad_pull_on_cfg[] = {
	{TLMM_PULL_SDC3_CLK, GPIO_CFG_NO_PULL},
	{TLMM_PULL_SDC3_CMD, GPIO_CFG_PULL_UP},
	{TLMM_PULL_SDC3_DATA, GPIO_CFG_PULL_UP}
};

static struct msm_mmc_pad_pull sdc3_pad_pull_off_cfg[] = {
	{TLMM_PULL_SDC3_CLK, GPIO_CFG_NO_PULL},
#ifdef CONFIG_LGE_SD_LIFETIME_STRENGTHEN
	{TLMM_PULL_SDC3_CMD, GPIO_CFG_PULL_UP},
	{TLMM_PULL_SDC3_DATA, GPIO_CFG_PULL_UP}
#else
	{TLMM_PULL_SDC3_CMD, GPIO_CFG_PULL_DOWN},
	{TLMM_PULL_SDC3_DATA, GPIO_CFG_PULL_DOWN}
#endif
};

static struct msm_mmc_pad_pull_data mmc_pad_pull_data[MAX_SDCC_CONTROLLER] = {
	[SDCC1] = {
		.on = sdc1_pad_pull_on_cfg,
		.off = sdc1_pad_pull_off_cfg,
		.size = ARRAY_SIZE(sdc1_pad_pull_on_cfg)
	},
	[SDCC3] = {
		.on = sdc3_pad_pull_on_cfg,
		.off = sdc3_pad_pull_off_cfg,
		.size = ARRAY_SIZE(sdc3_pad_pull_on_cfg)
	},
};

static struct msm_mmc_pad_drv_data mmc_pad_drv_data[MAX_SDCC_CONTROLLER] = {
	[SDCC1] = {
		.on = sdc1_pad_drv_on_cfg,
		.off = sdc1_pad_drv_off_cfg,
		.size = ARRAY_SIZE(sdc1_pad_drv_on_cfg)
	},
	[SDCC3] = {
		.on = sdc3_pad_drv_on_cfg,
		.off = sdc3_pad_drv_off_cfg,
		.size = ARRAY_SIZE(sdc3_pad_drv_on_cfg)
	},
};

static struct msm_mmc_pad_data mmc_pad_data[MAX_SDCC_CONTROLLER] = {
	[SDCC1] = {
		.pull = &mmc_pad_pull_data[SDCC1],
		.drv = &mmc_pad_drv_data[SDCC1]
	},
	[SDCC3] = {
		.pull = &mmc_pad_pull_data[SDCC3],
		.drv = &mmc_pad_drv_data[SDCC3]
	},
};

static struct msm_mmc_gpio sdc2_gpio[] = {
	{59, "sdc2_clk"},
	{57, "sdc2_cmd"},
	{62, "sdc2_dat_0"},
	{61, "sdc2_dat_1"},
	{60, "sdc2_dat_2"},
	{58, "sdc2_dat_3"},
};

static struct msm_mmc_gpio sdc4_gpio[] = {
	{68, "sdc4_clk"},
	{67, "sdc4_cmd"},
	{66, "sdc4_dat_0"},
	{65, "sdc4_dat_1"},
	{64, "sdc4_dat_2"},
	{63, "sdc4_dat_3"},
};

static struct msm_mmc_gpio_data mmc_gpio_data[MAX_SDCC_CONTROLLER] = {
	[SDCC2] = {
		.gpio = sdc2_gpio,
		.size = ARRAY_SIZE(sdc2_gpio),
	},
	[SDCC4] = {
		.gpio = sdc4_gpio,
		.size = ARRAY_SIZE(sdc4_gpio),
	}
};

static struct msm_mmc_pin_data mmc_slot_pin_data[MAX_SDCC_CONTROLLER] = {
	[SDCC1] = {
		.pad_data = &mmc_pad_data[SDCC1],
	},
	[SDCC2] = {
		.is_gpio = 1,
		.gpio_data = &mmc_gpio_data[SDCC2],
	},
	[SDCC3] = {
		.pad_data = &mmc_pad_data[SDCC3],
	},
	[SDCC4] = {
		.is_gpio = 1,
		.gpio_data = &mmc_gpio_data[SDCC4],
	},
};

#define MSM_MPM_PIN_SDC1_DAT1	17
#define MSM_MPM_PIN_SDC3_DAT1	21

#ifdef CONFIG_MMC_MSM_SDC1_SUPPORT
static unsigned int sdc1_sup_clk_rates[] = {
	400000, 24000000, 48000000, 96000000
};

static struct mmc_platform_data sdc1_data = {
	.ocr_mask       = MMC_VDD_27_28 | MMC_VDD_28_29,
#ifdef CONFIG_MMC_MSM_SDC1_8_BIT_SUPPORT
	.mmc_bus_width  = MMC_CAP_8_BIT_DATA,
#else
	.mmc_bus_width  = MMC_CAP_4_BIT_DATA,
#endif
	.sup_clk_table	= sdc1_sup_clk_rates,
	.sup_clk_cnt	= ARRAY_SIZE(sdc1_sup_clk_rates),
	.nonremovable	= 1,
	.pin_data	= &mmc_slot_pin_data[SDCC1],
	.vreg_data	= &mmc_slot_vreg_data[SDCC1],
	.uhs_caps	= MMC_CAP_1_8V_DDR | MMC_CAP_UHS_DDR50,
	.uhs_caps2	= MMC_CAP2_HS200_1_8V_SDR,
	.mpm_sdiowakeup_int = MSM_MPM_PIN_SDC1_DAT1,
	.msm_bus_voting_data = &sps_to_ddr_bus_voting_data,
};
static struct mmc_platform_data *apq8064_sdc1_pdata = &sdc1_data;
#else
static struct mmc_platform_data *apq8064_sdc1_pdata;
#endif

#ifdef CONFIG_MMC_MSM_SDC2_SUPPORT
static unsigned int sdc2_sup_clk_rates[] = {
	400000, 24000000, 48000000
};

static struct mmc_platform_data sdc2_data = {
	.ocr_mask       = MMC_VDD_27_28 | MMC_VDD_28_29,
	.mmc_bus_width  = MMC_CAP_4_BIT_DATA,
	.sup_clk_table	= sdc2_sup_clk_rates,
	.sup_clk_cnt	= ARRAY_SIZE(sdc2_sup_clk_rates),
	.pin_data	= &mmc_slot_pin_data[SDCC2],
	.sdiowakeup_irq = MSM_GPIO_TO_INT(61),
	.msm_bus_voting_data = &sps_to_ddr_bus_voting_data,
};
static struct mmc_platform_data *apq8064_sdc2_pdata = &sdc2_data;
#else
static struct mmc_platform_data *apq8064_sdc2_pdata;
#endif

#ifdef CONFIG_MMC_MSM_SDC3_SUPPORT
static unsigned int sdc3_sup_clk_rates[] = {
	400000, 24000000, 48000000, 96000000, 192000000
};

static struct mmc_platform_data sdc3_data = {
	.ocr_mask       = MMC_VDD_27_28 | MMC_VDD_28_29,
	.mmc_bus_width  = MMC_CAP_4_BIT_DATA,
	.sup_clk_table	= sdc3_sup_clk_rates,
	.sup_clk_cnt	= ARRAY_SIZE(sdc3_sup_clk_rates),
	.pin_data	= &mmc_slot_pin_data[SDCC3],
	.vreg_data	= &mmc_slot_vreg_data[SDCC3],
	.wpswitch_gpio	= PM8921_GPIO_PM_TO_SYS(17),
	.is_wpswitch_active_low = true,
	.status_gpio	= 26,
	.status_irq	= MSM_GPIO_TO_INT(26),
	.irq_flags	= IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING,
	.is_status_gpio_active_low = 1,
	.xpc_cap	= 1,
	.uhs_caps	= (MMC_CAP_UHS_SDR12 | MMC_CAP_UHS_SDR25 |
			MMC_CAP_UHS_SDR50 | MMC_CAP_UHS_DDR50 |
			MMC_CAP_UHS_SDR104 | MMC_CAP_MAX_CURRENT_800),
	.mpm_sdiowakeup_int = MSM_MPM_PIN_SDC3_DAT1,
	.msm_bus_voting_data = &sps_to_ddr_bus_voting_data,
};
static struct mmc_platform_data *apq8064_sdc3_pdata = &sdc3_data;
#else
static struct mmc_platform_data *apq8064_sdc3_pdata;
#endif


#ifdef CONFIG_MMC_MSM_SDC4_SUPPORT
static unsigned int sdc4_sup_clk_rates[] = {
	400000, 24000000, 48000000
};

static unsigned int g_wifi_detect;
static void *sdc4_dev;
void (*sdc4_status_cb)(int card_present, void *dev);
#if defined(CONFIG_LGE_BCM433X_PATCH) && !defined(CONFIG_BCMDHD_MODULE)
extern void
msmsdcc_set_mmc_enable(int card_present, void *dev_id);
#endif

int sdc4_status_register(void (*cb)(int card_present, void *dev), void *dev)
{
	if(sdc4_status_cb) {
		return -EINVAL;
	}
	sdc4_status_cb = cb;
	sdc4_dev = dev;
	return 0;
}

unsigned int sdc4_status(struct device *dev)
{
	return g_wifi_detect;
}

static struct mmc_platform_data sdc4_data = {
	.ocr_mask       = MMC_VDD_165_195 | MMC_VDD_27_28 | MMC_VDD_28_29,
	.mmc_bus_width  = MMC_CAP_4_BIT_DATA,
	.sup_clk_table	= sdc4_sup_clk_rates,
	.sup_clk_cnt	= ARRAY_SIZE(sdc4_sup_clk_rates),
	.pin_data	= &mmc_slot_pin_data[SDCC4],
#ifndef CONFIG_BCMDHD_MODULE
	.nonremovable   =  1,
#endif
	//                                                     
	//                                                   
	.status         = sdc4_status,
	.register_status_notify = sdc4_status_register,
};
static struct mmc_platform_data *apq8064_sdc4_pdata = &sdc4_data;
//             
#ifdef CONFIG_BROADCOM_WIFI_RESERVED_MEM

#define WLAN_STATIC_SCAN_BUF0		5
#define WLAN_STATIC_SCAN_BUF1		6
#define PREALLOC_WLAN_SEC_NUM		4
#define PREALLOC_WLAN_BUF_NUM		160
#define PREALLOC_WLAN_SECTION_HEADER	24

#define WLAN_SECTION_SIZE_0	(PREALLOC_WLAN_BUF_NUM * 128)
#define WLAN_SECTION_SIZE_1	(PREALLOC_WLAN_BUF_NUM * 128)
#define WLAN_SECTION_SIZE_2	(PREALLOC_WLAN_BUF_NUM * 512)
#define WLAN_SECTION_SIZE_3	(PREALLOC_WLAN_BUF_NUM * 1024)

#define DHD_SKB_HDRSIZE			336
#define DHD_SKB_1PAGE_BUFSIZE	((PAGE_SIZE*1)-DHD_SKB_HDRSIZE)
#define DHD_SKB_2PAGE_BUFSIZE	((PAGE_SIZE*2)-DHD_SKB_HDRSIZE)
#define DHD_SKB_4PAGE_BUFSIZE	((PAGE_SIZE*4)-DHD_SKB_HDRSIZE)

#define WLAN_SKB_BUF_NUM	17

static struct sk_buff *wlan_static_skb[WLAN_SKB_BUF_NUM];

struct wlan_mem_prealloc {
	void *mem_ptr;
	unsigned long size;
};

static struct wlan_mem_prealloc wlan_mem_array[PREALLOC_WLAN_SEC_NUM] = {
	{NULL, (WLAN_SECTION_SIZE_0 + PREALLOC_WLAN_SECTION_HEADER)},
	{NULL, (WLAN_SECTION_SIZE_1 + PREALLOC_WLAN_SECTION_HEADER)},
	{NULL, (WLAN_SECTION_SIZE_2 + PREALLOC_WLAN_SECTION_HEADER)},
	{NULL, (WLAN_SECTION_SIZE_3 + PREALLOC_WLAN_SECTION_HEADER)}
};

void *wlan_static_scan_buf0;
void *wlan_static_scan_buf1;
static void *brcm_wlan_mem_prealloc(int section, unsigned long size)
{
	if (section == PREALLOC_WLAN_SEC_NUM)
		return wlan_static_skb;
	if (section == WLAN_STATIC_SCAN_BUF0)
		return wlan_static_scan_buf0;
	if (section == WLAN_STATIC_SCAN_BUF1)
		return wlan_static_scan_buf1;
	if ((section < 0) || (section > PREALLOC_WLAN_SEC_NUM))
		return NULL;

	if (wlan_mem_array[section].size < size)
		return NULL;

	return wlan_mem_array[section].mem_ptr;
}

static int brcm_init_wlan_mem(void)
{
	int i;
	int j;

	for (i = 0; i < 8; i++) {
		wlan_static_skb[i] = dev_alloc_skb(DHD_SKB_1PAGE_BUFSIZE);
		if (!wlan_static_skb[i])
			goto err_skb_alloc;
	}

	for (; i < 16; i++) {
		wlan_static_skb[i] = dev_alloc_skb(DHD_SKB_2PAGE_BUFSIZE);
		if (!wlan_static_skb[i])
			goto err_skb_alloc;
	}

	wlan_static_skb[i] = dev_alloc_skb(DHD_SKB_4PAGE_BUFSIZE);
	if (!wlan_static_skb[i])
		goto err_skb_alloc;

	for (i = 0 ; i < PREALLOC_WLAN_SEC_NUM ; i++) {
		wlan_mem_array[i].mem_ptr =
				kmalloc(wlan_mem_array[i].size, GFP_KERNEL);

		if (!wlan_mem_array[i].mem_ptr)
			goto err_mem_alloc;
	}
	wlan_static_scan_buf0 = kmalloc (65536, GFP_KERNEL);
	if(!wlan_static_scan_buf0)
		goto err_mem_alloc;
	wlan_static_scan_buf1 = kmalloc (65536, GFP_KERNEL);
	if(!wlan_static_scan_buf1)
		goto err_mem_alloc;

	printk("%s: WIFI MEM Allocated\n", __FUNCTION__);
	return 0;

 err_mem_alloc:
	pr_err("Failed to mem_alloc for WLAN\n");
	for (j = 0 ; j < i ; j++)
		kfree(wlan_mem_array[j].mem_ptr);

	i = WLAN_SKB_BUF_NUM;

 err_skb_alloc:
	pr_err("Failed to skb_alloc for WLAN\n");
	for (j = 0 ; j < i ; j++)
		dev_kfree_skb(wlan_static_skb[j]);

	return -ENOMEM;
}
#endif /*                                   */

#define WLAN_POWER    PM8921_GPIO_PM_TO_SYS(CONFIG_BCMDHD_GPIO_WL_RESET) //             
#define WLAN_HOSTWAKE CONFIG_BCMDHD_GPIO_WL_HOSTWAKEUP
static unsigned wlan_wakes_msm[] = {
	GPIO_CFG(WLAN_HOSTWAKE, 0, GPIO_CFG_INPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA) };

/*                       */
/*
                                          
                                                                              
*/

int bcm_wifi_set_power(int enable)
{
	int ret = 0;
	if (enable)
	{
#if defined(CONFIG_LGE_BCM433X_PATCH) && !defined(CONFIG_BCMDHD_MODULE)
		msmsdcc_set_mmc_enable(enable,sdc4_dev);
#endif
		ret = gpio_direction_output(WLAN_POWER, 1); 
		if (ret) 
		{
			printk(KERN_ERR "%s: WL_REG_ON  failed to pull up (%d)\n",
					__func__, ret);
			return -EIO;
		}

		//                   
		mdelay(150); //                     
		printk("J:%s: applied delay. 150ms\n",__func__);
		printk(KERN_ERR "%s: wifi power successed to pull up\n",__func__);

	}
	else{
		ret = gpio_direction_output(WLAN_POWER, 0); 
		if (ret) 
		{
			printk(KERN_ERR "%s:  WL_REG_ON  failed to pull down (%d)\n",
					__func__, ret);
			return -EIO;
		}

		//                
		//                                    
#if defined(CONFIG_LGE_BCM433X_PATCH) && !defined(CONFIG_BCMDHD_MODULE)
		msmsdcc_set_mmc_enable(enable,sdc4_dev);
#endif
		printk(KERN_ERR "%s: wifi power successed to pull down\n",__func__);
	}

	return ret;
}

#define LGE_BCM_WIFI_DMA_QOS_CONTROL
#ifdef LGE_BCM_WIFI_DMA_QOS_CONTROL
static int wifi_dma_state; /*                                       */
static struct pm_qos_request wifi_dma_qos;
static struct delayed_work req_dma_work;
static uint32_t packet_transfer_cnt;

static void bcm_wifi_req_dma_work(struct work_struct *work)
{
	switch (wifi_dma_state) {
		case 2: /*            */
			if (packet_transfer_cnt < 100) {
				/*              */
				wifi_dma_state = 1;
				/*                                                                                              */
			} else {
				/*                */
				wifi_dma_state = 3;
				pm_qos_update_request(&wifi_dma_qos, 7);
				schedule_delayed_work(&req_dma_work, msecs_to_jiffies(50));
				/*                                                                                                */
			}
			break;

		case 3: /*              */
			if (packet_transfer_cnt < 10) {
				/*                */
				wifi_dma_state = 2;
				pm_qos_update_request(&wifi_dma_qos, PM_QOS_DEFAULT_VALUE);
				schedule_delayed_work(&req_dma_work, msecs_to_jiffies(1000));
				/*                                                                                                */
			} else {
				/*             */
				schedule_delayed_work(&req_dma_work, msecs_to_jiffies(50));
				/*                                                                                                   */
			}
			break;

		default:
			break;
	}

	packet_transfer_cnt = 0;
}

void bcm_wifi_req_dma_qos(int vote)
{
	if (vote)
		packet_transfer_cnt++;

	/*              */
	if (wifi_dma_state == 1 && vote) {
		wifi_dma_state = 2; /*      */
		schedule_delayed_work(&req_dma_work, msecs_to_jiffies(1000));
		/*                                                                  */
	}
}
#endif

int __init bcm_wifi_init_gpio_mem(void)
{
	int rc=0;
/*
                                                                
                                                                    
*/

#ifdef LGE_BCM_WIFI_DMA_QOS_CONTROL
	INIT_DELAYED_WORK(&req_dma_work, bcm_wifi_req_dma_work);
	pm_qos_add_request(&wifi_dma_qos, PM_QOS_CPU_DMA_LATENCY, PM_QOS_DEFAULT_VALUE);
	wifi_dma_state = 1; /*      */
	printk("%s: wifi_dma_qos is added\n", __func__);
#endif

	if (gpio_request(WLAN_POWER, "WL_REG_ON"))		
		printk("Failed to request gpio %d for WL_REG_ON\n", WLAN_POWER);	

	if (gpio_direction_output(WLAN_POWER, 0)) 
		printk(KERN_ERR "%s: WL_REG_ON  failed to pull down \n", __func__);
	//                      

	if (gpio_request(WLAN_HOSTWAKE, "wlan_wakes_msm"))		
		printk("Failed to request gpio %d for wlan_wakes_msm\n", WLAN_HOSTWAKE);			

	rc = gpio_tlmm_config(wlan_wakes_msm[0], GPIO_CFG_ENABLE);	
	if (rc)		
		printk(KERN_ERR "%s: Failed to configure wlan_wakes_msm = %d\n",__func__, rc);

	//                          

#ifdef CONFIG_BROADCOM_WIFI_RESERVED_MEM
	brcm_init_wlan_mem();
#endif

	printk("bcm_wifi_init_gpio_mem successfully \n");

	return 0;
}

static int bcm_wifi_reset(int on)
{
	return 0;
}

static int bcm_wifi_carddetect(int val)
{
	g_wifi_detect = val;
	if(sdc4_status_cb)
		sdc4_status_cb(val, sdc4_dev);
	else
		printk("%s:There is no callback for notify\n", __FUNCTION__);
	return 0;
}

static int bcm_wifi_get_mac_addr(unsigned char* buf)
{
	uint rand_mac;
	static unsigned char mymac[6] = {0,};
	const unsigned char nullmac[6] = {0,};
	pr_debug("%s: %p\n", __func__, buf);

	printk("[%s] Entering...in Board-l1-mmc.c\n", __func__  );

	if( buf == NULL ) return -EAGAIN;

	if( memcmp( mymac, nullmac, 6 ) != 0 )
	{
		/*                                          
                                              */
		memcpy( buf, mymac, 6 );
		return 0;
	}

	srandom32((uint)jiffies);
	rand_mac = random32();
	buf[0] = 0x00;
	buf[1] = 0x90;
	buf[2] = 0x4c;
	buf[3] = (unsigned char)rand_mac;
	buf[4] = (unsigned char)(rand_mac >> 8);
	buf[5] = (unsigned char)(rand_mac >> 16);

	memcpy( mymac, buf, 6 );

	printk("[%s] Exiting. MyMac :  %x : %x : %x : %x : %x : %x \n",__func__ , buf[0], buf[1], buf[2], buf[3], buf[4], buf[5] );

	return 0;
}

#define COUNTRY_BUF_SZ	4
struct cntry_locales_custom {
	char iso_abbrev[COUNTRY_BUF_SZ];
	char custom_locale[COUNTRY_BUF_SZ];
	int custom_locale_rev;
};

/*                         */
const struct cntry_locales_custom bcm_wifi_translate_custom_table[] = {
/*                                                                            */
           {"",       "GB",     0},
           {"AD",    "GB",     0},
           {"AE",    "KR",     24},
           {"AF",    "AF",     0},
           {"AG",    "US",     100},
           {"AI",     "US",     100},
           {"AL",    "GB",     0},
           {"AM",   "IL",      10},
           {"AN",   "BR",     0},
           {"AO",   "IL",      10},
           {"AR",    "BR",     0},
           {"AS",    "US",     100},
           {"AT",    "GB",     0},
           {"AU",    "AU",    2},
           {"AW",   "KR",     24},
           {"AZ",    "BR",     0},
           {"BA",    "GB",     0},
           {"BB",    "RU",     1},
           {"BD",    "CN",    0},
           {"BE",    "GB",     0},
           {"BF",    "CN",    0},
           {"BG",    "GB",     0},
           {"BH",    "RU",     1},
           {"BI",     "IL",      10},
           {"BJ",     "IL",      10},
           {"BM",   "US",     100},
           {"BN",    "RU",     1},
           {"BO",    "IL",      10},
           {"BR",    "BR",     0},
           {"BS",    "RU",     1},
           {"BT",    "IL",      10},
           {"BW",   "GB",     0},
           {"BY",    "GB",     0},
           {"BZ",    "IL",      10},
           {"CA",    "US",     100},
           {"CD",    "IL",      10},
           {"CF",    "IL",      10},
           {"CG",    "IL",      10},
           {"CH",    "GB",     0},
           {"CI",     "IL",      10},
           {"CK",    "BR",     0},
           {"CL",    "RU",     1},
           {"CM",   "IL",      10},
           {"CN",    "CN",    0},
           {"CO",    "BR",     0},
           {"CR",    "BR",     0},
           {"CU",    "BR",     0},
           {"CV",    "GB",     0},
           {"CX",    "AU",    2},
           {"CY",    "GB",     0},
           {"CZ",    "GB",     0},
           {"DE",    "GB",     0},
           {"DJ",    "IL",      10},
           {"DK",    "GB",     0},
           {"DM",   "BR",     0},
           {"DO",   "BR",     0},
           {"DZ",    "KW",    1},
           {"EC",    "BR",     0},
           {"EE",    "GB",     0},
           {"EG",    "RU",     1},
           {"ER",    "IL",      10},
           {"ES",    "GB",     0},
           {"ET",    "GB",     0},
           {"FI",     "GB",     0},
           {"FJ",     "IL",      10},
           {"FM",   "US",     100},
           {"FO",    "GB",     0},
           {"FR",    "GB",     0},
           {"GA",    "IL",      10},
           {"GB",    "GB",     0},
           {"GD",    "BR",     0},
           {"GE",    "GB",     0},
           {"GF",    "GB",     0},
           {"GH",    "BR",     0},
           {"GI",     "GB",     0},
           {"GM",   "IL",      10},
           {"GN",   "IL",      10},
           {"GP",    "GB",     0},
           {"GQ",   "IL",      10},
           {"GR",    "GB",     0},
           {"GT",    "RU",     1},
           {"GU",    "US",     100},
           {"GW",   "IL",      10},
           {"GY",    "QA",    0},
           {"HK",    "BR",     0},
           {"HN",   "CN",    0},
           {"HR",    "GB",     0},
           {"HT",    "RU",     1},
           {"HU",    "GB",     0},
           {"ID",     "QA",    0},
           {"IE",     "GB",     0},
           {"IL",     "IL",      10},
           {"IM",    "GB",     0},
           {"IN",    "RU",     1},
           {"IQ",    "IL",      10},
           {"IR",     "IL",      10},
           {"IS",     "GB",     0},
           {"IT",     "GB",     0},
           {"JE",     "GB",     0},
           {"JM",    "GB",     0},
           {"JP",     "JP",      5},
           {"KE",    "GB",     0},
           {"KG",    "IL",      10},
           {"KH",    "BR",     0},
           {"KI",     "AU",    2},
           {"KM",   "IL",      10},
           {"KP",    "IL",      10},
           {"KR",    "KR",     24},
           {"KW",   "KW",    1},
           {"KY",    "US",     100},
           {"KZ",    "BR",     0},
           {"LA",    "KR",     24},
           {"LB",    "BR",     0},
           {"LC",    "BR",     0},
           {"LI",     "GB",     0},
           {"LK",    "BR",     0},
           {"LR",    "BR",     0},
           {"LS",     "GB",     0},
           {"LT",     "GB",     0},
           {"LU",    "GB",     0},
           {"LV",     "GB",     0},
           {"LY",     "IL",      10},
           {"MA",   "KW",    1},
           {"MC",   "GB",     0},
           {"MD",   "GB",     0},
           {"ME",   "GB",     0},
           {"MF",   "GB",     0},
           {"MG",   "IL",      10},
           {"MH",   "BR",     0},
           {"MK",   "GB",     0},
           {"ML",    "IL",      10},
           {"MM",  "IL",      10},
           {"MN",   "IL",      10},
           {"MO",   "CN",    0},
           {"MP",   "US",     100},
           {"MQ",   "GB",     0},
           {"MR",   "GB",     0},
           {"MS",   "GB",     0},
           {"MT",   "GB",     0},
           {"MU",   "GB",     0},
           {"MD",   "GB",     0},
           {"ME",   "GB",     0},
           {"MF",   "GB",     0},
           {"MG",   "IL",      10},
           {"MH",   "BR",     0},
           {"MK",   "GB",     0},
           {"ML",    "IL",      10},
           {"MM",  "IL",      10},
           {"MN",   "IL",      10},
           {"MO",   "CN",    0},
           {"MP",   "US",     100},
           {"MQ",   "GB",     0},
           {"MR",   "GB",     0},
           {"MS",   "GB",     0},
           {"MT",   "GB",     0},
           {"MU",   "GB",     0},
           {"MV",   "RU",     1},
           {"MW",  "CN",    0},
           {"MX",   "RU",     1},
           {"MY",   "RU",     1},
           {"MZ",   "BR",     0},
           {"NA",   "BR",     0},
           {"NC",    "IL",      10},
           {"NE",    "BR",     0},
           {"NF",    "BR",     0},
           {"NG",   "NG",    0},
           {"NI",    "BR",     0},
           {"NL",    "GB",     0},
           {"NO",   "GB",     0},
           {"NP",    "SA",     0},
           {"NR",    "IL",      10},
           {"NU",   "BR",     0},
           {"NZ",    "BR",     0},
           {"OM",   "GB",     0},
           {"PA",    "RU",     1},
           {"PE",    "BR",     0},
           {"PF",    "GB",     0},
           {"PH",    "BR",     0},
           {"PK",    "CN",    0},
           {"PL",     "GB",     0},
           {"PM",   "GB",     0},
           {"PN",    "GB",     0},
           {"PR",    "US",     100},
           {"PS",    "BR",     0},
           {"PT",    "GB",     0},
           {"PW",   "BR",     0},
           {"PY",    "BR",     0},
           {"QA",   "CN",    0},
           {"RE",    "GB",     0},
           {"RKS",   "IL",     10},
           {"RO",    "GB",     0},
           {"RS",    "GB",     0},
           {"RU",    "RU",     10},
           {"RW",   "CN",    0},
           {"SA",    "SA",     0},
           {"SB",    "IL",      10},
           {"SC",    "IL",      10},
           {"SD",    "GB",     0},
           {"SE",    "GB",     0},
           {"SG",    "BR",     0},
           {"SI",     "GB",     0},
           {"SK",    "GB",     0},
           {"SKN",  "CN",   0},
           {"SL",     "IL",      10},
           {"SM",   "GB",     0},
           {"SN",    "GB",     0},
           {"SO",    "IL",      10},
           {"SR",    "IL",      10},
           {"SS",    "GB",     0},
           {"ST",    "IL",      10},
           {"SV",    "RU",     1},
           {"SY",    "BR",     0},
           {"SZ",    "IL",      10},
           {"TC",    "GB",     0},
           {"TD",    "IL",      10},
           {"TF",    "GB",     0},
           {"TG",    "IL",      10},
           {"TH",    "BR",     0},
           {"TJ",     "IL",      10},
           {"TL",     "BR",     0},
           {"TM",   "IL",      10},
           {"TN",    "KW",    1},
           {"TO",    "IL",      10},
           {"TR",    "GB",     0},
           {"TT",    "BR",     0},
           {"TV",    "IL",      10},
           {"TW",   "TW",    2},
           {"TZ",    "CN",    0},
           {"UA",    "RU",     1},
           {"UG",    "BR",     0},
           {"US",    "US",     100},
           {"UY",    "BR",     0},
           {"UZ",    "IL",      10},
           {"VA",    "GB",     0},
           {"VC",    "BR",     0},
           {"VE",    "RU",     1},
           {"VG",    "GB",     0},
           {"VI",     "US",     100},
           {"VN",    "BR",     0},
           {"VU",    "IL",      10},
           {"WS",   "SA",     0},
           {"YE",    "IL",      10},
           {"YT",    "GB",     0},
           {"ZA",    "GB",     0},
           {"ZM",   "RU",     1},
           {"ZW",   "BR",     0},
};

static void *bcm_wifi_get_country_code(char *ccode)
{
	int size, i;
	static struct cntry_locales_custom country_code;

	size = ARRAY_SIZE(bcm_wifi_translate_custom_table);

	if ((size == 0) || (ccode == NULL))
		return NULL;

	for (i = 0; i < size; i++) {
		if (strcmp(ccode, bcm_wifi_translate_custom_table[i].iso_abbrev) == 0)
			return (void *)&bcm_wifi_translate_custom_table[i];
	}

	memset(&country_code, 0, sizeof(struct cntry_locales_custom));
	strlcpy(country_code.custom_locale, ccode, COUNTRY_BUF_SZ);

	return (void *)&country_code;
}

static struct wifi_platform_data bcm_wifi_control = {
#ifdef CONFIG_BROADCOM_WIFI_RESERVED_MEM
	.mem_prealloc	= brcm_wlan_mem_prealloc,
#endif /*                                   */
	.set_power	= bcm_wifi_set_power,
	.set_reset      = bcm_wifi_reset,
	.set_carddetect = bcm_wifi_carddetect,
	.get_mac_addr   = bcm_wifi_get_mac_addr, //                       
	.get_country_code = bcm_wifi_get_country_code,
};

static struct resource wifi_resource[] = {
	[0] = {
		.name = "bcmdhd_wlan_irq",
		.start = MSM_GPIO_TO_INT(WLAN_HOSTWAKE),
		.end   = MSM_GPIO_TO_INT(WLAN_HOSTWAKE),
#ifdef CONFIG_BCMDHD_HW_OOB
		.flags = IORESOURCE_IRQ | IORESOURCE_IRQ_HIGHLEVEL | IORESOURCE_IRQ_SHAREABLE, //           
#else
		.flags = IORESOURCE_IRQ | IORESOURCE_IRQ_HIGHEDGE | IORESOURCE_IRQ_SHAREABLE, //               
#endif
		//                                                                                                                        
	},
};

static struct platform_device bcm_wifi_device = {
	.name           = "bcmdhd_wlan",
	.id             = 1,
	.num_resources  = ARRAY_SIZE(wifi_resource),
	.resource       = wifi_resource,
	.dev            = {
		.platform_data = &bcm_wifi_control,
	},
};
#else
static struct mmc_platform_data *apq8064_sdc4_pdata;
#endif

void __init apq8064_init_mmc(void)
{
	if ((machine_is_apq8064_rumi3()) || machine_is_apq8064_sim()) {
		if (apq8064_sdc1_pdata) {
			if (machine_is_apq8064_sim())
				apq8064_sdc1_pdata->disable_bam = true;
			apq8064_sdc1_pdata->disable_runtime_pm = true;
			apq8064_sdc1_pdata->disable_cmd23 = true;
		}
		if (apq8064_sdc3_pdata) {
			if (machine_is_apq8064_sim())
				apq8064_sdc3_pdata->disable_bam = true;
			apq8064_sdc3_pdata->disable_runtime_pm = true;
			apq8064_sdc3_pdata->disable_cmd23 = true;
		}
	}

	if (apq8064_sdc1_pdata)
		apq8064_add_sdcc(1, apq8064_sdc1_pdata);
	}

	msm_add_uio();

	if (apq8064_sdc2_pdata)
		apq8064_add_sdcc(2, apq8064_sdc2_pdata);

	if (apq8064_sdc3_pdata) {
		if (!machine_is_apq8064_cdp()) {
			apq8064_sdc3_pdata->wpswitch_gpio = 0;
#if defined(CONFIG_MACH_LGE)
			//                                                   
#endif
		}
		if (machine_is_mpq8064_cdp() || machine_is_mpq8064_hrd() ||
			machine_is_mpq8064_dtv()) {
			int rc;
			struct pm_gpio sd_card_det_init_cfg = {
				.direction      = PM_GPIO_DIR_IN,
				.output_buffer  = PM_GPIO_OUT_BUF_CMOS,
				.pull           = PM_GPIO_PULL_UP_30,
				.vin_sel        = PM_GPIO_VIN_S4,
				.out_strength   = PM_GPIO_STRENGTH_NO,
				.function       = PM_GPIO_FUNC_NORMAL,
			};

			apq8064_sdc3_pdata->status_gpio =
				PM8921_GPIO_PM_TO_SYS(31);
			apq8064_sdc3_pdata->status_irq =
				PM8921_GPIO_IRQ(PM8921_IRQ_BASE, 31);
			rc = pm8xxx_gpio_config(apq8064_sdc3_pdata->status_gpio,
					&sd_card_det_init_cfg);
			if (rc) {
				pr_info("%s: SD_CARD_DET GPIO%d config "
					"failed(%d)\n", __func__,
					apq8064_sdc3_pdata->status_gpio, rc);
				apq8064_sdc3_pdata->status_gpio = 0;
				apq8064_sdc3_pdata->status_irq = 0;
			}
		}
		if (machine_is_apq8064_cdp()) {
			int i;

			for (i = 0;
			     i < apq8064_sdc3_pdata->pin_data->pad_data->\
				 drv->size;
			     i++)
				apq8064_sdc3_pdata->pin_data->pad_data->\
					drv->on[i].val = GPIO_CFG_10MA;
		}
		apq8064_add_sdcc(3, apq8064_sdc3_pdata);
	}

	if (apq8064_sdc4_pdata) {
#ifdef CONFIG_MMC_MSM_SDC4_SUPPORT
		bcm_wifi_init_gpio_mem();
		platform_device_register(&bcm_wifi_device);
#endif /*                             */
		apq8064_add_sdcc(4, apq8064_sdc4_pdata);
	}

}
