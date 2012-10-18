#ifndef RC_INPUT_H_INCLUDED
#define RC_INPUT_H_INCLUDED

#include <SDL/SDL.h>
#include <SDL/SDL_mouse.h>
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

#include "RC_GlobalDefs.h"
#include "RC_Debug.h"
#include "RC_Lua.h"

#define INPUT_HANDLER_FILE "luascripts/rc_input.lua"


typedef struct
{
    int     kup;
    int     kdn;
    int     klf;
    int     krt;
    int     kup_state;
    int     kdn_state;
    int     klf_state;
    int     krt_state;
    int     m_x;
    int     m_y;
    int     m_relx;
    int     m_rely;
    Uint8   m_btn_state;
}RC_InputControls;


//extern void RC_InputInit(lua_State *L);
extern void RC_ScanKeyboardInput(SDL_Event *e, RC_InputControls *rc_i);
extern void RC_ScanMouseInput(SDL_Event *e, RC_InputControls *rc_i);
#endif // RC_INPUT_H_INCLUDED
