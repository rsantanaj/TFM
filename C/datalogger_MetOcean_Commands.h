/*
 * datalogger__MetOcean_Commands.h
 *
 *  Created on: 12/11/2019
 *      Author: José Raúl Santana Jiménez
 *
 *  License Creative Commons 3.0 (by-nc-nd)
 */

#ifndef APPS_DATALOGGER_DATALOGGER_METOCEAN_COMMANDS_H_
#define APPS_DATALOGGER_DATALOGGER_METOCEAN_COMMANDS_H_
#include "datalogger_MetOcean_TCP_protocol.h"
#include "datalogger_general.h"

//command out contains the string command that will be sent to the datalogger client
extern char* command_out;
extern uint32_t command_out_length;
extern datalogger_mode_satates_t datalogger_actual_mode;
extern wiced_utc_time_ms_t datalogger_programmed_acquire_mode_delay;
extern wiced_bool_t tcp_client_connected;


void metocean_download_data(uint16_t buffer_length, uint32_t memory_address);
void metocean_realtime_data(void);
void datalogger_get_mode(void);
void datalogger_get_date_time(void);
void datalogger_set_instrument(int* num_args, datalogger_protocol_argument_t* args, datalogger_instrument_t instruments[MAX_NUMBER_OF_INSTRUMENTS]);
void datalogger_get_instrument(int instrument_id, datalogger_instrument_t instruments[MAX_NUMBER_OF_INSTRUMENTS]);
void metocean_command_execute(datalogger_protocol_command_t* str_command, datalogger_instrument_t instruments[MAX_NUMBER_OF_INSTRUMENTS]);

void prueba(void);

#endif /* APPS_DATALOGGER_DATALOGGER_METOCEAN_COMMANDS_H_ */
