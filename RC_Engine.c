#include "RC_Engine.h"

RC_RenderData*
RC_Init(lua_State *L, Uint32 flags)
{
    RC_RenderData *rd = malloc(sizeof(RC_RenderData));

    if(luaL_loadfile(L, ENGINE_CONFIG_FILE) || lua_pcall(L, 0, 0, 0))
    {
        RC_LuaError(L, "Couldn't load config file %s\n", lua_tostring(L, -1));
    }
    lua_getglobal(L, "retro_screen_width");
    lua_getglobal(L, "retro_screen_height");
    lua_getglobal(L, "retro_bit_depth");
    lua_getglobal(L, "retro_texture_width");
    lua_getglobal(L, "retro_texture_height");
    lua_getglobal(L, "retro_wall_scale");
    lua_getglobal(L, "retro_version");

    if(!lua_isnumber(L, 1) || !lua_isnumber(L, 2) || !lua_isnumber(L, 3) ||
       !lua_isnumber(L, 4) || !lua_isnumber(L, 5) || !lua_isnumber(L, 6))
        RC_LuaError(L, "Screen initilization values must be numbers\n");

    rd->scr_width =  (Uint32)lua_tonumber(L, 1);
    rd->scr_height = (Uint32)lua_tonumber(L, 2);
    rd->bit_depth =  (Uint32)lua_tonumber(L, 3);
    rd->tex_width =  (Uint32)lua_tonumber(L, 4);
    rd->tex_height = (Uint32)lua_tonumber(L, 5);
    rd->wall_scale = (double)lua_tonumber(L, 6);
    rd->light_rad = 5.0;
    rd->render_light = FALSE;
    rd->screen = SDL_SetVideoMode(rd->scr_width, rd->scr_height, rd->bit_depth, flags);

    SDL_WM_SetCaption(lua_tostring(L, 7), lua_tostring(L, 7));

    if(flags & SDL_DOUBLEBUF)
        rd->frame_buffer = SDL_CreateRGBSurface(0, rd->scr_width, rd->scr_height, rd->bit_depth, 0, 0, 0, 0);
    else
        rd->frame_buffer = NULL;

    //clear lua stack
    lua_settop(L, 0);
    return rd;
}

void
RC_ResetVideoMode(RC_RenderData *rc_r, Uint32 flags)
{
    SDL_Surface *n_scr;
    n_scr = SDL_SetVideoMode(rc_r->scr_width, rc_r->scr_height, rc_r->bit_depth, flags);

    if(n_scr)
    {
        SDL_FreeSurface(rc_r->screen);
        rc_r->screen = n_scr;
    }
}

RC_PlayerCamera*
RC_InitCamera(double x, double y, double m_speed, double r_speed)
{
    RC_PlayerCamera *pc = malloc( sizeof(RC_PlayerCamera) );
    pc->pos_x = x;
    pc->pos_y = y;
    pc->dir_x = -1;
    pc->dir_y = 0;
    pc->plane_x = 0;
    pc->plane_y = 0.66;
    pc->move_speed = m_speed;
    pc->rot_speed = r_speed;

    return pc;
}
