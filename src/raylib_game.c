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
#include <math.h>
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

//----------------------------------------------------------------------------------
// Global Variables Definition (local to this module)
//----------------------------------------------------------------------------------
static const int screenWidth = 800;
static const int screenHeight = 450;

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

typedef struct 
{
    Vector3 current_pos, next_pos;
    Vector3 target_pos;
    Vector3 direction;
    float transition;
    float size;
    int model;
} pawn_pos_t;
#define PAWN_TRANSITION_T 0.2f
#define PAWN_JUMP_DIST 0.75f

#define HEX_X 6.925f
#define HEX_Y 6.0f

#define NUM_NPCS 4
typedef struct 
{
    int is_initialized;
    pawn_pos_t player_pawn;
    pawn_pos_t npcs[NUM_NPCS];
} game_state_t;

float ease_in_out_sine(float t)
{
    return -(cosf(PI * t) - 1.0f) * 0.5f;
}

static void pawn_update(float dt, pawn_pos_t *pawn)
{
    float size = pawn->size;

    if (pawn->transition < 1.0f)
    {
        pawn->transition += dt / PAWN_TRANSITION_T / size;
        if (pawn->transition > 1.0f) pawn->transition = 1.0f;    
    }
    else
    {
        pawn->current_pos = pawn->next_pos;
        pawn->next_pos = Vector3MoveTowards(pawn->next_pos, pawn->target_pos, PAWN_JUMP_DIST * size);
        pawn->transition = 0.0f;
    }

    float dist = Vector3Distance(pawn->current_pos, pawn->next_pos);
    Vector3 dir = Vector3Normalize(Vector3Subtract(pawn->next_pos, pawn->current_pos));
    pawn->direction = Vector3Lerp(pawn->direction, dir, dt * 8.0f);
    Vector3 pos = Vector3Lerp(pawn->current_pos, pawn->next_pos, ease_in_out_sine(pawn->transition));

    float jmp = sin(pawn->transition * PI) * dist;
    pos.y += jmp;

    float ang = atan2f(pawn->direction.x, pawn->direction.z);
    DrawModelEx(g_assets.pawns[pawn->model], pos, (Vector3){0.0f,1.0f,0.0f}, ang * RAD2DEG, (Vector3){size, size, size}, WHITE);
    pos.y = 0.1f;
    DrawModelEx(g_assets.pawn_shadow, pos, (Vector3){0.0f,1.0f,0.0f}, ang * RAD2DEG, (Vector3){size, size, size}, WHITE);
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
typedef struct 
{
    uint8_t center, corners[6];
    int conifer_count;
} hex_cell_t;
game_state_t g_game;

float stb_perlin_fbm_noise3(float x, float y, float z, float lacunarity, float gain, int octaves);

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
            (pawn_pos_t) {
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

        camera.target = Vector3Lerp(camera.target, g_game.player_pawn.current_pos, dt);
        camera.position = Vector3Add(camera.target, (Vector3){0.0f, 50.0f, -30.0f});

        
        BeginMode3D(camera);
        // DrawModel(g_assets.confirs[0], (Vector3){-3.0f, 0.0f, -0.5f}, 1.0f, WHITE);
        SetRandomSeed(312);
        // for (int i = 0; i < 30; i++)
        // {
        //     Vector3 pos = {GetRandomValue(-100, 100) * 0.1f + 7.0f, 0, GetRandomValue(-100, 100) * 0.1f + 3.0f};
        //     float height = GetRandomValue(7,12) * 0.1f;
        //     float width = (GetRandomValue(-2,2) * 0.1f) + height;
        //     DrawModelEx(g_assets.confirs[GetRandomValue(0, 2)], pos, (Vector3){0, 1.0f, 0}, GetRandomValue(0, 360),
        //         (Vector3){width, height, width}, WHITE);
        // }
        #define G MAP_TYPE_GRASS
        #define M MAP_TYPE_MUD
        #define W MAP_TYPE_WATER
        static hex_cell_t *map = 0;
        const int mapW = 64, mapH = 64;

        if (map == 0)
        {
            map = MemAlloc(sizeof(hex_cell_t) * mapW * mapH);
            map[32 + mapW * 32].conifer_count = 4;
            for (int x = 0; x < mapW; x++)
            {
                for (int y = 0; y < mapH; y++)
                {
                    Vector3 hex_pos ={
                        (x + 0.5f * (y & 1) + 32) * HEX_X,
                        0.0f,
                        (y + 32) * HEX_Y,
                    };
                    float freq = 0.002f;
                    float f = stb_perlin_fbm_noise3(hex_pos.x * freq,hex_pos.z * freq,1.0f,2.0f,0.5f,5);
                    if (f < -0.3f)
                    {
                        int idx = x + mapW * y;
                        map[idx].center = MAP_TYPE_WATER;
                        for (int i=0; i < 6; i+=1) map[idx].corners[i] = MAP_TYPE_WATER;
                    }

                }
            }
            
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
        for (int i = 0; i < mapW * mapH; i++)
        {
            hex_cell_t cell = map[i];
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
            }

            for (int c = 0; c < cell.conifer_count; c++)
            {
                repeat:
                float dx = GetRandomValue(-1000,1000) * 0.001f;
                float dy = GetRandomValue(-1000,1000) * 0.001f;
                if (dx*dx+dy*dy > 1.0f) goto repeat;
                Vector3 pos = hex_pos;
                pos.x += dx * HEX_X * 0.4f;
                pos.z += dy * HEX_Y * 0.4f;
                float height = GetRandomValue(7,12) * 0.07f;
                float width = (GetRandomValue(-2,2) * 0.1f) + height;
            
                DrawModelEx(g_assets.confirs[GetRandomValue(0, 2)], pos, (Vector3){0, 1.0f, 0}, GetRandomValue(0, 360),
                    (Vector3){width, height, width}, WHITE);
            }
        }

        float speed = 8.0f;
        if (IsKeyDown(KEY_D))
        {
            g_game.player_pawn.target_pos.x -= dt * speed;
        }
        if (IsKeyDown(KEY_A))
        {
            g_game.player_pawn.target_pos.x += dt * speed;
        }
        if (IsKeyDown(KEY_W))
        {
            g_game.player_pawn.target_pos.z += dt * speed;
        }
        if (IsKeyDown(KEY_S))
        {
            g_game.player_pawn.target_pos.z -= dt * speed;
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
