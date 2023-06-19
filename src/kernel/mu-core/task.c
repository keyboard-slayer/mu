#include <mu-hal/hal.h>
#include <mu-mem/heap.h>
#include <pico-adt/str.h>
#include <string.h>

#include "const.h"
#include "pmm.h"
#include "sched.h"
#include "task.h"

MaybeTaskPtr task_init(Str path, HalSpace *space)
{
    cleanup(heap_release) Alloc heap = heap_acquire();
    Task *self = Try(MaybeTaskPtr, heap.malloc(&heap, sizeof(Task)));
    heap.release(&heap);

    self->state = TASK_READY;
    self->path = path;
    self->tid = sched_next_tid();
    self->space = space;

    cleanup(pmm_release) Pmm pmm = pmm_acquire();
    self->stack = (uintptr_t)Try(MaybeTaskPtr, pmm.malloc(align_up(STACK_SIZE, PAGE_SIZE) / PAGE_SIZE, false)).ptr;
    pmm_release(&pmm);

    hal_space_map(space, USER_STACK_BASE, self->stack, STACK_SIZE, MU_MEM_READ | MU_MEM_WRITE | MU_MEM_USER);

    return Some(MaybeTaskPtr, self);
}

MaybeTaskPtr task_kernel(void)
{
    Alloc heap = heap_acquire();
    Task *self = Try(MaybeTaskPtr, heap.malloc(&heap, sizeof(Task)));
    heap.release(&heap);

    self->space = hal_space_kernel();
    self->state = TASK_READY;
    self->path = str("kernel");
    self->tid = 0;

    return Some(MaybeTaskPtr, self);
}