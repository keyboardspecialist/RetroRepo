#include "RC_Resources.h"
#include "RC_Debug.h"
#include <stdlib.h>
#include <string.h>
#include <time.h>

typedef struct
{
    Uint32 r1_cutoff, r2_cutoff;
    Uint32 reps;
}gen_params;

RC_Map *grid, *grid2;
Uint32 fillprob;
gen_params *params;
int generations;

Uint32
rpick(void)
{
    if(rand() % 100 < fillprob)
        return F_WALL;
    else
        return F_FLOOR;
}


void
initmap(int sx, int sy)
{
    int xi, yi;

    grid = malloc(sizeof(RC_Map));
    grid2 = malloc(sizeof(RC_Map));
    grid->mapsize_y = sy;
    grid->mapsize_x = sx;
    grid2->mapsize_y = sy;
    grid2->mapsize_x = sx;
    grid->map_data = malloc(sizeof(RC_MapCell) * grid->mapsize_x);
    grid2->map_data = malloc(sizeof(RC_MapCell) * grid2->mapsize_x);
    grid->map_name = NULL;
    grid2->map_name = NULL;
    grid->texture_file_names = NULL;
    grid2->texture_file_names = NULL;
    grid->texture_list = NULL;
    grid2->texture_list = NULL;
    grid2->texture_count = 0;

    for(xi = 0; xi < grid->mapsize_x; xi++)
    {
        grid->map_data[xi] = malloc(sizeof(RC_MapCell) * grid->mapsize_y);
        grid2->map_data[xi] = malloc(sizeof(RC_MapCell) * grid2->mapsize_y);
    }

    //random fill with probability
    for(xi = 1; xi < grid->mapsize_x; xi++)
    for(yi = 1; yi < grid->mapsize_y; yi++)
        grid->map_data[xi][yi].flags = rpick();

    //wall fill
    for(xi = 0; xi < grid2->mapsize_x; xi++)
    for(yi = 0; yi < grid2->mapsize_y; yi++)
        grid2->map_data[xi][yi].flags = F_WALL;

    //set borders to walls, there is no escape
    for(xi = 0; xi < grid->mapsize_x; xi++)
        grid->map_data[xi][0].flags = grid->map_data[xi][grid->mapsize_y-1].flags = F_WALL;
    for(yi = 0; yi < grid->mapsize_y; yi++)
        grid->map_data[0][yi].flags = grid->map_data[grid->mapsize_x-1][yi].flags = F_WALL;

RC_ConsoleDebugOut(NULL, "Cave Gen Init Finish...");
}

void
generation(void)
{
    int xi, yi, ii, jj;

    for(xi = 1; xi < grid->mapsize_x-1; xi++)
    for(yi = 1; yi < grid->mapsize_y-1; yi++)
    {
        Uint32 adjcount_r1 = 0, adjcount_r2 = 0;
        for(ii = -1; ii <= 1; ii++)
        for(jj = -1; jj <= 1; jj++)
        {
            if(!(grid->map_data[xi+ii][yi+jj].flags & F_FLOOR))
                adjcount_r1++;
        }
        for(ii=xi-2; ii <= xi+2; ii++)
        for(jj=yi-2; jj <= yi+2; jj++)
        {

            if(abs(ii-xi) == 2 && abs(jj-yi) == 2)
                continue;
            if(ii < 0 || jj < 0 || ii >= grid->mapsize_x || jj >= grid->mapsize_y)
                continue;
            if(!(grid->map_data[ii][jj].flags & F_FLOOR))
                adjcount_r2++;
        }
        if(adjcount_r1 >= params->r1_cutoff || adjcount_r2 <= params->r2_cutoff)
            grid2->map_data[xi][yi].flags = F_WALL;
        else
            grid2->map_data[xi][yi].flags = F_FLOOR;
    }

    for(xi = 1; xi < grid->mapsize_x; xi++)
    for(yi = 1; yi < grid->mapsize_y; yi++)
        grid->map_data[xi][yi].flags = grid2->map_data[xi][yi].flags;
RC_ConsoleDebugOut(NULL,"Cave Gen genpass finish...");
}

/* Simple texture pass. Basic for now while testing
 *
 */
void
texturePass(void) //TODO: FIX FLOOR AND CEIL TEXTURE ISSUE
{
    int i, j;
    Uint32 walltex =    0x00000003;
    Uint32 walltex2 =   0x00000003;
    Uint32 flctex =     0x00010200; //floor and ceil go together
    //char *txts[3] = {"textures/cave/t/swall.bmp", "textures/cave/t/sfloor.bmp", "textures/cave/t/sceil.bmp"};
    //grid->texture_list = RC_LoadTextures(txts, 3, &grid->texture_count);
    grid->texture_list = RC_LoadTexturesFromFolder("textures/cave/t/", &grid->texture_file_names, &grid->texture_count);

    for(i = 0; i < grid->mapsize_x; i++)
    for(j = 0; j < grid->mapsize_y; j++)
    {
        if(grid->map_data[i][j].flags & F_WALL)
        {
            if(rand() % 100 < 80)
                grid->map_data[i][j].texture_data = walltex;
            else
                grid->map_data[i][j].texture_data = walltex2;
        }
        else if(grid->map_data[i][j].flags & F_FLOOR)
        {
            grid->map_data[i][j].texture_data = flctex;
        }

    }
}

//find a starting location
//hopefully reliable **WILL CRASH**
void
findStart(void)
{
    Uint32 sx, sy;
    int done = 0, tries = 0;
    int space = 2;
    srand(time(NULL));

    int gi = 1;
RC_ConsoleDebugOut(NULL,"Cave Gen find start... [%d]", gi++);

    //randomly choose spots on the map, if its a floor tile check around it to make sure theres some room
    //if we keep failing, just pick one and be done with it
    while(!done && tries < 100)
    {
RC_ConsoleDebugOut(NULL,"Cave Gen find start... [%d]", gi++);
        sx = rand() % (grid->mapsize_x - space) + space;
        sy = rand() % (grid->mapsize_y - space) + space;
        if(grid->map_data[sx][sy].flags & F_FLOOR)
        {
            grid->start_x = sx + .5;
            grid->start_y = sy + .5;
            int x, y;
            for(x = sx-space; x < sx+space; x++)
            for(y = sy-space; y < sy+space; y++)
                if(grid->map_data[x][y].flags & F_FLOOR)
                {
                    done = 1;
                    continue;
                }
                else
                {
                    done = 0;
                    tries++;
                    break;
                }
        }
        else
        {
            tries++;
        }
    }
}

void
printMap()
{
    int i, j;
    for(i = 0; i < grid->mapsize_x; i++)
    {
        for(j = 0; j < grid->mapsize_y; j++)
        {
            printf(" %c", grid->map_data[i][j].flags & F_WALL ? '#' : '.');
        }
        puts("");
    }
}

RC_Map*
RC_MapGenCave(Uint32 seed, int x, int y, Uint32 fill, Uint32 r1, Uint32 r2, Uint32 reps)
{
RC_ConsoleDebugOut(NULL,"\nCave Gen Start...\n");
    Uint32 i;
    char name_buf[MAXNAMELEN];
    fillprob = fill;
    srand(seed);
    initmap(x, y);
    params = malloc(sizeof(gen_params));
    params->r1_cutoff = r1;
    params->r2_cutoff = r2;
    params->reps = reps;

    for(i = 0; i < params->reps; i++)
        generation();

    texturePass();
    findStart();

    sprintf(name_buf, "Random Cave Seed #%u", seed);
    grid->map_name = malloc(sizeof(char) * strlen(name_buf) + 1);
    strcpy(grid->map_name, name_buf);
    RC_FreeMap(grid2);
    free(params);

printMap();
    return grid;
}
