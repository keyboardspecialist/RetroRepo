#ifndef __RCRENDER_H__
#define __RCRENDER_H__


#include "RC_GlobalDefs.h"

#include <SDL/SDL.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include "RC_Resources.h"
#include "RC_Input.h"
#include "RC_Debug.h"

#define TICKS_PER_SECOND 25
#define SKIP_TICKS 1000 / TICKS_PER_SECOND
#define MAX_FRAMESKIP 5

#define NUMSPRITES 19

typedef struct
{
    SDL_Surface *screen;
    SDL_Surface *frame_buffer;
    Uint32      scr_width;
    Uint32      scr_height;
    Uint32      bit_depth;
    Uint32      tex_width;
    Uint32      tex_height;
    double      wall_scale;
    double      light_rad;
    Uint32      render_light;
}RC_RenderData;

/*
 * PlayerCamera struct
 * Defines a 2D camera
 */
typedef struct
{
    double pos_x;
    double pos_y;
    double dir_x;
    double dir_y;
    double plane_x;
    double plane_y;
    double move_speed;
    double rot_speed;
}RC_PlayerCamera;


extern int _spriteOrder[NUMSPRITES];
extern double _spriteDistance[NUMSPRITES];

extern void RC_UpdateCamera(RC_PlayerCamera *cam, RC_Map *map, RC_InputControls *rc_i, Uint32 dtime);
extern void RC_RaycastDraw(RC_RenderData *rd, RC_Map *map, RC_PlayerCamera *cam, RC_Sprite *sprites);
#endif
