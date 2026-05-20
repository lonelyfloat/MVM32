#include <stdlib.h>
#include <stdio.h>
#include "game/program.h"
#include "ui/ui.h"
#include "assets.h"
#include "raylib/raylib.h"

#if defined(__EMSCRIPTEN__)
    #include <emscripten/emscripten.h>
#endif

#if defined(DYNAMIC_LIB)
    #include <dlfcn.h>
#endif

#include "reload.h"

const int initialScreenWidth = 800;
const int initialScreenHeight = 600;

ReloadCallbacks calls;

Arena* gameArena;

int main(void) {
    InitReloadCallbacks(calls, Init, UpdateDrawFrame, Save, Load);
    #if defined(DYNAMIC_LIB)
        SetCompileInfo("./build/game.so", "make reload-lib");
        int r = ReloadCode(&calls);
        if(r) return r;
    #else 
        calls.init = Init;
        calls.update = UpdateDrawFrame;
        calls.save = Save;
        calls.load = Load;
    #endif

    // For now, 10MB of memory
    gameArena = CreateArena(10000000);

    InitWindow(initialScreenWidth, initialScreenHeight, "clamber game");
    calls.init(gameArena);
    InitUI();

    #if defined(__EMSCRIPTEN__)
        emscripten_set_main_loop(UpdateDrawFrame, 0, 1);
    #else
    SetTargetFPS(60);   

    while (!WindowShouldClose())    
    {
    #if defined(DYNAMIC_LIB)
        if(IsKeyDown(KEY_LEFT_CONTROL) && IsKeyPressed(KEY_R)) {
            HotReload(&calls, gameArena, "./assets/runtime_game_data");
        }
    #endif
        calls.update();
    }
    #endif
    CloseWindow();        
    printf("Closing game...\n");
    DestroyArena(gameArena);
    DestroyUI();
    return 0;
}
