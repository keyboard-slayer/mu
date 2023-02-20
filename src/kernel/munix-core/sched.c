#include "sched.h"
#include <debug/debug.h>
#include <misc/lock.h>

#include "heap.h"

static Spinlock lock = {0};
static size_t tid = 0;

Sched *sched_self(void)
{
    return &hal_cpu_self()->sched;
}

void sched_init(void)
{
    Task *idle = task_init("idle", hal_space_kernel());

    sched_self()->tick = 0;
    sched_self()->task_index = 0;
    vec_init(&sched_self()->tasks, heap_acquire);
    vec_push(&sched_self()->tasks, idle);
    sched_self()->is_init = true;
}

void sched_yield(HalRegs *regs)
{
    if (!sched_self()->is_init || sched_self()->tasks.length < 1)
    {
        return;
    }

    hal_cpu_cli();
    spinlock_acquire(&lock);

    Task *current_task = sched_self()->tasks.data[sched_self()->task_index];
    hal_ctx_save(&current_task->context, regs);

    do
    {
        sched_self()->task_index++;

        if (sched_self()->task_index >= sched_self()->tasks.length)
        {
            sched_self()->task_index = 0;
        }

        current_task = sched_self()->tasks.data[sched_self()->task_index];
    } while (current_task->state != TASK_READY);

    hal_ctx_restore(&current_task->context, regs);
    hal_space_apply(current_task->space);

    hal_cpu_sti();
    spinlock_release(&lock);
}

void sched_push_task(Task *task)
{
    size_t smallest = hal_cpu_get(0)->sched.tasks.length;
    int cpu_id = 0;

    for (size_t i = 1; i < hal_cpu_len(); i++)
    {
        if (smallest < hal_cpu_get(i)->sched.tasks.length)
        {
            smallest = i;
            cpu_id = i;
        }
    }

    vec_push(&hal_cpu_get(cpu_id)->sched.tasks, task);
}

size_t sched_next_tid(void)
{
    return ++tid;
}