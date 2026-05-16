#include "program.h"
#include <raylib/raylib.h>
#include <raylib/raymath.h>
#include "assets.h"
#include "ecs.h"
#include "ui/ui.h"
#include "component_types.h"
#include <stdio.h>
#include "utils.h"
#include <string.h>

ECS* ecs;

int screenWidth = 1280;
int screenHeight = 720;

Arena* arena;

const float acceleration = 100;
const float maxSpeed = 500;
const float groundFriction = 0.8;
const float gravity = 20;
const float coyoteTime = 0.1;
const float jumpBuffer = 0.06;
const float variableJumpFrac = 0.5;

Rectangle player = {0, 0, 50, 70};
Vector2 playerVelo = { 0 };

bool grounded = false;
bool canJump = false;
bool jumpTrigger = false;
float coyoteTimer = 0.0;
float jumpBufferTimer = 0.0;

Rectangle groundTest = {0, 600, 1000, 220};
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
    // ctx = GetImGuiContext();
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
    if(IsKeyDown(KEY_LEFT_CONTROL) && IsKeyPressed(KEY_L))
        Load("./assets/game_data");
    if(IsKeyDown(KEY_LEFT_CONTROL) && IsKeyPressed(KEY_S))
        Save("./assets/game_data"); 

    // Begin Player code
    if(grounded) {
        playerVelo.y = 0;
    }
    playerVelo.y += gravity;

    if(coyoteTimer >= 0) {
        coyoteTimer -= GetFrameTime();
    } else if(coyoteTimer != -1) {
        coyoteTimer = -1;
        canJump = false;
    }

    if(IsKeyPressed(KEY_C)) {
        if(canJump) {
            jumpTrigger = true;
        } else {
            jumpBufferTimer = jumpBuffer;
        }
    }

    if(jumpBufferTimer >= 0) {
        jumpBufferTimer -= GetFrameTime();
        if(canJump) {
            jumpTrigger = true;
            printf("trigger\n");
            jumpBufferTimer = -1;
        }
    } 

    if(jumpTrigger) {
        playerVelo.y = -600;
        jumpTrigger = false;
    }

    if(IsKeyReleased(KEY_C) && playerVelo.y < 0) {
        playerVelo.y *= variableJumpFrac;
    }

    int inputX = IsKeyDown(KEY_RIGHT) - IsKeyDown(KEY_LEFT);
    playerVelo.x += inputX * acceleration;
    playerVelo.x = Clamp(playerVelo.x, -maxSpeed, maxSpeed);
    if(grounded) {
        playerVelo.x *= groundFriction;
    }

    player.x += playerVelo.x * GetFrameTime();
    player.y += playerVelo.y * GetFrameTime();

    Vector2 impulse = ResolveRectStaticRect(player, groundTest);
    player.x += impulse.x;
    player.y += impulse.y;
    if(Vector2LengthSqr(impulse) > 0.01) {
        grounded = true;
        canJump = true;
    } else {
        if(grounded) {
            coyoteTimer = coyoteTime;
        }
        grounded = false;
    }


    // End Player code
    
    BeginUI();
    EntityPanel(arena,ecs, &inspect, IsKeyDown(KEY_LEFT_CONTROL) && IsKeyPressed(KEY_E));
    EndUI();


    // ---------------------------------------------------------------------------------
    // Draw
    // ---------------------------------------------------------------------------------
    BeginDrawing();
        ClearBackground(RAYWHITE);
        DrawUI();
        DrawRectangleRec(player, RED);
        DrawRectangleRec(groundTest, BLUE);
    EndDrawing();
}
