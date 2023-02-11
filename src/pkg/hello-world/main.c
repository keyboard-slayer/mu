#include <libc/unistd.h>
#include <misc/macro.h>

noreturn int _start(void)
{
    syscall(0);
    loop;
    unreachable
}