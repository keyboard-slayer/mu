#pragma once

#include <stddef.h>
#include <stdint.h>

#define MMAP_ENTRIES_LIMIT (128)

typedef enum
{
    MMAP_USABLE,
    MMAP_RESERVED,
    MMAP_RECLAIMABLE,
    MMAP_BAD_MEMORY,
    MMAP_BOOTLOADER_RECLAIMABLE,
    MMAP_KERNEL_AND_MODULES,
    MMAP_FRAMEBUFFER
} MmapType;

typedef struct
{
    uintptr_t base;
    size_t len;
    MmapType type;
} MmapEntry;

typedef struct
{
    size_t count;
    MmapEntry entries[MMAP_ENTRIES_LIMIT];
} Mmap;

uintptr_t abstract_apply_hhdm(uintptr_t addr);
Mmap abstract_get_mmap(void);