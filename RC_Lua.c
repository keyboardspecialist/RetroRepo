#include "RC_Lua.h"

void
RC_LuaInit(lua_State **L)
{
    *L = luaL_newstate();
    luaL_openlibs(*L);
}

void
RC_LuaClose(lua_State *L)
{
    lua_close(L);
}

void
RC_LuaError(lua_State *L, const char *fmt, ...)
{
    va_list argp;
    va_start(argp, fmt);
    vfprintf(stderr, fmt, argp);
    va_end(argp);
    lua_close(L);
    exit(EXIT_FAILURE);
}


