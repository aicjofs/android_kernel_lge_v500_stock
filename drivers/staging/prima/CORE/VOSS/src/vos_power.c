/*
 * Copyright (c) 2012, The Linux Foundation. All rights reserved.
 *
 * Previously licensed under the ISC license by Qualcomm Atheros, Inc.
 *
 *
 * Permission to use, copy, modify, and/or distribute this software for
 * any purpose with or without fee is hereby granted, provided that the
 * above copyright notice and this permission notice appear in all
 * copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL
 * WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE
 * AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL
 * DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR
 * PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */

/* 
       
             

        
                                                                      
                                                                      
                                                                     
                                                                      
                                           

*/

/*===========================================================================

  Copyright (c) 2008 QUALCOMM Incorporated. All Rights Reserved

  Qualcomm Proprietary

  Export of this technology or software is regulated by the U.S. Government.
  Diversion contrary to U.S. law prohibited.

  All ideas, data and information contained in or disclosed by
  this document are confidential and proprietary information of
  QUALCOMM Incorporated and all rights therein are expressly reserved.
  By accepting this material the recipient agrees that this material
  and the information contained therein are held in confidence and in
  trust and will not be used, copied, reproduced in whole or in part,
  nor its contents revealed in any manner to others without the express
  written permission of QUALCOMM Incorporated.

===========================================================================*/

/*                                                                           

                                               

                                                                     
                                                              

          

                                   
                                                                             

                                                                           */

/*                                                                           

                                        

                                                                           */
#include <vos_power.h>

#ifdef ANI_BUS_TYPE_SDIO
#include <libra_sdioif.h>
#endif

#ifdef MSM_PLATFORM
#include <mach/mpp.h>
#include <mach/vreg.h>
#include <linux/err.h>
#include <linux/delay.h>

#ifdef MSM_PLATFORM_7x30
#include <mach/irqs-7x30.h>
#include <linux/mfd/pmic8058.h>
#include <mach/rpc_pmapp.h>
#include <mach/pmic.h>
#endif

#ifdef MSM_PLATFORM_8660
#include <qcomwlan_pwrif.h>
#endif

#ifdef MSM_PLATFORM_7x27A
#include <linux/qcomwlan7x27a_pwrif.h>
#endif

#endif //            

#include <vos_sched.h>

//                                                                       
#define VOS_PWR_WIFI_ON_OFF_HACK
#ifdef VOS_PWR_WIFI_ON_OFF_HACK
#define VOS_PWR_SLEEP(ms) msleep(ms)
#else
#define VOS_PWR_SLEEP(ms)
#endif

/*                                                                           

                                             

                                                                           */

#define CHIP_POWER_ON         1
#define CHIP_POWER_OFF        0

//                                  
#define WLAN_LOW_SD_CONFIG_CLOCK_FREQ 400000

#ifdef MSM_PLATFORM_7x30

#define PM8058_GPIO_PM_TO_SYS(pm_gpio)  (pm_gpio + NR_GPIO_IRQS)

static const char* id = "WLAN";

struct wlan_pm8058_gpio {
    int gpio_num;
    struct pm_gpio gpio_cfg;
};


//                                            
static struct wlan_pm8058_gpio wlan_gpios_reset[] = {
    {PM8058_GPIO_PM_TO_SYS(22),{PM_GPIO_DIR_OUT, PM_GPIO_OUT_BUF_CMOS, 0, PM_GPIO_PULL_NO, 2, PM_GPIO_STRENGTH_LOW, PM_GPIO_FUNC_NORMAL, 0}},
};

//   
static struct wlan_pm8058_gpio wlan_gpios_reset_out[] = {
    {PM8058_GPIO_PM_TO_SYS(22),{PM_GPIO_DIR_OUT, PM_GPIO_OUT_BUF_CMOS, 1, PM_GPIO_PULL_NO, 2, PM_GPIO_STRENGTH_HIGH, PM_GPIO_FUNC_NORMAL, 0}},
};


/*                                     */
int vos_chip_power_qrf8600(int on)
{
    struct vreg *vreg_wlan = NULL;
    struct vreg *vreg_gp16 = NULL;
    struct vreg *vreg_gp15 = NULL;
    struct vreg *vreg_s2 = NULL;
    int rc = 0;

VOS_PWR_SLEEP(100);
    //                  
    vreg_wlan = vreg_get(NULL, "wlan");
    if (IS_ERR(vreg_wlan)) {
        VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR, "%s: wlan vreg get failed (%ld)\n",
                             __func__, PTR_ERR(vreg_wlan));
        return PTR_ERR(vreg_wlan);
    }

    //                   
    vreg_gp16 = vreg_get(NULL, "gp16");
    if (IS_ERR(vreg_gp16))  {
        VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR, "%s: gp16 vreg get failed (%ld)\n",
                             __func__, PTR_ERR(vreg_gp16));
        return PTR_ERR(vreg_gp16);
    }

    //                  
    vreg_gp15 = vreg_get(NULL, "gp15");
    if (IS_ERR(vreg_gp15))  {
        VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR, "%s: gp15 vreg get failed (%ld)\n",
                             __func__, PTR_ERR(vreg_gp15));
        return PTR_ERR(vreg_gp15);
    }

    //               
    vreg_s2 = vreg_get(NULL, "s2");
    if (IS_ERR(vreg_s2)) {
        VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR, "%s: s2 vreg get failed (%ld)\n",
                             __func__, PTR_ERR(vreg_s2));
        return PTR_ERR(vreg_s2);
    }

    if (on) {
        /*                                                                           */
        rc = pm8xxx_gpio_config(wlan_gpios_reset[0].gpio_num, &wlan_gpios_reset[0].gpio_cfg);
        if (rc) {
            VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR, "%s: pmic gpio %d config failed (%d)\n",
                            __func__, wlan_gpios_reset[0].gpio_num, rc);
            return -EIO;
        }
        VOS_PWR_SLEEP(300);
        VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_INFO, "WLAN put in reset mode \n");
#if 0
        rc = pmapp_clock_vote("wlan", PMAPP_CLOCK_ID_A0, PMAPP_CLOCK_VOTE_ON);
        if (rc) {
            VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR, "%s: Voting TCXO to ON failed. (%d)\n",__func__, rc);
            return -EIO;
        }
#endif
        /*                                                */
        rc = pmapp_clock_vote(id, PMAPP_CLOCK_ID_A0, PMAPP_CLOCK_VOTE_PIN_CTRL);
        if (rc) {
            VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR, "%s: Configuring TCXO to Pin controllable failed. (%d)\n",__func__, rc);
            return -EIO;
        }

        VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_INFO, "TCXO configured to be slave to WLAN_CLK_PWR-REQ \n");
        printk(KERN_ERR "TCXO is now slave to clk_pwr_req \n");

        //           
        msleep(10);

        /*                                          */
        rc = vreg_set_level(vreg_wlan, 3050);
        if (rc) {
            VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR, "%s: wlan vreg set level failed (%d)\n", __func__, rc);
            return -EIO;
        }

        rc = vreg_enable(vreg_wlan);
        if (rc) {
            VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR, "%s: wlan vreg enable failed. (%d)\n",__func__, rc);
            return -EIO;
        }

        VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_INFO, "2.9V Power Supply Enabled \n");
        printk(KERN_ERR "3.05V Supply Enabled \n");

        /*                                             */
        rc = vreg_set_level(vreg_gp16, 1200);
        if (rc) {
            VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR, "%s: gp16 vreg set level failed (%d)\n", __func__, rc);
            return -EIO;
        }

        rc = vreg_enable(vreg_gp16);
        if (rc) {
            VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR, "%s: gp16 vreg enable failed. (%d)\n",__func__, rc);
            return -EIO;
        }
        VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_INFO, "1.2V AON Power Supply Enabled \n");
        printk(KERN_ERR "1.2V AON Supply Enabled \n");

        //            
        msleep(1);

        VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_INFO, "1.2V AON Power Supply Enabled \n");

        rc = pm8xxx_gpio_config(wlan_gpios_reset_out[0].gpio_num, &wlan_gpios_reset_out[0].gpio_cfg);
        if (rc) {
            VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR, "%s: pmic gpio %d config failed (%d)\n",
                            __func__, wlan_gpios_reset_out[0].gpio_num, rc);
            return -EIO;
        }

        VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_INFO, "WLAN is out of reset now \n");
        printk(KERN_ERR "WLAN is out of reset \n");

        /*              */
        msleep(1);

        /*                                                                                                                        */

        /*                                     */
        rc = vreg_set_level(vreg_gp15, 1200);
        if (rc) {
            VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR, "%s: gp15 vreg set level failed (%d)\n", __func__, rc);
            return -EIO;
        }

#ifdef WLAN_FEATURE_VOS_POWER_VOTED_SUPPLY
        rc = vreg_enable(vreg_gp15);
        if (rc) {
            VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR, "%s: gp15 vreg enable failed. (%d)\n",__func__, rc);
            return -EIO;
        }
#else        
        /*                                                            */
        rc = pmapp_vreg_pincntrl_vote(id, PMAPP_VREG_LDO22, PMAPP_CLOCK_ID_A0, VOS_TRUE);
        if (rc) {
            VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR, "%s: gp15 vreg enable failed. (%d)\n",__func__, rc);
            return -EIO;
        }
        vos_sleep(5);
#endif

        VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_INFO, "1.2V sw Power Supply is enabled \n");
        printk(KERN_ERR "1.2V sw is enabled \n");

        /*                                     */
        rc = pmapp_vreg_level_vote(id, PMAPP_VREG_S2, 1300);
        if (rc) {
            VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR, "%s: s2 vreg set level failed (%d)\n", __func__, rc);
            return -EIO;
        }
        VOS_PWR_SLEEP(300);

#ifdef WLAN_FEATURE_VOS_POWER_VOTED_SUPPLY
        rc = vreg_enable(vreg_s2);
        if (rc) {
            VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR, "%s: s2 vreg enable failed. .(%d)\n",__func__, rc);
            return -EIO;
        }
        msleep(1);
#else        
        /*                                                            */
        rc = pmapp_vreg_pincntrl_vote(id, PMAPP_VREG_S2, PMAPP_CLOCK_ID_A0, VOS_TRUE);
        if (rc) {
            VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR, "%s: s2 vreg enable failed. (%d)\n",__func__, rc);
            return -EIO;
        }
        vos_sleep(5);
#endif

        VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_INFO, "1.3V sw Power Supply is enabled \n");
        printk(KERN_ERR "1.3V sw is enabled \n");

    } else {

        /* 
                                                                                        
                                                                                      
                                                                                              
                                                                                                 
                                                                                                  
                                                                                                           
                                                                                              
                                                                                                   
                                                                   

                            
                         
                                                                 
                                           
                       
         */
        /*                                                                                
                                
        */
        rc = pm8xxx_gpio_config(wlan_gpios_reset[0].gpio_num, &wlan_gpios_reset[0].gpio_cfg);
        if (rc) {
            VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR, "%s: pmic gpio %d config failed (%d)\n",
                            __func__, wlan_gpios_reset[0].gpio_num, rc);
            return -EIO;
        }

#ifdef WLAN_FEATURE_VOS_POWER_VOTED_SUPPLY
        /*                                                                                                                        */
        printk(KERN_ERR "power down switchable\n");
        rc = pmapp_vreg_level_vote(id, PMAPP_VREG_S2, 0);
        if (rc) {
            VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR, "%s: s2 vreg set level failed (%d)\n", __func__, rc);
            return -EIO;
        }
        
        /*         */    
        rc = vreg_disable(vreg_s2); 
        if (rc) {
            VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR, "%s: s2 vreg disable failed (%d)\n", __func__, rc);
            return -EIO;
        }

       VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_INFO, "1.3V sw is disabled \n");

        /*         */
        rc = vreg_disable(vreg_gp15); 
        if (rc) {
            VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR, "%s: gp15 vreg disable failed (%d)\n", __func__, rc);
            return -EIO;
        }

        VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_INFO, "1.2V sw is disabled \n");
#endif //                                          

        /*          */
        rc = vreg_disable(vreg_gp16); 
        if (rc) {
            VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR, "%s: gp16 vreg disable failed (%d)\n", __func__, rc);
            return -EIO;
        }

        VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_INFO, "1.2V AON is disabled \n");

#ifdef VOLANS_2_0
        /*                                                     */

        /*      */
        rc = vreg_disable(vreg_wlan); 
        if (rc) {
            VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR, "%s: wlan vreg disable failed (%d)\n", __func__, rc);
            return -EIO;
        }

        VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_INFO, "2.9V is disabled \n");
#endif
    }

    return rc;
}
#endif

#ifdef MSM_PLATFORM_7x27_FFA

#define MPP_4_CHIP_PWD_L 3 //                                    

//                                                       
int vos_chip_power_7x27_keypad( int on )
{
   struct vreg *vreg_wlan, *vreg_bt = NULL;
   int rc = 0;

   vreg_wlan = vreg_get(NULL, "wlan");
   if (IS_ERR(vreg_wlan)) {
      VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR, "%s: wlan vreg get failed (%ld)",
         __func__, PTR_ERR(vreg_wlan));
      return PTR_ERR(vreg_wlan);
   }

   vreg_bt = vreg_get(NULL, "gp6");
   if (IS_ERR(vreg_bt)) {
      VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR, "%s: gp6 vreg get failed (%ld)",
                __func__, PTR_ERR(vreg_bt));
      return PTR_ERR(vreg_bt);
   }

   if(on) {

      /*                             */
      rc = vreg_set_level(vreg_bt, 2600);
      if (rc) {
         VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR, "%s: vreg set level failed (%d)",__func__, rc);
         return -EIO;
      }
      rc = vreg_enable(vreg_bt);
      if (rc) {
         VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR, "%s: vreg enable failed (%d)",__func__, rc);
         return -EIO;
      }

      //                 
      rc = vreg_set_level(vreg_wlan, 1800);
      if (rc) {
         VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR, "%s: vreg set level failed (%d)", __func__, rc);
         return -EIO;
      }

      rc = vreg_enable(vreg_wlan);
      if (rc) {
         VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR, "%s: wlan vreg enable failed (%d)",__func__, rc);
         return -EIO;
      }

      msleep(100);

      //                                                   
      rc = mpp_config_digital_out(MPP_4_CHIP_PWD_L, 
         MPP_CFG(MPP_DLOGIC_LVL_MSMP, MPP_DLOGIC_OUT_CTRL_HIGH));
      if (rc) {
         VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR, "%s: MPP_4 pull high failed (%d)",__func__, rc);
         return -EIO;
      }

      VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR, "%s: Enabled power supply for WLAN", __func__);
 
      msleep(500);
   }
   else 
   {

       //                                          
       rc = mpp_config_digital_out(MPP_4_CHIP_PWD_L, 
          MPP_CFG(MPP_DLOGIC_LVL_MSMP, MPP_DLOGIC_OUT_CTRL_LOW));
       if (rc) {
          VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR, "%s: MPP_4 pull low failed (%d)",__func__, rc);
          return -EIO;
       }

       msleep(100);

      /*               */
      rc = vreg_disable(vreg_bt);
      if (rc) {
         VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR, "%s: vreg disable failed (%d)",__func__, rc);
         return -EIO;
      }

      /*               */
      rc = vreg_disable(vreg_wlan);
      if (rc) {
         VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR, "%s: wlan vreg disable failed (%d)",__func__, rc);
         return -EIO;
      }

      msleep(100);

      VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR, "%s: Disabled power supply for WLAN", __func__);
   }

   return 0;
}
#endif

/*                                                                           

                                       

                                                                           */

/* 
                                                                  

                                                                       
                                                                           
                                                                         
                                                            

                                                                           
              

                                                                          
                                                                                
                                                                       

          
                                      
                                                                            
                                                                           
                                              
                                                                              
                                                                                        
                                                                                   
                                                                                       
                                                                                         

*/
VOS_STATUS vos_chipPowerUp
(
  vos_call_status_type* status,
  vos_power_cb_type     callback,
  v_PVOID_t             user_data
)
{

#ifdef MSM_PLATFORM_8660
   if(vos_chip_power_qrf8615(CHIP_POWER_ON))
      return VOS_STATUS_E_FAILURE;
#endif

#ifdef MSM_PLATFORM_7x30
   if(vos_chip_power_qrf8600(CHIP_POWER_ON))
      return VOS_STATUS_E_FAILURE;
#endif

#ifdef MSM_PLATFORM_7x27_FFA
   if(vos_chip_power_7x27_keypad(CHIP_POWER_ON))
      return VOS_STATUS_E_FAILURE;
#endif

#ifdef MSM_PLATFORM_7x27A
   if(chip_power_qrf6285(CHIP_POWER_ON))
      return VOS_STATUS_E_FAILURE;
#endif

   return VOS_STATUS_SUCCESS;
}

/* 
                                                                      

                                                                       
                                                                           
                                                                         
                                                            

                                                                          
                                                                                
                                                                       

          
                                      
                                                                            
                                                                           
                                              
                                                                              
                                                                                        
                                                                                   
                                                                                       
                                                                                         

*/
VOS_STATUS vos_chipPowerDown
(
  vos_call_status_type* status,
  vos_power_cb_type     callback,
  v_PVOID_t             user_data
)
{

#ifdef ANI_BUS_TYPE_SDIO
#ifdef MSM_PLATFORM
   struct sdio_func *sdio_func_dev = NULL;

   //                         
   sdio_func_dev = libra_getsdio_funcdev();
   if (sdio_func_dev == NULL)
   {
      VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_FATAL, "%s: Libra WLAN sdio device is NULL \n"
             "exiting", __func__);
   }
   else 
   {
      //                                                                       
      //                                                                            
      //                                                        
      //                                                           
      //                           
      libra_sdio_set_clock(sdio_func_dev, WLAN_LOW_SD_CONFIG_CLOCK_FREQ);
   }
#endif
#endif

#ifdef MSM_PLATFORM_8660
   if(vos_chip_power_qrf8615(CHIP_POWER_OFF))
      return VOS_STATUS_E_FAILURE;
#endif

#ifdef MSM_PLATFORM_7x30
//                                 
   if(vos_chip_power_qrf8600(CHIP_POWER_OFF))
      return VOS_STATUS_E_FAILURE;
//       
#endif

#ifdef MSM_PLATFORM_7x27_FFA
   if(vos_chip_power_7x27_keypad(CHIP_POWER_OFF))
      return VOS_STATUS_E_FAILURE;
#endif

#ifdef MSM_PLATFORM_7x27A
   if(chip_power_qrf6285(CHIP_POWER_OFF))
      return VOS_STATUS_E_FAILURE;
#endif

   return VOS_STATUS_SUCCESS;
}

/* 
                                                             

                                                                       
                                                                           
                                                                         
                                                            

                                                                           
                                                                     

                                                                           
              

                                                                          
                                                         
                                                                                  
                                                                                
                                                                       

          
                                      
                                                                 
                                                                            
                                                                           
                                              
                                                                              
                                                                                        
                                                                                   
                                                                                       
                                                                                         

*/
VOS_STATUS vos_chipReset
(
  vos_call_status_type* status,
  v_BOOL_t              soft,
  vos_power_cb_type     callback,
  v_PVOID_t             user_data,
  vos_chip_reset_reason_type   reason
)
{
   VOS_STATUS vstatus;
   vstatus = vos_watchdog_chip_reset(reason);
   return vstatus;
}


/* 
                                                                        

                                                                       
                                                                           
                                                                         
                                                            

                                                                          
                                                                                
                                                                       

          
                                      
                                                                            
                                                                           
                                              
                                                                              
                                                                                        
                                                                                   
                                                                                       
                                                                                         

*/
VOS_STATUS vos_chipVoteOnPASupply
(
  vos_call_status_type* status,
  vos_power_cb_type     callback,
  v_PVOID_t             user_data
)
{
   return VOS_STATUS_SUCCESS;
}


/* 
                                                                        
                                                                                
                                                            

                                                                       
                                                                           
                                                                         
                                                            

                                                                          
                                                                                
                                                                       

          
                                      
                                                                            
                                                                           
                                              
                                                                              
                                                                                        
                                                                                      
                                                                                    
                                                                                       
                                                                                         

*/
VOS_STATUS vos_chipVoteOffPASupply
(
  vos_call_status_type* status,
  vos_power_cb_type     callback,
  v_PVOID_t             user_data
)
{
   return VOS_STATUS_SUCCESS;
}


/* 
                                                                   
                       

                                                                       
                                                                           
                                                                         
                                                            

                                                                          
                                                                                
                                                                       

          
                                      
                                                                            
                                                                           
                                              
                                                                              
                                                                                        
                                                                                   
                                                                                       
                                                                                         

*/
VOS_STATUS vos_chipAssertDeepSleep
(
  vos_call_status_type* status,
  vos_power_cb_type     callback,
  v_PVOID_t             user_data
)
{
#ifdef FIXME_VOLANS
#ifdef MSM_PLATFORM_7x27_FFA
   int rc = mpp_config_digital_out(MPP_4_CHIP_PWD_L, 
      MPP_CFG(MPP_DLOGIC_LVL_MSMP, MPP_DLOGIC_OUT_CTRL_LOW));
   if (rc) {
      VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR, "%s: Failed to pull high MPP_4_CHIP_PWD_L (%d)",
                __func__, rc);
      return VOS_STATUS_E_FAILURE;
   }

#endif

#ifdef MSM_PLATFORM_7x30
   //                                 
   int rc = pm8xxx_gpio_config(wlan_gpios_reset_out[0].gpio_num, &wlan_gpios_reset_out[0].gpio_cfg);
   if (rc) {
      VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR, "%s: pmic GPIO %d config failed (%d)",
         __func__, wlan_gpios_reset_out[0].gpio_num, rc);
      return VOS_STATUS_E_FAILURE;
   }
#endif
#endif //            

   return VOS_STATUS_SUCCESS;
}


/* 
                                                                             
                 

                                                                       
                                                                           
                                                                         
                                                            

                                                                          
                                                                                
                                                                       

          
                                      
                                                                            
                                                                           
                                              
                                                                              
                                                                                        
                                                                                   
                                                                                       
                                                                                         

*/
VOS_STATUS vos_chipDeAssertDeepSleep
(
  vos_call_status_type* status,
  vos_power_cb_type     callback,
  v_PVOID_t             user_data
)
{
#ifdef FIXME_VOLANS
#ifdef MSM_PLATFORM_7x27_FFA
   int rc = mpp_config_digital_out(MPP_4_CHIP_PWD_L, 
      MPP_CFG(MPP_DLOGIC_LVL_MSMP, MPP_DLOGIC_OUT_CTRL_HIGH));
   if (rc) {
      VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR, "%s: Failed to pull high MPP_4_CHIP_PWD_L (%d)",
         __func__, rc);
      return VOS_STATUS_E_FAILURE;
   }


#endif

#ifdef MSM_PLATFORM_7x30
   //                                 
   int rc = pm8xxx_gpio_config(wlan_gpios_reset[2].gpio_num, &wlan_gpios_reset[2].gpio_cfg);
   if (rc) {
      VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR, "%s: pmic GPIO %d config failed (%d)",
                __func__, wlan_gpios_reset[2].gpio_num, rc);
      return VOS_STATUS_E_FAILURE;
   }
#endif
#endif //            
   return VOS_STATUS_SUCCESS;
}

/* 
                                                                                        
                             

                                                                       
                                                                           
                                                                         
                                                            

                                                                          
                                                                                
                                                                       

          
                                      
                                                                            
                                                                           
                                              
                                                                              
                                                                                        
                                                                                   
                                                                                       
                                                                                         

*/
VOS_STATUS vos_chipExitDeepSleepVREGHandler
(
   vos_call_status_type* status,
   vos_power_cb_type     callback,
   v_PVOID_t             user_data
)
{
#ifdef FIXME_VOLANS
#ifdef MSM_PLATFORM_7x27_FFA
   struct vreg *vreg_wlan;
   int rc;

   vreg_wlan = vreg_get(NULL, "wlan");
   if (IS_ERR(vreg_wlan)) {
      VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR, "%s: wlan vreg get failed (%ld)",
            __func__, PTR_ERR(vreg_wlan));
      return VOS_STATUS_E_FAILURE;
   }

   rc = vreg_set_level(vreg_wlan, 1800);
   if (rc) {
      VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR, "%s: wlan vreg set level failed (%d)",
            __func__, rc);
      return VOS_STATUS_E_FAILURE;
   }

   rc = vreg_enable(vreg_wlan);
   if (rc) {
      VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR, "%s: wlan vreg enable failed (%d)",
            __func__, rc);
      return VOS_STATUS_E_FAILURE;
   }

   msleep(500);

   rc = vreg_set_level(vreg_wlan, 2600);
   if (rc) {
      VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR, "%s: wlan vreg set level failed (%d)",
            __func__, rc);
      return VOS_STATUS_E_FAILURE;
   }

   msleep(500);

   *status = VOS_CALL_SYNC;

#endif

#ifdef MSM_PLATFORM_7x30
   VOS_STATUS vosStatus;
   vos_call_status_type callType;

   vosStatus = vos_chipVoteOnBBAnalogSupply(&callType, NULL, NULL);
   VOS_ASSERT( VOS_IS_STATUS_SUCCESS( vosStatus ) );
   msleep(500);

#endif
#endif //            
   return VOS_STATUS_SUCCESS;
}

/* 
                                                                        

                                                                       
                                                                           
                                                                         
                                                            

                                                                          
                                                                                
                                                                       

          
                                      
                                                                            
                                                                           
                                              
                                                                              
                                                                                        
                                                                                   
                                                                                       
                                                                                         

*/
VOS_STATUS vos_chipVoteOnRFSupply
(
  vos_call_status_type* status,
  vos_power_cb_type     callback,
  v_PVOID_t             user_data
)
{
#ifdef FIXME_VOLANS
#ifdef MSM_PLATFORM_7x30
   int rc;
   struct vreg *vreg_s2 = NULL;
   struct vreg *vreg_s4 = NULL;

   //        
   vreg_s2 = vreg_get(NULL, "s2");
   if (IS_ERR(vreg_s2)) {
      printk(KERN_ERR "%s: s2 vreg get failed (%ld)\n",
         __func__, PTR_ERR(vreg_s2));
      return VOS_STATUS_E_FAILURE;
   }

   //       
   vreg_s4 = vreg_get(NULL, "s4");
   if (IS_ERR(vreg_s4)) {
      printk(KERN_ERR "%s: s4 vreg get failed (%ld)\n",
         __func__, PTR_ERR(vreg_s4));
      return VOS_STATUS_E_FAILURE;
   }

   rc = pmapp_vreg_level_vote(id, PMAPP_VREG_S2, 1300);
   if (rc) {
      VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_FATAL, "%s: s2 vreg vote "
          "level failed (%d)",__func__, rc);
      return VOS_STATUS_E_FAILURE;
   }

   rc = vreg_enable(vreg_s2);
   if (rc) {
      printk(KERN_ERR "%s: s2 vreg enable failed (%d)\n", __func__, rc);
      return VOS_STATUS_E_FAILURE;
   }

   rc = pmapp_vreg_level_vote(id, PMAPP_VREG_S4, 2200);
   if (rc) {
      VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_FATAL, "%s: s4 vreg vote "
          "level failed (%d)",__func__, rc);
      return VOS_STATUS_E_FAILURE;
   }

   rc = vreg_enable(vreg_s4);
   if (rc) {
      printk(KERN_ERR "%s: s4 vreg enable failed (%d)\n", __func__, rc);
      return VOS_STATUS_E_FAILURE;
   }

   return VOS_STATUS_SUCCESS;

#endif //                 
#endif //            
   return VOS_STATUS_SUCCESS;
}

/* 
                                                                        
                                                                                
                                                                             

                                                                       
                                                                           
                                                                         
                                                            

                                                                          
                                                                                
                                                                       

          
                                      
                                                                            
                                                                           
                                              
                                                                              
                                                                                        
                                                                                      
                                                                                    
                                                                                       
                                                                                         

*/
VOS_STATUS vos_chipVoteOffRFSupply
(
  vos_call_status_type* status,
  vos_power_cb_type     callback,
  v_PVOID_t             user_data
)
{
#ifdef FIXME_VOLANS
#ifdef MSM_PLATFORM_7x30

   int rc;
   struct vreg *vreg_s2;
   struct vreg *vreg_s4;

   //       
   vreg_s2 = vreg_get(NULL, "s2");
   if (IS_ERR(vreg_s2)) {
      printk(KERN_ERR "%s: s2 vreg get failed (%ld)\n",
         __func__, PTR_ERR(vreg_s2));
      return VOS_STATUS_E_FAILURE;
   }

   //       
   vreg_s4 = vreg_get(NULL, "s4");
   if (IS_ERR(vreg_s4)) {
      printk(KERN_ERR "%s: s4 vreg get failed (%ld)\n",
         __func__, PTR_ERR(vreg_s4));
      return VOS_STATUS_E_FAILURE;
   }

   rc = pmapp_vreg_level_vote(id, PMAPP_VREG_S2, 0);
   if (rc) {
      VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_WARN, "%s: s2 vreg vote "
          "level failed (%d)",__func__, rc);
   }

   rc = vreg_disable(vreg_s2);
   if (rc) {
      printk(KERN_ERR "%s: s2 vreg disable failed (%d)\n", __func__, rc);
   }

   rc = pmapp_vreg_level_vote(id, PMAPP_VREG_S4, 0);
   if (rc) {
      VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_WARN, "%s: s4 vreg vote "
          "level failed (%d)",__func__, rc);
   }

   rc = vreg_disable(vreg_s4); 
   if (rc) {
      printk(KERN_ERR "%s: s4 vreg disable failed (%d)\n", __func__, rc);
   }

   return VOS_STATUS_SUCCESS;

#endif //                 
#endif //            
   return VOS_STATUS_SUCCESS;
}

/* 
                                                                                
                           

                                                                       
                                                                           
                                                                         
                                                            

                                                                          
                                                                                
                                                                       

          
                                      
                                                                            
                                                                           
                                              
                                                                              
                                                                                        
                                                                                   
                                                                                       
                                                                                         

*/
VOS_STATUS vos_chipVoteOnBBAnalogSupply
(
  vos_call_status_type* status,
  vos_power_cb_type     callback,
  v_PVOID_t             user_data
)
{
#ifdef FIXME_VOLANS
#ifdef MSM_PLATFORM_7x30
   struct vreg *vreg_wlan2 = NULL;
   int rc;

   //                      
   vreg_wlan2 = vreg_get(NULL, "wlan2");
   if (IS_ERR(vreg_wlan2)) {
      VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_FATAL, "%s: wlan2 vreg get "
          "failed (%ld)", __func__, PTR_ERR(vreg_wlan2));
      return VOS_STATUS_E_FAILURE;
   }

   rc = vreg_set_level(vreg_wlan2, 2500);
   if (rc) {
      VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_FATAL, "%s: wlan2 vreg set "
          "level failed (%d)",__func__, rc);
      return VOS_STATUS_E_FAILURE;
   }

   rc = vreg_enable(vreg_wlan2);
   if (rc) {
      VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_FATAL, "%s: wlan2 vreg enable "
          "failed (%d)", __func__, rc);
      return VOS_STATUS_E_FAILURE;
   }
#endif
#endif //            
   return VOS_STATUS_SUCCESS;
}

/* 
                                                                                       

                                                                       
                                                                           
                                                                         
                                                            

                                                                          
                                                                                
                                                                       

          
                                      
                                                                            
                                                                           
                                              
                                                                              
                                                                                        
                                                                                      
                                                                                    
                                                                                       
                                                                                         

*/
VOS_STATUS vos_chipVoteOffBBAnalogSupply
(
  vos_call_status_type* status,
  vos_power_cb_type     callback,
  v_PVOID_t             user_data
)
{
#ifdef FIXME_VOLANS
#ifdef MSM_PLATFORM_7x30
   struct vreg *vreg_wlan2 = NULL;
   int rc;

   //                      
   vreg_wlan2 = vreg_get(NULL, "wlan2");
   if (IS_ERR(vreg_wlan2)) {
      VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_FATAL, "%s: wlan2 vreg get "
          "failed (%ld)", __func__, PTR_ERR(vreg_wlan2));
      return VOS_STATUS_E_FAILURE;
   }

   rc = vreg_disable(vreg_wlan2);
   if (rc) {
      VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_FATAL, "%s: wlan2 vreg disable "
          "failed (%d)", __func__, rc);
      return VOS_STATUS_E_FAILURE;
   }
#endif
#endif //            
   return VOS_STATUS_SUCCESS;
}

/* 
                                                                                    
                                                                           

                                                                       
                                                                           
                                                                         
                                                            

                                                                          
                                                                                
                                                                       

          
                                      
                                                                            
                                                                           
                                              
                                                                              
                                                                                        
                                                                                   
                                                                                       
                                                                                         

*/
VOS_STATUS vos_chipVoteOnXOBuffer
(
  vos_call_status_type* status,
  vos_power_cb_type     callback,
  v_PVOID_t             user_data
)
{
   return VOS_STATUS_SUCCESS;
}

/* 
                                                                           

                                                                       
                                                                           
                                                                         
                                                            

                                                                          
                                                                                
                                                                       

          
                                      
                                                                            
                                                                           
                                              
                                                                              
                                                                                        
                                                                                      
                                                                                    
                                                                                       
                                                                                         

*/
VOS_STATUS vos_chipVoteOffXOBuffer
(
  vos_call_status_type* status,
  vos_power_cb_type     callback,
  v_PVOID_t             user_data
)
{
   return VOS_STATUS_SUCCESS;
}

/* 
                                                              

                                                                       
                                                                           
                                                                         
                                                            

                                                                          
                                                                                
                                                                       
                                                        

          
                                      
                                                                            
                                                                           
                                              
                                                                              
                                                                                        
                                                                                      
                                                                                    
                                                                                       
                                                                                         

*/
VOS_STATUS vos_chipVoteXOCore
(
  vos_call_status_type* status,
  vos_power_cb_type     callback,
  v_PVOID_t             user_data,
  v_BOOL_t              force_enable
)
{
#ifdef FEATURE_WLAN_NON_INTEGRATED_SOC
    static v_BOOL_t is_vote_on;

#if defined(MSM_PLATFORM_8660) || defined(MSM_PLATFORM_7x30)
    int rc;
#endif

   /*                                                                                      
                                                      
    */
   if ((force_enable && is_vote_on) || (!force_enable && !is_vote_on)) {
      VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR,
         "Force XO Core %s called when already %s - directly return success %d", 
         force_enable ? "enable" : "disable", is_vote_on ? "enable" : "disable", 
         is_vote_on);
      goto success;
   }    

#ifdef MSM_PLATFORM_7x30
   rc = pmic_xo_core_force_enable(force_enable);
   if (rc) {
      VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_FATAL,
         "%s: pmic_xo_core_force_enable %s failed (%d)",__func__, 
         force_enable ? "enable" : "disable",rc);
      return VOS_STATUS_E_FAILURE;
   }
#endif

#ifdef MSM_PLATFORM_8660
   rc = qcomwlan_pmic_xo_core_force_enable(force_enable);
   if (rc) {
      VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_FATAL,
         "%s: pmic_xo_core_force_enable %s failed (%d)",__func__, 
         force_enable ? "enable" : "disable",rc);
      return VOS_STATUS_E_FAILURE;
   }
#endif
    is_vote_on=force_enable;

success:
   VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_WARN, "XO CORE ON vote %s successfully!",
                force_enable ? "enable" : "disable");

#endif /*                                 */

   return VOS_STATUS_SUCCESS;
}


/* 
                                                                                               
  
                                                                       
                                                                           
                                                                         
                                                            

                                                                                 
                                                                                
                                                                                  
              
   
                                                                          
                                                                                
                                                                       
                                                                                            
          
                                      
                                                                            
                                                                           
                                              
                                                                              
                                                                                        
                                                                                      
                                                                                    
                                                                                       
                                                                                         

*/
VOS_STATUS vos_chipVoteFreqFor1p3VSupply
(
  vos_call_status_type* status,
  vos_power_cb_type     callback,
  v_PVOID_t             user_data,
  v_U32_t               freq
)
{


#ifdef MSM_PLATFORM_8660
    if(freq == VOS_NV_FREQUENCY_FOR_1_3V_SUPPLY_3P2MH)
        {
            if(qcomwlan_freq_change_1p3v_supply(RPM_VREG_FREQ_3p20))
                return VOS_STATUS_E_FAILURE;
        }
    else
        {
            if(qcomwlan_freq_change_1p3v_supply(RPM_VREG_FREQ_1p60))
                return VOS_STATUS_E_FAILURE;
        }
#endif

    return VOS_STATUS_SUCCESS;
}
