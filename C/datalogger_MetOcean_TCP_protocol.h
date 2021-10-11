/*
 * datalogger_MetOcean_TCP_protocol.h
 *
 *  Created on: 10/11/2019
 *      Author: José Raúl Santana Jiménez
 *
 *  License Creative Commons 3.0 (by-nc-nd)
 */

#ifndef APPS_DATALOGGER_DATALOGGER_METOCEAN_TCP_PROTOCOL_H_
#define APPS_DATALOGGER_DATALOGGER_METOCEAN_TCP_PROTOCOL_H_
#include <string.h>
#include <stdlib.h>
#include "wiced.h"

#define MAX_NUMBER_OF_TCP_PROTOCOL_ARGUMENTS 5
#define MAX_COMMAND_RESPONSE_LENGTH 10

// Commands defined for the MetOcean TCP protocol
typedef enum
{
    SET,
    GET
} metocean_tcp_protocol_primary_commands_t;


typedef enum
{
    INSTRUMENT,
    MODE,
    DATE_TIME,
    REALTIME_DATA,
    MEMORY_DOWNLOAD,
    MEMORY_STATE,
    MEMORY_ERASE
}metocean_tcp_protocol_secondary_commands_t;

// Arguments defined for the MetOcean TCP protocol commands
typedef enum{

    INSTRUMENT_ID,
    TYPE_INSTRUMENT,
    TYPE_MODE,
    YEAR_t,
    MONTH_t,
    DAY_t,
    HOUR_T,
    MINUTE_t,
    SECOND_t,
    SECONDS_UTC_t,
    SECONDS_UTC_ACQUIRE_t,
    REALTIME_DATA_arg,
    MEMORY_STATE_Arg,
    MEMORY_WRITE_FLAG,
    MEMORY_READ_ADDRESS,
    MEMORY_READ_BUFFER_LENGTH,
    INTERVAL,
    SAMPLES_PER_INTERVAL,
    PROCESSING
}metocean_tcp_protocol_command_arguments_t;

// Values defined for TYPE_INSTRUMENT argument
typedef enum{
    NONE_It,
    SBE_37SM
}datalogger_type_instruments_values_t;

// Values defined for MODE  secundary command
typedef enum{
    CONFIGURING,
    ACQUIRING
} datalogger_mode_satates_t;

/**
 * MetOcean TCP protocol Result Type
 */
typedef enum
{
    PARSER_SUCESS,
    COMMAND_NOT_SUPPORTED
} metocean_tcp_protocol_result_t;

typedef enum
{
    NONE_p,
    AVERAGE,
    MEDIAN
}metocean_tcp_protocol_processing_values;

//This union defines all arguments that can be used in all commands
typedef union
{
    int instrument_id;
    int instrument_type;
    int datalogger_mode;
    wiced_utc_time_ms_t datalogger_utc_datetime;
    wiced_utc_time_ms_t datalogger_acquire_utc_datetime;
    wiced_bool_t acquisition_memory_write_flag;
    uint32_t memory_read_address_value;
    uint16_t memory_read_buffer_length_value;
    uint16_t sample_interval;
    uint8_t samples_per_interval;
    metocean_tcp_protocol_processing_values processing;

}datalogger_protocol_arg_t;

//this struct contains one pair type argument, argument value
typedef struct
{
    metocean_tcp_protocol_command_arguments_t argument_type;
    datalogger_protocol_arg_t argument_value;
}datalogger_protocol_argument_t;

//This is the struct obtained after parsing one MetOcean TCP protocol Word.
//After parsing, this struct will be passed to the execute command pipeline.
typedef struct
{
    metocean_tcp_protocol_primary_commands_t p_command;
    metocean_tcp_protocol_secondary_commands_t s_command;
    int number_of_args;
    datalogger_protocol_argument_t command_args[MAX_NUMBER_OF_TCP_PROTOCOL_ARGUMENTS];
    char* response[MAX_COMMAND_RESPONSE_LENGTH];

}datalogger_protocol_command_t;


datalogger_protocol_command_t datalogger_metocean_parser(char *command);

typedef enum
{
    DATALOGGER_STATE_SET,
    DATALOGGER_STATE_GET
} metocean_tcp_protocol_commands_t;

#endif /* APPS_DATALOGGER_DATALOGGER_METOCEAN_TCP_PROTOCOL_H_ */
