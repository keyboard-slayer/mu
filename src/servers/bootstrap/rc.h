#pragma once

#include <mu-api/api.h>
#include <mu-base/std.h>
#include <mu-ds/vec.h>

typedef struct
{
    MuCap vspace;
    Str path;
    MuArgs args;
} RcEntry;

typedef Vec(RcEntry) Maybe$(RCVec);