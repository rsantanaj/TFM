/*
 * datalogger_memory.h
 *
 *  Created on: 21/11/2019
 *      Author: José Raúl Satana Jiménez
 *
 *  License Creative Commons 3.0 (by-nc-nd)
 */

#ifndef APPS_DATALOGGER_DATALOGGER_MEMORY_H_
#define APPS_DATALOGGER_DATALOGGER_MEMORY_H_

#include "wiced.h"
#include "spi_flash.h"
#include "stdio.h"
#include "string.h"

typedef union
{
uint32 next_write_address;
uint8 next_write_address_bytes[4];
}SFlash_memory_pointer_t;

uint32_t datalogger_get_data_bytes_saved(void);

void datalogger_memory_init(void);

wiced_bool_t datalogger_memory_ready();

void datalogger_get_memory_size(void);

void datalogger_memory_write(uint8_t* buffer, int buffer_size);

void datalogger_memory_read(uint8_t* buffer, int buffer_size, uint32_t address_to_start);


#endif /* APPS_DATALOGGER_DATALOGGER_MEMORY_H_ */
