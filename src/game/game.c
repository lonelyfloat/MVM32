#include "program.h"
#include <raylib/raylib.h>
#include <raylib/raymath.h>
#include "assets.h"
#include "ecs.h"
#include "ui/ui.h"
#include "component_types.h"
#include <stdio.h>
#include "systems.h"
#include "player.h"
#include "child.h"
#include "utils.h"
#include "prefab.h"
#include "room.h"

ECS* ecs;
ECS* player;

int screenWidth = 800;
int screenHeight = 600;

Arena* arena;
bool paused = false;

ImGuiContext* ctx;

Room* currentRoom;

int totalRooms = 1;
int roomID = 0;
Room* allRooms;
char* roomName;

const int gridSize = 50;
Camera2D worldCamera = (Camera2D) {.offset=(Vector2){-gridSize/2.0,-gridSize/2.0},.target=(Vector2){},.rotation=0.0,.zoom=1.0};
Entity inspect = NULL_ENTITY;


// Editor stuff
enum EditorMode {
    EDITOR_TERRAIN,
    EDITOR_PORTAL
} editorMode;
Vector2 editorCursor;
Vector2 portalMin = (Vector2){};

int teleportID = 0;

bool editorEnabled = false;
int configPortal = -1;
bool dragging = false;
bool showRoomCreator;
Room hypotheticalRoom = (Room){};
int hypotheticalRoomID = 0;
// prospective room stats


void Init(Arena* gameArena) {
    arena = gameArena;
    roomName = ArenaAlloc(arena, 30);
    roomName[0] = '\0';
    printf("Initializing game...\n");
    ecs = InitECS(arena, 100, COMPONENT_COUNT);
    RegisterComponents(ecs, arena);
    player = InitECS(arena, 20, COMPONENT_COUNT);
    RegisterComponents(player, arena);
    InitAssets(gameArena, 10);
    LoadAsset("./assets/lab_tileset.png", "LabTileset", ASSET_TYPE_TEXTURE);
    LoadAsset("./assets/player.png", "Player", ASSET_TYPE_TEXTURE);
    allRooms = ArenaAlloc(gameArena, sizeof(Room)*totalRooms);
    for(int i = 0; i < totalRooms; ++i) {
        sprintf(roomName, "./assets/rooms/room%d", i);
        Room* room = LoadRoom(arena, roomName);
        allRooms[i] = *room;
    }
    currentRoom = &allRooms[0];
    roomID = 0;
    ApplyRoom(&ecs, currentRoom);
    sprintf(roomName, "./assets/rooms/room0");

    LoadEntitiesFromFile(player, arena, "./assets/prefabs/player");
    MergePrefab(ecs, player);
    SetRenderOrder(ecs);

    screenWidth = GetScreenWidth();
    screenHeight = GetScreenHeight();
    ctx = GetImGuiContext();
    ImGui_SetCurrentContext(ctx);
}

void Save(char* file) {
    Entity playerE = NULL_ENTITY;
    for(int i = 0; i < ecs->blocks[PLAYER_COMPONENT].count; ++i) {
        playerE = GetEntity(ecs, PLAYER_COMPONENT, i);
    }
    if(playerE != NULL_ENTITY) {
        MakePrefab(player, arena, ecs, playerE);
        SaveEntitiesToFile(player, "./assets/prefabs/player");
        KillPrefab(ecs, playerE);
    }
    currentRoom->entityData = ecs;
    sprintf(roomName, "./assets/rooms/room%d",roomID);
    SaveRoom(currentRoom, roomName);
    MergePrefab(ecs, player);
}

void Load(char* file) {
    sprintf(roomName, "./assets/rooms/room%d",roomID);
    currentRoom = LoadRoom(arena, roomName);
    ApplyRoom(&ecs, currentRoom);
    MergePrefab(ecs, player);
}

void UpdateEditor() {
    ImGui_Begin("Teleport", NULL, ImGuiWindowFlags_None);
    ImGui_TextUnformatted("Room ID:"); ImGui_SameLine();
    ImGui_InputInt("##RoomID",&teleportID);
    if(ImGui_Button("Teleport")) {
        roomID = teleportID;
        currentRoom = &allRooms[teleportID];
    }
    ImGui_End();
    ImGui_Begin("Create Room", &showRoomCreator, ImGuiWindowFlags_None);
    ImGui_TextUnformatted("Room ID:"); ImGui_SameLine();
    ImGui_InputInt("##RoomID",&hypotheticalRoomID);
    ImGui_TextUnformatted("Room Dimensions:"); 
    ImGui_InputInt("##RoomW",&hypotheticalRoom.width); 
    ImGui_InputInt("##RoomH",&hypotheticalRoom.height);
    if(ImGui_Button("Create Room")) {
        Room* new = NewRoom(arena,hypotheticalRoom.width, hypotheticalRoom.height, gridSize);
        sprintf(roomName, "./assets/rooms/room%d", hypotheticalRoomID);
        SaveRoom(new, roomName);
        currentRoom = new;
        roomID = hypotheticalRoomID;
    }
    ImGui_End();
    if(configPortal >= 0 && editorMode == EDITOR_PORTAL) {
        ImGui_Begin("Setup portal", NULL, ImGuiWindowFlags_None);
        ImGui_Text("Portal ID: %d", configPortal);
        ImGui_TextUnformatted("Dest Room");
        ImGui_SameLine();
        ImGui_InputInt("##DestR", &currentRoom->portals[configPortal].destinationRoom);
        ImGui_TextUnformatted("Dest Portal");
        ImGui_SameLine();
        ImGui_InputInt("##DestP", &currentRoom->portals[configPortal].destinationPortal);
        ImGui_End();
    }
    ImGui_Begin("Room Tools", NULL, ImGuiWindowFlags_None);
    if(ImGui_Button("Terrain")) {
        editorMode = EDITOR_TERRAIN;
    }
    if(ImGui_Button("Portal")) {
        editorMode = EDITOR_PORTAL;
    }
    ImGui_End();
    // 
    // Zoom based on mouse wheel
    float wheel = GetMouseWheelMove();
    ImGuiIO* io = ImGui_GetIO();
    if (wheel != 0 && !io->WantCaptureMouse)
    {
        // Get the world point that is under the mouse
        Vector2 mouseWorldPos = GetScreenToWorld2D(GetMousePosition(), worldCamera);
        // Set the offset to where the mouse is
        worldCamera.offset = GetMousePosition();
        // Set the target to match, so that the worldCamera maps the world space point
        // under the cursor to the screen space point under the cursor at any zoom
        worldCamera.target = mouseWorldPos;
        // Zoom increment
        // Uses log scaling to provide consistent zoom speed
        float scale = 0.2f*wheel;
        worldCamera.zoom = Clamp(expf(logf(worldCamera.zoom)+scale), 0.125f, 64.0f);
    }

    Vector2 mousePos = GetScreenToWorld2D(GetMousePosition(),worldCamera);;
    editorCursor.x = floor(mousePos.x/gridSize);
    editorCursor.y = floor(mousePos.y/gridSize);

    if(!io->WantCaptureMouse) {
        switch(editorMode) {
            case EDITOR_TERRAIN:
                if(IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
                    if(editorCursor.x >= 0 && editorCursor.x < currentRoom->width
                    && editorCursor.y >= 0 && editorCursor.y < currentRoom->height) {
                        currentRoom->editorGrid[(int)editorCursor.x][(int)editorCursor.y] = 1;
                        Autotile(currentRoom);
                    }
                }
                if(IsMouseButtonDown(MOUSE_RIGHT_BUTTON)) {
                    if(editorCursor.x >= 0 && editorCursor.x < currentRoom->width
                    && editorCursor.y >= 0 && editorCursor.y < currentRoom->height) {
                        currentRoom->editorGrid[(int)editorCursor.x][(int)editorCursor.y] = 0;
                        Autotile(currentRoom);
                    }
                }
                break;
            case EDITOR_PORTAL:
                if(IsKeyPressed(KEY_C)) {
                    for(int i = 0; i < currentRoom->portalCount; ++i) {
                        if(CheckCollisionPointRec(mousePos, currentRoom->portals[i].bounds)) {
                            configPortal = i;
                        }
                    }
                }
                if(IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                    portalMin = Vector2Scale(editorCursor, gridSize);
                    dragging = true;
                }
                if(IsMouseButtonPressed(MOUSE_RIGHT_BUTTON)) {
                    for(int i = 0; i < currentRoom->portalCount; ++i) {
                        if(CheckCollisionPointRec(mousePos, currentRoom->portals[i].bounds)) {
                            Portal temp = currentRoom->portals[currentRoom->portalCount-1];
                            currentRoom->portals[i] = temp;
                            currentRoom->portalCount--;
                        }
                    }
                }
                if(!IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
                    if(dragging && currentRoom->portalCount < 10) {
                        currentRoom->portals[currentRoom->portalCount].bounds = (Rectangle){
                            portalMin.x, 
                            portalMin.y,
                            gridSize*ceil(mousePos.x/gridSize) - portalMin.x,
                            gridSize*ceil(mousePos.y/gridSize) - portalMin.y
                        };
                        currentRoom->portalCount++;
                    }
                    dragging =false;
                }
                break;
        }
    }
}

void DrawElement(int i) {
    Entity e = GetEntity(ecs, ZORDER_COMPONENT, i);
    // printf("e: %x\n", e);
    if(!HasComponent(ecs, e, HITBOX_COMPONENT)) return;
    if(HasComponent(ecs, e, SPRITE_COMPONENT)) {
        Hitbox* hb = GetComponent(ecs, e, HITBOX_COMPONENT);
        Sprite* spr = GetComponent(ecs, e, SPRITE_COMPONENT);
        Texture2D* asset = GetAsset(spr->texture.key);
        if(spr->texture.length != 0) {
            DrawTexturePro(*asset, (Rectangle){0,0, (spr->flipped ? -1 : 1)*asset->width, asset->height}, (Rectangle){hb->pos.x, hb->pos.y, hb->scale.x, hb->scale.y}, (Vector2){}, 0, WHITE);
        }
    }
    else if(HasComponents(ecs, e, 2, IK_LEG_COMPONENT, DEBUG_SHAPE_COMPONENT)) {
        if(!HasComponent( ecs, e, RELATIONSHIP_COMPONENT)) return;
        Relationship* r = GetComponent(ecs, e, RELATIONSHIP_COMPONENT);
        DebugShape *d = GetComponent(ecs, e, DEBUG_SHAPE_COMPONENT);
        Entity last = e;
        Entity head = r->first;
        while(head != NULL_ENTITY) {
            if(!HasComponent( ecs, head, RELATIONSHIP_COMPONENT)) break;
            Hitbox* hb = GetComponent(ecs, head, HITBOX_COMPONENT);
            Hitbox* lastHB = GetComponent(ecs, last, HITBOX_COMPONENT);
            DrawLineEx(lastHB->pos, hb->pos, 10, d->col);
            r = GetComponent(ecs, head, RELATIONSHIP_COMPONENT);
            last = head;
            head = r->next;
        }
    }
}


void DrawEditor() {
    Vector2 o = GetWorldToScreen2D((Vector2){0,0}, worldCamera);
    Vector2 offset = (Vector2) {
        o.x - gridSize*worldCamera.zoom*ceil(o.x/(gridSize*worldCamera.zoom)),
        o.y - gridSize*worldCamera.zoom*ceil(o.y/(gridSize*worldCamera.zoom))};
    float guiGridSize = gridSize*worldCamera.zoom;
    float guiScreenWidth = screenWidth - offset.x;
    float guiScreenHeight = screenHeight - offset.y;
    Color gridColor = ColorAlpha(BLUE, 0.5);
    for(int x = 0; x < (int)(guiScreenWidth / guiGridSize); ++x) {
        DrawLine(offset.x + x*guiGridSize, offset.y, offset.x + x*guiGridSize, offset.y+guiScreenHeight, gridColor);
    }
    for(int y = 0; y < (int)(guiScreenHeight / guiGridSize); ++y) {
        DrawLine(offset.x, offset.y + y*guiGridSize, offset.x+guiScreenWidth, offset.y + y * guiGridSize, gridColor);
    }

    BeginMode2D(worldCamera);
    DrawRectangleLinesEx((Rectangle){0,0,currentRoom->width*gridSize,currentRoom->height*gridSize},10, RED);
    for(int i = 0; i < currentRoom->portalCount; ++i) {
        DrawRectangleRec(currentRoom->portals[i].bounds, ColorAlpha(GREEN, 0.5));
    }
    EndMode2D();
}

void UpdateDrawFrame(void) {
    // Update
    //----------------------------------------------------------------------------------
    if(IsKeyDown(KEY_LEFT_CONTROL) && IsKeyPressed(KEY_V)) {
        editorEnabled = !editorEnabled;
    }
    if(!editorEnabled) {
        PlayerSystem(ecs, currentRoom);
        VelocitySystem(ecs);
        IKPoleSystem(ecs);
        IKLegSystem(ecs,currentRoom);
        ResolveIK(ecs,100);
        ResolveRoomCollisions(ecs, currentRoom);
        PlayerSlopes(ecs, currentRoom);
    }
    if(IsKeyDown(KEY_LEFT_CONTROL) && IsKeyPressed(KEY_L)) {
        sprintf(roomName, "./assets/rooms/room%d",roomID);
        currentRoom = LoadRoom(arena, roomName);
        ApplyRoom(&ecs, currentRoom);
    }
    if(IsKeyDown(KEY_LEFT_CONTROL) && IsKeyPressed(KEY_S)) {
        Entity playerE = NULL_ENTITY;
        for(int i = 0; i < ecs->blocks[PLAYER_COMPONENT].count; ++i) {
            playerE = GetEntity(ecs, PLAYER_COMPONENT, i);
        }
        if(playerE != NULL_ENTITY) {
            MakePrefab(player, arena, ecs, playerE);
            SaveEntitiesToFile(player, "./assets/prefabs/player");
            KillPrefab(ecs, playerE);
        }
        currentRoom->entityData = ecs;
        SaveRoom(currentRoom, roomName);
        MergePrefab(ecs, player);
    }

    BeginUI();
    EntityPanel(arena,ecs, &inspect, IsKeyDown(KEY_LEFT_CONTROL) && IsKeyPressed(KEY_E));
    if(editorEnabled) {
        UpdateEditor();
    }
    EndUI();


    // ---------------------------------------------------------------------------------
    // Draw
    // ---------------------------------------------------------------------------------
    BeginDrawing();
        ClearBackground(BLACK);
        BeginMode2D(worldCamera);
        Texture2D* tileset = GetAsset("LabTileset");
        int stopIdx = 0;
        for(int i = 0; i < ecs->blocks[ZORDER_COMPONENT].count; ++i) {
            if(IndexComponent(ecs, ZOrder, ZORDER_COMPONENT,i).z > 0) { 
                stopIdx = i;
                break;
            }
            DrawElement(i);
        }
        DrawRoomTiles(currentRoom, tileset);
        // printf("START\n");
        for(int i = stopIdx; i < ecs->blocks[ZORDER_COMPONENT].count; ++i) {
            DrawElement(i);
        }
        // for(int i = 0; i < ecs->blocks[DEBUG_SHAPE_COMPONENT].count; ++i) {
        //     Entity e = GetEntity(ecs, DEBUG_SHAPE_COMPONENT, i);
        //     if(!HasComponent(ecs, e, HITBOX_COMPONENT)) continue;
        //     Hitbox* hb = GetComponent(ecs, e, HITBOX_COMPONENT);
        //     DebugShape d = IndexComponent(ecs, DebugShape,DEBUG_SHAPE_COMPONENT, i);
        //     DrawRectangleRec(HitboxToRect(*hb), d.col);
        // }
        EndMode2D();
        if(editorEnabled) {
            DrawEditor();
        }
        DrawUI();
    EndDrawing();
}
