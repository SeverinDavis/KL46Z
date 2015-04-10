/*
 * int.c
 *
 *  Created on: May 10, 2014
 *      Author: Severin
 */

#include "int.h"

/*
 * set the priority for a specified interrupt
 */
void int_init(IRQInterruptIndex p_vector, priority_t p_priority)
{
	int IRQ = p_vector - 16;
	NVIC_IP(IRQ / 4) &= ~(3 << ((8 * (IRQ % 4)) + 6));
	NVIC_IP(IRQ / 4) |= (p_priority << ((8 * (IRQ % 4)) + 6));
}

/*
 * unmasks all interrupts
 */
void int_all_unmask()
{
	NVIC_ISER = 0xFFFFFFFF;
}

/*
 * masks all interrupts
 */
void int_all_mask()
{
	NVIC_ICER = 0xFFFFFFFF;
}

/*
 * masks specified interrupt
 */
void int_mask(IRQInterruptIndex p_vector)
{
	int IRQ = p_vector - 16;
	NVIC_ICER = (1 << IRQ);
}

/*
 * unmasks specified interrupt
 */
void int_unmask(IRQInterruptIndex p_vector)
{
	int IRQ = p_vector - 16;
	NVIC_ISER = (1 << IRQ);
}
