#include <mu-base/std.h>
#include <mu-hal/hal.h>

#include "elf.h"

int _start()
{
    debugInfo("Hello from µ !");
    hal_parse_handover();
    hal_init();

    elf_load_module("/bin/bootstrap", (MuArgs){});
    loop;
}
