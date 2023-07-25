#pragma once

#include <stddef.h>

void *memcpy(void *restrict s1, const void *restrict s2, size_t n);

void *memset(void *s, int c, size_t n);

char *strrchr(const char *s, int c);

size_t strlen(const char *s);

int memcmp(const void *s1, const void *s2, size_t n);

int strcmp(char const *s1, char const *s2);