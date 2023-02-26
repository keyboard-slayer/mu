#include <stdint.h>

#include "asm.h"

uint8_t asm_in8(uint16_t port)
{
    uint8_t data;
    __asm__ volatile("inb %1, %0"
                     : "=a"(data)
                     : "d"(port));
    return data;
}

uint16_t asm_in16(uint16_t port)
{
    uint16_t data;
    __asm__ volatile("inw %1, %0"
                     : "=a"(data)
                     : "d"(port));
    return data;
}

void asm_out8(uint16_t port, uint8_t data)
{
    __asm__ volatile("outb %0, %1"
                     :
                     : "a"(data), "d"(port));
}

void asm_out16(uint16_t port, uint16_t data)
{
    __asm__ volatile("outw %0, %1"
                     :
                     : "a"(data), "d"(port));
}

void asm_write_msr(uint64_t msr, uint64_t value)
{
    uint32_t low = value & 0xFFFFFFFF;
    uint32_t high = value >> 32;
    __asm__ volatile("wrmsr" ::"c"((uint64_t)msr), "a"(low), "d"(high));
}

uint64_t asm_read_msr(uint64_t msr)
{
    uint32_t low, high;
    __asm__ volatile("rdmsr"
                     : "=a"(low), "=d"(high)
                     : "c"((uint64_t)msr));
    return ((uint64_t)high << 32) | low;
}
