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
    HalSpace *space = nullptr; // TODO DESTROY SPACE
    HandoverRecord file = handover_file_find(hal_get_handover(), name);
    debug_info("Loading module {}", name);
    Elf_Ehdr *hdr = (void *)file.start;

    if (memcmp(hdr->e_ident, ELFMAG, 4) != 0)
    {
        return None(MaybeTaskPtr);
    }

    if (!elf_is_correct_class(hdr))
    {
        return None(MaybeTaskPtr);
    }

    if (hal_space_create(&space) != MU_RES_OK)
    {
        return None(MaybeTaskPtr);
    }

    for (usize i = 0; i < hdr->e_phnum; i++)
    {
        Elf_Phdr *phdr = (Elf_Phdr *)(file.start + hdr->e_phoff + i * hdr->e_phentsize);

        if (phdr->p_type == PT_LOAD)
        {
            debug_info("Mapping program header start: {x} len: {x}", phdr->p_vaddr, phdr->p_memsz);
            cleanup(pmm_release) Pmm pmm = pmm_acquire();
            PmmObj paddr = Try(MaybeTaskPtr, pmm.malloc(phdr->p_memsz));
            pmm.release(&pmm);

            debug_info("Phdr will be copied over 0x{a}", paddr.ptr);

            if (hal_space_map(space, phdr->p_vaddr, paddr.ptr, align_up(phdr->p_memsz, PAGE_SIZE), MU_MEM_READ | MU_MEM_USER | MU_MEM_EXEC) != MU_RES_OK)
            {
                return None(MaybeTaskPtr);
            }

            memcpy((void *)hal_mmap_lower_to_upper(paddr.ptr), (void *)file.start + phdr->p_offset, phdr->p_filesz);
            memset((void *)hal_mmap_lower_to_upper(paddr.ptr) + phdr->p_filesz, 0, phdr->p_memsz - phdr->p_filesz);
        }
    }

    Task *task = Try(MaybeTaskPtr, task_init(str(name), space));
    if (hal_ctx_create(&task->context, hdr->e_entry, USER_STACK_BASE, args) != MU_RES_OK)
    {
        panic("Couldn't create context for ELF binary");
    }

    return Some(MaybeTaskPtr, task);
}