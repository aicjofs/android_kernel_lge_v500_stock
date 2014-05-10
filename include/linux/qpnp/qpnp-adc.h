/*
 * Copyright (c) 2012, The Linux Foundation. All rights reserved.
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
/*
                                            
  
 */

#ifndef __QPNP_ADC_H
#define __QPNP_ADC_H

#include <linux/kernel.h>
#include <linux/list.h>
/* 
                                                       
 */
enum qpnp_vadc_channels {
	USBIN = 0,
	DCIN,
	VCHG_SNS,
	SPARE1_03,
	SPARE2_03,
	VCOIN,
	VBAT_SNS,
	VSYS,
	DIE_TEMP,
	REF_625MV,
	REF_125V,
	CHG_TEMP,
	SPARE1,
	SPARE2,
	GND_REF,
	VDD_VADC,
	P_MUX1_1_1,
	P_MUX2_1_1,
	P_MUX3_1_1,
	P_MUX4_1_1,
	P_MUX5_1_1,
	P_MUX6_1_1,
	P_MUX7_1_1,
	P_MUX8_1_1,
	P_MUX9_1_1,
	P_MUX10_1_1,
	P_MUX11_1_1,
	P_MUX12_1_1,
	P_MUX13_1_1,
	P_MUX14_1_1,
	P_MUX15_1_1,
	P_MUX16_1_1,
	P_MUX1_1_3,
	P_MUX2_1_3,
	P_MUX3_1_3,
	P_MUX4_1_3,
	P_MUX5_1_3,
	P_MUX6_1_3,
	P_MUX7_1_3,
	P_MUX8_1_3,
	P_MUX9_1_3,
	P_MUX10_1_3,
	P_MUX11_1_3,
	P_MUX12_1_3,
	P_MUX13_1_3,
	P_MUX14_1_3,
	P_MUX15_1_3,
	P_MUX16_1_3,
	LR_MUX1_BATT_THERM,
	LR_MUX2_BAT_ID,
	LR_MUX3_XO_THERM,
	LR_MUX4_AMUX_THM1,
	LR_MUX5_AMUX_THM2,
	LR_MUX6_AMUX_THM3,
	LR_MUX7_HW_ID,
	LR_MUX8_AMUX_THM4,
	LR_MUX9_AMUX_THM5,
	LR_MUX10_USB_ID,
	AMUX_PU1,
	AMUX_PU2,
	LR_MUX3_BUF_XO_THERM_BUF,
	LR_MUX1_PU1_BAT_THERM,
	LR_MUX2_PU1_BAT_ID,
	LR_MUX3_PU1_XO_THERM,
	LR_MUX4_PU1_AMUX_THM1,
	LR_MUX5_PU1_AMUX_THM2,
	LR_MUX6_PU1_AMUX_THM3,
	LR_MUX7_PU1_AMUX_HW_ID,
	LR_MUX8_PU1_AMUX_THM4,
	LR_MUX9_PU1_AMUX_THM5,
	LR_MUX10_PU1_AMUX_USB_ID,
	LR_MUX3_BUF_PU1_XO_THERM_BUF,
	LR_MUX1_PU2_BAT_THERM,
	LR_MUX2_PU2_BAT_ID,
	LR_MUX3_PU2_XO_THERM,
	LR_MUX4_PU2_AMUX_THM1,
	LR_MUX5_PU2_AMUX_THM2,
	LR_MUX6_PU2_AMUX_THM3,
	LR_MUX7_PU2_AMUX_HW_ID,
	LR_MUX8_PU2_AMUX_THM4,
	LR_MUX9_PU2_AMUX_THM5,
	LR_MUX10_PU2_AMUX_USB_ID,
	LR_MUX3_BUF_PU2_XO_THERM_BUF,
	LR_MUX1_PU1_PU2_BAT_THERM,
	LR_MUX2_PU1_PU2_BAT_ID,
	LR_MUX3_PU1_PU2_XO_THERM,
	LR_MUX4_PU1_PU2_AMUX_THM1,
	LR_MUX5_PU1_PU2_AMUX_THM2,
	LR_MUX6_PU1_PU2_AMUX_THM3,
	LR_MUX7_PU1_PU2_AMUX_HW_ID,
	LR_MUX8_PU1_PU2_AMUX_THM4,
	LR_MUX9_PU1_PU2_AMUX_THM5,
	LR_MUX10_PU1_PU2_AMUX_USB_ID,
	LR_MUX3_BUF_PU1_PU2_XO_THERM_BUF,
	ALL_OFF,
	ADC_MAX_NUM,
};

/* 
                                                   
 */
enum qpnp_iadc_channels {
	INTERNAL_RSENSE = 0,
	EXTERNAL_RSENSE,
	ALT_LEAD_PAIR,
	GAIN_CALIBRATION_25MV,
	OFFSET_CALIBRATION_SHORT_CADC_LEADS,
	OFFSET_CALIBRATION_CSP_CSN,
	OFFSET_CALIBRATION_CSP2_CSN2,
	IADC_MUX_NUM,
};

#define QPNP_ADC_625_UV	625000
#define QPNP_ADC_HWMON_NAME_LENGTH				16

/* 
                                                           
                         
                        
                        
                        
                                                   
  
                                                                         
 */
enum qpnp_adc_decimation_type {
	DECIMATION_TYPE1 = 0,
	DECIMATION_TYPE2,
	DECIMATION_TYPE3,
	DECIMATION_TYPE4,
	DECIMATION_NONE,
};

/* 
                                                        
                                                               
                                                     
                                                            
  
                                                                    
                                                                  
                                            
 */
enum qpnp_adc_calib_type {
	CALIB_ABSOLUTE = 0,
	CALIB_RATIOMETRIC,
	CALIB_NONE,
};

/* 
                                                                
                                       
                                       
                                       
                                       
                                        
                                                           
  
                                                                        
              
 */
enum qpnp_adc_channel_scaling_param {
	PATH_SCALING0 = 0,
	PATH_SCALING1,
	PATH_SCALING2,
	PATH_SCALING3,
	PATH_SCALING4,
	PATH_SCALING_NONE,
};

/* 
                                                                           
                                                
                                                                          
                                                                            
                                                    
                                                                      
                                                                            
                                                 
 */
enum qpnp_adc_scale_fn_type {
	SCALE_DEFAULT = 0,
	SCALE_BATT_THERM,
	SCALE_PA_THERM,
	SCALE_PMIC_THERM,
	SCALE_XOTHERM,
	SCALE_NONE,
};

/* 
                                                                        
                                                           
                                                     
                                  
                                    
                                    
                                    
                                    
                                     
                                     
                                     
                                      
                                      
                                      
 */
enum qpnp_adc_fast_avg_ctl {
	ADC_FAST_AVG_SAMPLE_1 = 0,
	ADC_FAST_AVG_SAMPLE_2,
	ADC_FAST_AVG_SAMPLE_4,
	ADC_FAST_AVG_SAMPLE_8,
	ADC_FAST_AVG_SAMPLE_16,
	ADC_FAST_AVG_SAMPLE_32,
	ADC_FAST_AVG_SAMPLE_64,
	ADC_FAST_AVG_SAMPLE_128,
	ADC_FAST_AVG_SAMPLE_256,
	ADC_FAST_AVG_SAMPLE_512,
	ADC_FAST_AVG_SAMPLE_NONE,
};

/* 
                                                                          
                                                          
                                                
                                          
                                            
                                            
                                            
                                            
                                            
                                            
                                            
                                            
                                            
                                          
                                          
                                          
                                          
                                          
                                           
                              
 */
enum qpnp_adc_hw_settle_time {
	ADC_CHANNEL_HW_SETTLE_DELAY_0US = 0,
	ADC_CHANNEL_HW_SETTLE_DELAY_100US,
	ADC_CHANNEL_HW_SETTLE_DELAY_2000US,
	ADC_CHANNEL_HW_SETTLE_DELAY_300US,
	ADC_CHANNEL_HW_SETTLE_DELAY_400US,
	ADC_CHANNEL_HW_SETTLE_DELAY_500US,
	ADC_CHANNEL_HW_SETTLE_DELAY_600US,
	ADC_CHANNEL_HW_SETTLE_DELAY_700US,
	ADC_CHANNEL_HW_SETTLE_DELAY_800US,
	ADC_CHANNEL_HW_SETTLE_DELAY_900US,
	ADC_CHANNEL_HW_SETTLE_DELAY_1MS,
	ADC_CHANNEL_HW_SETTLE_DELAY_2MS,
	ADC_CHANNEL_HW_SETTLE_DELAY_4MS,
	ADC_CHANNEL_HW_SETTLE_DELAY_6MS,
	ADC_CHANNEL_HW_SETTLE_DELAY_8MS,
	ADC_CHANNEL_HW_SETTLE_DELAY_10MS,
	ADC_CHANNEL_HW_SETTLE_NONE,
};

/* 
                                                                 
                                                     
                                                    
                                             
                                                   
                                                          
                                                                 
                                                                      
                                            
                                         
                            
                                                                     
                                                              
                                            
 */
enum qpnp_vadc_mode_sel {
	ADC_OP_NORMAL_MODE = 0,
	ADC_OP_CONVERSION_SEQUENCER,
	ADC_OP_MEASUREMENT_INTERVAL,
	ADC_OP_MODE_NONE,
};

/* 
                                                                  
                              
                                           
                                                             
                                               
                      
 */
enum qpnp_vadc_trigger {
	ADC_GSM_PA_ON = 0,
	ADC_TX_GTR_THRES,
	ADC_CAMERA_FLASH_RAMP,
	ADC_DTEST,
	ADC_SEQ_NONE,
};

/* 
                                                                  
                                                         
                  
 */
enum qpnp_vadc_conv_seq_timeout {
	ADC_CONV_SEQ_TIMEOUT_0MS = 0,
	ADC_CONV_SEQ_TIMEOUT_1MS,
	ADC_CONV_SEQ_TIMEOUT_2MS,
	ADC_CONV_SEQ_TIMEOUT_3MS,
	ADC_CONV_SEQ_TIMEOUT_4MS,
	ADC_CONV_SEQ_TIMEOUT_5MS,
	ADC_CONV_SEQ_TIMEOUT_6MS,
	ADC_CONV_SEQ_TIMEOUT_7MS,
	ADC_CONV_SEQ_TIMEOUT_8MS,
	ADC_CONV_SEQ_TIMEOUT_9MS,
	ADC_CONV_SEQ_TIMEOUT_10MS,
	ADC_CONV_SEQ_TIMEOUT_11MS,
	ADC_CONV_SEQ_TIMEOUT_12MS,
	ADC_CONV_SEQ_TIMEOUT_13MS,
	ADC_CONV_SEQ_TIMEOUT_14MS,
	ADC_CONV_SEQ_TIMEOUT_15MS,
	ADC_CONV_SEQ_TIMEOUT_NONE,
};

/* 
                                                                
                                                      
                                              
 */
enum qpnp_adc_conv_seq_holdoff {
	ADC_SEQ_HOLD_25US = 0,
	ADC_SEQ_HOLD_50US,
	ADC_SEQ_HOLD_75US,
	ADC_SEQ_HOLD_100US,
	ADC_SEQ_HOLD_125US,
	ADC_SEQ_HOLD_150US,
	ADC_SEQ_HOLD_175US,
	ADC_SEQ_HOLD_200US,
	ADC_SEQ_HOLD_225US,
	ADC_SEQ_HOLD_250US,
	ADC_SEQ_HOLD_275US,
	ADC_SEQ_HOLD_300US,
	ADC_SEQ_HOLD_325US,
	ADC_SEQ_HOLD_350US,
	ADC_SEQ_HOLD_375US,
	ADC_SEQ_HOLD_400US,
	ADC_SEQ_HOLD_NONE,
};

/* 
                                                                      
                                             
                                                         
                                                                     
                                                       
                                                          
                                                                      
                                                       
                                               
 */
enum qpnp_adc_conv_seq_state {
	ADC_CONV_SEQ_IDLE = 0,
	ADC_CONV_TRIG_RISE,
	ADC_CONV_TRIG_HOLDOFF,
	ADC_CONV_MEAS_RISE,
	ADC_CONV_TRIG_FALL,
	ADC_CONV_FALL_HOLDOFF,
	ADC_CONV_MEAS_FALL,
	ADC_CONV_ERROR,
	ADC_CONV_NONE,
};

/* 
                                                                    
                                                         
                               
                                 
                                 
                                   
                                   
                                     
                                     
                                     
                                   
                                   
                                   
                                   
                                   
                                   
                                   
                                    
 */
enum qpnp_adc_meas_timer {
	ADC_MEAS_INTERVAL_0MS = 0,
	ADC_MEAS_INTERVAL_1P0MS,
	ADC_MEAS_INTERVAL_2P0MS,
	ADC_MEAS_INTERVAL_3P9MS,
	ADC_MEAS_INTERVAL_7P8MS,
	ADC_MEAS_INTERVAL_15P6MS,
	ADC_MEAS_INTERVAL_31P3MS,
	ADC_MEAS_INTERVAL_62P5MS,
	ADC_MEAS_INTERVAL_125MS,
	ADC_MEAS_INTERVAL_250MS,
	ADC_MEAS_INTERVAL_500MS,
	ADC_MEAS_INTERVAL_1S,
	ADC_MEAS_INTERVAL_2S,
	ADC_MEAS_INTERVAL_4S,
	ADC_MEAS_INTERVAL_8S,
	ADC_MEAS_INTERVAL_16S,
	ADC_MEAS_INTERVAL_NONE,
};

/* 
                                                              
                                                                              
           
                                                                               
           
 */
enum qpnp_adc_meas_interval_op_ctl {
	ADC_MEAS_INTERVAL_OP_SINGLE = 0,
	ADC_MEAS_INTERVAL_OP_CONTINUOUS,
	ADC_MEAS_INTERVAL_OP_NONE,
};

/* 
                                                                 
                                              
                                                
                                                                     
                                                                   
  
                                                                              
                           
 */
struct qpnp_vadc_linear_graph {
	int64_t dy;
	int64_t dx;
	int64_t adc_vref;
	int64_t adc_gnd;
};

/* 
                                                                         
                                        
                                                                     
                  
 */
struct qpnp_vadc_map_pt {
	int32_t x;
	int32_t y;
};

/* 
                                                                          
                                     
                                       
 */
struct qpnp_vadc_scaling_ratio {
	int32_t num;
	int32_t den;
};

/* 
                                                             
                                                  
                                                   
                                   
 */
struct qpnp_adc_properties {
	uint32_t	adc_vdd_reference;
	uint32_t	bitresolution;
	bool		bipolar;
};

/* 
                                                                              
                                                                           
                      
                                                                               
                        
                                                                              
 */
struct qpnp_vadc_chan_properties {
	uint32_t			offset_gain_numerator;
	uint32_t			offset_gain_denominator;
	struct qpnp_vadc_linear_graph	adc_graph[2];
};

/* 
                                                                 
                                                         
                                                                              
                           
                                                                           
                                                                  
                                                             
                                                                           
                                            
                                                            
                                                                
                                                             
                                                                   
 */
struct qpnp_vadc_result {
	uint32_t	chan;
	int32_t		adc_code;
	int64_t		measurement;
	int64_t		physical;
};

/* 
                                                                
                              
                                                                
                                                                        
                                                          
                                                                       
                                                            
                                                             
 */
struct qpnp_vadc_amux {
	char					*name;
	enum qpnp_vadc_channels			channel_num;
	enum qpnp_adc_channel_scaling_param	chan_path_prescaling;
	enum qpnp_adc_decimation_type		adc_decimation;
	enum qpnp_adc_scale_fn_type		adc_scale_fn;
	enum qpnp_adc_fast_avg_ctl		fast_avg_setup;
	enum qpnp_adc_hw_settle_time		hw_settle_time;
};

/* 
                                 
  
 */
static const struct qpnp_vadc_scaling_ratio qpnp_vadc_amux_scaling_ratio[] = {
	{1, 1},
	{1, 3},
	{1, 4},
	{1, 6},
	{1, 20}
};

/* 
                                                         
                                                          
                                                      
                                  
 */
struct qpnp_vadc_scale_fn {
	int32_t (*chan) (int32_t,
		const struct qpnp_adc_properties *,
		const struct qpnp_vadc_chan_properties *,
		struct qpnp_vadc_result *);
};

/* 
                                                               
                                                                 
                                                            
                                                  
                                          
                               
 */
struct qpnp_iadc_calib {
	enum qpnp_iadc_channels		channel;
	int32_t				offset;
	int32_t				gain;
};

/* 
                                                   
                                          
                                                
                                                             
                                                                
                                                                 
                                                              
                                                     
                                                                 
                   
                                                                  
 */
struct qpnp_adc_drv {
	struct spmi_device		*spmi;
	uint8_t				slave;
	uint16_t			offset;
	struct qpnp_adc_properties	*adc_prop;
	struct qpnp_adc_amux_properties	*amux_prop;
	struct qpnp_vadc_amux		*adc_channels;
	int				adc_irq;
	struct mutex			adc_lock;
	struct completion		adc_rslt_completion;
	struct qpnp_iadc_calib		calib;
};

/* 
                                                                     
                                                       
                                                         
                                           
                                                                   
                         
                                                                  
                                                 
                                                                  
                                                            
 */
struct qpnp_adc_amux_properties {
	uint32_t			amux_channel;
	uint32_t			decimation;
	uint32_t			mode_sel;
	uint32_t			hw_settle_time;
	uint32_t			fast_avg_setup;
	enum qpnp_vadc_trigger		trigger_channel;
	struct qpnp_vadc_chan_properties	chan_prop[0];
};

/*            */
#if defined(CONFIG_SENSORS_QPNP_ADC_VOLTAGE)				\
			|| defined(CONFIG_SENSORS_QPNP_ADC_VOLTAGE_MODULE)
/* 
                                                       
                                                   
                                                     
                                             
 */
int32_t qpnp_vadc_read(enum qpnp_vadc_channels channel,
				struct qpnp_vadc_result *result);

/* 
                                                                     
                        
                                                   
                                                     
                                             
 */
int32_t qpnp_vadc_conv_seq_request(
			enum qpnp_vadc_trigger trigger_channel,
			enum qpnp_vadc_channels channel,
			struct qpnp_vadc_result *result);

/* 
                                                                 
                                                            
 */
int32_t qpnp_vadc_check_result(int32_t *data);

/* 
                                                                      
                          
                                             
 */
int32_t qpnp_adc_get_devicetree_data(struct spmi_device *spmi,
					struct qpnp_adc_drv *adc_qpnp);

/* 
                                                                    
                                                                  
 */
int32_t qpnp_vadc_configure(
			struct qpnp_adc_amux_properties *chan_prop);

/* 
                                                                      
                                                          
                    
                                                      
                                                                    
                      
                                                                           
                     
                                            
 */
int32_t qpnp_adc_scale_default(int32_t adc_code,
			const struct qpnp_adc_properties *adc_prop,
			const struct qpnp_vadc_chan_properties *chan_prop,
			struct qpnp_vadc_result *chan_rslt);
/* 
                                                                         
                                                          
                                                               
                                  
                                                      
                                                                    
                      
                                                                           
                     
                                            
 */
int32_t qpnp_adc_scale_pmic_therm(int32_t adc_code,
			const struct qpnp_adc_properties *adc_prop,
			const struct qpnp_vadc_chan_properties *chan_prop,
			struct qpnp_vadc_result *chan_rslt);
/* 
                                                                         
                                                          
                                                     
                                                      
                                                                      
                      
                                                                           
                     
                                            
 */
int32_t qpnp_adc_scale_batt_therm(int32_t adc_code,
			const struct qpnp_adc_properties *adc_prop,
			const struct qpnp_vadc_chan_properties *chan_prop,
			struct qpnp_vadc_result *chan_rslt);
/* 
                                                                      
                                                          
                    
                                                      
                                                                      
                      
                                                                           
                     
                                            
 */
int32_t qpnp_adc_scale_batt_id(int32_t adc_code,
			const struct qpnp_adc_properties *adc_prop,
			const struct qpnp_vadc_chan_properties *chan_prop,
			struct qpnp_vadc_result *chan_rslt);
/* 
                                                                            
                                                          
                                                                    
       
                                                      
                                                                      
                      
                                                                           
                     
                                            
 */
int32_t qpnp_adc_tdkntcg_therm(int32_t adc_code,
			const struct qpnp_adc_properties *adc_prop,
			const struct qpnp_vadc_chan_properties *chan_prop,
			struct qpnp_vadc_result *chan_rslt);
/* 
                                                                  
                              
                                                                    
                    
 */
int32_t qpnp_vadc_is_ready(void);
#else
static inline int32_t qpnp_vadc_read(uint32_t channel,
				struct qpnp_vadc_result *result)
{ return -ENXIO; }
static inline int32_t qpnp_vadc_conv_seq_request(
			enum qpnp_vadc_trigger trigger_channel,
			enum qpnp_vadc_channels channel,
			struct qpnp_vadc_result *result)
{ return -ENXIO; }
static inline int32_t qpnp_adc_scale_default(int32_t adc_code,
			const struct qpnp_adc_properties *adc_prop,
			const struct qpnp_vadc_chan_properties *chan_prop,
			struct qpnp_vadc_result *chan_rslt)
{ return -ENXIO; }
static inline int32_t qpnp_adc_scale_pmic_therm(int32_t adc_code,
			const struct qpnp_adc_properties *adc_prop,
			const struct qpnp_vadc_chan_properties *chan_prop,
			struct qpnp_vadc_result *chan_rslt)
{ return -ENXIO; }
static inline int32_t qpnp_adc_scale_batt_therm(int32_t adc_code,
			const struct qpnp_adc_properties *adc_prop,
			const struct qpnp_vadc_chan_properties *chan_prop,
			struct qpnp_vadc_result *chan_rslt)
{ return -ENXIO; }
static inline int32_t qpnp_adc_scale_batt_id(int32_t adc_code,
			const struct qpnp_adc_properties *adc_prop,
			const struct qpnp_vadc_chan_properties *chan_prop,
			struct qpnp_vadc_result *chan_rslt)
{ return -ENXIO; }
static inline int32_t qpnp_adc_tdkntcg_therm(int32_t adc_code,
			const struct qpnp_adc_properties *adc_prop,
			const struct qpnp_vadc_chan_properties *chan_prop,
			struct qpnp_vadc_result *chan_rslt)
{ return -ENXIO; }
static inline int32_t qpnp_vadc_is_read(void)
{ return -ENXIO; }
#endif

/*            */
#if defined(CONFIG_SENSORS_QPNP_ADC_CURRENT)				\
			|| defined(CONFIG_SENSORS_QPNP_ADC_CURRENT_MODULE)
/* 
                                                               
                                                   
                                       
 */
int32_t qpnp_iadc_read(enum qpnp_iadc_channels channel,
							int32_t *result);
/* 
                                                                       
                    
                                              
 */
int32_t qpnp_iadc_get_gain(int32_t *result);

/* 
                                                                     
                                                 
                                                 
                                              
 */
int32_t qpnp_iadc_get_offset(enum qpnp_iadc_channels channel,
						int32_t *result);
/* 
                                                                  
                              
                                                                    
                    
 */
int32_t qpnp_iadc_is_ready(void);
#else
static inline int32_t qpnp_iadc_read(enum qpnp_iadc_channels channel,
							int *result)
{ return -ENXIO; }
static inline int32_t qpnp_iadc_get_gain(int32_t *result)
{ return -ENXIO; }
static inline int32_t qpnp_iadc_get_offset(enum qpnp_iadc_channels channel,
						int32_t *result)
{ return -ENXIO; }
static inline int32_t qpnp_iadc_is_read(void)
{ return -ENXIO; }
#endif

#endif
