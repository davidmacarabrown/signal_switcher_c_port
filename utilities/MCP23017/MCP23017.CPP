/* C/C++ Includes */
#include <stdio.h>

/* Pico SDK Includes */
#include "MCP23017.H"

MCP23017::MCP23017(){}
MCP23017::MCP23017(i2c_inst_t *i2c_instance,  uint8_t i2c_address)
{
    this->i2c_instance = i2c_instance;       
    this->i2c_address  = i2c_address;
}


/* Writes the current configuration values to the MCP23017 registers */
void MCP23017::write_configuration(void)
{
    uint8_t buffer[2];

    //IOCON
    buffer[0] = register_address_lookup[port_config.configured_mode][IOCONA];
    buffer[1] = port_config.io_config;
    i2c_write_blocking(i2c_instance, i2c_address, buffer, sizeof(buffer), false);

    port_config.configured_mode = port_config.port_mode;

    //IODIRA
    buffer[0] = register_address_lookup[port_config.port_mode][IODIRA];
    buffer[1] = port_config.iodir_a;
    i2c_write_blocking(i2c_instance, i2c_address, buffer, sizeof(buffer), false);

    //IODIRB
    buffer[0] = register_address_lookup[port_config.port_mode][IODIRB];
    buffer[1] = port_config.iodir_b;
    i2c_write_blocking(i2c_instance, i2c_address, buffer, sizeof(buffer), false);

    //IPOLA
    buffer[0] = register_address_lookup[port_config.port_mode][IPOLA];
    buffer[1] = port_config.ipol_a;
    i2c_write_blocking(i2c_instance, i2c_address, buffer, sizeof(buffer), false);

    //IPOLB
    buffer[0] = register_address_lookup[port_config.port_mode][IPOLB];
    buffer[1] = port_config.ipol_b;
    i2c_write_blocking(i2c_instance, i2c_address, buffer, sizeof(buffer), false);

    //GPINTENA
    buffer[0] = register_address_lookup[port_config.port_mode][GPINTENA];
    buffer[1] = port_config.gpint_a;
    i2c_write_blocking(i2c_instance, i2c_address, buffer, sizeof(buffer), false);
    
    //GPINTENB
    buffer[0] = register_address_lookup[port_config.port_mode][GPINTENB];
    buffer[1] = port_config.gpint_b;
    i2c_write_blocking(i2c_instance, i2c_address, buffer, sizeof(buffer), false);

    //DEFVALA
    buffer[0] = register_address_lookup[port_config.port_mode][DEFVALA];
    buffer[1] = port_config.defval_a;
    i2c_write_blocking(i2c_instance, i2c_address, buffer, sizeof(buffer), false);

    //DEFVALB
    buffer[0] = register_address_lookup[port_config.port_mode][DEFVALB];
    buffer[1] = port_config.defval_b;
    i2c_write_blocking(i2c_instance, i2c_address, buffer, sizeof(buffer), false);

    //INTCONA
    buffer[0] = register_address_lookup[port_config.port_mode][INTCONA];
    buffer[1] = port_config.intcon_a;
    i2c_write_blocking(i2c_instance, i2c_address, buffer, sizeof(buffer), false);

    //INTCONB
    buffer[0] = register_address_lookup[port_config.port_mode][INTCONB];
    buffer[1] = port_config.intcon_b;
    i2c_write_blocking(i2c_instance, i2c_address, buffer, sizeof(buffer), false);

    //GPPUA
    buffer[0] = register_address_lookup[port_config.port_mode][GPPUA];
    buffer[1] = port_config.gppu_a;
    i2c_write_blocking(i2c_instance, i2c_address, buffer, sizeof(buffer), false);

    //GPPUB
    buffer[0] = register_address_lookup[port_config.port_mode][GPPUB];
    buffer[1] = port_config.gppu_b;
    i2c_write_blocking(i2c_instance, i2c_address, buffer, sizeof(buffer), false);

    //INTFA
    buffer[0] = register_address_lookup[port_config.port_mode][INTFA];
    buffer[1] = port_config.intf_a;
    i2c_write_blocking(i2c_instance, i2c_address, buffer, sizeof(buffer), false);

    //INTFB
    buffer[0] = register_address_lookup[port_config.port_mode][INTFB];
    buffer[1] = port_config.intf_b;
    i2c_write_blocking(i2c_instance, i2c_address, buffer, sizeof(buffer), false);
}


void MCP23017::test_output(void)
{
    printf("Starting Output Test\n");

    write_mask(0, 0b11110000);
    write_mask(1, 0b00001111);

    sleep_ms(1000);

    write_mask(0, 0);
    write_mask(1, 0);    
}

void MCP23017::test_input(void)
{
    printf("Starting Input Test\n");

    while(1)
    {
        printf("Port A: %02x\n", read_input_mask(0));
        printf("Port B: %02x\n", read_input_mask(1));
        sleep_ms(1000);
    }
}

void MCP23017::write_mask(uint8_t port, uint8_t mask)
{
    uint8_t buffer[2];

    if(port == 0)
    {
        buffer[0] = register_address_lookup[port_config.port_mode][GPIOA];
    }
    else
    {
        buffer[0] = register_address_lookup[port_config.port_mode][GPIOB];
    }    

    buffer[1] = mask;
    i2c_write_blocking(i2c_instance, i2c_address, buffer, 2, false);
}

uint8_t MCP23017::read_input_mask(uint8_t port)
{
    uint8_t address;
    uint8_t data;

    switch(port)
    {
        case 0:
            address = register_address_lookup[port_config.port_mode][GPIOA];
            break;
        case 1:
            address = register_address_lookup[port_config.port_mode][GPIOB];
            break;
        default:
            break;
    }

    set_address_pointer(address);

    i2c_read_blocking(i2c_instance, i2c_address, &data, 1 , false);
    return data;
}

void MCP23017::set_address_pointer(uint8_t register_location)
{
    uint8_t command = register_address_lookup[port_config.port_mode][register_location];

    i2c_write_blocking(i2c_instance, i2c_address, &command, 1 , false);
}

void MCP23017::write_pin(uint8_t port, uint16_t pin, uint8_t state)
{
     if(state)
    {
        port_state[port] |= (0x01 << pin);
    }
    else
    {
        port_state[port] ^= (0x00 << pin);
    }

    write_mask(port, port_state[port]);
}

void MCP23017::set_port_mode(uint8_t port_mode)
{
    this->port_config.port_mode = port_mode;
}

void MCP23017::set_ioconfig(uint8_t io_config)
{
    this->port_config.io_config = io_config;
}

void MCP23017::set_iodir_a(uint8_t iodir_a)
{
    this->port_config.iodir_a = iodir_a;
}

void MCP23017::set_iodir_b(uint8_t iodir_b)
{
    this->port_config.iodir_b = iodir_b;
}

void MCP23017::set_ipol_a(uint8_t ipol_a)
{
    this->port_config.ipol_a = ipol_a;
}

void MCP23017::set_ipol_b(uint8_t ipol_b)
{
    this->port_config.ipol_b = ipol_b;
}

void MCP23017::set_gpint_a(uint8_t gpint_a)
{
    this->port_config.gpint_a = gpint_a;
}

void MCP23017::set_gpint_b(uint8_t gpint_b)
{
    this->port_config.gpint_b = gpint_b;
}

void MCP23017::set_defval_a(uint8_t defval_a)
{
    this->port_config.defval_a = defval_a;
}

void MCP23017::set_defval_b(uint8_t defval_b)
{
    this->port_config.defval_b = defval_b;
}

void MCP23017::set_intcon_a(uint8_t intcon_a)
{
    this->port_config.intcon_a = intcon_a;
}

void MCP23017::set_intcon_b(uint8_t intcon_b)
{
    this->port_config.intcon_b = intcon_b;
}

void MCP23017::set_gppu_a(uint8_t gppu_a)
{
    this->port_config.gppu_a = gppu_a;
}

void MCP23017::set_gppu_b(uint8_t gppu_b)
{
    this->port_config.gppu_b  = gppu_b;
}

void MCP23017::set_intf_a(uint8_t intf_a)
{
    this->port_config.intf_a = intf_a;
}

void MCP23017::set_intf_b(uint8_t intf_b)
{
    this->port_config.intf_b = intf_b;
}