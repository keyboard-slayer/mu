#include <abstract/const.h>
#include <abstract/cpu.h>
#include <abstract/entry.h>
#include <core/heap.h>
#include <core/pmm.h>
#include <debug/debug.h>

#include "x86_64/const.h"

#include "apic.h"
#include "asm.h"
#include "gdt.h"
#include "idt.h"
#include "smp.h"

static uintptr_t cr3;
static CpuImpl cpus[MAX_CPU_COUNT] = {};

CpuImpl *cpu_impl_self(void)
{
    return &cpus[lapic_id()];
}

static void smp_setup_core(void)
{
    cpu_impl_self()->present = true;

    asm_write_cr(3, cr3);
    gdt_flush(gdt_descriptor());
    idt_flush(idt_descriptor());
    gdt_init_tss();

    for (;;)
        ;
}

void smp_init(void)
{
    asm_read_cr(3, cr3);
    abstract_core_goto(smp_setup_core);
}