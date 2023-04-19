#pragma once

#include <mu-base/std.h>

typedef struct _Alloc
{
    MaybePtr (*malloc)(struct _Alloc *self, usize size);
    MaybePtr (*realloc)(struct _Alloc *self, void *ptr, usize size);
    MaybePtr (*calloc)(struct _Alloc *self, usize nmemb, usize size);
    void (*free)(struct _Alloc *self, void *ptr, usize size);
    void (*release)(struct _Alloc *self);
} Alloc;

typedef Alloc (*AllocAcquireFn)(void);