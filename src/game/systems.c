#include "systems.h"
#include <stdlib.h>
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
        for(int j = 0; j < ecs->blocks[STATIC_GEOMETRY_COMPONENT].count; ++j) {
            Entity stat = GetEntity(ecs, STATIC_GEOMETRY_COMPONENT, j);
            if(!HasComponent(ecs, stat, HITBOX_COMPONENT)) continue;
            Hitbox* staticHB = GetComponent(ecs, stat, HITBOX_COMPONENT);
            impulse->impulse = Vector2Add(impulse->impulse,
                    ResolveRectStaticRect(HitboxToRect(*hb), HitboxToRect(*staticHB)));
            if(impulse->autoApply) {
                hb->pos = Vector2Add(hb->pos, impulse->impulse);
            }
        }
    }

}

typedef struct Order {
    uint32_t i;
    ZOrder z;
} Order;

int compare(const void* _a, const void* _b) {
    // ascending order
    int a = ((Order*)_a)->z.z;
    int b = ((Order*)_b)->z.z;
    return a - b;
}

// Z-Ordering
void SetRenderOrder(ECS* ecs) {
    if(ecs->blocks[ZORDER_COMPONENT].count == 0) return;
    Order dataCopy[ecs->blocks[ZORDER_COMPONENT].count];
    for(int i = 0; i < ecs->blocks[ZORDER_COMPONENT].count; ++i) {
        dataCopy[i].i = i;
        dataCopy[i].z =  IndexComponent(ecs, ZOrder, ZORDER_COMPONENT,i);
    }
    qsort(dataCopy, ecs->blocks[ZORDER_COMPONENT].count, sizeof(Order),compare);
    for(int i = 0; i < ecs->blocks[ZORDER_COMPONENT].count; ++i) {
        uint32_t curr = i;
        uint32_t next = dataCopy[curr].i;
        while(curr != next) {
            // Swap
            EntityIndex currSparse = ecs->blocks[ZORDER_COMPONENT].indices[dataCopy[curr].i];
            Entity currDense = ecs->blocks[ZORDER_COMPONENT].entities[ecs->blocks[ZORDER_COMPONENT].indices[dataCopy[curr].i]];
            ZOrder currZ = dataCopy[currSparse].z;
            ecs->blocks[ZORDER_COMPONENT].indices[dataCopy[curr].i] = ecs->blocks[ZORDER_COMPONENT].indices[next]; 
            ecs->blocks[ZORDER_COMPONENT].entities[currSparse] = ecs->blocks[ZORDER_COMPONENT].indices[next]; 
            ((ZOrder*)ecs->blocks[ZORDER_COMPONENT].components)[currSparse] = ((ZOrder*)ecs->blocks[ZORDER_COMPONENT].components)[next];
            ecs->blocks[ZORDER_COMPONENT].entities[ecs->blocks[ZORDER_COMPONENT].indices[next]] = currDense; 
            ecs->blocks[ZORDER_COMPONENT].indices[next] = currSparse; 
            ((ZOrder*)ecs->blocks[ZORDER_COMPONENT].components)[next] = currZ; 
            // End swap
            dataCopy[curr].i = curr;
            curr = next;
            next = dataCopy[curr].i;
        }
    }

    for(int i = 0; i < ecs->blocks[ZORDER_COMPONENT].count; ++i) {
        ZOrder z = IndexComponent(ecs, ZOrder, ZORDER_COMPONENT, i);
        printf("z: %d\n", z.z);
    }
}
