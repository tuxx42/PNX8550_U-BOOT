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

#ifndef NXP_GLOBAL2_IP0128_H_
#define NXP_GLOBAL2_IP0128_H_

#define IP0128			((TO_UNCACHED(MMIO_BASE)) + 0x0004D000)	/* Global 2 */

/*
 * Register address definitions
 */
#define IP0128_ENABLE_INTA_O			(IP0128 | 0x00000050)


/*
 * Register layout definitions
 */

/* IP0128_ENABLE_INTA_O */
#define IP0128_ENABLE_INTA_O__DAC_PD		(1 << 1)
#define IP0128_ENABLE_INTA_O__ENABLE_INTA_O	(1 << 0)

#endif /* NXP_GLOBAL2_IP0128_H_ */
