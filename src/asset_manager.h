#ifndef ASSET_MANAGER_H
#define ASSET_MANAGER_H

#include "core.h"
#include "model.h"
#include "shader.h"

typedef struct Texture {
    GLuint id;
    int width;
    int height;
} Texture;

typedef struct TextureAsset {
    char name[128];
    Texture texture;
} TextureAsset;

typedef struct MaterialAsset {
    char name[128];
    Material material;
} MaterialAsset;

typedef struct ShaderAsset {
    char name[128];
    char vs_path[256];
    size_t vs_size;
    char fs_path[256];
    size_t fs_size;
    Shader shader;
    time_t vs_timestamp;
    time_t fs_timestamp;
} ShaderAsset;

typedef struct ModelAsset {
    char name[128];
    size_t model_size;
    Model model;
} ModelAsset;

void asset_manager_init(void);
void asset_manager_shutdown(void);

Material* asset_get_material(const char* name);
ShaderAsset* asset_get_shader(const char* name);
ModelAsset* asset_get_model(const char* name);
TextureAsset* asset_get_texture(const char* name);
GLuint asset_load_texture(const char* path);
ModelAsset* asset_create_model_empty(const char* name);

void asset_poll_watchers(void);
#endif
