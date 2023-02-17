#pragma once

#include <stdint.h>

#define PMLX_GET_INDEX(addr, level) (((uint64_t)addr & ((uint64_t)0x1ff << (12 + level * 9))) >> (12 + level * 9))
#define VMM_GET_ADDR(x)             ((x)&0x000ffffffffff000)
#define VMM_PRESENT                 (1 << 0)
#define VMM_WRITE                   (1 << 1)
#define VMM_USER                    (1 << 2)
#define VMM_HUGE                    (1 << 7)
#define VMM_NOEXE                   ((uint64_t)1 << 63)
#define MMAP_FAILURE                (1)
#define MMAP_SUCCESS                (0)

typedef uintptr_t *Space;

void vmm_init(void);