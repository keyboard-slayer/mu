#pragma once

#define STB_SPRINTF_NOFLOAT
#define STB_SPRINTF_IMPLEMENTATION
#define STB_SPRINTF_NOUNALIGNED

#include <misc/macro.h>
#include <traits/output.h>

#define debug(EVENT, ...) __debug_impl(__FILENAME__, __LINE__, EVENT, __VA_ARGS__);

#define non_null$(x) ({                                          \
    if ((uintptr_t)x == 0)                                       \
    {                                                            \
        debug(DEBUG_ERROR, "Assertion failed (" #x " is null)"); \
        debug_raise_exception();                                 \
    }                                                            \
    x;                                                           \
})

typedef enum
{
    DEBUG_NONE,
    DEBUG_INFO,
    DEBUG_WARN,
    DEBUG_ERROR,
    DEBUG_OK,

    DEBUG_EVENT_LENGTH
} DebugEvent;

void debug_raise_exception(void);
void debug_set_acquire_function(AcquireOutput func);
void __debug_impl(char const *filename, size_t lineno, DebugEvent event, char const *fmt, ...);