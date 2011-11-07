/*
 * (C) Copyright 2003
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
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
 * This file contains the configuration parameters for the pollin stb810 board.
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#define CONFIG_MIPS32		1	/* MIPS32 CPU core	*/
#define CONFIG_MIPS_CPU_PR4450	1	/* Philips PR4450 implementation */
#define CONFIG_SYS_MIPS_CACHE_MODE CONF_CM_UNCACHED	/* run uncached, slow as hell, but natsemi breaks otherwise */

#define CONFIG_BOOTDELAY	2	/* autoboot after 2 seconds	*/

#define CONFIG_BAUDRATE		38400

/* valid baudrates */
#define CONFIG_SYS_BAUDRATE_TABLE	{ 50, 75, 110, 134.5, 150, 300, 600, 1200, 2400, \
				  	  4800, 9600, 19200, 38400, 57600, 115200, 230400 }

#define	CONFIG_TIMESTAMP		/* Print image info with timestamp */
#define	CONFIG_BOOTARGS			\
	"console=ttyS0 "		\
	"stb810_display=pal "		\
	"nomainapp=1 "			\
	""

#define	CONFIG_EXTRA_ENV_SETTINGS						\
	"la=80100000\0"								\
	"ld=loadb $(la);sleep 5;go $(la)\0"					\
	"rootpath=/pollinux/nandfs\0"						\
	"nfsargs=setenv bootargs $(bootargs) root=/dev/nfs rw "			\
		"nfsroot=$(serverip):$(rootpath)\0"				\
	"addip=setenv bootargs $(bootargs) ip=$(ipaddr)\0"			\
	"nfsboot=nfs \"192.168.123.16:/pollinux/nandfs/boot/uImage\";"		\
		"run fsargs addip;bootm\0"					\
	""

/* Boot from NFS root */
#define CONFIG_BOOTCOMMAND	"run nfsboot"

/*-----------------------------------------------------------------------
 * Miscellaneous configurable options
 */
#undef	CONFIG_SYS_LONGHELP				/* undef to save memory      */
#define	CONFIG_SYS_PROMPT		"pollinux> "	/* Monitor Command Prompt    */
#define	CONFIG_SYS_CBSIZE		256		/* Console I/O Buffer Size   */
#define	CONFIG_SYS_PBSIZE (CONFIG_SYS_CBSIZE+sizeof(CONFIG_SYS_PROMPT)+16)  /* Print Buffer Size */
#define	CONFIG_SYS_MAXARGS		16		/* max number of command args*/

#define CONFIG_SYS_MALLOC_LEN		128*1024
#define CONFIG_SYS_BOOTPARAMS_LEN	128*1024

#define CONFIG_SYS_MIPS_TIMER_FREQ	250000000
#define CONFIG_SYS_HZ			1000
#define CONF_SLOWDOWN_IO

#define CONFIG_SYS_MONITOR_BASE		CONFIG_SYS_TEXT_BASE
#define CONFIG_SYS_MONITOR_LEN		(192 << 10)	/* No more than 192KiB */

#define CONFIG_SYS_INIT_SP_OFFSET	0x400000

#define CONFIG_SYS_SDRAM_BASE		0x80000000	/* Cached addr */
#define	CONFIG_SYS_LOAD_ADDR		0x82000000	/* default load address	*/
#define CONFIG_SYS_MEMTEST_START	0x80100000
#define CONFIG_SYS_MEMTEST_END		0x80800000

/*-----------------------------------------------------------------------
 * Flash and environment organization
 */
#define CONFIG_SYS_NO_FLASH

#define	CONFIG_ENV_IS_NOWHERE	1
/* Address and size of Primary Environment Sector	*/
#define CONFIG_ENV_ADDR		0xB0030000
#define CONFIG_ENV_SIZE		0x10000


/*-----------------------------------------------------------------------
 * Serial configuration
 */
#define CONFIG_IP0107_SERIAL			/* Has an IP0107 on SoC */
#define CONFIG_SERIAL_PORT_1	IP0107_1
#define CONFIG_SERIAL_PORT_2	IP0107_2
#define CONFIG_SERIAL_PORT_3	IP0107_3
#define CONFIG_CONSOLE_PORT	CONFIG_SERIAL_PORT_1
#define CONFIG_IP0107_CLOCK	3692300


/*-----------------------------------------------------------------------
 * USB configuration
 */
#if 0
#define CONFIG_CMD_USB

#define CONFIG_USB_OHCI_NEW
#define CONFIG_SYS_USB_OHCI_MAX_ROOT_PORTS	15
#define CONFIG_PCI_OHCI
#define CONFIG_PCI_EHCI_DEVNO			0
#define CONFIG_SYS_USB_OHCI_SLOT_NAME		"ohci_pci"
#endif

/*-----------------------------------------------------------------------
 * PCI configuration
 */
#define	CONFIG_PCI
#undef	CONFIG_PCI_SCAN_SHOW
#define	CONFIG_IPA051_PCIXIO

/*-----------------------------------------------------------------------
 * SATA configuration
 */
#define CONFIG_SATA_SIL3512
#define CONFIG_SYS_SATA_MAX_DEVICE	2
#define CONFIG_LIBATA
#define CONFIG_LBA48
#define CONFIG_DOS_PARTITION

/*-----------------------------------------------------------------------
 * Network configuration
 */
#define CONFIG_NATSEMI

#define CONFIG_PREBOOT		"echo;echo Welcome to pollinux board v1.1;echo"
#define CONFIG_IPADDR		192.168.123.15
#define CONFIG_ETHADDR		00:00:00:00:00:00
#define CONFIG_SERVERIP		192.168.123.16
#define CONFIG_NETMASK		255.255.255.0
#define CONFIG_HOSTNAME		"pollinux"
#define CONFIG_BOOTFILE		"/pollinux/nandfs/boot/uImage"	/* File to boot */

/*-----------------------------------------------------------------------
 * I2C Configuration
 */
#if 0
#define CONFIG_HARD_I2C			/* To enable I2C support	*/
#define CONFIG_NXP_I2C
#define CONFIG_NXP_I2C_IP0105
#define CONFIG_NXP_I2C_IP3203
#define CONFIG_SYS_I2C_SPEED		(150000)	/* I2C speed and slave address	*/
#define CONFIG_SYS_I2C_SLAVE		0
#define CONFIG_SYS_EEPROM_ADDR_LEN	1
#define CONFIG_I2C_CMD_TREE
#define CONFIG_I2C_MULTI_BUS
#endif

/*-----------------------------------------------------------------------
 * Cache Configuration
 */
#define CONFIG_SYS_DCACHE_SIZE		16384
#define CONFIG_SYS_ICACHE_SIZE		16384
#define CONFIG_SYS_CACHELINE_SIZE	32

/*-----------------------------------------------------------------------
 * Command line configuration.
 */
#include <config_cmd_default.h>

#define CONFIG_CMD_ELF
#define CONFIG_CMD_MII
#define CONFIG_CMD_PING
#define CONFIG_CMD_PCI
/* SATA stuff */
#define CONFIG_CMD_SATA
#define CONFIG_CMD_EXT2

#undef CONFIG_CMD_FPGA
#undef CONFIG_CMD_SAVEENV
#undef CONFIG_CMD_FLASH
#undef CONFIG_CMD_IDE
#undef CONFIG_CMD_LOADS

#endif	/* __CONFIG_H */
