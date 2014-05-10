/* Copyright (c) 2010, The Linux Foundation. All rights reserved.
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

#ifndef IMX074_H
#define IMX074_H
#include <linux/types.h>
#include <mach/board.h>
extern struct imx074_reg imx074_regs;
struct reg_struct_init {
    /*             */
	uint8_t pre_pll_clk_div; /*        */
	uint8_t plstatim; /*        */
	uint8_t reg_3024; /*      */
	uint8_t image_orientation;  /*       */
	uint8_t vndmy_ablmgshlmt; /*      */
	uint8_t y_opbaddr_start_di; /*      */
	uint8_t reg_0x3015; /*      */
	uint8_t reg_0x301c; /*      */
	uint8_t reg_0x302c; /*      */
	uint8_t reg_0x3031; /*      */
	uint8_t reg_0x3041; /*        */
	uint8_t reg_0x3051; /*        */
	uint8_t reg_0x3053; /*        */
	uint8_t reg_0x3057; /*        */
	uint8_t reg_0x305c; /*        */
	uint8_t reg_0x305d; /*        */
	uint8_t reg_0x3060; /*        */
	uint8_t reg_0x3065; /*        */
	uint8_t reg_0x30aa; /*        */
	uint8_t reg_0x30ab;
	uint8_t reg_0x30b0;
	uint8_t reg_0x30b2;
	uint8_t reg_0x30d3;
	uint8_t reg_0x3106;
	uint8_t reg_0x310c;
	uint8_t reg_0x3304;
	uint8_t reg_0x3305;
	uint8_t reg_0x3306;
	uint8_t reg_0x3307;
	uint8_t reg_0x3308;
	uint8_t reg_0x3309;
	uint8_t reg_0x330a;
	uint8_t reg_0x330b;
	uint8_t reg_0x330c;
	uint8_t reg_0x330d;
	uint8_t reg_0x330f;
	uint8_t reg_0x3381;
};

struct reg_struct {
	uint8_t pll_multiplier; /*        */
	uint8_t frame_length_lines_hi; /*       */
	uint8_t frame_length_lines_lo; /*       */
	uint8_t y_addr_start;  /*       */
	uint8_t y_add_end;  /*        */
	uint8_t x_output_size_msb;  /*        */
	uint8_t x_output_size_lsb;  /*        */
	uint8_t y_output_size_msb; /*        */
	uint8_t y_output_size_lsb; /*        */
	uint8_t x_even_inc;  /*        */
	uint8_t x_odd_inc; /*        */
	uint8_t y_even_inc;  /*        */
	uint8_t y_odd_inc; /*        */
	uint8_t hmodeadd;   /*        */
	uint8_t vmodeadd;   /*        */
	uint8_t vapplinepos_start;/*      */
	uint8_t vapplinepos_end;/*    */
	uint8_t shutter;	/*        */
	uint8_t haddave;	/*        */
	uint8_t lanesel;    /*        */
};

struct imx074_i2c_reg_conf {
	unsigned short waddr;
	unsigned short wdata;
};

enum imx074_test_mode_t {
	TEST_OFF,
	TEST_1,
	TEST_2,
	TEST_3
};

enum imx074_resolution_t {
	QTR_SIZE,
	FULL_SIZE,
	INVALID_SIZE
};
enum imx074_setting {
	RES_PREVIEW,
	RES_CAPTURE
};
enum mt9p012_reg_update {
	/*                                                               */
	REG_INIT,
	/*                                                */
	UPDATE_PERIODIC,
	/*                                          */
	UPDATE_ALL,
	/*                  */
	UPDATE_INVALID
};

struct imx074_reg {
	const struct reg_struct_init  *reg_pat_init;
	const struct reg_struct  *reg_pat;
};
#endif /*          */
