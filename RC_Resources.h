#ifndef __RCRESOURCES_H__
#define __RCRESOURCES_H__

#include "RC_GlobalDefs.h"
#include "RC_Debug.h"

#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include <string.h>
#include <dirent.h>

/* Map file constants and offsets
 *
 */
#define MAXNAMELEN 60
#define OFFSET_TEXTURES 0x40

/*
 * MapCell flags and defines
 */
#define M_TEXTURE(t) (t & 0x0000FFFF)
#define M_FLOOR(f)   (f & 0x0000FF00)>>8
#define M_CEIL(c)    (c & 0x00FF0000)>>16
#define M_ALPHA(a)   (a & 0xFF000000)

#define F_WALL 0x00000001
#define F_FLOOR 0x00000002
#define F_SPRITE 0x00000004
#define F_SEEN   0x00000008


/*
 *  MapCell struct
 *  Holds cell texture and flag data.
 *  _TEXTUREMASK(textureData) = integer value of texture
 *  _CEILTEXTUREMASK(textureData) = integer value of ceiling texture (only applicable to floor cells)
 *  _ALPHAMASK(textureData) = alpha transparency of texture
 *  flags - defines texture and object behavior
 */
typedef struct _RC_MapCell
{
    Uint32 texture_data;
    Uint32 flags;
}RC_MapCell;

typedef struct _RC_Map
{
    char *map_name;
    RC_MapCell **map_data;
    SDL_Surface **texture_list;
    char **texture_file_names;
    Uint32 texture_count;
    Uint32 mapsize_x;
    Uint32 mapsize_y;
    double start_x;
    double start_y;
}RC_Map;


typedef struct _RC_Sprite
{
  double x;
  double y;
  Uint32 texture;
  Uint32 flags;
}RC_Sprite;


extern SDL_Surface** RC_LoadTextures(char **list, Uint32 tex_count, Uint32 *text_count_store);
extern SDL_Surface** RC_LoadTexturesFromFolder(char *fpath, char ***files, Uint32 *text_count_store);
extern void RC_FreeMap(RC_Map *map);
extern int RC_SaveMap(RC_Map *map);
extern RC_Map* RC_LoadMap(const char *mapFile);
#endif
