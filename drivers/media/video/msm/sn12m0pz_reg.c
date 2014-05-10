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

#include "sn12m0pz.h"
/*                         */

const struct reg_struct_init iu060f_reg_pat_init[1] = {{
	/*             */
	0x4B, /*           */
	/*                     */
	0x04, /*           */
	0x00, /*           */
	/*                */
	0x00, /*                  */
	0x80, /*           */
	0x08, /*           */
	0x37, /*           */
	0x60, /*           */
	0x01, /*           */
	0x28, /*           */
	0x00, /*           */
	0x60, /*           */
	0x24, /*           */
	0x34, /*           */
	0x3B, /*           */
	0xC0, /*           */
	0x30, /*           */
	0x00, /*           */
	0x88, /*           */
	0x1C, /*           */
	0x32, /*           */
	0x83, /*           */
	0x04, /*           */
	0xC0, /*           */
	0x50, /*           */
	0xA5, /*           */
	0xA9, /*           */
	0x0C, /*           */
	0x55, /*           */
	0xCC, /*           */
	0x83, /*           */
	0xC7, /*           */
	0x07, /*           */
	0x32, /*           */
	0x35, /*           */
	0x35, /*           */
	0x73, /*           */
	0x80, /*           */
	0xBE, /*           */
	0xBD, /*           */
	0x82, /*           */
	0xBC, /*           */
	0x82, /*           */
	0xBC, /*           */
	0x0C, /*           */
	0x2C, /*           */
	0x83, /*           */
	0xAF, /*           */
	0x83, /*           */
	0xAF, /*           */
	0x06, /*           */
	0xBA, /*           */
	0xBE, /*           */
	0xD8, /*           */
	0x17, /*           */
	0xCF, /*           */
	0xF1, /*           */
	0xD8, /*           */
	0x17, /*           */
	0x09, /*            */
	0xC5, /*            */
	0x84, /*            */
	0x6C, /*            */
	0x0B, /*            */
	0x09, /*            */
	0x05, /*           */
	0x04, /*           */
	0x15, /*           */
	0x03, /*           */
	0x13, /*           */
	0x05, /*           */
	0x0B, /*           */
	0x04, /*           */
	0x0B, /*           */
	0x06  /*           */
}
};

/*                                      */
const struct reg_struct iu060f_reg_pat[3] = {
	{ /*         */
		0x22, /*    */ /*    */

		/*                */
		0x01, /*                            */
		0xFF, /*                            */

		/*              */
		/*                         
                           */

		0x06, /*                                  */
		0x02, /*                                  */
		0x10, /*                                  */
		0x70, /*                                  */
		0x07, /*                                  */
		0xe0, /*                                  */
		0x05, /*                                  */
		0xe8, /*                                  */
		0x01, /*                                  */
		0x03, /*                                  */
		0x01, /*                                  */
		0x03, /*                                  */
		0x46, /*                                  */
		0x86, /*                                  */
		0x01, /*                                  */

		0x00,  /*        */
		0x00,  /*        */
		0x00,  /*        */
		0x00,  /*        */
		0x0F,  /*        */
		0xBF,  /*        */
		0x0B,  /*        */
		0xCF,  /*        */
	},
	{ /*          */
		0x14, /*                                   */
		/*                                */

		/*                */
		0x0B, /*                            */
		0xFF, /*                            */

		/*              */
		/*      */
		0x0C,/*                             */
		0x02,/*                             */
		0x10,/*                          */
		0x70,/*                           */
		0x0F,/*                           */
		0xC0, /*                          */
		0x0B, /*                          */
		0xD0, /*                          */
		0x01, /*                          */
		0x01, /*                          */
		0x01, /*                                          */
		0x01, /*                                          */
		0x06, /*                                          */
		0x06, /*                                          */
		0x00, /*                                          */

		0x00,  /*        */
		0x00,  /*        */
		0x00,  /*        */
		0x00,  /*        */
		0x0F,  /*        */
		0xBF,  /*        */
		0x0B,  /*        */
		0xCF,  /*        */
	},
	/*                  */
	{
		0x1B, /*        */
		/*                */
		0x00, /*                            */
		0xFE, /*                            */

		/*              */
		/*                         
          */

		0x01, /*                                  */
		0x01, /*                                  */
		0x10, /*                                  */
		0x70, /*                                  */
		0x0f, /*                                  */
		0xc0, /*                                  */
		0x00, /*                                  */
		0xF8, /*                                  */
		0x01, /*                                  */
		0x01, /*                                  */
		0x09, /*                                  */
		0x07, /*                                  */
		0x46, /*                                  */
		0x86, /*                                  */
		0x00, /*                                  */
		/*                        */
		0x00, /*       */
		0x00, /*       */
		0x02, /*       */
		0x10, /*       */
		0x0F, /*       */
		0xBF, /*       */
		0x09, /*       */
		0xCF, /*       */
	}
};
struct sn12m0pz_reg sn12m0pz_regs = {
	.reg_pat = &iu060f_reg_pat[0],
	.reg_pat_init = &iu060f_reg_pat_init[0],
};

