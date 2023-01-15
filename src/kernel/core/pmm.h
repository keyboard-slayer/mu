#pragma once

#include <macro.h>
#include <traits/alloc.h>

#define PAGE_SIZE (kib(4))

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