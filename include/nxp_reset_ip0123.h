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

#ifndef NXP_RESET_IP0123_H
#define NXP_RESET_IP0123_H

/*
 * Register address definitions
 */
#define IP0123_RST_CTL				0x00000000
#define IP0123_RST_CAUSE			0x00000004
#define IP0123_RST_EN_WATCHDOG_RST		0x00000008
#define IP0123_RESERVED				0x0000000C
#define IP0123_MODULE_ID			0x00000FFC


/*
 * Register layout definitions
 */

/* IP0123_RST_CTL */
#define IP0123_RST_CTL__REL_MIPS_RST_N		(1 << 3)
#define IP0123_RST_CTL__DO_SW_RST		(1 << 2)
#define IP0123_RST_CTL__REL_SYS_RST_OUT		(1 << 1)
#define IP0123_RST_CTL__ASSERT_SYS_RST_OUT	(1 << 0)

#endif /* NXP_RESET_IP0123_H */
