#ifndef ASSET_MANAGER_H
#define ASSET_MANAGER_H

#include "core.h"
#include "model.h"
#include "shader.h"

typedef struct Texture {
} Texture;

typedef struct ShaderAsset {
    char name[128];
    char vs_path[256];
    char fs_path[256];
    Shader shader;
    time_t vs_timestamp;
    time_t fs_timestamp;
} ShaderAsset;

typedef struct ModelAsset {
    char name[128];
    Model model;
} ModelAsset;

void asset_manager_init(void);
void asset_manager_shutdown(void);

Material* asset_get_material(const char* name);
ShaderAsset* asset_get_shader(const char* name);
ModelAsset* asset_get_model(const char* name);

void asset_poll_watchers(void);
#endif
