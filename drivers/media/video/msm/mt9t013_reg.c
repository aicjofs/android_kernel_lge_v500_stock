/* Copyright (c) 2009, The Linux Foundation. All rights reserved.
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

#include "mt9t013.h"
#include <linux/kernel.h>

struct reg_struct const mt9t013_reg_pat[2] = {
	{ /*                                                 */
	/*                                                  
                  */
	8,

	/*                                                    
                  */
	1,

	/*                                                    
                  */
	2,

	/*                                                    
                     
                                                        */
	46,

	/*                                  */
	8,

	/*                                  */
	1,

	/*                          */
	16,

	/*                          */
	0x0111,

	/*                          */
	8,

	/*                          */
	2053,

	/*                          */
	8,

	/*                          */
	1541,

	/*                          */
	0x046C,

	/*                          */
	1024,

	/*                          */
	768,

	/*                               */
	2616,

	/*                               */
	916,

	/*                            */
	16,

	/*                            */
	1461
	},
	{ /*         */
	/*                                                   
                  */
	8,

	/*                                                   
                  */
	1,

	/*                                                   
                   */
	2,

	/*                                                 
                         
                                                         */
	46,

	/*                                  */
	8,

	/*                                  */
	1,

	/*                          */
	16,

	/*                          */
	0x0111,

	/*                          */
	8,

	/*                          */
	2071,

	/*                          */
	8,

	/*                          */
	1551,

	/*                          */
	0x0024,

	/*                          */
	2064,

	/*                          */
	1544,

	/*                            */
	2952,

	/*                                  */
	1629,

	/*                            */
	16,

	/*                            */
	733
	}
};

struct mt9t013_i2c_reg_conf const mt9t013_test_tbl[] = {
	{ 0x3044, 0x0544 & 0xFBFF },
	{ 0x30CA, 0x0004 | 0x0001 },
	{ 0x30D4, 0x9020 & 0x7FFF },
	{ 0x31E0, 0x0003 & 0xFFFE },
	{ 0x3180, 0x91FF & 0x7FFF },
	{ 0x301A, (0x10CC | 0x8000) & 0xFFF7 },
	{ 0x301E, 0x0000 },
	{ 0x3780, 0x0000 },
};

/*                                */
struct mt9t013_i2c_reg_conf const mt9t013_lc_tbl[] = {
	{ 0x360A, 0x0290 }, /*           */
	{ 0x360C, 0xC92D }, /*           */
	{ 0x360E, 0x0771 }, /*           */
	{ 0x3610, 0xE38C }, /*           */
	{ 0x3612, 0xD74F }, /*           */
	{ 0x364A, 0x168C }, /*           */
	{ 0x364C, 0xCACB }, /*           */
	{ 0x364E, 0x8C4C }, /*           */
	{ 0x3650, 0x0BEA }, /*           */
	{ 0x3652, 0xDC0F }, /*           */
	{ 0x368A, 0x70B0 }, /*           */
	{ 0x368C, 0x200B }, /*           */
	{ 0x368E, 0x30B2 }, /*           */
	{ 0x3690, 0xD04F }, /*           */
	{ 0x3692, 0xACF5 }, /*           */
	{ 0x36CA, 0xF7C9 }, /*           */
	{ 0x36CC, 0x2AED }, /*           */
	{ 0x36CE, 0xA652 }, /*           */
	{ 0x36D0, 0x8192 }, /*           */
	{ 0x36D2, 0x3A15 }, /*           */
	{ 0x370A, 0xDA30 }, /*           */
	{ 0x370C, 0x2E2F }, /*           */
	{ 0x370E, 0xBB56 }, /*           */
	{ 0x3710, 0x8195 }, /*           */
	{ 0x3712, 0x02F9 }, /*           */
	{ 0x3600, 0x0230 }, /*           */
	{ 0x3602, 0x58AD }, /*           */
	{ 0x3604, 0x18D1 }, /*           */
	{ 0x3606, 0x260D }, /*           */
	{ 0x3608, 0xF530 }, /*           */
	{ 0x3640, 0x17EB }, /*           */
	{ 0x3642, 0x3CAB }, /*           */
	{ 0x3644, 0x87CE }, /*           */
	{ 0x3646, 0xC02E }, /*           */
	{ 0x3648, 0xF48F }, /*           */
	{ 0x3680, 0x5350 }, /*           */
	{ 0x3682, 0x7EAF }, /*           */
	{ 0x3684, 0x4312 }, /*           */
	{ 0x3686, 0xC652 }, /*           */
	{ 0x3688, 0xBC15 }, /*           */
	{ 0x36C0, 0xB8AD }, /*           */
	{ 0x36C2, 0xBDCD }, /*           */
	{ 0x36C4, 0xE4B2 }, /*           */
	{ 0x36C6, 0xB50F }, /*           */
	{ 0x36C8, 0x5B95 }, /*           */
	{ 0x3700, 0xFC90 }, /*           */
	{ 0x3702, 0x8C51 }, /*           */
	{ 0x3704, 0xCED6 }, /*           */
	{ 0x3706, 0xB594 }, /*           */
	{ 0x3708, 0x0A39 }, /*           */
	{ 0x3614, 0x0230 }, /*           */
	{ 0x3616, 0x160D }, /*           */
	{ 0x3618, 0x08D1 }, /*           */
	{ 0x361A, 0x98AB }, /*           */
	{ 0x361C, 0xEA50 }, /*           */
	{ 0x3654, 0xB4EA }, /*           */
	{ 0x3656, 0xEA6C }, /*           */
	{ 0x3658, 0xFE08 }, /*           */
	{ 0x365A, 0x2C6E }, /*           */
	{ 0x365C, 0xEB0E }, /*           */
	{ 0x3694, 0x6DF0 }, /*           */
	{ 0x3696, 0x3ACF }, /*           */
	{ 0x3698, 0x3E0F }, /*           */
	{ 0x369A, 0xB2B1 }, /*           */
	{ 0x369C, 0xC374 }, /*           */
	{ 0x36D4, 0xF2AA }, /*           */
	{ 0x36D6, 0x8CCC }, /*           */
	{ 0x36D8, 0xDEF2 }, /*           */
	{ 0x36DA, 0xFA11 }, /*           */
	{ 0x36DC, 0x42F5 }, /*           */
	{ 0x3714, 0xF4F1 }, /*           */
	{ 0x3716, 0xF6F0 }, /*           */
	{ 0x3718, 0x8FD6 }, /*           */
	{ 0x371A, 0xEA14 }, /*           */
	{ 0x371C, 0x6338 }, /*           */
	{ 0x361E, 0x0350 }, /*           */
	{ 0x3620, 0x91AE }, /*           */
	{ 0x3622, 0x0571 }, /*           */
	{ 0x3624, 0x100D }, /*           */
	{ 0x3626, 0xCA70 }, /*           */
	{ 0x365E, 0xE6CB }, /*           */
	{ 0x3660, 0x50ED }, /*           */
	{ 0x3662, 0x3DAE }, /*           */
	{ 0x3664, 0xAA4F }, /*           */
	{ 0x3666, 0xDC50 }, /*           */
	{ 0x369E, 0x5470 }, /*           */
	{ 0x36A0, 0x1F6E }, /*           */
	{ 0x36A2, 0x6671 }, /*           */
	{ 0x36A4, 0xC010 }, /*           */
	{ 0x36A6, 0x8DF5 }, /*           */
	{ 0x36DE, 0x0B0C }, /*           */
	{ 0x36E0, 0x84CE }, /*           */
	{ 0x36E2, 0x8493 }, /*           */
	{ 0x36E4, 0xA610 }, /*           */
	{ 0x36E6, 0x50B5 }, /*           */
	{ 0x371E, 0x9651 }, /*           */
	{ 0x3720, 0x1EAB }, /*           */
	{ 0x3722, 0xAF76 }, /*           */
	{ 0x3724, 0xE4F4 }, /*           */
	{ 0x3726, 0x79F8 }, /*           */
	{ 0x3782, 0x0410 }, /*               */
	{ 0x3784, 0x0320 }, /*                */
	{ 0x3780, 0x8000 } /*                */
};

struct mt9t013_reg mt9t013_regs = {
	.reg_pat = &mt9t013_reg_pat[0],
	.reg_pat_size = ARRAY_SIZE(mt9t013_reg_pat),
	.ttbl = &mt9t013_test_tbl[0],
	.ttbl_size = ARRAY_SIZE(mt9t013_test_tbl),
	.lctbl = &mt9t013_lc_tbl[0],
	.lctbl_size = ARRAY_SIZE(mt9t013_lc_tbl),
	.rftbl = &mt9t013_lc_tbl[0],	/*                          */
	.rftbl_size = ARRAY_SIZE(mt9t013_lc_tbl)
};


