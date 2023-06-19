#include <libheap/libheap.h>
#include <mu-embed/alloc.h>
#include <mu-hal/hal.h>
#include <mu-mem/heap.h>
#include <mu-misc/lock.h>
#include <pico-misc/macro.h>

static Spinlock lock = {0};

static void *alloc_block(unused void *ctx, usize size)
{
    auto ptr = unwrap_or(embed_alloc(size), NULL);
    return (void *)ptr.ptr;
}

static void free_block(unused void *ctx, void *ptr, usize size)
{
    auto obj = (AllocObj){.ptr = (uintptr_t)ptr, .size = size};
    embed_free(&obj);
}

static void hook_log(unused void *ctx, unused enum HeapLogType type, unused const char *msg, unused va_list args)
{
    return;
}

static struct Heap heap_impl = (struct Heap){
    .alloc = alloc_block,
    .free = free_block,
    .log = hook_log,
};

static MaybePtr _heap_alloc(unused Alloc *self, usize size)
{
    void *res = heap_alloc(&heap_impl, size);

    if (!res)
    {
        return None(MaybePtr);
    }

    return Some(MaybePtr, res);
}

static void _heap_free(unused Alloc *self, void *ptr, unused usize size)
{
    heap_free(&heap_impl, ptr);
}

static MaybePtr _heap_calloc(unused Alloc *self, usize nmemb, usize size)
{
    void *res = heap_calloc(&heap_impl, nmemb, size);

    if (!res)
    {
        return None(MaybePtr);
    }

    return Some(MaybePtr, res);
}

static MaybePtr _heap_realloc(unused Alloc *self, void *ptr, usize size)
{
    void *res = heap_realloc(&heap_impl, ptr, size);

    if (!res)
    {
        return None(MaybePtr);
    }

    return Some(MaybePtr, res);
}

void heap_release(Alloc *alloc)
{
    spinlock_release(&lock);
    memset(alloc, 0, sizeof(Alloc));
}

Alloc heap_acquire(void)
{
    spinlock_acquire(&lock);
    return (Alloc){
        .malloc = _heap_alloc,
        .realloc = _heap_realloc,
        .calloc = _heap_calloc,
        .free = _heap_free,
        .release = heap_release,
    };
}
