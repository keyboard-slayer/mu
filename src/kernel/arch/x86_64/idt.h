#pragma once

#include <base/macro.h>
#include <stdint.h>

#define IDT_ENTRIES_LENGTH (256)

#define IDT_INT_PRESENT (1 << 7)
#define IDT_INT_GATE (0xe)

typedef struct packed
{
    uint16_t len;
    uintptr_t offset;
} IdtDesc;

typedef struct packed
{
    uint16_t offset_low;
    uint16_t selector;
    uint8_t ist;
    uint8_t type_attr;
    uint16_t offset_mid;
    uint32_t offset_high;
    uint32_t zero;
} IdtEntry;

typedef struct packed
{
    IdtEntry entries[IDT_ENTRIES_LENGTH];
} Idt;

void idt_init(void);
void idt_flush(uintptr_t);
uintptr_t idt_descriptor(void);

extern uintptr_t __interrupts_vector[];