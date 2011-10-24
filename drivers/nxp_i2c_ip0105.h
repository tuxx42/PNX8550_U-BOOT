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

#ifndef NXP_I2C_IP0105_H
#define NXP_I2C_IP0105_H

/*
 * WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING
 *
 * The implementation MUST NOT use static or global variables if the
 * I2C routines are used to read SDRAM configuration information
 * because this is done before the memories are initialized. Limited
 * use of stack-based variables are OK (the initial stack size is
 * limited).
 *
 * WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING
 */

/*
 * Layout of IIC registers.
 */
#define IP0105_CONTROL				(0x00000000)
#define IP0105_DATA				(0x00000004)
#define IP0105_STATUS				(0x00000008)
#define IP0105_ADDRESS				(0x0000000C)
#define IP0105_STOP				(0x00000010)
#define IP0105_PD				(0x00000014)
#define IP0105_BUS_SET				(0x00000018)
#define IP0105_BUS_OBSERVATION			(0x0000001C)
#define IP0105_INT_STATUS			(0x00000FE0)
#define IP0105_INT_ENABLE			(0x00000FE4)
#define IP0105_INT_CLEAR			(0x00000FE8)
#define IP0105_INT_SET				(0x00000FEC)
#define IP0105_POWER_DOWN			(0x00000FF4)
#define IP0105_MODULE_ID			(0x00000FFC)

/*
 * Control register fields
 */
#define IP0105_CONTROL__AA			(1 << 7)
#define IP0105_CONTROL__EN			(1 << 6)
#define IP0105_CONTROL__STA			(1 << 5)
#define IP0105_CONTROL__STO			(1 << 4)
#define IP0105_CONTROL__CR2			(7 << 0)
#define IP0105_CONTROL__CR2_400KHZ		(0 << 0)
#define IP0105_CONTROL__CR2_300KHZ		(1 << 0)
#define IP0105_CONTROL__CR2_200KHZ		(2 << 0)
#define IP0105_CONTROL__CR2_150KHZ		(3 << 0)
#define IP0105_CONTROL__CR2_100KHZ		(4 << 0)
#define IP0105_CONTROL__CR2_75KHZ		(5 << 0)
#define IP0105_CONTROL__CR2_50KHZ		(6 << 0)
#define IP0105_CONTROL__CR2_25KHZ		(7 << 0)

/*
 * Data register fields
 */
#define IP0105_DATA__DAT			(0xFF << 0)

/*
 * Status register fields
 */
#define IP0105_STATUS__STA			(0xF8)
#define IP0105_STATUS__STA_START_DONE		(0x08)		/* Start condition sent */
#define IP0105_STATUS__STA_RESTART_DONE		(0x10)		/* Start condition repeated */
#define IP0105_STATUS__STA_SLA_W_ACK		(0x18)		/* Ack after SLA + W */
#define IP0105_STATUS__STA_DATA_W_ACK		(0x28)		/* Ack after data transmit */
#define IP0105_STATUS__STA_DATA_W_NACK		(0x30)		/* Nack after data transmit */
#define IP0105_STATUS__STA_SLA_R_ACK		(0x40)		/* Ack after SLA + R */
#define IP0105_STATUS__STA_DATA_R_ACK		(0x50)		/* Data received, ack sent */
#define IP0105_STATUS__STA_DATA_R_NACK		(0x58)		/* Data received, nack sent */
#define IP0105_STATUS__STA_END			(0xC0)		/* Data received, nack sent */
#define IP0105_STATUS__STA_NOP			(0xF8)		/* Data received, nack sent */

/*
 * Address register fields
 */
#define IP0105_ADDRESS__SLAVE_ADDR		(0xFF << 1)
#define IP0105_ADDRESS__GEN_CALL_ADDR		(1 << 0)

/*
 * Stop register fields
 */
#define IP0105_STOP__STO			(1 << 0)

/*
 * PD register fields
 */
#define IP0105_PD__PD				(1 << 2)

/*
 * Bus set register fields
 */
#define IP0105_BUS_SET__SET_SCL_LOW		(1 << 1)
#define IP0105_BUS_SET__SET_SDA_LOW		(1 << 0)

/*
 * Bus observation register fields
 */
#define IP0105_BUS_OBSERVATION__OBSERVE_SCL	(1 << 1)
#define IP0105_BUS_OBSERVATION__OBSERVE_SDA	(1 << 0)


/*
 * Interrupt status register fields
 */
#define IP0105_INT_STATUS__INT_STATUS		(1 << 0)

/*
 * Interrupt enable register fields
 */
#define IP0105_INT_ENABLE__INT_ENABLE		(1 << 0)

/*
 * Interrupt clear register fields
 */
#define IP0105_INT_CLEAR__INT_CLEAR		(1 << 0)

/*
 * Interrupt set register fields
 */
#define IP0105_INT_CLEAR__INT_SET		(1 << 0)

/*
 * Power down register fields
 */
#define IP0105_POWER_DOWN__POWER_DOWN		(1 << 31)

/*
 * Power down register fields
 */
#define IP0105_MODULE_ID__MODULE_ID		(0xFFFF << 16)
#define IP0105_MODULE_ID__MODULE_ID_ID		(0x0105 << 16)
#define IP0105_MODULE_ID__MAJREV		(0xF << 12)
#define IP0105_MODULE_ID__MINREV		(0xF << 8)
#define IP0105_MODULE_ID__MODULE_APERTURE_SIZE	(0xFF << 0)

/*-----------------------------------------------------------------------
 * Initialization
 */
int nxp_i2c_ip0105_init (unsigned long bus_addr, int speed, int slaveaddr) ;

/*-----------------------------------------------------------------------
 * Probe to see if a chip is present.  Also good for checking for the
 * completion of EEPROM writes since the chip stops responding until
 * the write completes (typically 10mSec).
 */
int nxp_i2c_ip0105_probe(unsigned long bus_addr, uchar addr) ;

/*-----------------------------------------------------------------------
 * Read bytes
 */
int nxp_i2c_ip0105_read(unsigned long bus_addr, uchar chip, uint addr, int alen, uchar *buffer, int len) ;

/*-----------------------------------------------------------------------
 * Write bytes
 */
int nxp_i2c_ip0105_write(unsigned long bus_addr, uchar chip, uint addr, int alen, uchar *buffer, int len) ;
/*-----------------------------------------------------------------------
 * Read a register
 */
uchar nxp_i2c_ip0105_reg_read(unsigned long bus_addr, uchar i2c_addr, uchar reg) ;

/*-----------------------------------------------------------------------
 * Write a register
 */
void nxp_i2c_ip0105_reg_write(unsigned long bus_addr, uchar i2c_addr, uchar reg, uchar val) ;

/*-----------------------------------------------------------------------
 * Set bus speed
 */
int nxp_i2c_ip0105_set_speed(unsigned long bus_addr, unsigned int speed) ;

/*-----------------------------------------------------------------------
 * Get bus speed
 */
unsigned int nxp_i2c_ip0105_get_speed(unsigned long bus_addr) ;

#endif /* NXP_I2C_IP0105_H */
