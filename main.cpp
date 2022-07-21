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

InstructionHandler *instruction_handler;
StateManager *state_mgr;
OutputManager *output_mgr;
DisplayManager *display_mgr;
StorageManager *storage_mgr;

queue_t *core_0_queue_tx = new queue_t;
queue_t *core_0_queue_rx = new queue_t;


int main()
{   
    stdio_init_all();
    sleep_ms(5000);

    queue_init_with_spinlock(core_0_queue_tx, sizeof(QUEUE_ITEM_X), 5, 1);
    queue_init_with_spinlock(core_0_queue_rx, sizeof(QUEUE_ITEM_X), 5, 1);
    
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

#ifdef DEBUG
    printf("Objects Created\n");
    printf("Passing Queue pointer to core1\n");
#endif

    multicore_fifo_push_blocking((uint32_t)core_0_queue_rx);
    multicore_fifo_push_blocking((uint32_t)core_0_queue_tx);
    
#ifdef DEBUG
    printf("Done\n");
#endif
    /* TODO: Move output manager to other core? 
             Create object to manage the ports/decode the messages from the queue if output manager is staying on core 0 */

    display_mgr->clear();
    sleep_ms(200);
    display_mgr->test();
    
    instruction_handler->initialise(state_mgr, 
                                    output_mgr, 
                                    display_mgr, 
                                    storage_mgr,
                                    core_0_queue_tx,
                                    core_0_queue_rx);
                                         
    state_mgr->initialise(storage_mgr);
    storage_mgr->initialise(state_mgr);
    display_mgr->initialise(state_mgr);

    instruction_handler->startup_routine();

    while(true)
    {
        instruction_handler->read_queue();
        sleep_ms(16);
    }
}


