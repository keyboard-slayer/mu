#pragma once

#include <mu-base/types.h>

typedef struct _Writer
{
    void (*putc)(struct _Writer *self, char c);
    void (*puts)(struct _Writer *self, cstr s);
    void (*release)(struct _Writer *self);
} Writer;

void generic_puts(Writer *self, cstr s);