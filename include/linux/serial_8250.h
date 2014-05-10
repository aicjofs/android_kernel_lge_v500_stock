/*
 *  linux/include/linux/serial_8250.h
 *
 *  Copyright (C) 2004 Russell King
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */
#ifndef _LINUX_SERIAL_8250_H
#define _LINUX_SERIAL_8250_H

#include <linux/serial_core.h>
#include <linux/platform_device.h>

/*
                                                      
 */
struct plat_serial8250_port {
	unsigned long	iobase;		/*                 */
	void __iomem	*membase;	/*                        */
	resource_size_t	mapbase;	/*               */
	unsigned int	irq;		/*                  */
	unsigned long	irqflags;	/*                   */
	unsigned int	uartclk;	/*                 */
	void            *private_data;
	unsigned char	regshift;	/*                */
	unsigned char	iotype;		/*        */
	unsigned char	hub6;
	upf_t		flags;		/*             */
	unsigned int	type;		/*                   */
	unsigned int	(*serial_in)(struct uart_port *, int);
	void		(*serial_out)(struct uart_port *, int, int);
	void		(*set_termios)(struct uart_port *,
			               struct ktermios *new,
			               struct ktermios *old);
	int		(*handle_irq)(struct uart_port *);
	void		(*pm)(struct uart_port *, unsigned int state,
			      unsigned old);
};

/*
                                                            
                                                            
 */
enum {
	PLAT8250_DEV_LEGACY = -1,
	PLAT8250_DEV_PLATFORM,
	PLAT8250_DEV_PLATFORM1,
	PLAT8250_DEV_PLATFORM2,
	PLAT8250_DEV_FOURPORT,
	PLAT8250_DEV_ACCENT,
	PLAT8250_DEV_BOCA,
	PLAT8250_DEV_EXAR_ST16C554,
	PLAT8250_DEV_HUB6,
	PLAT8250_DEV_MCA,
	PLAT8250_DEV_AU1X00,
	PLAT8250_DEV_SM501,
};

/*
                                                        
                                                     
                                                      
                                
 */
struct uart_port;
struct uart_8250_port;

int serial8250_register_port(struct uart_port *);
void serial8250_unregister_port(int line);
void serial8250_suspend_port(int line);
void serial8250_resume_port(int line);

extern int early_serial_setup(struct uart_port *port);

extern int serial8250_find_port(struct uart_port *p);
extern int serial8250_find_port_for_earlycon(void);
extern int setup_early_serial8250_console(char *cmdline);
extern void serial8250_do_set_termios(struct uart_port *port,
		struct ktermios *termios, struct ktermios *old);
extern void serial8250_do_pm(struct uart_port *port, unsigned int state,
			     unsigned int oldstate);
extern int fsl8250_handle_irq(struct uart_port *port);
int serial8250_handle_irq(struct uart_port *port, unsigned int iir);
unsigned char serial8250_rx_chars(struct uart_8250_port *up, unsigned char lsr);
void serial8250_tx_chars(struct uart_8250_port *up);
unsigned int serial8250_modem_status(struct uart_8250_port *up);

extern void serial8250_set_isa_configurator(void (*v)
					(int port, struct uart_port *up,
						unsigned short *capabilities));

#endif
