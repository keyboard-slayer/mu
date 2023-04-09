#pragma once

#include <mu-x86_64/regs.h>

struct _HalCtx
{
    uintptr_t syscall_kernel_stack;
    uintptr_t syscall_user_stack;

    struct _HalRegs regs;
};

typedef struct
{
    u64 arg1;
    u64 arg2;
    u64 arg3;
    u64 arg4;
    u64 arg5;
} TaskArgs;