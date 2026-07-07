#ifndef __ASSETS_H__
#define __ASSETS_H__

#include <raylib.h>

typedef enum {
    MAP_TYPE_GRASS,
    MAP_TYPE_WATER,
    MAP_TYPE_MUD,
} map_type_e;

typedef struct {
    Model model;
    map_type_e center, corner_a, corner_b;
} tri_hex_t;

typedef struct {
    Texture2D modelAtlas;
    Model hex_grass;    
    Model hex_water;    
    Model pawns[2];
    Model pawn_shadow;    
    Model confirs[3];    

    tri_hex_t tri_hexes[8];

} game_assets_t;

extern game_assets_t g_assets;

void assets_load(void);

#endif