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

#ifndef NXP_PCIXIO_IPA051_H
#define NXP_PCIXIO_IPA051_H

/*
 * Timeout and retry time for IO and CFG accesses
 * in microseconds
 */
#define PCI_IO_TIMEOUT_US			10
#define PCI_IO_RETRYTIME_US			1

#define PCI_BYTE_ENABLE_MASK			0x0000000F
#define PCI_CFG_BUS_SHIFT			16
#define PCI_CFG_FUNC_SHIFT			8

/*
 * Register address definitions
 */
#define IPA051_PCI_SETUP			0x00000010
#define IPA051_PCI_CONTROL			0x00000014
#define IPA051_PCI_BASE1_LO			0x00000018
#define IPA051_PCI_BASE1_HI			0x0000001C
#define IPA051_PCI_BASE2_LO			0x00000020
#define IPA051_PCI_BASE2_HI			0x00000024
#define IPA051_READ_LIFETIME			0x00000028
#define IPA051_GPPM_ADDR			0x0000002C
#define IPA051_GPPM_WDAT			0x00000030
#define IPA051_GPPM_RDAT			0x00000034
#define IPA051_GPPM_CTRL			0x00000038
#define IPA051_UNLOCK_REGISTER			0x0000003C
#define IPA051_DEVICE_VENDORID			0x00000040
#define IPA051_CONFIG_CMD_STAT			0x00000044
#define IPA051_CLASS_CODE_REV_ID		0x00000048
#define IPA051_LATENCY_TIMER			0x0000004C
#define IPA051_BASE10				0x00000050
#define IPA051_BASE14				0x00000054
#define IPA051_BASE18				0x00000058
/*	IPA051_RESERVED				0x0000005C ... 0x00000060 */
#define IPA051_SUBSYSTEM_IDS			0x0000006C
/*      IPA051_RESERVED				0x00000070 */
#define IPA051_CAP_POINTER			0x00000074
/*      IPA051_RESERVED				0x00000078 */
#define IPA051_CONFIG_MISC			0x0000007C
#define IPA051_PMC				0x00000080
#define IPA051_PWR_STATE			0x00000084
#define IPA051_PCI_IO				0x00000088
#define IPA051_SLV_TUNING			0x0000008C
#define IPA051_DMA_TUNING			0x00000090
/*	IPA051_RESERVED				0x00000094 ... 0x000007FC */
#define IPA051_DMA_EADDR			0x00000800
#define IPA051_DMA_IADDR			0x00000804
#define IPA051_DMA_LENGTH			0x00000808
#define IPA051_DMA_CTRL				0x0000080C
#define IPA051_XIO_CTRL				0x00000810
#define IPA051_XIO_SEL0_PROF			0x00000814
#define IPA051_XIO_SEL1_PROF			0x00000818
#define IPA051_XIO_SEL2_PROF			0x0000081C
#define IPA051_GPXIO_ADDR			0x00000820
#define IPA051_GPXIO_WDATA			0x00000824
#define IPA051_GPXIO_RDATA			0x00000828
#define IPA051_GPXIO_CTRL			0x0000082C
#define IPA051_NAND_CTRLS			0x00000830
#define IPA051_XIO_SEL3_PROF			0x00000834
#define IPA051_XIO_SEL4_PROF			0x00000838
/*	IPA051_RESERVED				0x0000083C ... 0x00000FAC */
#define IPA051_GPXIO_INT_STATUS			0x00000FB0
#define IPA051_GPXIO_INT_MASK			0x00000FB4
#define IPA051_GPXIO_INT_CLR			0x00000FB8
#define IPA051_GPXIO_INT_SET			0x00000FBC
#define IPA051_GPPM_INT_STATUS			0x00000FC0
#define IPA051_GPPM_INT_MASK			0x00000FC4
#define IPA051_GPPM_INT_CLR			0x00000FC8
#define IPA051_GPPM_INT_SET			0x00000FCC
#define IPA051_DMA_INT_STATUS			0x00000FD0
#define IPA051_DMA_INT_MASK			0x00000FD4
#define IPA051_DMA_INT_CLR			0x00000FD8
#define IPA051_DMA_INT_SET			0x00000FDC
#define IPA051_PCI_INT_STATUS			0x00000FE0
#define IPA051_PCI_INT_MASK			0x00000FE4
#define IPA051_PCI_INT_CLR			0x00000FE8
#define IPA051_PCI_INT_SET			0x00000FEC
/*	IPA051_RESERVED				0x00000FF0 ... 0x00000FF8 */
#define IPA051_MODULE_ID			0x00000FFC


/*
 * Register layout definitions
 */

/* IPA051_GPXIO_CTRL register fields */
#define IPA051_GPXIO_CTRL__GPXIO_CYC_PENDING	(1 << 9)
#define IPA051_GPXIO_CTRL__GPXIO_DONE		(1 << 8)
#define IPA051_GPXIO_CTRL__CLR_GPXIO_DONE	(1 << 7)
#define IPA051_GPXIO_CTRL__GPXIO_INIT		(1 << 6)
#define IPA051_GPXIO_CTRL__GPXIO_RD		(1 << 4)
#define IPA051_GPXIO_CTRL__GPXIO_BEN(X)		((X) & 0x0000000F)

/* IPA051_NAND_CTRLS register fields */
#define IPA051_NAND_CTRLS__CTRLS_64MB		(1 << 21)
#define IPA051_NAND_CTRLS__CTRLS_INC_DATA	(1 << 20)
#define IPA051_NAND_CTRLS__CTRLS_CMD_PH(X)	(((X) & 0x00000003) << 18)
#define IPA051_NAND_CTRLS__CTRLS_ADR_PH(X)	(((X) & 0x00000003) << 16)
#define IPA051_NAND_CTRLS__COMMAND_B(X)		(((X) << 8) & 0x0000FF00)
#define IPA051_NAND_CTRLS__COMMAND_A(X)		((X) & 0x000000FF)

/* IPA051_GPXIO_STATUS register fields */
#define IPA051_GPXIO_STATUS__GPXIO_XIO_ACK_DONE	(1 << 14)
#define IPA051_GPXIO_STATUS__GPXIO_DONE		(1 << 13)
#define IPA051_GPXIO_STATUS__GPXIO_ERR		(1 << 9)
#define IPA051_GPXIO_STATUS__GPXIO_R_MABORT	(1 << 2)

/* IPA051_DMA_CTRL register fields */
#define IPA051_DMA_CTRL__SINGLE_DATA_PHASE	(1 << 10)
#define IPA051_DMA_CTRL__SND2XIO		(1 << 9)
#define IPA051_DMA_CTRL__FIX_ADDR		(1 << 8)
#define IPA051_DMA_CTRL__MAX_BURST_SIZE(x)	(((x) & 0x00000007) << 5)
#define IPA051_DMA_CTRL__MAX_BURST_SIZE_8	(0 << 5)
#define IPA051_DMA_CTRL__MAX_BURST_SIZE_16	(1 << 5)
#define IPA051_DMA_CTRL__MAX_BURST_SIZE_32	(2 << 5)
#define IPA051_DMA_CTRL__MAX_BURST_SIZE_64	(3 << 5)
#define IPA051_DMA_CTRL__MAX_BURST_SIZE_128	(4 << 5)
#define IPA051_DMA_CTRL__MAX_BURST_SIZE_256	(5 << 5)
#define IPA051_DMA_CTRL__MAX_BURST_SIZE_512	(6 << 5)
#define IPA051_DMA_CTRL__MAX_BURST_SIZE_NORES	(7 << 5)
#define IPA051_DMA_CTRL__INIT_DMA		(1 << 4)
#define IPA051_DMA_CTRL__CMD_TYPE(x)		((X) & 0x0000000F)

/* IPA051_XIO_SEL0_PROF, IPA051_XIO_SEL1_PROF, IPA051_XIO_SEL2_PROF,
 * IPA051_XIO_SEL3_PROF, IPA051_XIO_SEL4_PROF register fields */
#define IPA051_XIO_SELx_PROF__MISC_CTRL		(1 << 24)
#define IPA051_XIO_SELx_PROF__EN_16BIT		(1 << 23)
#define IPA051_XIO_SELx_PROF__USE_ACK		(1 << 22)
#define IPA051_XIO_SELx_PROF__REN_HIGH		0x00100000
#define IPA051_XIO_SELx_PROF__REN_LOW		0x00040000
#define IPA051_XIO_SELx_PROF__WEN_HIGH		0x00010000
#define IPA051_XIO_SELx_PROF__WEN_LOW		0x00004000
#define IPA051_XIO_SELx_PROF__WAIT		0x00000200
#define IPA051_XIO_SELx_PROF__OFFSET		0x00000020
#define IPA051_XIO_SELx_PROF__TYPE_68360	0x00000000
#define IPA051_XIO_SELx_PROF__TYPE_NOR		0x00000008
#define IPA051_XIO_SELx_PROF__TYPE_NAND		0x00000010
#define IPA051_XIO_SELx_PROF__TYPE_IDE		0x00000018
#define IPA051_XIO_SELx_PROF__SIZE_8MB		0x00000000
#define IPA051_XIO_SELx_PROF__SIZE_16MB		0x00000002
#define IPA051_XIO_SELx_PROF__SIZE_32MB		0x00000004
#define IPA051_XIO_SELx_PROF__SIZE_64MB		0x00000006
#define IPA051_XIO_SELx_PROF__ENAB		(1 << 0)

/* IPA051_GPPM_CTRL register fields */
#define IPA051_GPPM_CTRL__GPPM_DONE		(1<<10)
#define IPA051_GPPM_CTRL__INIT_PCI_CYCLE	(1<<9)
#define IPA051_GPPM_CTRL__GPPM_CMD_IO_RD	0x00000020
#define IPA051_GPPM_CTRL__GPPM_CMD_IO_WR	0x00000030
#define IPA051_GPPM_CTRL__GPPM_CMD_MEM_RD	0x00000060 /*?*/
#define IPA051_GPPM_CTRL__GPPM_CMD_MEM_WR	0x00000070 /*?*/
#define IPA051_GPPM_CTRL__GPPM_CMD_CFG_RD	0x000000A0
#define IPA051_GPPM_CTRL__GPPM_CMD_CFG_WR	0x000000B0
#define IPA051_GPPM_CTRL__GPPM_CMD(X)		(((X)&0x0000000F)<<4)
#define IPA051_GPPM_CTRL__GPPM_BEN(X)		((X)&0x0000000F)

/* IPA051_GPPM_INT register fields */
#define IPA051_GPPM_INT__GPPM_DONE		(1<<10)
#define IPA051_GPPM_INT__GPPM_ERR		(1<<9)
#define IPA051_GPPM_INT__GPPM_MSTR_PARITY_ERR	(1<<5)
#define IPA051_GPPM_INT__GPPM_ERR_PARITY	(1<<4)
#define IPA051_GPPM_INT__GPPM_R_MABORT		(1<<2)
#define IPA051_GPPM_INT__GPPM_R_TABORT		(1<<1)

/* IPA051_PCI_SETUP register fields */
#define IPA051_PCI_SETUP__DIS_REQGNT		(1<<30)
#define IPA051_PCI_SETUP__DIS_REQGNT_A		(1<<29)
#define IPA051_PCI_SETUP__DIS_REQGNT_B		(1<<28)
#define IPA051_PCI_SETUP__D2_SUPPORT		(1<<27)
#define IPA051_PCI_SETUP__D1_SUPPORT		(1<<26)
#define IPA051_PCI_SETUP__EN_TA			(1<<24)
#define IPA051_PCI_SETUP__EN_PCI2MMI		(1<<23)
#define IPA051_PCI_SETUP__EN_XIO		(1<<22)
#define IPA051_PCI_SETUP__BASE18_PREFETCHABLE	(1<<21)
#define IPA051_PCI_SETUP__BASE18_SIZ(X)		(X<<18)
#define IPA051_PCI_SETUP__EN_BASE18		(1<<17)
#define IPA051_PCI_SETUP__BASE14_PREFETCHABLE	(1<<16)
#define IPA051_PCI_SETUP__BASE14_SIZ(X)		(X<<12)
#define IPA051_PCI_SETUP__EN_BASE14		(1<<11)
#define IPA051_PCI_SETUP__BASE10_PREFETCHABLE	(1<<10)
#define IPA051_PCI_SETUP__BASE10_SIZ(X)		(X<<7)
#define IPA051_PCI_SETUP__EN_CONFIG_MANAG	(1<<1)
#define IPA051_PCI_SETUP__EN_PCI_ARB		(1<<0)
#define IPA051_PCI_SETUP__BASExx_SIZ_16M	0x00000003
#define IPA051_PCI_SETUP__BASExx_SIZ_32M	0x00000004
#define IPA051_PCI_SETUP__BASExx_SIZ_64M	0x00000005
#define IPA051_PCI_SETUP__BASExx_SIZ_128M	0x00000006


void init_nxp_ipa051_pci(struct pci_controller*) ;

#endif /* NXP_PCIXIO_IPA051_H */
