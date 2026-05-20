#include "player.h"
#include <raylib/raymath.h>
#include "component_types.h"

// Movement  constants
const float g_acceleration = 100;
const float g_maxSpeed = 400;
const float g_groundFriction = 0.8;
const float g_airResistance = 0.7;
const float g_gravity = 30;
const float g_coyoteTime = 0.1;
const float g_jumpBuffer = 0.06;
const float g_variableJumpFrac = 0.5;
const float g_collisionDampening = 5.0;
const float g_terminalYVelo = 1000;
const float g_jumpVelo = -750;
const float g_cornerTolerance = 5; // # of pixels

const float g_shootChargeTime = 0.2;
const float g_shootKnockback = 25;
const float g_shotLength = 0.2;


// End player variables and constants
void UpdatePlayer(ECS* ecs, Entity e) {
    Player* player = GetComponent(ecs, e, PLAYER_COMPONENT);
    Hitbox* hb = GetComponent(ecs, e, HITBOX_COMPONENT);
    Velocity* velo = GetComponent(ecs, e, VELOCITY_COMPONENT);
    Sprite* sprite = GetComponent(ecs, e, SPRITE_COMPONENT);
    Actor* actor = GetComponent(ecs, e, ACTOR_COMPONENT);

    bool groundChecked = false;
    Vector2 impulse = actor->impulse;
    hb->pos = Vector2Add(hb->pos, impulse);
    // if(impulse.x != 0 && impulse.y != 0) {
    //     player->onSlope = true;
    //     player->slopeDir = Vector2Normalize(impulse);

    // } else {
    // }
    if(impulse.x != 0) {
        velo->x = 0;
    }
    if(impulse.y != 0) {
        velo->y = 0;
    }
    if(!groundChecked) {
        if(impulse.y < 0) {
            player->grounded = true;
            groundChecked = true;
            player->canJump = true;
        } else {
            if(player->grounded) {
                player->coyoteTimer = g_coyoteTime;
            }
            player->grounded = false;
        }
    }
    // Gravity / terminal y velo
    velo->y += g_gravity;
    switch(player->playerState) {
        case PLAYER_NORMAL: {
                if(IsKeyDown(KEY_X)) {
                    player->playerState = PLAYER_CHARGING;
                    player->shootChargeTimer = g_shootChargeTime;
                    player->inputX = 0;
                    return;
                }
                player->inputX = IsKeyDown(KEY_RIGHT) - IsKeyDown(KEY_LEFT);

                //  Aim Direction code

                if(player->aimDir.x == 0 && player->aimDir.y == 0) {
                    player->aimDir.x = (!sprite->flipped * 2) - 1;
                }

                player->aimDir.y = IsKeyDown(KEY_DOWN) - IsKeyDown(KEY_UP);
                if(player->aimDir.y != 0) player->inputX = 0;
                if(player->inputX != 0) {
                    sprite->flipped = !(player->inputX > 0);
                    player->aimDir = (Vector2){player->inputX, 0};
                }
                if(player->aimDir.y != 0) player->aimDir.x = 0;
                // END input processing code


                // Jump logic code

                if(player->coyoteTimer >= 0) {
                    player->coyoteTimer -= GetFrameTime();
                } else if(player->coyoteTimer != -1) {
                    player->coyoteTimer = -1;
                    player->canJump = false;
                }

                if(IsKeyPressed(KEY_C)) {
                    if(player->canJump) {
                        player->jumpTrigger = true;
                    } else {
                        player->jumpBufferTimer = g_jumpBuffer;
                    }
                }

                if(player->jumpBufferTimer >= 0) {
                    player->jumpBufferTimer -= GetFrameTime();
                    if(player->canJump) {
                        player->jumpTrigger = true;
                        player->jumpBufferTimer = -1;
                    }
                } 

                if(player->jumpTrigger) {
                    velo->y = g_jumpVelo;
                    player->jumpTrigger = false;
                }

                if(IsKeyReleased(KEY_C) && velo->y < -100) {
                    velo->y *= g_variableJumpFrac;
                }



                // Horizontal movement code

                velo->x += player->inputX * g_acceleration;
                if(player->onSlope) {
                    float moveDistance = fabs(velo->x);
                    velo->y = player->slopeDir.y * moveDistance;
                    velo->x = player->slopeDir.x * velo->x;
                }
                velo->x = Clamp(velo->x, -g_maxSpeed, g_maxSpeed);
            }
            break;
        case PLAYER_CHARGING: 
            if(player->shootChargeTimer > 0) { // Shot is still charging
                player->shootChargeTimer -= GetFrameTime();
                if(!IsKeyDown(KEY_X)) { 
                    player->playerState = PLAYER_NORMAL;
                    player->shootChargeTimer = -1;
                }
            } else { // Shot is charged
                if(!IsKeyDown(KEY_X)) { 
                    player->playerState = PLAYER_SHOOTING;
                    player->shotDurationTimer = g_shotLength;
                }
            }
            break;
        case PLAYER_SHOOTING: 
        {
            if(player->shotDurationTimer > 0) {
                player->shotDurationTimer -= GetFrameTime();
                Vector2 playerKnockback = Vector2Scale(Vector2Negate(player->aimDir),g_shootKnockback);
                *velo = Vector2Add(playerKnockback, *velo);
            } else {
                player->playerState = PLAYER_NORMAL;
                player->shotDurationTimer = -1;
                player->shootChargeTimer = -1;
            }
            break;
        }
    }
    if(player->inputX == 0) {
        if(player->grounded) {
            velo->x *= g_groundFriction;
        } else {
            velo->x *= g_airResistance;
        }
    }
    velo->y = Clamp(velo->y, -g_terminalYVelo, g_terminalYVelo);
}

void PlayerSystem(ECS* ecs) {
    for(int i = 0; i < ecs->blocks[PLAYER_COMPONENT].count; ++i) {
        Entity e = GetEntity(ecs, PLAYER_COMPONENT, i);
        if(!HasComponents(ecs, e, 4, HITBOX_COMPONENT, VELOCITY_COMPONENT, SPRITE_COMPONENT, ACTOR_COMPONENT)) continue;
        UpdatePlayer(ecs, e);
    }
}

// FORMER CODE
//
//     if(impulse.x != 0) {
//         // Top corner rounding
//         if(fabs(staticRecs[i].y - (player.y + player.height)) < cornerTolerance && playerVelo.y > 0) {
//             player.y = staticRecs[i].y - player.height;
//         }
//         else {
//             playerVelo.x = 0;
//         }
//     }
// 
//     if(impulse.y != 0) {
//         // Bottom corner rounding
//         bool cornerLeft = fabs((player.x + player.width) - staticRecs[i].x) < cornerTolerance;
//         bool cornerRight = fabs(player.x - (staticRecs[i].x + staticRecs[i].width)) < cornerTolerance;
//         if(playerVelo.y < 0 && cornerLeft) {
//             player.x = staticRecs[i].x - player.width;
//         }
//         else if(playerVelo.y < 0 && cornerRight) {
//             player.x = staticRecs[i].x + staticRecs[i].width;
//         } else {
//             playerVelo.y = 0;
//         }
//     }
