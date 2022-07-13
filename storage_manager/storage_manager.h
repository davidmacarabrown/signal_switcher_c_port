#ifndef STORAGE_MANAGER_H
#define STORAGE_MANAGER_H

/* C/C++ Includes */

/* Pico Includes */
#include "pico/stdlib.h"

/* Project Includes */
#include "gpio_defs.h"
#include "CAT24C32.h"

class StateManager;
extern const char* PATCH_DEFAULT_TITLE;
class StorageManager
{
    private:
        CAT24C32 eeprom;
        StateManager *pStateManager;

        uint8_t write_buffer[CAT24C32_PAGE_SIZE];
        uint8_t read_buffer[CAT24C32_PAGE_SIZE];

    public:
        StorageManager(i2c_inst_t *i2c_instance, uint8_t i2c_address);
        void initialise(StateManager *pStateManager);
        void factory_reset(void);

        uint8_t validate_eeprom(void);

        void read_system_data(void);
        void write_system_data(void); //TODO: maybe don't need this...

        uint8_t write_patch_switch_data(void);
        BANK_DATA_X read_bank(uint8_t bank);
        PATCH_DATA_X read_patch(uint8_t bank, uint8_t patch);

        uint8_t write_system_flags(uint8_t mode, uint8_t ctrl_a, uint8_t ctrl_b);
        uint8_t write_active_bank(uint8_t bank);
        uint8_t write_active_patch(uint8_t patch);

        uint8_t write_patch_title(uint8_t bank, uint8_t patch, uint8_t* title);
        uint8_t write_patch_output_mask();
        uint8_t write_patch_midi_pc(uint8_t bank, uint8_t patch, uint8_t midi_pc_location, MIDI_PC_DATA_X x_midi_pc_data);
        uint8_t write_patch_midi_cc(uint8_t bank, uint8_t patch, uint8_t midi_cc_location, MIDI_CC_DATA_X x_midi_cc_data);

        void test(void);
};

#endif