#pragma once

#include <mu-base/std.h>
#include <mu-ds/either.h>

typedef struct packed
{
    char signature[8];
    u8 checksum;
    char oemid[6];
    u8 revision;
    u32 rsdt_addr;
    u32 length;
    u64 xsdt_addr;
    u8 ext_checksum;
    char reserved[3];
} Rsdp;

typedef struct packed
{
    char signature[4];
    u32 length;
    u8 revision;
    u8 checksum;
    char oemid[6];
    char oemtableid[8];
    u32 oemrevision;
    u32 creatorid;
    u32 creatorrevision;
} AcpiSdt;

typedef struct packed
{
    AcpiSdt header;
    u32 sdtAddr[];
} Rsdt;

typedef struct packed
{
    AcpiSdt header;
    u64 sdtAddr[];
} Xsdt;

void acpi_init(void);
AcpiSdt *acpi_parse_sdt(char const *tablename);

void *abstract_get_rsdp(void);

typedef Either(Rsdt *, Xsdt *) EitherRsdtXsdt;