#pragma once

#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <traits/alloc.h>

void vec_expand_(char **data, size_t *length, size_t *capacity, int memsz, AllocAcquireFn alloc_fn);

#define Vec(T)                \
    struct                    \
    {                         \
        T *data;              \
        size_t length;        \
        size_t capacity;      \
        size_t iter;          \
        AllocAcquireFn alloc; \
    }

#define vec_init(v, a) ({memset((v), 0, sizeof(*(v))); (v)->alloc = (a); })

#define vec_push(v, val)                                           \
    vec_expand_((char **)&(v)->data, &(v)->length, &(v)->capacity, \
                sizeof(*(v)->data), (v)->alloc);                   \
    (v)->data[(v)->length++] = (val)

#define vec_foreach(v, t)                                                  \
    if ((v)->length > 0)                                                   \
        for ((v)->iter = 0;                                                \
             (v)->iter < (v)->length && (((t) = (v)->data[(v)->iter]), 1); \
             ++(v)->iter)

#define vec_clear(v) \
    ((v)->length = 0)

#define vec_free(v) ({             \
    Alloc alloc = (v)->alloc();    \
    alloc.free(&alloc, (v)->data); \
    alloc.release(&alloc);         \
})

typedef Vec(char) VecChar;