#include "player.h"
#include <raylib/raymath.h>
#include "component_types.h"

// Movement  constants
const float g_acceleration = 400;
const float g_maxSpeed = 300;
const float g_groundFriction = 0.7;
const float g_airResistance = 0.7;
const float g_gravity = 40;
const float g_coyoteTime = 0.1;
const float g_jumpBuffer = 0.04;
const float g_variableJumpFrac = 0.5;
const float g_collisionDampening = 5.0;
const float g_terminalYVelo = 1000;
const float g_jumpVelo = -750;
const float g_cornerTolerance = 5; // # of pixels

const float g_shootChargeTime = 0.2;
const float g_shootKnockback = 25;
const float g_shotLength = 0.2;

const float g_SlopeSnap = 4;
const float g_SlopeBoost = 5;
const float g_SlopeWallTolerance = 1;


// End player variables and constants
void UpdatePlayer(ECS* ecs, Entity e, Room* room) {
    Player* player = GetComponent(ecs, e, PLAYER_COMPONENT);
    Hitbox* hb = GetComponent(ecs, e, HITBOX_COMPONENT);
    Velocity* velo = GetComponent(ecs, e, VELOCITY_COMPONENT);
    Sprite* sprite = GetComponent(ecs, e, SPRITE_COMPONENT);
    Actor* actor = GetComponent(ecs, e, ACTOR_COMPONENT);
    Vector2 impulse = actor->impulse;
    if(impulse.x != 0) {
        // velo->x = 0;
    }
    if(impulse.y != 0) {
        velo->y = 0;
    }
    // Collision handling
    if(impulse.y < 0 || (player->leftSlope || player->rightSlope)) {
        player->grounded = true;
        player->canJump = true;
    } else {
        if(player->grounded) {
            player->coyoteTimer = g_coyoteTime;
        }
        player->grounded = false;
    }
    if(!player->leftSlope && !player->rightSlope && !player->grounded) {
        velo->y += g_gravity;
    }
    velo->y = Clamp(velo->y, -g_terminalYVelo, g_terminalYVelo);


    RayCollision2D leftRaycast = CheckCollisionRayRoom((Vector2){hb->pos.x,hb->pos.y+hb->scale.y-20},(Vector2){0,1},room);
    RayCollision2D rightRaycast = CheckCollisionRayRoom((Vector2){hb->pos.x+hb->scale.x,hb->pos.y+hb->scale.y-20},(Vector2){0,1},room);
    RayCollision2D leftWallRaycast = (RayCollision2D){false, (Vector2){}, (Vector2){}};
    RayCollision2D rightWallRaycast = (RayCollision2D){false, (Vector2){}, (Vector2){}};
    leftWallRaycast = CheckCollisionRayRoom((Vector2){hb->pos.x,hb->pos.y+hb->scale.y/2},(Vector2){-1,0},room);
    rightWallRaycast = CheckCollisionRayRoom((Vector2){hb->pos.x+hb->scale.x,hb->pos.y+hb->scale.y/2},(Vector2){1,0},room);
    Vector2 leftNorm = leftRaycast.normal;
    Vector2 rightNorm = rightRaycast.normal;
    bool leftValid = leftNorm.x != 0 && leftNorm.y != 0;
    bool rightValid = rightNorm.x != 0 && rightNorm.y != 0;
    if(leftNorm.x != 0 && leftNorm.y < 0) {
        bool n = false;
        if (leftNorm.x > 0){ // left slope ascending
            if(fabs(hb->pos.y - leftRaycast.point.y) <= hb->scale.y) {
                player->leftSlope = true;
            }
            else if(rightValid) {
                n = fabs(rightRaycast.point.y - (hb->pos.y+hb->scale.y+hb->scale.x)) < g_SlopeSnap;
            } else {
                n = fabs(hb->pos.y - leftRaycast.point.y) < hb->scale.y + 10;
            }
            if(n) { // descending
                if(fabs(hb->pos.y - leftRaycast.point.y) < hb->scale.y+hb->scale.y) {
                     player->leftSlope = true;
                }
            }
        }
    } else {
        if(player->leftSlope) {
            // velo->x = g_SlopeBoost;
            velo->y = 0;
        }
        player->leftSlope = false;
    }
    if(player->leftSlope) {
        if(fabs(hb->pos.y - leftRaycast.point.y) > 100) {
            player->leftSlope = false;
        } else {
            hb->pos.y = leftRaycast.point.y - hb->scale.y;
        }
    }
    if(rightNorm.x != 0 && rightNorm.y < 0) {
        if(!player->leftSlope) {
            bool n = false;
            if (rightNorm.x < 0){ // right slope ascending
                if(fabs(hb->pos.y - rightRaycast.point.y) <= hb->scale.y) {
                    player->rightSlope = true;
                }
                else if(leftValid) {
                    n = fabs(leftRaycast.point.y - (hb->pos.y+hb->scale.y)) < g_SlopeSnap;
                } else {
                    n = fabs(hb->pos.y - rightRaycast.point.y) < hb->scale.y + 10;
                }
                if(n) { // descending
                    if(fabs(hb->pos.y - rightRaycast.point.y) <= hb->scale.y+hb->scale.y) {
                         player->rightSlope = true;
                    }
                }
            }
        }
    } else {
        if(player->rightSlope) {
            // velo->x += g_SlopeBoost;
            velo->y = 0;
        }
        player->rightSlope = false;
    }
    if(player->rightSlope) {
        if(fabs(hb->pos.y - rightRaycast.point.y) > 100) {
            player->rightSlope = false;
        } else {
            hb->pos.y = rightRaycast.point.y - hb->scale.y;
        }
    }

    if(!player->leftSlope && !player->rightSlope) {
        actor->autoApply = true;
    } else {
        actor->autoApply = false;
    }



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
                    player->leftSlope = false;
                    player->rightSlope = false;
                }

                if(IsKeyReleased(KEY_C) && velo->y < -100) {
                    velo->y *= g_variableJumpFrac;
                }



                // Horizontal movement code

                float horizInput = player->inputX;
                // if(player->leftSlope || player->rightSlope) {
                //     horizInput = player->inputX * 0.4;
                // }
                velo->x += horizInput * g_acceleration;
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

    if(leftWallRaycast.hit) {
        if(velo->x < 0 && fabs(leftWallRaycast.point.x - hb->pos.x) < g_SlopeWallTolerance) {
                velo->x = g_SlopeWallTolerance;
        }
    }
    if(rightWallRaycast.hit) {
        if(velo->x > 0 && fabs(rightWallRaycast.point.x - (hb->pos.x+hb->scale.x)) < g_SlopeWallTolerance) {
                velo->x = -g_SlopeWallTolerance;
        }
    }
    
}

void PlayerSystem(ECS* ecs, Room* room) {
    for(int i = 0; i < ecs->blocks[PLAYER_COMPONENT].count; ++i) {
        Entity e = GetEntity(ecs, PLAYER_COMPONENT, i);
        if(!HasComponents(ecs, e, 4, HITBOX_COMPONENT, VELOCITY_COMPONENT, SPRITE_COMPONENT, ACTOR_COMPONENT)) continue;
        UpdatePlayer(ecs, e, room);
        Hitbox* playerH = GetComponent(ecs, e, HITBOX_COMPONENT);
        // update leg positions
        if(!HasComponent(ecs, e, RELATIONSHIP_COMPONENT)) continue;
        Relationship* r = GetComponent(ecs, e, RELATIONSHIP_COMPONENT);
        Entity child = r->first;
        while(child != NULL_ENTITY) {
            if(!HasComponent(ecs, child, RELATIONSHIP_COMPONENT)) break;
            r = GetComponent(ecs, child, RELATIONSHIP_COMPONENT);
            if(!HasComponents(ecs, child,3, HITBOX_COMPONENT, IK_LEG_COMPONENT, OFFSET_COMPONENT)) {
                child = r->next;
                continue;
            }
            Offset* o = GetComponent(ecs, child, OFFSET_COMPONENT);
            Hitbox* hb = GetComponent(ecs, child, HITBOX_COMPONENT);
            hb->pos = Vector2Add(playerH->pos, *o);
            child = r->next;
        }
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
