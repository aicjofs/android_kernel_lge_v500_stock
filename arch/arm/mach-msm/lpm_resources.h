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

#ifndef __ARCH_ARM_MACH_MSM_LPM_RESOURCES_H
#define __ARCH_ARM_MACH_MSM_LPM_RESOURCES_H

#include "pm.h"

enum {
	MSM_LPM_PXO_OFF = 0,
	MSM_LPM_PXO_ON = 1,
};

enum {
	MSM_LPM_L2_CACHE_HSFS_OPEN = 0,
	MSM_LPM_L2_CACHE_GDHS = 1,
	MSM_LPM_L2_CACHE_RETENTION = 2,
	MSM_LPM_L2_CACHE_ACTIVE = 3,
};

struct msm_rpmrs_limits {
	uint32_t pxo;
	uint32_t l2_cache;
	uint32_t vdd_mem_upper_bound;
	uint32_t vdd_mem_lower_bound;
	uint32_t vdd_dig_upper_bound;
	uint32_t vdd_dig_lower_bound;

	uint32_t latency_us[NR_CPUS];
	uint32_t power[NR_CPUS];
};

struct msm_rpmrs_level {
	enum msm_pm_sleep_mode sleep_mode;
	struct msm_rpmrs_limits rs_limits;
	bool available;
	uint32_t latency_us;
	uint32_t steady_state_power;
	uint32_t energy_overhead;
	uint32_t time_overhead_us;
};

#ifdef CONFIG_MSM_RPM_SMD

/* 
                                                                             
                                                                             
                                                                              
              
  
                                                                
  
                                                                        
                   
 */
bool msm_lpm_level_beyond_limit(struct msm_rpmrs_limits *limits);

/* 
                                                                             
                                                                          
                                                                        
                 

                                       
                                                                               
                                                                     
                                   
                                                                            
  
                        
 */
int msm_lpmrs_enter_sleep(uint32_t sclk_count, struct msm_rpmrs_limits *limits,
	bool from_idle, bool notify_rpm);

/* 
                                                                  
                                                                          
                                                                     
                                   
                                                                            
                                                                  
 */
void msm_lpmrs_exit_sleep(struct msm_rpmrs_limits *limits,
	bool from_idle, bool notify_rpm, bool collapsed);
/* 
                                                                         
                                                                          
                         
  
                        
 */
int __init msm_lpmrs_module_init(void);

#else
static inline bool msm_lpm_level_beyond_limit(struct msm_rpmrs_limits *limits)
{
	return true;
}

static inline int msm_lpmrs_enter_sleep(uint32_t sclk_count,
	struct msm_rpmrs_limits *limits, bool from_idle, bool notify_rpm)
{
	return 0;
}

static inline void msm_lpmrs_exit_sleep(struct msm_rpmrs_limits *limits,
	bool from_idle, bool notify_rpm, bool collapsed)
{
	return;
}

static inline int __init msm_lpmrs_module_init(void)
{
	return 0;
}
#endif /*                    */

#endif
