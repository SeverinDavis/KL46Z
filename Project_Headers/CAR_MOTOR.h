/*
 * CAR_MOTOR.h
 *
 *  Created on: October 13, 2014
 *      Author: Severin
 */

#ifndef CAR_MOTOR_H
#define CAR_MOTOR_H

#include "derivative.h"
#include "uc_spi.h"
#include "uc_tpm.h"
#include "uc_dac.h"
#include "gpio.h"

typedef enum{
	CAR_MOTOR_dir_f,
	CAR_MOTOR_dir_b
}CAR_MOTOR_dir_t;

typedef enum{
	motor_0,
	motor_1,
	motor_2,
	motor_3
}CAR_MOTOR_motor_t;

//bit order md1, md2, md3
typedef enum{
	step_2   = 0b000,
	step_8   = 0b100,
	step_16  = 0b010,
	step_32  = 0b110,
	step_64  = 0b001,
	step_128 = 0b101,
	step_10  = 0b011,
	step_20  = 0b111
}CAR_MOTOR_step_size_t;


typedef enum{
	disable,
	enable
}CAR_MOTOR_state;

void CAR_MOTOR_init();
void CAR_MOTOR_manual_debug_init();
void CAR_MOTOR_update();
void CAR_MOTOR_set_MD(CAR_MOTOR_step_size_t);
void CAR_MOTOR_set_chip_en(CAR_MOTOR_state);
void CAR_MOTOR_set_output_en(CAR_MOTOR_state);
void CAR_MOTOR_set_rst(CAR_MOTOR_state);
void CAR_MOTOR_set_rst_cycle();
void CAR_MOTOR_set_current_limiter_en(CAR_MOTOR_state);
void CAR_MOTOR_CALLBACK_0();
void CAR_MOTOR_CALLBACK_1();
void CAR_MOTOR_CALLBACK_2();
void CAR_MOTOR_CALLBACK_3();
void WAKEUP_CALLBACK();
void CAR_MOTOR_motor_startup();
void CAR_MOTOR_shutdown();
void CAR_MOTOR_set_flags();
void CAR_MOTOR_set_t_period(CAR_MOTOR_motor_t, unsigned int);
void CAR_MOTOR_set_t_direction(CAR_MOTOR_motor_t, CAR_MOTOR_dir_t);
unsigned int get_a_period(unsigned int, unsigned int);
unsigned int get_d_period(unsigned int, unsigned int);
#endif
