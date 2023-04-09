#include <mu-api/api.h>
#include <mu-base/std.h>
#include <mu-core/heap.h>
#include <mu-hal/hal.h>

typedef MuRes Handler(MuArg arg1, MuArg arg2, MuArg arg3, MuArg arg4, MuArg arg5, MuArg arg6);

static MuRes sys_log(char const *str, usize len)
{
    for (usize i = 0; i < len; i++)
    {
        hal_serial_write(str + i, 1);
    }

    return MU_RES_OK;
}

static MuRes sys_exit(unused int status)
{
    for (;;)
        ;
    return MU_RES_OK;
}

static Handler *handlers[__MU_SYS_LEN] = {
    [MU_SYS_LOG] = (Handler *)sys_log,
    [MU_SYS_EXIT] = (Handler *)sys_exit};

MuRes mu_core_syscall(MuSyscall s, MuArgs args)
{
    if (s >= __MU_SYS_LEN)
    {
        return MU_RES_BAD_SYSCALL;
    }

    return handlers[s](args.arg1, args.arg2, args.arg3, args.arg4, args.arg5, args.arg6);
}