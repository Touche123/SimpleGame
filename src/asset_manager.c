#include "asset_manager.h"

#include <sys/stat.h>

#define MAX_ASSETS 128

static MaterialAsset material_assets[MAX_ASSETS];
static int material_asset_count = 0;

static ShaderAsset shader_assets[MAX_ASSETS];
static int shader_asset_count = 0;

static TextureAsset texture_assets[MAX_ASSETS];
static int texture_asset_count = 0;

static ModelAsset model_assets[MAX_ASSETS];
static int model_asset_count = 0;

static time_t asset_get_file_timestamp(const char* path) {
    struct stat attr;
    if (stat(path, &attr) == 0) {
        return attr.st_mtime;
    }
    return 0;
}

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

    // if (!asset_material_load(path, &asset->material)) {
    //     printf("[Material] Missing: %s (using default)\n", path);
    // } else {

    //}

    return &asset->material;
}

ModelAsset* asset_create_model_empty(const char* name) {
    for (int i = 0; i < model_asset_count; ++i) {
        if (strcmp(model_assets[i].name, name) == 0) {
            return &model_assets[i];
        }
    }

    if (model_asset_count >= MAX_ASSETS) return NULL;

    ModelAsset* asset = &model_assets[model_asset_count++];
    strncpy(asset->name, name, sizeof(asset->name));
    asset->name[sizeof(asset->name) - 1] = '\0';

    return asset;
}

ModelAsset* asset_get_model(const char* name) {
    for (int i = 0; i < model_asset_count; ++i) {
        if (strcmp(model_assets[i].name, name) == 0) {
            return &model_assets[i];
        }
    }

    if (model_asset_count >= MAX_ASSETS) return NULL;

    ModelAsset* asset = &model_assets[model_asset_count++];
    strncpy(asset->name, name, sizeof(asset->name));
    asset->name[sizeof(asset->name) - 1] = '\0';

    model_load(&asset->model, name);

    return asset;
}

ShaderAsset* asset_get_shader(const char* name) {
    for (int i = 0; i < shader_asset_count; ++i) {
        if (strcmp(shader_assets[i].name, name) == 0) {
            return &shader_assets[i];
        }
    }

    if (shader_asset_count >= MAX_ASSETS) return NULL;

    ShaderAsset* asset = &shader_assets[shader_asset_count++];
    strncpy(asset->name, name, sizeof(asset->name));
    asset->name[sizeof(asset->name) - 1] = '\0';

    snprintf(asset->vs_path, sizeof(asset->vs_path), "res/shaders/%s.vert", name);
    snprintf(asset->fs_path, sizeof(asset->fs_path), "res/shaders/%s.frag", name);

    asset->vs_timestamp = asset_get_file_timestamp(asset->vs_path);
    asset->fs_timestamp = asset_get_file_timestamp(asset->fs_path);

    printf("vs path: %s\n", asset->vs_path);
    printf("fs path: %s\n", asset->fs_path);
    char* vss_path = shader_read_file(asset->vs_path, asset->vs_size);
    char* fss_path = shader_read_file(asset->fs_path, asset->fs_size);
    asset->shader.id = shader_compile(asset->vs_path, asset->fs_path);

    asset->shader.vertex_shader_filename = asset->vs_path;
    asset->shader.fragment_shader_filename = asset->fs_path;
    // if (!shader_load(&asset->shader, asset->vs_path, asset->fs_path)) {
    //     printf("[Shader] Failed to load %s\n", name);
    // } {
    //     printf("[Shader] Loaded: %s\n", name);
    // }

    return asset;
}

void asset_shader_load(void) {
}

void asset_material_load(void) {
}

TextureAsset* asset_get_texture(const char* name) {
    for (int i = 0; i < texture_asset_count; ++i) {
        if (strcmp(texture_assets[i].name, name) == 0) {
            return &texture_assets[i].texture;
        }
    }

    if (texture_asset_count >= MAX_ASSETS) return NULL;

    GLuint texId = load_texture(name);
    if (texId == 0) {
        printf("[TextureAsset] Failed to load texture: %s\n", name);
        return NULL;
    }

    TextureAsset* asset = &texture_assets[texture_asset_count++];
    strncpy(asset->name, name, sizeof(asset->name));
    // asset->texture.id = texId;
    // asset->texture.width = 0;  // fyll i om du vill
    // asset->texture.height = 0;

    return &asset->texture;
}

GLuint asset_load_texture(const char* path) {
    int width, height, comp;
    unsigned char* image_data = stbi_load(path, &width, &height, &comp, 4);
    if (!image_data) {
        printf("[Error] Failed to load texture: %s\n", path);
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
