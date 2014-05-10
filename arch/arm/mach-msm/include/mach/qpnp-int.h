/* Copyright (c) 2012, The Linux Foundation. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#ifndef QPNPINT_H
#define QPNPINT_H

#include <linux/spmi.h>

struct qpnp_irq_spec {
	uint8_t slave; /*      */
	uint8_t per; /*       */
	uint8_t irq; /*     */
};

struct qpnp_local_int {
	 /*                                               */
	int (*mask)(struct spmi_controller *spmi_ctrl,
		    struct qpnp_irq_spec *spec,
		    uint32_t priv_d);
	 /*                                                   */
	int (*unmask)(struct spmi_controller *spmi_ctrl,
		      struct qpnp_irq_spec *spec,
		      uint32_t priv_d);
	/*                                               */
	int (*register_priv_data)(struct spmi_controller *spmi_ctrl,
				  struct qpnp_irq_spec *spec,
				  uint32_t *priv_d);
};

#ifdef CONFIG_MSM_QPNP_INT
/* 
                                                     
  
                                                      
                 
 */
int __init qpnpint_of_init(struct device_node *node,
			   struct device_node *parent);

/* 
                                                                     
  
                                                            
                                  
 */
int qpnpint_register_controller(struct device_node *node,
				struct spmi_controller *ctrl,
				struct qpnp_local_int *li_cb);

/* 
                                                       
  
                                          
 */
int qpnpint_handle_irq(struct spmi_controller *spmi_ctrl,
		       struct qpnp_irq_spec *spec);
#else
static inline int __init qpnpint_of_init(struct device_node *node,
				  struct device_node *parent)
{
	return -ENXIO;
}

static inline int qpnpint_register_controller(struct device_node *node,
					      struct spmi_controller *ctrl,
					      struct qpnp_local_int *li_cb)

{
	return -ENXIO;
}

static inline int qpnpint_handle_irq(struct spmi_controller *spmi_ctrl,
		       struct qpnp_irq_spec *spec)
{
	return -ENXIO;
}
#endif /*                     */
#endif /*           */
