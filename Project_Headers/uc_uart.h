/*
 * uc_uart.h
 *
 *  Created on: June 21, 2014
 *      Author: Severin
 */

#ifndef UC_UART_H_
#define UC_UART_H_

#include "derivative.h"
#include "int.h"

void uc_uart_init();
unsigned char uc_uart_get_data();
void uc_uart_set_callback(callback_t);
void uc_uart_mask_int();
void uc_uart_unmask_int();

#endif /* UC_UART_H_ */
