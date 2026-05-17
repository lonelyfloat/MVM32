#include "sprite.h"
#include <string.h>
#include <math.h>
#include <raylib/raylib.h>
#include "../assets.h"

void _Save_TextureAsset(FILE* stream, void* value) {
    TextureAsset* asset = value;
    fprintf(stream, "%d|", asset->length);
    for(int i = 0; i < asset->length; ++i) {
        fprintf(stream, "%c", asset->key[i]);
    }
    fprintf(stream, " ");
}

void _Load_TextureAsset(FILE* stream, Arena* arena, void* value) {
    TextureAsset* asset = value;
    fscanf(stream, "%d|", &asset->length);
    asset->key = ArenaAlloc(arena, asset->length+1);
    for(int i = 0; i < asset->length; ++i) {
        fscanf(stream, "%c", &asset->key[i]);
    }
    asset->key[asset->length] = '\0';
    fscanf(stream, " ");
}

#define MAX_LENGTH 300 

void _Allocate_TextureAsset(Arena* arena, void* value) {
    TextureAsset* asset = value;
    asset->length = 0;
    asset->key = ArenaAlloc(arena, MAX_LENGTH);
    asset->key[0] = '\0';
}

float min(float a, float b) {
    return a < b ? a : b;
}

const float maxTextureSize = 200;

void _UI_TextureAsset(char* name, void* value) {
    TextureAsset* asset = value;
    bool noKey = asset->key == NULL || asset->length <= 0;
    Texture2D* tex;
    float texWidth = 1.0;
    if(!noKey) {
        tex = GetAsset(asset->key);
        texWidth = tex->width;
    }
    ImVec2 cursorPos = ImGui_GetCursorPos();
    const float targetWidth = min(maxTextureSize, ImGui_GetContentRegionAvail().x * 2/3);
    float scale = targetWidth/texWidth;
    ImGui_ColorButtonEx("##TextureAsset_ColorBG", (ImVec4){1.0,0.0,1.0,1.0},
        ImGuiColorEditFlags_NoLabel | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoTooltip,
        (ImVec2){scale * texWidth, scale * texWidth});

    if(!noKey) {
        float verticalPadding = fabs(scale*tex->height - scale*texWidth) / 2;
        cursorPos.y += verticalPadding;
        ImGui_SetCursorPos(cursorPos);
        ImGui_ImageWithBg((ImTextureRef){._TexID = tex->id}, (ImVec2){scale * texWidth, scale * tex->height});
        cursorPos = ImGui_GetCursorPos();
        cursorPos.y += verticalPadding;
        ImGui_SetCursorPos(cursorPos);
    }

    if(ImGui_BeginCombo("##TextureAssetSearchCombo", asset->key, ImGuiComboFlags_None)) {
        ImGuiTextFilter filter = {};
         if (ImGui_IsWindowAppearing()) {
            ImGui_SetKeyboardFocusHere();
            ImGuiTextFilter_Clear(&filter);
        }
        int selectedKeyIdx = -1;
        AssetRegistry* registry = GetAssetRegistry();
        ImGuiTextFilter_Draw(&filter, "##TextureAssetSearch", 0.0);
        for(int i = 0; i < registry->count; ++i) {
            if(registry->assets[i].type != ASSET_TYPE_TEXTURE) continue;
            int len = strlen(registry->keys[i]);
            if(ImGuiTextFilter_PassFilter(&filter, registry->keys[i], (registry->keys[i] + len - 1))) {
                if(ImGui_Selectable(registry->keys[i])) {
                    selectedKeyIdx = i;
                }
            }
        }
        if(selectedKeyIdx != -1) {
            asset->key = registry->keys[selectedKeyIdx];
            asset->length = strlen(registry->keys[selectedKeyIdx]);
        }
        ImGui_EndCombo();
    }
}
