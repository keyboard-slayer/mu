#pragma once

#include <mu-base/std.h>
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
    Str path;
    HalCtx context;
    TaskState state;
    HalSpace *space;
    uintptr_t stack;
} Maybe$(Task);

MaybeTaskPtr task_init(Str path, HalSpace *space);
MaybeTaskPtr task_kernel(void);