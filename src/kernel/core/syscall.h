#pragma once

#include <stdint.h>

typedef struct
{
    uint64_t syscall_id;
    uint64_t arg1;
    uint64_t arg2;
    uint64_t arg3;
    uint64_t arg4;
    uint64_t arg5;
} SyscallArgs;