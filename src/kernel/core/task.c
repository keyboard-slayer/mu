#include "task.h"
#include <abstract/const.h>
#include <abstract/mem.h>
#include <debug/debug.h>
#include <string.h>

#include "heap.h"
#include "pmm.h"
#include "sched.h"

Task *task_init(char const *path, uintptr_t *space)
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

    kmmap(space, IPC_STRUCT_POS, (uintptr_t)self->ipc, sizeof(IpcBuffer), PROT_READ | PROT_WRITE | MMAP_USER | MMAP_HUGE);
    kmmap(space, USER_STACK_BASE, self->stack, STACK_SIZE, PROT_READ | PROT_WRITE | MMAP_USER | MMAP_HUGE);

    return self;
}