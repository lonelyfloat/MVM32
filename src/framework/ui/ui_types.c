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
