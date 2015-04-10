/*
 * main.c
 *
 *  Created on: May 7, 2014
 *      Author: Severin
 */

#include "derivative.h" /* include peripheral declarations */
#include "gpio.h"
#include "uc_pit.h"
#include "uc_spi.h"
#include "uc_sw.h"
#include "CAR_LED.h"
#include "CAR_MOTOR.h"
#include "CAR_XBEE.h"
#include "uc_uart.h"

#define PCKT_NUM 10

typedef enum{
	wait,
	first,
	second,
	valid
} xbee_state;

int switch_3_push = 0;
int switch_1_push = 0;
int xbee_pckt_cnt = 0;
char raw_pckts[PCKT_NUM] = {0,0,0,0,0,0,0,0,0,0}; 
int first_message = 0;
int deadman = 0;
int led_update = 0;
xbee_state xb_s = wait;

//has to run at lower rate than xbee callback
void PIT0_CALLBACK()
{
	//first message keeps car from shutting down on startup
	if(first_message == 1)
	{
		//xbee interrupt didn't occur to set deadman
		if(deadman == 0)
		{
			//shutdown XBee
			uc_uart_mask_int();
			CAR_XBEE_sleep();
			
			//set status LEDs
			CAR_LED_set_color(car_led_0, car_led_ylo);
			CAR_LED_set_color(car_led_1, car_led_ylo);
			CAR_LED_set_color(car_led_2, car_led_ylo);
			CAR_LED_set_color(car_led_3, car_led_ylo);
			CAR_LED_update();
			//shutdown motor
			CAR_MOTOR_shutdown();
			CAR_MOTOR_set_output_en(disable);
			CAR_MOTOR_update();
		}
		//xbee interrupt occurred. reset deadman
		else
		{
			deadman = 0;
		}
	}
}

/*
 * Callback for switch 1
 */
void SW1_CALLBACK()
{
	//toggles motor output
	if(switch_1_push == 0){

		CAR_MOTOR_set_output_en(enable);
		CAR_MOTOR_update();
		switch_1_push = 1;
	}
	else
	{
		CAR_MOTOR_set_output_en(disable);
		CAR_MOTOR_update();
		switch_1_push = 0;
	}
}

/*
 * Callback for switch 3
 */
void SW3_CALLBACK()
{
	//currently unused
	if(switch_3_push == 0)
	{
		switch_3_push = 1;
	}
	else if(switch_3_push == 1)
	{
		switch_3_push = 0;
	}
}

/*
 * Callback for xbee
 */
void XBEE_CALLBACK()
{
	//switches lights when first message is received
	if(first_message == 0)
	{
		first_message = 1;
		CAR_LED_set_color(car_led_0, car_led_wht);
		CAR_LED_set_color(car_led_1, car_led_wht);
		CAR_LED_set_color(car_led_2, car_led_red);
		CAR_LED_set_color(car_led_3, car_led_red);
		led_update = 1;
	}
	char rcv = uc_uart_get_data();
	
	//WAIT STATE
	if(xb_s == wait)
	{
		//if rcv is 0, move state to first
		if(rcv == 0x00)
		{
			xb_s = first;
		}
		//otherwise stay in wait state
	}
	
	//FIRST STATE
	else if(xb_s == first)
	{
		//if second 0 received move to second
		if(rcv == 0x00)
		{
			xb_s = second;
		}
		//else something went wrong and we wait again
		else
		{
			xb_s = wait;
		}
	}
	
	//SECOND STATE
	else if(xb_s == second)
	{
		//loop back to second state again if 0
		if(rcv == 0x00)
		{
			xb_s = second;
		}
		//we move into valid state
		else
		{
			xb_s = valid;
			//we set counter
			xbee_pckt_cnt = 0;
			//we move first valid character into raw rcv buffer
			raw_pckts[xbee_pckt_cnt] = rcv;
			xbee_pckt_cnt++;
		}
	}
	
	//VALID STATE
	else if(xb_s == valid)
	{
		raw_pckts[xbee_pckt_cnt] = rcv;
		xbee_pckt_cnt++;
		
		if(xbee_pckt_cnt == 10)
		{
			xb_s = wait;
			
			char check = raw_pckts[0] ^ raw_pckts[1] ^ raw_pckts[2] ^ raw_pckts[3] ^ raw_pckts[4] ^ raw_pckts[5] ^ raw_pckts[6] ^ raw_pckts[7] ^ raw_pckts[8];
			
			if(check == raw_pckts[9])
			{
				//process these now so we keep TPM interrupt lockout as short as possible.
				unsigned int period0 = 0;
				unsigned int period1 = 0;
				unsigned int period2 = 0;
				unsigned int period3 = 0;
								
				period0 = (raw_pckts[2] << 8) | raw_pckts[1];
				period1 = (raw_pckts[4] << 8) | raw_pckts[3];
				period2 = (raw_pckts[6] << 8) | raw_pckts[5];
				period3 = (raw_pckts[8] << 8) | raw_pckts[7];
								
				int direction0 = raw_pckts[0] & 1;
				int direction1 = (raw_pckts[0] >> 1) & 1;
				int direction2 = (raw_pckts[0] >> 2) & 1;
				int direction3 = (raw_pckts[0] >> 3) & 1;
																
				//access control, because TPM uses these. 
				//we can't modify them while TPM is using them.
				uc_tpm_mask_int();
				 //update actuals
				CAR_MOTOR_set_t_period(motor_0, period0);
				CAR_MOTOR_set_t_period(motor_1, period1);
				CAR_MOTOR_set_t_period(motor_2, period2);
				CAR_MOTOR_set_t_period(motor_3, period3);
				CAR_MOTOR_set_t_direction(motor_0, direction0);
				CAR_MOTOR_set_t_direction(motor_1, direction1);
				CAR_MOTOR_set_t_direction(motor_2, direction2);
				CAR_MOTOR_set_t_direction(motor_3, direction3);
				//set update flags in tpm
				CAR_MOTOR_set_flags();
				
				uc_tpm_unmask_int(); 
				//sets deadman variable to check for active host
				deadman = 1;
				
			}
		}
	}
}

/*
 * clock setup
 */
void clock_init()
{
	//set clock to blazing speed of 48Mhz. 
	//Timers won't run at right frequency unless this is called first.
	unsigned int set_clock= (MCG_C4 & ~0xE0) | 0xA0;
	MCG_C4 = set_clock;
}

/*
 * initializes modules
 */
void init()
{
	//mask all interrupts
	int_all_mask();
	
	//setup clock frequency
	clock_init();
	
	//setup spi
	uc_spi_init(spi_0); // CAR_LED and CAR_MOTOR dependencies
	
	//init car leds
	CAR_LED_init();
	
	//switches configured for interrupts
	uc_sw_init_int(switch_1, SW1_CALLBACK);
	uc_sw_init_int(switch_3, SW3_CALLBACK);

	CAR_MOTOR_init();
	
	CAR_XBEE_init();
	uc_uart_set_callback(XBEE_CALLBACK);
	
	//must be lower frequency than UART receive frequency
	pit_init(pit_0, priority_3, 1000000, PIT0_CALLBACK);
	pit_enable(pit_0);
	
	//enable interrupts
	int_all_unmask();
	
	//safe motor startup
	CAR_MOTOR_motor_startup();	
	CAR_MOTOR_set_output_en(disable);
}


/*
 * main
 */
int main(void)
{
	//initialize hardware
	init();
	
	CAR_LED_set_color(car_led_0, car_led_trq);
	CAR_LED_set_color(car_led_1, car_led_trq);
	CAR_LED_set_color(car_led_2, car_led_trq);
	CAR_LED_set_color(car_led_3, car_led_trq);
	led_update = 1;
	
	while(1)
	{
		if(led_update == 1)
		{
			led_update = 0;
			CAR_LED_update();
			
		}
	}

	return 1;
}





