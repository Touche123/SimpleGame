#include "mesh.h"

#include <stddef.h>

#define CGLTF_IMPLEMENTATION
#include "cgltf.h"

void mesh_init(Mesh *mesh, Vertex *vertices, uint16_t *indices, size_t vertex_count, size_t index_count) {
    glGenVertexArrays(1, &mesh->vao);
    glGenBuffers(1, &mesh->vbo);
    glGenBuffers(1, &mesh->ebo);

    glBindVertexArray(mesh->vao);

    glBindBuffer(GL_ARRAY_BUFFER, mesh->vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * vertex_count, vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint16_t) * index_count, indices, GL_STATIC_DRAW);

    // Position
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, position));
    glEnableVertexAttribArray(0);
    // Normal
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, normal));
    glEnableVertexAttribArray(1);
    // Texcoord
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, texcoord));
    glEnableVertexAttribArray(2);
    // Tangent
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, tangent));
    glEnableVertexAttribArray(3);
    // Bitangent
    glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, bitangent));
    glEnableVertexAttribArray(4);

    glBindVertexArray(0);

    mesh->vertex_count = vertex_count;
    mesh->index_count = index_count;

    mesh->vertices = malloc(sizeof(Vertex) * mesh->vertex_count);
    memcpy(mesh->vertices, vertices, sizeof(Vertex) * mesh->vertex_count);

    mesh->indices = malloc(sizeof(uint16_t) * mesh->index_count);
    memcpy(mesh->indices, indices, sizeof(uint16_t) * mesh->index_count);
}

void mesh_apply_material(Mesh *mesh, Shader *shader) {
    shader_uniform1i(shader, "useAlbedoMap", mesh->material.albedoTexture ? 1 : 0);
    shader_uniform1i(shader, "useMetallicMap", mesh->material.metallicTexture ? 1 : 0);
    shader_uniform1i(shader, "useRoughnessMap", mesh->material.roughnessTexture ? 1 : 0);
    shader_uniform1i(shader, "useAoMap", mesh->material.aoTexture ? 1 : 0);
    shader_uniform1i(shader, "useNormalMap", mesh->material.useNormalMap ? 1 : 0);
    shader_uniform3fv(shader, "albedoColor", mesh->material.albedoColor);
    shader_uniform1f(shader, "metallicValue", mesh->material.metallicValue);
    shader_uniform1f(shader, "roughnessValue", mesh->material.roughnessValue);
    shader_uniform1f(shader, "aoValue", mesh->material.aoValue);

    if (mesh->material.albedoTexture ? 1 : 0) {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, mesh->material.albedoTexture);
        glUniform1i(glGetUniformLocation(shader->id, "albedoMap"), 0);
    }
    if (mesh->material.metallicTexture ? 1 : 0) {
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, mesh->material.metallicTexture);
        glUniform1i(glGetUniformLocation(shader->id, "metallicMap"), 1);
    }
    if (mesh->material.roughnessTexture ? 1 : 0) {
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, mesh->material.roughnessTexture);
        glUniform1i(glGetUniformLocation(shader->id, "roughnessMap"), 2);
    }
    if (mesh->material.aoTexture ? 1 : 0) {
        glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_2D, mesh->material.aoTexture);
        glUniform1i(glGetUniformLocation(shader->id, "aoMap"), 3);
    }
    if (mesh->material.useNormalMap) {
        glActiveTexture(GL_TEXTURE4);
        glBindTexture(GL_TEXTURE_2D, mesh->material.normalTexture);
        glUniform1i(glGetUniformLocation(shader->id, "normalMap"), 4);
        glUniform1i(glGetUniformLocation(shader->id, "useNormalMap"), 1);
    } else {
        glUniform1i(glGetUniformLocation(shader->id, "useNormalMap"), 0);
    }
}
