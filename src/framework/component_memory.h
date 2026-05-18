#ifndef H_COMP_MEMORY
#define H_COMP_MEMORY

#include "ecs.h"

// Macro helpers for allocating newly-added / created components w/ Arenas

#define _ALLOCATOR_FIELD(type, name, ...) _Allocate_##type(arena, &component->name);

#define _GEN_COMPONENT_ALLOCATOR(T,X)\
void _Allocate_##T(Arena* arena, void* value) {\
    T* component = value;\
    X(_ALLOCATOR_FIELD)\
}

#define _ALLOCATOR_HEADER(type, ...) void _Allocate_##type(Arena*, void*);

#define _PRIM_ALLOCATOR(type, ...)\
void _Allocate_##type(Arena* arena, void* value) {\
    char* buf = value;\
    for(int b = 0; b < sizeof(type);++b) {\
        buf[b] = 0;\
    }\
}\

_ALLOCATOR_HEADER(int)
_ALLOCATOR_HEADER(bool)
_ALLOCATOR_HEADER(float)
_ALLOCATOR_HEADER(Entity)

#endif
