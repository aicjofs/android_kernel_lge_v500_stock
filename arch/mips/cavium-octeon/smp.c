/*
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 *
 * Copyright (C) 2004-2008, 2009, 2010 Cavium Networks
 */
#include <linux/cpu.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/smp.h>
#include <linux/interrupt.h>
#include <linux/kernel_stat.h>
#include <linux/sched.h>
#include <linux/module.h>

#include <asm/mmu_context.h>
#include <asm/time.h>
#include <asm/setup.h>

#include <asm/octeon/octeon.h>

#include "octeon_boot.h"

volatile unsigned long octeon_processor_boot = 0xff;
volatile unsigned long octeon_processor_sp;
volatile unsigned long octeon_processor_gp;

#ifdef CONFIG_HOTPLUG_CPU
uint64_t octeon_bootloader_entry_addr;
EXPORT_SYMBOL(octeon_bootloader_entry_addr);
#endif

static irqreturn_t mailbox_interrupt(int irq, void *dev_id)
{
	const int coreid = cvmx_get_core_num();
	uint64_t action;

	/*                                                                   */
	action = cvmx_read_csr(CVMX_CIU_MBOX_CLRX(coreid)) & 0xffff;

	/*                                          */
	cvmx_write_csr(CVMX_CIU_MBOX_CLRX(coreid), action);

	if (action & SMP_CALL_FUNCTION)
		smp_call_function_interrupt();
	if (action & SMP_RESCHEDULE_YOURSELF)
		scheduler_ipi();

	/*                                              */
	if (action & SMP_ICACHE_FLUSH)
		asm volatile ("synci 0($0)\n");
	return IRQ_HANDLED;
}

/* 
                                                                         
                                                                        
             
 */
void octeon_send_ipi_single(int cpu, unsigned int action)
{
	int coreid = cpu_logical_map(cpu);
	/*
                                                                 
                        
 */
	cvmx_write_csr(CVMX_CIU_MBOX_SETX(coreid), action);
}

static inline void octeon_send_ipi_mask(const struct cpumask *mask,
					unsigned int action)
{
	unsigned int i;

	for_each_cpu_mask(i, *mask)
		octeon_send_ipi_single(i, action);
}

/* 
                                                    
 */
static void octeon_smp_hotplug_setup(void)
{
#ifdef CONFIG_HOTPLUG_CPU
	struct linux_app_boot_info *labi;

	labi = (struct linux_app_boot_info *)PHYS_TO_XKSEG_CACHED(LABI_ADDR_IN_BOOTLOADER);
	if (labi->labi_signature != LABI_SIGNATURE)
		panic("The bootloader version on this board is incorrect.");

	octeon_bootloader_entry_addr = labi->InitTLBStart_addr;
#endif
}

static void octeon_smp_setup(void)
{
	const int coreid = cvmx_get_core_num();
	int cpus;
	int id;
	int core_mask = octeon_get_boot_coremask();
#ifdef CONFIG_HOTPLUG_CPU
	unsigned int num_cores = cvmx_octeon_num_cores();
#endif

	/*                                                           */
	for (id = 0; id < NR_CPUS; id++) {
		set_cpu_possible(id, id == 0);
		set_cpu_present(id, id == 0);
	}

	__cpu_number_map[coreid] = 0;
	__cpu_logical_map[0] = coreid;

	/*                                              */
	cpus = 1;
	for (id = 0; id < NR_CPUS; id++) {
		if ((id != coreid) && (core_mask & (1 << id))) {
			set_cpu_possible(cpus, true);
			set_cpu_present(cpus, true);
			__cpu_number_map[id] = cpus;
			__cpu_logical_map[cpus] = id;
			cpus++;
		}
	}

#ifdef CONFIG_HOTPLUG_CPU
	/*
                                                            
                                                              
                                            
  */
	for (id = 0; id < num_cores && id < NR_CPUS; id++) {
		if (!(core_mask & (1 << id))) {
			set_cpu_possible(cpus, true);
			__cpu_number_map[id] = cpus;
			__cpu_logical_map[cpus] = id;
			cpus++;
		}
	}
#endif

	octeon_smp_hotplug_setup();
}

/* 
                            
  
 */
static void octeon_boot_secondary(int cpu, struct task_struct *idle)
{
	int count;

	pr_info("SMP: Booting CPU%02d (CoreId %2d)...\n", cpu,
		cpu_logical_map(cpu));

	octeon_processor_sp = __KSTK_TOS(idle);
	octeon_processor_gp = (unsigned long)(task_thread_info(idle));
	octeon_processor_boot = cpu_logical_map(cpu);
	mb();

	count = 10000;
	while (octeon_processor_sp && count) {
		/*                                            */
		udelay(1);
		count--;
	}
	if (count == 0)
		pr_err("Secondary boot timeout\n");
}

/* 
                                                                      
                                          
 */
static void __cpuinit octeon_init_secondary(void)
{
	unsigned int sr;

	sr = set_c0_status(ST0_BEV);
	write_c0_ebase((u32)ebase);
	write_c0_status(sr);

	octeon_check_cpu_bist();
	octeon_init_cvmcount();

	octeon_irq_setup_secondary();
	raw_local_irq_enable();
}

/* 
                                      
  
 */
void octeon_prepare_cpus(unsigned int max_cpus)
{
#ifdef CONFIG_HOTPLUG_CPU
	struct linux_app_boot_info *labi;

	labi = (struct linux_app_boot_info *)PHYS_TO_XKSEG_CACHED(LABI_ADDR_IN_BOOTLOADER);

	if (labi->labi_signature != LABI_SIGNATURE)
		panic("The bootloader version on this board is incorrect.");
#endif
	/*
                                                            
                         
  */
	cvmx_write_csr(CVMX_CIU_MBOX_CLRX(cvmx_get_core_num()), 0xffff);
	if (request_irq(OCTEON_IRQ_MBOX0, mailbox_interrupt,
			IRQF_PERCPU | IRQF_NO_THREAD, "SMP-IPI",
			mailbox_interrupt)) {
		panic("Cannot request_irq(OCTEON_IRQ_MBOX0)");
	}
}

/* 
                                                                     
                       
 */
static void octeon_smp_finish(void)
{
#ifdef CONFIG_CAVIUM_GDB
	unsigned long tmp;
	/*                                                                     
                                                                      
            */
	asm volatile ("dmfc0 %0, $22\n"
		      "ori   %0, %0, 0x9100\n" "dmtc0 %0, $22\n" : "=r" (tmp));
#endif

	octeon_user_io_init();

	/*                                           */
	write_c0_compare(read_c0_count() + mips_hpt_frequency / HZ);
}

/* 
                                     
 */
static void octeon_cpus_done(void)
{
#ifdef CONFIG_CAVIUM_GDB
	unsigned long tmp;
	/*                                                                     
                                                                      
            */
	asm volatile ("dmfc0 %0, $22\n"
		      "ori   %0, %0, 0x9100\n" "dmtc0 %0, $22\n" : "=r" (tmp));
#endif
}

#ifdef CONFIG_HOTPLUG_CPU

/*                    */
DEFINE_PER_CPU(int, cpu_state);

extern void fixup_irqs(void);

static DEFINE_SPINLOCK(smp_reserve_lock);

static int octeon_cpu_disable(void)
{
	unsigned int cpu = smp_processor_id();

	if (cpu == 0)
		return -EBUSY;

	spin_lock(&smp_reserve_lock);

	set_cpu_online(cpu, false);
	cpu_clear(cpu, cpu_callin_map);
	local_irq_disable();
	fixup_irqs();
	local_irq_enable();

	flush_cache_all();
	local_flush_tlb_all();

	spin_unlock(&smp_reserve_lock);

	return 0;
}

static void octeon_cpu_die(unsigned int cpu)
{
	int coreid = cpu_logical_map(cpu);
	uint32_t mask, new_mask;
	const struct cvmx_bootmem_named_block_desc *block_desc;

	while (per_cpu(cpu_state, cpu) != CPU_DEAD)
		cpu_relax();

	/*
                                                                    
                                      
  */

	mask = 1 << coreid;
	/*                                                       */
	block_desc = cvmx_bootmem_find_named_block(LINUX_APP_BOOT_BLOCK_NAME);

	if (!block_desc) {
		struct linux_app_boot_info *labi;

		labi = (struct linux_app_boot_info *)PHYS_TO_XKSEG_CACHED(LABI_ADDR_IN_BOOTLOADER);

		labi->avail_coremask |= mask;
		new_mask = labi->avail_coremask;
	} else {		       /*                                  */
		uint32_t *p = (uint32_t *)PHYS_TO_XKSEG_CACHED(block_desc->base_addr +
							       AVAIL_COREMASK_OFFSET_IN_LINUX_APP_BOOT_BLOCK);
		*p |= mask;
		new_mask = *p;
	}

	pr_info("Reset core %d. Available Coremask = 0x%x \n", coreid, new_mask);
	mb();
	cvmx_write_csr(CVMX_CIU_PP_RST, 1 << coreid);
	cvmx_write_csr(CVMX_CIU_PP_RST, 0);
}

void play_dead(void)
{
	int cpu = cpu_number_map(cvmx_get_core_num());

	idle_task_exit();
	octeon_processor_boot = 0xff;
	per_cpu(cpu_state, cpu) = CPU_DEAD;

	mb();

	while (1)	/*                         */
		;
}

extern void kernel_entry(unsigned long arg1, ...);

static void start_after_reset(void)
{
	kernel_entry(0, 0, 0);  /*                               */
}

static int octeon_update_boot_vector(unsigned int cpu)
{

	int coreid = cpu_logical_map(cpu);
	uint32_t avail_coremask;
	const struct cvmx_bootmem_named_block_desc *block_desc;
	struct boot_init_vector *boot_vect =
		(struct boot_init_vector *)PHYS_TO_XKSEG_CACHED(BOOTLOADER_BOOT_VECTOR);

	block_desc = cvmx_bootmem_find_named_block(LINUX_APP_BOOT_BLOCK_NAME);

	if (!block_desc) {
		struct linux_app_boot_info *labi;

		labi = (struct linux_app_boot_info *)PHYS_TO_XKSEG_CACHED(LABI_ADDR_IN_BOOTLOADER);

		avail_coremask = labi->avail_coremask;
		labi->avail_coremask &= ~(1 << coreid);
	} else {		       /*                                  */
		avail_coremask = *(uint32_t *)PHYS_TO_XKSEG_CACHED(
			block_desc->base_addr + AVAIL_COREMASK_OFFSET_IN_LINUX_APP_BOOT_BLOCK);
	}

	if (!(avail_coremask & (1 << coreid))) {
		/*                                                              */
		cvmx_write_csr(CVMX_CIU_PP_RST, 1 << coreid);
		cvmx_write_csr(CVMX_CIU_PP_RST, 0);
	}

	boot_vect[coreid].app_start_func_addr =
		(uint32_t) (unsigned long) start_after_reset;
	boot_vect[coreid].code_addr = octeon_bootloader_entry_addr;

	mb();

	cvmx_write_csr(CVMX_CIU_NMI, (1 << coreid) & avail_coremask);

	return 0;
}

static int __cpuinit octeon_cpu_callback(struct notifier_block *nfb,
	unsigned long action, void *hcpu)
{
	unsigned int cpu = (unsigned long)hcpu;

	switch (action) {
	case CPU_UP_PREPARE:
		octeon_update_boot_vector(cpu);
		break;
	case CPU_ONLINE:
		pr_info("Cpu %d online\n", cpu);
		break;
	case CPU_DEAD:
		break;
	}

	return NOTIFY_OK;
}

static int __cpuinit register_cavium_notifier(void)
{
	hotcpu_notifier(octeon_cpu_callback, 0);
	return 0;
}
late_initcall(register_cavium_notifier);

#endif  /*                    */

struct plat_smp_ops octeon_smp_ops = {
	.send_ipi_single	= octeon_send_ipi_single,
	.send_ipi_mask		= octeon_send_ipi_mask,
	.init_secondary		= octeon_init_secondary,
	.smp_finish		= octeon_smp_finish,
	.cpus_done		= octeon_cpus_done,
	.boot_secondary		= octeon_boot_secondary,
	.smp_setup		= octeon_smp_setup,
	.prepare_cpus		= octeon_prepare_cpus,
#ifdef CONFIG_HOTPLUG_CPU
	.cpu_disable		= octeon_cpu_disable,
	.cpu_die		= octeon_cpu_die,
#endif
};
