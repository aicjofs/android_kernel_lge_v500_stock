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

#ifndef MSM_JPEG_CORE_H
#define MSM_JPEG_CORE_H

#include <linux/interrupt.h>
#include "msm_jpeg_hw.h"

#define msm_jpeg_core_buf msm_jpeg_hw_buf

irqreturn_t msm_jpeg_core_irq(int irq_num, void *context);

void msm_jpeg_core_irq_install(int (*irq_handler) (int, void *, void *));
void msm_jpeg_core_irq_remove(void);

int msm_jpeg_core_fe_buf_update(struct msm_jpeg_core_buf *buf);
int msm_jpeg_core_we_buf_update(struct msm_jpeg_core_buf *buf);
int msm_jpeg_core_we_buf_reset(struct msm_jpeg_hw_buf *buf);

int msm_jpeg_core_reset(uint8_t op_mode, void *base, int size);
int msm_jpeg_core_fe_start(void);

void msm_jpeg_core_release(int, int);
void msm_jpeg_core_init(void);
#endif /*                 */
