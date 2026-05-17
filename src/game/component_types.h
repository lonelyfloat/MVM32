#ifndef H_COMPONENT_T
#define H_COMPONENT_T

#include <raylib/raylib.h>
#include "component.h"
#include "components/sprite.h"
#include "raylib_allocators.h"
#include "ui/raylib_ui.h"
#include "player.h"

typedef Rectangle Hitbox;
#define HITBOX(X)\
    X(float, x)\
    X(float, y)\
    X(float, width)\
    X(float, height)\

typedef Vector2 Velocity;
#define VELOCITY(X)\
    X(float, x)\
    X(float, y)

#define HEALTH(X)\
    X(int, health)\
    X(int, maxHealth)\

#define STATIC_GEOMETRY(X)\
    X(bool, enabled)

#define ACTOR(X)\
    X(Vector2,impulse)\

#define DEBUG_SHAPE(X)\
    X(Color, col)

#define COMPONENTS(init)\
    init(Hitbox, HITBOX, STRUCT_DEFINED)\
    init(Velocity, VELOCITY, STRUCT_DEFINED)\
    init(Sprite, SPRITE)\
    init(Health, HEALTH)\
    init(Actor, ACTOR)\
    init(Player, PLAYER, STRUCT_DEFINED)\
    init(DebugShape, DEBUG_SHAPE)\
    init(StaticGeometry, STATIC_GEOMETRY)\

COMPONENT_HEADER(COMPONENTS)

#endif
