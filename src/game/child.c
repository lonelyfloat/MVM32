#include "child.h"
#include <raylib/raymath.h>

/*

Entity child1
-> (Rectangle){50,50,0,0}
-> child child2
Entity child2
-> (Rectangle){100,100,0,0}
-> (Vector2){100,100}

*/

Vector2 WorldToEntitySpace(ECS* ecs, Entity e,Vector2 pt) {
    assert(HasComponent(ecs, e, HITBOX_COMPONENT));
    Hitbox worldHB = GetWorldHitbox(ecs, e);
    return Vector2Subtract(pt, worldHB.pos);
}

Hitbox GetWorldHitbox(ECS* ecs, Entity e) {
    assert(HasComponent(ecs, e, HITBOX_COMPONENT));
    Hitbox* hitbox = GetComponent(ecs, e, HITBOX_COMPONENT);
    Hitbox result = *hitbox;
    if(!HasComponent(ecs, e, RELATIONSHIP_COMPONENT)) return result;
    Entity current = e;
    while(true) {
        Relationship* r = GetComponent(ecs, current, RELATIONSHIP_COMPONENT);
        Entity parent = r->parent;
        if(parent == NULL_ENTITY) break;
        if(!HasComponent(ecs, parent, HITBOX_COMPONENT)) return result;
        Hitbox* parentBox = GetComponent(ecs, parent, HITBOX_COMPONENT);
        result.pos = Vector2Add(result.pos, parentBox->pos);
        current = parent;
        
    }
    return result;
}
