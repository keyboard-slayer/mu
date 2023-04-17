#pragma once

#include <mu-base/std.h>

typedef struct packed
{
    char name[128];
    usize len;
    uintptr_t ptr;
} Module;

int bootstrap_check_in(void *channel, cstr domain);
