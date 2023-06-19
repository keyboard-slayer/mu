#include <mu-api/api.h>
#include <mu-embed/log.h>
#include <mu-embed/misc.h>
#include <pico-fmt/fmt.h>
#include <stdarg.h>

#include "debug.h"

static cstr event_header[DEBUG_EVENT_LENGTH] = {
    NULL,
    "[ INFO  ]",
    "[ WARN  ]",
    "[ PANIC ]",
};

static cstr event_colors[DEBUG_EVENT_LENGTH] = {
    NULL,
    "\033[34m",
    "\033[33m",
    "\033[31m",
};

void __debug_impl(const char *filename, usize lineno, DebugEvent event, const char *fmt, FmtArgs args)
{
    Writer writer = embed_acquire_writer();

    if (event != DEBUG_NONE)
    {
        fmt(&writer, "{}{} {}:{} \033[0m", event_colors[event], event_header[event], filename, lineno);
    }

    fmt_impl(&writer, fmt, args);
    writer.putc(&writer, '\n');

    writer.release(&writer);

    if (event == DEBUG_PANIC)
    {
        embed_abort();
    }
}
