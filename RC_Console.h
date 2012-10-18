#ifndef __RCCONSOLE_H__
#define __RCCONSOLE_H__

#include "RC_GlobalDefs.h"

#include <SDL_console/SDL_console.h>

extern ConsoleInformation *RC_ConsoleInit(const char *font_name, SDL_Surface *display_screen, int lines, SDL_Rect rect);
extern void RC_ConsoleDraw(ConsoleInformation *console);


#endif
