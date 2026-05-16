#include <raylib/raylib.h>
#include "ecs.h"

#include "serialize.h"

#define RECT(X)\
    X(float, x)\
    X(float, y)\
    X(float, width)\
    X(float, height)\

#define VEC2(X)\
    X(float, x)\
    X(float, y)

SERIALIZE_GEN_PRIM(float, "%f ")
SERIALIZE_GEN_PRIM(int, "%d ")
SERIALIZE_GEN_(Vector2, VEC2)
SERIALIZE_GEN_(Rectangle, RECT)

// Saving and loading ECS from file
#define _FERR(c, ...) do { if(c) { fprintf(stderr, __VA_ARGS__); return; } } while(0)

void LoadEntitiesFromFile(ECS* ecs, Arena* arena, char* filePath) {
    FILE* stream;
    stream = fopen(filePath, "r");
    if(!stream) {
        printf("ERROR: could not open ECS file %s\n", filePath);
        return;
    }
    uint32_t entityCount = 0;
    _FERR(fscanf(stream, "%x\n", &entityCount) != 1, "ERROR: %ld: fscanf failed in ECS file %s\n", ftell(stream), filePath);
    while(!feof(stream)) {
        uint64_t bitmask;
        int mask = 1;
        _FERR(fscanf(stream, "%lx:", &bitmask) != 1, "ERROR: %ld: fscanf failed in ECS file %s\n", ftell(stream), filePath);
        Entity e = CreateEntity(ecs);
        for(int i = 0; i < ecs->componentCount; ++i) {
            if(!(bitmask & mask)) {
                mask = mask << 1;
                continue;
            }
            // Add component {
            ecs->loadHooks[i](stream, arena, ecs->blocks[i].components + ecs->blocks[i].count*ecs->blocks[i].componentSize);
            ecs->blocks[i].entities[ecs->blocks[i].count] = e;
            ecs->blocks[i].indices[GetID(e)] = ecs->blocks[i].count;
            ecs->blocks[i].count += 1;
            // }
            mask = mask << 1;
            _FERR(fscanf(stream, ";") != 0, "ERROR: %ld: fscanf failed in ECS file %s\n", ftell(stream), filePath);
        }
        _FERR(fscanf(stream, "\n") != 0, "ERROR: %ld: fscanf failed in ECS file %s\n", ftell(stream), filePath);
    }
    fclose(stream);
}

void SaveEntitiesToFile(ECS* ecs, char* filePath) {
    FILE* stream;
    stream = fopen(filePath, "w");
    if(!stream) {
        printf("ERROR: could not open ECS file %s\n", filePath);
        return;
    }

    fprintf(stream, "%x\n",ecs->totalEntities);
    for(uint16_t en = 0; en < ecs->totalEntities; ++en) {
        if(en != GetID(ecs->entities[en])) {
            continue;
        }
        Entity e = ecs->entities[en];
        // Generate bitmask
        uint64_t bitmask = 0;
        uint64_t bit = 1;
        for(int i = 0; i < ecs->componentCount; ++i) {
            bitmask += bit*HasComponent(ecs, e, i);
            bit = bit << 1;
        }
        fprintf(stream, "%lx:", bitmask);
        for(int i = 0; i < ecs->componentCount; ++i) {
            if(!HasComponent(ecs, e, i)) continue;
            ecs->saveHooks[i](stream, ecs->blocks[i].components + ecs->blocks[i].indices[GetID(e)]*ecs->blocks[i].componentSize);
            fprintf(stream, ";");
        }
        fprintf(stream, "\n");
    }
    fclose(stream);
}
