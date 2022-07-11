#include "CAT24C32.h"
#include <memory.h>
#include <stdio.h>
#include "gpio_defs.h"
#include "debug.h"
#include <string>


CAT24C32::CAT24C32(i2c_inst_t *i2c_instance, uint8_t i2c_address)
{
    this->i2c_instance = i2c_instance;       
    this->i2c_address  = i2c_address;
}


void CAT24C32::write_multiple_bytes(uint8_t *source, uint16_t byte_address, uint16_t num_bytes)
{
    for(int i = 0; i < num_bytes; i++)
    {
        /* Prepare address bytes */
        command_buffer[0] = (uint8_t) (byte_address >> 8);
        command_buffer[1] = (uint8_t) byte_address;

        /* Copy the data to be written into the buffer at the correct position */
        memcpy(&command_buffer[2], &source[i], 1);

        i2c_write_blocking(i2c_instance, i2c_address, command_buffer, 3, false);
        sleep_ms(I2C_TIMING);
        byte_address++;
    }
}

/* Uncapped read function, must pass a pointer to an array 
big enough to hold the requested number of bytes */
void CAT24C32::read_multiple_bytes(uint16_t byte_address, uint16_t num_bytes, uint8_t *destination)
{
    command_buffer[0] = (uint8_t) (byte_address >> 8);
    command_buffer[1] = (uint8_t) byte_address;

    i2c_write_blocking(i2c_instance, i2c_address, command_buffer, 2, false);

    sleep_ms(I2C_TIMING);

    i2c_read_blocking(i2c_instance, i2c_address, destination, num_bytes, false);

    sleep_ms(I2C_TIMING);
}

/* Write a given single byte to the given address */
int CAT24C32::write_byte(uint8_t byte, uint16_t byte_address)
{
    /* Prepare address bytes */
    command_buffer[0] = (uint8_t) (byte_address >> 8);
    command_buffer[1] = (uint8_t) byte_address;

    command_buffer[2] = byte;

    i2c_write_blocking(i2c_instance, i2c_address, command_buffer, 3, false);
    sleep_ms(I2C_TIMING);

    /* Readback and compare */
    command_buffer[3] = read_byte(byte_address);

    return memcmp(&command_buffer[2], &command_buffer[3], 1);
}

/* Reads a single byte at the given address and returns it */
uint8_t CAT24C32::read_byte(uint16_t byte_address)
{
    uint8_t read_byte;

    /* Prepare address bytes */
    command_buffer[0] = (uint8_t) (byte_address >> 8);
    command_buffer[1] = (uint8_t) byte_address;

    /* Set the EEPROM internal address register by writing the 2 address bytes */
    i2c_write_blocking(i2c_instance, i2c_address, command_buffer, 2, false);
    sleep_ms(I2C_TIMING);

    i2c_read_blocking(i2c_instance, i2c_address, &read_byte, 1, false);

    return read_byte;
}

void CAT24C32::erase(void)
{
    uint16_t address = 0;
    uint8_t erase_buffer[CAT24C32_PAGE_SIZE];

    memset(erase_buffer, 0, CAT24C32_PAGE_SIZE);

    for(int i = 0; i < CAT24C32_PAGE_COUNT; i++)
    {
        write_multiple_bytes(erase_buffer, address, CAT24C32_PAGE_SIZE);
        address+= CAT24C32_PAGE_SIZE;
    }
}