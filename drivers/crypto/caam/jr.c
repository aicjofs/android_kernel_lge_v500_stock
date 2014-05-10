/*
 * CAAM/SEC 4.x transport/backend driver
 * JobR backend functionality
 *
 * Copyright 2008-2011 Freescale Semiconductor, Inc.
 */

#include "compat.h"
#include "regs.h"
#include "jr.h"
#include "desc.h"
#include "intern.h"

/*                                 */
static irqreturn_t caam_jr_interrupt(int irq, void *st_dev)
{
	struct device *dev = st_dev;
	struct caam_drv_private_jr *jrp = dev_get_drvdata(dev);
	u32 irqstate;

	/*
                                                   
                         
  */
	irqstate = rd_reg32(&jrp->rregs->jrintstatus);
	if (!irqstate)
		return IRQ_NONE;

	/*
                                                     
                                                       
                                     
  */
	if (irqstate & JRINT_JR_ERROR) {
		dev_err(dev, "job ring error: irqstate: %08x\n", irqstate);
		BUG();
	}

	/*                       */
	setbits32(&jrp->rregs->rconfig_lo, JRCFG_IMSK);

	/*                                                          */
	wr_reg32(&jrp->rregs->jrintstatus, irqstate);

	preempt_disable();
	tasklet_schedule(&jrp->irqtask[smp_processor_id()]);
	preempt_enable();

	return IRQ_HANDLED;
}

/*                                                          */
static void caam_jr_dequeue(unsigned long devarg)
{
	int hw_idx, sw_idx, i, head, tail;
	struct device *dev = (struct device *)devarg;
	struct caam_drv_private_jr *jrp = dev_get_drvdata(dev);
	void (*usercall)(struct device *dev, u32 *desc, u32 status, void *arg);
	u32 *userdesc, userstatus;
	void *userarg;
	unsigned long flags;

	spin_lock_irqsave(&jrp->outlock, flags);

	head = ACCESS_ONCE(jrp->head);
	sw_idx = tail = jrp->tail;

	while (CIRC_CNT(head, tail, JOBR_DEPTH) >= 1 &&
	       rd_reg32(&jrp->rregs->outring_used)) {

		hw_idx = jrp->out_ring_read_index;
		for (i = 0; CIRC_CNT(head, tail + i, JOBR_DEPTH) >= 1; i++) {
			sw_idx = (tail + i) & (JOBR_DEPTH - 1);

			smp_read_barrier_depends();

			if (jrp->outring[hw_idx].desc ==
			    jrp->entinfo[sw_idx].desc_addr_dma)
				break; /*       */
		}
		/*                                                    */
		BUG_ON(CIRC_CNT(head, tail + i, JOBR_DEPTH) <= 0);

		/*                                                  */
		dma_unmap_single(dev, jrp->outring[hw_idx].desc,
				 jrp->entinfo[sw_idx].desc_size,
				 DMA_TO_DEVICE);

		/*                                                        */
		jrp->entinfo[sw_idx].desc_addr_dma = 0;

		/*                                               */
		usercall = jrp->entinfo[sw_idx].callbk;
		userarg = jrp->entinfo[sw_idx].cbkarg;
		userdesc = jrp->entinfo[sw_idx].desc_addr_virt;
		userstatus = jrp->outring[hw_idx].jrstatus;

		smp_mb();

		jrp->out_ring_read_index = (jrp->out_ring_read_index + 1) &
					   (JOBR_DEPTH - 1);

		/*
                                                         
                                                       
                                                             
   */
		if (sw_idx == tail) {
			do {
				tail = (tail + 1) & (JOBR_DEPTH - 1);
				smp_read_barrier_depends();
			} while (CIRC_CNT(head, tail, JOBR_DEPTH) >= 1 &&
				 jrp->entinfo[tail].desc_addr_dma == 0);

			jrp->tail = tail;
		}

		/*          */
		wr_reg32(&jrp->rregs->outring_rmvd, 1);

		spin_unlock_irqrestore(&jrp->outlock, flags);

		/*                                  */
		usercall(dev, userdesc, userstatus, userarg);

		spin_lock_irqsave(&jrp->outlock, flags);

		head = ACCESS_ONCE(jrp->head);
		sw_idx = tail = jrp->tail;
	}

	spin_unlock_irqrestore(&jrp->outlock, flags);

	/*                        */
	clrbits32(&jrp->rregs->rconfig_lo, JRCFG_IMSK);
}

/* 
                                                                          
                                                                      
                 
                                                             
                                          
                                                                     
                                                 
  */
int caam_jr_register(struct device *ctrldev, struct device **rdev)
{
	struct caam_drv_private *ctrlpriv = dev_get_drvdata(ctrldev);
	struct caam_drv_private_jr *jrpriv = NULL;
	unsigned long flags;
	int ring;

	/*                                     */
	spin_lock_irqsave(&ctrlpriv->jr_alloc_lock, flags);
	for (ring = 0; ring < ctrlpriv->total_jobrs; ring++) {
		jrpriv = dev_get_drvdata(ctrlpriv->jrdev[ring]);
		if (jrpriv->assign == JOBR_UNASSIGNED) {
			jrpriv->assign = JOBR_ASSIGNED;
			*rdev = ctrlpriv->jrdev[ring];
			spin_unlock_irqrestore(&ctrlpriv->jr_alloc_lock, flags);
			return ring;
		}
	}

	/*                                              */
	spin_unlock_irqrestore(&ctrlpriv->jr_alloc_lock, flags);
	*rdev = NULL;

	return -ENODEV;
}
EXPORT_SYMBOL(caam_jr_register);

/* 
                                                                  
                                                                  
                                                 
                                                            
                          
  */
int caam_jr_deregister(struct device *rdev)
{
	struct caam_drv_private_jr *jrpriv = dev_get_drvdata(rdev);
	struct caam_drv_private *ctrlpriv;
	unsigned long flags;

	/*                                           */
	ctrlpriv = dev_get_drvdata(jrpriv->parentdev);

	/*
                                       
  */
	if (rd_reg32(&jrpriv->rregs->outring_used) ||
	    (rd_reg32(&jrpriv->rregs->inpring_avail) != JOBR_DEPTH))
		return -EBUSY;

	/*              */
	spin_lock_irqsave(&ctrlpriv->jr_alloc_lock, flags);
	jrpriv->assign = JOBR_UNASSIGNED;
	spin_unlock_irqrestore(&ctrlpriv->jr_alloc_lock, flags);

	return 0;
}
EXPORT_SYMBOL(caam_jr_deregister);

/* 
                                                                      
                                                                  
              
                                                                    
                                                    
                                                                  
                                                                    
                                                                    
                                                               
                 
                                                                      
                                             
                                                                      
                
                                                                   
                            
                                                                 
                                                            
                                                                  
                                                                 
                                                                     
                                       
                                                                  
                                   
                                                                 
               
  */
int caam_jr_enqueue(struct device *dev, u32 *desc,
		    void (*cbk)(struct device *dev, u32 *desc,
				u32 status, void *areq),
		    void *areq)
{
	struct caam_drv_private_jr *jrp = dev_get_drvdata(dev);
	struct caam_jrentry_info *head_entry;
	unsigned long flags;
	int head, tail, desc_size;
	dma_addr_t desc_dma;

	desc_size = (*desc & HDR_JD_LENGTH_MASK) * sizeof(u32);
	desc_dma = dma_map_single(dev, desc, desc_size, DMA_TO_DEVICE);
	if (dma_mapping_error(dev, desc_dma)) {
		dev_err(dev, "caam_jr_enqueue(): can't map jobdesc\n");
		return -EIO;
	}

	spin_lock_irqsave(&jrp->inplock, flags);

	head = jrp->head;
	tail = ACCESS_ONCE(jrp->tail);

	if (!rd_reg32(&jrp->rregs->inpring_avail) ||
	    CIRC_SPACE(head, tail, JOBR_DEPTH) <= 0) {
		spin_unlock_irqrestore(&jrp->inplock, flags);
		dma_unmap_single(dev, desc_dma, desc_size, DMA_TO_DEVICE);
		return -EBUSY;
	}

	head_entry = &jrp->entinfo[head];
	head_entry->desc_addr_virt = desc;
	head_entry->desc_size = desc_size;
	head_entry->callbk = (void *)cbk;
	head_entry->cbkarg = areq;
	head_entry->desc_addr_dma = desc_dma;

	jrp->inpring[jrp->inp_ring_write_index] = desc_dma;

	smp_wmb();

	jrp->inp_ring_write_index = (jrp->inp_ring_write_index + 1) &
				    (JOBR_DEPTH - 1);
	jrp->head = (head + 1) & (JOBR_DEPTH - 1);

	wmb();

	wr_reg32(&jrp->rregs->inpring_jobadd, 1);

	spin_unlock_irqrestore(&jrp->inplock, flags);

	return 0;
}
EXPORT_SYMBOL(caam_jr_enqueue);

static int caam_reset_hw_jr(struct device *dev)
{
	struct caam_drv_private_jr *jrp = dev_get_drvdata(dev);
	unsigned int timeout = 100000;

	/*
                                              
                               
  */
	setbits32(&jrp->rregs->rconfig_lo, JRCFG_IMSK);

	/*                                          */
	wr_reg32(&jrp->rregs->jrcommand, JRCR_RESET);
	while (((rd_reg32(&jrp->rregs->jrintstatus) & JRINT_ERR_HALT_MASK) ==
		JRINT_ERR_HALT_INPROGRESS) && --timeout)
		cpu_relax();

	if ((rd_reg32(&jrp->rregs->jrintstatus) & JRINT_ERR_HALT_MASK) !=
	    JRINT_ERR_HALT_COMPLETE || timeout == 0) {
		dev_err(dev, "failed to flush job ring %d\n", jrp->ridx);
		return -EIO;
	}

	/*                */
	timeout = 100000;
	wr_reg32(&jrp->rregs->jrcommand, JRCR_RESET);
	while ((rd_reg32(&jrp->rregs->jrcommand) & JRCR_RESET) && --timeout)
		cpu_relax();

	if (timeout == 0) {
		dev_err(dev, "failed to reset job ring %d\n", jrp->ridx);
		return -EIO;
	}

	/*                   */
	clrbits32(&jrp->rregs->rconfig_lo, JRCFG_IMSK);

	return 0;
}

/*
                                                       
 */
static int caam_jr_init(struct device *dev)
{
	struct caam_drv_private_jr *jrp;
	dma_addr_t inpbusaddr, outbusaddr;
	int i, error;

	jrp = dev_get_drvdata(dev);

	/*                                     */
	for_each_possible_cpu(i)
		tasklet_init(&jrp->irqtask[i], caam_jr_dequeue,
			     (unsigned long)dev);

	error = request_irq(jrp->irq, caam_jr_interrupt, IRQF_SHARED,
			    "caam-jobr", dev);
	if (error) {
		dev_err(dev, "can't connect JobR %d interrupt (%d)\n",
			jrp->ridx, jrp->irq);
		irq_dispose_mapping(jrp->irq);
		jrp->irq = 0;
		return -EINVAL;
	}

	error = caam_reset_hw_jr(dev);
	if (error)
		return error;

	jrp->inpring = kzalloc(sizeof(dma_addr_t) * JOBR_DEPTH,
			       GFP_KERNEL | GFP_DMA);
	jrp->outring = kzalloc(sizeof(struct jr_outentry) *
			       JOBR_DEPTH, GFP_KERNEL | GFP_DMA);

	jrp->entinfo = kzalloc(sizeof(struct caam_jrentry_info) * JOBR_DEPTH,
			       GFP_KERNEL);

	if ((jrp->inpring == NULL) || (jrp->outring == NULL) ||
	    (jrp->entinfo == NULL)) {
		dev_err(dev, "can't allocate job rings for %d\n",
			jrp->ridx);
		return -ENOMEM;
	}

	for (i = 0; i < JOBR_DEPTH; i++)
		jrp->entinfo[i].desc_addr_dma = !0;

	/*             */
	inpbusaddr = dma_map_single(dev, jrp->inpring,
				    sizeof(u32 *) * JOBR_DEPTH,
				    DMA_BIDIRECTIONAL);
	if (dma_mapping_error(dev, inpbusaddr)) {
		dev_err(dev, "caam_jr_init(): can't map input ring\n");
		kfree(jrp->inpring);
		kfree(jrp->outring);
		kfree(jrp->entinfo);
		return -EIO;
	}

	outbusaddr = dma_map_single(dev, jrp->outring,
				    sizeof(struct jr_outentry) * JOBR_DEPTH,
				    DMA_BIDIRECTIONAL);
	if (dma_mapping_error(dev, outbusaddr)) {
		dev_err(dev, "caam_jr_init(): can't map output ring\n");
			dma_unmap_single(dev, inpbusaddr,
					 sizeof(u32 *) * JOBR_DEPTH,
					 DMA_BIDIRECTIONAL);
		kfree(jrp->inpring);
		kfree(jrp->outring);
		kfree(jrp->entinfo);
		return -EIO;
	}

	jrp->inp_ring_write_index = 0;
	jrp->out_ring_read_index = 0;
	jrp->head = 0;
	jrp->tail = 0;

	wr_reg64(&jrp->rregs->inpring_base, inpbusaddr);
	wr_reg64(&jrp->rregs->outring_base, outbusaddr);
	wr_reg32(&jrp->rregs->inpring_size, JOBR_DEPTH);
	wr_reg32(&jrp->rregs->outring_size, JOBR_DEPTH);

	jrp->ringsize = JOBR_DEPTH;

	spin_lock_init(&jrp->inplock);
	spin_lock_init(&jrp->outlock);

	/*                                        */
	setbits32(&jrp->rregs->rconfig_lo, JOBR_INTC |
		  (JOBR_INTC_COUNT_THLD << JRCFG_ICDCT_SHIFT) |
		  (JOBR_INTC_TIME_THLD << JRCFG_ICTT_SHIFT));

	jrp->assign = JOBR_UNASSIGNED;
	return 0;
}

/*
                                                      
 */
int caam_jr_shutdown(struct device *dev)
{
	struct caam_drv_private_jr *jrp = dev_get_drvdata(dev);
	dma_addr_t inpbusaddr, outbusaddr;
	int ret, i;

	ret = caam_reset_hw_jr(dev);

	for_each_possible_cpu(i)
		tasklet_kill(&jrp->irqtask[i]);

	/*                   */
	free_irq(jrp->irq, dev);

	/*            */
	inpbusaddr = rd_reg64(&jrp->rregs->inpring_base);
	outbusaddr = rd_reg64(&jrp->rregs->outring_base);
	dma_unmap_single(dev, outbusaddr,
			 sizeof(struct jr_outentry) * JOBR_DEPTH,
			 DMA_BIDIRECTIONAL);
	dma_unmap_single(dev, inpbusaddr, sizeof(u32 *) * JOBR_DEPTH,
			 DMA_BIDIRECTIONAL);
	kfree(jrp->outring);
	kfree(jrp->inpring);
	kfree(jrp->entinfo);

	return ret;
}

/*
                                                                  
                                               
 */
int caam_jr_probe(struct platform_device *pdev, struct device_node *np,
		  int ring)
{
	struct device *ctrldev, *jrdev;
	struct platform_device *jr_pdev;
	struct caam_drv_private *ctrlpriv;
	struct caam_drv_private_jr *jrpriv;
	u32 *jroffset;
	int error;

	ctrldev = &pdev->dev;
	ctrlpriv = dev_get_drvdata(ctrldev);

	jrpriv = kmalloc(sizeof(struct caam_drv_private_jr),
			 GFP_KERNEL);
	if (jrpriv == NULL) {
		dev_err(ctrldev, "can't alloc private mem for job ring %d\n",
			ring);
		return -ENOMEM;
	}
	jrpriv->parentdev = ctrldev; /*                      */
	jrpriv->ridx = ring; /*                                          */

	/*
                                               
                                                         
                                                           
                                      
  */
	jroffset = (u32 *)of_get_property(np, "reg", NULL);
	jrpriv->rregs = (struct caam_job_ring __iomem *)((void *)ctrlpriv->ctrl
							 + *jroffset);

	/*                                           */
	jr_pdev = of_platform_device_create(np, NULL, ctrldev);
	if (jr_pdev == NULL) {
		kfree(jrpriv);
		return -EINVAL;
	}
	jrdev = &jr_pdev->dev;
	dev_set_drvdata(jrdev, jrpriv);
	ctrlpriv->jrdev[ring] = jrdev;

	/*                        */
	jrpriv->irq = of_irq_to_resource(np, 0, NULL);

	/*                                      */
	error = caam_jr_init(jrdev); /*                      */
	if (error) {
		kfree(jrpriv);
		return error;
	}

	return error;
}