#include <mu-core/heap.h>

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

char *strdup(char const *s)
{
    return strndup(s, strlen(s));
}

char *strndup(char const *s, usize n)
{
    Alloc heap = heap_acquire();
    char *ret = unwrap_or(heap.calloc(&heap, 1, n), NULL);
    heap.release(&heap);

    if (ret == NULL)
    {
        return ret;
    }

    memcpy(ret, s, n);
    return ret;
}