#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/interrupt.h>


#include "raontv.h"
#include "raontv_internal.h"

#include "mtv250.h"


//                          

#if defined(RTV_IF_SPI) || (defined(RTV_TDMB_ENABLE) && !defined(RTV_TDMB_FIC_POLLING_MODE))	

void mtv250_isr_handler(void)
{
#ifdef DEBUG_TS_READ_TIME
	unsigned long diff_jiffies;
#endif	
	U8 int_type_val1;
	MTV250_TS_PKT_INFO *tsp = NULL;

	RTV_GUARD_LOCK;

	RTV_MASTER_CHIP_SEL;

	RTV_REG_MAP_SEL(DD_PAGE);
	int_type_val1 = RTV_REG_GET(INT_E_STATL);
//                                             
	
#if defined(RTV_IF_SPI) 
	/*                                                                              
                              
                                            
                       
                
                                                                               */  
	if( int_type_val1 & (MSC1_E_INT|MSC1_E_OVER_FLOW|MSC1_E_UNDER_FLOW) )
	{
		if( int_type_val1 & (MSC1_E_OVER_FLOW|MSC1_E_UNDER_FLOW) )   //                                  
		{
			RTV_REG_SET(MSC1_E_CON, 0x00);  //                                    
			RTV_REG_SET(MSC1_E_CON, 0x05);  //                    
			RTV_REG_SET(INT_E_UCLRL, 0x04); //                             

			mtv250_cb_ptr->ovf_err_cnt++;
			DMBERR("[mtv isr] OvUf Err: 0x%02X\n", int_type_val1);
		}
		else
		{	
			/*                                     */
			tsp = mtv250_alloc_tsp();
			if(tsp != NULL)
			{
				RTV_REG_MAP_SEL(MSC1_PAGE);
				RTV_REG_BURST_GET(0x10, tsp->msc_buf, MTV_TS_THRESHOLD_SIZE+1);

				//                    

		#ifdef DEBUG_TS_READ_TIME
				mtv250_cb_ptr->after_burst_read_time = get_jiffies_64();
				diff_jiffies = mtv250_cb_ptr->after_burst_read_time - mtv250_cb_ptr->start_irq_time;						
				mtv250_cb_ptr->max_read_jiffies = MAX(mtv250_cb_ptr->max_read_jiffies, diff_jiffies);

				if(diff_jiffies >= 2)
				{
					printk("[mtv isr] max_read_jiffies: %u, diff_jiffies: %ld, diff_ms: %u\n", 
						mtv250_cb_ptr->max_read_jiffies, diff_jiffies, jiffies_to_msecs(diff_jiffies));
				}	
		#endif
							
				tsp->len = MTV_TS_THRESHOLD_SIZE;

			//                                                                                                                          

				RTV_REG_MAP_SEL(DD_PAGE);
				RTV_REG_SET(INT_E_UCLRL, 0x04); //                             

				/*                                         */
				mtv250_put_tsp(tsp);	
			}
			else
			{
				RTV_REG_SET(MSC1_E_CON, 0x00);	//                                    
				RTV_REG_SET(MSC1_E_CON, 0x05);	//                    
				RTV_REG_SET(INT_E_UCLRL, 0x04); //                             
				DMBERR("[mtv isr] No more TSP...\n");
			}
			
		#ifdef DEBUG_TS_READ_TIME
			diff_jiffies = get_jiffies_64() - mtv250_cb_ptr->start_irq_time;
			if(diff_jiffies >= 2)
			{
				printk("[mtv isr] total isr time: %ld ms\n", jiffies_to_msecs(diff_jiffies));
			}
		#endif		
		}
	}
#endif /*                         */

	if(tsp != NULL)
	{
    #ifndef MTV250_NON_BLOCKING_READ_MODE
		wake_up(&mtv250_cb_ptr->read_wq);			
    #endif
	}

	RTV_GUARD_FREE;
}


int mtv250_isr_thread(void *data)
{
	DMBMSG("[mtv250_isr_thread] Start...\n");

	while(!kthread_should_stop()) 
	{
		wait_event_interruptible(mtv250_cb_ptr->isr_wq,
				  			     kthread_should_stop() || mtv250_cb_ptr->isr_cnt);

		if(kthread_should_stop())
			break;

		if(mtv250_cb_ptr->is_power_on == TRUE)
		{
			mtv250_isr_handler();
			mtv250_cb_ptr->isr_cnt--;
		}
	}

	DMBMSG("[mtv250_isr_thread] Exit.\n");

	return 0;
}


irqreturn_t mtv250_isr(int irq, void *param)
{
	//                     

	if(mtv250_cb_ptr->is_power_on == TRUE)
	{
#ifdef DEBUG_TS_READ_TIME	
		mtv250_cb_ptr->start_irq_time = get_jiffies_64();
#endif	
		mtv250_cb_ptr->start_ts_int_cnt++;

		mtv250_cb_ptr->isr_cnt++;
		wake_up(&mtv250_cb_ptr->isr_wq);
	}

	return IRQ_HANDLED;
}

#endif /*                                                                                              */

