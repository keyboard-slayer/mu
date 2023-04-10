#pragma once

#define unused __attribute__((unused))
#define packed __attribute__((packed))

#ifndef unreachable
#    define unreachable() __builtin_unreachable();
#endif /* !unreachable */

#define noreturn __attribute__((noreturn))
#define weak     __attribute__((weak))
#define loop     for (;;)

#define kib(x) ((uintptr_t)(x)*1024)
#define mib(x) (kib(x) * 1024)
#define gib(x) (mib(x) * 1024)

#define align_up(x, align)   (((x) + (align)-1) & ~((align)-1))
#define align_down(x, align) ((x) & ~((align)-1))

#define __FILENAME__ (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)

#define irq(n)    (n + 32)
#define max(a, b) ((a) > (b) ? (a) : (b))
#define min(a, b) ((a) < (b) ? (a) : (b))
