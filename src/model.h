#ifndef MODEL_H
#define MODEL_H

#include <cglm/cglm.h>

#include "mesh.h"

typedef struct {
    Mesh* meshes;
    size_t mesh_count;
} Model;

void model_load(Model* model, const char* filename);
// void model_save(Model* model, const char* filename);
bool model_load_cached_material_from_file(const char* filename, Material* material);
#endif
