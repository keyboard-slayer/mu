#pragma once

#include <core/sched.h>

#if defined(__osdk_arch_x86_64__)
#    include <x86_64/smp.h>
#endif

#define MAX_CPU_COUNT (255)

typedef struct
{
    Sched sched;
} Cpu;

size_t cpu_id(void);
Cpu *cpu(size_t id);
Cpu *cpu_self(void);
size_t abstract_cpu_count(void);
void abstract_set_cpu_count(size_t c);