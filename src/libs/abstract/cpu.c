#include "cpu.h"

static Cpu _cpu[MAX_CPU_COUNT] = {};
static size_t _count = 0;

Cpu *cpu(size_t id)
{
    return &_cpu[id];
}

Cpu *cpu_self(void)
{
    return &_cpu[cpu_id()];
}

size_t abstract_cpu_count(void)
{
    return _count;
}

void abstract_set_cpu_count(size_t c)
{
    _count = c;
}