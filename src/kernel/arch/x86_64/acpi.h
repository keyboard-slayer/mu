#pragma once

#include <ds/either.h>
#include <base/macro.h>
#include <stdint.h>

typedef struct packed
{
    char signature[8];
    uint8_t checksum;
    char oemid[6];
    uint8_t revision;
    uint32_t rsdt_addr;
    uint32_t length;
    uint64_t xsdt_addr;
    uint8_t ext_checksum;
    char reserved[3];
} Rsdp;

typedef struct packed
{
    char signature[4];
    uint32_t length;
    uint8_t revision;
    uint8_t checksum;
    char oemid[6];
    char oemtableid[8];
    uint32_t oemrevision;
    uint32_t creatorid;
    uint32_t creatorrevision;
} AcpiSdt;

typedef struct packed
{
    AcpiSdt header;
    uint32_t sdtAddr[];
} Rsdt;

typedef struct packed
{
    AcpiSdt header;
    uint64_t sdtAddr[];
} Xsdt;

void acpi_init(void);
AcpiSdt *acpi_parse_sdt(char const *tablename);

void *abstract_get_rsdp(void);

typedef Either(Rsdt *, Xsdt *) EitherRsdtXsdt;