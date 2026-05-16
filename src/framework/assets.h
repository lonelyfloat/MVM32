#ifndef H_ASSETS
#define H_ASSETS

#include <stdlib.h>
#include <arena.h>

typedef enum AssetType {
    ASSET_TYPE_TEXTURE,
    ASSET_TYPE_SOUND,
    ASSET_TYPE_COUNT
} AssetType;

typedef struct AssetData {
    void* data;
    AssetType type;
    int index;
} AssetData;

typedef struct AssetRegistry {
    // Arena ptr for data
    Arena* arena;
    // Hash map of asset info
    AssetData* assets;
    // Stack of valid keys
    char** keys;

    size_t count;
    size_t capacity;
} AssetRegistry;

void InitAssets(Arena* arena, size_t capacity);
AssetRegistry* GetAssetRegistry();

void* LoadAsset(char* path, char* key, AssetType type);
void* GetAsset(char* key);

AssetData* GetAssetData(char* key);

void UnloadAsset(char* key);
void UnloadAllAssets();

#endif
