#include "cpuid.h"

CpuidResult cpuid(uint32_t leaf, uint32_t subleaf)
{
    uint32_t cpuid_max;
    CpuidResult result;

    __asm__ volatile(
        "cpuid"
        : "=a"(cpuid_max)
        : "a"(leaf & 0x80000000)
        : "ebx", "ecx", "edx");

    if (leaf > cpuid_max)
    {
        return (CpuidResult){.success = false};
    }

    __asm__ volatile(
        "cpuid"
        : "=a"(result.eax), "=b"(result.ebx), "=c"(result.ecx), "=d"(result.edx)
        : "a"(leaf), "c"(subleaf));

    result.success = true;
    return result;
}

bool cpuid_has_1gb_pages(void)
{
    CpuidResult result = cpuid(CPUID_EXTENDED_LEAF, 0);

    if (!result.success)
    {
        return false;
    }

    return result.edx & CPUID_EXFEATURE_PDPE1GB;
}
