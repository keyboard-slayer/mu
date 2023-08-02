#include <mu-api/api.h>
#include <mu-debug/debug.h>

#include "mman.h"

void *mmap(void *addr, size_t len, int prot, int flags, maybe_unused int fd, size_t offset)
{
    uintptr_t ptr;
    MuCap vmo;

    // === REMOVE ME === //
    MuCap self;
    if (mu_self(&self) != MU_RES_OK)
    {
        debug_warn("Couldn't get task information");
        return NULL;
    }

    MuCap space = ((MuTask *)self._raw)->space;
    // ================= //

    if (flags & MAP_PRIVATE)
    {
        debug_warn("Not implemented: mmap() with MAP_PRIVATE");
        return NULL;
    }
    else if (flags & MAP_SHARED)
    {
        if (mu_create_vmo(&vmo, 0, len, MU_MEM_LOW) != MU_RES_OK)
        {
            debug_warn("Couldn't create VMO");
            return NULL;
        }
    }

    if (!(flags & MAP_ANON))
    {
        // TODO: free create VMO
        debug_warn("Not implemented: mmap() with MAP_ANON not set");
        return NULL;
    }

    if (flags & MAP_FIXED)
    {
        ptr = (uintptr_t)addr;
        prot |= MU_MEM_NO_ALLOC;
    }

    if (mu_map(space, vmo, &ptr, offset, len, prot) != MU_RES_OK)
    {
        debug_warn("mu_map failed");
        return NULL;
    }

    memset((void *)ptr, 0, len);
    return (void *)ptr;
}