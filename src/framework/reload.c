#include <stdlib.h>
#include <stdio.h>
#include <dlfcn.h>
#include "reload.h"
#include "assets.h"

static void* handle = NULL;

#define CALL(dest, name, h) do { \
        dest = dlsym(h,name);\
        if(!dest) {\
            printf("error: %s\n", dlerror());\
            return 2;\
        }\
        fprintf(stderr, "Got symlink %s -> %p\n",name, dest);\
} while(0)\

static char* g_sharedLibrary = NULL;
static char* g_compileCmd = NULL;

void SetCompileInfo(char* sharedLibrary, char* compileCmd) {
    g_sharedLibrary = sharedLibrary;
    g_compileCmd = compileCmd;
}

int ReloadCode(ReloadCallbacks* calls) {
        if(handle) {
            fprintf(stderr, "Closing DLL Handle %p\n", handle);
            calls->init = NULL;
            calls->update = NULL;
            dlclose(handle);
            handle = NULL;
        }
        handle = dlopen(g_sharedLibrary, RTLD_NOW);
        if(!handle) {
            printf("error: %s\n", dlerror());
            return 1;
        } else {
            printf("Opening DLL Handle %p\n", handle);
        }
        CALL(calls->init, calls->names[RELOAD_CALLBACK_INIT], handle);
        CALL(calls->update, calls->names[RELOAD_CALLBACK_UPDATE], handle);
        CALL(calls->save, calls->names[RELOAD_CALLBACK_SAVE], handle);
        CALL(calls->load, calls->names[RELOAD_CALLBACK_LOAD], handle);
        return 0;
}

int HotReload(ReloadCallbacks* calls, Arena* arena, char* tempFile) {
    int err = system(g_compileCmd);
    if(!err) {
        calls->save(tempFile);
        int r = ReloadCode(calls);
        if(r) return r;
        printf("Unloading assets...\n");
        UnloadAllAssets();
        printf("Resetting memory region...\n");
        ArenaReset(arena);
        calls->init(arena);
        calls->load(tempFile);
    }
    return 0;
}
