/*
** =========================================================================
** File:
**     tspdrv.c
**
** Description:
**     TouchSense Kernel Module main entry-point.
**
** Portions Copyright (c) 2008-2011 Immersion Corporation. All Rights Reserved.
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

#ifndef __KERNEL__
#define __KERNEL__
#endif

//               
//               
//       

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/timer.h>
#include <linux/fs.h>
#include <linux/version.h>
#include <linux/miscdevice.h>
#include <linux/platform_device.h>
#include <asm/uaccess.h>
#include <tspdrv.h>
#include <ImmVibeSPI.c>
#if defined(VIBE_DEBUG) && defined(VIBE_RECORD)
#include <tspdrvRecorder.c>
#endif

/*                                     */
#define VERSION_STR " v3.4.55.8\n"                  /*                                        */
#define VERSION_STR_LEN 16                          /*                                                               */
static char g_szDeviceName[  (VIBE_MAX_DEVICE_NAME_LENGTH
                            + VERSION_STR_LEN)
                            * NUM_ACTUATORS];       /*                            */
static size_t g_cchDeviceName;                      /*                            */

/*                                              */
static char g_bIsPlaying = false;

/*                                  */
#define SPI_BUFFER_SIZE (NUM_ACTUATORS * (VIBE_OUTPUT_SAMPLE_SIZE + SPI_HEADER_SIZE))
static int g_bStopRequested = false;
static actuator_samples_buffer g_SamplesBuffer[NUM_ACTUATORS] = {{0}};
static char g_cWriteBuffer[SPI_BUFFER_SIZE];

/*                 */
#ifdef QA_TEST
#define FORCE_LOG_BUFFER_SIZE   128
#define TIME_INCREMENT          5
static int g_nTime = 0;
static int g_nForceLogIndex = 0;
static VibeInt8 g_nForceLog[FORCE_LOG_BUFFER_SIZE];
#endif

#if ((LINUX_VERSION_CODE & 0xFFFF00) < KERNEL_VERSION(2,6,0))
#error Unsupported Kernel version
#endif

#ifndef HAVE_UNLOCKED_IOCTL
#define HAVE_UNLOCKED_IOCTL 0
#endif

#ifdef IMPLEMENT_AS_CHAR_DRIVER
static int g_nMajor = 0;
#endif

/*                                                                      */
#ifdef CONFIG_HIGH_RES_TIMERS
    #include <VibeOSKernelLinuxHRTime.c>
#else
    #include <VibeOSKernelLinuxTime.c>
#endif

/*         */
static int open(struct inode *inode, struct file *file);
static int release(struct inode *inode, struct file *file);
static ssize_t read(struct file *file, char *buf, size_t count, loff_t *ppos);
static ssize_t write(struct file *file, const char *buf, size_t count, loff_t *ppos);
#if HAVE_UNLOCKED_IOCTL
static long unlocked_ioctl(struct file *file, unsigned int cmd, unsigned long arg);
#else
static int ioctl(struct inode *inode, struct file *file, unsigned int cmd, unsigned long arg);
#endif
static struct file_operations fops =
{
    .owner =            THIS_MODULE,
    .read =             read,
    .write =            write,
#if HAVE_UNLOCKED_IOCTL
    .unlocked_ioctl =   unlocked_ioctl,
#else
    .ioctl =            ioctl,
#endif
    .open =             open,
    .release =          release,
    .llseek =           default_llseek    /*                                                        */
};

#ifndef IMPLEMENT_AS_CHAR_DRIVER
static struct miscdevice miscdev =
{
	.minor =    MISC_DYNAMIC_MINOR,
	.name =     MODULE_NAME,
	.fops =     &fops
};
#endif

static int suspend(struct platform_device *pdev, pm_message_t state);
static int resume(struct platform_device *pdev);
static struct platform_driver platdrv =
{
    .suspend =  suspend,
    .resume =   resume,
    .driver =
    {
        .name = MODULE_NAME,
    },
};

static void platform_release(struct device *dev);
static struct platform_device platdev =
{
	.name =     MODULE_NAME,
	.id =       -1,                     /*                                     */
	.dev =
    {
		.platform_data = NULL,
		.release = platform_release,    /*                                                    */
	},
};

/*             */
MODULE_AUTHOR("Immersion Corporation");
MODULE_DESCRIPTION("TouchSense Kernel Module");
MODULE_LICENSE("GPL v2");

/*                  
                                                    
                                  
  */
static int val = 0;

static ssize_t
immersion_enable_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	if (sscanf(buf, "%d", &val) != 1)
		return -EINVAL;

	if(val == 1) {
		printk("immersion_enable_store() : vibrator enable\n");
		vibrator_power_set(1);
		vibratror_pwm_gpio_OnOFF(1);
		vibrator_pwm_set(1, 80, GP_CLK_N_DEFAULT);
		vibrator_ic_enable_set(1);

		DbgRecorderReset((val));
		DbgRecord((val,";------- TSPDRV_ENABLE_AMP ---------\n"));
	}
	else if (val == 0) {
		printk("immersion_enable_store() : vibrator disable\n");
		vibrator_ic_enable_set(0);
		vibrator_pwm_set(0, 0, GP_CLK_N_DEFAULT);
        vibratror_pwm_gpio_OnOFF(0);
		vibrator_power_set(0);
	}

	return count;
}

static ssize_t
immersion_enable_show(struct device *dev, struct device_attribute *attr,   char *buf)
{
	return sprintf(buf, val?"immersion_enable\n":"immersion_disable\n");
}

static struct device_attribute immersion_device_attrs[] = {
	__ATTR(vib_enable,  S_IRUGO | S_IWUSR, immersion_enable_show, immersion_enable_store),
};

/*                                                */

int __init tspdrv_init(void)
{
    int nRet, i;   /*                   */
	int err;

    DbgOut((KERN_INFO "tspdrv: init_module.\n"));

#ifdef IMPLEMENT_AS_CHAR_DRIVER
    g_nMajor = register_chrdev(0, MODULE_NAME, &fops);
    if (g_nMajor < 0)
    {
        DbgOut((KERN_ERR "tspdrv: can't get major number.\n"));
        return g_nMajor;
    }
#else
    nRet = misc_register(&miscdev);
	if (nRet)
    {
        DbgOut((KERN_ERR "tspdrv: misc_register failed.\n"));
		return nRet;
	}
#endif

	nRet = platform_device_register(&platdev);
	if (nRet)
    {
        DbgOut((KERN_ERR "tspdrv: platform_device_register failed.\n"));
    }

	nRet = platform_driver_register(&platdrv);
	if (nRet)
    {
        DbgOut((KERN_ERR "tspdrv: platform_driver_register failed.\n"));
    }

/*                  
                                                    
                                  
  */
	for (i = 0; i < ARRAY_SIZE(immersion_device_attrs); i++) {
			err = device_create_file(miscdev.this_device, &immersion_device_attrs[i]);
			if (err)
				return err;
	}

/*                                                */

    DbgRecorderInit(());

    ImmVibeSPI_ForceOut_Initialize();
    VibeOSKernelLinuxInitTimer();

    /*                                                            */
    g_cchDeviceName = 0;
    for (i=0; i<NUM_ACTUATORS; i++)
    {
        char *szName = g_szDeviceName + g_cchDeviceName;
        ImmVibeSPI_Device_GetName(i, szName, VIBE_MAX_DEVICE_NAME_LENGTH);

        /*                                                  */
        strcat(szName, VERSION_STR);
        g_cchDeviceName += strlen(szName);

        g_SamplesBuffer[i].nIndexPlayingBuffer = -1; /*             */
        g_SamplesBuffer[i].actuatorSamples[0].nBufferSize = 0;
        g_SamplesBuffer[i].actuatorSamples[1].nBufferSize = 0;
    }

    return 0;
}

void __exit tspdrv_exit(void)
{
    DbgOut((KERN_INFO "tspdrv: cleanup_module.\n"));

    DbgRecorderTerminate(());

    VibeOSKernelLinuxTerminateTimer();
    ImmVibeSPI_ForceOut_Terminate();

	platform_driver_unregister(&platdrv);
	platform_device_unregister(&platdev);

#ifdef IMPLEMENT_AS_CHAR_DRIVER
    unregister_chrdev(g_nMajor, MODULE_NAME);
#else
    misc_deregister(&miscdev);
#endif
}

static int open(struct inode *inode, struct file *file)
{
    DbgOut((KERN_INFO "tspdrv: open.\n"));

    if (!try_module_get(THIS_MODULE)) return -ENODEV;

    return 0;
}

static int release(struct inode *inode, struct file *file)
{
    DbgOut((KERN_INFO "tspdrv: release.\n"));

    /*
                                                                         
                                                                       
                                                              
    */
    VibeOSKernelLinuxStopTimer();

    /*
                                                                   
                                                                        
                    
    */
    file->private_data = (void*)NULL;

    module_put(THIS_MODULE);

    return 0;
}

static ssize_t read(struct file *file, char *buf, size_t count, loff_t *ppos)
{
    const size_t nBufSize = (g_cchDeviceName > (size_t)(*ppos)) ? min(count, g_cchDeviceName - (size_t)(*ppos)) : 0;

    /*                     */
    if (0 == nBufSize) return 0;

    if (0 != copy_to_user(buf, g_szDeviceName + (*ppos), nBufSize))
    {
        /*                                   */
        DbgOut((KERN_ERR "tspdrv: copy_to_user failed.\n"));
        return 0;
    }

    /*                                                    */
    *ppos += nBufSize;
    return nBufSize;
}

static ssize_t write(struct file *file, const char *buf, size_t count, loff_t *ppos)
{
    int i = 0;

    *ppos = 0;  /*                                         */

    /*
                                                 
                                                   
    */
    if (file->private_data != (void*)TSPDRV_MAGIC_NUMBER)
    {
        DbgOut((KERN_ERR "tspdrv: unauthorized write.\n"));
        return 0;
    }

    /*                                   */
    if (0 != copy_from_user(g_cWriteBuffer, buf, count))
    {
        /*                                   */
        DbgOut((KERN_ERR "tspdrv: copy_from_user failed.\n"));
        return 0;
    }

    /*                   */
    if ((count <= SPI_HEADER_SIZE) || (count > SPI_BUFFER_SIZE))
    {
        DbgOut((KERN_ERR "tspdrv: invalid write buffer size.\n"));
        return 0;
    }

    while (i < count)
    {
        int nIndexFreeBuffer;   /*                   */

        samples_buffer* pInputBuffer = (samples_buffer*)(&g_cWriteBuffer[i]);

        if ((i + SPI_HEADER_SIZE) >= count)
        {
            /*
                                                           
                                     
            */
            DbgOut((KERN_EMERG "tspdrv: invalid buffer index.\n"));
        }

        /*                 */
        if (8 != pInputBuffer->nBitDepth)
        {
            DbgOut((KERN_WARNING "tspdrv: invalid bit depth. Use default value (8).\n"));
        }

        /*                                                      */
#if (SPI_HEADER_SIZE != 3)
#error "SPI_HEADER_SIZE expected to be 3"
#endif

        /*                   */
        if ((i + SPI_HEADER_SIZE + pInputBuffer->nBufferSize) > count)
        {
            /*
                                                           
                                     
            */
            DbgOut((KERN_EMERG "tspdrv: invalid data size.\n"));
        }

        /*                      */
        if (NUM_ACTUATORS <= pInputBuffer->nActuatorIndex)
        {
            DbgOut((KERN_ERR "tspdrv: invalid actuator index.\n"));
            i += (SPI_HEADER_SIZE + pInputBuffer->nBufferSize);
            continue;
        }

        if (0 == g_SamplesBuffer[pInputBuffer->nActuatorIndex].actuatorSamples[0].nBufferSize)
        {
            nIndexFreeBuffer = 0;
        }
        else if (0 == g_SamplesBuffer[pInputBuffer->nActuatorIndex].actuatorSamples[1].nBufferSize)
        {
             nIndexFreeBuffer = 1;
        }
        else
        {
            /*                               */
            DbgOut((KERN_ERR "tspdrv: no room to store new samples.\n"));
            return 0;
        }

        /*                                                         */
        memcpy(&(g_SamplesBuffer[pInputBuffer->nActuatorIndex].actuatorSamples[nIndexFreeBuffer]), &g_cWriteBuffer[i], (SPI_HEADER_SIZE + pInputBuffer->nBufferSize));

        /*                                                                                                                              */
        if ( -1 == g_SamplesBuffer[pInputBuffer->nActuatorIndex].nIndexPlayingBuffer)
        {
           g_SamplesBuffer[pInputBuffer->nActuatorIndex].nIndexPlayingBuffer = nIndexFreeBuffer;
           g_SamplesBuffer[pInputBuffer->nActuatorIndex].nIndexOutputValue = 0;
        }

        /*                        */
        i += (SPI_HEADER_SIZE + pInputBuffer->nBufferSize);
    }

#ifdef QA_TEST
    g_nForceLog[g_nForceLogIndex++] = g_cSPIBuffer[0];
    if (g_nForceLogIndex >= FORCE_LOG_BUFFER_SIZE)
    {
        for (i=0; i<FORCE_LOG_BUFFER_SIZE; i++)
        {
            printk("<6>%d\t%d\n", g_nTime, g_nForceLog[i]);
            g_nTime += TIME_INCREMENT;
        }
        g_nForceLogIndex = 0;
    }
#endif

    /*                                                  */
    g_bIsPlaying = true;
    VibeOSKernelLinuxStartTimer();

    return count;
}

#if HAVE_UNLOCKED_IOCTL
static long unlocked_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
#else
static int ioctl(struct inode *inode, struct file *file, unsigned int cmd, unsigned long arg)
#endif
{
#ifdef QA_TEST
    int i;
#endif

    switch (cmd)
    {
        case TSPDRV_STOP_KERNEL_TIMER:
            /*
                                                                                             
                                                                     
            */
            if (true == g_bIsPlaying) g_bStopRequested = true;

#ifdef VIBEOSKERNELPROCESSDATA
            /*                                                    */
            VibeOSKernelProcessData(NULL);
#endif

#ifdef QA_TEST
            if (g_nForceLogIndex)
            {
                for (i=0; i<g_nForceLogIndex; i++)
                {
                    printk("<6>%d\t%d\n", g_nTime, g_nForceLog[i]);
                    g_nTime += TIME_INCREMENT;
                }
            }
            g_nTime = 0;
            g_nForceLogIndex = 0;
#endif
            break;

        case TSPDRV_MAGIC_NUMBER:
            file->private_data = (void*)TSPDRV_MAGIC_NUMBER;
            break;

        case TSPDRV_ENABLE_AMP:
            ImmVibeSPI_ForceOut_AmpEnable(arg);
            DbgRecorderReset((arg));
            DbgRecord((arg,";------- TSPDRV_ENABLE_AMP ---------\n"));
            break;

        case TSPDRV_DISABLE_AMP:
            /*                                                                                                            */
            /*                                                                                                           */
            if(!g_bStopRequested)
            {
                ImmVibeSPI_ForceOut_AmpDisable(arg);
            }
            break;

        case TSPDRV_GET_NUM_ACTUATORS:
            return NUM_ACTUATORS;
    }

    return 0;
}

static int suspend(struct platform_device *pdev, pm_message_t state)
{
    if (g_bIsPlaying)
    {
        DbgOut((KERN_INFO "tspdrv: can't suspend, still playing effects.\n"));
        return -EBUSY;
    }
    else
    {
        DbgOut((KERN_INFO "tspdrv: suspend.\n"));
        return 0;
    }
}

static int resume(struct platform_device *pdev)
{
    DbgOut((KERN_INFO "tspdrv: resume.\n"));

	return 0;   /*            */
}

static void platform_release(struct device *dev)
{
    DbgOut((KERN_INFO "tspdrv: platform_release.\n"));
}

module_init(tspdrv_init);
module_exit(tspdrv_exit);
