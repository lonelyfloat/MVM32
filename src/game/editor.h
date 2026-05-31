#ifndef H_EDITOR
#define H_EDITOR

#include <raylib/raylib.h>
#include "world.h"

void UpdateEditor(World* world, Camera2D* camera);
void DrawEditor(World* world, Camera2D* camera);

#endif
