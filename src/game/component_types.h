#ifndef H_COMPONENT_T
#define H_COMPONENT_T

#include <raylib/raylib.h>
#include "component.h"
#include "components/sprite.h"


#define POSITION(X)\
    X(float,x)\
    X(float,y)

#define COMPONENTS(init)\
    init(Position, POSITION)\
    init(Sprite, SPRITE)

COMPONENT_HEADER(COMPONENTS)

#endif
