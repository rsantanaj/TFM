/*
 * datalogger_Sensors.h
 *
 *  Created on: 19/11/2019
 *      Author: José Raúl Santana Jiménez
 *
 *  License Creative Commons 3.0 (by-nc-nd)
 */

#ifndef APPS_DATALOGGER_DATALOGGER_SENSORS_H_
#define APPS_DATALOGGER_DATALOGGER_SENSORS_H_
#include "datalogger_general.h"
#include "wiced.h"

#define BUFFER_REALTIME_OUT_MAX_LENGTH 100
#define MAX_RECEIVED_DATA_LENGTH_SBE37 100
#define MAX_NUMBER_OF_SAMPLES_PER_ACQUISITION 50

// Defines for the SBE 37 available parameters*******
#define TEMPERATURE 0
#define CONDUCTIVITY 1
#define PRESSURE 2
#define OXYGEN 3
#define SALINITY 4
//***************************************************


#define MEDIAN_t 2
#define AVERAGE_t 1


extern char* buffer_realtime_out;
extern char internal_buffer_realtime_out[200];
extern int length_buffer_realtime_out;

void SBE_print_buffer(uint32 arg);

float datalogger_median(float *vector_values, int vector_length);

float datalogger_average(float *vector_values, int vector_length);

wiced_bool_t datalogger_get_write_data_to_memory(void);

void datalogger_set_write_data_to_memory(wiced_bool_t write_data);

void datalogger_check_realtime_buffer_max_length(void);

void datalogger_sensors_clean_realtime_buffer(void);

void datalogger_sensors_init(void);

void datalogger_SBE37_acquire(datalogger_instrument_t* instrument, int instrument_id);


#endif /* APPS_DATALOGGER_DATALOGGER_SENSORS_H_ */
