#include <mu-api/api.h>
#include <mu-core/const.h>
#include <mu-core/pmm.h>
#include <mu-core/port.h>
#include <mu-core/sched.h>
#include <mu-debug/debug.h>
#include <mu-hal/hal.h>
#include <mu-mem/heap.h>
#include <stdint.h>
#include <string.h>


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
            switch (arg3)
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
                    Pmm pmm = pmm_acquire();
                    auto obj = pmm.malloc(arg2, high);

                    cap->_raw = obj.isSome ? (u64)unwrap(obj).ptr : 0;
                    pmm.release(&pmm);
                    break;
                }
            }

            break;
        }

        case MU_TYPE_VSPACE:
        {
            HalSpace *space;
            MuRes res = hal_space_create(&space);

            if (res != MU_RES_OK)
            {
                return res;
            }

            cap->_raw = (uintptr_t)space;
            break;
        }

        case MU_TYPE_TASK:
        {
            cstr name = (cstr)hal_mmap_lower_to_upper(arg1);
            cap->_raw = (uintptr_t)unwrap_or(task_init(str(name), (HalSpace *)arg2), NULL);

            if (!cap->_raw)
            {
                return MU_RES_NO_MEM;
            }

            break;
        }

        case MU_TYPE_PORT:
        {
            auto heap = heap_acquire();

            cap->_raw = (uintptr_t)unwrap_or(heap.calloc(&heap, 1, sizeof(MuPort)), NULL);
            auto port = (MuPort *)cap->_raw;

            heap.release(&heap);

            port->rights = arg1;
            vec_init(&port->msg, heap_acquire);

            if (!cap->_raw)
            {
                return MU_RES_NO_MEM;
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

static MuRes sys_map(MuCap space, MuCap vmo, uintptr_t virt, uintptr_t off, usize len, MuMapFlags flags)
{
    if (space._raw == 0)
    {
        return MU_RES_BAD_ARG;
    }

    return hal_space_map((HalSpace *)space._raw, virt, vmo._raw + off, len, flags | MU_MEM_USER);
}

static MuRes sys_start(MuCap task, uintptr_t ip, uintptr_t sp, MuArgs *args)
{
    Task *t = (Task *)task._raw;
    hal_ctx_create(&t->context, ip, sp, *args);
    sched_push_task(t);
    return MU_RES_OK;
}


static MuRes sys_ipc(MuCap *port, MuMsg *msg, MuIpcFlags flags)
{
    MuPort *p = (MuPort *)port->_raw;

    if (flags & MU_IPC_SEND)
    {
        MuPort *ret = (MuPort *)msg->reply_port._raw;

        if ((ret->rights & MU_IPC_SEND) == 0)
        {
            return MU_RES_BAD_CAP;
        }

        vec_push(&p->msg, msg);

        return MU_RES_OK;
    }
    else if (flags & MU_IPC_RECV)
    {
        if (p->msg.length == 0)
        {
            if (flags & MU_IPC_BLOCK)
            {
                while (p->msg.length == 0)
                    ;
            }
            else
            {
                return MU_RES_BAD_CAP;
            }
        }

        msg = vec_pop(&p->msg);

        return MU_RES_OK;
    }

    return MU_RES_NON_IMPLEM;
}

static Handler *handlers[__MU_SYS_LEN] = {
    [MU_SYS_LOG] = (Handler *)sys_log,
    [MU_SYS_EXIT] = (Handler *)sys_exit,
    [MU_SYS_SELF] = (Handler *)sys_self,
    [MU_SYS_CREATE] = (Handler *)sys_create,
    [MU_SYS_MAP] = (Handler *)sys_map,
    [MU_SYS_START] = (Handler *)sys_start,
    [MU_SYS_IPC] = (Handler *)sys_ipc,
};

MuRes mu_core_syscall(MuSyscall s, MuArgs args)
{
    if (s >= __MU_SYS_LEN)
    {
        return MU_RES_BAD_SYSCALL;
    }

    return handlers[s](args.arg1, args.arg2, args.arg3, args.arg4, args.arg5, args.arg6);
}