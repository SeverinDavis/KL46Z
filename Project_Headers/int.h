/*
 * int.h
 *
 *  Created on: May 10, 2014
 *      Author: Severin
 */

#ifndef INT_H_
#define INT_H_

#include "derivative.h"

typedef void (*callback_t)();

typedef enum{
	priority_0,
	priority_1,
	priority_2,
	priority_3,
} priority_t;

void int_init(IRQInterruptIndex, priority_t);
void int_all_unmask();
void int_all_mask();
void int_unmask(IRQInterruptIndex);
void int_mask(IRQInterruptIndex);

#endif /* INT_H_ */
