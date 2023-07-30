#pragma once

#include <mu-api/api.h>
#include <specs/elf.h>

typedef struct
{
    MuCap vspace;
} ElfOptionalParams;

typedef struct
{
    MuCap task;
    uintptr_t entry;
} Maybe$(ElfReturn);

MaybeElfReturn elf_parse_(char const name[static 1], uintptr_t elf_ptr, ElfOptionalParams params[static 1]);

#define elf_parse(name, elf_ptr, ...)                                                   \
    _Pragma("GCC diagnostic push")                                                      \
        _Pragma("GCC diagnostic ignored \"-Winitializer-overrides\"")                   \
            elf_parse_(name, elf_ptr, &(ElfOptionalParams){.vspace = {0}, __VA_ARGS__}) \
                _Pragma("GCC diagnostic pop")
