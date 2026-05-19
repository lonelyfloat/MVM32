#ifndef H_RELATIONSHIP
#define H_RELATIONSHIP

#include "ecs.h"

// Code for ECS relationships (parent/child entities), as a component
// Based off of this: https://skypjack.github.io/2019-06-25-ecs-baf-part-4/

#define RELATIONSHIP(X)\
    X(Entity, first)\
    X(Entity, parent)\
    X(Entity, next)\
    X(Entity, previous)

#endif
