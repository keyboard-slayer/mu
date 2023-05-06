#include <mu-base/std.h>

int mu_main(MuArgs args)
{
    debug_info("The value you passed is {x}", args.arg1);
    return 0;
}