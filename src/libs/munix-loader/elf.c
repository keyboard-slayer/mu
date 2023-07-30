#include <mu-embed/alloc.h>
#include <string.h>

#include "elf.h"

MaybeElfReturn elf_parse_(char const name[static 1], uintptr_t elf_ptr, ElfOptionalParams params[static 1])
{
    Elf_Ehdr *hdr = (void *)elf_ptr;
    MuCap vspace;
    MuCap task;

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
            AllocObj paddr = Try(MaybeElfReturn, embed_alloc(phdr->p_memsz));
            if (mu_map(vspace, (MuCap){paddr.ptr}, phdr->p_vaddr, 0, align_up(phdr->p_memsz, PAGE_SIZE), MU_MEM_READ | MU_MEM_WRITE | MU_MEM_EXEC) != MU_RES_OK)
            {
                return None(MaybeElfReturn);
            }

            memcpy((void *)paddr.ptr, (void *)elf_ptr + phdr->p_offset, phdr->p_filesz);
            memset((void *)paddr.ptr + phdr->p_filesz, 0, phdr->p_memsz - phdr->p_filesz);
        }
    }

    mu_create_task(&task, (MuCap){(uintptr_t)name}, vspace);
    ElfReturn ret = {task, hdr->e_entry};
    return Some(MaybeElfReturn, ret);
}