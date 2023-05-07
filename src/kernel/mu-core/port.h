#pragma once

#include <mu-ds/vec.h>

typedef Vec(MuMsg *) MuMsgVec;

typedef struct
{
    MuMsgVec msg;
    u8 rights;
} MuPort;