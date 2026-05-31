#ifndef H_PLAYER
#define H_PLAYER

#include <raylib/raylib.h>
#include "world.h"
#include "types.h"

typedef enum PlayerState {
    PLAYER_NORMAL, // moving around / idling
    PLAYER_CHARGING, // charging laser
    PLAYER_SHOOTING // shooting laser
} PlayerState;

typedef struct Player {
    Actor actor;
    Sprite sprite;
    // Player-specific scheisse
    PlayerState playerState;
    bool canJump;
    bool jumpTrigger;
    bool grounded;
    bool leftSlope;
    bool rightSlope;
    bool autoApply;
    Vector2 slopeDir;
    float coyoteTimer;
    float jumpBufferTimer;
    float shotDurationTimer; 
    float shootChargeTimer;
    Vector2 aimDir;
    int inputX;
} Player;

void UpdatePlayer(Player* player, World* room);
void UpdatePlayerSlopes(Player* player, World* room);

#endif
