#include "player.h"
#include <raylib/raymath.h>

// Movement  constants
const float g_acceleration = 400;
const float g_maxSpeed = 275;
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

const float g_SlopeSnap = 5;
const float g_SlopeBoost = 5;
const float g_SlopeWallTolerance = 1;


void UpdatePlayerSlopes(Player* player, World* world) {
    RayCollision2D leftRaycast = CheckCollisionRayWorld((Vector2){player->actor.hitbox.x,player->actor.hitbox.y+player->actor.hitbox.height-20},(Vector2){0,1},world);
    RayCollision2D rightRaycast = CheckCollisionRayWorld((Vector2){player->actor.hitbox.x+player->actor.hitbox.width,player->actor.hitbox.y+player->actor.hitbox.height-20},(Vector2){0,1},world);
    Vector2 leftNorm = leftRaycast.normal;
    Vector2 rightNorm = rightRaycast.normal;
    bool leftValid = leftNorm.x != 0 && leftNorm.y != 0;
    bool rightValid = rightNorm.x != 0 && rightNorm.y != 0;
    // left
    if(leftNorm.x != 0 && leftNorm.y < 0) {
        bool n = false;
        if (leftNorm.x > 0){ // left slope ascending
            if(fabs(player->actor.hitbox.y - leftRaycast.point.y) <= player->actor.hitbox.height) {
                player->leftSlope = true;
            }
            else if(rightValid) {
                n = fabs(rightRaycast.point.y - (player->actor.hitbox.y+player->actor.hitbox.height+player->actor.hitbox.width)) < g_SlopeSnap;
            } else {
                n = fabs(player->actor.hitbox.y - leftRaycast.point.y) < player->actor.hitbox.height + 10;
            }
            if(n) { // descending
                if(fabs(player->actor.hitbox.y - leftRaycast.point.y) < player->actor.hitbox.height+player->actor.hitbox.height) {
                     player->leftSlope = true;
                }
            }
        }
    } else {
        if(player->leftSlope) {
            // player->actor.velocity.x = g_SlopeBoost;
            player->actor.velocity.y = 0;
        }
        player->leftSlope = false;
    }
    if(player->leftSlope) {
        if(fabs(player->actor.hitbox.y - leftRaycast.point.y) > 150) {
            player->leftSlope = false;
        } else {
            player->actor.hitbox.y = leftRaycast.point.y - player->actor.hitbox.height;
        }
    }
    // right side
    if(rightNorm.x != 0 && rightNorm.y < 0) {
        if(!player->leftSlope) {
            bool n = false;
            if (rightNorm.x < 0){ // right slope ascending
                if(fabs(player->actor.hitbox.y - rightRaycast.point.y) <= player->actor.hitbox.height) {
                    player->rightSlope = true;
                }
                else if(leftValid) {
                    n = fabs(leftRaycast.point.y - (player->actor.hitbox.y+player->actor.hitbox.height+player->actor.hitbox.width)) < g_SlopeSnap;
                } else {
                    n = fabs(player->actor.hitbox.y - rightRaycast.point.y) < player->actor.hitbox.height + 10;
                }
                if(n) { // descending
                    if(fabs(player->actor.hitbox.y - rightRaycast.point.y) < player->actor.hitbox.height+player->actor.hitbox.height) {
                         player->rightSlope = true;
                    }
                }
            }
        }
    } else {
        if(player->rightSlope) {
            // player->actor.velocity.x = g_SlopeBoost;
            player->actor.velocity.y = 0;
        }
        player->rightSlope = false;
    }
    if(player->rightSlope) {
        if(fabs(player->actor.hitbox.y - rightRaycast.point.y) > 150) {
            player->rightSlope = false;
        } else {
            player->actor.hitbox.y = rightRaycast.point.y - player->actor.hitbox.height;
        }
    }

    if(!player->leftSlope && !player->rightSlope) {
        player->autoApply = true;
    } else {
        player->autoApply = false;
    }

    if(player->actor.impulse.y < 0) {
        player->autoApply = true;
    }
}


// End player variables and constants
void UpdatePlayer(Player* player, World* world) {
    bool topSlope = false;
    if(player->actor.impulse.y > 0) {
        if(player->actor.impulse.x != 0) {
            topSlope = true;
        }
    }

    if(!topSlope && player->actor.impulse.y != 0) {
        player->actor.velocity.y = 0;
    }

    // Collision handling
    if(player->actor.impulse.y < 0 || (player->leftSlope || player->rightSlope)) {
        player->grounded = true;
        player->canJump = true;
    } else {
        if(player->grounded) {
            player->coyoteTimer = g_coyoteTime;
        }
        player->grounded = false;
    }
    if((!player->leftSlope && !player->rightSlope && !player->grounded)) {
        player->actor.velocity.y += g_gravity;
    }
    player->actor.velocity.y = Clamp(player->actor.velocity.y, -g_terminalYVelo, g_terminalYVelo);


    RayCollision2D leftWallRaycast = (RayCollision2D){false, (Vector2){}, (Vector2){}};
    RayCollision2D rightWallRaycast = (RayCollision2D){false, (Vector2){}, (Vector2){}};
    leftWallRaycast = CheckCollisionRayWorld((Vector2){player->actor.hitbox.x,player->actor.hitbox.y+player->actor.hitbox.height/2},(Vector2){-1,0},world);
    rightWallRaycast = CheckCollisionRayWorld((Vector2){player->actor.hitbox.x+player->actor.hitbox.width,player->actor.hitbox.y+player->actor.hitbox.height/2},(Vector2){1,0},world);


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
                    player->aimDir.x = (!player->sprite.flipped * 2) - 1;
                }

                player->aimDir.y = IsKeyDown(KEY_DOWN) - IsKeyDown(KEY_UP);
                if(player->aimDir.y != 0) player->inputX = 0;
                if(player->inputX != 0) {
                    player->sprite.flipped = !(player->inputX > 0);
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
                    player->actor.velocity.y = g_jumpVelo;
                    player->jumpTrigger = false;
                    player->leftSlope = false;
                    player->rightSlope = false;
                }

                if(IsKeyReleased(KEY_C) && player->actor.velocity.y < -100) {
                    player->actor.velocity.y *= g_variableJumpFrac;
                }



                // Horizontal movement code

                float horizInput = player->inputX;
                // if(player->leftSlope || player->rightSlope) {
                //     horizInput = player->inputX * 0.4;
                // }
                player->actor.velocity.x += horizInput * g_acceleration;
                player->actor.velocity.x = Clamp(player->actor.velocity.x, -g_maxSpeed, g_maxSpeed);
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
                player->actor.velocity = Vector2Add(playerKnockback, player->actor.velocity);
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
            player->actor.velocity.x *= g_groundFriction;
        } else {
            player->actor.velocity.x *= g_airResistance;
        }
    }

    if(leftWallRaycast.hit) {
        if(player->actor.velocity.x < 0 && fabs(leftWallRaycast.point.x - player->actor.hitbox.x) < g_SlopeWallTolerance) {
                player->actor.velocity.x = 0;
        }
    }
    if(rightWallRaycast.hit) {
        if(player->actor.velocity.x > 0 && fabs(rightWallRaycast.point.x - (player->actor.hitbox.x+player->actor.hitbox.width)) < g_SlopeWallTolerance) {
                player->actor.velocity.x = 0;
        }
    }
    if(topSlope) {
        if(player->inputX == 0) {
            player->actor.velocity.x = 0;
            player->actor.velocity.y = g_gravity;
        } else {
            player->actor.velocity.y = (player->actor.impulse.y/fabs(player->actor.impulse.x)) * fabs(player->actor.velocity.x); 
            player->actor.velocity = Vector2Scale(Vector2Normalize(player->actor.velocity), fabs(player->actor.velocity.x));
        }
    }
    
}
