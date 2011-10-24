/*
 * (C) Copyright 2004
 * Pete Popov, Embedded Alley Solutions, Inc
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

#include <common.h>
#include <command.h>
#include <asm/io.h>
#include <asm/mipsregs.h>
#include <asm/addrspace.h>
#include <nxp_uart_ip0107.h>

#if defined(CONFIG_PCI)
#  include <pci.h>
#endif /* CONFIG_PCI */

#if defined(CONFIG_XIO)
#  include <linux/mtd/nand.h>
#endif /* CONFIG_XIO */

#if defined(CONFIG_PCI) || defined(CONFIG_XIO)
#  include <nxp_pcixio_ipa051.h>
#endif /* CONFIG_PCI || CONFIG_XIO */

/*
 * The rtl8139 ethernet driver uses inl/outl macros, yet
 * it ioremaps the ioaddr. Therefore, we need to set
 * mips_io_port_base to zero and define the registers in the
 * .h files to be in kseg1.
 */
/* unsigned long mips_io_port_base = 0; */ /* TODO: Why? */

long int initdram(int board_type)
{
	return SDRAM_SIZE;
}

/* In cpu/mips/cpu.c */
void write_one_tlb( int index, u32 pagemask, u32 hi, u32 low0, u32 low1 );

#if defined(CONFIG_PCI)
static struct pci_controller hose;
extern void init_nxp_ipa051_pci (struct pci_controller *);

void pci_init_board (void)
{
	init_nxp_ipa051_pci(&hose);
}
#endif

int checkboard (void)
{
#if 0
//#ifdef CONFIG_PCI
	int mem_size = initdram(0) / (1024 * 1024);
	int pci_mem_code;

	/* Calc the PCI mem size code */
	if (mem_size >= 128)
		pci_mem_code = IPA051_PCI_SETUP__BASExx_SIZ_128M;
	else if (mem_size >= 64)
		pci_mem_code = IPA051_PCI_SETUP__BASExx_SIZ_64M;
	else if (mem_size >= 32)
		pci_mem_code = IPA051_PCI_SETUP__BASExx_SIZ_32M;
	else
		pci_mem_code = IPA051_PCI_SETUP__BASExx_SIZ_16M;

	/* Set PCI_XIO registers */
	writel(PCIMEM_BASE, IPA051_PCI_BASE1_LO);
	writel(PCIMEM_BASE + PCIMEM_SIZE + 1, IPA051_PCI_BASE1_HI);
	writel(PCIIO_BASE, IPA051_PCI_BASE2_LO);
	writel(PCIIO_BASE + PCIIO_SIZE + 1, IPA051_PCI_BASE2_HI);

	/* Send memory transaction via PCI_BASE2 */
	writel(0x00000000, IPA051_PCI_IO);

	/* Unlock the setup register */
	writel(0xca, IPA051_UNLOCK_REGISTER);

	/*
	 * BAR0 of IP A051 (pci base 10) must be zero in order for ide
	 * to work, and in order for bus_to_baddr to work without any
	 * hacks.
	 */
	writel(0x00000000, IPA051_BASE10);

	/*
	 * These two bars are set by default or the boot code.
	 * However, it's safer to set them here so we're not boot
	 * code dependent.
	 */
	writel(MMIO_BASE, IPA051_BASE14);		/* PNX MMIO */
	writel(XIO_BASE, IPA051_BASE18);		/* XIO      */

	writel( IPA051_PCI_SETUP__EN_TA |
		IPA051_PCI_SETUP__EN_PCI2MMI |
		IPA051_PCI_SETUP__EN_XIO |
		IPA051_PCI_SETUP__BASE18_SIZ(IPA051_PCI_SETUP__BASExx_SIZ_128M) |
		IPA051_PCI_SETUP__EN_BASE18 |
		IPA051_PCI_SETUP__EN_BASE14 |
		IPA051_PCI_SETUP__BASE10_PREFETCHABLE |
		IPA051_PCI_SETUP__BASE10_SIZ(pci_mem_code) |
		IPA051_PCI_SETUP__EN_CONFIG_MANAG |
		IPA051_PCI_SETUP__EN_PCI_ARB,		/* Enable PCI arbiter */
		IPA051_PCI_SETUP );			/* PCI_SETUP */

	writel(0x00000000, IPA051_PCI_CONTROL);		/* PCI_CONTROL */
#else
	initdram(0);
#endif /* CONFIG_PCI */

	return 0;
}

#ifdef CONFIG_XIO
int board_nand_init (struct nand_chip *nand)
{
	volatile unsigned long *xioprofile = (volatile unsigned long *)nand->IO_ADDR_R ;

	/* Check if profile 'cntr' is enabled and configured for NAND FLASH */
	if ( ((*xioprofile & 0x00000001UL) != 0) &&
		((*xioprofile & 0x00000018UL) == 0x10) ) {
		unsigned long address ;

		/* Calculate the offset from the profile value */
		address = ((*xioprofile & 0x08000000UL) >> 23) |
			  ((*xioprofile & 0x000001E0UL) >> 5 ) ;
		address *= 0x00800000UL ;

		/* Add the address off XIO aperture */
		address += *(volatile unsigned long*)IPA051_BASE18 & (unsigned long)0xFFFFFFF0 ;

		nand->IO_ADDR_R = nand->IO_ADDR_W = (void  __iomem *)address ;
	} else
		nand->IO_ADDR_R = nand->IO_ADDR_W = NULL ;

	return -1;
}
#endif /* CONFIG_XIO */
