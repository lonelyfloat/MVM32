#include "program.h"
#include <raylib/raylib.h>
#include <raylib/raymath.h>
#include "assets.h"
#include "ui/ui.h"
#include <stdio.h>
#include "player.h"
#include "world.h"
#include "editor.h"

int screenWidth = 800;
int screenHeight = 600;

Arena* arena;

World* world;
Player player;


ImGuiContext* ctx;

const int gridSize = 50;
const int mapWidth = 9*16;  // In tiles
const int mapHeight = 7*12; // In tiles

Camera2D worldCamera = (Camera2D) {.offset=(Vector2){-gridSize/2.0,-gridSize/2.0},.target=(Vector2){},.rotation=0.0,.zoom=1.0};

bool editorEnabled = false;

void Init(Arena* gameArena) {
    arena = gameArena;
    printf("Initializing game...\n");
    // Asset loading
    InitAssets(gameArena, 10);
    LoadAsset("./assets/lab_tileset.png", "LabTileset", ASSET_TYPE_TEXTURE);
    LoadAsset("./assets/player.png", "Player", ASSET_TYPE_TEXTURE);

    // ImGui loading
    ctx = GetImGuiContext();
    ImGui_SetCurrentContext(ctx);

    world = LoadWorld(arena, "./assets/world_data");
}

void Save(char* file) {
    SaveWorld(world, "./assets/world_data");
}

void Load(char* file) {
    world = LoadWorld(arena, "./assets/world_data");
}

void UpdateDrawFrame(void) {
    // Update
    //----------------------------------------------------------------------------------
    if(IsKeyPressed(KEY_E)) {
        editorEnabled = !editorEnabled;
    }
    BeginUI();
    if(editorEnabled) {
        UpdateEditor(world, &worldCamera);
        if(IsKeyPressed(KEY_S)) {
            Save("a");
        }
        if(IsKeyPressed(KEY_L)) {
            Load("l");
        }
    }
    EndUI();


    // ---------------------------------------------------------------------------------
    // Draw
    // ---------------------------------------------------------------------------------
    BeginDrawing();
        ClearBackground(BLACK);
        Texture2D* tex = GetAsset("LabTileset");
        BeginMode2D(worldCamera);
        DrawWorldTiles(world, tex);
        EndMode2D();
        if(editorEnabled) {
            DrawEditor(world, &worldCamera);
        }
        DrawUI();
    EndDrawing();
}
