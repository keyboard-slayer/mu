#pragma once

#include <mu-base/std.h>

#define GDT_ACCESS_READ_WRITE      (1 << 1)
#define GDT_ACCESS_EXE             (1 << 3)
#define GDT_ACCESS_DESCRIPTOR      (1 << 4)
#define GDT_ACCESS_USER            (3 << 5)
#define GDT_ACCESS_PRESENT         (1 << 7)
#define GDT_FLAGS_LONG_MODE        (1 << 1)
#define GDT_FLAGS_SIZE             (1 << 2)
#define GDT_FLAGS_GRANULARITY      (1 << 3)
#define TSS_FLAGS_PRESENT          (1 << 7)
#define TSS_FLAGS_64BITS_AVAILABLE (0x9)

enum
{
    GDT_NULL_DESC,
    GDT_KERNEL_CODE,
    GDT_KERNEL_DATA,
    GDT_USER_DATA,
    GDT_USER_CODE,

    GDT_ENTRIES_LENGTH
};

typedef struct packed
{
    u16 limit_low;
    u16 base_low;
    u8 base_mid;
    u8 access;
    u8 limit_high : 4;
    u8 flags : 4;
    u8 base_high;
} GdtSegment;

typedef struct packed
{
    u16 length;
    u16 base_low;
    u8 base_mid;
    u8 flags1;
    u8 flags2;
    u8 base_high;
    u32 base_upper;
    u32 reserved;
} GdtTss;

typedef struct packed
{
    u32 reserved;
    u64 rsp[3];
    u64 reserved0;
    u64 ist[7];
    u32 reserved1;
    u32 reserved2;
    u16 reserved3;
    u16 iopb_offset;
} Tss;

typedef struct packed
{
    u16 len;
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