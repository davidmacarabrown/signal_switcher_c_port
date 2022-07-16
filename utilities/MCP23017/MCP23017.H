#ifndef MCP2301_H
#define MCP2301_H

/* C/C++ Includes */

/* Pico SDK Includes */
#include "pico/stdlib.h"
#include "hardware/i2c.h"

typedef enum port
{
    PORTA,
    PORTB
} PORT;

typedef enum device_mode
{
    MODE16BIT,
    MODE8BIT
} PORT_MODE;

/* Struct to hold values to apply to each register initialised to defaults so changes can be applied on a differential basis */
typedef struct mcp23017_config_x
{
    uint8_t port_mode = 0;
    uint8_t configured_mode = 0;
    uint8_t io_config   = 0x00;
    uint8_t iodir_a     = 0xFF;
    uint8_t iodir_b     = 0xFF;
    uint8_t ipol_a      = 0x00;
    uint8_t ipol_b      = 0x00;
    uint8_t gpint_a     = 0x00;
    uint8_t gpint_b     = 0x00;
    uint8_t defval_a    = 0x00;
    uint8_t defval_b    = 0x00;
    uint8_t intcon_a    = 0x00;
    uint8_t intcon_b    = 0x00;
    uint8_t gppu_a      = 0x00;
    uint8_t gppu_b      = 0x00;
    uint8_t intf_a      = 0x00;
    uint8_t intf_b      = 0x00;
} MCP23017_CONFIG_X;

class MCP23017
{
    private:
        i2c_inst_t *i2c_instance;
        uint8_t i2c_address;
        
        MCP23017_CONFIG_X port_config;

        uint8_t port_a_state;
        uint8_t port_b_state;

    public:
        MCP23017();
        MCP23017(i2c_inst_t *i2c_instance,  uint8_t i2c_address);
        void write_configuration(void);
        void write_to_port(uint8_t port, uint8_t mask);
        void test();

        void set_port_mode(uint8_t port_mode);
        void set_ioconfig(uint8_t io_config);
        void set_iodir_a(uint8_t iodir_a);
        void set_iodir_b(uint8_t iodir_b);
        void set_ipol_a(uint8_t ipol_a);
        void set_ipol_b(uint8_t ipol_b);
        void set_gpint_a(uint8_t gpint_a);
        void set_gpint_b(uint8_t gpint_b);
        void set_defval_a(uint8_t defval_a);
        void set_defval_b(uint8_t defval_b);
        void set_intcon_a(uint8_t intcon_a);
        void set_intcon_b(uint8_t intcon_b);
        void set_gppu_a(uint8_t gppu_a);
        void set_gppu_b(uint8_t gppu_b);
        void set_intf_a(uint8_t intf_a);
        void set_intf_b(uint8_t intf_b);
};



typedef enum register_position
{
    IODIRA,
    IODIRB,
    IPOLA,
    IPOLB,
    GPINTENA,
    GPINTENB,
    DEFVALA,
    DEFVALB,
    INTCONA,
    INTCONB,
    IOCONA,
    IOCONB,
    GPPUA,
    GPPUB,
    INTFA,
    INTFB,
    INTCAPA,
    INTCABB,
    GPIOA,
    GPIOB,
    OLATA,
    OLATB
} REGISTER_POSITION;


const uint8_t register_address_lookup[2][22]
{
    {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15}, // 16Bit
    {0x00, 0x10, 0x01, 0x11, 0x02, 0x12, 0x03, 0x13, 0x04, 0x14, 0x05, 0x15, 0x06, 0x16, 0x07, 0x17, 0x08, 0x18, 0x09, 0x19, 0x0A, 0x1A}  // 8Bit
};

#endif
