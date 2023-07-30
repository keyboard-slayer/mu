#include <handover/utils.h>
#include <mu-debug/debug.h>
#include <mu-hal/hal.h>
#include <mu-mem/heap.h>
#include <pico-ds/vec.h>
#include <specs/elf.h>

#include "const.h"
#include "handover/handover.h"
#include "sched.h"

int _start()
{
    debug_info("Hello from µ !");
    hal_parse_handover();
    hal_init();

    HalSpace *vspace;
    assert(hal_space_create(&vspace) == MU_RES_OK, "Couldn't create vspace for bootstrap");
    auto mod = handover_file_find(hal_get_handover(), "/bin/bootstrap");

    if (mod.size == 0)
    {
        panic("Couldn't find bootstrap");
    }

    Task *bootstrap = unwrap(elf_parse("/bin/bootstrap", mod.start, (uintptr_t)vspace, (MuArgs){0}));
    HandoverPayload *handover = hal_get_handover();
    usize handover_size = kib(16);

    Alloc heap = heap_acquire();
    HandoverPayload *handover_copy = unwrap(heap.malloc(&heap, handover_size));
    uintptr_t handover_addr = hal_mmap_upper_to_lower((uintptr_t)handover_copy);
    heap.release(&heap);

    memcpy(handover_copy, handover, handover_size);
    handover_copy->magic = 0xB00B1E5;

    if (hal_space_map(bootstrap->space, align_down(handover_addr, PAGE_SIZE),
                      align_down(handover_addr, PAGE_SIZE), hal_get_handover_size() + PAGE_SIZE, MU_MEM_USER | MU_MEM_READ) != MU_RES_OK)
    {
        panic("Couldn't map handover to bootstrap process");
    }

    for (size_t i = 0; i < handover_copy->count; i++)
    {
        HandoverRecord *rec = &handover_copy->records[i];
        if (rec->tag == HANDOVER_FILE)
        {
            rec->start = hal_mmap_upper_to_lower(rec->start);
            if (hal_space_map(bootstrap->space, align_down(rec->start, PAGE_SIZE), align_down(rec->start, PAGE_SIZE),
                              align_up(rec->size, PAGE_SIZE), MU_MEM_USER | MU_MEM_READ) != MU_RES_OK)
            {
                panic("Couldn't map file to bootstrap process");
            }
        }
    }

    bootstrap->context.regs.rdi = (uintptr_t)handover_addr;

    sched_push_task(bootstrap);
    loop;
}
