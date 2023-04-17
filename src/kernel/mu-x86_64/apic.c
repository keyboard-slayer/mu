#include <mu-base/std.h>
#include <mu-hal/hal.h>
#include <mu-misc/lock.h>

#include "apic.h"
#include "asm.h"
#include "pit.h"

static Madt *madt = NULL;

/* --- Lapic ---------------------------------------------------------------- */

unused static u32 lapic_read(u32 reg)
{
    return *((volatile u32 *)(hal_mmap_lower_to_upper(madt->lapic_addr) + reg));
}

static void lapic_write(u32 reg, u32 value)
{
    *((volatile u32 *)(hal_mmap_lower_to_upper(madt->lapic_addr) + reg)) = value;
}

void lapic_timer_start(void)
{
    u32 ticks;

    lapic_write(LAPIC_TPR, 0);

    lapic_write(LAPIC_TIMER_DIV, LAPIC_DIVIDE_BY_16);
    lapic_write(LAPIC_INIT, 0xffffffff);

    pit_sleep(10);

    lapic_write(LAPIC_TIMER_REG, LAPIC_TIMER_MASK);

    ticks = 0xffffffff - lapic_read(LAPIC_CURRENT);

    lapic_write(LAPIC_TIMER_REG, irq(0) | LAPIC_TIMER_PERIODIC);
    lapic_write(LAPIC_TIMER_DIV, LAPIC_DIVIDE_BY_16);
    lapic_write(LAPIC_INIT, ticks);
}

static void lapic_enable(void)
{
    asm_write_msr(MSR_APICBASE, (asm_read_msr(MSR_APICBASE) | LAPIC_ENABLE) & ~((1 << 10)));
    lapic_write(LAPIC_SPURIOUS, lapic_read(LAPIC_SPURIOUS) | 0x1ff);

    lapic_timer_start();
}

void lapic_timer_stop(void)
{
    lapic_write(LAPIC_INIT, 0);
    lapic_write(LAPIC_TIMER_REG, 1 << 16);
}

void lapic_eoi(void)
{
    if (madt == NULL)
    {
        return;
    }

    lapic_write(LAPIC_EOI, 0);
}

int lapic_id(void)
{
    if (madt == NULL)
    {
        return 0;
    }

    return lapic_read(LAPIC_CPU_ID) >> 24;
}

/* --- Ioapic --------------------------------------------------------------- */

static void ioapic_write(MadtIoapic *io_apic, u32 reg, u32 value)
{
    uintptr_t base = (uintptr_t)hal_mmap_lower_to_upper(io_apic->ioapic_addr);
    *(volatile u32 *)base = reg;
    *(volatile u32 *)(base + 16) = value;
}

static u32 ioapic_read(MadtIoapic *ioapic, u32 reg)
{
    uintptr_t base = (uintptr_t)hal_mmap_lower_to_upper(ioapic->ioapic_addr);
    *(volatile u32 *)(base) = reg;
    return *(volatile u32 *)(base + 0x10);
}

static void ioapic_redirect_legacy(void)
{
    for (usize i = 0; i < 16; i++)
    {
        ioapic_redirect_irq(0, irq(i), i);
    }
}

static MadtIso *madt_get_iso_irq(u8 irq)
{
    usize i = 0;
    while (i < madt->header.length - sizeof(Madt))
    {
        MadtEntry *entry = (MadtEntry *)madt->entries + i;

        if (entry->type == 2)
        {
            MadtIso *iso = (MadtIso *)entry;
            if (iso->irq_src == irq)
            {
                return iso;
            }
        }

        i += max(2, entry->length);
    }

    return NULL;
}

static usize ioapic_gsi_count(MadtIoapic *ioapic)
{
    u32 val = ioapic_read(ioapic, 1);
    IoapicVer *ver = (IoapicVer *)&val;
    return ver->max_redirect;
}

MadtIoapic *madt_get_ioapic_from_gsi(u32 gsi)
{
    usize i = 0;
    MadtEntry *entry;
    while (i < madt->header.length - sizeof(Madt))
    {
        entry = (MadtEntry *)(madt->entries + i);

        if (entry->type == 1)
        {
            MadtIoapic *ioapic = (MadtIoapic *)entry;

            if (gsi >= ioapic->gsib && gsi < ioapic->gsib + ioapic_gsi_count(ioapic))
            {
                return ioapic;
            }
        }

        i += max(2, entry->length);
    }

    return NULL;
}

static void ioapic_set_gsi_redirect(u32 lapic_id, u8 intno, u8 gsi, u16 flags)
{
    u32 io_redirect_table;
    IoapicRedirect redirect = {0};
    MadtIoapic *ioapic = madt_get_ioapic_from_gsi(gsi);

    if (ioapic == NULL)
    {
        return;
    }

    redirect.vector = intno;

    if (flags & IOAPIC_ACTIVE_HIGH_LOW)
    {
        redirect.polarity = 1;
    }

    if (flags & IOAPIC_TRIGGER_EDGE_LOW)
    {
        redirect.trigger = 1;
    }

    redirect.dest_id = lapic_id;

    io_redirect_table = (gsi - ioapic->gsib) * 2 + 16;
    ioapic_write(ioapic, io_redirect_table, (u32)redirect._raw.low_byte);
    ioapic_write(ioapic, io_redirect_table + 1, (u32)redirect._raw.high_byte);
}

void ioapic_redirect_irq(u32 lapic_id, u8 intno, u8 irq)
{
    MadtIso *iso = madt_get_iso_irq(irq);
    if (iso != NULL)
    {
        ioapic_set_gsi_redirect(lapic_id, intno, iso->gsi, iso->flags);
    }
    else
    {
        ioapic_set_gsi_redirect(lapic_id, intno, irq, 0);
    }
}

void apic_init(void)
{
    madt = (Madt *)acpi_parse_sdt("APIC");
    lapic_enable();
    debug_info("LAPIC enabled");
    ioapic_redirect_legacy();
    debug_info("IOAPIC enabled and interrupts redirected");

    hal_cpu_sti();
}
