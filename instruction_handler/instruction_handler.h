#ifndef INSTRUCTION_HANDLER_H
#define INSTRUCTION_HANDLER_H

/* C Includes */

/* Pico SDK Incldes */
#include "pico/stdlib.h"

/* Project Includes */
#include "gpio_defs.h"

#include "output_manager.h"
#include "state_manager.h"
#include "display_manager.h"
#include "storage_manager.h"

class InstructionHandler
{
    private:
        /* Global Variable Pointers */
        bool *command_flag;
        bool *irq_flag;
        
        /* Object Pointers */
        StateManager *pStateManager;
        OutputManager *pOutputManager;
        DisplayManager *pDisplayManager;
        StorageManager *pStorageManager;

        /* State */
        COM_BUFFER_X command_buffer;

        /* For formatting strings for HT16K33 */
        char msg_str[5];

    public:
        void initialise(
                            StateManager *pStateManager,
                            OutputManager *pOutputManager,
                            DisplayManager *pDisplayManager,
                            StorageManager *pStorageManager,
                            bool *command_flag,
                            bool *irq_flag
                            );
                            
        void startup_routine(void);
        void check_for_command_input(void);
        void read_input(void);
        void process_command(void);
        void numerical_command_handler(void);
        void mode_command_handler(void);
        void write_command_handler(void);
};

#endif
