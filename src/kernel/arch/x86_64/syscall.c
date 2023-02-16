#include "syscall.h"
#include <core/syscall.h>
#include <debug/debug.h>

#include "asm.h"
#include "gdt.h"
#include "regs.h"

void syscall_init(void)
{
    asm_write_msr(MSR_EFER, asm_read_msr(MSR_EFER) | 1);
    asm_write_msr(MSR_STAR, ((uint64_t)(GDT_KERNEL_CODE * 8) << STAR_KCODE_OFFSET) | ((uint64_t)(((GDT_USER_DATA - 1) * 8) | 3) << STAR_UCODE_OFFSET));
    asm_write_msr(MSR_LSTAR, (uint64_t)syscall_handle);
    asm_write_msr(MSR_SYSCALL_FLAG_MASK, 0xfffffffe);
}

// TODO: Remove this
int64_t syscall_log(Regs *regs)
{
    char const *s = (char const *)regs->rbx;
    debug(DEBUG_INFO, "syscall_log: %s", s);
    return 0;
}

SyscallHandler handlers[] = {
    [SYSCALL_LOG] = syscall_log,
};

int64_t syscall_handler(Regs *regs)
{
    return handlers[regs->rax](regs);
}