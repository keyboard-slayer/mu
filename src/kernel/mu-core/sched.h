#pragma once

#include <mu-core/task.h>
#include <stdbool.h>
#include <stddef.h>

typedef Vec(Task *) VecTask;

typedef struct
{
    size_t task_index;
    size_t tick;
    VecTask tasks;

    bool is_init;
} Sched;

void sched_yield(HalRegs *regs);

void sched_init(void);

void sched_push_task(Task *task);

void sched_hlt(void);

Task *task_self(void);

size_t sched_next_tid(void);

Sched *sched_self(void);