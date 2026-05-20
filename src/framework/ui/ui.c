#include "ui.h"
#include <float.h>
#include "raylib/raylib.h"
#include <string.h>
#include "assets.h"
#include <stdio.h>
#include "imgui_impl_raylib.h"

ImGuiContext* g_UI_ctx;
Texture2D magentaTex;

void InitUI() {
    g_UI_ctx = ImGui_CreateContext(NULL);
    ImGuiIO* io = ImGui_GetIO(); 
    (void)io;
    io->ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io->ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
    io->IniFilename = "build/imgui.ini";
    // tell ImGui the display scale
    if (!IsWindowState(FLAG_WINDOW_HIGHDPI))
    {
        io->DisplayFramebufferScale.x = GetWindowScaleDPI().x;
        io->DisplayFramebufferScale.y = GetWindowScaleDPI().y;
    }
    ImGui_StyleColorsDark(NULL);
    ImGuiStyle* style = ImGui_GetStyle();
    style->FontScaleDpi = 1.0;

    ImGui_ImplRaylib_Init();


}

ImGuiContext* GetImGuiContext() {
    return g_UI_ctx;
}

void BeginUI() {
    ImGui_ImplRaylib_NewFrame();
    ImGui_NewFrame();
}

void EndUI() {
    ImGui_Render();
}

void DrawUI() {
    ImGui_ImplRaylib_RenderDrawData(ImGui_GetDrawData());
}

void DestroyUI() {
    ImGui_ImplRaylib_Shutdown();
    ImGui_DestroyContext(g_UI_ctx);
}

// Jank alert !!!! 
static void AddEmptyComponent(Arena* arena, ECS* ecs, Entity e, ComponentIndex i) {
    ecs->allocHooks[i](arena,ecs->blocks[i].components + ecs->blocks[i].count*ecs->blocks[i].componentSize);
    ecs->blocks[i].entities[ecs->blocks[i].count] = e;
    ecs->blocks[i].indices[GetID(e)] = ecs->blocks[i].count;
    ecs->blocks[i].count += 1;
}

void EntityInspector(Arena* arena, ECS* ecs, Entity* inspected) {
    bool show = *inspected != NULL_ENTITY;
    if(show) {
        if(!show) *inspected = NULL_ENTITY;
        if(show) {
            ImVec2 region = (ImVec2){ImGui_GetContentRegionAvail().x, ImGui_GetContentRegionAvail().y};
            ImGui_BeginChild("PanelR", region, ImGuiChildFlags_Borders, ImGuiWindowFlags_None);
            if(ImGui_BeginCombo("##EntityInspectorCombo", "Add Component", ImGuiComboFlags_None)) {
                ImGuiTextFilter filter = {};
                 if (ImGui_IsWindowAppearing()) {
                    ImGui_SetKeyboardFocusHere();
                    ImGuiTextFilter_Clear(&filter);
                }
                int selectedComponentIdx = -1;
                ImGuiTextFilter_Draw(&filter, "##EntityInspectorSearch", 0.0);
                for(int i = 0; i < ecs->componentCount; ++i) {
                    if(HasComponent(ecs, *inspected, i)) continue;
                    bool selected = false;
                    if(i == selectedComponentIdx) selected = true;
                    int len = strlen(ecs->componentStrings[i]);
                    if(ImGuiTextFilter_PassFilter(&filter, ecs->componentStrings[i], (ecs->componentStrings[i] + len - 1))) {
                        if(ImGui_Selectable(ecs->componentStrings[i])) {
                            selectedComponentIdx = i;
                        }
                    }
                    if(selected) ImGui_SetItemDefaultFocus();
                }
                if(selectedComponentIdx != -1) {
                    AddEmptyComponent(arena, ecs, *inspected, selectedComponentIdx); 
                }
                ImGui_EndCombo();
            }
            for(int i = 0; i < ecs->componentCount; ++i) {
                if(HasComponent(ecs, *inspected, i)) {
                    ecs->uiHooks[i](GetComponent(ecs, *inspected, i));
                    ImGui_PushIDInt(i);
                    if(ImGui_Button("Remove")) {
                        RemoveComponent(ecs, *inspected, i);
                    }
                    ImGui_PopID();
                }
            }
        }
        ImGui_EndChild();
    }
}

static bool showEntityPanel = false;

void EntityPanel(Arena* arena, ECS* ecs, Entity* inspected, bool togglePanel) {
    if(togglePanel) showEntityPanel = !showEntityPanel;
    if(showEntityPanel) {
        float killButtonSpace = ImGui_CalcTextSize("Kill").x +
                                ImGui_GetStyle()->FramePadding.x + 
                                ImGui_GetStyle()->ItemSpacing.x + 
                                ImGui_GetStyle()->WindowPadding.x*2;
        float cloneButtonSpace = ImGui_CalcTextSize("Clone").x;
        ImGui_Begin("Entity Panel", &showEntityPanel, ImGuiWindowFlags_NoSavedSettings);
        char buf[100];
        Entity queried = NULL_ENTITY;
        float mult = 0.5;
        ImVec2 cr = (ImVec2){ImGui_GetContentRegionAvail().x, ImGui_GetContentRegionAvail().y};
        ImGui_SetWindowSize((ImVec2){400,300}, ImGuiCond_FirstUseEver);
        ImVec2 region = (ImVec2){cr.x*mult, cr.y};
        ImGui_BeginChild("ListPanel",region,ImGuiChildFlags_Borders | ImGuiChildFlags_ResizeX, ImGuiWindowFlags_None);
        cr = (ImVec2){ImGui_GetContentRegionAvail().x, ImGui_GetContentRegionAvail().y};
        region = (ImVec2){cr.x, cr.y};
        if(ImGui_Button("New")) {
            CreateEntity(ecs);
        }
        if(ImGui_BeginTable("EntityListTable", 2, ImGuiTableFlags_RowBg)) {
            for(int i = 0; i < ecs->totalEntities; ++i) {
                if(i != GetID(ecs->entities[i])) {
                    continue;
                }
                if(i == 0) {
                    ImGui_TableSetupColumnEx("##LeftColumn", ImGuiTableColumnFlags_WidthFixed, region.x-killButtonSpace-cloneButtonSpace,0); // Default to 100.0f
                }
                ImGui_TableNextColumn();
                sprintf(buf, "%d|v%d", GetID(ecs->entities[i]), GetVersion(ecs->entities[i]));
                ImGui_AlignTextToFramePadding();
                ImGui_PushIDInt(i);
                if(ImGui_Selectable(buf)) {
                    queried = ecs->entities[i];
                }
                ImGui_PopID();
                ImGui_SetNextItemWidth(-FLT_MIN);
                ImGui_TableNextColumn();
                ImGui_PushIDInt(i);
                if(ImGui_Button("Kill")) {
                    KillEntity(ecs, ecs->entities[i]);
                }
                ImGui_SameLine();
                if(ImGui_Button("Copy")) {
                    Entity copied = ecs->entities[i];
                    Entity new = CreateEntity(ecs);
                    for(int i = 0; i < ecs->componentCount; ++i) {
                        if(!HasComponent(ecs, copied, i)) continue;
                        void* oldData = GetComponent(ecs, copied, i);
                        AddEmptyComponent(arena, ecs, new, i);
                        void* newData = GetComponent(ecs, new, i);
                        memcpy(newData, oldData, ecs->blocks[i].componentSize);
                    }
                }
                ImGui_PopID();
            }
            if(*inspected == queried) {
                *inspected = NULL_ENTITY;

            }
            else if(queried != NULL_ENTITY) {
                *inspected = queried;
            }
            ImGui_EndTable();
        }
        ImGui_EndChild();
        ImGui_SameLine();
        EntityInspector(arena, ecs, inspected);
        ImGui_End();
    }
}

