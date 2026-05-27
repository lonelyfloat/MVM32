#ifndef H_PREFAB
#define H_PREFAB
#include "component_types.h"

// System for "prefabs" aka sets of entities related to eachother so they can be saved/loaded from files easily / more than once
void SaveEntityTree(ECS* ecs, Entity e, const char* filePath);
void LoadEntityTree(ECS* ecs, Arena* arena, const char* filePath);

void MakePrefab(ECS* prefab, Arena* arena, ECS* ecs, Entity e);
void MergePrefab(ECS* world, ECS* prefab);

// Kills an entity and all of its children
void KillPrefab(ECS* world, Entity e);

#endif
