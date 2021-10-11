/*
 * datalogger_general.h
 *
 *  Created on: 12/11/2019
 *      Author: José Raúl Santana Jiménez
 *
 *  License Creative Commons 3.0 (by-nc-nd)
 */

#ifndef APPS_DATALOGGER_DATALOGGER_GENERAL_H_
#define APPS_DATALOGGER_DATALOGGER_GENERAL_H_

#define MAX_NUMBER_OF_INSTRUMENTS 8
#include "datalogger_MetOcean_TCP_protocol.h"

typedef struct
{
    datalogger_type_instruments_values_t instrument_type;
    metocean_tcp_protocol_command_arguments_t acquisition_interval;
    metocean_tcp_protocol_command_arguments_t samples_per_acquisition;
    metocean_tcp_protocol_command_arguments_t processing;
    int enabled;

}datalogger_instrument_t;

extern char* command_out;

#endif /* APPS_DATALOGGER_DATALOGGER_GENERAL_H_ */
