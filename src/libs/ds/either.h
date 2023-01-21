#pragma once

#include <stdbool.h>

#define Either(T, U)  \
    struct            \
    {                 \
        union         \
        {             \
            T left;   \
            U right;  \
        };            \
        bool is_left; \
    }

#define Left(T, v)                  \
    (T)                             \
    {                               \
        .left = v, .is_left = true, \
    }

#define Right(T, v)                   \
    (T)                               \
    {                                 \
        .right = v, .is_left = false, \
    }