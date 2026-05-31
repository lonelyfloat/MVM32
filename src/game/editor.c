#include "editor.h"
#include "ui/ui.h"
#include <raylib/raymath.h>

#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600

enum EditorMode {
    EDITOR_TERRAIN,
    EDITOR_UTILS
} editorMode = EDITOR_TERRAIN;
const char* editorModeNames[2] = { "Terrain", "Utils" };

void UpdateEditor(World* world, Camera2D* camera) {
    ImGui_Begin("Editor Tools", NULL, ImGuiWindowFlags_None);
    if(ImGui_BeginCombo("##EditorModeCombo", editorModeNames[editorMode], ImGuiComboFlags_None)) {
        for(int i = 0; i < 2; ++i) {
            if(ImGui_Selectable(editorModeNames[i])) {
                editorMode = i;
            }
        }
        ImGui_EndCombo();
    }
    if(editorMode == EDITOR_UTILS) {
        // Extra ui for entity spawner tiles
    }
    ImGui_End();
    // 
    // Zoom based on mouse wheel
    float wheel = GetMouseWheelMove();
    ImGuiIO* io = ImGui_GetIO();
    Vector2 mousePos = GetScreenToWorld2D(GetMousePosition(),*camera);;
    if (wheel != 0 && !io->WantCaptureMouse)
    {
        // Get the world point that is under the mouse
        // Set the offset to where the mouse is
        camera->offset = GetMousePosition();
        // Set the target to match, so that the worldCamera maps the world space point
        // under the cursor to the screen space point under the cursor at any zoom
        camera->target = mousePos;
        // Zoom increment
        // Uses log scaling to provide consistent zoom speed
        float scale = 0.2f*wheel;
        camera->zoom = Clamp(expf(logf(camera->zoom)+scale), 0.125f, 64.0f);
    }

    Vector2 editorCursor = (Vector2){floor(mousePos.x/world->gridSize), floor(mousePos.y/world->gridSize)};

    if(!io->WantCaptureMouse) {
        if(IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
            if(editorCursor.x >= 0 && editorCursor.x < world->width
            && editorCursor.y >= 0 && editorCursor.y < world->height) {
                world->editorGrid[(int)editorCursor.x][(int)editorCursor.y] = 1;
                Autotile(world);
            }
        }
        if(IsMouseButtonDown(MOUSE_RIGHT_BUTTON)) {
            if(editorCursor.x >= 0 && editorCursor.x < world->width
            && editorCursor.y >= 0 && editorCursor.y < world->height) {
                world->editorGrid[(int)editorCursor.x][(int)editorCursor.y] = 0;
                Autotile(world);
            }
        }
    }
}

void DrawEditor(World* world, Camera2D* camera) {
    Vector2 o = GetWorldToScreen2D((Vector2){0,0}, *camera);
    Vector2 offset = (Vector2) {
        o.x - world->gridSize*camera->zoom*ceil(o.x/(world->gridSize*camera->zoom)),
        o.y - world->gridSize*camera->zoom*ceil(o.y/(world->gridSize*camera->zoom))};
    float guiGridSize = world->gridSize*camera->zoom;
    float guiScreenWidth = SCREEN_WIDTH - offset.x;
    float guiScreenHeight = SCREEN_HEIGHT - offset.y;
    Color gridColor = ColorAlpha(BLUE, 0.5);
    for(int x = 0; x < (int)(guiScreenWidth / guiGridSize); ++x) {
        DrawLine(offset.x + x*guiGridSize, offset.y, offset.x + x*guiGridSize, offset.y+guiScreenHeight, gridColor);
    }
    for(int y = 0; y < (int)(guiScreenHeight / guiGridSize); ++y) {
        DrawLine(offset.x, offset.y + y*guiGridSize, offset.x+guiScreenWidth, offset.y + y * guiGridSize, gridColor);
    }

    BeginMode2D(*camera);
    DrawRectangleLinesEx((Rectangle){0,0,world->width*world->gridSize,world->height*world->gridSize},10, RED);
    for(int i = 0; i < world->roomCount; ++i) {
        DrawRectangleLinesEx(world->rooms[i].bounds, 10, ColorAlpha(GREEN, 0.5));
    }
    EndMode2D();
}
