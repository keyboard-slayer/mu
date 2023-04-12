#include "writer.h"

void generic_puts(Writer *self, cstr s)
{
    while (*s)
    {
        self->putc(self, *s++);
    }
}