#ifndef DISPLAY_MANAGER_H
#define DISPLAY_MANAGER_H

/* C/C++ Includes */
#include <string_view>

/* Pico SDK Includes */

/* Project Includes */
#include "gpio_defs.h"
#include "HT16K33.h"
#include "state_manager.h"

class DisplayManager
{
    private:
        //HT16K33 *itsAlphaNumeric;
        HT16K33 itsAlphaNumeric;
        StateManager *pStateManager;

    public:
        DisplayManager(i2c_inst_t *i2c_instance,
                          uint8_t an_address);

        void initialise(StateManager *pStateManager);
        void update(void);
        void clear(void);
        void update_patch(uint8_t value);
        void update_bank(uint8_t value);
        void write_string(char *str);
        void write_character(char character, uint8_t position);
        void test(void);
};

#endif