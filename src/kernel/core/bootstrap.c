#include <abstract/arch.h>
#include <debug/debug.h>

#include "elf.h"

int _start()
{
    debug_set_acquire_function(abstract_serial_acquire);
    debug(DEBUG_INFO, "Hello, world!");

    arch_init();

    elf_load_module("/bin/hello-world");
    loop;
}