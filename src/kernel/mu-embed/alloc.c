#include <mu-core/pmm.h>
#include <mu-embed/alloc.h>

MaybePtr embed_alloc(usize size)
{
    Pmm pmm = pmm_acquire();
    PmmObj res = Try(MaybePtr, pmm.calloc(1, size));
    pmm.release(&pmm);

    return Some(MaybePtr, (void *)res.ptr);
}

void embed_free(void *ptr, usize size)
{
    Pmm pmm = pmm_acquire();

    PmmObj obj = {
        .ptr = (uintptr_t)ptr,
        .len = size};

    pmm.free(obj);
    pmm.release(&pmm);
}