#pragma once

#include <pico-misc/macro.h>
#include <pico-misc/types.h>

#define IDT_ENTRIES_LENGTH (256)
#define IDT_INT_PRESENT    (1 << 7)
#define IDT_INT_GATE       (0xe)

typedef struct packed
{
    u16 len;
    uintptr_t offset;
} IdtDesc;

typedef struct packed
{
    u16 offset_low;
    u16 selector;
    u8 ist;
    u8 type_attr;
    u16 offset_mid;
    u32 offset_high;
    u32 zero;
} IdtEntry;

typedef struct packed
{
    IdtEntry entries[IDT_ENTRIES_LENGTH];
} Idt;

void idt_init(void);

void idt_flush(uintptr_t);

uintptr_t idt_descriptor(void);

extern uintptr_t __interrupts_vector[];