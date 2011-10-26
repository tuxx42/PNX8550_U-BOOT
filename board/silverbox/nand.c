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
  1 20111016   laszloh     Modified /driver/mtd/nand/pnx8550_nand.c for u-boot
*/

#include <common.h>
#include <malloc.h>
#include <asm/io.h>
#include <asm/errno.h>
#include <asm/mipsregs.h>
#include <asm/addrspace.h>

#if (CONFIG_COMMANDS & CFG_CMD_NAND)

#include <nand.h>
#include <nxp_pcixio_ipa051.h>

#define THIS_MODULE_DESCRIPTION "Driver for NAND Flash on the PNX8550 XIO bus"
#define DEVNAME "pnx8550_nand"

#define ERROR(fmt, arg...) printk(KERN_ERR DEVNAME "(%s): " fmt "\n", __FUNCTION__,  ## arg )
#define INFO(fmt, arg...)  printk(KERN_INFO DEVNAME ": " fmt "\n", ## arg )

#undef DEBUG
#ifdef CFG_NAND_DEBUG
#	define DEBUG(fmt, arg...) printk(KERN_DEBUG DEVNAME "(%s): " fmt "\n", __FUNCTION__, ##  arg )
#else
#	define DEBUG( fmt, arg...)
#endif


/******************************************************************************
* ADDRESS MACROS                                                              *
*******************************************************************************/

#define NAND_ADDR(_col, _page) ((_col) & (mtd->oobblock - 1)) + ((_page) << this->page_shift)
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
* STATIC VARIABLES                                                            *
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
static struct nand_oobinfo nand16bit_oob_16 = {
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

static int is16bit = 0;
static int is64mb = 0;

static struct mtd_info *pnx8550_mtd;
static struct nand_chip *pnx8550_nand;

/******************************************************************************
* FUNCTION IMPLEMENTATION                                                     *
*******************************************************************************/

/**
 * Allocate the buffer used to tranfser data between flash and memory.
 * We use a transfer buffer so we know that the buffer is 32bit aligned.
 */
static void inline pnx8550_nand_alloc_transfer_buffer(void)
{
	size_t alloc_size;

    if (transferBuffer == NULL)
    {
		// We want to ensure that we don't share any cache lines with any other memory block.
		alloc_size = (pnx8550_mtd->oobblock + pnx8550_mtd->oobsize + CFG_CACHELINE_SIZE - 1) & ~(CFG_CACHELINE_SIZE-1);
		BUG_ON(alloc_size < pnx8550_mtd->oobblock + pnx8550_mtd->oobsize);

		transferBuffer = kmalloc(alloc_size, GFP_DMA | GFP_KERNEL);
		if (transferBuffer == NULL)
		{
			ERROR("Failed to allocate transfer buffer!");
		}
		else
		{
			// Make sure we've been allocated cache aligned (SLOB
			// is broken and hands out non-aligned addresses).
			BUG_ON(((unsigned long)transferBuffer) & (CFG_CACHELINE_SIZE-1));

			// Ensure that any pending cache writes to our memory block are flushed before we start using it.
/*			dma_cache_wback_inv((unsigned long)transferBuffer, alloc_size);	*/

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
    while((readl(IPA051 | IPA051_GPXIO_INT_STATUS) & IPA051_GPXIO_INT__ACK) == 0);
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
    u32 internal;
    u32 external;

    if (toxio)
    {
        cmd = IPA051_DMA_CTRL__CMD_WRITE;
        internal = (u32)virt_to_phys(from);
        external = (u32)to - KSEG1;
    }
    else
    {
        cmd = IPA051_DMA_CTRL__CMD_READ;
        internal = (u32)virt_to_phys(to);
        external = (u32)from - KSEG1;
    }

    writel(bytes >> 2, (IPA051 | IPA051_DMA_LENGTH));
    writel(external, (IPA051 | IPA051_DMA_EADDR));
    writel(internal, (IPA051 | IPA051_DMA_IADDR));
    writel(0xffff, (IPA051 | IPA051_DMA_INT_CLR));
    writel( IPA051_DMA_CTRL__MAX_BURST_SIZE_128	|
    		IPA051_DMA_CTRL__SND2XIO			|
    		IPA051_DMA_CTRL__INIT_DMA			|
    		cmd, (IPA051 | IPA051_DMA_CTRL));

    while((readl(IPA051 | IPA051_DMA_INT_STATUS) & IPA051_DMA_INT__COMPL) == 0);
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
 * Write function for 16bit buswidth with
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
    int gpxio;
    u_char *transBuf = (u_char *)buf;
    DEBUG("last_col_addr=%d last_page_addr=0x%x len=%d", last_col_addr, last_page_addr, len);
    /* some sanity checking, word access only please */
    if (len&1)
    {
        ERROR("non-word aligned length requested!");
    }


    pnx8550_nand_alloc_transfer_buffer();
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
    if ((last_col_addr + len) > mtd->oobblock)
        oobLen = (last_col_addr + len) - mtd->oobblock;
    pageLen = len - oobLen;

    /* Clear the done flag */
    gpxio = readl((IPA051 | IPA051_GPXIO_CTRL));
    writel(gpxio | IPA051_GPXIO_CTRL__GPXIO_DONE, (IPA051 | IPA051_GPXIO_CTRL));

    if (pageLen > 0)
    {
        NAND_TRANSFER_TO(addr, transBuf, pageLen);
    }

    if (oobLen > 0)
    {
        pnx8550_nand_wait_for_dev_ready();

        pnx8550_nand_register_setup(1, 0, 0, 1, is64mb, NAND_CMD_READOOB, 0);
        /* Work out where in the OOB we are going to start to write */
        addr = NAND_ADDR(last_col_addr - mtd->oobblock, last_page_addr);
        NAND_ADDR_SEND(addr);
        pnx8550_nand_register_setup(2, 3, 1, 1, is64mb, NAND_CMD_SEQIN, NAND_CMD_PAGEPROG);

        /* Clear the done flag */
        gpxio = readl((IPA051 | IPA051_GPXIO_CTRL));
        writel(gpxio | IPA051_GPXIO_CTRL__GPXIO_DONE, (IPA051 | IPA051_GPXIO_CTRL));
        NAND_TRANSFER_TO(addr, transBuf + pageLen, oobLen);
    }

    /*
       Increment the address so on the next write we write in the
       correct place.
    */
    last_col_addr += len;
    if (last_col_addr >= mtd->oobblock + mtd->oobsize)
    {
        last_col_addr -=mtd->oobblock + mtd->oobsize;
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
    DEBUG("last_col_addr=%d last_page_addr=0x%x len=%d", last_col_addr, last_page_addr, len);
    /* some sanity checking, word access only please */
    if (len&1)
    {
        ERROR("non-word aligned length");
    }

    pnx8550_nand_alloc_transfer_buffer();
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
    if ((last_col_addr + len) > mtd->oobblock)
        oobLen = (last_col_addr + len) - mtd->oobblock;

    pageLen = len - oobLen;
    if (pageLen)
    {
        NAND_TRANSFER_FROM(addr, transBuf, pageLen);
    }


    if (oobLen > 0)
    {
        pnx8550_nand_wait_for_dev_ready();

        pnx8550_nand_register_setup(1, 3, 1, 1, is64mb, NAND_CMD_READOOB, 0);
        addr = NAND_ADDR(last_col_addr - mtd->oobblock, last_page_addr);
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
    if (last_col_addr > mtd->oobblock + mtd->oobsize)
    {
        last_col_addr -=mtd->oobblock + mtd->oobsize;
        last_page_addr ++;
    }

    pnx8550_nand_wait_for_dev_ready();

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
    pnx8550_nand_alloc_transfer_buffer();
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
    int gpxio;
    DEBUG("command=0x%02x column=%d page_addr=0x%x",command, column, page_addr);
    /*
       If we are starting a write work out whether it is to the
       OOB or the main page and position the pointer correctly.
    */
    if (command == NAND_CMD_SEQIN) {
        int readcmd;
        int col = column;
        if (column >= mtd->oobblock) {
            /* OOB area */
            col -= mtd->oobblock;
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

        /* Clear the done flag */
        gpxio = readl((IPA051 | IPA051_GPXIO_CTRL));
        writel(gpxio | IPA051_GPXIO_CTRL__GPXIO_DONE, (IPA051 | IPA051_GPXIO_CTRL));

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
    reg_nand |= enable64M ? IPA051_NAND_CTRLS__CTRLS_64MB:0;
    reg_nand |= include_data ? IPA051_NAND_CTRLS__CTRLS_INC_DATA : 0;
    reg_nand |= IPA051_NAND_CTRLS__CTRLS_CMD_PH(cmd_no);
    reg_nand |= IPA051_NAND_CTRLS__CTRLS_ADR_PH(addr_no);
    reg_nand |= IPA051_NAND_CTRLS__COMMAND_A(cmd_a);
    reg_nand |= IPA051_NAND_CTRLS__COMMAND_B(cmd_b);
    writel(reg_nand, (IPA051 | IPA051_NAND_CTRLS));
    __asm__ __volatile__(                   \
    		".set   push\n\t"               \
    		".set   noreorder\n\t"          \
    		".set mips2\n\t"				\
    		"sync\n\t"                      \
    		".set   pop"                    \
			: : : "memory");
}


/*
 * Wait for the device to be ready for the next command
 */
static inline void pnx8550_nand_wait_for_dev_ready(void)
{
    DEBUG("");
    while((readl(IPA051 | IPA051_XIO_CTRL) & IPA051_XIO_CTRL__XIO_ACK) == 0);
}


/*
 * Return true if the device is ready, false otherwise
 */
static int pnx8550_nand_dev_ready(struct mtd_info *mtd)
{
    //DEBUG("");
    return ((readl(IPA051 | IPA051_XIO_CTRL) & IPA051_XIO_CTRL__XIO_ACK) != 0);
}


/*
 *  hardware specific access to control-lines
*/
static void pnx8550_nand_hwcontrol(struct mtd_info *mtd, int cmd, unsigned int ctrl)
{
   // Nothing to do here, its all done by the XIO block
}


int board_nand_init (struct nand_chip *nand)
{
    struct nand_chip *this;

    printk(KERN_INFO "%s (%s-%s)\n", THIS_MODULE_DESCRIPTION, __DATE__, __TIME__);

    /* Get pointer to private data */
    this = nand;
    pnx8550_nand = nand;

    // set pointer to mtd device
    pnx8550_mtd = nand_info;

    /* Initialize structures */
    memset(this, 0, sizeof(struct nand_chip));

    /* Work out address of Nand Flash */
    pNandAddr = (u16*) (KSEG1 | (readl(IPA051 | IPA051_BASE18) & (~0x7)));

    pNandAddr = (u16*)(((u32)pNandAddr) +
                ((readl(IPA051 | IPA051_XIO_SEL0_PROF) & IPA051_XIO_SELx_PROF__OFFSET_MASK) >>
                	IPA051_XIO_SELx_PROF__OFFSET_SHIFT) * 8 * 1024 * 1024);

#ifdef CFG_NAND_AUTO
        is16bit = (readl(IPA051 | IPA051_XIO_SEL0_PROF) & IPA051_XIO_SELx_PROF__EN_16BIT)?1:0;
        is64mb = (readl(IPA051 | IPA051_NAND_CTRLS) & IPA051_NAND_CTRLS__CTRLS_64MB)?1:0;
        printk(KERN_INFO "Auto detected a %s 64MByte device on a %d bit bus\n",
               (is64mb?">=":"<"), (is16bit?16:8));
#else
        is16bit = (CFG_NAND_WIDTH == 16);
        is64mb = (CFG_NAND_SIZE >= 64);
        printk(KERN_INFO "Specified a %uMByte device on a %u bit bus. is16bit = %d, is64mb = %d\n",
               nand_width, nand_size, is16bit, is64mb);
#endif

    this->chip_delay  = 15;
    if (is16bit)
    {
        this->options			= NAND_BUSWIDTH_16;
        this->read_byte			= pnx8550_nand_read_byte_16bit;
        this->badblock_pattern	= &nand16bit_memorybased;
        this->autooob			= &nand16bit_oob_16;
    }
    else
    {
        this->read_byte	= pnx8550_nand_read_byte_8bit;
    }
    this->cmdfunc		= pnx8550_nand_command;
    this->read_word		= pnx8550_nand_read_word;
    this->read_buf		= pnx8550_nand_read_buf;
    this->write_buf		= pnx8550_nand_write_buf;
    this->verify_buf	= pnx8550_nand_verify_buf;
    this->dev_ready		= pnx8550_nand_dev_ready;
    this->hwcontrol		= pnx8550_nand_hwcontrol;
    this->eccmode		= NAND_ECC_SOFT;

    /* We don't seem to have any mtd partitions */
#if 0

    /* Scan to find existence of the device */
    if (nand_scan(pnx8550_mtd, 1)) {
        ERROR("Exiting No Devices");
        return -ENXIO;
    }

    /* Overwrite initial User partition size based on the device size */
    partition_info[USER_PARTITION].size = (pnx8550_mtd.size - USER_BLOCK_START);

    /* Register the partitions */
    add_mtd_partitions(&pnx8550_mtd, partition_info, NUM_PARTITIONS);
#endif

    /* Return happy */
    return 0;
}

#endif
