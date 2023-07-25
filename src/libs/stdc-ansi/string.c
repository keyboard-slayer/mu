#include "string.h"

void *memcpy(void *restrict s1, const void *restrict s2, size_t n)
{
    char *restrict d = (char *)s1;
    const char *restrict s = s2;

    while (n--)
    {
        *d++ = *s++;
    }

    return s1;
}

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


int memcmp(const void *s1, const void *s2, size_t n)
{
    for (size_t i = 0; i < n; i++)
    {
        if (((char *)s1)[i] != ((char *)s2)[i])
        {
            return ((char *)s1)[i] - ((char *)s2)[i];
        }
    }

    return 0;
}


int strcmp(char const *s1, char const *s2)
{
    return memcmp(s1, s2, strlen(s1));
}