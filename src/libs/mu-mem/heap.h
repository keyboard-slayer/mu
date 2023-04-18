#pragma once

#include <mu-traits/alloc.h>

Alloc heap_acquire(void);
void heap_release(Alloc *self);