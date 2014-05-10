
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
 *
 */
#ifndef SN12M0PZ_H
#define SN12M0PZ_H

#include <linux/types.h>
extern struct sn12m0pz_reg sn12m0pz_regs; /*                    */
struct reg_struct{
	uint8_t pll_multiplier_lsb;            /*       */
	uint8_t coarse_integ_time_msb;   /*       */
	uint8_t coarse_integ_time_lsb;   /*       */
	uint8_t frame_length_lines_msb;        /*       */
	uint8_t frame_length_lines_lsb;        /*       */
	uint8_t line_length_pck_msb;           /*       */
	uint8_t line_length_pck_lsb;           /*       */
	uint8_t x_output_size_msb;             /*       */
	uint8_t x_output_size_lsb;             /*       */
	uint8_t y_output_size_msb;             /*       */
	uint8_t y_output_size_lsb;             /*       */
	uint8_t x_even_inc_lsb;                /*       */
	uint8_t x_odd_inc_lsb;                 /*       */
	uint8_t y_even_inc_lsb;                /*       */
	uint8_t y_odd_inc_lsb;                 /*       */
	uint8_t reg_0x3016;                    /*                */
	uint8_t reg_0x30E8;                    /*               */
	uint8_t reg_0x3301;                    /*                 */
	/*                         */
	uint8_t reg_0x0344;
	uint8_t reg_0x0345;
	uint8_t reg_0x0346;
	uint8_t reg_0x0347;
	uint8_t reg_0x0348;
	uint8_t reg_0x0349;
	uint8_t reg_0x034A;
	uint8_t reg_0x034B;
};
struct reg_struct_init{
	uint8_t reg_0x302B;/*       */

	uint8_t reg_0x30E5;/*       */
	uint8_t reg_0x3300;   /*       */

	uint8_t image_orient;   /*       */

	uint8_t reg_0x300A;   /*       */
	uint8_t reg_0x3014;   /*       */
	uint8_t reg_0x3015;   /*       */
	uint8_t reg_0x3017;   /*       */
	uint8_t reg_0x301C;   /*       */
	uint8_t reg_0x3031;   /*       */
	uint8_t reg_0x3040;   /*       */
	uint8_t reg_0x3041;   /*       */
	uint8_t reg_0x3051;   /*       */
	uint8_t reg_0x3053;   /*       */
	uint8_t reg_0x3055;   /*       */
	uint8_t reg_0x3057;   /*       */
	uint8_t reg_0x3060;   /*       */
	uint8_t reg_0x3065;   /*       */
	uint8_t reg_0x30AA;   /*       */
	uint8_t reg_0x30AB;   /*       */
	uint8_t reg_0x30B0;   /*       */
	uint8_t reg_0x30B2;   /*       */

	uint8_t reg_0x30D3;   /*       */
	uint8_t reg_0x30D8;   /*       */

	uint8_t reg_0x3106;   /*       */
	uint8_t reg_0x3108;   /*       */
	uint8_t reg_0x310A;   /*       */
	uint8_t reg_0x310C;   /*       */
	uint8_t reg_0x310E;   /*       */
	uint8_t reg_0x3126;   /*       */
	uint8_t reg_0x312E;   /*       */
	uint8_t reg_0x313C;   /*       */
	uint8_t reg_0x313E;   /*       */
	uint8_t reg_0x3140;   /*       */
	uint8_t reg_0x3142;   /*       */
	uint8_t reg_0x3144;   /*       */
	uint8_t reg_0x3148;   /*       */
	uint8_t reg_0x314A;   /*       */
	uint8_t reg_0x3166;   /*       */
	uint8_t reg_0x3168;   /*       */
	uint8_t reg_0x316F;   /*       */
	uint8_t reg_0x3171;   /*       */
	uint8_t reg_0x3173;   /*       */
	uint8_t reg_0x3175;   /*       */
	uint8_t reg_0x3177;   /*       */
	uint8_t reg_0x3179;   /*       */
	uint8_t reg_0x317B;   /*       */
	uint8_t reg_0x317D;   /*       */
	uint8_t reg_0x317F;   /*       */
	uint8_t reg_0x3181;   /*       */
	uint8_t reg_0x3184;   /*       */
	uint8_t reg_0x3185;   /*       */
	uint8_t reg_0x3187;   /*       */

	uint8_t reg_0x31A4;   /*       */
	uint8_t reg_0x31A6;   /*       */
	uint8_t reg_0x31AC;   /*       */
	uint8_t reg_0x31AE;   /*       */
	uint8_t reg_0x31B4;   /*       */
	uint8_t reg_0x31B6;   /*       */

	uint8_t reg_0x3254;   /*       */
	uint8_t reg_0x3256;   /*       */
	uint8_t reg_0x3258;   /*       */
	uint8_t reg_0x325A;   /*       */
	uint8_t reg_0x3260;   /*       */
	uint8_t reg_0x3262;   /*       */

	uint8_t reg_0x3304;   /*       */
	uint8_t reg_0x3305;   /*       */
	uint8_t reg_0x3306;   /*       */
	uint8_t reg_0x3307;   /*       */
	uint8_t reg_0x3308;   /*       */
	uint8_t reg_0x3309;   /*       */
	uint8_t reg_0x330A;   /*       */
	uint8_t reg_0x330B;   /*       */
	uint8_t reg_0x330C;   /*       */
	uint8_t reg_0x330D;   /*       */

};
struct sn12m0pz_reg{
	const struct reg_struct  *reg_pat;
	const struct reg_struct_init  *reg_pat_init;
};
#endif
