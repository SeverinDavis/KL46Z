/*
 * uc_uart.c
 *
 *  Created on: June 21, 2014
 *      Author: Severin
 */

#include "uc_uart.h"

static callback_t uart0_callback = 0;

/*
 * inits UART0
 */
void uc_uart_init()
{	//clock setup stuff
    MCG_C1 |= 0b11 << 1;
    MCG_C2 |= 1;
    
	SIM_SCGC4 |= 1 << 10;
	
	//baud rate. MCGIRCLK=2000000, OSR default = 15.
	//MCGIRCLK/((OSR+1) (13)) = 9600
	UART0_BDL = (unsigned char) 13;
	//clear relevant bits in BDH
	UART0_BDH &= 0xE0;
	UART0_BDH |= (unsigned char)((~0xE0)&(13 >> 8));
	
	//enable receiver
	UART0_C2 |= 1 << 2;
	
	//setting receiver interrupt enable
	UART0_C2 |= 1 << 5;

	//configuring interrupt.
    int_init(INT_UART0, priority_2);
    
    SIM_SOPT2 |= SIM_SOPT2_UART0SRC(3);
}

/*
 * sets interrupt callback for UART0
 */
void uc_uart_set_callback(callback_t p_callback)
{
	uart0_callback = p_callback;
}

/*
 * reads and returns incoming data for UART0
 */
unsigned char uc_uart_get_data()
{
	return UART0_D;
}

/*
 * interrupt handler
 */
void UART0_IRQHandler()
{
	if(uart0_callback)
	{
		uart0_callback();	
	}
	else
	{
		//data lost
		uc_uart_get_data();
	}
}

/*
 * disables uart interrupt
 */
void uc_uart_mask_int()
{
	int_mask(INT_UART0);
}

/*
 * enables uart interrupt
 */
void uc_uart_unmask_int()
{
	int_unmask(INT_UART0);
}
