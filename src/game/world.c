#include "world.h"
#include <serialize.h>
#include "utils.h"
#include <raylib/raymath.h>
    
// World* NewWorld(Arena* arena, int width, int height, int gridSize) {
//     World* results = ArenaAlloc(arena, sizeof(World));
//     results->width = width;
//     results->height = height;
//     results->gridSize = gridSize;
//     results->portalCount = 0;
//     results->portals = ArenaAlloc(arena, ROOM_MAX_PORTALS * sizeof(Portal));
//     results->colliderGrid = ArenaAlloc(arena, results->width*sizeof(uint8_t*));
//     results->editorGrid = ArenaAlloc(arena, results->width*sizeof(uint8_t*));
//     results->entityData = InitECS(arena, ROOM_MAX_ENTITIES, COMPONENT_COUNT); 
//     for(int x = 0; x < results->width; ++x) {
//         results->colliderGrid[x] = ArenaAlloc(arena, results->height*sizeof(uint8_t));
//         results->editorGrid[x] = ArenaAlloc(arena, results->height*sizeof(uint8_t));
//         for(int y = 0; y < results->height; ++y) {
//             results->colliderGrid[x][y] = 0;
//             results->editorGrid[x][y] = 0;
//         }
//     }
//     return results;
// }
// 
// World* LoadWorld(Arena* arena, const char* file) {
//     World* results = ArenaAlloc(arena, sizeof(World));
//     FILE* stream;
//     stream = fopen(file, "r");
//     if(!stream) {
//         printf("ERROR: could not open world file %s\n", file);
//         return NULL;
//     }
//     fscanf(stream,"%x\n", &results->gridSize);
//     fscanf(stream,"%x,%x\n", &results->width, &results->height);
//     results->colliderGrid = ArenaAlloc(arena,results->width*sizeof(uint8_t*));
//     results->editorGrid = ArenaAlloc(arena, results->width*sizeof(uint8_t*));
//     for(int x = 0; x < results->width; ++x) {
//         results->colliderGrid[x] = ArenaAlloc(arena, results->height*sizeof(uint8_t));
//         results->editorGrid[x] = ArenaAlloc(arena, results->height*sizeof(uint8_t));
//         for(int y = 0; y < results->height; ++y) {
//             fscanf(stream,"%hhx,%hhx ", &results->colliderGrid[x][y],&results->editorGrid[x][y]);
//         }
//         fscanf(stream,"\n");
//     }
//     int totalEntities = 0;
//     fscanf(stream,"%x\n", &totalEntities);
//     results->entityData = InitECS(arena, ROOM_MAX_ENTITIES, COMPONENT_COUNT);
//     RegisterComponents(results->entityData, arena);
//     for(int i = 0; i < totalEntities; ++i) {
//         Entity e = CreateEntity(results->entityData);
//         DeserializeEntity(stream, arena, results->entityData, e);
//     }
//     fscanf(stream, "%x\n", &results->portalCount);
//     results->portals = ArenaAlloc(arena,ROOM_MAX_PORTALS * sizeof(Portal));
//     for(int i = 0; i < results->portalCount; ++i) {
//         fscanf(stream, "%f,%f,%f,%f,%x,%x\n",
//             &results->portals[i].bounds.x,
//             &results->portals[i].bounds.y,
//             &results->portals[i].bounds.width,
//             &results->portals[i].bounds.height,
//             &results->portals[i].destinationWorld,
//             &results->portals[i].destinationPortal);
//     }
//     fclose(stream);
//     return results;
// }
// 
// void SaveWorld(World* world, const char* file) {
//     FILE* stream;
//     stream = fopen(file, "w");
//     if(!stream) {
//         printf("ERROR: could not open ECS world %s\n", file);
//         return;
//     }
// 
//     fprintf(stream,"%x\n", world->gridSize);
//     fprintf(stream,"%x,%x\n", world->width, world->height);
//     for(int x = 0; x < world->width; ++x) {
//         for(int y = 0; y < world->height; ++y) {
//             fprintf(stream,"%hhx,%hhx ", world->colliderGrid[x][y], world->editorGrid[x][y]);
//         }
//         fprintf(stream,"\n");
//     }
//     ECS* ecs = world->entityData;
//     int entitiesAlive = 0;
//     for(uint16_t en = 0; en < ecs->totalEntities; ++en) {
//         if(en != GetID(ecs->entities[en])) { 
//             continue;
//         }
//         entitiesAlive++;
//     }
// 
//     fprintf(stream,"%x\n", entitiesAlive);
//     for(uint16_t en = 0; en < ecs->totalEntities; ++en) {
//         if(en != GetID(ecs->entities[en])) {
//             continue;
//         }
//         SerializeEntity(stream, ecs, ecs->entities[en]);
//     }
//     fprintf(stream, "%x\n", world->portalCount);
//     for(int i = 0; i < world->portalCount; ++i) {
//         fprintf(stream, "%f,%f,%f,%f,%x,%x\n",
//             world->portals[i].bounds.x,
//             world->portals[i].bounds.y,
//             world->portals[i].bounds.width,
//             world->portals[i].bounds.height,
//             world->portals[i].destinationWorld,
//             world->portals[i].destinationPortal);
//     }
//     fclose(stream);
// }
// 
void Autotile(World* world) {
    for(int x = 0; x < world->width; ++x) {
        for(int y = 0; y < world->height; ++y) {
            uint8_t bitmask = 0;
            uint8_t bit = 1;
            int gridSpot = 0;
            if(y+1 < world->height) {
                gridSpot = world->editorGrid[x][y+1] > 0;
            }
            bitmask |= bit*gridSpot;
            bit = bit << 1;
            if(x+1 < world->width && y+1 < world->height) {
                gridSpot = world->editorGrid[x+1][y+1] > 0;
            }
            bitmask |= bit*gridSpot;
            bit = bit << 1;
            if(x+1 < world->width) {
                gridSpot = world->editorGrid[x+1][y] > 0;
            }
            bitmask |= bit*gridSpot;
            bit = bit << 1;
            gridSpot = world->editorGrid[x][y] > 0;
            bitmask |= bit*gridSpot;
            world->colliderGrid[x][y] = bitmask;
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
            vertices[1] = br;
            vertices[2] = bottom;
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
            vertices[2] = right;
            vertices[3] = br;
            vertices[4] = bottom;
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
// 
// void ApplyWorld(ECS** ecs, World* world) {
//     *ecs = world->entityData;
// }
// 
// const bool debugView = false;
// 
// void DrawWorldTiles(World* world, Texture2D* tileset) {
//     Vector2 points[6];
//     int pointsCount = 0;
//     int gridSize = world->gridSize;
//     for (int x = 0; x < world->width; ++x) {
//         for (int y = 0; y < world->height; ++y) {
//             DrawTexturePro(*tileset, 
//             (Rectangle){world->colliderGrid[x][y]*gridSize,0,gridSize,gridSize},
//             (Rectangle){gridSize*x + gridSize/2.0,gridSize*y + gridSize/2.0,gridSize,gridSize},
//             (Vector2){},0,WHITE);
//             if(debugView) {
//                 MarchingSquaresPolygon(world->colliderGrid[x][y], x+0.5,y+0.5,gridSize,points, &pointsCount);
//                 for(int i = 0; i < pointsCount; ++i) {
//                     DrawLineV(points[i], points[(i+1)%pointsCount], BLUE);
//                 }
//             }
//         }
//     }
// }
// 
// void ResolveWorldCollisions(ECS* ecs, World* world) {
//     int gridSize = world->gridSize;
//     Vector2 points[6];
//     int pointsCount;
//     int axesCount = 4;
//     Vector2 axes[4]; 
//     axes[0] = (Vector2){0,1};
//     axes[1] = (Vector2){1,0};
//     axes[2] = (Vector2){0.414141,0.414141};
//     axes[3] = (Vector2){-0.414141,0.414141};
//     for(int i = 0; i < ecs->blocks[ACTOR_COMPONENT].count; ++i) {
//         Entity e = GetEntity(ecs, ACTOR_COMPONENT, i);
//         if(!HasComponent(ecs, e, HITBOX_COMPONENT)) continue;
//         Actor* impulse = &IndexComponent(ecs, Actor, ACTOR_COMPONENT, i);
//         impulse->impulse = Vector2Zero();
//         Hitbox* hb = GetComponent(ecs, e, HITBOX_COMPONENT);
//         RayCollision2D ray = (RayCollision2D){false,(Vector2){}, (Vector2){}};
//         Vector2 avgImpulse = impulse->impulse;
//         float distance = 1000000;
//         Vector2 pt = (Vector2){hb->pos.x+hb->scale.x/2,hb->pos.y+hb->scale.y/2};
//         for(int x = 0; x < world->width; ++x) {
//             for(int y = 0; y < world->height; ++y) {
//                 if(world->colliderGrid[x][y] == 0) continue;
//                 MarchingSquaresPolygon(world->colliderGrid[x][y], x+0.5,y+0.5,gridSize,points, &pointsCount);
//                 // Raytrace for slopes
//                 for(int i = 0; i < pointsCount; ++i) {
//                     RayCollision2D r = CheckCollisionRayLine(pt,(Vector2){0,1},points[i], points[(i+1)%pointsCount]);
//                     if(r.hit) {
//                         float distSq = Vector2DistanceSqr(pt, r.point);
//                         if(distSq < distance) {
//                             distance = distSq;
//                             ray = r;
//                         }
//                     }
//                 }
//                 // SAT
//                 float overlap = 1000000;
//                 bool anOverlap = true;
//                 Vector2 smallest = (Vector2){};
//                 for(int ax = 0; ax < axesCount; ++ax) {
//                     Vector2 hbProj = ProjectRectangleOnAxis(HitboxToRect(*hb),axes[ax]);
//                     Vector2 tileProj = ProjectPolygonOnAxis(points, pointsCount, axes[ax]);
//                     float ov = GetSignedOverlap(hbProj, tileProj);
//                     if(ov == 0) {
//                         anOverlap = false;
//                         break;
//                     }
//                     if(fabs(ov) < fabs(overlap)) {
//                         overlap = ov;
//                         smallest = axes[ax];
//                     }
//                 }
//                 if(anOverlap) {
//                     impulse->impulse = Vector2Scale(smallest, overlap);
//                     if(impulse->autoApply) {
//                         hb->pos = Vector2Add(hb->pos, impulse->impulse);
//                     }
//                     avgImpulse = Vector2Add(avgImpulse, impulse->impulse);
//                 }
//             }
//         }
//         impulse->raycast = ray;
//     }
// }

RayCollision2D CheckCollisionRayWorld(Vector2 point, Vector2 direction, World* world) {
    int gridSize = world->gridSize;
    Vector2 points[6];
    int pointsCount = 0;
    RayCollision2D result = (RayCollision2D){false, (Vector2){}};
    float distanceS = 10000000;
    for (int x = 0; x < world->width; ++x) {
        for (int y = 0; y < world->height; ++y) {
            if(world->colliderGrid[x][y] == 0) continue;
            MarchingSquaresPolygon(world->colliderGrid[x][y], x+0.5,y+0.5,gridSize,points, &pointsCount);
            for(int i = 0; i < pointsCount; ++i) {
                RayCollision2D r = CheckCollisionRayLine(point,direction,points[i], points[(i+1)%pointsCount]);
                if(r.hit) {
                    float distSq = Vector2DistanceSqr(point, r.point);
                    if(distSq < distanceS) {
                        distanceS = distSq;
                        result = r;
                    }
                }
            }
        }
    }
    return result;
}

bool CheckCollisionPointWorld(Vector2 pt, World* world) {
    int gridSize = world->gridSize;
    Vector2 points[6];
    int pointsCount;
    for (int x = 0; x < world->width; ++x) {
        for (int y = 0; y < world->height; ++y) {
            if(world->colliderGrid[x][y] == 0) continue;
            MarchingSquaresPolygon(world->colliderGrid[x][y], x+0.5,y+0.5,gridSize,points, &pointsCount);
            if(CheckCollisionPointPoly(pt, points, pointsCount)) return true;

        }
    }
    return false;
}
