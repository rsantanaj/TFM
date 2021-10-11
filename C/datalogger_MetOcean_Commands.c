/*
 * datalogger__MetOcean_Commands.c
 *
 *  Created on: 12/11/2019
 *      Author: José Raúl Santana Jiménez
 *
 *  License Creative Commons 3.0 (by-nc-nd)
 */

#include "datalogger_MetOcean_Commands.h"
#include "datalogger_MetOcean_TCP_protocol.h"
#include "datalogger_general.h"
#include "datalogger_Sensors.h"
#include "wiced.h"
#include <stdio.h>
#include <string.h>
#include <inttypes.h>

char* command_out = "";
uint32_t command_out_length = 12;
datalogger_mode_satates_t datalogger_actual_mode = CONFIGURING;
wiced_utc_time_ms_t datalogger_programmed_acquire_mode_delay =0;
wiced_bool_t tcp_client_connected = WICED_FALSE;



void datalogger_get_mode(){
    unsigned char* aux_command;
    char* buffer;
    aux_command = (unsigned char *)malloc(8 * sizeof(unsigned char));
    aux_command[0] = (unsigned char)'1';
    aux_command[1] = (unsigned char)'-';
    aux_command[2] = (unsigned char)'1';
    aux_command[3] = (unsigned char)'(';
    aux_command[4] = (unsigned char)'2'; // "Instrument ID" type argument
    aux_command[5] = (unsigned char)',';
    aux_command[6] = (unsigned char)('0'+datalogger_actual_mode);
    aux_command[7] = (unsigned char) ')';
    command_out_length = 8;

    buffer=(char *)malloc(10 * sizeof(char));
    sprintf(buffer, "%s", aux_command);
    command_out = buffer;

}

// Function for Get Instrument Command.
void datalogger_get_instrument(int instrument_id, datalogger_instrument_t instruments[MAX_NUMBER_OF_INSTRUMENTS]){
    unsigned char* aux_command;
    unsigned char* sample_interval_string = (unsigned char *)malloc(10 * sizeof(unsigned char));
    unsigned char* samples_per_acquisition_string = (unsigned char *)malloc(10 * sizeof(unsigned char));
    unsigned char* processing_string = (unsigned char *)malloc(2 * sizeof(unsigned char));
    char* buffer=(char *)malloc(100 * sizeof(char));;
    aux_command = (unsigned char *)malloc(50 * sizeof(unsigned char));
    aux_command[0] = (unsigned char)'1';
    aux_command[1] = (unsigned char)'-';
    aux_command[2] = (unsigned char)'0';
    aux_command[3] = (unsigned char)'(';
    aux_command[4] = (unsigned char)'0'; // "Instrument ID" type argument
    aux_command[5] = (unsigned char)',';
    aux_command[6] = (unsigned char)('0'+instrument_id);
    aux_command[7] = (unsigned char) ';';
    aux_command[8] = (unsigned char)'1'; // "Type instrument" type argument
    aux_command[9] = (unsigned char)',';
    aux_command[10] = (unsigned char)('0'+ instruments[instrument_id].instrument_type);
    aux_command[11] = (unsigned char) ';';
    aux_command[12] = (unsigned char) '1';
    aux_command[13] = (unsigned char) '6';
    aux_command[14] = (unsigned char) ',';
    command_out_length = unsigned_to_decimal_string  ( instruments[instrument_id].acquisition_interval, sample_interval_string, 1, 10);
    command_out_length = command_out_length + unsigned_to_decimal_string  ( instruments[instrument_id].samples_per_acquisition, samples_per_acquisition_string, 1, 10);
    command_out_length = command_out_length + unsigned_to_decimal_string  ( instruments[instrument_id].processing, processing_string, 1, 1);
    sprintf(buffer, "%s", aux_command);
    strcat(buffer, sample_interval_string);
    strcat(buffer, ";17,");
    strcat(buffer, samples_per_acquisition_string);
    strcat(buffer, ";18,");
    strcat(buffer, processing_string);
    strcat(buffer,")");
    command_out_length = command_out_length+24;
    command_out = buffer;
}

// Function for the Set Instrument Command.
void datalogger_set_instrument(int* num_args, datalogger_protocol_argument_t args[], datalogger_instrument_t instruments[MAX_NUMBER_OF_INSTRUMENTS]){
    datalogger_instrument_t aux_instrument;
    int index_instrument;
    for (int i = 0; i< num_args; i++){
        switch (args[i].argument_type){
            case INSTRUMENT_ID:{
                index_instrument = args[i].argument_value.instrument_id;
                break;
            }
            case TYPE_INSTRUMENT:{
                aux_instrument.instrument_type = args[i].argument_value.instrument_type;
                break;
            }
            case INTERVAL:{
                aux_instrument.acquisition_interval = args[i].argument_value.sample_interval;
                break;
            }
            case SAMPLES_PER_INTERVAL:{
                aux_instrument.samples_per_acquisition = args[i].argument_value.samples_per_interval;
                break;
            }
            case PROCESSING:{
                aux_instrument.processing = args[i].argument_value.processing;
                break;
            }
            default:{
                printf("Undefined argument\n");
                break;
            }
        }

    }
    aux_instrument.enabled = 1;
    instruments[index_instrument] = aux_instrument;
}

void datalogger_get_date_time(void){
    unsigned char* aux_command;
    char* buffer;
    wiced_iso8601_time_t datalogger_datetime_iso;
    wiced_time_get_iso8601_time(&datalogger_datetime_iso);
    aux_command = (unsigned char *)malloc(40 * sizeof(unsigned char));
    aux_command[0] = (unsigned char)'1';
    aux_command[1] = (unsigned char)'-';
    aux_command[2] = (unsigned char)'2';
    aux_command[3] = (unsigned char)'(';
    aux_command[4] = (unsigned char)'3';
    aux_command[5] = (unsigned char)',';
    for (int i = 0; i < 4; i++){
        aux_command[i+6]=(unsigned char)datalogger_datetime_iso.year[i];
    }
    aux_command[10] = (unsigned char)';';
    aux_command[11] = (unsigned char)'4';
    aux_command[12] = (unsigned char)',';
    aux_command[13] = (unsigned char)datalogger_datetime_iso.month[0];
    aux_command[14] = (unsigned char)datalogger_datetime_iso.month[1];
    aux_command[15] = (unsigned char)';';
    aux_command[16] = (unsigned char)'5';
    aux_command[17] = (unsigned char)',';
    aux_command[18] = (unsigned char)datalogger_datetime_iso.day[0];
    aux_command[19] = (unsigned char)datalogger_datetime_iso.day[1];
    aux_command[20] = (unsigned char)';';
    aux_command[21] = (unsigned char)'6';
    aux_command[22] = (unsigned char)',';
    aux_command[23] = (unsigned char)datalogger_datetime_iso.hour[0];
    aux_command[24] = (unsigned char)datalogger_datetime_iso.hour[1];
    aux_command[25] = (unsigned char)';';
    aux_command[26] = (unsigned char)'7';
    aux_command[27] = (unsigned char)',';
    aux_command[28] = (unsigned char)datalogger_datetime_iso.minute[0];
    aux_command[29] = (unsigned char)datalogger_datetime_iso.minute[1];
    aux_command[30] = (unsigned char)';';
    aux_command[31] = (unsigned char)'8';
    aux_command[32] = (unsigned char)',';
    aux_command[33] = (unsigned char)datalogger_datetime_iso.second[0];
    aux_command[34] = (unsigned char)datalogger_datetime_iso.second[1];
    aux_command[35] = (unsigned char)')';
    command_out_length = 36;
    buffer=(char *)malloc(50 * sizeof(char));
    sprintf(buffer, "%s", aux_command);
    command_out = buffer;
    tcp_client_connected = WICED_TRUE;
}

void metocean_realtime_data(void){
    char* buffer;
    char aux_buffer[100];
    aux_buffer[0] = '1';
    aux_buffer[1] = '-';
    aux_buffer[2] = '3';
    aux_buffer[3] = '(';
     if (length_buffer_realtime_out != 0) {
        for (int i = 0; i< length_buffer_realtime_out; i++){
            aux_buffer[4 + i] = internal_buffer_realtime_out[i];
        }
        buffer=(char *)malloc(100 * sizeof(char));
        length_buffer_realtime_out = length_buffer_realtime_out +4;
        command_out_length=length_buffer_realtime_out;
        command_out = buffer;
    }
    else {
        // No real time data to send
        command_out = "1-3(0,0)";
        command_out_length = 8;
    }
     //If real time buffer is full, clean it
    datalogger_sensors_clean_realtime_buffer();
}

void metocean_download_data(uint16_t buffer_length, uint32_t memory_address){

    char* memory_buffer;
    char* aux_buffer;
    char* aux_command;
    char* buffer;
    aux_command = (char *)malloc(2*(buffer_length + 5) * sizeof(char));
    aux_buffer = (char *)malloc((buffer_length + 5) * sizeof(char));
    aux_buffer = "1-4(";

    memory_buffer=(char *)malloc(buffer_length * sizeof(char));
    buffer=(char *)malloc(buffer_length * sizeof(char));
    strcpy(aux_command, aux_buffer);
    datalogger_memory_read(memory_buffer, buffer_length, memory_address);
    sprintf(buffer, "%s", memory_buffer);
    strcat(aux_command, buffer);
    command_out = aux_command;
    command_out_length = buffer_length + 4;
}

void metocean_command_execute(datalogger_protocol_command_t* str_command, datalogger_instrument_t instruments[MAX_NUMBER_OF_INSTRUMENTS]){
    switch (str_command->s_command){
    case INSTRUMENT:{
        if(str_command->p_command == 0){
            datalogger_set_instrument(str_command->number_of_args, str_command->command_args, instruments);
        } else {
            if (str_command->command_args[0].argument_type == INSTRUMENT_ID) {
                datalogger_get_instrument(str_command->command_args[0].argument_value.instrument_id, instruments);

            }

        }
        break;
    } case MODE:{
        if (str_command->p_command == SET) {
            wiced_utc_time_ms_t datalogger_actual_utc_ms;
            wiced_time_get_utc_time_ms(&datalogger_actual_utc_ms);
            if (str_command->command_args[1].argument_value.datalogger_acquire_utc_datetime == 500) {
                datalogger_programmed_acquire_mode_delay=0;
            } else {
                datalogger_programmed_acquire_mode_delay = (str_command->command_args[1].argument_value.datalogger_acquire_utc_datetime) - datalogger_actual_utc_ms;
            }
            datalogger_sensors_clean_realtime_buffer();
            datalogger_set_write_data_to_memory((wiced_bool_t)str_command->command_args[2].argument_value.acquisition_memory_write_flag);
            datalogger_actual_mode = str_command->command_args[0].argument_value.datalogger_mode;
        } else {
            datalogger_get_mode();
        }
        break;
    } case DATE_TIME: {
        if (str_command->p_command == SET) {
            //Set the date and time of the datalogger
            wiced_time_set_utc_time_ms(&str_command->command_args[0].argument_value.datalogger_utc_datetime);
        } else {
            datalogger_get_date_time();
        }
        break;
    } case REALTIME_DATA: {
        metocean_realtime_data();
        break;
    } case MEMORY_DOWNLOAD: {
        char* buffer;
        metocean_download_data(str_command->command_args[1].argument_value.memory_read_buffer_length_value, str_command->command_args[0].argument_value.memory_read_address_value);
        break;
    } case MEMORY_STATE: {
        uint32_t bytes_in_memory;
        char* buffer;
        char* aux;
        char* aux_command;
        aux_command=(char *)malloc(100 * sizeof(char));
        buffer=(char *)malloc(50 * sizeof(char));
        aux = (char *)malloc(50 * sizeof(char));
        aux = "1-5(12,";
        bytes_in_memory = datalogger_get_data_bytes_saved();
        unsigned_to_decimal_string  ( bytes_in_memory, buffer,0,5);
        strcpy(aux_command, aux);
        strcat(aux_command, buffer);
        aux = ")";
        strcat(aux_command, aux);
        command_out = aux_command;
        command_out_length = strlen(command_out);
        break;
    } case MEMORY_ERASE:{
        datalogger_memory_init();
        break;
    }
    default:
        // Undefined command
        break;
    }
}

