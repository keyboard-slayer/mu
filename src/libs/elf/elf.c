#include <mu-api/api.h>
#include <mu-core/const.h>
#include <mu-embed/alloc.h>
#include <string.h>

#ifdef __osdk_freestanding__
#    include <mu-core/task.h>
#    include <mu-hal/hal.h>
#endif /* !__osdk_freestanding__ */

#include "elf.h"

MaybeMuCap elf_parse(cstr name, uintptr_t start, uintptr_t vspace, MuArgs args)
{
    debug_info("Loading module {}", name);
    Elf_Ehdr *hdr = (void *)start;

    if (memcmp(hdr->e_ident, ELFMAG, 4) != 0)
    {
        return None(MaybeMuCap);
    }

    if (!elf_is_correct_class(hdr))
    {
        return None(MaybeMuCap);
    }

    for (usize i = 0; i < hdr->e_phnum; i++)
    {
        Elf_Phdr *phdr = (Elf_Phdr *)(start + hdr->e_phoff + i * hdr->e_phentsize);

        if (phdr->p_type == PT_LOAD)
        {
            debug_info("({}) Mapping program header start: {x} len: {x}", name, phdr->p_vaddr, phdr->p_memsz);
            AllocObj paddr = Try(MaybeMuCap, embed_alloc(phdr->p_memsz));

            debug_info("({}) Phdr will be copied over 0x{a}", name, paddr.ptr);

#ifdef __osdk_freestanding__
            if (hal_space_map((HalSpace *)vspace, phdr->p_vaddr, hal_mmap_upper_to_lower(paddr.ptr), align_up(phdr->p_memsz, PAGE_SIZE), MU_MEM_READ | MU_MEM_WRITE | MU_MEM_USER | MU_MEM_EXEC) != MU_RES_OK)
#else
            if (mu_map((MuCap){vspace}, (MuCap){paddr.ptr}, phdr->p_vaddr, 0, align_up(phdr->p_memsz, PAGE_SIZE), MU_MEM_READ | MU_MEM_WRITE | MU_MEM_EXEC) != MU_RES_OK)
#endif
            {
                return None(MaybeMuCap);
            }

            memcpy((void *)paddr.ptr, (void *)start + phdr->p_offset, phdr->p_filesz);
            memset((void *)paddr.ptr + phdr->p_filesz, 0, phdr->p_memsz - phdr->p_filesz);
        }
    }

#ifdef __osdk_freestanding__
    Task *task_impl = Try(MaybeMuCap, task_init(str(name), (HalSpace *)vspace));
    MuCap task = (MuCap){(uintptr_t)task_impl};

    if (hal_ctx_create(&task_impl->context, hdr->e_entry, USER_STACK_BASE, args) != MU_RES_OK)
    {
        return None(MaybeMuCap);
    }
#else
    MuCap task;
    if (mu_create_task(&task, (MuCap){(uintptr_t)name}, (MuCap){vspace}) != MU_RES_OK || mu_start(task, hdr->e_entry, USER_STACK_BASE, args) != MU_RES_OK)
    {
        return None(MaybeMuCap);
    }
#endif

    return Some(MaybeMuCap, task);
}