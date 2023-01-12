#include <abstract/arch.h>

#include "gdt.h"

void abstract_arch_init(void)
{
    gdt_init();
}