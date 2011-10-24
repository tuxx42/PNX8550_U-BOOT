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

#ifdef CONFIG_NXP_I2C

#include <asm/io.h>
#include <i2c.h>

#ifdef CONFIG_NXP_I2C_IP0105
#  include "nxp_i2c_ip0105.h"
#endif /* CONFIG_NXP_I2C_IP0105 */
#ifdef CONFIG_NXP_I2C_IP3203
#  include "nxp_i2c_ip3203.h"
#endif /* CONFIG_NXP_I2C_IP3203 */

/*-----------------------------------------------------------------------
 * Definitions
 */

#define MAX_I2C_BUSSES	8

typedef struct
{
	unsigned long	address ;
	int		(*init)		(unsigned long, int, int) ;
	int		(*probe)	(unsigned long, uchar) ;
	int		(*read)		(unsigned long, uchar, uint, int, uchar*, int) ;
	int		(*write)	(unsigned long, uchar, uint, int, uchar*, int) ;
	uchar		(*reg_read)	(unsigned long, uchar, uchar) ;
	void		(*reg_write)	(unsigned long, uchar, uchar, uchar) ;
	int		(*set_speed)	(unsigned long, unsigned int) ;
	unsigned int	(*get_speed)	(unsigned long) ;
} i2cbus ;


/*-----------------------------------------------------------------------
 * Local functions
 */

/* WARNING: We're ignoring the warning about globals because we have RAM
 * from the moment we've started */
int	curr_bus       = -1 ;
int	last_bus       = -1 ;
i2cbus	i2c_buses[MAX_I2C_BUSSES] ;

/*=====================================================================*/
/*                         Public Functions                            */
/*=====================================================================*/

/*-----------------------------------------------------------------------
 * Initialization
 */
void i2c_init (int speed, int slaveaddr)
{
	/*
	 * WARNING: Do NOT save speed in a static variable: if the
	 * I2C routines are called before RAM is initialized (to read
	 * the DIMM SPD, for instance), RAM won't be usable and your
	 * system will crash.
	 */
	unsigned long	addr ;
	int		new_bus = 0;

	addr = TO_UNCACHED(MMIO_BASE) ;
	while ( (new_bus < MAX_I2C_BUSSES) &&
		(addr < (TO_UNCACHED(MMIO_BASE) + MMIO_SIZE)) ) {
		unsigned long mod_id   =  (readl(addr + 0x0FFC) & 0xFFFF0000) >> 16 ;
		unsigned long mod_size = ((readl(addr + 0x0FFC) & 0x000000FF) + 1) * 0x1000 ;

		switch (mod_id) {
#ifdef CONFIG_NXP_I2C_IP0105
		case 0x0105:
			i2c_buses[new_bus].address	= addr ;
			i2c_buses[new_bus].init		= &nxp_i2c_ip0105_init ;
			i2c_buses[new_bus].probe	= &nxp_i2c_ip0105_probe ;
			i2c_buses[new_bus].read		= &nxp_i2c_ip0105_read ;
			i2c_buses[new_bus].write	= &nxp_i2c_ip0105_write ;
			i2c_buses[new_bus].reg_read	= &nxp_i2c_ip0105_reg_read ;
			i2c_buses[new_bus].reg_write	= &nxp_i2c_ip0105_reg_write ;
			i2c_buses[new_bus].set_speed	= &nxp_i2c_ip0105_set_speed ;
			i2c_buses[new_bus].get_speed	= &nxp_i2c_ip0105_get_speed ;
			break ;
#endif /* CONFIG_NXP_I2C_IP0105 */
#ifdef CONFIG_NXP_I2C_IP3203
		case 0x3203:
			i2c_buses[new_bus].address	= addr ;
			i2c_buses[new_bus].init		= &nxp_i2c_ip3203_init ;
			i2c_buses[new_bus].probe	= &nxp_i2c_ip3203_probe ;
			i2c_buses[new_bus].read		= &nxp_i2c_ip3203_read ;
			i2c_buses[new_bus].write	= &nxp_i2c_ip3203_write ;
			i2c_buses[new_bus].reg_read	= &nxp_i2c_ip3203_reg_read ;
			i2c_buses[new_bus].reg_write	= &nxp_i2c_ip3203_reg_write ;
			i2c_buses[new_bus].set_speed	= &nxp_i2c_ip3203_set_speed ;
			i2c_buses[new_bus].get_speed	= &nxp_i2c_ip3203_get_speed ;
			break ;
#endif /* CONFIG_NXP_I2C_IP3203 */
		default:
			i2c_buses[new_bus].address	= 0 ;
			break ;
		}

		if (i2c_buses[new_bus].address) {
			if( i2c_buses[new_bus].init( i2c_buses[new_bus].address, speed, slaveaddr) == 0 ) {
				printf("I2C:   Bus #%d, at 0x%08X\n", new_bus, i2c_buses[new_bus].address);
				last_bus = new_bus ;
				new_bus++ ;
			}
		}

		/* Increment address pointer to the next IP block */
		addr += mod_size ;
	}

	if (last_bus != -1)
		curr_bus = 0 ;
	else
		printf("I2C:   No supported bus found\n");
}

/*-----------------------------------------------------------------------
 * Probe to see if a chip is present.  Also good for checking for the
 * completion of EEPROM writes since the chip stops responding until
 * the write completes (typically 10mSec).
 */
int i2c_probe(uchar addr)
{
	if (i2c_buses[curr_bus].address)
		return( i2c_buses[curr_bus].probe( i2c_buses[curr_bus].address, addr ) ) ;
	else
		return (-1);
}

/*-----------------------------------------------------------------------
 * Read bytes
 */
int  i2c_read(uchar chip, uint addr, int alen, uchar *buffer, int len)
{
	if (i2c_buses[curr_bus].address)
		return( i2c_buses[curr_bus].read( i2c_buses[curr_bus].address, chip, addr, alen, buffer, len ) ) ;
	else
		return (-1);
}

/*-----------------------------------------------------------------------
 * Write bytes
 */
int  i2c_write(uchar chip, uint addr, int alen, uchar *buffer, int len)
{
	if (i2c_buses[curr_bus].address)
		return( i2c_buses[curr_bus].write( i2c_buses[curr_bus].address, chip, addr, alen, buffer, len ) ) ;
	else
		return (-1);
}

/*-----------------------------------------------------------------------
 * Read a register
 */
uchar i2c_reg_read(uchar i2c_addr, uchar reg)
{
	if (i2c_buses[curr_bus].address)
		return( i2c_buses[curr_bus].reg_read( i2c_buses[curr_bus].address, i2c_addr, reg ) ) ;
	else
		return (-1);
}

/*-----------------------------------------------------------------------
 * Write a register
 */
void i2c_reg_write(uchar i2c_addr, uchar reg, uchar val)
{
	if (i2c_buses[curr_bus].address)
		i2c_buses[curr_bus].reg_write( i2c_buses[curr_bus].address, i2c_addr, reg, val ) ;
}

int i2c_set_bus_num(unsigned int bus)
{
	if (bus > last_bus)
		return(-1);

	curr_bus = bus ;

	return(0);
}

unsigned int i2c_get_bus_num(void)
{
	return(curr_bus);
}

int i2c_set_bus_speed(unsigned int speed)
{
	if (i2c_buses[curr_bus].address)
		return( i2c_buses[curr_bus].set_speed( i2c_buses[curr_bus].address, speed ) ) ;
	else
		return (-1);
}

unsigned int i2c_get_bus_speed(void)
{
	if (i2c_buses[curr_bus].address)
		return( i2c_buses[curr_bus].get_speed( i2c_buses[curr_bus].address ) ) ;
	else
		return (-1);
}

#endif	/* CONFIG_NXP_I2C */
