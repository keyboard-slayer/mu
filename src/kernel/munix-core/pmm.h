#pragma once

#include <misc/macro.h>
#include <traits/alloc.h>

typedef struct
{
    uint64_t size;
    uint64_t last_used;
    uint8_t *bitmap;
} PmmBitmap;

void pmm_init(void);
Alloc pmm_acquire(void);
void pmm_release(Alloc *self);
uint64_t pmm_available_pages(void);