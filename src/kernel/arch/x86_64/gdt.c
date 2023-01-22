#include "gdt.h"
#include <debug/debug.h>

static Gdt gdt;
static GdtDesc gdt_desc = {
    .len = sizeof(gdt) - 1,
    .offset = (uintptr_t)&gdt,
};

static void gdt_lazy_init(GdtSegment *self, uint8_t access, uint8_t flags)
{
    memset(self, 0, sizeof(GdtSegment));

    if (access == 0 && flags == 0)
    {
        return;
    }

    self->access = access | GDT_ACCESS_PRESENT | GDT_ACCESS_READ_WRITE | GDT_ACCESS_DESCRIPTOR;
    self->flags = flags | GDT_FLAGS_GRANULARITY;

    self->base_high = 0;
    self->base_mid = 0;
    self->base_low = 0;

    self->limit_low = 0xffff;
    self->limit_high = 0x0f;
}

static void gdt_tss_init(GdtTss *self)
{
    uintptr_t tss_ptr = (uintptr_t)self;
    *self = (GdtTss){
        .length = sizeof(GdtTss),

        .base_low = tss_ptr & 0xffff,
        .base_mid = (tss_ptr >> 16) & 0xff,
        .base_high = (tss_ptr >> 24) & 0xff,
        .base_upper = tss_ptr >> 32,

        .flags1 = TSS_FLAGS_PRESENT | TSS_FLAGS_64BITS_AVAILABLE,
        .flags2 = 0,

        .reserved = 0,
    };
}

uintptr_t gdt_descriptor(void)
{
    return (uintptr_t)&gdt_desc;
}

void gdt_init(void)
{
    gdt_lazy_init(&gdt.entries[NULL_DESC], 0, 0);

    gdt_lazy_init(&gdt.entries[KERNEL_CODE], GDT_ACCESS_EXE, GDT_FLAGS_LONG_MODE);
    gdt_lazy_init(&gdt.entries[KERNEL_DATA], 0, GDT_FLAGS_SIZE);

    gdt_lazy_init(&gdt.entries[USER_CODE], GDT_ACCESS_USER | GDT_ACCESS_EXE, GDT_FLAGS_LONG_MODE);
    gdt_lazy_init(&gdt.entries[USER_DATA], GDT_ACCESS_USER, GDT_FLAGS_SIZE);

    gdt_tss_init(&gdt.tss);

    gdt_flush(gdt_descriptor());
    tss_flush();
}