/*
 * linux/arch/arm/mach-at91/board-rm9200dk.c
 *
 *  Copyright (C) 2005 SAN People
 *
 *  Epson S1D framebuffer glue code is:
 *     Copyright (C) 2005 Thibaut VARENE <varenet@parisc-linux.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <linux/types.h>
#include <linux/gpio.h>
#include <linux/init.h>
#include <linux/mm.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/spi/spi.h>
#include <linux/mtd/physmap.h>

#include <asm/setup.h>
#include <asm/mach-types.h>
#include <asm/irq.h>

#include <asm/mach/arch.h>
#include <asm/mach/map.h>
#include <asm/mach/irq.h>

#include <mach/hardware.h>
#include <mach/board.h>
#include <mach/at91rm9200_mc.h>
#include <mach/at91_ramc.h>

#include "generic.h"


static void __init dk_init_early(void)
{
	/*                                          */
	at91_initialize(18432000);

	/*                */
	at91_init_leds(AT91_PIN_PB2, AT91_PIN_PB2);

	/*                               */
	at91_register_uart(0, 0, 0);

	/*                                                        */
	at91_register_uart(AT91RM9200_ID_US1, 1, ATMEL_UART_CTS | ATMEL_UART_RTS
			   | ATMEL_UART_DTR | ATMEL_UART_DSR | ATMEL_UART_DCD
			   | ATMEL_UART_RI);

	/*                                        */
	at91_set_serial_console(0);
}

static struct macb_platform_data __initdata dk_eth_data = {
	.phy_irq_pin	= AT91_PIN_PC4,
	.is_rmii	= 1,
};

static struct at91_usbh_data __initdata dk_usbh_data = {
	.ports		= 2,
	.vbus_pin	= {-EINVAL, -EINVAL},
	.overcurrent_pin= {-EINVAL, -EINVAL},
};

static struct at91_udc_data __initdata dk_udc_data = {
	.vbus_pin	= AT91_PIN_PD4,
	.pullup_pin	= AT91_PIN_PD5,
};

static struct at91_cf_data __initdata dk_cf_data = {
	.irq_pin	= -EINVAL,
	.det_pin	= AT91_PIN_PB0,
	.vcc_pin	= -EINVAL,
	.rst_pin	= AT91_PIN_PC5,
};

#ifndef CONFIG_MTD_AT91_DATAFLASH_CARD
static struct at91_mmc_data __initdata dk_mmc_data = {
	.slot_b		= 0,
	.wire4		= 1,
	.det_pin	= -EINVAL,
	.wp_pin		= -EINVAL,
	.vcc_pin	= -EINVAL,
};
#endif

static struct spi_board_info dk_spi_devices[] = {
	{	/*                */
		.modalias	= "mtd_dataflash",
		.chip_select	= 0,
		.max_speed_hz	= 15 * 1000 * 1000,
	},
	{	/*                                  */
		.modalias	= "ur6hcps2",
		.chip_select	= 1,
		.max_speed_hz	= 250 *  1000,
	},
	{	/*                                                        */
		.modalias	= "tlv1504",
		.chip_select	= 2,
		.max_speed_hz	= 20 * 1000 * 1000,
	},
#ifdef CONFIG_MTD_AT91_DATAFLASH_CARD
	{	/*                */
		.modalias	= "mtd_dataflash",
		.chip_select	= 3,
		.max_speed_hz	= 15 * 1000 * 1000,
	}
#endif
};

static struct i2c_board_info __initdata dk_i2c_devices[] = {
	{
		I2C_BOARD_INFO("ics1523", 0x26),
	},
	{
		I2C_BOARD_INFO("x9429", 0x28),
	},
	{
		I2C_BOARD_INFO("24c1024", 0x50),
	}
};

static struct mtd_partition __initdata dk_nand_partition[] = {
	{
		.name	= "NAND Partition 1",
		.offset	= 0,
		.size	= MTDPART_SIZ_FULL,
	},
};

static struct atmel_nand_data __initdata dk_nand_data = {
	.ale		= 22,
	.cle		= 21,
	.det_pin	= AT91_PIN_PB1,
	.rdy_pin	= AT91_PIN_PC2,
	.enable_pin	= -EINVAL,
	.ecc_mode	= NAND_ECC_SOFT,
	.on_flash_bbt	= 1,
	.parts		= dk_nand_partition,
	.num_parts	= ARRAY_SIZE(dk_nand_partition),
};

#define DK_FLASH_BASE	AT91_CHIPSELECT_0
#define DK_FLASH_SIZE	SZ_2M

static struct physmap_flash_data dk_flash_data = {
	.width		= 2,
};

static struct resource dk_flash_resource = {
	.start		= DK_FLASH_BASE,
	.end		= DK_FLASH_BASE + DK_FLASH_SIZE - 1,
	.flags		= IORESOURCE_MEM,
};

static struct platform_device dk_flash = {
	.name		= "physmap-flash",
	.id		= 0,
	.dev		= {
				.platform_data	= &dk_flash_data,
			},
	.resource	= &dk_flash_resource,
	.num_resources	= 1,
};

static struct gpio_led dk_leds[] = {
	{
		.name			= "led0",
		.gpio			= AT91_PIN_PB2,
		.active_low		= 1,
		.default_trigger	= "heartbeat",
	}
};

static void __init dk_board_init(void)
{
	/*        */
	at91_add_device_serial();
	/*          */
	at91_add_device_eth(&dk_eth_data);
	/*          */
	at91_add_device_usbh(&dk_usbh_data);
	/*            */
	at91_add_device_udc(&dk_udc_data);
	at91_set_multi_drive(dk_udc_data.pullup_pin, 1);	/*                                  */
	/*               */
	at91_add_device_cf(&dk_cf_data);
	/*     */
	at91_add_device_i2c(dk_i2c_devices, ARRAY_SIZE(dk_i2c_devices));
	/*     */
	at91_add_device_spi(dk_spi_devices, ARRAY_SIZE(dk_spi_devices));
#ifdef CONFIG_MTD_AT91_DATAFLASH_CARD
	/*                */
	at91_set_gpio_output(AT91_PIN_PB7, 0);
#else
	/*     */
	at91_set_gpio_output(AT91_PIN_PB7, 1);	/*                                                            */
	at91_add_device_mmc(0, &dk_mmc_data);
#endif
	/*      */
	at91_add_device_nand(&dk_nand_data);
	/*           */
	platform_device_register(&dk_flash);
	/*      */
	at91_gpio_leds(dk_leds, ARRAY_SIZE(dk_leds));
	/*     */
//                       
}

MACHINE_START(AT91RM9200DK, "Atmel AT91RM9200-DK")
	/*                              */
	.timer		= &at91rm9200_timer,
	.map_io		= at91_map_io,
	.init_early	= dk_init_early,
	.init_irq	= at91_init_irq_default,
	.init_machine	= dk_board_init,
MACHINE_END
