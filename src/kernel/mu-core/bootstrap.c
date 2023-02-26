#include <debug/debug.h>
#include <mu-hal/hal.h>

#include "elf.h"

int _start()
{
    debug(DEBUG_INFO, "Hello from µ !");
    hal_parse_handover();
    hal_init();

    elf_load_module("/bin/hello-world", (MuArgs){.arg1 = 0xb00b1e5});
    loop;
}
