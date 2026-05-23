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
    InitAssets(gameArena, 2);
    LoadAsset("./assets/temp_tileset.png", "TempTileset", ASSET_TYPE_TEXTURE);
    allRooms = ArenaAlloc(gameArena, sizeof(Room)*totalRooms);
    allRooms[0] = *NewRoom(arena, screenWidth/gridSize + 2,screenHeight/gridSize + 2,50);
    currentRoom = &allRooms[0];
    roomID = 0;
    sprintf(roomName, "./assets/rooms/room0");
    screenWidth = GetScreenWidth();
    screenHeight = GetScreenHeight();
    ctx = GetImGuiContext();
    ImGui_SetCurrentContext(ctx);
}

void Save(char* file) {
    SaveEntitiesToFile(ecs, file);
}

void Load(char* file) {
    NukeECS(ecs);
    LoadEntitiesFromFile(ecs, arena, file);
}

void UpdateEditor() {
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
    ImGui_Text("dragging: %s", dragging ? "true" : "false");
    ImGui_End();
    // 
    // Zoom based on mouse wheel
    float wheel = GetMouseWheelMove();
    if (wheel != 0)
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

    ImGuiIO* io = ImGui_GetIO();
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


void DrawEditor() {
    Vector2 o = GetWorldToScreen2D((Vector2){0,0}, worldCamera);
    Vector2 offset = (Vector2) {
        o.x - gridSize*worldCamera.zoom*ceil(o.x/(gridSize*worldCamera.zoom)),
        o.y - gridSize*worldCamera.zoom*ceil(o.y/(gridSize*worldCamera.zoom))};
    float guiGridSize = gridSize*worldCamera.zoom;
    float guiScreenWidth = screenWidth - offset.x;
    float guiScreenHeight = screenHeight - offset.y;
    for(int x = 0; x < (int)(guiScreenWidth / guiGridSize); ++x) {
        DrawLine(offset.x + x*guiGridSize, offset.y, offset.x + x*guiGridSize, offset.y+guiScreenHeight, GRAY);
    }
    for(int y = 0; y < (int)(guiScreenHeight / guiGridSize); ++y) {
        DrawLine(offset.x, offset.y + y*guiGridSize, offset.x+guiScreenWidth, offset.y + y * guiGridSize, GRAY);
    }

    BeginMode2D(worldCamera);
    DrawRectangleLinesEx((Rectangle){0,0,currentRoom->width*gridSize,currentRoom->height*gridSize},2, RED);
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
        ResolveIK(ecs,100);
        IKLegSystem(ecs,currentRoom);
        ResolveRoomCollisions(ecs, currentRoom);
        PlayerSlopes(ecs, currentRoom);
    }
    if(IsKeyDown(KEY_LEFT_CONTROL) && IsKeyPressed(KEY_L)) {
        sprintf(roomName, "./assets/rooms/room%d",roomID);
        currentRoom = LoadRoom(arena, roomName);
        ApplyRoom(&ecs, currentRoom);
    }
    if(IsKeyDown(KEY_LEFT_CONTROL) && IsKeyPressed(KEY_S)) {
        currentRoom->entityData = ecs;
        SaveRoom(currentRoom, roomName);
    }

    BeginUI();
    EntityPanel(arena,ecs, &inspect, IsKeyDown(KEY_LEFT_CONTROL) && IsKeyPressed(KEY_E));
    if(editorEnabled) {
        UpdateEditor();
        ImGui_Begin("prefab menu", NULL, ImGuiWindowFlags_None);
        if(inspect != NULL_ENTITY) {
            if(ImGui_Button("Save Prefab")) {
                ECS* entity = MakePrefab(arena, ecs, inspect);
                SaveEntitiesToFile(entity, "./assets/prefabs/test");

            }
        }
        if(ImGui_Button("Load Prefab")) {
            ECS* test = InitECS(arena,10, COMPONENT_COUNT);
            RegisterComponents(test, arena);
            LoadEntitiesFromFile(test, arena, "./assets/prefabs/test");
            MergePrefab(ecs, test);

        }
        ImGui_End();
    }
    EndUI();


    // ---------------------------------------------------------------------------------
    // Draw
    // ---------------------------------------------------------------------------------
    BeginDrawing();
        ClearBackground(RAYWHITE);
        BeginMode2D(worldCamera);
        Texture2D* tileset = GetAsset("TempTileset");
        DrawRoomTiles(currentRoom, tileset);
        for(int i = 0; i < ecs->blocks[DEBUG_SHAPE_COMPONENT].count; ++i) {
            Entity e = GetEntity(ecs, DEBUG_SHAPE_COMPONENT, i);
            if(!HasComponent(ecs, e, HITBOX_COMPONENT)) continue;
            Hitbox* hb = GetComponent(ecs, e, HITBOX_COMPONENT);
            DebugShape d = IndexComponent(ecs, DebugShape,DEBUG_SHAPE_COMPONENT, i);
            DrawRectangleRec(HitboxToRect(*hb), d.col);
        }
        EndMode2D();
        if(editorEnabled) {
            DrawEditor();
        }
        DrawUI();
    EndDrawing();
}
