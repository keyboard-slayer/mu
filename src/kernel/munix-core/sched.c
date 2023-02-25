#include "sched.h"
#include <debug/debug.h>
#include <misc/lock.h>
#include <munix-hal/hal.h>

#include "heap.h"

#if defined(__x86_64__)
#    include <munix-x86_64/cpu.h>
#else
#    error "Unsupported architecture"
#endif

static Spinlock lock = {0};
static size_t tid = 0;

Sched *sched_self(void)
{
    return &hal_cpu_self()->sched;
}

void sched_init(void)
{
    sched_self()->tick = 0;
    sched_self()->task_index = 0;

    Task *kernel_task = task_kernel();
    vec_init(&sched_self()->tasks, heap_acquire);
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

    hal_ctx_restore(&task_self()->context, regs);
    hal_space_apply(task_self()->space);

    debug(DEBUG_INFO, "Switching to task %s", task_self()->path);

    spinlock_release(&lock);
    return;
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

    debug(DEBUG_INFO, "Pushing task %s to cpu %d", task->path, cpu_id);
    vec_push(&hal_cpu_get(cpu_id)->sched.tasks, task);
}

size_t sched_next_tid(void)
{
    return ++tid;
}

void sched_hlt()
{
    for (size_t i = 0; i < hal_cpu_len(); i++)
    {
        hal_cpu_get(i)->sched.is_init = false;
    }
}