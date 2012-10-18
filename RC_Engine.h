#ifndef __RCENGINE_H__
#define __RCENGINE_H__

#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include <SDL/SDL_ttf.h>

#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

#include "RC_GlobalDefs.h"
#include "RC_Render.h"
#include "RC_Resources.h"
#include "RC_Lua.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#define ENGINE_CONFIG_FILE "luascripts/engine_init.lua"



extern RC_RenderData* RC_Init(lua_State *L, Uint32 flags);
extern RC_PlayerCamera* RC_InitCamera(double x, double y, double m_speed, double r_speed);
extern void RC_ResetVideoMode(RC_RenderData *rc_r, Uint32 flags);
#endif
