#pragma once

#include <stdbool.h>
#include <stdint.h>

#define CPUID_EXTENDED_LEAF      (0x80000001)
#define CPUID_EXFEATURE_PDPE1GB  (1 << 26)
#define CPUID_SSE_SUPPORT        (1 << 25)
#define CPUID_SSE2_SUPPORT       (1 << 26)
#define CPUID_XSAVE_SUPPORT      (1 << 26)
#define CPUID_FEATURE_IDENTIFIER (0x1)

typedef struct
{
    uint32_t eax;
    uint32_t ebx;
    uint32_t ecx;
    uint32_t edx;

    bool success;
} CpuidResult;

CpuidResult cpuid(uint32_t leaf, uint32_t subleaf);

bool cpuid_has_1gb_pages(void);
bool cpuid_has_sse(void);
bool cpuid_has_sse2(void);
bool cpuid_has_xsave(void);