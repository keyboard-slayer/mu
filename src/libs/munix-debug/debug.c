#include <pico-misc/types.h>
#include <pico-traits/writer.h>

#include "debug.h"
#include "mu-api/api.h"

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

static void logger_puts(unused Writer *writer, char const *s)
{
    mu_log(s, strlen(s));
}

static void logger_putc(Writer *writer, char c)
{
    char buf[2] = {c, '\0'};
    logger_puts(writer, buf);
}

void __debug_impl(const char *filename, usize lineno, DebugEvent event, const char *fmt, FmtArgs args)
{
    MuCap self;
    int64_t tid = -1;

    if (mu_self(&self) == MU_RES_OK)
    {
        tid = ((MuTask *)self._raw)->tid;
    }

    Writer writer = {
        .putc = logger_putc,
        .puts = logger_puts,
    };

    if (event != DEBUG_NONE)
    {
        fmt(&writer, "{}{} pid: {} - {}:{} \033[0m", event_colors[event], event_header[event], tid, filename, lineno);
    }

    fmt_impl(&writer, fmt, args);

    writer.putc(&writer, '\n');

    if (event == DEBUG_PANIC)
    {
        mu_exit(1);
    }
}