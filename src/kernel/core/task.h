#pragma once

#include <abstract/arch.h>
#include <abstract/ctx.h>
#include <ds/vec.h>

typedef enum
{
    TASK_READY
} TaskState;

typedef struct
{
    size_t tid;
    Context context;
    TaskState state;
    Space space;
} Task;

typedef Vec(Task *) VecTask;