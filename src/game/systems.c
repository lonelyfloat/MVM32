#include "systems.h"
#include "child.h"
#include <raylib/raymath.h>
#include "utils.h"

void VelocitySystem(ECS* ecs) {
    for(int i = 0; i < ecs->blocks[VELOCITY_COMPONENT].count; ++i) {
        Entity e = GetEntity(ecs, ACTOR_COMPONENT, i);
        if(!HasComponent(ecs, e, HITBOX_COMPONENT)) continue;
        Hitbox* hb = GetComponent(ecs, e, HITBOX_COMPONENT);
        hb->pos = Vector2Add(hb->pos, Vector2Scale(IndexComponent(ecs, Velocity, VELOCITY_COMPONENT, i), GetFrameTime()));
    }
}

void CollisionSystem(ECS* ecs) {
    for(int i = 0; i < ecs->blocks[ACTOR_COMPONENT].count; ++i) {
        Entity e = GetEntity(ecs, ACTOR_COMPONENT, i);
        if(!HasComponent(ecs, e, HITBOX_COMPONENT)) continue;
        Actor* impulse = &IndexComponent(ecs, Actor, ACTOR_COMPONENT, i);
        impulse->impulse = Vector2Zero();
        Hitbox* hb = GetComponent(ecs, e, HITBOX_COMPONENT);
        Hitbox moving = GetWorldHitbox(ecs, e);
        for(int j = 0; j < ecs->blocks[STATIC_GEOMETRY_COMPONENT].count; ++j) {
            Entity stat = GetEntity(ecs, STATIC_GEOMETRY_COMPONENT, j);
            if(!HasComponent(ecs, stat, HITBOX_COMPONENT)) continue;
            Hitbox staticBox = GetWorldHitbox(ecs, stat);
            impulse->impulse = Vector2Add(impulse->impulse,
                    ResolveRectStaticRect(HitboxToRect(moving), HitboxToRect(staticBox)));
            if(impulse->autoApply) {
                hb->pos = Vector2Add(hb->pos, impulse->impulse);
            }
        }
    }
}
