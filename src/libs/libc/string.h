#pragma once

#include <pico-misc/types.h>
#include <string.h>

usize strlen(cstr s);

void *memset(void *s, int c, usize n);

void *memcpy(void *dest, const void *src, usize n);

int memcmp(const void *s1, const void *s2, usize n);

int strcmp(cstr s1, cstr s2);

int strncmp(cstr s1, cstr s2, usize len);