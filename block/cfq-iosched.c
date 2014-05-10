/*
 *  CFQ, or complete fairness queueing, disk scheduler.
 *
 *  Based on ideas from a previously unfinished io
 *  scheduler (round robin per-process disk scheduling) and Andrea Arcangeli.
 *
 *  Copyright (C) 2003 Jens Axboe <axboe@kernel.dk>
 */
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/blkdev.h>
#include <linux/elevator.h>
#include <linux/jiffies.h>
#include <linux/rbtree.h>
#include <linux/ioprio.h>
#include <linux/blktrace_api.h>
#include "blk.h"
#include "cfq.h"

/*
           
 */
/*                                   */
static const int cfq_quantum = 8;
static const int cfq_fifo_expire[2] = { HZ / 4, HZ / 8 };
/*                                */
static const int cfq_back_max = 16 * 1024;
/*                             */
static const int cfq_back_penalty = 2;
static const int cfq_slice_sync = HZ / 10;
static int cfq_slice_async = HZ / 25;
static const int cfq_slice_async_rq = 2;
static int cfq_slice_idle = HZ / 125;
static int cfq_group_idle = HZ / 125;
static const int cfq_target_latency = HZ * 3/10; /*        */
static const int cfq_hist_divisor = 4;

/*
                                  
 */
#define CFQ_IDLE_DELAY		(HZ / 5)

/*
                                                        
 */
#define CFQ_MIN_TT		(2)

#define CFQ_SLICE_SCALE		(5)
#define CFQ_HW_QUEUE_MIN	(5)
#define CFQ_SERVICE_SHIFT       12

#define CFQQ_SEEK_THR		(sector_t)(8 * 100)
#define CFQQ_CLOSE_THR		(sector_t)(8 * 1024)
#define CFQQ_SECT_THR_NONROT	(sector_t)(2 * 32)
#define CFQQ_SEEKY(cfqq)	(hweight32(cfqq->seek_history) > 32/8)

#define RQ_CIC(rq)		icq_to_cic((rq)->elv.icq)
#define RQ_CFQQ(rq)		(struct cfq_queue *) ((rq)->elv.priv[0])
#define RQ_CFQG(rq)		(struct cfq_group *) ((rq)->elv.priv[1])

static struct kmem_cache *cfq_pool;

#define CFQ_PRIO_LISTS		IOPRIO_BE_NR
#define cfq_class_idle(cfqq)	((cfqq)->ioprio_class == IOPRIO_CLASS_IDLE)
#define cfq_class_rt(cfqq)	((cfqq)->ioprio_class == IOPRIO_CLASS_RT)

#define sample_valid(samples)	((samples) > 80)
#define rb_entry_cfqg(node)	rb_entry((node), struct cfq_group, rb_node)

struct cfq_ttime {
	unsigned long last_end_request;

	unsigned long ttime_total;
	unsigned long ttime_samples;
	unsigned long ttime_mean;
};

/*
                                                                  
                                                                    
                                                                       
                                                          
 */
struct cfq_rb_root {
	struct rb_root rb;
	struct rb_node *left;
	unsigned count;
	unsigned total_weight;
	u64 min_vdisktime;
	struct cfq_ttime ttime;
};
#define CFQ_RB_ROOT	(struct cfq_rb_root) { .rb = RB_ROOT, \
			.ttime = {.last_end_request = jiffies,},}

/*
                                 
 */
struct cfq_queue {
	/*                 */
	int ref;
	/*                                */
	unsigned int flags;
	/*                 */
	struct cfq_data *cfqd;
	/*                     */
	struct rb_node rb_node;
	/*                  */
	unsigned long rb_key;
	/*                  */
	struct rb_node p_node;
	/*                                     */
	struct rb_root *p_root;
	/*                                 */
	struct rb_root sort_list;
	/*                                              */
	struct request *next_rq;
	/*                              */
	int queued[2];
	/*                              */
	int allocated[2];
	/*                                    */
	struct list_head fifo;

	/*                                                             */
	unsigned long dispatch_start;
	unsigned int allocated_slice;
	unsigned int slice_dispatch;
	/*                                                                 */
	unsigned long slice_start;
	unsigned long slice_end;
	long slice_resid;

	/*                           */
	int prio_pending;
	/*                                                                   */
	int dispatched;

	/*                       */
	unsigned short ioprio, org_ioprio;
	unsigned short ioprio_class;

	pid_t pid;

	u32 seek_history;
	sector_t last_request_pos;

	struct cfq_rb_root *service_tree;
	struct cfq_queue *new_cfqq;
	struct cfq_group *cfqg;
	/*                                                                  */
	unsigned long nr_sectors;
};

/*
                                    
                                                       
 */
enum wl_prio_t {
	BE_WORKLOAD = 0,
	RT_WORKLOAD = 1,
	IDLE_WORKLOAD = 2,
	CFQ_PRIO_NR,
};

/*
                                     
 */
enum wl_type_t {
	ASYNC_WORKLOAD = 0,
	SYNC_NOIDLE_WORKLOAD = 1,
	SYNC_WORKLOAD = 2
};

/*                                                  */
struct cfq_group {
	/*                           */
	struct rb_node rb_node;

	/*                        */
	u64 vdisktime;
	unsigned int weight;
	unsigned int new_weight;
	bool needs_update;

	/*                                        */
	int nr_cfqq;

	/*
                                                                     
                                                                   
                                                                    
                                                                
  */
	unsigned int busy_queues_avg[CFQ_PRIO_NR];
	/*
                                                                   
                                                               
                                                                   
                                                                    
                                    
                                          
  */
	struct cfq_rb_root service_trees[2][3];
	struct cfq_rb_root service_tree_idle;

	unsigned long saved_workload_slice;
	enum wl_type_t saved_workload;
	enum wl_prio_t saved_serving_prio;
	struct blkio_group blkg;
#ifdef CONFIG_CFQ_GROUP_IOSCHED
	struct hlist_node cfqd_node;
	int ref;
#endif
	/*                                                                   */
	int dispatched;
	struct cfq_ttime ttime;
};

struct cfq_io_cq {
	struct io_cq		icq;		/*                          */
	struct cfq_queue	*cfqq[2];
	struct cfq_ttime	ttime;
};

/*
                                   
 */
struct cfq_data {
	struct request_queue *queue;
	/*                                  */
	struct cfq_rb_root grp_service_tree;
	struct cfq_group root_group;

	/*
                                       
  */
	enum wl_prio_t serving_prio;
	enum wl_type_t serving_type;
	unsigned long workload_expires;
	struct cfq_group *serving_group;

	/*
                                                                 
                                                             
                                                     
  */
	struct rb_root prio_trees[CFQ_PRIO_LISTS];

	unsigned int busy_queues;
	unsigned int busy_sync_queues;

	int rq_in_driver;
	int rq_in_flight[2];

	/*
                         
  */
	int rq_queued;
	int hw_tag;
	/*
                 
                                                                                          
                                                                      
                
  */
	int hw_tag_est_depth;
	unsigned int hw_tag_samples;

	/*
                          
  */
	struct timer_list idle_slice_timer;
	struct work_struct unplug_work;

	struct cfq_queue *active_queue;
	struct cfq_io_cq *active_cic;

	/*
                                      
  */
	struct cfq_queue *async_cfqq[2][IOPRIO_BE_NR];
	struct cfq_queue *async_idle_cfqq;

	sector_t last_position;

	/*
                             
  */
	unsigned int cfq_quantum;
	unsigned int cfq_fifo_expire[2];
	unsigned int cfq_back_penalty;
	unsigned int cfq_back_max;
	unsigned int cfq_slice[2];
	unsigned int cfq_slice_async_rq;
	unsigned int cfq_slice_idle;
	unsigned int cfq_group_idle;
	unsigned int cfq_latency;
	unsigned int cfq_target_latency;

	/*
                                                  
  */
	struct cfq_queue oom_cfqq;

	unsigned long last_delayed_sync;

	/*                                                */
	struct hlist_head cfqg_list;

	/*                                                */
	unsigned int nr_blkcg_linked_grps;
};

static struct cfq_group *cfq_get_next_cfqg(struct cfq_data *cfqd);

static struct cfq_rb_root *service_tree_for(struct cfq_group *cfqg,
					    enum wl_prio_t prio,
					    enum wl_type_t type)
{
	if (!cfqg)
		return NULL;

	if (prio == IDLE_WORKLOAD)
		return &cfqg->service_tree_idle;

	return &cfqg->service_trees[prio][type];
}

enum cfqq_state_flags {
	CFQ_CFQQ_FLAG_on_rr = 0,	/*                          */
	CFQ_CFQQ_FLAG_wait_request,	/*                       */
	CFQ_CFQQ_FLAG_must_dispatch,	/*                            */
	CFQ_CFQQ_FLAG_must_alloc_slice,	/*                           */
	CFQ_CFQQ_FLAG_fifo_expire,	/*                            */
	CFQ_CFQQ_FLAG_idle_window,	/*                      */
	CFQ_CFQQ_FLAG_prio_changed,	/*                           */
	CFQ_CFQQ_FLAG_slice_new,	/*                                 */
	CFQ_CFQQ_FLAG_sync,		/*                   */
	CFQ_CFQQ_FLAG_coop,		/*                */
	CFQ_CFQQ_FLAG_split_coop,	/*                              */
	CFQ_CFQQ_FLAG_deep,		/*                                   */
	CFQ_CFQQ_FLAG_wait_busy,	/*                          */
};

#define CFQ_CFQQ_FNS(name)						\
static inline void cfq_mark_cfqq_##name(struct cfq_queue *cfqq)		\
{									\
	(cfqq)->flags |= (1 << CFQ_CFQQ_FLAG_##name);			\
}									\
static inline void cfq_clear_cfqq_##name(struct cfq_queue *cfqq)	\
{									\
	(cfqq)->flags &= ~(1 << CFQ_CFQQ_FLAG_##name);			\
}									\
static inline int cfq_cfqq_##name(const struct cfq_queue *cfqq)		\
{									\
	return ((cfqq)->flags & (1 << CFQ_CFQQ_FLAG_##name)) != 0;	\
}

CFQ_CFQQ_FNS(on_rr);
CFQ_CFQQ_FNS(wait_request);
CFQ_CFQQ_FNS(must_dispatch);
CFQ_CFQQ_FNS(must_alloc_slice);
CFQ_CFQQ_FNS(fifo_expire);
CFQ_CFQQ_FNS(idle_window);
CFQ_CFQQ_FNS(prio_changed);
CFQ_CFQQ_FNS(slice_new);
CFQ_CFQQ_FNS(sync);
CFQ_CFQQ_FNS(coop);
CFQ_CFQQ_FNS(split_coop);
CFQ_CFQQ_FNS(deep);
CFQ_CFQQ_FNS(wait_busy);
#undef CFQ_CFQQ_FNS

#ifdef CONFIG_CFQ_GROUP_IOSCHED
#define cfq_log_cfqq(cfqd, cfqq, fmt, args...)	\
	blk_add_trace_msg((cfqd)->queue, "cfq%d%c %s " fmt, (cfqq)->pid, \
			cfq_cfqq_sync((cfqq)) ? 'S' : 'A', \
			blkg_path(&(cfqq)->cfqg->blkg), ##args)

#define cfq_log_cfqg(cfqd, cfqg, fmt, args...)				\
	blk_add_trace_msg((cfqd)->queue, "%s " fmt,			\
				blkg_path(&(cfqg)->blkg), ##args)       \

#else
#define cfq_log_cfqq(cfqd, cfqq, fmt, args...)	\
	blk_add_trace_msg((cfqd)->queue, "cfq%d " fmt, (cfqq)->pid, ##args)
#define cfq_log_cfqg(cfqd, cfqg, fmt, args...)		do {} while (0)
#endif
#define cfq_log(cfqd, fmt, args...)	\
	blk_add_trace_msg((cfqd)->queue, "cfq " fmt, ##args)

/*                                           */
#define for_each_cfqg_st(cfqg, i, j, st) \
	for (i = 0; i <= IDLE_WORKLOAD; i++) \
		for (j = 0, st = i < IDLE_WORKLOAD ? &cfqg->service_trees[i][j]\
			: &cfqg->service_tree_idle; \
			(i < IDLE_WORKLOAD && j <= SYNC_WORKLOAD) || \
			(i == IDLE_WORKLOAD && j == 0); \
			j++, st = i < IDLE_WORKLOAD ? \
			&cfqg->service_trees[i][j]: NULL) \

static inline bool cfq_io_thinktime_big(struct cfq_data *cfqd,
	struct cfq_ttime *ttime, bool group_idle)
{
	unsigned long slice;
	if (!sample_valid(ttime->ttime_samples))
		return false;
	if (group_idle)
		slice = cfqd->cfq_group_idle;
	else
		slice = cfqd->cfq_slice_idle;
	return ttime->ttime_mean > slice;
}

static inline bool iops_mode(struct cfq_data *cfqd)
{
	/*
                                                                  
                                                                  
                                                                  
                                                                   
                                                                 
  */
	if (!cfqd->cfq_slice_idle && cfqd->hw_tag)
		return true;
	else
		return false;
}

static inline enum wl_prio_t cfqq_prio(struct cfq_queue *cfqq)
{
	if (cfq_class_idle(cfqq))
		return IDLE_WORKLOAD;
	if (cfq_class_rt(cfqq))
		return RT_WORKLOAD;
	return BE_WORKLOAD;
}


static enum wl_type_t cfqq_type(struct cfq_queue *cfqq)
{
	if (!cfq_cfqq_sync(cfqq))
		return ASYNC_WORKLOAD;
	if (!cfq_cfqq_idle_window(cfqq))
		return SYNC_NOIDLE_WORKLOAD;
	return SYNC_WORKLOAD;
}

static inline int cfq_group_busy_queues_wl(enum wl_prio_t wl,
					struct cfq_data *cfqd,
					struct cfq_group *cfqg)
{
	if (wl == IDLE_WORKLOAD)
		return cfqg->service_tree_idle.count;

	return cfqg->service_trees[wl][ASYNC_WORKLOAD].count
		+ cfqg->service_trees[wl][SYNC_NOIDLE_WORKLOAD].count
		+ cfqg->service_trees[wl][SYNC_WORKLOAD].count;
}

static inline int cfqg_busy_async_queues(struct cfq_data *cfqd,
					struct cfq_group *cfqg)
{
	return cfqg->service_trees[RT_WORKLOAD][ASYNC_WORKLOAD].count
		+ cfqg->service_trees[BE_WORKLOAD][ASYNC_WORKLOAD].count;
}

static void cfq_dispatch_insert(struct request_queue *, struct request *);
static struct cfq_queue *cfq_get_queue(struct cfq_data *, bool,
				       struct io_context *, gfp_t);

static inline struct cfq_io_cq *icq_to_cic(struct io_cq *icq)
{
	/*                                                           */
	return container_of(icq, struct cfq_io_cq, icq);
}

static inline struct cfq_io_cq *cfq_cic_lookup(struct cfq_data *cfqd,
					       struct io_context *ioc)
{
	if (ioc)
		return icq_to_cic(ioc_lookup_icq(ioc, cfqd->queue));
	return NULL;
}

static inline struct cfq_queue *cic_to_cfqq(struct cfq_io_cq *cic, bool is_sync)
{
	return cic->cfqq[is_sync];
}

static inline void cic_set_cfqq(struct cfq_io_cq *cic, struct cfq_queue *cfqq,
				bool is_sync)
{
	cic->cfqq[is_sync] = cfqq;
}

static inline struct cfq_data *cic_to_cfqd(struct cfq_io_cq *cic)
{
	return cic->icq.q->elevator->elevator_data;
}

/*
                                                                         
                                                     
 */
static inline bool cfq_bio_sync(struct bio *bio)
{
	return bio_data_dir(bio) == READ || (bio->bi_rw & REQ_SYNC);
}

/*
                                                                          
                                    
 */
static inline void cfq_schedule_dispatch(struct cfq_data *cfqd)
{
	if (cfqd->busy_queues) {
		cfq_log(cfqd, "schedule dispatch");
		kblockd_schedule_work(cfqd->queue, &cfqd->unplug_work);
	}
}

/*
                                                                          
                                                                            
                                                  
 */
static inline int cfq_prio_slice(struct cfq_data *cfqd, bool sync,
				 unsigned short prio)
{
	const int base_slice = cfqd->cfq_slice[sync];

	WARN_ON(prio >= IOPRIO_BE_NR);

	return base_slice + (base_slice/CFQ_SLICE_SCALE * (4 - prio));
}

static inline int
cfq_prio_to_slice(struct cfq_data *cfqd, struct cfq_queue *cfqq)
{
	return cfq_prio_slice(cfqd, cfq_cfqq_sync(cfqq), cfqq->ioprio);
}

static inline u64 cfq_scale_slice(unsigned long delta, struct cfq_group *cfqg)
{
	u64 d = delta << CFQ_SERVICE_SHIFT;

	d = d * BLKIO_WEIGHT_DEFAULT;
	do_div(d, cfqg->weight);
	return d;
}

static inline u64 max_vdisktime(u64 min_vdisktime, u64 vdisktime)
{
	s64 delta = (s64)(vdisktime - min_vdisktime);
	if (delta > 0)
		min_vdisktime = vdisktime;

	return min_vdisktime;
}

static inline u64 min_vdisktime(u64 min_vdisktime, u64 vdisktime)
{
	s64 delta = (s64)(vdisktime - min_vdisktime);
	if (delta < 0)
		min_vdisktime = vdisktime;

	return min_vdisktime;
}

static void update_min_vdisktime(struct cfq_rb_root *st)
{
	struct cfq_group *cfqg;

	if (st->left) {
		cfqg = rb_entry_cfqg(st->left);
		st->min_vdisktime = max_vdisktime(st->min_vdisktime,
						  cfqg->vdisktime);
	}
}

/*
                                                   
                                                                               
                                                          
 */

static inline unsigned cfq_group_get_avg_queues(struct cfq_data *cfqd,
					struct cfq_group *cfqg, bool rt)
{
	unsigned min_q, max_q;
	unsigned mult  = cfq_hist_divisor - 1;
	unsigned round = cfq_hist_divisor / 2;
	unsigned busy = cfq_group_busy_queues_wl(rt, cfqd, cfqg);

	min_q = min(cfqg->busy_queues_avg[rt], busy);
	max_q = max(cfqg->busy_queues_avg[rt], busy);
	cfqg->busy_queues_avg[rt] = (mult * max_q + min_q + round) /
		cfq_hist_divisor;
	return cfqg->busy_queues_avg[rt];
}

static inline unsigned
cfq_group_slice(struct cfq_data *cfqd, struct cfq_group *cfqg)
{
	struct cfq_rb_root *st = &cfqd->grp_service_tree;

	return cfqd->cfq_target_latency * cfqg->weight / st->total_weight;
}

static inline unsigned
cfq_scaled_cfqq_slice(struct cfq_data *cfqd, struct cfq_queue *cfqq)
{
	unsigned slice = cfq_prio_to_slice(cfqd, cfqq);
	if (cfqd->cfq_latency) {
		/*
                                                               
                                     
   */
		unsigned iq = cfq_group_get_avg_queues(cfqd, cfqq->cfqg,
						cfq_class_rt(cfqq));
		unsigned sync_slice = cfqd->cfq_slice[1];
		unsigned expect_latency = sync_slice * iq;
		unsigned group_slice = cfq_group_slice(cfqd, cfqq->cfqg);

		if (expect_latency > group_slice) {
			unsigned base_low_slice = 2 * cfqd->cfq_slice_idle;
			/*                                         
                        */
			unsigned low_slice =
				min(slice, base_low_slice * slice / sync_slice);
			/*                                                 
                              */
			slice = max(slice * group_slice / expect_latency,
				    low_slice);
		}
	}
	return slice;
}

static inline void
cfq_set_prio_slice(struct cfq_data *cfqd, struct cfq_queue *cfqq)
{
	unsigned slice = cfq_scaled_cfqq_slice(cfqd, cfqq);

	cfqq->slice_start = jiffies;
	cfqq->slice_end = jiffies + slice;
	cfqq->allocated_slice = slice;
	cfq_log_cfqq(cfqd, cfqq, "set_slice=%lu", cfqq->slice_end - jiffies);
}

/*
                                                                        
                                                                     
                          
 */
static inline bool cfq_slice_used(struct cfq_queue *cfqq)
{
	if (cfq_cfqq_slice_new(cfqq))
		return false;
	if (time_before(jiffies, cfqq->slice_end))
		return false;

	return true;
}

/*
                                                                        
                                                                        
                                                                     
 */
static struct request *
cfq_choose_req(struct cfq_data *cfqd, struct request *rq1, struct request *rq2, sector_t last)
{
	sector_t s1, s2, d1 = 0, d2 = 0;
	unsigned long back_max;
#define CFQ_RQ1_WRAP	0x01 /*                 */
#define CFQ_RQ2_WRAP	0x02 /*                 */
	unsigned wrap = 0; /*                                          */

	if (rq1 == NULL || rq1 == rq2)
		return rq2;
	if (rq2 == NULL)
		return rq1;

	if (rq_is_sync(rq1) != rq_is_sync(rq2))
		return rq_is_sync(rq1) ? rq1 : rq2;

	if ((rq1->cmd_flags ^ rq2->cmd_flags) & REQ_PRIO)
		return rq1->cmd_flags & REQ_PRIO ? rq1 : rq2;

	s1 = blk_rq_pos(rq1);
	s2 = blk_rq_pos(rq2);

	/*
                                    
  */
	back_max = cfqd->cfq_back_max * 2;

	/*
                                                               
                                                                
                         
  */
	if (s1 >= last)
		d1 = s1 - last;
	else if (s1 + back_max >= last)
		d1 = (last - s1) * cfqd->cfq_back_penalty;
	else
		wrap |= CFQ_RQ1_WRAP;

	if (s2 >= last)
		d2 = s2 - last;
	else if (s2 + back_max >= last)
		d2 = (last - s2) * cfqd->cfq_back_penalty;
	else
		wrap |= CFQ_RQ2_WRAP;

	/*                     */

	/*
                                                               
                                                         
  */
	switch (wrap) {
	case 0: /*                                              */
		if (d1 < d2)
			return rq1;
		else if (d2 < d1)
			return rq2;
		else {
			if (s1 >= s2)
				return rq1;
			else
				return rq2;
		}

	case CFQ_RQ2_WRAP:
		return rq1;
	case CFQ_RQ1_WRAP:
		return rq2;
	case (CFQ_RQ1_WRAP|CFQ_RQ2_WRAP): /*                  */
	default:
		/*
                                
                                                  
                                         
                                                  
   */
		if (s1 <= s2)
			return rq1;
		else
			return rq2;
	}
}

/*
                                           
 */
static struct cfq_queue *cfq_rb_first(struct cfq_rb_root *root)
{
	/*                       */
	if (!root->count)
		return NULL;

	if (!root->left)
		root->left = rb_first(&root->rb);

	if (root->left)
		return rb_entry(root->left, struct cfq_queue, rb_node);

	return NULL;
}

static struct cfq_group *cfq_rb_first_group(struct cfq_rb_root *root)
{
	if (!root->left)
		root->left = rb_first(&root->rb);

	if (root->left)
		return rb_entry_cfqg(root->left);

	return NULL;
}

static void rb_erase_init(struct rb_node *n, struct rb_root *root)
{
	rb_erase(n, root);
	RB_CLEAR_NODE(n);
}

static void cfq_rb_erase(struct rb_node *n, struct cfq_rb_root *root)
{
	if (root->left == n)
		root->left = NULL;
	rb_erase_init(n, &root->rb);
	--root->count;
}

/*
                                                              
 */
static struct request *
cfq_find_next_rq(struct cfq_data *cfqd, struct cfq_queue *cfqq,
		  struct request *last)
{
	struct rb_node *rbnext = rb_next(&last->rb_node);
	struct rb_node *rbprev = rb_prev(&last->rb_node);
	struct request *next = NULL, *prev = NULL;

	BUG_ON(RB_EMPTY_NODE(&last->rb_node));

	if (rbprev)
		prev = rb_entry_rq(rbprev);

	if (rbnext)
		next = rb_entry_rq(rbnext);
	else {
		rbnext = rb_first(&cfqq->sort_list);
		if (rbnext && rbnext != &last->rb_node)
			next = rb_entry_rq(rbnext);
	}

	return cfq_choose_req(cfqd, next, prev, blk_rq_pos(last));
}

static unsigned long cfq_slice_offset(struct cfq_data *cfqd,
				      struct cfq_queue *cfqq)
{
	/*
                                        
  */
	return (cfqq->cfqg->nr_cfqq - 1) * (cfq_prio_slice(cfqd, 1, 0) -
		       cfq_prio_slice(cfqd, cfq_cfqq_sync(cfqq), cfqq->ioprio));
}

static inline s64
cfqg_key(struct cfq_rb_root *st, struct cfq_group *cfqg)
{
	return cfqg->vdisktime - st->min_vdisktime;
}

static void
__cfq_group_service_tree_add(struct cfq_rb_root *st, struct cfq_group *cfqg)
{
	struct rb_node **node = &st->rb.rb_node;
	struct rb_node *parent = NULL;
	struct cfq_group *__cfqg;
	s64 key = cfqg_key(st, cfqg);
	int left = 1;

	while (*node != NULL) {
		parent = *node;
		__cfqg = rb_entry_cfqg(parent);

		if (key < cfqg_key(st, __cfqg))
			node = &parent->rb_left;
		else {
			node = &parent->rb_right;
			left = 0;
		}
	}

	if (left)
		st->left = &cfqg->rb_node;

	rb_link_node(&cfqg->rb_node, parent, node);
	rb_insert_color(&cfqg->rb_node, &st->rb);
}

static void
cfq_update_group_weight(struct cfq_group *cfqg)
{
	BUG_ON(!RB_EMPTY_NODE(&cfqg->rb_node));
	if (cfqg->needs_update) {
		cfqg->weight = cfqg->new_weight;
		cfqg->needs_update = false;
	}
}

static void
cfq_group_service_tree_add(struct cfq_rb_root *st, struct cfq_group *cfqg)
{
	BUG_ON(!RB_EMPTY_NODE(&cfqg->rb_node));

	cfq_update_group_weight(cfqg);
	__cfq_group_service_tree_add(st, cfqg);
	st->total_weight += cfqg->weight;
}

static void
cfq_group_notify_queue_add(struct cfq_data *cfqd, struct cfq_group *cfqg)
{
	struct cfq_rb_root *st = &cfqd->grp_service_tree;
	struct cfq_group *__cfqg;
	struct rb_node *n;

	cfqg->nr_cfqq++;
	if (!RB_EMPTY_NODE(&cfqg->rb_node))
		return;

	/*
                                                                 
                                                                   
                                                                      
  */
	n = rb_last(&st->rb);
	if (n) {
		__cfqg = rb_entry_cfqg(n);
		cfqg->vdisktime = __cfqg->vdisktime + CFQ_IDLE_DELAY;
	} else
		cfqg->vdisktime = st->min_vdisktime;
	cfq_group_service_tree_add(st, cfqg);
}

static void
cfq_group_service_tree_del(struct cfq_rb_root *st, struct cfq_group *cfqg)
{
	st->total_weight -= cfqg->weight;
	if (!RB_EMPTY_NODE(&cfqg->rb_node))
		cfq_rb_erase(&cfqg->rb_node, st);
}

static void
cfq_group_notify_queue_del(struct cfq_data *cfqd, struct cfq_group *cfqg)
{
	struct cfq_rb_root *st = &cfqd->grp_service_tree;

	BUG_ON(cfqg->nr_cfqq < 1);
	cfqg->nr_cfqq--;

	/*                                                                 */
	if (cfqg->nr_cfqq)
		return;

	cfq_log_cfqg(cfqd, cfqg, "del_from_rr group");
	cfq_group_service_tree_del(st, cfqg);
	cfqg->saved_workload_slice = 0;
	cfq_blkiocg_update_dequeue_stats(&cfqg->blkg, 1);
}

static inline unsigned int cfq_cfqq_slice_usage(struct cfq_queue *cfqq,
						unsigned int *unaccounted_time)
{
	unsigned int slice_used;

	/*
                                                               
                                                           
  */
	if (!cfqq->slice_start || cfqq->slice_start == jiffies) {
		/*
                                                               
                                                                
                                                                
                                  
   */
		slice_used = max_t(unsigned, (jiffies - cfqq->dispatch_start),
					1);
	} else {
		slice_used = jiffies - cfqq->slice_start;
		if (slice_used > cfqq->allocated_slice) {
			*unaccounted_time = slice_used - cfqq->allocated_slice;
			slice_used = cfqq->allocated_slice;
		}
		if (time_after(cfqq->slice_start, cfqq->dispatch_start))
			*unaccounted_time += cfqq->slice_start -
					cfqq->dispatch_start;
	}

	return slice_used;
}

static void cfq_group_served(struct cfq_data *cfqd, struct cfq_group *cfqg,
				struct cfq_queue *cfqq)
{
	struct cfq_rb_root *st = &cfqd->grp_service_tree;
	unsigned int used_sl, charge, unaccounted_sl = 0;
	int nr_sync = cfqg->nr_cfqq - cfqg_busy_async_queues(cfqd, cfqg)
			- cfqg->service_tree_idle.count;

	BUG_ON(nr_sync < 0);
	used_sl = charge = cfq_cfqq_slice_usage(cfqq, &unaccounted_sl);

	if (iops_mode(cfqd))
		charge = cfqq->slice_dispatch;
	else if (!cfq_cfqq_sync(cfqq) && !nr_sync)
		charge = cfqq->allocated_slice;

	/*                                                       */
	cfq_group_service_tree_del(st, cfqg);
	cfqg->vdisktime += cfq_scale_slice(charge, cfqg);
	/*                                                     */
	cfq_group_service_tree_add(st, cfqg);

	/*                                               */
	if (time_after(cfqd->workload_expires, jiffies)) {
		cfqg->saved_workload_slice = cfqd->workload_expires
						- jiffies;
		cfqg->saved_workload = cfqd->serving_type;
		cfqg->saved_serving_prio = cfqd->serving_prio;
	} else
		cfqg->saved_workload_slice = 0;

	cfq_log_cfqg(cfqd, cfqg, "served: vt=%llu min_vt=%llu", cfqg->vdisktime,
					st->min_vdisktime);
	cfq_log_cfqq(cfqq->cfqd, cfqq,
		     "sl_used=%u disp=%u charge=%u iops=%u sect=%lu",
		     used_sl, cfqq->slice_dispatch, charge,
		     iops_mode(cfqd), cfqq->nr_sectors);
	cfq_blkiocg_update_timeslice_used(&cfqg->blkg, used_sl,
					  unaccounted_sl);
	cfq_blkiocg_set_start_empty_time(&cfqg->blkg);
}

#ifdef CONFIG_CFQ_GROUP_IOSCHED
static inline struct cfq_group *cfqg_of_blkg(struct blkio_group *blkg)
{
	if (blkg)
		return container_of(blkg, struct cfq_group, blkg);
	return NULL;
}

static void cfq_update_blkio_group_weight(void *key, struct blkio_group *blkg,
					  unsigned int weight)
{
	struct cfq_group *cfqg = cfqg_of_blkg(blkg);
	cfqg->new_weight = weight;
	cfqg->needs_update = true;
}

static void cfq_init_add_cfqg_lists(struct cfq_data *cfqd,
			struct cfq_group *cfqg, struct blkio_cgroup *blkcg)
{
	struct backing_dev_info *bdi = &cfqd->queue->backing_dev_info;
	unsigned int major, minor;

	/*
                                                                
                                                                
                                                                    
                 
  */
	if (bdi->dev) {
		sscanf(dev_name(bdi->dev), "%u:%u", &major, &minor);
		cfq_blkiocg_add_blkio_group(blkcg, &cfqg->blkg,
					(void *)cfqd, MKDEV(major, minor));
	} else
		cfq_blkiocg_add_blkio_group(blkcg, &cfqg->blkg,
					(void *)cfqd, 0);

	cfqd->nr_blkcg_linked_grps++;
	cfqg->weight = blkcg_get_weight(blkcg, cfqg->blkg.dev);

	/*                        */
	hlist_add_head(&cfqg->cfqd_node, &cfqd->cfqg_list);
}

/*
                                                                        
                                                                          
                          
 */
static struct cfq_group * cfq_alloc_cfqg(struct cfq_data *cfqd)
{
	struct cfq_group *cfqg = NULL;
	int i, j, ret;
	struct cfq_rb_root *st;

	cfqg = kzalloc_node(sizeof(*cfqg), GFP_ATOMIC, cfqd->queue->node);
	if (!cfqg)
		return NULL;

	for_each_cfqg_st(cfqg, i, j, st)
		*st = CFQ_RB_ROOT;
	RB_CLEAR_NODE(&cfqg->rb_node);

	cfqg->ttime.last_end_request = jiffies;

	/*
                                                               
                                                          
                                                          
                                                              
  */
	cfqg->ref = 1;

	ret = blkio_alloc_blkg_stats(&cfqg->blkg);
	if (ret) {
		kfree(cfqg);
		return NULL;
	}

	return cfqg;
}

static struct cfq_group *
cfq_find_cfqg(struct cfq_data *cfqd, struct blkio_cgroup *blkcg)
{
	struct cfq_group *cfqg = NULL;
	void *key = cfqd;
	struct backing_dev_info *bdi = &cfqd->queue->backing_dev_info;
	unsigned int major, minor;

	/*
                                                            
                             
  */
	if (blkcg == &blkio_root_cgroup)
		cfqg = &cfqd->root_group;
	else
		cfqg = cfqg_of_blkg(blkiocg_lookup_group(blkcg, key));

	if (cfqg && !cfqg->blkg.dev && bdi->dev && dev_name(bdi->dev)) {
		sscanf(dev_name(bdi->dev), "%u:%u", &major, &minor);
		cfqg->blkg.dev = MKDEV(major, minor);
	}

	return cfqg;
}

/*
                                                                            
           
 */
static struct cfq_group *cfq_get_cfqg(struct cfq_data *cfqd)
{
	struct blkio_cgroup *blkcg;
	struct cfq_group *cfqg = NULL, *__cfqg = NULL;
	struct request_queue *q = cfqd->queue;

	rcu_read_lock();
	blkcg = task_blkio_cgroup(current);
	cfqg = cfq_find_cfqg(cfqd, blkcg);
	if (cfqg) {
		rcu_read_unlock();
		return cfqg;
	}

	/*
                                                                       
                                                                       
                                                                 
   
                                                                  
                                                                
                                      
  */

	rcu_read_unlock();
	spin_unlock_irq(q->queue_lock);

	cfqg = cfq_alloc_cfqg(cfqd);

	spin_lock_irq(q->queue_lock);

	rcu_read_lock();
	blkcg = task_blkio_cgroup(current);

	/*
                                                                  
                                             
  */
	__cfqg = cfq_find_cfqg(cfqd, blkcg);

	if (__cfqg) {
		kfree(cfqg);
		rcu_read_unlock();
		return __cfqg;
	}

	if (!cfqg)
		cfqg = &cfqd->root_group;

	cfq_init_add_cfqg_lists(cfqd, cfqg, blkcg);
	rcu_read_unlock();
	return cfqg;
}

static inline struct cfq_group *cfq_ref_get_cfqg(struct cfq_group *cfqg)
{
	cfqg->ref++;
	return cfqg;
}

static void cfq_link_cfqq_cfqg(struct cfq_queue *cfqq, struct cfq_group *cfqg)
{
	/*                                                      */
	if (!cfq_cfqq_sync(cfqq))
		cfqg = &cfqq->cfqd->root_group;

	cfqq->cfqg = cfqg;
	/*                        */
	cfqq->cfqg->ref++;
}

static void cfq_put_cfqg(struct cfq_group *cfqg)
{
	struct cfq_rb_root *st;
	int i, j;

	BUG_ON(cfqg->ref <= 0);
	cfqg->ref--;
	if (cfqg->ref)
		return;
	for_each_cfqg_st(cfqg, i, j, st)
		BUG_ON(!RB_EMPTY_ROOT(&st->rb));
	free_percpu(cfqg->blkg.stats_cpu);
	kfree(cfqg);
}

static void cfq_destroy_cfqg(struct cfq_data *cfqd, struct cfq_group *cfqg)
{
	/*                                                             */
	BUG_ON(hlist_unhashed(&cfqg->cfqd_node));

	hlist_del_init(&cfqg->cfqd_node);

	BUG_ON(cfqd->nr_blkcg_linked_grps <= 0);
	cfqd->nr_blkcg_linked_grps--;

	/*
                                                                    
                                            
  */
	cfq_put_cfqg(cfqg);
}

static void cfq_release_cfq_groups(struct cfq_data *cfqd)
{
	struct hlist_node *pos, *n;
	struct cfq_group *cfqg;

	hlist_for_each_entry_safe(cfqg, pos, n, &cfqd->cfqg_list, cfqd_node) {
		/*
                                                              
                                                              
               
   */
		if (!cfq_blkiocg_del_blkio_group(&cfqg->blkg))
			cfq_destroy_cfqg(cfqd, cfqg);
	}
}

/*
                                                                             
                                                                           
                                                                         
                                           
  
                                                                          
                                                                              
             
  
                                                                          
                                                                        
                        
 */
static void cfq_unlink_blkio_group(void *key, struct blkio_group *blkg)
{
	unsigned long  flags;
	struct cfq_data *cfqd = key;

	spin_lock_irqsave(cfqd->queue->queue_lock, flags);
	cfq_destroy_cfqg(cfqd, cfqg_of_blkg(blkg));
	spin_unlock_irqrestore(cfqd->queue->queue_lock, flags);
}

#else /*               */
static struct cfq_group *cfq_get_cfqg(struct cfq_data *cfqd)
{
	return &cfqd->root_group;
}

static inline struct cfq_group *cfq_ref_get_cfqg(struct cfq_group *cfqg)
{
	return cfqg;
}

static inline void
cfq_link_cfqq_cfqg(struct cfq_queue *cfqq, struct cfq_group *cfqg) {
	cfqq->cfqg = cfqg;
}

static void cfq_release_cfq_groups(struct cfq_data *cfqd) {}
static inline void cfq_put_cfqg(struct cfq_group *cfqg) {}

#endif /*               */

/*
                                                                  
                                                                   
                              
 */
static void cfq_service_tree_add(struct cfq_data *cfqd, struct cfq_queue *cfqq,
				 bool add_front)
{
	struct rb_node **p, *parent;
	struct cfq_queue *__cfqq;
	unsigned long rb_key;
	struct cfq_rb_root *service_tree;
	int left;
	int new_cfqq = 1;

	service_tree = service_tree_for(cfqq->cfqg, cfqq_prio(cfqq),
						cfqq_type(cfqq));
	if (cfq_class_idle(cfqq)) {
		rb_key = CFQ_IDLE_DELAY;
		parent = rb_last(&service_tree->rb);
		if (parent && parent != &cfqq->rb_node) {
			__cfqq = rb_entry(parent, struct cfq_queue, rb_node);
			rb_key += __cfqq->rb_key;
		} else
			rb_key += jiffies;
	} else if (!add_front) {
		/*
                                                       
                                                      
                                                            
                                                    
   */
		rb_key = cfq_slice_offset(cfqd, cfqq) + jiffies;
		rb_key -= cfqq->slice_resid;
		cfqq->slice_resid = 0;
	} else {
		rb_key = -HZ;
		__cfqq = cfq_rb_first(service_tree);
		rb_key += __cfqq ? __cfqq->rb_key : jiffies;
	}

	if (!RB_EMPTY_NODE(&cfqq->rb_node)) {
		new_cfqq = 0;
		/*
                                      
   */
		if (rb_key == cfqq->rb_key &&
		    cfqq->service_tree == service_tree)
			return;

		cfq_rb_erase(&cfqq->rb_node, cfqq->service_tree);
		cfqq->service_tree = NULL;
	}

	left = 1;
	parent = NULL;
	cfqq->service_tree = service_tree;
	p = &service_tree->rb.rb_node;
	while (*p) {
		struct rb_node **n;

		parent = *p;
		__cfqq = rb_entry(parent, struct cfq_queue, rb_node);

		/*
                                               
   */
		if (time_before(rb_key, __cfqq->rb_key))
			n = &(*p)->rb_left;
		else {
			n = &(*p)->rb_right;
			left = 0;
		}

		p = n;
	}

	if (left)
		service_tree->left = &cfqq->rb_node;

	cfqq->rb_key = rb_key;
	rb_link_node(&cfqq->rb_node, parent, p);
	rb_insert_color(&cfqq->rb_node, &service_tree->rb);
	service_tree->count++;
	if (add_front || !new_cfqq)
		return;
	cfq_group_notify_queue_add(cfqd, cfqq->cfqg);
}

static struct cfq_queue *
cfq_prio_tree_lookup(struct cfq_data *cfqd, struct rb_root *root,
		     sector_t sector, struct rb_node **ret_parent,
		     struct rb_node ***rb_link)
{
	struct rb_node **p, *parent;
	struct cfq_queue *cfqq = NULL;

	parent = NULL;
	p = &root->rb_node;
	while (*p) {
		struct rb_node **n;

		parent = *p;
		cfqq = rb_entry(parent, struct cfq_queue, p_node);

		/*
                                                          
                          
   */
		if (sector > blk_rq_pos(cfqq->next_rq))
			n = &(*p)->rb_right;
		else if (sector < blk_rq_pos(cfqq->next_rq))
			n = &(*p)->rb_left;
		else
			break;
		p = n;
		cfqq = NULL;
	}

	*ret_parent = parent;
	if (rb_link)
		*rb_link = p;
	return cfqq;
}

static void cfq_prio_tree_add(struct cfq_data *cfqd, struct cfq_queue *cfqq)
{
	struct rb_node **p, *parent;
	struct cfq_queue *__cfqq;

	if (cfqq->p_root) {
		rb_erase(&cfqq->p_node, cfqq->p_root);
		cfqq->p_root = NULL;
	}

	if (cfq_class_idle(cfqq))
		return;
	if (!cfqq->next_rq)
		return;

	cfqq->p_root = &cfqd->prio_trees[cfqq->org_ioprio];
	__cfqq = cfq_prio_tree_lookup(cfqd, cfqq->p_root,
				      blk_rq_pos(cfqq->next_rq), &parent, &p);
	if (!__cfqq) {
		rb_link_node(&cfqq->p_node, parent, p);
		rb_insert_color(&cfqq->p_node, cfqq->p_root);
	} else
		cfqq->p_root = NULL;
}

/*
                                              
 */
static void cfq_resort_rr_list(struct cfq_data *cfqd, struct cfq_queue *cfqq)
{
	/*
                                                             
  */
	if (cfq_cfqq_on_rr(cfqq)) {
		cfq_service_tree_add(cfqd, cfqq, 0);
		cfq_prio_tree_add(cfqd, cfqq);
	}
}

/*
                                                                        
                                                     
 */
static void cfq_add_cfqq_rr(struct cfq_data *cfqd, struct cfq_queue *cfqq)
{
	cfq_log_cfqq(cfqd, cfqq, "add_to_rr");
	BUG_ON(cfq_cfqq_on_rr(cfqq));
	cfq_mark_cfqq_on_rr(cfqq);
	cfqd->busy_queues++;
	if (cfq_cfqq_sync(cfqq))
		cfqd->busy_sync_queues++;

	cfq_resort_rr_list(cfqd, cfqq);
}

/*
                                                                      
                    
 */
static void cfq_del_cfqq_rr(struct cfq_data *cfqd, struct cfq_queue *cfqq)
{
	cfq_log_cfqq(cfqd, cfqq, "del_from_rr");
	BUG_ON(!cfq_cfqq_on_rr(cfqq));
	cfq_clear_cfqq_on_rr(cfqq);

	if (!RB_EMPTY_NODE(&cfqq->rb_node)) {
		cfq_rb_erase(&cfqq->rb_node, cfqq->service_tree);
		cfqq->service_tree = NULL;
	}
	if (cfqq->p_root) {
		rb_erase(&cfqq->p_node, cfqq->p_root);
		cfqq->p_root = NULL;
	}

	cfq_group_notify_queue_del(cfqd, cfqq->cfqg);
	BUG_ON(!cfqd->busy_queues);
	cfqd->busy_queues--;
	if (cfq_cfqq_sync(cfqq))
		cfqd->busy_sync_queues--;
}

/*
                            
 */
static void cfq_del_rq_rb(struct request *rq)
{
	struct cfq_queue *cfqq = RQ_CFQQ(rq);
	const int sync = rq_is_sync(rq);

	BUG_ON(!cfqq->queued[sync]);
	cfqq->queued[sync]--;

	elv_rb_del(&cfqq->sort_list, rq);

	if (cfq_cfqq_on_rr(cfqq) && RB_EMPTY_ROOT(&cfqq->sort_list)) {
		/*
                                                             
                                                             
                    
   */
		if (cfqq->p_root) {
			rb_erase(&cfqq->p_node, cfqq->p_root);
			cfqq->p_root = NULL;
		}
	}
}

static void cfq_add_rq_rb(struct request *rq)
{
	struct cfq_queue *cfqq = RQ_CFQQ(rq);
	struct cfq_data *cfqd = cfqq->cfqd;
	struct request *prev;

	cfqq->queued[rq_is_sync(rq)]++;

	elv_rb_add(&cfqq->sort_list, rq);

	if (!cfq_cfqq_on_rr(cfqq))
		cfq_add_cfqq_rr(cfqd, cfqq);

	/*
                                                          
  */
	prev = cfqq->next_rq;
	cfqq->next_rq = cfq_choose_req(cfqd, cfqq->next_rq, rq, cfqd->last_position);

	/*
                                                       
  */
	if (prev != cfqq->next_rq)
		cfq_prio_tree_add(cfqd, cfqq);

	BUG_ON(!cfqq->next_rq);
}

static void cfq_reposition_rq_rb(struct cfq_queue *cfqq, struct request *rq)
{
	elv_rb_del(&cfqq->sort_list, rq);
	cfqq->queued[rq_is_sync(rq)]--;
	cfq_blkiocg_update_io_remove_stats(&(RQ_CFQG(rq))->blkg,
					rq_data_dir(rq), rq_is_sync(rq));
	cfq_add_rq_rb(rq);
	cfq_blkiocg_update_io_add_stats(&(RQ_CFQG(rq))->blkg,
			&cfqq->cfqd->serving_group->blkg, rq_data_dir(rq),
			rq_is_sync(rq));
}

static struct request *
cfq_find_rq_fmerge(struct cfq_data *cfqd, struct bio *bio)
{
	struct task_struct *tsk = current;
	struct cfq_io_cq *cic;
	struct cfq_queue *cfqq;

	cic = cfq_cic_lookup(cfqd, tsk->io_context);
	if (!cic)
		return NULL;

	cfqq = cic_to_cfqq(cic, cfq_bio_sync(bio));
	if (cfqq) {
		sector_t sector = bio->bi_sector + bio_sectors(bio);

		return elv_rb_find(&cfqq->sort_list, sector);
	}

	return NULL;
}

static void cfq_activate_request(struct request_queue *q, struct request *rq)
{
	struct cfq_data *cfqd = q->elevator->elevator_data;

	cfqd->rq_in_driver++;
	cfq_log_cfqq(cfqd, RQ_CFQQ(rq), "activate rq, drv=%d",
						cfqd->rq_in_driver);

	cfqd->last_position = blk_rq_pos(rq) + blk_rq_sectors(rq);
}

static void cfq_deactivate_request(struct request_queue *q, struct request *rq)
{
	struct cfq_data *cfqd = q->elevator->elevator_data;

	WARN_ON(!cfqd->rq_in_driver);
	cfqd->rq_in_driver--;
	cfq_log_cfqq(cfqd, RQ_CFQQ(rq), "deactivate rq, drv=%d",
						cfqd->rq_in_driver);
}

static void cfq_remove_request(struct request *rq)
{
	struct cfq_queue *cfqq = RQ_CFQQ(rq);

	if (cfqq->next_rq == rq)
		cfqq->next_rq = cfq_find_next_rq(cfqq->cfqd, cfqq, rq);

	list_del_init(&rq->queuelist);
	cfq_del_rq_rb(rq);

	cfqq->cfqd->rq_queued--;
	cfq_blkiocg_update_io_remove_stats(&(RQ_CFQG(rq))->blkg,
					rq_data_dir(rq), rq_is_sync(rq));
	if (rq->cmd_flags & REQ_PRIO) {
		WARN_ON(!cfqq->prio_pending);
		cfqq->prio_pending--;
	}
}

static int cfq_merge(struct request_queue *q, struct request **req,
		     struct bio *bio)
{
	struct cfq_data *cfqd = q->elevator->elevator_data;
	struct request *__rq;

	__rq = cfq_find_rq_fmerge(cfqd, bio);
	if (__rq && elv_rq_merge_ok(__rq, bio)) {
		*req = __rq;
		return ELEVATOR_FRONT_MERGE;
	}

	return ELEVATOR_NO_MERGE;
}

static void cfq_merged_request(struct request_queue *q, struct request *req,
			       int type)
{
	if (type == ELEVATOR_FRONT_MERGE) {
		struct cfq_queue *cfqq = RQ_CFQQ(req);

		cfq_reposition_rq_rb(cfqq, req);
	}
}

static void cfq_bio_merged(struct request_queue *q, struct request *req,
				struct bio *bio)
{
	cfq_blkiocg_update_io_merged_stats(&(RQ_CFQG(req))->blkg,
					bio_data_dir(bio), cfq_bio_sync(bio));
}

static void
cfq_merged_requests(struct request_queue *q, struct request *rq,
		    struct request *next)
{
	struct cfq_queue *cfqq = RQ_CFQQ(rq);
	struct cfq_data *cfqd = q->elevator->elevator_data;

	/*
                                               
  */
	if (!list_empty(&rq->queuelist) && !list_empty(&next->queuelist) &&
	    time_before(rq_fifo_time(next), rq_fifo_time(rq))) {
		list_move(&rq->queuelist, &next->queuelist);
		rq_set_fifo_time(rq, rq_fifo_time(next));
	}

	if (cfqq->next_rq == next)
		cfqq->next_rq = rq;
	cfq_remove_request(next);
	cfq_blkiocg_update_io_merged_stats(&(RQ_CFQG(rq))->blkg,
					rq_data_dir(next), rq_is_sync(next));

	cfqq = RQ_CFQQ(next);
	/*
                                                                    
                                                    
                                                               
  */
	if (cfq_cfqq_on_rr(cfqq) && RB_EMPTY_ROOT(&cfqq->sort_list) &&
	    cfqq != cfqd->active_queue)
		cfq_del_cfqq_rr(cfqd, cfqq);
}

static int cfq_allow_merge(struct request_queue *q, struct request *rq,
			   struct bio *bio)
{
	struct cfq_data *cfqd = q->elevator->elevator_data;
	struct cfq_io_cq *cic;
	struct cfq_queue *cfqq;

	/*
                                                       
  */
	if (cfq_bio_sync(bio) && !rq_is_sync(rq))
		return false;

	/*
                                                               
                                     
  */
	cic = cfq_cic_lookup(cfqd, current->io_context);
	if (!cic)
		return false;

	cfqq = cic_to_cfqq(cic, cfq_bio_sync(bio));
	return cfqq == RQ_CFQQ(rq);
}

static inline void cfq_del_timer(struct cfq_data *cfqd, struct cfq_queue *cfqq)
{
	del_timer(&cfqd->idle_slice_timer);
	cfq_blkiocg_update_idle_time_stats(&cfqq->cfqg->blkg);
}

static void __cfq_set_active_queue(struct cfq_data *cfqd,
				   struct cfq_queue *cfqq)
{
	if (cfqq) {
		cfq_log_cfqq(cfqd, cfqq, "set_active wl_prio:%d wl_type:%d",
				cfqd->serving_prio, cfqd->serving_type);
		cfq_blkiocg_update_avg_queue_size_stats(&cfqq->cfqg->blkg);
		cfqq->slice_start = 0;
		cfqq->dispatch_start = jiffies;
		cfqq->allocated_slice = 0;
		cfqq->slice_end = 0;
		cfqq->slice_dispatch = 0;
		cfqq->nr_sectors = 0;

		cfq_clear_cfqq_wait_request(cfqq);
		cfq_clear_cfqq_must_dispatch(cfqq);
		cfq_clear_cfqq_must_alloc_slice(cfqq);
		cfq_clear_cfqq_fifo_expire(cfqq);
		cfq_mark_cfqq_slice_new(cfqq);

		cfq_del_timer(cfqd, cfqq);
	}

	cfqd->active_queue = cfqq;
}

/*
                                                                   
 */
static void
__cfq_slice_expired(struct cfq_data *cfqd, struct cfq_queue *cfqq,
		    bool timed_out)
{
	cfq_log_cfqq(cfqd, cfqq, "slice expired t=%d", timed_out);

	if (cfq_cfqq_wait_request(cfqq))
		cfq_del_timer(cfqd, cfqq);

	cfq_clear_cfqq_wait_request(cfqq);
	cfq_clear_cfqq_wait_busy(cfqq);

	/*
                                                               
                                                                
                                                                
                       
  */
	if (cfq_cfqq_coop(cfqq) && CFQQ_SEEKY(cfqq))
		cfq_mark_cfqq_split_coop(cfqq);

	/*
                                                                   
  */
	if (timed_out) {
		if (cfq_cfqq_slice_new(cfqq))
			cfqq->slice_resid = cfq_scaled_cfqq_slice(cfqd, cfqq);
		else
			cfqq->slice_resid = cfqq->slice_end - jiffies;
		cfq_log_cfqq(cfqd, cfqq, "resid=%ld", cfqq->slice_resid);
	}

	cfq_group_served(cfqd, cfqq->cfqg, cfqq);

	if (cfq_cfqq_on_rr(cfqq) && RB_EMPTY_ROOT(&cfqq->sort_list))
		cfq_del_cfqq_rr(cfqd, cfqq);

	cfq_resort_rr_list(cfqd, cfqq);

	if (cfqq == cfqd->active_queue)
		cfqd->active_queue = NULL;

	if (cfqd->active_cic) {
		put_io_context(cfqd->active_cic->icq.ioc);
		cfqd->active_cic = NULL;
	}
}

static inline void cfq_slice_expired(struct cfq_data *cfqd, bool timed_out)
{
	struct cfq_queue *cfqq = cfqd->active_queue;

	if (cfqq)
		__cfq_slice_expired(cfqd, cfqq, timed_out);
}

/*
                                                                 
                                                          
 */
static struct cfq_queue *cfq_get_next_queue(struct cfq_data *cfqd)
{
	struct cfq_rb_root *service_tree =
		service_tree_for(cfqd->serving_group, cfqd->serving_prio,
					cfqd->serving_type);

	if (!cfqd->rq_queued)
		return NULL;

	/*                              */
	if (!service_tree)
		return NULL;
	if (RB_EMPTY_ROOT(&service_tree->rb))
		return NULL;
	return cfq_rb_first(service_tree);
}

static struct cfq_queue *cfq_get_next_queue_forced(struct cfq_data *cfqd)
{
	struct cfq_group *cfqg;
	struct cfq_queue *cfqq;
	int i, j;
	struct cfq_rb_root *st;

	if (!cfqd->rq_queued)
		return NULL;

	cfqg = cfq_get_next_cfqg(cfqd);
	if (!cfqg)
		return NULL;

	for_each_cfqg_st(cfqg, i, j, st)
		if ((cfqq = cfq_rb_first(st)) != NULL)
			return cfqq;
	return NULL;
}

/*
                                              
 */
static struct cfq_queue *cfq_set_active_queue(struct cfq_data *cfqd,
					      struct cfq_queue *cfqq)
{
	if (!cfqq)
		cfqq = cfq_get_next_queue(cfqd);

	__cfq_set_active_queue(cfqd, cfqq);
	return cfqq;
}

static inline sector_t cfq_dist_from_last(struct cfq_data *cfqd,
					  struct request *rq)
{
	if (blk_rq_pos(rq) >= cfqd->last_position)
		return blk_rq_pos(rq) - cfqd->last_position;
	else
		return cfqd->last_position - blk_rq_pos(rq);
}

static inline int cfq_rq_close(struct cfq_data *cfqd, struct cfq_queue *cfqq,
			       struct request *rq)
{
	return cfq_dist_from_last(cfqd, rq) <= CFQQ_CLOSE_THR;
}

static struct cfq_queue *cfqq_close(struct cfq_data *cfqd,
				    struct cfq_queue *cur_cfqq)
{
	struct rb_root *root = &cfqd->prio_trees[cur_cfqq->org_ioprio];
	struct rb_node *parent, *node;
	struct cfq_queue *__cfqq;
	sector_t sector = cfqd->last_position;

	if (RB_EMPTY_ROOT(root))
		return NULL;

	/*
                                                               
                       
  */
	__cfqq = cfq_prio_tree_lookup(cfqd, root, sector, &parent, NULL);
	if (__cfqq)
		return __cfqq;

	/*
                                                                 
                                    
  */
	__cfqq = rb_entry(parent, struct cfq_queue, p_node);
	if (cfq_rq_close(cfqd, cur_cfqq, __cfqq->next_rq))
		return __cfqq;

	if (blk_rq_pos(__cfqq->next_rq) < sector)
		node = rb_next(&__cfqq->p_node);
	else
		node = rb_prev(&__cfqq->p_node);
	if (!node)
		return NULL;

	__cfqq = rb_entry(node, struct cfq_queue, p_node);
	if (cfq_rq_close(cfqd, cur_cfqq, __cfqq->next_rq))
		return __cfqq;

	return NULL;
}

/*
                 
                                                                         
                                          
  
                                                                          
                                                                            
                                                                            
              
 */
static struct cfq_queue *cfq_close_cooperator(struct cfq_data *cfqd,
					      struct cfq_queue *cur_cfqq)
{
	struct cfq_queue *cfqq;

	if (cfq_class_idle(cur_cfqq))
		return NULL;
	if (!cfq_cfqq_sync(cur_cfqq))
		return NULL;
	if (CFQQ_SEEKY(cur_cfqq))
		return NULL;

	/*
                                                                   
  */
	if (cur_cfqq->cfqg->nr_cfqq == 1)
		return NULL;

	/*
                                                              
                                                               
                                                           
  */
	cfqq = cfqq_close(cfqd, cur_cfqq);
	if (!cfqq)
		return NULL;

	/*                                                              */
	if (cur_cfqq->cfqg != cfqq->cfqg)
		return NULL;

	/*
                                             
  */
	if (!cfq_cfqq_sync(cfqq))
		return NULL;
	if (CFQQ_SEEKY(cfqq))
		return NULL;

	/*
                                                     
  */
	if (cfq_class_rt(cfqq) != cfq_class_rt(cur_cfqq))
		return NULL;

	return cfqq;
}

/*
                                                                  
 */

static bool cfq_should_idle(struct cfq_data *cfqd, struct cfq_queue *cfqq)
{
	enum wl_prio_t prio = cfqq_prio(cfqq);
	struct cfq_rb_root *service_tree = cfqq->service_tree;

	BUG_ON(!service_tree);
	BUG_ON(!service_tree->count);

	if (!cfqd->cfq_slice_idle)
		return false;

	/*                                    */
	if (prio == IDLE_WORKLOAD)
		return false;

	/*                                                          */
	if (cfq_cfqq_idle_window(cfqq) &&
	   !(blk_queue_nonrot(cfqd->queue) && cfqd->hw_tag))
		return true;

	/*
                                                   
                          
  */
	if (service_tree->count == 1 && cfq_cfqq_sync(cfqq) &&
	   !cfq_io_thinktime_big(cfqd, &service_tree->ttime, false))
		return true;
	cfq_log_cfqq(cfqd, cfqq, "Not idling. st->count:%d",
			service_tree->count);
	return false;
}

static void cfq_arm_slice_timer(struct cfq_data *cfqd)
{
	struct cfq_queue *cfqq = cfqd->active_queue;
	struct cfq_io_cq *cic;
	unsigned long sl, group_idle = 0;

	/*
                                                                   
                                                                       
                                 
  */
	if (blk_queue_nonrot(cfqd->queue) && cfqd->hw_tag)
		return;

	WARN_ON(!RB_EMPTY_ROOT(&cfqq->sort_list));
	WARN_ON(cfq_cfqq_slice_new(cfqq));

	/*
                                                                
  */
	if (!cfq_should_idle(cfqd, cfqq)) {
		/*                                         */
		if (cfqd->cfq_group_idle)
			group_idle = cfqd->cfq_group_idle;
		else
			return;
	}

	/*
                                                     
  */
	if (cfqq->dispatched)
		return;

	/*
                               
  */
	cic = cfqd->active_cic;
	if (!cic || !atomic_read(&cic->icq.ioc->nr_tasks))
		return;

	/*
                                                               
                                                                
               
  */
	if (sample_valid(cic->ttime.ttime_samples) &&
	    (cfqq->slice_end - jiffies < cic->ttime.ttime_mean)) {
		cfq_log_cfqq(cfqd, cfqq, "Not idling. think_time:%lu",
			     cic->ttime.ttime_mean);
		return;
	}

	/*                                                          */
	if (group_idle && cfqq->cfqg->nr_cfqq > 1)
		return;

	cfq_mark_cfqq_wait_request(cfqq);

	if (group_idle)
		sl = cfqd->cfq_group_idle;
	else
		sl = cfqd->cfq_slice_idle;

	mod_timer(&cfqd->idle_slice_timer, jiffies + sl);
	cfq_blkiocg_update_set_idle_time_stats(&cfqq->cfqg->blkg);
	cfq_log_cfqq(cfqd, cfqq, "arm_idle: %lu group_idle: %d", sl,
			group_idle ? 1 : 0);
}

/*
                                                                       
 */
static void cfq_dispatch_insert(struct request_queue *q, struct request *rq)
{
	struct cfq_data *cfqd = q->elevator->elevator_data;
	struct cfq_queue *cfqq = RQ_CFQQ(rq);

	cfq_log_cfqq(cfqd, cfqq, "dispatch_insert");

	cfqq->next_rq = cfq_find_next_rq(cfqd, cfqq, rq);
	cfq_remove_request(rq);
	cfqq->dispatched++;
	(RQ_CFQG(rq))->dispatched++;
	elv_dispatch_sort(q, rq);

	cfqd->rq_in_flight[cfq_cfqq_sync(cfqq)]++;
	cfqq->nr_sectors += blk_rq_sectors(rq);
	cfq_blkiocg_update_dispatch_stats(&cfqq->cfqg->blkg, blk_rq_bytes(rq),
					rq_data_dir(rq), rq_is_sync(rq));
}

/*
                                                                     
 */
static struct request *cfq_check_fifo(struct cfq_queue *cfqq)
{
	struct request *rq = NULL;

	if (cfq_cfqq_fifo_expire(cfqq))
		return NULL;

	cfq_mark_cfqq_fifo_expire(cfqq);

	if (list_empty(&cfqq->fifo))
		return NULL;

	rq = rq_entry_fifo(cfqq->fifo.next);
	if (time_before(jiffies, rq_fifo_time(rq)))
		rq = NULL;

	cfq_log_cfqq(cfqq->cfqd, cfqq, "fifo=%p", rq);
	return rq;
}

static inline int
cfq_prio_to_maxrq(struct cfq_data *cfqd, struct cfq_queue *cfqq)
{
	const int base_rq = cfqd->cfq_slice_async_rq;

	WARN_ON(cfqq->ioprio >= IOPRIO_BE_NR);

	return 2 * base_rq * (IOPRIO_BE_NR - cfqq->ioprio);
}

/*
                                           
 */
static int cfqq_process_refs(struct cfq_queue *cfqq)
{
	int process_refs, io_refs;

	io_refs = cfqq->allocated[READ] + cfqq->allocated[WRITE];
	process_refs = cfqq->ref - io_refs;
	BUG_ON(process_refs < 0);
	return process_refs;
}

static void cfq_setup_merge(struct cfq_queue *cfqq, struct cfq_queue *new_cfqq)
{
	int process_refs, new_process_refs;
	struct cfq_queue *__cfqq;

	/*
                                                                  
                                                                
                                                               
                            
  */
	if (!cfqq_process_refs(new_cfqq))
		return;

	/*                                                     */
	while ((__cfqq = new_cfqq->new_cfqq)) {
		if (__cfqq == cfqq)
			return;
		new_cfqq = __cfqq;
	}

	process_refs = cfqq_process_refs(cfqq);
	new_process_refs = cfqq_process_refs(new_cfqq);
	/*
                                                          
                                
  */
	if (process_refs == 0 || new_process_refs == 0)
		return;

	/*
                                                        
  */
	if (new_process_refs >= process_refs) {
		cfqq->new_cfqq = new_cfqq;
		new_cfqq->ref += process_refs;
	} else {
		new_cfqq->new_cfqq = cfqq;
		cfqq->ref += new_process_refs;
	}
}

static enum wl_type_t cfq_choose_wl(struct cfq_data *cfqd,
				struct cfq_group *cfqg, enum wl_prio_t prio)
{
	struct cfq_queue *queue;
	int i;
	bool key_valid = false;
	unsigned long lowest_key = 0;
	enum wl_type_t cur_best = SYNC_NOIDLE_WORKLOAD;

	for (i = 0; i <= SYNC_WORKLOAD; ++i) {
		/*                                   */
		queue = cfq_rb_first(service_tree_for(cfqg, prio, i));
		if (queue &&
		    (!key_valid || time_before(queue->rb_key, lowest_key))) {
			lowest_key = queue->rb_key;
			cur_best = i;
			key_valid = true;
		}
	}

	return cur_best;
}

static void choose_service_tree(struct cfq_data *cfqd, struct cfq_group *cfqg)
{
	unsigned slice;
	unsigned count;
	struct cfq_rb_root *st;
	unsigned group_slice;
	enum wl_prio_t original_prio = cfqd->serving_prio;

	/*                                      */
	if (cfq_group_busy_queues_wl(RT_WORKLOAD, cfqd, cfqg))
		cfqd->serving_prio = RT_WORKLOAD;
	else if (cfq_group_busy_queues_wl(BE_WORKLOAD, cfqd, cfqg))
		cfqd->serving_prio = BE_WORKLOAD;
	else {
		cfqd->serving_prio = IDLE_WORKLOAD;
		cfqd->workload_expires = jiffies + 1;
		return;
	}

	if (original_prio != cfqd->serving_prio)
		goto new_workload;

	/*
                                                  
                                                         
                   
  */
	st = service_tree_for(cfqg, cfqd->serving_prio, cfqd->serving_type);
	count = st->count;

	/*
                                                                        
  */
	if (count && !time_after(jiffies, cfqd->workload_expires))
		return;

new_workload:
	/*                                    */
	cfqd->serving_type =
		cfq_choose_wl(cfqd, cfqg, cfqd->serving_prio);
	st = service_tree_for(cfqg, cfqd->serving_prio, cfqd->serving_type);
	count = st->count;

	/*
                                                                  
                                                               
                                             
  */
	group_slice = cfq_group_slice(cfqd, cfqg);

	slice = group_slice * count /
		max_t(unsigned, cfqg->busy_queues_avg[cfqd->serving_prio],
		      cfq_group_busy_queues_wl(cfqd->serving_prio, cfqd, cfqg));

	if (cfqd->serving_type == ASYNC_WORKLOAD) {
		unsigned int tmp;

		/*
                                                        
                                                                
                                                             
                                                             
                                           
   */
		tmp = cfqd->cfq_target_latency *
			cfqg_busy_async_queues(cfqd, cfqg);
		tmp = tmp/cfqd->busy_queues;
		slice = min_t(unsigned, slice, tmp);

		/*                                                 
                                 */
		slice = slice * cfqd->cfq_slice[0] / cfqd->cfq_slice[1];
	} else
		/*                                                    */
		slice = max(slice, 2 * cfqd->cfq_slice_idle);

	slice = max_t(unsigned, slice, CFQ_MIN_TT);
	cfq_log(cfqd, "workload slice:%d", slice);
	cfqd->workload_expires = jiffies + slice;
}

static struct cfq_group *cfq_get_next_cfqg(struct cfq_data *cfqd)
{
	struct cfq_rb_root *st = &cfqd->grp_service_tree;
	struct cfq_group *cfqg;

	if (RB_EMPTY_ROOT(&st->rb))
		return NULL;
	cfqg = cfq_rb_first_group(st);
	update_min_vdisktime(st);
	return cfqg;
}

static void cfq_choose_cfqg(struct cfq_data *cfqd)
{
	struct cfq_group *cfqg = cfq_get_next_cfqg(cfqd);

	cfqd->serving_group = cfqg;

	/*                                */
	if (cfqg->saved_workload_slice) {
		cfqd->workload_expires = jiffies + cfqg->saved_workload_slice;
		cfqd->serving_type = cfqg->saved_workload;
		cfqd->serving_prio = cfqg->saved_serving_prio;
	} else
		cfqd->workload_expires = jiffies - 1;

	choose_service_tree(cfqd, cfqg);
}

/*
                                                                 
                                                                         
 */
static struct cfq_queue *cfq_select_queue(struct cfq_data *cfqd)
{
	struct cfq_queue *cfqq, *new_cfqq = NULL;

	cfqq = cfqd->active_queue;
	if (!cfqq)
		goto new_queue;

	if (!cfqd->rq_queued)
		return NULL;

	/*
                                                                 
  */
	if (cfq_cfqq_wait_busy(cfqq) && !RB_EMPTY_ROOT(&cfqq->sort_list))
		goto expire;

	/*
                                                                   
  */
	if (cfq_slice_used(cfqq) && !cfq_cfqq_must_dispatch(cfqq)) {
		/*
                                                               
                                                             
                                                      
    
                                                             
                                                              
                                             
   */
		if (cfqq->cfqg->nr_cfqq == 1 && RB_EMPTY_ROOT(&cfqq->sort_list)
		    && cfqq->dispatched && cfq_should_idle(cfqd, cfqq)) {
			cfqq = NULL;
			goto keep_queue;
		} else
			goto check_group_idle;
	}

	/*
                                                                
             
  */
	if (!RB_EMPTY_ROOT(&cfqq->sort_list))
		goto keep_queue;

	/*
                                                               
                                                               
                                                                   
                                                                   
  */
	new_cfqq = cfq_close_cooperator(cfqd, cfqq);
	if (new_cfqq) {
		if (!cfqq->new_cfqq)
			cfq_setup_merge(cfqq, new_cfqq);
		goto expire;
	}

	/*
                                                                  
                                                                
                                                                    
  */
	if (timer_pending(&cfqd->idle_slice_timer)) {
		cfqq = NULL;
		goto keep_queue;
	}

	/*
                                                                 
                                     
   */
	if (CFQQ_SEEKY(cfqq) && cfq_cfqq_idle_window(cfqq) &&
	    (cfq_cfqq_slice_new(cfqq) ||
	    (cfqq->slice_end - jiffies > jiffies - cfqq->slice_start))) {
		cfq_clear_cfqq_deep(cfqq);
		cfq_clear_cfqq_idle_window(cfqq);
	}

	if (cfqq->dispatched && cfq_should_idle(cfqd, cfqq)) {
		cfqq = NULL;
		goto keep_queue;
	}

	/*
                                                                   
                                              
  */
check_group_idle:
	if (cfqd->cfq_group_idle && cfqq->cfqg->nr_cfqq == 1 &&
	    cfqq->cfqg->dispatched &&
	    !cfq_io_thinktime_big(cfqd, &cfqq->cfqg->ttime, true)) {
		cfqq = NULL;
		goto keep_queue;
	}

expire:
	cfq_slice_expired(cfqd, 0);
new_queue:
	/*
                                                              
                
  */
	if (!new_cfqq)
		cfq_choose_cfqg(cfqd);

	cfqq = cfq_set_active_queue(cfqd, new_cfqq);
keep_queue:
	return cfqq;
}

static int __cfq_forced_dispatch_cfqq(struct cfq_queue *cfqq)
{
	int dispatched = 0;

	while (cfqq->next_rq) {
		cfq_dispatch_insert(cfqq->cfqd->queue, cfqq->next_rq);
		dispatched++;
	}

	BUG_ON(!list_empty(&cfqq->fifo));

	/*                                                                 */
	__cfq_slice_expired(cfqq->cfqd, cfqq, 0);
	return dispatched;
}

/*
                                                                   
                            
 */
static int cfq_forced_dispatch(struct cfq_data *cfqd)
{
	struct cfq_queue *cfqq;
	int dispatched = 0;

	/*                                                        */
	cfq_slice_expired(cfqd, 0);
	while ((cfqq = cfq_get_next_queue_forced(cfqd)) != NULL) {
		__cfq_set_active_queue(cfqd, cfqq);
		dispatched += __cfq_forced_dispatch_cfqq(cfqq);
	}

	BUG_ON(cfqd->busy_queues);

	cfq_log(cfqd, "forced_dispatch=%d", dispatched);
	return dispatched;
}

static inline bool cfq_slice_used_soon(struct cfq_data *cfqd,
	struct cfq_queue *cfqq)
{
	/*                                                       */
	if (cfq_cfqq_slice_new(cfqq))
		return true;
	if (time_after(jiffies + cfqd->cfq_slice_idle * cfqq->dispatched,
		cfqq->slice_end))
		return true;

	return false;
}

static bool cfq_may_dispatch(struct cfq_data *cfqd, struct cfq_queue *cfqq)
{
	unsigned int max_dispatch;

	/*
                                                
  */
	if (cfq_should_idle(cfqd, cfqq) && cfqd->rq_in_flight[BLK_RW_ASYNC])
		return false;

	/*
                                                                        
  */
	if (cfqd->rq_in_flight[BLK_RW_SYNC] && !cfq_cfqq_sync(cfqq))
		return false;

	max_dispatch = max_t(unsigned int, cfqd->cfq_quantum / 2, 1);
	if (cfq_class_idle(cfqq))
		max_dispatch = 1;

	/*
                                                      
  */
	if (cfqq->dispatched >= max_dispatch) {
		bool promote_sync = false;
		/*
                                                           
   */
		if (cfq_class_idle(cfqq))
			return false;

		/*
                                    
                                                     
                                                            
                                                              
                                              
   */
		if (cfq_cfqq_sync(cfqq) && cfqd->busy_sync_queues == 1)
			promote_sync = true;

		/*
                                                            
   */
		if (cfqd->busy_queues > 1 && cfq_slice_used_soon(cfqd, cfqq) &&
				!promote_sync)
			return false;

		/*
                              
   */
		if (cfqd->busy_queues == 1 || promote_sync)
			max_dispatch = -1;
		else
			/*
                                                          
                                                     
                                                   
                                              
      */
			max_dispatch = cfqd->cfq_quantum;
	}

	/*
                                                               
                                                              
                                         
  */
	if (!cfq_cfqq_sync(cfqq) && cfqd->cfq_latency) {
		unsigned long last_sync = jiffies - cfqd->last_delayed_sync;
		unsigned int depth;

		depth = last_sync / cfqd->cfq_slice[1];
		if (!depth && !cfqq->dispatched)
			depth = 1;
		if (depth < max_dispatch)
			max_dispatch = depth;
	}

	/*
                                                    
  */
	return cfqq->dispatched < max_dispatch;
}

/*
                                                                 
                 
 */
static bool cfq_dispatch_request(struct cfq_data *cfqd, struct cfq_queue *cfqq)
{
	struct request *rq;

	BUG_ON(RB_EMPTY_ROOT(&cfqq->sort_list));

	if (!cfq_may_dispatch(cfqd, cfqq))
		return false;

	/*
                                                      
  */
	rq = cfq_check_fifo(cfqq);
	if (!rq)
		rq = cfqq->next_rq;

	/*
                                            
  */
	cfq_dispatch_insert(cfqd->queue, rq);

	if (!cfqd->active_cic) {
		struct cfq_io_cq *cic = RQ_CIC(rq);

		atomic_long_inc(&cic->icq.ioc->refcount);
		cfqd->active_cic = cic;
	}

	return true;
}

/*
                                                                            
                
 */
static int cfq_dispatch_requests(struct request_queue *q, int force)
{
	struct cfq_data *cfqd = q->elevator->elevator_data;
	struct cfq_queue *cfqq;

	if (!cfqd->busy_queues)
		return 0;

	if (unlikely(force))
		return cfq_forced_dispatch(cfqd);

	cfqq = cfq_select_queue(cfqd);
	if (!cfqq)
		return 0;

	/*
                                                       
  */
	if (!cfq_dispatch_request(cfqd, cfqq))
		return 0;

	cfqq->slice_dispatch++;
	cfq_clear_cfqq_must_dispatch(cfqq);

	/*
                                                                       
                                               
  */
	if (cfqd->busy_queues > 1 && ((!cfq_cfqq_sync(cfqq) &&
	    cfqq->slice_dispatch >= cfq_prio_to_maxrq(cfqd, cfqq)) ||
	    cfq_class_idle(cfqq))) {
		cfqq->slice_end = jiffies + 1;
		cfq_slice_expired(cfqd, 0);
	}

	cfq_log_cfqq(cfqd, cfqq, "dispatched a request");
	return 1;
}

/*
                                                                          
                                                                            
  
                                                                    
                                
 */
static void cfq_put_queue(struct cfq_queue *cfqq)
{
	struct cfq_data *cfqd = cfqq->cfqd;
	struct cfq_group *cfqg;

	BUG_ON(cfqq->ref <= 0);

	cfqq->ref--;
	if (cfqq->ref)
		return;

	cfq_log_cfqq(cfqd, cfqq, "put_queue");
	BUG_ON(rb_first(&cfqq->sort_list));
	BUG_ON(cfqq->allocated[READ] + cfqq->allocated[WRITE]);
	cfqg = cfqq->cfqg;

	if (unlikely(cfqd->active_queue == cfqq)) {
		__cfq_slice_expired(cfqd, cfqq, 0);
		cfq_schedule_dispatch(cfqd);
	}

	BUG_ON(cfq_cfqq_on_rr(cfqq));
	kmem_cache_free(cfq_pool, cfqq);
	cfq_put_cfqg(cfqg);
}

static void cfq_put_cooperator(struct cfq_queue *cfqq)
{
	struct cfq_queue *__cfqq, *next;

	/*
                                                               
                                                                 
                                                               
  */
	__cfqq = cfqq->new_cfqq;
	while (__cfqq) {
		if (__cfqq == cfqq) {
			WARN(1, "cfqq->new_cfqq loop detected\n");
			break;
		}
		next = __cfqq->new_cfqq;
		cfq_put_queue(__cfqq);
		__cfqq = next;
	}
}

static void cfq_exit_cfqq(struct cfq_data *cfqd, struct cfq_queue *cfqq)
{
	if (unlikely(cfqq == cfqd->active_queue)) {
		__cfq_slice_expired(cfqd, cfqq, 0);
		cfq_schedule_dispatch(cfqd);
	}

	cfq_put_cooperator(cfqq);

	cfq_put_queue(cfqq);
}

static void cfq_init_icq(struct io_cq *icq)
{
	struct cfq_io_cq *cic = icq_to_cic(icq);

	cic->ttime.last_end_request = jiffies;
}

static void cfq_exit_icq(struct io_cq *icq)
{
	struct cfq_io_cq *cic = icq_to_cic(icq);
	struct cfq_data *cfqd = cic_to_cfqd(cic);

	if (cic->cfqq[BLK_RW_ASYNC]) {
		cfq_exit_cfqq(cfqd, cic->cfqq[BLK_RW_ASYNC]);
		cic->cfqq[BLK_RW_ASYNC] = NULL;
	}

	if (cic->cfqq[BLK_RW_SYNC]) {
		cfq_exit_cfqq(cfqd, cic->cfqq[BLK_RW_SYNC]);
		cic->cfqq[BLK_RW_SYNC] = NULL;
	}
}

static void cfq_init_prio_data(struct cfq_queue *cfqq, struct io_context *ioc)
{
	struct task_struct *tsk = current;
	int ioprio_class;

	if (!cfq_cfqq_prio_changed(cfqq))
		return;

	ioprio_class = IOPRIO_PRIO_CLASS(ioc->ioprio);
	switch (ioprio_class) {
	default:
		printk(KERN_ERR "cfq: bad prio %x\n", ioprio_class);
	case IOPRIO_CLASS_NONE:
		/*
                                                 
   */
		cfqq->ioprio = task_nice_ioprio(tsk);
		cfqq->ioprio_class = task_nice_ioclass(tsk);
		break;
	case IOPRIO_CLASS_RT:
		cfqq->ioprio = task_ioprio(ioc);
		cfqq->ioprio_class = IOPRIO_CLASS_RT;
		break;
	case IOPRIO_CLASS_BE:
		cfqq->ioprio = task_ioprio(ioc);
		cfqq->ioprio_class = IOPRIO_CLASS_BE;
		break;
	case IOPRIO_CLASS_IDLE:
		cfqq->ioprio_class = IOPRIO_CLASS_IDLE;
		cfqq->ioprio = 7;
		cfq_clear_cfqq_idle_window(cfqq);
		break;
	}

	/*
                                                                       
                                      
  */
	cfqq->org_ioprio = cfqq->ioprio;
	cfq_clear_cfqq_prio_changed(cfqq);
}

static void changed_ioprio(struct cfq_io_cq *cic)
{
	struct cfq_data *cfqd = cic_to_cfqd(cic);
	struct cfq_queue *cfqq;

	if (unlikely(!cfqd))
		return;

	cfqq = cic->cfqq[BLK_RW_ASYNC];
	if (cfqq) {
		struct cfq_queue *new_cfqq;
		new_cfqq = cfq_get_queue(cfqd, BLK_RW_ASYNC, cic->icq.ioc,
						GFP_ATOMIC);
		if (new_cfqq) {
			cic->cfqq[BLK_RW_ASYNC] = new_cfqq;
			cfq_put_queue(cfqq);
		}
	}

	cfqq = cic->cfqq[BLK_RW_SYNC];
	if (cfqq)
		cfq_mark_cfqq_prio_changed(cfqq);
}

static void cfq_init_cfqq(struct cfq_data *cfqd, struct cfq_queue *cfqq,
			  pid_t pid, bool is_sync)
{
	RB_CLEAR_NODE(&cfqq->rb_node);
	RB_CLEAR_NODE(&cfqq->p_node);
	INIT_LIST_HEAD(&cfqq->fifo);

	cfqq->ref = 0;
	cfqq->cfqd = cfqd;

	cfq_mark_cfqq_prio_changed(cfqq);

	if (is_sync) {
		if (!cfq_class_idle(cfqq))
			cfq_mark_cfqq_idle_window(cfqq);
		cfq_mark_cfqq_sync(cfqq);
	}
	cfqq->pid = pid;
}

#ifdef CONFIG_CFQ_GROUP_IOSCHED
static void changed_cgroup(struct cfq_io_cq *cic)
{
	struct cfq_queue *sync_cfqq = cic_to_cfqq(cic, 1);
	struct cfq_data *cfqd = cic_to_cfqd(cic);
	struct request_queue *q;

	if (unlikely(!cfqd))
		return;

	q = cfqd->queue;

	if (sync_cfqq) {
		/*
                                                           
                                                           
   */
		cfq_log_cfqq(cfqd, sync_cfqq, "changed cgroup");
		cic_set_cfqq(cic, NULL, 1);
		cfq_put_queue(sync_cfqq);
	}
}
#endif  /*                          */

static struct cfq_queue *
cfq_find_alloc_queue(struct cfq_data *cfqd, bool is_sync,
		     struct io_context *ioc, gfp_t gfp_mask)
{
	struct cfq_queue *cfqq, *new_cfqq = NULL;
	struct cfq_io_cq *cic;
	struct cfq_group *cfqg;

retry:
	cfqg = cfq_get_cfqg(cfqd);
	cic = cfq_cic_lookup(cfqd, ioc);
	/*                        */
	cfqq = cic_to_cfqq(cic, is_sync);

	/*
                                                          
                                                              
  */
	if (!cfqq || cfqq == &cfqd->oom_cfqq) {
		cfqq = NULL;
		if (new_cfqq) {
			cfqq = new_cfqq;
			new_cfqq = NULL;
		} else if (gfp_mask & __GFP_WAIT) {
			spin_unlock_irq(cfqd->queue->queue_lock);
			new_cfqq = kmem_cache_alloc_node(cfq_pool,
					gfp_mask | __GFP_ZERO,
					cfqd->queue->node);
			spin_lock_irq(cfqd->queue->queue_lock);
			if (new_cfqq)
				goto retry;
		} else {
			cfqq = kmem_cache_alloc_node(cfq_pool,
					gfp_mask | __GFP_ZERO,
					cfqd->queue->node);
		}

		if (cfqq) {
			cfq_init_cfqq(cfqd, cfqq, current->pid, is_sync);
			cfq_init_prio_data(cfqq, ioc);
			cfq_link_cfqq_cfqg(cfqq, cfqg);
			cfq_log_cfqq(cfqd, cfqq, "alloced");
		} else
			cfqq = &cfqd->oom_cfqq;
	}

	if (new_cfqq)
		kmem_cache_free(cfq_pool, new_cfqq);

	return cfqq;
}

static struct cfq_queue **
cfq_async_queue_prio(struct cfq_data *cfqd, int ioprio_class, int ioprio)
{
	switch (ioprio_class) {
	case IOPRIO_CLASS_RT:
		return &cfqd->async_cfqq[0][ioprio];
	case IOPRIO_CLASS_BE:
		return &cfqd->async_cfqq[1][ioprio];
	case IOPRIO_CLASS_IDLE:
		return &cfqd->async_idle_cfqq;
	default:
		BUG();
	}
}

static struct cfq_queue *
cfq_get_queue(struct cfq_data *cfqd, bool is_sync, struct io_context *ioc,
	      gfp_t gfp_mask)
{
	const int ioprio = task_ioprio(ioc);
	const int ioprio_class = task_ioprio_class(ioc);
	struct cfq_queue **async_cfqq = NULL;
	struct cfq_queue *cfqq = NULL;

	if (!is_sync) {
		async_cfqq = cfq_async_queue_prio(cfqd, ioprio_class, ioprio);
		cfqq = *async_cfqq;
	}

	if (!cfqq)
		cfqq = cfq_find_alloc_queue(cfqd, is_sync, ioc, gfp_mask);

	/*
                                                                       
  */
	if (!is_sync && !(*async_cfqq)) {
		cfqq->ref++;
		*async_cfqq = cfqq;
	}

	cfqq->ref++;
	return cfqq;
}

static void
__cfq_update_io_thinktime(struct cfq_ttime *ttime, unsigned long slice_idle)
{
	unsigned long elapsed = jiffies - ttime->last_end_request;
	elapsed = min(elapsed, 2UL * slice_idle);

	ttime->ttime_samples = (7*ttime->ttime_samples + 256) / 8;
	ttime->ttime_total = (7*ttime->ttime_total + 256*elapsed) / 8;
	ttime->ttime_mean = (ttime->ttime_total + 128) / ttime->ttime_samples;
}

static void
cfq_update_io_thinktime(struct cfq_data *cfqd, struct cfq_queue *cfqq,
			struct cfq_io_cq *cic)
{
	if (cfq_cfqq_sync(cfqq)) {
		__cfq_update_io_thinktime(&cic->ttime, cfqd->cfq_slice_idle);
		__cfq_update_io_thinktime(&cfqq->service_tree->ttime,
			cfqd->cfq_slice_idle);
	}
#ifdef CONFIG_CFQ_GROUP_IOSCHED
	__cfq_update_io_thinktime(&cfqq->cfqg->ttime, cfqd->cfq_group_idle);
#endif
}

static void
cfq_update_io_seektime(struct cfq_data *cfqd, struct cfq_queue *cfqq,
		       struct request *rq)
{
	sector_t sdist = 0;
	sector_t n_sec = blk_rq_sectors(rq);
	if (cfqq->last_request_pos) {
		if (cfqq->last_request_pos < blk_rq_pos(rq))
			sdist = blk_rq_pos(rq) - cfqq->last_request_pos;
		else
			sdist = cfqq->last_request_pos - blk_rq_pos(rq);
	}

	cfqq->seek_history <<= 1;
	if (blk_queue_nonrot(cfqd->queue))
		cfqq->seek_history |= (n_sec < CFQQ_SECT_THR_NONROT);
	else
		cfqq->seek_history |= (sdist > CFQQ_SEEK_THR);
}

/*
                                                                           
                    
 */
static void
cfq_update_idle_window(struct cfq_data *cfqd, struct cfq_queue *cfqq,
		       struct cfq_io_cq *cic)
{
	int old_idle, enable_idle;

	/*
                                              
  */
	if (!cfq_cfqq_sync(cfqq) || cfq_class_idle(cfqq))
		return;

	enable_idle = old_idle = cfq_cfqq_idle_window(cfqq);

	if (cfqq->queued[0] + cfqq->queued[1] >= 4)
		cfq_mark_cfqq_deep(cfqq);

	if (cfqq->next_rq && (cfqq->next_rq->cmd_flags & REQ_NOIDLE))
		enable_idle = 0;
	else if (!atomic_read(&cic->icq.ioc->nr_tasks) ||
		 !cfqd->cfq_slice_idle ||
		 (!cfq_cfqq_deep(cfqq) && CFQQ_SEEKY(cfqq)))
		enable_idle = 0;
	else if (sample_valid(cic->ttime.ttime_samples)) {
		if (cic->ttime.ttime_mean > cfqd->cfq_slice_idle)
			enable_idle = 0;
		else
			enable_idle = 1;
	}

	if (old_idle != enable_idle) {
		cfq_log_cfqq(cfqd, cfqq, "idle=%d", enable_idle);
		if (enable_idle)
			cfq_mark_cfqq_idle_window(cfqq);
		else
			cfq_clear_cfqq_idle_window(cfqq);
	}
}

/*
                                                                            
                                                     
 */
static bool
cfq_should_preempt(struct cfq_data *cfqd, struct cfq_queue *new_cfqq,
		   struct request *rq)
{
	struct cfq_queue *cfqq;

	cfqq = cfqd->active_queue;
	if (!cfqq)
		return false;

	if (cfq_class_idle(new_cfqq))
		return false;

	if (cfq_class_idle(cfqq))
		return true;

	/*
                                                                         
  */
	if (cfq_class_rt(cfqq) && !cfq_class_rt(new_cfqq))
		return false;

	/*
                                                                  
                                            
  */
	if (rq_is_sync(rq) && !cfq_cfqq_sync(cfqq))
		return true;

	if (new_cfqq->cfqg != cfqq->cfqg)
		return false;

	if (cfq_slice_used(cfqq))
		return true;

	/*                                                            */
	if (cfqd->serving_type == SYNC_NOIDLE_WORKLOAD &&
	    cfqq_type(new_cfqq) == SYNC_NOIDLE_WORKLOAD &&
	    new_cfqq->service_tree->count == 2 &&
	    RB_EMPTY_ROOT(&cfqq->sort_list))
		return true;

	/*
                                                                 
                                                                      
  */
	if ((rq->cmd_flags & REQ_PRIO) && !cfqq->prio_pending)
		return true;

	/*
                                                                     
  */
	if (cfq_class_rt(new_cfqq) && !cfq_class_rt(cfqq))
		return true;

	/*                                                      */
	if (RB_EMPTY_ROOT(&cfqq->sort_list) && !cfq_should_idle(cfqd, cfqq))
		return true;

	if (!cfqd->active_cic || !cfq_cfqq_wait_request(cfqq))
		return false;

	/*
                                                              
                                
  */
	if (cfq_rq_close(cfqd, cfqq, rq))
		return true;

	return false;
}

/*
                                                                            
                                         
 */
static void cfq_preempt_queue(struct cfq_data *cfqd, struct cfq_queue *cfqq)
{
	enum wl_type_t old_type = cfqq_type(cfqd->active_queue);

	cfq_log_cfqq(cfqd, cfqq, "preempt");
	cfq_slice_expired(cfqd, 1);

	/*
                                                                 
                  
  */
	if (old_type != cfqq_type(cfqq))
		cfqq->cfqg->saved_workload_slice = 0;

	/*
                                                              
                                             
  */
	BUG_ON(!cfq_cfqq_on_rr(cfqq));

	cfq_service_tree_add(cfqd, cfqq, 1);

	cfqq->slice_end = 0;
	cfq_mark_cfqq_slice_new(cfqq);
}

/*
                                                                         
                                  
 */
static void
cfq_rq_enqueued(struct cfq_data *cfqd, struct cfq_queue *cfqq,
		struct request *rq)
{
	struct cfq_io_cq *cic = RQ_CIC(rq);

	cfqd->rq_queued++;
	if (rq->cmd_flags & REQ_PRIO)
		cfqq->prio_pending++;

	cfq_update_io_thinktime(cfqd, cfqq, cic);
	cfq_update_io_seektime(cfqd, cfqq, rq);
	cfq_update_idle_window(cfqd, cfqq, cic);

	cfqq->last_request_pos = blk_rq_pos(rq) + blk_rq_sectors(rq);

	if (cfqq == cfqd->active_queue) {
		/*
                                                          
                                                                
                                                             
                                                                
                                                               
                                                          
                                                          
                                           
   */
		if (cfq_cfqq_wait_request(cfqq)) {
			if (blk_rq_bytes(rq) > PAGE_CACHE_SIZE ||
			    cfqd->busy_queues > 1) {
				cfq_del_timer(cfqd, cfqq);
				cfq_clear_cfqq_wait_request(cfqq);
				__blk_run_queue(cfqd->queue);
			} else {
				cfq_blkiocg_update_idle_time_stats(
						&cfqq->cfqg->blkg);
				cfq_mark_cfqq_must_dispatch(cfqq);
			}
		}
	} else if (cfq_should_preempt(cfqd, cfqq, rq)) {
		/*
                                                         
                                                               
                                                              
                                                   
   */
		cfq_preempt_queue(cfqd, cfqq);
		__blk_run_queue(cfqd->queue);
	}
}

static void cfq_insert_request(struct request_queue *q, struct request *rq)
{
	struct cfq_data *cfqd = q->elevator->elevator_data;
	struct cfq_queue *cfqq = RQ_CFQQ(rq);

	cfq_log_cfqq(cfqd, cfqq, "insert_request");
	cfq_init_prio_data(cfqq, RQ_CIC(rq)->icq.ioc);

	rq_set_fifo_time(rq, jiffies + cfqd->cfq_fifo_expire[rq_is_sync(rq)]);
	list_add_tail(&rq->queuelist, &cfqq->fifo);
	cfq_add_rq_rb(rq);
	cfq_blkiocg_update_io_add_stats(&(RQ_CFQG(rq))->blkg,
			&cfqd->serving_group->blkg, rq_data_dir(rq),
			rq_is_sync(rq));
	cfq_rq_enqueued(cfqd, cfqq, rq);
}

/*
                                                                
                   
 */
static void cfq_update_hw_tag(struct cfq_data *cfqd)
{
	struct cfq_queue *cfqq = cfqd->active_queue;

	if (cfqd->rq_in_driver > cfqd->hw_tag_est_depth)
		cfqd->hw_tag_est_depth = cfqd->rq_in_driver;

	if (cfqd->hw_tag == 1)
		return;

	if (cfqd->rq_queued <= CFQ_HW_QUEUE_MIN &&
	    cfqd->rq_in_driver <= CFQ_HW_QUEUE_MIN)
		return;

	/*
                                                                      
                                                                       
        
  */
	if (cfqq && cfq_cfqq_idle_window(cfqq) &&
	    cfqq->dispatched + cfqq->queued[0] + cfqq->queued[1] <
	    CFQ_HW_QUEUE_MIN && cfqd->rq_in_driver < CFQ_HW_QUEUE_MIN)
		return;

	if (cfqd->hw_tag_samples++ < 50)
		return;

	if (cfqd->hw_tag_est_depth >= CFQ_HW_QUEUE_MIN)
		cfqd->hw_tag = 1;
	else
		cfqd->hw_tag = 0;
}

static bool cfq_should_wait_busy(struct cfq_data *cfqd, struct cfq_queue *cfqq)
{
	struct cfq_io_cq *cic = cfqd->active_cic;

	/*                                               */
	if (!RB_EMPTY_ROOT(&cfqq->sort_list))
		return false;

	/*                                                    */
	if (cfqq->cfqg->nr_cfqq > 1)
		return false;

	/*                                                    */
	if (cfq_io_thinktime_big(cfqd, &cfqq->cfqg->ttime, true))
		return false;

	if (cfq_slice_used(cfqq))
		return true;

	/*                                                  */
	if (cic && sample_valid(cic->ttime.ttime_samples)
	    && (cfqq->slice_end - jiffies < cic->ttime.ttime_mean))
		return true;

	/*
                                                                   
                                                                    
                                                                     
                                                                   
                                              
  */
	if (cfqq->slice_end - jiffies == 1)
		return true;

	return false;
}

static void cfq_completed_request(struct request_queue *q, struct request *rq)
{
	struct cfq_queue *cfqq = RQ_CFQQ(rq);
	struct cfq_data *cfqd = cfqq->cfqd;
	const int sync = rq_is_sync(rq);
	unsigned long now;

	now = jiffies;
	cfq_log_cfqq(cfqd, cfqq, "complete rqnoidle %d",
		     !!(rq->cmd_flags & REQ_NOIDLE));

	cfq_update_hw_tag(cfqd);

	WARN_ON(!cfqd->rq_in_driver);
	WARN_ON(!cfqq->dispatched);
	cfqd->rq_in_driver--;
	cfqq->dispatched--;
	(RQ_CFQG(rq))->dispatched--;
	cfq_blkiocg_update_completion_stats(&cfqq->cfqg->blkg,
			rq_start_time_ns(rq), rq_io_start_time_ns(rq),
			rq_data_dir(rq), rq_is_sync(rq));

	cfqd->rq_in_flight[cfq_cfqq_sync(cfqq)]--;

	if (sync) {
		struct cfq_rb_root *service_tree;

		RQ_CIC(rq)->ttime.last_end_request = now;

		if (cfq_cfqq_on_rr(cfqq))
			service_tree = cfqq->service_tree;
		else
			service_tree = service_tree_for(cfqq->cfqg,
				cfqq_prio(cfqq), cfqq_type(cfqq));
		service_tree->ttime.last_end_request = now;
		if (!time_after(rq->start_time + cfqd->cfq_fifo_expire[1], now))
			cfqd->last_delayed_sync = now;
	}

#ifdef CONFIG_CFQ_GROUP_IOSCHED
	cfqq->cfqg->ttime.last_end_request = now;
#endif

	/*
                                                                 
                                                             
  */
	if (cfqd->active_queue == cfqq) {
		const bool cfqq_empty = RB_EMPTY_ROOT(&cfqq->sort_list);

		if (cfq_cfqq_slice_new(cfqq)) {
			cfq_set_prio_slice(cfqd, cfqq);
			cfq_clear_cfqq_slice_new(cfqq);
		}

		/*
                                                                
               
   */
		if (cfq_should_wait_busy(cfqd, cfqq)) {
			unsigned long extend_sl = cfqd->cfq_slice_idle;
			if (!cfqd->cfq_slice_idle)
				extend_sl = cfqd->cfq_group_idle;
			cfqq->slice_end = jiffies + extend_sl;
			cfq_mark_cfqq_wait_busy(cfqq);
			cfq_log_cfqq(cfqd, cfqq, "will busy wait");
		}

		/*
                              
                     
                           
                   
                                             
                                       
   */
		if (cfq_slice_used(cfqq) || cfq_class_idle(cfqq))
			cfq_slice_expired(cfqd, 1);
		else if (sync && cfqq_empty &&
			 !cfq_close_cooperator(cfqd, cfqq)) {
			cfq_arm_slice_timer(cfqd);
		}
	}

	if (!cfqd->rq_in_driver)
		cfq_schedule_dispatch(cfqd);
}

static inline int __cfq_may_queue(struct cfq_queue *cfqq)
{
	if (cfq_cfqq_wait_request(cfqq) && !cfq_cfqq_must_alloc_slice(cfqq)) {
		cfq_mark_cfqq_must_alloc_slice(cfqq);
		return ELV_MQUEUE_MUST;
	}

	return ELV_MQUEUE_MAY;
}

static int cfq_may_queue(struct request_queue *q, int rw)
{
	struct cfq_data *cfqd = q->elevator->elevator_data;
	struct task_struct *tsk = current;
	struct cfq_io_cq *cic;
	struct cfq_queue *cfqq;

	/*
                                                                  
                                                                      
                                                                   
                 
  */
	cic = cfq_cic_lookup(cfqd, tsk->io_context);
	if (!cic)
		return ELV_MQUEUE_MAY;

	cfqq = cic_to_cfqq(cic, rw_is_sync(rw));
	if (cfqq) {
		cfq_init_prio_data(cfqq, cic->icq.ioc);

		return __cfq_may_queue(cfqq);
	}

	return ELV_MQUEUE_MAY;
}

/*
                       
 */
static void cfq_put_request(struct request *rq)
{
	struct cfq_queue *cfqq = RQ_CFQQ(rq);

	if (cfqq) {
		const int rw = rq_data_dir(rq);

		BUG_ON(!cfqq->allocated[rw]);
		cfqq->allocated[rw]--;

		/*                               */
		cfq_put_cfqg(RQ_CFQG(rq));
		rq->elv.priv[0] = NULL;
		rq->elv.priv[1] = NULL;

		cfq_put_queue(cfqq);
	}
}

static struct cfq_queue *
cfq_merge_cfqqs(struct cfq_data *cfqd, struct cfq_io_cq *cic,
		struct cfq_queue *cfqq)
{
	cfq_log_cfqq(cfqd, cfqq, "merging with queue %p", cfqq->new_cfqq);
	cic_set_cfqq(cic, cfqq->new_cfqq, 1);
	cfq_mark_cfqq_coop(cfqq->new_cfqq);
	cfq_put_queue(cfqq);
	return cic_to_cfqq(cic, 1);
}

/*
                                                                          
                                               
 */
static struct cfq_queue *
split_cfqq(struct cfq_io_cq *cic, struct cfq_queue *cfqq)
{
	if (cfqq_process_refs(cfqq) == 1) {
		cfqq->pid = current->pid;
		cfq_clear_cfqq_coop(cfqq);
		cfq_clear_cfqq_split_coop(cfqq);
		return cfqq;
	}

	cic_set_cfqq(cic, NULL, 1);

	cfq_put_cooperator(cfqq);

	cfq_put_queue(cfqq);
	return NULL;
}
/*
                                                             
 */
static int
cfq_set_request(struct request_queue *q, struct request *rq, gfp_t gfp_mask)
{
	struct cfq_data *cfqd = q->elevator->elevator_data;
	struct cfq_io_cq *cic = icq_to_cic(rq->elv.icq);
	const int rw = rq_data_dir(rq);
	const bool is_sync = rq_is_sync(rq);
	struct cfq_queue *cfqq;
	unsigned int changed;

	might_sleep_if(gfp_mask & __GFP_WAIT);

	spin_lock_irq(q->queue_lock);

	/*                              */
	changed = icq_get_changed(&cic->icq);
	if (unlikely(changed & ICQ_IOPRIO_CHANGED))
		changed_ioprio(cic);
#ifdef CONFIG_CFQ_GROUP_IOSCHED
	if (unlikely(changed & ICQ_CGROUP_CHANGED))
		changed_cgroup(cic);
#endif

new_queue:
	cfqq = cic_to_cfqq(cic, is_sync);
	if (!cfqq || cfqq == &cfqd->oom_cfqq) {
		cfqq = cfq_get_queue(cfqd, is_sync, cic->icq.ioc, gfp_mask);
		cic_set_cfqq(cic, cfqq, is_sync);
	} else {
		/*
                                                         
   */
		if (cfq_cfqq_coop(cfqq) && cfq_cfqq_split_coop(cfqq)) {
			cfq_log_cfqq(cfqd, cfqq, "breaking apart cfqq");
			cfqq = split_cfqq(cic, cfqq);
			if (!cfqq)
				goto new_queue;
		}

		/*
                                                          
                                                        
                                                               
                                                        
   */
		if (cfqq->new_cfqq)
			cfqq = cfq_merge_cfqqs(cfqd, cic, cfqq);
	}

	cfqq->allocated[rw]++;

	cfqq->ref++;
	rq->elv.priv[0] = cfqq;
	rq->elv.priv[1] = cfq_ref_get_cfqg(cfqq->cfqg);
	spin_unlock_irq(q->queue_lock);
	return 0;
}

static void cfq_kick_queue(struct work_struct *work)
{
	struct cfq_data *cfqd =
		container_of(work, struct cfq_data, unplug_work);
	struct request_queue *q = cfqd->queue;

	spin_lock_irq(q->queue_lock);
	__blk_run_queue(cfqd->queue);
	spin_unlock_irq(q->queue_lock);
}

/*
                                                                              
 */
static void cfq_idle_slice_timer(unsigned long data)
{
	struct cfq_data *cfqd = (struct cfq_data *) data;
	struct cfq_queue *cfqq;
	unsigned long flags;
	int timed_out = 1;

	cfq_log(cfqd, "idle timer fired");

	spin_lock_irqsave(cfqd->queue->queue_lock, flags);

	cfqq = cfqd->active_queue;
	if (cfqq) {
		timed_out = 0;

		/*
                                                              
   */
		if (cfq_cfqq_must_dispatch(cfqq))
			goto out_kick;

		/*
            
   */
		if (cfq_slice_used(cfqq))
			goto expire;

		/*
                                                           
                                       
   */
		if (!cfqd->busy_queues)
			goto out_cont;

		/*
                                                              
   */
		if (!RB_EMPTY_ROOT(&cfqq->sort_list))
			goto out_kick;

		/*
                                                                
   */
		cfq_clear_cfqq_deep(cfqq);
	}
expire:
	cfq_slice_expired(cfqd, timed_out);
out_kick:
	cfq_schedule_dispatch(cfqd);
out_cont:
	spin_unlock_irqrestore(cfqd->queue->queue_lock, flags);
}

static void cfq_shutdown_timer_wq(struct cfq_data *cfqd)
{
	del_timer_sync(&cfqd->idle_slice_timer);
	cancel_work_sync(&cfqd->unplug_work);
}

static void cfq_put_async_queues(struct cfq_data *cfqd)
{
	int i;

	for (i = 0; i < IOPRIO_BE_NR; i++) {
		if (cfqd->async_cfqq[0][i])
			cfq_put_queue(cfqd->async_cfqq[0][i]);
		if (cfqd->async_cfqq[1][i])
			cfq_put_queue(cfqd->async_cfqq[1][i]);
	}

	if (cfqd->async_idle_cfqq)
		cfq_put_queue(cfqd->async_idle_cfqq);
}

static void cfq_exit_queue(struct elevator_queue *e)
{
	struct cfq_data *cfqd = e->elevator_data;
	struct request_queue *q = cfqd->queue;
	bool wait = false;

	cfq_shutdown_timer_wq(cfqd);

	spin_lock_irq(q->queue_lock);

	if (cfqd->active_queue)
		__cfq_slice_expired(cfqd, cfqd->active_queue, 0);

	cfq_put_async_queues(cfqd);
	cfq_release_cfq_groups(cfqd);

	/*
                                                                  
                                               
  */
	if (cfqd->nr_blkcg_linked_grps)
		wait = true;

	spin_unlock_irq(q->queue_lock);

	cfq_shutdown_timer_wq(cfqd);

	/*
                                                                   
                                                            
                                                              
                                                                   
                     
   
                                                                      
                                                                        
                                                                       
  */
	if (wait)
		synchronize_rcu();

#ifdef CONFIG_CFQ_GROUP_IOSCHED
	/*                                      */
	free_percpu(cfqd->root_group.blkg.stats_cpu);
#endif
	kfree(cfqd);
}

static void *cfq_init_queue(struct request_queue *q)
{
	struct cfq_data *cfqd;
	int i, j;
	struct cfq_group *cfqg;
	struct cfq_rb_root *st;

	cfqd = kmalloc_node(sizeof(*cfqd), GFP_KERNEL | __GFP_ZERO, q->node);
	if (!cfqd)
		return NULL;

	/*                        */
	cfqd->grp_service_tree = CFQ_RB_ROOT;

	/*                 */
	cfqg = &cfqd->root_group;
	for_each_cfqg_st(cfqg, i, j, st)
		*st = CFQ_RB_ROOT;
	RB_CLEAR_NODE(&cfqg->rb_node);

	/*                                                 */
	cfqg->weight = 2*BLKIO_WEIGHT_DEFAULT;

#ifdef CONFIG_CFQ_GROUP_IOSCHED
	/*
                                                                     
                                                                      
                                                                     
                                                               
                          
  */
	cfqg->ref = 2;

	if (blkio_alloc_blkg_stats(&cfqg->blkg)) {
		kfree(cfqg);
		kfree(cfqd);
		return NULL;
	}

	rcu_read_lock();

	cfq_blkiocg_add_blkio_group(&blkio_root_cgroup, &cfqg->blkg,
					(void *)cfqd, 0);
	rcu_read_unlock();
	cfqd->nr_blkcg_linked_grps++;

	/*                              */
	hlist_add_head(&cfqg->cfqd_node, &cfqd->cfqg_list);
#endif
	/*
                                                                  
                                                                     
                               
  */
	for (i = 0; i < CFQ_PRIO_LISTS; i++)
		cfqd->prio_trees[i] = RB_ROOT;

	/*
                                                                     
                                                                  
                                
  */
	cfq_init_cfqq(cfqd, &cfqd->oom_cfqq, 1, 0);
	cfqd->oom_cfqq.ref++;
	cfq_link_cfqq_cfqg(&cfqd->oom_cfqq, &cfqd->root_group);

	cfqd->queue = q;

	init_timer(&cfqd->idle_slice_timer);
	cfqd->idle_slice_timer.function = cfq_idle_slice_timer;
	cfqd->idle_slice_timer.data = (unsigned long) cfqd;

	INIT_WORK(&cfqd->unplug_work, cfq_kick_queue);

	cfqd->cfq_quantum = cfq_quantum;
	cfqd->cfq_fifo_expire[0] = cfq_fifo_expire[0];
	cfqd->cfq_fifo_expire[1] = cfq_fifo_expire[1];
	cfqd->cfq_back_max = cfq_back_max;
	cfqd->cfq_back_penalty = cfq_back_penalty;
	cfqd->cfq_slice[0] = cfq_slice_async;
	cfqd->cfq_slice[1] = cfq_slice_sync;
	cfqd->cfq_target_latency = cfq_target_latency;
	cfqd->cfq_slice_async_rq = cfq_slice_async_rq;
	cfqd->cfq_slice_idle = cfq_slice_idle;
	cfqd->cfq_group_idle = cfq_group_idle;
	cfqd->cfq_latency = 1;
	cfqd->hw_tag = -1;
	/*
                                                                     
                                                               
  */
	cfqd->last_delayed_sync = jiffies - HZ;
	return cfqd;
}

/*
                        
 */
static ssize_t
cfq_var_show(unsigned int var, char *page)
{
	return sprintf(page, "%d\n", var);
}

static ssize_t
cfq_var_store(unsigned int *var, const char *page, size_t count)
{
	char *p = (char *) page;

	*var = simple_strtoul(p, &p, 10);
	return count;
}

#define SHOW_FUNCTION(__FUNC, __VAR, __CONV)				\
static ssize_t __FUNC(struct elevator_queue *e, char *page)		\
{									\
	struct cfq_data *cfqd = e->elevator_data;			\
	unsigned int __data = __VAR;					\
	if (__CONV)							\
		__data = jiffies_to_msecs(__data);			\
	return cfq_var_show(__data, (page));				\
}
SHOW_FUNCTION(cfq_quantum_show, cfqd->cfq_quantum, 0);
SHOW_FUNCTION(cfq_fifo_expire_sync_show, cfqd->cfq_fifo_expire[1], 1);
SHOW_FUNCTION(cfq_fifo_expire_async_show, cfqd->cfq_fifo_expire[0], 1);
SHOW_FUNCTION(cfq_back_seek_max_show, cfqd->cfq_back_max, 0);
SHOW_FUNCTION(cfq_back_seek_penalty_show, cfqd->cfq_back_penalty, 0);
SHOW_FUNCTION(cfq_slice_idle_show, cfqd->cfq_slice_idle, 1);
SHOW_FUNCTION(cfq_group_idle_show, cfqd->cfq_group_idle, 1);
SHOW_FUNCTION(cfq_slice_sync_show, cfqd->cfq_slice[1], 1);
SHOW_FUNCTION(cfq_slice_async_show, cfqd->cfq_slice[0], 1);
SHOW_FUNCTION(cfq_slice_async_rq_show, cfqd->cfq_slice_async_rq, 0);
SHOW_FUNCTION(cfq_low_latency_show, cfqd->cfq_latency, 0);
SHOW_FUNCTION(cfq_target_latency_show, cfqd->cfq_target_latency, 1);
#undef SHOW_FUNCTION

#define STORE_FUNCTION(__FUNC, __PTR, MIN, MAX, __CONV)			\
static ssize_t __FUNC(struct elevator_queue *e, const char *page, size_t count)	\
{									\
	struct cfq_data *cfqd = e->elevator_data;			\
	unsigned int __data;						\
	int ret = cfq_var_store(&__data, (page), count);		\
	if (__data < (MIN))						\
		__data = (MIN);						\
	else if (__data > (MAX))					\
		__data = (MAX);						\
	if (__CONV)							\
		*(__PTR) = msecs_to_jiffies(__data);			\
	else								\
		*(__PTR) = __data;					\
	return ret;							\
}
STORE_FUNCTION(cfq_quantum_store, &cfqd->cfq_quantum, 1, UINT_MAX, 0);
STORE_FUNCTION(cfq_fifo_expire_sync_store, &cfqd->cfq_fifo_expire[1], 1,
		UINT_MAX, 1);
STORE_FUNCTION(cfq_fifo_expire_async_store, &cfqd->cfq_fifo_expire[0], 1,
		UINT_MAX, 1);
STORE_FUNCTION(cfq_back_seek_max_store, &cfqd->cfq_back_max, 0, UINT_MAX, 0);
STORE_FUNCTION(cfq_back_seek_penalty_store, &cfqd->cfq_back_penalty, 1,
		UINT_MAX, 0);
STORE_FUNCTION(cfq_slice_idle_store, &cfqd->cfq_slice_idle, 0, UINT_MAX, 1);
STORE_FUNCTION(cfq_group_idle_store, &cfqd->cfq_group_idle, 0, UINT_MAX, 1);
STORE_FUNCTION(cfq_slice_sync_store, &cfqd->cfq_slice[1], 1, UINT_MAX, 1);
STORE_FUNCTION(cfq_slice_async_store, &cfqd->cfq_slice[0], 1, UINT_MAX, 1);
STORE_FUNCTION(cfq_slice_async_rq_store, &cfqd->cfq_slice_async_rq, 1,
		UINT_MAX, 0);
STORE_FUNCTION(cfq_low_latency_store, &cfqd->cfq_latency, 0, 1, 0);
STORE_FUNCTION(cfq_target_latency_store, &cfqd->cfq_target_latency, 1, UINT_MAX, 1);
#undef STORE_FUNCTION

#define CFQ_ATTR(name) \
	__ATTR(name, S_IRUGO|S_IWUSR, cfq_##name##_show, cfq_##name##_store)

static struct elv_fs_entry cfq_attrs[] = {
	CFQ_ATTR(quantum),
	CFQ_ATTR(fifo_expire_sync),
	CFQ_ATTR(fifo_expire_async),
	CFQ_ATTR(back_seek_max),
	CFQ_ATTR(back_seek_penalty),
	CFQ_ATTR(slice_sync),
	CFQ_ATTR(slice_async),
	CFQ_ATTR(slice_async_rq),
	CFQ_ATTR(slice_idle),
	CFQ_ATTR(group_idle),
	CFQ_ATTR(low_latency),
	CFQ_ATTR(target_latency),
	__ATTR_NULL
};

static struct elevator_type iosched_cfq = {
	.ops = {
		.elevator_merge_fn = 		cfq_merge,
		.elevator_merged_fn =		cfq_merged_request,
		.elevator_merge_req_fn =	cfq_merged_requests,
		.elevator_allow_merge_fn =	cfq_allow_merge,
		.elevator_bio_merged_fn =	cfq_bio_merged,
		.elevator_dispatch_fn =		cfq_dispatch_requests,
		.elevator_add_req_fn =		cfq_insert_request,
		.elevator_activate_req_fn =	cfq_activate_request,
		.elevator_deactivate_req_fn =	cfq_deactivate_request,
		.elevator_completed_req_fn =	cfq_completed_request,
		.elevator_former_req_fn =	elv_rb_former_request,
		.elevator_latter_req_fn =	elv_rb_latter_request,
		.elevator_init_icq_fn =		cfq_init_icq,
		.elevator_exit_icq_fn =		cfq_exit_icq,
		.elevator_set_req_fn =		cfq_set_request,
		.elevator_put_req_fn =		cfq_put_request,
		.elevator_may_queue_fn =	cfq_may_queue,
		.elevator_init_fn =		cfq_init_queue,
		.elevator_exit_fn =		cfq_exit_queue,
	},
	.icq_size	=	sizeof(struct cfq_io_cq),
	.icq_align	=	__alignof__(struct cfq_io_cq),
	.elevator_attrs =	cfq_attrs,
	.elevator_name	=	"cfq",
	.elevator_owner =	THIS_MODULE,
};

#ifdef CONFIG_CFQ_GROUP_IOSCHED
static struct blkio_policy_type blkio_policy_cfq = {
	.ops = {
		.blkio_unlink_group_fn =	cfq_unlink_blkio_group,
		.blkio_update_group_weight_fn =	cfq_update_blkio_group_weight,
	},
	.plid = BLKIO_POLICY_PROP,
};
#else
static struct blkio_policy_type blkio_policy_cfq;
#endif

static int __init cfq_init(void)
{
	int ret;

	/*
                                  
  */
	if (!cfq_slice_async)
		cfq_slice_async = 1;
	if (!cfq_slice_idle)
		cfq_slice_idle = 1;

#ifdef CONFIG_CFQ_GROUP_IOSCHED
	if (!cfq_group_idle)
		cfq_group_idle = 1;
#else
		cfq_group_idle = 0;
#endif
	cfq_pool = KMEM_CACHE(cfq_queue, 0);
	if (!cfq_pool)
		return -ENOMEM;

	ret = elv_register(&iosched_cfq);
	if (ret) {
		kmem_cache_destroy(cfq_pool);
		return ret;
	}

	blkio_policy_register(&blkio_policy_cfq);

	return 0;
}

static void __exit cfq_exit(void)
{
	blkio_policy_unregister(&blkio_policy_cfq);
	elv_unregister(&iosched_cfq);
	kmem_cache_destroy(cfq_pool);
}

module_init(cfq_init);
module_exit(cfq_exit);

MODULE_AUTHOR("Jens Axboe");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Completely Fair Queueing IO scheduler");