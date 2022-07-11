#ifndef OUTPUT_MANAGER_H
#define OUTPUT_MANAGER_H

#include "gpio_defs.h"
#include "pico/stdlib.h"
#include "state_manager.h"

class OutputManager
{
    private:
        StateManager* pStateManager;

    public:
        OutputManager();
        void initialise(StateManager *p_state_mgr);
        void single_blink(uint32_t led);
        void rapid_blink(uint32_t led);
        void reset(void);
        void update(void);
        void set_one(uint32_t led, uint8_t state);
};
#endif