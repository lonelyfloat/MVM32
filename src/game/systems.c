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

void IKPoleSystem(ECS *ecs) {
    for(int i = 0; i < ecs->blocks[IK_POLE_COMPONENT].count; ++i) {
        Entity e = GetEntity(ecs, IK_POLE_COMPONENT, i);
        IKPole pole = IndexComponent(ecs, IKPole, IK_POLE_COMPONENT, i);
        if(!HasComponents(ecs, e, 2, IK_ROOT_COMPONENT, RELATIONSHIP_COMPONENT)) continue;
        Relationship* r = GetComponent(ecs, e, RELATIONSHIP_COMPONENT);
        Entity child = r->first;
        while(child != NULL_ENTITY) {
            if(!HasComponent(ecs, child, RELATIONSHIP_COMPONENT)) break;
            r = GetComponent(ecs, child, RELATIONSHIP_COMPONENT);
            if(!HasComponents(ecs, child,2, HITBOX_COMPONENT, IK_NODE_COMPONENT)) {
                child = r->next;
                continue;
            }
            Hitbox* hb = GetComponent(ecs, child, HITBOX_COMPONENT);
            hb->pos = Vector2Add(hb->pos, Vector2Scale(pole, 20));
            child = r->next;
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
            uint32_t currIdx = dataCopy[curr].i;
            uint32_t nextIdx = dataCopy[next].i;
            Entity tempDense = ecs->blocks[ZORDER_COMPONENT].entities[currIdx];
            Entity nextDense = ecs->blocks[ZORDER_COMPONENT].entities[nextIdx];
            EntityIndex tempSparse = ecs->blocks[ZORDER_COMPONENT].indices[GetID(tempDense)];
            EntityIndex nextSparse = ecs->blocks[ZORDER_COMPONENT].indices[GetID(nextDense)];
            ZOrder tempZ = ((ZOrder*)ecs->blocks[ZORDER_COMPONENT].components)[tempSparse];
            ecs->blocks[ZORDER_COMPONENT].entities[currIdx] = ecs->blocks[ZORDER_COMPONENT].entities[nextIdx];
            ecs->blocks[ZORDER_COMPONENT].indices[GetID(tempDense)] = ecs->blocks[ZORDER_COMPONENT].indices[GetID(nextDense)];
            ((ZOrder*)ecs->blocks[ZORDER_COMPONENT].components)[tempSparse] = ((ZOrder*)ecs->blocks[ZORDER_COMPONENT].components)[nextSparse];
            ecs->blocks[ZORDER_COMPONENT].entities[nextIdx] = tempDense;
            ecs->blocks[ZORDER_COMPONENT].indices[GetID(nextDense)] = tempSparse;
            ((ZOrder*)ecs->blocks[ZORDER_COMPONENT].components)[nextSparse] = tempZ;
            // End swap
            dataCopy[curr].i = curr;
            curr = next;
            next = dataCopy[curr].i;
        }
    }
}
