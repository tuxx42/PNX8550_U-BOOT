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

#ifdef CONFIG_NXP_I2C_IP0105

#include <asm/io.h>
#include "nxp_i2c_ip0105.h"

/*-----------------------------------------------------------------------
 * Definitions
 */


/*-----------------------------------------------------------------------
 * Local functions
 */

static int do_i2c (unsigned long	bus_addr,
		   uchar		i2c_addr,
		   uchar		i2c_write,
		   uchar		*data,
		   int			*length,
		   uchar		*ack)
{
	uchar	result = 0 ;
	uchar	state ;
	uint	data_ndx = 0 ;
	int	cntr ;

	state = readl(bus_addr + IP0105_STATUS) & IP0105_STATUS__STA ;
	if (state != 0xF8) {
		printf ("\nI2C Error: Bus not in idle state\n") ;
		return -1 ;
	}

	do {
		switch (state) {
		case 0x00:  /* Bus error */
			result = -1 ;
			break ;

		case 0x08:  /* Start condition is transmitted */
		case 0x10:  /* Repeated start condition is transmitted */
			/* Clear start condition bit */
			writel( readl(bus_addr + IP0105_CONTROL) & ~IP0105_CONTROL__STA,
				bus_addr + IP0105_CONTROL );

			/* Write address and R/W bit into data register */
			writel( ((i2c_addr << 1) | (i2c_write?0:1)) & IP0105_DATA__DAT,
				bus_addr + IP0105_DATA );

			/* Clear interrupt */
			writel( IP0105_INT_CLEAR__INT_CLEAR,
				bus_addr + IP0105_INT_CLEAR );
			break ;

		case 0x18:  /* Slave addres + write is transmitted, Ack received */
		case 0x20:  /* Slave addres + write is transmitted, NAck received */
		case 0x40:  /* Slave addres + read is transmitted, Ack received */
		case 0x48:  /* Slave addres + read is transmitted, NAck received */
			*ack = ((state==0x18)||(state==0x40))?1:0;
			if (i2c_write) {
				if ((*length-data_ndx > 0) && *ack) {
					/* Write the next data byte */
					writel( data[data_ndx++] & IP0105_DATA__DAT, bus_addr + IP0105_DATA );
				} else
					/* Set stop condition bit */
					writel( readl(bus_addr + IP0105_STOP) | IP0105_STOP__STO,
						bus_addr + IP0105_STOP );
			} else {
				if ((*length-data_ndx > 0) && *ack) {
					if (*length-data_ndx == 1)
						/* Indicate next byte read is the last byte by not acknowledging it */
						writel( readl(bus_addr + IP0105_CONTROL) & ~IP0105_CONTROL__AA,
							bus_addr + IP0105_CONTROL );
				} else {
					/* Set stop condition bit */
					writel( readl(bus_addr + IP0105_STOP) | IP0105_STOP__STO,
						bus_addr + IP0105_STOP );
				}
			}

			/* Clear interrupt */
			writel(IP0105_INT_CLEAR__INT_CLEAR, bus_addr + IP0105_INT_CLEAR);
			break ;

		case 0x28:  /* Data byte has been transmitted, Ack received */
		case 0x30:  /* Data byte has been transmitted, NAck received */
			*ack = (state==0x28)?1:0;
			if ((*length-data_ndx > 0) && *ack) {
				writel( data[data_ndx++] & IP0105_DATA__DAT, bus_addr + IP0105_DATA );
			} else {
				/* Set stop condition bit */
				writel( readl(bus_addr + IP0105_STOP) | IP0105_STOP__STO,
					bus_addr + IP0105_STOP );
			}

			/* Clear interrupt */
			writel( IP0105_INT_CLEAR__INT_CLEAR,
				bus_addr + IP0105_INT_CLEAR );
			break ;

		case 0x50:  /* Data byte has been received, Ack was returned */
		case 0x58:  /* Data byte has been received, NAck was returned */
			*ack = (state == 0x50)?1:0;
			data[data_ndx++] = readl(bus_addr + IP0105_DATA) & IP0105_DATA__DAT ;

			switch(*length-data_ndx) {
			case 0:
				/* Set stop condition bit */
				writel( readl(bus_addr + IP0105_STOP) | IP0105_STOP__STO,
					bus_addr + IP0105_STOP );
				break;
			case 1:
				/* Indicate last byte reading by not acknowledging it */
				writel( readl(bus_addr + IP0105_CONTROL) & ~IP0105_CONTROL__AA,
					bus_addr + IP0105_CONTROL );
				break;
			}

			/* Clear interrupt */
			writel( IP0105_INT_CLEAR__INT_CLEAR,
				bus_addr + IP0105_INT_CLEAR );
			break ;

		case 0x38:
		case 0x60:
		case 0x68:
		case 0x70:
		case 0x78:
		case 0x80:
		case 0x88:
		case 0x90:
		case 0x98:
		case 0xA0:
		case 0xA8:
		case 0xB0:
		case 0xB8:
		case 0xC0:
		case 0xC8:
			/* Slave mode and recovery modes not yet supported */
			result = -1 ;
			break ;

		case 0xF8:  /* Idle */
			/* Enable auto acknowledge if required */
			if( (*length-data_ndx > 1) &&
				(i2c_write == 0) )
				writel( readl(bus_addr + IP0105_CONTROL) | IP0105_CONTROL__AA,
					bus_addr + IP0105_CONTROL );
			else
				writel( readl(bus_addr + IP0105_CONTROL) & ~IP0105_CONTROL__AA,
					bus_addr + IP0105_CONTROL );

			/* Set start condition bit */
			writel( readl(bus_addr + IP0105_CONTROL) | IP0105_CONTROL__STA,
				bus_addr + IP0105_CONTROL );

			/* Clear interrupt */
			writel( IP0105_INT_CLEAR__INT_CLEAR,
				bus_addr + IP0105_INT_CLEAR );
			break ;

		default:
			result = -2 ;
			break ;
		}

		/* Wait for a new I2C event */
		cntr = 4000 ;
		while (!(readl(bus_addr+IP0105_INT_STATUS) & IP0105_INT_STATUS__INT_STATUS) &&
			(cntr-- > 0))
			; /* nop */

		/* Read the new state */
		state = readl(bus_addr + IP0105_STATUS) & IP0105_STATUS__STA ;
	} while ((state != 0xF8) && (result == 0)) ;

	*length = data_ndx ;

	return(result);
}


/*=====================================================================*/
/*                         Public Functions                            */
/*=====================================================================*/

/*-----------------------------------------------------------------------
 * Initialization
 */
int nxp_i2c_ip0105_init (unsigned long bus_addr, int speed, int slaveaddr)
{
	/*
	 * WARNING: Do NOT save speed in a static variable: if the
	 * I2C routines are called before RAM is initialized (to read
	 * the DIMM SPD, for instance), RAM won't be usable and your
	 * system will crash.
	 */
	unsigned int reg ;

	/* Power up the module */
	writel( readl(bus_addr + IP0105_POWER_DOWN) & ~IP0105_POWER_DOWN__POWER_DOWN,
		bus_addr + IP0105_POWER_DOWN );

	if ( (readl(bus_addr + IP0105_MODULE_ID) & IP0105_MODULE_ID__MODULE_ID) !=
	     IP0105_MODULE_ID__MODULE_ID_ID ) {
		/* Power down the module */
		writel( readl(bus_addr + IP0105_POWER_DOWN) | IP0105_POWER_DOWN__POWER_DOWN,
			bus_addr + IP0105_POWER_DOWN );
		return(-1);
	}

	/* Configure the module */
	reg = readl(bus_addr + IP0105_CONTROL);
	reg &= ~(IP0105_CONTROL__EN |		/* Disable the module */
			IP0105_CONTROL__AA |		/* Disable returning acknowledges */
			IP0105_CONTROL__STA |		/* Clear start condition initiation */
			IP0105_CONTROL__STO );		/* Clear stop condition initiation */
	reg |= IP0105_CONTROL__CR2_150KHZ ;	/* Set bus speed to 150kHz */
	writel( reg, bus_addr + IP0105_CONTROL);

	/* Disable interrupt */
	writel( readl(bus_addr + IP0105_INT_ENABLE) & ~IP0105_INT_ENABLE__INT_ENABLE,
		bus_addr + IP0105_INT_ENABLE );

	/* Clear pending interrupt */
	writel( IP0105_INT_CLEAR__INT_CLEAR,
		bus_addr + IP0105_INT_CLEAR );

	/* Enable the module */
	writel( readl(bus_addr + IP0105_CONTROL) | IP0105_CONTROL__EN,
		bus_addr + IP0105_CONTROL );

	return(0);
}

/*-----------------------------------------------------------------------
 * Probe to see if a chip is present.  Also good for checking for the
 * completion of EEPROM writes since the chip stops responding until
 * the write completes (typically 10mSec).
 */
int nxp_i2c_ip0105_probe(unsigned long bus_addr, uchar addr)
{
	int	len = 0 ;
	uchar	ack ;

	/*
	 * perform 1 byte write transaction with just address byte
	 * (fake write)
	 */
	do_i2c(bus_addr, addr, 1, NULL, &len, &ack);

	return( ack ? 0 : 1);
}

/*-----------------------------------------------------------------------
 * Read bytes
 */
int  nxp_i2c_ip0105_read(unsigned long bus_addr, uchar chip, uint addr, int alen, uchar *buffer, int len)
{
	uchar	ack ;
	uchar	addr_bytes[4];
	int	length = len ;

#ifdef CFG_I2C_EEPROM_ADDR_OVERFLOW
	/*
	 * EEPROM chips that implement "address overflow" are ones
	 * like Catalyst 24WC04/08/16 which has 9/10/11 bits of
	 * address and the extra bits end up in the "chip address"
	 * bit slots. This makes a 24WC08 (1Kbyte) chip look like
	 * four 256 byte chips.
	 *
	 * Note that we consider the length of the address field to
	 * still be one byte because the extra address bits are
	 * hidden in the chip address.
	 */
	chip |= ((addr >> (alen * 8)) & CFG_I2C_EEPROM_ADDR_OVERFLOW);

	printf("i2c_read: fix addr_overflow: chip %02X addr %02X\n",
		chip, addr);
#endif

	/* Send the address pointer first */
	switch (alen) {
	default:
		printf("\nI2C Error: Unsupported address length!\n");
		break ;
	case 4:
		addr_bytes[3] = (addr & 0xFF000000) >> 24 ;
	case 3:
		addr_bytes[2] = (addr & 0x00FF0000) >> 16 ;
	case 2:
		addr_bytes[1] = (addr & 0x0000FF00) >> 8 ;
	case 1:
		addr_bytes[0] = (addr & 0x000000FF) ;
		do_i2c (bus_addr, chip, 1, addr_bytes, &alen, &ack);
		break ;
	case 0:
		/* Send not address pointer at all */
		break ;
	}

	/* Send the data */
	do_i2c (bus_addr, chip, 0, buffer, &len, &ack);
	if (len != length)
		printf("\nI2C Error: %d bytes of %d bytes read from device 0x%02X\n", len, length, chip);

	return(0);
}

/*-----------------------------------------------------------------------
 * Write bytes
 */
int  nxp_i2c_ip0105_write(unsigned long bus_addr, uchar chip, uint addr, int alen, uchar *buffer, int len)
{
	uchar	ack ;
	uchar	addr_bytes[4];
	int	length = len ;

	/* Send the address pointer first */
	switch (alen) {
	default:
		printf("\nI2C Error: Unsupported address length!\n");
		break ;
	case 4:
		addr_bytes[3] = (addr & 0xFF000000) >> 24 ;
	case 3:
		addr_bytes[2] = (addr & 0x00FF0000) >> 16 ;
	case 2:
		addr_bytes[1] = (addr & 0x0000FF00) >> 8 ;
	case 1:
		addr_bytes[0] = (addr & 0x000000FF) ;
		do_i2c (bus_addr, chip, 1, addr_bytes, &alen, &ack);
		break ;
	case 0:
		/* Send not address pointer at all */
		break ;
	}

	/* Send the data */
	do_i2c (bus_addr, chip, 1, buffer, &len, &ack);
	if (len != length)
		printf("\nI2C Error: %d bytes of %d bytes written to device 0x%02X\n", len, length, chip);

	return(0);
}

/*-----------------------------------------------------------------------
 * Read a register
 */
uchar nxp_i2c_ip0105_reg_read(unsigned long bus_addr, uchar i2c_addr, uchar reg)
{
	int	len = 1 ;
	uchar	buf;
	uchar	ack ;

	do_i2c (bus_addr, i2c_addr, 0, &buf, &len, &ack);
	if (len != 1)
		printf("\nI2C Error: Register read from device 0x%02X\n", i2c_addr);

	return(buf);
}

/*-----------------------------------------------------------------------
 * Write a register
 */
void nxp_i2c_ip0105_reg_write(unsigned long bus_addr, uchar i2c_addr, uchar reg, uchar val)
{
	int	len = 1 ;
	uchar	ack ;

	do_i2c (bus_addr, i2c_addr, 1, &val, &len, &ack);
	if (len != 1)
		printf("\nI2C Error: Register write to device 0x%02X\n", i2c_addr);
}

/*-----------------------------------------------------------------------
 * Set bus speed
 */
int nxp_i2c_ip0105_set_speed(unsigned long bus_addr, unsigned int speed)
{
	return (0) ;
}

/*-----------------------------------------------------------------------
 * Get bus speed
 */
unsigned int nxp_i2c_ip0105_get_speed(unsigned long bus_addr)
{
	printf("\nI2C Error: Setting speed is not supported yet\n");

	return (150000) ;
}


#endif	/* CONFIG_NXP_I2C_IP0105 */
