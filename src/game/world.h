#ifndef H_ROOM
#define H_ROOM

#include <ecs.h>
#include <raylib/raylib.h>
#include "raycast.h"

typedef struct Room {
    Rectangle bounds;
    bool active;
} Room;

typedef enum Tile {
    TILE_MS_0 = 0,
    TILE_MS_1,
    TILE_MS_2,
    TILE_MS_3,
    TILE_MS_4,
    TILE_MS_5,
    TILE_MS_6,
    TILE_MS_7,
    TILE_MS_8,
    TILE_MS_9,
    TILE_MS_10,
    TILE_MS_11,
    TILE_MS_12,
    TILE_MS_13,
    TILE_MS_14,
    TILE_MS_15,
    TILE_TYPE_COUNT
} Tile;

// For the static colliders for the entire world
typedef struct World {
    int gridSize;
    int width,height; // in tiles
    uint8_t** editorGrid;
    uint8_t** colliderGrid;
    int roomCount;
    Room* rooms;
} World;

World* NewWorld(Arena* arena, int width, int height, int gridSize);
World* LoadWorld(Arena* arena, const char* file);
void SaveWorld(World* world, const char* file);

void ApplyWorld(ECS** ecs, World* world);

void Autotile(World* world);
void DrawWorldTiles(World* world, Texture2D* tileset);

void ResolveWorldCollisions(ECS* ecs, World* world);

RayCollision2D CheckCollisionRayWorld(Vector2 point, Vector2 direction, World* world);
bool CheckCollisionPointWorld(Vector2 pt, World* world);

/* Format:
%x ; gridSize
%x,%x ; dimensions
%x %x ... ; x dimension times
%x ; number of entities
; standard ECS serialization system

*/

#endif
