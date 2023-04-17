#include <handover/utils.h>
#include <mu-base/std.h>
#include <mu-hal/hal.h>
#include <string.h>

#include "const.h"
#include "elf.h"
#include "pmm.h"
#include "sched.h"

MaybeTaskPtr elf_load_module(cstr name, MuArgs args)
{
    HalSpace *space;
    HandoverRecord file = handover_file_find(hal_get_handover(), name);
    debug_info("Loading module {}", name);
    Elf_Ehdr *hdr = (void *)file.start;

    if (memcmp(hdr->e_ident, ELFMAG, 4) != 0)
    {
        panic("{} is not a valid ELF binary", name);
    }

    if (!elf_is_correct_class$(hdr))
    {
        panic("{} doesn't have the correct binary class", name);
    }

    if (hal_space_create(&space) != MU_RES_OK)
    {
        panic("Couldn't create space for ELF binary");
    }

    for (usize i = 0; i < hdr->e_phnum; i++)
    {
        Elf_Phdr *phdr = (Elf_Phdr *)(file.start + hdr->e_phoff + i * hdr->e_phentsize);

        if (phdr->p_type == PT_LOAD)
        {
            debug_info("Mapping program header start: {x} end: {x}", phdr->p_vaddr, phdr->p_vaddr + phdr->p_memsz);
            Alloc pmm = pmm_acquire();

            usize size = align_up(phdr->p_memsz, PAGE_SIZE);
            uintptr_t paddr = (uintptr_t)Try(MaybeTaskPtr, pmm.malloc(&pmm, size / PAGE_SIZE));
            pmm.release(&pmm);

            debug_info("Phdr will be copied over 0x{a}", paddr);

            if (hal_space_map(space, phdr->p_vaddr, paddr, size, MU_MEM_READ | MU_MEM_USER | MU_MEM_EXEC) != MU_RES_OK)
            {
                panic("Couldn't map ELF binary");
            }

            memcpy((void *)hal_mmap_lower_to_upper(paddr), (void *)file.start + phdr->p_offset, phdr->p_filesz);
            memset((void *)hal_mmap_lower_to_upper(paddr) + phdr->p_filesz, 0, phdr->p_memsz - phdr->p_filesz);
        }
    }

    Task *task = Try(MaybeTaskPtr, task_init(str(name), space));
    if (hal_ctx_create(&task->context, hdr->e_entry, USER_STACK_BASE, args) != MU_RES_OK)
    {
        panic("Couldn't create context for ELF binary");
    }

    return Just(MaybeTaskPtr, task);
}