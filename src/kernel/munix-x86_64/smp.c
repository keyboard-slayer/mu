#include "smp.h"
#include <debug/debug.h>
#include <misc/lock.h>
#include <munix-core/heap.h>
#include <munix-core/pmm.h>
#include <munix-hal/hal.h>

#include "apic.h"
#include "asm.h"
#include "cpu.h"
#include "gdt.h"
#include "idt.h"
#include "syscall.h"

static uintptr_t cr3;
static HalCpu cpus[HAL_CPU_MAX_LEN] = {};
static Spinlock lock = {0};
static size_t count = 0;

HalCpu *hal_cpu_self(void)
{
    return hal_cpu_get(lapic_id());
}

HalCpu *hal_cpu_get(size_t id)
{
    return &cpus[id];
}

size_t hal_cpu_len()
{
    return count;
}

static void smp_setup_core(void)
{
    spinlock_acquire(&lock);
    count++;
    hal_cpu_self()->present = true;
    hal_cpu_self()->id = lapic_id();

    asm_write_cr(3, cr3);
    gdt_flush(gdt_descriptor());
    idt_flush(idt_descriptor());
    gdt_init_tss();
    syscall_init();
    sched_init();
    spinlock_release(&lock);
    loop;
}

void smp_init(void)
{
    asm_read_cr(3, cr3);
    hal_cpu_goto(smp_setup_core);
}