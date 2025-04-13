#include "asset_manager.h"

#define MAX_ASSETS 128

typedef struct MaterialAsset{
    char name[128];
    Material material;
} MaterialAsset;

typedef struct ShaderAsset {
    char name[128];
    Shader shader;
} ShaderAsset;

typedef struct TextureAsset {
    char name[128];
    Texture texture;
} TextureAsset;

static MaterialAsset material_assets[MAX_ASSETS];
static int material_asset_count = 0;

static ShaderAsset shader_assets[MAX_ASSETS];
static int shader_asset_count = 0;

static TextureAsset texture_assets[MAX_ASSETS];
static int texture_asset_count = 0;

void asset_manager_init(void) {
    
}

void asset_manager_shutdown(void) {
    material_asset_count = 0;
    shader_asset_count = 0;
    texture_asset_count = 0;
}

Material* asset_get_material(const char* name) {
    for (int i = 0; i < material_asset_count; ++i) {
        if (strcmp(material_assets[i].name, name) == 0) {
            return &material_assets[i].material;
        }
    }

    if (material_asset_count >= MAX_ASSETS) return NULL;

    MaterialAsset* asset = &material_assets[material_asset_count++];
    strncpy(asset->name, name, sizeof(asset->name));

    char path[256];
    snprintf(path, sizeof(path), "materials/%s.mat", name);


    if (!load_material_from_file(path, &asset->material)) {
        printf("[Material] Missing: %s (using default)\n", path);
    } else {

    }

    return &asset->material;
}

Shader* asset_get_shader(const char* name) {
    for (int i = 0; i < shader_asset_count; ++i) {
        if (strcmp(shader_assets[i].name, name) == 0) {
            return &shader_assets[i].shader;
        }
    }

    if (shader_asset_count >= MAX_ASSETS) return NULL;

    ShaderAsset* asset = &shader_assets[shader_asset_count++];
    strncpy(asset->name, name, sizeof(asset->name));

    char vs_path[256], fs_path[256];
    snprintf(vs_path, sizeof(vs_path), "shaders/%s.vert", name);
    snprintf(fs_path, sizeof(fs_path), "shaders/%s.frag", name);

    if (!shader_load(&asset->shader, vs_path, fs_path)) {
        printf("[Shader] Failed to load %s\n", name);
    }

    return &asset->shader;
}

void asset_shader_load(void) {
}

void asset_material_load(void) {
}

