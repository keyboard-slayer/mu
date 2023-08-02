#include <mu-debug/debug.h>
#include <mu-embed/alloc.h>
#include <string.h>
#include <sys/mman.h>

#include "elf.h"

MaybeElfReturn elf_parse_(char const name[static 1], uintptr_t elf_ptr, ElfOptionalParams params[static 1])
{
    Elf_Ehdr *hdr = (void *)elf_ptr;
    MuCap vspace;
    MuCap task;
    MuCap vmo;
    uintptr_t paddr;

    // === REMOVE ME ===

    MuCap self;

    if (mu_self(&self) != MU_RES_OK)
    {
        debug_warn("Couldn't get task implem");
        return None(MaybeElfReturn);
    }

    MuCap space = ((MuTask *)self._raw)->space;

    // =================

    if (params->vspace._raw == 0 && mu_create_vspace(&vspace) != MU_RES_OK)
    {
        return None(MaybeElfReturn);
    }
    else
    {
        vspace = params->vspace;
    }

    if (memcmp(hdr->e_ident, ELFMAG, 4) != 0)
    {
        return None(MaybeElfReturn);
    }

    if (!elf_is_correct_class(hdr))
    {
        return None(MaybeElfReturn);
    }

    for (usize i = 0; i < hdr->e_phnum; i++)
    {
        Elf_Phdr *phdr = (Elf_Phdr *)(elf_ptr + hdr->e_phoff + i * hdr->e_phentsize);

        if (phdr->p_type == PT_LOAD)
        {
            debug_info("({}) Mapping program header start: {x} len: {x}", name, phdr->p_vaddr, phdr->p_memsz);
            uintptr_t vaddr = phdr->p_vaddr;

            if (mu_create_vmo(&vmo, 0, phdr->p_memsz, MU_MEM_LOW) != MU_RES_OK)
            {
                debug_warn("Couldn't create VMO");
                return None(MaybeElfReturn);
            }

            if (mu_map(space, vmo, &paddr, 0, phdr->p_memsz, MU_MEM_READ | MU_MEM_WRITE) != MU_RES_OK)
            {
                debug_warn("Failed to map ELF segment (for copy)");
                return None(MaybeElfReturn);
            }

            if (mu_map(vspace, vmo, (uintptr_t *)&vaddr, 0, phdr->p_memsz, MU_MEM_READ | MU_MEM_WRITE | MU_MEM_EXEC | MU_MEM_NO_ALLOC) != MU_RES_OK)
            {
                debug_warn("Failed to map ELF segment");
                return None(MaybeElfReturn);
            }

            memcpy((void *)paddr, (void *)elf_ptr + phdr->p_offset, phdr->p_filesz);
            memset((void *)paddr + phdr->p_filesz, 0, phdr->p_memsz - phdr->p_filesz);
        }
    }

    void *name_str = mmap(NULL, strlen(name) + 1, PROT_WRITE | PROT_READ, MAP_SHARED | MAP_ANON, -1, 0);
    if (name_str == NULL)
    {
        return None(MaybeElfReturn);
    }

    memcpy(name_str, name, strlen(name));

    if (mu_create_task(&task, (uintptr_t)(name_str), vspace) != MU_RES_OK)
    {
        debug_warn("Failed to create task");
        return None(MaybeElfReturn);
    }

    ElfReturn ret = {task, hdr->e_entry};
    return Some(MaybeElfReturn, ret);
}