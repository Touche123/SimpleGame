#include "mesh.h"

#include <stddef.h>

#define CGLTF_IMPLEMENTATION
#include "cgltf.h"

void mesh_load(Mesh *mesh, const char *filename) {
    cgltf_options options = {0};
    cgltf_data *data = NULL;
    cgltf_result result = cgltf_parse_file(&options, filename, &data);
    if (result != cgltf_result_success) {
        fprintf(stderr, "Failed to parse glTF file\n");
    }

    result = cgltf_load_buffers(&options, data, filename);
    if (result != cgltf_result_success) {
        fprintf(stderr, "Failed to load buffers\n");
        cgltf_free(data);
    }
    result = cgltf_validate(data);

    cgltf_mesh *cgltf_mesh = &data->meshes[0];
    cgltf_primitive *prim = &cgltf_mesh->primitives[0];

    // Loopa igenom alla material i glTF-filen
    for (size_t i = 0; i < data->materials_count; i++) {
        cgltf_material *material = &data->materials[i];

        // Hämta PBR-materialet
        cgltf_pbr_metallic_roughness *pbr = &material->pbr_metallic_roughness;

        mesh->material.base_color_factor[0] = pbr->base_color_factor[0];
        mesh->material.base_color_factor[1] = pbr->base_color_factor[1];
        mesh->material.base_color_factor[2] = pbr->base_color_factor[2];
        mesh->material.base_color_factor[3] = pbr->base_color_factor[3];

        if (pbr->metallic_factor) {
            printf("Metallic Factor: %.2f\n", pbr->metallic_factor);
        }

        if (pbr->roughness_factor) {
            printf("Roughness Factor: %.2f\n", pbr->roughness_factor);
        }

        // Om du har fler egenskaper som normalmaps, occlusion maps etc. kan du också läsa in dem
    }

    size_t vertex_count = prim->attributes[0].data->count;
    Vertex *vertices = malloc(sizeof(Vertex) * vertex_count);

    for (size_t i = 0; i < prim->attributes_count; ++i) {
        cgltf_attribute *attr = &prim->attributes[i];
        cgltf_accessor *accessor = attr->data;
        float *buffer = (float *)((uint8_t *)accessor->buffer_view->buffer->data + accessor->buffer_view->offset + accessor->offset);

        for (size_t v = 0; v < accessor->count; ++v) {
            if (strcmp(attr->name, "POSITION") == 0) {
                memcpy(vertices[v].position, &buffer[v * 3], sizeof(float) * 3);
            } else if (strcmp(attr->name, "NORMAL") == 0) {
                memcpy(vertices[v].normal, &buffer[v * 3], sizeof(float) * 3);
            } else if (strcmp(attr->name, "TEXCOORD_0") == 0) {
                memcpy(vertices[v].texcoord, &buffer[v * 2], sizeof(float) * 2);
            }
        }
    }

    cgltf_accessor *index_accessor = prim->indices;
    uint16_t *indices = (uint16_t *)((uint8_t *)index_accessor->buffer_view->buffer->data +
                                     index_accessor->buffer_view->offset + index_accessor->offset);

    //GLuint texture = 0;
    //if (data->images_count > 0) {
    //    cgltf_image *img = &data->images[0];
    //    int w, h, comp;
    //    unsigned char *image_data = stbi_load(img->uri, &w, &h, &comp, 4);

    //    glGenTextures(1, &texture);
    //    glBindTexture(GL_TEXTURE_2D, texture);
    //    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, image_data);
    //    glGenerateMipmap(GL_TEXTURE_2D);

    //    stbi_image_free(image_data);
    //}
    //if (texture == 0) {
    //    unsigned char white_pixel[] = {255, 255, 255, 255};
    //    glGenTextures(1, &texture);
    //    glBindTexture(GL_TEXTURE_2D, texture);
    //    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, white_pixel);
    //}

    //mesh->texture = texture;

    size_t index_count = index_accessor->count;
    // cgltf_free(data);
    //  free(vertices);
    mesh_init(mesh, vertices, indices, vertex_count, index_count);
}

void mesh_init(Mesh *mesh, Vertex *vertices, uint16_t *indices, size_t vertex_count, size_t index_count) {
    glGenVertexArrays(1, &mesh->vao);
    glGenBuffers(1, &mesh->vbo);
    glGenBuffers(1, &mesh->ebo);

    glBindVertexArray(mesh->vao);

    // VBO - vertex buffer
    glBindBuffer(GL_ARRAY_BUFFER, mesh->vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * vertex_count, vertices, GL_STATIC_DRAW);

    // EBO - element buffer
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint16_t) * index_count, indices, GL_STATIC_DRAW);

    // Position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, position));
    glEnableVertexAttribArray(0);

    // Normal attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, normal));
    glEnableVertexAttribArray(1);

    // Texcoord attribute
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, texcoord));
    glEnableVertexAttribArray(2);

    glBindVertexArray(0);

    // Store the index count for rendering
    mesh->index_count = index_count;
}
