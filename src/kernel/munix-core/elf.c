#include <debug/debug.h>
#include <handover/utils.h>
#include <misc/macro.h>
#include <munix-hal/hal.h>
#include <string.h>

#include "const.h"
#include "elf.h"
#include "pmm.h"
#include "sched.h"
#include "task.h"

void elf_load_module(char const *name)
{
    HandoverRecord file = handover_file_find(hal_get_handover(), name);
    Elf_Ehdr *hdr = (void *)file.start;

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

    HalSpace *space;
    if (hal_space_create(&space) != MU_RES_OK)
    {
        debug(DEBUG_ERROR, "Couldn't create space for ELF binary");
        debug_raise_exception();
    }

    for (size_t i = 0; i < hdr->e_phnum; i++)
    {
        Elf_Phdr *phdr = (Elf_Phdr *)(file.start + hdr->e_phoff + i * hdr->e_phentsize);

        if (phdr->p_type == PT_LOAD)
        {
            Alloc pmm = pmm_acquire();

            size_t size = align_up(phdr->p_memsz, PAGE_SIZE);
            uintptr_t base = (uintptr_t)non_null$(pmm.malloc(&pmm, size / PAGE_SIZE));
            pmm.release(&pmm);

            if (hal_space_map(space, phdr->p_vaddr, base, size, MU_MEM_READ | MU_MEM_WRITE | MU_MEM_USER | MU_MEM_HUGE) != MU_RES_OK)
            {
                debug(DEBUG_ERROR, "Couldn't map ELF binary");
                debug_raise_exception();
            }

            memcpy((void *)hal_mmap_lower_to_upper(base), (void *)file.start + phdr->p_offset, phdr->p_filesz);
            memcpy((void *)hal_mmap_lower_to_upper(base) + phdr->p_filesz,
                   (void *)(file.start + phdr->p_offset + phdr->p_filesz),
                   phdr->p_memsz - phdr->p_filesz);
        }
    }

    Task *task = task_init(name, space);
    hal_ctx_create(&task->context, hdr->e_entry, task->stack, (MuArgs){});

    sched_push_task(task);
}