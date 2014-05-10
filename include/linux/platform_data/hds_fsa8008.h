/* include/linux/platform_data/hds_fsa8008.h
 *
 * Copyright (C) 2012 LG Electronics Inc.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#ifndef __HDS_FSA8008_H__
#define __HDS_FSA8008_H__

struct fsa8008_platform_data {
	const char *switch_name;  /*                    */
	const char *keypad_name;  /*                    */

	unsigned int key_code;    /*                   */

	unsigned int gpio_detect; /*                                      */
	unsigned int gpio_detect_can_wakeup;
	unsigned int gpio_mic_en; /*                    */
	unsigned int gpio_mic_bias_en; /*                         */
	unsigned int gpio_jpole;  /*                        */
	unsigned int gpio_key;    /*            */

	/*                                            */
	unsigned int latency_for_detection;

	/*                                                      */
	void (*set_headset_mic_bias)(int enable);
	void (*set_uart_console)(int enable);
};

#endif /*                   */
