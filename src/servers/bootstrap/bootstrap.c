#include <mu-api/api.h>
#include <mu-api/bootstrap.h>

#include "mu-base/fmt.h"

void embed_abort(void)
{
    mu_exit(1);
    for (;;)
        ;
}

int mu_main(MuArgs args)
{
    Module *mods = (Module *)args.arg1;
    usize len = args.arg2;

    debug_info("I love devse");

    if (!len)
    {
        panic("No modules found");
    }

    for (usize i = 0; i < len; i++)
    {
        debug_info("Found {}", mods[i].name);
        if (memcmp(mods[i].name, "/etc/rc.json", 12) == 0)
        {
            debug_info("reading at {a}", mods[i].ptr);
            auto rc = str_n(mods[i].len, mods[i].ptr);
            debug_info("{}", rc);
        }
    }

    return 0;
}