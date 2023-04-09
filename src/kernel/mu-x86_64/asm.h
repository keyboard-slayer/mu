#pragma once

#include <mu-base/std.h>

#define MSR_EFER              (0xC0000080)
#define MSR_STAR              (0xC0000081)
#define MSR_LSTAR             (0xC0000082)
#define MSR_SYSCALL_FLAG_MASK (0xC0000084)
#define STAR_KCODE_OFFSET     (32)
#define STAR_UCODE_OFFSET     (48)

#define MSR_KERN_GS_BASE (0xc0000102)
#define MSR_GS_BASE      (0xC0000101)

#define asm_read_cr(n, reg) __asm__ volatile("mov %%cr" #n ", %0" \
                                             : "=r"(reg))

#define asm_write_cr(n, x) __asm__ volatile("mov %0, %%cr" #n ::"r"((x)))

u8 asm_in8(u16 port);
u16 asm_in16(u16 port);

void asm_out8(u16 port, u8 data);
void asm_out16(u16 port, u16 data);

void asm_write_msr(u64 msr, u64 value);
u64 asm_read_msr(u64 msr);