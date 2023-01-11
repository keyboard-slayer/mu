#pragma once

#include <libc/string.h>

#define unused __attribute__((unused))

#define __FILENAME__ (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)