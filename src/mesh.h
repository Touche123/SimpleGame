#ifndef MESH_H
#define MESH_H

#include "core.h"
#include "glad.h"
#include "material.h"
#include "shader.h"

typedef struct Vertex {
    vec3 position;
    vec3 normal;
    vec2 texcoord;
    vec3 tangent;
    vec3 bitangent;
} Vertex;

typedef struct Mesh {
    GLuint vao, vbo, ebo;
    float* vertices;
    size_t vertex_count;
    uint16_t* indices;
    size_t index_count;
    Material material;
} Mesh;

void mesh_init(Mesh* mesh, Vertex* vertices, uint16_t* indices, size_t vertex_count, size_t index_count);
void mesh_apply_material(Mesh* mesh, Shader* shader);
#endif
