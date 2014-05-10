/*
** =========================================================================
** File:
**     ImmVibeSPI.c
**
** Description: 
**     Device-dependent functions called by Immersion TSP API
**     to control PWM duty cycle, amp enable/disable, save IVT file, etc...
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
//                                                                             
#ifdef CONFIG_MACH_LGE_L9II_COMMON
#include <linux/io.h>
#include <linux/err.h>
#include <linux/gpio.h>
#include <linux/regulator/msm-gpio-regulator.h>

#include <mach/irqs.h>
#include <mach/gpiomux.h>
#include <mach/msm_iomap.h>
#include <mach/msm_xo.h>
#include "../../kernel/arch/arm/mach-msm/lge/fx3/board-fx3.h"
#include <linux/delay.h>
#else
#include "tspdrv_util.h"
extern struct pm8xxx_vib *vib_dev;
#endif
//                                                                             

#ifdef IMMVIBESPIAPI
#undef IMMVIBESPIAPI
#endif
#define IMMVIBESPIAPI static

/*
                                       
*/
#define NUM_ACTUATORS 1

#define PWM_DUTY_MAX    579 /*                             */

static bool g_bAmpEnabled = false;

//                                                                             
#ifdef CONFIG_MACH_LGE_L9II_COMMON
/*                                     */
#define REG_WRITEL(value, reg)				writel(value, (MSM_CLK_CTL_BASE + reg))
#define REG_READL(reg)						readl((MSM_CLK_CTL_BASE + reg))

#define GPn_MD_REG(n)						(0x2D00+32*(n))
#define GPn_NS_REG(n)						(0x2D24+32*(n))

#define GPIO_HAPTIC_EN			6
#define GPIO_HAPTIC_PWR_EN		54

/*                                      
                                     
                                     
                                      
                                      
                                      
                                      
*/
#define GPIO_LIN_MOTOR_PWM		37
#define GP_CLK_ID				2 /*          */

#define GP_CLK_M_DEFAULT		1
#define GP_CLK_N_DEFAULT		166  //                   
#define GP_CLK_D_MAX			GP_CLK_N_DEFAULT
#define GP_CLK_D_HALF			(GP_CLK_N_DEFAULT >> 1)


static struct msm_xo_voter *vib_clock;
static int vibrator_clock_init(void)
{
    int rc;
    /*                 */
    vib_clock = msm_xo_get(MSM_XO_TCXO_D0, "vib_clock");

    if (IS_ERR(vib_clock)) {
        rc = PTR_ERR(vib_clock);
        printk(KERN_ERR "%s: Couldn't get TCXO_D0 vote for Vib(%d)\n",
                __func__, rc);
    }
    return rc;
}

static int vibrator_clock_on(void)
{
    int rc;
    rc = msm_xo_mode_vote(vib_clock, MSM_XO_MODE_ON);
    if (rc < 0) {
        printk(KERN_ERR "%s: Failed to vote for TCX0_D0 ON (%d)\n",
                __func__, rc);
    }
    return rc;
}

static int vibrator_clock_off(void)
{
    int rc;
    rc = msm_xo_mode_vote(vib_clock, MSM_XO_MODE_OFF);
    if (rc < 0) {
        printk(KERN_ERR "%s: Failed to vote for TCX0_D0 OFF (%d)\n",
                __func__, rc);
    }
    return rc;
}

static int vibratror_pwm_gpio_OnOFF(int OnOFF)
{
    int rc = 0;

    if (OnOFF) {
		rc = gpio_request(GPIO_LIN_MOTOR_PWM, "lin_motor_pwm");
		if (unlikely(rc < 0)) {
			printk("%s:GPIO_LIN_MOTOR_PWM(%d) request failed(%d)\n", __func__, GPIO_LIN_MOTOR_PWM, rc);
			return rc;
		}
    } else {
        gpio_free(GPIO_LIN_MOTOR_PWM);
    }
    return 0;
}

static void vibrator_power(int on)
{
	gpio_set_value(GPIO_HAPTIC_PWR_EN, on);
    mdelay(5);
}

static void vibrator_pwm_set(int enable, int amp, int n_value)
{
    uint M_VAL	= GP_CLK_M_DEFAULT;
	uint D_VAL	= GP_CLK_D_MAX;
	uint D_INV	= 0; /*                                    */

	amp = (amp + 127)/2; //               

	if (enable)
	{
        vibrator_clock_on();

		//               
		//                                                         
		D_VAL = ((GP_CLK_D_MAX * amp) >> 7) ;

		if (D_VAL > GP_CLK_D_HALF)
		{
			if (D_VAL == GP_CLK_D_MAX)
			{      /*                 */
				D_VAL = 2;
			}
			else
			{
				D_VAL = GP_CLK_D_MAX - D_VAL;
			}

			D_INV = 1;
		}

		REG_WRITEL(
			(((M_VAL & 0xffU) << 16U) + 	/*              */
			((~(D_VAL << 1)) & 0xffU)),		/*            */
			GPn_MD_REG(GP_CLK_ID));

		REG_WRITEL(
			((((~(n_value-M_VAL)) & 0xffU) << 16U) + /*              */
			(1U << 11U) +  				/*                               */
			((D_INV & 0x01U) << 10U) +	/*                            */
			(1U << 9U) +				/*                               */
			(1U << 8U) +				/*                            */
			(0U << 7U) +				/*                                */
			(2U << 5U) +				/*                                       */
			(3U << 3U) +				/*                               */
			(5U << 0U)), 				/*                          */
			GPn_NS_REG(GP_CLK_ID));
	}
	else
	{
        vibrator_clock_off();
		REG_WRITEL(
			((((~(n_value-M_VAL)) & 0xffU) << 16U) + /*              */
			(0U << 11U) +	/*                                */
			(0U << 10U) +	/*                            */
			(0U << 9U) +	/*                                */
			(0U << 8U) +	/*                             */
			(0U << 7U) +	/*                                */
			(2U << 5U) +	/*                                       */
			(3U << 3U) +	/*                               */
			(5U << 0U)),	/*                          */
			GPn_NS_REG(GP_CLK_ID));

    }
}

static void vibrator_ic_enable_set(int enable)
{
	gpio_set_value(GPIO_HAPTIC_EN, enable);
}
#endif
//                                                                             

/*
                                               
*/
IMMVIBESPIAPI VibeStatus ImmVibeSPI_ForceOut_AmpDisable(VibeUInt8 nActuatorIndex)
{
    if (g_bAmpEnabled)
    {
        //                                                         

//                                                                             
#ifdef CONFIG_MACH_LGE_L9II_COMMON
		vibrator_ic_enable_set(0);
		vibrator_pwm_set(0, 0, GP_CLK_N_DEFAULT);
		vibratror_pwm_gpio_OnOFF(0);
		vibrator_power(0);
#else
		pm8xxx_vib_set(vib_dev, 0, 0);
#endif
//                                                                             
		g_bAmpEnabled = false;
    }

    return VIBE_S_SUCCESS;
}

/*
                                             
*/
IMMVIBESPIAPI VibeStatus ImmVibeSPI_ForceOut_AmpEnable(VibeUInt8 nActuatorIndex)
{
    if (!g_bAmpEnabled)
    {
        //                                                        

//                                                                             
#ifdef CONFIG_MACH_LGE_L9II_COMMON
		vibrator_power(1);
        vibratror_pwm_gpio_OnOFF(1);
		vibrator_pwm_set(1, 0, GP_CLK_N_DEFAULT);
		vibrator_ic_enable_set(1);
#else
		pm8xxx_vib_set(vib_dev, 1, 0);
#endif
//                                                                             
        g_bAmpEnabled = true;
    }

    return VIBE_S_SUCCESS;
}

/*
                                                                     
*/
IMMVIBESPIAPI VibeStatus ImmVibeSPI_ForceOut_Initialize(void)
{
#ifdef CONFIG_MACH_LGE_L9II_COMMON
    int rc;
#endif

    printk("[VIBRATOR] %s\n", __func__);

    g_bAmpEnabled = true;   /*                                                           */

#ifdef CONFIG_MACH_LGE_L9II_COMMON
	rc = gpio_request(GPIO_HAPTIC_PWR_EN, "haptic_pwr_en");
	if (unlikely(rc < 0)) {
		printk("%s:GPIO_HAPTIC_PWR_EN(%d) request failed(%d)\n", __func__, GPIO_HAPTIC_PWR_EN, rc);
		return rc;
	}
	gpio_direction_output(GPIO_HAPTIC_PWR_EN, 0);

	rc = gpio_request(GPIO_HAPTIC_EN, "haptic_en");
	if (unlikely(rc < 0)) {
		printk("%s:GPIO_HAPTIC_EN(%d) request failed(%d)\n", __func__, GPIO_HAPTIC_EN, rc);
		return rc;
	}
	gpio_direction_output(GPIO_HAPTIC_EN, 0);

	rc = gpio_request(GPIO_LIN_MOTOR_PWM, "lin_motor_pwm");
	if (unlikely(rc < 0)) {
		printk("%s:GPIO_LIN_MOTOR_PWM(%d) request failed(%d)\n", __func__, GPIO_LIN_MOTOR_PWM, rc);
		return rc;
	}

    vibrator_clock_init();
#endif	

    /* 
                   
                                                                    
                                                                                      
                       
    */
    ImmVibeSPI_ForceOut_AmpDisable(0);

    return VIBE_S_SUCCESS;
}

/*
                                                                  
*/
IMMVIBESPIAPI VibeStatus ImmVibeSPI_ForceOut_Terminate(void)
{
    //                                                        

    /* 
                   
                                                                    
                                                                                      
                       
    */
    ImmVibeSPI_ForceOut_AmpDisable(0);

//                                                                             
#ifdef CONFIG_MACH_LGE_L9II_COMMON
#if 0
    /*                   */
    PWM_CTRL  = 0;                  /*                         */
    PWM_PERIOD = PWM_DUTY_MAX;      /*                                      */

    /*                       */
    PWM_DUTY = (PWM_DUTY_MAX+1)>>1; /*                                      */
#endif
#endif
//                                                                             

    return VIBE_S_SUCCESS;
}

/*
                                                     
*/
IMMVIBESPIAPI VibeStatus ImmVibeSPI_ForceOut_SetSamples(VibeUInt8 nActuatorIndex, VibeUInt16 nOutputSignalBitDepth, VibeUInt16 nBufferSizeInBytes, VibeInt8* pForceOutputBuffer)
{
    VibeInt8 nForce;

//                      

    switch (nOutputSignalBitDepth)
    {
        case 8:
            /*                                                  */
            if (nBufferSizeInBytes != 1) return VIBE_E_FAIL;

            nForce = pForceOutputBuffer[0];
            break;
        case 16:
            /*                                                  */
            if (nBufferSizeInBytes != 2) return VIBE_E_FAIL;

            /*                           */
            nForce = ((VibeInt16*)pForceOutputBuffer)[0] >> 8;
            break;
        default:
            /*                      */
            return VIBE_E_FAIL;
    }
//                                                                             
#ifdef CONFIG_MACH_LGE_L9II_COMMON
	/*                                                    */
	if (nForce > 127)
		nForce = 127;
	if (nForce < -127)
		nForce = -127;

	vibrator_pwm_set(1, nForce, GP_CLK_N_DEFAULT);
#else
//                                                                             
    if (nForce <= 0)
    {                
        pm8xxx_vib_set(vib_dev, 0, 0);        
    }
    else
    {
        /*        */
        pm8xxx_vib_set(vib_dev, 1, (nForce * 31) / 128 + 1);
    }
//                                                                             
#endif
//                                                                             

    return VIBE_S_SUCCESS;
}

/*
                                                  
*/
#if 0
IMMVIBESPIAPI VibeStatus ImmVibeSPI_ForceOut_SetFrequency(VibeUInt8 nActuatorIndex, VibeUInt16 nFrequencyParameterID, VibeUInt32 nFrequencyParameterValue)
{
    /*                                            */

    return VIBE_S_SUCCESS;
}
#endif

/*
                                                                            
*/
IMMVIBESPIAPI VibeStatus ImmVibeSPI_Device_GetName(VibeUInt8 nActuatorIndex, char *szDevName, int nSize)
{
    if ((!szDevName) || (nSize < 1)) return VIBE_E_FAIL;

    //                                                    

    strncpy(szDevName, "FX3", nSize-1);
    szDevName[nSize - 1] = '\0';    /*                                         */

    return VIBE_S_SUCCESS;
}
