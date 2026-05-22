#ifndef H_PREFAB
#define H_PREFAB
#include "component_types.h"

// System for "prefabs" aka sets of entities related to eachother so they can be saved/loaded from files easily / more than once
void SaveEntityTree(ECS* ecs, Entity e, const char* filePath);
void LoadEntityTree(ECS* ecs, Arena* arena, const char* filePath);

ECS* MakePrefab(Arena* arena, ECS* ecs, Entity e);
void MergePrefab(ECS* world, ECS* prefab);

#endif
