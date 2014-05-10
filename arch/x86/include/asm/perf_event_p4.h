/*
                                             
 */

#ifndef PERF_EVENT_P4_H
#define PERF_EVENT_P4_H

#include <linux/cpu.h>
#include <linux/bitops.h>

/*
                                               
                                                  
                                                   
                                                    
                     
 */
#define ARCH_P4_TOTAL_ESCR	(46)
#define ARCH_P4_RESERVED_ESCR	(2) /*                                 */
#define ARCH_P4_MAX_ESCR	(ARCH_P4_TOTAL_ESCR - ARCH_P4_RESERVED_ESCR)
#define ARCH_P4_MAX_CCCR	(18)

#define ARCH_P4_CNTRVAL_BITS	(40)
#define ARCH_P4_CNTRVAL_MASK	((1ULL << ARCH_P4_CNTRVAL_BITS) - 1)
#define ARCH_P4_UNFLAGGED_BIT	((1ULL) << (ARCH_P4_CNTRVAL_BITS - 1))

#define P4_ESCR_EVENT_MASK	0x7e000000U
#define P4_ESCR_EVENT_SHIFT	25
#define P4_ESCR_EVENTMASK_MASK	0x01fffe00U
#define P4_ESCR_EVENTMASK_SHIFT	9
#define P4_ESCR_TAG_MASK	0x000001e0U
#define P4_ESCR_TAG_SHIFT	5
#define P4_ESCR_TAG_ENABLE	0x00000010U
#define P4_ESCR_T0_OS		0x00000008U
#define P4_ESCR_T0_USR		0x00000004U
#define P4_ESCR_T1_OS		0x00000002U
#define P4_ESCR_T1_USR		0x00000001U

#define P4_ESCR_EVENT(v)	((v) << P4_ESCR_EVENT_SHIFT)
#define P4_ESCR_EMASK(v)	((v) << P4_ESCR_EVENTMASK_SHIFT)
#define P4_ESCR_TAG(v)		((v) << P4_ESCR_TAG_SHIFT)

#define P4_CCCR_OVF			0x80000000U
#define P4_CCCR_CASCADE			0x40000000U
#define P4_CCCR_OVF_PMI_T0		0x04000000U
#define P4_CCCR_OVF_PMI_T1		0x08000000U
#define P4_CCCR_FORCE_OVF		0x02000000U
#define P4_CCCR_EDGE			0x01000000U
#define P4_CCCR_THRESHOLD_MASK		0x00f00000U
#define P4_CCCR_THRESHOLD_SHIFT		20
#define P4_CCCR_COMPLEMENT		0x00080000U
#define P4_CCCR_COMPARE			0x00040000U
#define P4_CCCR_ESCR_SELECT_MASK	0x0000e000U
#define P4_CCCR_ESCR_SELECT_SHIFT	13
#define P4_CCCR_ENABLE			0x00001000U
#define P4_CCCR_THREAD_SINGLE		0x00010000U
#define P4_CCCR_THREAD_BOTH		0x00020000U
#define P4_CCCR_THREAD_ANY		0x00030000U
#define P4_CCCR_RESERVED		0x00000fffU

#define P4_CCCR_THRESHOLD(v)		((v) << P4_CCCR_THRESHOLD_SHIFT)
#define P4_CCCR_ESEL(v)			((v) << P4_CCCR_ESCR_SELECT_SHIFT)

#define P4_GEN_ESCR_EMASK(class, name, bit)	\
	class##__##name = ((1 << bit) << P4_ESCR_EVENTMASK_SHIFT)
#define P4_ESCR_EMASK_BIT(class, name)		class##__##name

/*
                                              
                               
                                             
                                                 
  
                                                    
                                                       
                                                
  
                                                    
 */
#define p4_config_pack_escr(v)		(((u64)(v)) << 32)
#define p4_config_pack_cccr(v)		(((u64)(v)) & 0xffffffffULL)
#define p4_config_unpack_escr(v)	(((u64)(v)) >> 32)
#define p4_config_unpack_cccr(v)	(((u64)(v)) & 0xffffffffULL)

#define p4_config_unpack_emask(v)			\
	({						\
		u32 t = p4_config_unpack_escr((v));	\
		t = t &  P4_ESCR_EVENTMASK_MASK;	\
		t = t >> P4_ESCR_EVENTMASK_SHIFT;	\
		t;					\
	})

#define p4_config_unpack_event(v)			\
	({						\
		u32 t = p4_config_unpack_escr((v));	\
		t = t &  P4_ESCR_EVENT_MASK;		\
		t = t >> P4_ESCR_EVENT_SHIFT;		\
		t;					\
	})

#define P4_CONFIG_HT_SHIFT		63
#define P4_CONFIG_HT			(1ULL << P4_CONFIG_HT_SHIFT)

/*
                                            
                                             
                                          
                 
 */
#define P4_CONFIG_ALIASABLE		(1 << 9)

/*
                                           
 */
#define P4_CONFIG_MASK_ESCR		\
	P4_ESCR_EVENT_MASK	|	\
	P4_ESCR_EVENTMASK_MASK	|	\
	P4_ESCR_TAG_MASK	|	\
	P4_ESCR_TAG_ENABLE

#define P4_CONFIG_MASK_CCCR		\
	P4_CCCR_EDGE		|	\
	P4_CCCR_THRESHOLD_MASK	|	\
	P4_CCCR_COMPLEMENT	|	\
	P4_CCCR_COMPARE		|	\
	P4_CCCR_THREAD_ANY	|	\
	P4_CCCR_RESERVED

/*                                                       */
#define P4_CONFIG_MASK				  	  \
	(p4_config_pack_escr(P4_CONFIG_MASK_ESCR))	| \
	(p4_config_pack_cccr(P4_CONFIG_MASK_CCCR))

/*
                                                     
                                                        
 */
#define P4_CONFIG_EVENT_ALIAS_MASK			  \
	(p4_config_pack_escr(P4_CONFIG_MASK_ESCR)	| \
	 p4_config_pack_cccr(P4_CCCR_EDGE		| \
			     P4_CCCR_THRESHOLD_MASK	| \
			     P4_CCCR_COMPLEMENT		| \
			     P4_CCCR_COMPARE))

#define  P4_CONFIG_EVENT_ALIAS_IMMUTABLE_BITS		  \
	((P4_CONFIG_HT)					| \
	 p4_config_pack_escr(P4_ESCR_T0_OS		| \
			     P4_ESCR_T0_USR		| \
			     P4_ESCR_T1_OS		| \
			     P4_ESCR_T1_USR)		| \
	 p4_config_pack_cccr(P4_CCCR_OVF		| \
			     P4_CCCR_CASCADE		| \
			     P4_CCCR_FORCE_OVF		| \
			     P4_CCCR_THREAD_ANY		| \
			     P4_CCCR_OVF_PMI_T0		| \
			     P4_CCCR_OVF_PMI_T1		| \
			     P4_CONFIG_ALIASABLE))

static inline bool p4_is_event_cascaded(u64 config)
{
	u32 cccr = p4_config_unpack_cccr(config);
	return !!(cccr & P4_CCCR_CASCADE);
}

static inline int p4_ht_config_thread(u64 config)
{
	return !!(config & P4_CONFIG_HT);
}

static inline u64 p4_set_ht_bit(u64 config)
{
	return config | P4_CONFIG_HT;
}

static inline u64 p4_clear_ht_bit(u64 config)
{
	return config & ~P4_CONFIG_HT;
}

static inline int p4_ht_active(void)
{
#ifdef CONFIG_SMP
	return smp_num_siblings > 1;
#endif
	return 0;
}

static inline int p4_ht_thread(int cpu)
{
#ifdef CONFIG_SMP
	if (smp_num_siblings == 2)
		return cpu != cpumask_first(__get_cpu_var(cpu_sibling_map));
#endif
	return 0;
}

static inline int p4_should_swap_ts(u64 config, int cpu)
{
	return p4_ht_config_thread(config) ^ p4_ht_thread(cpu);
}

static inline u32 p4_default_cccr_conf(int cpu)
{
	/*
                                                 
                                                      
                                                    
  */
	u32 cccr = P4_CCCR_THREAD_ANY;

	if (!p4_ht_thread(cpu))
		cccr |= P4_CCCR_OVF_PMI_T0;
	else
		cccr |= P4_CCCR_OVF_PMI_T1;

	return cccr;
}

static inline u32 p4_default_escr_conf(int cpu, int exclude_os, int exclude_usr)
{
	u32 escr = 0;

	if (!p4_ht_thread(cpu)) {
		if (!exclude_os)
			escr |= P4_ESCR_T0_OS;
		if (!exclude_usr)
			escr |= P4_ESCR_T0_USR;
	} else {
		if (!exclude_os)
			escr |= P4_ESCR_T1_OS;
		if (!exclude_usr)
			escr |= P4_ESCR_T1_USR;
	}

	return escr;
}

/*
                                                             
                                                                
                                                             
                         
 */
enum P4_EVENTS {
	P4_EVENT_TC_DELIVER_MODE,
	P4_EVENT_BPU_FETCH_REQUEST,
	P4_EVENT_ITLB_REFERENCE,
	P4_EVENT_MEMORY_CANCEL,
	P4_EVENT_MEMORY_COMPLETE,
	P4_EVENT_LOAD_PORT_REPLAY,
	P4_EVENT_STORE_PORT_REPLAY,
	P4_EVENT_MOB_LOAD_REPLAY,
	P4_EVENT_PAGE_WALK_TYPE,
	P4_EVENT_BSQ_CACHE_REFERENCE,
	P4_EVENT_IOQ_ALLOCATION,
	P4_EVENT_IOQ_ACTIVE_ENTRIES,
	P4_EVENT_FSB_DATA_ACTIVITY,
	P4_EVENT_BSQ_ALLOCATION,
	P4_EVENT_BSQ_ACTIVE_ENTRIES,
	P4_EVENT_SSE_INPUT_ASSIST,
	P4_EVENT_PACKED_SP_UOP,
	P4_EVENT_PACKED_DP_UOP,
	P4_EVENT_SCALAR_SP_UOP,
	P4_EVENT_SCALAR_DP_UOP,
	P4_EVENT_64BIT_MMX_UOP,
	P4_EVENT_128BIT_MMX_UOP,
	P4_EVENT_X87_FP_UOP,
	P4_EVENT_TC_MISC,
	P4_EVENT_GLOBAL_POWER_EVENTS,
	P4_EVENT_TC_MS_XFER,
	P4_EVENT_UOP_QUEUE_WRITES,
	P4_EVENT_RETIRED_MISPRED_BRANCH_TYPE,
	P4_EVENT_RETIRED_BRANCH_TYPE,
	P4_EVENT_RESOURCE_STALL,
	P4_EVENT_WC_BUFFER,
	P4_EVENT_B2B_CYCLES,
	P4_EVENT_BNR,
	P4_EVENT_SNOOP,
	P4_EVENT_RESPONSE,
	P4_EVENT_FRONT_END_EVENT,
	P4_EVENT_EXECUTION_EVENT,
	P4_EVENT_REPLAY_EVENT,
	P4_EVENT_INSTR_RETIRED,
	P4_EVENT_UOPS_RETIRED,
	P4_EVENT_UOP_TYPE,
	P4_EVENT_BRANCH_RETIRED,
	P4_EVENT_MISPRED_BRANCH_RETIRED,
	P4_EVENT_X87_ASSIST,
	P4_EVENT_MACHINE_CLEAR,
	P4_EVENT_INSTR_COMPLETED,
};

#define P4_OPCODE(event)		event##_OPCODE
#define P4_OPCODE_ESEL(opcode)		((opcode & 0x00ff) >> 0)
#define P4_OPCODE_EVNT(opcode)		((opcode & 0xff00) >> 8)
#define P4_OPCODE_PACK(event, sel)	(((event) << 8) | sel)

/*
                                                      
                                            
  
                                                                  
                                                            
                                                            
                  
  
                                                               
                                                             
                    
 */
enum P4_EVENT_OPCODES {
	P4_OPCODE(P4_EVENT_TC_DELIVER_MODE)		= P4_OPCODE_PACK(0x01, 0x01),
	/*
                         
                         
  */

	P4_OPCODE(P4_EVENT_BPU_FETCH_REQUEST)		= P4_OPCODE_PACK(0x03, 0x00),
	/*
                          
                          
  */

	P4_OPCODE(P4_EVENT_ITLB_REFERENCE)		= P4_OPCODE_PACK(0x18, 0x03),
	/*
                           
                           
  */

	P4_OPCODE(P4_EVENT_MEMORY_CANCEL)		= P4_OPCODE_PACK(0x02, 0x05),
	/*
                          
                            
  */

	P4_OPCODE(P4_EVENT_MEMORY_COMPLETE)		= P4_OPCODE_PACK(0x08, 0x02),
	/*
                           
                             
  */

	P4_OPCODE(P4_EVENT_LOAD_PORT_REPLAY)		= P4_OPCODE_PACK(0x04, 0x02),
	/*
                           
                             
  */

	P4_OPCODE(P4_EVENT_STORE_PORT_REPLAY)		= P4_OPCODE_PACK(0x05, 0x02),
	/*
                           
                             
  */

	P4_OPCODE(P4_EVENT_MOB_LOAD_REPLAY)		= P4_OPCODE_PACK(0x03, 0x02),
	/*
                          
                          
  */

	P4_OPCODE(P4_EVENT_PAGE_WALK_TYPE)		= P4_OPCODE_PACK(0x01, 0x04),
	/*
                          
                          
  */

	P4_OPCODE(P4_EVENT_BSQ_CACHE_REFERENCE)		= P4_OPCODE_PACK(0x0c, 0x07),
	/*
                          
                          
  */

	P4_OPCODE(P4_EVENT_IOQ_ALLOCATION)		= P4_OPCODE_PACK(0x03, 0x06),
	/*
                          
                          
  */

	P4_OPCODE(P4_EVENT_IOQ_ACTIVE_ENTRIES)		= P4_OPCODE_PACK(0x1a, 0x06),
	/*
                          
  */

	P4_OPCODE(P4_EVENT_FSB_DATA_ACTIVITY)		= P4_OPCODE_PACK(0x17, 0x06),
	/*
                          
                          
  */

	P4_OPCODE(P4_EVENT_BSQ_ALLOCATION)		= P4_OPCODE_PACK(0x05, 0x07),
	/*
                          
  */

	P4_OPCODE(P4_EVENT_BSQ_ACTIVE_ENTRIES)		= P4_OPCODE_PACK(0x06, 0x07),
	/*
                                            
                          
  */

	P4_OPCODE(P4_EVENT_SSE_INPUT_ASSIST)		= P4_OPCODE_PACK(0x34, 0x01),
	/*
                           
                             
  */

	P4_OPCODE(P4_EVENT_PACKED_SP_UOP)		= P4_OPCODE_PACK(0x08, 0x01),
	/*
                           
                             
  */

	P4_OPCODE(P4_EVENT_PACKED_DP_UOP)		= P4_OPCODE_PACK(0x0c, 0x01),
	/*
                           
                             
  */

	P4_OPCODE(P4_EVENT_SCALAR_SP_UOP)		= P4_OPCODE_PACK(0x0a, 0x01),
	/*
                           
                             
  */

	P4_OPCODE(P4_EVENT_SCALAR_DP_UOP)		= P4_OPCODE_PACK(0x0e, 0x01),
	/*
                           
                             
  */

	P4_OPCODE(P4_EVENT_64BIT_MMX_UOP)		= P4_OPCODE_PACK(0x02, 0x01),
	/*
                           
                             
  */

	P4_OPCODE(P4_EVENT_128BIT_MMX_UOP)		= P4_OPCODE_PACK(0x1a, 0x01),
	/*
                           
                             
  */

	P4_OPCODE(P4_EVENT_X87_FP_UOP)			= P4_OPCODE_PACK(0x04, 0x01),
	/*
                           
                             
  */

	P4_OPCODE(P4_EVENT_TC_MISC)			= P4_OPCODE_PACK(0x06, 0x01),
	/*
                         
                         
  */

	P4_OPCODE(P4_EVENT_GLOBAL_POWER_EVENTS)		= P4_OPCODE_PACK(0x13, 0x06),
	/*
                          
                          
  */

	P4_OPCODE(P4_EVENT_TC_MS_XFER)			= P4_OPCODE_PACK(0x05, 0x00),
	/*
                         
                         
  */

	P4_OPCODE(P4_EVENT_UOP_QUEUE_WRITES)		= P4_OPCODE_PACK(0x09, 0x00),
	/*
                         
                         
  */

	P4_OPCODE(P4_EVENT_RETIRED_MISPRED_BRANCH_TYPE)	= P4_OPCODE_PACK(0x05, 0x02),
	/*
                           
                           
  */

	P4_OPCODE(P4_EVENT_RETIRED_BRANCH_TYPE)		= P4_OPCODE_PACK(0x04, 0x02),
	/*
                           
                           
  */

	P4_OPCODE(P4_EVENT_RESOURCE_STALL)		= P4_OPCODE_PACK(0x01, 0x01),
	/*
                                
                                
  */

	P4_OPCODE(P4_EVENT_WC_BUFFER)			= P4_OPCODE_PACK(0x05, 0x05),
	/*
                          
                            
  */

	P4_OPCODE(P4_EVENT_B2B_CYCLES)			= P4_OPCODE_PACK(0x16, 0x03),
	/*
                          
                          
  */

	P4_OPCODE(P4_EVENT_BNR)				= P4_OPCODE_PACK(0x08, 0x03),
	/*
                          
                          
  */

	P4_OPCODE(P4_EVENT_SNOOP)			= P4_OPCODE_PACK(0x06, 0x03),
	/*
                          
                          
  */

	P4_OPCODE(P4_EVENT_RESPONSE)			= P4_OPCODE_PACK(0x04, 0x03),
	/*
                          
                          
  */

	P4_OPCODE(P4_EVENT_FRONT_END_EVENT)		= P4_OPCODE_PACK(0x08, 0x05),
	/*
                                
                                
  */

	P4_OPCODE(P4_EVENT_EXECUTION_EVENT)		= P4_OPCODE_PACK(0x0c, 0x05),
	/*
                                
                                
  */

	P4_OPCODE(P4_EVENT_REPLAY_EVENT)		= P4_OPCODE_PACK(0x09, 0x05),
	/*
                                
                                
  */

	P4_OPCODE(P4_EVENT_INSTR_RETIRED)		= P4_OPCODE_PACK(0x02, 0x04),
	/*
                                
                                
  */

	P4_OPCODE(P4_EVENT_UOPS_RETIRED)		= P4_OPCODE_PACK(0x01, 0x04),
	/*
                                
                                
  */

	P4_OPCODE(P4_EVENT_UOP_TYPE)			= P4_OPCODE_PACK(0x02, 0x02),
	/*
                                
                                
  */

	P4_OPCODE(P4_EVENT_BRANCH_RETIRED)		= P4_OPCODE_PACK(0x06, 0x05),
	/*
                                
                                
  */

	P4_OPCODE(P4_EVENT_MISPRED_BRANCH_RETIRED)	= P4_OPCODE_PACK(0x03, 0x04),
	/*
                                
                                
  */

	P4_OPCODE(P4_EVENT_X87_ASSIST)			= P4_OPCODE_PACK(0x03, 0x05),
	/*
                                
                                
  */

	P4_OPCODE(P4_EVENT_MACHINE_CLEAR)		= P4_OPCODE_PACK(0x02, 0x05),
	/*
                                
                                
  */

	P4_OPCODE(P4_EVENT_INSTR_COMPLETED)		= P4_OPCODE_PACK(0x07, 0x04),
	/*
                                
                                
  */
};

/*
                                                   
                                         
  
                                                  
 */
enum P4_ESCR_EMASKS {
	P4_GEN_ESCR_EMASK(P4_EVENT_TC_DELIVER_MODE, DD, 0),
	P4_GEN_ESCR_EMASK(P4_EVENT_TC_DELIVER_MODE, DB, 1),
	P4_GEN_ESCR_EMASK(P4_EVENT_TC_DELIVER_MODE, DI, 2),
	P4_GEN_ESCR_EMASK(P4_EVENT_TC_DELIVER_MODE, BD, 3),
	P4_GEN_ESCR_EMASK(P4_EVENT_TC_DELIVER_MODE, BB, 4),
	P4_GEN_ESCR_EMASK(P4_EVENT_TC_DELIVER_MODE, BI, 5),
	P4_GEN_ESCR_EMASK(P4_EVENT_TC_DELIVER_MODE, ID, 6),

	P4_GEN_ESCR_EMASK(P4_EVENT_BPU_FETCH_REQUEST, TCMISS, 0),

	P4_GEN_ESCR_EMASK(P4_EVENT_ITLB_REFERENCE, HIT, 0),
	P4_GEN_ESCR_EMASK(P4_EVENT_ITLB_REFERENCE, MISS, 1),
	P4_GEN_ESCR_EMASK(P4_EVENT_ITLB_REFERENCE, HIT_UK, 2),

	P4_GEN_ESCR_EMASK(P4_EVENT_MEMORY_CANCEL, ST_RB_FULL, 2),
	P4_GEN_ESCR_EMASK(P4_EVENT_MEMORY_CANCEL, 64K_CONF, 3),

	P4_GEN_ESCR_EMASK(P4_EVENT_MEMORY_COMPLETE, LSC, 0),
	P4_GEN_ESCR_EMASK(P4_EVENT_MEMORY_COMPLETE, SSC, 1),

	P4_GEN_ESCR_EMASK(P4_EVENT_LOAD_PORT_REPLAY, SPLIT_LD, 1),

	P4_GEN_ESCR_EMASK(P4_EVENT_STORE_PORT_REPLAY, SPLIT_ST, 1),

	P4_GEN_ESCR_EMASK(P4_EVENT_MOB_LOAD_REPLAY, NO_STA, 1),
	P4_GEN_ESCR_EMASK(P4_EVENT_MOB_LOAD_REPLAY, NO_STD, 3),
	P4_GEN_ESCR_EMASK(P4_EVENT_MOB_LOAD_REPLAY, PARTIAL_DATA, 4),
	P4_GEN_ESCR_EMASK(P4_EVENT_MOB_LOAD_REPLAY, UNALGN_ADDR, 5),

	P4_GEN_ESCR_EMASK(P4_EVENT_PAGE_WALK_TYPE, DTMISS, 0),
	P4_GEN_ESCR_EMASK(P4_EVENT_PAGE_WALK_TYPE, ITMISS, 1),

	P4_GEN_ESCR_EMASK(P4_EVENT_BSQ_CACHE_REFERENCE, RD_2ndL_HITS, 0),
	P4_GEN_ESCR_EMASK(P4_EVENT_BSQ_CACHE_REFERENCE, RD_2ndL_HITE, 1),
	P4_GEN_ESCR_EMASK(P4_EVENT_BSQ_CACHE_REFERENCE, RD_2ndL_HITM, 2),
	P4_GEN_ESCR_EMASK(P4_EVENT_BSQ_CACHE_REFERENCE, RD_3rdL_HITS, 3),
	P4_GEN_ESCR_EMASK(P4_EVENT_BSQ_CACHE_REFERENCE, RD_3rdL_HITE, 4),
	P4_GEN_ESCR_EMASK(P4_EVENT_BSQ_CACHE_REFERENCE, RD_3rdL_HITM, 5),
	P4_GEN_ESCR_EMASK(P4_EVENT_BSQ_CACHE_REFERENCE, RD_2ndL_MISS, 8),
	P4_GEN_ESCR_EMASK(P4_EVENT_BSQ_CACHE_REFERENCE, RD_3rdL_MISS, 9),
	P4_GEN_ESCR_EMASK(P4_EVENT_BSQ_CACHE_REFERENCE, WR_2ndL_MISS, 10),

	P4_GEN_ESCR_EMASK(P4_EVENT_IOQ_ALLOCATION, DEFAULT, 0),
	P4_GEN_ESCR_EMASK(P4_EVENT_IOQ_ALLOCATION, ALL_READ, 5),
	P4_GEN_ESCR_EMASK(P4_EVENT_IOQ_ALLOCATION, ALL_WRITE, 6),
	P4_GEN_ESCR_EMASK(P4_EVENT_IOQ_ALLOCATION, MEM_UC, 7),
	P4_GEN_ESCR_EMASK(P4_EVENT_IOQ_ALLOCATION, MEM_WC, 8),
	P4_GEN_ESCR_EMASK(P4_EVENT_IOQ_ALLOCATION, MEM_WT, 9),
	P4_GEN_ESCR_EMASK(P4_EVENT_IOQ_ALLOCATION, MEM_WP, 10),
	P4_GEN_ESCR_EMASK(P4_EVENT_IOQ_ALLOCATION, MEM_WB, 11),
	P4_GEN_ESCR_EMASK(P4_EVENT_IOQ_ALLOCATION, OWN, 13),
	P4_GEN_ESCR_EMASK(P4_EVENT_IOQ_ALLOCATION, OTHER, 14),
	P4_GEN_ESCR_EMASK(P4_EVENT_IOQ_ALLOCATION, PREFETCH, 15),

	P4_GEN_ESCR_EMASK(P4_EVENT_IOQ_ACTIVE_ENTRIES, DEFAULT, 0),
	P4_GEN_ESCR_EMASK(P4_EVENT_IOQ_ACTIVE_ENTRIES, ALL_READ, 5),
	P4_GEN_ESCR_EMASK(P4_EVENT_IOQ_ACTIVE_ENTRIES, ALL_WRITE, 6),
	P4_GEN_ESCR_EMASK(P4_EVENT_IOQ_ACTIVE_ENTRIES, MEM_UC, 7),
	P4_GEN_ESCR_EMASK(P4_EVENT_IOQ_ACTIVE_ENTRIES, MEM_WC, 8),
	P4_GEN_ESCR_EMASK(P4_EVENT_IOQ_ACTIVE_ENTRIES, MEM_WT, 9),
	P4_GEN_ESCR_EMASK(P4_EVENT_IOQ_ACTIVE_ENTRIES, MEM_WP, 10),
	P4_GEN_ESCR_EMASK(P4_EVENT_IOQ_ACTIVE_ENTRIES, MEM_WB, 11),
	P4_GEN_ESCR_EMASK(P4_EVENT_IOQ_ACTIVE_ENTRIES, OWN, 13),
	P4_GEN_ESCR_EMASK(P4_EVENT_IOQ_ACTIVE_ENTRIES, OTHER, 14),
	P4_GEN_ESCR_EMASK(P4_EVENT_IOQ_ACTIVE_ENTRIES, PREFETCH, 15),

	P4_GEN_ESCR_EMASK(P4_EVENT_FSB_DATA_ACTIVITY, DRDY_DRV, 0),
	P4_GEN_ESCR_EMASK(P4_EVENT_FSB_DATA_ACTIVITY, DRDY_OWN, 1),
	P4_GEN_ESCR_EMASK(P4_EVENT_FSB_DATA_ACTIVITY, DRDY_OTHER, 2),
	P4_GEN_ESCR_EMASK(P4_EVENT_FSB_DATA_ACTIVITY, DBSY_DRV, 3),
	P4_GEN_ESCR_EMASK(P4_EVENT_FSB_DATA_ACTIVITY, DBSY_OWN, 4),
	P4_GEN_ESCR_EMASK(P4_EVENT_FSB_DATA_ACTIVITY, DBSY_OTHER, 5),

	P4_GEN_ESCR_EMASK(P4_EVENT_BSQ_ALLOCATION, REQ_TYPE0, 0),
	P4_GEN_ESCR_EMASK(P4_EVENT_BSQ_ALLOCATION, REQ_TYPE1, 1),
	P4_GEN_ESCR_EMASK(P4_EVENT_BSQ_ALLOCATION, REQ_LEN0, 2),
	P4_GEN_ESCR_EMASK(P4_EVENT_BSQ_ALLOCATION, REQ_LEN1, 3),
	P4_GEN_ESCR_EMASK(P4_EVENT_BSQ_ALLOCATION, REQ_IO_TYPE, 5),
	P4_GEN_ESCR_EMASK(P4_EVENT_BSQ_ALLOCATION, REQ_LOCK_TYPE, 6),
	P4_GEN_ESCR_EMASK(P4_EVENT_BSQ_ALLOCATION, REQ_CACHE_TYPE, 7),
	P4_GEN_ESCR_EMASK(P4_EVENT_BSQ_ALLOCATION, REQ_SPLIT_TYPE, 8),
	P4_GEN_ESCR_EMASK(P4_EVENT_BSQ_ALLOCATION, REQ_DEM_TYPE, 9),
	P4_GEN_ESCR_EMASK(P4_EVENT_BSQ_ALLOCATION, REQ_ORD_TYPE, 10),
	P4_GEN_ESCR_EMASK(P4_EVENT_BSQ_ALLOCATION, MEM_TYPE0, 11),
	P4_GEN_ESCR_EMASK(P4_EVENT_BSQ_ALLOCATION, MEM_TYPE1, 12),
	P4_GEN_ESCR_EMASK(P4_EVENT_BSQ_ALLOCATION, MEM_TYPE2, 13),

	P4_GEN_ESCR_EMASK(P4_EVENT_BSQ_ACTIVE_ENTRIES, REQ_TYPE0, 0),
	P4_GEN_ESCR_EMASK(P4_EVENT_BSQ_ACTIVE_ENTRIES, REQ_TYPE1, 1),
	P4_GEN_ESCR_EMASK(P4_EVENT_BSQ_ACTIVE_ENTRIES, REQ_LEN0, 2),
	P4_GEN_ESCR_EMASK(P4_EVENT_BSQ_ACTIVE_ENTRIES, REQ_LEN1, 3),
	P4_GEN_ESCR_EMASK(P4_EVENT_BSQ_ACTIVE_ENTRIES, REQ_IO_TYPE, 5),
	P4_GEN_ESCR_EMASK(P4_EVENT_BSQ_ACTIVE_ENTRIES, REQ_LOCK_TYPE, 6),
	P4_GEN_ESCR_EMASK(P4_EVENT_BSQ_ACTIVE_ENTRIES, REQ_CACHE_TYPE, 7),
	P4_GEN_ESCR_EMASK(P4_EVENT_BSQ_ACTIVE_ENTRIES, REQ_SPLIT_TYPE, 8),
	P4_GEN_ESCR_EMASK(P4_EVENT_BSQ_ACTIVE_ENTRIES, REQ_DEM_TYPE, 9),
	P4_GEN_ESCR_EMASK(P4_EVENT_BSQ_ACTIVE_ENTRIES, REQ_ORD_TYPE, 10),
	P4_GEN_ESCR_EMASK(P4_EVENT_BSQ_ACTIVE_ENTRIES, MEM_TYPE0, 11),
	P4_GEN_ESCR_EMASK(P4_EVENT_BSQ_ACTIVE_ENTRIES, MEM_TYPE1, 12),
	P4_GEN_ESCR_EMASK(P4_EVENT_BSQ_ACTIVE_ENTRIES, MEM_TYPE2, 13),

	P4_GEN_ESCR_EMASK(P4_EVENT_SSE_INPUT_ASSIST, ALL, 15),

	P4_GEN_ESCR_EMASK(P4_EVENT_PACKED_SP_UOP, ALL, 15),

	P4_GEN_ESCR_EMASK(P4_EVENT_PACKED_DP_UOP, ALL, 15),

	P4_GEN_ESCR_EMASK(P4_EVENT_SCALAR_SP_UOP, ALL, 15),

	P4_GEN_ESCR_EMASK(P4_EVENT_SCALAR_DP_UOP, ALL, 15),

	P4_GEN_ESCR_EMASK(P4_EVENT_64BIT_MMX_UOP, ALL, 15),

	P4_GEN_ESCR_EMASK(P4_EVENT_128BIT_MMX_UOP, ALL, 15),

	P4_GEN_ESCR_EMASK(P4_EVENT_X87_FP_UOP, ALL, 15),

	P4_GEN_ESCR_EMASK(P4_EVENT_TC_MISC, FLUSH, 4),

	P4_GEN_ESCR_EMASK(P4_EVENT_GLOBAL_POWER_EVENTS, RUNNING, 0),

	P4_GEN_ESCR_EMASK(P4_EVENT_TC_MS_XFER, CISC, 0),

	P4_GEN_ESCR_EMASK(P4_EVENT_UOP_QUEUE_WRITES, FROM_TC_BUILD, 0),
	P4_GEN_ESCR_EMASK(P4_EVENT_UOP_QUEUE_WRITES, FROM_TC_DELIVER, 1),
	P4_GEN_ESCR_EMASK(P4_EVENT_UOP_QUEUE_WRITES, FROM_ROM, 2),

	P4_GEN_ESCR_EMASK(P4_EVENT_RETIRED_MISPRED_BRANCH_TYPE, CONDITIONAL, 1),
	P4_GEN_ESCR_EMASK(P4_EVENT_RETIRED_MISPRED_BRANCH_TYPE, CALL, 2),
	P4_GEN_ESCR_EMASK(P4_EVENT_RETIRED_MISPRED_BRANCH_TYPE, RETURN, 3),
	P4_GEN_ESCR_EMASK(P4_EVENT_RETIRED_MISPRED_BRANCH_TYPE, INDIRECT, 4),

	P4_GEN_ESCR_EMASK(P4_EVENT_RETIRED_BRANCH_TYPE, CONDITIONAL, 1),
	P4_GEN_ESCR_EMASK(P4_EVENT_RETIRED_BRANCH_TYPE, CALL, 2),
	P4_GEN_ESCR_EMASK(P4_EVENT_RETIRED_BRANCH_TYPE, RETURN, 3),
	P4_GEN_ESCR_EMASK(P4_EVENT_RETIRED_BRANCH_TYPE, INDIRECT, 4),

	P4_GEN_ESCR_EMASK(P4_EVENT_RESOURCE_STALL, SBFULL, 5),

	P4_GEN_ESCR_EMASK(P4_EVENT_WC_BUFFER, WCB_EVICTS, 0),
	P4_GEN_ESCR_EMASK(P4_EVENT_WC_BUFFER, WCB_FULL_EVICTS, 1),

	P4_GEN_ESCR_EMASK(P4_EVENT_FRONT_END_EVENT, NBOGUS, 0),
	P4_GEN_ESCR_EMASK(P4_EVENT_FRONT_END_EVENT, BOGUS, 1),

	P4_GEN_ESCR_EMASK(P4_EVENT_EXECUTION_EVENT, NBOGUS0, 0),
	P4_GEN_ESCR_EMASK(P4_EVENT_EXECUTION_EVENT, NBOGUS1, 1),
	P4_GEN_ESCR_EMASK(P4_EVENT_EXECUTION_EVENT, NBOGUS2, 2),
	P4_GEN_ESCR_EMASK(P4_EVENT_EXECUTION_EVENT, NBOGUS3, 3),
	P4_GEN_ESCR_EMASK(P4_EVENT_EXECUTION_EVENT, BOGUS0, 4),
	P4_GEN_ESCR_EMASK(P4_EVENT_EXECUTION_EVENT, BOGUS1, 5),
	P4_GEN_ESCR_EMASK(P4_EVENT_EXECUTION_EVENT, BOGUS2, 6),
	P4_GEN_ESCR_EMASK(P4_EVENT_EXECUTION_EVENT, BOGUS3, 7),

	P4_GEN_ESCR_EMASK(P4_EVENT_REPLAY_EVENT, NBOGUS, 0),
	P4_GEN_ESCR_EMASK(P4_EVENT_REPLAY_EVENT, BOGUS, 1),

	P4_GEN_ESCR_EMASK(P4_EVENT_INSTR_RETIRED, NBOGUSNTAG, 0),
	P4_GEN_ESCR_EMASK(P4_EVENT_INSTR_RETIRED, NBOGUSTAG, 1),
	P4_GEN_ESCR_EMASK(P4_EVENT_INSTR_RETIRED, BOGUSNTAG, 2),
	P4_GEN_ESCR_EMASK(P4_EVENT_INSTR_RETIRED, BOGUSTAG, 3),

	P4_GEN_ESCR_EMASK(P4_EVENT_UOPS_RETIRED, NBOGUS, 0),
	P4_GEN_ESCR_EMASK(P4_EVENT_UOPS_RETIRED, BOGUS, 1),

	P4_GEN_ESCR_EMASK(P4_EVENT_UOP_TYPE, TAGLOADS, 1),
	P4_GEN_ESCR_EMASK(P4_EVENT_UOP_TYPE, TAGSTORES, 2),

	P4_GEN_ESCR_EMASK(P4_EVENT_BRANCH_RETIRED, MMNP, 0),
	P4_GEN_ESCR_EMASK(P4_EVENT_BRANCH_RETIRED, MMNM, 1),
	P4_GEN_ESCR_EMASK(P4_EVENT_BRANCH_RETIRED, MMTP, 2),
	P4_GEN_ESCR_EMASK(P4_EVENT_BRANCH_RETIRED, MMTM, 3),

	P4_GEN_ESCR_EMASK(P4_EVENT_MISPRED_BRANCH_RETIRED, NBOGUS, 0),

	P4_GEN_ESCR_EMASK(P4_EVENT_X87_ASSIST, FPSU, 0),
	P4_GEN_ESCR_EMASK(P4_EVENT_X87_ASSIST, FPSO, 1),
	P4_GEN_ESCR_EMASK(P4_EVENT_X87_ASSIST, POAO, 2),
	P4_GEN_ESCR_EMASK(P4_EVENT_X87_ASSIST, POAU, 3),
	P4_GEN_ESCR_EMASK(P4_EVENT_X87_ASSIST, PREA, 4),

	P4_GEN_ESCR_EMASK(P4_EVENT_MACHINE_CLEAR, CLEAR, 0),
	P4_GEN_ESCR_EMASK(P4_EVENT_MACHINE_CLEAR, MOCLEAR, 1),
	P4_GEN_ESCR_EMASK(P4_EVENT_MACHINE_CLEAR, SMCLEAR, 2),

	P4_GEN_ESCR_EMASK(P4_EVENT_INSTR_COMPLETED, NBOGUS, 0),
	P4_GEN_ESCR_EMASK(P4_EVENT_INSTR_COMPLETED, BOGUS, 1),
};

/*
                                                  
                                         
 */
#define P4_PEBS_CONFIG_ENABLE		(1 << 7)
#define P4_PEBS_CONFIG_UOP_TAG		(1 << 8)
#define P4_PEBS_CONFIG_METRIC_MASK	0x3f
#define P4_PEBS_CONFIG_MASK		0xff

/*
                                              
                                                     
 */
#define P4_PEBS_ENABLE			0x02000000U
#define P4_PEBS_ENABLE_UOP_TAG		0x01000000U

#define p4_config_unpack_metric(v)	(((u64)(v)) & P4_PEBS_CONFIG_METRIC_MASK)
#define p4_config_unpack_pebs(v)	(((u64)(v)) & P4_PEBS_CONFIG_MASK)

#define p4_config_pebs_has(v, mask)	(p4_config_unpack_pebs(v) & (mask))

enum P4_PEBS_METRIC {
	P4_PEBS_METRIC__none,

	P4_PEBS_METRIC__1stl_cache_load_miss_retired,
	P4_PEBS_METRIC__2ndl_cache_load_miss_retired,
	P4_PEBS_METRIC__dtlb_load_miss_retired,
	P4_PEBS_METRIC__dtlb_store_miss_retired,
	P4_PEBS_METRIC__dtlb_all_miss_retired,
	P4_PEBS_METRIC__tagged_mispred_branch,
	P4_PEBS_METRIC__mob_load_replay_retired,
	P4_PEBS_METRIC__split_load_retired,
	P4_PEBS_METRIC__split_store_retired,

	P4_PEBS_METRIC__max
};

/*
                                                      
  
                                                   
                                                        
                                                    
                                                     
  
                                                      
                                                         
                                                          
                                                      
                                                 
  
                                                       
                                                       
                                                      
                                                    
                                         
  
                                                   
                                                      
             
  
                 
                 
                                
                                        
                                                 
                                                      
                          
                    
                       
                      
                 
                                                    
                                                     
                                                     
                                        
                                                  
                                              
  
                  
                  
                                                 
                                                
                                                 
                                                
                       
                      
                                                          
                           
                                                    
 */

#endif /*                 */

