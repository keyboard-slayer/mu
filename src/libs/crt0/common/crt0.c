#include <pico-misc/macro.h>
#include <pico-misc/types.h>
#include <mu-api/api.h>

weak int mu_main(unused MuArgs const args)
{
    return 0;
}

void _entry(u64 arg1, u64 arg2, u64 arg3, u64 arg4, u64 arg5, u64 arg6)
{
    MuArgs args = {
        .arg1 = arg1,
        .arg2 = arg2,
        .arg3 = arg3,
        .arg4 = arg4,
        .arg5 = arg5,
        .arg6 = arg6,
    };

    mu_exit(mu_main(args));
    unreachable();
}