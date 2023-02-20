#include <debug/debug.h>
#include <munix-hal/hal.h>

#include "elf.h"

int _start()
{
    debug_set_acquire_function(hal_serial_acquire);
    hal_init();
    elf_load_module("/bin/hello-world");
    loop;
}
