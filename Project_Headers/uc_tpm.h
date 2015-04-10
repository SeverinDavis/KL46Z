/*
 * uc_tpm.h
 *
 *  Created on: June 15, 2014
 *      Author: Severin
 */

#ifndef UC_TPM_H_
#define UC_TPM_H_

#include "derivative.h"
#include "gpio.h"
#include "int.h"

typedef enum{
	tpm_chan_0,
	tpm_chan_1,
	tpm_chan_2,
	tpm_chan_3,
	tpm_chan_4,
	tpm_chan_5
} tpm_chan_t;

void uc_tpm_init();
void uc_tpm_set_callback(tpm_chan_t, callback_t);
void uc_tpm_set_compare_val(tpm_chan_t, int);
void uc_tpm_unmask_int();
void uc_tpm_mask_int();
unsigned int uc_tpm_time_left(tpm_chan_t);
void uc_tpm_pulse_asap(tpm_chan_t p_tpm_chan);
void uc_tpm_set_neg_compare_value(tpm_chan_t p_tpm_chan, unsigned neg_value);
void uc_tpm_sleep(tpm_chan_t p_tpm_chan);

#endif /* UC_TPM_H_ */
