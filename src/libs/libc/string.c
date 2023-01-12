#include <stddef.h>

#include "string.h"

char *strrchr(const char *s, int c)
{
    char *p = NULL;

    do
    {
        if (*s == c)
        {
            p = (char *)s;
        }
    } while (*s++);

    return p;
}

size_t strlen(const char *s)
{
    size_t len = 0;

    while (*s++)
    {
        len++;
    }

    return len;
}

void *memset(void *s, int c, size_t n)
{
    char *p = s;

    while (n--)
    {
        *p++ = c;
    }

    return s;
}

void *memcpy(void *dest, const void *src, size_t n)
{
    char *d = dest;
    const char *s = src;

    while (n--)
    {
        *d++ = *s++;
    }

    return dest;
}