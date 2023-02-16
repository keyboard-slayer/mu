#pragma once

#include <misc/lock.h>
#include <misc/macro.h>
#include <stddef.h>
#include <stdint.h>

#define IPC_STRUCT_POS  (0xc1000000)
#define IPC_BUFFER_SIZE (kib(4))

typedef struct
{
    uint8_t data[IPC_BUFFER_SIZE];
    size_t size;
} IpcBuffer;

IpcBuffer *ipc_buffer_get(void);