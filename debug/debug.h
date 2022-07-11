#ifndef DEBUG_H
#define DEGUG_H

#include "pico/stdlib.h"

void debug(int time);
void hang();
void print_buff(uint8_t* src, uint16_t num_bytes);

#endif