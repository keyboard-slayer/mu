#include <debug/debug.h>
#include <mu-hal/hal.h>
#include <string.h>

#include "acpi.h"

static EitherRsdtXsdt sdt;

void acpi_init(void)
{
    Rsdp *rsdp = abstract_get_rsdp();

    if (rsdp->revision >= 2 && rsdp->xsdt_addr != 0)
    {
        debug(DEBUG_INFO, "ACPI: Using XSDT");
        sdt = Right(EitherRsdtXsdt, (Xsdt *)hal_mmap_lower_to_upper(rsdp->xsdt_addr));
    }
    else
    {
        debug(DEBUG_INFO, "ACPI: Using RSDT");
        sdt = Left(EitherRsdtXsdt, (Rsdt *)hal_mmap_lower_to_upper(rsdp->rsdt_addr));
    }
}

static int acpi_checksum(AcpiSdt *table)
{
    uint8_t sum = 0;

    for (size_t i = 0; i < table->length; i++)
    {
        sum += ((char *)table)[i];
    }

    return sum == 0;
}

AcpiSdt *acpi_parse_sdt(char const *tablename)
{
    size_t entry_count = 0;
    AcpiSdt *tmp;

    if (sdt.is_left)
    {
        entry_count = sdt.left->header.length - sizeof(sdt.left->header) / 4;
    }
    else
    {
        entry_count = sdt.right->header.length - sizeof(sdt.right->header) / 8;
    }

    for (size_t i = 0; i < entry_count; i++)
    {
        if (sdt.is_left)
        {
            tmp = (AcpiSdt *)(hal_mmap_lower_to_upper(sdt.left->sdtAddr[i]));
        }
        else
        {
            tmp = (AcpiSdt *)(hal_mmap_lower_to_upper(sdt.right->sdtAddr[i]));
        }

        if (memcmp(tmp->signature, tablename, 4) == 0 && acpi_checksum(tmp))
        {
            return tmp;
        }
    }

    return NULL;
}