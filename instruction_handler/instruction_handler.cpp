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
                                        queue_t *tx_queue,
                                        queue_t *rx_queue)
{
    this->pStateManager = pStateManager;
    this->pOutputManager = pOutputManager;
    this->pDisplayManager = pDisplayManager;
    this->pStorageManager = pStorageManager;
    this->tx_queue = tx_queue;
    this->rx_queue = rx_queue;
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
        pDisplayManager->update();
    }
    else
    {
        char str[4];
        memcpy(str, "ERom", 4); //TODO: fix this...
        pDisplayManager->write_string(str);
    }
}

/****************************************************************
Function:   decode_command
Arguments:  none
Return:     none

Fetches a 32bit wide bitmask of all GPIO, masking it further with
GPIO_SWITCH_MASK to filter out the pins which are not relevant,
effectively leaving pins 16 - 22. The resulting value is written
to the command buffer.
****************************************************************/
void InstructionHandler::read_queue(void)
{
    if(queue_try_remove(rx_queue, &queue_store))
    {
        switch(queue_store.instruction_code)
        {
            case PORT_INPUT:
                decode_port_input();
                break;
        }
    }
}

void InstructionHandler::decode_port_input(void)
{
    switch(queue_store.data[0])
    {
        case PORTA:
            switch(queue_store.data[1])
            {
                case SW_1_MASK: /* fall through */
                case SW_2_MASK: /* fall through */
                case SW_3_MASK: /* fall through */
                case SW_4_MASK: /* fall through */
                case SW_5_MASK:
                    port_a_command_handler(queue_store.data[1]); //TODO: Pass value of port mask in
                    break;

                case PATCH_INC_MASK:
                    /* do something manspider! */
                    pStateManager->increment_bank();
                    pStateManager->load_new_bank();
                    pStateManager->set_active_patch(5); //TODO: this is a placeholder for now
                    pDisplayManager->update();
                    break;

                case PATCH_DEC_MASK:
                    /* Execute order 66! */
                    pStateManager->decrement_bank();
                    pStateManager->load_new_bank();
                    pStateManager->set_active_patch(5); //TODO: this is a placeholder for now
                    pDisplayManager->update();
                    break; /* it will be done my lord */
            }
            
        case PORTB:
            switch(queue_store.data[1])
            {
                case SW_WRITE_MASK:
                    write_command_handler();
                    break;
                
                case SW_MODE_MASK:
                    mode_command_handler();
                    break;
            }

        default:
            break;

    }
}

void InstructionHandler::port_a_command_handler(uint8_t input_mask)
{
    uint8_t array_pos;


#ifdef DEBUG
    printf("Numerical Handler\n");
    printf("Command: %d\n", input_mask);

#endif

    /* Convert the captured bit mask value into a zero-indexed value to enable indexing of patch array locations */
    for(uint8_t i = 0; i < NUM_LOOPS; i++)
    {
        if(input_lookup[i] == input_mask)
        {
            array_pos = i;
            break; /* found what we need so break */
        }
    }

#ifdef DEBUG
    printf("Mode: %d\n ", pStateManager->get_mode());
    printf("Single Input: %d\n", array_pos);
    #endif

    switch(pStateManager->get_mode())
    {
        /* Manual mode: change the state of one output and update */
        case MANUAL:
    
    #ifdef DEBUG
            printf("Toggling: %d\n", array_pos);
    
    #endif
            pStateManager->toggle_single_output_state(array_pos);
            break;

        /* Program mode: load the newly selected patch into the output state */
        case PROGRAM:
    
    #ifdef DEBUG
            printf("Now Using Patch: %d - %s\n", (array_pos + 1), pStateManager->get_active_patch_title());
    
    #endif
            pStateManager->set_active_patch(array_pos);
            pStateManager->load_output_state();
            break;
            
        case WRITE:
    
    #ifdef DEBUG
            printf("Set Write Loc: %d\n", array_pos);
    
    #endif
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
            pStateManager->load_output_state();
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
    static char *str;
    
    switch(pStateManager->get_mode())
    {
        case MANUAL: // fall through
        case PROGRAM:
            pStateManager->set_mode(WRITE);
            pStateManager->set_write_location(6);
            break;

        /* insert current output state into appropriate store location and save to flash */
        case WRITE:
            /* if a location has been selected */
            if(pStateManager->copy_output_state()) 
            {
                /* if the write is readback successfully */
                if(pStorageManager->write_patch_switch_data() == 0)
                {
                    pDisplayManager->clear();
                    memcpy(str, ">OK<", 4);
                    pStateManager->set_mode(PROGRAM);
                    pStateManager->set_active_patch(pStateManager->get_write_location());
                }
                else
                {
                    memcpy(str, "-Err", 4);
                }
                sleep_ms(500);
                pDisplayManager->write_string(str); //TODO: Blinking???
            }
            else
            {
                memcpy(str, "Sel_", 4);
                pDisplayManager->write_string(str);
                sleep_ms(500);
            }

            //TODO: stuff here
            break;
        default:
            //should never get here
            break;
    }
    pDisplayManager->update();
}
