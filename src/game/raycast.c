#include "raycast.h"
#include <raylib/raymath.h>

RayCollision2D CheckCollisionRayLine(Vector2 point, Vector2 dir, Vector2 startPos, Vector2 endPos) {
    Vector2 p4 = (Vector2){point.x+dir.x, point.y+dir.y};
    float den = (startPos.x - endPos.x)*(point.y - p4.y) - (startPos.y - endPos.y)*(point.x-p4.x);
    if(den == 0) return (RayCollision2D){false, (Vector2){}, (Vector2){}};
    float t = ((startPos.x - point.x)*(point.y-p4.y) - (startPos.y - point.y)*(point.x-p4.x)) / den;
    float u = -((startPos.x - endPos.x)*(startPos.y-point.y) - (startPos.y - endPos.y)*(startPos.x - point.x)) / den;
    if(t <= 0 || t >= 1 || u <= 0) return (RayCollision2D){false, (Vector2){}, (Vector2){}};
    Vector2 pt = (Vector2){startPos.x + t * (endPos.x - startPos.x), startPos.y + t * (endPos.y - startPos.y)};
    Vector2 v3 = Vector2Normalize(Vector2Subtract(endPos, startPos));
    Vector2 normal = (Vector2){v3.y,-v3.x};
    return (RayCollision2D){true, pt, normal};
}
