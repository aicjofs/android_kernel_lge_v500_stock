/* Copyright (c) 2011-2013, The Linux Foundation. All rights reserved.
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

#ifndef __PM8XXX_CHARGER_H
#define __PM8XXX_CHARGER_H

#include <linux/errno.h>
#include <linux/power_supply.h>

#define PM8921_CHARGER_DEV_NAME	"pm8921-charger"

struct pm8xxx_charger_core_data {
	unsigned int	vbat_channel;
	unsigned int	batt_temp_channel;
	unsigned int	batt_id_channel;
};

enum pm8921_chg_cold_thr {
	PM_SMBC_BATT_TEMP_COLD_THR__LOW,
	PM_SMBC_BATT_TEMP_COLD_THR__HIGH
};

enum pm8921_chg_hot_thr	{
	PM_SMBC_BATT_TEMP_HOT_THR__LOW,
	PM_SMBC_BATT_TEMP_HOT_THR__HIGH
};

enum pm8921_usb_ov_threshold {
	PM_USB_OV_5P5V,
	PM_USB_OV_6V,
	PM_USB_OV_6P5V,
	PM_USB_OV_7V,
};

enum pm8921_usb_debounce_time {
	PM_USB_BYPASS_DEBOUNCER,
	PM_USB_DEBOUNCE_20P5MS,
	PM_USB_DEBOUNCE_40P5MS,
	PM_USB_DEBOUNCE_80P5MS,
};

enum pm8921_chg_led_src_config {
	LED_SRC_GND,
	LED_SRC_VPH_PWR,
	LED_SRC_5V,
	LED_SRC_MIN_VPH_5V,
	LED_SRC_BYPASS,
};

/* 
                                        
                                                  
                                                   
                                                 
                                                                         
                                                                      
                                                                        
                                                                         
                                                                     
                                                          
                       
                                                                        
                                                                       
                                                
                           
                                                                          
                                              
                                                                       
                                              
               
                                                               
                                                              
                                                             
                                       
                                                              
                                                            
                                       
                                                                       
                                                            
         
                                                                
                                         
                                               
                                                                   
                                                                    
                                                                
                                                                
                              
                                                
                                                   
                                                                    
                                                                    
                                               
                                                                 
                                                      
                                                                         
                                                                         
                                                              
                                                         
                                         
                                                                          
              
                                                                     
                                                                      
                                                       
                                                             
                                                 
                                                                 
                                                        
                                                        
                                                       
                                                                     
                                                       
                                                          
                
                                                                    
                                                                    
                                                       
                                                            
                                                                         
                                                  
                              
                                                                        
                                                 
                              
                                                                          
                                         
                                                                           
                                                
                                                
                      
                                                                        
                                                     
                                                 
                                   
 */
struct pm8921_charger_platform_data {
	struct pm8xxx_charger_core_data	charger_cdata;
	unsigned int			ttrkl_time;
	unsigned int			update_time;
	unsigned int			max_voltage;
	unsigned int			min_voltage;
	unsigned int			uvd_thresh_voltage;
	unsigned int			safe_current_ma;
	unsigned int			alarm_low_mv;
	unsigned int			alarm_high_mv;
	unsigned int			resume_voltage_delta;
	int				resume_charge_percent;
	unsigned int			term_current;
	int				cool_temp;
	int				warm_temp;
	unsigned int			temp_check_period;
	unsigned int			max_bat_chg_current;
	unsigned int			usb_max_current;
	unsigned int			cool_bat_chg_current;
	unsigned int			warm_bat_chg_current;
#ifdef CONFIG_LGE_CHARGER_TEMP_SCENARIO
	int						temp_level_1;
#if defined(CONFIG_MACH_APQ8064_J1SP)
/*                                         */
	int						temp_level_1_1;
#endif
	int						temp_level_2;
	int						temp_level_3;
	int						temp_level_4;
	int						temp_level_5;
#endif
	unsigned int			cool_bat_voltage;
	unsigned int			warm_bat_voltage;
	int				hysteresis_temp;
	unsigned int			(*get_batt_capacity_percent) (void);
	int64_t				batt_id_min;
	int64_t				batt_id_max;
	bool				keep_btm_on_suspend;
	bool				has_dc_supply;
	int				trkl_voltage;
	int				weak_voltage;
	int				trkl_current;
	int				weak_current;
	int				vin_min;
	int				*thermal_mitigation;
	int				thermal_levels;
	enum pm8921_chg_cold_thr	cold_thr;
	enum pm8921_chg_hot_thr		hot_thr;
	int				rconn_mohm;
	enum pm8921_chg_led_src_config	led_src_config;
	int				battery_less_hardware;
	int				btc_override;
	int				btc_override_cold_degc;
	int				btc_override_hot_degc;
	int				btc_delay_ms;
	int				btc_panic_if_cant_stop_chg;
	int				stop_chg_upon_expiry;
	bool				disable_chg_rmvl_wrkarnd;
	bool				enable_tcxo_warmup_delay;
#ifdef CONFIG_LGE_PM
	/*                    */
	int 			eoc_check_soc;
#endif
};

enum pm8921_charger_source {
	PM8921_CHG_SRC_NONE,
	PM8921_CHG_SRC_USB,
	PM8921_CHG_SRC_DC,
};
#ifdef CONFIG_BATTERY_MAX17043
void pm8921_charger_force_update_batt_psy(void);
#endif

#if defined(CONFIG_PM8921_CHARGER) || defined(CONFIG_PM8921_CHARGER_MODULE)
void pm8921_charger_vbus_draw(unsigned int mA);
int pm8921_charger_register_vbus_sn(void (*callback)(int));
void pm8921_charger_unregister_vbus_sn(void (*callback)(int));
/* 
                          
  
                                                    
  
                                                                              
                           
 */
int pm8921_charger_enable(bool enable);

/* 
                                                   
  
                                                                 
 */
int pm8921_is_usb_chg_plugged_in(void);

/* 
                                                 
  
                                                                
 */
int pm8921_is_dc_chg_plugged_in(void);

/* 
                              
  
                                               
 */
int pm8921_is_battery_present(void);
#ifdef CONFIG_LGE_PM
/*                                                                                     */
int pm8921_is_real_battery_present(void);
/*                                                                                     */
int pm8921_chg_get_fsm_state(void);
#endif

/* 
                                                                      
  
                                          
 */
int pm8921_set_max_battery_charge_current(int ma);

/* 
                                                                  
  
                                             
  
                                                    
                                                            
                                                             
                                                     
 */
int pm8921_disable_input_current_limit(bool disable);

/* 
                                                         
  
                                
  
                                                          
                                                               
                                                   
 */

int pm8921_set_usb_power_supply_type(enum power_supply_type type);

/* 
                                                                      
                                                                        
  
                                                                          
                                                                   
 */
int pm8921_disable_source_current(bool disable);

/* 
                                  
                                              
                                              
 */
int pm8921_regulate_input_voltage(int voltage);
/* 
                               
                                                                               
                                                
  
                                                             
 */
bool pm8921_is_battery_charging(int *source);

/* 
                                                     
  
 */
int pm8921_batt_temperature(void);
/* 
                                 
                                         
                        
 */
int pm8921_usb_ovp_set_threshold(enum pm8921_usb_ov_threshold ov);

/* 
                                 
                              
  
                                                             
  
 */
int pm8921_usb_ovp_set_hystersis(enum pm8921_usb_debounce_time ms);

/* 
                           
  
                                                                        
                                                                    
                                                                   
  
 */
int pm8921_usb_ovp_disable(int disable);
#ifdef CONFIG_LGE_PM
     /*                    */
int pm8921_get_batt_state(void);
int pm8921_force_start_charging(void);
int pm8921_get_batt_health(void);
#endif
/* 
                                               
  
                                                                     
                             
 */
int pm8921_is_batfet_closed(void);

/*                         */
int pm8921_chg_batfet_set_ext(int on);
int pm8921_chg_batfet_get_ext(void);
/*                         */

#else
static inline void pm8921_charger_vbus_draw(unsigned int mA)
{
}
static inline int pm8921_charger_register_vbus_sn(void (*callback)(int))
{
	return -ENXIO;
}
static inline void pm8921_charger_unregister_vbus_sn(void (*callback)(int))
{
}
static inline int pm8921_charger_enable(bool enable)
{
	return -ENXIO;
}
static inline int pm8921_is_usb_chg_plugged_in(void)
{
	return -ENXIO;
}
static inline int pm8921_is_dc_chg_plugged_in(void)
{
	return -ENXIO;
}
static inline int pm8921_is_battery_present(void)
{
	return -ENXIO;
}
static inline int pm8917_set_under_voltage_detection_threshold(int mv)
{
	return -ENXIO;
}
static inline int pm8921_disable_input_current_limit(bool disable)
{
	return -ENXIO;
}
static inline int pm8921_set_usb_power_supply_type(enum power_supply_type type)
{
	return -ENXIO;
}
static inline int pm8921_set_max_battery_charge_current(int ma)
{
	return -ENXIO;
}
static inline int pm8921_disable_source_current(bool disable)
{
	return -ENXIO;
}
static inline int pm8921_regulate_input_voltage(int voltage)
{
	return -ENXIO;
}
static inline bool pm8921_is_battery_charging(int *source)
{
	*source = PM8921_CHG_SRC_NONE;
	return 0;
}
static inline int pm8921_batt_temperature(void)
{
	return -ENXIO;
}
#if defined(CONFIG_BATTERY_MAX17043) || defined(CONFIG_BATTERY_MAX17048)
static inline void pm8921_charger_force_update_batt_psy(void)
{
}
#endif
static inline int pm8921_usb_ovp_set_threshold(enum pm8921_usb_ov_threshold ov)
{
	return -ENXIO;
}
static inline int pm8921_usb_ovp_set_hystersis(enum pm8921_usb_debounce_time ms)
{
	return -ENXIO;
}
static inline int pm8921_usb_ovp_disable(int disable)
{
	return -ENXIO;
}
static inline int pm8921_is_batfet_closed(void)
{
	return 1;
}
#endif

#endif

