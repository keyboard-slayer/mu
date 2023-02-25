#include <munix-hal/hal.h>
#include <stdatomic.h>

#include "lock.h"

void spinlock_acquire(Spinlock *self)
{
    while (!__sync_bool_compare_and_swap(self, 0, 1))
    {
        hal_cpu_relax();
    }
}

void spinlock_release(Spinlock *self)
{
    __sync_bool_compare_and_swap(self, 1, 0);
}