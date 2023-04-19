#pragma once

#include <mu-api/api.h>
#include <mu-base/std.h>

#define PAGE_SIZE (kib(4))

typedef struct
{
    uintptr_t ptr;
    usize size;
} Maybe$(AllocObj);

MaybeAllocObj embed_alloc(usize size);
void embed_free(AllocObj *self);