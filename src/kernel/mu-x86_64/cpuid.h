#pragma once

#include <pico-misc/types.h>

#define CPUID_EXTENDED_LEAF      (0x80000001)
#define CPUID_EXFEATURE_PDPE1GB  (1 << 26)
#define CPUID_SSE_SUPPORT        (1 << 25)
#define CPUID_SSE2_SUPPORT       (1 << 26)
#define CPUID_XSAVE_SUPPORT      (1 << 26)
#define CPUID_FEATURE_IDENTIFIER (0x1)

typedef struct
{
    u32 eax;
    u32 ebx;
    u32 ecx;
    u32 edx;

    bool success;
} CpuidResult;

CpuidResult cpuid(u32 leaf, u32 subleaf);

bool cpuid_has_1gb_pages(void);
bool cpuid_has_sse(void);
bool cpuid_has_sse2(void);
bool cpuid_has_xsave(void);