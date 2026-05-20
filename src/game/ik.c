#include "ik.h"
#include <raylib/raymath.h>
#include "component_types.h"
#include "child.h"
#include "room.h"
#include "raycast.h"
#include "utils.h"

void ResolveIK(ECS* ecs, int iterations) {
    for(int i = 0; i < ecs->blocks[IK_ROOT_COMPONENT].count; ++i) {
        Entity entity = GetEntity(ecs,IK_ROOT_COMPONENT,i); // temp assume its the root
        if(!HasComponents(ecs, entity, 2, RELATIONSHIP_COMPONENT, HITBOX_COMPONENT)) continue;
        IKRoot ikRoot = IndexComponent(ecs, IKRoot, IK_ROOT_COMPONENT, i);
        ikRoot.target = GetMousePosition();
        Relationship* r = GetComponent(ecs, entity, RELATIONSHIP_COMPONENT);
        if(r->first == NULL_ENTITY) continue;
        Entity nextEntity = r->first;
        for(int k = 0; k < iterations; ++k) {
            entity = GetEntity(ecs,IK_ROOT_COMPONENT,i); // temp assume its the root
            ikRoot = IndexComponent(ecs, IKRoot, IK_ROOT_COMPONENT, i);
            r = GetComponent(ecs, entity, RELATIONSHIP_COMPONENT);
            if(r->first == NULL_ENTITY) continue;
            nextEntity = r->first;
            // FABRIK - Forward Reaching
            if(ikRoot.hasTarget) {
                // Get the last segment in the IK chain
                while(true) {
                    if(!HasComponent(ecs, nextEntity, RELATIONSHIP_COMPONENT)) break;
                    r = GetComponent(ecs, nextEntity, RELATIONSHIP_COMPONENT);
                    entity = nextEntity;
                    if(r->next == NULL_ENTITY) break;
                    nextEntity = r->next;
                }
                assert(HasComponent(ecs, nextEntity, HITBOX_COMPONENT));
                Hitbox* hb = GetComponent(ecs, nextEntity, HITBOX_COMPONENT);
                entity = nextEntity;
                hb->pos = ikRoot.target;
                while(true) {
                    IKNode* node = GetComponent(ecs, nextEntity, IK_NODE_COMPONENT);
                    r = GetComponent(ecs, nextEntity, RELATIONSHIP_COMPONENT);
                    if(r->previous == NULL_ENTITY) break;
                    entity = r->previous;
                    Hitbox* previousHB = GetComponent(ecs, entity, HITBOX_COMPONENT);
                    Vector2 dir = Vector2Normalize(Vector2Subtract(previousHB->pos, hb->pos));
                    previousHB->pos = Vector2Add(hb->pos, Vector2Scale(dir, node->length));
                    // 
                    nextEntity = entity;
                }
            }
            entity = GetEntity(ecs,IK_ROOT_COMPONENT,i); // temp assume its the root
            r = GetComponent(ecs, entity, RELATIONSHIP_COMPONENT);
            nextEntity = r->first;
            // "Forward kinematics" (it's actually backwards kinematics in the FABRIK algo)
            while(true) {
                Hitbox* hb = GetComponent(ecs, entity, HITBOX_COMPONENT);;
                if(!HasComponents(ecs, nextEntity, 3, HITBOX_COMPONENT, IK_NODE_COMPONENT, RELATIONSHIP_COMPONENT)) break;
                Hitbox* nextHB = GetComponent(ecs, nextEntity, HITBOX_COMPONENT);
                IKNode* node = GetComponent(ecs, nextEntity, IK_NODE_COMPONENT);
                r = GetComponent(ecs, nextEntity, RELATIONSHIP_COMPONENT);
                Vector2 dir = Vector2Normalize(Vector2Subtract(nextHB->pos, hb->pos));
                nextHB->pos = Vector2Add(hb->pos, Vector2Scale(dir, node->length));
                entity = nextEntity;
                if(r->next == NULL_ENTITY) break;
                nextEntity = r->next;
            }
        }
    }
}

void IKLegSystem(ECS* ecs, Room* room) {
    for(int i = 0; i < ecs->blocks[IK_LEG_COMPONENT].count; ++i) {
        IKLeg leg = IndexComponent(ecs, IKLeg, IK_LEG_COMPONENT, i);
        Entity e = GetEntity(ecs, IK_LEG_COMPONENT, i);
        if(!HasComponents(ecs, e, 2, HITBOX_COMPONENT, IK_ROOT_COMPONENT)) continue;
        Hitbox *hb = GetComponent(ecs, e, HITBOX_COMPONENT);
        IKRoot* root = GetComponent(ecs, e, IK_ROOT_COMPONENT);
        RayCollision2D raycast = CheckCollisionRayRoom((Vector2){hb->pos.x, hb->pos.y}, (Vector2){0,1}, room);
        if(raycast.hit && fabs(root->target.x - raycast.point.x) > leg.strideLength) {
            float sign = -(root->target.x - raycast.point.x)/fabs(root->target.x - raycast.point.x);
            raycast = CheckCollisionRayRoom((Vector2){hb->pos.x + leg.strideLength*sign, hb->pos.y}, (Vector2){0,1}, room);
            if(!raycast.hit) continue;
            root->target = (Vector2){raycast.point.x, raycast.point.y};
        }
    }
}
