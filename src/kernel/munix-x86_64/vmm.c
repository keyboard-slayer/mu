#include <debug/debug.h>
#include <handover/handover.h>
#include <handover/utils.h>
#include <munix-api/api.h>
#include <munix-core/const.h>
#include <munix-core/pmm.h>
#include <munix-hal/hal.h>

#include "asm.h"
#include "cpuid.h"
#include "vmm.h"

static size_t page_size = mib(2);
static uintptr_t *pml4 = NULL;

extern char text_start_addr[];
extern char text_end_addr[];
extern char rodata_start_addr[];
extern char rodata_end_addr[];
extern char data_start_addr[];
extern char data_end_addr[];

static int64_t transform_flags(MuMapFlags flags)
{
    int64_t ret_flags = VMM_NOEXE | VMM_PRESENT;

    if (flags & MU_MEM_READ)
    {
    }

    if (flags & MU_MEM_NONE)
    {
        ret_flags &= ~VMM_PRESENT;
    }

    if (flags & MU_MEM_WRITE)
    {
        ret_flags |= VMM_WRITE;
    }

    if (flags & MU_MEM_EXEC)
    {
        ret_flags &= ~VMM_NOEXE;
    }

    if (flags & MU_MEM_USER)
    {
        ret_flags |= VMM_USER;
    }

    if (flags & MU_MEM_HUGE)
    {
        ret_flags |= VMM_HUGE;
    }

    return ret_flags;
}

static uintptr_t *vmm_get_pml_alloc(uintptr_t *pml, size_t index, bool alloc)
{
    if ((pml[index] & VMM_PRESENT) != 0)
    {
        return (void *)(hal_mmap_lower_to_upper(VMM_GET_ADDR(pml[index])));
    }
    else if (alloc)
    {
        Alloc pmm = pmm_acquire();

        void *ptr = (void *)pmm.malloc(&pmm, PAGE_SIZE);
        if (ptr == NULL)
        {
            return NULL;
        }

        uintptr_t ptr_hhdm = hal_mmap_lower_to_upper((uintptr_t)ptr);

        memset((void *)ptr_hhdm, 0, PAGE_SIZE);
        pml[index] = (uintptr_t)ptr | VMM_PRESENT | VMM_WRITE | VMM_USER;
        pmm_release(&pmm);

        return (uintptr_t *)ptr_hhdm;
    }

    return NULL;
}

static MuRes kmmap_page(uintptr_t *pml, uint64_t virt, uint64_t phys, int64_t flags)
{
    size_t pml1_entry = PMLX_GET_INDEX(virt, 0);
    size_t pml2_entry = PMLX_GET_INDEX(virt, 1);
    size_t pml3_entry = PMLX_GET_INDEX(virt, 2);
    size_t pml4_entry = PMLX_GET_INDEX(virt, 3);

    uintptr_t *pml3 = vmm_get_pml_alloc(pml, pml4_entry, true);
    if (pml3 == NULL)
    {
        return MU_RES_NO_MEM;
    }

    if (cpuid_has_1gb_pages() && flags & VMM_HUGE)
    {
        pml3[pml3_entry] = phys | flags;
        return MU_RES_OK;
    }

    uintptr_t *pml2 = vmm_get_pml_alloc(pml3, pml3_entry, true);
    if (pml2 == NULL)
    {
        return MU_RES_NO_MEM;
    }

    if (flags & VMM_HUGE)
    {
        pml2[pml2_entry] = phys | flags;
        return MU_RES_OK;
    }

    uintptr_t *pml1 = vmm_get_pml_alloc(pml2, pml2_entry, true);
    if (pml1 == NULL)
    {
        return MU_RES_NO_MEM;
    }

    pml1[pml1_entry] = phys | flags;
    return MU_RES_OK;
}

static void kmmap_section(uintptr_t start, uintptr_t end, uint8_t flags)
{
    HalAddr kaddr = hal_mmap_kaddr();
    int64_t flags_arch = transform_flags(flags);
    size_t end_loop = align_up(end, PAGE_SIZE);

    for (size_t i = align_down(start, PAGE_SIZE); i < end_loop; i += PAGE_SIZE)
    {
        uintptr_t phys = i - kaddr.virt + kaddr.phys;

        if (kmmap_page(pml4, i, phys, flags_arch) != MU_RES_OK)
        {
            debug(DEBUG_ERROR, "Couldn't map kernel sections");
            debug_raise_exception();
        }
    }
}

MuRes hal_space_map(HalSpace *self, uintptr_t virt, uintptr_t phys, size_t len, MuMapFlags flags)
{
    int64_t flags_arch = transform_flags(flags);
    const size_t map_psize = flags & MU_MEM_HUGE ? page_size : PAGE_SIZE;

    size_t end = align_up(len, map_psize);
    size_t aligned_virt = align_down(virt, map_psize);
    size_t aligned_phys = align_down(phys, map_psize);

    for (size_t i = 0; i < end; i += map_psize)
    {
        int ret = kmmap_page((uintptr_t *)self, aligned_virt + i, aligned_phys + i, flags_arch);

        if (ret != MU_RES_OK)
        {
            return ret;
        }
    }

    return MU_RES_OK;
}

void vmm_init(void)
{
    Alloc pmm = pmm_acquire();
    HandoverPayload *handover = hal_get_handover();
    HandoverRecord record;

    pml4 = pmm.malloc(&pmm, PAGE_SIZE);

    if (pml4 == NULL)
    {
        debug(DEBUG_ERROR, "Couldn't allocate memory for pml4");
        debug_raise_exception();
    }

    debug(DEBUG_INFO, "PML4: 0x%p", pml4);

    pml4 = (uintptr_t *)hal_mmap_lower_to_upper((uintptr_t)pml4);
    memset(pml4, 0, PAGE_SIZE);
    pmm.release(&pmm);

    if (cpuid_has_1gb_pages())
    {
        debug(DEBUG_INFO, "1GB pages are supported");
        page_size = gib(1);
    }
    else
    {
        debug(DEBUG_INFO, "1GB pages are not supported, defaulting to 2MB pages");
        page_size = mib(2);
    }

    kmmap_section((uintptr_t)text_start_addr, (uintptr_t)text_end_addr, MU_MEM_READ | MU_MEM_EXEC);
    kmmap_section((uintptr_t)rodata_start_addr, (uintptr_t)rodata_end_addr, MU_MEM_READ);
    kmmap_section((uintptr_t)data_start_addr, (uintptr_t)data_end_addr, MU_MEM_READ | MU_MEM_WRITE);

    debug(DEBUG_INFO, "Kernel sections mapped");

    size_t end = max(gib(4), pmm_available_pages() * PAGE_SIZE);
    uint64_t flags = transform_flags(MU_MEM_WRITE | MU_MEM_READ | MU_MEM_HUGE);

    for (size_t i = 1; i < end; i += page_size)
    {
        kmmap_page(pml4, hal_mmap_lower_to_upper(i), i, flags);
    }

    handover_foreach_record(handover, record)
    {
        if (hal_space_map((HalSpace *)pml4, hal_mmap_lower_to_upper(record.start), record.start, record.size, MU_MEM_READ | MU_MEM_WRITE | MU_MEM_HUGE) != MU_RES_OK)
        {
            debug(DEBUG_ERROR, "Couldn't map kernel properly");
            debug_raise_exception();
        }
    }

    debug(DEBUG_INFO, "Memory mapped");

    hal_space_apply((HalSpace *)pml4);

    debug(DEBUG_INFO, "Space applied");
}

void hal_space_apply(HalSpace *space)
{
    asm_write_cr(3, hal_mmap_upper_to_lower((uintptr_t)space));
}

MuRes hal_space_create(HalSpace **self)
{
    Alloc pmm = pmm_acquire();
    void *ptr = pmm.calloc(&pmm, 1, PAGE_SIZE);
    pmm_release(&pmm);

    if (ptr == NULL)
    {
        return MU_RES_NO_MEM;
    }

    uintptr_t *space = (uintptr_t *)hal_mmap_lower_to_upper((uintptr_t)ptr);

    for (size_t i = 255; i < 512; i++)
    {
        space[i] = pml4[i];
    }

    *self = (HalSpace *)space;

    return MU_RES_OK;
}

HalSpace *hal_space_kernel(void)
{
    return (HalSpace *)pml4;
}