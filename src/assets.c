#include "assets.h"

game_assets_t g_assets;


Model load_model(const char *path)
{
    Model m = LoadModel(TextFormat("resources/%s.glb",path));
    m.materials[0].maps[MATERIAL_MAP_ALBEDO].texture = g_assets.modelAtlas;
    return m;
}

model_data_t load_model_hull(const char *path)
{
    Model m = load_model(path);
    
    b3Vec3 *hull_points;
    int point_count = 0;
    for (int i = 0; i < m.meshCount; i++)
    {
        point_count += m.meshes[i].vertexCount;
    }
    hull_points = MemAlloc(sizeof(hull_points) * point_count);
    point_count = 0;
    // add points but weed out duplicated ones
    for (int i = 0; i < m.meshCount; i++)
    {
        for (int j = 0; j < m.meshes[i].vertexCount; j++)
        {
            Vector3 p = {
                m.meshes[i].vertices[j * 3],
                m.meshes[i].vertices[j * 3 + 1],
                m.meshes[i].vertices[j * 3 + 2],
            };
            for (int k = 0; k < point_count; k++)
            {
                float dx = p.x - hull_points[k].x;
                float dy = p.y - hull_points[k].y;
                float dz = p.z - hull_points[k].z;
                float dsq = dx * dx + dy * dy + dz * dz;
                if (dsq < 0.05f) goto ignore;
            }
            hull_points[point_count++] = (b3Vec3){
                p.x, p.y, p.z
            };
            ignore:
        }
    }

    b3HullData *hull = b3CreateHull(hull_points, point_count, 24);


    MemFree(hull_points);
    return (model_data_t){
        .model = m,
        .hulldata = hull,
    };
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
    g_assets.pawns[0].body = load_model("pawn");
    g_assets.pawns[0].hand = load_model("pawn_hand");
    g_assets.pawns[1].body = load_model("pawn-2");
    g_assets.pawns[1].hand = load_model("pawn_hand");
    g_assets.backpack = load_model("pawn-backpack");

    g_assets.pawn_shadow = load_model("pawn-shadow");
    g_assets.tree_shadow = load_model("prop_tree_shadow");
    g_assets.conifirs[g_assets.conifir_count++] = load_model_hull("prop_conifir-1");
    g_assets.conifirs[g_assets.conifir_count++] = load_model_hull("prop_conifir-2");
    g_assets.conifirs[g_assets.conifir_count++] = load_model_hull("prop_conifir-1a");
    g_assets.conifirs[g_assets.conifir_count++] = load_model_hull("prop_conifir-2a");
    g_assets.conifirs[g_assets.conifir_count++] = load_model_hull("prop_conifir-3");
    g_assets.conifirs[g_assets.conifir_count++] = load_model_hull("prop_conifir-4");
    g_assets.conifirs[g_assets.conifir_count++] = load_model_hull("prop_tree-stump");

    g_assets.trees[g_assets.tree_count++] = load_model_hull("prop_tree-1");
    g_assets.trees[g_assets.tree_count++] = load_model_hull("prop_tree-2");
    g_assets.trees[g_assets.tree_count++] = load_model_hull("prop_tree-3");
    g_assets.trees[g_assets.tree_count++] = load_model_hull("prop_tree-4");
    g_assets.trees[g_assets.tree_count++] = load_model_hull("prop_tree-stump");
    g_assets.trees[g_assets.tree_count++] = load_model_hull("prop_tree-dead");
    
    g_assets.rocks[g_assets.rocks_count++] = load_model_hull("prop_rock");
    g_assets.rocks[g_assets.rocks_count++] = load_model_hull("prop_rock.001");
    
    g_assets.log = load_model_hull("log");
    g_assets.rock_item = load_model_hull("rock-item");
    g_assets.high_grass[g_assets.high_grass_count++] = load_model("prop-high-grass");

    int tcnt = 0;
    g_assets.tri_hexes[tcnt++] = load_tri_hex("t_grass_000", MAP_TYPE_GRASS, MAP_TYPE_GRASS, MAP_TYPE_GRASS);
    g_assets.tri_hexes[tcnt++] = load_tri_hex("t_grass_mud_001", MAP_TYPE_GRASS, MAP_TYPE_GRASS, MAP_TYPE_MUD);
    g_assets.tri_hexes[tcnt++] = load_tri_hex("t_grass_mud_010", MAP_TYPE_GRASS, MAP_TYPE_MUD, MAP_TYPE_GRASS);
    g_assets.tri_hexes[tcnt++] = load_tri_hex("t_grass_mud_011", MAP_TYPE_GRASS, MAP_TYPE_MUD, MAP_TYPE_MUD);
    g_assets.tri_hexes[tcnt++] = load_tri_hex("t_water_000", MAP_TYPE_WATER, MAP_TYPE_WATER, MAP_TYPE_WATER);
    g_assets.tri_hexes[tcnt++] = load_tri_hex("t_water_mud_001", MAP_TYPE_WATER, MAP_TYPE_WATER, MAP_TYPE_MUD);
    g_assets.tri_hexes[tcnt++] = load_tri_hex("t_water_mud_011", MAP_TYPE_WATER, MAP_TYPE_MUD, MAP_TYPE_MUD);
    g_assets.tri_hexes[tcnt++] = load_tri_hex("t_water_mud_010", MAP_TYPE_WATER, MAP_TYPE_MUD, MAP_TYPE_WATER);
    g_assets.tri_hex_count = tcnt;

    g_assets.crate = load_model("crate");
}