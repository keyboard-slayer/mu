#pragma once

#include <libc/string.h>

#define unused __attribute__((unused))
#define packed __attribute__((packed))

#if defined(HOST_IS_WINDOWS)
#    define __FILENAME__ (strrchr(__FILE__, '\\') ? strrchr(__FILE__, '\\') + 1 : __FILE__)
#else
#    define __FILENAME__ (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)
#endif
