#include "output.h"

size_t generic_puts(Output *self, const char *s, size_t limit)
{
    size_t out = 0;

    while (*s && out < limit)
    {
        self->putc(self, *s++);
        out++;
    }

    return out;
}