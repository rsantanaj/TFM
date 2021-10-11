/*
 * datalogger_Serial_Port.h
 *
 *  Created on: 19/11/2019
 *      Author: José Raúl Santana Jiménez
 *
 *  License Creative Commons 3.0 (by-nc-nd)
 */

#ifndef APPS_DATALOGGER_DATALOGGER_SERIAL_PORT_H_
#define APPS_DATALOGGER_DATALOGGER_SERIAL_PORT_H_

#include "wiced.h"
#include "platform_peripheral.h"
#include "platform_init.h"

// Definitions for datalogger serial port management
typedef enum{
    SERIAL1,
    SERIAL2,
    SERIAL3,
    SERIAL4,
    SERIAL5,
    SERIAL6,
    SERIAL7,
    SERIAL8
}datalogger_asinchronous_serial_ports_t;

typedef struct {
    datalogger_asinchronous_serial_ports_t port;
    wiced_gpio_t Tx;
    wiced_gpio_t Rx;
}datalogger_asinchronous_serial_port_t;

int get_serialport_buffer_length(int instrument_id);

wiced_bool_t serial_new_byte_received(uint32_t arg);

void print_buffer(uint32_t arg);

void serial_start_isr(uint32_t arg);

void datalogger_receive(uint32_t arg);

void datalogger_send(wiced_bool_t byte[], int serial_port_id);

void datalogger_char_to_byte(char ch, wiced_bool_t byte[]);

void serial_send_string(char* string_to_send, int number_of_chars, int serial_port_id, int baud_rate);

void datalogger_serial_ports_init(void);

#endif /* APPS_DATALOGGER_DATALOGGER_SERIAL_PORT_H_ */
