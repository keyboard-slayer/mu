#pragma once

#include <mu-base/std.h>
#include <mu-traits/alloc.h>

typedef struct
{
    u64 size;
    u64 last_used_low;
    u64 last_used_high;
    u8 *bitmap;
} PmmBitmap;

typedef struct
{
    uintptr_t ptr;
    usize len;
} Maybe$(PmmObj);

typedef struct _Pmm
{
    MaybePmmObj (*malloc)(usize size, bool high);
    MaybePmmObj (*calloc)(usize nmemb, usize size, bool high);
    void (*free)(PmmObj obj);
    void (*release)(struct _Pmm *self);
    bool high;
} Pmm;

void pmm_init(void);
Pmm pmm_acquire(void);
void pmm_release(Pmm *self);
void pmm_free(PmmObj *obj);
u64 pmm_available_pages(void);
MaybePtr pmm_alloc_page(usize pages, bool high);
