/*
** =========================================================================
** File:
**     VibeOSKernelLinuxHRTime.c
**
** Description:
**     High Resolution Time helper functions for Linux.
**
** Portions Copyright (c) 2010-2011 Immersion Corporation. All Rights Reserved.
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

/*
                                                                               
                                                                                  
                         
*/

#ifndef CONFIG_HIGH_RES_TIMERS
#warning "The Kernel does not have high resolution timers enabled. Either provide a non hr-timer implementation of VibeOSKernelLinuxTime.c or re-compile your kernel with CONFIG_HIGH_RES_TIMERS=y"
#endif

#include <linux/hrtimer.h>
#include <linux/mutex.h>

#define WATCHDOG_TIMEOUT    10  /*                        */

/*                                      */
#ifndef DEFINE_SEMAPHORE
#define DEFINE_SEMAPHORE(name) \
    struct semaphore name = __SEMAPHORE_INITIALIZER(name, 1)
#endif

/*                  */
static bool g_bTimerStarted = false;
static struct hrtimer g_tspTimer;
static ktime_t g_ktFiveMs;
static int g_nWatchdogCounter = 0;

DEFINE_SEMAPHORE(g_hMutex);

/*                      */
static void VibeOSKernelLinuxStartTimer(void);
static void VibeOSKernelLinuxStopTimer(void);
static int VibeOSKernelProcessData(void* data);
#define VIBEOSKERNELPROCESSDATA

static inline int VibeSemIsLocked(struct semaphore *lock)
{
#if ((LINUX_VERSION_CODE & 0xFFFFFF) < KERNEL_VERSION(2,6,27))
    return atomic_read(&lock->count) != 1;
#else
    return (lock->count) != 1;
#endif
}

static enum hrtimer_restart tsp_timer_interrupt(struct hrtimer *timer)
{
    /*                                          */
    hrtimer_forward_now(timer, g_ktFiveMs);

    if(g_bTimerStarted)
    {
        if (VibeSemIsLocked(&g_hMutex)) up(&g_hMutex);
    }

    return HRTIMER_RESTART;
}

static int VibeOSKernelProcessData(void* data)
{
    int i;
    int nActuatorNotPlaying = 0;

    for (i = 0; i < NUM_ACTUATORS; i++)
    {
        actuator_samples_buffer *pCurrentActuatorSample = &(g_SamplesBuffer[i]);

        if (-1 == pCurrentActuatorSample->nIndexPlayingBuffer)
        {
            nActuatorNotPlaying++;
            if ((NUM_ACTUATORS == nActuatorNotPlaying) && ((++g_nWatchdogCounter) > WATCHDOG_TIMEOUT))
            {
                VibeInt8 cZero[1] = {0};

                /*                                                                                                   */
                ImmVibeSPI_ForceOut_SetSamples(i, 8, 1, cZero);
                ImmVibeSPI_ForceOut_AmpDisable(i);
                VibeOSKernelLinuxStopTimer();

                /*                        */
                g_nWatchdogCounter = 0;
            }
        }
        else
        {
            /*                         */
            if (VIBE_E_FAIL == ImmVibeSPI_ForceOut_SetSamples(
                pCurrentActuatorSample->actuatorSamples[(int)pCurrentActuatorSample->nIndexPlayingBuffer].nActuatorIndex,
                pCurrentActuatorSample->actuatorSamples[(int)pCurrentActuatorSample->nIndexPlayingBuffer].nBitDepth,
                pCurrentActuatorSample->actuatorSamples[(int)pCurrentActuatorSample->nIndexPlayingBuffer].nBufferSize,
                pCurrentActuatorSample->actuatorSamples[(int)pCurrentActuatorSample->nIndexPlayingBuffer].dataBuffer))
            {
                /*                                                                                 */
                hrtimer_forward_now(&g_tspTimer, g_ktFiveMs);
            }

            pCurrentActuatorSample->nIndexOutputValue += pCurrentActuatorSample->actuatorSamples[(int)pCurrentActuatorSample->nIndexPlayingBuffer].nBufferSize;

            if (pCurrentActuatorSample->nIndexOutputValue >= pCurrentActuatorSample->actuatorSamples[(int)pCurrentActuatorSample->nIndexPlayingBuffer].nBufferSize)
            {
                /*                                     */
                pCurrentActuatorSample->actuatorSamples[(int)pCurrentActuatorSample->nIndexPlayingBuffer].nBufferSize = 0;

                /*               */
                (pCurrentActuatorSample->nIndexPlayingBuffer) ^= 1;
                pCurrentActuatorSample->nIndexOutputValue = 0;

                /*                                                */
                if (g_bStopRequested)
                {
                    pCurrentActuatorSample->nIndexPlayingBuffer = -1;

                    ImmVibeSPI_ForceOut_AmpDisable(i);
                }
            }
        }
    }

    /*                                 */
    if (g_bStopRequested)
    {
        VibeOSKernelLinuxStopTimer();

        /*                        */
        g_nWatchdogCounter = 0;

        if (VibeSemIsLocked(&g_hMutex)) up(&g_hMutex);
        return 1;   /*                                            */
    }

    return 0;
}

static void VibeOSKernelLinuxInitTimer(void)
{
    /*                                    */
    g_ktFiveMs = ktime_set(0, 5000000);

    hrtimer_init(&g_tspTimer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);

    /*                                                                                     */
    g_tspTimer.function = tsp_timer_interrupt;
}

static void VibeOSKernelLinuxStartTimer(void)
{
    int i;
    int res;

    /*                        */
    g_nWatchdogCounter = 0;

    if (!g_bTimerStarted)
    {
        if (!VibeSemIsLocked(&g_hMutex)) res = down_interruptible(&g_hMutex); /*              */

        g_bTimerStarted = true;

        /*                 */
        hrtimer_start(&g_tspTimer, g_ktFiveMs, HRTIMER_MODE_REL);

        /*                                                                                                                  */
        for (i = 0; i < NUM_ACTUATORS; i++)
        {
            if ((g_SamplesBuffer[i].actuatorSamples[0].nBufferSize) || (g_SamplesBuffer[i].actuatorSamples[1].nBufferSize))
            {
                g_SamplesBuffer[i].nIndexOutputValue = 0;
                return;
            }
        }
    }

    if (0 != VibeOSKernelProcessData(NULL)) return;

    /*
                                                   
                                                                             
    */
    res = down_interruptible(&g_hMutex);  /*                                             */
    if (res != 0)
    {
        DbgOut((KERN_INFO "VibeOSKernelLinuxStartTimer: down_interruptible interrupted by a signal.\n"));
    }
}

static void VibeOSKernelLinuxStopTimer(void)
{
    int i;

    if (g_bTimerStarted)
    {
        g_bTimerStarted = false;
        hrtimer_cancel(&g_tspTimer);
    }

    /*                       */
    for (i = 0; i < NUM_ACTUATORS; i++)
    {
        g_SamplesBuffer[i].nIndexPlayingBuffer = -1;
        g_SamplesBuffer[i].actuatorSamples[0].nBufferSize = 0;
        g_SamplesBuffer[i].actuatorSamples[1].nBufferSize = 0;
    }
    g_bStopRequested = false;
    g_bIsPlaying = false;
}

static void VibeOSKernelLinuxTerminateTimer(void)
{
    VibeOSKernelLinuxStopTimer();
    if (VibeSemIsLocked(&g_hMutex)) up(&g_hMutex);
}
