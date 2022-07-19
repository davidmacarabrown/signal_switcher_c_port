#include "core_1.h"

MCP23017 *input_port;
MCP23017 *output_port;

queue_t* intra_core_queue_rx;
queue_t* intra_core_queue_tx;

bool input_latch;

void core_1_main(void)
{
    uint8_t mask_a = 0;
    uint8_t mask_b = 0;
    uint8_t payload = 0;

        i2c_init(i2c0, 400000);
#ifdef DEBUG
    printf("I2C0 Init Done\n");
#endif

    gpio_set_function(I2C0_DATA, GPIO_FUNC_I2C);
    gpio_set_function(I2C0_CLOCK, GPIO_FUNC_I2C);
    gpio_pull_up(I2C0_DATA);
    gpio_pull_up(I2C0_CLOCK);

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
    input_port->write_configuration();

    output_port = new MCP23017(i2c0, 0x20);

    output_port->set_port_mode(1);
    output_port->set_ioconfig(0b10110010);
    output_port->set_iodir_a(0x00);
    output_port->set_iodir_b(0x00);
    output_port->write_configuration();

    /* Tx/Rx Queue */
    intra_core_queue_rx = (queue_t *)multicore_fifo_pop_blocking();
    intra_core_queue_tx = (queue_t *)multicore_fifo_pop_blocking();

    while(1)
    {
        mask_a = input_port->read_input_mask(PORTA);
        mask_b = input_port->read_input_mask(PORTB);

        if(mask_a || mask_b)
        {
            printf("Detected Input\n");
            sleep_ms(50);
            mask_a = input_port->read_input_mask(PORTA);
            mask_b = input_port->read_input_mask(PORTB);

            if((mask_a || mask_b) && (input_latch == false))
            {
                if(mask_a)
                {
                    queue_try_add(intra_core_queue_tx, &mask_a);
                    printf("Core 1: %02x\n", mask_a);
                }
                else
                {
                    queue_try_add(intra_core_queue_tx, &mask_b);
                }
                
                input_latch = true;
            }
        }

        if((!mask_a) && (!mask_b))
        {
            input_latch = false;
        }


        if(queue_try_remove(intra_core_queue_rx, &payload))
        {
            printf("Payload: %02X", payload);
        }
        else
        {
            payload = 0;
        }

        sleep_ms(100);

    }
}