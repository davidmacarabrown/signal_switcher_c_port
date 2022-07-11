#include "debug.h"
#include <stdio.h>
#include <array>

void debug(int time)
{
    gpio_init(25);
    gpio_set_dir(25, GPIO_OUT);
    gpio_put(25, 0);

    for(uint8_t i = 0; i< time; i++)
    {
        gpio_put(25, 1);
        sleep_ms(200);
        gpio_put(25, 0);
        sleep_ms(200);
    }
}

void hang()
{
    printf("Hanging\n"); fflush(stdout);
    while(true)
    {
        sleep_ms(1000);
    }
}

void print_buff(uint8_t* src, uint16_t num_bytes)
{
    for(uint16_t i = 0; i < num_bytes; i++)
    {
        printf("[%02X]", src[i]);
        if((i + 1) % 8 == 0)
        {
            printf("\n");
        }
    }
    printf("\n");
    fflush(stdout);
};