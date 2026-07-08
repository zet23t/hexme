#ifndef __ASSETS_H__
#define __ASSETS_H__

#include <raylib.h>
#include "box3d/box3d.h"

typedef enum {
    MAP_TYPE_NONE,
    MAP_TYPE_GRASS,
    MAP_TYPE_WATER,
    MAP_TYPE_MUD,
} map_type_e;

typedef struct {
    Model model;
    map_type_e center, corner_a, corner_b;
} tri_hex_t;

typedef struct {
    Model body;
    Model hand;
} pawn_t;

typedef struct
{
    Model model;
    int tickets;
} biome_element_t;

#define MAX_BIOME_ELEMENTS 16
#define MAX_BIOME_DEFS 8

typedef struct {
    map_type_e matched_biomes[3];
    int min_biome_corner_matches[6];
    int max_biome_corner_matches[6];
    biome_element_t biome_elements[MAX_BIOME_ELEMENTS];
    int biome_element_count;
} biome_def_t;

typedef struct {
    Model model;
    b3HullData *hulldata;
} model_data_t;

typedef struct {
    Texture2D modelAtlas;
    Model hex_grass;    
    Model hex_water;    
    pawn_t pawns[2];
    Model pawn_shadow;    
    Model tree_shadow;    

    Model crate;

    model_data_t conifirs[14];    
    int conifir_count;
    
    model_data_t trees[14];    
    int tree_count;

    model_data_t rocks[4];
    int rocks_count;    

    Model high_grass[8];
    int high_grass_count;

    tri_hex_t tri_hexes[32];
    int tri_hex_count;

    biome_def_t biome_defs[MAX_BIOME_DEFS];
    int biome_def_count;

} game_assets_t;

extern game_assets_t g_assets;

void assets_load(void);

#endif