#include <mu-api/api.h>
#include <mu-mem/heap.h>
#include <pico-ds/vec.h>
#include <pico-misc/macro.h>
#include <pico-traits/writer.h>
#include <string.h>

static VecChar buf;

static void release(unused Writer *self)
{
    mu_log(buf.data, buf.length);
    vec_free(&buf);
    return;
}

static void putc(unused Writer *self, char c)
{
    vec_push(&buf, c);
}

Writer embed_acquire_writer(void)
{
    vec_init(&buf, heap_acquire);

    return (Writer){
        .putc = putc,
        .release = release,
        .puts = generic_puts,
    };
}