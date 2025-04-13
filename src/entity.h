#ifndef ENTITY_H
#define ENTITY_H

#include "model.h"

typedef struct {
    float position[3];
    float rotation[3];
    float scale[3];
} Transform;

typedef struct {
    int id;
    Model* model;
    Transform transform;
} Entity;

#endif