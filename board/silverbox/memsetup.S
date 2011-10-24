/*
 * (C) Copyright 2006
 * Philips Semiconductors B.V.
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
#include <version.h>
#include <asm/regdef.h>
#include <asm/mipsregs.h>

/*
 * This HUGE macro is a compile-time implementation of the LOG2 algorythm
 */
#define LOG2(x) ((((x)==0x00000001)/-1*0x00) |\
		 (((x)==0x00000002)/-1*0x01) |\
		 (((x)==0x00000004)/-1*0x02) |\
		 (((x)==0x00000008)/-1*0x03) |\
		 (((x)==0x00000010)/-1*0x04) |\
		 (((x)==0x00000020)/-1*0x05) |\
		 (((x)==0x00000040)/-1*0x06) |\
		 (((x)==0x00000080)/-1*0x07) |\
		 (((x)==0x00000100)/-1*0x08) |\
		 (((x)==0x00000200)/-1*0x09) |\
		 (((x)==0x00000400)/-1*0x0A) |\
		 (((x)==0x00000800)/-1*0x0B) |\
		 (((x)==0x00001000)/-1*0x0C) |\
		 (((x)==0x00002000)/-1*0x0D) |\
		 (((x)==0x00004000)/-1*0x0E) |\
		 (((x)==0x00008000)/-1*0x0F) |\
		 (((x)==0x00010000)/-1*0x10) |\
		 (((x)==0x00020000)/-1*0x11) |\
		 (((x)==0x00040000)/-1*0x12) |\
		 (((x)==0x00080000)/-1*0x13) |\
		 (((x)==0x00100000)/-1*0x14) |\
		 (((x)==0x00200000)/-1*0x15) |\
		 (((x)==0x00400000)/-1*0x16) |\
		 (((x)==0x00800000)/-1*0x17) |\
		 (((x)==0x01000000)/-1*0x18) |\
		 (((x)==0x02000000)/-1*0x19) |\
		 (((x)==0x04000000)/-1*0x1A) |\
		 (((x)==0x08000000)/-1*0x1B) |\
		 (((x)==0x10000000)/-1*0x1C) |\
		 (((x)==0x20000000)/-1*0x1D) |\
		 (((x)==0x40000000)/-1*0x1E) |\
		 (((x)==0x80000000)/-1*0x1F) )

/* Memory sub-system initialization code */

	.text
	.set	noreorder
	.set	mips32
	.globl	lowlevel_init

lowlevel_init:
	/* Setup CMEM0 to MMIO address space */
	lui	t0, (MMIO_BASE >> 16)				/* Set offset */
	ori	t0, t0, (LOG2(MMIO_SIZE >> 20)) << 1 |		/* Set size   */ \
			CP0_CMEMx__ENABLE			/* Set Enable */
	mtc0	t0, CP0_CMEM0, 4

	/* Setup CMEM1 to XIO space */
	lui	t0, (XIO_BASE >> 16)				/* Set offset */
	ori	t0, t0, (LOG2(XIO_SIZE >> 20)) << 1 |		/* Set size   */ \
			CP0_CMEMx__ENABLE			/* Set Enable */
	mtc0	t0, CP0_CMEM1, 5

	/* Setup CMEM2 to PCI memory space */
	lui	t0, (PCIMEM_BASE >> 16)				/* Set offset */
	ori	t0, t0, (LOG2(PCIMEM_SIZE >> 20)) << 1 |	/* Set size   */ \
			CP0_CMEMx__ENABLE			/* Set Enable */
	mtc0	t0, CP0_CMEM2, 6

	/* Setup CMEM3, to PCI I/O space */
	lui	t0, (PCIIO_BASE >> 16)				/* Set offset */
	ori	t0, t0, (LOG2(PCIIO_SIZE >> 20)) << 1 |		/* Set size   */ \
			CP0_CMEMx__ENABLE			/* Set Enable */
	mtc0	t0, CP0_CMEM3, 7

	nop	/* Make sure CMEM area's aren't accesed withing 3 instructions */

	j	ra
	nop
