#pragma once

#include <stdint.h>

#define Maybe(T)     \
    struct           \
    {                \
        bool isSome; \
        T value;     \
    }

#define Some(T, x)      \
    (T)                 \
    {                   \
        .isSome = true, \
        .value = x,     \
    }

#define None(T)         \
    (T)                 \
    {                   \
        .isSome = false \
    }

#define Try(T, EXPR)                  \
    ({                                \
        typeof(EXPR) __expr = (EXPR); \
        if (!__expr.isSome)           \
            return None(T);           \
        __expr.value;                 \
    })

#define unwrap(EXPR)                                  \
    ({                                                \
        typeof(EXPR) __expr = (EXPR);                 \
        if (!__expr.isSome)                           \
            panic("Couldn't unwrap value of " #EXPR); \
        __expr.value;                                 \
    })

#define unwrap_or(EXPR, DEFAULT)      \
    ({                                \
        typeof(EXPR) __expr = (EXPR); \
        if (!__expr.isSome)           \
            DEFAULT;                  \
        __expr.value;                 \
    })

#define Maybe$(N)                        \
    N##_;                                \
    typedef Maybe(N##_) Maybe##N;        \
    typedef Maybe(N##_ *) Maybe##N##Ptr; \
    typedef N##_ N;

typedef Maybe(void *) MaybePtr;
typedef Maybe(uintptr_t) MayeUint;