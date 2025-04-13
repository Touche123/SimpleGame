#ifndef SCENE_H
#define SCENE_H

#include "entity.h"
#include "defines.h"

#define MAX_ENTITIES 1024

typedef struct Scene {
    Entity entities[MAX_ENTITIES];
    size_t count;
} Scene;

void scene_init(Scene* scene);
Entity* scene_create_entity(Scene* scene, Model* model);

#endif
