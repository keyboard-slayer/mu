#include <libc/unistd.h>
#include <misc/macro.h>

noreturn int _start(void)
{
    int a = 9;
    int b = a - 9;
    unused int c = a / b;

    syscall(0);
    loop;
    unreachable
}