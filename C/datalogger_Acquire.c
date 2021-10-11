/*
 * datalogger_Aquire.c
 *
 *  Created on: 18/11/2019
 *      Author: José Raúl Santana Jiménez
 *
 *  License Creative Commons 3.0 (by-nc-nd)
 */
# include "datalogger_Acquire.h"
# include "datalogger_Sensors.h"
# include "wiced.h"

// Threads and semaphores for handling the instruments acquisition
static wiced_semaphore_t semaphoreHandle_Instrument[MAX_NUMBER_OF_INSTRUMENTS];
static wiced_thread_t ThreadHandle_Instrument[MAX_NUMBER_OF_INSTRUMENTS];
// Variables for instruments threads arguments
static int argument_instrument[MAX_NUMBER_OF_INSTRUMENTS];

static datalogger_instrument_t acquisition_datalogger_available_instruments[MAX_NUMBER_OF_INSTRUMENTS];

// Flag used to start and stop instruments acquisition
static int stop_instrument_aquiring;

void datalogger_instrument_acquire(int Instrument_Id){
    switch (acquisition_datalogger_available_instruments[Instrument_Id].instrument_type){
    case NONE_It:{
        break;
    }
    case SBE_37SM: {
        datalogger_SBE37_acquire(&acquisition_datalogger_available_instruments[Instrument_Id], Instrument_Id);
        break;
    }
    }
}

void acquisition_instrument(uint32_t arg) {
    wiced_rtos_delay_milliseconds(500);
    while(1) {

        if (stop_instrument_aquiring == 1) {
            wiced_rtos_get_semaphore(&(semaphoreHandle_Instrument[arg]), WICED_WAIT_FOREVER);
        }
        wiced_rtos_delay_milliseconds(acquisition_datalogger_available_instruments[arg].acquisition_interval);

        datalogger_instrument_acquire(arg);
        wiced_rtos_delay_milliseconds(5000);
        SBE_print_buffer(arg);
        wiced_rtos_delay_milliseconds(1000);
    }
}


void set_stop_instrument_acquiring(int value){
    stop_instrument_aquiring = value;
}

void acquisition_mode_init(){
/* Set up the instruments semaphore handlers and Initialize and start instruments threads */
    stop_instrument_aquiring = 1;
    for (int i = 0; i< MAX_NUMBER_OF_INSTRUMENTS; i++){
        argument_instrument[i]=i;
        wiced_rtos_init_semaphore(&(semaphoreHandle_Instrument[i]));
        wiced_rtos_create_thread(&(ThreadHandle_Instrument[i]), THREAD_PRIORITY_INSTRUMENT, NULL, acquisition_instrument, THREAD_STACK_SIZE_INSTRUMENT, argument_instrument[i]);
    }
}

// Start running all threads for acquiring enabled instruments data and store in memory
void acquisition_mode_start(datalogger_instrument_t* avaliable_instruments){
    for (int i = 0; i < MAX_NUMBER_OF_INSTRUMENTS; i++){
        acquisition_datalogger_available_instruments[i] = avaliable_instruments[i];
        if (avaliable_instruments[i].enabled == 1){
                wiced_rtos_set_semaphore(&(semaphoreHandle_Instrument[i]));
        }
    }
}

