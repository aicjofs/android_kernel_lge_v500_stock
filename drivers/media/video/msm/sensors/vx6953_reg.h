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


struct reg_struct_init vx6953_reg_init[1] = {
	{
		10,			/*                      */
		10,			/*            */
		9,			/*                           */
		4,		/*                            */
		133,		/*                           */
		10,		/*                           */
		0x08,		/*            */
		0x02,		/*            */
		0x01,		/*                               */
		0x30,		/*            */
		0x33,		/*            */
		0x09,		/*            */
		0x1F,		/*            */
		0x03,		/*            */
		0x11,		/*            */
		0x09,		/*            */
		0x38,		/*            */
		0x00,		/*          */
		0x80,		/*          */
		0x01,		/*          */
		0x4F,		/*          */
		0x18,		/*          */
		0x00,		/*           */
		0x20,		/*            */
		0x90,		/*            */
		0x20,		/*            */
		0x80,		/*            */
		0x00,		/*            */
		0x00,		/*            */
	}
};
struct reg_struct vx6953_reg_pat[2] = {
	{/*         */
		0x03,	/*                                       */
		0xd0,	/*                                       */
		0xc0,	/*                                      */
		0x03,	/*                                  */
		0xf0,	/*                                  */
		0x0b,	/*                                */
		0x74,	/*                                */
		0x03,	/*            */
		0x00,	/*            */
		0x01,	/*            */
		0x6a,	/*            */
		0x03,	/*            */
		0x2c,	/*            */
		0x00,	/*            */
		0x24,	/*            */
		0x81,	/*            */
		0x02,	/*                          */
		0x01,	/*            */
		0x22,	/*            */
		0x04,	/*            */
		0x03,	/*            */
		0x03,	/*            */
		0x05,	/*            */
		0x18,	/*            */
		0x03,	/*            */
		0xd4,	/*            */
		0x02,	/*      */
		0x04,	/*      */
		0x08,	/*      */
		0x2c,	/*      */
		0x01,   /*      */
		0x02,   /*     */
		0x01,   /*      */
		0x01,   /*      */
		0x05,   /*      */
		0x02,
		0x04,
	},
	{ /*          */
		0x07,/*                                       */
		0x00,/*                                       */
		0xc0,/*                                      */
		0x07,/*                                  */
		0xd0,/*                                  */
		0x0b,/*                               */
		0x8c,/*                               */
		0x01,/*            */
		0x00,/*            */
		0x00,/*            */
		0x55,/*            */
		0x01,/*            */
		0x23,/*            */
		0x00,/*            */
		0x24,/*            */
		0xb7,/*            */
		0x01,/*                             */
		0x00,/*            */
		0x00,/*            */
		0x00,/*            */
		0x01,/*            */
		0x01,/*            */
		0x0A,/*            */
		0x30,/*            */
		0x07,/*            */
		0xA8,/*            */
		0x02,/*      */
		0x0d,/*      */
		0x07,/*      */
		0x7d,/*      */
		0x01,/*      */
		0x02,/*     */
		0x01,/*      */
		0x01,/*      */
		0x05, /*      */
		0x02,
		0x00,
	}
};



struct vx6953_reg vx6953_regs = {
	.reg_pat_init = &vx6953_reg_init[0],
	.reg_pat = &vx6953_reg_pat[0],
};
