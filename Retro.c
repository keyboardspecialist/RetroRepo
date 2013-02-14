#include "Retro.h"


#define _DEBUG_MODE_

/*
    GLOBAL VARIABLES
*/
int __opengl;
int _debug_mode;
int _engine_running;
int _dev_console_active;
double _FPS;

RC_RenderData *_render_data;
RC_PlayerCamera *_pcam;
RC_Map *_world_map;
RC_InputControls _input;
ConsoleInformation  *_dev_console;
lua_State *RC_L;

Retro_Clock r_clock;
Retro_MMap r_mmap;

//deprecated
void
debug(const char *msg)
{
    if(_debug_mode)
    {
        if(_dev_console)
            CON_Out(_dev_console, msg);
        else
            puts(msg);
    }
}


void
Retro_doRndMap()
{

    RC_SaveMap(_world_map);

printf("\npos x %f pos y %f dx %f dy %f ms %f rs %f\n", _pcam->pos_x, _pcam->pos_y, _pcam->dir_x, _pcam->dir_y, _pcam->move_speed, _pcam->rot_speed);
}

int
Retro_init(void)
{
    int ret = TRUE;

//GLOBAL INITS
_FPS = 0;
_engine_running = TRUE;
_dev_console_active = FALSE;
_world_map = NULL;
_pcam = NULL;

//__opengl = FALSE; no longer in use
#ifdef _DEBUG_MODE_
_debug_mode = TRUE;
#endif

    RC_LuaInit(&RC_L);
    if(SDL_Init(SDL_INIT_EVERYTHING) == 0)
    {
        Uint32 flags;
        debug("SDL_Init() successful...");
  /*      if(__opengl)
            flags = SDL_HWSURFACE | SDL_GL_DOUBLEBUFFER | SDL_OPENGL;
        else*/
            flags = SDL_HWSURFACE | SDL_DOUBLEBUF;
        if((_render_data = RC_Init(RC_L, flags)) != NULL)
        {
            //TURN ON LIGHTING FOR NOW
            _render_data->render_light = FALSE;
            //BILINEAR FILTERING
            _render_data->bi_filter = TRUE;
            /* Initialize the console, so we  can start sending debug statements to it */
            SDL_Rect r;
            r.x = 0;
            r.y = 0;
            r.w = 800;
            r.h = 400;
            _dev_console = RC_ConsoleInit("ConsoleFont.bmp", _render_data->frame_buffer, 100, r);
            CON_SetExecuteFunction(_dev_console, Retro_ConsoleEventHandler);
            RC_SetDebugConsole(_dev_console);

            debug("RC_Init() successful...");
            if(TTF_Init() == 0)
                debug("TTF_Init() successful...");
            else
            {
                ret = FALSE;
                debug("TTF_Init() failed...");
            }
        }
        else
        {
            ret = FALSE;
            debug("RC_Init() failed...");
        }
    }
    else
    {
        ret = FALSE;
        debug("SDL_Init() failed...");
    }

    if(ret)
    {
        SDL_EnableKeyRepeat(SDL_DEFAULT_REPEAT_DELAY, SDL_DEFAULT_REPEAT_INTERVAL);
        _world_map = NULL;
        _pcam = RC_InitCamera(0.0, 0.0, 25.5, 18.0);

        _input.kup = SDLK_r;
        _input.kdn = SDLK_f;
        _input.krt = SDLK_g;
        _input.klf = SDLK_d;
        _input.kup_state = SDL_KEYUP;
        _input.kdn_state = SDL_KEYUP;
        _input.krt_state = SDL_KEYUP;
        _input.klf_state = SDL_KEYUP;

    }
    return ret;
}

void
Retro_EventHandler(SDL_Event *event)
{
    const int etype = event->type;
    RC_ScanKeyboardInput(event, &_input);
    RC_ScanMouseInput(event, &_input);
    switch(etype)
    {
        case SDL_QUIT:
            _engine_running = FALSE;
            break;
        case SDL_KEYDOWN:
        {
            SDL_KeyboardEvent kb = event->key;
            SDL_keysym k = kb.keysym;

            switch(k.sym)
            {
                case SDLK_BACKQUOTE:
                        _dev_console_active = TRUE;
                        CON_Show(_dev_console);
                    break;
                case SDLK_l:
                    _render_data->render_light = !_render_data->render_light;
                    break;
                case SDLK_b:
                    _render_data->bi_filter = !_render_data->bi_filter;
                    break;
                default:
                    printf("\nKEY : %d", k.sym);
                    break;
            }
        }break;
    }
}

void
Retro_ConsoleEventHandler(ConsoleInformation *cl, char *cmd)
{
    char *delims = " ";
    char *tok = strtok(cmd, delims);

    /*
     * Some basic top level commands, parsing can come later
     */
    if(strcmp(tok, "gencave") == 0)
    {
        RC_ConsoleDebugOut(NULL, "Making a cave");
        srand(time(NULL));

        if(_world_map)
            RC_FreeMap(_world_map);
        //these values generate some pretty good maps. its just for testing afterall
        _world_map = RC_MapGenCave(rand() % 10000 + 5, 32, 32, 40, 5, 1, 8);
        _pcam->pos_x = _world_map->start_x;
        _pcam->pos_y = _world_map->start_y;
        Retro_MiniMapInit();
        RC_ConsoleDebugOut(NULL, "Finished");
    }
    else if(strcmp(tok, "savemap") == 0)
    {
        if(_world_map)
        {
            RC_SaveMap(_world_map);
            RC_ConsoleDebugOut(NULL, "Map Saved /maps/%s", _world_map->map_name);
        }
    }
    else if(strcmp(tok, "loadmap") == 0)
    {
        char *fldr = "maps/";
        char *fname;
        tok = strtok(NULL, delims);
        if(_world_map)
            RC_FreeMap(_world_map);

        //naive quote trim
        fname = malloc(sizeof(char) * strlen(fldr) + strlen(tok));
        strcpy(fname, fldr);
        strncat(fname, tok+1, strlen(tok)-2);

        _world_map = RC_LoadMap(fname);
        _pcam->pos_x = _world_map->start_x;
        _pcam->pos_y = _world_map->start_y;
        Retro_MiniMapInit();
        free(fname);
    }
    else if(strcmp(tok, "stats") == 0)
    {
        tok = strtok(NULL, delims);
        if(strcmp(tok, "display") == 0)
        {
            RC_ConsoleDebugOut(NULL, "[====Display Stats====]");
            RC_ConsoleDebugOut(NULL, "Display Width [%d]",_render_data->scr_width);
            RC_ConsoleDebugOut(NULL, "Display Height [%d]",_render_data->scr_height);
            RC_ConsoleDebugOut(NULL, "Bit Depth [%d]",_render_data->bit_depth);
            RC_ConsoleDebugOut(NULL, "Texture Width [%d]",_render_data->tex_width);
            RC_ConsoleDebugOut(NULL, "Texture Height [%d]",_render_data->tex_height);
            RC_ConsoleDebugOut(NULL, "[=====================]");
        }
    }
    else if(strcmp(tok, "noclip") == 0)
    {
        //cheater, its gonna explode

    }
    else if(strcmp(tok, "-goober") == 0)
    {
        //for centrinia :)
    }
    else if(strcmp(tok, "ex_lighting") == 0)
    {
        //toggle lighting
        tok = strtok(NULL, delims);
        int do_l = strtol(tok, NULL, 10); //a bit unsafe

        if(do_l)
        {
            _render_data->render_light = TRUE;
        }
        else
        {
            _render_data->render_light = FALSE;
        }

    }
    else if(strcmp(tok, "ex_bfilter") == 0)
    {
        //toggle lighting
        tok = strtok(NULL, delims);
        int do_l = strtol(tok, NULL, 10); //a bit unsafe

        if(do_l)
        {
            _render_data->bi_filter = TRUE;
        }
        else
        {
            _render_data->bi_filter = FALSE;
        }

    }
    else if(strcmp(tok, "timescale") == 0)
    {
        tok = strtok(NULL, delims);
        double t_scale = strtod(tok, NULL); //a bit unsafe

        if(t_scale <= 0)
            t_scale = 1.0;

        r_clock.time_scale = t_scale;
        RC_ConsoleDebugOut(NULL, "Time Scale set to: %d", r_clock.time_scale);

    }
    else if(strcmp(tok, "fullscreen") == 0)
    {
        tok = strtok(NULL, delims);
        int var = strtol(tok, NULL, 10);

        if(var != 0)
        {
            RC_ResetVideoMode(_render_data, SDL_HWSURFACE | SDL_DOUBLEBUF | SDL_FULLSCREEN);
        }
        else
        {
            RC_ResetVideoMode(_render_data, SDL_HWSURFACE | SDL_DOUBLEBUF);
        }
    }
}

void
Retro_DebugInfoDraw(void)
{
    if(TTF_WasInit())
    {
        SDL_Color color = {0xFF, 0xFF, 0xFF};
        SDL_Surface *fps, *xy, *rot;
        TTF_Font *font = TTF_OpenFont("Garuda.ttf", 18);
        if(!font)
        {
            RC_ConsoleDebugOut(NULL,"\nTTF_OpenFont failed: %s\n", TTF_GetError());
            return;
        }
        char buf[100];
        sprintf(buf, "FPS: %f", _FPS);
        if(!(fps=TTF_RenderText_Solid(font, buf, color)))
        {
            RC_ConsoleDebugOut(NULL,"TTF_RenderText failed: %s", TTF_GetError());
            return;
        }
        sprintf(buf, "X[%3.2f] Y[%3.2f]", _pcam->pos_x, _pcam->pos_y);
        if(!(xy=TTF_RenderText_Solid(font, buf, color)))
        {
            RC_ConsoleDebugOut(NULL,"TTF_RenderText failed: %s", TTF_GetError());
            return;
        }
        sprintf(buf, "Cam X[%3.2f] Y[%3.2f]", _pcam->dir_x, _pcam->dir_y);
        if(!(rot=TTF_RenderText_Solid(font, buf, color)))
        {
            RC_ConsoleDebugOut(NULL,"TTF_RenderText failed: %s", TTF_GetError());
            return;
        }

        SDL_Rect r;
        r.x = 0;
        r.y = 0;
        r.h = fps->h;
        r.w = fps->w;
        SDL_BlitSurface(fps, NULL, _render_data->frame_buffer, &r);
        r.y = r.y + xy->h;
        r.h = xy->h;
        r.w = xy->w;
        SDL_BlitSurface(xy, NULL, _render_data->frame_buffer, &r);
        r.y = r.y + rot->h;
        r.h = rot->h;
        r.w = rot->w;
        SDL_BlitSurface(rot, NULL, _render_data->frame_buffer, &r);
        SDL_FreeSurface(fps);
        SDL_FreeSurface(xy);
        SDL_FreeSurface(rot);
        TTF_CloseFont(font);
    }
}

void
Retro_MiniMapInit(void)
{
    if(_world_map)
    {
        r_mmap.c_size = 4;
        r_mmap.m_rect = malloc(sizeof(SDL_Rect));
        r_mmap.c_rect = malloc(sizeof(SDL_Rect));
        r_mmap.m_rect->w = _world_map->mapsize_x * r_mmap.c_size;
        r_mmap.m_rect->h = _world_map->mapsize_y * r_mmap.c_size;
        r_mmap.m_rect->y = 0;
        r_mmap.m_rect->x = _render_data->scr_width - r_mmap.m_rect->w;
        r_mmap.c_rect->w = r_mmap.c_size;
        r_mmap.c_rect->h = r_mmap.c_size;
        r_mmap.offset_x = _render_data->scr_width - r_mmap.m_rect->w;
        r_mmap.offset_y = 0;

        r_mmap.s_flr = 0x007F7F7F;
        r_mmap.s_wall = 0x00FFFFFF;
        r_mmap.u_seen = 0x00000000;
        r_mmap.mmap = SDL_CreateRGBSurface(0, r_mmap.m_rect->w, r_mmap.m_rect->h, _render_data->bit_depth, 0, 0, 0, 0);
        SDL_SetAlpha(r_mmap.mmap, 0, 180);

        int x=0,y=0;
        for(x=0; x < _world_map->mapsize_x; x++)
        for(y=0; y < _world_map->mapsize_y; y++)
        {
            r_mmap.c_rect->x = (r_mmap.c_size * x);
            r_mmap.c_rect->y = (r_mmap.c_size * y);
            SDL_FillRect(r_mmap.mmap,r_mmap.c_rect, r_mmap.u_seen);
        }
    }
}

void
Retro_MiniMap(void)
{
    int x, y;
    x = (int)_pcam->pos_x;
    y = (int)_pcam->pos_y;
    //flag seen cells; TODO: only see cells in front of the camera; Only redraw currently seen cells
    int i,j;
    Uint32 fl;
    for(i = x-1; i < x+1; i++)
    for(j = y-1; j < y+1; j++)
    {
        if(i >= _world_map->mapsize_x) continue;
        if(j >= _world_map->mapsize_y) continue;
        if(i<0) continue;
        if(j<0) continue;
        fl = _world_map->map_data[i][j].flags;
        if(!(fl&F_SEEN))
        {
            _world_map->map_data[i][j].flags |= F_SEEN;
        }

    }
    Uint32 xx, yy;
    for(xx = 0; xx < _world_map->mapsize_x ; xx++)
    for(yy = _world_map->mapsize_y-1; yy >0 ; yy--)
    {
       // fprintf(stderr, "xx %d yy %d\n", xx, yy);
        Uint32 fl = _world_map->map_data[xx][yy].flags;
        if(fl & F_SEEN)
        {
            if(fl & F_WALL)
            {
                r_mmap.c_rect->x = (r_mmap.c_size * xx);
                r_mmap.c_rect->y = (r_mmap.c_size * yy);
                SDL_FillRect(r_mmap.mmap,r_mmap.c_rect, r_mmap.s_wall);
            }
            if(fl & F_FLOOR)
            {
                r_mmap.c_rect->x = (r_mmap.c_size * xx);
                r_mmap.c_rect->y = (r_mmap.c_size * yy);
                SDL_FillRect(r_mmap.mmap,r_mmap.c_rect, r_mmap.s_flr);
            }
        }
   /*     else
        {
            r_mmap.c_rect->x = (r_mmap.c_size * xx);
            r_mmap.c_rect->y = (r_mmap.c_size * yy);
            SDL_FillRect(r_mmap.mmap,r_mmap.c_rect, r_mmap.u_seen);
        }*/
    }
    SDL_BlitSurface(r_mmap.mmap, NULL, _render_data->frame_buffer, r_mmap.m_rect);
}

void
Retro_cleanup(void)
{
    SDL_FreeSurface(_render_data->screen);
    SDL_FreeSurface(_render_data->frame_buffer);
    CON_Destroy(_dev_console);
    if(_world_map)
        RC_FreeMap(_world_map); //safe to call on NULL, but just to be sure
    free(_pcam);
    RC_LuaClose(RC_L);
    RC_RaycastEngineCleanup(_render_data);
    free(_render_data);
    TTF_Quit();
    SDL_Quit();
}


void
Retro_update(Uint32 dtime)
{
    SDL_Event e;
    while(SDL_PollEvent(&e))
    {
        if(_dev_console_active)
            if(!CON_Events(&e))
                continue;
        Retro_EventHandler(&e);
    }
    RC_UpdateCamera(_pcam, _world_map, &_input, dtime);
}

void
Retro_render(void)
{   //placeholder
    RC_Sprite _sprite[1];

    SDL_Rect r;

    SDL_FillRect(_render_data->frame_buffer, NULL, 0);
    if(_world_map != NULL)
    {
        SDL_LockSurface(_render_data->frame_buffer);
        RC_RaycastDraw(_render_data, _world_map, _pcam, _sprite);
        SDL_UnlockSurface(_render_data->frame_buffer);
        Retro_MiniMap();
    }
    Retro_DebugInfoDraw();
    if(_dev_console_active)
    {
        RC_ConsoleDraw(_dev_console);
    }

    //debugDraw()

    r.x = 0;
    r.y = 0;
    SDL_BlitSurface(_render_data->frame_buffer, NULL, _render_data->screen, &r);
    SDL_Flip(_render_data->screen);
}

int
Retro_exec(void)
{
    int ret = 0, skip=0;
    if(Retro_init() == FALSE)
    {
        ret = -1;
    }
    else
    {
        r_clock.time_scale = 1.0;
        r_clock.new_time = SDL_GetTicks();
        r_clock.old_time = r_clock.new_time;
        r_clock.next_tick = r_clock.new_time;
        while(_engine_running)
        {
            r_clock.old_time = r_clock.new_time;
            skip = 0;
            if(r_clock.delta_time < 1)
                r_clock.delta_time = 1;

            while(SDL_GetTicks() > r_clock.next_tick && skip < MAX_FRAMESKIP)
            {
                Retro_update(SKIP_TICKS * r_clock.time_scale);
                r_clock.next_tick += SKIP_TICKS;
                skip++;
            }
            r_clock.delta_time *= r_clock.time_scale;
            Retro_update(r_clock.delta_time);
            Retro_render();
            r_clock.new_time = SDL_GetTicks();
            r_clock.delta_time = r_clock.new_time - r_clock.old_time;


            _FPS = 1.0 / ((r_clock.new_time - r_clock.old_time) / 1000.0);
        }
    }
    Retro_cleanup();
    return ret;
}



int
main(int argc, char **argv)
{
    if(Retro_exec() == -1)
        debug("Engine Error, closing...");
    return 0;
}
