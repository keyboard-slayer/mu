#include <mu-api/api.h>
#include <pico-adt/maybe.h>
#include <pico-adt/str.h>
#include <pico-ds/vec.h>
#include <pico-misc/types.h>

typedef struct
{
    Str path;
    MuArgs args;
    MuCap vspace;
} RcEntry;

typedef Vec(RcEntry) Maybe$(RcVec);