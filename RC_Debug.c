#include "RC_Debug.h"

ConsoleInformation *_dbg_console;

void
RC_SetDebugConsole(ConsoleInformation *cl)
{
    if(cl)
    {
        _dbg_console = cl;
        RC_ConsoleDebugOut(_dbg_console, "Debug Console Initialized...");
    }
}
void
RC_ConsoleDebugOut(ConsoleInformation *cl, const char *fmt, ...)
{
    char buffer[256]; //maybe enough
    va_list args;
    va_start(args, fmt);
    vsprintf(buffer, fmt, args);
    va_end(args);

    if(cl)
    {
        CON_Out(cl, buffer);
    }
    else
    {
        CON_Out(_dbg_console, buffer);
    }
}
