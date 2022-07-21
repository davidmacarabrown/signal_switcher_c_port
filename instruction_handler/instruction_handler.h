#ifndef INSTRUCTION_HANDLER_H
#define INSTRUCTION_HANDLER_H

/* C Includes */

/* Pico SDK Incldes */
#include "pico/stdlib.h"
#include "pico/util/queue.h"

/* Project Includes */
#include "gpio_defs.h"

#include "output_manager.h"
#include "state_manager.h"
#include "display_manager.h"
#include "storage_manager.h"

class InstructionHandler
{
    private:

        /* Object Pointers */
        StateManager *pStateManager;
        OutputManager *pOutputManager;
        DisplayManager *pDisplayManager;
        StorageManager *pStorageManager;

        queue_t *tx_queue;
        queue_t *rx_queue;

        QUEUE_ITEM_X queue_store;

        /* For formatting strings for HT16K33 */
        char msg_str[5];

    public:

        void initialise(StateManager *pStateManager,
                            OutputManager *pOutputManager,
                            DisplayManager *pDisplayManager,
                            StorageManager *pStorageManager,
                            queue_t *tx_queue,
                            queue_t *rx_queue
                            );
                            
        void startup_routine(void);
        void read_queue(void);
        void decode_port_input(void);
        void port_a_command_handler(uint8_t input_mask);
        void mode_command_handler(void);
        void write_command_handler(void);
};

#endif
