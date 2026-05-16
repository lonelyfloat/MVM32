#ifndef H_UTILS
#define H_UTILS

// An extension to raylib's collision functionality that allows static-body collision resolution
// All return the projection vector

#include <raylib/raylib.h>
// All of the functions I want:

typedef Vector2 Triangle[3];

// Working: 
Vector2 ResolveRectStaticRect(Rectangle moving, Rectangle st);
Vector2 ResolveCircleStaticRect(Vector2 center, float radius, Rectangle rec);

// In progress:
Vector2 ResolveCircleStaticTriangle(Vector2 center, float radius, Triangle tri);
Vector2 ResolveRectStaticTriangle(Rectangle rec, Vector2 triStart, Vector2 triBounds);

// Low-Priority:
// Vector2 ResolveTriangleStaticRect(Triangle tri, Rectangle rec);
// Vector2 ResolveTriangleStaticCircle(Triangle tri, Vector2 center, float radius);
// Vector2 ResolveRectStaticCircle(Rectangle, Vector2 center, float radius);
// Vector2 ResolveTriangleStaticTriangle(Triangle moving, Triangle static);


/* In-built components?? 
Haven't decided what to do with this code yet

// macros to generate component-based usage

#define INTERNAL_COMPONENTS(init)\
    init(Position, POSITION)\
    init(Velocity, VELOCITY)\
    init(Collider, COLLIDER)\
    init(Grounded, GROUNDED)

#define INTERNAL_TAGS(init)\

typedef Vector2 Position;

typedef Vector2 Velocity;

typedef enum CollisionShape {
    RECT_COLLIDER,
    CIRCLE_COLLIDER,
} CollisionShape;

typedef struct Collider {
    union {
        // For a rectangle or triangle this is the width & height
        Vector2 bounds; 
        float radius;
    } sizeData;
    CollisionShape shape;
} Collider;

// Collider(shape, Vector2(bounds.x,bounds.y))

typedef struct Grounded { float gravity; bool grounded;  } Grounded;
*/
#endif
