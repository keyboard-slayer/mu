#include <mu-mem/heap.h>

void *_json_realloc(void *ptr, usize size)
{
    Alloc heap = heap_acquire();
    void *res = unwrap_or(heap.realloc(&heap, ptr, size), NULL);
    heap.release(&heap);

    return res;
}

void _json_free(void *ptr, size_t size)
{
    Alloc heap = heap_acquire();
    heap.free(&heap, ptr, size);
    heap.release(&heap);
}