#include "program.h"
#include <raylib/raylib.h>
#include <raylib/raymath.h>
#include "assets.h"
#include "ui/ui.h"
#include <stdio.h>
#include "player.h"
#include "world.h"

int screenWidth = 800;
int screenHeight = 600;

Arena* arena;

ImGuiContext* ctx;

const int gridSize = 50;
Camera2D worldCamera = (Camera2D) {.offset=(Vector2){-gridSize/2.0,-gridSize/2.0},.target=(Vector2){},.rotation=0.0,.zoom=1.0};

void Init(Arena* gameArena) {
    arena = gameArena;
    printf("Initializing game...\n");
    InitAssets(gameArena, 10);
    LoadAsset("./assets/lab_tileset.png", "LabTileset", ASSET_TYPE_TEXTURE);
    LoadAsset("./assets/player.png", "Player", ASSET_TYPE_TEXTURE);
    screenWidth = GetScreenWidth();
    screenHeight = GetScreenHeight();
    ctx = GetImGuiContext();
    ImGui_SetCurrentContext(ctx);
}

void Save(char* file) {
}

void Load(char* file) {
}

void UpdateDrawFrame(void) {
    // Update
    //----------------------------------------------------------------------------------
    BeginUI();
    EndUI();


    // ---------------------------------------------------------------------------------
    // Draw
    // ---------------------------------------------------------------------------------
    BeginDrawing();
        ClearBackground(BLACK);
        DrawUI();
    EndDrawing();
}
