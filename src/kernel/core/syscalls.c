#include <debug/debug.h>
#include <munix-api/api.h>

typedef MuRes Handler(MuArg arg1, MuArg arg2, MuArg arg3, MuArg arg4, MuArg arg5, MuArg arg6);

static MuRes sys_log(char const *str, size_t len)
{
    (void)len; // FIXME: Use len
    debug(DEBUG_INFO, "log: %s", str);
    return MU_RES_OK;
}

static Handler *handlers[__MU_SYS_LEN] = {
    [MU_SYS_LOG] = (Handler *)sys_log,
};

MuRes mu_core_syscall(MuSyscall s, MuArgs args)
{
    if (s >= __MU_SYS_LEN)
    {
        return MU_RES_BAD_SYSCALL;
    }

    return handlers[s](args.arg1, args.arg2, args.arg3, args.arg4, args.arg5, args.arg6);
}