#include "vec.h"
#include <assert.h>
#include <stdlib.h>

void vec_expand_(char **data, size_t *length, size_t *capacity, int memsz, AllocAcquireFn alloc_fn)
{
    if (*length + 1 > *capacity)
    {
        void *ptr;
        size_t n = (*capacity == 0) ? 1 : *capacity << 1;

        Alloc alloc = alloc_fn();
        ptr = alloc.realloc(&alloc, *data, n * memsz);
        alloc.release(&alloc);

        assert(ptr != NULL);

        *data = ptr;
        *capacity = n;
    }
}