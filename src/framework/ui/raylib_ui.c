#include "raylib_ui.h"
#include "dear_bindings/dcimgui.h"
#include <stdio.h>

void _UI_Vector2(char* name, void* value) {
    Vector2* vec2 = value;
    ImGui_PushID(name);
    ImGui_TextUnformatted(name);
    ImGui_SameLine();
    ImGui_TextUnformatted("x");
    ImGui_SameLine();
    ImGui_InputFloat("##ElemX",&vec2->x);
    ImGui_TextUnformatted("y");
    ImGui_SameLine();
    ImGui_InputFloat("##ElemY",&vec2->y);
    ImGui_PopID();
}


void _UI_Rectangle(char* name, void* value) {
    Rectangle* rec = value;
    float pos[2] = {rec->x, rec->y};
    ImGui_InputFloat2(name, pos);
    float size[2] = {rec->width, rec->height};
    ImGui_PushID(name);
    ImGui_InputFloat2("##RectangleSize", size);
    ImGui_PopID();
    rec->x = pos[0];
    rec->y = pos[1];
    rec->width = size[0];
    rec->height = size[1];
}

void _UI_Color(char* name, void* value) {
    Color* col = value;
    Vector4 colNormal = ColorNormalize(*col);
    float colFloat[4] = {colNormal.x, colNormal.y, colNormal.z, colNormal.w};
    ImGui_PushID(name);
    ImGui_TextUnformatted(name);
    ImGui_SameLine();
    ImGui_ColorEdit4("##color", colFloat, ImGuiColorEditFlags_None);
    colNormal.x = colFloat[0];
    colNormal.y = colFloat[1];
    colNormal.z = colFloat[2];
    colNormal.w = colFloat[3];
    *col = ColorFromNormalized(colNormal);
    ImGui_PopID();
}
