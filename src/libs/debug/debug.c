#include "debug.h"
#include <stdarg.h>
#include <traits/output.h>

#include "__stb_sprintf.h"

static AcquireOutput acquire = NULL;
static char const *event_header[DEBUG_EVENT_LENGTH] = {
    NULL,
    "[ INFO  ]",
    "[ WARN  ]",
    "[ ERROR ]",
    "[ OK    ]",
};

static char const *event_colors[DEBUG_EVENT_LENGTH] = {
    NULL,
    "\033[34m",
    "\033[33m",
    "\033[31m",
    "\033[32m",
};

void debug_set_acquire_function(AcquireOutput func)
{
    acquire = func;
}

void __debug_impl(const char *filename, size_t lineno, DebugEvent event, const char *fmt, ...)
{
    char buffer[1024] = {0};
    va_list ap;
    Output out = acquire();

    if (event != DEBUG_NONE)
    {
        out.puts(&out, event_colors[event], strlen(event_colors[event]));
        out.puts(&out, event_header[event], strlen(event_header[event]));
        out.putc(&out, ' ');
        memcpy(buffer, filename, strlen(filename));
        char *ptr = strrchr(buffer, '.');

        if (ptr)
        {
            *ptr = '\0';
        }

        out.puts(&out, buffer, strlen(buffer));

        stbsp_snprintf(buffer, 1024, ":%ld ", lineno);
        out.puts(&out, buffer, strlen(buffer));
        out.puts(&out, "\033[0m", 4);
    }

    va_start(ap, fmt);
    stbsp_vsnprintf(buffer, 1024, fmt, ap);
    out.puts(&out, buffer, strlen(buffer));
    va_end(ap);

    out.putc(&out, '\n');
    out.release(&out);
}