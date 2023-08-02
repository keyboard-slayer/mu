#pragma once

#define PMLX_GET_INDEX(addr, level) (((u64)addr & ((u64)0x1ff << (12 + level * 9))) >> (12 + level * 9))
#define VMM_PRESENT                 (1 << 0)
#define VMM_WRITE                   (1 << 1)
#define VMM_USER                    (1 << 2)
#define VMM_HUGE                    (1 << 7)
#define VMM_NOEXE                   ((u64)1 << 63)
#define VMM_GET_ADDR(x)             ((x)&0x000ffffffffff000)
#define VMM_GET_FLAGS(x)            ((x)&0xfff0000000000fff)

void vmm_init(void);