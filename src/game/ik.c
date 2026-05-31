#include "ik.h"
#include <raylib/raymath.h>
#include "world.h"
#include "raycast.h"
#include "utils.h"

// void ResolveIK(ECS* ecs, int iterations) {
//     for(int i = 0; i < ecs->blocks[IK_ROOT_COMPONENT].count; ++i) {
//         Entity entity = GetEntity(ecs,IK_ROOT_COMPONENT,i); // temp assume its the root
//         if(!HasComponents(ecs, entity, 2, RELATIONSHIP_COMPONENT, HITBOX_COMPONENT)) continue;
//         IKRoot ikRoot = IndexComponent(ecs, IKRoot, IK_ROOT_COMPONENT, i);
//         ikRoot.target = GetMousePosition();
//         Relationship* r = GetComponent(ecs, entity, RELATIONSHIP_COMPONENT);
//         if(r->first == NULL_ENTITY) continue;
//         Entity nextEntity = r->first;
//         for(int k = 0; k < iterations; ++k) {
//             entity = GetEntity(ecs,IK_ROOT_COMPONENT,i); // temp assume its the root
//             ikRoot = IndexComponent(ecs, IKRoot, IK_ROOT_COMPONENT, i);
//             r = GetComponent(ecs, entity, RELATIONSHIP_COMPONENT);
//             if(r->first == NULL_ENTITY) continue;
//             nextEntity = r->first;
//             // FABRIK - Forward Reaching
//             if(ikRoot.hasTarget) {
//                 // Get the last segment in the IK chain
//                 while(true) {
//                     if(!HasComponent(ecs, nextEntity, RELATIONSHIP_COMPONENT)) break;
//                     r = GetComponent(ecs, nextEntity, RELATIONSHIP_COMPONENT);
//                     entity = nextEntity;
//                     if(r->next == NULL_ENTITY) break;
//                     nextEntity = r->next;
//                 }
//                 assert(HasComponent(ecs, nextEntity, HITBOX_COMPONENT));
//                 Hitbox* hb = GetComponent(ecs, nextEntity, HITBOX_COMPONENT);
//                 entity = nextEntity;
//                 hb->pos = ikRoot.target;
//                 while(true) {
//                     IKNode* node = GetComponent(ecs, nextEntity, IK_NODE_COMPONENT);
//                     r = GetComponent(ecs, nextEntity, RELATIONSHIP_COMPONENT);
//                     if(r->previous == NULL_ENTITY) break;
//                     entity = r->previous;
//                     Hitbox* previousHB = GetComponent(ecs, entity, HITBOX_COMPONENT);
//                     Vector2 dir = Vector2Normalize(Vector2Subtract(previousHB->pos, hb->pos));
//                     previousHB->pos = Vector2Add(hb->pos, Vector2Scale(dir, node->length));
//                     // 
//                     nextEntity = entity;
//                 }
//             }
//             entity = GetEntity(ecs,IK_ROOT_COMPONENT,i); // temp assume its the root
//             r = GetComponent(ecs, entity, RELATIONSHIP_COMPONENT);
//             nextEntity = r->first;
//             // "Forward kinematics" (it's actually backwards kinematics in the FABRIK algo)
//             while(true) {
//                 Hitbox* hb = GetComponent(ecs, entity, HITBOX_COMPONENT);;
//                 if(!HasComponents(ecs, nextEntity, 3, HITBOX_COMPONENT, IK_NODE_COMPONENT, RELATIONSHIP_COMPONENT)) break;
//                 Hitbox* nextHB = GetComponent(ecs, nextEntity, HITBOX_COMPONENT);
//                 IKNode* node = GetComponent(ecs, nextEntity, IK_NODE_COMPONENT);
//                 r = GetComponent(ecs, nextEntity, RELATIONSHIP_COMPONENT);
//                 Vector2 dir = Vector2Normalize(Vector2Subtract(nextHB->pos, hb->pos));
//                 nextHB->pos = Vector2Add(hb->pos, Vector2Scale(dir, node->length));
//                 entity = nextEntity;
//                 if(r->next == NULL_ENTITY) break;
//                 nextEntity = r->next;
//             }
//         }
//     }
// }
// 
// bool GroundedLeg(ECS* ecs, Entity e, Room* room) {
//     if(!HasComponents(ecs, e, 4, HITBOX_COMPONENT, RELATIONSHIP_COMPONENT, IK_ROOT_COMPONENT, IK_POLE_COMPONENT)) return false;
//     IKRoot* root = GetComponent(ecs, e, IK_ROOT_COMPONENT);
//     Relationship* r = GetComponent(ecs, e, RELATIONSHIP_COMPONENT);
//     Entity end = r->first;
//     while(true){
//         Relationship* r = GetComponent(ecs, end, RELATIONSHIP_COMPONENT);
//         if(r->next == NULL_ENTITY) break;
//         end = r->next;
//     }
//     Hitbox* endHB = GetComponent(ecs, end, HITBOX_COMPONENT); 
//     if(CheckCollisionPointRoom(endHB->pos, room)) {
//         printf("%x g\n", e);
//         return true;
//     }
//     return false;
// }
// 
// int GetGroundedLegs(ECS* ecs, Entity e, Room* room) {
//     int grounded = 0;
//     Entity current = e;
//     while(true) {
//         Relationship* r = GetComponent(ecs, current, RELATIONSHIP_COMPONENT);
//         if(r->previous == NULL_ENTITY) break;
//         current = r->previous;
//     }
//     // current is now the head of the list
//     while(true) {
//         if(current != e && GroundedLeg(ecs, current, room)) grounded++;
//         Relationship* r = GetComponent(ecs, current, RELATIONSHIP_COMPONENT);
//         if(r->next == NULL_ENTITY) break;
//         current = r->next;
//         if(!HasComponents(ecs, current, 3, IK_LEG_COMPONENT, IK_ROOT_COMPONENT, HITBOX_COMPONENT)) continue;
//     }
//     return grounded;
// }
// 
// void IKLegSystem(ECS* ecs, Room* room) {
//     for(int i = 0; i < ecs->blocks[IK_LEG_COMPONENT].count; ++i) {
//         IKLeg leg = IndexComponent(ecs, IKLeg, IK_LEG_COMPONENT, i);
//         Entity e = GetEntity(ecs, IK_LEG_COMPONENT, i);
//         if(!HasComponents(ecs, e, 4, HITBOX_COMPONENT, RELATIONSHIP_COMPONENT, IK_ROOT_COMPONENT, IK_POLE_COMPONENT)) continue;
//         Hitbox *hb = GetComponent(ecs, e, HITBOX_COMPONENT);
//         IKRoot* root = GetComponent(ecs, e, IK_ROOT_COMPONENT);
//         IKPole* p = GetComponent(ecs, e, IK_POLE_COMPONENT);
//         float sign = p->x;
//         RayCollision2D raycast = CheckCollisionRayRoom((Vector2){hb->pos.x + sign*leg.xOffset, hb->pos.y}, (Vector2){0,1}, room);
//         if(raycast.hit) {
//             Vector2 targetPt = Vector2Add(hb->pos, Vector2Scale((Vector2){raycast.normal.y, -raycast.normal.x}, leg.xOffset*(-sign)));
//             raycast = CheckCollisionRayRoom(targetPt, Vector2Negate(raycast.normal), room);
//             int n = GetGroundedLegs(ecs, e, room);
//             if(raycast.hit && Vector2Distance(raycast.point,root->target) > leg.strideLength && n > 0) {
//                 root->target = (Vector2){raycast.point.x, raycast.point.y};
//                 break;
//             }
//         }
//     }
// }

