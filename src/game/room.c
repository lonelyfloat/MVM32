#include "room.h"
#include <serialize.h>
#include "utils.h"
#include <raylib/raymath.h>
#include "component_types.h"
    
Room* NewRoom(Arena* arena, int width, int height) {
    Room* results = ArenaAlloc(arena, sizeof(Room));
    results->width = width;
    results->height = height;
    results->colliderGrid = ArenaAlloc(arena, results->width*sizeof(uint8_t*));
    results->editorGrid = ArenaAlloc(arena, results->width*sizeof(uint8_t*));
    for(int x = 0; x < results->width; ++x) {
        results->colliderGrid[x] = ArenaAlloc(arena, results->height*sizeof(uint8_t));
        results->editorGrid[x] = ArenaAlloc(arena, results->height*sizeof(uint8_t));
        for(int y = 0; y < results->height; ++y) {
            results->colliderGrid[x][y] = 0;
            results->editorGrid[x][y] = 0;
        }
    }
    return results;
}

Room* LoadRoom(ECS* ecs, Arena* arena, const char* file) {
    Room* results = ArenaAlloc(arena, sizeof(Room));
    FILE* stream;
    stream = fopen(file, "r");
    if(!stream) {
        printf("ERROR: could not open room file %s\n", file);
        return NULL;
    }
    fscanf(stream,"%x,%x\n", &results->width, &results->height);
    results->colliderGrid = ArenaAlloc(arena,results->width*sizeof(uint8_t*));
    results->editorGrid = ArenaAlloc(arena, results->width*sizeof(uint8_t*));
    for(int x = 0; x < results->width; ++x) {
        results->colliderGrid[x] = ArenaAlloc(arena, results->height*sizeof(uint8_t));
        results->editorGrid[x] = ArenaAlloc(arena, results->height*sizeof(uint8_t));
        for(int y = 0; y < results->height; ++y) {
            fscanf(stream,"%hhx,%hhx ", &results->colliderGrid[x][y],&results->editorGrid[x][y]);
        }
        fscanf(stream,"\n");
    }
    NukeECS(ecs);
    int totalEntities = 0;
    fscanf(stream,"%x\n", &totalEntities);
    while(!feof(stream)) {
        Entity e = CreateEntity(ecs);
        DeserializeEntity(stream, arena, ecs, e);
    }
    fclose(stream);
    return results;
}

void SaveRoom(ECS* ecs, Room* room, const char* file) {
    FILE* stream;
    stream = fopen(file, "w");
    if(!stream) {
        printf("ERROR: could not open ECS room %s\n", file);
        return;
    }
    fprintf(stream,"%x,%x\n", room->width, room->height);
    for(int x = 0; x < room->width; ++x) {
        for(int y = 0; y < room->height; ++y) {
            fprintf(stream,"%hhx,%hhx ", room->colliderGrid[x][y], room->editorGrid[x][y]);
        }
        fprintf(stream,"\n");
    }
    fprintf(stream,"%x\n", ecs->totalEntities);
    for(uint16_t en = 0; en < ecs->totalEntities; ++en) {
        if(en != GetID(ecs->entities[en])) {
            continue;
        }
        SerializeEntity(stream, ecs, ecs->entities[en]);
    }
    fclose(stream);
}

void Autotile(Room* room) {
    for(int x = 0; x < room->width; ++x) {
        for(int y = 0; y < room->height; ++y) {
            uint8_t bitmask = 0;
            uint8_t bit = 1;
            int gridSpot = 0;
            if(y+1 < room->height) {
                gridSpot = room->editorGrid[x][y+1] > 0;
            }
            bitmask |= bit*gridSpot;
            bit = bit << 1;
            if(x+1 < room->width && y+1 < room->height) {
                gridSpot = room->editorGrid[x+1][y+1] > 0;
            }
            bitmask |= bit*gridSpot;
            bit = bit << 1;
            if(x+1 < room->width) {
                gridSpot = room->editorGrid[x+1][y] > 0;
            }
            bitmask |= bit*gridSpot;
            bit = bit << 1;
            gridSpot = room->editorGrid[x][y] > 0;
            bitmask |= bit*gridSpot;
            room->colliderGrid[x][y] = bitmask;
        }
    }
}


static void MarchingSquaresPolygon(int value, float x, float y, int gridSize, Vector2* vertices, int* vertexCount) {
    const Vector2 left = (Vector2){x*gridSize,(y+0.5)*gridSize};
    const Vector2 right = (Vector2){(x+1)*gridSize, (y+0.5)*gridSize};
    const Vector2 top = (Vector2){(x+0.5)*gridSize, (y)*gridSize};
    const Vector2 bottom = (Vector2){(x+0.5)*gridSize, (y+1)*gridSize};
    const Vector2 tl = (Vector2){x*gridSize,y*gridSize};
    const Vector2 tr = (Vector2){(x+1)*gridSize,y*gridSize};
    const Vector2 bl = (Vector2){(x)*gridSize,(y+1)*gridSize};
    const Vector2 br = (Vector2){(x+1)*gridSize,(y+1)*gridSize};
    switch(value) {
        case 1:
            *vertexCount = 3;
            vertices[0] = left;
            vertices[1] = bottom;
            vertices[2] = bl;
            break;
        case 2:
            *vertexCount = 3;
            vertices[0] = right;
            vertices[1] = bottom;
            vertices[2] = br;
            break;
        case 3:
            *vertexCount = 4;
            vertices[0] = left;
            vertices[1] = right;
            vertices[2] = br;
            vertices[3] = bl;
            break;
        case 4:
            *vertexCount = 3;
            vertices[0] = top;
            vertices[1] = tr;
            vertices[2] = right;
            break;
        case 5:
            *vertexCount = 6;
            vertices[0] = top;
            vertices[1] = tr;
            vertices[2] = right;
            vertices[3] = bottom;
            vertices[4] = bl;
            vertices[5] = left;
            break;
        case 6:
            *vertexCount = 4;
            vertices[0] = top;
            vertices[1] = tr;
            vertices[2] = br;
            vertices[3] = bottom;
            break;
        case 7:
            *vertexCount = 5;
            vertices[0] = top;
            vertices[1] = tr;
            vertices[2] = br;
            vertices[3] = bl;
            vertices[4] = left;
            break;
        case 8:
            *vertexCount = 3;
            vertices[0] = tl;
            vertices[1] = top;
            vertices[2] = left;
            break;
        case 9:
            *vertexCount = 4;
            vertices[0] = tl;
            vertices[1] = top;
            vertices[2] = bottom;
            vertices[3] = bl;
            break;
        case 10:
            *vertexCount = 6;
            vertices[0] = tl;
            vertices[1] = top;
            vertices[2] = right;
            vertices[3] = br;
            vertices[4] = bottom;
            vertices[5] = left;
            break;
        case 11:
            *vertexCount = 5;
            vertices[0] = tl;
            vertices[1] = top;
            vertices[2] = right;
            vertices[3] = br;
            vertices[4] = bl;
            break;
        case 12:
            *vertexCount = 4;
            vertices[0] = tl;
            vertices[1] = tr;
            vertices[2] = right;
            vertices[3] = left;
            break;
        case 13:
            *vertexCount = 5;
            vertices[0] = tl;
            vertices[1] = tr;
            vertices[2] = right;
            vertices[3] = bottom;
            vertices[4] = bl;
            break;
        case 14:
            *vertexCount = 5;
            vertices[0] = tl;
            vertices[1] = tr;
            vertices[2] = br;
            vertices[3] = bottom;
            vertices[4] = right;
            break;
        case 15:
            *vertexCount = 4;
            vertices[0] = tl;
            vertices[1] = tr;
            vertices[2] = br;
            vertices[3] = bl;
            break;

    }
}

void DrawRoomTiles(Room* room, int gridSize, Texture2D* tileset) {
    for (int x = 0; x < room->width; ++x) {
        for (int y = 0; y < room->height; ++y) {
            DrawTexturePro(*tileset, 
            (Rectangle){room->colliderGrid[x][y]*gridSize,0,gridSize,gridSize},
            (Rectangle){gridSize*x + gridSize/2.0,gridSize*y + gridSize/2.0,gridSize,gridSize},
            (Vector2){},0,WHITE);
        }
    }
}

void ResolveRoomCollisions(ECS* ecs, int gridSize, Room* room) {
    Vector2 points[6];
    int pointsCount;
    int axesCount = 4;
    Vector2 axes[4]; 
    axes[0] = (Vector2){0,1};
    axes[1] = (Vector2){1,0};
    axes[2] = (Vector2){0.414141,0.414141};
    axes[3] = (Vector2){-0.414141,0.414141};
    for(int i = 0; i < ecs->blocks[ACTOR_COMPONENT].count; ++i) {
        Entity e = GetEntity(ecs, ACTOR_COMPONENT, i);
        if(!HasComponent(ecs, e, HITBOX_COMPONENT)) continue;
        Actor* impulse = &IndexComponent(ecs, Actor, ACTOR_COMPONENT, i);
        impulse->impulse = Vector2Zero();
        Hitbox* hb = GetComponent(ecs, e, HITBOX_COMPONENT);
        for(int x = 0; x < room->width; ++x) {
            for(int y = 0; y < room->height; ++y) {
                MarchingSquaresPolygon(room->colliderGrid[x][y], x+0.5,y+0.5,gridSize,points, &pointsCount);
                float overlap = 1000000;
                bool anOverlap = true;
                Vector2 smallest = (Vector2){};
                for(int ax = 0; ax < axesCount; ++ax) {
                    Vector2 hbProj = ProjectRectangleOnAxis(HitboxToRect(*hb),axes[ax]);
                    Vector2 tileProj = ProjectPolygonOnAxis(points, pointsCount, axes[ax]);
                    float ov = GetSignedOverlap(hbProj, tileProj);
                    if(ov == 0) {
                        anOverlap = false;
                        break;
                    }
                    if(fabs(ov) < fabs(overlap)) {
                        overlap = ov;
                        smallest = axes[ax];
                    }
                }
                if(anOverlap) {
                    impulse->impulse = Vector2Scale(smallest, overlap);
                }
            }
        }
    }
}
