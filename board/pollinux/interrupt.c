/*
 * interrupt.c
 *
 *  Created on: Nov 25, 2011
 *      Author: laszlo
 */
#include <common.h>
#include <asm/mipsregs.h>
#include <asm/io.h>
#include <asm/mach-pnx8550/pnx8550.h>
#include <asm/mach-pnx8550/int.h>

static inline void modify_cp0_intmask(unsigned clr_mask, unsigned set_mask)
{
	unsigned long status = read_c0_status();

	status &= ~((clr_mask & 0xFF) << 8);
	status |= (set_mask & 0xFF) << 8;

	write_c0_status(status);
}

static inline void mask_gic_int(unsigned int irq_nr)
{
	/* interrupt disabled, bit 26(WE_ENABLE)=1 and bit 16(enable)=0 */
	PNX8550_GIC_REQ(irq_nr) = 1<<28; /* set priority to 0 */
}

static inline void mask_irq(unsigned int irq_nr)
{
	if ((PNX8550_INT_CP0_MIN <= irq_nr) && (irq_nr <= PNX8550_INT_CP0_MAX)) {
		modify_cp0_intmask(1 << irq_nr, 0);
	} else if ((PNX8550_INT_GIC_MIN <= irq_nr) &&
		(irq_nr <= PNX8550_INT_GIC_MAX)) {
		mask_gic_int(irq_nr - PNX8550_INT_GIC_MIN);
	} else if ((PNX8550_INT_TIMER_MIN <= irq_nr) &&
		(irq_nr <= PNX8550_INT_TIMER_MAX)) {
		modify_cp0_intmask(1 << 7, 0);
	} else {
		printf("mask_irq: irq %d doesn't exist!\n", irq_nr);
	}
}

void disable_pic(void)
{
	int i;
	int configPR;

	for (i = 0; i < PNX8550_INT_CP0_TOTINT; i++) {
		mask_irq(i);	/* mask the irq just in case  */
	}
	/* Priority level 0 */
	PNX8550_GIC_PRIMASK_0 = PNX8550_GIC_PRIMASK_1 = 0;

	/* Set int vector table address */
	PNX8550_GIC_VECTOR_0 = PNX8550_GIC_VECTOR_1 = 0;
}
