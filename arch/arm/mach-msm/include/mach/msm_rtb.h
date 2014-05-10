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
#ifndef __MSM_RTB_H__
#define __MSM_RTB_H__

/*
                                                                
                                                                     
 */
enum logk_event_type {
	LOGK_NONE = 0,
	LOGK_READL = 1,
	LOGK_WRITEL = 2,
	LOGK_LOGBUF = 3,
	LOGK_HOTPLUG = 4,
	LOGK_CTXID = 5,
	LOGK_TIMESTAMP = 6,
};

#define LOGTYPE_NOPC 0x80

struct msm_rtb_platform_data {
	unsigned int size;
};

#if defined(CONFIG_MSM_RTB)
/*
                                            
 */
int uncached_logk_pc(enum logk_event_type log_type, void *caller,
				void *data);

/*
                                            
 */
int uncached_logk(enum logk_event_type log_type, void *data);

#define ETB_WAYPOINT  do { \
				BRANCH_TO_NEXT_ISTR; \
				nop(); \
				BRANCH_TO_NEXT_ISTR; \
				nop(); \
			} while (0)

#define BRANCH_TO_NEXT_ISTR  asm volatile("b .+4\n" : : : "memory")
/*
                                                                    
              
 */
#define LOG_BARRIER	do { \
				mb(); \
				isb();\
			 } while (0)
#else
#if defined(CONFIG_MACH_APQ8064_AWIFI)
static inline int uncached_logk_pc(enum logk_event_type log_type,
					void *caller,
					void *data) { return 0; }

static inline int uncached_logk(enum logk_event_type log_type,
					void *data) { return 1; }

#define ETB_WAYPOINT  do { \
				BRANCH_TO_NEXT_ISTR; \
				nop(); \
				BRANCH_TO_NEXT_ISTR; \
				nop(); \
			} while (0)

#define BRANCH_TO_NEXT_ISTR  asm volatile("b .+4\n" : : : "memory")
/*
                                                                            
                                             
 */
#define LOG_BARRIER	do { \
				mb(); \
				isb();\
			 } while (0)

#else
static inline int uncached_logk_pc(enum logk_event_type log_type,
                    void *caller,
                    void *data) { return 0; }

static inline int uncached_logk(enum logk_event_type log_type,
                    void *data) { return 0; }

#define ETB_WAYPOINT
#define BRANCH_TO_NEXT_ISTR
/*
                                                                            
                                             
 */
#define LOG_BARRIER     nop()
#endif
#endif
#endif
