#include "component_types.h"

void SaveEntityTree_(ECS* ecs, Entity e, const char* filePath, Entity* buffer, int* idx) {
    Entity current = e;
    // Exit cases (pop the stack)
    if(!HasComponent(ecs, e, RELATIONSHIP_COMPONENT)) return;
    Relationship* r = GetComponent(ecs, e, RELATIONSHIP_COMPONENT);
    if(r->first == NULL_ENTITY) { 
        buffer[*idx] = e;
        *idx += 1;
        return;
    }
    // Traverse the linked list
    current = r->first;
    while(current != NULL_ENTITY) {
        // Exit case (pop the stack)
        if(!HasComponent(ecs, current, RELATIONSHIP_COMPONENT)) return;
        r = GetComponent(ecs, current, RELATIONSHIP_COMPONENT);
        // Push the stack
        SaveEntityTree_(ecs, current, filePath, buffer, idx);
        current = r->next;
    }
    buffer[*idx] = e;
    *idx += 1;
}

// Saving and loading ECS from file
#define _FERR(c, ...) do { if(c) { fprintf(stderr, __VA_ARGS__); return; } } while(0)

void SaveEntityTree(ECS* ecs, Entity e, const char* filePath) {
    int count = 0;
    Entity stack[ecs->maxEntities] = {};
    EntityIndex sparse[ecs->maxEntities] = {};
    for(int i = 0; i < ecs->maxEntities; ++i) {
        sparse[i] = NULL_ENTITY;
    }
    SaveEntityTree_(ecs, e, filePath, stack, &count);
    for(int i = 0; i < count; ++i) {
        sparse[GetID(stack[i])] = i;
    }

    FILE* stream = fopen(filePath, "w");
    if(!stream) {
        printf("ERROR: could not open ECS file %s\n", filePath);
        return;
    }

    fprintf(stream, "%x\n",count);
    for(uint16_t en = 0; en < count; ++en) {
        Entity e = stack[en];
        // Generate bitmask
        uint64_t bitmask = 0;
        uint64_t bit = 1;
        for(int i = 0; i < ecs->componentCount; ++i) {
            bitmask += bit*HasComponent(ecs, e, i);
            bit = bit << 1;
        }
        fprintf(stream, "%lx:", bitmask);
        Relationship rOld;
        Relationship* r = NULL;
        for(int i = 0; i < ecs->componentCount; ++i) {
            if(!HasComponent(ecs, e, i)) continue;
            if(i == RELATIONSHIP_COMPONENT) {
                r = GetComponent(ecs, e, RELATIONSHIP_COMPONENT);
                rOld = *r;
                if(r->first != NULL_ENTITY) {
                    Entity newidx = sparse[GetID(r->first)];
                    if(sparse[GetID(r->first)] != NULL_ENTITY) {
                        r->first = newidx;
                    }
                }
                if(r->next != NULL_ENTITY) {
                    Entity newidx = sparse[GetID(r->next)];
                    if(sparse[GetID(r->next)] != NULL_ENTITY) {
                        r->next = newidx;
                    }
                }
                if(r->previous != NULL_ENTITY) {
                    Entity newidx = sparse[GetID(r->previous)];
                    if(sparse[GetID(r->previous)] != NULL_ENTITY) {
                        r->previous = newidx;
                    }
                }
                if(r->parent != NULL_ENTITY) {
                    Entity newidx = sparse[GetID(r->parent)];
                    if(sparse[GetID(r->parent)] != NULL_ENTITY) {
                        r->parent = newidx;
                    }
                }
            }
            ecs->saveHooks[i](stream, ecs->blocks[i].components + ecs->blocks[i].indices[GetID(e)]*ecs->blocks[i].componentSize);
            if(i == RELATIONSHIP_COMPONENT) {
                *r = rOld;
            }
            fprintf(stream, ";");
        }
        fprintf(stream, "\n");
    }
    fclose(stream);
}

void LoadEntityTree(ECS* ecs, Arena* arena, const char* filePath) {
    FILE* stream = fopen(filePath, "r");
    if(!stream) {
        printf("ERROR: could not open ECS file %s\n", filePath);
        return;
    }
    uint32_t entityCount = 0;
    uint32_t entities = 0;
    _FERR(fscanf(stream, "%x\n", &entityCount) != 1, "ERROR: %ld: fscanf failed in ECS file %s\n", ftell(stream), filePath);
    Entity stack[entityCount] = {};
    for(int i = 0; i < entityCount; ++i) {
        stack[i] = CreateEntity(ecs);
    }
    while(!feof(stream)) {
        uint64_t bitmask = 0;
        int mask = 1;
        _FERR(fscanf(stream, "%lx:", &bitmask) != 1, "ERROR: %ld: fscanf failed in ECS file %s\n", ftell(stream), filePath);
        Entity e = stack[entities];
        entities++;
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
            if(i == RELATIONSHIP_COMPONENT) {
                Relationship* r = GetComponent(ecs, e, RELATIONSHIP_COMPONENT); 
                if(r->first != NULL_ENTITY) {
                    r->first = stack[r->first];
                }
                if(r->parent != NULL_ENTITY) {
                    r->parent = stack[r->parent];
                }
                if(r->next != NULL_ENTITY) {
                    r->next = stack[r->next];
                }
                if(r->previous != NULL_ENTITY) {
                    r->previous = stack[r->previous];
                }
            }
            // }
            mask = mask << 1;
            _FERR(fscanf(stream, ";") != 0, "ERROR: %ld: fscanf failed in ECS file %s\n", ftell(stream), filePath);
        }
        _FERR(fscanf(stream, "\n") != 0, "ERROR: %ld: fscanf failed in ECS file %s\n", ftell(stream), filePath);
    }
    fclose(stream);
}
