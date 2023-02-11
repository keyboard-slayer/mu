#pragma once

#if defined(__x86_64__)
#    include <x86_64/ctx.h>
#endif

void context_init(Context *self, uintptr_t ip, TaskArgs args);
void context_save(Context *self, Regs *regs);
void context_switch(Context *self, Regs *regs);