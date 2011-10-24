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

#ifndef NXP_GPIO_IP010F_H
#define NXP_GPIO_IP010F_H

/*
 * Register address definitions
 */
#define IP010F_DIRQ0			0x00000500
#define IP010F_MODE_CONTROL1		0x00000004


/*
 * Register layout definitions
 */

/* IP010F_MODE_CONTROL1 */
#define IP010F_MODE_CONTROL1__MC31(X)	(((X) & 0x00000003) << 30)
#define IP010F_MODE_CONTROL1__MC30(X)	(((X) & 0x00000003) << 28)
#define IP010F_MODE_CONTROL1__MC29(X)	(((X) & 0x00000003) << 26)
#define IP010F_MODE_CONTROL1__MC28(X)	(((X) & 0x00000003) << 24)
#define IP010F_MODE_CONTROL1__MC27(X)	(((X) & 0x00000003) << 22)
#define IP010F_MODE_CONTROL1__MC26(X)	(((X) & 0x00000003) << 20)
#define IP010F_MODE_CONTROL1__MC25(X)	(((X) & 0x00000003) << 18)
#define IP010F_MODE_CONTROL1__MC24(X)	(((X) & 0x00000003) << 16)
#define IP010F_MODE_CONTROL1__MC23(X)	(((X) & 0x00000003) << 14)
#define IP010F_MODE_CONTROL1__MC22(X)	(((X) & 0x00000003) << 12)
#define IP010F_MODE_CONTROL1__MC21(X)	(((X) & 0x00000003) << 10)
#define IP010F_MODE_CONTROL1__MC20(X)	(((X) & 0x00000003) <<  8)
#define IP010F_MODE_CONTROL1__MC19(X)	(((X) & 0x00000003) <<  6)
#define IP010F_MODE_CONTROL1__MC18(X)	(((X) & 0x00000003) <<  4)
#define IP010F_MODE_CONTROL1__MC17(X)	(((X) & 0x00000003) <<  2)
#define IP010F_MODE_CONTROL1__MC16(X)	(((X) & 0x00000003) <<  0)


/*
 * Mode definitions
 */

/* IP010F_MODE_CONTROLx__MCx */
#define IP010F_MODE_CONTROLx__RETAIN	0x00000000
#define IP010F_MODE_CONTROLx__PRIMOP	0x00000001
#define IP010F_MODE_CONTROLx__NO_OPENDR	0x00000002
#define IP010F_MODE_CONTROLx__OPENDR	0x00000003

#endif /* NXP_GPIO_IP010F_H */
