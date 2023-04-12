#include <mu-core/pmm.h>
#include <mu-embed/alloc.h>

MaybePtr embed_alloc(usize size)
{
    Alloc pmm = pmm_acquire();
    MaybePtr res = pmm.calloc(&pmm, 1, size);
    pmm.release(&pmm);

    return res;
}

void embed_free(void *ptr, usize size)
{
    Alloc pmm = pmm_acquire();
    pmm.free(&pmm, ptr, size);
    pmm.release(&pmm);
}