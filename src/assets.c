#include "assets.h"

game_assets_t g_assets;

Model load_model(const char *path)
{
    Model m = LoadModel(path);
    m.materials[0].maps[MATERIAL_MAP_ALBEDO].texture = g_assets.modelAtlas;
    return m;
}

void assets_load(void)
{
    g_assets.modelAtlas = LoadTexture("resources/atlas.png");
    SetTextureFilter(g_assets.modelAtlas, TEXTURE_FILTER_BILINEAR);
    g_assets.hex_grass = load_model("resources/h_grass.glb");
    g_assets.hex_water = load_model("resources/h_water.glb");
    g_assets.pawns[0] = load_model("resources/pawn.glb");
    g_assets.pawns[1] = load_model("resources/pawn-2.glb");
    g_assets.pawn_shadow = load_model("resources/pawn-shadow.glb");
    g_assets.confirs[0] = load_model("resources/prop_confir-1.glb");
    g_assets.confirs[1] = load_model("resources/prop_confir-2.glb");
    g_assets.confirs[2] = load_model("resources/prop_confir-3.glb");
    
}