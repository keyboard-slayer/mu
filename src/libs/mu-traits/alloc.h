#pragma once

#include <mu-base/std.h>

typedef struct _Alloc
{
    void *(*malloc)(struct _Alloc *self, usize size);
    void *(*realloc)(struct _Alloc *self, void *ptr, usize size);
    void *(*calloc)(struct _Alloc *self, usize nmemb, usize size);
    void (*free)(struct _Alloc *self, void *ptr, usize size);
    void (*release)(struct _Alloc *self);
} Alloc;

typedef Alloc (*AllocAcquireFn)(void);

void *generic_calloc(Alloc *self, usize nmemb, usize size);