/*
MIT License

Copyright (c) 2021-2022 The brutal operating system contributors

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#pragma once

#include "macro.h"
#include "maybe.h"
#include "types.h"

/* --- Required Macros ------------------------------------------------------ */

#define _MatchConst(T, EXPR) \
    T:                       \
    EXPR,                    \
        const T : EXPR

/* --- Null Terminated String ----------------------------------------------- */

static inline size_t cstr_len(u8 const *str)
{
    usize size = 0;

    while (str[size])
    {
        size++;
    }

    return size;
}

/* --- Inline String Buffer ------------------------------------------------- */

typedef struct
{
    usize len;
    u8 buf[];
} InlineStr;

/* --- Non Owning Strings --------------------------------------------------- */

typedef struct
{
    usize len;
    u8 const *buf;
} Maybe$(Str);

#define nullstr str_const$("")

static inline bool str_empty(Str const str)
{
    return str.len == 0;
}

static inline bool str_any(Str const str)
{
    return str.len != 0;
}

[[gnu::used]] static inline bool str_eq(Str const lhs, Str const rhs)
{
    if (lhs.len != rhs.len)
    {
        return false;
    }

    for (usize i = 0; i < lhs.len; i++)
    {
        if (lhs.buf[i] != rhs.buf[i])
        {
            return false;
        }
    }

    return true;
}

/* --- Fix Size Strings ----------------------------------------------------- */

#define StrFix(N)  \
    struct         \
    {              \
        usize len; \
        u8 buf[N]; \
    }

typedef StrFix(8) StrFix8;
typedef StrFix(16) StrFix16;
typedef StrFix(32) StrFix32;
typedef StrFix(64) StrFix64;
typedef StrFix(128) StrFix128;

/* --- Cast Between String Types -------------------------------------------- */

static inline Str str_forward(Str str) { return str; }
static inline Str str_make_from_inline_str(InlineStr *str) { return (Str){str->len, str->buf}; }
static inline Str str_make_from_cstr(char const *cstr) { return (Str){cstr_len((u8 const *)cstr), (u8 *)cstr}; }
static inline Str str_make_from_cstr8(char const *cstr) { return (Str){cstr_len((u8 const *)cstr), (u8 *)cstr}; }
static inline Str str_make_from_str_fix8(StrFix8 const *str_fix) { return (Str){str_fix->len, (u8 *)str_fix->buf}; }
static inline Str str_make_from_str_fix16(StrFix16 const *str_fix) { return (Str){str_fix->len, (u8 *)str_fix->buf}; }
static inline Str str_make_from_str_fix32(StrFix32 const *str_fix) { return (Str){str_fix->len, (u8 *)str_fix->buf}; }
static inline Str str_make_from_str_fix64(StrFix64 const *str_fix) { return (Str){str_fix->len, (u8 *)str_fix->buf}; }
static inline Str str_make_from_str_fix128(StrFix128 const *str_fix) { return (Str){str_fix->len, (u8 *)str_fix->buf}; }

// clang-format off

// Create a new instance of a non owning string.
#define str$(EXPR)                                          \
    _Generic((EXPR),                                        \
        Str              : str_forward,                     \
        InlineStr *      : str_make_from_inline_str,        \
        _MatchConst(char *, str_make_from_cstr),            \
        _MatchConst(u8 *, str_make_from_cstr8),        \
        _MatchConst(StrFix8 *, str_make_from_str_fix8),     \
        _MatchConst(StrFix16 *, str_make_from_str_fix16),   \
        _MatchConst(StrFix32 *, str_make_from_str_fix32),   \
        _MatchConst(StrFix64 *, str_make_from_str_fix64),   \
        _MatchConst(StrFix128 *, str_make_from_str_fix128)  \
    )(EXPR)

#define str_const$(STR)  (Str const){sizeof(STR) - 1, ((uint8_t const*)(STR))}

// clang-format on

#define str_n$(n, str)         \
    (Str)                      \
    {                          \
        (n), (u8 const *)(str) \
    }

// Create a new instance of a fix size string.
#define str_fix$(T, STR) (                             \
    {                                                  \
        T dst_str = {};                                \
        Str src_str = str$(STR);                       \
        memcpy(dst_str.buf, src_str.buf, src_str.len); \
        dst_str.len = src_str.len;                     \
        dst_str;                                       \
    })

#define str_sub(str, start, end) \
    str_n$((end) - (start), (u8 const *)str.buf + (start))