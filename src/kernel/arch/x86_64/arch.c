#include <abstract/arch.h>
#include <pmm.h>

#include "gdt.h"
#include "idt.h"
#include "vmm.h"

void arch_init(void)
{
    gdt_init();
    idt_init();
    pmm_init();
    vmm_init();
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