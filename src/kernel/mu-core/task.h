#pragma once

#include <mu-hal/hal.h>
#include <pico-adt/str.h>
#include <pico-ds/vec.h>
#include <tiny-vmem/vmem.h>

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
    Vmem vmem;
} Maybe$(Task);

MaybeTaskPtr task_init(Str path, HalSpace *space);

MaybeTaskPtr task_kernel(void);