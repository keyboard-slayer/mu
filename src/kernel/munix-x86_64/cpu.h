#pragma once

#include <munix-core/sched.h>

struct _HalCpu
{
    bool present;
    size_t id;
    Sched sched;
};