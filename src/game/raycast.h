#ifndef H_RAYCAST
#define H_RAYCAST

#include <raylib/raylib.h>
#include "component_memory.h"
#include "serialize.h"
#include "ui/ui_types.h"

typedef struct RayCollision2D {
    bool hit;
    Vector2 point;
    Vector2 normal;
} RayCollision2D;

RayCollision2D CheckCollisionRayLine(Vector2 point, Vector2 direction, Vector2 startPos, Vector2 endPos);

UI_GEN_HEADER(RayCollision2D)
SERIALIZE_GEN_HEADER(RayCollision2D)
_ALLOCATOR_HEADER(RayCollision2D)


#endif
