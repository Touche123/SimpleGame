#include "model.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cgltf.h"
#include "mesh.h"
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

// void model_save(Model *model, const char *filename) {
// }

bool model_save_material_to_file(const char *filename, const Material *material) {
    printf("Saving material to file..\n");
    FILE *f;

    fopen_s(&f, filename, "w");
    if (!f) {
        printf("Failed to save material to file: %s", filename);
        return false;
    }

    fprintf(f, "base_color = %f, %f, %f\n",
            material->albedoColor[0],
            material->albedoColor[1],
            material->albedoColor[2]);

    fprintf(f, "metallic = %f\n", material->metallicValue);
    fprintf(f, "roughness = %f\n", material->roughnessValue);
    fclose(f);
    return true;
}

Material convert_from_cgltf(const cgltf_material *src) {
    Material mat = {0};

    const cgltf_pbr_metallic_roughness *pbr = &src->pbr_metallic_roughness;

    for (int i = 0; i < 3; ++i) {
        printf("pbr->base_color_factor[%d]: %f\n", i, pbr->base_color_factor[i]);
        mat.albedoColor[i] = pbr->base_color_factor[i];
    }

    mat.metallicValue = pbr->metallic_factor;
    mat.roughnessValue = pbr->roughness_factor;

    model_save_material_to_file(src->name, &mat);

    return mat;
}

bool model_load_cached_material_from_file(const char *filename, Material *material) {
    printf("Loading cached material...\n");
    FILE *f;
    fopen_s(&f, "res/Material.001", "r");
    if (!f) {
        fprintf(stderr, "Failed to load cached material: filename: %s\n", filename);
        return false;
    }

    char line[256];
    while (fgets(line, sizeof(line), f)) {
        sscanf_s(line, "base_color = %f, %f, %f, %f",
                 &material->albedoColor[0],
                 &material->albedoColor[1],
                 &material->albedoColor[2]);

        sscanf_s(line, "metallic = %f", &material->metallicValue);
        sscanf_s(line, "roughness = %f", &material->roughnessValue);
    }

    fclose(f);
    return true;
}

GLuint load_texture(const char *filename) {
    int width, height, comp;
    unsigned char *image_data = stbi_load(filename, &width, &height, &comp, 4);
    if (!image_data) {
        printf("[Error] Failed to load texture: %s\n", filename);
        return 0;
    }

    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image_data);
    glGenerateMipmap(GL_TEXTURE_2D);

    stbi_image_free(image_data);
    return texture;
}

GLuint create_white_texture() {
    unsigned char white_pixel[] = {255, 255, 255, 255};
    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, white_pixel);
    return texture;
}

void join_paths(const char *base_path, const char *filename, char *out_path, size_t out_size) {
    snprintf(out_path, out_size, "%s/%s", base_path, filename);
}

GLuint try_load_texture(const char *base_path, const cgltf_texture_view *texture_view) {
    if (texture_view && texture_view->texture && texture_view->texture->image) {
        const char *img_uri = texture_view->texture->image->uri;
        char full_path[512];
        join_paths(base_path, img_uri, full_path, sizeof(full_path));
        GLuint texId = load_texture(full_path);
        if (texId != 0) {
            printf("[Texture] Loaded: %s\n", full_path);
            return texId;
        }
    }
    return create_white_texture();  // fallback
}

void calculate_tangents(Vertex *vertices, uint16_t *indices, size_t index_count) {
    for (size_t i = 0; i < index_count; i += 3) {
        Vertex *v0 = &vertices[indices[i + 0]];
        Vertex *v1 = &vertices[indices[i + 1]];
        Vertex *v2 = &vertices[indices[i + 2]];

        vec3 edge1, edge2;
        glm_vec3_sub(v1->position, v0->position, edge1);
        glm_vec3_sub(v2->position, v0->position, edge2);

        vec2 deltaUV1, deltaUV2;
        glm_vec2_sub(v1->texcoord, v0->texcoord, deltaUV1);
        glm_vec2_sub(v2->texcoord, v0->texcoord, deltaUV2);

        float f = 1.0f / (deltaUV1[0] * deltaUV2[1] - deltaUV2[0] * deltaUV1[1]);

        vec3 tangent, bitangent;
        tangent[0] = f * (deltaUV2[1] * edge1[0] - deltaUV1[1] * edge2[0]);
        tangent[1] = f * (deltaUV2[1] * edge1[1] - deltaUV1[1] * edge2[1]);
        tangent[2] = f * (deltaUV2[1] * edge1[2] - deltaUV1[1] * edge2[2]);

        bitangent[0] = f * (-deltaUV2[0] * edge1[0] + deltaUV1[0] * edge2[0]);
        bitangent[1] = f * (-deltaUV2[0] * edge1[1] + deltaUV1[0] * edge2[1]);
        bitangent[2] = f * (-deltaUV2[0] * edge1[2] + deltaUV1[0] * edge2[2]);

        glm_vec3_normalize(tangent);
        glm_vec3_normalize(bitangent);

        glm_vec3_copy(tangent, v0->tangent);
        glm_vec3_copy(tangent, v1->tangent);
        glm_vec3_copy(tangent, v2->tangent);

        glm_vec3_copy(bitangent, v0->bitangent);
        glm_vec3_copy(bitangent, v1->bitangent);
        glm_vec3_copy(bitangent, v2->bitangent);
    }
}

void model_load(Model *model, const char *filename) {
    cgltf_options options = {0};
    cgltf_data *data = NULL;
    if (cgltf_parse_file(&options, filename, &data) != cgltf_result_success ||
        cgltf_load_buffers(&options, data, filename) != cgltf_result_success ||
        cgltf_validate(data) != cgltf_result_success) {
        fprintf(stderr, "Failed to load glTF file: %s\n", filename);
        return;
    }

    model->meshes = malloc(sizeof(Mesh) * data->meshes_count);
    model->mesh_count = data->meshes_count;

    const char *last_slash = strrchr(filename, '/');
    char base_path[512] = {0};
    if (last_slash) {
        size_t dir_len = last_slash - filename;
        strncpy(base_path, filename, dir_len);
        base_path[dir_len] = '\0';
    }

    for (size_t i = 0; i < data->meshes_count; i++) {
        cgltf_mesh *cgltf_mesh = &data->meshes[i];
        cgltf_primitive *prim = &cgltf_mesh->primitives[0];

        cgltf_material *material = prim->material;
        Material mat = {0};

        if (material) {
            mat.albedoTexture = try_load_texture(base_path, &material->pbr_metallic_roughness.base_color_texture);
            mat.useAlbedoMap = material->pbr_metallic_roughness.base_color_texture.texture != NULL;

            mat.metallicTexture = try_load_texture(base_path, &material->pbr_metallic_roughness.metallic_roughness_texture);
            mat.useMetallicMap = material->pbr_metallic_roughness.metallic_roughness_texture.texture != NULL;

            mat.roughnessTexture = mat.metallicTexture;  // metallic_roughness_texture is shared!
            mat.useRoughnessMap = mat.useMetallicMap;

            mat.aoTexture = try_load_texture(base_path, &material->occlusion_texture);
            mat.useAoMap = material->occlusion_texture.texture != NULL;

            mat.normalTexture = try_load_texture(base_path, &material->normal_texture);
            mat.useNormalMap = material->normal_texture.texture != NULL;

            mat.metallicValue = material->pbr_metallic_roughness.metallic_factor;
            mat.roughnessValue = material->pbr_metallic_roughness.roughness_factor;
            memcpy(mat.albedoColor, material->pbr_metallic_roughness.base_color_factor, sizeof(float) * 3);
        }

        model->meshes[i].material = mat;

        // Vertices och indices (oförändrat):
        size_t vertex_count = prim->attributes[0].data->count;
        Vertex *vertices = malloc(sizeof(Vertex) * vertex_count);

        for (size_t a = 0; a < prim->attributes_count; ++a) {
            cgltf_attribute *attr = &prim->attributes[a];
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
        uint16_t *indices = (uint16_t *)((uint8_t *)index_accessor->buffer_view->buffer->data + index_accessor->buffer_view->offset + index_accessor->offset);
        size_t index_count = index_accessor->count;

        calculate_tangents(vertices, indices, index_count);
        mesh_init(&model->meshes[i], vertices, indices, vertex_count, index_count);
    }

    cgltf_free(data);
}