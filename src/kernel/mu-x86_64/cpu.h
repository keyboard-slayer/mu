#pragma once

#include <mu-core/sched.h>

#include "gdt.h"

struct _HalCpu
{
    bool present;
    size_t id;
    Sched sched;
    Tss tss;
};