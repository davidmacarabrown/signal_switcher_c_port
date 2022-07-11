#include "output_manager.h"
#include "pico/stdlib.h"
#include "gpio_defs.h"

// TODO: Move this stuff to the int main() setup and get rid of constructor??
OutputManager::OutputManager()
{
    gpio_init(RELAY_ONE);
    gpio_init(RELAY_TWO);
    gpio_init(RELAY_THREE);
    gpio_init(RELAY_FOUR);
    gpio_init(RELAY_FIVE);
    gpio_init(RELAY_AMP_SW_A);
    gpio_init(RELAY_AMP_SW_B);
    gpio_init(LED_WRITE);

    gpio_set_dir(RELAY_ONE, GPIO_OUT);
    gpio_set_dir(RELAY_TWO, GPIO_OUT);
    gpio_set_dir(RELAY_THREE, GPIO_OUT);
    gpio_set_dir(RELAY_FOUR, GPIO_OUT);
    gpio_set_dir(RELAY_FIVE, GPIO_OUT);
    gpio_set_dir(RELAY_AMP_SW_A, GPIO_OUT);
    gpio_set_dir(RELAY_AMP_SW_B, GPIO_OUT);
    gpio_set_dir(LED_WRITE, GPIO_OUT);

    reset();
}

void OutputManager::initialise(StateManager *p_state_mgr)
{
    pStateManager = p_state_mgr;
}

void OutputManager::single_blink(uint32_t led)
{
    gpio_put(led, LOW);
    gpio_put(led, HIGH);
    sleep_ms(SLOW_BLINK_TIME);
    gpio_put(led, LOW);
}

void OutputManager::rapid_blink(uint32_t led)
{
    uint8_t i = 0;
    gpio_put(led, LOW); /* Put given GPIO low as precaution */

    while (i < BLINK_REPEATS)
    {
        gpio_put(led, HIGH);
        sleep_ms(RAPID_BLINK_TIME);
        gpio_put(led, LOW);
        sleep_ms(RAPID_BLINK_TIME);
        i++;
    }
}

void OutputManager::reset(void)
{
    gpio_put_masked(OUTPUT_MASK, LOW);
}


void OutputManager::update(void)
{   
    uint32_t mask = pStateManager->get_output_mask();
    reset();
    gpio_put_masked(mask ,mask);
}

void OutputManager::set_one(uint32_t led, uint8_t state)
{
    gpio_put(led, state);
}