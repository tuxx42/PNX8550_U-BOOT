/*
 * Copyright (C) 2005 Koninklijke Philips Electronics N.V.
 * All Rights Reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */
/*
Rev Date       Author      Comments
--------------------------------------------------------------------------------
  6 20051210   burningh    Adding support for devices larger than 32MB
*/
/*
 * Based on:
 *  drivers/mtd/nand/pnx8550.c
 * Torbjorn Lundberg
 * $Id: pnx8550_nand.c,v 1.8 2004/11/12 10:46:58 tobbe Exp $
 *
 *  Overview:
 *   This is a device driver for NAND flash devices used with the PNX8550.
 *   It can currently cope with 8 and 16 bit devices up to 64MByte in size.
 */

#include <common.h>
#include <malloc.h>
#include <linux/string.h>
#include <linux/types.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/nand.h>
#include <linux/mtd/nand_ecc.h>
#include <linux/mtd/partitions.h>
#include <asm/io.h>
#include <asm/errno.h>
#include <asm/mach-pnx8550/nand.h>

/******************************************************************************
* LOCAL MACROS                                                                *
*******************************************************************************/

#define THIS_MODULE_DESCRIPTION "Driver for NAND Flash on the PNX8550 XIO bus"
#define DEVNAME "pnx8550_nand"

#define ERROR(fmt, arg...) printk(KERN_ERR DEVNAME "(%s): " fmt "\n", __FUNCTION__,  ## arg )
#define INFO(fmt, arg...)  printk(KERN_INFO DEVNAME ": " fmt "\n", ## arg )

#undef DEBUG
#ifdef DEBUG
#  undef DEBUG
#  define DEBUG(fmt, arg...) printk(KERN_DEBUG DEVNAME "(%s): " fmt "\n", __FUNCTION__, ##  arg )
#else
#  define DEBUG( fmt, arg...)
#endif

#define XIO_WAIT_TIMEOUT	200000	// 2s wait for xio ack (in 10 us)
#define DMA_WAIT_TIMEOUT	3000000	// 30s wair for dma (in 10 us)

/* NB. USER_SIZE is just used as a default value, which gets overwritten after the device has been recognised during module_init */

#define NAND_ADDR(_col, _page) ((_col) & (mtd->writesize - 1)) + ((_page) << this->page_shift)

#define NAND_ADDR_SEND(_addr) pNandAddr[(_addr)/sizeof(u16)] = 0

#define NAND_TRANSFER_TO(_addr, _buffer, _bytes) pnx8550_nand_transfer((_buffer), ((u8*)pNandAddr) + (_addr), (_bytes), 1)

#define NAND_TRANSFER_FROM(_addr, _buffer, _bytes) pnx8550_nand_transfer(((u8*)pNandAddr) + (_addr), (_buffer), (_bytes), 0)

/******************************************************************************
* STATIC FUNCTION PROTOTYPES                                                  *
*******************************************************************************/

static void pnx8550_nand_register_setup(u_char cmd_no, u_char addr_no,
                 u_char include_data, u_char monitor_ACK, u_char enable64M,
                 int cmd_a, int cmd_b);

static inline void pnx8550_nand_wait_for_dev_ready(void);

static void pnx8550_nand_transfer(void *from, void *to, int bytes, int toxio);

static void pnx8550_nand_transferDMA(void *from, void *to, int bytes, int toxio);

/******************************************************************************
* LOCAL TYPEDEFS                                                              *
*******************************************************************************/

/******************************************************************************
* EXPORTED DATA                                                               *
*******************************************************************************/

/******************************************************************************
* LOCAL DATA                                                                  *
*******************************************************************************/

/* Bad block descriptor for 16Bit nand flash */
static uint8_t scan_ff_pattern[] = { 0xff, 0xff };
static struct nand_bbt_descr nand16bit_memorybased = {
    .options = 0,
    .offs = 0,
    .len = 2,
    .pattern = scan_ff_pattern
};

/* OOB Placement information that lines up with the boot loader code */
static struct nand_ecclayout nand16bit_oob_16 = {
    .eccbytes = 6,
    .eccpos = {2, 3, 4, 5, 6, 7},
    .oobfree = { {8, 8} }
};

/* Pointer into XIO for access to the NAND flash device */
static volatile u16 *pNandAddr;

/* Last command sent to the pnx8550_nand_command function */
static int last_command   = -1;
/*
  Next column address to read/write, set by pnx8550_nand_command
  updated by the read/write functions
*/
static int last_col_addr  = -1;
/*
  Next page address to read/write, set by pnx8550_nand_command
  updated by the read/write functions
*/
static int last_page_addr = -1;

/*
    32bit Aligned/DMA buffer
*/
static u_char *transferBuffer = NULL;

/*
    Whether the device is on a 16bit bus or not.
*/
static int is16bit = 0;

/*
    Whether the device is >= 64MB or not.
*/
static int is64mb = 0;

/* bad block descriptor located in the "middle" of the flash
 *  this is pretty evil, but since the end is used by the microBTM we don't
 *  have a real choice here
 */
#ifdef CONFIG_NAND_BBT

static u8 bbt_pattern[] = {'B', 'b', 't', '0' };
static u8 mirror_pattern[] = {'1', 't', 'b', 'B' };

static struct nand_bbt_descr nand_main_bbt_decr = {
    .options = NAND_BBT_ABSPAGE | NAND_BBT_CREATE | NAND_BBT_WRITE |
			NAND_BBT_2BIT | NAND_BBT_VERSION,
	.pages[0] = 0x460,
    .offs = 1,
    .len = 4,
    .pattern = bbt_pattern
};

static struct nand_bbt_descr nand_mirror_bbt_decr = {
    .options = NAND_BBT_ABSPAGE | NAND_BBT_CREATE | NAND_BBT_WRITE |
			NAND_BBT_2BIT | NAND_BBT_VERSION,
	.pages[0] = 0x480,
    .offs = 1,
    .len = 4,
    .pattern = mirror_pattern
};

#endif

#ifdef CONFIG_NAND_WINCE_ECC
// wince oob placement
static struct nand_ecclayout nand8bit_oob_wince = {
    .eccbytes = 6,
    .eccpos = {8, 9, 10, 11, 12, 13},
    .oobfree = { {0, 8} },
};

static u_char eccArray[] = {
		0x00, 0x01, 0x01, 0x02, 0x01, 0x02, 0x02, 0x03, 0x01, 0x02, 0x02, 0x03, 0x02, 0x03, 0x03, 0x04,
		0x01, 0x02, 0x02, 0x03, 0x02, 0x03, 0x03, 0x04, 0x02, 0x03, 0x03, 0x04, 0x03, 0x04, 0x04, 0x05,
		0x01, 0x02, 0x02, 0x03, 0x02, 0x03, 0x03, 0x04, 0x02, 0x03, 0x03, 0x04, 0x03, 0x04, 0x04, 0x05,
		0x02, 0x03, 0x03, 0x04, 0x03, 0x04, 0x04, 0x05, 0x03, 0x04, 0x04, 0x05, 0x04, 0x05, 0x05, 0x06,
		0x01, 0x02, 0x02, 0x03, 0x02, 0x03, 0x03, 0x04, 0x02, 0x03, 0x03, 0x04, 0x03, 0x04, 0x04, 0x05,
		0x02, 0x03, 0x03, 0x04, 0x03, 0x04, 0x04, 0x05, 0x03, 0x04, 0x04, 0x05, 0x04, 0x05, 0x05, 0x06,
		0x02, 0x03, 0x03, 0x04, 0x03, 0x04, 0x04, 0x05, 0x03, 0x04, 0x04, 0x05, 0x04, 0x05, 0x05, 0x06,
		0x03, 0x04, 0x04, 0x05, 0x04, 0x05, 0x05, 0x06, 0x04, 0x05, 0x05, 0x06, 0x05, 0x06, 0x06, 0x07,
		0x01, 0x02, 0x02, 0x03, 0x02, 0x03, 0x03, 0x04, 0x02, 0x03, 0x03, 0x04, 0x03, 0x04, 0x04, 0x05,
		0x02, 0x03, 0x03, 0x04, 0x03, 0x04, 0x04, 0x05, 0x03, 0x04, 0x04, 0x05, 0x04, 0x05, 0x05, 0x06,
		0x02, 0x03, 0x03, 0x04, 0x03, 0x04, 0x04, 0x05, 0x03, 0x04, 0x04, 0x05, 0x04, 0x05, 0x05, 0x06,
		0x03, 0x04, 0x04, 0x05, 0x04, 0x05, 0x05, 0x06, 0x04, 0x05, 0x05, 0x06, 0x05, 0x06, 0x06, 0x07,
		0x02, 0x03, 0x03, 0x04, 0x03, 0x04, 0x04, 0x05, 0x03, 0x04, 0x04, 0x05, 0x04, 0x05, 0x05, 0x06,
		0x03, 0x04, 0x04, 0x05, 0x04, 0x05, 0x05, 0x06, 0x04, 0x05, 0x05, 0x06, 0x05, 0x06, 0x06, 0x07,
		0x03, 0x04, 0x04, 0x05, 0x04, 0x05, 0x05, 0x06, 0x04, 0x05, 0x05, 0x06, 0x05, 0x06, 0x06, 0x07,
		0x04, 0x05, 0x05, 0x06, 0x05, 0x06, 0x06, 0x07, 0x05, 0x06, 0x06, 0x07, 0x06, 0x07, 0x07, 0x08,
		0x00, 0x01, 0x04, 0x05, 0x10, 0x11, 0x14, 0x15, 0x40, 0x41, 0x44, 0x45, 0x50, 0x51, 0x54, 0x55,
		0x00, 0x00, 0x01, 0x01, 0x00, 0x00, 0x01, 0x01, 0x02, 0x02, 0x03, 0x03, 0x02, 0x02, 0x03, 0x03,
		0x00, 0x00, 0x01, 0x01,	0x00, 0x00, 0x01, 0x01, 0x02, 0x02, 0x03, 0x03, 0x02, 0x02, 0x03, 0x03,
		0x04, 0x04, 0x05, 0x05,	0x04, 0x04, 0x05, 0x05, 0x06, 0x06, 0x07, 0x07,	0x06, 0x06, 0x07, 0x07,
		0x04, 0x04, 0x05, 0x05, 0x04, 0x04, 0x05, 0x05, 0x06, 0x06, 0x07, 0x07, 0x06, 0x06, 0x07, 0x07,
		0x00, 0x00, 0x01, 0x01,	0x00, 0x00, 0x01, 0x01, 0x02, 0x02, 0x03, 0x03, 0x02, 0x02, 0x03, 0x03,
		0x00, 0x00, 0x01, 0x01, 0x00, 0x00, 0x01, 0x01, 0x02, 0x02, 0x03, 0x03,	0x02, 0x02, 0x03, 0x03,
		0x04, 0x04, 0x05, 0x05, 0x04, 0x04, 0x05, 0x05, 0x06, 0x06, 0x07, 0x07, 0x06, 0x06, 0x07, 0x07,
		0x04, 0x04, 0x05, 0x05,	0x04, 0x04, 0x05, 0x05, 0x06, 0x06, 0x07, 0x07, 0x06, 0x06, 0x07, 0x07,
		0x08, 0x08, 0x09, 0x09, 0x08, 0x08, 0x09, 0x09, 0x0A, 0x0A, 0x0B, 0x0B, 0x0A, 0x0A, 0x0B, 0x0B,
		0x08, 0x08, 0x09, 0x09, 0x08, 0x08, 0x09, 0x09, 0x0A, 0x0A, 0x0B, 0x0B, 0x0A, 0x0A, 0x0B, 0x0B,
		0x0C, 0x0C, 0x0D, 0x0D, 0x0C, 0x0C, 0x0D, 0x0D, 0x0E, 0x0E, 0x0F, 0x0F, 0x0E, 0x0E, 0x0F, 0x0F,
		0x0C, 0x0C, 0x0D, 0x0D, 0x0C, 0x0C, 0x0D, 0x0D, 0x0E, 0x0E, 0x0F, 0x0F, 0x0E, 0x0E, 0x0F, 0x0F,
		0x08, 0x08, 0x09, 0x09, 0x08, 0x08, 0x09, 0x09,	0x0A, 0x0A, 0x0B, 0x0B, 0x0A, 0x0A, 0x0B, 0x0B,
		0x08, 0x08, 0x09, 0x09, 0x08, 0x08, 0x09, 0x09, 0x0A, 0x0A, 0x0B, 0x0B, 0x0A, 0x0A, 0x0B, 0x0B,
		0x0C, 0x0C, 0x0D, 0x0D, 0x0C, 0x0C, 0x0D, 0x0D, 0x0E, 0x0E, 0x0F, 0x0F, 0x0E, 0x0E, 0x0F, 0x0F,
		0x0C, 0x0C, 0x0D, 0x0D, 0x0C, 0x0C, 0x0D, 0x0D, 0x0E, 0x0E, 0x0F, 0x0F, 0x0E, 0x0E, 0x0F, 0x0F,
		0x52, 0x53, 0x44, 0x53, 0x67, 0x98, 0xE5, 0x1F, 0x71, 0x94, 0x75, 0x45, 0x9F, 0x77, 0x16, 0x55,
		0xA8, 0xCD, 0x45, 0x06, 0x11, 0x00, 0x00, 0x00,
};

#endif

/******************************************************************************
* FUNCTION IMPLEMENTATION                                                     *
*******************************************************************************/

/**
 * Allocate the buffer used to tranfser data between flash and memory.
 * We use a transfer buffer so we know that the buffer is 32bit aligned.
 */
static void inline pnx8550_nand_alloc_transfer_buffer(struct mtd_info *mtd)
{
    if (transferBuffer == NULL)
    {
	// We want to ensure that we don't share any cache lines with any other memory block.
	size_t alloc_size = (mtd->writesize + mtd->oobsize + CONFIG_SYS_CACHELINE_SIZE - 1) & ~(CONFIG_SYS_CACHELINE_SIZE-1);
	BUG_ON(alloc_size < mtd->writesize + mtd->oobsize);
        transferBuffer = memalign(CONFIG_SYS_CACHELINE_SIZE, alloc_size);
        if (transferBuffer == NULL)
        {
            ERROR("Failed to allocate transfer buffer!");
        }
        else
        {
	    // Make sure we've been allocated cache aligned (SLOB
	    // is broken and hands out non-aligned addresses).
	    BUG_ON(((unsigned long)transferBuffer) & (CONFIG_SYS_CACHELINE_SIZE-1));

	    // Ensure that any pending cache writes to our memory block are flushed before we start using it.
//	    dma_cache_wback_inv((unsigned long)transferBuffer, alloc_size);
	    
            transferBuffer = (u_char *)(((__u32)transferBuffer - KSEG0) + KSEG1);
            DEBUG("transferBuffer allocated at %p", transferBuffer);
        }
    }
}
/**
 * Transfer data to/from the NAND chip.
 * This function decides whether to use DMA or not depending on
 * the amount of data to transfer and the alignment of the buffers.
 *
 * @from:  Address to transfer data from
 * @to:    Address to transfer the data to
 * @bytes: Number of bytes to transfer
 * @toxio: Whether the transfer is going to XIO or not.
 */
static void pnx8550_nand_transfer(void *from, void *to, int bytes, int toxio)
{
	u32 timeout = 0;
    u16 *from16 = (u16*)from;
    u16 *to16 = (u16*)to;
    int error = 0;
    int i;
    if (((u32)from & 3) && ((u32)to & 3))
    {
        INFO("Both from and to buffers are not 32bit aligned, will not use fastest transfer mechanism");
        error = 1;
    }
    else
    {
        if ((u32)from & 3)
        {
            INFO("from buffer not 32bit aligned, will not use fastest transfer mechanism");
            error = 1;
        }
        if ((u32)to & 3)
        {
            INFO("to buffer not 32bit aligned, will not use fastest transfer mechanism");
            error = 1;
        }
    }
    if (error)
    {
        DEBUG("NAND Base address: %p", pNandAddr);
        DEBUG("From : %p To : %p bytes: %d toxio:%d", from, to, bytes, toxio);
        DEBUG("Last Command : %02x Page: %d Column:%d", last_command, last_page_addr, last_col_addr);
    }
    if (((bytes & 3) || (bytes < 16)) || ((u32)to & 3) || ((u32)from & 3))
    {
        if (((bytes & 1) == 0) &&
            (((u32)to & 1) == 0) &&
            (((u32)from & 1) == 0))
        {
            int words = bytes / 2;

            for (i = 0; i < words; i ++)
            {
                to16[i] = from16[i];
            }
        }
        else
        {
            if (is16bit)
            {
                ERROR("Transfer failed, byte-aligned transfers not allowed on a 16Bit bus!");
            }
            else
            {
                for (i = 0; i < bytes; i ++)
                {
                    ((u_char*)to)[i] = ((u_char*)from)[i];
                }
            }
        }
    }
    else
    {
        pnx8550_nand_transferDMA(from, to, bytes, toxio);
    }
    while((PNX8550_GPXIO_INT_STATUS & PNX8550_XIO_INT_ACK) == 0)
    {
    	udelay(10);
    	timeout++;
    	if(timeout > XIO_WAIT_TIMEOUT)
    	{
    		// we waited for too long on the Flash chip
    		ERROR("Timeout on XIO ACK wait!");
    		BUG();
    	}
    }
}

/**
 * Transfer data to/from the NAND chip using DMA
 *
 * @from:  Address to transfer data from
 * @to:    Address to transfer the data to
 * @bytes: Number of bytes to transfer
 * @toxio: Whether the transfer is going to XIO or not.
 */
static void pnx8550_nand_transferDMA(void *from, void *to, int bytes, int toxio)
{
    int cmd = 0;
    u32 timeout = 0;
    u32 internal;
    u32 external;

    if (toxio)
    {
        cmd = PNX8550_DMA_CTRL_PCI_CMD_WRITE;
        internal = (u32)virt_to_phys(from);
        external = (u32)to - KSEG1;
    }
    else
    {
        cmd = PNX8550_DMA_CTRL_PCI_CMD_READ;
        internal = (u32)virt_to_phys(to);
        external = (u32)from - KSEG1;
    }

    PNX8550_DMA_TRANS_SIZE = bytes >> 2; /* Length in words */
    PNX8550_DMA_EXT_ADDR   = external;
    PNX8550_DMA_INT_ADDR   = internal;
    PNX8550_DMA_INT_CLEAR  = 0xffff;
    PNX8550_DMA_CTRL       = PNX8550_DMA_CTRL_BURST_128 |
                             PNX8550_DMA_CTRL_SND2XIO   |
                             PNX8550_DMA_CTRL_INIT_DMA  |
                             cmd;

    while((PNX8550_DMA_INT_STATUS & PNX8550_DMA_INT_COMPL) == 0)
    {
    	udelay(10);
    	timeout++;
    	if(timeout > DMA_WAIT_TIMEOUT)
    	{
    		// we waited for too long on the Flash chip
    		ERROR("Timeout on DMA complete wait!");
    		BUG();
    	}
    }
}

/**
 * pnx8550_nand_read_byte_16bit - read one byte endianess aware from the chip
 * @mtd:    MTD device structure
 *
 */
static u_char pnx8550_nand_read_byte_16bit(struct mtd_info *mtd)
{
    struct nand_chip *this = mtd->priv;
    u16 data = 0;
    int addr = NAND_ADDR(last_col_addr, last_page_addr);
    DEBUG("last_col_addr=%d last_page_addr=0x%x", last_col_addr, last_page_addr);
    /*
        Read ID is a special case as we have to read BOTH bytes at the same
        time otherwise it doesn't work, once we have both bytes we work out
        which one we want.
    */
    if (last_command == NAND_CMD_READID)
    {
        u32 *pNandAddr32 = (u32 *)pNandAddr;
        u32 data32;
        data32 = cpu_to_le32(pNandAddr32[0]);
        if (last_col_addr)
        {
            data = (u16)(data32 >> 16);
        }
        else
        {
            data = (u16)data32;
        }
    }
    else
    {
        data = cpu_to_le16(pNandAddr[(addr / sizeof(u16))]);
        if ((addr & 0x1) == 1)
        {
            data = (data & 0xff00) >> 16;
        }
    }
    /*
       Status is a special case, we don't need to increment the address
       because the address isn't used by the chip
    */
    if (last_command != NAND_CMD_STATUS)
    {
        last_col_addr ++;
    }
    return data & 0xff;
}



/**
 * pnx8550_nand_write_byte_16bit - write one byte endianess aware to the chip
 * @mtd:    MTD device structure
 * @byte:   pointer to data byte to write
 *
 * Write function for 16bit buswith with
 * endianess conversion
 */
static void pnx8550_nand_write_byte_16bit(struct mtd_info *mtd, u_char byte)
{
    struct nand_chip *this = mtd->priv;
    int addr = NAND_ADDR(last_col_addr, last_page_addr);
    DEBUG("last_col_addr=%d last_page_addr=0x%x byte=0x%x", last_col_addr, last_page_addr, byte & 0xff);
    pNandAddr[(addr / sizeof(u16))] = le16_to_cpu((u16) byte);
}

/**
 * pnx8550_nand_read_byte_8bit - read one byte endianess aware from the chip
 * @mtd:    MTD device structure
 *
 */
static u_char pnx8550_nand_read_byte_8bit(struct mtd_info *mtd)
{
    struct nand_chip *this = mtd->priv;
    u_char data = 0;
    int addr = NAND_ADDR(last_col_addr, last_page_addr);
    DEBUG("last_col_addr=%d last_page_addr=0x%x", last_col_addr, last_page_addr);
    /*
        Read ID is a special case as we have to read BOTH bytes at the same
        time otherwise it doesn't work, once we have both bytes we work out
        which one we want.
    */
    if (last_command == NAND_CMD_READID)
    {
        u16 data16;
        data16 = cpu_to_le16(pNandAddr[0]);
        if (last_col_addr)
        {
            data = (u_char)(data16 >> 8);
        }
        else
        {
            data = (u_char)data16;
        }
    }
    else
    {
        data = ((u_char*)pNandAddr)[addr];
    }
    /*
       Status is a special case, we don't need to increment the address
       because the address isn't used by the chip
    */
    if (last_command != NAND_CMD_STATUS)
    {
        last_col_addr ++;
    }
    return data & 0xff;
}



/**
 * pnx8550_nand_write_byte_8bit - write one byte endianess aware to the chip
 * @mtd:    MTD device structure
 * @byte:   pointer to data byte to write
 */
static void pnx8550_nand_write_byte_8bit(struct mtd_info *mtd, u_char byte)
{
    struct nand_chip *this = mtd->priv;
    int addr = NAND_ADDR(last_col_addr, last_page_addr);
    DEBUG("last_col_addr=%d last_page_addr=0x%x byte=0x%x", last_col_addr, last_page_addr, byte & 0xff);
    ((u_char*)pNandAddr)[addr] = byte;
}
/**
 * pnx8550_nand_read_word - read one word from the chip
 * @mtd:    MTD device structure
 *
 * Read function for 16bit buswith without
 * endianess conversion
 */
static u16 pnx8550_nand_read_word(struct mtd_info *mtd)
{
    struct nand_chip *this = mtd->priv;
    int addr = NAND_ADDR(last_col_addr, last_page_addr);
    u16 data = pNandAddr[(addr / sizeof(u16))];
    DEBUG("last_col_addr=%d last_page_addr=0x%x", last_col_addr, last_page_addr);
    return  data;
}

/**
 * pnx8550_nand_write_word - write one word to the chip
 * @mtd:    MTD device structure
 * @word:   data word to write
 *
 * Write function for 16bit buswith without
 * endianess conversion
 */
static void pnx8550_nand_write_word(struct mtd_info *mtd, u16 word)
{
    struct nand_chip *this = mtd->priv;
    int addr = NAND_ADDR(last_col_addr, last_page_addr);
    DEBUG("last_col_addr=%d last_page_addr=0x%x word=0x%x", last_col_addr, last_page_addr, word & 0xffff);
    pNandAddr[(addr / sizeof(u16))] = word;
}

/**
 * pnx8550_nand_write_buf - write buffer to chip
 * @mtd:    MTD device structure
 * @buf:    data buffer
 * @len:    number of bytes to write
 *
 */
static void pnx8550_nand_write_buf(struct mtd_info *mtd, const u_char * buf, int len)
{
    struct nand_chip *this = mtd->priv;
    int addr = NAND_ADDR(last_col_addr, last_page_addr);
    int pageLen;
    int oobLen = 0;
    u_char *transBuf = (u_char *)buf;
    DEBUG("last_col_addr=%d last_page_addr=0x%x len=%d", last_col_addr, last_page_addr, len);
    /* some sanity checking, word access only please */
    if (len&1)
    {
        ERROR("non-word aligned length requested!");
    }

    pnx8550_nand_alloc_transfer_buffer(mtd);
    if (transferBuffer)
    {
        memcpy(transferBuffer, buf, len);
        transBuf = transferBuffer;
    }

    /*
        Work out whether we are going to write to the OOB area
        after a standard page write.
        This is not the case when the command function is called
        with a column address > page size. Then we write as though
        it is to the page rather than the OOB as the command function
        has already selected the OOB area.
    */
    if ((last_col_addr + len) > mtd->writesize)
        oobLen = (last_col_addr + len) - mtd->writesize;
    pageLen = len - oobLen;

    /* Clear the done flag */
    PNX8550_GPXIO_CTRL |= PNX8550_GPXIO_CLR_DONE;

    if (pageLen > 0)
    {
        NAND_TRANSFER_TO(addr, transBuf, pageLen);
    }

    if (oobLen > 0)
    {
        pnx8550_nand_wait_for_dev_ready();

        pnx8550_nand_register_setup(1, 0, 0, 1, is64mb, NAND_CMD_READOOB, 0);
        /* Work out where in the OOB we are going to start to write */
        addr = NAND_ADDR(last_col_addr - mtd->writesize, last_page_addr);
        NAND_ADDR_SEND(addr);
        pnx8550_nand_register_setup(2, 3, 1, 1, is64mb, NAND_CMD_SEQIN, NAND_CMD_PAGEPROG);

        /* Clear the done flag */
        PNX8550_GPXIO_CTRL |= PNX8550_GPXIO_CLR_DONE;
        NAND_TRANSFER_TO(addr, transBuf + pageLen, oobLen);
    }

    /*
       Increment the address so on the next write we write in the
       correct place.
    */
    last_col_addr += len;
    if (last_col_addr >= mtd->writesize + mtd->oobsize)
    {
        last_col_addr -=mtd->writesize + mtd->oobsize;
        last_page_addr ++;
    }
}

/**
 * pnx8550_nand_read_buf - read chip data into buffer
 * @mtd:    MTD device structure
 * @buf:    buffer to store date
 * @len:    number of bytes to read
 *
 */
static void pnx8550_nand_read_buf(struct mtd_info *mtd, u_char * buf, int len)
{
    struct nand_chip *this = mtd->priv;
    int addr = NAND_ADDR(last_col_addr, last_page_addr);
    int pageLen;
    int oobLen = 0;
    u_char *transBuf = buf;
    DEBUG("last_col_addr=%d last_page_addr=0x%x addr=0x%x len=%d", last_col_addr, last_page_addr, addr, len);
    /* some sanity checking, word access only please */
    if (len&1)
    {
        ERROR("non-word aligned length");
    }

    pnx8550_nand_alloc_transfer_buffer(mtd);
    if (transferBuffer)
    {
        transBuf = transferBuffer;
    }


    /*
        Work out whether we are going to read the OOB area
        after a standard page read.
        This is not the case when the command function is called
        with a column address > page size. Then we read as though
        it is from the page rather than the OOB as the command
        function has already selected the OOB area.
    */
    if ((last_col_addr + len) > mtd->writesize)
        oobLen = (last_col_addr + len) - mtd->writesize;

    pageLen = len - oobLen;
    if (pageLen)
    {
        NAND_TRANSFER_FROM(addr, transBuf, pageLen);
    }

    if (oobLen > 0)
    {
        pnx8550_nand_register_setup(1, 3, 1, 1, is64mb, NAND_CMD_READOOB, 0);
        addr = NAND_ADDR(last_col_addr - mtd->writesize, last_page_addr);
        NAND_TRANSFER_FROM(addr, transBuf + pageLen, oobLen);
    }

    if (transBuf != buf)
    {
        memcpy(buf, transBuf, len);
    }

    /*
       Increment the address so on the next read we read from the
       correct place.
    */
    last_col_addr += len;
    if (last_col_addr > mtd->writesize + mtd->oobsize)
    {
        last_col_addr -=mtd->writesize + mtd->oobsize;
        last_page_addr ++;
    }

    return;
}
/**
 * pnx8550_nand_verify_buf -  Verify chip data against buffer
 * @mtd:    MTD device structure
 * @buf:    buffer containing the data to compare
 * @len:    number of bytes to compare
 *
 */
static int pnx8550_nand_verify_buf(struct mtd_info *mtd, const u_char *buf, int len)
{
    int result = 0;
    DEBUG("last_col_addr=%d last_page_addr=0x%x len=%d", last_col_addr, last_page_addr, len);
    pnx8550_nand_alloc_transfer_buffer(mtd);
    if (transferBuffer == NULL)
    {
        return -ENOMEM;
    }
    /* some sanity checking, word access only please */
    if (len&1)
    {
        ERROR("non-word aligned length");
    }
    pnx8550_nand_read_buf(mtd, transferBuffer, len);
    if (memcmp(buf, transferBuffer, len))
    {
        result = -EFAULT;
    }

    return result;

}

/**
 * pnx8550_nand_command - Send command to NAND device
 * @mtd:    MTD device structure
 * @command:    the command to be sent
 * @column: the column address for this command, -1 if none
 * @page_addr:  the page address for this command, -1 if none
 *
 * Send command to NAND device.
 */
static void pnx8550_nand_command(struct mtd_info *mtd, unsigned command,
                 int column, int page_addr)
{
    register struct nand_chip *this = mtd->priv;
    u_char addr_no = 0;
    int addr;
    DEBUG("command=0x%02x column=%d page_addr=0x%x",command, column, page_addr);
    /*
       If we are starting a write work out whether it is to the
       OOB or the main page and position the pointer correctly.
    */
    if (command == NAND_CMD_SEQIN) {
        int readcmd;
        int col = column;
        if (column >= mtd->writesize) {
            /* OOB area */
            col -= mtd->writesize;
            readcmd = NAND_CMD_READOOB;
        } else {
            readcmd = NAND_CMD_READ0;
        }
        pnx8550_nand_register_setup(1, 0, 0, 1, is64mb, readcmd, 0);
        addr = NAND_ADDR(col, page_addr);
        NAND_ADDR_SEND(addr);
    }

    /* Check the number of address bytes */
    if ((column == -1) && (page_addr == -1)) {
        addr_no = 0;
        column = 0;
        page_addr = 0;
    } else if ((column == -1) && (page_addr != -1)) {
        addr_no = 2;
        column = 0;
    } else if ((column != -1) && (page_addr == -1)) {
        addr_no = 1;
        page_addr = 0;
    } else {
        /* NB. This also allows for commands on 64MB devices which require 4 address bytes.
           In that case the "is64mb" flag is used as a discriminator. */
        addr_no = 3;
    }

    last_command   = command;
    last_col_addr  = column;
    last_page_addr = page_addr;

    switch (command) {

    case NAND_CMD_PAGEPROG:
        // Nothing to do, we've already done it!
        return;

    case NAND_CMD_SEQIN:
        if (addr_no != 3)
            ERROR("Error. Command %02x needs 3 byte address, but addr_no = %d", command, addr_no);
        pnx8550_nand_register_setup(2, 3, 1, 1, is64mb, NAND_CMD_SEQIN, NAND_CMD_PAGEPROG);
        return;

    case NAND_CMD_ERASE1:
        if (addr_no != 2)
            ERROR("Error. Command %02x needs 2 byte address, but addr_no = %d", command, addr_no);

        PNX8550_GPXIO_CTRL |= PNX8550_GPXIO_CLR_DONE;

        pnx8550_nand_register_setup(2, 2, 0, 1, is64mb, NAND_CMD_ERASE1, NAND_CMD_ERASE2);
        addr = NAND_ADDR(column, page_addr);
        NAND_ADDR_SEND(addr);
        return;

    case NAND_CMD_ERASE2:
        // Nothing to do, we've already done it!
        return;

    case NAND_CMD_STATUS:
        if (addr_no != 0)
            ERROR("Error. Command %02x needs 0 byte address, but addr_no = %d", command, addr_no);
        pnx8550_nand_register_setup(1, 0, 1, 0, is64mb, NAND_CMD_STATUS, 0);
        return;

    case NAND_CMD_RESET:
        if (addr_no != 0)
            ERROR("Error. Command %02x needs 0 byte address, but addr_no = %d", command, addr_no);
        pnx8550_nand_register_setup(1, 0, 0, 0, 0, NAND_CMD_RESET, 0);
        addr = NAND_ADDR(column,page_addr);
        NAND_ADDR_SEND(addr);
        return;

    case NAND_CMD_READ1:
    case NAND_CMD_READ0:
        if (addr_no != 3)
            ERROR("Error. Command %02x needs 3 byte address, but addr_no = %d", command, addr_no);

        pnx8550_nand_register_setup(1, 3, 1, 1, is64mb, command, 0);
        return;


    case NAND_CMD_READOOB:
        if (addr_no != 3)
            ERROR("NAND: Error. Command %02x needs 3 byte address, but addr_no = %d", command, addr_no);
        pnx8550_nand_register_setup(1, 3, 1, 1, is64mb, NAND_CMD_READOOB, 0);
        return;

    case NAND_CMD_READID:
        if (addr_no != 1)
            ERROR("NAND: Error. Command %02x needs 1 byte address, but addr_no = %d",
                   command, addr_no);
        pnx8550_nand_register_setup(1, 1, 1, 0, 0, NAND_CMD_READID, 0);
        return;
    }
}

/*
 * Setup the registers in PCIXIO
 */
static void pnx8550_nand_register_setup(u_char cmd_no,
                        u_char addr_no,
                        u_char include_data,
                        u_char monitor_ACK,
                        u_char enable64M,
                        int cmd_a,
                        int cmd_b)
{
    unsigned int reg_nand = 0;
    reg_nand |= enable64M ? PNX8550_XIO_FLASH_64MB:0;
    reg_nand |= include_data ? PNX8550_XIO_FLASH_INC_DATA: 0;
    reg_nand |= PNX8550_XIO_FLASH_CMD_PH(cmd_no);
    reg_nand |= PNX8550_XIO_FLASH_ADR_PH(addr_no);
    reg_nand |= PNX8550_XIO_FLASH_CMD_A(cmd_a);
    reg_nand |= PNX8550_XIO_FLASH_CMD_B(cmd_b);
    PNX8550_XIO_FLASH_CTRL = reg_nand;
    __asm__ __volatile__(                   \
    		".set   push\n\t"               \
    		".set   noreorder\n\t"          \
    		"lw     $0,%0\n\t"              \
    		"nop\n\t"                       \
    		".set   pop"                    \
    		: /* no output */               \
    		: "m" (*(int *)CKSEG1)          \
    		: "memory");
}

/*
 * Wait for the device to be ready for the next command
 */
static inline void pnx8550_nand_wait_for_dev_ready(void)
{
	u32 timeout = 0;

    DEBUG("");
    while((PNX8550_XIO_CTRL & PNX8550_XIO_CTRL_XIO_ACK) == 0)
    {
    	udelay(10);
    	timeout++;
    	if(timeout > XIO_WAIT_TIMEOUT)
    	{
    		// we waited for too long on the Flash chip
    		ERROR("Timeout on XIO ACK wait!");
    		BUG();
    	}
    }
}

/*
 * Return true if the device is ready, false otherwise
 */
static int pnx8550_nand_dev_ready(struct mtd_info *mtd)
{
    DEBUG("");
    return ((PNX8550_XIO_CTRL & PNX8550_XIO_CTRL_XIO_ACK) != 0);
}

/*
 *  hardware specific access to control-lines
*/
static void pnx8550_nand_hwcontrol(struct mtd_info *mtd, int cmd, unsigned int ctrl)
{
   // Nothing to do here, its all done by the XIO block
}

#ifdef CONFIG_NAND_WINCE_ECC

static int pnx8550_nand_calculate_ecc(struct mtd_info *mtd, const u_char *dat,
		       u_char *ecc_code)
{
#warning This code is converted from assembler!
	u32 a0, a1, a2, a3;
	u32 t0, t1, t2, t3, t4, t5, t6, t7, t8, t9;
	u32 v0, v1;

	a0 = (u32) dat;
	a1 = (u32) ecc_code;

	a3 = 0;
	a2 = 0;
	v1 = 0;
	t0 = 0;
	t1 = a0;
	v0 = (u32) eccArray;

	for (t0 = 0; t0 < 0x40; t0++) {
		a0 = *((u32*) t1);
		v1 = a0 ^ v1;
		a0 = (a0 >> 16) ^ a0;
		t4 = ((a0 >> 8) ^ a0);
		t7 = *(eccArray + (t4 & 0xFF));
		if (t7 & 1) {
			a3 ^= t0;
			a2 ^= ~t0;
		}
		t1 = t1 + 4;
	}
	t1 = (a3 >> 4) & 3;
	t7 = (a2 >> 4) & 3;
	t4 = *(eccArray + 0x100 + t1);
	a0 = *(eccArray + 0x100 + t7);

	t1 = ((t4 << 1) | a0) << 7;
	t5 = *(eccArray + 0x100 + (a3 & 0xF));
	t6 = t1 | t5;

	a2 = *(eccArray + 0x100 + (a2 & 0xF));
	a3 = (t6 << 1) | a2;

	a0 = 0x55555555;
	a2 = v1 & a0;
	a2 = (a2 >> 16) ^ a2;
	t2 = (a2 >> 8) ^ a2;
	t6 = 0xAAAAAAAA;
	a2 = v1 & t6;
	a2 = (a2 >> 16) ^ a2;

	t5 = *(eccArray + (t2 & 0xFF));
	t9 = (a2 >> 8) ^ a2;

	t4 = 0x33333333;
	a2 = v1 & t4;
	a0 = t5 & 1;

	a2 = (a2 >> 16) ^ a2;
	t7 = (a2 >> 8) ^ a2;
	t1 = *(eccArray + (t9 & 0xFF));
	t3 = (t1 & 1) << 1;

	a2 = *(eccArray + (t7 & 0xFF));
	t2 = 0xCCCCCCCC;
	t0 = a2 & 1;
	a2 = v1 & t2;
	a0 = t3 | a0;
	a2 = (a2 >> 16) ^ a2;
	t5 = (a2 >> 8) ^ a2;
	t8 = *(eccArray + (t5 & 0xFF));
	t1 = t0 << 2;

	a0 = t1 | a0;
	a2 = (t8 & 1) << 3;
	t0 = 0xF0F0F0F;
	a0 = a2 | a0;
	a2 = v1 & t0;
	a2 = (a2 >> 16) ^ a2;
	t3 = (a2 >> 8) ^ a2;
	t9 = 0xF0F0F0F0;
	t6 = *(eccArray + (t3 & 0xFF));
	t7 = t6 & 1;
	a2 = v1 & t9;
	a2 = (a2 >> 16) ^ a2;
	t2 = (a2 >> 8) ^ a2;
	a0 = (t7 << 4) | a0;

	t5 = *(eccArray + (t2 & 0xFF));
	t8 = 0xFF00FF;
	a2 = v1 & t8;
	t6 = t5 & 1;
	a2 = (a2 >> 16) ^ a2;
	t5 = 0xFF00FF00;
	t0 = a2 & 0xFF;
	a2 = v1 & t5;
	a0 = (t6 << 5) | a0;
	t7 = (a2 >> 16) ^ a2;
	t2 = *(eccArray + t0);
	t9 = (t7 >> 8) & 0xFF;
	t0 = *(eccArray + t9);
	t3 = t2 & 1;
	a2 = v1 & 0xFFFF;
	a0 = (t3 << 6) | a0;
	t4 = (a2 >> 8) ^ a2;
	t7 = *(eccArray + (t4 & 0xFF));
	t2 = (t0 & 1) << 7;
	a0 = t2 | a0;
	t9 = (t7 & 1) << 8;
	v1 = v1 >> 16;
	a2 = t9 | a0;
	t0 = (v1 >> 8) ^ v1;
	t3 = *(eccArray + (t0 & 0xFF));
	*((u8*) a1) = a3;
	t5 = (t3 & 1) << 9;
	v0 = t5 | a2;
	t8 = (a3 >> 8) | (v0 << 4);
	v0 = (v0 >> 4) | 0xC0;
	*((u8*) a1 + 1) = t8;
	*((u8*) a1 + 2) = v0;

	DEBUG("ECC: %02x %02x %02x", ecc_code[0], ecc_code[1], ecc_code[2]);

	return 0;
}

static void pnx8550_nand_ecc_hwctrl(struct mtd_info *mtdinfo, int mode)
{
	//do nothing
}

static inline int is_buf_blank(uint8_t *buf, size_t len)
{
	for (; len > 0; len--)
		if (*buf++ != 0xff)
			return 0;
	return 1;
}

/**
 * pnx8550_nand_correct_data - [NAND Interface] Detect and correct bit error(s)
 * @mtd:	MTD block structure
 * @dat:	raw data read from the chip
 * @read_ecc:	ECC from the chip
 * @calc_ecc:	the ECC calculated from raw data
 *
 * Detect and correct a 1 bit error for 256 byte block
 */
int pnx8550_nand_correct_data(struct mtd_info *mtd, u_char *dat, u_char *read_ecc,
		u_char *calc_ecc) {

	uint8_t s0, s1, s2;

#ifdef CONFIG_MTD_NAND_ECC_SMC
	s0 = calc_ecc[0] ^ read_ecc[0];
	s1 = calc_ecc[1] ^ read_ecc[1];
	s2 = calc_ecc[2] ^ read_ecc[2];
#else
	s1 = calc_ecc[0] ^ read_ecc[0];
	s0 = calc_ecc[1] ^ read_ecc[1];
	s2 = calc_ecc[2] ^ read_ecc[2];
#endif
	if ((s0 | s1 | s2) == 0)
		return 0;
	
	// ignore ECC code of empty pages since they don't match the oob data
	// oob: FF FF FF	ecc: 00 00 C0
	if(is_buf_blank(dat, 256))
		return 0;

	ERROR( "exp: %02x %02x %02x got: %02x %02x %02x",
			read_ecc[0], read_ecc[1], read_ecc[2],
			calc_ecc[0], calc_ecc[1], calc_ecc[2]);

	ERROR("Error correction not implemented!");
	return -1;
}

/**
 * nand_read_page_swecc - [REPLACABLE] software ecc based page read function
 * @mtd:	mtd info structure
 * @chip:	nand chip info structure
 * @buf:	buffer to store read data
 * @page:	page number to read
 */
static int pnx8550_nand_read_page_swecc(struct mtd_info *mtd, struct nand_chip *chip,
				uint8_t *buf, int page)
{
	int i, eccsize = chip->ecc.size;
	int eccbytes = chip->ecc.bytes;
	int eccsteps = chip->ecc.steps;
	uint8_t *p = buf;
	uint8_t *ecc_calc = chip->buffers->ecccalc;
	uint8_t *ecc_code = chip->buffers->ecccode;
	uint32_t *eccpos = chip->ecc.layout->eccpos;

	chip->ecc.read_page_raw(mtd, chip, buf, page);

	for (i = 0; eccsteps; eccsteps--, i += eccbytes, p += eccsize)
		chip->ecc.calculate(mtd, p, &ecc_calc[i]);

	for (i = 0; i < chip->ecc.total; i++)
		ecc_code[i] = chip->oob_poi[eccpos[i]];

	eccsteps = chip->ecc.steps;
	p = buf;

	for (i = 0 ; eccsteps; eccsteps--, i += eccbytes, p += eccsize) {
		int stat;

		stat = chip->ecc.correct(mtd, p, &ecc_code[i], &ecc_calc[i]);
		if (stat < 0)
			mtd->ecc_stats.failed++;
		else
			mtd->ecc_stats.corrected += stat;
	}
	return 0;
}

/**
 * nand_write_page_swecc - [REPLACABLE] software ecc based page write function
 * @mtd:	mtd info structure
 * @chip:	nand chip info structure
 * @buf:	data buffer
 */
static void pnx8550_nand_write_page_swecc(struct mtd_info *mtd, struct nand_chip *chip,
				  const uint8_t *buf)
{
	int i, eccsize = chip->ecc.size;
	int eccbytes = chip->ecc.bytes;
	int eccsteps = chip->ecc.steps;
	uint8_t *ecc_calc = chip->buffers->ecccalc;
	const uint8_t *p = buf;
	uint32_t *eccpos = chip->ecc.layout->eccpos;

	/* Software ecc calculation */
	for (i = 0; eccsteps; eccsteps--, i += eccbytes, p += eccsize)
		chip->ecc.calculate(mtd, p, &ecc_calc[i]);

	for (i = 0; i < chip->ecc.total; i++)
		chip->oob_poi[eccpos[i]] = ecc_calc[i];

	chip->ecc.write_page_raw(mtd, chip, buf);
}

#endif

/*
 * Main initialization routine
 */
int board_nand_init(struct nand_chip *nand)
{
    struct nand_chip *this = nand;

    DEBUG(KERN_INFO "%s (%s-%s)\n", THIS_MODULE_DESCRIPTION, __DATE__, __TIME__);

    /* Initialize structures */
    memset(this, 0, sizeof(struct nand_chip));

    /* Work out address of Nand Flash */
    pNandAddr = (u16*) (KSEG1 | (PNX8550_BASE18_ADDR & (~0x7)));

    pNandAddr = (u16*)(((u32)pNandAddr) +
                ((PNX8550_XIO_SEL0 & PNX8550_XIO_SEL0_OFFSET_MASK) >>
                  PNX8550_XIO_SEL0_OFFSET_SHIFT) * 8 * 1024 * 1024);

#ifdef CONFIG_NAND_DETECT
        is16bit = (PNX8550_XIO_SEL0 & PNX8550_XIO_SEL0_EN_16BIT)?1:0;
        is64mb = (PNX8550_XIO_FLASH_CTRL & PNX8550_XIO_FLASH_64MB)?1:0;
        DEBUG(KERN_INFO "Auto detected a %s 64MByte device on a %d bit bus\n",
               (is64mb?">=":"<"), (is16bit?16:8));
#else
        is16bit = (CONFIG_NAND_WIDTH == 16);
        is64mb = (CONFIG_NAND_SIZE >= 64);
        printk(KERN_INFO "Specified a %uMByte device on a %u bit bus. is16bit = %d, is64mb = %d\n",
               nand_width, nand_size, is16bit, is64mb);
#endif

    this->chip_delay  = 15;
    if (is16bit)
    {
        this->options     = NAND_BUSWIDTH_16;
        this->read_byte   = pnx8550_nand_read_byte_16bit;
        this->badblock_pattern = &nand16bit_memorybased;
        this->ecc.layout  = &nand16bit_oob_16;
    }
    else
    {
        this->read_byte   = pnx8550_nand_read_byte_8bit;
    }
    this->cmdfunc     = pnx8550_nand_command;
    this->read_word   = pnx8550_nand_read_word;
    this->read_buf    = pnx8550_nand_read_buf;
    this->write_buf   = pnx8550_nand_write_buf;
    this->verify_buf  = pnx8550_nand_verify_buf;
    this->dev_ready   = pnx8550_nand_dev_ready;
    this->cmd_ctrl    = pnx8550_nand_hwcontrol;

#ifdef CONFIG_NAND_WINCE_ECC
    this->ecc.layout  = &nand8bit_oob_wince;

    // only way to override the software ecc
    this->ecc.mode      = NAND_ECC_HW;
    this->ecc.calculate = pnx8550_nand_calculate_ecc;
    this->ecc.hwctl     = pnx8550_nand_ecc_hwctrl;
    this->ecc.correct   = pnx8550_nand_correct_data;
    this->ecc.size      = 256;
    this->ecc.bytes     = 3;

    // no other way than copy & paste
    this->ecc.read_page  = pnx8550_nand_read_page_swecc;
    this->ecc.write_page = pnx8550_nand_write_page_swecc;
#else
    this->ecc.mode     = NAND_ECC_SOFT;
#endif

#ifdef CONFIG_NAND_BBT
    this->options = NAND_USE_FLASH_BBT;
    this->bbt_td  = &nand_main_bbt_decr;
    this->bbt_md  = &nand_mirror_bbt_decr;
#endif

    return 0;
}
