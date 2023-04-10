#include <mu-base/std.h>

#include "vec.h"

void vec_expand_(char **data, usize *length, usize *capacity, int memsz, AllocAcquireFn alloc_fn)
{
    if (*length + 1 > *capacity)
    {
        void *ptr;
        usize n = (*capacity == 0) ? 1 : *capacity << 1;

        Alloc alloc = alloc_fn();
        ptr = unwrap(alloc.realloc(&alloc, *data, n * memsz));
        alloc.release(&alloc);

        *data = ptr;
        *capacity = n;
    }
}