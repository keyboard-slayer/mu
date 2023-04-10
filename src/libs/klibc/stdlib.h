#pragma once

#include <mu-base/debug.h>

static inline void abort(char const *msg)
{
    panic(msg);
}