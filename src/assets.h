#ifndef __ASSETS_H__
#define __ASSETS_H__

#include <raylib.h>



typedef struct {
    Texture2D modelAtlas;
    Model hex_grass;    
    Model hex_water;    
    Model pawns[2];    
    Model confirs[3];    

} game_assets_t;

extern game_assets_t g_assets;

void assets_load(void);

#endif