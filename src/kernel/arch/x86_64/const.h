#pragma once

#include <misc/macro.h>

#define KERNEL_STACK_SIZE (0x1000)
#define PAGE_SIZE         (kib(4))
#define STACK_SIZE        (0x4000)
#define USER_STACK_BASE   (0xc0000000)