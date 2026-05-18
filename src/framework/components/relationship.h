#ifndef H_RELATIONSHIP
#define H_RELATIONSHIP

#include "ecs.h"

// Code for ECS relationships (parent/child entities), as a component
// Based off of this: https://skypjack.github.io/2019-06-25-ecs-baf-part-4/

/* Nominal struct:
typedef struct Relationship {
    Entity first; // first child of this entity, if present
    Entity parent; // Parent of this entity, if present
    Entity next; // If this is a child, the next sibling in the list
    Entity previous; // If this is a child, the previous sibling in the list
} Relationship;
*/

#define RELATIONSHIP(X)\
    X(Entity, first)\
    X(Entity, parent)\
    X(Entity, next)\
    X(Entity, previous)

#endif
