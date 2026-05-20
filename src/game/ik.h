#ifndef H_IK
#define H_IK

#include <ecs.h>
#include <raylib/raylib.h>

// Components & systems for IK 'nodes'

#define IK_ROOT(X)\
    X(bool, hasTarget)\
    X(Vector2, target)

#define IK_NODE(X)\
    X(float, length)

void ResolveIK(ECS* ecs);

#define IK_LEG(X)\
    X(float, xOffset)\
    X(float, strideLength)

// void IKLegSystem(ECS* ecs, Vector2 groundA, Vector2 groundB);

#endif
