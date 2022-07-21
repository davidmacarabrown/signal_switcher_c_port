/* C Includes */
#include <stdio.h>
#include <memory.h>

/* Pico SDK Includes */
#include "pico/multicore.h"
#include "pico/util/queue.h"

/* Project Includes */
#include "core_1.h"
#include "gpio_defs.h"
#include "MCP23017.H"


MCP23017 *input_port;
MCP23017 *output_port;

queue_t* core_1_queue_rx;
queue_t* core_1_queue_tx;

QUEUE_ITEM_X queue_store;

uint8_t port_value = 0;

void port_interrupt_callback(uint32_t gpio, uint32_t events);

void core_1_main(void)
{
    i2c_init(i2c0, 400000);

#ifdef DEBUG
    printf("I2C0 Init Done\n");
#endif

    /* GPIO Configuration */
    gpio_set_function(I2C0_DATA, GPIO_FUNC_I2C);
    gpio_set_function(I2C0_CLOCK, GPIO_FUNC_I2C);
    gpio_pull_up(I2C0_DATA);
    gpio_pull_up(I2C0_CLOCK);
    gpio_init(PORTA_INTERRUPT);
    gpio_init(PORTB_INTERRUPT);
    gpio_set_dir(PORTA_INTERRUPT, GPIO_IN);
    gpio_set_dir(PORTB_INTERRUPT, GPIO_IN);

    gpio_set_irq_enabled_with_callback(PORTA_INTERRUPT, GPIO_IRQ_EDGE_FALL, true, (gpio_irq_callback_t)port_interrupt_callback);
    gpio_set_irq_enabled(PORTB_INTERRUPT, GPIO_IRQ_EDGE_FALL, true);

    /* Port Configurations */
    input_port = new MCP23017(i2c0, 0x21);

    input_port->set_port_mode(0);
    input_port->set_ioconfig(0b00101000);
    input_port->set_iodir_a(0xFF);
    input_port->set_iodir_b(0xFF);
    input_port->set_gppu_a (0xFF);
    input_port->set_gppu_b (0xFF);
    input_port->set_ipol_a (0xFF);
    input_port->set_ipol_b (0xFF);
    input_port->set_intcon_a(0xFF);
    input_port->set_intcon_b(0xFF);
    input_port->write_configuration();

    output_port = new MCP23017(i2c0, 0x20);

    output_port->set_port_mode(0);
    output_port->set_ioconfig(0b00111000);
    output_port->set_iodir_a(0x00);
    output_port->set_iodir_b(0x00);
    output_port->write_configuration();

    /* Tx/Rx Queue */
    core_1_queue_rx = (queue_t *)multicore_fifo_pop_blocking();
    core_1_queue_tx = (queue_t *)multicore_fifo_pop_blocking();

    while(1)
    {
        if(queue_try_remove(core_1_queue_rx, &queue_store))
        {
            // do something with the payload
        }
        else
        {
            memset(&queue_store, 0, sizeof(QUEUE_ITEM_X));
        }
    }
}

void port_interrupt_callback(uint32_t gpio, uint32_t events)
{
    QUEUE_ITEM_X command;
    
    command.instruction_code = PORT_INPUT;

#ifdef DEBUG
    printf("Detected Input\n");
#endif

    sleep_ms(50);

    switch(gpio)
    {
        case PORTA_INTERRUPT:
            port_value = input_port->read_input_mask(PORTA);
            command.data[0] = PORTA;
            break;
        case PORTB_INTERRUPT:
            port_value = input_port->read_input_mask(PORTB);
            command.data[0] = PORTB;
            break;
        default:
            break;
    }

    command.data[1] = port_value;
    queue_try_add(core_1_queue_tx, &port_value);
}