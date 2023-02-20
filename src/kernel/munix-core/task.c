#include "task.h"
#include <debug/debug.h>
#include <munix-hal/hal.h>
#include <string.h>

#include "const.h"
#include "heap.h"
#include "pmm.h"
#include "sched.h"

Task *task_init(char const *path, HalSpace *space)
{
    Alloc heap = heap_acquire();
    Task *self = non_null$(heap.malloc(&heap, sizeof(Task)));
    self->ipc = non_null$(heap.malloc(&heap, sizeof(IpcBuffer)));
    heap.release(&heap);

    self->state = TASK_READY;
    self->path = non_null$(strdup(path));
    self->tid = sched_next_tid();
    self->space = space;

    Alloc pmm = pmm_acquire();
    self->stack = (uintptr_t)non_null$(pmm.malloc(&pmm, align_up(STACK_SIZE, PAGE_SIZE) / PAGE_SIZE));
    pmm_release(&pmm);

    hal_space_map(space, IPC_STRUCT_POS, (uintptr_t)self->ipc, sizeof(IpcBuffer), MU_MEM_READ | MU_MEM_WRITE | MU_MEM_USER | MU_MEM_HUGE);
    hal_space_map(space, USER_STACK_BASE, self->stack, STACK_SIZE, MU_MEM_READ | MU_MEM_WRITE | MU_MEM_USER | MU_MEM_HUGE);

    return self;
}