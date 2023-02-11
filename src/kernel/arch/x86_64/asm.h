#pragma once

#include <stdint.h>

#define MSR_EFER (0xC0000080)
#define MSR_STAR (0xC0000081)
#define MSR_LSTAR (0xC0000082)
#define MSR_SYSCALL_FLAG_MASK (0xC0000084)
#define STAR_KCODE_OFFSET (32)
#define STAR_UCODE_OFFSET (48)

#define MSR_KERN_GS_BASE (0xc0000102)
#define MSR_GS_BASE (0xC0000101)

#define asm_read_cr(n, reg) __asm__ volatile("mov %%cr" #n ", %0" \
                                             : "=r"(reg))

#define asm_write_cr(n, x) __asm__ volatile("mov %0, %%cr" #n ::"r"((x)))

uint8_t asm_in8(uint16_t port);
uint16_t asm_in16(uint16_t port);

void asm_out8(uint16_t port, uint8_t data);
void asm_out16(uint16_t port, uint16_t data);

void asm_write_msr(uint64_t msr, uint64_t value);
uint64_t asm_read_msr(uint64_t msr);