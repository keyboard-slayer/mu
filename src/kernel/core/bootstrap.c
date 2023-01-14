#include <abstract/arch.h>
#include <debug/debug.h>

#include "pmm.h"

int _start()
{
    debug_set_acquire_function(abstract_serial_acquire);
    debug(DEBUG_INFO, "Hello, world!");

    pmm_init();
    arch_init();

    for (;;)
        ;
}