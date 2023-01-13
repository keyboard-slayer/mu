#pragma once

#include <stddef.h>
#include <traits/output.h>

void arch_init(void);
void arch_cli(void);
void arch_sti(void);
void arch_hlt(void);

Output abstract_serial_acquire(void);