#pragma once

#include <base/macro.h>
#include <traits/alloc.h>

typedef struct
{
    size_t size;
    size_t last_used;
    uint8_t *bitmap;
} PmmBitmap;

void pmm_init(void);
Alloc pmm_acquire(void);
void pmm_release(Alloc *self);
size_t pmm_available_pages(void);