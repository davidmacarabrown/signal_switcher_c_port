/* C Includes */
#include <stdlib.h>
#include <stdio.h>
#include <memory.h>

/* Pico SDK Includes */
#include "pico/stdlib.h"
#include "hardware/timer.h"
#include "hardware/sync.h"
#include "hardware/i2c.h"
#include "pico/multicore.h"
#include "pico/util/queue.h"

/* Project Includes */
#include "core_1.h"
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

InstructionHandler *instruction_handler;
StateManager *state_mgr;
OutputManager *output_mgr;
DisplayManager *display_mgr;
StorageManager *storage_mgr;

queue_t *intra_core_queue_tx = new queue_t;
queue_t *intra_core_queue_rx = new queue_t;

int main()
{   
    stdio_init_all();
    sleep_ms(5000);

    queue_init_with_spinlock(intra_core_queue_tx, 1, 5, 1);
    queue_init_with_spinlock(intra_core_queue_rx, 1, 5, 1);

    multicore_launch_core1(core_1_main);

    i2c_init(i2c1, 400000);
#ifdef DEBUG
    printf("I2C1 Init Done\n");fflush(stdout);
    fflush(stdout);
#endif
    
    gpio_set_function(I2C1_DATA, GPIO_FUNC_I2C);
    gpio_set_function(I2C1_CLOCK, GPIO_FUNC_I2C);
    gpio_pull_up(I2C1_DATA);
    gpio_pull_up(I2C1_CLOCK);

#ifdef DEBUG
    printf("i2c Pins set\n");fflush(stdout);
#endif

    instruction_handler = new InstructionHandler;
    output_mgr = new OutputManager;
    display_mgr = new DisplayManager(i2c1, QUAD_ADDR);
    storage_mgr = new StorageManager(i2c1, EEPROM_ADDR);
    state_mgr = new StateManager;
    printf("Objects Created\n");

    printf("Passing Queue pointer to core1\n");
    multicore_fifo_push_blocking((uint32_t)intra_core_queue_tx);
    multicore_fifo_push_blocking((uint32_t)intra_core_queue_rx);
    printf("Done\n");

    uint8_t send = 0;

    while(1)
    {        
        if(queue_try_remove(intra_core_queue_rx, &send))
        {
            printf("Core 0: [%02X]\n", send);
            queue_try_add(intra_core_queue_tx, &send);
        }   
        else
        {
            send = 0;
        }

        sleep_ms(100);
    }
    /* TODO: Move output manager to other core? 
             Create object to manage the ports/decode the messages from the queue if output manager is staying on core 0 */

    display_mgr->clear();
    sleep_ms(200);
    display_mgr->test();
    
    instruction_handler->initialise(state_mgr, 
                                    output_mgr, 
                                    display_mgr, 
                                    storage_mgr,
                                    &command_flag, 
                                    &irq_flag);
                                         
    state_mgr->initialise(storage_mgr);
    storage_mgr->initialise(state_mgr);
    display_mgr->initialise(state_mgr);

    instruction_handler->startup_routine();

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

// void input_detect_callback(uint gpio, uint32_t events)
// {
//     if(command_flag == false && irq_flag == false)
//     {
//         command_flag = true;
//         irq_flag = true;
//     }
// }

