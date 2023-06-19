#pragma once

#include <pico-misc/types.h>

void pit_init(void);
void pit_sleep(u16);
void pit_set_reload_value(u16);