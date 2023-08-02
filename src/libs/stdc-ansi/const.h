#pragma once

#define USER_STACK_TOP  (0x7fffffffe000)
#define USER_STACK_SIZE (mib(2))
#define USER_STACK_BASE (USER_STACK_TOP - USER_STACK_SIZE)
#define USER_HEAP_BASE  (0x80000000000)
#define USER_HEAP_SIZE  (gib(4))
#define USER_HEAP_TOP   (USER_HEAP_BASE + USER_HEAP_SIZE)
#define PAGE_SIZE       (kib(4))