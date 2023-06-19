#pragma once

#include <mu-api/api.h>
#include <pico-ds/vec.h>

typedef Vec(MuMsg *) MuMsgVec;

typedef struct
{
    MuMsgVec msg;
    u8 rights;
} MuPort;