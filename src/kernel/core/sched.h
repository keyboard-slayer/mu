#pragma once

#include <stdbool.h>
#include <stddef.h>

#include "task.h"

typedef struct
{
    size_t task_index;
    size_t tick;
    VecTask tasks;

    bool is_init;
} Sched;

void sched_yield(Regs *regs);
void sched_init(void);