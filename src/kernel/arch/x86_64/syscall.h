#pragma once

#include "regs.h"

typedef int64_t (*SyscallHandler)(Regs *);

void syscall_handle(void);
void syscall_init(void);