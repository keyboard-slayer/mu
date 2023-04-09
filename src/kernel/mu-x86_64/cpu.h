#pragma once

#include <mu-core/sched.h>

#include "gdt.h"

struct _HalCpu
{
    bool present;
    usize id;
    Sched sched;
    Tss tss;
};