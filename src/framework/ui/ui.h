#ifndef H_UI
#define H_UI
#include "dear_bindings/dcimgui.h"
#include "ecs.h"

void InitUI();
void BeginUI();
void EndUI();
void DrawUI();
void DestroyUI();

void EntityInspector(Arena* arena, ECS* ecs, Entity* inspected);
ImGuiContext* GetImGuiContext();
void EntityPanel(Arena* arena, ECS* ecs, Entity* inspected, bool togglePanel);

void* GetBackgroundTexture();

#endif
