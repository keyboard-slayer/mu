#include <mu-core/sched.h>
#include <mu-debug/debug.h>
#include <mu-hal/hal.h>
#include <mu-sync/lock.h>
#include <pico-misc/types.h>

#include "apic.h"
#include "asm.h"
#include "regs.h"

unused static char *exception_messages[32] = {
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

struct _StackFrame
{
    struct _StackFrame *rbp;
    u64 rip;
};

static usize dump_backtrace(uintptr_t rbp)
{
    struct _StackFrame *stackframe = (void *)rbp;

    usize i = 0;

    debug(DEBUG_NONE, "Backtrace: ");

    while (stackframe)
    {
        debug(DEBUG_NONE, "* {a}", stackframe->rip);
        stackframe = stackframe->rbp;
    }

    return i;
}

static void log_exception(HalRegs const *regs)
{
    u64 cr0;
    u64 cr2;
    u64 cr3;
    u64 cr4;

    asm_read_cr(0, cr0);
    asm_read_cr(2, cr2);
    asm_read_cr(3, cr3);
    asm_read_cr(4, cr4);

    auto sched = sched_self();

    debug(DEBUG_NONE, "\n\n------------------------------------------------------------------------------------\n");
    if (!sched->is_init)
    {
        debug(DEBUG_NONE, "{} on core {} (0x{x}) Err: 0x{x}", exception_messages[regs->intno], lapic_id(), regs->intno, regs->err);
    }
    else
    {
        debug(DEBUG_NONE, "{} on core {} task {} (0x{x}) Err: 0x{x}", exception_messages[regs->intno], lapic_id(), sched->tasks.data[sched->task_index]->path, regs->intno, regs->err);
    }
    debug(DEBUG_NONE, "RAX {a} RBX {a} RCX {a} RDX {a}", regs->rax, regs->rbx, regs->rcx, regs->rdx);
    debug(DEBUG_NONE, "RSI {a} RDI {a} RBP {a} RSP {a}", regs->rsi, regs->rdi, regs->rbp, regs->rsp);
    debug(DEBUG_NONE, "R8  {a} R9  {a} R10 {a} R11 {a}", regs->r8, regs->r9, regs->r10, regs->r11);
    debug(DEBUG_NONE, "R12 {a} R13 {a} R14 {a} R15 {a}", regs->r12, regs->r13, regs->r14, regs->r15);
    debug(DEBUG_NONE, "CR0 {a} CR2 {a} CR3 {a} CR4 {a}", cr0, cr2, cr3, cr4);
    debug(DEBUG_NONE, "CS  {a} SS  {a} FLG {a}", regs->cs, regs->ss, regs->rflags);
    debug(DEBUG_NONE, "RIP \033[7m{a}\033[0m\n", regs->rip);
    dump_backtrace(regs->rbp);
    debug(DEBUG_NONE, "\n------------------------------------------------------------------------------------");
}

uintptr_t interrupt_handler(u64 rsp)
{
    HalRegs *regs = (HalRegs *)rsp;

    if (regs->intno < irq(0))
    {
        log_exception(regs);

        loop
        {
            hal_cpu_cli();
            hal_cpu_relax();
        }
    }
    else
    {
        switch (regs->intno)
        {
            case irq(0):
            {
                sched_yield(regs);
                break;
            }
        }
    }

    lapic_eoi();
    return rsp;
}