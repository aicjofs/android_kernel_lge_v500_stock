/*
 *
 * Copyright (C) 2008-2009 ST-Ericsson AB
 * License terms: GNU General Public License (GPL) version 2
 *
 * Author: Rickard Andersson <rickard.andersson@stericsson.com>
 * Author: Linus Walleij <linus.walleij@stericsson.com>
 *
 */
#ifndef __ASMARM_TCM_H
#define __ASMARM_TCM_H

#ifndef CONFIG_HAVE_TCM
#error "You should not be including tcm.h unless you have a TCM!"
#endif

#include <linux/compiler.h>

/*                         */
#define __tcmdata __section(.tcm.data)
/*                         */
#define __tcmconst __section(.tcm.rodata)
/*                                                            */
#define __tcmfunc __attribute__((long_call)) __section(.tcm.text) noinline
/*                                                           */
#define __tcmlocalfunc __section(.tcm.text)

void *tcm_alloc(size_t len);
void tcm_free(void *addr, size_t len);
bool tcm_dtcm_present(void);
bool tcm_itcm_present(void);

#endif