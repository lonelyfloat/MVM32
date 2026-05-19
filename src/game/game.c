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
#include "child.h"
#include "utils.h"
#include "prefab.h"
#include <string.h>

ECS* ecs;

int screenWidth = 1280;
int screenHeight = 720;

Arena* arena;
bool paused = false;

ImGuiContext* ctx;

Entity inspect = NULL_ENTITY;
void Init(Arena* gameArena) {
    arena = gameArena;
    printf("Initializing game...\n");
    ecs = InitECS(arena, 100, COMPONENT_COUNT);
    RegisterComponents(ecs, arena);
    InitAssets(gameArena, 2);
    screenWidth = GetScreenWidth();
    screenHeight = GetScreenHeight();
    ctx = GetImGuiContext();
    ImGui_SetCurrentContext(ctx);
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
        FABRIKSystem(ecs, 50);
        CollisionSystem(ecs);
        PlayerSystem(ecs);
    }
    if(IsKeyDown(KEY_LEFT_CONTROL) && IsKeyPressed(KEY_L))
        Load("./assets/game_data");
    if(IsKeyDown(KEY_LEFT_CONTROL) && IsKeyPressed(KEY_S))
        Save("./assets/game_data"); 

    BeginUI();
    EntityPanel(arena,ecs, &inspect, IsKeyDown(KEY_LEFT_CONTROL) && IsKeyPressed(KEY_E));
    ImGui_Begin("tree test", NULL, ImGuiWindowFlags_None);
    if(inspect != NULL_ENTITY) {
        if(ImGui_Button("Tree print")) {
            SaveEntityTree(ecs, inspect, "./assets/prefabs/entity1");
        }
    }
    if(ImGui_Button("Tree load")) {
        LoadEntityTree(ecs, arena, "./assets/prefabs/entity1");
    }
    ImGui_End();
    EndUI();


    // ---------------------------------------------------------------------------------
    // Draw
    // ---------------------------------------------------------------------------------
    BeginDrawing();
        ClearBackground(RAYWHITE);
        for(int i = 0; i < ecs->blocks[DEBUG_SHAPE_COMPONENT].count; ++i) {
            Entity e = GetEntity(ecs, DEBUG_SHAPE_COMPONENT, i);
            if(!HasComponent(ecs, e, HITBOX_COMPONENT)) continue;
            Hitbox h = GetWorldHitbox(ecs, e);
            DebugShape d = IndexComponent(ecs, DebugShape,DEBUG_SHAPE_COMPONENT, i);
            DrawRectangleV(h.pos, h.scale, d.col);
        }
        DrawUI();
    EndDrawing();
}
