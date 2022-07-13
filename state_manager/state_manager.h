#ifndef STATE_MANAGER_H
#define STATE_MANAGER_H

/* C/C++ Includes */
#include <array>

/* Pico SDK Incldes */
#include "pico/stdlib.h"

/* Project Includes */
#include "gpio_defs.h"

class StorageManager;

class StateManager
{
    private:
        /* Associations */
        StorageManager *pStorageManager;

        /* Malleable register of the current output state, effectively the "source of truth" for what is active at any given time */
        uint8_t output_mask;

        /* In-Memory storage of all patch data */
        // bank_x bank_array[NUM_BANKS];
        BANK_DATA_X loaded_bank;

        /* Mode & Patch Info */
        uint8_t prev_mode;
        uint8_t active_bank;
        uint8_t active_patch;
        uint8_t selected_bank;
        uint8_t selected_patch;
        uint8_t write_location;

        /* System Info */
        uint8_t current_mode;
        uint8_t ext_ctrl_a_type;
        uint8_t ext_ctrl_b_type;

    public:
        void initialise(StorageManager *pStorageManager);
        void load_memory_store(void);
        void load_new_bank(void);
        void toggle_single_output_state(uint8_t index);
        void load_output_state(void);
        bool copy_output_state(void);
        uint8_t get_output_mask(void);
        void clear_output_mask(void);

        uint8_t get_mode(void);
        void set_mode(uint8_t new_mode);
        void set_prev_mode(uint8_t mode);
        void use_prev_mode(void);

        uint8_t get_active_bank(void);
        void set_active_bank(uint8_t bank);

        void increment_bank(void);
        void decrement_bank(void);

        void set_active_patch(uint8_t patch);
        uint8_t get_active_patch(void);

        char * get_active_patch_title(void);

        void set_selected_bank(uint8_t bank);
        void set_selected_patch(uint8_t);

        void set_write_location(uint8_t loc);
        uint8_t get_write_location(void);

        void set_ext_ctrl_a_type(uint8_t type);
        uint8_t get_ext_ctrl_a_type(void);

        void set_ext_ctrl_b_type(uint8_t type);
        uint8_t get_ext_ctrl_b_type(void);
};

#endif
