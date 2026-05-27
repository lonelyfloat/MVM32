#ifndef H_COMPONENT_T
#define H_COMPONENT_T

#include <raylib/raylib.h>
#include "component.h"
#include "components/sprite.h"
#include "components/relationship.h"
#include "raylib_allocators.h"
#include "ui/raylib_ui.h"
#include "ik.h"
#include "player.h"

#define HITBOX(X)\
    X(Vector2, pos)\
    X(Vector2, scale)\

typedef Vector2 Velocity;
#define VELOCITY(X)\
    X(float, x)\
    X(float, y)

typedef Vector2 Offset;
#define OFFSET(X)\
    X(float, x)\
    X(float, y)

typedef Vector2 IKPole;
#define IK_POLE(X)\
    X(float, x)\
    X(float, y)

#define ZORDER(X)\
    X(int, z)

#define HEALTH(X)\
    X(int, health)\
    X(int, maxHealth)\

#define STATIC_GEOMETRY(X)\
    X(bool, enabled)

#define ACTOR(X)\
    X(Vector2,impulse)\
    X(RayCollision2D,raycast)\
    X(bool,autoApply)\

#define DEBUG_SHAPE(X)\
    X(Color, col)

#define COMPONENTS(init)\
    init(Hitbox, HITBOX)\
    init(Velocity, VELOCITY, STRUCT_DEFINED)\
    init(Sprite, SPRITE)\
    init(Health, HEALTH)\
    init(Actor, ACTOR)\
    init(Player, PLAYER, STRUCT_DEFINED)\
    init(DebugShape, DEBUG_SHAPE)\
    init(StaticGeometry, STATIC_GEOMETRY)\
    init(Relationship, RELATIONSHIP)\
    init(IKNode, IK_NODE)\
    init(IKRoot, IK_ROOT)\
    init(IKLeg, IK_LEG)\
    init(Offset, OFFSET, STRUCT_DEFINED)\
    init(ZOrder, ZORDER)\
    init(IKPole, IK_POLE, STRUCT_DEFINED) 

COMPONENT_HEADER(COMPONENTS)

static inline Rectangle HitboxToRect(Hitbox hb) {
    return (Rectangle){hb.pos.x, hb.pos.y, hb.scale.x, hb.scale.y};
}

static inline Hitbox RectToHitbox(Rectangle r) {
    return (Hitbox){.pos = {r.x,r.y}, .scale={r.width,r.height}};
}

#endif
