#include "ik.h"
#include <raylib/raymath.h>
#include "component_types.h"
#include "child.h"
#include "utils.h"

void FABRIKSystem(ECS* ecs, int iterations) {
    for(int i = 0; i < ecs->blocks[FABRIK_ROOT_COMPONENT].count; ++i) {
        Entity e = GetEntity(ecs, FABRIK_ROOT_COMPONENT, i);
        if(!HasComponents(ecs, e, 2, RELATIONSHIP_COMPONENT, HITBOX_COMPONENT)) {
            continue;
        }
        FabrikRoot origin = IndexComponent(ecs, FabrikRoot, FABRIK_ROOT_COMPONENT, i);
        Relationship* r = GetComponent(ecs, e, RELATIONSHIP_COMPONENT);
        Hitbox* hb = GetComponent(ecs, e, HITBOX_COMPONENT);
        Vector2 originPos = hb->pos;
        Vector2 targetRelative = WorldToEntitySpace(ecs, e, origin.target);
        printf("%f, %f\n", targetRelative.x, targetRelative.y);
        for(int k = 0; k < iterations; ++k) {
            Entity currentNode = e;
            Entity nextSegment = r->first;
            // Edge case: cannot reach even in straight line
            if(Vector2DistanceSqr(hb->pos, targetRelative) > origin.lengthSum*origin.lengthSum) {
                Vector2 dir = Vector2Normalize(Vector2Subtract(targetRelative, hb->pos));
                while(nextSegment != NULL_ENTITY) {
                    if(!HasComponents(ecs, nextSegment, 3, HITBOX_COMPONENT, FABRIK_NODE_COMPONENT, RELATIONSHIP_COMPONENT)) break;
                    Hitbox* hb = GetComponent(ecs, currentNode, HITBOX_COMPONENT);
                    FabrikNode* node = GetComponent(ecs, nextSegment, FABRIK_NODE_COMPONENT);
                    Relationship* r = GetComponent(ecs, nextSegment, RELATIONSHIP_COMPONENT);
                    hb->pos = Vector2Add(hb->pos, Vector2Scale(dir, node->length));
                    currentNode = nextSegment;
                    nextSegment = r->next;
                }
            }
            // Forwards reaching
            // Get the last point in the IK chain
            while(currentNode != NULL_ENTITY) {
                if(!HasComponent(ecs, currentNode, RELATIONSHIP_COMPONENT)) break;
                Relationship* r = GetComponent(ecs, currentNode, RELATIONSHIP_COMPONENT);
                if(r->next == NULL_ENTITY) break;
                currentNode = r->next;
            }
            // Now, currentNode is the last point
            hb = GetComponent(ecs, currentNode, HITBOX_COMPONENT);
            hb->pos = targetRelative;
            Entity previousNode = currentNode;
            // Traverse backwards through the list
            while(currentNode != NULL_ENTITY) {
                if(!HasComponents(ecs, currentNode, 3, HITBOX_COMPONENT, FABRIK_NODE_COMPONENT, RELATIONSHIP_COMPONENT)) break;
                r = GetComponent(ecs, currentNode, RELATIONSHIP_COMPONENT);
                FabrikNode* node = GetComponent(ecs, currentNode, FABRIK_NODE_COMPONENT);
                previousNode = r->previous;
                if(previousNode == NULL_ENTITY) break;
                Hitbox* previousHB = GetComponent(ecs, previousNode, HITBOX_COMPONENT);
                hb = GetComponent(ecs, currentNode, HITBOX_COMPONENT);
                Vector2 dir = Vector2Normalize(Vector2Subtract(previousHB->pos, hb->pos));
                previousHB->pos = Vector2Add(hb->pos, Vector2Scale(dir, node->length));
                currentNode = previousNode;
            }

            // Backwards reaching
            currentNode = e; // Set currentNode to the root
            hb = GetComponent(ecs, currentNode, HITBOX_COMPONENT);
            Entity nextNode = currentNode;
            hb->pos = originPos;
            while(currentNode != NULL_ENTITY) {
                hb = GetComponent(ecs, currentNode, HITBOX_COMPONENT);
                r = GetComponent(ecs, currentNode, RELATIONSHIP_COMPONENT);
                if(r->next == NULL_ENTITY) break;
                nextNode = r->next;
                Hitbox* nextHB = GetComponent(ecs, nextNode, HITBOX_COMPONENT);
                FabrikNode* node = GetComponent(ecs, nextNode, FABRIK_NODE_COMPONENT);
                Vector2 dir = Vector2Normalize(Vector2Subtract(nextHB->pos, hb->pos));
                nextHB->pos = Vector2Add(hb->pos, Vector2Scale(dir, node->length));
                currentNode = nextNode;
                if(!HasComponents(ecs, currentNode, 3, HITBOX_COMPONENT, FABRIK_NODE_COMPONENT, RELATIONSHIP_COMPONENT)) break;
            }
        }
    }
}
