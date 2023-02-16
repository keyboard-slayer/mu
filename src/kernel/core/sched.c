#include "sched.h"
#include <abstract/arch.h>
#include <abstract/cpu.h>
#include <abstract/mem.h>
#include <debug/debug.h>
#include <misc/lock.h>

#include "heap.h"

static Spinlock lock = {0};
static size_t tid = 0;

Sched *sched_self(void)
{
    return &cpu_self()->sched;
}

void sched_init(void)
{
    Task *idle = task_init("idle", abstract_get_kernel_space());

    sched_self()->tick = 0;
    sched_self()->task_index = 0;
    vec_init(&sched_self()->tasks, heap_acquire);
    vec_push(&sched_self()->tasks, idle);
    sched_self()->is_init = true;
}

void sched_yield(Regs *regs)
{
    if (!sched_self()->is_init || sched_self()->tasks.length < 1)
    {
        return;
    }

    arch_cli();
    spinlock_acquire(&lock);

    Task *current_task = sched_self()->tasks.data[sched_self()->task_index];
    context_save(&current_task->context, regs);

    do
    {
        sched_self()->task_index++;

        if (sched_self()->task_index >= sched_self()->tasks.length)
        {
            sched_self()->task_index = 0;
        }

        current_task = sched_self()->tasks.data[sched_self()->task_index];

    } while (current_task->state != TASK_READY);

    context_switch(&current_task->context, regs);
    abstract_switch_space(current_task->space);

    arch_sti();
    spinlock_release(&lock);
}

void sched_push_task(Task *task)
{
    size_t smallest = cpu(0)->sched.tasks.length;
    int cpu_id = 0;

    for (size_t i = 1; i < abstract_cpu_count(); i++)
    {
        if (smallest < cpu(i)->sched.tasks.length)
        {
            smallest = i;
            cpu_id = i;
        }
    }

    vec_push(&cpu(cpu_id)->sched.tasks, task);
}

size_t sched_next_tid(void)
{
    return ++tid;
}