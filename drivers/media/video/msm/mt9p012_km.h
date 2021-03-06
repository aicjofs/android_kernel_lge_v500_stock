/* Copyright (c) 2009-2012, The Linux Foundation. All rights reserved.
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

#ifndef MT9P012_KM_H
#define MT9P012_KM_H

#include <linux/types.h>
#include <mach/board.h>

extern struct mt9p012_km_reg mt9p012_km_regs;	/*                       */

struct reg_struct {
	uint16_t vt_pix_clk_div;     /*        */
	uint16_t vt_sys_clk_div;     /*        */
	uint16_t pre_pll_clk_div;    /*        */
	uint16_t pll_multiplier;     /*        */
	uint16_t op_pix_clk_div;     /*        */
	uint16_t op_sys_clk_div;     /*        */
	uint16_t scale_m;            /*        */
	uint16_t row_speed;          /*        */
	uint16_t x_addr_start;       /*        */
	uint16_t x_addr_end;         /*        */
	uint16_t y_addr_start;       /*        */
	uint16_t y_addr_end;         /*        */
	uint16_t read_mode;          /*        */
	uint16_t x_output_size ;     /*        */
	uint16_t y_output_size;      /*        */
	uint16_t line_length_pck;    /*        */
	uint16_t frame_length_lines; /*        */
	uint16_t coarse_int_time;    /*        */
	uint16_t fine_int_time;      /*        */
};


struct mt9p012_km_i2c_reg_conf {
	unsigned short waddr;
	unsigned short wdata;
};


struct mt9p012_km_reg {
	struct reg_struct const *reg_pat;
	uint16_t reg_pat_size;
	struct mt9p012_km_i2c_reg_conf const *ttbl;
	uint16_t ttbl_size;
	struct mt9p012_km_i2c_reg_conf const *lctbl;
	uint16_t lctbl_size;
	struct mt9p012_km_i2c_reg_conf const *rftbl;
	uint16_t rftbl_size;
};

#endif /*              */
