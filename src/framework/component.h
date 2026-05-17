#ifndef H_COMPONENT 
#define H_COMPONENT

#include "ecs.h"
#include <stdio.h>
#include "serialize.h"
#include "ui/ui_types.h"
#include "ui/ui.h"
#include "component_memory.h"

// NOTE: THIS IS THE MACRO TO USE! 
#define COMPONENT_HEADER(table) \
typedef enum ComponentType { table(_ENUM_TOOL) COMPONENT_COUNT } ComponentType; \
void RegisterComponents(ECS* ecs, Arena* arena);\
table(_GEN_HEADERS)

#define COMPONENT_IMPLEMENTATION(table)\
table(_GEN_COMP_IMPL)\
void RegisterComponents(ECS* ecs, Arena* arena) {\
    table(_INIT_TOOL)\
    ecs->componentStrings = ArenaAlloc(arena, ecs->componentCount*sizeof(char*));\
    table(_STR_TOOL)\
}\

// Internals for GEN_COMPONENT_TABLE


#define _GEN_HEADERS(t,v,...) _GEN_HEADERS_ ## __VA_ARGS__ (t,v)

#define _GEN_COMP_IMPL(t,v,...) _GEN_COMP_IMPL_ ## __VA_ARGS__ (t,v)

#define _MAKE_STRUCT(type, name, ...) type name;
#define _COMPONENT_NAME(v) v##_COMPONENT
#define _TAG_NAME(v) v##_TAG
#define _ENUM_TOOL(T,v,...) _COMPONENT_NAME(v),
#define _ENUM_TAG_TOOL(v) _TAG_NAME(v),

#define _MAKE_TAGGED_UNION_ENUM(type,name,enumname,...) enumname,
#define _GEN_HEADERS_TAGGED_UNION(T,v)\
typedef enum T##Type {\
     v(_MAKE_TAGGED_UNION_ENUM)\
     v##_TYPE_COUNT\
} T##Type;\
typedef struct T {\
    union {\
        v(_MAKE_STRUCT)\
    } data;\
    T##Type type;\
} T;\
void Add##T(ECS* ecs, Entity entity, T value);\
SERIALIZE_GEN_HEADER(T)\
UI_GEN_COMPONENT_HEADER(T)\
_ALLOCATOR_HEADER(T)\

#define _GEN_COMP_IMPL_TAGGED_UNION(T,v)\
void Add##T(ECS* ecs, Entity entity, T value) { \
    ((T*)(ecs->blocks[(_COMPONENT_NAME(v))].components))[ecs->blocks[(_COMPONENT_NAME(v))].count] = value;\
    ecs->blocks[(_COMPONENT_NAME(v))].entities[ecs->blocks[(_COMPONENT_NAME(v))].count] = entity;\
    ecs->blocks[(_COMPONENT_NAME(v))].indices[GetID(entity)] = ecs->blocks[(_COMPONENT_NAME(v))].count;\
    ecs->blocks[(_COMPONENT_NAME(v))].count += 1;\
}\
SERIALIZE_GEN_TAGGED_UNION(T,v)\
UI_GEN_TAGGED_UNION(T,v)\
_GEN_COMPONENT_ALLOCATOR(T,v)

#define _GEN_HEADERS_STRUCT_DEFINED(T,v)\
void Add##T(ECS* ecs, Entity entity, T value);\
SERIALIZE_GEN_HEADER(T)\
UI_GEN_COMPONENT_HEADER(T)\

#define _GEN_HEADERS_(T,v)\
typedef struct T {\
    v(_MAKE_STRUCT)\
} T;\
void Add##T(ECS* ecs, Entity entity, T value);\
SERIALIZE_GEN_HEADER(T)\
UI_GEN_COMPONENT_HEADER(T)\

#define _GEN_COMP_IMPL_(T,v)\
void Add##T(ECS* ecs, Entity entity, T value) { \
    ((T*)(ecs->blocks[(_COMPONENT_NAME(v))].components))[ecs->blocks[(_COMPONENT_NAME(v))].count] = value;\
    ecs->blocks[(_COMPONENT_NAME(v))].entities[ecs->blocks[(_COMPONENT_NAME(v))].count] = entity;\
    ecs->blocks[(_COMPONENT_NAME(v))].indices[GetID(entity)] = ecs->blocks[(_COMPONENT_NAME(v))].count;\
    ecs->blocks[(_COMPONENT_NAME(v))].count += 1;\
}\
SERIALIZE_GEN_(T,v)\
UI_GEN_(T,v)\
_GEN_COMPONENT_ALLOCATOR(T,v)\

#define _GEN_COMP_IMPL_STRUCT_DEFINED(T,v)\
void Add##T(ECS* ecs, Entity entity, T value) { \
    ((T*)(ecs->blocks[(_COMPONENT_NAME(v))].components))[ecs->blocks[(_COMPONENT_NAME(v))].count] = value;\
    ecs->blocks[(_COMPONENT_NAME(v))].entities[ecs->blocks[(_COMPONENT_NAME(v))].count] = entity;\
    ecs->blocks[(_COMPONENT_NAME(v))].indices[GetID(entity)] = ecs->blocks[(_COMPONENT_NAME(v))].count;\
    ecs->blocks[(_COMPONENT_NAME(v))].count += 1;\
}\
SERIALIZE_GEN_(T,v)\
UI_GEN_(T,v)\
_GEN_COMPONENT_ALLOCATOR(T,v)\


#define _ADD_TOOL(T, v,...) static inline void Add##T(ECS* ecs, Entity entity, T value) { \
    ((T*)(ecs->blocks[(_COMPONENT_NAME(v))].components))[ecs->blocks[(_COMPONENT_NAME(v))].count] = value; \
    ecs->blocks[(_COMPONENT_NAME(v))].entities[ecs->blocks[(_COMPONENT_NAME(v))].count] = entity; \
    ecs->blocks[(_COMPONENT_NAME(v))].indices[GetID(entity)] = ecs->blocks[(_COMPONENT_NAME(v))].count; \
    ecs->blocks[(_COMPONENT_NAME(v))].count += 1; \
} 

#define _INIT_TOOL(T, v,...) \
    InitComponentBlock(ecs, arena, sizeof(T), _COMPONENT_NAME(v));\
    ecs->saveHooks[_COMPONENT_NAME(v)] = _Save_##T;\
    ecs->loadHooks[_COMPONENT_NAME(v)] = _Load_##T;\
    ecs->uiHooks[_COMPONENT_NAME(v)] = _UI_Component_##T;\
    ecs->allocHooks[_COMPONENT_NAME(v)] = _Allocate_##T;

#define _STR_TOOL(T,v,...) ecs->componentStrings[_COMPONENT_NAME(v)] = ArenaPushString(arena, #T);

// void AddComponent(ECS* ecs, Entity entity, ComponentType comp, ...);

#endif
