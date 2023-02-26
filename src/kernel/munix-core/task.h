#pragma once

#include <ds/vec.h>
#include <misc/macro.h>
#include <munix-hal/hal.h>

#include <munix-x86_64/ctx.h>

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
    HalSpace *space;
    uintptr_t stack;
} Task;

Task *task_init(char const *path, HalSpace *space);
Task *task_kernel(void);