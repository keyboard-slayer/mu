#pragma once

#include <mu-base/std.h>

MaybePtr embed_alloc(usize size);
void embed_free(void *ptr, usize size);