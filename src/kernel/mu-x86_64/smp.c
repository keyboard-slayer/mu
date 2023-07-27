#include <mu-core/pmm.h>
#include <mu-debug/debug.h>
#include <mu-hal/hal.h>
#include <mu-mem/heap.h>
#include <mu-sync/lock.h>

#include "apic.h"
#include "asm.h"
#include "cpu.h"
#include "gdt.h"
#include "idt.h"
#include "smp.h"
#include "syscall.h"

static uintptr_t cr3;
static HalCpu cpus[HAL_CPU_MAX_LEN] = {};
static usize count = 0;
static Spinlock lock = {0};

HalCpu *hal_cpu_self(void)
{
    return hal_cpu_get(lapic_id());
}

HalCpu *hal_cpu_get(usize id)
{
    return &cpus[id];
}

static void smp_setup_core(void)
{
    spinlock_acquire(&lock);
    hal_cpu_self()->present = true;
    hal_cpu_self()->id = lapic_id();

    asm_write_cr(3, cr3);
    gdt_flush(gdt_descriptor());
    idt_flush(idt_descriptor());
    gdt_init_tss();
    syscall_init();
    sched_init();
    apic_init();

    debug_info("Core {} is up and running!", hal_cpu_self()->id);
    spinlock_release(&lock);

    count++;
    loop;
}

void smp_init(void)
{
    hal_cpu_self()->present = true;
    hal_cpu_self()->id = lapic_id();

    asm_read_cr(3, cr3);
    hal_cpu_goto(smp_setup_core);
    while (count != hal_cpu_len() - 1)
        ;
    debug_info("All cores are up and running!");
}