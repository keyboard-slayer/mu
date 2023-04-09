#include <mu-base/std.h>
#include <mu-core/const.h>
#include <mu-core/heap.h>
#include <mu-hal/hal.h>
#include <mu-misc/lock.h>

#include "cpu.h"
#include "gdt.h"
#include "smp.h"

static Gdt gdt = {0};
static Spinlock lock = {0};
static GdtDesc gdt_desc = {
    .len = sizeof(gdt) - 1,
    .offset = (uintptr_t)&gdt,
};

static void gdt_lazy_init(GdtSegment *self, u8 access, u8 flags)
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

void gdt_load_tss(Tss *tss)
{
    uintptr_t tss_ptr = (uintptr_t)tss;

    spinlock_acquire(&lock);

    gdt.tss = (GdtTss){
        .length = sizeof(GdtTss),

        .base_low = tss_ptr & 0xffff,
        .base_mid = (tss_ptr >> 16) & 0xff,
        .base_high = (tss_ptr >> 24) & 0xff,
        .base_upper = tss_ptr >> 32,

        .flags1 = TSS_FLAGS_PRESENT | TSS_FLAGS_64BITS_AVAILABLE,
        .flags2 = 0,

        .reserved = 0,
    };

    spinlock_release(&lock);
}

uintptr_t gdt_descriptor(void)
{
    return (uintptr_t)&gdt_desc;
}

void gdt_init(void)
{
    gdt_lazy_init(&gdt.entries[GDT_NULL_DESC], 0, 0);

    gdt_lazy_init(&gdt.entries[GDT_KERNEL_CODE], GDT_ACCESS_EXE, GDT_FLAGS_LONG_MODE);
    gdt_lazy_init(&gdt.entries[GDT_KERNEL_DATA], 0, GDT_FLAGS_SIZE);

    gdt_lazy_init(&gdt.entries[GDT_USER_DATA], GDT_ACCESS_USER, GDT_FLAGS_SIZE);
    gdt_lazy_init(&gdt.entries[GDT_USER_CODE], GDT_ACCESS_USER | GDT_ACCESS_EXE, GDT_FLAGS_LONG_MODE);
    gdt_load_tss(NULL);

    gdt_flush(gdt_descriptor());
    debugInfo("GDT initialized");

    tss_flush();
    debugInfo("TSS flushed");
}
void gdt_init_tss(void)
{
    Alloc heap = heap_acquire();
    hal_cpu_self()->tss.ist[0] = (uintptr_t)non_null$((heap.malloc(&heap, KERNEL_STACK_SIZE)) + KERNEL_STACK_SIZE);
    hal_cpu_self()->tss.ist[1] = (uintptr_t)non_null$((heap.malloc(&heap, KERNEL_STACK_SIZE)) + KERNEL_STACK_SIZE);
    hal_cpu_self()->tss.rsp[0] = (uintptr_t)non_null$((heap.malloc(&heap, KERNEL_STACK_SIZE)) + KERNEL_STACK_SIZE);
    heap.release(&heap);

    debugInfo("TSS initialized (rsp0: %p, ist1: %p, ist2: %p)", hal_cpu_self()->tss.rsp[0], hal_cpu_self()->tss.ist[1], hal_cpu_self()->tss.ist[2]);

    gdt_load_tss(&hal_cpu_self()->tss);
    tss_flush();
}