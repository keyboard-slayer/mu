#pragma once

#include <traits/alloc.h>

Alloc heap_acquire(void);
void heap_release(Alloc *alloc);