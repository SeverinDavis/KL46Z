/*
 * uc_sw.h
 *
 *  Created on: Aug 28, 2014
 *      Author: Severin
 */

#ifndef UC_SW_H_
#define UC_SW_H_

#include "derivative.h"
#include "int.h"

typedef enum{
	switch_1,
	switch_3	
} switch_t;

void uc_sw_init(switch_t);
void uc_sw_init_int(switch_t, callback_t);

int uc_sw_poll_switch1();
int uc_sw_poll_switch3();

#endif
