/*
 * uc_sw.c
 *
 *  Created on: Aug 28, 2014
 *      Author: Severin
 */

#include "uc_sw.h"
#include "gpio.h"

/*
 * configures switch for polling
 */
void uc_sw_init(switch_t p_switch)
{
	if(p_switch == switch_1)
	{
		//setup GPIO for relevant pin
		gpio_port_init(port_C, pin_3, alt_1, input);
		
		//enable pullup res
		PORTC_PCR3 |= 1 << 1;
	}
	
	if(p_switch == switch_3)
	{
		gpio_port_init(port_C, pin_12, alt_1, input);
		PORTC_PCR12 |= 1 << 1;
	}
}

/*
 * configures switch as interrupt
 */
void uc_sw_init_int(switch_t p_switch, callback_t p_callback)
{
	if(p_switch == switch_1)
	{
		//setup GPIO for relevant pin
		gpio_port_init(port_C, pin_3, alt_1, input);
		
		//enable pullup res
		PORTC_PCR3 |= 1 << 1;
		gpio_enable_interrupt(port_C, pin_3, trig_int_negedge, p_callback);
	}
	
	if(p_switch == switch_3)
	{
		gpio_port_init(port_C, pin_12, alt_1, input);
		PORTC_PCR12 |= 1 << 1;
		
		gpio_enable_interrupt(port_C, pin_12, trig_int_negedge, p_callback);
	}
	
	
}

/*
 * returns state of switch1
 */
int uc_sw_poll_switch1()
{
	return gpio_get_pin_state(port_C, pin_3);
}

/*
 * returns stae of switch3
 */
int uc_sw_poll_switch3()
{
	return gpio_get_pin_state(port_C, pin_12);
}
