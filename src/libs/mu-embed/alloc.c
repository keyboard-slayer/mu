#include <mu-api/api.h>

#include "alloc.h"

MaybeAllocObj embed_alloc(usize size)
{
    MuCap ptr;
    MuCap task;

    // TODO FIX ME space == nullptr
    // clang-format off
    
    if (mu_create_vmo(&ptr, 0, align_up(size, PAGE_SIZE), MU_MEM_LOW) != MU_RES_OK \
        || mu_self((MuCap *)&task) != MU_RES_OK \
        || mu_map(((MuTask *)task._raw)->space, ptr, ptr._raw, 0, align_up(size, PAGE_SIZE), MU_MEM_USER | MU_MEM_READ | MU_MEM_WRITE) != MU_RES_OK)
    {
        return None(MaybeAllocObj);
    }
    // clang-format on

    auto obj = (AllocObj){
        .ptr = ptr._raw,
        .size = size,
    };

    return Some(MaybeAllocObj, obj);
}

void embed_free(AllocObj *self)
{
    MuTask task;

    mu_self((MuCap *)&task);

    // TODO mu_close

    mu_unmap(task.space, self->ptr, self->size);
}