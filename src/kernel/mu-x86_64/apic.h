#pragma once

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
    u32 lapic_addr;
    u32 flags;
    u8 entries[];
} Madt;

typedef struct packed
{
    u8 type;
    u8 length;
} MadtEntry;

typedef struct packed
{
    MadtEntry header;
    u8 ioapic_id;
    u8 _reserved;
    u32 ioapic_addr;
    u32 gsib;
} MadtIoapic;

typedef struct packed
{
    u8 version;
    u8 reserved;
    u8 max_redirect;
    u8 reserved2;
} IoapicVer;

typedef union packed
{
    struct packed
    {
        u8 vector;
        u8 delivery_mode : 3;
        u8 dest_mode : 1;
        u8 delivery_status : 1;
        u8 polarity : 1;
        u8 remote_irr : 1;
        u8 trigger : 1;
        u8 mask : 1;
        u8 reserved : 7;
        u8 dest_id;
    };

    struct packed
    {
        u32 low_byte;
        u32 high_byte;
    } _raw;
} IoapicRedirect;

typedef struct packed
{
    MadtEntry header;
    u8 bus_src;
    u8 irq_src;
    u32 gsi;
    u16 flags;
} MadtIso;

void apic_init(void);
void lapic_eoi(void);
int lapic_id(void);
void ioapic_redirect_irq(u32 lapic_id, u8 intno, u8 irq);
void lapic_timer_start(void);
void lapic_timer_stop(void);