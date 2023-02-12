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
    char const *path;
    Context context;
    TaskState state;
    Space space;
    uintptr_t stack;
} Task;

Task *task_init(char const *path, uintptr_t *space);
typedef Vec(Task *) VecTask;