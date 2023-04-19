#include <mu-base/std.h>
#include <mu-hal/hal.h>
#include <mu-mem/heap.h>
#include <mu-misc/lock.h>

#include "sched.h"

#if defined(__x86_64__)
#    include <mu-x86_64/cpu.h>
#else
#    error "Unsupported architecture"
#endif

static Spinlock lock = {0};
static usize tid = 0;

Sched *sched_self(void)
{
    return &hal_cpu_self()->sched;
}

void sched_init(void)
{
    sched_self()->tick = 0;
    sched_self()->task_index = 0;

    vec_init(&sched_self()->tasks, heap_acquire);

    Task *kernel_task = unwrap(task_kernel());
    vec_push(&sched_self()->tasks, kernel_task);

    sched_self()->is_init = true;
}

Task *task_self(void)
{
    return sched_self()->tasks.data[sched_self()->task_index];
}

void sched_yield(HalRegs *regs)
{
    if (!sched_self()->is_init || sched_self()->tasks.length < 1)
    {
        return;
    }

    spinlock_acquire(&lock);
    hal_ctx_save(&task_self()->context, regs);

    do
    {
        sched_self()->task_index++;

        if (sched_self()->task_index == sched_self()->tasks.length)
        {
            sched_self()->task_index = 0;
        }

    } while (task_self()->state != TASK_READY);

    hal_space_apply(task_self()->space);
    hal_ctx_restore(&task_self()->context, regs);

    spinlock_release(&lock);
    return;
}

void sched_push_task(Task *task)
{
    usize smallest = hal_cpu_get(0)->sched.tasks.length;
    int cpu_id = 0;

    for (usize i = 1; i < hal_cpu_len(); i++)
    {
        if (smallest > hal_cpu_get(i)->sched.tasks.length)
        {
            smallest = i;
            cpu_id = i;
        }
    }

    debug_info("Pushing Task {} to CPU {}", task->tid, cpu_id);
    vec_push(&hal_cpu_get(cpu_id)->sched.tasks, task);
}

usize sched_next_tid(void)
{
    return ++tid;
}

void sched_hlt()
{
    for (usize i = 0; i < hal_cpu_len(); i++)
    {
        hal_cpu_get(i)->sched.is_init = false;
    }
}