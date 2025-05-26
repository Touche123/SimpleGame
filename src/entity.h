#ifndef ENTITY_H
#define ENTITY_H

#include <cglm/cglm.h>

#include "model.h"

typedef struct {
    vec3 position;
    vec3 rotation;
    vec3 scale;
} Transform;

typedef struct {
    int id;
    Model* model;
    Transform transform;
} Entity;

void entity_get_model_matrix(Entity* e, mat4 out);
#endif