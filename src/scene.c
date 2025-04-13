#include "scene.h"

void scene_init(Scene *scene) {
    memset(scene, 0, sizeof(Scene));
}

Entity *scene_create_entity(Scene *scene, Model *model) {
    if (scene->count >= MAX_ENTITIES) return (void *)0;  // No more space for new entities

    Entity *e = &scene->entities[scene->count++];
    e->id = scene->count - 1;
    e->model = model;
    e->transform.position[0] = e->transform.position[1] = e->transform.position[2] = 0.0f;
    e->transform.rotation[0] = e->transform.rotation[1] = e->transform.rotation[2] = 0.0f;
    e->transform.scale[0] = e->transform.scale[1] = e->transform.scale[2] = 1.0f;

    return e;
}
