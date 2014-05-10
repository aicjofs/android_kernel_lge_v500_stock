/*
 * wm5100.h  --  WM5100 ALSA SoC Audio driver
 *
 * Copyright 2011 Wolfson Microelectronics plc
 *
 * Author: Mark Brown <broonie@opensource.wolfsonmicro.com>
 *
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef WM5100_ASOC_H
#define WM5100_ASOC_H

#include <sound/soc.h>
#include <linux/regmap.h>

int wm5100_detect(struct snd_soc_codec *codec, struct snd_soc_jack *jack);

#define WM5100_CLK_AIF1     1
#define WM5100_CLK_AIF2     2
#define WM5100_CLK_AIF3     3
#define WM5100_CLK_SYSCLK   4
#define WM5100_CLK_ASYNCCLK 5
#define WM5100_CLK_32KHZ    6
#define WM5100_CLK_OPCLK    7

#define WM5100_CLKSRC_MCLK1    0
#define WM5100_CLKSRC_MCLK2    1
#define WM5100_CLKSRC_SYSCLK   2
#define WM5100_CLKSRC_FLL1     4
#define WM5100_CLKSRC_FLL2     5
#define WM5100_CLKSRC_AIF1BCLK 8
#define WM5100_CLKSRC_AIF2BCLK 9
#define WM5100_CLKSRC_AIF3BCLK 10
#define WM5100_CLKSRC_ASYNCCLK 0x100

#define WM5100_FLL1 1
#define WM5100_FLL2 2

#define WM5100_FLL_SRC_MCLK1    0x0
#define WM5100_FLL_SRC_MCLK2    0x1
#define WM5100_FLL_SRC_FLL1     0x4
#define WM5100_FLL_SRC_FLL2     0x5
#define WM5100_FLL_SRC_AIF1BCLK 0x8
#define WM5100_FLL_SRC_AIF2BCLK 0x9
#define WM5100_FLL_SRC_AIF3BCLK 0xa

/*
                   
 */
#define WM5100_SOFTWARE_RESET                   0x00
#define WM5100_DEVICE_REVISION                  0x01
#define WM5100_CTRL_IF_1                        0x10
#define WM5100_TONE_GENERATOR_1                 0x20
#define WM5100_PWM_DRIVE_1                      0x30
#define WM5100_PWM_DRIVE_2                      0x31
#define WM5100_PWM_DRIVE_3                      0x32
#define WM5100_CLOCKING_1                       0x100
#define WM5100_CLOCKING_3                       0x101
#define WM5100_CLOCKING_4                       0x102
#define WM5100_CLOCKING_5                       0x103
#define WM5100_CLOCKING_6                       0x104
#define WM5100_CLOCKING_7                       0x107
#define WM5100_CLOCKING_8                       0x108
#define WM5100_ASRC_ENABLE                      0x120
#define WM5100_ASRC_STATUS                      0x121
#define WM5100_ASRC_RATE1                       0x122
#define WM5100_ISRC_1_CTRL_1                    0x141
#define WM5100_ISRC_1_CTRL_2                    0x142
#define WM5100_ISRC_2_CTRL1                     0x143
#define WM5100_ISRC_2_CTRL_2                    0x144
#define WM5100_FLL1_CONTROL_1                   0x182
#define WM5100_FLL1_CONTROL_2                   0x183
#define WM5100_FLL1_CONTROL_3                   0x184
#define WM5100_FLL1_CONTROL_5                   0x186
#define WM5100_FLL1_CONTROL_6                   0x187
#define WM5100_FLL1_EFS_1                       0x188
#define WM5100_FLL2_CONTROL_1                   0x1A2
#define WM5100_FLL2_CONTROL_2                   0x1A3
#define WM5100_FLL2_CONTROL_3                   0x1A4
#define WM5100_FLL2_CONTROL_5                   0x1A6
#define WM5100_FLL2_CONTROL_6                   0x1A7
#define WM5100_FLL2_EFS_1                       0x1A8
#define WM5100_MIC_CHARGE_PUMP_1                0x200
#define WM5100_MIC_CHARGE_PUMP_2                0x201
#define WM5100_HP_CHARGE_PUMP_1                 0x202
#define WM5100_LDO1_CONTROL                     0x211
#define WM5100_MIC_BIAS_CTRL_1                  0x215
#define WM5100_MIC_BIAS_CTRL_2                  0x216
#define WM5100_MIC_BIAS_CTRL_3                  0x217
#define WM5100_ACCESSORY_DETECT_MODE_1          0x280
#define WM5100_HEADPHONE_DETECT_1               0x288
#define WM5100_HEADPHONE_DETECT_2               0x289
#define WM5100_MIC_DETECT_1                     0x290
#define WM5100_MIC_DETECT_2                     0x291
#define WM5100_MIC_DETECT_3                     0x292
#define WM5100_MISC_CONTROL                     0x2BB
#define WM5100_INPUT_ENABLES                    0x301
#define WM5100_INPUT_ENABLES_STATUS             0x302
#define WM5100_IN1L_CONTROL                     0x310
#define WM5100_IN1R_CONTROL                     0x311
#define WM5100_IN2L_CONTROL                     0x312
#define WM5100_IN2R_CONTROL                     0x313
#define WM5100_IN3L_CONTROL                     0x314
#define WM5100_IN3R_CONTROL                     0x315
#define WM5100_IN4L_CONTROL                     0x316
#define WM5100_IN4R_CONTROL                     0x317
#define WM5100_RXANC_SRC                        0x318
#define WM5100_INPUT_VOLUME_RAMP                0x319
#define WM5100_ADC_DIGITAL_VOLUME_1L            0x320
#define WM5100_ADC_DIGITAL_VOLUME_1R            0x321
#define WM5100_ADC_DIGITAL_VOLUME_2L            0x322
#define WM5100_ADC_DIGITAL_VOLUME_2R            0x323
#define WM5100_ADC_DIGITAL_VOLUME_3L            0x324
#define WM5100_ADC_DIGITAL_VOLUME_3R            0x325
#define WM5100_ADC_DIGITAL_VOLUME_4L            0x326
#define WM5100_ADC_DIGITAL_VOLUME_4R            0x327
#define WM5100_OUTPUT_ENABLES_2                 0x401
#define WM5100_OUTPUT_STATUS_1                  0x402
#define WM5100_OUTPUT_STATUS_2                  0x403
#define WM5100_CHANNEL_ENABLES_1                0x408
#define WM5100_OUT_VOLUME_1L                    0x410
#define WM5100_OUT_VOLUME_1R                    0x411
#define WM5100_DAC_VOLUME_LIMIT_1L              0x412
#define WM5100_DAC_VOLUME_LIMIT_1R              0x413
#define WM5100_OUT_VOLUME_2L                    0x414
#define WM5100_OUT_VOLUME_2R                    0x415
#define WM5100_DAC_VOLUME_LIMIT_2L              0x416
#define WM5100_DAC_VOLUME_LIMIT_2R              0x417
#define WM5100_OUT_VOLUME_3L                    0x418
#define WM5100_OUT_VOLUME_3R                    0x419
#define WM5100_DAC_VOLUME_LIMIT_3L              0x41A
#define WM5100_DAC_VOLUME_LIMIT_3R              0x41B
#define WM5100_OUT_VOLUME_4L                    0x41C
#define WM5100_OUT_VOLUME_4R                    0x41D
#define WM5100_DAC_VOLUME_LIMIT_5L              0x41E
#define WM5100_DAC_VOLUME_LIMIT_5R              0x41F
#define WM5100_DAC_VOLUME_LIMIT_6L              0x420
#define WM5100_DAC_VOLUME_LIMIT_6R              0x421
#define WM5100_DAC_AEC_CONTROL_1                0x440
#define WM5100_OUTPUT_VOLUME_RAMP               0x441
#define WM5100_DAC_DIGITAL_VOLUME_1L            0x480
#define WM5100_DAC_DIGITAL_VOLUME_1R            0x481
#define WM5100_DAC_DIGITAL_VOLUME_2L            0x482
#define WM5100_DAC_DIGITAL_VOLUME_2R            0x483
#define WM5100_DAC_DIGITAL_VOLUME_3L            0x484
#define WM5100_DAC_DIGITAL_VOLUME_3R            0x485
#define WM5100_DAC_DIGITAL_VOLUME_4L            0x486
#define WM5100_DAC_DIGITAL_VOLUME_4R            0x487
#define WM5100_DAC_DIGITAL_VOLUME_5L            0x488
#define WM5100_DAC_DIGITAL_VOLUME_5R            0x489
#define WM5100_DAC_DIGITAL_VOLUME_6L            0x48A
#define WM5100_DAC_DIGITAL_VOLUME_6R            0x48B
#define WM5100_PDM_SPK1_CTRL_1                  0x4C0
#define WM5100_PDM_SPK1_CTRL_2                  0x4C1
#define WM5100_PDM_SPK2_CTRL_1                  0x4C2
#define WM5100_PDM_SPK2_CTRL_2                  0x4C3
#define WM5100_AUDIO_IF_1_1                     0x500
#define WM5100_AUDIO_IF_1_2                     0x501
#define WM5100_AUDIO_IF_1_3                     0x502
#define WM5100_AUDIO_IF_1_4                     0x503
#define WM5100_AUDIO_IF_1_5                     0x504
#define WM5100_AUDIO_IF_1_6                     0x505
#define WM5100_AUDIO_IF_1_7                     0x506
#define WM5100_AUDIO_IF_1_8                     0x507
#define WM5100_AUDIO_IF_1_9                     0x508
#define WM5100_AUDIO_IF_1_10                    0x509
#define WM5100_AUDIO_IF_1_11                    0x50A
#define WM5100_AUDIO_IF_1_12                    0x50B
#define WM5100_AUDIO_IF_1_13                    0x50C
#define WM5100_AUDIO_IF_1_14                    0x50D
#define WM5100_AUDIO_IF_1_15                    0x50E
#define WM5100_AUDIO_IF_1_16                    0x50F
#define WM5100_AUDIO_IF_1_17                    0x510
#define WM5100_AUDIO_IF_1_18                    0x511
#define WM5100_AUDIO_IF_1_19                    0x512
#define WM5100_AUDIO_IF_1_20                    0x513
#define WM5100_AUDIO_IF_1_21                    0x514
#define WM5100_AUDIO_IF_1_22                    0x515
#define WM5100_AUDIO_IF_1_23                    0x516
#define WM5100_AUDIO_IF_1_24                    0x517
#define WM5100_AUDIO_IF_1_25                    0x518
#define WM5100_AUDIO_IF_1_26                    0x519
#define WM5100_AUDIO_IF_1_27                    0x51A
#define WM5100_AUDIO_IF_2_1                     0x540
#define WM5100_AUDIO_IF_2_2                     0x541
#define WM5100_AUDIO_IF_2_3                     0x542
#define WM5100_AUDIO_IF_2_4                     0x543
#define WM5100_AUDIO_IF_2_5                     0x544
#define WM5100_AUDIO_IF_2_6                     0x545
#define WM5100_AUDIO_IF_2_7                     0x546
#define WM5100_AUDIO_IF_2_8                     0x547
#define WM5100_AUDIO_IF_2_9                     0x548
#define WM5100_AUDIO_IF_2_10                    0x549
#define WM5100_AUDIO_IF_2_11                    0x54A
#define WM5100_AUDIO_IF_2_18                    0x551
#define WM5100_AUDIO_IF_2_19                    0x552
#define WM5100_AUDIO_IF_2_26                    0x559
#define WM5100_AUDIO_IF_2_27                    0x55A
#define WM5100_AUDIO_IF_3_1                     0x580
#define WM5100_AUDIO_IF_3_2                     0x581
#define WM5100_AUDIO_IF_3_3                     0x582
#define WM5100_AUDIO_IF_3_4                     0x583
#define WM5100_AUDIO_IF_3_5                     0x584
#define WM5100_AUDIO_IF_3_6                     0x585
#define WM5100_AUDIO_IF_3_7                     0x586
#define WM5100_AUDIO_IF_3_8                     0x587
#define WM5100_AUDIO_IF_3_9                     0x588
#define WM5100_AUDIO_IF_3_10                    0x589
#define WM5100_AUDIO_IF_3_11                    0x58A
#define WM5100_AUDIO_IF_3_18                    0x591
#define WM5100_AUDIO_IF_3_19                    0x592
#define WM5100_AUDIO_IF_3_26                    0x599
#define WM5100_AUDIO_IF_3_27                    0x59A
#define WM5100_PWM1MIX_INPUT_1_SOURCE           0x640
#define WM5100_PWM1MIX_INPUT_1_VOLUME           0x641
#define WM5100_PWM1MIX_INPUT_2_SOURCE           0x642
#define WM5100_PWM1MIX_INPUT_2_VOLUME           0x643
#define WM5100_PWM1MIX_INPUT_3_SOURCE           0x644
#define WM5100_PWM1MIX_INPUT_3_VOLUME           0x645
#define WM5100_PWM1MIX_INPUT_4_SOURCE           0x646
#define WM5100_PWM1MIX_INPUT_4_VOLUME           0x647
#define WM5100_PWM2MIX_INPUT_1_SOURCE           0x648
#define WM5100_PWM2MIX_INPUT_1_VOLUME           0x649
#define WM5100_PWM2MIX_INPUT_2_SOURCE           0x64A
#define WM5100_PWM2MIX_INPUT_2_VOLUME           0x64B
#define WM5100_PWM2MIX_INPUT_3_SOURCE           0x64C
#define WM5100_PWM2MIX_INPUT_3_VOLUME           0x64D
#define WM5100_PWM2MIX_INPUT_4_SOURCE           0x64E
#define WM5100_PWM2MIX_INPUT_4_VOLUME           0x64F
#define WM5100_OUT1LMIX_INPUT_1_SOURCE          0x680
#define WM5100_OUT1LMIX_INPUT_1_VOLUME          0x681
#define WM5100_OUT1LMIX_INPUT_2_SOURCE          0x682
#define WM5100_OUT1LMIX_INPUT_2_VOLUME          0x683
#define WM5100_OUT1LMIX_INPUT_3_SOURCE          0x684
#define WM5100_OUT1LMIX_INPUT_3_VOLUME          0x685
#define WM5100_OUT1LMIX_INPUT_4_SOURCE          0x686
#define WM5100_OUT1LMIX_INPUT_4_VOLUME          0x687
#define WM5100_OUT1RMIX_INPUT_1_SOURCE          0x688
#define WM5100_OUT1RMIX_INPUT_1_VOLUME          0x689
#define WM5100_OUT1RMIX_INPUT_2_SOURCE          0x68A
#define WM5100_OUT1RMIX_INPUT_2_VOLUME          0x68B
#define WM5100_OUT1RMIX_INPUT_3_SOURCE          0x68C
#define WM5100_OUT1RMIX_INPUT_3_VOLUME          0x68D
#define WM5100_OUT1RMIX_INPUT_4_SOURCE          0x68E
#define WM5100_OUT1RMIX_INPUT_4_VOLUME          0x68F
#define WM5100_OUT2LMIX_INPUT_1_SOURCE          0x690
#define WM5100_OUT2LMIX_INPUT_1_VOLUME          0x691
#define WM5100_OUT2LMIX_INPUT_2_SOURCE          0x692
#define WM5100_OUT2LMIX_INPUT_2_VOLUME          0x693
#define WM5100_OUT2LMIX_INPUT_3_SOURCE          0x694
#define WM5100_OUT2LMIX_INPUT_3_VOLUME          0x695
#define WM5100_OUT2LMIX_INPUT_4_SOURCE          0x696
#define WM5100_OUT2LMIX_INPUT_4_VOLUME          0x697
#define WM5100_OUT2RMIX_INPUT_1_SOURCE          0x698
#define WM5100_OUT2RMIX_INPUT_1_VOLUME          0x699
#define WM5100_OUT2RMIX_INPUT_2_SOURCE          0x69A
#define WM5100_OUT2RMIX_INPUT_2_VOLUME          0x69B
#define WM5100_OUT2RMIX_INPUT_3_SOURCE          0x69C
#define WM5100_OUT2RMIX_INPUT_3_VOLUME          0x69D
#define WM5100_OUT2RMIX_INPUT_4_SOURCE          0x69E
#define WM5100_OUT2RMIX_INPUT_4_VOLUME          0x69F
#define WM5100_OUT3LMIX_INPUT_1_SOURCE          0x6A0
#define WM5100_OUT3LMIX_INPUT_1_VOLUME          0x6A1
#define WM5100_OUT3LMIX_INPUT_2_SOURCE          0x6A2
#define WM5100_OUT3LMIX_INPUT_2_VOLUME          0x6A3
#define WM5100_OUT3LMIX_INPUT_3_SOURCE          0x6A4
#define WM5100_OUT3LMIX_INPUT_3_VOLUME          0x6A5
#define WM5100_OUT3LMIX_INPUT_4_SOURCE          0x6A6
#define WM5100_OUT3LMIX_INPUT_4_VOLUME          0x6A7
#define WM5100_OUT3RMIX_INPUT_1_SOURCE          0x6A8
#define WM5100_OUT3RMIX_INPUT_1_VOLUME          0x6A9
#define WM5100_OUT3RMIX_INPUT_2_SOURCE          0x6AA
#define WM5100_OUT3RMIX_INPUT_2_VOLUME          0x6AB
#define WM5100_OUT3RMIX_INPUT_3_SOURCE          0x6AC
#define WM5100_OUT3RMIX_INPUT_3_VOLUME          0x6AD
#define WM5100_OUT3RMIX_INPUT_4_SOURCE          0x6AE
#define WM5100_OUT3RMIX_INPUT_4_VOLUME          0x6AF
#define WM5100_OUT4LMIX_INPUT_1_SOURCE          0x6B0
#define WM5100_OUT4LMIX_INPUT_1_VOLUME          0x6B1
#define WM5100_OUT4LMIX_INPUT_2_SOURCE          0x6B2
#define WM5100_OUT4LMIX_INPUT_2_VOLUME          0x6B3
#define WM5100_OUT4LMIX_INPUT_3_SOURCE          0x6B4
#define WM5100_OUT4LMIX_INPUT_3_VOLUME          0x6B5
#define WM5100_OUT4LMIX_INPUT_4_SOURCE          0x6B6
#define WM5100_OUT4LMIX_INPUT_4_VOLUME          0x6B7
#define WM5100_OUT4RMIX_INPUT_1_SOURCE          0x6B8
#define WM5100_OUT4RMIX_INPUT_1_VOLUME          0x6B9
#define WM5100_OUT4RMIX_INPUT_2_SOURCE          0x6BA
#define WM5100_OUT4RMIX_INPUT_2_VOLUME          0x6BB
#define WM5100_OUT4RMIX_INPUT_3_SOURCE          0x6BC
#define WM5100_OUT4RMIX_INPUT_3_VOLUME          0x6BD
#define WM5100_OUT4RMIX_INPUT_4_SOURCE          0x6BE
#define WM5100_OUT4RMIX_INPUT_4_VOLUME          0x6BF
#define WM5100_OUT5LMIX_INPUT_1_SOURCE          0x6C0
#define WM5100_OUT5LMIX_INPUT_1_VOLUME          0x6C1
#define WM5100_OUT5LMIX_INPUT_2_SOURCE          0x6C2
#define WM5100_OUT5LMIX_INPUT_2_VOLUME          0x6C3
#define WM5100_OUT5LMIX_INPUT_3_SOURCE          0x6C4
#define WM5100_OUT5LMIX_INPUT_3_VOLUME          0x6C5
#define WM5100_OUT5LMIX_INPUT_4_SOURCE          0x6C6
#define WM5100_OUT5LMIX_INPUT_4_VOLUME          0x6C7
#define WM5100_OUT5RMIX_INPUT_1_SOURCE          0x6C8
#define WM5100_OUT5RMIX_INPUT_1_VOLUME          0x6C9
#define WM5100_OUT5RMIX_INPUT_2_SOURCE          0x6CA
#define WM5100_OUT5RMIX_INPUT_2_VOLUME          0x6CB
#define WM5100_OUT5RMIX_INPUT_3_SOURCE          0x6CC
#define WM5100_OUT5RMIX_INPUT_3_VOLUME          0x6CD
#define WM5100_OUT5RMIX_INPUT_4_SOURCE          0x6CE
#define WM5100_OUT5RMIX_INPUT_4_VOLUME          0x6CF
#define WM5100_OUT6LMIX_INPUT_1_SOURCE          0x6D0
#define WM5100_OUT6LMIX_INPUT_1_VOLUME          0x6D1
#define WM5100_OUT6LMIX_INPUT_2_SOURCE          0x6D2
#define WM5100_OUT6LMIX_INPUT_2_VOLUME          0x6D3
#define WM5100_OUT6LMIX_INPUT_3_SOURCE          0x6D4
#define WM5100_OUT6LMIX_INPUT_3_VOLUME          0x6D5
#define WM5100_OUT6LMIX_INPUT_4_SOURCE          0x6D6
#define WM5100_OUT6LMIX_INPUT_4_VOLUME          0x6D7
#define WM5100_OUT6RMIX_INPUT_1_SOURCE          0x6D8
#define WM5100_OUT6RMIX_INPUT_1_VOLUME          0x6D9
#define WM5100_OUT6RMIX_INPUT_2_SOURCE          0x6DA
#define WM5100_OUT6RMIX_INPUT_2_VOLUME          0x6DB
#define WM5100_OUT6RMIX_INPUT_3_SOURCE          0x6DC
#define WM5100_OUT6RMIX_INPUT_3_VOLUME          0x6DD
#define WM5100_OUT6RMIX_INPUT_4_SOURCE          0x6DE
#define WM5100_OUT6RMIX_INPUT_4_VOLUME          0x6DF
#define WM5100_AIF1TX1MIX_INPUT_1_SOURCE        0x700
#define WM5100_AIF1TX1MIX_INPUT_1_VOLUME        0x701
#define WM5100_AIF1TX1MIX_INPUT_2_SOURCE        0x702
#define WM5100_AIF1TX1MIX_INPUT_2_VOLUME        0x703
#define WM5100_AIF1TX1MIX_INPUT_3_SOURCE        0x704
#define WM5100_AIF1TX1MIX_INPUT_3_VOLUME        0x705
#define WM5100_AIF1TX1MIX_INPUT_4_SOURCE        0x706
#define WM5100_AIF1TX1MIX_INPUT_4_VOLUME        0x707
#define WM5100_AIF1TX2MIX_INPUT_1_SOURCE        0x708
#define WM5100_AIF1TX2MIX_INPUT_1_VOLUME        0x709
#define WM5100_AIF1TX2MIX_INPUT_2_SOURCE        0x70A
#define WM5100_AIF1TX2MIX_INPUT_2_VOLUME        0x70B
#define WM5100_AIF1TX2MIX_INPUT_3_SOURCE        0x70C
#define WM5100_AIF1TX2MIX_INPUT_3_VOLUME        0x70D
#define WM5100_AIF1TX2MIX_INPUT_4_SOURCE        0x70E
#define WM5100_AIF1TX2MIX_INPUT_4_VOLUME        0x70F
#define WM5100_AIF1TX3MIX_INPUT_1_SOURCE        0x710
#define WM5100_AIF1TX3MIX_INPUT_1_VOLUME        0x711
#define WM5100_AIF1TX3MIX_INPUT_2_SOURCE        0x712
#define WM5100_AIF1TX3MIX_INPUT_2_VOLUME        0x713
#define WM5100_AIF1TX3MIX_INPUT_3_SOURCE        0x714
#define WM5100_AIF1TX3MIX_INPUT_3_VOLUME        0x715
#define WM5100_AIF1TX3MIX_INPUT_4_SOURCE        0x716
#define WM5100_AIF1TX3MIX_INPUT_4_VOLUME        0x717
#define WM5100_AIF1TX4MIX_INPUT_1_SOURCE        0x718
#define WM5100_AIF1TX4MIX_INPUT_1_VOLUME        0x719
#define WM5100_AIF1TX4MIX_INPUT_2_SOURCE        0x71A
#define WM5100_AIF1TX4MIX_INPUT_2_VOLUME        0x71B
#define WM5100_AIF1TX4MIX_INPUT_3_SOURCE        0x71C
#define WM5100_AIF1TX4MIX_INPUT_3_VOLUME        0x71D
#define WM5100_AIF1TX4MIX_INPUT_4_SOURCE        0x71E
#define WM5100_AIF1TX4MIX_INPUT_4_VOLUME        0x71F
#define WM5100_AIF1TX5MIX_INPUT_1_SOURCE        0x720
#define WM5100_AIF1TX5MIX_INPUT_1_VOLUME        0x721
#define WM5100_AIF1TX5MIX_INPUT_2_SOURCE        0x722
#define WM5100_AIF1TX5MIX_INPUT_2_VOLUME        0x723
#define WM5100_AIF1TX5MIX_INPUT_3_SOURCE        0x724
#define WM5100_AIF1TX5MIX_INPUT_3_VOLUME        0x725
#define WM5100_AIF1TX5MIX_INPUT_4_SOURCE        0x726
#define WM5100_AIF1TX5MIX_INPUT_4_VOLUME        0x727
#define WM5100_AIF1TX6MIX_INPUT_1_SOURCE        0x728
#define WM5100_AIF1TX6MIX_INPUT_1_VOLUME        0x729
#define WM5100_AIF1TX6MIX_INPUT_2_SOURCE        0x72A
#define WM5100_AIF1TX6MIX_INPUT_2_VOLUME        0x72B
#define WM5100_AIF1TX6MIX_INPUT_3_SOURCE        0x72C
#define WM5100_AIF1TX6MIX_INPUT_3_VOLUME        0x72D
#define WM5100_AIF1TX6MIX_INPUT_4_SOURCE        0x72E
#define WM5100_AIF1TX6MIX_INPUT_4_VOLUME        0x72F
#define WM5100_AIF1TX7MIX_INPUT_1_SOURCE        0x730
#define WM5100_AIF1TX7MIX_INPUT_1_VOLUME        0x731
#define WM5100_AIF1TX7MIX_INPUT_2_SOURCE        0x732
#define WM5100_AIF1TX7MIX_INPUT_2_VOLUME        0x733
#define WM5100_AIF1TX7MIX_INPUT_3_SOURCE        0x734
#define WM5100_AIF1TX7MIX_INPUT_3_VOLUME        0x735
#define WM5100_AIF1TX7MIX_INPUT_4_SOURCE        0x736
#define WM5100_AIF1TX7MIX_INPUT_4_VOLUME        0x737
#define WM5100_AIF1TX8MIX_INPUT_1_SOURCE        0x738
#define WM5100_AIF1TX8MIX_INPUT_1_VOLUME        0x739
#define WM5100_AIF1TX8MIX_INPUT_2_SOURCE        0x73A
#define WM5100_AIF1TX8MIX_INPUT_2_VOLUME        0x73B
#define WM5100_AIF1TX8MIX_INPUT_3_SOURCE        0x73C
#define WM5100_AIF1TX8MIX_INPUT_3_VOLUME        0x73D
#define WM5100_AIF1TX8MIX_INPUT_4_SOURCE        0x73E
#define WM5100_AIF1TX8MIX_INPUT_4_VOLUME        0x73F
#define WM5100_AIF2TX1MIX_INPUT_1_SOURCE        0x740
#define WM5100_AIF2TX1MIX_INPUT_1_VOLUME        0x741
#define WM5100_AIF2TX1MIX_INPUT_2_SOURCE        0x742
#define WM5100_AIF2TX1MIX_INPUT_2_VOLUME        0x743
#define WM5100_AIF2TX1MIX_INPUT_3_SOURCE        0x744
#define WM5100_AIF2TX1MIX_INPUT_3_VOLUME        0x745
#define WM5100_AIF2TX1MIX_INPUT_4_SOURCE        0x746
#define WM5100_AIF2TX1MIX_INPUT_4_VOLUME        0x747
#define WM5100_AIF2TX2MIX_INPUT_1_SOURCE        0x748
#define WM5100_AIF2TX2MIX_INPUT_1_VOLUME        0x749
#define WM5100_AIF2TX2MIX_INPUT_2_SOURCE        0x74A
#define WM5100_AIF2TX2MIX_INPUT_2_VOLUME        0x74B
#define WM5100_AIF2TX2MIX_INPUT_3_SOURCE        0x74C
#define WM5100_AIF2TX2MIX_INPUT_3_VOLUME        0x74D
#define WM5100_AIF2TX2MIX_INPUT_4_SOURCE        0x74E
#define WM5100_AIF2TX2MIX_INPUT_4_VOLUME        0x74F
#define WM5100_AIF3TX1MIX_INPUT_1_SOURCE        0x780
#define WM5100_AIF3TX1MIX_INPUT_1_VOLUME        0x781
#define WM5100_AIF3TX1MIX_INPUT_2_SOURCE        0x782
#define WM5100_AIF3TX1MIX_INPUT_2_VOLUME        0x783
#define WM5100_AIF3TX1MIX_INPUT_3_SOURCE        0x784
#define WM5100_AIF3TX1MIX_INPUT_3_VOLUME        0x785
#define WM5100_AIF3TX1MIX_INPUT_4_SOURCE        0x786
#define WM5100_AIF3TX1MIX_INPUT_4_VOLUME        0x787
#define WM5100_AIF3TX2MIX_INPUT_1_SOURCE        0x788
#define WM5100_AIF3TX2MIX_INPUT_1_VOLUME        0x789
#define WM5100_AIF3TX2MIX_INPUT_2_SOURCE        0x78A
#define WM5100_AIF3TX2MIX_INPUT_2_VOLUME        0x78B
#define WM5100_AIF3TX2MIX_INPUT_3_SOURCE        0x78C
#define WM5100_AIF3TX2MIX_INPUT_3_VOLUME        0x78D
#define WM5100_AIF3TX2MIX_INPUT_4_SOURCE        0x78E
#define WM5100_AIF3TX2MIX_INPUT_4_VOLUME        0x78F
#define WM5100_EQ1MIX_INPUT_1_SOURCE            0x880
#define WM5100_EQ1MIX_INPUT_1_VOLUME            0x881
#define WM5100_EQ1MIX_INPUT_2_SOURCE            0x882
#define WM5100_EQ1MIX_INPUT_2_VOLUME            0x883
#define WM5100_EQ1MIX_INPUT_3_SOURCE            0x884
#define WM5100_EQ1MIX_INPUT_3_VOLUME            0x885
#define WM5100_EQ1MIX_INPUT_4_SOURCE            0x886
#define WM5100_EQ1MIX_INPUT_4_VOLUME            0x887
#define WM5100_EQ2MIX_INPUT_1_SOURCE            0x888
#define WM5100_EQ2MIX_INPUT_1_VOLUME            0x889
#define WM5100_EQ2MIX_INPUT_2_SOURCE            0x88A
#define WM5100_EQ2MIX_INPUT_2_VOLUME            0x88B
#define WM5100_EQ2MIX_INPUT_3_SOURCE            0x88C
#define WM5100_EQ2MIX_INPUT_3_VOLUME            0x88D
#define WM5100_EQ2MIX_INPUT_4_SOURCE            0x88E
#define WM5100_EQ2MIX_INPUT_4_VOLUME            0x88F
#define WM5100_EQ3MIX_INPUT_1_SOURCE            0x890
#define WM5100_EQ3MIX_INPUT_1_VOLUME            0x891
#define WM5100_EQ3MIX_INPUT_2_SOURCE            0x892
#define WM5100_EQ3MIX_INPUT_2_VOLUME            0x893
#define WM5100_EQ3MIX_INPUT_3_SOURCE            0x894
#define WM5100_EQ3MIX_INPUT_3_VOLUME            0x895
#define WM5100_EQ3MIX_INPUT_4_SOURCE            0x896
#define WM5100_EQ3MIX_INPUT_4_VOLUME            0x897
#define WM5100_EQ4MIX_INPUT_1_SOURCE            0x898
#define WM5100_EQ4MIX_INPUT_1_VOLUME            0x899
#define WM5100_EQ4MIX_INPUT_2_SOURCE            0x89A
#define WM5100_EQ4MIX_INPUT_2_VOLUME            0x89B
#define WM5100_EQ4MIX_INPUT_3_SOURCE            0x89C
#define WM5100_EQ4MIX_INPUT_3_VOLUME            0x89D
#define WM5100_EQ4MIX_INPUT_4_SOURCE            0x89E
#define WM5100_EQ4MIX_INPUT_4_VOLUME            0x89F
#define WM5100_DRC1LMIX_INPUT_1_SOURCE          0x8C0
#define WM5100_DRC1LMIX_INPUT_1_VOLUME          0x8C1
#define WM5100_DRC1LMIX_INPUT_2_SOURCE          0x8C2
#define WM5100_DRC1LMIX_INPUT_2_VOLUME          0x8C3
#define WM5100_DRC1LMIX_INPUT_3_SOURCE          0x8C4
#define WM5100_DRC1LMIX_INPUT_3_VOLUME          0x8C5
#define WM5100_DRC1LMIX_INPUT_4_SOURCE          0x8C6
#define WM5100_DRC1LMIX_INPUT_4_VOLUME          0x8C7
#define WM5100_DRC1RMIX_INPUT_1_SOURCE          0x8C8
#define WM5100_DRC1RMIX_INPUT_1_VOLUME          0x8C9
#define WM5100_DRC1RMIX_INPUT_2_SOURCE          0x8CA
#define WM5100_DRC1RMIX_INPUT_2_VOLUME          0x8CB
#define WM5100_DRC1RMIX_INPUT_3_SOURCE          0x8CC
#define WM5100_DRC1RMIX_INPUT_3_VOLUME          0x8CD
#define WM5100_DRC1RMIX_INPUT_4_SOURCE          0x8CE
#define WM5100_DRC1RMIX_INPUT_4_VOLUME          0x8CF
#define WM5100_HPLP1MIX_INPUT_1_SOURCE          0x900
#define WM5100_HPLP1MIX_INPUT_1_VOLUME          0x901
#define WM5100_HPLP1MIX_INPUT_2_SOURCE          0x902
#define WM5100_HPLP1MIX_INPUT_2_VOLUME          0x903
#define WM5100_HPLP1MIX_INPUT_3_SOURCE          0x904
#define WM5100_HPLP1MIX_INPUT_3_VOLUME          0x905
#define WM5100_HPLP1MIX_INPUT_4_SOURCE          0x906
#define WM5100_HPLP1MIX_INPUT_4_VOLUME          0x907
#define WM5100_HPLP2MIX_INPUT_1_SOURCE          0x908
#define WM5100_HPLP2MIX_INPUT_1_VOLUME          0x909
#define WM5100_HPLP2MIX_INPUT_2_SOURCE          0x90A
#define WM5100_HPLP2MIX_INPUT_2_VOLUME          0x90B
#define WM5100_HPLP2MIX_INPUT_3_SOURCE          0x90C
#define WM5100_HPLP2MIX_INPUT_3_VOLUME          0x90D
#define WM5100_HPLP2MIX_INPUT_4_SOURCE          0x90E
#define WM5100_HPLP2MIX_INPUT_4_VOLUME          0x90F
#define WM5100_HPLP3MIX_INPUT_1_SOURCE          0x910
#define WM5100_HPLP3MIX_INPUT_1_VOLUME          0x911
#define WM5100_HPLP3MIX_INPUT_2_SOURCE          0x912
#define WM5100_HPLP3MIX_INPUT_2_VOLUME          0x913
#define WM5100_HPLP3MIX_INPUT_3_SOURCE          0x914
#define WM5100_HPLP3MIX_INPUT_3_VOLUME          0x915
#define WM5100_HPLP3MIX_INPUT_4_SOURCE          0x916
#define WM5100_HPLP3MIX_INPUT_4_VOLUME          0x917
#define WM5100_HPLP4MIX_INPUT_1_SOURCE          0x918
#define WM5100_HPLP4MIX_INPUT_1_VOLUME          0x919
#define WM5100_HPLP4MIX_INPUT_2_SOURCE          0x91A
#define WM5100_HPLP4MIX_INPUT_2_VOLUME          0x91B
#define WM5100_HPLP4MIX_INPUT_3_SOURCE          0x91C
#define WM5100_HPLP4MIX_INPUT_3_VOLUME          0x91D
#define WM5100_HPLP4MIX_INPUT_4_SOURCE          0x91E
#define WM5100_HPLP4MIX_INPUT_4_VOLUME          0x91F
#define WM5100_DSP1LMIX_INPUT_1_SOURCE          0x940
#define WM5100_DSP1LMIX_INPUT_1_VOLUME          0x941
#define WM5100_DSP1LMIX_INPUT_2_SOURCE          0x942
#define WM5100_DSP1LMIX_INPUT_2_VOLUME          0x943
#define WM5100_DSP1LMIX_INPUT_3_SOURCE          0x944
#define WM5100_DSP1LMIX_INPUT_3_VOLUME          0x945
#define WM5100_DSP1LMIX_INPUT_4_SOURCE          0x946
#define WM5100_DSP1LMIX_INPUT_4_VOLUME          0x947
#define WM5100_DSP1RMIX_INPUT_1_SOURCE          0x948
#define WM5100_DSP1RMIX_INPUT_1_VOLUME          0x949
#define WM5100_DSP1RMIX_INPUT_2_SOURCE          0x94A
#define WM5100_DSP1RMIX_INPUT_2_VOLUME          0x94B
#define WM5100_DSP1RMIX_INPUT_3_SOURCE          0x94C
#define WM5100_DSP1RMIX_INPUT_3_VOLUME          0x94D
#define WM5100_DSP1RMIX_INPUT_4_SOURCE          0x94E
#define WM5100_DSP1RMIX_INPUT_4_VOLUME          0x94F
#define WM5100_DSP1AUX1MIX_INPUT_1_SOURCE       0x950
#define WM5100_DSP1AUX2MIX_INPUT_1_SOURCE       0x958
#define WM5100_DSP1AUX3MIX_INPUT_1_SOURCE       0x960
#define WM5100_DSP1AUX4MIX_INPUT_1_SOURCE       0x968
#define WM5100_DSP1AUX5MIX_INPUT_1_SOURCE       0x970
#define WM5100_DSP1AUX6MIX_INPUT_1_SOURCE       0x978
#define WM5100_DSP2LMIX_INPUT_1_SOURCE          0x980
#define WM5100_DSP2LMIX_INPUT_1_VOLUME          0x981
#define WM5100_DSP2LMIX_INPUT_2_SOURCE          0x982
#define WM5100_DSP2LMIX_INPUT_2_VOLUME          0x983
#define WM5100_DSP2LMIX_INPUT_3_SOURCE          0x984
#define WM5100_DSP2LMIX_INPUT_3_VOLUME          0x985
#define WM5100_DSP2LMIX_INPUT_4_SOURCE          0x986
#define WM5100_DSP2LMIX_INPUT_4_VOLUME          0x987
#define WM5100_DSP2RMIX_INPUT_1_SOURCE          0x988
#define WM5100_DSP2RMIX_INPUT_1_VOLUME          0x989
#define WM5100_DSP2RMIX_INPUT_2_SOURCE          0x98A
#define WM5100_DSP2RMIX_INPUT_2_VOLUME          0x98B
#define WM5100_DSP2RMIX_INPUT_3_SOURCE          0x98C
#define WM5100_DSP2RMIX_INPUT_3_VOLUME          0x98D
#define WM5100_DSP2RMIX_INPUT_4_SOURCE          0x98E
#define WM5100_DSP2RMIX_INPUT_4_VOLUME          0x98F
#define WM5100_DSP2AUX1MIX_INPUT_1_SOURCE       0x990
#define WM5100_DSP2AUX2MIX_INPUT_1_SOURCE       0x998
#define WM5100_DSP2AUX3MIX_INPUT_1_SOURCE       0x9A0
#define WM5100_DSP2AUX4MIX_INPUT_1_SOURCE       0x9A8
#define WM5100_DSP2AUX5MIX_INPUT_1_SOURCE       0x9B0
#define WM5100_DSP2AUX6MIX_INPUT_1_SOURCE       0x9B8
#define WM5100_DSP3LMIX_INPUT_1_SOURCE          0x9C0
#define WM5100_DSP3LMIX_INPUT_1_VOLUME          0x9C1
#define WM5100_DSP3LMIX_INPUT_2_SOURCE          0x9C2
#define WM5100_DSP3LMIX_INPUT_2_VOLUME          0x9C3
#define WM5100_DSP3LMIX_INPUT_3_SOURCE          0x9C4
#define WM5100_DSP3LMIX_INPUT_3_VOLUME          0x9C5
#define WM5100_DSP3LMIX_INPUT_4_SOURCE          0x9C6
#define WM5100_DSP3LMIX_INPUT_4_VOLUME          0x9C7
#define WM5100_DSP3RMIX_INPUT_1_SOURCE          0x9C8
#define WM5100_DSP3RMIX_INPUT_1_VOLUME          0x9C9
#define WM5100_DSP3RMIX_INPUT_2_SOURCE          0x9CA
#define WM5100_DSP3RMIX_INPUT_2_VOLUME          0x9CB
#define WM5100_DSP3RMIX_INPUT_3_SOURCE          0x9CC
#define WM5100_DSP3RMIX_INPUT_3_VOLUME          0x9CD
#define WM5100_DSP3RMIX_INPUT_4_SOURCE          0x9CE
#define WM5100_DSP3RMIX_INPUT_4_VOLUME          0x9CF
#define WM5100_DSP3AUX1MIX_INPUT_1_SOURCE       0x9D0
#define WM5100_DSP3AUX2MIX_INPUT_1_SOURCE       0x9D8
#define WM5100_DSP3AUX3MIX_INPUT_1_SOURCE       0x9E0
#define WM5100_DSP3AUX4MIX_INPUT_1_SOURCE       0x9E8
#define WM5100_DSP3AUX5MIX_INPUT_1_SOURCE       0x9F0
#define WM5100_DSP3AUX6MIX_INPUT_1_SOURCE       0x9F8
#define WM5100_ASRC1LMIX_INPUT_1_SOURCE         0xA80
#define WM5100_ASRC1RMIX_INPUT_1_SOURCE         0xA88
#define WM5100_ASRC2LMIX_INPUT_1_SOURCE         0xA90
#define WM5100_ASRC2RMIX_INPUT_1_SOURCE         0xA98
#define WM5100_ISRC1DEC1MIX_INPUT_1_SOURCE      0xB00
#define WM5100_ISRC1DEC2MIX_INPUT_1_SOURCE      0xB08
#define WM5100_ISRC1DEC3MIX_INPUT_1_SOURCE      0xB10
#define WM5100_ISRC1DEC4MIX_INPUT_1_SOURCE      0xB18
#define WM5100_ISRC1INT1MIX_INPUT_1_SOURCE      0xB20
#define WM5100_ISRC1INT2MIX_INPUT_1_SOURCE      0xB28
#define WM5100_ISRC1INT3MIX_INPUT_1_SOURCE      0xB30
#define WM5100_ISRC1INT4MIX_INPUT_1_SOURCE      0xB38
#define WM5100_ISRC2DEC1MIX_INPUT_1_SOURCE      0xB40
#define WM5100_ISRC2DEC2MIX_INPUT_1_SOURCE      0xB48
#define WM5100_ISRC2DEC3MIX_INPUT_1_SOURCE      0xB50
#define WM5100_ISRC2DEC4MIX_INPUT_1_SOURCE      0xB58
#define WM5100_ISRC2INT1MIX_INPUT_1_SOURCE      0xB60
#define WM5100_ISRC2INT2MIX_INPUT_1_SOURCE      0xB68
#define WM5100_ISRC2INT3MIX_INPUT_1_SOURCE      0xB70
#define WM5100_ISRC2INT4MIX_INPUT_1_SOURCE      0xB78
#define WM5100_GPIO_CTRL_1                      0xC00
#define WM5100_GPIO_CTRL_2                      0xC01
#define WM5100_GPIO_CTRL_3                      0xC02
#define WM5100_GPIO_CTRL_4                      0xC03
#define WM5100_GPIO_CTRL_5                      0xC04
#define WM5100_GPIO_CTRL_6                      0xC05
#define WM5100_MISC_PAD_CTRL_1                  0xC23
#define WM5100_MISC_PAD_CTRL_2                  0xC24
#define WM5100_MISC_PAD_CTRL_3                  0xC25
#define WM5100_MISC_PAD_CTRL_4                  0xC26
#define WM5100_MISC_PAD_CTRL_5                  0xC27
#define WM5100_MISC_GPIO_1                      0xC28
#define WM5100_INTERRUPT_STATUS_1               0xD00
#define WM5100_INTERRUPT_STATUS_2               0xD01
#define WM5100_INTERRUPT_STATUS_3               0xD02
#define WM5100_INTERRUPT_STATUS_4               0xD03
#define WM5100_INTERRUPT_RAW_STATUS_2           0xD04
#define WM5100_INTERRUPT_RAW_STATUS_3           0xD05
#define WM5100_INTERRUPT_RAW_STATUS_4           0xD06
#define WM5100_INTERRUPT_STATUS_1_MASK          0xD07
#define WM5100_INTERRUPT_STATUS_2_MASK          0xD08
#define WM5100_INTERRUPT_STATUS_3_MASK          0xD09
#define WM5100_INTERRUPT_STATUS_4_MASK          0xD0A
#define WM5100_INTERRUPT_CONTROL                0xD1F
#define WM5100_IRQ_DEBOUNCE_1                   0xD20
#define WM5100_IRQ_DEBOUNCE_2                   0xD21
#define WM5100_FX_CTRL                          0xE00
#define WM5100_EQ1_1                            0xE10
#define WM5100_EQ1_2                            0xE11
#define WM5100_EQ1_3                            0xE12
#define WM5100_EQ1_4                            0xE13
#define WM5100_EQ1_5                            0xE14
#define WM5100_EQ1_6                            0xE15
#define WM5100_EQ1_7                            0xE16
#define WM5100_EQ1_8                            0xE17
#define WM5100_EQ1_9                            0xE18
#define WM5100_EQ1_10                           0xE19
#define WM5100_EQ1_11                           0xE1A
#define WM5100_EQ1_12                           0xE1B
#define WM5100_EQ1_13                           0xE1C
#define WM5100_EQ1_14                           0xE1D
#define WM5100_EQ1_15                           0xE1E
#define WM5100_EQ1_16                           0xE1F
#define WM5100_EQ1_17                           0xE20
#define WM5100_EQ1_18                           0xE21
#define WM5100_EQ1_19                           0xE22
#define WM5100_EQ1_20                           0xE23
#define WM5100_EQ2_1                            0xE26
#define WM5100_EQ2_2                            0xE27
#define WM5100_EQ2_3                            0xE28
#define WM5100_EQ2_4                            0xE29
#define WM5100_EQ2_5                            0xE2A
#define WM5100_EQ2_6                            0xE2B
#define WM5100_EQ2_7                            0xE2C
#define WM5100_EQ2_8                            0xE2D
#define WM5100_EQ2_9                            0xE2E
#define WM5100_EQ2_10                           0xE2F
#define WM5100_EQ2_11                           0xE30
#define WM5100_EQ2_12                           0xE31
#define WM5100_EQ2_13                           0xE32
#define WM5100_EQ2_14                           0xE33
#define WM5100_EQ2_15                           0xE34
#define WM5100_EQ2_16                           0xE35
#define WM5100_EQ2_17                           0xE36
#define WM5100_EQ2_18                           0xE37
#define WM5100_EQ2_19                           0xE38
#define WM5100_EQ2_20                           0xE39
#define WM5100_EQ3_1                            0xE3C
#define WM5100_EQ3_2                            0xE3D
#define WM5100_EQ3_3                            0xE3E
#define WM5100_EQ3_4                            0xE3F
#define WM5100_EQ3_5                            0xE40
#define WM5100_EQ3_6                            0xE41
#define WM5100_EQ3_7                            0xE42
#define WM5100_EQ3_8                            0xE43
#define WM5100_EQ3_9                            0xE44
#define WM5100_EQ3_10                           0xE45
#define WM5100_EQ3_11                           0xE46
#define WM5100_EQ3_12                           0xE47
#define WM5100_EQ3_13                           0xE48
#define WM5100_EQ3_14                           0xE49
#define WM5100_EQ3_15                           0xE4A
#define WM5100_EQ3_16                           0xE4B
#define WM5100_EQ3_17                           0xE4C
#define WM5100_EQ3_18                           0xE4D
#define WM5100_EQ3_19                           0xE4E
#define WM5100_EQ3_20                           0xE4F
#define WM5100_EQ4_1                            0xE52
#define WM5100_EQ4_2                            0xE53
#define WM5100_EQ4_3                            0xE54
#define WM5100_EQ4_4                            0xE55
#define WM5100_EQ4_5                            0xE56
#define WM5100_EQ4_6                            0xE57
#define WM5100_EQ4_7                            0xE58
#define WM5100_EQ4_8                            0xE59
#define WM5100_EQ4_9                            0xE5A
#define WM5100_EQ4_10                           0xE5B
#define WM5100_EQ4_11                           0xE5C
#define WM5100_EQ4_12                           0xE5D
#define WM5100_EQ4_13                           0xE5E
#define WM5100_EQ4_14                           0xE5F
#define WM5100_EQ4_15                           0xE60
#define WM5100_EQ4_16                           0xE61
#define WM5100_EQ4_17                           0xE62
#define WM5100_EQ4_18                           0xE63
#define WM5100_EQ4_19                           0xE64
#define WM5100_EQ4_20                           0xE65
#define WM5100_DRC1_CTRL1                       0xE80
#define WM5100_DRC1_CTRL2                       0xE81
#define WM5100_DRC1_CTRL3                       0xE82
#define WM5100_DRC1_CTRL4                       0xE83
#define WM5100_DRC1_CTRL5                       0xE84
#define WM5100_HPLPF1_1                         0xEC0
#define WM5100_HPLPF1_2                         0xEC1
#define WM5100_HPLPF2_1                         0xEC4
#define WM5100_HPLPF2_2                         0xEC5
#define WM5100_HPLPF3_1                         0xEC8
#define WM5100_HPLPF3_2                         0xEC9
#define WM5100_HPLPF4_1                         0xECC
#define WM5100_HPLPF4_2                         0xECD
#define WM5100_DSP1_DM_0                        0x4000
#define WM5100_DSP1_DM_1                        0x4001
#define WM5100_DSP1_DM_2                        0x4002
#define WM5100_DSP1_DM_3                        0x4003
#define WM5100_DSP1_DM_508                      0x41FC
#define WM5100_DSP1_DM_509                      0x41FD
#define WM5100_DSP1_DM_510                      0x41FE
#define WM5100_DSP1_DM_511                      0x41FF
#define WM5100_DSP1_PM_0                        0x4800
#define WM5100_DSP1_PM_1                        0x4801
#define WM5100_DSP1_PM_2                        0x4802
#define WM5100_DSP1_PM_3                        0x4803
#define WM5100_DSP1_PM_4                        0x4804
#define WM5100_DSP1_PM_5                        0x4805
#define WM5100_DSP1_PM_1530                     0x4DFA
#define WM5100_DSP1_PM_1531                     0x4DFB
#define WM5100_DSP1_PM_1532                     0x4DFC
#define WM5100_DSP1_PM_1533                     0x4DFD
#define WM5100_DSP1_PM_1534                     0x4DFE
#define WM5100_DSP1_PM_1535                     0x4DFF
#define WM5100_DSP1_ZM_0                        0x5000
#define WM5100_DSP1_ZM_1                        0x5001
#define WM5100_DSP1_ZM_2                        0x5002
#define WM5100_DSP1_ZM_3                        0x5003
#define WM5100_DSP1_ZM_2044                     0x57FC
#define WM5100_DSP1_ZM_2045                     0x57FD
#define WM5100_DSP1_ZM_2046                     0x57FE
#define WM5100_DSP1_ZM_2047                     0x57FF
#define WM5100_DSP2_DM_0                        0x6000
#define WM5100_DSP2_DM_1                        0x6001
#define WM5100_DSP2_DM_2                        0x6002
#define WM5100_DSP2_DM_3                        0x6003
#define WM5100_DSP2_DM_508                      0x61FC
#define WM5100_DSP2_DM_509                      0x61FD
#define WM5100_DSP2_DM_510                      0x61FE
#define WM5100_DSP2_DM_511                      0x61FF
#define WM5100_DSP2_PM_0                        0x6800
#define WM5100_DSP2_PM_1                        0x6801
#define WM5100_DSP2_PM_2                        0x6802
#define WM5100_DSP2_PM_3                        0x6803
#define WM5100_DSP2_PM_4                        0x6804
#define WM5100_DSP2_PM_5                        0x6805
#define WM5100_DSP2_PM_1530                     0x6DFA
#define WM5100_DSP2_PM_1531                     0x6DFB
#define WM5100_DSP2_PM_1532                     0x6DFC
#define WM5100_DSP2_PM_1533                     0x6DFD
#define WM5100_DSP2_PM_1534                     0x6DFE
#define WM5100_DSP2_PM_1535                     0x6DFF
#define WM5100_DSP2_ZM_0                        0x7000
#define WM5100_DSP2_ZM_1                        0x7001
#define WM5100_DSP2_ZM_2                        0x7002
#define WM5100_DSP2_ZM_3                        0x7003
#define WM5100_DSP2_ZM_2044                     0x77FC
#define WM5100_DSP2_ZM_2045                     0x77FD
#define WM5100_DSP2_ZM_2046                     0x77FE
#define WM5100_DSP2_ZM_2047                     0x77FF
#define WM5100_DSP3_DM_0                        0x8000
#define WM5100_DSP3_DM_1                        0x8001
#define WM5100_DSP3_DM_2                        0x8002
#define WM5100_DSP3_DM_3                        0x8003
#define WM5100_DSP3_DM_508                      0x81FC
#define WM5100_DSP3_DM_509                      0x81FD
#define WM5100_DSP3_DM_510                      0x81FE
#define WM5100_DSP3_DM_511                      0x81FF
#define WM5100_DSP3_PM_0                        0x8800
#define WM5100_DSP3_PM_1                        0x8801
#define WM5100_DSP3_PM_2                        0x8802
#define WM5100_DSP3_PM_3                        0x8803
#define WM5100_DSP3_PM_4                        0x8804
#define WM5100_DSP3_PM_5                        0x8805
#define WM5100_DSP3_PM_1530                     0x8DFA
#define WM5100_DSP3_PM_1531                     0x8DFB
#define WM5100_DSP3_PM_1532                     0x8DFC
#define WM5100_DSP3_PM_1533                     0x8DFD
#define WM5100_DSP3_PM_1534                     0x8DFE
#define WM5100_DSP3_PM_1535                     0x8DFF
#define WM5100_DSP3_ZM_0                        0x9000
#define WM5100_DSP3_ZM_1                        0x9001
#define WM5100_DSP3_ZM_2                        0x9002
#define WM5100_DSP3_ZM_3                        0x9003
#define WM5100_DSP3_ZM_2044                     0x97FC
#define WM5100_DSP3_ZM_2045                     0x97FD
#define WM5100_DSP3_ZM_2046                     0x97FE
#define WM5100_DSP3_ZM_2047                     0x97FF

#define WM5100_REGISTER_COUNT                   1435
#define WM5100_MAX_REGISTER                     0x97FF

/*
                     
 */

/*
                             
 */
#define WM5100_SW_RST_DEV_ID1_MASK              0xFFFF  /*                         */
#define WM5100_SW_RST_DEV_ID1_SHIFT                  0  /*                         */
#define WM5100_SW_RST_DEV_ID1_WIDTH                 16  /*                         */

/*
                              
 */
#define WM5100_DEVICE_REVISION_MASK             0x000F  /*                         */
#define WM5100_DEVICE_REVISION_SHIFT                 0  /*                         */
#define WM5100_DEVICE_REVISION_WIDTH                 4  /*                         */

/*
                         
 */
#define WM5100_AUTO_INC                         0x0001  /*          */
#define WM5100_AUTO_INC_MASK                    0x0001  /*          */
#define WM5100_AUTO_INC_SHIFT                        0  /*          */
#define WM5100_AUTO_INC_WIDTH                        1  /*          */

/*
                                
 */
#define WM5100_TONE_RATE_MASK                   0x3000  /*                     */
#define WM5100_TONE_RATE_SHIFT                      12  /*                     */
#define WM5100_TONE_RATE_WIDTH                       2  /*                     */
#define WM5100_TONE_OFFSET_MASK                 0x0300  /*                     */
#define WM5100_TONE_OFFSET_SHIFT                     8  /*                     */
#define WM5100_TONE_OFFSET_WIDTH                     2  /*                     */
#define WM5100_TONE2_ENA                        0x0002  /*           */
#define WM5100_TONE2_ENA_MASK                   0x0002  /*           */
#define WM5100_TONE2_ENA_SHIFT                       1  /*           */
#define WM5100_TONE2_ENA_WIDTH                       1  /*           */
#define WM5100_TONE1_ENA                        0x0001  /*           */
#define WM5100_TONE1_ENA_MASK                   0x0001  /*           */
#define WM5100_TONE1_ENA_SHIFT                       0  /*           */
#define WM5100_TONE1_ENA_WIDTH                       1  /*           */

/*
                           
 */
#define WM5100_PWM_RATE_MASK                    0x3000  /*                    */
#define WM5100_PWM_RATE_SHIFT                       12  /*                    */
#define WM5100_PWM_RATE_WIDTH                        2  /*                    */
#define WM5100_PWM_CLK_SEL_MASK                 0x0300  /*                     */
#define WM5100_PWM_CLK_SEL_SHIFT                     8  /*                     */
#define WM5100_PWM_CLK_SEL_WIDTH                     2  /*                     */
#define WM5100_PWM2_OVD                         0x0020  /*          */
#define WM5100_PWM2_OVD_MASK                    0x0020  /*          */
#define WM5100_PWM2_OVD_SHIFT                        5  /*          */
#define WM5100_PWM2_OVD_WIDTH                        1  /*          */
#define WM5100_PWM1_OVD                         0x0010  /*          */
#define WM5100_PWM1_OVD_MASK                    0x0010  /*          */
#define WM5100_PWM1_OVD_SHIFT                        4  /*          */
#define WM5100_PWM1_OVD_WIDTH                        1  /*          */
#define WM5100_PWM2_ENA                         0x0002  /*          */
#define WM5100_PWM2_ENA_MASK                    0x0002  /*          */
#define WM5100_PWM2_ENA_SHIFT                        1  /*          */
#define WM5100_PWM2_ENA_WIDTH                        1  /*          */
#define WM5100_PWM1_ENA                         0x0001  /*          */
#define WM5100_PWM1_ENA_MASK                    0x0001  /*          */
#define WM5100_PWM1_ENA_SHIFT                        0  /*          */
#define WM5100_PWM1_ENA_WIDTH                        1  /*          */

/*
                           
 */
#define WM5100_PWM1_LVL_MASK                    0x03FF  /*                  */
#define WM5100_PWM1_LVL_SHIFT                        0  /*                  */
#define WM5100_PWM1_LVL_WIDTH                       10  /*                  */

/*
                           
 */
#define WM5100_PWM2_LVL_MASK                    0x03FF  /*                  */
#define WM5100_PWM2_LVL_SHIFT                        0  /*                  */
#define WM5100_PWM2_LVL_WIDTH                       10  /*                  */

/*
                            
 */
#define WM5100_CLK_32K_SRC_MASK                 0x000F  /*                     */
#define WM5100_CLK_32K_SRC_SHIFT                     0  /*                     */
#define WM5100_CLK_32K_SRC_WIDTH                     4  /*                     */

/*
                            
 */
#define WM5100_SYSCLK_FREQ_MASK                 0x0700  /*                      */
#define WM5100_SYSCLK_FREQ_SHIFT                     8  /*                      */
#define WM5100_SYSCLK_FREQ_WIDTH                     3  /*                      */
#define WM5100_SYSCLK_ENA                       0x0040  /*            */
#define WM5100_SYSCLK_ENA_MASK                  0x0040  /*            */
#define WM5100_SYSCLK_ENA_SHIFT                      6  /*            */
#define WM5100_SYSCLK_ENA_WIDTH                      1  /*            */
#define WM5100_SYSCLK_SRC_MASK                  0x000F  /*                    */
#define WM5100_SYSCLK_SRC_SHIFT                      0  /*                    */
#define WM5100_SYSCLK_SRC_WIDTH                      4  /*                    */

/*
                            
 */
#define WM5100_SAMPLE_RATE_1_MASK               0x001F  /*                       */
#define WM5100_SAMPLE_RATE_1_SHIFT                   0  /*                       */
#define WM5100_SAMPLE_RATE_1_WIDTH                   5  /*                       */

/*
                            
 */
#define WM5100_SAMPLE_RATE_2_MASK               0x001F  /*                       */
#define WM5100_SAMPLE_RATE_2_SHIFT                   0  /*                       */
#define WM5100_SAMPLE_RATE_2_WIDTH                   5  /*                       */

/*
                            
 */
#define WM5100_SAMPLE_RATE_3_MASK               0x001F  /*                       */
#define WM5100_SAMPLE_RATE_3_SHIFT                   0  /*                       */
#define WM5100_SAMPLE_RATE_3_WIDTH                   5  /*                       */

/*
                            
 */
#define WM5100_ASYNC_CLK_FREQ_MASK              0x0700  /*                         */
#define WM5100_ASYNC_CLK_FREQ_SHIFT                  8  /*                         */
#define WM5100_ASYNC_CLK_FREQ_WIDTH                  3  /*                         */
#define WM5100_ASYNC_CLK_ENA                    0x0040  /*               */
#define WM5100_ASYNC_CLK_ENA_MASK               0x0040  /*               */
#define WM5100_ASYNC_CLK_ENA_SHIFT                   6  /*               */
#define WM5100_ASYNC_CLK_ENA_WIDTH                   1  /*               */
#define WM5100_ASYNC_CLK_SRC_MASK               0x000F  /*                       */
#define WM5100_ASYNC_CLK_SRC_SHIFT                   0  /*                       */
#define WM5100_ASYNC_CLK_SRC_WIDTH                   4  /*                       */

/*
                            
 */
#define WM5100_ASYNC_SAMPLE_RATE_MASK           0x001F  /*                           */
#define WM5100_ASYNC_SAMPLE_RATE_SHIFT               0  /*                           */
#define WM5100_ASYNC_SAMPLE_RATE_WIDTH               5  /*                           */

/*
                             
 */
#define WM5100_ASRC2L_ENA                       0x0008  /*            */
#define WM5100_ASRC2L_ENA_MASK                  0x0008  /*            */
#define WM5100_ASRC2L_ENA_SHIFT                      3  /*            */
#define WM5100_ASRC2L_ENA_WIDTH                      1  /*            */
#define WM5100_ASRC2R_ENA                       0x0004  /*            */
#define WM5100_ASRC2R_ENA_MASK                  0x0004  /*            */
#define WM5100_ASRC2R_ENA_SHIFT                      2  /*            */
#define WM5100_ASRC2R_ENA_WIDTH                      1  /*            */
#define WM5100_ASRC1L_ENA                       0x0002  /*            */
#define WM5100_ASRC1L_ENA_MASK                  0x0002  /*            */
#define WM5100_ASRC1L_ENA_SHIFT                      1  /*            */
#define WM5100_ASRC1L_ENA_WIDTH                      1  /*            */
#define WM5100_ASRC1R_ENA                       0x0001  /*            */
#define WM5100_ASRC1R_ENA_MASK                  0x0001  /*            */
#define WM5100_ASRC1R_ENA_SHIFT                      0  /*            */
#define WM5100_ASRC1R_ENA_WIDTH                      1  /*            */

/*
                             
 */
#define WM5100_ASRC2L_ENA_STS                   0x0008  /*                */
#define WM5100_ASRC2L_ENA_STS_MASK              0x0008  /*                */
#define WM5100_ASRC2L_ENA_STS_SHIFT                  3  /*                */
#define WM5100_ASRC2L_ENA_STS_WIDTH                  1  /*                */
#define WM5100_ASRC2R_ENA_STS                   0x0004  /*                */
#define WM5100_ASRC2R_ENA_STS_MASK              0x0004  /*                */
#define WM5100_ASRC2R_ENA_STS_SHIFT                  2  /*                */
#define WM5100_ASRC2R_ENA_STS_WIDTH                  1  /*                */
#define WM5100_ASRC1L_ENA_STS                   0x0002  /*                */
#define WM5100_ASRC1L_ENA_STS_MASK              0x0002  /*                */
#define WM5100_ASRC1L_ENA_STS_SHIFT                  1  /*                */
#define WM5100_ASRC1L_ENA_STS_WIDTH                  1  /*                */
#define WM5100_ASRC1R_ENA_STS                   0x0001  /*                */
#define WM5100_ASRC1R_ENA_STS_MASK              0x0001  /*                */
#define WM5100_ASRC1R_ENA_STS_SHIFT                  0  /*                */
#define WM5100_ASRC1R_ENA_STS_WIDTH                  1  /*                */

/*
                            
 */
#define WM5100_ASRC_RATE1_MASK                  0x0006  /*                    */
#define WM5100_ASRC_RATE1_SHIFT                      1  /*                    */
#define WM5100_ASRC_RATE1_WIDTH                      2  /*                    */

/*
                               
 */
#define WM5100_ISRC1_DFS_ENA                    0x2000  /*               */
#define WM5100_ISRC1_DFS_ENA_MASK               0x2000  /*               */
#define WM5100_ISRC1_DFS_ENA_SHIFT                  13  /*               */
#define WM5100_ISRC1_DFS_ENA_WIDTH                   1  /*               */
#define WM5100_ISRC1_CLK_SEL_MASK               0x0300  /*                       */
#define WM5100_ISRC1_CLK_SEL_SHIFT                   8  /*                       */
#define WM5100_ISRC1_CLK_SEL_WIDTH                   2  /*                       */
#define WM5100_ISRC1_FSH_MASK                   0x000C  /*                   */
#define WM5100_ISRC1_FSH_SHIFT                       2  /*                   */
#define WM5100_ISRC1_FSH_WIDTH                       2  /*                   */
#define WM5100_ISRC1_FSL_MASK                   0x0003  /*                   */
#define WM5100_ISRC1_FSL_SHIFT                       0  /*                   */
#define WM5100_ISRC1_FSL_WIDTH                       2  /*                   */

/*
                               
 */
#define WM5100_ISRC1_INT1_ENA                   0x8000  /*                */
#define WM5100_ISRC1_INT1_ENA_MASK              0x8000  /*                */
#define WM5100_ISRC1_INT1_ENA_SHIFT                 15  /*                */
#define WM5100_ISRC1_INT1_ENA_WIDTH                  1  /*                */
#define WM5100_ISRC1_INT2_ENA                   0x4000  /*                */
#define WM5100_ISRC1_INT2_ENA_MASK              0x4000  /*                */
#define WM5100_ISRC1_INT2_ENA_SHIFT                 14  /*                */
#define WM5100_ISRC1_INT2_ENA_WIDTH                  1  /*                */
#define WM5100_ISRC1_INT3_ENA                   0x2000  /*                */
#define WM5100_ISRC1_INT3_ENA_MASK              0x2000  /*                */
#define WM5100_ISRC1_INT3_ENA_SHIFT                 13  /*                */
#define WM5100_ISRC1_INT3_ENA_WIDTH                  1  /*                */
#define WM5100_ISRC1_INT4_ENA                   0x1000  /*                */
#define WM5100_ISRC1_INT4_ENA_MASK              0x1000  /*                */
#define WM5100_ISRC1_INT4_ENA_SHIFT                 12  /*                */
#define WM5100_ISRC1_INT4_ENA_WIDTH                  1  /*                */
#define WM5100_ISRC1_DEC1_ENA                   0x0200  /*                */
#define WM5100_ISRC1_DEC1_ENA_MASK              0x0200  /*                */
#define WM5100_ISRC1_DEC1_ENA_SHIFT                  9  /*                */
#define WM5100_ISRC1_DEC1_ENA_WIDTH                  1  /*                */
#define WM5100_ISRC1_DEC2_ENA                   0x0100  /*                */
#define WM5100_ISRC1_DEC2_ENA_MASK              0x0100  /*                */
#define WM5100_ISRC1_DEC2_ENA_SHIFT                  8  /*                */
#define WM5100_ISRC1_DEC2_ENA_WIDTH                  1  /*                */
#define WM5100_ISRC1_DEC3_ENA                   0x0080  /*                */
#define WM5100_ISRC1_DEC3_ENA_MASK              0x0080  /*                */
#define WM5100_ISRC1_DEC3_ENA_SHIFT                  7  /*                */
#define WM5100_ISRC1_DEC3_ENA_WIDTH                  1  /*                */
#define WM5100_ISRC1_DEC4_ENA                   0x0040  /*                */
#define WM5100_ISRC1_DEC4_ENA_MASK              0x0040  /*                */
#define WM5100_ISRC1_DEC4_ENA_SHIFT                  6  /*                */
#define WM5100_ISRC1_DEC4_ENA_WIDTH                  1  /*                */
#define WM5100_ISRC1_NOTCH_ENA                  0x0001  /*                 */
#define WM5100_ISRC1_NOTCH_ENA_MASK             0x0001  /*                 */
#define WM5100_ISRC1_NOTCH_ENA_SHIFT                 0  /*                 */
#define WM5100_ISRC1_NOTCH_ENA_WIDTH                 1  /*                 */

/*
                              
 */
#define WM5100_ISRC2_DFS_ENA                    0x2000  /*               */
#define WM5100_ISRC2_DFS_ENA_MASK               0x2000  /*               */
#define WM5100_ISRC2_DFS_ENA_SHIFT                  13  /*               */
#define WM5100_ISRC2_DFS_ENA_WIDTH                   1  /*               */
#define WM5100_ISRC2_CLK_SEL_MASK               0x0300  /*                       */
#define WM5100_ISRC2_CLK_SEL_SHIFT                   8  /*                       */
#define WM5100_ISRC2_CLK_SEL_WIDTH                   2  /*                       */
#define WM5100_ISRC2_FSH_MASK                   0x000C  /*                   */
#define WM5100_ISRC2_FSH_SHIFT                       2  /*                   */
#define WM5100_ISRC2_FSH_WIDTH                       2  /*                   */
#define WM5100_ISRC2_FSL_MASK                   0x0003  /*                   */
#define WM5100_ISRC2_FSL_SHIFT                       0  /*                   */
#define WM5100_ISRC2_FSL_WIDTH                       2  /*                   */

/*
                               
 */
#define WM5100_ISRC2_INT1_ENA                   0x8000  /*                */
#define WM5100_ISRC2_INT1_ENA_MASK              0x8000  /*                */
#define WM5100_ISRC2_INT1_ENA_SHIFT                 15  /*                */
#define WM5100_ISRC2_INT1_ENA_WIDTH                  1  /*                */
#define WM5100_ISRC2_INT2_ENA                   0x4000  /*                */
#define WM5100_ISRC2_INT2_ENA_MASK              0x4000  /*                */
#define WM5100_ISRC2_INT2_ENA_SHIFT                 14  /*                */
#define WM5100_ISRC2_INT2_ENA_WIDTH                  1  /*                */
#define WM5100_ISRC2_INT3_ENA                   0x2000  /*                */
#define WM5100_ISRC2_INT3_ENA_MASK              0x2000  /*                */
#define WM5100_ISRC2_INT3_ENA_SHIFT                 13  /*                */
#define WM5100_ISRC2_INT3_ENA_WIDTH                  1  /*                */
#define WM5100_ISRC2_INT4_ENA                   0x1000  /*                */
#define WM5100_ISRC2_INT4_ENA_MASK              0x1000  /*                */
#define WM5100_ISRC2_INT4_ENA_SHIFT                 12  /*                */
#define WM5100_ISRC2_INT4_ENA_WIDTH                  1  /*                */
#define WM5100_ISRC2_DEC1_ENA                   0x0200  /*                */
#define WM5100_ISRC2_DEC1_ENA_MASK              0x0200  /*                */
#define WM5100_ISRC2_DEC1_ENA_SHIFT                  9  /*                */
#define WM5100_ISRC2_DEC1_ENA_WIDTH                  1  /*                */
#define WM5100_ISRC2_DEC2_ENA                   0x0100  /*                */
#define WM5100_ISRC2_DEC2_ENA_MASK              0x0100  /*                */
#define WM5100_ISRC2_DEC2_ENA_SHIFT                  8  /*                */
#define WM5100_ISRC2_DEC2_ENA_WIDTH                  1  /*                */
#define WM5100_ISRC2_DEC3_ENA                   0x0080  /*                */
#define WM5100_ISRC2_DEC3_ENA_MASK              0x0080  /*                */
#define WM5100_ISRC2_DEC3_ENA_SHIFT                  7  /*                */
#define WM5100_ISRC2_DEC3_ENA_WIDTH                  1  /*                */
#define WM5100_ISRC2_DEC4_ENA                   0x0040  /*                */
#define WM5100_ISRC2_DEC4_ENA_MASK              0x0040  /*                */
#define WM5100_ISRC2_DEC4_ENA_SHIFT                  6  /*                */
#define WM5100_ISRC2_DEC4_ENA_WIDTH                  1  /*                */
#define WM5100_ISRC2_NOTCH_ENA                  0x0001  /*                 */
#define WM5100_ISRC2_NOTCH_ENA_MASK             0x0001  /*                 */
#define WM5100_ISRC2_NOTCH_ENA_SHIFT                 0  /*                 */
#define WM5100_ISRC2_NOTCH_ENA_WIDTH                 1  /*                 */

/*
                                
 */
#define WM5100_FLL1_ENA                         0x0001  /*          */
#define WM5100_FLL1_ENA_MASK                    0x0001  /*          */
#define WM5100_FLL1_ENA_SHIFT                        0  /*          */
#define WM5100_FLL1_ENA_WIDTH                        1  /*          */

/*
                                
 */
#define WM5100_FLL1_OUTDIV_MASK                 0x3F00  /*                      */
#define WM5100_FLL1_OUTDIV_SHIFT                     8  /*                      */
#define WM5100_FLL1_OUTDIV_WIDTH                     6  /*                      */
#define WM5100_FLL1_FRATIO_MASK                 0x0007  /*                     */
#define WM5100_FLL1_FRATIO_SHIFT                     0  /*                     */
#define WM5100_FLL1_FRATIO_WIDTH                     3  /*                     */

/*
                                
 */
#define WM5100_FLL1_THETA_MASK                  0xFFFF  /*                     */
#define WM5100_FLL1_THETA_SHIFT                      0  /*                     */
#define WM5100_FLL1_THETA_WIDTH                     16  /*                     */

/*
                                
 */
#define WM5100_FLL1_N_MASK                      0x03FF  /*                */
#define WM5100_FLL1_N_SHIFT                          0  /*                */
#define WM5100_FLL1_N_WIDTH                         10  /*                */

/*
                                
 */
#define WM5100_FLL1_REFCLK_DIV_MASK             0x00C0  /*                         */
#define WM5100_FLL1_REFCLK_DIV_SHIFT                 6  /*                         */
#define WM5100_FLL1_REFCLK_DIV_WIDTH                 2  /*                         */
#define WM5100_FLL1_REFCLK_SRC_MASK             0x000F  /*                         */
#define WM5100_FLL1_REFCLK_SRC_SHIFT                 0  /*                         */
#define WM5100_FLL1_REFCLK_SRC_WIDTH                 4  /*                         */

/*
                            
 */
#define WM5100_FLL1_LAMBDA_MASK                 0xFFFF  /*                      */
#define WM5100_FLL1_LAMBDA_SHIFT                     0  /*                      */
#define WM5100_FLL1_LAMBDA_WIDTH                    16  /*                      */

/*
                                
 */
#define WM5100_FLL2_ENA                         0x0001  /*          */
#define WM5100_FLL2_ENA_MASK                    0x0001  /*          */
#define WM5100_FLL2_ENA_SHIFT                        0  /*          */
#define WM5100_FLL2_ENA_WIDTH                        1  /*          */

/*
                                
 */
#define WM5100_FLL2_OUTDIV_MASK                 0x3F00  /*                      */
#define WM5100_FLL2_OUTDIV_SHIFT                     8  /*                      */
#define WM5100_FLL2_OUTDIV_WIDTH                     6  /*                      */
#define WM5100_FLL2_FRATIO_MASK                 0x0007  /*                     */
#define WM5100_FLL2_FRATIO_SHIFT                     0  /*                     */
#define WM5100_FLL2_FRATIO_WIDTH                     3  /*                     */

/*
                                
 */
#define WM5100_FLL2_THETA_MASK                  0xFFFF  /*                     */
#define WM5100_FLL2_THETA_SHIFT                      0  /*                     */
#define WM5100_FLL2_THETA_WIDTH                     16  /*                     */

/*
                                
 */
#define WM5100_FLL2_N_MASK                      0x03FF  /*                */
#define WM5100_FLL2_N_SHIFT                          0  /*                */
#define WM5100_FLL2_N_WIDTH                         10  /*                */

/*
                                
 */
#define WM5100_FLL2_REFCLK_DIV_MASK             0x00C0  /*                         */
#define WM5100_FLL2_REFCLK_DIV_SHIFT                 6  /*                         */
#define WM5100_FLL2_REFCLK_DIV_WIDTH                 2  /*                         */
#define WM5100_FLL2_REFCLK_SRC_MASK             0x000F  /*                         */
#define WM5100_FLL2_REFCLK_SRC_SHIFT                 0  /*                         */
#define WM5100_FLL2_REFCLK_SRC_WIDTH                 4  /*                         */

/*
                            
 */
#define WM5100_FLL2_LAMBDA_MASK                 0xFFFF  /*                      */
#define WM5100_FLL2_LAMBDA_SHIFT                     0  /*                      */
#define WM5100_FLL2_LAMBDA_WIDTH                    16  /*                      */

/*
                                   
 */
#define WM5100_CP2_BYPASS                       0x0020  /*            */
#define WM5100_CP2_BYPASS_MASK                  0x0020  /*            */
#define WM5100_CP2_BYPASS_SHIFT                      5  /*            */
#define WM5100_CP2_BYPASS_WIDTH                      1  /*            */
#define WM5100_CP2_ENA                          0x0001  /*         */
#define WM5100_CP2_ENA_MASK                     0x0001  /*         */
#define WM5100_CP2_ENA_SHIFT                         0  /*         */
#define WM5100_CP2_ENA_WIDTH                         1  /*         */

/*
                                   
 */
#define WM5100_LDO2_VSEL_MASK                   0xF800  /*                     */
#define WM5100_LDO2_VSEL_SHIFT                      11  /*                     */
#define WM5100_LDO2_VSEL_WIDTH                       5  /*                     */

/*
                                  
 */
#define WM5100_CP1_ENA                          0x0001  /*         */
#define WM5100_CP1_ENA_MASK                     0x0001  /*         */
#define WM5100_CP1_ENA_SHIFT                         0  /*         */
#define WM5100_CP1_ENA_WIDTH                         1  /*         */

/*
                              
 */
#define WM5100_LDO1_BYPASS                      0x0002  /*             */
#define WM5100_LDO1_BYPASS_MASK                 0x0002  /*             */
#define WM5100_LDO1_BYPASS_SHIFT                     1  /*             */
#define WM5100_LDO1_BYPASS_WIDTH                     1  /*             */

/*
                                 
 */
#define WM5100_MICB1_DISCH                      0x0040  /*             */
#define WM5100_MICB1_DISCH_MASK                 0x0040  /*             */
#define WM5100_MICB1_DISCH_SHIFT                     6  /*             */
#define WM5100_MICB1_DISCH_WIDTH                     1  /*             */
#define WM5100_MICB1_RATE                       0x0020  /*            */
#define WM5100_MICB1_RATE_MASK                  0x0020  /*            */
#define WM5100_MICB1_RATE_SHIFT                      5  /*            */
#define WM5100_MICB1_RATE_WIDTH                      1  /*            */
#define WM5100_MICB1_LVL_MASK                   0x001C  /*                   */
#define WM5100_MICB1_LVL_SHIFT                       2  /*                   */
#define WM5100_MICB1_LVL_WIDTH                       3  /*                   */
#define WM5100_MICB1_BYPASS                     0x0002  /*              */
#define WM5100_MICB1_BYPASS_MASK                0x0002  /*              */
#define WM5100_MICB1_BYPASS_SHIFT                    1  /*              */
#define WM5100_MICB1_BYPASS_WIDTH                    1  /*              */
#define WM5100_MICB1_ENA                        0x0001  /*           */
#define WM5100_MICB1_ENA_MASK                   0x0001  /*           */
#define WM5100_MICB1_ENA_SHIFT                       0  /*           */
#define WM5100_MICB1_ENA_WIDTH                       1  /*           */

/*
                                 
 */
#define WM5100_MICB2_DISCH                      0x0040  /*             */
#define WM5100_MICB2_DISCH_MASK                 0x0040  /*             */
#define WM5100_MICB2_DISCH_SHIFT                     6  /*             */
#define WM5100_MICB2_DISCH_WIDTH                     1  /*             */
#define WM5100_MICB2_RATE                       0x0020  /*            */
#define WM5100_MICB2_RATE_MASK                  0x0020  /*            */
#define WM5100_MICB2_RATE_SHIFT                      5  /*            */
#define WM5100_MICB2_RATE_WIDTH                      1  /*            */
#define WM5100_MICB2_LVL_MASK                   0x001C  /*                   */
#define WM5100_MICB2_LVL_SHIFT                       2  /*                   */
#define WM5100_MICB2_LVL_WIDTH                       3  /*                   */
#define WM5100_MICB2_BYPASS                     0x0002  /*              */
#define WM5100_MICB2_BYPASS_MASK                0x0002  /*              */
#define WM5100_MICB2_BYPASS_SHIFT                    1  /*              */
#define WM5100_MICB2_BYPASS_WIDTH                    1  /*              */
#define WM5100_MICB2_ENA                        0x0001  /*           */
#define WM5100_MICB2_ENA_MASK                   0x0001  /*           */
#define WM5100_MICB2_ENA_SHIFT                       0  /*           */
#define WM5100_MICB2_ENA_WIDTH                       1  /*           */

/*
                                 
 */
#define WM5100_MICB3_DISCH                      0x0040  /*             */
#define WM5100_MICB3_DISCH_MASK                 0x0040  /*             */
#define WM5100_MICB3_DISCH_SHIFT                     6  /*             */
#define WM5100_MICB3_DISCH_WIDTH                     1  /*             */
#define WM5100_MICB3_RATE                       0x0020  /*            */
#define WM5100_MICB3_RATE_MASK                  0x0020  /*            */
#define WM5100_MICB3_RATE_SHIFT                      5  /*            */
#define WM5100_MICB3_RATE_WIDTH                      1  /*            */
#define WM5100_MICB3_LVL_MASK                   0x001C  /*                   */
#define WM5100_MICB3_LVL_SHIFT                       2  /*                   */
#define WM5100_MICB3_LVL_WIDTH                       3  /*                   */
#define WM5100_MICB3_BYPASS                     0x0002  /*              */
#define WM5100_MICB3_BYPASS_MASK                0x0002  /*              */
#define WM5100_MICB3_BYPASS_SHIFT                    1  /*              */
#define WM5100_MICB3_BYPASS_WIDTH                    1  /*              */
#define WM5100_MICB3_ENA                        0x0001  /*           */
#define WM5100_MICB3_ENA_MASK                   0x0001  /*           */
#define WM5100_MICB3_ENA_SHIFT                       0  /*           */
#define WM5100_MICB3_ENA_WIDTH                       1  /*           */

/*
                                         
 */
#define WM5100_ACCDET_BIAS_SRC_MASK             0xC000  /*                           */
#define WM5100_ACCDET_BIAS_SRC_SHIFT                14  /*                           */
#define WM5100_ACCDET_BIAS_SRC_WIDTH                 2  /*                           */
#define WM5100_ACCDET_SRC                       0x2000  /*            */
#define WM5100_ACCDET_SRC_MASK                  0x2000  /*            */
#define WM5100_ACCDET_SRC_SHIFT                     13  /*            */
#define WM5100_ACCDET_SRC_WIDTH                      1  /*            */
#define WM5100_ACCDET_MODE_MASK                 0x0003  /*                     */
#define WM5100_ACCDET_MODE_SHIFT                     0  /*                     */
#define WM5100_ACCDET_MODE_WIDTH                     2  /*                     */

/*
                                    
 */
#define WM5100_HP_HOLDTIME_MASK                 0x00E0  /*                     */
#define WM5100_HP_HOLDTIME_SHIFT                     5  /*                     */
#define WM5100_HP_HOLDTIME_WIDTH                     3  /*                     */
#define WM5100_HP_CLK_DIV_MASK                  0x0018  /*                    */
#define WM5100_HP_CLK_DIV_SHIFT                      3  /*                    */
#define WM5100_HP_CLK_DIV_WIDTH                      2  /*                    */
#define WM5100_HP_STEP_SIZE                     0x0002  /*              */
#define WM5100_HP_STEP_SIZE_MASK                0x0002  /*              */
#define WM5100_HP_STEP_SIZE_SHIFT                    1  /*              */
#define WM5100_HP_STEP_SIZE_WIDTH                    1  /*              */
#define WM5100_HP_POLL                          0x0001  /*         */
#define WM5100_HP_POLL_MASK                     0x0001  /*         */
#define WM5100_HP_POLL_SHIFT                         0  /*         */
#define WM5100_HP_POLL_WIDTH                         1  /*         */

/*
                                    
 */
#define WM5100_HP_DONE                          0x0080  /*         */
#define WM5100_HP_DONE_MASK                     0x0080  /*         */
#define WM5100_HP_DONE_SHIFT                         7  /*         */
#define WM5100_HP_DONE_WIDTH                         1  /*         */
#define WM5100_HP_LVL_MASK                      0x007F  /*                */
#define WM5100_HP_LVL_SHIFT                          0  /*                */
#define WM5100_HP_LVL_WIDTH                          7  /*                */

/*
                              
 */
#define WM5100_ACCDET_BIAS_STARTTIME_MASK       0xF000  /*                                 */
#define WM5100_ACCDET_BIAS_STARTTIME_SHIFT          12  /*                                 */
#define WM5100_ACCDET_BIAS_STARTTIME_WIDTH           4  /*                                 */
#define WM5100_ACCDET_RATE_MASK                 0x0F00  /*                      */
#define WM5100_ACCDET_RATE_SHIFT                     8  /*                      */
#define WM5100_ACCDET_RATE_WIDTH                     4  /*                      */
#define WM5100_ACCDET_DBTIME                    0x0002  /*               */
#define WM5100_ACCDET_DBTIME_MASK               0x0002  /*               */
#define WM5100_ACCDET_DBTIME_SHIFT                   1  /*               */
#define WM5100_ACCDET_DBTIME_WIDTH                   1  /*               */
#define WM5100_ACCDET_ENA                       0x0001  /*            */
#define WM5100_ACCDET_ENA_MASK                  0x0001  /*            */
#define WM5100_ACCDET_ENA_SHIFT                      0  /*            */
#define WM5100_ACCDET_ENA_WIDTH                      1  /*            */

/*
                              
 */
#define WM5100_ACCDET_LVL_SEL_MASK              0x00FF  /*                        */
#define WM5100_ACCDET_LVL_SEL_SHIFT                  0  /*                        */
#define WM5100_ACCDET_LVL_SEL_WIDTH                  8  /*                        */

/*
                              
 */
#define WM5100_ACCDET_LVL_MASK                  0x07FC  /*                     */
#define WM5100_ACCDET_LVL_SHIFT                      2  /*                     */
#define WM5100_ACCDET_LVL_WIDTH                      9  /*                     */
#define WM5100_ACCDET_VALID                     0x0002  /*              */
#define WM5100_ACCDET_VALID_MASK                0x0002  /*              */
#define WM5100_ACCDET_VALID_SHIFT                    1  /*              */
#define WM5100_ACCDET_VALID_WIDTH                    1  /*              */
#define WM5100_ACCDET_STS                       0x0001  /*            */
#define WM5100_ACCDET_STS_MASK                  0x0001  /*            */
#define WM5100_ACCDET_STS_SHIFT                      0  /*            */
#define WM5100_ACCDET_STS_WIDTH                      1  /*            */

/*
                              
 */
#define WM5100_HPCOM_SRC                         0x200  /*           */
#define WM5100_HPCOM_SRC_SHIFT                       9  /*           */

/*
                               
 */
#define WM5100_IN4L_ENA                         0x0080  /*          */
#define WM5100_IN4L_ENA_MASK                    0x0080  /*          */
#define WM5100_IN4L_ENA_SHIFT                        7  /*          */
#define WM5100_IN4L_ENA_WIDTH                        1  /*          */
#define WM5100_IN4R_ENA                         0x0040  /*          */
#define WM5100_IN4R_ENA_MASK                    0x0040  /*          */
#define WM5100_IN4R_ENA_SHIFT                        6  /*          */
#define WM5100_IN4R_ENA_WIDTH                        1  /*          */
#define WM5100_IN3L_ENA                         0x0020  /*          */
#define WM5100_IN3L_ENA_MASK                    0x0020  /*          */
#define WM5100_IN3L_ENA_SHIFT                        5  /*          */
#define WM5100_IN3L_ENA_WIDTH                        1  /*          */
#define WM5100_IN3R_ENA                         0x0010  /*          */
#define WM5100_IN3R_ENA_MASK                    0x0010  /*          */
#define WM5100_IN3R_ENA_SHIFT                        4  /*          */
#define WM5100_IN3R_ENA_WIDTH                        1  /*          */
#define WM5100_IN2L_ENA                         0x0008  /*          */
#define WM5100_IN2L_ENA_MASK                    0x0008  /*          */
#define WM5100_IN2L_ENA_SHIFT                        3  /*          */
#define WM5100_IN2L_ENA_WIDTH                        1  /*          */
#define WM5100_IN2R_ENA                         0x0004  /*          */
#define WM5100_IN2R_ENA_MASK                    0x0004  /*          */
#define WM5100_IN2R_ENA_SHIFT                        2  /*          */
#define WM5100_IN2R_ENA_WIDTH                        1  /*          */
#define WM5100_IN1L_ENA                         0x0002  /*          */
#define WM5100_IN1L_ENA_MASK                    0x0002  /*          */
#define WM5100_IN1L_ENA_SHIFT                        1  /*          */
#define WM5100_IN1L_ENA_WIDTH                        1  /*          */
#define WM5100_IN1R_ENA                         0x0001  /*          */
#define WM5100_IN1R_ENA_MASK                    0x0001  /*          */
#define WM5100_IN1R_ENA_SHIFT                        0  /*          */
#define WM5100_IN1R_ENA_WIDTH                        1  /*          */

/*
                                      
 */
#define WM5100_IN4L_ENA_STS                     0x0080  /*              */
#define WM5100_IN4L_ENA_STS_MASK                0x0080  /*              */
#define WM5100_IN4L_ENA_STS_SHIFT                    7  /*              */
#define WM5100_IN4L_ENA_STS_WIDTH                    1  /*              */
#define WM5100_IN4R_ENA_STS                     0x0040  /*              */
#define WM5100_IN4R_ENA_STS_MASK                0x0040  /*              */
#define WM5100_IN4R_ENA_STS_SHIFT                    6  /*              */
#define WM5100_IN4R_ENA_STS_WIDTH                    1  /*              */
#define WM5100_IN3L_ENA_STS                     0x0020  /*              */
#define WM5100_IN3L_ENA_STS_MASK                0x0020  /*              */
#define WM5100_IN3L_ENA_STS_SHIFT                    5  /*              */
#define WM5100_IN3L_ENA_STS_WIDTH                    1  /*              */
#define WM5100_IN3R_ENA_STS                     0x0010  /*              */
#define WM5100_IN3R_ENA_STS_MASK                0x0010  /*              */
#define WM5100_IN3R_ENA_STS_SHIFT                    4  /*              */
#define WM5100_IN3R_ENA_STS_WIDTH                    1  /*              */
#define WM5100_IN2L_ENA_STS                     0x0008  /*              */
#define WM5100_IN2L_ENA_STS_MASK                0x0008  /*              */
#define WM5100_IN2L_ENA_STS_SHIFT                    3  /*              */
#define WM5100_IN2L_ENA_STS_WIDTH                    1  /*              */
#define WM5100_IN2R_ENA_STS                     0x0004  /*              */
#define WM5100_IN2R_ENA_STS_MASK                0x0004  /*              */
#define WM5100_IN2R_ENA_STS_SHIFT                    2  /*              */
#define WM5100_IN2R_ENA_STS_WIDTH                    1  /*              */
#define WM5100_IN1L_ENA_STS                     0x0002  /*              */
#define WM5100_IN1L_ENA_STS_MASK                0x0002  /*              */
#define WM5100_IN1L_ENA_STS_SHIFT                    1  /*              */
#define WM5100_IN1L_ENA_STS_WIDTH                    1  /*              */
#define WM5100_IN1R_ENA_STS                     0x0001  /*              */
#define WM5100_IN1R_ENA_STS_MASK                0x0001  /*              */
#define WM5100_IN1R_ENA_STS_SHIFT                    0  /*              */
#define WM5100_IN1R_ENA_STS_WIDTH                    1  /*              */

/*
                              
 */
#define WM5100_IN_RATE_MASK                     0xC000  /*                   */
#define WM5100_IN_RATE_SHIFT                        14  /*                   */
#define WM5100_IN_RATE_WIDTH                         2  /*                   */
#define WM5100_IN1_OSR                          0x2000  /*         */
#define WM5100_IN1_OSR_MASK                     0x2000  /*         */
#define WM5100_IN1_OSR_SHIFT                        13  /*         */
#define WM5100_IN1_OSR_WIDTH                         1  /*         */
#define WM5100_IN1_DMIC_SUP_MASK                0x1800  /*                        */
#define WM5100_IN1_DMIC_SUP_SHIFT                   11  /*                        */
#define WM5100_IN1_DMIC_SUP_WIDTH                    2  /*                        */
#define WM5100_IN1_MODE_MASK                    0x0600  /*                   */
#define WM5100_IN1_MODE_SHIFT                        9  /*                   */
#define WM5100_IN1_MODE_WIDTH                        2  /*                   */
#define WM5100_IN1L_PGA_VOL_MASK                0x00FE  /*                      */
#define WM5100_IN1L_PGA_VOL_SHIFT                    1  /*                      */
#define WM5100_IN1L_PGA_VOL_WIDTH                    7  /*                      */

/*
                              
 */
#define WM5100_IN1R_PGA_VOL_MASK                0x00FE  /*                      */
#define WM5100_IN1R_PGA_VOL_SHIFT                    1  /*                      */
#define WM5100_IN1R_PGA_VOL_WIDTH                    7  /*                      */

/*
                              
 */
#define WM5100_IN2_OSR                          0x2000  /*         */
#define WM5100_IN2_OSR_MASK                     0x2000  /*         */
#define WM5100_IN2_OSR_SHIFT                        13  /*         */
#define WM5100_IN2_OSR_WIDTH                         1  /*         */
#define WM5100_IN2_DMIC_SUP_MASK                0x1800  /*                        */
#define WM5100_IN2_DMIC_SUP_SHIFT                   11  /*                        */
#define WM5100_IN2_DMIC_SUP_WIDTH                    2  /*                        */
#define WM5100_IN2_MODE_MASK                    0x0600  /*                   */
#define WM5100_IN2_MODE_SHIFT                        9  /*                   */
#define WM5100_IN2_MODE_WIDTH                        2  /*                   */
#define WM5100_IN2L_PGA_VOL_MASK                0x00FE  /*                      */
#define WM5100_IN2L_PGA_VOL_SHIFT                    1  /*                      */
#define WM5100_IN2L_PGA_VOL_WIDTH                    7  /*                      */

/*
                              
 */
#define WM5100_IN2R_PGA_VOL_MASK                0x00FE  /*                      */
#define WM5100_IN2R_PGA_VOL_SHIFT                    1  /*                      */
#define WM5100_IN2R_PGA_VOL_WIDTH                    7  /*                      */

/*
                              
 */
#define WM5100_IN3_OSR                          0x2000  /*         */
#define WM5100_IN3_OSR_MASK                     0x2000  /*         */
#define WM5100_IN3_OSR_SHIFT                        13  /*         */
#define WM5100_IN3_OSR_WIDTH                         1  /*         */
#define WM5100_IN3_DMIC_SUP_MASK                0x1800  /*                        */
#define WM5100_IN3_DMIC_SUP_SHIFT                   11  /*                        */
#define WM5100_IN3_DMIC_SUP_WIDTH                    2  /*                        */
#define WM5100_IN3_MODE_MASK                    0x0600  /*                   */
#define WM5100_IN3_MODE_SHIFT                        9  /*                   */
#define WM5100_IN3_MODE_WIDTH                        2  /*                   */
#define WM5100_IN3L_PGA_VOL_MASK                0x00FE  /*                      */
#define WM5100_IN3L_PGA_VOL_SHIFT                    1  /*                      */
#define WM5100_IN3L_PGA_VOL_WIDTH                    7  /*                      */

/*
                              
 */
#define WM5100_IN3R_PGA_VOL_MASK                0x00FE  /*                      */
#define WM5100_IN3R_PGA_VOL_SHIFT                    1  /*                      */
#define WM5100_IN3R_PGA_VOL_WIDTH                    7  /*                      */

/*
                              
 */
#define WM5100_IN4_OSR                          0x2000  /*         */
#define WM5100_IN4_OSR_MASK                     0x2000  /*         */
#define WM5100_IN4_OSR_SHIFT                        13  /*         */
#define WM5100_IN4_OSR_WIDTH                         1  /*         */
#define WM5100_IN4_DMIC_SUP_MASK                0x1800  /*                        */
#define WM5100_IN4_DMIC_SUP_SHIFT                   11  /*                        */
#define WM5100_IN4_DMIC_SUP_WIDTH                    2  /*                        */
#define WM5100_IN4_MODE_MASK                    0x0600  /*                   */
#define WM5100_IN4_MODE_SHIFT                        9  /*                   */
#define WM5100_IN4_MODE_WIDTH                        2  /*                   */
#define WM5100_IN4L_PGA_VOL_MASK                0x00FE  /*                      */
#define WM5100_IN4L_PGA_VOL_SHIFT                    1  /*                      */
#define WM5100_IN4L_PGA_VOL_WIDTH                    7  /*                      */

/*
                              
 */
#define WM5100_IN4R_PGA_VOL_MASK                0x00FE  /*                      */
#define WM5100_IN4R_PGA_VOL_SHIFT                    1  /*                      */
#define WM5100_IN4R_PGA_VOL_WIDTH                    7  /*                      */

/*
                           
 */
#define WM5100_IN_RXANC_SEL_MASK                0x0007  /*                      */
#define WM5100_IN_RXANC_SEL_SHIFT                    0  /*                      */
#define WM5100_IN_RXANC_SEL_WIDTH                    3  /*                      */

/*
                                   
 */
#define WM5100_IN_VD_RAMP_MASK                  0x0070  /*                    */
#define WM5100_IN_VD_RAMP_SHIFT                      4  /*                    */
#define WM5100_IN_VD_RAMP_WIDTH                      3  /*                    */
#define WM5100_IN_VI_RAMP_MASK                  0x0007  /*                    */
#define WM5100_IN_VI_RAMP_SHIFT                      0  /*                    */
#define WM5100_IN_VI_RAMP_WIDTH                      3  /*                    */

/*
                                       
 */
#define WM5100_IN_VU                            0x0200  /*       */
#define WM5100_IN_VU_MASK                       0x0200  /*       */
#define WM5100_IN_VU_SHIFT                           9  /*       */
#define WM5100_IN_VU_WIDTH                           1  /*       */
#define WM5100_IN1L_MUTE                        0x0100  /*           */
#define WM5100_IN1L_MUTE_MASK                   0x0100  /*           */
#define WM5100_IN1L_MUTE_SHIFT                       8  /*           */
#define WM5100_IN1L_MUTE_WIDTH                       1  /*           */
#define WM5100_IN1L_VOL_MASK                    0x00FF  /*                  */
#define WM5100_IN1L_VOL_SHIFT                        0  /*                  */
#define WM5100_IN1L_VOL_WIDTH                        8  /*                  */

/*
                                       
 */
#define WM5100_IN_VU                            0x0200  /*       */
#define WM5100_IN_VU_MASK                       0x0200  /*       */
#define WM5100_IN_VU_SHIFT                           9  /*       */
#define WM5100_IN_VU_WIDTH                           1  /*       */
#define WM5100_IN1R_MUTE                        0x0100  /*           */
#define WM5100_IN1R_MUTE_MASK                   0x0100  /*           */
#define WM5100_IN1R_MUTE_SHIFT                       8  /*           */
#define WM5100_IN1R_MUTE_WIDTH                       1  /*           */
#define WM5100_IN1R_VOL_MASK                    0x00FF  /*                  */
#define WM5100_IN1R_VOL_SHIFT                        0  /*                  */
#define WM5100_IN1R_VOL_WIDTH                        8  /*                  */

/*
                                       
 */
#define WM5100_IN_VU                            0x0200  /*       */
#define WM5100_IN_VU_MASK                       0x0200  /*       */
#define WM5100_IN_VU_SHIFT                           9  /*       */
#define WM5100_IN_VU_WIDTH                           1  /*       */
#define WM5100_IN2L_MUTE                        0x0100  /*           */
#define WM5100_IN2L_MUTE_MASK                   0x0100  /*           */
#define WM5100_IN2L_MUTE_SHIFT                       8  /*           */
#define WM5100_IN2L_MUTE_WIDTH                       1  /*           */
#define WM5100_IN2L_VOL_MASK                    0x00FF  /*                  */
#define WM5100_IN2L_VOL_SHIFT                        0  /*                  */
#define WM5100_IN2L_VOL_WIDTH                        8  /*                  */

/*
                                       
 */
#define WM5100_IN_VU                            0x0200  /*       */
#define WM5100_IN_VU_MASK                       0x0200  /*       */
#define WM5100_IN_VU_SHIFT                           9  /*       */
#define WM5100_IN_VU_WIDTH                           1  /*       */
#define WM5100_IN2R_MUTE                        0x0100  /*           */
#define WM5100_IN2R_MUTE_MASK                   0x0100  /*           */
#define WM5100_IN2R_MUTE_SHIFT                       8  /*           */
#define WM5100_IN2R_MUTE_WIDTH                       1  /*           */
#define WM5100_IN2R_VOL_MASK                    0x00FF  /*                  */
#define WM5100_IN2R_VOL_SHIFT                        0  /*                  */
#define WM5100_IN2R_VOL_WIDTH                        8  /*                  */

/*
                                       
 */
#define WM5100_IN_VU                            0x0200  /*       */
#define WM5100_IN_VU_MASK                       0x0200  /*       */
#define WM5100_IN_VU_SHIFT                           9  /*       */
#define WM5100_IN_VU_WIDTH                           1  /*       */
#define WM5100_IN3L_MUTE                        0x0100  /*           */
#define WM5100_IN3L_MUTE_MASK                   0x0100  /*           */
#define WM5100_IN3L_MUTE_SHIFT                       8  /*           */
#define WM5100_IN3L_MUTE_WIDTH                       1  /*           */
#define WM5100_IN3L_VOL_MASK                    0x00FF  /*                  */
#define WM5100_IN3L_VOL_SHIFT                        0  /*                  */
#define WM5100_IN3L_VOL_WIDTH                        8  /*                  */

/*
                                       
 */
#define WM5100_IN_VU                            0x0200  /*       */
#define WM5100_IN_VU_MASK                       0x0200  /*       */
#define WM5100_IN_VU_SHIFT                           9  /*       */
#define WM5100_IN_VU_WIDTH                           1  /*       */
#define WM5100_IN3R_MUTE                        0x0100  /*           */
#define WM5100_IN3R_MUTE_MASK                   0x0100  /*           */
#define WM5100_IN3R_MUTE_SHIFT                       8  /*           */
#define WM5100_IN3R_MUTE_WIDTH                       1  /*           */
#define WM5100_IN3R_VOL_MASK                    0x00FF  /*                  */
#define WM5100_IN3R_VOL_SHIFT                        0  /*                  */
#define WM5100_IN3R_VOL_WIDTH                        8  /*                  */

/*
                                       
 */
#define WM5100_IN_VU                            0x0200  /*       */
#define WM5100_IN_VU_MASK                       0x0200  /*       */
#define WM5100_IN_VU_SHIFT                           9  /*       */
#define WM5100_IN_VU_WIDTH                           1  /*       */
#define WM5100_IN4L_MUTE                        0x0100  /*           */
#define WM5100_IN4L_MUTE_MASK                   0x0100  /*           */
#define WM5100_IN4L_MUTE_SHIFT                       8  /*           */
#define WM5100_IN4L_MUTE_WIDTH                       1  /*           */
#define WM5100_IN4L_VOL_MASK                    0x00FF  /*                  */
#define WM5100_IN4L_VOL_SHIFT                        0  /*                  */
#define WM5100_IN4L_VOL_WIDTH                        8  /*                  */

/*
                                       
 */
#define WM5100_IN_VU                            0x0200  /*       */
#define WM5100_IN_VU_MASK                       0x0200  /*       */
#define WM5100_IN_VU_SHIFT                           9  /*       */
#define WM5100_IN_VU_WIDTH                           1  /*       */
#define WM5100_IN4R_MUTE                        0x0100  /*           */
#define WM5100_IN4R_MUTE_MASK                   0x0100  /*           */
#define WM5100_IN4R_MUTE_SHIFT                       8  /*           */
#define WM5100_IN4R_MUTE_WIDTH                       1  /*           */
#define WM5100_IN4R_VOL_MASK                    0x00FF  /*                  */
#define WM5100_IN4R_VOL_SHIFT                        0  /*                  */
#define WM5100_IN4R_VOL_WIDTH                        8  /*                  */

/*
                                   
 */
#define WM5100_OUT6L_ENA                        0x0800  /*           */
#define WM5100_OUT6L_ENA_MASK                   0x0800  /*           */
#define WM5100_OUT6L_ENA_SHIFT                      11  /*           */
#define WM5100_OUT6L_ENA_WIDTH                       1  /*           */
#define WM5100_OUT6R_ENA                        0x0400  /*           */
#define WM5100_OUT6R_ENA_MASK                   0x0400  /*           */
#define WM5100_OUT6R_ENA_SHIFT                      10  /*           */
#define WM5100_OUT6R_ENA_WIDTH                       1  /*           */
#define WM5100_OUT5L_ENA                        0x0200  /*           */
#define WM5100_OUT5L_ENA_MASK                   0x0200  /*           */
#define WM5100_OUT5L_ENA_SHIFT                       9  /*           */
#define WM5100_OUT5L_ENA_WIDTH                       1  /*           */
#define WM5100_OUT5R_ENA                        0x0100  /*           */
#define WM5100_OUT5R_ENA_MASK                   0x0100  /*           */
#define WM5100_OUT5R_ENA_SHIFT                       8  /*           */
#define WM5100_OUT5R_ENA_WIDTH                       1  /*           */
#define WM5100_OUT4L_ENA                        0x0080  /*           */
#define WM5100_OUT4L_ENA_MASK                   0x0080  /*           */
#define WM5100_OUT4L_ENA_SHIFT                       7  /*           */
#define WM5100_OUT4L_ENA_WIDTH                       1  /*           */
#define WM5100_OUT4R_ENA                        0x0040  /*           */
#define WM5100_OUT4R_ENA_MASK                   0x0040  /*           */
#define WM5100_OUT4R_ENA_SHIFT                       6  /*           */
#define WM5100_OUT4R_ENA_WIDTH                       1  /*           */

/*
                                  
 */
#define WM5100_OUT3L_ENA_STS                    0x0020  /*               */
#define WM5100_OUT3L_ENA_STS_MASK               0x0020  /*               */
#define WM5100_OUT3L_ENA_STS_SHIFT                   5  /*               */
#define WM5100_OUT3L_ENA_STS_WIDTH                   1  /*               */
#define WM5100_OUT3R_ENA_STS                    0x0010  /*               */
#define WM5100_OUT3R_ENA_STS_MASK               0x0010  /*               */
#define WM5100_OUT3R_ENA_STS_SHIFT                   4  /*               */
#define WM5100_OUT3R_ENA_STS_WIDTH                   1  /*               */
#define WM5100_OUT2L_ENA_STS                    0x0008  /*               */
#define WM5100_OUT2L_ENA_STS_MASK               0x0008  /*               */
#define WM5100_OUT2L_ENA_STS_SHIFT                   3  /*               */
#define WM5100_OUT2L_ENA_STS_WIDTH                   1  /*               */
#define WM5100_OUT2R_ENA_STS                    0x0004  /*               */
#define WM5100_OUT2R_ENA_STS_MASK               0x0004  /*               */
#define WM5100_OUT2R_ENA_STS_SHIFT                   2  /*               */
#define WM5100_OUT2R_ENA_STS_WIDTH                   1  /*               */
#define WM5100_OUT1L_ENA_STS                    0x0002  /*               */
#define WM5100_OUT1L_ENA_STS_MASK               0x0002  /*               */
#define WM5100_OUT1L_ENA_STS_SHIFT                   1  /*               */
#define WM5100_OUT1L_ENA_STS_WIDTH                   1  /*               */
#define WM5100_OUT1R_ENA_STS                    0x0001  /*               */
#define WM5100_OUT1R_ENA_STS_MASK               0x0001  /*               */
#define WM5100_OUT1R_ENA_STS_SHIFT                   0  /*               */
#define WM5100_OUT1R_ENA_STS_WIDTH                   1  /*               */

/*
                                  
 */
#define WM5100_OUT6L_ENA_STS                    0x0800  /*               */
#define WM5100_OUT6L_ENA_STS_MASK               0x0800  /*               */
#define WM5100_OUT6L_ENA_STS_SHIFT                  11  /*               */
#define WM5100_OUT6L_ENA_STS_WIDTH                   1  /*               */
#define WM5100_OUT6R_ENA_STS                    0x0400  /*               */
#define WM5100_OUT6R_ENA_STS_MASK               0x0400  /*               */
#define WM5100_OUT6R_ENA_STS_SHIFT                  10  /*               */
#define WM5100_OUT6R_ENA_STS_WIDTH                   1  /*               */
#define WM5100_OUT5L_ENA_STS                    0x0200  /*               */
#define WM5100_OUT5L_ENA_STS_MASK               0x0200  /*               */
#define WM5100_OUT5L_ENA_STS_SHIFT                   9  /*               */
#define WM5100_OUT5L_ENA_STS_WIDTH                   1  /*               */
#define WM5100_OUT5R_ENA_STS                    0x0100  /*               */
#define WM5100_OUT5R_ENA_STS_MASK               0x0100  /*               */
#define WM5100_OUT5R_ENA_STS_SHIFT                   8  /*               */
#define WM5100_OUT5R_ENA_STS_WIDTH                   1  /*               */
#define WM5100_OUT4L_ENA_STS                    0x0080  /*               */
#define WM5100_OUT4L_ENA_STS_MASK               0x0080  /*               */
#define WM5100_OUT4L_ENA_STS_SHIFT                   7  /*               */
#define WM5100_OUT4L_ENA_STS_WIDTH                   1  /*               */
#define WM5100_OUT4R_ENA_STS                    0x0040  /*               */
#define WM5100_OUT4R_ENA_STS_MASK               0x0040  /*               */
#define WM5100_OUT4R_ENA_STS_SHIFT                   6  /*               */
#define WM5100_OUT4R_ENA_STS_WIDTH                   1  /*               */

/*
                                    
 */
#define WM5100_HP3L_ENA                         0x0020  /*          */
#define WM5100_HP3L_ENA_MASK                    0x0020  /*          */
#define WM5100_HP3L_ENA_SHIFT                        5  /*          */
#define WM5100_HP3L_ENA_WIDTH                        1  /*          */
#define WM5100_HP3R_ENA                         0x0010  /*          */
#define WM5100_HP3R_ENA_MASK                    0x0010  /*          */
#define WM5100_HP3R_ENA_SHIFT                        4  /*          */
#define WM5100_HP3R_ENA_WIDTH                        1  /*          */
#define WM5100_HP2L_ENA                         0x0008  /*          */
#define WM5100_HP2L_ENA_MASK                    0x0008  /*          */
#define WM5100_HP2L_ENA_SHIFT                        3  /*          */
#define WM5100_HP2L_ENA_WIDTH                        1  /*          */
#define WM5100_HP2R_ENA                         0x0004  /*          */
#define WM5100_HP2R_ENA_MASK                    0x0004  /*          */
#define WM5100_HP2R_ENA_SHIFT                        2  /*          */
#define WM5100_HP2R_ENA_WIDTH                        1  /*          */
#define WM5100_HP1L_ENA                         0x0002  /*          */
#define WM5100_HP1L_ENA_MASK                    0x0002  /*          */
#define WM5100_HP1L_ENA_SHIFT                        1  /*          */
#define WM5100_HP1L_ENA_WIDTH                        1  /*          */
#define WM5100_HP1R_ENA                         0x0001  /*          */
#define WM5100_HP1R_ENA_MASK                    0x0001  /*          */
#define WM5100_HP1R_ENA_SHIFT                        0  /*          */
#define WM5100_HP1R_ENA_WIDTH                        1  /*          */

/*
                                
 */
#define WM5100_OUT_RATE_MASK                    0xC000  /*                    */
#define WM5100_OUT_RATE_SHIFT                       14  /*                    */
#define WM5100_OUT_RATE_WIDTH                        2  /*                    */
#define WM5100_OUT1_OSR                         0x2000  /*          */
#define WM5100_OUT1_OSR_MASK                    0x2000  /*          */
#define WM5100_OUT1_OSR_SHIFT                       13  /*          */
#define WM5100_OUT1_OSR_WIDTH                        1  /*          */
#define WM5100_OUT1_MONO                        0x1000  /*           */
#define WM5100_OUT1_MONO_MASK                   0x1000  /*           */
#define WM5100_OUT1_MONO_SHIFT                      12  /*           */
#define WM5100_OUT1_MONO_WIDTH                       1  /*           */
#define WM5100_OUT1L_ANC_SRC                    0x0800  /*               */
#define WM5100_OUT1L_ANC_SRC_MASK               0x0800  /*               */
#define WM5100_OUT1L_ANC_SRC_SHIFT                  11  /*               */
#define WM5100_OUT1L_ANC_SRC_WIDTH                   1  /*               */
#define WM5100_OUT1L_PGA_VOL_MASK               0x00FE  /*                       */
#define WM5100_OUT1L_PGA_VOL_SHIFT                   1  /*                       */
#define WM5100_OUT1L_PGA_VOL_WIDTH                   7  /*                       */

/*
                                
 */
#define WM5100_OUT1R_ANC_SRC                    0x0800  /*               */
#define WM5100_OUT1R_ANC_SRC_MASK               0x0800  /*               */
#define WM5100_OUT1R_ANC_SRC_SHIFT                  11  /*               */
#define WM5100_OUT1R_ANC_SRC_WIDTH                   1  /*               */
#define WM5100_OUT1R_PGA_VOL_MASK               0x00FE  /*                       */
#define WM5100_OUT1R_PGA_VOL_SHIFT                   1  /*                       */
#define WM5100_OUT1R_PGA_VOL_WIDTH                   7  /*                       */

/*
                                      
 */
#define WM5100_OUT1L_VOL_LIM_MASK               0x00FF  /*                       */
#define WM5100_OUT1L_VOL_LIM_SHIFT                   0  /*                       */
#define WM5100_OUT1L_VOL_LIM_WIDTH                   8  /*                       */

/*
                                      
 */
#define WM5100_OUT1R_VOL_LIM_MASK               0x00FF  /*                       */
#define WM5100_OUT1R_VOL_LIM_SHIFT                   0  /*                       */
#define WM5100_OUT1R_VOL_LIM_WIDTH                   8  /*                       */

/*
                                
 */
#define WM5100_OUT2_OSR                         0x2000  /*          */
#define WM5100_OUT2_OSR_MASK                    0x2000  /*          */
#define WM5100_OUT2_OSR_SHIFT                       13  /*          */
#define WM5100_OUT2_OSR_WIDTH                        1  /*          */
#define WM5100_OUT2_MONO                        0x1000  /*           */
#define WM5100_OUT2_MONO_MASK                   0x1000  /*           */
#define WM5100_OUT2_MONO_SHIFT                      12  /*           */
#define WM5100_OUT2_MONO_WIDTH                       1  /*           */
#define WM5100_OUT2L_ANC_SRC                    0x0800  /*               */
#define WM5100_OUT2L_ANC_SRC_MASK               0x0800  /*               */
#define WM5100_OUT2L_ANC_SRC_SHIFT                  11  /*               */
#define WM5100_OUT2L_ANC_SRC_WIDTH                   1  /*               */
#define WM5100_OUT2L_PGA_VOL_MASK               0x00FE  /*                       */
#define WM5100_OUT2L_PGA_VOL_SHIFT                   1  /*                       */
#define WM5100_OUT2L_PGA_VOL_WIDTH                   7  /*                       */

/*
                                
 */
#define WM5100_OUT2R_ANC_SRC                    0x0800  /*               */
#define WM5100_OUT2R_ANC_SRC_MASK               0x0800  /*               */
#define WM5100_OUT2R_ANC_SRC_SHIFT                  11  /*               */
#define WM5100_OUT2R_ANC_SRC_WIDTH                   1  /*               */
#define WM5100_OUT2R_PGA_VOL_MASK               0x00FE  /*                       */
#define WM5100_OUT2R_PGA_VOL_SHIFT                   1  /*                       */
#define WM5100_OUT2R_PGA_VOL_WIDTH                   7  /*                       */

/*
                                      
 */
#define WM5100_OUT2L_VOL_LIM_MASK               0x00FF  /*                       */
#define WM5100_OUT2L_VOL_LIM_SHIFT                   0  /*                       */
#define WM5100_OUT2L_VOL_LIM_WIDTH                   8  /*                       */

/*
                                      
 */
#define WM5100_OUT2R_VOL_LIM_MASK               0x00FF  /*                       */
#define WM5100_OUT2R_VOL_LIM_SHIFT                   0  /*                       */
#define WM5100_OUT2R_VOL_LIM_WIDTH                   8  /*                       */

/*
                                
 */
#define WM5100_OUT3_OSR                         0x2000  /*          */
#define WM5100_OUT3_OSR_MASK                    0x2000  /*          */
#define WM5100_OUT3_OSR_SHIFT                       13  /*          */
#define WM5100_OUT3_OSR_WIDTH                        1  /*          */
#define WM5100_OUT3_MONO                        0x1000  /*           */
#define WM5100_OUT3_MONO_MASK                   0x1000  /*           */
#define WM5100_OUT3_MONO_SHIFT                      12  /*           */
#define WM5100_OUT3_MONO_WIDTH                       1  /*           */
#define WM5100_OUT3L_ANC_SRC                    0x0800  /*               */
#define WM5100_OUT3L_ANC_SRC_MASK               0x0800  /*               */
#define WM5100_OUT3L_ANC_SRC_SHIFT                  11  /*               */
#define WM5100_OUT3L_ANC_SRC_WIDTH                   1  /*               */
#define WM5100_OUT3L_PGA_VOL_MASK               0x00FE  /*                       */
#define WM5100_OUT3L_PGA_VOL_SHIFT                   1  /*                       */
#define WM5100_OUT3L_PGA_VOL_WIDTH                   7  /*                       */

/*
                                
 */
#define WM5100_OUT3R_ANC_SRC                    0x0800  /*               */
#define WM5100_OUT3R_ANC_SRC_MASK               0x0800  /*               */
#define WM5100_OUT3R_ANC_SRC_SHIFT                  11  /*               */
#define WM5100_OUT3R_ANC_SRC_WIDTH                   1  /*               */
#define WM5100_OUT3R_PGA_VOL_MASK               0x00FE  /*                       */
#define WM5100_OUT3R_PGA_VOL_SHIFT                   1  /*                       */
#define WM5100_OUT3R_PGA_VOL_WIDTH                   7  /*                       */

/*
                                      
 */
#define WM5100_OUT3L_VOL_LIM_MASK               0x00FF  /*                       */
#define WM5100_OUT3L_VOL_LIM_SHIFT                   0  /*                       */
#define WM5100_OUT3L_VOL_LIM_WIDTH                   8  /*                       */

/*
                                      
 */
#define WM5100_OUT3R_VOL_LIM_MASK               0x00FF  /*                       */
#define WM5100_OUT3R_VOL_LIM_SHIFT                   0  /*                       */
#define WM5100_OUT3R_VOL_LIM_WIDTH                   8  /*                       */

/*
                                
 */
#define WM5100_OUT4_OSR                         0x2000  /*          */
#define WM5100_OUT4_OSR_MASK                    0x2000  /*          */
#define WM5100_OUT4_OSR_SHIFT                       13  /*          */
#define WM5100_OUT4_OSR_WIDTH                        1  /*          */
#define WM5100_OUT4L_ANC_SRC                    0x0800  /*               */
#define WM5100_OUT4L_ANC_SRC_MASK               0x0800  /*               */
#define WM5100_OUT4L_ANC_SRC_SHIFT                  11  /*               */
#define WM5100_OUT4L_ANC_SRC_WIDTH                   1  /*               */
#define WM5100_OUT4L_VOL_LIM_MASK               0x00FF  /*                       */
#define WM5100_OUT4L_VOL_LIM_SHIFT                   0  /*                       */
#define WM5100_OUT4L_VOL_LIM_WIDTH                   8  /*                       */

/*
                                
 */
#define WM5100_OUT4R_ANC_SRC                    0x0800  /*               */
#define WM5100_OUT4R_ANC_SRC_MASK               0x0800  /*               */
#define WM5100_OUT4R_ANC_SRC_SHIFT                  11  /*               */
#define WM5100_OUT4R_ANC_SRC_WIDTH                   1  /*               */
#define WM5100_OUT4R_VOL_LIM_MASK               0x00FF  /*                       */
#define WM5100_OUT4R_VOL_LIM_SHIFT                   0  /*                       */
#define WM5100_OUT4R_VOL_LIM_WIDTH                   8  /*                       */

/*
                                      
 */
#define WM5100_OUT5_OSR                         0x2000  /*          */
#define WM5100_OUT5_OSR_MASK                    0x2000  /*          */
#define WM5100_OUT5_OSR_SHIFT                       13  /*          */
#define WM5100_OUT5_OSR_WIDTH                        1  /*          */
#define WM5100_OUT5L_ANC_SRC                    0x0800  /*               */
#define WM5100_OUT5L_ANC_SRC_MASK               0x0800  /*               */
#define WM5100_OUT5L_ANC_SRC_SHIFT                  11  /*               */
#define WM5100_OUT5L_ANC_SRC_WIDTH                   1  /*               */
#define WM5100_OUT5L_VOL_LIM_MASK               0x00FF  /*                       */
#define WM5100_OUT5L_VOL_LIM_SHIFT                   0  /*                       */
#define WM5100_OUT5L_VOL_LIM_WIDTH                   8  /*                       */

/*
                                      
 */
#define WM5100_OUT5R_ANC_SRC                    0x0800  /*               */
#define WM5100_OUT5R_ANC_SRC_MASK               0x0800  /*               */
#define WM5100_OUT5R_ANC_SRC_SHIFT                  11  /*               */
#define WM5100_OUT5R_ANC_SRC_WIDTH                   1  /*               */
#define WM5100_OUT5R_VOL_LIM_MASK               0x00FF  /*                       */
#define WM5100_OUT5R_VOL_LIM_SHIFT                   0  /*                       */
#define WM5100_OUT5R_VOL_LIM_WIDTH                   8  /*                       */

/*
                                      
 */
#define WM5100_OUT6_OSR                         0x2000  /*          */
#define WM5100_OUT6_OSR_MASK                    0x2000  /*          */
#define WM5100_OUT6_OSR_SHIFT                       13  /*          */
#define WM5100_OUT6_OSR_WIDTH                        1  /*          */
#define WM5100_OUT6L_ANC_SRC                    0x0800  /*               */
#define WM5100_OUT6L_ANC_SRC_MASK               0x0800  /*               */
#define WM5100_OUT6L_ANC_SRC_SHIFT                  11  /*               */
#define WM5100_OUT6L_ANC_SRC_WIDTH                   1  /*               */
#define WM5100_OUT6L_VOL_LIM_MASK               0x00FF  /*                       */
#define WM5100_OUT6L_VOL_LIM_SHIFT                   0  /*                       */
#define WM5100_OUT6L_VOL_LIM_WIDTH                   8  /*                       */

/*
                                      
 */
#define WM5100_OUT6R_ANC_SRC                    0x0800  /*               */
#define WM5100_OUT6R_ANC_SRC_MASK               0x0800  /*               */
#define WM5100_OUT6R_ANC_SRC_SHIFT                  11  /*               */
#define WM5100_OUT6R_ANC_SRC_WIDTH                   1  /*               */
#define WM5100_OUT6R_VOL_LIM_MASK               0x00FF  /*                       */
#define WM5100_OUT6R_VOL_LIM_SHIFT                   0  /*                       */
#define WM5100_OUT6R_VOL_LIM_WIDTH                   8  /*                       */

/*
                                    
 */
#define WM5100_AEC_LOOPBACK_SRC_MASK            0x003C  /*                          */
#define WM5100_AEC_LOOPBACK_SRC_SHIFT                2  /*                          */
#define WM5100_AEC_LOOPBACK_SRC_WIDTH                4  /*                          */
#define WM5100_AEC_ENA_STS                      0x0002  /*             */
#define WM5100_AEC_ENA_STS_MASK                 0x0002  /*             */
#define WM5100_AEC_ENA_STS_SHIFT                     1  /*             */
#define WM5100_AEC_ENA_STS_WIDTH                     1  /*             */
#define WM5100_AEC_LOOPBACK_ENA                 0x0001  /*                  */
#define WM5100_AEC_LOOPBACK_ENA_MASK            0x0001  /*                  */
#define WM5100_AEC_LOOPBACK_ENA_SHIFT                0  /*                  */
#define WM5100_AEC_LOOPBACK_ENA_WIDTH                1  /*                  */

/*
                                     
 */
#define WM5100_OUT_VD_RAMP_MASK                 0x0070  /*                     */
#define WM5100_OUT_VD_RAMP_SHIFT                     4  /*                     */
#define WM5100_OUT_VD_RAMP_WIDTH                     3  /*                     */
#define WM5100_OUT_VI_RAMP_MASK                 0x0007  /*                     */
#define WM5100_OUT_VI_RAMP_SHIFT                     0  /*                     */
#define WM5100_OUT_VI_RAMP_WIDTH                     3  /*                     */

/*
                                        
 */
#define WM5100_OUT_VU                           0x0200  /*        */
#define WM5100_OUT_VU_MASK                      0x0200  /*        */
#define WM5100_OUT_VU_SHIFT                          9  /*        */
#define WM5100_OUT_VU_WIDTH                          1  /*        */
#define WM5100_OUT1L_MUTE                       0x0100  /*            */
#define WM5100_OUT1L_MUTE_MASK                  0x0100  /*            */
#define WM5100_OUT1L_MUTE_SHIFT                      8  /*            */
#define WM5100_OUT1L_MUTE_WIDTH                      1  /*            */
#define WM5100_OUT1L_VOL_MASK                   0x00FF  /*                   */
#define WM5100_OUT1L_VOL_SHIFT                       0  /*                   */
#define WM5100_OUT1L_VOL_WIDTH                       8  /*                   */

/*
                                        
 */
#define WM5100_OUT_VU                           0x0200  /*        */
#define WM5100_OUT_VU_MASK                      0x0200  /*        */
#define WM5100_OUT_VU_SHIFT                          9  /*        */
#define WM5100_OUT_VU_WIDTH                          1  /*        */
#define WM5100_OUT1R_MUTE                       0x0100  /*            */
#define WM5100_OUT1R_MUTE_MASK                  0x0100  /*            */
#define WM5100_OUT1R_MUTE_SHIFT                      8  /*            */
#define WM5100_OUT1R_MUTE_WIDTH                      1  /*            */
#define WM5100_OUT1R_VOL_MASK                   0x00FF  /*                   */
#define WM5100_OUT1R_VOL_SHIFT                       0  /*                   */
#define WM5100_OUT1R_VOL_WIDTH                       8  /*                   */

/*
                                        
 */
#define WM5100_OUT_VU                           0x0200  /*        */
#define WM5100_OUT_VU_MASK                      0x0200  /*        */
#define WM5100_OUT_VU_SHIFT                          9  /*        */
#define WM5100_OUT_VU_WIDTH                          1  /*        */
#define WM5100_OUT2L_MUTE                       0x0100  /*            */
#define WM5100_OUT2L_MUTE_MASK                  0x0100  /*            */
#define WM5100_OUT2L_MUTE_SHIFT                      8  /*            */
#define WM5100_OUT2L_MUTE_WIDTH                      1  /*            */
#define WM5100_OUT2L_VOL_MASK                   0x00FF  /*                   */
#define WM5100_OUT2L_VOL_SHIFT                       0  /*                   */
#define WM5100_OUT2L_VOL_WIDTH                       8  /*                   */

/*
                                        
 */
#define WM5100_OUT_VU                           0x0200  /*        */
#define WM5100_OUT_VU_MASK                      0x0200  /*        */
#define WM5100_OUT_VU_SHIFT                          9  /*        */
#define WM5100_OUT_VU_WIDTH                          1  /*        */
#define WM5100_OUT2R_MUTE                       0x0100  /*            */
#define WM5100_OUT2R_MUTE_MASK                  0x0100  /*            */
#define WM5100_OUT2R_MUTE_SHIFT                      8  /*            */
#define WM5100_OUT2R_MUTE_WIDTH                      1  /*            */
#define WM5100_OUT2R_VOL_MASK                   0x00FF  /*                   */
#define WM5100_OUT2R_VOL_SHIFT                       0  /*                   */
#define WM5100_OUT2R_VOL_WIDTH                       8  /*                   */

/*
                                        
 */
#define WM5100_OUT_VU                           0x0200  /*        */
#define WM5100_OUT_VU_MASK                      0x0200  /*        */
#define WM5100_OUT_VU_SHIFT                          9  /*        */
#define WM5100_OUT_VU_WIDTH                          1  /*        */
#define WM5100_OUT3L_MUTE                       0x0100  /*            */
#define WM5100_OUT3L_MUTE_MASK                  0x0100  /*            */
#define WM5100_OUT3L_MUTE_SHIFT                      8  /*            */
#define WM5100_OUT3L_MUTE_WIDTH                      1  /*            */
#define WM5100_OUT3L_VOL_MASK                   0x00FF  /*                   */
#define WM5100_OUT3L_VOL_SHIFT                       0  /*                   */
#define WM5100_OUT3L_VOL_WIDTH                       8  /*                   */

/*
                                        
 */
#define WM5100_OUT_VU                           0x0200  /*        */
#define WM5100_OUT_VU_MASK                      0x0200  /*        */
#define WM5100_OUT_VU_SHIFT                          9  /*        */
#define WM5100_OUT_VU_WIDTH                          1  /*        */
#define WM5100_OUT3R_MUTE                       0x0100  /*            */
#define WM5100_OUT3R_MUTE_MASK                  0x0100  /*            */
#define WM5100_OUT3R_MUTE_SHIFT                      8  /*            */
#define WM5100_OUT3R_MUTE_WIDTH                      1  /*            */
#define WM5100_OUT3R_VOL_MASK                   0x00FF  /*                   */
#define WM5100_OUT3R_VOL_SHIFT                       0  /*                   */
#define WM5100_OUT3R_VOL_WIDTH                       8  /*                   */

/*
                                        
 */
#define WM5100_OUT_VU                           0x0200  /*        */
#define WM5100_OUT_VU_MASK                      0x0200  /*        */
#define WM5100_OUT_VU_SHIFT                          9  /*        */
#define WM5100_OUT_VU_WIDTH                          1  /*        */
#define WM5100_OUT4L_MUTE                       0x0100  /*            */
#define WM5100_OUT4L_MUTE_MASK                  0x0100  /*            */
#define WM5100_OUT4L_MUTE_SHIFT                      8  /*            */
#define WM5100_OUT4L_MUTE_WIDTH                      1  /*            */
#define WM5100_OUT4L_VOL_MASK                   0x00FF  /*                   */
#define WM5100_OUT4L_VOL_SHIFT                       0  /*                   */
#define WM5100_OUT4L_VOL_WIDTH                       8  /*                   */

/*
                                        
 */
#define WM5100_OUT_VU                           0x0200  /*        */
#define WM5100_OUT_VU_MASK                      0x0200  /*        */
#define WM5100_OUT_VU_SHIFT                          9  /*        */
#define WM5100_OUT_VU_WIDTH                          1  /*        */
#define WM5100_OUT4R_MUTE                       0x0100  /*            */
#define WM5100_OUT4R_MUTE_MASK                  0x0100  /*            */
#define WM5100_OUT4R_MUTE_SHIFT                      8  /*            */
#define WM5100_OUT4R_MUTE_WIDTH                      1  /*            */
#define WM5100_OUT4R_VOL_MASK                   0x00FF  /*                   */
#define WM5100_OUT4R_VOL_SHIFT                       0  /*                   */
#define WM5100_OUT4R_VOL_WIDTH                       8  /*                   */

/*
                                        
 */
#define WM5100_OUT_VU                           0x0200  /*        */
#define WM5100_OUT_VU_MASK                      0x0200  /*        */
#define WM5100_OUT_VU_SHIFT                          9  /*        */
#define WM5100_OUT_VU_WIDTH                          1  /*        */
#define WM5100_OUT5L_MUTE                       0x0100  /*            */
#define WM5100_OUT5L_MUTE_MASK                  0x0100  /*            */
#define WM5100_OUT5L_MUTE_SHIFT                      8  /*            */
#define WM5100_OUT5L_MUTE_WIDTH                      1  /*            */
#define WM5100_OUT5L_VOL_MASK                   0x00FF  /*                   */
#define WM5100_OUT5L_VOL_SHIFT                       0  /*                   */
#define WM5100_OUT5L_VOL_WIDTH                       8  /*                   */

/*
                                        
 */
#define WM5100_OUT_VU                           0x0200  /*        */
#define WM5100_OUT_VU_MASK                      0x0200  /*        */
#define WM5100_OUT_VU_SHIFT                          9  /*        */
#define WM5100_OUT_VU_WIDTH                          1  /*        */
#define WM5100_OUT5R_MUTE                       0x0100  /*            */
#define WM5100_OUT5R_MUTE_MASK                  0x0100  /*            */
#define WM5100_OUT5R_MUTE_SHIFT                      8  /*            */
#define WM5100_OUT5R_MUTE_WIDTH                      1  /*            */
#define WM5100_OUT5R_VOL_MASK                   0x00FF  /*                   */
#define WM5100_OUT5R_VOL_SHIFT                       0  /*                   */
#define WM5100_OUT5R_VOL_WIDTH                       8  /*                   */

/*
                                        
 */
#define WM5100_OUT_VU                           0x0200  /*        */
#define WM5100_OUT_VU_MASK                      0x0200  /*        */
#define WM5100_OUT_VU_SHIFT                          9  /*        */
#define WM5100_OUT_VU_WIDTH                          1  /*        */
#define WM5100_OUT6L_MUTE                       0x0100  /*            */
#define WM5100_OUT6L_MUTE_MASK                  0x0100  /*            */
#define WM5100_OUT6L_MUTE_SHIFT                      8  /*            */
#define WM5100_OUT6L_MUTE_WIDTH                      1  /*            */
#define WM5100_OUT6L_VOL_MASK                   0x00FF  /*                   */
#define WM5100_OUT6L_VOL_SHIFT                       0  /*                   */
#define WM5100_OUT6L_VOL_WIDTH                       8  /*                   */

/*
                                        
 */
#define WM5100_OUT_VU                           0x0200  /*        */
#define WM5100_OUT_VU_MASK                      0x0200  /*        */
#define WM5100_OUT_VU_SHIFT                          9  /*        */
#define WM5100_OUT_VU_WIDTH                          1  /*        */
#define WM5100_OUT6R_MUTE                       0x0100  /*            */
#define WM5100_OUT6R_MUTE_MASK                  0x0100  /*            */
#define WM5100_OUT6R_MUTE_SHIFT                      8  /*            */
#define WM5100_OUT6R_MUTE_WIDTH                      1  /*            */
#define WM5100_OUT6R_VOL_MASK                   0x00FF  /*                   */
#define WM5100_OUT6R_VOL_SHIFT                       0  /*                   */
#define WM5100_OUT6R_VOL_WIDTH                       8  /*                   */

/*
                                  
 */
#define WM5100_SPK1R_MUTE                       0x2000  /*            */
#define WM5100_SPK1R_MUTE_MASK                  0x2000  /*            */
#define WM5100_SPK1R_MUTE_SHIFT                     13  /*            */
#define WM5100_SPK1R_MUTE_WIDTH                      1  /*            */
#define WM5100_SPK1L_MUTE                       0x1000  /*            */
#define WM5100_SPK1L_MUTE_MASK                  0x1000  /*            */
#define WM5100_SPK1L_MUTE_SHIFT                     12  /*            */
#define WM5100_SPK1L_MUTE_WIDTH                      1  /*            */
#define WM5100_SPK1_MUTE_ENDIAN                 0x0100  /*                  */
#define WM5100_SPK1_MUTE_ENDIAN_MASK            0x0100  /*                  */
#define WM5100_SPK1_MUTE_ENDIAN_SHIFT                8  /*                  */
#define WM5100_SPK1_MUTE_ENDIAN_WIDTH                1  /*                  */
#define WM5100_SPK1_MUTE_SEQ1_MASK              0x00FF  /*                        */
#define WM5100_SPK1_MUTE_SEQ1_SHIFT                  0  /*                        */
#define WM5100_SPK1_MUTE_SEQ1_WIDTH                  8  /*                        */

/*
                                  
 */
#define WM5100_SPK1_FMT                         0x0001  /*          */
#define WM5100_SPK1_FMT_MASK                    0x0001  /*          */
#define WM5100_SPK1_FMT_SHIFT                        0  /*          */
#define WM5100_SPK1_FMT_WIDTH                        1  /*          */

/*
                                  
 */
#define WM5100_SPK2R_MUTE                       0x2000  /*            */
#define WM5100_SPK2R_MUTE_MASK                  0x2000  /*            */
#define WM5100_SPK2R_MUTE_SHIFT                     13  /*            */
#define WM5100_SPK2R_MUTE_WIDTH                      1  /*            */
#define WM5100_SPK2L_MUTE                       0x1000  /*            */
#define WM5100_SPK2L_MUTE_MASK                  0x1000  /*            */
#define WM5100_SPK2L_MUTE_SHIFT                     12  /*            */
#define WM5100_SPK2L_MUTE_WIDTH                      1  /*            */
#define WM5100_SPK2_MUTE_ENDIAN                 0x0100  /*                  */
#define WM5100_SPK2_MUTE_ENDIAN_MASK            0x0100  /*                  */
#define WM5100_SPK2_MUTE_ENDIAN_SHIFT                8  /*                  */
#define WM5100_SPK2_MUTE_ENDIAN_WIDTH                1  /*                  */
#define WM5100_SPK2_MUTE_SEQ1_MASK              0x00FF  /*                        */
#define WM5100_SPK2_MUTE_SEQ1_SHIFT                  0  /*                        */
#define WM5100_SPK2_MUTE_SEQ1_WIDTH                  8  /*                        */

/*
                                  
 */
#define WM5100_SPK2_FMT                         0x0001  /*          */
#define WM5100_SPK2_FMT_MASK                    0x0001  /*          */
#define WM5100_SPK2_FMT_SHIFT                        0  /*          */
#define WM5100_SPK2_FMT_WIDTH                        1  /*          */

/*
                               
 */
#define WM5100_AIF1_BCLK_INV                    0x0080  /*               */
#define WM5100_AIF1_BCLK_INV_MASK               0x0080  /*               */
#define WM5100_AIF1_BCLK_INV_SHIFT                   7  /*               */
#define WM5100_AIF1_BCLK_INV_WIDTH                   1  /*               */
#define WM5100_AIF1_BCLK_FRC                    0x0040  /*               */
#define WM5100_AIF1_BCLK_FRC_MASK               0x0040  /*               */
#define WM5100_AIF1_BCLK_FRC_SHIFT                   6  /*               */
#define WM5100_AIF1_BCLK_FRC_WIDTH                   1  /*               */
#define WM5100_AIF1_BCLK_MSTR                   0x0020  /*                */
#define WM5100_AIF1_BCLK_MSTR_MASK              0x0020  /*                */
#define WM5100_AIF1_BCLK_MSTR_SHIFT                  5  /*                */
#define WM5100_AIF1_BCLK_MSTR_WIDTH                  1  /*                */
#define WM5100_AIF1_BCLK_FREQ_MASK              0x001F  /*                        */
#define WM5100_AIF1_BCLK_FREQ_SHIFT                  0  /*                        */
#define WM5100_AIF1_BCLK_FREQ_WIDTH                  5  /*                        */

/*
                               
 */
#define WM5100_AIF1TX_DAT_TRI                   0x0020  /*                */
#define WM5100_AIF1TX_DAT_TRI_MASK              0x0020  /*                */
#define WM5100_AIF1TX_DAT_TRI_SHIFT                  5  /*                */
#define WM5100_AIF1TX_DAT_TRI_WIDTH                  1  /*                */
#define WM5100_AIF1TX_LRCLK_SRC                 0x0008  /*                  */
#define WM5100_AIF1TX_LRCLK_SRC_MASK            0x0008  /*                  */
#define WM5100_AIF1TX_LRCLK_SRC_SHIFT                3  /*                  */
#define WM5100_AIF1TX_LRCLK_SRC_WIDTH                1  /*                  */
#define WM5100_AIF1TX_LRCLK_INV                 0x0004  /*                  */
#define WM5100_AIF1TX_LRCLK_INV_MASK            0x0004  /*                  */
#define WM5100_AIF1TX_LRCLK_INV_SHIFT                2  /*                  */
#define WM5100_AIF1TX_LRCLK_INV_WIDTH                1  /*                  */
#define WM5100_AIF1TX_LRCLK_FRC                 0x0002  /*                  */
#define WM5100_AIF1TX_LRCLK_FRC_MASK            0x0002  /*                  */
#define WM5100_AIF1TX_LRCLK_FRC_SHIFT                1  /*                  */
#define WM5100_AIF1TX_LRCLK_FRC_WIDTH                1  /*                  */
#define WM5100_AIF1TX_LRCLK_MSTR                0x0001  /*                   */
#define WM5100_AIF1TX_LRCLK_MSTR_MASK           0x0001  /*                   */
#define WM5100_AIF1TX_LRCLK_MSTR_SHIFT               0  /*                   */
#define WM5100_AIF1TX_LRCLK_MSTR_WIDTH               1  /*                   */

/*
                               
 */
#define WM5100_AIF1RX_LRCLK_INV                 0x0004  /*                  */
#define WM5100_AIF1RX_LRCLK_INV_MASK            0x0004  /*                  */
#define WM5100_AIF1RX_LRCLK_INV_SHIFT                2  /*                  */
#define WM5100_AIF1RX_LRCLK_INV_WIDTH                1  /*                  */
#define WM5100_AIF1RX_LRCLK_FRC                 0x0002  /*                  */
#define WM5100_AIF1RX_LRCLK_FRC_MASK            0x0002  /*                  */
#define WM5100_AIF1RX_LRCLK_FRC_SHIFT                1  /*                  */
#define WM5100_AIF1RX_LRCLK_FRC_WIDTH                1  /*                  */
#define WM5100_AIF1RX_LRCLK_MSTR                0x0001  /*                   */
#define WM5100_AIF1RX_LRCLK_MSTR_MASK           0x0001  /*                   */
#define WM5100_AIF1RX_LRCLK_MSTR_SHIFT               0  /*                   */
#define WM5100_AIF1RX_LRCLK_MSTR_WIDTH               1  /*                   */

/*
                               
 */
#define WM5100_AIF1_TRI                         0x0040  /*          */
#define WM5100_AIF1_TRI_MASK                    0x0040  /*          */
#define WM5100_AIF1_TRI_SHIFT                        6  /*          */
#define WM5100_AIF1_TRI_WIDTH                        1  /*          */
#define WM5100_AIF1_RATE_MASK                   0x0003  /*                   */
#define WM5100_AIF1_RATE_SHIFT                       0  /*                   */
#define WM5100_AIF1_RATE_WIDTH                       2  /*                   */

/*
                               
 */
#define WM5100_AIF1_FMT_MASK                    0x0007  /*                  */
#define WM5100_AIF1_FMT_SHIFT                        0  /*                  */
#define WM5100_AIF1_FMT_WIDTH                        3  /*                  */

/*
                               
 */
#define WM5100_AIF1TX_BCPF_MASK                 0x1FFF  /*                      */
#define WM5100_AIF1TX_BCPF_SHIFT                     0  /*                      */
#define WM5100_AIF1TX_BCPF_WIDTH                    13  /*                      */

/*
                               
 */
#define WM5100_AIF1RX_BCPF_MASK                 0x1FFF  /*                      */
#define WM5100_AIF1RX_BCPF_SHIFT                     0  /*                      */
#define WM5100_AIF1RX_BCPF_WIDTH                    13  /*                      */

/*
                               
 */
#define WM5100_AIF1TX_WL_MASK                   0x3F00  /*                    */
#define WM5100_AIF1TX_WL_SHIFT                       8  /*                    */
#define WM5100_AIF1TX_WL_WIDTH                       6  /*                    */
#define WM5100_AIF1TX_SLOT_LEN_MASK             0x00FF  /*                         */
#define WM5100_AIF1TX_SLOT_LEN_SHIFT                 0  /*                         */
#define WM5100_AIF1TX_SLOT_LEN_WIDTH                 8  /*                         */

/*
                               
 */
#define WM5100_AIF1RX_WL_MASK                   0x3F00  /*                    */
#define WM5100_AIF1RX_WL_SHIFT                       8  /*                    */
#define WM5100_AIF1RX_WL_WIDTH                       6  /*                    */
#define WM5100_AIF1RX_SLOT_LEN_MASK             0x00FF  /*                         */
#define WM5100_AIF1RX_SLOT_LEN_SHIFT                 0  /*                         */
#define WM5100_AIF1RX_SLOT_LEN_WIDTH                 8  /*                         */

/*
                                
 */
#define WM5100_AIF1TX1_SLOT_MASK                0x003F  /*                      */
#define WM5100_AIF1TX1_SLOT_SHIFT                    0  /*                      */
#define WM5100_AIF1TX1_SLOT_WIDTH                    6  /*                      */

/*
                                
 */
#define WM5100_AIF1TX2_SLOT_MASK                0x003F  /*                      */
#define WM5100_AIF1TX2_SLOT_SHIFT                    0  /*                      */
#define WM5100_AIF1TX2_SLOT_WIDTH                    6  /*                      */

/*
                                
 */
#define WM5100_AIF1TX3_SLOT_MASK                0x003F  /*                      */
#define WM5100_AIF1TX3_SLOT_SHIFT                    0  /*                      */
#define WM5100_AIF1TX3_SLOT_WIDTH                    6  /*                      */

/*
                                
 */
#define WM5100_AIF1TX4_SLOT_MASK                0x003F  /*                      */
#define WM5100_AIF1TX4_SLOT_SHIFT                    0  /*                      */
#define WM5100_AIF1TX4_SLOT_WIDTH                    6  /*                      */

/*
                                
 */
#define WM5100_AIF1TX5_SLOT_MASK                0x003F  /*                      */
#define WM5100_AIF1TX5_SLOT_SHIFT                    0  /*                      */
#define WM5100_AIF1TX5_SLOT_WIDTH                    6  /*                      */

/*
                                
 */
#define WM5100_AIF1TX6_SLOT_MASK                0x003F  /*                      */
#define WM5100_AIF1TX6_SLOT_SHIFT                    0  /*                      */
#define WM5100_AIF1TX6_SLOT_WIDTH                    6  /*                      */

/*
                                
 */
#define WM5100_AIF1TX7_SLOT_MASK                0x003F  /*                      */
#define WM5100_AIF1TX7_SLOT_SHIFT                    0  /*                      */
#define WM5100_AIF1TX7_SLOT_WIDTH                    6  /*                      */

/*
                                
 */
#define WM5100_AIF1TX8_SLOT_MASK                0x003F  /*                      */
#define WM5100_AIF1TX8_SLOT_SHIFT                    0  /*                      */
#define WM5100_AIF1TX8_SLOT_WIDTH                    6  /*                      */

/*
                                
 */
#define WM5100_AIF1RX1_SLOT_MASK                0x003F  /*                      */
#define WM5100_AIF1RX1_SLOT_SHIFT                    0  /*                      */
#define WM5100_AIF1RX1_SLOT_WIDTH                    6  /*                      */

/*
                                
 */
#define WM5100_AIF1RX2_SLOT_MASK                0x003F  /*                      */
#define WM5100_AIF1RX2_SLOT_SHIFT                    0  /*                      */
#define WM5100_AIF1RX2_SLOT_WIDTH                    6  /*                      */

/*
                                
 */
#define WM5100_AIF1RX3_SLOT_MASK                0x003F  /*                      */
#define WM5100_AIF1RX3_SLOT_SHIFT                    0  /*                      */
#define WM5100_AIF1RX3_SLOT_WIDTH                    6  /*                      */

/*
                                
 */
#define WM5100_AIF1RX4_SLOT_MASK                0x003F  /*                      */
#define WM5100_AIF1RX4_SLOT_SHIFT                    0  /*                      */
#define WM5100_AIF1RX4_SLOT_WIDTH                    6  /*                      */

/*
                                
 */
#define WM5100_AIF1RX5_SLOT_MASK                0x003F  /*                      */
#define WM5100_AIF1RX5_SLOT_SHIFT                    0  /*                      */
#define WM5100_AIF1RX5_SLOT_WIDTH                    6  /*                      */

/*
                                
 */
#define WM5100_AIF1RX6_SLOT_MASK                0x003F  /*                      */
#define WM5100_AIF1RX6_SLOT_SHIFT                    0  /*                      */
#define WM5100_AIF1RX6_SLOT_WIDTH                    6  /*                      */

/*
                                
 */
#define WM5100_AIF1RX7_SLOT_MASK                0x003F  /*                      */
#define WM5100_AIF1RX7_SLOT_SHIFT                    0  /*                      */
#define WM5100_AIF1RX7_SLOT_WIDTH                    6  /*                      */

/*
                                
 */
#define WM5100_AIF1RX8_SLOT_MASK                0x003F  /*                      */
#define WM5100_AIF1RX8_SLOT_SHIFT                    0  /*                      */
#define WM5100_AIF1RX8_SLOT_WIDTH                    6  /*                      */

/*
                                
 */
#define WM5100_AIF1TX8_ENA                      0x0080  /*             */
#define WM5100_AIF1TX8_ENA_MASK                 0x0080  /*             */
#define WM5100_AIF1TX8_ENA_SHIFT                     7  /*             */
#define WM5100_AIF1TX8_ENA_WIDTH                     1  /*             */
#define WM5100_AIF1TX7_ENA                      0x0040  /*             */
#define WM5100_AIF1TX7_ENA_MASK                 0x0040  /*             */
#define WM5100_AIF1TX7_ENA_SHIFT                     6  /*             */
#define WM5100_AIF1TX7_ENA_WIDTH                     1  /*             */
#define WM5100_AIF1TX6_ENA                      0x0020  /*             */
#define WM5100_AIF1TX6_ENA_MASK                 0x0020  /*             */
#define WM5100_AIF1TX6_ENA_SHIFT                     5  /*             */
#define WM5100_AIF1TX6_ENA_WIDTH                     1  /*             */
#define WM5100_AIF1TX5_ENA                      0x0010  /*             */
#define WM5100_AIF1TX5_ENA_MASK                 0x0010  /*             */
#define WM5100_AIF1TX5_ENA_SHIFT                     4  /*             */
#define WM5100_AIF1TX5_ENA_WIDTH                     1  /*             */
#define WM5100_AIF1TX4_ENA                      0x0008  /*             */
#define WM5100_AIF1TX4_ENA_MASK                 0x0008  /*             */
#define WM5100_AIF1TX4_ENA_SHIFT                     3  /*             */
#define WM5100_AIF1TX4_ENA_WIDTH                     1  /*             */
#define WM5100_AIF1TX3_ENA                      0x0004  /*             */
#define WM5100_AIF1TX3_ENA_MASK                 0x0004  /*             */
#define WM5100_AIF1TX3_ENA_SHIFT                     2  /*             */
#define WM5100_AIF1TX3_ENA_WIDTH                     1  /*             */
#define WM5100_AIF1TX2_ENA                      0x0002  /*             */
#define WM5100_AIF1TX2_ENA_MASK                 0x0002  /*             */
#define WM5100_AIF1TX2_ENA_SHIFT                     1  /*             */
#define WM5100_AIF1TX2_ENA_WIDTH                     1  /*             */
#define WM5100_AIF1TX1_ENA                      0x0001  /*             */
#define WM5100_AIF1TX1_ENA_MASK                 0x0001  /*             */
#define WM5100_AIF1TX1_ENA_SHIFT                     0  /*             */
#define WM5100_AIF1TX1_ENA_WIDTH                     1  /*             */

/*
                                
 */
#define WM5100_AIF1RX8_ENA                      0x0080  /*             */
#define WM5100_AIF1RX8_ENA_MASK                 0x0080  /*             */
#define WM5100_AIF1RX8_ENA_SHIFT                     7  /*             */
#define WM5100_AIF1RX8_ENA_WIDTH                     1  /*             */
#define WM5100_AIF1RX7_ENA                      0x0040  /*             */
#define WM5100_AIF1RX7_ENA_MASK                 0x0040  /*             */
#define WM5100_AIF1RX7_ENA_SHIFT                     6  /*             */
#define WM5100_AIF1RX7_ENA_WIDTH                     1  /*             */
#define WM5100_AIF1RX6_ENA                      0x0020  /*             */
#define WM5100_AIF1RX6_ENA_MASK                 0x0020  /*             */
#define WM5100_AIF1RX6_ENA_SHIFT                     5  /*             */
#define WM5100_AIF1RX6_ENA_WIDTH                     1  /*             */
#define WM5100_AIF1RX5_ENA                      0x0010  /*             */
#define WM5100_AIF1RX5_ENA_MASK                 0x0010  /*             */
#define WM5100_AIF1RX5_ENA_SHIFT                     4  /*             */
#define WM5100_AIF1RX5_ENA_WIDTH                     1  /*             */
#define WM5100_AIF1RX4_ENA                      0x0008  /*             */
#define WM5100_AIF1RX4_ENA_MASK                 0x0008  /*             */
#define WM5100_AIF1RX4_ENA_SHIFT                     3  /*             */
#define WM5100_AIF1RX4_ENA_WIDTH                     1  /*             */
#define WM5100_AIF1RX3_ENA                      0x0004  /*             */
#define WM5100_AIF1RX3_ENA_MASK                 0x0004  /*             */
#define WM5100_AIF1RX3_ENA_SHIFT                     2  /*             */
#define WM5100_AIF1RX3_ENA_WIDTH                     1  /*             */
#define WM5100_AIF1RX2_ENA                      0x0002  /*             */
#define WM5100_AIF1RX2_ENA_MASK                 0x0002  /*             */
#define WM5100_AIF1RX2_ENA_SHIFT                     1  /*             */
#define WM5100_AIF1RX2_ENA_WIDTH                     1  /*             */
#define WM5100_AIF1RX1_ENA                      0x0001  /*             */
#define WM5100_AIF1RX1_ENA_MASK                 0x0001  /*             */
#define WM5100_AIF1RX1_ENA_SHIFT                     0  /*             */
#define WM5100_AIF1RX1_ENA_WIDTH                     1  /*             */

/*
                               
 */
#define WM5100_AIF2_BCLK_INV                    0x0080  /*               */
#define WM5100_AIF2_BCLK_INV_MASK               0x0080  /*               */
#define WM5100_AIF2_BCLK_INV_SHIFT                   7  /*               */
#define WM5100_AIF2_BCLK_INV_WIDTH                   1  /*               */
#define WM5100_AIF2_BCLK_FRC                    0x0040  /*               */
#define WM5100_AIF2_BCLK_FRC_MASK               0x0040  /*               */
#define WM5100_AIF2_BCLK_FRC_SHIFT                   6  /*               */
#define WM5100_AIF2_BCLK_FRC_WIDTH                   1  /*               */
#define WM5100_AIF2_BCLK_MSTR                   0x0020  /*                */
#define WM5100_AIF2_BCLK_MSTR_MASK              0x0020  /*                */
#define WM5100_AIF2_BCLK_MSTR_SHIFT                  5  /*                */
#define WM5100_AIF2_BCLK_MSTR_WIDTH                  1  /*                */
#define WM5100_AIF2_BCLK_FREQ_MASK              0x001F  /*                        */
#define WM5100_AIF2_BCLK_FREQ_SHIFT                  0  /*                        */
#define WM5100_AIF2_BCLK_FREQ_WIDTH                  5  /*                        */

/*
                               
 */
#define WM5100_AIF2TX_DAT_TRI                   0x0020  /*                */
#define WM5100_AIF2TX_DAT_TRI_MASK              0x0020  /*                */
#define WM5100_AIF2TX_DAT_TRI_SHIFT                  5  /*                */
#define WM5100_AIF2TX_DAT_TRI_WIDTH                  1  /*                */
#define WM5100_AIF2TX_LRCLK_SRC                 0x0008  /*                  */
#define WM5100_AIF2TX_LRCLK_SRC_MASK            0x0008  /*                  */
#define WM5100_AIF2TX_LRCLK_SRC_SHIFT                3  /*                  */
#define WM5100_AIF2TX_LRCLK_SRC_WIDTH                1  /*                  */
#define WM5100_AIF2TX_LRCLK_INV                 0x0004  /*                  */
#define WM5100_AIF2TX_LRCLK_INV_MASK            0x0004  /*                  */
#define WM5100_AIF2TX_LRCLK_INV_SHIFT                2  /*                  */
#define WM5100_AIF2TX_LRCLK_INV_WIDTH                1  /*                  */
#define WM5100_AIF2TX_LRCLK_FRC                 0x0002  /*                  */
#define WM5100_AIF2TX_LRCLK_FRC_MASK            0x0002  /*                  */
#define WM5100_AIF2TX_LRCLK_FRC_SHIFT                1  /*                  */
#define WM5100_AIF2TX_LRCLK_FRC_WIDTH                1  /*                  */
#define WM5100_AIF2TX_LRCLK_MSTR                0x0001  /*                   */
#define WM5100_AIF2TX_LRCLK_MSTR_MASK           0x0001  /*                   */
#define WM5100_AIF2TX_LRCLK_MSTR_SHIFT               0  /*                   */
#define WM5100_AIF2TX_LRCLK_MSTR_WIDTH               1  /*                   */

/*
                               
 */
#define WM5100_AIF2RX_LRCLK_INV                 0x0004  /*                  */
#define WM5100_AIF2RX_LRCLK_INV_MASK            0x0004  /*                  */
#define WM5100_AIF2RX_LRCLK_INV_SHIFT                2  /*                  */
#define WM5100_AIF2RX_LRCLK_INV_WIDTH                1  /*                  */
#define WM5100_AIF2RX_LRCLK_FRC                 0x0002  /*                  */
#define WM5100_AIF2RX_LRCLK_FRC_MASK            0x0002  /*                  */
#define WM5100_AIF2RX_LRCLK_FRC_SHIFT                1  /*                  */
#define WM5100_AIF2RX_LRCLK_FRC_WIDTH                1  /*                  */
#define WM5100_AIF2RX_LRCLK_MSTR                0x0001  /*                   */
#define WM5100_AIF2RX_LRCLK_MSTR_MASK           0x0001  /*                   */
#define WM5100_AIF2RX_LRCLK_MSTR_SHIFT               0  /*                   */
#define WM5100_AIF2RX_LRCLK_MSTR_WIDTH               1  /*                   */

/*
                               
 */
#define WM5100_AIF2_TRI                         0x0040  /*          */
#define WM5100_AIF2_TRI_MASK                    0x0040  /*          */
#define WM5100_AIF2_TRI_SHIFT                        6  /*          */
#define WM5100_AIF2_TRI_WIDTH                        1  /*          */
#define WM5100_AIF2_RATE_MASK                   0x0003  /*                   */
#define WM5100_AIF2_RATE_SHIFT                       0  /*                   */
#define WM5100_AIF2_RATE_WIDTH                       2  /*                   */

/*
                               
 */
#define WM5100_AIF2_FMT_MASK                    0x0007  /*                  */
#define WM5100_AIF2_FMT_SHIFT                        0  /*                  */
#define WM5100_AIF2_FMT_WIDTH                        3  /*                  */

/*
                               
 */
#define WM5100_AIF2TX_BCPF_MASK                 0x1FFF  /*                      */
#define WM5100_AIF2TX_BCPF_SHIFT                     0  /*                      */
#define WM5100_AIF2TX_BCPF_WIDTH                    13  /*                      */

/*
                               
 */
#define WM5100_AIF2RX_BCPF_MASK                 0x1FFF  /*                      */
#define WM5100_AIF2RX_BCPF_SHIFT                     0  /*                      */
#define WM5100_AIF2RX_BCPF_WIDTH                    13  /*                      */

/*
                               
 */
#define WM5100_AIF2TX_WL_MASK                   0x3F00  /*                    */
#define WM5100_AIF2TX_WL_SHIFT                       8  /*                    */
#define WM5100_AIF2TX_WL_WIDTH                       6  /*                    */
#define WM5100_AIF2TX_SLOT_LEN_MASK             0x00FF  /*                         */
#define WM5100_AIF2TX_SLOT_LEN_SHIFT                 0  /*                         */
#define WM5100_AIF2TX_SLOT_LEN_WIDTH                 8  /*                         */

/*
                               
 */
#define WM5100_AIF2RX_WL_MASK                   0x3F00  /*                    */
#define WM5100_AIF2RX_WL_SHIFT                       8  /*                    */
#define WM5100_AIF2RX_WL_WIDTH                       6  /*                    */
#define WM5100_AIF2RX_SLOT_LEN_MASK             0x00FF  /*                         */
#define WM5100_AIF2RX_SLOT_LEN_SHIFT                 0  /*                         */
#define WM5100_AIF2RX_SLOT_LEN_WIDTH                 8  /*                         */

/*
                                
 */
#define WM5100_AIF2TX1_SLOT_MASK                0x003F  /*                      */
#define WM5100_AIF2TX1_SLOT_SHIFT                    0  /*                      */
#define WM5100_AIF2TX1_SLOT_WIDTH                    6  /*                      */

/*
                                
 */
#define WM5100_AIF2TX2_SLOT_MASK                0x003F  /*                      */
#define WM5100_AIF2TX2_SLOT_SHIFT                    0  /*                      */
#define WM5100_AIF2TX2_SLOT_WIDTH                    6  /*                      */

/*
                                
 */
#define WM5100_AIF2RX1_SLOT_MASK                0x003F  /*                      */
#define WM5100_AIF2RX1_SLOT_SHIFT                    0  /*                      */
#define WM5100_AIF2RX1_SLOT_WIDTH                    6  /*                      */

/*
                                
 */
#define WM5100_AIF2RX2_SLOT_MASK                0x003F  /*                      */
#define WM5100_AIF2RX2_SLOT_SHIFT                    0  /*                      */
#define WM5100_AIF2RX2_SLOT_WIDTH                    6  /*                      */

/*
                                
 */
#define WM5100_AIF2TX2_ENA                      0x0002  /*             */
#define WM5100_AIF2TX2_ENA_MASK                 0x0002  /*             */
#define WM5100_AIF2TX2_ENA_SHIFT                     1  /*             */
#define WM5100_AIF2TX2_ENA_WIDTH                     1  /*             */
#define WM5100_AIF2TX1_ENA                      0x0001  /*             */
#define WM5100_AIF2TX1_ENA_MASK                 0x0001  /*             */
#define WM5100_AIF2TX1_ENA_SHIFT                     0  /*             */
#define WM5100_AIF2TX1_ENA_WIDTH                     1  /*             */

/*
                                
 */
#define WM5100_AIF2RX2_ENA                      0x0002  /*             */
#define WM5100_AIF2RX2_ENA_MASK                 0x0002  /*             */
#define WM5100_AIF2RX2_ENA_SHIFT                     1  /*             */
#define WM5100_AIF2RX2_ENA_WIDTH                     1  /*             */
#define WM5100_AIF2RX1_ENA                      0x0001  /*             */
#define WM5100_AIF2RX1_ENA_MASK                 0x0001  /*             */
#define WM5100_AIF2RX1_ENA_SHIFT                     0  /*             */
#define WM5100_AIF2RX1_ENA_WIDTH                     1  /*             */

/*
                               
 */
#define WM5100_AIF3_BCLK_INV                    0x0080  /*               */
#define WM5100_AIF3_BCLK_INV_MASK               0x0080  /*               */
#define WM5100_AIF3_BCLK_INV_SHIFT                   7  /*               */
#define WM5100_AIF3_BCLK_INV_WIDTH                   1  /*               */
#define WM5100_AIF3_BCLK_FRC                    0x0040  /*               */
#define WM5100_AIF3_BCLK_FRC_MASK               0x0040  /*               */
#define WM5100_AIF3_BCLK_FRC_SHIFT                   6  /*               */
#define WM5100_AIF3_BCLK_FRC_WIDTH                   1  /*               */
#define WM5100_AIF3_BCLK_MSTR                   0x0020  /*                */
#define WM5100_AIF3_BCLK_MSTR_MASK              0x0020  /*                */
#define WM5100_AIF3_BCLK_MSTR_SHIFT                  5  /*                */
#define WM5100_AIF3_BCLK_MSTR_WIDTH                  1  /*                */
#define WM5100_AIF3_BCLK_FREQ_MASK              0x001F  /*                        */
#define WM5100_AIF3_BCLK_FREQ_SHIFT                  0  /*                        */
#define WM5100_AIF3_BCLK_FREQ_WIDTH                  5  /*                        */

/*
                               
 */
#define WM5100_AIF3TX_DAT_TRI                   0x0020  /*                */
#define WM5100_AIF3TX_DAT_TRI_MASK              0x0020  /*                */
#define WM5100_AIF3TX_DAT_TRI_SHIFT                  5  /*                */
#define WM5100_AIF3TX_DAT_TRI_WIDTH                  1  /*                */
#define WM5100_AIF3TX_LRCLK_SRC                 0x0008  /*                  */
#define WM5100_AIF3TX_LRCLK_SRC_MASK            0x0008  /*                  */
#define WM5100_AIF3TX_LRCLK_SRC_SHIFT                3  /*                  */
#define WM5100_AIF3TX_LRCLK_SRC_WIDTH                1  /*                  */
#define WM5100_AIF3TX_LRCLK_INV                 0x0004  /*                  */
#define WM5100_AIF3TX_LRCLK_INV_MASK            0x0004  /*                  */
#define WM5100_AIF3TX_LRCLK_INV_SHIFT                2  /*                  */
#define WM5100_AIF3TX_LRCLK_INV_WIDTH                1  /*                  */
#define WM5100_AIF3TX_LRCLK_FRC                 0x0002  /*                  */
#define WM5100_AIF3TX_LRCLK_FRC_MASK            0x0002  /*                  */
#define WM5100_AIF3TX_LRCLK_FRC_SHIFT                1  /*                  */
#define WM5100_AIF3TX_LRCLK_FRC_WIDTH                1  /*                  */
#define WM5100_AIF3TX_LRCLK_MSTR                0x0001  /*                   */
#define WM5100_AIF3TX_LRCLK_MSTR_MASK           0x0001  /*                   */
#define WM5100_AIF3TX_LRCLK_MSTR_SHIFT               0  /*                   */
#define WM5100_AIF3TX_LRCLK_MSTR_WIDTH               1  /*                   */

/*
                               
 */
#define WM5100_AIF3RX_LRCLK_INV                 0x0004  /*                  */
#define WM5100_AIF3RX_LRCLK_INV_MASK            0x0004  /*                  */
#define WM5100_AIF3RX_LRCLK_INV_SHIFT                2  /*                  */
#define WM5100_AIF3RX_LRCLK_INV_WIDTH                1  /*                  */
#define WM5100_AIF3RX_LRCLK_FRC                 0x0002  /*                  */
#define WM5100_AIF3RX_LRCLK_FRC_MASK            0x0002  /*                  */
#define WM5100_AIF3RX_LRCLK_FRC_SHIFT                1  /*                  */
#define WM5100_AIF3RX_LRCLK_FRC_WIDTH                1  /*                  */
#define WM5100_AIF3RX_LRCLK_MSTR                0x0001  /*                   */
#define WM5100_AIF3RX_LRCLK_MSTR_MASK           0x0001  /*                   */
#define WM5100_AIF3RX_LRCLK_MSTR_SHIFT               0  /*                   */
#define WM5100_AIF3RX_LRCLK_MSTR_WIDTH               1  /*                   */

/*
                               
 */
#define WM5100_AIF3_TRI                         0x0040  /*          */
#define WM5100_AIF3_TRI_MASK                    0x0040  /*          */
#define WM5100_AIF3_TRI_SHIFT                        6  /*          */
#define WM5100_AIF3_TRI_WIDTH                        1  /*          */
#define WM5100_AIF3_RATE_MASK                   0x0003  /*                   */
#define WM5100_AIF3_RATE_SHIFT                       0  /*                   */
#define WM5100_AIF3_RATE_WIDTH                       2  /*                   */

/*
                               
 */
#define WM5100_AIF3_FMT_MASK                    0x0007  /*                  */
#define WM5100_AIF3_FMT_SHIFT                        0  /*                  */
#define WM5100_AIF3_FMT_WIDTH                        3  /*                  */

/*
                               
 */
#define WM5100_AIF3TX_BCPF_MASK                 0x1FFF  /*                      */
#define WM5100_AIF3TX_BCPF_SHIFT                     0  /*                      */
#define WM5100_AIF3TX_BCPF_WIDTH                    13  /*                      */

/*
                               
 */
#define WM5100_AIF3RX_BCPF_MASK                 0x1FFF  /*                      */
#define WM5100_AIF3RX_BCPF_SHIFT                     0  /*                      */
#define WM5100_AIF3RX_BCPF_WIDTH                    13  /*                      */

/*
                               
 */
#define WM5100_AIF3TX_WL_MASK                   0x3F00  /*                    */
#define WM5100_AIF3TX_WL_SHIFT                       8  /*                    */
#define WM5100_AIF3TX_WL_WIDTH                       6  /*                    */
#define WM5100_AIF3TX_SLOT_LEN_MASK             0x00FF  /*                         */
#define WM5100_AIF3TX_SLOT_LEN_SHIFT                 0  /*                         */
#define WM5100_AIF3TX_SLOT_LEN_WIDTH                 8  /*                         */

/*
                               
 */
#define WM5100_AIF3RX_WL_MASK                   0x3F00  /*                    */
#define WM5100_AIF3RX_WL_SHIFT                       8  /*                    */
#define WM5100_AIF3RX_WL_WIDTH                       6  /*                    */
#define WM5100_AIF3RX_SLOT_LEN_MASK             0x00FF  /*                         */
#define WM5100_AIF3RX_SLOT_LEN_SHIFT                 0  /*                         */
#define WM5100_AIF3RX_SLOT_LEN_WIDTH                 8  /*                         */

/*
                                
 */
#define WM5100_AIF3TX1_SLOT_MASK                0x003F  /*                      */
#define WM5100_AIF3TX1_SLOT_SHIFT                    0  /*                      */
#define WM5100_AIF3TX1_SLOT_WIDTH                    6  /*                      */

/*
                                
 */
#define WM5100_AIF3TX2_SLOT_MASK                0x003F  /*                      */
#define WM5100_AIF3TX2_SLOT_SHIFT                    0  /*                      */
#define WM5100_AIF3TX2_SLOT_WIDTH                    6  /*                      */

/*
                                
 */
#define WM5100_AIF3RX1_SLOT_MASK                0x003F  /*                      */
#define WM5100_AIF3RX1_SLOT_SHIFT                    0  /*                      */
#define WM5100_AIF3RX1_SLOT_WIDTH                    6  /*                      */

/*
                                
 */
#define WM5100_AIF3RX2_SLOT_MASK                0x003F  /*                      */
#define WM5100_AIF3RX2_SLOT_SHIFT                    0  /*                      */
#define WM5100_AIF3RX2_SLOT_WIDTH                    6  /*                      */

/*
                                
 */
#define WM5100_AIF3TX2_ENA                      0x0002  /*             */
#define WM5100_AIF3TX2_ENA_MASK                 0x0002  /*             */
#define WM5100_AIF3TX2_ENA_SHIFT                     1  /*             */
#define WM5100_AIF3TX2_ENA_WIDTH                     1  /*             */
#define WM5100_AIF3TX1_ENA                      0x0001  /*             */
#define WM5100_AIF3TX1_ENA_MASK                 0x0001  /*             */
#define WM5100_AIF3TX1_ENA_SHIFT                     0  /*             */
#define WM5100_AIF3TX1_ENA_WIDTH                     1  /*             */

/*
                                
 */
#define WM5100_AIF3RX2_ENA                      0x0002  /*             */
#define WM5100_AIF3RX2_ENA_MASK                 0x0002  /*             */
#define WM5100_AIF3RX2_ENA_SHIFT                     1  /*             */
#define WM5100_AIF3RX2_ENA_WIDTH                     1  /*             */
#define WM5100_AIF3RX1_ENA                      0x0001  /*             */
#define WM5100_AIF3RX1_ENA_MASK                 0x0001  /*             */
#define WM5100_AIF3RX1_ENA_SHIFT                     0  /*             */
#define WM5100_AIF3RX1_ENA_WIDTH                     1  /*             */

#define WM5100_MIXER_VOL_MASK                0x00FE  /*                   */
#define WM5100_MIXER_VOL_SHIFT                    1  /*                   */
#define WM5100_MIXER_VOL_WIDTH                    7  /*                   */

/*
                              
 */
#define WM5100_GP1_DIR                          0x8000  /*         */
#define WM5100_GP1_DIR_MASK                     0x8000  /*         */
#define WM5100_GP1_DIR_SHIFT                        15  /*         */
#define WM5100_GP1_DIR_WIDTH                         1  /*         */
#define WM5100_GP1_PU                           0x4000  /*        */
#define WM5100_GP1_PU_MASK                      0x4000  /*        */
#define WM5100_GP1_PU_SHIFT                         14  /*        */
#define WM5100_GP1_PU_WIDTH                          1  /*        */
#define WM5100_GP1_PD                           0x2000  /*        */
#define WM5100_GP1_PD_MASK                      0x2000  /*        */
#define WM5100_GP1_PD_SHIFT                         13  /*        */
#define WM5100_GP1_PD_WIDTH                          1  /*        */
#define WM5100_GP1_POL                          0x0400  /*         */
#define WM5100_GP1_POL_MASK                     0x0400  /*         */
#define WM5100_GP1_POL_SHIFT                        10  /*         */
#define WM5100_GP1_POL_WIDTH                         1  /*         */
#define WM5100_GP1_OP_CFG                       0x0200  /*            */
#define WM5100_GP1_OP_CFG_MASK                  0x0200  /*            */
#define WM5100_GP1_OP_CFG_SHIFT                      9  /*            */
#define WM5100_GP1_OP_CFG_WIDTH                      1  /*            */
#define WM5100_GP1_DB                           0x0100  /*        */
#define WM5100_GP1_DB_MASK                      0x0100  /*        */
#define WM5100_GP1_DB_SHIFT                          8  /*        */
#define WM5100_GP1_DB_WIDTH                          1  /*        */
#define WM5100_GP1_LVL                          0x0040  /*         */
#define WM5100_GP1_LVL_MASK                     0x0040  /*         */
#define WM5100_GP1_LVL_SHIFT                         6  /*         */
#define WM5100_GP1_LVL_WIDTH                         1  /*         */
#define WM5100_GP1_FN_MASK                      0x003F  /*                */
#define WM5100_GP1_FN_SHIFT                          0  /*                */
#define WM5100_GP1_FN_WIDTH                          6  /*                */

/*
                              
 */
#define WM5100_GP2_DIR                          0x8000  /*         */
#define WM5100_GP2_DIR_MASK                     0x8000  /*         */
#define WM5100_GP2_DIR_SHIFT                        15  /*         */
#define WM5100_GP2_DIR_WIDTH                         1  /*         */
#define WM5100_GP2_PU                           0x4000  /*        */
#define WM5100_GP2_PU_MASK                      0x4000  /*        */
#define WM5100_GP2_PU_SHIFT                         14  /*        */
#define WM5100_GP2_PU_WIDTH                          1  /*        */
#define WM5100_GP2_PD                           0x2000  /*        */
#define WM5100_GP2_PD_MASK                      0x2000  /*        */
#define WM5100_GP2_PD_SHIFT                         13  /*        */
#define WM5100_GP2_PD_WIDTH                          1  /*        */
#define WM5100_GP2_POL                          0x0400  /*         */
#define WM5100_GP2_POL_MASK                     0x0400  /*         */
#define WM5100_GP2_POL_SHIFT                        10  /*         */
#define WM5100_GP2_POL_WIDTH                         1  /*         */
#define WM5100_GP2_OP_CFG                       0x0200  /*            */
#define WM5100_GP2_OP_CFG_MASK                  0x0200  /*            */
#define WM5100_GP2_OP_CFG_SHIFT                      9  /*            */
#define WM5100_GP2_OP_CFG_WIDTH                      1  /*            */
#define WM5100_GP2_DB                           0x0100  /*        */
#define WM5100_GP2_DB_MASK                      0x0100  /*        */
#define WM5100_GP2_DB_SHIFT                          8  /*        */
#define WM5100_GP2_DB_WIDTH                          1  /*        */
#define WM5100_GP2_LVL                          0x0040  /*         */
#define WM5100_GP2_LVL_MASK                     0x0040  /*         */
#define WM5100_GP2_LVL_SHIFT                         6  /*         */
#define WM5100_GP2_LVL_WIDTH                         1  /*         */
#define WM5100_GP2_FN_MASK                      0x003F  /*                */
#define WM5100_GP2_FN_SHIFT                          0  /*                */
#define WM5100_GP2_FN_WIDTH                          6  /*                */

/*
                              
 */
#define WM5100_GP3_DIR                          0x8000  /*         */
#define WM5100_GP3_DIR_MASK                     0x8000  /*         */
#define WM5100_GP3_DIR_SHIFT                        15  /*         */
#define WM5100_GP3_DIR_WIDTH                         1  /*         */
#define WM5100_GP3_PU                           0x4000  /*        */
#define WM5100_GP3_PU_MASK                      0x4000  /*        */
#define WM5100_GP3_PU_SHIFT                         14  /*        */
#define WM5100_GP3_PU_WIDTH                          1  /*        */
#define WM5100_GP3_PD                           0x2000  /*        */
#define WM5100_GP3_PD_MASK                      0x2000  /*        */
#define WM5100_GP3_PD_SHIFT                         13  /*        */
#define WM5100_GP3_PD_WIDTH                          1  /*        */
#define WM5100_GP3_POL                          0x0400  /*         */
#define WM5100_GP3_POL_MASK                     0x0400  /*         */
#define WM5100_GP3_POL_SHIFT                        10  /*         */
#define WM5100_GP3_POL_WIDTH                         1  /*         */
#define WM5100_GP3_OP_CFG                       0x0200  /*            */
#define WM5100_GP3_OP_CFG_MASK                  0x0200  /*            */
#define WM5100_GP3_OP_CFG_SHIFT                      9  /*            */
#define WM5100_GP3_OP_CFG_WIDTH                      1  /*            */
#define WM5100_GP3_DB                           0x0100  /*        */
#define WM5100_GP3_DB_MASK                      0x0100  /*        */
#define WM5100_GP3_DB_SHIFT                          8  /*        */
#define WM5100_GP3_DB_WIDTH                          1  /*        */
#define WM5100_GP3_LVL                          0x0040  /*         */
#define WM5100_GP3_LVL_MASK                     0x0040  /*         */
#define WM5100_GP3_LVL_SHIFT                         6  /*         */
#define WM5100_GP3_LVL_WIDTH                         1  /*         */
#define WM5100_GP3_FN_MASK                      0x003F  /*                */
#define WM5100_GP3_FN_SHIFT                          0  /*                */
#define WM5100_GP3_FN_WIDTH                          6  /*                */

/*
                              
 */
#define WM5100_GP4_DIR                          0x8000  /*         */
#define WM5100_GP4_DIR_MASK                     0x8000  /*         */
#define WM5100_GP4_DIR_SHIFT                        15  /*         */
#define WM5100_GP4_DIR_WIDTH                         1  /*         */
#define WM5100_GP4_PU                           0x4000  /*        */
#define WM5100_GP4_PU_MASK                      0x4000  /*        */
#define WM5100_GP4_PU_SHIFT                         14  /*        */
#define WM5100_GP4_PU_WIDTH                          1  /*        */
#define WM5100_GP4_PD                           0x2000  /*        */
#define WM5100_GP4_PD_MASK                      0x2000  /*        */
#define WM5100_GP4_PD_SHIFT                         13  /*        */
#define WM5100_GP4_PD_WIDTH                          1  /*        */
#define WM5100_GP4_POL                          0x0400  /*         */
#define WM5100_GP4_POL_MASK                     0x0400  /*         */
#define WM5100_GP4_POL_SHIFT                        10  /*         */
#define WM5100_GP4_POL_WIDTH                         1  /*         */
#define WM5100_GP4_OP_CFG                       0x0200  /*            */
#define WM5100_GP4_OP_CFG_MASK                  0x0200  /*            */
#define WM5100_GP4_OP_CFG_SHIFT                      9  /*            */
#define WM5100_GP4_OP_CFG_WIDTH                      1  /*            */
#define WM5100_GP4_DB                           0x0100  /*        */
#define WM5100_GP4_DB_MASK                      0x0100  /*        */
#define WM5100_GP4_DB_SHIFT                          8  /*        */
#define WM5100_GP4_DB_WIDTH                          1  /*        */
#define WM5100_GP4_LVL                          0x0040  /*         */
#define WM5100_GP4_LVL_MASK                     0x0040  /*         */
#define WM5100_GP4_LVL_SHIFT                         6  /*         */
#define WM5100_GP4_LVL_WIDTH                         1  /*         */
#define WM5100_GP4_FN_MASK                      0x003F  /*                */
#define WM5100_GP4_FN_SHIFT                          0  /*                */
#define WM5100_GP4_FN_WIDTH                          6  /*                */

/*
                              
 */
#define WM5100_GP5_DIR                          0x8000  /*         */
#define WM5100_GP5_DIR_MASK                     0x8000  /*         */
#define WM5100_GP5_DIR_SHIFT                        15  /*         */
#define WM5100_GP5_DIR_WIDTH                         1  /*         */
#define WM5100_GP5_PU                           0x4000  /*        */
#define WM5100_GP5_PU_MASK                      0x4000  /*        */
#define WM5100_GP5_PU_SHIFT                         14  /*        */
#define WM5100_GP5_PU_WIDTH                          1  /*        */
#define WM5100_GP5_PD                           0x2000  /*        */
#define WM5100_GP5_PD_MASK                      0x2000  /*        */
#define WM5100_GP5_PD_SHIFT                         13  /*        */
#define WM5100_GP5_PD_WIDTH                          1  /*        */
#define WM5100_GP5_POL                          0x0400  /*         */
#define WM5100_GP5_POL_MASK                     0x0400  /*         */
#define WM5100_GP5_POL_SHIFT                        10  /*         */
#define WM5100_GP5_POL_WIDTH                         1  /*         */
#define WM5100_GP5_OP_CFG                       0x0200  /*            */
#define WM5100_GP5_OP_CFG_MASK                  0x0200  /*            */
#define WM5100_GP5_OP_CFG_SHIFT                      9  /*            */
#define WM5100_GP5_OP_CFG_WIDTH                      1  /*            */
#define WM5100_GP5_DB                           0x0100  /*        */
#define WM5100_GP5_DB_MASK                      0x0100  /*        */
#define WM5100_GP5_DB_SHIFT                          8  /*        */
#define WM5100_GP5_DB_WIDTH                          1  /*        */
#define WM5100_GP5_LVL                          0x0040  /*         */
#define WM5100_GP5_LVL_MASK                     0x0040  /*         */
#define WM5100_GP5_LVL_SHIFT                         6  /*         */
#define WM5100_GP5_LVL_WIDTH                         1  /*         */
#define WM5100_GP5_FN_MASK                      0x003F  /*                */
#define WM5100_GP5_FN_SHIFT                          0  /*                */
#define WM5100_GP5_FN_WIDTH                          6  /*                */

/*
                              
 */
#define WM5100_GP6_DIR                          0x8000  /*         */
#define WM5100_GP6_DIR_MASK                     0x8000  /*         */
#define WM5100_GP6_DIR_SHIFT                        15  /*         */
#define WM5100_GP6_DIR_WIDTH                         1  /*         */
#define WM5100_GP6_PU                           0x4000  /*        */
#define WM5100_GP6_PU_MASK                      0x4000  /*        */
#define WM5100_GP6_PU_SHIFT                         14  /*        */
#define WM5100_GP6_PU_WIDTH                          1  /*        */
#define WM5100_GP6_PD                           0x2000  /*        */
#define WM5100_GP6_PD_MASK                      0x2000  /*        */
#define WM5100_GP6_PD_SHIFT                         13  /*        */
#define WM5100_GP6_PD_WIDTH                          1  /*        */
#define WM5100_GP6_POL                          0x0400  /*         */
#define WM5100_GP6_POL_MASK                     0x0400  /*         */
#define WM5100_GP6_POL_SHIFT                        10  /*         */
#define WM5100_GP6_POL_WIDTH                         1  /*         */
#define WM5100_GP6_OP_CFG                       0x0200  /*            */
#define WM5100_GP6_OP_CFG_MASK                  0x0200  /*            */
#define WM5100_GP6_OP_CFG_SHIFT                      9  /*            */
#define WM5100_GP6_OP_CFG_WIDTH                      1  /*            */
#define WM5100_GP6_DB                           0x0100  /*        */
#define WM5100_GP6_DB_MASK                      0x0100  /*        */
#define WM5100_GP6_DB_SHIFT                          8  /*        */
#define WM5100_GP6_DB_WIDTH                          1  /*        */
#define WM5100_GP6_LVL                          0x0040  /*         */
#define WM5100_GP6_LVL_MASK                     0x0040  /*         */
#define WM5100_GP6_LVL_SHIFT                         6  /*         */
#define WM5100_GP6_LVL_WIDTH                         1  /*         */
#define WM5100_GP6_FN_MASK                      0x003F  /*                */
#define WM5100_GP6_FN_SHIFT                          0  /*                */
#define WM5100_GP6_FN_WIDTH                          6  /*                */

/*
                                  
 */
#define WM5100_LDO1ENA_PD                       0x8000  /*            */
#define WM5100_LDO1ENA_PD_MASK                  0x8000  /*            */
#define WM5100_LDO1ENA_PD_SHIFT                     15  /*            */
#define WM5100_LDO1ENA_PD_WIDTH                      1  /*            */
#define WM5100_MCLK2_PD                         0x2000  /*          */
#define WM5100_MCLK2_PD_MASK                    0x2000  /*          */
#define WM5100_MCLK2_PD_SHIFT                       13  /*          */
#define WM5100_MCLK2_PD_WIDTH                        1  /*          */
#define WM5100_MCLK1_PD                         0x1000  /*          */
#define WM5100_MCLK1_PD_MASK                    0x1000  /*          */
#define WM5100_MCLK1_PD_SHIFT                       12  /*          */
#define WM5100_MCLK1_PD_WIDTH                        1  /*          */
#define WM5100_RESET_PU                         0x0002  /*          */
#define WM5100_RESET_PU_MASK                    0x0002  /*          */
#define WM5100_RESET_PU_SHIFT                        1  /*          */
#define WM5100_RESET_PU_WIDTH                        1  /*          */
#define WM5100_ADDR_PD                          0x0001  /*         */
#define WM5100_ADDR_PD_MASK                     0x0001  /*         */
#define WM5100_ADDR_PD_SHIFT                         0  /*         */
#define WM5100_ADDR_PD_WIDTH                         1  /*         */

/*
                                  
 */
#define WM5100_DMICDAT4_PD                      0x0008  /*             */
#define WM5100_DMICDAT4_PD_MASK                 0x0008  /*             */
#define WM5100_DMICDAT4_PD_SHIFT                     3  /*             */
#define WM5100_DMICDAT4_PD_WIDTH                     1  /*             */
#define WM5100_DMICDAT3_PD                      0x0004  /*             */
#define WM5100_DMICDAT3_PD_MASK                 0x0004  /*             */
#define WM5100_DMICDAT3_PD_SHIFT                     2  /*             */
#define WM5100_DMICDAT3_PD_WIDTH                     1  /*             */
#define WM5100_DMICDAT2_PD                      0x0002  /*             */
#define WM5100_DMICDAT2_PD_MASK                 0x0002  /*             */
#define WM5100_DMICDAT2_PD_SHIFT                     1  /*             */
#define WM5100_DMICDAT2_PD_WIDTH                     1  /*             */
#define WM5100_DMICDAT1_PD                      0x0001  /*             */
#define WM5100_DMICDAT1_PD_MASK                 0x0001  /*             */
#define WM5100_DMICDAT1_PD_SHIFT                     0  /*             */
#define WM5100_DMICDAT1_PD_WIDTH                     1  /*             */

/*
                                  
 */
#define WM5100_AIF1RXLRCLK_PU                   0x0020  /*                */
#define WM5100_AIF1RXLRCLK_PU_MASK              0x0020  /*                */
#define WM5100_AIF1RXLRCLK_PU_SHIFT                  5  /*                */
#define WM5100_AIF1RXLRCLK_PU_WIDTH                  1  /*                */
#define WM5100_AIF1RXLRCLK_PD                   0x0010  /*                */
#define WM5100_AIF1RXLRCLK_PD_MASK              0x0010  /*                */
#define WM5100_AIF1RXLRCLK_PD_SHIFT                  4  /*                */
#define WM5100_AIF1RXLRCLK_PD_WIDTH                  1  /*                */
#define WM5100_AIF1BCLK_PU                      0x0008  /*             */
#define WM5100_AIF1BCLK_PU_MASK                 0x0008  /*             */
#define WM5100_AIF1BCLK_PU_SHIFT                     3  /*             */
#define WM5100_AIF1BCLK_PU_WIDTH                     1  /*             */
#define WM5100_AIF1BCLK_PD                      0x0004  /*             */
#define WM5100_AIF1BCLK_PD_MASK                 0x0004  /*             */
#define WM5100_AIF1BCLK_PD_SHIFT                     2  /*             */
#define WM5100_AIF1BCLK_PD_WIDTH                     1  /*             */
#define WM5100_AIF1RXDAT_PU                     0x0002  /*              */
#define WM5100_AIF1RXDAT_PU_MASK                0x0002  /*              */
#define WM5100_AIF1RXDAT_PU_SHIFT                    1  /*              */
#define WM5100_AIF1RXDAT_PU_WIDTH                    1  /*              */
#define WM5100_AIF1RXDAT_PD                     0x0001  /*              */
#define WM5100_AIF1RXDAT_PD_MASK                0x0001  /*              */
#define WM5100_AIF1RXDAT_PD_SHIFT                    0  /*              */
#define WM5100_AIF1RXDAT_PD_WIDTH                    1  /*              */

/*
                                  
 */
#define WM5100_AIF2RXLRCLK_PU                   0x0020  /*                */
#define WM5100_AIF2RXLRCLK_PU_MASK              0x0020  /*                */
#define WM5100_AIF2RXLRCLK_PU_SHIFT                  5  /*                */
#define WM5100_AIF2RXLRCLK_PU_WIDTH                  1  /*                */
#define WM5100_AIF2RXLRCLK_PD                   0x0010  /*                */
#define WM5100_AIF2RXLRCLK_PD_MASK              0x0010  /*                */
#define WM5100_AIF2RXLRCLK_PD_SHIFT                  4  /*                */
#define WM5100_AIF2RXLRCLK_PD_WIDTH                  1  /*                */
#define WM5100_AIF2BCLK_PU                      0x0008  /*             */
#define WM5100_AIF2BCLK_PU_MASK                 0x0008  /*             */
#define WM5100_AIF2BCLK_PU_SHIFT                     3  /*             */
#define WM5100_AIF2BCLK_PU_WIDTH                     1  /*             */
#define WM5100_AIF2BCLK_PD                      0x0004  /*             */
#define WM5100_AIF2BCLK_PD_MASK                 0x0004  /*             */
#define WM5100_AIF2BCLK_PD_SHIFT                     2  /*             */
#define WM5100_AIF2BCLK_PD_WIDTH                     1  /*             */
#define WM5100_AIF2RXDAT_PU                     0x0002  /*              */
#define WM5100_AIF2RXDAT_PU_MASK                0x0002  /*              */
#define WM5100_AIF2RXDAT_PU_SHIFT                    1  /*              */
#define WM5100_AIF2RXDAT_PU_WIDTH                    1  /*              */
#define WM5100_AIF2RXDAT_PD                     0x0001  /*              */
#define WM5100_AIF2RXDAT_PD_MASK                0x0001  /*              */
#define WM5100_AIF2RXDAT_PD_SHIFT                    0  /*              */
#define WM5100_AIF2RXDAT_PD_WIDTH                    1  /*              */

/*
                                  
 */
#define WM5100_AIF3RXLRCLK_PU                   0x0020  /*                */
#define WM5100_AIF3RXLRCLK_PU_MASK              0x0020  /*                */
#define WM5100_AIF3RXLRCLK_PU_SHIFT                  5  /*                */
#define WM5100_AIF3RXLRCLK_PU_WIDTH                  1  /*                */
#define WM5100_AIF3RXLRCLK_PD                   0x0010  /*                */
#define WM5100_AIF3RXLRCLK_PD_MASK              0x0010  /*                */
#define WM5100_AIF3RXLRCLK_PD_SHIFT                  4  /*                */
#define WM5100_AIF3RXLRCLK_PD_WIDTH                  1  /*                */
#define WM5100_AIF3BCLK_PU                      0x0008  /*             */
#define WM5100_AIF3BCLK_PU_MASK                 0x0008  /*             */
#define WM5100_AIF3BCLK_PU_SHIFT                     3  /*             */
#define WM5100_AIF3BCLK_PU_WIDTH                     1  /*             */
#define WM5100_AIF3BCLK_PD                      0x0004  /*             */
#define WM5100_AIF3BCLK_PD_MASK                 0x0004  /*             */
#define WM5100_AIF3BCLK_PD_SHIFT                     2  /*             */
#define WM5100_AIF3BCLK_PD_WIDTH                     1  /*             */
#define WM5100_AIF3RXDAT_PU                     0x0002  /*              */
#define WM5100_AIF3RXDAT_PU_MASK                0x0002  /*              */
#define WM5100_AIF3RXDAT_PU_SHIFT                    1  /*              */
#define WM5100_AIF3RXDAT_PU_WIDTH                    1  /*              */
#define WM5100_AIF3RXDAT_PD                     0x0001  /*              */
#define WM5100_AIF3RXDAT_PD_MASK                0x0001  /*              */
#define WM5100_AIF3RXDAT_PD_SHIFT                    0  /*              */
#define WM5100_AIF3RXDAT_PD_WIDTH                    1  /*              */

/*
                              
 */
#define WM5100_OPCLK_SEL_MASK                   0x0003  /*                   */
#define WM5100_OPCLK_SEL_SHIFT                       0  /*                   */
#define WM5100_OPCLK_SEL_WIDTH                       2  /*                   */

/*
                                     
 */
#define WM5100_GP6_EINT                         0x0020  /*          */
#define WM5100_GP6_EINT_MASK                    0x0020  /*          */
#define WM5100_GP6_EINT_SHIFT                        5  /*          */
#define WM5100_GP6_EINT_WIDTH                        1  /*          */
#define WM5100_GP5_EINT                         0x0010  /*          */
#define WM5100_GP5_EINT_MASK                    0x0010  /*          */
#define WM5100_GP5_EINT_SHIFT                        4  /*          */
#define WM5100_GP5_EINT_WIDTH                        1  /*          */
#define WM5100_GP4_EINT                         0x0008  /*          */
#define WM5100_GP4_EINT_MASK                    0x0008  /*          */
#define WM5100_GP4_EINT_SHIFT                        3  /*          */
#define WM5100_GP4_EINT_WIDTH                        1  /*          */
#define WM5100_GP3_EINT                         0x0004  /*          */
#define WM5100_GP3_EINT_MASK                    0x0004  /*          */
#define WM5100_GP3_EINT_SHIFT                        2  /*          */
#define WM5100_GP3_EINT_WIDTH                        1  /*          */
#define WM5100_GP2_EINT                         0x0002  /*          */
#define WM5100_GP2_EINT_MASK                    0x0002  /*          */
#define WM5100_GP2_EINT_SHIFT                        1  /*          */
#define WM5100_GP2_EINT_WIDTH                        1  /*          */
#define WM5100_GP1_EINT                         0x0001  /*          */
#define WM5100_GP1_EINT_MASK                    0x0001  /*          */
#define WM5100_GP1_EINT_SHIFT                        0  /*          */
#define WM5100_GP1_EINT_WIDTH                        1  /*          */

/*
                                     
 */
#define WM5100_DSP_IRQ6_EINT                    0x0020  /*               */
#define WM5100_DSP_IRQ6_EINT_MASK               0x0020  /*               */
#define WM5100_DSP_IRQ6_EINT_SHIFT                   5  /*               */
#define WM5100_DSP_IRQ6_EINT_WIDTH                   1  /*               */
#define WM5100_DSP_IRQ5_EINT                    0x0010  /*               */
#define WM5100_DSP_IRQ5_EINT_MASK               0x0010  /*               */
#define WM5100_DSP_IRQ5_EINT_SHIFT                   4  /*               */
#define WM5100_DSP_IRQ5_EINT_WIDTH                   1  /*               */
#define WM5100_DSP_IRQ4_EINT                    0x0008  /*               */
#define WM5100_DSP_IRQ4_EINT_MASK               0x0008  /*               */
#define WM5100_DSP_IRQ4_EINT_SHIFT                   3  /*               */
#define WM5100_DSP_IRQ4_EINT_WIDTH                   1  /*               */
#define WM5100_DSP_IRQ3_EINT                    0x0004  /*               */
#define WM5100_DSP_IRQ3_EINT_MASK               0x0004  /*               */
#define WM5100_DSP_IRQ3_EINT_SHIFT                   2  /*               */
#define WM5100_DSP_IRQ3_EINT_WIDTH                   1  /*               */
#define WM5100_DSP_IRQ2_EINT                    0x0002  /*               */
#define WM5100_DSP_IRQ2_EINT_MASK               0x0002  /*               */
#define WM5100_DSP_IRQ2_EINT_SHIFT                   1  /*               */
#define WM5100_DSP_IRQ2_EINT_WIDTH                   1  /*               */
#define WM5100_DSP_IRQ1_EINT                    0x0001  /*               */
#define WM5100_DSP_IRQ1_EINT_MASK               0x0001  /*               */
#define WM5100_DSP_IRQ1_EINT_SHIFT                   0  /*               */
#define WM5100_DSP_IRQ1_EINT_WIDTH                   1  /*               */

/*
                                     
 */
#define WM5100_SPK_SHUTDOWN_WARN_EINT           0x8000  /*                        */
#define WM5100_SPK_SHUTDOWN_WARN_EINT_MASK      0x8000  /*                        */
#define WM5100_SPK_SHUTDOWN_WARN_EINT_SHIFT         15  /*                        */
#define WM5100_SPK_SHUTDOWN_WARN_EINT_WIDTH          1  /*                        */
#define WM5100_SPK_SHUTDOWN_EINT                0x4000  /*                   */
#define WM5100_SPK_SHUTDOWN_EINT_MASK           0x4000  /*                   */
#define WM5100_SPK_SHUTDOWN_EINT_SHIFT              14  /*                   */
#define WM5100_SPK_SHUTDOWN_EINT_WIDTH               1  /*                   */
#define WM5100_HPDET_EINT                       0x2000  /*            */
#define WM5100_HPDET_EINT_MASK                  0x2000  /*            */
#define WM5100_HPDET_EINT_SHIFT                     13  /*            */
#define WM5100_HPDET_EINT_WIDTH                      1  /*            */
#define WM5100_ACCDET_EINT                      0x1000  /*             */
#define WM5100_ACCDET_EINT_MASK                 0x1000  /*             */
#define WM5100_ACCDET_EINT_SHIFT                    12  /*             */
#define WM5100_ACCDET_EINT_WIDTH                     1  /*             */
#define WM5100_DRC_SIG_DET_EINT                 0x0200  /*                  */
#define WM5100_DRC_SIG_DET_EINT_MASK            0x0200  /*                  */
#define WM5100_DRC_SIG_DET_EINT_SHIFT                9  /*                  */
#define WM5100_DRC_SIG_DET_EINT_WIDTH                1  /*                  */
#define WM5100_ASRC2_LOCK_EINT                  0x0100  /*                 */
#define WM5100_ASRC2_LOCK_EINT_MASK             0x0100  /*                 */
#define WM5100_ASRC2_LOCK_EINT_SHIFT                 8  /*                 */
#define WM5100_ASRC2_LOCK_EINT_WIDTH                 1  /*                 */
#define WM5100_ASRC1_LOCK_EINT                  0x0080  /*                 */
#define WM5100_ASRC1_LOCK_EINT_MASK             0x0080  /*                 */
#define WM5100_ASRC1_LOCK_EINT_SHIFT                 7  /*                 */
#define WM5100_ASRC1_LOCK_EINT_WIDTH                 1  /*                 */
#define WM5100_FLL2_LOCK_EINT                   0x0008  /*                */
#define WM5100_FLL2_LOCK_EINT_MASK              0x0008  /*                */
#define WM5100_FLL2_LOCK_EINT_SHIFT                  3  /*                */
#define WM5100_FLL2_LOCK_EINT_WIDTH                  1  /*                */
#define WM5100_FLL1_LOCK_EINT                   0x0004  /*                */
#define WM5100_FLL1_LOCK_EINT_MASK              0x0004  /*                */
#define WM5100_FLL1_LOCK_EINT_SHIFT                  2  /*                */
#define WM5100_FLL1_LOCK_EINT_WIDTH                  1  /*                */
#define WM5100_CLKGEN_ERR_EINT                  0x0002  /*                 */
#define WM5100_CLKGEN_ERR_EINT_MASK             0x0002  /*                 */
#define WM5100_CLKGEN_ERR_EINT_SHIFT                 1  /*                 */
#define WM5100_CLKGEN_ERR_EINT_WIDTH                 1  /*                 */
#define WM5100_CLKGEN_ERR_ASYNC_EINT            0x0001  /*                       */
#define WM5100_CLKGEN_ERR_ASYNC_EINT_MASK       0x0001  /*                       */
#define WM5100_CLKGEN_ERR_ASYNC_EINT_SHIFT           0  /*                       */
#define WM5100_CLKGEN_ERR_ASYNC_EINT_WIDTH           1  /*                       */

/*
                                     
 */
#define WM5100_AIF3_ERR_EINT                    0x2000  /*               */
#define WM5100_AIF3_ERR_EINT_MASK               0x2000  /*               */
#define WM5100_AIF3_ERR_EINT_SHIFT                  13  /*               */
#define WM5100_AIF3_ERR_EINT_WIDTH                   1  /*               */
#define WM5100_AIF2_ERR_EINT                    0x1000  /*               */
#define WM5100_AIF2_ERR_EINT_MASK               0x1000  /*               */
#define WM5100_AIF2_ERR_EINT_SHIFT                  12  /*               */
#define WM5100_AIF2_ERR_EINT_WIDTH                   1  /*               */
#define WM5100_AIF1_ERR_EINT                    0x0800  /*               */
#define WM5100_AIF1_ERR_EINT_MASK               0x0800  /*               */
#define WM5100_AIF1_ERR_EINT_SHIFT                  11  /*               */
#define WM5100_AIF1_ERR_EINT_WIDTH                   1  /*               */
#define WM5100_CTRLIF_ERR_EINT                  0x0400  /*                 */
#define WM5100_CTRLIF_ERR_EINT_MASK             0x0400  /*                 */
#define WM5100_CTRLIF_ERR_EINT_SHIFT                10  /*                 */
#define WM5100_CTRLIF_ERR_EINT_WIDTH                 1  /*                 */
#define WM5100_ISRC2_UNDERCLOCKED_EINT          0x0200  /*                         */
#define WM5100_ISRC2_UNDERCLOCKED_EINT_MASK     0x0200  /*                         */
#define WM5100_ISRC2_UNDERCLOCKED_EINT_SHIFT         9  /*                         */
#define WM5100_ISRC2_UNDERCLOCKED_EINT_WIDTH         1  /*                         */
#define WM5100_ISRC1_UNDERCLOCKED_EINT          0x0100  /*                         */
#define WM5100_ISRC1_UNDERCLOCKED_EINT_MASK     0x0100  /*                         */
#define WM5100_ISRC1_UNDERCLOCKED_EINT_SHIFT         8  /*                         */
#define WM5100_ISRC1_UNDERCLOCKED_EINT_WIDTH         1  /*                         */
#define WM5100_FX_UNDERCLOCKED_EINT             0x0080  /*                      */
#define WM5100_FX_UNDERCLOCKED_EINT_MASK        0x0080  /*                      */
#define WM5100_FX_UNDERCLOCKED_EINT_SHIFT            7  /*                      */
#define WM5100_FX_UNDERCLOCKED_EINT_WIDTH            1  /*                      */
#define WM5100_AIF3_UNDERCLOCKED_EINT           0x0040  /*                        */
#define WM5100_AIF3_UNDERCLOCKED_EINT_MASK      0x0040  /*                        */
#define WM5100_AIF3_UNDERCLOCKED_EINT_SHIFT          6  /*                        */
#define WM5100_AIF3_UNDERCLOCKED_EINT_WIDTH          1  /*                        */
#define WM5100_AIF2_UNDERCLOCKED_EINT           0x0020  /*                        */
#define WM5100_AIF2_UNDERCLOCKED_EINT_MASK      0x0020  /*                        */
#define WM5100_AIF2_UNDERCLOCKED_EINT_SHIFT          5  /*                        */
#define WM5100_AIF2_UNDERCLOCKED_EINT_WIDTH          1  /*                        */
#define WM5100_AIF1_UNDERCLOCKED_EINT           0x0010  /*                        */
#define WM5100_AIF1_UNDERCLOCKED_EINT_MASK      0x0010  /*                        */
#define WM5100_AIF1_UNDERCLOCKED_EINT_SHIFT          4  /*                        */
#define WM5100_AIF1_UNDERCLOCKED_EINT_WIDTH          1  /*                        */
#define WM5100_ASRC_UNDERCLOCKED_EINT           0x0008  /*                        */
#define WM5100_ASRC_UNDERCLOCKED_EINT_MASK      0x0008  /*                        */
#define WM5100_ASRC_UNDERCLOCKED_EINT_SHIFT          3  /*                        */
#define WM5100_ASRC_UNDERCLOCKED_EINT_WIDTH          1  /*                        */
#define WM5100_DAC_UNDERCLOCKED_EINT            0x0004  /*                       */
#define WM5100_DAC_UNDERCLOCKED_EINT_MASK       0x0004  /*                       */
#define WM5100_DAC_UNDERCLOCKED_EINT_SHIFT           2  /*                       */
#define WM5100_DAC_UNDERCLOCKED_EINT_WIDTH           1  /*                       */
#define WM5100_ADC_UNDERCLOCKED_EINT            0x0002  /*                       */
#define WM5100_ADC_UNDERCLOCKED_EINT_MASK       0x0002  /*                       */
#define WM5100_ADC_UNDERCLOCKED_EINT_SHIFT           1  /*                       */
#define WM5100_ADC_UNDERCLOCKED_EINT_WIDTH           1  /*                       */
#define WM5100_MIXER_UNDERCLOCKED_EINT          0x0001  /*                         */
#define WM5100_MIXER_UNDERCLOCKED_EINT_MASK     0x0001  /*                         */
#define WM5100_MIXER_UNDERCLOCKED_EINT_SHIFT         0  /*                         */
#define WM5100_MIXER_UNDERCLOCKED_EINT_WIDTH         1  /*                         */

/*
                                         
 */
#define WM5100_DSP_IRQ6_STS                     0x0020  /*              */
#define WM5100_DSP_IRQ6_STS_MASK                0x0020  /*              */
#define WM5100_DSP_IRQ6_STS_SHIFT                    5  /*              */
#define WM5100_DSP_IRQ6_STS_WIDTH                    1  /*              */
#define WM5100_DSP_IRQ5_STS                     0x0010  /*              */
#define WM5100_DSP_IRQ5_STS_MASK                0x0010  /*              */
#define WM5100_DSP_IRQ5_STS_SHIFT                    4  /*              */
#define WM5100_DSP_IRQ5_STS_WIDTH                    1  /*              */
#define WM5100_DSP_IRQ4_STS                     0x0008  /*              */
#define WM5100_DSP_IRQ4_STS_MASK                0x0008  /*              */
#define WM5100_DSP_IRQ4_STS_SHIFT                    3  /*              */
#define WM5100_DSP_IRQ4_STS_WIDTH                    1  /*              */
#define WM5100_DSP_IRQ3_STS                     0x0004  /*              */
#define WM5100_DSP_IRQ3_STS_MASK                0x0004  /*              */
#define WM5100_DSP_IRQ3_STS_SHIFT                    2  /*              */
#define WM5100_DSP_IRQ3_STS_WIDTH                    1  /*              */
#define WM5100_DSP_IRQ2_STS                     0x0002  /*              */
#define WM5100_DSP_IRQ2_STS_MASK                0x0002  /*              */
#define WM5100_DSP_IRQ2_STS_SHIFT                    1  /*              */
#define WM5100_DSP_IRQ2_STS_WIDTH                    1  /*              */
#define WM5100_DSP_IRQ1_STS                     0x0001  /*              */
#define WM5100_DSP_IRQ1_STS_MASK                0x0001  /*              */
#define WM5100_DSP_IRQ1_STS_SHIFT                    0  /*              */
#define WM5100_DSP_IRQ1_STS_WIDTH                    1  /*              */

/*
                                         
 */
#define WM5100_SPK_SHUTDOWN_WARN_STS            0x8000  /*                       */
#define WM5100_SPK_SHUTDOWN_WARN_STS_MASK       0x8000  /*                       */
#define WM5100_SPK_SHUTDOWN_WARN_STS_SHIFT          15  /*                       */
#define WM5100_SPK_SHUTDOWN_WARN_STS_WIDTH           1  /*                       */
#define WM5100_SPK_SHUTDOWN_STS                 0x4000  /*                  */
#define WM5100_SPK_SHUTDOWN_STS_MASK            0x4000  /*                  */
#define WM5100_SPK_SHUTDOWN_STS_SHIFT               14  /*                  */
#define WM5100_SPK_SHUTDOWN_STS_WIDTH                1  /*                  */
#define WM5100_HPDET_STS                        0x2000  /*           */
#define WM5100_HPDET_STS_MASK                   0x2000  /*           */
#define WM5100_HPDET_STS_SHIFT                      13  /*           */
#define WM5100_HPDET_STS_WIDTH                       1  /*           */
#define WM5100_DRC_SID_DET_STS                  0x0200  /*                 */
#define WM5100_DRC_SID_DET_STS_MASK             0x0200  /*                 */
#define WM5100_DRC_SID_DET_STS_SHIFT                 9  /*                 */
#define WM5100_DRC_SID_DET_STS_WIDTH                 1  /*                 */
#define WM5100_ASRC2_LOCK_STS                   0x0100  /*                */
#define WM5100_ASRC2_LOCK_STS_MASK              0x0100  /*                */
#define WM5100_ASRC2_LOCK_STS_SHIFT                  8  /*                */
#define WM5100_ASRC2_LOCK_STS_WIDTH                  1  /*                */
#define WM5100_ASRC1_LOCK_STS                   0x0080  /*                */
#define WM5100_ASRC1_LOCK_STS_MASK              0x0080  /*                */
#define WM5100_ASRC1_LOCK_STS_SHIFT                  7  /*                */
#define WM5100_ASRC1_LOCK_STS_WIDTH                  1  /*                */
#define WM5100_FLL2_LOCK_STS                    0x0008  /*               */
#define WM5100_FLL2_LOCK_STS_MASK               0x0008  /*               */
#define WM5100_FLL2_LOCK_STS_SHIFT                   3  /*               */
#define WM5100_FLL2_LOCK_STS_WIDTH                   1  /*               */
#define WM5100_FLL1_LOCK_STS                    0x0004  /*               */
#define WM5100_FLL1_LOCK_STS_MASK               0x0004  /*               */
#define WM5100_FLL1_LOCK_STS_SHIFT                   2  /*               */
#define WM5100_FLL1_LOCK_STS_WIDTH                   1  /*               */
#define WM5100_CLKGEN_ERR_STS                   0x0002  /*                */
#define WM5100_CLKGEN_ERR_STS_MASK              0x0002  /*                */
#define WM5100_CLKGEN_ERR_STS_SHIFT                  1  /*                */
#define WM5100_CLKGEN_ERR_STS_WIDTH                  1  /*                */
#define WM5100_CLKGEN_ERR_ASYNC_STS             0x0001  /*                      */
#define WM5100_CLKGEN_ERR_ASYNC_STS_MASK        0x0001  /*                      */
#define WM5100_CLKGEN_ERR_ASYNC_STS_SHIFT            0  /*                      */
#define WM5100_CLKGEN_ERR_ASYNC_STS_WIDTH            1  /*                      */

/*
                                         
 */
#define WM5100_AIF3_ERR_STS                     0x2000  /*              */
#define WM5100_AIF3_ERR_STS_MASK                0x2000  /*              */
#define WM5100_AIF3_ERR_STS_SHIFT                   13  /*              */
#define WM5100_AIF3_ERR_STS_WIDTH                    1  /*              */
#define WM5100_AIF2_ERR_STS                     0x1000  /*              */
#define WM5100_AIF2_ERR_STS_MASK                0x1000  /*              */
#define WM5100_AIF2_ERR_STS_SHIFT                   12  /*              */
#define WM5100_AIF2_ERR_STS_WIDTH                    1  /*              */
#define WM5100_AIF1_ERR_STS                     0x0800  /*              */
#define WM5100_AIF1_ERR_STS_MASK                0x0800  /*              */
#define WM5100_AIF1_ERR_STS_SHIFT                   11  /*              */
#define WM5100_AIF1_ERR_STS_WIDTH                    1  /*              */
#define WM5100_CTRLIF_ERR_STS                   0x0400  /*                */
#define WM5100_CTRLIF_ERR_STS_MASK              0x0400  /*                */
#define WM5100_CTRLIF_ERR_STS_SHIFT                 10  /*                */
#define WM5100_CTRLIF_ERR_STS_WIDTH                  1  /*                */
#define WM5100_ISRC2_UNDERCLOCKED_STS           0x0200  /*                        */
#define WM5100_ISRC2_UNDERCLOCKED_STS_MASK      0x0200  /*                        */
#define WM5100_ISRC2_UNDERCLOCKED_STS_SHIFT          9  /*                        */
#define WM5100_ISRC2_UNDERCLOCKED_STS_WIDTH          1  /*                        */
#define WM5100_ISRC1_UNDERCLOCKED_STS           0x0100  /*                        */
#define WM5100_ISRC1_UNDERCLOCKED_STS_MASK      0x0100  /*                        */
#define WM5100_ISRC1_UNDERCLOCKED_STS_SHIFT          8  /*                        */
#define WM5100_ISRC1_UNDERCLOCKED_STS_WIDTH          1  /*                        */
#define WM5100_FX_UNDERCLOCKED_STS              0x0080  /*                     */
#define WM5100_FX_UNDERCLOCKED_STS_MASK         0x0080  /*                     */
#define WM5100_FX_UNDERCLOCKED_STS_SHIFT             7  /*                     */
#define WM5100_FX_UNDERCLOCKED_STS_WIDTH             1  /*                     */
#define WM5100_AIF3_UNDERCLOCKED_STS            0x0040  /*                       */
#define WM5100_AIF3_UNDERCLOCKED_STS_MASK       0x0040  /*                       */
#define WM5100_AIF3_UNDERCLOCKED_STS_SHIFT           6  /*                       */
#define WM5100_AIF3_UNDERCLOCKED_STS_WIDTH           1  /*                       */
#define WM5100_AIF2_UNDERCLOCKED_STS            0x0020  /*                       */
#define WM5100_AIF2_UNDERCLOCKED_STS_MASK       0x0020  /*                       */
#define WM5100_AIF2_UNDERCLOCKED_STS_SHIFT           5  /*                       */
#define WM5100_AIF2_UNDERCLOCKED_STS_WIDTH           1  /*                       */
#define WM5100_AIF1_UNDERCLOCKED_STS            0x0010  /*                       */
#define WM5100_AIF1_UNDERCLOCKED_STS_MASK       0x0010  /*                       */
#define WM5100_AIF1_UNDERCLOCKED_STS_SHIFT           4  /*                       */
#define WM5100_AIF1_UNDERCLOCKED_STS_WIDTH           1  /*                       */
#define WM5100_ASRC_UNDERCLOCKED_STS            0x0008  /*                       */
#define WM5100_ASRC_UNDERCLOCKED_STS_MASK       0x0008  /*                       */
#define WM5100_ASRC_UNDERCLOCKED_STS_SHIFT           3  /*                       */
#define WM5100_ASRC_UNDERCLOCKED_STS_WIDTH           1  /*                       */
#define WM5100_DAC_UNDERCLOCKED_STS             0x0004  /*                      */
#define WM5100_DAC_UNDERCLOCKED_STS_MASK        0x0004  /*                      */
#define WM5100_DAC_UNDERCLOCKED_STS_SHIFT            2  /*                      */
#define WM5100_DAC_UNDERCLOCKED_STS_WIDTH            1  /*                      */
#define WM5100_ADC_UNDERCLOCKED_STS             0x0002  /*                      */
#define WM5100_ADC_UNDERCLOCKED_STS_MASK        0x0002  /*                      */
#define WM5100_ADC_UNDERCLOCKED_STS_SHIFT            1  /*                      */
#define WM5100_ADC_UNDERCLOCKED_STS_WIDTH            1  /*                      */
#define WM5100_MIXER_UNDERCLOCKED_STS           0x0001  /*                        */
#define WM5100_MIXER_UNDERCLOCKED_STS_MASK      0x0001  /*                        */
#define WM5100_MIXER_UNDERCLOCKED_STS_SHIFT          0  /*                        */
#define WM5100_MIXER_UNDERCLOCKED_STS_WIDTH          1  /*                        */

/*
                                          
 */
#define WM5100_IM_GP6_EINT                      0x0020  /*             */
#define WM5100_IM_GP6_EINT_MASK                 0x0020  /*             */
#define WM5100_IM_GP6_EINT_SHIFT                     5  /*             */
#define WM5100_IM_GP6_EINT_WIDTH                     1  /*             */
#define WM5100_IM_GP5_EINT                      0x0010  /*             */
#define WM5100_IM_GP5_EINT_MASK                 0x0010  /*             */
#define WM5100_IM_GP5_EINT_SHIFT                     4  /*             */
#define WM5100_IM_GP5_EINT_WIDTH                     1  /*             */
#define WM5100_IM_GP4_EINT                      0x0008  /*             */
#define WM5100_IM_GP4_EINT_MASK                 0x0008  /*             */
#define WM5100_IM_GP4_EINT_SHIFT                     3  /*             */
#define WM5100_IM_GP4_EINT_WIDTH                     1  /*             */
#define WM5100_IM_GP3_EINT                      0x0004  /*             */
#define WM5100_IM_GP3_EINT_MASK                 0x0004  /*             */
#define WM5100_IM_GP3_EINT_SHIFT                     2  /*             */
#define WM5100_IM_GP3_EINT_WIDTH                     1  /*             */
#define WM5100_IM_GP2_EINT                      0x0002  /*             */
#define WM5100_IM_GP2_EINT_MASK                 0x0002  /*             */
#define WM5100_IM_GP2_EINT_SHIFT                     1  /*             */
#define WM5100_IM_GP2_EINT_WIDTH                     1  /*             */
#define WM5100_IM_GP1_EINT                      0x0001  /*             */
#define WM5100_IM_GP1_EINT_MASK                 0x0001  /*             */
#define WM5100_IM_GP1_EINT_SHIFT                     0  /*             */
#define WM5100_IM_GP1_EINT_WIDTH                     1  /*             */

/*
                                          
 */
#define WM5100_IM_DSP_IRQ6_EINT                 0x0020  /*                  */
#define WM5100_IM_DSP_IRQ6_EINT_MASK            0x0020  /*                  */
#define WM5100_IM_DSP_IRQ6_EINT_SHIFT                5  /*                  */
#define WM5100_IM_DSP_IRQ6_EINT_WIDTH                1  /*                  */
#define WM5100_IM_DSP_IRQ5_EINT                 0x0010  /*                  */
#define WM5100_IM_DSP_IRQ5_EINT_MASK            0x0010  /*                  */
#define WM5100_IM_DSP_IRQ5_EINT_SHIFT                4  /*                  */
#define WM5100_IM_DSP_IRQ5_EINT_WIDTH                1  /*                  */
#define WM5100_IM_DSP_IRQ4_EINT                 0x0008  /*                  */
#define WM5100_IM_DSP_IRQ4_EINT_MASK            0x0008  /*                  */
#define WM5100_IM_DSP_IRQ4_EINT_SHIFT                3  /*                  */
#define WM5100_IM_DSP_IRQ4_EINT_WIDTH                1  /*                  */
#define WM5100_IM_DSP_IRQ3_EINT                 0x0004  /*                  */
#define WM5100_IM_DSP_IRQ3_EINT_MASK            0x0004  /*                  */
#define WM5100_IM_DSP_IRQ3_EINT_SHIFT                2  /*                  */
#define WM5100_IM_DSP_IRQ3_EINT_WIDTH                1  /*                  */
#define WM5100_IM_DSP_IRQ2_EINT                 0x0002  /*                  */
#define WM5100_IM_DSP_IRQ2_EINT_MASK            0x0002  /*                  */
#define WM5100_IM_DSP_IRQ2_EINT_SHIFT                1  /*                  */
#define WM5100_IM_DSP_IRQ2_EINT_WIDTH                1  /*                  */
#define WM5100_IM_DSP_IRQ1_EINT                 0x0001  /*                  */
#define WM5100_IM_DSP_IRQ1_EINT_MASK            0x0001  /*                  */
#define WM5100_IM_DSP_IRQ1_EINT_SHIFT                0  /*                  */
#define WM5100_IM_DSP_IRQ1_EINT_WIDTH                1  /*                  */

/*
                                          
 */
#define WM5100_IM_SPK_SHUTDOWN_WARN_EINT        0x8000  /*                           */
#define WM5100_IM_SPK_SHUTDOWN_WARN_EINT_MASK   0x8000  /*                           */
#define WM5100_IM_SPK_SHUTDOWN_WARN_EINT_SHIFT      15  /*                           */
#define WM5100_IM_SPK_SHUTDOWN_WARN_EINT_WIDTH       1  /*                           */
#define WM5100_IM_SPK_SHUTDOWN_EINT             0x4000  /*                      */
#define WM5100_IM_SPK_SHUTDOWN_EINT_MASK        0x4000  /*                      */
#define WM5100_IM_SPK_SHUTDOWN_EINT_SHIFT           14  /*                      */
#define WM5100_IM_SPK_SHUTDOWN_EINT_WIDTH            1  /*                      */
#define WM5100_IM_HPDET_EINT                    0x2000  /*               */
#define WM5100_IM_HPDET_EINT_MASK               0x2000  /*               */
#define WM5100_IM_HPDET_EINT_SHIFT                  13  /*               */
#define WM5100_IM_HPDET_EINT_WIDTH                   1  /*               */
#define WM5100_IM_ACCDET_EINT                   0x1000  /*                */
#define WM5100_IM_ACCDET_EINT_MASK              0x1000  /*                */
#define WM5100_IM_ACCDET_EINT_SHIFT                 12  /*                */
#define WM5100_IM_ACCDET_EINT_WIDTH                  1  /*                */
#define WM5100_IM_DRC_SIG_DET_EINT              0x0200  /*                     */
#define WM5100_IM_DRC_SIG_DET_EINT_MASK         0x0200  /*                     */
#define WM5100_IM_DRC_SIG_DET_EINT_SHIFT             9  /*                     */
#define WM5100_IM_DRC_SIG_DET_EINT_WIDTH             1  /*                     */
#define WM5100_IM_ASRC2_LOCK_EINT               0x0100  /*                    */
#define WM5100_IM_ASRC2_LOCK_EINT_MASK          0x0100  /*                    */
#define WM5100_IM_ASRC2_LOCK_EINT_SHIFT              8  /*                    */
#define WM5100_IM_ASRC2_LOCK_EINT_WIDTH              1  /*                    */
#define WM5100_IM_ASRC1_LOCK_EINT               0x0080  /*                    */
#define WM5100_IM_ASRC1_LOCK_EINT_MASK          0x0080  /*                    */
#define WM5100_IM_ASRC1_LOCK_EINT_SHIFT              7  /*                    */
#define WM5100_IM_ASRC1_LOCK_EINT_WIDTH              1  /*                    */
#define WM5100_IM_FLL2_LOCK_EINT                0x0008  /*                   */
#define WM5100_IM_FLL2_LOCK_EINT_MASK           0x0008  /*                   */
#define WM5100_IM_FLL2_LOCK_EINT_SHIFT               3  /*                   */
#define WM5100_IM_FLL2_LOCK_EINT_WIDTH               1  /*                   */
#define WM5100_IM_FLL1_LOCK_EINT                0x0004  /*                   */
#define WM5100_IM_FLL1_LOCK_EINT_MASK           0x0004  /*                   */
#define WM5100_IM_FLL1_LOCK_EINT_SHIFT               2  /*                   */
#define WM5100_IM_FLL1_LOCK_EINT_WIDTH               1  /*                   */
#define WM5100_IM_CLKGEN_ERR_EINT               0x0002  /*                    */
#define WM5100_IM_CLKGEN_ERR_EINT_MASK          0x0002  /*                    */
#define WM5100_IM_CLKGEN_ERR_EINT_SHIFT              1  /*                    */
#define WM5100_IM_CLKGEN_ERR_EINT_WIDTH              1  /*                    */
#define WM5100_IM_CLKGEN_ERR_ASYNC_EINT         0x0001  /*                          */
#define WM5100_IM_CLKGEN_ERR_ASYNC_EINT_MASK    0x0001  /*                          */
#define WM5100_IM_CLKGEN_ERR_ASYNC_EINT_SHIFT        0  /*                          */
#define WM5100_IM_CLKGEN_ERR_ASYNC_EINT_WIDTH        1  /*                          */

/*
                                          
 */
#define WM5100_IM_AIF3_ERR_EINT                 0x2000  /*                  */
#define WM5100_IM_AIF3_ERR_EINT_MASK            0x2000  /*                  */
#define WM5100_IM_AIF3_ERR_EINT_SHIFT               13  /*                  */
#define WM5100_IM_AIF3_ERR_EINT_WIDTH                1  /*                  */
#define WM5100_IM_AIF2_ERR_EINT                 0x1000  /*                  */
#define WM5100_IM_AIF2_ERR_EINT_MASK            0x1000  /*                  */
#define WM5100_IM_AIF2_ERR_EINT_SHIFT               12  /*                  */
#define WM5100_IM_AIF2_ERR_EINT_WIDTH                1  /*                  */
#define WM5100_IM_AIF1_ERR_EINT                 0x0800  /*                  */
#define WM5100_IM_AIF1_ERR_EINT_MASK            0x0800  /*                  */
#define WM5100_IM_AIF1_ERR_EINT_SHIFT               11  /*                  */
#define WM5100_IM_AIF1_ERR_EINT_WIDTH                1  /*                  */
#define WM5100_IM_CTRLIF_ERR_EINT               0x0400  /*                    */
#define WM5100_IM_CTRLIF_ERR_EINT_MASK          0x0400  /*                    */
#define WM5100_IM_CTRLIF_ERR_EINT_SHIFT             10  /*                    */
#define WM5100_IM_CTRLIF_ERR_EINT_WIDTH              1  /*                    */
#define WM5100_IM_ISRC2_UNDERCLOCKED_EINT       0x0200  /*                            */
#define WM5100_IM_ISRC2_UNDERCLOCKED_EINT_MASK  0x0200  /*                            */
#define WM5100_IM_ISRC2_UNDERCLOCKED_EINT_SHIFT      9  /*                            */
#define WM5100_IM_ISRC2_UNDERCLOCKED_EINT_WIDTH      1  /*                            */
#define WM5100_IM_ISRC1_UNDERCLOCKED_EINT       0x0100  /*                            */
#define WM5100_IM_ISRC1_UNDERCLOCKED_EINT_MASK  0x0100  /*                            */
#define WM5100_IM_ISRC1_UNDERCLOCKED_EINT_SHIFT      8  /*                            */
#define WM5100_IM_ISRC1_UNDERCLOCKED_EINT_WIDTH      1  /*                            */
#define WM5100_IM_FX_UNDERCLOCKED_EINT          0x0080  /*                         */
#define WM5100_IM_FX_UNDERCLOCKED_EINT_MASK     0x0080  /*                         */
#define WM5100_IM_FX_UNDERCLOCKED_EINT_SHIFT         7  /*                         */
#define WM5100_IM_FX_UNDERCLOCKED_EINT_WIDTH         1  /*                         */
#define WM5100_IM_AIF3_UNDERCLOCKED_EINT        0x0040  /*                           */
#define WM5100_IM_AIF3_UNDERCLOCKED_EINT_MASK   0x0040  /*                           */
#define WM5100_IM_AIF3_UNDERCLOCKED_EINT_SHIFT       6  /*                           */
#define WM5100_IM_AIF3_UNDERCLOCKED_EINT_WIDTH       1  /*                           */
#define WM5100_IM_AIF2_UNDERCLOCKED_EINT        0x0020  /*                           */
#define WM5100_IM_AIF2_UNDERCLOCKED_EINT_MASK   0x0020  /*                           */
#define WM5100_IM_AIF2_UNDERCLOCKED_EINT_SHIFT       5  /*                           */
#define WM5100_IM_AIF2_UNDERCLOCKED_EINT_WIDTH       1  /*                           */
#define WM5100_IM_AIF1_UNDERCLOCKED_EINT        0x0010  /*                           */
#define WM5100_IM_AIF1_UNDERCLOCKED_EINT_MASK   0x0010  /*                           */
#define WM5100_IM_AIF1_UNDERCLOCKED_EINT_SHIFT       4  /*                           */
#define WM5100_IM_AIF1_UNDERCLOCKED_EINT_WIDTH       1  /*                           */
#define WM5100_IM_ASRC_UNDERCLOCKED_EINT        0x0008  /*                           */
#define WM5100_IM_ASRC_UNDERCLOCKED_EINT_MASK   0x0008  /*                           */
#define WM5100_IM_ASRC_UNDERCLOCKED_EINT_SHIFT       3  /*                           */
#define WM5100_IM_ASRC_UNDERCLOCKED_EINT_WIDTH       1  /*                           */
#define WM5100_IM_DAC_UNDERCLOCKED_EINT         0x0004  /*                          */
#define WM5100_IM_DAC_UNDERCLOCKED_EINT_MASK    0x0004  /*                          */
#define WM5100_IM_DAC_UNDERCLOCKED_EINT_SHIFT        2  /*                          */
#define WM5100_IM_DAC_UNDERCLOCKED_EINT_WIDTH        1  /*                          */
#define WM5100_IM_ADC_UNDERCLOCKED_EINT         0x0002  /*                          */
#define WM5100_IM_ADC_UNDERCLOCKED_EINT_MASK    0x0002  /*                          */
#define WM5100_IM_ADC_UNDERCLOCKED_EINT_SHIFT        1  /*                          */
#define WM5100_IM_ADC_UNDERCLOCKED_EINT_WIDTH        1  /*                          */
#define WM5100_IM_MIXER_UNDERCLOCKED_EINT       0x0001  /*                            */
#define WM5100_IM_MIXER_UNDERCLOCKED_EINT_MASK  0x0001  /*                            */
#define WM5100_IM_MIXER_UNDERCLOCKED_EINT_SHIFT      0  /*                            */
#define WM5100_IM_MIXER_UNDERCLOCKED_EINT_WIDTH      1  /*                            */

/*
                                    
 */
#define WM5100_IM_IRQ                           0x0001  /*        */
#define WM5100_IM_IRQ_MASK                      0x0001  /*        */
#define WM5100_IM_IRQ_SHIFT                          0  /*        */
#define WM5100_IM_IRQ_WIDTH                          1  /*        */

/*
                                 
 */
#define WM5100_SPK_SHUTDOWN_WARN_DB             0x0200  /*                      */
#define WM5100_SPK_SHUTDOWN_WARN_DB_MASK        0x0200  /*                      */
#define WM5100_SPK_SHUTDOWN_WARN_DB_SHIFT            9  /*                      */
#define WM5100_SPK_SHUTDOWN_WARN_DB_WIDTH            1  /*                      */
#define WM5100_SPK_SHUTDOWN_DB                  0x0100  /*                 */
#define WM5100_SPK_SHUTDOWN_DB_MASK             0x0100  /*                 */
#define WM5100_SPK_SHUTDOWN_DB_SHIFT                 8  /*                 */
#define WM5100_SPK_SHUTDOWN_DB_WIDTH                 1  /*                 */
#define WM5100_FLL1_LOCK_IRQ_DB                 0x0008  /*                  */
#define WM5100_FLL1_LOCK_IRQ_DB_MASK            0x0008  /*                  */
#define WM5100_FLL1_LOCK_IRQ_DB_SHIFT                3  /*                  */
#define WM5100_FLL1_LOCK_IRQ_DB_WIDTH                1  /*                  */
#define WM5100_FLL2_LOCK_IRQ_DB                 0x0004  /*                  */
#define WM5100_FLL2_LOCK_IRQ_DB_MASK            0x0004  /*                  */
#define WM5100_FLL2_LOCK_IRQ_DB_SHIFT                2  /*                  */
#define WM5100_FLL2_LOCK_IRQ_DB_WIDTH                1  /*                  */
#define WM5100_CLKGEN_ERR_IRQ_DB                0x0002  /*                   */
#define WM5100_CLKGEN_ERR_IRQ_DB_MASK           0x0002  /*                   */
#define WM5100_CLKGEN_ERR_IRQ_DB_SHIFT               1  /*                   */
#define WM5100_CLKGEN_ERR_IRQ_DB_WIDTH               1  /*                   */
#define WM5100_CLKGEN_ERR_ASYNC_IRQ_DB          0x0001  /*                         */
#define WM5100_CLKGEN_ERR_ASYNC_IRQ_DB_MASK     0x0001  /*                         */
#define WM5100_CLKGEN_ERR_ASYNC_IRQ_DB_SHIFT         0  /*                         */
#define WM5100_CLKGEN_ERR_ASYNC_IRQ_DB_WIDTH         1  /*                         */

/*
                                 
 */
#define WM5100_AIF_ERR_DB                       0x0001  /*            */
#define WM5100_AIF_ERR_DB_MASK                  0x0001  /*            */
#define WM5100_AIF_ERR_DB_SHIFT                      0  /*            */
#define WM5100_AIF_ERR_DB_WIDTH                      1  /*            */

/*
                          
 */
#define WM5100_FX_STS_MASK                      0xFFC0  /*                 */
#define WM5100_FX_STS_SHIFT                          6  /*                 */
#define WM5100_FX_STS_WIDTH                         10  /*                 */
#define WM5100_FX_RATE_MASK                     0x0003  /*                 */
#define WM5100_FX_RATE_SHIFT                         0  /*                 */
#define WM5100_FX_RATE_WIDTH                         2  /*                 */

/*
                        
 */
#define WM5100_EQ1_B1_GAIN_MASK                 0xF800  /*                       */
#define WM5100_EQ1_B1_GAIN_SHIFT                    11  /*                       */
#define WM5100_EQ1_B1_GAIN_WIDTH                     5  /*                       */
#define WM5100_EQ1_B2_GAIN_MASK                 0x07C0  /*                      */
#define WM5100_EQ1_B2_GAIN_SHIFT                     6  /*                      */
#define WM5100_EQ1_B2_GAIN_WIDTH                     5  /*                      */
#define WM5100_EQ1_B3_GAIN_MASK                 0x003E  /*                     */
#define WM5100_EQ1_B3_GAIN_SHIFT                     1  /*                     */
#define WM5100_EQ1_B3_GAIN_WIDTH                     5  /*                     */
#define WM5100_EQ1_ENA                          0x0001  /*         */
#define WM5100_EQ1_ENA_MASK                     0x0001  /*         */
#define WM5100_EQ1_ENA_SHIFT                         0  /*         */
#define WM5100_EQ1_ENA_WIDTH                         1  /*         */

/*
                        
 */
#define WM5100_EQ1_B4_GAIN_MASK                 0xF800  /*                       */
#define WM5100_EQ1_B4_GAIN_SHIFT                    11  /*                       */
#define WM5100_EQ1_B4_GAIN_WIDTH                     5  /*                       */
#define WM5100_EQ1_B5_GAIN_MASK                 0x07C0  /*                      */
#define WM5100_EQ1_B5_GAIN_SHIFT                     6  /*                      */
#define WM5100_EQ1_B5_GAIN_WIDTH                     5  /*                      */

/*
                        
 */
#define WM5100_EQ1_B1_A_MASK                    0xFFFF  /*                   */
#define WM5100_EQ1_B1_A_SHIFT                        0  /*                   */
#define WM5100_EQ1_B1_A_WIDTH                       16  /*                   */

/*
                        
 */
#define WM5100_EQ1_B1_B_MASK                    0xFFFF  /*                   */
#define WM5100_EQ1_B1_B_SHIFT                        0  /*                   */
#define WM5100_EQ1_B1_B_WIDTH                       16  /*                   */

/*
                        
 */
#define WM5100_EQ1_B1_PG_MASK                   0xFFFF  /*                    */
#define WM5100_EQ1_B1_PG_SHIFT                       0  /*                    */
#define WM5100_EQ1_B1_PG_WIDTH                      16  /*                    */

/*
                        
 */
#define WM5100_EQ1_B2_A_MASK                    0xFFFF  /*                   */
#define WM5100_EQ1_B2_A_SHIFT                        0  /*                   */
#define WM5100_EQ1_B2_A_WIDTH                       16  /*                   */

/*
                        
 */
#define WM5100_EQ1_B2_B_MASK                    0xFFFF  /*                   */
#define WM5100_EQ1_B2_B_SHIFT                        0  /*                   */
#define WM5100_EQ1_B2_B_WIDTH                       16  /*                   */

/*
                        
 */
#define WM5100_EQ1_B2_C_MASK                    0xFFFF  /*                   */
#define WM5100_EQ1_B2_C_SHIFT                        0  /*                   */
#define WM5100_EQ1_B2_C_WIDTH                       16  /*                   */

/*
                        
 */
#define WM5100_EQ1_B2_PG_MASK                   0xFFFF  /*                    */
#define WM5100_EQ1_B2_PG_SHIFT                       0  /*                    */
#define WM5100_EQ1_B2_PG_WIDTH                      16  /*                    */

/*
                         
 */
#define WM5100_EQ1_B3_A_MASK                    0xFFFF  /*                   */
#define WM5100_EQ1_B3_A_SHIFT                        0  /*                   */
#define WM5100_EQ1_B3_A_WIDTH                       16  /*                   */

/*
                         
 */
#define WM5100_EQ1_B3_B_MASK                    0xFFFF  /*                   */
#define WM5100_EQ1_B3_B_SHIFT                        0  /*                   */
#define WM5100_EQ1_B3_B_WIDTH                       16  /*                   */

/*
                         
 */
#define WM5100_EQ1_B3_C_MASK                    0xFFFF  /*                   */
#define WM5100_EQ1_B3_C_SHIFT                        0  /*                   */
#define WM5100_EQ1_B3_C_WIDTH                       16  /*                   */

/*
                         
 */
#define WM5100_EQ1_B3_PG_MASK                   0xFFFF  /*                    */
#define WM5100_EQ1_B3_PG_SHIFT                       0  /*                    */
#define WM5100_EQ1_B3_PG_WIDTH                      16  /*                    */

/*
                         
 */
#define WM5100_EQ1_B4_A_MASK                    0xFFFF  /*                   */
#define WM5100_EQ1_B4_A_SHIFT                        0  /*                   */
#define WM5100_EQ1_B4_A_WIDTH                       16  /*                   */

/*
                         
 */
#define WM5100_EQ1_B4_B_MASK                    0xFFFF  /*                   */
#define WM5100_EQ1_B4_B_SHIFT                        0  /*                   */
#define WM5100_EQ1_B4_B_WIDTH                       16  /*                   */

/*
                         
 */
#define WM5100_EQ1_B4_C_MASK                    0xFFFF  /*                   */
#define WM5100_EQ1_B4_C_SHIFT                        0  /*                   */
#define WM5100_EQ1_B4_C_WIDTH                       16  /*                   */

/*
                         
 */
#define WM5100_EQ1_B4_PG_MASK                   0xFFFF  /*                    */
#define WM5100_EQ1_B4_PG_SHIFT                       0  /*                    */
#define WM5100_EQ1_B4_PG_WIDTH                      16  /*                    */

/*
                         
 */
#define WM5100_EQ1_B5_A_MASK                    0xFFFF  /*                   */
#define WM5100_EQ1_B5_A_SHIFT                        0  /*                   */
#define WM5100_EQ1_B5_A_WIDTH                       16  /*                   */

/*
                         
 */
#define WM5100_EQ1_B5_B_MASK                    0xFFFF  /*                   */
#define WM5100_EQ1_B5_B_SHIFT                        0  /*                   */
#define WM5100_EQ1_B5_B_WIDTH                       16  /*                   */

/*
                         
 */
#define WM5100_EQ1_B5_PG_MASK                   0xFFFF  /*                    */
#define WM5100_EQ1_B5_PG_SHIFT                       0  /*                    */
#define WM5100_EQ1_B5_PG_WIDTH                      16  /*                    */

/*
                        
 */
#define WM5100_EQ2_B1_GAIN_MASK                 0xF800  /*                       */
#define WM5100_EQ2_B1_GAIN_SHIFT                    11  /*                       */
#define WM5100_EQ2_B1_GAIN_WIDTH                     5  /*                       */
#define WM5100_EQ2_B2_GAIN_MASK                 0x07C0  /*                      */
#define WM5100_EQ2_B2_GAIN_SHIFT                     6  /*                      */
#define WM5100_EQ2_B2_GAIN_WIDTH                     5  /*                      */
#define WM5100_EQ2_B3_GAIN_MASK                 0x003E  /*                     */
#define WM5100_EQ2_B3_GAIN_SHIFT                     1  /*                     */
#define WM5100_EQ2_B3_GAIN_WIDTH                     5  /*                     */
#define WM5100_EQ2_ENA                          0x0001  /*         */
#define WM5100_EQ2_ENA_MASK                     0x0001  /*         */
#define WM5100_EQ2_ENA_SHIFT                         0  /*         */
#define WM5100_EQ2_ENA_WIDTH                         1  /*         */

/*
                        
 */
#define WM5100_EQ2_B4_GAIN_MASK                 0xF800  /*                       */
#define WM5100_EQ2_B4_GAIN_SHIFT                    11  /*                       */
#define WM5100_EQ2_B4_GAIN_WIDTH                     5  /*                       */
#define WM5100_EQ2_B5_GAIN_MASK                 0x07C0  /*                      */
#define WM5100_EQ2_B5_GAIN_SHIFT                     6  /*                      */
#define WM5100_EQ2_B5_GAIN_WIDTH                     5  /*                      */

/*
                        
 */
#define WM5100_EQ2_B1_A_MASK                    0xFFFF  /*                   */
#define WM5100_EQ2_B1_A_SHIFT                        0  /*                   */
#define WM5100_EQ2_B1_A_WIDTH                       16  /*                   */

/*
                        
 */
#define WM5100_EQ2_B1_B_MASK                    0xFFFF  /*                   */
#define WM5100_EQ2_B1_B_SHIFT                        0  /*                   */
#define WM5100_EQ2_B1_B_WIDTH                       16  /*                   */

/*
                        
 */
#define WM5100_EQ2_B1_PG_MASK                   0xFFFF  /*                    */
#define WM5100_EQ2_B1_PG_SHIFT                       0  /*                    */
#define WM5100_EQ2_B1_PG_WIDTH                      16  /*                    */

/*
                        
 */
#define WM5100_EQ2_B2_A_MASK                    0xFFFF  /*                   */
#define WM5100_EQ2_B2_A_SHIFT                        0  /*                   */
#define WM5100_EQ2_B2_A_WIDTH                       16  /*                   */

/*
                        
 */
#define WM5100_EQ2_B2_B_MASK                    0xFFFF  /*                   */
#define WM5100_EQ2_B2_B_SHIFT                        0  /*                   */
#define WM5100_EQ2_B2_B_WIDTH                       16  /*                   */

/*
                        
 */
#define WM5100_EQ2_B2_C_MASK                    0xFFFF  /*                   */
#define WM5100_EQ2_B2_C_SHIFT                        0  /*                   */
#define WM5100_EQ2_B2_C_WIDTH                       16  /*                   */

/*
                        
 */
#define WM5100_EQ2_B2_PG_MASK                   0xFFFF  /*                    */
#define WM5100_EQ2_B2_PG_SHIFT                       0  /*                    */
#define WM5100_EQ2_B2_PG_WIDTH                      16  /*                    */

/*
                         
 */
#define WM5100_EQ2_B3_A_MASK                    0xFFFF  /*                   */
#define WM5100_EQ2_B3_A_SHIFT                        0  /*                   */
#define WM5100_EQ2_B3_A_WIDTH                       16  /*                   */

/*
                         
 */
#define WM5100_EQ2_B3_B_MASK                    0xFFFF  /*                   */
#define WM5100_EQ2_B3_B_SHIFT                        0  /*                   */
#define WM5100_EQ2_B3_B_WIDTH                       16  /*                   */

/*
                         
 */
#define WM5100_EQ2_B3_C_MASK                    0xFFFF  /*                   */
#define WM5100_EQ2_B3_C_SHIFT                        0  /*                   */
#define WM5100_EQ2_B3_C_WIDTH                       16  /*                   */

/*
                         
 */
#define WM5100_EQ2_B3_PG_MASK                   0xFFFF  /*                    */
#define WM5100_EQ2_B3_PG_SHIFT                       0  /*                    */
#define WM5100_EQ2_B3_PG_WIDTH                      16  /*                    */

/*
                         
 */
#define WM5100_EQ2_B4_A_MASK                    0xFFFF  /*                   */
#define WM5100_EQ2_B4_A_SHIFT                        0  /*                   */
#define WM5100_EQ2_B4_A_WIDTH                       16  /*                   */

/*
                         
 */
#define WM5100_EQ2_B4_B_MASK                    0xFFFF  /*                   */
#define WM5100_EQ2_B4_B_SHIFT                        0  /*                   */
#define WM5100_EQ2_B4_B_WIDTH                       16  /*                   */

/*
                         
 */
#define WM5100_EQ2_B4_C_MASK                    0xFFFF  /*                   */
#define WM5100_EQ2_B4_C_SHIFT                        0  /*                   */
#define WM5100_EQ2_B4_C_WIDTH                       16  /*                   */

/*
                         
 */
#define WM5100_EQ2_B4_PG_MASK                   0xFFFF  /*                    */
#define WM5100_EQ2_B4_PG_SHIFT                       0  /*                    */
#define WM5100_EQ2_B4_PG_WIDTH                      16  /*                    */

/*
                         
 */
#define WM5100_EQ2_B5_A_MASK                    0xFFFF  /*                   */
#define WM5100_EQ2_B5_A_SHIFT                        0  /*                   */
#define WM5100_EQ2_B5_A_WIDTH                       16  /*                   */

/*
                         
 */
#define WM5100_EQ2_B5_B_MASK                    0xFFFF  /*                   */
#define WM5100_EQ2_B5_B_SHIFT                        0  /*                   */
#define WM5100_EQ2_B5_B_WIDTH                       16  /*                   */

/*
                         
 */
#define WM5100_EQ2_B5_PG_MASK                   0xFFFF  /*                    */
#define WM5100_EQ2_B5_PG_SHIFT                       0  /*                    */
#define WM5100_EQ2_B5_PG_WIDTH                      16  /*                    */

/*
                        
 */
#define WM5100_EQ3_B1_GAIN_MASK                 0xF800  /*                       */
#define WM5100_EQ3_B1_GAIN_SHIFT                    11  /*                       */
#define WM5100_EQ3_B1_GAIN_WIDTH                     5  /*                       */
#define WM5100_EQ3_B2_GAIN_MASK                 0x07C0  /*                      */
#define WM5100_EQ3_B2_GAIN_SHIFT                     6  /*                      */
#define WM5100_EQ3_B2_GAIN_WIDTH                     5  /*                      */
#define WM5100_EQ3_B3_GAIN_MASK                 0x003E  /*                     */
#define WM5100_EQ3_B3_GAIN_SHIFT                     1  /*                     */
#define WM5100_EQ3_B3_GAIN_WIDTH                     5  /*                     */
#define WM5100_EQ3_ENA                          0x0001  /*         */
#define WM5100_EQ3_ENA_MASK                     0x0001  /*         */
#define WM5100_EQ3_ENA_SHIFT                         0  /*         */
#define WM5100_EQ3_ENA_WIDTH                         1  /*         */

/*
                        
 */
#define WM5100_EQ3_B4_GAIN_MASK                 0xF800  /*                       */
#define WM5100_EQ3_B4_GAIN_SHIFT                    11  /*                       */
#define WM5100_EQ3_B4_GAIN_WIDTH                     5  /*                       */
#define WM5100_EQ3_B5_GAIN_MASK                 0x07C0  /*                      */
#define WM5100_EQ3_B5_GAIN_SHIFT                     6  /*                      */
#define WM5100_EQ3_B5_GAIN_WIDTH                     5  /*                      */

/*
                        
 */
#define WM5100_EQ3_B1_A_MASK                    0xFFFF  /*                   */
#define WM5100_EQ3_B1_A_SHIFT                        0  /*                   */
#define WM5100_EQ3_B1_A_WIDTH                       16  /*                   */

/*
                        
 */
#define WM5100_EQ3_B1_B_MASK                    0xFFFF  /*                   */
#define WM5100_EQ3_B1_B_SHIFT                        0  /*                   */
#define WM5100_EQ3_B1_B_WIDTH                       16  /*                   */

/*
                        
 */
#define WM5100_EQ3_B1_PG_MASK                   0xFFFF  /*                    */
#define WM5100_EQ3_B1_PG_SHIFT                       0  /*                    */
#define WM5100_EQ3_B1_PG_WIDTH                      16  /*                    */

/*
                        
 */
#define WM5100_EQ3_B2_A_MASK                    0xFFFF  /*                   */
#define WM5100_EQ3_B2_A_SHIFT                        0  /*                   */
#define WM5100_EQ3_B2_A_WIDTH                       16  /*                   */

/*
                        
 */
#define WM5100_EQ3_B2_B_MASK                    0xFFFF  /*                   */
#define WM5100_EQ3_B2_B_SHIFT                        0  /*                   */
#define WM5100_EQ3_B2_B_WIDTH                       16  /*                   */

/*
                        
 */
#define WM5100_EQ3_B2_C_MASK                    0xFFFF  /*                   */
#define WM5100_EQ3_B2_C_SHIFT                        0  /*                   */
#define WM5100_EQ3_B2_C_WIDTH                       16  /*                   */

/*
                        
 */
#define WM5100_EQ3_B2_PG_MASK                   0xFFFF  /*                    */
#define WM5100_EQ3_B2_PG_SHIFT                       0  /*                    */
#define WM5100_EQ3_B2_PG_WIDTH                      16  /*                    */

/*
                         
 */
#define WM5100_EQ3_B3_A_MASK                    0xFFFF  /*                   */
#define WM5100_EQ3_B3_A_SHIFT                        0  /*                   */
#define WM5100_EQ3_B3_A_WIDTH                       16  /*                   */

/*
                         
 */
#define WM5100_EQ3_B3_B_MASK                    0xFFFF  /*                   */
#define WM5100_EQ3_B3_B_SHIFT                        0  /*                   */
#define WM5100_EQ3_B3_B_WIDTH                       16  /*                   */

/*
                         
 */
#define WM5100_EQ3_B3_C_MASK                    0xFFFF  /*                   */
#define WM5100_EQ3_B3_C_SHIFT                        0  /*                   */
#define WM5100_EQ3_B3_C_WIDTH                       16  /*                   */

/*
                         
 */
#define WM5100_EQ3_B3_PG_MASK                   0xFFFF  /*                    */
#define WM5100_EQ3_B3_PG_SHIFT                       0  /*                    */
#define WM5100_EQ3_B3_PG_WIDTH                      16  /*                    */

/*
                         
 */
#define WM5100_EQ3_B4_A_MASK                    0xFFFF  /*                   */
#define WM5100_EQ3_B4_A_SHIFT                        0  /*                   */
#define WM5100_EQ3_B4_A_WIDTH                       16  /*                   */

/*
                         
 */
#define WM5100_EQ3_B4_B_MASK                    0xFFFF  /*                   */
#define WM5100_EQ3_B4_B_SHIFT                        0  /*                   */
#define WM5100_EQ3_B4_B_WIDTH                       16  /*                   */

/*
                         
 */
#define WM5100_EQ3_B4_C_MASK                    0xFFFF  /*                   */
#define WM5100_EQ3_B4_C_SHIFT                        0  /*                   */
#define WM5100_EQ3_B4_C_WIDTH                       16  /*                   */

/*
                         
 */
#define WM5100_EQ3_B4_PG_MASK                   0xFFFF  /*                    */
#define WM5100_EQ3_B4_PG_SHIFT                       0  /*                    */
#define WM5100_EQ3_B4_PG_WIDTH                      16  /*                    */

/*
                         
 */
#define WM5100_EQ3_B5_A_MASK                    0xFFFF  /*                   */
#define WM5100_EQ3_B5_A_SHIFT                        0  /*                   */
#define WM5100_EQ3_B5_A_WIDTH                       16  /*                   */

/*
                         
 */
#define WM5100_EQ3_B5_B_MASK                    0xFFFF  /*                   */
#define WM5100_EQ3_B5_B_SHIFT                        0  /*                   */
#define WM5100_EQ3_B5_B_WIDTH                       16  /*                   */

/*
                         
 */
#define WM5100_EQ3_B5_PG_MASK                   0xFFFF  /*                    */
#define WM5100_EQ3_B5_PG_SHIFT                       0  /*                    */
#define WM5100_EQ3_B5_PG_WIDTH                      16  /*                    */

/*
                        
 */
#define WM5100_EQ4_B1_GAIN_MASK                 0xF800  /*                       */
#define WM5100_EQ4_B1_GAIN_SHIFT                    11  /*                       */
#define WM5100_EQ4_B1_GAIN_WIDTH                     5  /*                       */
#define WM5100_EQ4_B2_GAIN_MASK                 0x07C0  /*                      */
#define WM5100_EQ4_B2_GAIN_SHIFT                     6  /*                      */
#define WM5100_EQ4_B2_GAIN_WIDTH                     5  /*                      */
#define WM5100_EQ4_B3_GAIN_MASK                 0x003E  /*                     */
#define WM5100_EQ4_B3_GAIN_SHIFT                     1  /*                     */
#define WM5100_EQ4_B3_GAIN_WIDTH                     5  /*                     */
#define WM5100_EQ4_ENA                          0x0001  /*         */
#define WM5100_EQ4_ENA_MASK                     0x0001  /*         */
#define WM5100_EQ4_ENA_SHIFT                         0  /*         */
#define WM5100_EQ4_ENA_WIDTH                         1  /*         */

/*
                        
 */
#define WM5100_EQ4_B4_GAIN_MASK                 0xF800  /*                       */
#define WM5100_EQ4_B4_GAIN_SHIFT                    11  /*                       */
#define WM5100_EQ4_B4_GAIN_WIDTH                     5  /*                       */
#define WM5100_EQ4_B5_GAIN_MASK                 0x07C0  /*                      */
#define WM5100_EQ4_B5_GAIN_SHIFT                     6  /*                      */
#define WM5100_EQ4_B5_GAIN_WIDTH                     5  /*                      */

/*
                        
 */
#define WM5100_EQ4_B1_A_MASK                    0xFFFF  /*                   */
#define WM5100_EQ4_B1_A_SHIFT                        0  /*                   */
#define WM5100_EQ4_B1_A_WIDTH                       16  /*                   */

/*
                        
 */
#define WM5100_EQ4_B1_B_MASK                    0xFFFF  /*                   */
#define WM5100_EQ4_B1_B_SHIFT                        0  /*                   */
#define WM5100_EQ4_B1_B_WIDTH                       16  /*                   */

/*
                        
 */
#define WM5100_EQ4_B1_PG_MASK                   0xFFFF  /*                    */
#define WM5100_EQ4_B1_PG_SHIFT                       0  /*                    */
#define WM5100_EQ4_B1_PG_WIDTH                      16  /*                    */

/*
                        
 */
#define WM5100_EQ4_B2_A_MASK                    0xFFFF  /*                   */
#define WM5100_EQ4_B2_A_SHIFT                        0  /*                   */
#define WM5100_EQ4_B2_A_WIDTH                       16  /*                   */

/*
                        
 */
#define WM5100_EQ4_B2_B_MASK                    0xFFFF  /*                   */
#define WM5100_EQ4_B2_B_SHIFT                        0  /*                   */
#define WM5100_EQ4_B2_B_WIDTH                       16  /*                   */

/*
                        
 */
#define WM5100_EQ4_B2_C_MASK                    0xFFFF  /*                   */
#define WM5100_EQ4_B2_C_SHIFT                        0  /*                   */
#define WM5100_EQ4_B2_C_WIDTH                       16  /*                   */

/*
                        
 */
#define WM5100_EQ4_B2_PG_MASK                   0xFFFF  /*                    */
#define WM5100_EQ4_B2_PG_SHIFT                       0  /*                    */
#define WM5100_EQ4_B2_PG_WIDTH                      16  /*                    */

/*
                         
 */
#define WM5100_EQ4_B3_A_MASK                    0xFFFF  /*                   */
#define WM5100_EQ4_B3_A_SHIFT                        0  /*                   */
#define WM5100_EQ4_B3_A_WIDTH                       16  /*                   */

/*
                         
 */
#define WM5100_EQ4_B3_B_MASK                    0xFFFF  /*                   */
#define WM5100_EQ4_B3_B_SHIFT                        0  /*                   */
#define WM5100_EQ4_B3_B_WIDTH                       16  /*                   */

/*
                         
 */
#define WM5100_EQ4_B3_C_MASK                    0xFFFF  /*                   */
#define WM5100_EQ4_B3_C_SHIFT                        0  /*                   */
#define WM5100_EQ4_B3_C_WIDTH                       16  /*                   */

/*
                         
 */
#define WM5100_EQ4_B3_PG_MASK                   0xFFFF  /*                    */
#define WM5100_EQ4_B3_PG_SHIFT                       0  /*                    */
#define WM5100_EQ4_B3_PG_WIDTH                      16  /*                    */

/*
                         
 */
#define WM5100_EQ4_B4_A_MASK                    0xFFFF  /*                   */
#define WM5100_EQ4_B4_A_SHIFT                        0  /*                   */
#define WM5100_EQ4_B4_A_WIDTH                       16  /*                   */

/*
                         
 */
#define WM5100_EQ4_B4_B_MASK                    0xFFFF  /*                   */
#define WM5100_EQ4_B4_B_SHIFT                        0  /*                   */
#define WM5100_EQ4_B4_B_WIDTH                       16  /*                   */

/*
                         
 */
#define WM5100_EQ4_B4_C_MASK                    0xFFFF  /*                   */
#define WM5100_EQ4_B4_C_SHIFT                        0  /*                   */
#define WM5100_EQ4_B4_C_WIDTH                       16  /*                   */

/*
                         
 */
#define WM5100_EQ4_B4_PG_MASK                   0xFFFF  /*                    */
#define WM5100_EQ4_B4_PG_SHIFT                       0  /*                    */
#define WM5100_EQ4_B4_PG_WIDTH                      16  /*                    */

/*
                         
 */
#define WM5100_EQ4_B5_A_MASK                    0xFFFF  /*                   */
#define WM5100_EQ4_B5_A_SHIFT                        0  /*                   */
#define WM5100_EQ4_B5_A_WIDTH                       16  /*                   */

/*
                         
 */
#define WM5100_EQ4_B5_B_MASK                    0xFFFF  /*                   */
#define WM5100_EQ4_B5_B_SHIFT                        0  /*                   */
#define WM5100_EQ4_B5_B_WIDTH                       16  /*                   */

/*
                         
 */
#define WM5100_EQ4_B5_PG_MASK                   0xFFFF  /*                    */
#define WM5100_EQ4_B5_PG_SHIFT                       0  /*                    */
#define WM5100_EQ4_B5_PG_WIDTH                      16  /*                    */

/*
                             
 */
#define WM5100_DRC_SIG_DET_RMS_MASK             0xF800  /*                           */
#define WM5100_DRC_SIG_DET_RMS_SHIFT                11  /*                           */
#define WM5100_DRC_SIG_DET_RMS_WIDTH                 5  /*                           */
#define WM5100_DRC_SIG_DET_PK_MASK              0x0600  /*                         */
#define WM5100_DRC_SIG_DET_PK_SHIFT                  9  /*                         */
#define WM5100_DRC_SIG_DET_PK_WIDTH                  2  /*                         */
#define WM5100_DRC_NG_ENA                       0x0100  /*            */
#define WM5100_DRC_NG_ENA_MASK                  0x0100  /*            */
#define WM5100_DRC_NG_ENA_SHIFT                      8  /*            */
#define WM5100_DRC_NG_ENA_WIDTH                      1  /*            */
#define WM5100_DRC_SIG_DET_MODE                 0x0080  /*                  */
#define WM5100_DRC_SIG_DET_MODE_MASK            0x0080  /*                  */
#define WM5100_DRC_SIG_DET_MODE_SHIFT                7  /*                  */
#define WM5100_DRC_SIG_DET_MODE_WIDTH                1  /*                  */
#define WM5100_DRC_SIG_DET                      0x0040  /*             */
#define WM5100_DRC_SIG_DET_MASK                 0x0040  /*             */
#define WM5100_DRC_SIG_DET_SHIFT                     6  /*             */
#define WM5100_DRC_SIG_DET_WIDTH                     1  /*             */
#define WM5100_DRC_KNEE2_OP_ENA                 0x0020  /*                  */
#define WM5100_DRC_KNEE2_OP_ENA_MASK            0x0020  /*                  */
#define WM5100_DRC_KNEE2_OP_ENA_SHIFT                5  /*                  */
#define WM5100_DRC_KNEE2_OP_ENA_WIDTH                1  /*                  */
#define WM5100_DRC_QR                           0x0010  /*        */
#define WM5100_DRC_QR_MASK                      0x0010  /*        */
#define WM5100_DRC_QR_SHIFT                          4  /*        */
#define WM5100_DRC_QR_WIDTH                          1  /*        */
#define WM5100_DRC_ANTICLIP                     0x0008  /*              */
#define WM5100_DRC_ANTICLIP_MASK                0x0008  /*              */
#define WM5100_DRC_ANTICLIP_SHIFT                    3  /*              */
#define WM5100_DRC_ANTICLIP_WIDTH                    1  /*              */
#define WM5100_DRCL_ENA                         0x0002  /*          */
#define WM5100_DRCL_ENA_MASK                    0x0002  /*          */
#define WM5100_DRCL_ENA_SHIFT                        1  /*          */
#define WM5100_DRCL_ENA_WIDTH                        1  /*          */
#define WM5100_DRCR_ENA                         0x0001  /*          */
#define WM5100_DRCR_ENA_MASK                    0x0001  /*          */
#define WM5100_DRCR_ENA_SHIFT                        0  /*          */
#define WM5100_DRCR_ENA_WIDTH                        1  /*          */

/*
                             
 */
#define WM5100_DRC_ATK_MASK                     0x1E00  /*                  */
#define WM5100_DRC_ATK_SHIFT                         9  /*                  */
#define WM5100_DRC_ATK_WIDTH                         4  /*                  */
#define WM5100_DRC_DCY_MASK                     0x01E0  /*                 */
#define WM5100_DRC_DCY_SHIFT                         5  /*                 */
#define WM5100_DRC_DCY_WIDTH                         4  /*                 */
#define WM5100_DRC_MINGAIN_MASK                 0x001C  /*                     */
#define WM5100_DRC_MINGAIN_SHIFT                     2  /*                     */
#define WM5100_DRC_MINGAIN_WIDTH                     3  /*                     */
#define WM5100_DRC_MAXGAIN_MASK                 0x0003  /*                     */
#define WM5100_DRC_MAXGAIN_SHIFT                     0  /*                     */
#define WM5100_DRC_MAXGAIN_WIDTH                     2  /*                     */

/*
                             
 */
#define WM5100_DRC_NG_MINGAIN_MASK              0xF000  /*                          */
#define WM5100_DRC_NG_MINGAIN_SHIFT                 12  /*                          */
#define WM5100_DRC_NG_MINGAIN_WIDTH                  4  /*                          */
#define WM5100_DRC_NG_EXP_MASK                  0x0C00  /*                      */
#define WM5100_DRC_NG_EXP_SHIFT                     10  /*                      */
#define WM5100_DRC_NG_EXP_WIDTH                      2  /*                      */
#define WM5100_DRC_QR_THR_MASK                  0x0300  /*                    */
#define WM5100_DRC_QR_THR_SHIFT                      8  /*                    */
#define WM5100_DRC_QR_THR_WIDTH                      2  /*                    */
#define WM5100_DRC_QR_DCY_MASK                  0x00C0  /*                    */
#define WM5100_DRC_QR_DCY_SHIFT                      6  /*                    */
#define WM5100_DRC_QR_DCY_WIDTH                      2  /*                    */
#define WM5100_DRC_HI_COMP_MASK                 0x0038  /*                     */
#define WM5100_DRC_HI_COMP_SHIFT                     3  /*                     */
#define WM5100_DRC_HI_COMP_WIDTH                     3  /*                     */
#define WM5100_DRC_LO_COMP_MASK                 0x0007  /*                     */
#define WM5100_DRC_LO_COMP_SHIFT                     0  /*                     */
#define WM5100_DRC_LO_COMP_WIDTH                     3  /*                     */

/*
                             
 */
#define WM5100_DRC_KNEE_IP_MASK                 0x07E0  /*                      */
#define WM5100_DRC_KNEE_IP_SHIFT                     5  /*                      */
#define WM5100_DRC_KNEE_IP_WIDTH                     6  /*                      */
#define WM5100_DRC_KNEE_OP_MASK                 0x001F  /*                     */
#define WM5100_DRC_KNEE_OP_SHIFT                     0  /*                     */
#define WM5100_DRC_KNEE_OP_WIDTH                     5  /*                     */

/*
                             
 */
#define WM5100_DRC_KNEE2_IP_MASK                0x03E0  /*                      */
#define WM5100_DRC_KNEE2_IP_SHIFT                    5  /*                      */
#define WM5100_DRC_KNEE2_IP_WIDTH                    5  /*                      */
#define WM5100_DRC_KNEE2_OP_MASK                0x001F  /*                      */
#define WM5100_DRC_KNEE2_OP_SHIFT                    0  /*                      */
#define WM5100_DRC_KNEE2_OP_WIDTH                    5  /*                      */

/*
                           
 */
#define WM5100_LHPF1_MODE                       0x0002  /*            */
#define WM5100_LHPF1_MODE_MASK                  0x0002  /*            */
#define WM5100_LHPF1_MODE_SHIFT                      1  /*            */
#define WM5100_LHPF1_MODE_WIDTH                      1  /*            */
#define WM5100_LHPF1_ENA                        0x0001  /*           */
#define WM5100_LHPF1_ENA_MASK                   0x0001  /*           */
#define WM5100_LHPF1_ENA_SHIFT                       0  /*           */
#define WM5100_LHPF1_ENA_WIDTH                       1  /*           */

/*
                           
 */
#define WM5100_LHPF1_COEFF_MASK                 0xFFFF  /*                      */
#define WM5100_LHPF1_COEFF_SHIFT                     0  /*                      */
#define WM5100_LHPF1_COEFF_WIDTH                    16  /*                      */

/*
                           
 */
#define WM5100_LHPF2_MODE                       0x0002  /*            */
#define WM5100_LHPF2_MODE_MASK                  0x0002  /*            */
#define WM5100_LHPF2_MODE_SHIFT                      1  /*            */
#define WM5100_LHPF2_MODE_WIDTH                      1  /*            */
#define WM5100_LHPF2_ENA                        0x0001  /*           */
#define WM5100_LHPF2_ENA_MASK                   0x0001  /*           */
#define WM5100_LHPF2_ENA_SHIFT                       0  /*           */
#define WM5100_LHPF2_ENA_WIDTH                       1  /*           */

/*
                           
 */
#define WM5100_LHPF2_COEFF_MASK                 0xFFFF  /*                      */
#define WM5100_LHPF2_COEFF_SHIFT                     0  /*                      */
#define WM5100_LHPF2_COEFF_WIDTH                    16  /*                      */

/*
                           
 */
#define WM5100_LHPF3_MODE                       0x0002  /*            */
#define WM5100_LHPF3_MODE_MASK                  0x0002  /*            */
#define WM5100_LHPF3_MODE_SHIFT                      1  /*            */
#define WM5100_LHPF3_MODE_WIDTH                      1  /*            */
#define WM5100_LHPF3_ENA                        0x0001  /*           */
#define WM5100_LHPF3_ENA_MASK                   0x0001  /*           */
#define WM5100_LHPF3_ENA_SHIFT                       0  /*           */
#define WM5100_LHPF3_ENA_WIDTH                       1  /*           */

/*
                           
 */
#define WM5100_LHPF3_COEFF_MASK                 0xFFFF  /*                      */
#define WM5100_LHPF3_COEFF_SHIFT                     0  /*                      */
#define WM5100_LHPF3_COEFF_WIDTH                    16  /*                      */

/*
                           
 */
#define WM5100_LHPF4_MODE                       0x0002  /*            */
#define WM5100_LHPF4_MODE_MASK                  0x0002  /*            */
#define WM5100_LHPF4_MODE_SHIFT                      1  /*            */
#define WM5100_LHPF4_MODE_WIDTH                      1  /*            */
#define WM5100_LHPF4_ENA                        0x0001  /*           */
#define WM5100_LHPF4_ENA_MASK                   0x0001  /*           */
#define WM5100_LHPF4_ENA_SHIFT                       0  /*           */
#define WM5100_LHPF4_ENA_WIDTH                       1  /*           */

/*
                           
 */
#define WM5100_LHPF4_COEFF_MASK                 0xFFFF  /*                      */
#define WM5100_LHPF4_COEFF_SHIFT                     0  /*                      */
#define WM5100_LHPF4_COEFF_WIDTH                    16  /*                      */

/*
                              
 */
#define WM5100_DSP1_DM_START_1_MASK             0x00FF  /*                       */
#define WM5100_DSP1_DM_START_1_SHIFT                 0  /*                       */
#define WM5100_DSP1_DM_START_1_WIDTH                 8  /*                       */

/*
                              
 */
#define WM5100_DSP1_DM_START_MASK               0xFFFF  /*                        */
#define WM5100_DSP1_DM_START_SHIFT                   0  /*                        */
#define WM5100_DSP1_DM_START_WIDTH                  16  /*                        */

/*
                              
 */
#define WM5100_DSP1_DM_1_1_MASK                 0x00FF  /*                   */
#define WM5100_DSP1_DM_1_1_SHIFT                     0  /*                   */
#define WM5100_DSP1_DM_1_1_WIDTH                     8  /*                   */

/*
                              
 */
#define WM5100_DSP1_DM_1_MASK                   0xFFFF  /*                    */
#define WM5100_DSP1_DM_1_SHIFT                       0  /*                    */
#define WM5100_DSP1_DM_1_WIDTH                      16  /*                    */

/*
                                
 */
#define WM5100_DSP1_DM_254_1_MASK               0x00FF  /*                     */
#define WM5100_DSP1_DM_254_1_SHIFT                   0  /*                     */
#define WM5100_DSP1_DM_254_1_WIDTH                   8  /*                     */

/*
                                
 */
#define WM5100_DSP1_DM_254_MASK                 0xFFFF  /*                      */
#define WM5100_DSP1_DM_254_SHIFT                     0  /*                      */
#define WM5100_DSP1_DM_254_WIDTH                    16  /*                      */

/*
                                
 */
#define WM5100_DSP1_DM_END_1_MASK               0x00FF  /*                     */
#define WM5100_DSP1_DM_END_1_SHIFT                   0  /*                     */
#define WM5100_DSP1_DM_END_1_WIDTH                   8  /*                     */

/*
                                
 */
#define WM5100_DSP1_DM_END_MASK                 0xFFFF  /*                      */
#define WM5100_DSP1_DM_END_SHIFT                     0  /*                      */
#define WM5100_DSP1_DM_END_WIDTH                    16  /*                      */

/*
                              
 */
#define WM5100_DSP1_PM_START_2_MASK             0x00FF  /*                       */
#define WM5100_DSP1_PM_START_2_SHIFT                 0  /*                       */
#define WM5100_DSP1_PM_START_2_WIDTH                 8  /*                       */

/*
                              
 */
#define WM5100_DSP1_PM_START_1_MASK             0xFFFF  /*                        */
#define WM5100_DSP1_PM_START_1_SHIFT                 0  /*                        */
#define WM5100_DSP1_PM_START_1_WIDTH                16  /*                        */

/*
                              
 */
#define WM5100_DSP1_PM_START_MASK               0xFFFF  /*                        */
#define WM5100_DSP1_PM_START_SHIFT                   0  /*                        */
#define WM5100_DSP1_PM_START_WIDTH                  16  /*                        */

/*
                              
 */
#define WM5100_DSP1_PM_1_2_MASK                 0x00FF  /*                   */
#define WM5100_DSP1_PM_1_2_SHIFT                     0  /*                   */
#define WM5100_DSP1_PM_1_2_WIDTH                     8  /*                   */

/*
                              
 */
#define WM5100_DSP1_PM_1_1_MASK                 0xFFFF  /*                    */
#define WM5100_DSP1_PM_1_1_SHIFT                     0  /*                    */
#define WM5100_DSP1_PM_1_1_WIDTH                    16  /*                    */

/*
                              
 */
#define WM5100_DSP1_PM_1_MASK                   0xFFFF  /*                    */
#define WM5100_DSP1_PM_1_SHIFT                       0  /*                    */
#define WM5100_DSP1_PM_1_WIDTH                      16  /*                    */

/*
                                 
 */
#define WM5100_DSP1_PM_510_2_MASK               0x00FF  /*                     */
#define WM5100_DSP1_PM_510_2_SHIFT                   0  /*                     */
#define WM5100_DSP1_PM_510_2_WIDTH                   8  /*                     */

/*
                                 
 */
#define WM5100_DSP1_PM_510_1_MASK               0xFFFF  /*                      */
#define WM5100_DSP1_PM_510_1_SHIFT                   0  /*                      */
#define WM5100_DSP1_PM_510_1_WIDTH                  16  /*                      */

/*
                                 
 */
#define WM5100_DSP1_PM_510_MASK                 0xFFFF  /*                      */
#define WM5100_DSP1_PM_510_SHIFT                     0  /*                      */
#define WM5100_DSP1_PM_510_WIDTH                    16  /*                      */

/*
                                 
 */
#define WM5100_DSP1_PM_END_2_MASK               0x00FF  /*                     */
#define WM5100_DSP1_PM_END_2_SHIFT                   0  /*                     */
#define WM5100_DSP1_PM_END_2_WIDTH                   8  /*                     */

/*
                                 
 */
#define WM5100_DSP1_PM_END_1_MASK               0xFFFF  /*                      */
#define WM5100_DSP1_PM_END_1_SHIFT                   0  /*                      */
#define WM5100_DSP1_PM_END_1_WIDTH                  16  /*                      */

/*
                                 
 */
#define WM5100_DSP1_PM_END_MASK                 0xFFFF  /*                      */
#define WM5100_DSP1_PM_END_SHIFT                     0  /*                      */
#define WM5100_DSP1_PM_END_WIDTH                    16  /*                      */

/*
                              
 */
#define WM5100_DSP1_ZM_START_1_MASK             0x00FF  /*                       */
#define WM5100_DSP1_ZM_START_1_SHIFT                 0  /*                       */
#define WM5100_DSP1_ZM_START_1_WIDTH                 8  /*                       */

/*
                              
 */
#define WM5100_DSP1_ZM_START_MASK               0xFFFF  /*                        */
#define WM5100_DSP1_ZM_START_SHIFT                   0  /*                        */
#define WM5100_DSP1_ZM_START_WIDTH                  16  /*                        */

/*
                              
 */
#define WM5100_DSP1_ZM_1_1_MASK                 0x00FF  /*                   */
#define WM5100_DSP1_ZM_1_1_SHIFT                     0  /*                   */
#define WM5100_DSP1_ZM_1_1_WIDTH                     8  /*                   */

/*
                              
 */
#define WM5100_DSP1_ZM_1_MASK                   0xFFFF  /*                    */
#define WM5100_DSP1_ZM_1_SHIFT                       0  /*                    */
#define WM5100_DSP1_ZM_1_WIDTH                      16  /*                    */

/*
                                 
 */
#define WM5100_DSP1_ZM_1022_1_MASK              0x00FF  /*                      */
#define WM5100_DSP1_ZM_1022_1_SHIFT                  0  /*                      */
#define WM5100_DSP1_ZM_1022_1_WIDTH                  8  /*                      */

/*
                                 
 */
#define WM5100_DSP1_ZM_1022_MASK                0xFFFF  /*                       */
#define WM5100_DSP1_ZM_1022_SHIFT                    0  /*                       */
#define WM5100_DSP1_ZM_1022_WIDTH                   16  /*                       */

/*
                                 
 */
#define WM5100_DSP1_ZM_END_1_MASK               0x00FF  /*                     */
#define WM5100_DSP1_ZM_END_1_SHIFT                   0  /*                     */
#define WM5100_DSP1_ZM_END_1_WIDTH                   8  /*                     */

/*
                                 
 */
#define WM5100_DSP1_ZM_END_MASK                 0xFFFF  /*                      */
#define WM5100_DSP1_ZM_END_SHIFT                     0  /*                      */
#define WM5100_DSP1_ZM_END_WIDTH                    16  /*                      */

/*
                              
 */
#define WM5100_DSP2_DM_START_1_MASK             0x00FF  /*                       */
#define WM5100_DSP2_DM_START_1_SHIFT                 0  /*                       */
#define WM5100_DSP2_DM_START_1_WIDTH                 8  /*                       */

/*
                              
 */
#define WM5100_DSP2_DM_START_MASK               0xFFFF  /*                        */
#define WM5100_DSP2_DM_START_SHIFT                   0  /*                        */
#define WM5100_DSP2_DM_START_WIDTH                  16  /*                        */

/*
                              
 */
#define WM5100_DSP2_DM_1_1_MASK                 0x00FF  /*                   */
#define WM5100_DSP2_DM_1_1_SHIFT                     0  /*                   */
#define WM5100_DSP2_DM_1_1_WIDTH                     8  /*                   */

/*
                              
 */
#define WM5100_DSP2_DM_1_MASK                   0xFFFF  /*                    */
#define WM5100_DSP2_DM_1_SHIFT                       0  /*                    */
#define WM5100_DSP2_DM_1_WIDTH                      16  /*                    */

/*
                                
 */
#define WM5100_DSP2_DM_254_1_MASK               0x00FF  /*                     */
#define WM5100_DSP2_DM_254_1_SHIFT                   0  /*                     */
#define WM5100_DSP2_DM_254_1_WIDTH                   8  /*                     */

/*
                                
 */
#define WM5100_DSP2_DM_254_MASK                 0xFFFF  /*                      */
#define WM5100_DSP2_DM_254_SHIFT                     0  /*                      */
#define WM5100_DSP2_DM_254_WIDTH                    16  /*                      */

/*
                                
 */
#define WM5100_DSP2_DM_END_1_MASK               0x00FF  /*                     */
#define WM5100_DSP2_DM_END_1_SHIFT                   0  /*                     */
#define WM5100_DSP2_DM_END_1_WIDTH                   8  /*                     */

/*
                                
 */
#define WM5100_DSP2_DM_END_MASK                 0xFFFF  /*                      */
#define WM5100_DSP2_DM_END_SHIFT                     0  /*                      */
#define WM5100_DSP2_DM_END_WIDTH                    16  /*                      */

/*
                              
 */
#define WM5100_DSP2_PM_START_2_MASK             0x00FF  /*                       */
#define WM5100_DSP2_PM_START_2_SHIFT                 0  /*                       */
#define WM5100_DSP2_PM_START_2_WIDTH                 8  /*                       */

/*
                              
 */
#define WM5100_DSP2_PM_START_1_MASK             0xFFFF  /*                        */
#define WM5100_DSP2_PM_START_1_SHIFT                 0  /*                        */
#define WM5100_DSP2_PM_START_1_WIDTH                16  /*                        */

/*
                              
 */
#define WM5100_DSP2_PM_START_MASK               0xFFFF  /*                        */
#define WM5100_DSP2_PM_START_SHIFT                   0  /*                        */
#define WM5100_DSP2_PM_START_WIDTH                  16  /*                        */

/*
                              
 */
#define WM5100_DSP2_PM_1_2_MASK                 0x00FF  /*                   */
#define WM5100_DSP2_PM_1_2_SHIFT                     0  /*                   */
#define WM5100_DSP2_PM_1_2_WIDTH                     8  /*                   */

/*
                              
 */
#define WM5100_DSP2_PM_1_1_MASK                 0xFFFF  /*                    */
#define WM5100_DSP2_PM_1_1_SHIFT                     0  /*                    */
#define WM5100_DSP2_PM_1_1_WIDTH                    16  /*                    */

/*
                              
 */
#define WM5100_DSP2_PM_1_MASK                   0xFFFF  /*                    */
#define WM5100_DSP2_PM_1_SHIFT                       0  /*                    */
#define WM5100_DSP2_PM_1_WIDTH                      16  /*                    */

/*
                                 
 */
#define WM5100_DSP2_PM_510_2_MASK               0x00FF  /*                     */
#define WM5100_DSP2_PM_510_2_SHIFT                   0  /*                     */
#define WM5100_DSP2_PM_510_2_WIDTH                   8  /*                     */

/*
                                 
 */
#define WM5100_DSP2_PM_510_1_MASK               0xFFFF  /*                      */
#define WM5100_DSP2_PM_510_1_SHIFT                   0  /*                      */
#define WM5100_DSP2_PM_510_1_WIDTH                  16  /*                      */

/*
                                 
 */
#define WM5100_DSP2_PM_510_MASK                 0xFFFF  /*                      */
#define WM5100_DSP2_PM_510_SHIFT                     0  /*                      */
#define WM5100_DSP2_PM_510_WIDTH                    16  /*                      */

/*
                                 
 */
#define WM5100_DSP2_PM_END_2_MASK               0x00FF  /*                     */
#define WM5100_DSP2_PM_END_2_SHIFT                   0  /*                     */
#define WM5100_DSP2_PM_END_2_WIDTH                   8  /*                     */

/*
                                 
 */
#define WM5100_DSP2_PM_END_1_MASK               0xFFFF  /*                      */
#define WM5100_DSP2_PM_END_1_SHIFT                   0  /*                      */
#define WM5100_DSP2_PM_END_1_WIDTH                  16  /*                      */

/*
                                 
 */
#define WM5100_DSP2_PM_END_MASK                 0xFFFF  /*                      */
#define WM5100_DSP2_PM_END_SHIFT                     0  /*                      */
#define WM5100_DSP2_PM_END_WIDTH                    16  /*                      */

/*
                              
 */
#define WM5100_DSP2_ZM_START_1_MASK             0x00FF  /*                       */
#define WM5100_DSP2_ZM_START_1_SHIFT                 0  /*                       */
#define WM5100_DSP2_ZM_START_1_WIDTH                 8  /*                       */

/*
                              
 */
#define WM5100_DSP2_ZM_START_MASK               0xFFFF  /*                        */
#define WM5100_DSP2_ZM_START_SHIFT                   0  /*                        */
#define WM5100_DSP2_ZM_START_WIDTH                  16  /*                        */

/*
                              
 */
#define WM5100_DSP2_ZM_1_1_MASK                 0x00FF  /*                   */
#define WM5100_DSP2_ZM_1_1_SHIFT                     0  /*                   */
#define WM5100_DSP2_ZM_1_1_WIDTH                     8  /*                   */

/*
                              
 */
#define WM5100_DSP2_ZM_1_MASK                   0xFFFF  /*                    */
#define WM5100_DSP2_ZM_1_SHIFT                       0  /*                    */
#define WM5100_DSP2_ZM_1_WIDTH                      16  /*                    */

/*
                                 
 */
#define WM5100_DSP2_ZM_1022_1_MASK              0x00FF  /*                      */
#define WM5100_DSP2_ZM_1022_1_SHIFT                  0  /*                      */
#define WM5100_DSP2_ZM_1022_1_WIDTH                  8  /*                      */

/*
                                 
 */
#define WM5100_DSP2_ZM_1022_MASK                0xFFFF  /*                       */
#define WM5100_DSP2_ZM_1022_SHIFT                    0  /*                       */
#define WM5100_DSP2_ZM_1022_WIDTH                   16  /*                       */

/*
                                 
 */
#define WM5100_DSP2_ZM_END_1_MASK               0x00FF  /*                     */
#define WM5100_DSP2_ZM_END_1_SHIFT                   0  /*                     */
#define WM5100_DSP2_ZM_END_1_WIDTH                   8  /*                     */

/*
                                 
 */
#define WM5100_DSP2_ZM_END_MASK                 0xFFFF  /*                      */
#define WM5100_DSP2_ZM_END_SHIFT                     0  /*                      */
#define WM5100_DSP2_ZM_END_WIDTH                    16  /*                      */

/*
                              
 */
#define WM5100_DSP3_DM_START_1_MASK             0x00FF  /*                       */
#define WM5100_DSP3_DM_START_1_SHIFT                 0  /*                       */
#define WM5100_DSP3_DM_START_1_WIDTH                 8  /*                       */

/*
                              
 */
#define WM5100_DSP3_DM_START_MASK               0xFFFF  /*                        */
#define WM5100_DSP3_DM_START_SHIFT                   0  /*                        */
#define WM5100_DSP3_DM_START_WIDTH                  16  /*                        */

/*
                              
 */
#define WM5100_DSP3_DM_1_1_MASK                 0x00FF  /*                   */
#define WM5100_DSP3_DM_1_1_SHIFT                     0  /*                   */
#define WM5100_DSP3_DM_1_1_WIDTH                     8  /*                   */

/*
                              
 */
#define WM5100_DSP3_DM_1_MASK                   0xFFFF  /*                    */
#define WM5100_DSP3_DM_1_SHIFT                       0  /*                    */
#define WM5100_DSP3_DM_1_WIDTH                      16  /*                    */

/*
                                
 */
#define WM5100_DSP3_DM_254_1_MASK               0x00FF  /*                     */
#define WM5100_DSP3_DM_254_1_SHIFT                   0  /*                     */
#define WM5100_DSP3_DM_254_1_WIDTH                   8  /*                     */

/*
                                
 */
#define WM5100_DSP3_DM_254_MASK                 0xFFFF  /*                      */
#define WM5100_DSP3_DM_254_SHIFT                     0  /*                      */
#define WM5100_DSP3_DM_254_WIDTH                    16  /*                      */

/*
                                
 */
#define WM5100_DSP3_DM_END_1_MASK               0x00FF  /*                     */
#define WM5100_DSP3_DM_END_1_SHIFT                   0  /*                     */
#define WM5100_DSP3_DM_END_1_WIDTH                   8  /*                     */

/*
                                
 */
#define WM5100_DSP3_DM_END_MASK                 0xFFFF  /*                      */
#define WM5100_DSP3_DM_END_SHIFT                     0  /*                      */
#define WM5100_DSP3_DM_END_WIDTH                    16  /*                      */

/*
                              
 */
#define WM5100_DSP3_PM_START_2_MASK             0x00FF  /*                       */
#define WM5100_DSP3_PM_START_2_SHIFT                 0  /*                       */
#define WM5100_DSP3_PM_START_2_WIDTH                 8  /*                       */

/*
                              
 */
#define WM5100_DSP3_PM_START_1_MASK             0xFFFF  /*                        */
#define WM5100_DSP3_PM_START_1_SHIFT                 0  /*                        */
#define WM5100_DSP3_PM_START_1_WIDTH                16  /*                        */

/*
                              
 */
#define WM5100_DSP3_PM_START_MASK               0xFFFF  /*                        */
#define WM5100_DSP3_PM_START_SHIFT                   0  /*                        */
#define WM5100_DSP3_PM_START_WIDTH                  16  /*                        */

/*
                              
 */
#define WM5100_DSP3_PM_1_2_MASK                 0x00FF  /*                   */
#define WM5100_DSP3_PM_1_2_SHIFT                     0  /*                   */
#define WM5100_DSP3_PM_1_2_WIDTH                     8  /*                   */

/*
                              
 */
#define WM5100_DSP3_PM_1_1_MASK                 0xFFFF  /*                    */
#define WM5100_DSP3_PM_1_1_SHIFT                     0  /*                    */
#define WM5100_DSP3_PM_1_1_WIDTH                    16  /*                    */

/*
                              
 */
#define WM5100_DSP3_PM_1_MASK                   0xFFFF  /*                    */
#define WM5100_DSP3_PM_1_SHIFT                       0  /*                    */
#define WM5100_DSP3_PM_1_WIDTH                      16  /*                    */

/*
                                 
 */
#define WM5100_DSP3_PM_510_2_MASK               0x00FF  /*                     */
#define WM5100_DSP3_PM_510_2_SHIFT                   0  /*                     */
#define WM5100_DSP3_PM_510_2_WIDTH                   8  /*                     */

/*
                                 
 */
#define WM5100_DSP3_PM_510_1_MASK               0xFFFF  /*                      */
#define WM5100_DSP3_PM_510_1_SHIFT                   0  /*                      */
#define WM5100_DSP3_PM_510_1_WIDTH                  16  /*                      */

/*
                                 
 */
#define WM5100_DSP3_PM_510_MASK                 0xFFFF  /*                      */
#define WM5100_DSP3_PM_510_SHIFT                     0  /*                      */
#define WM5100_DSP3_PM_510_WIDTH                    16  /*                      */

/*
                                 
 */
#define WM5100_DSP3_PM_END_2_MASK               0x00FF  /*                     */
#define WM5100_DSP3_PM_END_2_SHIFT                   0  /*                     */
#define WM5100_DSP3_PM_END_2_WIDTH                   8  /*                     */

/*
                                 
 */
#define WM5100_DSP3_PM_END_1_MASK               0xFFFF  /*                      */
#define WM5100_DSP3_PM_END_1_SHIFT                   0  /*                      */
#define WM5100_DSP3_PM_END_1_WIDTH                  16  /*                      */

/*
                                 
 */
#define WM5100_DSP3_PM_END_MASK                 0xFFFF  /*                      */
#define WM5100_DSP3_PM_END_SHIFT                     0  /*                      */
#define WM5100_DSP3_PM_END_WIDTH                    16  /*                      */

/*
                              
 */
#define WM5100_DSP3_ZM_START_1_MASK             0x00FF  /*                       */
#define WM5100_DSP3_ZM_START_1_SHIFT                 0  /*                       */
#define WM5100_DSP3_ZM_START_1_WIDTH                 8  /*                       */

/*
                              
 */
#define WM5100_DSP3_ZM_START_MASK               0xFFFF  /*                        */
#define WM5100_DSP3_ZM_START_SHIFT                   0  /*                        */
#define WM5100_DSP3_ZM_START_WIDTH                  16  /*                        */

/*
                              
 */
#define WM5100_DSP3_ZM_1_1_MASK                 0x00FF  /*                   */
#define WM5100_DSP3_ZM_1_1_SHIFT                     0  /*                   */
#define WM5100_DSP3_ZM_1_1_WIDTH                     8  /*                   */

/*
                              
 */
#define WM5100_DSP3_ZM_1_MASK                   0xFFFF  /*                    */
#define WM5100_DSP3_ZM_1_SHIFT                       0  /*                    */
#define WM5100_DSP3_ZM_1_WIDTH                      16  /*                    */

/*
                                 
 */
#define WM5100_DSP3_ZM_1022_1_MASK              0x00FF  /*                      */
#define WM5100_DSP3_ZM_1022_1_SHIFT                  0  /*                      */
#define WM5100_DSP3_ZM_1022_1_WIDTH                  8  /*                      */

/*
                                 
 */
#define WM5100_DSP3_ZM_1022_MASK                0xFFFF  /*                       */
#define WM5100_DSP3_ZM_1022_SHIFT                    0  /*                       */
#define WM5100_DSP3_ZM_1022_WIDTH                   16  /*                       */

/*
                                 
 */
#define WM5100_DSP3_ZM_END_1_MASK               0x00FF  /*                     */
#define WM5100_DSP3_ZM_END_1_SHIFT                   0  /*                     */
#define WM5100_DSP3_ZM_END_1_WIDTH                   8  /*                     */

/*
                                 
 */
#define WM5100_DSP3_ZM_END_MASK                 0xFFFF  /*                      */
#define WM5100_DSP3_ZM_END_SHIFT                     0  /*                      */
#define WM5100_DSP3_ZM_END_WIDTH                    16  /*                      */

bool wm5100_readable_register(struct device *dev, unsigned int reg);
bool wm5100_volatile_register(struct device *dev, unsigned int reg);

extern struct reg_default wm5100_reg_defaults[WM5100_REGISTER_COUNT];

#endif
