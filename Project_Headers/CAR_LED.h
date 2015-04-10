/*
 * CAR_LED.h
 *
 *  Created on: September 12, 2014
 *      Author: Severin
 */

#ifndef CAR_LED_H
#define CAR_LED_H

#include "derivative.h"
#include "uc_spi.h"
#include "gpio.h"

typedef enum{
	car_led_off,
	car_led_blu,
	car_led_grn,
	car_led_trq,
	car_led_red,
	car_led_ppl,
	car_led_ylo, //#yolo!
	car_led_wht
} car_led_color_t;

typedef enum{
	car_led_0,
	car_led_1,
	car_led_2,
	car_led_3
} car_led_t;

void CAR_LED_init();
void CAR_LED_update();
void CAR_LED_set_color(car_led_t, car_led_color_t);

#endif
