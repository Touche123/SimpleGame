#include "scene.h"

void scene_init(Scene *scene) {
    memset(scene, 0, sizeof(Scene));
}

Entity *scene_create_entity(Scene *scene, Model *model) {
    if (scene->entity_count >= MAX_ENTITIES) return (void *)0;  // No more space for new entities

    Entity *e = &scene->entities[scene->entity_count++];
    e->id = scene->entity_count - 1;
    e->model = model;
    e->transform.position[0] = e->transform.position[1] = e->transform.position[2] = 0.0f;
    e->transform.rotation[0] = e->transform.rotation[1] = e->transform.rotation[2] = 0.0f;
    e->transform.scale[0] = e->transform.scale[1] = e->transform.scale[2] = 1.0f;

    return e;
}

Light *scene_create_light(Scene *scene, vec3 position, vec3 color, float intensity) {
    if (scene->light_count >= MAX_LIGHTS) return NULL;  // No more space for new lights

    Light *light = &scene->lights[scene->light_count++];
    glm_vec3_copy(position, light->position);
    glm_vec3_copy(color, light->color);
    light->intensity = intensity;

    return light;
}