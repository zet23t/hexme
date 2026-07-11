/*******************************************************************************************
*
*   raylib game template
*
*
*   Code licensed under an unmodified zlib/libpng license, which is an OSI-certified,
*   BSD-like license that allows static linking with closed source software
*
*   Copyright (c) 2021-2026 Ramon Santamaria (@raysan5)
*
********************************************************************************************/
#include "raylib.h"
#include "raymath.h"
#include "rlgl.h"
#include <math.h>
#include "box3d/box3d.h"
#include <inttypes.h>
#include "screens.h"    // NOTE: Declares global (extern) variables and screens functions
#include "assets.h"

#if defined(PLATFORM_WEB)
    #include <emscripten/emscripten.h>      // Emscripten library
#endif

#include <stdio.h>                          // Required for: printf()
#include <stdlib.h>                         // Required for: 
#include <string.h>                         // Required for:

//----------------------------------------------------------------------------------
// Defines and Macros
//----------------------------------------------------------------------------------
// Simple log system to avoid printf() calls if required
// NOTE: Avoiding those calls, also avoids const strings memory usage
#define SUPPORT_LOG_INFO
#if defined(SUPPORT_LOG_INFO)
    #define LOG(...) printf(__VA_ARGS__)
#else
    #define LOG(...)
#endif

//----------------------------------------------------------------------------------
// Shared Variables Definition (global)
// NOTE: Those variables are shared between modules through screens.h
//----------------------------------------------------------------------------------
GameScreen currentScreen = LOGO;
Font font = { 0 };
Music music = { 0 };
Sound fxCoin = { 0 };

typedef enum {
    GROUP_INTERACTIBLE = 1
} collision_group_t;

typedef enum {
    UD_TREE = 1,
    UD_CONIFER,
    UD_LOG,
    UD_ROCK,
    UD_ROCKITEM,
} userdata_info_t;

//----------------------------------------------------------------------------------
// Global Variables Definition (local to this module)
//----------------------------------------------------------------------------------
static const int screenWidth = 720;
static const int screenHeight = 720;

// Required variables to manage screen transitions (fade-in, fade-out)
static float transAlpha = 0.0f;
static bool onTransition = false;
static bool transFadeOut = false;
static int transFromScreen = -1;
static GameScreen transToScreen = UNKNOWN;

//----------------------------------------------------------------------------------
// Module Functions Declaration
//----------------------------------------------------------------------------------
static void ChangeToScreen(int screen);     // Change to screen, no transition effect

static void TransitionToScreen(int screen); // Request transition to next screen
static void UpdateTransition(void);         // Update transition effect
static void DrawTransition(void);           // Draw transition effect (full-screen rectangle)

static void UpdateDrawFrame(void);          // Update and draw one frame

//----------------------------------------------------------------------------------
// Program main entry point
//----------------------------------------------------------------------------------
int main(void)
{
    // Initialization
    //---------------------------------------------------------
    InitWindow(screenWidth, screenHeight, "raylib game template");

    InitAudioDevice();      // Initialize audio device

    // Load global data (assets that must be available in all screens, i.e. font)
    font = LoadFont("resources/mecha.png");
    //music = LoadMusicStream("resources/ambient.ogg"); // TODO: Load music
    fxCoin = LoadSound("resources/coin.wav");

    assets_load();

    SetMusicVolume(music, 1.0f);
    PlayMusicStream(music);

    // Setup and init first screen
    currentScreen = LOGO;
    InitLogoScreen();

#if defined(PLATFORM_WEB)
    emscripten_set_main_loop(UpdateDrawFrame, 60, 1);
#else
    SetTargetFPS(60);       // Set our game to run at 60 frames-per-second
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        UpdateDrawFrame();
    }
#endif

    // De-Initialization
    //--------------------------------------------------------------------------------------
    // Unload current screen data before closing
    switch (currentScreen)
    {
        case LOGO: UnloadLogoScreen(); break;
        case TITLE: UnloadTitleScreen(); break;
        case OPTIONS: UnloadOptionsScreen(); break;
        case GAMEPLAY: UnloadGameplayScreen(); break;
        case ENDING: UnloadEndingScreen(); break;
        default: break;
    }

    // Unload global data loaded
    UnloadFont(font);
    UnloadMusicStream(music);
    UnloadSound(fxCoin);

    CloseAudioDevice();     // Close audio context

    CloseWindow();          // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}

//----------------------------------------------------------------------------------
// Module Functions Definition
//----------------------------------------------------------------------------------
// Change to next screen, no transition
static void ChangeToScreen(int screen)
{
    // Unload current screen
    switch (currentScreen)
    {
        case LOGO: UnloadLogoScreen(); break;
        case TITLE: UnloadTitleScreen(); break;
        case OPTIONS: UnloadOptionsScreen(); break;
        case GAMEPLAY: UnloadGameplayScreen(); break;
        case ENDING: UnloadEndingScreen(); break;
        default: break;
    }

    // Init next screen
    switch (screen)
    {
        case LOGO: InitLogoScreen(); break;
        case TITLE: InitTitleScreen(); break;
        case OPTIONS: InitOptionsScreen(); break;
        case GAMEPLAY: InitGameplayScreen(); break;
        case ENDING: InitEndingScreen(); break;
        default: break;
    }

    currentScreen = screen;
}

// Request transition to next screen
static void TransitionToScreen(int screen)
{
    onTransition = true;
    transFadeOut = false;
    transFromScreen = currentScreen;
    transToScreen = screen;
    transAlpha = 0.0f;
}

// Update transition effect (fade-in, fade-out)
static void UpdateTransition(void)
{
    if (!transFadeOut)
    {
        transAlpha += 0.05f;

        // NOTE: Due to float internal representation, condition jumps on 1.0f instead of 1.05f
        // For that reason we compare against 1.01f, to avoid last frame loading stop
        if (transAlpha > 1.01f)
        {
            transAlpha = 1.0f;

            // Unload current screen
            switch (transFromScreen)
            {
                case LOGO: UnloadLogoScreen(); break;
                case TITLE: UnloadTitleScreen(); break;
                case OPTIONS: UnloadOptionsScreen(); break;
                case GAMEPLAY: UnloadGameplayScreen(); break;
                case ENDING: UnloadEndingScreen(); break;
                default: break;
            }

            // Load next screen
            switch (transToScreen)
            {
                case LOGO: InitLogoScreen(); break;
                case TITLE: InitTitleScreen(); break;
                case OPTIONS: InitOptionsScreen(); break;
                case GAMEPLAY: InitGameplayScreen(); break;
                case ENDING: InitEndingScreen(); break;
                default: break;
            }

            currentScreen = transToScreen;

            // Activate fade out effect to next loaded screen
            transFadeOut = true;
        }
    }
    else  // Transition fade out logic
    {
        transAlpha -= 0.02f;

        if (transAlpha < -0.01f)
        {
            transAlpha = 0.0f;
            transFadeOut = false;
            onTransition = false;
            transFromScreen = -1;
            transToScreen = UNKNOWN;
        }
    }
}

// Draw transition effect (full-screen rectangle)
static void DrawTransition(void)
{
    DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), Fade(BLACK, transAlpha));
}

typedef union {
    struct {
        uint32_t type:4;
        uint32_t subid:5;
        uint32_t id:20;
    };
    uint32_t raw;
} body_userdata_t;

typedef struct 
{
    b3BodyId kinematic;
    b3ShapeId forward_sensor;
    Vector3 current_pos, next_pos;
    Vector3 target_pos;
    Vector3 direction;
    float transition;
    int step;
    float size;
    int model;
    uint8_t has_backpack:1, action:1;
} pawn_instance_t;
#define PAWN_TRANSITION_T 0.2f
#define PAWN_JUMP_DIST 0.75f

#define HEX_X 6.925f
#define HEX_Y 6.0f

#define NUM_NPCS 4

typedef struct 
{
    int active;
    b3BodyId body;
    b3ShapeId shape;
    Vector3 scale;
} entity_t;

typedef struct 
{
    entity_t *entities;
    int entities_capacity;
    int entities_count;
} entity_list_t;

typedef struct 
{
    int is_initialized;
    pawn_instance_t player_pawn;
    pawn_instance_t npcs[NUM_NPCS];
    b3WorldId world_id;
    entity_list_t logs;
    entity_list_t rock_items;
} game_state_t;


typedef struct 
{
    uint8_t center, corners[6];
    uint8_t physics_initialized;
    uint16_t tree_bits;
    uint16_t conifer_bits;
    uint16_t rock_bits;
    uint16_t high_grass_bits;
} hex_cell_t;
game_state_t g_game;

void entity_list_draw_all(entity_list_t *list, Model *model)
{
    for (int i = 0; i < list->entities_count; i++)
    {
        if (!list->entities[i].active) continue;
        entity_t *e = &list->entities[i];
        if (!b3Body_IsValid(e->body))
        {
            e->active = 0;
            continue;
        }
        b3BodyId bodyId = e->body;
        b3Vec3 position = b3Body_GetPosition(bodyId);
        b3Quat rotation = b3Body_GetRotation(bodyId);
        float angle;
        b3Vec3 axis = b3GetAxisAngle(&angle, rotation);
        rlPushMatrix();
        rlTranslatef(position.x, position.y, position.z);
        rlRotatef(angle * RAD2DEG, axis.x, axis.y, axis.z);
        DrawModelEx(*model, (Vector3){0, 0, 0}, (Vector3){axis.x, axis.y, axis.z}, 0.0f * RAD2DEG, e->scale, WHITE);
        rlPopMatrix();
        
        rlPushMatrix();
        rlTranslatef(position.x, 0.05f, position.z);
        rlScalef(e->scale.x, 0.0f, e->scale.z);
        rlRotatef(angle * RAD2DEG, axis.x, axis.y, axis.z);
        DrawModelEx(*model, (Vector3){0,0,0}, (Vector3){0.0f, axis.y, 0.0f}, 0.0f * RAD2DEG, 
            (Vector3){1.15f, 1.15f, 1.15f}, (Color){0, 0, 0, 40});
        rlPopMatrix();

    }
}

entity_t* entity_list_add_entity(entity_list_t *list, Vector3 position, Vector3 axis, float radians, Vector3 scale, userdata_info_t type, b3ShapeDef shape, b3HullData *hulldata)
{
    int index = -1;
    for (int i = 0; i < list->entities_count; i++)
    {
        if (!list->entities[i].active)
        {
            index = i;
            break;
        }
    }
    if (index < 0)
    {
        if (list->entities_count >= list->entities_capacity)
        {
            list->entities_capacity = list->entities_capacity + 64;
            list->entities = MemRealloc(list->entities, sizeof(entity_t) * list->entities_capacity);
        }
        index = list->entities_count++;
    }
    body_userdata_t ud = {
        .type = UD_LOG,
        .id = index
    };

    b3BodyDef bodyDef = b3DefaultBodyDef();
    bodyDef.type = b3_dynamicBody;
    bodyDef.position = (b3Vec3){ 
        position.x, 
        position.y,
        position.z
    };
    bodyDef.rotation = b3MakeQuatFromAxisAngle((b3Vec3){axis.x, axis.y, axis.z}, radians);
    bodyDef.userData = (void*)ud.raw;
    entity_t *entity = &list->entities[index];
    entity->active = 1;
    entity->body = b3CreateBody(g_game.world_id, &bodyDef);
    b3ShapeDef shapeDef = b3DefaultShapeDef();
    shapeDef.density = 1.0f;
    shapeDef.baseMaterial.restitution = 0.1f;
    shapeDef.baseMaterial.friction = 0.8f;
    shapeDef.userData = (void*)ud.raw;
    entity->scale = scale;
    entity->shape = b3CreateTransformedHullShape(entity->body, &shapeDef, hulldata, b3Transform_identity, 
        (b3Vec3){scale.x, scale.y, scale.z});
}

float randf(float min, float max)
{
    return GetRandomValue(min * 1000, max * 1000) / 1000.0f; 
}

void world_spawn_entities(Vector3 world_pos, int count, entity_list_t *list, model_data_t *model, userdata_info_t ud)
{
    for (int i = 0; i < count; i++)
    {
        b3ShapeDef shapedef = b3DefaultShapeDef();
        shapedef.enableSensorEvents = 1;
        Vector3 pos = {
            world_pos.x + randf(-0.25f, 0.25f),
            world_pos.y + randf(-0.25f, 0.25f),
            world_pos.z + randf(-0.25f, 0.25f),
        };
        Vector3 axis = Vector3Normalize((Vector3){
            randf(-1.0f, 1.0f),
            randf(-1.0f, 1.0f),
            randf(-1.0f, 1.0f),
        });

        float radians = randf(-PI, PI);
        Vector3 scale = {randf(0.4f, 0.6f), randf(0.4f, 0.6f), randf(0.4f, 0.6f)};
        entity_list_add_entity(list, pos, axis, radians, scale, ud, shapedef, model->hulldata);
    }
}

void world_spawn_logs(Vector3 world_pos, int count)
{
    world_spawn_entities(world_pos, count, &g_game.logs, &g_assets.log, UD_LOG);
}

void world_spawn_rock_items(Vector3 world_pos, int count)
{
    world_spawn_entities(world_pos, count, &g_game.rock_items, &g_assets.rock_item, UD_ROCKITEM);
}

static hex_cell_t *g_map = 0;

float ease_in_out_sine(float t)
{
    return -(cosf(PI * t) - 1.0f) * 0.5f;
}

static void pawn_update(float dt, pawn_instance_t *pawn)
{
    float size = pawn->size;

    if (pawn->transition < 1.0f)
    {
        pawn->transition += dt / PAWN_TRANSITION_T / size;
        if (pawn->transition > 1.0f) pawn->transition = 1.0f;    
    }
    else
    {
        b3Capsule mover;
        mover.center1 = (b3Vec3){ 0.0f, 0.35f, 0.0f };  // bottom sphere center
        mover.center2 = (b3Vec3){ 0.0f, 1.45f, 0.0f };  // top sphere center
        mover.radius  = 0.2f * size;
        Vector3 next_pos = Vector3MoveTowards(pawn->next_pos, pawn->target_pos, PAWN_JUMP_DIST * size);
        Vector3 delta = Vector3Subtract(next_pos, pawn->next_pos);

        float frac = b3World_CastMover(g_game.world_id, (b3Vec3){pawn->next_pos.x, pawn->next_pos.y + 0.5f, pawn->next_pos.z},
            &mover, (b3Vec3){delta.x, delta.y, delta.z}, (b3QueryFilter){.categoryBits = -1, .maskBits = -1}, NULL, NULL);
        frac += 0.2f;
        if (frac < 1.0f)
        {
            if (frac < 0.3f)
            {
                next_pos = pawn->next_pos;
            }
            else next_pos = Vector3Add(pawn->next_pos, Vector3Scale(delta, frac));
        }

        pawn->current_pos = pawn->next_pos;
        pawn->next_pos = next_pos;
        pawn->transition = 0.0f;
        pawn->step ++;
    }


    float dist = Vector3Distance(pawn->current_pos, pawn->next_pos);
    Vector3 dir = Vector3Normalize(Vector3Subtract(pawn->next_pos, pawn->current_pos));
    pawn->direction = Vector3Lerp(pawn->direction, dir, dt * 8.0f);
    Vector3 pos = Vector3Lerp(pawn->current_pos, pawn->next_pos, ease_in_out_sine(pawn->transition));

    float jmp = sin(pawn->transition * PI) * dist;
    pos.y += jmp;

    float odd = (pawn->step & 1) * 2.0f - 1.0f;
    float ang = atan2f(pawn->direction.x, pawn->direction.z) - odd * 0.05f * dist;
    DrawModelEx(g_assets.pawns[pawn->model].body, pos, (Vector3){0.0f,1.0f,0.0f}, ang * RAD2DEG, (Vector3){size, size, size}, WHITE);
    if (pawn->has_backpack)
    {
        DrawModelEx(g_assets.backpack, pos, (Vector3){0.0f,1.0f,0.0f}, ang * RAD2DEG, (Vector3){size, size, size}, WHITE);
    }


    Vector3 right = {-cos(ang), 0.0f, sin(ang)};
    Vector3 forward = {sin(ang), 0.0f, cos(ang)};
    float rv = size * .5f;
    float frv = size * (.4f + odd * .2f * jmp);
    float flv = size * (.4f - odd * .2f * jmp);
    float hry = -jmp * .1f * odd;
    float hly = jmp * .1f * odd;
    Vector3 right_hand_pos = {right.x * rv + forward.x * frv + pos.x, hry + size * .05f, right.z * rv + pos.z + forward.z * frv};
    Vector3 left_hand_pos = {-right.x * rv + forward.x * flv + pos.x, hly + size * .05f, -right.z * rv + pos.z + forward.z * flv};
    DrawModelEx(g_assets.pawns[pawn->model].hand, right_hand_pos, (Vector3){0.0f,1.0f,0.0f}, ang * RAD2DEG, (Vector3){size, size, size}, WHITE);
    DrawModelEx(g_assets.pawns[pawn->model].hand, left_hand_pos, (Vector3){0.0f,1.0f,0.0f}, ang * RAD2DEG, (Vector3){size, size, size}, WHITE);

    pos.y = 0.1f;
    DrawModelEx(g_assets.pawn_shadow, pos, (Vector3){0.0f,1.0f,0.0f}, ang * RAD2DEG, (Vector3){size, size, size}, WHITE);

	if ( pawn->kinematic.index1 )
    {

        b3Body_SetTargetTransform( pawn->kinematic, (b3WorldTransform) { (b3Vec3){pos.x, pos.y, pos.z}, b3Quat_identity }, 1.0f/60.0f, true );

        int capacity = b3Shape_GetSensorCapacity(pawn->forward_sensor);
        b3ShapeId overlaps[64] = {0};
        int count = b3Shape_GetSensorData(pawn->forward_sensor, overlaps, capacity);
        for (int i = 0; i < count; i++)
        {
            b3ShapeId visitorId = overlaps[i];
            // Ensure the visitorId is valid
            if (b3Shape_IsValid(visitorId) == false)
            {
                continue;
            }
            body_userdata_t userdata = {.raw =(uint32_t) b3Shape_GetUserData(visitorId)};
            if(userdata.type == UD_TREE)
            {
                b3Vec3 pos = b3Body_GetPosition(b3Shape_GetBody(visitorId));
                world_spawn_logs((Vector3){pos.x, pos.y, pos.z}, 3);
                b3DestroyShape(visitorId, 0);
                g_map[userdata.id].tree_bits &= ~(1<<userdata.subid);
            }
            else 
            if(userdata.type == UD_CONIFER)
            {
                b3Vec3 pos = b3Body_GetPosition(b3Shape_GetBody(visitorId));
                world_spawn_logs((Vector3){pos.x, pos.y, pos.z}, 3);
                b3DestroyShape(visitorId, 0);
                g_map[userdata.id].conifer_bits &= ~(1<<userdata.subid);

                // printf("unkonwn %d %d %d\n",userdata.type, userdata.id, userdata.subid);
            }
            else if (userdata.type == UD_ROCK)
            {
                b3Vec3 pos = b3Body_GetPosition(b3Shape_GetBody(visitorId));
                b3DestroyShape(visitorId, 0);

                world_spawn_rock_items((Vector3){pos.x, pos.y, pos.z}, 3);
                g_map[userdata.id].rock_bits &= ~(1<<userdata.subid);

            }
        }
    }

}

void draw_hex_outline(Vector3 center, float radius, Color color)
{
    for (float f = 0.0f; f <= 6.0f; f++)
    {
        float a1 = f / 3.0f * PI;
        float a2 = (f+1.0f) / 3.0f * PI;
        float dx1 = sinf(a1) * radius;
        float dy1 = cosf(a1) * radius;
        float dx2 = sinf(a2) * radius;
        float dy2 = cosf(a2) * radius;
        DrawLine3D(
            (Vector3){center.x + dx1, center.y, center.z + dy1},
            (Vector3){center.x + dx2, center.y, center.z + dy2}, color
        );
    }
}

float stb_perlin_fbm_noise3(float x, float y, float z, float lacunarity, float gain, int octaves);


// Source - https://stackoverflow.com/a/20117209
// Posted by Roman Reiner, modified by community. See post 'Timeline' for change history
// Retrieved 2026-07-08, License - CC BY-SA 3.0

int is_inside_hexagon(float x, float y)
{
    // Check length (squared) against inner and outer radius
    float l2 = x * x + y * y;
    if (l2 > 1.0f) return false;
    if (l2 < 0.75f) return true; // (sqrt(3)/2)^2 = 3/4

    // Check against borders
    float px = x * 1.15470053838f; // 2/sqrt(3)
    if (px > 1.0f || px < -1.0f) return false;

    float py = 0.5f * px + y;
    if (py > 1.0f || py < -1.0f) return false;

    if (px - py > 1.0f || px - py < -1.0f) return false;

    return true;
}

#define HEX_POINT_COUNT 12

uint32_t get_random_bits(int count, int max_count)
{
    if (count > max_count) count = max_count;
    count = GetRandomValue(count / 2, count);
    if (count >= HEX_POINT_COUNT) return 0xffffffff;
    uint32_t bits = 0;
    while (count > 0)
    {
        int select = GetRandomValue(0, HEX_POINT_COUNT - 1);
        if ((bits >> select) & 1) continue;
        bits |= 1 << select;
        count--;
    }
    return bits;
}


// Update and draw game frame
static void UpdateDrawFrame(void)
{
    if (!g_game.is_initialized)
    {
        g_game.is_initialized = 1;
        for (int i = 0; i < NUM_NPCS; i++)
        {
            Vector3 pos = {GetRandomValue(-100, 100) * 0.1f, 0, GetRandomValue(-100, 100) * 0.1f};
            g_game.npcs[i] = 
            (pawn_instance_t) {
                .current_pos = pos,
                .next_pos = pos,
                .target_pos = pos,
                .size = 0.6f,
                .model = 1,
            };
        }
        g_game.player_pawn.size = 1.0f;
    }

    float dt = GetFrameTime();

    BeginDrawing();

        ClearBackground(BLACK);

        static Camera3D camera = {
            .fovy = 25.0f,
            .position = {0.0f, 50.0f, -30.0f},
            .target = {0.0f, 0.0f, 0.0f},
            .projection = CAMERA_PERSPECTIVE,
            .up = {0.0f, 1.0f, 0.0f}
        };
        static float cam_dist = 100.0f;
        float cam_zoom_speed = 20.0f;
        if (IsKeyDown(KEY_LEFT_SHIFT)) cam_zoom_speed = 60.0f;
        if (IsKeyDown(KEY_O)) cam_dist = cam_dist + dt * cam_zoom_speed;
        if (IsKeyDown(KEY_I)) cam_dist = cam_dist - dt * cam_zoom_speed;
        cam_dist = Clamp(cam_dist, 50.0f, 400.0f);
        Vector3 cam_dir = Vector3Normalize((Vector3){0.0f, 50.0f,-30.0f});
        camera.target = Vector3Lerp(camera.target, g_game.player_pawn.current_pos, dt);
        camera.position = Vector3Add(camera.target, Vector3Scale(cam_dir, cam_dist));

        
        BeginMode3D(camera);
        // DrawModel(g_assets.conifirs[0], (Vector3){-3.0f, 0.0f, -0.5f}, 1.0f, WHITE);
        SetRandomSeed(312);
        // for (int i = 0; i < 30; i++)
        // {
        //     Vector3 pos = {GetRandomValue(-100, 100) * 0.1f + 7.0f, 0, GetRandomValue(-100, 100) * 0.1f + 3.0f};
        //     float height = GetRandomValue(7,12) * 0.1f;
        //     float width = (GetRandomValue(-2,2) * 0.1f) + height;
        //     DrawModelEx(g_assets.conifirs[GetRandomValue(0, 2)], pos, (Vector3){0, 1.0f, 0}, GetRandomValue(0, 360),
        //         (Vector3){width, height, width}, WHITE);
        // }
        #define G MAP_TYPE_GRASS
        #define M MAP_TYPE_MUD
        #define W MAP_TYPE_WATER
        const int mapW = 64, mapH = 64;
        static b3WorldDef worldDef;
        static b3WorldId worldId;
        
        static b3BodyId bodyIds[120];
        static int bodyIdCount = 0;
        // random points inside a hex
        static Vector2 hex_points[HEX_POINT_COUNT];
        const float hex_point_min_dist_sq = 0.4f * 0.4f;
        static b3BodyId groundId;

        if (g_map == 0)
        {
            SetRandomSeed(123);
            for (int i = 0; i < HEX_POINT_COUNT; i++)
            {
                retry:
                float dx = (float) GetRandomValue(-1000, 1000) / 1000.0f;
                float dy = (float) GetRandomValue(-1000, 1000) / 1000.0f;
                if (!is_inside_hexagon(dx, dy)) goto retry;
                hex_points[i] = (Vector2){dx, dy};
                for (int j = 0; j < i; j++)
                {
                    if (Vector2DistanceSqr(hex_points[i], hex_points[j]) < hex_point_min_dist_sq)
                    {
                        goto retry;
                    }
                }
            }

            worldDef = b3DefaultWorldDef();
            worldDef.gravity = (b3Vec3){ 0.0f, -8.0f, 0.0f };
            worldId = b3CreateWorld(&worldDef);
            g_game.world_id = worldId;
            b3BodyDef groundBodyDef = b3DefaultBodyDef();
            groundBodyDef.position = (b3Vec3){ 0.0f, 0.0f, 0.0f };
            groundId = b3CreateBody(worldId, &groundBodyDef);
            b3BoxHull groundBox = b3MakeOffsetBoxHull(5000.0f, 10.0f, 5000.0f, (b3Vec3){0.0f, -10.0f, 0.0f});
            b3ShapeDef groundShapeDef = b3DefaultShapeDef();
            groundShapeDef.baseMaterial.restitution = 0.1f;
            groundShapeDef.baseMaterial.friction = 0.8f;
            b3CreateHullShape(groundId, &groundShapeDef, &groundBox.base);

            for (int i = 0; i < 20; i++)
            {
                b3BodyDef bodyDef = b3DefaultBodyDef();
                bodyDef.type = b3_dynamicBody;
                bodyDef.position = (b3Vec3){ 0.0f, 14.0f + i * 1.5f, 0.0f };
                bodyIds[i] = b3CreateBody(worldId, &bodyDef);
                b3BoxHull dynamicBox = b3MakeCubeHull(0.5f);
    
                b3ShapeDef shapeDef = b3DefaultShapeDef();
                shapeDef.density = 1.0f;
                shapeDef.baseMaterial.restitution = 0.1f;
                shapeDef.baseMaterial.friction = 0.8f;
    
                b3CreateHullShape(bodyIds[i], &shapeDef, &dynamicBox.base);
                bodyIdCount++;
            }

            g_map = MemAlloc(sizeof(hex_cell_t) * mapW * mapH);
            // g_map[32 + mapW * 32].conifer_count = 4;
            for (int x = 0; x < mapW; x++)
            {
                for (int y = 0; y < mapH; y++)
                {
                    Vector3 hex_pos ={
                        (x + 0.5f * (y & 1) - 32) * HEX_X,
                        0.0f,
                        (y - 32) * HEX_Y,
                    };
                    float dist = Vector3Length(hex_pos);
                    float freq = 0.012f;
                    float f = stb_perlin_fbm_noise3(hex_pos.x * freq,hex_pos.z * freq,1.0f,2.0f,0.5f,5) - 0.15f - dist * 0.003f;
                    int idx = x + mapW * y;
                    if (f < -0.3f)
                    {
                        g_map[idx].center = MAP_TYPE_WATER;
                        for (int i=0; i < 6; i+=1) g_map[idx].corners[i] = MAP_TYPE_WATER;
                    }
                    else
                    {
                        g_map[idx].center = MAP_TYPE_GRASS;
                        for (int i=0; i < 6; i+=1) g_map[idx].corners[i] = MAP_TYPE_GRASS;
                    }
                    if (f > -0.32f)
                    {
                        float tfreq = freq * 6.0f;
                        float tf = stb_perlin_fbm_noise3(hex_pos.x * tfreq,hex_pos.z * tfreq,2.0f,2.0f,0.5f,3);
                        if (tf > -0.3f)
                        {
                            if (tf > -0.15f)
                            {
                                g_map[idx].conifer_bits = get_random_bits((f + 0.3f) * 80, 7);
                            }
                            else
                            {
                                g_map[idx].tree_bits = get_random_bits((f + 0.3f) * 80, 7);
                                
                            }
                            g_map[idx].high_grass_bits = get_random_bits(GetRandomValue(0,3), 3);
                        }
                        else if (tf < -0.5f)
                        {
                            g_map[idx].rock_bits = get_random_bits((f + 0.3f) * 50, 7);
                            g_map[idx].high_grass_bits = get_random_bits(GetRandomValue(0,8), 8);
                        }
                        else if (g_map[idx].center == MAP_TYPE_GRASS)
                        {
                            g_map[idx].high_grass_bits = get_random_bits(GetRandomValue(2,12), 8);
                        }
                    }
                    else if (g_map[idx].center == MAP_TYPE_GRASS)
                    {
                        g_map[idx].high_grass_bits = get_random_bits(GetRandomValue(2,12), 8);
                    }

                }
            }
            for (int x = 0; x < mapW; x++)
            {
                for (int y = 0; y < mapH; y++)
                {
                    int idx = x + mapW * y;
                    if (x < mapW - 1)
                    {
                        int right = (x + 1) + mapW * y;
                        if ((g_map[idx].center == MAP_TYPE_GRASS && g_map[right].center == MAP_TYPE_WATER) ||
                            (g_map[idx].center == MAP_TYPE_WATER && g_map[right].center == MAP_TYPE_GRASS))
                        {
                            g_map[idx].corners[4] = MAP_TYPE_MUD;
                            g_map[idx].corners[3] = MAP_TYPE_MUD;
                            g_map[right].corners[0] = MAP_TYPE_MUD;
                            g_map[right].corners[1] = MAP_TYPE_MUD;
                        }
                    }
                    if (y > 0)
                    {
                        int top_left_x = (x + ((y + 1) & 1) - 1);
                        if (top_left_x > 0 && top_left_x < mapW)
                        {
                            int top_left = top_left_x + mapW * (y - 1);
                            if ((g_map[idx].center == MAP_TYPE_GRASS && g_map[top_left].center == MAP_TYPE_WATER) ||
                                (g_map[idx].center == MAP_TYPE_WATER && g_map[top_left].center == MAP_TYPE_GRASS))
                            {
                                g_map[idx].corners[0] = MAP_TYPE_MUD;
                                g_map[idx].corners[5] = MAP_TYPE_MUD;
                                g_map[top_left].corners[2] = MAP_TYPE_MUD;
                                g_map[top_left].corners[3] = MAP_TYPE_MUD;
                            }
                        }
                        int top_right_x = top_left_x + 1;
                        if (top_right_x < mapW)
                        {
                            int top_right = top_right_x + mapW * (y - 1);
                            if ((g_map[idx].center == MAP_TYPE_GRASS && g_map[top_right].center == MAP_TYPE_WATER) ||
                                (g_map[idx].center == MAP_TYPE_WATER && g_map[top_right].center == MAP_TYPE_GRASS))
                            {
                                g_map[idx].corners[4] = MAP_TYPE_MUD;
                                g_map[idx].corners[5] = MAP_TYPE_MUD;
                                g_map[top_right].corners[2] = MAP_TYPE_MUD;
                                g_map[top_right].corners[1] = MAP_TYPE_MUD;
                            }
                        }
                    }
                }
            }
            

            // find a pawn spawn pos: some beach close to the g_map center

            int closest_dist = 0xffffff;
            int closest_x = 32, closest_y = 32;
            for (int x = 0; x < mapW; x++)
            {
                for (int y = 0; y < mapH; y++)
                {
                    int dx = x - mapW / 2;
                    int dy = y - mapH / 2;
                    int dist = dx * dx + dy * dy;
                    if (dist > closest_dist) continue;

                    int idx = x + mapW * y;
                    
                    if (g_map[idx].center != MAP_TYPE_GRASS) continue;
                    int beach_count = 0;
                    for (int c = 0; c < 6; c++) beach_count += g_map[idx].corners[c] == MAP_TYPE_MUD;
                    if (beach_count >0 && beach_count < 3) {
                        closest_x = x;
                        closest_y = y;
                        closest_dist = dist;
                    }
                }
            }

            Vector3 hex_pos ={
                ((closest_x - 32.5f) + 0.5f * (closest_y & 1) ) * HEX_X,
                0.0f,
                (closest_y - 32.5f) * HEX_Y,
            };
            printf(">> %f %f : %d %d\n", hex_pos.x, hex_pos.z, closest_x, closest_y);
            g_game.player_pawn.current_pos = g_game.player_pawn.next_pos = g_game.player_pawn.target_pos = hex_pos;
            
            g_game.player_pawn.has_backpack = 1;


            b3BodyDef bodyDef = b3DefaultBodyDef();
            bodyDef.type = b3_kinematicBody;
            bodyDef.position = (b3Vec3){ hex_pos.x, hex_pos.y, hex_pos.z };
            
            g_game.player_pawn.kinematic = b3CreateBody(worldId, &bodyDef);
            float linkRadius = 0.25f;
            float linkExtent = 0.85f;
            b3Capsule capsule = { { 0.0f, linkRadius, 0.0f }, { 0.0f, linkExtent, 0.0f }, linkRadius };

            b3ShapeDef shapeDef = b3DefaultShapeDef();
            shapeDef.density = 1.0f;
            shapeDef.baseMaterial.restitution = 0.1f;
            shapeDef.baseMaterial.friction = 0.8f;
            b3CreateCapsuleShape( g_game.player_pawn.kinematic, &shapeDef, &capsule );    

            b3ShapeDef sensor = b3DefaultShapeDef();
            sensor.density = 0.0f;
            sensor.isSensor = 1;
            sensor.enableSensorEvents = 1;
            g_game.player_pawn.forward_sensor = b3CreateSphereShape(g_game.player_pawn.kinematic, &sensor, &(b3Sphere){.radius = 1.0f});
        }
        // {
        //     {G, {G, G, M, M, M, G}, 3}, {G, {M, M, G, G, G, M}}, {G, {G, G, G, G, G, G}},
        //     {G, {G, M, M, M, M, G}, 15}, {G, {M, M, M, M, G, M}}, {G, {G, M, M, M, G, G}},
        //     {W, {M, W, W, W, M, M}}, {W, {M, W, W, W, M, M}}, {W, {M, W, W, W, W, M}},

        // } 
        Ray center = GetScreenToWorldRay((Vector2){GetScreenWidth()/2, GetScreenHeight()/2}, camera);
        float cdist = center.position.y / -center.direction.y;
        Vector3 camcen = {
            center.position.x + center.direction.x * cdist,
            center.position.y + center.direction.y * cdist,
            center.position.z + center.direction.z * cdist,
        };
        #define MAX_SHADOW_COUNT 1024
        static Vector4 shadows[MAX_SHADOW_COUNT];
        int shadow_count = 0;
        for (int i = 0; i < mapW * mapH; i++)
        {
            hex_cell_t cell = g_map[i];
            int x = -i % mapW;
            int y = -i / mapW;
            Vector3 hex_pos ={
                (x + 0.5f * (y & 1) + 32) * HEX_X,
                0.0f,
                (y + 32) * HEX_Y,
            };
            float cdx = (hex_pos.x - camcen.x) / HEX_X;
            float cdy = (hex_pos.z - camcen.z) / HEX_Y;
            if (cdx > 10 || cdx < -10 || cdy < -10 || cdy > 10)
            {
                continue;
            }
            for (int c = 0; c < 6; c++)
            {
                int ca = cell.corners[c];
                int cb = cell.corners[(c + 1) % 6];
                tri_hex_t *match = 0;
                for (int t = 0; t < g_assets.tri_hex_count; t++)
                {
                    tri_hex_t *th = &g_assets.tri_hexes[t];
                    if (th->center == cell.center && th->corner_a == ca && th->corner_b == cb)
                    {
                        match = th;
                        break;
                    }
                }
                if (match)
                {
                    DrawModelEx(match->model,hex_pos, (Vector3){0.0f, 1.0f, 0.0f}, c * 60, (Vector3){1.0f, 1.0f, 1.0f}, WHITE);
                }
                else
                {
                    DrawCube(hex_pos, 1,1,1, RED);
                    printf("%d\n",g_assets.tri_hex_count);
                    // DrawModelEx(g_assets.tri_hexes[6].model,hex_pos, (Vector3){0.0f, 1.0f, 0.0f}, c * 60, (Vector3){1.0f, 1.0f, 1.0f}, WHITE);
                    
                }
            }

            for (int i = 0; i < 32; i++)
            {
                DrawCube((Vector3){hex_points[i].x * HEX_X * .5f - HEX_X * 2, 0, hex_points[i].y * HEX_Y * .5f + HEX_Y * 2}, 0.1f, 0.2f, 0.1f, GREEN);
            }

            SetRandomSeed(x + y * 1283);
            for (int c = 0; c < HEX_POINT_COUNT; c++)
            {
                if ((cell.tree_bits >> c & 1) == 0) continue;
                SetRandomSeed(x + y * 1283 + c);

                Vector3 pos = hex_pos;
                pos.x += hex_points[c].x * HEX_X * 0.5f;
                pos.z += hex_points[c].y * HEX_Y * 0.5f;
                float height = GetRandomValue(7,12) * 0.07f;
                float width = (GetRandomValue(-2,2) * 0.1f) + height;

                model_data_t *m = &g_assets.trees[GetRandomValue(0, g_assets.tree_count - 1)];
                if (!cell.physics_initialized && m->hulldata)
                {
                    b3ShapeDef shapeDef = b3DefaultShapeDef();
                    shapeDef.enableSensorEvents = 1;
                    shapeDef.filter.groupIndex = GROUP_INTERACTIBLE;
                    b3BodyDef bdef = b3DefaultBodyDef();
                    bdef.position.x = pos.x;
                    bdef.position.y = pos.y;
                    bdef.position.z = pos.z;
                    body_userdata_t ud = {.type = UD_TREE, .id = i, .subid = c};
                    bdef.userData = (void*)ud.raw;
                    shapeDef.userData = (void*)ud.raw;
                    b3BodyId body = b3CreateBody(worldId, &bdef);
                    b3Transform t = b3Transform_identity;
                    
                    b3CreateTransformedHullShape(body, &shapeDef, m->hulldata, t, (b3Vec3){width * .7f, height, width * .7f});
                }
                
                DrawModelEx(m->model, pos, (Vector3){0, 1.0f, 0}, GetRandomValue(0, 360),
                    (Vector3){width, height, width}, WHITE);
                
                
                
                if (shadow_count < MAX_SHADOW_COUNT)
                {
                    shadows[shadow_count++] = (Vector4){pos.x, pos.y, pos.z, width};
                }
            }
            for (int c = 0; c < HEX_POINT_COUNT; c++)
            {
                if ((cell.conifer_bits >> c & 1) == 0) continue;

                SetRandomSeed(x + y * 1285 + c);

                Vector3 pos = hex_pos;
                pos.x += hex_points[c].x * HEX_X * 0.5f;
                pos.z += hex_points[c].y * HEX_Y * 0.5f;
                float height = GetRandomValue(7,12) * 0.07f;
                float width = (GetRandomValue(-2,2) * 0.1f) + height;
            

                model_data_t *m = &g_assets.conifirs[GetRandomValue(0, g_assets.conifir_count - 1)];
                if (!cell.physics_initialized && m->hulldata)
                {
                    b3ShapeDef shapeDef = b3DefaultShapeDef();
                    b3BodyDef bdef = b3DefaultBodyDef();
                    bdef.position.x = pos.x;
                    bdef.position.y = pos.y;
                    bdef.position.z = pos.z;
                    
                    body_userdata_t ud = {.type = UD_CONIFER, .id = i, .subid = c};
                    bdef.userData = (void*)ud.raw;
                    shapeDef.userData = (void*)ud.raw;
                    shapeDef.enableSensorEvents = 1;
                    
                    b3BodyId body = b3CreateBody(worldId, &bdef);
                    b3CreateTransformedHullShape(body, &shapeDef, m->hulldata, b3Transform_identity, (b3Vec3){width*.7f, height, width*.7f});
                    
                }

                DrawModelEx(m->model, pos, (Vector3){0, 1.0f, 0}, GetRandomValue(0, 360),
                    (Vector3){width, height, width}, WHITE);
                
                if (shadow_count < MAX_SHADOW_COUNT)
                {
                    shadows[shadow_count++] = (Vector4){pos.x, pos.y, pos.z, width};
                }
            }
            for (int c = 0; c < HEX_POINT_COUNT; c++)
            {
                if ((cell.rock_bits >> c & 1) == 0) continue;
                SetRandomSeed(x + y * 1288 + c);

                Vector3 pos = hex_pos;
                pos.x += hex_points[c].x * HEX_X * 0.5f;
                pos.z += hex_points[c].y * HEX_Y * 0.5f;

                float height = GetRandomValue(7,12) * 0.07f;
                float width = (GetRandomValue(-2,2) * 0.1f) + height;

                
                model_data_t *m = &g_assets.rocks[GetRandomValue(0, g_assets.rocks_count - 1)];
                if (!cell.physics_initialized && m->hulldata)
                {
                    b3ShapeDef shapeDef = b3DefaultShapeDef();
                    b3BodyDef bdef = b3DefaultBodyDef();
                    bdef.position.x = pos.x;
                    bdef.position.y = pos.y;
                    bdef.position.z = pos.z;

                    body_userdata_t ud = {.type = UD_ROCK, .id = i, .subid = c};
                    bdef.userData = (void*)ud.raw;
                    shapeDef.userData = (void*)ud.raw;
                    shapeDef.enableSensorEvents = 1;
                    
                    b3BodyId body = b3CreateBody(worldId, &bdef);
                    b3CreateTransformedHullShape(body, &shapeDef, m->hulldata, b3Transform_identity, (b3Vec3){width, height, width});
                }
            
                DrawModelEx(m->model, pos, (Vector3){0, 1.0f, 0}, GetRandomValue(0, 360),
                    (Vector3){width, height, width}, WHITE);
                if (shadow_count < MAX_SHADOW_COUNT)
                {
                    shadows[shadow_count++] = (Vector4){pos.x, pos.y, pos.z, width};
                }
            }
            for (int g = 0; g < HEX_POINT_COUNT;g++)
            {
                if ((cell.high_grass_bits >> g & 1) == 0) continue;
                SetRandomSeed(x + y * 128 + g);

                Vector3 pos = hex_pos;
                pos.x += hex_points[g].x * HEX_X * 0.5f;
                pos.z += hex_points[g].y * HEX_Y * 0.5f;

                float height = GetRandomValue(7,12) * 0.07f;
                float width = (GetRandomValue(-2,2) * 0.1f) + height;
            
                DrawModelEx(g_assets.high_grass[GetRandomValue(0, g_assets.high_grass_count - 1)], pos, (Vector3){0, 1.0f, 0}, GetRandomValue(0, 360),
                    (Vector3){width, height, width}, WHITE);
            }

            g_map[i].physics_initialized = 1;
        }

        static float accumulator = 0;
        const float sim_hz = 1.0f/60.0f;
        accumulator += dt;
        while (accumulator > sim_hz)
        {
            accumulator -= sim_hz;

            b3World_Step(worldId, sim_hz, 4);

        }

        for (int i = 0; i < bodyIdCount; i++)
        {
            b3BodyId bodyId = bodyIds[i];
            b3Vec3 position = b3Body_GetPosition(bodyId);
            b3Quat rotation = b3Body_GetRotation(bodyId);
            float angle;
            b3Vec3 axis = b3GetAxisAngle(&angle, rotation);
            DrawModelEx(g_assets.crate, (Vector3){position.x, position.y, position.z}, (Vector3){axis.x, axis.y, axis.z}, angle * RAD2DEG, (Vector3){1.0f, 1.0f, 1.0f}, WHITE);
        }

        entity_list_draw_all(&g_game.logs, &g_assets.log.model);
        entity_list_draw_all(&g_game.rock_items, &g_assets.rock_item.model);

        rlDisableDepthMask();
        for (int i = 0; i < shadow_count; i++)
        {
            Vector4 v = shadows[i];
            DrawModel(g_assets.tree_shadow, (Vector3){v.x, v.y + 0.1f, v.z}, v.w * 1.2f, WHITE);
        }
        rlEnableDepthMask();

        float speed = 8.0f;
        int moving = 0;
        if (IsKeyDown(KEY_D))
        {
            moving = 1;
            g_game.player_pawn.target_pos.x -= dt * speed;
        }
        if (IsKeyDown(KEY_A))
        {
            moving = 1;
            g_game.player_pawn.target_pos.x += dt * speed;
        }
        if (IsKeyDown(KEY_W))
        {
            moving = 1;
            g_game.player_pawn.target_pos.z += dt * speed;
        }
        if (IsKeyDown(KEY_S))
        {
            moving = 1;
            g_game.player_pawn.target_pos.z -= dt * speed;
        }
        if (IsKeyPressed(KEY_E) && !moving)
        {
            g_game.player_pawn.action = 1;

            // box spawning, maybe useful for later
            // b3BodyDef bodyDef = b3DefaultBodyDef();
            // bodyDef.type = b3_dynamicBody;
            // bodyDef.position = (b3Vec3){ 
            //     g_game.player_pawn.current_pos.x, 
            //     g_game.player_pawn.current_pos.y + 5.0f,
            //     g_game.player_pawn.current_pos.z
            // };
            // bodyIds[bodyIdCount] = b3CreateBody(worldId, &bodyDef);
            // b3BoxHull dynamicBox = b3MakeCubeHull(0.5f);

            // b3ShapeDef shapeDef = b3DefaultShapeDef();
            // shapeDef.density = 1.0f;
            // shapeDef.baseMaterial.restitution = 0.1f;
            // shapeDef.baseMaterial.friction = 0.8f;

            // b3CreateHullShape(bodyIds[bodyIdCount++], &shapeDef, &dynamicBox.base);
        }
        else
        {
            g_game.player_pawn.action = 0;
        }
        g_game.player_pawn.target_pos = Vector3MoveTowards(g_game.player_pawn.next_pos, g_game.player_pawn.target_pos, PAWN_JUMP_DIST);
        pawn_update(dt, &g_game.player_pawn);
        // DrawModelEx(g_assets.pawn, g_game.player_pawn.current_pos, (Vector3){0.0f,1.0f,0.0f}, 133.0f, (Vector3){1.0f, 1.0f, 1.0f}, WHITE);

        float pdir = atan2f(g_game.player_pawn.direction.x, g_game.player_pawn.direction.y);
        for (int i = 0; i < NUM_NPCS; i++)
        {
            float ang = (float) i / (float) NUM_NPCS * PI * 2.0f * 0.3f + pdir;

            float dx = cosf(ang), dy = sinf(ang);
            Vector3 src = g_game.player_pawn.current_pos;
            src.x += dx * 3.0f;
            src.z += dy * 3.0f;
            float dist = Vector3Distance(src, g_game.npcs[i].current_pos);
            if (dist > 2.0f + i * .3f)
            {
                Vector3 target = Vector3MoveTowards(src, g_game.npcs[i].current_pos, 1.0f);
                g_game.npcs[i].target_pos = target;
            }
            pawn_update(dt, &g_game.npcs[i]);
        }

        Ray mouse_ray = GetScreenToWorldRay(GetMousePosition(), camera);
        float hdist = mouse_ray.position.y / -mouse_ray.direction.y;
        Vector3 ground_pos = {
            mouse_ray.position.x + mouse_ray.direction.x * hdist,
            mouse_ray.position.y + mouse_ray.direction.y * hdist,
            mouse_ray.position.z + mouse_ray.direction.z * hdist,
        };

        DrawSphere(ground_pos, 0.5f, RED);

        Vector3 hex_pos = ground_pos;
        int hy = (int)roundf(hex_pos.z / HEX_Y);
        float offset = (hy % 2 * 0.5f);
        int hx = (int)roundf(hex_pos.x / HEX_X - offset);
        hex_pos = (Vector3){HEX_X*(offset + hx), 0.0f, HEX_Y * hy};

        hex_pos.y +=0.05f;

        draw_hex_outline(hex_pos, HEX_X * 0.5f, WHITE);
        

        EndMode3D();

        DrawText(TextFormat("%.2f %.2f", g_game.player_pawn.target_pos.x, g_game.player_pawn.target_pos.z), 10, 10, 20, WHITE);

    EndDrawing();
    //----------------------------------------------------------------------------------
}
