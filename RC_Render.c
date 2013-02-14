#include "RC_Render.h"


void
RC_UpdateCamera(RC_PlayerCamera *cam, RC_Map *map, RC_InputControls *rc_i, Uint32 dtime)
{
    double delta = (double)dtime / (double)SKIP_TICKS;
    if(rc_i->kup_state == SDL_KEYDOWN)
    {
        double new_posx = cam->pos_x + cam->dir_x * (cam->move_speed * delta);
        double new_posy = cam->pos_y + cam->dir_y * (cam->move_speed * delta);

        if((int)new_posx >= map->mapsize_x)
            new_posx = map->mapsize_x-1;
        else if((int)new_posx < 0)
            new_posx = 0;

        if((int)new_posy >= map->mapsize_y)
            new_posy = map->mapsize_y-1;
        else if((int)new_posy < 0)
            new_posy = 0;

        if((map->map_data[(int)new_posx][(int)cam->pos_y]).flags & F_FLOOR)
        cam->pos_x = new_posx;
        if((map->map_data[(int)cam->pos_x][(int)new_posy]).flags & F_FLOOR)
        cam->pos_y = new_posy;
    }
    if(rc_i->kdn_state == SDL_KEYDOWN)
    {
        double new_posx = cam->pos_x - cam->dir_x * (cam->move_speed * delta);
        double new_posy = cam->pos_y - cam->dir_y * (cam->move_speed * delta);

        if((int)new_posx >= map->mapsize_x)
            new_posx = map->mapsize_x-1;
        else if((int)new_posx < 0)
            new_posx = 0;

        if((int)new_posy >= map->mapsize_y)
            new_posy = map->mapsize_y-1;
        else if((int)new_posy < 0)
            new_posy = 0;

        if((map->map_data[(int)new_posx][(int)cam->pos_y]).flags & F_FLOOR)
        cam->pos_x = new_posx;
        if((map->map_data[(int)cam->pos_x][(int)new_posy]).flags & F_FLOOR)
        cam->pos_y = new_posy;
    }
    if(rc_i->krt_state == SDL_KEYDOWN)
    //if(rc_i->m_relx > 2)
    {
        double oldDirX = cam->dir_x;
        double oldPlaneX = cam->plane_x;
        double accel = fabs(rc_i->m_relx / delta);
        double d_rot = (cam->rot_speed) * delta;

        cam->dir_x = cam->dir_x * cos(-d_rot) - cam->dir_y * sin(-d_rot);
        cam->dir_y = oldDirX * sin(-d_rot) + cam->dir_y * cos(-d_rot);
        cam->plane_x = cam->plane_x * cos(-d_rot) - cam->plane_y * sin(-d_rot);
        cam->plane_y = oldPlaneX * sin(-d_rot) + cam->plane_y * cos(-d_rot);
    }
    if(rc_i->klf_state == SDL_KEYDOWN)
    //if(rc_i->m_relx < -2 || rc_i->m_relx > 2)
    {
        double oldDirX = cam->dir_x;
        double oldPlaneX = cam->plane_x;
        double accel = fabs(rc_i->m_relx / delta);
        double d_rot = (cam->rot_speed ) * delta;

        cam->dir_x = cam->dir_x * cos(d_rot) - cam->dir_y * sin(d_rot);
        cam->dir_y = oldDirX * sin(d_rot) + cam->dir_y * cos(d_rot);
        cam->plane_x = cam->plane_x * cos(d_rot) - cam->plane_y * sin(d_rot);
        cam->plane_y = oldPlaneX * sin(d_rot) + cam->plane_y * cos(d_rot);
    }
}


/*
void
RC_DrawVertLine(SDL_Surface *drawsurf, int start, int end, int x, ColorRGB color)
{
    SDL_Rect px;
    px.x = x;
    px.w = 1;
    px.h = end - start + 1;
    px.y = start;
    SDL_FillRect(drawsurf, &px, color);
}
*/

/* Insertion sort for sorting sprites by distance for the Z-Buffer
 */
void
RC_SpriteSort(int *order, double *dist, int amount)
{
    int i;
    for(i=1; i<amount; i++)
    {
        int j;
        double d = dist[i];
        int o = order[i];
        for(j=i-1; j>=0; j--)
        {
            if(dist[j] > d)
                break;
            dist[j+1] = dist[j];
            order[j+1] = order[j];
        }

        dist[j+1] = d;
        order[j+1] = o;
    }
}


void
darken_pixel(Uint32 *pix, double dist)
{
    int a = 255 - (int) (dist*48);
    if(a < 0) a = 0;
    Uint32 c1 = *pix;
    Uint32 c2 = (c1&0xFF00FF) * a >>8;
    c1 = (c1&0xFF00) * a >>8;
    *pix =((c1&0xFF00) | (c2&0xFF00FF));
}

/*
    ridiculous code by centrinia

#define RADIX_POINT             16
#define R(v) ((v & 0x00FF0000)>>16)
#define G(v) ((v & 0x0000FF00)>>8)
#define B(v) (v & 0x000000FF)

#define umul_ppmm(w1, w0, u, v) \
  __asm__ ("mull %3"                                                    \
           : "=a" (w0), "=d" (w1)                                       \
           : "%0" ((Uint32)(u)), "rm" ((Uint32)(v)))
#define umuladd_smm(s, u, v,t,t1) \
  __asm__ ("mull %4\n"                                                  \
           "\taddl %1,%2"                       \
           : "=a" (t0),"=d" (t1),"=rm"(s)                                       \
           : "%0" ((Uint32)(u)), "rm" ((Uint32)(v)),"2"((Uint32)(s)))
//typedef unsigned int Uint32;
Uint32
float_to_int (float foo, int shit)
{
  union
  {
    Uint32 bar;
    float foo;
  } baz;
  int exp;
  Uint32 man;
  baz.foo = foo;
  exp = (baz.bar >> 23) & 0xff;
  man = baz.bar & 0x7fffff;
  if (exp == 0)
    {
      exp = -126;
    }
  else
    {
      exp -= 127;
      man |= 0x800000;
    }
  exp -= 23;
  if ((shit + exp) >= 0)
    {
      man = man << (shit + exp);
    }
  else if ((shit + exp) < 0)
    {
      man = man >> (-(exp + shit));
    }

  return man;
}

Uint32
bit_reverse (Uint32 x)
{
  x = ((x & 0xaaaaaaaa) >> 1) | ((x & 0x55555555) << 1);
  x = ((x & 0xcccccccc) >> 2) | ((x & 0x33333333) << 1);
  x = ((x & 0xf0f0f0f0) >> 4) | ((x & 0x0f0f0f0f) << 4);
  x = ((x & 0xff00ff00) >> 8) | ((x & 0x00ff00ff) << 8);
  x = ((x & 0xffff0000) >> 16) | ((x & 0x0000ffff) << 16);
  return x;
}

Uint32
popcount (Uint32 x)
{
  x = ((x & 0xaaaaaaaa) >> 1) + (x & 0x55555555);
  x = ((x & 0xcccccccc) >> 2) + (x & 0x33333333);
  x = ((x & 0xf0f0f0f0) >> 4) + (x & 0x0f0f0f0f);
  x = ((x & 0xff00ff00) >> 8) + (x & 0x00ff00ff);
  x = ((x & 0xffff0000) >> 16) + (x & 0x0000ffff);
  return x;
}

Uint32
log2plus1 (Uint32 x)
{
  if (x == 0)
    {
      return 0;
    }
  x = bit_reverse (x);
  x ^= x - 1;
  return popcount (~x);
}

Uint32
mix (Uint32 a, Uint32 b, Uint32 t)
{
  Uint32 c = b - a;
  c *= t;
  c += a << RADIX_POINT;
  c >>= RADIX_POINT;
  return c;
}

Uint32
bimix (Uint32 a00, Uint32 a01, Uint32 a10, Uint32 a11, Uint32 u, Uint32 v)
{
  Uint32 c0 = (a01 - a00) * u + (a00 << RADIX_POINT);
  Uint32 c1 = (a11 - a10) * u + (a10 << RADIX_POINT);
  Uint32 w;
  Uint32 c;
  umul_ppmm (c, w, c1 - c0, v);
  c += c0 >> RADIX_POINT;

  c -= c1 < c0 ? v : 0;

  return c;
}

void
bilinear_filter (Uint32 * pixels, const Uint32 w, double u, double v,
                 Uint32 * c)
{
  Uint32 wlog = log2plus1 (w) - 1;
  Uint32 fx = float_to_int (u, RADIX_POINT + wlog);
  Uint32 fy = float_to_int (v, RADIX_POINT + wlog);
  //Uint32 px = float_to_int (u, wlog);
  //Uint32 py = float_to_int (v, wlog);
  Uint32 px = fx >> RADIX_POINT;
  Uint32 py = fy >> RADIX_POINT;
  int px_index = px & (w - 1);
  int pxp1_index = (px + 1) & (w - 1);
  int py_row = (py & (w - 1)) << wlog;
  int pyp1_row = ((py + 1) & (w - 1)) << wlog;
  Uint32 p1 = pixels[py_row + px_index];
  Uint32 p2 = pixels[py_row + pxp1_index];
  Uint32 p3 = pixels[pyp1_row + px_index];
  Uint32 p4 = pixels[pyp1_row + pxp1_index];

  //calc fractional portion
  Uint32 Fx = fx & 0x0000FFFF;
  Uint32 Fy = fy & 0x0000FFFF;

#if 0
  Uint32 FxFy = Fx * Fy;
  Uint32 w1 = FxFy - ((Fx + Fy) << RADIX_POINT);
  Uint32 w2 = (Fx << RADIX_POINT) - FxFy;
  Uint32 w3 = (Fy << RADIX_POINT) - FxFy;
  Uint32 w4 = FxFy;
  w1 >>= RADIX_POINT;
  w2 >>= RADIX_POINT;
  w3 >>= RADIX_POINT;
  w4 >>= RADIX_POINT;

  //calculate weighted sum of all pixels, ignoring alpha channel
  Uint32 o_r = (R (p1) * w1 + R (p2) * w2 + R (p3) * w3 + R (p4) * w4);
  Uint32 o_g = (G (p1) * w1 + G (p2) * w2 + G (p3) * w3 + G (p4) * w4);
  Uint32 o_b = (B (p1) * w1 + B (p2) * w2 + B (p3) * w3 + B (p4) * w4);

  o_r = o_r & 0x00ff0000;
  o_g = o_g & 0x00ff0000;
  o_b = o_b & 0x00ff0000;
  o_g >>= 8;
  o_b >>= 16;
#else

#if 1
  Uint32 o_r = bimix (R (p1), R (p2), R (p3), R (p4), Fx, Fy);
  Uint32 o_g = bimix (G (p1), G (p2), G (p3), G (p4), Fx, Fy);
  Uint32 o_b = bimix (B (p1), B (p2), B (p3), B (p4), Fx, Fy);
#else
  Uint32 o_r = mix (mix (R (p1), R (p2), Fx), mix (R (p3), R (p4), Fx), Fy);
  Uint32 o_g = mix (mix (G (p1), G (p2), Fx), mix (G (p3), G (p4), Fx), Fy);
  Uint32 o_b = mix (mix (B (p1), B (p2), Fx), mix (B (p3), B (p4), Fx), Fy);
#endif
  o_r &= 0xff;
  o_g &= 0xff;
  o_b &= 0xff;
  o_r <<= 16;
  o_g <<= 8;
#endif

  *c = o_r | o_g | o_b;
}

*/
/*
    This implementation uses fixed point math to avoid float -> int conversions

    It uses floats now, and is slower than the above code
*/

#define R(v) ((v & 0x00FF0000)>>16)
#define G(v) ((v & 0x0000FF00)>>8)
#define B(v) (v & 0x000000FF)


void
bilinear_filter(Uint32 *pixels, const Uint32 w, double u, double v, Uint32 *c)
{
/*    Uint32 fx = (Uint32)(u * 65536.0);
    Uint32 fy = (Uint32)(v * 65536.0);
    Uint32 px = fx & 0xFFFF0000; //floor
    Uint32 py = fy & 0xFFFF0000;
*/
    int px = (int)u;
    int py = (int)v;

    Uint32 p1 = pixels[w * ((py) & (w)) + ((px) & (w))];
    Uint32 p2 = pixels[w * ((py) & (w)) + ((px+1) & (w-1))];
    Uint32 p3 = pixels[w * ((py+1) & (w-1)) + ((px) & (w))];
    Uint32 p4 = pixels[w * ((py+1) & (w-1)) + ((px+1) & (w-1))];

    //calc fractional portion
    //Uint32 Fx = fx & 0x0000FFFF;
    //Uint32 Fy = fy & 0x0000FFFF;
    //Uint32 Fx1 = 0x00010000 - Fx; // 1 - Fx
    //Uint32 Fy1 = 0x00010000 - Fy; // 1 - Fy
    float Fx = u - px;
    float Fy = v - py;
    float Fx1 = 1.0f - Fx;
    float Fy1 = 1.0f - Fy;
 /*
    Uint32 w1 = (Fx1 * Fy1) ;
    Uint32 w2 = (Fx * Fy1) ;
    Uint32 w3 = (Fx1 * Fy) ;
    Uint32 w4 = (Fx * Fy) ;
*/
    int w1 = Fx1 * Fy1 * 256.0f;
    int w2 = Fx * Fy1 * 256.0f;
    int w3 = Fx1 * Fy * 256.0f;
    int w4 = Fx * Fy * 256.0f;

    //calculate weighted sum of all pixels, ignoring alpha channel
    Uint32 o_r = (R(p1) * w1 + R(p2) * w2 + R(p3) * w3 + R(p4) * w4);
    Uint32 o_g = (G(p1) * w1 + G(p2) * w2 + G(p3) * w3 + G(p4) * w4);
    Uint32 o_b = (B(p1) * w1 + B(p2) * w2 + B(p3) * w3 + B(p4) * w4);

    //o_r = o_r >= 255 ? 255 : o_r;
    //o_g = o_g >= 255 ? 255 : o_g;
    //o_b = o_b >= 255 ? 255 : o_b;
    *c = ( (o_r>>8) <<16) | ((o_g>>8) <<8) | (o_b>>8);
}



/*
    Lookup table for floor and ceiling distance data.
    Naively calcs for every vertical pixel, but this is a small price to pay for overall performance.
*/
double *cam_x_lookup=NULL;
double *fc_lookup=NULL;
double **fl_dist_lookup=NULL;
double scr_h2;
double sch_ws;
void
RC_Precalc(RC_RenderData *rd)
{
    free(fc_lookup);
    free(cam_x_lookup);

    if(fl_dist_lookup)
    {
        int fx = 0;
        for(; fx < rd->scr_width; fx++)
            free(fl_dist_lookup[fx]);
        free(fl_dist_lookup);
    }

    cam_x_lookup = malloc(sizeof(double) * rd->scr_width);
    fl_dist_lookup = malloc(sizeof(double) * rd->scr_width);
    fc_lookup = malloc(sizeof(double) * rd->scr_height);

    int x=0;
    for(; x < rd->scr_width; x++)
    {
        cam_x_lookup[x] = 2 * x / (double)rd->scr_width - 1;

        fl_dist_lookup[x] = malloc(sizeof(double) * rd->scr_height);
    }

    int y=0;
    for(; y < rd->scr_height; y++)
    {
        fc_lookup[y] = rd->scr_height / (2.0 * y - rd->scr_height);
    }

    int xx=0, yy=0;
    for(xx=0; xx < rd->scr_width; xx++)
    for(yy=0; yy < rd->scr_height; yy++)
    {
        fl_dist_lookup[xx][yy] = fc_lookup[yy]*2 + (fc_lookup[yy]*fabs(cam_x_lookup[xx]*cam_x_lookup[xx]));
    }
    scr_h2 = rd->scr_height / 2;
    sch_ws = rd->scr_height * rd->wall_scale;
}

void
RC_RaycastDraw(RC_RenderData *rd, RC_Map *map, RC_PlayerCamera *cam, RC_Sprite *sprites)
{
    double cameraX, rayPosX, rayPosY, rayDirX, rayDirY, sideDistX,
            sideDistY, deltaDistX, deltaDistY, perpWallDist;
    int mapX, mapY, stepX, stepY, hit, side, lineHeight, drawStart, drawEnd;
    int x;

 //   double _ZBuffer[rd->scr_width];

    for(x = 0; x < rd->scr_width; x++)
    {
        cameraX = cam_x_lookup[x];
        rayPosX = cam->pos_x;
        rayPosY = cam->pos_y;
        rayDirX = cam->dir_x + cam->plane_x * cameraX;
        rayDirY = cam->dir_y + cam->plane_y * cameraX;

        mapX = (int)rayPosX;
        mapY = (int)rayPosY;

        deltaDistX = sqrt(1 + (rayDirY * rayDirY) / (rayDirX * rayDirX));
        deltaDistY = sqrt(1 + (rayDirX * rayDirX) / (rayDirY * rayDirY));

        hit = 0;
        if(rayDirX < 0)
        {
            stepX = -1;
            sideDistX = (rayPosX - mapX) * deltaDistX;
        }
        else
        {
            stepX = 1;
            sideDistX = (mapX + 1.0 - rayPosX) * deltaDistX;
        }
        if(rayDirY < 0)
        {
            stepY = -1;
            sideDistY = (rayPosY - mapY) * deltaDistY;
        }
        else
        {
            stepY = 1;
            sideDistY = (mapY + 1.0 - rayPosY) * deltaDistY;
        }

        //loop til we hit a wall
        while(!hit)
        {
            if(sideDistX < sideDistY)
            {
                sideDistX += deltaDistX;
                mapX += stepX;
                side = 0;
            }
            else
            {
                sideDistY += deltaDistY;
                mapY += stepY;
                side = 1;
            }
            if( (map->map_data[mapX][mapY]).flags & F_WALL) hit = 1;
        }

        if(side == 0)
            perpWallDist = fabs((mapX - rayPosX + (1 - stepX)/2) / rayDirX);
        else
            perpWallDist = fabs((mapY - rayPosY + (1 - stepY)/2) / rayDirY);


        lineHeight = abs( (int)( rd->scr_height * rd->wall_scale / perpWallDist) );
        drawStart = -lineHeight/2 + rd->scr_height/2;
        drawEnd = lineHeight/2 + rd->scr_height/2;

        if(drawStart < 0)
            drawStart = 0;
        if(drawEnd >= rd->scr_height)
            drawEnd = rd->scr_height-1;

        //now the fun stuff, textures
        int useTexture = (M_TEXTURE((map->map_data[mapX][mapY]).texture_data)) - 1;
        int texX;
        double wallX;

        //wall projections
        if(side == 1)
            wallX = rayPosX + ((mapY - rayPosY + (1 - stepY) /2) / rayDirY) * rayDirX;
        else
            wallX = rayPosY + ((mapX - rayPosX + (1 - stepX) /2) / rayDirX) * rayDirY;

        double tu = wallX;
        wallX -= floor(wallX);
        texX = (int)(wallX * (double)rd->tex_width);
        if(side == 0 && rayDirX > 0) texX = rd->tex_width - texX - 1;
        if(side == 1 && rayDirY < 0) texX = rd->tex_width - texX - 1;

        int y;
        double ldist;
        double camX2;
        if(rd->render_light)
        {
            camX2 = fabs(cameraX*cameraX);
            ldist = 2*perpWallDist + (perpWallDist*camX2);
        }
        for(y = drawStart; y < drawEnd; y++)
        {
            int d = (y<<8) - ((rd->scr_height)<<7) + (lineHeight<<7);
            int texY = ((d * rd->tex_height) / lineHeight) >>8;

            Uint32 color;
            if(rd->bi_filter)
            {
                //double tu = (texX / (double)rd->tex_width);
                double tv = (texY / (double)rd->tex_height);
                bilinear_filter(((Uint32 *)(*(map->texture_list+useTexture))->pixels),rd->tex_width, tu, tv, &color);
                //bilinear_filter( ((Uint32 *)(*(map->texture_list+useTexture))->pixels), rd->tex_width, tu, tv, &color);
            }
            else
            {
                color = ((Uint32 *)(*(map->texture_list+useTexture))->pixels)[rd->tex_height * texY + texX];
            }

            if(rd->render_light) darken_pixel(&color, ldist);
            ((Uint32*)(rd->frame_buffer)->pixels)[rd->scr_width * y + x] = color;
        }

        //set ZBuffer distance for sprite drawing
       // _ZBuffer[x] = perpWallDist;

        //floor and ceiling projections
        //since the camera cannot move up or down we can do both at once
        double floorXWall, floorYWall;
        if(side == 0 && rayDirX > 0)
        {
            floorXWall = mapX;
            floorYWall = mapY + wallX;
        }
        else if(side == 0 && rayDirX < 0)
        {
            floorXWall = mapX + 1.0;
            floorYWall = mapY + wallX;
        }
        else if(side == 1 && rayDirY > 0)
        {
            floorXWall = mapX + wallX;
            floorYWall = mapY;
        }
        else
        {
            floorXWall = mapX + wallX;
            floorYWall = mapY + 1.0;
        }

        double distWall, distPlayer, currentDist;
        Uint32 tFloor=1, tCeil=2;
        distWall = perpWallDist;
        distPlayer = 0.0;
        if(drawEnd < 0) drawEnd = rd->scr_height;

        for(y = drawEnd + 1; y < rd->scr_height; y++)
        {
            currentDist = fc_lookup[y]; //floor/ceil lookup table
            double weight = (currentDist - distPlayer) / (distWall - distPlayer);
            double currentFloorX = weight * floorXWall + (1.0 - weight) * cam->pos_x;
            double currentFloorY = weight * floorYWall + (1.0 - weight) * cam->pos_y;
            tFloor = (M_FLOOR((map->map_data[(int)currentFloorX][(int)currentFloorY]).texture_data))-1;
            tCeil = (M_CEIL((map->map_data[(int)currentFloorX][(int)currentFloorY]).texture_data))-1;

            //double fu, fv;
            int floorTexX, floorTexY;
            //fu = fmod(currentFloorX * rd->tex_width,(double)rd->tex_width);
            //fv = fmod(currentFloorY * rd->tex_height,(double)rd->tex_height);
            //floorTexX = floor(fu);
            //floorTexY = floor(fv);
            floorTexX = (int)(((int)(currentFloorX * rd->tex_width)) % rd->tex_width);
            floorTexY = (int)(((int)(currentFloorY * rd->tex_height)) % rd->tex_height);
            //draw floor
            Uint32 color;
            //fu -= floorTexX;
            //fv -= floorTexY;
 /*           if(rd->bi_filter)
                //bilinear_filter( (Uint32*)(*(map->texture_list+tFloor))->pixels, (*(map->texture_list+tFloor))->w, fu, fv, &color);
                bilinear_filter( *(map->texture_list+tFloor), fu, fv, &color);
*/
            color = ((Uint32 *)(*(map->texture_list+tFloor))->pixels)[rd->tex_width * floorTexY + floorTexX];

            if(rd->render_light) darken_pixel(&color, fl_dist_lookup[x][y]);
            ((Uint32*)rd->frame_buffer->pixels)[rd->scr_width * y + x] = color;

            //draw ceiling, inverse of the floor
            color = ((Uint32 *)(*(map->texture_list+tCeil))->pixels)[rd->tex_width * floorTexY + floorTexX];
            if(rd->render_light) darken_pixel(&color, fl_dist_lookup[x][y]);
            ((Uint32*)rd->frame_buffer->pixels)[rd->scr_width * (rd->scr_height - y) + x] = color;
        }
    }

#ifdef DO_SPRITES
    //sprite casting
    int i;
    for(i = 0; i < NUMSPRITES; i++)
    {
        _spriteOrder[i] = i;
        _spriteDistance[i] = ((cam->pos_x - sprites[i].x) * (cam->pos_x * sprites[i].x) + (cam->pos_y - sprites[i].y) * (cam->pos_y - sprites[i].y));
    }
    RC_SpriteSort(_spriteOrder, _spriteDistance, NUMSPRITES);

    for(i = 0; i < NUMSPRITES; i++)
    {
        double spriteX = sprites[_spriteOrder[i]].x - cam->pos_x;
        double spriteY = sprites[_spriteOrder[i]].y - cam->pos_y;

      //transform sprite with the inverse camera matrix
      // [ plane_x   dir_x ] -1                                       [ dir_y      -dir_x ]
      // [               ]       =  1/(plane_x*dir_y-dir_x*plane_y) *   [                 ]
      // [ plane_y   dir_y ]                                          [ -plane_y  plane_x ]
        double invDet = 1.0 / (cam->plane_x * cam->dir_y - cam->dir_x * cam->plane_y);
        double transformX = invDet * (cam->dir_y * spriteX - cam->dir_x * spriteY);
        double transformY = invDet * (-cam->plane_y * spriteX + cam->plane_x * spriteY);

        int spriteScreenX = (int)((rd->scr_width>>1) * (1 + transformX / transformY));
        int spriteHeight = abs((int)(rd->scr_height / transformY));
        int drawStartY = (-spriteHeight >>1) + scr_h2;
        int drawEndY = (spriteHeight >>1) + scr_h2;

        if(drawStartY < 0) drawStartY = 0;
        if(drawEndY >= rd->scr_height) drawEndY = rd->scr_height - 1;

        int spriteWidth = abs((int)(rd->scr_height / transformY));
        int drawStartX = (-spriteWidth >>1) + spriteScreenX;
        int drawEndX = (spriteWidth >>1) + spriteScreenX;

        if(drawStartX < 0) drawStartX = 0;
        if(drawEndX >= rd->scr_width) drawEndX = rd->scr_width - 1;

        int stripe, y;
        for(stripe = drawStartX; stripe < drawEndX; stripe++)
        {
            int texX = (int)( 256 * ((stripe - ((-spriteWidth >>1) + spriteScreenX)) * rd->tex_width / spriteWidth) >>8);
            if(transformY > 0 && stripe > 0 && stripe < rd->scr_width && transformY < _ZBuffer[stripe])
                for(y = drawStartY; y < drawEndY; y++)
                {
                    int d = y * 256 - rd->scr_height * 128 + spriteHeight * 128;
                    int texY = ((d * rd->tex_height) / spriteHeight) >> 8;
                    if(texX > rd->tex_width) texX = rd->tex_width-1;
                    if(texY > rd->tex_height) texY = rd->tex_height-1;
                    Uint32 color = ((Uint32 *)(*(map->texture_list+sprites[_spriteOrder[i]].texture))->pixels)[rd->tex_width * texY + texX];
                    if((color & 0x00FFFFFF) != 0) ((Uint32*)rd->frame_buffer->pixels)[rd->scr_width * y + stripe] = color; //inefficient, need to trim sprites
                }
        }

    }
#endif
}
/*  For when I remember what I was doing
void
RC_VoxelCast(SDL_)
{

}
*/
void
RC_RaycastEngineCleanup(RC_RenderData *rd)
{
    free(fc_lookup);
    free(cam_x_lookup);

    int x = 0;
    for(; x < rd->scr_width; x++)
        free(fl_dist_lookup[x]);
    free(fl_dist_lookup);
}
