/*

  Broadcom B43 wireless driver

  DMA ringbuffer and descriptor allocation/management

  Copyright (c) 2005, 2006 Michael Buesch <m@bues.ch>

  Some code in this file is derived from the b44.c driver
  Copyright (C) 2002 David S. Miller
  Copyright (C) Pekka Pietikainen

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; see the file COPYING.  If not, write to
  the Free Software Foundation, Inc., 51 Franklin Steet, Fifth Floor,
  Boston, MA 02110-1301, USA.

*/

#include "b43.h"
#include "dma.h"
#include "main.h"
#include "debugfs.h"
#include "xmit.h"

#include <linux/dma-mapping.h>
#include <linux/pci.h>
#include <linux/delay.h>
#include <linux/skbuff.h>
#include <linux/etherdevice.h>
#include <linux/slab.h>
#include <asm/div64.h>


/*                                              
                                                                         
                        */
#define TX_SLOTS_PER_FRAME	2

static u32 b43_dma_address(struct b43_dma *dma, dma_addr_t dmaaddr,
			   enum b43_addrtype addrtype)
{
	u32 uninitialized_var(addr);

	switch (addrtype) {
	case B43_DMA_ADDR_LOW:
		addr = lower_32_bits(dmaaddr);
		if (dma->translation_in_low) {
			addr &= ~SSB_DMA_TRANSLATION_MASK;
			addr |= dma->translation;
		}
		break;
	case B43_DMA_ADDR_HIGH:
		addr = upper_32_bits(dmaaddr);
		if (!dma->translation_in_low) {
			addr &= ~SSB_DMA_TRANSLATION_MASK;
			addr |= dma->translation;
		}
		break;
	case B43_DMA_ADDR_EXT:
		if (dma->translation_in_low)
			addr = lower_32_bits(dmaaddr);
		else
			addr = upper_32_bits(dmaaddr);
		addr &= SSB_DMA_TRANSLATION_MASK;
		addr >>= SSB_DMA_TRANSLATION_SHIFT;
		break;
	}

	return addr;
}

/*                */
static
struct b43_dmadesc_generic *op32_idx2desc(struct b43_dmaring *ring,
					  int slot,
					  struct b43_dmadesc_meta **meta)
{
	struct b43_dmadesc32 *desc;

	*meta = &(ring->meta[slot]);
	desc = ring->descbase;
	desc = &(desc[slot]);

	return (struct b43_dmadesc_generic *)desc;
}

static void op32_fill_descriptor(struct b43_dmaring *ring,
				 struct b43_dmadesc_generic *desc,
				 dma_addr_t dmaaddr, u16 bufsize,
				 int start, int end, int irq)
{
	struct b43_dmadesc32 *descbase = ring->descbase;
	int slot;
	u32 ctl;
	u32 addr;
	u32 addrext;

	slot = (int)(&(desc->dma32) - descbase);
	B43_WARN_ON(!(slot >= 0 && slot < ring->nr_slots));

	addr = b43_dma_address(&ring->dev->dma, dmaaddr, B43_DMA_ADDR_LOW);
	addrext = b43_dma_address(&ring->dev->dma, dmaaddr, B43_DMA_ADDR_EXT);

	ctl = bufsize & B43_DMA32_DCTL_BYTECNT;
	if (slot == ring->nr_slots - 1)
		ctl |= B43_DMA32_DCTL_DTABLEEND;
	if (start)
		ctl |= B43_DMA32_DCTL_FRAMESTART;
	if (end)
		ctl |= B43_DMA32_DCTL_FRAMEEND;
	if (irq)
		ctl |= B43_DMA32_DCTL_IRQ;
	ctl |= (addrext << B43_DMA32_DCTL_ADDREXT_SHIFT)
	    & B43_DMA32_DCTL_ADDREXT_MASK;

	desc->dma32.control = cpu_to_le32(ctl);
	desc->dma32.address = cpu_to_le32(addr);
}

static void op32_poke_tx(struct b43_dmaring *ring, int slot)
{
	b43_dma_write(ring, B43_DMA32_TXINDEX,
		      (u32) (slot * sizeof(struct b43_dmadesc32)));
}

static void op32_tx_suspend(struct b43_dmaring *ring)
{
	b43_dma_write(ring, B43_DMA32_TXCTL, b43_dma_read(ring, B43_DMA32_TXCTL)
		      | B43_DMA32_TXSUSPEND);
}

static void op32_tx_resume(struct b43_dmaring *ring)
{
	b43_dma_write(ring, B43_DMA32_TXCTL, b43_dma_read(ring, B43_DMA32_TXCTL)
		      & ~B43_DMA32_TXSUSPEND);
}

static int op32_get_current_rxslot(struct b43_dmaring *ring)
{
	u32 val;

	val = b43_dma_read(ring, B43_DMA32_RXSTATUS);
	val &= B43_DMA32_RXDPTR;

	return (val / sizeof(struct b43_dmadesc32));
}

static void op32_set_current_rxslot(struct b43_dmaring *ring, int slot)
{
	b43_dma_write(ring, B43_DMA32_RXINDEX,
		      (u32) (slot * sizeof(struct b43_dmadesc32)));
}

static const struct b43_dma_ops dma32_ops = {
	.idx2desc = op32_idx2desc,
	.fill_descriptor = op32_fill_descriptor,
	.poke_tx = op32_poke_tx,
	.tx_suspend = op32_tx_suspend,
	.tx_resume = op32_tx_resume,
	.get_current_rxslot = op32_get_current_rxslot,
	.set_current_rxslot = op32_set_current_rxslot,
};

/*                */
static
struct b43_dmadesc_generic *op64_idx2desc(struct b43_dmaring *ring,
					  int slot,
					  struct b43_dmadesc_meta **meta)
{
	struct b43_dmadesc64 *desc;

	*meta = &(ring->meta[slot]);
	desc = ring->descbase;
	desc = &(desc[slot]);

	return (struct b43_dmadesc_generic *)desc;
}

static void op64_fill_descriptor(struct b43_dmaring *ring,
				 struct b43_dmadesc_generic *desc,
				 dma_addr_t dmaaddr, u16 bufsize,
				 int start, int end, int irq)
{
	struct b43_dmadesc64 *descbase = ring->descbase;
	int slot;
	u32 ctl0 = 0, ctl1 = 0;
	u32 addrlo, addrhi;
	u32 addrext;

	slot = (int)(&(desc->dma64) - descbase);
	B43_WARN_ON(!(slot >= 0 && slot < ring->nr_slots));

	addrlo = b43_dma_address(&ring->dev->dma, dmaaddr, B43_DMA_ADDR_LOW);
	addrhi = b43_dma_address(&ring->dev->dma, dmaaddr, B43_DMA_ADDR_HIGH);
	addrext = b43_dma_address(&ring->dev->dma, dmaaddr, B43_DMA_ADDR_EXT);

	if (slot == ring->nr_slots - 1)
		ctl0 |= B43_DMA64_DCTL0_DTABLEEND;
	if (start)
		ctl0 |= B43_DMA64_DCTL0_FRAMESTART;
	if (end)
		ctl0 |= B43_DMA64_DCTL0_FRAMEEND;
	if (irq)
		ctl0 |= B43_DMA64_DCTL0_IRQ;
	ctl1 |= bufsize & B43_DMA64_DCTL1_BYTECNT;
	ctl1 |= (addrext << B43_DMA64_DCTL1_ADDREXT_SHIFT)
	    & B43_DMA64_DCTL1_ADDREXT_MASK;

	desc->dma64.control0 = cpu_to_le32(ctl0);
	desc->dma64.control1 = cpu_to_le32(ctl1);
	desc->dma64.address_low = cpu_to_le32(addrlo);
	desc->dma64.address_high = cpu_to_le32(addrhi);
}

static void op64_poke_tx(struct b43_dmaring *ring, int slot)
{
	b43_dma_write(ring, B43_DMA64_TXINDEX,
		      (u32) (slot * sizeof(struct b43_dmadesc64)));
}

static void op64_tx_suspend(struct b43_dmaring *ring)
{
	b43_dma_write(ring, B43_DMA64_TXCTL, b43_dma_read(ring, B43_DMA64_TXCTL)
		      | B43_DMA64_TXSUSPEND);
}

static void op64_tx_resume(struct b43_dmaring *ring)
{
	b43_dma_write(ring, B43_DMA64_TXCTL, b43_dma_read(ring, B43_DMA64_TXCTL)
		      & ~B43_DMA64_TXSUSPEND);
}

static int op64_get_current_rxslot(struct b43_dmaring *ring)
{
	u32 val;

	val = b43_dma_read(ring, B43_DMA64_RXSTATUS);
	val &= B43_DMA64_RXSTATDPTR;

	return (val / sizeof(struct b43_dmadesc64));
}

static void op64_set_current_rxslot(struct b43_dmaring *ring, int slot)
{
	b43_dma_write(ring, B43_DMA64_RXINDEX,
		      (u32) (slot * sizeof(struct b43_dmadesc64)));
}

static const struct b43_dma_ops dma64_ops = {
	.idx2desc = op64_idx2desc,
	.fill_descriptor = op64_fill_descriptor,
	.poke_tx = op64_poke_tx,
	.tx_suspend = op64_tx_suspend,
	.tx_resume = op64_tx_resume,
	.get_current_rxslot = op64_get_current_rxslot,
	.set_current_rxslot = op64_set_current_rxslot,
};

static inline int free_slots(struct b43_dmaring *ring)
{
	return (ring->nr_slots - ring->used_slots);
}

static inline int next_slot(struct b43_dmaring *ring, int slot)
{
	B43_WARN_ON(!(slot >= -1 && slot <= ring->nr_slots - 1));
	if (slot == ring->nr_slots - 1)
		return 0;
	return slot + 1;
}

static inline int prev_slot(struct b43_dmaring *ring, int slot)
{
	B43_WARN_ON(!(slot >= 0 && slot <= ring->nr_slots - 1));
	if (slot == 0)
		return ring->nr_slots - 1;
	return slot - 1;
}

#ifdef CONFIG_B43_DEBUG
static void update_max_used_slots(struct b43_dmaring *ring,
				  int current_used_slots)
{
	if (current_used_slots <= ring->max_used_slots)
		return;
	ring->max_used_slots = current_used_slots;
	if (b43_debug(ring->dev, B43_DBG_DMAVERBOSE)) {
		b43dbg(ring->dev->wl,
		       "max_used_slots increased to %d on %s ring %d\n",
		       ring->max_used_slots,
		       ring->tx ? "TX" : "RX", ring->index);
	}
}
#else
static inline
    void update_max_used_slots(struct b43_dmaring *ring, int current_used_slots)
{
}
#endif /*       */

/*                           */
static inline int request_slot(struct b43_dmaring *ring)
{
	int slot;

	B43_WARN_ON(!ring->tx);
	B43_WARN_ON(ring->stopped);
	B43_WARN_ON(free_slots(ring) == 0);

	slot = next_slot(ring, ring->current_slot);
	ring->current_slot = slot;
	ring->used_slots++;

	update_max_used_slots(ring, ring->used_slots);

	return slot;
}

static u16 b43_dmacontroller_base(enum b43_dmatype type, int controller_idx)
{
	static const u16 map64[] = {
		B43_MMIO_DMA64_BASE0,
		B43_MMIO_DMA64_BASE1,
		B43_MMIO_DMA64_BASE2,
		B43_MMIO_DMA64_BASE3,
		B43_MMIO_DMA64_BASE4,
		B43_MMIO_DMA64_BASE5,
	};
	static const u16 map32[] = {
		B43_MMIO_DMA32_BASE0,
		B43_MMIO_DMA32_BASE1,
		B43_MMIO_DMA32_BASE2,
		B43_MMIO_DMA32_BASE3,
		B43_MMIO_DMA32_BASE4,
		B43_MMIO_DMA32_BASE5,
	};

	if (type == B43_DMA_64BIT) {
		B43_WARN_ON(!(controller_idx >= 0 &&
			      controller_idx < ARRAY_SIZE(map64)));
		return map64[controller_idx];
	}
	B43_WARN_ON(!(controller_idx >= 0 &&
		      controller_idx < ARRAY_SIZE(map32)));
	return map32[controller_idx];
}

static inline
    dma_addr_t map_descbuffer(struct b43_dmaring *ring,
			      unsigned char *buf, size_t len, int tx)
{
	dma_addr_t dmaaddr;

	if (tx) {
		dmaaddr = dma_map_single(ring->dev->dev->dma_dev,
					 buf, len, DMA_TO_DEVICE);
	} else {
		dmaaddr = dma_map_single(ring->dev->dev->dma_dev,
					 buf, len, DMA_FROM_DEVICE);
	}

	return dmaaddr;
}

static inline
    void unmap_descbuffer(struct b43_dmaring *ring,
			  dma_addr_t addr, size_t len, int tx)
{
	if (tx) {
		dma_unmap_single(ring->dev->dev->dma_dev,
				 addr, len, DMA_TO_DEVICE);
	} else {
		dma_unmap_single(ring->dev->dev->dma_dev,
				 addr, len, DMA_FROM_DEVICE);
	}
}

static inline
    void sync_descbuffer_for_cpu(struct b43_dmaring *ring,
				 dma_addr_t addr, size_t len)
{
	B43_WARN_ON(ring->tx);
	dma_sync_single_for_cpu(ring->dev->dev->dma_dev,
				    addr, len, DMA_FROM_DEVICE);
}

static inline
    void sync_descbuffer_for_device(struct b43_dmaring *ring,
				    dma_addr_t addr, size_t len)
{
	B43_WARN_ON(ring->tx);
	dma_sync_single_for_device(ring->dev->dev->dma_dev,
				   addr, len, DMA_FROM_DEVICE);
}

static inline
    void free_descriptor_buffer(struct b43_dmaring *ring,
				struct b43_dmadesc_meta *meta)
{
	if (meta->skb) {
		dev_kfree_skb_any(meta->skb);
		meta->skb = NULL;
	}
}

static int alloc_ringmemory(struct b43_dmaring *ring)
{
	gfp_t flags = GFP_KERNEL;

	/*                                                             
                                                              
                                                                    
                                                                     
                                                                   
                                                                        
                                                                        
                                 
  */
	u16 ring_mem_size = (ring->type == B43_DMA_64BIT) ?
				B43_DMA64_RINGMEMSIZE : B43_DMA32_RINGMEMSIZE;

	ring->descbase = dma_alloc_coherent(ring->dev->dev->dma_dev,
					    ring_mem_size, &(ring->dmabase),
					    flags);
	if (!ring->descbase) {
		b43err(ring->dev->wl, "DMA ringmemory allocation failed\n");
		return -ENOMEM;
	}
	memset(ring->descbase, 0, ring_mem_size);

	return 0;
}

static void free_ringmemory(struct b43_dmaring *ring)
{
	u16 ring_mem_size = (ring->type == B43_DMA_64BIT) ?
				B43_DMA64_RINGMEMSIZE : B43_DMA32_RINGMEMSIZE;
	dma_free_coherent(ring->dev->dev->dma_dev, ring_mem_size,
			  ring->descbase, ring->dmabase);
}

/*                          */
static int b43_dmacontroller_rx_reset(struct b43_wldev *dev, u16 mmio_base,
				      enum b43_dmatype type)
{
	int i;
	u32 value;
	u16 offset;

	might_sleep();

	offset = (type == B43_DMA_64BIT) ? B43_DMA64_RXCTL : B43_DMA32_RXCTL;
	b43_write32(dev, mmio_base + offset, 0);
	for (i = 0; i < 10; i++) {
		offset = (type == B43_DMA_64BIT) ? B43_DMA64_RXSTATUS :
						   B43_DMA32_RXSTATUS;
		value = b43_read32(dev, mmio_base + offset);
		if (type == B43_DMA_64BIT) {
			value &= B43_DMA64_RXSTAT;
			if (value == B43_DMA64_RXSTAT_DISABLED) {
				i = -1;
				break;
			}
		} else {
			value &= B43_DMA32_RXSTATE;
			if (value == B43_DMA32_RXSTAT_DISABLED) {
				i = -1;
				break;
			}
		}
		msleep(1);
	}
	if (i != -1) {
		b43err(dev->wl, "DMA RX reset timed out\n");
		return -ENODEV;
	}

	return 0;
}

/*                          */
static int b43_dmacontroller_tx_reset(struct b43_wldev *dev, u16 mmio_base,
				      enum b43_dmatype type)
{
	int i;
	u32 value;
	u16 offset;

	might_sleep();

	for (i = 0; i < 10; i++) {
		offset = (type == B43_DMA_64BIT) ? B43_DMA64_TXSTATUS :
						   B43_DMA32_TXSTATUS;
		value = b43_read32(dev, mmio_base + offset);
		if (type == B43_DMA_64BIT) {
			value &= B43_DMA64_TXSTAT;
			if (value == B43_DMA64_TXSTAT_DISABLED ||
			    value == B43_DMA64_TXSTAT_IDLEWAIT ||
			    value == B43_DMA64_TXSTAT_STOPPED)
				break;
		} else {
			value &= B43_DMA32_TXSTATE;
			if (value == B43_DMA32_TXSTAT_DISABLED ||
			    value == B43_DMA32_TXSTAT_IDLEWAIT ||
			    value == B43_DMA32_TXSTAT_STOPPED)
				break;
		}
		msleep(1);
	}
	offset = (type == B43_DMA_64BIT) ? B43_DMA64_TXCTL : B43_DMA32_TXCTL;
	b43_write32(dev, mmio_base + offset, 0);
	for (i = 0; i < 10; i++) {
		offset = (type == B43_DMA_64BIT) ? B43_DMA64_TXSTATUS :
						   B43_DMA32_TXSTATUS;
		value = b43_read32(dev, mmio_base + offset);
		if (type == B43_DMA_64BIT) {
			value &= B43_DMA64_TXSTAT;
			if (value == B43_DMA64_TXSTAT_DISABLED) {
				i = -1;
				break;
			}
		} else {
			value &= B43_DMA32_TXSTATE;
			if (value == B43_DMA32_TXSTAT_DISABLED) {
				i = -1;
				break;
			}
		}
		msleep(1);
	}
	if (i != -1) {
		b43err(dev->wl, "DMA TX reset timed out\n");
		return -ENODEV;
	}
	/*                                */
	msleep(1);

	return 0;
}

/*                                            */
static bool b43_dma_mapping_error(struct b43_dmaring *ring,
				  dma_addr_t addr,
				  size_t buffersize, bool dma_to_device)
{
	if (unlikely(dma_mapping_error(ring->dev->dev->dma_dev, addr)))
		return 1;

	switch (ring->type) {
	case B43_DMA_30BIT:
		if ((u64)addr + buffersize > (1ULL << 30))
			goto address_error;
		break;
	case B43_DMA_32BIT:
		if ((u64)addr + buffersize > (1ULL << 32))
			goto address_error;
		break;
	case B43_DMA_64BIT:
		/*                                         
                          */
		break;
	}

	/*                    */
	return 0;

address_error:
	/*                                                */
	unmap_descbuffer(ring, addr, buffersize, dma_to_device);

	return 1;
}

static bool b43_rx_buffer_is_poisoned(struct b43_dmaring *ring, struct sk_buff *skb)
{
	unsigned char *f = skb->data + ring->frameoffset;

	return ((f[0] & f[1] & f[2] & f[3] & f[4] & f[5] & f[6] & f[7]) == 0xFF);
}

static void b43_poison_rx_buffer(struct b43_dmaring *ring, struct sk_buff *skb)
{
	struct b43_rxhdr_fw4 *rxhdr;
	unsigned char *frame;

	/*                                                    */

	rxhdr = (struct b43_rxhdr_fw4 *)(skb->data);
	rxhdr->frame_len = 0;

	B43_WARN_ON(ring->rx_buffersize < ring->frameoffset + sizeof(struct b43_plcp_hdr6) + 2);
	frame = skb->data + ring->frameoffset;
	memset(frame, 0xFF, sizeof(struct b43_plcp_hdr6) + 2 /*         */);
}

static int setup_rx_descbuffer(struct b43_dmaring *ring,
			       struct b43_dmadesc_generic *desc,
			       struct b43_dmadesc_meta *meta, gfp_t gfp_flags)
{
	dma_addr_t dmaaddr;
	struct sk_buff *skb;

	B43_WARN_ON(ring->tx);

	skb = __dev_alloc_skb(ring->rx_buffersize, gfp_flags);
	if (unlikely(!skb))
		return -ENOMEM;
	b43_poison_rx_buffer(ring, skb);
	dmaaddr = map_descbuffer(ring, skb->data, ring->rx_buffersize, 0);
	if (b43_dma_mapping_error(ring, dmaaddr, ring->rx_buffersize, 0)) {
		/*                                 */
		gfp_flags |= GFP_DMA;

		dev_kfree_skb_any(skb);

		skb = __dev_alloc_skb(ring->rx_buffersize, gfp_flags);
		if (unlikely(!skb))
			return -ENOMEM;
		b43_poison_rx_buffer(ring, skb);
		dmaaddr = map_descbuffer(ring, skb->data,
					 ring->rx_buffersize, 0);
		if (b43_dma_mapping_error(ring, dmaaddr, ring->rx_buffersize, 0)) {
			b43err(ring->dev->wl, "RX DMA buffer allocation failed\n");
			dev_kfree_skb_any(skb);
			return -EIO;
		}
	}

	meta->skb = skb;
	meta->dmaaddr = dmaaddr;
	ring->ops->fill_descriptor(ring, desc, dmaaddr,
				   ring->rx_buffersize, 0, 0, 0);

	return 0;
}

/*                                  
                                    
 */
static int alloc_initial_descbuffers(struct b43_dmaring *ring)
{
	int i, err = -ENOMEM;
	struct b43_dmadesc_generic *desc;
	struct b43_dmadesc_meta *meta;

	for (i = 0; i < ring->nr_slots; i++) {
		desc = ring->ops->idx2desc(ring, i, &meta);

		err = setup_rx_descbuffer(ring, desc, meta, GFP_KERNEL);
		if (err) {
			b43err(ring->dev->wl,
			       "Failed to allocate initial descbuffers\n");
			goto err_unwind;
		}
	}
	mb();
	ring->used_slots = ring->nr_slots;
	err = 0;
      out:
	return err;

      err_unwind:
	for (i--; i >= 0; i--) {
		desc = ring->ops->idx2desc(ring, i, &meta);

		unmap_descbuffer(ring, meta->dmaaddr, ring->rx_buffersize, 0);
		dev_kfree_skb(meta->skb);
	}
	goto out;
}

/*                                        
                                                  
                                  
 */
static int dmacontroller_setup(struct b43_dmaring *ring)
{
	int err = 0;
	u32 value;
	u32 addrext;
	bool parity = ring->dev->dma.parity;
	u32 addrlo;
	u32 addrhi;

	if (ring->tx) {
		if (ring->type == B43_DMA_64BIT) {
			u64 ringbase = (u64) (ring->dmabase);
			addrext = b43_dma_address(&ring->dev->dma, ringbase, B43_DMA_ADDR_EXT);
			addrlo = b43_dma_address(&ring->dev->dma, ringbase, B43_DMA_ADDR_LOW);
			addrhi = b43_dma_address(&ring->dev->dma, ringbase, B43_DMA_ADDR_HIGH);

			value = B43_DMA64_TXENABLE;
			value |= (addrext << B43_DMA64_TXADDREXT_SHIFT)
			    & B43_DMA64_TXADDREXT_MASK;
			if (!parity)
				value |= B43_DMA64_TXPARITYDISABLE;
			b43_dma_write(ring, B43_DMA64_TXCTL, value);
			b43_dma_write(ring, B43_DMA64_TXRINGLO, addrlo);
			b43_dma_write(ring, B43_DMA64_TXRINGHI, addrhi);
		} else {
			u32 ringbase = (u32) (ring->dmabase);
			addrext = b43_dma_address(&ring->dev->dma, ringbase, B43_DMA_ADDR_EXT);
			addrlo = b43_dma_address(&ring->dev->dma, ringbase, B43_DMA_ADDR_LOW);

			value = B43_DMA32_TXENABLE;
			value |= (addrext << B43_DMA32_TXADDREXT_SHIFT)
			    & B43_DMA32_TXADDREXT_MASK;
			if (!parity)
				value |= B43_DMA32_TXPARITYDISABLE;
			b43_dma_write(ring, B43_DMA32_TXCTL, value);
			b43_dma_write(ring, B43_DMA32_TXRING, addrlo);
		}
	} else {
		err = alloc_initial_descbuffers(ring);
		if (err)
			goto out;
		if (ring->type == B43_DMA_64BIT) {
			u64 ringbase = (u64) (ring->dmabase);
			addrext = b43_dma_address(&ring->dev->dma, ringbase, B43_DMA_ADDR_EXT);
			addrlo = b43_dma_address(&ring->dev->dma, ringbase, B43_DMA_ADDR_LOW);
			addrhi = b43_dma_address(&ring->dev->dma, ringbase, B43_DMA_ADDR_HIGH);

			value = (ring->frameoffset << B43_DMA64_RXFROFF_SHIFT);
			value |= B43_DMA64_RXENABLE;
			value |= (addrext << B43_DMA64_RXADDREXT_SHIFT)
			    & B43_DMA64_RXADDREXT_MASK;
			if (!parity)
				value |= B43_DMA64_RXPARITYDISABLE;
			b43_dma_write(ring, B43_DMA64_RXCTL, value);
			b43_dma_write(ring, B43_DMA64_RXRINGLO, addrlo);
			b43_dma_write(ring, B43_DMA64_RXRINGHI, addrhi);
			b43_dma_write(ring, B43_DMA64_RXINDEX, ring->nr_slots *
				      sizeof(struct b43_dmadesc64));
		} else {
			u32 ringbase = (u32) (ring->dmabase);
			addrext = b43_dma_address(&ring->dev->dma, ringbase, B43_DMA_ADDR_EXT);
			addrlo = b43_dma_address(&ring->dev->dma, ringbase, B43_DMA_ADDR_LOW);

			value = (ring->frameoffset << B43_DMA32_RXFROFF_SHIFT);
			value |= B43_DMA32_RXENABLE;
			value |= (addrext << B43_DMA32_RXADDREXT_SHIFT)
			    & B43_DMA32_RXADDREXT_MASK;
			if (!parity)
				value |= B43_DMA32_RXPARITYDISABLE;
			b43_dma_write(ring, B43_DMA32_RXCTL, value);
			b43_dma_write(ring, B43_DMA32_RXRING, addrlo);
			b43_dma_write(ring, B43_DMA32_RXINDEX, ring->nr_slots *
				      sizeof(struct b43_dmadesc32));
		}
	}

out:
	return err;
}

/*                              */
static void dmacontroller_cleanup(struct b43_dmaring *ring)
{
	if (ring->tx) {
		b43_dmacontroller_tx_reset(ring->dev, ring->mmio_base,
					   ring->type);
		if (ring->type == B43_DMA_64BIT) {
			b43_dma_write(ring, B43_DMA64_TXRINGLO, 0);
			b43_dma_write(ring, B43_DMA64_TXRINGHI, 0);
		} else
			b43_dma_write(ring, B43_DMA32_TXRING, 0);
	} else {
		b43_dmacontroller_rx_reset(ring->dev, ring->mmio_base,
					   ring->type);
		if (ring->type == B43_DMA_64BIT) {
			b43_dma_write(ring, B43_DMA64_RXRINGLO, 0);
			b43_dma_write(ring, B43_DMA64_RXRINGHI, 0);
		} else
			b43_dma_write(ring, B43_DMA32_RXRING, 0);
	}
}

static void free_all_descbuffers(struct b43_dmaring *ring)
{
	struct b43_dmadesc_meta *meta;
	int i;

	if (!ring->used_slots)
		return;
	for (i = 0; i < ring->nr_slots; i++) {
		/*                                  */
		ring->ops->idx2desc(ring, i, &meta);

		if (!meta->skb || b43_dma_ptr_is_poisoned(meta->skb)) {
			B43_WARN_ON(!ring->tx);
			continue;
		}
		if (ring->tx) {
			unmap_descbuffer(ring, meta->dmaaddr,
					 meta->skb->len, 1);
		} else {
			unmap_descbuffer(ring, meta->dmaaddr,
					 ring->rx_buffersize, 0);
		}
		free_descriptor_buffer(ring, meta);
	}
}

static u64 supported_dma_mask(struct b43_wldev *dev)
{
	u32 tmp;
	u16 mmio_base;

	switch (dev->dev->bus_type) {
#ifdef CONFIG_B43_BCMA
	case B43_BUS_BCMA:
		tmp = bcma_aread32(dev->dev->bdev, BCMA_IOST);
		if (tmp & BCMA_IOST_DMA64)
			return DMA_BIT_MASK(64);
		break;
#endif
#ifdef CONFIG_B43_SSB
	case B43_BUS_SSB:
		tmp = ssb_read32(dev->dev->sdev, SSB_TMSHIGH);
		if (tmp & SSB_TMSHIGH_DMA64)
			return DMA_BIT_MASK(64);
		break;
#endif
	}

	mmio_base = b43_dmacontroller_base(0, 0);
	b43_write32(dev, mmio_base + B43_DMA32_TXCTL, B43_DMA32_TXADDREXT_MASK);
	tmp = b43_read32(dev, mmio_base + B43_DMA32_TXCTL);
	if (tmp & B43_DMA32_TXADDREXT_MASK)
		return DMA_BIT_MASK(32);

	return DMA_BIT_MASK(30);
}

static enum b43_dmatype dma_mask_to_engine_type(u64 dmamask)
{
	if (dmamask == DMA_BIT_MASK(30))
		return B43_DMA_30BIT;
	if (dmamask == DMA_BIT_MASK(32))
		return B43_DMA_32BIT;
	if (dmamask == DMA_BIT_MASK(64))
		return B43_DMA_64BIT;
	B43_WARN_ON(1);
	return B43_DMA_30BIT;
}

/*                               */
static
struct b43_dmaring *b43_setup_dmaring(struct b43_wldev *dev,
				      int controller_index,
				      int for_tx,
				      enum b43_dmatype type)
{
	struct b43_dmaring *ring;
	int i, err;
	dma_addr_t dma_test;

	ring = kzalloc(sizeof(*ring), GFP_KERNEL);
	if (!ring)
		goto out;

	ring->nr_slots = B43_RXRING_SLOTS;
	if (for_tx)
		ring->nr_slots = B43_TXRING_SLOTS;

	ring->meta = kcalloc(ring->nr_slots, sizeof(struct b43_dmadesc_meta),
			     GFP_KERNEL);
	if (!ring->meta)
		goto err_kfree_ring;
	for (i = 0; i < ring->nr_slots; i++)
		ring->meta->skb = B43_DMA_PTR_POISON;

	ring->type = type;
	ring->dev = dev;
	ring->mmio_base = b43_dmacontroller_base(type, controller_index);
	ring->index = controller_index;
	if (type == B43_DMA_64BIT)
		ring->ops = &dma64_ops;
	else
		ring->ops = &dma32_ops;
	if (for_tx) {
		ring->tx = true;
		ring->current_slot = -1;
	} else {
		if (ring->index == 0) {
			switch (dev->fw.hdr_format) {
			case B43_FW_HDR_598:
				ring->rx_buffersize = B43_DMA0_RX_FW598_BUFSIZE;
				ring->frameoffset = B43_DMA0_RX_FW598_FO;
				break;
			case B43_FW_HDR_410:
			case B43_FW_HDR_351:
				ring->rx_buffersize = B43_DMA0_RX_FW351_BUFSIZE;
				ring->frameoffset = B43_DMA0_RX_FW351_FO;
				break;
			}
		} else
			B43_WARN_ON(1);
	}
#ifdef CONFIG_B43_DEBUG
	ring->last_injected_overflow = jiffies;
#endif

	if (for_tx) {
		/*                                                                   */
		BUILD_BUG_ON(B43_TXRING_SLOTS % TX_SLOTS_PER_FRAME != 0);

		ring->txhdr_cache = kcalloc(ring->nr_slots / TX_SLOTS_PER_FRAME,
					    b43_txhdr_size(dev),
					    GFP_KERNEL);
		if (!ring->txhdr_cache)
			goto err_kfree_meta;

		/*                                        */
		dma_test = dma_map_single(dev->dev->dma_dev,
					  ring->txhdr_cache,
					  b43_txhdr_size(dev),
					  DMA_TO_DEVICE);

		if (b43_dma_mapping_error(ring, dma_test,
					  b43_txhdr_size(dev), 1)) {
			/*             */
			kfree(ring->txhdr_cache);
			ring->txhdr_cache = kcalloc(ring->nr_slots / TX_SLOTS_PER_FRAME,
						    b43_txhdr_size(dev),
						    GFP_KERNEL | GFP_DMA);
			if (!ring->txhdr_cache)
				goto err_kfree_meta;

			dma_test = dma_map_single(dev->dev->dma_dev,
						  ring->txhdr_cache,
						  b43_txhdr_size(dev),
						  DMA_TO_DEVICE);

			if (b43_dma_mapping_error(ring, dma_test,
						  b43_txhdr_size(dev), 1)) {

				b43err(dev->wl,
				       "TXHDR DMA allocation failed\n");
				goto err_kfree_txhdr_cache;
			}
		}

		dma_unmap_single(dev->dev->dma_dev,
				 dma_test, b43_txhdr_size(dev),
				 DMA_TO_DEVICE);
	}

	err = alloc_ringmemory(ring);
	if (err)
		goto err_kfree_txhdr_cache;
	err = dmacontroller_setup(ring);
	if (err)
		goto err_free_ringmemory;

      out:
	return ring;

      err_free_ringmemory:
	free_ringmemory(ring);
      err_kfree_txhdr_cache:
	kfree(ring->txhdr_cache);
      err_kfree_meta:
	kfree(ring->meta);
      err_kfree_ring:
	kfree(ring);
	ring = NULL;
	goto out;
}

#define divide(a, b)	({	\
	typeof(a) __a = a;	\
	do_div(__a, b);		\
	__a;			\
  })

#define modulo(a, b)	({	\
	typeof(a) __a = a;	\
	do_div(__a, b);		\
  })

/*                        */
static void b43_destroy_dmaring(struct b43_dmaring *ring,
				const char *ringname)
{
	if (!ring)
		return;

#ifdef CONFIG_B43_DEBUG
	{
		/*                        */
		u64 failed_packets = ring->nr_failed_tx_packets;
		u64 succeed_packets = ring->nr_succeed_tx_packets;
		u64 nr_packets = failed_packets + succeed_packets;
		u64 permille_failed = 0, average_tries = 0;

		if (nr_packets)
			permille_failed = divide(failed_packets * 1000, nr_packets);
		if (nr_packets)
			average_tries = divide(ring->nr_total_packet_tries * 100, nr_packets);

		b43dbg(ring->dev->wl, "DMA-%u %s: "
		       "Used slots %d/%d, Failed frames %llu/%llu = %llu.%01llu%%, "
		       "Average tries %llu.%02llu\n",
		       (unsigned int)(ring->type), ringname,
		       ring->max_used_slots,
		       ring->nr_slots,
		       (unsigned long long)failed_packets,
		       (unsigned long long)nr_packets,
		       (unsigned long long)divide(permille_failed, 10),
		       (unsigned long long)modulo(permille_failed, 10),
		       (unsigned long long)divide(average_tries, 100),
		       (unsigned long long)modulo(average_tries, 100));
	}
#endif /*       */

	/*                                                       
                                                                 
  */
	dmacontroller_cleanup(ring);
	free_all_descbuffers(ring);
	free_ringmemory(ring);

	kfree(ring->txhdr_cache);
	kfree(ring->meta);
	kfree(ring);
}

#define destroy_ring(dma, ring) do {				\
	b43_destroy_dmaring((dma)->ring, __stringify(ring));	\
	(dma)->ring = NULL;					\
    } while (0)

void b43_dma_free(struct b43_wldev *dev)
{
	struct b43_dma *dma;

	if (b43_using_pio_transfers(dev))
		return;
	dma = &dev->dma;

	destroy_ring(dma, rx_ring);
	destroy_ring(dma, tx_ring_AC_BK);
	destroy_ring(dma, tx_ring_AC_BE);
	destroy_ring(dma, tx_ring_AC_VI);
	destroy_ring(dma, tx_ring_AC_VO);
	destroy_ring(dma, tx_ring_mcast);
}

static int b43_dma_set_mask(struct b43_wldev *dev, u64 mask)
{
	u64 orig_mask = mask;
	bool fallback = false;
	int err;

	/*                                                            
                                                           */
	while (1) {
		err = dma_set_mask(dev->dev->dma_dev, mask);
		if (!err) {
			err = dma_set_coherent_mask(dev->dev->dma_dev, mask);
			if (!err)
				break;
		}
		if (mask == DMA_BIT_MASK(64)) {
			mask = DMA_BIT_MASK(32);
			fallback = true;
			continue;
		}
		if (mask == DMA_BIT_MASK(32)) {
			mask = DMA_BIT_MASK(30);
			fallback = true;
			continue;
		}
		b43err(dev->wl, "The machine/kernel does not support "
		       "the required %u-bit DMA mask\n",
		       (unsigned int)dma_mask_to_engine_type(orig_mask));
		return -EOPNOTSUPP;
	}
	if (fallback) {
		b43info(dev->wl, "DMA mask fallback from %u-bit to %u-bit\n",
			(unsigned int)dma_mask_to_engine_type(orig_mask),
			(unsigned int)dma_mask_to_engine_type(mask));
	}

	return 0;
}

/*                                                                           
                                               
 */
static bool b43_dma_translation_in_low_word(struct b43_wldev *dev,
					    enum b43_dmatype type)
{
	if (type != B43_DMA_64BIT)
		return 1;

#ifdef CONFIG_B43_SSB
	if (dev->dev->bus_type == B43_BUS_SSB &&
	    dev->dev->sdev->bus->bustype == SSB_BUSTYPE_PCI &&
	    !(dev->dev->sdev->bus->host_pci->is_pcie &&
	      ssb_read32(dev->dev->sdev, SSB_TMSHIGH) & SSB_TMSHIGH_DMA64))
			return 1;
#endif
	return 0;
}

int b43_dma_init(struct b43_wldev *dev)
{
	struct b43_dma *dma = &dev->dma;
	int err;
	u64 dmamask;
	enum b43_dmatype type;

	dmamask = supported_dma_mask(dev);
	type = dma_mask_to_engine_type(dmamask);
	err = b43_dma_set_mask(dev, dmamask);
	if (err)
		return err;

	switch (dev->dev->bus_type) {
#ifdef CONFIG_B43_BCMA
	case B43_BUS_BCMA:
		dma->translation = bcma_core_dma_translation(dev->dev->bdev);
		break;
#endif
#ifdef CONFIG_B43_SSB
	case B43_BUS_SSB:
		dma->translation = ssb_dma_translation(dev->dev->sdev);
		break;
#endif
	}
	dma->translation_in_low = b43_dma_translation_in_low_word(dev, type);

	dma->parity = true;
#ifdef CONFIG_B43_BCMA
	/*                                                        */
	if (dev->dev->bus_type == B43_BUS_BCMA)
		dma->parity = false;
#endif

	err = -ENOMEM;
	/*                        */
	dma->tx_ring_AC_BK = b43_setup_dmaring(dev, 0, 1, type);
	if (!dma->tx_ring_AC_BK)
		goto out;

	dma->tx_ring_AC_BE = b43_setup_dmaring(dev, 1, 1, type);
	if (!dma->tx_ring_AC_BE)
		goto err_destroy_bk;

	dma->tx_ring_AC_VI = b43_setup_dmaring(dev, 2, 1, type);
	if (!dma->tx_ring_AC_VI)
		goto err_destroy_be;

	dma->tx_ring_AC_VO = b43_setup_dmaring(dev, 3, 1, type);
	if (!dma->tx_ring_AC_VO)
		goto err_destroy_vi;

	dma->tx_ring_mcast = b43_setup_dmaring(dev, 4, 1, type);
	if (!dma->tx_ring_mcast)
		goto err_destroy_vo;

	/*                       */
	dma->rx_ring = b43_setup_dmaring(dev, 0, 0, type);
	if (!dma->rx_ring)
		goto err_destroy_mcast;

	/*                                        */
	B43_WARN_ON(dev->dev->core_rev < 5);

	b43dbg(dev->wl, "%u-bit DMA initialized\n",
	       (unsigned int)type);
	err = 0;
out:
	return err;

err_destroy_mcast:
	destroy_ring(dma, tx_ring_mcast);
err_destroy_vo:
	destroy_ring(dma, tx_ring_AC_VO);
err_destroy_vi:
	destroy_ring(dma, tx_ring_AC_VI);
err_destroy_be:
	destroy_ring(dma, tx_ring_AC_BE);
err_destroy_bk:
	destroy_ring(dma, tx_ring_AC_BK);
	return err;
}

/*                                      */
static u16 generate_cookie(struct b43_dmaring *ring, int slot)
{
	u16 cookie;

	/*                                      
                                               
                         
                                                 
                                       
                                                     
                         
  */
	cookie = (((u16)ring->index + 1) << 12);
	B43_WARN_ON(slot & ~0x0FFF);
	cookie |= (u16)slot;

	return cookie;
}

/*                                                                    */
static
struct b43_dmaring *parse_cookie(struct b43_wldev *dev, u16 cookie, int *slot)
{
	struct b43_dma *dma = &dev->dma;
	struct b43_dmaring *ring = NULL;

	switch (cookie & 0xF000) {
	case 0x1000:
		ring = dma->tx_ring_AC_BK;
		break;
	case 0x2000:
		ring = dma->tx_ring_AC_BE;
		break;
	case 0x3000:
		ring = dma->tx_ring_AC_VI;
		break;
	case 0x4000:
		ring = dma->tx_ring_AC_VO;
		break;
	case 0x5000:
		ring = dma->tx_ring_mcast;
		break;
	}
	*slot = (cookie & 0x0FFF);
	if (unlikely(!ring || *slot < 0 || *slot >= ring->nr_slots)) {
		b43dbg(dev->wl, "TX-status contains "
		       "invalid cookie: 0x%04X\n", cookie);
		return NULL;
	}

	return ring;
}

static int dma_tx_fragment(struct b43_dmaring *ring,
			   struct sk_buff *skb)
{
	const struct b43_dma_ops *ops = ring->ops;
	struct ieee80211_tx_info *info = IEEE80211_SKB_CB(skb);
	struct b43_private_tx_info *priv_info = b43_get_priv_tx_info(info);
	u8 *header;
	int slot, old_top_slot, old_used_slots;
	int err;
	struct b43_dmadesc_generic *desc;
	struct b43_dmadesc_meta *meta;
	struct b43_dmadesc_meta *meta_hdr;
	u16 cookie;
	size_t hdrsize = b43_txhdr_size(ring->dev);

	/*                                                             
                                                                    
                                    
  */

	old_top_slot = ring->current_slot;
	old_used_slots = ring->used_slots;

	/*                            */
	slot = request_slot(ring);
	desc = ops->idx2desc(ring, slot, &meta_hdr);
	memset(meta_hdr, 0, sizeof(*meta_hdr));

	header = &(ring->txhdr_cache[(slot / TX_SLOTS_PER_FRAME) * hdrsize]);
	cookie = generate_cookie(ring, slot);
	err = b43_generate_txhdr(ring->dev, header,
				 skb, info, cookie);
	if (unlikely(err)) {
		ring->current_slot = old_top_slot;
		ring->used_slots = old_used_slots;
		return err;
	}

	meta_hdr->dmaaddr = map_descbuffer(ring, (unsigned char *)header,
					   hdrsize, 1);
	if (b43_dma_mapping_error(ring, meta_hdr->dmaaddr, hdrsize, 1)) {
		ring->current_slot = old_top_slot;
		ring->used_slots = old_used_slots;
		return -EIO;
	}
	ops->fill_descriptor(ring, desc, meta_hdr->dmaaddr,
			     hdrsize, 1, 0, 0);

	/*                             */
	slot = request_slot(ring);
	desc = ops->idx2desc(ring, slot, &meta);
	memset(meta, 0, sizeof(*meta));

	meta->skb = skb;
	meta->is_last_fragment = true;
	priv_info->bouncebuffer = NULL;

	meta->dmaaddr = map_descbuffer(ring, skb->data, skb->len, 1);
	/*                                                        */
	if (b43_dma_mapping_error(ring, meta->dmaaddr, skb->len, 1)) {
		priv_info->bouncebuffer = kmemdup(skb->data, skb->len,
						  GFP_ATOMIC | GFP_DMA);
		if (!priv_info->bouncebuffer) {
			ring->current_slot = old_top_slot;
			ring->used_slots = old_used_slots;
			err = -ENOMEM;
			goto out_unmap_hdr;
		}

		meta->dmaaddr = map_descbuffer(ring, priv_info->bouncebuffer, skb->len, 1);
		if (b43_dma_mapping_error(ring, meta->dmaaddr, skb->len, 1)) {
			kfree(priv_info->bouncebuffer);
			priv_info->bouncebuffer = NULL;
			ring->current_slot = old_top_slot;
			ring->used_slots = old_used_slots;
			err = -EIO;
			goto out_unmap_hdr;
		}
	}

	ops->fill_descriptor(ring, desc, meta->dmaaddr, skb->len, 0, 1, 1);

	if (info->flags & IEEE80211_TX_CTL_SEND_AFTER_DTIM) {
		/*                                               
                                                           */
		b43_shm_write16(ring->dev, B43_SHM_SHARED,
				B43_SHM_SH_MCASTCOOKIE, cookie);
	}
	/*                               */
	wmb();
	ops->poke_tx(ring, next_slot(ring, slot));
	return 0;

out_unmap_hdr:
	unmap_descbuffer(ring, meta_hdr->dmaaddr,
			 hdrsize, 1);
	return err;
}

static inline int should_inject_overflow(struct b43_dmaring *ring)
{
#ifdef CONFIG_B43_DEBUG
	if (unlikely(b43_debug(ring->dev, B43_DBG_DMAOVERFLOW))) {
		/*                                                      
                                                      */
		unsigned long next_overflow;

		next_overflow = ring->last_injected_overflow + HZ;
		if (time_after(jiffies, next_overflow)) {
			ring->last_injected_overflow = jiffies;
			b43dbg(ring->dev->wl,
			       "Injecting TX ring overflow on "
			       "DMA controller %d\n", ring->index);
			return 1;
		}
	}
#endif /*                  */
	return 0;
}

/*                                                                    */
static struct b43_dmaring *select_ring_by_priority(struct b43_wldev *dev,
						   u8 queue_prio)
{
	struct b43_dmaring *ring;

	if (dev->qos_enabled) {
		/*                      */
		switch (queue_prio) {
		default:
			B43_WARN_ON(1);
			/*             */
		case 0:
			ring = dev->dma.tx_ring_AC_VO;
			break;
		case 1:
			ring = dev->dma.tx_ring_AC_VI;
			break;
		case 2:
			ring = dev->dma.tx_ring_AC_BE;
			break;
		case 3:
			ring = dev->dma.tx_ring_AC_BK;
			break;
		}
	} else
		ring = dev->dma.tx_ring_AC_BE;

	return ring;
}

int b43_dma_tx(struct b43_wldev *dev, struct sk_buff *skb)
{
	struct b43_dmaring *ring;
	struct ieee80211_hdr *hdr;
	int err = 0;
	struct ieee80211_tx_info *info = IEEE80211_SKB_CB(skb);

	hdr = (struct ieee80211_hdr *)skb->data;
	if (info->flags & IEEE80211_TX_CTL_SEND_AFTER_DTIM) {
		/*                                                */
		ring = dev->dma.tx_ring_mcast;
		/*                                              
                            */
		hdr->frame_control |= cpu_to_le16(IEEE80211_FCTL_MOREDATA);
	} else {
		/*                                             */
		ring = select_ring_by_priority(
			dev, skb_get_queue_mapping(skb));
	}

	B43_WARN_ON(!ring->tx);

	if (unlikely(ring->stopped)) {
		/*                                               
                                                      
                                                                
                                        */
		if (b43_debug(dev, B43_DBG_DMAVERBOSE))
			b43err(dev->wl, "Packet after queue stopped\n");
		err = -ENOSPC;
		goto out;
	}

	if (unlikely(WARN_ON(free_slots(ring) < TX_SLOTS_PER_FRAME))) {
		/*                                                    
                                   */
		b43err(dev->wl, "DMA queue overflow\n");
		err = -ENOSPC;
		goto out;
	}

	/*                                                                 
                                                                  
                                                    */
	ring->queue_prio = skb_get_queue_mapping(skb);

	err = dma_tx_fragment(ring, skb);
	if (unlikely(err == -ENOKEY)) {
		/*                                                      
                                                   */
		dev_kfree_skb_any(skb);
		err = 0;
		goto out;
	}
	if (unlikely(err)) {
		b43err(dev->wl, "DMA tx mapping failure\n");
		goto out;
	}
	if ((free_slots(ring) < TX_SLOTS_PER_FRAME) ||
	    should_inject_overflow(ring)) {
		/*                       */
		unsigned int skb_mapping = skb_get_queue_mapping(skb);
		ieee80211_stop_queue(dev->wl->hw, skb_mapping);
		dev->wl->tx_queue_stopped[skb_mapping] = 1;
		ring->stopped = true;
		if (b43_debug(dev, B43_DBG_DMAVERBOSE)) {
			b43dbg(dev->wl, "Stopped TX ring %d\n", ring->index);
		}
	}
out:

	return err;
}

void b43_dma_handle_txstatus(struct b43_wldev *dev,
			     const struct b43_txstatus *status)
{
	const struct b43_dma_ops *ops;
	struct b43_dmaring *ring;
	struct b43_dmadesc_meta *meta;
	int slot, firstused;
	bool frame_succeed;

	ring = parse_cookie(dev, status->cookie, &slot);
	if (unlikely(!ring))
		return;
	B43_WARN_ON(!ring->tx);

	/*                                                             
                                                                 
               */
	firstused = ring->current_slot - ring->used_slots + 1;
	if (firstused < 0)
		firstused = ring->nr_slots + firstused;
	if (unlikely(slot != firstused)) {
		/*                                                   
                                                                  */
		b43dbg(dev->wl, "Out of order TX status report on DMA ring %d. "
		       "Expected %d, but got %d\n",
		       ring->index, firstused, slot);
		return;
	}

	ops = ring->ops;
	while (1) {
		B43_WARN_ON(slot < 0 || slot >= ring->nr_slots);
		/*                                  */
		ops->idx2desc(ring, slot, &meta);

		if (b43_dma_ptr_is_poisoned(meta->skb)) {
			b43dbg(dev->wl, "Poisoned TX slot %d (first=%d) "
			       "on ring %d\n",
			       slot, firstused, ring->index);
			break;
		}
		if (meta->skb) {
			struct b43_private_tx_info *priv_info =
				b43_get_priv_tx_info(IEEE80211_SKB_CB(meta->skb));

			unmap_descbuffer(ring, meta->dmaaddr, meta->skb->len, 1);
			kfree(priv_info->bouncebuffer);
			priv_info->bouncebuffer = NULL;
		} else {
			unmap_descbuffer(ring, meta->dmaaddr,
					 b43_txhdr_size(dev), 1);
		}

		if (meta->is_last_fragment) {
			struct ieee80211_tx_info *info;

			if (unlikely(!meta->skb)) {
				/*                                                 
                                         */
				b43dbg(dev->wl, "TX status unexpected NULL skb "
				       "at slot %d (first=%d) on ring %d\n",
				       slot, firstused, ring->index);
				break;
			}

			info = IEEE80211_SKB_CB(meta->skb);

			/*
                                                       
                                     
    */
			frame_succeed = b43_fill_txstatus_report(dev, info, status);
#ifdef CONFIG_B43_DEBUG
			if (frame_succeed)
				ring->nr_succeed_tx_packets++;
			else
				ring->nr_failed_tx_packets++;
			ring->nr_total_packet_tries += status->frame_count;
#endif /*       */
			ieee80211_tx_status(dev->wl->hw, meta->skb);

			/*                                            
                          */
			meta->skb = B43_DMA_PTR_POISON;
		} else {
			/*                                                
                                                     
    */
			if (unlikely(meta->skb)) {
				b43dbg(dev->wl, "TX status unexpected non-NULL skb "
				       "at slot %d (first=%d) on ring %d\n",
				       slot, firstused, ring->index);
				break;
			}
		}

		/*                                                           */
		ring->used_slots--;

		if (meta->is_last_fragment) {
			/*                                
                                          */
			break;
		}
		slot = next_slot(ring, slot);
	}
	if (ring->stopped) {
		B43_WARN_ON(free_slots(ring) < TX_SLOTS_PER_FRAME);
		ring->stopped = false;
	}

	if (dev->wl->tx_queue_stopped[ring->queue_prio]) {
		dev->wl->tx_queue_stopped[ring->queue_prio] = 0;
	} else {
		/*                                                      
                     */
		ieee80211_wake_queue(dev->wl->hw, ring->queue_prio);
		if (b43_debug(dev, B43_DBG_DMAVERBOSE)) {
			b43dbg(dev->wl, "Woke up TX ring %d\n", ring->index);
		}
	}
	/*                        */
	ieee80211_queue_work(dev->wl->hw, &dev->wl->tx_work);
}

static void dma_rx(struct b43_dmaring *ring, int *slot)
{
	const struct b43_dma_ops *ops = ring->ops;
	struct b43_dmadesc_generic *desc;
	struct b43_dmadesc_meta *meta;
	struct b43_rxhdr_fw4 *rxhdr;
	struct sk_buff *skb;
	u16 len;
	int err;
	dma_addr_t dmaaddr;

	desc = ops->idx2desc(ring, *slot, &meta);

	sync_descbuffer_for_cpu(ring, meta->dmaaddr, ring->rx_buffersize);
	skb = meta->skb;

	rxhdr = (struct b43_rxhdr_fw4 *)skb->data;
	len = le16_to_cpu(rxhdr->frame_len);
	if (len == 0) {
		int i = 0;

		do {
			udelay(2);
			barrier();
			len = le16_to_cpu(rxhdr->frame_len);
		} while (len == 0 && i++ < 5);
		if (unlikely(len == 0)) {
			dmaaddr = meta->dmaaddr;
			goto drop_recycle_buffer;
		}
	}
	if (unlikely(b43_rx_buffer_is_poisoned(ring, skb))) {
		/*                                   
                                                                           */
		b43dbg(ring->dev->wl, "DMA RX: Dropping poisoned buffer.\n");
		dmaaddr = meta->dmaaddr;
		goto drop_recycle_buffer;
	}
	if (unlikely(len + ring->frameoffset > ring->rx_buffersize)) {
		/*                                                
                                        
                                                            
                                              
   */
		int cnt = 0;
		s32 tmp = len;

		while (1) {
			desc = ops->idx2desc(ring, *slot, &meta);
			/*                                */
			b43_poison_rx_buffer(ring, meta->skb);
			sync_descbuffer_for_device(ring, meta->dmaaddr,
						   ring->rx_buffersize);
			*slot = next_slot(ring, *slot);
			cnt++;
			tmp -= ring->rx_buffersize;
			if (tmp <= 0)
				break;
		}
		b43err(ring->dev->wl, "DMA RX buffer too small "
		       "(len: %u, buffer: %u, nr-dropped: %d)\n",
		       len, ring->rx_buffersize, cnt);
		goto drop;
	}

	dmaaddr = meta->dmaaddr;
	err = setup_rx_descbuffer(ring, desc, meta, GFP_ATOMIC);
	if (unlikely(err)) {
		b43dbg(ring->dev->wl, "DMA RX: setup_rx_descbuffer() failed\n");
		goto drop_recycle_buffer;
	}

	unmap_descbuffer(ring, dmaaddr, ring->rx_buffersize, 0);
	skb_put(skb, len + ring->frameoffset);
	skb_pull(skb, ring->frameoffset);

	b43_rx(ring->dev, skb, rxhdr);
drop:
	return;

drop_recycle_buffer:
	/*                                   */
	b43_poison_rx_buffer(ring, skb);
	sync_descbuffer_for_device(ring, dmaaddr, ring->rx_buffersize);
}

void b43_dma_rx(struct b43_dmaring *ring)
{
	const struct b43_dma_ops *ops = ring->ops;
	int slot, current_slot;
	int used_slots = 0;

	B43_WARN_ON(ring->tx);
	current_slot = ops->get_current_rxslot(ring);
	B43_WARN_ON(!(current_slot >= 0 && current_slot < ring->nr_slots));

	slot = ring->current_slot;
	for (; slot != current_slot; slot = next_slot(ring, slot)) {
		dma_rx(ring, &slot);
		update_max_used_slots(ring, ++used_slots);
	}
	wmb();
	ops->set_current_rxslot(ring, slot);
	ring->current_slot = slot;
}

static void b43_dma_tx_suspend_ring(struct b43_dmaring *ring)
{
	B43_WARN_ON(!ring->tx);
	ring->ops->tx_suspend(ring);
}

static void b43_dma_tx_resume_ring(struct b43_dmaring *ring)
{
	B43_WARN_ON(!ring->tx);
	ring->ops->tx_resume(ring);
}

void b43_dma_tx_suspend(struct b43_wldev *dev)
{
	b43_power_saving_ctl_bits(dev, B43_PS_AWAKE);
	b43_dma_tx_suspend_ring(dev->dma.tx_ring_AC_BK);
	b43_dma_tx_suspend_ring(dev->dma.tx_ring_AC_BE);
	b43_dma_tx_suspend_ring(dev->dma.tx_ring_AC_VI);
	b43_dma_tx_suspend_ring(dev->dma.tx_ring_AC_VO);
	b43_dma_tx_suspend_ring(dev->dma.tx_ring_mcast);
}

void b43_dma_tx_resume(struct b43_wldev *dev)
{
	b43_dma_tx_resume_ring(dev->dma.tx_ring_mcast);
	b43_dma_tx_resume_ring(dev->dma.tx_ring_AC_VO);
	b43_dma_tx_resume_ring(dev->dma.tx_ring_AC_VI);
	b43_dma_tx_resume_ring(dev->dma.tx_ring_AC_BE);
	b43_dma_tx_resume_ring(dev->dma.tx_ring_AC_BK);
	b43_power_saving_ctl_bits(dev, 0);
}

static void direct_fifo_rx(struct b43_wldev *dev, enum b43_dmatype type,
			   u16 mmio_base, bool enable)
{
	u32 ctl;

	if (type == B43_DMA_64BIT) {
		ctl = b43_read32(dev, mmio_base + B43_DMA64_RXCTL);
		ctl &= ~B43_DMA64_RXDIRECTFIFO;
		if (enable)
			ctl |= B43_DMA64_RXDIRECTFIFO;
		b43_write32(dev, mmio_base + B43_DMA64_RXCTL, ctl);
	} else {
		ctl = b43_read32(dev, mmio_base + B43_DMA32_RXCTL);
		ctl &= ~B43_DMA32_RXDIRECTFIFO;
		if (enable)
			ctl |= B43_DMA32_RXDIRECTFIFO;
		b43_write32(dev, mmio_base + B43_DMA32_RXCTL, ctl);
	}
}

/*                                                              
                                                                      */
void b43_dma_direct_fifo_rx(struct b43_wldev *dev,
			    unsigned int engine_index, bool enable)
{
	enum b43_dmatype type;
	u16 mmio_base;

	type = dma_mask_to_engine_type(supported_dma_mask(dev));

	mmio_base = b43_dmacontroller_base(type, engine_index);
	direct_fifo_rx(dev, type, mmio_base, enable);
}
