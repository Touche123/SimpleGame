#ifndef ENTITY_H
#define ENTITY_H

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

#endif