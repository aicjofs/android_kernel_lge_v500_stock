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
 */

#ifndef __PM8XXX_BMS_BATTERYDATA_H
#define __PM8XXX_BMS_BATTERYDATA_H

#include <linux/errno.h>

#define FCC_CC_COLS		5
#define FCC_TEMP_COLS		8

#ifdef CONFIG_MACH_APQ8064_AWIFI
#define PC_CC_ROWS             30
#define PC_CC_COLS             13
#define PC_TEMP_ROWS		31
#define PC_TEMP_COLS		8
#else
#define PC_CC_ROWS             29
#define PC_CC_COLS             13
#define PC_TEMP_ROWS		29
#define PC_TEMP_COLS		8
#endif

#define MAX_SINGLE_LUT_COLS	20

struct single_row_lut {
	int x[MAX_SINGLE_LUT_COLS];
	int y[MAX_SINGLE_LUT_COLS];
	int cols;
};

/* 
                  
                                                                  
                                                                
                                                               
                               
                                                                 
                                                                          
                                                                  
                                
 */
struct sf_lut {
	int rows;
	int cols;
	int row_entries[PC_CC_COLS];
	int percent[PC_CC_ROWS];
	int sf[PC_CC_ROWS][PC_CC_COLS];
};

/* 
                           
                                                                    
                                                                 
                                                                      
                                                                
                                                                           
                                                                  
                                 
 */
struct pc_temp_ocv_lut {
	int rows;
	int cols;
	int temp[PC_TEMP_COLS];
	int percent[PC_TEMP_ROWS];
	int ocv[PC_TEMP_ROWS][PC_TEMP_COLS];
};

enum battery_type {
	BATT_UNKNOWN = 0,
	BATT_PALLADIUM,
	BATT_DESAY,
#ifdef CONFIG_LGE_PM
	BATT_LGE, /*                                                              */
	BATT_1900_LGE,
	BATT_2100_LGE,
	BATT_2200_LGE,
	BATT_4600_LGE
#endif
};

/* 
                            
                                         
                                                  
                                                                           
                                                                       
                       
                                                                      
                                   
                                                                           
                                         
                                                                     
                                        
                                                                      
                         
 */

struct bms_battery_data {
	unsigned int		fcc;
	struct single_row_lut	*fcc_temp_lut;
	struct single_row_lut	*fcc_sf_lut;
	struct pc_temp_ocv_lut	*pc_temp_ocv_lut;
	struct sf_lut		*pc_sf_lut;
	struct sf_lut		*rbatt_sf_lut;
	int			default_rbatt_mohm;
	int			delta_rbatt_mohm;
	int			rbatt_capacitive_mohm;
};

#if defined(CONFIG_PM8921_BMS) || \
	defined(CONFIG_PM8921_BMS_MODULE)
extern struct bms_battery_data  palladium_1500_data;
extern struct bms_battery_data  desay_5200_data;

#ifdef CONFIG_LGE_PM
#ifdef CONFIG_MACH_APQ8064_AWIFI
extern struct bms_battery_data  LGE_4600_PMH_data;
#else
extern struct bms_battery_data  lge_1900_data;
extern struct bms_battery_data  lge_1840_data;
extern struct bms_battery_data  LGE_2100_PMH_data;
extern struct bms_battery_data  LGE_2200_PMH_data;
#endif
#endif

int interpolate_fcc(struct single_row_lut *fcc_temp_lut, int batt_temp);
int interpolate_scalingfactor(struct sf_lut *sf_lut, int row_entry, int pc);
int interpolate_scalingfactor_fcc(struct single_row_lut *fcc_sf_lut,
				int cycles);
int interpolate_pc(struct pc_temp_ocv_lut *pc_temp_ocv,
				int batt_temp_degc, int ocv);
int interpolate_ocv(struct pc_temp_ocv_lut *pc_temp_ocv,
				int batt_temp_degc, int pc);
int linear_interpolate(int y0, int x0, int y1, int x1, int x);
int is_between(int left, int right, int value);
#else
static inline int interpolate_fcc(struct single_row_lut *fcc_temp_lut,
			int batt_temp)
{
	return -EINVAL;
}
static inline int interpolate_scalingfactor(struct sf_lut *sf_lut,
			int row_entry, int pc)
{
	return -EINVAL;
}
static inline int interpolate_scalingfactor_fcc(
			struct single_row_lut *fcc_sf_lut, int cycles)
{
	return -EINVAL;
}
static inline int interpolate_pc(struct pc_temp_ocv_lut *pc_temp_ocv,
			int batt_temp_degc, int ocv)
{
	return -EINVAL;
}
static inline int interpolate_ocv(struct pc_temp_ocv_lut *pc_temp_ocv,
			int batt_temp_degc, int pc)
{
	return -EINVAL;
}
static inline int linear_interpolate(int y0, int x0, int y1, int x1, int x)
{
	return -EINVAL;
}
static inline int is_between(int left, int right, int value)
{
	return -EINVAL;
}
#endif

#endif
