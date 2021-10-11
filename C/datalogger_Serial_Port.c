/*
 * datalogger_Serial_Port.c
 *
 *  Created on: 19/11/2019
 *      Author: José Raúl Santana Jiménez
 *
 *  License Creative Commons 3.0 (by-nc-nd)
 */

#include "datalogger_general.h"
#include "datalogger_Serial_port.h"

#define THREAD_PRIORITY_SERIAL_PORT            (5)
#define THREAD_STACK_SIZE_SERIAL_PORT        (1024)
#define MAX_SERIAL_INPUT_BUFFER_LENGTH       (1024)

static datalogger_asinchronous_serial_port_t available_instruments_serial_ports[MAX_NUMBER_OF_INSTRUMENTS];

// Threads and semaphores for handling the serial ports
static wiced_semaphore_t semaphoreHandle_Serial_Port[MAX_NUMBER_OF_INSTRUMENTS];
static wiced_thread_t ThreadHandle_Serial_Port[MAX_NUMBER_OF_INSTRUMENTS];
wiced_bool_t serial_buffer_input[MAX_NUMBER_OF_INSTRUMENTS][MAX_SERIAL_INPUT_BUFFER_LENGTH]={WICED_TRUE};
int serial_buffer_input_pointer[MAX_NUMBER_OF_INSTRUMENTS]={0};
wiced_bool_t serial_port_new_byte_received[MAX_NUMBER_OF_INSTRUMENTS];


wiced_bool_t serial_new_byte_received(uint32_t arg){
    return serial_port_new_byte_received[arg];
}

uint8 serial_port_buffer_get_byte(int port_id, int index){
    int bit_LSB = (index * 8) + 7;
    int bit_MSB = (index * 8);
    int actual_bit;
    uint8 aux = 0;
    int base = 1;
    for(int i = bit_LSB; i<= bit_MSB; i--){
        actual_bit = serial_buffer_input[port_id][i];
        aux = aux + (actual_bit*base);
        base = base *2;
    }
    return aux;
}

int get_serialport_buffer_length(int instrument_id){
    return serial_buffer_input_pointer[instrument_id];
}

void datalogger_clear_serial_port_buffer(instrument_id){
    serial_buffer_input_pointer[instrument_id] = 0;
}

void serial_start_isr(uint32_t arg)
{
    wiced_rtos_set_semaphore(&semaphoreHandle_Serial_Port[arg]); /* Set the semaphore */
}

void print_buffer(uint32_t arg){
    int j = 0;
    for (int i = 0; i < serial_buffer_input_pointer[arg]; i++){
        if (j == 7) {
            j = 0;
            printf("\n");
        } else {
            j++;
        }
    }
    serial_buffer_input_pointer[arg] = 0;
}


void datalogger_receive(uint32_t arg)
{
    wiced_bool_t byte[8];
    wiced_bool_t estado = WICED_FALSE;
    int index = (int)arg;
    while(1)
    {
        wiced_rtos_get_semaphore(&semaphoreHandle_Serial_Port[index], WICED_WAIT_FOREVER); // whait for the start bit
        wiced_gpio_input_irq_disable(available_instruments_serial_ports[index].Rx); // Unable the IRS
        for (int i = 0; i <= 7; i++){
            if (estado == WICED_FALSE) {
                estado = WICED_TRUE;
            } else {
                estado = WICED_FALSE;
            }
            byte[i] =  wiced_gpio_input_get(available_instruments_serial_ports[index].Rx);
            serial_buffer_input[arg][serial_buffer_input_pointer[arg]] = byte[i];
            serial_buffer_input_pointer[arg] =serial_buffer_input_pointer[arg]+1;
            serial_port_new_byte_received[index]=WICED_TRUE;
            wiced_rtos_delay_microseconds( 97 );
        }
        wiced_gpio_input_irq_enable(available_instruments_serial_ports[arg].Rx, IRQ_TRIGGER_FALLING_EDGE, serial_start_isr, arg);
    }
}

void datalogger_send(wiced_bool_t byte[], int serial_port_id) {
    int aux = 1;
    wiced_gpio_output_low(available_instruments_serial_ports[serial_port_id].Tx); // start bit
    wiced_rtos_delay_microseconds( 100 );
    for (int i = 0; i <= 7; i++) {
        if (byte[i] == WICED_FALSE) {
             wiced_gpio_output_low(available_instruments_serial_ports[serial_port_id].Tx);
        } else {
            wiced_gpio_output_high(available_instruments_serial_ports[serial_port_id].Tx);
        }
        wiced_rtos_delay_microseconds( 100 );
    }
    wiced_gpio_output_high(available_instruments_serial_ports[serial_port_id].Tx); // bit de stop
    wiced_rtos_delay_microseconds( 100 );
}


void datalogger_char_to_byte(char ch, wiced_bool_t byte[]) {
    int integer_ch;
    int j = 0;
    integer_ch = (unsigned int)ch;
    while(integer_ch >= 2) {
        byte[j] = integer_ch % 2;
        integer_ch = integer_ch/2;
        j=j+1;
    }
    byte[j] = integer_ch;
    j = j+1;
    while (j <= 7){
        byte[j] = 0;
        j = j+1;
    }
}

void serial_send_string(char* string_to_send, int number_of_chars, int serial_port_id, int baud_rate){
    wiced_bool_t byte[8];
    for (int i = 0; i< number_of_chars; i++){
        datalogger_char_to_byte(string_to_send[i], byte);
        datalogger_send(byte, serial_port_id);
    }
}

void datalogger_serial_ports_init(void){
    available_instruments_serial_ports[0].Tx = WICED_GPIO_61;
    available_instruments_serial_ports[1].Tx = WICED_GPIO_63;
    available_instruments_serial_ports[2].Tx = WICED_GPIO_66;
    available_instruments_serial_ports[3].Tx = WICED_GPIO_69;
    available_instruments_serial_ports[4].Tx = WICED_GPIO_71;
    available_instruments_serial_ports[5].Tx = WICED_GPIO_98;
    available_instruments_serial_ports[6].Tx = WICED_GPIO_93;
    available_instruments_serial_ports[7].Tx = WICED_GPIO_39;
    available_instruments_serial_ports[0].Rx = WICED_GPIO_62;
    available_instruments_serial_ports[1].Rx = WICED_GPIO_65;
    available_instruments_serial_ports[2].Rx = WICED_GPIO_67;
    available_instruments_serial_ports[3].Rx = WICED_GPIO_70;
    available_instruments_serial_ports[4].Rx = WICED_GPIO_72;
    available_instruments_serial_ports[5].Rx = WICED_GPIO_99;
    available_instruments_serial_ports[6].Rx = WICED_GPIO_94;
    available_instruments_serial_ports[7].Rx = WICED_GPIO_40;

    for (int i = 0; i< MAX_NUMBER_OF_INSTRUMENTS; i++){
        available_instruments_serial_ports[i].port = i;
        wiced_gpio_init(available_instruments_serial_ports[i].Tx, OUTPUT_PUSH_PULL);
        wiced_gpio_init(available_instruments_serial_ports[i].Rx, INPUT_HIGH_IMPEDANCE);
        serial_buffer_input_pointer[i]=0;
        serial_buffer_input[i][0]=WICED_FALSE;

        wiced_gpio_output_high(available_instruments_serial_ports[i].Tx);

        wiced_rtos_init_semaphore(&(semaphoreHandle_Serial_Port[i]));
        wiced_rtos_create_thread(&(ThreadHandle_Serial_Port[i]), THREAD_PRIORITY_SERIAL_PORT, NULL, datalogger_receive, THREAD_STACK_SIZE_SERIAL_PORT, i);

        wiced_gpio_input_irq_enable(available_instruments_serial_ports[i].Rx, IRQ_TRIGGER_FALLING_EDGE, serial_start_isr, i);

    }
}


