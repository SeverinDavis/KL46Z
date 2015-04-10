/*
 * uc_pit.h
 *
 *  Created on: Jan 18, 2014
 *      Author: Severin
 */

#ifndef UC_PIT_H_
#define UC_PIT_H_

#include "derivative.h"
#include "int.h"

typedef enum{
	pit_0,
	pit_1
} pit_t;

void pit_enable(pit_t);
void pit_disable(pit_t);
void pit_init(pit_t, priority_t, int, callback_t);
void pit_set_callback(pit_t, callback_t);

#endif /* UC_PIT_H_ */
