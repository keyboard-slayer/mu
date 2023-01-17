#include <abstract/arch.h>
#include <debug/debug.h>
#include <macro.h>
#include <stdint.h>

#include "asm.h"
#include "regs.h"

static char *exception_messages[32] = {
    "Division By Zero",
    "Debug",
    "Non Maskable Interrupt",
    "Breakpoint",
    "Detected Overflow",
    "Out Of Bounds",
    "Invalid Opcode",
    "No Coprocessor",
    "Double Fault",
    "Coprocessor Segment Overrun",
    "Bad Tss",
    "Segment Not Present",
    "StackFault",
    "General Protection Fault",
    "Page Fault",
    "Unknown Interrupt",
    "Coprocessor Fault",
    "Alignment Check",
    "Machine Check",
    "SIMD Floating-Point Exception",
    "Virtualization Exception",
    "Control Protection Exception",
    "Reserved",
    "Hypervisor Injection Exception",
    "VMM Communication Exception",
    "Security Exception",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
};

static void log_exception(Regs const *regs)
{
    uint64_t cr0;
    uint64_t cr2;
    uint64_t cr3;
    uint64_t cr4;

    asm_read_cr(0, cr0);
    asm_read_cr(2, cr2);
    asm_read_cr(3, cr3);
    asm_read_cr(4, cr4);

    debug(DEBUG_NONE, "\n\n------------------------------------------------------------------------------------\n");
    debug(DEBUG_NONE, "%s on core 0 (0x%x) Err: %x", exception_messages[regs->intno], regs->intno, regs->err);
    debug(DEBUG_NONE, "RAX %p RBX %p RCX %p RDX %p", regs->rax, regs->rbx, regs->rcx, regs->rdx);
    debug(DEBUG_NONE, "RSI %p RDI %p RBP %p RSP %p", regs->rsi, regs->rdi, regs->rbp, regs->rsp);
    debug(DEBUG_NONE, "R8  %p R9  %p R10 %p R11 %p", regs->r8, regs->r9, regs->r10, regs->r11);
    debug(DEBUG_NONE, "R12 %p R13 %p R14 %p R15 %p", regs->r12, regs->r13, regs->r14, regs->r15);
    debug(DEBUG_NONE, "CR0 %p CR2 %p CR3 %p CR4 %p", cr0, cr2, cr3, cr4);
    debug(DEBUG_NONE, "CS  %p SS  %p FLG %p", regs->cs, regs->ss, regs->rflags);
    debug(DEBUG_NONE, "RIP \033[7m%p\033[0m", regs->rip);
    debug(DEBUG_NONE, "\n------------------------------------------------------------------------------------");
}

uintptr_t interrupt_handler(uint64_t rsp)
{
    Regs *regs = (Regs *)rsp;

    if (regs->intno < irq(0))
    {
        log_exception(regs);

        if (regs->intno != 1)
        {
            for (;;)
            {
                arch_cli();
                arch_hlt();
            }
        }
    }

    return rsp;
}