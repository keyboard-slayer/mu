#include <misc/macro.h>
#include <munix-api/api.h>

#include "debug/debug.h"

static void puts(char const *s)
{
    (void)s;
    size_t len = 0;
    while (s[len++])
        ;
    mu_log(s, len);
}

noreturn int _start(void)
{
    for (char c = 'a'; c <= 'z'; c++)
    {
        char s[2] = {c, '\0'};
        puts(s);
    }

    loop;
    unreachable;
}