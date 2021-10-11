/*
 * datalogger_Sensors.c
 *
 *  Created on: 19/11/2019
 *      Author: José Raúl Santana Jiménez
 */
#include "datalogger_Sensors.h"
#include "datalogger_Serial_Port.h"
#include "datalogger_memory.h"

// Mutex for locking the memory access between the sensor threads
static wiced_mutex_t memory_access_mutex;


char* buffer_realtime_out = "";
char internal_buffer_realtime_out[200];
int length_buffer_realtime_out = 0;
wiced_bool_t write_data_to_memory = WICED_TRUE;


float datalogger_median(float *vector_values, int vector_length){
  float median = 0.0;
  float aux_value = 0.0;
  int minimun_value_pointer = 0;
  int median_pointer = 0;
  // sort vector_values from min to max values
  for (int i = 0; i< vector_length; i++){
      minimun_value_pointer = i;
      for (int j = minimun_value_pointer+1; j<vector_length; j++) {
          if (vector_values[j] < vector_values[i]){
              minimun_value_pointer = j;
          }
      }
      aux_value = vector_values[i];
      vector_values[i] = vector_values[minimun_value_pointer];
      vector_values[minimun_value_pointer] = aux_value;
  }
  median_pointer = vector_length / 2;
  median = vector_values[median_pointer];
  return median;
}

float datalogger_average(float *vector_values, int vector_length){
    float average = 0.0;
    for (int i = 0; i< vector_length; i++) {
        average = average + vector_values[i];
    }
    average = average /vector_length;
    return average;
}


wiced_bool_t datalogger_get_write_data_to_memory(void){
    return write_data_to_memory;
}

void datalogger_set_write_data_to_memory(wiced_bool_t write_data){
    write_data_to_memory = write_data;
}

void datalogger_sensors_init(void) {
    buffer_realtime_out=(char *)malloc(100 * sizeof(char));
    buffer_realtime_out = "";
     wiced_rtos_init_mutex(&memory_access_mutex);

}

void datalogger_sensors_clean_realtime_buffer(void){
    buffer_realtime_out = "";
    for (int i = 0; i < length_buffer_realtime_out; i++){
        internal_buffer_realtime_out[length_buffer_realtime_out] = NULL;
    }
    length_buffer_realtime_out = 0;
}

void datalogger_check_realtime_buffer_max_length(void){
    if (length_buffer_realtime_out >= BUFFER_REALTIME_OUT_MAX_LENGTH){
        datalogger_sensors_clean_realtime_buffer();
    }
}

void SBE_print_buffer(uint32 arg){
    print_buffer(arg);
}

void datalogger_obtain_datetime_string(char *datetime_buffer){
    wiced_iso8601_time_t datalogger_datetime_iso;
    char *year = (char *)malloc(5 * sizeof(char));
    char *month = (char *)malloc(3 * sizeof(char));
    char *day = (char *)malloc(3 * sizeof(char));
    char *hour = (char *)malloc(3 * sizeof(char));
    char *minutes = (char *)malloc(3 * sizeof(char));
    char *seconds = (char *)malloc(3 * sizeof(char));
    wiced_time_get_iso8601_time(&datalogger_datetime_iso);
    sprintf(year, "%s", datalogger_datetime_iso.year);
    sprintf(month, "%s", datalogger_datetime_iso.month);
    sprintf(day, "%s", datalogger_datetime_iso.day);
    sprintf(hour, "%s", datalogger_datetime_iso.hour);
    sprintf(minutes, "%s", datalogger_datetime_iso.minute);
    sprintf(seconds, "%s", datalogger_datetime_iso.second);
    strcpy(datetime_buffer, month);
    strcat(datetime_buffer, "/");
    strcat(datetime_buffer, day);
    strcat(datetime_buffer, "/");
    strcat(datetime_buffer, year);
    strcat(datetime_buffer, " ");
    strcat(datetime_buffer, hour);
    strcat(datetime_buffer, ":");
    strcat(datetime_buffer, minutes);
    strcat(datetime_buffer, ":");
    strcat(datetime_buffer, seconds);
}

void datalogger_SBE37_acquire(datalogger_instrument_t* instrument, int instrument_id){
    char SBE37_command[3];
    char *string_data_result = (char *)malloc(100 * sizeof(char));
    char *instrument_string = (char *)malloc(20 * sizeof(char));
    char *instrument_id_char = (char *)malloc(sizeof(char));
    char * parameter_string = (char *)malloc(10 * sizeof(char));
    int number_of_bytes_received = 0;
    float temperature[MAX_NUMBER_OF_SAMPLES_PER_ACQUISITION] = {0.0};
    float conductivity[MAX_NUMBER_OF_SAMPLES_PER_ACQUISITION] = {0.0};
    float pressure[MAX_NUMBER_OF_SAMPLES_PER_ACQUISITION] = {0.0};
    float oxygen[MAX_NUMBER_OF_SAMPLES_PER_ACQUISITION] = {0.0};
    float salinity[MAX_NUMBER_OF_SAMPLES_PER_ACQUISITION] = {0.0};
    float base = 10.0;
    float temperature_final = 0.0;
    float conductivity_final = 0.0;
    float pressure_final = 0.0;
    float oxygen_final = 0.0;
    float salinity_final = 0.0;
    int sign = 1;
    int frames_received = 0;
    int data_parameter = TEMPERATURE;
    float aux_data_int_part = 0.0; //integer part of one value
    float aux_data_frac_part = 0.0; //fractional par of one value
    float aux_data = 0.0; // auxiliar value used during conversion from string to float
    wiced_bool_t frame_end = WICED_FALSE;
    wiced_bool_t fractional = WICED_FALSE;

    char* datalogger_datetime = (char *)malloc(20* sizeof(char));
    int k = -1;
    SBE37_command[0] = 't';
    SBE37_command[1] = 's';
    SBE37_command[2] = '\r';
    int memory_write_buffer_length = 0;
    uint8 input_data[MAX_RECEIVED_DATA_LENGTH_SBE37];
    datalogger_check_realtime_buffer_max_length();
    for (int l = 0; l< instrument->samples_per_acquisition; l++) {
        serial_send_string(SBE37_command, 3, instrument_id, 9600);
        wiced_rtos_delay_milliseconds(4000); //time to allow the SBE37 to send the acquired data
        if (serial_new_byte_received(instrument_id)){

                number_of_bytes_received = get_serialport_buffer_length(instrument_id);
                if (number_of_bytes_received <= MAX_RECEIVED_DATA_LENGTH_SBE37){ //search the '<' char
                    k = -1;
                    do {
                        k++;
                        input_data[k] = serial_port_buffer_get_byte(instrument_id, k);
                        if (input_data[k] == '<') {
                            frame_end = WICED_TRUE;
                        }
                    }while(frame_end == WICED_FALSE);
                }
                if (frame_end == WICED_TRUE) {
                    k--;
                    frames_received++;
                    for (int i = 0; i<k; i++){
                        if ((unsigned char)input_data[i] != ','){
                            if((unsigned char)input_data[i] != ' '){
                                if((unsigned char)input_data[i] == '-'){
                                    sign = -1;
                                }
                                if ((unsigned char)input_data[i] == '.'){
                                    fractional = WICED_TRUE;
                                    base = 0.1;
                                }
                                if (fractional) {// Obtain the fractional part of the data
                                    aux_data_frac_part = aux_data_frac_part + ((float)(input_data[i] - '0')*base);
                                    base = base*0.1;
                                } else { //Obtain the integer part of the data
                                    aux_data_int_part = aux_data_int_part*base + ((float)(input_data[i] - '0'));
                                    base = base*10;
                                }
                            }
                        } else {
                            fractional = WICED_FALSE;
                            base = 10.0;
                            aux_data = ((float)sign*aux_data_int_part)+aux_data_frac_part;
                            switch (data_parameter) {
                            case TEMPERATURE:{
                                temperature[l] = aux_data;
                                data_parameter = CONDUCTIVITY;
                                break;
                            }
                            case CONDUCTIVITY:{
                                conductivity[l] = aux_data;
                                data_parameter = PRESSURE;
                                break;
                            }
                            case PRESSURE:{
                                pressure[l] = aux_data;
                                data_parameter = OXYGEN;
                                break;
                            }
                            case OXYGEN:{
                                oxygen[l] = aux_data;
                                data_parameter = SALINITY;
                                break;
                            }
                            case SALINITY:{
                                salinity[l] = aux_data;
                                break;
                            }
                            default:
                                break;
                            }
                            sign = 1;
                            aux_data = 0.0;
                            aux_data_frac_part = 0.0;
                            aux_data_int_part = 0.0;
                        }
                    }
                    datalogger_clear_serial_port_buffer(instrument_id);
                }
            }
    }
    switch (instrument->processing) {
    case MEDIAN_t:{
        temperature_final = datalogger_median(&temperature[0], frames_received);
        conductivity_final = datalogger_median(&conductivity[0], frames_received);
        pressure_final = datalogger_median(&pressure[0], frames_received);
        oxygen_final = datalogger_median(&oxygen[0], frames_received);
        salinity_final = datalogger_median(&salinity[0], frames_received);
        break;
    }
    case AVERAGE_t:{
        temperature_final = datalogger_average(&temperature[0], frames_received);
        conductivity_final = datalogger_average(&conductivity[0], frames_received);
        pressure_final = datalogger_average(&pressure[0], frames_received);
        oxygen_final = datalogger_average(&oxygen[0], frames_received);
        salinity_final = datalogger_average(&salinity[0], frames_received);
        break;
    }
    default:{
        temperature_final = temperature[0];
        conductivity_final = conductivity[0];
        pressure_final = pressure[0];
        oxygen_final = oxygen[0];
        salinity_final = salinity[0];
        break;
    }
    }

    instrument_string  = "Instrument ";
    unsigned_to_decimal_string ((uint32_t)instrument_id, instrument_id_char, 1, 1);
    strcpy(string_data_result, instrument_string);
    strcat(string_data_result, instrument_id_char);
    strcat(string_data_result, ": ");
    memory_write_buffer_length = 14 + float_to_string(parameter_string, 10, temperature_final ,4);
    strcat(string_data_result, parameter_string);
    parameter_string = ", ";
    memory_write_buffer_length = memory_write_buffer_length + 2;
    strcat(string_data_result, parameter_string);
    parameter_string = "";
    memory_write_buffer_length = memory_write_buffer_length + float_to_string(parameter_string, 10, conductivity_final ,5);
    strcat(string_data_result, parameter_string);
    parameter_string = ", ";
    memory_write_buffer_length = memory_write_buffer_length + 2;
    strcat(string_data_result, parameter_string);
    parameter_string = "";
    memory_write_buffer_length = memory_write_buffer_length + float_to_string(parameter_string, 10, pressure_final ,3);
    strcat(string_data_result, parameter_string);
    parameter_string = ", ";
    memory_write_buffer_length = memory_write_buffer_length + 2;
    strcat(string_data_result, parameter_string);
    parameter_string = "";
    memory_write_buffer_length = memory_write_buffer_length + float_to_string(parameter_string, 10, oxygen_final ,3);
    strcat(string_data_result, parameter_string);
    parameter_string = ", ";
    memory_write_buffer_length = memory_write_buffer_length + 2;
    strcat(string_data_result, parameter_string);
    parameter_string = "";
    memory_write_buffer_length = memory_write_buffer_length + float_to_string(parameter_string, 10, salinity_final ,3);
    strcat(string_data_result, parameter_string);
    parameter_string = ", ";
    memory_write_buffer_length = memory_write_buffer_length + 2;
    strcat(string_data_result, parameter_string);
    parameter_string = "";
    datalogger_obtain_datetime_string(datalogger_datetime);
    strcat(string_data_result,datalogger_datetime);
    memory_write_buffer_length = memory_write_buffer_length+19;
    strcat(string_data_result, "\n");
    memory_write_buffer_length++;
    strcat(internal_buffer_realtime_out, string_data_result);
    length_buffer_realtime_out = length_buffer_realtime_out + memory_write_buffer_length;
    if (write_data_to_memory == '1'){
        wiced_rtos_lock_mutex(&memory_access_mutex); // Block the access to the memory
        datalogger_memory_write(string_data_result, memory_write_buffer_length);
        wiced_rtos_unlock_mutex(&memory_access_mutex); // Un block the access to the memory
    }}


