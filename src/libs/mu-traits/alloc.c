#include <string.h>

#include "alloc.h"

void *generic_calloc(Alloc *self, usize nmemb, usize size)
{
    void *ptr = self->malloc(self, nmemb * size);

    if (ptr != NULL)
    {
        memset(ptr, 0, nmemb + size);
    }

    return ptr;
}