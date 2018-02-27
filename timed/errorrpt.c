#ifdef __NT__

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#endif

#include <stdarg.h>

#include "includes.h"

void debug(const char *format, ...)
{
    va_list args;
    char buffer[2048];
    va_start(args, format);
    vsprintf(buffer, format, args);
    va_end(args);
#ifdef __NT__
    OutputDebugString(buffer);
#else
    fprintf(stderr, "%s\n", buffer);
#endif
}

void showerror(void)
{
    if (msgapierr == 0)
        return;

    sprintf(msg, "Extended info: %s", errmsgs[msgapierr]);

    Message(msg, -1, 0, YES);
}
