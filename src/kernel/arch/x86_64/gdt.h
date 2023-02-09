#pragma once

#include <base/macro.h>
#include <stdint.h>

#define GDT_ACCESS_READ_WRITE (1 << 1)
#define GDT_ACCESS_EXE (1 << 3)
#define GDT_ACCESS_DESCRIPTOR (1 << 4)
#define GDT_ACCESS_USER (3 << 5)
#define GDT_ACCESS_PRESENT (1 << 7)

#define GDT_FLAGS_LONG_MODE (1 << 1)
#define GDT_FLAGS_SIZE (1 << 2)
#define GDT_FLAGS_GRANULARITY (1 << 3)

#define TSS_FLAGS_PRESENT (1 << 7)
#define TSS_FLAGS_64BITS_AVAILABLE (0x9)

enum _GDT_ENTRY
{
    NULL_DESC,
    KERNEL_CODE,
    KERNEL_DATA,
    USER_CODE,
    USER_DATA,

    GDT_ENTRIES_LENGTH
};

typedef struct packed
{
    uint16_t limit_low;
    uint16_t base_low;
    uint8_t base_mid;
    uint8_t access;
    uint8_t limit_high : 4;
    uint8_t flags : 4;
    uint8_t base_high;
} GdtSegment;

typedef struct packed
{
    uint16_t length;
    uint16_t base_low;
    uint8_t base_mid;
    uint8_t flags1;
    uint8_t flags2;
    uint8_t base_high;
    uint32_t base_upper;
    uint32_t reserved;
} GdtTss;

typedef struct packed
{
    uint32_t reserved;
    uint64_t rsp[3];
    uint64_t reserved0;
    uint64_t ist[7];
    uint32_t reserved1;
    uint32_t reserved2;
    uint16_t reserved3;
    uint16_t iopb_offset;
} Tss;

typedef struct packed
{
    uint16_t len;
    uintptr_t offset;
} GdtDesc;

typedef struct packed
{
    GdtSegment entries[GDT_ENTRIES_LENGTH];
    GdtTss tss;
} Gdt;

void gdt_init(void);
void gdt_init_tss(void);
void gdt_load_tss(Tss *self);
void gdt_flush(uintptr_t);
void tss_flush(void);
uintptr_t gdt_descriptor(void);