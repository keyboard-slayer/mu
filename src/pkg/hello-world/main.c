#include <misc/macro.h>
#include <munix-api/api.h>

#include "debug/debug.h"

static void puts(char const *s)
{
    size_t len = 0;
    while (s[len++])
        ;
    mu_log(s, len);
}

noreturn int _start(void)
{
    (void)puts;
    // __asm__ volatile("int $1");
    // puts("Hello, World !");
    // puts("Hello, World 2 !");
    loop;
    unreachable;
}