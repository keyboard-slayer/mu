#include <handover/utils.h>
#include <mu-base/std.h>
#include <mu-hal/hal.h>
#include <mu-misc/lock.h>
#include <mu-traits/alloc.h>
#include <stdbool.h>

#include "const.h"
#include "pmm.h"

static Spinlock lock = {0};
static usize available_pages = 0;
static PmmBitmap bitmap = {0};

static void pmm_unset(uintptr_t base, usize length)
{
    for (usize i = 0; i < length; i++)
    {
        bitmap.bitmap[(i + base) / 8] &= ~(1 << ((i + base) % 8));
        available_pages++;
    }
}

static int bitmap_is_bit_set(usize index)
{
    return bitmap.bitmap[index / 8] & (1 << (index % 8));
}

static void pmm_set_used(u64 base, u64 length)
{
    for (usize i = 0; i < length; i++)
    {
        bitmap.bitmap[(i + base) / 8] |= (1 << ((i + base) % 8));
        available_pages--;
    }
}

static void *pmm_inner(usize pages)
{
    usize page_start_index;
    void *ret;
    usize size = 0;

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

static void *pmm_alloc_page(usize pages)
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

MaybePtr pmm_alloc(unused Alloc *self, usize size)
{
    usize pages = align_up(size, PAGE_SIZE) / PAGE_SIZE;
    void *res = pmm_alloc_page(pages);

    if (!res)
    {
        return None(MaybePtr);
    }

    return Just(MaybePtr, res);
}

void pmm_free(unused Alloc *self, void *ptr, usize size)
{
    usize base = align_down((uintptr_t)ptr, PAGE_SIZE) / PAGE_SIZE;
    usize pages = align_up(size, PAGE_SIZE) / PAGE_SIZE;
    pmm_unset(base, pages);
}

void pmm_init(void)
{
    HandoverPayload *handover = hal_get_handover();
    HandoverRecord last_entry = handover->records[handover->count - 1];
    HandoverRecord record;

    bitmap.size = align_up((last_entry.start + last_entry.size) / (PAGE_SIZE * 8), PAGE_SIZE);
    debug_info("Bitmap size: 0x{x}", bitmap.size);

    handover_foreach_record(handover, record)
    {
        if (record.tag == HANDOVER_FREE && record.size >= bitmap.size)
        {
            bitmap.bitmap = (void *)(hal_mmap_lower_to_upper(record.start));
            record.start += bitmap.size;
            record.size -= bitmap.size;
            break;
        }
    }

    if (bitmap.bitmap == NULL)
    {
        panic("No usable memory for bitmap");
    }

    debug_info("Bitmap at: 0x{a}", hal_mmap_upper_to_lower((uintptr_t)bitmap.bitmap));

    memset(bitmap.bitmap, 0xff, bitmap.size);

    handover_foreach_record(handover, record)
    {
        record = handover->records[i];

        if (record.tag == HANDOVER_FREE)
        {
            pmm_unset(align_down(record.start, PAGE_SIZE) / PAGE_SIZE, align_up(record.size, PAGE_SIZE) / PAGE_SIZE);
        }
    }

    debug_info("Available pages: 0x{x}", available_pages);
    debug_info("PMM initialized");
}

MaybePtr pmm_calloc(Alloc *self, usize nmemb, usize size)
{
    void *ptr = Try(MaybePtr, self->malloc(self, nmemb * size));
    memset((void *)hal_mmap_lower_to_upper((uintptr_t)ptr), 0, nmemb * size);

    return Just(MaybePtr, ptr);
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

usize pmm_available_pages(void)
{
    return available_pages;
}