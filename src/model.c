#include "model.h"

#include <stdio.h>

#include "cgltf.h"
#include "mesh.h"
#include <string.h>
#include <stdlib.h>

void model_save(Model *model, const char *filename) {

}

bool model_save_material_to_file(const char *filename, const Material* material) {
    FILE *f; 

    fopen_s(&f, filename, "w");
    if (!f) return false;

    fprintf(f, "base_color = %f, %f, %f, %f\n",
            material->base_color_factor[0],
            material->base_color_factor[1],
            material->base_color_factor[2],
            material->base_color_factor[3]);

    fprintf(f, "metallic = %f\n", material->metallic_factor);
    fprintf(f, "roughness = %f\n", material->roughness_factor);
    fclose(f);
    return true;
}

Material convert_from_cgltf(const cgltf_material* src) {
    Material mat = {0};

    const cgltf_pbr_metallic_roughness* pbr = &src->pbr_metallic_roughness;

    for (int i = 0; i < 4; ++i) {
        mat.base_color_factor[i] = pbr->base_color_factor[i];
    }

    mat.metallic_factor = pbr->metallic_factor;
    mat.roughness_factor = pbr->roughness_factor;
    
    model_save_material_to_file(src->name, &mat);    

    return mat;
}

bool model_load_cached_material_from_file(const char* filename, Material* material) {
    FILE *f;
    fopen_s(&f, "res/Material.001", "r");
    if (!f) {
        fprintf(stderr, "Failed to load cached material: filename: %s\n", filename);
        return false;
    }

    char line[256];
    while (fgets(line, sizeof(line), f)) {
        sscanf_s(line, "base_color = %f, %f, %f, %f",
                &material->base_color_factor[0],
                &material->base_color_factor[1],
                &material->base_color_factor[2],
                &material->base_color_factor[3]);

        sscanf_s(line, "metallic = %f", &material->metallic_factor);
        sscanf_s(line, "roughness = %f", &material->roughness_factor);
    }

    fclose(f);
    return true;
}

void model_load(Model *model, const char *filename) {
    cgltf_options options = {0};
    cgltf_data *data = NULL;
    cgltf_result result = cgltf_parse_file(&options, filename, &data);
    if (result != cgltf_result_success) {
        fprintf(stderr, "Failed to parse glTF file\n");
	return;
    }
    result = cgltf_load_buffers(&options, data, filename);
    if (result != cgltf_result_success) {
        fprintf(stderr, "Failed to load buffers\n");
        cgltf_free(data);
	return;
    }
    result = cgltf_validate(data);

    model->meshes = malloc(sizeof(Mesh) * data->meshes_count);
    model->mesh_count = data->meshes_count;

    for (size_t i = 0; i < data->meshes_count; i++) {
        cgltf_mesh *cgltf_mesh = &data->meshes[i];
        cgltf_primitive *prim = &cgltf_mesh->primitives[0];

        // Loopa igenom alla material i glTF-filen
        for (size_t m = 0; m < data->materials_count; m++) {
            cgltf_material *material = &data->materials[m];
            if (!model_load_cached_material_from_file(
                        filename, 
                        &model->meshes[i].material))
                model->meshes[i].material = convert_from_cgltf(material);
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

        //model->meshes[i].texture = texture;

        size_t index_count = index_accessor->count;
        // cgltf_free(data);
        //  free(vertices);
        mesh_init(&model->meshes[i], vertices, indices, vertex_count, index_count);
    }
}
