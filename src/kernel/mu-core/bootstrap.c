#include <elf/elf.h>
#include <handover/utils.h>
#include <mu-api/bootstrap.h>
#include <mu-base/std.h>
#include <mu-core/const.h>
#include <mu-ds/vec.h>
#include <mu-hal/hal.h>
#include <mu-mem/heap.h>

#include <mu-x86_64/asm.h>

#include "sched.h"

typedef Vec(Module) VecModule;

static void passModules(Task *task)
{
    HandoverRecord rec;
    VecModule mods;

    auto handover = hal_get_handover();
    vec_init(&mods, heap_acquire);

    handover_foreach_record(handover, rec)
    {
        if (rec.tag == HANDOVER_FILE)
        {
            uintptr_t addr = hal_mmap_upper_to_lower(rec.start);
            Module mod = (Module){.name = {0}, .ptr = addr, .len = rec.size};
            cstr filename = (cstr)handover + rec.file.name;

            if (memcmp(filename, "/bin/bootstrap", 14) == 0)
            {
                continue;
            }

            hal_space_map(task->space, addr, addr, align_up(rec.size, PAGE_SIZE), MU_MEM_USER | MU_MEM_READ);

            memcpy(&mod.name, filename, strlen(filename));
            vec_push(&mods, mod);
        }
    }

    uintptr_t addr = hal_mmap_upper_to_lower((uintptr_t)mods.data);
    hal_space_map(task->space, align_down(addr, PAGE_SIZE), align_down(addr, PAGE_SIZE), align_up(mods.capacity * sizeof(Module), PAGE_SIZE), MU_MEM_USER | MU_MEM_READ);

    task->context.regs.rdi = addr;
    task->context.regs.rsi = mods.length;
}

int _start()
{
    debug_info("Hello from µ !");
    hal_parse_handover();
    hal_init();

    HalSpace *vspace;
    assert(hal_space_create(&vspace) == MU_RES_OK, "Couldn't create vspace for bootstrap");
    auto mod = handover_file_find(hal_get_handover(), "/bin/bootstrap");
    MuCap bootstrap = unwrap(elf_parse("/bin/bootstrap", mod.start, (uintptr_t)vspace, (MuArgs){0}));
    passModules((Task *)bootstrap._raw);

    sched_push_task((Task *)bootstrap._raw);
    loop;
}
