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

#ifndef PNX8550_GLB_H
#define PNX8550_GLB_H

#define TO_PHYSICAL(addr)	((unsigned long)(addr) & 0x1FFFFFFFUL)
#define TO_UNCACHED(addr)	((unsigned long)(addr) | 0xA0000000UL)
#define TO_CACHED(addr)		((unsigned long)(addr) | 0x80000000UL)

/* Physical Mappings */
#define SDRAM_BASE		0x00000000
#define SDRAM_SIZE		0x08000000  /* 128MiB */
#define XIO_BASE		0x10000000
#define XIO_SIZE		0x08000000	/* 128MiB (pwr of 2, 1 MiB minimum) */
#define MMIO_BASE		0x1BE00000
#define MMIO_SIZE		0x00200000	/*  2MiB (pwr of 2, 1 MiB minimum) */
#define PCIMEM_BASE		0x12000000
#define PCIMEM_SIZE		0x08000000	/* 128MiB (pwr of 2, 1 MiB minimum) */
#define PCIIO_BASE		0x1C000000
#define PCIIO_SIZE		0x02000000	/* 32MiB (pwr of 2, 1 MiB minimum) */

/* MDCS/TDCS busses in MMIO */
#define MDCS_BASE		(TO_UNCACHED(MMIO_BASE) + 0x00000000)
#define MDCS_SIZE		0x00100000	/*  1MiB */
#define TDCS_BASE		(TO_UNCACHED(MMIO_BASE) + 0x00100000)
#define TDCS_SIZE		0x00100000	/*  1MiB */

/* IP block register offsets */
#define IPA051			((TO_UNCACHED(MMIO_BASE)) + 0x00040000)	/* PCIXIO */
#define IP0123			((TO_UNCACHED(MMIO_BASE)) + 0x00060000)	/* Reset */
#define IP0126			((TO_UNCACHED(MMIO_BASE)) + 0x00063000)	/* Global */
#define IP0128			((TO_UNCACHED(MMIO_BASE)) + 0x0004D000)	/* Global 2 */
#define IP2031			((TO_UNCACHED(MMIO_BASE)) + 0x00065000)	/* SDRAM */
#define IP010F			((TO_UNCACHED(MMIO_BASE)) + 0x00104000)	/* GPIO */
#define IP0107_1		((TO_UNCACHED(MMIO_BASE)) + 0x0004A000)	/* UART1 */
#define IP0107_2		((TO_UNCACHED(MMIO_BASE)) + 0x0004B000)	/* UART2 */
#define IP0107_3		((TO_UNCACHED(MMIO_BASE)) + 0x0004C000)	/* UART3 */

#endif /* PNX8550_GLB_H */
