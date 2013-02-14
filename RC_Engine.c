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

/*
    if(flags & SDL_OPENGL)
    { //some standard values for now
        SDL_GL_SetAttribute(SDL_GL_RED_SIZE,            8);
        SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE,          8);
        SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE,           8);
        SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE,          8);

        SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE,          16);
        SDL_GL_SetAttribute(SDL_GL_BUFFER_SIZE,            32);

        SDL_GL_SetAttribute(SDL_GL_ACCUM_RED_SIZE,        8);
        SDL_GL_SetAttribute(SDL_GL_ACCUM_GREEN_SIZE,    8);
        SDL_GL_SetAttribute(SDL_GL_ACCUM_BLUE_SIZE,        8);
        SDL_GL_SetAttribute(SDL_GL_ACCUM_ALPHA_SIZE,    8);

        SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS,  1);

        SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES,  2);

        rd->screen = SDL_SetVideoMode(rd->scr_width, rd->scr_height, rd->bit_depth, flags);

        glClearColor(0,0,0,0);
        glViewport(0,0,rd->scr_width, rd->scr_height);
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        glOrtho(0,rd->scr_width, rd->scr_height, 0, 1, -1);
        glMatrixMode(GL_MODELVIEW);
        glEnable(GL_TEXTURE_2D);
        glLoadIdentity();
    }
    else
    {*/
        rd->screen = SDL_SetVideoMode(rd->scr_width, rd->scr_height, rd->bit_depth, flags);
        if(flags & SDL_DOUBLEBUF)
            rd->frame_buffer = SDL_CreateRGBSurface(0, rd->scr_width, rd->scr_height, rd->bit_depth, 0, 0, 0, 0);
        else
            rd->frame_buffer = NULL;
    //}


    SDL_WM_SetCaption(lua_tostring(L, 7), lua_tostring(L, 7));
    //clear lua stack
    lua_settop(L, 0);
    //precalc floor/ceil data
    RC_Precalc(rd);
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
        //recalc precalc :)
        RC_Precalc(rc_r);
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
