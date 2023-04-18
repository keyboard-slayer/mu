#include <handover/utils.h>
#include <mu-base/std.h>
#include <mu-hal/hal.h>
#include <mu-misc/lock.h>
#include <mu-traits/alloc.h>

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

static void *pmm_inner(usize pages, bool high)
{
    usize page_start_index;
    void *ret;
    usize size = 0;

    usize *start = high ? &bitmap.last_used_high : &bitmap.last_used_low;
    usize end = high ? 0 : bitmap.size;

    while (*start < end)
    {
        if (!bitmap_is_bit_set(high ? (*start)-- : (*start)++))
        {
            if (++size == pages)
            {
                page_start_index = *start - pages;
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

MaybePtr pmm_alloc_page(usize pages, bool high)
{
    void *ret = pmm_inner(pages, high);

    if (ret == NULL)
    {
        bitmap.last_used_low = 0;
        ret = pmm_inner(pages, high);
    }

    if (ret == NULL)
    {
        return None(MaybePtr);
    }

    return Some(MaybePtr, ret);
}

MaybePmmObj pmm_alloc(usize size)
{
    usize pages = align_up(size, PAGE_SIZE) / PAGE_SIZE;
    void *ptr = Try(MaybePmmObj, pmm_alloc_page(pages, false));
    PmmObj obj = {
        .ptr = (uintptr_t)ptr,
        .len = size,
    };

    return Some(MaybePmmObj, obj);
}

void pmm_free(PmmObj *obj)
{
    usize base = align_down((uintptr_t)obj->ptr, PAGE_SIZE) / PAGE_SIZE;
    usize pages = align_up(obj->len, PAGE_SIZE) / PAGE_SIZE;
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
    bitmap.last_used_high = bitmap.size - 1;

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

MaybePmmObj pmm_calloc(usize nmemb, usize size)
{
    PmmObj ptr = Try(MaybePmmObj, pmm_alloc(nmemb * size));
    memset((void *)hal_mmap_lower_to_upper(ptr.ptr), 0, nmemb * size);
    return Some(MaybePmmObj, ptr);
}

void pmm_release(Pmm *self)
{
    *self = (Pmm){0};
    spinlock_release(&lock);
}

void _pmm_free(PmmObj obj)
{
    pmm_free(&obj);
}

Pmm pmm_acquire(void)
{
    spinlock_acquire(&lock);
    return (Pmm){
        .malloc = pmm_alloc,
        .free = _pmm_free,
        .release = pmm_release,
        .calloc = pmm_calloc,
    };
}

usize pmm_available_pages(void)
{
    return available_pages;
}