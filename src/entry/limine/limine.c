#include "limine.h"
#include <abstract/arch.h>
#include <abstract/entry.h>
#include <debug/debug.h>
#include <stdint.h>

static Mmap mmap = {0};

volatile static struct limine_memmap_request memmap_request = {
    .id = LIMINE_MEMMAP_REQUEST,
    .response = 0,
    .revision = 0,
};

volatile static struct limine_hhdm_request hhdm_request = {
    .id = LIMINE_HHDM_REQUEST,
    .response = 0,
    .revision = 0,
};

volatile static struct limine_kernel_address_request kaddr_req = {
    .id = LIMINE_KERNEL_ADDRESS_REQUEST,
    .response = 0,
    .revision = 0,
};

uintptr_t abstract_remove_hhdm(uintptr_t addr)
{
    if (hhdm_request.response == NULL)
    {
        debug(DEBUG_ERROR, "Couldn't get HHDM");
        debug_raise_exception();
    }

    return addr - hhdm_request.response->offset;
}

uintptr_t abstract_apply_hhdm(uintptr_t addr)
{
    if (hhdm_request.response == NULL)
    {
        debug(DEBUG_ERROR, "Couldn't get HHDM");
        debug_raise_exception();
    }

    return addr + hhdm_request.response->offset;
}

Kaddr abstract_get_kaddr(void)
{
    if (kaddr_req.response == NULL)
    {
        debug(DEBUG_ERROR, "Couldn't get Kernel addresses");
        debug_raise_exception();
    }

    return (Kaddr){
        .phys = kaddr_req.response->physical_base,
        .virt = kaddr_req.response->virtual_base,
    };
}

Mmap abstract_get_mmap(void)
{
    if (memmap_request.response == NULL)
    {
        debug(DEBUG_ERROR, "Couldn't get memory maps");
        debug_raise_exception();
    }

    if (mmap.count == 0)
    {
        mmap.count = memmap_request.response->entry_count;

        for (size_t i = 0; i < mmap.count; i++)
        {
            mmap.entries[i].base = memmap_request.response->entries[i]->base;
            mmap.entries[i].len = memmap_request.response->entries[i]->length;
            mmap.entries[i].type = memmap_request.response->entries[i]->type;
        }
    }

    return mmap;
}