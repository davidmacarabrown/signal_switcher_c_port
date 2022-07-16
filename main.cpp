/* C Includes */
#include <stdlib.h>
#include <stdio.h>
#include <memory.h>

/* Pico SDK Includes */
#include "pico/stdlib.h"
#include "hardware/timer.h"
#include "hardware/sync.h"
#include "hardware/i2c.h"

/* Project Includes */
#include "gpio_defs.h"
#include "output_manager.h"
#include "state_manager.h"
#include "instruction_handler.h"
#include "display_manager.h"
#include "storage_manager.h"
#include "CAT24C32.h"
#include "MCP23017.H"

#include "debug.h"

void input_detect_callback(uint gpio, uint32_t events);
void startup_routine(void);

//void check_for_command_input(repeating_timer *t, bool *command_flag, bool *interrupt_in_progress);
uint8_t command_id;
bool command_flag = false;
bool irq_flag = false;
repeating_timer_t timer;

MCP23017 *output_port;
MCP23017 *input_port;


InstructionHandler *instruction_handler;
StateManager *state_mgr;
OutputManager *output_mgr;
DisplayManager *display_mgr;
StorageManager *storage_mgr;

int main()
{   
    uint8_t result = 1;
    uint8_t mode;

    stdio_init_all();
    //sleep_ms(5000);
    printf("Hello Mate\n");

    i2c_init(i2c0, 400000);
    printf("I2C0 Init Done\n");
    fflush(stdout);
    i2c_init(i2c1, 400000);
    printf("I2C1 Init Done\n");fflush(stdout);
    gpio_set_function(I2C0_DATA, GPIO_FUNC_I2C);
    gpio_set_function(I2C0_CLOCK, GPIO_FUNC_I2C);
    gpio_pull_up(I2C0_DATA);
    gpio_pull_up(I2C0_CLOCK);
    
    gpio_set_function(I2C1_DATA, GPIO_FUNC_I2C);
    gpio_set_function(I2C1_CLOCK, GPIO_FUNC_I2C);
    gpio_pull_up(I2C1_DATA);
    gpio_pull_up(I2C1_CLOCK);
    printf("Pin i2c set\n");fflush(stdout);

    for(uint8_t i = SW_ONE; i<=SW_WRITE; i++)
    {
        gpio_init(i);
        gpio_set_dir(i, GPIO_IN);
        gpio_pull_down(i);
    }

    output_port = new MCP23017(i2c0, 0x20);
    input_port = new MCP23017(i2c0, 0x21);

    instruction_handler = new InstructionHandler;
    output_mgr = new OutputManager;
    display_mgr = new DisplayManager(i2c1, QUAD_ADDR);
    storage_mgr = new StorageManager(i2c1, EEPROM_ADDR);
    state_mgr = new StateManager;

    

    printf("Created object\n");fflush(stdout);
    output_port->test();
    printf("Test Done\n");fflush(stdout);

    // InstructionHandler instruction_handler;
    // OutputManager output_mgr;
    // DisplayManager display_mgr(i2c1, QUAD_ADDR);
    // StorageManager storage_mgr(i2c1, EEPROM_ADDR);
    // StateManager state_mgr; 

    display_mgr->clear();
    sleep_ms(200);
    display_mgr->test();
    
    gpio_set_irq_enabled_with_callback
    (
        SW_ONE,
        GPIO_IRQ_EDGE_RISE,
        1,
        &input_detect_callback
    );    

    for(uint8_t i = SW_TWO; i <= SW_WRITE; i++)
    {
        gpio_set_irq_enabled(i, GPIO_IRQ_EDGE_RISE, 1);
    }

    //storage_mgr.test();
    


    instruction_handler->initialise(state_mgr, 
                                    output_mgr, 
                                    display_mgr, 
                                    storage_mgr,
                                    &command_flag, 
                                    &irq_flag);
                                         
    output_mgr->initialise(state_mgr);
    state_mgr->initialise(storage_mgr);
    storage_mgr->initialise(state_mgr);
    display_mgr->initialise(state_mgr);

    instruction_handler->startup_routine();

    //startup_routine();
    printf("<main> Mode: %d\n", state_mgr->get_mode());    

    //add_repeating_timer_ms(16, check_for_input, NULL, &timer);
    //gpio_put(PICO_LED, HIGH);

    while(true)
    {
        //printf("running\n"); fflush(stdout);
        //instruction handler decode input
        
        /* 
        TODO: Call InstructionHandler::update()
        Implement state to hold the command, and whether or not it has been processed.
        Once processed and executed set this back to false.
        */
        
        //printf("MASK: %d\n", gpio_get_all());
        instruction_handler->check_for_command_input();
        sleep_ms(16);
    }
}

void input_detect_callback(uint gpio, uint32_t events)
{
    if(command_flag == false && irq_flag == false)
    {
        command_flag = true;
        irq_flag = true;
    }
}
