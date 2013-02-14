#ifndef __RETRO_H__
#define __RETRO_H__

#include "RC_Debug.h"

#include "RC_Engine.h"
#include "RC_Console.h"
#include "RC_Resources.h"
#include "RC_Lua.h"
#include "RC_Render.h"
#include "RC_Input.h"

#include <SDL/SDL.h>
#include <SDL/SDL_ttf.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define VERSION "0.1.6a"


typedef struct
{
    Uint64 new_time;
    Uint64 old_time;
    Uint64 next_tick;
    double time_scale;
    Uint32 delta_time;
}Retro_Clock;

typedef struct
{
    SDL_Surface *mmap;
    SDL_Rect    *m_rect;
    SDL_Rect    *c_rect;
    Uint32 offset_x;
    Uint32 offset_y;
    Uint32 s_wall;
    Uint32 s_flr;
    Uint32 u_seen;
    Uint8  c_size;
}Retro_MMap;

extern void Retro_ConsoleEventHandler(ConsoleInformation *cl, char *cmd);
extern void Retro_EventHandler(SDL_Event *event);
extern int Retro_init(void);
extern void Retro_cleanup(void);
extern void Retro_update(Uint32 dtime);
extern void Retro_render(void);
extern int Retro_exec(void);
extern void Retro_MiniMap(void);
extern void Retro_MiniMapInit(void);


extern int _debug_mode;
extern int __opengl;
extern int _engine_running;
extern int _dev_console_active;
extern double _FPS;
extern RC_RenderData *_render_data;
extern RC_PlayerCamera *_pcam;
extern RC_InputControls _input;
extern RC_Map *_world_map;
extern ConsoleInformation  *_dev_console;
extern lua_State *RC_L;

extern Retro_Clock r_clock;
extern Retro_MMap r_mmap;
#endif
