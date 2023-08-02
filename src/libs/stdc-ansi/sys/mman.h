#pragma once

#include <stddef.h>

typedef enum
{
    PROT_READ = 1 << 0,
    PROT_WRITE = 1 << 1,
    PROT_EXEC = 1 << 2,
} MmapProt;

typedef enum
{
    MAP_PRIVATE = 1 << 0, /* TODO: Needs Copy-on-write */
    MAP_SHARED = 1 << 1,
    MAP_ANON = 1 << 2,
    MAP_FIXED = 1 << 3,
} MmapFlags;

void *mmap(void *addr, size_t len, int prot, int flags, int fd, size_t offset);