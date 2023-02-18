#include <abstract/arch.h>
#include <stdatomic.h>

#include "lock.h"

void spinlock_acquire(Spinlock *self)
{
    
    while (!__sync_bool_compare_and_swap(self, 0, 1))
    {
        arch_pause();
    }
}

void spinlock_release(Spinlock *self)
{
    __sync_bool_compare_and_swap(self, 1, 0);
}