#pragma once

#include <pico-misc/types.h>

typedef struct packed
{
    u8 blue;
    u8 green;
    u8 red;
    u8 __unused;
} FramebufferPixel;

typedef struct
{
    FramebufferPixel *pixels;
    usize width;
    usize height;
    u16 pitch;
    u16 format;
} Framebuffer;