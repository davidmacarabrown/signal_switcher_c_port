#ifndef CAT24C32_H
#define CAT24C32_H

/* C/C++ Includes */

/* Pico SDK Includes */
#include "pico/stdlib.h"
#include "hardware/i2c.h"

#define CAT24C32_PAGE_SIZE   32
#define CAT24C32_PAGE_COUNT  128
#define CAT24C32_TOTAL_BYTES 4096
#define I2C_TIMING 5

class CAT24C32
{
    private:
        i2c_inst_t *i2c_instance;
        uint8_t i2c_address;

        /* Buffer for constructing write commands, 2 Address bytes + 32 Data bytes */
        uint8_t command_buffer[CAT24C32_PAGE_SIZE + 2];
        
    public:
        CAT24C32(){};
        CAT24C32(i2c_inst_t *i2c_instance, uint8_t i2c_address); //TODO: Page size/page count/total bytes in constructor?

        void write_multiple_bytes(uint8_t *source, uint16_t byte_address, uint16_t num_bytes);
        void read_multiple_bytes(uint16_t byte_address, uint16_t num_bytes, uint8_t *destination);

        int write_byte(uint8_t byte, uint16_t byte_address);
        uint8_t read_byte(uint16_t byte_address);

        void erase(void);

        void test(void);
};

#endif

//TODO: For later... page counter to increment pages?