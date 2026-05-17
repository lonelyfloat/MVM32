#include "systems.h"
#include <raylib/raymath.h>
#include "utils.h"

void VelocitySystem(ECS* ecs) {
    for(int i = 0; i < ecs->blocks[VELOCITY_COMPONENT].count; ++i) {
        Entity e = GetEntity(ecs, ACTOR_COMPONENT, i);
        if(!HasComponent(ecs, e, HITBOX_COMPONENT)) continue;
        Hitbox* hb = GetComponent(ecs, e, HITBOX_COMPONENT);
        hb->x += IndexComponent(ecs, Velocity, VELOCITY_COMPONENT, i).x * GetFrameTime();
        hb->y += IndexComponent(ecs, Velocity, VELOCITY_COMPONENT, i).y * GetFrameTime();
    }
}

void CollisionSystem(ECS* ecs) {
    for(int i = 0; i < ecs->blocks[ACTOR_COMPONENT].count; ++i) {
        Entity e = GetEntity(ecs, ACTOR_COMPONENT, i);
        if(!HasComponent(ecs, e, HITBOX_COMPONENT)) continue;
        Actor* impulse = &IndexComponent(ecs, Actor, ACTOR_COMPONENT, i);
        impulse->impulse = Vector2Zero();
        Hitbox* moving = GetComponent(ecs, e, HITBOX_COMPONENT);
        for(int j = 0; j < ecs->blocks[STATIC_GEOMETRY_COMPONENT].count; ++j) {
            Entity stat = GetEntity(ecs, STATIC_GEOMETRY_COMPONENT, j);
            if(!HasComponent(ecs, stat, HITBOX_COMPONENT)) continue;
            Hitbox* staticBox = GetComponent(ecs, stat, HITBOX_COMPONENT);
            impulse->impulse = Vector2Add(impulse->impulse, ResolveRectStaticRect(*moving, *staticBox));
        }
    }
}
