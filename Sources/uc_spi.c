/*
 * uc_spi.c
 *
 *  Created on: June 15, 2014
 *      Author: Severin
 */

#include "uc_spi.h"

/*
 * initializes spi. (untested for spi1)
 */
void uc_spi_init(spi_t p_spi)
{
	switch(p_spi)
	{
	case spi_0: 
		SIM_SCGC4 |= 1 << 22;
		SPI0_BR = 2 << 4;
		SPI0_BR |= 8;
		
		SPI0_C1 = SPI_C1_MSTR_MASK;

		gpio_port_init(port_E, pin_17, alt_2, output);
		gpio_port_init(port_E, pin_18, alt_2, output);
		
		SPI0_C1 |= 1 << 6;
		break;
		
	case spi_1:
		SIM_SCGC4 |= 1 << 23;
		SPI1_BR = 2 << 4;
		SPI1_BR |= 8;
		
		SPI1_C1 = SPI_C1_MSTR_MASK | SPI_C1_SSOE_MASK;

		gpio_port_init(port_D, pin_5, alt_2, output);
		gpio_port_init(port_D, pin_6, alt_2, output);
		gpio_port_init(port_D, pin_7, alt_2, output);
		gpio_port_init(port_D, pin_4, alt_1, output);
		gpio_set_pin_state(port_D, pin_4, 0);
		SPI1_C1 |= 1 << 6;
		break;
		
	default:
		break;
	}
}

/*
 * sends a byte (untested for spi1)
 */
void uc_spi_send(spi_t p_spi, char p_char)
{
	int i;
	switch(p_spi)
	{
	case spi_0:
		
		//waiting for spi to be available
		while((0b100000 & SPI0_S) == 0)  
		{} 
		//send char
		SPI0_DL = p_char;
		
		//sucky busy wait loop. 2000 seems to be about right.
		//they should really have some kind of transmit complete flag. 
		//this is ridonkadonk
		for(i = 0; i<2100; i++)
		{
			asm("nop");
		}
		break;
	
	case spi_1:
		gpio_set_pin_state(port_D, pin_4, 1);
		while(!(0b100000 & SPI1_S))  
		{} 
		SPI1_DL = p_char;
		
		for(i =0; i<2100; i++)
		{
			asm("nop");
		}
		gpio_set_pin_state(port_D, pin_4, 0);
		break;
		
	default:
		break;
	}
}
