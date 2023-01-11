// A purpose for GDT

#pragma once
#include <stdint.h>
#include <macro.h>

typedef struct packed
{
    uint16_t *limitLow;
    uint16_t *baseLow;
    uint8_t *baseMiddle;
    uint8_t *baseHigh;
    uint8_t *granularity;
    uint8_t *access;
    uint8_t *flags;

}GDTEntry;

typedef struct packed
{
    uintptr_t *limit;
    uint16_t *base;
}GDTPtr;


void gdt_init();

void gdt_load();



