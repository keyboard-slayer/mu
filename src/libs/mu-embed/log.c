#include <mu-api/api.h>

#include "log.h"

static void dummy_lock(unused Writer *self)
{
    return;
}

static void putc(unused Writer *self, char c)
{
    char buf[2] = {c, '\0'};
    mu_log(buf, 1);
}

Writer embed_acquire_writer(void)
{
    return (Writer){
        .putc = putc,
        .release = dummy_lock,
        .puts = generic_puts,
    };
}