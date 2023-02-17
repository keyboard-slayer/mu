#pragma once

#if defined(__x86_64__)
#    include <munix-x86_64/vmm.h>
#endif

#include <stddef.h>
#include <traits/output.h>

void arch_init(void);
void arch_cli(void);
void arch_sti(void);
void arch_hlt(void);
void arch_pause(void);
void abstract_switch_space(Space space);

Output abstract_serial_acquire(void);