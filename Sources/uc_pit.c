/*
 * uc_pit.c
 *
 *  Created on: May 11, 2014
 *      Author: Severin
 */

#include "uc_pit.h"
#include "uc_led.h"

static callback_t pit0_callback = 0;
static callback_t pit1_callback = 0;

/*
 * enables pit, duh
 */
void pit_enable(pit_t p_timer)
{
	PIT_TCTRL(p_timer) |= 1;
}

/*
 * disables pit
 */
void pit_disable(pit_t p_timer)
{
	PIT_TCTRL(p_timer) &= ~(1);
}

/*
 * initializes pit with microsecond period
 */
void pit_init(pit_t p_timer, priority_t p_priority, int p_us_period, callback_t p_callback)
{
	//PIT clock gating
	SIM_SCGC6 |= 1<<23;

	//enable PIT
	PIT_MCR = 0;

	int_init(INT_PIT, p_priority);

	pit_set_callback(p_timer, p_callback);

	PIT_LDVAL(p_timer) = 24 * p_us_period; 
	PIT_TCTRL(p_timer) |= 0b10;
}

/*
 * sets pit callback
 */
void pit_set_callback(pit_t p_timer, callback_t p_callback)
{
	if(p_timer == pit_0)
	{
		pit0_callback = p_callback;
	}
	if(p_timer == pit_1)
	{
		pit1_callback = p_callback;
	}	
}


/*
 * interrupt handler
 */
void PIT_IRQHandler()
{
	if(PIT_TFLG0 != 0)
	{
		PIT_TFLG0 = 1;
		if(pit0_callback)
		{
			pit0_callback();
		}
	}
	
	if(PIT_TFLG1 != 0)
		{
			PIT_TFLG1 = 1;
			if(pit1_callback)
			{
				pit1_callback();
			}
		}
}
