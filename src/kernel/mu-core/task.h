#pragma once

#include <mu-ds/vec.h>
#include <mu-hal/hal.h>

#include <mu-x86_64/ctx.h>

typedef enum
{
    TASK_READY
} TaskState;

typedef struct
{
    usize tid;
    char const *path;
    HalCtx context;
    TaskState state;
    HalSpace *space;
    uintptr_t stack;
} Task;

Task *task_init(char const *path, HalSpace *space);
Task *task_kernel(void);