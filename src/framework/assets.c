#include <raylib/raylib.h>
#include <stdio.h>
#include <string.h>
#include "assets.h"

static AssetRegistry g_Assets;

void InitAssets(Arena* arena, size_t capacity) {
    g_Assets.assets = ArenaAlloc(arena, sizeof(AssetData)*capacity);
    g_Assets.keys = ArenaAlloc(arena, sizeof(char*) * capacity);
    g_Assets.count = 0;
    g_Assets.capacity = capacity;
    g_Assets.arena = arena;
}

// FNV1a hash algorithm
const size_t BASE = 0x811c9dc5;
const size_t PRIME = 0x01000193;
static size_t hash(char *str) {
    size_t initial = BASE;
    while(*str) {
        initial ^= *str++;
        initial *= PRIME;
    }
    return initial;
}

void* LoadAsset(char* path, char* key, AssetType type) {
    void* result = NULL;
    switch(type) {
        case ASSET_TYPE_TEXTURE:
            {
                Texture2D tex = LoadTexture(path);
                result = ArenaAlloc(g_Assets.arena, sizeof(Texture2D));
                memcpy(result, &tex, sizeof(Texture2D));
            }
            break;
        case ASSET_TYPE_SOUND: 
            {
                Sound sound = LoadSound(path);
                result = ArenaAlloc(g_Assets.arena, sizeof(Sound));
                memcpy(result, &sound, sizeof(Sound));
            }
            break;
        default:
            printf("ERROR: Unexpected AssetType in LoadAsset function\n");
            break;
    }
    g_Assets.assets[hash(key) % g_Assets.capacity] = (AssetData) {
        .data = result,
        .type = type,
        .index = g_Assets.count
    };
    g_Assets.keys[g_Assets.count] = ArenaPushString(g_Assets.arena, key);
    g_Assets.count++;
    return result;
}

void* GetAsset(char* key) {
    if(key == NULL) return NULL;
    return g_Assets.assets[hash(key)% g_Assets.capacity].data;
}

AssetData* GetAssetData(char* key) {
    return &g_Assets.assets[hash(key)% g_Assets.capacity];
}

void UnloadAsset(char* key) {
    AssetData asset = g_Assets.assets[hash(key) % g_Assets.capacity];
    g_Assets.keys[asset.index] = g_Assets.keys[g_Assets.count-1];
    switch(asset.type) {
        case ASSET_TYPE_TEXTURE:
            UnloadTexture(*((Texture2D*)asset.data));
            break;
        case ASSET_TYPE_SOUND:
            UnloadSound(*((Sound*)asset.data));
            break;
        default: 
            printf("ERROR: Unexpected AssetType in UnloadAsset function\n");
            break;
    }

    g_Assets.count--;
}

void UnloadAllAssets() {
    while(g_Assets.count > 0) {
        UnloadAsset(g_Assets.keys[0]);
    }
}

AssetRegistry* GetAssetRegistry() {
    return &g_Assets;
}
