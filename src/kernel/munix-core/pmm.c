#include <debug/debug.h>
#include <handover/utils.h>
#include <misc/lock.h>
#include <munix-hal/hal.h>
#include <stdbool.h>
#include <traits/alloc.h>

#include "const.h"
#include "pmm.h"

static Spinlock lock = {0};
static size_t available_pages = 0;
static PmmBitmap bitmap = {0};

static void pmm_unset(uintptr_t base, size_t length)
{
    for (size_t i = 0; i < length; i++)
    {
        bitmap.bitmap[(i + base) / 8] &= ~(1 << ((i + base) % 8));
        available_pages++;
    }
}

static int bitmap_is_bit_set(size_t index)
{
    return bitmap.bitmap[index / 8] & (1 << (index % 8));
}

static void pmm_set_used(uint64_t base, uint64_t length)
{
    for (size_t i = 0; i < length; i++)
    {
        bitmap.bitmap[(i + base) / 8] |= (1 << ((i + base) % 8));
        available_pages--;
    }
}

static void *pmm_inner(size_t pages)
{
    size_t page_start_index;
    void *ret;
    size_t size = 0;

    while (bitmap.last_used < bitmap.size)
    {
        if (!bitmap_is_bit_set(bitmap.last_used++))
        {
            if (++size == pages)
            {
                page_start_index = bitmap.last_used - pages;
                pmm_set_used(page_start_index, pages);

                ret = (void *)(page_start_index * PAGE_SIZE);
                return ret;
            }
        }
        else
        {
            size = 0;
        }
    }

    return NULL;
}

static void *pmm_alloc_page(size_t pages)
{
    void *ret = pmm_inner(pages);

    if (ret == NULL)
    {
        bitmap.last_used = 0;
        ret = pmm_inner(pages);
    }

    if (ret == NULL)
    {
        return NULL;
    }

    return ret;
}

void *pmm_alloc(unused Alloc *self, size_t size)
{
    size_t pages = align_up(size, PAGE_SIZE) / PAGE_SIZE;
    return pmm_alloc_page(pages);
}

void pmm_free(unused Alloc *self, void *ptr, size_t size)
{
    size_t base = align_down((uintptr_t)ptr, PAGE_SIZE) / PAGE_SIZE;
    size_t pages = align_up(size, PAGE_SIZE) / PAGE_SIZE;
    pmm_unset(base, pages);
}

void pmm_init(void)
{
    HandoverPayload *handover = hal_get_handover();
    HandoverRecord last_entry = handover->records[handover->count - 2];
    HandoverRecord record;

    bitmap.size = align_up((last_entry.start + last_entry.size) / (PAGE_SIZE * 8), PAGE_SIZE);
    debug(DEBUG_INFO, "Bitmap size: 0x%x", bitmap.size);

    handover_foreach_record(handover, record)
    {
        if (record.tag == HANDOVER_FREE && record.size >= bitmap.size)
        {
            bitmap.bitmap = (void *)(hal_mmap_lower_to_upper(record.start));
            record.start += bitmap.size;
            record.size -= bitmap.size;
        }
    }

    if (bitmap.bitmap == NULL)
    {
        debug(DEBUG_ERROR, "No usable memory for bitmap");
        debug_raise_exception();
    }

    memset(bitmap.bitmap, 0xff, bitmap.size);

    handover_foreach_record(handover, record)
    {
        record = handover->records[i];

        if (record.tag == HANDOVER_FREE)
        {
            pmm_unset(align_down(record.start, PAGE_SIZE) / PAGE_SIZE, align_up(record.size, PAGE_SIZE) / PAGE_SIZE);
        }
    }

    debug(DEBUG_INFO, "Available pages: 0x%x", available_pages);
    debug(DEBUG_INFO, "PMM initialized");
}

void *pmm_calloc(Alloc *self, size_t nmemb, size_t size)
{
    void *ptr = self->malloc(self, nmemb * size);

    if (ptr != NULL)
    {
        memset((void *)hal_mmap_lower_to_upper((uintptr_t)ptr), 0, nmemb + size);
    }

    return ptr;
}

void pmm_release(Alloc *self)
{
    *self = (Alloc){0};
    spinlock_release(&lock);
}

Alloc pmm_acquire(void)
{
    spinlock_acquire(&lock);
    return (Alloc){
        .malloc = pmm_alloc,
        .free = pmm_free,
        .release = pmm_release,
        .calloc = pmm_calloc,
        .realloc = NULL,
    };
}

size_t pmm_available_pages(void)
{
    return available_pages;
}