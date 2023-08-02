#include <const.h>
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

#include "tiny-vmem/vmem.h"

typedef MuRes Handler(MuArg arg1, MuArg arg2, MuArg arg3, MuArg arg4, MuArg arg5, MuArg arg6);

static bool is_user_stack_address(uintptr_t addr)
{
    return addr >= USER_STACK_BASE && addr < USER_STACK_TOP;
}

static bool is_user_heap_address(uintptr_t addr)
{
    return addr >= USER_HEAP_BASE && addr < USER_HEAP_TOP;
}

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
                    cleanup(pmm_release) Pmm pmm = pmm_acquire();
                    auto obj = pmm.malloc(arg2, high);

                    if (!obj.isSome)
                    {
                        debug_warn("Cannot allocate memory");
                        return MU_RES_NO_MEM;
                    }

                    cap->_raw = (u64)unwrap(obj).ptr;
                    break;
                }

                default:
                {
                    debug_warn("Invalid memory type");
                    return MU_RES_BAD_ARG;
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
            auto sched = sched_self();
            Task const *task = sched->tasks.data[sched->task_index];

            debug_info("arg1: {x}", arg1);

            if (is_user_heap_address(arg1))
            {
                uintptr_t paddr;

                if (hal_space_virt2phys(task->space, arg1, &paddr) != MU_RES_OK)
                {
                    debug_warn("Cannot resolve virtual address to map to");
                    return MU_RES_BAD_ARG;
                }

                if (paddr == 0)
                {
                    debug_warn("Task name cannot be NULL");
                    return MU_RES_BAD_ARG;
                }

                arg1 = (MuArg)hal_mmap_lower_to_upper(paddr);
            }
            else if (is_user_stack_address(arg1))
            {
                if (arg1 == 0)
                {
                    return MU_RES_BAD_ARG;
                }
            }
            else
            {
                return MU_RES_BAD_ARG;
            }

            debug_info("Creating task with name {}", (cstr)arg1);
            cap->_raw = (uintptr_t)unwrap_or(task_init(str((cstr)arg1), (HalSpace *)arg2), NULL);

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
        debug_warn("Cannot create object");
        return MU_RES_NO_MEM;
    }

    return MU_RES_OK;
}

static MuRes sys_map(MuCap space, MuCap vmo, uintptr_t *virt, uintptr_t off, usize len, MuMapFlags flags)
{
    if (space._raw == 0)
    {
        return MU_RES_BAD_ARG;
    }

    if (is_user_heap_address((uintptr_t)virt))
    {
        uintptr_t paddr;
        if (hal_space_virt2phys((HalSpace *)space._raw, (uintptr_t)virt, &paddr) != MU_RES_OK)
        {
            debug_warn("Cannot resolve virtual address to map to");
            return MU_RES_BAD_ARG;
        }

        virt = (uintptr_t *)hal_mmap_upper_to_lower(paddr);
    }
    else if (!is_user_stack_address((uintptr_t)virt))
    {
        debug_warn("Cannot resolve virtual address to map to");
        return MU_RES_BAD_ARG;
    }

    if (!(flags & MU_MEM_NO_ALLOC))
    {
        auto sched = sched_self();
        Task *task = sched->tasks.data[sched->task_index];

        *virt = (uintptr_t)vmem_alloc(&task->vmem, len, VM_INSTANTFIT);

        if (virt == NULL)
        {
            debug_warn("Failed to allocate virtual memory for mapping");
            return MU_RES_NO_MEM;
        }
    }

    flags = flags & ~MU_MEM_NO_ALLOC;
    return hal_space_map((HalSpace *)space._raw, *virt, align_down(vmo._raw + off, PAGE_SIZE),
                         align_up(len, PAGE_SIZE), flags | MU_MEM_USER);
}

static MuRes sys_start(MuCap task, uintptr_t ip, uintptr_t sp, MuArgs *args)
{
    Task *t = (Task *)task._raw;
    hal_ctx_create(&t->context, ip, sp, *args);
    sched_push_task(t);
    return MU_RES_OK;
}

static MuRes sys_ipc(MuCap *port, MuMsg *msg, MuMsgFlags flags)
{
    MuPort *p = (MuPort *)port->_raw;
    auto sched = sched_self();
    Task *task = sched->tasks.data[sched->task_index];

    if (flags & MU_MSG_SEND)
    {
        MuPort *ret = (MuPort *)msg->reply_port._raw;

        if ((ret->rights & MU_PORT_SEND) == 0)
        {
            return MU_RES_BAD_CAP;
        }

        if (is_user_heap_address((uintptr_t)msg))
        {
            uintptr_t phys;

            if (hal_space_virt2phys(task->space, (uintptr_t)msg, &phys) != MU_RES_OK)
            {
                debug_warn("Kernel wasn't able to transfer IPC");
                return MU_RES_BAD_ARG;
            }

            vec_push(&p->msg, (MuMsg *)phys);
        }
        else
        {
            // Can you do that ?
            return MU_RES_BAD_ARG;
        }

        return MU_RES_OK;
    }
    else if (flags & MU_MSG_RECV)
    {
        if ((p->rights & MU_PORT_RECV) == 0)
        {
            return MU_RES_BAD_CAP;
        }

        if (p->msg.length == 0)
        {
            if (flags & MU_MSG_BLOCK)
            {
                while (p->msg.length == 0)
                    ;
            }
            else
            {
                return MU_RES_BAD_CAP;
            }
        }

        MuMsg *tmp = vec_pop(&p->msg);

        if (is_user_heap_address((uintptr_t)msg))
        {
            // FIXME
            uintptr_t phys;
            hal_space_virt2phys(task->space, (uintptr_t)msg, &phys);
            memcpy((void *)hal_mmap_lower_to_upper(phys), (void *)hal_mmap_lower_to_upper((uintptr_t)tmp), sizeof(MuMsg));

            debug_info("OK");
        }

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