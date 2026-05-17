#include "program.h"
#include <raylib/raylib.h>
#include <raylib/raymath.h>
#include "assets.h"
#include "ecs.h"
#include "ui/ui.h"
#include "component_types.h"
#include <stdio.h>
#include "systems.h"
#include "player.h"
#include "utils.h"
#include <string.h>

ECS* ecs;

int screenWidth = 1280;
int screenHeight = 720;

Arena* arena;
bool paused = false;

// Test enemy
// Rectangle enemy = {400, 240, 50, 50};
// const float enemySpeed = 100;
// Vector2 enemyVelo = {enemySpeed,0};

// #define STATIC_RECS 3
// Rectangle staticRecs[STATIC_RECS] = {
//     {0, 600, 1280, 120},
//     {900, 436, 200, 40},
//     {400, 290, 200, 40}
// };
// ImGuiContext* ctx;

Entity inspect = NULL_ENTITY;
void Init(Arena* gameArena) {
    arena = gameArena;
    printf("Initializing game...\n");
    ecs = InitECS(arena, 100, COMPONENT_COUNT);
    RegisterComponents(ecs, arena);
    InitAssets(gameArena, 2);
    screenWidth = GetScreenWidth();
    screenHeight = GetScreenHeight();
    // ctx = GehealthtImGuiContext();
    // ImGui_SetCurrentContext(ctx);
}

void Save(char* file) {
    SaveEntitiesToFile(ecs, file);
}

void Load(char* file) {
    NukeECS(ecs);
    LoadEntitiesFromFile(ecs, arena, file);
}

void UpdateDrawFrame(void) {

    // Update
    //----------------------------------------------------------------------------------
    // Enemy code
    // if(enemy.x+enemy.width >= staticRecs[2].x+staticRecs[2].width) 
    //     enemyVelo.x *= -1;
    // if(enemy.x <= staticRecs[2].x) 
    //     enemyVelo.x *= -1;
    // enemy.x += enemyVelo.x * GetFrameTime();
    // enemy.y += enemyVelo.y * GetFrameTime();

    if(IsKeyDown(KEY_LEFT_CONTROL) && IsKeyPressed(KEY_P)) {
        paused = !paused;
    }
    if(!paused) {
        VelocitySystem(ecs);
        CollisionSystem(ecs);
        PlayerSystem(ecs);
    }
    if(IsKeyDown(KEY_LEFT_CONTROL) && IsKeyPressed(KEY_L))
        Load("./assets/game_data");
    if(IsKeyDown(KEY_LEFT_CONTROL) && IsKeyPressed(KEY_S))
        Save("./assets/game_data"); 

    BeginUI();
    EntityPanel(arena,ecs, &inspect, IsKeyDown(KEY_LEFT_CONTROL) && IsKeyPressed(KEY_E));
    EndUI();


    // ---------------------------------------------------------------------------------
    // Draw
    // ---------------------------------------------------------------------------------
    BeginDrawing();
        ClearBackground(RAYWHITE);
        for(int i = 0; i < ecs->blocks[DEBUG_SHAPE_COMPONENT].count; ++i) {
            Entity e = GetEntity(ecs, DEBUG_SHAPE_COMPONENT, i);
            if(!HasComponent(ecs, e, HITBOX_COMPONENT)) continue;
            Hitbox* h = GetComponent(ecs, e, HITBOX_COMPONENT);
            DebugShape d = IndexComponent(ecs, DebugShape,DEBUG_SHAPE_COMPONENT, i);
            DrawRectangleRec(*h, d.col);
        }
        // DrawRectangleRec(enemy, PINK);
        DrawUI();
    EndDrawing();
}
