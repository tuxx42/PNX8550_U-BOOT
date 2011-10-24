#!/bin/bash
# generates a WinCe1.nb0 file capable of 
# loading a linux kernel in elf format. 
#
# For this file to work you must have:
#   vmlinux in this folder
#
# after compilation is ready, the elf 
# can be loaded by issuing following 
# commands in u-boot:
#   cp.b 0x80200000 0x82000000 <sizeof vmlinux>
#   <be patient>
#   bootelf 0x82000000

cp u-boot.bin WinCe1.nb0
dd if=/dev/null of=WinCe1.nb0 bs=1 count=0 seek=1048576
dd if=vmlinux of=WinCe1.nb0 oflag=append conv=notrunc
