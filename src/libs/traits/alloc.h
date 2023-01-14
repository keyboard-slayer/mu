#pragma once

#include <stddef.h>
#include <stdint.h>

typedef struct _Alloc
{
    void *(*malloc)(struct _Alloc *self, size_t size);
    void *(*realloc)(struct _Alloc *self, void *ptr, size_t size);
    void *(*calloc)(struct _Alloc *self, size_t nmemb, size_t size);
    void (*free)(struct _Alloc *self, void *ptr, size_t size);
    void (*release)(struct _Alloc *self);
} Alloc;

void *generic_calloc(Alloc *self, size_t nmemb, size_t size);