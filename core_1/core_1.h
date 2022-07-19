#ifndef CORE_1_H
#define CORE_1_H

#include <stdio.h>

#include "pico/multicore.h"
#include "pico/util/queue.h"

#include "gpio_defs.h"
#include "MCP23017.H"

void core_1_main(void);

#endif