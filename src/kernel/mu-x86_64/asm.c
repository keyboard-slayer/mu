#include "asm.h"

u8 asm_in8(u16 port)
{
    u8 data;
    __asm__ volatile("inb %1, %0"
                     : "=a"(data)
                     : "d"(port));
    return data;
}

u16 asm_in16(u16 port)
{
    u16 data;
    __asm__ volatile("inw %1, %0"
                     : "=a"(data)
                     : "d"(port));
    return data;
}

void asm_out8(u16 port, u8 data)
{
    __asm__ volatile("outb %0, %1"
                     :
                     : "a"(data), "d"(port));
}

void asm_out16(u16 port, u16 data)
{
    __asm__ volatile("outw %0, %1"
                     :
                     : "a"(data), "d"(port));
}

void asm_write_msr(u64 msr, u64 value)
{
    u32 low = value & 0xFFFFFFFF;
    u32 high = value >> 32;
    __asm__ volatile("wrmsr" ::"c"((u64)msr), "a"(low), "d"(high));
}

u64 asm_read_msr(u64 msr)
{
    u32 low, high;
    __asm__ volatile("rdmsr"
                     : "=a"(low), "=d"(high)
                     : "c"((u64)msr));
    return ((u64)high << 32) | low;
}
