#include <misc/macro.h>
#include <munix-api/api.h>

static void puts(char const *s)
{
    size_t len = 0;
    while (s[len])
        len++;
    mu_log(s, len);
}

noreturn int _start(void)
{
    puts("Hello, World !");
    puts("Hello, World 2 !");
    loop;
    unreachable;
}