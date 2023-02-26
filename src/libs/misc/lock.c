#include <munix-hal/hal.h>
#include <stdatomic.h>

#include "lock.h"

static size_t retain_count = 0;

static void retain_interrupts()
{
    if (retain_count == 0)
    {
        hal_cpu_cli();
    }

    retain_count++;
}

static void release_interrupts()
{
    retain_count--;
    if (retain_count == 0)
    {
        hal_cpu_sti();
    }
}

void spinlock_acquire(Spinlock *self)
{
    retain_interrupts();
    while (!__sync_bool_compare_and_swap(self, 0, 1))
    {
        hal_cpu_relax();
    }
}

void spinlock_release(Spinlock *self)
{
    __sync_bool_compare_and_swap(self, 1, 0);
    release_interrupts();
}