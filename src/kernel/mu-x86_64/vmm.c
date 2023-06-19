#include <handover/utils.h>
#include <mu-core/const.h>
#include <mu-core/pmm.h>
#include <mu-debug/debug.h>
#include <mu-hal/hal.h>

#include "asm.h"
#include "cpuid.h"
#include "mu-api/api.h"
#include "vmm.h"

static usize page_size = mib(2);
static uintptr_t *pml4 = NULL;

extern char text_start_addr[];
extern char text_end_addr[];
extern char rodata_start_addr[];
extern char rodata_end_addr[];
extern char data_start_addr[];
extern char data_end_addr[];

static i64 transform_flags(MuMapFlags flags)
{
    i64 ret_flags = VMM_NOEXE | VMM_PRESENT;

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

static MaybePtr vmm_get_pml_alloc(uintptr_t *pml, usize index, bool alloc)
{
    if ((pml[index] & VMM_PRESENT))
    {
        return Some(MaybePtr, (void *)(hal_mmap_lower_to_upper(VMM_GET_ADDR(pml[index]))));
    }
    else if (alloc)
    {
        cleanup(pmm_release) Pmm pmm = pmm_acquire();
        uintptr_t *ptr = (uintptr_t *)Try(MaybePtr, pmm.calloc(1, PAGE_SIZE, false)).ptr;
        uintptr_t ptr_hhdm = hal_mmap_lower_to_upper((uintptr_t)ptr);

        pml[index] = (uintptr_t)ptr | VMM_PRESENT | VMM_WRITE | VMM_USER;
        pmm_release(&pmm);

        return Some(MaybePtr, (void *)ptr_hhdm);
    }

    return None(MaybePtr);
}

static MuRes kmmap_page(uintptr_t *pml, u64 virt, u64 phys, i64 flags)
{
    if (phys % PAGE_SIZE != 0 || virt % PAGE_SIZE != 0)
    {
        return MU_RES_NON_ALIGN;
    }

    usize pml1_entry = PMLX_GET_INDEX(virt, 0);
    usize pml2_entry = PMLX_GET_INDEX(virt, 1);
    usize pml3_entry = PMLX_GET_INDEX(virt, 2);
    usize pml4_entry = PMLX_GET_INDEX(virt, 3);

    uintptr_t *pml3 = unwrap_or(vmm_get_pml_alloc(pml, pml4_entry, true), NULL);
    if (pml3 == NULL)
    {
        return MU_RES_NO_MEM;
    }

    if (cpuid_has_1gb_pages() && flags & VMM_HUGE)
    {
        pml3[pml3_entry] = phys | flags;
        return MU_RES_OK;
    }

    uintptr_t *pml2 = unwrap_or(vmm_get_pml_alloc(pml3, pml3_entry, true), NULL);
    if (pml2 == NULL)
    {
        return MU_RES_NO_MEM;
    }

    if (flags & VMM_HUGE)
    {
        pml2[pml2_entry] = phys | flags;
        return MU_RES_OK;
    }

    uintptr_t *pml1 = unwrap_or(vmm_get_pml_alloc(pml2, pml2_entry, true), NULL);
    if (pml1 == NULL)
    {
        return MU_RES_NO_MEM;
    }

    pml1[pml1_entry] = phys | flags;
    return MU_RES_OK;
}

static void kmmap_section(uintptr_t start, uintptr_t end, u8 flags)
{
    HalAddr kaddr = hal_mmap_kaddr();
    i64 flags_arch = transform_flags(flags);
    usize end_loop = align_up(end, PAGE_SIZE);

    for (usize i = align_down(start, PAGE_SIZE); i < end_loop; i += PAGE_SIZE)
    {
        uintptr_t phys = i - kaddr.virt + kaddr.phys;

        assert(kmmap_page(pml4, i, phys, flags_arch) == MU_RES_OK, "Couldn't map kernel sections");
    }
}

MuRes hal_space_map(HalSpace *self, uintptr_t virt, uintptr_t phys, usize len, MuMapFlags flags)
{
    if (phys % PAGE_SIZE != 0 || virt % PAGE_SIZE != 0 || len % PAGE_SIZE != 0)
    {
        return MU_RES_NON_ALIGN;
    }

    i64 flags_arch = transform_flags(flags);
    const usize map_psize = flags & MU_MEM_HUGE ? page_size : PAGE_SIZE;

    usize end = align_up(len, map_psize);
    usize aligned_virt = align_down(virt, map_psize);
    usize aligned_phys = align_down(phys, map_psize);

    for (usize i = 0; i < end; i += map_psize)
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
    Pmm pmm = pmm_acquire();
    HandoverPayload *handover = hal_get_handover();
    HandoverRecord record;

    auto alloc = pmm.calloc(1, PAGE_SIZE, false);

    if (!alloc.isSome)
    {
        panic("Couldn't allocate memory for pml4");
    }

    pml4 = (uintptr_t *)alloc.value.ptr;

    debug_info("PML4: 0x{a}", (uintptr_t)pml4);

    pml4 = (uintptr_t *)hal_mmap_lower_to_upper((uintptr_t)pml4);
    pmm.release(&pmm);

    if (cpuid_has_1gb_pages())
    {
        debug_info("1GB pages are supported");
        page_size = gib(1);
    }
    else
    {
        debug_info("1GB pages are not supported, defaulting to 2MB pages");
        page_size = mib(2);
    }

    kmmap_section((uintptr_t)text_start_addr, (uintptr_t)text_end_addr, MU_MEM_READ | MU_MEM_EXEC);
    kmmap_section((uintptr_t)rodata_start_addr, (uintptr_t)rodata_end_addr, MU_MEM_READ);
    kmmap_section((uintptr_t)data_start_addr, (uintptr_t)data_end_addr, MU_MEM_READ | MU_MEM_WRITE);

    debug_info("Kernel sections mapped");

    usize end = max(gib(4), pmm_available_pages() * PAGE_SIZE);
    u64 flags = transform_flags(MU_MEM_WRITE | MU_MEM_READ | MU_MEM_HUGE);

    for (usize i = page_size; i < end; i += page_size)
    {
        kmmap_page(pml4, hal_mmap_lower_to_upper(i), i, flags);
    }

    handover_foreach_record(handover, record)
    {
        if (record.tag != HANDOVER_FILE && record.tag != HANDOVER_FB && hal_space_map((HalSpace *)pml4, hal_mmap_lower_to_upper(record.start), record.start, record.size, MU_MEM_READ | MU_MEM_WRITE | MU_MEM_HUGE) != MU_RES_OK)
        {
            panic("Couldn't map kernel properly");
        }
    }

    debug_info("Memory mapped");

    hal_space_apply((HalSpace *)pml4);

    debug_info("Space applied");
}

void hal_space_apply(HalSpace *space)
{
    asm_write_cr(3, hal_mmap_upper_to_lower((uintptr_t)space));
}

MuRes hal_space_create(HalSpace **self)
{
    Pmm pmm = pmm_acquire();
    auto ptr = pmm.calloc(1, PAGE_SIZE, false);
    pmm_release(&pmm);

    if (!ptr.isSome)
    {
        return MU_RES_NO_MEM;
    }

    uintptr_t *space = (uintptr_t *)hal_mmap_lower_to_upper((uintptr_t)ptr.value.ptr);
    memset((void *)space, 0, PAGE_SIZE);

    for (usize i = 255; i < 512; i++)
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