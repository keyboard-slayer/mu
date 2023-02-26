#include <misc/macro.h>
#include <munix-api/api.h>
#include <stdint.h>

weak int mu_main(unused MuArgs const args)
{
    return 0;
}

noreturn void _entry(uint64_t arg1, uint64_t arg2, uint64_t arg3, uint64_t arg4, uint64_t arg5, uint64_t arg6)
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
    unreachable;
}