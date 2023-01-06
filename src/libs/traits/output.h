#pragma once

#include <stddef.h>

typedef struct _Output
{
    void (*putc)(struct _Output *self, char c);
    size_t (*puts)(struct _Output *self, const char *s, size_t limit);
    void (*release)(struct _Output *self);
} Output;

size_t generic_puts(Output *self, const char *s, size_t limit);