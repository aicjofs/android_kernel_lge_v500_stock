/*
 *  linux/arch/cris/kernel/ptrace.c
 *
 * Parts taken from the m68k port.
 *
 * Copyright (c) 2000, 2001, 2002 Axis Communications AB
 *
 * Authors:   Bjorn Wesen
 *
 */

#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/mm.h>
#include <linux/smp.h>
#include <linux/errno.h>
#include <linux/ptrace.h>
#include <linux/user.h>
#include <linux/tracehook.h>

#include <asm/uaccess.h>
#include <asm/page.h>
#include <asm/pgtable.h>
#include <asm/processor.h>


/*                                               
                                             
 */
extern int do_signal(int canrestart, struct pt_regs *regs);


void do_notify_resume(int canrestart, struct pt_regs *regs,
		      __u32 thread_info_flags)
{
	/*                                   */
	if (thread_info_flags & _TIF_SIGPENDING)
		do_signal(canrestart,regs);

	if (thread_info_flags & _TIF_NOTIFY_RESUME) {
		clear_thread_flag(TIF_NOTIFY_RESUME);
		tracehook_notify_resume(regs);
		if (current->replacement_session_keyring)
			key_replace_session_keyring();
	}
}
