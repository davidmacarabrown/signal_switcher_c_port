#ifndef OUTPUT_MANAGER_H
#define OUTPUT_MANAGER_H

#include "gpio_defs.h"
#include "pico/stdlib.h"
#include "state_manager.h"
#include "MCP23017.H"

class OutputManager
{
    private:
        StateManager* pStateManager;
        MCP23017*     pOutputPort;

        uint8_t port_state[2];

    public:
        void initialise(StateManager *p_state_mgr, MCP23017 *pOutputPort);
        void single_blink(uint32_t led);
        void rapid_blink(uint32_t led);
        void reset(void);
        void update(void);
        void set_one(uint8_t pin, uint8_t state);
};
#endif