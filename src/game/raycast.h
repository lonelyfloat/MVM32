#ifndef H_RAYCAST
#define H_RAYCAST

#include <raylib/raylib.h>

typedef struct RayCollision2D {
    bool hit;
    Vector2 point;
} RayCollision2D;

RayCollision2D CheckCollisionRayLine(Vector2 point, Vector2 direction, Vector2 startPos, Vector2 endPos);

#endif
