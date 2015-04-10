/*
 * gpio.c
 *
 *  Created on: May 7, 2014
 *      Author: Severin
 */

#include "gpio.h"
#include "uc_led.h"

#define PORTX_PCR_BASE 			0x40049000

#define FGPIOX_PDOR_BASE    	0xF80FF000                    
#define FGPIOX_PSOR_BASE    	0xF80FF004            
#define FGPIOX_PCOR_BASE    	0xF80FF008        
#define FGPIOX_PTOR_BASE    	0xF80FF00C                  
#define FGPIOX_PDIR_BASE    	0xF80FF010                       
#define FGPIOX_PDDR_BASE    	0xF80FF014

static callback_t gpio_c_callback[32] ={0};
static callback_t gpio_d_callback[32] ={0};

/*
 * sets pin direction
 */
void gpio_pin_set_dir(port_t p_port, pin_t p_pin, dir_t p_dir)
{
	unsigned int * custom_PDDR = (unsigned int *)(FGPIOX_PDDR_BASE + (p_port * 0x40));
	
	*custom_PDDR &= ~(1 << p_pin);
	
	*custom_PDDR |= p_dir << p_pin;
}

/*
 *  sets pin function (check reference manual for pin specific functions)
 */
void gpio_pin_set_alt(port_t p_port, pin_t p_pin, alt_t p_alt)
{
    unsigned int * custom_PCR = (unsigned int *)(PORTX_PCR_BASE + p_port * 0x1000 + p_pin * 0x4);

    *custom_PCR &= ~0x700;
    
    *custom_PCR |= p_alt << 8;
}

/*
 * initializes a port/pin
 */
void gpio_port_init(port_t p_port, pin_t p_pin, alt_t p_alt, dir_t p_dir)
{
	SIM_SCGC5 |= 1 << (p_port + 9); // enable clock to port
	
	gpio_pin_set_alt(p_port, p_pin, p_alt);
	
	gpio_pin_set_dir(p_port, p_pin, p_dir);
	
}

/*
 * returns the state of a pin
 */
int gpio_get_pin_state(port_t p_port, pin_t p_pin)
{
	unsigned int * custom_PDIR = (unsigned int *)(FGPIOX_PDIR_BASE + (p_port * 0x40));
			
	return (*custom_PDIR >> p_pin) & 1;
}

/*
 * sets the state of a gpio pin
 */
void gpio_set_pin_state(port_t p_port, pin_t p_pin, int p_state)
{
	if(p_state)
	{
		unsigned int * custom_PSOR = (unsigned int *)(FGPIOX_PSOR_BASE + (p_port * 0x40));
		
		*custom_PSOR = 1 << p_pin;	
	}
	
	else
	{
		unsigned int * custom_PCOR = (unsigned int *)(FGPIOX_PCOR_BASE + (p_port * 0x40));
		
		*custom_PCOR = 1 << p_pin;	
		
	}
}

/*
 * toggles the state of a gpio pin
 */
void gpio_toggle_pin_state(port_t p_port, pin_t p_pin)
{
	unsigned int * custom_PTOR = (unsigned int *)(FGPIOX_PTOR_BASE + (p_port * 0x40));
	
	*custom_PTOR = 1 << p_pin;	
}

/*
 *  enables an interrupt on a pin. needs edge trigger and callback function specified
 */
void gpio_enable_interrupt(port_t p_port, pin_t p_pin, trig_t p_trig, callback_t p_callback)
{
	   unsigned int * custom_PCR = (unsigned int *)(PORTX_PCR_BASE + p_port * 0x1000 + p_pin * 0x4);
	   
	   if(p_port == port_C)
	   {
		   gpio_c_callback[p_pin] = p_callback; 
	   }
	   else if(p_port == port_D)
	   {
		   gpio_d_callback[p_pin] = p_callback;
	   }
	   //enable pin interrupt with trigger
	   *custom_PCR &= ~PORT_PCR_IRQC_MASK;
	   *custom_PCR |= PORT_PCR_IRQC(p_trig);
	   
	   //config priority
	   int_init(INT_PORTC_PORTD, priority_1);
}

/*
 * interrupt handler for all pins on ports C and D
 */
void PORTCD_IRQHandler()
{
	int i = 0;
	
	//check all portc sources
	for(i = 0; i < 32; i++)
	{
		if((PORTC_PCR(i) & PORT_PCR_ISF_MASK) == PORT_PCR_ISF_MASK)
		{
			//clear interrupt
			PORTC_PCR(i) |= PORT_PCR_ISF_MASK;
			if(gpio_c_callback[i])
			{
				gpio_c_callback[i]();
			}
		}
	}
	
	//check all portd sources
	for(i = 0; i < 32; i++)
		{
			if((PORTD_PCR(i) & PORT_PCR_ISF_MASK) == PORT_PCR_ISF_MASK)
			{
				PORTD_PCR(i) |= PORT_PCR_ISF_MASK;
				if(gpio_d_callback[i])
				{
					gpio_d_callback[i]();
				}
			}
		}
}
