#include <string.h>

#include "alloc.h"

MaybePtr generic_calloc(Alloc *self, usize nmemb, usize size)
{
    auto ptr = self->malloc(self, nmemb * size);

    if (ptr.isJust)
    {
        memset(ptr.value, 0, nmemb + size);
    }

    return ptr;
}