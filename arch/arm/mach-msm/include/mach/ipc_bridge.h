/* Copyright (c) 2013 The Linux Foundation. All rights reserved.
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

#ifndef __MSM_IPC_BRIDGE_H__
#define __MSM_IPC_BRIDGE_H__

#include <linux/platform_device.h>

/*
                                                                   
                                                                    
                                                                     
                                                                  
                                                                      
                                                                         
                                                                      
                                                                         
                     
 */

/* 
                                                                 
                                 
                                                  
                                                    
                                                                      
                                                                      
                                                                       
                                                                        
                                                                       
                             
                                                                          
                                                                     
                                                                       
                                                                         
                                                            
                                                                            
                                                                           
                                                                                
                                                                               
                                                                       
                                                                        
                                                                              
 */
struct ipc_bridge_platform_data {
	unsigned int max_read_size;
	unsigned int max_write_size;
	int (*open)(struct platform_device *pdev);
	int (*read)(struct platform_device *pdev, char *buf,
			unsigned int count);
	int (*write)(struct platform_device *pdev, char *buf,
			unsigned int count);
	void (*close)(struct platform_device *pdev);
};

#endif
