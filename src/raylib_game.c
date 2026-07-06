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
}

game_state_t g_game;
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


    BeginDrawing();

        ClearBackground(BLACK);

        Camera3D camera = {
            .fovy = 25.0f,
            .position = {0.0f, 50.0f, -30.0f},
            .target = {0.0f, 0.0f, 0.0f},
            .projection = CAMERA_PERSPECTIVE,
            .up = {0.0f, 1.0f, 0.0f}
        };

        BeginMode3D(camera);
        DrawModel(g_assets.hex_grass, (Vector3){0}, 1.0f, WHITE);
        DrawModel(g_assets.hex_grass, (Vector3){6.925f, 0.0f, 0.0f}, 1.0f, WHITE);
        DrawModel(g_assets.hex_water, (Vector3){6.925f*0.5f, 0.0f, -6.0f}, 1.0f, WHITE);

        float dt = GetFrameTime();
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

        EndMode3D();

        DrawText(TextFormat("%.2f %.2f", g_game.player_pawn.target_pos.x, g_game.player_pawn.target_pos.z), 10, 10, 20, WHITE);

    EndDrawing();
    //----------------------------------------------------------------------------------
}
