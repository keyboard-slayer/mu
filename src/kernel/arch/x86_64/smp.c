#include <abstract/entry.h>
#include <core/pmm.h>
#include <debug/debug.h>

#include "asm.h"
#include "gdt.h"
#include "idt.h"
#include "smp.h"

static uintptr_t cr3;

static void smp_setup_core(void)
{
    asm_write_cr(3, cr3);
    gdt_flush(gdt_descriptor());
    idt_flush(idt_descriptor());

    debug_raise_exception();

    for (;;)
        ;
}

void smp_init(void)
{
    asm_read_cr(3, cr3);
    abstract_core_goto(smp_setup_core);
}