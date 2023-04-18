#pragma once

#include <mu-core/task.h>

typedef Vec(Task *) VecTask;

typedef struct
{
    usize task_index;
    usize tick;
    VecTask tasks;

    bool is_init;
} Sched;

void sched_yield(HalRegs *regs);

void sched_init(void);

void sched_push_task(Task *task);

void sched_hlt(void);

Task *task_self(void);

usize sched_next_tid(void);

Sched *sched_self(void);