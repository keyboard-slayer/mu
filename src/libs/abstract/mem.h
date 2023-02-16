#pragma once

#include <stddef.h>
#include <stdint.h>

#include "arch.h"

#define PROT_NONE  (1 << 0)
#define PROT_READ  (1 << 1)
#define PROT_WRITE (1 << 2)
#define PROT_EXEC  (1 << 3)
#define MMAP_HUGE  (1 << 4)
#define MMAP_USER  (1 << 5)
#define MMAP_DEBUG (1 << 6)

int kmmap(Space space, uintptr_t virt, uintptr_t phys, size_t length, uint8_t flags);
Space abstract_create_space(void);
Space abstract_get_kernel_space(void);