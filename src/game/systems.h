#ifndef H_SYSTEMS
#define H_SYSTEMS
// A bunch of simple systems that don't need to be in separate files etc
#include "component_types.h"

void VelocitySystem(ECS* ecs);
void CollisionSystem(ECS* ecs);
void IKPoleSystem(ECS *ecs);
void SetRenderOrder(ECS* ecs);

#endif
