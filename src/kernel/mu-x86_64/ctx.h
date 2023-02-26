#pragma once

#include <stdint.h>

#include <mu-x86_64/regs.h>

struct _HalCtx
{
    uintptr_t syscall_kernel_stack;
    uintptr_t syscall_user_stack;

    struct _HalRegs regs;
};

typedef struct
{
    uint64_t arg1;
    uint64_t arg2;
    uint64_t arg3;
    uint64_t arg4;
    uint64_t arg5;
} TaskArgs;