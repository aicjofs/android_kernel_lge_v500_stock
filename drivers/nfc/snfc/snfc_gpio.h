/*
             
 
*/


#ifndef __SNFC_GPIO_H__
#define __SNFC_GPIO_H__


/*
                            
 */
#include <linux/gpio.h>
#include "snfc_common.h"
/*      */

enum{
  GPIO_DIRECTION_IN = 0,
  GPIO_DIRECTION_OUT,
};

enum{
  GPIO_LOW_VALUE = 0,
  GPIO_HIGH_VALUE,
};

enum{
  GPIO_CONFIG_ENABLE = 0,
  GPIO_CONFIG_DISABLE,
};

/*          */
#define GPIO_SNFC_PON		37  	//       

/*          */
#define GPIO_SNFC_RFS   	29		//        

/*          */
#define GPIO_SNFC_INT   	38		//       

/*               */
#define GPIO_SNFC_INTU   	23		//        

/*           */
#ifdef CONFIG_CXD2235AGG_GJ_KDDI
#define GPIO_SNFC_HSEL		57		//        
#else
#define GPIO_SNFC_HSEL		69		//        
#endif

#define SNFC_GPIO_CFG(gpio, func, dir, pull, drvstr) \
    ((((gpio) & 0x3FF) << 4)        |   \
    ((func) & 0xf)                  |   \
    (((dir) & 0x1) << 14)           |   \
    (((pull) & 0x3) << 15)          |   \
    (((drvstr) & 0xF) << 17))

extern int gpio_rfs;

/*
                     
 */
int snfc_gpio_open(int gpionum, int direction, int value);
void snfc_gpio_write(int gpionum, int value);
int snfc_gpio_read(int gpionum);

#endif  //               
