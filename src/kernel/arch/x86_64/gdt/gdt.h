// A purpose for GDT

#pragma once

// Entry of table

struct gdt_entry
{
    u16int base_low;
    u16int limit_low;
    u8int access_byte;
    u8int base_middle;
    u8int base_high;
}__attribute__((packed));

// Pointer of table

struct gdt_ptr
{
    u16int limit;
    u32bit base;
}__attribute__((packed));

