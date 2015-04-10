/*
 * uc_spi.h
 *
 *  Created on: June 15, 2014
 *      Author: Severin
 */

#ifndef UC_SPI_H_
#define UC_SPI_H_

#include "derivative.h"
#include "gpio.h"

typedef enum{
	spi_0,
	spi_1	
} spi_t;

void uc_spi_init(spi_t);
void uc_spi_send(spi_t, char);

#endif /* UC_SPI_H_ */

