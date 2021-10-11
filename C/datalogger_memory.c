/*
 * datalogger_memory.c
 *
 *  Created on: 21/11/2019
 *      Author: José Raúl Santana Jiménez
 *
 *  License Creative Commons 3.0 (by-nc-nd)
 */

#include "datalogger_memory.h"
#include <inttypes.h>

#define PACKET_SIZE (100u)


sflash_handle_t qspi_flash_handle;
sflash_write_allowed_t qspi_flash_allowed_write_in = SFLASH_WRITE_ALLOWED;
void* qspi_flash_peripheral_id=0;

unsigned long flash_size=0;
uint32_t extMemAddress = 0x1000000; // base address of the external memory
uint32_t lastMemAddress = 0x3FFFFFC;
uint32_t pointer_next_MemAddress = 0x1000000;

SFlash_memory_pointer_t memory_pointer_bytes_uint_32;



uint8_t txBuffer[PACKET_SIZE];
uint8_t rxBuffer[PACKET_SIZE];

// Check if the memory has the correct format
wiced_bool_t datalogger_memory_ready(){
    int ret=-1;
    uint8_t valid_address_indicator;
    ret = sflash_read(&qspi_flash_handle, lastMemAddress+4, &valid_address_indicator, 1);
    if (valid_address_indicator == (uint8_t)0xAA) {
        return WICED_TRUE;
    } else {
        return WICED_FALSE;
    }
}

// Initialize the memoru with the correct format
void datalogger_memory_init(void){
    int ret=-1;
    uint8_t init_valid_address_indicator[5];
    init_valid_address_indicator[0] = 0x00;
    init_valid_address_indicator[1] = 0x00;
    init_valid_address_indicator[2] = 0x00;
    init_valid_address_indicator[3] = 0x01;
    init_valid_address_indicator[3] = 0xAA;
    ret = -1;
    ret = sflash_sector_erase(&qspi_flash_handle, 16773119); // Erase the last sector on the SFLASH
    sflash_sector_erase(&qspi_flash_handle, extMemAddress); // Erase the first data sector on the SFLASH
    //Initialize last data address and the valid address indicator 16773119
    ret = sflash_write(&qspi_flash_handle, lastMemAddress, (uint8_t*)init_valid_address_indicator, 5);
    WPRINT_APP_INFO(("Erase Sector | status: %d\r\n", ret));
}

void datalogger_get_memory_size(void){
    int ret=-1;
    ret = init_sflash(&qspi_flash_handle, qspi_flash_peripheral_id, qspi_flash_allowed_write_in);
    sflash_get_size(&qspi_flash_handle, &flash_size);
    WPRINT_APP_INFO(("Memory Size: %lu | status: %d\r\n", flash_size, ret));
}

void datalogger_memory_write(uint8_t* buffer, int buffer_size){
    uint8_t aux_byte = 0xAA;
    int ret=-1;
    ret = -1;
    // Get the direction of the last byte written
    sflash_read(&qspi_flash_handle, lastMemAddress, &memory_pointer_bytes_uint_32.next_write_address_bytes[0], 4);
    ret = sflash_write(&qspi_flash_handle, memory_pointer_bytes_uint_32.next_write_address, buffer, buffer_size);

    memory_pointer_bytes_uint_32.next_write_address = memory_pointer_bytes_uint_32.next_write_address + buffer_size;
    sflash_sector_erase(&qspi_flash_handle, 16773119);
    sflash_write(&qspi_flash_handle, lastMemAddress, &memory_pointer_bytes_uint_32.next_write_address_bytes[0], 4);
    sflash_write(&qspi_flash_handle, lastMemAddress+3, &aux_byte, 1);
    ret = -1;
}

void datalogger_memory_read(uint8_t* buffer, int buffer_size, uint32_t address_to_start){
    int ret=-1;
    ret = -1;
    ret = sflash_read(&qspi_flash_handle, pointer_next_MemAddress + address_to_start, buffer, buffer_size);
}

uint32_t datalogger_get_data_bytes_saved(void){
    sflash_read(&qspi_flash_handle, lastMemAddress, &memory_pointer_bytes_uint_32.next_write_address_bytes[0], 4);
    return memory_pointer_bytes_uint_32.next_write_address - extMemAddress;
}
