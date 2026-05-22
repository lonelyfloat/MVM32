#ifndef H_ECS
#define H_ECS

// Extremely simple ECS system

#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>

#include "arena.h"

typedef uint32_t Entity; 
// Layout:      ID: 0xFFFFFF00
//         Version: 0x000000FF

// These two types are also interchangable, I'm just pedantic
typedef uint32_t EntityCount; 
typedef uint32_t EntityIndex;

// Helper functions for entities: 
static inline Entity ConstructEntity(EntityIndex id, uint8_t version) {
    return (id << 8) + version;
}
static inline EntityIndex GetID(Entity e) {
    return (e & 0xFFFFFF00) >> 8;
}
static inline uint8_t GetVersion(Entity e) {
    return e & 0x000000FF;
}

// The BAD, EVIL entity ID.
static const Entity INVALID_ID = 0xFFFFFF;

static const Entity NULL_ENTITY = 0xFFFFFF00;

// Sparse set of all the components of a given type
typedef struct ComponentBlock { 
    void* components;
    Entity* entities; // dense array (index -> id)
    Entity* indices; // sparse array (id -> index)
    EntityCount count;
    EntityCount capacity;
    size_t componentSize;
} ComponentBlock;

// Function ptrs to serializers
typedef void (*SaveHook)(FILE*, void*);
typedef void (*LoadHook)(FILE*, Arena*, void*);
typedef void (*AllocateHook)(Arena*, void*);

// Function ptr to UI function
typedef void (*UIHook)(void*);

// ECS data structure holding all component blocks + external info
typedef struct ECS {
    ComponentBlock* blocks;
    Entity* entities;
    EntityCount maxEntities;
    EntityIndex destroyed;
    EntityCount totalEntities;
    EntityCount available;
    // Fptrs and other info per-component
    SaveHook* saveHooks;
    LoadHook* loadHooks;
    AllocateHook* allocHooks;
    UIHook* uiHooks;

    char** componentStrings;
    size_t componentCount;
} ECS;


typedef uint32_t ComponentIndex; // Should only be 0-64 b/c of the bitmask for serialization

ECS* InitECS(Arena* arena, uint32_t maxEntities, size_t componentCount);
Entity CreateEntity(ECS* ecs);
void RemoveComponent(ECS* ecs, Entity entity, ComponentIndex type); 
void* GetComponent(ECS* ecs, Entity entity, ComponentIndex type); // Returns pointer to the component
bool HasComponent(ECS* ecs, Entity entity, ComponentIndex type); 
bool HasComponents(ECS* ecs, Entity entity, int dependencyCount, ...); 
Entity GetEntity(ECS* ecs, ComponentIndex block, EntityIndex blockIndex); // Returns the entity given the index in component iteration 
void KillEntity(ECS* ecs, Entity entity); 

void NukeECS(ECS* ecs); // Kill all entities

void AddEmptyComponent(Arena* arena, ECS* ecs, Entity e, ComponentIndex i);



// Used by macro in "component.h"
void InitComponentBlock(ECS* ecs, Arena* arena, size_t componentSize, ComponentIndex type);

// MACROS

#define IndexComponent(ecs, T, type, ...) (((T*)(ecs)->blocks[(type)].components)[(__VA_ARGS__)])

#endif
