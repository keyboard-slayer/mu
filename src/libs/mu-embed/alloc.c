#include <mu-api/api.h>

#include "alloc.h"

MaybePtr embed_alloc(usize size)
{
    MuCap ptr;
    MuTask task;

    mu_create_vmo(&ptr, 0, size, MU_MEM_HIGH);
    mu_self((MuCap *)&task);

    if (ptr._raw == 0)
    {
        return None(MaybePtr);
    }

    mu_map(task.space, ptr, ptr._raw, 0, size, MU_MEM_USER | MU_MEM_READ | MU_MEM_WRITE);
    return Some(MaybePtr, (void *)ptr._raw);
}

void embed_free(void *ptr, usize size)
{
    MuTask task;

    mu_self((MuCap *)&task);

    mu_unmap(task.space, (uintptr_t)ptr, size);
}