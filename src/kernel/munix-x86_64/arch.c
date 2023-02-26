#include <munix-core/pmm.h>
#include <munix-core/sched.h>
#include <munix-hal/hal.h>

#include "acpi.h"
#include "apic.h"
#include "gdt.h"
#include "idt.h"
#include "smp.h"
#include "syscall.h"
#include "vmm.h"

void hal_init(void)
{
    gdt_init();
    idt_init();
    pmm_init();
    vmm_init();
    acpi_init();
    apic_init();
    gdt_init_tss();
    syscall_init();
    smp_init();
    sched_init();
}

void hal_cpu_cli(void)
{
    __asm__("cli");
}

void hal_cpu_sti(void)
{
    __asm__("sti");
}

void hal_cpu_halt(void)
{
    __asm__("hlt");
}

void hal_cpu_relax(void)
{
    __asm__("pause");
}

void hal_cpu_stop(void)
{
    while (true)
    {
        __asm__("cli");
        __asm__("hlt");
    }
}
