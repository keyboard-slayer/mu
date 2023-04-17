#include <mu-api/api.h>
#include <mu-base/std.h>
#include <mu-core/sched.h>
#include <mu-hal/hal.h>

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
    unused MuTask *self = (MuTask *)cap;
    auto sched = sched_self();
    Task *task = sched->tasks.data[sched->task_index];

    self->path = task->path;
    self->space = (MuCap){._raw = (usize)task->space};
    self->tid = task->tid;

    return MU_RES_OK;
}

static Handler *handlers[__MU_SYS_LEN] = {
    [MU_SYS_LOG] = (Handler *)sys_log,
    [MU_SYS_EXIT] = (Handler *)sys_exit,
    [MU_SYS_SELF] = (Handler *)sys_self

};

MuRes mu_core_syscall(MuSyscall s, MuArgs args)
{
    if (s >= __MU_SYS_LEN)
    {
        return MU_RES_BAD_SYSCALL;
    }

    return handlers[s](args.arg1, args.arg2, args.arg3, args.arg4, args.arg5, args.arg6);
}