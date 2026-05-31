#include "world.h"
#include "utils.h"
#include <raylib/raymath.h>

#define MAX_ROOMS 20

World* NewWorld(Arena* arena, int width, int height, int gridSize) {
    World* results = ArenaAlloc(arena, sizeof(World));
    results->width = width;
    results->height = height;
    results->gridSize = gridSize;
    results->roomCount = 0;
    results->rooms = ArenaAlloc(arena, MAX_ROOMS * sizeof(Room));
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

World* LoadWorld(Arena* arena, const char* file) {
    World* results = ArenaAlloc(arena, sizeof(World));
    FILE* stream;
    stream = fopen(file, "r");
    if(!stream) {
        printf("ERROR: could not open world file %s\n", file);
        return NULL;
    }
    fscanf(stream,"%x\n", &results->gridSize);
    fscanf(stream,"%x,%x\n", &results->width, &results->height);
    results->colliderGrid = ArenaAlloc(arena,results->width*sizeof(uint8_t*));
    results->editorGrid = ArenaAlloc(arena, results->width*sizeof(uint8_t*));
    for(int x = 0; x < results->width; ++x) {
        results->colliderGrid[x] = ArenaAlloc(arena, results->height*sizeof(uint8_t));
        results->editorGrid[x] = ArenaAlloc(arena, results->height*sizeof(uint8_t));
        for(int y = 0; y < results->height; ++y) {
            fscanf(stream,"%hhx,%hhx ", (uint8_t*)&results->colliderGrid[x][y],&results->editorGrid[x][y]);
        }
        fscanf(stream,"\n");
    }
    int totalEntities = 0;
    fscanf(stream,"%x\n", &totalEntities);
    fscanf(stream, "%x\n", &results->roomCount);
    results->rooms = ArenaAlloc(arena,MAX_ROOMS * sizeof(Room));
    for(int i = 0; i < results->roomCount; ++i) {
        fscanf(stream, "%f,%f,%f,%f\n",
            &results->rooms[i].bounds.x,
            &results->rooms[i].bounds.y,
            &results->rooms[i].bounds.width,
            &results->rooms[i].bounds.height);
        results->rooms[i].active = false;
    }
    fclose(stream);
    return results;
}

void SaveWorld(World* world, const char* file) {
    FILE* stream;
    stream = fopen(file, "w");
    if(!stream) {
        printf("ERROR: could not open ECS world %s\n", file);
        return;
    }
    fprintf(stream,"%x\n", world->gridSize);
    fprintf(stream,"%x,%x\n", world->width, world->height);
    for(int x = 0; x < world->width; ++x) {
        for(int y = 0; y < world->height; ++y) {
            fprintf(stream,"%hhx,%hhx ", world->colliderGrid[x][y], world->editorGrid[x][y]);
        }
        fprintf(stream,"\n");
    }
    fprintf(stream, "%x\n", world->roomCount);
    for(int i = 0; i < world->roomCount; ++i) {
        fprintf(stream, "%f,%f,%f,%f\n",
            world->rooms[i].bounds.x,
            world->rooms[i].bounds.y,
            world->rooms[i].bounds.width,
            world->rooms[i].bounds.height);
    }
    fclose(stream);
}

static inline int IsTileValid(int tile) {
    return (tile > 0 && tile < 16) ? 1 : 0;
}

void Autotile(World* world) {
    for(int x = 0; x < world->width; ++x) {
        for(int y = 0; y < world->height; ++y) {
            uint8_t bitmask = 0;
            uint8_t bit = 1;
            int gridSpot = 0;
            if(y+1 < world->height) {
                gridSpot = IsTileValid(world->editorGrid[x][y+1]);
            }
            bitmask |= bit*gridSpot;
            bit = bit << 1;
            if(x+1 < world->width && y+1 < world->height) {
                gridSpot = IsTileValid(world->editorGrid[x+1][y+1]);
            }
            bitmask |= bit*gridSpot;
            bit = bit << 1;
            if(x+1 < world->width) {
                gridSpot = IsTileValid(world->editorGrid[x+1][y]);
            }
            bitmask |= bit*gridSpot;
            bit = bit << 1;
            gridSpot = IsTileValid(world->editorGrid[x][y]);
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
        case TILE_MS_1:
            *vertexCount = 3;
            vertices[0] = left;
            vertices[1] = bottom;
            vertices[2] = bl;
            break;
        case TILE_MS_2:
            *vertexCount = 3;
            vertices[0] = right;
            vertices[1] = br;
            vertices[2] = bottom;
            break;
        case TILE_MS_3:
            *vertexCount = 4;
            vertices[0] = left;
            vertices[1] = right;
            vertices[2] = br;
            vertices[3] = bl;
            break;
        case TILE_MS_4:
            *vertexCount = 3;
            vertices[0] = top;
            vertices[1] = tr;
            vertices[2] = right;
            break;
        case TILE_MS_5:
            *vertexCount = 6;
            vertices[0] = top;
            vertices[1] = tr;
            vertices[2] = right;
            vertices[3] = bottom;
            vertices[4] = bl;
            vertices[5] = left;
            break;
        case TILE_MS_6:
            *vertexCount = 4;
            vertices[0] = top;
            vertices[1] = tr;
            vertices[2] = br;
            vertices[3] = bottom;
            break;
        case TILE_MS_7:
            *vertexCount = 5;
            vertices[0] = top;
            vertices[1] = tr;
            vertices[2] = br;
            vertices[3] = bl;
            vertices[4] = left;
            break;
        case TILE_MS_8:
            *vertexCount = 3;
            vertices[0] = tl;
            vertices[1] = top;
            vertices[2] = left;
            break;
        case TILE_MS_9:
            *vertexCount = 4;
            vertices[0] = tl;
            vertices[1] = top;
            vertices[2] = bottom;
            vertices[3] = bl;
            break;
        case TILE_MS_10:
            *vertexCount = 6;
            vertices[0] = tl;
            vertices[1] = top;
            vertices[2] = right;
            vertices[3] = br;
            vertices[4] = bottom;
            vertices[5] = left;
            break;
        case TILE_MS_11:
            *vertexCount = 5;
            vertices[0] = tl;
            vertices[1] = top;
            vertices[2] = right;
            vertices[3] = br;
            vertices[4] = bl;
            break;
        case TILE_MS_12:
            *vertexCount = 4;
            vertices[0] = tl;
            vertices[1] = tr;
            vertices[2] = right;
            vertices[3] = left;
            break;
        case TILE_MS_13:
            *vertexCount = 5;
            vertices[0] = tl;
            vertices[1] = tr;
            vertices[2] = right;
            vertices[3] = bottom;
            vertices[4] = bl;
            break;
        case TILE_MS_14:
            *vertexCount = 5;
            vertices[0] = tl;
            vertices[1] = tr;
            vertices[2] = right;
            vertices[3] = br;
            vertices[4] = bottom;
            break;
        case TILE_MS_15:
            *vertexCount = 4;
            vertices[0] = tl;
            vertices[1] = tr;
            vertices[2] = br;
            vertices[3] = bl;
            break;
        default:
            *vertexCount = 0;
            break;
    }
}

const bool debugView = false;

void DrawWorldTiles(World* world, Texture2D* tileset) {
    Vector2 points[6];
    int pointsCount = 0;
    int gridSize = world->gridSize;
    for (int x = 0; x < world->width; ++x) {
        for (int y = 0; y < world->height; ++y) {
            DrawTexturePro(*tileset, 
            (Rectangle){world->colliderGrid[x][y]*gridSize,0,gridSize,gridSize},
            (Rectangle){gridSize*x + gridSize/2.0,gridSize*y + gridSize/2.0,gridSize,gridSize},
            (Vector2){},0,WHITE);
            if(debugView) {
                MarchingSquaresPolygon(world->colliderGrid[x][y], x+0.5,y+0.5,gridSize,points, &pointsCount);
                for(int i = 0; i < pointsCount; ++i) {
                    DrawLineV(points[i], points[(i+1)%pointsCount], BLUE);
                }
            }
        }
    }
}

void ResolveWorldCollisions(Actor* actor, World* world) {
    int gridSize = world->gridSize;
    Vector2 points[6];
    int pointsCount;
    int axesCount = 4;
    Vector2 axes[4]; 
    axes[0] = (Vector2){0,1};
    axes[1] = (Vector2){1,0};
    axes[2] = (Vector2){0.414141,0.414141};
    axes[3] = (Vector2){-0.414141,0.414141};
    for(int x = 0; x < world->width; ++x) {
        for(int y = 0; y < world->height; ++y) {
            if(!IsTileValid(world->colliderGrid[x][y])) continue;
            MarchingSquaresPolygon(world->colliderGrid[x][y], x+0.5,y+0.5,gridSize,points, &pointsCount);
            // SAT
            float overlap = 1000000;
            bool anOverlap = true;
            Vector2 smallest = (Vector2){};
            for(int ax = 0; ax < axesCount; ++ax) {
                Vector2 hbProj = ProjectRectangleOnAxis(actor->hitbox,axes[ax]);
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
                actor->impulse = Vector2Scale(smallest, overlap);
                actor->hitbox.x += actor->impulse.x;
                actor->hitbox.y += actor->impulse.y;
            }
        }
    }
}

RayCollision2D CheckCollisionRayWorld(Vector2 point, Vector2 direction, World* world) {
    int gridSize = world->gridSize;
    Vector2 points[6];
    int pointsCount = 0;
    RayCollision2D result = (RayCollision2D){false, (Vector2){}};
    float distanceS = 10000000;
    for (int x = 0; x < world->width; ++x) {
        for (int y = 0; y < world->height; ++y) {
            if(!IsTileValid(world->colliderGrid[x][y])) continue;
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
            if(!IsTileValid(world->colliderGrid[x][y])) continue;
            MarchingSquaresPolygon(world->colliderGrid[x][y], x+0.5,y+0.5,gridSize,points, &pointsCount);
            if(CheckCollisionPointPoly(pt, points, pointsCount)) return true;

        }
    }
    return false;
}
