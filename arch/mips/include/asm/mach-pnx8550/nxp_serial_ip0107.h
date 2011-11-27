/*
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
 *
 */

#ifndef NXP_SERIAL_IP0107_H_
#define NXP_SERIAL_IP0107_H_

#define IP0107_1		((TO_UNCACHED(MMIO_BASE)) + 0x0004A000)	/* UART1 */
#define IP0107_2		((TO_UNCACHED(MMIO_BASE)) + 0x0004B000)	/* UART2 */
#define IP0107_3		((TO_UNCACHED(MMIO_BASE)) + 0x0004C000)	/* UART3 */

/*
 * Register address definitions
 */
#define IP0107_LCR_FCR(BASE)			((BASE) + 0x00000000)
#define IP0107_MCR_MSR(BASE)			((BASE) + 0x00000004)
#define IP0107_BAUDRATE(BASE)			((BASE) + 0x00000008)
#define IP0107_CONFIG(BASE)			((BASE) + 0x0000000C)
#define IP0107_FIFOS_RBR_THR(BASE)		((BASE) + 0x00000028)
#define IP0107_INT_ST(BASE)			((BASE) + 0x00000FE0)
#define IP0107_INT_EN(BASE)			((BASE) + 0x00000FE4)
#define IP0107_INT_CLR(BASE)			((BASE) + 0x00000FE8)
#define IP0107_INT_SET(BASE)			((BASE) + 0x00000FEC)
#define IP0107_PD(BASE)				((BASE) + 0x00000FF4)
#define IP0107_MODULE_ID(BASE)			((BASE) + 0x00000FEC)

/*
 * Register layout definitions
 */

/* IP0107_LCR_FCR */
#define IP0107_LCR_FCR__TXBREAK			(1 << 30)
#define IP0107_LCR_FCR__EVENPARITY		(1 << 28)
#define IP0107_LCR_FCR__ENPARITY		(1 << 27)
#define IP0107_LCR_FCR__TWOSTOP			(1 << 26)
#define IP0107_LCR_FCR__BIT_8			(1 << 24)
#define IP0107_LCR_FCR__FIFO_THRES(X)		(((X) & 0x00000003) << 22)
#define IP0107_LCR_FCR__FIFO_THRES_1		(0x00000000 << 22)
#define IP0107_LCR_FCR__FIFO_THRES_4		(0x00000001 << 22)
#define IP0107_LCR_FCR__FIFO_THRES_8		(0x00000002 << 22)
#define IP0107_LCR_FCR__FIFO_THRES_14		(0x00000003 << 22)
#define IP0107_LCR_FCR__TX_FIFO_RST		(1 << 18)
#define IP0107_LCR_FCR__RX_FIFO_RST		(1 << 17)
#define IP0107_LCR_FCR__RX_FIFO_PT_ADV		(1 << 16)

/* IP0107_MCR_MSR */
#define IP0107_MCR_MSR__SCR			0xFF000000
#define IP0107_MCR_MSR__DCD			(1 << 23)
#define IP0107_MCR_MSR__CTS			(1 << 20)
#define IP0107_MCR_MSR__LOOP			(1 << 4)
#define IP0107_MCR_MSR__RTS			(1 << 1)
#define IP0107_MCR_MSR__DTR			(1 << 0)

/* IP0107_BAUDRATE */
#define IP0107_BAUDRATE__BAUDRATE(X)		((X) & 0x0000FFFF)
#define IP0107_BAUD2CODE(BAUD,CLOCK)			(CLOCK / (0x10 * (BAUD)) - 1)

/* IP0107_INT_ST, IP0107_INT_EN, IP0107_INT_CLR, IP0107_INT_SET */
#define IP0107_INT_x__TX			0x00000080
#define IP0107_INT_x__EMPTY			0x00000040
#define IP0107_INT_x__RCVTO			0x00000020
#define IP0107_INT_x__RX			0x00000010
#define IP0107_INT_x__RXOVRN			0x00000008
#define IP0107_INT_x__FRERR			0x00000004
#define IP0107_INT_x__BREAK			0x00000002
#define IP0107_INT_x__PARITY			0x00000001
#define IP0107_INT_x__ALLRX			0x0000003F
#define IP0107_INT_x__ALLTX			0x000000C0

/* IP0107_FIFOS_RBR_THR */
#define IP0107_FIFOS_RBR_THR__TXFIFO_STA	(0x1F << 16)
#define IP0107_FIFOS_RBR_THR__RX_BRK		(1 << 15)
#define IP0107_FIFOS_RBR_THR__RX_FE		(1 << 14)
#define IP0107_FIFOS_RBR_THR__RX_PAR		(1 << 13)
#define IP0107_FIFOS_RBR_THR__RXFIFO_STA	(0x1F << 8)
#define IP0107_FIFOS_RBR_THR__RBR(X)		((X) & 0x000000FF)
#define IP0107_FIFOS_RBR_THR__THR(X)		((X) & 0x000000FF)

#endif /* NXP_SERIAL_IP0107_H_ */
