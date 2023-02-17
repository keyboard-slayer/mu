#pragma once

#include <stdbool.h>

#include "apic.h"
#include "gdt.h"

typedef struct
{
    bool present;
    Tss tss;
} CpuImpl;

void smp_init(void);
CpuImpl *cpu_impl_self(void);