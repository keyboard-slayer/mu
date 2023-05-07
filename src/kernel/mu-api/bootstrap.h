#pragma once

#include <mu-base/std.h>

typedef struct packed
{
    char name[128];
    usize len;
    uintptr_t ptr;
} Maybe$(Module);

enum
{
    BOOTSTRAP_REGISTER_SERVER = 0xdeadbeef,
};

MuCap bootstrap_lookup(cstr domain);
void bootstrap_register_server(MuCap port, cstr domain);