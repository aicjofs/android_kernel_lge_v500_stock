/*
 * zsmalloc memory allocator
 *
 * Copyright (C) 2011  Nitin Gupta
 *
 * This code is released using a dual license strategy: BSD/GPL
 * You can choose the license that better fits your requirements.
 *
 * Released under the terms of 3-clause BSD License
 * Released under the terms of GNU General Public License Version 2.0
 */

#ifndef _ZS_MALLOC_INT_H_
#define _ZS_MALLOC_INT_H_

#include <linux/kernel.h>
#include <linux/spinlock.h>
#include <linux/types.h>

/*
                                                                          
                                                                         
                                                                            
                                       
 */
#define ZS_ALIGN		8

/*
                                                                            
                                                       
 */
#define ZS_MAX_ZSPAGE_ORDER 2
#define ZS_MAX_PAGES_PER_ZSPAGE (_AC(1, UL) << ZS_MAX_ZSPAGE_ORDER)

/*
                                                   
                                   
  
                                                         
                                                             
                                      
  
                                                                  
 */

#ifndef MAX_PHYSMEM_BITS
#ifdef CONFIG_HIGHMEM64G
#define MAX_PHYSMEM_BITS 36
#else /*                    */
/*
                                                                           
                
 */
#define MAX_PHYSMEM_BITS BITS_PER_LONG
#endif
#endif
#define _PFN_BITS		(MAX_PHYSMEM_BITS - PAGE_SHIFT)
#define OBJ_INDEX_BITS	(BITS_PER_LONG - _PFN_BITS)
#define OBJ_INDEX_MASK	((_AC(1, UL) << OBJ_INDEX_BITS) - 1)

#define MAX(a, b) ((a) >= (b) ? (a) : (b))
/*                                                */
#define ZS_MIN_ALLOC_SIZE \
	MAX(32, (ZS_MAX_PAGES_PER_ZSPAGE << PAGE_SHIFT >> OBJ_INDEX_BITS))
#define ZS_MAX_ALLOC_SIZE	PAGE_SIZE

/*
                                                                        
                   
                                                                           
                                 
                                                                      
                                                                   
                                                                         
                                                                       
  
                                                                          
                  
 */
#define ZS_SIZE_CLASS_DELTA	16
#define ZS_SIZE_CLASSES		((ZS_MAX_ALLOC_SIZE - ZS_MIN_ALLOC_SIZE) / \
					ZS_SIZE_CLASS_DELTA + 1)

/*
                                                                 
 */
enum fullness_group {
	ZS_ALMOST_FULL,
	ZS_ALMOST_EMPTY,
	_ZS_NR_FULLNESS_GROUPS,

	ZS_EMPTY,
	ZS_FULL
};

/*
                                                           
                    
                                  
                                               
                                
  
                                  
                                
                       
                       
  
                              
 */
static const int fullness_threshold_frac = 4;

struct mapping_area {
	struct vm_struct *vm;
	pte_t *vm_ptes[2];
	char *vm_addr;
};

struct size_class {
	/*
                                                          
                
  */
	int size;
	unsigned int index;

	/*                                                               */
	int zspage_order;

	spinlock_t lock;

	/*       */
	u64 pages_allocated;

	struct page *fullness_list[_ZS_NR_FULLNESS_GROUPS];
};

/*
                                                           
                                                                  
  
                                                             
 */
struct link_free {
	/*                                                    */
	void *next;
};

struct zs_pool {
	struct size_class size_class[ZS_SIZE_CLASSES];

	gfp_t flags;	/*                                         */
	const char *name;
};

#endif
