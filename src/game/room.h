#ifndef H_ROOM
#define H_ROOM

#include <ecs.h>
#include <raylib/raylib.h>

// For the static colliders for each in-game room
typedef struct Room {
    int width,height; // in tiles
    uint8_t** editorGrid;
    uint8_t** colliderGrid;
} Room;


Room* NewRoom(Arena* arena, int width, int height);
Room* LoadRoom(ECS* ecs, Arena* arena, const char* file);
void SaveRoom(ECS* ecs, Room* room, const char* file);

void Autotile(Room* room);
void DrawRoomTiles(Room* room, int gridSize, Texture2D* tileset);

void ResolveRoomCollisions(ECS* ecs, int gridSize, Room* room);
/* Format:
%x,%x ; dimensions
%x %x ... ; x dimension times
%x ; number of entities
; standard ECS serialization system

*/

#endif
