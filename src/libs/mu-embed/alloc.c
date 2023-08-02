#include <mu-api/api.h>
#include <mu-debug/debug.h>
#include <pico-misc/macro.h>
#include <sys/mman.h>

#include "alloc.h"

MaybeAllocObj embed_alloc(usize size)
{
    // FIXME: change to MAP_PRIVATE
    void *ptr = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANON, -1, 0);

    if (ptr == NULL)
    {
        return None(MaybeAllocObj);
    }

    AllocObj obj = (AllocObj){
        .ptr = (uintptr_t)ptr,
        .size = size,
    };

    return Some(MaybeAllocObj, obj);
}

void embed_free(AllocObj *self)
{
    MuTask task;

    mu_self((MuCap *)&task);

    // TODO munmap

    mu_unmap(task.space, self->ptr, self->size);
}