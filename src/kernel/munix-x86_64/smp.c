#include "smp.h"
#include <debug/debug.h>
#include <munix-core/heap.h>
#include <munix-core/pmm.h>
#include <munix-hal/hal.h>

#include "apic.h"
#include "asm.h"
#include "gdt.h"
#include "idt.h"
#include "syscall.h"

static uintptr_t cr3;
static HalCpu cpus[HAL_CPU_MAX_LEN] = {};

HalCpu *hal_cpu_self(void)
{
    return &cpus[lapic_id()];
}

static void smp_setup_core(void)
{
    hal_cpu_self()->present = true;
    hal_cpu_self()->id = lapic_id();

    asm_write_cr(3, cr3);
    gdt_flush(gdt_descriptor());
    idt_flush(idt_descriptor());
    gdt_init_tss();
    syscall_init();
    sched_init();
    loop;
}

void smp_init(void)
{
    asm_read_cr(3, cr3);
    hal_cpu_goto(smp_setup_core);
}