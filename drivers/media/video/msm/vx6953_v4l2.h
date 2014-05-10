/* Copyright (c) 2011, The Linux Foundation. All rights reserved.
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

#ifndef VX6953_V4L2_H
#define VX6953_V4L2_H
#include <linux/types.h>
#include <mach/board.h>
extern struct vx6953_reg vx6953_regs;
struct reg_struct_init {
	uint8_t reg_0x0112;      /*       */
	uint8_t reg_0x0113;      /*       */
	uint8_t vt_pix_clk_div;  /*       */
	uint8_t pre_pll_clk_div; /*       */
	uint8_t pll_multiplier;  /*       */
	uint8_t op_pix_clk_div;  /*       */
	uint8_t reg_0x3030;      /*      */
	uint8_t reg_0x0111;      /*      */
	uint8_t reg_0x0b00;      /*      */
	uint8_t reg_0x3001;      /*      */
	uint8_t reg_0x3004;      /*      */
	uint8_t reg_0x3007;      /*      */
	uint8_t reg_0x3016;      /*      */
	uint8_t reg_0x301d;      /*      */
	uint8_t reg_0x317e;      /*      */
	uint8_t reg_0x317f;      /*      */
	uint8_t reg_0x3400;      /*      */
	uint8_t reg_0x0b06;      /*      */
	uint8_t reg_0x0b07;      /*      */
	uint8_t reg_0x0b08;      /*      */
	uint8_t reg_0x0b09;      /*      */
	uint8_t reg_0x0136;
	uint8_t reg_0x0137;
	/*      */
	uint8_t reg_0x0b83;      /*      */
	uint8_t reg_0x0b84;      /*      */
	uint8_t reg_0x0b85;      /*      */
	uint8_t reg_0x0b88;      /*      */
	uint8_t reg_0x0b89;      /*      */
	uint8_t reg_0x0b8a;      /*      */
	};
struct reg_struct {
	uint8_t coarse_integration_time_hi; /*                              */
	uint8_t coarse_integration_time_lo; /*                              */
	uint8_t analogue_gain_code_global;
	uint8_t frame_length_lines_hi; /*       */
	uint8_t frame_length_lines_lo; /*       */
	uint8_t line_length_pck_hi;    /*       */
	uint8_t line_length_pck_lo;    /*       */
	uint8_t reg_0x3005;   /*       */
	uint8_t reg_0x3010;  /*       */
	uint8_t reg_0x3011;  /*       */
	uint8_t reg_0x301a;  /*       */
	uint8_t reg_0x3035;  /*       */
	uint8_t reg_0x3036;   /*       */
	uint8_t reg_0x3041;  /*      */
	uint8_t reg_0x3042;  /*      */
	uint8_t reg_0x3045;  /*      */
	uint8_t reg_0x0b80;   /*       */
	uint8_t reg_0x0900;   /*      */
	uint8_t reg_0x0901;   /*       */
	uint8_t reg_0x0902;   /*      */
	uint8_t reg_0x0383;   /*      */
	uint8_t reg_0x0387;   /*       */
	uint8_t reg_0x034c;   /*       */
	uint8_t reg_0x034d;   /*      */
	uint8_t reg_0x034e;   /*       */
	uint8_t reg_0x034f;   /*       */
	uint8_t reg_0x1716; /*      */
	uint8_t reg_0x1717; /*      */
	uint8_t reg_0x1718; /*      */
	uint8_t reg_0x1719; /*      */
	uint8_t reg_0x3210;/*      */
	uint8_t reg_0x111; /*     */
	uint8_t reg_0x3410;  /*      */
	uint8_t reg_0x3098;
	uint8_t reg_0x309D;
	uint8_t reg_0x0200;
	uint8_t reg_0x0201;
	};
struct vx6953_i2c_reg_conf {
	unsigned short waddr;
	unsigned short wdata;
};

enum vx6953_test_mode_t {
	TEST_OFF,
	TEST_1,
	TEST_2,
	TEST_3
};

enum vx6953_resolution_t {
	QTR_SIZE,
	FULL_SIZE,
	INVALID_SIZE
};
enum vx6953_setting {
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

enum sensor_revision_t {
	VX6953_STM5M0EDOF_CUT_1,
	VX6953_STM5M0EDOF_CUT_2,
	VX6953_STM5M0EDOF_CUT_3
};
enum edof_mode_t {
	VX6953_EDOF_DISABLE,       /*      */
	VX6953_EDOF_APPLICATION,   /*      */
	VX6953_EDOF_ESTIMATION     /*      */
};
struct vx6953_reg {
	const struct reg_struct_init  *reg_pat_init;
	const struct reg_struct  *reg_pat;
};
#endif /*          */
