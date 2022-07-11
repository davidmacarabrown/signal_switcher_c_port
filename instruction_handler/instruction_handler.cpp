/* C Includes */
#include <stdio.h>
#include <string.h>

/* Pico SDK Incldes */
#include "pico/stdlib.h"
#include "hardware/timer.h"

/* Project Includes */
#include "gpio_defs.h"
#include "instruction_handler.h"

void InstructionHandler::initialise(StateManager *pStateManager,
                                          OutputManager *pOutputManager,
                                          DisplayManager *pDisplayManager,
                                          StorageManager *pStorageManager,
                                          bool *command_flag,
                                          bool *irq_flag)
{
    this->pStateManager = pStateManager;
    this->pOutputManager = pOutputManager;
    this->pDisplayManager = pDisplayManager;
    this->pStorageManager = pStorageManager;
    this->command_flag = command_flag;
    this->irq_flag = irq_flag;
}

void InstructionHandler::startup_routine(void)
{
    uint8_t result;
    uint8_t mode;
    
    result = pStorageManager->validate_eeprom();

    #ifdef DEBUG
    printf("Eprom Validated - Result: %d\n", result);
    #endif

    if(result == 0)
    {
        pStateManager->load_memory_store();

        #ifdef DEBUG
        printf("Memory store loaded\n");
        #endif

        sleep_ms(10);

        mode = pStateManager->get_mode();
        #ifdef DEBUG
        printf("Read Mode: %d\n", mode);
        #endif

        switch(mode)
        {
            case MANUAL:
                pOutputManager->reset();
                break;
            case PROGRAM:
                pStateManager->load_output_state();
                break;
            default:
                //no-op
                break;
        }
        pOutputManager->update();
        pDisplayManager->update_mode(mode); //TODO: Rework this not to need args?
        pDisplayManager->update();
    }
    else
    {
        char str[4];
        memcpy(str, "ERom", 4);
        pDisplayManager->write_string(str);
    }
}


/****************************************************************
Function:   check_for_command_input
Arguments:  pointer to repeating timer object
            pointer to command flag
            pointer to interrupt status
Return:     none

Checks for a command flag and if true calls decode_input() to
establish what command to execute. Releases interrupt flag.
****************************************************************/
void InstructionHandler::check_for_command_input(void)
{
    if(*command_flag)
    {
        printf("Processing a command\n"); fflush(stdout);
        read_input();
        process_command();
        /* Tight loop to halt execution until all inputs are released */
        while(true)
        {
            printf("Spin poll\n");
            /* If any of the pins present in GPIO_SWITCH_MASK are still active this will result in a value > 0 */
            if((gpio_get_all() & GPIO_SWITCH_MASK) == 0)
            {
                break;
            }
            /* Sleep time to effectively poll at 60Hz... may replace with repeating timer later? */
            sleep_ms(16);
        }
    }
    *irq_flag = false;
}


/****************************************************************
Function:   decode_input
Arguments:  none
Return:     none

Fetches a 32bit wide bitmask of all GPIO, masking it further with
GPIO_SWITCH_MASK to filter out the pins which are not relevant,
effectively leaving pins 16 - 22. The resulting value is written
to the command buffer.
****************************************************************/
void InstructionHandler::read_input(void)
{
        sleep_ms(50);
        command_buffer.command = gpio_get_all() & GPIO_SWITCH_MASK;
        printf("GPIO & MASK: %d\n", command_buffer.command);

        /* Set instruction_handler internal flag to true, set global flag back to false */
        *command_flag = false;
        command_buffer.command_flag = true;
}

void InstructionHandler::process_command(void)
{
    if(command_buffer.command_flag) /* if there is no command to process do nothing */
    {
        switch(command_buffer.command)
        {
            case SW_1_MASK: /* fall through */
            case SW_2_MASK: /* fall through */
            case SW_3_MASK: /* fall through */
            case SW_4_MASK: /* fall through */
            case SW_5_MASK:
                numerical_command_handler();
                break;

            case SW_MODE_MASK:
                mode_command_handler();
                break;

            case SW_WRITE_MASK:
                write_command_handler();
                break;

            case PATCH_INC_MASK:
                /* do something manspider! */
                pStateManager->increment_bank();
                break;

            case PATCH_DEC_MASK:
                /* Execute order 66! */
                pStateManager->decrement_bank();
                break; /* it will be done my lord */

            default:
                /* we should never get here... */
                break;
        }
        /* Reset flag and buffer for next iteration */
        command_buffer.command_flag = false;
        command_buffer.command = 0;
    }
}

void InstructionHandler::numerical_command_handler()
{
    uint8_t array_pos;

    printf("Numerical Handler\n");
    printf("Command: %d\n", command_buffer.command);

    /* Convert the captured bit mask value into a zero-indexed value to enable indexing of patch array locations */
    for(uint8_t i = 0; i < NUM_LOOPS; i++)
    {
        if(input_lookup[i] == command_buffer.command)
        {
            array_pos = i;
            break; /* found what we need so break */
        }
    }
    printf("Mode: %d\n ", pStateManager->get_mode());
    printf("Single Input: %d\n", array_pos);

    switch(pStateManager->get_mode())
    {
        /* Manual mode: change the state of one output and update */
        case MANUAL:
            printf("Toggling: %d\n", array_pos);
            pStateManager->toggle_single_output_state(array_pos);
            break;

        /* Program mode: load the newly selected patch into the output state */
        case PROGRAM:
            printf("Now Using Patch: %d - %s\n", (array_pos + 1), pStateManager->get_active_patch_title());
            pStateManager->set_active_patch(array_pos);
            pStateManager->load_output_state();
            break;
            
        case WRITE:
            printf("Set Write Loc: %d\n", array_pos);
            pStateManager->set_write_location(array_pos);
            break;
    }

    /* common calls go here... e.g. update display(s)/outputs... */

    pDisplayManager->update();
    pOutputManager->update();
}

void InstructionHandler::mode_command_handler()
{
    uint8_t previous_mode = pStateManager->get_mode();
    pStateManager->set_prev_mode(previous_mode);
    pStateManager->clear_output_mask();
    pOutputManager->update();

    switch(previous_mode)
    {
        case MANUAL: // enter Program
            pStateManager->set_mode(PROGRAM);
            break;

        case PROGRAM: // enter Manual
            pStateManager->set_mode(MANUAL);
            break;

        case WRITE: // write
            pStateManager->set_mode(MANUAL);
            pStateManager->set_write_location(6); //TODO: Figure out a different way to lock this?
            //TODO: Implement underscore blink for patch location prior to selection
            break;

        case MENU:
            pStateManager->use_prev_mode();
            // exit the menu... if/when it is implemented
            break;
        default:
            // should never get here...
            break;
    }

    // common calls go here, e.g. update display... outputs etc.
    pDisplayManager->update();
    pOutputManager->update();
}

void InstructionHandler::write_command_handler()
{
    char str[4];
    
    switch(pStateManager->get_mode())
    {
        case MANUAL: // fall through
        case PROGRAM:
            pStateManager->set_mode(WRITE);
            pDisplayManager->update_mode(WRITE);
            pStateManager->set_write_location(6);
            break;

        /* insert current output state into appropriate store location and save to flash */
        case WRITE:
            /* if a location has been selected */
            if(pStateManager->copy_output_state()) 
            {
                /* if the write is readback successfully */
                if(pStorageManager->write_patch_switch_data(pStateManager->get_active_bank(), pStateManager->get_write_location(), pStateManager->get_output_mask()) == 0)
                {
                    pDisplayManager->clear();
                    memcpy(str, ">OK<", 4);
                    pStateManager->set_mode(PROGRAM);
                }
                else
                {
                    memcpy(str, "-Err", 4);
                    pOutputManager->rapid_blink(LED_WRITE);
                }
                sleep_ms(500);
                pDisplayManager->write_string(str); //TODO: Blinking???
            }
            else
            {
                memcpy(str, "Sel_", 4);
                pDisplayManager->write_string(str);
                sleep_ms(500);
                pDisplayManager->clear();
                pDisplayManager->update_mode(WRITE);
                pDisplayManager->update_bank(pStateManager->get_active_bank());
                pDisplayManager->write_character('_', 3);
            }

            //TODO: stuff here
            break;
        default:
            //should never get here
            break;
    }
    pDisplayManager->update();
}
