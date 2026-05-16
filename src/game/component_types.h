#ifndef H_COMPONENT_T
#define H_COMPONENT_T

#include <raylib/raylib.h>
#include "component.h"
#include "components/sprite.h"
#include "raylib_allocators.h"
#include "ui/raylib_ui.h".h"

#define HITBOX(X)\
    X(Rectangle,rec)

#define COMPONENTS(init)\
    init(Hitbox, HITBOX)\
    init(Sprite, SPRITE)

COMPONENT_HEADER(COMPONENTS)

#endif
