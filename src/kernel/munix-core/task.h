#pragma once

#include <ds/vec.h>
#include <ipc/ipc.h>
#include <misc/macro.h>
#include <munix-hal/hal.h>

typedef enum
{
    TASK_READY
} TaskState;

typedef struct
{
    size_t tid;
    char const *path;
    HalCtx context;
    TaskState state;
    IpcBuffer *ipc;
    HalSpace *space;
    uintptr_t stack;
} Task;

Task *task_init(char const *path, HalSpace *space);