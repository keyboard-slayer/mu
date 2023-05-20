#pragma once

#include <mu-base/std.h>

typedef struct packed
{
    char name[128];
    usize len;
    uintptr_t ptr;
} Maybe$(Module);