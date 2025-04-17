#ifndef ASSET_MANAGER_H
#define ASSET_MANAGER_H

#include "defines.h"
#include "shader.h"
#include "model.h"

typedef struct Texture{
} Texture;

typedef struct ShaderAsset {
    char name[128];
    char vs_path[256];
    char fs_path[256];
    Shader shader;
    time_t vs_timestamp;
    time_t fs_timestamp;
} ShaderAsset;

void asset_manager_init(void);
void asset_manager_shutdown(void);

Material* asset_get_material(const char* name);
ShaderAsset* asset_get_shader(const char* name);

void asset_poll_watchers(void);
#endif
