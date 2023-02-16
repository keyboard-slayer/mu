#pragma once

#include <stdint.h>

#include "regs.h"

typedef struct
{
    uintptr_t syscall_kernel_stack;
    uintptr_t syscall_user_stack;

    Regs regs;
} Context;

typedef struct
{
    uint64_t arg1;
    uint64_t arg2;
    uint64_t arg3;
    uint64_t arg4;
    uint64_t arg5;
} TaskArgs;