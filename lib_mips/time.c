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

#include <common.h>
#include <asm/mipsregs.h>

#define TICKS_PER_USEC		1000000

/*
 * timer without interrupts
 */
int timer_init(void)
{
#ifdef CONFIG_MIPS_CPU_PR4450
	int configPR;

	/* enable and start counter */
	configPR = read_c0_configPR();
	configPR &= ~0x8;
	write_c0_configPR(configPR);

#endif /* CONFIG_MIPS_CPU_PR4450 */

	write_32bit_cp0_register(CP0_COUNT, 0);
	write_32bit_cp0_register(CP0_COMPARE, ~0);

	return 0;
}

ulong get_timer(ulong base)
{
	return (read_32bit_cp0_register(CP0_COUNT)/(CFG_CP0_COUNT_RATE/CFG_HZ)) - base;
}

void udelay (unsigned long usec)
{
	ulong startTicks = read_32bit_cp0_register(CP0_COUNT);
	ulong delayTicks = usec*(CFG_CP0_COUNT_RATE/TICKS_PER_USEC);
	ulong endTicks;

	/* Safeguard for too-long delays */
	if (delayTicks >= 0x80000000)
		delayTicks = 0x7FFFFFFF;

	/* Calculate end of delay (once) */
	endTicks = startTicks + delayTicks;

	/* If end of delay is behind COUNT rol-over, wait for COUNT to rol-over first */
	while ((read_32bit_cp0_register(CP0_COUNT) & 0x80000000) > (endTicks & 0x80000000))
		; /* nop */

	/* Wait for end of delay */
	while (read_32bit_cp0_register(CP0_COUNT) < endTicks)
		; /* nop */
}

/*
 * This function is derived from PowerPC code (read timebase as long long).
 * On MIPS it just returns the timer value.
 */
unsigned long long get_ticks(void)
{
	return read_32bit_cp0_register(CP0_COUNT);
}

/*
 * This function is derived from PowerPC code (timebase clock frequency).
 * On MIPS it returns the number of timer ticks per second.
 */
ulong get_tbclk(void)
{
	return CFG_CP0_COUNT_RATE;
}
