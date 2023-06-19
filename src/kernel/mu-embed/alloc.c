#include <mu-core/pmm.h>
#include <mu-embed/alloc.h>
#include <mu-hal/hal.h>

MaybeAllocObj embed_alloc(usize size)
{
    Pmm pmm = pmm_acquire();
    PmmObj res = Try(MaybeAllocObj, pmm.calloc(1, size, false));
    pmm.release(&pmm);

    AllocObj obj = (AllocObj){
        .ptr = hal_mmap_lower_to_upper(res.ptr),
        .size = size,
    };

    return Some(MaybeAllocObj, obj);
}

void embed_free(AllocObj *self)
{
    Pmm pmm = pmm_acquire();

    PmmObj obj = {
        .ptr = self->ptr,
        .len = self->size,
    };

    pmm.free(obj);
    pmm.release(&pmm);
}