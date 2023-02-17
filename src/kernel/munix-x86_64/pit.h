#pragma once

#include <stdint.h>

void pit_init(void);
void pit_sleep(uint16_t);
void pit_set_reload_value(uint16_t);