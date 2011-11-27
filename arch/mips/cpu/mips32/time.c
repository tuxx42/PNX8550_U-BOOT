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

static unsigned long timestamp;

/* how many counter cycles in a jiffy */
#define CYCLES_PER_JIFFY	(CONFIG_SYS_MIPS_TIMER_FREQ + CONFIG_SYS_HZ / 2) / CONFIG_SYS_HZ
#define TICKS_PER_USEC		1000000

/*
 * timer without interrupts
 */

int timer_init(void)
{
#ifdef CONFIG_MIPS_CPU_PR4450
	unsigned int config7;

	/* enable and start counter */
	config7 = read_c0_config7();
	config7 &= ~0x00000008;
	write_c0_config7(config7);

#endif // CONFIG_MIPS_CPU_PR4450

	/* Set up the timer for the first expiration. */
	timestamp = 0;
	write_c0_count(0);
	write_c0_compare(~0);

	return 0;
}

ulong get_timer(ulong base)
{
	return (read_c0_count()/(CONFIG_SYS_MIPS_TIMER_FREQ/CONFIG_SYS_HZ)) - base;
#ifndef CONFIG_MIPS_CPU_PR4450
	unsigned int count;
	unsigned int expirelo = read_c0_compare();

	/* Check to see if we have missed any timestamps. */
	count = read_c0_count();
	while ((count - expirelo) < 0x7fffffff) {
		expirelo += CYCLES_PER_JIFFY;
		timestamp++;
	}
	write_c0_compare(expirelo);

	return (timestamp - base);
#endif
}

void __udelay(unsigned long usec)
{
	ulong startTicks = read_c0_count();
	ulong delayTicks = usec*(CONFIG_SYS_MIPS_TIMER_FREQ/TICKS_PER_USEC);
	ulong endTicks;

	/* Safeguard for too-long delays */
	if (delayTicks >= 0x80000000)
		delayTicks = 0x7FFFFFFF;

	/* Calculate end of delay (once) */
	endTicks = startTicks + delayTicks;

	/* If end of delay is behind COUNT rol-over, wait for COUNT to rol-over first */
	while ((read_c0_count() & 0x80000000) > (endTicks & 0x80000000))
		; /* nop */

	/* Wait for end of delay */
	while (read_c0_count() < endTicks)
		; /* nop */

#ifndef CONFIG_MIPS_CPU_PR4450
	unsigned int tmo;

	tmo = read_c0_count() + (usec * (CONFIG_SYS_MIPS_TIMER_FREQ / 1000000));
	while ((tmo - read_c0_count()) < 0x7fffffff)
		/*NOP*/;
#endif
}

void reset_timer(void)
{
	/* Set up the timer for the first expiration. */
	write_c0_count(0);
	write_c0_compare(~0);
}

/*
 * This function is derived from PowerPC code (read timebase as long long).
 * On MIPS it just returns the timer value.
 */
unsigned long long get_ticks(void)
{
	return get_timer(0);
}

/*
 * This function is derived from PowerPC code (timebase clock frequency).
 * On MIPS it returns the number of timer ticks per second.
 */
ulong get_tbclk(void)
{
	return CONFIG_SYS_MIPS_TIMER_FREQ;
}
