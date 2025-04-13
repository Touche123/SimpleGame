#ifndef MESH_H
#define MESH_H

#include "glad.h"
#include "defines.h"

typedef struct Vertex {
    float position[3];
    float normal[3];
    float texcoord[2];
} Vertex;

typedef struct Material {
    float base_color_factor[4];
    float metallic_factor;
    float roughness_factor;
} Material;

typedef struct Mesh {
    GLuint vao, vbo, ebo;
    size_t index_count;
    GLuint texture;
    Material material;
} Mesh;

void mesh_init(Mesh* mesh, Vertex* vertices, uint16_t* indices, size_t vertex_count, size_t index_count);
void mesh_load(Mesh* mesh, const char* filename);
#endif
