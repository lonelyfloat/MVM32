#ifndef H_TYPES
#define H_TYPES

#include <raylib/raylib.h>
#include "components/sprite.h"

// General types for use throughout the game

typedef struct {
    TextureAsset* asset;
    int width, height;
    bool flipped;
    int currentFrame;
} Sprite;

typedef struct {
    Rectangle hitbox;
    Vector2 velocity;
    Vector2 impulse;
} Actor;

#endif
