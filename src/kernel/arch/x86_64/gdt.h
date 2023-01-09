// A purpose for GDT

#pragma once
#include <stdint.h>
#include <macro.h>

struct gdt_entry
{
    u16int base_low;
    u16int limit_low;
    u8int access_byte;
    u8int base_middle;
    u8int base_high;
}__attribute__((packed));
typedef struct gdt_entry gdt_entry_t;

struct gdt_ptr
{
    u16int limit;
    uintptr_t base;
}__attribute__((packed));
typedef struct gdt_ptr gdt_ptr_t;

void gdt_init();

