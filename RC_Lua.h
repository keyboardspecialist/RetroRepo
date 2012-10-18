#ifndef __RCLUA_H__
#define __RCLUA_H__

#include "RC_GlobalDefs.h"
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>


extern void RC_LuaInit(lua_State **L);
extern void RC_LuaClose();
extern void RC_LuaError(lua_State *L, const char *fmt, ...);

#endif
