#include "raylib_ui.h"
#include "dear_bindings/dcimgui.h"
#include <stdio.h>

void _UI_Vector2(char* name, void* value) {
    Vector2* vec2 = value;
    ImGui_PushID(name);
    ImGui_TextUnformatted("x");
    ImGui_SameLine();
    ImGui_InputFloat("##ElemX",&vec2->x);
    ImGui_SameLine();
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
    ImGui_InputFloat2("##RectangleSize", pos);
    ImGui_PopID();
    rec->x = pos[0];
    rec->y = pos[1];
    rec->width = size[0];
    rec->height = size[1];
}
