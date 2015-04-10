/*
 * CAR_MOTOR.c
 *
 *  Created on: October 13, 2014
 *      Author: Severin
 */

#include "CAR_MOTOR.h"

/*MOTOR SETTING FIELDS
 * 
 * 
 * |ST			|MD1	|MD2	|MD3	|OE				|RST	|unused	|unused|
 * 1 active			check manual for MD	 1 active		0 reset
 * 														1 normal
 * 										
 * 
 */

#define A_TABLE_SZ 	13
#define D_TABLE_SZ 	10
#define A_D_CNT_MAX	20
#define FLIP_ZERO	7000
#define VEL_OFF 0xFFFF

//A bunch of globals to save states
static char car_motor = 0b00000000;

static volatile CAR_MOTOR_dir_t target_direction[4] ={1,1,1,1};
static volatile CAR_MOTOR_dir_t current_direction[4] ={1,1,1,1};
static volatile unsigned int current_period[4] ={VEL_OFF,VEL_OFF,VEL_OFF,VEL_OFF};
static volatile unsigned int target_period[4] ={VEL_OFF,VEL_OFF,VEL_OFF,VEL_OFF};
static volatile unsigned int previous_period[4] = {VEL_OFF,VEL_OFF,VEL_OFF,VEL_OFF};
static volatile int update_flag[4] = {0,0,0,0};

static int a_d_cnt[4] = {0, 0, 0, 0};

unsigned int get_a_period(unsigned int, unsigned int);
unsigned int get_d_period(unsigned int, unsigned int);
void CAR_MOTOR_set_direction(CAR_MOTOR_motor_t, CAR_MOTOR_dir_t);
unsigned int CAR_MOTOR_compute_period(CAR_MOTOR_motor_t , CAR_MOTOR_dir_t c_direction, CAR_MOTOR_dir_t t_direction, unsigned int c_period, unsigned int t_period);

//acceleration table
/*
65535	574.8127001
32767	572.286256
16383	567.2667428
8191	557.3611758
4095	538.0830159
2047	501.6431061
1023	436.9772255
511		336.9501875
255		218.4828964
127		121.3861211
63		62.26720821
31		30.91113864
15		14.98988865
*/

static const unsigned int a_bound[A_TABLE_SZ] = {15, 	31, 	63, 	127, 	255, 	511, 	1023, 	2047, 	4095, 	8191, 	16383, 	32767, 	65535};
static const unsigned int a_table[A_TABLE_SZ] = {15, 	31, 	63, 	126, 	247, 	457, 	741, 	1007, 	1190, 	1297, 	1354, 	1384, 	1399};

//deceleration table
/*
574	49602
572	31006
568	17680
560	9459
544	4848
512	2398
448	1126
320	462
64	65
2	2
*/

static const unsigned int d_bound[D_TABLE_SZ] = {10, 	450, 	898, 	1153, 	1280, 	1343, 	1374, 	1389, 	1396, 	1399};
static const unsigned int d_table[D_TABLE_SZ] = {10, 	501, 	1505, 	3439, 	7080, 	13680, 	24509, 	39305, 	54549, 	65534};

/*
 * Initializes pins to control shift registers and configures "direction" pins as GPIO
 * SPI needs to be initialized separately
 */
void CAR_MOTOR_init()
{
	//set up current-limiting digital - analog converter. still needs separate call to set specific voltage
	uc_dac_init();
	
	CAR_MOTOR_set_MD(step_128);
	CAR_MOTOR_update();
	
	//CS/RCLK
	gpio_port_init(port_E, pin_16, alt_1, output);
	gpio_set_pin_state(port_E, pin_16, 1);
	
	//output enable
	gpio_port_init(port_B, pin_1, alt_1, output);
	gpio_set_pin_state(port_B, pin_1, 0);
		
	//clear and reset
	gpio_port_init(port_B, pin_2, alt_1, output);
	gpio_set_pin_state(port_B, pin_0, 0);
	gpio_set_pin_state(port_B, pin_0, 1);
	
	//direction
	gpio_port_init(port_E, pin_0, alt_1, output); 
	gpio_port_init(port_E, pin_1, alt_1, output);
	gpio_port_init(port_E, pin_2, alt_1, output);
	gpio_port_init(port_E, pin_3, alt_1, output);
	
	uc_tpm_init();
	uc_tpm_set_callback(tpm_chan_2, CAR_MOTOR_CALLBACK_0);
	uc_tpm_set_callback(tpm_chan_3, CAR_MOTOR_CALLBACK_1);
	uc_tpm_set_callback(tpm_chan_4, CAR_MOTOR_CALLBACK_2);
	uc_tpm_set_callback(tpm_chan_5, CAR_MOTOR_CALLBACK_3);
	uc_tpm_set_callback(tpm_chan_1, WAKEUP_CALLBACK);

	
	CAR_MOTOR_set_direction(motor_0, target_direction[motor_0]);
	CAR_MOTOR_set_direction(motor_1, target_direction[motor_1]);
	CAR_MOTOR_set_direction(motor_2, target_direction[motor_2]);
	CAR_MOTOR_set_direction(motor_3, target_direction[motor_3]);
}

/*
 *  updates shift register outputs to whatever was set in global setting variable
 */
void CAR_MOTOR_update()
{
	//CS/RCLK low
	gpio_set_pin_state(port_E, pin_16, 0);
		
	//send four times to update all motors
	uc_spi_send(spi_0, car_motor);
	uc_spi_send(spi_0, car_motor);
	uc_spi_send(spi_0, car_motor);
	uc_spi_send(spi_0, car_motor);

	//CS/RCLK back up
	gpio_set_pin_state(port_E, pin_16, 1);	
}

/*
 * sets MD on the LV8727
 */
void CAR_MOTOR_set_MD(CAR_MOTOR_step_size_t p_step_size)
{
	car_motor &= ~(7 << 4);
	car_motor |= p_step_size << 4;
}

/*
 * enables/disables the LV8727
 */
void CAR_MOTOR_set_chip_en(CAR_MOTOR_state p_state)
{
	car_motor &= ~(1 << 7);
	car_motor |= p_state << 7;
}

/*
 * enables/disables LV8727 output
 */
void CAR_MOTOR_set_output_en(CAR_MOTOR_state p_state)
{
	car_motor &= ~(1 << 3);
	car_motor |= p_state << 3;
}

/*
 * sets the reset on the LV8727
 */
void CAR_MOTOR_set_rst(CAR_MOTOR_state p_state)
{
	car_motor &= ~(1 << 2);
	car_motor |= p_state << 2;
}

/*
 * performs a reset cycle on LV8727 automatically
 */
void CAR_MOTOR_set_rst_cycle()
{
	CAR_MOTOR_set_rst(disable);
	CAR_MOTOR_update();
	CAR_MOTOR_set_rst(enable);
	CAR_MOTOR_update();
}

/*
 * enables enables current limiter pin for LV8727 current limiting
 */
void CAR_MOTOR_set_current_limiter_en(CAR_MOTOR_state p_state)
{
	if(p_state)
	{
		/*
		dac param	voltage (V)		output current (A)
		0			0.000805664		0.000994647
		200			0.161938477		0.199924045
		400			0.323071289		0.398853443
		600			0.484204102		0.597782841
		800			0.645336914		0.79671224
		1000		0.806469727		0.995641638
		1200		0.967602539		1.194571036
		1400		1.128735352		1.393500434
		1600		1.289868164		1.592429832
		 */

		//needs tuning
		uc_dac_set_output(1100);	
	}
	else
	{
		uc_dac_set_output(0);
	}
}

/*
 * get acceleration period based on old period and target
 */
unsigned int get_a_period(unsigned int c_period, unsigned int t_period)
{
	int i = 1;
	//loop through entire table. exit early if case matched and value was set.
	while(i <= A_TABLE_SZ)
	{
		if(c_period < a_bound[i])
		{
			unsigned int base_period = a_table[i-1];
			unsigned int period_adjust = c_period - a_bound[i-1];
			unsigned int numerator = period_adjust*(a_table[i] - a_table[i-1]);
			unsigned int offset = numerator/(a_bound[i] - a_bound[i-1]);
			return base_period + offset;
		}
		i++;
	}
	return a_table[A_TABLE_SZ-1];
}

/*
 * get deceleration period based on old period and target
 */
unsigned int get_d_period(unsigned int c_period, unsigned int t_period)
{
	int i = 1;
	//loop through entire table. exit early if case matched and value was set.
	while(i <= D_TABLE_SZ)
	{
		if(c_period < d_bound[i])
		{
			unsigned int base_period = d_table[i-1];
			unsigned int period_adjust = c_period - d_bound[i-1];
			unsigned int numerator = period_adjust*(d_table[i] - d_table[i-1]);
			unsigned int offset = numerator/(d_bound[i] - d_bound[i-1]);
			return base_period + offset;
		}
		i++;
	}
	return t_period;
}

/*
 * Should be called by TPM and performs motor 0 stepping
 */
void CAR_MOTOR_CALLBACK_0()
{
	//local copies. don't want the uart interrupt changing these while we're using them.
	//not sure if it matters, but it's safe.
	unsigned int c_period = current_period[0];
	unsigned int t_period = target_period[0];
	CAR_MOTOR_dir_t c_direction= current_direction[0];
	CAR_MOTOR_dir_t t_direction= target_direction[0];
	
	unsigned int final_period = CAR_MOTOR_compute_period(motor_0 , c_direction, t_direction, c_period, t_period);
	
	previous_period[0] = current_period[0];
	current_period[0] = final_period;
	
	uc_tpm_set_compare_val(tpm_chan_2, final_period);		
	
	update_flag[0] = 0;
}

/*
 * Should be called by TPM and performs motor 1 stepping
 */
void CAR_MOTOR_CALLBACK_1()
{
	unsigned int c_period = current_period[1];
	unsigned int t_period = target_period[1];
	CAR_MOTOR_dir_t c_direction= current_direction[1];
	CAR_MOTOR_dir_t t_direction= target_direction[1];
		
	unsigned int final_period = CAR_MOTOR_compute_period(motor_1 , c_direction, t_direction, c_period, t_period);
		
	previous_period[1] =current_period[1];
	current_period[1] = final_period;
		
	uc_tpm_set_compare_val(tpm_chan_3, final_period);

	update_flag[1] = 0;
}

/*
 * Should be called by TPM and performs motor 2 stepping
 */
void CAR_MOTOR_CALLBACK_2()
{
	unsigned int c_period = current_period[2];
	unsigned int t_period = target_period[2];
	CAR_MOTOR_dir_t c_direction= current_direction[2];
	CAR_MOTOR_dir_t t_direction= target_direction[2];
			
	unsigned int final_period = CAR_MOTOR_compute_period(motor_2 , c_direction, t_direction, c_period, t_period);
			
	previous_period[2] =current_period[2];
	current_period[2] = final_period;
			
	uc_tpm_set_compare_val(tpm_chan_4, final_period);

	update_flag[2] = 0;
}

/*
 * Should be called by TPM and performs motor 3 stepping
 */
void CAR_MOTOR_CALLBACK_3()
{
	unsigned int c_period = current_period[3];
	unsigned int t_period = target_period[3];
	CAR_MOTOR_dir_t c_direction= current_direction[3];
	CAR_MOTOR_dir_t t_direction= target_direction[3];
			
	unsigned int final_period = CAR_MOTOR_compute_period(motor_3 , c_direction, t_direction, c_period, t_period);
			
	previous_period[3] =current_period[3];
	current_period[3] = final_period;
			
	uc_tpm_set_compare_val(tpm_chan_5, final_period);

	update_flag[3] = 0;
}

/*
 * called by channel 1 to update other channels preemptively. Triggered by UART
 */
void WAKEUP_CALLBACK()
{
	//set channel back to sleep mode
	uc_tpm_set_compare_val(tpm_chan_1, VEL_OFF);
	
	int i = 0;
	//check if motors need updating
	for(i = 0; i < 4; i++)
	{
		//check if the regular interrupt occurred after values were update. 
		//If flag is 0, update already occured on regular motor interrupt.
		if(update_flag[i] == 1)
		{
			//if current velocity is 0...
			if(current_period[i] == VEL_OFF)
			{
				//...and we want to accelerate
				if(target_period[i] != VEL_OFF)
				{
					//switch to new direction and update current
					current_direction[i] = target_direction[i];
					CAR_MOTOR_set_direction(i, target_direction[i]);
					//interrupt ASAP
					uc_tpm_pulse_asap(i+2);
					//update current period to slowest period
					current_period[i] = VEL_OFF-1;	
				}
			}
			else
			{
				//check how much time is left
				unsigned int time_left = uc_tpm_time_left(i+2);
				
				if(time_left > 3)
				{
					unsigned int final_period = CAR_MOTOR_compute_period(i, current_direction[i], target_direction[i], previous_period[i], target_period[i]);
					
					if(final_period == VEL_OFF)
					{
						uc_tpm_sleep(i+2);
						current_period[i] = VEL_OFF;
						previous_period[i] = VEL_OFF;
					}
					else
					{
						if(final_period > current_period[i])
						{
							//rewrite comparison value
							uc_tpm_set_compare_val(i+2, final_period - current_period[i]);
						}
						else if(current_period[i] > final_period)
						{
							unsigned int time_difference = current_period[i] - final_period;
							//counter hasnt passed pulse point yet
							if(time_difference <= (time_left + 3))
							{
								uc_tpm_set_neg_compare_value(i+2, time_difference);
							}
							//counter passed pulse point...
							else
							{
								uc_tpm_pulse_asap(i+2);
							}
						}
						current_period[i] = final_period;				
					}
				}
			}
			update_flag[i] = 0;
		}
	}
}

/*
 * sets the direction for a specific motor
 */
void CAR_MOTOR_set_direction(CAR_MOTOR_motor_t p_motor, CAR_MOTOR_dir_t p_dir)
{
	if(p_motor == motor_0 || p_motor == motor_2)
	{
		gpio_set_pin_state(port_E, p_motor, !p_dir);
	}
	else
	{
		gpio_set_pin_state(port_E, p_motor, p_dir);
	}
}

/*
 * sets new target period
 */
void CAR_MOTOR_set_t_period(CAR_MOTOR_motor_t p_motor, unsigned int p_target)
{
	target_period[p_motor]=p_target;

}

void CAR_MOTOR_set_t_direction(CAR_MOTOR_motor_t p_motor, CAR_MOTOR_dir_t p_dir)
{
	target_direction[p_motor]=p_dir;
}

/*
 * starts up LV8727s safely.
 */
void CAR_MOTOR_motor_startup()
{
	//motor startup sequence
	//set a md and turn off output. enable chip.
	//update motors
	
	CAR_MOTOR_set_MD(step_2);
	CAR_MOTOR_set_output_en(disable);
	CAR_MOTOR_set_chip_en(enable);
	CAR_MOTOR_update();
	
	//start current limiting analog value
	CAR_MOTOR_set_current_limiter_en(enable);
	
	//reset chip, since it just intitialized
	//rst_cycle() auto-cycles and updates
	CAR_MOTOR_set_rst_cycle();
	
}

unsigned int CAR_MOTOR_compute_period(CAR_MOTOR_motor_t p_motor, CAR_MOTOR_dir_t c_direction, CAR_MOTOR_dir_t t_direction, unsigned int c_period, unsigned int t_period)
{
	unsigned int final_period = 0;
	//if same direction, simple a or d
	if(c_direction == t_direction)
	{
		//too fast, decelerate
		if(c_period < t_period)
		{
			//get decel value
			final_period = get_d_period(c_period, t_period);
			
			//if no progress is made, begin acceleration counter manual increment.
			if(final_period == c_period)
			{
				//end of acceleration counter reached, manually increment and reset counter for next iteration
				if(a_d_cnt[p_motor] == A_D_CNT_MAX)
				{
					final_period++;
					a_d_cnt[p_motor] = 0;
				}
				//increment counter
				else
				{
					a_d_cnt[p_motor]++;
				}
			}
			//reset counter just in case.
			else
			{
				a_d_cnt[p_motor] = 0;
			}
			
			//check if target overshot and correct
			if(final_period > t_period)
			{
				final_period = t_period;
			}
		}
		
		//too slow, accelerate
		else if(c_period > t_period)
		{
			final_period = get_a_period(c_period, t_period);
			
			//if no progress is made, begin acceleration counter manual increment.
			if(final_period == c_period)
			{
				//end of acceleration counter reached, manually increment and reset counter for next iteration
				if(a_d_cnt[p_motor] == A_D_CNT_MAX)
				{
					final_period--;
					a_d_cnt[p_motor] = 0;
				}
				//increment counter
				else
				{
					a_d_cnt[p_motor]++;
				}
			}
			//reset counter just in case.
			else
			{
				a_d_cnt[p_motor] = 0;
			}
			
			//check if target overshot and correct
			if(final_period < t_period)
			{
				final_period = t_period;
			}
		}
			
		//target speed reached, continue
		else
		{
			final_period = t_period;
		}
	}
	
	//decelerate until direction flip
	else
	{
		//current period is already so low that we accelerate right away
		if(c_period >= FLIP_ZERO)
		{
			//flip direction
			current_direction[p_motor] = t_direction;
			CAR_MOTOR_set_direction(p_motor, t_direction);

			//get acceleration because we're speeding up now
			final_period = get_a_period(c_period, t_period);
			
			//check if target overshot and correct for overshoot
			if(final_period < t_period)
			{
				final_period = t_period;
			}
		}
		//haven't slowed down enough yet
		else
		{
			//if we overshoot "0" speed via deceleration, set at "0" speed = 2000 period
			final_period = get_d_period(c_period, 65535);
			
			if(final_period == c_period)
			{
							//end of acceleration counter reached, manually increment and reset counter for next iteration
				if(a_d_cnt[p_motor] == A_D_CNT_MAX)
				{
					final_period++;
					a_d_cnt[p_motor] = 0;
				}
				//increment counter
				else
				{
					a_d_cnt[p_motor]++;
				}
			}
			//reset counter just in case.
			else
			{
				a_d_cnt[p_motor] = 0;
			}
			
			if(final_period > FLIP_ZERO)
			{
				final_period = FLIP_ZERO;
				//flip direction
				current_direction[p_motor] = t_direction;
				CAR_MOTOR_set_direction(p_motor, t_direction);
			}
		}
	}	
	
	return final_period;
}


void CAR_MOTOR_shutdown()
{
	//stop all motors
	CAR_MOTOR_set_t_period(motor_0, VEL_OFF);
	CAR_MOTOR_set_t_period(motor_1, VEL_OFF);
	CAR_MOTOR_set_t_period(motor_2, VEL_OFF);
	CAR_MOTOR_set_t_period(motor_3, VEL_OFF);
	CAR_MOTOR_set_t_direction(motor_0, CAR_MOTOR_dir_b);
	CAR_MOTOR_set_t_direction(motor_1, CAR_MOTOR_dir_b);
	CAR_MOTOR_set_t_direction(motor_2, CAR_MOTOR_dir_b);
	CAR_MOTOR_set_t_direction(motor_3, CAR_MOTOR_dir_b);
	
	//spin until all motors are stopped
	int stop_flag = 0;
	do{
		int i = 0;
		for(i = 0; i < 4; i++)
		{
			if(current_direction[i] != VEL_OFF)
			{
				stop_flag = 1;
			}
		}
	}while(stop_flag == 1);
}


void CAR_MOTOR_set_flags()
{

	update_flag[0] = 1;
	update_flag[1] = 1;
	update_flag[2] = 1;
	update_flag[3] = 1;
	
	//set tpm to go off ASAP
	uc_tpm_pulse_asap(tpm_chan_1);
}

