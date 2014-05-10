/*
 *  MAX17043_fuelgauge.c
 *  fuel-gauge systems for lithium-ion (Li+) batteries
 *
 *  Copyright (C) 2009 LG Electronics
 *  Dajin Kim <dajin.kim@lge.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/module.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/platform_device.h>
#include <linux/mutex.h>
#include <linux/err.h>
#include <linux/i2c.h>
#include <linux/delay.h>
#include <linux/power_supply.h>
#include <linux/max17043_fuelgauge.h>
#include <linux/slab.h>
#include <linux/gpio.h>
#include <linux/delay.h>
#include <linux/io.h>
#include <mach/msm_iomap.h>
#include <linux/mfd/pm8xxx/pm8921-charger.h>

/*                                                                                           */
#include <linux/mfd/pm8xxx/pm8xxx-adc.h>
#if defined (CONFIG_LGE_PM) || defined (CONFIG_LGE_PM_BATTERY_ID_CHECKER)
#include <mach/board_lge.h>
#endif
/*                                   */
#define RCOMP_BL44JN	(0xB8)	/*                                  */
#define RCOMP_BL53QH	(0x44)  /*                                   */
#define MONITOR_LEVEL	(2)

#define MAX17043_VCELL_REG	0x02
#define MAX17043_SOC_REG	0x04
#define MAX17043_MODE_REG	0x06
#define MAX17043_VER_REG	0x08
#define MAX17043_CONFIG_REG	0x0C
#define MAX17043_CMD_REG	0xFE

#define MAX17043_VCELL_MSB	0x02
#define MAX17043_VCELL_LSB	0x03
#define MAX17043_SOC_MSB	0x04
#define MAX17043_SOC_LSB	0x05
#define MAX17043_MODE_MSB	0x06
#define MAX17043_MODE_LSB	0x07
#define MAX17043_VER_MSB	0x08
#define MAX17043_VER_LSB	0x09
#define MAX17043_CONFIG_MSB	0x0C
#define MAX17043_CONFIG_LSB	0x0D
#define MAX17043_CMD_MSB	0xFE
#define MAX17043_CMD_LSB	0xFF

#define MAX17043_WORK_DELAY		(30 * HZ)	/*     */
#define MAX17043_BATTERY_FULL		95		/*              */
#define MAX17043_TOLERANCE		10		/*              */

extern void pm8921_charger_force_update_batt_psy(void);

struct max17043_chip {
	struct i2c_client		*client;
	struct delayed_work		work;
#if 0 /*                             */
	struct work_struct		alert_work;
#endif
	struct power_supply		battery;

	/*                               */
	int vcell;				/*                      */
	int soc;				/*                    */
	int version;			/*                       */
	int config;				/*                          */

	/*                        */
	int voltage;		/*                                           */
	int capacity;		/*                                         */
	max17043_status status;	/*                   */
/*                                                                                           */
	u8			starting_rcomp;
	int			temp_co_hot;
	int			temp_co_cold;
/*                                   */
/*                                                                                          */
	//                                          
/*                                   */
};

#if 0
/*
                           
                                                
                             
 */
struct max17043_calibration_data {
	int voltage;	/*               */
	int capacity;	/*               */
	int gradient;	/*                 */
	int intercept;	/*                  */
};

/*                        */
static struct max17043_calibration_data without_charger[] = {
	{3953,		81,		9,		3242},
	{3800,		58,		7,		3403},
	{3740,		40,		3,		3611},
	{3695,		20,		2,		3650},
	{3601,		4,		6,		3574},
	{3300,		0,		55,		3548},
	{ -1, -1, -1, -1},	/*             */
};
/*                        */
static struct max17043_calibration_data with_charger[] = {
	{3865,		2,		66,		3709},
	{3956,		19,		5,		3851},
	{4021,		46,		2,		3912},
	{4088,		61,		5,		3813},
	{4158,		71,		7,		3689},
	{4200,		100,		2,		4042},
	{ -1, -1, -1, -1},	/*             */
};
#endif

/*                                                                           */
extern int lge_battery_info;
/*                                                                        */
static struct max17043_chip *reference;

#if 0 //                 
/*                                                              */
extern int lge_power_test_flag;
/*                                    */
#endif
int need_to_quickstart;
EXPORT_SYMBOL(need_to_quickstart);
static int max17043_write_data(struct i2c_client *client, int reg, const u8 *values, int length)
{
		int ret;

		ret = i2c_smbus_write_i2c_block_data(client, reg, length, values);

		if (ret < 0)
				dev_err(&client->dev, "%s: err %d\n", __func__, ret);

		return ret;
}

static int max17043_read_data(struct i2c_client *client, int reg, u8 *values, int length)
{
		int ret;

		ret = i2c_smbus_read_i2c_block_data(client, reg, length, values);

		if (ret < 0)
				dev_err(&client->dev, "%s: err %d\n", __func__, ret);

		return ret;
}
static int max17043_write_reg(struct i2c_client *client, int reg, u16 value)
{
	int ret = 0;

	value = ((value & 0xFF00) >> 8) | ((value & 0xFF) << 8);
	ret = i2c_smbus_write_word_data(client, reg, value);

	if (ret < 0)
		dev_err(&client->dev, "%s: err %d\n", __func__, ret);

	return ret;
}
static int max17043_read_reg(struct i2c_client *client, int reg)
{
	int ret = 0;

	ret = i2c_smbus_read_word_data(client, reg);

	if (ret < 0) {
		dev_err(&client->dev, "%s: err %d\n", __func__, ret);
		return ret;
	}

	return ((ret & 0xFF00) >> 8) | ((ret & 0xFF) << 8);
}
static int max17043_reset(struct i2c_client *client)
{
	struct max17043_chip *chip = i2c_get_clientdata(client);
#ifdef CONFIG_BATTERY_MAX17043
	/*                                                              */
	max17043_write_reg(client, MAX17043_MODE_REG, 0x4000);
#else
	max17043_write_reg(client, MAX17043_CMD_REG, 0x5400);
#endif
	chip->status = MAX17043_RESET;

	dev_info(&client->dev, "MAX17043 Fuel-Gauge Reset(quickstart)\n");

	return 0;
}

static int max17043_quickstart(struct i2c_client *client)
{
	struct max17043_chip *chip = i2c_get_clientdata(client);

	max17043_write_reg(client, MAX17043_MODE_REG, 0x4000);

	chip->status = MAX17043_QUICKSTART;

	dev_info(&client->dev, "MAX17043 Fuel-Gauge Quick-Start\n");

	return 0;
}
static int max17043_read_vcell(struct i2c_client *client)
{
	struct max17043_chip *chip = i2c_get_clientdata(client);
	u16 value;

	value = max17043_read_reg(client, MAX17043_VCELL_REG);

	if (value < 0)
		return value;

	chip->vcell = value >> 4;

	return 0;
}
static int max17043_read_soc(struct i2c_client *client)
{
	struct max17043_chip *chip = i2c_get_clientdata(client);
	u16 value;

	value = max17043_read_reg(client, MAX17043_SOC_REG);
	printk(KERN_INFO "%s: max17043_read_reg return with %d\n", __func__, value);
	if (value < 0)
		return value;

	chip->soc = value;

	return 0;
}

static int max17043_read_version(struct i2c_client *client)
{
	struct max17043_chip *chip = i2c_get_clientdata(client);
	u16 value;

	value = max17043_read_reg(client, MAX17043_VER_REG);

	chip->version = value;

	dev_info(&client->dev, "MAX17043 Fuel-Gauge Ver %d\n", value);

	return 0;
}
static int max17043_read_config(struct i2c_client *client)
{
	struct max17043_chip *chip = i2c_get_clientdata(client);
	u16 value;

	value = max17043_read_reg(client, MAX17043_CONFIG_REG);

	if (value < 0)
		return value;

	pr_info("Sleep-%d, ALSC-%d, ALRT-%d, ATHD-%d\n", value&0x80, value&0x40, value&0x20, value&0x1F);
	chip->config = value;

	return 0;
}
static int max17043_write_config(struct i2c_client *client)
{
	struct max17043_chip *chip = i2c_get_clientdata(client);

	max17043_write_reg(client, MAX17043_CONFIG_REG, chip->config);

	return 0;
}

#ifdef CONFIG_BATTERY_MAX17043
/*
                                                                          
 */
#define EMPTY_SOC	0
static int max17043_get_capacity_from_soc(void)
{
	u8 buf[2];
	long batt_soc = 0;
	long adjust_soc = 0;
	
	if (reference == NULL) {
		return 100;
	}

	buf[0] = (reference->soc & 0x0000FF00) >> 8;
	buf[1] = (reference->soc & 0x000000FF);

/*                                                                 */
	batt_soc = ((buf[0]*256)+buf[1])*19531; /*             */
	pr_info("%s: batt_soc is %d(0x%02x:0x%02x):%ld\n", __func__, (int)(batt_soc/10000000), buf[0], buf[1], batt_soc);

	adjust_soc = 9300000;
	batt_soc /= adjust_soc;
	pr_info("adjust_soc = %ld, batt_soc = %ld\n", adjust_soc, batt_soc);

	if(batt_soc < 0)
		batt_soc = 0;

	if(batt_soc > 100)
		batt_soc = 100;

/*                                                               */
	return batt_soc;
}
#endif

static int max17043_need_quickstart(int charging)
{
#if 0
/*                                                              */
#ifdef CONFIG_BATTERY_MAX17043
	struct max17043_ocv_to_soc_data *data;
	int i = 0;
	int expected;
	int diff;
	int ratio_soc = 0;
	int ratio_vol = 0;
#else
	struct max17043_calibration_data *data;
#endif
	int vol;
	int level;

	if (reference == NULL)
		return 0;

	/*                  */
	vol = reference->voltage;
#ifdef CONFIG_BATTERY_MAX17043
	level = max17043_get_capacity_from_soc();

	if(level != 0){
		level = (level * 95) / 100;
	}
#else
	level = reference->soc >> 8;
#endif
	if (level > 100)
		level = 100;
	else if (level < 0)
		level = 0;

#ifdef CONFIG_BATTERY_MAX17043
	printk(KERN_INFO "[Battery] %s : Check SOC.\n",__func__);
#if 0
	data = reference->cal_data;

	if( data == NULL ){
		printk(KERN_INFO "[Battery] %s : NO DATA.\n",__func__);
		return 0;
	}

	while(data[i].voltage != -1){
		if(vol <= data[i].voltage)
			break;
		i++;
	}

	if(data[i].voltage == -1){
		if((level == 100) || (level > data[i-1].soc - MAX17043_TOLERANCE) || (need_to_quickstart > 2)){
			need_to_quickstart = 0;
			return 0;
		}else{
			need_to_quickstart += 1;
			return 1;
		}
	}
	else if(i == 0){
		need_to_quickstart = 0;
		return 0;
	}

	ratio_vol = (vol - data[i-1].voltage) * (data[i].soc - data[i-1].soc);
	ratio_soc = ratio_vol /	((data[i].voltage - data[i-1].voltage));

	expected = ratio_soc + data[i-1].soc;
#endif
#else
	/*                    */
	if (charging) {
		data = with_charger;
		while (data[i].voltage != -1) {
			if (vol <= data[i].voltage)
				break;
			i++;
		}
	} else {
		data = without_charger;
		while (data[i].voltage != -1) {
			if (vol >= data[i].voltage)
				break;
			i++;
		}
	}

	/*                 */
	if (data[i].voltage == -1) {
		if (charging) {
			if (level == 100)
				return 0;
			else
				return 1;
		} else {
			if (level == 0)
				return 0;
			else
				return 1;
		}
	}

	/*                */
	expected = (vol - data[i].intercept) / data[i].gradient;
#endif
/*                          */

	if (expected > 100)
		expected = 100;
	else if (expected < 0)
		expected = 0;
	diff = expected - level;

	printk(KERN_INFO "[Battery] quickstart voltage(%d) base soc : expected(%d)/read(%d)\n",vol,expected,level);

	/*       */
	if (diff < -MAX17043_TOLERANCE || diff > MAX17043_TOLERANCE)
		need_to_quickstart += 1;
	else
		need_to_quickstart = 0;

	/*                                    
                                      
  */
	if (need_to_quickstart > 2)
		need_to_quickstart = 0;
#endif
	//                          
	return 0;
}
static int max17043_set_rcomp(int rcomp)
{
	if (reference == NULL)
		return -1;

	rcomp &= 0xff;
	reference->config = ((reference->config & 0x00ff) | (rcomp << 8));

	max17043_write_config(reference->client);

	return 0;
}
static int max17043_set_athd(int level)
{
	if (reference == NULL)
		return -1;

	if (level > 32)
		level = 32;
	else if (level < 1)
		level = 1;

	level = 32 - level;
	if (level == (reference->config & 0x1F))
		return level;

	reference->config = ((reference->config & 0xffe0) | level);
	max17043_write_config(reference->client);

	return level;
}
static int max17043_clear_interrupt(struct i2c_client *client)
{
	struct max17043_chip *chip = i2c_get_clientdata(client);

	if (chip->config & 0x20) {
		chip->config &= 0xffdf;
		max17043_write_config(chip->client);
	}

	return 0;
}
static int max17043_update(struct i2c_client *client)
{
	struct max17043_chip *chip = i2c_get_clientdata(client);
	int ret;

	printk(KERN_INFO "%s: start\n", __func__);
/*                                                                                           */
	max17043_set_rcomp_by_temperature();
/*                                   */

#if 1 /*          */
{
	u8 org_ocv_msb, org_ocv_lsb;
	u8 values[2];
	int batt_mv;

	/*                     */
	values[0] = 0x4A; values[1] = 0x57;
	max17043_write_data(client, 0x3E, &values[0], 2);

	/*         */
	max17043_read_data(client, 0x0E, &values[0], 2);
	org_ocv_msb = values[0]; org_ocv_lsb = values[1];
	batt_mv = ((values[0] << 4) + (values[1] >> 4));
	batt_mv = (batt_mv*125)/100;
	pr_info("%s : ocv is 0x%02x%02x - %d\n", __func__, org_ocv_msb, org_ocv_lsb, batt_mv);

	/*                  */
	values[0] = 0x00; values[1] = 0x00;
	max17043_write_data(client, 0x3E, &values[0], 2);
}
#endif

	ret = max17043_read_vcell(client);
	if (ret < 0)
		return ret;
	ret = max17043_read_soc(client);
	if (ret < 0)
		return ret;

	printk(KERN_INFO "%s: read reg vcell %d, soc %d\n", __func__, chip->vcell, chip->soc);

	/*                                 */
	chip->voltage = (chip->vcell * 5) >> 2;	/*                 */
#ifdef CONFIG_BATTERY_MAX17043
	printk(KERN_INFO "%s: chip->capacity %d, read from maxim...\n", __func__, chip->capacity);
	chip->capacity = max17043_get_capacity_from_soc();
	printk(KERN_INFO "%s: chip->capacity %d, after read...\n", __func__, chip->capacity);
#else
	chip->capacity = chip->soc >> 8;
#endif

	/*                       */
//                                                                 

	if (chip->capacity > 100)
		chip->capacity = 100;
	else if (chip->capacity < 0)
		chip->capacity = 0;

	chip->status = MAX17043_WORKING;

	return 0;
}
static void max17043_work(struct work_struct *work)
{
	struct max17043_chip *chip;
	int charging = 0, source = 0;

	chip = container_of(work, struct max17043_chip, work.work);

	charging = pm8921_is_battery_charging(&source);
	printk(KERN_INFO "%s: charging %d\n", __func__, charging);
	switch (chip->status) {
	case MAX17043_RESET:
		max17043_read_version(chip->client);
		max17043_read_config(chip->client);
		max17043_set_rcomp(chip->starting_rcomp);
	case MAX17043_QUICKSTART:
		max17043_update(chip->client);
		if (max17043_need_quickstart(charging)) {
			max17043_reset(chip->client);
			schedule_delayed_work(&chip->work,  HZ);
			return;
		}
		max17043_set_athd(MONITOR_LEVEL);
		max17043_clear_interrupt(chip->client);
		need_to_quickstart = 0;
		break;
	case MAX17043_WORKING:
	default:
		max17043_update(chip->client);
		break;
	}
	schedule_delayed_work(&chip->work, MAX17043_WORK_DELAY);

	printk(KERN_INFO "[Battery] max17043_work : soc(%d), volt(%d)\n",
				reference->capacity, reference->voltage);
	printk(KERN_INFO "[Battery] max17043_work : soc(%d), volt(%d)\n",
				chip->capacity, chip->voltage);

}

#if 0 /*                             */
static void max17043_alert_work(struct work_struct *work)
{
	if (reference == NULL)
		return;

	if (reference->status == MAX17043_WORKING) {
		cancel_delayed_work_sync(&reference->work);
		schedule_delayed_work(&reference->work, MAX17043_WORK_DELAY);
	}
	max17043_update(reference->client);

	max17043_read_config(reference->client);
	max17043_clear_interrupt(reference->client);

	if (!pm8921_is_dc_chg_plugged_in() && !pm8921_is_usb_chg_plugged_in())
}
#endif

static irqreturn_t max17043_interrupt_handler(int irq, void *data)
{
#if 0 /*                             */
	if (reference == NULL)
		return IRQ_HANDLED;
	schedule_work(&reference->alert_work);
#endif
	//                                       
	max17043_clear_interrupt(reference->client);
	return IRQ_HANDLED;
}

#if 0	/*                                                       */
/*                                                                     */
static enum power_supply_property max17043_battery_props[] = {
	POWER_SUPPLY_PROP_VOLTAGE_NOW,
	POWER_SUPPLY_PROP_CAPACITY,
};
static int max17043_get_property(struct power_supply *psy,
			    enum power_supply_property psp,
			    union power_supply_propval *val)
{
	struct max17043_chip *chip = container_of(psy,
				struct max17043_chip, battery);

	switch (psp) {
	case POWER_SUPPLY_PROP_VOLTAGE_NOW:
		val->intval = chip->voltage;
		break;
	case POWER_SUPPLY_PROP_CAPACITY:
		val->intval = chip->capacity;
		break;
	default:
		return -EINVAL;
	}
	return 0;
}
/*                                                     */
#endif
/*                                           */

/*                                                              */
void max17043_power_quickstart(void)
{
	int charging = 0;
	int source = 0;

	pr_err("[mansu.lee] %s.\n",__func__);

	charging = pm8921_is_battery_charging(&source);

	max17043_quickstart(reference->client);
	/*                                         */
	msleep(300);

	max17043_update(reference->client);

	do{
		max17043_need_quickstart(charging);
	}while(need_to_quickstart != 0);
}
/*                          */

ssize_t max17043_show_volt(struct device *dev,
			 struct device_attribute *attr,
			 char *buf)
{
	int voltage;
	if (reference == NULL)
		return snprintf(buf, PAGE_SIZE, "ERROR\n");
#ifdef CONFIG_BATTERY_MAX17043
	/*                                                              */
	if(0 /*                        */){
		cancel_delayed_work(&reference->work);

#if 0 //                 
		pm8921_charger_enable(0);
#endif
		pm8921_disable_source_current(1);

		max17043_power_quickstart();
		voltage = ((reference->vcell * 5) >> 2);

		pm8921_disable_source_current(0);
#if 0 //                 
		pm8921_charger_enable(1);
#endif
		schedule_delayed_work(&reference->work, HZ);

		return snprintf(buf, PAGE_SIZE, "%d\n", voltage);
	}
	/*                          */
#endif
	return snprintf(buf, PAGE_SIZE, "%d\n", (reference->vcell * 5) >> 2);
}
DEVICE_ATTR(volt, 0444, max17043_show_volt, NULL);

ssize_t max17043_show_version(struct device *dev,
			 struct device_attribute *attr,
			 char *buf)
{
	if (reference == NULL)
		return snprintf(buf, PAGE_SIZE, "ERROR\n");

	return snprintf(buf, PAGE_SIZE, "%d\n", reference->version);
}
DEVICE_ATTR(version, 0444, max17043_show_version, NULL);

ssize_t max17043_show_config(struct device *dev,
			 struct device_attribute *attr,
			 char *buf)
{
	if (reference == NULL)
		return snprintf(buf, PAGE_SIZE, "ERROR\n");

	return snprintf(buf, PAGE_SIZE, "%d\n", reference->config);
}
DEVICE_ATTR(config, 0444, max17043_show_config, NULL);

ssize_t max17043_show_soc(struct device *dev,
			 struct device_attribute *attr,
			 char *buf)
{
	int level;

	if (reference == NULL)
		return snprintf(buf, PAGE_SIZE, "ERROR\n");
#ifdef CONFIG_BATTERY_MAX17043
	/*                                                              */
	if(0 /*                        */){
		cancel_delayed_work(&reference->work);
#if 0 //                 
		pm8921_charger_enable(0);
#endif
		pm8921_disable_source_current(1);

		max17043_power_quickstart();
		level = max17043_get_capacity_from_soc();

		pm8921_disable_source_current(0);
#if 0 //                 
		pm8921_charger_enable(1);
#endif
		schedule_delayed_work(&reference->work, HZ);

		if(level != 0){
			if(level >= 99)
				level = (level * 915) /1000;
			else
				level = (level * 100) /100;
		}

		if (level > 100)
			level = 100;
		else if (level < 0)
			level = 0;

		return snprintf(buf, PAGE_SIZE, "%d\n", level);
	}
	/*                                                              */
#endif
	/*                                                             */
	/*                                                  */

	level = reference->capacity;
	/*                        */

	return snprintf(buf, PAGE_SIZE, "%d\n", level);
}
DEVICE_ATTR(soc, 0444, max17043_show_soc, NULL);

ssize_t max17043_show_status(struct device *dev,
			 struct device_attribute *attr,
			 char *buf)
{
	if (reference == NULL)
		return snprintf(buf, PAGE_SIZE, "ERROR\n");

	switch (reference->status) {
	case MAX17043_RESET:
		return snprintf(buf, PAGE_SIZE, "reset\n");
	case MAX17043_QUICKSTART:
		return snprintf(buf, PAGE_SIZE, "quickstart\n");
	case MAX17043_WORKING:
		return snprintf(buf, PAGE_SIZE, "working\n");
	default:
		return snprintf(buf, PAGE_SIZE, "ERROR\n");
	}
}
ssize_t max17043_store_status(struct device *dev,
			  struct device_attribute *attr,
			  const char *buf,
			  size_t count)
{
	if (reference == NULL)
		return -1;

	if (strncmp(buf, "reset", 5) == 0) {
		cancel_delayed_work(&reference->work);
		max17043_reset(reference->client);
		schedule_delayed_work(&reference->work, HZ);
	} else if (strncmp(buf, "quickstart", 10) == 0) {
		cancel_delayed_work(&reference->work);
		max17043_quickstart(reference->client);
		schedule_delayed_work(&reference->work, HZ);
	} else if (strncmp(buf, "working", 7) == 0) {
		/*            */
	} else {
		return -1;
	}
	return count;
}
DEVICE_ATTR(state, 0664, max17043_show_status, max17043_store_status);
/*                                         */

/*                                       */
int __max17043_get_capacity(void)
{
	if (reference == NULL)	/*                                   */
		return 100;			/*                    */
	return reference->capacity;
}
EXPORT_SYMBOL(__max17043_get_capacity);
int __max17043_get_voltage(void)
{
	if (reference == NULL)	/*                                   */
		return 4350;		/*                    */
	return reference->voltage;
}
EXPORT_SYMBOL(__max17043_get_voltage);
int max17043_do_calibrate(void)
{
	if (reference == NULL)
		return -1;

	cancel_delayed_work(&reference->work);
	max17043_quickstart(reference->client);
	schedule_delayed_work(&reference->work, HZ);

	return 0;
}
EXPORT_SYMBOL(max17043_do_calibrate);

/*                                                                                           */

void max17040_init_model(struct i2c_client *client)
{
	u8 org_rcomp_msb, org_rcomp_lsb;
	u8 org_ocv_msb, org_ocv_lsb;
	u8 reg;
	u8 values[32];
	int len;
	int ret;
	int batt_mv;

#if 0
  u8 buf[5];
	if (lge_check_battery()) {
		buf[0] = 0x40; buf[1] = 0x00;
		max17043_write_data(max17040_i2c_client, MAX17040_MODE_MSB, &buf[0], 2);
		msleep(300);
	}
#endif
	max17043_read_data(client, MAX17043_VCELL_MSB, &values[0], 2);
	batt_mv = ((values[0] << 4) + (values[1] >> 4));
	batt_mv = (batt_mv*125)/100;
	pr_info("%s : Read VCELL - %d \n", __func__, batt_mv);

	/*                         */
	values[0] = 0x4A; values[1] = 0x57;
	max17043_write_data(client, 0x3E, &values[0], 2);

	/*                                          */
	max17043_read_data(client, MAX17043_CONFIG_REG, &values[0], 2);
	org_rcomp_msb = values[0]; org_rcomp_lsb = values[1];
	max17043_read_data(client, 0x0E, &values[0], 2);
	org_ocv_msb = values[0]; org_ocv_lsb = values[1];
	batt_mv = ((values[0] << 4) + (values[1] >> 4));
	batt_mv = (batt_mv*125)/100;
	pr_info("%s : ocv = 0x%02x%02x - %d\n", __func__, org_ocv_msb, org_ocv_lsb, batt_mv);
	pr_info("%s : rcomp = 0x%02x%02x \n", __func__, org_rcomp_msb, org_rcomp_lsb);

	/*                                              */
	if(org_ocv_msb == 0xFF && org_ocv_lsb == 0xFF){
		values[0] = 0x4A; values[1] = 0x57;
		ret = max17043_write_data(client, 0x3E, &values[0], 2);
		}

	/*                        */
	values[0] = 0xE2; values[1] = 0x60;
	max17043_write_data(client, 0x0E, &values[0], 2);

	/*                          */
	values[0] = 0xFF; values[1] = 0x00;
	max17043_write_data(client, 0x0C, &values[0], 2);

	/*                                           */
	len = 16;
	reg = 0x40;
	values[0]  = 0xAC; values[1]  = 0x70; values[2]  = 0xB6; values[3]	= 0xC0;
	values[4]  = 0xB7; values[5]  = 0xB0; values[6]  = 0xBB; values[7]	= 0x10;
	values[8]  = 0xBB; values[9]  = 0xC0; values[10] = 0xBC; values[11] = 0x30;
	values[12] = 0xBC; values[13] = 0xF0; values[14] = 0xBE; values[15] = 0x90;
	max17043_write_data(client, reg, &values[0], len);

	reg = 0x50;
	values[0]  = 0xBF; values[1]  = 0x90; values[2]  = 0xC2; values[3]	= 0xD0;
	values[4]  = 0xC5; values[5]  = 0x60; values[6]  = 0xC8; values[7]	= 0x70;
	values[8]  = 0xCA; values[9]  = 0xF0; values[10] = 0xCE; values[11] = 0x70;
	values[12] = 0xD1; values[13] = 0xA0; values[14] = 0xD8; values[15] = 0x60;
	max17043_write_data(client, reg, &values[0], len);

	reg = 0x60;
	values[0]  = 0x03; values[1]  = 0x00; values[2]  = 0x2E; values[3]	= 0x40;
	values[4]  = 0x16; values[5]  = 0x00; values[6]  = 0x2E; values[7]	= 0x00;
	values[8]  = 0x50; values[9]  = 0x20; values[10] = 0x62; values[11] = 0x20;
	values[12] = 0x2B; values[13] = 0xE0; values[14] = 0x29; values[15] = 0xA0;
	max17043_write_data(client, reg, &values[0], len);

	reg = 0x70;
	values[0]  = 0x13; values[1]  = 0xE0; values[2]  = 0x13; values[3]	= 0x20;
	values[4]  = 0x12; values[5]  = 0x00; values[6]  = 0x13; values[7]	= 0x20;
	values[8]  = 0x0F; values[9]  = 0x40; values[10] = 0x0D; values[11] = 0xE0;
	values[12] = 0x0C; values[13] = 0xA0; values[14] = 0x0C; values[15] = 0xA0;
	max17043_write_data(client, reg, &values[0], len);

	/*                                                                */
	len = 16;
	reg = 0x80;
	values[0]  = 0x00; values[1]  = 0x80; values[2]  = 0x00; values[3]	= 0x80;
	values[4]  = 0x00; values[5]  = 0x80; values[6]  = 0x00; values[7]	= 0x80;
	values[8]  = 0x00; values[9]  = 0x80; values[10] = 0x00; values[11] = 0x80;
	values[12] = 0x00; values[13] = 0x80; values[14] = 0x00; values[15] = 0x80;
	max17043_write_data(client, reg, &values[0], len);

	reg = 0x90;
	values[0]  = 0x00; values[1]  = 0x80; values[2]  = 0x00; values[3]	= 0x80;
	values[4]  = 0x00; values[5]  = 0x80; values[6]  = 0x00; values[7]	= 0x80;
	values[8]  = 0x00; values[9]  = 0x80; values[10] = 0x00; values[11] = 0x80;
	values[12] = 0x00; values[13] = 0x80; values[14] = 0x00; values[15] = 0x80;
	max17043_write_data(client, reg, &values[0], len);


	/*                          */
	msleep(200);

	/*                        */
	values[0] = 0xE2; values[1] = 0x60;
	max17043_write_data(client, 0x0E, &values[0], 2);

	/*                                 */
	values[0] = 0x00; values[1] = 0x00;
	max17043_write_data(client, 0x0A, &values[0], 2);

	/*                                 */
	values[0] = 0x00; values[1] = 0x00;
	max17043_write_data(client, 0x3E, &values[0], 2);


	/*                                   */
	msleep(200);

	/*                       */
	len = max17043_read_data(client, 0x04, &values[0], 2);

	if (values[0] >= 0xE6 && values[0] <= 0xE8)
		pr_info("%s: Model Guage was loaded successful!!!(%02x)\n", __func__, values[0]);
	else
		pr_info("%s: Model Guage was not loaded successful!!!T.T(%02x)\n", __func__, values[0]);

	/*                           */
	values[0] = 0x4A; values[1] = 0x57;
	max17043_write_data(client, 0x3E, &values[0], 2);


	/*                            */
	values[0] = org_rcomp_msb; values[1] = org_rcomp_lsb;
	max17043_write_data(client, 0x0C, &values[0], 2);
	values[0] = org_ocv_msb; values[1] = org_ocv_lsb;
	max17043_write_data(client, 0x0E, &values[0], 2);

	{
	max17043_read_data(client, 0x0E, &values[0], 2);
	org_ocv_msb = values[0]; org_ocv_lsb = values[1];
	batt_mv = ((values[0] << 4) + (values[1] >> 4));
	batt_mv = (batt_mv*125)/100;
	pr_info("%s : Reread ocv 0x%02x%02x - %d\n", __func__, org_ocv_msb, org_ocv_lsb, batt_mv);
			}

	/*                        */
	values[0] = 0x00; values[1] = 0x00;
	max17043_write_data(client, 0x3E, &values[0], 2);

	/*                                      */
	msleep(200);	

}
EXPORT_SYMBOL(max17040_init_model);


int max17043_set_rcomp_by_temperature(void)
{
	u8 startingRcomp = reference->starting_rcomp;
	int PreviousRcomp = 0;
	int tempCoHot = reference->temp_co_hot;		/*    */
	int tempCoCold = reference->temp_co_cold;	/*      */
	//            
	int newRcomp;
	int temp;
	int ret;
	struct pm8xxx_adc_chan_result result;

//                                         
//                             
//              
//                                              

	if(reference == NULL)
	{
		pr_err("error : reference pointer is NULL!!");
		return -1;
	}
	ret = pm8xxx_adc_read(CHANNEL_BATT_THERM, &result);
	if (ret)
	{
		pr_err("error reading adc channel = %d, ret = %d\n",
		CHANNEL_BATT_THERM, ret);
		return ret;
	}
	temp = result.physical;
	temp /= 10;

	PreviousRcomp = max17043_read_reg(reference->client, MAX17043_CONFIG_REG);
	PreviousRcomp = (PreviousRcomp & 0xFF00) >> 8;
	if (PreviousRcomp < 0)
		return PreviousRcomp;

	pr_info(" max17043 check temp = %d, PreviousRcomp =0x%02X\n", temp, PreviousRcomp);

	if (temp > 20)
		newRcomp = startingRcomp + (int)((temp - 20)*tempCoHot/1000);
	else if (temp < 20){
		newRcomp = startingRcomp + (int)((temp - 20)*tempCoCold/100);
		}
	else
		newRcomp = startingRcomp;

	if (newRcomp > 0xFF)
		newRcomp = 0xFF;
	else if (newRcomp < 0)
		newRcomp = 0;

	if (newRcomp != PreviousRcomp)
	{
		pr_info("RCOMP: new rcomp is 0x%02X(0x%02X)\n", newRcomp, startingRcomp);
		max17043_set_rcomp(newRcomp);
	}
	return 0;
}
/*                                   */

EXPORT_SYMBOL(max17043_set_rcomp_by_temperature);
int max17043_set_alert_level(int alert_level)
{
	return max17043_set_athd(alert_level);
}
EXPORT_SYMBOL(max17043_set_alert_level);
/*             */
/*                                                                           */
int max17043_set_operation(void)
{
	int ret = ENABLE_MAX17043_WORK;

#ifdef CONFIG_LGE_PM_BATTERY_ID_CHECKER
	if (lge_battery_info == BATT_ID_DS2704_N || lge_battery_info == BATT_ID_DS2704_L ||
		lge_battery_info == BATT_ID_ISL6296_N || lge_battery_info == BATT_ID_ISL6296_L ||
		lge_battery_info == BATT_ID_DS2704_C || lge_battery_info == BATT_ID_ISL6296_C)
	{
		ret = ENABLE_MAX17043_WORK;
	}
	else
	{
		ret = DISABLE_MAX17043_WORK;
	}
#else

	/*                                                                   */
	/*
        

                                      

                                                

                                                                     
                       

              
                                                                     
                          
                              
  
     
  
                                         
  
                                                          
                                                   
                                
 */

	ret = ENABLE_MAX17043_WORK;

#endif
	return ret;

}
/*                                                                        */

static int max17043_probe(struct i2c_client *client,
			const struct i2c_device_id *id)
{
	struct i2c_adapter *adapter = to_i2c_adapter(client->dev.parent);
/*                                                                                           */
	struct max17043_platform_data *pdata = client->dev.platform_data;
/*                                   */
	struct max17043_chip *chip;
	int ret = 0;
	u8 values[4];

	printk("[MAX17043]%s:START!!\n\n", __func__);
	if (!i2c_check_functionality(adapter, I2C_FUNC_SMBUS_WORD_DATA)) {
		printk(KERN_ERR " [MAX17043] i2c_check_functionality fail\n");
		return -EIO;
	}

	chip = kzalloc(sizeof(*chip), GFP_KERNEL);
	if (!chip) {
		printk(KERN_ERR " [MAX17043] kzalloc fail\n");
		return -ENOMEM;
	}

	ret = gpio_request(client->irq, "max17043_alert");
	if (ret < 0) {
		printk(KERN_ERR " [MAX17043] GPIO Request Failed\n");
		goto err_gpio_request_failed;
	}
	gpio_direction_input(client->irq);

	ret = request_irq(gpio_to_irq(client->irq),
			max17043_interrupt_handler,
			IRQF_TRIGGER_FALLING,
			"MAX17043_Alert", NULL);
	if (ret < 0) {
		printk(KERN_ERR " [MAX17043] IRQ Request Failed\n");
		goto err_request_irq_failed;
	}

	ret = enable_irq_wake(gpio_to_irq(client->irq));
	if (ret < 0) {
		printk(KERN_ERR "[MAX17043] set irq to wakeup source failed.\n");
		goto err_request_wakeup_irq_failed;
	}

	chip->client = client;

	i2c_set_clientdata(client, chip);

#if 0	/*                                                       */
	chip->battery.name		= "battery";
	chip->battery.type		= POWER_SUPPLY_TYPE_BATTERY;
	chip->battery.get_property	= max17043_get_property;
	chip->battery.properties	= max17042_battery_props;
	chip->battery.num_properties	= ARRAY_SIZE(max17043_battery_props);

	ret = power_supply_register(&client->dev, &chip->battery);
	if (ret) {
		dev_err(&client->dev, "failed: power supply register\n");
		goto err_power_supply_register_failed;
	}
#endif

#if 0
	max17040_init_model(client);
	max17043_get_capacity_from_soc();
#endif

#if 1
{	
	u8 org_ocv_msb, org_ocv_lsb;
	u8 org_rcomp_msb, org_rcomp_lsb;
	u8 values[2];
	int batt_mv;
	long batt_soc = 0;

	/*                     */
	values[0] = 0x4A; values[1] = 0x57;
	max17043_write_data(client, 0x3E, &values[0], 2);

	/*         */
	max17043_read_data(client, 0x0E, &values[0], 2);
	org_ocv_msb = values[0]; org_ocv_lsb = values[1];
	batt_mv = ((values[0] << 4) + (values[1] >> 4));
	batt_mv = (batt_mv*125)/100;
	pr_info("%s : Read OCV 0x%02x%02x - %d\n", __func__, org_ocv_msb, org_ocv_lsb, batt_mv);

	/*          */
	max17043_read_soc(client);
	values[0] = (chip->soc & 0x0000FF00) >> 8;
	values[1] = (chip->soc & 0x000000FF);

	batt_soc = ((values[0]*256)+values[1])*19531;
	pr_info("%s: Read SOC %d(0x%02x:0x%02x):%ld\n", __func__, (int)(batt_soc/10000000), values[0], values[1], batt_soc);
	
	/*           */
	max17043_read_data(client, MAX17043_CONFIG_REG, &values[0], 2);
	org_rcomp_msb = values[0]; org_rcomp_lsb = values[1];
	pr_info("%s : Read RCOMP = 0x%02x%02x \n", __func__, org_rcomp_msb, org_rcomp_lsb);

	/*                  */
	values[0] = 0x00; values[1] = 0x00;
	max17043_write_data(client, 0x3E, &values[0], 2);
}
#endif

	/*                                                                */
	ret = device_create_file(&client->dev, &dev_attr_soc);
	if (ret < 0) {
		pr_err("%s:File device creation failed: %d\n", __func__, ret);
		ret = -ENODEV;
		goto err_create_file_soc_failed;
	}
	/*                                                                  */
	ret = device_create_file(&client->dev, &dev_attr_state);
	if (ret < 0) {
		pr_err("%s:File device creation failed: %d\n", __func__, ret);
		ret = -ENODEV;
		goto err_create_file_state_failed;
	}
	ret = device_create_file(&client->dev, &dev_attr_volt);
	if (ret < 0) {
		pr_err("%s:File device creation failed: %d\n", __func__, ret);
		ret = -ENODEV;
		goto err_create_file_soc_failed;
	}
	ret = device_create_file(&client->dev, &dev_attr_config);
	if (ret < 0) {
		pr_err("%s:File device creation failed: %d\n", __func__, ret);
		ret = -ENODEV;
		goto err_create_file_state_failed;
	}
	ret = device_create_file(&client->dev, &dev_attr_version);
	if (ret < 0) {
		pr_err("%s:File device creation failed: %d\n", __func__, ret);
		ret = -ENODEV;
		goto err_create_file_state_failed;
	}
	chip->vcell = 3480;
	chip->soc = 100 << 8;
	chip->voltage = 4350;
	chip->capacity = 100;
	chip->config = 0x971C;
/*                                                                                           */
	chip->starting_rcomp = pdata->starting_rcomp;
	chip->temp_co_hot = pdata->temp_co_hot;
	chip->temp_co_cold = pdata->temp_co_cold;
/*                                   */

/*                                                                                     */
	//                                     
/*                                   */

	INIT_DELAYED_WORK_DEFERRABLE(&chip->work, max17043_work);
#if 0 /*                             */
	INIT_WORK(&chip->alert_work, max17043_alert_work);
#endif

	reference = chip;
	max17043_read_version(client);
	max17043_read_config(client);
	max17043_set_rcomp(chip->starting_rcomp);
	max17043_set_athd(MONITOR_LEVEL);
	max17043_clear_interrupt(client);
	//          
	values[0] = 0x82; values[1] = 0x00;
	max17043_write_data(client, 0x18, &values[0], 2);
	//                                  
	//                                       

/*                                                                           */
	ret = max17043_set_operation();
	if(!ret)
	{
		pr_err("%s: battery is not present : %d\n", __func__, ret);
		return ret;
	}
/*                                                                        */

	if (need_to_quickstart == -1) {
		max17043_quickstart(client);
		need_to_quickstart = 0;
		schedule_delayed_work(&chip->work, HZ);
		return 0;
	} else {
		max17043_update(client);
		schedule_delayed_work(&chip->work, MAX17043_WORK_DELAY);
	}

	printk("[MAX17043]%s:END!!\n\n", __func__);
	return 0;

err_create_file_state_failed:
	device_remove_file(&client->dev, &dev_attr_soc);
err_create_file_soc_failed:
#if 0	/*                                                        */
err_power_supply_register_failed:
	i2c_set_clientdata(client, NULL);
#endif
	kfree(chip);
	disable_irq_wake(gpio_to_irq(client->irq));
err_request_wakeup_irq_failed:
	free_irq(gpio_to_irq(client->irq), NULL);
err_request_irq_failed:
	gpio_free(client->irq);
err_gpio_request_failed:

	return ret;
}

static int __devexit max17043_remove(struct i2c_client *client)
{
	struct max17043_chip *chip = i2c_get_clientdata(client);

	power_supply_unregister(&chip->battery);
	cancel_delayed_work(&chip->work);
#if 0 /*                             */
	flush_work(&chip->alert_work);
#endif
	i2c_set_clientdata(client, NULL);
	pr_info("max17043_remove -  kfree(chip) !!");
	kfree(chip);
	return 0;
}

#ifdef CONFIG_PM_RUNTIME
static int max17043_suspend(struct i2c_client *client,
		pm_message_t state)
{
	struct max17043_chip *chip = i2c_get_clientdata(client);

	cancel_delayed_work_sync(&chip->work);
#if 0 /*                             */
	flush_work(&chip->alert_work);
#endif
	client->dev.power.power_state = state;

	return 0;
}

static int max17043_resume(struct i2c_client *client)
{
	struct max17043_chip *chip = i2c_get_clientdata(client);
	int ret = 0;

	printk("%s \n", __func__);
/*                                                                           */
	ret = max17043_set_operation();
	if(!ret)
	{
		pr_err("%s: battery is not present or not valid battery : %d\n", __func__, ret);
		return ret;
	}
/*                                                                        */

	schedule_delayed_work(&chip->work, HZ/2);
	client->dev.power.power_state = PMSG_ON;
	/*                                                        */
	max17043_read_config(client);
	max17043_clear_interrupt(client);
	return 0;
}
#else
#define max17043_suspend NULL
#define max17043_resume NULL
#endif /*           */

static const struct i2c_device_id max17043_id[] = {
	{ "max17043", 0 },
	{ }
};
MODULE_DEVICE_TABLE(i2c, max17043_id);

static struct i2c_driver max17043_i2c_driver = {
	.driver	= {
		.name	= "max17043",
		.owner	= THIS_MODULE,
	},
	.probe		= max17043_probe,
	.remove		= __devexit_p(max17043_remove),
	.suspend	= max17043_suspend,
	.resume		= max17043_resume,
	.id_table	= max17043_id,
};

/*                                */
static s32 __init max17043_state(char *str)
{
	switch (str[0]) {
	case 'g':	/*                          */
	case 'q':	/*                 */
		need_to_quickstart = 0;
		break;
	case 'b':	/*                                    */
		need_to_quickstart = 1;
		break;
	case 'e':	/*                                       */
		need_to_quickstart = -1;
		break;
	default:
		/*                    */
		break;
	}
	return 0;
}
__setup("fuelgauge=", max17043_state);
#if 0
static int __init max17043_init(void)
{
	return i2c_add_driver(&max17043_i2c_driver);
}
module_init(max17043_init);

static void __exit max17043_exit(void)
{
	i2c_del_driver(&max17043_i2c_driver);
}
module_exit(max17043_exit);
#endif
module_i2c_driver(max17043_i2c_driver);

MODULE_AUTHOR("LG Electronics");
MODULE_DESCRIPTION("MAX17043 Fuel Gauge");
MODULE_LICENSE("GPL");
