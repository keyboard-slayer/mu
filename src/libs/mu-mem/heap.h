#pragma once

#include <pico-traits/alloc.h>

Alloc heap_acquire(void);
void heap_release(Alloc *self);