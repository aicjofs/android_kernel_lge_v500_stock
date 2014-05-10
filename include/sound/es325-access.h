/*
 * es325.h  --  ES325 Soc Audio access values
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef _ES325_ACCESS_H
#define _ES325_ACCESS_H

#define ES325_CMD_ACCESS_WR_MAX 4
#define ES325_CMD_ACCESS_RD_MAX 4

struct es325_cmd_access {
	u8 read_msg[ES325_CMD_ACCESS_RD_MAX];
	unsigned int read_msg_len;
	u8 write_msg[ES325_CMD_ACCESS_WR_MAX];
	unsigned int write_msg_len;
	unsigned int val_shift;
	unsigned int val_max;
};

static unsigned short es325_algo_paramid[] = {
	0x0002, /*            */
	0x0003, /*          */
	0x0004, /*        */
	0x0005, /*                     */
	0x0006, /*                    */
	0x0007, /*                        */
	0x0009, /*            */
	0x000D, /*                          */
	0x000E, /*                   */
	0x0010, /*        */
	0x0011, /*             */
	0x0012, /*                    */
	0x0015, /*          */
	0x0016, /*               */
	0x001A, /*                  */
	0x001B, /*                        */
	0x001C, /*                 */
	0x001F, /*            */
	0x0020, /*            */
	0x0023, /*         */
	0x0025, /*                                 */
	0x0026, /*                     */
	0x0027, /*                       */
	0x0028, /*        */
	0x0029, /*                     */
	0x002A, /*                    */
	0x002B, /*                        */
	0x002C, /*                     */
	0x002D, /*                       */
	0x002E, /*              */
	0x0030, /*        */
	0x0031, /*        */
	0x0034, /*         */
	0x0039, /*                        */
	0x003A, /*                 */
	0x003D, /*              */
	0x003E, /*                  */
	0x003F, /*                  */
	0x0040, /*                          */
	0x0042, /*                         */
	0x0043, /*                        */
	0x0047, /*                             */
	0x004B, /*                         */
	0x004C, /*                         */
	0x004D, /*        */
	0x004E, /*             */
	0x0100, /*                 */
	0x0102, /*                 */
	0x1001, /*                       */
	0x1002, /*                       */
	0x1003, /*                     */
	0x1004, /*                  */
	0x1005, /*                  */
	0x1006, /*                     */
	0x1007, /*                            */
	0x1008, /*              */
	0x1009, /*                     */
	0x100A, /*                        */
	0x100B, /*                */
	0x100C, /*                         */
	0x100D, /*                           */
	0x100E, /*                     */
	0x100F, /*                                  */
	0x1010, /*            */
	0x1011, /*                     */
	0x1012, /*                */
	0x1013, /*                              */
	0x1014, /*                              */
	0x1015, /*                              */
	0x1016, /*                              */
	0x1017, /*                              */
	0x1018, /*                              */
	0x1019, /*                              */
	0x101A, /*                              */
	0x101B, /*                              */
	0x101C, /*                               */
	0x101D, /*              */
	0x101E, /*                       */
	0x101F, /*                */
	0x1020, /*           */
	0x2000, /*       */
	0x2001, /*                            */
	0x2002, /*            */
	0x2004, /*                           */
	0x2006, /*                     */
	0x2007, /*                           */
	0x2008, /*                     */
	0x2009, /*                                  */
	0x200A, /*                        */
	0x200B, /*                                  */
	0x200C, /*                                */
	0x200D, /*                                */
	0x200E, /*                             */
	0x200F, /*                            */
	0x2010, /*                              */
	0x2011, /*                              */
	0x2013, /*                             */
	0x2014, /*                         */
	0x2015, /*                        */
	0x2016, /*                     */
	0x2017, /*                               */
	0x2018, /*                             */
	0x2019, /*                             */
	0x201A, /*                          */
	0x201B, /*                         */
	0x201C, /*                           */
	0x201D, /*                           */
	0x201F, /*                  */
	0x2020, /*                          */
	0x2021, /*                      */
	0x3001, /*                           */
	0x3002, /*                           */
	0x3003, /*                            */
	0x3005, /*                            */
	0x3006, /*                        */
	0x3007, /*                           */
	0x3008, /*                           */
	0x3009, /*                            */
	0x300A, /*                          */
	0x0053, /*                 */
	0x0054, /*               */
	0x004f, /*            */
	0x0050, /*                    */
	0x0051, /*             */
	0x0052, /*                    */
	0x001e, /*                   */
	0x0037, /*               */
};

static unsigned short es325_dev_paramid[] = {
	0x0A00, /*                */
	0x0A01, /*                           */
	0x0A02, /*                        */
	0x0A03, /*                        */
	0x0A04, /*                  */
	0x0A05, /*              */
	0x0A06, /*                */
	0x0A07, /*                       */
	0x0A08, /*                   */
	0x0A09, /*                     */
	0x0A0A, /*                          */
	0x0A0B, /*                   */
	0x0B00, /*                */
	0x0B01, /*                           */
	0x0B02, /*                        */
	0x0B03, /*                        */
	0x0B04, /*                  */
	0x0B05, /*              */
	0x0B06, /*                */
	0x0B07, /*                       */
	0x0B08, /*                   */
	0x0B09, /*                     */
	0x0B0A, /*                          */
	0x0B0B, /*                   */
	0x0C00, /*                */
	0x0C01, /*                           */
	0x0C02, /*                        */
	0x0C03, /*                        */
	0x0C04, /*                  */
	0x0C05, /*              */
	0x0C06, /*                */
	0x0C07, /*                       */
	0x0C08, /*                   */
	0x0C09, /*                     */
	0x0C0A, /*                          */
	0x0C0B, /*                   */
	0x0D00, /*                */
	0x0D01, /*                           */
	0x0D02, /*                        */
	0x0D03, /*                        */
	0x0D04, /*                  */
	0x0D05, /*              */
	0x0D06, /*                */
	0x0D07, /*                       */
	0x0D08, /*                   */
	0x0D09, /*                     */
	0x0D0A, /*                          */
	0x0D0B, /*                   */
	0x0900, /*                            */
};

static struct es325_cmd_access es325_cmd_access[] = {
	{ /*             */
		.read_msg = { 0x80, 0x10, 0x00, 0x00 },
		.read_msg_len = 4,
		.write_msg = { 0x80, 0x10, 0x00, 0x00 },
		.write_msg_len = 4,
		.val_shift = 0,
		.val_max = 1,
	},
	{ /*           */
		.read_msg = { 0x80, 0x25, 0x00, 0x00 },
		.read_msg_len = 4,
		.write_msg = { 0x80, 0x25, 0x00, 0x00 },
		.write_msg_len = 4,
		.val_shift = 0,
		.val_max = 5,
	},
	{ /*              */
		.read_msg = { 0x80, 0x28, 0x00, 0x00 },
		.read_msg_len = 4,
		.write_msg = { 0x80, 0x28, 0x00, 0x00 },
		.write_msg_len = 4,
		.val_shift = 0,
		.val_max = 8,
	},
	{ /*        */
		.read_msg = { 0x80, 0x31, 0x00, 0x00 },
		.read_msg_len = 4,
		.write_msg = { 0x80, 0x31, 0x00, 0x00 },
		.write_msg_len = 4,
		.val_shift = 0,
		.val_max = 15,
	},
	{ /*            */
		.read_msg = { 0x80, 0x42, 0x00, 0x00 },
		.read_msg_len = 4,
		.write_msg = { 0x80, 0x42, 0x00, 0x00 },
		.write_msg_len = 4,
		.val_shift = 0,
		.val_max = 15,
	},
	{ /*                 */
		.read_msg = { 0x80, 0x43, 0x00, 0x00 },
		.read_msg_len = 4,
		.write_msg = { 0x80, 0x1c, 0x00, 0x00 },
		.write_msg_len = 4,
		.val_shift = 0,
		.val_max = 1,
	},
	{ /*                  */
		.read_msg = { 0x80, 0x4b, 0x00, 0x00 },
		.read_msg_len = 4,
		.write_msg = { 0x80, 0x4c, 0x00, 0x00 },
		.write_msg_len = 4,
		.val_shift = 0,
		.val_max = 5,
	},
	{ /*             */
		.read_msg = { 0x80, 0x4d, 0x00, 0x00 },
		.read_msg_len = 4,
		.write_msg = { 0x80, 0x4e, 0x00, 0x00 },
		.write_msg_len = 4,
		.val_shift = 0,
		.val_max = 65535,
	},
	{ /*               */
		.read_msg = { 0x80, 0x4f, 0x00, 0x00 },
		.read_msg_len = 4,
		.write_msg = { 0x80, 0x4f, 0x00, 0x00 },
		.write_msg_len = 4,
		.val_shift = 0,
		.val_max = 4,
	},
	{ /*                      */
		.read_msg = { 0x80, 0x52, 0x00, 0x00 },
		.read_msg_len = 4,
		.write_msg = { 0x80, 0x52, 0x00, 0x00 },
		.write_msg_len = 4,
		.val_shift = 0,
		.val_max = 65535,
	},
	{ /*           */
		.read_msg = { 0x80, 0x5b, 0x00, 0x00 },
		.read_msg_len = 4,
		.write_msg = { 0x80, 0x5a, 0x00, 0x00 },
		.write_msg_len = 4,
		.val_shift = 0,
		.val_max = 65535,
	},
	{ /*           */
		.read_msg = { 0x80, 0x5d, 0x00, 0x00 },
		.read_msg_len = 4,
		.write_msg = { 0x80, 0x5c, 0x00, 0x00 },
		.write_msg_len = 4,
		.val_shift = 0,
		.val_max = 4,
	},
	{ /*                 */
		.read_msg = { 0x80, 0x65, 0x00, 0x00 },
		.read_msg_len = 4,
		.write_msg = { 0x80, 0x5e, 0x00, 0x00 },
		.write_msg_len = 4,
		.val_shift = 0,
		.val_max = 5,
	},
	{ /*                           */
		.read_msg = { 0x80, 0x13, 0x00, 0x00 },
		.read_msg_len = 4,
		.write_msg = { 0x80, 0x13, 0x00, 0x00 },
		.write_msg_len = 4,
		.val_shift = 0,
		.val_max = 65535,
	},
	{ /*                            */
		.read_msg = { 0x80, 0x13, 0x00, 0x01 },
		.read_msg_len = 4,
		.write_msg = { 0x80, 0x13, 0x00, 0x01 },
		.write_msg_len = 4,
		.val_shift = 0,
		.val_max = 65535,
	},
	{ /*                            */
		.read_msg = { 0x80, 0x13, 0x00, 0x02 },
		.read_msg_len = 4,
		.write_msg = { 0x80, 0x13, 0x00, 0x02 },
		.write_msg_len = 4,
		.val_shift = 0,
		.val_max = 65535,
	},
	{ /*                             */
		.read_msg = { 0x80, 0x13, 0x00, 0x03 },
		.read_msg_len = 4,
		.write_msg = { 0x80, 0x13, 0x00, 0x03 },
		.write_msg_len = 4,
		.val_shift = 0,
		.val_max = 65535,
	},
	{ /*                           */
		.read_msg = { 0x80, 0x13, 0x00, 0x04 },
		.read_msg_len = 4,
		.write_msg = { 0x80, 0x13, 0x00, 0x04 },
		.write_msg_len = 4,
		.val_shift = 0,
		.val_max = 65535,
	},
	{ /*                            */
		.read_msg = { 0x80, 0x13, 0x00, 0x05 },
		.read_msg_len = 4,
		.write_msg = { 0x80, 0x13, 0x00, 0x05 },
		.write_msg_len = 4,
		.val_shift = 0,
		.val_max = 65535,
	},
	{ /*                            */
		.read_msg = { 0x80, 0x13, 0x00, 0x06 },
		.read_msg_len = 4,
		.write_msg = { 0x80, 0x13, 0x00, 0x06 },
		.write_msg_len = 4,
		.val_shift = 0,
		.val_max = 65535,
	},
	{ /*                             */
		.read_msg = { 0x80, 0x13, 0x00, 0x07 },
		.read_msg_len = 4,
		.write_msg = { 0x80, 0x13, 0x00, 0x07 },
		.write_msg_len = 4,
		.val_shift = 0,
		.val_max = 65535,
	},
	{ /*                           */
		.read_msg = { 0x80, 0x13, 0x00, 0x08 },
		.read_msg_len = 4,
		.write_msg = { 0x80, 0x13, 0x00, 0x08 },
		.write_msg_len = 4,
		.val_shift = 0,
		.val_max = 65535,
	},
	{ /*                            */
		.read_msg = { 0x80, 0x13, 0x00, 0x09 },
		.read_msg_len = 4,
		.write_msg = { 0x80, 0x13, 0x00, 0x09 },
		.write_msg_len = 4,
		.val_shift = 0,
		.val_max = 65535,
	},
	{ /*                            */
		.read_msg = { 0x80, 0x13, 0x00, 0x0a },
		.read_msg_len = 4,
		.write_msg = { 0x80, 0x13, 0x00, 0x0a },
		.write_msg_len = 4,
		.val_shift = 0,
		.val_max = 65535,
	},
	{ /*                             */
		.read_msg = { 0x80, 0x13, 0x00, 0x0b },
		.read_msg_len = 4,
		.write_msg = { 0x80, 0x13, 0x00, 0x0b },
		.write_msg_len = 4,
		.val_shift = 0,
		.val_max = 65535,
	},
	{ /*                           */
		.read_msg = { 0x80, 0x13, 0x00, 0x0c },
		.read_msg_len = 4,
		.write_msg = { 0x80, 0x13, 0x00, 0x0c },
		.write_msg_len = 4,
		.val_shift = 0,
		.val_max = 65535,
	},
	{ /*                            */
		.read_msg = { 0x80, 0x13, 0x00, 0x0d },
		.read_msg_len = 4,
		.write_msg = { 0x80, 0x13, 0x00, 0x0d },
		.write_msg_len = 4,
		.val_shift = 0,
		.val_max = 65535,
	},
	{ /*                            */
		.read_msg = { 0x80, 0x13, 0x00, 0x0e },
		.read_msg_len = 4,
		.write_msg = { 0x80, 0x13, 0x00, 0x0e },
		.write_msg_len = 4,
		.val_shift = 0,
		.val_max = 65535,
	},
	{ /*                             */
		.read_msg = { 0x80, 0x13, 0x00, 0x0f },
		.read_msg_len = 4,
		.write_msg = { 0x80, 0x13, 0x00, 0x0f },
		.write_msg_len = 4,
		.val_shift = 0,
		.val_max = 65535,
	},
	{ /*                            */
		.read_msg = { 0x80, 0x14, 0x00, 0x00 },
		.read_msg_len = 4,
		.write_msg = { 0x80, 0x14, 0x00, 0x00 },
		.write_msg_len = 4,
		.val_shift = 0,
		.val_max = 65535,
	},
	{ /*                             */
		.read_msg = { 0x80, 0x14, 0x00, 0x01 },
		.read_msg_len = 4,
		.write_msg = { 0x80, 0x14, 0x00, 0x01 },
		.write_msg_len = 4,
		.val_shift = 0,
		.val_max = 65535,
	},
	{ /*                             */
		.read_msg = { 0x80, 0x14, 0x00, 0x02 },
		.read_msg_len = 4,
		.write_msg = { 0x80, 0x14, 0x00, 0x02 },
		.write_msg_len = 4,
		.val_shift = 0,
		.val_max = 65535,
	},
	{ /*                              */
		.read_msg = { 0x80, 0x14, 0x00, 0x03 },
		.read_msg_len = 4,
		.write_msg = { 0x80, 0x14, 0x00, 0x03 },
		.write_msg_len = 4,
		.val_shift = 0,
		.val_max = 65535,
	},
	{ /*                            */
		.read_msg = { 0x80, 0x14, 0x00, 0x04 },
		.read_msg_len = 4,
		.write_msg = { 0x80, 0x14, 0x00, 0x04 },
		.write_msg_len = 4,
		.val_shift = 0,
		.val_max = 65535,
	},
	{ /*                             */
		.read_msg = { 0x80, 0x14, 0x00, 0x05 },
		.read_msg_len = 4,
		.write_msg = { 0x80, 0x14, 0x00, 0x05 },
		.write_msg_len = 4,
		.val_shift = 0,
		.val_max = 65535,
	},
	{ /*                             */
		.read_msg = { 0x80, 0x14, 0x00, 0x06 },
		.read_msg_len = 4,
		.write_msg = { 0x80, 0x14, 0x00, 0x06 },
		.write_msg_len = 4,
		.val_shift = 0,
		.val_max = 65535,
	},
	{ /*                              */
		.read_msg = { 0x80, 0x14, 0x00, 0x07 },
		.read_msg_len = 4,
		.write_msg = { 0x80, 0x14, 0x00, 0x07 },
		.write_msg_len = 4,
		.val_shift = 0,
		.val_max = 65535,
	},
	{ /*                            */
		.read_msg = { 0x80, 0x14, 0x00, 0x08 },
		.read_msg_len = 4,
		.write_msg = { 0x80, 0x14, 0x00, 0x08 },
		.write_msg_len = 4,
		.val_shift = 0,
		.val_max = 65535,
	},
	{ /*                             */
		.read_msg = { 0x80, 0x14, 0x00, 0x09 },
		.read_msg_len = 4,
		.write_msg = { 0x80, 0x14, 0x00, 0x09 },
		.write_msg_len = 4,
		.val_shift = 0,
		.val_max = 65535,
	},
	{ /*                             */
		.read_msg = { 0x80, 0x14, 0x00, 0x0a },
		.read_msg_len = 4,
		.write_msg = { 0x80, 0x14, 0x00, 0x0a },
		.write_msg_len = 4,
		.val_shift = 0,
		.val_max = 65535,
	},
	{ /*                              */
		.read_msg = { 0x80, 0x14, 0x00, 0x0b },
		.read_msg_len = 4,
		.write_msg = { 0x80, 0x14, 0x00, 0x0b },
		.write_msg_len = 4,
		.val_shift = 0,
		.val_max = 65535,
	},
	{ /*                            */
		.read_msg = { 0x80, 0x14, 0x00, 0x0c },
		.read_msg_len = 4,
		.write_msg = { 0x80, 0x14, 0x00, 0x0c },
		.write_msg_len = 4,
		.val_shift = 0,
		.val_max = 65535,
	},
	{ /*                             */
		.read_msg = { 0x80, 0x14, 0x00, 0x0d },
		.read_msg_len = 4,
		.write_msg = { 0x80, 0x14, 0x00, 0x0d },
		.write_msg_len = 4,
		.val_shift = 0,
		.val_max = 65535,
	},
	{ /*                             */
		.read_msg = { 0x80, 0x14, 0x00, 0x0e },
		.read_msg_len = 4,
		.write_msg = { 0x80, 0x14, 0x00, 0x0e },
		.write_msg_len = 4,
		.val_shift = 0,
		.val_max = 65535,
	},
	{ /*                              */
		.read_msg = { 0x80, 0x14, 0x00, 0x0f },
		.read_msg_len = 4,
		.write_msg = { 0x80, 0x14, 0x00, 0x0f },
		.write_msg_len = 4,
		.val_shift = 0,
		.val_max = 65535,
	},
	{ /*                      */
		.read_msg = { 0x80, 0x1D, 0x00, 0x01 },
		.read_msg_len = 4,
		.write_msg = { 0x80, 0x15, 0x00, 0x01 },
		.write_msg_len = 4,
		.val_shift = 0,
		.val_max = 255,
	},
	{ /*                        */
		.read_msg = { 0x80, 0x1D, 0x00, 0x02 },
		.read_msg_len = 4,
		.write_msg = { 0x80, 0x15, 0x02, 0x00 },
		.write_msg_len = 4,
		.val_shift = 0,
		.val_max = 255,
	},
	{ /*                       */
		.read_msg = { 0x80, 0x1D, 0x00, 0x03 },
		.read_msg_len = 4,
		.write_msg = { 0x80, 0x15, 0x03, 0x00 },
		.write_msg_len = 4,
		.val_shift = 0,
		.val_max = 255,
	},
	{ /*                   */
		.read_msg = { 0x80, 0x1D, 0x00, 0x04 },
		.read_msg_len = 4,
		.write_msg = { 0x80, 0x15, 0x04, 0x00 },
		.write_msg_len = 4,
		.val_shift = 0,
		.val_max = 255,
	},
	{ /*                   */
		.read_msg = { 0x80, 0x1D, 0x00, 0x05 },
		.read_msg_len = 4,
		.write_msg = { 0x80, 0x15, 0x05, 0x00 },
		.write_msg_len = 4,
		.val_shift = 0,
		.val_max = 255,
	},
	{ /*                     */
		.read_msg = { 0x80, 0x1D, 0x00, 0x06 },
		.read_msg_len = 4,
		.write_msg = { 0x80, 0x15, 0x06, 0x00 },
		.write_msg_len = 4,
		.val_shift = 0,
		.val_max = 255,
	},
	{ /*                     */
		.read_msg = { 0x80, 0x1D, 0x00, 0x07 },
		.read_msg_len = 4,
		.write_msg = { 0x80, 0x15, 0x07, 0x00 },
		.write_msg_len = 4,
		.val_shift = 0,
		.val_max = 255,
	},
	{ /*                     */
		.read_msg = { 0x80, 0x1D, 0x00, 0x08 },
		.read_msg_len = 4,
		.write_msg = { 0x80, 0x15, 0x08, 0x00 },
		.write_msg_len = 4,
		.val_shift = 0,
		.val_max = 255,
	},
	{ /*                     */
		.read_msg = { 0x80, 0x1D, 0x00, 0x09 },
		.read_msg_len = 4,
		.write_msg = { 0x80, 0x15, 0x09, 0x00 },
		.write_msg_len = 4,
		.val_shift = 0,
		.val_max = 255,
	},
	{ /*                      */
		.read_msg = { 0x80, 0x1D, 0x00, 0x0A },
		.read_msg_len = 4,
		.write_msg = { 0x80, 0x15, 0x0a, 0x00 },
		.write_msg_len = 4,
		.val_shift = 0,
		.val_max = 255,
	},
	{ /*                      */
		.read_msg = { 0x80, 0x1D, 0x00, 0x0B },
		.read_msg_len = 4,
		.write_msg = { 0x80, 0x15, 0x0b, 0x00 },
		.write_msg_len = 4,
		.val_shift = 0,
		.val_max = 255,
	},
	{ /*                    */
		.read_msg = { 0x80, 0x1D, 0x00, 0x10 },
		.read_msg_len = 4,
		.write_msg = { 0x80, 0x15, 0x10, 0x00 },
		.write_msg_len = 4,
		.val_shift = 0,
		.val_max = 255,
	},
	{ /*                     */
		.read_msg = { 0x80, 0x1D, 0x00, 0x11 },
		.read_msg_len = 4,
		.write_msg = { 0x80, 0x15, 0x11, 0x00 },
		.write_msg_len = 4,
		.val_shift = 0,
		.val_max = 255,
	},
	{ /*                     */
		.read_msg = { 0x80, 0x1D, 0x00, 0x12 },
		.read_msg_len = 4,
		.write_msg = { 0x80, 0x15, 0x12, 0x00 },
		.write_msg_len = 4,
		.val_shift = 0,
		.val_max = 255,
	},
	{ /*                      */
		.read_msg = { 0x80, 0x1D, 0x00, 0x13 },
		.read_msg_len = 4,
		.write_msg = { 0x80, 0x15, 0x13, 0x00 },
		.write_msg_len = 4,
		.val_shift = 0,
		.val_max = 255,
	},
	{ /*                      */
		.read_msg = { 0x80, 0x1D, 0x00, 0x14 },
		.read_msg_len = 4,
		.write_msg = { 0x80, 0x15, 0x14, 0x00 },
		.write_msg_len = 4,
		.val_shift = 0,
		.val_max = 255,
	},
	{ /*                      */
		.read_msg = { 0x80, 0x1D, 0x00, 0x15 },
		.read_msg_len = 4,
		.write_msg = { 0x80, 0x15, 0x15, 0x00 },
		.write_msg_len = 4,
		.val_shift = 0,
		.val_max = 255,
	},
	{ /*                      */
		.read_msg = { 0x80, 0x1D, 0x00, 0x16 },
		.read_msg_len = 4,
		.write_msg = { 0x80, 0x15, 0x16, 0x00 },
		.write_msg_len = 4,
		.val_shift = 0,
		.val_max = 255,
	},
	{ /*              */
		.read_msg = { 0x80, 0x59, 0x00, 0x00 },
		.read_msg_len = 4,
		.write_msg = { 0x80, 0x58, 0x00, 0x00 },
		.write_msg_len = 4,
		.val_shift = 0,
		.val_max = 15,
	},
	{ /*              */
		.read_msg = { 0x80, 0x59, 0x10, 0x00 },
		.read_msg_len = 4,
		.write_msg = { 0x80, 0x58, 0x10, 0x00 },
		.write_msg_len = 4,
		.val_shift = 0,
		.val_max = 15,
	},
	{ /*              */
		.read_msg = { 0x80, 0x59, 0x20, 0x00 },
		.read_msg_len = 4,
		.write_msg = { 0x80, 0x58, 0x20, 0x00 },
		.write_msg_len = 4,
		.val_shift = 0,
		.val_max = 15,
	},
	{ /*              */
		.read_msg = { 0x80, 0x59, 0x30, 0x00 },
		.read_msg_len = 4,
		.write_msg = { 0x80, 0x58, 0x30, 0x00 },
		.write_msg_len = 4,
		.val_shift = 0,
		.val_max = 15,
	},
	{ /*                  */
		.read_msg = { 0x80, 0x5B, 0x04, 0x00 },
		.read_msg_len = 4,
		.write_msg = { 0x80, 0x5a, 0x04, 0x00 },
		.write_msg_len = 4,
		.val_shift = 0,
		.val_max = 15,
	},
	{ /*                    */
		.read_msg = { 0x80, 0x5B, 0x08, 0x00 },
		.read_msg_len = 4,
		.write_msg = { 0x80, 0x5a, 0x08, 0x00 },
		.write_msg_len = 4,
		.val_shift = 0,
		.val_max = 15,
	},
	{ /*                   */
		.read_msg = { 0x80, 0x5B, 0x0C, 0x00 },
		.read_msg_len = 4,
		.write_msg = { 0x80, 0x5a, 0x0c, 0x00 },
		.write_msg_len = 4,
		.val_shift = 0,
		.val_max = 15,
	},
	{ /*               */
		.read_msg = { 0x80, 0x5B, 0x10, 0x00 },
		.read_msg_len = 4,
		.write_msg = { 0x80, 0x5a, 0x10, 0x00 },
		.write_msg_len = 4,
		.val_shift = 0,
		.val_max = 15,
	},
	{ /*               */
		.read_msg = { 0x80, 0x5B, 0x14, 0x00 },
		.read_msg_len = 4,
		.write_msg = { 0x80, 0x5a, 0x14, 0x00 },
		.write_msg_len = 4,
		.val_shift = 0,
		.val_max = 15,
	},
	{ /*                 */
		.read_msg = { 0x80, 0x5B, 0x18, 0x00 },
		.read_msg_len = 4,
		.write_msg = { 0x80, 0x5a, 0x18, 0x00 },
		.write_msg_len = 4,
		.val_shift = 0,
		.val_max = 15,
	},
	{ /*                 */
		.read_msg = { 0x80, 0x5B, 0x1C, 0x00 },
		.read_msg_len = 4,
		.write_msg = { 0x80, 0x5a, 0x1c, 0x00 },
		.write_msg_len = 4,
		.val_shift = 0,
		.val_max = 15,
	},
	{ /*                 */
		.read_msg = { 0x80, 0x5B, 0x20, 0x00 },
		.read_msg_len = 4,
		.write_msg = { 0x80, 0x5a, 0x20, 0x00 },
		.write_msg_len = 4,
		.val_shift = 0,
		.val_max = 15,
	},
	{ /*                 */
		.read_msg = { 0x80, 0x5B, 0x24, 0x00 },
		.read_msg_len = 4,
		.write_msg = { 0x80, 0x5a, 0x24, 0x00 },
		.write_msg_len = 4,
		.val_shift = 0,
		.val_max = 15,
	},
	{ /*                  */
		.read_msg = { 0x80, 0x5B, 0x28, 0x00 },
		.read_msg_len = 4,
		.write_msg = { 0x80, 0x5a, 0x28, 0x00 },
		.write_msg_len = 4,
		.val_shift = 0,
		.val_max = 15,
	},
	{ /*                  */
		.read_msg = { 0x80, 0x5B, 0x2C, 0x00 },
		.read_msg_len = 4,
		.write_msg = { 0x80, 0x5a, 0x2c, 0x00 },
		.write_msg_len = 4,
		.val_shift = 0,
		.val_max = 15,
	},
	{ /*                 */
		.read_msg = { 0x80, 0x5B, 0x00, 0x00 },
		.read_msg_len = 4,
		.write_msg = { 0x80, 0x5a, 0x40, 0x00 },
		.write_msg_len = 4,
		.val_shift = 10,
		.val_max = 6,
	},
	{ /*                 */
		.read_msg = { 0x80, 0x5B, 0x00, 0x01 },
		.read_msg_len = 4,
		.write_msg = { 0x80, 0x5a, 0x40, 0x01 },
		.write_msg_len = 4,
		.val_shift = 10,
		.val_max = 6,
	},
	{ /*                 */
		.read_msg = { 0x80, 0x5B, 0x00, 0x02 },
		.read_msg_len = 4,
		.write_msg = { 0x80, 0x5a, 0x40, 0x02 },
		.write_msg_len = 4,
		.val_shift = 10,
		.val_max = 6,
	},
	{ /*                 */
		.read_msg = { 0x80, 0x5B, 0x00, 0x03 },
		.read_msg_len = 4,
		.write_msg = { 0x80, 0x5a, 0x40, 0x03 },
		.write_msg_len = 4,
		.val_shift = 10,
		.val_max = 6,
	},
	{ /*                 */
		.read_msg = { 0x80, 0x5B, 0x00, 0x04 },
		.read_msg_len = 4,
		.write_msg = { 0x80, 0x5a, 0x40, 0x04 },
		.write_msg_len = 4,
		.val_shift = 10,
		.val_max = 6,
	},
	{ /*                 */
		.read_msg = { 0x80, 0x5B, 0x00, 0x05 },
		.read_msg_len = 4,
		.write_msg = { 0x80, 0x5a, 0x40, 0x05 },
		.write_msg_len = 4,
		.val_shift = 10,
		.val_max = 6,
	},
	{ /*                 */
		.read_msg = { 0x80, 0x5B, 0x00, 0x06 },
		.read_msg_len = 4,
		.write_msg = { 0x80, 0x5a, 0x40, 0x06 },
		.write_msg_len = 4,
		.val_shift = 10,
		.val_max = 6,
	},
	{ /*                 */
		.read_msg = { 0x80, 0x5B, 0x00, 0x07 },
		.read_msg_len = 4,
		.write_msg = { 0x80, 0x5a, 0x40, 0x07 },
		.write_msg_len = 4,
		.val_shift = 10,
		.val_max = 6,
	},
	{ /*                 */
		.read_msg = { 0x80, 0x5B, 0x00, 0x08 },
		.read_msg_len = 4,
		.write_msg = { 0x80, 0x5a, 0x40, 0x08 },
		.write_msg_len = 4,
		.val_shift = 10,
		.val_max = 6,
	},
	{ /*                 */
		.read_msg = { 0x80, 0x5B, 0x00, 0x09 },
		.read_msg_len = 4,
		.write_msg = { 0x80, 0x5a, 0x40, 0x09 },
		.write_msg_len = 4,
		.val_shift = 10,
		.val_max = 6,
	},
	{ /*                  */
		.read_msg = { 0x80, 0x5B, 0x00, 0x0A },
		.read_msg_len = 4,
		.write_msg = { 0x80, 0x5a, 0x40, 0x0a },
		.write_msg_len = 4,
		.val_shift = 10,
		.val_max = 6,
	},
	{ /*                  */
		.read_msg = { 0x80, 0x5B, 0x00, 0x0B },
		.read_msg_len = 4,
		.write_msg = { 0x80, 0x5a, 0x40, 0x0b },
		.write_msg_len = 4,
		.val_shift = 10,
		.val_max = 6,
	},
	{ /*                  */
		.read_msg = { 0x80, 0x5B, 0x00, 0x0C },
		.read_msg_len = 4,
		.write_msg = { 0x80, 0x5a, 0x40, 0x0c },
		.write_msg_len = 4,
		.val_shift = 10,
		.val_max = 6,
	},
	{ /*                  */
		.read_msg = { 0x80, 0x5B, 0x00, 0x0D },
		.read_msg_len = 4,
		.write_msg = { 0x80, 0x5a, 0x40, 0x0d },
		.write_msg_len = 4,
		.val_shift = 10,
		.val_max = 6,
	},
	{ /*                  */
		.read_msg = { 0x80, 0x5B, 0x00, 0x0E },
		.read_msg_len = 4,
		.write_msg = { 0x80, 0x5a, 0x40, 0x0e },
		.write_msg_len = 4,
		.val_shift = 10,
		.val_max = 6,
	},
	{ /*                  */
		.read_msg = { 0x80, 0x5B, 0x00, 0x0F },
		.read_msg_len = 4,
		.write_msg = { 0x80, 0x5a, 0x40, 0x0f },
		.write_msg_len = 4,
		.val_shift = 10,
		.val_max = 6,
	},
	{ /*                  */
		.read_msg = { 0x80, 0x5B, 0x00, 0x10 },
		.read_msg_len = 4,
		.write_msg = { 0x80, 0x5a, 0x40, 0x10 },
		.write_msg_len = 4,
		.val_shift = 10,
		.val_max = 6,
	},
	{ /*                  */
		.read_msg = { 0x80, 0x5B, 0x00, 0x11 },
		.read_msg_len = 4,
		.write_msg = { 0x80, 0x5a, 0x40, 0x11 },
		.write_msg_len = 4,
		.val_shift = 10,
		.val_max = 6,
	},
	{ /*                  */
		.read_msg = { 0x80, 0x5B, 0x00, 0x12 },
		.read_msg_len = 4,
		.write_msg = { 0x80, 0x5a, 0x40, 0x12 },
		.write_msg_len = 4,
		.val_shift = 10,
		.val_max = 6,
	},
	{ /*                  */
		.read_msg = { 0x80, 0x5B, 0x00, 0x13 },
		.read_msg_len = 4,
		.write_msg = { 0x80, 0x5a, 0x40, 0x13 },
		.write_msg_len = 4,
		.val_shift = 10,
		.val_max = 6,
	},
	{ /*                  */
		.read_msg = { 0x80, 0x5B, 0x00, 0x14 },
		.read_msg_len = 4,
		.write_msg = { 0x80, 0x5a, 0x40, 0x14 },
		.write_msg_len = 4,
		.val_shift = 10,
		.val_max = 6,
	},
	{ /*                  */
		.read_msg = { 0x80, 0x5B, 0x00, 0x15 },
		.read_msg_len = 4,
		.write_msg = { 0x80, 0x5a, 0x40, 0x15 },
		.write_msg_len = 4,
		.val_shift = 10,
		.val_max = 6,
	},
	{ /*                  */
		.read_msg = { 0x80, 0x5B, 0x00, 0x16 },
		.read_msg_len = 4,
		.write_msg = { 0x80, 0x5a, 0x40, 0x16 },
		.write_msg_len = 4,
		.val_shift = 10,
		.val_max = 6,
	},
	{ /*                  */
		.read_msg = { 0x80, 0x5B, 0x00, 0x17 },
		.read_msg_len = 4,
		.write_msg = { 0x80, 0x5a, 0x40, 0x17 },
		.write_msg_len = 4,
		.val_shift = 10,
		.val_max = 6,
	},
	{ /*                  */
		.read_msg = { 0x80, 0x5B, 0x00, 0x18 },
		.read_msg_len = 4,
		.write_msg = { 0x80, 0x5a, 0x40, 0x18 },
		.write_msg_len = 4,
		.val_shift = 10,
		.val_max = 6,
	},
	{ /*                  */
		.read_msg = { 0x80, 0x5B, 0x00, 0x19 },
		.read_msg_len = 4,
		.write_msg = { 0x80, 0x5a, 0x40, 0x19 },
		.write_msg_len = 4,
		.val_shift = 10,
		.val_max = 6,
	},
	{ /*                  */
		.read_msg = { 0x80, 0x5B, 0x00, 0x1A },
		.read_msg_len = 4,
		.write_msg = { 0x80, 0x5a, 0x40, 0x1a },
		.write_msg_len = 4,
		.val_shift = 10,
		.val_max = 6,
	},
	{ /*                  */
		.read_msg = { 0x80, 0x5B, 0x00, 0x1B },
		.read_msg_len = 4,
		.write_msg = { 0x80, 0x5a, 0x40, 0x1b },
		.write_msg_len = 4,
		.val_shift = 10,
		.val_max = 6,
	},
	{ /*                  */
		.read_msg = { 0x80, 0x5B, 0x00, 0x1C },
		.read_msg_len = 4,
		.write_msg = { 0x80, 0x5a, 0x40, 0x1c },
		.write_msg_len = 4,
		.val_shift = 10,
		.val_max = 6,
	},
	{ /*                  */
		.read_msg = { 0x80, 0x5B, 0x00, 0x1D },
		.read_msg_len = 4,
		.write_msg = { 0x80, 0x5a, 0x40, 0x1d },
		.write_msg_len = 4,
		.val_shift = 10,
		.val_max = 6,
	},
	{ /*                  */
		.read_msg = { 0x80, 0x5B, 0x00, 0x1E },
		.read_msg_len = 4,
		.write_msg = { 0x80, 0x5a, 0x40, 0x1e },
		.write_msg_len = 4,
		.val_shift = 10,
		.val_max = 6,
	},
	{ /*                  */
		.read_msg = { 0x80, 0x5B, 0x00, 0x1F },
		.read_msg_len = 4,
		.write_msg = { 0x80, 0x5a, 0x40, 0x1f },
		.write_msg_len = 4,
		.val_shift = 10,
		.val_max = 6,
	},
	{ /*                 */
		.read_msg = { 0x80, 0x5B, 0x00, 0x20 },
		.read_msg_len = 4,
		.write_msg = { 0x80, 0x5a, 0x40, 0x20 },
		.write_msg_len = 4,
		.val_shift = 10,
		.val_max = 6,
	},
	{ /*                 */
		.read_msg = { 0x80, 0x5B, 0x00, 0x21 },
		.read_msg_len = 4,
		.write_msg = { 0x80, 0x5a, 0x40, 0x21 },
		.write_msg_len = 4,
		.val_shift = 10,
		.val_max = 6,
	},
	{ /*                 */
		.read_msg = { 0x80, 0x5B, 0x00, 0x22 },
		.read_msg_len = 4,
		.write_msg = { 0x80, 0x5a, 0x40, 0x22 },
		.write_msg_len = 4,
		.val_shift = 10,
		.val_max = 6,
	},
	{ /*                 */
		.read_msg = { 0x80, 0x5B, 0x00, 0x23 },
		.read_msg_len = 4,
		.write_msg = { 0x80, 0x5a, 0x40, 0x23 },
		.write_msg_len = 4,
		.val_shift = 10,
		.val_max = 6,
	},
	{ /*                 */
		.read_msg = { 0x80, 0x5B, 0x00, 0x24 },
		.read_msg_len = 4,
		.write_msg = { 0x80, 0x5a, 0x40, 0x24 },
		.write_msg_len = 4,
		.val_shift = 10,
		.val_max = 6,
	},
	{ /*                 */
		.read_msg = { 0x80, 0x5B, 0x00, 0x25 },
		.read_msg_len = 4,
		.write_msg = { 0x80, 0x5a, 0x40, 0x25 },
		.write_msg_len = 4,
		.val_shift = 10,
		.val_max = 6,
	},
	{ /*                 */
		.read_msg = { 0x80, 0x5B, 0x00, 0x26 },
		.read_msg_len = 4,
		.write_msg = { 0x80, 0x5a, 0x40, 0x26 },
		.write_msg_len = 4,
		.val_shift = 10,
		.val_max = 6,
	},
	{ /*                 */
		.read_msg = { 0x80, 0x5B, 0x00, 0x27 },
		.read_msg_len = 4,
		.write_msg = { 0x80, 0x5a, 0x40, 0x27 },
		.write_msg_len = 4,
		.val_shift = 10,
		.val_max = 6,
	},
	{ /*                 */
		.read_msg = { 0x80, 0x5B, 0x00, 0x28 },
		.read_msg_len = 4,
		.write_msg = { 0x80, 0x5a, 0x40, 0x28 },
		.write_msg_len = 4,
		.val_shift = 10,
		.val_max = 6,
	},
	{ /*                 */
		.read_msg = { 0x80, 0x5B, 0x00, 0x29 },
		.read_msg_len = 4,
		.write_msg = { 0x80, 0x5a, 0x40, 0x29 },
		.write_msg_len = 4,
		.val_shift = 10,
		.val_max = 6,
	},
	{ /*                  */
		.read_msg = { 0x80, 0x5B, 0x00, 0x2A },
		.read_msg_len = 4,
		.write_msg = { 0x80, 0x5a, 0x40, 0x2a },
		.write_msg_len = 4,
		.val_shift = 10,
		.val_max = 6,
	},
	{ /*                  */
		.read_msg = { 0x80, 0x5B, 0x00, 0x2B },
		.read_msg_len = 4,
		.write_msg = { 0x80, 0x5a, 0x40, 0x2b },
		.write_msg_len = 4,
		.val_shift = 10,
		.val_max = 6,
	},
	{ /*                  */
		.read_msg = { 0x80, 0x5B, 0x00, 0x2C },
		.read_msg_len = 4,
		.write_msg = { 0x80, 0x5a, 0x40, 0x2c },
		.write_msg_len = 4,
		.val_shift = 10,
		.val_max = 6,
	},
	{ /*                  */
		.read_msg = { 0x80, 0x5B, 0x00, 0x2D },
		.read_msg_len = 4,
		.write_msg = { 0x80, 0x5a, 0x40, 0x2d },
		.write_msg_len = 4,
		.val_shift = 10,
		.val_max = 6,
	},
	{ /*                  */
		.read_msg = { 0x80, 0x5B, 0x00, 0x2E },
		.read_msg_len = 4,
		.write_msg = { 0x80, 0x5a, 0x40, 0x2e },
		.write_msg_len = 4,
		.val_shift = 10,
		.val_max = 6,
	},
	{ /*                  */
		.read_msg = { 0x80, 0x5B, 0x00, 0x2F },
		.read_msg_len = 4,
		.write_msg = { 0x80, 0x5a, 0x40, 0x2f },
		.write_msg_len = 4,
		.val_shift = 10,
		.val_max = 6,
	},
	{ /*                  */
		.read_msg = { 0x80, 0x5B, 0x00, 0x30 },
		.read_msg_len = 4,
		.write_msg = { 0x80, 0x5a, 0x40, 0x30 },
		.write_msg_len = 4,
		.val_shift = 10,
		.val_max = 6,
	},
	{ /*                  */
		.read_msg = { 0x80, 0x5B, 0x00, 0x31 },
		.read_msg_len = 4,
		.write_msg = { 0x80, 0x5a, 0x40, 0x31 },
		.write_msg_len = 4,
		.val_shift = 10,
		.val_max = 6,
	},
	{ /*                  */
		.read_msg = { 0x80, 0x5B, 0x00, 0x32 },
		.read_msg_len = 4,
		.write_msg = { 0x80, 0x5a, 0x40, 0x32 },
		.write_msg_len = 4,
		.val_shift = 10,
		.val_max = 6,
	},
	{ /*                  */
		.read_msg = { 0x80, 0x5B, 0x00, 0x33 },
		.read_msg_len = 4,
		.write_msg = { 0x80, 0x5a, 0x40, 0x33 },
		.write_msg_len = 4,
		.val_shift = 10,
		.val_max = 6,
	},
	{ /*                  */
		.read_msg = { 0x80, 0x5B, 0x00, 0x34 },
		.read_msg_len = 4,
		.write_msg = { 0x80, 0x5a, 0x40, 0x34 },
		.write_msg_len = 4,
		.val_shift = 10,
		.val_max = 6,
	},
	{ /*                  */
		.read_msg = { 0x80, 0x5B, 0x00, 0x35 },
		.read_msg_len = 4,
		.write_msg = { 0x80, 0x5a, 0x40, 0x35 },
		.write_msg_len = 4,
		.val_shift = 10,
		.val_max = 6,
	},
	{ /*                  */
		.read_msg = { 0x80, 0x5B, 0x00, 0x36 },
		.read_msg_len = 4,
		.write_msg = { 0x80, 0x5a, 0x40, 0x36 },
		.write_msg_len = 4,
		.val_shift = 10,
		.val_max = 6,
	},
	{ /*                  */
		.read_msg = { 0x80, 0x5B, 0x00, 0x37 },
		.read_msg_len = 4,
		.write_msg = { 0x80, 0x5a, 0x40, 0x37 },
		.write_msg_len = 4,
		.val_shift = 10,
		.val_max = 6,
	},
	{ /*                  */
		.read_msg = { 0x80, 0x5B, 0x00, 0x38 },
		.read_msg_len = 4,
		.write_msg = { 0x80, 0x5a, 0x40, 0x38 },
		.write_msg_len = 4,
		.val_shift = 10,
		.val_max = 6,
	},
	{ /*                  */
		.read_msg = { 0x80, 0x5B, 0x00, 0x39 },
		.read_msg_len = 4,
		.write_msg = { 0x80, 0x5a, 0x40, 0x39 },
		.write_msg_len = 4,
		.val_shift = 10,
		.val_max = 6,
	},
	{ /*                  */
		.read_msg = { 0x80, 0x5B, 0x00, 0x3A },
		.read_msg_len = 4,
		.write_msg = { 0x80, 0x5a, 0x40, 0x3a },
		.write_msg_len = 4,
		.val_shift = 10,
		.val_max = 6,
	},
	{ /*                  */
		.read_msg = { 0x80, 0x5B, 0x00, 0x3B },
		.read_msg_len = 4,
		.write_msg = { 0x80, 0x5a, 0x40, 0x3b },
		.write_msg_len = 4,
		.val_shift = 10,
		.val_max = 6,
	},
	{ /*                  */
		.read_msg = { 0x80, 0x5B, 0x00, 0x3C },
		.read_msg_len = 4,
		.write_msg = { 0x80, 0x5a, 0x40, 0x3c },
		.write_msg_len = 4,
		.val_shift = 10,
		.val_max = 6,
	},
	{ /*                  */
		.read_msg = { 0x80, 0x5B, 0x00, 0x3D },
		.read_msg_len = 4,
		.write_msg = { 0x80, 0x5a, 0x40, 0x3d },
		.write_msg_len = 4,
		.val_shift = 10,
		.val_max = 6,
	},
	{ /*                  */
		.read_msg = { 0x80, 0x5B, 0x00, 0x3E },
		.read_msg_len = 4,
		.write_msg = { 0x80, 0x5a, 0x40, 0x3e },
		.write_msg_len = 4,
		.val_shift = 10,
		.val_max = 6,
	},
	{ /*                  */
		.read_msg = { 0x80, 0x5B, 0x00, 0x3F },
		.read_msg_len = 4,
		.write_msg = { 0x80, 0x5a, 0x40, 0x3f },
		.write_msg_len = 4,
		.val_shift = 10,
		.val_max = 6,
	},
	{ /*                 */
		.read_msg = { 0x80, 0x5B, 0x00, 0x40 },
		.read_msg_len = 4,
		.write_msg = { 0x80, 0x5a, 0x40, 0x40 },
		.write_msg_len = 4,
		.val_shift = 10,
		.val_max = 6,
	},
	{ /*                 */
		.read_msg = { 0x80, 0x5B, 0x00, 0x41 },
		.read_msg_len = 4,
		.write_msg = { 0x80, 0x5a, 0x40, 0x41 },
		.write_msg_len = 4,
		.val_shift = 10,
		.val_max = 6,
	},
	{ /*                 */
		.read_msg = { 0x80, 0x5B, 0x00, 0x42 },
		.read_msg_len = 4,
		.write_msg = { 0x80, 0x5a, 0x40, 0x42 },
		.write_msg_len = 4,
		.val_shift = 10,
		.val_max = 6,
	},
	{ /*                 */
		.read_msg = { 0x80, 0x5B, 0x00, 0x43 },
		.read_msg_len = 4,
		.write_msg = { 0x80, 0x5a, 0x40, 0x43 },
		.write_msg_len = 4,
		.val_shift = 10,
		.val_max = 6,
	},
	{ /*                 */
		.read_msg = { 0x80, 0x5B, 0x00, 0x44 },
		.read_msg_len = 4,
		.write_msg = { 0x80, 0x5a, 0x40, 0x44 },
		.write_msg_len = 4,
		.val_shift = 10,
		.val_max = 6,
	},
	{ /*                 */
		.read_msg = { 0x80, 0x5B, 0x00, 0x45 },
		.read_msg_len = 4,
		.write_msg = { 0x80, 0x5a, 0x40, 0x45 },
		.write_msg_len = 4,
		.val_shift = 10,
		.val_max = 6,
	},
	{ /*                 */
		.read_msg = { 0x80, 0x5B, 0x00, 0x46 },
		.read_msg_len = 4,
		.write_msg = { 0x80, 0x5a, 0x40, 0x46 },
		.write_msg_len = 4,
		.val_shift = 10,
		.val_max = 6,
	},
	{ /*                 */
		.read_msg = { 0x80, 0x5B, 0x00, 0x47 },
		.read_msg_len = 4,
		.write_msg = { 0x80, 0x5a, 0x40, 0x47 },
		.write_msg_len = 4,
		.val_shift = 10,
		.val_max = 6,
	},
	{ /*                 */
		.read_msg = { 0x80, 0x5B, 0x00, 0x48 },
		.read_msg_len = 4,
		.write_msg = { 0x80, 0x5a, 0x40, 0x48 },
		.write_msg_len = 4,
		.val_shift = 10,
		.val_max = 6,
	},
	{ /*                 */
		.read_msg = { 0x80, 0x5B, 0x00, 0x49 },
		.read_msg_len = 4,
		.write_msg = { 0x80, 0x5a, 0x40, 0x49 },
		.write_msg_len = 4,
		.val_shift = 10,
		.val_max = 6,
	},
	{ /*                  */
		.read_msg = { 0x80, 0x5B, 0x00, 0x4A },
		.read_msg_len = 4,
		.write_msg = { 0x80, 0x5a, 0x40, 0x4a },
		.write_msg_len = 4,
		.val_shift = 10,
		.val_max = 6,
	},
	{ /*                  */
		.read_msg = { 0x80, 0x5B, 0x00, 0x4B },
		.read_msg_len = 4,
		.write_msg = { 0x80, 0x5a, 0x40, 0x4b },
		.write_msg_len = 4,
		.val_shift = 10,
		.val_max = 6,
	},
	{ /*                  */
		.read_msg = { 0x80, 0x5B, 0x00, 0x4C },
		.read_msg_len = 4,
		.write_msg = { 0x80, 0x5a, 0x40, 0x4c },
		.write_msg_len = 4,
		.val_shift = 10,
		.val_max = 6,
	},
	{ /*                  */
		.read_msg = { 0x80, 0x5B, 0x00, 0x4D },
		.read_msg_len = 4,
		.write_msg = { 0x80, 0x5a, 0x40, 0x4d },
		.write_msg_len = 4,
		.val_shift = 10,
		.val_max = 6,
	},
	{ /*                  */
		.read_msg = { 0x80, 0x5B, 0x00, 0x4E },
		.read_msg_len = 4,
		.write_msg = { 0x80, 0x5a, 0x40, 0x4e },
		.write_msg_len = 4,
		.val_shift = 10,
		.val_max = 6,
	},
	{ /*                  */
		.read_msg = { 0x80, 0x5B, 0x00, 0x4F },
		.read_msg_len = 4,
		.write_msg = { 0x80, 0x5a, 0x40, 0x4f },
		.write_msg_len = 4,
		.val_shift = 10,
		.val_max = 6,
	},
	{ /*                  */
		.read_msg = { 0x80, 0x5B, 0x00, 0x50 },
		.read_msg_len = 4,
		.write_msg = { 0x80, 0x5a, 0x40, 0x50 },
		.write_msg_len = 4,
		.val_shift = 10,
		.val_max = 6,
	},
	{ /*                  */
		.read_msg = { 0x80, 0x5B, 0x00, 0x51 },
		.read_msg_len = 4,
		.write_msg = { 0x80, 0x5a, 0x40, 0x51 },
		.write_msg_len = 4,
		.val_shift = 10,
		.val_max = 6,
	},
	{ /*                  */
		.read_msg = { 0x80, 0x5B, 0x00, 0x52 },
		.read_msg_len = 4,
		.write_msg = { 0x80, 0x5a, 0x40, 0x52 },
		.write_msg_len = 4,
		.val_shift = 10,
		.val_max = 6,
	},
	{ /*                  */
		.read_msg = { 0x80, 0x5B, 0x00, 0x53 },
		.read_msg_len = 4,
		.write_msg = { 0x80, 0x5a, 0x40, 0x53 },
		.write_msg_len = 4,
		.val_shift = 10,
		.val_max = 6,
	},
	{ /*                  */
		.read_msg = { 0x80, 0x5B, 0x00, 0x54 },
		.read_msg_len = 4,
		.write_msg = { 0x80, 0x5a, 0x40, 0x54 },
		.write_msg_len = 4,
		.val_shift = 10,
		.val_max = 6,
	},
	{ /*                  */
		.read_msg = { 0x80, 0x5B, 0x00, 0x55 },
		.read_msg_len = 4,
		.write_msg = { 0x80, 0x5a, 0x40, 0x55 },
		.write_msg_len = 4,
		.val_shift = 10,
		.val_max = 6,
	},
	{ /*                  */
		.read_msg = { 0x80, 0x5B, 0x00, 0x56 },
		.read_msg_len = 4,
		.write_msg = { 0x80, 0x5a, 0x40, 0x56 },
		.write_msg_len = 4,
		.val_shift = 10,
		.val_max = 6,
	},
	{ /*                  */
		.read_msg = { 0x80, 0x5B, 0x00, 0x57 },
		.read_msg_len = 4,
		.write_msg = { 0x80, 0x5a, 0x40, 0x57 },
		.write_msg_len = 4,
		.val_shift = 10,
		.val_max = 6,
	},
	{ /*                  */
		.read_msg = { 0x80, 0x5B, 0x00, 0x58 },
		.read_msg_len = 4,
		.write_msg = { 0x80, 0x5a, 0x40, 0x58 },
		.write_msg_len = 4,
		.val_shift = 10,
		.val_max = 6,
	},
	{ /*                  */
		.read_msg = { 0x80, 0x5B, 0x00, 0x59 },
		.read_msg_len = 4,
		.write_msg = { 0x80, 0x5a, 0x40, 0x59 },
		.write_msg_len = 4,
		.val_shift = 10,
		.val_max = 6,
	},
	{ /*                  */
		.read_msg = { 0x80, 0x5B, 0x00, 0x5A },
		.read_msg_len = 4,
		.write_msg = { 0x80, 0x5a, 0x40, 0x5a },
		.write_msg_len = 4,
		.val_shift = 10,
		.val_max = 6,
	},
	{ /*                  */
		.read_msg = { 0x80, 0x5B, 0x00, 0x5B },
		.read_msg_len = 4,
		.write_msg = { 0x80, 0x5a, 0x40, 0x5b },
		.write_msg_len = 4,
		.val_shift = 10,
		.val_max = 6,
	},
	{ /*                  */
		.read_msg = { 0x80, 0x5B, 0x00, 0x5C },
		.read_msg_len = 4,
		.write_msg = { 0x80, 0x5a, 0x40, 0x5c },
		.write_msg_len = 4,
		.val_shift = 10,
		.val_max = 6,
	},
	{ /*                  */
		.read_msg = { 0x80, 0x5B, 0x00, 0x5D },
		.read_msg_len = 4,
		.write_msg = { 0x80, 0x5a, 0x40, 0x5d },
		.write_msg_len = 4,
		.val_shift = 10,
		.val_max = 6,
	},
	{ /*                  */
		.read_msg = { 0x80, 0x5B, 0x00, 0x5E },
		.read_msg_len = 4,
		.write_msg = { 0x80, 0x5a, 0x40, 0x5e },
		.write_msg_len = 4,
		.val_shift = 10,
		.val_max = 6,
	},
	{ /*                  */
		.read_msg = { 0x80, 0x5B, 0x00, 0x5F },
		.read_msg_len = 4,
		.write_msg = { 0x80, 0x5a, 0x40, 0x5f },
		.write_msg_len = 4,
		.val_shift = 10,
		.val_max = 6,
	},
	{ /*                 */
		.read_msg = { 0x80, 0x5B, 0x00, 0x60 },
		.read_msg_len = 4,
		.write_msg = { 0x80, 0x5a, 0x40, 0x60 },
		.write_msg_len = 4,
		.val_shift = 10,
		.val_max = 6,
	},
	{ /*                 */
		.read_msg = { 0x80, 0x5B, 0x00, 0x61 },
		.read_msg_len = 4,
		.write_msg = { 0x80, 0x5a, 0x40, 0x61 },
		.write_msg_len = 4,
		.val_shift = 10,
		.val_max = 6,
	},
	{ /*                 */
		.read_msg = { 0x80, 0x5B, 0x00, 0x62 },
		.read_msg_len = 4,
		.write_msg = { 0x80, 0x5a, 0x40, 0x62 },
		.write_msg_len = 4,
		.val_shift = 10,
		.val_max = 6,
	},
	{ /*                 */
		.read_msg = { 0x80, 0x5B, 0x00, 0x63 },
		.read_msg_len = 4,
		.write_msg = { 0x80, 0x5a, 0x40, 0x63 },
		.write_msg_len = 4,
		.val_shift = 10,
		.val_max = 6,
	},
	{ /*                 */
		.read_msg = { 0x80, 0x5B, 0x00, 0x64 },
		.read_msg_len = 4,
		.write_msg = { 0x80, 0x5a, 0x40, 0x64 },
		.write_msg_len = 4,
		.val_shift = 10,
		.val_max = 6,
	},
	{ /*                 */
		.read_msg = { 0x80, 0x5B, 0x00, 0x65 },
		.read_msg_len = 4,
		.write_msg = { 0x80, 0x5a, 0x40, 0x65 },
		.write_msg_len = 4,
		.val_shift = 10,
		.val_max = 6,
	},
	{ /*                 */
		.read_msg = { 0x80, 0x5B, 0x00, 0x66 },
		.read_msg_len = 4,
		.write_msg = { 0x80, 0x5a, 0x40, 0x66 },
		.write_msg_len = 4,
		.val_shift = 10,
		.val_max = 6,
	},
	{ /*                 */
		.read_msg = { 0x80, 0x5B, 0x00, 0x67 },
		.read_msg_len = 4,
		.write_msg = { 0x80, 0x5a, 0x40, 0x67 },
		.write_msg_len = 4,
		.val_shift = 10,
		.val_max = 6,
	},
	{ /*                 */
		.read_msg = { 0x80, 0x5B, 0x00, 0x68 },
		.read_msg_len = 4,
		.write_msg = { 0x80, 0x5a, 0x40, 0x68 },
		.write_msg_len = 4,
		.val_shift = 10,
		.val_max = 6,
	},
	{ /*                 */
		.read_msg = { 0x80, 0x5B, 0x00, 0x69 },
		.read_msg_len = 4,
		.write_msg = { 0x80, 0x5a, 0x40, 0x69 },
		.write_msg_len = 4,
		.val_shift = 10,
		.val_max = 6,
	},
	{ /*                  */
		.read_msg = { 0x80, 0x5B, 0x00, 0x6A },
		.read_msg_len = 4,
		.write_msg = { 0x80, 0x5a, 0x40, 0x6a },
		.write_msg_len = 4,
		.val_shift = 10,
		.val_max = 6,
	},
	{ /*                  */
		.read_msg = { 0x80, 0x5B, 0x00, 0x6B },
		.read_msg_len = 4,
		.write_msg = { 0x80, 0x5a, 0x40, 0x6b },
		.write_msg_len = 4,
		.val_shift = 10,
		.val_max = 6,
	},
	{ /*                  */
		.read_msg = { 0x80, 0x5B, 0x00, 0x6C },
		.read_msg_len = 4,
		.write_msg = { 0x80, 0x5a, 0x40, 0x6c },
		.write_msg_len = 4,
		.val_shift = 10,
		.val_max = 6,
	},
	{ /*                  */
		.read_msg = { 0x80, 0x5B, 0x00, 0x6D },
		.read_msg_len = 4,
		.write_msg = { 0x80, 0x5a, 0x40, 0x6d },
		.write_msg_len = 4,
		.val_shift = 10,
		.val_max = 6,
	},
	{ /*                  */
		.read_msg = { 0x80, 0x5B, 0x00, 0x6E },
		.read_msg_len = 4,
		.write_msg = { 0x80, 0x5a, 0x40, 0x6e },
		.write_msg_len = 4,
		.val_shift = 10,
		.val_max = 6,
	},
	{ /*                  */
		.read_msg = { 0x80, 0x5B, 0x00, 0x6F },
		.read_msg_len = 4,
		.write_msg = { 0x80, 0x5a, 0x40, 0x6f },
		.write_msg_len = 4,
		.val_shift = 10,
		.val_max = 6,
	},
	{ /*                  */
		.read_msg = { 0x80, 0x5B, 0x00, 0x70 },
		.read_msg_len = 4,
		.write_msg = { 0x80, 0x5a, 0x40, 0x70 },
		.write_msg_len = 4,
		.val_shift = 10,
		.val_max = 6,
	},
	{ /*                  */
		.read_msg = { 0x80, 0x5B, 0x00, 0x71 },
		.read_msg_len = 4,
		.write_msg = { 0x80, 0x5a, 0x40, 0x71 },
		.write_msg_len = 4,
		.val_shift = 10,
		.val_max = 6,
	},
	{ /*                  */
		.read_msg = { 0x80, 0x5B, 0x00, 0x72 },
		.read_msg_len = 4,
		.write_msg = { 0x80, 0x5a, 0x40, 0x72 },
		.write_msg_len = 4,
		.val_shift = 10,
		.val_max = 6,
	},
	{ /*                  */
		.read_msg = { 0x80, 0x5B, 0x00, 0x73 },
		.read_msg_len = 4,
		.write_msg = { 0x80, 0x5a, 0x40, 0x73 },
		.write_msg_len = 4,
		.val_shift = 10,
		.val_max = 6,
	},
	{ /*                  */
		.read_msg = { 0x80, 0x5B, 0x00, 0x74 },
		.read_msg_len = 4,
		.write_msg = { 0x80, 0x5a, 0x40, 0x74 },
		.write_msg_len = 4,
		.val_shift = 10,
		.val_max = 6,
	},
	{ /*                  */
		.read_msg = { 0x80, 0x5B, 0x00, 0x75 },
		.read_msg_len = 4,
		.write_msg = { 0x80, 0x5a, 0x40, 0x75 },
		.write_msg_len = 4,
		.val_shift = 10,
		.val_max = 6,
	},
	{ /*                  */
		.read_msg = { 0x80, 0x5B, 0x00, 0x76 },
		.read_msg_len = 4,
		.write_msg = { 0x80, 0x5a, 0x40, 0x76 },
		.write_msg_len = 4,
		.val_shift = 10,
		.val_max = 6,
	},
	{ /*                  */
		.read_msg = { 0x80, 0x5B, 0x00, 0x77 },
		.read_msg_len = 4,
		.write_msg = { 0x80, 0x5a, 0x40, 0x77 },
		.write_msg_len = 4,
		.val_shift = 10,
		.val_max = 6,
	},
	{ /*                  */
		.read_msg = { 0x80, 0x5B, 0x00, 0x78 },
		.read_msg_len = 4,
		.write_msg = { 0x80, 0x5a, 0x40, 0x78 },
		.write_msg_len = 4,
		.val_shift = 10,
		.val_max = 6,
	},
	{ /*                  */
		.read_msg = { 0x80, 0x5B, 0x00, 0x79 },
		.read_msg_len = 4,
		.write_msg = { 0x80, 0x5a, 0x40, 0x79 },
		.write_msg_len = 4,
		.val_shift = 10,
		.val_max = 6,
	},
	{ /*                  */
		.read_msg = { 0x80, 0x5B, 0x00, 0x7A },
		.read_msg_len = 4,
		.write_msg = { 0x80, 0x5a, 0x40, 0x7a },
		.write_msg_len = 4,
		.val_shift = 10,
		.val_max = 6,
	},
	{ /*                  */
		.read_msg = { 0x80, 0x5B, 0x00, 0x7B },
		.read_msg_len = 4,
		.write_msg = { 0x80, 0x5a, 0x40, 0x7b },
		.write_msg_len = 4,
		.val_shift = 10,
		.val_max = 6,
	},
	{ /*                  */
		.read_msg = { 0x80, 0x5B, 0x00, 0x7C },
		.read_msg_len = 4,
		.write_msg = { 0x80, 0x5a, 0x40, 0x7c },
		.write_msg_len = 4,
		.val_shift = 10,
		.val_max = 6,
	},
	{ /*                  */
		.read_msg = { 0x80, 0x5B, 0x00, 0x7D },
		.read_msg_len = 4,
		.write_msg = { 0x80, 0x5a, 0x40, 0x7d },
		.write_msg_len = 4,
		.val_shift = 10,
		.val_max = 6,
	},
	{ /*                  */
		.read_msg = { 0x80, 0x5B, 0x00, 0x7E },
		.read_msg_len = 4,
		.write_msg = { 0x80, 0x5a, 0x40, 0x7e },
		.write_msg_len = 4,
		.val_shift = 10,
		.val_max = 6,
	},
	{ /*                  */
		.read_msg = { 0x80, 0x5B, 0x00, 0x7F },
		.read_msg_len = 4,
		.write_msg = { 0x80, 0x5a, 0x40, 0x7f },
		.write_msg_len = 4,
		.val_shift = 10,
		.val_max = 6,
	},
	{ /*                   */
		.read_msg = { 0x80, 0x5B, 0x00, 0xAA },
		.read_msg_len = 4,
		.write_msg = { 0x80, 0x5a, 0x40, 0xaa },
		.write_msg_len = 4,
		.val_shift = 10,
		.val_max = 6,
	},
	{ /*                   */
		.read_msg = { 0x80, 0x5B, 0x00, 0xAB },
		.read_msg_len = 4,
		.write_msg = { 0x80, 0x5a, 0x40, 0xab },
		.write_msg_len = 4,
		.val_shift = 10,
		.val_max = 6,
	},
	{ /*                   */
		.read_msg = { 0x80, 0x5B, 0x00, 0xAC },
		.read_msg_len = 4,
		.write_msg = { 0x80, 0x5a, 0x40, 0xac },
		.write_msg_len = 4,
		.val_shift = 10,
		.val_max = 6,
	},
	{ /*                   */
		.read_msg = { 0x80, 0x5B, 0x00, 0xAD },
		.read_msg_len = 4,
		.write_msg = { 0x80, 0x5a, 0x40, 0xad },
		.write_msg_len = 4,
		.val_shift = 10,
		.val_max = 6,
	},
	{ /*                   */
		.read_msg = { 0x80, 0x5B, 0x00, 0xAE },
		.read_msg_len = 4,
		.write_msg = { 0x80, 0x5a, 0x40, 0xae },
		.write_msg_len = 4,
		.val_shift = 10,
		.val_max = 6,
	},
	{ /*                   */
		.read_msg = { 0x80, 0x5B, 0x00, 0xAF },
		.read_msg_len = 4,
		.write_msg = { 0x80, 0x5a, 0x40, 0xaf },
		.write_msg_len = 4,
		.val_shift = 10,
		.val_max = 6,
	},
	{ /*       */
		.read_msg = { 0x80, 0x5A, 0x00, 0x00 },
		.read_msg_len = 4,
		.write_msg = { 0x80, 0x59, 0x00, 0x00 },
		.write_msg_len = 4,
		.val_shift = 0,
		.val_max = 0,
	},
};

#endif /*                 */
