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

#include <common.h>
#include <pci.h>
#include <nxp_pcixio_ipa051.h>
#include <asm/io.h>
#include <asm/addrspace.h>


#if defined(CONFIG_PCI) && defined(CONFIG_NXP_PCIXIO_IPA051)
static inline void nxp_ipa051_pci_clear_status( void )
{
	unsigned long pci_stat;

	pci_stat = inl(IPA051 + IPA051_GPPM_INT_STATUS);
	outl(pci_stat, IPA051 + IPA051_GPPM_INT_CLR);
}

static inline unsigned int nxp_ipa051_pci_calc_cfg_addr( pci_dev_t	dev,
							 int		where )
{
	unsigned int	addr;
	u32		bus;
	u32		device;

	bus    = ((dev & 0x00FF0000) >> 16);
	device = ((dev & 0x0000F800) >> 11);

	addr  = ((bus > 0) ? ((bus << PCI_CFG_BUS_SHIFT) | 1) : 0);
	addr |= (((dev >> 8) & 0xFF) << PCI_CFG_FUNC_SHIFT) | (where & 0xFC);

	return addr;
}

static int nxp_ipa051_pci_io ( unsigned int	ioaddr,
			       unsigned int	pci_mode,
			       unsigned int	pci_cmd,
			       unsigned int	*val )
{
	unsigned long retries = 0;

	/* Clear pending interrupt status */
	if (inl(IPA051 + IPA051_GPPM_INT_STATUS)) {
		nxp_ipa051_pci_clear_status();
		while (!(inl(IPA051 + IPA051_GPPM_INT_STATUS) == 0)) ;
	}

	outl(ioaddr, IPA051 + IPA051_GPPM_ADDR);

	if ((pci_cmd == IPA051_GPPM_CTRL__GPPM_CMD_IO_WR) ||
	    (pci_cmd == IPA051_GPPM_CTRL__GPPM_CMD_CFG_WR))
		outl(*val, IPA051 + IPA051_GPPM_WDAT);

	outl(IPA051_GPPM_CTRL__INIT_PCI_CYCLE | pci_cmd | (pci_mode & PCI_BYTE_ENABLE_MASK),
	     IPA051 + IPA051_GPPM_CTRL);

	while (1) {
		if (inl(IPA051 + IPA051_GPPM_INT_STATUS) & IPA051_GPPM_INT__GPPM_DONE) {
			if ((pci_cmd == IPA051_GPPM_CTRL__GPPM_CMD_IO_RD) ||
			    (pci_cmd == IPA051_GPPM_CTRL__GPPM_CMD_CFG_RD))
				*val = inl(IPA051 + IPA051_GPPM_RDAT);
			nxp_ipa051_pci_clear_status();
			return 0;
		} else if (inl(IPA051 + IPA051_GPPM_INT_STATUS) & IPA051_GPPM_INT__GPPM_R_MABORT) {
			break;
		}

		if (retries > (PCI_IO_TIMEOUT_US/PCI_IO_RETRYTIME_US)) {
			printf("%s : Arbiter Locked.\n", __FUNCTION__);
			return -1;
		}
		retries ++ ;
		udelay(PCI_IO_RETRYTIME_US);
	}

	nxp_ipa051_pci_clear_status();
	if ((pci_cmd == IPA051_GPPM_CTRL__GPPM_CMD_IO_RD) ||
	    (pci_cmd == IPA051_GPPM_CTRL__GPPM_CMD_IO_WR)) {
		printf("%s timeout (GPPM_CTRL=%X) ioaddr %X pci_cmd %X\n",
		       __FUNCTION__, inl(IPA051 + IPA051_GPPM_CTRL), ioaddr,
		       pci_cmd);
	}

	if ((pci_cmd == IPA051_GPPM_CTRL__GPPM_CMD_IO_RD) ||
	    (pci_cmd == IPA051_GPPM_CTRL__GPPM_CMD_CFG_RD))
		*val = 0xFFFFFFFF;
	return 0;
}

/*
 * We can't address 8 and 16 bit words directly.  Instead we have to
 * read/write a 32bit word and mask/modify the data we actually want.
 */
static int nxp_ipa051_pcibios_read_config_byte( struct pci_controller	*hose,
						pci_dev_t		dev,
						int			where,
						u8			*val )
{
	unsigned int	data = 0;
	int		err;

	if (dev == 0)
		return -1;

	err = nxp_ipa051_pci_io( nxp_ipa051_pci_calc_cfg_addr(dev, where),
				 ~(1 << (where & 3)),
				 IPA051_GPPM_CTRL__GPPM_CMD_CFG_RD,
				 &data );

	switch (where & 0x03) {
	case 0:
		*val = (unsigned char)(data & 0x000000FF);
		break;
	case 1:
		*val = (unsigned char)((data & 0x0000FF00) >> 8);
		break;
	case 2:
		*val = (unsigned char)((data & 0x00FF0000) >> 16);
		break;
	case 3:
		*val = (unsigned char)((data & 0xFF000000) >> 24);
		break;
	}

	return err;
}

static int nxp_ipa051_pcibios_read_config_word( struct pci_controller	*hose,
						pci_dev_t		dev,
						int			where,
						u16			*val )
{
	unsigned int	data = 0;
	int		err;

	if (dev == 0)
		return -1;

	if (where & 0x01)
		return -1;

	err = nxp_ipa051_pci_io( nxp_ipa051_pci_calc_cfg_addr(dev, where),
				 ~(3 << (where & 3)),
				 IPA051_GPPM_CTRL__GPPM_CMD_CFG_RD,
				 &data );

	switch (where & 0x02) {
	case 0:
		*val = (unsigned short)(data & 0x0000FFFF);
		break;
	case 2:
		*val = (unsigned short)((data & 0xFFFF0000) >> 16);
		break;
	}

	return err;
}

static int nxp_ipa051_pcibios_read_config_dword( struct pci_controller	*hose,
						 pci_dev_t		dev,
						 int			where,
						 u32			*val )
{
	int err;

	if (dev == 0)
		return -1;

	if (where & 0x03)
		return -1;

	err = nxp_ipa051_pci_io( nxp_ipa051_pci_calc_cfg_addr(dev, where),
				 0,
				 IPA051_GPPM_CTRL__GPPM_CMD_CFG_RD,
				 val );

	return err;
}

static int nxp_ipa051_pcibios_write_config_byte( struct pci_controller	*hose,
						 pci_dev_t		dev,
						 int			where,
						 u8			val )
{
	unsigned int	data = (unsigned int)val;
	int		err;

	if (dev == 0)
		return -1;

	switch (where & 0x03) {
	case 1:
		data = (data << 8);
		break;
	case 2:
		data = (data << 16);
		break;
	case 3:
		data = (data << 24);
		break;
	default:
		break;
	}

	err = nxp_ipa051_pci_io( nxp_ipa051_pci_calc_cfg_addr(dev, where),
				 ~(1 << (where & 3)),
				 IPA051_GPPM_CTRL__GPPM_CMD_CFG_WR,
				 &data );

	return err;
}

static int nxp_ipa051_pcibios_write_config_word( struct pci_controller	*hose,
						 pci_dev_t		dev,
						 int			where,
						 u16			val )
{
	unsigned int	data = (unsigned int)val;
	int		err;

	if (dev == 0)
		return -1;

	if (where & 0x01)
		return -1;

	switch (where & 0x02) {
	case 2:
		data = (data << 16);
		break;
	default:
		break;
	}

	err = nxp_ipa051_pci_io( nxp_ipa051_pci_calc_cfg_addr(dev, where),
				 ~(3 << (where & 3)),
				 IPA051_GPPM_CTRL__GPPM_CMD_CFG_WR,
				 &data );

	return err;
}

static int nxp_ipa051_pcibios_write_config_dword( struct pci_controller	*hose,
						  pci_dev_t		dev,
						  int			where,
						  u32			val )
{
	int err;

	if (dev == 0)
		return -1;

	if (where & 0x03)
		return -1;

	err = nxp_ipa051_pci_io( nxp_ipa051_pci_calc_cfg_addr(dev, where),
				 0,
				 IPA051_GPPM_CTRL__GPPM_CMD_CFG_WR, &val );

	return err;
}


/*
 *	Initialize Module
 */
void init_nxp_ipa051_pci( struct pci_controller	*hose )
{
	hose->first_busno = 0x00;
	hose->last_busno  = 0xFF;

	/* PCI memory space #1 */
	pci_set_region( hose->regions + 0,
			PCIMEM_BASE,
			TO_UNCACHED(PCIMEM_BASE),
			PCIMEM_SIZE,
			PCI_REGION_MEM );

	/* PCI I/O space #2 */
	pci_set_region( hose->regions + 1,
			PCIIO_BASE,
			TO_UNCACHED(PCIIO_BASE),
			PCIIO_SIZE,
			PCI_REGION_IO );

	/* System memory space */
	pci_set_region( hose->regions + 2,
			SDRAM_BASE,
			TO_CACHED(SDRAM_BASE),
			SDRAM_SIZE,
			PCI_REGION_MEM | PCI_REGION_MEMORY );

	hose->region_count = 3;

	hose->read_byte   = nxp_ipa051_pcibios_read_config_byte;
	hose->read_word   = nxp_ipa051_pcibios_read_config_word;
	hose->read_dword  = nxp_ipa051_pcibios_read_config_dword;
	hose->write_byte  = nxp_ipa051_pcibios_write_config_byte;
	hose->write_word  = nxp_ipa051_pcibios_write_config_word;
	hose->write_dword = nxp_ipa051_pcibios_write_config_dword;

	pci_register_hose(hose);

	hose->last_busno = pci_hose_scan(hose);
}

#endif /* CONFIG_PCI && CONFIG_NXP_PCIXIO_IPA051 */
