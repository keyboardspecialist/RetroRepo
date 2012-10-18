#include "Retro.h"


#define _DEBUG_MODE_

/*
    GLOBAL VARIABLES
*/
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

#ifdef _DEBUG_MODE_
_debug_mode = TRUE;
#endif

    RC_LuaInit(&RC_L);
    if(SDL_Init(SDL_INIT_EVERYTHING) == 0)
    {
        debug("SDL_Init() successful...");
        if((_render_data = RC_Init(RC_L, SDL_HWSURFACE | SDL_DOUBLEBUF)) != NULL)
        {
            //TURN ON LIGHTING FOR NOW
            _render_data->render_light = TRUE;
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
        case SDL_KEYDOWN:  //TODO: smooth input, detect held keys
        {
            SDL_KeyboardEvent kb = event->key;
            SDL_keysym k = kb.keysym;

            switch(k.sym)
            {
                case SDLK_BACKQUOTE:
                    _dev_console_active = !_dev_console_active;
                    if(_dev_console_active)
                        CON_Show(_dev_console);
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
Retro_fpsInfoDraw(void)
{
    if(TTF_WasInit())
    {
        SDL_Color color = {0xFF, 0xFF, 0xFF};
        SDL_Surface *fps;
        TTF_Font *font = TTF_OpenFont("Garuda.ttf", 24);
        if(!font)
        {
            RC_ConsoleDebugOut(NULL,"\nTTF_OpenFont failed: %s\n", TTF_GetError());
            return;
        }
        char buf[15];
        sprintf(buf, "FPS: %f", _FPS);
        if(!(fps=TTF_RenderText_Solid(font, buf, color)))
        {
            RC_ConsoleDebugOut(NULL,"TTF_RenderText failed: %s", TTF_GetError());
            return;
        }
        SDL_BlitSurface(fps, NULL, _render_data->frame_buffer, NULL);
        SDL_FreeSurface(fps);
        TTF_CloseFont(font);
    }
}

void
Retro_cleanup(void)
{
    SDL_FreeSurface(_render_data->screen);
    SDL_FreeSurface(_render_data->frame_buffer);
    free(_render_data);
    //CON_Free(_dev_console);
    CON_Destroy(_dev_console);
    if(_world_map)
        RC_FreeMap(_world_map); //safe to call on NULL, but just to be sure
    free(_pcam);
    RC_LuaClose(RC_L);
    SDL_Quit();
    TTF_Quit();
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
    }
    Retro_fpsInfoDraw();
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
        ret = -1;
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
        Retro_cleanup();
    }

    return ret;
}



int
main(int argc, char **argv)
{
    if(Retro_exec() == -1)
        debug("Engine Error, closing...");
    return 0;
}
