/*
 * datalogger_MetOcean_TCP_protocol.c
 *
 *  Created on: 10/11/2019
 *      Author: José Raúl Santana Jjiménez
 *
 *  License Creative Commons 3.0 (by-nc-nd)
 */

#include "datalogger_MetOcean_TCP_protocol.h"
#include <string.h>
#include <stdio.h>
#include <inttypes.h>

datalogger_protocol_command_t datalogger_metocean_parser(char *command)
{
    char* numeric_argument_value;
    char aux_value[30];
    int aux_value_length = 0;
    numeric_argument_value=(char *)malloc(20 * sizeof(char));
    char copy_command[40];
    unsigned char actual_char;
    memset(copy_command, '\0', sizeof(copy_command));
    strcpy(copy_command, &command[0]);
    datalogger_protocol_command_t aux_struct_command;
    int aux_int = 0;
    int pointer = 0;
    int num_args = 0;
    int tup_component = 0;
    int arg_magnitude_order = 1;
    actual_char=(unsigned char)copy_command[pointer];
    // Get the primary command
    while (actual_char != '-') {
        aux_int = aux_int*arg_magnitude_order + ((int)(actual_char - '0'));
        arg_magnitude_order = arg_magnitude_order*10;
        pointer++;
        actual_char=(unsigned char)copy_command[pointer];
    };
    aux_struct_command.p_command = aux_int;
    arg_magnitude_order = 1;
    pointer++;
    aux_int = 0;
    actual_char=(unsigned char)copy_command[pointer];
    // Get the secondary command
    while (actual_char != '(') {
        aux_int = aux_int*arg_magnitude_order + ((int)(actual_char - '0'));
        arg_magnitude_order = arg_magnitude_order*10;
        pointer++;
        actual_char=(unsigned char)copy_command[pointer];
    };
    aux_struct_command.s_command = aux_int;
    do {
        aux_int =0;
        arg_magnitude_order = 1;
        pointer++;
        actual_char=(unsigned char)copy_command[pointer];
        while (actual_char != ','){
            aux_int = aux_int*arg_magnitude_order + ((int)(actual_char - '0'));
            arg_magnitude_order = arg_magnitude_order*10;
            pointer++;
            actual_char=(unsigned char)copy_command[pointer];
        };
        pointer ++;
        actual_char=(unsigned char)copy_command[pointer];
        switch ((unsigned int)aux_int) {
        case INSTRUMENT_ID: {
            aux_struct_command.command_args[num_args].argument_type = INSTRUMENT_ID;
            arg_magnitude_order = 1;
            aux_int =0;
            while ((actual_char != ';') && (actual_char != ')')){
                aux_int = aux_int*arg_magnitude_order + ((int)(actual_char - '0'));
                arg_magnitude_order = arg_magnitude_order*10;
                pointer++;
                actual_char=(unsigned char)copy_command[pointer];
            }
            aux_struct_command.command_args[num_args].argument_value.instrument_id = aux_int;
            break;
        }
        case TYPE_INSTRUMENT:{
            arg_magnitude_order = 1;
            aux_int =0;
            while ((actual_char != ';') && (actual_char != ')')){
                aux_int = aux_int*arg_magnitude_order + ((int)(actual_char - '0'));
                arg_magnitude_order = arg_magnitude_order*10;
                pointer++;
                actual_char=(unsigned char)copy_command[pointer];
            }
            aux_struct_command.command_args[num_args].argument_type = TYPE_INSTRUMENT;
            aux_struct_command.command_args[num_args].argument_value.instrument_type = aux_int;
            break;
        }
        case TYPE_MODE:{
            // Only one char for defining the TYPE MODE value
            aux_struct_command.command_args[num_args].argument_type = TYPE_MODE;
            actual_char=(unsigned char)copy_command[pointer];
            aux_struct_command.command_args[num_args].argument_value.datalogger_mode = (int)(actual_char - '0');
            pointer++;
            actual_char=(unsigned char)copy_command[pointer];
            break;
        } case SECONDS_UTC_t:
          case SECONDS_UTC_ACQUIRE_t:
            printf("OBTENIENDO LOS SEGUNDOS EN UTC\n");
            char* buffer;
            char utc_seconds_aux[30];
            wiced_utc_time_ms_t datalogger_datetime;
            actual_char=(unsigned char)copy_command[pointer];
            int seconds_length = 0;
            while ((unsigned char )actual_char != (unsigned char)')' && (unsigned char )actual_char != (unsigned char)';'){
                utc_seconds_aux[seconds_length] = actual_char;
                seconds_length++;
                pointer++;
                actual_char=(unsigned char)copy_command[pointer];
            }
            utc_seconds_aux[seconds_length] = '5';
            utc_seconds_aux[seconds_length+1] = '0';
            utc_seconds_aux[seconds_length+2] = '0';
            buffer=(char *)malloc((seconds_length+3) * sizeof(char));
            sprintf(buffer, "%s", utc_seconds_aux);
            datalogger_datetime = (wiced_utc_time_ms_t)atof(buffer);
            if (aux_int == SECONDS_UTC_t) {
                aux_struct_command.command_args[num_args].argument_type= SECONDS_UTC_t;
                aux_struct_command.command_args[num_args].argument_value.datalogger_utc_datetime = datalogger_datetime;
            } else if(aux_int == SECONDS_UTC_ACQUIRE_t) {
                aux_struct_command.command_args[num_args].argument_type= SECONDS_UTC_ACQUIRE_t;
                aux_struct_command.command_args[num_args].argument_value.datalogger_acquire_utc_datetime = datalogger_datetime;
            }
            break;
          case MEMORY_WRITE_FLAG: {
              aux_struct_command.command_args[num_args].argument_type = MEMORY_WRITE_FLAG;
              aux_struct_command.command_args[num_args].argument_value.acquisition_memory_write_flag = (wiced_bool_t)copy_command[pointer];
              actual_char=(unsigned char)copy_command[pointer];
              pointer++;
              actual_char=(unsigned char)copy_command[pointer];
              break;
          } case MEMORY_READ_ADDRESS:
            case MEMORY_READ_BUFFER_LENGTH:
            case INTERVAL:
            case SAMPLES_PER_INTERVAL:
              actual_char=(unsigned char)copy_command[pointer];
              aux_value_length = 0;
              while ((unsigned char )actual_char != (unsigned char)')' && (unsigned char )actual_char != (unsigned char)';'){
                  aux_value[aux_value_length] = actual_char;
                  aux_value_length++;
                  pointer++;
                  actual_char=(unsigned char)copy_command[pointer];
              }
              sprintf(numeric_argument_value, "%s", aux_value);
              if(aux_int == MEMORY_READ_ADDRESS) {
                  aux_struct_command.command_args[num_args].argument_type = MEMORY_READ_ADDRESS;
                  aux_struct_command.command_args[num_args].argument_value.memory_read_address_value =(uint32_t)atof(numeric_argument_value);
              } else if (aux_int == MEMORY_READ_BUFFER_LENGTH) {
                  aux_struct_command.command_args[num_args].argument_type = MEMORY_READ_BUFFER_LENGTH;
                  aux_struct_command.command_args[num_args].argument_value.memory_read_buffer_length_value = (uint16_t)atof(numeric_argument_value);
              } else if(aux_int == INTERVAL){
                  aux_struct_command.command_args[num_args].argument_type = INTERVAL;
                  aux_struct_command.command_args[num_args].argument_value.sample_interval = (uint16_t)atof(numeric_argument_value);
              } else if (aux_int == SAMPLES_PER_INTERVAL){
                  aux_struct_command.command_args[num_args].argument_type = SAMPLES_PER_INTERVAL;
                  aux_struct_command.command_args[num_args].argument_value.samples_per_interval = (uint8_t)atof(numeric_argument_value);
              }
              break;
            case PROCESSING:{
                // Only one char for defining the PROCESSING value
                aux_struct_command.command_args[num_args].argument_type = PROCESSING;
                actual_char=(unsigned char)copy_command[pointer];
                aux_struct_command.command_args[num_args].argument_value.processing = (int)(actual_char - '0');
                pointer++;
                actual_char=(unsigned char)copy_command[pointer];
                break;
            }
          default:
            // Undefined value
              break;
        };
        num_args++;
    }while (actual_char != ')') ;
    aux_struct_command.number_of_args = num_args;
    return aux_struct_command;
}
