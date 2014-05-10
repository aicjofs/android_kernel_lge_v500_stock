/*
 * Compressed RAM block device
 *
 * Copyright (C) 2008, 2009, 2010  Nitin Gupta
 *
 * This code is released using a dual license strategy: BSD/GPL
 * You can choose the licence that better fits your requirements.
 *
 * Released under the terms of 3-clause BSD License
 * Released under the terms of GNU General Public License Version 2.0
 *
 * Project home: http://compcache.googlecode.com
 */

#ifndef _ZRAM_DRV_H_
#define _ZRAM_DRV_H_

#include <linux/spinlock.h>
#include <linux/mutex.h>

#include "../zsmalloc/zsmalloc.h"

/*
                                              
                                                  
 */
static const unsigned max_num_devices = 32;

/*
                                                 
  
                                                               
                                                              
 */
struct zobj_header {
#if 0
	u32 table_idx;
#endif
};

/*                           */

/*                                          */
static const unsigned default_disksize_perc_ram = 25;

/*
                                                           
                          
 */
static const size_t max_zpage_size = PAGE_SIZE / 4 * 3;

/*
                                                      
                                                   
                                                      
 */

/*                              */

#define SECTOR_SHIFT		9
#define SECTOR_SIZE		(1 << SECTOR_SHIFT)
#define SECTORS_PER_PAGE_SHIFT	(PAGE_SHIFT - SECTOR_SHIFT)
#define SECTORS_PER_PAGE	(1 << SECTORS_PER_PAGE_SHIFT)
#define ZRAM_LOGICAL_BLOCK_SHIFT 12
#define ZRAM_LOGICAL_BLOCK_SIZE	(1 << ZRAM_LOGICAL_BLOCK_SHIFT)
#define ZRAM_SECTOR_PER_LOGICAL_BLOCK	\
	(1 << (ZRAM_LOGICAL_BLOCK_SHIFT - SECTOR_SHIFT))

/*                                             */
enum zram_pageflags {
	/*                             */
	ZRAM_UNCOMPRESSED,

	/*                                 */
	ZRAM_ZERO,

	__NR_ZRAM_PAGEFLAGS,
};

/*                   */

/*                              */
struct table {
	void *handle;
	u16 size;	/*                                */
	u8 count;	/*                                 */
	u8 flags;
} __attribute__((aligned(4)));

struct zram_stats {
	u64 compr_size;		/*                                 */
	u64 num_reads;		/*                     */
	u64 num_writes;		/*        */
	u64 failed_reads;	/*                      */
	u64 failed_writes;	/*                                   */
	u64 invalid_io;		/*                               */
	u64 notify_free;	/*                                     */
	u32 pages_zero;		/*                          */
	u32 pages_stored;	/*                               */
	u32 good_compress;	/*                                        */
	u32 pages_expand;	/*                           */
};

struct zram {
	struct zs_pool *mem_pool;
	void *compress_workmem;
	void *compress_buffer;
	struct table *table;
	spinlock_t stat64_lock;	/*                      */
	struct rw_semaphore lock; /*                                      
                                            */
	struct request_queue *queue;
	struct gendisk *disk;
	int init_done;
	/*                                                                    */
	struct rw_semaphore init_lock;
	/*
                                                               
                           
  */
	u64 disksize;	/*       */

	struct zram_stats stats;
};

extern struct zram *zram_devices;
unsigned int zram_get_num_devices(void);
#ifdef CONFIG_SYSFS
extern struct attribute_group zram_disk_attr_group;
#endif

extern int zram_init_device(struct zram *zram);
extern void __zram_reset_device(struct zram *zram);

#endif
