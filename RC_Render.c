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
    if(a < 15) a = 15;
    Uint32 c1 = *pix;
    Uint32 c2 = (c1&0xFF00FF) * a >>8;
    c1 = (c1&0xFF00FF00) * a >>8;
    *pix =((c1&0xFF00FF00) | (c2&0xFF00FF));
}

void
RC_RaycastDraw(RC_RenderData *rd, RC_Map *map, RC_PlayerCamera *cam, RC_Sprite *sprites)
{
    double cameraX, rayPosX, rayPosY, rayDirX, rayDirY, sideDistX,
            sideDistY, deltaDistX, deltaDistY, perpWallDist;
    int mapX, mapY, stepX, stepY, hit, side, lineHeight, drawStart, drawEnd;
    int x;
 //   double _ZBuffer[rd->scr_width];

//printf("map - %s \n mapx %d mapy %d txcnt %d \n", map->map_name, map->mapsize_x, map->mapsize_y, map->texture_count);

//printf("render data: screen:%d fbuf:%d screenw %d screenh %d bits %d", rd->screen!=NULL, rd->frame_buffer!=NULL, rd->scr_width, rd->scr_height, rd->bit_depth);

    for(x = 0; x < rd->scr_width; x++)
    {
        cameraX = 2 * x / (double)rd->scr_width - 1;
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


        lineHeight = abs( (int)( (rd->scr_height * rd->wall_scale) / perpWallDist) );
        drawStart = -lineHeight / 2 + rd->scr_height / 2;
        drawEnd = lineHeight / 2 + rd->scr_height / 2;
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

        wallX -= floor(wallX);
        texX = (int)(wallX * (double)rd->tex_width);
        if(side == 0 && rayDirX > 0) texX = rd->tex_width - texX - 1;
        if(side == 1 && rayDirY < 0) texX = rd->tex_width - texX - 1;


        int y;
        for(y = drawStart; y < drawEnd; y++)
        {
            int d = y * 256 - rd->scr_height * 128 + lineHeight * 128;
            int texY = ((d * rd->tex_height) / lineHeight) >>8;
            Uint32 color = ((Uint32 *)(*(map->texture_list+useTexture))->pixels)[rd->tex_height * texY + texX];
            //if(side == 1) color = (color >> 1) & 8355711; //magic

            if(rd->render_light) darken_pixel(&color,2*perpWallDist + (perpWallDist*fabs(cameraX*cameraX)));
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
            currentDist = rd->scr_height / (2.0 * y - rd->scr_height); //consider lookup table here
            double weight = (currentDist - distPlayer) / (distWall - distPlayer);
            double currentFloorX = weight * floorXWall + (1.0 - weight) * cam->pos_x;
            double currentFloorY = weight * floorYWall + (1.0 - weight) * cam->pos_y;
            tFloor = (M_FLOOR((map->map_data[(int)currentFloorX][(int)currentFloorY]).texture_data))-1;
            tCeil = (M_CEIL((map->map_data[(int)currentFloorX][(int)currentFloorY]).texture_data))-1;

            int floorTexX, floorTexY;
            floorTexX = (int)(((int)(currentFloorX * rd->tex_width)) % rd->tex_width);
            floorTexY = (int)(((int)(currentFloorY * rd->tex_height)) % rd->tex_height);
            //draw floor
            Uint32 color = ((Uint32 *)(*(map->texture_list+tFloor))->pixels)[rd->tex_width * floorTexY + floorTexX]; //RGBA >> ARGB
            //color = (color >> 1) & 8355711; //magic
            if(rd->render_light) darken_pixel(&color, 2*currentDist + (currentDist*fabs(cameraX*cameraX)));
            ((Uint32*)rd->frame_buffer->pixels)[rd->scr_width * y + x] = color;

            //draw ceiling, inverse of the floor
            color = ((Uint32 *)(*(map->texture_list+tCeil))->pixels)[rd->tex_width * floorTexY + floorTexX];
            if(rd->render_light) darken_pixel(&color, 2*currentDist + (currentDist*fabs(cameraX*cameraX)));
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
        int drawStartY = (-spriteHeight >>1) + rd->scr_height / 2;
        int drawEndY = (spriteHeight >>1) + rd->scr_height / 2;

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



void
RC_RaycastEngineCleanup(void)
{

}
