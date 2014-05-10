/*
** =========================================================================
** File:
**     tspdrv.h
**
** Description:
**     Constants and type definitions for the TouchSense Kernel Module.
**
** Portions Copyright (c) 2008-2010 Immersion Corporation. All Rights Reserved.
**
** This file contains Original Code and/or Modifications of Original Code
** as defined in and that are subject to the GNU Public License v2 -
** (the 'License'). You may not use this file except in compliance with the
** License. You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software Foundation, Inc.,
** 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA or contact
** TouchSenseSales@immersion.com.
**
** The Original Code and all software distributed under the License are
** distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
** EXPRESS OR IMPLIED, AND IMMERSION HEREBY DISCLAIMS ALL SUCH WARRANTIES,
** INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY, FITNESS
** FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT. Please see
** the License for the specific language governing rights and limitations
** under the License.
** =========================================================================
*/

#ifndef _TSPDRV_H
#define _TSPDRV_H

/*           */
#define MODULE_NAME                         "tspdrv"
#define TSPDRV                              "/dev/"MODULE_NAME
#define TSPDRV_MAGIC_NUMBER                 0x494D4D52
#define TSPDRV_STOP_KERNEL_TIMER            _IO(TSPDRV_MAGIC_NUMBER & 0xFF, 1)
/*
                         
                                                                              
*/
#define TSPDRV_ENABLE_AMP                   _IO(TSPDRV_MAGIC_NUMBER & 0xFF, 3)
#define TSPDRV_DISABLE_AMP                  _IO(TSPDRV_MAGIC_NUMBER & 0xFF, 4)
#define TSPDRV_GET_NUM_ACTUATORS            _IO(TSPDRV_MAGIC_NUMBER & 0xFF, 5)
#define VIBE_MAX_DEVICE_NAME_LENGTH			64
#define SPI_HEADER_SIZE                     3   /*                                        */
#define VIBE_OUTPUT_SAMPLE_SIZE             50  /*                                           */

/*                  */
#ifdef __KERNEL__
typedef int8_t		VibeInt8;
typedef u_int8_t	VibeUInt8;
typedef int16_t		VibeInt16;
typedef u_int16_t	VibeUInt16;
typedef int32_t		VibeInt32;
typedef u_int32_t	VibeUInt32;
typedef u_int8_t	VibeBool;
typedef VibeInt32	VibeStatus;

typedef struct
{
    VibeUInt8 nActuatorIndex;  /*                            */
    VibeUInt8 nBitDepth;       /*                       */
    VibeUInt8 nBufferSize;     /*                       */
    VibeUInt8 dataBuffer[VIBE_OUTPUT_SAMPLE_SIZE];
} samples_buffer;

typedef struct
{
    VibeInt8 nIndexPlayingBuffer;
    VibeUInt8 nIndexOutputValue;
    samples_buffer actuatorSamples[2]; /*                                                 */
} actuator_samples_buffer;

#endif

/*                              */
#define VIBE_S_SUCCESS                      0	/*         */
#define VIBE_E_FAIL						    -4	/*               */

#if defined(VIBE_RECORD) && defined(VIBE_DEBUG)
    void _RecorderInit(void);
    void _RecorderTerminate(void);
    void _RecorderReset(int nActuator);
    void _Record(int actuatorIndex, const char *format,...);
#endif

/*                     */
#ifdef __KERNEL__
    #if 1 //               
        #define DbgOut(_x_) printk _x_
    #else   /*            */
        #define DbgOut(_x_)
    #endif  /*            */

    #if defined(VIBE_RECORD) && defined(VIBE_DEBUG)
        #define DbgRecorderInit(_x_) _RecorderInit _x_
        #define DbgRecorderTerminate(_x_) _RecorderTerminate _x_
        #define DbgRecorderReset(_x_) _RecorderReset _x_
        #define DbgRecord(_x_) _Record _x_
    #else /*                                             */
        #define DbgRecorderInit(_x_)
        #define DbgRecorderTerminate(_x_)
        #define DbgRecorderReset(_x_)
        #define DbgRecord(_x_)
    #endif /*                                             */
#endif  /*            */

#endif  /*           */
