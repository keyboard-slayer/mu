#pragma once

#include <stdbool.h>

#define Maybe(T)     \
    struct           \
    {                \
        bool isJust; \
        T value;     \
    }

#define Just(T, x)      \
    (T)                 \
    {                   \
        .isJust = true, \
        .value = x,     \
    }

#define None(T)         \
    (T)                 \
    {                   \
        .isJust = false \
    }

#define Try(T, EXP)         \
    ({                      \
        if (!EXP.isJust)    \
            return None(T); \
        EXP.value;          \
    })

#define unwrap(EXP)                                  \
    ({                                               \
        if (!EXP.isJust)                             \
            panic("Couldn't unwrap value of " #EXP); \
        EXP.value;                                   \
    })

#define unwrap_or(EXP, DEFAULT) \
    ({                          \
        if (!EXP.isJust)        \
            DEFAULT;            \
        EXP.value;              \
    })

#define Maybe$(N)                        \
    N##_;                                \
    typedef Maybe(N##_) Maybe##N;        \
    typedef Maybe(N##_ *) Maybe##N##Ptr; \
    typedef N##_ N;

typedef Maybe(void *) MaybePtr;
