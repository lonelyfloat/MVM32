#ifndef H_PLAYER
#define H_PLAYER

#include <raylib/raylib.h>
#include <stdio.h>
#include "arena.h"
#include "serialize.h"
#include "component_memory.h"
#include "ui/ui_types.h"

typedef enum PlayerState {
    PLAYER_NORMAL, // moving around / idling
    PLAYER_CHARGING, // charging laser
    PLAYER_SHOOTING // shooting laser
} PlayerState;
static inline void _Load_PlayerState(FILE* stream, Arena* arena, void* value) {
    _Load_int(stream, arena, value);
}
static inline void _Save_PlayerState(FILE* stream, void* value) {
    _Save_int(stream, value);
}
static inline void _Allocate_PlayerState(Arena* arena, void* value) {
    _Allocate_int(arena, value);
}

static inline void _UI_PlayerState(char* name, void* value) {
    _UI_int(name, value);
}

typedef struct Player {
    PlayerState playerState;
    bool canJump;
    bool jumpTrigger;
    bool grounded;
    float coyoteTimer;
    float jumpBufferTimer;
    float shotDurationTimer; 
    float shootChargeTimer;
    Vector2 aimDir;
    int inputX;
} Player;


#define PLAYER(X)\
    X(PlayerState, playerState)\
    X(bool, canJump)\
    X(bool, jumpTrigger)\
    X(bool, grounded)\
    X(float, coyoteTimer)\
    X(float, jumpBufferTimer)\
    X(float, shotDurationTimer)\
    X(float, shootChargeTimer)\
    X(Vector2, aimDir)\
    X(int, inputX)\

void PlayerSystem(ECS* ecs);
#endif
