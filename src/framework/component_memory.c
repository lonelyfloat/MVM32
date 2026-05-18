#include "component_memory.h"

_PRIM_ALLOCATOR(float)
_PRIM_ALLOCATOR(bool)
_PRIM_ALLOCATOR(int)

void _Allocate_Entity(Arena *arena, void *value) {
  Entity *e = value;
  *e = NULL_ENTITY;
}
