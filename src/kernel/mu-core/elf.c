#include <const.h>
#include <mu-api/api.h>
#include <mu-core/const.h>
#include <mu-core/task.h>
#include <mu-debug/debug.h>
#include <mu-embed/alloc.h>
#include <mu-hal/hal.h>
#include <specs/elf.h>
#include <string.h>

MaybeTaskPtr elf_parse(cstr name, uintptr_t start, uintptr_t vspace, MuArgs args)
{
    debug_info("Loading module {}", name);
    Elf_Ehdr *hdr = (void *)start;

    if (memcmp(hdr->e_ident, ELFMAG, 4) != 0)
    {
        return None(MaybeTaskPtr);
    }

    if (!elf_is_correct_class(hdr))
    {
        return None(MaybeTaskPtr);
    }

    for (usize i = 0; i < hdr->e_phnum; i++)
    {
        Elf_Phdr *phdr = (Elf_Phdr *)(start + hdr->e_phoff + i * hdr->e_phentsize);

        if (phdr->p_type == PT_LOAD)
        {
            debug_info("({}) Mapping program header start: {x} len: {x}", name, phdr->p_vaddr, phdr->p_memsz);
            AllocObj paddr = Try(MaybeTaskPtr, embed_alloc(phdr->p_memsz));

            debug_info("({}) Phdr will be copied over 0x{a}", name, paddr.ptr);

            if (hal_space_map((HalSpace *)vspace, phdr->p_vaddr, hal_mmap_upper_to_lower(paddr.ptr), align_up(phdr->p_memsz, PAGE_SIZE), MU_MEM_READ | MU_MEM_WRITE | MU_MEM_USER | MU_MEM_EXEC) != MU_RES_OK)
            {
                return None(MaybeTaskPtr);
            }

            memcpy((void *)paddr.ptr, (void *)start + phdr->p_offset, phdr->p_filesz);
            memset((void *)paddr.ptr + phdr->p_filesz, 0, phdr->p_memsz - phdr->p_filesz);
        }
    }

    Task *task = Try(MaybeTaskPtr, task_init(str(name), (HalSpace *)vspace));

    if (hal_ctx_create(&task->context, hdr->e_entry, USER_STACK_TOP, args) != MU_RES_OK)
    {
        return None(MaybeTaskPtr);
    }

    return Some(MaybeTaskPtr, task);
}