/*
 * uc_led.h
 *
 *  Created on: Dec 24, 2013
 *      Author: Severin
 */

#ifndef UC_LED_H_
#define UC_LED_H_

#include "derivative.h"
#include "gpio.h"

typedef enum{
	led_green,
	led_red
} led_t;

void uc_led_all_init();
void uc_led_init(led_t);
void uc_led_on(led_t);
void uc_led_off(led_t);
void uc_led_toggle(led_t);

#endif /* UC_LED_H_ */
