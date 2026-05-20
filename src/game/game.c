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
#include "room.h"

ECS* ecs;

int screenWidth = 800;
int screenHeight = 600;

Arena* arena;
bool paused = false;

ImGuiContext* ctx;

Room* currentRoom;

Entity inspect = NULL_ENTITY;

// Editor stuff
const int gridSize = 50;
Vector2 editorCursor;

void Init(Arena* gameArena) {
    arena = gameArena;
    printf("Initializing game...\n");
    ecs = InitECS(arena, 100, COMPONENT_COUNT);
    RegisterComponents(ecs, arena);
    InitAssets(gameArena, 2);
    LoadAsset("./assets/temp_tileset.png", "TempTileset", ASSET_TYPE_TEXTURE);
    currentRoom = NewRoom(arena, screenWidth/gridSize + 2,screenHeight/gridSize + 2,50);
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

void UpdateEditor() {
    Vector2 mousePos = GetMousePosition();
    editorCursor.x = floor(mousePos.x/gridSize);
    editorCursor.y = floor(mousePos.y/gridSize);
    ImGuiIO* io = ImGui_GetIO();
    if(!io->WantCaptureMouse) {
        if(IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
            currentRoom->editorGrid[(int)editorCursor.x][(int)editorCursor.y] = 1;
            Autotile(currentRoom);
        }
    }
}

void DrawEditor() {
    for(int i = 1; i <= screenWidth/gridSize; ++i) {
        DrawLine(i*gridSize, 0, i*gridSize, screenHeight, GRAY);
    }
    for(int i = 1; i <= screenHeight/gridSize; ++i) {
        DrawLine(0, i*gridSize, screenWidth, i*gridSize, GRAY);
    }
    DrawRectangleLinesEx((Rectangle){editorCursor.x*gridSize, editorCursor.y*gridSize, gridSize,gridSize}, 2, RED);
    Texture2D* tileset = GetAsset("TempTileset");
    DrawRoomTiles(currentRoom, tileset);
}

void UpdateDrawFrame(void) {
    UpdateEditor();
    // Update
    //----------------------------------------------------------------------------------
    if(IsKeyDown(KEY_LEFT_CONTROL) && IsKeyPressed(KEY_P)) {
        paused = !paused;
    }
    if(!paused) {
        VelocitySystem(ecs);
        IKLegSystem(ecs,currentRoom);
        ResolveIK(ecs,100);
        CollisionSystem(ecs);
        ResolveRoomCollisions(ecs, currentRoom);
        PlayerSystem(ecs);
    }
    if(IsKeyDown(KEY_LEFT_CONTROL) && IsKeyPressed(KEY_L))
        currentRoom = LoadRoom(ecs, arena, "./assets/rooms/temp_room");
    if(IsKeyDown(KEY_LEFT_CONTROL) && IsKeyPressed(KEY_S))
        SaveRoom(ecs, currentRoom, "./assets/rooms/temp_room");

    BeginUI();
    EntityPanel(arena,ecs, &inspect, IsKeyDown(KEY_LEFT_CONTROL) && IsKeyPressed(KEY_E));
    ImGui_Begin("Room Tools", NULL, ImGuiWindowFlags_None);
    ImGui_End();
    EndUI();


    // ---------------------------------------------------------------------------------
    // Draw
    // ---------------------------------------------------------------------------------
    BeginDrawing();
        ClearBackground(RAYWHITE);
        DrawEditor();
        for(int i = 0; i < ecs->blocks[DEBUG_SHAPE_COMPONENT].count; ++i) {
            Entity e = GetEntity(ecs, DEBUG_SHAPE_COMPONENT, i);
            if(!HasComponent(ecs, e, HITBOX_COMPONENT)) continue;
            Hitbox* hb = GetComponent(ecs, e, HITBOX_COMPONENT);
            DebugShape d = IndexComponent(ecs, DebugShape,DEBUG_SHAPE_COMPONENT, i);
            DrawRectangleV(hb->pos, hb->scale, d.col);
        }
        for(int i = 0; i < ecs->blocks[IK_ROOT_COMPONENT].count; ++i) {
            IKRoot c = IndexComponent(ecs, IKRoot, IK_ROOT_COMPONENT, i);
            DrawCircleV(c.target,10, PINK);
        }
        DrawUI();
    EndDrawing();
}
