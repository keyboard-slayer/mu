#include <mu-api/api.h>
#include <mu-base/std.h>
#include <mu-core/const.h>
#include <mu-core/pmm.h>
#include <mu-core/sched.h>
#include <mu-hal/hal.h>
#include <mu-mem/heap.h>

typedef MuRes Handler(MuArg arg1, MuArg arg2, MuArg arg3, MuArg arg4, MuArg arg5, MuArg arg6);

static MuRes sys_log(cstr str, usize len)
{
    auto writer = hal_acquire_serial();
    for (usize i = 0; i < len; i++)
    {
        writer.putc(&writer, str[i]);
    }

    writer.release(&writer);

    return MU_RES_OK;
}

static MuRes sys_exit(unused int status)
{
    for (;;)
        ;
    return MU_RES_OK;
}

static MuRes sys_self(MuCap *cap)
{
    Alloc heap = heap_acquire();
    MuTask *self = unwrap_or(heap.calloc(&heap, 1, sizeof(MuTask)), NULL);
    heap.release(&heap);

    if (self == NULL)
    {
        return MU_RES_NO_MEM;
    }

    auto sched = sched_self();
    Task *task = sched->tasks.data[sched->task_index];

    self->space = (MuCap){._raw = (usize)task->space};
    self->tid = task->tid;

    cap->_raw = (uintptr_t)hal_mmap_upper_to_lower((uintptr_t)self);
    hal_space_map(task->space, align_down(cap->_raw, PAGE_SIZE), align_down(cap->_raw, PAGE_SIZE), align_up(sizeof(MuTask), PAGE_SIZE), MU_MEM_USER | MU_MEM_READ);

    return MU_RES_OK;
}

static MuRes sys_create(MuType type, unused MuCap *cap, unused MuArg arg1, unused MuArg arg2, MuArg arg3, unused MuArg arg4)
{
    bool high = arg3 & MU_MEM_HIGH;

    switch (type)
    {
    case MU_TYPE_VMO:
    {
        debug_info("{}", arg4);
        switch (arg4)
        {
        case MU_MEM_DMA:
        {
            return MU_RES_NON_IMPLEM;
            break;
        }
        case MU_MEM_LOW:
            [[fallthrough]];
        case MU_MEM_HIGH:
        {
            cap->_raw = (uintptr_t)unwrap_or(pmm_alloc_page(arg3, high), NULL);
            break;
        }
        }

        break;
    }

    default:
    {
        return MU_RES_NON_IMPLEM;
    }
    }

    if (!cap->_raw)
    {
        return MU_RES_NO_MEM;
    }

    return MU_RES_OK;
}

static Handler *handlers[__MU_SYS_LEN] = {
    [MU_SYS_LOG] = (Handler *)sys_log,
    [MU_SYS_EXIT] = (Handler *)sys_exit,
    [MU_SYS_SELF] = (Handler *)sys_self,
    [MU_SYS_CREATE] = (Handler *)sys_create,
};

MuRes mu_core_syscall(MuSyscall s, MuArgs args)
{
    if (s >= __MU_SYS_LEN)
    {
        return MU_RES_BAD_SYSCALL;
    }

    return handlers[s](args.arg1, args.arg2, args.arg3, args.arg4, args.arg5, args.arg6);
}