#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdarg.h>
#include <stdlib.h>
#include "ecs.h"

// COMPONENT BLOCK FUNCTIONS (ALL LIBRARY-INTERNAL!)

void InitComponentBlock(ECS* ecs, Arena* arena, size_t componentSize, ComponentIndex type) { 
    ecs->blocks[type].capacity = ecs->maxEntities; 
    ecs->blocks[type].count = 0; 
    ecs->blocks[type].componentSize = componentSize; 
    ecs->blocks[type].indices = ArenaAlloc(arena, ecs->maxEntities * sizeof(Entity)); 
    ecs->blocks[type].entities = ArenaAlloc(arena,ecs->maxEntities * sizeof(Entity)); 
    ecs->blocks[type].components = ArenaAlloc(arena, ecs->maxEntities * componentSize); 
}

void ComponentBlockRemove(ComponentBlock* block, Entity entity) {
    block->count--;
    Entity deletedIdx = block->indices[GetID(entity)];
    void* last = (block->components + ((block->count) * (block->componentSize)));
    void* dest = (block->components + (deletedIdx * (block->componentSize)));
    memcpy(dest, last, block->componentSize);
    Entity lastE = block->entities[block->count];
    block->entities[deletedIdx] = lastE;
    block->indices[GetID(lastE)] = deletedIdx;
}

bool ComponentBlockHasEntity(ComponentBlock* block, Entity entity) {
    EntityIndex index = block->indices[GetID(entity)];
    return block->indices[GetID(entity)] < block->count && block->entities[index] == entity;
}

// ECS FUNCTIONS: 

ECS* InitECS(Arena* arena, uint32_t maxEntities, size_t componentCount) {
    ECS* ecs = ArenaAlloc(arena, sizeof(ECS));
    ecs->maxEntities = maxEntities;
    ecs->componentCount = componentCount; 
    ecs->destroyed = INVALID_ID;
    ecs->entities = ArenaAlloc(arena, maxEntities * sizeof(Entity));
    ecs->totalEntities = 0;
    ecs->available = 0;
    ecs->blocks = ArenaAlloc(arena, componentCount * sizeof(ComponentBlock));
    ecs->loadHooks = ArenaAlloc(arena, componentCount * sizeof(LoadHook));
    ecs->allocHooks = ArenaAlloc(arena, componentCount * sizeof(AllocateHook));
    ecs->saveHooks = ArenaAlloc(arena, componentCount * sizeof(SaveHook));
    ecs->uiHooks = ArenaAlloc(arena, componentCount * sizeof(UIHook));
    return ecs;
}

Entity CreateEntity(ECS* ecs) {
    if(ecs->available != 0) {
        Entity dest = ecs->destroyed;
        ecs->destroyed = GetID(ecs->entities[dest]);
        ecs->entities[dest] = ConstructEntity(dest, GetVersion(ecs->entities[dest]));
        ecs->available--;
        return ecs->entities[dest];
    }
    uint32_t id = ecs->totalEntities;
    ecs->entities[id] = ConstructEntity(id, 0);
    ecs->totalEntities++;
    return ecs->entities[id];
}

void RemoveComponent(ECS* ecs, Entity entity, ComponentIndex type) {
    ComponentBlockRemove(&ecs->blocks[type], entity);
}

void* GetComponent(ECS* ecs, Entity entity, ComponentIndex type) {
    EntityIndex idx = ecs->blocks[type].indices[GetID(entity)];
    size_t compS = ecs->blocks[type].componentSize;
    char* result = (ecs->blocks[type].components + (idx * compS));
    return result;
}

bool HasComponent(ECS* ecs, Entity entity, ComponentIndex type) {
    return ComponentBlockHasEntity(&ecs->blocks[type], entity);
}

bool HasComponents(ECS* ecs, Entity entity, int dependencyCount, ...) {
    va_list args;
    va_start(args, dependencyCount);
    for(int i = 0; i < dependencyCount; ++i) {
        if(!HasComponent(ecs, entity, va_arg(args, unsigned int))) {
            va_end(args);
            return false;
        }
    }
    va_end(args);
    return true;
}

Entity GetEntity(ECS* ecs, ComponentIndex block, EntityIndex blockIndex) {
    EntityIndex id = GetID(ecs->blocks[block].entities[blockIndex]);
    return ecs->entities[id];
}

void KillEntity(ECS* ecs, Entity entity) {
    if(GetID(entity) == INVALID_ID) return;

    // Remove all of the components.
    for(int i = 0; i < ecs->componentCount; ++i) {
        if(HasComponent(ecs, entity, i)) {
            RemoveComponent(ecs, entity, i);
        }
    }

    EntityIndex oldID = GetID(entity);
    ecs->entities[oldID] = ConstructEntity(ecs->destroyed, GetVersion(entity) + 1);
    ecs->destroyed = oldID;
    ecs->available++;
}


void AddEmptyComponent(Arena* arena, ECS* ecs, Entity e, ComponentIndex i) {
    ecs->allocHooks[i](arena,ecs->blocks[i].components + ecs->blocks[i].count*ecs->blocks[i].componentSize);
    ecs->blocks[i].entities[ecs->blocks[i].count] = e;
    ecs->blocks[i].indices[GetID(e)] = ecs->blocks[i].count;
    ecs->blocks[i].count += 1;
}

EntityCount GetEntityAmount(ECS* ecs) {
    if(ecs->destroyed == INVALID_ID) {
        return ecs->totalEntities;
    } else {
        return ecs->totalEntities - 1;
    }
}


void NukeECS(ECS* ecs) {
    ecs->destroyed = INVALID_ID;
    ecs->totalEntities = 0;
    ecs->available = 0;
    for(int i = 0; i < ecs->componentCount; ++i) {
        ecs->blocks[i].count = 0; 
    }
}
