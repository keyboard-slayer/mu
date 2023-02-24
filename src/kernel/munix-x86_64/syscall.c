#include <debug/debug.h>
#include <munix-core/core.h>
#include <munix-hal/hal.h>

#include <munix-x86_64/asm.h>
#include <munix-x86_64/gdt.h>
#include <munix-x86_64/syscall.h>

void syscall_init(void)
{
    asm_write_msr(MSR_EFER, asm_read_msr(MSR_EFER) | 1);
    asm_write_msr(MSR_STAR, ((uint64_t)(GDT_KERNEL_CODE * 8) << STAR_KCODE_OFFSET) | ((uint64_t)(((GDT_USER_DATA - 1) * 8) | 3) << STAR_UCODE_OFFSET));
    asm_write_msr(MSR_LSTAR, (uint64_t)syscall_handle);
    asm_write_msr(MSR_SYSCALL_FLAG_MASK, 0xfffffffe);
}

void syscall_set_gs(uintptr_t addr)
{
    asm_write_msr(MSR_GS_BASE, addr);
    asm_write_msr(MSR_KERN_GS_BASE, addr);
}

int64_t syscall_handler(HalRegs *regs)
{
    debug(DEBUG_INFO, "syscall: %d", regs->rax);
    return mu_core_syscall(regs->rax, (MuArgs){regs->rdi, regs->rsi, regs->rdx, regs->r10, regs->r8, regs->r9});
}