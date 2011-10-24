#
# (C) Copyright 2003
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.
# (C) Copyright 2006
# Philips Semiconductors B.V.
#
# See file CREDITS for list of people who contributed to this
# project.
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License as
# published by the Free Software Foundation; either version 2 of
# the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston,
# MA 02111-1307 USA
#

#
# Philips Consumer Electronics developed board for PNX8550
#

#  Text base should eiter be 0x80000000 for cached memory
#  or 0xA0000000 for uncached memory to properly locate the
#  the exeception table because it's not relocated at runtime.
#  However, EJTAG debuggers usually require an interrupt
#  vector and some memory space for their CPU control code.
#  Since U-Boot doens't actually use any exceptions, an
#  offset may be added to satisfy the debugger's needs.
#  Beware: exceptions aren't trapped if an offset is used!
TEXT_BASE = 0xA0010000 # Use offset of 0x10000
