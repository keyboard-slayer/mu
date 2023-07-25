#include <pico-misc/macro.h>
#include <munix-debug/debug.h>
#include <unistd.h>

noreturn int mu_main(MuArgs args)
{
    unused MuCap bootstrap = {._raw = args.arg1 };
    debug_info("Hello, World !");
    loop;
    unreachable();
}