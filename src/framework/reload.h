#ifndef H_RELOAD
#define H_RELOAD

#include "arena.h"

#ifdef DYNAMIC_LIB 
#define P_STR(...) #__VA_ARGS__
#define STR(a) P_STR(a)
#define DYNAMIC_LIB_NAME STR(DYNAMIC_LIB)
#endif

typedef void (*MemoryCallback)(Arena*);
typedef void (*UpdateCallback)(void);
typedef void (*FileCallback)(char*);

typedef enum ReloadCallbackType {
    RELOAD_CALLBACK_INIT,
    RELOAD_CALLBACK_UPDATE,
    RELOAD_CALLBACK_SAVE,
    RELOAD_CALLBACK_LOAD,
    RELOAD_CALLBACK_COUNT
} ReloadCallbackType;

typedef struct ReloadCallbacks {
    MemoryCallback init;
    UpdateCallback update;
    FileCallback save, load;
    char* names[RELOAD_CALLBACK_COUNT];
} ReloadCallbacks;

#define InitReloadCallbacks(calls, _init, _update, _save, _load)\
    do {\
    (calls).names[RELOAD_CALLBACK_INIT] = #_init;\
    (calls).names[RELOAD_CALLBACK_UPDATE] = #_update;\
    (calls).names[RELOAD_CALLBACK_SAVE] = #_save;\
    (calls).names[RELOAD_CALLBACK_LOAD] = #_load;\
    } while(0)

void SetCompileInfo(char* sharedLibrary, char* compileCmd);
int ReloadCode(ReloadCallbacks* calls);
int HotReload(ReloadCallbacks* calls, Arena* arena, char* tempFile);

#endif
