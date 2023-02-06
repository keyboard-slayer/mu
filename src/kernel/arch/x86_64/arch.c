#include <abstract/arch.h>
#include <core/pmm.h>

#include "acpi.h"
#include "apic.h"
#include "gdt.h"
#include "idt.h"
#include "smp.h"
#include "vmm.h"

void arch_init(void)
{
    gdt_init();
    idt_init();
    pmm_init();
    vmm_init();
    acpi_init();
    apic_init();
    smp_init();
}

void arch_cli(void)
{
    __asm__("cli");
}

void arch_sti(void)
{
    __asm__("sti");
}

void arch_hlt(void)
{
    __asm__("hlt");
}

void arch_pause(void)
{
    __asm__("pause");
}

void debug_raise_exception(void)
{
    __asm__("int $1");
}