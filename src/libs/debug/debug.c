#include <mu-hal/hal.h>
#include <stdarg.h>

#define STB_SPRINTF_NOFLOAT
#define STB_SPRINTF_IMPLEMENTATION
#define STB_SPRINTF_NOUNALIGNED
#include "__stb_sprintf.h"

#include "debug.h"

static char const *event_header[DEBUG_EVENT_LENGTH] = {
    NULL,
    "[ INFO  ]",
    "[ WARN  ]",
    "[ PANIC ]",
};

static char const *event_colors[DEBUG_EVENT_LENGTH] = {
    NULL,
    "\033[34m",
    "\033[33m",
    "\033[31m",
};

void __debug_impl(const char *filename, size_t lineno, DebugEvent event, const char *fmt, ...)
{
    va_list ap;

    hal_serial_acquire();
    static char buffer[1024] = {0};

    if (event != DEBUG_NONE)
    {
        hal_serial_write(event_colors[event], strlen(event_colors[event]));
        hal_serial_write(event_header[event], strlen(event_header[event]));
        hal_serial_write(" ", 1);
        memcpy(buffer, filename, strlen(filename));
        char *ptr = strrchr(buffer, '.');

        if (ptr)
        {
            *ptr = '\0';
        }

        hal_serial_write(buffer, strlen(buffer));

        stbsp_snprintf(buffer, 1024, ":%ld ", lineno);

        hal_serial_write(buffer, strlen(buffer));
        hal_serial_write("\033[0m", 4);
    }

    va_start(ap, fmt);
    stbsp_vsnprintf(buffer, 1024, fmt, ap);
    hal_serial_write(buffer, strlen(buffer));
    va_end(ap);
    hal_serial_write("\n", 1);

    hal_serial_release();

    if (event == DEBUG_PANIC)
    {
        hal_cpu_stop();
    }
}
