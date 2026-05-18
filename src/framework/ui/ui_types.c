#include "ui_types.h"
#include "ui.h"
#include "float.h"

void _UI_float(char* name, void* value) {
    ImGui_TextUnformatted(name);
    ImGui_SameLine();
    char buf[32];
    sprintf(buf, "##%s", name);
    ImGui_InputFloat(buf,value);
}

void _UI_int(char* name, void* value) {
    ImGui_TextUnformatted(name);
    ImGui_SameLine();
    char buf[32];
    sprintf(buf, "##%s", name);
    ImGui_InputInt(buf, value);
}

void _UI_bool(char* name, void* value) {
    ImGui_TextUnformatted(name);
    ImGui_SameLine();
    char buf[32];
    sprintf(buf, "##%s", name);
    ImGui_Checkbox(buf, value);
}

void _UI_Entity(char* name, void* value) {
    Entity* e = value;
    int e2[2] = { GetID(*e), GetVersion(*e) };
    ImGui_TextUnformatted(name);
    ImGui_SameLine();
    char buf[32];
    sprintf(buf, "##%s##Table", name);
    ImGui_PushID(name);
    ImGui_InputInt2("##EntityID", e2, ImGuiInputTextFlags_None);
    ImGui_PopID();
    if(e2[0] < 0 || e2[1] < 0) {
        *e = NULL_ENTITY;
    } else {
        *e = ConstructEntity(e2[0], e2[1]);
    }
}
