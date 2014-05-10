/* include/linux/platform_data/hds_max1462x.h
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

#ifndef __HDS_MAX1462X_H__
#define __HDS_MAX1462X_H__

struct max1462x_platform_data {
    /*           */
    const char *switch_name;    /*                    */
    const char *keypad_name;    /*                    */

    unsigned int key_code;      /*                                                                                                   */

    unsigned int gpio_mode;     /*                          */
    unsigned int gpio_det;      /*                                      */
    unsigned int gpio_swd;      /*                                                                               */

    unsigned int latency_for_detection; /*                                         */
    unsigned int latency_for_key;       /*                                            */

    /*          */
    unsigned int adc_mpp_num;   /*                                              */
    unsigned int adc_channel;   /*                                           */

    unsigned int external_ldo_mic_bias;         /*                                  */
    void (*set_headset_mic_bias)(int enable);   /*                                               */
};

#endif /*                    */
