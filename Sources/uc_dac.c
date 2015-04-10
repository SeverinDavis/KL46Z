/*
 * uc_dac.c
 *
 *  Created on: June 8, 2013
 *      Author: Severin
 */

#include "uc_dac.h"

/*
 * initialize d to a converter
 */
void uc_dac_init()
{
	gpio_port_init(port_E, pin_30, alt_0, output);
	SIM_SCGC6  |= 1 << 31;
	uc_dac_set_output(0);
	
	DAC0_C0 |= 1 << 6;
	DAC0_C0 |= 1 << 7;
}

/*
 * sets output of dac
 */
void uc_dac_set_output(int p_numerator)
{
	//p_numerator is out of 4095. VOUT = VIN * (1+p_numerator)/4096
	//MAX VOUT = VIN
	//MIN VOUT = (1/4096) * VIN
	DAC0_DAT0L = p_numerator;
	DAC0_DAT0H = (p_numerator >> 8);
}
