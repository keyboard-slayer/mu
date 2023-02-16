#include "elf.h"
#include <abstract/const.h>
#include <abstract/entry.h>
#include <abstract/mem.h>
#include <debug/debug.h>
#include <misc/macro.h>
#include <string.h>

#include "pmm.h"
#include "sched.h"
#include "task.h"

void elf_load_module(char const *name)
{
    Module elf = abstract_get_module(name);
    Elf_Ehdr *hdr = (void *)elf.addr;

    if (memcmp(hdr->e_ident, ELFMAG, 4) != 0)
    {
        debug(DEBUG_ERROR, "%s is not a valid ELF binary", name);
        debug_raise_exception();
    }

    if (!elf_is_correct_class$(hdr))
    {
        debug(DEBUG_ERROR, "%s doesn't have the correct binary class", name);
        debug_raise_exception();
    }

    Space space = abstract_create_space();

    for (size_t i = 0; i < hdr->e_phnum; i++)
    {
        Elf_Phdr *phdr = (Elf_Phdr *)(elf.addr + hdr->e_phoff + i * hdr->e_phentsize);

        if (phdr->p_type == PT_LOAD)
        {
            Alloc pmm = pmm_acquire();

            size_t size = align_up(phdr->p_memsz, PAGE_SIZE);
            uintptr_t base = (uintptr_t)non_null$(pmm.malloc(&pmm, size / PAGE_SIZE));
            pmm.release(&pmm);

            if (kmmap(space, phdr->p_vaddr, base, size, PROT_READ | PROT_EXEC | MMAP_USER | MMAP_DEBUG) == MMAP_FAILURE)
            {
                debug(DEBUG_ERROR, "Couldn't map ELF binary");
                debug_raise_exception();
            }

            memcpy((void *)abstract_apply_hhdm(base), (void *)elf.addr + phdr->p_offset, phdr->p_filesz);
            memcpy((void *)abstract_apply_hhdm(base) + phdr->p_filesz,
                   (void *)(elf.addr + phdr->p_offset + phdr->p_filesz),
                   phdr->p_memsz - phdr->p_filesz);
        }
    }

    Task *task = task_init(elf.name, space);
    context_init(&task->context, hdr->e_entry, (TaskArgs){});

    sched_push_task(task);
}