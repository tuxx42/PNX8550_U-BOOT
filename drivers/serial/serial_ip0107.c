/*
 * NXP Semiconductors IP0107 UART support
 *
 * Copyright (c) 2004	Pete Popov, Embedded Alley Solutions, Inc
 * ppopov@embeddedalley.com
 * (C) Copyright 2006
 * NXP Semiconductors,
 * Robert Delien robert.delien@nxp.com, Hans Zuidam hans.zuidam@nxp.com
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <config.h>

#include <common.h>
#include <asm/io.h>
#include <asm/pnx8550.h>
#include <nxp_serial_ip0107.h>

/******************************************************************************
*
* serial_init - initialize a channel
*
* This routine initializes the number of data bits, parity
* and set the selected baud rate. Interrupts are disabled.
* Set the modem control signals if the option is selected.
*
* RETURNS: N/A
*/

int serial_init (void)
{
	/* power up the IP module */
	writel(0, IP0107_PD(CONFIG_CONSOLE_PORT));

	/* Set bit rate generator */
	serial_setbrg();

	return 0;
}

void serial_setbrg (void)
{
	/* Set line control parameters */
	writel(IP0107_LCR_FCR__BIT_8, IP0107_LCR_FCR(CONFIG_CONSOLE_PORT));

	/* Set modem control parameters */
	writel( IP0107_MCR_MSR__RTS | IP0107_MCR_MSR__DTR,
		IP0107_MCR_MSR(CONFIG_CONSOLE_PORT) );

	/* Set bit rate */
	writel( IP0107_BAUDRATE__BAUDRATE(IP0107_BAUD2CODE(CONFIG_BAUDRATE, CONFIG_IP0107_CLOCK)),
		IP0107_BAUDRATE(CONFIG_CONSOLE_PORT) );
}

void serial_putc (const char c)
{
	if (c == '\n')
		serial_putc ('\r');

	/* Wait for UARTA_TX register to empty */
	while( readl(IP0107_FIFOS_RBR_THR(CONFIG_CONSOLE_PORT)) &
	       IP0107_FIFOS_RBR_THR__TXFIFO_STA );

	/* Clear TX and EMPTY interrupt */
	writel( IP0107_INT_x__TX | IP0107_INT_x__EMPTY,
		IP0107_INT_CLR(CONFIG_CONSOLE_PORT) );

	/* Send the character */
	writel( IP0107_FIFOS_RBR_THR__THR(c),
		IP0107_FIFOS_RBR_THR(CONFIG_CONSOLE_PORT) );

	/* Wait for UARTA_TX register to empty */
	while (readl(IP0107_FIFOS_RBR_THR(CONFIG_CONSOLE_PORT)) &
	       IP0107_FIFOS_RBR_THR__TXFIFO_STA) ;

	/* Clear TX and EMPTY interrupt */
	writel( IP0107_INT_x__TX | IP0107_INT_x__EMPTY,
		IP0107_INT_CLR(CONFIG_CONSOLE_PORT) );
}

void serial_puts (const char *s)
{
	while (*s)
		serial_putc (*s++);
}

int serial_getc (void)
{
	unsigned char ch;

	while (!serial_tstc ());

	/* Read one character */
	ch = IP0107_FIFOS_RBR_THR__RBR(readl(IP0107_FIFOS_RBR_THR(CONFIG_CONSOLE_PORT)));

	/* Advance the RX FIFO read pointer */
	writel( readl(IP0107_LCR_FCR(CONFIG_CONSOLE_PORT)) | IP0107_LCR_FCR__RX_FIFO_PT_ADV,
		IP0107_LCR_FCR(CONFIG_CONSOLE_PORT) );

	return ch;
}

int serial_tstc (void)
{
	if ((readl(IP0107_FIFOS_RBR_THR(CONFIG_CONSOLE_PORT)) & IP0107_FIFOS_RBR_THR__RXFIFO_STA) >> 8)
		return 1;
	else
		return 0;
}
