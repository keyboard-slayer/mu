#include <abstract/arch.h>
#include <abstract/const.h>
#include <abstract/entry.h>
#include <abstract/mem.h>
#include <debug/debug.h>
#include <munix-core/pmm.h>

#include "asm.h"
#include "cpuid.h"
#include "vmm.h"

static size_t page_size = mib(2);
static Space pml4 = NULL;

extern char text_start_addr[];
extern char text_end_addr[];
extern char rodata_start_addr[];
extern char rodata_end_addr[];
extern char data_start_addr[];
extern char data_end_addr[];

static int64_t transform_flags(uint8_t flags)
{
    int64_t ret_flags = VMM_NOEXE | VMM_PRESENT;

    if (flags & PROT_READ)
    {
    }

    if (flags & PROT_NONE)
    {
        ret_flags &= ~VMM_PRESENT;
    }

    if (flags & PROT_WRITE)
    {
        ret_flags |= VMM_WRITE;
    }

    if (flags & PROT_EXEC)
    {
        ret_flags &= ~VMM_NOEXE;
    }

    if (flags & MMAP_USER)
    {
        ret_flags |= VMM_USER;
    }

    if (flags & MMAP_HUGE)
    {
        ret_flags |= VMM_HUGE;
    }

    return ret_flags;
}

static Space vmm_get_pml_alloc(Space pml, size_t index, bool alloc)
{
    if ((pml[index] & VMM_PRESENT) != 0)
    {
        return (void *)(abstract_apply_hhdm(VMM_GET_ADDR(pml[index])));
    }
    else if (alloc)
    {
        Alloc pmm = pmm_acquire();

        void *ptr = (void *)pmm.malloc(&pmm, PAGE_SIZE);
        if (ptr == NULL)
        {
            return NULL;
        }

        uintptr_t ptr_hhdm = abstract_apply_hhdm((uintptr_t)ptr);

        memset((void *)ptr_hhdm, 0, PAGE_SIZE);
        pml[index] = (uintptr_t)ptr | VMM_PRESENT | VMM_WRITE | VMM_USER;
        pmm_release(&pmm);

        return (Space)ptr_hhdm;
    }

    return NULL;
}

static int kmmap_page(Space pml, uint64_t virt, uint64_t phys, int64_t flags)
{
    size_t pml1_entry = PMLX_GET_INDEX(virt, 0);
    size_t pml2_entry = PMLX_GET_INDEX(virt, 1);
    size_t pml3_entry = PMLX_GET_INDEX(virt, 2);
    size_t pml4_entry = PMLX_GET_INDEX(virt, 3);

    Space pml3 = vmm_get_pml_alloc(pml, pml4_entry, true);
    if (pml3 == NULL)
    {
        return MMAP_FAILURE;
    }

    if (cpuid_has_1gb_pages() && flags & VMM_HUGE)
    {
        pml3[pml3_entry] = phys | flags;
        return MMAP_SUCCESS;
    }

    Space pml2 = vmm_get_pml_alloc(pml3, pml3_entry, true);
    if (pml2 == NULL)
    {
        return MMAP_FAILURE;
    }

    if (flags & VMM_HUGE)
    {
        pml2[pml2_entry] = phys | flags;
        return MMAP_SUCCESS;
    }

    Space pml1 = vmm_get_pml_alloc(pml2, pml2_entry, true);
    if (pml1 == NULL)
    {
        return MMAP_FAILURE;
    }

    pml1[pml1_entry] = phys | flags;
    return MMAP_SUCCESS;
}

static void kmmap_section(uintptr_t start, uintptr_t end, uint8_t flags)
{
    Kaddr kaddr = abstract_get_kaddr();
    int64_t flags_arch = transform_flags(flags);
    size_t end_loop = align_up(end, PAGE_SIZE);

    for (size_t i = align_down(start, PAGE_SIZE); i < end_loop; i += PAGE_SIZE)
    {
        uintptr_t phys = i - kaddr.virt + kaddr.phys;

        if (kmmap_page(pml4, i, phys, flags_arch) == MMAP_FAILURE)
        {
            debug(DEBUG_ERROR, "Couldn't map kernel sections");
            debug_raise_exception();
        }
    }
}

int kmmap(Space space, uintptr_t virt, uintptr_t phys, size_t length, uint8_t flags)
{
    int64_t flags_arch = transform_flags(flags);
    const size_t map_psize = flags & MMAP_HUGE ? page_size : PAGE_SIZE;

    size_t end = align_up(length, map_psize);
    size_t aligned_virt = align_down(virt, map_psize);
    size_t aligned_phys = align_down(phys, map_psize);

    for (size_t i = 0; i < end; i += map_psize)
    {
        int ret = kmmap_page(space, aligned_virt + i, aligned_phys + i, flags_arch);

        if (ret == MMAP_FAILURE)
        {
            return MMAP_FAILURE;
        }
    }

    return MMAP_SUCCESS;
}

void vmm_init(void)
{
    Alloc pmm = pmm_acquire();
    Mmap mmaps = abstract_get_mmap();

    pml4 = pmm.malloc(&pmm, PAGE_SIZE);

    if (pml4 == NULL)
    {
        debug(DEBUG_ERROR, "Couldn't allocate memory for pml4");
        debug_raise_exception();
    }

    pml4 = (Space)abstract_apply_hhdm((uintptr_t)pml4);
    memset(pml4, 0, PAGE_SIZE);
    pmm.release(&pmm);

    if (cpuid_has_1gb_pages())
    {
        page_size = gib(1);
    }

    kmmap_section((uintptr_t)text_start_addr, (uintptr_t)text_end_addr, PROT_READ | PROT_EXEC);
    kmmap_section((uintptr_t)rodata_start_addr, (uintptr_t)rodata_end_addr, PROT_READ);
    kmmap_section((uintptr_t)data_start_addr, (uintptr_t)data_end_addr, PROT_READ | PROT_WRITE);

    size_t end = max(gib(4), pmm_available_pages() * PAGE_SIZE);
    uint64_t flags = transform_flags(PROT_WRITE | PROT_READ | MMAP_HUGE);

    for (size_t i = 1; i < end; i += page_size)
    {
        kmmap_page(pml4, abstract_apply_hhdm(i), i, flags);
    }

    for (size_t i = 0; i < mmaps.count; i++)
    {
        if (kmmap(pml4, abstract_apply_hhdm(mmaps.entries[i].base), mmaps.entries[i].base, mmaps.entries[i].len, PROT_READ | PROT_WRITE | MMAP_HUGE) == MMAP_FAILURE)
        {
            debug(DEBUG_ERROR, "Couldn't map kernel properly");
            debug_raise_exception();
        }
    }

    abstract_switch_space(pml4);
}

void abstract_switch_space(Space space)
{
    asm_write_cr(3, abstract_remove_hhdm((uintptr_t)space));
}

Space abstract_create_space(void)
{
    Alloc pmm = pmm_acquire();
    Space space = (Space)abstract_apply_hhdm((uintptr_t)non_null$(pmm.calloc(&pmm, 1, PAGE_SIZE)));

    for (size_t i = 255; i < 512; i++)
    {
        space[i] = pml4[i];
    }

    pmm_release(&pmm);

    return space;
}

Space abstract_get_kernel_space(void)
{
    return pml4;
}