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

#ifndef NXP_SDRAM_IP2031_H
#define NXP_SDRAM_IP2031_H

/*
 * Register address definitions
 */
/* DDR defines for IP2031 SDRAM controller */
#define IP2031_DDR_DEF_BANK_SWITCH		((IP2031) + 0x000004)
#define IP2031_DDR_RANK0_ADDR_LO		((IP2031) + 0x000010)
#define IP2031_DDR_RANK0_ADDR_HI		((IP2031) + 0x000014)
#define IP2031_DDR_RANK1_ADDR_HI		((IP2031) + 0x000018)
#define IP2031_DDR_REGION1_BASE			((IP2031) + 0x000040)
#define IP2031_DDR_REGION1_MASK			((IP2031) + 0x000044)
#define IP2031_DDR_REGION1_BANK_SWITCH		((IP2031) + 0x000048)
#define IP2031_DDR_RANK0_COLUMN_WIDTH		((IP2031) + 0x0000C0)
#define IP2031_DDR_RANK0_ROW_WIDTH		((IP2031) + 0x0000C4)
#define IP2031_DDR_RANK1_COLUMN_WIDTH		((IP2031) + 0x0000D0)
#define IP2031_DDR_RANK1_ROW_WIDTH		((IP2031) + 0x0000D4)


#endif /* NXP_SDRAM_IP2031_H */
