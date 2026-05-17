#ifndef H_SPRITE
#define H_SPRITE

#include "../component.h"

// "TextureAsset" struct that wraps the key of an asset, and UI for displaying/serializing textureassets
// + rudimentary "Sprite" component (probably not used)

typedef struct TextureAsset {
    int length;
    char* key;
} TextureAsset;

#define SPRITE(X)\
    X(TextureAsset, texture)\
    X(bool, flipped)

void _Save_TextureAsset(FILE* stream, void* value);
void _Load_TextureAsset(FILE* stream, Arena* arena, void* value);
void _UI_TextureAsset(char* name, void* value);
void _Allocate_TextureAsset(Arena* arena, void* value);

#endif
