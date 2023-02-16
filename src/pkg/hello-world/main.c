#include <libc/unistd.h>
#include <misc/macro.h>

static void puts(char *s)
{
    syscall(0, (uintptr_t)s);
}

noreturn int _start(void)
{
    puts("Hello, World !");
    puts("Hello, World 2 !");
    loop;
    unreachable;
}