/*
 * datalogger_general.c
 *
 *  Created on: 12/11/2019
 *      Author: José Raúl Santana Jiménez
 *
 *  License Creative Commons 3.0 (by-nc-nd)
 */
#include "datalogger_general.h"

void datalogger_init(void){
    //Initialize the datalogger_available_instruments array
    for (int i = 0; i<MAX_NUMBER_OF_INSTRUMENTS; i++){
        datalogger_available_instruments[i].enabled = WICED_FALSE;
        datalogger_available_instruments[i].instrument_type = NONE_It;
    }
}



