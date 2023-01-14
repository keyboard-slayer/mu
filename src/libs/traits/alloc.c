#include <libc/string.h>

#include "alloc.h"

void *generic_calloc(Alloc *self, size_t nmemb, size_t size)
{
    void *ptr = self->malloc(self, nmemb * size);

    if (ptr != NULL)
    {
        memset(ptr, 0, nmemb + size);
    }

    return ptr;
}