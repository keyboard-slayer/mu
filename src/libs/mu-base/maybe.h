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

#define Try(T, EXPR)                  \
    ({                                \
        typeof(EXPR) __expr = (EXPR); \
        if (!__expr.isJust)           \
            return None(T);           \
        __expr.value;                 \
    })

#define unwrap(EXPR)                                  \
    ({                                                \
        typeof(EXPR) __expr = (EXPR);                 \
        if (!__expr.isJust)                           \
            panic("Couldn't unwrap value of " #EXPR); \
        __expr.value;                                 \
    })

#define unwrap_or(EXPR, DEFAULT)      \
    ({                                \
        typeof(EXPR) __expr = (EXPR); \
        if (!__expr.isJust)           \
            DEFAULT;                  \
        __expr.value;                 \
    })

#define Maybe$(N)                        \
    N##_;                                \
    typedef Maybe(N##_) Maybe##N;        \
    typedef Maybe(N##_ *) Maybe##N##Ptr; \
    typedef N##_ N;

typedef Maybe(void *) MaybePtr;
