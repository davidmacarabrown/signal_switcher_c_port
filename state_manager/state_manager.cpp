/* C Includes */
#include <memory.h>
#include <stdio.h>
#include <malloc.h>

/* Pico SDK Incldes */
#include "pico/stdlib.h"

/* Project Includes */
#include "state_manager.h"
#include "storage_manager.h"

void StateManager::initialise(StorageManager *pStorageManager)
{
    #ifdef DEBUG
    printf("Init Storage Manager\n");
    #endif

    this->pStorageManager = pStorageManager;
    output_mask = 0;
}

/****************************************************************
Function:   load_memory_store
Arguments:  none
Return:     void

Calls for a read operation from the flash storage, which will
memcpy() the bytes to a local buffer which is unpacked into a
memory_store_x structure, the values are then copied to local state.
****************************************************************/
void StateManager::load_memory_store(void)
{
    #ifdef DEBUG
    printf("StateManager::load_memory_store()\n");
    fflush(stdout);
    #endif

    pStorageManager->read_system_data();

    #ifdef DEBUG
    printf("StateManager::load_memory_store()\n");
    printf(" - Loaded memory store\n");
    printf("Mode: %d\n", current_mode);
    fflush(stdout);
    #endif

    loaded_bank = pStorageManager->read_bank(active_bank);
    #ifdef DEBUG
    printf(" - Loaded active bank: %d\n", active_bank);
    fflush(stdout);
    #endif
}


/****************************************************************
Function:   toggle_single_output_state
Arguments:  (uint8_t) index
Return:     void

Toggles a single output state index position to the opposite.
****************************************************************/
void StateManager::toggle_single_output_state(uint8_t index)
{
    uint8_t new_mask;
    uint8_t xor_mask = (OUTPUT_SHIFT_MASK << index);
    printf("XOR Mask: %02x\n", xor_mask); 
    new_mask = output_mask ^ xor_mask;
    printf("OUTPUT Mask: %02x\n", new_mask);

    this->output_mask = new_mask;
}


/****************************************************************
Function:   load_output_state
Arguments:  void
Return:     void

Loads a complete set of patch output values into the output_state.
****************************************************************/
void StateManager::load_output_state(void)
{
    this->output_mask = loaded_bank.patch_array[active_patch].output_mask;
}

void StateManager::clear_output_mask(void)
{
    this->output_mask = 0;
}

/****************************************************************
Function:   copy_output_state
Arguments:  void
Return:     void

Copies the malleable output_state array values into the currently
selected BANK/PATCH position in the bank_array to be written to
flash.
****************************************************************/
bool StateManager::copy_output_state(void)
{
    if(write_location < 5)
    {
        loaded_bank.patch_array[active_patch].output_mask = output_mask;
        return true;
    }
    else
    {
        return false;
    }
}


/****************************************************************
Public Get/Set functions for data access and modification.
****************************************************************/

uint8_t StateManager::get_output_mask(void)
{
    return output_mask;
}

uint8_t StateManager::get_mode(void)
{
    return current_mode;
}

void StateManager::set_mode(uint8_t new_mode)
{
    #ifdef DEBUG
    printf("StateManager::set_mode() - setting Mode to %d\n", new_mode);
    #endif

    this->current_mode = new_mode;

    #ifdef DEBUG
    printf(" - Set current_mode to: %d\n", current_mode);
    fflush(stdout);
    #endif
}

void StateManager::set_prev_mode(uint8_t mode)
{
    prev_mode = mode;
}

void StateManager::use_prev_mode(void)
{
    current_mode = prev_mode;
}

u_int8_t StateManager::get_active_bank(void)
{
  return active_bank;
}

void StateManager::set_active_bank(uint8_t bank)
{
    active_bank = bank;
}

void StateManager::increment_bank(void)
{
    if(active_bank < (NUM_BANKS - 1))
    {
        active_bank++;
    }
}

void StateManager::decrement_bank(void)
{
    if(active_bank > 0)
    {
        active_bank--;
    }
}

u_int8_t StateManager::get_active_patch(void)
{
  return active_patch;
}

void StateManager::set_active_patch(uint8_t patch)
{
    active_patch = patch;
}

char * StateManager::get_active_patch_title(void)
{
    return loaded_bank.patch_array[active_patch].title;
}

void StateManager::set_selected_bank(uint8_t bank)
{
    selected_bank = bank;
}

void StateManager::set_selected_patch(uint8_t patch)
{
    selected_patch = patch;
}

void StateManager::set_write_location(uint8_t loc)
{
    write_location = loc;
}

uint8_t StateManager::get_write_location(void)
{
    return write_location;
}

void StateManager::set_ext_ctrl_a_type(uint8_t type)
{
    this->ext_ctrl_a_type = type;
}

uint8_t StateManager::get_ext_ctrl_a_type(void)
{
    return this->ext_ctrl_a_type;
}

void StateManager::set_ext_ctrl_b_type(uint8_t type)
{
    this->ext_ctrl_b_type = type;
}

uint8_t StateManager::get_ext_ctrl_b_type(void)
{
    return this->ext_ctrl_b_type;
}