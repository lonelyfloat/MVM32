#ifndef H_CHILD
#define H_CHILD

#include "component_types.h" 

Hitbox GetWorldHitbox(ECS* ecs, Entity e);
Vector2 WorldToEntitySpace(ECS* ecs, Entity e,Vector2 pt);

#endif
