/*
 * CAR_XBEE.h
 *
 *  Created on: November 9, 2014
 *      Author: Severin
 */

#ifndef CAR_XBEE_H
#define CAR_XBEE_H

#include "derivative.h"
#include "gpio.h"
#include "uc_uart.h"

void CAR_XBEE_init();
void CAR_XBEE_reset_cycle();
void CAR_XBEE_on();
void CAR_XBEE_sleep();

#endif
