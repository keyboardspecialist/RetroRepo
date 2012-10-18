#ifndef RC_DEBUG_H_INCLUDED
#define RC_DEBUG_H_INCLUDED

#include "RC_GlobalDefs.h"
#include <SDL_console/SDL_console.h>
#include <stdarg.h>
#include <stdio.h>

extern ConsoleInformation *_dbg_console;

extern void RC_SetDebugConsole(ConsoleInformation *cl);
extern void RC_ConsoleDebugOut(ConsoleInformation *cl, const char *fmt, ...);


#endif // RC_DEBUG_H_INCLUDED
