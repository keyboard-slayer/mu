#pragma once

#include <mu-base/str.h>

#define mu_always_inline static __attribute__((always_inline, used))

/* --- Types ---------------------------------------------------------------- */

typedef enum
{
    MU_TYPE_NONE,

    MU_TYPE_TASK,
    MU_TYPE_VSPACE,
    MU_TYPE_VMO,

    MU_TYPE_EVENT,
    MU_TYPE_IOSPACE,

    MU_TYPE_CNODE,
    MU_TYPE_PORT,
} MuType;

typedef enum
{
    MU_SYS_LOG,
    MU_SYS_START,
    MU_SYS_WAIT,
    MU_SYS_EXIT,
    MU_SYS_SELF,
    MU_SYS_BOOTSTRAP_PORT,

    MU_SYS_MAP,
    MU_SYS_UNMAP,

    MU_SYS_CREATE,
    MU_SYS_CLOSE,
    MU_SYS_DUP,

    MU_SYS_IPC,

    MU_SYS_BIND,
    MU_SYS_UNBIND,
    MU_SYS_ACK,

    MU_SYS_IN,
    MU_SYS_OUT,

    __MU_SYS_LEN,
} MuSyscall;

typedef enum
{
    MU_EV_IRQ,
} MuEvent;

typedef enum
{
    MU_RES_OK,
    MU_RES_BAD_CAP,
    MU_RES_BAD_SYSCALL,
    MU_RES_NO_MEM,
    MU_RES_NON_ALIGN,
    MU_RES_NON_IMPLEM,
    MU_RES_BAD_ARG,

    __MU_RES_LEN,
} MuRes;

typedef uintptr_t MuArg;

typedef struct MuArgs
{
    MuArg arg1;
    MuArg arg2;
    MuArg arg3;
    MuArg arg4;
    MuArg arg5;
    MuArg arg6;
} MuArgs;

typedef struct
{
    u64 _raw;
} Maybe$(MuCap);

typedef struct
{
    MuArg label;
    MuCap reply_port;
    MuArgs args;
} MuMsg;

typedef struct
{
    usize tid;
    MuCap space;
} MuTask;

typedef enum
{
    MU_MEM_NONE = 0,
    MU_MEM_READ = 1 << 0,
    MU_MEM_WRITE = 1 << 1,
    MU_MEM_EXEC = 1 << 2,
    MU_MEM_USER = 1 << 3,
    MU_MEM_HUGE = 1 << 4,
} MuMapFlags;

typedef enum
{
    MU_MEM_LOW = 1 << 0,
    MU_MEM_HIGH = 1 << 1,
    MU_MEM_DMA = 1 << 2,
} MuMemFlags;

typedef enum
{
    MU_IPC_BLOCK = 1 << 0,
    MU_IPC_SEND = 1 << 1,
    MU_IPC_RECV = 1 << 2,
} MuIpcFlags;

typedef enum
{
    MU_IO_8 = 1 << 0,
    MU_IO_16 = 1 << 1,
    MU_IO_32 = 1 << 2,
    MU_IO_64 = 1 << 3,
} MuIoFlags;

/* --- IPC ------------------------------------------------------------------ */

#define mu_msg(label, port, ...) __mu_msg(label, port, __VA_ARGS__, 0, 0, 0, 0, 0, 0)

#define __mu_msg(l, p, a1, a2, a3, a4, a5, a6, ...) \
    (MuMsg)                                         \
    {                                               \
        .label = (MuArg)l,                          \
        .reply_port = (MuCap)p,                     \
        .args = {                                   \
            .arg1 = (MuArg)a1,                      \
            .arg2 = (MuArg)a2,                      \
            .arg3 = (MuArg)a3,                      \
            .arg4 = (MuArg)a4,                      \
            .arg5 = (MuArg)a5,                      \
            .arg6 = (MuArg)a6,                      \
        },                                          \
    }

/* --- Syscall -------------------------------------------------------------- */

#ifdef __osdk_arch_x86_64__

mu_always_inline MuRes __mu_syscall_impl(MuSyscall s, MuArg arg1, MuArg arg2, MuArg arg3, MuArg arg4, MuArg arg5, MuArg arg6)
{
    MuRes res;

    // s : rax, arg1 : rdi, arg2 : rsi, arg3 : rdx, arg4 : r10, arg5 : r8, arg6 : r9
    __asm__ volatile(
        "syscall"
        : "=a"(res)
        : "a"(s), "D"(arg1), "S"(arg2), "d"(arg3), "r"(arg4), "r"(arg5), "r"(arg6)
        : "rcx", "r11", "memory");

    return res;
}

#else
#    error "Unsupported architecture"
#endif

#define __mu_syscall(id, a1, a2, a3, a4, a5, a6, ...) __mu_syscall_impl(id, a1, a2, a3, a4, a5, a6)

#define mu_syscall(...) __mu_syscall(__VA_ARGS__, 0, 0, 0, 0, 0, 0)

/* --- Syscalls -------------------------------------------------------------- */

mu_always_inline MuRes mu_log(cstr str, usize len)
{
    return mu_syscall(MU_SYS_LOG, (MuArg)str, (MuArg)len);
}

mu_always_inline MuRes mu_start(MuCap task, uintptr_t ip, uintptr_t sp, MuArgs args)
{
    return mu_syscall(MU_SYS_START, task._raw, ip, sp, (MuArg)&args);
}

mu_always_inline MuRes mu_wait(MuCap task, MuArg *res)
{
    return mu_syscall(MU_SYS_WAIT, task._raw, (MuArg)res);
}

mu_always_inline MuRes mu_exit(MuArg res)
{
    return mu_syscall(MU_SYS_EXIT, res);
}

mu_always_inline MuRes mu_panic(cstr str, usize len)
{
    mu_log(str, len);
    return mu_exit(1);
}

mu_always_inline MuRes mu_map(MuCap space, MuCap vmo, uintptr_t virt, usize off, usize len, MuMapFlags flags)
{
    return mu_syscall(MU_SYS_MAP, space._raw, vmo._raw, virt, off, len, (MuArg)flags);
}

mu_always_inline MuRes mu_unmap(MuCap space, uintptr_t virt, usize len)
{
    return mu_syscall(MU_SYS_UNMAP, space._raw, virt, len);
}

mu_always_inline MuRes mu_create(MuType type, MuCap *cap, MuArg arg1, MuArg arg2, MuArg arg3, MuArg arg4)
{
    return mu_syscall(MU_SYS_CREATE, (MuArg)type, (MuArg)cap, arg1, arg2, arg3, arg4);
}

mu_always_inline MuRes mu_create_task(MuCap *cap, MuCap name, MuCap vspace)
{
    return mu_syscall(MU_SYS_CREATE, (MuArg)MU_TYPE_TASK, (MuArg)cap, name._raw, vspace._raw);
}

mu_always_inline MuRes mu_create_vspace(MuCap *cap)
{
    return mu_syscall(MU_SYS_CREATE, (MuArg)MU_TYPE_VSPACE, (MuArg)cap);
}

mu_always_inline MuRes mu_create_vmo(MuCap *cap, uintptr_t phys, usize size, MuMemFlags flags)
{
    return mu_syscall(MU_SYS_CREATE, (MuArg)MU_TYPE_VMO, (MuArg)cap, phys, size, (MuArg)flags);
}

mu_always_inline MuRes mu_create_event(MuCap *cap, MuEvent type, MuArg sel)
{
    return mu_syscall(MU_SYS_CREATE, (MuArg)MU_TYPE_EVENT, (MuArg)cap, (MuArg)type, sel);
}

mu_always_inline MuRes mu_create_iospace(MuCap *cap, uintptr_t base, usize len)
{
    return mu_syscall(MU_SYS_CREATE, (MuArg)MU_TYPE_IOSPACE, (MuArg)cap, base, len);
}

mu_always_inline MuRes mu_create_cnode(MuCap *cap, usize len)
{
    return mu_syscall(MU_SYS_CREATE, (MuArg)MU_TYPE_CNODE, (MuArg)cap, len);
}

mu_always_inline MuRes mu_create_port(MuCap *cap, MuIpcFlags right)
{
    return mu_syscall(MU_SYS_CREATE, (MuArg)MU_TYPE_PORT, (MuArg)cap, right);
}

mu_always_inline MuRes mu_close(MuCap cap)
{
    return mu_syscall(MU_SYS_CLOSE, cap._raw);
}

mu_always_inline MuRes mu_dup(MuCap cap, MuCap *new_cap)
{
    return mu_syscall(MU_SYS_DUP, cap._raw, (MuArg)new_cap);
}

mu_always_inline MuRes mu_ipc(MuCap *port, MuMsg *msg, MuIpcFlags flags)
{
    return mu_syscall(MU_SYS_IPC, (MuArg)port, (MuArg)msg, (MuArg)flags);
}

mu_always_inline MuRes mu_call(MuCap port, MuMsg *msg)
{
    return mu_ipc(&port, msg, MU_IPC_SEND | MU_IPC_RECV);
}

mu_always_inline MuRes mu_bind(MuCap event, MuCap port, MuArg sel)
{
    return mu_syscall(MU_SYS_BIND, event._raw, port._raw, sel);
}

mu_always_inline MuRes mu_unbind(MuCap event, MuArg sel)
{
    return mu_syscall(MU_SYS_UNBIND, event._raw, (MuArg)sel);
}

mu_always_inline MuRes mu_ack(MuCap event, MuArg sel)
{
    return mu_syscall(MU_SYS_ACK, event._raw, (MuArg)sel);
}

mu_always_inline MuRes mu_in(MuCap iospace, uintptr_t port, uintptr_t *val, MuIoFlags flags)
{
    return mu_syscall(MU_SYS_IN, iospace._raw, port, (MuArg)val, (MuArg)flags);
}

mu_always_inline MuRes mu_out(MuCap iospace, uintptr_t port, uintptr_t val, MuIoFlags flags)
{
    return mu_syscall(MU_SYS_OUT, iospace._raw, port, val, (MuArg)flags);
}

mu_always_inline MuRes mu_self(MuCap *cap)
{
    return mu_syscall(MU_SYS_SELF, (MuArg)cap);
}

mu_always_inline MuRes mu_bootstrap_port(MuCap *cap)
{
    return mu_syscall(MU_SYS_BOOTSTRAP_PORT, (MuArg)cap);
}