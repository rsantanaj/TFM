/*
 * datalogger_Acquire.h
 *
 *  Created on: 18/11/2019
 *      Author: José Raúl Santana Jiménez
 *
 *  License Creative Commons 3.0 (by-nc-nd)
 */

#ifndef APPS_DATALOGGER_DATALOGGER_ACQUIRE_H_
#define APPS_DATALOGGER_DATALOGGER_ACQUIRE_H_

#define THREAD_PRIORITY_INSTRUMENT            (6)
#define THREAD_STACK_SIZE_INSTRUMENT        (1024)
#include "wiced.h"
#include "datalogger_general.h"

void datalogger_instrument_acquire(int Instrument_Id);

void acquisition_mode_init(void);
void acquisition_mode_start(datalogger_instrument_t* avaliable_instruments);

void aquisition_instrument(uint32_t arg);
void set_stop_instrument_acquiring(int value);

#endif /* APPS_DATALOGGER_DATALOGGER_ACQUIRE_H_ */
