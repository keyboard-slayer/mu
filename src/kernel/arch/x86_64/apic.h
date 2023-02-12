#pragma once

#include <misc/macro.h>

#include "acpi.h"

#define LAPIC_EOI               (0x0b0)
#define MSR_APICBASE            (0x01b)
#define LAPIC_ENABLE            (0x800)
#define LAPIC_SPURIOUS          (0x0f0)
#define LAPIC_CPU_ID            (0x020)
#define LAPIC_EOI               (0x0b0)
#define LAPIC_SPURIOUS          (0x0f0)
#define LAPIC_TPR               (0x080)
#define LAPIC_TIMER_REG         (0x320)
#define LAPIC_INIT              (0x380)
#define LAPIC_CURRENT           (0x390)
#define LAPIC_TIMER_DIV         (0x3e0)
#define LAPIC_ENABLE            (0x800)
#define LAPIC_DIVIDE_BY_16      (3)
#define LAPIC_TIMER_MASK        (0x10000)
#define LAPIC_TIMER_PERIODIC    (0x20000)
#define IOAPIC_ACTIVE_HIGH_LOW  (1 << 1)
#define IOAPIC_TRIGGER_EDGE_LOW (1 << 3)

typedef struct packed
{
    AcpiSdt header;
    uint32_t lapic_addr;
    uint32_t flags;
    uint8_t entries[];
} Madt;

typedef struct packed
{
    uint8_t type;
    uint8_t length;
} MadtEntry;

typedef struct packed
{
    MadtEntry header;
    uint8_t ioapic_id;
    uint8_t _reserved;
    uint32_t ioapic_addr;
    uint32_t gsib;
} MadtIoapic;

typedef struct packed
{
    uint8_t version;
    uint8_t reserved;
    uint8_t max_redirect;
    uint8_t reserved2;
} IoapicVer;

typedef union packed
{
    struct packed
    {
        uint8_t vector;
        uint8_t delivery_mode : 3;
        uint8_t dest_mode : 1;
        uint8_t delivery_status : 1;
        uint8_t polarity : 1;
        uint8_t remote_irr : 1;
        uint8_t trigger : 1;
        uint8_t mask : 1;
        uint8_t reserved : 7;
        uint8_t dest_id;
    } _redirect;

    struct packed
    {
        uint32_t low_byte;
        uint32_t high_byte;
    } _raw;
} IoapicRedirect;

typedef struct packed
{
    MadtEntry header;
    uint8_t bus_src;
    uint8_t irq_src;
    uint32_t gsi;
    uint16_t flags;
} MadtIso;

void apic_init(void);
void lapic_eoi(void);
int lapic_id(void);
void ioapic_redirect_irq(uint32_t lapic_id, uint8_t intno, uint8_t irq);