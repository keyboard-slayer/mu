#pragma once

#include <string.h>
#include <misc/macro.h>

#define non_null$(x) ({                             \
    if ((uintptr_t)x == 0)                          \
    {                                               \
        panic("Assertion failed (" #x " is null)"); \
    }                                               \
    x;                                              \
})

typedef enum
{
    DEBUG_NONE,
    DEBUG_INFO,
    DEBUG_WARN,
    DEBUG_PANIC,

    DEBUG_EVENT_LENGTH
} DebugEvent;

void __debug_impl(char const *filename, size_t lineno, DebugEvent event, char const *fmt, ...);

#define debug(EVENT, ...) __debug_impl(__FILENAME__, __LINE__, EVENT, __VA_ARGS__);

#define debugInfo(...) debug(DEBUG_INFO, __VA_ARGS__)

#define debugWarn(...) debug(DEBUG_WARN, __VA_ARGS__)

#define panic(...) \
    ({__debug_impl(__FILENAME__, __LINE__, DEBUG_PANIC, __VA_ARGS__); \
    unreachable(); })