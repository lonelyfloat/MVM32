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
const float airResistance = 0.7;
const float gravity = 30;
const float coyoteTime = 0.1;
const float jumpBuffer = 0.06;
const float variableJumpFrac = 0.5;
const float collisionDampening = 5.0;
const float terminalYVelo = 1000;
const float jumpVelo = -750;
const float cornerTolerance = 15; // # of pixels

Rectangle player = {0, 0, 50, 70};
Vector2 playerVelo = { 0 };

bool grounded = false;
bool canJump = false;
bool jumpTrigger = false;
float coyoteTimer = 0.0;
float jumpBufferTimer = 0.0;

#define STATIC_RECS 3
Rectangle staticRecs[STATIC_RECS] = {
    {0, 600, 1280, 120},
    {900, 400, 200, 40},
    {400, 290, 200, 40}
};
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
    playerVelo.y += gravity;
    playerVelo.y = Clamp(playerVelo.y, -terminalYVelo, terminalYVelo);

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
            jumpBufferTimer = -1;
        }
    } 

    if(jumpTrigger) {
        playerVelo.y = jumpVelo;
        jumpTrigger = false;
    }

    if(IsKeyReleased(KEY_C) && playerVelo.y < -100) {
        playerVelo.y *= variableJumpFrac;
    }

    int inputX = IsKeyDown(KEY_RIGHT) - IsKeyDown(KEY_LEFT);
    playerVelo.x += inputX * acceleration;
    playerVelo.x = Clamp(playerVelo.x, -maxSpeed, maxSpeed);
    if(inputX == 0) {
        if(grounded) {
            playerVelo.x *= groundFriction;
        } else {
            playerVelo.x *= airResistance;
        }
    }

    player.x += playerVelo.x * GetFrameTime();
    player.y += playerVelo.y * GetFrameTime();

    bool groundChecked = false;
    for(int i = 0; i < STATIC_RECS; ++i) {
        Vector2 impulse = ResolveRectStaticRect(player, staticRecs[i]);
        player.x += impulse.x;
        player.y += impulse.y;
        if(impulse.x != 0) {
            // Top corner rounding
            if(fabs(staticRecs[i].y - (player.y + player.height)) < cornerTolerance && playerVelo.y > 0) {
                player.y = staticRecs[i].y - player.height;
                printf("CORNER\n");
            }
            else {
                playerVelo.x = 0;
            }
        }

        if(impulse.y != 0) {

            bool cornerLeft = fabs((player.x + player.width) - staticRecs[i].x) < cornerTolerance;
            bool cornerRight = fabs(player.x - (staticRecs[i].x + staticRecs[i].width)) < cornerTolerance;
            if(playerVelo.y < 0 && cornerLeft) {
                    player.x = staticRecs[i].x - player.width;
            }
            else if(playerVelo.y < 0 && cornerRight) {
                    player.x = staticRecs[i].x + staticRecs[i].width;
            } else {
                playerVelo.y = 0;
            }
        }
        if(!groundChecked) {
            if(impulse.y < 0) {
                grounded = true;
                groundChecked = true;
                canJump = true;
            } else {
                if(grounded) {
                    coyoteTimer = coyoteTime;
                }
                grounded = false;
            }
        }
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
        for(int i = 0; i < STATIC_RECS; ++i) {
            DrawRectangleRec(staticRecs[i], BLUE);
            DrawRectangle(staticRecs[i].x, staticRecs[i].y, cornerTolerance, cornerTolerance, GREEN);
            DrawRectangle(staticRecs[i].x  + staticRecs[i].width - cornerTolerance, staticRecs[i].y, cornerTolerance, cornerTolerance, GREEN);

            DrawRectangle(staticRecs[i].x, staticRecs[i].y + staticRecs[i].height - cornerTolerance, cornerTolerance, cornerTolerance, PURPLE);
            DrawRectangle(staticRecs[i].x  + staticRecs[i].width - cornerTolerance, staticRecs[i].y + staticRecs[i].height - cornerTolerance, cornerTolerance, cornerTolerance, PURPLE);
        }
    EndDrawing();
}
