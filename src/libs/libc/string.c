#include <mu-mem/heap.h>

usize strlen(const char *s)
{
    usize len = 0;

    while (*s++)
    {
        len++;
    }

    return len;
}

void *memset(void *s, int c, usize n)
{
    char *p = s;

    while (n--)
    {
        *p++ = c;
    }

    return s;
}

void *memcpy(void *dest, const void *src, usize n)
{
    char *d = dest;
    const char *s = src;

    while (n--)
    {
        *d++ = *s++;
    }

    return dest;
}

int memcmp(const void *s1, const void *s2, usize n)
{
    for (usize i = 0; i < n; i++)
    {
        if (((char *)s1)[i] != ((char *)s2)[i])
        {
            return ((char *)s1)[i] - ((char *)s2)[i];
        }
    }

    return 0;
}

int strncmp(cstr s1, cstr s2, usize n)
{
    for (usize i = 0; i < n; i++)
    {
        if (s1[i] != s2[i])
        {
            return s1[i] - s2[i];
        }
    }

    return 0;
}

int strcmp(cstr s1, cstr s2)
{
    return strncmp(s1, s2, strlen(s1));
}