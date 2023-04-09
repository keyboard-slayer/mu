#pragma once

#include <mu-base/std.h>
#include <mu-traits/alloc.h>

typedef struct
{
    u64 size;
    u64 last_used;
    u8 *bitmap;
} PmmBitmap;

void pmm_init(void);
Alloc pmm_acquire(void);
void pmm_release(Alloc *self);
u64 pmm_available_pages(void);