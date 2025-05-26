#ifndef SCENE_H
#define SCENE_H

#include "core.h"
#include "entity.h"

#define MAX_ENTITIES 1024
#define MAX_LIGHTS 64

typedef struct {
    vec3 position;
    vec3 color;
    float intensity;
} Light;

typedef struct Scene {
    Entity entities[MAX_ENTITIES];
    size_t entity_count;

    Light lights[MAX_LIGHTS];
    size_t light_count;
} Scene;

void scene_init(Scene* scene);
Entity* scene_create_entity(Scene* scene, Model* model);
Light* scene_create_light(Scene* scene, vec3 position, vec3 color, float intensity);

#endif
