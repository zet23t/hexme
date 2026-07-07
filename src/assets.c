#include "assets.h"

game_assets_t g_assets;


Model load_model(const char *path)
{
    Model m = LoadModel(TextFormat("resources/%s.glb",path));
    m.materials[0].maps[MATERIAL_MAP_ALBEDO].texture = g_assets.modelAtlas;
    return m;
}

tri_hex_t load_tri_hex(const char *name, map_type_e center, map_type_e a, map_type_e b)
{
    Model m = load_model(name);
    return (tri_hex_t){
        .model = m, .center = center, .corner_a = a, .corner_b = b
    };
}

void assets_load(void)
{
    g_assets.modelAtlas = LoadTexture("resources/atlas.png");
    SetTextureFilter(g_assets.modelAtlas, TEXTURE_FILTER_BILINEAR);
    g_assets.hex_grass = load_model("h_grass");
    g_assets.hex_water = load_model("h_water");
    g_assets.pawns[0] = load_model("pawn");
    g_assets.pawns[1] = load_model("pawn-2");
    g_assets.pawn_shadow = load_model("pawn-shadow");
    g_assets.confirs[0] = load_model("prop_confir-1");
    g_assets.confirs[1] = load_model("prop_confir-2");
    g_assets.confirs[2] = load_model("prop_confir-3");

    int tcnt = 0;
    g_assets.tri_hexes[tcnt++] = load_tri_hex("t_grass_mud_001", MAP_TYPE_GRASS, MAP_TYPE_GRASS, MAP_TYPE_MUD);
    g_assets.tri_hexes[tcnt++] = load_tri_hex("t_grass_mud_010", MAP_TYPE_GRASS, MAP_TYPE_MUD, MAP_TYPE_GRASS);
    g_assets.tri_hexes[tcnt++] = load_tri_hex("t_grass_mud_011", MAP_TYPE_GRASS, MAP_TYPE_MUD, MAP_TYPE_MUD);
    
}