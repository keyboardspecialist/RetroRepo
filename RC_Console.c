#include "RC_Console.h"

ConsoleInformation*
RC_ConsoleInit(const char *font_name, SDL_Surface *display_screen, int lines, SDL_Rect rect)
{
    ConsoleInformation *ci = CON_Init(font_name, display_screen, lines, rect);
    CON_Position(ci, 0, 0);
    CON_Alpha(ci, 160);
    CON_Topmost(ci);
    CON_SetPrompt(ci, "-->");
    CON_SetHideKey(ci, 96 /*tilde*/);
    return ci;
}

void
RC_ConsoleDraw(ConsoleInformation *console)
{
    CON_DrawConsole(console);
}
