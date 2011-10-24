/*
 * (C) Copyright 2003
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
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
 */

/*
 * This file contains the configuration parameters for the NXP
 * Silverbox, which is a variant of the Jaguar Backplane System (JBS.)
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#define CONFIG_SILVERBOX

#include <pnx8550_glb.h>

/* PNX8550 has a (modified) MIPS32 core */
#define CONFIG_MIPS32					/* MIPS32 CPU core */
#define CONFIG_MIPS_CPU_PR4450				/* Philips PR4450 implementation */

#define CONFIG_BOOTDELAY	5	/* autoboot after 5 seconds	*/

#define CONFIG_BAUDRATE		38400

/* valid baudrates */
#define CFG_BAUDRATE_TABLE	{ 50, 75, 110, 134.5, 150, 300, 600, 1200, 2400, \
				  4800, 9600, 19200, 38400, 57600, 115200, 230400 }

#define CONFIG_TIMESTAMP		/* Print image info with timestamp */
#undef	CONFIG_BOOTARGS

/*
#define CONFIG_EXTRA_ENV_SETTINGS					\
	"addmisc=setenv bootargs $(bootargs) "				\
		"console=ttyS0,38400n8 "				\
		"panic=1\0"						\
	""
*/

#define CONFIG_BOOTCOMMAND	"loadb 0x80100000;sleep 5;go 0x80100000"

#define CONFIG_COMMANDS	      (	(CONFIG_CMD_DFL		|\
				 CFG_CMD_PCI		|\
				 CFG_CMD_ELF		|\
				 CFG_CMD_I2C		|\
				 CFG_CMD_MEMORY		|\
				 CFG_CMD_EEPROM		|\
				 CFG_CMD_NET            |\
				 CFG_CMD_PING)		&\
 				~(CFG_CMD_ENV | CFG_CMD_FLASH | CFG_CMD_IMLS ) )
#include <cmd_confdefs.h>

/*
 * Miscellaneous configurable options
 */
#define CFG_LONGHELP				/* undef to save memory      */
#define CFG_PROMPT		"# "	/* Monitor Command Prompt    */
#define CFG_CBSIZE		256		/* Console I/O Buffer Size   */
#define CFG_PBSIZE		(CFG_CBSIZE + sizeof(CFG_PROMPT) + 16)	/* Print Buffer Size */
#define CFG_MAXARGS		16		/* max number of command args*/

#define CFG_MALLOC_LEN		128*1024

#define CFG_BOOTPARAMS_LEN	128*1024

#define CFG_CP0_COUNT_RATE	250000000	/* Equal to CPU core frequency */
#define CFG_HZ			1000

#define CFG_SDRAM_BASE		0x80000000     /* Cached addr */
#define CFG_LOAD_ADDR		0x82000000     /* load address of zImage  */
#define CFG_MEMTEST_START	0x80100000
#define CFG_MEMTEST_END		0x80800000

/*
 * Serial port configuration
 */
#define CFG_IP0107_UART		1		/* Has an IP0107 on SoC */
#define CFG_SERIAL_PORT_1	IP0107_1
#define CFG_SERIAL_PORT_2	IP0107_2
#define CFG_SERIAL_PORT_3	IP0107_3
#define CFG_CONSOLE_PORT	CFG_SERIAL_PORT_1
#define CFG_IP0107_CLOCK	3692300

/* environment organization */

/* The following #defines are needed to get flash environment right */
#define CFG_MONITOR_BASE	TEXT_BASE
#define CFG_MONITOR_LEN		(192 << 10)	/* No more than 192KiB */

#define CFG_INIT_SP_OFFSET	0x400000

/* Address and size of Primary Environment Sector	*/
#define CFG_NO_FLASH
#define CFG_ENV_IS_NOWHERE
#define CFG_ENV_ADDR		0x80104000
#define CFG_ENV_SIZE		0x00004000


/*-----------------------------------------------------------------------
 * Cache Configuration
 */
#define CFG_DCACHE_SIZE		16384
#define CFG_ICACHE_SIZE		16384
#define CFG_CACHELINE_SIZE	32

/*-----------------------------------------------------------------------
 * PCI stuff
 */
#define CONFIG_PCI
#define CONFIG_PCI_PNP
#define CONFIG_NXP_PCIXIO_IPA051

/*-----------------------------------------------------------------------
 * Network stuff
 */
#define CONFIG_NATSEMI
#define NATSEMI_DEBUG
#define CONFIG_NET_MULTI

#define CONFIG_SERVERIP		192.168.123.16		/* IP address of tftp server */
#define CONFIG_IPADDR		192.168.123.15		/* Our IP address */
#define CONFIG_NETMASK		255.255.255.0		/* Our net mask */
#define CONFIG_BOOTFILE		/vmlinux-sb		/* File to boot */
#define CONFIG_LOADADDR		0x82000000		/* SDRAM address to load files to */

/*-----------------------------------------------------------------------
 * I2C stuff
 */
#define CONFIG_NXP_I2C
#define CONFIG_NXP_I2C_IP0105
#define CONFIG_NXP_I2C_IP3203
#define CONFIG_HARD_I2C
#define CFG_I2C_SPEED			(150000)	/* 150kHz */
#define CFG_I2C_SLAVE			0
#define CFG_I2C_EEPROM_ADDR_LEN		1
#define CONFIG_I2C_CMD_TREE
#define CONFIG_I2C_MULTI_BUS

#endif	/* __CONFIG_H */
