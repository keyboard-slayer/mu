#include <mu-base/std.h>
#include <mu-core/core.h>
#include <mu-hal/hal.h>

#include <mu-x86_64/asm.h>
#include <mu-x86_64/gdt.h>
#include <mu-x86_64/syscall.h>

void syscall_init(void)
{
    asm_write_msr(MSR_EFER, asm_read_msr(MSR_EFER) | 1);
    asm_write_msr(MSR_STAR, ((u64)(GDT_KERNEL_CODE * 8) << STAR_KCODE_OFFSET) | ((u64)(((GDT_USER_DATA - 1) * 8) | 3) << STAR_UCODE_OFFSET));
    asm_write_msr(MSR_LSTAR, (u64)syscall_handle);
    asm_write_msr(MSR_SYSCALL_FLAG_MASK, 0xfffffffe);
}

void syscall_set_gs(uintptr_t addr)
{
    asm_write_msr(MSR_GS_BASE, addr);
    asm_write_msr(MSR_KERN_GS_BASE, addr);
}

i64 syscall_handler(HalRegs *regs)
{
    return mu_core_syscall(regs->rax, (MuArgs){regs->rdi, regs->rsi, regs->rdx, regs->r10, regs->r8, regs->r9});
}