#include "RC_Resources.h"


/* Load texture memory
 *
 *
 */
SDL_Surface**
RC_LoadTextures(char **list, Uint32 tex_count, Uint32 *text_count_store)
{
    Uint32 i;
    SDL_Surface *s_tmp;
    SDL_Surface **tex_list;
    tex_list = malloc( sizeof(SDL_Surface*) * tex_count );
    for(i = 0; i < tex_count; i++)
    {
        s_tmp = IMG_Load(list[i]);
        if(s_tmp)
            printf("\n Texture %s Loaded ...\n", list[i]);

        tex_list[i] = SDL_DisplayFormatAlpha(s_tmp);
        SDL_FreeSurface(s_tmp);
    }

    *text_count_store = i;
    return tex_list;
}

//load textures directly from a folder
SDL_Surface**
RC_LoadTexturesFromFolder(char *fpath, char ***files, Uint32 *text_count_store) //relative path
{
    Uint32 fcnt = 0, i=0;
    char **fList;
    DIR *tfldr = opendir(fpath);
    struct dirent *ent;

    if(tfldr != NULL)
    {
        //count the files
        while((ent = readdir(tfldr)) != NULL)
        {
            fcnt++;
        }
        if(fcnt > 0)
        {
            fList = malloc(sizeof(char*) * fcnt);
            rewinddir(tfldr);
            while((ent = readdir(tfldr)) != NULL)
            {
                if(strcmp(ent->d_name, ".") != 0 && strcmp(ent->d_name, "..") != 0)
                {
                    fList[i] = malloc(sizeof(char) * (strlen(fpath) + strlen(ent->d_name)+1));
                    strcpy(fList[i], fpath);
                    strcat(fList[i], ent->d_name);
                    i++;
                }
            }
        }
        else
        {
            return NULL;
        }

        *files = fList;
        int j = 0;
        for(; j < i; j++)
            printf("\n[fList] Files Read %s...\n", fList[j]);
        j = 0;
        for(; j < i; j++)
            printf("\n[files] Files Read %s...\n", (*(*files+j)) );
        closedir(tfldr);
        free(ent);
        return RC_LoadTextures(fList, i, text_count_store);
    }
    return NULL;
}

/* Free map memory
 *
 *
 */
void
RC_FreeMap(RC_Map *map)
{
    Uint32 i;
    if(map->texture_list != NULL)
    {
        for(i = 0; i < map->texture_count; i++)
            SDL_FreeSurface(map->texture_list[i]);

    }
    free(map->texture_list);
    if(map->texture_count > 0)
    {
        for(i = 0; i < map->texture_count; i++)
            free(map->texture_file_names[i]);

    }
    free(map->texture_file_names);
    if(map->map_data != NULL)
    {
        for(i = 0; i < map->mapsize_x; i++)
            free(map->map_data[i]);

    }
    free(map->map_data);
    free(map->map_name);
    free(map);
}


int
RC_SaveMap(RC_Map *map)
{
    FILE *fp;
    char *noname = "noname.map";
    char *null = "\0";
    char fbuf[128];
    if(map->map_name == NULL)
    {
        map->map_name = malloc(sizeof(char) * strlen(noname) + 1);
        strcpy(map->map_name, noname);
    }

    sprintf(fbuf, "./maps/%s.map", map->map_name);
    if((fp = fopen(fbuf, "wb")) != NULL)
    {
    RC_ConsoleDebugOut(NULL,"Saving map %s in '/maps' ...", map->map_name);
        //write name
        fwrite(map->map_name, sizeof(char), strlen(map->map_name), fp);

        //pad
        int c = 0;
        for(; c < MAXNAMELEN - strlen(map->map_name); c++)
            fwrite(null, sizeof(char), 1, fp);

        fseek(fp, OFFSET_TEXTURES, SEEK_SET);

        fwrite(&map->texture_count, sizeof(Uint32), 1, fp);
        int i=0;
        for(; i < map->texture_count; i++)
        {
            fwrite(map->texture_file_names[i], sizeof(char), strlen(map->texture_file_names[i]), fp);
            RC_ConsoleDebugOut(NULL, "\nWriting texture '%s' %d of %d...\n", map->texture_file_names[i], i+1, map->texture_count);
            fwrite(null, sizeof(char), 1, fp);
        }

    RC_ConsoleDebugOut(NULL,"Writing map data, X [%d] Y [%d] ...", map->mapsize_x, map->mapsize_y);
        fwrite(&map->mapsize_x, sizeof(Uint32), 1, fp);
        fwrite(&map->mapsize_y, sizeof(Uint32), 1, fp);
        fwrite(&map->start_x, sizeof(double), 1, fp);
        fwrite(&map->start_y, sizeof(double), 1, fp);
        int x, y;
        for(x=0; x < map->mapsize_x; x++)
        for(y=0; y < map->mapsize_y; y++)
            fwrite(&map->map_data[x][y], sizeof(RC_MapCell), 1, fp);

    RC_ConsoleDebugOut(NULL,"Writing Finished...");
        fclose(fp);
        return TRUE;
    }
    return FALSE;
}
/* Parse binary file and generate our map
 *
 * Offsets
 * 0x00 - map name/description
 * 0x40 - texture list size
 * 0x44 - texture list
 * end of texture list + 1 - map data
 */
RC_Map*
RC_LoadMap(const char *map_file)
{
    FILE *fp;
    RC_Map *new_map;
    if((fp = fopen(map_file, "rb")) != NULL)
    {
    RC_ConsoleDebugOut(NULL,"Loading '%s' ...", map_file);
        new_map = malloc(sizeof(RC_Map));
        char buf[MAXNAMELEN];
        int i = 0;
        do
        {
            fread(&buf[i], sizeof(char), 1, fp);
            i++;
        }while(buf[i-1] != '\0' && i < MAXNAMELEN);
    RC_ConsoleDebugOut(NULL,"Map name '%s' ...", buf[i]);
        if(strlen(buf) > 0)
        {
            new_map->map_name = malloc(sizeof(char) * (strlen(buf) + 1));
            strcpy(new_map->map_name, buf);
        }
        else
            new_map->map_name = NULL;

        fseek(fp, OFFSET_TEXTURES, SEEK_SET);
        i = 0;
        char **tl;
        char tbuf[MAXNAMELEN+1];
        Uint32 tc;
        int j = 0;
        fread(&tc, sizeof(Uint32), 1, fp);
        tl = malloc(sizeof(char*) * tc);
        for(; i < tc;)
        {
            char p;
            fread(&p, sizeof(char), 1, fp);
            tbuf[j++] = p;
            if(p=='\0')
            {
                tl[i] = malloc(sizeof(char) * j);
                strcpy(tl[i], tbuf);
                memset(&tbuf, 0, MAXNAMELEN+1);
                i++;
                j=0;
            }
        }
        new_map->texture_file_names = tl;
        new_map->texture_list = RC_LoadTextures(tl, tc, &new_map->texture_count);
    RC_ConsoleDebugOut(NULL,"Loaded %d textures...", tc);

        fread(&new_map->mapsize_x, sizeof(Uint32), 1, fp);
        fread(&new_map->mapsize_y, sizeof(Uint32), 1, fp);
        fread(&new_map->start_x, sizeof(double), 1, fp);
        fread(&new_map->start_y, sizeof(double), 1, fp);
    RC_ConsoleDebugOut(NULL,"Map X [%d] Y [%d] : Start X [%3.2f] Y [%3.2f] ...", new_map->mapsize_x, new_map->mapsize_y, new_map->start_x, new_map->start_y);
        new_map->map_data = malloc(sizeof(RC_MapCell) * new_map->mapsize_x);
        int k = 0;
        for(; k < new_map->mapsize_x; k++)
            new_map->map_data[k] = malloc(sizeof(RC_MapCell) * new_map->mapsize_y);

        int x, y;
        for(x=0; x < new_map->mapsize_x; x++)
        for(y=0; y < new_map->mapsize_y; y++)
            fread(&new_map->map_data[x][y], sizeof(RC_MapCell), 1, fp);
    RC_ConsoleDebugOut(NULL,"Loading Finished...");
        fclose(fp);
    }
    else
    {
        RC_ConsoleDebugOut(NULL,"Map file maps/%s not found", map_file);
        return NULL;
    }
    return new_map;
}
