#ifndef H_IK
#define H_IK

#include <ecs.h>
#include <raylib/raylib.h>

// Components & systems for FABRIK 'nodes'

#define FABRIK_ROOT(X)\
    X(Vector2, target)\
    X(float, lengthSum)

#define FABRIK_NODE(X)\
    X(float, length)

void FABRIKSystem(ECS* ecs, int iterations);

#endif
