#ifndef H_ROOM
#define H_ROOM

#include <ecs.h>
#include <raylib/raylib.h>
#include "raycast.h"

// For the static colliders for each in-game room
typedef struct Room {
    int gridSize;
    int width,height; // in tiles
    uint8_t** editorGrid;
    uint8_t** colliderGrid;
} Room;


Room* NewRoom(Arena* arena, int width, int height, int gridSize);
Room* LoadRoom(ECS* ecs, Arena* arena, const char* file);
void SaveRoom(ECS* ecs, Room* room, const char* file);

void Autotile(Room* room);
void DrawRoomTiles(Room* room, Texture2D* tileset);

void ResolveRoomCollisions(ECS* ecs, Room* room);
RayCollision2D CheckCollisionRayRoom(Vector2 point, Vector2 direction, Room* room);

/* Format:
%x ; gridSize
%x,%x ; dimensions
%x %x ... ; x dimension times
%x ; number of entities
; standard ECS serialization system

*/

#endif
