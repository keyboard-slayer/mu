#pragma once

#include <mu-base/str.h>
#include <mu-traits/writer.h>

#include "map.h"

typedef enum
{
    FMT_INT,
    FMT_CHAR,
    FMT_STR,
} FmtValueType;

typedef struct
{
    FmtValueType type;

    union
    {
        i64 _int;
        char _char;
        cstr _str;
    };
} FmtValue;

typedef struct
{
    cstr fmt;
    usize size;
    usize offset;
} FmtParser;

typedef struct
{
    usize count;
    FmtValue *values;
} FmtArgs;

[[gnu::used]] static inline FmtValue fmtvali(i64 val)
{
    return (FmtValue){.type = FMT_INT, ._int = val};
}

[[gnu::used]] static inline FmtValue fmtvalcs(cstr val)
{
    return (FmtValue){.type = FMT_STR, ._str = val};
}

[[gnu::used]] static inline FmtValue fmtvalc(char val)
{
    return (FmtValue){.type = FMT_CHAR, ._char = val};
}

[[gnu::used]] static inline FmtValue fmtvals(Str str)
{
    return (FmtValue){.type = FMT_STR, ._str = (cstr)str.buf};
}

// clang-format off
#define SELECT_VALUE(__value) _Generic (                \
    (__value),                                          \
    int: fmtvali,                                       \
    usize: fmtvali,                                     \
    i64: fmtvali,                                       \
    cstr: fmtvalcs,                                     \
    char *: fmtvalcs,                                   \
    char: fmtvalc,                                      \
    Str: fmtvals                                        \
)(__value),

#define PRINT_ARGS_(...)                                                                           \
    (FmtArgs)                                                                                      \
    {                                                                                              \
        0, (FmtValue[]){},                                                                         \
    }

#define PRINT_ARGS_N(...)                                                                          \
    (FmtArgs)                                                                                      \
    {                                                                                              \
        GET_ARG_COUNT(__VA_ARGS__), (FmtValue[]){MAP(SELECT_VALUE, __VA_ARGS__)},                  \
    }

#define PRINT_ARGS(...) PRINT_ARGS_##__VA_OPT__(N)(__VA_ARGS__)
#define fmt(WRITER, FORMAT, ...) fmt_impl((Writer *) WRITER, FORMAT, PRINT_ARGS(__VA_ARGS__))

// clang-format on

void fmt_impl(Writer *writer, cstr fmt, FmtArgs args);