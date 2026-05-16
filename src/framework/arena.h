#ifndef H_ARENA
#define H_ARENA

#include <stdlib.h>

typedef struct Arena {
    char* data;
    size_t count;
    size_t capacity;
    size_t startPoint;
} Arena;

Arena* CreateArena(size_t max_bytes);
void DestroyArena(Arena* arena); 

void* ArenaAlloc(Arena* arena, size_t bytes);
void ArenaReset(Arena* arena);

Arena* GrabSection(Arena* arena, size_t bytes);

char* ArenaPushString(Arena* arena, const char* cstr);

#ifdef ARENA_IMPLEMENTATION
#include <stdio.h>
Arena* CreateArena(size_t max_bytes) {
    Arena* arena = malloc(sizeof(Arena));
    arena->count = 0;
    arena->startPoint = 0;
    arena->capacity = max_bytes;
    arena->data = malloc(max_bytes);
    return arena;
}

void DestroyArena(Arena* arena) {
    free(arena->data);
    free(arena);
}

void* ArenaAlloc(Arena* arena, size_t bytes) {
    size_t idx = arena->count;
    arena->count += bytes;
    if(arena->count > arena->capacity) {
        // If there is not room for the allocation, abort
        // TODO: see if there's a better way of handling this (realloc?)
        fprintf(stderr, "ERROR: arena count exceeds capacity\n");
        exit(1);
    }
    return &arena->data[idx];
}

void ArenaReset(Arena* arena) {
    arena->count = 0;
}

Arena* GrabSection(Arena* arena, size_t bytes) {
    Arena* result = ArenaAlloc(arena, sizeof(Arena));
    result->count = 0; 
    result->capacity = bytes;
    result->startPoint = arena->count;
    result->data = ArenaAlloc(arena, bytes);
    return result;
}

void ReleaseSection(Arena* parent, Arena* section) {
    parent->count = section->count;
}

char* ArenaPushString(Arena* arena, const char* cstr) {
    int i = 0;
    while(cstr[i] != '\0') {
        i++;
    }
    i+= 1;
    char* ptr = ArenaAlloc(arena, i);
    i = 0;
    while(cstr[i] != '\0') {
        ptr[i] = cstr[i];
        i++;
    }
    ptr[i] = '\0';
    return ptr;
}
#endif

#endif
