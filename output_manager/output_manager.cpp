#include "output_manager.h"
#include "pico/stdlib.h"
#include "gpio_defs.h"

void OutputManager::initialise(StateManager *pStateManager, MCP23017 *pOutputPort)
{
    this->pStateManager = pStateManager;
    this->pOutputPort   = pOutputPort;
}

void OutputManager::single_blink(uint32_t led)
{
    set_one(led, LOW);
    set_one(led, HIGH);
    sleep_ms(SLOW_BLINK_TIME);
    set_one(led, LOW);
}

void OutputManager::rapid_blink(uint32_t led)
{
    uint8_t counter;

    set_one(led, LOW);

    while(counter < BLINK_REPEATS)
    {
        set_one(led, HIGH);
        sleep_ms(RAPID_BLINK_TIME);
        set_one(led, LOW);
        sleep_ms(RAPID_BLINK_TIME);
        counter++;
    }
    
}

void OutputManager::reset(void)
{
    pOutputPort->write_mask(0, 0x00);
}


void OutputManager::update(void)
{   
    pOutputPort->write_mask(0, pStateManager->get_output_mask());
}

void OutputManager::set_one(uint8_t pin, uint8_t state)
{
    if(pin <= 7) // pin is on Port A
    {
        pOutputPort->write_pin(0, pin, state);
    }
    else // pin is on Port B
    {
        pin -= 8;
        pOutputPort->write_pin(1, pin, state);
    }
}