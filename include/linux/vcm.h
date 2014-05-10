/* Copyright (c) 2010, The Linux Foundation. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#ifndef _VCM_H_
#define _VCM_H_

/*                                                                     */

#include <linux/vcm_types.h>

/*
                                                     
  
                                                                
                                                                        
                                                                     
                                                                       
  
                                                                      
                                                                       
                                                                        
                                                                   
          
  
                                                                
                                                           
                                                                    
                      
  
                                                           
                                                              
                                                                      
                                                             
  
                                                                      
                                                                   
                                                                      
                                               
 */


/*
                                       
  
                                                                
                                                                
                                                                      
                                                                  
 */

/* 
                                     
                                                        
                                                          
                                   
  
                                          
  
                                                                
                                                             
                                                                    
                                   
  
                                                              
                                                                    
                  
 */
struct vcm *vcm_create(unsigned long start_addr, size_t len);


/* 
                                                                           
                                                         
                                                        
  
                                                                        
             
  
                                                          
  
                                                                       
 */
struct vcm *vcm_create_from_prebuilt(size_t ext_vcm_id);


/* 
                            
                              
  
                                                                          
                                                                      
                    
  
                                                                      
 */
struct vcm *vcm_clone(struct vcm *vcm);


/* 
                                                                     
                                                          
                
  
                                                       
 */
size_t vcm_get_start_addr(struct vcm *vcm);


/* 
                                                    
                                                              
  
                                                                 
                     
 */
size_t vcm_get_len(struct vcm *vcm);


/* 
                           
                                              
  
                                                      
                                                           
                                                    
                                        
 */
int vcm_free(struct vcm *vcm);


/*
                                                            
  
 */

/* 
                                                          
                                         
                                                         
                                   
                                        
  
                                                                 
  
                                                                      
                                                         
 */
struct res *vcm_reserve(struct vcm *vcm, size_t len, u32 attr);


/* 
                                                                     
                                                                 
                                                       
                                        
                                        
  
                                                                      
           
 */
struct res *vcm_reserve_at(enum memtarget_t memtarget, struct vcm *vcm,
			   size_t len, u32 attr);


/* 
                                                                   
                                                      
  
                                                                        
                                                                       
                                  
 */
struct vcm *vcm_get_vcm_from_res(struct res *res);


/* 
                                               
                                       
  
                                                                 
                  
                                           
                                        
 */
int vcm_unreserve(struct res *res);


/* 
                                                                  
                                              
                                        
  
                                                                   
                                                                      
               
  
                                                                  
                                    
 */
int vcm_set_res_attr(struct res *res, u32 attr);


/* 
                                                                         
                                     
 */
size_t vcm_get_num_res(struct vcm *vcm);


/* 
                                                            
                                     
                                                         
                
  
                                                                  
  
                                                                     
                                          
 */
struct res *vcm_get_next_res(struct vcm *vcm, struct res *res);


/* 
                                                                   
                                     
                                        
                                      
  
                                                  
 */
size_t vcm_res_copy(struct res *to, size_t to_off, struct res *from, size_t
		    from_off, size_t len);


/* 
                                                                      
                           
 */
size_t vcm_get_min_page_size(void);


/* 
                                              
                                                           
                    
                                                            
                              
  
                                                                       
                                                                          
                            
  
                                                                      
              
                                                  
                                     
                                                                
 */
int vcm_back(struct res *res, struct physmem *physmem);


/* 
                                       
                                    
  
                                                                         
                                   
  
                                                               
              
                           
                                                                
 */
int vcm_unback(struct res *res);


/* 
                                                                  
                                          
                                       
                                                
  
                                                                     
                
  
                                                           
                          
 */
struct physmem *vcm_phys_alloc(enum memtype_t memtype, size_t len, u32 attr);


/* 
                                                
                                              
  
                                                                      
                                                             
                                            
 */
int vcm_phys_free(struct physmem *physmem);


/* 
                                                              
                                              
  
                                                                      
                          
                           
 */
struct physmem *vcm_get_physmem_from_res(struct res *res);


/* 
                                                                        
                                                  
  
                                                                    
                     
 */
enum memtype_t vcm_get_memtype_of_physalloc(struct physmem *physmem);


/*
                                                                          
  
 */

/* 
                                               
                                     
                                               
                                        
  
                                                                        
                                                                         
 */
struct avcm *vcm_assoc(struct vcm *vcm, struct device *dev, u32 attr);


/* 
                                                   
                                            
  
                                        
                                                  
                                          
 */
int vcm_deassoc(struct avcm *avcm);


/* 
                                                   
                                
                                                      
  
                                                                           
  
                                         
                                                  
                                          
 */
int vcm_set_assoc_attr(struct avcm *avcm, u32 attr);


/* 
                                                      
                                
  
                                    
 */
u32 vcm_get_assoc_attr(struct avcm *avcm);


/* 
                                     
                                
  
                                               
  
                                         
                           
                     
                                   
                        
 */
int vcm_activate(struct avcm *avcm);


/* 
                                                
                                  
  
                                         
                                
                           
                        
 */
int vcm_deactivate(struct avcm *avcm);


/* 
                                                
                                
  
                                                               
  
 */
int vcm_is_active(struct avcm *avcm);


/*
                                                 
 */

/* 
                                                
                                     
                               
  
                                                          
  
                                                         
 */
struct bound *vcm_create_bound(struct vcm *vcm, size_t len);


/* 
                                   
                                
  
                                                        
                                                                 
                                 
 */
int vcm_free_bound(struct bound *bound);


/* 
                                                                     
                                      
                                     
                                        
  
                                                                    
              
 */
struct res *vcm_reserve_from_bound(struct bound *bound, size_t len,
				   u32 attr);


/* 
                                                                               
                                  
  
                                                                              
              
                           
 */
size_t vcm_get_bound_start_addr(struct bound *bound);



/*
                                                               
 */

/* 
                                                            
                                         
  
                                                        
                                                  
  
                                             
 */
struct physmem *vcm_map_phys_addr(phys_addr_t phys, size_t len);


/* 
                                                                              
                                      
                                                            
                         
                                                          
  
                                                               
                                                                   
                                                                  
                               
  
                                                  
 */
size_t vcm_get_next_phys_addr(struct physmem *physmem, phys_addr_t phys,
			      size_t *len);


/* 
                                                                   
                                      
  
  
                                                                           
                    
                      
  
                                                                
                                                                      
                              
 */
phys_addr_t vcm_get_dev_addr(struct res *res);


/* 
                                                                         
                                              
                                               
  
                                                                  
                       
 */
struct res *vcm_get_res(unsigned long dev_addr, struct vcm *vcm);


/* 
                                                                  
                                        
                                    
                                         
  
                                                                          
                                                      
  
                                                                           
                    
                         
 */
size_t vcm_translate(struct device *src_dev, struct vcm *src_vcm,
		     struct vcm *dst_vcm);


/* 
                                                                       
                          
                                        
 */
size_t vcm_get_phys_num_res(phys_addr_t phys);


/* 
                                                                             
                  
                                       
                                                                  
                 
                                               
  
                                                                      
 */
struct res *vcm_get_next_phys_res(phys_addr_t phys, struct res *res,
				  size_t *len);


/* 
                                                                          
                                    
  
                                             
 */
phys_addr_t vcm_get_pgtbl_pa(struct vcm *vcm);


/* 
                                                                       
                                    
                                      
  
                                                                          
                                                                          
              
 */
phys_addr_t vcm_get_cont_memtype_pa(enum memtype_t memtype);


/* 
                                                             
                              
                                      
  
                                                                          
                                                                          
              
 */
size_t vcm_get_cont_memtype_len(enum memtype_t memtype);


/* 
                                                                            
                                     
                                         
  
                                                                           
                                       
 */
phys_addr_t vcm_dev_addr_to_phys_addr(struct vcm *vcm, unsigned long dev_addr);


/*
              
  
             
 */

/* 
                                    
                     
                          
                                                              
             
  
                                                            
                                      
 */
int vcm_hook(struct device *dev, vcm_handler handler, void *data);



/*
                                            
  
               
 */

/* 
                                                                         
                    
 */
size_t vcm_hw_ver(size_t dev);

#endif /*         */

